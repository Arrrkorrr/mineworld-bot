#ifndef ENUM_IMMIGRATION_HPP
#define ENUM_IMMIGRATION_HPP

enum InvitePermissions
{
    EVERYONE = 0,
    GOVERNMENT_ONLY = 1,
    PRIME_MINISTER_MINIMUM = 2,
    LEADER_ONLY = 3
};

enum JoinConditions
{
    OPENED = 0,
    ON_INVITATION = 1,
    CLOSED = 2
};

#endif
