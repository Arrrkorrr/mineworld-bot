#include "events.hpp"

#include "../menus/menus.hpp"

#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <mysql/mysql.h>
#include <string>

/*
    Event that triggers when a select menu option has been clicked.

    Tasks:
        1) Add listener to bot.
        2) Handle the request depending on select menu ID.
        3) Handle the request depending on selected option.

    Parameters (variable_name / type / description):
        - bot      / dpp::cluster / Client of the bot with all related information.
        - database / MYSQL*       / MineWorld database.

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
        const std::string select_menu_id = event.custom_id;
        const std::string selected_option = event.values[0];

        if (select_menu_id == "nation_info")
        {
            ////////////////// 3) //////////////////
            if (selected_option == "media")
                Menus::NationInfo::media(bot, database, event);
            else if (selected_option == "overview")
                Menus::NationInfo::overview(bot, database, event);
            else if (selected_option == "politics")
                Menus::NationInfo::politics(bot, database, event);
            else if (selected_option == "warfare")
                Menus::NationInfo::warfare(bot, database, event);
            else if (selected_option == "united_nations")
                Menus::NationInfo::united_nations(bot, database, event);
            else if (selected_option == "close")
                bot.message_delete(event.command.msg.id, event.command.channel_id);
        }
    });
}
