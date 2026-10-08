DROP DATABASE IF EXISTS mineworld;

CREATE DATABASE mineworld;
USE mineworld;

CREATE TABLE config (
    member_role BIGINT DEFAULT 0,
    welcome_channel BIGINT DEFAULT 0,
    world_channel BIGINT DEFAULT 0,
    journalism_channel BIGINT DEFAULT 0
);

CREATE TABLE nations (
    nation_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    display_name VARCHAR(50) NOT NULL,
    description VARCHAR(500) DEFAULT "No description." NOT NULL,
    role_id BIGINT DEFAULT 0,
    join_condition TINYINT DEFAULT 1,
    invite_permission TINYINT DEFAULT 0,
    creation_time BIGINT DEFAULT 0,
    tux_balance BIGINT DEFAULT 0,
    government_type TINYINT DEFAULT 0,
    ideology TINYINT DEFAULT 8,
    leadership_changes BIGINT DEFAULT 0,
    last_leadership_change BIGINT DEFAULT 0,
    government_changes BIGINT DEFAULT 0,
    last_government_change BIGINT DEFAULT 0,
    media_freedom TINYINT DEFAULT 100,
    media_posts BIGINT DEFAULT 0,
    last_post BIGINT DEFAULT 0,
    censored_posts BIGINT DEFAULT 0,
    media_whitelist BOOLEAN DEFAULT 0,
    media_blacklist BOOLEAN DEFAULT 0,
    last_manual_censorship BIGINT DEFAULT 0,
    last_automatic_censorship BIGINT DEFAULT 0,
    nuclear_state TINYINT DEFAULT 0,
    acquired_nuclear_time BIGINT DEFAULT 0,
    veto_state BOOLEAN DEFAULT 0,
    veto_usage_count BIGINT DEFAULT 0,
    last_veto_usage BIGINT DEFAULT 0,
    resolutions_count BIGINT DEFAULT 0,
    last_resolution BIGINT DEFAULT 0,
    passed_resolutions BIGINT DEFAULT 0,
    last_passed_resolution BIGINT DEFAULT 0
);

CREATE TABLE nationality (
    user_id BIGINT PRIMARY KEY NOT NULL,
    nation_id VARCHAR(50) DEFAULT 0,
    rank TINYINT DEFAULT 0,
    last_rank_update BIGINT DEFAULT 0,
    joining_time BIGINT DEFAULT 0
);

CREATE TABLE relations (
    defining_nation VARCHAR(50) NOT NULL,
    targeted_nation VARCHAR(50) NOT NULL,
    score TINYINT DEFAULT 50,
    PRIMARY KEY (defining_nation, targeted_nation)
);

CREATE TABLE sanctions (
    resolution_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    pending BOOLEAN DEFAULT TRUE,
    nation_id VARCHAR(50) NOT NULL,
    sanctioned_nation VARCHAR(50) NOT NULL,
    sanction_type TINYINT,
    sanction_title VARCHAR(100) NOT NULL,
    sanction_details VARCHAR(500) NOT NULL,
    sanction_amount BIGINT NOT NULL,
    sanction_start BIGINT NOT NULL,
    sanction_duration VARCHAR(3) NOT NULL,
    sanction_end BIGINT NOT NULL,
    vote_start BIGINT DEFAULT (UNIX_TIMESTAMP()) NOT NULL,
    vote_duration VARCHAR(3) NOT NULL,
    vote_end BIGINT NOT NULL,
    votes_for TEXT DEFAULT "",
    votes_against TEXT DEFAULT ""
);

CREATE TABLE laws (
    resolution_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    pending BOOLEAN DEFAULT TRUE,
    nation_id VARCHAR(50) NOT NULL,
    law_title VARCHAR(100) NOT NULL,
    law_details VARCHAR(500) NOT NULL,
    law_adoption_time BIGINT DEFAULT 0,
    vote_start BIGINT DEFAULT (UNIX_TIMESTAMP()) NOT NULL,
    vote_duration VARCHAR(3) NOT NULL,
    vote_end BIGINT NOT NULL,
    votes_for TEXT DEFAULT "",
    votes_against TEXT DEFAULT ""
);

CREATE TABLE un_membership (
    nation_id VARCHAR(50) PRIMARY KEY NOT NULL,
    pending BOOLEAN DEFAULT TRUE,
    invited_by VARCHAR(50) NOT NULL,
    joining_time BIGINT DEFAULT 0,
    vote_start BIGINT DEFAULT (UNIX_TIMESTAMP()) NOT NULL,
    vote_duration VARCHAR(3) NOT NULL,
    vote_end BIGINT NOT NULL,
    votes_for TEXT DEFAULT "",
    votes_against TEXT DEFAULT ""
);

CREATE TABLE journalism (
    user_id BIGINT PRIMARY KEY NOT NULL,
    status TINYINT
);

CREATE TABLE invitations (
    user_id BIGINT NOT NULL,
    nation_id VARCHAR(50) NOT NULL,
    invited_by VARCHAR(50) NOT NULL,
    creation_time BIGINT NOT NULL,
    PRIMARY KEY (user_id, nation_id)
);
