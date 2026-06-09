#include "game/game_over.h"

#include "affine_background.h"
#include "audio_utils.h"
#include "button.h"
#include "game.h"
#include "graphic_utils.h"
#include "layout.h"
#include "random.h"
#include "selection_grid.h"
#include "soundbank.h"
#include "timer.h"
#include "util.h"

#include "background_game_over_gfx.h"

#include <tonc.h>

#define RED_BTN_MAIN_COLOR_PAL_IDX     6
#define BLUE_BTN_MAIN_COLOR_PAL_IDX    7
#define REUSE_SEED_BTN_OUTLINE_PAL_IDX 8
#define NEW_RUN_BTN_OUTLINE_PAL_IDX    9
#define MAIN_MENU_BTN_OUTLINE_PAL_IDX  10
#define ENDLESS_BTN_OUTLINE_PAL_IDX    11

enum EndCondition
{
    END_CONDITION_NONE,
    END_CONDITION_WIN,
    END_CONDITION_LOSS
};

// clang-format off
static const Rect     GAME_OVER_ANIM_RECT             = {  4,   5,  25,  31};

static const Rect     GAME_OVER_TEXT_TILES_SRC_RECT_1 = {  0,  20,   3,  21};
static const Rect     GAME_OVER_TEXT_TILES_SRC_RECT_2 = {  0,  22,   3,  23};
static const Rect     GAME_OVER_TEXT_TILES_SRC_RECT_3 = {  0,  24,   1,  25};
static const BG_POINT GAME_OVER_TEXT_TILES_DEST_POS_1 = { 10,  21};
static const BG_POINT GAME_OVER_TEXT_TILES_DEST_POS_2 = { 14,  21};
static const BG_POINT GAME_OVER_TEXT_TILES_DEST_POS_3 = { 18,  21};

static const Rect     DEFEATED_BY_FRAME_SRC_RECT      = { 27,  22,  31,  28};
static const BG_POINT DEFEATED_BY_FRAME_DEST_POS      = { 21,  23};
static const BG_POINT DEFEATED_BY_TOKEN_INIT_POS      = {175, 207};

static const BG_POINT NEW_RUN_BTN_3X3_SRC_POS         = { 26,  29};
static const BG_POINT MAIN_MENU_BTN_3X3_SRC_POS       = { 29,  29};
static const Rect     NEW_RUN_BTN_3X3_DEST_RECT       = {  5,  29,  13,  31};
static const Rect     MAIN_MENU_BTN_3X3_DEST_RECT     = { 14,  29,  24,  31};
static const Rect     MAIN_MENU_SEED_OVERLAP_SRC_RECT = {  2,  29,   3,  29};
static const BG_POINT MAIN_MENU_SEED_OVERLAP_DEST_POS = {  5,  29};

static const Rect     REUSE_SEED_BTN_OFF_SRC_RECT     = {  0,  30,   1,  31};
static const Rect     REUSE_SEED_BTN_ON_SRC_RECT      = {  2,  30,   3,  31};
static const BG_POINT REUSE_SEED_BTN_DEST_POS         = {  4,  11};

static const BG_POINT BEST_HAND_VALUE_POS             = {126,  64};
static const BG_POINT MOST_PLAYED_HAND_VALUE_POS      = {112,  80};
static const BG_POINT SEED_VALUE_POS                  = {112,  96};

static const BG_POINT ENDLESS_BTN_TEXT_POS            = { 72, 112};
static const BG_POINT NEW_RUN_BTN_TEXT_POS            = { 48, 112};
static const BG_POINT MAIN_MENU_BTN_TEXT_POS          = {120, 112};
// clang-format on

static const u32 GAME_OVER_ANIM_FRAMES = 17;
static const mm_byte GAME_OVER_SFX_VOL = 178;     // 70% of MM_SFX_FULL_VOLUME
static const mm_word GAME_OVER_WHOOSH_RATE = 922; // 90% of MM_BASE_PITCH_RATE

// Common Selection Grid structs and funcs

static int game_over_get_row_size1(void)
{
    return 1;
}
static int game_over_get_row_size2(void)
{
    return 2;
}

static void reuse_seed_on_pressed(void);
static void new_run_on_pressed(void);
static void main_menu_on_pressed(void);
static void endless_on_pressed(void);

static Button reuse_seed_button = {
    REUSE_SEED_BTN_OUTLINE_PAL_IDX,
    RED_BTN_MAIN_COLOR_PAL_IDX,
    reuse_seed_on_pressed,
    NULL
};
static Button new_run_button = {
    NEW_RUN_BTN_OUTLINE_PAL_IDX,
    RED_BTN_MAIN_COLOR_PAL_IDX,
    new_run_on_pressed,
    NULL
};
static Button main_menu_button = {
    MAIN_MENU_BTN_OUTLINE_PAL_IDX,
    RED_BTN_MAIN_COLOR_PAL_IDX,
    main_menu_on_pressed,
    NULL
};
static Button endless_button = {
    ENDLESS_BTN_OUTLINE_PAL_IDX,
    BLUE_BTN_MAIN_COLOR_PAL_IDX,
    endless_on_pressed,
    NULL
};

// WIN Selection Grid

enum GameOverWinRows
{
    GAME_OVER_WIN_NEW_RUN_ROW,
    GAME_OVER_WIN_SEED_MENU_ROW,
    GAME_OVER_WIN_ENDLESS_ROW,
    GAME_OVER_WIN_ROW_MAX
};

static void game_over_win_on_key_transit(SelectionGrid* selection_grid, Selection* selection);
static bool game_over_win_on_selection_changed(
    SelectionGrid* selection_grid,
    int row_idx,
    const Selection* prev_selection,
    const Selection* new_selection
);

static const SelectionGridRow game_over_win_selection_rows[] = {
    {
        0,
        game_over_get_row_size1,
        game_over_win_on_selection_changed,
        game_over_win_on_key_transit,
        {.wrap = false, .has_h_exit_idx = true, .h_exit_idx = 1}
    }, {
        1,
        game_over_get_row_size2,
        game_over_win_on_selection_changed,
        game_over_win_on_key_transit,
        {.wrap = false}
    }, {
        2,
        game_over_get_row_size1,
        game_over_win_on_selection_changed,
        game_over_win_on_key_transit,
        {.wrap = false}
    }
};

static const Selection GAME_OVER_WIN_INIT_SEL = {0, GAME_OVER_WIN_ENDLESS_ROW};
static SelectionGrid game_over_win_selection_grid = {
    game_over_win_selection_rows,
    GAME_OVER_WIN_ROW_MAX,
    GAME_OVER_WIN_INIT_SEL
};

// LOSE Selection Grid

enum GameOverLoseRows
{
    GAME_OVER_LOSE_SEED_ROW,
    GAME_OVER_LOSE_RUN_MENU_ROW,
    GAME_OVER_LOSE_ROW_MAX
};

static void game_over_lose_on_key_transit(SelectionGrid* selection_grid, Selection* selection);
static bool game_over_lose_on_selection_changed(
    SelectionGrid* selection_grid,
    int row_idx,
    const Selection* prev_selection,
    const Selection* new_selection
);

static const SelectionGridRow game_over_lose_selection_rows[] = {
    {
        GAME_OVER_LOSE_SEED_ROW,
        game_over_get_row_size1,
        game_over_lose_on_selection_changed,
        game_over_lose_on_key_transit,
        {.wrap = false}
    }, {
        GAME_OVER_LOSE_RUN_MENU_ROW,
        game_over_get_row_size2,
        game_over_lose_on_selection_changed,
        game_over_lose_on_key_transit,
        {.wrap = false}
    }
};

static const Selection GAME_OVER_LOSE_INIT_SEL = {0, GAME_OVER_LOSE_RUN_MENU_ROW};
static SelectionGrid game_over_lose_selection_grid = {
    game_over_lose_selection_rows,
    GAME_OVER_LOSE_ROW_MAX,
    GAME_OVER_LOSE_INIT_SEL
};

// Internal Variables

static enum EndCondition condition = END_CONDITION_NONE;
static u32 timer = TM_ZERO;
static bool reuse_seed = false;
static bool continue_run = false;

static void game_over_change_background(enum EndCondition init_condition)
{
    condition = init_condition;
    //condition = END_CONDITION_WIN;
    timer = TM_ZERO;
    reuse_seed = false;
    continue_run = false;

    // Clears the round end menu
    toggle_windows(false, false);
    GRIT_CPY(pal_bg_mem, background_game_over_gfxPal);
    GRIT_CPY(&tile_mem[MAIN_BG_CBB], background_game_over_gfxTiles);
    GRIT_CPY(&se_mem[MAIN_BG_SBB], background_game_over_gfxMap);

    // Move blind token offscreen, will move it back into view if contition is END_CONDITION_LOSS
    sprite_position(
        g_game_vars.playing_blind_token,
        DEFEATED_BY_TOKEN_INIT_POS.x,
        DEFEATED_BY_TOKEN_INIT_POS.y
    );

    button_set_highlight(&new_run_button, false);
    button_set_highlight(&main_menu_button, false);
    button_set_highlight(&reuse_seed_button, false);
    button_set_highlight(&endless_button, false);

    // Using the text color to match the Win/Loss condition
    switch (condition)
    {
        case END_CONDITION_WIN:
            affine_background_set_color(TEXT_CLR_BLUE);

            // Highlight Endless mode button
            button_set_highlight(&endless_button, true);

            break;

        case END_CONDITION_LOSS:
            affine_background_set_color(TEXT_CLR_RED);

            // Change top text to "GAME OVER"
            main_bg_se_copy_rect(GAME_OVER_TEXT_TILES_SRC_RECT_1, GAME_OVER_TEXT_TILES_DEST_POS_1);
            main_bg_se_copy_rect(GAME_OVER_TEXT_TILES_SRC_RECT_2, GAME_OVER_TEXT_TILES_DEST_POS_2);
            main_bg_se_copy_rect(GAME_OVER_TEXT_TILES_SRC_RECT_3, GAME_OVER_TEXT_TILES_DEST_POS_3);

            // Move side buttons to the bottom in lieu of the "Endless" button
            main_bg_se_copy_expand_3x3_rect(NEW_RUN_BTN_3X3_DEST_RECT, NEW_RUN_BTN_3X3_SRC_POS);
            main_bg_se_copy_rect(MAIN_MENU_SEED_OVERLAP_SRC_RECT, MAIN_MENU_SEED_OVERLAP_DEST_POS);
            main_bg_se_copy_expand_3x3_rect(MAIN_MENU_BTN_3X3_DEST_RECT, MAIN_MENU_BTN_3X3_SRC_POS);

            // Change side buttons to blind frame
            main_bg_se_copy_rect(DEFEATED_BY_FRAME_SRC_RECT, DEFEATED_BY_FRAME_DEST_POS);

            // Highlight New Run button
            button_set_highlight(&new_run_button, true);
            break;

        default:
            break;
    }
}

void game_win_on_init(void)
{
    play_sfx(SFX_GAME_WIN, MM_BASE_PITCH_RATE, SFX_DEFAULT_VOLUME);

    game_over_change_background(END_CONDITION_WIN);
}

void game_lose_on_init(void)
{
    play_sfx(SFX_NEGATIVE_LOSE, MM_BASE_PITCH_RATE / 2, GAME_OVER_SFX_VOL);
    play_sfx(SFX_WHOOSH2_LOSE, GAME_OVER_WHOOSH_RATE, GAME_OVER_SFX_VOL);

    play_lose_music();

    game_over_change_background(END_CONDITION_LOSS);
}

/**
 * @brief Polls user input to detect "Retry" button press.
 */
static inline void game_over_process_user_input()
{
    switch (condition)
    {
        case END_CONDITION_WIN:
            selection_grid_process_input(&game_over_win_selection_grid);
            break;
        case END_CONDITION_LOSS:
            selection_grid_process_input(&game_over_lose_selection_grid);
            break;
        default:
            break;
    }
}

void game_over_on_update(void)
{
    timer++;

    // Need to clear text here or the number of cards remaining in deck stays for some reason
    if (timer == 1)
        tte_erase_screen();

    if (timer < GAME_OVER_ANIM_FRAMES)
    {
        main_bg_se_move_rect_1_tile_vert(GAME_OVER_ANIM_RECT, SCREEN_UP);

        if (condition == END_CONDITION_LOSS)
        {
            sprite_position(
                g_game_vars.playing_blind_token,
                g_game_vars.playing_blind_token->pos.x,
                g_game_vars.playing_blind_token->pos.y - TILE_SIZE
            );
        }
    }
    else if (timer == GAME_OVER_ANIM_FRAMES)
    {
        // TODO get value from game vars
        char best_hand_str[UINT_MAX_DIGITS + 1];
        truncate_uint_to_suffixed_str(MAX_BASE36, 5, best_hand_str);
        tte_printf(
            "#{P:%d,%d; cx:0x%X000}%s",
            BEST_HAND_VALUE_POS.x,
            BEST_HAND_VALUE_POS.y,
            TTE_RED_PB,
            best_hand_str
        );

        // TODO get value from game vars
        tte_printf(
            "#{P:%d,%d; cx:0x%X000}Flush 5",
            MOST_PLAYED_HAND_VALUE_POS.x,
            MOST_PLAYED_HAND_VALUE_POS.y,
            TTE_WHITE_PB
        );

        // Run's seed
        char seed_str[BASE36_MAX_DIGITS + 1] = {'\0'};
        u32_to_base36(g_game_vars.rng_info.seed, seed_str);
        tte_printf(
            "#{P:%d,%d; cx:0x%X000}%s",
            SEED_VALUE_POS.x,
            SEED_VALUE_POS.y,
            TTE_WHITE_PB,
            seed_str
        );

        switch (condition)
        {
            case END_CONDITION_WIN:
                tte_printf(
                    "#{P:%d,%d; cx:0x%X000}Endless Mode",
                    ENDLESS_BTN_TEXT_POS.x,
                    ENDLESS_BTN_TEXT_POS.y,
                    TTE_WHITE_PB
                );
                break;
            case END_CONDITION_LOSS:
                tte_printf(
                    "#{P:%d,%d; cx:0x%X000}New Run",
                    NEW_RUN_BTN_TEXT_POS.x,
                    NEW_RUN_BTN_TEXT_POS.y,
                    TTE_WHITE_PB
                );
                tte_printf(
                    "#{P:%d,%d; cx:0x%X000}Main Menu",
                    MAIN_MENU_BTN_TEXT_POS.x,
                    MAIN_MENU_BTN_TEXT_POS.y,
                    TTE_WHITE_PB
                );
                break;
            default:
                break;
        }
    }

    game_over_process_user_input();
}

void game_over_on_exit(void)
{
    toggle_windows(false, false);

    play_regular_music();
    condition = END_CONDITION_NONE;

    u32 previous_seed = g_game_vars.rng_info.seed;

    if (!continue_run)
        game_reset();

    if (!reuse_seed)
        rng_shuffle_seed();
    else
        g_game_vars.rng_info.seed = previous_seed;
}

// WIN SelectionGrid

static Button* game_over_win_get_button_from_sel(const Selection* selection)
{
    switch (selection->y)
    {
        case GAME_OVER_WIN_NEW_RUN_ROW:
            return &new_run_button;
        case GAME_OVER_WIN_SEED_MENU_ROW:
            if (selection->x == 0)
                return &reuse_seed_button;
            else
                return &main_menu_button;
        case GAME_OVER_WIN_ENDLESS_ROW:
            return &endless_button;
        default:
            break;
    }
    return NULL;
}

static void game_over_win_on_key_transit(SelectionGrid* selection_grid, Selection* selection)
{
    if (key_hit(SELECT_CARD))
        button_press(game_over_win_get_button_from_sel(selection));
}

static bool game_over_win_on_selection_changed(
    SelectionGrid* selection_grid,
    int row_idx,
    const Selection* prev_selection,
    const Selection* new_selection
)
{
    bool proceed_selection = true;

    // Remove highlight from previous button
    if (row_idx == prev_selection->y)
        button_set_highlight(game_over_win_get_button_from_sel(prev_selection), false);

    // Highlight new button
    if (row_idx == new_selection->y)
    {
        Selection shifted_selection = *new_selection;

        // If we are landing on the second row from another one, always land on x = 1
        // which corresponds to the "main menu" button, this makes the most sense
        if (new_selection->y == GAME_OVER_WIN_SEED_MENU_ROW && prev_selection->y != new_selection->y)
        {
            shifted_selection.x = 1;
            selection_grid->selection = shifted_selection;
            proceed_selection = false;
        }

        button_set_highlight(game_over_win_get_button_from_sel(&shifted_selection), true);
    }

    return proceed_selection;
}

// LOSE SelectionGrid

static Button* game_over_lose_get_button_from_sel(const Selection* selection)
{
    switch (selection->y)
    {
        case GAME_OVER_LOSE_SEED_ROW:
            return &reuse_seed_button;
        case GAME_OVER_LOSE_RUN_MENU_ROW:
            if (selection->x == 0)
                return &new_run_button;
            else
                return &main_menu_button;
        default:
            break;
    }
    return NULL;
}

static void game_over_lose_on_key_transit(SelectionGrid* selection_grid, Selection* selection)
{
    if (key_hit(SELECT_CARD))
        button_press(game_over_lose_get_button_from_sel(selection));
}

static bool game_over_lose_on_selection_changed(
    SelectionGrid* selection_grid,
    int row_idx,
    const Selection* prev_selection,
    const Selection* new_selection
)
{
    // Remove highlight from previous button
    if (row_idx == prev_selection->y)
        button_set_highlight(game_over_lose_get_button_from_sel(prev_selection), false);

    // Highlight new button
    if (row_idx == new_selection->y)
        button_set_highlight(game_over_lose_get_button_from_sel(new_selection), true);

    return true;
}

// Buttons

static void reuse_seed_on_pressed(void)
{
    reuse_seed = !reuse_seed;
    main_bg_se_copy_rect(
        reuse_seed ? REUSE_SEED_BTN_ON_SRC_RECT : REUSE_SEED_BTN_OFF_SRC_RECT,
        REUSE_SEED_BTN_DEST_POS
    );
}

static void new_run_on_pressed(void)
{
    game_change_state(GAME_STATE_RUN_SETUP);
}

static void main_menu_on_pressed(void)
{
    game_change_state(GAME_STATE_MAIN_MENU);
}

static void endless_on_pressed(void)
{
    continue_run = true;
    game_change_state(GAME_STATE_SHOP);
}
