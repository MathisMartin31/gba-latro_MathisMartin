#ifndef GAME_ITEM_DESCRIPTION_H
#define GAME_ITEM_DESCRIPTION_H

#include "item.h"
#include "list.h"
#include "state_machine.h"

void item_description_set_target(Item* desc_item, List* item_list);

/**
 * @brief Get a pointer to the Item we are currently showing the description of.
 *
 * @return Item*
 */
Item* item_description_get_target(void);

/**
 * @brief Determines if the Item description is currently up.
 *
 * To be used by menus to determine if they need to wait for the description to go away.
 *
 * @return bool
 */
bool item_description_is_shown(void);

#endif // GAME_ITEM_DESCRIPTION_H