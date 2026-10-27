#include "nation.hpp"

#include "../../enum/nations.hpp"
#include "../../utils/database/database.hpp"
#include "../../utils/text/text.hpp"

#include <dpp/cluster.h>
#include <dpp/colors.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <dpp/message.h>
#include <map>
#include <mysql/mysql.h>
#include <string>

/*
    Display information and stats of a nation.

    Tasks:
        1) Prepare information.
            a. Get nation ID from command parameter.
            b. Retrieve some information about the nation.
            c. Get the amount of members in the nation.
        2) Send information to user.
            a. Format information.
            b. Construct embed.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster              / Client of the bot with all related information.
        - database  / MYSQL*                    / MineWorld database.
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
    ///////// a. /////////
    const std::string nation_id = std::to_string(std::get<int64_t>(event.get_parameter("nation_id")));

    ///////// b. /////////
    const std::string query =
    (
        std::string("SELECT nations.display_name, nations.description, nations.deleted, nations.creation_time, nations.role_id, governments.government_type, governments.ideology, immigration.join_condition ") +
                    "FROM nations " +
                    "JOIN governments ON governments.nation_id = nations.nation_id " +
                    "JOIN immigration ON immigration.nation_id = nations.nation_id " +
                    "WHERE nations.nation_id = '" + nation_id + "' LIMIT 1"
    );

    Database::Output nation = Database::db_query(database, query);

    if (nation.size() == 0)
    {
        event.reply(dpp::message(":prohibited: Nation ID `" + nation_id + "` does not exist.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// c. /////////
    Database::Output nation_members = Database::db_query(database, "SELECT 1 FROM nationality WHERE nation_id = '" + nation_id + "'");

    ////////////////// 2) //////////////////
    ///////// a. /////////
    const std::string nation_creation_time = (nation[0]["nations.creation_time"] == "0" ? "Never" : "<t:" + nation[0]["nations.creation_time"] + ":f>");
    const std::string nation_description = nation[0]["nations.description"];
    const std::string nation_government_type = Text::get_government_type(std::stoi(nation[0]["governments.government_type"]));
    const std::string nation_ideology = Text::get_ideology(std::stoi(nation[0]["governments.ideology"]));
    const std::string nation_join_condition = Text::get_join_condition(std::stoi(nation[0]["immigration.join_condition"]));
    const std::string nation_members_count = std::to_string(nation_members[0].size());
    const std::string nation_name = nation[0]["nations.display_name"];
    const std::string nation_role = (nation[0]["nations.role_id"] == "0" ? "No role generated yet" : "<@&" + nation[0]["nations.role_id"] + "> ||" + nation[0]["role_id"] + "||");
    const std::string nation_status = (std::stoi(nation[0]["nations.deleted"]) == EXISTING_NATION ? "Active" : "Collapsed");

    ///////// b. /////////
    const std::string executer_avatar_url = event.command.usr.get_avatar_url();
    const std::string executer_username = event.command.usr.username;

    const dpp::embed embed = dpp::embed()
    .set_color(dpp::colors::cream_white)
    .set_title(nation_name)
    .add_field
    (
        ":information_source: Description",
        nation_description
    )
    .add_field
    (
        ":eye: Overview",

        "**Display Name**: " + nation_name + ".\n" +
        "**Status**: " + nation_status + ".\n" +
        "**Nation ID**: " + nation_id + ".\n" +
        "**Government**: " + nation_government_type + ".\n" +
        "**Ideology**: " + nation_ideology + ".\n" +
        "**Member count**: " + nation_members_count+ " member(s).\n" +
        "**Role**: " + nation_role + ".\n" +
        "**Became active**: " + nation_creation_time + ".\n" +
        "**Join condition**: " + nation_join_condition + "."
    )
    .set_footer(dpp::embed_footer().set_icon(executer_avatar_url).set_text("Requested by " + executer_username + "."));

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
