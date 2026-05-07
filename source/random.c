#include "random.h"

#include "game_variables.h"

#include <stdlib.h>

void shuffle_rng_seed(void)
{
    g_game_vars.rng_seed++;
    // If the keys have changed, make it more pseudo-random
    if (key_curr_state() != key_prev_state())
    {
        g_game_vars.rng_seed *= 2;
    }
}

u32 get_rand(void)
{
    g_game_vars.rng_step++;
    return rand();
}