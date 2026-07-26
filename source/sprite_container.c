#include "sprite_container.h"

#include "sprite.h"

void layout_container_update(LayoutContainer* container)
{
    POINT pos_size;
    int max_length;
    int start_pos;

    if (container->direction == LAYOUT_DIR_HORIZONTAL)
    {
        pos_size.x = container->sprite_pos_size.left;
        pos_size.y = container->sprite_pos_size.right;

        max_length = container->pos.right;
        start_pos = container->pos.left;
    }
    else
    {
        pos_size.x = container->sprite_pos_size.top;
        pos_size.y = container->sprite_pos_size.bottom;

        max_length = container->pos.bottom;
        start_pos = container->pos.top;
    }

    int nb_sprites = list_get_len(container->contents);
    int spacing = container->minimum_spacing;
    int naive_length =
        nb_sprites * pos_size.y + (nb_sprites - 1) * spacing;

    int overrun = naive_length - max_length;

    // If sprites take too much space, correct the spacing
    if (overrun > 0)
    {
        spacing -= overrun / (nb_sprites - 1);
    }
    // If they fit inside the container, correct the starting point if they need to be centered
    else if (container->justification == LAYOUT_JUST_CENTER)
    {
        start_pos -= overrun / 2;
    }

    // Set sprite positions
    SpriteObject* sprite_object = NULL;
    ListItr itr = list_itr_create(container->contents);

    while ((sprite_object = list_itr_next(&itr)))
    {
        FIXED* coord = (container->direction == LAYOUT_DIR_HORIZONTAL) ? &sprite_object->tx : &sprite_object->ty;
        *coord = int2fx(start_pos - pos_size.x);

        start_pos += pos_size.y + spacing;
    }
}