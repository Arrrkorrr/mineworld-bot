#ifndef UTILS_TEXT_NATIONS_HPP
#define UTILS_TEXT_NATIONS_HPP

#include <string>

namespace Nations
{
    ///////////////////////////
    ///// governments.cpp /////
    ///////////////////////////

    std::string get_government_type
    (
        const int &index
    );

    std::string get_ideology
    (
        const int &index
    );

    ///////////////////////////
    ///// immigration.cpp /////
    ///////////////////////////

    std::string get_join_condition
    (
        const int &index
    );

    std::string get_invite_permission
    (
        const int &index
    );

    /////////////////////
    ///// media.cpp /////
    /////////////////////

    std::string get_media_freedom_rating
    (
        const int &score
    );

    ///////////////////////
    ///// nations.cpp /////
    ///////////////////////

    std::string get_relation_rating
    (
        const int &score
    );

    /////////////////////
    ///// ranks.cpp /////
    /////////////////////

    std::string get_rank
    (
        const int &index
    );
}

#endif
