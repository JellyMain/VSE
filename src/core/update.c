#include "VSE/update.h"
#include "VSE/render.h"
#include "VSE/input.h"
#include "VSE/ui.h"
#include "VSE/tween.h"
#include "VSE/window.h"
#include "VSE/component.h"
#include "VSE/engine.h"


VSE_UpdateSystem *VSE_CreateUpdateSystem()
{
	VSE_UpdateSystem *updateSystem = calloc(1, sizeof(VSE_UpdateSystem));
	updateSystem->updatables = VSE_ListCreate(0);
	return updateSystem;
}


VSE_Updatable *VSE_CreateUpdatable(void *data, void (*Update)(void *self, VSE_Engine *engine, float deltaTime))
{
	VSE_Updatable *updatable = calloc(1, sizeof(VSE_Updatable));
	updatable->data = data;
	updatable->Update = Update;
	return updatable;
}


void VSE_DestroyUpdatable(VSE_Updatable *updatable)
{
	if (updatable == NULL)
	{
		return;
	}

	free(updatable);
}


void VSE_AddUpdatable(VSE_Engine *engine, VSE_Updatable *updatable)
{
	VSE_ListAdd(engine->updateSystem->updatables, updatable);
}


bool VSE_HasUpdatable(VSE_Engine *engine, VSE_Updatable *updatable)
{
	for (int i = 0; i < engine->updateSystem->updatables->size; i++)
	{
		if (engine->updateSystem->updatables->elements[i] == updatable)
		{
			return true;
		}
	}

	return false;
}


void VSE_RemoveUpdatable(VSE_Engine *engine, VSE_Updatable *updatable)
{
	VSE_ListRemove(engine->updateSystem->updatables, updatable);
}


void VSE_AddDefaultUpdatables(VSE_Engine *engine)
{
	VSE_AddUpdatable(engine, VSE_CreateInputUpdatable());
	VSE_AddUpdatable(engine, VSE_CreateRenderUpdatable());
	VSE_AddUpdatable(engine, VSE_CreateWindowsUpdatable());
	VSE_AddUpdatable(engine, VSE_CreateTweenersUpdatable());
	VSE_AddUpdatable(engine, VSE_CreateBehaviourUpdatable());
}
