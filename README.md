# MineWorld - Discord bot
Source code of MineWorld, a private Discord bot. <br/>
This project is based on the [D++ library](https://github.com/brainboxdotcc/DPP). <br/>
Discord server using the bot: https://discord.gg/zkJ75UDVEV. <br/>

# 📦 Requirements
- CMake.
- MariaDB server.

# ⚙️ Configuration
In the `./config` server, you must create the file `bot.config` and set the following config in there. <br/>
``` config
TOKEN=bot_token_here
DB_NAME=mineworld
DB_HOST=database_address
DB_PORT=database_port
DB_USER=database_username
DB_PASSWORD=database_password
```

# 🖥️ Database
- **config** (member_role, welcome_channel, world_channel, journalism_channel).
- **invitations** (<ins>(user_id, nation_id)</ins>, invited_by, creation_time).
- **journalism** (<ins>user_id</ins>, status).
- **laws** (<ins>resolution_id</ins>, pending, nation_id, law_title, law_details, law_adoption_time, vote_start, vote_duration, vote_end, votes_for, votes_against)
- **nationality** (<ins>user_id</ins>, nation_id, rank, last_rank_update, joining_time).
- **nations** (<ins>nation_id</ins>, display_name, description, role_id, join_condition, invite_permission, creation_time, tux_balance, government_type, ideology, leadership_changes, 
               last_leadership_change, government_changes, last_government_change, media_freedom, media_posts, last_post, censored_posts, media_whitelist, media_blacklist, 
               last_manual_censorship, last_automatic_censorship, nuclear_state, acquired_nuclear_time, veto_state, veto_usage_count, last_veto_usage, resolutions_count, last_resolution, 
               passed_resolutions, last_passed_resolution).
- **news** (<ins>news_id</ins>, title, content).
- **relations** (<ins>(defining_nation, targeted_nation)</ins>, score).
- **sanctions** (<ins>resolution_id</ins>, pending, nation_id, sanctioned_nation, sanction_type, sanction_title, sanction_details, sanction_amount, sanction_start, sanction_duration, 
                 sanction_end,vote_start, vote_duration, vote_end, votes_for, votes_against).
- **un_membership** (<ins>nation_id</ins>, pending, invited_by, joining_time, vote_start, vote_duration, vote_end, votes_for, votes_against).

# 📥 Installation
1) Download the project. <br/>
2) Set the `bot.config` file. <br/>
3) Build and run using the `build.sh` script. <br/>
4) Enjoy! <br/>

# 🤝 User Agreement
By downloading and/or using this program, you confirm that you are solely responsible for how you use this software. You agree as well that this agreement extends to any prior version of the program, and any new version of the user agreement in any future update, overwrites this one.
