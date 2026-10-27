#include "events.hpp"

#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>

/*
    Event triggered when a new message is sent on Discord.

    Tasks:
        1) Add listener to bot.
        2) Handle messages depending on message content.

    Parameters (variable_name / type / description):
        - bot / dpp::cluster / Client of the bot with all related information.

    Returns (type + description):
        No object returned.
*/
void Events::message_create
(
    dpp::cluster &bot
)
{
    ////////////////// 1) //////////////////
    bot.on_message_create([&bot](const dpp::message_create_t &event)
    {
        ////////////////// 2) //////////////////
        const std::string message_content = event.msg.content;

        if (message_content == "<@1469410323776667679>")
            event.reply(":wave: The bot is online and functioning!");
    });
}
