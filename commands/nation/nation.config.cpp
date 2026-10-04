#include "nation.hpp"

#include <dpp/dpp.h>
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
    event.reply(dpp::message(":prohibited: Coming soon!").set_flags(dpp::m_ephemeral));
}
