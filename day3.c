#include <stdio.h>
#define MAX_BLOCK_SIZE 1000


int main() {
    FILE *fptr;
    fptr = fopen("inputs/day3.txt", "r");
    if (fptr == NULL) printf("Input could not be opened.");

    long solution = 0;
    char block[MAX_BLOCK_SIZE+1];
    while (fgets(block, sizeof(block), fptr)) {
        printf("%s", block);
        int max = -1;
        int max_i;
        int max2 = -1;
        int max2_i;
        int num;
        int i = 0;


        while (block[i] != '\n') {
            num = block[i] - '0';
            if (num > max && block[i+1] != '\n') {
                max = num;
                max_i = i;
            }
            i++;
        }

        i = max_i+1;
        while (block[i] != '\n') {
            num = block[i] - '0';
            if (num > max2) {
                max2 = num;
                max2_i = i;
            }

            i++;

        }

        printf("max: %d, max2: %d \n", max, max2);
        if (max_i < max2_i) {
            printf("%d%d\n", max, max2);
            solution += 10*max + max2;
        } else {
            printf("%d%d\n", max2, max);
            solution += 10*max2 + max;
        }
        printf("\n");
    }

    printf("Solution: %ld", solution);

    fclose(fptr);

    return 0;
}
