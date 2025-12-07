#include <stdio.h>
#define MAX_LINE_LEN 1000

int main() {
    FILE* fptr = fopen("inputs/day7.txt", "r");

    char line[MAX_LINE_LEN];
    long beams[MAX_LINE_LEN];
    long beams_next[MAX_LINE_LEN];

    // beams and beams_next are never used directly.
    // They are access through these two pointers, which
    // we swap after processing each line
    long *beams_ptr = beams;
    long *beams_next_ptr = beams_next;

    // Initialise both state arrays to 0
    for (int i=0; i<MAX_LINE_LEN; i++) {
        beams_next_ptr[i] = 0;
        beams_ptr[i] = 0;
    }

    int first_line = 1;
    int line_len = 0;
    // Process each line
    while (fgets(line, sizeof(line), fptr)) {
        // Process each char in line
        for (int i=0; line[i]!='\0'; i++) {
            if (first_line) line_len++;
            if (line[i] == 'S') {
                // First particle is generated
                beams_next_ptr[i] = 1;
            } else {
                long carry_over = line[i] == '^' ? 0 : beams_ptr[i];
                long split_left = 0, split_right = 0;
                if (i-1 >= 0 && line[i-1] == '^') {
                    split_left = beams_ptr[i-1];
                }
                if (i+1 >= 0 && line[i+1] == '^') {
                    split_right = beams_ptr[i+1];
                }
                // Each position keeps track of how many timelines have a
                // particle passing through. If no splitter above, however
                // many above carry on undisturbed, else those would be moved
                // left and right, and we'd count those in a different iteration.
                // If there are splinters left and/or right, we need to add the
                // particles in each timeline joining this position.
                beams_next_ptr[i] = carry_over + split_left + split_right;
            }
        }

        // Print next state for debugging
        for (int i=0; i<line_len; i++) {
            printf("%ld", beams_next_ptr[i]);
        }
        printf("\n");

        // Swap old and new state
        long *tmp = beams_ptr;
        beams_ptr = beams_next_ptr;
        beams_next_ptr = tmp;

        first_line = 0;
    }
    fclose(fptr);

    long solution = 0;
    // beams_ptr contains the last state
    for (int i=0; i<line_len; i++) {
        solution += beams_ptr[i];
    }


    printf("Solution: %ld\n", solution);

    return 0;
}
