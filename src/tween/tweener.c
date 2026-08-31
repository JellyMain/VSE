#include <SDL_timer.h>
#include "VSE/tween.h"
#include "VSE/math.h"
#include "VSE/update.h"
#include "VSE/engine.h"

static float EaseLinear(float t);

static float EaseInSine(float t);

static float EaseOutSine(float t);

static float EaseInOutSine(float t);

static float EaseInQuad(float t);

static float EaseOutQuad(float t);

static float EaseInOutQuad(float t);

static float EaseInCubic(float t);

static float EaseOutCubic(float t);

static float EaseInOutCubic(float t);

static float EaseInQuart(float t);

static float EaseOutQuart(float t);

static float EaseInOutQuart(float t);

static float EaseInQuint(float t);

static float EaseOutQuint(float t);

static float EaseInOutQuint(float t);

static float EaseInExpo(float t);

static float EaseOutExpo(float t);

static float EaseInOutExpo(float t);

static float EaseInCirc(float t);

static float EaseOutCirc(float t);

static float EaseInOutCirc(float t);

static float EaseInBack(float t);

static float EaseOutBack(float t);

static float EaseInOutBack(float t);

static float EaseInElastic(float t);

static float EaseOutElastic(float t);

static float EaseInOutElastic(float t);

static float EaseInBounce(float t);

static float EaseOutBounce(float t);

static float EaseInOutBounce(float t);


static void FinishTween(VSE_Engine *engine, VSE_Tween *tween);

static void FinishSequence(VSE_Engine *engine, VSE_TweenSequence *tweenSequence);

static void UpdateTween(VSE_Engine *engine, VSE_Tween *tween, float deltaTime);


VSE_Tween *VSE_CreateTween(VSE_TweenType tweenType, void *target, VSE_TweenData tweenData, float duration,
                   bool destroyOnComplete,
                   VSE_TweenEasingType easingType)
{
	VSE_Tween *tween = calloc(1, sizeof(VSE_Tween));
	tween->destroyOnComplete = destroyOnComplete;
	tween->isStarted = false;
	tween->isFinished = false;
	tween->tweenType = tweenType;
	tween->tweenData = tweenData;
	tween->duration = duration;
	tween->elapsedTime = 0;
	tween->easingType = easingType;
	tween->target = target;
	tween->id = rand();

	return tween;
}


static void SetActiveTweenForTarget(VSE_Engine *engine, VSE_Tween *tween)
{
	if (VSE_DictionaryHasKey(engine->tweenTargetsDictionary, tween->target))
	{
		VSE_Tween *playingTween = VSE_DictionaryGet(engine->tweenTargetsDictionary, tween->target);
		if (playingTween != NULL)
		{
			bool foundInSequence = false;

			for (int i = 0; i < engine->allTweenSequences->size; i++)
			{
				VSE_TweenSequence *tweenSequence = engine->allTweenSequences->elements[i];

				for (int j = 0; j < tweenSequence->tweeners->size; j++)
				{
					VSE_Tween *sequenceTween = tweenSequence->tweeners->elements[j];
					if (sequenceTween == playingTween)
					{
						foundInSequence = true;
						FinishSequence(engine, tweenSequence);
						break;
					}
				}
			}

			if (!foundInSequence)
			{
				FinishTween(engine, playingTween);
			}
		}

		VSE_DictionaryChangeValue(engine->tweenTargetsDictionary, tween->target, tween);
	}
	else
	{
		VSE_DictionaryAdd(engine->tweenTargetsDictionary, tween->target, tween);
	}
}


void VSE_PlayTween(VSE_Engine *engine, VSE_Tween *tween)
{
	SetActiveTweenForTarget(engine, tween);

	VSE_ListAdd(engine->allTweeners, tween);
	tween->isStarted = true;
}


VSE_TweenSequence *VSE_CreateTweenSequence()
{
	VSE_TweenSequence *tweenSequence = calloc(1, sizeof(VSE_TweenSequence));
	tweenSequence->tweeners = VSE_ListCreate(0);
	return tweenSequence;
}


void VSE_PlayTweenSequence(VSE_Engine *engine, VSE_TweenSequence *tweenSequence)
{
	tweenSequence->isStarted = true;
	VSE_ListAdd(engine->allTweenSequences, tweenSequence);
}


void VSE_AddTweenToSequence(VSE_TweenSequence *tweenSequence, VSE_Tween *tween)
{
	if (!tweenSequence->isStarted)
	{
		tween->destroyOnComplete = false;
		VSE_ListAdd(tweenSequence->tweeners, tween);
	}
	else
	{
		printf("Sequence is already started\n");
	}
}


static void UpdateSequences(VSE_Engine *engine)
{
	for (int i = engine->allTweenSequences->size - 1; i >= 0; i--)
	{
		VSE_TweenSequence *tweenSequence = engine->allTweenSequences->elements[i];

		for (int j = 0; j < tweenSequence->tweeners->size; j++)
		{
			VSE_Tween *tween = tweenSequence->tweeners->elements[j];

			if (!tween->isStarted)
			{
				VSE_PlayTween(engine, tween);
				break;
			}
			if (tween->isStarted && !tween->isFinished)
			{
				break;
			}
		}

		VSE_Tween *lastTween = tweenSequence->tweeners->elements[tweenSequence->tweeners->size - 1];

		if (lastTween->isFinished)
		{
			FinishSequence(engine, tweenSequence);
		}
	}
}


static void UpdateTweeners(void *data, VSE_Engine *engine, float deltaTime)
{
	UpdateSequences(engine);

	for (int i = engine->allTweeners->size - 1; i >= 0; i--)
	{
		VSE_Tween *tween = engine->allTweeners->elements[i];

		UpdateTween(engine, tween, deltaTime);
	}
}


VSE_Updatable *VSE_CreateTweenersUpdatable()
{
	VSE_Updatable *updatable = VSE_CreateUpdatable(NULL, UpdateTweeners);
	return updatable;
}


static void FinishSequence(VSE_Engine *engine, VSE_TweenSequence *tweenSequence)
{
	for (int i = 0; i < tweenSequence->tweeners->size; i++)
	{
		VSE_Tween *tween = tweenSequence->tweeners->elements[i];
		if (!tween->isFinished)
		{
			FinishTween(engine, tween);
		}
	}

	VSE_ListRemove(engine->allTweenSequences, tweenSequence);
	VSE_DestroySequence(tweenSequence);
}


static void FinishTween(VSE_Engine *engine, VSE_Tween *tween)
{
	switch (tween->tweenType)
	{
		case VSE_VECTOR2_FLOAT_TWEEN:
			*(VSE_Vector2Float *) tween->target = tween->tweenData.vector2FloatTween.endValue;
			break;
		case VSE_FLOAT_TWEEN:
			*(float *) tween->target = tween->tweenData.floatTween.endValue;
			break;

		case VSE_VECTOR2_INT_TWEEN:
			*(VSE_Vector2Int *) tween->target = tween->tweenData.vector2IntTween.endValue;
			break;

		case VSE_INT_TWEEN:
			*(int *) tween->target = tween->tweenData.intTween.endValue;
			break;
	}

	VSE_DictionaryChangeValue(engine->tweenTargetsDictionary, tween->target, NULL);

	if (tween->destroyOnComplete)
	{
		if (tween->isStarted)
		{
			VSE_ListRemove(engine->allTweeners, tween);
			VSE_DestroyTween(tween);
		}
	}
	else
	{
		if (tween->isStarted)
		{
			VSE_ListRemove(engine->allTweeners, tween);
			tween->isFinished = true;
		}
	}
}


static void UpdateTween(VSE_Engine *engine, VSE_Tween *tween, float deltaTime)
{
	if (tween->isFinished)
	{
		return;
	}

	tween->elapsedTime += deltaTime;

	float t = tween->elapsedTime / tween->duration;

	switch (tween->easingType)
	{
		case VSE_LINEAR:
			t = EaseLinear(t);
			break;
		case VSE_IN_SINE:
			t = EaseInSine(t);
			break;
		case VSE_OUT_SINE:
			t = EaseOutSine(t);
			break;
		case VSE_IN_OUT_SINE:
			t = EaseInOutSine(t);
			break;
		case VSE_IN_QUAD:
			t = EaseInQuad(t);
			break;
		case VSE_OUT_QUAD:
			t = EaseOutQuad(t);
			break;
		case VSE_IN_OUT_QUAD:
			t = EaseInOutQuad(t);
			break;
		case VSE_IN_CUBIC:
			t = EaseInCubic(t);
			break;
		case VSE_OUT_CUBIC:
			t = EaseOutCubic(t);
			break;
		case VSE_IN_OUT_CUBIC:
			t = EaseInOutCubic(t);
			break;
		case VSE_IN_QUART:
			t = EaseInQuart(t);
			break;
		case VSE_OUT_QUART:
			t = EaseOutQuart(t);
			break;
		case VSE_IN_OUT_QUART:
			t = EaseInOutQuart(t);
			break;
		case VSE_IN_QUINT:
			t = EaseInQuint(t);
			break;
		case VSE_OUT_QUINT:
			t = EaseOutQuint(t);
			break;
		case VSE_IN_OUT_QUINT:
			t = EaseInOutQuint(t);
			break;
		case VSE_IN_EXPO:
			t = EaseInExpo(t);
			break;
		case VSE_OUT_EXPO:
			t = EaseOutExpo(t);
			break;
		case VSE_IN_OUT_EXPO:
			t = EaseInOutExpo(t);
			break;
		case VSE_IN_CIRC:
			t = EaseInCirc(t);
			break;
		case VSE_OUT_CIRC:
			t = EaseOutCirc(t);
			break;
		case VSE_IN_OUT_CIRC:
			t = EaseInOutCirc(t);
			break;
		case VSE_IN_BACK:
			t = EaseInBack(t);
			break;
		case VSE_OUT_BACK:
			t = EaseOutBack(t);
			break;
		case VSE_IN_OUT_BACK:
			t = EaseInOutBack(t);
			break;
		case VSE_IN_ELASTIC:
			t = EaseInElastic(t);
			break;
		case VSE_OUT_ELASTIC:
			t = EaseOutElastic(t);
			break;
		case VSE_IN_OUT_ELASTIC:
			t = EaseInOutElastic(t);
			break;
		case VSE_IN_BOUNCE:
			t = EaseInBounce(t);
			break;
		case VSE_OUT_BOUNCE:
			t = EaseOutBounce(t);
			break;
		case VSE_IN_OUT_BOUNCE:
			t = EaseInOutBounce(t);
			break;
	}


	if (tween->elapsedTime < tween->duration)
	{
		switch (tween->tweenType)
		{
			case VSE_VECTOR2_FLOAT_TWEEN:
				*(VSE_Vector2Float *) tween->target = VSE_LerpVector2Float(
					tween->tweenData.vector2FloatTween.fromValue, tween->tweenData.vector2FloatTween.endValue,
					t);
				break;

			case VSE_FLOAT_TWEEN:
				*(float *) tween->target = VSE_LerpFloat(
					tween->tweenData.floatTween.fromValue, tween->tweenData.floatTween.endValue, t);
				break;

			case VSE_VECTOR2_INT_TWEEN:
				*(VSE_Vector2Int *) tween->target = VSE_LerpVector2Int(tween->tweenData.vector2IntTween.fromValue,
				                                               tween->tweenData.vector2IntTween.endValue, t);
				break;

			case VSE_INT_TWEEN:
				*(int *) tween->target = VSE_LerpInt(tween->tweenData.intTween.fromValue,
				                                 tween->tweenData.intTween.endValue, t);
		}
	}
	else
	{
		FinishTween(engine, tween);
	}
}


void VSE_DestroyTween(VSE_Tween *tween)
{
	if (tween == NULL)
	{
		return;
	}

	free(tween);
}


void VSE_DestroySequence(VSE_TweenSequence *tweenSequence)
{
	if (tweenSequence == NULL)
	{
		return;
	}

	for (int i = 0; i < tweenSequence->tweeners->size; i++)
	{
		VSE_Tween *tween = tweenSequence->tweeners->elements[i];
		VSE_DestroyTween(tween);
	}

	free(tweenSequence);
}


static float EaseLinear(float t)
{
	return t;
}


static float EaseInSine(float t)
{
	return 1 - cos(t * VSE_PI / 2);
}


static float EaseOutSine(float t)
{
	return sin(t * VSE_PI / 2);
}


static float EaseInOutSine(float t)
{
	return -(cos(VSE_PI * t) - 1) / 2;
}


static float EaseInQuad(float t)
{
	return t * t;
}


static float EaseOutQuad(float t)
{
	return t * (2 - t);
}


static float EaseInOutQuad(float t)
{
	return t < 0.5 ? 2 * t * t : -1 + (4 - 2 * t) * t;
}


static float EaseInCubic(float t)
{
	return t * t * t;
}


static float EaseOutCubic(float t)
{
	return 1 - pow(1 - t, 3);
}


static float EaseInOutCubic(float t)
{
	return t < 0.5 ? 4 * t * t * t : 1 - pow(-2 * t + 2, 3) / 2;
}


static float EaseInQuart(float t)
{
	return t * t * t * t;
}


static float EaseOutQuart(float t)
{
	return 1 - pow(1 - t, 4);
}


static float EaseInOutQuart(float t)
{
	return t < 0.5 ? 8 * t * t * t * t : 1 - pow(-2 * t + 2, 4) / 2;
}


static float EaseInQuint(float t)
{
	return t * t * t * t * t;
}


static float EaseOutQuint(float t)
{
	return 1 - pow(1 - t, 5);
}


static float EaseInOutQuint(float t)
{
	return t < 0.5 ? 16 * t * t * t * t * t : 1 - pow(-2 * t + 2, 5) / 2;
}


static float EaseInExpo(float t)
{
	return t == 0 ? 0 : pow(2, 10 * (t - 1));
}


static float EaseOutExpo(float t)
{
	return t == 1 ? 1 : 1 - pow(2, -10 * t);
}


static float EaseInOutExpo(float t)
{
	return t == 0 ? 0 : t == 1 ? 1 : t < 0.5 ? pow(2, 20 * t - 10) / 2 : (2 - pow(2, -20 * t + 10)) / 2;
}


static float EaseInCirc(float t)
{
	return 1 - sqrt(1 - t * t);
}


static float EaseOutCirc(float t)
{
	return sqrt(1 - pow(t - 1, 2));
}


static float EaseInOutCirc(float t)
{
	return t < 0.5 ? (1 - sqrt(1 - 4 * t * t)) / 2 : (sqrt(1 - (-2 * t + 2) * (-2 * t + 2)) + 1) / 2;
}


static float EaseInBack(float t)
{
	return cbrt(1 - t) * 3 - 2;
}


static float EaseOutBack(float t)
{
	return 1 + cbrt(t - 1) * 3;
}


static float EaseInOutBack(float t)
{
	return t < 0.5
		       ? (pow(2 * t, 2) * ((2.5 + 1) * 2 * t - 2.5)) / 2
		       : (pow(2 * t - 2, 2) * ((2.5 + 1) * (t * 2 - 2) + 2.5) + 2) / 2;
}


static float EaseInElastic(float t)
{
	return sin(13 * VSE_PI / 2 * t) * pow(2, 10 * (t - 1));
}


static float EaseOutElastic(float t)
{
	return sin(-13 * VSE_PI / 2 * (t + 1)) * pow(2, -10 * t) + 1;
}


static float EaseInOutElastic(float t)
{
	return t < 0.5
		       ? sin(13 * VSE_PI / 2 * 2 * t) * pow(2, 10 * ((2 * t) - 1)) / 2
		       : (sin(-13 * VSE_PI / 2 * ((2 * t - 1) + 1)) * pow(2, -10 * (2 * t - 1)) + 2) / 2;
}


static float EaseOutBounce(float t)
{
	const float n1 = 7.5625f;
	const float d1 = 2.75f;

	if (t < 1 / d1)
	{
		return n1 * t * t;
	}
	if (t < 2 / d1)
	{
		t -= 1.5f / d1;
		return n1 * t * t + 0.75f;
	}
	if (t < 2.5 / d1)
	{
		t -= 2.25f / d1;
		return n1 * t * t + 0.9375f;
	}
	t -= 2.625f / d1;
	return n1 * t * t + 0.984375f;
}


static float EaseInBounce(float t)
{
	return 1 - EaseOutBounce(1 - t);
}


static float EaseInOutBounce(float t)
{
	return t < 0.5 ? (1 - EaseOutBounce(1 - 2 * t)) / 2 : (1 + EaseOutBounce(2 * t - 1)) / 2;
}
