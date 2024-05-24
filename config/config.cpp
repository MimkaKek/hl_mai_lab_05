#include "config.h"

Config::Config()
{
        _host = std::getenv("DB_HOST");
        _port = std::getenv("DB_PORT");
        _login = std::getenv("DB_LOGIN");
        _password = std::getenv("DB_PASSWORD");
        _database = std::getenv("DB_DATABASE");
        _mongo = std::getenv("MONGO_HOST");
        _mongo_port = std::getenv("MONGO_PORT");
        _mongo_database = std::getenv("MONGO_DATABASE");
        _user_service_host = std::getenv("USER_SERVICE_HOST");
        _user_service_port = std::getenv("USER_SERVICE_PORT");
        _party_service_host = std::getenv("PARTY_SERVICE_HOST");
        _party_service_port = std::getenv("PARTY_SERVICE_PORT");
}

Config &Config::get()
{
    static Config _instance;
    return _instance;
}

const std::string &Config::get_port() const
{
    return _port;
}

const std::string &Config::get_host() const
{
    return _host;
}

const std::string &Config::get_login() const
{
    return _login;
}

const std::string &Config::get_password() const
{
    return _password;
}
const std::string &Config::get_database() const
{
    return _database;
}
const std::string &Config::get_user_service_host() const
{
    return _user_service_host;
}
const std::string &Config::get_user_service_port() const
{
    return _user_service_port;
}
const std::string &Config::get_party_service_host() const
{
    return _user_service_host;
}
const std::string &Config::get_party_service_port() const
{
    return _user_service_port;
}


std::string &Config::port()
{
    return _port;
}

std::string &Config::host()
{
    return _host;
}

std::string &Config::login()
{
    return _login;
}

std::string &Config::password()
{
    return _password;
}

std::string &Config::database()
{
    return _database;
}

std::string &Config::user_service_host()
{
    return _user_service_host;
}
std::string &Config::user_service_port()
{
    return _user_service_port;
}
std::string &Config::party_service_host()
{
    return _user_service_host;
}
std::string &Config::party_service_port()
{
    return _user_service_port;
}

std::string& Config::mongo(){
    return _mongo;
}
std::string& Config::mongo_port(){
    return _mongo_port;
}
std::string& Config::mongo_database(){
    return _mongo_database;
}

const std::string& Config::get_mongo() const{
    return _mongo;
}
const std::string& Config::get_mongo_port() const{
    return _mongo_port;
}
const std::string& Config::get_mongo_database() const{
    return _mongo_database;
}