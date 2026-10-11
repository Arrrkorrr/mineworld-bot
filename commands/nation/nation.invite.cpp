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
    Invite a player into a nation.

    Tasks:
        1) We start with some basic verification.
            a. Compare the user ID provided and the ID of the command executer. We deny the request if the IDs match.
            b. Try to get some information about the executer in the "nationality" table. If we get no data back, it likely means that the executer is stateless.
            c. Try to get some information about the executer nation.
            d. Verify that the nation was not set as "closed" or "opened" by the government.
        2) Process the invitation request.
            a. Verify that executer meets permission requirements to send invitations according to nation government config.
            b. Check that the user do not already have a pending invitation. If we find one, we check that it did not expire.
            c. Register the new pending invitation.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster              / Client of the bot with all related information.
        - database  / MYSQL*                    / MineWorld database
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
    const dpp::snowflake user_id = std::get<dpp::snowflake>(event.get_parameter("player"));
    const dpp::snowflake executer_id = event.command.usr.id;

    if (user_id == executer_id)
    {
        event.reply(dpp::message(":prohibited: You can not send an invitation to yourself.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// b. /////////
    Database::Output nationality = Database::db_query(database, "SELECT nation_id, rank FROM nationality WHERE user_id = '" + std::to_string(executer_id) + "' LIMIT 1");

    if (nationality.size() == 0)
    {
        event.reply(dpp::message(":prohibited: You can not send an invitation to anyone while being stateless.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// c. /////////
    const std::string nation_id = nationality[0]["nation_id"];
    Database::Output nations = Database::db_query(database, "SELECT display_name, government_type, join_condition, invite_permission FROM nations WHERE nation_id = '" + nation_id + "' LIMIT 1");

    if (nations.size() == 0)
    {
        Logs::log("Warning: Nation ID " + nation_id + " missing in database -> /nation invite.");
        return event.reply(dpp::message(":prohibited: Something went wrong while retrieving information about nation `" + nation_id + "`.").set_flags(dpp::m_ephemeral));
    }

    ///////// d. /////////
    const std::string government_type = Text::get_government_type(stoi(nations[0]["government_type"]));
    const int join_condition = std::stoi(nations[0]["join_condition"]);
    const std::string display_name = nations[0]["display_name"];

    if (join_condition == OPENED)
    {
        event.reply(dpp::message(":prohibited: The " + government_type + " of " + display_name + " is already opened for anyone to join.").set_flags(dpp::m_ephemeral));
        return;
    }

    if (join_condition == CLOSED)
    {
        event.reply(dpp::message(":prohibited: The " + government_type + " of " + display_name + " is currently closed to anyone.").set_flags(dpp::m_ephemeral));
        return;
    }

    ////////////////// 2) //////////////////
    ///////// a. /////////
    const int permission = std::stoi(nations[0]["invite_permission"]);
    const int rank = std::stoi(nationality[0]["rank"]);

    if ((permission == GOV_ONLY && rank < MINISTER) || (permission == PM_MINIMUM && rank < PRIME_MINISTER) || (permission == LEADER_ONLY && rank < LEADER))
    {
        event.reply(dpp::message(":prohibited: You are not allowed to send invitations due to current immigration laws imposed by your government.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// b. /////////
    Database::Output invitation = Database::db_query(database, "SELECT creation_time FROM invitations WHERE user_id = '" + std::to_string(user_id) + "' AND nation_id = '" + nation_id + "' LIMIT 1");
    const int64_t now = Miscellaneous::get_current_timestamp();

    if (invitation.size() != 0)
    {
        const int64_t invitation_time = std::stoll(invitation[0]["creation_time"]);
        const int expiration = 86400; // 24 hours.

        if (invitation_time + expiration > now)
        {
            event.reply(dpp::message(":prohibited: <@" + std::to_string(user_id) + "> already has a pending invitation to join the " + government_type + " of " + display_name + ".").set_flags(dpp::m_ephemeral));
            return;
        }
        else Database::db_query(database, "DELETE FROM invitations WHERE user_id = '" + std::to_string(user_id) + "' AND nation_id = '" + nation_id + "'");
    }

    ///////// c. /////////
    Database::db_query(database, "INSERT INTO invitations (user_id, nation_id, invited_by, creation_time) VALUES ('" + std::to_string(user_id) + "', '" + nation_id + "', '" + std::to_string(executer_id) + "', '" + std::to_string(now) + "')");
    event.reply(dpp::message(":envelope: An invitation is now pending for <@" + std::to_string(user_id) + "> to join the " + government_type + " of " + display_name + ".\n:warning: They have 24 hours to run `/nation join` before it expires.").set_flags(dpp::m_ephemeral));
}
