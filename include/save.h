/**
 * @file save.h
 *
 * @brief Utils functions to save/load data structures from/to the SRAM.
 */
#ifndef SAVE_H
#define SAVE_H

#include "game_variables.h"

#include <tonc.h>

void save_game();
void load_game();

#endif // SAVE_H
