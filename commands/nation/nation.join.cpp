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
    Join a nation.

    Tasks:
        1) We start with some basic verification.
            a. Sanitize user input to prevent SQL injections as much as possible.
            b. Try to get some information about the nation from the database to check that it exists.
            c. Verify that the nation was not set as "closed" by the government.
            d. Try to get some information about the user in the "nationality" table. If we get some data back, it means that the user is already part of a nation.
            e. Try to get some information about the nation that the user is part of to make a clean detailed error message.
        2) We process the joining request.
            a. If the nation is set as "opened" by the government, we immediately register the user as citizen of the nation in the database.
            b. If the nation is set as "on invitation", we verify that the user has a pending invitation.
            c. Verify that the invitation has not expired yet. If it expired, we directly delete the invitation from the database.
            d. Try to give the nation role to the user.
            e. Get bot config to retrieve essentials information for later.
            f. Check that the "gossip" channel and role are valid, and send an embed notifying other players of the new citizen.
               We also make a "dynamic" notification in case the user joined with an invitation to precise who invited them.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster              / Client of the bot with all related information.
        - database  / MYSQL*                    / MineWorld database
        - event     / dpp::interaction_create_t / All information about the event.

    Returns (type + description):
        No object returned.
*/
void Nation::join_nation
(
    dpp::cluster                    &bot,
    MYSQL*                          &database,
    const dpp::interaction_create_t &event
)
{
    ////////////////// 1) //////////////////
    ///////// a. /////////
    const std::string nation_id = Database::sanitize_input(database, std::get<std::string>(event.get_parameter("nation_id")));

    ///////// b. /////////
    Database::Output nations = Database::db_query(database, "SELECT display_name, government_type, join_condition, role_id FROM nations WHERE nation_id = '" + nation_id + "' LIMIT 1");

    if (nations.size() == 0)
    {
        event.reply(dpp::message(":prohibited: Nation `" + nation_id + "` does not exist.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// c. /////////
    const std::string government_type = Text::get_government_type(std::stoi(nations[0]["government_type"]));
    const int join_condition = std::stoi(nations[0]["join_condition"]);
    const std::string display_name = nations[0]["display_name"];

    if (join_condition == CLOSED)
    {
        event.reply(dpp::message(":prohibited: The " + government_type + " of " + display_name + " is currently closed to anyone.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// d. /////////
    const dpp::snowflake user_id = event.command.usr.id;
    Database::Output nationality = Database::db_query(database, "SELECT nation_id, rank FROM nationality WHERE user_id = '" + std::to_string(user_id) + "' LIMIT 1");

    if (nationality.size() != 0)
    {
        ///////// e. /////////
        const std::string current_nation_id = nationality[0]["nation_id"];
        Database::Output current = Database::db_query(database, "SELECT display_name, government_type FROM nations WHERE nation_id = '" + current_nation_id + "' LIMIT 1");

        if (current.size() == 0)
        {
            Logs::log("Warning: Bad nation_id " + current_nation_id + " -> /nation claim.");
            return event.reply(dpp::message(":prohibited: You already part of another nation.").set_flags(dpp::m_ephemeral));
        }

        const std::string current_name = current[0]["display_name"];
        const std::string current_gov_type = current[0]["government_type"];

        return event.reply(dpp::message(":prohibited: You are already part of the " + current_gov_type + " of " + current_name + ".").set_flags(dpp::m_ephemeral));
    }

    ////////////////// 2) //////////////////
    ///////// a. /////////
    const std::string now = std::to_string(Miscellaneous::get_current_timestamp());
    const std::string citizen = std::to_string(CITIZEN);

    if (join_condition == OPENED)
    {
        Database::db_query(database, "INSERT INTO nationality (user_id, nation_id, rank, last_rank_update, joining_time) VALUES ('" + std::to_string(user_id) + "', '" + nation_id + "', '" + citizen + "', '" + now + "', '" + now + "')");
        event.reply(dpp::message(":hammer_pick: You are now a citizen of the " + government_type + " of " + display_name + ".").set_flags(dpp::m_ephemeral));
    }

    ///////// b. /////////
    std::string inviter_id;

    if (join_condition == ON_INVITATION)
    {
        Database::Output invitation = Database::db_query(database, "SELECT invited_by, creation_time FROM invitations WHERE user_id = '" + std::to_string(user_id) + "' AND nation_id = '" + nation_id + "' LIMIT 1");

        if (invitation.size() == 0)
        {
            event.reply(dpp::message(":prohibited: You need an invitation to join the " + government_type + " of " + display_name + ".").set_flags(dpp::m_ephemeral));
            return;
        }

        ///////// c. /////////
        const int64_t invitation_time = std::stoll(invitation[0]["creation_time"]);
        const int expiration = 86400; // 24 hours.

        if (invitation_time + expiration < std::stoll(now))
        {
            Database::db_query(database, "DELETE FROM invitations WHERE user_id = '" + std::to_string(user_id) + "' AND nation_id = '" + nation_id + "'");
            return event.reply(dpp::message(":prohibited: Your invitation to join the " + government_type + " of " + display_name + " has expired.").set_flags(dpp::m_ephemeral));
        }

        Database::db_query(database, "INSERT INTO nationality (user_id, nation_id, rank, last_rank_update, joining_time) VALUES ('" + std::to_string(user_id) + "', '" + nation_id + "', '" + citizen + "', '" + now + "', '" + now + "')");
        event.reply(dpp::message(":hammer_pick: You are now a citizen of the " + government_type + " of " + display_name + ".").set_flags(dpp::m_ephemeral));
        inviter_id = invitation[0]["invited_by"];
    }

    ///////// d. /////////
    const dpp::snowflake guild_id = event.command.guild_id;
    const dpp::snowflake role_id = dpp::snowflake(nations[0]["role_id"]);

    bot.guild_member_add_role(guild_id, user_id, role_id, [role_id, user_id](const dpp::confirmation_callback_t &callback)
    {
        if (callback.is_error())
            Logs::log("Warning: Failed to give role " + std::to_string(role_id) + " to " + std::to_string(user_id) + " with error " + callback.get_error().human_readable + " -> /nation join.");
    });

    ///////// e. /////////
    Database::Output config = Database::db_query(database, "SELECT world_channel FROM config LIMIT 1");

    if (config.size() == 0)
    {
        Logs::log("Warning: No config for " + std::to_string(guild_id) + " -> /nation join.");
        return;
    }

    const dpp::snowflake world_channel = dpp::snowflake(config[0]["world_channel"]);

    ///////// f. /////////
    std::string was_invited;

    if (join_condition == ON_INVITATION)
    {
        Database::Output inviter = Database::db_query(database, "SELECT rank FROM nationality WHERE user_id = '" + inviter_id + "' AND nation_id = '" + nation_id + "' LIMIT 1");

        if (inviter.size() != 0)
        {
            const std::string rank = Text::get_rank(std::stoi(inviter[0]["rank"]));
            was_invited = " They were invited by " + rank + " <@" + inviter_id + ">.";
        }
    }

    const dpp::embed embed = dpp::embed()
    .set_color(dpp::colors::light_green)
    .set_title("New Citizen")
    .set_description("<@" + std::to_string(user_id) + "> just received their citizenship from the " + government_type + " of " + display_name + "." + was_invited);

    bot.message_create
    (
        dpp::message(world_channel, "").add_embed(embed),
        [world_channel](const dpp::confirmation_callback_t &callback)
        {
            if (callback.is_error())
                Logs::log("Warning: Failed to send message in " + std::to_string(world_channel) + " with error " + callback.get_error().human_readable + " -> /nation join.");
        }
    );
}
