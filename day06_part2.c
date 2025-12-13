#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#define MAX_LINE_LEN 10000
#define MAX_LINES 1000

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
    int max_width = 0;

    // Load all lines
    while (fgets(lines[line_count], MAX_LINE_LEN, fptr)) {
        lines[line_count][strcspn(lines[line_count], "\n")] = '\0';
        int len = strlen(lines[line_count]);
        if (len > max_width) max_width = len;
        if (len > 0) line_count++;
    }
    fclose(fptr);

    int num_rows = line_count - 1;
    char *op_line = lines[line_count - 1];

    // Pad all lines to max_width
    for (int r = 0; r < line_count; r++) {
        int len = strlen(lines[r]);
        for (int i = len; i < max_width; i++)
            lines[r][i] = ' ';
        lines[r][max_width] = '\0';
    }

    long total = 0;
    int in_group = 0;
    char op = 0;
    long result = 0;
    int first_in_group = 1;

    // Iterate over columns
    for (int col = 0; col < max_width; col++) {
        // Check if column has any digit
        char digit_buf[MAX_LINES];
        int digit_idx = 0;

        // Iterate over rows within column (digits within number)
        // to identify number
        for (int r = 0; r < num_rows; r++) {
            if (isdigit(lines[r][col]))
                digit_buf[digit_idx++] = lines[r][col];
        }

        // Now, if we have a number, prepare for first
        // operation, or perform operation
        if (digit_idx > 0) {
            // This column has digits - we're in a group
            digit_buf[digit_idx] = '\0';
            long num = atol(digit_buf);

            if (first_in_group) {
                result = num;
                first_in_group = 0;
            } else {
                result = apply_op(op, result, num);
            }
            in_group = 1;
        } else {
            // Gap column - end of group if we were in one
            if (in_group) {
                total += result;
                first_in_group = 1;
                in_group = 0;
            }
        }

        // Check for operator
        if (op_line[col] == '+' || op_line[col] == '*')
            op = op_line[col];
    }

    // Process last group (no gap column to trigger accumulation)
    if (in_group)
        total += result;

    printf("Sum of all groups: %ld\n", total);

    free(lines);
    return 0;
}
