#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <map>
#include <mysql/mysql.h>
#include <string>
#include <vector>

namespace Database
{
    using Output = std::vector<std::map<std::string, std::string>>;

    ///////////////////////////////////////////////////
    //////////////////// Functions ////////////////////
    ///////////////////////////////////////////////////

    //////////////////////////////////
    ///// connection.handler.cpp /////
    //////////////////////////////////

    void connect_database
    (
        MYSQL*            &database,
        const std::string &database_name,
        const std::string &database_host,
        const int         &database_port,
        const std::string &user,
        const std::string &password
    );

    void disconnect_database
    (
        MYSQL* &database_obj
    );

    ///////////////////////////////
    ///// queries.handler.cpp /////
    ///////////////////////////////

    Output db_query
    (
        MYSQL*            &database,
        const std::string &query
    );

    ///////////////////////////////
    ///// query.sanitizer.cpp /////
    ///////////////////////////////

    std::string sanitize_input
    (
        MYSQL*            &database,
        const std::string &input
    );

    /////////////////////////////////////////////////
    //////////////////// Classes ////////////////////
    /////////////////////////////////////////////////

    //////////////////////////////////
    ///// connection.handler.cpp /////
    //////////////////////////////////

    class database_handler
    {
        public:
            database_handler
            (
                MYSQL*            &database,
                const std::string &database_name,
                const std::string &database_host,
                const int         &database_port,
                const std::string &user,
                const std::string &password
            );

            ~database_handler();

            // Prevent data duplication.
            database_handler(const database_handler&) = delete;
            database_handler &operator = (const database_handler&) = delete;
        private:
            MYSQL* database;
    };
}

#endif
