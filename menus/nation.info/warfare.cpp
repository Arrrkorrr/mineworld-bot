#include "nation.info.hpp"

#include "../../config/enumerations.hpp"
#include "../../utils/database/database.hpp"
#include "../../utils/text/text.hpp"

#include <dpp/dpp.h>
#include <mysql/mysql.h>
#include <string>

/*
    Display the "Warfare" menu of the /nation info command.

    Tasks:
        1) We do some verification.
            a. Verify that we can get some nation ID from the message content (4 "||" characters + at least one character).
            b. Verify that the nation exists.
        2) Process the menu display request.
            a. Try to get all citizens of the nation with the "Military" rank and format a list.
            b. Try to get the score and nation ID of the lowest relation available for this nation. We default to 50% if nothing comes out.
            c. Try to get more information about the nation. If it fails, we keep the nation ID.
            d. Try to get the score and nation ID of the highest relation available for this nation. We default to 50% if nothing comes out.
            e. Try to get more information about the nation. If it fails, we keep the nation ID.
            f. Try to get the average relation score using all registered scores. We default to 50% if nothing comes out.
            g. Count all economic sanctions and fines that the nation received.
            h. Prepare some information and statistics to display.
            i. Format the embed.
            j. Edit the message to delete the old embed and set the new one.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster        / Client of the bot with all related information.
        - database  / MYSQL*              / MineWorld database
        - event     / dpp::select_click_t / All information about the event.

    Returns (type + description):
        No object returned.
*/
void NationInfo::warfare
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
    Database::Output nations = Database::db_query(database, "SELECT display_name, ideology, nuclear_state, acquired_nuclear_time FROM nations WHERE nation_id = '" + nation_id + "' LIMIT 1");

    if (nations.size() == 0)
    {
        event.reply(dpp::message(":prohibited: Nation `" + nation_id + "` does not exist.").set_flags(dpp::m_ephemeral));
        return;
    }

    ////////////////// 2) //////////////////
    ///////// a. /////////
    Database::Output nation_military = Database::db_query(database, "SELECT user_id FROM nationality WHERE nation_id = '" + nation_id + "' AND rank = '" + std::to_string(MILITARY) + "'");
    std::string military;

    if (military.size() != 0)
    {
        for (std::map<std::string, std::string> &user : nation_military)
        {
            if (!military.empty())
                military += ", ";

            military += "<@" + user["user_id"] + ">";
        }
    }
    else military = "None dedicated";

    ///////// b. /////////
    Database::Output lowest = Database::db_query(database, "SELECT targeted_nation, score FROM relations WHERE defining_nation = '" + nation_id + "' ORDER BY score ASC LIMIT 1");
    std::string lowest_relation = "50%";

    if (lowest.size() != 0)
    {
        const std::string target_nation_id = lowest[0]["targeted_nation"];
        const std::string score = lowest[0]["score"];
        std::string display_name = nation_id;

        ///////// c. /////////
        Database::Output target_nation = Database::db_query(database, "SELECT display_name FROM nations WHERE nation_id = '" + target_nation_id + "' LIMIT 1");

        if (target_nation.size() != 0)
            display_name = target_nation[0]["display_name"];

        lowest_relation = score + "% - " + display_name;
    }

    ///////// d. /////////
    Database::Output highest = Database::db_query(database, "SELECT targeted_nation, score FROM relations WHERE defining_nation = '" + nation_id + "' ORDER BY score DESC LIMIT 1");
    std::string highest_relation = "50%";

    if (highest.size() != 0)
    {
        const std::string target_nation_id = highest[0]["targeted_nation"];
        const std::string score = highest[0]["score"];
        std::string display_name = target_nation_id;

        ///////// e. /////////
        Database::Output target_nation = Database::db_query(database, "SELECT display_name FROM nations WHERE nation_id = '" + target_nation_id + "' LIMIT 1");

        if (target_nation.size() != 0)
            display_name = target_nation[0]["display_name"];

        highest_relation = score + "% - " + display_name;
    }

    ///////// f. /////////
    Database::Output average = Database::db_query(database, "SELECT CAST(ROUND(COALESCE(AVG(score), 50)) AS UNSIGNED) as score FROM relations WHERE defining_nation = '" + nation_id + "'");
    std::string average_relation = "50%";

    if (average.size() != 0 && !average[0]["score"].empty())
        average_relation = average[0]["score"] + "%";

    ///////// g. /////////
    Database::Output sanctions = Database::db_query(database, "SELECT 1 FROM sanctions WHERE nation_id = '" + nation_id + "' AND pending = 0");

    ///////// h. /////////
    const std::string display_name = nations[0]["display_name"];
    const std::string ideology = Text::get_ideology(std::stoi(nations[0]["ideology"]));
    const std::string nuclear_acquisition = (nations[0]["acquired_nuclear_time"] == "0" ? "Never" : "<t:" + nations[0]["acquired_nuclear_time"] + ":f>");
    const std::string nuclear_state = (nations[0]["nuclear_state"] == "0" ? "No" : "Yes");

    ///////// i. /////////
    const dpp::embed embed = dpp::embed()
    .set_color(dpp::colors::dark_green)
    .set_title(display_name)
    .add_field
    (
        ":military_helmet: Military Personnel",
        military + "."
    )
    .add_field
    (
        ":earth_africa: International Relations",

        "**Lowest relation**: " + lowest_relation + ".\n" +
        "**Highest relation**: " + highest_relation + ".\n" +
        "**Average relation score**: " + average_relation + ".\n" +
        "*Keep in mind that only defined relations are counted.\n" +
        "We default to 50% if no data is available.*"
    )
    .add_field
    (
        ":bar_chart: Data and Statistics",

        "**Ideology**: " + ideology + ".\n" +
        "**Nuclear state**: " + nuclear_state + ".\n" +
        "**Nuclear acquisition**: " + nuclear_acquisition + ".\n" +
        "**Sanctions**: " + std::to_string(sanctions.size()) + "."
    )
    .set_footer(dpp::embed_footer().set_icon(event.command.usr.get_avatar_url()).set_text("Requested by " + event.command.usr.username + "."));

    ///////// j. /////////
    dpp::message message = event.command.msg;

    message.embeds.clear();
    message.add_embed(embed);

    bot.message_edit(message);
    event.reply();
}
