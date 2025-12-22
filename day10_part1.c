#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX_LINE_LEN 500
#define MAX_LIGHTS 20
#define MAX_BUTTONS 20
#define MAX_LIGHTS_PER_BUTTON 9
#define WIRING_STRIDE (MAX_LIGHTS_PER_BUTTON + 1)
#define QUEUE_SIZE 1000000

typedef unsigned long long State;

typedef struct {
    State state;
    int num_presses;
} QueueNode;

// Here I was experimenting with global variables for no good reason
static int g_n_lights;
static int g_n_buttons;
// g_wiring[i*WIRING_STRIDE] = count of lights this button toggles
// g_wiring[i*WIRING_STRIDE + 1] through g_wiring[i*WIRING_STRIDE + count] = the light indices
static int g_lights[MAX_LIGHTS];
static int g_wiring[MAX_BUTTONS * WIRING_STRIDE];

static inline int wiring_index(int button, int offset) {
    return button * WIRING_STRIDE + offset;
}

State lights_to_state(int lights[], int n) {
    State s = 0;
    for (int i = 0; i < n; i++) {
        if (lights[i]) s |= (1ULL << i);
    }
    return s;
}

State apply_button(State state, int button_idx) {
    int count = g_wiring[wiring_index(button_idx, 0)];
    for (int i = 0; i < count; i++) {
        int light_idx = g_wiring[wiring_index(button_idx, i + 1)];
        state ^= (1ULL << light_idx);
    }
    return state;
}

int solution() {
    State initial = 0;
    State target = lights_to_state(g_lights, g_n_lights);

    if (initial == target) return 0;

    QueueNode *queue = malloc(QUEUE_SIZE * sizeof(QueueNode));
    bool *visited = calloc(1 << g_n_lights, sizeof(bool));

    int front = 0, back = 0;

    queue[back].state = initial;
    queue[back].num_presses = 0;
    visited[initial] = true;
    back++;

    while (front < back) {
        QueueNode current = queue[front++];

        for (int button = 0; button < g_n_buttons; button++) {
            State new_state = apply_button(current.state, button);

            if (new_state == target) {
                int result = current.num_presses + 1;
                free(queue);
                free(visited);
                return result;
            }

            if (!visited[new_state]) {
                visited[new_state] = true;
                queue[back].state = new_state;
                queue[back].num_presses = current.num_presses + 1;
                back++;

                if (back >= QUEUE_SIZE) {
                    fprintf(stderr, "Queue overflow\n");
                    free(queue);
                    free(visited);
                    return -1; // No solution found
                }
            }
        }
    }

    free(queue);
    free(visited);
    return -1;
}

void read_lights(char *line, int *i, int *n_lights, int *lights) {
    (*i)++; // skip '['
    *n_lights = 0;

    while (line[*i] != ']' && line[*i] != '\0') {
        if (line[*i] == '.') {
            lights[*n_lights] = 0;
            (*n_lights)++;
        } else if (line[*i] == '#') {
            lights[*n_lights] = 1;
            (*n_lights)++;
        }
        (*i)++;
    }
}

void read_wirings(char *line, int *i, int *n_buttons, int *wiring) {
    (*i)++; // skip '('

    int button = *n_buttons;
    int count = 0;

    while (line[*i] != ')' && line[*i] != '\0') {
        if (line[*i] >= '0' && line[*i] <= '9') {
            int num = 0;
            while (line[*i] >= '0' && line[*i] <= '9') {
                num = num * 10 + (line[*i] - '0');
                (*i)++;
            }
            wiring[wiring_index(button, count + 1)] = num;
            count++;
        } else {
            (*i)++;
        }
    }

    wiring[wiring_index(button, 0)] = count;
    (*n_buttons)++;
}

void read_joltage(char *line, int *i, int *joltage) {
    (*i)++; // skip '{'
    int idx = 0;

    while (line[*i] != '}' && line[*i] != '\0') {
        if (line[*i] >= '0' && line[*i] <= '9') {
            int num = 0;
            while (line[*i] >= '0' && line[*i] <= '9') {
                num = num * 10 + (line[*i] - '0');
                (*i)++;
            }
            joltage[idx] = num;
            idx++;
        } else {
            (*i)++;
        }
    }
}

int process_line(char *line, int *n_lights, int *n_buttons, int *lights, int *wiring, int *joltage) {
    int i = 0;
    *n_lights = 0;
    *n_buttons = 0;

    char c = line[i];
    while (c != '\0') {
        if (c == '[') read_lights(line, &i, n_lights, lights);
        if (c == '(') read_wirings(line, &i, n_buttons, wiring);
        if (c == '{') read_joltage(line, &i, joltage);
        i++;
        c = line[i];
    }

    printf("\nLights (target): ");
    for (int j = 0; j < *n_lights; j++) {
        printf("%d ", lights[j]);
    }
    printf("\n");

    g_n_lights = *n_lights;
    g_n_buttons = *n_buttons;
    memcpy(g_lights, lights, sizeof(int) * (*n_lights));
    memcpy(g_wiring, wiring, sizeof(int) * (*n_buttons) * WIRING_STRIDE);

    int result = solution();
    printf("Minimum presses: %d\n", result);

    return result;
}

int main() {
    FILE *fptr = fopen("inputs/day10.txt", "r");

    char line[MAX_LINE_LEN];
    int lights[MAX_LIGHTS];
    int wiring[MAX_BUTTONS * WIRING_STRIDE];
    int joltage[MAX_LIGHTS];
    int n_lights = 0;
    int n_buttons = 0;
    int total = 0;

    while (fgets(line, sizeof(line), fptr)) {
        int result = process_line(line, &n_lights, &n_buttons, lights, wiring, joltage);
        if (result > 0) total += result; // Currently ignores -1s
    }

    printf("\nSolution: %d\n", total);

    fclose(fptr);
    return 0;
}
