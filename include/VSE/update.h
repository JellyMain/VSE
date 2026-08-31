#pragma once
#include <stdbool.h>
#include "VSE/fwd.h"
#include "VSE/list.h"

struct VSE_Engine;


typedef struct VSE_Updatable
{
	void *data;
	void (*Update)(void *self, struct VSE_Engine *engine, float deltaTime);
} VSE_Updatable;


typedef struct VSE_UpdateSystem
{
	VSE_List *updatables;
} VSE_UpdateSystem;


/** Creates the registry that holds per-frame updatables. */
VSE_UpdateSystem *VSE_CreateUpdateSystem();
/** Wraps a function as a per-frame callback.
 *  @param data relayed back as the callback's first argument, never dereferenced here */
VSE_Updatable *VSE_CreateUpdatable(void *data, void (*Update)(void *self, VSE_Engine *engine, float deltaTime));
/** Frees an updatable. Does not remove it from the engine first. */
void VSE_DestroyUpdatable(VSE_Updatable *updatable);

/** Registers an updatable to be ticked every frame. */
void VSE_AddUpdatable(VSE_Engine *engine, VSE_Updatable *updatable);
/** @return true when this updatable is currently registered. */
bool VSE_HasUpdatable(VSE_Engine *engine, VSE_Updatable *updatable);
/** Unregisters an updatable so it stops being ticked. */
void VSE_RemoveUpdatable(VSE_Engine *engine, VSE_Updatable *updatable);

/** Registers the engine's own per-frame systems: input, render, windows,
 *  tweens and UI. Call after clearing a scene. */
void VSE_AddDefaultUpdatables(VSE_Engine *engine);
