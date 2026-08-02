/**
 * @file random.h
 *
 * @brief Common functions to handle RNG manipulation. Using this interface has three goals:
 * 1. Make RNG repeatable so that people can share seeds and saves, and end up with the same run
 * 2. Prevent the players from engaging in save-scumming since the pseudo-RNG is deterministic
 * 3. Accomodate for base-36 seeds, so that we can freely chose one with alpha-numeric characters in
 * the seed screen input
 */
#ifndef RANDOM_H
#define RANDOM_H

#include <tonc.h>

/**
 * @brief Keys to different independent RNG sequences
 */
enum RngSequence
{
    RNG_SEQ_CARD_SHUFFLE,
    RNG_SEQ_BLIND,
    RNG_SEQ_SHOP_ITEMS,
    RNG_SEQ_SKIP_TAGS,

    // Each Joker with a random effect has its own independent RNG sequence
    RNG_SEQ_JOKER_MISPRINT,
    RNG_SEQ_JOKER_RESERVED_PARKING,
    RNG_SEQ_JOKER_BUSINESS_CARD,

    // For non-gameplay related things such as sound effects or visual effects, so as to not
    // interfere with important stuff like Shop rolls or Joker effects.
    RNG_SEQ_MISC,

    RNG_SEQ_MAX
};

/**
 * @brief Information to track and restore RNG state
 */
typedef struct
{
    /** Initial seed */
    u32 seed;
    /** Individual states for independent rng sequences */
    u32 states[RNG_SEQ_MAX];
} RngInfo;

/**
 * @brief Starts counting CPU cycles, this will be used by rng_shuffle_seed to
 *         generate a more random seed. To be called once on game start.
 */
void rng_init(void);

/**
 * @brief Accumulates CPU cycles for RNG seed generation, call exaclty once per frame.
 */
void rng_update(void);

/**
 * @brief Set the rng seed to the chosen value, and reset the step counter to 0.
 *         The seed will be capped at `MAX_BASE36` for compatibility with the Seed Input screen
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
 * @brief Get the next "randomly" generated number in the sequence corresponding to the given type.
 *
 * @param key key to the RNG sequence we need to pull a random number from
 *
 * @return u32
 */
u32 rng_get_u32(enum RngSequence key);

/**
 * @brief Restore RNG info struct in the GameVariables. Sets the `seed` and restores the state for
 *         each independent sequence
 *
 * @param info RngInfo struct applied
 *
 * @note Consistency of the RNG after reloading the RngInfo struct has yet to be tested properly
 */
void rng_restore(RngInfo info);

#endif // RANDOM_H
