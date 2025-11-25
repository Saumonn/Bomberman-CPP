CXX      = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -pthread $(shell pkg-config --cflags opencv4)
LDLIBS   = -pthread $(shell pkg-config --libs opencv4)

TARGET = bomberman
SERVER = bomberman_server

SRCS = main.cpp \
       bomberman.cpp \
       controller.cpp \
       view_opencv.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(TARGET) $(SERVER)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

$(SERVER): server.o
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp bomberman.h controller.h view_opencv.h levels.h
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)
