/**
 * @file pause_menu.h
 *
 * @brief Pause menu (Hand stats and Peek Deck) state functions.
 */
#ifndef GAME_PAUSE_MENU_H
#define GAME_PAUSE_MENU_H

/**
 * @brief Change the pause menu background
 */
void game_pause_menu_change_background(void);

/**
 * @brief Pause menu state initialization
 */
void game_pause_menu_on_init(void);

/**
 * @brief Pause menu state update
 */
void game_pause_menu_on_update(void);

/**
 * @brief Pause menu cleanup
 */
void game_pause_menu_on_exit(void);

#endif // GAME_PAUSE_MENU_H