#ifndef COMMON_H
#define COMMON_H

#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/NumericString.h"
#include "Poco/Base64Decoder.h"
#include "Poco/JWT/Token.h"
#include "Poco/JWT/Signer.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/URI.h"
#include "../../config/config.h"
#include <istream>
#include <ostream>
#include <fstream>
#include <iostream>
#include <string>

using namespace Poco::Net;

bool hasSubstr(const std::string &str, const std::string &substr);
Poco::JSON::Object::Ptr auth_user(HTTPServerRequest &request, bool is_remote = true);
bool get_identity(const std::string identity, std::string &login, std::string &password);
std::string getJWTKey();
std::string generate_token(long &id, std::string &login);
bool extract_payload_local(std::string &jwt_token, long &id, std::string &login);
bool extract_payload_remote(std::string &jwt_token, long &id, std::string &login);
bool init_party_remote(std::string &jwt_token, long id_trip);
Poco::JSON::Array::Ptr get_parties(std::string &jwt_token);
bool is_in_party(std::string &jwt_token, long &id_trip);

void response_error(Poco::Net::HTTPResponse::HTTPStatus code, 
                    std::string type, 
                    std::string instance, 
                    std::string title, 
                    std::string detail, 
                    HTTPServerResponse &response);
#endif