#include "game/common_ui.h"

#include "blind_select.h"
#include "game.h"
#include "game/main_menu.h"
#include "game/options_menu.h"
#include "game/round.h"
#include "game/round_end.h"
#include "game/run_setup.h"
#include "game/shop.h"
#include "layout.h"

// Sprite Layers

/**
 * @brief Tile index LUT for all sprite types. Filled at init by calling `common_ui_init`
 *
 * @sa common_ui_init
 */
static int s_sprite_tids[MAX_SPRITE_TYPE] = {0};

/**
 * @brief Starting layer LUT for all sprite types. Filled at init by calling `common_ui_init`
 *
 * @sa common_ui_init
 */
static int s_sprite_starting_layers[MAX_SPRITE_TYPE] = {0};

// clang-format off
static const int s_sprite_counts[MAX_SPRITE_TYPE] = {
    [CARD_SPRITE]            = MAX_HAND_SIZE,
    [CARD_PLAYED_SPRITE]     = MAX_SELECTION_SIZE,
    [CARD_UNDISCARD_SPRITE]  = 1,
    [BLIND_TOKEN_SPRITE]     = MAX_BLIND_TOKEN,
    [SKIP_TAG_SPRITE]        = MAX_SKIP_TAGS,
    [JOKER_SPRITE]           = MAX_ACTIVE_JOKERS,
    [DECK_SPRITE]            = 1
};
static const int s_sprite_sizes[MAX_SPRITE_TYPE] = {
    [CARD_SPRITE]            = CARD_SPRITE_SIZE,
    [CARD_PLAYED_SPRITE]     = CARD_SPRITE_SIZE,
    [CARD_UNDISCARD_SPRITE]  = CARD_SPRITE_SIZE,
    [BLIND_TOKEN_SPRITE]     = BLIND_SPRITE_SIZE,
    [SKIP_TAG_SPRITE]        = SKIP_TAG_SPRITE_SIZE,
    [JOKER_SPRITE]           = JOKER_SPRITE_SIZE,
    [DECK_SPRITE]            = CARD_SPRITE_SIZE
};
// clang-format on

void common_ui_init(void)
{
    // Start at CARD_PLAYED_SPRITE, since CARD_SPRITE is first and both the TID
    // and starting layer are 0
    for (enum SpriteType sprite_type = 1; sprite_type < MAX_SPRITE_TYPE; sprite_type++)
    {
        s_sprite_tids[sprite_type] =
            s_sprite_tids[sprite_type - 1] +
            s_sprite_counts[sprite_type - 1] * s_sprite_sizes[sprite_type - 1];
        s_sprite_starting_layers[sprite_type] =
            s_sprite_starting_layers[sprite_type - 1] + s_sprite_counts[sprite_type - 1];
    }
}

int get_sprite_tid(enum SpriteType sprite_type, int layer)
{
    return s_sprite_tids[sprite_type] + layer * s_sprite_sizes[sprite_type];
}

int get_sprite_starting_layer(enum SpriteType sprite_type)
{
    return s_sprite_starting_layers[sprite_type];
}

// Backgrounds

typedef void (*BackgroundRenderCallback)(void);

static enum BackgroundId s_background = BG_NONE;

// Map to fill in for refactor
static const BackgroundRenderCallback bgCallbacks[] = {
    [BG_NONE] = NULL,
    [BG_CARD_SELECTING] = game_round_change_background_selecting,
    [BG_CARD_PLAYING] = game_round_change_background_playing,
    [BG_ROUND_END] = game_round_end_change_background,
    [BG_SHOP] = game_shop_change_background,
    [BG_BLIND_SELECT] = game_blind_select_change_background,
    [BG_RUN_SETUP] = game_run_setup_change_background,
    [BG_OPTIONS_MENU] = game_options_menu_change_background,
    [BG_MAIN_MENU] = game_main_menu_change_background,
};

enum BackgroundId get_current_background(void)
{
    return s_background;
}

void change_background(enum BackgroundId id, bool force_redraw)
{
    if (force_redraw)
    {
        s_background = BG_NONE;
    }
    if (id != s_background && bgCallbacks[id] != NULL)
    {
        bgCallbacks[id]();
    }
    s_background = id;
}

void reset_top_left_panel_bottom_row(void)
{
    BG_POINT top_left_panel_bottom_row_pos = TOP_LEFT_PANEL_POINT;
    // Use the source rect height to offset to the bottom row point
    top_left_panel_bottom_row_pos.y += rect_height(&TOP_LEFT_ITEM_SRC_RECT) - 1;
    main_bg_se_copy_rect(TOP_LEFT_PANEL_BOTTOM_ROW_RESET_RECT, top_left_panel_bottom_row_pos);
}
