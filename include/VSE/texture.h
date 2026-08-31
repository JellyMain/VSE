#pragma once
#include <SDL.h>
#include "VSE/fwd.h"
#include "VSE/types.h"

typedef struct VSE_Texture
{
	VSE_GLuint textureId;
	int width;
	int height;
} VSE_Texture;

typedef struct VSE_TextAtlas
{
	SDL_Rect characterRects[256];
	VSE_Texture *atlasTexture;
} VSE_TextAtlas;

/** Uploads an SDL surface as a GL texture. The surface is not taken over. */
VSE_Texture *VSE_CreateTextureFromSurface(SDL_Surface *surface);

/** Loads an image from under assetRoot. @return NULL if the file could not be read. */
VSE_Texture *VSE_LoadTexture(const char *fileName);

/** Creates a solid-colour texture of the given size, with no image file involved. */
VSE_Texture *VSE_CreateRect(VSE_Vector2Float size, SDL_Color color);
