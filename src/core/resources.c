#include "VSE/config.h"
#include <stddef.h>
#include <stdio.h>


static const char *g_behavioursRoot = "";
static const char *g_assetRoot = "";
static const char *g_shaderRoot = "";
static const char *g_uiFontPath = "";

void VSE_SetResourceRoots(const VSE_Config *config)
{
    g_assetRoot = config->assetRoot ? config->assetRoot : "";
    g_shaderRoot = config->shaderRoot ? config->shaderRoot : "";
    g_uiFontPath = config->uiFontPath ? config->uiFontPath : "";
    g_behavioursRoot = config->behavioursRoot ? config->behavioursRoot : "";
}

void VSE_ResolveAssetPath(char *buffer, size_t bufferSize, const char *fileName)
{
    snprintf(buffer, bufferSize, "%s%s", g_assetRoot, fileName);
}

void VSE_ResolveOriginalBehaviourDLL(char *buffer, size_t bufferSize,
                                     const char *fileName)
{
    snprintf(buffer, bufferSize, "%s%s.dll", g_behavioursRoot, fileName);
}

void VSE_ResolveCopyBehaviourDLL(char *buffer, size_t bufferSize,
                                 const char *fileName, int copyIndex)
{
    snprintf(buffer, bufferSize, "%s%s_%d.dll", g_behavioursRoot, fileName,
             copyIndex);
}

void VSE_ResolveShaderPath(char *buffer, size_t bufferSize,
                           const char *fileName)
{
    snprintf(buffer, bufferSize, "%s%s", g_shaderRoot, fileName);
}

const char *VSE_DefaultUIFontPath(void)
{
    return g_uiFontPath;
}
