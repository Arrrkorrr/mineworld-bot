#include "nation.hpp"

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
#include <mysql/mysql.h>
#include <string>

/*
    Change the rank of a member of a nation.

    Tasks:
        1) Some verification.
            a. Verify that the executer has a MineWorld account linked to their Discord account.
            b. Verify that the executer are not trying to change their own rank.
            c. Verify that the target has a MineWorld account linked to their Discord account.
            d. Verify that the new rank is valid.
            e. Verify that the executer is not stateless.
            f. Get some information about the executer nation.
            g. Verify that the target is not stateless.
            h. Verify that the executer and target are in the same nation.
        2) Compare ranks.
            a. Verify that the target does not already have the rank.
            b. Verify that the executer are not giving their own rank, except if it's Head of State.
            c. Verify that the executer has a higher rank than the new rank.
            d. Verify that the executer and target do not share the same rank.
            e. Verify that the executer has a higher rank than the target.
        3) Process rank modification request.
            a. Update target rank.
            b. If new rank is Head of State, update statistics and demote the executer to Minister.
            c. If it's a government change, update statistics.
            d. If new rank is Prime Minister, demote the current Prime Minister to Minister.
        4) Notify the world.
            a. Get bot configuration.
            b. Generate embed depending on the new rank given.
            c. Send the notification embed in the world channel.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster              / Client of the bot with all related information.
        - database  / MYSQL*                    / MineWorld database.
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
    const std::string executer_discord_id = std::to_string(event.command.usr.id);
    Database::Output executer_credentials = Database::db_query(database, "SELECT user_id FROM players WHERE discord_id = '" + executer_discord_id + "' LIMIT 1");

    if (executer_credentials.size() == 0)
    {
        event.reply(dpp::message(":prohibited: No MineWorld account is linked to your Discord account. Create a MineWorld account using `/account register` or link your already existing account with `/account discord`.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// b. /////////
    const std::string target_discord_id = std::to_string(std::get<dpp::snowflake>(event.get_parameter("member")));

    if (executer_discord_id == target_discord_id)
    {
        event.reply(dpp::message(":prohibited: You can not kick yourself. To leave your nation, run `/nation leave`.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// c. /////////
    Database::Output target_credentials = Database::db_query(database, "SELECT user_id FROM players WHERE discord_id = '" + target_discord_id + "' LIMIT 1");

    if (target_credentials.size() == 0)
    {
        event.reply(dpp::message(":prohibited: <@" + target_discord_id + "> does not have a MineWorld account linked to their Discord account.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// d. /////////
    const int64_t new_rank = std::get<int64_t>(event.get_parameter("new_rank"));

    if (new_rank < CITIZEN || new_rank > HEAD_OF_STATE)
    {
        event.reply(dpp::message(":prohibited: Invalid new rank `" + std::to_string(new_rank) + "` provided.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// e. /////////
    const std::string executer_user_id = executer_credentials[0]["user_id"];
    Database::Output executer_nationality = Database::db_query(database, "SELECT nation_id, rank FROM nationalities WHERE user_id = '" + executer_user_id + "' LIMIT 1");

    if (executer_nationality.size() == 0)
    {
        event.reply(dpp::message(":prohibited: You can not perform this action while being stateless.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// f. /////////
    const std::string executer_nation_id = executer_nationality[0]["nation_id"];
    Database::Output executer_nation = Database::db_query(database, "SELECT nations.display_name, governments.government_type, governments.leader_count, governments.government_count FROM nations JOIN governments ON governments.nation_id = nations.nation_id WHERE nation_id = '" + executer_nation_id + "' AND nations.deleted = '" + std::to_string(EXISTING_NATION) + "' LIMIT 1");

    if (executer_nation.size() == 0)
    {
        Logs::log("Warning: Nation ID " + executer_nation_id + " missing in database -> /nation leave.");
        return event.reply(dpp::message(":prohibited: Your nation `" + executer_nation_id + "` no longer exists.").set_flags(dpp::m_ephemeral));
    }

    ///////// g. /////////
    const std::string target_user_id = target_credentials[0]["user_id"];
    Database::Output target_nationality = Database::db_query(database, "SELECT nation_id, rank FROM nationalities WHERE user_id = '" + target_user_id + "' LIMIT 1");

    const std::string executer_nation_name = executer_nation[0]["nations.display_name"];
    const std::string executer_nation_government_type = Text::get_government_type(std::stoi(executer_nation[0]["governments.government_type"]));
    const std::string new_rank_name = Text::get_rank(new_rank);

    if (target_nationality.size() == 0)
    {
        event.reply(dpp::message(":prohibited: You can not set the rank of <@" + target_discord_id + "> to " + new_rank_name + " of the " + executer_nation_government_type + " of " + executer_nation_name + " as they are stateless.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// h. /////////
    const std::string target_nation_id = target_nationality[0]["nation_id"];
    const int target_rank = std::stoi(target_nationality[0]["rank"]);
    const std::string target_rank_name = Text::get_rank(target_rank);

    if (target_nation_id != executer_nation_id)
    {
        Database::Output target_nation = Database::db_query(database, "SELECT nations.display_name, governments.government_type FROM nations JOIN governments ON governments.nation_id = nations.nation_id WHERE nation_id = '" + target_nation_id + "' LIMIT 1");

        if (target_nation.size() == 0)
        {
            Logs::log("Warning: Nation ID " + target_nation_id + " missing in database -> /nation rank.");
            return event.reply(dpp::message(":prohibited: You can not set the rank of <@" + target_discord_id + "> to " + new_rank_name + " of the " + executer_nation_government_type + " of " + executer_nation_name + " as they are part of another nation as " + target_rank_name + ".").set_flags(dpp::m_ephemeral));
        }

        const std::string target_nation_name = target_nation[0]["nations.display_name"];
        const std::string target_nation_government_type = target_nation[0]["governments.government_type"];

        return event.reply(dpp::message(":prohibited: You can not set the rank of <@" + target_discord_id + "> to " + new_rank_name + " of the " + executer_nation_government_type + " of " + executer_nation_name + " as they are part of " + target_nation_name + " as " + target_rank_name + ".").set_flags(dpp::m_ephemeral));
    }

    ////////////////// 2) //////////////////
    ///////// a. /////////
    if (target_rank == new_rank)
    {
        event.reply(dpp::message(":prohibited: <@" + target_discord_id + "> already holds the " + new_rank_name + " rank.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// b. /////////
    const int executer_rank = std::stoi(executer_nationality[0]["rank"]);

    if (new_rank == executer_rank && new_rank != HEAD_OF_STATE)
    {
        event.reply(dpp::message(":prohibited: You can not set the rank of " + target_rank_name + " <@" + target_discord_id + "> to " + new_rank_name + " of the " + executer_nation_government_type + " of " + executer_nation_name + " as this rank is your current rank.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// c. /////////
    const std::string executer_rank_name = Text::get_rank(executer_rank);

    if (new_rank > executer_rank)
    {
        event.reply(dpp::message(":prohibited: You can not set the rank of " + target_rank_name + " <@" + target_discord_id + "> to " + new_rank_name + " of the " + executer_nation_government_type + " of " + executer_nation_name + " as the new rank is higher than yours (" + new_rank_name + " > " + executer_rank_name + ").").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// d. /////////
    if (target_rank == executer_rank)
    {
        event.reply(dpp::message(":prohibited: You can not set the rank of " + target_rank_name + " <@" + target_discord_id + "> to " + new_rank_name + " of the " + executer_nation_government_type + " of " + executer_nation_name + " as you both share the " + executer_rank_name + ".").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// e. /////////
    if (target_rank > executer_rank)
    {
        event.reply(dpp::message(":prohibited: You can not set the rank of " + target_rank_name +  " <@" + target_discord_id + "> to " + new_rank_name + " of the " + executer_nation_government_type + " of " + executer_nation_name + " as they have a higher rank than you (" + target_rank_name + " > " + executer_rank_name + ").").set_flags(dpp::m_ephemeral));
        return;
    }

    ////////////////// 3) //////////////////
    ///////// a. /////////
    Database::db_query(database, "UPDATE nationalities SET rank = '" + std::to_string(new_rank) + "' WHERE user_id = '" + target_user_id + "'");

    const bool is_promotion = (new_rank > target_rank);
    const std::string emoji = (is_promotion ? ":arrow_up:" : ":arrow_down:");
    const std::string verb = (is_promotion ? "promoted" : "demoted");

    event.reply(dpp::message(emoji + " <@" + target_discord_id + "> has been " + verb + " from " + target_rank_name + " to " + new_rank_name + " of the " + executer_nation_government_type + " of " + executer_nation_name + ".").set_flags(dpp::m_ephemeral));

    ///////// b. /////////
    const std::string new_leader_count = std::to_string(std::stoi(executer_nation[0]["governments.leader_count"]) + 1);
    const std::string now = std::to_string(Miscellaneous::get_current_timestamp());

    if (new_rank == HEAD_OF_STATE)
    {
        Database::db_query(database, "UPDATE governments SET leader_count = '" + new_leader_count + "', last_leader_change = '" + now + "' WHERE nation_id = '" + executer_nation_id + "'");
        Database::db_query(database, "UPDATE nationalities SET rank = '" + std::to_string(MINISTER) + "' WHERE nation_id = '" + executer_nation_id + "' AND rank = '" + std::to_string(HEAD_OF_STATE) + "'");

        Logs::log("Warning: Automatically demoted current Head of State of " + executer_nation_name + " -> /nation rank.");
    }

    ///////// c. /////////
    const bool is_government_change = ( (target_rank >= MINISTER && target_rank != HEAD_OF_STATE) || (new_rank >= MINISTER && new_rank != HEAD_OF_STATE));
    const std::string new_government_count = std::to_string(std::stoi(executer_nation[0]["governments.government_count"]) + 1);

    if (is_government_change)
    {
        Database::db_query(database, "UPDATE governments SET government_count = '" + new_government_count + "', last_government_change = '" + now + "' WHERE nation_id = '" + executer_nation_id + "'");

        ///////// d. /////////
        if (new_rank == PRIME_MINISTER)
        {
            Database::db_query(database, "UPDATE nationalities SET rank = '" + std::to_string(MINISTER) + "' WHERE nation_id = '" + executer_nation_id + "' AND rank = '" + std::to_string(PRIME_MINISTER) + "'");
            Logs::log("Warning: Automatically demoted current Prime Minister of " + executer_nation_name + " -> /nation rank.");
        }
    }

    ////////////////// 4) //////////////////
    ///////// a. /////////
    Database::Output config = Database::db_query(database, "SELECT world_channel FROM config LIMIT 1");

    if (config.size() == 0)
    {
        Logs::log("Warning: No config data -> /nation rank.");
        return;
    }

    ///////// b. /////////
    const dpp::snowflake guild_id = event.command.guild_id;
    const dpp::snowflake world_channel = dpp::snowflake(config[0]["world_channel"]);

    dpp::embed embed = dpp::embed();

    if (new_rank == HEAD_OF_STATE)
    {
        embed.set_color(dpp::colors::gold)
        .set_title("Leadership Change")
        .set_description("The Head of State <@" + executer_discord_id + "> of the " + executer_nation_government_type + " of " + executer_nation_name + " resigned from their functions and named the " + target_rank_name + " <@" + target_discord_id + "> as their successor.");
    }
    else if (new_rank == PRIME_MINISTER)
    {
        embed.set_color(dpp::colors::gold)
        .set_title("Prime Minister Change")
        .set_description("The Head of State <@" + executer_discord_id + "> named the " + target_rank_name + " <@" + target_discord_id + "> as the new Prime Minister of the " + executer_nation_government_type + " of " + executer_nation_name + ".");
    }
    else
    {
        const uint32_t color = (is_promotion ? dpp::colors::light_green : dpp::colors::red);

        embed.set_color(color)
        .set_title("Rank Modification")
        .set_description("The " + executer_rank_name + " <@" + executer_discord_id + "> just " + verb + " the " + target_rank_name + " <@" + target_discord_id + "> to " + new_rank_name + " of the " + executer_nation_government_type + " of " + executer_nation_name + ".");
    }

    ///////// c. /////////
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
