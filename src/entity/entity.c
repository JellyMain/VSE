#include <string.h>
#include "VSE/component.h"
#include "VSE/engine.h"
#include "VSE/update.h"
#include "VSE/render.h"
#include "VSE/math.h"


typedef struct
{
	const char *name;
	size_t size;
} ComponentTypeInfo;


static ComponentTypeInfo g_componentTypes[VSE_COMPONENT_TYPE_MAX] = {
	[VSE_COMPONENT_SPRITE_RENDERER] = {"SpriteRenderer", sizeof(VSE_SpriteRenderer)},
	[VSE_COMPONENT_COLLIDER] = {"Collider", sizeof(VSE_Collider)},
	[VSE_COMPONENT_BEHAVIOUR] = {"Behaviour", 0},
	[VSE_COMPONENT_RECT_TRANSFORM] = {"RectTransform", sizeof(VSE_RectTransform)},
	[VSE_COMPONENT_DYNAMIC_TEXT] = {"DynamicText", sizeof(VSE_DynamicText)},
	[VSE_COMPONENT_INTERACTABLE] = {"Interactable", sizeof(VSE_Interactable)},
	[VSE_COMPONENT_INPUT_FIELD] = {"InputField", sizeof(VSE_InputField)},
};

/* Every built-in must appear above with its real size, or VSE_AddComponent
 * allocates nothing and VSE_GetComponent hands back NULL. */
_Static_assert(VSE_COMPONENT_BUILTIN_COUNT == 7, "register every built-in component type");

static int g_componentTypeCount = VSE_COMPONENT_BUILTIN_COUNT;


VSE_ComponentType VSE_ComponentTypeRegister(const char *name, size_t size)
{
	if (g_componentTypeCount >= VSE_COMPONENT_TYPE_MAX)
	{
		SDL_Log("VSE_ComponentTypeRegister: out of component type slots (max %d)",
		        VSE_COMPONENT_TYPE_MAX);
		return -1;
	}

	VSE_ComponentType type = g_componentTypeCount++;
	g_componentTypes[type].name = name;
	g_componentTypes[type].size = size;
	return type;
}


const char *VSE_ComponentTypeName(VSE_ComponentType type)
{
	if (type < 0 || type >= g_componentTypeCount || g_componentTypes[type].name == NULL)
	{
		return "?";
	}

	return g_componentTypes[type].name;
}


VSE_Entity *VSE_EntityCreate(VSE_Engine *engine, VSE_Vector2Float position, VSE_Vector2Float scale)
{
	VSE_Entity *entity = calloc(1, sizeof(VSE_Entity));

	entity->transform.position = position;
	entity->transform.scale = scale;
	entity->components = VSE_ListCreate(0);
	entity->active = true;

	VSE_ListAdd(engine->allEntities, entity);

	return entity;
}


VSE_Entity *VSE_EntityCreateSprite(VSE_Engine *engine, VSE_Texture *texture, VSE_Material *material,
                                   VSE_Vector2Float position, VSE_Vector2Float scale)
{
	VSE_Entity *entity = VSE_EntityCreate(engine, position, scale);

	VSE_SpriteRenderer sprite = {
		.texture = texture,
		.material = material,
		.originalSize = {texture ? (float) texture->width : 0.0f,
		                 texture ? (float) texture->height : 0.0f},
	};

	VSE_AddComponent(entity, VSE_COMPONENT_SPRITE_RENDERER, &sprite);
	VSE_AddGameEntityToAllDrawLists(engine, entity);

	return entity;
}


VSE_Component *VSE_AddComponent(VSE_Entity *entity, VSE_ComponentType type, const void *initData)
{
	if (entity == NULL || type < 0 || type >= g_componentTypeCount)
	{
		SDL_Log("VSE_AddComponent: bad entity or component type %d", type);
		return NULL;
	}

	VSE_Component *component = calloc(1, sizeof(VSE_Component));
	component->type = type;
	component->entity = entity;

	size_t size = g_componentTypes[type].size;
	if (size > 0)
	{
		component->data = calloc(1, size);
		if (initData != NULL)
		{
			memcpy(component->data, initData, size);
		}
	}

	VSE_ListAdd(entity->components, component);

	return component;
}


VSE_Component *VSE_AddBehaviour(VSE_Entity *entity, VSE_ComponentUpdateFn update, void *userData)
{
	VSE_Component *component = VSE_AddComponent(entity, VSE_COMPONENT_BEHAVIOUR, NULL);
	if (component == NULL)
	{
		return NULL;
	}

	component->Update = update;
	component->userData = userData;
	return component;
}


VSE_Component *VSE_GetComponentRef(VSE_Entity *entity, VSE_ComponentType type)
{
	if (entity == NULL)
	{
		return NULL;
	}

	for (int i = 0; i < entity->components->size; i++)
	{
		VSE_Component *component = entity->components->elements[i];
		if (component->type == type)
		{
			return component;
		}
	}

	return NULL;
}


void *VSE_GetComponent(VSE_Entity *entity, VSE_ComponentType type)
{
	VSE_Component *component = VSE_GetComponentRef(entity, type);
	return component != NULL ? component->data : NULL;
}


bool VSE_HasComponent(VSE_Entity *entity, VSE_ComponentType type)
{
	return VSE_GetComponentRef(entity, type) != NULL;
}


static void DestroyComponent(VSE_Component *component)
{
	if (component->Destroy != NULL)
	{
		component->Destroy(component);
	}

	free(component->data);
	free(component);
}


void VSE_RemoveComponent(VSE_Entity *entity, VSE_ComponentType type)
{
	VSE_Component *component = VSE_GetComponentRef(entity, type);
	if (component == NULL)
	{
		return;
	}

	VSE_ListRemove(entity->components, component);
	DestroyComponent(component);
}


void VSE_EntityDestroy(VSE_Engine *engine, VSE_Entity *entity)
{
	if (entity == NULL)
	{
		return;
	}

	for (int i = 0; i < entity->components->size; i++)
	{
		DestroyComponent(entity->components->elements[i]);
	}

	VSE_ListDestroy(entity->components);

	if (engine != NULL)
	{
		VSE_ListRemove(engine->allEntities, entity);
		VSE_RemoveGameEntityFromAllDrawLists(engine, entity);
	}

	free(entity);
}


// --- systems ---------------------------------------------------------------

static void UpdateBehaviours(void *data, VSE_Engine *engine, float deltaTime)
{
	for (int i = 0; i < engine->allEntities->size; i++)
	{
		VSE_Entity *entity = engine->allEntities->elements[i];
		if (!entity->active)
		{
			continue;
		}

		for (int j = 0; j < entity->components->size; j++)
		{
			VSE_Component *component = entity->components->elements[j];
			if (component->Update != NULL)
			{
				component->Update(component, engine, deltaTime);
			}
		}
	}
}


VSE_Updatable *VSE_CreateBehaviourUpdatable()
{
	return VSE_CreateUpdatable(NULL, UpdateBehaviours);
}


bool VSE_EntityBounds(VSE_Engine *engine, VSE_Entity *entity, VSE_Vector2Float position,
                      VSE_Vector2Float *outMin, VSE_Vector2Float *outMax)
{
	VSE_Vector2Float size;
	VSE_Vector2Float offset = VSE_VECTOR2_FLOAT_ZERO;

	VSE_Collider *collider = VSE_GetComponent(entity, VSE_COMPONENT_COLLIDER);

	if (collider != NULL)
	{
		size = collider->size;
		offset = collider->offset;
	}
	else
	{
		VSE_SpriteRenderer *sprite = VSE_GetComponent(entity, VSE_COMPONENT_SPRITE_RENDERER);
		if (sprite == NULL)
		{
			return false;
		}

		size = sprite->originalSize;
	}

	float width = size.x * engine->pixelsPerUnit * entity->transform.scale.x;
	float height = size.y * engine->pixelsPerUnit * entity->transform.scale.y;

	float centerX = position.x + offset.x;
	float centerY = position.y + offset.y;

	*outMin = (VSE_Vector2Float){centerX - width / 2.0f, centerY - height / 2.0f};
	*outMax = (VSE_Vector2Float){centerX + width / 2.0f, centerY + height / 2.0f};

	return true;
}


bool VSE_EntitiesOverlap(VSE_Engine *engine, VSE_Entity *a, VSE_Vector2Float positionA,
                         VSE_Entity *b, VSE_Vector2Float positionB)
{
	VSE_Vector2Float minA, maxA, minB, maxB;

	if (!VSE_EntityBounds(engine, a, positionA, &minA, &maxA) ||
	    !VSE_EntityBounds(engine, b, positionB, &minB, &maxB))
	{
		return false;
	}

	return VSE_AreBoxesOverlapping(minA, maxA, minB, maxB);
}