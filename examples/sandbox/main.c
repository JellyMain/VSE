#include "VSE/components.h"
#include "VSE/engine.h"
#include "VSE/entity.h"
#include "VSE/fwd.h"
#include "VSE/types.h"
#include "VSE/update.h"
#include "VSE/window.h"
#include <stdlib.h>


int main(void)
{
    VSE_Config config = {.behavioursRoot = "build/debug/examples/sandbox/behaviours/",
                         .assetRoot = "examples/sandbox/assets/",
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
        VSE_CreateSpriteRendererComponent("Bunny.png");


    VSE_AddComponent(player, spriteRenderer);

    VSE_AddBehaviour(engine, player, "health");

    while (1)
    {
        VSE_Tick(engine);
    }
}
