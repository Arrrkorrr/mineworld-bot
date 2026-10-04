#include "events.hpp"

#include "../autocomplete/autocomplete.hpp"

#include <dpp/dpp.h>
#include <mysql/mysql.h>
#include <string>

/*
    Event that triggers when a slash command parameter using auto complete is interacted with.

    Tasks:
        1) Add the listener to the bot.
        2) Handle the auto complete request.
            a. Handle the request depending on the command name.
            b. Handle the request more specifically depending on the subcommand name.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster / Client of the bot with all related information.
        - database  / MYSQL*       / MineWorld database

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
        if (event.name == "nation")
        {
            ////////////////// 3) //////////////////
            const std::string subcommand = event.options[0].name;

            if (subcommand == "info" || subcommand == "join" || subcommand == "relation")
                Autocomplete::nations(bot, database, event);
            else if (subcommand == "rank")
                Autocomplete::ranks(bot, event);
        }
    });
}
