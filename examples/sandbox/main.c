#include "VSE/component.h"
#include "VSE/engine.h"
#include "VSE/fwd.h"
#include "VSE/render.h"
#include "VSE/types.h"
#include "VSE/update.h"
#include "VSE/window.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct Health
{
    int currentHealth;
} Health;

void UpdateHealth(void *data)
{

    Health *healthData = data;

    healthData->currentHealth--;

    printf("Current health %d \n", healthData->currentHealth);
}

int main(void)
{
    VSE_Config config = {.assetRoot = "examples/sandbox/assets",
                         .shaderRoot = "shaders/",
                         .pixelsPerUnit = 5};

    VSE_Engine *engine = VSE_Init(&config);

    VSE_AddDefaultUpdatables(engine);

    VSE_CreateGameWindowWithRenderer(
        engine, (VSE_Vector2Int){.x = 100, .y = 100},
        (VSE_Vector2Int){.x = 600, .y = 600}, VSE_WINDOW_SCREEN_SPACE,
        VSE_FIXED_SIZE, "Hello");


    VSE_Entity *player =
        VSE_CreateEntity(engine, NULL, (VSE_Vector2Float){.x = 50, .y = 50},
                         VSE_VECTOR2_FLOAT_ONE);


    VSE_Component *spriteRenderer =
        VSE_CreateSpriteRendererComponent("/Bunny.png");


    VSE_AddComponent(player, spriteRenderer);


    Health *healthData = calloc(1, sizeof(Health));
    healthData->currentHealth = 100;

    VSE_Component *healthComponent = VSE_CreateComponent(
        BEHAVIOUR, "Health", healthData, NULL, UpdateHealth, NULL);


    VSE_AddComponent(player, healthComponent);


    while (1)
    {
        VSE_Tick(engine);
    }
}
