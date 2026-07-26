/**
 * @file sprite_container.h
 * @brief Definitions for the LayoutContainer data structure and its handling functions
 */

#ifndef SPRITE_CONTAINER_H
#define SPRITE_CONTAINER_H

#include "graphic_utils.h"
#include "list.h"

enum LayoutDirection
{
    LAYOUT_DIR_HORIZONTAL,
    LAYOUT_DIR_VERTICAL,
    LAYOUT_DIR_MAX
};

enum LayoutJustification
{
    LAYOUT_JUST_BEGIN,
    LAYOUT_JUST_CENTER
};

/**
 * @brief A structure organizing a List of SpriteObjects
 */
typedef struct LayoutContainer
{
    /**
     * @brief A Rect holding the position and size of the Container as follows:
     *         `{left=posX, top=posY, right=width, bottom=height}`
     */
    Rect pos;

    /**
     * @brief Whether the sprites are to be organized horizontally or vertically
     */
    enum LayoutDirection direction;

    /**
     * @brief Determines the alignment of the sprites
     */
    enum LayoutJustification justification;

    /**
     * @brief List of SpriteObjects to organize
     */
    List* contents;

    /**
     * @brief The position and size of the object actually represented within their Sprite
     *
     * For the Skip Tags, the sprite is 16x16 ps, but the actual object is only 10x10, which means
     * that `real_pos_size` will be `{2, 2, 10, 10}`
     *
     * The Sprites should all be uniform within a single Container, hence why we'll save memory space
     * by storing the information here instead of in each Sprite
     */
    Rect sprite_pos_size;

    /**
     * @brief By how much do we need to space the Sprites, in case there are not enough to fill the
     *         whole Container.
     */
    int minimum_spacing;
} LayoutContainer;

/**
 * @brief Parse the contents List and give a new target position to every SpriteObject inside
 *
 * Do not run this on every frame, only when the List's contents actually change
 *
 * @param container the LayoutContainer to update
 */
void layout_container_update(LayoutContainer* container);

#define LAYOUT_CONTAINER_DEFAULT             \
    {                                        \
        .pos = {0, 0, 1, 1},                 \
        .direction = LAYOUT_DIR_HORIZONTAL,  \
        .justification = LAYOUT_JUST_CENTER, \
        .contents = NULL                     \
        .real_pos_size = {0, 0, 1, 1}        \
        .minimum_spacing = 0                 \
}

#endif // SPRITE_CONTAINER_H
