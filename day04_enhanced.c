#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COLS 1000
#define MAX_ROWS 1000

typedef struct {
    int r, c;
} Cell;

typedef struct {
    Cell* data;
    int front, rear, capacity;
} Queue;

void queue_init(Queue* q, int capacity) {
    q->data = malloc(capacity * sizeof(Cell));
    q->front = q->rear = 0;
    q->capacity = capacity;
}

void queue_push(Queue* q, int r, int c) {
    q->data[q->rear++] = (Cell){r, c};
}

Cell queue_pop(Queue* q) {
    return q->data[q->front++];
}

int queue_empty(Queue* q) {
    return q->front >= q->rear;
}

int valid_coordinates(int r, int c, int n_rows, int n_cols) {
    return r >= 0 && c >= 0 && r < n_rows && c < n_cols;
}

int count_neighbours(int r, int c, int n_rows, int n_cols, int** grid) {
    int total = 0;
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;
            int r2 = r + dr, c2 = c + dc;
            if (valid_coordinates(r2, c2, n_rows, n_cols) && grid[r2][c2]) {
                total++;
            }
        }
    }
    return total;
}

int main() {
    FILE* fptr = fopen("inputs/day4.txt", "r");

    int n_rows = 0, n_cols = 0;
    int* grid[MAX_ROWS];
    int* in_queue[MAX_ROWS];  // Track what's already queued
    char buffer[MAX_COLS];

    // Read grid
    while (fgets(buffer, sizeof(buffer), fptr)) {
        if (n_cols == 0) n_cols = strlen(buffer) - 1;
        grid[n_rows] = malloc(n_cols * sizeof(int));
        in_queue[n_rows] = calloc(n_cols, sizeof(int));
        for (int i = 0; i < n_cols; i++) {
            grid[n_rows][i] = (buffer[i] == '@') ? 1 : 0;
        }
        n_rows++;
    }
    fclose(fptr);

    // Initialize queue with all removable cells
    Queue q;
    queue_init(&q, n_rows * n_cols);

    // Find all the initial removable paper cells
    for (int r = 0; r < n_rows; r++) {
        for (int c = 0; c < n_cols; c++) {
            if (grid[r][c] && count_neighbours(r, c, n_rows, n_cols, grid) < 4) {
                queue_push(&q, r, c);
                in_queue[r][c] = 1;
            }
        }
    }

    int solution = 0;

    // From an initial queue of removable cells, remove them, and then look at their neighbours,
    // some of which will be added to the queue, and so on
    while (!queue_empty(&q)) {
        Cell cell = queue_pop(&q);
        int r = cell.r, c = cell.c;

        // Remove this cell
        grid[r][c] = 0;
        solution++;
        in_queue[r][c] = 0;

        // Check all neighbours of this cell, which might now be removable
        for (int dr = -1; dr <= 1; dr++) {
            for (int dc = -1; dc <= 1; dc++) {
                if (dr == 0 && dc == 0) continue;
                int r2 = r + dr, c2 = c + dc;
                if (valid_coordinates(r2, c2, n_rows, n_cols) &&
                    grid[r2][c2] && !in_queue[r2][c2]) {
                    queue_push(&q, r2, c2);
                    in_queue[r2][c2] = 1;
                }
            }
        }
    }

    printf("The solution was %d\n", solution);

    // Cleanup
    for (int i = 0; i < n_rows; i++) {
        free(grid[i]);
        free(in_queue[i]);
    }
    free(q.data);

    return 0;
}
