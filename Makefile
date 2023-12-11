CXX=g++
CXXFLAGS=-g -std=c++17 -Wall -pedantic

EXECUTABLE=service

$(EXECUTABLE): Main.o BackgroundProcess.o LocalSocket.o
	@$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	@rm -f *.o
	@rm -f $(EXECUTABLE)
