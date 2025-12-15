#include <stdio.h>

#define MAX_LINE_LEN 1000

int main() {
    FILE* fptr = fopen("inputs/day7.txt", "r");

    char line[MAX_LINE_LEN];
    int beams[MAX_LINE_LEN];
    int beams_next[MAX_LINE_LEN];

    // beams and beams_next are never used directly.
    // They are access through these two pointers, which
    // we swap after processing each line
    int *beams_ptr = beams;
    int *beams_next_ptr = beams_next;

    int solution = 0;
    int first_line = 1;
    int line_len = 0;
    int line_i = 0;
    // Process each line
    while (fgets(line, sizeof(line), fptr)) {
        // Initialise the next state to all zeroes
        for (int i=0; i<MAX_LINE_LEN; i++) {
            beams_next_ptr[i] = 0;
        }
        line_i++;
        printf("On line %d\n", line_i);
        for (int i=0; line[i]!='\0'; i++) {
            if (first_line) line_len++;
            if (line[i] == 'S') {
                // First beam is generated
                beams_next_ptr[i] = 1;
            } else if (line[i] == '.' && beams_ptr[i] && !first_line) {
                // Beam carries on undisturbed
                beams_next_ptr[i] = 1;
            } else if (line[i] == '^' && beams_ptr[i]) {
                // We've encountered a splitter
                solution++;
                if (i-1 >= 0) {
                    printf("setting %d\n", i-1);
                    beams_next_ptr[i-1] = 1;
                }
                if (i+1 < line_len) {
                    printf("setting %d\n", i+1);
                    beams_next_ptr[i+1] = 1;
                }
            }
        }

        // Print next state for debugging
        for (int i=0; i<line_len; i++) {
            printf("%d", beams_next_ptr[i]);
        }
        printf("\n");

        // Swap old and new state
        int *tmp = beams_ptr;
        beams_ptr = beams_next_ptr;
        beams_next_ptr = tmp;

        first_line = 0;
    }
    fclose(fptr);

    printf("Solution: %d\n", solution);

    return 0;
}
