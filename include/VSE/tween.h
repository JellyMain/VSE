#pragma once
#include <stdbool.h>
#include "VSE/fwd.h"
#include "VSE/types.h"
#include "VSE/list.h"
#include "VSE/update.h"

typedef enum
{
	VSE_VECTOR2_FLOAT_TWEEN,
	VSE_FLOAT_TWEEN,
	VSE_VECTOR2_INT_TWEEN,
	VSE_INT_TWEEN,
} VSE_TweenType;


typedef enum
{
	VSE_IN_SINE,
	VSE_OUT_SINE,
	VSE_IN_OUT_SINE,
	VSE_IN_QUAD,
	VSE_OUT_QUAD,
	VSE_IN_OUT_QUAD,
	VSE_IN_CUBIC,
	VSE_OUT_CUBIC,
	VSE_IN_OUT_CUBIC,
	VSE_IN_QUART,
	VSE_OUT_QUART,
	VSE_IN_OUT_QUART,
	VSE_IN_QUINT,
	VSE_OUT_QUINT,
	VSE_IN_OUT_QUINT,
	VSE_IN_EXPO,
	VSE_OUT_EXPO,
	VSE_IN_OUT_EXPO,
	VSE_IN_CIRC,
	VSE_OUT_CIRC,
	VSE_IN_OUT_CIRC,
	VSE_IN_BACK,
	VSE_OUT_BACK,
	VSE_IN_OUT_BACK,
	VSE_IN_ELASTIC,
	VSE_OUT_ELASTIC,
	VSE_IN_OUT_ELASTIC,
	VSE_IN_BOUNCE,
	VSE_OUT_BOUNCE,
	VSE_IN_OUT_BOUNCE,
	VSE_LINEAR,
} VSE_TweenEasingType;


typedef union
{
	struct
	{
		VSE_Vector2Float fromValue;
		VSE_Vector2Float endValue;
	} vector2FloatTween;

	struct
	{
		float fromValue;
		float endValue;
	} floatTween;

	struct
	{
		VSE_Vector2Int fromValue;
		VSE_Vector2Int endValue;
	} vector2IntTween;

	struct
	{
		int fromValue;
		int endValue;
	} intTween;
} VSE_TweenData;


typedef struct VSE_Tween
{
	VSE_TweenType tweenType;
	VSE_TweenData tweenData;
	float duration;
	float elapsedTime;
	bool isStarted;
	bool isFinished;
	bool destroyOnComplete;
	VSE_TweenEasingType easingType;
	void *target;
	int id;
} VSE_Tween;


typedef struct VSE_TweenSequence
{
	VSE_List *tweeners;
	bool isStarted;
	bool isFinished;
} VSE_TweenSequence;


/** Animates *target from data.fromValue to data.endValue over duration seconds.
 *  @param tweenType MUST match the VSE_TweenData member you filled
 *  @param target written to directly; must outlive the tween
 *  @param destroyOnComplete free the tween when it finishes
 *  @return the tween; pass it to VSE_PlayTween to start it */
VSE_Tween *VSE_CreateTween(VSE_TweenType tweenType, void *target, VSE_TweenData tweenData, float duration,
                   bool destroyOnComplete,
                   VSE_TweenEasingType easingType);


/** Creates an empty sequence whose steps run one after another. */
VSE_TweenSequence *VSE_CreateTweenSequence();

/** Starts a sequence, applying the same one-animation-per-target rule. */
void VSE_PlayTweenSequence(VSE_Engine *engine, VSE_TweenSequence *tweenSequence);

/** Starts a tween. If the target is already being animated, that animation is
 *  finished first so two tweens never fight over one value. */
void VSE_PlayTween(VSE_Engine *engine, VSE_Tween *tween);

/** The per-frame tween system. Registered via VSE_AddDefaultUpdatables. */
VSE_Updatable *VSE_CreateTweenersUpdatable();

/** Appends a step. Steps run in the order added. */
void VSE_AddTweenToSequence(VSE_TweenSequence *tweenSequence, VSE_Tween *tween);

/** Frees a tween. Needed only when destroyOnComplete was false. */
void VSE_DestroyTween(VSE_Tween *tween);

/** Frees a sequence and its steps. */
void VSE_DestroySequence(VSE_TweenSequence *tweenSequence);
