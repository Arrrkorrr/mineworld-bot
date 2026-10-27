#include "database.hpp"

#include "../logs/logs.hpp"

#include <mysql/mysql.h>
#include <string>

///////////////////////////////////////////////////
//////////////////// Functions ////////////////////
///////////////////////////////////////////////////

/*
    Connect to a database.

    Tasks:
        1) Initialize MySQL.
        2) Connect to the database with the credentials provided.

    Parameters (variable_name / type / description):
        - database      / MYSQL* / Object that will point to the database once connected.
        - database_name / string / Name of the database to use.
        - database_host / string / IP address or domain name of the database.
        - database_port / int    / Port of the database.
        - user          / string / User to use in order to connect to the database.
        - password      / string / Password of the user.

    Returns (type + description):
        No object returned.
*/
void Database::connect_database
(
    MYSQL*            &database,
    const std::string &database_name,
    const std::string &database_host,
    const int         &database_port,
    const std::string &user,
    const std::string &password
)
{
    ////////////////// 1) //////////////////
    Logs::log("Connecting to the database..");
    database = mysql_init(nullptr);

    if (database == nullptr)
        Logs::crash_log("Failed to initialize mysql for database connection!");

    const bool reconnect = true;
    mysql_options(database, MYSQL_OPT_RECONNECT, &reconnect);

    ////////////////// 2) //////////////////
    const MYSQL* connection = mysql_real_connect(database, database_host.c_str(), user.c_str(), password.c_str(), database_name.c_str(), database_port, nullptr, 0);

    if (connection == nullptr)
    {
        const std::string error = std::string(mysql_error(database));

        mysql_close(database);
        Logs::crash_log("Failed to establish connection with the database. Error code -> " + error + ".");
    }

    Logs::log("Connection to database " + database_name + "@" + database_host + " as " + user + " established.");
}



/*
    Disconnect from a database.

    Tasks:
        1) Verify that the database is valid.
        2) Disconnect from the database.

    Parameters (variable_name / type / description):
        - database / MYSQL* / Database to disconnect from.

    Returns (type + description):
        No object returned.
*/
void Database::disconnect_database
(
    MYSQL* &database
)
{
    ////////////////// 1) //////////////////
    if (database == nullptr)
    {
        Logs::log("Warning: Failed to disconnect from database! The database provided is not valid.");
        return;
    }

    ////////////////// 2) //////////////////
    Logs::log("Disconnecting from database " + std::string(database -> db) + "@" + std::string(database -> host) + "..");

    mysql_close(database);
    database = nullptr;

    Logs::log("Disconnected from the database.");
}

/////////////////////////////////////////////////
//////////////////// Classes ////////////////////
/////////////////////////////////////////////////

Database::database_handler::database_handler
(
    MYSQL*            &database,
    const std::string &database_name,
    const std::string &database_host,
    const int         &database_port,
    const std::string &user,
    const std::string &password
)
    : database(database)
{
    connect_database(database, database_name, database_host, database_port, user, password);
}

Database::database_handler::~database_handler()
{
    disconnect_database(database);
}
