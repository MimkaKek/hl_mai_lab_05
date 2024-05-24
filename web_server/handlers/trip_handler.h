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
#include <Poco/JSON/Array.h>
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

#include "../../database/trip.h"
#include "../lib/common.h"

class TripHandler : public HTTPRequestHandler
{

    public:
    TripHandler(const std::string &format) : _format(format)
    {
    }

    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response)
    {
        HTMLForm form(request, request.stream());
        std::string instance = "/trip";
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

            if (hasSubstr(request.getURI(), "/trips") && (request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET))
            {
                Poco::JSON::Array arr;
                Poco::JSON::Array::Ptr parties = get_parties(authinfo);
                for (Poco::JSON::Array::ConstIterator it = parties->begin(); it != parties->end(); ++it) {
                    Poco::JSON::Object::Ptr party_obj = it->extract<Poco::JSON::Object::Ptr>();
                    long id_trip = party_obj->getValue<long>("id_trip");
                    arr.add(database::Trip::read_by_id(id_trip)->toJSON());
                }
                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(arr, ostr);

                return;
            }
            else if (hasSubstr(request.getURI(), "/trip") && (request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET))
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
                long id = atol(form.get("id_trip").c_str());
                if (!is_in_party(authinfo, id)) {
                    response_error(Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED, 
                                   "/errors/not_authorized",
                                   instance,
                                   "Internal exception", 
                                   "User not in party of trip...", 
                                   response);
                    return;
                }
                
                std::optional<database::Trip> result = database::Trip::read_by_id(id);
                if (result)
                {
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(result->toJSON(), ostr);
                    return;
                }
                else
                {
                    response_error(Poco::Net::HTTPResponse::HTTP_NOT_FOUND, 
                                   "/errors/not_found",
                                   instance,
                                   "Internal exception", 
                                   "Trip doesn't exists...", 
                                   response);
                    return;
                }
            }
            else if (hasSubstr(request.getURI(), "/trip") && request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST)
            {
                if (!form.has("id_trip") || !form.has("id_path") || !form.has("name") || !form.has("start_time") || !form.has("fin_time"))
                {
                    response_error(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, 
                                   "/errors/bad_request", 
                                   instance,
                                   "Internal exception", 
                                   "Request doesn't have nedeed data...", 
                                   response);
                    return;
                }
                database::Trip trip;
                trip.id() = atol(form.get("id_trip").c_str());
                trip.id_owner() = id_user;
                trip.id_path() = atol(form.get("id_path").c_str());
                trip.name() = form.get("name");
                trip.start_time() = form.get("start_time");
                trip.fin_time() = form.get("fin_time");
                trip.add();
                
                if (!init_party_remote(authinfo, trip.get_id())) {
                    response_error(Poco::Net::HTTPResponse::HTTP_FORBIDDEN, 
                                   "/errors/forbidden", 
                                   instance,
                                   "Internal exception", 
                                   "Failed to create new party...", 
                                   response);
                    return;
                }

                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();
                ostr << trip.get_id();
                return;
            }
            else if (hasSubstr(request.getURI(), "/trip") && request.getMethod() == Poco::Net::HTTPRequest::HTTP_PUT)
            {
                if (!form.has("id") || !form.has("id_path") || !form.has("name") || !form.has("start_time") || !form.has("fin_time"))
                {
                    response_error(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, 
                                   "/errors/bad_request", 
                                   instance,
                                   "Internal exception", 
                                   "Request doesn't have nedeed data...", 
                                   response);
                    return;
                }
                database::Trip trip;
                trip.id() = atol(form.get("id").c_str());
                trip.id_owner() = id_user;
                trip.id_path() = atol(form.get("id_path").c_str());
                trip.name() = form.get("name");
                trip.start_time() = form.get("start_time");
                trip.fin_time() = form.get("fin_time");

                trip.update();

                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();
                ostr << trip.get_id();
                return;
            }
        }
        catch (std::exception &ex)
        {
            std::cout << "Error: " << ex.what() << std::endl;
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
