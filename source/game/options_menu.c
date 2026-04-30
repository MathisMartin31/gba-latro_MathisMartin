#include "game/options_menu.h"

#include "affine_background.h"
#include "audio_utils.h"
#include "background_options_menu_gfx.h"
#include "button.h"
#include "game.h"
#include "game/common_ui.h"
#include "game_variables.h"
#include "graphic_utils.h"
#include "save.h"
#include "soundbank.h"

#include <stdint.h>
#include <string.h>
#include <tonc.h>
#include <tonc_math.h>
#include <tonc_memdef.h>

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
#define MENU_BUTTON_MAIN_COLOR_PAL_IDX          1
#define BACK_BUTTON_MAIN_COLOR_PAL_IDX          3
#define SPEED_LESS_BUTTON_OUTLINE_COLOR_PAL_IDX 4
#define SPEED_BUTTON_OUTLINE_COLOR_PAL_IDX      5
#define SPEED_MORE_BUTTON_OUTLINE_COLOR_PAL_IDX 6
#define CONTRAST_BUTTON_OUTLINE_COLOR_PAL_IDX   7
#define MUSIC_BUTTON_OUTLINE_COLOR_PAL_IDX      8
#define SOUND_BUTTON_OUTLINE_COLOR_PAL_IDX      9
#define BACK_BUTTON_OUTLINE_COLOR_PAL_IDX       10

// Positions/Rects used to construct and update the menu
// clang-format off

// Values in tiles
static const Rect     OPTIONS_TILES_SRC_RECT                   = { 0,  0, 29, 19};
static const BG_POINT OPTIONS_TILES_DEST_POS                   = { 0, 20};
static const Rect     OPTIONS_BACKGROUND_DEST_RECT             = { 3,  0, 26, 19};
static const BG_POINT OPTIONS_BACKGROUND_SRC_RECT_3X3_POS      = { 0, 20};
static const Rect     OPTIONS_BACKGROUND_CLEAR_SRC_RECT        = {16,  0, 29,  5};
static const BG_POINT OPTIONS_BACKGROUND_CLEAR_DEST_POS        = { 0,  0};

static const Rect     OPTIONS_SPEED_BTN_SRC_RECT               = { 4, 20,  9, 21};
static const BG_POINT OPTIONS_SPEED_BTN_DEST_POS               = {12,  3};
static const Rect     OPTIONS_SPEED_LESS_ACTIVE_BTN_SRC_RECT   = { 3, 20,  3, 21};
static const Rect     OPTIONS_SPEED_LESS_DISABLED_BTN_SRC_RECT = { 9, 22,  9, 23};
static const BG_POINT OPTIONS_SPEED_LESS_BTN_DEST_POS          = {11,  3};
static const Rect     OPTIONS_SPEED_MORE_ACTIVE_BTN_SRC_RECT   = {10, 20, 10, 21};
static const Rect     OPTIONS_SPEED_MORE_DISABLED_BTN_SRC_RECT = {10, 22, 10, 23};
static const BG_POINT OPTIONS_SPEED_MORE_BTN_DEST_POS          = {18,  3};
static const Rect     OPTIONS_SPEED_VALUES[GAME_SPEED_MAX]   = { { 6, 20,  7, 21},
                                                                 { 3, 22,  4, 23},
                                                                 { 5, 22,  6, 23},
                                                                 { 7, 22,  8, 23} };
static const BG_POINT OPTIONS_SPEED_VALUE_DEST_POS             = {14,  3};

static const Rect     OPTIONS_CONTRAST_BTN_SRC_RECT            = { 3, 24,  8, 25};
static const BG_POINT OPTIONS_CONTRAST_BTN_DEST_POS            = {12,  7};
static const Rect     OPTIONS_CONTRAST_VALUE_YES_SRC_RECT      = { 4, 24,  7, 25};
static const Rect     OPTIONS_CONTRAST_VALUE_NO_SRC_RECT       = { 9, 24, 12, 25};
static const BG_POINT OPTIONS_CONTRAST_VALUE_DEST_POS          = {13,  7};


static const Rect     OPTIONS_MUSIC_BAR_LEFT_END_SRC           = {11, 20, 11, 20};
static const BG_POINT OPTIONS_MUSIC_BAR_LEFT_END_DEST_POS      = { 4, 11};
static const Rect     OPTIONS_MUSIC_BAR_RIGHT_END_SRC          = {15, 20, 15, 20};
static const BG_POINT OPTIONS_MUSIC_BAR_RIGHT_END_DEST_POS     = {25, 11};
static const Rect     OPTIONS_SOUND_BAR_LEFT_END_SRC           = {11, 22, 11, 22};
static const BG_POINT OPTIONS_SOUND_BAR_LEFT_END_DEST_POS      = { 4, 14};
static const Rect     OPTIONS_SOUND_BAR_RIGHT_END_SRC          = {15, 22, 15, 22};
static const BG_POINT OPTIONS_SOUND_BAR_RIGHT_END_DEST_POS     = {25, 14};
static const Rect     OPTIONS_MUSIC_SLIDER_FULL_SRC            = {12, 20, 12, 20};
static const Rect     OPTIONS_MUSIC_SLIDER_MID_SRC             = {13, 20, 13, 20};
static const Rect     OPTIONS_MUSIC_SLIDER_EMPTY_SRC           = {14, 20, 14, 20};
static const Rect     OPTIONS_SOUND_SLIDER_FULL_SRC            = {12, 22, 12, 22};
static const Rect     OPTIONS_SOUND_SLIDER_MID_SRC             = {13, 22, 13, 22};
static const Rect     OPTIONS_SOUND_SLIDER_EMPTY_SRC           = {14, 22, 14, 22};
static const BG_POINT OPTIONS_MUSIC_SLIDER_START_POS           = { 5, 11};
static const BG_POINT OPTIONS_SOUND_SLIDER_START_POS           = { 5, 14};
static const BG_POINT OPTIONS_VOLUME_BAR_SHADOW_SRC_POS        = {12, 21};
static const Rect     OPTIONS_MUSIC_BAR_SHADOW_DEST            = { 5, 12, 24, 12};
static const Rect     OPTIONS_SOUND_BAR_SHADOW_DEST            = { 5, 15, 24, 15};

static const Rect     OPTIONS_BACK_BTN_DEST_RECT               = { 4, 16, 25, 18};
static const BG_POINT OPTIONS_BACK_BTN_SRC_RECT_3X3_POS        = { 0, 23};

// Values in pixels
static const BG_POINT OPTIONS_GAME_SPEED_TEXT_POS    = { 82,  16};
static const BG_POINT OPTIONS_HIGH_CONTRAST_TEXT_POS = { 40,  48};
static const BG_POINT OPTIONS_MUSIC_VOLUME_TEXT_POS  = { 56,  80};
static const BG_POINT OPTIONS_MUSIC_VALUE_TEXT_POS   = {160,  80};
static const BG_POINT OPTIONS_SOUND_VOLUME_TEXT_POS  = { 56, 104};
static const BG_POINT OPTIONS_SOUND_VALUE_TEXT_POS   = {160, 104};
static const BG_POINT OPTIONS_BACK_TEXT_POS          = {104, 136};
// clang-format on

static bool game_speed_changed = false;
static bool high_contrast_changed = false;
static bool music_volume_changed = false;
static bool sound_volume_changed = false;

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

    if (highlighted_btn != GAME_SPEED_BTN_IDX)
    {
        memcpy16(
            &pal_bg_mem[SPEED_BUTTON_OUTLINE_COLOR_PAL_IDX],
            &pal_bg_mem[MENU_BUTTON_MAIN_COLOR_PAL_IDX],
            1
        );
    }
    if (highlighted_btn != HIGH_CONTRAST_BTN_IDX)
    {
        memcpy16(
            &pal_bg_mem[CONTRAST_BUTTON_OUTLINE_COLOR_PAL_IDX],
            &pal_bg_mem[MENU_BUTTON_MAIN_COLOR_PAL_IDX],
            1
        );
    }
    if (highlighted_btn != MUSIC_VOLUME_BTN_IDX)
    {
        memcpy16(
            &pal_bg_mem[MUSIC_BUTTON_OUTLINE_COLOR_PAL_IDX],
            &pal_bg_mem[MENU_BUTTON_MAIN_COLOR_PAL_IDX],
            1
        );
    }
    if (highlighted_btn != SOUND_VOLUME_BTN_IDX)
    {
        memcpy16(
            &pal_bg_mem[SOUND_BUTTON_OUTLINE_COLOR_PAL_IDX],
            &pal_bg_mem[MENU_BUTTON_MAIN_COLOR_PAL_IDX],
            1
        );
    }
    if (highlighted_btn != BACK_BTN_IDX)
    {
        memcpy16(
            &pal_bg_mem[BACK_BUTTON_OUTLINE_COLOR_PAL_IDX],
            &pal_bg_mem[BACK_BUTTON_MAIN_COLOR_PAL_IDX],
            1
        );
    }
}

void game_options_menu_change_background(void)
{
    tte_erase_screen();
    CBB_CLEAR(MAIN_BG_CBB);

    GRIT_CPY(pal_bg_mem, background_options_menu_gfxPal);
    GRIT_CPY(&tile_mem[MAIN_BG_CBB], background_options_menu_gfxTiles);
    GRIT_CPY(&se_mem[MAIN_BG_SBB], background_options_menu_gfxMap);

    // Recreate menu frame

    // Copy tiles out of frame and clean up initial copy
    main_bg_se_copy_rect(OPTIONS_TILES_SRC_RECT, OPTIONS_TILES_DEST_POS);
    main_bg_se_copy_rect(OPTIONS_BACKGROUND_CLEAR_SRC_RECT, OPTIONS_BACKGROUND_CLEAR_DEST_POS);

    // Recreate base background frame
    main_bg_se_copy_expand_3x3_rect(
        OPTIONS_BACKGROUND_DEST_RECT,
        OPTIONS_BACKGROUND_SRC_RECT_3X3_POS
    );

    // Buttons
    main_bg_se_copy_rect(OPTIONS_SPEED_BTN_SRC_RECT, OPTIONS_SPEED_BTN_DEST_POS);
    main_bg_se_copy_rect(OPTIONS_CONTRAST_BTN_SRC_RECT, OPTIONS_CONTRAST_BTN_DEST_POS);
    main_bg_se_copy_expand_3x3_rect(OPTIONS_BACK_BTN_DEST_RECT, OPTIONS_BACK_BTN_SRC_RECT_3X3_POS);

    // Volume sliders
    main_bg_se_copy_rect(OPTIONS_MUSIC_BAR_LEFT_END_SRC, OPTIONS_MUSIC_BAR_LEFT_END_DEST_POS);
    main_bg_se_copy_rect(OPTIONS_MUSIC_BAR_RIGHT_END_SRC, OPTIONS_MUSIC_BAR_RIGHT_END_DEST_POS);
    main_bg_se_copy_rect(OPTIONS_SOUND_BAR_LEFT_END_SRC, OPTIONS_SOUND_BAR_LEFT_END_DEST_POS);
    main_bg_se_copy_rect(OPTIONS_SOUND_BAR_RIGHT_END_SRC, OPTIONS_SOUND_BAR_RIGHT_END_DEST_POS);
    main_bg_se_copy_expand_3w_row(OPTIONS_MUSIC_BAR_SHADOW_DEST, OPTIONS_VOLUME_BAR_SHADOW_SRC_POS);
    main_bg_se_copy_expand_3w_row(OPTIONS_SOUND_BAR_SHADOW_DEST, OPTIONS_VOLUME_BAR_SHADOW_SRC_POS);

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
        "#{P:%d,%d; cx:0x%X000}Music Volume",
        OPTIONS_MUSIC_VOLUME_TEXT_POS.x,
        OPTIONS_MUSIC_VOLUME_TEXT_POS.y,
        TTE_WHITE_PB
    );
    tte_printf(
        "#{P:%d,%d; cx:0x%X000}Sound Volume",
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

void game_options_menu_on_init(void)
{
    game_speed_changed = true;
    high_contrast_changed = true;
    music_volume_changed = true;
    sound_volume_changed = true;
    selection_y = 0;

    change_background(BG_OPTIONS_MENU);
    // Do a first update right off the bat
    game_options_menu_on_update();
}

void game_options_menu_on_update(void)
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
            memset16(&pal_bg_mem[SPEED_BUTTON_OUTLINE_COLOR_PAL_IDX], BTN_HIGHLIGHT_COLOR, 1);
            if (key_hit(KEY_LEFT) && g_game_vars.game_speed > GAME_SPEED_MIN)
            {
                g_game_vars.game_speed--;
                game_speed_changed = true;
            }
            else if (key_hit(KEY_RIGHT) && g_game_vars.game_speed < GAME_SPEED_MAX)
            {
                g_game_vars.game_speed++;
                game_speed_changed = true;
            }
            break;
        }

        // Toggle value by pressing the A button
        case HIGH_CONTRAST_BTN_IDX:
        {
            // Highlight button
            memset16(&pal_bg_mem[CONTRAST_BUTTON_OUTLINE_COLOR_PAL_IDX], BTN_HIGHLIGHT_COLOR, 1);
            if (key_hit(SELECT_CARD))
            {
                g_game_vars.high_contrast = (g_game_vars.high_contrast == 1) ? false : true;
                high_contrast_changed = true;
            }
            break;
        }

        // Sliders : decrease/increase value by pressing left/right until bar is empty/full
        case MUSIC_VOLUME_BTN_IDX:
        {
            // Highlight button
            memset16(&pal_bg_mem[MUSIC_BUTTON_OUTLINE_COLOR_PAL_IDX], BTN_HIGHLIGHT_COLOR, 1);
            if (key_hit(KEY_LEFT) && g_game_vars.music_volume > VOLUME_VALUE_MIN)
            {
                g_game_vars.music_volume--;
                music_volume_changed = true;
            }
            else if (key_hit(KEY_RIGHT) && g_game_vars.music_volume < VOLUME_VALUE_MAX)
            {
                g_game_vars.music_volume++;
                music_volume_changed = true;
            }
            break;
        }
        case SOUND_VOLUME_BTN_IDX:
        {
            // Highlight button
            memset16(&pal_bg_mem[SOUND_BUTTON_OUTLINE_COLOR_PAL_IDX], BTN_HIGHLIGHT_COLOR, 1);
            if (key_hit(KEY_LEFT) && g_game_vars.sound_volume > VOLUME_VALUE_MIN)
            {
                g_game_vars.sound_volume--;
                sound_volume_changed = true;
            }
            else if (key_hit(KEY_RIGHT) && g_game_vars.sound_volume < VOLUME_VALUE_MAX)
            {
                g_game_vars.sound_volume++;
                sound_volume_changed = true;
            }
            break;
        }

        case BACK_BTN_IDX:
        {
            // Highlight button
            memset16(&pal_bg_mem[BACK_BUTTON_OUTLINE_COLOR_PAL_IDX], BTN_HIGHLIGHT_COLOR, 1);
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

    // check if need to disable game speed arrows
    if (game_speed_changed)
    {
        if (g_game_vars.game_speed == GAME_SPEED_MIN)
        {
            main_bg_se_copy_rect(
                OPTIONS_SPEED_LESS_DISABLED_BTN_SRC_RECT,
                OPTIONS_SPEED_LESS_BTN_DEST_POS
            );
        }
        else
        {
            main_bg_se_copy_rect(
                OPTIONS_SPEED_LESS_ACTIVE_BTN_SRC_RECT,
                OPTIONS_SPEED_LESS_BTN_DEST_POS
            );
        }

        if (g_game_vars.game_speed == GAME_SPEED_MAX)
        {
            main_bg_se_copy_rect(
                OPTIONS_SPEED_MORE_DISABLED_BTN_SRC_RECT,
                OPTIONS_SPEED_MORE_BTN_DEST_POS
            );
        }
        else
        {
            main_bg_se_copy_rect(
                OPTIONS_SPEED_MORE_ACTIVE_BTN_SRC_RECT,
                OPTIONS_SPEED_MORE_BTN_DEST_POS
            );
        }

        main_bg_se_copy_rect(
            OPTIONS_SPEED_VALUES[g_game_vars.game_speed - 1],
            OPTIONS_SPEED_VALUE_DEST_POS
        );

        game_speed_changed = false;
    }

    if (high_contrast_changed)
    {
        if (g_game_vars.high_contrast)
        {
            main_bg_se_copy_rect(
                OPTIONS_CONTRAST_VALUE_YES_SRC_RECT,
                OPTIONS_CONTRAST_VALUE_DEST_POS
            );
        }
        else
        {
            main_bg_se_copy_rect(
                OPTIONS_CONTRAST_VALUE_NO_SRC_RECT,
                OPTIONS_CONTRAST_VALUE_DEST_POS
            );
        }

        high_contrast_changed = false;
    }

    if (music_volume_changed)
    {
        int i = 0;
        BG_POINT slider_segment_dest = OPTIONS_MUSIC_SLIDER_START_POS;

        // full part of the bar
        for (; i < g_game_vars.music_volume - 1; i++)
        {
            main_bg_se_copy_rect(OPTIONS_MUSIC_SLIDER_FULL_SRC, slider_segment_dest);
            slider_segment_dest.x++;
        }

        // at exactly music_volume we either:
        //  - are in the middle of the bar, then we draw the frontier between full and empty tiles
        //  - are at the end, then draw a full segment
        if (g_game_vars.music_volume == VOLUME_VALUE_MAX)
        {
            main_bg_se_copy_rect(OPTIONS_MUSIC_SLIDER_FULL_SRC, slider_segment_dest);
        }
        else
        {
            if (g_game_vars.music_volume != VOLUME_VALUE_MIN)
            {
                // draw middle point
                main_bg_se_copy_rect(OPTIONS_MUSIC_SLIDER_MID_SRC, slider_segment_dest);
                i++;
                slider_segment_dest.x++;
            }

            // empty part of the bar
            for (; i < VOLUME_VALUE_MAX; i++)
            {
                main_bg_se_copy_rect(OPTIONS_MUSIC_SLIDER_EMPTY_SRC, slider_segment_dest);
                slider_segment_dest.x++;
            }
        }

        tte_printf(
            "#{P:%d,%d; cx:0x%X000}%3d",
            OPTIONS_MUSIC_VALUE_TEXT_POS.x,
            OPTIONS_MUSIC_VALUE_TEXT_POS.y,
            TTE_WHITE_PB,
            (g_game_vars.music_volume * VOLUME_VALUE_INCREMENT)
        );

        music_volume_changed = false;
    }

    if (sound_volume_changed)
    {
        int i = 0;
        BG_POINT slider_segment_dest = OPTIONS_SOUND_SLIDER_START_POS;

        for (; i < g_game_vars.sound_volume - 1; i++)
        {
            main_bg_se_copy_rect(OPTIONS_SOUND_SLIDER_FULL_SRC, slider_segment_dest);
            slider_segment_dest.x++;
        }

        if (g_game_vars.sound_volume == VOLUME_VALUE_MAX)
        {
            main_bg_se_copy_rect(OPTIONS_SOUND_SLIDER_FULL_SRC, slider_segment_dest);
        }
        else
        {
            if (g_game_vars.sound_volume != VOLUME_VALUE_MIN)
            {
                main_bg_se_copy_rect(OPTIONS_SOUND_SLIDER_MID_SRC, slider_segment_dest);
                i++;
                slider_segment_dest.x++;
            }

            for (; i < VOLUME_VALUE_MAX; i++)
            {
                main_bg_se_copy_rect(OPTIONS_SOUND_SLIDER_EMPTY_SRC, slider_segment_dest);
                slider_segment_dest.x++;
            }
        }

        tte_printf(
            "#{P:%d,%d; cx:0x%X000}%3d",
            OPTIONS_SOUND_VALUE_TEXT_POS.x,
            OPTIONS_SOUND_VALUE_TEXT_POS.y,
            TTE_WHITE_PB,
            (g_game_vars.sound_volume * VOLUME_VALUE_INCREMENT)
        );

        sound_volume_changed = false;
    }
}

void game_options_menu_on_exit(void)
{
    save_game();
    tte_erase_screen();
}
