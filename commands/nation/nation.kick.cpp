#include "nation.hpp"

#include "../../enum/ranks.hpp"
#include "../../utils/database/database.hpp"
#include "../../utils/logs/logs.hpp"
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
    Kick a member from a nation.

    Tasks:
        1) Some verification.
            a. Verify that the executer has a MineWorld account linked to their Discord account.
            b. Verify that the executer are not trying to kick themselves.
            c. Verify that the target has a MineWorld account linked to their Discord account.
            d. Verify that the executer is not stateless.
            e. Get some information about the executer nation.
            f. Verify that the target is not stateless.
            g. Verify that the executer and target are in the same nation.
        2) Compare ranks.
            a. Verify that the executer has one of the required ranks to kick.
            b. Verify that the executer and target do not share the same rank.
            c. Verify that the executer has a higher rank than the target.
        3) Process kick request.
            a. Kick the target from the nation.
            b. Remove the nation role off the target.
            c. Get bot configuration.
            d. Send a notification embed in the world channel.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster              / Client of the bot with all related information.
        - database  / MYSQL*                    / MineWorld database.
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
    const dpp::snowflake executer_discord_id = event.command.usr.id;
    Database::Output executer_credentials = Database::db_query(database, "SELECT user_id FROM players WHERE discord_id = '" + std::to_string(executer_discord_id) + "' LIMIT 1");

    if (executer_credentials.size() == 0)
    {
        event.reply(dpp::message(":prohibited: No MineWorld account is linked to your Discord account. Create a MineWorld account using `/account register` or link your already existing account with `/account discord`.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// b. /////////
    const dpp::snowflake target_discord_id = std::get<dpp::snowflake>(event.get_parameter("member"));

    if (executer_discord_id == target_discord_id)
    {
        event.reply(dpp::message(":prohibited: You can not kick yourself. To leave your nation, run `/nation leave`.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// c. /////////
    Database::Output target_credentials = Database::db_query(database, "SELECT user_id FROM players WHERE discord_id = '" + std::to_string(target_discord_id) + "' LIMIT 1");

    if (target_credentials.size() == 0)
    {
        event.reply(dpp::message(":prohibited: <@" + std::to_string(target_discord_id) + "> does not have a MineWorld account linked to their Discord account.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// d. /////////
    const std::string executer_user_id = executer_credentials[0]["user_id"];
    Database::Output executer_nationality = Database::db_query(database, "SELECT nation_id, rank FROM nationalities WHERE user_id = '" + executer_user_id + "' LIMIT 1");

    if (executer_nationality.size() == 0)
    {
        event.reply(dpp::message(":prohibited: You can not perform this action while being stateless.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// e. /////////
    const std::string executer_nation_id = executer_nationality[0]["nation_id"];
    Database::Output executer_nation = Database::db_query(database, "SELECT nations.display_name, nations.role_id governments.government_type FROM nations JOIN governments ON governments.nation_id = nations.nation_id WHERE nation_id = '" + executer_nation_id + "' LIMIT 1");

    if (executer_nation.size() == 0)
    {
        Logs::log("Warning: Nation ID " + executer_nation_id + " missing in database -> /nation kick.");
        return event.reply(dpp::message(":prohibited: Something went wrong while retrieving information about nation `" + executer_nation_id + "`.").set_flags(dpp::m_ephemeral));
    }

    ///////// f. /////////
    const std::string target_user_id = target_credentials[0]["user_id"];
    Database::Output target_nationality = Database::db_query(database, "SELECT nation_id, rank FROM nationalities WHERE user_id = '" + target_user_id + "' LIMIT 1");

    const std::string executer_nation_government_type = Text::get_government_type(std::stoi(executer_nation[0]["governments.government_type"]));
    const std::string executer_nation_name = executer_nation[0]["nations.display_name"];

    if (target_nationality.size() == 0)
    {
        event.reply(dpp::message(":prohibited: You can not kick <@" + std::to_string(target_discord_id) + "> out of the " + executer_nation_government_type + " of " + executer_nation_name + " as they are stateless.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// g. /////////
    const std::string target_nation_id = target_nationality[0]["nation_id"];

    if (target_nation_id != executer_nation_id)
    {
        Database::Output target_nation = Database::db_query(database, "SELECT display_name, government_type FROM nations WHERE nation_id = '" + target_nation_id + "' LIMIT 1");

        if (target_nation.size() == 0)
        {
            Logs::log("Warning: Nation ID " + target_nation_id + " missing in database -> /nation kick.");
            return event.reply(dpp::message(":prohibited: You can not kick <@" + std::to_string(target_discord_id) + "> as they are not part of the " + executer_nation_government_type + " of " + executer_nation_name + ".").set_flags(dpp::m_ephemeral));
        }

        const std::string target_nation_name = target_nation[0]["display_name"];
        const std::string target_nation_government_type = Text::get_government_type(std::stoi(target_nation[0]["government_type"]));

        return event.reply(dpp::message(":prohibited: You can not kick <@" + std::to_string(target_discord_id) + "> out of the " + executer_nation_government_type + " of " + executer_nation_name + " as they are part of " + target_nation_government_type + " " + target_nation_name + ".").set_flags(dpp::m_ephemeral));
    }

    ////////////////// 2) //////////////////
    ///////// a. /////////
    const int executer_rank = std::stoi(executer_nationality[0]["rank"]);
    const std::string executer_rank_name = Text::get_rank(executer_rank);

    if (executer_rank == CITIZEN || executer_rank == MILITARY)
    {
        event.reply(dpp::message(":prohibited: As a " + executer_rank_name + ", you are not allowed to kick any member from the " + executer_nation_government_type + " of " + executer_nation_name + ".").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// b. /////////
    const int target_rank = std::stoi(target_nationality[0]["rank"]);
    const std::string target_rank_name = Text::get_rank(target_rank);

    if (target_rank == executer_rank)
    {
        event.reply(dpp::message(":prohibited: You can not kick " + target_rank_name + " <@" + std::to_string(target_discord_id) + "> out of the " + executer_nation_government_type + " of " + executer_nation_name + " as you share the same rank.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// c. /////////
    if (target_rank > executer_rank)
    {
        event.reply(dpp::message(":prohibited: You can not kick " + target_rank_name + " <@" + std::to_string(target_discord_id) + "> out of the " + executer_nation_government_type + " of " + executer_nation_name + " as they have a higher rank than you (" + target_rank_name + " > " + executer_rank_name + ").").set_flags(dpp::m_ephemeral));
        return;
    }

    ////////////////// 3) //////////////////
    ///////// a. /////////
    Database::db_query(database, "DELETE FROM nationalities WHERE user_id = '" + target_user_id + "'");
    event.reply(dpp::message(":hammer: <@" + std::to_string(target_discord_id) + "> has been kicked from the " + executer_nation_government_type + " of " + executer_nation_name + ".").set_flags(dpp::m_ephemeral));

    ///////// b. /////////
    const dpp::snowflake guild_id = event.command.guild_id;
    const dpp::snowflake nation_role_id = dpp::snowflake(executer_nation[0]["nations.role_id"]);

    bot.guild_member_remove_role(guild_id, target_discord_id, nation_role_id, [nation_role_id, target_discord_id](const dpp::confirmation_callback_t &callback)
    {
        if (callback.is_error())
            Logs::log("Warning: Failed to remove role " + std::to_string(nation_role_id) + " to " + std::to_string(target_discord_id) + " with error " + callback.get_error().human_readable + " -> /nation kick.");
    });

    ///////// c. /////////
    Database::Output config = Database::db_query(database, "SELECT world_channel FROM config LIMIT 1");

    if (config.size() == 0)
    {
        Logs::log("Warning: No config data -> /nation kick.");
        return;
    }

    ///////// d. /////////
    const dpp::snowflake world_channel = dpp::snowflake(config[0]["world_channel"]);

    const dpp::embed embed = dpp::embed()
    .set_color(dpp::colors::red)
    .set_title("Citizenship Removal")
    .set_description("The " + target_rank_name + " <@" + std::to_string(target_discord_id) + "> was kicked out of the " + executer_nation_government_type + " of " + executer_nation_name + " by the " + executer_rank_name + " <@" + std::to_string(executer_discord_id) + ">.");

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
