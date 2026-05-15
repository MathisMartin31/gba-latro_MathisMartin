#ifndef SKIP_TAGS_H
#define SKIP_TAGS_H

#include "joker.h"
#include "sprite.h"

#include <tonc.h>

#define MAX_SKIP_TAGS 16

#define SKIP_TAG_SPRITE_OFFSET 4
// Put Tag sprites after the Jokers'
#define SKIP_TAG_STARTING_LAYER (JOKER_STARTING_LAYER + MAX_ACTIVE_JOKERS)
#define SMALL_BLIND_SKIP_TAG_LAYER 0
#define BIG_BLIND_SKIP_TAG_LAYER 1
// There are max 2 skip tags visible in the Blind Select screen
// The sprites for the ones we own will be stored after them.
#define OWNED_SKIP_TAG_STARTING_LAYER 2
// Tile ID for the starting index in the tile memory
#define SKIP_TAG_TID (SKIP_TAG_STARTING_LAYER * SKIP_TAG_SPRITE_OFFSET)
#define SKIP_TAGS_PB 3

enum SkipTagEvent
{
    SKIP_TAG_EVENT_INSTANT,
    SKIP_TAG_EVENT_ON_SHOP,
    SKIP_TAG_EVENT_ON_ROUND,
    SKIP_TAG_EVENT_ON_BOSS_BLIND
};

#define SKIP_TAG_TYPE_UNCOMMON   0
#define SKIP_TAG_TYPE_RARE       1
#define SKIP_TAG_TYPE_INVESTMENT 6
#define SKIP_TAG_TYPE_BOSS       8
#define SKIP_TAG_TYPE_HANDY      13
#define SKIP_TAG_TYPE_GARBAGE    14
#define SKIP_TAG_TYPE_COUPON     16
#define SKIP_TAG_TYPE_DOUBLE     17
#define SKIP_TAG_TYPE_JUGGLE     18
#define SKIP_TAG_TYPE_TOP_UP     20
#define SKIP_TAG_TYPE_SPEED      21
#define SKIP_TAG_TYPE_ECONOMY    23

#define NB_SKIP_TAG_TYPES 12

typedef struct SkipTag
{
    u8 type;
    SpriteObject* sprite_object;

} SkipTag;

SkipTag* skip_tag_new(u8 tag_type);
void skip_tag_set_sprite(SkipTag* tag, int layer);
void skip_tag_destroy(SkipTag** tag);
SkipTag* roll_skip_tag(void);

void add_skip_tag(SkipTag** blind_tag);
void remove_skip_tag(int tag_idx);

#endif // SKIP_TAGS_H
