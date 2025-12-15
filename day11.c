/* WORK IN PROGRESS */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_NODES 1000
#define MAX_LINE_LENGTH 500
#define MAX_NAME_LEN 20
#define HASH_SIZE 1024

//------------  Nodes and edges ------------

typedef struct Edge {
    struct Node *dest;
    struct Edge *next;
} Edge;

typedef struct Node {
    int index;
    int visited;
    char name[MAX_NAME_LEN];
    struct Node *hash_next; // linked list for nodes in same hash bucket
    struct Node *topological_next; // linked list for topological order
    struct Edge *child; // linked list of outgoing edges
    int ways;
} Node;


Node *node_create(int i, const char *name) {
    Node *new_node = malloc(sizeof(Node));
    new_node->index = i;
    new_node->visited = 0;

    strncpy(new_node->name, name, MAX_NAME_LEN-1);
    new_node->name[MAX_NAME_LEN-1] = '\0'; // The line above pads the rest

    new_node->hash_next = NULL;
    new_node->child = NULL;
    new_node->topological_next = NULL;

    new_node->ways = 0;

    return new_node;
}

void node_destroy(Node *n) {
    Edge *e = n->child;
    while (e) {
        Edge *next = e->next;
        free(e);
        e = next;
    }
    n->child = NULL;

    free(n);
}

void node_add_child(Node *from, Node *to) {
    Edge *e = malloc(sizeof(Edge));
    Edge *e_next = from->child;
    e->dest = to;
    e->next = e_next;
    from->child = e;
}

void node_count_paths(Node *n) {
    Edge *e = n->child;
    while (e) {
        e->dest->ways += n->ways;
        e = e->next;
    }
}

//------------ Hash map ------------

// djb2 hash by Daniel J. Bernstein
unsigned long hash_char(const char *str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; // hash * 33 + c

    return hash % HASH_SIZE;
}

int hash_name_to_i(Node **hash_table, const char *name, Node **all_nodes, int *node_count) {
    unsigned int hash = hash_char(name);

    // Search bucket
    for (Node *n = hash_table[hash]; n; n = n->hash_next) {
        if (strcmp(n->name, name) == 0) {
            return n->index;
        }
    }

    // Create new entry
    Node *new_node = node_create(*node_count, name);
    all_nodes[(*node_count)++] = new_node;
    new_node->hash_next = hash_table[hash]; // The node at the bucket end points to NULL
    hash_table[hash] = new_node;

    return new_node->index;
}

//------------ Graph exploration ------------


void dfs_visit(Node *node, Node **last_node) {
    node->visited = 1;
    // Visit all unvisited children
    Edge *child = node->child;
    while (child) {
        if (!child->dest->visited)
            dfs_visit(child->dest, last_node);
        child = child->next;
    }
    // Add to topological order
    node->topological_next = *last_node;
    *last_node = node;
}

void topological_sort(Node *start) {
    // Used to track node at the bottom of the stack
    Node *null_node = NULL;
    Node **last_node = &null_node;

    dfs_visit(start, last_node);
}


int main() {
    Node *hash_table[HASH_SIZE];
    for (int i = 0; i < HASH_SIZE; i++) {
        hash_table[i] = NULL;
    }

    Node *all_nodes[MAX_NODES];
    int node_count = 0;

    FILE* fptr = fopen("inputs/day11.txt", "r");
    char line[MAX_LINE_LENGTH];
    // Read input; map names to indices; create adjacency lists
    while (fgets(line, sizeof(line), fptr)) {
        char name[MAX_NAME_LEN];
        int name_len = 0;

        int from_i;

        for (int i = 0; line[i] != '\0'; i++) {
            if (line[i] == ':') {
                name[name_len] = '\0';
                name_len = 0;

                from_i = hash_name_to_i(hash_table, name, all_nodes, &node_count);
                i++; // Skip blank space after colon
            } else if (line[i] == ' ' || line[i] == '\n') {
                name[name_len] = '\0';
                name_len = 0;

                int to_i = hash_name_to_i(hash_table, name, all_nodes, &node_count);
                node_add_child(all_nodes[from_i], all_nodes[to_i]);
            } else {
                name[name_len++] = line[i];
            }
        }
    }
    fclose(fptr);

    // Verify construction of nodes
    for (int i = 0; i < node_count; i++) {
        printf("%s: ", all_nodes[i]->name);
        Edge *next_child = all_nodes[i]->child;
        while (next_child) {
            printf("%s,", next_child->dest->name);
            next_child = next_child->next;
        }
        printf("\n");
    }

    Node *n = all_nodes[hash_name_to_i(hash_table, "you", all_nodes, &node_count)];
    topological_sort(n);
    n->ways = 1;
    while (n) {
        node_count_paths(n);
        n = n->topological_next;
    }

    Node *out = all_nodes[hash_name_to_i(hash_table, "out", all_nodes, &node_count)];

    printf("Solution: %d\n", out->ways);

    // Clean up
    for (int i = 0; i < node_count; i++) {
        node_destroy(all_nodes[i]);
    }

    return 0;
}
