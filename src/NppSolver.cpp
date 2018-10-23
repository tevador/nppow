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

#include "NppSolver.h"
#include <stdexcept>
#include <algorithm>

void sortLastElement(std::vector<NppNode*>& list) {
	//single iteration of insertion sort - O(N)
	auto it = list.end() - 1;
	auto last = it[0];
	for (; it > list.begin(); --it) {
		if (it[-1]->getValue() > last->getValue()) {
			it[0] = it[-1];
		}
		else break;
	}
	it[0] = last;
}

void NppSolver::resetWorkingSet() {
	workingSet.clear();
	nodes.erase(nodes.begin() + N, nodes.end());
	for (auto i = nodes.begin(); i != nodes.end(); ++i) {
		workingSet.push_back(&(*i));
	}
	isSorted = true;
}

uint64_t NppSolver::getDifference(bitstack_t treePath) {
	while (treePath != 0) { //pre KK
		if (!isSorted) {
			sortLastElement(workingSet);
			isSorted = true;
		}
		auto a = workingSet.end()[-1];
		auto b = workingSet.end()[-2];
		workingSet.pop_back();
		auto add = (treePath & 1UL) != 0;
		treePath >>= 1; //pop
		nodes.push_back(NppNode(a, b, (TreeOperation)add));
		workingSet.end()[-1] = &nodes.back();
		isSorted = add;
	}
	while (workingSet.size() > 1) { //KK
		sortLastElement(workingSet);
		auto a = workingSet.end()[-1];
		auto b = workingSet.end()[-2];
		workingSet.pop_back();
		nodes.push_back(NppNode(a, b, TreeOperation::Subtraction));
		workingSet.end()[-1] = &nodes.back();
	}
	uint64_t diff = workingSet[0]->getValue();
	return diff;
}

bool NppSolver::verifySolution(uint128_t& solution) {
	if ((solution.lo & 1) == 0)
		return false;
	uint64_t diff = 0;
	for (int i = 0; i < 64; ++i) {
		if ((solution.lo & (1ULL << i)) != 0)
			diff += numbers[i];
		else
			diff -= numbers[i];
		if ((solution.hi & (1ULL << i)) != 0)
			diff += numbers[i + 64];
		else
			diff -= numbers[i + 64];
	}
	return (diff + 1) <= 2; //-1, 0, 1
}

//next number with the same number of set bits
//returns ~0 (all bits set) at the end of the sequence
uint64_t nextBitCombination(uint64_t x) {
	if (x == 0)
		return ~0;
	uint64_t lo = x & (0 - x);
	uint64_t lz = (x + lo) & ~x;
	x |= lz;
	x &= ~(lz - 1);
	x |= (lz / lo / 2) - 1;
	return x;
}

int NppSolver::solve(byte * input, size_t inputSize, uint128_t * solutions, size_t maxSolutions, size_t maxLeaves, bool fullProbe) {
	if (inputSize < N * B / 8) {
		throw std::runtime_error("Invalid input size");
	}

	nodes.clear();

	constexpr uint64_t mask = (1ULL << B) - 1;

	//unpack N B-bit numbers into N 64-bit numbers
	for (int i = 0; i < N; ++i) {
		uint32_t index = i * B / 8;
		uint64_t value = *((uint64_t*)(input + index));
		if ((B % 8) != 0) {
			int shift = (i * B) % 8;
			value >>= shift;
		}
		value &= mask;
		nodes.push_back(NppNode(value, i));
		numbers[i] = value;
	}

	std::sort(nodes.begin(), nodes.end(), [](const NppNode& a, const NppNode& b) {
		return a.getValue() < b.getValue();
	});

	bitstack_t treePath = 0;
	int solutionsCount = 0;
	int additionCount = 0;
	size_t leavesCount = 0;

	while (leavesCount < maxLeaves && solutionsCount < maxSolutions) {
		resetWorkingSet();
		uint64_t diff = getDifference(treePath);
		if (diff == 0 || diff == 1)	{
			uint128_t solution;
			solution.fromTree(workingSet[0]);
			if ((solution.lo & 1) == 0)	{
				solution.lo = ~solution.lo;
				solution.hi = ~solution.hi;
			}
			*solutions++ = solution;
			++solutionsCount;
			if (!fullProbe)
				break;
		}
		leavesCount++;
		treePath = nextBitCombination(treePath);
		if (treePath == ~0) {
			additionCount++;
			treePath = (1ULL << additionCount) - 1;
		}
	}

	return solutionsCount;
}

void uint128_t::addNode(NppNode* n, bool first) {
	if (n->isLeaf()) {
		if (first) {
			if (n->getIndex() < 64)	{
				lo |= UINT64_C(1) << n->getIndex();
			}
			else {
				hi |= UINT64_C(1) << (n->getIndex() - 64);
			}
		}
	}
	else
	{
		addNode(n->getLeft(), first);
		if (n->getOperation() == TreeOperation::Addition) {
			addNode(n->getRight(), first);
		}
		else {
			addNode(n->getRight(), !first);
		}
	}
}

void uint128_t::fromTree(NppNode * root) {
	hi = lo = UINT64_C(0);
	addNode(root, true);
}
