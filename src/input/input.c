#include <SDL_events.h>
#include "VSE/input.h"
#include "VSE/vse.h"
#include "VSE/update.h"
#include "VSE/window.h"

static void ProcessInput(void *data, VSE_Engine *engine, float deltaTime);

static bool isLeftMouseButtonClicked = false;
static VSE_KeyboardKey pressedKey = SDLK_UNKNOWN;
static bool hasKeyPressed = false;


VSE_Vector2Float VSE_GetMoveDirection()
{
	VSE_Vector2Float input = {0, 0};

	const Uint8 *keyState = SDL_GetKeyboardState(NULL);

	if (keyState[SDL_SCANCODE_W])
	{
		input.y = -1;
	}
	if (keyState[SDL_SCANCODE_S])
	{
		input.y = 1;
	}
	if (keyState[SDL_SCANCODE_A])
	{
		input.x = -1;
	}
	if (keyState[SDL_SCANCODE_D])
	{
		input.x = 1;
	}

	return input;
}


VSE_Updatable *VSE_CreateInputUpdatable()
{
	VSE_Updatable *updatable = VSE_CreateUpdatable(NULL, ProcessInput);
	return updatable;
}


static void ProcessInput(void *data, VSE_Engine *engine, float deltaTime)
{
	isLeftMouseButtonClicked = false;
	pressedKey = SDLK_UNKNOWN;
	hasKeyPressed = false;

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
			{
				exit(0);
				break;
			}

			case SDL_WINDOWEVENT:
			{
				switch (event.window.event)
				{
					case SDL_WINDOWEVENT_ENTER:
						VSE_Window *window = VSE_GetWindowById(engine, event.window.windowID);
						VSE_SetFocusWindow(engine, window);
						break;

					default:
						break;
				}
				break;
			}


			case SDL_MOUSEBUTTONDOWN:
			{
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					printf("Left mouse button clicked\n");
					isLeftMouseButtonClicked = true;
				}
				break;
			}


			case SDL_KEYDOWN:
			{
				pressedKey = event.key.keysym.sym;
				hasKeyPressed = true;
				break;
			}

			default:
				break;
		}
	}
}


bool VSE_IsLeftMouseButtonClicked()
{
	return isLeftMouseButtonClicked;
}


VSE_KeyboardKey VSE_GetKeyPressed()
{
	return pressedKey;
}


VSE_Vector2Float VSE_GetMousePosition()
{
	int xPosition;
	int yPosition;

	SDL_GetMouseState(&xPosition, &yPosition);

	return (VSE_Vector2Float){xPosition, yPosition};
}