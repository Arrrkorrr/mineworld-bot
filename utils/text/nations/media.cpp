#include "nations.hpp"

#include "../../../enum/media.hpp"

#include <string>

/*
    Get a media freedom rating from the score.

    Parameters (variable_name / type / description):
        - score / int / Media freedom score.

    Returns (type + description):
        A string containing the media freedom rating.
*/
std::string Nations::get_media_freedom_rating
(
    const int &score
)
{
    if (score <= INEXISTENT)
        return "Inexistent";
    else if (score <= HIGH_CENSORSHIP)
        return "High censorship";
    else if (score <= COMPLICATED)
        return "Complicated";
    else if (score <= GOOD)
        return "Good";
    else if (score <= VERY_GOOD)
        return "Very good";
    else if (score <= EXCELLENT)
        return "Excellent";
    else return "Unknown";
}
