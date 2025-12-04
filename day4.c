#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_COLS 1000
#define MAX_ROWS 1000

void process_line(char* buffer, int n_cols, int* n_rows, int** grid) {
    grid[*n_rows] = malloc(n_cols * sizeof(int));
    for (int i=0; buffer[i] != '\0'; i++) {
        grid[*n_rows][i] = buffer[i] == '@' ? 1 : 0;
    }
    (*n_rows)++;
}

int valid_coordinates(int r, int c, int n_rows, int n_cols) {
    if ((r < 0) || (c < 0) || (r >= n_rows) || (c >= n_cols)) return 0;
    return 1;
}

int count_removable(int n_rows, int n_cols, int** grid) {
    int removable = 0;
    for (int r=0; r<n_rows; r++) {
        for (int c=0; c<n_cols; c++) {
            int total = 0;
            if (!grid[r][c]) continue;

            // Iterate over adjacent cells
            for (int r2=r-1; r2<=r+1; r2++) {
                for (int c2=c-1; c2<=c+1; c2++) {
                    if (r2 == r && c2 == c) continue;
                    if (valid_coordinates(r2, c2, n_rows, n_cols)) {
                        if (grid[r2][c2]) total++;
                    }
                }
            }

            if (total < 4) removable++;
        }
    }
    return removable;
}

int main() {
    FILE* fptr;
    fptr = fopen("inputs/day4.txt", "r");

    int n_rows = 0;
    int n_cols = 0;
    int* grid[MAX_ROWS];

    char buffer[MAX_COLS];
    // Read first line to determine number of columns
    fgets(buffer, sizeof(buffer), fptr);
    n_cols = strlen(buffer) - 1;
    process_line(buffer, n_cols, &n_rows, grid);
    // Read all remaining lines
    while (fgets(buffer, sizeof(buffer), fptr)) {
        process_line(buffer, n_cols, &n_rows, grid);
    }
    // Print grid for sense-checking
    for (int r=0; r<n_rows; r++) {
        for (int c=0; c<n_cols; c++) {
            printf("%d", grid[r][c]);
        }
        printf("\n");
    }
    printf("%d x %d\n", n_cols, n_rows);

    int solution = 0;
    solution += count_removable(n_rows, n_cols, grid);
    printf("The solution was %d\n", solution);

    // Clean up
    for (int i=0; i<n_rows; i++) {
        free(grid[i]);
    }
    fclose(fptr);

    return 0;
}
