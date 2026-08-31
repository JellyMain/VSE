/* Bounds, clamping and interpolation. These take plain values -- the maths
 * layer deliberately knows nothing about entities. */
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "VSE/math.h"

#define NEAR(a, b) (fabsf((a) - (b)) < 0.0001f)

int main(void)
{
    /* Clamping */
    assert(NEAR(VSE_ClampFloat(5.0f, 0.0f, 3.0f), 3.0f));
    assert(NEAR(VSE_ClampFloat(-5.0f, 0.0f, 3.0f), 0.0f));
    assert(NEAR(VSE_ClampFloat(1.5f, 0.0f, 3.0f), 1.5f));
    assert(VSE_ClampInt(10, 0, 5) == 5);
    assert(VSE_ClampInt(-1, 0, 5) == 0);

    VSE_Vector2Float cf = VSE_ClampVector2Float((VSE_Vector2Float){9, -9},
                                                (VSE_Vector2Float){0, 0},
                                                (VSE_Vector2Float){5, 5});
    assert(NEAR(cf.x, 5.0f) && NEAR(cf.y, 0.0f));

    /* Points in bounds */
    VSE_Vector2Float lo = {0, 0}, hi = {10, 10};
    assert(VSE_IsPointInBounds((VSE_Vector2Float){5, 5}, lo, hi));
    assert(VSE_IsPointInBounds((VSE_Vector2Float){0, 0}, lo, hi));   /* inclusive */
    assert(!VSE_IsPointInBounds((VSE_Vector2Float){11, 5}, lo, hi));

    /* A box fully inside its bounds, and one poking out */
    assert(VSE_IsBoxInBounds((VSE_Vector2Float){2, 2}, (VSE_Vector2Float){8, 8}, lo, hi));
    assert(!VSE_IsBoxInBounds((VSE_Vector2Float){2, 2}, (VSE_Vector2Float){12, 8}, lo, hi));
    assert(VSE_IsBoxInBounds(lo, hi, lo, hi));                        /* exact fit */

    /* Overlap is exclusive at the edges: touching is not overlapping */
    assert(VSE_AreBoxesOverlapping((VSE_Vector2Float){0, 0}, (VSE_Vector2Float){10, 10},
                                   (VSE_Vector2Float){5, 5}, (VSE_Vector2Float){15, 15}));
    assert(!VSE_AreBoxesOverlapping((VSE_Vector2Float){0, 0}, (VSE_Vector2Float){10, 10},
                                    (VSE_Vector2Float){10, 0}, (VSE_Vector2Float){20, 10}));
    assert(!VSE_AreBoxesOverlapping((VSE_Vector2Float){0, 0}, (VSE_Vector2Float){10, 10},
                                    (VSE_Vector2Float){20, 20}, (VSE_Vector2Float){30, 30}));

    /* Interpolation */
    assert(NEAR(VSE_LerpFloat(0.0f, 10.0f, 0.0f), 0.0f));
    assert(NEAR(VSE_LerpFloat(0.0f, 10.0f, 1.0f), 10.0f));
    assert(NEAR(VSE_LerpFloat(0.0f, 10.0f, 0.5f), 5.0f));
    assert(VSE_LerpInt(0, 10, 0.5f) == 5);

    VSE_Vector2Float lv = VSE_LerpVector2Float((VSE_Vector2Float){0, 10},
                                               (VSE_Vector2Float){10, 0}, 0.5f);
    assert(NEAR(lv.x, 5.0f) && NEAR(lv.y, 5.0f));

    /* Percentage change drives the window's scale-on-resize behaviour */
    assert(NEAR(VSE_GetPercentageChange(100.0f, 150.0f), 50.0f));
    assert(NEAR(VSE_GetPercentageChange(100.0f, 50.0f), -50.0f));

    /* World -> screen subtracts the window's viewport offset */
    VSE_Vector2Float s = VSE_WorldToScreen((VSE_Vector2Float){300, 200}, (VSE_Vector2Int){100, 50});
    assert(NEAR(s.x, 200.0f) && NEAR(s.y, 150.0f));

    puts("test_math: PASSED");
    return 0;
}
