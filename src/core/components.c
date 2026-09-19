#include "VSE/components.h"
#include "VSE/config.h"
#include "VSE/engine.h"
#include "VSE/entity.h"
#include "VSE/list.h"
#include "VSE/update.h"
#include <fileapi.h>
#include <libloaderapi.h>
#include <minwindef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

typedef struct TimerData
{
    float timer;
} TimerData;

void SpriteRendererUpdate(void *data)
{
    printf("Updating sprite renderer \n");
}

static const VSE_LifecycleCallbacks g_lifecycleCallbacks[VSE_COMPONENTS_COUNT] =
    {[VSE_SPRITE_RENDERER_COMPONENT] = {
         .Start = NULL, .Update = &SpriteRendererUpdate, .Cleanup = NULL}};

VSE_BehaviourDesc *VSE_GetDescriptorFromDll(HMODULE module)
{
    typedef VSE_BehaviourDesc *(*VSE_GetBehaviourDescFn)(void);

    VSE_GetBehaviourDescFn GetDesc =
        (VSE_GetBehaviourDescFn)GetProcAddress(module, "VSE_GetDescriptor");

    if (GetDesc == NULL)
    {
        printf("Function VSE_GetDescriptor was not found in the module");
        return NULL;
    }

    return GetDesc();
}

void VSE_AddComponent(VSE_Entity *entity, VSE_Component *component)
{
    VSE_List *componentTypeList =
        VSE_ListGet(entity->components, component->type);

    if (component->type != VSE_BEHAVIOUR)
    {
        if (VSE_ListGetSize(componentTypeList) > 0)
        {
            printf("Can't add more than one instance of standard component");
            return;
        }
    }

    VSE_ListAdd(componentTypeList, component);
}

void VSE_AddBehaviour(VSE_Engine *engine, VSE_Entity *entity, const char *name)
{
    for (int i = 0; i < engine->loadedBehavioursDLLData->size; i++)
    {
        VSE_BehaviourDLLData *behaviourData =
            VSE_ListGet(engine->loadedBehavioursDLLData, i);
        if (strcmp(behaviourData->name, name) == 0)
        {
            void *data = calloc(1, behaviourData->dataSize);
            VSE_Component *behaviourComponent =
                VSE_CreateComponent(VSE_BEHAVIOUR, name, data, behaviourData);

            VSE_AddComponent(entity, behaviourComponent);
            return;
        }
    }


    char originalDllPath[150];
    VSE_ResolveOriginalBehaviourDLL(originalDllPath, sizeof(originalDllPath),
                                    name);

    char copyDllPath[200];
    VSE_ResolveCopyBehaviourDLL(copyDllPath, sizeof(copyDllPath), name, 0);

    if (!CopyFileA(originalDllPath, copyDllPath, FALSE))
    {
        printf("Failed to copy from %s to %s \n", originalDllPath, copyDllPath);
        return;
    }

    HMODULE module = LoadLibraryA(copyDllPath);

    if (module == NULL)
    {
        printf("Failed to get dll with name %s\n", name);
        return;
    }

    VSE_BehaviourDesc *descriptor = VSE_GetDescriptorFromDll(module);

    if (descriptor == NULL)
    {
        FreeLibrary(module);
        return;
    }

    VSE_BehaviourDLLData *behaviourData =
        calloc(1, sizeof(VSE_BehaviourDLLData));

    strcpy(behaviourData->name, descriptor->name);
    strcpy(behaviourData->pathToOriginal, originalDllPath);
    behaviourData->dataSize = descriptor->dataSize;
    behaviourData->copyCounter = 0;
    behaviourData->Start = descriptor->Start;
    behaviourData->Update = descriptor->Update;
    behaviourData->Cleanup = descriptor->Cleanup;
    behaviourData->currentCopyModule = module;
    behaviourData->lastEditiedTime =
        VSE_GetFileLastWrittenTime(originalDllPath);

    VSE_ListAdd(engine->loadedBehavioursDLLData, behaviourData);

    void *data = calloc(1, descriptor->dataSize);
    VSE_Component *behaviourComponent =
        VSE_CreateComponent(VSE_BEHAVIOUR, name, data, behaviourData);

    VSE_AddComponent(entity, behaviourComponent);
}

VSE_Component *VSE_CreateComponent(VSE_ComponentType componentType,
                                   const char *componentName, void *data,
                                   VSE_BehaviourDLLData *behaviourData)
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
    component->behaviourData = behaviourData;

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
        VSE_SPRITE_RENDERER_COMPONENT, SPRITE_RENDERER, data, NULL);

    return spriteRenderer;
}

VSE_Component *VSE_GetComponent(VSE_Entity *entity,
                                VSE_ComponentType componentType,
                                const char *componentName)
{
    if (componentType == VSE_BEHAVIOUR)
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

static void UpdateComonents(void *data, VSE_Engine *engine, float deltaTime)
{
    for (int i = 0; i < engine->allEntities->size; i++)
    {
        VSE_Entity *entity = engine->allEntities->elements[i];
        if (!entity->active)
        {
            continue;
        }

        for (int k = 0; k < entity->components->size; k++)
        {
            int type = k;

            VSE_List *componentTypeList = VSE_ListGet(entity->components, type);

            for (int j = 0; j < componentTypeList->size; j++)
            {
                VSE_Component *component = VSE_ListGet(componentTypeList, j);

                if (component->type == VSE_BEHAVIOUR)
                {
                    if (component->behaviourData->Update != NULL)
                    {
                        void *data = component->data;
                        component->behaviourData->Update(data);
                    }
                }
                else
                {
                    VSE_LifecycleCallbacks callbacks =
                        g_lifecycleCallbacks[component->type];
                    if (callbacks.Update != NULL)
                    {
                        void *data = component->data;
                        callbacks.Update(data);
                    }
                }
            }
        }
    }
}

static void ObserveDlls(void *data, VSE_Engine *engine, float deltaTime)
{
    TimerData *timerData = data;

    timerData->timer += deltaTime;

    if (timerData->timer >= 2)
    {
        timerData->timer = 0;
        for (int i = 0; i < engine->loadedBehavioursDLLData->size; i++)
        {
            VSE_BehaviourDLLData *behaviourData =
                VSE_ListGet(engine->loadedBehavioursDLLData, i);

            printf("Path to original : %s\n", behaviourData->pathToOriginal);

            uint64_t currentTimestamp =
                VSE_GetFileLastWrittenTime(behaviourData->pathToOriginal);

            if (currentTimestamp != behaviourData->lastEditiedTime)
            {
                printf("DLL was changed - current timestamp: %llu, last saved "
                       "timestamp: %llu\n",
                       currentTimestamp, behaviourData->lastEditiedTime);

                behaviourData->lastEditiedTime = currentTimestamp;
                behaviourData->copyCounter++;

                char copyDllPath[200];
                VSE_ResolveCopyBehaviourDLL(copyDllPath, sizeof(copyDllPath),
                                            behaviourData->name,
                                            behaviourData->copyCounter);


                if (!CopyFileA(behaviourData->pathToOriginal, copyDllPath,
                               FALSE))
                {
                    printf("failed to copy from %s to %s \n",
                           behaviourData->pathToOriginal, copyDllPath);
                    continue;
                }

                HMODULE module = LoadLibraryA(copyDllPath);

                if (module == NULL)
                {
                    printf("Failed to get dll with name %s\n",
                           behaviourData->name);
                    continue;
                }

                VSE_BehaviourDesc *descriptor =
                    VSE_GetDescriptorFromDll(module);

                behaviourData->Start = descriptor->Start;
                behaviourData->Update = descriptor->Update;
                behaviourData->Cleanup = descriptor->Cleanup;

                HMODULE oldHandle = behaviourData->currentCopyModule;

                if (FreeLibrary(oldHandle))
                {
                    printf("Freed old dll\n");
                }

                behaviourData->currentCopyModule = module;
            }
            else
            {
                printf("DLL DIDN'T CHANGE\n");
            }
        }
    }
}

VSE_Updatable *VSE_CreateComponentsUpdatable()
{
    return VSE_CreateUpdatable(NULL, UpdateComonents);
}

VSE_Updatable *VSE_CreateDLLObserverUpdatable()
{
    TimerData *timerData = calloc(1, sizeof(TimerData));

    return VSE_CreateUpdatable(timerData, ObserveDlls);
}
