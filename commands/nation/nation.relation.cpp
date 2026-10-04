#include "nation.hpp"

#include "../../config/enumerations.hpp"
#include "../../utils/database/database.hpp"
#include "../../utils/logs/logs.hpp"
#include "../../utils/text/text.hpp"

#include <algorithm>
#include <dpp/dpp.h>
#include <mysql/mysql.h>
#include <string>

/*
    Set a relation score with another nation.

    Tasks:
        1) We do some verification first.
            a. Sanitize the nation ID input and verify that the targeted nation exists.
            b. Verify that user is not stateless.
            c. Try to get some information about user nation.
            d. Verify that the targeted nation and user nation IDs do not match.
            e. Verify user rank.
        2) We process the relation modification request.
            a. Get current relation score defined by user nation towards targeted nation. Default score is 50 (Neutrality) if none was ever defined.
            b. Get current relation score defined by targeted nation towards user nation.
            c. Update user nation definition of its relation with the targeted nation in the database.
            d. Try to retrieve some configuration from the database.
            e. Send a message in the gossip channel pinging the gossip role if possible, announcing the relation score modification.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster              / Client of the bot with all related information.
        - database  / MYSQL*                    / MineWorld database
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
    std::string target_id = std::get<std::string>(event.get_parameter("nation_id"));
    const int64_t score = std::clamp(std::get<int64_t>(event.get_parameter("score")), 0L, 100L);

    target_id = Database::sanitize_input(database, target_id);
    Database::Output target = Database::db_query(database, "SELECT display_name FROM nations WHERE nation_id = '" + target_id + "' LIMIT 1");

    if (target.size() == 0)
    {
        event.reply(dpp::message(":prohibited: Nation `" + target_id + "` does not exist.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// b. /////////
    const dpp::snowflake user_id = event.command.usr.id;
    Database::Output nationality = Database::db_query(database, "SELECT nation_id, rank FROM nationality WHERE user_id = '" + std::to_string(user_id) + "' LIMIT 1");

    if (nationality.size() == 0)
    {
        event.reply(dpp::message(":prohibited: You can not perform this action while being stateless.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// c. /////////
    const std::string nation_id = nationality[0]["nation_id"];
    Database::Output nations = Database::db_query(database, "SELECT display_name FROM nations WHERE nation_id = '" + nation_id + "' LIMIT 1");

    if (nations.size() == 0)
    {
        event.reply(dpp::message(":prohibited: Nation `" + nation_id + "` does not exist.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// d. /////////
    const std::string display_name = nations[0]["display_name"];

    if (nation_id == target_id)
    {
        event.reply(dpp::message(":prohibited: You can not change the relation of " + display_name + " with itself.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// e. /////////
    const int rank = std::stoi(nationality[0]["rank"]);
    const std::string target_name = target[0]["display_name"];

    if (rank < MINISTER)
    {
        event.reply(dpp::message(":prohibited: You must be a government official of " + display_name + " to update the relation score with " + target_name + ".").set_flags(dpp::m_ephemeral));
        return;
    }

    ////////////////// 2) //////////////////
    ///////// a. /////////
    int nation_relation = 50;
    Database::Output nation_query = Database::db_query(database, "SELECT score FROM relations WHERE defining_nation = '" + nation_id + "' AND targeted_nation = '" + target_id + "' LIMIT 1");

    if (nation_query.size() != 0)
        nation_relation = std::stoi(nation_query[0]["score"]);

    ///////// b. /////////
    int target_relation = 50;
    Database::Output target_query = Database::db_query(database, "SELECT score FROM relations WHERE defining_nation = '" + target_id + "' AND targeted_nation = '" + nation_id + "' LIMIT 1");

    if (target_query.size() != 0)
        target_relation = std::stoi(target_query[0]["score"]);

    ///////// c. /////////
    const int current_relation = (nation_relation + target_relation) / 2;
    const std::string current_rating = Text::get_relation_rating(current_relation);

    const int new_relation = (score + target_relation) / 2;
    const std::string new_rating = Text::get_relation_rating(new_relation);

    const bool increased = (new_relation > current_relation);
    const std::string emoji = (increased ? ":arrow_up:" : ":arrow_down:");
    const std::string verb = (increased ? "improved" : "degraded");

    if (nation_query.size() == 0)
        Database::db_query(database, "INSERT INTO relations (defining_nation, targeted_nation, score) VALUES ('" + nation_id + "', '" + target_id + "', '" + std::to_string(score) + "')");
    else Database::db_query(database, "UPDATE relations SET score = '" + std::to_string(score) + "' WHERE defining_nation = '" + nation_id + "' AND targeted_nation = '" + target_id + "'");

    event.reply(dpp::message(emoji + " The relation between " + display_name + " and " + target_name + " has officially " + verb + ".\n- Before: " + current_rating + " (" + std::to_string(current_relation) + "% - " + std::to_string(nation_relation) + "%, " + std::to_string(target_relation) + "%).\n- After: " + new_rating + " (" + std::to_string(new_relation) + "% - " + std::to_string(score) + "%, " + std::to_string(target_relation) + "%).").set_flags(dpp::m_ephemeral));

    ///////// d. /////////
    Database::Output config = Database::db_query(database, "SELECT world_channel FROM config LIMIT 1");

    if (config.size() == 0)
    {
        Logs::log("Warning: No config data -> /nation relation.");
        return;
    }

    const dpp::snowflake world_channel = dpp::snowflake(config[0]["world_channel"]);

    ///////// e. /////////
    const dpp::snowflake guild_id = event.command.guild_id;
    const uint32_t color = (increased ? dpp::colors::light_green : dpp::colors::red);

    const dpp::embed embed = dpp::embed()
    .set_color(color)
    .set_title("Relation Updated")
    .set_description("The relation between " + display_name + " and " + target_name + " has " + verb + " from " + current_rating + " (" + std::to_string(current_relation) + "%) to " + new_rating + " (" + std::to_string(new_relation) + "%)!");

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
