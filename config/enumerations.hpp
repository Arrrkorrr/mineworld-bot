#ifndef ENUMERATIONS_HPP
#define ENUMERATIONS_HPP

enum InvitePermissions
{
    EVERYONE = 0,
    GOV_ONLY = 1,
    PM_MINIMUM = 2,
    LEADER_ONLY = 3
};

enum JoinConditions
{
    OPENED = 0,
    ON_INVITATION = 1,
    CLOSED = 2
};

enum Journalism
{
    WHITELIST = 0,
    BLACKLIST = 1
};

enum Ranks
{
    CITIZEN = 0,
    MILITARY = 1,
    MINISTER = 2,
    PRIME_MINISTER = 3,
    LEADER = 4
};

enum Sanctions
{
    TRADE_PENALTY = 0,
    IMPORT_BAN = 1,
    EXPORT_BAN = 2,
    TRADE_BAN = 3,
    FINE = 4,
    VETO_ABUSE = 5,
    EVENTS_BAN = 6
};

#endif
