#include "text.hpp"

#include <algorithm>
#include <cctype>
#include <string>


/*
    Convert some text to lower cases only.

   Tasks:
       1) Replace each character to its lower case version one by one.

   Parameters (variable_name / type / description):
       - input / string / Text to format.

   Returns (type + description):
       A string containing the formatted text.
*/
std::string Text::to_lowercase
(
    const std::string &input
)
{
    std::string output = input;

    ////////////////// 1) //////////////////
    std::transform(output.begin(), output.end(), output.begin(), [](unsigned char character)
    {
        return std::tolower(character);
    });

    return output;
}
