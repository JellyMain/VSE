# VSE -- Very Simple Engine
#
#   mingw32-make          build build/libVSE.a
#   mingw32-make test     build and run the test binaries
#   mingw32-make clean    remove build/
#
# Override VCPKG if SDL2 lives elsewhere:
#   mingw32-make VCPKG=/path/to/installed/triplet

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

CFLAGS   := -std=c11 -g -Wall -Wextra -Wno-unused-parameter

TEST_SRCS := $(wildcard tests/*.c)
TESTS     := $(TEST_SRCS:tests/%.c=$(BUILD)/tests/%.exe)

.PHONY: all test clean
all: $(LIB)

$(LIB): $(OBJS)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $^
	@echo "built $@"

# -MMD -MP emits a .d per object listing the headers it pulled in, so touching a
# header rebuilds exactly the objects that depend on it.
$(BUILD)/obj/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD)/tests/%.exe: tests/%.c $(LIB)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIB) -L$(VCPKG)/lib -lSDL2 -lSDL2_image -lSDL2_ttf -lopengl32 -o $@
	@cp -n $(VCPKG)/bin/*.dll $(dir $@) 2>/dev/null || true

test: $(TESTS)
	@for t in $(TESTS); do echo "== $$t"; ./$$t || exit 1; done
	@echo "all tests passed"

clean:
	rm -rf $(BUILD)

-include $(OBJS:.o=.d)
