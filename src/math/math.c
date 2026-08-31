#include "VSE/math.h"


float VSE_ClampFloat(float value, float min, float max)
{
	if (value < min)
	{
		return min;
	}
	if (value > max)
	{
		return max;
	}

	return value;
}


int VSE_ClampInt(int value, int min, int max)
{
	if (value < min)
	{
		return min;
	}
	if (value > max)
	{
		return max;
	}
	return value;
}


VSE_Vector2Int VSE_ClampVector2Int(VSE_Vector2Int value, VSE_Vector2Int min, VSE_Vector2Int max)
{
	VSE_Vector2Int result = value;

	if (value.x < min.x)
	{
		result.x = min.x;
	}
	else if (value.x > max.x)
	{
		result.x = max.x;
	}

	if (value.y < min.y)
	{
		result.y = min.y;
	}
	else if (value.y > max.y)
	{
		result.y = max.y;
	}


	return result;
}


VSE_Vector2Float VSE_ClampVector2Float(VSE_Vector2Float value, VSE_Vector2Float min, VSE_Vector2Float max)
{
	VSE_Vector2Float result = value;
	if (value.x < min.x)
	{
		result.x = min.x;
	}
	else if (value.x > max.x)
	{
		result.x = max.x;
	}
	if (value.y < min.y)
	{
		result.y = min.y;
	}
	else if (value.y > max.y)
	{
		result.y = max.y;
	}
	return result;
}


bool VSE_IsBoxInBounds(VSE_Vector2Float boxMin, VSE_Vector2Float boxMax, VSE_Vector2Float boundsMin,
                   VSE_Vector2Float boundsMax)
{
	return boxMin.x >= boundsMin.x &&
	       boxMax.x <= boundsMax.x &&
	       boxMin.y >= boundsMin.y &&
	       boxMax.y <= boundsMax.y;
}


bool VSE_IsPointInBounds(VSE_Vector2Float point, VSE_Vector2Float boundsMin, VSE_Vector2Float boundsMax)
{
	return point.x >= boundsMin.x && point.x <= boundsMax.x &&
	       point.y >= boundsMin.y && point.y <= boundsMax.y;
}


bool VSE_AreBoxesOverlapping(VSE_Vector2Float minA, VSE_Vector2Float maxA, VSE_Vector2Float minB,
                         VSE_Vector2Float maxB)
{
	return minA.x < maxB.x &&
	       maxA.x > minB.x &&
	       minA.y < maxB.y &&
	       maxA.y > minB.y;
}


float VSE_GetPercentageChange(float oldValue, float newValue)
{
	if (oldValue == 0)
	{
		return 0;
	}
	return (newValue - oldValue) * 100 / oldValue;
}


VSE_Vector2Float VSE_GetPercentageChangeVector2(VSE_Vector2Float oldValue, VSE_Vector2Float newValue)
{
	VSE_Vector2Float result;
	result.x = VSE_GetPercentageChange(oldValue.x, newValue.x);
	result.y = VSE_GetPercentageChange(oldValue.y, newValue.y);
	return result;
}


float VSE_LerpFloat(float a, float b, float t)
{
	return (1 - t) * a + t * b;
}


VSE_Vector2Float VSE_LerpVector2Float(VSE_Vector2Float a, VSE_Vector2Float b, float t)
{
	VSE_Vector2Float result;
	result.x = VSE_LerpFloat(a.x, b.x, t);
	result.y = VSE_LerpFloat(a.y, b.y, t);
	return result;
}


int VSE_LerpInt(int a, int b, float t)
{
	return (int) VSE_LerpFloat((float) a, (float) b, t);
}


VSE_Vector2Int VSE_LerpVector2Int(VSE_Vector2Int a, VSE_Vector2Int b, float t)
{
	VSE_Vector2Int result;
	result.x = VSE_LerpInt(a.x, b.x, t);
	result.y = VSE_LerpInt(a.y, b.y, t);
	return result;
}