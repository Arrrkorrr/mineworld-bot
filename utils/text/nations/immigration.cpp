#include "nations.hpp"

#include "../../../enum/immigration.hpp"

#include <string>

/*
    Get full join condition name.

    Parameters (variable_name / type / description):
        - index / int / Join condition from database.

    Returns (type + description):
        A string containing the join condition name.
*/
std::string Nations::get_join_condition
(
    const int &index
)
{
    switch (index)
    {
        case OPENED:
            return "Opened";
        case ON_INVITATION:
            return "On Invitation";
        case CLOSED:
            return "Closed";
    }

    return "Unknown";
}



/*
    Get full invite permission name.

    Parameters (variable_name / type / description):
        - index / int / Invite permission from database.

    Returns (type + description):
        A string containing the invite permission name.
*/
std::string Nations::get_invite_permission
(
    const int &index
)
{
    switch (index)
    {
        case EVERYONE:
            return "Everyone";
        case GOVERNMENT_ONLY:
            return "Government Only";
        case PRIME_MINISTER_MINIMUM:
            return "Prime Minister / Head of State";
        case LEADER_ONLY:
            return "Head of State";
    }

    return "Unknown";
}