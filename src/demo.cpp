/*
(c) 2018 tevador <tevador@gmail.com>

This file is part of nppow.

nppow is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

nppow is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with nppow.  If not, see<http://www.gnu.org/licenses/>.
*/

#include "keccak-tiny/keccak-tiny.h"
#include "NppSolver.h"
#include <iostream>
#include <chrono>
#include <sstream>

                                                                                                         /*NONCE*/
byte blockTemplateHex[] = "0707f7a4f0d605b303260816ba3f10902e1a145ac5fad3aa3af6ea44c11869dc4f853f002b2eea0000000077b206"
                                                                                       /*-- NPP SOLUTION GOES HERE --*/
                          "a02ca5b1d4ce6bbfdf0acac38bded34d2dcdeef95cd20cefc12f61d5610900000000000000000000000000000000";

byte parseNibble(byte hex) {
	hex &= ~0x20;
	if (hex & 0x40) {
		hex -= 'A' - 10;
	}
	else {
		hex &= 0xf;
	}
	return hex;
}

void hex2bin(byte *in, int length, byte *out) {
	for (int i = 0; i < length; i += 2) {
		byte nibble1 = parseNibble(*in++);
		byte nibble2 = parseNibble(*in++);
		*out++ = nibble1 << 4 | nibble2;
	}
}

constexpr char hexmap[] = "0123456789abcdef";

void outputHex(std::ostream& os, const byte* data, int length) {
	for (int i = 0; i < length; ++i) {
		os << hexmap[(data[i] & 0xF0) >> 4];
		os << hexmap[data[i] & 0x0F];
	}
}

template<typename T>
bool tryParse(char* buffer, T& out) {
	std::istringstream ss(buffer);
	if (!(ss >> out)) {
		std::cout << "Invalid value '" << buffer << "'" << std::endl;
		return false;
	}
	return true;
}

int main(int argc, char** argv) {
	constexpr int nonceOffset = 39;
	constexpr int blockTemplateBaseSize = 76;

	uint32_t noncesCount;
	size_t maxLeaves = 16;
	uint32_t startingNonce = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	bool fullProbe = true;

	if (argc < 2) {
		std::cout << "Usage: " << std::endl << argv[0] << " noncesCount [maxLeaves(16)] [fullProbe(1)] [startingNonce]" << std::endl;
		return 0;
	}

	if (!tryParse(argv[1], noncesCount)) {
		return 1;
	}

	if (argc >= 3) {
		if (!tryParse(argv[2], maxLeaves)) {
			return 1;
		}
	}

	if (argc >= 4) {
		if (!tryParse(argv[3], fullProbe)) {
			return 1;
		}
	}

	if (argc >= 5) {
		if (!tryParse(argv[4], startingNonce)) {
			return 1;
		}
	}

	std::cout << "Running nonces " << startingNonce << " - " << startingNonce + noncesCount - 1 <<  ", leaves per nonce: " << maxLeaves << (fullProbe ? "" : " (stopping at first solution)") << std::endl;

	byte blockTemplate[sizeof(blockTemplateHex) / 2];

	hex2bin(blockTemplateHex, sizeof(blockTemplateHex), blockTemplate);

	uint32_t* noncePtr = (uint32_t*)(blockTemplate + nonceOffset);
	uint128_t* nppSolutionPtr = (uint128_t*)(blockTemplate + blockTemplateBaseSize);

	uint128_t solutions[10];
	byte numbersBuffer[B * N / 8];
	byte powHash[256 / 8];
	int totalSolutions = 0;

	NppSolver solver;

	auto hptStart = std::chrono::high_resolution_clock::now();
	for (uint32_t nonce = startingNonce; nonce < startingNonce + noncesCount; ++nonce) {
		*noncePtr = nonce;
		shake256(numbersBuffer, sizeof(numbersBuffer), blockTemplate, blockTemplateBaseSize);
		int solutionsCount = solver.solve(numbersBuffer, sizeof(numbersBuffer), solutions, sizeof(solutions) / sizeof(uint128_t), maxLeaves, fullProbe);
		for (int sol = 0; sol < solutionsCount; ++sol) {
			auto solution = solutions[sol];
			*nppSolutionPtr = solution;
			sha3_256(powHash, sizeof(powHash), blockTemplate, sizeof(blockTemplate));
			std::cout << "Nonce: " << nonce << ", Solution = ";
			outputHex(std::cout, (byte*)&solution, sizeof(solution));
			//std::cout << ", PoW: ";
			//outputHex(std::cout, powHash, sizeof(powHash));
			std::cout << ", Valid = " << solver.verifySolution(solution) << std::endl;
		}
		totalSolutions += solutionsCount;
	}
	auto hptEnd = std::chrono::high_resolution_clock::now();
	std::cout << "Performance: " << totalSolutions / std::chrono::duration<double>(hptEnd - hptStart).count() << " solutions per second" << std::endl;
	std::cout << "Nonces per solution: " << noncesCount / (double)totalSolutions << std::endl;
	return 0;
}
