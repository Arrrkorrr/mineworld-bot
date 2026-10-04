#include "events.hpp"

#include "../commands/commands.hpp"

#include <dpp/dpp.h>
#include <mysql/mysql.h>
#include <string>

/*
    Event that triggers when a slash command has been executed.

    Tasks:
        1) Add the listener to the bot.
        2) Handle the slash command execution.
            a. Handle the request depending on the command name.
            b. Handle the request depending on the subcommand name.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster / Client of the bot with all related information.
        - database  / MYSQL*       / MineWorld database

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
        if (event.command.get_command_name() == "ban")
            Commands::ban(bot, event);
        else if (event.command.get_command_name() == "journalism")
            Commands::journalism(event);
        else if (event.command.get_command_name() == "kick")
            Commands::kick(bot, event);
        else if (event.command.get_command_name() == "nation")
        {
            ///////// b. /////////
            const std::string subcommand = event.command.get_command_interaction().options[0].name;

            if (subcommand == "create")
                Commands::Nation::nation_create(bot, database, event);
            else if (subcommand == "config")
                Commands::Nation::nation_config(bot, database, event);
            else if (subcommand == "info")
                Commands::Nation::nation_info(bot, database, event);
            else if (subcommand == "invite")
                Commands::Nation::nation_invite(bot, database, event);
            else if (subcommand == "join")
                Commands::Nation::join_nation(bot, database, event);
            else if (subcommand == "kick")
                Commands::Nation::nation_kick(bot, database, event);
            else if (subcommand == "leave")
                Commands::Nation::leave_nation(bot, database, event);
            else if (subcommand == "rank")
                Commands::Nation::nation_rank(bot, database, event);
            else if (subcommand == "relation")
                Commands::Nation::nation_relation(bot, database, event);
        }
        else if (event.command.get_command_name() == "ping")
            Commands::ping(bot, event);
        else if (event.command.get_command_name() == "play")
            Commands::play(event);
        else if (event.command.get_command_name() == "rules")
            Commands::rules(event);
    });
}
