#include "parsers.hpp"

#include "../logs/logs.hpp"
#include "../text/text.hpp"

#include <fstream>
#include <map>
#include <string>

/*
    Parse a config file.
    Warning: If 2+ lines in the config file declare the same key, the most recent value found will be returned.

    Tasks:
        1) Open config file.
        2) Proceed to parsing.
            a. Check every single line of config file.
            b. Verify that line is valid.
            c. Verify that line contains an equal sign.
            d. If key is valid, register the key and its value.
        3) Free the file.

    Parameters (variable_name / type / description):
        - file_path / string / Path to the config file to parse.

    Returns (type + description):
        A map containing the parsed keys and their values.
*/
std::map<std::string, std::string> Parsers::parse_config_file
(
    const std::string &file_path
)
{
    ////////////////// 1) //////////////////
    Logs::log("Parsing \"" + file_path + "\" config file..");

    std::map<std::string, std::string> output;
    std::ifstream file(file_path);

    if (!file.is_open())
    {
        Logs::log("Warning: Failed to open file for parsing.");
        return output;
    }

    ////////////////// 2) //////////////////
    ///////// a. /////////
    std::string line;

    while (getline(file, line))
    {
        ///////// b. /////////
        if (line.empty() || line[0] == '#')
            continue;

        ///////// c. /////////
        const size_t equal_sign = line.find("=");

        if (equal_sign == std::string::npos)
        {
            Logs::log("\nWarning: Ignored invalid line -> \"" + line + "\".");
            continue;
        }

        ///////// d. /////////
        const std::string key = Text::trim(line.substr(0, equal_sign));
        const std::string value = Text::trim(line.substr(equal_sign + 1));

        if (key.empty())
        {
            Logs::log("\nWarning: Ignored keyless line -> \"" + line + "\".");
            continue;
        }

        output[key] = value;
    }

    ////////////////// 3) //////////////////
    file.close();
    Logs::log("Config file parsed -> " + std::to_string(output.size()) + " keys have been loaded.");

    return output;
}
