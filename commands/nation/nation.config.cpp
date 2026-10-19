#include "nation.hpp"

#include "../../utils/database/database.hpp"

#include <dpp/dpp.h>
#include <dpp/message.h>
#include <dpp/snowflake.h>
#include <mysql/mysql.h>

/*
    Edit configuration of a nation.

    Tasks:
        1)

    Parameters:
        - bot       / dpp::cluster              / Client of the bot with all related information.
        - database  / MYSQL*                    / MineWorld Database.
        - event     / dpp::interaction_create_t / Event information.

    Returns:
        No object returned.
*/
void Nation::nation_config
(
    dpp::cluster                    &bot,
    MYSQL*                          &database,
    const dpp::interaction_create_t &event
)
{
    const dpp::snowflake user_id = event.command.usr.id;
    Database::Output nationality = Database::db_query(database, "SELECT nation_id FROM nationality WHERE user_id = " + std::to_string(user_id) + " LIMIT 1");

    if (nationality.size() == 0)
    {
        event.reply(dpp::message(":warning: You can not perform this action while being stateless!").set_flags(dpp::m_ephemeral));
        return;
    }

    event.reply(dpp::message(":prohibited: Coming soon!").set_flags(dpp::m_ephemeral));
}
