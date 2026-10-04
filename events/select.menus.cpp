#include "events.hpp"

#include "../menus/menus.hpp"
#include "../utils/logs/logs.hpp"

#include <dpp/dpp.h>
#include <mysql/mysql.h>
#include <string>

/*
    Event that triggers when a select menu option has been clicked.

    Tasks:
        1) Add the listener to the bot.
        2) Handle the request depending on the select menu ID.
        3) Handle the request depending on the menu value.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster / Client of the bot with all related information.
        - database  / MYSQL*       / MineWorld database

    Returns (type + description):
        No object returned.
*/
void Events::select_menu_click
(
    dpp::cluster &bot,
    MYSQL*       &database
)
{
    ////////////////// 1) //////////////////
    bot.on_select_click([&bot, &database](dpp::select_click_t event)
    {
        ////////////////// 2) //////////////////
        const std::string ID = event.custom_id;
        const std::string option = event.values[0];

        if (ID == "nation_info")
        {
            ////////////////// 3) //////////////////
            if (option == "media")
                Menus::NationInfo::media(bot, database, event);
            else if (option == "overview")
                Menus::NationInfo::overview(bot, database, event);
            else if (option == "politics")
                Menus::NationInfo::politics(bot, database, event);
            else if (option == "warfare")
                Menus::NationInfo::warfare(bot, database, event);
            else if (option == "united_nations")
                Menus::NationInfo::united_nations(bot, database, event);
            else if (option == "close")
                bot.message_delete(event.command.msg.id, event.command.channel_id);
        }
    });
}
