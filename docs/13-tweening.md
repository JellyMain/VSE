# Tweening

A tween animates a value in place, over time, through an easing curve. It writes directly to memory
you give it, so it works on anything — an entity's position, a UI element's scale, a float you own.

## Creating and playing

```c
VSE_TweenData data = {
    .vector2FloatTween = {
        .fromValue = {1, 1},
        .endValue  = {1.5f, 1.5f},
    }
};

VSE_Tween *t = VSE_CreateTween(
    VSE_VECTOR2_FLOAT_TWEEN,   /* must match the union member you filled */
    &uiEntity->parentScale,    /* target -- written to directly          */
    data,
    0.5f,                      /* duration in seconds                    */
    true,                      /* free the tween when it finishes        */
    VSE_OUT_QUINT);

VSE_PlayTween(engine, t);
```

The target is a raw pointer and is **not** kept alive by the tween — it must outlive the animation.
Tweening a field of an entity you are about to destroy is a use-after-free.

`VSE_TweenType` must match the `VSE_TweenData` member you filled: `VSE_VECTOR2_FLOAT_TWEEN` with
`vector2FloatTween`, `VSE_FLOAT_TWEEN` with `floatTween`, and likewise for the int variants. Nothing
checks this — a mismatch writes the wrong number of bytes.

## One tween per target

Playing a tween on a target that is already animating **finishes the existing one first**, snapping
it to its end value. That is what makes hover-in/hover-out work: waggling the mouse over a button
cannot leave two tweens fighting over its scale.

The engine tracks this in a dictionary keyed by target pointer. If the running tween belongs to a
sequence, the whole sequence is finished, not just that one step.

## Sequences

```c
VSE_TweenSequence *seq = VSE_CreateTweenSequence();
VSE_AddTweenToSequence(seq, growTween);
VSE_AddTweenToSequence(seq, shrinkTween);
VSE_PlayTweenSequence(engine, seq);
```

Steps run one after another in the order added.

## Easing

31 curves, named `VSE_IN_*`, `VSE_OUT_*`, `VSE_IN_OUT_*` over Sine, Quad, Cubic, Quart, Quint, Expo,
Circ, Back, Elastic and Bounce, plus `VSE_LINEAR`.

Rules of thumb: `VSE_OUT_*` for things entering or responding to input (fast, then settling),
`VSE_IN_*` for things leaving, `VSE_IN_OUT_*` for moves between two resting states. `VSE_OUT_BACK`
and `VSE_OUT_ELASTIC` overshoot, which reads as playful; `VSE_OUT_QUINT` is a safe default for UI.

## Lifetime

With `destroyOnComplete = true` the tween frees itself on finish — the usual choice for
fire-and-forget UI animation. With `false` it stays in the registry and you own it; free it with
`VSE_DestroyTween` (or `VSE_DestroySequence`).

`VSE_CleanUpScene` clears the tween-target dictionary, so tweens do not survive a scene change.

## Worked example: a button that grows on hover

```c
static void OnHover(VSE_Engine *engine, VSE_Entity *button)
{
    VSE_RectTransform *rect = VSE_GetComponent(button, VSE_COMPONENT_RECT_TRANSFORM);
    VSE_TweenData d = { .vector2FloatTween = { .fromValue = {1, 1}, .endValue = {1.5f, 1.5f} } };
    VSE_PlayTween(engine, VSE_CreateTween(VSE_VECTOR2_FLOAT_TWEEN, &rect->parentScale,
                                          d, 0.5f, true, VSE_OUT_QUINT));
}

static void OnHoverExit(VSE_Engine *engine, VSE_Entity *button)
{
    VSE_RectTransform *rect = VSE_GetComponent(button, VSE_COMPONENT_RECT_TRANSFORM);
    VSE_TweenData d = { .vector2FloatTween = { .fromValue = {1.5f, 1.5f}, .endValue = {1, 1} } };
    VSE_PlayTween(engine, VSE_CreateTween(VSE_VECTOR2_FLOAT_TWEEN, &rect->parentScale,
                                          d, 0.5f, true, VSE_OUT_QUINT));
}
```

`parentScale` rather than `scale`, so the button's label scales with it.
