#pragma once

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Timestamp.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/Exception.h"
#include "Poco/ThreadPool.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include <iostream>
#include <iostream>
#include <fstream>

using Poco::DateTimeFormat;
using Poco::DateTimeFormatter;
using Poco::ThreadPool;
using Poco::Timestamp;
using Poco::Net::HTMLForm;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerParams;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::NameValueCollection;
using Poco::Net::ServerSocket;
using Poco::Util::Application;
using Poco::Util::HelpFormatter;
using Poco::Util::Option;
using Poco::Util::OptionCallback;
using Poco::Util::OptionSet;
using Poco::Util::ServerApplication;

#include "../../database/party.h"
#include "../lib/common.h"

class PartyHandler : public HTTPRequestHandler
{

public:
    PartyHandler(const std::string &format) : _format(format)
    {
    }

    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response)
    {
        HTMLForm form(request, request.stream());
        std::string instance = "/party";
        try
        {
            Poco::JSON::Object::Ptr ret = auth_user(request);
            if (!ret->getValue<bool>("status")) {
                response_error(Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED, 
                                "/errors/not_authorized", 
                                instance,
                                "Internal exception", 
                                "Failed token check...", 
                                response);
                return;
            }

            long id_user         = ret->getValue<long>("id_user");
            std::string login    = ret->getValue<std::string>("login");
            std::string authinfo = ret->getValue<std::string>("authinfo");

            if (hasSubstr(request.getURI(), "/parties") && (request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET))
            {
                auto results = database::Party::read_by_id_part(id_user);
                Poco::JSON::Array arr;
                for (auto s : results)
                    arr.add(s.toJSON());
                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(arr, ostr);

                return;
            }
            else if (hasSubstr(request.getURI(), "/party/join") && (request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST))
            {
                if (!form.has("id_trip"))
                {
                    response_error(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, 
                                   "/errors/bad_request", 
                                   instance,
                                   "Internal exception", 
                                   "Request doesn't have nedeed data...", 
                                   response);
                    return;
                }
                long id_trip = atol(form.get("id_trip").c_str());
                std::optional<std::vector<database::Party>> result = database::Party::read_by_id_trip(id_trip);
                if (!result)
                {
                    response_error(Poco::Net::HTTPResponse::HTTP_NOT_FOUND, 
                                   "/errors/not_found",
                                   instance,
                                   "Internal exception", 
                                   "Party doesn't exists...", 
                                   response);
                    return;
                }

                for (auto party: *result) {
                    if(party.get_id_participant() == id_user) {
                        response_error(Poco::Net::HTTPResponse::HTTP_FORBIDDEN, 
                                        "/errors/already_exists",
                                        instance,
                                        "Internal exception", 
                                        "Party already exists...", 
                                        response);
                        return;                   
                    }
                }

                database::Party party;
                party.id_participant() = id_user;
                party.id_trip() = id_trip;
                party.add();
                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();
                ostr << party.get_id_trip();
                return;
                
            }
            else if (hasSubstr(request.getURI(), "/party") && (request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET))
            {
                if (!form.has("id_trip"))
                {
                    response_error(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, 
                                   "/errors/bad_request", 
                                   instance,
                                   "Internal exception", 
                                   "Request doesn't have nedeed data...", 
                                   response);
                    return;
                }
                long id_trip = atol(form.get("id_trip").c_str());

                std::optional<std::vector<database::Party>> result = database::Party::read_by_id_trip(id_trip);
                if (result)
                {
                    for (auto party: *result) {
                        if(party.get_id_participant() == id_user) {
                            response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                            response.setChunkedTransferEncoding(true);
                            response.setContentType("application/json");
                            std::ostream &ostr = response.send();
                            Poco::JSON::Stringifier::stringify(party.toJSON(), ostr);
                            return;                   
                        }
                    }
                    
                    response_error(Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED, 
                                   "/errors/not_authorized",
                                   instance,
                                   "Internal exception", 
                                   "User not in party...", 
                                   response);
                    return;
                }
                else
                {
                    response_error(Poco::Net::HTTPResponse::HTTP_NOT_FOUND, 
                                   "/errors/not_found",
                                   instance,
                                   "Internal exception", 
                                   "Party doesn't exists...", 
                                   response);
                    return;
                }
            }
            else if (hasSubstr(request.getURI(), "/party") && request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST)
            {
                if (!form.has("id_trip"))
                {
                    response_error(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, 
                                   "/errors/bad_request", 
                                   instance,
                                   "Internal exception", 
                                   "Request doesn't have nedeed data...", 
                                   response);
                    return;
                }
                database::Party party;
                party.id_participant() = id_user;
                party.id_trip() = atol(form.get("id_trip").c_str());
                party.add();
                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();
                ostr << party.get_id_trip();
                return;
            }
        }
        catch (std::exception &e)
        {
            std::cout << e.what() << std::endl;
            response_error(Poco::Net::HTTPResponse::HTTP_FORBIDDEN, 
                            "/errors/forbidden", 
                            instance,
                            "Internal exception", 
                            "Check logs for info...", 
                            response);
        }

        response_error(Poco::Net::HTTPResponse::HTTP_NOT_FOUND, 
                        "/errors/not_found", 
                        instance,
                        "Internal exception", 
                        "Request not found", 
                        response);
    }

private:
    std::string _format;
};
