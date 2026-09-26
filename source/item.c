#include "item.h"

#include "item_funcs.h"
#include "mgba_logger.h"
#include "util.h"

#include <tonc.h>

Item* item_roll_new(enum ItemType item_type, enum RngSequence key)
{
    if ((int)item_type < 0 || item_type >= ITEM_NUM_TYPES)
    {
        MGBA_FUNC_ERROR("Invalid type %d", item_type);
        return NULL;
    }

    ItemFuncs* item_funcs = get_item_type_funcs(item_type);

    GBAL_RETURN_IF_NULL_RET(item_funcs, NULL);
    if (item_funcs->roll_new == NULL)
    {
        MGBA_FUNC_ERROR("Unimplemented 'roll_new' function called for item type %d", item_type);
        return NULL;
    }

    return item_funcs->roll_new(key);
}

int item_get_buy_price(Item* item)
{
    GBAL_RETURN_IF_NULL_RET(item, UNDEFINED);

    ItemFuncs* item_funcs = get_item_type_funcs(item->type);
    GBAL_RETURN_IF_NULL_RET(item_funcs, UNDEFINED);
    if (item_funcs->get_buy_price == NULL)
    {
        MGBA_FUNC_ERROR(
            "Unimplemented 'get_buy_price' function called for item type %d",
            item->type
        );
        return UNDEFINED;
    }

    return item_funcs->get_buy_price(item);
}

int item_get_sell_price(Item* item)
{
    GBAL_RETURN_IF_NULL_RET(item, UNDEFINED);

    ItemFuncs* item_funcs = get_item_type_funcs(item->type);
    GBAL_RETURN_IF_NULL_RET(item_funcs, UNDEFINED);
    if (item_funcs->get_sell_price == NULL)
    {
        MGBA_FUNC_ERROR(
            "Unimplemented 'get_sell_price' function called for item type %d",
            item->type
        );
        return UNDEFINED;
    }

    return item_funcs->get_sell_price(item);
}

const char* item_get_name(Item* item)
{
    GBAL_RETURN_IF_NULL_RET(item, ITEM_NAME_DEFAULT);

    ItemFuncs* item_funcs = get_item_type_funcs(item->type);
    GBAL_RETURN_IF_NULL_RET(item_funcs, ITEM_NAME_DEFAULT);
    if (item_funcs->get_name == NULL)
    {
        MGBA_FUNC_ERROR("Unimplemented 'get_name' function called for item type %d", item->type);
        return ITEM_NAME_DEFAULT;
    }

    return item_funcs->get_name(item);
}

ItemSubtypeInfo item_get_subtype_info(Item* item)
{
    ItemSubtypeInfo error_info = ITEM_SUBTYPE_INFO_DEFAULT;

    GBAL_RETURN_IF_NULL_RET(item, error_info);

    ItemFuncs* item_funcs = get_item_type_funcs(item->type);
    GBAL_RETURN_IF_NULL_RET(item_funcs, error_info);
    if (item_funcs->get_subtype_info == NULL)
    {
        MGBA_FUNC_ERROR(
            "Unimplemented 'get_subtype_info' function called for item type %d",
            item->type
        );
        return error_info;
    }

    return item_funcs->get_subtype_info(item);
}

void item_acquire(Item* item)
{
    GBAL_RETURN_IF_NULL_VOID(item);

    ItemFuncs* item_funcs = get_item_type_funcs(item->type);
    GBAL_RETURN_IF_NULL_VOID(item_funcs);
    GBAL_RETURN_IF_NULL_VOID(item_funcs->acquire);
    if (item_funcs->acquire == NULL)
    {
        MGBA_FUNC_ERROR("Unimplemented 'acquire' function called for item type %d", item->type);
        return;
    }

    item_funcs->acquire(item);
}

bool item_can_acquire(Item* item)
{
    GBAL_RETURN_IF_NULL_RET(item, false);
    ItemFuncs* item_funcs = get_item_type_funcs(item->type);
    GBAL_RETURN_IF_NULL_RET(item_funcs, false);
    if (item_funcs->can_acquire == NULL)
    {
        MGBA_FUNC_ERROR("Unimplemented 'can_acquire' function called for item type %d", item->type);
        return false;
    }

    return item_funcs->can_acquire(item);
}

void item_dispose(Item** item)
{
    GBAL_RETURN_IF_NULL_VOID(item);
    GBAL_RETURN_IF_NULL_VOID(*item);
    ItemFuncs* item_funcs = get_item_type_funcs((*item)->type);
    GBAL_RETURN_IF_NULL_VOID(item_funcs);
    if (item_funcs->dispose == NULL)
    {
        MGBA_FUNC_ERROR("Unimplemented 'dispose' function called for item type %d", (*item)->type);
        return;
    }

    item_funcs->dispose(item);
}

void item_print_buy_price_under(Item* item)
{
    GBAL_RETURN_IF_NULL_VOID(item);
    sprite_object_print_price_under((SpriteObject*)item, item_get_buy_price(item));
}

int item_print_description(Item* item, Rect dest_rect)
{
    GBAL_RETURN_IF_NULL_RET(item, 0);
    ItemFuncs* item_funcs = get_item_type_funcs(item->type);
    GBAL_RETURN_IF_NULL_RET(item_funcs, 0);
    if (item_funcs->print_description == NULL)
    {
        MGBA_FUNC_ERROR(
            "Unimplemented 'print_description' function called for item type %d",
            item->type
        );
        return 0;
    }

    return item_funcs->print_description(item, dest_rect);
}