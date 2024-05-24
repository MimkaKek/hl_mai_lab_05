#ifndef PATH_HANDLER_H
#define PATH_HANDLER_H

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

#include "../../database/path.h"
#include "../lib/common.h"

class PathHandler : public HTTPRequestHandler
{
public:
    PathHandler(const std::string &format) : _format(format)
    {
    }

    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response)
    {
        HTMLForm form(request, request.stream());
        std::string instance = "/path";
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

            if (hasSubstr(request.getURI(), "/path/search") && (request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET))
            {
                if (!form.has("startpoint"))
                {
                    response_error(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, 
                                   "/errors/bad_request", 
                                   instance,
                                   "Internal exception", 
                                   "Request doesn't have nedeed data...", 
                                   response);
                    return;
                }
                std::string _startpoint = form.get("startpoint");
                auto results = database::Path::search(_startpoint);
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
            else if (hasSubstr(request.getURI(), "/path") && (request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET))
            {
                if (!form.has("id"))
                {
                    response_error(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, 
                                   "/errors/bad_request", 
                                   instance,
                                   "Internal exception", 
                                   "Request doesn't have nedeed data...", 
                                   response);
                    return;
                }
                long id = atol(form.get("id").c_str());

                std::optional<database::Path> result = database::Path::read_by_id(id);
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
                                   "Path doesn't exists...", 
                                   response);
                    return;
                }
            }
            else if (hasSubstr(request.getURI(), "/path") && (request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST))
            {
                if (!form.has("startpoint") || !form.has("endpoint"))
                {
                    response_error(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, 
                                   "/errors/bad_request", 
                                   instance,
                                   "Internal exception", 
                                   "Request doesn't have nedeed data...", 
                                   response);
                    return;
                }
                auto path = database::Path();
                path.startpoint() = form.get("startpoint");
                path.endpoint()   = form.get("endpoint");
                path.save_to_db();
                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();
                ostr << path.get_id();
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
#endif