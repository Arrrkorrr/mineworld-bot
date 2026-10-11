#include "nation.info.hpp"

#include "../../config/enumerations.hpp"
#include "../../utils/database/database.hpp"
#include "../../utils/text/text.hpp"

#include <dpp/dpp.h>
#include <mysql/mysql.h>
#include <string>

/*
    Display the "Media" menu of the /nation info command.

    Tasks:
        1) We do some verification.
            a. Verify that we can get some nation ID from the message content (4 "||" characters + at least one character).
            b. Verify that the nation exists.
        2) Process the menu display request.
            a. Try to get all users of the nation that are blacklisted and format a list.
            b. Try to get all users of the nation that are whitelisted and format a list.
            c. Prepare some information and statistics to display.
            d. Format the embed.
            e. Edit the message to delete the old embed and set the new one.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster        / Client of the bot with all related information.
        - database  / MYSQL*              / MineWorld database
        - event     / dpp::select_click_t / All information about the event.

    Returns (type + description):
        No object returned.
*/
void NationInfo::media
(
    dpp::cluster              &bot,
    MYSQL*                    &database,
    const dpp::select_click_t &event
)
{
    ////////////////// 1) //////////////////
    ///////// a. /////////
    const int message_length = event.command.msg.content.size();

    if (message_length < 5)
    {
        event.reply(dpp::message(":prohibited: Something went wrong while retrieving nation ID.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// b. /////////
    const std::string nation_id = event.command.msg.content.substr(2, message_length - 4);
    Database::Output nations = Database::db_query(database, "SELECT display_name, media_posts, last_post, censored_posts, last_manual_censorship, last_automatic_censorship, media_freedom, media_blacklist, media_whitelist FROM nations WHERE nation_id = '" + nation_id + "' LIMIT 1");

    if (nations.size() == 0)
    {
        event.reply(dpp::message(":prohibited: Nation `" + nation_id + "` does not exist.").set_flags(dpp::m_ephemeral));
        return;
    }

    ////////////////// 2) //////////////////
    ///////// a. /////////
    Database::Output nation_blacklist = Database::db_query(database, "SELECT user_id FROM journalism WHERE status = '" + std::to_string(BLACKLIST) + "'");
    std::string blacklist;

    if (blacklist.size() != 0)
    {
        for (std::map<std::string, std::string> &user : nation_blacklist)
        {
            if (!blacklist.empty())
                blacklist += ", ";

            blacklist += "<@" + user["user_id"] + ">";
        }
    }
    else blacklist = "None";

    ///////// b. /////////
    Database::Output nation_whitelist = Database::db_query(database, "SELECT user_id FROM journalism WHERE status = '" + std::to_string(WHITELIST) + "'");
    std::string whitelist;

    if (whitelist.size() != 0)
    {
        for (std::map<std::string, std::string> &user : nation_whitelist)
        {
            if (!whitelist.empty())
                whitelist += ", ";

            whitelist += "<@" + user["user_id"] + ">";
        }
    }
    else whitelist = "None";

    ///////// c. /////////
    const std::string display_name = nations[0]["display_name"];

    const std::string post_count = nations[0]["media_posts"];
    const int percentage = (post_count == "0" ? 0 : (int)(std::stoi(nations[0]["censored_posts"]) / std::stoi(post_count)));
    const std::string censored_posts = nations[0]["censored_posts"] + " post(s) (" + std::to_string(percentage * 100) + "%)";

    const std::string freedom_rating = (Text::get_media_freedom_rating(std::stoi(nations[0]["media_freedom"])) + " (" + nations[0]["media_freedom"] + "%)");
    const std::string last_automatic = (nations[0]["last_automatic_censorship"] == "0" ? "Never" : "<t:" + nations[0]["last_automatic_censorship"] + ":f>");
    const std::string last_manual = (nations[0]["last_manual_censorship"] == "0" ? "Never" : "<t:" + nations[0]["last_manual_censorship"] + ":f>");
    const std::string last_post = (nations[0]["last_post"] == "0" ? "Never" : "<t:" + nations[0]["last_post"] + ":f>");
    const std::string media_blacklist = (nations[0]["media_blacklist"] == "0" ? "Off" : "On");
    const std::string media_whitelist = (nations[0]["media_whitelist"] == "0" ? "Off" : "On");

    ///////// d. /////////
    const dpp::embed embed = dpp::embed()
    .set_color(dpp::colors::cyan)
    .set_title(display_name)
    .add_field
    (
        ":dove: Media",

        "**Freedom of speech**: " + freedom_rating + ".\n" +
        "**Publications**: " + post_count + " post(s).\n" +
        "**Last publication**: " + last_post + "."
    )
    .add_field
    (
        ":mute: Censorship",

        "**Censored posts**: " + censored_posts + ".\n" +
        "**Media blacklist**: " + media_blacklist + ".\n" +
        "**Media whitelist**: " + media_whitelist + ".\n" +
        "**Last manual censorship**: " + last_manual + ".\n" +
        "**Last automatic censorship**: " + last_automatic + "."
    )
    .add_field
    (
        ":scroll: Lists",

        "**Blacklisted users**: " + blacklist + ".\n" +
        "**Whitelisted users**: " + whitelist + "."
    )
    .set_footer(dpp::embed_footer().set_icon(event.command.usr.get_avatar_url()).set_text("Requested by " + event.command.usr.username + "."));

    ///////// e. /////////
    dpp::message message = event.command.msg;

    message.embeds.clear();
    message.add_embed(embed);

    bot.message_edit(message);
    event.reply();
}
