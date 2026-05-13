#include "random.h"

#include "game_variables.h"

#include <stdlib.h>
#include <tonc.h>

void rng_start_sampling(void)
{
    profile_start();
}

void rng_cancel_sampling(void)
{
    (void)profile_stop();
}

void rng_set_seed(u32 seed)
{
    g_game_vars.rng_seed = (seed % MAX_SEED);
    g_game_vars.rng_step = 0;
    srand(g_game_vars.rng_seed);
}

void rng_shuffle_seed(void)
{
    // Count the total cycles since either game boot or game_over_on_init.
    // Overflow is not a concern here.
    static u32 cpu_counter = 0;
    cpu_counter += profile_stop();

    rng_set_seed(cpu_counter);
}

u32 rng_get_u32(void)
{
    g_game_vars.rng_step++;
    return rand();
}
