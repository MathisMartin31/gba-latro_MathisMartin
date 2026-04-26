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

#define VERSION_BASE 0x0000
#define GAME_BASE    0x0010
#define LISTS_BASE   0x0060

// Spells GBAL, used to determine if the save data is junk
#define SAVE_VALIDATION_MAGIC 0x4C414247
#define SAVE_VALIDATION_SIZE  10

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

static inline bool check_save()
{
    u8 validation[SAVE_VALIDATION_SIZE] = {0};
    read_sram(VERSION_BASE, validation, SAVE_VALIDATION_SIZE);

    u32 magic_word  = validation[0] + (validation[1] << 8) + (validation[2] << 16) + (validation[3] << 24);
    //u32 githash_low = validation[4] + (validation[5] << 8) + (validation[6] << 16) + (validation[7] << 24);
    //u16 githash_high = validation[8] + (validation[9] << 8);

    bool is_valid = (magic_word == SAVE_VALIDATION_MAGIC);

    return is_valid;
}

static inline void set_save_valid()
{
    u8 validation[SAVE_VALIDATION_SIZE] = {0};
    u32 magic_word = SAVE_VALIDATION_MAGIC;
    memcpy(validation, &magic_word, 4);
    write_sram(VERSION_BASE, validation, SAVE_VALIDATION_SIZE);
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
