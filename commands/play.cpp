#include "commands.hpp"

#include <dpp/dpp.h>
#include <string>

/*
    Explains how to play to MineWorld.

    Tasks:
        1) Reply to the interaction.

    Parameters (variable_name / type / description):
        - event     / dpp::interaction_create_t / All information about the event.

    Returns (type + description):
        No object returned.
*/
void Commands::play
(
    const dpp::interaction_create_t &event
)
{
    event.reply
    (
        dpp::message
        (
            std::string("### How to play to Mineworld\n") +
            "- Create a new Forge instance `47.4.23` for Minecraft `1.20.1`.\n" +
            "- Download a mod pack from [here](https://mineworld.apkop.eu/play).\n" +
            "- Join the server at the address `51.75.140.147`."
        ).set_flags(dpp::m_ephemeral)
    );
}
