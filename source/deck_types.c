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
// clang-format off
static const PrintDescCallback deck_description_functions[DECK_TYPE_MAX] = {
    [DECK_TYPE_RED]     = print_desc_red_deck,
    [DECK_TYPE_BLUE]    = print_desc_blue_deck,
    [DECK_TYPE_YELLOW]  = print_desc_yellow_deck,
    [DECK_TYPE_GREEN]   = print_desc_green_deck,
    [DECK_TYPE_BLACK]   = print_desc_black_deck,
    [DECK_TYPE_PAINTED] = print_desc_painted_deck
};
// clang-format on

static void red_deck_effect(enum DeckEvent event);
static void blue_deck_effect(enum DeckEvent event);
static void yellow_deck_effect(enum DeckEvent event);
static void black_deck_effect(enum DeckEvent event);
static void painted_deck_effect(enum DeckEvent event);

typedef void (*DeckEffectCallback)(enum DeckEvent);
// clang-format off
static const DeckEffectCallback deck_effect_functions[DECK_TYPE_MAX] = {
    [DECK_TYPE_RED]     = red_deck_effect,
    [DECK_TYPE_BLUE]    = blue_deck_effect,
    [DECK_TYPE_YELLOW]  = yellow_deck_effect,
    [DECK_TYPE_GREEN]   = NULL, // Green Deck is special
    [DECK_TYPE_BLACK]   = black_deck_effect,
    [DECK_TYPE_PAINTED] = painted_deck_effect
};
// clang-format on

void deck_print_name(enum DeckType deck, BG_POINT pos)
{
    tte_printf("#{P:%d,%d; cx:0x%X000}%s", pos.x, pos.y, TTE_WHITE_PB, DECK_NAMES[deck]);
}

void deck_print_description(enum DeckType deck, Rect dest_rect)
{
    if (deck_description_functions[deck] == NULL)
        return;
    deck_description_functions[deck](dest_rect);
}

void deck_apply_effect_for_event(enum DeckType deck, enum DeckEvent event)
{
    if (deck_effect_functions[deck] == NULL)
        return;
    deck_effect_functions[deck](event);
}

// Print Deck functions

static void print_desc_red_deck(Rect dest_rect)
{
    static const char desc[] = "\n" TTE_RED_TAG "+1 " TTE_BLACK_TAG "discard every Round";
    tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static void print_desc_blue_deck(Rect dest_rect)
{
    static const char desc[] = "\n" TTE_BLUE_TAG "+1 " TTE_BLACK_TAG "hand every Round";
    (void)tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static void print_desc_yellow_deck(Rect dest_rect)
{
    static const char desc[] = "\n" TTE_BLACK_TAG "Start with an extra " TTE_YELLOW_TAG "$10";
    (void)tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static void print_desc_green_deck(Rect dest_rect)
{
    static const char desc[] =
        TTE_BLACK_TAG "End of Round:\n\n-" TTE_YELLOW_TAG "$2 " TTE_BLACK_TAG "per " TTE_BLUE_TAG
                      "Hand" TTE_BLACK_TAG "\n-" TTE_YELLOW_TAG "$2 " TTE_BLACK_TAG
                      "per " TTE_RED_TAG "Disc." TTE_BLACK_TAG "\n-No " TTE_YELLOW_TAG "Interest";
    (void)tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_LEFT, SCREEN_LEFT, true);
}

static void print_desc_black_deck(Rect dest_rect)
{
    static const char desc[] = TTE_YELLOW_TAG "\n+1 " TTE_BLACK_TAG "Joker slot\n" TTE_BLUE_TAG
                                              "-1 " TTE_BLACK_TAG "hand every Round";
    (void)tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static void print_desc_painted_deck(Rect dest_rect)
{
    static const char desc[] = TTE_YELLOW_TAG "\n+2 " TTE_BLACK_TAG "Hand size\n" TTE_RED_TAG
                                              "-1 " TTE_BLACK_TAG "Joker slot";
    (void)tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

// Deck Effect functions

static void red_deck_effect(enum DeckEvent event)
{
    switch (event)
    {
        case DECK_EVENT_ON_GAME_START:
        case DECK_EVENT_ON_ROUND_END:
            g_game_vars.discards = BASE_DISCARDS + 1;
            break;

        default:
            break;
    }
}

static void blue_deck_effect(enum DeckEvent event)
{
    switch (event)
    {
        case DECK_EVENT_ON_GAME_START:
        case DECK_EVENT_ON_ROUND_END:
            g_game_vars.hands = BASE_HANDS + 1;
            break;

        default:
            break;
    }
}

static void yellow_deck_effect(enum DeckEvent event)
{
    switch (event)
    {
        case DECK_EVENT_ON_GAME_START:
            g_game_vars.money = STARTING_MONEY + 10;
            break;

        default:
            break;
    }
}

static void black_deck_effect(enum DeckEvent event)
{
    switch (event)
    {
        case DECK_EVENT_ON_GAME_START:
            g_game_vars.nb_held_jokers = BASE_JOKERS_HELD_SIZE + 1;
            g_game_vars.hands = BASE_HANDS - 1;
            break;

        case DECK_EVENT_ON_ROUND_END:
            g_game_vars.hands = BASE_HANDS - 1;
            break;

        default:
            break;
    }
}

static void painted_deck_effect(enum DeckEvent event)
{
    switch (event)
    {
        case DECK_EVENT_ON_GAME_START:
            g_game_vars.nb_held_jokers = BASE_JOKERS_HELD_SIZE - 1;
            g_game_vars.hand_size = DEFAULT_HAND_SIZE + 2;
            break;

        case DECK_EVENT_ON_ROUND_END:
            g_game_vars.hand_size = DEFAULT_HAND_SIZE + 2;
            break;

        default:
            break;
    }
}
