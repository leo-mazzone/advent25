#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define MAX_LINE_LEN 100
#define MAX_CORNERS 1000
#define MAX_GRID_SIDE 100000

typedef struct {
    long x, y;
} Corner;

int compare_long(const void *a, const void *b) {
    long la = *(const long *)a;
    long lb = *(const long *)b;
    if (la < lb) return -1;
    if (la > lb) return 1;
    return 0;
}

void mark_interior_points(
    Corner corners[], int n_corners, int grid[][MAX_GRID_SIDE], long min_y, long max_y
) {
    // Check each horizontal stripe for pairs of intersecting edges
    // (it's always pairs for a polygon)
    for (long y = min_y + 1; y < max_y; y++) {
        long intersections[MAX_CORNERS];
        int n_intersections = 0;

        // For all possible vertices
        for (int i = 0; i < n_corners; i++) {
            // Get edge (defined in input as adjacent vertices)
            Corner v1 = corners[i];
            Corner v2 = corners[(i + 1) % n_corners];

            if (v1.y == v2.y) continue;

            if ((v1.y <= y && y < v2.y) || (v2.y <= y && y < v1.y)) {
                intersections[n_intersections++] = v1.x;
            }
        }

        // For this stripe, sort intersecting edges by x coordinate
        qsort(intersections, n_intersections, sizeof(long), compare_long);

        // Mark all cells between start and end intersections as green
        for (int i = 0; i + 1 < n_intersections; i += 2) {
            long x_start = intersections[i] + 1;
            long x_end = intersections[i + 1] - 1;

            for (long x = x_start; x <= x_end; x++) {
                grid[x][y] = 1;
            }
        }
    }
}

// Checking whether a potential rectangle is completely covered by green tiles
// naively (looking at each position) is O(height*width) and is too expensive because
// we do it for every possible pair of corners, which is O(N^2). Instead, we pre-compute
// the count of elements in a rectangular subset of the grid. See, for instance
// https://www.geeksforgeeks.org/dsa/prefix-sum-2d-array/
void build_prefix_sums(
    int grid[][MAX_GRID_SIDE], long prefix[][MAX_GRID_SIDE], long max_x, long max_y
) {
    // First cell
    prefix[0][0] = grid[0][0];

    // First row
    for (long x = 1; x <= max_x; x++) {
        prefix[x][0] = prefix[x - 1][0] + grid[x][0];
    }

    // First column
    for (long y = 1; y <= max_y; y++) {
        prefix[0][y] = prefix[0][y - 1] + grid[0][y];
    }

    // Rest of the grid
    for (long x = 1; x <= max_x; x++) {
        for (long y = 1; y <= max_y; y++) {
            prefix[x][y] = grid[x][y] // 0 or 1
                         + prefix[x - 1][y] // rectangle to the left (A)
                         + prefix[x][y - 1] // rectangle to the top (B)
                         - prefix[x - 1][y - 1]; // double-counted overlap of A and B
        }
    }
}

// Count how many green tiles in rectangle constructed between two corners
long rectangle_sum(long prefix[][MAX_GRID_SIDE], long x1, long y1, long x2, long y2) {
    // Ensure x1 <= x2 and y1 <= y2
    if (x1 > x2) { long t = x1; x1 = x2; x2 = t; }
    if (y1 > y2) { long t = y1; y1 = y2; y2 = t; }


    // ┌───────────────┐
    // │   A   |   B   │
    // ├───────|───────┤
    // │   C   │ query │
    // └───────────────┘

    long sum = prefix[x2][y2]; // A + B + C + query
    sum -= prefix[x1 - 1][y2]; // - (A + C)
    sum -= prefix[x2][y1 - 1]; // - (A + B)
    sum += prefix[x1 - 1][y1 - 1]; // + C = query

    return sum;
}

int is_valid(Corner a, Corner b, long prefix[][MAX_GRID_SIDE]) {
    long min_x = (a.x < b.x) ? a.x : b.x;
    long max_x = (a.x > b.x) ? a.x : b.x;
    long min_y = (a.y < b.y) ? a.y : b.y;
    long max_y = (a.y > b.y) ? a.y : b.y;

    // Interior region (excluding boundary)
    long interior_x1 = min_x + 1;
    long interior_y1 = min_y + 1;
    long interior_x2 = max_x - 1;
    long interior_y2 = max_y - 1;

    // No interior points
    if (interior_x1 > interior_x2 || interior_y1 > interior_y2) {
        return 1;
    }

    // If a potential rectangle is entirely covered in green tiles,
    // actual equals expected
    long expected = (interior_x2 - interior_x1 + 1) * (interior_y2 - interior_y1 + 1);
    long actual = rectangle_sum(prefix, interior_x1, interior_y1, interior_x2, interior_y2);

    return actual == expected;
}

int main() {
    Corner corners[MAX_CORNERS];
    int (*grid)[MAX_GRID_SIDE] = calloc(MAX_GRID_SIDE, sizeof(*grid));
    long (*prefix)[MAX_GRID_SIDE] = calloc(MAX_GRID_SIDE, sizeof(*prefix));

    int n_corners = 0;
    long min_x = LONG_MAX, max_x = LONG_MIN;
    long min_y = LONG_MAX, max_y = LONG_MIN;

    char line[MAX_LINE_LEN];
    FILE *fptr = fopen("inputs/day9.txt", "r");
    // Load input into list of corners (red tiles)
    while (fgets(line, sizeof(line), fptr)) {
        Corner c;
        sscanf(line, "%ld,%ld", &c.x, &c.y);
        corners[n_corners++] = c;
        if (c.x > max_x) max_x = c.x;
        if (c.x < min_x) min_x = c.x;
        if (c.y > max_y) max_y = c.y;
        if (c.y < min_y) min_y = c.y;
    }
    fclose(fptr);

    // Find all green tiles
    mark_interior_points(corners, n_corners, grid, min_y, max_y);
    build_prefix_sums(grid, prefix, max_x, max_y);

    long max = 0;
    // Look at all possible pairs of red tiles
    for (int i = 0; i < n_corners; i++) {
        for (int j = i + 1; j < n_corners; j++) {
            Corner a = corners[i];
            Corner b = corners[j];
            long area = (labs(a.x - b.x) + 1) * (labs(a.y - b.y) + 1);

            if (area > max && is_valid(a, b, prefix)) {
                max = area;
            }
        }
    }

    free(grid);
    free(prefix);

    printf("Max: %ld\n", max);

    return 0;
}
