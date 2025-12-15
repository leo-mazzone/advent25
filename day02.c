#include <stdio.h>
#include <stdlib.h>

#define MAX_DIGITS 100

int count_digits(long n) {
    if (n == 0) return 1;
    int count = 0;
    while (n != 0) {
        n /= 10;
        count++;
    }
    return count;
}

void check_number(long n, long *sum) {
    int num_digits = count_digits(n);
    int max_pattern_len = num_digits / 2;
    for (int pattern_len=1; pattern_len<=max_pattern_len; pattern_len++) {
        // Check if the total length is divisible by pattern length
        if (num_digits % pattern_len == 1) continue;

        int num_reps = num_digits / pattern_len;

        // Extract the pattern (first pattern_len digits)
        long divisor = 1;
        for (int i = 0; i < num_digits - pattern_len; i++) {
            divisor *= 10;
        }
        long pattern = n / divisor;

        // Calculate the multiplier: 10^(pattern_len * (n-1)) + ... + 10^pattern_len + 1
        // This is a geometric series: (10^(pattern_len * num_reps) - 1) / (10^pattern_len - 1)

        // The base is a power of 10 with as many zeroes as the length of the pattern
        long base = 1;
        for (int i = 0; i < pattern_len; i++) {
            base *= 10;
        }

        // The multiplier is a sum of powers of 10, with a 1 for each position in the number
        // where we need to place our pattern. We start from 1 (the least significant or right-most
        // appearance of the pattern), and then use the base to add enough 0s to move to the next
        // slot for our pattern
        long multiplier = 0;
        long power = 1;
        for (int i = 0; i < num_reps; i++) {
            multiplier += power;
            power *= base;
        }

        long expected_n = pattern * multiplier;

        if (n == expected_n) {
            printf("found one: %ld (pattern: %ld repeated %d times)\n",
                    n, pattern, num_reps);
            *sum += n;
            return;
        }
    }
}

void check_range(long left, long right, long *sum) {
    printf("%ld - %ld \n", left, right);
    for (long i=left; i<=right; i++) {
        check_number(i, sum);
    }
}

int main() {
    FILE* fptr;
    fptr = fopen("inputs/day2.txt", "r");
    int c;

    int n_digits = 0;
    char buffer[MAX_DIGITS];
    long left;
    long right;
    long sum = 0;

    while ((c = fgetc(fptr)) != EOF) {

        char cc = (char)c;

        if (cc == '-') {
            buffer[n_digits] = 0;
            left = atol(buffer);
            n_digits = 0;
        } else if (cc == ',') {
            buffer[n_digits] = 0;
            right = atol(buffer);
            check_range(left, right, &sum);
            n_digits = 0;
        } else {
            buffer[n_digits] = cc;
            n_digits += 1;
        }
    }
    buffer[n_digits] = 0;
    right = atol(buffer);
    check_range(left, right, &sum);

    printf("Solution 2: %ld\n", sum);
    return 0;
}
