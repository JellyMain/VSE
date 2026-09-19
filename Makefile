#===============================================================================
# BASE
#
# Reusable part, copy this section into a new project. It gives you:
#   - gcc on Windows (cmd.exe), debug and release builds: make BUILD=debug
#   - compiling any .c file into build/<type>/<same path>.o
#   - header dependency tracking, so changing a .h recompiles what includes it
#   - make compile_commands (compile_commands.json for clangd) and make clean
#
# Then add your targets below it. Minimal example for one .exe:
#
#   INCLUDES += -Isrc
#   APP_SOURCES := $(wildcard src/*.c)
#   ALL_SOURCES += $(APP_SOURCES)
#
#   all: $(BUILD_DIR)/app.exe
#
#   $(BUILD_DIR)/app.exe: $(call objects_of,$(APP_SOURCES))
#   	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@
#
# Source paths must be relative and inside the project folder (no ../).
#===============================================================================

SHELL = cmd.exe
CC = gcc
CFLAGS = -std=c23 -Wall -Wextra -MMD -MP
LDFLAGS =
LDLIBS =
INCLUDES =

#build type: make BUILD=debug or make BUILD=release (default)
BUILD ?= release
ifeq ($(BUILD),debug)
  CFLAGS += -g -O0
else ifeq ($(BUILD),release)
  CFLAGS += -O2 -DNDEBUG
else
  $(error Unknown BUILD '$(BUILD)', use debug or release)
endif

BUILD_ROOT:= build
BUILD_DIR:= $(BUILD_ROOT)/$(BUILD)

#sources compiled by this Makefile, used for compile_commands.json. Add yours with ALL_SOURCES += ...
ALL_SOURCES:=

#object files for a list of sources: src/main.c -> build/release/src/main.o
objects_of = $(patsubst %.c,$(BUILD_DIR)/%.o,$(1))

#converts / to \ for cmd.exe commands
win_path = $(subst /,\,$(1))

#all files matching a pattern in a folder and its subfolders: $(call rwildcard,build,*.d)
rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$(d),$(2)) $(filter $(subst *,%,$(2)),$(d)))

.DEFAULT_GOAL:= all


$(BUILD_DIR)/%.o: %.c
	@if not exist $(call win_path,$(@D)) mkdir $(call win_path,$(@D))
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@


clean:
	@if exist $(BUILD_ROOT) rmdir /S /Q $(BUILD_ROOT)


#one entry per source in ALL_SOURCES, with the same flags the build uses
CDB_ENTRY = {"directory": "$(CURDIR)", "file": "$(1)", "output": "$(call objects_of,$(1))", "command": "$(CC) $(INCLUDES) $(CFLAGS) -c $(1) -o $(call objects_of,$(1))"}

compile_commands:
	$(file >compile_commands.json,[$(call CDB_ENTRY,$(firstword $(ALL_SOURCES)))$(foreach src,$(wordlist 2,$(words $(ALL_SOURCES)),$(ALL_SOURCES)),,$(call CDB_ENTRY,$(src)))])
	@echo wrote compile_commands.json


.PHONY: clean compile_commands

#header dependencies written by -MMD
-include $(call rwildcard,$(BUILD_DIR),*.d)



#===============================================================================
# VSE PROJECT -- Very Simple Engine
#
# make                    the library, build/<type>/libVSE.a
# make example            build examples/sandbox without launching it
# make behaviours         build only examples/sandbox/behaviours/*.c into .dlls
# make run                build the library + examples/sandbox, then launch it
# make run EXAMPLE=pong   same, for examples/pong
# make run_tests          build and run tests/*.c (no tests checked in yet)
#
# make VCPKG=C:/path/to/installed/triplet   if SDL2 lives elsewhere
#===============================================================================

VCPKG ?= C:/vcpkg/installed/x64-mingw-dynamic

#include/ is the public API, src/ holds internal headers, glad and SDL are private deps
INCLUDES += -Iinclude -Isrc -Ivendor/glad/include \
            -I$(VCPKG)/include -I$(VCPKG)/include/SDL2

CFLAGS  += -Wno-unused-parameter -DSDL_MAIN_HANDLED
LDFLAGS += -L$(VCPKG)/lib
LDLIBS  += -lSDL2 -lSDL2_image -lSDL2_ttf -lopengl32

LIBRARY:= $(BUILD_DIR)/libVSE.a

all: $(LIBRARY)

.PHONY: all example behaviours run run_tests

#SDL's DLLs next to an .exe: Windows searches the executable's own folder first, so the
#binary runs from any working directory. $(1) is the folder to copy them into.
copy_dlls = if exist $(call win_path,$(VCPKG)/bin/*.dll) copy /Y $(call win_path,$(VCPKG)/bin/*.dll) $(call win_path,$(1)) >nul


#--- library: libVSE.a with the engine and the bundled glad loader -----------

LIBRARY_SOURCES:= $(wildcard src/*/*.c) vendor/glad/src/glad.c
ALL_SOURCES += $(LIBRARY_SOURCES)

#deleted first: `ar r` *updates* an archive, so a member whose .c you later delete would
#otherwise sit in libVSE.a forever
$(LIBRARY): $(call objects_of,$(LIBRARY_SOURCES))
	@if exist $(call win_path,$@) del /Q $(call win_path,$@)
	ar rcs $@ $^


#--- examples: one folder under examples/, every .c in it links into one .exe -

EXAMPLE ?= sandbox
EXAMPLE_DIR:= examples/$(EXAMPLE)
EXAMPLE_SOURCES:= $(wildcard $(EXAMPLE_DIR)/*.c)
EXAMPLE_EXE:= $(BUILD_DIR)/examples/$(EXAMPLE).exe

#every example is indexed by clangd, not just the one being built
ALL_SOURCES += $(wildcard examples/*/*.c)

#behaviours: every .c in the example's behaviours/ folder becomes its own .dll, loaded at
#runtime by name -- VSE_AddBehaviour(engine, entity, "health") opens <behavioursRoot>/health.dll.
#They are deliberately NOT linked into the .exe: EXAMPLE_SOURCES globs $(EXAMPLE_DIR)/*.c
#only, so this subfolder never reaches the example's link line.
BEHAVIOUR_SOURCES:= $(wildcard $(EXAMPLE_DIR)/behaviours/*.c)
BEHAVIOUR_DLLS:= $(patsubst %.c,$(BUILD_DIR)/%.dll,$(BEHAVIOUR_SOURCES))
ALL_SOURCES += $(wildcard examples/*/behaviours/*.c)

#.c -> .o -> .dll is a chain of two implicit rules, which make treats the .o as a temporary
#of and deletes -- and then rebuilds the .dll from scratch on the next run, every run.
#.SECONDARY keeps the objects, so the .d files below stay meaningful and builds stay incremental.
.SECONDARY: $(call objects_of,$(BEHAVIOUR_SOURCES))

#without this an empty example folder reaches the linker and dies on a missing WinMain
ifneq ($(filter run example,$(MAKECMDGOALS)),)
ifeq ($(EXAMPLE_SOURCES),)
$(error no .c files in $(EXAMPLE_DIR)/, write $(EXAMPLE_DIR)/main.c first)
endif
endif

#the .exe depends on $(LIBRARY), which depends on the engine objects, so `run` after an
#edit anywhere in src/ or include/ recompiles exactly what changed and relaunches --
#the library is never installed or copied, it is linked in place
$(EXAMPLE_EXE): $(call objects_of,$(EXAMPLE_SOURCES)) $(LIBRARY)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@
	@$(call copy_dlls,$(@D))

#-shared is the whole difference between a .dll and an .exe -- nothing in the .c says which
#it is. The .o comes from the BASE rule, so -MMD tracking applies here too: editing
#include/VSE/component.h rebuilds every behaviour .dll that includes it.
#
#--no-undefined: a behaviour that calls an engine function directly cannot work -- the engine
#lives in the .exe, not in the .dll. Without this flag the link succeeds and LoadLibraryA
#fails at runtime with a bare error 126; with it, the build fails and names the symbol.
$(BUILD_DIR)/%.dll: $(BUILD_DIR)/%.o
	$(CC) -shared $< -Wl,--no-undefined -o $@

behaviours: $(BEHAVIOUR_DLLS)

example: $(EXAMPLE_EXE) $(BEHAVIOUR_DLLS)

#runs from the repo root, so assetRoot/shaderRoot in the example's VSE_Config are
#written relative to the root ("shaders/", "examples/sandbox/assets/")
run: $(EXAMPLE_EXE) $(BEHAVIOUR_DLLS)
	$(call win_path,$(EXAMPLE_EXE))


#--- tests: one .exe per tests/*.c, each linked against the library -----------

TESTS_SOURCES:= $(wildcard tests/*.c)
TESTS_EXES:= $(TESTS_SOURCES:tests/%.c=$(BUILD_DIR)/tests/%.exe)
ALL_SOURCES += $(TESTS_SOURCES)

$(BUILD_DIR)/tests/%.exe: $(BUILD_DIR)/tests/%.o $(LIBRARY)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@
	@$(call copy_dlls,$(@D))

#stops at the first failing test
run_tests: $(TESTS_EXES)
	$(foreach t,$(TESTS_EXES),$(call win_path,$(t)) &&) echo all tests passed
