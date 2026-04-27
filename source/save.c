#include "save.h"

#include <string.h>

#include "game.h"
#include "joker.h"
#include "list.h"
#include "util.h"

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

typedef struct SaveCheckInfo
{
    u32 magic;
    bool dirty;
    char githash[7];
} SaveCheckInfo;

enum DelimiterTag
{
    DTAG_JOKER,
    DTAG_TAROT,
    DTAG_PLANET,
    DTAG_VOUCHER,
    DTAG_SKIP_TAG,
    DTAG_END,
    DTAG_INVALID = UNDEFINED
};

extern char balatro_version[];

// DelimiterTag is an enum of size sizeof(int) = 4
#define CARDS_TAG_SIZE 4

/**
 ** @brief Write raw binary data to SRAM
 *
 * @param sram_base address written to in the SRAM
 * @param bytes pointer to the written data
 * @param size number of bytes written
 */
static inline void write_sram(u32 sram_base, const u8* bytes, u32 size)
{
    if (sram_base + size >= SRAM_SIZE)
        return;

    for(u32 i = 0; i < size; i++)
    {
        sram_mem[sram_base + i] = bytes[i];
    }
}

/**
 ** @brief Read raw binary data from SRAM
 *
 * @sa write_sram
 */
static inline void read_sram(u32 sram_base, u8* bytes, u32 size)
{
    if (sram_base + size >= SRAM_SIZE)
        return;

    for(u32 i = 0; i < size; i++)
    {
        bytes[i] = sram_mem[sram_base + i];
    }
}

// Check 7 chars of balatrà_version after the "GBALATRO_VERSION" preffix 
static inline bool check_hash(const char* preffix)
{
    for (u32 i = 0; i < CHECK_HASH_SIZE; i++)
    {
        if (balatro_version[GIT_HASH_START + i] != preffix[i])
        {
            return false;
        }
    }

    return true;
}

// This works because the balatro_version has "-dirty" added at the end if it's dirty
static inline bool is_version_dirty()
{
    return strlen(balatro_version) > CHECK_HASH_SIZE;
}

static inline bool check_save()
{
    SaveCheckInfo check;
    read_sram(CHECK_BASE, (u8*)&check, sizeof(check));

    bool is_valid = (check.magic == CHECK_MAGIC) &&
                    check.dirty == is_version_dirty() &&
                    check_hash(check.githash);

    return is_valid;
}

static inline void set_save_valid()
{
    SaveCheckInfo check = {};
    check.magic = CHECK_MAGIC;
    check.dirty = is_version_dirty();
    memcpy(
        &(check.githash),
        (void*)(&balatro_version) + GIT_HASH_START,
        CHECK_HASH_SIZE
    );

    write_sram(CHECK_BASE, (const u8*)&check, sizeof(check));
}

/**
 ** @brief Save game data to SRAM.
 */
void save_game(void)
{
    set_save_valid();
    write_sram(GAME_BASE, (const u8*)&game_vars, sizeof(game_vars));
}

/**
 ** @brief Load game data from SRAM.
 *
 * @sa save_game
 */
void load_game(void)
{
    if (!check_save())
        return;

    read_sram(GAME_BASE, (u8*)&game_vars, sizeof(game_vars));

    // Data validation in case we start from junk data
    if (game_vars.game_speed < GAME_SPEED_MIN || game_vars.game_speed > GAME_SPEED_MAX)
        game_vars.game_speed = DEFAULT_GAME_SPEED;

    game_vars.high_contrast = ((u8)game_vars.high_contrast != 1) ? false : true;

    if (game_vars.music_volume > VOLUME_VALUE_MAX)
        game_vars.music_volume = DEFAULT_MUSIC_VOLUME;

    if (game_vars.sound_volume > VOLUME_VALUE_MAX)
        game_vars.sound_volume = DEFAULT_SOUND_VOLUME;
}
