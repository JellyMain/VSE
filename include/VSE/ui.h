#pragma once
#include <SDL.h>
#include <stdbool.h>
#include "VSE/fwd.h"
#include "VSE/types.h"
#include "VSE/texture.h"
#include "VSE/material.h"
#include "VSE/update.h"
#include "VSE/list.h"
#include "VSE/window.h"
#include "VSE/component.h"


/** How VSE_RenderDynamicText places a string relative to its position. */
typedef enum
{
	VSE_HORIZONTAL_CENTERED,
	VSE_HORIZONTAL_NOT_CENTERED,
} VSE_DynamicTextHorizontalAlignment;


/** How to build a button. Zero-initialise and set only what you need:
 *
 *  @code
 *  VSE_Entity *play = VSE_CreateButton(engine, &(VSE_ButtonDesc){
 *      .window   = menuWindow,
 *      .position = menuWindow->windowCenterPoint,
 *      .texture  = buttonTexture,
 *      .text     = "Play",
 *      .userData = game,
 *      .OnClick  = OnPlayClicked,
 *  });
 *  @endcode
 */
typedef struct VSE_ButtonDesc
{
	/** Required: the window the button is drawn in. */
	VSE_Window *window;
	VSE_Vector2Float position;
	/** Defaults to {1,1} when left zero. */
	VSE_Vector2Float scale;

	/** NULL builds a solid rect from `size` and `backgroundColor` instead. */
	VSE_Texture *texture;
	/** NULL uses the default shader pair. */
	VSE_Material *material;
	VSE_Vector2Float size;
	SDL_Color backgroundColor;

	/** NULL or empty for a button with no label. */
	const char *text;
	/** Defaults to {0.2f, 0.2f} when left zero. */
	VSE_Vector2Float textScale;
	SDL_Color textColor;

	/** Relayed to OnClick, never dereferenced by the engine. */
	void *userData;
	void (*OnClick)(VSE_Engine *engine, void *userData);
	void (*OnClickAnimation)(VSE_Engine *engine, VSE_Entity *button);
	void (*OnHover)(VSE_Engine *engine, VSE_Entity *button);
	void (*OnHoverExit)(VSE_Engine *engine, VSE_Entity *button);

	/** Inherit scale and movement from another UI entity, or NULL. */
	VSE_Entity *parent;
} VSE_ButtonDesc;


/** How to build a text input field. */
typedef struct VSE_InputFieldDesc
{
	/** Required: the window the field is drawn in. */
	VSE_Window *window;
	VSE_Vector2Float position;
	/** Defaults to {1,1} when left zero. */
	VSE_Vector2Float scale;

	/** NULL builds a solid rect from `size` and `backgroundColor` instead. */
	VSE_Texture *texture;
	/** NULL uses the default shader pair. */
	VSE_Material *material;
	VSE_Vector2Float size;
	SDL_Color backgroundColor;

	/** Defaults to {1,1} when left zero. */
	VSE_Vector2Float textScale;
	/** Defaults to 10 when left zero. */
	int maxLength;

	VSE_Entity *parent;
} VSE_InputFieldDesc;


/** Creates a UI entity: a transform plus a VSE_RectTransform, registered in the
 *  window's UI draw list. Add an image or text component to make it visible, or
 *  an interactable to make it respond to the pointer. */
VSE_Entity *VSE_CreateUIEntity(VSE_Engine *engine, VSE_Window *window, VSE_Vector2Float position,
                               VSE_Vector2Float scale, VSE_Entity *parent);

/** Rasterises text once into a texture, using config.uiFontPath at 64pt.
 *  Sharp but fixed: changing the string means creating a new entity.
 *  @param parent inherit scale and movement from another UI entity, or NULL */
VSE_Entity *VSE_CreateStaticText(VSE_Engine *engine, VSE_Window *window, const char *text,
                                 SDL_Color textColor, VSE_Vector2Float position,
                                 VSE_Vector2Float scale, VSE_Entity *parent);

/** Creates text drawn from the glyph atlas each frame, so `text` may change.
 *  The string is borrowed, not copied -- it must outlive the entity. */
VSE_Entity *VSE_CreateDynamicText(VSE_Engine *engine, VSE_Window *window, char *text,
                                  VSE_Vector2Float position, VSE_Vector2Float scale,
                                  VSE_Entity *parent);

/** Creates a clickable button, plus a child label when `desc->text` is set.
 *  @return the button entity, carrying a VSE_Interactable */
VSE_Entity *VSE_CreateButton(VSE_Engine *engine, const VSE_ButtonDesc *desc);

/** Creates a focusable text field. Click to focus, Backspace deletes, Escape
 *  releases. Read the text via VSE_GetComponent(e, VSE_COMPONENT_INPUT_FIELD). */
VSE_Entity *VSE_CreateInputField(VSE_Engine *engine, const VSE_InputFieldDesc *desc);

/** Attaches a debug outline drawn around `connected` while debugMode is on. */
void VSE_CreateGizmo(VSE_Engine *engine, VSE_Window *window, SDL_Color color, float thickness,
                     VSE_Entity *connected);

/** Bakes ASCII glyphs into one texture for VSE_RenderDynamicText.
 *  @param fontPath relative to assetRoot */
VSE_TextAtlas *VSE_CreateTextAtlas(char *fontPath, int fontSize);

/** The per-frame UI system: parent scale/position propagation, hover and clicks. */
VSE_Updatable *VSE_CreateUIUpdatable();

/** Recomputes one window's contained-entity list and applies scale-on-resize. */
void VSE_UpdateWindow(VSE_Engine *engine, VSE_Window *window);
