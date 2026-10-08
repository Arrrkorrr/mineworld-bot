#include "nation.hpp"

#include "../../config/enumerations.hpp"
#include "../../utils/database/database.hpp"
#include "../../utils/logs/logs.hpp"
#include "../../utils/miscellaneous/miscellaneous.hpp"
#include "../../utils/text/text.hpp"

#include <dpp/dpp.h>
#include <mysql/mysql.h>
#include <string>

/*
    Change the rank of a member of a nation.

    Tasks:
        1) We do some basic verification.
            a. Verify that the new rank provided by the executer is valid.
            b. Verify that the executer is not trying to modify their own rank.
            c. Verify that the executer is part of a nation.
            d. Try to retrieve some information about the executer nation.
            e. Verify that the targeted user is not stateless.
            f. Verify that both the executer and user are part of the same nation.
            g. Verify that the user does not already have the rank to give.
            h. Verify that the executer is not trying to give their own rank (except if they are the Head of State and trying to transfer leadership).
            i. Verify that the executer is not trying to give a rank that is higher than theirs.
            j. Verify that the executer and user ranks do not match.
            k. Verify that the executer rank is higher than the user rank.
        2) Process the rank modification request.
            a. Register the user rank as the new rank provided by the executer.
            b. If the Head of State or Prime Minister ranks were given, we try to demote the current users holding these ranks to Minister. We also update the nation stats.
            c. Try to retrieve some configuration from the database.
            d. Send a message in the gossip channel pinging the gossip role if possible, announcing the rank modification depending on what happened.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster              / Client of the bot with all related information.
        - database  / MYSQL*                    / MineWorld database
        - event     / dpp::interaction_create_t / All information about the event.

    Returns (type + description):
        No object returned.
*/
void Nation::nation_rank
(
    dpp::cluster                    &bot,
    MYSQL*                          &database,
    const dpp::interaction_create_t &event
)
{
    ////////////////// 1) //////////////////
    ///////// a. /////////
    const dpp::snowflake user_id = std::get<dpp::snowflake>(event.get_parameter("member"));
    const int64_t new_rank = std::get<int64_t>(event.get_parameter("new_rank"));

    if (new_rank < CITIZEN || new_rank > LEADER)
    {
        event.reply(dpp::message(":prohibited: Invalid new rank `" + std::to_string(new_rank) + "` provided.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// b. /////////
    const dpp::snowflake executer_id = event.command.usr.id;

    if (user_id == executer_id)
    {
        event.reply(dpp::message(":prohibited: You can not modify your own rank yourself.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// c. /////////
    Database::Output executer_nationality = Database::db_query(database, "SELECT nation_id, rank FROM nationality WHERE user_id = '" + std::to_string(executer_id) + "' LIMIT 1");

    if (executer_nationality.size() == 0)
    {
        event.reply(dpp::message(":prohibited: You can not perform this action while being stateless.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// d. /////////
    const std::string executer_nation_id = executer_nationality[0]["nation_id"];
    Database::Output nations = Database::db_query(database, "SELECT display_name, government_type, leadership_changes, government_changes FROM nations WHERE nation_id = '" + executer_nation_id + "' LIMIT 1");

    if (nations.size() == 0)
    {
        Logs::log("Warning: Nation ID " + executer_nation_id + " missing in database -> /nation leave.");
        return event.reply(dpp::message(":prohibited: Something went wrong while retrieving information about nation `" + executer_nation_id + "`.").set_flags(dpp::m_ephemeral));
    }

    ///////// e. /////////
    Database::Output user_nationality = Database::db_query(database, "SELECT nation_id, rank FROM nationality WHERE user_id = '" + std::to_string(user_id) + "' LIMIT 1");

    const std::string display_name = nations[0]["display_name"];
    const std::string government_type = Text::get_government_type(std::stoi(nations[0]["government_type"]));
    const std::string rank_name = Text::get_rank(new_rank);

    if (user_nationality.size() == 0)
    {
        event.reply(dpp::message(":prohibited: You can not set the rank of <@" + std::to_string(user_id) + "> to " + rank_name + " of the " + government_type + " of " + display_name + " as they are stateless.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// f. /////////
    const std::string user_nation_id = user_nationality[0]["nation_id"];

    if (user_nation_id != executer_nation_id)
    {
        Database::Output user_nation = Database::db_query(database, "SELECT display_name FROM nations WHERE nation_id = '" + user_nation_id + "' LIMIT 1");
        const std::string user_rank = Text::get_rank(std::stoi(user_nationality[0]["rank"]));

        if (user_nation.size() == 0)
        {
            Logs::log("Warning: Nation ID " + user_nation_id + " missing in database -> /nation rank.");
            return event.reply(dpp::message(":prohibited: You can not set the rank of <@" + std::to_string(user_id) + "> to " + rank_name + " of the " + government_type + " of " + display_name + " as they are part of another nation as " + user_rank + ".").set_flags(dpp::m_ephemeral));
        }

        const std::string nation_name = user_nation[0]["display_name"];
        return event.reply(dpp::message(":prohibited: You can not set the rank of <@" + std::to_string(user_id) + "> to " + rank_name + " of the " + government_type + " of " + display_name + " as they are part of " + nation_name + " as " + user_rank + ".").set_flags(dpp::m_ephemeral));
    }

    ///////// g. /////////
    const int user_rank = std::stoi(user_nationality[0]["rank"]);
    const std::string user_rank_name = Text::get_rank(user_rank);

    if (user_rank == new_rank)
    {
        event.reply(dpp::message(":prohibited: <@" + std::to_string(user_id) + "> already holds the " + rank_name + " rank.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// h. /////////
    const int executer_rank = std::stoi(executer_nationality[0]["rank"]);

    if (new_rank == executer_rank && new_rank != LEADER)
    {
        event.reply(dpp::message(":prohibited: You can not set the rank of " + user_rank_name + " <@" + std::to_string(user_id) + "> to " + rank_name + " of the " + government_type + " of " + display_name + " as the new rank is your current rank.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// i. /////////
    const std::string executer_rank_name = Text::get_rank(executer_rank);

    if (new_rank > executer_rank)
    {
        event.reply(dpp::message(":prohibited: You can not set the rank of " + user_rank_name + " <@" + std::to_string(user_id) + "> to " + rank_name + " of the " + government_type + " of " + display_name + " as the new rank is higher than yours (" + rank_name + " > " + executer_rank_name + ").").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// j. /////////
    if (user_rank == executer_rank)
    {
        event.reply(dpp::message(":prohibited: You can not set the rank of " + user_rank_name + " <@" + std::to_string(user_id) + "> to " + rank_name + " of the " + government_type + " of " + display_name + " as you both have the " + executer_rank_name + ".").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// k. /////////
    if (user_rank > executer_rank)
    {
        event.reply(dpp::message(":prohibited: You can not set the rank of " + user_rank_name +  " <@" + std::to_string(user_id) + "> to " + rank_name + " of the " + government_type + " of " + display_name + " as they have a higher rank than you (" + user_rank_name + " > " + executer_rank_name + ").").set_flags(dpp::m_ephemeral));
        return;
    }

    ////////////////// 2) //////////////////
    ///////// a. /////////
    Database::db_query(database, "UPDATE nationality SET rank = '" + std::to_string(new_rank) + "' WHERE user_id = '" + std::to_string(user_id) + "'");

    const bool promotion = (new_rank > user_rank);
    const std::string emoji = (promotion ? ":arrow_up:" : ":arrow_down:");
    const std::string verb = (promotion ? "promoted" : "demoted");

    event.reply(dpp::message(emoji + " <@" + std::to_string(user_id) + "> has been " + verb + " from " + user_rank_name + " to " + rank_name + " of the " + government_type + " of " + display_name + ".").set_flags(dpp::m_ephemeral));

    ///////// b. /////////
    const std::string leadership_changes = std::to_string(std::stoi(nations[0]["leadership_changes"]) + 1);
    const std::string now = std::to_string(Miscellaneous::get_current_timestamp());

    if (new_rank == LEADER)
    {
        Database::db_query(database, "UPDATE nations SET leadership_changes = '" + leadership_changes + "', last_leadership_change = '" + now + "' WHERE nation_id = '" + user_nation_id + "'");
        Database::db_query(database, "UPDATE nationality SET rank = '" + std::to_string(MINISTER) + "' WHERE nation_id = '" + user_nation_id + "' AND rank = '" + std::to_string(LEADER) + "'");

        Logs::log("Warning: Automatically demoted current Head of State of " + display_name + " -> /nation rank.");
    }

    const bool is_government_change = ((user_rank > MILITARY && user_rank != LEADER) || (new_rank > MILITARY && new_rank != LEADER));
    const std::string government_changes = std::to_string(std::stoi(nations[0]["government_changes"]) + 1);

    if (is_government_change)
    {
        Database::db_query(database, "UPDATE nations SET government_changes = '" + government_changes + "', last_government_change = '" + now + "' WHERE nation_id = '" + user_nation_id + "'");

        if (new_rank == PRIME_MINISTER)
        {
            Database::db_query(database, "UPDATE nationality SET rank = '" + std::to_string(MINISTER) + "' WHERE nation_id = '" + user_nation_id + "' AND rank = '" + std::to_string(PRIME_MINISTER) + "'");
            Logs::log("Warning: Automatically demoted current Prime Minister of " + display_name + " -> /nation rank.");
        }
    }

    ///////// c. /////////
    Database::Output config = Database::db_query(database, "SELECT world_channel FROM config LIMIT 1");

    if (config.size() == 0)
    {
        Logs::log("Warning: No config data -> /nation rank.");
        return;
    }

    const dpp::snowflake world_channel = dpp::snowflake(config[0]["world_channel"]);

    ///////// d. /////////
    const dpp::snowflake guild_id = event.command.guild_id;
    dpp::embed embed = dpp::embed();

    if (new_rank == LEADER)
    {
        embed.set_color(dpp::colors::gold)
        .set_title("Leadership Change")
        .set_description("The Head of State <@" + std::to_string(executer_id) + "> of the " + government_type + " of " + display_name + " resigned from their functions and named " + user_rank_name + " <@" + std::to_string(user_id) + "> as their successor.");
    }
    else if (new_rank == PRIME_MINISTER)
    {
        embed.set_color(dpp::colors::gold)
        .set_title("Prime Minister Change")
        .set_description("The Head of State <@" + std::to_string(executer_id) + "> named " + user_rank_name + " <@" + std::to_string(user_id) + "> as the new Prime Minister of the " + government_type + " of " + display_name + ".");
    }
    else
    {
        const uint32_t color = (promotion ? dpp::colors::light_green : dpp::colors::red);

        embed.set_color(color)
        .set_title("Rank Modification")
        .set_description(executer_rank_name + " <@" + std::to_string(executer_id) + "> just " + verb + " " + user_rank_name + " <@" + std::to_string(user_id) + "> to " + rank_name + " of the " + government_type + " of " + display_name + ".");
    }

    bot.message_create
    (
        dpp::message(world_channel, "").add_embed(embed),
        [world_channel](const dpp::confirmation_callback_t &callback)
        {
            if (callback.is_error())
                Logs::log("Warning: Failed to send message in " + std::to_string(world_channel) + " with error " + callback.get_error().human_readable + " -> /nation rank.");
        }
    );
}
