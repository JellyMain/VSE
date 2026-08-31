#pragma once
#include <SDL.h>
#include "VSE/fwd.h"
#include "VSE/types.h"
#include "VSE/list.h"

typedef enum
{
	VSE_WINDOW_SCREEN_SPACE,
	VSE_WINDOW_WORLD_SPACE,
} VSE_WindowRenderType;


typedef enum
{
	VSE_RESIZABLE,
	VSE_FIXED_SIZE,
	VSE_SCALE_WITH_RESIZE
} VSE_WindowType;


typedef struct VSE_Window
{
	SDL_Window *sdlWindow;
	VSE_Vector2Int position;
	VSE_Vector2Int size;
	VSE_Vector2Int viewportOffset;
	VSE_Vector2Int lastFrameSize;
	SDL_Renderer *renderer;
	VSE_WindowRenderType renderType;
	VSE_WindowType windowType;
	VSE_List *entitiesInWindowList;
	VSE_List *gameEntitiesDrawList;
	VSE_List *uiEntitiesDrawList;
	VSE_List *gizmosEntitiesDrawList;
	VSE_Vector2Float windowCenterPoint;
	VSE_Vector2Float upperRightPoint;
	VSE_Vector2Float lowerRightPoint;
	VSE_Vector2Float upperLeftPoint;
	VSE_Vector2Float lowerLeftPoint;
	VSE_GLuint FBO;
	VSE_GLuint FBOTexture;
	VSE_GLuint RBO;
} VSE_Window;


/** @return the window with this SDL window id, or NULL. */
VSE_Window *VSE_GetWindowById(VSE_Engine *engine, uint32_t id);

/** Makes this the window that receives UI interaction, and raises it. */
void VSE_SetFocusWindow(VSE_Engine *engine, VSE_Window *window);

/** Opens an OS window with its own framebuffer and adds it to the engine.
 *  @param renderType WORLD_SPACE shares one world across windows; SCREEN_SPACE is window-local
 *  @param windowType FIXED_SIZE, RESIZABLE, or SCALE_WITH_RESIZE to scale contents
 *  @return the window, owned by the engine */
VSE_Window *VSE_CreateGameWindowWithRenderer(VSE_Engine *engine, VSE_Vector2Int position, VSE_Vector2Int size,
                                     VSE_WindowRenderType renderType,
                                     VSE_WindowType windowType,
                                     char *title);

/** Hash over a window's identity, for use as a dictionary key. */
unsigned int VSE_HashWindow(void *window);

/** Compares two windows by address. */
bool VSE_WindowEquals(void *window1, void *window2);

/** @return the centre of the primary display, for placing a window. */
VSE_Vector2Int VSE_GetDisplayCenterPosition();

/** The per-frame window system: tracks position and size, applies scale-on-resize. */
VSE_Updatable *VSE_CreateWindowsUpdatable();

/** Runs the window system for one frame. Registered via VSE_AddDefaultUpdatables. */
void VSE_UpdateWindows(void *data, VSE_Engine *engine, float deltaTime);
