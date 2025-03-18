# Compiler
CXX := g++

# Directories
INCLUDE_DIR := include
SOURCE_DIR := source
DEPENDENCIES_DIR := dependencies
BUILD_DIR := build

# Flags
CXXFLAGS := -Wall -Wextra -I$(INCLUDE_DIR) -I$(DEPENDENCIES_DIR)

# Source and Object Files
SOURCES := $(wildcard $(SOURCE_DIR)/*.cpp)
OBJECTS := $(patsubst $(SOURCE_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))

# Executable Name
TARGET := my_program

# Default Target
all: $(TARGET)

# Link Object Files to Create Executable
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET)

# Compile Source Files into Object Files
$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create Build Directory if It Doesn't Exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Clean Build Files
clean:
	rm -rf $(BUILD_DIR) $(TARGET)
