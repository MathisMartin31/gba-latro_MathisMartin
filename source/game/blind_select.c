#include "blind_select.h"

#include "audio_utils.h"
#include "game.h"
#include "game_variables.h"
#include "graphic_utils.h"
#include "sprite.h"

static void game_blind_select_start_anim_seq(void);
static void game_blind_select_handle_input(void);
static void game_blind_select_selected_anim_seq(void);
static void game_blind_select_display_blind_panel(void);
static Rect game_blind_select_get_req_score_rect(enum BlindTokens blind);
static void game_blind_select_print_blinds_reqs_and_rewards(void);

enum BlindSelectState
{
    START_ANIM_SEQ,
    BLIND_SELECT,
    BLIND_SELECTED_ANIM_SEQ,
    DISPLAY_BLIND_PANEL,
    BLIND_SELECT_MAX
};

// The sprites that display the blinds when in "GAME_BLIND_SELECT" state
// There are only 3 blinds per Ante, so we don't need more sprites than that
enum BlindTokens
{
    SMALL_BLIND,
    BIG_BLIND,
    BOSS_BLIND,
    NUM_BLINDS_PER_ANTE
};


static const SubStateActionFn blind_select_state_actions[] = {
    game_blind_select_start_anim_seq,
    game_blind_select_handle_input,
    game_blind_select_selected_anim_seq,
    game_blind_select_display_blind_panel
};

static int selection_x = 0;
static int selection_y = 0;

enum BlindSelectState substate;

static const Rect POP_MENU_ANIM_RECT        = {9,       7,      24,     31 };
static const u32 TM_END_ANIM_SEQ = 12;

static void game_blind_select_start_anim_seq()
{
    main_bg_se_copy_rect_1_tile_vert(POP_MENU_ANIM_RECT, SCREEN_UP);

    for (int i = 0; i < NUM_BLINDS_PER_ANTE; i++)
    {
        sprite_position(
            blind_select_tokens[i],
            blind_select_tokens[i]->pos.x,
            blind_select_tokens[i]->pos.y - TILE_SIZE
        );
    }

    if (g_game_vars.timer == TM_END_ANIM_SEQ)
    {
        game_blind_select_print_blinds_reqs_and_rewards();
        substate = BLIND_SELECT;
        g_game_vars.timer = TM_ZERO; // Reset the timer
    }
}

void game_blind_select_on_init(void)
{
    //change_background(BG_BLIND_SELECT);
    selection_x = 0;
    selection_y = 0;

    play_sfx(SFX_POP, MM_BASE_PITCH_RATE, SFX_DEFAULT_VOLUME);
}

static void game_blind_select_on_update(void)
{
    if (substate == BLIND_SELECT_MAX)
    {
        game_change_state(GAME_STATE_PLAYING);
        return;
    }

    blind_select_state_actions[substate]();
}
