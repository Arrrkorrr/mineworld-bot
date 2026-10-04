#include "config/tweaks.hpp"
#include "events/events.hpp"

#include "utils/database/database.hpp"
#include "utils/files/files.hpp"
#include "utils/logs/logs.hpp"
#include "utils/miscellaneous/miscellaneous.hpp"
#include "utils/parsers/parsers.hpp"

#include <dpp/dpp.h>
#include <filesystem>
#include <map>
#include <mysql/mysql.h>
#include <string>

/*
    Main function of the project.

    Tasks:
        1) Handle the logs file.
            a. Remove the old bot.logs file if it exists.
            b. Create a new bot.logs file if the log file is turned on.
        2) Load configuration.
            a. Parse and retrieve values from the bot.config file.
            b. Verify that the port in the config is valid or we fall back on MariaDB default port.
        3) Establish a connection to the database.
        4) Start the bot.
            a. Establish a connection to bot client with its token and intents.
            b. Add all event listeners to the bot.
            c. Start the bot and put an infinite wait so it never stops.

    Parameters (variable_name / type / description):
        No parameters.

    Returns (type + description):
        An integer containing the exit code of the program.
*/
int main()
{
    ////////////////// 1) //////////////////
    ///////// a. /////////
    if (std::filesystem::exists("bot.logs"))
        std::filesystem::remove("bot.logs");

    ///////// b. /////////
    if constexpr (Tweaks::ENABLE_LOGS_FILE)
        Files::create_new_empty_file("bot.logs");

    ////////////////// 2) //////////////////
    ///////// a. /////////
    std::map<std::string, std::string> config = Parsers::parse_config_file("bot.config");

    const std::string token = config["TOKEN"];
    const std::string db_name = config["DB_NAME"];
    const std::string db_host = config["DB_HOST"];
    const std::string db_port_str = config["DB_PORT"];
    const std::string db_user = config["DB_USER"];
    const std::string db_password = config["DB_PASSWORD"];

    ///////// b. /////////
    int db_port = 0;

    if (!Miscellaneous::is_an_integer(db_port_str))
    {
        Logs::log("Warning: Invalid port provided, selected MariaDB's default port 3306.");
        db_port = 3306;
    }
    else db_port = stoi(db_port_str);

    ////////////////// 3) //////////////////
    MYSQL* database = nullptr;
    Database::database_handler db_handler(database, db_name, db_host, db_port, db_user, db_password);

    ////////////////// 4) //////////////////
    ///////// a. /////////
    const uint32_t intents = dpp::i_all_intents;
    dpp::cluster bot(token, intents);

    ///////// b. /////////
    Events::ready(bot);
    Events::message_create(bot);
    Events::guild_member_add(bot, database);
    Events::guild_member_remove(bot);
    Events::interaction_create(bot, database);
    Events::autocomplete(bot, database);
    Events::form_submit(bot, database);
    Events::button_click(bot, database);
    Events::select_menu_click(bot, database);

    ///////// c. /////////
    bot.start(dpp::st_wait);
    return 0;
}
