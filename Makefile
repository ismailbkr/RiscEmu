CC ?= cc
CPPFLAGS := -Iinclude
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror -O2
DEPFLAGS := -MMD -MP

BUILD_DIR := build
APP := $(BUILD_DIR)/rv32emu
TEST_APP := $(BUILD_DIR)/test_runner

COMMON_SOURCES := src/rv32_types.c src/memory.c src/decoder.c
APP_SOURCES := src/main.c $(COMMON_SOURCES)
TEST_SOURCES := tests/test.c tests/test_runner.c tests/test_memory.c \
	tests/test_decoder.c $(COMMON_SOURCES)
APP_OBJECTS := $(APP_SOURCES:%.c=$(BUILD_DIR)/%.o)
TEST_OBJECTS := $(TEST_SOURCES:%.c=$(BUILD_DIR)/%.o)
DEPENDENCIES := $(APP_OBJECTS:.o=.d) $(TEST_OBJECTS:.o=.d)

.PHONY: all test clean

all: $(APP)

$(APP): $(APP_OBJECTS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $^ -o $@

$(TEST_APP): $(TEST_OBJECTS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

test: $(TEST_APP)
	./$(TEST_APP)

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPENDENCIES)

