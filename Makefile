LDLIBS = -lSDL -lSDL_image
CXXFLAGS = -std=c++11 -g -Wall -pedantic

CPP = $(wildcard *.cpp)
CS = $(wildcard *.c)
OBJS = $(notdir $(CPP:.cpp=.o))
OBJS += $(notdir $(CS:.c=.o))

all: pong

pong: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

run: all
	./pong

runfb:
	./pong fb

clean:
	rm -f *.o
	rm pong
