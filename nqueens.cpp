// g++ nqueens.cpp -o nqueens
// ./nqueens

#include <bits/stdc++.h>
using namespace std;

int verify(const int* board, int n) {
    int m = 2 * n - 1;
    std::vector<int> rowConstraints(n, 0);
    std::vector<int> rightConstraints(m, 0);
    std::vector<int> leftConstraints(m, 0);

    for (int col = 0; col < n; col++) {
        int row = board[col];
        rowConstraints[row]++;
        rightConstraints[row + col]++;
        leftConstraints[row - col + n - 1]++;
    }

    for (int col = 0; col < n; col++) {
        int row = board[col];
        if (rowConstraints[row] +
            rightConstraints[row + col] +
            leftConstraints[row - col + n - 1] != 3)
        {
            return 0;
        }
    }
    return 1;
}

static std::mt19937_64 rng((uint64_t)chrono::high_resolution_clock::now().time_since_epoch().count());

inline int count_total_conflicts(int N, const vector<int>& queens) {
    vector<int> rows(N, 0), d1(2 * N, 0), d2(2 * N, 0);
    int c_count = 0;

    for (int c = 0; c < N; ++c) {
        int r = queens[c];
        rows[r]++;
        d1[r - c + N]++;
        d2[r + c]++;
    }

    for (int x : rows) if(x > 1) c_count += x - 1;
    for (int x : d1) if(x > 1) c_count += x - 1;
    for (int x : d2) if(x > 1) c_count += x - 1;
    
    return c_count;
}

void optimized_greedy_init(int N, vector<int>& queens, 
                           vector<int>& row_count,
                           vector<int>& diag1, 
                           vector<int>& diag2) {
    fill(row_count.begin(), row_count.end(), 0);
    fill(diag1.begin(), diag1.end(), 0);
    fill(diag2.begin(), diag2.end(), 0);
    
    int sample_size;
    if (N <= 1000) {
        sample_size = N;
    } else if (N <= 10000) {
        sample_size = 2000;
    } else {
        sample_size = 5000;
    }
    
    for (int col = 0; col < N; ++col) {
        int best_row = 0;
        int min_conflicts = INT_MAX;
        
        for (int i = 0; i < sample_size; ++i) {
            int row = (sample_size >= N) ? i : (rng() % N);
            int conflicts = row_count[row] + diag1[row - col + N] + diag2[row + col];
            
            if (conflicts < min_conflicts || (conflicts == min_conflicts && (rng() & 1))) {
                min_conflicts = conflicts;
                best_row = row;
                if (conflicts == 0) break;
            }
        }
        
        queens[col] = best_row;
        row_count[best_row]++;
        diag1[best_row - col + N]++;
        diag2[best_row + col]++;
    }
}

pair<vector<int>, int> min_conflicts_aggressive(int N, vector<int> queens, int max_steps, ofstream* log = nullptr) {
    vector<int> row_count(N, 0);
    vector<int> diag1(2 * N, 0);
    vector<int> diag2(2 * N, 0);
    
    for (int c = 0; c < N; ++c) {
        int r = queens[c];
        row_count[r]++;
        diag1[r - c + N]++;
        diag2[r + c]++;
    }
    
    auto get_conflicts = [&](int r, int c) {
        return row_count[r] + diag1[r - c + N] + diag2[r + c];
    };
    
    auto calc_conflict = [&](int c) {
        int r = queens[c];
        return (row_count[r] - 1) + (diag1[r - c + N] - 1) + (diag2[r + c] - 1);
    };
    
    vector<int> conflicted;
    for (int c = 0; c < N; ++c) {
        if (calc_conflict(c) > 0) {
            conflicted.push_back(c);
        }
    }

    if (log) {
        *log << "  Initial conflicts: " << conflicted.size() << " queens\n";
    }

    int row_sample;
    if (N <= 1000) {
        row_sample = N;
    } else if (N <= 10000) {
        row_sample = 2000;
    } else {
        row_sample = 5000;
    }
    
    for (int step = 0; step < max_steps; ++step) {
        if (conflicted.empty()) {
            if (log) *log << "  SOLVED at step " << step << "\n";
            return {queens, step};
        }
        
        int idx = rng() % conflicted.size();
        int col = conflicted[idx];
        
        if (calc_conflict(col) == 0) {
            conflicted[idx] = conflicted.back();
            conflicted.pop_back();
            continue;
        }
        
        int old_row = queens[col];
        int old_conflicts = calc_conflict(col);
        
        row_count[old_row]--;
        diag1[old_row - col + N]--;
        diag2[old_row + col]--;
        
        int best_row = old_row;
        int min_conf = get_conflicts(old_row, col);
        
        for (int i = 0; i < row_sample; ++i) {
            int r = (row_sample >= N) ? i : (rng() % N);
            int conf = get_conflicts(r, col);
            
            if (conf < min_conf || (conf == min_conf && (rng() & 1))) {
                min_conf = conf;
                best_row = r;
                if (conf == 0) break;
            }
        }
        
        queens[col] = best_row;
        row_count[best_row]++;
        diag1[best_row - col + N]++;
        diag2[best_row + col]++;
        
        bool resolved = false;
        if (calc_conflict(col) == 0) {
            conflicted[idx] = conflicted.back();
            conflicted.pop_back();
            resolved = true;
        }
        
        if (log && (N <= 100 || step % 10 == 0 || resolved)) {
            *log << "  Step " << step << ": Moved Q" << col 
                 << " from row " << old_row << " (conflicts=" << old_conflicts 
                 << ") to row " << best_row << " (conflicts=" << min_conf << ")";
            if (resolved) *log << " [RESOLVED]";
            *log << " | Remaining conflicts: " << conflicted.size() << "\n";
        }
        
        if (step % 100 == 0 && step > 0) {
            conflicted.clear();
            for (int c = 0; c < N; ++c) {
                if (calc_conflict(c) > 0) {
                    conflicted.push_back(c);
                }
            }
        }
    }

    if (log) *log << "  FAILED: Max steps reached with " << conflicted.size() << " conflicts remaining\n";
    return {{}, max_steps};
}

pair<vector<int>, int> solve_nqueens(int N, int max_total_steps = 1000000) {
    if (N == 1) return {{0}, 0};
    if (N <= 3) return {{}, 0};
    
    ofstream detailed_log("detailed_log_N" + to_string(N) + ".txt");
    if (detailed_log) {
        detailed_log << "=== Min-Conflicts N-Queens Solver ===\n";
        detailed_log << "N = " << N << "\n";
        detailed_log << "Max total steps = " << max_total_steps << "\n\n";
    }
    
    int total_steps = 0;
    
    int max_attempts, steps_per_attempt;
    if (N <= 10000) {
        max_attempts = 200;
        steps_per_attempt = 5000;
    } else if (N <= 100000) {
        max_attempts = 100;
        steps_per_attempt = 10000;
    } else {
        max_attempts = 50;
        steps_per_attempt = 1000000;
    }
    
    for (int attempt = 0; attempt < max_attempts && total_steps < max_total_steps; ++attempt) {
        vector<int> queens(N);
        vector<int> row_count(N);
        vector<int> diag1(2 * N);
        vector<int> diag2(2 * N);
        
        if (detailed_log) {
            detailed_log << "--- Attempt " << (attempt + 1) << " ---\n";
        }
        
        optimized_greedy_init(N, queens, row_count, diag1, diag2);
        
        int initial_conflicts = count_total_conflicts(N, queens);
        
        if (detailed_log) {
            detailed_log << "Greedy initialization complete\n";
            detailed_log << "Initial conflict count: " << initial_conflicts << "\n";
        }
        
        if (initial_conflicts == 0) {
            if (detailed_log) {
                detailed_log << "SOLVED during initialization!\n";
            }
            return {queens, 0};
        }
        
        ofstream* log_ptr = (N <= 100 || attempt < 3) ? &detailed_log : nullptr;
        auto [solution, steps] = min_conflicts_aggressive(N, queens, steps_per_attempt, log_ptr);
        total_steps += steps;
        
        if (!solution.empty() && count_total_conflicts(N, solution) == 0) {
            if (detailed_log) {
                detailed_log << "\n=== SOLUTION FOUND ===\n";
                detailed_log << "Total attempts: " << (attempt + 1) << "\n";
                detailed_log << "Total steps: " << total_steps << "\n";
            }
            return {solution, total_steps};
        }
        
        if (N >= 100000 && attempt % 10 == 9) {
            cerr << "  Attempt " << (attempt+1) << "/" << max_attempts 
                 << ", steps: " << total_steps << "\r" << flush;
        }
    }
    
    if (detailed_log) {
        detailed_log << "\n=== FAILED ===\n";
        detailed_log << "Could not find solution within " << max_attempts << " attempts\n";
    }
    
    return {{}, total_steps};
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    ofstream fout("results.txt", ios::app);
    if (!fout) {
        cerr << "Failed to open results.txt\n";
        return 1;
    }

    cerr << "Running target N values (10, 100, 1000, 10000, 100000, 1000000)...\n";

    vector<int> targets = {10, 100, 1000, 10000, 100000, 1000000};

    for (int N : targets) {
        cerr << "Solving N=" << N << " ..." << endl;
        auto t0 = chrono::high_resolution_clock::now();
        auto res = solve_nqueens(N);
        auto t1 = chrono::high_resolution_clock::now();

        auto &solution = res.first;
        int steps = res.second;
        double elapsed = chrono::duration<double>(t1 - t0).count();

        fout << "N=" << N 
             << " steps=" << steps 
             << " time=" << elapsed << "s";
        
        if (!solution.empty()) {
            if (verify(solution.data(), N)) {
                fout << " status=OK\n";
                cerr << "  Verified OK. time=" << elapsed << "s steps=" << steps << "\n";
            } else {
                if (count_total_conflicts(N, solution) == 0) {
                    fout << " status=INTERNAL_OK\n";
                    cerr << "  Internal verification OK but verify failed (unexpected)\n";
                } else {
                    fout << " status=FAIL\n";
                    cerr << "  Solution invalid (conflicts remain)\n";
                }
            }
        }
        else {
            fout << " status=FAIL\n";
            cerr << "  No solution found for N=" << N << " (in given attempts/steps)\n";
        }
        
        // Output solution visualization for small N
        if (N <= 10000 && !solution.empty()) {
            ofstream solution_file("solution_N" + to_string(N) + ".txt");
            if (solution_file) {
                solution_file << "Solution for N=" << N << "\n";
                solution_file << "Queen positions (column, row):\n";
                for (int col = 0; col < N; ++col) {
                    solution_file << "Q" << col << ": (" << col << ", " << solution[col] << ")\n";
                }
                solution_file << "\nBoard visualization:\n";
                for (int row = 0; row < N; ++row) {
                    for (int col = 0; col < N; ++col) {
                        solution_file << (solution[col] == row ? "Q " : ". ");
                    }
                    solution_file << "\n";
                }
            }
        }
    }

    cerr << "\nFinished! Results appended to results.txt\n";
    cerr << "Detailed logs saved to detailed_log_N*.txt\n";
    cerr << "Solution visualizations saved to solution_N*.txt (for N<=100)\n";
    
    fout.close();
    return 0;
}