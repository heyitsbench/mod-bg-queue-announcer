#include "BGQueueAnnouncer.h"
#include "Chat.h"
#include "CommandScript.h"
#include "Language.h"
#include "Player.h"
#include "PlayerSettings.h"
#include "RBAC.h"

using namespace Acore::ChatCommands;

// The PvP halves of `.settings announcer`. The core still registers
// `settings announcer autobroadcast`; the command loader merges both tables into
// the same node, so the player-facing syntax is unchanged.
class bg_queue_announcer_commandscript : public CommandScript
{
public:
    bg_queue_announcer_commandscript() : CommandScript("bg_queue_announcer_commandscript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable announcerCommandTable =
        {
            { "bg",       HandleSettingsAnnouncerBGQueue,    rbac::RBAC_PERM_COMMAND_SETTINGS_ANNOUNCER, Console::No },
            { "arena",    HandleSettingsAnnouncerArenaQueue, rbac::RBAC_PERM_COMMAND_SETTINGS_ANNOUNCER, Console::No },
            { "pvpstart", HandleSettingsAnnouncerPvPStart,   rbac::RBAC_PERM_COMMAND_SETTINGS_ANNOUNCER, Console::No },
            { "pvpall",   HandleSettingsAnnouncerPvPAll,     rbac::RBAC_PERM_COMMAND_SETTINGS_ANNOUNCER, Console::No },
        };
        static ChatCommandTable playerSettingsCommandTable =
        {
            { "announcer", announcerCommandTable },
        };
        static ChatCommandTable commandTable =
        {
            { "settings", playerSettingsCommandTable },
        };
        return commandTable;
    }

private:
    static bool ToggleAnnouncerFlag(ChatHandler* handler, bool on, uint32 flag, char const* label)
    {
        Player* player = handler->GetPlayer();

        PlayerSetting setting = player->GetPlayerSetting(AzerothcorePSSource, SETTING_ANNOUNCER_FLAGS);

        on ? setting.RemoveFlag(flag) : setting.AddFlag(flag);
        player->UpdatePlayerSetting(AzerothcorePSSource, SETTING_ANNOUNCER_FLAGS, setting.value);

        handler->SetSentErrorMessage(false);
        handler->PSendSysMessage(on ? LANG_CMD_SETTINGS_ANNOUNCER_ON : LANG_CMD_SETTINGS_ANNOUNCER_OFF, label);
        return true;
    }

    static bool HandleSettingsAnnouncerBGQueue(ChatHandler* handler, bool on)
    {
        return ToggleAnnouncerFlag(handler, on, PVP_ANNOUNCER_FLAG_DISABLE_BG_QUEUE, "battleground queue");
    }

    static bool HandleSettingsAnnouncerArenaQueue(ChatHandler* handler, bool on)
    {
        return ToggleAnnouncerFlag(handler, on, PVP_ANNOUNCER_FLAG_DISABLE_ARENA_QUEUE, "arena queue");
    }

    static bool HandleSettingsAnnouncerPvPStart(ChatHandler* handler, bool on)
    {
        return ToggleAnnouncerFlag(handler, on, PVP_ANNOUNCER_FLAG_DISABLE_PVP_START, "PvP start");
    }

    static bool HandleSettingsAnnouncerPvPAll(ChatHandler* handler, bool on)
    {
        return ToggleAnnouncerFlag(handler, on, PVP_ANNOUNCER_FLAG_DISABLE_PVP_ALL, "PvP");
    }
};

void AddSC_bg_queue_announcer_commandscript()
{
    new bg_queue_announcer_commandscript();
}
