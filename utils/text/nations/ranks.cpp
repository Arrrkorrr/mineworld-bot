#include "nations.hpp"

#include "../../../enum/ranks.hpp"

#include <string>

/*
    Get full rank name.

    Parameters (variable_name / type / description):
        - index / int / Rank index from database.

    Returns (type + description):
        A string containing the rank name.
*/
std::string Nations::get_rank
(
    const int &index
)
{
    switch (index)
    {
        case CITIZEN:
            return "Citizen";
        case MILITARY:
            return "Military";
        case MINISTER:
            return "Minister";
        case PRIME_MINISTER:
            return "Prime Minister";
        case HEAD_OF_STATE:
            return "Head of State";
    }

    return "Unknown";
}