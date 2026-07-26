
enum LayoutDirection
{
    LAYOUT_DIR_HORIZONTAL,
    LAYOUT_DIR_VERTICAL,
    LAYOUT_DIR_MAX
};

enum LayoutJustification
{
    LAYOUT_JUST_LEFT,
    LAYOUT_JUST_CENTER
}

typedef struct LayoutContainer
{
    Rect pos;
    enum LayoutDirection direction;
    
    List* contents;
} LayoutContainer;