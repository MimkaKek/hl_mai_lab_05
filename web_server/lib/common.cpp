#include "common.h"
#include <cstddef>
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
    Poco::URI uri("http://" + Config::get().get_user_service_host() + ":" + Config::get().get_user_service_port() + "/user/auth_check");
    Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());

    std::string path(uri.getPathAndQuery());
    Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_GET, path, Poco::Net::HTTPMessage::HTTP_1_1);
    req.setCredentials("Bearer", jwt_token);
    session.sendRequest(req);
    Poco::Net::HTTPResponse res;
    std::istream &is = session.receiveResponse(res);
    if (res.getStatus() != Poco::Net::HTTPResponse::HTTP_OK) {
        return false;
    }

    Poco::JSON::Parser parser;
    Poco::Dynamic::Var result = parser.parse(is);
    Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
    id = object->getValue<long>("id");
    login = object->getValue<std::string>("login");

    return true;
}

bool init_party_remote(std::string &jwt_token, long id_trip) {
    if (jwt_token.length() == 0) {
        return false;
    }
    Poco::URI uri("http://" + Config::get().get_party_service_host() + ":" + Config::get().get_party_service_port() + "/party?id_trip=" + std::to_string(id_trip));
    Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());
    std::string path(uri.getPathAndQuery());
    Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_POST, path, Poco::Net::HTTPMessage::HTTP_1_1);
    req.setCredentials("Bearer", jwt_token);
    session.sendRequest(req);
    Poco::Net::HTTPResponse res;
    session.receiveResponse(res);
    if (res.getStatus() != Poco::Net::HTTPResponse::HTTP_OK) {
        return false;
    }
    return true;
}

Poco::JSON::Array::Ptr get_parties(std::string &jwt_token) {
    if (jwt_token.length() == 0) {
        return nullptr;
    }
    Poco::URI uri("http://" + Config::get().get_party_service_host() + ":" + Config::get().get_party_service_port() + "/parties");
    Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());
    std::string path(uri.getPathAndQuery());
    Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_GET, path, Poco::Net::HTTPMessage::HTTP_1_1);
    req.setCredentials("Bearer", jwt_token);
    session.sendRequest(req);
    Poco::Net::HTTPResponse res;
    std::istream &response_data = session.receiveResponse(res);
    if (res.getStatus() != Poco::Net::HTTPResponse::HTTP_OK) {
        return nullptr;
    }
    Poco::JSON::Parser parser;
    Poco::Dynamic::Var result = parser.parse(response_data);
    return result.extract<Poco::JSON::Array::Ptr>();
}

bool is_in_party(std::string &jwt_token, long &id_trip) {
    if (jwt_token.length() == 0) {
        return false;
    }
    Poco::URI uri("http://" + Config::get().get_party_service_host() + ":" + Config::get().get_party_service_port() + "/party?id_trip=" + std::to_string(id_trip));
    Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());
    std::string path(uri.getPathAndQuery());
    Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_GET, path, Poco::Net::HTTPMessage::HTTP_1_1);
    req.setCredentials("Bearer", jwt_token);
    session.sendRequest(req);
    Poco::Net::HTTPResponse res;
    session.receiveResponse(res);
    return (res.getStatus() == Poco::Net::HTTPResponse::HTTP_OK);

}

void response_error(Poco::Net::HTTPResponse::HTTPStatus code, 
                    std::string type, 
                    std::string instance, 
                    std::string title, 
                    std::string detail, 
                    HTTPServerResponse &response)
{
    response.setStatus(code);
    response.setChunkedTransferEncoding(true);
    response.setContentType("application/json");
    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
    root->set("type", type);
    root->set("title", title);
    root->set("status", code);
    root->set("detail", detail);
    root->set("instance", instance);
    std::ostream &ostr = response.send();
    Poco::JSON::Stringifier::stringify(root, ostr);
}