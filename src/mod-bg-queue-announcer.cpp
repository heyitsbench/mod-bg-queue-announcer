#include "ArenaTeam.h"
#include "ArenaTeamMgr.h"
#include "BGQueueAnnouncer.h"
#include "Battlefield.h"
#include "Battleground.h"
#include "BattlegroundMgr.h"
#include "BattlegroundQueue.h"
#include "BattlegroundUtils.h"
#include "Chat.h"
#include "Config.h"
#include "DBCStores.h"
#include "GameTime.h"
#include "Log.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "StringFormat.h"
#include <algorithm>
#include <array>
#include <unordered_map>

namespace
{
    // Immediate-mode announcer debounce (ms): fires on the next periodic pass, by
    // which point a same-tick queue burst has collapsed into one aggregated line.
    constexpr int32 BG_QUEUE_ANNOUNCER_IMMEDIATE_DEBOUNCE = 1;

    // Module configuration, cached on config (re)load the same way the core used to
    // cache these keys in WorldConfig.
    struct AnnouncerConfig
    {
        bool     Enable             = false;
        bool     BGPlayerOnly       = false;
        bool     BGTimed            = false;
        uint32   BGTimer            = 30000;
        uint32   SpamDelay          = 30;
        uint32   LimitMinLevel      = 0;
        uint32   LimitMinPlayers    = 3;
        bool     ArenaEnable        = false;
        bool     ArenaPlayerOnly    = false;
        uint32   ArenaDetail        = 3;
    };

    AnnouncerConfig _config;

    // Spam protection - throttles world announcements per player and per bracket.
    // Formerly BGSpamProtect (sBGSpam) in the core.
    std::unordered_map<ObjectGuid /*player guid*/, uint32 /*time*/> _players;
    std::unordered_map<uint32 /*bracket+bg key*/, uint32 /*time*/> _brackets;

    void AddTime(ObjectGuid guid)
    {
        _players.insert_or_assign(guid, GameTime::GetGameTime().count());
    }

    uint32 GetTime(ObjectGuid guid)
    {
        auto const& itr = _players.find(guid);
        if (itr != _players.end())
        {
            return itr->second;
        }

        return 0;
    }

    bool IsCorrectDelay(ObjectGuid guid)
    {
        // Skip if spam time < 30 secs (default)
        return GameTime::GetGameTime().count() - GetTime(guid) >= _config.SpamDelay;
    }

    void AddTime(uint32 key)
    {
        _brackets.insert_or_assign(key, GameTime::GetGameTime().count());
    }

    uint32 GetTime(uint32 key)
    {
        auto const& itr = _brackets.find(key);
        if (itr != _brackets.end())
            return itr->second;

        return 0;
    }

    bool IsCorrectDelay(uint32 key)
    {
        // Skip if spam time < 30 secs (default)
        return GameTime::GetGameTime().count() - GetTime(key) >= _config.SpamDelay;
    }

    bool CanAnnounce(Player* player, Battleground* bg, uint32 minLevel, uint32 queueTotal)
    {
        ObjectGuid guid = player->GetGUID();

        // Check prev time
        if (!IsCorrectDelay(guid))
        {
            return false;
        }

        if (bg)
        {
            // When limited, it announces only if there are at least LimitMinPlayers in queue
            if (_config.LimitMinLevel && minLevel >= _config.LimitMinLevel)
            {
                // limit only RBG for 80, WSG for lower levels
                auto bgTypeToLimit = minLevel == 80 ? BATTLEGROUND_RB : BATTLEGROUND_WS;

                if (bg->GetBgTypeID() == bgTypeToLimit && queueTotal < _config.LimitMinPlayers)
                {
                    return false;
                }
            }
        }

        AddTime(guid);
        return true;
    }

    bool CanAnnounce(Battleground* bg, BattlegroundBracketId bracketId, uint32 minLevel, uint32 queueTotal)
    {
        if (!bg)
            return false;

        uint32 key = uint32(bg->GetBgTypeID()) * MAX_BATTLEGROUND_BRACKETS + uint32(bracketId);

        // Check prev time
        if (!IsCorrectDelay(key))
            return false;

        // When limited, it announces only if there are at least LimitMinPlayers in queue
        if (_config.LimitMinLevel && minLevel >= _config.LimitMinLevel)
        {
            // limit only RBG for 80, WSG for lower levels
            auto bgTypeToLimit = minLevel == 80 ? BATTLEGROUND_RB : BATTLEGROUND_WS;

            if (bg->GetBgTypeID() == bgTypeToLimit && queueTotal < _config.LimitMinPlayers)
                return false;
        }

        AddTime(key);
        return true;
    }
}

class BGQueueAnnouncerConfig : public WorldScript
{
public:
    BGQueueAnnouncerConfig() : WorldScript("BGQueueAnnouncerConfig", { WORLDHOOK_ON_AFTER_CONFIG_LOAD }) { }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        _config.Enable          = sConfigMgr->GetOption<bool>("Battleground.QueueAnnouncer.Enable", false);
        _config.BGPlayerOnly    = sConfigMgr->GetOption<bool>("Battleground.QueueAnnouncer.PlayerOnly", false);
        _config.BGTimed         = sConfigMgr->GetOption<bool>("Battleground.QueueAnnouncer.Timed", false);
        _config.BGTimer         = sConfigMgr->GetOption<uint32>("Battleground.QueueAnnouncer.Timer", 30000);
        _config.SpamDelay       = sConfigMgr->GetOption<uint32>("Battleground.QueueAnnouncer.SpamProtection.Delay", 30);
        _config.LimitMinLevel   = sConfigMgr->GetOption<uint32>("Battleground.QueueAnnouncer.Limit.MinLevel", 0);
        _config.LimitMinPlayers = sConfigMgr->GetOption<uint32>("Battleground.QueueAnnouncer.Limit.MinPlayers", 3);
        _config.ArenaEnable     = sConfigMgr->GetOption<bool>("Arena.QueueAnnouncer.Enable", false);
        _config.ArenaPlayerOnly = sConfigMgr->GetOption<bool>("Arena.QueueAnnouncer.PlayerOnly", false);
        _config.ArenaDetail     = sConfigMgr->GetOption<uint32>("Arena.QueueAnnouncer.Detail", 3);
    }
};

class BGQueueAnnouncer : public AllBattlegroundScript
{
public:
    BGQueueAnnouncer() : AllBattlegroundScript("BGQueueAnnouncer",
        {
            ALLBATTLEGROUNDHOOK_ON_BATTLEGROUND_START,
            ALLBATTLEGROUNDHOOK_ON_QUEUE_GROUP_JOINED,
            ALLBATTLEGROUNDHOOK_ON_QUEUE_PLAYER_REMOVED,
            ALLBATTLEGROUNDHOOK_ON_QUEUE_BRACKET_UPDATE
        }) { }

    // Formerly announced inline by Battleground::_ProcessJoin, immediately before
    // this same hook fired.
    void OnBattlegroundStart(Battleground* bg) override
    {
        if (!_config.Enable)
            return;

        ChatHandler(nullptr).SendWorldTextOptional(ANNOUNCER_STRING_BG_STARTED_WORLD, PVP_ANNOUNCER_FLAG_DISABLE_PVP_START,
            bg->GetName(), std::min(bg->GetMinLevel(), (uint32)80), std::min(bg->GetMaxLevel(), (uint32)80));
    }

    void OnBattlegroundQueueGroupJoined(BattlegroundQueue* queue, Player* leader, GroupQueueInfo* ginfo, PvPDifficultyEntry const* bracketEntry, bool isRated, bool isPremade) override
    {
        // Arena announcements first, exactly where BattlegroundQueue::AddGroup used to call
        // SendJoinMessageArenaQueue - before the battleground template is resolved.
        SendJoinMessageArenaQueue(queue, leader, ginfo, bracketEntry, isRated);

        Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(ginfo->BgTypeId);
        if (!bg)
            return;

        if (!isRated && !isPremade && _config.Enable)
            SendMessageBGQueue(queue, leader, bg, bracketEntry);
    }

    void OnBattlegroundQueuePlayerRemoved(BattlegroundQueue* /*queue*/, GroupQueueInfo* ginfo) override
    {
        if (!_config.ArenaEnable)
            return;

        ArenaTeam* team = sArenaTeamMgr->GetArenaTeamById(ginfo->ArenaTeamId);
        if (!team)
            return;

        if (!ginfo->IsRated)
            return;

        uint8 ArenaType = ginfo->ArenaType;
        uint32 ArenaTeamRating = ginfo->ArenaTeamRating;
        std::string TeamName = team->GetName();

        if (ArenaType && ginfo->Players.empty())
        {
            switch (_config.ArenaDetail)
            {
            case 3:
                ChatHandler(nullptr).SendWorldTextOptional(ANNOUNCER_STRING_ARENA_EXIT_NAME_RATING, PVP_ANNOUNCER_FLAG_DISABLE_ARENA_QUEUE, TeamName.c_str(), ArenaType, ArenaType, ArenaTeamRating);
                break;
            case 2:
                ChatHandler(nullptr).SendWorldTextOptional(ANNOUNCER_STRING_ARENA_EXIT_NAME, PVP_ANNOUNCER_FLAG_DISABLE_ARENA_QUEUE, TeamName, ArenaType, ArenaType);
                break;
            case 1:
                ChatHandler(nullptr).SendWorldTextOptional(ANNOUNCER_STRING_ARENA_EXIT_RATING, PVP_ANNOUNCER_FLAG_DISABLE_ARENA_QUEUE, ArenaType, ArenaType, ArenaTeamRating);
                break;
            default:
                ChatHandler(nullptr).SendWorldTextOptional(ANNOUNCER_STRING_ARENA_EXIT, PVP_ANNOUNCER_FLAG_DISABLE_ARENA_QUEUE, ArenaType, ArenaType);
                break;
            }
        }
    }

    void OnBattlegroundQueueBracketUpdate(BattlegroundQueue* queue, uint32 diff, BattlegroundQueueTypeId bgQueueTypeId, BattlegroundBracketId bracket_id) override
    {
        BattlegroundTypeId bgTypeId = BattlegroundMgr::BGTemplateId(bgQueueTypeId);
        Battleground* bg_template = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
        if (!bg_template)
        {
            return;
        }

        PvPDifficultyEntry const* bracketEntry = GetBattlegroundBracketById(bg_template->GetMapId(), bracket_id);
        if (!bracketEntry)
        {
            return;
        }

        AnnounceState& state = GetState(queue);

        // Armed per-bracket timer drives both Timed mode and the deferred immediate
        // announcement; the spam-window/Limit throttle gates only immediate mode.
        bool const isTimed = _config.BGTimed;

        uint32 qPlayers = 0;

        if (state.crossfactioned)
            qPlayers = queue->GetPlayersCountInGroupsQueue(bracket_id, BG_QUEUE_CFBG);
        else
            qPlayers = queue->GetPlayersCountInGroupsQueue(bracket_id, BG_QUEUE_NORMAL_HORDE) + queue->GetPlayersCountInGroupsQueue(bracket_id, BG_QUEUE_NORMAL_ALLIANCE);

        if (!qPlayers)
        {
            state.timer[bracket_id] = -1;
            return;
        }

        if (state.timer[bracket_id] >= 0)
        {
            if (state.timer[bracket_id] <= static_cast<int32>(diff))
            {
                state.timer[bracket_id] = -1;

                uint32 q_min_level = std::min(bracketEntry->minLevel, (uint32) 80);

                if (!isTimed && !CanAnnounce(bg_template, bracket_id, q_min_level, qPlayers))
                    return;

                auto bgName = bg_template->GetName();
                uint32 MaxPlayers = GetMinPlayersPerTeam(bg_template, bracketEntry) * 2;
                uint32 q_max_level = std::min(bracketEntry->maxLevel, (uint32) 80);

                ChatHandler(nullptr).SendWorldTextOptional(ANNOUNCER_STRING_BG_QUEUE_WORLD, PVP_ANNOUNCER_FLAG_DISABLE_BG_QUEUE, bgName.c_str(), q_min_level, q_max_level, qPlayers, MaxPlayers);
            }
            else
            {
                state.timer[bracket_id] -= static_cast<int32>(diff);
            }
        }
    }

private:
    // Per-queue announcement state, formerly held on BattlegroundQueue in the core.
    struct AnnounceState
    {
        AnnounceState() { timer.fill(-1); }

        std::array<int32, MAX_BATTLEGROUND_BRACKETS> timer;
        bool crossfactioned = false;
    };

    // Queue objects are members of BattlegroundMgr's fixed queue array and live for the
    // server lifetime, so keying by pointer is safe.
    std::unordered_map<BattlegroundQueue*, AnnounceState> _states;

    AnnounceState& GetState(BattlegroundQueue* queue)
    {
        return _states[queue];
    }

    static void SetQueueAnnouncementTimer(AnnounceState& state, uint32 bracketId, int32 timer, bool isCrossFactionBG)
    {
        state.timer[bracketId] = timer;
        state.crossfactioned = isCrossFactionBG;
    }

    // Formerly BattlegroundQueue::SendMessageBGQueue.
    void SendMessageBGQueue(BattlegroundQueue* queue, Player* leader, Battleground* bg, PvPDifficultyEntry const* bracketEntry)
    {
        if (bg->isArena())
        {
            // Skip announce for arena skirmish
            return;
        }

        BattlegroundBracketId bracketId = bracketEntry->GetBracketId();
        auto bgName = bg->GetName();
        uint32 MinPlayers = GetMinPlayersPerTeam(bg, bracketEntry);
        uint32 MaxPlayers = MinPlayers * 2;
        uint32 q_min_level = std::min(bracketEntry->minLevel, (uint32)80);
        uint32 q_max_level = std::min(bracketEntry->maxLevel, (uint32)80);
        uint32 qHorde = queue->GetPlayersCountInGroupsQueue(bracketId, BG_QUEUE_NORMAL_HORDE);
        uint32 qAlliance = queue->GetPlayersCountInGroupsQueue(bracketId, BG_QUEUE_NORMAL_ALLIANCE);
        auto qTotal = qHorde + qAlliance;

        LOG_DEBUG("bg.battleground", "> Queue status for {} (Lvl: {} to {}) Queued: {} (Need at least {} more)",
            bgName, q_min_level, q_max_level, qAlliance + qHorde, MaxPlayers - qTotal);

        // Show queue status to player only (when joining battleground queue or Arena and arena world announcer is disabled)
        if (_config.BGPlayerOnly)
        {
            ChatHandler(leader->GetSession()).PSendSysMessage(ANNOUNCER_STRING_BG_QUEUE_SELF, bgName, q_min_level, q_max_level,
                qAlliance, (MinPlayers > qAlliance) ? MinPlayers - qAlliance : (uint32)0,
                qHorde, (MinPlayers > qHorde) ? MinPlayers - qHorde : (uint32)0);
        }
        else // Show queue status to server (when joining battleground queue)
        {
            AnnounceState& state = GetState(queue);

            if (_config.BGTimed)
            {
                if (state.timer[bracketId] < 0)
                {
                    state.timer[bracketId] = static_cast<int32>(_config.BGTimer);
                }
            }
            else
            {
                // Arm the per-bracket debounce; first join arms, rest are no-ops.
                // OnBattlegroundQueueBracketUpdate emits the aggregated line later.
                if (state.timer[bracketId] < 0)
                    SetQueueAnnouncementTimer(state, bracketId, BG_QUEUE_ANNOUNCER_IMMEDIATE_DEBOUNCE, false);
            }
        }
    }

    // Formerly BattlegroundQueue::SendJoinMessageArenaQueue.
    void SendJoinMessageArenaQueue(BattlegroundQueue* queue, Player* leader, GroupQueueInfo* ginfo, PvPDifficultyEntry const* bracketEntry, bool isRated)
    {
        if (!_config.ArenaEnable)
            return;

        if (!isRated)
        {
            Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(ginfo->BgTypeId);
            if (!bg)
            {
                LOG_ERROR("bg.arena", "> Not found bg template for bgtype id {}", uint32(ginfo->BgTypeId));
                return;
            }

            if (!bg->isArena())
            {
                // Skip announce for non arena
                return;
            }

            BattlegroundBracketId bracketId = bracketEntry->GetBracketId();
            auto bgName = bg->GetName();
            auto arenatype = Acore::StringFormat("{}v{}", ginfo->ArenaType, ginfo->ArenaType);
            uint32 playersNeed = ArenaTeam::GetReqPlayersForType(ginfo->ArenaType);
            uint32 q_min_level = std::min(bracketEntry->minLevel, (uint32)80);
            uint32 q_max_level = std::min(bracketEntry->maxLevel, (uint32)80);
            uint32 qPlayers = queue->GetPlayersCountInGroupsQueue(bracketId, BG_QUEUE_NORMAL_HORDE) + queue->GetPlayersCountInGroupsQueue(bracketId, BG_QUEUE_NORMAL_ALLIANCE);

            LOG_DEBUG("bg.arena", "> Queue status for {} (skirmish {}) (Lvl: {} to {}) Queued: {} (Need at least {} more)",
                bgName, arenatype, q_min_level, q_max_level, qPlayers, playersNeed - qPlayers);

            if (_config.ArenaPlayerOnly)
            {
                ChatHandler(leader->GetSession()).PSendSysMessage(ANNOUNCER_STRING_ARENA_QUEUE_SELF,
                    bgName, arenatype, q_min_level, q_max_level, qPlayers, playersNeed - qPlayers);
            }
            else
            {
                if (!CanAnnounce(leader, bg, q_min_level, qPlayers))
                {
                    return;
                }

                ChatHandler(nullptr).SendWorldTextOptional(ANNOUNCER_STRING_ARENA_QUEUE_WORLD, PVP_ANNOUNCER_FLAG_DISABLE_ARENA_QUEUE, bgName.c_str(), arenatype.c_str(), q_min_level, q_max_level, qPlayers, playersNeed);
            }
        }
        else
        {
            ArenaTeam* team = sArenaTeamMgr->GetArenaTeamById(ginfo->ArenaTeamId);
            if (!team || !ginfo->IsRated)
            {
                return;
            }

            uint8 ArenaType = ginfo->ArenaType;
            uint32 ArenaTeamRating = ginfo->ArenaTeamRating;
            std::string TeamName = team->GetName();

            switch (_config.ArenaDetail)
            {
            case 3:
                ChatHandler(nullptr).SendWorldTextOptional(ANNOUNCER_STRING_ARENA_JOIN_NAME_RATING, PVP_ANNOUNCER_FLAG_DISABLE_ARENA_QUEUE, TeamName.c_str(), ArenaType, ArenaType, ArenaTeamRating);
                break;
            case 2:
                ChatHandler(nullptr).SendWorldTextOptional(ANNOUNCER_STRING_ARENA_JOIN_NAME, PVP_ANNOUNCER_FLAG_DISABLE_ARENA_QUEUE, TeamName, ArenaType, ArenaType);
                break;
            case 1:
                ChatHandler(nullptr).SendWorldTextOptional(ANNOUNCER_STRING_ARENA_JOIN_RATING, PVP_ANNOUNCER_FLAG_DISABLE_ARENA_QUEUE, ArenaType, ArenaType, ArenaTeamRating);
                break;
            default:
                ChatHandler(nullptr).SendWorldTextOptional(ANNOUNCER_STRING_ARENA_JOIN, PVP_ANNOUNCER_FLAG_DISABLE_ARENA_QUEUE, ArenaType, ArenaType);
                break;
            }
        }
    }
};

// Formerly announced inline at the end of BattlefieldWG::OnBattleStart.
class BGQueueAnnouncerBattlefield : public BattlefieldScript
{
public:
    BGQueueAnnouncerBattlefield() : BattlefieldScript("BGQueueAnnouncerBattlefield", { BATTLEFIELDHOOK_ON_WAR_START }) { }

    void OnBattlefieldWarStart(Battlefield* bf) override
    {
        if (!_config.Enable)
            return;

        // Wintergrasp is the only battlefield the core ever announced.
        if (bf->GetTypeId() != BATTLEFIELD_WG)
            return;

        ChatHandler(nullptr).SendWorldText(ANNOUNCER_STRING_WINTERGRASP_STARTED_WORLD);
    }
};

// Add all scripts in one
void AddBGQueueAnnouncerScripts()
{
    new BGQueueAnnouncerConfig();
    new BGQueueAnnouncer();
    new BGQueueAnnouncerBattlefield();
}
