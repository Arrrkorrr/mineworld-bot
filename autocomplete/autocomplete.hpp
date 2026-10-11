#ifndef AUTOCOMPLETE_HPP
#define AUTOCOMPLETE_HPP

#include <dpp/dpp.h>
#include <mysql/mysql.h>

namespace Autocomplete
{
    ///////////////////////
    ///// nations.cpp /////
    ///////////////////////

    void nations
    (
        dpp::cluster              &bot,
        MYSQL*                    &database,
        const dpp::autocomplete_t &event
    );

    ////////////////////////////////
    ///// government.types.cpp /////
    ////////////////////////////////

    void government_types
    (
        dpp::cluster              &bot,
        const dpp::autocomplete_t &event
    );

    //////////////////////////
    ///// ideologies.cpp /////
    //////////////////////////

    void ideologies
    (
        dpp::cluster              &bot,
        const dpp::autocomplete_t &event
    );

    /////////////////////
    ///// ranks.cpp /////
    /////////////////////

    void ranks
    (
        dpp::cluster              &bot,
        const dpp::autocomplete_t &event
    );
}

#endif
