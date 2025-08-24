# Project settings
CXX = g++ -m32
CXXFLAGS = -Wall -Wextra -std=c++17 -I. -O2 -static
LDFLAGS = -lgdi32 -lcomdlg32 -lole32 -luuid -luser32 -mwindows -static -lcomctl32

# Output target
TARGET = LegacyResource

# Source and object files
SRC = main.cpp edit.cpp startup.cpp window.cpp dialogedit.cpp crtrep.cpp
OBJ = $(SRC:.cpp=.o)

# Resource object file
RES_OBJ = Resource.res.o

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJ) $(RES_OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

# Compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile resource file to object file
%.res.o: %.res
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f $(OBJ) $(RES_OBJ) $(TARGET)

# Optional rebuild target
rebuild: clean all

.PHONY: all clean rebuild
