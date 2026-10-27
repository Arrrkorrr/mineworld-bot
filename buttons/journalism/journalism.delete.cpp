#include "journalism.hpp"

#include "../../enum/journalism.hpp"
#include "../../utils/database/database.hpp"

#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <dpp/message.h>
#include <mysql/mysql.h>
#include <string>

/*
    Delete a post made in the journalism channel.

    Tasks:
        1) Some verification.
            a. Verify that the executer has a MineWorld account linked to their Discord account.
            b. Retrieve the post ID from the message and try to retrieve its publisher ID.
            c. Verify that the publisher ID and command executer ID are similar.
        2) Remove the post.
            a. Delete the post and buttons.
            b. Edit message content to a censorship notice.
            c. Set post visibility to removed.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster        / Client of the bot with all related information.
        - database  / MYSQL*              / MineWorld database.
        - event     / dpp::button_click_t / All information about the event.

    Returns (type + description):
        No object returned.
*/
void Journalism::journalism_delete
(
    dpp::cluster        &bot,
    MYSQL*              &database,
    dpp::button_click_t &event
)
{
    ////////////////// 1) //////////////////
    ///////// a. /////////
    const std::string executer_discord_id = std::to_string(event.command.usr.id);
    Database::Output executer_credentials = Database::db_query(database, "SELECT user_id FROM players WHERE discord_id = '" + executer_discord_id + "' LIMIT 1");

    if (executer_credentials.size() == 0)
    {
        event.reply(dpp::message(":prohibited: No MineWorld account is linked to your Discord account. Create a MineWorld account using `/account register` or link your already existing account with `/account discord`.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// b. /////////
    const int message_length = event.command.msg.content.size();
    const std::string post_id = event.command.msg.content.substr(2, message_length - 4);

    Database::Output post = Database::db_query(database, "SELECT user_id FROM publications WHERE post_id = '" + post_id + "' LIMIT 1");

    if (post.size() == 0)
    {
        event.reply(dpp::message(":prohibited: Could not retrieve any information about this post.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// c. /////////
    const std::string executer_user_id = executer_credentials[0]["user_id"];
    const std::string publisher_user_id = post[0]["user_id"];

    if (publisher_user_id != executer_user_id)
    {
        event.reply(dpp::message(":prohibited: You can not delete the posts published by other people.").set_flags(dpp::m_ephemeral));
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

    ///////// c. /////////
    Database::db_query(database, "UPDATE publications SET visibility = '" + std::to_string(REMOVED_POST) + "' WHERE post_id = '" + post_id + "'");

    event.reply();
}
