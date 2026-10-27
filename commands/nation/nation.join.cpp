#include "nation.hpp"

#include "../../enum/immigration.hpp"
#include "../../enum/nations.hpp"
#include "../../enum/ranks.hpp"
#include "../../utils/database/database.hpp"
#include "../../utils/logs/logs.hpp"
#include "../../utils/miscellaneous/miscellaneous.hpp"
#include "../../utils/text/text.hpp"

#include <dpp/cluster.h>
#include <dpp/colors.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <dpp/message.h>
#include <dpp/restresults.h>
#include <dpp/snowflake.h>
#include <mysql/mysql.h>
#include <string>

/*
    Join a nation.

    Tasks:
        1) Some verification.
            a. Verify that the executer has a MineWorld account linked to their Discord account.
            b. Get nation ID from command parameter and sanitize it.
            c. Get information about the nation.
            d. Verify that the nation is not closed.
            e. Verify that the executer is not already part of another nation.
        2) Process joining request.
            a. Grant citizenship to the executer if the nation is opened.
            b. If the nation is on invitation, verify that the executer has been invited.
            c. Verify that the invitation did not expire.
            d. Grant citizenship to the executer.
            e. Get information about the user who invited the executer.
            f. Add the nation role to the executer.
        3) Notify the world.
            a. Get bot configuration.
            b. Format text in case the executer was invited by someone.
            c. Send a notification embed in the world channel.

    Parameters (variable_name / type / description):
        - bot      / dpp::cluster              / Client of the bot with all related information.
        - database / MYSQL*                    / MineWorld database.
        - event    / dpp::interaction_create_t / All information about the event.

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
    const dpp::snowflake executer_discord_id = std::to_string(event.command.usr.id);
    Database::Output executer_credentials = Database::db_query(database, "SELECT user_id FROM players WHERE discord_id = '" + std::to_string(executer_discord_id) + "' LIMIT 1");

    if (executer_credentials.size() == 0)
    {
        event.reply(dpp::message(":prohibited: No MineWorld account is linked to your Discord account. Create a MineWorld account using `/account register` or link your already existing account with `/account discord`.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// b. /////////
    const std::string nation_id = Database::sanitize_input(database, std::to_string(std::get<int64_t>(event.get_parameter("nation_id"))));

    ///////// c. /////////
    const std::string query =
    (
        std::string("SELECT nations.display_name, nations.role_id, governments.government_type, immigration.join_condition, ") +
                    "FROM nations " +
                    "JOIN governments ON governments.nation_id = nations.nation_id " +
                    "JOIN immigration ON immigration.nation_id = nations.nation_id " +
                    "WHERE nations.nation_id = '" + nation_id + "' AND nations.deleted = '" + std::to_string(EXISTING_NATION) + "' LIMIT 1"
    );

    Database::Output nation = Database::db_query(database, query);

    if (nation.size() == 0)
    {
        event.reply(dpp::message(":prohibited: Nation `" + nation_id + "` does not or no longer exist.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// d. /////////
    const std::string government_type = Text::Nations::get_government_type(std::stoi(nation[0]["governments.government_type"]));
    const int join_condition = std::stoi(nation[0]["immigration.join_condition"]);
    const std::string nation_name = nation[0]["nations.display_name"];

    if (join_condition == CLOSED)
    {
        event.reply(dpp::message(":prohibited: The " + government_type + " of " + nation_name + " is currently closed to anyone.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// e. /////////
    const std::string executer_user_id = executer_credentials[0]["user_id"];
    Database::Output executer_nationality = Database::db_query(database, "SELECT nation_id, rank FROM nationality WHERE user_id = '" + executer_user_id + "' LIMIT 1");

    if (executer_nationality.size() != 0)
    {
        const std::string executer_nation_id = executer_nationality[0]["nation_id"];
        Database::Output executer_nation = Database::db_query(database, "SELECT nations.display_name, governments.government_type FROM nations JOIN governments ON governments.nation_id = nations.nation_id WHERE nation_id = '" + executer_nation_id + "' LIMIT 1");

        if (executer_nation.size() == 0)
        {
            Logs::log("Warning: Bad nation_id " + executer_nation_id + " -> /nation claim.");
            return event.reply(dpp::message(":prohibited: You already part of another nation.").set_flags(dpp::m_ephemeral));
        }

        const std::string executer_nation_name = executer_nation[0]["nations.display_name"];
        const std::string executer_government_type = executer_nation[0]["governments.government_type"];

        return event.reply(dpp::message(":prohibited: You are already part of the " + executer_nation_name + " of " + executer_nation_name + ".").set_flags(dpp::m_ephemeral));
    }

    ////////////////// 2) //////////////////
    ///////// a. /////////
    const std::string now = std::to_string(Miscellaneous::get_current_timestamp());
    const std::string citizen = std::to_string(CITIZEN);

    if (join_condition == OPENED)
    {
        Database::db_query(database, "INSERT INTO nationality (user_id, nation_id, rank, last_rank_update, joining_time) VALUES ('" + executer_user_id + "', '" + nation_id + "', '" + citizen + "', '" + now + "', '" + now + "')");
        event.reply(dpp::message(":hammer_pick: You are now a citizen of the " + government_type + " of " + nation_name + ".").set_flags(dpp::m_ephemeral));
    }

    ///////// b. /////////
    std::string inviter_user_id, inviter_discord_id;

    if (join_condition == ON_INVITATION)
    {
        Database::Output invitation = Database::db_query(database, "SELECT invited_by, creation_time FROM invitations WHERE user_id = '" + executer_user_id + "' AND nation_id = '" + nation_id + "' LIMIT 1");

        if (invitation.size() == 0)
        {
            event.reply(dpp::message(":prohibited: You need an invitation to join the " + government_type + " of " + nation_name + ".").set_flags(dpp::m_ephemeral));
            return;
        }

        ///////// c. /////////
        const int64_t invitation_time = std::stoll(invitation[0]["creation_time"]);
        const int expiration_delay = 86400; // 24 hours.

        if (invitation_time + expiration_delay < std::stoll(now))
        {
            Database::db_query(database, "DELETE FROM invitations WHERE user_id = '" + executer_user_id + "' AND nation_id = '" + nation_id + "'");
            return event.reply(dpp::message(":prohibited: Your invitation to join the " + government_type + " of " + nation_name + " has expired.").set_flags(dpp::m_ephemeral));
        }

        ///////// d. /////////
        Database::db_query(database, "INSERT INTO nationality (user_id, nation_id, rank, last_rank_update, joining_time) VALUES ('" + executer_user_id + "', '" + nation_id + "', '" + citizen + "', '" + now + "', '" + now + "')");
        event.reply(dpp::message(":hammer_pick: You are now a citizen of the " + government_type + " of " + nation_name + ".").set_flags(dpp::m_ephemeral));

        ///////// e. /////////
        inviter_user_id = invitation[0]["invited_by"];
        Database::Output inviter_credentials = Database::db_query(database, "SELECT discord_id FROM players WHERE user_id = '" + inviter_user_id + "' LIMIT 1");

        if (inviter_credentials.size() != 0)
            inviter_discord_id = inviter_credentials[0]["discord_id"];
    }

    ///////// f. /////////
    const dpp::snowflake guild_id = event.command.guild_id;
    const dpp::snowflake nation_role_id = dpp::snowflake(nation[0]["nations.role_id"]);

    bot.guild_member_add_role(guild_id, executer_discord_id, nation_role_id, [nation_role_id, executer_discord_id](const dpp::confirmation_callback_t &callback)
    {
        if (callback.is_error())
            Logs::log("Warning: Failed to give role " + std::to_string(nation_role_id) + " to " + std::to_string(executer_discord_id) + " with error " + callback.get_error().human_readable + " -> /nation join.");
    });

    ////////////////// 3) //////////////////
    ///////// a. /////////
    Database::Output config = Database::db_query(database, "SELECT world_channel FROM config LIMIT 1");

    if (config.size() == 0)
    {
        Logs::log("Warning: No config for " + std::to_string(guild_id) + " -> /nation join.");
        return;
    }

    const dpp::snowflake world_channel = dpp::snowflake(config[0]["world_channel"]);

    ///////// b. /////////
    std::string was_invited_text;

    if (join_condition == ON_INVITATION)
    {
        Database::Output inviter_nationality = Database::db_query(database, "SELECT rank FROM nationalities JOIN WHERE user_id = '" + inviter_user_id + "' AND nation_id = '" + nation_id + "' LIMIT 1");

        if (inviter_nationality.size() != 0)
        {
            const std::string rank = Text::Nations::get_rank(std::stoi(inviter_nationality[0]["rank"]));
            was_invited_text = " They were invited by the " + rank + " <@" + inviter_discord_id + "> of " + nation_name + ".";
        }
    }

    ///////// c. /////////
    const dpp::embed embed = dpp::embed()
    .set_color(dpp::colors::light_green)
    .set_title("New Citizen")
    .set_description("<@" + std::to_string(executer_discord_id) + "> just received their citizenship from the " + government_type + " of " + nation_name + "." + was_invited_text);

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
