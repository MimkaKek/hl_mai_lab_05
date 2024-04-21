#include "database.h"
#include "../config/config.h"
#include "trip.h"

#include <Poco/Data/SessionFactory.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

#include <sstream>
#include <exception>
#include <string>

using namespace Poco::Data::Keywords;

namespace database
{

    void Trip::init()
    {
        // TODO
    }

    Poco::JSON::Object::Ptr Trip::toJSON() const
    {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("id", _id);
        root->set("id_path", _id_path);
        root->set("id_owner", _id_owner);
        root->set("name", _name);
        root->set("start_time", _start_time);
        root->set("fin_time", _fin_time);
        return root;
    }

    Trip Trip::fromJSON(const std::string &str)
    {
        Trip trip;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(str);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
    
        trip.id()         = object->getValue<long>("id");
        trip.id_path()    = object->getValue<long>("id_path");
        trip.id_owner()   = object->getValue<long>("id_owner");
        trip.name()       = object->getValue<std::string>("name");
        trip.start_time() = object->getValue<std::string>("start_time");
        trip.fin_time()   = object->getValue<std::string>("fin_namer");

        return trip;
    }

    std::optional<Trip> Trip::read_by_id(long id)
    {
        std::optional<Trip> result;
        std::map<std::string,long> params;
        params["id"] = id;
        std::cout << "Before get" << std::endl;
        std::vector<std::string> results = database::Database::get().get_from_mongo("trips",params);
        std::cout << "After get" << std::endl;

        std::cout << "Before empty()" << std::endl;
        if(!results.empty()) {
            std::cout << "Before fromJSON()" << std::endl;
            std::cout << results[0] << std::endl;
            result = fromJSON(results[0]);
            std::cout << "After fromJSON()" << std::endl;
        }
        std::cout << "Before return result" << std::endl;
        return result;
    }

    std::vector<Trip> Trip::read_by_id_owner(long id_owner)
    {
        std::vector<Trip> result;
        std::map<std::string,long> params;
        params["id_owner"] = id_owner;

        std::vector<std::string> results = database::Database::get().get_from_mongo("trips",params);

        for(std::string& s : results) 
            result.push_back(fromJSON(s));

        return result;
    }

    void Trip::add()
    {
        database::Database::get().send_to_mongo("trips",toJSON());
    }

    void Trip::update()
    {
        std::map<std::string,long> params;
        params["id"] = _id;
        database::Database::get().update_mongo("trips",params,toJSON());
    }

    long Trip::get_id() const
    {
        return _id;
    }

    long Trip::get_id_path() const
    {
        return _id_path;
    }
    
    long Trip::get_id_owner() const
    {
        return _id_owner;
    }

    const std::string &Trip::get_name() const
    {
        return _name;
    }

    const std::string &Trip::get_start_time() const
    {
        return _start_time;
    }

    const std::string &Trip::get_fin_time() const
    {
        return _fin_time;
    }

    long &Trip::id()
    {
        return _id;
    }

    long &Trip::id_path()
    {
        return _id_path;
    }

    long &Trip::id_owner()
    {
        return _id_owner;
    }

    std::string &Trip::name()
    {
        return _name;
    }

    std::string &Trip::start_time()
    {
        return _start_time;
    }

    std::string &Trip::fin_time()
    {
        return _fin_time;
    }
}