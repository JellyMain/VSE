#include "VSE/component.h"
#include "VSE/engine.h"
#include "VSE/list.h"
#include "VSE/material.h"
#include "VSE/render.h"
#include "VSE/texture.h"
#include "VSE/update.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

VSE_Entity *VSE_CreateEntity(VSE_Engine *engine, VSE_Vector2Float position,
                             VSE_Vector2Float scale)
{
    VSE_Entity *entity = calloc(1, sizeof(VSE_Entity));

    entity->transform.position = position;
    entity->transform.scale = scale;
    entity->components = VSE_ListCreate(0);
    entity->active = true;

    VSE_ListAdd(engine->allEntities, entity);

    return entity;
}

void VSE_AddComponent(VSE_Entity *entity, VSE_Component *component)
{
    VSE_ListAdd(entity->components, component);
}

VSE_Component *VSE_GetComponent(VSE_Entity *entity, const char *componentName)
{
    for (int i = 0; i < entity->components->size; i++)
    {
        VSE_Component *component = VSE_ListGet(entity->components, i);
        if (strcmp(componentName, component->name) == 0)
        {
            return component;
        }
    }

    printf("Component not found: %s\n", componentName);
    return NULL;
}

VSE_Component *VSE_CreateSpriteRendererComponent(const char *texturePath)
{
    VSE_Component *spriteRenderer = calloc(1, sizeof(VSE_Component));
    VSE_SpriteRendererComponentData *data =
        calloc(1, sizeof(VSE_SpriteRendererComponentData));

    data->material = VSE_CreateMaterial(NULL, NULL);
    data->texture = VSE_LoadTexture(texturePath);

    spriteRenderer->data = data;
    spriteRenderer->name = SPRITE_RENDERER_COMPONENT;

    spriteRenderer->Start = NULL;
    spriteRenderer->Update = NULL;
    spriteRenderer->Cleanup = NULL;

    return spriteRenderer;
}

void VSE_EntityDestroy(VSE_Engine *engine, VSE_Entity *entity)
{
    if (entity == NULL)
    {
        return;
    }

    for (int i = 0; i < entity->components->size; i++)
    {
        // DestroyComponent(entity->components->elements[i]);
    }

    VSE_ListDestroy(entity->components);

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

        for (int j = 0; j < entity->components->size; j++)
        {
            VSE_Component *component = entity->components->elements[j];
        }
    }
}

VSE_Updatable *VSE_CreateBehaviourUpdatable()
{
    return VSE_CreateUpdatable(NULL, UpdateBehaviours);
}
