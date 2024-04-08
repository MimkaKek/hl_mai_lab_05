#ifndef PARTY_H
#define PARTY_H

#include <string>
#include <vector>
#include "Poco/JSON/Object.h"
#include <optional>

namespace database
{
    class Party {
        private:
            long        _id;
            long        _id_trip;
            long        _id_participant;

        public:
            long               get_id() const;
            long               get_id_trip() const;
            long               get_id_participant() const;

            long&        id();
            long&        id_trip();
            long&        id_participant();

            static void init();
            static std::optional<Party> read_by_id(long id);
            static std::vector<Party> read_all();
            static std::vector<Party> search(long id_part);
            void save_to_db();

            Poco::JSON::Object::Ptr toJSON() const;
            static Party fromJSON(const std::string& str);

    };
}

#endif