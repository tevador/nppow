CXXFLAGS=-Wall -std=c++0x -march=native -O3
EXECUTABLE=npp
LDFLAGS=
OBJ=keccak-tiny.o NppSolver.o demo.o

$(EXECUTABLE): $(OBJ)
	$(CXX) -o $@ $(OBJ) $(LDFLAGS)

keccak-tiny.o: keccak-tiny/keccak-tiny.c
	$(CXX) -o $@ -c $(CXXFLAGS) keccak-tiny/keccak-tiny.c

NppSolver.o: NppSolver.cpp
demo.o: demo.cpp

clean:
	rm -f $(EXECUTABLE) $(OBJ)
