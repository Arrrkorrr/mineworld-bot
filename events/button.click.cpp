#include "events.hpp"

#include "../buttons/buttons.hpp"

#include <dpp/dpp.h>
#include <mysql/mysql.h>
#include <string>

/*
    Event that triggers when a button has been clicked.

    Tasks:
        1) Add the listener to the bot.
        2) Handle request depending on the button ID.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster / Client of the bot with all related information.
        - database  / MYSQL*       / MineWorld database

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
        const std::string ID = event.custom_id;

        if (ID == "journalism_censor" || ID == "journalism_blacklist")
            Buttons::journalism_censor(bot, database, event, ID);
        else if (ID == "journalism_delete")
            Buttons::journalism_delete(bot, event);
    });
}
