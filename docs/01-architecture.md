# Architecture

## The frame loop

The game owns the loop; VSE owns what happens inside one frame.

```c
while (1)
{
    HandleGameStates(engine, game);   /* your code: scene switches */
    VSE_Tick(engine);                 /* engine: one frame */
}
```

`VSE_Tick` updates `engine->time` and `engine->deltaTime`, refreshes the FPS counter, then calls
every registered updatable in registration order.

The loop is yours rather than the engine's for one reason: switching scenes destroys windows and
entities, and that must happen *between* frames, never inside one. Keeping the loop in your code
makes that ordering obvious.

## Two levels of per-frame code

**Updatables** are global passes — one call per frame each, not tied to any entity.
`VSE_AddDefaultUpdatables` registers six:

| order | system | what it does |
|---|---|---|
| 1 | input | drains the SDL event queue, latches key/mouse state |
| 2 | render | draws every window |
| 3 | windows | tracks position/size, applies scale-on-resize |
| 4 | tweens | advances active tweens and sequences |
| 5 | UI | propagates parent scale/position for entities with a rect transform, dispatches hover and clicks |
| 6 | behaviours | ticks every component with an `Update` hook |

Order matters. Behaviours run *after* render, so a position written this frame appears next frame.

**Behaviours** are per-entity logic — a component with an `Update` hook. Use these for game logic;
use updatables only when you genuinely need a global pass.

```c
VSE_AddBehaviour(player, UpdatePlayer, game);

void UpdatePlayer(VSE_Component *self, VSE_Engine *engine, float deltaTime)
{
    VSE_Entity  *me   = self->entity;      /* the entity it is attached to */
    GameContext *game = self->userData;    /* whatever you passed in */
    me->transform.position.x += 100.0f * deltaTime;
}
```

## What the engine owns, and what it doesn't

`VSE_Engine` holds engine state only: the renderer, GL context, the entity/window/tween registries,
timing. **It contains nothing game-specific and never will** — that boundary is the whole point of
the library.

Your game keeps its own context struct, which VSE never sees:

```c
typedef struct {
    GameState  state, pendingState;
    LevelData *levelData;
    bool       hasWon;
} GameContext;
```

### How game state reaches engine callbacks

Through **per-object user pointers** that the engine carries but never dereferences. There is
deliberately no `engine->userData`: a single untyped global would be a worse version of what the
existing per-object slots already do.

| carrier | set at | read in |
|---|---|---|
| `VSE_Component.userData` | `VSE_AddBehaviour(e, fn, userData)` | `self->userData` |
| `VSE_Interactable.data` | `VSE_ButtonDesc.userData` | the `void *userData` parameter |
| `VSE_Updatable.data` | `VSE_CreateUpdatable(data, fn)` | the `void *self` parameter |
| `VSE_Entity.userData` | assign after creation, or `VSE_ButtonDesc.userData` | via the entity in a callback |

The last is for callbacks that receive only the entity — `OnHover`, `OnHoverExit`,
`OnInteractionAnimation`.

## Ownership and teardown

- The engine owns every entity, gizmo, window and updatable you create through it. UI entities
  are entities: one type, one registry (`engine->allEntities`).
- An entity owns its components, and frees each component's `data` block with it.
- `VSE_CleanUpScene(engine)` destroys all of the above and clears the tween registry.
- Textures and materials are freed with the sprite that references them.

Switching scenes is therefore:

```c
VSE_DisableLiveResizeRendering();
VSE_CleanUpScene(engine);
VSE_AddDefaultUpdatables(engine);   /* teardown removed these too */
BuildMyNewScene(engine, game);
VSE_EnableLiveResizeRendering(engine);
```

Forgetting `VSE_AddDefaultUpdatables` after a teardown leaves a black, unresponsive window — nothing
is registered to render or read input.

## Rendering

Every window has its own framebuffer. Per frame, per window: bind the FBO, draw game entities, then
UI entities, then gizmos if `debugMode`, then blit through the post-processing chain to the screen.

The renderer asks entities what they *have*, not what they are:

```c
VSE_SpriteRenderer *sprite = VSE_GetComponent(entity, VSE_COMPONENT_SPRITE_RENDERER);
if (sprite == NULL) continue;
```

so an entity with no sprite is simply invisible rather than a special case.

All windows share one GL context, created against a hidden 1x1 window at startup. That is why
`VSE_CleanUpScene` re-binds the context before destroying anything.

## Layers

```
game code
    |
    v
VSE public API  (include/VSE/*.h -- each header compiles standalone)
    |
    v
VSE internals   (src/)
    |
    v
SDL2 + glad     (glad is private; no GL type escapes the public headers)
```

SDL is a *public* dependency: `SDL_Color`, `SDL_Keycode` and `SDL_Window*` appear in the API.
OpenGL is *private* — public headers use `VSE_GLuint`, and a `_Static_assert` in `src/render/opengl.c`
checks it still matches the real `GLuint`.
