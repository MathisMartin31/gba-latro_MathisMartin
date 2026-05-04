/**
 * @file layout.h
 *
 * @brief Header of shared rects and points needed for ui
 */
#ifndef LAYOUT_H
#define LAYOUT_H

#include "graphic_utils.h"

#define MENU_POP_OUT_ANIM_FRAMES 20

// clang-format off
static const BG_POINT JOKER_DISCARD_TARGET = {240,  30};
static const BG_POINT HELD_JOKERS_POS      = {108,  10};

static const Rect POP_MENU_ANIM_RECT       = {9,    7,   24,   31 };
static const Rect TOP_LEFT_PANEL_ANIM_RECT = {0,    0,    8,    4 };
// clang-format on

#endif // LAYOUT_H
