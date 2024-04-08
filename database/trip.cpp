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
using Poco::Data::Session;
using Poco::Data::Statement;

namespace database
{

    void Trip::init()
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement create_stmt(session);
            create_stmt << "CREATE TABLE IF NOT EXISTS trips (id SERIAL,"
                        << "id_path SERIAL NOT NULL REFERENCES paths(id),"
                        << "id_owner SERIAL NOT NULL REFERENCES users(id),"
                        << "name VARCHAR(256) NOT NULL,"
                        << "start_time VARCHAR(256) NOT NULL,"
                        << "fin_time VARCHAR(256) NOT NULL);", now;
        }

        catch (Poco::Data::PostgreSQL::PostgreSQLException &e)
        {
            std::cout << "connection:" << e.displayText() << std::endl;
            throw;
        }
        catch (Poco::Data::ConnectionFailedException &e)
        {
            std::cout << "connection:" << e.displayText() << std::endl;
            throw;
        }
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

        trip.id() = object->getValue<long>("id");
        trip.id_path() = object->getValue<long>("id_path");
        trip.id_owner() = object->getValue<long>("id_owner");
        trip.name() = object->getValue<std::string>("name");
        trip.start_time() = object->getValue<std::string>("start_time");
        trip.fin_time() = object->getValue<std::string>("fin_namer");

        return trip;
    }

    std::optional<Trip> Trip::read_by_id(long id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement select(session);
            Trip a;
            select << "SELECT id, id_path, id_owner, name, start_time, fin_time FROM trips where id=$1",
                into(a._id),
                into(a._id_path),
                into(a._id_owner),
                into(a._name),
                into(a._start_time),
                into(a._fin_time),
                use(id),
                range(0, 1); //  iterate over result set one row at a time

            select.execute();
            Poco::Data::RecordSet rs(select);
            if (rs.moveFirst())
                return a;
        }

        catch (Poco::Data::PostgreSQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
        }
        catch (Poco::Data::PostgreSQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
        }
        return {};
    }

    std::vector<Trip> Trip::read_all()
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            std::vector<Trip> result;
            Trip a;
            select << "SELECT id, id_path, id_owner, name, start_time, fin_time FROM trips",
                into(a._id),
                into(a._id_path),
                into(a._id_owner),
                into(a._name),
                into(a._start_time),
                into(a._fin_time),
                range(0, 1); //  iterate over result set one row at a time

            while (!select.done())
            {
                if (select.execute())
                    result.push_back(a);
            }
            return result;
        }

        catch (Poco::Data::PostgreSQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::PostgreSQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    std::vector<Trip> Trip::search(std::string name)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            std::vector<Trip> result;
            Trip a;
            name += "%";
            select << "SELECT id, id_path, id_owner, name, start_time, fin_time FROM trips where name LIKE $1",
                into(a._id),
                into(a._id_path),
                into(a._id_owner),
                into(a._name),
                into(a._start_time),
                into(a._fin_time),
                use(name),
                range(0, 1); //  iterate over result set one row at a time

            while (!select.done())
            {
                if (select.execute())
                    result.push_back(a);
            }
            return result;
        }

        catch (Poco::Data::PostgreSQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::PostgreSQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    void Trip::save_to_db()
    {

        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement insert(session);

            insert << "INSERT INTO trips (id_path, id_owner, name, start_time, fin_time) VALUES($1, $2, $3, $4, $5)",
                use(_id_path),
                use(_id_owner),
                use(_name),
                use(_start_time),
                use(_fin_time);

            insert.execute();

            Poco::Data::Statement select(session);
            select << "SELECT LASTVAL()",
                into(_id),
                range(0, 1); //  iterate over result set one row at a time

            if (!select.done())
            {
                select.execute();
            }
            std::cout << "inserted:" << _id << std::endl;
        }
        catch (Poco::Data::PostgreSQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::PostgreSQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
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