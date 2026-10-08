#include "nation.hpp"

#include "../../config/enumerations.hpp"
#include "../../config/tweaks.hpp"
#include "../../utils/database/database.hpp"
#include "../../utils/logs/logs.hpp"
#include "../../utils/miscellaneous/miscellaneous.hpp"
#include "../../utils/text/text.hpp"

#include <algorithm>
#include <dpp/dpp.h>
#include <mysql/mysql.h>
#include <string>

/*
    Create a new nation.

    Tasks:
        1) Do some verification.
            a. Get and sanitize user inputs.
            b. Verify that the nation does not already exist.
            c. Verify that the user is not already part of another nation.
            d. Get some information about the user current nationality to make a cleaner message.
        2) Grant ownership to the user.
            a. Register the new nation and try to get the newly generated nation_id
            b. Register the user as the Head of State of the new nation.
            c. Create a new role for the nation.
            d. Try to give the new role to the user.
            e. Get bot config to retrieve essential information.
            f. Try to send an embed notifying other players of the nation creation.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster              / Client of the bot with all related information.
        - database  / MYSQL*                    / MineWorld database
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
    ///////// a. //////////
    const std::string display_name = Database::sanitize_input(database, std::get<std::string>(event.get_parameter("display_name")));
    const std::string description = Database::sanitize_input(database, std::get<std::string>(event.get_parameter("description")));
    const int64_t gov_type = std::clamp(std::get<int64_t>(event.get_parameter("government_type")), 0L, (int64_t) Tweaks::MAX_GOVERNMENT_TYPES);
    const int64_t ideology = std::clamp(std::get<int64_t>(event.get_parameter("ideology")), 0L, (int64_t) Tweaks::MAX_IDEOLOGIES);

    ///////// b. /////////
    const std::string lowercase = Text::to_lowercase(display_name);
    Database::Output nations = Database::db_query(database, "SELECT 1 FROM nations WHERE LOWER(display_name) = '" + lowercase + "' LIMIT 1");

    if (nations.size() != 0)
    {
        event.reply(dpp::message(":prohibited: Nation `" + display_name + "` already exists.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// c. /////////
    const dpp::snowflake user_id = event.command.usr.id;
    Database::Output nationality = Database::db_query(database, "SELECT nation_id, rank FROM nationality WHERE user_id = '" + std::to_string(user_id) + "' LIMIT 1");

    if (nationality.size() != 0)
    {
        ///////// d. /////////
        const std::string current_nation_id = nationality[0]["nation_id"];
        Database::Output current = Database::db_query(database, "SELECT display_name, government_type FROM nations WHERE nation_id = '" + current_nation_id + "' LIMIT 1");

        if (current.size() == 0)
        {
            event.reply(dpp::message(":prohibited: You already part of another nation.").set_flags(dpp::m_ephemeral));
            return;
        }

        const std::string current_name = current[0]["display_name"];
        const std::string government_type = Text::get_government_type(std::stoi(current[0]["government_type"]));
        const std::string rank = Text::get_rank(std::stoi(nationality[0]["rank"]));

        return event.reply(dpp::message(":prohibited: You are already part of the " + government_type + " of " + current_name + " as " + rank + ".").set_flags(dpp::m_ephemeral));
    }

    ////////////////// 2) //////////////////
    ///////// a. /////////
    const std::string now = std::to_string(Miscellaneous::get_current_timestamp());

    Database::db_query(database, "INSERT INTO nations (display_name, description, creation_time, government_type, ideology) VALUES ('" + display_name + "', '" + description + "', '" + now + "', '" + std::to_string(gov_type) + "', '" + std::to_string(ideology) + "')");
    Database::Output created_nation = Database::db_query(database, "SELECT nation_id FROM nations WHERE display_name = '" + display_name + "' LIMIT 1");

    if (created_nation.size() == 0)
    {
        event.reply(dpp::message(":prohibited: Failed to create nation `" + display_name + "`.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// b. /////////
    const std::string government_type = Text::get_government_type(gov_type);
    const std::string nation_id = created_nation[0]["nation_id"];
    const std::string leader = std::to_string(LEADER);

    Database::db_query(database, "INSERT INTO nationality (user_id, nation_id, rank, last_rank_update, joining_time) VALUES ('" + std::to_string(user_id) + "', '" + nation_id + "', '" + leader + "', '" + now + "', '" + now + "')");
    event.reply(dpp::message(":military_medal: You are now the Head of State of the " + government_type + " of " + display_name + ".").set_flags(dpp::m_ephemeral));

    ///////// c. /////////
    const dpp::snowflake guild_id = event.command.guild_id;
    const dpp::role new_role = dpp::role().set_guild_id(guild_id).set_name(display_name).set_color(dpp::colors::white);

    bot.role_create(new_role, [&bot, &database, display_name, guild_id, nation_id, user_id](const dpp::confirmation_callback_t &callback)
    {
        if (callback.is_error())
        {
            Logs::log("Warning: Failed to create role for " + display_name + " with error " + callback.get_error().human_readable + " -> /nation create.");
            return;
        }

        const dpp::snowflake role_id = std::get<dpp::role>(callback.value).id;
        Database::db_query(database, "UPDATE nations SET role_id = '" + std::to_string(role_id) + "' WHERE nation_id = '" + nation_id + "'");

        ///////// d. /////////
        bot.guild_member_add_role(guild_id, user_id, role_id, [&bot, role_id, user_id](const dpp::confirmation_callback_t &callback)
        {
            if (callback.is_error())
                Logs::log("Warning: Failed to give role " + std::to_string(role_id) + " to " + std::to_string(user_id) + " with error " + callback.get_error().human_readable + " -> /nation create.");
        });
    });

    ///////// e. /////////
    Database::Output config = Database::db_query(database, "SELECT world_channel FROM config LIMIT 1");

    if (config.size() == 0)
    {
        Logs::log("Warning: No config data -> /nation create.");
        return;
    }

    const dpp::snowflake world_channel = dpp::snowflake(config[0]["world_channel"]);

    ///////// f. /////////
    const dpp::embed embed = dpp::embed()
    .set_color(dpp::colors::light_green)
    .set_title("New Nation")
    .set_description("Stateless <@" + std::to_string(user_id) + "> created and took the leadership of the " + government_type + " of " + display_name + " that advocates " + Text::get_ideology(ideology) + ".");

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
