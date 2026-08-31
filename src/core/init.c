#include <SDL_image.h>
#include <SDL_ttf.h>
#include "VSE/engine.h"
#include "VSE/render.h"
#include "VSE/ui.h"
#include "VSE/config.h"


void VSE_InitSDL2(void)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		SDL_Log("Sdl video init error");
		return;
	}

	IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);


	if (TTF_Init() == -1)
	{
		SDL_Log("TTF_Init failed: %s", TTF_GetError());
	}
}


VSE_Engine *VSE_Init(const VSE_Config *config)
{
	if (!config)
	{
		SDL_Log("VSE_Init: config is NULL");
		return NULL;
	}

	VSE_SetResourceRoots(config);

	VSE_Engine *engine = VSE_CreateEngine(config);

	VSE_InitSDL2();
	VSE_InitOpenGL(engine);

	engine->textAtlas = VSE_CreateTextAtlas((char *) config->debugFontPath, config->debugFontSize);

	return engine;
}