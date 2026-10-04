#include "autocomplete.hpp"

#include <dpp/dpp.h>

/*
    Auto complete slash commands that need to display all government types.

    Tasks:
        1) Verify that it is the correct parameter being focused and typed into.
        2) Display all existing government types to the user.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster        / Client of the bot with all related information.
        - event     / dpp::autocomplete_t / All information about the event.

    Returns (type + description):
        No object returned.
*/
void Autocomplete::government_types
(
    dpp::cluster              &bot,
    const dpp::autocomplete_t &event
)
{
    ////////////////// 1) //////////////////
    for (const auto &option : event.options)
        if (option.focused && option.name != "government_type") return;

    ////////////////// 2) //////////////////
    bot.interaction_response_create
    (
        event.command.id,
        event.command.token,
        dpp::interaction_response(dpp::ir_autocomplete_reply)
            .add_autocomplete_choice(dpp::command_option_choice("Presidential Republic", "0"))
            .add_autocomplete_choice(dpp::command_option_choice("Parliamentary Republic", "1"))
            .add_autocomplete_choice(dpp::command_option_choice("Federal Republic", "2"))
            .add_autocomplete_choice(dpp::command_option_choice("Monarchy", "3"))
            .add_autocomplete_choice(dpp::command_option_choice("Constitutional Monarchy", "4"))
            .add_autocomplete_choice(dpp::command_option_choice("Confederation", "5"))
            .add_autocomplete_choice(dpp::command_option_choice("Military Rule", "6"))
            .add_autocomplete_choice(dpp::command_option_choice("Anarchy", "7"))
            .add_autocomplete_choice(dpp::command_option_choice("Oligarchy", "8"))
            .add_autocomplete_choice(dpp::command_option_choice("Aristocracy", "9"))
            .add_autocomplete_choice(dpp::command_option_choice("One-Party State", "10"))
    );
}
