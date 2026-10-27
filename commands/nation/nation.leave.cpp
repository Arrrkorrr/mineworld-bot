#include "nation.hpp"

#include "../../enum/nations.hpp"
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
    Leave a nation.

    Tasks:
        1) Some verification.
            a. Verify that the executer has a MineWorld account linked to their Discord account.
            b. Verify that the executer is not stateless.
            c. Get some information about the nation.
            d. Remove the citizenship to the executer in case the nation is no longer existing.
            e. Verify that the executer is not the Head of State in a populated nation.
        2) Process leaving request.
            a. Remove the citizenship to the executer.
            b. Remove the nation role to the executer.
            c. Get bot configuration.
            d. Send a notification embed in the world channel.

    Parameters (variable_name / type / description):
        - bot      / dpp::cluster              / Client of the bot with all related information.
        - database / MYSQL*                    / MineWorld database.
        - event    / dpp::interaction_create_t / All information about the event.

    Returns (type + description):
        No object returned.
*/
void Nation::leave_nation
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
    const std::string executer_user_id = executer_credentials[0]["user_id"];
    Database::Output executer_nationality = Database::db_query(database, "SELECT nation_id, rank FROM nationalities WHERE user_id = '" + executer_user_id + "' LIMIT 1");

    if (executer_nationality.size() == 0)
    {
        event.reply(dpp::message(":prohibited: You can not perform this action white being stateless.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// c. /////////
    const std::string executer_nation_id = executer_nationality[0]["nation_id"];
    Database::Output executer_nation = Database::db_query(database, "SELECT nations.display_name, nations.deleted, nations.role_id, governments.government_type FROM nations JOIN governments ON governments.nation_id = nations.nation_id WHERE nation_id = '" + executer_nation_id + "' LIMIT 1");

    if (executer_nation.size() == 0)
    {
        Logs::log("Warning: Nation ID " + executer_nation_id + " missing in database -> /nation leave.");
        return event.reply(dpp::message(":prohibited: Something went wrong while retrieving information about nation `" + executer_nation_id + "`.").set_flags(dpp::m_ephemeral));
    }

    ///////// d. /////////
    const std::string executer_nation_government_type = Text::Nations::get_government_type(std::stoi(executer_nation[0]["governments.government_type"]));
    const std::string executer_nation_name = executer_nation[0]["nations.display_name"];
    const int executer_nation_status = std::stoi(executer_nation[0]["nations.deleted"]);
    const int executer_rank = std::stoi(executer_nation[0]["rank"]);
    const std::string executer_rank_name = Text::Nations::get_rank(executer_rank);

    const bool is_invalid_nation = (executer_nation_status == COLLAPSED_NATION || executer_nation_status == NATION_REMOVED_MODERATION);

    if (is_invalid_nation)
    {
        Database::db_query(database, "DELETE FROM nationalities WHERE user_id = '" + executer_user_id + "'");
        return event.reply(dpp::message(":wave: You left the " + executer_nation_government_type + " of " + executer_nation_name + " and lost your " + executer_rank_name + " rank.").set_flags(dpp::m_ephemeral));
    }

    ///////// e. /////////
    Database::Output executer_nation_members = Database::db_query(database, "SELECT 1 FROM nationality WHERE nation_id = '" + executer_nation_id + "'");
    const int executer_nation_member_count = executer_nation_members.size();

    if (executer_rank == HEAD_OF_STATE && executer_nation_member_count > 1)
    {
        event.reply(dpp::message(":prohibited: You can not leave the " + executer_nation_government_type + " of " + executer_nation_name + " as the Head of State as there are " + std::to_string(executer_nation_member_count - 1) + " other member(s). Either transfer ownership using `/nation rank` or remove all other members using `/nation kick` in order to leave.").set_flags(dpp::m_ephemeral));
        return;
    }

    ////////////////// 2) //////////////////
    ///////// a. /////////
    Database::db_query(database, "DELETE FROM nationalities WHERE user_id = '" + executer_user_id + "'");
    event.reply(dpp::message(":wave: You left the " + executer_nation_government_type + " of " + executer_nation_name + " and lost your " + executer_rank_name + " rank.").set_flags(dpp::m_ephemeral));

    ///////// b. /////////
    const dpp::snowflake guild_id = event.command.guild_id;
    const dpp::snowflake nation_role_id = dpp::snowflake(executer_nation[0]["role_id"]);

    bot.guild_member_remove_role(guild_id, executer_discord_id, nation_role_id, [nation_role_id, executer_discord_id](const dpp::confirmation_callback_t &callback)
    {
        if (callback.is_error())
            Logs::log("Warning: Failed to remove role " + std::to_string(nation_role_id) + " to " + std::to_string(executer_discord_id) + " with error " + callback.get_error().human_readable + " -> /nation leave.");
    });

    ///////// c. /////////
    Database::Output config = Database::db_query(database, "SELECT world_channel FROM config LIMIT 1");

    if (config.size() == 0)
    {
        Logs::log("Warning: No config data -> /nation leave.");
        return;
    }

    ///////// d. /////////
    const dpp::snowflake world_channel = dpp::snowflake(config[0]["world_channel"]);

    const dpp::embed embed = dpp::embed()
    .set_color(dpp::colors::red)
    .set_title("Citizenship Renounced")
    .set_description("The " + executer_rank_name + " <@" + std::to_string(executer_discord_id) + "> just left the " + executer_nation_government_type + " of " + executer_nation_name + " and are now stateless.");

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
