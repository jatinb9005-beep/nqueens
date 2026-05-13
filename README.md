# nqueens
Large-Scale N-Queens Solver
A C++ implementation of the classic N-Queens problem using the min-conflicts local search heuristic, designed to scale efficiently to boards of up to 1 million queens.
Built for CP 468 — Artificial Intelligence at Wilfrid Laurier University.

About the Problem
The N-Queens problem asks: can you place N queens on an N×N chessboard such that no two queens attack each other (no shared row, column, or diagonal)?
Brute-force and backtracking approaches become impractical as N grows. This solver uses local search with the min-conflicts heuristic, which can solve very large instances efficiently.

How It Works
The solver uses a multi-stage approach:

Optimized Greedy Initialization — Places each queen column-by-column in the row with the fewest existing conflicts, producing a strong starting state. For large N, this samples a subset of candidate rows rather than scanning all N.
Min-Conflicts Local Search — Repeatedly picks a conflicted queen and moves it to the row that minimizes its conflicts. Random tie-breaking is used when multiple rows are equally good.
Adaptive Sampling — At each step, the search samples a bounded number of candidate rows for very large N (e.g., 5,000 rows even when N = 1,000,000), trading some optimality for tractable runtimes.
Restart Strategy — Each problem size is given multiple attempts with fresh greedy initializations. The conflict list is also periodically rebuilt to recover from stale state and escape local minima.

Key Data Structures

row_count[N] — number of queens in each row
diag1[2N] — queens on each "/" diagonal
diag2[2N] — queens on each "" diagonal

These allow O(1) conflict checks and updates per queen movement, which is critical for performance at large N.

