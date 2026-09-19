#pragma once
#include "VSE/fwd.h"
#include "VSE/types.h"
#include <SDL.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static const char *const SPRITE_RENDERER = "SpriteRenderer";

typedef enum VSE_ComponentType
{
    VSE_SPRITE_RENDERER_COMPONENT,
    VSE_BEHAVIOUR,
    VSE_COMPONENTS_COUNT
} VSE_ComponentType;

typedef struct VSE_LifecycleCallbacks
{
    void (*Start)(void *data);
    void (*Update)(void *data);
    void (*Cleanup)(void *data);
} VSE_LifecycleCallbacks;

typedef struct VSE_Component
{
    const char *name;
    struct VSE_BehaviourDLLData *behaviourData;
    VSE_ComponentType type;
    void *data;
} VSE_Component;

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

typedef struct VSE_BehaviourDesc
{
    char *name;
    size_t dataSize;
    void (*Start)(void *data);
    void (*Update)(void *data);
    void (*Cleanup)(void *data);
} VSE_BehaviourDesc;

typedef struct VSE_BehaviourDLLData
{
    char name[50];
    size_t dataSize;
    void (*Start)(void *data);
    void (*Update)(void *data);
    void (*Cleanup)(void *data);
    int copyCounter;
    char pathToOriginal[200];
    void *currentCopyModule;
    uint64_t lastEditiedTime;
} VSE_BehaviourDLLData;

void VSE_AddComponent(VSE_Entity *entity, VSE_Component *component);

void VSE_AddBehaviour(VSE_Engine *engine, VSE_Entity *entity, const char *name);

VSE_Component *VSE_GetComponent(VSE_Entity *entity,
                                VSE_ComponentType componentType,
                                const char *componentName);

VSE_Component *VSE_CreateComponent(VSE_ComponentType componentType,
                                   const char *componentName, void *data,
                                   VSE_BehaviourDLLData *behaviourData);

VSE_Component *VSE_CreateSpriteRendererComponent(const char *texturePath);


VSE_Updatable *VSE_CreateComponentsUpdatable();


VSE_Updatable *VSE_CreateDLLObserverUpdatable();

uint64_t VSE_GetFileLastWrittenTime(char *fileName);
