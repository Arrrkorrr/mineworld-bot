#include "commands.hpp"

#include <dpp/dpp.h>
#include <string>

/*
    Display rules to the user.

    Tasks:
        1) Reply to the interaction.

    Parameters (variable_name / type / description):
        - event     / dpp::interaction_create_t / All information about the event.

    Returns (type + description):
        No object returned.
*/
void Commands::rules
(
    const dpp::interaction_create_t &event
)
{
    event.reply
    (
        dpp::message
        (
            std::string("## :judge: Rules\n\n") +
            ":warning: The staff and owners of this project are **NOT responsible** in case of losses, doxxing, scams, harassment or ANY **harmful actions** committed by **non-staff members**, but we will take actions accordingly.\n" +
            "The staff can **sanction** and **censor** members, or **take decisions** at their **own discretion**. The MineWorld bot can also **automatically censor** content.\n" +
            "But if you feel that a sanction was **totally unfair**/**unjustified**, you can contest at `arkorbyte@proton.me`.\n\n" +
            "### :speech_left: Discord server\n" +
            "- You must comply with **Discord Terms of Service** and **Community Guidelines** that you may find [here](https://discord.com/terms).\n" +
            "- **Impersonating** staff members is **STRICTLY PROHIBITED**.\n" +
            "- Using `@everyone` and `@here` are **reserved** to **staff ONLY**.\n" +
            "- Talk in the **right channels**, off-topic conversations may be **deleted**.\n" +
            "- Due to **moderation concerns**, any message written in any language that is not **English**, **French** or **Russian** may be removed (*this list depends on staff proficiency in certain languages and may be updated*).\n" +
            "- Otherwise, anything that is not **EXPLICITLY marked** as prohibited is **IMPLICITLY allowed**.\n\n" +
            "### :pick: MineWorld\n" +
            "MineWorld aims to be a **semi-anarchist** server to grant as much **freedom of actions** to the players. But to keep the server **playable** and **fair**:\n" +
            "- Using **cheats**, **automated clients** or **custom mods** that modify the behavior of your game and/or give unfair gameplay advantages are **STRICTLY PROHIBITED**.\n" +
            "- **Optimization mods** such as OptiFine or Embeddium, and **texture packs**/**shaders** are **AUTHORIZED**.\n" +
            "- In-game **glitches** and **exploits** are NOT prohibited but **may be patched** by the development team at their own discretion.\n" +
            "- Purposefully making the server **crash** and **attempts of piracy** are **STRICTLY PROHIBITED**.\n" +
            "- Otherwise, anything that is not **EXPLICITLY marked** as prohibited is **IMPLICITLY allowed**."
        ).set_flags(dpp::m_ephemeral)
    );
}
