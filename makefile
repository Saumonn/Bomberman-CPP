CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
LDFLAGS = -lncurses

TARGET = bomberman
SRCS = bomberman.cpp view.cpp controller.cpp main.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp bomberman.h view.h levels.h controller.h
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)




#CXX      = g++
#CXXFLAGS = -Wall -Wextra -std=c++17 $(shell pkg-config --cflags opencv4)
#LDLIBS   = $(shell pkg-config --libs opencv4)

#TARGET = bomberman

#SRCS = main.cpp \
#       bomberman.cpp \
#       controller.cpp \
#       view_opencv.cpp

#OBJS = $(SRCS:.cpp=.o)

#all: $(TARGET)

#$(TARGET): $(OBJS)
#	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

#%.o: %.cpp bomberman.h controller.h view_opencv.h levels.h
#	$(CXX) $(CXXFLAGS) -c $<

#clean:
#	rm -f $(OBJS) $(TARGET)
