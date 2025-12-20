#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>

#define MAX_LINE_LEN 500
#define MAX_COUNTERS 20
#define MAX_BUTTONS 25

static int g_n_counters;
static int g_n_buttons;
static long long g_targets[MAX_COUNTERS];
static int g_button_affects[MAX_BUTTONS][MAX_COUNTERS];

// Use rational arithmetic: store as numerator/denominator pairs
// Means we can check if a solution is an integer without rounding errors
typedef struct {
    long long num;
    long long den;
} Rational;

// After Gaussian elimination:
static Rational rref[MAX_COUNTERS][MAX_BUTTONS + 1];
static int pivot_col[MAX_COUNTERS];
static int pivot_row_for_col[MAX_BUTTONS];
static int free_vars[MAX_BUTTONS];
static int n_pivots;
static int n_free;

static long long best_result;
static long long max_target;

//------------ Rational arithmetic ------------

// Euclidean algorithm
// Repeatedly applies gcd(a, b) = gcd(b, a mod b)
long long gcd(long long a, long long b) {
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    while (b) {
        long long t = b;
        b = a % b;
        a = t;
    }
    return a;
}

void simplify(Rational *r) {
    if (r->num == 0) {
        r->den = 1;
        return;
    }
    long long g = gcd(r->num, r->den);
    r->num /= g;
    r->den /= g;
    if (r->den < 0) {
        r->num = -r->num;
        r->den = -r->den;
    }
}

Rational rat_add(Rational a, Rational b) {
    Rational r;
    r.num = a.num * b.den + b.num * a.den;
    r.den = a.den * b.den;
    simplify(&r);
    return r;
}

Rational rat_sub(Rational a, Rational b) {
    Rational r;
    r.num = a.num * b.den - b.num * a.den;
    r.den = a.den * b.den;
    simplify(&r);
    return r;
}

Rational rat_mul(Rational a, Rational b) {
    Rational r;
    r.num = a.num * b.num;
    r.den = a.den * b.den;
    simplify(&r);
    return r;
}

Rational rat_div(Rational a, Rational b) {
    Rational r;
    r.num = a.num * b.den;
    r.den = a.den * b.num;
    simplify(&r);
    return r;
}

bool rat_is_zero(Rational r) {
    return r.num == 0;
}

Rational rat_from_int(long long x) {
    Rational r = {x, 1};
    return r;
}

// ------------ Solve linear system ------------

// Bring augmented matrix to Reduced Row Echelon Form (RREF)
void gaussian_eliminate() {
    Rational aug[MAX_COUNTERS][MAX_BUTTONS + 1];

    // Fill in augmented matrix
    for (int j = 0; j < g_n_counters; j++) {
        for (int i = 0; i < g_n_buttons; i++) {
            aug[j][i] = rat_from_int(g_button_affects[i][j]);
        }
        aug[j][g_n_buttons] = rat_from_int(g_targets[j]);
    }

    n_pivots = 0;
    for (int i = 0; i < g_n_buttons; i++) {
        pivot_row_for_col[i] = -1;
    }

    // The next row that needs a pivot
    int next_pivot_row = 0;

    // Find first non-zero entry in this column. We stop searching if:
    // - We've processed all columns, or
    // - We've filled all rows with pivots
    for (int col = 0; col < g_n_buttons && next_pivot_row < g_n_counters; col++) {
        int piv_row = -1;
        for (int row = next_pivot_row; row < g_n_counters; row++) {
            if (!rat_is_zero(aug[row][col])) {
                piv_row = row;
                break;
            }
        }

        if (piv_row < 0) continue; // No pivot in this column (it's a free variable), skip it

        // Swap rows so we move the pivot to current row that needs one
        for (int c = 0; c <= g_n_buttons; c++) {
            Rational tmp = aug[next_pivot_row][c];
            aug[next_pivot_row][c] = aug[piv_row][c];
            aug[piv_row][c] = tmp;
        }

        // Scale pivot row so pivot = 1
        Rational scale = aug[next_pivot_row][col];
        for (int c = 0; c <= g_n_buttons; c++) {
            aug[next_pivot_row][c] = rat_div(aug[next_pivot_row][c], scale);
        }

        // Eliminate other values in this column
        for (int row = 0; row < g_n_counters; row++) {
            if (row != next_pivot_row && !rat_is_zero(aug[row][col])) {
                Rational factor = aug[row][col];
                for (int c = 0; c <= g_n_buttons; c++) {
                    aug[row][c] = rat_sub(aug[row][c], rat_mul(factor, aug[next_pivot_row][c]));
                }
            }
        }

        pivot_col[next_pivot_row] = col;
        pivot_row_for_col[col] = next_pivot_row;
        next_pivot_row++;
    }

    n_pivots = next_pivot_row;

    // Copy to rref
    for (int i = 0; i < g_n_counters; i++) {
        for (int j = 0; j <= g_n_buttons; j++) {
            rref[i][j] = aug[i][j];
        }
    }

    // Identify free variables
    n_free = 0;
    for (int col = 0; col < g_n_buttons; col++) {
        if (pivot_row_for_col[col] < 0) {
            free_vars[n_free++] = col;
        }
    }
}

bool is_consistent() {
    // After gaussian elimination, we expect the bottom
    // (n_rows - n_pivot_rows) to have 0 in the augmenting
    // column. If not, can't solve
    for (int row = n_pivots; row < g_n_counters; row++) {
        if (!rat_is_zero(rref[row][g_n_buttons])) {
            return false;
        }
    }
    return true;
}

// Given free variable assignments, compute pivot values
// Returns false if any pivot is not a non-negative integer
bool compute_pivots(long long free_vals[], long long pivot_vals[]) {
    for (int row = 0; row < n_pivots; row++) {
        // val = target - sum(coeff * free_val)
        Rational val = rref[row][g_n_buttons];

        for (int f = 0; f < n_free; f++) {
            int col = free_vars[f];
            Rational contrib = rat_mul(rref[row][col], rat_from_int(free_vals[f]));
            val = rat_sub(val, contrib);
        }

        // Check if val is a non-negative integer
        if (val.den != 1 && val.den != -1) return false;
        long long ival = (val.den == 1) ? val.num : -val.num;
        if (ival < 0) return false;

        pivot_vals[row] = ival;
    }
    return true;
}

// Could the current partial assignment of free variables lead to a valid solution?
// I.e. given the free variables we've assigned so far, is it still possible for all
// pivot variables to end up non-negative?
bool can_be_feasible(long long free_vals[], int idx) {
    // Check each pivot variable
    for (int row = 0; row < n_pivots; row++) {
        Rational val = rref[row][g_n_buttons];

        // Check all currently assigned free variables
        // Contributions from unassigned variables are not known yet
        for (int f = 0; f < idx; f++) {
            int col = free_vars[f];
            val = rat_sub(val, rat_mul(rref[row][col], rat_from_int(free_vals[f])));
        }

        // If val < 0, check if remaining free vars can help
        bool val_negative = (val.num < 0 && val.den > 0) || (val.num > 0 && val.den < 0);

        if (val_negative) {
            bool can_recover = false;
            for (int f = idx; f < n_free; f++) {
                int col = free_vars[f];
                // Coefficient is negative if it can increase val when multiplied by positive free var
                // val_new = val - coeff * free_val
                // For val to increase, we need coeff < 0
                Rational coeff = rref[row][col];
                bool coeff_negative = (coeff.num < 0 && coeff.den > 0) || (coeff.num > 0 && coeff.den < 0);
                if (coeff_negative) {
                    can_recover = true;
                    break;
                }
            }
            if (!can_recover) return false;
        }
    }
    return true;
}

// Search all combinations of values to assign free variables,
// recursing on next index, and with three optimisations:
// - Prune search branch if free variable values in this branch would
//   force pivot variable to be negative
// - Prune search branch if free variable values in this branch would
//   exceed max button presses found with a different solution
// - Determine dynamically upper bound on next variable
void search(int idx, long long free_vals[], long long current_sum) {
    // `best_result` tracks best across leaves of the search tree
    if (current_sum >= best_result) return;

    if (!can_be_feasible(free_vals, idx)) return;

    // Last free variable
    if (idx == n_free) {
        long long pivot_vals[MAX_COUNTERS];
        if (compute_pivots(free_vals, pivot_vals)) {
            long long total = current_sum;
            for (int row = 0; row < n_pivots; row++) {
                total += pivot_vals[row];
            }
            if (total < best_result) {
                best_result = total;
            }
        }
        return;
    }

    // Dynamically limit how high we search for next free variable
    // No point trying values that would make the total sum exceed our
    // best known solution
    long long max_val = max_target; // Reasonable upper bound
    // Reduce `max_val` to the largest value that keeps us below `best_result`
    if (current_sum + max_val >= best_result) {
        max_val = best_result - current_sum - 1;
    }
    if (max_val < 0) return;

    for (long long v = 0; v <= max_val; v++) {
        free_vals[idx] = v;
        search(idx + 1, free_vals, current_sum + v);
    }
}

long long solve_machine() {
    bool all_zero = true;
    max_target = 0;
    for (int i = 0; i < g_n_counters; i++) {
        if (g_targets[i] != 0) all_zero = false;
        if (g_targets[i] > max_target) max_target = g_targets[i];
    }
    if (all_zero) return 0;

    gaussian_eliminate();

    if (!is_consistent()) return -1;

    if (n_free == 0) {
        long long pivot_vals[MAX_COUNTERS];
        long long dummy[1] = {0};
        // Must pass a value of the right type for free_vals, but because
        // the global n_free is 0, will be ignored by compute_pivots()
        if (compute_pivots(dummy, pivot_vals)) {
            long long total = 0;
            for (int row = 0; row < n_pivots; row++) {
                total += pivot_vals[row];
            }
            return total;
        }
        return -1;
    }

    // Multiple solutions: must search
    // Any valid solution should need at most one press per button
    long long upper = 0;
    for (int i = 0; i < g_n_counters; i++) {
        upper += g_targets[i];
    }
    best_result = upper + 1;

    long long free_vals[MAX_BUTTONS] = {0};
    // Start searching at index 0; and with current sum of 0
    search(0, free_vals, 0);

    // No solution if search() can't find a best_result that's at most `upper`
    return best_result > upper ? -1 : best_result;
}

// ------------ Read and process input ------------

void read_wirings(char *line, int *i, int *n_buttons, int button_affects[][MAX_COUNTERS]) {
    (*i)++;
    int button = *n_buttons;

    for (int j = 0; j < MAX_COUNTERS; j++) {
        button_affects[button][j] = 0;
    }

    while (line[*i] != ')' && line[*i] != '\0') {
        if (line[*i] >= '0' && line[*i] <= '9') {
            int num = 0;
            while (line[*i] >= '0' && line[*i] <= '9') {
                num = num * 10 + (line[*i] - '0');
                (*i)++;
            }
            if (num < MAX_COUNTERS) {
                button_affects[button][num] = 1;
            }
        } else {
            (*i)++;
        }
    }
    (*n_buttons)++;
}

int read_joltage(char *line, int *i, long long *targets) {
    (*i)++;
    int idx = 0;
    while (line[*i] != '}' && line[*i] != '\0') {
        if (line[*i] >= '0' && line[*i] <= '9') {
            long long num = 0;
            while (line[*i] >= '0' && line[*i] <= '9') {
                num = num * 10 + (line[*i] - '0');
                (*i)++;
            }
            targets[idx] = num;
            idx++;
        } else {
            (*i)++;
        }
    }
    return idx;
}

long long process_line(char *line, int line_num) {
    int i = 0;
    int n_counters = 0;
    int n_buttons = 0;
    long long targets[MAX_COUNTERS] = {0};
    int button_affects[MAX_BUTTONS][MAX_COUNTERS] = {0};

    char c = line[i];
    while (c != '\0') {
        if (c == '(') read_wirings(line, &i, &n_buttons, button_affects);
        if (c == '{') n_counters = read_joltage(line, &i, targets);
        i++;
        c = line[i];
    }

    g_n_counters = n_counters;
    g_n_buttons = n_buttons;
    memcpy(g_targets, targets, sizeof(long long) * n_counters);
    memcpy(g_button_affects, button_affects, sizeof(button_affects));

    long long result = solve_machine();

    printf("Line %d: C=%d, B=%d, Free=%d -> %lld\n", line_num, n_counters, n_buttons, n_free, result);

    return result;
}

int main() {
    FILE *fptr = fopen("inputs/day10.txt", "r");

    char line[MAX_LINE_LEN];
    long long total = 0;
    int line_num = 0;

    while (fgets(line, sizeof(line), fptr)) {
        line_num++;
        long long result = process_line(line, line_num);
        if (result > 0) {
            total += result;
        }
    }

    printf("Solution: %lld\n", total);

    fclose(fptr);
    return 0;
}
