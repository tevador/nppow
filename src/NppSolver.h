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

#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

constexpr int N = 128;
constexpr int B = 42;

#define byte unsigned char
#define bitstack_t uint64_t

enum TreeOperation : bool {
	Addition = true,
	Subtraction = false,
};

class NppNode;

class NppSolution {
private:
	void addNode(NppNode* n, bool first);
public:
	void fromTree(NppNode* root);
	uint64_t lo;
	uint64_t hi;
};

class NppNode {
private:
	uint64_t value;
	NppNode* left;
	NppNode* right;
	uint32_t index;
	TreeOperation operation;
public:
	NppNode(uint64_t v, uint32_t i) : value(v), left(nullptr), right(nullptr), index(i) {
	}
	NppNode(NppNode* l, NppNode* r, TreeOperation op) : left(l), right(r), operation(op) {
		if (operation == TreeOperation::Addition) {
			value = left->value + right->value;
		}
		else {
			value = left->value - right->value;
		}
	}
	bool isLeaf() const {
		return left == nullptr;
	}
	uint64_t getValue() const {
		return value;
	}
	uint32_t getIndex() const {
		return index;
	}
	NppNode* getLeft() const {
		return left;
	}
	NppNode* getRight() const {
		return right;
	}
	TreeOperation getOperation() const {
		return operation;
	}
};

class NppSolver {
private:
	uint64_t numbers[N];
	std::vector<NppNode> nodes;
	std::vector<NppNode*> workingSet;
	bool isSorted;
	void resetWorkingSet();
	uint64_t getDifference(bitstack_t treeNode);
public:
	NppSolver() {
		nodes.reserve(2 * N);
		workingSet.reserve(N);
	}
	int solve(byte* input, size_t inputSize, NppSolution* solutions, size_t maxSolutions, size_t maxNodes, bool fullProbe);
	bool verifySolution(NppSolution& solution);
};
