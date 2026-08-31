#include <stdbool.h>
#include "VSE/engine.h"
#include "VSE/render.h"
#include "VSE/window.h"
#include "VSE/update.h"


VSE_Engine *VSE_CreateEngine(const VSE_Config *config)
{
	VSE_Engine *engine = calloc(1, sizeof(VSE_Engine));

	engine->updateSystem = VSE_CreateUpdateSystem();
	engine->pixelsPerUnit = config->pixelsPerUnit;
	engine->debugMode = config->debugMode;
	engine->allGizmosEntities = VSE_ListCreate(0);
	engine->allEntities = VSE_ListCreate(0);
	engine->allTweeners = VSE_ListCreate(0);
	engine->allTweenSequences = VSE_ListCreate(0);
	engine->allWindows = VSE_ListCreate(0);
	engine->tweenTargetsDictionary = VSE_DictionaryCreate(VSE_HashPointer, VSE_PointerEquals);
	engine->focusedWindow = NULL;
	engine->time = 0;
	engine->debugData.fps = 0;
	engine->debugData.drawCalls = 0;

	return engine;
}


void VSE_Tick(VSE_Engine *engine)
{
	engine->time = SDL_GetTicks() / 1000.0f;

	Uint64 now = SDL_GetPerformanceCounter();
	if (engine->lastFrameTime == 0)
	{
		engine->lastFrameTime = now;
	}

	engine->deltaTime = (now - engine->lastFrameTime) / (float) SDL_GetPerformanceFrequency();
	engine->lastFrameTime = now;

	engine->fpsTimer += engine->deltaTime;
	engine->countedFrames++;

	if (engine->fpsTimer >= 1.0f)
	{
		engine->debugData.fps = engine->countedFrames / engine->fpsTimer;
		engine->countedFrames = 0;
		engine->fpsTimer = 0.0f;
	}

	VSE_List *updatables = engine->updateSystem->updatables;

	for (int i = 0; i < updatables->size; i++)
	{
		VSE_Updatable *updatable = updatables->elements[i];
		updatable->Update(updatable->data, engine, engine->deltaTime);
	}
}


/* SDL event filters take no user pointer of their own, so the engine under the
 * filter is held here. This engine supports one instance per process. */
static VSE_Engine *g_liveResizeEngine = NULL;


static int LiveResizeFilter(void *userdata, SDL_Event *event)
{
	(void) userdata;

	if (event->type == SDL_SYSWMEVENT || event->type == SDL_WINDOWEVENT)
	{
		if (g_liveResizeEngine != NULL)
		{
			VSE_UpdateRenderer(NULL, g_liveResizeEngine, 0);
			VSE_UpdateWindows(NULL, g_liveResizeEngine, 0);
		}
	}

	return 1;
}


void VSE_EnableLiveResizeRendering(VSE_Engine *engine)
{
	g_liveResizeEngine = engine;
	SDL_SetEventFilter(LiveResizeFilter, NULL);
}


void VSE_DisableLiveResizeRendering(void)
{
	SDL_SetEventFilter(NULL, NULL);
	g_liveResizeEngine = NULL;
}
