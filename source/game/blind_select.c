#include "blind_select.h"

#include "audio_utils.h"
#include "blind.h"
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


// TODO: this will be refactored into common state machine 
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
static const u32 TM_BLIND_SELECT_START = 1;

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

static inline void game_blind_select_erase_blind_reqs_and_rewards()
{
    for (enum BlindTokens curr_blind = SMALL_BLIND; curr_blind < NUM_BLINDS_PER_ANTE; curr_blind++)
    {
        Rect blind_req_and_reward_rect = SINGLE_BLIND_SEL_REQ_SCORE_RECT;

        // To account for both raised blind and reward
        blind_req_and_reward_rect.top -= TILE_SIZE;
        blind_req_and_reward_rect.bottom += TILE_SIZE;

        // To account for overflow
        blind_req_and_reward_rect.right += TILE_SIZE;

        blind_req_and_reward_rect.left +=
            curr_blind * rect_width(&SINGLE_BLIND_SELECT_RECT) * TILE_SIZE;
        blind_req_and_reward_rect.right +=
            curr_blind * rect_width(&SINGLE_BLIND_SELECT_RECT) * TILE_SIZE;

        tte_erase_rect_wrapper(blind_req_and_reward_rect);
    }
}

// TODO: Clean this up
static void increment_blind(enum BlindState increment_reason)
{
    // cannot do blind++ anymore, we need to go SMALL->BIG->next_boss->SMALL...
    switch (g_game_vars.current_blind)
    {
        // defeated small blind: go to big
        case BLIND_TYPE_SMALL:
            g_game_vars.current_blind = BLIND_TYPE_BIG;
            blinds_states[SMALL_BLIND] = increment_reason;
            blinds_states[BIG_BLIND] = BLIND_STATE_CURRENT;
            break;
        // defeated big blind: go to next boss
        case BLIND_TYPE_BIG:
            g_game_vars.current_blind = next_boss_blind;
            blinds_states[BIG_BLIND] = increment_reason;
            blinds_states[BOSS_BLIND] = BLIND_STATE_CURRENT;
            break;
        // defeated a boss: reset everything
        default:
            g_game_vars.current_blind = BLIND_TYPE_SMALL;
            blinds_states[SMALL_BLIND] = BLIND_STATE_CURRENT; // Reset the blinds to the first one
            blinds_states[BIG_BLIND] = BLIND_STATE_UPCOMING;  // Set the next blind to upcoming
            blinds_states[BOSS_BLIND] = BLIND_STATE_UPCOMING; // Set the next blind to upcoming
            break;
    }
}

static void game_blind_select_handle_input()
{
    if (g_game_vars.timer == TM_BLIND_SELECT_START && g_game_vars.current_blind == BLIND_TYPE_BOSS)
    {
        selection_y = 0;
    }

    // Blind select input logic
    if (key_hit(KEY_UP))
    {
        selection_y = 0;
    }
    else if (key_hit(KEY_DOWN) && g_game_vars.current_blind <= BLIND_TYPE_BIG)
    {
        selection_y = 1;
    }
    else if (key_hit(SELECT_CARD))
    {
        game_blind_select_erase_blind_reqs_and_rewards();

        if (selection_y == 0) // Blind selected
        {
            play_sfx(SFX_BUTTON, MM_BASE_PITCH_RATE, BUTTON_SFX_VOLUME);
            substate = BLIND_SELECTED_ANIM_SEQ;
            g_game_vars.timer = TM_ZERO;
            ++g_game_vars.round;
            display_round();
        }
        // TODO: the else if is funky here
        else if (g_game_vars.current_blind <= BLIND_TYPE_BIG)
        {
            play_sfx(SFX_BUTTON, MM_BASE_PITCH_RATE, BUTTON_SFX_VOLUME);
            increment_blind(BLIND_STATE_SKIPPED);

            selection_y = 0; // Reset selection to first option

            background = UNDEFINED; // Force refresh of the background
            change_background(BG_BLIND_SELECT);

            // TODO: Create a generic vertical move by any number of tiles to avoid for loops?
            for (int i = 0; i < 12; i++)
            {
                main_bg_se_copy_rect_1_tile_vert(POP_MENU_ANIM_RECT, SCREEN_UP);
            }

            for (int i = 0; i < NUM_BLINDS_PER_ANTE; i++)
            {
                sprite_position(
                    blind_select_tokens[i],
                    blind_select_tokens[i]->pos.x,
                    blind_select_tokens[i]->pos.y - (TILE_SIZE * 12)
                );
            }

            game_blind_select_print_blinds_reqs_and_rewards();

            g_game_vars.timer = TM_ZERO;
        }
    }

    if (selection_y == 0)
    {
        memset16(&pal_bg_mem[BLIND_SELECT_BTN_SELECTED_BORDER_PID], 0xFFFF, 1);
        memcpy16(
            &pal_bg_mem[BLIND_SKIP_BTN_SELECTED_BORDER_PID],
            &pal_bg_mem[BLIND_SKIP_BTN_PID],
            1
        );
    }
    else
    {
        memcpy16(
            &pal_bg_mem[BLIND_SELECT_BTN_SELECTED_BORDER_PID],
            &pal_bg_mem[BLIND_SELECT_BTN_PID],
            1
        );
        memset16(&pal_bg_mem[BLIND_SKIP_BTN_SELECTED_BORDER_PID], 0xFFFF, 1);
    }
}

static void game_blind_select_selected_anim_seq()
{
    if (g_game_vars.timer < 15)
    {
        Rect blinds_rect = POP_MENU_ANIM_RECT;
        blinds_rect.top -= 1; // Because of the raised blind
        main_bg_se_move_rect_1_tile_vert(blinds_rect, SCREEN_DOWN);

        for (int i = 0; i < NUM_BLINDS_PER_ANTE; i++)
        {
            sprite_position(
                blind_select_tokens[i],
                blind_select_tokens[i]->pos.x,
                blind_select_tokens[i]->pos.y + TILE_SIZE
            );
        }
    }
    else if (g_game_vars.timer >= MENU_POP_OUT_ANIM_FRAMES)
    {
        for (int i = 0; i < NUM_BLINDS_PER_ANTE; i++)
        {
            obj_hide(blind_select_tokens[i]->obj);
        }

        substate = DISPLAY_BLIND_PANEL; // Reset the state
        g_game_vars.timer = TM_ZERO;                           // Reset the timer
    }
}

void game_blind_select_on_init(void)
{
    //change_background(BG_BLIND_SELECT);
    selection_x = 0;
    selection_y = 0;

    play_sfx(SFX_POP, MM_BASE_PITCH_RATE, SFX_DEFAULT_VOLUME);
}

void game_blind_select_on_update(void)
{
    if (substate == BLIND_SELECT_MAX)
    {
        game_change_state(GAME_STATE_PLAYING);
        return;
    }

    blind_select_state_actions[substate]();
}
