#include "logs.hpp"

#include "../files/files.hpp"
#include "../miscellaneous/miscellaneous.hpp"
#include "../text/text.hpp"
#include "../../config/tweaks.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <time.h>
#include <vector>

/*
    Send a message in the terminal.

    Tasks:
        1) Verify that the message to log is not empty.
        2) Handle the log.
            a. Get the current timestamp and convert into a data.
            b. Format the message. Date format: DD.MM.YY-HH:MN:SS.
            c. Send the log into the terminal.
        3) If enabled (config/tweaks.hpp), we also write the log into the "bot.logs".

    Parameters (variable_name / type / description):
        - message / string / Message to log.

    Returns (type + description):
        No object returned.
*/
void Logs::log
(
    const std::string &message
)
{
    ////////////////// 1) //////////////////
    if (Text::trim(message).size() < 1)
        return;

    ////////////////// 2) //////////////////
    ///////// a. /////////
    const time_t timestamp = Miscellaneous::get_current_timestamp();
    const std::vector<std::string> date = Miscellaneous::get_date_from_timestamp(timestamp);

    ///////// b. /////////
    const std::string date_format = date[2] + "." + date[1] + "." + date[0] + "-" + date[3] + ":" + date[4] + ":" + date[5];
    const std::string log = "[" + date_format + "] " + message + "\n"; // [time] message

    ///////// c. /////////
    std::cout << log;

    ////////////////// 3) //////////////////
    if constexpr (Tweaks::ENABLE_LOGS_FILE)
        Files::write_file(true, log, "bot.logs");
}



/*
    Send a message in the terminal and throw a runtime error.

    Tasks:
        1) Send the log into the terminal.
        2) If enabled (config/tweaks.hpp), we also write the log into the "bot.logs".
        3) Throw a runtime error with the message.

    Parameters (variable_name / type / description):
        - message / string / Message to log as a crash.

    Returns (type + description):
        No object returned.
*/
void Logs::crash_log
(
    const std::string &message
)
{
    ////////////////// 1) //////////////////
    std::cout << "[crash] " << message << "\n";

    ////////////////// 2) //////////////////
    if constexpr (Tweaks::ENABLE_LOGS_FILE)
        Files::write_file(true, message, "bot.logs");

    ////////////////// 3) //////////////////
    throw std::runtime_error(message);
}
