#pragma once
#include <stddef.h>
#include <stdbool.h>
#include "VSE/fwd.h"

/** Startup configuration. The engine owns no paths of its own: the game says
 *  where its assets live, and where the shader tree it wants to use lives. */
typedef struct VSE_Config
{
	/** Prefix for texture and font loads, e.g. "Assets/". */
	const char *assetRoot;
	/** Prefix for shader loads, e.g. "Shaders/". */
	const char *shaderRoot;
	/** Font used for the debug overlay, relative to assetRoot. */
	const char *debugFontPath;
	int debugFontSize;
	/** Font used by static text when no other is given, relative to assetRoot. */
	const char *uiFontPath;
	int pixelsPerUnit;
	bool debugMode;
} VSE_Config;


/** Records where resources live. Process-global, because the loaders below are
 *  called from places that have no engine pointer to hand; this engine supports
 *  a single instance per process. */
void VSE_SetResourceRoots(const VSE_Config *config);

/** Writes assetRoot + fileName into buffer. */
void VSE_ResolveAssetPath(char *buffer, size_t bufferSize, const char *fileName);

/** Writes shaderRoot + fileName into buffer. */
void VSE_ResolveShaderPath(char *buffer, size_t bufferSize, const char *fileName);

/** Font for static text when a call site does not name one. Never NULL. */
const char *VSE_DefaultUIFontPath(void);
