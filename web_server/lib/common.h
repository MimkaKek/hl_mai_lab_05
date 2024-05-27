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

void response_error(const Poco::Net::HTTPResponse::HTTPStatus &code, 
                    const std::string &type, 
                    const std::string &instance, 
                    const std::string &title, 
                    const std::string &detail, 
                    HTTPServerResponse &response);
                    
void send_response(Poco::Net::HTTPServerResponse& response, 
                   Poco::Net::HTTPResponse::HTTPStatus status, 
                   std::string& body_response);

void send_request(const std::string &method,
                  const std::string &uri_path, 
                  const std::string &auth_type, 
                  const std::string &auth_value, 
                  const std::string &body,
                  std::string &body_response,
                  Poco::Net::HTTPResponse::HTTPStatus &status);

const std::string AUTH_BEARER = "Bearer";
const std::string AUTH_BASIC  = "Basic";
#endif