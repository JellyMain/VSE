#include "VSE/entity.h"
#include "VSE/engine.h"
#include "VSE/fwd.h"
#include "VSE/list.h"
#include "VSE/render.h"
#include <stdio.h>
#include <stdlib.h>

VSE_Entity *VSE_CreateEntity(VSE_Engine *engine, VSE_Window *window,
                             VSE_Vector2Float position, VSE_Vector2Float scale)
{
    VSE_Entity *entity = calloc(1, sizeof(VSE_Entity));

    if (entity == NULL)
    {
        printf("Can't allocate entity\n");
        return NULL;
    }

    entity->transform.position = position;
    entity->transform.scale = scale;
    entity->components = VSE_ListCreate(VSE_COMPONENTS_COUNT);


    for (int i = 0; i < VSE_COMPONENTS_COUNT; i++)
    {
        VSE_List *componentList = VSE_ListCreate(0);
        VSE_ListAdd(entity->components, componentList);
    }

    entity->active = true;

    VSE_ListAdd(engine->allEntities, entity);

    if (window == NULL)
    {
        if (VSE_ListGetSize(engine->allWindows) == 0)
        {
            printf("AllWindos list is empty \n");
            return NULL;
        }

        window = VSE_ListGet(engine->allWindows, 0);
    }

    VSE_AddEntityToDrawList(window, entity);

    return entity;
}

void VSE_EntityDestroy(VSE_Engine *engine, VSE_Entity *entity)
{
    if (entity == NULL)
    {
        return;
    }

    // for (int i = 0; i < entity->components->size; i++)
    // {
    //     // DestroyComponent(entity->components->elements[i]);
    // }

    // VSE_ListDestroy(entity->components);

    if (engine != NULL)
    {
        VSE_ListRemove(engine->allEntities, entity);
        VSE_RemoveGameEntityFromAllDrawLists(engine, entity);
    }

    free(entity);
}
