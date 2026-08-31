# Getting started

A complete VSE program: initialise, open a window, put a sprite in it, run the loop.

## The minimal app

```c
#include "VSE/vse.h"

int main(void)
{
    VSE_Config config = {
        .assetRoot     = "Assets/",            /* where your textures and fonts live */
        .shaderRoot    = "../VSE/shaders/",    /* VSE's shader tree */
        .debugFontPath = "ByteBounce.ttf",     /* relative to assetRoot */
        .debugFontSize = 16,
        .uiFontPath    = "MyFont.ttf",         /* used by static text */
        .pixelsPerUnit = 2,
        .debugMode     = true,                 /* FPS overlay + gizmo rects */
    };

    VSE_Engine *engine = VSE_Init(&config);

    VSE_AddDefaultUpdatables(engine);

    VSE_CreateGameWindowWithRenderer(engine,
        (VSE_Vector2Int){100, 100},            /* position on the desktop */
        (VSE_Vector2Int){640, 480},            /* size */
        VSE_WINDOW_WORLD_SPACE,
        VSE_FIXED_SIZE,
        "My Game");

    VSE_Texture  *texture  = VSE_LoadTexture("Player.png");
    VSE_Material *material = VSE_CreateMaterial(NULL, NULL);   /* NULL = default shaders */

    VSE_EntityCreateSprite(engine, texture, material,
                           (VSE_Vector2Float){300, 300},
                           VSE_VECTOR2_FLOAT_ONE);

    while (1)
    {
        VSE_Tick(engine);
    }
}
```

Build it against a sibling VSE checkout — see the README for the Makefile fragment.

## What each piece is

**`VSE_Init`** brings up SDL, creates the hidden GL context that all windows share, loads the
built-in shaders, and returns the engine. It takes every path it needs from `VSE_Config`; VSE
hardcodes no paths of its own.

**`VSE_AddDefaultUpdatables`** registers the engine's six per-frame systems: input, render, windows,
tweens, UI, and behaviours. Without it nothing happens. Call it again after every
`VSE_CleanUpScene`.

**Windows are real OS windows.** Each gets its own framebuffer, so a game can be spread across
several of them. `VSE_WINDOW_WORLD_SPACE` means entities are positioned in a world coordinate space
shared by every window — move the window on your desktop and you see a different part of the world.
`VSE_WINDOW_SCREEN_SPACE` positions relative to that window's own corner, which is what you want for
menus.

**`pixelsPerUnit`** scales every sprite and collider. At 2, a 32x32 texture draws 64x64.

**Materials.** `VSE_CreateMaterial(NULL, NULL)` uses the default vertex and fragment shaders. Pass
paths relative to `shaderRoot` to use your own.

## Working directory matters

Asset and shader paths resolve against the process's working directory, so run the game from the
directory that `assetRoot` is relative to. The bundled `Makefile`'s `run` target does this.

## Where next

- `01-architecture.md` — the frame loop, and who owns what
- `02-entities-components.md` — the component model
- `99-cookbook.md` — recipes lifted from a real game
