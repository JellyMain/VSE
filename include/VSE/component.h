#pragma once
#include <SDL.h>
#include <stdbool.h>
#include <stddef.h>
#include "VSE/fwd.h"
#include "VSE/types.h"
#include "VSE/texture.h"
#include "VSE/material.h"
#include "VSE/list.h"

struct VSE_Engine;
struct VSE_Entity;
struct VSE_Component;


/** Identifies a component kind. Built-ins occupy the low ids; a game gets more
 *  from VSE_ComponentTypeRegister. */
typedef int VSE_ComponentType;

enum
{
	VSE_COMPONENT_SPRITE_RENDERER,
	VSE_COMPONENT_COLLIDER,
	VSE_COMPONENT_BEHAVIOUR,
	VSE_COMPONENT_RECT_TRANSFORM,
	VSE_COMPONENT_DYNAMIC_TEXT,
	VSE_COMPONENT_INTERACTABLE,
	VSE_COMPONENT_INPUT_FIELD,
	VSE_COMPONENT_BUILTIN_COUNT
};

/** Upper bound on distinct component types in one process. */
#define VSE_COMPONENT_TYPE_MAX 64


typedef void (*VSE_ComponentStartFn)(struct VSE_Component *self, struct VSE_Engine *engine);
typedef void (*VSE_ComponentUpdateFn)(struct VSE_Component *self, struct VSE_Engine *engine,
                                      float deltaTime);
typedef void (*VSE_ComponentDestroyFn)(struct VSE_Component *self);


/** One component attached to one entity.
 *
 *  `data` points at a block sized by the component type's registration, owned by
 *  the component and freed with it. The lifecycle hooks are optional: any
 *  component that sets `Update` is ticked by the behaviour system. */
typedef struct VSE_Component
{
	VSE_ComponentType type;
	void *data;
	/** The entity this is attached to, so a component can reach its siblings. */
	struct VSE_Entity *entity;
	/** Relayed to the hooks below, never dereferenced by the engine. */
	void *userData;
	VSE_ComponentStartFn Start;
	VSE_ComponentUpdateFn Update;
	VSE_ComponentDestroyFn Destroy;
} VSE_Component;


/** Where an entity is and how big it is. Every entity has one and it cannot be
 *  removed, so the common case costs no lookup. */
typedef struct VSE_Transform
{
	VSE_Vector2Float position;
	VSE_Vector2Float scale;
} VSE_Transform;


typedef struct VSE_Entity
{
	VSE_Transform transform;
	/** VSE_Component*, owned by this entity. */
	VSE_List *components;
	/** Relayed to callbacks, never dereferenced by the engine. */
	void *userData;
	bool active;
} VSE_Entity;


// --- built-in components ---------------------------------------------------

/** Draws a texture at the entity's transform. */
typedef struct VSE_SpriteRenderer
{
	VSE_Texture *texture;
	VSE_Material *material;
	/** Untransformed texture size in pixels; filled in at creation. */
	VSE_Vector2Float originalSize;
	/** Size after pixelsPerUnit and transform scale; recomputed each frame. */
	VSE_Vector2Float size;
	int sortOrder;
} VSE_SpriteRenderer;


/** An axis-aligned box for overlap tests. Independent of the sprite, so a
 *  hitbox need not match the artwork. */
typedef struct VSE_Collider
{
	/** Untransformed box size in pixels; scales like a sprite. */
	VSE_Vector2Float size;
	/** Offset from the entity's position, in the same units as size. */
	VSE_Vector2Float offset;
	bool isTrigger;
} VSE_Collider;


// --- component types -------------------------------------------------------

/** Registers a new component kind and returns its id.
 *
 *  @param name  used only for diagnostics; not copied, so pass a literal
 *  @param size  bytes to allocate for each instance's `data`, or 0 for a
 *               component that carries no data of its own
 *  @return the new id, or -1 if VSE_COMPONENT_TYPE_MAX is exhausted */
VSE_ComponentType VSE_ComponentTypeRegister(const char *name, size_t size);

/** Diagnostic name for a component type, or "?" if unknown. */
const char *VSE_ComponentTypeName(VSE_ComponentType type);


// --- entities --------------------------------------------------------------

/** Creates an entity and registers it with the engine. It starts with a
 *  transform and no components. */
VSE_Entity *VSE_EntityCreate(VSE_Engine *engine, VSE_Vector2Float position, VSE_Vector2Float scale);

/** Convenience: an entity carrying a VSE_SpriteRenderer, added to every
 *  window's draw list. `originalSize` is taken from the texture. */
VSE_Entity *VSE_EntityCreateSprite(VSE_Engine *engine, VSE_Texture *texture, VSE_Material *material,
                                   VSE_Vector2Float position, VSE_Vector2Float scale);

/** Runs each component's Destroy hook, frees the components and the entity,
 *  and removes it from the engine and from every window draw list. */
void VSE_EntityDestroy(VSE_Engine *engine, VSE_Entity *entity);


// --- components ------------------------------------------------------------

/** Attaches a component of `type`. If the type has a non-zero size, `data` is
 *  allocated and `initData` copied into it when not NULL.
 *  @return the component, owned by the entity */
VSE_Component *VSE_AddComponent(VSE_Entity *entity, VSE_ComponentType type, const void *initData);

/** Attaches a component whose only job is to run `update` every frame.
 *  `userData` is relayed to the callback untouched. */
VSE_Component *VSE_AddBehaviour(VSE_Entity *entity, VSE_ComponentUpdateFn update, void *userData);

/** @return the component's `data`, or NULL if the entity has no such component.
 *  For a zero-size type this returns NULL even when present -- use
 *  VSE_HasComponent or VSE_GetComponentRef instead. */
void *VSE_GetComponent(VSE_Entity *entity, VSE_ComponentType type);

/** @return the component itself, or NULL if absent. */
VSE_Component *VSE_GetComponentRef(VSE_Entity *entity, VSE_ComponentType type);

/** @return true when the entity carries a component of this type. */
bool VSE_HasComponent(VSE_Entity *entity, VSE_ComponentType type);

/** Runs the Destroy hook and frees the component. No-op if absent. */
void VSE_RemoveComponent(VSE_Entity *entity, VSE_ComponentType type);


// --- systems ---------------------------------------------------------------

/** Ticks every component that set an Update hook. Registered by
 *  VSE_AddDefaultUpdatables. */
VSE_Updatable *VSE_CreateBehaviourUpdatable();

/** World-space bounds of an entity's collider, or of its sprite if it has no
 *  collider. @return false if it has neither. */
bool VSE_EntityBounds(VSE_Engine *engine, VSE_Entity *entity, VSE_Vector2Float position,
                      VSE_Vector2Float *outMin, VSE_Vector2Float *outMax);

/** Axis-aligned overlap test between two entities' bounds. */
bool VSE_EntitiesOverlap(VSE_Engine *engine, VSE_Entity *a, VSE_Vector2Float positionA,
                         VSE_Entity *b, VSE_Vector2Float positionB);


// --- UI components ---------------------------------------------------------

/** Marks an entity as UI and carries its layout state.
 *
 *  An entity with this component is laid out in its window's own coordinates
 *  and drawn after the world, and its size additionally scales by the
 *  accumulated scale of its parents. */
typedef struct VSE_RectTransform
{
	/** Product of every ancestor's scale. Maintained by the UI system. */
	VSE_Vector2Float parentScale;
	/** Previous frame's position, used to carry children along. */
	VSE_Vector2Float lastPosition;
	VSE_Entity *parent;
	/** VSE_Entity*, borrowed. */
	VSE_List *children;
} VSE_RectTransform;


/** Text drawn from the glyph atlas each frame, so the string may change.
 *  For fixed labels prefer VSE_CreateStaticText, which is sharper. */
typedef struct VSE_DynamicText
{
	/** Borrowed; the entity does not own or copy it. */
	char *text;
} VSE_DynamicText;


/** Makes an entity respond to the pointer. Hit tested against its drawn size,
 *  and only in the focused window. */
typedef struct VSE_Interactable
{
	/** Relayed to OnClick, never dereferenced by the engine. */
	void *data;
	void (*OnClick)(struct VSE_Engine *engine, void *data);
	void (*OnClickAnimation)(struct VSE_Engine *engine, VSE_Entity *entity);
	void (*OnHover)(struct VSE_Engine *engine, VSE_Entity *entity);
	void (*OnHoverExit)(struct VSE_Engine *engine, VSE_Entity *entity);
	bool isHovered;
} VSE_Interactable;


/** An editable text buffer. Click to focus, Escape to release. */
typedef struct VSE_InputField
{
	char *text;
	int maxLength;
	struct VSE_Updatable *readKeyboardUpdatable;
} VSE_InputField;


/** A debug outline drawn around another entity when debugMode is on. */
typedef struct VSE_GizmoEntity
{
	VSE_Entity *connectedEntity;
	SDL_Color color;
	float thickness;
} VSE_GizmoEntity;
