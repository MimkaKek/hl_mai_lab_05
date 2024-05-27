#include "common.h"
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StreamCopier.h>
#include <cstddef>
#include <iostream>
#include <sstream>
#include <string>


bool hasSubstr(const std::string &str, const std::string &substr)
{
    if (str.size() < substr.size())
        return false;
    for (size_t i = 0; i <= str.size() - substr.size(); ++i)
    {
        bool ok{true};
        for (size_t j = 0; ok && (j < substr.size()); ++j)
            ok = (str[i + j] == substr[j]);
        if (ok)
            return true;
    }
    return false;
}

Poco::JSON::Object::Ptr auth_user(HTTPServerRequest &request, bool is_remote) {
    std::string scheme;
    std::string info;
    long id_user {-1};
    std::string login;
    request.getCredentials(scheme, info);
    std::cout << "scheme: " << scheme << " identity: " << info << std::endl;

    bool extract_status = is_remote ? extract_payload_remote(info,id_user,login) : extract_payload_local(info,id_user,login);

    if(!extract_status) {
        Poco::JSON::Object::Ptr ret = new Poco::JSON::Object();
        ret->set("status", false);
        return ret;
    }
    std::cout << "id:" << id_user << " login:" << login << std::endl;
    Poco::JSON::Object::Ptr ret = new Poco::JSON::Object();
    ret->set("status", true);
    ret->set("id_user", id_user);
    ret->set("login", login);
    ret->set("authinfo", info);
    return ret;
}

bool get_identity(const std::string identity, std::string &login, std::string &password)
{
    std::istringstream istr(identity);
    std::ostringstream ostr;
    Poco::Base64Decoder b64in(istr);
    copy(std::istreambuf_iterator<char>(b64in),
         std::istreambuf_iterator<char>(),
         std::ostreambuf_iterator<char>(ostr));
    std::string decoded = ostr.str();

    size_t pos = decoded.find(':');
    login = decoded.substr(0, pos);
    password = decoded.substr(pos + 1);
    return true;
}

std::string getJWTKey() {
    if (std::getenv("JWT_KEY") != nullptr) {
        return std::getenv("JWT_KEY");
    }
    return "0123456789ABCDEF0123456789ABCDEF";
}

std::string generate_token(long &id, std::string &login) {
    Poco::JWT::Token token;
    token.setType("JWT");
    token.setSubject("login");
    token.payload().set("login", login);
    token.payload().set("id", id);
    token.setIssuedAt(Poco::Timestamp());

    Poco::JWT::Signer signer(getJWTKey());
    return signer.sign(token, Poco::JWT::Signer::ALGO_HS256);
}

bool extract_payload_local(std::string &jwt_token, long &id, std::string &login) {
    if (jwt_token.length() == 0) {
        return false;
    }

    Poco::JWT::Signer signer(getJWTKey());
    try {
        Poco::JWT::Token token = signer.verify(jwt_token);
        if (token.payload().has("login") && token.payload().has("id")) {
            login = token.payload().getValue<std::string>("login");
            id = token.payload().getValue<long>("id");
            return true;
        }
        std::cout << "Not enough fields in token" << std::endl;

    } catch (...) {
        std::cout << "Token verification failed" << std::endl;
    }
    return false;
}

bool extract_payload_remote(std::string &jwt_token, long &id, std::string &login) {
    if (jwt_token.length() == 0) {
        return false;
    }

    std::string url_path        = "http://" + Config::get().get_user_service_host() + ":" + Config::get().get_user_service_port();
    std::string urn_path        = "/auth_check";
    std::string auth_type       = AUTH_BEARER;
    std::string uri_path        = url_path + urn_path;
    std::string body            = "";
    std::string body_response   = "";
    Poco::Net::HTTPResponse::HTTPStatus status;
    
    send_request(Poco::Net::HTTPRequest::HTTP_GET, uri_path, auth_type, jwt_token, body, body_response, status);
    if (status != Poco::Net::HTTPResponse::HTTP_OK) {
        return false;
    }

    Poco::JSON::Parser parser;
    Poco::Dynamic::Var result = parser.parse(body_response);
    Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
    id    = object->getValue<long>("id");
    login = object->getValue<std::string>("login");

    return true;
}

bool init_party_remote(std::string &jwt_token, long id_trip) {
    if (jwt_token.length() == 0) {
        return false;
    }
    std::string url_path        = "http://" + Config::get().get_party_service_host() + ":" + Config::get().get_party_service_port();
    std::string urn_path        = "/party?id_trip=" + std::to_string(id_trip);
    std::string uri_path        = url_path + urn_path;
    std::string auth_type       = AUTH_BEARER;
    std::string body            = "";
    std::string body_response   = "";
    Poco::Net::HTTPResponse::HTTPStatus status;
    send_request(Poco::Net::HTTPRequest::HTTP_POST, uri_path, auth_type, jwt_token, body, body_response, status);
    return (status == Poco::Net::HTTPResponse::HTTP_OK);
}

Poco::JSON::Array::Ptr get_parties(std::string &jwt_token) {
    if (jwt_token.length() == 0) {
        return nullptr;
    }
    std::string url_path        = "http://" + Config::get().get_party_service_host() + ":" + Config::get().get_party_service_port();
    std::string urn_path        = "/parties";
    std::string uri_path        = url_path + urn_path;
    std::string auth_type       = AUTH_BEARER;
    std::string body            = "";
    std::string body_response   = "";
    Poco::Net::HTTPResponse::HTTPStatus status;
    send_request(Poco::Net::HTTPRequest::HTTP_GET, uri_path, auth_type, jwt_token, body, body_response, status);
    if (status != Poco::Net::HTTPResponse::HTTP_OK) {
        return nullptr;
    }
    Poco::JSON::Parser parser;
    Poco::Dynamic::Var result = parser.parse(body_response);
    return result.extract<Poco::JSON::Array::Ptr>();
}

bool is_in_party(std::string &jwt_token, long &id_trip) {
    if (jwt_token.length() == 0) {
        return false;
    }
    std::string url_path        = "http://" + Config::get().get_party_service_host() + ":" + Config::get().get_party_service_port();
    std::string urn_path        = "/party?id_trip=" + std::to_string(id_trip);
    std::string uri_path        = url_path + urn_path;
    std::string auth_type       = AUTH_BEARER;
    std::string body            = "";
    std::string body_response   = "";
    Poco::Net::HTTPResponse::HTTPStatus status;
    send_request(Poco::Net::HTTPRequest::HTTP_GET, uri_path, auth_type, jwt_token, body, body_response, status);
    return (status == Poco::Net::HTTPResponse::HTTP_OK);
}

void send_response(Poco::Net::HTTPServerResponse& response, Poco::Net::HTTPResponse::HTTPStatus status, std::string& body_response) {
    response.setStatus(status);
    response.setChunkedTransferEncoding(true);
    response.setContentType("application/json");
    std::ostream &ostr = response.send();
    ostr << body_response;
    ostr.flush();
}

void response_error(const Poco::Net::HTTPResponse::HTTPStatus &status, 
                    const std::string &type, 
                    const std::string &instance, 
                    const std::string &title, 
                    const std::string &detail, 
                    HTTPServerResponse &response)
{
    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
    root->set("type", type);
    root->set("title", title);
    root->set("status", std::to_string(status));
    root->set("detail", detail);
    root->set("instance", instance);
    std::stringstream ss;
    Poco::JSON::Stringifier::stringify(root, ss);
    std::string body_response = ss.str();
    send_response(response, status, body_response);
}



void send_request(const std::string &method, 
                  const std::string &uri_path,
                  const std::string &auth_type, 
                  const std::string &auth_value, 
                  const std::string &body,
                  std::string &body_response,
                  Poco::Net::HTTPResponse::HTTPStatus &status)
{
    try
    {
        std::cout << "Making request ("<< method << "): " << uri_path << std::endl;
        std::cout << "Authorization: '" << auth_type << "' '" << auth_value << "'" << std::endl;
        std::cout << "Request body:\n" << body << std::endl;

        Poco::URI uri(uri_path);
        std::string path(uri.getPathAndQuery());
        Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());
        Poco::Net::HTTPRequest request(method, path, Poco::Net::HTTPMessage::HTTP_1_1);
        
        request.setContentType("application/json");
        request.setContentLength(body.length());
        if (!auth_type.empty() && !auth_value.empty())
        {
            request.set("Authorization", auth_type + " " + auth_value);
        }
        session.sendRequest(request) << body;

        Poco::Net::HTTPResponse response;
        std::istream &rs = session.receiveResponse(response);
        status = response.getStatus();
        std::stringstream ss;
        Poco::StreamCopier::copyStream(rs, ss);
        body_response = ss.str();
        std::cout << "Response Status: " << status << std::endl;
        std::cout << "Response Body:\n" << body_response << std::endl;
        return;
    }
    catch (Poco::Exception &ex)
    {
        std::cerr << "Send Request Exception: " << ex.displayText() << std::endl;
        std::cerr << "Message: " << ex.message() << std::endl;
        status = Poco::Net::HTTPResponse::HTTP_FORBIDDEN;
        return;
    }
    status = Poco::Net::HTTPResponse::HTTP_FORBIDDEN;
    return;
}