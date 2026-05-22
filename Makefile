# Makefile for LibStreamX (workshop codebase).
#
# Targets:
#   make             build static lib + test runner + cli
#   make test        run unit tests
#   make asan        build with AddressSanitizer + UBSan
#   make clean       remove build artifacts

ifeq ($(OS),Windows_NT)
    MKDIR = if not exist $(subst /,\,$(1)) mkdir $(subst /,\,$(1))
    RM = if exist $(subst /,\,$(1)) rmdir /s /q $(subst /,\,$(1))
    EXE = .exe
    FIX_PATH = $(subst /,\,$(1))
else
    MKDIR = mkdir -p $(1)
    RM = rm -rf $(1)
    EXE =
    FIX_PATH = $(1)
endif

CC      = gcc
# -D_POSIX_C_SOURCE=200809L exposes POSIX symbols (strdup, etc.) that the
# library relies on; without it, glibc's <string.h> hides strdup under
# -std=c99 and the implicit-int fallback truncates the returned pointer.
CFLAGS  = -Wall -Wextra -O2 -std=c99 -D_POSIX_C_SOURCE=200809L -Iinclude
LDFLAGS =

# Sanitizer overlay — enabled via `make asan` / `make ubsan`.
ASAN_FLAGS  = -fsanitize=address,undefined -fno-omit-frame-pointer -g -O1
UBSAN_FLAGS = -fsanitize=undefined -fno-omit-frame-pointer -g -O1

SRC_DIR   = src
INC_DIR   = include
TEST_DIR  = tests
BUILD_DIR = build
OBJ_DIR   = $(BUILD_DIR)/obj

# Library sources (everything except main.c, which is only for the CLI).
LIB_SRC = \
    $(SRC_DIR)/ringbuf.c \
    $(SRC_DIR)/packet.c \
    $(SRC_DIR)/parser.c \
    $(SRC_DIR)/logger.c \
    $(SRC_DIR)/arena.c \
    $(SRC_DIR)/config.c \
    $(SRC_DIR)/cli.c

LIB_OBJ = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(LIB_SRC))

LIB_A       = $(BUILD_DIR)/libstreamx.a
TEST_RUNNER = $(BUILD_DIR)/test_runner$(EXE)
CLI_BIN     = $(BUILD_DIR)/streamx-cli$(EXE)

# Per-module test sources: tests/test_main.c plus tests/test_<module>.c
TEST_SRC = $(wildcard $(TEST_DIR)/test_*.c)

.PHONY: all clean test asan ubsan

all: $(LIB_A) $(TEST_RUNNER) $(CLI_BIN)

$(LIB_A): $(LIB_OBJ)
	@$(call MKDIR,$(BUILD_DIR))
	ar rcs $@ $(LIB_OBJ)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@$(call MKDIR,$(OBJ_DIR))
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_RUNNER): $(TEST_SRC) $(LIB_A)
	@$(call MKDIR,$(BUILD_DIR))
	$(CC) $(CFLAGS) -I$(TEST_DIR) $(TEST_SRC) $(LIB_A) -o $@ $(LDFLAGS)

$(CLI_BIN): $(SRC_DIR)/main.c $(LIB_A)
	@$(call MKDIR,$(BUILD_DIR))
	$(CC) $(CFLAGS) $(SRC_DIR)/main.c $(LIB_A) -o $@ $(LDFLAGS)

test: $(TEST_RUNNER)
	@echo Running LibStreamX test suite...
	@$(call FIX_PATH,$(TEST_RUNNER))

# Sanitizer build: nukes the build dir and rebuilds with ASan/UBSan.
asan:
	@$(call RM,$(BUILD_DIR))
	$(MAKE) all CFLAGS="$(CFLAGS) $(ASAN_FLAGS)" LDFLAGS="$(LDFLAGS) $(ASAN_FLAGS)"

ubsan:
	@$(call RM,$(BUILD_DIR))
	$(MAKE) all CFLAGS="$(CFLAGS) $(UBSAN_FLAGS)" LDFLAGS="$(LDFLAGS) $(UBSAN_FLAGS)"

clean:
	@echo Cleaning build artifacts...
	@$(call RM,$(BUILD_DIR))
