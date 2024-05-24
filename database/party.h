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
            long        _id_trip;
            long        _id_participant;

        public:
            long               get_id_trip() const;
            long               get_id_participant() const;

            long&              id_trip();
            long&              id_participant();

            static void init();
            static std::vector<Party> read_by_id_part(long id_part);
            static std::vector<Party> read_by_id_trip(long id_trip);
            void   add();
            void   update();

            Poco::JSON::Object::Ptr toJSON() const;
            static Party fromJSON(const std::string& str, bool is_mongo);

    };
}

#endif