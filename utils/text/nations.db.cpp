#include "text.hpp"

#include <string>
#include <vector>

/*
    Get full government type name.

    Tasks:
        1) Declare all government types available.
        2) Try to get a government type using the index provided.

    Parameters (variable_name / type / description):
        - index / int / Government type from database.

    Returns (type + description):
        A string containing the government type name.
*/
std::string Text::get_government_type
(
    const int &index
)
{
    ////////////////// 1) //////////////////
    const std::vector<std::string> governments
    {
        "Presidential Republic",   "Parliamentary Republic",  "Federal Republic", "Monarchy",
        "Constitutional Monarchy", "Confederation",           "Military Rule",    "Anarchy",
        "Oligarchy",               "Aristocracy",            "One-Party State"
    };

    ////////////////// 2) //////////////////
    const std::string type = governments[index];
    return type;
}



/*
    Get full ideology name.

    Tasks:
        1) Declare all ideologies available.
        2) Try to get an ideology using the index provided.

    Parameters (variable_name / type / description):
        - index / int / Ideology from database.

    Returns (type + description):
        A string containing the ideology name.
*/
std::string Text::get_ideology
(
    const int &index
)
{
    ////////////////// 1) //////////////////
    const std::vector<std::string> ideologies
    {
        "Liberalism", "Conservatism", "Socialism",   "Communism", "Nationalism",
        "Centrism",   "Militarism",   "Imperialism", "Pacifism",  "Neutralism"
    };

    ////////////////// 2) //////////////////
    const std::string ideology = ideologies[index];
    return ideology;
}


/*
    Get full join condition name.

    Tasks:
        1) Declare all join conditions available.
        2) Try to get a value depending on the index provided.

    Parameters (variable_name / type / description):
        - index / int / Join condition from database.

    Returns (type + description):
        A string containing the join condition name.
*/
std::string Text::get_join_condition
(
    const int &index
)
{
    ////////////////// 1) //////////////////
    const std::vector<std::string> condition_values { "Opened", "On Invitation", "Closed" };

    ////////////////// 2) //////////////////
    const std::string output = condition_values[index];
    return output;
}



/*
    Get full invite permission name.

    Tasks:
        1) Declare all invite permissions available.
        2) Try to get a value depending on the index provided.

    Parameters (variable_name / type / description):
        - index / int / Invite permission from database.

    Returns (type + description):
        A string containing the invite permission name.
*/
std::string Text::get_invite_permission
(
    const int &index
)
{
    ////////////////// 1) //////////////////
    const std::vector<std::string> invite_permissions
    {
        "Everyone", "Government Only", "Prime Minister+", "Head of State Only"
    };

    ////////////////// 2) //////////////////
    const std::string permission = invite_permissions[index];
    return permission;
}



/*
    Get full rank name.

    Tasks:
        1) Declare all ranks available.
        2) Try to get a rank depending on the index provided.

    Parameters (variable_name / type / description):
        - index / int / Rank index from database.

    Returns (type + description):
        A string containing the rank name.
*/
std::string Text::get_rank
(
    const int &index
)
{
    ////////////////// 1) //////////////////
    const std::vector<std::string> ranks
    {
        "Citizen", "Soldier", "Minister", "Prime Minister", "Head of State"
    };

    ////////////////// 2) //////////////////
    const std::string rank = ranks[index];
    return rank;
}



/*
    Get a media freedom rating from the score.

    Tasks:
        1) Check the score value and return a rating depending on it.

    Parameters (variable_name / type / description):
        - score / int / Media freedom score.

    Returns (type + description):
        A string containing the media freedom rating.
*/
std::string Text::get_media_freedom_rating
(
    const int &score
)
{
    if (score <= 25)
        return "Inexistant";
    else if (score <= 50)
        return "High censorship";
    else if (score <= 70)
        return "Complicated";
    else if (score <= 85)
        return "Good";
    else if (score <= 99)
        return "Very good";
    else return "Excellent";
}



/*
    Get a relation rating between two nations from the score.

    Tasks:
        1) Check the score value and return a rating depending on it.

    Parameters (variable_name / type / description):
        - score / int / Nations relation score.

    Returns (type + description):
        A string containing the relation rating.
*/
std::string Text::get_relation_rating
(
    const int &score
)
{
    if (score <= 10)
        return "Hostility";
    else if (score <= 20)
        return "Hatred";
    else if (score <= 40)
        return "Tensions";
    else if (score <= 60)
        return "Neutrality";
    else if (score <= 80)
        return "Cordiality";
    else if (score <= 90)
        return "Friendship";
    else return "Harmony";
}
