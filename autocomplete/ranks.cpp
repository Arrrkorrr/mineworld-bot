#include "autocomplete.hpp"

#include <dpp/appcommand.h>
#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <dpp/snowflake.h>
#include <string>
#include <vector>

/*
    Auto complete slash commands that need to display all ranks.

    Tasks:
        1) Verify that the focused input is rank.
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
    const std::vector<dpp::command_option> command_inputs = event.options[0].options;
    bool rank_input_focused = false;

    for (const dpp::command_option &option : command_inputs)
        if (option.focused && option.name == "new_rank") rank_input_focused = true;

    if (!rank_input_focused)
        return;

    ////////////////// 2) //////////////////
    const dpp::snowflake command_id = event.command.id;
    const std::string command_token = event.command.token;

    bot.interaction_response_create
    (
        command_id,
        command_token,
        dpp::interaction_response(dpp::ir_autocomplete_reply)
            .add_autocomplete_choice(dpp::command_option_choice("Citizen", "0"))
            .add_autocomplete_choice(dpp::command_option_choice("Military", "1"))
            .add_autocomplete_choice(dpp::command_option_choice("Minister", "2"))
            .add_autocomplete_choice(dpp::command_option_choice("Prime Minister", "3"))
            .add_autocomplete_choice(dpp::command_option_choice("Head of State", "4"))
    );
}
