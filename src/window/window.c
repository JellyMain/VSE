#include "VSE/window.h"

#include "VSE/render.h"
#include "VSE/update.h"
#include "VSE/ui.h"
#include "VSE/math.h"
#include "VSE/component.h"
#include "VSE/engine.h"


VSE_Window *VSE_GetWindowById(VSE_Engine *engine, uint32_t id)
{
	for (int i = 0; i < engine->allWindows->size; i++)
	{
		VSE_Window *window = VSE_ListGet(engine->allWindows, i);

		if (SDL_GetWindowID(window->sdlWindow) == id)
		{
			return window;
		}
	}

	SDL_Log("VSE_Window with id %d not found", id);
	return NULL;
}


void VSE_SetFocusWindow(VSE_Engine *engine, VSE_Window *window)
{
	engine->focusedWindow = window;
	SDL_RaiseWindow(window->sdlWindow);
}


VSE_Window *VSE_CreateGameWindowWithRenderer(VSE_Engine *engine, VSE_Vector2Int position, VSE_Vector2Int size,
                                     VSE_WindowRenderType renderType, VSE_WindowType windowType, char *title)
{
	Uint32 windowFlags = SDL_WINDOW_SHOWN;

	if (windowType != VSE_FIXED_SIZE)
	{
		windowFlags |= SDL_WINDOW_RESIZABLE;
	}

	windowFlags |= SDL_WINDOW_OPENGL;

	SDL_Window *sdlWindow = SDL_CreateWindow(title, position.x, position.y, size.x, size.y,
	                                         windowFlags);

	VSE_Window *window = calloc(1, sizeof(VSE_Window));

	SDL_Renderer *renderer = SDL_CreateRenderer(sdlWindow, -1,
	                                            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	VSE_Vector2Float windowCenterPoint = {size.x / 2, size.y / 2};
	VSE_Vector2Float windowUpperLeftPoint = {0, 0};
	VSE_Vector2Float windowLowerLeftPoint = {0, size.y};
	VSE_Vector2Float windowUpperRightPoint = {size.x, 0};
	VSE_Vector2Float windowLowerRightPoint = {size.x, size.y};

	window->sdlWindow = sdlWindow;

	window->renderer = renderer;
	window->position = position;
	window->windowCenterPoint = windowCenterPoint;
	window->upperRightPoint = windowUpperRightPoint;
	window->lowerRightPoint = windowLowerRightPoint;
	window->upperLeftPoint = windowUpperLeftPoint;
	window->lowerLeftPoint = windowLowerLeftPoint;
	window->size = size;
	window->renderType = renderType;
	window->windowType = windowType;
	window->lastFrameSize = size;
	window->entitiesInWindowList = VSE_ListCreate(0);
	window->gameEntitiesDrawList = VSE_ListCreate(0);
	window->uiEntitiesDrawList = VSE_ListCreate(0);
	window->gizmosEntitiesDrawList = VSE_ListCreate(0);

	VSE_CreateWindowFBO(engine, window);

	VSE_ListAdd(engine->allWindows, window);

	VSE_SetFocusWindow(engine, window);

	return window;
}


void VSE_UpdateWindow(VSE_Engine *engine, VSE_Window *window)
{
	VSE_ListClear(window->entitiesInWindowList);

	for (int i = 0; i < engine->allEntities->size; i++)
	{
		VSE_Entity *entity = engine->allEntities->elements[i];

		VSE_Vector2Float minBounds = {window->position.x, window->position.y};
		VSE_Vector2Float maxBounds = {
			window->position.x + window->size.x, window->position.y + window->size.y
		};

		VSE_Vector2Float entityMin, entityMax;


		if (VSE_IsBoxInBounds(entityMin, entityMax, minBounds, maxBounds))
		{
			VSE_ListAdd(window->entitiesInWindowList, entity);
		}
	}


	SDL_GetWindowPosition(window->sdlWindow, &window->position.x, &window->position.y);
	window->viewportOffset = window->position;


	SDL_GetWindowSize(window->sdlWindow, &window->size.x, &window->size.y);

	VSE_Vector2Float resizePercentDelta = VSE_GetPercentageChangeVector2(
		(VSE_Vector2Float){window->lastFrameSize.x, window->lastFrameSize.y},
		(VSE_Vector2Float){window->size.x, window->size.y});


	window->lastFrameSize = window->size;

	if (window->windowType == VSE_SCALE_WITH_RESIZE)
	{
		for (int i = 0; i < window->entitiesInWindowList->size; i++)
		{
			VSE_Entity *entity = window->entitiesInWindowList->elements[i];

			if (resizePercentDelta.x != 0.0f)
			{
				entity->transform.scale.x =
						entity->transform.scale.x * (1.0f + resizePercentDelta.x / 100.0f);
			}

			if (resizePercentDelta.y != 0.0f)
			{
				entity->transform.scale.y =
						entity->transform.scale.y * (1.0f + resizePercentDelta.y / 100.0f);
			}
		}
	}
}


void VSE_UpdateWindows(void *data, VSE_Engine *engine, float deltaTime)
{
	for (int i = 0; i < engine->allWindows->size; i++)
	{
		VSE_Window *window = engine->allWindows->elements[i];
		VSE_UpdateWindow(engine, window);
	}
}


VSE_Updatable *VSE_CreateWindowsUpdatable()
{
	VSE_Updatable *updatable = VSE_CreateUpdatable(NULL, VSE_UpdateWindows);
	return updatable;
}


VSE_Vector2Int VSE_GetDisplayCenterPosition()
{
	SDL_DisplayMode displayMode;
	SDL_GetCurrentDisplayMode(0, &displayMode);
	int centerX = displayMode.w / 2;
	int centerY = displayMode.h / 2;

	return (VSE_Vector2Int){centerX, centerY};
}


unsigned int VSE_HashWindow(void *window)
{
	if (window == NULL)
	{
		return 0;
	}

	VSE_Window *windowPtr = window;

	unsigned int hash = 5381;

	hash = (hash << 5) + hash + (unsigned int) (uintptr_t) windowPtr->sdlWindow;

	hash = (hash << 5) + hash + (unsigned int) windowPtr->renderType;
	hash = (hash << 5) + hash + (unsigned int) windowPtr->windowType;

	hash = (hash << 5) + hash + (unsigned int) (uintptr_t) windowPtr->renderer;


	return hash;
}


bool VSE_WindowEquals(void *window1, void *window2)
{
	if (window1 == NULL || window2 == NULL)
	{
		return false;
	}

	VSE_Window *window1Ptr = window1;
	VSE_Window *window2Ptr = window2;

	return window1Ptr == window2Ptr;
}
