#include <SDL_image.h>
#include <SDL_log.h>
#include <SDL_ttf.h>
#include "VSE/ui.h"
#include "VSE/texture.h"
#include "VSE/config.h"




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
