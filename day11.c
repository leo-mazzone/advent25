/* WORK IN PROGRESS */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LINE_LENGTH 500
#define MAX_NAME_LEN 20
#define HASH_SIZE 1024

typedef struct Node {
    char name[MAX_NAME_LEN];
    int index;
    struct Node *next;
} Node;

// Because these are global variables, C initialises all values to 0
Node *hash_table[HASH_SIZE];
int node_count;

// djb2 hash by Daniel J. Bernstein
unsigned long hash_char(const char *str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; // hash * 33 + c

    return hash % HASH_SIZE;
}

int get_or_create_index(const char *name) {
    unsigned int hash = hash_char(name);

    // Search bucket
    for (Node *n = hash_table[hash]; n; n = n->next) {
        if (strcmp(n->name, name) == 0) {
            return n->index;
        }
    }

    // Return new entry
    Node *new_node = malloc(sizeof(Node));
    strncpy(new_node->name, name, MAX_NAME_LEN-1);
    new_node->name[MAX_NAME_LEN - 1] = '\0'; // Line above pads the rest
    new_node->index = node_count++;
    new_node->next = hash_table[hash]; // The node at the bucket end points to NULL
    hash_table[hash] = new_node;
    return new_node->index;
}

int main() {
    FILE* fptr = fopen("inputs/day11s.txt", "r");
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), fptr)) {
        char name[MAX_NAME_LEN];
        int name_len = 0;
        for (int i = 0; line[i] != '\0'; i++) {
            if (line[i] == ':') {
                name[name_len] = '\0';
                int name_i = get_or_create_index(name);
                printf("%s (%d); ", name, name_i);
                name_len = 0;
                i++; // Skip blank space after colon
            } else if (line[i] == ' ' || line[i] == '\n') {
                name[name_len] = '\0';
                int name_i = get_or_create_index(name);
                printf("%s (%d) ", name, name_i);
                name_len = 0;
            } else {
                name[name_len++] = line[i];
            }
        }
        printf("\n");
    }
    fclose(fptr);

    return 0;
}
