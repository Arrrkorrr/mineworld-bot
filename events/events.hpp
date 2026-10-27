#ifndef EVENTS_HPP
#define EVENTS_HPP

#include <dpp/cluster.h>
#include <dpp/dpp.h>
#include <mysql/mysql.h>

namespace Events
{
    ////////////////////////////
    ///// autocomplete.cpp /////
    ////////////////////////////

    void autocomplete
    (
        dpp::cluster &bot,
        MYSQL*       &database
    );

    ////////////////////////////
    ///// button.click.cpp /////
    ////////////////////////////

    void button_click
    (
        dpp::cluster &bot,
        MYSQL*       &database
    );

    ///////////////////////////
    ///// form.submit.cpp /////
    ///////////////////////////

    void form_submit
    (
        dpp::cluster &bot,
        MYSQL*       &database
    );

    ////////////////////////////////
    ///// guild.member.add.cpp /////
    ////////////////////////////////

    void guild_member_add
    (
        dpp::cluster &bot,
        MYSQL*       &database
    );

    ///////////////////////////////////
    ///// guild.member.remove.cpp /////
    ///////////////////////////////////

    void guild_member_remove
    (
        dpp::cluster &bot
    );

    //////////////////////////////////
    ///// interaction.create.cpp /////
    //////////////////////////////////

    void interaction_create
    (
        dpp::cluster &bot,
        MYSQL*       &database
    );

    //////////////////////////////
    ///// message.create.cpp /////
    //////////////////////////////

    void message_create
    (
        dpp::cluster &bot
    );

    /////////////////////
    ///// ready.cpp /////
    /////////////////////

    void ready
    (
        dpp::cluster &bot
    );

    ////////////////////////////
    ///// select.menus.cpp /////
    ////////////////////////////

    void select_menu_click
    (
        dpp::cluster &bot,
        MYSQL*       &database
    );
}

#endif
