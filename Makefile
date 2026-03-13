CXX := c++
CXXFLAGS := -std=c++20 -O3 -Wall -Wextra -Wpedantic -pthread -Iinclude

BUILD_DIR := build
TEST_BIN := $(BUILD_DIR)/test_spsc
BENCH_BIN := $(BUILD_DIR)/bench_spsc

.PHONY: all test bench clean

all: test bench

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(TEST_BIN): tests/test_spsc.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BENCH_BIN): benchmarks/bench_spsc.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

test: $(TEST_BIN)

bench: $(BENCH_BIN)

clean:
	rm -rf $(BUILD_DIR)
