#ifndef ENUM_JOURNALISM_HPP
#define ENUM_JOURNALISM_HPP

enum Publishers
{
    WHITELISTED = 0,
    BLACKLISTED = 1
};

enum PostVisibility
{
    VISIBLE_POST = 0,
    REMOVED_POST = 1,
    CENSORED_POST = 2,
    POST_REMOVED_MODERATION = 3
};

enum FreedomRatingScores
{
    CENSORED_POST_SCORE_HIT = 3,
    BLACKLISTED_USER_SCORE_HIT = 5,
    FREE_POST_SCORE = 1
};

#endif
