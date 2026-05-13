/**
 * @file random.h
 *
 * @brief Common functions to handle RNG manipulation.
 */
#ifndef RANDOM_H
#define RANDOM_H

#include <tonc.h>

#define MAX_SEED 0x81BF0FFF // Hex value of "ZZZZZZ" in base 36

/**
 * @brief Starts a counter accumulating CPU cycles that will be used by rng_shuffle_seed to
 *         generate a more random seed.
 */
void rng_start_sampling(void);

/**
 * @brief Simply stops the CPU cycles count in case we don't need it after all.
 */
void rng_cancel_sampling(void);

/**
 * @brief Set the rng seed to the chosen value, and reset the step counter to 0.
 *         The seed will be capped at `MAX_SEED` for compatibility with the Seed Input screen
 *         used to choose a seed for seeded runs.
 *
 * @param seed the new RNG seed 
 */
void rng_set_seed(u32 seed);

/**
 * @brief Uses the CPU cycles counter to randomize the RNG seed as much as possible.
 *         This will be called by the main menu and the game over screens so that
 *         the next run's seed isn't the same as the last's.
 *         rng_start_sampling needs to have been called, and will stop the profiling.
 */
void rng_shuffle_seed(void);

/**
 * @brief Get the next "randomly" generated number in the sequence from the current seed.
 *
 * @return u32
 */
u32 rng_get_u32(void);

#endif // RANDOM_H
