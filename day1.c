#include <stdio.h>
#include <stdlib.h>
#define WHEEL_LEN 100
#define MAX_MOVEMENT_DIGITS 4

int main() {
    FILE* fptr;
    fptr = fopen("inputs/day1.txt", "r");
    if (fptr == NULL) printf("Input could not be opened.");

    char command[1+MAX_MOVEMENT_DIGITS+1];
    int position = 50;
    int zero_counter = 0;
    while (fgets(command, sizeof(command), fptr)) {
        int movement = atoi(command+1);
        int zero_increment = 0;
        if (command[0] == 'L') {
            printf("%d L %d", position, movement);
            if (movement == position) {
                zero_increment = 1;
            } else if (movement > position) {
                int add1 = position != 0; // Don't count current 0
                // First go to zero, then see how many moves left you have left
                zero_increment = add1 + ((movement - position) / WHEEL_LEN);
            }
            zero_counter += zero_increment;
            // If movement makes position negative, we must convert it
            // to positive first, before bringing it back to 0-99 range
            position = ((position - movement) % WHEEL_LEN + WHEEL_LEN) % WHEEL_LEN;
        } else {
            printf("%d R %d", position, movement);
            zero_increment = (movement + position) / WHEEL_LEN;
            zero_counter += zero_increment;

            position = (position + movement) % WHEEL_LEN;
        }
        printf(" -> %d", position);
        printf(" (new zeroes: %d)\n", zero_increment);
    }

    fclose(fptr);

    printf("%d\n", zero_counter);


    return 0;
}
