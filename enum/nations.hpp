#ifndef ENUMERATIONS_HPP
#define ENUMERATIONS_HPP

enum NationStatus
{
    EXISTING_NATION = 0,
    COLLAPSED_NATION = 1,
    NATION_REMOVED_MODERATION = 2
};

enum NationRelationRatings
{
    HOSTILITY = 10,
    HATRED = 20,
    TENSIONS = 40,
    NEUTRALITY = 60,
    CORDIALITY = 80,
    FRIENDSHIP = 90,
    HARMONY = 100
};

#endif
