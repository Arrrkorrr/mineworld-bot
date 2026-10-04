#include "events.hpp"

#include "../modals/modals.hpp"

#include <dpp/dpp.h>
#include <mysql/mysql.h>
#include <string>

/*
    Event that triggers when a modal has been submitted.

    Tasks:
        1) Add the listener to the bot.
        2) Handle request depending on the modal ID.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster / Client of the bot with all related information.
        - database  / MYSQL*       / MineWorld database

    Returns (type + description):
        No object returned.
*/
void Events::form_submit
(
    dpp::cluster &bot,
    MYSQL*       &database
)
{
    ////////////////// 1) //////////////////
    bot.on_form_submit([&bot, &database](const dpp::form_submit_t &event)
    {
        ////////////////// 2) //////////////////
        const std::string ID = event.custom_id;

        if (ID == "journalism_modal")
            Modals::journalism(bot, database, event);
    });
}
