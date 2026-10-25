#include "nation.hpp"

#include "../../enum/ranks.hpp"
#include "../../utils/database/database.hpp"
#include "../../utils/logs/logs.hpp"
#include "../../utils/text/text.hpp"

#include <algorithm>
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
    Set a relation score with another nation.

    Tasks:
        1) Some verification.
            a. Verify that the executer has a MineWorld account linked to their Discord account.
            b. Verify that the executer is not stateless.
            c. Get information about the targeted nation.
            d. Get information about the executer nation.
            e. Verify that the executer nation ID and targeted nation ID do not match.
            f. Verify that the executer is part of the nation government.
        2) Process relation modification request.
            a. Get current score defined by the executer nation.
            b. Get current score defined by the targeted nation.
            c. Calculate current relation score and rating.
            d. Calculate new relation score and rating.
            e. Set new score in the database.
        3) Notify the world.
            a. Get bot configuration.
            b. Send a notification embed in the world channel.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster              / Client of the bot with all related information.
        - database  / MYSQL*                    / MineWorld database.
        - event     / dpp::interaction_create_t / All information about the event.

    Returns (type + description):
        No object returned.
*/
void Nation::nation_relation
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
    const std::string executer_user_id = executer_credentials[0]["user_id"];
    Database::Output executer_nationality = Database::db_query(database, "SELECT nation_id, rank FROM nationality WHERE user_id = '" + executer_user_id + "' LIMIT 1");

    if (executer_nationality.size() == 0)
    {
        event.reply(dpp::message(":prohibited: You can not perform this action while being stateless.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// c. /////////
    const std::string targeted_nation_id = Database::sanitize_input(database, std::to_string(std::get<int64_t>(event.get_parameter("nation_id"))));
    Database::Output targeted_nation = Database::db_query(database, "SELECT nations.display_name, governments.government_type FROM nations JOIN governments ON governments.nation_id = nations.nation_id WHERE nation_id = '" + targeted_nation_id + "' LIMIT 1");

    if (targeted_nation.size() == 0)
    {
        event.reply(dpp::message(":prohibited: Nation `" + targeted_nation_id + "` does not exist.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// d. /////////
    const std::string executer_nation_id = executer_nationality[0]["nation_id"];
    Database::Output executer_nation = Database::db_query(database, "SELECT nations.display_name, governments.government_type FROM nations JOIN governments ON governments.nation_id = nations.nation_id WHERE nation_id = '" + executer_nation_id + "' LIMIT 1");

    if (executer_nation.size() == 0)
    {
        Logs::log("Warning: Nation ID " + executer_nation_id + " missing in database -> /nation relation.");
        return event.reply(dpp::message(":prohibited: Something went wrong while retrieving information about nation `" + executer_nation_id + "`.").set_flags(dpp::m_ephemeral));
    }

    ///////// e. /////////
    const std::string executer_nation_name = executer_nationality[0]["nations.display_name"];
    const std::string executer_nation_government_type = Text::get_government_type(std::stoi(executer_nationality[0]["governments.government_type"]));

    if (executer_nation_id == targeted_nation_id)
    {
        event.reply(dpp::message(":prohibited: You can not change the relation of the " + executer_nation_government_type + " of " + executer_nation_name + " with itself.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// f. /////////
    const int executer_rank = std::stoi(executer_nationality[0]["rank"]);
    const std::string targeted_nation_name = targeted_nation[0]["nations.display_name"];
    const std::string targeted_nation_government_type = Text::get_government_type(std::stoi(targeted_nation[0]["governments.government_type"]));

    if (executer_rank < MINISTER)
    {
        event.reply(dpp::message(":prohibited: You must be a government official of the " + executer_nation_government_type + " of " + executer_nation_name + " to update the relation score with the " + targeted_nation_government_type + " of " + targeted_nation_name + ".").set_flags(dpp::m_ephemeral));
        return;
    }

    ////////////////// 2) //////////////////
    ///////// a. /////////
    int current_executer_nation_relation = 50;
    Database::Output executer_nation_score = Database::db_query(database, "SELECT score FROM relations WHERE defining_nation = '" + executer_nation_id + "' AND targeted_nation = '" + targeted_nation_id + "' LIMIT 1");

    if (executer_nation_score.size() != 0)
        current_executer_nation_relation = std::stoi(executer_nation_score[0]["score"]);

    ///////// b. /////////
    int current_targeted_nation_relation = 50;
    Database::Output targeted_nation_score = Database::db_query(database, "SELECT score FROM relations WHERE defining_nation = '" + targeted_nation_id + "' AND targeted_nation = '" + executer_nation_id + "' LIMIT 1");

    if (targeted_nation_score.size() != 0)
        current_targeted_nation_relation = std::stoi(targeted_nation_score[0]["score"]);

    ///////// c. /////////
    const int current_relation_score = (current_executer_nation_relation + current_targeted_nation_relation) / 2;
    const std::string current_relation_rating = Text::get_relation_rating(current_relation_score);

    ///////// d. /////////
    const int new_score = std::clamp(std::get<int64_t>(event.get_parameter("score")), 0L, 100L);

    const int new_relation_score = (new_score + current_targeted_nation_relation) / 2;
    const std::string new_relation_rating = Text::get_relation_rating(new_relation_score);

    ///////// e. /////////
    const bool increased = (new_relation_score > current_relation_score);
    const std::string emoji = (increased ? ":arrow_up:" : ":arrow_down:");
    const std::string verb = (increased ? "improved" : "degraded");

    if (executer_nation_score.size() == 0)
        Database::db_query(database, "INSERT INTO relations VALUES ('" + executer_nation_id + "', '" + targeted_nation_id + "', '" + std::to_string(new_score) + "')");
    else Database::db_query(database, "UPDATE relations SET score = '" + std::to_string(new_score) + "' WHERE defining_nation = '" + executer_nation_id + "' AND targeted_nation = '" + targeted_nation_id + "'");

    event.reply(dpp::message(emoji + " The relation between the " + executer_nation_government_type + " of " + executer_nation_name + " and the " + targeted_nation_government_type + " of " + targeted_nation_name + " has officially " + verb + ".\n- Before: " + current_relation_rating + " (" + std::to_string(current_relation_score) + "% - " + std::to_string(current_executer_nation_relation) + "%, " + std::to_string(current_targeted_nation_relation) + "%).\n- After: " + new_relation_rating + " (" + std::to_string(new_relation_score) + "% - " + std::to_string(new_score) + "%, " + std::to_string(current_targeted_nation_relation) + "%).").set_flags(dpp::m_ephemeral));

    ////////////////// 3) //////////////////
    ///////// a. /////////
    Database::Output config = Database::db_query(database, "SELECT world_channel FROM config LIMIT 1");

    if (config.size() == 0)
    {
        Logs::log("Warning: No config data -> /nation relation.");
        return;
    }

    ///////// b. /////////
    const uint32_t color = (increased ? dpp::colors::light_green : dpp::colors::red);
    const dpp::snowflake guild_id = event.command.guild_id;
    const dpp::snowflake world_channel = dpp::snowflake(config[0]["world_channel"]);

    const dpp::embed embed = dpp::embed()
    .set_color(color)
    .set_title("Relation Updated")
    .set_description("The relation between the " + executer_nation_government_type + " of " + executer_nation_name + " and the " + targeted_nation_government_type + " of" + targeted_nation_name + " has " + verb + " from " + current_relation_rating + " (" + std::to_string(current_relation_score) + "%) to " + new_relation_rating + " (" + std::to_string(new_relation_score) + "%)!");

    bot.message_create
    (
        dpp::message(world_channel, "").add_embed(embed),
        [world_channel](const dpp::confirmation_callback_t &callback)
        {
            if (callback.is_error())
                Logs::log("Warning: Failed to send message in " + std::to_string(world_channel) + " with error " + callback.get_error().human_readable + " -> /nation relation.");
        }
    );
}
