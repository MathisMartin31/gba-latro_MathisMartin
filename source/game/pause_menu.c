#include "game/pause_menu.h"

#include "background_pause_menu_gfx.h"
#include "button.h"
#include "game.h"
#include "game/common_ui.h"
#include "game_variables.h"
#include "graphic_utils.h"
#include "selection_grid.h"
#include "util.h"

#include <stdint.h>
#include <string.h>
#include <tonc.h>

enum PauseTabs
{
    PEEK_DECK_TAB,
    HAND_LEVELS_TAB
};

enum PauseButtons
{
    DECK_BTN_IDX,
    HANDS_BTN_IDX,
    BACK_BTN_IDX
};

// Color palette indices

// button colors
#define MENU_BTN_MAIN_COLOR_PAL_IDX     1
#define DECK_BTN_OUTLINE_COLOR_PAL_IDX  2
#define HANDS_BTN_OUTLINE_COLOR_PAL_IDX 3
#define BACK_BTN_MAIN_COLOR_PAL_IDX     4
#define BACK_BTN_OUTLINE_COLOR_PAL_IDX  5

// Cards normal colors
#define SPADES_LOGO_MAIN_COLOR         16
#define HEARTS_LOGO_MAIN_COLOR         17
#define CLUBS_LOGO_MAIN_COLOR          18
#define DIAMONDS_LOGO_MAIN_COLOR       19
#define SPADES_LOGO_BACKGROUND_COLOR   20
#define HEARTS_LOGO_BACKGROUND_COLOR   21
#define CLUBS_LOGO_BACKGROUND_COLOR    22
#define DIAMONDS_LOGO_BACKGROUND_COLOR 23
// all high contrast colors are offset by 8 in the fixed Aseprite palette
#define CARDS_HIGH_CONTRAST_PAL_OFFSET 8

static enum PauseTabs curr_tab = PEEK_DECK_TAB;
static bool hand_levels_are_secret = false;

static void construct_peek_deck_background(void)
{

}

static void construct_hand_levels_background(void)
{

}

void game_pause_menu_change_background(void)
{

}

void game_pause_menu_on_init(void)
{

}

void game_pause_menu_on_update(void)
{

}

void game_pause_menu_on_exit(void)
{

}
