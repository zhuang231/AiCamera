TARGET = test
CXX = g++
CXXFLAGS = -std=c++17 -Icore $(shell pkg-config --cflags libcamera libdrm libjpeg)
LDFLAGS = $(shell pkg-config --libs libcamera libdrm libjpeg)
LIBS =

SRC = $(wildcard core/*.cpp) test.cpp
OBJ = $(SRC:.cpp=.o)

$(TARGET): $(OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS) $(LIBS)

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

clean:
	rm -f $(OBJ) $(TARGET)