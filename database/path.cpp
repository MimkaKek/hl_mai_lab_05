#include "path.h"
#include "database.h"

#include <Poco/Data/SessionFactory.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

using namespace Poco::Data::Keywords;
using Poco::Data::Statement;

namespace database
{

    void Path::init()
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement create_stmt(session);
            create_stmt << "CREATE TABLE IF NOT EXISTS paths (id SERIAL,"
                        << "startpoint VARCHAR(256) NOT NULL,"
                        << "endpoint VARCHAR(256) NOT NULL);", now;

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

    Poco::JSON::Object::Ptr Path::toJSON() const
    {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("id", _id);
        root->set("startpoint", _startpoint);
        root->set("endpoint", _endpoint);
        return root;
    }

    Path Path::fromJSON(const std::string &str)
    {
        Path path;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(str);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

        path.id() = object->getValue<long>("id");
        path.startpoint() = object->getValue<std::string>("startpoint");
        path.endpoint() = object->getValue<std::string>("endpoint");

        return path;
    }

    std::optional<Path> Path::read_by_id(long id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement select(session);
            Path a;
            select << "SELECT id, startpoint, endpoint FROM paths where id=$1",
                into(a._id),
                into(a._startpoint),
                into(a._endpoint),
                use(id),
                range(0, 1);

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

    std::vector<Path> Path::read_all()
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            std::vector<Path> result;
            Path a;
            select << "SELECT id, startpoint, endpoint FROM paths",
                into(a._id),
                into(a._startpoint),
                into(a._endpoint),
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

    std::vector<Path> Path::search(std::string startpoint)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            std::vector<Path> result;
            Path a;
            startpoint += "%";
            select << "SELECT id, startpoint, endpoint FROM paths where startpoint LIKE $1",
                into(a._id),
                into(a._startpoint),
                into(a._endpoint),
                use(startpoint),
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

    void Path::save_to_db()
    {

        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement insert(session);

            insert << "INSERT INTO paths (startpoint,endpoint) VALUES($1, $2)",
                use(_startpoint),
                use(_endpoint);

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

    long Path::get_id() const
    {
        return _id;
    }

    const std::string &Path::get_startpoint() const
    {
        return _startpoint;
    }

    const std::string &Path::get_endpoint() const
    {
        return _endpoint;
    }

    long &Path::id()
    {
        return _id;
    }

    std::string &Path::startpoint()
    {
        return _startpoint;
    }

    std::string &Path::endpoint()
    {
        return _endpoint;
    }
}