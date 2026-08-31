# Entities and components

An entity is a transform plus a bag of components — the Unity GameObject model, not an ECS. See
"Why not an ECS" at the bottom for the reasoning.

```c
typedef struct VSE_Entity {
    VSE_Transform  transform;    /* position + scale; always present */
    VSE_List      *components;
    void          *userData;
    bool           active;
} VSE_Entity;
```

## Transform is not a component

Every entity has exactly one and it cannot be removed, so it lives directly on the struct. The
common case — reading a position — costs a field access, not a lookup.

```c
entity->transform.position.x += 10.0f;
```

## Creating entities

```c
/* Bare: a transform and nothing else. Invisible, no bounds. */
VSE_Entity *marker = VSE_EntityCreate(engine, position, scale);

/* With a sprite, registered in every window's draw list. */
VSE_Entity *player = VSE_EntityCreateSprite(engine, texture, material, position, scale);
```

`VSE_EntityCreateSprite` fills `originalSize` from the texture and adds the entity to the draw list
of every window that exists *at that moment* — so create your windows before your entities.

## Built-in components

Seven, covering both worlds:

| id | data | purpose |
|---|---|---|
| `VSE_COMPONENT_SPRITE_RENDERER` | `VSE_SpriteRenderer` | draws a texture (world entities and UI alike) |
| `VSE_COMPONENT_COLLIDER` | `VSE_Collider` | an overlap box, independent of the sprite |
| `VSE_COMPONENT_BEHAVIOUR` | none | per-entity logic, via `VSE_AddBehaviour` |
| `VSE_COMPONENT_RECT_TRANSFORM` | `VSE_RectTransform` | marks an entity as UI; parent/child and accumulated scale |
| `VSE_COMPONENT_DYNAMIC_TEXT` | `VSE_DynamicText` | a string redrawn from the glyph atlas each frame |
| `VSE_COMPONENT_INTERACTABLE` | `VSE_Interactable` | responds to hover and clicks |
| `VSE_COMPONENT_INPUT_FIELD` | `VSE_InputField` | an editable text buffer |

```c
typedef struct VSE_SpriteRenderer {
    VSE_Texture      *texture;
    VSE_Material     *material;
    VSE_Vector2Float  originalSize;   /* texture size, set at creation */
    VSE_Vector2Float  size;           /* after pixelsPerUnit and scale; per frame */
    int               sortOrder;
} VSE_SpriteRenderer;

typedef struct VSE_Collider {
    VSE_Vector2Float size;      /* untransformed, scales like a sprite */
    VSE_Vector2Float offset;    /* from the entity's position */
    bool             isTrigger;
} VSE_Collider;
```

A collider is **independent of the sprite** — a hitbox smaller than the artwork, or offset from it,
is just a value:

```c
VSE_Collider hitbox = { .size = {24, 24}, .offset = {0, 8} };
VSE_AddComponent(player, VSE_COMPONENT_COLLIDER, &hitbox);
```

`VSE_EntityBounds` uses the collider when present and falls back to the sprite otherwise, so an
entity with only a sprite still collides sensibly.

```c
VSE_Vector2Float min, max;
if (VSE_EntityBounds(engine, e, e->transform.position, &min, &max)) { ... }

if (VSE_EntitiesOverlap(engine, a, a->transform.position, b, b->transform.position)) { ... }
```

Both return `false` for an entity with neither component.

## Behaviours

```c
VSE_AddBehaviour(entity, UpdateFn, userData);
```

Any component with an `Update` hook is ticked by the behaviour system, so `VSE_AddBehaviour` is just
a convenience for "a component whose only job is logic".

```c
static void UpdatePlayer(VSE_Component *self, VSE_Engine *engine, float deltaTime)
{
    VSE_Entity  *me   = self->entity;
    GameContext *game = self->userData;

    VSE_SpriteRenderer *sprite = VSE_GetComponent(me, VSE_COMPONENT_SPRITE_RENDERER);
    /* siblings are reachable through self->entity */
}
```

One entity can carry several behaviours; they run in the order added.

## Your own component types

Register once, then use like any built-in. The registered size is what lets `VSE_AddComponent`
allocate the block and copy your init data, so a new kind needs no boilerplate.

```c
typedef struct { int hp, maxHp; } Health;

VSE_ComponentType HEALTH = VSE_ComponentTypeRegister("Health", sizeof(Health));

VSE_AddComponent(player, HEALTH, &(Health){ .hp = 100, .maxHp = 100 });

Health *h = VSE_GetComponent(player, HEALTH);
if (h != NULL && h->hp <= 0) { ... }
```

Register at startup and keep the id somewhere reachable — a file-scope variable in the module that
owns the component is usually right. Ids are assigned in registration order, so don't persist them.

The registered **size is what makes the component work**: it is what `VSE_AddComponent` allocates and
copies into. A type registered with the wrong size gets a `data` block that does not match what you
read back; a built-in added to the enum but left out of the engine's own table would allocate nothing
and hand back `NULL` from `VSE_GetComponent`, which is why a `_Static_assert` guards that table.

Pass `size = 0` for a pure marker component with no data. `VSE_GetComponent` returns `NULL` for
those even when present, so use `VSE_HasComponent` to test for them.

## The API

| function | notes |
|---|---|
| `VSE_AddComponent(e, type, initData)` | `initData` may be `NULL` to zero-initialise |
| `VSE_GetComponent(e, type)` | the data block, or `NULL` |
| `VSE_GetComponentRef(e, type)` | the `VSE_Component` itself — needed for zero-size types and lifecycle hooks |
| `VSE_HasComponent(e, type)` | presence test |
| `VSE_RemoveComponent(e, type)` | runs `Destroy`, frees the data |
| `VSE_EntityDestroy(engine, e)` | destroys all components, unregisters the entity |

Lookup is a linear scan over the handful of components an entity has. At the scale this engine
targets — tens of entities — that is not worth optimising, and the API is written so the storage
can be swapped for dense pools later without touching game code.

## Lifecycle hooks

`VSE_Component` carries optional `Start`, `Update` and `Destroy` hooks. Set them on the component
returned by `VSE_AddComponent`:

```c
VSE_Component *c = VSE_AddComponent(e, MY_TYPE, &init);
c->Destroy = ReleaseMyThing;   /* runs on RemoveComponent and EntityDestroy */
```

## Why not an ECS

A real ECS — integer entity ids, pure-data components in dense arrays, logic in systems — pays off
at thousands of entities, through cache-friendly iteration. This engine runs games with *tens* of
entities, so that payoff is zero, while the cost is real: sparse sets, id generations, query
intersection, and rewriting every callback into a system. The UI, which is hierarchical and
event-driven, would fight that model hardest — and under this model it is simply entities with a
`VSE_RectTransform`, sharing the same lookup as everything else.

The public API (`VSE_AddComponent` / `VSE_GetComponent`) is deliberately storage-agnostic, so if an
entity count ever justifies dense pools, that is an internal change.
