#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "nation/nation.hpp"

#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>

namespace Commands
{
    ///////////////////
    ///// ban.cpp /////
    ///////////////////

    void ban
    (
        dpp::cluster                    &bot,
        const dpp::interaction_create_t &event
    );

    //////////////////////////
    ///// journalism.cpp /////
    //////////////////////////

    void journalism
    (
        const dpp::interaction_create_t &event
    );

    /////////////////////
    ///// kick.cpp //////
    /////////////////////

    void kick
    (
        dpp::cluster                    &bot,
        const dpp::interaction_create_t &event
    );

    ////////////////////
    ///// ping.cpp /////
    ////////////////////

    void ping
    (
        const dpp::cluster              &bot,
        const dpp::interaction_create_t &event
    );

    ////////////////////
    ///// play.cpp /////
    ////////////////////

    void play
    (
        const dpp::interaction_create_t &event
    );

    /////////////////////
    ///// rules.cpp /////
    /////////////////////

    void rules
    (
        const dpp::interaction_create_t &event
    );

    //////////////////////
    ///// nation.hpp /////
    //////////////////////

    namespace Nation = ::Nation;
}

#endif
