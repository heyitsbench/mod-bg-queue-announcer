# mod-bg-queue-announcer

Every PvP **announcement** AzerothCore used to make from the core, extracted into a module.

## What it does

With this module installed and `Battleground.QueueAnnouncer.Enable = 1`, the server announces:

- **Battleground queue joins**, in either immediate (debounced, spam-protected) or timed mode, to the
  world or to the joining player only.
- **Arena skirmish queue joins**, to the world or to the joining player only.
- **Rated arena team queue joins and exits**, with configurable detail.
- **Battleground starts** ("... Started!"), as a world announcement.
- **Wintergrasp battle starts**, as a world announcement.

It also provides the player-facing opt-outs for all of the above: `.settings announcer bg`,
`arena`, `pvpstart` and `pvpall`.

Without this module, the core announces none of it. The zone-scoped Wintergrasp start warning (the
blizzlike one, sent through `creature_text` on the battlefield stalker) is core behaviour and is
unaffected.

## How it works

Four `AllBattlegroundScript` / `BattlefieldScript` hooks carry the announcements out of the core.
Three are new; three older veto-style hooks were removed in exchange, so the core's hook count is
unchanged.

| Hook | Fired from | Purpose |
| ---- | ---------- | ------- |
| `OnBattlegroundQueueGroupJoined` | `BattlegroundQueue::AddGroup` | A group joined a BG or arena queue |
| `OnBattlegroundQueuePlayerRemoved` | `BattlegroundQueue::RemovePlayer` | A rated team left the arena queue |
| `OnBattlegroundQueueBracketUpdate` | `BattlegroundMgr::Update` | Periodic pass that emits timed/deferred lines |
| `OnBattlegroundStart` | `Battleground::_ProcessJoin` | Pre-existing hook; a battleground went live |
| `OnBattlefieldWarStart` | `Battlefield::StartBattle` | New; a battlefield battle went live |

Removed from the core in exchange: `CanSendMessageBGQueue`, `OnBeforeSendJoinMessageArenaQueue` and
`OnBeforeSendExitMessageArenaQueue`, which existed only to gate the core announcer.

`OnBattlegroundQueueGroupJoined` fires at the exact point `SendJoinMessageArenaQueue` used to be
called — before the battleground template is resolved — and the module then performs the template
lookup and the `!isRated && !isPremade && Enable` check the core used to do before
`SendMessageBGQueue`.

The announcement timer state formerly stored on each `BattlegroundQueue`
(`_queueAnnouncementTimer` / `_queueAnnouncementCrossfactioned`) is now held inside the module, keyed
by queue. The core's `BGSpamProtect` (`sBGSpam`) singleton, which nothing else used, moved in here as
well.

## Configuration

Every option moved out of `worldserver.conf` into `mod-bg-queue-announcer.conf`, keeping its name.
To migrate, copy across any value you had customised:

| Option | Effect |
| ------ | ------ |
| `Battleground.QueueAnnouncer.Enable` | Master switch for all announcements below |
| `Battleground.QueueAnnouncer.PlayerOnly` | BG queue status to the joining player instead of the world |
| `Battleground.QueueAnnouncer.Timed` / `.Timer` | Timed instead of immediate BG queue announcements |
| `Battleground.QueueAnnouncer.SpamProtection.Delay` | Per-player / per-bracket throttle, in seconds |
| `Battleground.QueueAnnouncer.Limit.MinLevel` / `.Limit.MinPlayers` | Suppress low-population announcements |
| `Arena.QueueAnnouncer.Enable` | Additionally required for any arena announcement |
| `Arena.QueueAnnouncer.PlayerOnly` | Arena queue status to the joining player instead of the world |
| `Arena.QueueAnnouncer.Detail` | How much rated-team detail to include |

## Database

The module ships its own strings, so nothing of the announcer is left in the core's DB:

- `acore_string` **41000-41013** — the fourteen announcement strings, moved verbatim (translations
  included) from the core's 711, 712, 713, 717, 718, 719, 726, 773-778 and 20078. The core deletes
  those rows in a `pending_db_world` revision.
- `command` — the `settings announcer bg|arena|pvpstart|pvpall` help rows. The core keeps only
  `settings announcer autobroadcast`.

## The `.settings announcer` command

`.settings announcer` became a subcommand node so that it can be split. The core registers
`autobroadcast`; this module registers `bg`, `arena`, `pvpstart` and `pvpall`. The syntax players
type is unchanged (`.settings announcer bg on`), and the opt-out bits keep the values the core used
(1, 2 and 8, alongside the core-owned autobroadcast bit 4) in the same `character_settings` index, so
existing player preferences carry over untouched.

## Installation

```
cd azerothcore-wotlk/modules
# place mod-bg-queue-announcer here
cd ../build && cmake .. && make -j$(nproc)
```

Then copy `mod-bg-queue-announcer.conf.dist` to `mod-bg-queue-announcer.conf` in your config
directory, adjust it, and apply the module's SQL.
