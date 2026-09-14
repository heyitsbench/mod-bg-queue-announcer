#ifndef MOD_BG_QUEUE_ANNOUNCER_H_
#define MOD_BG_QUEUE_ANNOUNCER_H_

#include "Define.h"

// acore_string entries shipped by this module, see
// data/sql/db-world/base/mod_bg_queue_announcer_acore_string.sql.
// They were moved out of the core's own range; the comments name the entry
// each one replaced.
enum BGQueueAnnouncerString : uint32
{
    ANNOUNCER_STRING_BG_QUEUE_SELF             = 41000, // was 711
    ANNOUNCER_STRING_BG_QUEUE_WORLD            = 41001, // was 712
    ANNOUNCER_STRING_ARENA_QUEUE_SELF          = 41002, // was 713
    ANNOUNCER_STRING_BG_STARTED_WORLD          = 41003, // was 717
    ANNOUNCER_STRING_ARENA_JOIN_NAME_RATING    = 41004, // was 718
    ANNOUNCER_STRING_ARENA_EXIT_NAME_RATING    = 41005, // was 719
    ANNOUNCER_STRING_ARENA_QUEUE_WORLD         = 41006, // was 726
    ANNOUNCER_STRING_ARENA_JOIN_NAME           = 41007, // was 773
    ANNOUNCER_STRING_ARENA_EXIT_NAME           = 41008, // was 774
    ANNOUNCER_STRING_ARENA_JOIN_RATING         = 41009, // was 775
    ANNOUNCER_STRING_ARENA_EXIT_RATING         = 41010, // was 776
    ANNOUNCER_STRING_ARENA_JOIN                = 41011, // was 777
    ANNOUNCER_STRING_ARENA_EXIT                = 41012, // was 778
    ANNOUNCER_STRING_WINTERGRASP_STARTED_WORLD = 41013  // was 20078
};

// Per-player opt-out bits, toggled by `.settings announcer`. They are stored in
// the core's SETTING_ANNOUNCER_FLAGS index next to the core-owned
// ANNOUNCER_FLAG_DISABLE_AUTOBROADCAST (4), and deliberately keep the values the
// core used, so existing character_settings rows keep working unchanged.
enum PvPAnnouncerFlags : uint8
{
    PVP_ANNOUNCER_FLAG_DISABLE_BG_QUEUE    = 1,
    PVP_ANNOUNCER_FLAG_DISABLE_ARENA_QUEUE = 2,
//  ANNOUNCER_FLAG_DISABLE_AUTOBROADCAST   = 4,
    PVP_ANNOUNCER_FLAG_DISABLE_PVP_START   = 8,
    PVP_ANNOUNCER_FLAG_DISABLE_PVP_ALL     = PVP_ANNOUNCER_FLAG_DISABLE_BG_QUEUE
                                           | PVP_ANNOUNCER_FLAG_DISABLE_ARENA_QUEUE
                                           | PVP_ANNOUNCER_FLAG_DISABLE_PVP_START
};

#endif // MOD_BG_QUEUE_ANNOUNCER_H_
