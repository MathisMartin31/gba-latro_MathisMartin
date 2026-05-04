#include "game/joker_row.h"

#include "game.h"
#include "game_variables.h"
#include "joker.h"
#include "layout.h"
#include "list.h"
#include "util.h"

int jokers_sel_row_get_size(void)
{
    return list_get_len(get_jokers_list());
}

static Rect get_text_rect_under_sprite_object(SpriteObject* sprite_object)
{
    int height = 0;
    int width = 0;

    if (sprite_object_get_dimensions(sprite_object, &width, &height) == false)
    {
        // fallback
        height = CARD_SPRITE_SIZE;
        width = CARD_SPRITE_SIZE;
    }

    Rect ret_rect = {0};

    ret_rect.left = fx2int(sprite_object->tx);
    ret_rect.top = fx2int(sprite_object->ty) + height + TILE_SIZE;
    ret_rect.right = ret_rect.left + width;
    ret_rect.bottom = ret_rect.top + TTE_CHAR_SIZE;

    return ret_rect;
}

void erase_price_under_sprite_object(SpriteObject* sprite_object)
{
    Rect price_rect = get_text_rect_under_sprite_object(sprite_object);

    // Add SPRITE_FOCUS_RAISE_PX to cover the focused case
    price_rect.bottom = price_rect.bottom + SPRITE_FOCUS_RAISE_PX;

    tte_erase_rect_wrapper(price_rect);
}

void print_price_under_sprite_object(SpriteObject* sprite_object, int price)
{
    Rect price_rect = get_text_rect_under_sprite_object(sprite_object);

    char price_str_buff[INT_MAX_DIGITS + 2]; // + 2 for null-terminator and "$"

    snprintf(price_str_buff, sizeof(price_str_buff), "$%d", price);

    update_text_rect_to_center_str(&price_rect, price_str_buff, SCREEN_LEFT);

    tte_printf("#{P:%d,%d; cx:0x%X000}$%d", price_rect.left, price_rect.top, TTE_YELLOW_PB, price);
}

bool jokers_sel_row_on_selection_changed(
    SelectionGrid* selection_grid,
    int row_idx,
    const Selection* prev_selection,
    const Selection* new_selection
)
{
    List* owned_jokers_list = get_jokers_list();

    // swap Jokers if the A button is held down and all Jokers are on the same row
    bool swapping =
        key_is_down(SELECT_CARD) && new_selection->y == row_idx && prev_selection->y == row_idx;

    if (prev_selection->y == row_idx)
    {
        JokerObject* joker_object =
            (JokerObject*)list_get_at_idx(owned_jokers_list, prev_selection->x);
        // Don't change focus from current Joker if swapping
        if (joker_object != NULL && !swapping)
        {
            erase_price_under_sprite_object(joker_object->sprite_object);
            sprite_object_set_focus(joker_object->sprite_object, false);
        }
    }

    if (new_selection->y == row_idx)
    {
        JokerObject* joker_object =
            (JokerObject*)list_get_at_idx(owned_jokers_list, new_selection->x);
        if (joker_object != NULL)
        {
            if (!swapping)
            {
                sprite_object_set_focus(joker_object->sprite_object, true);
            }
            // If we land on this row while the A button is being held, we are in swapping mode
            // This means that we need to hide the price, whether we were already
            // on this row or if we come from another
            if (!key_is_down(SELECT_CARD))
            {
                print_price_under_sprite_object(
                    joker_object->sprite_object,
                    joker_get_sell_value(joker_object->joker)
                );
            }
        }
    }

    if (swapping)
    {
        list_swap(
            owned_jokers_list,
            (unsigned int)prev_selection->x,
            (unsigned int)new_selection->x
        );
    }

    return true;
}

static inline void joker_start_discard_animation(JokerObject* joker_object)
{
    joker_object->sprite_object->tx = int2fx(JOKER_DISCARD_TARGET.x);
    joker_object->sprite_object->ty = int2fx(JOKER_DISCARD_TARGET.y);
    list_push_back(get_discarded_jokers_list(), joker_object);
}

static inline void game_sell_joker(int joker_idx)
{
    List* owned_jokers_list = get_jokers_list();

    if (joker_idx < 0 || joker_idx >= list_get_len(owned_jokers_list))
        return;

    JokerObject* joker_object = (JokerObject*)list_get_at_idx(owned_jokers_list, joker_idx);
    g_game_vars.money += joker_get_sell_value(joker_object->joker);
    display_money();
    erase_price_under_sprite_object(joker_object->sprite_object);

    remove_owned_joker(joker_idx);

    joker_start_discard_animation(joker_object);
}

void jokers_sel_row_on_key_transit(SelectionGrid* selection_grid, Selection* selection)
{
    JokerObject* joker_object = (JokerObject*)list_get_at_idx(get_jokers_list(), selection->x);
    if (joker_object != NULL)
    {
        if (key_hit(SELECT_CARD))
        {
            erase_price_under_sprite_object(joker_object->sprite_object);
        }
        else if (key_released(SELECT_CARD))
        {
            print_price_under_sprite_object(
                joker_object->sprite_object,
                joker_get_sell_value(joker_object->joker)
            );
        }
    }

    if (key_hit(SELL_KEY))
    {
        int sold_joker_idx = selection->x;

        // Move the selection away from the jokers so it doesn't point to an invalid place
        // Do this before selling the joker so valid row sizes are used
        selection_grid_move_selection_vert(selection_grid, SCREEN_DOWN);

        game_sell_joker(sold_joker_idx);
    }
}