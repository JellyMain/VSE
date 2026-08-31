#include "VSE/math.h"


VSE_Vector2Float VSE_WorldToScreen(VSE_Vector2Float worldPosition, VSE_Vector2Int viewportOffset)
{
	return (VSE_Vector2Float)
	{
		worldPosition.x - viewportOffset.x,
		worldPosition.y - viewportOffset.y
	};
}