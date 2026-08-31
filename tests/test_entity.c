/* Entity and component behaviour: lookup, data copying, and the thing the old
 * fat-struct model could not express -- a hitbox independent of the sprite. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "VSE/vse.h"

static VSE_Engine *MakeEngine(int pixelsPerUnit)
{
    VSE_Engine *engine = calloc(1, sizeof(VSE_Engine));
    engine->pixelsPerUnit = pixelsPerUnit;
    engine->allEntities = VSE_ListCreate(0);
    engine->allWindows = VSE_ListCreate(0);
    return engine;
}

int main(void)
{
    VSE_Engine *engine = MakeEngine(2);

    VSE_Entity *e = VSE_EntityCreate(engine, (VSE_Vector2Float){100, 100}, VSE_VECTOR2_FLOAT_ONE);
    assert(e->active);
    assert(e->components->size == 0);

    VSE_SpriteRenderer sprite = { .originalSize = {100, 100} };
    VSE_AddComponent(e, VSE_COMPONENT_SPRITE_RENDERER, &sprite);
    assert(VSE_HasComponent(e, VSE_COMPONENT_SPRITE_RENDERER));

    VSE_Vector2Float min, max;

    /* No collider: bounds fall back to the sprite. 100 * ppu(2) * scale(1) = 200 */
    assert(VSE_EntityBounds(engine, e, e->transform.position, &min, &max));
    assert(max.x - min.x == 200.0f && max.y - min.y == 200.0f);

    /* A collider half the sprite's size must win over the texture. */
    VSE_Collider collider = { .size = {50, 50} };
    VSE_AddComponent(e, VSE_COMPONENT_COLLIDER, &collider);
    assert(VSE_EntityBounds(engine, e, e->transform.position, &min, &max));
    assert(max.x - min.x == 100.0f && max.y - min.y == 100.0f);

    /* Offset shifts the box without moving the entity. */
    VSE_Collider *c = VSE_GetComponent(e, VSE_COMPONENT_COLLIDER);
    c->offset = (VSE_Vector2Float){10, 0};
    assert(VSE_EntityBounds(engine, e, e->transform.position, &min, &max));
    assert(min.x == 60.0f && max.x == 160.0f);
    c->offset = VSE_VECTOR2_FLOAT_ZERO;

    /* Transform scale still applies to the collider. */
    e->transform.scale = (VSE_Vector2Float){2, 2};
    assert(VSE_EntityBounds(engine, e, e->transform.position, &min, &max));
    assert(max.x - min.x == 200.0f);
    e->transform.scale = VSE_VECTOR2_FLOAT_ONE;

    /* Overlap uses collider bounds. */
    VSE_Entity *other = VSE_EntityCreate(engine, (VSE_Vector2Float){150, 100}, VSE_VECTOR2_FLOAT_ONE);
    VSE_AddComponent(other, VSE_COMPONENT_COLLIDER, &collider);
    assert(VSE_EntitiesOverlap(engine, e, e->transform.position, other, other->transform.position));
    other->transform.position = (VSE_Vector2Float){400, 100};
    assert(!VSE_EntitiesOverlap(engine, e, e->transform.position, other, other->transform.position));

    /* An entity with neither component has no bounds. */
    VSE_Entity *bare = VSE_EntityCreate(engine, VSE_VECTOR2_FLOAT_ZERO, VSE_VECTOR2_FLOAT_ONE);
    assert(!VSE_EntityBounds(engine, bare, bare->transform.position, &min, &max));

    /* Remove. */
    VSE_RemoveComponent(e, VSE_COMPONENT_COLLIDER);
    assert(!VSE_HasComponent(e, VSE_COMPONENT_COLLIDER));
    assert(VSE_GetComponent(e, VSE_COMPONENT_COLLIDER) == NULL);

    /* A game-registered component type, with initData copied in. */
    typedef struct { int hp; } Health;
    VSE_ComponentType HEALTH = VSE_ComponentTypeRegister("Health", sizeof(Health));
    assert(HEALTH >= VSE_COMPONENT_BUILTIN_COUNT);
    VSE_AddComponent(e, HEALTH, &(Health){ .hp = 42 });
    assert(((Health *)VSE_GetComponent(e, HEALTH))->hp == 42);
    assert(VSE_GetComponent(bare, HEALTH) == NULL);

    /* A component can reach its own entity. */
    VSE_Component *ref = VSE_GetComponentRef(e, HEALTH);
    assert(ref->entity == e);

    VSE_EntityDestroy(engine, bare);

    puts("test_entity: PASSED");
    return 0;
}
