CXXFLAGS=-Wall -std=c++0x -march=native -O3
EXECUTABLE=nppow
LDFLAGS=
OBJ=keccak-tiny.o NppSolver.o demo.o uint256_t.o uint128_t.o

$(EXECUTABLE): $(OBJ)
	$(CXX) -o $@ $(OBJ) $(LDFLAGS)

keccak-tiny.o: keccak-tiny/keccak-tiny.c
	$(CXX) -o $@ -c $(CXXFLAGS) keccak-tiny/keccak-tiny.c

NppSolver.o: NppSolver.cpp
demo.o: demo.cpp
uint256_t.o: uint256_t/uint256_t.cpp
	$(CXX) -o $@ -c $(CXXFLAGS) uint256_t/uint256_t.cpp
uint128_t.o: uint256_t/uint128_t.cpp
	$(CXX) -o $@ -c $(CXXFLAGS) uint256_t/uint128_t.cpp

clean:
	rm -f $(EXECUTABLE) $(OBJ)
