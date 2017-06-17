LDLIBS = -lSDL -lSDL_image
CXXFLAGS += -DDENANO -std=c++11 -g -Wall -pedantic

CPP = $(wildcard *.cpp)
CS = $(shell find . -name '*.c')
OBJS = $(notdir $(CPP:.cpp=.o))
OBJS += $(CS:.c=.o)


all: pong

pong: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

run: all
	./pong

runfb:
	./pong fb

clean:
	find . -name '*.o' -exec rm -rf {} \;
	rm pong
