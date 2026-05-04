/**
 * @file joker_row.h
 *
 * @brief All the functions use specifically to interact with Jokers.
 *         This includes browsing, swapping, selling, etc.
 */
#ifndef GAME_JOKER_ROW_H
#define GAME_JOKER_ROW_H

#include "selection_grid.h"
#include "sprite.h"

void erase_price_under_sprite_object(SpriteObject* sprite_object);
void print_price_under_sprite_object(SpriteObject* sprite_object, int price);

int jokers_sel_row_get_size(void);
bool jokers_sel_row_on_selection_changed(
    SelectionGrid* selection_grid,
    int row_idx,
    const Selection* prev_selection,
    const Selection* new_selection
);
void jokers_sel_row_on_key_transit(SelectionGrid* selection_grid, Selection* selection);

#endif // GAME_JOKER_SEL_H