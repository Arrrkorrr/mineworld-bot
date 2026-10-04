#ifndef UTILS_TEXT_HPP
#define UTILS_TEXT_HPP

#include <string>

namespace Text
{
    /////////////////////////
    ///// lowercase.cpp /////
    /////////////////////////

    std::string to_lowercase
    (
        const std::string &input
    );

    //////////////////////////
    ///// nations.db.cpp /////
    //////////////////////////

    std::string get_government_type
    (
        const int &index
    );

    std::string get_ideology
    (
        const int &index
    );

    std::string get_join_condition
    (
        const int &index
    );

    std::string get_invite_permission
    (
        const int &index
    );

    std::string get_rank
    (
        const int &index
    );

    std::string get_media_freedom_rating
    (
        const int &score
    );

    std::string get_relation_rating
    (
        const int &score
    );

    ////////////////////
    ///// trim.cpp /////
    ////////////////////

    std::string trim
    (
        const std::string &input
    );
}

#endif
