#ifndef UTILS_TEXT_HPP
#define UTILS_TEXT_HPP

#include "nations/nations.hpp"

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

    ////////////////////
    ///// trim.cpp /////
    ////////////////////

    std::string trim
    (
        const std::string &input
    );

    ///////////////////////////////
    ///// nations/nations.hpp /////
    ///////////////////////////////

    namespace Nations = ::Nations;
}

#endif
