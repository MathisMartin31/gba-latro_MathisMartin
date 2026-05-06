/**
 * @file save.c
 */
#include "save.h"

#include "audio_utils.h"
#include "game.h"
#include "joker.h"
#include "list.h"
#include "util.h"
#include "version.h"

#include <stdlib.h>
#include <string.h>

// See https://gbadev.net/gbadoc/memory.html for more details on SRAM
// A few important pieces of info:
//   - Read/Writes are limited to 8-bits words so they are done byte per byte
//   - Memory is filled with 1s by default
//     (at least in mgba, not sure about real HW)

#define CHECK_BASE 0x0000
#define GAME_BASE  0x0010

#define CHECK_MAGIC     0x4C414247 // Spells GBAL, used to determine if the save data is junk
#define CHECK_HASH_SIZE 7
#define GIT_HASH_START  17 // starts after "GBALATRO-VERSION:" in the gbalatro_version var

#define SAVE_LABEL_SIZE 16

// clang-format off
/**
 * @brief SaveHeader for validation checks
 *
 * Structure holding save data header info to be packed and written to SRAM for validation.
 * Defined in this discussion as follows: https://github.com/GBALATRO/balatro-gba/discussions/450
 * word | Byte 0 | Byte 1 | Byte 2 | Byte 3 | name         | purpose
 * -----|--------|--------|--------|--------|--------------|------------------------------------------------------------------
 * 0    | 0x47   | 0x42   | 0x41   | 0x4C   | MAGIC        | Identify if proceeding data is valid and not junk, spells "GBAL"
 * 1    | Dirty  | H[0]   | H[1]   | H[2]   | GITHASH_LOW  | Dirty flag, followed by the first 3 bytes of shortened git hash H
 * 2    | H[3]   | H[4]   | H[5]   | H[6]   | GITHASH_HIGH | Last 4 bytes of shortened git hash H, with a dirty flag
 */
// clang-format on
typedef struct SaveHeader
{
    u32 magic;
    bool dirty;
    char githash[CHECK_HASH_SIZE];
} SaveHeader;

/**
 * @brief SaveData will contain the data to be saved to SRAM.
 *
 * GameVariables was used for this purpose at first, but some data needed to be shared but
 * not saved, so it couldn't be dumped "as is" anymore and this struct had to be created.
 *
 * Some data will be taken directly from GameVariables like the RNG seed/step and timers, some
 * will be encoded/decoded to/from minimal necessary data like the Jokers.
 *
 * Some magic numbers will also be mixed in, these are there only to make sections more
 * obvious to the eye when opening the data in an hex viewer/editor.
 * I recommend using `xxd -l 512 -e -g 4 <gbalatro.sav>`
 */
typedef struct SaveData
{
    char tag_internal[SAVE_LABEL_SIZE];

    s32 timer;
    u32 rng_seed;
    u32 rng_step;
    int round;
    int ante;
    int money;
    u32 padding0[2];

    char tag_options[SAVE_LABEL_SIZE];

    u8 game_speed;
    bool high_contrast;
    u8 music_volume;
    u8 sound_volume;
    u32 padding1[3];

    char tag_jokers[SAVE_LABEL_SIZE];

    u32 jokers_data[2 * MAX_JOKERS_HELD_SIZE];

    char tag_end[4];
} SaveData;

// clang-format off
static const SaveData SaveData_default =
{
    .tag_internal = "-INTERNAL DATA -",
    .timer = 0,
    .rng_seed = 0,
    .rng_step = 0,
    .round = 0,
    .ante = 0,
    .money = 0,
    .padding0 = {UNDEFINED, UNDEFINED},

    .tag_options = "- OPTIONS DATA -",
    .game_speed = 0,
    .high_contrast = false,
    .music_volume = 0,
    .sound_volume = 0,
    .padding1 = {UNDEFINED, UNDEFINED, UNDEFINED},

    .tag_jokers = "- OWNED JOKERS -",
    .jokers_data = {},

    .tag_end = "_END"
};
// clang-format on

/**
 * @brief Write raw binary data to SRAM
 *
 * @param sram_base address written to in the SRAM
 * @param bytes pointer to the written data
 * @param size number of bytes written
 */
static inline void write_sram(u32 sram_base, const u8* bytes, u32 size)
{
    if (sram_base + size > SRAM_SIZE)
        return;

    for (u32 i = 0; i < size; i++)
    {
        sram_mem[sram_base + i] = bytes[i];
    }
}

/**
 * @brief Read raw binary data from SRAM
 *
 * @sa write_sram
 */
static inline void read_sram(u32 sram_base, u8* bytes, u32 size)
{
    if (sram_base + size > SRAM_SIZE)
        return;

    for (u32 i = 0; i < size; i++)
    {
        bytes[i] = sram_mem[sram_base + i];
    }
}

/**
 * @brief Checks the 7 chars of gbalatro_version after the "GBALATRO_VERSION" prefix
 *         representing the git hash of the code the build is based on.
 *
 * @returns true if the git hash of the ROM is equal to the hash saved in SRAM.
 *          false if they are different.
 */
static inline bool check_hash(const char* prefix)
{
    for (u32 i = 0; i < CHECK_HASH_SIZE; i++)
    {
        if (gbalatro_version[GIT_HASH_START + i] != prefix[i])
        {
            return false;
        }
    }

    return true;
}

/**
 * @brief Determines if the current build is considered "dirty" aka has uncommitted changes.
 *         This works because the gbalatro_version string has "-dirty" added at the end if it's
 *         dirty.
 *
 * @returns true if version is dirty, false otherwise.
 */
static inline bool is_version_dirty()
{
    return strlen(gbalatro_version) > GIT_HASH_START + CHECK_HASH_SIZE;
}

/**
 * @brief Writes a magic number and ROM version info to SRAM to signal that the
 *         save data exists and allow the game to determine if it is compatible.
 */
static inline void set_save_header()
{
    SaveHeader check = {};
    check.magic = CHECK_MAGIC;
    check.dirty = is_version_dirty();
    memcpy(&(check.githash), gbalatro_version + GIT_HASH_START, CHECK_HASH_SIZE);

    write_sram(CHECK_BASE, (const u8*)&check, sizeof(check));
}

/**
 * @brief Reads whether the save data exists and is valid.
 *
 * @sa set_save_valid
 */
static inline bool check_save_header()
{
    SaveHeader check;
    read_sram(CHECK_BASE, (u8*)&check, sizeof(check));

    bool is_valid = (check.magic == CHECK_MAGIC) && check.dirty == is_version_dirty() &&
                    check_hash(check.githash);

    return is_valid;
}

void save_game(void)
{
    set_save_header();

    SaveData save_data = SaveData_default;

    // Fixed data

    save_data.timer = g_game_vars.timer;
    save_data.rng_seed = g_game_vars.rng_seed;
    save_data.rng_step = g_game_vars.rng_step;
    save_data.round = g_game_vars.round;
    save_data.ante = g_game_vars.ante;
    save_data.money = g_game_vars.money;

    save_data.game_speed = g_game_vars.game_speed;
    save_data.high_contrast = g_game_vars.high_contrast;
    save_data.music_volume = g_game_vars.music_volume;
    save_data.sound_volume = g_game_vars.sound_volume;

    // Lists

    List* jokers_list = get_jokers_list();
    u32 nb_jokers = list_get_len(jokers_list);

    int i = 0;
    for (; i < nb_jokers; i++)
    {
        JokerObject* joker_object = list_get_at_idx(jokers_list, i);
        save_data.jokers_data[2 * i] = (u32)joker_object->joker->id;
        save_data.jokers_data[2 * i + 1] = joker_object->joker->persistent_state;
    }
    for (; i < MAX_JOKERS_HELD_SIZE; i++)
    {
        save_data.jokers_data[2 * i] = UNDEFINED;
        save_data.jokers_data[2 * i + 1] = UNDEFINED;
    }

    write_sram(GAME_BASE, (const u8*)&save_data, sizeof(save_data));
}

void load_game(void)
{
    if (!check_save_header())
        return;

    SaveData save_data = SaveData_default;
    read_sram(GAME_BASE, (u8*)&save_data, sizeof(save_data));

    g_game_vars.timer = save_data.timer;
    g_game_vars.rng_seed = save_data.rng_seed;
    g_game_vars.rng_step = save_data.rng_step;
    g_game_vars.round = save_data.round;
    g_game_vars.ante = save_data.ante;
    g_game_vars.money = save_data.money;

    g_game_vars.game_speed = save_data.game_speed;
    g_game_vars.high_contrast = save_data.high_contrast;
    g_game_vars.music_volume = save_data.music_volume;
    g_game_vars.sound_volume = save_data.sound_volume;

    // TODO: load Jokers from stored minimal data

    // return to where we were in the random sequence so that the run stays reproducible
    for (u32 i = 0; i < g_game_vars.rng_step; i++)
    {
        (void)rand();
    }

    mmSetModuleVolume(MM_MODULE_FULL_VOLUME * g_game_vars.music_volume / VOLUME_OPTION_MAX);
}
