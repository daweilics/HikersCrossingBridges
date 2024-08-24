# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++14 -Wall -Wextra -Werror -O2
LDFLAGS = -pthread

# Directories
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include
YAML_SRC_DIR = yaml-cpp/src
YAML_BUILD_DIR = build/yaml-cpp
YAML_INCLUDE_DIR = yaml-cpp/include
YAML_LIB = $(YAML_BUILD_DIR)/libyaml-cpp.a
TEST_DIR = test
TEST_BUILD_DIR = $(BUILD_DIR)/test
GTEST_DIR = googletest/googletest
GTEST_BUILD_DIR = $(BUILD_DIR)/gtest

# YAML lib
YAML_SOURCES = $(wildcard $(YAML_SRC_DIR)/*.cpp)
YAML_OBJECTS = $(YAML_SOURCES:$(YAML_SRC_DIR)/%.cpp=$(YAML_BUILD_DIR)/%.o)

# Source and object files
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS = $(OBJECTS:.o=.d)

# Test source and object files
TEST_SOURCES = $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJECTS = $(TEST_SOURCES:$(TEST_DIR)/%.cpp=$(TEST_BUILD_DIR)/%.o)

# Executable names
TARGET = hiker
TEST_TARGET = run_tests

# Default target
all: $(TARGET)

# Build the main executable
$(TARGET): $(YAML_OBJECTS) $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $(YAML_OBJECTS) $(OBJECTS)

#$(YAML_LIB): $(YAML_OBJECTS)
#	ar rcs $@ $(YAML_OBJECTS)

# Build Google Test
$(GTEST_BUILD_DIR)/libgtest.a: $(GTEST_DIR)/src/gtest-all.cc
	@mkdir -p $(GTEST_BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(GTEST_DIR) -I$(GTEST_DIR)/include -I$(INCLUDE_DIR) -c $(GTEST_DIR)/src/gtest-all.cc -o $(GTEST_BUILD_DIR)/gtest-all.o
	ar rcs $@ $(GTEST_BUILD_DIR)/gtest-all.o

$(GTEST_BUILD_DIR)/gtest_main.o: $(GTEST_DIR)/src/gtest_main.cc
	@mkdir -p $(GTEST_BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(GTEST_DIR)/include  -MMD -MP -c $< -o $@

# Build test executable
OBJECTS_NO_MAIN = $(filter-out $(BUILD_DIR)/main.o, $(OBJECTS))
$(TEST_TARGET): $(TEST_OBJECTS) $(OBJECTS_NO_MAIN) $(YAML_OBJECTS) $(GTEST_BUILD_DIR)/gtest_main.o $(GTEST_BUILD_DIR)/libgtest.a
	$(CXX) $(LDFLAGS) -o $@ $(TEST_OBJECTS) $(OBJECTS_NO_MAIN) $(YAML_OBJECTS) $(GTEST_BUILD_DIR)/gtest_main.o $(GTEST_BUILD_DIR)/libgtest.a

# Compile YAML source files into object files
$(YAML_BUILD_DIR)/%.o: $(YAML_SRC_DIR)/%.cpp
	@mkdir -p $(YAML_BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(YAML_INCLUDE_DIR) -MMD -MP -c $< -o $@

# Compile source files into object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -MMD -MP -c $< -o $@

# Compile test source files into test object files
$(TEST_BUILD_DIR)/%.o: $(TEST_DIR)/%.cpp
	@mkdir -p $(TEST_BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -I$(GTEST_DIR)/include -MMD -MP -c $< -o $@

# Include dependency files
-include $(DEPS)

# Run unit tests
test: $(TEST_TARGET)
	./$(TEST_TARGET)

# Clean build artifacts
clean:
	rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/*.d $(TEST_BUILD_DIR)/*.o $(TARGET) $(TEST_TARGET)

deepclean: clean
	rm -f $(YAML_BUILD_DIR)/*.o $(YAML_BUILD_DIR)/*.d $(GTEST_BUILD_DIR)/libgtest.a

# Clean and rebuild
rebuild: clean all

.PHONY: all clean deepclean rebuild test
