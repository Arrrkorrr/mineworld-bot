#include "events.hpp"

#include <dpp/dpp.h>

/*
    Event that triggers when a new message is sent on Discord.

    Tasks:
        1) Add the listener to the bot.
        2) Handle messages depending on the message content.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster / Client of the bot with all related information.

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
        if (event.msg.content == "<@1469410323776667679>")
            event.reply(":wave: The bot is online and functioning!");
    });
}
