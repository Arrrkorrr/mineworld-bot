DROP DATABASE IF EXISTS mineworld;

CREATE DATABASE mineworld;
USE mineworld;



CREATE TABLE config
(
    member_role BIGINT DEFAULT NULL,
    welcome_channel BIGINT DEFAULT NULL,
    world_channel BIGINT DEFAULT NULL,
    journalism_channel BIGINT DEFAULT NULL,
    mineworld_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL
);



CREATE TABLE news
(
    news_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    title VARCHAR(50) NOT NULL,
    summary TEXT NOT NULL,
    article TEXT NOT NULL,
    release_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL
);





CREATE TABLE players
(
    user_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    username VARCHAR(50) NOT NULL UNIQUE,
    password VARCHAR(128) NOT NULL,
    discord_id BIGINT NOT NULL UNIQUE,
    minecraft_uuid VARCHAR(36) NOT NULL UNIQUE
);



CREATE TABLE journalism
(
    user_id BIGINT PRIMARY KEY,
    status TINYINT CHECK (status BETWEEN 0 AND 1)
);



CREATE TABLE invitations
(
    user_id BIGINT NOT NULL,
    nation_id BIGINT NOT NULL,
    invited_by BIGINT NOT NULL,
    creation_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL,
    PRIMARY KEY(user_id, nation_id)
);





CREATE TABLE nations
(
    nation_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    display_name VARCHAR(50) NOT NULL UNIQUE,
    description VARCHAR(500) DEFAULT "No description." NOT NULL,
    deleted TINYINT CHECK (deleted BETWEEN 0 AND 2) DEFAULT 0 NOT NULL,
    role_id BIGINT NOT NULL UNIQUE,
    creation_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL
);



CREATE TABLE nationalities
(
    user_id BIGINT PRIMARY KEY,
    nation_id BIGINT NOT NULL,
    rank TINYINT CHECK (rank BETWEEN 0 AND 4) DEFAULT 0,
    joining_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL,
    last_rank_modification TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL
);





CREATE TABLE economies
(
    nation_id BIGINT PRIMARY KEY,
    tux_balance BIGINT DEFAULT 0 NOT NULL
);



CREATE TABLE gdp
(
    nation_id BIGINT NOT NULL,
    gdp BIGINT NOT NULL,
    year BIGINT NOT NULL,
    PRIMARY KEY(nation_id, year)
);



CREATE TABLE loans
(
    loan_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    nation_id BIGINT NOT NULL,
    tux_amount BIGINT NOT NULL,
    interests TINYINT CHECK (interests BETWEEN 5 AND 50) NOT NULL,
    paid BOOLEAN DEFAULT false NOT NULL,
    loan_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL
);





CREATE TABLE governments
(
    nation_id BIGINT PRIMARY KEY,
    government_type TINYINT CHECK (government_type BETWEEN 0 AND 10) NOT NULL,
    ideology TINYINT CHECK (ideology BETWEEN 0 AND 9) NOT NULL,
    leader_count BIGINT DEFAULT 1 NOT NULL,
    government_count BIGINT DEFAULT 1 NOT NULL,
    last_leader_change TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL,
    last_government_change TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL
);



CREATE TABLE warfare
(
    nation_id BIGINT PRIMARY KEY,
    nuclear_state BOOLEAN DEFAULT false NOT NULL,
    nuclear_acquisition_time TIMESTAMP DEFAULT NULL
);



CREATE TABLE immigration
(
    nation_id BIGINT PRIMARY KEY,
    join_condition TINYINT CHECK (join_condition BETWEEN 0 AND 2) DEFAULT 1 NOT NULL,
    invite_permission TINYINT CHECK (invite_permission BETWEEN 0 AND 3) DEFAULT 0 NOT NULL
);



CREATE TABLE relations
(
    nation_id BIGINT,
    targeted_nation BIGINT,
    score TINYINT CHECK (score BETWEEN 0 AND 100) NOT NULL,
    PRIMARY KEY(nation_id, targeted_nation)
);





CREATE TABLE media
(
    nation_id BIGINT PRIMARY KEY,
    freedom_rating TINYINT CHECK (freedom_rating BETWEEN 0 AND 100) DEFAULT 100 NOT NULL,
    blacklist BOOLEAN DEFAULT false NOT NULL,
    whitelist BOOLEAN DEFAULT false NOT NULL,
    last_manual_censorship TIMESTAMP DEFAULT NULL,
    last_automatic_censorship TIMESTAMP DEFAULT NULL
);



CREATE TABLE publications
(
    post_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    nation_id BIGINT NOT NULL,
    user_id BIGINT NOT NULL,
    message_id BIGINT NOT NULL,
    visible TINYINT CHECK (visible BETWEEN 0 AND 3) NOT NULL,
    title VARCHAR(50) NOT NULL,
    content VARCHAR(1000) NOT NULL,
    top_right_image_url TEXT,
    bottom_image_url TEXT,
    publication_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL
);





CREATE TABLE united_nations
(
    nation_id BIGINT PRIMARY KEY,
    veto_state BOOLEAN DEFAULT false NOT NULL,
    veto_count BIGINT DEFAULT 0 NOT NULL,
    last_veto TIMESTAMP DEFAULT NULL
);



CREATE TABLE resolutions
(
    resolution_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    pending BOOLEAN DEFAULT true NOT NULL,
    adopted BOOLEAN DEFAULT false NOT NULL,
    vetoed_by BIGINT DEFAULT NULL,
    nation_id BIGINT NOT NULL,
    type TINYINT CHECK (type BETWEEN 0 AND 2) NOT NULL,
    vote_beginning TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL,
    vote_duration TIMESTAMP NOT NULL
);



CREATE TABLE votes
(
    resolution_id BIGINT,
    nation_id BIGINT,
    voted_for BOOLEAN NOT NULL,
    voting_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL,
    PRIMARY KEY(resolution_id, nation_id)
);



CREATE TABLE membership_resolutions
(
    resolution_id BIGINT PRIMARY KEY,
    invited_nation BIGINT NOT NULL
);



CREATE TABLE laws_resolutions
(
    resolution_id BIGINT PRIMARY KEY,
    title VARCHAR(50) NOT NULL,
    details VARCHAR(1000) NOT NULL
);



CREATE TABLE sanctions_resolutions
(
    resolution_id BIGINT PRIMARY KEY,
    targeted_nation BIGINT NOT NULL,
    type TINYINT CHECK (type BETWEEN 0 AND 6) NOT NULL,
    title VARCHAR(50) NOT NULL,
    details VARCHAR(1000) NOT NULL,
    amount BIGINT NOT NULL,
    duration TIMESTAMP NOT NULL
);





CREATE TABLE admin_shop
(
    listing_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    item_id VARCHAR(50) NOT NULL,
    active BOOLEAN DEFAULT true NOT NULL,
    base_price BIGINT NOT NULL,
    max_stock BIGINT NOT NULL,
    current_stock BIGINT NOT NULL,
    auto_restock BOOLEAN DEFAULT true NOT NULL,
    last_restock_time TIMESTAMP DEFAULT NULL
);



CREATE TABLE shop_prices_history
(
    listing_id BIGINT,
    base_price BIGINT,
    actual_price BIGINT NOT NULL,
    price_update_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL,
    PRIMARY KEY(listing_id, price_update_time)
);



CREATE TABLE market
(
    listing_id BIGINT PRIMARY KEY,
    item_id VARCHAR(50) NOT NULL,
    user_id BIGINT NOT NULL,
    visibility TINYINT CHECK (visibility BETWEEN 0 AND 2) NOT NULL,
    price BIGINT NOT NULL,
    quantity BIGINT NOT NULL,
    listing_creation_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL
);



CREATE TABLE items
(
    item_id VARCHAR(50) PRIMARY KEY,
    display_name VARCHAR(50) NOT NULL,
    categories VARCHAR(50) NOT NULL
);
