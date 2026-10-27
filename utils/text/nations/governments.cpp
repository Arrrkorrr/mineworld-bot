#include "nations.hpp"

#include "../../../enum/governments.hpp"

#include <string>

/*
    Get full government type name.

    Parameters (variable_name / type / description):
        - index / int / Government type from database.

    Returns (type + description):
        A string containing the government type name.
*/
std::string Nations::get_government_type
(
    const int &index
)
{
    switch (index)
    {
        case ANARCHY:
            return "Anarchy";
        case ARISTOCRACY:
            return "Aristocracy";
        case CONFEDERATION:
            return "Confederation";
        case CONSTITUTIONAL_MONARCHY:
            return "Constitutional Monarchy";
        case FEDERAL_REPUBLIC:
            return "Federal Republic";
        case MILITARY_RULE:
            return "Military Rule";
        case MONARCHY:
            return "Monarchy";
        case OLIGARCHY:
            return "Oligarchy";
        case ONE_PARTY_STATE:
            return "One-Party State";
        case PARLIAMENTARY_REPUBLIC:
            return "Parliamentary Republic";
        case PRESIDENTIAL_REPUBLIC:
            return "Presidential Republic";
    }

    return "Unknown";
}



/*
    Get full ideology name.

    Parameters (variable_name / type / description):
        - index / int / Ideology from database.

    Returns (type + description):
        A string containing the ideology name.
*/
std::string Nations::get_ideology
(
    const int &index
)
{
    switch (index)
    {
        case CENTRISM:
            return "Centrism";
        case COMMUNISM:
            return "Communism";
        case CONSERVATISM:
            return "Conservatism";
        case IMPERIALISM:
            return "Imperialism";
        case LIBERALISM:
            return "Liberalism";
        case MILITARISM:
            return "Militarism";
        case NATIONALISM:
            return "Nationalism";
        case NEUTRALISM:
            return "Neutralism";
        case PACIFISM:
            return "Pacifism";
        case SOCIALISM:
            return "Socialism";
    }

    return "Unknown";
}
