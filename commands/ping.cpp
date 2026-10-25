#include "commands.hpp"

#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <dpp/message.h>
#include <string>

/*
    Get the latency of the bot.

    Tasks:
        1) Get bot's Discord client using the first shard.
        2) Display bot latency.
            a. Get the latency from shard and convert it to ms.
            b. If the bot just started, the query may fail.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster              / Client of the bot with all related information.
        - event     / dpp::interaction_create_t / All information about the event.

    Returns (type + description):
        No object returned.
*/
void Commands::ping
(
    const dpp::cluster              &bot,
    const dpp::interaction_create_t &event
)
{
    ////////////////// 1) //////////////////
    const dpp::discord_client *shard = bot.get_shard(0);

    if (!shard)
    {
        event.reply(dpp::message(":prohibited: Something went wrong while getting shard.").set_flags(dpp::m_ephemeral));
        return;
    }

    ////////////////// 2) //////////////////
    ///////// a. /////////
    const double latency = shard -> websocket_ping * 1000;

    ///////// b. /////////
    if (latency <= 0)
    {
        event.reply(dpp::message(":prohibited: Gateway API is still warming up..").set_flags(dpp::m_ephemeral));
        return;
    }

    event.reply(dpp::message(":ping_pong: Bot latency: " + std::to_string((int) latency) + "ms.").set_flags(dpp::m_ephemeral));
}
