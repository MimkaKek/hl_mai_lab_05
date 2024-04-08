#ifndef TRIP_H
#define TRIP_H

#include <string>
#include <vector>
#include "Poco/JSON/Object.h"
#include <optional>

namespace database
{
    class Trip {
        private:
            long        _id;
            long        _id_path;
            long        _id_owner;

            std::string _name;
            std::string _start_time;
            std::string _fin_time;

        public:
            long get_id() const;
            long get_id_path() const;
            long get_id_owner() const;
            const std::string& get_name() const;
            const std::string& get_start_time() const;
            const std::string& get_fin_time() const;

            long&        id();
            long&        id_path();
            long&        id_owner();
            std::string& name();
            std::string& start_time();
            std::string& fin_time();

            static void init();
            static std::optional<Trip> read_by_id(long id);
            static std::vector<Trip> read_all();
            static std::vector<Trip> search(std::string begin);
            void save_to_db();

            Poco::JSON::Object::Ptr toJSON() const;
            static Trip fromJSON(const std::string& str);

    };
}

#endif