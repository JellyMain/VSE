#include "VSE/component.h"
#include "VSE/dictionary.h"
#include "VSE/engine.h"
#include "VSE/fwd.h"
#include "VSE/list.h"
#include "VSE/material.h"
#include "VSE/render.h"
#include "VSE/texture.h"
#include "VSE/update.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wctype.h>

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
    entity->components = VSE_ListCreate(COMPONENTS_COUNT);


    for (int i = 0; i < COMPONENTS_COUNT; i++)
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

void VSE_AddComponent(VSE_Entity *entity, VSE_Component *component)
{
    VSE_List *componentTypeList =
        VSE_ListGet(entity->components, component->type);

    if (component->type != BEHAVIOUR)
    {
        if (VSE_ListGetSize(componentTypeList) > 0)
        {
            printf("Can't add more than one instance of standard component");
            return;
        }
    }

    VSE_ListAdd(componentTypeList, component);
}

VSE_Component *VSE_GetComponent(VSE_Entity *entity,
                                VSE_ComponentType componentType,
                                const char *componentName)
{

    if (componentType == BEHAVIOUR)
    {
        VSE_List *behavioursList =
            VSE_ListGet(entity->components, componentType);

        for (int i = 0; i < behavioursList->size; i++)
        {
            VSE_Component *component = VSE_ListGet(behavioursList, i);
            if (strcmp(componentName, component->name) == 0)
            {
                return component;
            }
        }

        printf("Behaviour component not found: %s\n", componentName);
        return NULL;
    }
    else
    {
        VSE_List *standardComponentList =
            VSE_ListGet(entity->components, componentType);

        VSE_Component *component = VSE_ListGet(standardComponentList, 0);
        return component;
    }
}

VSE_Component *VSE_CreateComponent(VSE_ComponentType componentType,
                                   const char *componentName, void *data,
                                   void (*Start)(void *data),
                                   void (*Update)(void *data),
                                   void (*Cleanup)(void *data))
{

    VSE_Component *component = calloc(1, sizeof(VSE_Component));
    if (component == NULL)
    {
        printf("Can't allocate component\n");
        return NULL;
    }

    component->name = componentName;
    component->type = componentType;
    component->data = data;
    component->Start = Start;
    component->Update = Update;
    component->Cleanup = Cleanup;

    return component;
}

VSE_Component *VSE_CreateSpriteRendererComponent(const char *texturePath)
{
    VSE_SpriteRendererComponentData *data =
        calloc(1, sizeof(VSE_SpriteRendererComponentData));

    if (data == NULL)
    {
        printf("Can't allocate Sprite Renderer Data");
        return NULL;
    }

    data->material = VSE_CreateMaterial(NULL, NULL);
    data->texture = VSE_LoadTexture(texturePath);

    VSE_Component *spriteRenderer = VSE_CreateComponent(
        SPRITE_RENDERER_COMPONENT, SPRITE_RENDERER, data, NULL, NULL, NULL);

    return spriteRenderer;
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

// --- systems ---------------------------------------------------------------

static void UpdateBehaviours(void *data, VSE_Engine *engine, float deltaTime)
{
    for (int i = 0; i < engine->allEntities->size; i++)
    {
        VSE_Entity *entity = engine->allEntities->elements[i];
        if (!entity->active)
        {
            continue;
        }

        int type = BEHAVIOUR;

        VSE_List *behavioursList = VSE_ListGet(entity->components, type);

        for (int j = 0; j < behavioursList->size; j++)
        {
            VSE_Component *component = VSE_ListGet(behavioursList, j);
            if (component->Update != NULL)
            {
                void *data = component->data;
                component->Update(data);
            }
        }
    }
}

VSE_Updatable *VSE_CreateBehaviourUpdatable()
{
    return VSE_CreateUpdatable(NULL, UpdateBehaviours);
}
