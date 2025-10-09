# Makefile
PROJECT = MemForge
VERSION = 1.0.0

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
TEST_DIR = tests
BENCHMARK_DIR = benchmarks
EXAMPLE_DIR = examples

# Compiler settings
# MemForge Build Configuration - OPTIMAL SETTINGS
CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -D_DEFAULT_SOURCE
CFLAGS += -fPIC -fstack-protector-strong -D_FORTIFY_SOURCE=2

# Debug flags
DEBUG_CFLAGS = -g -O0 -fsanitize=address -fsanitize=undefined
DEBUG_CFLAGS += -Wno-unused-parameter -Wno-unused-variable

# Release flags  
RELEASE_CFLAGS = -O2 -DNDEBUG -flto

# Shared flags
SHARED_FLAGS = -shared

# Thread flags
THREAD_FLAGS = -pthread

# Platform-specific
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    CFLAGS += -D_POSIX_C_SOURCE=200809L
    LDFLAGS += -pthread -lm
endif

# Source files
CORE_SOURCES = $(wildcard $(SRC_DIR)/core/*.c)
STRATEGY_SOURCES = $(wildcard $(SRC_DIR)/strategies/*.c)
PLATFORM_SOURCES = $(wildcard $(SRC_DIR)/platform/*.c)

SOURCES = $(CORE_SOURCES) $(STRATEGY_SOURCES) $(PLATFORM_SOURCES)

# Targets
.PHONY: all debug release static shared test examples benchmarks clean install

all: debug

debug: CFLAGS += $(DEBUG_CFLAGS)
debug: shared static

release: CFLAGS += $(RELEASE_CFLAGS)
release: shared static

shared: $(BUILD_DIR)/debug/$(PROJECT).so

static: $(BUILD_DIR)/debug/$(PROJECT).a

$(BUILD_DIR)/debug/$(PROJECT).so: $(SOURCES)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(SHARED_FLAGS) $(THREAD_FLAGS) -o $@ $(SOURCES)

$(BUILD_DIR)/debug/$(PROJECT).a: $(SOURCES:.c=.o)
	@mkdir -p $(@D)
	ar rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

test: debug
	$(MAKE) -C $(TEST_DIR)

examples: debug
	$(MAKE) -C $(EXAMPLE_DIR)

benchmarks: release
	$(MAKE) -C $(BENCHMARK_DIR)

clean:
	rm -rf $(BUILD_DIR)
	$(MAKE) -C $(TEST_DIR) clean
	$(MAKE) -C $(EXAMPLE_DIR) clean
	$(MAKE) -C $(BENCHMARK_DIR) clean

install: release
	cp $(BUILD_DIR)/release/$(PROJECT).so /usr/local/lib/
	cp $(BUILD_DIR)/release/$(PROJECT).a /usr/local/lib/
	cp -r $(INCLUDE_DIR)/memalloc /usr/local/include/

.PHONY: all debug release shared static test examples benchmarks clean install