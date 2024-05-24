#ifndef USEHANDLER_H
#define USEHANDLER_H

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
#include <Poco/Hash.h>
#include <exception>
#include <iostream>
#include <fstream>
#include <unordered_map>

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

#include "../../database/user.h"
#include "../lib/common.h"

class UserHandler : public HTTPRequestHandler
{
private:
    bool check_name(const std::string &name, std::string &reason)
    {
        if (name.length() < 3)
        {
            reason = "Name must be at leas 3 signs";
            return false;
        }

        if (name.find(' ') != std::string::npos)
        {
            reason = "Name can't contain spaces";
            return false;
        }

        if (name.find('\t') != std::string::npos)
        {
            reason = "Name can't contain spaces";
            return false;
        }

        return true;
    };

    bool check_email(const std::string &email, std::string &reason)
    {
        if (email.find('@') == std::string::npos)
        {
            reason = "Email must contain @";
            return false;
        }

        if (email.find(' ') != std::string::npos)
        {
            reason = "EMail can't contain spaces";
            return false;
        }

        if (email.find('\t') != std::string::npos)
        {
            reason = "EMail can't contain spaces";
            return false;
        }

        return true;
    };

public:
    UserHandler(const std::string &format) : _format(format)
    {
    }

    Poco::JSON::Object::Ptr mask_password(Poco::JSON::Object::Ptr src)
    {
        if (src->has("password"))
            src->set("password", "*******");
        return src;
    }

    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response)
    {
        HTMLForm form(request, request.stream());
        std::string instance {"/user"};
        try
        {
            std::string scheme;
            std::string info;
            request.getCredentials(scheme, info);
            std::cout << "scheme: " << scheme << " identity: " << info << std::endl;
            std::string login, password;

            if (hasSubstr(request.getURI(), "/user/auth_check") && request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) 
            {
                if (scheme == "Bearer") {
                    long id_user;
                    if(!extract_payload_local(info,id_user,login)) {
                        response_error(Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED, 
                                    "/errors/not_authorized", 
                                    instance,
                                    "Internal exception", 
                                    "Failed token check...", 
                                    response);
                        return;               
                    }
                    else {
                        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK);
                        response.setChunkedTransferEncoding(true);
                        response.setContentType("application/json");
                        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                        root->set("id", id_user);
                        root->set("login", login);
                        std::ostream &ostr = response.send();
                        Poco::JSON::Stringifier::stringify(root, ostr);
                    }
                }
                response_error(Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED, 
                               "/errors/not_authorized", 
                                    instance,
                              "Internal exception", 
                             "Wrong auth scheme...", 
                                  response);
                return;
            }
            else if (hasSubstr(request.getURI(), "/user/auth") && request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET)
            {
                if (scheme == "Basic")
                {
                    get_identity(info, login, password);
                    auto hasher = Poco::Hash<std::string>();
                    std::size_t hashed_pass = hasher(password) >> 1;
                    if (auto id = database::User::auth(login, hashed_pass))
                    {
                        std::string token = generate_token(*id,login);
                        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                        response.setChunkedTransferEncoding(true);
                        response.setContentType("application/json");
                        std::ostream &ostr = response.send();
                        ostr << "{ \"id\" : \"" << *id << "\", \"Token\" : \""<< token <<"\"}" << std::endl;
                        return;
                    }
                }
                response_error(Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED, 
                               "/errors/not_authorized", 
                               instance,
                               "Internal exception", 
                               "Wrong auth scheme...", 
                               response);
                return;
            }
            else if (hasSubstr(request.getURI(), "/user/search") && request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET)
            {

                std::string instance = "/user";
                Poco::JSON::Object::Ptr ret = auth_user(request, false);
                if (!ret->getValue<bool>("status")) {
                    response_error(Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED, 
                                    "/errors/not_authorized", 
                                    instance,
                                    "Internal exception", 
                                    "Failed token check...", 
                                    response);
                    return;
                }

                if (!form.has("first_name") || !form.has("last_name"))
                {
                    response_error(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, 
                                    "/errors/bad_request", 
                                    instance,
                                    "Internal exception", 
                                    "Request doesn't have needed data", 
                                    response);
                    return;
                }

                std::string fn = form.get("first_name");
                std::string ln = form.get("last_name");
                auto results = database::User::search(fn, ln);
                Poco::JSON::Array arr;
                for (auto s : results)
                    arr.add(mask_password(s.toJSON()));
                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(arr, ostr);

                return;
            }
            else if (hasSubstr(request.getURI(), "/user") && (request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET))
            {

                std::string instance = "/user";
                Poco::JSON::Object::Ptr ret = auth_user(request, false);
                if (!ret->getValue<bool>("status")) {
                    response_error(Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED, 
                                    "/errors/not_authorized", 
                                    instance,
                                    "Internal exception", 
                                    "Failed token check...", 
                                    response);
                    return;
                }

                if (!form.has("id"))
                {
                    response_error(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, 
                                    "/errors/bad_request", 
                                    instance,
                                    "Internal exception", 
                                    "Request doesn't have needed data", 
                                    response);
                    return;
                }
                long id = atol(form.get("id").c_str());

                std::optional<database::User> result = database::User::read_by_id(id);
                if (result)
                {
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(mask_password(result->toJSON()), ostr);
                    return;
                }
                else
                {
                    response_error(Poco::Net::HTTPResponse::HTTP_NOT_FOUND, 
                                    "/errors/not_found", 
                                    instance,
                                    "Internal exception", 
                                    "User not found", 
                                    response);
                    return;
                }
            }
            else if (hasSubstr(request.getURI(), "/user") && request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST)
            {
                if (!form.has("first_name") || !form.has("last_name") || !form.has("email") || !form.has("title") || !form.has("login") || !form.has("password"))
                {
                    response_error(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, 
                                    "/errors/bad_request", 
                                    instance,
                                    "Internal exception", 
                                    "Request doesn't have needed data", 
                                    response);
                    return;
                }
                database::User user;
                auto hasher = Poco::Hash<std::string>();
                user.first_name() = form.get("first_name");
                user.last_name() = form.get("last_name");
                user.email() = form.get("email");
                user.title() = form.get("title");
                user.login() = form.get("login");
                user.password() = hasher(form.get("password")) >> 1;

                bool check_result = true;
                std::string message;
                std::string reason;

                if (!check_name(user.get_first_name(), reason))
                {
                    check_result = false;
                    message += reason;
                    message += "<br>";
                }

                if (!check_name(user.get_last_name(), reason))
                {
                    check_result = false;
                    message += reason;
                    message += "<br>";
                }

                if (!check_email(user.get_email(), reason))
                {
                    check_result = false;
                    message += reason;
                    message += "<br>";
                }

                if (check_result)
                {
                    user.save_to_mysql();
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    std::ostream &ostr = response.send();
                    ostr << user.get_id();
                    return;
                }
                else
                {
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
                    std::ostream &ostr = response.send();
                    ostr << message;
                    response.send();
                    return;
                }
            }
        }
        catch (Poco::Exception &e) 
        {
            std::cout << e.message() << std::endl;
            response_error(Poco::Net::HTTPResponse::HTTP_FORBIDDEN, 
                            "/errors/forbidden", 
                            instance,
                            "Internal exception", 
                            "Check logs for info...", 
                            response);
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
#endif