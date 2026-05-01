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
#define LISTS_BASE 0x0060

#define CHECK_MAGIC     0x4C414247 // Spells GBAL, used to determine if the save data is junk
#define CHECK_HASH_SIZE 7
#define GIT_HASH_START  17 // starts after "GBALATRO-VERSION:" in the balatro_version var

// clang-format off
/**
 * @brief Structurer holding save data header info to be packed and written to SRAM for validation.
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
    char githash[7];
} SaveHeader;

typedef struct JokerSaveData
{
    u32 id; // Inefficient here, but keeps the save data aligned on 4 bytes
    u32 persistent_state;
} JokerSaveData;
#define JOKER_DATA_SIZE 8

#define DTAG_JOKER    0x524B4F4A // Spells JOKR
#define DTAG_TAROT    0x544F5254 // Spells TROT
#define DTAG_PLANET   0x544E4C50 // Spells PLNT
#define DTAG_VOUCHER  0x52484356 // Spells VCHR
#define DTAG_SKIP_TAG 0x47415453 // Spells STAG
#define DTAG_END      0x444E45ff // Spells END
#define DTAG_SIZE     4

/**
 ** @brief Write raw binary data to SRAM
 *
 * @param sram_base pointer to address written to in the SRAM. Will get incremented by `size`.
 * @param bytes pointer to the written data
 * @param size number of bytes written
 */
static inline void write_sram(u32* sram_base, const u8* bytes, u32 size)
{
    // Don't know if check is necessary since we'll never reach 32kB of save data
    // if (*sram_base + size >= SRAM_SIZE)
    //     return;

    for (u32 i = 0; i < size; i++)
    {
        sram_mem[*sram_base] = bytes[i];
        (*sram_base)++;
    }
}

/**
 ** @brief Read raw binary data from SRAM
 *
 * @sa write_sram
 */
static inline void read_sram(u32* sram_base, u8* bytes, u32 size)
{
    // Same as write_sram
    // if (*sram_base + size >= SRAM_SIZE)
    //     return;

    for (u32 i = 0; i < size; i++)
    {
        bytes[i] = sram_mem[*sram_base];
        (*sram_base)++;
    }
}

/**
 ** @brief Clear entirety of SRAM by filling it with ones.
 */
static inline void clear_sram()
{
    for (u32 i = 0; i < SRAM_SIZE; i++)
    {
        sram_mem[i] = 0xff;
    }
}

/**
 ** @brief Checks the 7 chars of balatro_version after the "GBALATRO_VERSION" prefix
 *         representing the git hash of the code the build is based on.
 *
 * @returns true if the git hash of the ROM is equal to the hash saved in SRAM.
 *          false if they are different.
 */
static inline bool check_hash(const char* prefix)
{
    for (u32 i = 0; i < CHECK_HASH_SIZE; i++)
    {
        if (balatro_version[GIT_HASH_START + i] != prefix[i])
        {
            return false;
        }
    }

    return true;
}

/**
 ** @brief Determines if the current build is considered "dirty" aka has uncommited changes.
 *         This works because the balatro_version string has "-dirty" added at the end if it's
 *         dirty.
 *
 * @returns true if version is dirty, false otherwise.
 */
static inline bool is_version_dirty()
{
    return strlen(balatro_version) > CHECK_HASH_SIZE;
}

/**
 ** @brief Writes a magic number and ROM version info to SRAM to signal that the
 *         save data exists and allow the game to determine if it is compatible.
 */
static inline void set_save_header()
{
    SaveHeader check = {};
    check.magic = CHECK_MAGIC;
    check.dirty = is_version_dirty();
    memcpy(&(check.githash), (void*)(&balatro_version) + GIT_HASH_START, CHECK_HASH_SIZE);

    u32 write_address = CHECK_BASE;
    write_sram(&write_address, (const u8*)&check, sizeof(check));
}

/**
 ** @brief Reads whether the save data exists and is valid.
 *
 * @sa set_save_valid
 */
static inline bool check_save_header()
{
    SaveHeader check;
    u32 write_address = CHECK_BASE;
    read_sram(&write_address, (u8*)&check, sizeof(check));

    bool is_valid = (check.magic == CHECK_MAGIC) && check.dirty == is_version_dirty() &&
                    check_hash(check.githash);

    return is_valid;
}

void save_game(void)
{
    clear_sram();

    // Save fixed vars (money, etc)
    set_save_header();
    u32 write_address = GAME_BASE;
    write_sram(&write_address, (const u8*)&g_game_vars, sizeof(g_game_vars));

    write_address = LISTS_BASE;
    u32 list_tag = DTAG_JOKER;
    ListItr itr;

    // Save Jokers

    List* jokers = get_jokers_list();
    u32 nb_jokers = list_get_len(jokers);

    // Prefix array with delimiter and size of array so we can read it later
    write_sram(&write_address, (const u8*)&list_tag, DTAG_SIZE);
    write_sram(&write_address, (const u8*)&nb_jokers, sizeof(nb_jokers));

    if (nb_jokers > 0)
    {
        itr = list_itr_create(jokers);
        JokerObject* joker_object;
        while ((joker_object = list_itr_next(&itr)))
        {
            JokerSaveData data = {
                (u32)joker_object->joker->id,
                joker_object->joker->persistent_state
            };
            write_sram(&write_address, (const u8*)&data, sizeof(data));
        }
    }

    // Save Tarot cards

    list_tag = DTAG_TAROT;
    write_sram(&write_address, (const u8*)&list_tag, DTAG_SIZE);
    write_sram(&write_address, (const u8*)&nb_jokers, sizeof(nb_jokers));

    list_tag = DTAG_END;
    write_sram(&write_address, (const u8*)&list_tag, DTAG_SIZE);
}

void load_game(void)
{
    if (!check_save_header())
        return;

    u32 read_address = GAME_BASE;
    read_sram(&read_address, (u8*)&g_game_vars, sizeof(g_game_vars));

    // Data validation in case we start from junk data
    if (g_game_vars.game_speed < GAME_SPEED_MIN || g_game_vars.game_speed > GAME_SPEED_MAX)
        g_game_vars.game_speed = DEFAULT_GAME_SPEED;

    g_game_vars.high_contrast = ((u8)g_game_vars.high_contrast != 1) ? false : true;

    if (g_game_vars.music_volume > VOLUME_VALUE_MAX)
        g_game_vars.music_volume = DEFAULT_MUSIC_VOLUME;

    if (g_game_vars.sound_volume > VOLUME_VALUE_MAX)
        g_game_vars.sound_volume = DEFAULT_SOUND_VOLUME;

    // return to where we were in the random sequence so that the run stays reproducible
    for (u32 i = 0; i < g_game_vars.rng_step; i++)
    {
        (void)rand();
    }

    mmSetModuleVolume(MM_FULL_MODULE_VOLUME * g_game_vars.music_volume / VOLUME_VALUE_MAX);
}
