# Cookbook

Recipes taken from a real game built on VSE. Each is a complete, working pattern.

## A scene switch

Scenes are yours, not the engine's. VSE gives you teardown; you decide what states exist.

```c
/* game.h */
typedef enum { NONE_STATE, MENU_STATE, GAMEPLAY_STATE, GAME_OVER_STATE } GameState;

typedef struct {
    GameState  state, pendingState;
    LevelData *levelData;
    bool       hasWon;
} GameContext;

/* Requested from anywhere; applied between frames. */
void SetPendingState(GameContext *game, GameState state)
{
    if (game->pendingState != NONE_STATE) return;   /* first request wins */
    game->pendingState = state;
}

void EnterState(VSE_Engine *engine, GameContext *game, GameState state)
{
    VSE_DisableLiveResizeRendering();
    SDL_GL_MakeCurrent(engine->hiddenWindow, engine->glContext);

    VSE_CleanUpScene(engine);
    VSE_AddDefaultUpdatables(engine);     /* teardown removed these too */

    switch (state)
    {
        case MENU_STATE:     CreateMainMenu(engine, game); break;
        case GAMEPLAY_STATE: CreateLevel(engine, game, 0); break;
        case GAME_OVER_STATE:CreateWinScreen(engine);      break;
        default: break;
    }

    game->state = state;
}

/* main loop */
while (1)
{
    if (game->pendingState != NONE_STATE)
    {
        EnterState(engine, game, game->pendingState);
        game->pendingState = NONE_STATE;
        VSE_EnableLiveResizeRendering(engine);
    }
    VSE_Tick(engine);
}
```

Requesting a state rather than entering one directly matters: a button handler runs *inside* a
frame, and destroying the scene there would pull the ground out from under the loop that is
iterating it.

## A button that scales on hover

```c
static void OnHover(VSE_Engine *engine, VSE_Entity *button)
{
    VSE_RectTransform *rect = VSE_GetComponent(button, VSE_COMPONENT_RECT_TRANSFORM);
    VSE_TweenData d = { .vector2FloatTween = { .fromValue = {1, 1}, .endValue = {1.5f, 1.5f} } };
    VSE_PlayTween(engine, VSE_CreateTween(VSE_VECTOR2_FLOAT_TWEEN, &rect->parentScale,
                                          d, 0.5f, true, VSE_OUT_QUINT));
}

static void OnHoverExit(VSE_Engine *engine, VSE_Entity *button)
{
    VSE_RectTransform *rect = VSE_GetComponent(button, VSE_COMPONENT_RECT_TRANSFORM);
    VSE_TweenData d = { .vector2FloatTween = { .fromValue = {1.5f, 1.5f}, .endValue = {1, 1} } };
    VSE_PlayTween(engine, VSE_CreateTween(VSE_VECTOR2_FLOAT_TWEEN, &rect->parentScale,
                                          d, 0.5f, true, VSE_OUT_QUINT));
}
```

Tween `parentScale`, not `scale`, so the label scales with the button. Flicking the mouse on and off
is safe: starting a tween on a target that is already animating finishes the old one first.

## A menu

```c
void CreateMainMenu(VSE_Engine *engine, GameContext *game)
{
    VSE_Window *w = VSE_CreateGameWindowWithRenderer(engine, VSE_GetDisplayCenterPosition(),
                                                    (VSE_Vector2Int){500, 600},
                                                    VSE_WINDOW_SCREEN_SPACE, VSE_FIXED_SIZE, "Menu");

    SDL_SetWindowPosition(w->sdlWindow, w->position.x - w->size.x / 2,
                                        w->position.y - w->size.y / 2);

    VSE_CreateButton(engine, &(VSE_ButtonDesc){
        .window    = w,
        .position  = w->windowCenterPoint,
        .scale     = {2, 2},
        .texture   = VSE_LoadTexture("Button.png"),
        .text      = "Play",
        .textColor = (SDL_Color){0, 0, 0},
        .userData  = game,
        .OnClick   = OnPlayClicked,
        .OnHover   = OnHover,
        .OnHoverExit = OnHoverExit,
    });
}

static void OnPlayClicked(VSE_Engine *engine, void *userData)
{
    SetPendingState((GameContext *)userData, GAMEPLAY_STATE);
}
```

`userData` is what lets the handler reach game state without the engine knowing anything about it.
`VSE_ButtonDesc` is zero-initialised, so anything you leave out takes a sensible default.

## A player with a hitbox smaller than its sprite

```c
VSE_Entity *CreatePlayer(VSE_Engine *engine, VSE_Vector2Float position)
{
    VSE_Texture  *tex = VSE_LoadTexture("Player.png");
    VSE_Material *mat = VSE_CreateMaterial(NULL, NULL);

    VSE_Entity *player = VSE_EntityCreateSprite(engine, tex, mat, position, VSE_VECTOR2_FLOAT_ONE);

    /* Forgiving hitbox: narrower than the artwork, shifted to the feet. */
    VSE_Collider hitbox = { .size = {tex->width * 0.6f, tex->height * 0.5f},
                            .offset = {0, tex->height * 0.25f} };
    VSE_AddComponent(player, VSE_COMPONENT_COLLIDER, &hitbox);

    VSE_AddBehaviour(player, UpdatePlayer, NULL);
    return player;
}
```

## Keeping an entity inside the windows

In a world-space game the windows *are* the playfield, so movement is only allowed where the entity
still fits inside one:

```c
static bool FitsInSomeWindow(VSE_Engine *engine, VSE_Entity *e, VSE_Vector2Float position)
{
    VSE_Vector2Float min, max;
    if (!VSE_EntityBounds(engine, e, position, &min, &max)) return false;

    for (int i = 0; i < engine->allWindows->size; i++)
    {
        VSE_Window *w = engine->allWindows->elements[i];
        VSE_Vector2Float lo = {w->position.x, w->position.y};
        VSE_Vector2Float hi = {w->position.x + w->size.x, w->position.y + w->size.y};
        if (VSE_IsBoxInBounds(min, max, lo, hi)) return true;
    }
    return false;
}

/* in the behaviour: test the position before committing to it */
VSE_Vector2Float next = { me->transform.position.x + dir.x * SPEED * deltaTime,
                          me->transform.position.y + dir.y * SPEED * deltaTime };
if (FitsInSomeWindow(engine, me, next)) me->transform.position = next;
```

Because bounds come from the collider, shrinking the hitbox lets the player edge further past a
window's visible border — a tuning knob rather than a code change.

## A trigger that fires once

```c
static void UpdateLevelTarget(VSE_Component *self, VSE_Engine *engine, float deltaTime)
{
    GameContext *game = self->userData;
    if (game->hasWon) return;                       /* fire once */

    VSE_Entity *player = game->levelData->player;
    if (player == NULL) return;

    if (VSE_EntitiesOverlap(engine, player, player->transform.position,
                            self->entity, self->entity->transform.position))
    {
        game->hasWon = true;
        SetPendingState(game, GAME_OVER_STATE);
    }
}

VSE_AddBehaviour(levelTarget, UpdateLevelTarget, game);
```

## A component of your own

```c
/* health.h */
typedef struct { int hp, maxHp; } Health;
extern VSE_ComponentType HEALTH;

/* health.c */
VSE_ComponentType HEALTH;
void RegisterHealth(void) { HEALTH = VSE_ComponentTypeRegister("Health", sizeof(Health)); }

void Damage(VSE_Entity *e, int amount)
{
    Health *h = VSE_GetComponent(e, HEALTH);
    if (h == NULL) return;                     /* not everything has health */
    h->hp -= amount;
}
```

Call `RegisterHealth()` once at startup, before any entity uses it.

## A post-processing effect

```c
VSE_Material *wobble = VSE_CreateMaterial("PostProcessing/wobble.frag",
                                          "PostProcessing/post.vert");
VSE_AddUniformToMaterial(wobble, "time", VSE_UNIFORM_FLOAT, &engine->time);
VSE_AddPostProcessingEffect(engine, "wobble", wobble);
```

The uniform is stored **by pointer**, so `&engine->time` keeps feeding the shader every frame with
no per-frame work. The pointed-at value must outlive the material.

## A window whose contents scale as you resize it

```c
VSE_CreateGameWindowWithRenderer(engine, position, size,
                                 VSE_WINDOW_WORLD_SPACE,
                                 VSE_SCALE_WITH_RESIZE,      /* <- */
                                 "Scaling Window");
```

Entities fully inside the window have their transform scale multiplied by the frame's percentage
size change.

## Drawing an entity in one window only

```c
VSE_Entity *e = VSE_EntityCreate(engine, position, scale);   /* not ...CreateSprite */

VSE_SpriteRenderer sprite = { .texture = tex, .material = mat,
                              .originalSize = {tex->width, tex->height} };
VSE_AddComponent(e, VSE_COMPONENT_SPRITE_RENDERER, &sprite);
VSE_AddGameEntityToDrawList(justThisWindow, e);
```

`VSE_EntityCreateSprite` adds to *every* window, which is usually what you want in world space but
never what you want for a HUD.
