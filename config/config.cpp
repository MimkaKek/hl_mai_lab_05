#include "config.h"
#include <cstddef>

Config::Config()
{
        _host = std::getenv("DB_HOST") ? std::getenv("DB_HOST") : "";
        _port = std::getenv("DB_PORT") ? std::getenv("DB_PORT") : "";
        _login = std::getenv("DB_LOGIN") ? std::getenv("DB_LOGIN") : "";
        _password = std::getenv("DB_PASSWORD") ? std::getenv("DB_PASSWORD") : "";
        _database = std::getenv("DB_DATABASE") ? std::getenv("DB_DATABASE") : "";

        _mongo = std::getenv("MONGO_HOST") ? std::getenv("MONGO_HOST") : "";
        _mongo_port = std::getenv("MONGO_PORT") ? std::getenv("MONGO_PORT") : "";
        _mongo_database = std::getenv("MONGO_DATABASE") ? std::getenv("MONGO_DATABASE") : "";

        _user_service_host = std::getenv("USER_SERVICE_HOST") ? std::getenv("USER_SERVICE_HOST") : "";
        _user_service_port = std::getenv("USER_SERVICE_PORT") ? std::getenv("USER_SERVICE_PORT") : "";
        
        _party_service_host = std::getenv("PARTY_SERVICE_HOST") ? std::getenv("PARTY_SERVICE_HOST") : "";
        _party_service_port = std::getenv("PARTY_SERVICE_PORT") ? std::getenv("PARTY_SERVICE_PORT") : "";

        _path_service_host = std::getenv("PATH_SERVICE_HOST") ? std::getenv("PATH_SERVICE_HOST") : "";
        _path_service_port = std::getenv("PATH_SERVICE_PORT") ? std::getenv("PATH_SERVICE_PORT") : "";

        _trip_service_host = std::getenv("TRIP_SERVICE_HOST") ? std::getenv("TRIP_SERVICE_HOST") : "";
        _trip_service_port = std::getenv("TRIP_SERVICE_PORT") ? std::getenv("TRIP_SERVICE_PORT") : "";

        _redis_cache_host = std::getenv("REDIS_CACHE_HOST") ? std::getenv("REDIS_CACHE_HOST") : "";
        _redis_cache_port = std::getenv("REDIS_CACHE_PORT") ? std::getenv("REDIS_CACHE_PORT") : "";
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
    return _party_service_host;
}
const std::string &Config::get_party_service_port() const
{
    return _party_service_port;
}
const std::string &Config::get_path_service_host() const
{
    return _path_service_host;
}
const std::string &Config::get_path_service_port() const
{
    return _path_service_port;
}
const std::string &Config::get_trip_service_host() const
{
    return _trip_service_host;
}
const std::string &Config::get_trip_service_port() const
{
    return _trip_service_port;
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
    return _party_service_host;
}
std::string &Config::party_service_port()
{
    return _party_service_port;
}
std::string &Config::path_service_host()
{
    return _path_service_host;
}
std::string &Config::path_service_port()
{
    return _path_service_port;
}
std::string &Config::trip_service_host()
{
    return _trip_service_host;
}
std::string &Config::trip_service_port()
{
    return _trip_service_port;
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

const std::string& Config::get_redis_cache_host() const
{
    return _redis_cache_host;
}

std::string& Config::redis_cache_host()
{
    return _redis_cache_host;
}

const std::string& Config::get_redis_cache_port() const
{
    return _redis_cache_port;
}

std::string& Config::redis_cache_port()
{
    return _redis_cache_port;
}