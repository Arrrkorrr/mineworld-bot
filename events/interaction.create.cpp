#include "events.hpp"

#include "../commands/commands.hpp"

#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <mysql/mysql.h>
#include <string>

/*
    Event triggered when a slash command has been executed.

    Tasks:
        1) Add listener to bot.
        2) Handle slash command execution.
            a. Handle the request depending on command name.
            b. Handle the request depending on subcommand_name name.

    Parameters (variable_name / type / description):
        - bot      / dpp::cluster / Client of the bot with all related information.
        - database / MYSQL*       / MineWorld database.

    Returns (type + description):
        No object returned.
*/
void Events::interaction_create
(
    dpp::cluster &bot,
    MYSQL*       &database
)
{
    ////////////////// 1) //////////////////
    bot.on_interaction_create([&bot, &database](const dpp::interaction_create_t event)
    {
        ////////////////// 2) //////////////////
        ///////// a. /////////
        const std::string command_name = event.command.get_command_name();

        if (command_name == "ban")
            Commands::ban(bot, event);
        else if (command_name == "journalism")
            Commands::journalism(event);
        else if (command_name == "kick")
            Commands::kick(bot, event);
        else if (command_name == "nation")
        {
            ///////// b. /////////
            const std::string subcommand_name = event.command.get_command_interaction().options[0].name;

            if (subcommand_name == "create")
                Commands::Nation::nation_create(bot, database, event);
            else if (subcommand_name == "info")
                Commands::Nation::nation_info(bot, database, event);
            else if (subcommand_name == "invite")
                Commands::Nation::nation_invite(bot, database, event);
            else if (subcommand_name == "join")
                Commands::Nation::join_nation(bot, database, event);
            else if (subcommand_name == "kick")
                Commands::Nation::nation_kick(bot, database, event);
            else if (subcommand_name == "leave")
                Commands::Nation::leave_nation(bot, database, event);
            else if (subcommand_name == "rank")
                Commands::Nation::nation_rank(bot, database, event);
            else if (subcommand_name == "relation")
                Commands::Nation::nation_relation(bot, database, event);
        }
        else if (command_name == "ping")
            Commands::ping(bot, event);
        else if (command_name == "play")
            Commands::play(event);
        else if (command_name == "rules")
            Commands::rules(event);
    });
}
