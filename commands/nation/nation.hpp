#ifndef COMMANDS_NATION_HPP
#define COMMANDS_NATION_HPP

#include <dpp/dpp.h>
#include <mysql/mysql.h>

namespace Nation
{
    ////////////////////////////
    ///// nation.create.cpp /////
    ////////////////////////////

    void nation_create
    (
        dpp::cluster                    &bot,
        MYSQL*                          &database,
        const dpp::interaction_create_t &event
    );

    /////////////////////////////
    ///// nation.config.cpp /////
    /////////////////////////////

    void nation_config
    (
        dpp::cluster                    &bot,
        MYSQL*                          &database,
        const dpp::interaction_create_t &event
    );

    ///////////////////////////
    ///// nation.info.cpp /////
    ///////////////////////////

    void nation_info
    (
        dpp::cluster                    &bot,
        MYSQL*                          &database,
        const dpp::interaction_create_t &event
    );

    /////////////////////////////
    ///// nation.invite.cpp /////
    /////////////////////////////

    void nation_invite
    (
        dpp::cluster                    &bot,
        MYSQL*                          &database,
        const dpp::interaction_create_t &event
    );

    ///////////////////////////
    ///// nation.join.cpp /////
    ///////////////////////////

    void join_nation
    (
        dpp::cluster                    &bot,
        MYSQL*                          &database,
        const dpp::interaction_create_t &event
    );

    ///////////////////////////
    ///// nation.kick.cpp /////
    ///////////////////////////

    void nation_kick
    (
        dpp::cluster                    &bot,
        MYSQL*                          &database,
        const dpp::interaction_create_t &event
    );

    ////////////////////////////
    ///// nation.leave.cpp /////
    ////////////////////////////

    void leave_nation
    (
        dpp::cluster                    &bot,
        MYSQL*                          &database,
        const dpp::interaction_create_t &event
    );

    ///////////////////////////
    ///// nation.rank.cpp /////
    ///////////////////////////

    void nation_rank
    (
        dpp::cluster                    &bot,
        MYSQL*                          &database,
        const dpp::interaction_create_t &event
    );

    ///////////////////////////////
    ///// nation.relation.cpp /////
    ///////////////////////////////

    void nation_relation
    (
        dpp::cluster                    &bot,
        MYSQL*                          &database,
        const dpp::interaction_create_t &event
    );
}

#endif
