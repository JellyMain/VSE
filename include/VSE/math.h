#pragma once
#include <stdbool.h>
#include "VSE/fwd.h"
#include "VSE/types.h"
#include "VSE/component.h"

#define VSE_PI 3.14159265359

/** @return value limited to the inclusive range [min, max]. */
float VSE_ClampFloat(float value, float min, float max);

/** @return value limited to the inclusive range [min, max]. */
int VSE_ClampInt(int value, int min, int max);

/** Clamps each component independently. */
VSE_Vector2Int VSE_ClampVector2Int(VSE_Vector2Int value, VSE_Vector2Int min, VSE_Vector2Int max);

/** Clamps each component independently. */
VSE_Vector2Float VSE_ClampVector2Float(VSE_Vector2Float value, VSE_Vector2Float min, VSE_Vector2Float max);

/** True when the box [boxMin,boxMax] lies entirely inside [boundsMin,boundsMax]. */
bool VSE_IsBoxInBounds(VSE_Vector2Float boxMin, VSE_Vector2Float boxMax, VSE_Vector2Float boundsMin,
                   VSE_Vector2Float boundsMax);

/** @return true when the point lies inside the box, edges included. */
bool VSE_IsPointInBounds(VSE_Vector2Float point, VSE_Vector2Float boundsMin, VSE_Vector2Float boundsMax);

/** True when two axis-aligned boxes overlap. */
bool VSE_AreBoxesOverlapping(VSE_Vector2Float minA, VSE_Vector2Float maxA, VSE_Vector2Float minB,
                         VSE_Vector2Float maxB);

/** @return the change from oldValue to newValue as a percentage. */
float VSE_GetPercentageChange(float oldValue, float newValue);

/** Percentage change per component; drives scale-on-resize. */
VSE_Vector2Float VSE_GetPercentageChangeVector2(VSE_Vector2Float oldValue, VSE_Vector2Float newValue);

/** @return a linearly interpolated between a and b at t in 0..1. */
float VSE_LerpFloat(float a, float b, float t);

/** Component-wise linear interpolation. */
VSE_Vector2Float VSE_LerpVector2Float(VSE_Vector2Float a, VSE_Vector2Float b, float t);

/** Linear interpolation, truncated to int. */
int VSE_LerpInt(int a, int b, float t);

/** Component-wise linear interpolation, truncated to int. */
VSE_Vector2Int VSE_LerpVector2Int(VSE_Vector2Int a, VSE_Vector2Int b, float t);


/** @return a world position converted into a window's local pixels. */
VSE_Vector2Float VSE_WorldToScreen(VSE_Vector2Float worldPosition, VSE_Vector2Int viewportOffset);
