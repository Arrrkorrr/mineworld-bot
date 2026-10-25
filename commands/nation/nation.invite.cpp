#include "nation.hpp"

#include "../../enum/immigration.hpp"
#include "../../enum/ranks.hpp"
#include "../../utils/database/database.hpp"
#include "../../utils/logs/logs.hpp"
#include "../../utils/miscellaneous/miscellaneous.hpp"
#include "../../utils/text/text.hpp"

#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <dpp/message.h>
#include <dpp/snowflake.h>
#include <mysql/mysql.h>
#include <string>

/*
    Invite a player into a nation.

    Tasks:
        1) Some verification.
            a. Verify that the executer has a MineWorld account linked to their Discord account.
            b. Verify that the executer and target Discord IDs do not match.
            c. Verify that the target has a MineWorld account linked to their Discord account.
            d. Verify that the executer is not stateless.
            e. Get information about the nation.
            f. Verify that the nation is not opened.
            g. Verify that the nation is not closed.
        2) Verify immigration laws.
            a. Verify that the executer is a government official if required.
            b. Verify that the executer is the Prime Minister or Head of State if required.
            c. Verify that the executer is the Head of State if required.
        3) Process invitation request.
            a. Check that the target do not already have a pending invitation. If there is one, verify that it did not expire.
            b. Register pending invitation.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster              / Client of the bot with all related information.
        - database  / MYSQL*                    / MineWorld database.
        - event     / dpp::interaction_create_t / All information about the event.

    Returns (type + description):
        No object returned.
*/
void Nation::nation_invite
(
    dpp::cluster                    &bot,
    MYSQL*                          &database,
    const dpp::interaction_create_t &event
)
{
    ////////////////// 1) //////////////////
    ///////// a. /////////
    const std::string executer_discord_id = std::to_string(event.command.usr.id);
    Database::Output executer_credentials = Database::db_query(database, "SELECT user_id FROM players WHERE discord_id = '" + executer_discord_id + "' LIMIT 1");

    if (executer_credentials.size() == 0)
    {
        event.reply(":prohibited: No MineWorld account is linked to your Discord account. Create a MineWorld account using `/account register` or link your already existing account with `/account discord`.");
        return;
    }

    ///////// b. /////////
    const std::string target_discord_id = std::to_string(std::get<dpp::snowflake>(event.get_parameter("player")));

    if (target_discord_id == executer_discord_id)
    {
        event.reply(dpp::message(":prohibited: You can not send an invitation to yourself.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// c. /////////
    Database::Output target_credentials = Database::db_query(database, "SELECT user_id FROM players WHERE discord_id = '" + target_discord_id + "' LIMIT 1");

    if (target_credentials.size() == 0)
    {
        event.reply(":prohibited: <@" + target_discord_id + "> does not have a MineWorld account linked to their Discord account.");
        return;
    }

    ///////// d. /////////
    const std::string executer_user_id = executer_credentials[0]["user_id"];
    Database::Output executer_nationality = Database::db_query(database, "SELECT nation_id, rank FROM nationality WHERE user_id = '" + executer_user_id + "' LIMIT 1");

    if (executer_nationality.size() == 0)
    {
        event.reply(dpp::message(":prohibited: You can not send an invitation to anyone while being stateless.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// e. /////////
    const std::string nation_id = executer_nationality[0]["nation_id"];
    const std::string query =
    (
        std::string("SELECT nations.display_name, governments.government_type, immigration.join_condition, immigration.invite_permission ") +
                    "FROM nations " +
                    "JOIN governments ON governments.nation_id = nations.nation_id " +
                    "JOIN immigration ON immigration.nation_id = nations.nation_id " +
                    "WHERE nations.nation_id = '" + nation_id + "' LIMIT 1"
    );

    Database::Output executer_nation = Database::db_query(database, query);

    if (executer_nation.size() == 0)
    {
        Logs::log("Warning: Nation ID " + nation_id + " missing in database -> /nation invite.");
        return event.reply(dpp::message(":prohibited: Something went wrong while retrieving information about nation `" + nation_id + "`.").set_flags(dpp::m_ephemeral));
    }

    ///////// f. /////////
    const std::string government_type_name = Text::get_government_type(stoi(executer_nation[0]["governments.government_type"]));
    const int join_condition = std::stoi(executer_nation[0]["immigration.join_condition"]);
    const std::string nation_name = executer_nation[0]["nations.display_name"];

    if (join_condition == OPENED)
    {
        event.reply(dpp::message(":prohibited: The " + government_type_name + " of " + nation_name + " is already opened for anyone to join.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// g. /////////
    if (join_condition == CLOSED)
    {
        event.reply(dpp::message(":prohibited: The " + government_type_name + " of " + nation_name + " is currently closed to anyone.").set_flags(dpp::m_ephemeral));
        return;
    }

    ////////////////// 2) //////////////////
    ///////// a. /////////
    const int permission = std::stoi(executer_nation[0]["immigration.invite_permission"]);
    const int rank = std::stoi(executer_nationality[0]["rank"]);

    if (permission == GOVERNMENT_ONLY && rank < MINISTER)
    {
        event.reply(dpp::message(":prohibited: Due to current immigration laws in the " + government_type_name + " of " + nation_name + ", only government officials may send invitations.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// b. /////////
    if (permission == PRIME_MINISTER_MINIMUM && rank < PRIME_MINISTER)
    {
        event.reply(dpp::message(":prohibited: Due to current immigration laws in the " + government_type_name + " of " + nation_name + ", only the Prime Minister and Head of State may send invitations.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// c. /////////
    if (permission == LEADER_ONLY && rank < HEAD_OF_STATE)
    {
        event.reply(dpp::message(":prohibited: Due to current immigration laws in the " + government_type_name + " of " + nation_name + ", only the Head of State may send invitations.").set_flags(dpp::m_ephemeral));
        return;
    }

    ////////////////// 3) //////////////////
    ///////// a. /////////
    const int64_t now = Miscellaneous::get_current_timestamp();
    const std::string target_user_id = target_credentials[0]["user_id"];

    Database::Output invitation = Database::db_query(database, "SELECT creation_time FROM invitations WHERE user_id = '" + target_user_id + "' AND nation_id = '" + nation_id + "' LIMIT 1");

    if (invitation.size() != 0)
    {
        const int64_t invitation_creation_time = std::stoll(invitation[0]["creation_time"]);
        const int expiration_delay = 86400; // 24 hours.

        if (invitation_creation_time + expiration_delay > now)
        {
            event.reply(dpp::message(":prohibited: <@" + target_discord_id + "> already has a pending invitation to join the " + government_type_name + " of " + nation_name + ".").set_flags(dpp::m_ephemeral));
            return;
        }
        else Database::db_query(database, "DELETE FROM invitations WHERE user_id = '" + target_user_id + "' AND nation_id = '" + nation_id + "'");
    }

    ///////// b. /////////
    Database::db_query(database, "INSERT INTO invitations VALUES ('" + target_user_id + "', '" + nation_id + "', '" + executer_user_id + "', '" + std::to_string(now) + "')");
    event.reply(dpp::message(":envelope: An invitation is now pending for <@" + target_discord_id + "> to join the " + government_type_name + " of " + nation_name + ".\n:warning: They have 24 hours to run `/nation join` before it expires.").set_flags(dpp::m_ephemeral));
}
