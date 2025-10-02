#include "game.h"
#include "joker.h"
#include "util.h"
#include "hand_analysis.h"
#include "list.h"
#include <stdlib.h>

static JokerEffect default_joker_effect(Joker *joker, Card *scored_card) {
    JokerEffect effect = {0};
    if (scored_card == NULL) effect.mult = 4;
    return effect;
}

static JokerEffect sinful_joker_effect(Card *scored_card, u8 sinful_suit) {
    JokerEffect effect = {0};
    if (scored_card != NULL && scored_card->suit == sinful_suit)
        effect.mult = 3;
    return effect;
}

static JokerEffect greedy_joker_effect(Joker *joker, Card *scored_card) {
    return sinful_joker_effect(scored_card, DIAMONDS);
}

static JokerEffect lusty_joker_effect(Joker *joker, Card *scored_card) {
    return sinful_joker_effect(scored_card, HEARTS);
}

static JokerEffect wrathful_joker_effect(Joker *joker, Card *scored_card) {
    return sinful_joker_effect(scored_card, SPADES);
}

static JokerEffect gluttonous_joker_effect(Joker *joker, Card *scored_card) {
    return sinful_joker_effect(scored_card, CLUBS);
}

static JokerEffect jolly_joker_effect(Joker *joker, Card *scored_card) {
    JokerEffect effect = {0};
    if (scored_card != NULL)
        return effect; // if card != null, we are not at the end-phase of scoring yet

    // This is really inefficient but the only way at the moment to check for whole-hand conditions
    u8 suits[NUM_SUITS];
    u8 ranks[NUM_RANKS];
    get_played_distribution(ranks, suits);

    if (hand_contains_n_of_a_kind(ranks) >= 2)
        effect.mult = 8;
    return effect;
}

static JokerEffect zany_joker_effect(Joker *joker, Card *scored_card) {
    JokerEffect effect = {0};
    if (scored_card != NULL)
        return effect; // if card != null, we are not at the end-phase of scoring yet

    u8 suits[NUM_SUITS];
    u8 ranks[NUM_RANKS];
    get_played_distribution(ranks, suits);

    if (hand_contains_n_of_a_kind(ranks) >= 3)
        effect.mult = 12;
    return effect;
}

static JokerEffect mad_joker_effect(Joker *joker, Card *scored_card) {
    JokerEffect effect = {0};
    if (scored_card != NULL)
        return effect; // if card != null, we are not at the end-phase of scoring yet

    u8 suits[NUM_SUITS];
    u8 ranks[NUM_RANKS];
    get_played_distribution(ranks, suits);

    if (hand_contains_two_pair(ranks))
        effect.mult = 10;
    return effect;
}

static JokerEffect crazy_joker_effect(Joker *joker, Card *scored_card) {
    JokerEffect effect = {0};
    if (scored_card != NULL)
        return effect; // if card != null, we are not at the end-phase of scoring yet

    u8 suits[NUM_SUITS];
    u8 ranks[NUM_RANKS];
    get_played_distribution(ranks, suits);

    if (hand_contains_straight(ranks))
        effect.mult = 12;
    return effect;
}

static JokerEffect droll_joker_effect(Joker *joker, Card *scored_card) {
    JokerEffect effect = {0};
    if (scored_card != NULL)
        return effect; // if card != null, we are not at the end-phase of scoring yet

    u8 suits[NUM_SUITS];
    u8 ranks[NUM_RANKS];
    get_played_distribution(ranks, suits);

    if (hand_contains_flush(suits))
        effect.mult = 10;
    return effect;
}

static JokerEffect sly_joker_effect(Joker *joker, Card *scored_card) {
    JokerEffect effect = {0};
    if (scored_card != NULL)
        return effect; // if card != null, we are not at the end-phase of scoring yet

    u8 suits[NUM_SUITS];
    u8 ranks[NUM_RANKS];
    get_played_distribution(ranks, suits);

    if (hand_contains_n_of_a_kind(ranks) >= 2)
        effect.chips = 50;
    return effect;
}

static JokerEffect wily_joker_effect(Joker *joker, Card *scored_card) {
    JokerEffect effect = {0};
    if (scored_card != NULL)
        return effect; // if card != null, we are not at the end-phase of scoring yet

    u8 suits[NUM_SUITS];
    u8 ranks[NUM_RANKS];
    get_played_distribution(ranks, suits);

    if (hand_contains_n_of_a_kind(ranks) >= 3)
        effect.chips = 100;
    return effect;
}

static JokerEffect clever_joker_effect(Joker *joker, Card *scored_card) {
    JokerEffect effect = {0};
    if (scored_card != NULL)
        return effect; // if card != null, we are not at the end-phase of scoring yet

    u8 suits[NUM_SUITS];
    u8 ranks[NUM_RANKS];
    get_played_distribution(ranks, suits);

    if (hand_contains_two_pair(ranks))
        effect.chips = 80;
    return effect;
}

static JokerEffect devious_joker_effect(Joker *joker, Card *scored_card) {
    JokerEffect effect = {0};
    if (scored_card != NULL)
        return effect; // if card != null, we are not at the end-phase of scoring yet

    u8 suits[NUM_SUITS];
    u8 ranks[NUM_RANKS];
    get_played_distribution(ranks, suits);

    if (hand_contains_straight(ranks))
        effect.chips = 100;
    return effect;
}

static JokerEffect crafty_joker_effect(Joker *joker, Card *scored_card) {
    JokerEffect effect = {0};
    if (scored_card != NULL)
        return effect; // if card != null, we are not at the end-phase of scoring yet

    u8 suits[NUM_SUITS];
    u8 ranks[NUM_RANKS];
    get_played_distribution(ranks, suits);

    if (hand_contains_flush(suits))
        effect.chips = 80;
    return effect;
}

static JokerEffect half_joker_effect(Joker *joker, Card *scored_card) {
    JokerEffect effect = {0};
    if (scored_card != NULL)
        return effect; // if card != null, we are not at the end-phase of scoring yet

    int played_size = get_played_top() + 1;
    if (played_size <= 3) 
        effect.mult = 20;

    return effect;
}

static JokerEffect joker_stencil_effect(Joker *joker, Card *scored_card) {
    JokerEffect effect = {0};
    if (scored_card != NULL)
        return effect; // if card != null, we are not at the end-phase of scoring yet

    List* jokers = get_jokers();

    // +1 xmult per empty joker slot...
    int num_jokers = list_get_size(jokers);

    effect.xmult = (MAX_JOKERS_HELD_SIZE) - num_jokers;

    // ...and also each stencil_joker adds +1 xmult
    
    for (int i = 0; i < num_jokers; i++ )
    {
        JokerObject* joker_object = list_get(jokers, i);
        if (joker_object->joker->id == JOKER_STENCIL_ID)
            effect.xmult++;
    }

    return effect;
}

#define MISPRINT_MAX_MULT 23
static JokerEffect misprint_joker_effect(Joker *joker, Card *scored_card) {
    JokerEffect effect = {0};
    if (scored_card != NULL)
        return effect; // if card != null, we are not at the end-phase of scoring yet

    effect.mult = random() % (MISPRINT_MAX_MULT + 1);

    return effect;
}

static JokerEffect walkie_talkie_joker_effect(Joker *joker, Card *scored_card) {
    JokerEffect effect = {0};
    if (scored_card == NULL)
        return effect;

    if (scored_card->rank == TEN || scored_card->rank == FOUR) {
            effect.chips = 10;
            effect.mult = 4;
    }

    return effect;
}

static JokerEffect fibonnaci_joker_effect(Joker *joker, Card *scored_card) {
    JokerEffect effect = {0};
    if (scored_card == NULL)
        return effect;

    switch (scored_card->rank) {
        case ACE: case TWO: case THREE: case FIVE: case EIGHT:
            effect.mult = 8;
        default:
            break;
    }

    return effect;
}

static JokerEffect banner_joker_effect(Joker *joker, Card *scored_card) {
    JokerEffect effect = {0};
    if (scored_card != NULL)
        return effect; // if card != null, we are not at the end-phase of scoring yet

    effect.chips = 30 * get_num_discards_remaining();

    return effect;
}

static JokerEffect mystic_summit_joker_effect(Joker *joker, Card *scored_card) {
    JokerEffect effect = {0};
    if (scored_card != NULL)
        return effect; // if card != null, we are not at the end-phase of scoring yet

    if (get_num_discards_remaining() == 0)
        effect.mult = 15;

    return effect;
}

static JokerEffect blackboard_joker_effect(Joker *joker, Card *scored_card) {
    JokerEffect effect = {0};
    if (scored_card != NULL)
        return effect; // if card != null, we are not at the end-phase of scoring yet

    bool all_cards_are_spades_or_clubs = true;
    CardObject** hand = get_hand_array();
    int hand_size = hand_get_size();
    for (int i = 0; i < hand_size; i++ )
    {
        u8 suit = hand[i]->card->suit;
        if (suit == HEARTS || suit == DIAMONDS) {
            all_cards_are_spades_or_clubs = false;
            break;
        }
    }

    if (all_cards_are_spades_or_clubs)
        effect.xmult = 3;

    return effect;
}

static JokerEffect blue_joker_effect(Joker *joker, Card *scored_card) {
    JokerEffect effect = {0};
    if (scored_card != NULL)
        return effect; // if card != null, we are not at the end-phase of scoring yet

    effect.chips = (get_deck_top() + 1) * 2;

    return effect;
}

// Using __attribute__((unused)) for jokers with no sprites yet to avoid warning
// Remove the attribute once they have sprites
__attribute__((unused))
static JokerEffect raised_fist_joker_effect(Joker *joker, Card *scored_card) 
{
    JokerEffect effect = {0};
    if (scored_card != NULL)
        return effect; // if card != null, we are not at the end-phase of scoring yet

    // Find the lowest rank card in hand
    // Aces are always considered high value, even in an ace-low straight
    u8 lowest_value = IMPOSSIBLY_HIGH_CARD_VALUE;
    CardObject** hand = get_hand_array();
    int hand_size = hand_get_size();
    for (int i = 0; i < hand_size; i++ )
    {
        u8 value = card_get_value(hand[i]->card);
        if (lowest_value > value)
            lowest_value = value;
    }

    if (lowest_value != IMPOSSIBLY_HIGH_CARD_VALUE)
        effect.mult = lowest_value * 2;

    return effect;
} 

__attribute__((unused))
static JokerEffect reserved_parking_joker_effect(Joker *joker, Card *scored_card) {
    JokerEffect effect = {0};
    if (scored_card != NULL)
        return effect; // if card != null, we are not at the end-phase of scoring yet

    CardObject** hand = get_hand_array();
    int hand_size = hand_get_size();
    for (int i = 0; i < hand_size; i++ )
    {
        if ((random() % 2 == 0) && card_is_face(hand[i]->card)) {
            effect.money += 1;
        }
    }

    return effect;
};

static JokerEffect business_card_joker_effect(Joker *joker, Card *scored_card) {
    JokerEffect effect = {0};
    if (scored_card == NULL)
        return effect;

    if ((random() % 2 == 0) && card_is_face(scored_card)) {
        effect.money = 2;
    }

    return effect;
}

static JokerEffect scholar_joker_effect(Joker *joker, Card *scored_card) {
    JokerEffect effect = {0};
    if (scored_card == NULL)
        return effect;

    if (scored_card->rank == ACE) {
        effect.chips = 20;
        effect.mult = 4;
    }

    return effect;
}

static JokerEffect scary_face_joker_effect(Joker *joker, Card *scored_card) {
    JokerEffect effect = {0};
    if (scored_card == NULL)
        return effect;

    if (card_is_face(scored_card)) {
        effect.chips = 30;
    }

    return effect;
}

 __attribute__((unused))
static JokerEffect abstract_joker_effect(Joker *joker, Card *scored_card) {
    JokerEffect effect = {0};
    if (scored_card != NULL)
        return effect; // if card != null, we are not at the end-phase of scoring yet

    // +1 xmult per occupied joker slot
    int num_jokers = list_get_size(get_jokers());
    effect.mult = num_jokers * 3;

    return effect;
}

__attribute__((unused))
static JokerEffect bull_joker_effect(Joker *joker, Card *scored_card) {
    JokerEffect effect = {0};
    if (scored_card != NULL)
        return effect; // if card != null, we are not at the end-phase of scoring yet

    effect.chips = get_money() * 2;

    return effect;
}

static JokerEffect smiley_face_joker_effect(Joker *joker, Card *scored_card) {
    JokerEffect effect = {0};
    if (scored_card == NULL)
        return effect;

    if (card_is_face(scored_card)) {
        effect.mult = 5;
    }

    return effect;
}

static JokerEffect even_steven_joker_effect(Joker *joker, Card *scored_card) {
    JokerEffect effect = {0};
    if (scored_card == NULL)
        return effect;

    if (card_get_value(scored_card) % 2 == 0) {
        switch (scored_card->rank) {
            case KING: case QUEEN: case JACK:
                break;
            default:
                effect.mult = 4;
        }
    }

    return effect;
}

static JokerEffect odd_todd_joker_effect(Joker *joker, Card *scored_card) {
    JokerEffect effect = {0};
    if (scored_card == NULL)
        return effect;

    if (card_get_value(scored_card) % 2 == 1) // todo test ace
        effect.chips = 31;

    return effect;
}

__attribute__((unused))
static JokerEffect acrobat_joker_effect(Joker *joker, Card *scored_card) {
    JokerEffect effect = {0};
    if (scored_card == NULL)
        return effect;

    // 0 remaining hands mean we're scoring the last hand
    if (get_num_hands_remaining() == 0) {
        effect.xmult = 3;
    }

    return effect;
}

/* The index of a joker in the registry matches its ID.
 * The joker sprites are matched by ID so the position in the registry
 * determines the joker's sprite.
 * Each consecutive NUM_JOKERS_PER_SPRITESHEET (defined in joker.c) jokers
 * share a spritesheet and thus a color palette.
 * To make better use of color palettes jokers may be rearranged here
 * (and put together in the matching spritesheet) to share a color palette.
 * Otherwise the order is similar to the wiki.
 */
const JokerInfo joker_registry[] = {
    { COMMON_JOKER,   2, NULL, NULL, default_joker_effect, NULL, NULL, NULL, NULL },  // DEFAULT_JOKER_ID = 0
    { COMMON_JOKER,   5, greedy_joker_effect, NULL, NULL, NULL, NULL, NULL, NULL },   // GREEDY_JOKER_ID  = 1
    { COMMON_JOKER,   5, lusty_joker_effect, NULL, NULL, NULL, NULL, NULL, NULL },    // etc...  2
    { COMMON_JOKER,   5, wrathful_joker_effect, NULL, NULL, NULL, NULL, NULL, NULL },         // 3
    { COMMON_JOKER,   5, gluttonous_joker_effect, NULL, NULL, NULL, NULL, NULL, NULL },       // 4
    { COMMON_JOKER,   3, NULL, NULL, jolly_joker_effect, NULL, NULL, NULL, NULL },            // 5
    { COMMON_JOKER,   4, NULL, NULL, zany_joker_effect, NULL, NULL, NULL, NULL },             // 6
    { COMMON_JOKER,   4, NULL, NULL, mad_joker_effect, NULL, NULL, NULL, NULL },              // 7
    { COMMON_JOKER,   4, NULL, NULL, crazy_joker_effect, NULL, NULL, NULL, NULL },            // 8
    { COMMON_JOKER,   4, NULL, NULL, droll_joker_effect, NULL, NULL, NULL, NULL },            // 9
    { COMMON_JOKER,   3, NULL, NULL, sly_joker_effect, NULL, NULL, NULL, NULL },              // 10
    { COMMON_JOKER,   4, NULL, NULL, wily_joker_effect, NULL, NULL, NULL, NULL },             // 11
    { COMMON_JOKER,   4, NULL, NULL, clever_joker_effect, NULL, NULL, NULL, NULL },           // 12
    { COMMON_JOKER,   4, NULL, NULL, devious_joker_effect, NULL, NULL, NULL, NULL },          // 13
    { COMMON_JOKER,   4, NULL, NULL, crafty_joker_effect, NULL, NULL, NULL, NULL },           // 14
    { COMMON_JOKER,   5, NULL, NULL, half_joker_effect, NULL, NULL, NULL, NULL },             // 15
    { UNCOMMON_JOKER, 8, NULL, NULL, joker_stencil_effect, NULL, NULL, NULL, NULL },          // 16
    { COMMON_JOKER,   5, NULL, NULL, banner_joker_effect, NULL, NULL, NULL, NULL },           // 17
    { COMMON_JOKER,   4, walkie_talkie_joker_effect, NULL, NULL, NULL, NULL, NULL, NULL },    // 18
    { UNCOMMON_JOKER, 8, fibonnaci_joker_effect, NULL, NULL, NULL, NULL, NULL, NULL },        // 19
    { UNCOMMON_JOKER, 6, NULL, NULL, blackboard_joker_effect, NULL, NULL, NULL, NULL },       // 20
    { COMMON_JOKER,   5, NULL, NULL, mystic_summit_joker_effect, NULL, NULL, NULL, NULL },    // 21
    { COMMON_JOKER,   4, NULL, NULL, misprint_joker_effect, NULL, NULL, NULL, NULL },         // 22
    { COMMON_JOKER,   4, even_steven_joker_effect, NULL, NULL, NULL, NULL, NULL, NULL },      // 23
    { COMMON_JOKER,   5, NULL, NULL, blue_joker_effect, NULL, NULL, NULL, NULL },             // 24
    { COMMON_JOKER,   4, odd_todd_joker_effect, NULL, NULL, NULL, NULL, NULL, NULL },         // 25
    { COMMON_JOKER,   4, scholar_joker_effect, NULL, NULL, NULL, NULL, NULL, NULL },          // 26
    { COMMON_JOKER,   4, business_card_joker_effect, NULL, NULL, NULL, NULL, NULL, NULL },    // 27
    // Business card should be paired with Shortcut for palette optimization when it's added
    { COMMON_JOKER,   4, scary_face_joker_effect, NULL, NULL, NULL, NULL, NULL, NULL },       // 28
    { COMMON_JOKER,   4, smiley_face_joker_effect, NULL, NULL, NULL, NULL, NULL, NULL },      // 29

    // The following jokers don't have sprites yet, 
    // uncomment them when their sprites are added.
#if 0
    { COMMON_JOKER,   5, NULL, raised_fist_joker_effect, NULL, NULL, NULL, NULL, NULL },
    { COMMON_JOKER,   6, NULL, reserved_parking_joker_effect, NULL, NULL, NULL, NULL, NULL },

    { COMMON_JOKER,   4, NULL, NULL, abstract_joker_effect, NULL, NULL, NULL, NULL },
    { UNCOMMON_JOKER, 6, NULL, NULL, bull_joker_effect, NULL, NULL, NULL, NULL},
    { UNCOMMON_JOKER, 6, NULL, NULL, acrobat_joker_effect, NULL, NULL, NULL, NULL },
#endif
};

static const size_t joker_registry_size = NUM_ELEM_IN_ARR(joker_registry);

const JokerInfo* get_joker_registry_entry(int joker_id) {
    if (joker_id < 0 || (size_t)joker_id >= joker_registry_size) {
        return NULL;
    }
    return &joker_registry[joker_id];
}

size_t get_joker_registry_size(void) {
    return joker_registry_size;
}
