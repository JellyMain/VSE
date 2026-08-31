#pragma once
#include <SDL.h>
#include "VSE/fwd.h"
#include "VSE/types.h"
#include "VSE/update.h"

typedef SDL_Keycode VSE_KeyboardKey;


/** @return WASD direction with components in -1..1, read live from the keyboard.
 *  Note y increases downward, so W yields y = -1. */
VSE_Vector2Float VSE_GetMoveDirection();

/** @return the cursor position in pixels, relative to the focused window. */
VSE_Vector2Float VSE_GetMousePosition();

/** @return true only during the frame in which the press arrived. */
bool VSE_IsLeftMouseButtonClicked();

/** The per-frame input system. Must run first; VSE_AddDefaultUpdatables does that. */
VSE_Updatable *VSE_CreateInputUpdatable();

/** @return the last key pressed this frame, or SDLK_UNKNOWN. Holds a single
 *  key, so it cannot report two pressed in the same frame. */
VSE_KeyboardKey VSE_GetKeyPressed();
