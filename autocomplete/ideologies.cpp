#include "autocomplete.hpp"

#include "../enum/governments.hpp"

#include <dpp/appcommand.h>
#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <dpp/snowflake.h>
#include <string>
#include <vector>

/*
    Auto complete for slash commands that need to display all ideologies.

    Tasks:
        1) Verify that the focused input is ideology.
        2) Display all existing ideologies to the user.

    Parameters (variable_name / type / description):
        - bot   / dpp::cluster        / Client of the bot with all related information.
        - event / dpp::autocomplete_t / All information about the event.

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
    const std::vector<dpp::command_option> command_inputs = event.options[0].options;
    bool ideology_input_focused = false;

    for (const dpp::command_option &option : command_inputs)
        if (option.focused && option.name == "ideology") ideology_input_focused = true;

    if (!ideology_input_focused)
        return;

    ////////////////// 2) //////////////////
    const dpp::snowflake command_id = event.command.id;
    const std::string command_token = event.command.token;

    bot.interaction_response_create
    (
        command_id,
        command_token,
        dpp::interaction_response(dpp::ir_autocomplete_reply)
            .add_autocomplete_choice(dpp::command_option_choice("Liberalism", std::to_string(LIBERALISM)))
            .add_autocomplete_choice(dpp::command_option_choice("Conservatism", std::to_string(CONSERVATISM)))
            .add_autocomplete_choice(dpp::command_option_choice("Socialism", std::to_string(SOCIALISM)))
            .add_autocomplete_choice(dpp::command_option_choice("Communism", std::to_string(COMMUNISM)))
            .add_autocomplete_choice(dpp::command_option_choice("Nationalism", std::to_string(NATIONALISM)))
            .add_autocomplete_choice(dpp::command_option_choice("Centrism", std::to_string(CENTRISM)))
            .add_autocomplete_choice(dpp::command_option_choice("Militarism", std::to_string(MILITARISM)))
            .add_autocomplete_choice(dpp::command_option_choice("Imperialism", std::to_string(IMPERIALISM)))
            .add_autocomplete_choice(dpp::command_option_choice("Pacifism", std::to_string(PACIFISM)))
            .add_autocomplete_choice(dpp::command_option_choice("Neutralism", std::to_string(NEUTRALISM)))
    );
}
