#include "skip_tag.h"

#include "joker.h"
#include "game_variables.h"
#include "pool.h"
#include "skip_tags_gfx.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h> //#include "random.h"
#include <tonc.h>

#define MAX_NON_OVERLAPING_TAG_SPRITES 5

#define NB_ANTE1_SKIP_TAGS 9

// clang-format off
// Points                                         x    y
static const BG_POINT OWNED_SKIP_TAGS_BASE_POS = {219, 97};
// Sizes
static const int OWNED_SKIP_TAGS_STACK_HEIGHT = 64;
static const int OWNED_SKIP_TAGS_SPACING      = 12;
// clang-format on

// Not all Skip Tags can be drawn on Ante 1, so we need 2 different roll tables
// one for Ante 1 only, and one for all other Antes.
static const u8 ante1_skip_tags_roll_table[NB_ANTE1_SKIP_TAGS] = {
    SKIP_TAG_TYPE_UNCOMMON,
    SKIP_TAG_TYPE_RARE,
    SKIP_TAG_TYPE_INVESTMENT,
    SKIP_TAG_TYPE_BOSS,
    SKIP_TAG_TYPE_COUPON,
    SKIP_TAG_TYPE_DOUBLE,
    SKIP_TAG_TYPE_JUGGLE,
    SKIP_TAG_TYPE_SPEED,
    SKIP_TAG_TYPE_ECONOMY
};
static const u8 all_skip_tags_roll_table[NB_SKIP_TAG_TYPES] = {
    SKIP_TAG_TYPE_UNCOMMON,
    SKIP_TAG_TYPE_RARE,
    SKIP_TAG_TYPE_INVESTMENT,
    SKIP_TAG_TYPE_BOSS,
    SKIP_TAG_TYPE_HANDY,
    SKIP_TAG_TYPE_GARBAGE,
    SKIP_TAG_TYPE_COUPON,
    SKIP_TAG_TYPE_DOUBLE,
    SKIP_TAG_TYPE_JUGGLE,
    SKIP_TAG_TYPE_TOP_UP,
    SKIP_TAG_TYPE_SPEED,
    SKIP_TAG_TYPE_ECONOMY
};

// SkipTag new/destroy functions

SkipTag* skip_tag_new(u8 tag_type)
{
    SkipTag* tag = POOL_GET(SkipTag);

    tag->type = tag_type;
    tag->sprite_object = sprite_object_new();

    return tag;
}

void skip_tag_set_sprite(SkipTag* tag, BG_POINT pos, int layer)
{
    if (tag == NULL)
        return;

    // Set tags palette the first time we ask for a sprite
    static bool pb_init = false;
    if (!pb_init)
    {
        GRIT_CPY(&pal_obj_mem[PAL_ROW_LEN * SKIP_TAGS_PB], skip_tags_gfxPal);
        pb_init = true;
    }

    int tile_index = SKIP_TAG_TID + (layer * SKIP_TAG_SPRITE_OFFSET);
    memcpy32(
        &tile_mem[TILE_MEM_OBJ_CHARBLOCK0_IDX][tile_index],
        &skip_tags_gfxTiles
            [tag->type * SKIP_TAG_SPRITE_OFFSET * TILE_SIZE],
        SKIP_TAG_SPRITE_OFFSET * TILE_SIZE
    );
    Sprite* sprite = sprite_new(
        ATTR0_SQUARE | ATTR0_4BPP | ATTR0_AFF,
        ATTR1_SIZE_16,
        tile_index,
        SKIP_TAGS_PB,
        layer + SKIP_TAG_STARTING_LAYER
    );
    sprite_position(sprite, pos.x, pos.y);
    sprite_object_set_sprite(tag->sprite_object, sprite);
    sprite_object_snap_to(tag->sprite_object, pos, false);
    sprite_object_update(tag->sprite_object);
}

void skip_tag_destroy(SkipTag** tag)
{
    if (*tag == NULL)
        return;
    sprite_object_destroy(&((*tag)->sprite_object));
    POOL_FREE(SkipTag, *tag);
    *tag = NULL;
}

// Misc functions

SkipTag* roll_skip_tag(void)
{
    u8 tag_type = 0;
    if (g_game_vars.ante == 1)
    {
        tag_type = ante1_skip_tags_roll_table[rand() % NB_ANTE1_SKIP_TAGS];
    }
    else
    {
        tag_type = all_skip_tags_roll_table[rand() % NB_SKIP_TAG_TYPES];
    }

    return skip_tag_new(tag_type);
}

static inline int get_skip_tag_sprites_spacing(void)
{
    int nb_owned_tags = list_get_len(&g_game_vars.owned_skip_tags);
    return (nb_owned_tags <= MAX_NON_OVERLAPING_TAG_SPRITES)
             ? OWNED_SKIP_TAGS_SPACING
             : OWNED_SKIP_TAGS_STACK_HEIGHT / nb_owned_tags;
}

static void rearrange_skip_tag_sprites(int nb_owned_tags, int tag_spacing)
{
    BG_POINT unused_pos = {UNDEFINED, UNDEFINED};
    
    for (int idx = 0; idx < nb_owned_tags; idx++)
    {
        SkipTag* tmp_tag = list_get_at_idx(&g_game_vars.owned_skip_tags, idx);
        BG_POINT dest_pos = {
            OWNED_SKIP_TAGS_BASE_POS.x,
            OWNED_SKIP_TAGS_BASE_POS.y - idx * tag_spacing
        };
        sprite_object_slide_from_to(tmp_tag->sprite_object, unused_pos, dest_pos);
    }
}

void add_skip_tag(SkipTag** blind_tag)
{
    if (blind_tag == NULL)
        return;

    SkipTag* new_tag = skip_tag_new((*blind_tag)->type);
    // Add to the back, so that the oldest (at the bottom) has the lowest sprite
    // index and is thus shown on top of the others
    list_push_back(&g_game_vars.owned_skip_tags, new_tag);

    int nb_owned_tags = list_get_len(&g_game_vars.owned_skip_tags);

    BG_POINT old_tag_pos = {
        fx2int((*blind_tag)->sprite_object->x),
        fx2int((*blind_tag)->sprite_object->y)
    };
    BG_POINT new_tag_pos = OWNED_SKIP_TAGS_BASE_POS;

    // If all the tags fix on the screen without issue,
    // just move the new tag to the top of the stack and that's it
    if (nb_owned_tags <= MAX_NON_OVERLAPING_TAG_SPRITES)
    {
        new_tag_pos.y -= (nb_owned_tags - 1) * OWNED_SKIP_TAGS_SPACING;
    }
    // If it's going to overlap the consumables' frame, then we must move all
    // existing tags down a bit so we can put the new one at the very top
    else
    {
        int even_spacing = get_skip_tag_sprites_spacing();

        // Exclude the new one from the loop here, as it will be snapped into place directly
        rearrange_skip_tag_sprites(nb_owned_tags - 1, even_spacing);
        new_tag_pos.y -= even_spacing * (nb_owned_tags - 1);
    }

    skip_tag_set_sprite(new_tag, old_tag_pos, OWNED_SKIP_TAG_STARTING_LAYER + nb_owned_tags);
    sprite_object_snap_to(new_tag->sprite_object, new_tag_pos, true);
    sprite_object_update(new_tag->sprite_object);

    skip_tag_destroy(blind_tag);

    // TODO: REMOVE
    // Need to call it someplace where a timer is put in place
    // so it doesn't all happen in one frame.
    // Just like Jokers when scoring a hand.
    int i = 0;
    check_and_apply_tag_for_event(&i, SKIP_TAG_EVENT_IMMEDIATE);
}

void remove_skip_tag(int tag_idx)
{
    // Remove the tag itself
    SkipTag* tag = list_get_at_idx(&g_game_vars.owned_skip_tags, tag_idx);
    (void)list_remove_at_idx(&g_game_vars.owned_skip_tags, tag_idx);
    skip_tag_destroy(&tag);

    rearrange_skip_tag_sprites(list_get_len(&g_game_vars.owned_skip_tags), get_skip_tag_sprites_spacing());
}

// Use Tags list index instead of iterator because we need to be able to remove
// Tags while we are iterating. Might change it later though.
void check_and_apply_tag_for_event(int* tag_idx, enum SkipTagEvent tag_event)
{
    for (; *tag_idx < list_get_len(&g_game_vars.owned_skip_tags); (*tag_idx)++)
    {
        SkipTag* tag = list_get_at_idx(&g_game_vars.owned_skip_tags, *tag_idx);
        const SkipTagInfo* info = get_skip_tag_registry_entry(tag->type);

        if (info == NULL || info->event_type != tag_event)
            continue;

        // return early in case the tag triggers/is consumed
        if (info->tag_effect_func())
        {
            //tte_printf(
            //    "#{P:0,0; cx:0x%X000}Scored Tag type: %d",
            //    TTE_RED_PB,
            //    tag->type
            //);
            remove_skip_tag(*tag_idx);
            return;
        }
    }
}
