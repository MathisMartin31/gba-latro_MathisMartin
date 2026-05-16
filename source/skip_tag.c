#include "skip_tag.h"

#include "joker.h"
#include "game_variables.h"
#include "graphic_utils.h"
#include "pool.h"
#include "skip_tags_gfx.h"

#include <stdio.h>
#include <stdlib.h> //#include "random.h"
#include <tonc.h>

#define MAX_NON_OVERLAPING_TAG_SPRITES 5

#define NB_ANTE1_SKIP_TAGS 9

// clang-format off
// Points                                         x    y
static const BG_POINT OWNED_SKIP_TAGS_BASE_POS = {219, 97};
// Sizes
static const int SKIP_TAG_SPRITE_HEIGHT       = 10;
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

void skip_tag_set_sprite(SkipTag* tag, int layer)
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
    sprite_object_set_sprite(tag->sprite_object, sprite);
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

static void rearrange_skip_tags_sprites(void)
{
    FIXED prev_tags_posX[MAX_SKIP_TAGS] = {0};
    FIXED prev_tags_posY[MAX_SKIP_TAGS] = {0};

    // Start by destroying all the sprites only so the newly ordered sprites don't overlap
    SkipTag* tag;
    ListItr owned_tags_itr = list_itr_create(&g_game_vars.owned_skip_tags);
    int i = 0;
    while ((tag = list_itr_next(&owned_tags_itr)))
    {
        prev_tags_posX[i] = tag->sprite_object->x;
        prev_tags_posY[i] = tag->sprite_object->y;
        sprite_object_destroy(&(tag->sprite_object));
        i++;
    }

    int nb_owned_tags = i;
    int even_spacing =
        (OWNED_SKIP_TAGS_STACK_HEIGHT - SKIP_TAG_SPRITE_HEIGHT) / (nb_owned_tags - 1);

    // Then recreate them all in order
    tag = NULL;
    owned_tags_itr = list_itr_create(&g_game_vars.owned_skip_tags);
    i = 0;
    while ((tag = list_itr_next(&owned_tags_itr)))
    {
        skip_tag_set_sprite(tag, OWNED_SKIP_TAG_STARTING_LAYER + i);

        // Restore the previous position so that the sprite doesn't spawn at (0,0)
        tag->sprite_object->x = prev_tags_posX[i];
        tag->sprite_object->y = prev_tags_posY[i];
        tag->sprite_object->tx = int2fx(OWNED_SKIP_TAGS_BASE_POS.x);

        // Direct it towards its new position.
        // Need to fit all tags (10px each, 2px spacing) vertically within a height of 64px

        // 5 tags fit nicely in `5*10 + 4*2 = 58px` but not one more
        if (nb_owned_tags <= MAX_NON_OVERLAPING_TAG_SPRITES)
        {
            tag->sprite_object->ty = int2fx(OWNED_SKIP_TAGS_BASE_POS.y - i * OWNED_SKIP_TAGS_SPACING);
        }
        // Beyond that, we need to compute the spacing to distribute sprites evenly,
        // knowing the first one doesn't move.
        // This means we have to spread `nb_tags-1` tags over `64 - 10 = 54px`.
        else
        {
            tag->sprite_object->ty = int2fx(OWNED_SKIP_TAGS_BASE_POS.y - i * even_spacing);
        }

        i++;

        sprite_object_update(tag->sprite_object);

        tte_printf(
            "#{P:%d,%d; cx:0x%X000}%d:(%ld,%ld)",
            0, 8*i,
            TTE_RED_PB,
            i,
            tag->sprite_object->x,
            tag->sprite_object->y
        );
    }
}

void add_skip_tag(SkipTag** blind_tag)
{
    if (blind_tag == NULL)
        return;

    SkipTag* new_tag = skip_tag_new((*blind_tag)->type);
    int new_tag_layer = list_get_len(&g_game_vars.owned_skip_tags);

    skip_tag_set_sprite(new_tag, OWNED_SKIP_TAG_STARTING_LAYER + new_tag_layer);

    BG_POINT to = {
        OWNED_SKIP_TAGS_BASE_POS.x,
        OWNED_SKIP_TAGS_BASE_POS.y - new_tag_layer * OWNED_SKIP_TAGS_SPACING
    };

    sprite_object_snap_to(new_tag->sprite_object, to, true);

    // Add to the back, so that the oldest (at the bottom) has the lowest sprite
    // index and is thus shown on top of the others
    list_push_back(&g_game_vars.owned_skip_tags, new_tag);
    skip_tag_destroy(blind_tag);

    return;
    rearrange_skip_tags_sprites();
}

void remove_skip_tag(int tag_idx)
{

}
