#include "miscellaneous.hpp"

#include <dpp/dpp.h>
#include <dpp/role.h>
#include <dpp/snowflake.h>

/*
    Return position of the highest role of a guild member.

    Tasks:
        1) Setup the starting highest position to 0 (no role).
        2) Analyze member roles.
            a. Check each role position by ID.
            b. If it's highest role found yet, we register it as it.

    Parameters (variable_name / type / description):
        - member / dpp::guild_member / Targeted member of the guild.

    Returns (type + description):
        A uint8_t containing the position index of highest role of this member.
*/
uint8_t Miscellaneous::highest_role_position
(
    const dpp::guild_member &member
)
{
    ////////////////// 1) //////////////////
    uint8_t highest_role_position = 0;

    ////////////////// 2) //////////////////
    ///////// a. /////////
    for (const dpp::snowflake &role_id : member.get_roles())
    {
        const dpp::role *role = dpp::find_role(role_id);
        const uint8_t role_position = role -> position;

        ///////// b. /////////
        if (role && role_position > highest_role_position)
            highest_role_position = role_position;
    }

    return highest_role_position;
}
