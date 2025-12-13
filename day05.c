
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    long start;
    long end;
} Range;

int compare_ranges(const void *a, const void *b) {
    const Range *ra = a;
    const Range *rb = b;
    if (ra->start != rb->start)
        return (ra->start > rb->start) - (ra->start < rb->start);
    return (ra->end > rb->end) - (ra->end < rb->end);
}

int main() {
    FILE *fp = fopen("inputs/day5.txt", "r");

    char line[256];
    Range *ranges = NULL;
    size_t num_ranges = 0;
    size_t ranges_cap = 0;
    int in_second_section = 0;
    long part1 = 0;

    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';

        if (line[0] == '\0') {
            in_second_section = 1;
            continue;
        }

        if (!in_second_section) {
            // Parse and store ranges
            long start, end;
            if (sscanf(line, "%ld-%ld", &start, &end) == 2) {
                if (num_ranges >= ranges_cap) {
                    ranges_cap = ranges_cap ? ranges_cap * 2 : 16;
                    ranges = realloc(ranges, ranges_cap * sizeof(Range));
                }
                ranges[num_ranges].start = start;
                ranges[num_ranges].end = end;
                num_ranges++;
            }
        } else {
            // Check if value falls within any range
            long val;
            if (sscanf(line, "%ld", &val) == 1) {
                for (size_t i = 0; i < num_ranges; i++) {
                    if (val >= ranges[i].start && val <= ranges[i].end) {
                        part1++;
                        break;
                    }
                }
            }
        }
    }

    fclose(fp);

    // Sort ranges by start
    qsort(ranges, num_ranges, sizeof(Range), compare_ranges);

    // Merge overlapping ranges and count unique values
    long part2 = 0;
    long current_end = -1;

    for (size_t i = 0; i < num_ranges; i++) {
        if (ranges[i].start > current_end) {
            // No overlap, add full range
            part2 += ranges[i].end - ranges[i].start + 1;
            current_end = ranges[i].end;
        } else if (ranges[i].end > current_end) {
            // Partial overlap, add only the new part
            part2 += ranges[i].end - current_end;
            current_end = ranges[i].end;
        }
        // Fully contained ranges add nothing
    }


    free(ranges);

    printf("%ld\n", part1);
    printf("%ld\n", part2);
    return 0;
}
