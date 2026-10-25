#include "nation.hpp"

#include "../../config/tweaks.hpp"
#include "../../enum/ranks.hpp"
#include "../../utils/database/database.hpp"
#include "../../utils/logs/logs.hpp"
#include "../../utils/miscellaneous/miscellaneous.hpp"
#include "../../utils/text/text.hpp"

#include <algorithm>
#include <dpp/cluster.h>
#include <dpp/colors.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <dpp/message.h>
#include <dpp/restresults.h>
#include <dpp/role.h>
#include <dpp/snowflake.h>
#include <mysql/mysql.h>
#include <string>

/*
    Create a new nation.

    Tasks:
        1) Some verification.
            a. Verify that the executer has a MineWorld account linked to their Discord account.
            b. Get command parameters and sanitize them.
            c. Verify that the nation does not already exist.
            d. Verify that the executer is stateless.
            e. If they are not stateless, get information about executer current nationality.
        2) Create the nation.
            a. Register the new nation.
            b. Retrieve the ID of the new nation.
            c. Register the nation in other tables.
            d. Set the executer as the Head of State of the nation.
            e. Create a role for the nation.
            f. Give the role to the executer.
        3) Notify the world.
            a. Get bot configuration.
            b. Send a notification embed in the world channel.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster              / Client of the bot with all related information.
        - database  / MYSQL*                    / MineWorld database.
        - event     / dpp::interaction_create_t / All information about the event.

    Returns (type + description):
        No object returned.
*/
void Nation::nation_create
(
    dpp::cluster                    &bot,
    MYSQL*                          &database,
    const dpp::interaction_create_t &event
)
{
    ////////////////// 1) //////////////////
    ///////// a. /////////
    const dpp::snowflake executer_discord_id = event.command.usr.id;
    Database::Output executer_credentials = Database::db_query(database, "SELECT user_id FROM players WHERE discord_id = '" + std::to_string(executer_discord_id) + "' LIMIT 1");

    if (executer_credentials.size() == 0)
    {
        event.reply(":prohibited: No MineWorld account is linked to your Discord account. Create a MineWorld account using `/account register` or link your already existing account with `/account discord`.");
        return;
    }

    ///////// b. /////////
    const std::string nation_name = Database::sanitize_input(database, std::get<std::string>(event.get_parameter("display_name")));
    const std::string nation_description = Database::sanitize_input(database, std::get<std::string>(event.get_parameter("description")));
    const int64_t government_type = std::clamp(std::get<int64_t>(event.get_parameter("government_type")), 0L, (int64_t) Tweaks::MAX_GOVERNMENT_TYPES);
    const int64_t ideology = std::clamp(std::get<int64_t>(event.get_parameter("ideology")), 0L, (int64_t) Tweaks::MAX_IDEOLOGIES);

    ///////// c. /////////
    const std::string lowercase_nation_name = Text::to_lowercase(nation_name);
    Database::Output already_existing_nation = Database::db_query(database, "SELECT 1 FROM nations WHERE LOWER(display_name) = '" + lowercase_nation_name + "' LIMIT 1");

    if (already_existing_nation.size() != 0)
    {
        event.reply(dpp::message(":prohibited: Nation `" + nation_name + "` already exists.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// d. /////////
    const std::string executer_id = executer_credentials[0]["user_id"];
    Database::Output executer_nationality = Database::db_query(database, "SELECT nation_id, rank FROM nationalities WHERE user_id = '" + executer_id + "' LIMIT 1");

    if (executer_nationality.size() != 0)
    {
        ///////// e. /////////
        const std::string executer_nation_id = executer_nationality[0]["nation_id"];
        Database::Output executer_nation = Database::db_query(database, "SELECT nations.display_name, governments.government_type FROM nations JOIN governments ON governments.nation_id = nations.nation_id WHERE nation_id = '" + executer_nation_id + "' LIMIT 1");

        if (executer_nation.size() == 0)
        {
            event.reply(dpp::message(":prohibited: You already part of another nation.").set_flags(dpp::m_ephemeral));
            return;
        }

        const std::string executer_nation_name = executer_nation[0]["nations.display_name"];
        const std::string executer_government_type = Text::get_government_type(std::stoi(executer_nation[0]["governments.government_type"]));
        const std::string executer_rank_name = Text::get_rank(std::stoi(executer_nationality[0]["rank"]));

        return event.reply(dpp::message(":prohibited: You are already part of the " + executer_government_type + " of " + executer_nation_name + " as " + executer_rank_name + ".").set_flags(dpp::m_ephemeral));
    }

    ////////////////// 2) //////////////////
    ///////// a. /////////
    const std::string now = std::to_string(Miscellaneous::get_current_timestamp());
    Database::db_query(database, "INSERT INTO nations (display_name, description, creation_time) VALUES ('" + nation_name + "', '" + nation_description + "', '" + now + "')");

    ///////// b. /////////
    Database::Output new_nation = Database::db_query(database, "SELECT nation_id FROM nations WHERE display_name = '" + nation_name + "' LIMIT 1");

    if (new_nation.size() == 0)
    {
        event.reply(dpp::message(":prohibited: Failed to create nation `" + nation_name + "`.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// c. /////////
    const std::string nation_id = new_nation[0]["nation_id"];

    Database::db_query(database, "INSERT INTO economies (nation_id) VALUES ('" + nation_id + "')");
    Database::db_query(database, "INSERT INTO governments (nation_id, government_type, ideology, last_leader_change, last_government_change) VALUES ('" + nation_id + "', '" + std::to_string(government_type) + "', '" + std::to_string(ideology) + "', '" + now + "', '" + now + "')");
    Database::db_query(database, "INSERT INTO immigration (nation_id) VALUES ('" + nation_id + "')");
    Database::db_query(database, "INSERT INTO media (nation_id) VALUES ('" + nation_id + "')");
    Database::db_query(database, "INSERT INTO warfare (nation_id) VALUES ('" + nation_id + "')");

    ///////// d. /////////
    const std::string government_type_name = Text::get_government_type(government_type);
    const std::string head_of_state_rank = std::to_string(HEAD_OF_STATE);

    Database::db_query(database, "INSERT INTO nationality (user_id, nation_id, rank, last_rank_update, joining_time) VALUES ('" + executer_id + "', '" + nation_id + "', '" + head_of_state_rank + "', '" + now + "', '" + now + "')");
    event.reply(dpp::message(":military_medal: You are now the Head of State of the " + government_type_name + " of " + nation_name + ".").set_flags(dpp::m_ephemeral));

    ///////// e. /////////
    const dpp::snowflake guild_id = event.command.guild_id;
    const dpp::role nation_role = dpp::role().set_guild_id(guild_id).set_name(nation_name).set_color(dpp::colors::white);

    bot.role_create(nation_role, [&bot, &database, nation_name, guild_id, nation_id, executer_discord_id](const dpp::confirmation_callback_t &callback)
    {
        if (callback.is_error())
        {
            Logs::log("Warning: Failed to create role for " + nation_name + " with error " + callback.get_error().human_readable + " -> /nation create.");
            return;
        }

        const dpp::snowflake nation_role_id = std::get<dpp::role>(callback.value).id;
        Database::db_query(database, "UPDATE nations SET role_id = '" + std::to_string(nation_role_id) + "' WHERE nation_id = '" + nation_id + "'");

        ///////// f. /////////
        bot.guild_member_add_role(guild_id, executer_discord_id, nation_role_id, [&bot, nation_role_id, executer_discord_id](const dpp::confirmation_callback_t &callback)
        {
            if (callback.is_error())
                Logs::log("Warning: Failed to give role " + std::to_string(nation_role_id) + " to " + std::to_string(executer_discord_id) + " with error " + callback.get_error().human_readable + " -> /nation create.");
        });
    });

    ////////////////// 3) //////////////////
    ///////// a. /////////
    Database::Output config = Database::db_query(database, "SELECT world_channel FROM config LIMIT 1");

    if (config.size() == 0)
    {
        Logs::log("Warning: No config data -> /nation create.");
        return;
    }

    ///////// b. /////////
    const dpp::snowflake world_channel = dpp::snowflake(config[0]["world_channel"]);

    const dpp::embed embed = dpp::embed()
    .set_color(dpp::colors::light_green)
    .set_title("New Nation")
    .set_description("Stateless <@" + std::to_string(executer_discord_id) + "> created and took the leadership of the " + government_type_name + " of " + nation_name + " that advocates " + Text::get_ideology(ideology) + ".");

    bot.message_create
    (
        dpp::message(world_channel, "").add_embed(embed),
        [world_channel](const dpp::confirmation_callback_t &callback)
        {
            if (callback.is_error())
                Logs::log("Warning: Failed to send message in " + std::to_string(world_channel) + " with error " + callback.get_error().human_readable + " -> /nation create.");
        }
    );
}
