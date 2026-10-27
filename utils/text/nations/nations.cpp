#include "nations.hpp"

#include "../../../enum/nations.hpp"

#include <string>

/*
    Get relation rating between two nations from their score.

    Parameters (variable_name / type / description):
        - score / int / Nations relation score.

    Returns (type + description):
        A string containing the relation rating.
*/
std::string Nations::get_relation_rating
(
    const int &score
)
{
    if (score <= HOSTILITY)
        return "Hostility";
    else if (score <= HATRED)
        return "Hatred";
    else if (score <= TENSIONS)
        return "Tensions";
    else if (score <= NEUTRALITY)
        return "Neutrality";
    else if (score <= CORDIALITY)
        return "Cordiality";
    else if (score <= FRIENDSHIP)
        return "Friendship";
    else if (score <= HARMONY)
        return "Harmony";
    else return "Unknown";
}
