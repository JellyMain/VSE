# VSE documentation

Read in this order on a first pass. Coming back after a break, `01-architecture.md` and
`99-cookbook.md` are usually enough to reload the model.

| | |
|---|---|
| [00-getting-started.md](00-getting-started.md) | A complete minimal app, and what each piece does |
| [01-architecture.md](01-architecture.md) | The frame loop, systems vs behaviours, ownership, teardown, what the engine does and does not know |
| [02-entities-components.md](02-entities-components.md) | The component model, built-ins, registering your own, why not an ECS |
| [10-windows.md](10-windows.md) | Multi-window games, render spaces, resize behaviour, draw lists |
| [11-rendering.md](11-rendering.md) | Textures, materials, uniforms, post-processing, text, coordinates |
| [12-ui.md](12-ui.md) | Buttons, text, input fields, parenting, reaching game state from a handler |
| [13-tweening.md](13-tweening.md) | Tweens, sequences, easing, the one-tween-per-target rule |
| [14-datastructures.md](14-datastructures.md) | `VSE_List`, the typed macro list, `VSE_Dictionary` |
| [15-input.md](15-input.md) | Polled input, frame-scoped state, movement |
| [99-cookbook.md](99-cookbook.md) | Working recipes lifted from a real game |

Per-function reference lives in the headers as doc comments, which clangd surfaces on hover — so in
an editor with clangd (Zed, VS Code, CLion) hovering any `VSE_` symbol shows what it does. There is
no generated HTML reference and no Doxygen dependency.

## Conventions

- Everything public is prefixed `VSE_`. Anything without the prefix is not part of the API.
- Positions are the **centre** of a thing. The origin is the top left; **y increases downward**.
- Sizes in components are untransformed; the engine multiplies by `pixelsPerUnit` and transform scale.
- `NULL` generally means "use the default" — `VSE_CreateMaterial(NULL, NULL)` is the default shader
  pair, a `NULL` button texture means a solid colour rect.
- The engine never dereferences a `userData` pointer. It hands back exactly what you gave it.
- Lists and dictionaries hold **borrowed** pointers and free neither keys nor values.
