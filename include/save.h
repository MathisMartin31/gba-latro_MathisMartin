/**
 * @file save.h
 *
 * @brief Utils functions to save/load data structures from/to the SRAM.
 */
#ifndef SAVE_H
#define SAVE_H

#include "game_variables.h"

#include <tonc.h>

/**
 * @brief Save game data to SRAM.
 */
void save_game();

/**
 * @brief Load game data from SRAM.
 *
 * @sa save_game
 */
void load_game();

#endif // SAVE_H
