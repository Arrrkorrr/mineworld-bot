#include "commands.hpp"

#include "../utils/logs/logs.hpp"
#include "../utils/miscellaneous/miscellaneous.hpp"

#include <dpp/dpp.h>
#include <string>

/*
    Ban a member from the Discord server.

    Tasks:
        1) Get command parameters.
        2) We do some verification.
            a. Verify that the user and executer IDs do not match.
            b. Verify that the executer is not trying to ban the bot.
            c. Try to get some information about the user within the guild.
            d. Try to get some information about the guild.
            e. Verify that the user is not trying to ban the guild owner.
            f. Verify that the executer has the required permissions to perform a ban themselves.
            g. Verify that the executer is the server owner in order to be able to ban an administrator.
            h. Verify that the executer highest role is higher than the user highest role in the hierarchy.
            i. Try to get the bot permissions within the guild.
            j. Try to get some information about the bot within the guild.
            k. Verify that the bot has the required permissions to perform a ban.
        3) We process the ban request.
            a. Register the ban reason in audit logs.
            b. Perform the ban.
            c. Verify that the ban did not fail.

    Parameters (variable_name / type / description):
        - bot       / dpp::cluster              / Client of the bot with all related information.
        - event     / dpp::interaction_create_t / All information about the event.

    Returns (type + description):
        No object returned.
*/
void Commands::ban
(
    dpp::cluster                    &bot,
    const dpp::interaction_create_t &event
)
{
    ////////////////// 1) //////////////////
    const dpp::snowflake user_id = std::get<dpp::snowflake>(event.get_parameter("member"));
    const std::string reason = std::get<std::string>(event.get_parameter("reason"));
    const bool silent = std::get<bool>(event.get_parameter("silent"));

    ////////////////// 2) //////////////////
    ///////// a. //////////
    const dpp::snowflake executer_id = event.command.usr.id;

    if (user_id == executer_id)
    {
        event.reply(dpp::message(":prohibited: You are not allowed to ban yourself.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// b. //////////
    if (user_id == bot.me.id)
    {
        event.reply(dpp::message("<:putin_gun:1516736231357153491> I WILL NOT GO DOWN WITHOUT A FIGHT!").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// c. //////////
    const auto member_it = event.command.resolved.members.find(user_id);

    if (member_it == event.command.resolved.members.end())
    {
        event.reply(dpp::message(":prohibited: This member was not found in this guild.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// d. //////////
    const dpp::snowflake guild_id = event.command.guild_id;
    const dpp::guild *guild = dpp::find_guild(guild_id);

    if (!guild)
    {
        event.reply(dpp::message(":prohibited: Something went wrong while retrieving guild data.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// e. //////////
    if (user_id == guild -> owner_id)
    {
        event.reply(dpp::message(":prohibited: You can not ban the guild owner.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// f. //////////
    const dpp::guild_member executer = event.command.member;
    const dpp::permission executer_permissions = guild -> base_permissions(executer);

    if (!(executer_permissions & dpp::p_ban_members))
    {
        event.reply(dpp::message(":prohibited: You do not have the required permissions to run this command.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// g. //////////
    const dpp::guild_member member = member_it -> second;
    const dpp::permission member_permissions = guild -> base_permissions(member);

    if ((member_permissions & dpp::p_administrator) && executer_id != guild -> owner_id)
    {
        event.reply(dpp::message(":prohibited: Only the guild owner is allowed to ban administrators.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// h. //////////
    const uint8_t highest_member_role = Miscellaneous::highest_role_position(member);
    const uint8_t highest_executer_role = Miscellaneous::highest_role_position(executer);

    if (highest_member_role >= highest_executer_role && executer_id != guild -> owner_id)
    {
        event.reply(dpp::message(":prohibited: You can not ban this member.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// i. //////////
    const dpp::permission bot_permissions = event.command.app_permissions;

    if (!(bot_permissions & dpp::p_ban_members))
    {
        event.reply(dpp::message(":prohibited: I do not have the required permissions to ban.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// j. //////////
    const auto bot_member_it = guild -> members.find(bot.me.id);

    if (bot_member_it == guild -> members.end())
    {
        event.reply(dpp::message(":prohibited: Failed to find the bot in guild data.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// k. //////////
    const dpp::guild_member bot_member = bot_member_it -> second;
    const uint8_t highest_bot_role = Miscellaneous::highest_role_position(bot_member);

    if (highest_member_role >= highest_bot_role)
    {
        event.reply(dpp::message(":prohibited: I can not ban this member.").set_flags(dpp::m_ephemeral));
        return;
    }

    ////////////////// 3) //////////////////
    ///////// a. //////////
    bot.set_audit_reason("[" + std::to_string(executer_id) + "] " + reason);

    ///////// b. //////////
    bot.guild_ban_add(guild_id, user_id, 0, [=](const dpp::confirmation_callback_t &callback)
    {
        ///////// c. //////////
        if (callback.is_error())
        {
            event.reply(dpp::message(":prohibited: Something went wrong while banning the member.").set_flags(dpp::m_ephemeral));
            return;
        }

        event.reply(dpp::message(":man_judge: <@" + std::to_string(user_id) + "> has been banned!").set_flags(silent ? dpp::m_ephemeral : 0));
        Logs::log("[ban] " + std::to_string(executer_id) + " banned " + std::to_string(user_id) + " from " + std::to_string(guild_id) +  " for \"" + reason + "\".");
    });
}
