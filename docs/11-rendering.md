# Rendering

## Per frame, per window

Bind the window's framebuffer → clear → game entities → UI entities → gizmos (if `debugMode`) →
post-processing chain → swap.

All windows share one GL context, created against a hidden 1x1 window at startup.

## Textures and materials

```c
VSE_Texture  *tex = VSE_LoadTexture("Player.png");            /* relative to assetRoot */
VSE_Material *mat = VSE_CreateMaterial(NULL, NULL);           /* NULL = default shaders */
VSE_Material *custom = VSE_CreateMaterial("MyEffect.frag", "MyEffect.vert");  /* rel. to shaderRoot */
```

A solid-colour texture without an image file:

```c
VSE_Texture *rect = VSE_CreateRect((VSE_Vector2Float){100, 50}, (SDL_Color){255, 0, 0, 255});
```

Materials are **not shared** — each entity that needs one gets its own, because uniforms live on the
material. Two entities sharing a material share its uniform values.

## Uniforms

Every material gets a `projection` uniform automatically, updated per window per frame. Add your
own by pointer — the material stores the pointer, so the value tracks whatever it points at:

```c
VSE_AddUniformToMaterial(mat, "time", VSE_UNIFORM_FLOAT, &engine->time);
```

That means the pointed-at value must outlive the material. `&engine->time` is safe; a local is not.

For a one-off write, `VSE_SetShaderUniform(mat->shaderProgram, "time", VSE_UNIFORM_FLOAT, &t)`.

Supported types are `VSE_UNIFORM_FLOAT` and `VSE_UNIFORM_MAT4F`.

## Post-processing

Effects are materials over a fullscreen quad, applied in registration order:

```c
VSE_Material *vignette = VSE_CreateMaterial("PostProcessing/Vignette.frag",
                                            "PostProcessing/post.vert");
VSE_AddPostProcessingEffect(engine, "vignette", vignette);

VSE_Material *wobble = VSE_CreateMaterial("PostProcessing/wobble.frag",
                                          "PostProcessing/post.vert");
VSE_AddUniformToMaterial(wobble, "time", VSE_UNIFORM_FLOAT, &engine->time);
VSE_AddPostProcessingEffect(engine, "wobble", wobble);
```

VSE ships `Vignette.frag`, `wobble.frag`, `grayScale.frag` and the `post.vert` they share, but the
engine does not name them — you build the material and register it, so your own effects work the
same way.

## Text

Two kinds, for different jobs.

**Static text** rasterises a string to a texture once, via `VSE_CreateStaticText`. Sharp at any
size, but changing it means recreating the entity. Use for labels.

**Dynamic text** draws from a pre-baked glyph atlas each frame, so the string can change freely. Use
for scores and counters. `VSE_Init` builds the atlas at `engine->textAtlas` from
`config.debugFontPath`.

```c
VSE_RenderDynamicText(engine->textAtlas, "Score: 100",
                      (VSE_Vector2Float){10, 10}, VSE_VECTOR2_FLOAT_ONE,
                      VSE_HORIZONTAL_NOT_CENTERED, engine->renderer, projectionMatrix);
```

The atlas covers ASCII digits, letters and common punctuation; anything else advances one pixel.

## Debug rendering

With `config.debugMode = true` the engine draws an FPS counter and a red gizmo rect around each UI
entity, labelled with its position and size. Gizmos are created automatically for buttons and input
fields.

## Coordinates

The projection puts the origin at the **top left** with **y increasing downward**, matching SDL. A
sprite's position is its centre.
