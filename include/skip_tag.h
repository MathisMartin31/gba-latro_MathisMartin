/**
 * @file skip_tag.h
 * @brief Data structures and functions related to the handling of Skip Tags
 */
#ifndef SKIP_TAGS_H
#define SKIP_TAGS_H

#include "graphic_utils.h"
#include "joker.h"
#include "sprite.h"

#include <tonc.h>

#define MAX_SKIP_TAGS 16

#define SKIP_TAG_SPRITE_OFFSET 4
// Put Tag sprites after the Jokers'
#define SKIP_TAG_STARTING_LAYER    (JOKER_STARTING_LAYER + MAX_ACTIVE_JOKERS)
#define SMALL_BLIND_SKIP_TAG_LAYER 0
#define BIG_BLIND_SKIP_TAG_LAYER   1
// There are max 2 skip tags visible in the Blind Select screen
// The sprites for the ones we own will be stored after them.
#define OWNED_SKIP_TAG_STARTING_LAYER 2
// Tile ID for the starting index in the tile memory
#define SKIP_TAG_TID (SKIP_TAG_STARTING_LAYER * SKIP_TAG_SPRITE_OFFSET)

#define INVESTMENT_TAG_REWARD 25

// clang-format off
/**
 * @brief List of all Skip Tags, in the order they are in on their spritesheet.
 *         Not all of them are implemented (or will be) but at least they are in order.
 *
 * @sa NB_SKIP_TAG_TYPES, ante1_skip_tags_roll_table, all_skip_tags_roll_table
 */
enum SkipTagTypes
{
    SKIP_TAG_TYPE_UNCOMMON,    // TODO
    SKIP_TAG_TYPE_RARE,        // TODO
    SKIP_TAG_TYPE_NEGATIVE,    // NOT PLANNED
    SKIP_TAG_TYPE_FOIL,        // NOT PLANNED
    SKIP_TAG_TYPE_HOLOGRAPHIC, // NOT PLANNED
    SKIP_TAG_TYPE_POLYCHROME,  // NOT PLANNED
    SKIP_TAG_TYPE_INVESTMENT,
    SKIP_TAG_TYPE_VOUCHER,     // NEEDS VOUCHERS
    SKIP_TAG_TYPE_BOSS,
    SKIP_TAG_TYPE_STANDARD,    // NEEDS PACKS
    SKIP_TAG_TYPE_CHARM,       // NEEDS PACKS
    SKIP_TAG_TYPE_METEOR,      // NEEDS PACKS
    SKIP_TAG_TYPE_BUFFOON,     // NEEDS PACKS
    SKIP_TAG_TYPE_HANDY,
    SKIP_TAG_TYPE_GARBAGE,
    SKIP_TAG_TYPE_ETHEREAL,    // NEEDS PACKS
    SKIP_TAG_TYPE_COUPON,      // TODO
    SKIP_TAG_TYPE_DOUBLE,
    SKIP_TAG_TYPE_JUGGLE,      // TODO
    SKIP_TAG_TYPE_D6,          // TODO
    SKIP_TAG_TYPE_TOP_UP,
    SKIP_TAG_TYPE_SPEED,
    SKIP_TAG_TYPE_ORBITAL,     // NEEDS HANDS MENU
    SKIP_TAG_TYPE_ECONOMY,
    MAX_SKIP_TAG_TYPES
};
// clang-format off

typedef struct SkipTag
{
    u8 type;
    SpriteObject* sprite_object;

} SkipTag;

enum SkipTagEvent
{
    SKIP_TAG_EVENT_NONE,           // For tags not yet impelemented, does nothing
    SKIP_TAG_EVENT_IMMEDIATE,      // Triggers immediately after being bought.
                                   // (e.g. Boss, Charm, Double,...)
    SKIP_TAG_EVENT_ON_ROUND_START, // Triggers when starting a new round (Juggler)
    SKIP_TAG_EVENT_ON_ROUND_END,   // Triggers when beating a round (Investment)
    SKIP_TAG_EVENT_ON_SHOP_INIT,   // Triggers when entering the Shop (e.g. Coupon, D6,...)
    SKIP_TAG_EVENT_ON_SHOP_REROLL  // Triggers when rolling items for sale in the Shop
                                   // (Foil, Holographic, Negative, Polychrome)
};

enum SkipTagEffect
{
    SKIP_TAG_EFFECT_NONE,
    SKIP_TAG_EFFECT_TRIGGER,
    SKIP_TAG_EFFECT_END
};

// SkipTagCallbacks will attempt to either return whether the SkipTag can be activated or trigger it
typedef bool (*SkipTagCondition)(void);
typedef void (*SkipTagCallback)(void);

typedef struct
{
    enum SkipTagEvent event_type;
    SkipTagCondition tag_condition_func;
    SkipTagCallback tag_effect_func;
} SkipTagInfo;

const SkipTagInfo* get_skip_tag_registry_entry(int tag_id);
size_t get_skip_tag_registry_size(void);

SkipTag* skip_tag_new(u8 tag_type);
void skip_tag_set_sprite(SkipTag* tag, BG_POINT pos, int layer);
void skip_tag_destroy(SkipTag** tag);
SkipTag* roll_skip_tag(void);

/**
 * @brief Checks if the given tag type is present in owned Tags list
 *
 * @param tag_type SkipTagType to check
 * @return true if tag type is present, false otherwise
 */
bool skip_tag_is_owned(u8 tag_type);

/**
 * @brief Adds the SkipTag to the owned list, snaps it into position and sets the original pointer
 *         to NULL
 *
 * @param blind_tag 
 */
void add_skip_tag(SkipTag** blind_tag);

/**
 * @brief Pops the Tag at index in owned list, destroys it and rearranges the remaining Tags' sprites
 *
 * @param tag_idx index of the Tag to remove in owned list
 */
void remove_skip_tag(int tag_idx);

enum SkipTagEffect skip_tag_check_and_apply_for_event_loop(int timer, enum SkipTagEvent tag_event);

#endif // SKIP_TAGS_H
