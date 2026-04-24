/**
 * @file save.h
 *
 * @brief Utils functions to save/load data structures from/to the SRAM.
 */
#ifndef SAVE_H
#define SAVE_H

#include <tonc.h>
#include "game_variables.h"

void clear_sram(void);

void save_options(GameVariables* vars_ptr);
void load_options(GameVariables* vars_ptr);

#endif // SAVE_H
