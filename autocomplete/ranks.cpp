#include "autocomplete.hpp"

#include <dpp/dpp.h>

/*
    Auto complete slash commands that need to display all ranks.

    Tasks:
        1) Verify that it is the correct parameter being focused and typed into.
        2) Display all existing ranks to the user.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster        / Client of the bot with all related information.
        - event     / dpp::autocomplete_t / All information about the event.

    Returns (type + description):
        No object returned.
*/
void Autocomplete::ranks
(
    dpp::cluster              &bot,
    const dpp::autocomplete_t &event
)
{
    ////////////////// 1) //////////////////
    bool focused = false;

    for (const auto &option : event.options[0].options)
        if (option.focused && option.name == "new_rank") focused = true;

    if (!focused)
        return;

    ////////////////// 2) //////////////////
    bot.interaction_response_create
    (
        event.command.id,
        event.command.token,
        dpp::interaction_response(dpp::ir_autocomplete_reply)
            .add_autocomplete_choice(dpp::command_option_choice("Citizen", "0"))
            .add_autocomplete_choice(dpp::command_option_choice("Military", "1"))
            .add_autocomplete_choice(dpp::command_option_choice("Minister", "2"))
            .add_autocomplete_choice(dpp::command_option_choice("Prime Minister", "3"))
            .add_autocomplete_choice(dpp::command_option_choice("Head of State", "4"))
    );
}
