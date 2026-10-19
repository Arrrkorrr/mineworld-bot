#include "autocomplete.hpp"

#include <dpp/appcommand.h>
#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <dpp/snowflake.h>
#include <string>
#include <vector>

/*
    Auto complete for slash commands that need to display all government types.

    Tasks:
        1) Verify that the focused input is government_type.
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
    const std::vector<dpp::command_option> command_inputs = event.options[0].options;
    bool government_type_input_focused = false;

    for (const dpp::command_option &option : command_inputs)
        if (option.focused && option.name == "government_type") government_type_input_focused = true;

    if (!government_type_input_focused)
        return;

    ////////////////// 2) //////////////////
    const dpp::snowflake command_id = event.command.id;
    const std::string command_token = event.command.token;

    bot.interaction_response_create
    (
        command_id,
        command_token,
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
