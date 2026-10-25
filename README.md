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
- **admin_shop** (<ins>listing_id</ins>, item_id, active, base_price, max_stock, current_stock, auto_restock, last_restock_time).
- **config** (member_role, welcome_channel, world_channel, journalism_channel, moderators_channel, mineworld_time).
- **economies** (<ins>nation_id</ins>, tux_balance).
- **gdp** (<ins>(nation_id, year)</ins>, gdp).
- **governments** (<ins>nation_id</ins>, government_type, ideology, leader_count, government_count, last_leader_change, last_government_change).
- **immigration** (<ins>nation_id</ins>, join_condition, invite_permission).
- **invitations** (<ins>(user_id, nation_id)</ins>, invited_by, creation_time).
- **items** (<ins>item_id</ins>, display_name, resolutions).
- **journalism** (<ins>user_id</ins>, status).
- **laws_resolutions** (<ins>resolution_id</ins>, title, details).
- **loans** (<ins>loan_id</ins>, nation_id, tux_amount, interests, paid, loan_time).
- **market** (<ins>listing_id</ins>, item_id, user_id, visibility, price, quantity, listing_creation_time).
- **media** (<ins>nation_id</ins>, freedom_rating, blacklist, whitelist, last_manual_censorship, last_automatic_censorship).
- **membership_resolutions** (<ins>resolution_id</ins>, invited_nation).
- **nationalities** (<ins>user_id</ins>, nation_id, rank, joining_time, last_rank_modification).
- **nations** (<ins>nation_id</ins>, display_name, description, deleted, role_id, creation_time).
- **news** (<ins>news_id</ins>, title, summary, article, release_date).
- **players** (<ins>user_id</ins>, username, password, discord_id, minecraft_uuid).
- **publications** (<ins>post_id</ins>, nation_id, user_id, message_id, visible, title, content, top_right_image_url, bottom_image_url, publication_time).
- **relations** (<ins>(nation_id, targeted_nation)</ins>, score).
- **resolutions** (<ins>resolution_id</ins>, pending, adopted, vetoed_by, nation_id, type, vote_beginning, vote_duration).
- **sanctions_resolutions** (<ins>resolution_id</ins>, targeted_nation, type, title, details, amount, duration).
- **shop_prices_history** (<ins>(listing_id, price_update_time)</ins>, base_price, actual_price).
- **united_nations** (<ins>nation_id</ins>, veto_state, veto_count, last_veto).
- **votes** (<ins>(resolution_id, nation_id)</ins>, voted_for, voting_time).
- **warfare** (<ins>nation_id</ins>, nuclear_state, nuclear_acquisition_time).

# 📥 Installation
1) Download the project. <br/>
2) Set the `bot.config` file. <br/>
3) Build and run using the `build.sh` script. <br/>
4) Enjoy! <br/>

# 🤝 User Agreement
By downloading and/or using this program, you confirm that you are solely responsible for how you use this software. You agree as well that this agreement extends to any prior version of the program, and any new version of the user agreement in any future update, overwrites this one.
