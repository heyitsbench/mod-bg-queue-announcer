--
-- mod-bg-queue-announcer: the PvP halves of `.settings announcer`.
--
-- The core keeps `settings announcer autobroadcast`; these four subcommands are
-- registered by this module and merge into the same command node.
--

DELETE FROM `command` WHERE `name` IN ('settings announcer bg', 'settings announcer arena', 'settings announcer pvpstart', 'settings announcer pvpall');
INSERT INTO `command` (`name`, `security`, `help`) VALUES
('settings announcer bg',       1, 'Syntax: .settings announcer bg <on/off>.\nEnables or disables receiving battleground queue announcements.'),
('settings announcer arena',    1, 'Syntax: .settings announcer arena <on/off>.\nEnables or disables receiving arena queue announcements.'),
('settings announcer pvpstart', 1, 'Syntax: .settings announcer pvpstart <on/off>.\nEnables or disables receiving battleground start announcements.'),
('settings announcer pvpall',   1, 'Syntax: .settings announcer pvpall <on/off>.\nEnables or disables receiving every PvP announcement at once.');
