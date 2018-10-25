## The number partitioning problem

The number partitioning problem (NPP) is one of the simplest NP-complete problems and has been extensively studied in literature due to its importance in multiprocessor scheduling.

The problem is defined as follows: Given a list of N positive integer numbers, partition these numbers into two subsets such that the sum of numbers in one subset is as close as possible to the sum of numbers in the other subset. If the sums of the subsets are equal (or differ by 1 in the case the total sum is an odd number), we call this a *perfect partition*.

If the numbers in the list are unbounded, deciding whether a perfect partition exists or not is an NP-hard problem. Typically, integer numbers in a computer are bounded by 2<sup>B</sup>, where *B* is the number of bits. In this case, the probability that a perfect partition exists depends on the value of *q* = B/N. For uniformly distributed numbers, the expected number of perfect partitions is 2<sup>N(c-q)</sup>, where *c* = 1 - O(N<sup>-1</sup> log N) is the critical number. If *q* is less than the critical value, perfect partitions are abundant - this is called the "easy phase". When *q* reaches the critical value, a phase transition occurs into the "hard phase" (q > c), where the existence of a perfect partition becomes unlikely. [1]

### Heuristics
What is interesting about NPP is the lack of good heuristics (unlike other NP-hard problems). Although the problem has been studied for decades, the best polynomial time heuristics to date is the Karmarkar and Karp (KK) algorithm presented in 1982. This algorithm progressively replaces the two biggest numbers in the list by their difference (this is equivalent to placing each of the two elements in opposite subsets). At the end, just one number remains, which represents the final difference of sums of the two subsets. [2]

The quality of the solution provided by the KK algorithm depends on the value of q. For very small values of q, KK always gives the perfect partition. For larger values of q, the result is almost always suboptimal.

An algorithm that is eventually guaranteed to find a perfect partition (if it exists) is the *complete differencing algorithm* (CDA). It is similar to the KK algorithm, except in each step, the two largest numbers can be replaced by either their sum or by their difference (this represents putting these numbers in the same or opposite subsets, respectively). This amounts to searching a binary tree with 2<sup>N-1</sup> leaves. Pedroso (2008) presented an efficient method to search the tree using a variant of beam search, where the branches that have the fewest addition nodes are probed first. [3] This can yield a solution much faster than a brute-force search, especially in the easy phase.

The essence of all existing heuristics is finding the two largest elements. Fastest way to do it is to sort the initial list and keep it sorted in each step.

### Quantum algorithms
The [Grover's algorithm](https://en.wikipedia.org/wiki/Grover%27s_algorithm) can be used on a quantum computer to decrease the complexity of a brute-force search to 2<sup>N/2</sup> instead of 2<sup>N</sup>. This is applicable mostly to the hard phase of NPP, where no better method than exhaustive search exists.

De Raedt et al. (2001) presented a quantum algorithm to determine the exact number of solutions of a particular NPP problem in polynomial time [4]. This is applicable mostly to the decision variant of NPP (finding whether a solution exists), but can be also used to construct a valid solution by repeating the algorithm and progressively fixing elements to a particular subset. The algorithm has a practical problem in that is requires up to 2<sup>N</sup> spin measurements to distinguish cases with 0 and 1 solutions with sufficient precision. This will most likely be a limiting factor when trying to find a solution using this algorithm both in the hard and easy phase.

## Proof of work

The Number partitioning proof of work (NPPoW) presented here uses a list of 128 randomly generated 42-bit numbers with the goal of finding the perfect partition. Values B = 42 and N = 128 correspond to the "easy" phase and we can calculate that there are on average ≈2<sup>83</sup> perfect partitions out of the 2<sup>128</sup> possible ones. These parameters were chosen specifically so that the proposed mining algorithm can find approximately 1 perfect partition per second per CPU core.

The list of numbers is generated at random by hashing the current block header using the SHAKE-256 hashing function, which is a NIST-approved [variant of SHA-3](https://en.wikipedia.org/wiki/SHA-3#Instances) with a variable output length. In this case, the selected output length is 5376 bits (128 × 42).

### Mining
Mining is realized by searching the CDA binary tree using a variant of the beam search method described in Ref. [3]. During the search, the branches that have the least number of additions are probed first, so the search starts with the KK algorithm which has no additions. The mining algorithm has a low memory requirement of a few kiB.

On average, about 150000 leaves of the binary tree must be probed to find a perfect partition. A miner can choose how many leaves they will probe for a given tree before trying another set of numbers (by modifying the nonce). Theoretically, a single nonce can provide up to ≈2<sup>83</sup> solutions. The mining performance doesn't differ much whether a single nonce is being mined or nonces are changed frequently.

### Solution encoding
Once a perfect partition is found, it is encoded as a 128-bit number, where each bit determines whether the corresponding number should be placed in the first subset (1) or the second subset (0). To make this encoding unique, the solution is required to be an odd number, i.e. the first number in the list is always placed in the first subset.

The encoded solution is then appended to the block header and the whole header is hashed once again, this time using the SHA3-256 hash function to determine the final PoW. The block is solved if the difficulty of the PoW hash meets the required value for the current block.

Since the solution must be included in the block, its small size (16 bytes) is an advantage of NPPoW. Existing asymmetric PoW algorithms have significantly larger solutions - 1344 bytes for Equihash (N = 200, K = 9) and 168 bytes for Cuckoo cycle (N = 32).

### Verification
A solution can be instantly verified by simply calculating the sums of the two subsets. The verification routine is just 16 lines of C++ code:

```C++
bool NppSolver::verifySolution(NppSolution& solution) {
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
```
(Note: This doesn't include the initial SHAKE-256 hash that is used to generate the numbers from the block header and the final SHA3 hash that is used to determine the difficulty.)

### Parameters

In the presented case, the difficulty of the NPP is fixed with the given parameters (N = 128, B = 42) and the difficulty of the PoW is adjusted by requiring a particular SHA3 hash of the resulting block.

Firstly, the values of the parameters N and B can be modified to increase or lower the difficulty of finding a solution. Generally, increasing N makes the search space larger, but perfect partitions become easier to find (approximately linearly). Increasing B makes finding perfect partitions exponentially harder. The parameters can be selected based on the particular security and performance requirements. For efficient CPU mining performance, parameters should be selected so that B + log<sub>2</sub>N <= 64.

Another option is to adjust the values N and B dynamically for difficulty targeting and remove the final SHA3 hash. However, this would require finding a good function of N and B for smooth difficulty adjustments and would make pooled mining more complicated, if not impossible.

### ASIC friendliness

As the whole algorithm has a very simple definition and a limited set of operations, it can be easily implemented in hardware. A minimal hardware miner needs to implement only a single iteration of insertion sort using N-1 comparators, integer addition and subtraction and some simple logic to encode the solution. Hashing can be done entirely in software with low impact on performance.

It can be argued that NPPoW can result in a more egalitarian ASIC development than e.g. pure SHA-256 or SHA-3 PoW. This is due to the simplicity of NPP compared to the complex hashing algorithms, where the R&D cost to compete with existing products can be substantial.

## Concept code

Compile the project using `make` (Linux) or Visual Studio 2017 (Windows). Run as:
```
nppow noncesCount [maxLeaves(65)] [fullProbe(1)] [startingNonce]
```
* **noncesCount** - The number of nonces to be tested. It is a required parameter (50000 is a good number when using the default parameters).
- **maxLeaves** - The number of leaves of the binary tree to be probed for each nonce. When set to 1, mining is reduced to the KK algorithm (i.e. only the subtraction branches are taken). Default is 65.
* **fullProbe** - If set to 0, maximum of one solution per nonce will be found. Default value is 1, i.e. the probing continues until *maxLeaves* are visited.
* **startingNonce** - the starting nonce. Default is a pseudorandom value.

## References
[1] S. Mertens, The Easiest Hard Problem: Number Partitioning, October 2003, [available online](https://arxiv.org/abs/cond-mat/0310317)

[2] N. Karmarkar and R. Karp, The differencing method of set partitioning. Technical Report
UCB/CSD 82/113, 1982

[3] J. P. Pedroso and M. Kubo, Heuristics and exact methods for number partitioning, 2008, [available online](http://www.dcc.fc.up.pt/~jpp/publications/PDF/numpartition-DCC.pdf)

[4] H. De Raedt et al., Number Partitioning on a Quantum Computer, April 2001, [available online](https://arxiv.org/abs/quant-ph/0010018)
