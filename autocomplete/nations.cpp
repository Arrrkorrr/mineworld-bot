#include "autocomplete.hpp"

#include "../enum/nations.hpp"
#include "../utils/database/database.hpp"

#include <dpp/appcommand.h>
#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <dpp/snowflake.h>
#include <map>
#include <mysql/mysql.h>
#include <string>

/*
    Auto complete for slash commands that need to display all existing nations.

    Tasks:
        1) Get the data to display back to the user.
            a. Browse the list of the command inputs looking for the focused input.
            b. Verify that the focused input is nation_id and get the data typed in it.
            c. Format the query to make to the database.
            d. Make the query to the database.
        2) Display the data to the user.
            a. Format the data that came from the request.
            b. Return data to the user.

    Parameters (variable_name / type / description):
        - bot      / dpp::cluster        / Client of the bot with all related information.
        - database / MYSQL*              / MineWorld database.
        - event    / dpp::autocomplete_t / All information about the event.

    Returns (type + description):
        No object returned.
*/
void Autocomplete::nations
(
    dpp::cluster              &bot,
    MYSQL*                    &database,
    const dpp::autocomplete_t &event
)
{
    ////////////////// 1) //////////////////
    ///////// a. /////////
    std::string focused_input_data;

    for (const dpp::command_option &option : event.options[0].options)
    {
        if (option.focused)
        {
            ///////// b. /////////
            if (option.name == "nation_id")
            {
                const std::string value = std::get<std::string>(option.value);
                focused_input_data = Database::sanitize_input(database, value);
            }
            else return;

            break;
        }
    }

    ///////// c. /////////
    std::string query = "SELECT nation_id, display_name FROM nations";

    if (!focused_input_data.empty())
        query += " WHERE deleted = '" + std::to_string(EXISTING_NATION) + "' (nation_id LIKE '%" + focused_input_data + "%' OR display_name LIKE '%" + focused_input_data + "%')";

    query += " ORDER BY display_name ASC LIMIT 25";

    ///////// d. /////////
    Database::Output nations = Database::db_query(database, query);

    ////////////////// 2) //////////////////
    ///////// a. /////////
    dpp::interaction_response auto_complete_data(dpp::ir_autocomplete_reply);

    for (std::map<std::string, std::string> &nation : nations)
    {
        const std::string nation_name = nation["display_name"];
        const std::string nation_id = nation["nation_id"];

        auto_complete_data.add_autocomplete_choice(dpp::command_option_choice(nation_name + " (" + nation_id + ")", nation_id));
    }

    ///////// b. /////////
    const dpp::snowflake command_id = event.command.id;
    const std::string command_token = event.command.token;

    bot.interaction_response_create(command_id, command_token, auto_complete_data);
}
