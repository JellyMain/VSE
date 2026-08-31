# Windows

Every VSE window is a real OS window with its own OpenGL framebuffer. Games can spread across
several of them, and the windows' positions on the desktop are part of the game.

```c
VSE_Window *w = VSE_CreateGameWindowWithRenderer(engine,
    (VSE_Vector2Int){300, 100},     /* desktop position */
    (VSE_Vector2Int){400, 400},     /* size */
    VSE_WINDOW_WORLD_SPACE,
    VSE_SCALE_WITH_RESIZE,
    "Window A");
```

## Render space

| value | meaning |
|---|---|
| `VSE_WINDOW_WORLD_SPACE` | entities sit in one world shared by every window; each window is a viewport onto it, offset by its desktop position. Moving the window scrolls the view. |
| `VSE_WINDOW_SCREEN_SPACE` | positions are relative to this window's own top-left. Use for menus and HUDs. |

## Resize behaviour

| value | meaning |
|---|---|
| `VSE_FIXED_SIZE` | not resizable |
| `VSE_RESIZABLE` | resizable; contents keep their size |
| `VSE_SCALE_WITH_RESIZE` | resizable; entities inside scale with the window |

`VSE_SCALE_WITH_RESIZE` multiplies the transform scale of every entity currently inside the window
by the frame's percentage size change. It affects entities *fully within* the window's bounds — see
`entitiesInWindowList`, recomputed each frame.

## Useful fields

```c
w->position           /* desktop position, refreshed each frame */
w->size               /* current size */
w->windowCenterPoint  /* centre in window-local coordinates */
w->viewportOffset     /* world -> screen offset for WORLD_SPACE */
```

`windowCenterPoint` is the usual anchor for screen-space UI:

```c
VSE_CreateStaticText(w, engine, "Paused", (SDL_Color){255,255,255},
                     w->windowCenterPoint, (VSE_Vector2Float){0.3f, 0.3f}, NULL);
```

Note it is computed at creation from the initial size and is not updated on resize.

## Centring a window on screen

`VSE_CreateGameWindowWithRenderer` treats the position as the window's top-left, so centring means
creating it and then offsetting by half its size:

```c
VSE_Window *menu = VSE_CreateGameWindowWithRenderer(engine, VSE_GetDisplayCenterPosition(),
                                                   (VSE_Vector2Int){500, 600},
                                                   VSE_WINDOW_SCREEN_SPACE, VSE_FIXED_SIZE, "Menu");
SDL_SetWindowPosition(menu->sdlWindow,
                      menu->position.x - menu->size.x / 2,
                      menu->position.y - menu->size.y / 2);
```

## Draw lists

Each window keeps its own draw lists. `VSE_EntityCreateSprite` adds to every window that exists at
that moment, so **create windows before entities**. To place an entity in one window only:

```c
VSE_Entity *e = VSE_EntityCreate(engine, position, scale);
VSE_AddComponent(e, VSE_COMPONENT_SPRITE_RENDERER, &sprite);
VSE_AddGameEntityToDrawList(specificWindow, e);
```

## Focus

`engine->focusedWindow` is the window the mouse last entered; UI interaction is dispatched only for
that window. `VSE_SetFocusWindow` sets it and raises the window.

## Dragging and resizing

While the OS drags or resizes a window it blocks the main loop. `VSE_EnableLiveResizeRendering`
installs an SDL event filter that keeps rendering during the drag. Disable it before tearing down a
scene — it renders from inside an event callback and must not run while windows are being destroyed.
