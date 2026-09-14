CXX ?= c++
AR ?= ar
PYTHON ?= python3
CPPFLAGS ?=
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic
LDFLAGS ?=
LDLIBS ?=

BUILD_DIR := build
LIB_OBJECTS := $(BUILD_DIR)/lexer.o $(BUILD_DIR)/token.o
OBJECTS := $(LIB_OBJECTS) $(BUILD_DIR)/main.o $(BUILD_DIR)/lexer_tests.o
LIBRARY := $(BUILD_DIR)/libdlang_lexer.a

.PHONY: all test clean
all: dlang

dlang: $(BUILD_DIR)/main.o $(LIBRARY)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(LIBRARY): $(LIB_OBJECTS)
	$(AR) rcs $@ $^

$(BUILD_DIR)/lexer_tests: $(BUILD_DIR)/lexer_tests.o $(LIBRARY)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) -Iinclude $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/lexer_tests.o: tests/lexer_tests.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) -Iinclude $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

test: dlang $(BUILD_DIR)/lexer_tests
	./$(BUILD_DIR)/lexer_tests
	$(PYTHON) tests/test_integration.py ./dlang

clean:
	$(RM) -r $(BUILD_DIR) dlang

-include $(OBJECTS:.o=.d)
