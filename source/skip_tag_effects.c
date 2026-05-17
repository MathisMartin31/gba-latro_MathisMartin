#include "game.h"
#include "game_variables.h"
#include "skip_tag.h"
#include "util.h"

#include <stdlib.h>
#include <tonc.h>

#define REGISTER_SKIP_TAG_EFFECT_FUNC(tag_effect_name) \
    static bool tag_effect_name();    \

REGISTER_SKIP_TAG_EFFECT_FUNC(skip_tag_effect_noop)
REGISTER_SKIP_TAG_EFFECT_FUNC(skip_tag_effect_uncommon)
REGISTER_SKIP_TAG_EFFECT_FUNC(skip_tag_effect_rare)
REGISTER_SKIP_TAG_EFFECT_FUNC(skip_tag_effect_investment)
REGISTER_SKIP_TAG_EFFECT_FUNC(skip_tag_effect_boss)
REGISTER_SKIP_TAG_EFFECT_FUNC(skip_tag_effect_handy)
REGISTER_SKIP_TAG_EFFECT_FUNC(skip_tag_effect_garbage)
REGISTER_SKIP_TAG_EFFECT_FUNC(skip_tag_effect_coupon)
REGISTER_SKIP_TAG_EFFECT_FUNC(skip_tag_effect_double)
REGISTER_SKIP_TAG_EFFECT_FUNC(skip_tag_effect_juggle)
REGISTER_SKIP_TAG_EFFECT_FUNC(skip_tag_effect_top_up)
REGISTER_SKIP_TAG_EFFECT_FUNC(skip_tag_effect_speed)
REGISTER_SKIP_TAG_EFFECT_FUNC(skip_tag_effect_economy)

// The index of a skip tag in the registry matches its ID.
// clang-format off
const SkipTagInfo skip_tag_registry[] = 
{
    { SKIP_TAG_EVENT_ON_SHOP,      skip_tag_effect_uncommon   }, // UNCOMMON   = 0
    { SKIP_TAG_EVENT_ON_SHOP,      skip_tag_effect_rare       }, // RARE       = 1
    { SKIP_TAG_EVENT_NONE,         skip_tag_effect_noop       }, // 2
    { SKIP_TAG_EVENT_NONE,         skip_tag_effect_noop       }, // 3
    { SKIP_TAG_EVENT_NONE,         skip_tag_effect_noop       }, // 4
    { SKIP_TAG_EVENT_NONE,         skip_tag_effect_noop       }, // 5
    { SKIP_TAG_EVENT_ON_ROUND_END, skip_tag_effect_investment }, // INVESTMENT = 6
    { SKIP_TAG_EVENT_NONE,         skip_tag_effect_noop       }, // 7
    { SKIP_TAG_EVENT_IMMEDIATE,    skip_tag_effect_boss       }, // BOSS       = 8
    { SKIP_TAG_EVENT_NONE,         skip_tag_effect_noop       }, // 9
    { SKIP_TAG_EVENT_NONE,         skip_tag_effect_noop       }, // 10
    { SKIP_TAG_EVENT_NONE,         skip_tag_effect_noop       }, // 11
    { SKIP_TAG_EVENT_NONE,         skip_tag_effect_noop       }, // 12
    { SKIP_TAG_EVENT_IMMEDIATE,    skip_tag_effect_handy      }, // HANDY      = 13
    { SKIP_TAG_EVENT_IMMEDIATE,    skip_tag_effect_garbage    }, // GARBAGE    = 14
    { SKIP_TAG_EVENT_NONE,         skip_tag_effect_noop       }, // 15
    { SKIP_TAG_EVENT_ON_SHOP,      skip_tag_effect_coupon     }, // COUPON     = 16
    { SKIP_TAG_EVENT_IMMEDIATE,    skip_tag_effect_double     }, // DOUBLE     = 17
    { SKIP_TAG_EVENT_ON_ROUND,     skip_tag_effect_juggle     }, // JUGGLE     = 18
    { SKIP_TAG_EVENT_NONE,         skip_tag_effect_noop       }, // 19
    { SKIP_TAG_EVENT_IMMEDIATE,    skip_tag_effect_top_up     }, // TOP_UP     = 20
    { SKIP_TAG_EVENT_IMMEDIATE,    skip_tag_effect_speed      }, // SPEED      = 21
    { SKIP_TAG_EVENT_NONE,         skip_tag_effect_noop       }, // 22
    { SKIP_TAG_EVENT_IMMEDIATE,    skip_tag_effect_economy    }, // ECONOMY    = 23
};
// clang-format on

static const size_t skip_tag_registry_size = NUM_ELEM_IN_ARR(skip_tag_registry);

const SkipTagInfo* get_skip_tag_registry_entry(int tag_id)
{
    if (tag_id < 0 || (size_t)tag_id >= skip_tag_registry_size)
    {
        return NULL;
    }
    return &skip_tag_registry[tag_id];
}

size_t get_skip_tag_registry_size(void)
{
    return skip_tag_registry_size;
}

// EFFECTS IMPLEMENTATION

static bool skip_tag_effect_noop()
{
    return false;
}

static bool skip_tag_effect_uncommon()
{
    return false;
}

static bool skip_tag_effect_rare()
{
    return false;
}

static bool skip_tag_effect_investment()
{
    if (g_game_vars.current_blind != BLIND_TYPE_BOSS)
        return false;

    // TODO: reroll boss blind

    return true;
}

static bool skip_tag_effect_boss()
{
    return true;
}

static bool skip_tag_effect_handy()
{
    return true;
}

static bool skip_tag_effect_garbage()
{
    return false;
}

static bool skip_tag_effect_coupon()
{
    return false;
}

static bool skip_tag_effect_double()
{
    return false;
}

static bool skip_tag_effect_juggle()
{
    return false;
}

static bool skip_tag_effect_top_up()
{
    return false;
}

static bool skip_tag_effect_speed()
{
    return false;
}

static bool skip_tag_effect_economy()
{
    int added_money = 0;
    if (g_game_vars.money > 0)
        added_money = min(g_game_vars.money, 40);

    g_game_vars.money += added_money;
    display_money();

    return true;
}
