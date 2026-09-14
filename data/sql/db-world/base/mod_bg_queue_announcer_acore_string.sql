--
-- mod-bg-queue-announcer: announcer strings.
--
-- These rows were moved verbatim (translations included) out of the core's
-- acore_string 711/712/713/717/718/719/726/773-778 and 20078 into the module's
-- own 41000-41013 range, so the core can no longer collide with them.
--

DELETE FROM `acore_string` WHERE `entry` BETWEEN 41000 AND 41013;
INSERT INTO `acore_string` (`entry`, `content_default`, `locale_koKR`, `locale_frFR`, `locale_deDE`, `locale_zhCN`, `locale_zhTW`, `locale_esES`, `locale_esMX`, `locale_ruRU`) VALUES
-- 41000 (was 711): BG queue status, to the joining player
(41000,'Queue status for {} (Lvl: {} to {})\nQueued alliances: {} (Need at least {} more)\nQueued hordes: {} (Need at least {} more)',NULL,NULL,'Warteschlangenstatus für {} (Lvl: {} to {})\n Warteschlange Allianz: {} (Brauchen noch mindestens {} mehr)\n Warteschlange Horde: {} (Brauchen noch mindestens {} mehr)','|cffff0000[战场公告]:|r {} -- [{}-{}] 联盟: 目前{}人 (需要: {}人),\n\n 部落: 目前{}人 (需要: {}人)|r',NULL,NULL,NULL,NULL),
-- 41001 (was 712): BG queue status, world announcement
(41001,'|cffff0000[BG Queue Announcer]:|r {} -- [{}-{}] [{}/{}]|r',NULL,NULL,'|cffff0000[BG Ansager für Warteschlange]:|r {} -- [{}-{}] A: {}/{}, H: {}/{}|r','|cffff0000[战场公告]:|r {} -- [{}-{}] 联盟: 目前{}人 (需要: {}人),\n\n 部落: 目前{}人 (需要: {}人)|r',NULL,NULL,NULL,NULL),
-- 41002 (was 713): Arena skirmish queue status, to the joining player
(41002,'Queue status for {} (skirmish {}) (Lvl: {} to {})\nQueued: {} (Need at least {} more)',NULL,NULL,'Warteschlangenstatus für {} (Scharmützel {}) (Stufe: {} bis {})\nIn Warteschlange: {} (Mindestens {} weitere benötigt)',NULL,NULL,'Estado de cola para {} (escaramuza {}) (Nivel: {} a {})\nEn cola: {} (se necesitan al menos {} más)','Estado de cola para {} (escaramuza {}) (Nivel: {} a {})\nEn cola: {} (se necesitan al menos {} más)',NULL),
-- 41003 (was 717): Battleground started, world announcement
(41003,'|cffff0000[BG Queue Announcer]:|r {} -- [{}-{}] Started!|r',NULL,NULL,'|cffff0000[BG Ansager für Warteschlange]:|r {} -- [{}-{}] beginnt!|r','|cffff0000[战场列队公告]:|r {} -- [{}-{}] 开始！|r',NULL,NULL,NULL,NULL),
-- 41004 (was 718): Rated arena team joined queue (name + rating)
(41004,'|cffff0000[Arena Queue Announcer]:|r {} -- Joined : {}x{} : {}|r',NULL,'|cffff0000[Annonce File d\'Attente Arène]:|r {} -- Rejoint : {}x{} : {}|r','|cffff0000[BG Ansager für Warteschlange]:|r {} -- beigetreten : {}x{} : {}|r','|cffff0000[竞技场列队公告]:|r {} -- 加入 : {}x{} : {}|r',NULL,NULL,NULL,NULL),
-- 41005 (was 719): Rated arena team left queue (name + rating)
(41005,'|cffff0000[Arena Queue Announcer]:|r {} -- Exited : {}x{} : {}|r',NULL,'|cffff0000[Annonce File d\'Attente Arène]:|r {} -- Quitté : {}x{} : {}|r','|cffff0000[BG Ansager für Warteschlange]:|r {} -- verlassen : {}x{} : {}|r','|cffff0000[竞技场列队公告]:|r {} -- 退出 : {}x{} : {}|r',NULL,NULL,NULL,NULL),
-- 41006 (was 726): Arena skirmish queue status, world announcement
(41006,'|cffff0000[Arena Queue]:|r {} (skirmish {}) -- [{}-{}] [{}/{}]|r',NULL,NULL,'|cffff0000[Arena-Warteschlange]:|r {} (Scharmützel {}) -- [{}-{}] [{}/{}]|r',NULL,NULL,'|cffff0000[Cola de arena]:|r {} (escaramuza {}) -- [{}-{}] [{}/{}]|r','|cffff0000[Cola de arena]:|r {} (escaramuza {}) -- [{}-{}] [{}/{}]|r',NULL),
-- 41007 (was 773): Rated arena team joined queue (name)
(41007,'|cffff0000[Arena Queue Announcer]:|r {} -- Joined : {}x{}|r',NULL,'|cffff0000[Annonce File d\'Attente Arène]:|r {} -- Rejoint : {}x{}|r','|cffff0000[BG Ansager für Warteschlange]:|r {} -- beigetreten : {}x{}|r','|cffff0000[竞技场列队公告]:|r {} -- 加入 : {}x{}|r',NULL,NULL,NULL,NULL),
-- 41008 (was 774): Rated arena team left queue (name)
(41008,'|cffff0000[Arena Queue Announcer]:|r {} -- Exited : {}x{}|r',NULL,'|cffff0000[Annonce File d\'Attente Arène]:|r {} -- Quitté : {}x{}|r','|cffff0000[BG Ansager für Warteschlange]:|r {} -- verlassen : {}x{}|r','|cffff0000[竞技场列队公告]:|r {} -- 退出 : {}x{}|r',NULL,NULL,NULL,NULL),
-- 41009 (was 775): Rated arena team joined queue (rating)
(41009,'|cffff0000[Arena Queue Announcer]:|r Joined : {}x{} : {}|r',NULL,'|cffff0000[Annonce File d\'Attente Arène]:|r Rejoint : {}x{} : {}|r','|cffff0000[BG Ansager für Warteschlange]:|r beigetreten : {}x{} : {}|r','|cffff0000[竞技场列队公告]:|r 加入 : {}x{} : {}|r',NULL,NULL,NULL,NULL),
-- 41010 (was 776): Rated arena team left queue (rating)
(41010,'|cffff0000[Arena Queue Announcer]:|r Exited : {}x{} : {}|r',NULL,'|cffff0000[Annonce File d\'Attente Arène]:|r Quitté : {}x{} : {}|r','|cffff0000[BG Ansager für Warteschlange]:|r verlassen : {}x{} : {}|r','|cffff0000[竞技场列队公告]:|r 退出 : {}x{} : {}|r',NULL,NULL,NULL,NULL),
-- 41011 (was 777): Rated arena team joined queue
(41011,'|cffff0000[Arena Queue Announcer]:|r Joined : {}x{}|r',NULL,'|cffff0000[Annonce File d\'Attente Arène]:|r Rejoint : {}x{}|r','|cffff0000[BG Ansager für Warteschlange]:|r beigetreten : {}x{}|r','|cffff0000[竞技场列队公告]:|r 加入 : {}x{}|r',NULL,NULL,NULL,NULL),
-- 41012 (was 778): Rated arena team left queue
(41012,'|cffff0000[Arena Queue Announcer]:|r Exited : {}x{}|r',NULL,'|cffff0000[Annonce File d\'Attente Arène]:|r Quitté : {}x{}|r','|cffff0000[BG Ansager für Warteschlange]:|r verlassen : {}x{}|r','|cffff0000[竞技场列队公告]:|r 退出 : {}x{}|r',NULL,NULL,NULL,NULL),
-- 41013 (was 20078): Wintergrasp battle started, world announcement
(41013,'|cffff0000[Wintergrasp]:|r Battle started!|r',NULL,NULL,'|cffff0000[Wintersturm]:|r Kampf hat begonnen!|r','|cffff0000[冬拥湖]:|r 战斗开始了！|r',NULL,NULL,NULL,NULL);
