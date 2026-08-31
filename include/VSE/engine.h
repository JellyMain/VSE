#pragma once
#include <SDL.h>
#include <stdbool.h>
#include "VSE/fwd.h"
#include "VSE/texture.h"
#include "VSE/render.h"
#include "VSE/window.h"
#include "VSE/update.h"
#include "VSE/list.h"
#include "VSE/dictionary.h"
#include "VSE/config.h"
#include "VSE/component.h"
#include "VSE/material.h"

typedef struct VSE_DebugData
{
	float fps;
	int drawCalls;
} VSE_DebugData;


/** Engine-owned state. Contains nothing game-specific: a game keeps its own
 *  context struct and reaches it through the per-object userData pointers that
 *  the engine relays but never dereferences. */
typedef struct VSE_Engine
{
	VSE_UpdateSystem *updateSystem;
	VSE_Renderer *renderer;
	int pixelsPerUnit;
	VSE_List *allEntities;
	VSE_List *allGizmosEntities;
	VSE_List *allTweeners;
	VSE_List *allTweenSequences;
	VSE_List *allWindows;
	VSE_Dictionary *tweenTargetsDictionary;
	VSE_TextAtlas *textAtlas;
	bool debugMode;
	SDL_GLContext glContext;
	SDL_Window *hiddenWindow;
	VSE_Window *focusedWindow;
	/** Seconds since the previous tick. Set by VSE_Tick. */
	float deltaTime;
	/** Seconds since startup. Set by VSE_Tick. */
	float time;
	VSE_DebugData debugData;
	/* Frame-timing accumulators, owned by VSE_Tick. */
	Uint64 lastFrameTime;
	float fpsTimer;
	float countedFrames;
} VSE_Engine;


/** Destroys every entity, UI entity, gizmo, window and updatable the engine
 *  owns, and clears the tween registry. Call before building a new scene. */
void VSE_CleanUpScene(VSE_Engine *engine);

/** Frees a material, its uniform map and its GL program. */
void VSE_CleanUpMaterial(VSE_Material *material);
/** Destroys a window and its SDL resources. */
void VSE_CleanUpWindow(VSE_Window *window);
/** Frees an entity along with its sprite's texture and material. */
void VSE_CleanUpGameEntity(VSE_Entity *entity);
/** Frees a UI entity, its children list, texture, material and interaction. */
void VSE_CleanUpUIEntity(VSE_Entity *entity);


/** Allocates the engine and its registries. Owns no game state. */
VSE_Engine *VSE_CreateEngine(const VSE_Config *config);


/** Brings up SDL, OpenGL and the engine. Returns the engine, ready to tick. */
VSE_Engine *VSE_Init(const VSE_Config *config);

/** Brings up SDL video, image and TTF. VSE_Init calls this for you. */
void VSE_InitSDL2(void);


/** Advances one frame: updates `time` and `deltaTime`, refreshes the FPS
 *  counter, then runs every registered updatable. Call this once per iteration
 *  of your own loop, so the game stays in control of state transitions. */
void VSE_Tick(VSE_Engine *engine);

/** Keeps windows rendering while the OS is dragging or resizing one, which
 *  otherwise blocks the main loop until the drag ends.
 *  Disable it before tearing a scene down -- it renders from inside an SDL
 *  event callback, so it must not run while windows are being destroyed. */
void VSE_EnableLiveResizeRendering(VSE_Engine *engine);
/** Removes the live-resize event filter. Call before tearing a scene down: it
 *  renders from inside an SDL callback and must not run while windows die. */
void VSE_DisableLiveResizeRendering(void);
