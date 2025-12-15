#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LEN 10000
#define MAX_LINES 1000

// Parse numbers in a line
int parse_numbers(const char *line, long *output) {
    char *copy = strdup(line);  // Make a copy since strtok modifies the string
    int count = 0;
    char *saveptr;
    char *token = strtok_r(copy, " \t\n", &saveptr);
    while (token != NULL) {
        output[count++] = atol(token);
        token = strtok_r(NULL, " \t\n", &saveptr);
    }
    free(copy);
    return count;
}

// Parse operators in a line
int parse_operators(const char *line, char *output) {
    char *copy = strdup(line);  // Make a copy since strtok modifies the string
    int count = 0;
    char *saveptr;
    char *token = strtok_r(copy, " \t\n", &saveptr);
    while (token != NULL) {
        output[count++] = token[0];
        token = strtok_r(NULL, " \t\n", &saveptr);
    }
    free(copy);
    return count;
}

long apply_op(char op, long a, long b) {
    switch (op) {
        case '+': return a + b;
        case '*': return a * b;
        default: return 0;
    }
}

int main() {
    FILE* fptr = fopen("inputs/day6.txt", "r");

    // Pointer to an array of chars
    // This lets me use lines[i][j] as if we had a 2D array with MAX_LINES
    // rows and MAX_LINE_LEN columns
    char (*lines)[MAX_LINE_LEN] = malloc(MAX_LINES * MAX_LINE_LEN);
    int line_count = 0;

    // Load all lines
    while (fgets(lines[line_count], MAX_LINE_LEN, fptr)) {
        lines[line_count][strcspn(lines[line_count], "\n")] = '\0';
        if (strlen(lines[line_count]) > 0) {
            line_count++;
        }
    }
    fclose(fptr);

    int num_rows = line_count - 1;

    // Parse numbers first
    // This is a pointer to a pointer of numbers, allowing to use
    // numbers[i] as an array of numbers
    long **numbers = malloc(num_rows * sizeof(long *));
    int num_cols = 0;
    for (int r = 0; r < num_rows; r++) {
        numbers[r] = malloc(MAX_LINE_LEN * sizeof(long));
        num_cols = parse_numbers(lines[r], numbers[r]);
    }

    // Parse operators
    char *operators = malloc(MAX_LINE_LEN);
    parse_operators(lines[line_count - 1], operators);

    printf("DEBUG: %d rows, %d cols\n", num_rows, num_cols);

    long total = 0;
    // For each column that needs processing
    for (int col = 0; col < num_cols; col++) {
        long result = numbers[0][col];
        // For each number to be summed or multiplied
        for (int row = 1; row < num_rows; row++) {
            result = apply_op(operators[col], result, numbers[row][col]);
        }
        total += result;
    }

    printf("Sum of all columns: %ld\n", total);

    for (int r = 0; r < num_rows; r++) {
        free(numbers[r]);
    }
    free(numbers);
    free(operators);
    free(lines);

    return 0;
}
