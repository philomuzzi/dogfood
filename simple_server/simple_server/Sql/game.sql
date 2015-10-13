DROP TABLE IF EXISTS `player`;
CREATE TABLE IF NOT EXISTS `player` (
		`accid` varchar(48) NOT NULL default '0' UNIQUE,
		`charid` INTEGER PRIMARY KEY AUTOINCREMENT,
		`name` varchar(48) NOT NULL default '',
		`account` varchar(48) NOT NULL default '',
		`databinary` blob NOT NULL
	);

DROP TABLE IF EXISTS `systemmail`;
CREATE TABLE IF NOT EXISTS `systemmail` (
		`id` UNSIGNED BIG INT PRIMARY KEY NOT NULL,
		`time` INTEGER NOT NULL,
        `status` int(10)  NOT NULL,
		`databinary` blob NOT NULL
	);

DROP TABLE IF EXISTS `usermail`;
CREATE TABLE IF NOT EXISTS `usermail` (
		`id` UNSIGNED BIG INT PRIMARY KEY NOT NULL,
		`time` INTEGER NOT NULL,
		`send_accid` varchar(48) NOT NULL default '0',
		`recv_accid` varchar(48) NOT NULL default '0',
		`databinary` blob NOT NULL
	);
