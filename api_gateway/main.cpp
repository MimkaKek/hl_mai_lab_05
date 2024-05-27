#include <Poco/Net/HTTPResponse.h>
#include <string>
#include <iostream>
#include <sstream>

#include <Poco/URI.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/StreamCopier.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Object.h>

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"

using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerParams;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::ServerSocket;

#include "../database/cache.h"
#include "../config/config.h"
#include "../web_server/lib/common.h"
#include "../web_server/lib/circuitbreaker.h"

class GatewayHandler : public HTTPRequestHandler
{
protected:
    std::string get_key(const std::string &method, const std::string &base_path, const std::string &query, const std::string &basic_auth){
        return method+":"+base_path+":"+query+":"+basic_auth;
    }

    std::string get_cached(const std::string &method, const std::string &base_path, const std::string &query, const std::string &basic_auth){
        std::string key =  get_key(method, base_path, query, basic_auth);
        std::string result;

        if(database::Cache::get().get(key,result)){
            return result;
        } else {
            return std::string();
        }
    }

    void put_cache(const std::string &method, const std::string &base_path, const std::string &query, const std::string &basic_auth,const std::string &result){
        std::string key =  get_key(method, base_path, query, basic_auth);
        database::Cache::get().put(key,result);
    }

public:
    void handleRequest(HTTPServerRequest &request, [[maybe_unused]] HTTPServerResponse &response)
    {

        static CircuitBreaker circuitBreaker;

        std::string base_url_user  = "http://" + Config::get().get_user_service_host() + ":" + Config::get().get_user_service_port();
        std::string base_url_party = "http://" + Config::get().get_party_service_host() + ":" + Config::get().get_party_service_port();
        std::string base_url_trip  = "http://" + Config::get().get_trip_service_host() + ":" + Config::get().get_trip_service_port();
        std::string base_url_path  = "http://" + Config::get().get_path_service_host() + ":" + Config::get().get_path_service_port();

        std::string scheme;
        std::string info;
        request.getCredentials(scheme, info);

        std::string login, password;
        get_identity(info, login, password);
        Poco::Net::HTTPResponse::HTTPStatus status_response;
        std::string body_response;

        // EXTRACT BODY
        std::istream& istr = request.stream();
        std::stringstream ss;
        Poco::StreamCopier::copyStream(istr, ss);
        std::string body = ss.str();

        // USER REGISTER
        if (hasSubstr(request.getURI(), "/user") && request.getMethod() == "POST") 
        {
            if (!circuitBreaker.check(base_url_user)) 
            {
                response_error(Poco::Net::HTTPResponse::HTTP_FORBIDDEN, 
                                    "/errors/service_timeout", 
                                    "/gateway",
                                    "Internal exception", 
                                    "Waiting for " + base_url_user + " service up...", 
                                    response);
                return;
            }
            send_request(request.getMethod(), base_url_user + request.getURI(), AUTH_BASIC, info, body, body_response, status_response);
            if (status_response == Poco::Net::HTTPResponse::HTTP_OK 
                || status_response == Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED 
                || status_response == Poco::Net::HTTPResponse::HTTP_BAD_REQUEST) 
            {
                circuitBreaker.success(base_url_user);
            }
            else 
            {
                circuitBreaker.fail(base_url_user);
            }
            send_response(response, status_response, body_response);
            return;
        }

        // MAIN
        if (scheme == AUTH_BASIC)
        {
            
            // GETTING SERVICE URL

            std::string base_url = "";

            if (hasSubstr(request.getURI(),"/user") ||
                hasSubstr(request.getURI(),"/user/search"))
                base_url = base_url_user;

            if (hasSubstr(request.getURI(),"/path") ||
                hasSubstr(request.getURI(),"/path/search"))
                base_url = base_url_path;

            if (hasSubstr(request.getURI(),"/trip") ||
                hasSubstr(request.getURI(),"/trips"))
                base_url = base_url_trip;

            if (hasSubstr(request.getURI(),"/parties") ||
                hasSubstr(request.getURI(),"/party") ||
                hasSubstr(request.getURI(),"/party/join"))
                base_url = base_url_party;
            
            // CHECK CACHED RESPONSE

            if(request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) 
            {
                std::string cache_result = get_cached(request.getMethod(), base_url, request.getURI(),info);
                if(!cache_result.empty())
                {
                    std::cout << "# api gateway - from cache : " << cache_result << std::endl;
                    send_response(response, Poco::Net::HTTPResponse::HTTP_OK, cache_result);
                    return;
                }
                std::cout << "# api gateway - cache empty, prepare request..." << std::endl;
            }

            // CB CHECK

            if (!circuitBreaker.check(base_url)) 
            {
                response_error(Poco::Net::HTTPResponse::HTTP_FORBIDDEN, 
                                    "/errors/service_timeout", 
                                    "/gateway",
                                    "Internal exception", 
                                    "Waiting for " + base_url + " service up...", 
                                    response);
                return;
            }

            std::cout << "# api gateway - redirect to " << base_url << std::endl;

            // AUTH USER
            std::string token;
            send_request(Poco::Net::HTTPRequest::HTTP_GET, base_url_user + "/auth", AUTH_BASIC, info, body, token, status_response);
            if (status_response != Poco::Net::HTTPResponse::HTTP_OK || token.empty())
            {
                circuitBreaker.fail(base_url_user);
                response_error(Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED, 
                                    "/errors/not_authorized", 
                                    "/user",
                                    "Internal exception", 
                                    "Error while auth user...", 
                                    response);
                return;
            }

            // SEND SERVICE REQUEST
            std::string body_response;
            send_request(request.getMethod(), base_url + request.getURI(), AUTH_BEARER, token, body, body_response, status_response);
            std::cout << "# api gateway - result: " << std::endl << body_response << std::endl;

            if (status_response == Poco::Net::HTTPResponse::HTTP_OK 
                || status_response == Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED 
                || status_response == Poco::Net::HTTPResponse::HTTP_BAD_REQUEST) 
            {
                    circuitBreaker.success(base_url);
                    send_response(response, status_response, body_response);
                    put_cache(request.getMethod(), base_url, request.getURI(),info,body_response);
            }
            else 
            {
                circuitBreaker.fail(base_url);
                send_response(response, status_response, body_response);
            }
        }
    };
};

class HTTPRequestFactory : public HTTPRequestHandlerFactory
{
    HTTPRequestHandler *createRequestHandler([[maybe_unused]] const HTTPServerRequest &request)
    {
        return new GatewayHandler();
    }
};

class HTTPWebServer : public Poco::Util::ServerApplication
{
protected:
    int main([[maybe_unused]] const std::vector<std::string> &args)
    {
        ServerSocket svs(Poco::Net::SocketAddress("0.0.0.0", 8888));
        HTTPServer srv(new HTTPRequestFactory(), svs, new HTTPServerParams);

        std::cout << "Started gatweay on port: 8888" << std::endl;
        srv.start();
        waitForTerminationRequest();
        srv.stop();

        return Application::EXIT_OK;
    }
};

int main(int argc, char *argv[])
{
    HTTPWebServer app;
    return app.run(argc, argv);
}