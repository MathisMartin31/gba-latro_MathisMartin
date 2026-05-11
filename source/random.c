#include "random.h"

#include "game_variables.h"

#include <stdlib.h>
#include <tonc.h>

u32 rng_salt = 0;

void rng_init(void)
{
    profile_start();
}

void rng_shuffle_seed(void)
{
    // Count the total cycles since game boot. Overflow is not an issue here.
    rng_salt += profile_stop();

    g_game_vars.rng_seed++;
    // If the keys have changed, make it more pseudo-random
    if (key_curr_state() != key_prev_state())
    {
        g_game_vars.rng_seed *= 2;
    }

    profile_start();
}

u32 rng_get_u32(void)
{
    g_game_vars.rng_step++;
    return rand();
}
