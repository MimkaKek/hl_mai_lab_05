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

#include "../../database/trip.h"
#include "../../helper.h"
#include "../lib/common.h"

class TripHandler : public HTTPRequestHandler
{

public:
    TripHandler(const std::string &format) : _format(format)
    {
    }

    void handleRequest(HTTPServerRequest &request,
                       HTTPServerResponse &response)
    {
        HTMLForm form(request, request.stream());
        try
        {
            std::string scheme;
            std::string info;
            long id_user {-1};
            std::string login;
            request.getCredentials(scheme, info);
            std::cout << "scheme: " << scheme << " identity: " << info << std::endl;
            if(scheme == "Bearer") {
                if(!extract_payload(info,id_user,login)) {
                    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_FORBIDDEN);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                    root->set("type", "/errors/not_authorized");
                    root->set("title", "Internal exception");
                    root->set("status", "403");
                    root->set("detail", "user not authorized");
                    root->set("instance", "/trip");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(root, ostr);
                    return;                   
                }
            }
            std::cout << "id:" << id_user << " login:" << login << std::endl;

            if (hasSubstr(request.getURI(), "/trips") && (request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET))
            {
                //long id = atol(form.get("user_id").c_str());
                auto results = database::Trip::read_by_id_owner(id_user);
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
            else if (hasSubstr(request.getURI(), "/trip") && (request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET))
            {
                std::cout << "Test 1" << std::endl;
                long id_trip = atol(form.get("id").c_str());

                std::cout << "Test 2" << std::endl;
                std::optional<database::Trip> result = database::Trip::read_by_id(id_trip);
                std::cout << "Test 3" << std::endl;
                if (result)
                {
                    if(result->get_id_owner() != id_user) {
                        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_FORBIDDEN);
                        response.setChunkedTransferEncoding(true);
                        response.setContentType("application/json");
                        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                        root->set("type", "/errors/not_authorized");
                        root->set("title", "Internal exception");
                        root->set("status", "403");
                        root->set("detail", "user not authorized");
                        root->set("instance", "/trip");
                        std::ostream &ostr = response.send();
                        Poco::JSON::Stringifier::stringify(root, ostr);
                        return;                   
                    }

                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(result->toJSON(), ostr);
                    return;
                }
                else
                {
                    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                    root->set("type", "/errors/not_found");
                    root->set("title", "Internal exception");
                    root->set("status", "404");
                    root->set("detail", "not found by trip id");
                    root->set("instance", "/trip");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(root, ostr);
                    return;
                }
            }
            else if (hasSubstr(request.getURI(), "/trip") && request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST)
            {
                database::Trip trip;
                trip.id() = atol(form.get("id").c_str());
                trip.id_owner() = id_user;
                trip.id_path() = atol(form.get("id_path").c_str());
                trip.name() = form.get("name");
                trip.start_time() = form.get("start_time");
                trip.fin_time() = form.get("fin_time");

                trip.add();

                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();
                ostr << trip.get_id();
                return;
            }
            else if (hasSubstr(request.getURI(), "/trip") && request.getMethod() == Poco::Net::HTTPRequest::HTTP_PUT)
            {
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
            std::cout << "exception:" << ex.what() << std::endl;
        }

        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/errors/not_found");
        root->set("title", "Internal exception");
        root->set("status", Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
        root->set("detail", "request ot found");
        root->set("instance", "/trip");
        std::ostream &ostr = response.send();
        std::cout << "Request (TRIP) not found!" << std::endl;
        Poco::JSON::Stringifier::stringify(root, ostr);
    }

private:
    std::string _format;
};
