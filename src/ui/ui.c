#include <SDL_image.h>
#include <SDL_log.h>
#include <SDL_ttf.h>
#include "VSE/ui.h"
#include "VSE/update.h"
#include "VSE/render.h"
#include "VSE/input.h"
#include "VSE/window.h"
#include "VSE/math.h"
#include "VSE/texture.h"
#include "VSE/material.h"
#include "VSE/config.h"
#include "VSE/component.h"
#include "VSE/engine.h"


static void ReadKeyboardInput(void *data, VSE_Engine *engine, float deltaTime);
static void StartReadingKeyboardInput(VSE_Engine *engine, void *data);


VSE_TextAtlas *VSE_CreateTextAtlas(char *fileName, int fontSize)
{
	VSE_TextAtlas *textAtlas = calloc(1, sizeof(VSE_TextAtlas));
	memset(textAtlas->characterRects, 0, sizeof(textAtlas->characterRects));

	char buffer[150];
	VSE_ResolveAssetPath(buffer, sizeof(buffer), fileName);
	TTF_Font *font = TTF_OpenFont(buffer, fontSize);

	if (font == NULL)
	{
		SDL_Log("TTF_OpenFont failed: %s", TTF_GetError());
		free(textAtlas);
		return NULL;
	}

	char *characters =
			"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~ ";

	int numberOfCharacters = strlen(characters);

	int charWidth;
	int charHeight;
	int charsMaxHeight = 0;

	int atlasWidth = 0;
	int atlasHeight = 0;

	for (int i = 0; i < numberOfCharacters; i++)
	{
		char character = characters[i];
		TTF_SizeText(font, &character, &charWidth, &charHeight);
		if (charHeight > charsMaxHeight)
		{
			charsMaxHeight = charHeight;
		}

		SDL_Rect characterRect = {atlasWidth, 0, charWidth, charHeight};
		textAtlas->characterRects[(int) character] = characterRect;

		atlasWidth += charWidth;
	}

	atlasHeight = charsMaxHeight;

	SDL_Surface *atlasSurface = SDL_CreateRGBSurface(0, atlasWidth, atlasHeight, 32,
	                                                 0x000000FF,
	                                                 0x0000FF00,
	                                                 0x00FF0000,
	                                                 0xFF000000);

	if (!atlasSurface)
	{
		SDL_Log("Unable to create texture surface: %s", SDL_GetError());
		TTF_CloseFont(font);
		free(textAtlas);
		return NULL;
	}

	// Fill with transparent color (RGBA: 0, 0, 0, 0)
	SDL_FillRect(atlasSurface, NULL, SDL_MapRGBA(atlasSurface->format, 0, 0, 0, 0));

	for (int i = 0; i < numberOfCharacters; i++)
	{
		SDL_Rect characterRect = textAtlas->characterRects[(int) characters[i]];
		char text[2] = {characters[i], '\0'};

		SDL_Surface *characterSurface = TTF_RenderText_Blended(
			font, text, (SDL_Color){255, 255, 255, 255});

		if (characterSurface == NULL)
		{
			continue;
		}

		SDL_BlitSurface(characterSurface, NULL, atlasSurface, &characterRect);

		textAtlas->characterRects[(int) characters[i]] = characterRect;

		SDL_FreeSurface(characterSurface);
	}

	textAtlas->atlasTexture = VSE_CreateTextureFromSurface(atlasSurface);

	SDL_FreeSurface(atlasSurface);
	TTF_CloseFont(font);

	return textAtlas;
}


VSE_Entity *VSE_CreateUIEntity(VSE_Engine *engine, VSE_Window *window, VSE_Vector2Float position,
                               VSE_Vector2Float scale, VSE_Entity *parent)
{
	VSE_Entity *entity = VSE_EntityCreate(engine, position, scale);

	VSE_RectTransform rect = {
		.parentScale = VSE_VECTOR2_FLOAT_ONE,
		.lastPosition = position,
		.parent = parent,
		.children = VSE_ListCreate(0),
	};

	if (parent != NULL)
	{
		VSE_RectTransform *parentRect = VSE_GetComponent(parent, VSE_COMPONENT_RECT_TRANSFORM);

		if (parentRect != NULL)
		{
			rect.parentScale.x = parentRect->parentScale.x * parent->transform.scale.x;
			rect.parentScale.y = parentRect->parentScale.y * parent->transform.scale.y;
		}
	}

	VSE_AddComponent(entity, VSE_COMPONENT_RECT_TRANSFORM, &rect);

	if (parent != NULL)
	{
		VSE_RectTransform *parentRect = VSE_GetComponent(parent, VSE_COMPONENT_RECT_TRANSFORM);

		if (parentRect != NULL)
		{
			VSE_ListAdd(parentRect->children, entity);
		}
	}

	VSE_AddUIEntityToDrawList(window, entity);

	return entity;
}


/** Attaches an image, sizing it from the texture. */
static void AddImage(VSE_Entity *entity, VSE_Texture *texture, VSE_Material *material)
{
	VSE_SpriteRenderer image = {
		.texture = texture,
		.material = material != NULL ? material : VSE_CreateMaterial(NULL, NULL),
		.originalSize = {
			texture != NULL ? (float) texture->width : 0.0f,
			texture != NULL ? (float) texture->height : 0.0f
		},
	};

	VSE_AddComponent(entity, VSE_COMPONENT_SPRITE_RENDERER, &image);
}


VSE_Entity *VSE_CreateStaticText(VSE_Engine *engine, VSE_Window *window, const char *text,
                                 SDL_Color textColor, VSE_Vector2Float position,
                                 VSE_Vector2Float scale, VSE_Entity *parent)
{
	char buffer[150];
	VSE_ResolveAssetPath(buffer, sizeof(buffer), VSE_DefaultUIFontPath());
	TTF_Font *font = TTF_OpenFont(buffer, 64);

	if (font == NULL)
	{
		SDL_Log("TTF_OpenFont failed: %s", TTF_GetError());
		return NULL;
	}

	SDL_Surface *textSurface = TTF_RenderText_Solid(font, text, textColor);

	if (textSurface == NULL)
	{
		SDL_Log("TTF_RenderText_Solid failed: %s", TTF_GetError());
		TTF_CloseFont(font);
		return NULL;
	}

	VSE_Entity *entity = VSE_CreateUIEntity(engine, window, position, scale, parent);
	AddImage(entity, VSE_CreateTextureFromSurface(textSurface), NULL);

	TTF_CloseFont(font);
	SDL_FreeSurface(textSurface);

	return entity;
}


VSE_Entity *VSE_CreateDynamicText(VSE_Engine *engine, VSE_Window *window, char *text,
                                  VSE_Vector2Float position, VSE_Vector2Float scale,
                                  VSE_Entity *parent)
{
	VSE_Entity *entity = VSE_CreateUIEntity(engine, window, position, scale, parent);

	VSE_DynamicText dynamicText = {.text = text};
	VSE_AddComponent(entity, VSE_COMPONENT_DYNAMIC_TEXT, &dynamicText);

	return entity;
}


/** A zeroed VSE_Vector2Float in a desc means "use this default". */
static VSE_Vector2Float OrDefault(VSE_Vector2Float value, VSE_Vector2Float fallback)
{
	if (value.x == 0.0f && value.y == 0.0f)
	{
		return fallback;
	}

	return value;
}


VSE_Entity *VSE_CreateButton(VSE_Engine *engine, const VSE_ButtonDesc *desc)
{
	if (desc == NULL || desc->window == NULL)
	{
		SDL_Log("VSE_CreateButton: desc and desc->window are required");
		return NULL;
	}

	VSE_Vector2Float scale = OrDefault(desc->scale, VSE_VECTOR2_FLOAT_ONE);

	VSE_Entity *button = VSE_CreateUIEntity(engine, desc->window, desc->position, scale,
	                                        desc->parent);

	VSE_Texture *texture = desc->texture != NULL
		                       ? desc->texture
		                       : VSE_CreateRect(desc->size, desc->backgroundColor);

	AddImage(button, texture, desc->material);

	VSE_Interactable interactable = {
		.data = desc->userData,
		.OnClick = desc->OnClick,
		.OnClickAnimation = desc->OnClickAnimation,
		.OnHover = desc->OnHover,
		.OnHoverExit = desc->OnHoverExit,
	};
	VSE_AddComponent(button, VSE_COMPONENT_INTERACTABLE, &interactable);

	button->userData = desc->userData;

	VSE_CreateGizmo(engine, desc->window, (SDL_Color){255, 0, 0, 255}, 1, button);

	if (desc->text != NULL && desc->text[0] != '\0')
	{
		VSE_Vector2Float textScale = OrDefault(desc->textScale, (VSE_Vector2Float){0.2f, 0.2f});
		VSE_CreateStaticText(engine, desc->window, desc->text, desc->textColor, desc->position,
		                     textScale, button);
	}

	return button;
}


VSE_Entity *VSE_CreateInputField(VSE_Engine *engine, const VSE_InputFieldDesc *desc)
{
	if (desc == NULL || desc->window == NULL)
	{
		SDL_Log("VSE_CreateInputField: desc and desc->window are required");
		return NULL;
	}

	VSE_Vector2Float scale = OrDefault(desc->scale, VSE_VECTOR2_FLOAT_ONE);
	int maxLength = desc->maxLength > 0 ? desc->maxLength : 10;

	VSE_Entity *field = VSE_CreateUIEntity(engine, desc->window, desc->position, scale,
	                                       desc->parent);

	VSE_Texture *texture = desc->texture != NULL
		                       ? desc->texture
		                       : VSE_CreateRect(desc->size, desc->backgroundColor);

	AddImage(field, texture, desc->material);

	VSE_InputField inputField = {
		.text = calloc(maxLength + 1, sizeof(char)),
		.maxLength = maxLength,
		.readKeyboardUpdatable = NULL,
	};
	VSE_AddComponent(field, VSE_COMPONENT_INPUT_FIELD, &inputField);

	VSE_InputField *stored = VSE_GetComponent(field, VSE_COMPONENT_INPUT_FIELD);
	stored->readKeyboardUpdatable = VSE_CreateUpdatable(field, ReadKeyboardInput);

	/* Clicking the field is what starts keyboard capture. */
	VSE_Interactable interactable = {
		.data = field,
		.OnClick = StartReadingKeyboardInput,
	};
	VSE_AddComponent(field, VSE_COMPONENT_INTERACTABLE, &interactable);

	VSE_CreateGizmo(engine, desc->window, (SDL_Color){255, 0, 0, 255}, 1, field);

	VSE_Vector2Float textScale = OrDefault(desc->textScale, VSE_VECTOR2_FLOAT_ONE);
	VSE_CreateDynamicText(engine, desc->window, stored->text, desc->position, textScale, field);

	return field;
}


void VSE_CreateGizmo(VSE_Engine *engine, VSE_Window *window, SDL_Color color, float thickness,
                     VSE_Entity *connected)
{
	VSE_GizmoEntity *gizmo = calloc(1, sizeof(VSE_GizmoEntity));
	gizmo->connectedEntity = connected;
	gizmo->color = color;
	gizmo->thickness = thickness;

	VSE_ListAdd(engine->allGizmosEntities, gizmo);
	VSE_ListAdd(window->gizmosEntitiesDrawList, gizmo);
}


static void StartReadingKeyboardInput(VSE_Engine *engine, void *data)
{
	VSE_Entity *field = data;
	VSE_InputField *inputField = VSE_GetComponent(field, VSE_COMPONENT_INPUT_FIELD);

	if (inputField == NULL)
	{
		return;
	}

	if (!VSE_HasUpdatable(engine, inputField->readKeyboardUpdatable))
	{
		VSE_AddUpdatable(engine, inputField->readKeyboardUpdatable);
	}
}


static void ReadKeyboardInput(void *data, VSE_Engine *engine, float deltaTime)
{
	VSE_Entity *field = data;
	VSE_InputField *inputField = VSE_GetComponent(field, VSE_COMPONENT_INPUT_FIELD);

	if (inputField == NULL)
	{
		return;
	}

	VSE_KeyboardKey key = VSE_GetKeyPressed();

	if (key == SDLK_UNKNOWN)
	{
		return;
	}

	if (key == SDLK_ESCAPE)
	{
		if (VSE_HasUpdatable(engine, inputField->readKeyboardUpdatable))
		{
			VSE_RemoveUpdatable(engine, inputField->readKeyboardUpdatable);
		}

		return;
	}

	int currentLength = strlen(inputField->text);

	if (key == SDLK_BACKSPACE)
	{
		if (currentLength > 0)
		{
			inputField->text[currentLength - 1] = '\0';
		}

		return;
	}

	if (currentLength < inputField->maxLength)
	{
		inputField->text[currentLength] = key;
		inputField->text[currentLength + 1] = '\0';
	}
}


/** Pushes this entity's accumulated scale onto its children. */
static void UpdateChildrenScale(VSE_Entity *entity)
{
	VSE_RectTransform *rect = VSE_GetComponent(entity, VSE_COMPONENT_RECT_TRANSFORM);

	if (rect == NULL || rect->children->size == 0)
	{
		return;
	}

	VSE_Vector2Float accumulated = {
		rect->parentScale.x * entity->transform.scale.x,
		rect->parentScale.y * entity->transform.scale.y
	};

	for (int i = 0; i < rect->children->size; i++)
	{
		VSE_Entity *child = rect->children->elements[i];
		VSE_RectTransform *childRect = VSE_GetComponent(child, VSE_COMPONENT_RECT_TRANSFORM);

		if (childRect != NULL)
		{
			childRect->parentScale = accumulated;
		}
	}
}


/** Carries children along by however far this entity moved since last frame. */
static void UpdateChildrenPosition(VSE_Entity *entity)
{
	VSE_RectTransform *rect = VSE_GetComponent(entity, VSE_COMPONENT_RECT_TRANSFORM);

	if (rect == NULL || rect->children->size == 0)
	{
		return;
	}

	VSE_Vector2Float delta = {
		entity->transform.position.x - rect->lastPosition.x,
		entity->transform.position.y - rect->lastPosition.y
	};

	if (delta.x != 0.0f || delta.y != 0.0f)
	{
		for (int i = 0; i < rect->children->size; i++)
		{
			VSE_Entity *child = rect->children->elements[i];
			child->transform.position.x += delta.x;
			child->transform.position.y += delta.y;
		}
	}

	rect->lastPosition = entity->transform.position;
}


/** Hover and click dispatch. An entity takes part purely by carrying a
 *  VSE_Interactable -- there is no type to switch on. */
static void HandleInteractions(VSE_Engine *engine)
{
	if (engine->focusedWindow == NULL)
	{
		return;
	}

	VSE_List *drawList = engine->focusedWindow->uiEntitiesDrawList;

	for (int i = 0; i < drawList->size; i++)
	{
		VSE_Entity *entity = VSE_ListGet(drawList, i);
		VSE_Interactable *interactable = VSE_GetComponent(entity, VSE_COMPONENT_INTERACTABLE);
		VSE_SpriteRenderer *image = VSE_GetComponent(entity, VSE_COMPONENT_SPRITE_RENDERER);

		if (interactable == NULL || image == NULL)
		{
			continue;
		}

		VSE_Vector2Float mousePosition = VSE_GetMousePosition();

		VSE_Vector2Float boundsMin = {
			entity->transform.position.x - image->size.x / 2,
			entity->transform.position.y - image->size.y / 2
		};
		VSE_Vector2Float boundsMax = {
			entity->transform.position.x + image->size.x / 2,
			entity->transform.position.y + image->size.y / 2
		};

		if (VSE_IsPointInBounds(mousePosition, boundsMin, boundsMax))
		{
			if (!interactable->isHovered)
			{
				interactable->isHovered = true;

				if (interactable->OnHover != NULL)
				{
					interactable->OnHover(engine, entity);
				}
			}

			if (VSE_IsLeftMouseButtonClicked())
			{
				if (interactable->OnClick != NULL)
				{
					interactable->OnClick(engine, interactable->data);
				}

				if (interactable->OnClickAnimation != NULL)
				{
					interactable->OnClickAnimation(engine, entity);
				}
			}
		}
		else
		{
			if (interactable->isHovered && interactable->OnHoverExit != NULL)
			{
				interactable->OnHoverExit(engine, entity);
			}

			interactable->isHovered = false;
		}
	}
}


static void UpdateUIElements(void *data, VSE_Engine *engine, float deltaTime)
{
	for (int i = 0; i < engine->allEntities->size; i++)
	{
		VSE_Entity *entity = engine->allEntities->elements[i];

		if (!VSE_HasComponent(entity, VSE_COMPONENT_RECT_TRANSFORM))
		{
			continue;
		}

		UpdateChildrenScale(entity);
		UpdateChildrenPosition(entity);
	}

	HandleInteractions(engine);
}


VSE_Updatable *VSE_CreateUIUpdatable()
{
	return VSE_CreateUpdatable(NULL, UpdateUIElements);
}
