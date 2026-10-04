#include "autocomplete.hpp"

#include "../utils/database/database.hpp"

#include <dpp/dpp.h>
#include <map>
#include <mysql/mysql.h>
#include <string>

/*
    Auto complete slash commands that need to display all existing nations.

    Tasks:
        1) Get the data to display back to the user.
            a. Browse the list of event options looking for the focused option.
            b. If we find a focused option, verify it's the correct one and register the data typed in it.
            c. Format the query to make to the database, and use the data typed by the user to make a search.
            d. Make the query to the database.
        2) Display the data to the user.
            a. Format all data that came from the request.
            b. Return the formatted data to the user.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster        / Client of the bot with all related information.
        - database  / MYSQL*              / MineWorld database
        - event     / dpp::autocomplete_t / All information about the event.

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
    std::string focused_value;

    for (const dpp::command_option &option : event.options[0].options)
    {
        if (option.focused)
        {
            ///////// b. /////////
            if (option.name != "nation_id")
                return;

            focused_value = std::get<std::string>(option.value);
            break;
        }
    }

    ///////// c. /////////
    std::string query = "SELECT nation_id, display_name FROM nations states";

    if (!focused_value.empty())
    {
        focused_value = Database::sanitize_input(database, focused_value);
        query += " WHERE (states.nation_id LIKE '%" + focused_value + "%' OR states.display_name LIKE '%" + focused_value + "%')";
    }

    query += " ORDER BY states.display_name ASC LIMIT 25";

    ///////// d. /////////
    Database::Output empty_nations = Database::db_query(database, query);

    ////////////////// 2) //////////////////
    ///////// a. /////////
    dpp::interaction_response output(dpp::ir_autocomplete_reply);

    for (std::map<std::string, std::string> &empty_nation : empty_nations)
    {
        const std::string nation_id = empty_nation["nation_id"];
        const std::string display_name = empty_nation["display_name"];

        output.add_autocomplete_choice(dpp::command_option_choice(display_name + " (" + nation_id + ")", nation_id));
    }

    ///////// b. /////////
    bot.interaction_response_create(event.command.id, event.command.token, output);
}
