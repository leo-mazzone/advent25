#include <stdio.h>

#define MAX_BLOCK_SIZE 1000
#define ROUNDS 12


int main() {
    FILE *fptr;
    fptr = fopen("inputs/day3.txt", "r");
    if (fptr == NULL) printf("Input could not be opened.");

    long solution = 0;
    char block_string[MAX_BLOCK_SIZE+1];

    while (fgets(block_string, sizeof(block_string), fptr)) {
        int block[MAX_BLOCK_SIZE];
        int length = 0;

        // Read one line into an array of int
        for (int i = 0; block_string[i] != '\0' && block_string[i] != '\n'; i++) {
            block[length] = block_string[i] - '0';
            length++;
        }

        int max[ROUNDS];
        int position = 0;
        // Each round finds one battery to switch on
        // We choose digits left to right. We give maximum priority to
        // the current digit at each round, as long as there are enough
        // digits left for the remaining rounds
        for (int round=0; round<ROUNDS; round++) {
            max[round] = 0;
            for (int i=position; i<length-(ROUNDS-1-round); i++) {
                if (block[i] > max[round]) {
                    max[round] = block[i];
                    position = i + 1;
                }
            }
        }

        printf("%s", block_string);

        long power = 1;
        long joltage = 0;
        // Once you have all the digits, you need to reconstruct the joltage (sic)
        // which I do by summing each digit times its significance as a power of 10
        for (int i=ROUNDS-1; i>=0; i--) {
            joltage += power * max[i];
            power *= 10;

        }
        printf("%ld", joltage);
        printf("\n\n");
        solution += joltage;


    }


    printf("Solution 2: %ld\n", solution);

    fclose(fptr);

    return 0;
}
