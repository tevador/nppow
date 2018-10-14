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
#include <exception>
#include <algorithm>

void sortLastElement(std::vector<NppNode*>& list) {
	//single iteration of insertion sort - O(N)
	auto last = list.back();
	size_t i = list.size() - 1;
	for (; i > 0; --i) {
		if (list[i - 1]->getValue() > last->getValue()) {
			list[i] = list[i - 1];
		}
		else {
			break;
		}
	}
	list[i] = last;
}

uint64_t NppSolver::getDifference(bitstack_t treeNode, uint128_t & solution) {
	tree.clear();
	nodes.erase(nodes.begin() + N, nodes.end());
	for (auto i = nodes.begin(); i != nodes.end(); ++i) {
		tree.push_back(&(*i));
	}
	bool isSorted = true;

	while (treeNode != 0) { //pre KK
		if (!isSorted) {
			sortLastElement(tree);
			isSorted = true;
		}
		auto a = tree[tree.size() - 1];
		auto b = tree[tree.size() - 2];
		tree.pop_back();

		auto add = (treeNode & 1UL) != 0;
		treeNode >>= 1; //pop

		if (add) {
			nodes.push_back(NppNode(a, b, TreeOperation::Addition));
			tree[tree.size() - 1] = &nodes.back();
		}
		else
		{
			nodes.push_back(NppNode(a, b, TreeOperation::Subtraction));
			tree[tree.size() - 1] = &nodes.back();
			isSorted = false;
		}
	}
	return getDifferenceKK(solution); //KK
}

uint64_t NppSolver::getDifferenceKK(uint128_t & solution) {
	while (tree.size() > 1) {
		sortLastElement(tree);
		auto a = tree[tree.size() - 1];
		auto b = tree[tree.size() - 2];
		tree.pop_back();
		nodes.push_back(NppNode(a, b, TreeOperation::Subtraction));
		tree[tree.size() - 1] = &nodes.back();
	}
	uint64_t diff = tree[0]->getValue();
	solution.fromTree(tree[0]);
	return diff;
}

bool NppSolver::verifySolution(uint128_t& solution) {
	if ((solution.lo & 1) == 0) {
		return false;
	}
	uint64_t diff = 0;
	for (int i = 0; i < 64; ++i) {
		if ((solution.lo & (1ULL << i)) != 0) {
			diff += numbers[i];
		}
		else {
			diff -= numbers[i];
		}
		if ((solution.hi & (1ULL << i)) != 0) {
			diff += numbers[i + 64];
		}
		else {
			diff -= numbers[i + 64];
		}
	}
	return (diff + 1) <= 2;
}

int NppSolver::solve(byte * input, size_t inputSize, uint128_t * solutions, size_t maxSolutions, size_t maxNodes, bool fullProbe) {
	if (inputSize < N * B / 8) {
		throw std::exception("Invalid input size");
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

	bitstack_t treeNode = 0;
	int solutionsCount = 0;

	while (treeNode < maxNodes && solutionsCount  < maxSolutions) {
		uint128_t solution;
		uint64_t diff = getDifference(treeNode, solution);

		if (diff == 0 || diff == 1)	{
			if ((solution.lo & 1) == 0)	{
				solution.lo = ~solution.lo;
				solution.hi = ~solution.hi;
			}
			*solutions++ = solution;
			++solutionsCount;
			if (!fullProbe)
				break;
		}
		treeNode++;
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
