#include "game/options_menu.h"

#include "affine_background.h"
#include "audio_utils.h"
#include "background_options_menu_gfx.h"
#include "button.h"
#include "game.h"
#include "game/common_ui.h"
#include "game_variables.h"
#include "graphic_utils.h"
#include "soundbank.h"

#include <stdint.h>
#include <tonc.h>
#include <tonc_math.h>
#include <tonc_memdef.h>

#define GAME_SPEED_MIN 1
#define GAME_SPEED_MAX 4

// Volume is store from 0 to 20 but is an increment of 5 so 0 to 100 will be displayed
#define MUSIC_VOLUME_MIN 0
#define MUSIC_VOLUME_MAX 20
#define SOUND_VOLUME_MIN 0
#define SOUND_VOLUME_MAX 20

// Button indices
enum OptionButtons
{
    GAME_SPEED_BTN_IDX,
    HIGH_CONTRAST_BTN_IDX,
    MUSIC_VOLUME_BTN_IDX,
    SOUND_VOLUME_BTN_IDX,
    BACK_BTN_IDX,
    NB_OPTIONS_BUTTONS
};

// Color palette indices
#define MENU_BUTTON_MAIN_COLOR_PAL_IDX           1
#define BACK_BUTTON_MAIN_COLOR_PAL_IDX           3
#define SPEED_LESS_BUTTON_OUTLINE_COLOR_PAL_IDX  4
#define SPEED_BUTTON_OUTLINE_COLOR_PAL_IDX       5
#define SPEED_MORE_BUTTON_OUTLINE_COLOR_PAL_IDX  6
#define CONTRAST_BUTTON_OUTLINE_COLOR_PAL_IDX    7
#define MUSIC_BUTTON_OUTLINE_COLOR_PAL_IDX       8
#define SOUND_BUTTON_OUTLINE_COLOR_PAL_IDX       9
#define BACK_BUTTON_OUTLINE_COLOR_PAL_IDX       10

// Positions/Rects used to construct and update the menu
// clang-format off

// Values in tiles
//static const Rect     OPTIONS_BACKGROUND_DEST_RECT             = { 1,  0, 28, 19};
//static const BG_POINT OPTIONS_BACKGROUND_SRC_RECT_3X3_POS      = { 0, 20};

//static const Rect     OPTIONS_SPEED_BTN_SRC_RECT               = { 4, 20,  9, 21};
//static const BG_POINT OPTIONS_SPEED_BTN_DEST_POS               = {12,  3};
//static const Rect     OPTIONS_SPEED_LESS_ACTIVE_BTN_SRC_RECT   = { 3, 20,  3, 21};
//static const Rect     OPTIONS_SPEED_LESS_DISABLED_BTN_SRC_RECT = { 9, 22,  9, 23};
//static const BG_POINT OPTIONS_SPEED_LESS_BTN_DEST_POS          = {11,  3};
//static const Rect     OPTIONS_SPEED_MORE_ACTIVE_BTN_SRC_RECT   = {10, 20, 10, 21};
//static const Rect     OPTIONS_SPEED_MORE_DISABLED_BTN_SRC_RECT = {10, 22, 10, 23};
//static const BG_POINT OPTIONS_SPEED_MORE_BTN_DEST_POS          = {18,  3};
//static const Rect     OPTIONS_SPEED_VALUE_1_SRC_RECT           = { 6, 20,  7, 21};
//static const Rect     OPTIONS_SPEED_VALUE_2_SRC_RECT           = { 3, 22,  4, 23};
//static const Rect     OPTIONS_SPEED_VALUE_3_SRC_RECT           = { 5, 22,  6, 23};
//static const Rect     OPTIONS_SPEED_VALUE_4_SRC_RECT           = { 7, 22,  8, 23};
//static const BG_POINT OPTIONS_SPEED_VALUE_DEST_POS             = {14,  3};

//static const Rect     OPTIONS_CONTRAST_BTN_SRC_RECT            = { 3, 24,  8, 25};
//static const BG_POINT OPTIONS_CONTRAST_BTN_DEST_POS            = {12,  7};
//static const Rect     OPTIONS_CONTRAST_VALUE_YES_SRC_RECT      = { 3, 24,  8, 25};
//static const Rect     OPTIONS_CONTRAST_VALUE_NO_SRC_RECT       = { 3, 24,  8, 25};
//static const BG_POINT OPTIONS_CONTRAST_VALUE_DEST_POS          = {13,  7};

//static const Rect     OPTIONS_BACK_BTN_DEST_RECT               = { 2, 16, 27, 18};
//static const BG_POINT OPTIONS_BACK_BTN_SRC_RECT_3X3_POS        = { 0, 23};

// Values in pixels
static const BG_POINT OPTIONS_GAME_SPEED_TEXT_POS    = { 82,  16};
static const BG_POINT OPTIONS_HIGH_CONTRAST_TEXT_POS = { 40,  48};
static const BG_POINT OPTIONS_MUSIC_VOLUME_TEXT_POS  = { 56,  80};
static const BG_POINT OPTIONS_SOUND_VOLUME_TEXT_POS  = { 56, 104};
static const BG_POINT OPTIONS_BACK_TEXT_POS          = {104, 136};
// clang-format on

// Current selected button index
static enum OptionButtons selection_y = GAME_SPEED_BTN_IDX;

static void disable_all_outlines_except_self(enum OptionButtons highlighted_btn)
{
    // These two get disabled no matter what
    memcpy16(
        &pal_bg_mem[SPEED_LESS_BUTTON_OUTLINE_COLOR_PAL_IDX],
        &pal_bg_mem[MENU_BUTTON_MAIN_COLOR_PAL_IDX],
        1
    );
    memcpy16(
        &pal_bg_mem[SPEED_MORE_BUTTON_OUTLINE_COLOR_PAL_IDX],
        &pal_bg_mem[MENU_BUTTON_MAIN_COLOR_PAL_IDX],
        1
    );

    if (highlighted_btn != GAME_SPEED_BTN_IDX) {
        memcpy16(
            &pal_bg_mem[SPEED_BUTTON_OUTLINE_COLOR_PAL_IDX],
            &pal_bg_mem[MENU_BUTTON_MAIN_COLOR_PAL_IDX],
            1
        );
    }
    if (highlighted_btn != HIGH_CONTRAST_BTN_IDX) {
        memcpy16(
            &pal_bg_mem[CONTRAST_BUTTON_OUTLINE_COLOR_PAL_IDX],
            &pal_bg_mem[MENU_BUTTON_MAIN_COLOR_PAL_IDX],
            1
        );
    }
    if (highlighted_btn != MUSIC_VOLUME_BTN_IDX) {
        memcpy16(
            &pal_bg_mem[MUSIC_BUTTON_OUTLINE_COLOR_PAL_IDX],
            &pal_bg_mem[MENU_BUTTON_MAIN_COLOR_PAL_IDX],
            1
        );
    }
    if (highlighted_btn != SOUND_VOLUME_BTN_IDX) {
        memcpy16(
            &pal_bg_mem[SOUND_BUTTON_OUTLINE_COLOR_PAL_IDX],
            &pal_bg_mem[MENU_BUTTON_MAIN_COLOR_PAL_IDX],
            1
        );
    }
    if (highlighted_btn != BACK_BTN_IDX) {
        memcpy16(
            &pal_bg_mem[BACK_BUTTON_OUTLINE_COLOR_PAL_IDX],
            &pal_bg_mem[BACK_BUTTON_MAIN_COLOR_PAL_IDX],
            1
        );
    } 

    u16 color = 0x213f;

    memcpy16(
        &pal_bg_mem[MENU_BUTTON_MAIN_COLOR_PAL_IDX],
        &color,
        1
    );
}

void game_options_menu_change_background(void)
{
    toggle_windows(false, false);
    tte_erase_screen();

    GRIT_CPY(pal_bg_mem, background_options_menu_gfxPal);
    GRIT_CPY(&tile_mem[MAIN_BG_CBB], background_options_menu_gfxTiles);
    GRIT_CPY(&se_mem[MAIN_BG_SBB], background_options_menu_gfxMap);

    tte_printf(
        "#{P:%d,%d; cx:0x%X000}Game Speed",
        OPTIONS_GAME_SPEED_TEXT_POS.x,
        OPTIONS_GAME_SPEED_TEXT_POS.y,
        TTE_WHITE_PB
    );
    tte_printf(
        "#{P:%d,%d; cx:0x%X000}High Contrast Cards",
        OPTIONS_HIGH_CONTRAST_TEXT_POS.x,
        OPTIONS_HIGH_CONTRAST_TEXT_POS.y,
        TTE_WHITE_PB
    );
    tte_printf(
        "#{P:%d,%d; cx:0x%X000}Music Volume 999",
        OPTIONS_MUSIC_VOLUME_TEXT_POS.x,
        OPTIONS_MUSIC_VOLUME_TEXT_POS.y,
        TTE_WHITE_PB
    );
    tte_printf(
        "#{P:%d,%d; cx:0x%X000}Sound Volume 999",
        OPTIONS_SOUND_VOLUME_TEXT_POS.x,
        OPTIONS_SOUND_VOLUME_TEXT_POS.y,
        TTE_WHITE_PB
    );
    tte_printf(
        "#{P:%d,%d; cx:0x%X000}Back",
        OPTIONS_BACK_TEXT_POS.x,
        OPTIONS_BACK_TEXT_POS.y,
        TTE_WHITE_PB
    );

    // Disable the button highlight colors
    disable_all_outlines_except_self(NB_OPTIONS_BUTTONS);
}

void game_options_menu_on_init()
{

}

void game_options_menu_on_update()
{
    if (key_hit(KEY_UP))
    {
        if (selection_y > 0)
        {
            selection_y--;
        }
    }
    else if (key_hit(KEY_DOWN))
    {
        if (selection_y < NB_OPTIONS_BUTTONS - 1)
        {
            selection_y++;
        }
    }

    disable_all_outlines_except_self(selection_y);

    switch (selection_y)
    {
        // Increase/decrease value with left/right arrow keys if possible
        case GAME_SPEED_BTN_IDX:
        {
            // Highlight button
            memset16(
                &pal_bg_mem[SPEED_BUTTON_OUTLINE_COLOR_PAL_IDX],
                BTN_HIGHLIGHT_COLOR,
                1
            );
            if (key_hit(KEY_LEFT))
            {
                // TODO : decrease value in GameVariables if bigger than 1
                // TODO : disable left arrow if we arrive at value of 1
            }
            else if (key_hit(KEY_RIGHT))
            {
                // TODO : increase value in GameVariables if smaller than 4
                // TODO : disable right arrow if we arrive at value of 4
            }
            break;
        }

        // Toggle value by pressing the A button
        case HIGH_CONTRAST_BTN_IDX:
        {
            // Highlight button
            memset16(
                &pal_bg_mem[CONTRAST_BUTTON_OUTLINE_COLOR_PAL_IDX],
                BTN_HIGHLIGHT_COLOR,
                1
            );
            if (key_hit(SELECT_CARD))
            {
                // TODO : invert value in GameVariables
                // TODO : copy Yes/No tiles when needed
            }
            break;
        }

        // Sliders : decrease/increase value by pressing left/right until bar is empty/full
        case MUSIC_VOLUME_BTN_IDX:
        {
            // Highlight button
            memset16(
                &pal_bg_mem[MUSIC_BUTTON_OUTLINE_COLOR_PAL_IDX],
                BTN_HIGHLIGHT_COLOR,
                1
            );
            if (key_hit(KEY_LEFT))
            {
                // TODO : decrease value in GameVariables if bigger than 0
            }
            else if (key_hit(KEY_RIGHT))
            {
                // TODO : increase value in GameVariables if smaller than 20
            }
            break;
        }
        case SOUND_VOLUME_BTN_IDX:
        {
            // Highlight button
            memset16(
                &pal_bg_mem[SOUND_BUTTON_OUTLINE_COLOR_PAL_IDX],
                BTN_HIGHLIGHT_COLOR,
                1
            );
            if (key_hit(KEY_LEFT))
            {
                // TODO : decrease value in GameVariables if bigger than 0
            }
            else if (key_hit(KEY_RIGHT))
            {
                // TODO : increase value in GameVariables if smaller than 20
            }
            break;
        }

        case BACK_BTN_IDX:
        {
            // Highlight button
            memset16(
                &pal_bg_mem[BACK_BUTTON_OUTLINE_COLOR_PAL_IDX],
                BTN_HIGHLIGHT_COLOR,
                1
            );
            if (key_hit(SELECT_CARD))
            {
                // TODO : save GameVariables struct in memory
                change_background(BG_MAIN_MENU);
                game_change_state(GAME_STATE_MAIN_MENU);
            }
            break;
        }

        // Should not happen
        default:
            break;
    }
}

void game_options_menu_on_exit()
{

}
