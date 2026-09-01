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

}


void VSE_CleanUpUIEntity(VSE_Entity *entity)
{
	if (entity == NULL)
	{
		return;
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
