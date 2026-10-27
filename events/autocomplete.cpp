#include "events.hpp"

#include "../autocomplete/autocomplete.hpp"

#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <mysql/mysql.h>
#include <string>

/*
    Event triggered when a slash command input using auto complete is interacted with.

    Tasks:
        1) Add listener to bot.
        2) Handle auto complete request.
            a. Handle the request depending on command name.
            b. Handle the request more specifically depending on subcommand name.

    Parameters (variable_name / type / description):
        - bot      / dpp::cluster / Client of the bot with all related information.
        - database / MYSQL*       / MineWorld database.

    Returns (type + description):
        No object returned.
*/
void Events::autocomplete
(
    dpp::cluster &bot,
    MYSQL*       &database
)
{
    ////////////////// 1) //////////////////
    bot.on_autocomplete([&bot, &database](const dpp::autocomplete_t &event)
    {
        ////////////////// 2) //////////////////
        const std::string command_name = event.name;

        if (command_name == "nation")
        {
            ////////////////// 3) //////////////////
            const std::string subcommand_name = event.options[0].name;

            if (subcommand_name == "create")
            {
                Autocomplete::government_types(bot, event);
                Autocomplete::ideologies(bot, event);
            }
            else if (subcommand_name == "info" || subcommand_name == "join" || subcommand_name == "relation")
                Autocomplete::nations(bot, database, event);
            else if (subcommand_name == "rank")
                Autocomplete::ranks(bot, event);
        }
    });
}
