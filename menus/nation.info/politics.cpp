#include "nation.info.hpp"

#include "../../config/enumerations.hpp"
#include "../../utils/database/database.hpp"
#include "../../utils/text/text.hpp"

#include <dpp/dpp.h>
#include <mysql/mysql.h>
#include <string>

/*
    Display the "Politics" menu of the /nation info command.

    Tasks:
        1) Do some verification.
            a. Verify that we can get some nation ID from the message content (4 "||" characters + at least one character).
            b. Verify that the nation exists.
        2) Process the menu display request.
            a. Try to get the Head of State and Prime Minister of the nation.
            b. Try to get all ministers of the nation and format a list.
            c. Try to get all citizens of the nation and format a list.
            d. Count all economic sanctions and fines that the nation received.
            e. Prepare some information and statistics to display.
            f. Format the embed.
            g. Edit the message to delete the old embed and set the new one.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster        / Client of the bot with all related information.
        - database  / MYSQL*              / MineWorld database
        - event     / dpp::select_click_t / All information about the event.

    Returns (type + description):
        No object returned.
*/
void NationInfo::politics
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
    Database::Output nations = Database::db_query(database, "SELECT display_name, government_type, ideology, join_condition, invite_permission, tux_balance, media_freedom, leadership_changes, last_leadership_change, government_changes, last_government_change FROM nations WHERE nation_id = '" + nation_id + "' LIMIT 1");

    if (nations.size() == 0)
    {
        event.reply(dpp::message(":prohibited: Nation ID `" + nation_id + "` does not exist.").set_flags(dpp::m_ephemeral));
        return;
    }

    ////////////////// 2) //////////////////
    ///////// a. /////////
    Database::Output nation_leader = Database::db_query(database, "SELECT user_id FROM nationality WHERE nation_id = '" + nation_id + "' AND rank = '" + std::to_string(LEADER) + "' LIMIT 1");
    Database::Output nation_pm = Database::db_query(database, "SELECT user_id FROM nationality WHERE nation_id = '" + nation_id + "' AND rank = '" + std::to_string(PRIME_MINISTER) + "' LIMIT 1");

    const std::string leader = (nation_leader.size() != 0) ? "<@" + nation_leader[0]["user_id"] + ">" : "None";
    const std::string prime_minister = (nation_pm.size() != 0) ? "<@" + nation_pm[0]["user_id"] + ">" : "None";

    ///////// b. /////////
    Database::Output nation_ministers = Database::db_query(database, "SELECT user_id FROM nationality WHERE nation_id = '" + nation_id + "' AND rank = '" + std::to_string(MINISTER) + "'");
    std::string ministers;

    if (ministers.size() != 0)
    {
        for (std::map<std::string, std::string> &user : nation_ministers)
        {
            if (!ministers.empty())
                ministers += ", ";

            ministers += "<@" + user["user_id"] + ">";
        }
    }
    else ministers = "None";

    ///////// c. /////////
    Database::Output nation_citizens = Database::db_query(database, "SELECT user_id FROM nationality WHERE nation_id = '" + nation_id + "' AND rank = '" + std::to_string(CITIZEN) + "'");
    std::string citizens;

    if (citizens.size() != 0)
    {
        for (std::map<std::string, std::string> &user : nation_citizens)
        {
            if (!citizens.empty())
                citizens += ", ";

            citizens += "<@" + user["user_id"] + ">";
        }
    }
    else citizens = "None";

    ///////// d. /////////
    const std::string trade_penalty = std::to_string(TRADE_PENALTY);
    const std::string import_ban = std::to_string(IMPORT_BAN);
    const std::string export_ban = std::to_string(EXPORT_BAN);
    const std::string trade_ban = std::to_string(TRADE_BAN);
    const std::string fine = std::to_string(FINE);

    Database::Output economic_sanctions = Database::db_query(database, "SELECT 1 FROM sanctions WHERE nation_id = '" + nation_id + "' AND pending = 0 AND sanction_type IN ('" + trade_penalty + "', '" + import_ban + "', '" + export_ban + "', '" + trade_ban + "')");
    Database::Output fines = Database::db_query(database, "SELECt 1 FROM sanctions WHERE nation_id = '" + nation_id + "' AND pending = 0 AND sanction_type = '" + fine + "'");

    ///////// e. /////////
    const std::string display_name = nations[0]["display_name"];
    const std::string freedom_rating = Text::get_media_freedom_rating(std::stoi(nations[0]["media_freedom"])) + " (" + nations[0]["media_freedom"] + "%)";
    const std::string government_type = Text::get_government_type(std::stoi(nations[0]["government_type"]));
    const std::string ideology = Text::get_ideology(std::stoi(nations[0]["ideology"]));
    const std::string invite_permission = Text::get_invite_permission(std::stoi(nations[0]["invite_permission"]));
    const std::string join_condition = Text::get_join_condition(std::stoi(nations[0]["join_condition"]));
    const std::string last_leader = (nations[0]["last_leadership_change"] == "0" ? "Never" : "<t:" + nations[0]["last_leadership_change"] + ":f>");
    const std::string last_government = (nations[0]["last_government_change"] == "0" ? "Never" : "<t:" + nations[0]["last_government_change"] + ":f>");

    ///////// f. /////////
    const dpp::embed embed = dpp::embed()
    .set_color(dpp::colors::gold)
    .set_title(display_name)
    .add_field
    (
        ":classical_building: Government",

        "**Head of State:** " + leader + ".\n" +
        "**Prime Minister**: " + prime_minister + ".\n" +
        "**Ministry**: " + ministers + "."
    )
    .add_field
    (
        ":homes: Citizens of " + display_name,
        citizens + "."
    )
    .add_field
    (
        ":dart: Viewpoint",

        "**Government**: " + government_type + ".\n" +
        "**Ideology**: " + ideology + ".\n" +
        "**Freedom of speech**: " + freedom_rating + "."
    )
    .add_field
    (
        ":coin: Economy",

        "**Tux balance**: " + nations[0]["tux_balance"] + "Ť.\n" +
        "**Trade sanctions**: " + std::to_string(economic_sanctions.size()) + ".\n" +
        "**Fines**: " + std::to_string(fines.size()) + "."
    )
    .add_field
    (
        ":luggage: Immigration Laws",

        "**Join Condition**: " + join_condition + ".\n" +
        "**Invitation Permission**: " + invite_permission + "."
    )
    .add_field
    (
        ":bar_chart: Statistics",

        "**Leader changes**: " + nations[0]["leadership_changes"] + ".\n" +
        "**Last leader change**: " + last_leader + ".\n" +
        "**Government changes**: " + nations[0]["government_changes"] + ".\n" +
        "**Last government change**: " + last_government + "."
    )
    .set_footer(dpp::embed_footer().set_icon(event.command.usr.get_avatar_url()).set_text("Requested by " + event.command.usr.username + "."));

    ///////// g. /////////
    dpp::message message = event.command.msg;

    message.embeds.clear();
    message.add_embed(embed);

    bot.message_edit(message);
    event.reply();
}
