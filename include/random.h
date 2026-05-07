/**
 * @file random.h
 *
 * @brief Common functions to handle RNG manipulation.
 */
#ifndef RANDOM_H
#define RANDOM_H

#include <tonc.h>

/**
 * @brief Use the global timer and user inputs to try and randomize the RNG seed as
 *         much as possible. This will be called by the main menu and the gmae over
 *         screens so that the next run's seed isn't the same as the last's.
 */
void shuffle_rng_seed(void);

/**
 * @brief Get the next "randomly" generated number in the sequence from the current seed.
 * 
 * @return u32
 */
u32 get_rand(void);

#endif // RANDOM_H
