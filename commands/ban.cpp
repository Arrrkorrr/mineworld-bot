#include "commands.hpp"

#include "../utils/logs/logs.hpp"
#include "../utils/miscellaneous/miscellaneous.hpp"

#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <dpp/guild.h>
#include <dpp/message.h>
#include <dpp/permissions.h>
#include <dpp/restresults.h>
#include <dpp/snowflake.h>
#include <string>

/*
    Ban a member from the Discord server.

    Tasks:
        1) Get command parameters.
        2) Some verification.
            a. Verify that the executer are not trying to ban themselves.
            b. Verify that the executer is not trying to ban the bot.
            c. Get some information about the target within the guild.
            d. Get some information about the guild.
            e. Verify that the executer is not trying to ban the guild owner.
            f. Verify that the executer has required permissions to ban.
            g. Verify that the executer is the server owner in order to ban administrators.
            h. Verify that the executer is higher than the target in the role hierarchy.
            i. Get bot permissions within the guild.
            j. Get some information about the bot within the guild.
            k. Verify that the bot has required permissions to ban.
        3) Ban the target.
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
    const dpp::snowflake target_discord_id = std::get<dpp::snowflake>(event.get_parameter("member"));
    const std::string ban_reason = std::get<std::string>(event.get_parameter("reason"));
    const bool is_silent_ban = std::get<bool>(event.get_parameter("silent"));

    ////////////////// 2) //////////////////
    ///////// a. //////////
    const dpp::snowflake executer_discord_id = event.command.usr.id;

    if (target_discord_id == executer_discord_id)
    {
        event.reply(dpp::message(":prohibited: You are not allowed to ban yourself.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// b. //////////
    const dpp::snowflake bot_discord_id = bot.me.id;

    if (target_discord_id == bot_discord_id)
    {
        event.reply(dpp::message("<:putin_gun:1516736231357153491> I WILL NOT GO DOWN WITHOUT A FIGHT!").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// c. //////////
    const auto target_member_iterator = event.command.resolved.members.find(target_discord_id);

    if (target_member_iterator == event.command.resolved.members.end())
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
    const dpp::snowflake owner_discord_id = guild -> owner_id;

    if (target_discord_id == owner_discord_id)
    {
        event.reply(dpp::message(":prohibited: You can not ban the guild owner.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// f. //////////
    const dpp::guild_member executer_guild_member = event.command.member;
    const dpp::permission executer_permissions = guild -> base_permissions(executer_guild_member);

    if (!(executer_permissions & dpp::p_ban_members))
    {
        event.reply(dpp::message(":prohibited: You do not have the required permissions to run this command.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// g. //////////
    const dpp::guild_member target_guild_member = target_member_iterator -> second;
    const dpp::permission target_permissions = guild -> base_permissions(target_guild_member);

    if ((target_permissions & dpp::p_administrator) && executer_discord_id != owner_discord_id)
    {
        event.reply(dpp::message(":prohibited: Only the guild owner is allowed to ban administrators.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// h. //////////
    const uint8_t highest_target_role = Miscellaneous::highest_role_position(target_guild_member);
    const uint8_t highest_executer_role = Miscellaneous::highest_role_position(executer_guild_member);

    if (highest_target_role >= highest_executer_role && executer_discord_id != owner_discord_id)
    {
        event.reply(dpp::message(":prohibited: You can not ban this member due to role hierarchy.").set_flags(dpp::m_ephemeral));
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
    const auto bot_member_iterator = guild -> members.find(bot_discord_id);

    if (bot_member_iterator == guild -> members.end())
    {
        event.reply(dpp::message(":prohibited: Failed to find the bot in guild data.").set_flags(dpp::m_ephemeral));
        return;
    }

    ///////// k. //////////
    const dpp::guild_member bot_guild_member = bot_member_iterator -> second;
    const uint8_t highest_bot_role = Miscellaneous::highest_role_position(bot_guild_member);

    if (highest_target_role >= highest_bot_role)
    {
        event.reply(dpp::message(":prohibited: I can not ban this member due to role hierarchy.").set_flags(dpp::m_ephemeral));
        return;
    }

    ////////////////// 3) //////////////////
    ///////// a. //////////
    bot.set_audit_reason("[" + std::to_string(executer_discord_id) + "] " + ban_reason);

    ///////// b. //////////
    bot.guild_ban_add(guild_id, target_discord_id, 0, [=](const dpp::confirmation_callback_t &callback)
    {
        ///////// c. //////////
        if (callback.is_error())
        {
            event.reply(dpp::message(":prohibited: Something went wrong while banning the member.").set_flags(dpp::m_ephemeral));
            return;
        }

        event.reply(dpp::message(":man_judge: <@" + std::to_string(target_discord_id) + "> has been banned!").set_flags(is_silent_ban ? dpp::m_ephemeral : 0));
        Logs::log("[ban] " + std::to_string(executer_discord_id) + " banned " + std::to_string(target_discord_id) + " from " + std::to_string(guild_id) +  " for \"" + ban_reason + "\".");
    });
}
