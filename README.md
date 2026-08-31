# VSE — Very Simple Engine

A small, hand-written 2D game engine in C11, built on SDL2 and OpenGL. Rendering, tweening, UI,
windowing and data structures are all implemented from scratch rather than pulled in as
dependencies — the point is to understand every layer, not to hide it.

VSE draws each window through its own framebuffer, which makes multi-window games (where the OS
windows themselves are part of the game) straightforward.

## Building

Requires MinGW-w64 gcc and SDL2 / SDL2_image / SDL2_ttf. Paths to SDL come from a `VCPKG` variable:

```sh
mingw32-make                                   # -> build/libVSE.a
mingw32-make VCPKG=/path/to/installed/triplet  # if SDL2 lives elsewhere
mingw32-make test                              # build and run the test binaries
mingw32-make clean
```

## Using it from a game

Clone VSE next to your game and point the game's build at it — nothing is copied:

```make
VSE_DIR ?= ../VSE
CPPFLAGS += -I$(VSE_DIR)/include
LDLIBS   += $(VSE_DIR)/build/libVSE.a

$(VSE_DIR)/build/libVSE.a: force
	@$(MAKE) -C $(VSE_DIR)
force: ;
```

Then one `make` in the game rebuilds the engine first if its sources changed.

```c
#include "VSE/vse.h"

VSE_Config config = {
    .assetRoot     = "Assets/",
    .shaderRoot    = "../VSE/shaders/",
    .debugFontPath = "ByteBounce.ttf",
    .debugFontSize = 16,
    .uiFontPath    = "MyFont.ttf",
    .pixelsPerUnit = 2,
    .debugMode     = true,
};

VSE_Engine *engine = VSE_Init(&config);
```

VSE owns no paths of its own — the game says where its assets live and which shader tree to use.

## What the engine owns, and what it doesn't

`VSE_Engine` holds only engine state: the renderer, the entity/window/tween registries, timing.
It has no idea your game exists. Game state reaches engine-invoked callbacks through per-object
`userData` pointers that the engine relays but never dereferences:

```c
void OnPlay(VSE_Engine *engine, void *data)
{
    GameContext *game = data;   /* your type, passed at creation */
    game->pendingState = GAMEPLAY;
}
```

## Entities and components

An entity is a transform plus a list of components, in the style of Unity's GameObject:

```c
VSE_Entity *player = VSE_EntityCreateSprite(engine, texture, material, position, scale);

VSE_Collider hitbox = { .size = {24, 24} };   /* independent of the sprite */
VSE_AddComponent(player, VSE_COMPONENT_COLLIDER, &hitbox);

VSE_AddBehaviour(player, UpdatePlayer, game);
```

Built-ins are `VSE_SpriteRenderer`, `VSE_Collider` and behaviours. Register your own:

```c
typedef struct { int hp; } Health;
VSE_ComponentType HEALTH = VSE_ComponentTypeRegister("Health", sizeof(Health));
VSE_AddComponent(player, HEALTH, &(Health){ .hp = 100 });

Health *h = VSE_GetComponent(player, HEALTH);
```

The registered size is what lets `VSE_AddComponent` allocate and copy your init data, so a new
component kind needs no boilerplate.

Any component that sets an `Update` hook is ticked once per frame by the behaviour system. The
engine's own per-frame passes (input, render, windows, tweens, UI) are `VSE_Updatable`s registered
by `VSE_AddDefaultUpdatables`.

## Layout

```
include/VSE/   public headers -- each compiles standalone; no GL headers leak out
src/           implementation
vendor/glad/   OpenGL loader, private to the library
shaders/       default, gizmo and post-processing shaders
tests/         plain assert-based binaries, one per file
docs/          guides
```

## Docs

Start with `docs/00-getting-started.md`.
