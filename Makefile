CXX = g++-13
CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -Iinc

SRC = $(wildcard src/*.cpp)
OBJ = $(patsubst src/%.cpp,build/%.o,$(SRC))

LIBS = -lavformat -lavcodec -lavutil -lswresample -lSDL2 -lpthread

TARGET = player

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LIBS)

build/%.o: src/%.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf build $(TARGET)