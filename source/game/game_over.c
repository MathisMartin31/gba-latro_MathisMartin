#include "game/main_menu.h"

#include "affine_background.h"
#include "audio_utils.h"
#include "button.h"
#include "game.h"
#include "graphic_utils.h"
#include "layout.h"
#include "soundbank.h"
#include "util.h"

#include <tonc.h>

#define GAME_OVER_ANIM_FRAMES 15

// clang-format off
static const BG_POINT GAME_OVER_SRC_RECT_3X3_POS = { 25,  29};
static const Rect     GAME_OVER_DIALOG_DEST_RECT = { 11,  21,  23,  28};
static const Rect     GAME_OVER_ANIM_RECT        = { 11,   8,  23,  28};
static const Rect     GAME_LOSE_MSG_TEXT_RECT    = {104,  72, UNDEFINED, UNDEFINED};
// 1 character to the right of GAME_LOSE
static const Rect     GAME_WIN_MSG_TEXT_RECT     = {112,  72, UNDEFINED, UNDEFINED};
static const BG_POINT NEW_RUN_BTN_DEST_POS       = { 15,  26};
static const Rect     NEW_RUN_BTN_SRC_RECT       = {  0,  30,   4,  31};
// clang-format on

static void game_over_init(void)
{
    // Clears the round end menu
    main_bg_se_clear_rect(POP_MENU_ANIM_RECT);
    main_bg_se_copy_expand_3x3_rect(GAME_OVER_DIALOG_DEST_RECT, GAME_OVER_SRC_RECT_3X3_POS);
    main_bg_se_copy_rect(NEW_RUN_BTN_SRC_RECT, NEW_RUN_BTN_DEST_POS);
}

void game_lose_on_init(void)
{
    game_over_init();
    // Using the text color to match the "Game Over" text
    affine_background_set_color(TEXT_CLR_RED);
}

void game_win_on_init(void)
{
    game_over_init();
    // Using the text color to match the "You Win" text
    affine_background_set_color(TEXT_CLR_BLUE);
}

/**
 * @brief Makes the message background frame move 1 tile up, called until it is in its place.
 */
static inline void game_over_anim_frame(void)
{
    main_bg_se_move_rect_1_tile_vert(GAME_OVER_ANIM_RECT, SCREEN_UP);
}

/**
 * @brief Polls user input to detect "Retry" button press.
 */
static inline void game_over_process_user_input()
{
    if (key_hit(SELECT_CARD))
    {
        play_sfx(SFX_BUTTON, MM_BASE_PITCH_RATE, BUTTON_SFX_VOLUME);
        game_change_state(GAME_STATE_BLIND_SELECT);
    }
}

void game_lose_on_update(void)
{
    if (g_game_vars.timer < GAME_OVER_ANIM_FRAMES)
    {
        game_over_anim_frame();
    }
    else if (g_game_vars.timer == GAME_OVER_ANIM_FRAMES)
    {
        tte_printf(
            "#{P:%d,%d; cx:0x%X000}GAME OVER",
            GAME_LOSE_MSG_TEXT_RECT.left,
            GAME_LOSE_MSG_TEXT_RECT.top,
            TTE_RED_PB
        );
    }

    game_over_process_user_input();
}

void game_win_on_update(void)
{
    if (g_game_vars.timer < GAME_OVER_ANIM_FRAMES)
    {
        game_over_anim_frame();
    }
    else if (g_game_vars.timer == GAME_OVER_ANIM_FRAMES)
    {
        tte_printf(
            "#{P:%d,%d; cx:0x%X000}YOU WIN",
            GAME_WIN_MSG_TEXT_RECT.left,
            GAME_WIN_MSG_TEXT_RECT.top,
            TTE_BLUE_PB
        );
    }

    game_over_process_user_input();
}

void game_over_on_exit(void)
{
    game_clear();
}
