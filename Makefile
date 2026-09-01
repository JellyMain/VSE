# VSE -- Very Simple Engine
#
#   mingw32-make                  build build/libVSE.a
#   mingw32-make run              build the library + examples/sandbox, then launch it
#   mingw32-make run EXAMPLE=pong same, for examples/pong
#   mingw32-make example          build the example without launching it
#   mingw32-make test             build and run tests/*.c (no tests checked in yet)
#   mingw32-make clean            remove build/
#   mingw32-make compile_commands.json   regenerate the clangd compilation database
#
# Override VCPKG if SDL2 lives elsewhere:
#   mingw32-make VCPKG=/path/to/installed/triplet

# GNU Make on Windows only invokes $(SHELL) for a recipe line that contains a shell
# metacharacter (&, |, <, >, ;, ...); a "plain" line is spawned directly via
# CreateProcess and fails unless its first word happens to be a real .exe already on
# PATH. Pinning SHELL here means that whenever a line *does* get shell-routed, it
# finds a shell regardless of the invoking terminal's PATH (mingw32-make otherwise
# falls back to cmd.exe when sh.exe isn't on PATH). Must be the 8.3 short path --
# spaces in SHELL break how Make builds the CreateProcess call.
SHELL  := C:/PROGRA~1/Git/bin/sh.exe

CC     := gcc
AR     := ar
BUILD  ?= build
VCPKG  ?= C:/vcpkg/installed/x64-mingw-dynamic

LIB := $(BUILD)/libVSE.a

SRCS := $(wildcard src/*/*.c) vendor/glad/src/glad.c
OBJS := $(SRCS:%.c=$(BUILD)/obj/%.o)

# include/ is public; src/ is for internal headers; glad and SDL are private deps
CPPFLAGS := -Iinclude -Isrc -Ivendor/glad/include \
            -I$(VCPKG)/include -I$(VCPKG)/include/SDL2 \
            -DSDL_MAIN_HANDLED

CFLAGS   := -std=c23 -g -Wall -Wextra -Wno-unused-parameter

# Shared by every rule that links an executable (tests and examples both).
LDFLAGS  := -L$(VCPKG)/lib
LDLIBS   := -lSDL2 -lSDL2_image -lSDL2_ttf -lopengl32

TEST_SRCS := $(wildcard tests/*.c)
TESTS     := $(TEST_SRCS:tests/%.c=$(BUILD)/tests/%.exe)

# An example is one directory under examples/; every .c directly inside it becomes one
# executable. The binary depends on $(LIB), which depends on $(OBJS), so `run` after an
# edit anywhere in src/ or include/ recompiles exactly what changed and relaunches --
# the library is never installed or copied, it is linked in place.
EXAMPLE      ?= sandbox
EXAMPLE_DIR  := examples/$(EXAMPLE)
EXAMPLE_SRCS := $(wildcard $(EXAMPLE_DIR)/*.c)
EXAMPLE_OBJS := $(EXAMPLE_SRCS:%.c=$(BUILD)/obj/%.o)
EXAMPLE_BIN  := $(BUILD)/examples/$(EXAMPLE).exe

# Without this, an empty example dir reaches the linker and dies on a missing WinMain.
ifneq ($(filter run example,$(MAKECMDGOALS)),)
ifeq ($(EXAMPLE_SRCS),)
$(error no .c files in $(EXAMPLE_DIR)/ -- write $(EXAMPLE_DIR)/main.c first)
endif
endif

.PHONY: all test clean compile_commands.json example run
all: $(LIB)

# rm first: `ar r` *updates* an archive, so a member whose .c you later delete would
# otherwise sit in libVSE.a forever.
$(LIB): $(OBJS)
	@mkdir -p $(dir $@);
	@rm -f $@;
	$(AR) rcs $@ $^
	@echo "built $@";

# -MMD -MP emits a .d per object listing the headers it pulled in, so touching a
# header rebuilds exactly the objects that depend on it. Matches example sources too:
# examples/sandbox/main.c -> build/obj/examples/sandbox/main.o
$(BUILD)/obj/%.o: %.c
	@mkdir -p $(dir $@);
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD)/tests/%.exe: tests/%.c $(LIB)
	@mkdir -p $(dir $@);
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIB) $(LDFLAGS) $(LDLIBS) -o $@
	@cp -n $(VCPKG)/bin/*.dll $(dir $@) 2>/dev/null || true

test: $(TESTS)
	@for t in $(TESTS); do echo "== $$t"; ./$$t || exit 1; done
	@echo "all tests passed";

# The DLL copy puts SDL next to the .exe; Windows searches the executable's own
# directory first, so the example runs from any working directory.
$(EXAMPLE_BIN): $(EXAMPLE_OBJS) $(LIB)
	@mkdir -p $(dir $@);
	$(CC) $(EXAMPLE_OBJS) $(LIB) $(LDFLAGS) $(LDLIBS) -o $@
	@cp -n $(VCPKG)/bin/*.dll $(dir $@) 2>/dev/null || true
	@echo "built $@";

example: $(EXAMPLE_BIN)

# Runs from the repo root, so assetRoot/shaderRoot in the example's VSE_Config are
# written relative to the root ("shaders/", "examples/sandbox/assets/").
run: $(EXAMPLE_BIN)
	./$(EXAMPLE_BIN)

clean:
	rm -rf $(BUILD);

# Regenerates the clangd compilation database from data the build already has, so
# clangd can background-index every .c file without requiring it to be opened first.
# Re-run this after adding/removing a .c file or changing CPPFLAGS/CFLAGS.
CDB_SRCS := $(SRCS) $(EXAMPLE_SRCS)
compile_commands.json:
	@printf '[\n' > $@
	@n=$(words $(CDB_SRCS)); i=0; \
	for f in $(CDB_SRCS); do \
		i=$$((i+1)); \
		obj="$(BUILD)/obj/$${f%.c}.o"; \
		printf '  {\n    "directory": "$(CURDIR)",\n    "file": "%s",\n    "command": "$(CC) $(CPPFLAGS) $(CFLAGS) -c %s -o %s"\n  }' "$$f" "$$f" "$$obj" >> $@; \
		if [ $$i -lt $$n ]; then printf ',\n' >> $@; else printf '\n' >> $@; fi; \
	done
	@printf ']\n' >> $@
	@echo "generated $@ ($(words $(CDB_SRCS)) entries)";

-include $(OBJS:.o=.d) $(EXAMPLE_OBJS:.o=.d)
