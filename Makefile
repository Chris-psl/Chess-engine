# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude

# Libraries
LIBS = -lsfml-graphics -lsfml-window -lsfml-system

# Target output
TARGET = main
TEST = engine_test

# Source and build directories
SRC_DIR = src
OBJ_DIR = obj
INCLUDE = include

# Find all .cpp files in src
SRC = $(wildcard $(SRC_DIR)/*.cpp)

# Generate matching .o files in obj/
OBJ = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC))

# Default build rule
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(TARGET) $(LIBS)

# Rule to compile each .cpp into .o inside obj/
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Ensure obj/ directory exists
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Test target
$(TEST): $(OBJ_DIR)/engine.o $(filter-out $(OBJ_DIR)/main.o, $(OBJ))
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

# Clean up
clean:
	rm -rf $(OBJ_DIR) $(TARGET)
