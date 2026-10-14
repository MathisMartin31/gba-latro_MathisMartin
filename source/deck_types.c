/**
 * @file deck_types.c
 *
 * @brief Implementation of functions related to the different types of Decks.
 */
#include "deck_types.h"

#include "game.h"

#include <tonc.h>

#define TTE_COLOR_TEXT_FORMAT "#{cx:0x%X000}%s"

/**
 * @brief List of DeckType names, all formatted to be centered in a string of 13 characters.
 */
static const char DECK_NAMES[DECK_TYPE_MAX][DECK_NAME_LENGTH] = {
    "  Red Deck   ",
    "  Blue Deck  ",
    " Yellow Deck ",
    " Green Deck  ",
    " Black Deck  ",
    "Painted Deck "
};

static void print_desc_red_deck(Rect dest_rect);
static void print_desc_blue_deck(Rect dest_rect);
static void print_desc_yellow_deck(Rect dest_rect);
static void print_desc_green_deck(Rect dest_rect);
static void print_desc_black_deck(Rect dest_rect);
static void print_desc_painted_deck(Rect dest_rect);

typedef void (*PrintDescCallback)(Rect);
static const PrintDescCallback deck_description_functions[DECK_TYPE_MAX] = {
    print_desc_red_deck,
    print_desc_blue_deck,
    print_desc_yellow_deck,
    print_desc_green_deck,
    print_desc_black_deck,
    print_desc_painted_deck
};

void print_deck_name(enum DeckType deck, BG_POINT pos)
{
    tte_printf("#{P:%d,%d; cx:0x%X000}%s", pos.x, pos.y, TTE_WHITE_PB, DECK_NAMES[deck]);
}

void print_deck_description(enum DeckType deck, Rect dest_rect)
{
    if (deck_description_functions[deck] == NULL)
        return;

    deck_description_functions[deck](dest_rect);
}

/**
 * @brief Print the decription of the Red Deck
 * ```
 * "             "
 * " +1 discard  "
 * " every Round "
 * "             "
 * "             "
 * ```
 * @param pos position of the rectagle to print the description to
 */
static void print_desc_red_deck(Rect dest_rect)
{
    static const char desc[] = "\n" TTE_RED_TAG "+1 " TTE_BLACK_TAG "discard every Round";
    tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

/**
 * @brief Print the decription of the Blue Deck
 * ```
 * "             "
 * "   +1 hand   "
 * " every Round "
 * "             "
 * "             "
 * ```
 * @param pos position of the rectagle to print the description to
 */
static void print_desc_blue_deck(Rect dest_rect)
{
    static const char desc[] = "\n" TTE_BLUE_TAG "+1 " TTE_BLACK_TAG "hand every Round";
    (void)tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

/**
 * @brief Print the decription of the Yellow Deck
 * ```
 * "             "
 * " Start with  "
 * "an extra $10 "
 * "             "
 * "             "
 * ```
 * @param pos position of the rectagle to print the description to
 */
static void print_desc_yellow_deck(Rect dest_rect)
{
    static const char desc[] = "\n" TTE_BLACK_TAG "Start with an extra " TTE_YELLOW_TAG "$10";
    (void)tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

/**
 * @brief Print the decription of the Green Deck
 * ```
 * "End of Round:"
 * "             "
 * "-$2 per Hand "
 * "-$2 per Disc."
 * "-No Interest "
 * ```
 * @param pos position of the rectagle to print the description to
 */
static void print_desc_green_deck(Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "End of Round:\n\n-" TTE_YELLOW_TAG "$2 " TTE_BLACK_TAG "per " TTE_BLUE_TAG
                      "Hand" TTE_BLACK_TAG "\n-" TTE_YELLOW_TAG "$2 " TTE_BLACK_TAG
                      "per " TTE_RED_TAG "Disc." TTE_BLACK_TAG "\n-No " TTE_YELLOW_TAG "Interest";
    (void)tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_LEFT, SCREEN_LEFT, true);
}

/**
 * @brief Print the decription of the Black Deck
 * ```
 * "             "
 * "+1 Joker slot"
 * "   -1 hand   "
 * " every Round "
 * "             "
 * ```
 * @param pos position of the rectagle to print the description to
 */
static void print_desc_black_deck(Rect dest_rect)
{
    static const char desc[] = TTE_YELLOW_TAG "\n+1 " TTE_BLACK_TAG "Joker slot\n" TTE_BLUE_TAG
                                              "-1 " TTE_BLACK_TAG "hand every Round";
    (void)tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

/**
 * @brief Print the decription of the Painted Deck
 * ```
 * "             "
 * "+2 Hand size "
 * "-1 Joker slot"
 * "             "
 * "             "
 * ```
 * @param pos position of the rectagle to print the description to
 */
static void print_desc_painted_deck(Rect dest_rect)
{
    static const char desc[] = TTE_YELLOW_TAG "\n+2 " TTE_BLACK_TAG "Hand size\n" TTE_RED_TAG
                                              "-1 " TTE_BLACK_TAG "Joker slot";
    (void)tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}
