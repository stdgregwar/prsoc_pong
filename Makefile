LDLIBS = -lSDL -lSDL_image
CXXFLAGS = -std=c++11 -g -Wall -pedantic

all: pong

pong:
	$(CXX) $(CXXFLAGS) -o $@ main.cpp $^ $(LDLIBS)

run: all
	./pong

runfb:
	./pong fb

clean:
	rm -f *.o
	rm pong
