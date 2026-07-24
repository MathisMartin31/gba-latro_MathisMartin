#include "random.h"

#include "game_variables.h"
#include "util.h"

#include <stdlib.h>
#include <tonc.h>

// Accumulate timer 1 into a bigger variable so we can generate more diverse seeds
static u32 s_timer_acc = 0;

// Timers usage docs: https://gbadev.net/tonc/timers.html
void rng_init(void)
{
    REG_TM1D = 0;
    REG_TM1CNT = TM_FREQ_1 | TM_ENABLE; // using timer with x1 prescale
}

void rng_update(void)
{
    s_timer_acc += (u32)REG_TM1D;
}

void rng_set_seed(u32 seed)
{
    // We store the seed to display it at the end of the run, but here it's only used to generate
    // the independent rng sequences' initial states. We also avoid the seed 0 as the Xorshift32
    // method used will stay stuck.
    g_game_vars.rng_info.seed = (seed == 0 ? MAX_BASE36 : seed) % (MAX_BASE36 + 1);
    srand(g_game_vars.rng_info.seed);

    // Generate rng states for all RngTypes categories using the given seed
    for (enum RngType type = 0; type < RNG_TYPE_MAX; type++)
    {
        g_game_vars.rng_info.states[type] = rand();
    }
}

void rng_shuffle_seed(void)
{
    rng_set_seed(s_timer_acc);
}

u32 rng_get_u32(enum RngType type)
{
    // Xorshift32
    uint32_t old_state = g_game_vars.rng_info.states[type];
    old_state ^= old_state << 13;
    old_state ^= old_state >> 17;
    old_state ^= old_state << 5;

    g_game_vars.rng_info.states[type] = old_state;
    return old_state;
}

void rng_restore(RngInfo info)
{
    g_game_vars.rng_info = info;
}
