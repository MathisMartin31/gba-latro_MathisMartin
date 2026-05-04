/**
 * @file game_state_machine.h
 *
 * @brief Game global state machine structs definitions
 */
#ifndef GAME_STATE_MACHINE_H
#define GAME_STATE_MACHINE_H

#include <tonc.h>

typedef void (*GameStateCallback)(void);
typedef void (*SubStateActionFn)(void);

typedef struct
{
    int substate;
    GameStateCallback on_init;
    GameStateCallback on_update;
    GameStateCallback on_exit;
} StateInfo;

// Enum value names in ../include/def_state_info_table.h
enum GameState
{
#define DEF_STATE_INFO(stateEnum, on_init, on_update, on_exit) stateEnum,
#include "def_state_info_table.h"
#undef DEF_STATE_INFO
    GAME_STATE_MAX,
    GAME_STATE_UNDEFINED
};

extern StateInfo state_info[];

// The current game state, this is used to determine what the game is doing at any given time
extern enum GameState game_state;
extern enum HandState hand_state;
extern enum PlayState play_state;

#endif // GAME_VARIABLES_H