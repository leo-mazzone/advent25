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

void checkNumber(long n, long *sum) {
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
        long base = 1;
        for (int i = 0; i < pattern_len; i++) {
            base *= 10;
        }

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

void checkRange(long left, long right, long *sum) {
    printf("%ld - %ld \n", left, right);
    for (long i=left; i<=right; i++) {
        checkNumber(i, sum);
    }
}

int main() {
    FILE* fptr;
    fptr = fopen("inputs/day2s.txt", "r");
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
            checkRange(left, right, &sum);
            n_digits = 0;
        } else {
            buffer[n_digits] = cc;
            n_digits += 1;
        }
    }
    buffer[n_digits] = 0;
    right = atol(buffer);
    checkRange(left, right, &sum);

    printf("The solution is %ld\n", sum);
    return 0;
}
