#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_LEN_LINE 100
#define MAX_DIGIT 3
#define MAX_CELLS 9
#define MAX_SHAPES 9
#define MAX_CONFIGS 8

typedef struct {
    int row;
    int col;
} Coordinates;

typedef struct {
    int n_cells;
    int n_configs;

    int height;
    int width;

    Coordinates cells[MAX_CONFIGS * MAX_CELLS];
} Shape;

void shape_add_coordinates(Shape *shape, int row, int col) {
    shape->cells[shape->n_cells++] = (Coordinates){row, col};
}

int coord_to_region_i(Coordinates c, int width) {
    return c.row * width + c.col;
}

void print_region(int reg[], int h, int w) {
    printf("\e[1;1H\e[2J");
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            Coordinates c = {i, j};
            int reg_i = coord_to_region_i(c, w);
            if (reg[reg_i] < 9)
                printf("%d", reg[reg_i]);
            else
                printf("*");
        }
        printf("\n");
    }
    printf("\n\n");
}


bool next_shape(int repetitions[], int rep_size, int *shape, int *rep) {
    (*rep)++;

    // Skip to next shape type(s) if done with current (handles 0 repetitions)
    while (*shape < rep_size && *rep >= repetitions[*shape]) {
        (*shape)++;
        *rep = 0;
    }

    // Return false if we've exhausted all shapes
    return *shape < rep_size;
}

Coordinates cell_i_to_coord(int i, int conf, Shape s) {
    return s.cells[i + (conf * s.n_cells)];
}


bool can_fit(int region[], int width, Shape s, int conf, int row, int col) {
    for (int i = 0; i < s.n_cells; i++) {
        Coordinates c = cell_i_to_coord(i, conf, s);
        c.row += row;
        c.col += col;
        if (region[coord_to_region_i(c, width)] < 9) {
            return false;
        }
    }
    return true;
}

void set_region(int region[], int h, int w, Shape s, int conf, int row, int col, int val) {
    for (int i = 0; i < s.n_cells; i++) {
        Coordinates c = cell_i_to_coord(i, conf, s);
        c.row += row;
        c.col += col;
        region[coord_to_region_i(c, w)] = val;
    }

    // For debugging
    // if (val != 9) {
    //     print_region(region, h, w);
    // }
}

// Encode position as single int for ordering comparisons
int encode_placement(int row, int col, int w) {
    return row * w + col;
}

int fit_shapes(
    int reg[],
    int w,
    int h,
    int repetitions[],
    Shape shapes[],
    int n_shapes,
    int s_i,
    int rep_i,
    int last_placement  // For ordering: -1 if first of this shape type, else last placement code
) {
    Shape s = shapes[s_i];
    int max_row = h - s.height;
    int max_col = w - s.width;

    for (int row = 0; row <= max_row; row++) {
        for (int col = 0; col <= max_col; col++) {
            for (int conf = 0; conf < s.n_configs; conf++) {
                // For multiple copies of same shape, enforce ordering to avoid redundant permutations
                int current_placement = encode_placement(row, col, w);
                if (last_placement >= 0 && current_placement <= last_placement) {
                    continue;
                }

                if (!can_fit(reg, w, s, conf, row, col)) continue;

                set_region(reg, h, w, s, conf, row, col, s_i);

                int next_s_i = s_i;
                int next_rep_i = rep_i;

                // Bottom of recursion
                if (!next_shape(repetitions, n_shapes, &next_s_i, &next_rep_i))
                    return 1;

                // Determine last_placement for next call
                int next_last_placement;
                if (next_s_i == s_i) {
                    // Same shape type, pass current placement for ordering
                    next_last_placement = current_placement;
                } else {
                    // Different shape type, reset ordering
                    next_last_placement = -1;
                }

                // Propagate solution up
                if (fit_shapes(reg, w, h, repetitions, shapes, n_shapes, next_s_i, next_rep_i, next_last_placement))
                    return 1;

                // Backtrack
                set_region(reg, h, w, s, conf, row, col, 9);
            }
        }
    }

    return 0;
}

int process_region(char line[], Shape shapes[], int n_shapes) {
    // printf("region\n");
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


    int region[r_width * r_height];
    // Using 9 to mean "empty" cause it was useful for debug print statements
    for (int i = 0; i < r_width * r_height ; i ++) region[i] = 9;

    if (rep_size != n_shapes) perror("Invalid line");

    int shapes_area = 0;
    for (int i = 0; i < n_shapes; i++) {
        shapes_area += shapes[i].n_cells * repetitions[i];
    }

    if (shapes_area > r_width * r_height) return 0;

    int s_i = 0;
    int rep_i = 0;

    // Skip shapes with 0 repetitions at the start
    while (s_i < n_shapes && repetitions[s_i] == 0) {
        s_i++;
    }

    // If no shapes to place
    if (s_i >= n_shapes) return 1;

    if (fit_shapes(region, r_width, r_height, repetitions, shapes, n_shapes, s_i, rep_i, -1))
        return 1;
    else return 0;
}


Coordinates translate(Coordinates c, int conf_type) {
    int centre_row = 1;
    int centre_col = 1;

    int t_row = c.row - centre_row;
    int t_col = c.col - centre_col;

    switch (conf_type) {
    case 0:
        return c;
    case 1:
        return (Coordinates){t_col + centre_row, -t_row + centre_col};
    case 2:
        return (Coordinates){-t_row + centre_row, -t_col + centre_col};
    case 3:
        return (Coordinates){-t_col + centre_row, t_row + centre_col};
    case 4:
        return (Coordinates){t_row + centre_row, -t_col + centre_col};
    case 5:
        return (Coordinates){t_col + centre_row, t_row + centre_col};
    case 6:
        return (Coordinates){-t_row + centre_row, t_col + centre_col};
    default:
        return (Coordinates){-t_col + centre_row, -t_row + centre_col};
    }
}

void print_shape(Shape s, int version) {
    for (int r = 0; r < s.height; r++) {
        for (int c = 0; c < s.width; c++) {
            int in_shape = 0;

            for (int l = version * s.n_cells; l < (version+1) * s.n_cells; l++) {
                if (s.cells[l].row == r && s.cells[l].col == c) {
                    in_shape = 1;
                    break;
                }
            }
            if (in_shape)
                printf("#");
            else
                printf(".");
        }
        printf("\n");
    }
    printf("\n\n");
}

int compare_coords(const void *a, const void *b) {
    const Coordinates *ca = (const Coordinates *)a;
    const Coordinates *cb = (const Coordinates *)b;
    if (ca->row != cb->row)
        return ca->row - cb->row;
    return ca->col - cb->col;
}

void normalize_config(Coordinates *cells, int n_cells, Coordinates *normalized) {
    // Subtracting min coordinates isn't actually helpful with this input I don't
    // think, though it is in general
    for (int i = 0; i < n_cells; i++) {
        normalized[i] = cells[i];
    }

    int min_row = normalized[0].row;
    int min_col = normalized[0].col;
    for (int i = 1; i < n_cells; i++) {
        if (normalized[i].row < min_row) min_row = normalized[i].row;
        if (normalized[i].col < min_col) min_col = normalized[i].col;
    }

    for (int i = 0; i < n_cells; i++) {
        normalized[i].row -= min_row;
        normalized[i].col -= min_col;
    }

    qsort(normalized, n_cells, sizeof(Coordinates), compare_coords);
}

bool configs_equal(Coordinates *a, Coordinates *b, int n_cells) {
    for (int i = 0; i < n_cells; i++) {
        if (a[i].row != b[i].row || a[i].col != b[i].col)
            return false;
    }
    return true;
}

void remove_duplicate_configs(Shape *s) {
    Coordinates normalized_configs[MAX_CONFIGS][MAX_CELLS];
    int unique_indices[MAX_CONFIGS];
    int n_unique = 0;

    for (int conf = 0; conf < 8; conf++) {
        normalize_config(&s->cells[conf * s->n_cells], s->n_cells, normalized_configs[conf]);
    }

    for (int conf = 0; conf < 8; conf++) {
        bool is_duplicate = false;
        for (int u = 0; u < n_unique; u++) {
            if (configs_equal(normalized_configs[conf], normalized_configs[unique_indices[u]], s->n_cells)) {
                is_duplicate = true;
                break;
            }
        }
        if (!is_duplicate) {
            unique_indices[n_unique++] = conf;
        }
    }

    Coordinates new_cells[MAX_CONFIGS * MAX_CELLS];
    for (int u = 0; u < n_unique; u++) {
        int old_conf = unique_indices[u];
        for (int i = 0; i < s->n_cells; i++) {
            new_cells[u * s->n_cells + i] = s->cells[old_conf * s->n_cells + i];
        }
    }

    for (int i = 0; i < n_unique * s->n_cells; i++) {
        s->cells[i] = new_cells[i];
    }

    s->n_configs = n_unique;

    printf("Shape has %d unique configurations\n", n_unique);
}

void finalise_shapes(Shape shapes[], int n_shapes) {
    for (int i = 0; i < n_shapes; i++) {
        shapes[i].height = 3;
        shapes[i].width = 3;
        shapes[i].n_configs = 0;

        for (int conf_type = 0; conf_type < 8; conf_type++) {
            for (int j = 0; j < shapes[i].n_cells; j++) {
                int target_cell = j + (conf_type * shapes[i].n_cells);
                shapes[i].cells[target_cell] = translate(shapes[i].cells[j], conf_type);
            }
        }

        shapes[i].n_configs = 8;

        remove_duplicate_configs(&shapes[i]);

        print_shape(shapes[i], 0);
    }
}

int main() {
    FILE *fptr = fopen("inputs/day12.txt", "r");

    int solution = 0;

    char line[MAX_LEN_LINE];
    int shape_row = 0;

    Shape shapes[MAX_SHAPES];
    int n_shapes = 0;

    bool read_regions = false;
    while (fgets(line, sizeof(line), fptr)) {

        for (int i = 0; line[i] != '\0'; i++) {
            if (!read_regions) {
                if (line[i] == 'x') {
                    finalise_shapes(shapes, n_shapes);
                    read_regions = true;
                    solution += process_region(line, shapes, n_shapes);
                    break;
                } else if (line[i] == ':') {
                    n_shapes++;
                    shape_row = 0;
                    shapes[n_shapes-1].n_cells = 0;
                    break;
                } else if (line[i] == '\n') {
                    shape_row++;
                    break;
                } else if (line[i] == '#')
                    shape_add_coordinates(&shapes[n_shapes-1], shape_row, i);

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
