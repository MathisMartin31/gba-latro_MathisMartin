/**
 * @file deck_types.h
 *
 * @brief Functions related to the different types of Decks.
 */
#ifndef DECK_TYPES_H
#define DECK_TYPES_H

#include "graphic_utils.h"

#define DECK_NAME_LENGTH 14 // 13 chars + '\0'
#define DECK_DESC_WIDTH  DECK_NAME_LENGTH
#define DECK_DESC_HEIGHT 5

enum DeckType
{
    DECK_TYPE_RED,
    DECK_TYPE_BLUE,
    DECK_TYPE_YELLOW,
    DECK_TYPE_GREEN,
    DECK_TYPE_BLACK,
    DECK_TYPE_PAINTED,
    DECK_TYPE_MAX
};

enum DeckEvent
{
    DECK_EVENT_ON_GAME_START,
    DECK_EVENT_ON_ROUND_END,
    DECK_EVENT_MAX
};

/**
 * @brief Prints the Deck name string to the screen at the given position.
 *
 * @param deck Decktype we want the name of.
 * @param pos screen position in pixels to print the name at.
 */
void deck_print_name(enum DeckType deck, BG_POINT pos);

/**
 * @brief Prints the Deck description string to the screen inside the given rectangle region.
 *
 * @param deck DeckType we want the description of.
 * @param dest_rect rectangle to display the Deck description into
 */
void deck_print_description(enum DeckType deck, Rect dest_rect);

void deck_apply_effect_for_event(enum DeckType deck, enum DeckEvent event);

#endif // DECK_TYPES_H
