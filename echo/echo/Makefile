# Define the compiler
CXX = g++

# Define the compiler flags
CXXFLAGS = -Wall -Wextra -std=c++11

# Define the source file and the output executable name
SRC = echo.cpp
OUT = echo_server

# Default target
all: $(OUT)

# Rule to create the executable
$(OUT): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(OUT) $(SRC)

# Rule to clean the build directory
clean:
	rm -f $(OUT)

# Rule to run the server
run: $(OUT)
	./$(OUT)

.PHONY: all clean run
