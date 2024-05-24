#include "database.h"
#include "trip.h"

#include <Poco/Data/SessionFactory.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

#include <ostream>
#include <string>

using namespace Poco::Data::Keywords;

namespace database
{

    void Trip::init() {}

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

    Trip Trip::fromJSON(const std::string &str, bool is_mongo = true)
    {
        std::string str_to_parse = str;

        if (is_mongo) {
            int start = str.find("_id");
            int end = str.find(",",start);
            std::string s1 = str.substr(0,start-1);
            std::string s2 = str.substr(end+1);
            str_to_parse = s1+s2;
        }

        std::cout << str_to_parse << std::endl;

        Trip trip;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(str_to_parse);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

        trip.id()         = object->getValue<long>("id");
        trip.id_path()    = object->getValue<long>("id_path");
        trip.id_owner()   = object->getValue<long>("id_owner");
        trip.name()       = object->getValue<std::string>("name");
        trip.start_time() = object->getValue<std::string>("start_time");
        trip.fin_time()   = object->getValue<std::string>("fin_time");

        return trip;
    }

    std::optional<Trip> Trip::read_by_id(long id)
    {
        std::optional<Trip> result;
        std::map<std::string,long> params;
        params["id"] = id;
        
        std::vector<std::string> results = database::Database::get().get_from_mongo("trips",params);

        if(!results.empty()) {
            result = fromJSON(results[0]);
        }
        return result;
    }

    std::vector<Trip> Trip::read_by_id_owner(long id_owner)
    {
        std::vector<Trip> result;
        std::map<std::string,long> params;
        std::cout << "id_owner: " << id_owner << std::endl;
        params["id_owner"] = id_owner;

        std::vector<std::string> results = database::Database::get().get_from_mongo("trips",params);

        for(std::string& s : results) {
            std::cout << s << std::endl;
            result.push_back(fromJSON(s));
            std::cout << "Pushed!" << std::endl;
        }

        std::cout << "readed!";

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