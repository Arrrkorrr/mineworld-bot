#include "nation.info.hpp"

#include "../../utils/database/database.hpp"
#include "../../utils/text/text.hpp"

#include <dpp/dpp.h>
#include <mysql/mysql.h>
#include <string>

/*
    Display the "Overview" menu of the "/nation info" command.

    Tasks:
        1) Do some verification.
            a. Verify that we can get some nation ID from the message content (4 "||" characters + at least one character).
            b. Verify that the nation exists.
        2) Process the menu display request.
            a. Count the amount of members available for this nation ID.
            b. Prepare some information and statistics to display.
            c. Format the embed.
            d. Edit the message to delete the old embed and set the new one.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster        / Client of the bot with all related information.
        - database  / MYSQL*              / MineWorld database
        - event     / dpp::select_click_t / All information about the event.

    Returns (type + description):
        No object returned.
*/
void NationInfo::overview
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
    Database::Output nations = Database::db_query(database, "SELECT display_name, description, claim_time, government_type, ideology, join_condition, role_id FROM nations WHERE nation_id = '" + nation_id + "' LIMIT 1");

    if (nations.size() == 0)
    {
        event.reply(dpp::message(":prohibited: Nation ID `" + nation_id + "` does not exist.").set_flags(dpp::m_ephemeral));
        return;
    }

    ////////////////// 2) //////////////////
    ///////// a. /////////
    Database::Output members = Database::db_query(database, "SELECT 1 FROM nationality WHERE nation_id = '" + nation_id + "'");

    ///////// b. /////////
    const std::string creation_time = (nations[0]["creation_time"] == "0" ? "Never" : "<t:" + nations[0]["creation_time"] + ":f>");
    const std::string display_name = nations[0]["display_name"];
    const std::string government_type = Text::get_government_type(std::stoi(nations[0]["government_type"]));
    const std::string ideology = Text::get_ideology(std::stoi(nations[0]["ideology"]));
    const std::string join_condition = Text::get_join_condition(std::stoi(nations[0]["join_condition"]));
    const std::string role_id = (nations[0]["role_id"] == "0" ? "No role generated yet" : "<@&" + nations[0]["role_id"] + "> ||" + nations[0]["role_id"] + "||");

    ///////// c. /////////
    const dpp::embed embed = dpp::embed()
    .set_color(dpp::colors::cream_white)
    .set_title(display_name)
    .add_field
    (
        ":information_source: Description",
        nations[0]["description"]
    )
    .add_field
    (
        ":eye: Overview",

        "**Display Name**: " + display_name + ".\n" +
        "**Nation ID**: " + nation_id + ".\n" +
        "**Government**: " + government_type + ".\n" +
        "**Ideology**: " + ideology + ".\n" +
        "**Member count**: " + std::to_string(members.size()) + " member(s).\n" +
        "**Role**: " + role_id + ".\n" +
        "**Became active**: " + creation_time + ".\n" +
        "**Join condition**: " + join_condition + "."
    )
    .set_footer(dpp::embed_footer().set_icon(event.command.usr.get_avatar_url()).set_text("Requested by " + event.command.usr.username + "."));

    ///////// d. /////////
    dpp::message message = event.command.msg;

    message.embeds.clear();
    message.add_embed(embed);

    bot.message_edit(message);
    event.reply();
}
