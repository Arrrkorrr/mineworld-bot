#include "nation.info.hpp"

#include "../../config/enumerations.hpp"
#include "../../utils/database/database.hpp"

#include <dpp/dpp.h>
#include <mysql/mysql.h>
#include <string>

/*
    Display the "United Nations" menu of the /nation info command.

    Tasks:
        1) We do some verification.
            a. Verify that we can get some nation ID from the message content (4 "||" characters + at least one character).
            b. Verify that the nation exists.
        2) Process the menu display request.
            a. Count all sanctions from all existing sanction types.
            b. Count all resolutions proposed by the nation.
            c. Try to get some information about any potential membership of the nation in the United Nations. Set default data as if the nation is not a member.
            d. If the nation is part of the UN, retrieve all basic information.
            e. Try to get the display name of the nation that invited the targeted nation.
            f. Parse the IDs of the nations that voted in favor of the membership and register their flags.
            g. Parse the IDs of the nations that voted against the membership and register their flags.
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
void NationInfo::united_nations
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
    Database::Output nations = Database::db_query(database, "SELECT display_name, nuclear_state, acquired_nuclear_time, veto_state, resolutions_count, last_resolution, veto_usage_count, last_veto_usage, passed_resolutions, last_passed_resolution FROM nations WHERE nation_id = '" + nation_id + "' LIMIT 1");

    if (nations.size() == 0)
    {
        event.reply(dpp::message(":prohibited: Nation `" + nation_id + "` does not exist.").set_flags(dpp::m_ephemeral));
        return;
    }

    ////////////////// 2) //////////////////
    ///////// a. /////////
    Database::Output trade_penalties = Database::db_query(database, "SELECT 1 FROM sanctions WHERE sanctioned_nation = '" + nation_id + "' AND pending = 0 AND sanction_type = " + std::to_string(TRADE_PENALTY));
    Database::Output import_bans = Database::db_query(database, "SELECT 1 FROM sanctions WHERE sanctioned_nation = '" + nation_id + "' AND pending = 0 AND sanction_type = " + std::to_string(IMPORT_BAN));
    Database::Output export_bans = Database::db_query(database, "SELECT 1 FROM sanctions WHERE sanctioned_nation = '" + nation_id + "' AND pending = 0 AND sanction_type = " + std::to_string(EXPORT_BAN));
    Database::Output trade_bans = Database::db_query(database, "SELECT 1 FROM sanctions WHERE sanctioned_nation = '" + nation_id + "' AND pending = 0 AND sanction_type = " + std::to_string(TRADE_BAN));
    Database::Output fines = Database::db_query(database, "SELECT 1 FROM sanctions WHERE sanctioned_nation = '" + nation_id + "' AND pending = 0 AND sanction_type = " + std::to_string(FINE));
    Database::Output veto_abuses = Database::db_query(database, "SELECT 1 FROM sanctions WHERE sanctioned_nation = '" + nation_id + "' AND pending = 0 AND sanction_type = " + std::to_string(VETO_ABUSE));
    Database::Output events_bans = Database::db_query(database, "SELECT 1 FROM sanctions WHERE sanctioned_nation = '" + nation_id + "' AND pending = 0 AND sanction_type = " + std::to_string(EVENTS_BAN));

    ///////// b. /////////
    Database::Output sanctions = Database::db_query(database, "SELECT 1 FROM sanctions WHERE nation_id = '" + nation_id + "' AND pending = 0");
    Database::Output laws = Database::db_query(database, "SELECT 1 FROM sanctions WHERE nation_id = '" + nation_id + "' AND pending = 0");
    Database::Output proposed_memberships = Database::db_query(database, "SELECt 1 FROM un_membership WHERE invited_by = '" + nation_id + "' AND pending = 0");

    ///////// c. /////////
    Database::Output membership = Database::db_query(database, "SELECT joining_time, invited_by, vote_start, vote_duration, votes_for, votes_against FROM un_membership WHERE nation_id = '" + nation_id + "' AND pending = 0 LIMIT 1");

    std::string is_un_state = "No";
    std::string joining_time = "Never";
    std::string invited_by = "Not a UN member yet";
    std::string vote_proposition = "Not a UN member yet";
    std::string nations_for = "Not a UN member yet";
    std::string nations_against = "Not a UN member yet";

    ///////// d. /////////
    if (membership.size() != 0)
    {
        is_un_state = "Yes";
        invited_by = membership[0]["invited_by"];
        joining_time = ("<t:" + membership[0]["joining_time"] + ":f>");
        nations_against = "";
        nations_for = "";
        vote_proposition = ("<t:" + membership[0]["vote_start"] + ":f>.\n**Vote Duration**: " + membership[0]["vote_duration"]);

        ///////// e. /////////
        Database::Output inviting_nation = Database::db_query(database, "SELECT display_name FROM nations WHERE nation_id = '" + invited_by + "'");

        if (inviting_nation.size() != 0)
            invited_by = inviting_nation[0]["display_name"];

        ///////// f. /////////
        std::istringstream first_stream(membership[0]["votes_for"]);
        std::string id;
        int count = 10;

        while (std::getline(first_stream, id, ','))
        {
            if (count <= 0)
                break;

            Database::Output voting_nation = Database::db_query(database, "SELECT display_name FROM nations WHERE nation_id = '" + id + "' LIMIT 1");

            if (voting_nation.size() == 0)
                continue;

            nations_for += (voting_nation[0]["display_name"]);
            count--;
        }

        ///////// g. /////////
        std::istringstream second_stream(membership[0]["votes_against"]);
        nations_against = "";
        count = 10;

        while (std::getline(second_stream, id, ','))
        {
            if (count <= 0)
                break;

            Database::Output voting_nation = Database::db_query(database, "SELECT display_name FROM nations WHERE nation_id = '" + id + "' LIMIT 1");

            if (voting_nation.size() == 0)
                continue;

            nations_against += (voting_nation[0]["display_name"]);
            count--;
        }
    }

    ///////// h. /////////
    const std::string display_name = nations[0]["display_name"];

    const std::string adopted_resolutions = nations[0]["passed_resolutions"];
    const std::string is_veto_state = (nations[0]["veto_state"] == "0" ? "No" : "Yes");
    const std::string last_adopted_resolution = (nations[0]["last_passed_resolution"] == "0" ? "Never" : "<t:" + nations[0]["last_passed_resolution"] + ":f>");
    const std::string last_resolution = (nations[0]["last_resolution"] == "0" ? "Never" : "<t:" + nations[0]["last_resolution"] + ":f>");
    const std::string last_veto = (nations[0]["last_veto_usage"] == "0" ? "Never" : "<t:" + nations[0]["last_veto_usage"] + ":f>");
    const std::string resolutions_count = nations[0]["resolutions_count"];
    const std::string veto_count = nations[0]["veto_usage_count"];

    ///////// i. /////////
    const dpp::embed embed = dpp::embed()
    .set_color(dpp::colors::light_blue)
    .set_title(display_name)
    .add_field
    (
        ":mag: Profile",

        "**Member of United Nations**: " + is_un_state + ".\n" +
        "**Veto State**: " + is_veto_state + "."
    )
    .add_field
    (
        ":information: Membership Details",

        "**Joined United Nations**: " + joining_time + ".\n" +
        "**Inviting Nation**: " + invited_by + ".\n" +
        "**Proposition Vote**: " + vote_proposition + ".\n" +
        "**Favorable Nations**: " + nations_for + " nations.\n" +
        "**Nations Against** " + nations_against + " nation."
    )
    .add_field
    (
        ":bar_chart: Activity",

        "**Proposed Resolutions**: " + resolutions_count + ".\n" +
        "**Adopted Resolutions**: " + adopted_resolutions + ".\n" +
        "**Sanctions Proposed**: " + std::to_string(sanctions.size()) + ".\n" +
        "**Laws Proposed**: " + std::to_string(laws.size()) + ".\n" +
        "**Memberships Proposed**: " + std::to_string(proposed_memberships.size()) + ".\n" +
        "**Veto Count**: " + veto_count + ".\n" +
        "**Last Resolution**: " + last_resolution + ".\n" +
        "**Last Adopted Resolution**: " + last_adopted_resolution + ".\n" +
        "**Last Veto**: " + last_veto + "."
    )
    .add_field
    (
        ":hammer: Sanctions",

        "**Trade Penalties**: " + std::to_string(trade_penalties.size()) + ".\n" +
        "**Import Bans**: " + std::to_string(import_bans.size()) + ".\n" +
        "**Export Bans**: " + std::to_string(export_bans.size()) + ".\n" +
        "**Trade Bans**: " + std::to_string(trade_bans.size()) + ".\n" +
        "**Fines**: " + std::to_string(fines.size()) + ".\n" +
        "**Veto Abuses**: " + std::to_string(veto_abuses.size()) + ".\n" +
        "**Events Bans**: " + std::to_string(events_bans.size()) + "."
    )
    .set_footer(dpp::embed_footer().set_icon(event.command.usr.get_avatar_url()).set_text("Requested by " + event.command.usr.username + "."));

    ///////// j. /////////
    dpp::message message = event.command.msg;

    message.embeds.clear();
    message.add_embed(embed);

    bot.message_edit(message);
    event.reply();
}
