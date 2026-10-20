#ifndef BUTTONS_JOURNALISM_HPP
#define BUTTONS_JOURNALISM_HPP

#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <mysql/mysql.h>
#include <string>

///////////////////////////////////////////////////
//////////////////// Structure ////////////////////
///////////////////////////////////////////////////

struct BlacklistInfo
{
    const std::string publisher_id;
    const int freedom_rating;
    const std::string nation_id;
};

///////////////////////////////////////////////////
//////////////////// Namespace ////////////////////
///////////////////////////////////////////////////

namespace Journalism
{
    ////////////////////////////////////
    ///// journalism.blacklist.cpp /////
    ////////////////////////////////////

    void journalism_blacklist
    (
        dpp::cluster        &bot,
        MYSQL*              &database,
        dpp::button_click_t &event
    );

    /////////////////////////////////
    ///// journalism.censor.cpp /////
    /////////////////////////////////

    BlacklistInfo journalism_censor
    (
        dpp::cluster        &bot,
        MYSQL*              &database,
        dpp::button_click_t &event
    );

    /////////////////////////////////
    ///// journalism.delete.cpp /////
    /////////////////////////////////

    void journalism_delete
    (
        dpp::cluster        &bot,
        MYSQL*              &database,
        dpp::button_click_t &event
    );
}

#endif
