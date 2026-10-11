#include "nation.hpp"

#include "../../config/enumerations.hpp"
#include "../../utils/database/database.hpp"
#include "../../utils/logs/logs.hpp"
#include "../../utils/text/text.hpp"

#include <dpp/dpp.h>
#include <mysql/mysql.h>
#include <string>

/*
    Kick a member from a nation.

    Tasks:
        1) We start with some basic verification.
            a. Compare the user ID provided and the ID of the command executer. We deny the request if the IDs match.
            b. Try to get some information about the executer in the "nationality" table. If we get no data back, it likely means that the executer is stateless.
            c. Try to get some information about the executer nation.
            d. Try to get some information about the user in the "nationality" table. If we get no data back, it likely means that the user is stateless.
            e. Check that both users are in the same nation by comparing nation IDs. If it does not match, we query some information about user nation for a cleaner error message.
            f. Verify that executer has a rank that is not CITIZEN or MILITARY to grant the permission to kick.
            g. Compare both user ranks, if they match, we deny the kick request.
            h. Compare both user ranks, if user rank is higher than executer rank, we deny the kick request. We do two separated checks for a more accurate error message.
        2) Process the kick request.
            a. Remove the user from the "nationality" table.
            b. Get nation role ID and try to remove it from the user.
            c. Determine the role to remove depending on user rank, and get some required IDs from the "config" table.
            d. Check that the "gossip" channel and role are valid, and send an embed notifying other players of the kick.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster              / Client of the bot with all related information.
        - database  / MYSQL*                    / MineWorld database
        - event     / dpp::interaction_create_t / All information about the event.

    Returns (type + description):
        No object returned.
*/
void Nation::nation_kick
(
    dpp::cluster                    &bot,
    MYSQL*                          &database,
    const dpp::interaction_create_t &event
)
{
    ////////////////// 1) //////////////////
    ///////// a. /////////
    const dpp::snowflake user_id = std::get<dpp::snowflake>(event.get_parameter("member"));
    const dpp::snowflake executer_id = event.command.usr.id;

    if (executer_id == user_id)
    {
        event.reply(dpp::message(":prohibited: You can not kick yourself. To leave a nation, run `/nation leave`.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// b. /////////
    Database::Output executer_nationality = Database::db_query(database, "SELECT nation_id, rank FROM nationality WHERE user_id = '" + std::to_string(executer_id) + "' LIMIT 1");

    if (executer_nationality.size() == 0)
    {
        event.reply(dpp::message(":prohibited: You can not perform this action while being stateless.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// c. /////////
    const std::string executer_nation_id = executer_nationality[0]["nation_id"];
    Database::Output nations = Database::db_query(database, "SELECT display_name, government_type, role_id FROM nations WHERE nation_id = '" + executer_nation_id + "' LIMIT 1");

    if (nations.size() == 0)
    {
        Logs::log("Warning: Nation ID " + executer_nation_id + " missing in database -> /nation kick.");
        return event.reply(dpp::message(":prohibited: Something went wrong while retrieving information about nation `" + executer_nation_id + "`.").set_flags(dpp::m_ephemeral));
    }

    ///////// d. /////////
    Database::Output user_nationality = Database::db_query(database, "SELECT nation_id, rank FROM nationality WHERE user_id = '" + std::to_string(user_id) + "' LIMIT 1");

    const std::string display_name = nations[0]["display_name"];
    const std::string government_type = Text::get_government_type(std::stoi(nations[0]["government_type"]));

    if (user_nationality.size() == 0)
    {
        event.reply(dpp::message(":prohibited: You can not kick <@" + std::to_string(user_id) + "> out of the " + government_type + " of " + display_name + " as they are stateless.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// e. /////////
    const std::string user_nation_id = user_nationality[0]["nation_id"];
    const int user_rank = std::stoi(user_nationality[0]["rank"]);
    const std::string user_rank_name = Text::get_rank(user_rank);

    if (user_nation_id != executer_nation_id)
    {
        Database::Output user_nation = Database::db_query(database, "SELECT display_name, government_type FROM nations WHERE nation_id = '" + user_nation_id + "' LIMIT 1");

        if (user_nation.size() == 0)
        {
            Logs::log("Warning: Nation ID " + user_nation_id + " missing in database -> /nation kick.");
            return event.reply(dpp::message(":prohibited: You can not kick <@" + std::to_string(user_id) + "> as they are not part of the " + government_type + " of " + display_name + ".").set_flags(dpp::m_ephemeral));
        }

        const std::string user_nation_name = user_nation[0]["display_name"];
        const std::string user_gov_type = Text::get_government_type(std::stoi(user_nation[0]["government_type"]));

        return event.reply(dpp::message(":prohibited: You can not kick <@" + std::to_string(user_id) + "> out of the " + government_type + " of " + display_name + " as they are part of " + user_gov_type + " " + user_nation_name + ".").set_flags(dpp::m_ephemeral));
    }

    ///////// f. /////////
    const int executer_rank = std::stoi(executer_nationality[0]["rank"]);
    const std::string executer_rank_name = Text::get_rank(executer_rank);

    if (executer_rank == CITIZEN || executer_rank == MILITARY)
    {
        event.reply(dpp::message(":prohibited: As a " + executer_rank_name + ", you are not allowed to kick any member from the " + government_type + " of " + display_name + ".").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// g. /////////
    if (user_rank == executer_rank)
    {
        event.reply(dpp::message(":prohibited: You can not kick " + user_rank_name + " <@" + std::to_string(user_id) + "> out of the " + government_type + " of " + display_name + " as you share the same rank.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// h. /////////
    if (user_rank > executer_rank)
    {
        event.reply(dpp::message(":prohibited: You can not kick " + user_rank_name + " <@" + std::to_string(user_id) + "> out of the " + government_type + " of " + display_name + " as they have a higher rank than you (" + user_rank_name + " > " + executer_rank_name + ").").set_flags(dpp::m_ephemeral));
        return;
    }

    ////////////////// 2) //////////////////
    ///////// a. /////////
    Database::db_query(database, "DELETE FROM nationality WHERE user_id = '" + std::to_string(user_id) + "'");
    event.reply(dpp::message(":hammer: <@" + std::to_string(user_id) + "> has been kicked from the " + government_type + " of " + display_name + ".").set_flags(dpp::m_ephemeral));

    ///////// b. /////////
    const dpp::snowflake guild_id = event.command.guild_id;
    const dpp::snowflake role_id = dpp::snowflake(nations[0]["role_id"]);

    bot.guild_member_remove_role(guild_id, user_id, role_id, [role_id, user_id](const dpp::confirmation_callback_t &callback)
    {
        if (callback.is_error())
            Logs::log("Warning: Failed to remove role " + std::to_string(role_id) + " to " + std::to_string(user_id) + " with error " + callback.get_error().human_readable + " -> /nation kick.");
    });

    ///////// c. /////////
    Database::Output config = Database::db_query(database, "SELECT world_channel FROM config LIMIT 1");

    if (config.size() == 0)
    {
        Logs::log("Warning: No config data -> /nation kick.");
        return;
    }

    const dpp::snowflake world_channel = dpp::snowflake(config[0]["world_channel"]);

    ///////// d. /////////
    const dpp::embed embed = dpp::embed()
    .set_color(dpp::colors::red)
    .set_title("Citizenship Removal")
    .set_description(user_rank_name + " <@" + std::to_string(user_id) + "> was kicked out of the " + government_type + " of " + display_name + " by " + executer_rank_name + " <@" + std::to_string(executer_id) + ">.");

    bot.message_create
    (
        dpp::message(world_channel, "").add_embed(embed),
        [world_channel](const dpp::confirmation_callback_t &callback)
        {
            if (callback.is_error())
                Logs::log("Warning: Failed to send message in " + std::to_string(world_channel) + " with error " + callback.get_error().human_readable + " -> /nation kick.");
        }
    );
}
