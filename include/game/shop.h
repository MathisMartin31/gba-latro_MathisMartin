/**
 * @file shop.h
 *
 * @brief Shop state functions.
 */
#ifndef GAME_SHOP_H
#define GAME_SHOP_H

#include "joker.h"

#include <stdbool.h>

/**
 * @brief Initialize the shop for a run.
 * Resets all the shop data for the run, needs to be called once per run.
 */
void game_shop_reset(void);

int game_shop_get_reroll_cost(void);
void game_shop_set_reroll_cost(int cost);

/**
 * @brief Get a pointer to the Card we are currently showing the description of.
 *
 * @return JokerObject*
 */
JokerObject* game_shop_get_description_card(void);

/**
 * @brief Set whether a Joker can appear in the shop.
 *
 * @param avail - true to make it available to the shop to appear in
 *                false to make it unavailable.
 */
void game_shop_set_joker_avail(int joker_id, bool avail);

/**
 * @brief Rolls a random Joker ID among the available ones
 */
int game_shop_get_rand_available_joker_id(void);

/**
 * @brief Rolls a random Joker ID among the available ones with a given rarity
 *
 * @param joker_rarity
 */
int game_shop_get_rand_available_joker_id_with_rarity(int joker_rarity);

/**
 * @brief Change to the shop background
 */
void game_shop_change_background(void);

/**
 * @brief Shop state initialization
 */
void game_shop_on_init(void);

/**
 * @brief Shop state update
 */
void game_shop_on_update(void);

/**
 * @brief Shop cleanup
 */
void game_shop_on_exit(void);

#endif // GAME_SHOP_H