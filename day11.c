#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_NODES 1000
#define MAX_LINE_LENGTH 500
#define MAX_NAME_LEN 20
#define HASH_SIZE 1024

int debug = 0;

//------------  Nodes and edges ------------

typedef struct Edge {
    struct Node *dest;
    struct Edge *next;
} Edge;

typedef struct Node {
    // Node identity
    int index;
    char name[MAX_NAME_LEN];
    // Graph exploration tracking
    int visited;
    // Linked lists
    struct Node *hash_next; // Nodes in the same hash bucket
    struct Node *topological_next; // Nodes in topological order
    struct Edge *child; // Outgoing edges
} Node;

Node *node_create(int i, const char *name) {
    Node *new_node = malloc(sizeof(Node));
    new_node->index = i;
    strncpy(new_node->name, name, MAX_NAME_LEN-1);
    new_node->name[MAX_NAME_LEN-1] = '\0'; // The line above pads the rest

    new_node->visited = 0;

    new_node->hash_next = NULL;
    new_node->child = NULL;
    new_node->topological_next = NULL;

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

void topological_sort(Node *all_nodes[], int node_count) {
    // Used to track node at the bottom of the stack
    Node *null_node = NULL;
    Node **last_node = &null_node;

    for (int i = 0; i < node_count; i++) {
        if (!all_nodes[i]->visited)
            dfs_visit(all_nodes[i], last_node);
    }

    if (debug) {
        printf("\n\ntopological oder:");
        printf("\n--------\n");
        Node *n = *last_node;
        while (n) {
            printf("%s ", n->name);
            n = n->topological_next;
        }
        printf("\n--------\n\n");
    }
}

void node_propagate_paths(Node *n, int path_counts[]) {
    Edge *e = n->child;
    while (e) {
        path_counts[e->dest->index] += path_counts[n->index];
        e = e->next;
    }
}

int count_paths(Node *from, Node *to, int node_count) {
    int path_counts[node_count];
    for (int i = 0; i < node_count; i++) path_counts[i] = 0;
    path_counts[from->index] = 1;

    Node *n = from;
    // In topological order, have each node propagate the count of paths up to it
    // to its children (in dynamic programming fashion)
    while (n && n != to) {
        node_propagate_paths(n, path_counts);
        n = n->topological_next;
    }
    if (!n) {
        return 0;
    }

    return path_counts[to->index];
}

//------------ Solution ------------

int main(int argc, char *argv[]) {
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "--debug") == 0) {
                debug = 1;
            }
        }
    }
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
    if (debug) {
        for (int i = 0; i < node_count; i++) {
            printf("%s: ", all_nodes[i]->name);
            Edge *next_child = all_nodes[i]->child;
            while (next_child) {
                printf("%s,", next_child->dest->name);
                next_child = next_child->next;
            }
            printf("\n");
        }
    }

    // Create topological ordering (one where for all u->v edges, u comes before v)
    topological_sort(all_nodes, node_count);
    // Resolve all interesting nodes
    Node *you = all_nodes[hash_name_to_i(hash_table, "you", all_nodes, &node_count)];
    Node *out = all_nodes[hash_name_to_i(hash_table, "out", all_nodes, &node_count)];
    Node *svr = all_nodes[hash_name_to_i(hash_table, "svr", all_nodes, &node_count)];
    Node *fft = all_nodes[hash_name_to_i(hash_table, "fft", all_nodes, &node_count)];
    Node *dac = all_nodes[hash_name_to_i(hash_table, "dac", all_nodes, &node_count)];

    // Compute solutions
    printf("Solution 1: %d \n", count_paths(you, out, node_count));

    // Acceptable path 1
    int svr_fft = count_paths(svr, fft, node_count);
    int fft_dac = count_paths(fft, dac, node_count);
    int dac_out = count_paths(dac, out, node_count);
    // Acceptable path 2
    int svr_dac = count_paths(svr, dac, node_count);
    int dac_fft = count_paths(dac, fft, node_count);
    int fft_out = count_paths(fft, out, node_count);

    printf("Solution 2: %ld \n", (long)svr_fft * fft_dac * dac_out + (long)svr_dac * dac_fft * fft_out);

    // Clean up
    for (int i = 0; i < node_count; i++) {
        node_destroy(all_nodes[i]);
    }

    return 0;
}
