#include "text.hpp"

#include <string>

/*
    Remove starting and trailing whitespaces.

    Tasks:
        1) Detect position of the first character that is not a whitespace from beginning.
        2) Detect position of the first character that is not a whitespace, from ending.
        3) Set output as starting and ending from positions retrieved previously.

    Parameters (variable_name / type / description):
        - input / string / Text to trim.

    Returns (type + description):
        A string containing the trimmed text.
*/
std::string Text::trim
(
    const std::string &input
)
{
    ////////////////// 1) //////////////////
    const size_t real_start = input.find_first_not_of(" \t\r\n");

    if (real_start == std::string::npos)
        return "";

    ////////////////// 2) //////////////////
    const size_t real_end = input.find_last_not_of(" \t\r\n");

    ////////////////// 3) //////////////////
    const std::string output = input.substr(real_start, real_end - real_start + 1);
    return output;
}
