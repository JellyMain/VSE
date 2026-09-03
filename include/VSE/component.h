#pragma once
#include "VSE/fwd.h"
#include "VSE/list.h"
#include "VSE/types.h"
#include <SDL.h>
#include <stdbool.h>
#include <stddef.h>


static const char *const SPRITE_RENDERER = "SpriteRenderer";

typedef enum VSE_ComponentType
{
    SPRITE_RENDERER_COMPONENT,
    BEHAVIOUR,
    COMPONENTS_COUNT
} VSE_ComponentType;

typedef struct VSE_Component
{
    const char *name;
    VSE_ComponentType type;
    void *data;
    void (*Start)(void *data);
    void (*Update)(void *data);
    void (*Cleanup)(void *data);
} VSE_Component;

/** Where an entity is and how big it is. Every entity has one and it cannot be
 *  removed, so the common case costs no lookup. */
typedef struct VSE_Transform
{
    VSE_Vector2Float position;
    VSE_Vector2Float scale;
} VSE_Transform;

typedef struct VSE_SpriteRendererComponentData
{
    VSE_Vector2Float size;
    VSE_Material *material;
    VSE_Texture *texture;
} VSE_SpriteRendererComponentData;

typedef struct VSE_Entity
{
    VSE_Transform transform;
    VSE_List *components; //VSE_List(of VSE_List(of VSE_Component))
    bool active;
} VSE_Entity;

/** Creates an entity and registers it with the engine. It starts with a
 *  transform and no components. */
VSE_Entity *VSE_CreateEntity(VSE_Engine *engine, VSE_Window *window,
                             VSE_Vector2Float position, VSE_Vector2Float scale);

void VSE_AddComponent(VSE_Entity *entity, VSE_Component *component);

VSE_Component *VSE_GetComponent(VSE_Entity *entity, VSE_ComponentType componentType,  const char *componentName);

VSE_Component *VSE_CreateComponent(VSE_ComponentType componentType ,const char *componentName, void *data,
                                   void (*Start)(void *data),
                                   void (*Update)(void *data),
                                   void (*Cleanup)(void *data));

VSE_Component *VSE_CreateSpriteRendererComponent(const char *texturePath);


/** Runs each component's Destroy hook, frees the components and the entity,
 *  and removes it from the engine and from every window draw list. */
void VSE_EntityDestroy(VSE_Engine *engine, VSE_Entity *entity);

VSE_Updatable *VSE_CreateBehaviourUpdatable();
