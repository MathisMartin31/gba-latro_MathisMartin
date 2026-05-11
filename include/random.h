/**
 * @file random.h
 *
 * @brief Common functions to handle RNG manipulation.
 */
#ifndef RANDOM_H
#define RANDOM_H

#include <tonc.h>

/**
 * @brief Initialized the internal structures used to try and generate better randomness.
 *         Must only be called once on boot.
 */
void rng_init(void);

/**
 * @brief Use the global timer and user inputs to try and randomize the RNG seed as
 *         much as possible. This will be called by the main menu and the gmae over
 *         screens so that the next run's seed isn't the same as the last's.
 */
void rng_shuffle_seed(void);

/**
 * @brief Get the next "randomly" generated number in the sequence from the current seed.
 *
 * @return u32
 */
u32 rng_get_u32(void);

#endif // RANDOM_H
