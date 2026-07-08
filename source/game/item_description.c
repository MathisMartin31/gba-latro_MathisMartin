#include "game/item_description.h"

#include "game.h"
#include "game/round.h"
#include "game/shop.h"
#include "layout.h"
#include "skip_tag.h"
#include "string.h"
#include "timer.h"
#include "util.h"

#define TM_SHOW_ITEM_DESC_WAIT 12
#define TM_HIDE_DECK_WAIT      5

// clang-format off
// Positions in tiles
static const BG_POINT OWNED_ITEMS_PANEL_3X3_SRC_POS = { 29, 22};
static const Rect     OWNED_JOKERS_PANEL_RECT       = {  9,  1, 21,  5};
static const Rect     OWNED_CONSUMABLES_PANEL_RECT  = { 23,  1, 28,  5};
static const Rect     OWNED_ITEMS_PANEL_RECT        = {  9,  1, 28,  5};
static const Rect     OWNED_ITEMS_PANEL_ANIM_CLEAR  = {  9,  0, 28,  1};
static const Rect     ITEM_DESC_9_PTCH_TO_RECT      = {  9,  6, 28, 18};
static const NinePatchRect ITEM_DESC_9_PTCH_SRC = {
                                        .patch_rect = { 27, 25, 31, 31},
                                        .margins    = {  2,  3,  2,  3}
};
static const int      ITEM_DESC_MAX_TEXT_HEIGHT     = ITEM_DESC_9_PTCH_TO_RECT.bottom -
                                                      ITEM_DESC_9_PTCH_TO_RECT.top + 1 -
                                                      ITEM_DESC_9_PTCH_SRC.margins.top -
                                                      ITEM_DESC_9_PTCH_SRC.margins.bottom;
static const Rect     ITEM_DESC_TEXT_RECT           = { 11,  9, 26, 18};
static const Rect     ITEM_NAME_TEXT_RECT           = { 10,  7, 27,  7};

// Positions in pixels
static const int      OWNED_ITEMS_HIDE_Y_OFFSET   = 50;
static const int      HAND_CARDS_HIDE_Y           = 155;
static const int      HAND_CARDS_SELECTED_Y       = 75;
static const int      HAND_CARDS_UNSELECTED_Y     = 90;
static const BG_POINT ITEM_DESCRIPTION_SPRITE_POS = {135,   9};
// clang-format on

enum ItemDescriptionStates
{
    ITEM_DESC_STATE_SHOW,
    ITEM_DESC_STATE_HIDE,
    ITEM_DESC_STATE_END,
    ITEM_DESC_STATE_MAX
};

enum ItemDescriptionColors
{
    ITEM_DESC_PAL_IDX_MAIN,
    ITEM_DESC_PAL_IDX_SHADOW,
    ITEM_DESC_PAL_IDX_MAX
};

static void s_item_description_show_on_init(void);
static void s_item_description_show_on_update(void);
static void s_item_description_hide_on_init(void);
static void s_item_description_hide_on_update(void);
static void s_item_description_end_on_init(void);

// clang-format off
static StateInfo s_item_description_state_fn[ITEM_DESC_STATE_MAX] = {
    [ITEM_DESC_STATE_SHOW] = {
        .on_init   = s_item_description_show_on_init,
        .on_update = s_item_description_show_on_update,
        .on_exit   = noop
    },
    [ITEM_DESC_STATE_HIDE] = {
        .on_init   = s_item_description_hide_on_init,
        .on_update = s_item_description_hide_on_update,
        .on_exit   = noop
    },
    [ITEM_DESC_STATE_END] = {
        .on_init   = s_item_description_end_on_init,
        .on_update = noop,
        .on_exit   = noop
    }
};
// clang-format on

static StateMachine s_desc_sm = STATE_MACHINE_DEFINE(s_item_description_state_fn, ITEM_DESC_STATE_MAX);

static bool s_is_desc_shown = false;
static enum GameState s_desc_menu = GAME_STATE_UNDEFINED;
static s32 s_timer = TM_ZERO;

static int s_menu_colors[ITEM_DESC_PAL_IDX_MAX] = {0};

static Item* s_description_item = NULL;
static FIXED s_description_item_original_x_pos = UNDEFINED;
static FIXED s_description_item_original_y_pos = UNDEFINED;
static List* s_description_item_original_list = NULL;

static s32 s_show_description_anim_progress = 0;

void item_description_set_target(Item* desc_item, List* item_list)
{
    GBAL_RETURN_IF_NULL_VOID(desc_item);
    GBAL_RETURN_IF_NULL_VOID(item_list);

    s_desc_menu = game_get_state();
    s_is_desc_shown = true;

    s_description_item = desc_item;
    s_description_item_original_x_pos = desc_item->tx;
    s_description_item_original_y_pos = desc_item->ty;
    s_description_item_original_list = item_list;

    switch (s_desc_menu)
    {
        case GAME_STATE_SHOP:
            s_menu_colors[ITEM_DESC_PAL_IDX_MAIN] = (int)SHOP_DESC_RARITY_MAIN_COLOR_PAL_IDX;
            s_menu_colors[ITEM_DESC_PAL_IDX_SHADOW] = (int)SHOP_DESC_RARITY_SHADOW_COLOR_PAL_IDX;

            break;

        case GAME_STATE_ROUND:
            s_menu_colors[ITEM_DESC_PAL_IDX_MAIN] = (int)ROUND_DESC_RARITY_MAIN_COLOR_PAL_IDX;
            s_menu_colors[ITEM_DESC_PAL_IDX_SHADOW] = (int)ROUND_DESC_RARITY_SHADOW_COLOR_PAL_IDX;
            break;

        default:
            break;
    }

    s_timer = TM_ZERO;
    state_machine_register(&s_desc_sm);
    state_machine_change_state(&s_desc_sm, ITEM_DESC_STATE_SHOW);
}

bool item_description_is_shown(void)
{
    return s_is_desc_shown;
}

Item* item_description_get_target(void)
{
    return s_description_item;
}

static void s_item_description_show_on_init(void)
{
    // This starts at 0, then gets incremented up to TM_SHOW_ITEM_DESC_WAIT. Will be used to
    // revert the animation if the B button is released midway through it
    s_show_description_anim_progress = 0;

    // Erase shop/round text
    tte_erase_rect_wrapper(PLAYING_SCREEN_RECT);

    // If shown during the Round, disable the window in the middle of the screen
    if (s_desc_menu == GAME_STATE_ROUND)
        toggle_windows(false, true);

    // Move all other Sprites offscreen

    Item* tmp_item = NULL;

    // Owned Jokers
    ListItr itr = list_itr_create(get_jokers_list());
    while ((tmp_item = (Item*)list_itr_next(&itr)))
    {
        if (tmp_item != s_description_item)
            tmp_item->ty -= int2fx(OWNED_ITEMS_HIDE_Y_OFFSET);
    }

    switch (s_desc_menu)
    {
        // Cards in Hand
        case GAME_STATE_ROUND:
            CardObject** hand = get_hand_array();
            int nb_cards = get_hand_top() + 1;
            for (int i = 0; i < nb_cards; i++)
            {
                ((SpriteObject*)hand[i])->ty = int2fx(HAND_CARDS_HIDE_Y);
            }
            break;
        
        // Shop Jokers
        case GAME_STATE_SHOP:
            itr = list_itr_create(game_shop_get_items());
            while ((tmp_item = (Item*)list_itr_next(&itr)))
            {
                if (tmp_item != s_description_item)
                    tmp_item->ty = int2fx(SHOP_JOKER_SPRITES_INIT_POS.y + TILE_SIZE);
            }
            break;

        default:
            break;
    }

    // Owned SkipTags
    move_owned_skip_tags_offscreen(true);

    // Set description_card new target position

    s_description_item->tx = int2fx(ITEM_DESCRIPTION_SPRITE_POS.x);
    s_description_item->ty = int2fx(ITEM_DESCRIPTION_SPRITE_POS.y);
}

static void s_item_description_show_on_update(void)
{
    s_timer++;

    if (s_timer <= TM_SHOW_ITEM_DESC_WAIT)
    {
        s_show_description_anim_progress++;

        // Hide Deck (last frames only)
        if (TM_SHOW_ITEM_DESC_WAIT - s_timer < TM_HIDE_DECK_WAIT)
            main_bg_se_move_rect_1_tile_vert(DECK_ANIM_RECT, SCREEN_DOWN);
        // Hide shop panel
        main_bg_se_move_rect_1_tile_vert(POP_MENU_ANIM_RECT, SCREEN_DOWN);
        // Hide Owned Cards panels
        main_bg_se_move_rect_1_tile_vert(OWNED_ITEMS_PANEL_RECT, SCREEN_UP);
    }

    // Anim end
    else if (s_timer == TM_SHOW_ITEM_DESC_WAIT + 1)
    {
        // Compute needed space for the description
        int desc_bottom_offset =
            ITEM_DESC_MAX_TEXT_HEIGHT -
            item_print_description(s_description_item, ITEM_DESC_TEXT_RECT);

        // Print Rarity/Type and change color or the panel
        // Do it before drawing the panel so the color is already set
        const char* subtype_str = item_get_subtype_string(s_description_item);
        tte_printf(
            TTE_WHITE_TAG "#{P:%d,%d}%*s%s",
            ITEM_DESC_TEXT_RECT.left * TILE_SIZE,
            (ITEM_DESC_TEXT_RECT.bottom - desc_bottom_offset - 1) * TILE_SIZE,
            (rect_width(&ITEM_DESC_TEXT_RECT) - strlen(subtype_str)) / 2,
            "",
            subtype_str
        );
        u32 item_colors = item_get_subtype_colors(s_description_item);
        pal_bg_mem[s_menu_colors[ITEM_DESC_PAL_IDX_MAIN]] = item_colors & UINT16_MAX;
        pal_bg_mem[s_menu_colors[ITEM_DESC_PAL_IDX_SHADOW]] = (item_colors >> 16) & UINT16_MAX;

        // Draw description panel
        Rect actual_dest_rect = ITEM_DESC_9_PTCH_TO_RECT;
        actual_dest_rect.bottom -= desc_bottom_offset;
        main_bg_se_copy_expand_9_patch(actual_dest_rect, &ITEM_DESC_9_PTCH_SRC);

        // Print joker name
        const char* item_name = item_get_name(s_description_item);
        tte_printf(
            TTE_WHITE_TAG "#{P:%d,%d}%*s%s",
            ITEM_NAME_TEXT_RECT.left * TILE_SIZE,
            ITEM_NAME_TEXT_RECT.top * TILE_SIZE,
            (rect_width(&ITEM_NAME_TEXT_RECT) - strlen(item_name)) / 2,
            "",
            item_name
        );
    }

    // Actively wait for the B button to be released
    if (!key_held(DESELECT_CARDS))
    {
        s_timer = TM_ZERO;
        state_machine_change_state(&s_desc_sm, ITEM_DESC_STATE_HIDE);
    }
}

static void s_item_description_hide_on_init(void)
{
    // Erase shop text and Joker Description frame if we had time to draw them
    if (s_show_description_anim_progress >= TM_SHOW_ITEM_DESC_WAIT)
    {
        main_bg_se_clear_rect(ITEM_DESC_9_PTCH_TO_RECT);
    }
    // Or clear the owned cards' panel that haven't finished moving up
    else
    {
        main_bg_se_clear_rect(OWNED_ITEMS_PANEL_ANIM_CLEAR);
    }

    tte_erase_rect_wrapper(PLAYING_SCREEN_RECT);

    // If shown during the Round, enable the window in the middle of the screen
    if (s_desc_menu == GAME_STATE_ROUND)
        toggle_windows(true, true);

    // Redraw Jokers/Consumables frames
    main_bg_se_copy_expand_3x3_rect(OWNED_JOKERS_PANEL_RECT, OWNED_ITEMS_PANEL_3X3_SRC_POS);
    main_bg_se_copy_expand_3x3_rect(OWNED_CONSUMABLES_PANEL_RECT, OWNED_ITEMS_PANEL_3X3_SRC_POS);

    // Move Sprites back to their positions

    Item* tmp_item = NULL;

    // Owned Jokers
    ListItr itr = list_itr_create(get_jokers_list());
    while ((tmp_item = (Item*)list_itr_next(&itr)))
    {
        if (tmp_item != s_description_item)
            tmp_item->ty = int2fx(HELD_JOKERS_POS.y);
    }

    switch (s_desc_menu)
    {
        // Cards in Hand
        case GAME_STATE_ROUND:
            CardObject** hand = get_hand_array();
            int nb_cards = get_hand_top() + 1;
            for (int i = 0; i < nb_cards; i++)
            {
                ((SpriteObject*)hand[i])->ty = int2fx(
                    card_object_is_selected(hand[i]) ? HAND_CARDS_SELECTED_Y
                                                     : HAND_CARDS_UNSELECTED_Y
                );
            }
            break;
        
        // Shop Jokers
        case GAME_STATE_SHOP:
            itr = list_itr_create(game_shop_get_items());
            while ((tmp_item = (Item*)list_itr_next(&itr)))
            {
                if (tmp_item != s_description_item)
                    tmp_item->ty = int2fx(ITEM_SHOP_Y);
            }
            break;

        default:
            break;
    }

    move_owned_skip_tags_offscreen(false);

    s_description_item->tx = s_description_item_original_x_pos;
    s_description_item->ty = s_description_item_original_y_pos;
}

static void s_item_description_hide_on_update(void)
{
    s_timer++;

    if (s_timer <= s_show_description_anim_progress)
    {
        // Show Deck (last frames only)
        if (s_show_description_anim_progress > TM_HIDE_DECK_WAIT &&
            s_timer < (s_show_description_anim_progress - (TM_HIDE_DECK_WAIT + 1)))
        {
            main_bg_se_move_rect_1_tile_vert(DECK_ANIM_RECT, SCREEN_UP);
        }
        // Show menu panel
        main_bg_se_move_rect_1_tile_vert(POP_MENU_ANIM_RECT, SCREEN_UP);
    }

    // Last anim frame (no need to wait for the Joker to have stopped for this):
    else if (s_timer == s_show_description_anim_progress + 1)
    {
        s_timer = TM_ZERO;
        state_machine_change_state(&s_desc_sm, ITEM_DESC_STATE_END);
    }
}

static void s_item_description_end_on_init(void)
{
    // Need to account for the description_card being selected if it came from the shop.
    if (s_description_item_original_list == game_shop_get_items())
        s_description_item->ty += int2fx(TILE_SIZE);

    if (s_desc_menu == GAME_STATE_SHOP)
    {
        // Print price under shop Jokers
        Item* item = NULL;
        ListItr itr = list_itr_create(game_shop_get_items());
        while ((item = list_itr_next(&itr)))
        {
            item_print_buy_price_under(item);
        }

        if (s_description_item_original_list == game_shop_get_items())
            s_description_item->ty -= int2fx(TILE_SIZE);

        // Print Reroll prince
        tte_printf(
            "#{P:%d,%d; cx:0x%X000}$%d",
            SHOP_REROLL_RECT.left,
            SHOP_REROLL_RECT.top,
            TTE_WHITE_PB,
            game_shop_get_reroll_cost()
        );
    }

    // Print Deck size that was erased
    display_deck_size_max();

    // if we are NOT pressing A, print the price under the description card if it's a card we owned.
    if (!key_held(SELECT_CARD) && s_description_item_original_list == get_jokers_list())
    {
        sprite_object_print_price_under(
            (SpriteObject*)s_description_item,
            item_get_sell_price(s_description_item)
        );
    }

    state_machine_remove(&s_desc_sm);

    s_description_item = NULL;
    s_is_desc_shown = false;
}