#include "autocomplete.hpp"

#include <dpp/dpp.h>

/*
    Auto complete slash commands that need to display all ideologies.

    Tasks:
        1) Verify that it is the correct parameter being focused and typed into.
        2) Display all existing ideologies to the user.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster        / Client of the bot with all related information.
        - event     / dpp::autocomplete_t / All information about the event.

    Returns (type + description):
        No object returned.
*/
void Autocomplete::ideologies
(
    dpp::cluster              &bot,
    const dpp::autocomplete_t &event
)
{
    ////////////////// 1) //////////////////
    bool focused = false;

    for (const auto &option : event.options[0].options)
        if (option.focused && option.name == "ideology") focused = true;

    if (!focused)
        return;

    ////////////////// 2) //////////////////
    bot.interaction_response_create
    (
        event.command.id,
        event.command.token,
        dpp::interaction_response(dpp::ir_autocomplete_reply)
            .add_autocomplete_choice(dpp::command_option_choice("Liberalism", "0"))
            .add_autocomplete_choice(dpp::command_option_choice("Conservatism", "1"))
            .add_autocomplete_choice(dpp::command_option_choice("Socialism", "2"))
            .add_autocomplete_choice(dpp::command_option_choice("Communism", "3"))
            .add_autocomplete_choice(dpp::command_option_choice("Nationalism", "4"))
            .add_autocomplete_choice(dpp::command_option_choice("Centrism", "5"))
            .add_autocomplete_choice(dpp::command_option_choice("Militarism", "6"))
            .add_autocomplete_choice(dpp::command_option_choice("Imperialism", "7"))
            .add_autocomplete_choice(dpp::command_option_choice("Pacifism", "8"))
            .add_autocomplete_choice(dpp::command_option_choice("Neutralism", "9"))
    );
}
