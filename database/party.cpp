#include "party.h"
#include "database.h"
#include "../config/config.h"

#include <Poco/Data/SessionFactory.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

#include <sstream>
#include <exception>

using namespace Poco::Data::Keywords;

namespace database
{

    void Party::init() {}

    Poco::JSON::Object::Ptr Party::toJSON() const
    {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("id", _id);
        root->set("id_trip", _id);
        root->set("id_participant", _id);
        return root;
    }

    Party Party::fromJSON(const std::string &str)
    {
        Party party;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(str);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

        party.id()             = object->getValue<long>("id");
        party.id_trip()        = object->getValue<long>("id_trip");
        party.id_participant() = object->getValue<long>("id_participant");

        return party;
    }

    std::optional<Party> Party::read_by_id(long id)
    {
        std::optional<Party> result;
        std::map<std::string, long> params;
        params["id"] = id;
        std::vector<std::string> results = database::Database::get().get_from_mongo("parties",params);

        if(!results.empty())
            result = fromJSON(results[0]);
        
        return result;
    }

    std::vector<Party> Party::read_by_id_part(long id_part)
    {
        std::vector<Party> result;
        std::map<std::string,long> params;
        params["id_participant"] = id_part;

        std::vector<std::string> results = database::Database::get().get_from_mongo("parties",params);

        for(std::string& s : results) 
            result.push_back(fromJSON(s));

        return result;
    }

    std::vector<Party> Party::read_by_id_trip(long id_trip)
    {
        std::vector<Party> result;
        std::map<std::string,long> params;
        params["id_trip"] = id_trip;

        std::vector<std::string> results = database::Database::get().get_from_mongo("parties",params);

        for(std::string& s : results) 
            result.push_back(fromJSON(s));

        return result;
    }

    void Party::add()
    {
        database::Database::get().send_to_mongo("parties",toJSON());
    }

    void Party::update()
    {
        std::map<std::string,long> params;
        params["id"] = _id;
        database::Database::get().update_mongo("parties",params,toJSON());
    }

    long Party::get_id() const
    {
        return _id;
    }

    long Party::get_id_trip() const
    {
        return _id_trip;
    }
    
    long Party::get_id_participant() const
    {
        return _id_participant;
    }


    long &Party::id()
    {
        return _id;
    }

    long &Party::id_participant()
    {
        return _id_participant;
    }

    long &Party::id_trip()
    {
        return _id_trip;
    }
}