#include <stdio.h>
#include "VSE/config.h"


static const char *g_assetRoot = "";
static const char *g_shaderRoot = "";
static const char *g_uiFontPath = "";


void VSE_SetResourceRoots(const VSE_Config *config)
{
	g_assetRoot = config->assetRoot ? config->assetRoot : "";
	g_shaderRoot = config->shaderRoot ? config->shaderRoot : "";
	g_uiFontPath = config->uiFontPath ? config->uiFontPath : "";
}


void VSE_ResolveAssetPath(char *buffer, size_t bufferSize, const char *fileName)
{
	snprintf(buffer, bufferSize, "%s%s", g_assetRoot, fileName);
}


void VSE_ResolveShaderPath(char *buffer, size_t bufferSize, const char *fileName)
{
	snprintf(buffer, bufferSize, "%s%s", g_shaderRoot, fileName);
}


const char *VSE_DefaultUIFontPath(void)
{
	return g_uiFontPath;
}