#include "events.hpp"

#include "../utils/database/database.hpp"
#include "../utils/logs/logs.hpp"

#include <dpp/dpp.h>
#include <mysql/mysql.h>
#include <string>

/*
    Event that triggers when a new member joins a guild.

    Tasks:
        1) Add the listener to the bot.
        2) Handle the new member.
            a. Get some information about the event and log the arrival.
            b. Try to get some configuration from the database.
            c. Try to give the "Member" role to the new member.
            d. Try to send a welcome message.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster / Client of the bot with all related information.
        - database  / MYSQL*       / MineWorld database

    Returns (type + description):
        No object returned.
*/
void Events::guild_member_add
(
    dpp::cluster &bot,
    MYSQL*       &database
)
{
    ////////////////// 1) //////////////////
    bot.on_guild_member_add([&bot, &database](const dpp::guild_member_add_t &event)
    {
        ////////////////// 2) //////////////////
        ///////// a. /////////
        const dpp::snowflake guild_id = event.adding_guild.id;
        const dpp::snowflake user_id = event.added.user_id;

        Logs::log("New member on guild " + std::to_string(guild_id) + " -> " + std::to_string(user_id) + ".");

        ///////// b. /////////
        Database::Output config = Database::db_query(database, "SELECT member_role, welcome_channel FROM config LIMIT 1");

        if (config.size() == 0)
        {
            Logs::log("Warning: No config available -> guild member add.");
            return;
        }

        ///////// c. /////////
        const dpp::snowflake member_role = dpp::snowflake(config[0]["member_role"]);

        bot.guild_member_add_role(guild_id, user_id, member_role, [member_role, user_id](const dpp::confirmation_callback_t &callback)
        {
            if (callback.is_error())
                Logs::log("Warning: Failed to add role " + std::to_string(member_role) + " to " + std::to_string(user_id) + " with error " + callback.get_error().human_readable + " -> guild member add.");
        });

        ///////// d. /////////
        const dpp::snowflake welcome_channel = dpp::snowflake(config[0]["welcome_channel"]);

        bot.message_create
        (
            dpp::message(welcome_channel, ":wave: **Welcome** to <@" + std::to_string(user_id) + "> who **just joined** the server!"),
            [welcome_channel](const dpp::confirmation_callback_t &callback)
            {
                if (callback.is_error())
                    Logs::log("Warning: Failed to send message in " + std::to_string(welcome_channel) + " with error " + callback.get_error().human_readable + " -> guild member add.");
            }
        );
    });
}
