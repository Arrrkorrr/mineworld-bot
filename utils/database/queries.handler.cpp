#include "database.hpp"

#include "../logs/logs.hpp"

#include <mysql/mysql.h>
#include <string>

/*
    Make a query to the database.

    Tasks:
        1) Verify that the database is valid.
        2) Make the query to the database.
        3) Process output data.
            a. Get query result.
            b. Register the column names of the table as map keys.
            c. Register the rows data and associate it to all map keys.
            d. Free allocated memory.

    Parameters (variable_name / type / description):
        - database / MYSQL* / Database to query.
        - query    / string / Query to send to the database.

    Returns (type + description):
        A vector list containing maps of strings that represents the output data from the query.
*/
Database::Output Database::db_query
(
    MYSQL*            &database,
    const std::string &query
)
{
    ////////////////// 1) //////////////////
    Database::Output output;

    if (database == nullptr)
    {
        Logs::log("Warning: Query \"" + query + "\" failed -> invalid database provided.");
        return output;
    }

    ////////////////// 2) //////////////////
    const int query_result = mysql_query(database, query.c_str());

    if (query_result != 0)
    {
        Logs::log("Warning: Query \"" + query + "\" failed with error code -> " + std::string(mysql_error(database)) + ".");
        return output;
    }

    ////////////////// 3) //////////////////
    ///////// a. /////////
    MYSQL_RES* result = mysql_store_result(database);

    if (result == nullptr)
    {
        if (mysql_field_count(database) > 0)
            Logs::log("Warning: Failed to get query result with error code -> " + std::string(mysql_error(database)) + ".");

        return output;
    }

    ///////// b. /////////
    int fields_count = mysql_num_fields(result);
    const MYSQL_FIELD* fields = mysql_fetch_fields(result);

    std::vector<std::string> column_names;

    for (int i = 0; i < fields_count; i++)
        column_names.push_back(fields[i].name);

    ///////// c. /////////
    MYSQL_ROW row;

    while ((row = mysql_fetch_row(result)) != nullptr)
    {
        std::map<std::string, std::string> row_data;

        for (int i = 0; i < fields_count; i++)
            row_data[column_names[i]] = row[i];

        output.push_back(row_data);
    }

    ///////// d. /////////
    mysql_free_result(result);
    return output;
}
