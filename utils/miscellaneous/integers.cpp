#include "miscellaneous.hpp"

#include <regex>
#include <string>

/*
    Check if an input is an integer.

    Tasks:
        1) Create a pattern that detects any non-numeric characters.
        2) Apply pattern to input.

    Parameters (variable_name / type / description):
        - input / string / Input to check.

    Returns (type + description):
        A boolean confirming whether the input is an integer or not.
*/
bool Miscellaneous::is_an_integer
(
    const std::string &input
)
{
    ////////////////// 1) //////////////////
    static const std::regex pattern(R"(^-?\d+$)");

    ////////////////// 2) //////////////////
    const bool is_an_integer = std::regex_match(input, pattern);

    return is_an_integer;
}
