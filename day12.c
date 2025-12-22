#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_LEN_LINE 100
#define MAX_DIGIT 3
#define MAX_CELLS 9
#define MAX_SHAPES 9

typedef struct {
    int row;
    int col;
} Coordinates;

typedef struct {
    int n_cells;
    Coordinates cells[MAX_CELLS];
} Shape;

void shape_add_coordinates(Shape *shape, int row, int col) {
    shape->cells[shape->n_cells++] = (Coordinates){row, col};
}

bool next_shape(int repetitions[], int rep_size, int *shape, int *rep) {
    if (*rep == rep_size-1 && *shape == repetitions[*rep]) return false;
    if (*shape == repetitions[*rep]-1) {
        if (*rep == rep_size-1) return false;
        *rep = 0;
        (*shape)++;
        return true;
    }
    (*rep)++;
    return true;
}

int coord_to_i(Coordinates c, int width) {
    return c.row + (c.col * width);
}

bool can_fit(bool region[], int width, Shape s) {
    for (int i = 0; i < s.n_cells; i++) {
        Coordinates c = s.cells[i];
        if (region[coord_to_i(c, width)]) return false;
    }
    return true;
}

void set_region(bool region[], int width, Shape s, bool val) {
    for (int i = 0; i < s.n_cells; i++) {
        Coordinates c = s.cells[i];
        region[coord_to_i(c, width)] = val;
    }
}

int fit_shapes(
    bool region[],
    int width,
    int repetitions[],
    int rep_size,
    Shape shapes[],
    int n_shapes,
    int shape,
    int rep
) {
    // for all valid configurations:
    Shape s = shapes[shape];
    for () {
        if (!can_fit(region, width, s)) continue;

        set_region(region, width, s, true);

        if (!next_shape(repetitions, rep_size, &shape, &rep))
            return 1;
        if (fit_shapes(region, width, repetitions, rep_size, shapes, n_shapes, shape, rep))
            return 1;

        // Backtrack
        set_region(region, width, s, false);
    }



    return 0;
}




int process_region(char line[], Shape shapes[], int n_shapes) {
    char buffer[MAX_DIGIT+1];
    int size_buffer = 0;

    int repetitions[MAX_SHAPES];
    int rep_size = 0;

    int r_width;
    int r_height;
    for (int i = 0; line[i] != '\0'; i++) {
        if (isdigit(line[i])) {
            buffer[size_buffer++] = line[i];
        } else if (line[i] == 'x') {
            buffer[size_buffer] = '\0';
            r_width = atoi(buffer);
            size_buffer = 0;
        } else if (line[i] == ':') {
            buffer[size_buffer] = '\0';
            r_height = atoi(buffer);
            size_buffer = 0;
        } else if (line[i] == ' ' || line[i] == '\n') {
            if (!size_buffer) continue;
            buffer[size_buffer] = '\0';
            repetitions[rep_size++] = atoi(buffer);
            size_buffer = 0;
        }
    }

    bool region[r_width * r_height];
    for (int i = 0; i < r_width * r_height ; i ++) region[i] = false;


    if (fit_shapes(region, r_width, repetitions, rep_size, shapes, n_shapes, 0, 0))
        return 1;
    else return 0;
}

int main() {
    FILE *fptr = fopen("inputs/day12s.txt", "r");

    int solution = 0;

    char line[MAX_LEN_LINE];
    int shape_col = 0;

    Shape shapes[MAX_SHAPES];
    int n_shapes = 0;

    bool read_regions = false;
    while (fgets(line, sizeof(line), fptr)) {

        for (int i = 0; line[i] != '\0'; i++) {
            if (!read_regions) {
                if (line[i] == 'x') {
                    read_regions = true;
                    solution += process_region(line, shapes, n_shapes);
                    break;
                } else if (line[i] == ':') {
                    n_shapes++;
                    shape_col = 0;
                    shapes[n_shapes-1].n_cells = 0;
                    break;
                } else if (line[i] == '\n') {
                    shape_col++;
                    break;
                } else if (line[i] == '#')
                    shape_add_coordinates(&shapes[n_shapes-1], i, shape_col);

            } else {
                solution += process_region(line, shapes, n_shapes);
                break;
            }

        }

    }

    printf("Solution: %d\n", solution);


    fclose(fptr);
    return 0;
}
