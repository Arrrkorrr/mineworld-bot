#include "buttons.hpp"

#include "../config/enumerations.hpp"
#include "../utils/database/database.hpp"
#include "../utils/logs/logs.hpp"
#include "../utils/miscellaneous/miscellaneous.hpp"
#include "../utils/text/text.hpp"

#include <algorithm>
#include <dpp/dpp.h>
#include <mysql/mysql.h>
#include <string>

/*
    Delete a post made in the journalism channel.

    Tasks:
        1) We do some verification first.
            a. Try to get some information about the post.
            b. Verify that the user is not trying to remove a post of another person.
        2) Proceed to the removal of the post.
            a. Delete the embed containing the post itself and all buttons.
            b. Modify the message to say it was removed.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster        / Client of the bot with all related information.
        - event     / dpp::button_click_t / All information about the event.

    Returns (type + description):
        No object returned.
*/
void Buttons::journalism_delete
(
    dpp::cluster        &bot,
    dpp::button_click_t &event
)
{
    ////////////////// 1) //////////////////
    ///////// a. /////////
    const int message_length = event.command.msg.content.size();
    const std::string message_content = event.command.msg.content.substr(2, message_length - 4);
    const size_t dot_position = message_content.find(".");

    if (dot_position == std::string::npos)
    {
        event.reply(dpp::message(":prohibited: Something went wrong while retrieving post information.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// b. /////////
    const std::string user_id = message_content.substr(0, dot_position);
    const dpp::snowflake executer_id = event.command.usr.id;

    if (user_id != std::to_string(executer_id))
    {
        event.reply(dpp::message(":prohibited: You can not remove other people posts.").set_flags(dpp::m_ephemeral));
        return;
    }

    ////////////////// 2) //////////////////
    ///////// a. /////////
    dpp::message message = event.command.msg;
    message.set_flags(message.flags | dpp::m_suppress_embeds);
    message.components.clear();

    ///////// b. /////////
    message.set_content(":warning: Post deleted by their publisher.");
    bot.message_edit(message);

    event.reply(dpp::message(":wastebasket: Your post has been removed.").set_flags(dpp::m_ephemeral));
}



/*
    Censor a post made in the journalism channel.

    Tasks:
        1) We do some verification first.
            a. Verify that the executer is not stateless.
            b. Try to get some information about the executer nation.
            c. Verify that the executer is part of the government.
            d. Detect the position of the dot in the message content for parsing.
            e. Verify that the nation the post was published from matches the executer nation.
            f. Verify that the executer and user IDs do not match.
            g. Try to get some information about the post publisher.
            h. If they are in the same nation, verify that the user does not have a higher rank than the executer.
            i. Verify that the user does not have the same rank as the executer.
        2) We proceed the censor request.
            a. We delete the post itself (the embed) and remove all buttons.
            b. Edit the message content to say that the post was censored.
            c. Update nation stats in the database.
        3) If it was the blacklist button that was originally pressed, we also blacklist the user if they are still in the nation.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster        / Client of the bot with all related information.
        - database  / MYSQL*              / MineWorld database
        - event     / dpp::button_click_t / All information about the event.
        - ID        / string              / ID of the button pressed.

    Returns (type + description):
        No object returned.
*/
void Buttons::journalism_censor
(
    dpp::cluster        &bot,
    MYSQL*              &database,
    dpp::button_click_t &event,
    const std::string   &ID
)
{
    ////////////////// 1) //////////////////
    ///////// a. /////////
    const dpp::snowflake executer_id = event.command.usr.id;
    Database::Output executer_nationality = Database::db_query(database, "SELECT nation_id, rank FROM nationality WHERE user_id = '" + std::to_string(executer_id) + "' LIMIT 1");

    if (executer_nationality.size() == 0)
    {
        event.reply(dpp::message(":prohibited: You can not perform this action being stateless.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// b. /////////
    const std::string nation_id = executer_nationality[0]["nation_id"];
    Database::Output nations = Database::db_query(database, "SELECT display_name FROM nations WHERE nation_id = '" + nation_id + "' LIMIT 1");

    if (nations.size() == 0)
    {
        event.reply(dpp::message(":prohibited: Nation `" + nation_id + "` does not exist.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// c. /////////
    const int executer_rank = std::stoi(executer_nationality[0]["rank"]);
    const std::string display_name = nations[0]["display_name"];

    if (executer_rank < MINISTER)
    {
        event.reply(dpp::message(":prohibited: You must be a government official of " + display_name + " to censor posts.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// d. /////////
    const int message_length = event.command.msg.content.size();
    const std::string message_content = event.command.msg.content.substr(2, message_length - 4);
    const size_t dot_position = message_content.find(".");

    if (dot_position == std::string::npos)
    {
        event.reply(dpp::message(":prohibited: Something went wrong while retrieving post information.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// e. /////////
    const std::string post_nation_id = message_content.substr(dot_position + 1);
    const std::string rank_name = Text::get_rank(executer_rank);

    if (nation_id != post_nation_id)
    {
        Database::Output post_nation = Database::db_query(database, "SELECT display_name FROM nations WHERE nation_id = '" + post_nation_id + "' LIMIT 1");

        if (post_nation.size() == 0)
        {
            Logs::log("Warning: Nation ID " + post_nation_id + " missing in database -> journalism_censor button.");
            return event.reply(dpp::message(":prohibited: You can not censor a post published from another country being " + rank_name + " of " + display_name + ".").set_flags(dpp::m_ephemeral));
        }

        const std::string post_nation_name = post_nation[0]["display_name"];
        return event.reply(dpp::message(":prohibited: You can not censor a post published from " + post_nation_name + " being " + rank_name + " of " + display_name + ".").set_flags(dpp::m_ephemeral));
    }

    ///////// f. /////////
    const std::string user_id = message_content.substr(0, dot_position);

    if (user_id == std::to_string(executer_id))
    {
        event.reply(dpp::message(":prohibited: You can not censor yourself. If you wish to remove your post, press the Delete button instead.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// g. /////////
    Database::Output user_nationality = Database::db_query(database, "SELECT nation_id, rank FROM nationality WHERE user_id = '" + user_id + "' LIMIT 1");

    const int user_rank = std::stoi(user_nationality[0]["rank"]);
    const std::string user_rank_name = Text::get_rank(user_rank);
    std::string user_nation_id;

    if (user_nationality.size() != 0)
    {
        user_nation_id = user_nationality[0]["nation_id"];

        if (user_nation_id == nation_id)
        {
            ///////// h. /////////
            if (user_rank > executer_rank)
            {
                event.reply(dpp::message(":prohibited: You can not censor a post published by " + user_rank_name + " <@" + user_id + "> of " + display_name + " as they have a higher rank than you (" + user_rank_name + " > " + rank_name + ").").set_flags(dpp::m_ephemeral));
                return;
            }

            ///////// i. /////////
            if (user_rank == executer_rank)
            {
                event.reply(dpp::message(":prohibited: You can not censor a post published by " + user_rank_name + " <@" + user_id + "> of " + display_name + " as you share the same rank.").set_flags(dpp::m_ephemeral));
                return;
            }
        }
    }

    ////////////////// 2) //////////////////
    ///////// a. /////////
    dpp::message message = event.command.msg;
    message.set_flags(message.flags | dpp::m_suppress_embeds);
    message.components.clear();

    ///////// b. /////////
    message.set_content(":warning: Post taken down by the government of " + display_name + ".");
    bot.message_edit(message);

    ///////// c. /////////
    const int score_hit = (ID == "journalism_blacklist" ? 8 : 3);
    const int media_freedom = std::clamp(std::stoll(nations[0]["media_freedom"]) - score_hit, 0LL, 100LL);
    const int censored_posts = std::stoll(nations[0]["censored_posts"]) + 1;
    const std::string timestamp = std::to_string(Miscellaneous::get_current_timestamp());

    Database::db_query(database, "UPDATE nations SET media_freedom = '" + std::to_string(media_freedom) + "', censored_posts = '" + std::to_string(censored_posts) + "', last_manual_censorship = '" + timestamp + "' WHERE nation_id = '" + nation_id + "'");

    ////////////////// 3) //////////////////
    if (ID == "journalism_blacklist" && user_nation_id == nation_id)
    {
        Database::Output registered = Database::db_query(database, "SELECT 1 FROM journalism WHERE user_id = '" + user_id + "' LIMIT 1");

        if (registered.size() == 0)
            Database::db_query(database, "INSERT INTO journalism (user_id, status) VALUES ('" + user_id + "', 1)");
        else Database::db_query(database, "UPDATE journalism SET status = 1 WHERE user_id = '" + user_id + "'");

        event.reply(dpp::message(":wastebasket: This post has been censored and " + user_rank_name + " <@" + user_id + "> blacklisted. The media freedom rating of " + display_name + " was hit by 8 points.").set_flags(dpp::m_ephemeral));
    }
    else event.reply(dpp::message(":wastebasket: This post published by " + user_rank_name + " <@" + user_id + "> has been censored. The media freedom rating of " + display_name + " was hit by 3 points.").set_flags(dpp::m_ephemeral));
}
