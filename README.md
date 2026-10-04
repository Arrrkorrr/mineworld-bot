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

# 🖥️ Database setup
``` sql
- Database "mineworld"
├── Table "config"
|  ├── member_role BIGINT DEFAULT 0
|  ├── welcome_channel BIGINT DEFAULT 0
|  ├── flags_url TEXT DEFAULT ''
|  ├── world_channel BIGINT DEFAULT 0
|  ├── journalism_channel BIGINT DEFAULT 0
├── Table "nations"
|  ├── nation_id BIGINT PRIMARY KEY AUTO_INCREMENT
|  ├── display_name VARCHAR(50) NOT NULL
|  ├── description VARCHAR(500) DEFAULT "No description." NOT NULL
|  ├── role_id BIGINT DEFAULT 0
|  ├── join_condition TINYINT DEFAULT 1
|  ├── invite_permission TINYINT DEFAULT 0
|  ├── creation_time BIGINT DEFAULT (UNIX_TIMESTAMP()) NOT NULL
|  ├── tux_balance BIGINT DEFAULT 0
|  ├── government_type TINYINT DEFAULT 0
|  ├── ideology TINYINT DEFAULT 8
|  ├── leadership_changes BIGINT DEFAULT 0
|  ├── last_leadership_change BIGINT DEFAULT 0
|  ├── government_changes BIGINT DEFAULT 0
|  ├── last_government_change BIGINT DEFAULT 0
|  ├── media_freedom TINYINT DEFAULT 100
|  ├── media_posts BIGINT DEFAULT 0
|  ├── last_post BIGINT DEFAULT 0
|  ├── censored_posts BIGINT DEFAULT 0
|  ├── media_whitelist BOOLEAN DEFAULT 0
|  ├── media_blacklist BOOLEAN DEFAULT 0
|  ├── last_manual_censorship BIGINT DEFAULT 0
|  ├── last_automatic_censorship BIGINT DEFAULT 0
|  ├── nuclear_state TINYINT DEFAULT 0
|  ├── acquired_nuclear_time BIGINT DEFAULT 0
|  ├── veto_state BOOLEAN DEFAULT 0
|  ├── veto_usage_count BIGINT DEFAULT 0
|  ├── last_veto_usage BIGINT DEFAULT 0
|  ├── resolutions_count BIGINT DEFAULT 0
|  ├── last_resolution BIGINT DEFAULT 0
|  ├── passed_resolutions BIGINT DEFAULT 0
|  ├── last_passed_resolution BIGINT DEFAULT 0
├── Table "nationality"
|  ├── user_id BIGINT PRIMARY KEY NOT NULL
|  ├── nation_id VARCHAR(50) DEFAULT 0
|  ├── rank TINYINT DEFAULT 0
|  ├── last_rank_update BIGINT DEFAULT 0
|  ├── joining_time BIGINT DEFAULT 0
├── Table "relations"
|  ├── defining_nation VARCHAR(50) NOT NULL
|  ├── targeted_nation VARCHAR(50) NOT NULL
|  ├── score TINYINT DEFAULT 50
|  ├── PRIMARY KEY (defining_nation, targeted_nation)
├── Table "sanctions"
|  ├── resolution_id BIGINT PRIMARY KEY AUTO_INCREMENT
|  ├── pending BOOLEAN DEFAULT 1
|  ├── nation_id VARCHAR(50) NOT NULL
|  ├── sanctioned_nation VARCHAR(50) NOT NULL
|  ├── sanction_type TINYINT
|  ├── sanction_title VARCHAR(100) NOT NULL
|  ├── sanction_details VARCHAR(500) NOT NULL
|  ├── sanction_amount BIGINT NOT NULL
|  ├── sanction_start BIGINT NOT NULL
|  ├── sanction_duration VARCHAR(3) NOT NULL
|  ├── sanction_end BIGINT NOT NULL
|  ├── vote_start BIGINT DEFAULT (UNIX_TIMESTAMP()) NOT NULL
|  ├── vote_duration VARCHAR(3) NOT NULL
|  ├── vote_end BIGINT NOT NULL
|  ├── votes_for TEXT DEFAULT ""
|  ├── votes_against TEXT DEFAULT ""
├── Table "laws"
|  ├── resolution_id BIGINT PRIMARY KEY AUTO_INCREMENT
|  ├── pending BOOLEAN DEFAULT 1
|  ├── nation_id VARCHAR(50) NOT NULL
|  ├── law_title VARCHAR(100) NOT NULL
|  ├── law_details VARCHAR(500) NOT NULL
|  ├── law_adoption_time BIGINT DEFAULT 0
|  ├── vote_start BIGINT DEFAULT (UNIX_TIMESTAMP()) NOT NULL
|  ├── vote_duration VARCHAR(3) NOT NULL
|  ├── vote_end BIGINT NOT NULL
|  ├── votes_for TEXT DEFAULT ""
|  ├── votes_against TEXT DEFAULT ""
├── Table "un_membership"
|  ├── nation_id VARCHAR(50) PRIMARY KEY NOT NULL
|  ├── pending BOOLEAN DEFAULT 1
|  ├── invited_by VARCHAR(50) NOT NULL
|  ├── joining_time BIGINT DEFAULT 0
|  ├── vote_start BIGINT DEFAULT (UNIX_TIMESTAMP()) NOT NULL
|  ├── vote_duration VARCHAR(3) NOT NULL
|  ├── vote_end BIGINT NOT NULL
|  ├── votes_for TEXT DEFAULT ""
|  ├── votes_against TEXT DEFAULT ""
├── Table "journalism"
|  ├── user_id BIGINT PRIMARY KEY NOT NULL
|  ├── status TINYINT
├── Table "invitations"
|  ├── user_id BIGINT NOT NULL
|  ├── nation_id VARCHAR(50) NOT NULL
|  ├── invited_by VARCHAR(50) NOT NULL
|  ├── creation_time BIGINT NOT NULL
|  ├── PRIMARY KEY (user_id, nation_id)
```

# 📥 Installation
1) Download the project. <br/>
2) Set the `bot.config` file. <br/>
3) Build and run using the `build.sh` script. <br/>
4) Enjoy! <br/>

# 🤝 User Agreement
By downloading and/or using this program, you confirm that you are solely responsible for how you use this software. You agree as well that this agreement extends to any prior version of the program, and any new version of the user agreement in any future update, overwrites this one.
