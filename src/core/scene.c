#include "VSE/engine.h"
#include "VSE/update.h"
#include "VSE/dictionary.h"
#include "VSE/component.h"
#include "glad/glad.h"


void VSE_CleanUpMaterial(VSE_Material *material)
{
	for (int i = 0; i < material->materialUniforms->allPairs->size; i++)
	{
		VSE_KeyValuePair *pair = VSE_DictionaryGetPair(material->materialUniforms, i);
		VSE_UniformTypeValuePair *typeValuePair = pair->value;
		free(typeValuePair);
	}

	VSE_DictionaryClear(material->materialUniforms);
	VSE_DictionaryDestroy(material->materialUniforms);
	glDeleteProgram(material->shaderProgram);
	free(material);
}


void VSE_CleanUpWindow(VSE_Window *window)
{
	if (window == NULL)
	{
		return;
	}

	SDL_DestroyWindow(window->sdlWindow);
	SDL_DestroyRenderer(window->renderer);

	free(window);
}


void VSE_CleanUpGameEntity(VSE_Entity *entity)
{
	if (entity == NULL)
	{
		return;
	}

	VSE_SpriteRenderer *sprite = VSE_GetComponent(entity, VSE_COMPONENT_SPRITE_RENDERER);

	if (sprite != NULL)
	{
		if (sprite->texture != NULL)
		{
			glDeleteTextures(1, &sprite->texture->textureId);
			free(sprite->texture);
		}

		if (sprite->material != NULL)
		{
			VSE_CleanUpMaterial(sprite->material);
		}
	}

	// The engine's registries are cleared wholesale by VSE_CleanUpScene, so this
	// frees the entity without the per-list removal VSE_EntityDestroy does.
	VSE_EntityDestroy(NULL, entity);
}


void VSE_CleanUpUIEntity(VSE_Entity *entity)
{
	if (entity == NULL)
	{
		return;
	}

	VSE_RectTransform *rect = VSE_GetComponent(entity, VSE_COMPONENT_RECT_TRANSFORM);

	if (rect != NULL)
	{
		VSE_ListDestroy(rect->children);
	}

	VSE_InputField *inputField = VSE_GetComponent(entity, VSE_COMPONENT_INPUT_FIELD);

	if (inputField != NULL)
	{
		free(inputField->text);
		VSE_DestroyUpdatable(inputField->readKeyboardUpdatable);
	}

	VSE_CleanUpGameEntity(entity);
}


void VSE_CleanUpScene(VSE_Engine *engine)
{
	for (int i = 0; i < engine->updateSystem->updatables->size; ++i)
	{
		VSE_Updatable *updatable = engine->updateSystem->updatables->elements[i];
		VSE_DestroyUpdatable(updatable);
	}

	for (int i = 0; i < engine->allGizmosEntities->size; i++)
	{
		VSE_GizmoEntity *gizmoEntity = engine->allGizmosEntities->elements[i];
		free(gizmoEntity);
	}

	for (int i = 0; i < engine->allEntities->size; i++)
	{
		VSE_Entity *entity = engine->allEntities->elements[i];

		/* UI entities own a little more: a children list, maybe a text buffer. */
		if (VSE_HasComponent(entity, VSE_COMPONENT_RECT_TRANSFORM))
		{
			VSE_CleanUpUIEntity(entity);
		}
		else
		{
			VSE_CleanUpGameEntity(entity);
		}
	}



	VSE_ListClear(engine->updateSystem->updatables);
	VSE_ListClear(engine->allEntities);
	VSE_ListClear(engine->allGizmosEntities);

	for (int i = 0; i < engine->allWindows->size; i++)
	{
		VSE_Window *window = engine->allWindows->elements[i];
		VSE_CleanUpWindow(window);
	}

	VSE_ListClear(engine->allWindows);
	VSE_DictionaryClear(engine->tweenTargetsDictionary);
}
