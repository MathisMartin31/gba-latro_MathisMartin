#include "save.h"

#include <string.h>

#include "game.h"
#include "joker.h"
#include "list.h"
#include "util.h"

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

#define OPTIONS_BASE            0x0010
#define OPTIONS_SPEED_OFFSET    0
#define OPTIONS_CONTRAST_OFFSET 4
#define OPTIONS_MUSIC_OFFSET    5
#define OPTIONS_SOUND_OFFSET    6

#define GAME_BASE                0x0030
#define GAME_SAVE_VERSION_OFFSET 0
#define GAME_SEED_OFFSET         4
#define GAME_MONEY_OFFSET        8
#define GAME_ANTE_OFFSET         12
#define GAME_ROUND_OFFSET        16
#define GAME_BLIND_OFFSET        20
#define GAME_NEXT_BOSS_OFFSET    24

#define CARDS_BASE 0x0060

// This will be used to determine if the save is compatible
#define SAVEFILE_VERSION 0

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
    u32 clear[80] = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
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
 ** @brief Load options values from SRAM.
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

/**
 ** @brief Save game data to SRAM.
 *
 * @param vars_ptr Pointer to the game_variables structure holding global data.
 */
void save_game(GameVariables* vars_ptr)
{
    // Fixed position vars

    u32 savefile_version = SAVEFILE_VERSION;
    int money = get_money();
    int ante = get_ante();
    int round = get_round();
    int current_blind = get_current_blind();
    int next_boss_blind = get_next_boss_blind();

    // clang-format off
    sram_write32(&savefile_version,     GAME_BASE, GAME_SAVE_VERSION_OFFSET);
    sram_write32(&(vars_ptr->rng_seed), GAME_BASE, GAME_SEED_OFFSET);
    sram_write32(&money,                GAME_BASE, GAME_MONEY_OFFSET);
    sram_write32(&ante,                 GAME_BASE, GAME_ANTE_OFFSET);
    sram_write32(&round,                GAME_BASE, GAME_ROUND_OFFSET);
    sram_write32(&current_blind,        GAME_BASE, GAME_BLIND_OFFSET);
    sram_write32(&next_boss_blind,      GAME_BASE, GAME_NEXT_BOSS_OFFSET);
    // clang-format on

    // Variable length stuff.
    // See discussion https://github.com/GBALATRO/balatro-gba/discussions/450 for
    // technical details on how to handle saving/loading lists of objects.

    u32 current_offset = 0;
    int current_dtag;

    List* jokers = get_jokers_list();
    ListItr itr = list_itr_create(jokers);
    JokerObject* joker_object;
    current_dtag = DTAG_JOKER;
    while ((joker_object = list_itr_next(&itr)))
    {
        sram_write32(&current_dtag, CARDS_BASE, current_offset);
        current_offset += CARDS_TAG_SIZE;
        sram_write8(&(joker_object->joker->id), CARDS_BASE, current_offset);
        current_offset += 1;
        sram_write32(&(joker_object->joker->persistent_state), CARDS_BASE, current_offset);
        current_offset += 4;
    }

    // TODO: Consumables
    // TODO: Skip Tags
    // TODO: Vouchers

    current_dtag = DTAG_END;
    sram_write32(&current_dtag, CARDS_BASE, current_offset);
}

/**
 ** @brief Load game data from SRAM.
 *
 * @sa save_game
 */
void load_game(GameVariables* vars_ptr)
{
    // Check save version, exit immediately if mismatched
    u32 save_version;
    sram_read32(&save_version, GAME_BASE, GAME_SAVE_VERSION_OFFSET);
    if (save_version != SAVEFILE_VERSION)
    {
        return;
    }

    // TODO: Fixed position vars

    // Variable length stuff.

    List* jokers = get_jokers_list();
    u8 id;
    u32 persistent_state;
    Joker* joker;
    JokerObject* joker_object;

    int current_dtag = DTAG_INVALID;
    int current_offset = 0;

    do
    {
        // Read delimiter tag to determine how to handle the data
        sram_read32(&current_dtag, CARDS_BASE, current_offset);
        current_offset += 4;

        switch (current_dtag)
        {
            case DTAG_JOKER:
            {
                sram_read8(&id, CARDS_BASE, current_offset);
                current_offset += 1;
                sram_read32(&persistent_state, CARDS_BASE, current_offset);
                current_offset += 4;

                joker = joker_new(id);
                joker->persistent_state = persistent_state;
                joker_object = joker_object_new(joker);

                list_push_back(jokers, joker_object);

                break;
            }

            case DTAG_END:
                return;
            
            default:
                continue;
        }
    } while (current_dtag != DTAG_END);
}

