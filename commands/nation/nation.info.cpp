#include "nation.hpp"

#include "../../utils/database/database.hpp"
#include "../../utils/text/text.hpp"

#include <dpp/dpp.h>
#include <map>
#include <mysql/mysql.h>
#include <string>

/*
    Display information and stats of a nation.

    Tasks:
        1) Get the nation ID from the command parameter.
        2) Verify that the nation exists.
        3) Handle the information request.
            a. Count the amount of members available for this nation ID.
            b. Prepare some data and formatting the information.
            c. Set the embed and select menu to be sent.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster              / Client of the bot with all related information.
        - database  / MYSQL*                    / MineWorld database
        - event     / dpp::interaction_create_t / All information about the event.

    Returns (type + description):
        No object returned.
*/
void Nation::nation_info
(
    dpp::cluster                    &bot,
    MYSQL*                          &database,
    const dpp::interaction_create_t &event
)
{
    ////////////////// 1) //////////////////
    const std::string nation_id = std::to_string(std::get<int64_t>(event.get_parameter("nation_id")));

    ////////////////// 2) //////////////////
    Database::Output nations = Database::db_query(database, "SELECT display_name, description, creation_time, government_type, ideology, join_condition, role_id FROM nations WHERE nation_id = '" + nation_id + "' LIMIT 1");

    if (nations.size() == 0)
    {
        event.reply(dpp::message(":prohibited: Nation ID `" + nation_id + "` does not exist.").set_flags(dpp::m_ephemeral));
        return;
    }

    ////////////////// 3) //////////////////
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

    const dpp::component select_menu = dpp::component()
    .add_component (
        dpp::component()
        .set_type(dpp::cot_selectmenu)
        .set_id("nation_info")
        .set_placeholder("Select a category of information.")
        .add_select_option(dpp::select_option("Overview", "overview", "Basic information about the nation.").set_emoji(u8"👁️"))
        .add_select_option(dpp::select_option("Politics, Economy & Population", "politics", "Political and economic life of the nation.").set_emoji(u8"🪙"))
        .add_select_option(dpp::select_option("Media", "media", "Freedom of speech and press situation in the nation.").set_emoji(u8"📰"))
        .add_select_option(dpp::select_option("Warfare", "warfare", "Military capacities of the nation.").set_emoji(u8"🪖"))
        .add_select_option(dpp::select_option("United Nations", "united_nations", "International performance of the nation.").set_emoji(u8"🇺🇳"))
        .add_select_option(dpp::select_option("Close", "close", "Delete this message.").set_emoji(u8"🚫"))
    );

    event.reply(dpp::message("||" + nation_id + "||").add_embed(embed).add_component(select_menu));
}
