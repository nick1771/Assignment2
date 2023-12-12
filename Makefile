CXX=g++
CXXFLAGS=-g -std=c++17 -Wall -pedantic

EXECUTABLE=service

OBJECTS=Main.o BackgroundService.o LocalSocket.o BroadcastSocket.o InterfaceAddress.o
TEST_OBJECTS=MainTest.o BackgroundService.o LocalSocket.o BroadcastSocket.o InterfaceAddress.o

$(EXECUTABLE): $(OBJECTS)
	@$(CXX) $(CXXFLAGS) -o $@ $^

build-test: $(TEST_OBJECTS)
	@$(CXX) $(CXXFLAGS) -o tests $^

test: build-test
	@./tests

clean:
	@rm -f *.o
	@rm -f $(EXECUTABLE)
	@rm -f tests
