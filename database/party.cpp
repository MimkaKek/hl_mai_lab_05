#include "party.h"
#include "database.h"
#include <Poco/Data/SessionFactory.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

using namespace Poco::Data::Keywords;

namespace database
{

    void Party::init() {}

    Poco::JSON::Object::Ptr Party::toJSON() const
    {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("id_trip", _id_trip);
        root->set("id_participant", _id_participant);
        return root;
    }

    Party Party::fromJSON(const std::string &str, bool is_mongo = true)
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

        Party party;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(str_to_parse);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

        party.id_trip()        = object->getValue<long>("id_trip");
        party.id_participant() = object->getValue<long>("id_participant");

        return party;
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

    long Party::get_id_trip() const
    {
        return _id_trip;
    }
    
    long Party::get_id_participant() const
    {
        return _id_participant;
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