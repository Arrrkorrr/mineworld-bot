#include "modals.hpp"

#include "../utils/database/database.hpp"
#include "../utils/miscellaneous/miscellaneous.hpp"
#include "../utils/text/text.hpp"

#include <algorithm>
#include <dpp/dpp.h>
#include <mysql/mysql.h>
#include <string>

/*
    Make a post in the journalism channel.

    Tasks:
        1) Get the modal field values.
        2) We do some verification process.
            a. Verify that the user is not stateless.
            b. Try to get some information and stats about the nation.
            c. Try to get the current authorization of the user in case the whitelist or the blacklist is enable.
            d. If the whitelist is enabled, verify that the user is part of it.
            e. If the blacklist is enabled, verify that the user is not part of it.
        3) Handle the post request.
            a. Update nation stats in the database.
            b. Format the message and the components.
            c. Try to publish the post in the journalism channel.

    Parameters (variable_name / type / description):
        - bot      / dpp::cluster       / Client of the bot with all related information.
        - database / MYSQL*             / MineWorld database
        - event    / dpp::form_submit_t / All information about the event.

    Returns (type + description):
        No object returned.
*/
void Modals::journalism
(
    dpp::cluster             &bot,
    MYSQL*                   &database,
    const dpp::form_submit_t &event
)
{
    ////////////////// 1) //////////////////
    const std::string article_title = std::get<std::string>(event.components[0].value);
    const std::string article_content = std::get<std::string>(event.components[1].value);
    const std::string top_image_url = std::get<std::string>(event.components[2].value);
    const std::string bottom_image_url = std::get<std::string>(event.components[3].value);

    ////////////////// 2) //////////////////
    ///////// a. /////////
    const dpp::snowflake user_id = event.command.usr.id;
    Database::Output nationality = Database::db_query(database, "SELECT nation_id FROM nationality WHERE user_id = '" + std::to_string(user_id) + "' LIMIT 1");

    if (nationality.size() == 0)
    {
        event.reply(dpp::message(":prohibited: You can not perform this action being stateless.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// b. ////////
    const std::string nation_id = nationality[0]["nation_id"];
    Database::Output nations = Database::db_query(database, "SELECT display_name, government_type, media_blacklist, media_whitelist, media_posts, censored_posts, media_freedom FROM nations WHERE nation_id = '" + nation_id + "' LIMIT 1");

    if (nations.size() == 0)
    {
        event.reply(dpp::message(":prohibited: Nation `" + nation_id + "` does not exist.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// c. /////////
    Database::Output journalism = Database::db_query(database, "SELECT status FROM journalism WHERE user_id = '" + std::to_string(user_id) + "' LIMIT 1");
    int user_status = -1;

    if (journalism.size() != 0)
        user_status = std::stoi(journalism[0]["status"]);

    ///////// d. /////////
    const std::string whitelist = nations[0]["media_whitelist"];
    const std::string media_posts = std::to_string(std::stoll(nations[0]["media_posts"]) + 1);
    const std::string now = std::to_string(Miscellaneous::get_current_timestamp());
    const std::string censored_posts = std::to_string(std::stoll(nations[0]["censored_posts"]) + 1);
    const std::string display_name = nations[0]["display_name"];
    const std::string government_type = Text::get_government_type(std::stoi(nations[0]["government_type"]));

    if (whitelist == "1" && user_status != 0)
    {
        Database::db_query(database, "UPDATE nations SET media_posts = '" + media_posts + "', censored_posts = '" + censored_posts + "', last_a_censorship = '" + now + "' WHERE nation_id = '" + nation_id + "'");
        return event.reply(dpp::message(":prohibited: Your post has been **automatically censored** by the government of the " + government_type + " of " + display_name + ".").set_flags(dpp::m_ephemeral));;
    }

    ///////// e. /////////
    const std::string blacklist = nations[0]["media_blacklist"];
    const int media_freedom = std::stoi(nations[0]["media_freedom"]);

    if (blacklist == "1" && user_status == 1)
    {
        const std::string new_media_freedom = std::to_string(std::clamp(media_freedom - 1, 0, 100));

        Database::db_query(database, "UPDATE nations SET media_freedom = '" + new_media_freedom + "', media_posts = '" + media_posts + "', censored_posts = '" + censored_posts + "', last_a_censorship = '" + now + "' WHERE nation_id = '" + nation_id + "'");
        return event.reply(dpp::message(":prohibited: Your post has been **automatically censored** by the government of the " + government_type + " of " + display_name + ".").set_flags(dpp::m_ephemeral));;
    }

    ////////////////// 3) //////////////////
    ///////// a. /////////
    const int change = (whitelist == "1" ? 0 : 1);
    const std::string new_media_freedom = std::to_string(std::clamp(media_freedom + change, 0, 100));

    Database::db_query(database, "UPDATE nations SET media_freedom = '" + new_media_freedom + "', media_posts = '" + media_posts + "', last_post = '" + now + "' WHERE nation_id = '" + nation_id + "'");

    ///////// b. /////////
    const dpp::embed embed = dpp::embed()
    .set_color(dpp::colors::cream_white)\
    .set_description(article_content)
    .set_thumbnail(top_image_url)
    .set_image(bottom_image_url)
    .set_footer(dpp::embed_footer().set_text("Published by " + event.command.usr.username + " from the " + government_type + " of " + display_name + "."));

    const dpp::component buttons = dpp::component()
    .add_component (
        dpp::component()
        .set_label("Delete")
        .set_emoji(u8"🗑️")
        .set_type(dpp::cot_button)
        .set_style(dpp::cos_secondary)
        .set_id("journalism_delete")
    )
    .add_component (
        dpp::component()
        .set_label("Censor")
        .set_emoji(u8"🤫")
        .set_type(dpp::cot_button)
        .set_style(dpp::cos_secondary)
        .set_id("journalism_censor")
    )
    .add_component (
        dpp::component()
        .set_label("Blacklist")
        .set_emoji(u8"⚠️")
        .set_type(dpp::cot_button)
        .set_style(dpp::cos_danger)
        .set_id("journalism_blacklist")
    );

    ///////// c. /////////
    Database::Output config = Database::db_query(database, "SELECT journalism_channel FROM config LIMIT 1");

    if (config.size() == 0)
    {
        event.reply(dpp::message(":prohibited: No configuration is available to find the journalism channel.").set_flags(dpp::m_ephemeral));
        return;
    }

    const dpp::snowflake journalism_channel = dpp::snowflake(config[0]["journalism_channel"]);
    const dpp::snowflake guild_id = event.command.guild_id;
    const bool channel_exists = (dpp::find_channel(journalism_channel) -> guild_id == guild_id);

    if (!channel_exists)
    {
        event.reply(dpp::message(":prohibited: No channel `" + std::to_string(journalism_channel) + "` found in this guild.").set_flags(dpp::m_ephemeral));
        return;
    }

    bot.message_create(dpp::message(journalism_channel, "||" + std::to_string(user_id) + "." + nation_id + "||").add_embed(embed).add_component(buttons));
    event.reply(dpp::message(":newspaper: Your post has been published from the " + government_type + " of " + display_name + " in <#" + std::to_string(journalism_channel) + ">.").set_flags(dpp::m_ephemeral));
}
