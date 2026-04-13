CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I./src
LDFLAGS = -lraylib -lopengl32 -lgdi32 -lwinmm

SRC = src/main.cpp \
      src/Board.cpp \
      src/Game.cpp \
      src/Tetromino.cpp

OBJ = $(SRC:.cpp=.o)

TARGET = game

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET).exe $(LDFLAGS)

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	del /Q src\*.o $(TARGET).exe 2>nul || exit 0

run: $(TARGET)
	.\$(TARGET).exe
