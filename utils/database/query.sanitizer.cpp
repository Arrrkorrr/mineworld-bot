#include "database.hpp"

#include <mysql/mysql.h>
#include <string>

/*
    Sanitize an input to prevent SQL injection.

    Tasks:
        1) Set the output string as the input length * 2 + 1 as it is the maximum possible length.
        2) Use MySQL built-in fuction to sanitize and resize the output to the sanitized length.

    Parameters (variable_name / type / description):
        - database / MYSQL* / MineWorld database
        - input    / string / Data to sanitize.

    Returns (type + description):
        A string containing the sanitized input.
*/
std::string Database::sanitize_input
(
    MYSQL*            &database,
    const std::string &input
)
{
    ////////////////// 1) //////////////////
    std::string output;
    output.resize(input.length() * 2 + 1);

    ////////////////// 2) //////////////////
    const unsigned long length = mysql_real_escape_string(database, output.data(), input.c_str(), static_cast<unsigned long>(input.length()));
    output.resize(length);

    return output;
}
