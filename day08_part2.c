#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_LINE_LEN 100
#define MAX_POINTS 2000

typedef struct {
    int x, y, z;
} Point;

long square(int a) {
    return (long)a * (long)a;
}

long distance_sq(Point p1, Point p2) {
    return square(p1.x - p2.x) + square(p1.y - p2.y) + square(p1.z - p2.z);
}

typedef struct {
    int i, j;
    long distance;
} Pair;

int pair_cmp(const void *a, const void *b) {
    const Pair *pa = a;
    const Pair *pb = b;
    if (pa->distance < pb->distance) return -1;
    if (pa->distance > pb->distance) return 1;
    return 0;
}

// --- Disjoint set (for connected components) ---
// Classic implementation with heuristics, as illustrated by Comen, Leiserson, Rivest & Stein
// (https://en.wikipedia.org/wiki/Introduction_to_Algorithms)
// Here modified to determine whether two points are already connected
// This lets us decrease the number of points to connect, and thus determine when they're all connected
// It's essentially doing Kruskal's algorithm: we first sort weights (distances in this case), then
// iterate over distances and expand connected components as we go.

typedef struct {
    size_t *parent;
    int *rank;
    size_t size;
} DisjointSet;

DisjointSet *ds_create(int size) {
    DisjointSet *ds = malloc(sizeof(DisjointSet));
    if (!ds) return NULL;

    ds->parent = malloc(size * sizeof(size_t));
    if (!ds->parent) {
        free(ds);
        return NULL;
    }

    ds->rank = calloc(size, sizeof(int));
    if (!ds->rank) {
        free(ds->parent);
        free(ds);
        return NULL;
    }

    for (int i = 0; i < size; i++) {
        ds->parent[i] = i;
    }
    ds->size = size;

    return ds;
}

void ds_destroy(DisjointSet *ds) {
    if (!ds) return;
    free(ds->parent);
    free(ds->rank);
    free(ds);
}

size_t ds_find(DisjointSet *ds, size_t x) {
    if (ds->parent[x] != x) {
        ds->parent[x] = ds_find(ds, ds->parent[x]);
    }
    return ds->parent[x];
}

void ds_link(DisjointSet *ds, size_t x, size_t y) {
    if (ds->rank[x] > ds->rank[y]) {
        ds->parent[y] = x;
    } else {
        ds->parent[x] = y;
        if (ds->rank[x] == ds->rank[y]) ds->rank[y]++;
    }
}

// Returns 1 if union actually merged two different components, 0 otherwise
// Two points are already in the same component if their root is the same
int ds_union(DisjointSet *ds, size_t x, size_t y) {
    size_t root_x = ds_find(ds, x);
    size_t root_y = ds_find(ds, y);
    if (root_x == root_y) return 0;
    ds_link(ds, root_x, root_y);
    return 1;
}

int main() {
    int n_points = 0;
    Point points[MAX_POINTS];

    // Parse coordinates
    char line[MAX_LINE_LEN];
    FILE *fptr = fopen("inputs/day8.txt", "r");

    while (fgets(line, sizeof(line), fptr)) {
        Point p;
        sscanf(line, "%d,%d,%d", &p.x, &p.y, &p.z);
        points[n_points++] = p;
    }
    fclose(fptr);

    int n_pairs = n_points * (n_points - 1) / 2;
    Pair *pairs = malloc(n_pairs * sizeof(Pair));

    int pair_idx = 0;
    for (int i=0; i<n_points; i++) {
        for (int j=i+1; j<n_points; j++) {
            Point p1 = points[i];
            Point p2 = points[j];
            pairs[pair_idx++] = (Pair){i, j, distance_sq(p1, p2)};
        }
    }

    qsort(pairs, pair_idx, sizeof(Pair), pair_cmp);

    DisjointSet *djs = ds_create(n_points);


    int n_components = n_points;

    for (int i=0; i<pair_idx; i++) {
        Pair p = pairs[i];

        if (ds_union(djs, p.i, p.j)) {
            // Successfully merged two different components
            n_components--;

            if (n_components == 1) {
                // This edge connected the last two components
                printf("%ld\n", (long)points[p.i].x * (long)points[p.j].x);
                break;
            }
        }
    }

    free(pairs);
    ds_destroy(djs);

    return 0;
}
