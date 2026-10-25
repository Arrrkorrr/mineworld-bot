#include "journalism.hpp"

#include "../../enum/journalism.hpp"
#include "../../utils/database/database.hpp"

#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <mysql/mysql.h>
#include <string>

/*
    Delete a post made in the journalism channel and blacklist its publisher.

    Tasks:
        1) Delete the post and retrieve some information. If user is an empty string, it means that the deletion failed.
        2) Blacklist the user.
            a. Verify that the deletion succeeded by verifying the retrieved information.
            b. Either INSERT or UPDATE user journalism status as blacklisted.
            c. Hit the nation media freedom rating.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster        / Client of the bot with all related information.
        - database  / MYSQL*              / MineWorld database.
        - event     / dpp::button_click_t / All information about the event.

    Returns (type + description):
        No object returned.
*/
void Journalism::journalism_blacklist
(
    dpp::cluster        &bot,
    MYSQL*              &database,
    dpp::button_click_t &event
)
{
    ////////////////// 1) //////////////////
    const BlacklistInfo information = Journalism::journalism_censor(bot, database, event);
    const std::string publisher_id = information.publisher_id;

    if (publisher_id == "")
        return;

    ////////////////// 2) //////////////////
    ///////// a. /////////
    const int freedom_rating = information.freedom_rating;
    const std::string nation_id = information.nation_id;

    if (freedom_rating == -1 || nation_id == "")
        return;

    ///////// b. /////////
    Database::Output user_registered = Database::db_query(database, "SELECT 1 FROM journalism WHERE user_id = '" + publisher_id + "' LIMIT 1");

    if (user_registered.size() == 0)
        Database::db_query(database, "INSERT INTO journalism VALUES ('" + publisher_id + "', 1)");
    else Database::db_query(database, "UPDATE journalism SET status = '" + std::to_string(BLACKLISTED) + "' WHERE user_id = '" + publisher_id + "'");

    ///////// c. /////////
    const int new_freedom_rating = std::clamp(freedom_rating - BLACKLISTED_USER_SCORE_HIT, 0, 100);
    Database::db_query(database, "UPDATE media SET media_freedom = '" + std::to_string(new_freedom_rating) + "' WHERE nation_id = '" + nation_id + "'");
}
