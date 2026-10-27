#include "events.hpp"

#include "../utils/logs/logs.hpp"

#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <string>

/*
    Event triggered when a member leaves.

    Tasks:
        1) Add listener to bot.
        2) Log the departure.

    Parameters (variable_name / type / description):
        - bot / dpp::cluster / Client of the bot with all related information.

    Returns (type + description):
        No object returned.
*/
void Events::guild_member_remove
(
    dpp::cluster &bot
)
{
    ////////////////// 1) //////////////////
    bot.on_guild_member_remove([&bot](const dpp::guild_member_remove_t &event)
    {
        ////////////////// 2) //////////////////
        const std::string member_discord_id = std::to_string(event.removed.id);
        Logs::log("Member left -> " + member_discord_id + ".");
    });
}
