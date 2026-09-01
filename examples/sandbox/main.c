#include "VSE/component.h"
#include "VSE/engine.h"
#include "VSE/fwd.h"
#include "VSE/render.h"
#include "VSE/types.h"
#include "VSE/update.h"
#include "VSE/window.h"

int main(void)
{
    VSE_Config config = {.assetRoot = "examples/sandbox/assets",
                         .shaderRoot = "shaders/",
                         .pixelsPerUnit = 5};

    VSE_Engine *engine = VSE_Init(&config);

    VSE_AddDefaultUpdatables(engine);

    VSE_Window *window = VSE_CreateGameWindowWithRenderer(
        engine, (VSE_Vector2Int){.x = 100, .y = 100},
        (VSE_Vector2Int){.x = 600, .y = 600}, VSE_WINDOW_SCREEN_SPACE,
        VSE_FIXED_SIZE, "Hello");


    VSE_Entity *player = VSE_CreateEntity(
        engine, (VSE_Vector2Float){.x = 50, .y = 50}, VSE_VECTOR2_FLOAT_ONE);

    VSE_AddGameEntityToDrawList(window, player);

    VSE_Component *spriteRenderer =
        VSE_CreateSpriteRendererComponent("/Bunny.png");
    VSE_AddComponent(player, spriteRenderer);


    while (1)
    {
        VSE_Tick(engine);
    }
}
