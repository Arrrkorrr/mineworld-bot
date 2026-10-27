#include "events.hpp"

#include "../buttons/buttons.hpp"

#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <mysql/mysql.h>
#include <string>

/*
    Event triggered when a button has been pressed.

    Tasks:
        1) Add listener to bot.
        2) Handle request depending on button ID.

    Parameters (variable_name / type / description):
        - bot      / dpp::cluster / Client of the bot with all related information.
        - database / MYSQL*       / MineWorld database.

    Returns (type + description):
        No object returned.
*/
void Events::button_click
(
    dpp::cluster &bot,
    MYSQL*       &database
)
{
    ////////////////// 1) //////////////////
    bot.on_button_click([&bot, &database](dpp::button_click_t event)
    {
        ////////////////// 2) //////////////////
        const std::string button_id = event.custom_id;

        if (button_id == "journalism_blacklist")
            Buttons::Journalism::journalism_blacklist(bot, database, event);
        else if (button_id == "journalism_censor")
            Buttons::Journalism::journalism_censor(bot, database, event);
        else if (button_id == "journalism_delete")
            Buttons::Journalism::journalism_delete(bot, database, event);
    });
}
