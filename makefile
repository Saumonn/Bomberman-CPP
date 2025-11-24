CXX      = g++
CXXFLAGS = -Wall -Wextra -std=c++17 $(shell pkg-config --cflags opencv4)
LDLIBS   = $(shell pkg-config --libs opencv4)

TARGET = bomberman

SRCS = main.cpp \
       bomberman.cpp \
       controller.cpp \
       view_opencv.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.cpp bomberman.h controller.h view_opencv.h levels.h
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)
