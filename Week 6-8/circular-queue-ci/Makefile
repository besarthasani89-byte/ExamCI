CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -Iinclude
BUILD_DIR := build
TEST_SRC := test/test_circular_queue.cpp
TEST_BIN := $(BUILD_DIR)/queue_tests

GTEST_DIR := /usr/src/googletest/googletest
GTEST_SRC :=
LDLIBS := -pthread

ifneq ($(wildcard $(GTEST_DIR)/src/gtest-all.cc),)
GTEST_SRC += $(GTEST_DIR)/src/gtest-all.cc
GTEST_SRC += $(GTEST_DIR)/src/gtest_main.cc
CXXFLAGS += -I$(GTEST_DIR) -I$(GTEST_DIR)/include
else
LDLIBS += -lgtest -lgtest_main
endif

all: $(TEST_BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TEST_BIN): $(TEST_SRC) include/circular_queue.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(TEST_SRC) $(GTEST_SRC) $(LDLIBS) -o $(TEST_BIN)

test: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all test clean
