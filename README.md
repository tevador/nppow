## The number partitioning problem

The number partitioning problem (NPP) is one of the simplest NP-hard problems and has some interesting mathematical properties that can be useful in a secure proof of work (PoW) algorithm.

The problem itself is defined as follows: Given a list of N positive integer numbers, partition these numbers into two subsets such that the sum of numbers in one subset is as close as possible to the sum of numbers in the other subset. If the sums of the subsets are equal (or differ by 1 in the case the total sum is an odd number), we call this a *perfect partition*.

If the numbers in the list are unbounded, deciding whether a perfect partition exists or not is an NP-hard problem. Typically, integer numbers in a computer are bounded by 2<sup>B</sup>, where B is the number of bits. In this case, the probability that a perfect partition exists depends on the value of q = B/N. For uniformly distributed numbers, the expected number of perfect partitions is 2<sup>N(c-q)</sup>, where *c* is the critical number, which depends on N, but approximately c ≈ 1.0. If q is less than the critical value, perfect partitions are abundant, while for q > c, finding a perfect partition is extremely rare. [1]

One interesting property of the NPP is the randomness of the cost function. Loosely speaking, for solutions "close" to the perfect partition, the difference between the sums of the subgroups is essentially random, so no algorithm is better than a random or sequential search.

### Heuristics

This inherent "randomness" of NPP partially explains the lack of good heuristics (unlike other NP-hard problems). Although the problem has been studied for decades, the best polynomial time heuristic to date is the Karmarkar and Karp (KK) algorithm. This algorithm progressively replaces the two biggest numbers in the list by their difference (this is equivalent to placing each of the two elements in opposite subsets). At the end, just one number remains, which represents the final difference of sums of the two subsets.

The quality of the solution provided by the KK algorithm depends on the value of q. For very small values of q, KK always gives the perfect partition. For larger values of q, the result is almost always suboptimal.

An algorithm that is guaranteed to find a perfect partition (if it exists) is the *complete differencing algorithm* (CDA). It is similar to the KK algorithm, except in each step, the two largest numbers can be replaced by either their difference or by their sum (this represents putting these numbers in the same or opposite subsets, respectively). This amounts to searching a binary tree with 2<sup>N</sup> leaves. It can be made more efficient by selective pruning and applying the KK algorithm.

The essence of the KK and CDA algorithms is finding the two largest elements. Fastest way to do it is to sort the initial list and keep it sorted in each step.

## Proof of work

The best property of the NPP in relation to PoW is that a perfect partition can be very easily verified by simply calculating the sums. On the other hand, finding the perfect partition is a hard problem.

The Number partitioning proof of work (NPPoW) presented here uses a list of 128 randomly generated 42-bit numbers with the goal of finding the perfect partition. With B = 42 and N = 128, we can calculate that there are on average ≈2<sup>83</sup> perfect partitions out of the 2<sup>128</sup> possible ones. These parameters were chosen specifically so that a modified CDA algorithm can find approximately 1 perfect partition per second on a modern CPU (using 1 thread).

The list of numbers is generated at random by hashing the current block header using the SHAKE-256 hashing function, which is a NIST-approved [variant of SHA-3](https://en.wikipedia.org/wiki/SHA-3#Instances) with a variable output length. In this case, the selected output length is 5376 bits (128 × 42).

Once a perfect partition is found, it is encoded as a 128-bit number, where each bit determines whether the corresponding number should be placed in the first subset (1) or the second subset (0). To make this encoding unique, the solution is required to be an odd number, i.e. the first number in the list is always placed in the first subset.

The encoded solution is then appended to the block header and the whole header is hashed once again, this time using the SHA3-256 hash function to determine the final PoW.

Since the solution must be included in the block, its small size (16 bytes) is another advantage of NPPoW. Existing asymmetric PoW algorithms have significantly larger solutions - 1344 bytes for Equihash and 168 bytes for Cuckoo cycle.

### ASIC friendliness

As the whole algorithm has a very simple definition and a limited set of operations, it can be easily implemented in hardware. A minimal hardware miner needs only an efficient sorting algorithm, integer addition and subtraction and some simple logic to encode the solution. Hashing can be done entirely in software with low impact on performance.

It can be argued that NPPoW can result in a more egalitarian ASIC development than e.g. pure SHA-256 or SHA-3 PoW. This is because the majority of the performance is determined by the sorting algorithm. Hardware sorting engines have been extensively studied and the techniques are well established, unlike the relatively complex hashing algorithms, where the R&D cost to compete with existing products can be substantial.

## Concept code

Compile the code and run as:
```
nppow noncesCount [maxLeaves(65)] [fullProbe(1)] [startingNonce]
```
* **noncesCount** - The number of nonces to be tested. It is a required parameter (50000 is a good number when using the default parameters).
- **maxLeaves** - The number of leaves of the binary tree to be probed for each nonce. When set to 1, mining is reduced to the KK algorithm (i.e. only the subtraction branches are taken). Default is 65.
* **fullProbe** - If set to 0, maximum of one solution per nonce will be found. Default value is 1, i.e. the probing continues until *maxLeaves* are visited.
* **startingNonce** - the starting nonce. Default is a pseudorandom value.

### References
[1] Stephan Mertens, The Easiest Hard Problem: Number Partitioning, October 2003, [available online](https://arxiv.org/abs/cond-mat/0310317)

