# README

My solutions to the 2025 edition of [Advent of Code](https://adventofcode.com/).

This was a learning exercise in C, a language with which I felt quite rusty. As a result, my approaches and implementation don't strive to be optimal.

The first few days were trivial, and only include the solution to part 2 (the harder problem).

## Problems

### Day 1
We have a wheel with a head pointing at 0, and a list of rotations left or right.

- (**Part 1**) How many items in the sequence of movements bring the head back to the start position?
- (**Part 2**) How many times does a movement bring the head over the start position?

Solved using modular arithmetic.

### Day 2
Each line supplies a range. Find all the values in the range that:

- (**Part 1**) Are a string of digit repeated twice.
- (**Part 2**) Are a string of digit repeated any K number of times.

Solved by iterating over left subsets of digits and comparing expectation (value by repeating subsets to fill actual length) with actual value.

### Day 3
Each line has a sequence of digits.

- (**Part 1**) Choose the two digits that if read left to right give the highest value
- (**Part 2**) Choose the 12 digits that if read left to right give the highest value.

Generalised solution to K digits: pick the largest value within the largest possible range available to each digit. A better solution (not implemented) might use a stack, appending values and popping all values that are smaller than the current value to append.

### Day 4
2D grid of cells, some of which can be removed based on some rules.

- (**Part 1**) First iteration of removal.
- (**Part 2**) All subsequent iterations.

I solved the problem twice: naively by iterating over each position in the grid at each iteration; more cleverly by using a stack of positions to revisit in subsequent iterations.

### Day 5
Given a list of ranges and a list of integers:

- (**Part 1**) Count how many integers are within any of the ranges. Solved trivially.
- (**Part 2**) Ignoring the second part of the input, count how many unique values are within any of the ranges. Solved by sorting and merging ranges.

### Day 6
Solve simple arithmetic problems. Difficulty is parsing input which is expressed in a bizarre way in **part 1**, and an even more bizarre one in **part 2**.

### Day 7
2D grid, with binary indicator on first row propagating down, until it's deviated or split by obstacle.

- (**Part 1**) Obstacles split value above: count the number of times the input is split. Solved trivially by updating the state line by line.
- (**Part 2**) Obstacles deviate value above: count the number of total paths the value could take. Solved with dynamic programming by iterating over every row and counting in each cell the sum of states up to that point.

### Day 8
Given coordinates in 3D space:

- (**Part 1**) Connect closest 1000 pairs of coordinates, then identify 3 largest connected components. Solved with disjoint sets.
- (**Part 2**) Connect pairs of coordinates, lowest to greatest distance, until all coordinates are connected; identify last 2 coordinates to be connected. Re-used disjoint sets solution to keep track of how many disconnected components remain.

### Day 9
Considering 2D grid, and positions which are "red:

- (**Part 1**) Find the largest rectangle you can use by choosing two red corners. Trivial.
- (**Part 2**) Consider each red position, in the order it's given, as the corner of a polygon. Find the largest rectangle you can use by choosing two red corners, such that the rectangle is inside the polygon. Solved by first finding all the cells inside the polygon with a sweep line algorithm; then using dynamic programming to count number of cells inside the polygon on any rectangular area, and using it to validate solutions.

### Day 10
- (**Part 1**) For each line starts from array of 0s, and wants to turn it into supplied target binary array, by actioning buttons each of which flips one or more bit. Solved through brute force.
- (**Part 2**) Can be expressed as a system of linear equations with integer solutions (one for each input line). Solved with Gaussian elimination, and then searching the space of free variables for minimal solution, pruning search space with a few tricks.

### Day 11
We are given a directed graph. We are asked to count the number of paths:

- (**Part 1**) From a start to an end node. Solved by topologially sorting nodes, and then dynamic programming by stepping through nodes in topological order.
- (**Part 2**) From a start to an end node through other two nodes. Reused algorithm for part 1, by counting paths between each pair of nodes to reach.

### Day 12
Only one part. Given a set of different shapes on a 2D grid; also given, on each line, a 2D board, and a number of desired shapes of each kind to place. The problem was to count how many of those lines could be satisfied, if one found just the right way to fit the desired shapes by translating them, flipping them and rotating them.

Solved using a recursive search and pruning the search space by ignoring different ways of producing identical results on the grid.
