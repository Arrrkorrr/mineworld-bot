#include "journalism.hpp"

#include "../../enum/journalism.hpp"
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
    Censor a post made in the journalism channel.

    Tasks:
        1) Some verification.
            a. Verify that the executer is not stateless.
            b. Get some information about the executer nation.
            c. Verify that the executer is part of the government of their nation.
            d. Get some information about the post.
            e. Verify that the executer and post nation IDs match.
            f. Verify that the executer are not trying to censor themselves.
            g. Verify that the publisher is still in the executer nation to compare ranks.
            h. If they share the same nation, verify that the publisher does not have a higher rank than the executer's.
            i. If they share the same nation, verify that the publisher and the executer does not share the same rank.
        2) Censor the post.
            a. Delete the post and buttons.
            b. Edit message content to a censorship notice.
            c. Set post visibility to censored.
        3) Update media statistics.
            a. Retrieve current media stats of the nation.
            b. Update the stats.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster        / Client of the bot with all related information.
        - database  / MYSQL*              / MineWorld database.
        - event     / dpp::button_click_t / All information about the event.

    Returns (type + description):
        A BlacklistInfo object that contains the publisher user ID, the nation freedom rating and ID.
*/
BlacklistInfo Journalism::journalism_censor
(
    dpp::cluster        &bot,
    MYSQL*              &database,
    dpp::button_click_t &event
)
{
    ////////////////// 1) //////////////////
    ///////// a. /////////
    const std::string executer_discord_id = std::to_string(event.command.usr.id);
    Database::Output executer_nationality = Database::db_query(database, "SELECT nationalities.nation_id, nationalities.rank, players.user_id FROM nationalities JOIN players ON players.user_id = nationalities.user_id WHERE players.discord_id = '" + executer_discord_id + "' LIMIT 1");

    if (executer_nationality.size() == 0)
    {
        event.reply(dpp::message(":prohibited: You can not perform this action while being stateless.").set_flags(dpp::m_ephemeral));
        return { "", -1, "" };
    }

    ///////// b. /////////
    const std::string nation_id = executer_nationality[0]["nationalities.nation_id"];
    Database::Output nation = Database::db_query(database, "SELECT nations.display_name, governments.government_type FROM nations JOIN governments ON governments.nation_id = nations.nation_id WHERE nations.nation_id = '" + nation_id + "' LIMIT 1");

    if (nation.size() == 0)
    {
        event.reply(dpp::message(":prohibited: Nation `" + nation_id + "` does not exist.").set_flags(dpp::m_ephemeral));
        return { "", -1, "" };
    }

    ///////// c. /////////
    const std::string nation_name = nation[0]["nations.display_name"];
    const int executer_rank = std::stoi(executer_nationality[0]["nationalities.rank"]);
    const std::string government_type = Text::get_government_type(std::stoi(nation[0]["governments.government_type"]));

    if (executer_rank < MINISTER)
    {
        event.reply(dpp::message(":prohibited: You must be a government official of the " + government_type + " of " + nation_name + " in order to censor posts.").set_flags(dpp::m_ephemeral));
        return { "", -1, "" };
    }

    ///////// d. /////////
    const int message_length = event.command.msg.content.size();
    const std::string post_id = event.command.msg.content.substr(2, message_length - 4);

    Database::Output post = Database::db_query(database, "SELECT nation_id, user_id FROM publications WHERE post_id = '" + post_id + "' LIMIT 1");

    if (post.size() == 0)
    {
        event.reply(dpp::message(":prohibited: Could not retrieve any information about this post."));
        return { "", -1, "" };
    }

    ///////// e. /////////
    const std::string post_nation_id = post[0]["nation_id"];
    const std::string rank_name = Text::get_rank(executer_rank);

    if (nation_id != post_nation_id)
    {
        Database::Output publication_nation = Database::db_query(database, "SELECT nations.display_name, governments.government_type FROM nations JOIN governments ON governments.nation_id = nations.nation_id WHERE nation_id = '" + post_nation_id + "' LIMIT 1");

        if (publication_nation.size() == 0)
        {
            Logs::log("Warning: Nation ID " + post_nation_id + " missing in database -> journalism_censor button.");
            event.reply(dpp::message(":prohibited: You can not censor a post published from another nation being " + rank_name + " of the " + government_type + " of " + nation_name + ".").set_flags(dpp::m_ephemeral));
            return { "", -1, "" };
        }

        const std::string publication_nation_name = publication_nation[0]["nations.display_name"];
        const std::string publication_nation_government_type = Text::get_government_type(std::stoi(publication_nation[0]["governments.government_type"]));

        event.reply(dpp::message(":prohibited: You can not censor a post published from the " + publication_nation_government_type + " of " + publication_nation_name + " being " + rank_name + " of the " + government_type + " of " + nation_name + ".").set_flags(dpp::m_ephemeral));
        return { "", -1, "" };
    }

    ///////// f. /////////
    const std::string executer_id = executer_nationality[0]["players.user_id"];
    const std::string publisher_id = post[0]["user_id"];

    if (publisher_id == executer_id)
    {
        event.reply(dpp::message(":prohibited: You can not censor yourself. In order to remove your own posts, press the `🗑️ Delete` button.").set_flags(dpp::m_ephemeral));
        return { "", -1, "" };
    }

    ///////// g. /////////
    Database::Output publisher_nationality = Database::db_query(database, "SELECT nationalities.nation_id, nationalities.rank, players.discord_id FROM nationalities JOIN players ON players.user_id = nationalities.user_id WHERE nationalities.user_id = '" + publisher_id + "' LIMIT 1");

    const int publisher_rank = std::stoi(publisher_nationality[0]["nationalities.rank"]);
    const std::string publisher_rank_name = Text::get_rank(publisher_rank);

    if (publisher_nationality.size() != 0)
    {
        const std::string publisher_nation_id = publisher_nationality[0]["nationalities.nation_id"];

        if (publisher_nation_id == nation_id)
        {
            const std::string publisher_discord_id = publisher_nationality[0]["players.discord_id"];

            ///////// h. /////////
            if (publisher_rank > executer_rank)
            {
                event.reply(dpp::message(":prohibited: You can not censor a post published by " + publisher_rank_name + " <@" + publisher_discord_id + "> of the " + government_type + " of " + nation_name + " as they have a higher rank than you (" + publisher_rank_name + " > " + rank_name + ").").set_flags(dpp::m_ephemeral));
                return { "", -1, "" };
            }

            ///////// i. /////////
            if (publisher_rank == executer_rank)
            {
                event.reply(dpp::message(":prohibited: You can not censor a post published by " + publisher_rank_name + " <@" + publisher_discord_id + "> of the " + government_type + " of " + nation_name + " as you share the same rank.").set_flags(dpp::m_ephemeral));
                return { "", -1, "" };
            }
        }
    }

    ////////////////// 2) //////////////////
    ///////// a. /////////
    dpp::message message = event.command.msg;

    message.set_flags(message.flags | dpp::m_suppress_embeds);
    message.components.clear();

    ///////// b. /////////
    message.set_content(":warning: Post taken down by the government of the " + government_type + " of " + nation_name + ".");
    bot.message_edit(message);

    ///////// c. /////////
    Database::db_query(database, "UPDATE publications SET visibility = '" + std::to_string(CENSORED_POST) + "' WHERE post_id = '" + post_id + "'");

    ////////////////// 3) //////////////////
    ///////// a. /////////
    Database::Output media = Database::db_query(database, "SELECT freedom_rating FROM media WHERE nation_id = '" + nation_id + "' LIMIT 1");

    if (media.size() == 0)
    {
        Logs::log("Warning: Failed to find any media information about " + nation_id + " -> journalism_censor button.");
        event.reply();
        return { publisher_id, -1, "" };
    }

    ///////// b. /////////
    const int freedom_rating = std::stoi(nation[0]["freedom_rating"]);
    const int new_freedom_rating = std::clamp(freedom_rating - CENSORED_POST_SCORE_HIT, 0, 100);
    const std::string now = std::to_string(Miscellaneous::get_current_timestamp());

    Database::db_query(database, "UPDATE media SET media_freedom = '" + std::to_string(new_freedom_rating) + "', last_manual_censorship = '" + now + "' WHERE nation_id = '" + nation_id + "'");
    event.reply();

    return { publisher_id, new_freedom_rating, nation_id };
}
