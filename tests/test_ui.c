/* UI entities are ordinary entities carrying UI components. These tests cover
 * the parts that need no GL context: the hierarchy, scale accumulation, and the
 * fact that behaviour is now decided by which components an entity has. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "VSE/vse.h"

static VSE_Engine *MakeEngine(int pixelsPerUnit)
{
    VSE_Engine *engine = calloc(1, sizeof(VSE_Engine));
    engine->pixelsPerUnit = pixelsPerUnit;
    engine->allEntities = VSE_ListCreate(0);
    engine->allWindows = VSE_ListCreate(0);
    engine->allGizmosEntities = VSE_ListCreate(0);
    return engine;
}

static VSE_Window *MakeWindow(void)
{
    VSE_Window *w = calloc(1, sizeof(VSE_Window));
    w->uiEntitiesDrawList = VSE_ListCreate(0);
    w->gameEntitiesDrawList = VSE_ListCreate(0);
    w->gizmosEntitiesDrawList = VSE_ListCreate(0);
    return w;
}

int main(void)
{
    VSE_Engine *engine = MakeEngine(2);
    VSE_Window *window = MakeWindow();

    /* A UI entity is a plain entity that happens to carry a rect transform. */
    VSE_Entity *panel = VSE_CreateUIEntity(engine, window, (VSE_Vector2Float){100, 100},
                                           (VSE_Vector2Float){2, 2}, NULL);
    assert(VSE_HasComponent(panel, VSE_COMPONENT_RECT_TRANSFORM));
    assert(window->uiEntitiesDrawList->size == 1);

    VSE_RectTransform *panelRect = VSE_GetComponent(panel, VSE_COMPONENT_RECT_TRANSFORM);
    assert(panelRect->parentScale.x == 1.0f);   /* no parent */
    assert(panelRect->parent == NULL);
    assert(panelRect->children->size == 0);

    /* A child records its parent and is recorded by it. */
    VSE_Entity *label = VSE_CreateUIEntity(engine, window, (VSE_Vector2Float){100, 100},
                                           (VSE_Vector2Float){1, 1}, panel);
    VSE_RectTransform *labelRect = VSE_GetComponent(label, VSE_COMPONENT_RECT_TRANSFORM);
    assert(labelRect->parent == panel);
    assert(panelRect->children->size == 1);
    assert(VSE_ListGet(panelRect->children, 0) == label);

    /* The child inherits the parent's accumulated scale at creation. */
    assert(labelRect->parentScale.x == 2.0f && labelRect->parentScale.y == 2.0f);

    /* Interactable and dynamic text are just components, so an entity can be
     * asked what it does rather than what it is. */
    assert(!VSE_HasComponent(panel, VSE_COMPONENT_INTERACTABLE));
    VSE_AddComponent(panel, VSE_COMPONENT_INTERACTABLE, &(VSE_Interactable){ .data = engine });
    assert(VSE_HasComponent(panel, VSE_COMPONENT_INTERACTABLE));

    VSE_Interactable *it = VSE_GetComponent(panel, VSE_COMPONENT_INTERACTABLE);
    assert(it->data == engine);
    assert(it->isHovered == false);

    char buffer[16] = "score";
    VSE_Entity *counter = VSE_CreateDynamicText(engine, window, buffer,
                                                VSE_VECTOR2_FLOAT_ZERO,
                                                VSE_VECTOR2_FLOAT_ONE, NULL);
    VSE_DynamicText *text = VSE_GetComponent(counter, VSE_COMPONENT_DYNAMIC_TEXT);
    assert(text != NULL);
    assert(text->text == buffer);          /* borrowed, not copied */
    assert(!VSE_HasComponent(counter, VSE_COMPONENT_SPRITE_RENDERER));

    /* A UI entity and a world entity are the same type now. */
    VSE_Entity *world = VSE_EntityCreate(engine, VSE_VECTOR2_FLOAT_ZERO, VSE_VECTOR2_FLOAT_ONE);
    assert(!VSE_HasComponent(world, VSE_COMPONENT_RECT_TRANSFORM));
    assert(engine->allEntities->size == 4);   /* panel, label, counter, world */

    puts("test_ui: PASSED");
    return 0;
}
