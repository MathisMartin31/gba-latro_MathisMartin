#include "save.h"

#include <string.h>

// See https://gbadev.net/gbadoc/memory.html for more details on SRAM
// A few important pieces of info:
//   - Read/Writes are limited to 8-bits words.
//   - Saving and loading bigger values in one memcpy does not work and results
//     in weird memory contents.
//   - Memory is filled with 1s by default
//     (at least in mgba, not sure about real HW)

// Sections' bases are arbitrary and with a lot of margin to work with.
// Offsets are relative to the sections' base and are in bytes
// (the size of a write in SRAM)
// This can be moved to a separate file if it starts to become too long.

#define OPTIONS_BASE            0x0000
#define OPTIONS_SPEED_OFFSET    0
#define OPTIONS_CONTRAST_OFFSET 4
#define OPTIONS_MUSIC_OFFSET    5
#define OPTIONS_SOUND_OFFSET    6

#define GAME_BASE         0x1000
#define GAME_TIMER_OFFSET 0
#define GAME_SEED_OFFSET  4

/**
 * @brief Write a single 8-bits word to SRAM.
 *
 * @param data Pointer to the data to be written to SRAM.
 * @param sram_base Address of the SRAM section to be written to (options, game state...).
 * @param offset Position of the word to write, in numbers of words relative to the section start.
 */
static inline void sram_write8(const void* data, u32 sram_base, u32 offset)
{
    memcpy(sram_mem + sram_base + offset, data, 1);
}

/**
 * @brief Read a single 8-bits word from SRAM.
 *
 * @sa sram_write8
 */
static inline void sram_read8(void* data, u32 sram_base, u32 offset)
{
    memcpy(data, sram_mem + sram_base + offset, 1);
}

/**
 * @brief Write a 16-bits word to SRAM by interpreting it as 2 8-bits words.
 *
 * @sa sram_write8
 */
static inline void sram_write16(const void* data, u32 sram_base, u32 offset)
{
    const void* l_addr = data;
    const void* r_addr = data + 1;
    sram_write8(l_addr, sram_base, offset);
    sram_write8(r_addr, sram_base, offset + 1);
}

/**
 * @brief Read a 16-bits word from SRAM.
 *
 * @sa sram_write16
 */
static inline void sram_read16(void* data, u32 sram_base, u32 offset)
{
    void* l_addr = data;
    void* r_addr = data + 1;
    sram_read8(l_addr, sram_base, offset);
    sram_read8(r_addr, sram_base, offset + 1);
}

/**
 * @brief Write a 32-bits word to SRAM.
 *        This is done by recursively intepreting it as 2 16-bits words, then 4 8-bits words.
 *
 * @sa sram_write16
 */
static inline void sram_write32(const void* data, u32 sram_base, u32 offset)
{
    const void* l_addr = data;
    const void* r_addr = data + 2;
    sram_write16(l_addr, sram_base, offset);
    sram_write16(r_addr, sram_base, offset + 2);
}

/**
 * @brief Read a 32-bits word from SRAM.
 *
 * @sa sram_write32
 */
static inline void sram_read32(void* data, u32 sram_base, u32 offset)
{
    void* l_addr = data;
    void* r_addr = data + 2;
    sram_read16(l_addr, sram_base, offset);
    sram_read16(r_addr, sram_base, offset + 2);
}

/**
 * @brief Write a boolean to SRAM inside a single 8-bits word.
 *
 * @sa sram_write8
 */
static inline void sram_writeBool(const bool* data, u32 sram_base, u32 offset)
{
    u8 data_int = *data ? 1 : 0;
    memcpy(sram_mem + sram_base + offset, &data_int, 1);
}

/**
 * @brief Read a single boolean from SRAM.
 *
 * @sa sram_read8
 */
static inline void sram_readBool(void* data, u32 sram_base, u32 offset)
{
    u8 data_int;
    memcpy(&data_int, sram_mem + sram_base + offset, 1);
    *((bool*)data) = (data_int == 1) ? true : false;
}

/**
 * @brief Do a write of 1s big enough to cover all the currently used SRAM space
 */
void clear_sram(void)
{
    // clang-format off
    u32 clear[40] = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
    };
    // clang-format on
    memcpy(sram_mem, clear, sizeof(clear));
}

/**
 * @brief Save options values into SRAM. This includes game speed, high contrast
 *        cards usage, and music and sounds volume slider values.
 *
 * @param vars_ptr Pointer to the game_variables structure holding all the data.
 */
void save_options(GameVariables* vars_ptr)
{
    // clang-format off
    sram_write32  (&(vars_ptr->game_speed),    OPTIONS_BASE, OPTIONS_SPEED_OFFSET);
    sram_writeBool(&(vars_ptr->high_contrast), OPTIONS_BASE, OPTIONS_CONTRAST_OFFSET);
    sram_write8   (&(vars_ptr->music_volume),  OPTIONS_BASE, OPTIONS_MUSIC_OFFSET);
    sram_write8   (&(vars_ptr->sound_volume),  OPTIONS_BASE, OPTIONS_SOUND_OFFSET);
    // clang-format on
}

/**
 ** @brief Save options values from SRAM.
 *
 * @sa save_options
 */
void load_options(GameVariables* vars_ptr)
{
    // clang-format off
    sram_read32  (&(vars_ptr->game_speed),    OPTIONS_BASE, OPTIONS_SPEED_OFFSET);
    sram_readBool(&(vars_ptr->high_contrast), OPTIONS_BASE, OPTIONS_CONTRAST_OFFSET);
    sram_read8   (&(vars_ptr->music_volume),  OPTIONS_BASE, OPTIONS_MUSIC_OFFSET);
    sram_read8   (&(vars_ptr->sound_volume),  OPTIONS_BASE, OPTIONS_SOUND_OFFSET);
    // clang-format on

    // Data validation
    if (vars_ptr->game_speed < GAME_SPEED_MIN || vars_ptr->game_speed > GAME_SPEED_MAX)
        vars_ptr->game_speed = DEFAULT_GAME_SPEED;

    if (vars_ptr->music_volume > VOLUME_VALUE_MAX)
        vars_ptr->music_volume = DEFAULT_MUSIC_VOLUME;

    if (vars_ptr->sound_volume > VOLUME_VALUE_MAX)
        vars_ptr->sound_volume = DEFAULT_SOUND_VOLUME;
}
