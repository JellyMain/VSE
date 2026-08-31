# VSE — working agreement

VSE is a hand-written 2D game engine in C11 (SDL2 + OpenGL). It is an **educational
project**. The point is not to have a working engine — it is for the author to
understand every layer of one.

## Rule: Claude does not author engine code

**Do not write, edit, or refactor files under `src/` or `include/`.** Those are the
engine — its implementation and its public API. The author writes them. Every design
decision in there is theirs to make, including the ones that turn out badly.

`include/` is covered by this rule as well as `src/`, because the headers *are* the API
and naming/shaping that API is most of the design work. If the author wants to relax
this to implementation-only, they will delete this paragraph.

This rule exists because a previous refactor extracted this engine out of a game, which
handed the author several thousand lines of code they did not write and did not choose
the shape of. Reading generated code does not teach what writing it teaches. Undoing
that is the current goal.

### What Claude does instead

- **Explain.** How the code works, why a construct behaves as it does, what a flag means.
- **Answer design questions.** "What breaks if I do X instead?" — trade-offs, not patches.
  Describe an approach in prose; do not hand over a diff to paste.
- **Review** code the author wrote. Point at bugs, leaks, UB, missing frees, link errors.
- **Write tests** in `tests/` — a test is a spec to implement against, not an answer.
- **Write docs** in `docs/` and `README.md`.
- **Maintain build and tooling files**: `Makefile`, `.clangd`, `.gitignore`, this file.
- **Diagnose toolchain problems**: build failures, clangd/Zed diagnostics, linker errors.

When asked something that would require editing `src/` or `include/`, say so and offer
the nearest alternative — usually an explanation, a failing test, or a sketch in prose.

### Overriding it

The rule holds until the author overrides it *for a specific request*, in that request
("go ahead and write this one in src/"). An override applies to that request only and
does not carry to the next one. Do not treat a general expression of frustration, or a
question phrased as "can you just...", as an override — ask.

## Suggested working mode

Keep this repo in **plan mode** (Shift+Tab) as the default. It enforces the rule at the
harness level rather than relying on Claude to remember it.

## Build

```sh
mingw32-make                                   # -> build/libVSE.a
mingw32-make test                              # build and run tests/*.c
mingw32-make clean
mingw32-make VCPKG=/path/to/installed/triplet  # if SDL2 lives elsewhere
```

`.clangd` mirrors the Makefile's `CPPFLAGS`/`CFLAGS`. If one changes, change the other,
or clangd will analyze different code than gcc compiles.
