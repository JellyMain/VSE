# Input

Input is polled, not event-driven. The input system drains SDL's event queue once per frame and
latches what happened; you read that state from anywhere during the frame.

```c
VSE_Vector2Float dir = VSE_GetMoveDirection();   /* WASD, components in -1..1 */
VSE_Vector2Float mouse = VSE_GetMousePosition(); /* window-relative pixels */
bool clicked = VSE_IsLeftMouseButtonClicked();   /* true only on the frame of the press */
VSE_KeyboardKey key = VSE_GetKeyPressed();       /* SDL keycode, or SDLK_UNKNOWN */
```

## Frame-scoped state

`VSE_IsLeftMouseButtonClicked` and `VSE_GetKeyPressed` are **edge-triggered**: they report a press
that happened during this frame's event drain and reset at the start of the next one. Reading them
twice in a frame gives the same answer; missing a frame misses the press.

`VSE_GetMoveDirection` is **level-triggered** — it reads SDL's live keyboard state, so it reflects
what is held down right now.

Because input runs first among the default updatables, everything registered after it sees a
consistent snapshot.

## Movement

```c
static void UpdatePlayer(VSE_Component *self, VSE_Engine *engine, float deltaTime)
{
    VSE_Vector2Float dir = VSE_GetMoveDirection();

    if (dir.x == 0 && dir.y == 0)
    {
        return;
    }

    VSE_Entity *me = self->entity;
    me->transform.position.x += dir.x * SPEED * deltaTime;
    me->transform.position.y += dir.y * SPEED * deltaTime;
}
```

Remember the projection: **y increases downward**, so W gives `y = -1`.

Always multiply by `deltaTime`, or speed becomes frame-rate dependent.

## Only one key per frame

`VSE_GetKeyPressed` holds a single keycode — the last `SDL_KEYDOWN` of the frame. That is enough for
text entry at human typing speed, but it cannot express two keys pressed in the same frame. For
simultaneous keys, read SDL's keyboard state directly the way `VSE_GetMoveDirection` does.

## Mouse position and windows

`VSE_GetMousePosition` returns coordinates relative to the window with mouse focus, which is what UI
hit testing wants. In a multi-window game, check `engine->focusedWindow` to know which window those
coordinates belong to — the same pixel position means different things in different windows.

## Quitting

The input system handles `SDL_QUIT` itself and calls `exit(0)`. Closing any window quits the game.
To run cleanup first, you would need to handle the event before the input system does.

## Text entry

Input fields consume keystrokes through their own updatable, registered on focus and removed on
Escape. `VSE_GetKeyPressed` is what they read, so a field with focus and your own key handling will
both see the same press — check whether a field is focused before acting on a key.
