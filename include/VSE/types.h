#pragma once
#include <stdbool.h>
#include "VSE/fwd.h"

/** Marker consumed by an external metadata generator; expands to nothing.
 *  Defined here so VSE carries no serialization dependency of its own. */
#ifndef SERIALIZABLE
#define SERIALIZABLE
#endif


/** Marks a symbol that is deliberately kept but may currently be unreferenced,
 *  such as a debug helper. */
#if defined(__GNUC__) || defined(__clang__)
#define VSE_MAYBE_UNUSED __attribute__((unused))
#else
#define VSE_MAYBE_UNUSED
#endif

/** Mirrors OpenGL's VSE_GLuint so the public API needs no GL headers. VSE checks
 *  this matches the real VSE_GLuint at compile time (see src/render/opengl.c). */
typedef unsigned int VSE_GLuint;

#define VSE_VECTOR2_FLOAT_ONE (VSE_Vector2Float){1.0f, 1.0f}
#define VSE_VECTOR2_INT_ONE (VSE_Vector2Int){1, 1}
#define VSE_VECTOR2_FLOAT_ZERO (VSE_Vector2Float){0.0f, 0.0f}
#define VSE_VECTOR2_INT_ZERO (VSE_Vector2Int){0,0}


typedef struct VSE_Vector2Int
{
	int x;
	int y;
} VSE_Vector2Int;


typedef struct SERIALIZABLE VSE_Vector2Float
{
	float x;
	float y;
} VSE_Vector2Float;
