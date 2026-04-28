#include "game/common_ui.h"

#include "game.h"
#include "game/main_menu.h"
#include "game/options_menu.h"

typedef void (*BackgroundRenderCallback)(void);

static enum BackgroundId background = BG_NONE;

// Map to fill in for refactor
static const BackgroundRenderCallback bgCallbacks[] = {
    [BG_NONE] = NULL,
    [BG_CARD_SELECTING] = NULL,
    [BG_CARD_PLAYING] = NULL,
    [BG_ROUND_END] = NULL,
    [BG_SHOP] = NULL,
    [BG_BLIND_SELECT] = NULL,
    [BG_OPTIONS_MENU] = game_options_menu_change_background,
    [BG_MAIN_MENU] = game_main_menu_change_background,
};

void change_background(enum BackgroundId id)
{
    if (id != background && bgCallbacks[id] != NULL)
    {
        bgCallbacks[id]();
    }
    background = id;

    // Can be removed once all states have their own "change_background" func
    change_background_legacy(id);
}
