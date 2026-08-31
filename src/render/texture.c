#include "VSE/texture.h"
#include "VSE/config.h"
#include <SDL_image.h>
#include <SDL_surface.h>
#include "glad/glad.h"
#include "VSE/render.h"


VSE_Texture *VSE_CreateRect(VSE_Vector2Float size, SDL_Color color)
{
	VSE_Texture *texture = calloc(1, sizeof(VSE_Texture));

	unsigned char pixel[4] = {color.r, color.g, color.b, color.a};

	GLuint textureId;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


	texture->textureId = textureId;
	texture->height = size.y;
	texture->width = size.x;

	return texture;
}


VSE_Texture *VSE_CreateTextureFromSurface(SDL_Surface *surface)
{
	VSE_Texture *texture = calloc(1, sizeof(VSE_Texture));

	SDL_Surface *convertedSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);

	GLuint textureId;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);

	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
	             convertedSurface->pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


	texture->textureId = textureId;
	texture->height = convertedSurface->h;
	texture->width = convertedSurface->w;

	SDL_FreeSurface(convertedSurface);

	return texture;
}


VSE_Material *VSE_CreateMaterial(char *fragShaderName, char *vertShaderName)
{
	VSE_Material *material = calloc(1, sizeof(VSE_Material));

	material->shaderProgram = VSE_CreateShaderProgram(vertShaderName, fragShaderName);
	glUseProgram(material->shaderProgram);
	material->materialUniforms = VSE_DictionaryCreate(VSE_HashString, VSE_StringEquals);
	VSE_AddUniformToMaterial(material, "projection", VSE_UNIFORM_MAT4F, NULL);
	return material;
}


void VSE_AddUniformToMaterial(VSE_Material *material, char *uniformName, VSE_UniformType uniformType, void *value)
{
	VSE_UniformTypeValuePair *uniformTypeValuePair = calloc(1, sizeof(VSE_UniformTypeValuePair));
	uniformTypeValuePair->uniformType = uniformType;
	uniformTypeValuePair->uniformValue = value;
	VSE_DictionaryAdd(material->materialUniforms, uniformName, uniformTypeValuePair);
}


VSE_Texture *VSE_LoadTexture(const char *fileName)
{
	VSE_Texture *texture = calloc(1, sizeof(VSE_Texture));

	char buffer[150];
	VSE_ResolveAssetPath(buffer, sizeof(buffer), fileName);

	SDL_Surface *surface = IMG_Load(buffer);
	if (!surface)
	{
		SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR,
		               "Failed to load image: %s, Error: %s", buffer, IMG_GetError());
		return NULL;
	}

	GLuint textureId;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);

	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	texture->textureId = textureId;
	texture->height = surface->h;
	texture->width = surface->w;

	SDL_FreeSurface(surface);

	return texture;
}