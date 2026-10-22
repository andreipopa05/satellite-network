/*POPA Andrei-Eduard*/
#include "functions.h"

int Rel_Min_Heap(TSatellite *a, TSatellite *b) {
    if (a->frequency != b->frequency) {
        return a->frequency < b->frequency;
    }

    // Satellites with the same frequency are ordered by name
    return strcmp(a->name, b->name) < 0;
}

THeap *Build_Network(int nrMax, TFCmp comp) {
    THeap *network = (THeap*) malloc(sizeof(struct Min_Heap));
    if (!network) {
        return NULL;
    }

    // Also allocate memory for the link satellites that will be added
    network->info = (TSatellite**) malloc(2 * nrMax * sizeof(TSatellite*));
    if (!network->info) {
        free(network);
        return NULL;
    }

    network->nrMax = 2 * nrMax;
    network->nrElem = 0;
    network->comp = comp;

    return network;
}

TSatellite *Create_Bond_sat(TSatellite *a, TSatellite *b) {
    TSatellite *s = (TSatellite*) calloc(1, sizeof(TSatellite));
    if (!s) {
        return NULL;
    }

    // The left child is the smaller one
    if (a->frequency > b->frequency || (a->frequency == b->frequency &&
        strcmp(a->name, b->name) > 0)) {
        TSatellite *aux = a;
        a = b;
        b = aux;
    }

    // Attach the children to the link satellite
    s->left = a;
    s->right = b;
    a->parent = s;
    b->parent = s;

    // The sum of the two is the parent's frequency
    s->frequency = a->frequency + b->frequency;

    // The concatenation of the two names is the parent's name
    s->name = (char*) malloc(strlen(a->name) + strlen(b->name) + 1);
    if (!s->name) {
        free(s);
        return NULL;
    }
    strcpy(s->name, a->name);
    strcat(s->name, b->name);

    return s;
}

void InsertHeap(THeap *h, TSatellite *val) {
    if (h->nrElem >= h->nrMax) {
        return;
    }
    h->info[h->nrElem] = val;
    h->nrElem++;

    int poz = h->nrElem - 1;

    while (poz > 0) {
        int father = (poz - 1) / 2;

        if (h->comp(h->info[father], h->info[poz])) {
            break;
        }

        TSatellite *aux = h->info[poz];
        h->info[poz] = h->info[father];
        h->info[father] = aux;

        poz = father;
    }
}

TSatellite *Create_Satellite(int freq, char id[16], TSatellite *left,
                             TSatellite *right) {
    // Allocate memory
    TSatellite *sat = (TSatellite*) calloc(1, sizeof(TSatellite));
    if (!sat) {
        return NULL;
    }

    // Set the name and the frequency
    sat->name = strdup(id);
    if (!sat->name) {
        free(sat);
        return NULL;
    }
    sat->frequency = freq;

    // Set the links
    sat->left = left;
    sat->right = right;

    return sat;
}

TSatellite *ExtrHeap(THeap *h) {
    if (h->nrElem == 0) {
        return NULL;
    }

    TSatellite *val = h->info[0];

    h->nrElem--;
    int poz = h->nrElem;
    h->info[0] = h->info[poz];

    int i = 0;

    while (2 * i + 1 < h->nrElem) {
        int left_son = 2 * i + 1;
        int right_son = 2 * i + 2;
        int saved = left_son;

        if (right_son < h->nrElem &&
            h->comp(h->info[right_son], h->info[left_son])) {
            saved = right_son;
        }

        if (h->comp(h->info[i], h->info[saved])) {
            break;
        }

        TSatellite *aux = h->info[i];
        h->info[i] = h->info[saved];
        h->info[saved] = aux;

        i = saved;
    }

    return val;
}

void Build_Tree(THeap *h) {
    if (!h) {
        return;
    }

    while (h->nrElem > 1) {
        // Extract two satellites at a time from the heap, in order
        TSatellite *son1 = ExtrHeap(h);
        TSatellite *son2 = ExtrHeap(h);

        // Create the link node and add it to the heap
        TSatellite *father = Create_Bond_sat(son1, son2);
        InsertHeap(h, father);
    }
}

static int Count_Tree(TSatellite *root) {
    if (!root) {
        return 0;
    }
    return 1 + Count_Tree(root->left) + Count_Tree(root->right);
}

void Print_Tree(TSatellite *root, FILE *fout) {
    if (!root) {
        return;
    }

    // Queue
    TSatellite **sat_vect = (TSatellite**) malloc(Count_Tree(root) *
                                                  sizeof(TSatellite*));
                                                  if (!sat_vect) {
        return;
    }

    int start = 0;
    int stop = 0;
    sat_vect[stop] = root; // Add the root to the queue
    stop = 1;

    while (start < stop) {  // While there are elements in the queue
        int branch = stop - start; // Number of nodes on the current level

        for (int i = 0; i < branch; i++) {
            TSatellite *sat = sat_vect[start++]; // Current node
            fprintf(fout, "%d-%s", sat->frequency, sat->name); // Print

            if (i < branch - 1) {  // Space unless it is the last node on the level
                fprintf(fout, " ");
            }

            // Add the current node's children to the queue
            if (sat->left) {
                sat_vect[stop++] = sat->left;
            }
            if (sat->right) {
                sat_vect[stop++] = sat->right;
            }
        }

        fprintf(fout, "\n"); // Newline after each level
    }

    free(sat_vect);
}

void Distr_Tree(TSatellite *root) {
    if (!root) {
        return;
    }

    Distr_Tree(root->left);
    Distr_Tree(root->right);
    Distr_Tree(root->first_child);
    Distr_Tree(root->next_sibling);
    free(root->name);
    free(root);
}

void Decipher(char codes[15][1001], TSatellite *sat_vect, int sat_nr,
              int codes_nr, TSatellite *root, FILE *fout) {
    if (!root || !sat_vect) {
        return;
    }

    TSatellite *saved_root = root;

    for (int i = 0; i < codes_nr; i++) {  // Go through the array of codes
        saved_root = root;
        int j = 0;

        // Go through each character
        for (j = 0; j < (int)strlen(codes[i]); j++) {
            if (codes[i][j] == '0') {
                if (saved_root->left) {  // Move left if possible
                    saved_root = saved_root->left;
                } else {  // If it is not possible to move left any more
                    // Check that it is an original satellite, not a link node
                    int check = 0;
                    for (int t = 0; t < sat_nr; t++) {
                        if (strcmp(saved_root->name, sat_vect[t].name) == 0) {
                            check = 1;
                            break;
                        }
                    }
                    if (check == 1) {
                        fprintf(fout, "%s ", saved_root->name);
                    }
                    // Move on to the next satellite's code: restart from the root
                    // and check the character at index j again, because it
                    // belongs to the next code
                    j--;
                    saved_root = root;
                }
            } else if (codes[i][j] == '1') {
                if (saved_root->right) {  // Move right if possible
                    saved_root = saved_root->right;
                } else {  // If it is not possible to move right any more
                    // Check that it is an original satellite, not a link node
                    int check = 0;
                    for (int t = 0; t < sat_nr; t++) {
                        if (strcmp(saved_root->name, sat_vect[t].name) == 0) {
                            check = 1;
                            break;
                        }
                    }
                    if (check == 1) {
                        fprintf(fout, "%s ", saved_root->name);
                    }
                    // Move on to the next satellite's code: restart from the root
                    // and check the character at index j again, because it
                    // belongs to the next code
                    j--;
                    saved_root = root;
                }
            }
        }
        // Print the satellite reached at the end of the code
        int check = 0; // Check that this one is not a link node either
        for (int t = 0; t < sat_nr; t++) {
            if (strcmp(saved_root->name, sat_vect[t].name) == 0) {
                check = 1;
                break;
            }
        }
        if (check == 1) {
            fprintf(fout, "%s ", saved_root->name);
        }
        fprintf(fout, "\n");
    }
}

// Checks whether the leaf with the given name exists under node
static int Has_Satellite(TSatellite *node, const char *name) {
    if (!node) {
        return 0;
    }

    if (!node->left && !node->right) {
        return strcmp(node->name, name) == 0;
    }

    return Has_Satellite(node->left, name) || Has_Satellite(node->right, name);
}

void Cipher(char sat_names[256][256], int sat_names_nr, TSatellite *root,
            FILE *fout) {
    if (!root) {
        return;
    }

    TSatellite *saved_root = root;

    for (int i = 0; i < sat_names_nr; i++) {
        saved_root = root;

        // If the satellite is not in the tree, skip it
        if (!Has_Satellite(root, sat_names[i])) {
            continue;
        }

        // Walk down until reaching the searched satellite
        while (saved_root->left || saved_root->right) {
            if (Has_Satellite(saved_root->left, sat_names[i])) {
                saved_root = saved_root->left;
                fprintf(fout, "0");
            } else {
                saved_root = saved_root->right;
                fprintf(fout, "1");
            }
        }
    }
    fprintf(fout, "\n");
}


// The traversal from task 3, implemented as a function that finds a
// satellite in the tree
TSatellite *Find_Satellite(TSatellite *root, char *name) {
    if (!root || !name) {
        return NULL;
    }

    // The satellite is not in the tree
    if (!Has_Satellite(root, name)) {
        return NULL;
    }

    TSatellite *saved_root = root;

    // Walk down until reaching the searched satellite
    while (saved_root->left || saved_root->right) {
        if (Has_Satellite(saved_root->left, name)) {
            saved_root = saved_root->left;
        } else {
            saved_root = saved_root->right;
        }
    }

    return saved_root;
}

// Check whether the satellite is in root's subtree
int Check_Bond(TSatellite *root, TSatellite *sat) {
    if (!root) {
        return 0;
    }
    if (root == sat) {
        return 1;
    }
    return Check_Bond(root->left, sat) || Check_Bond(root->right, sat);
}

TSatellite *Find_faulty_bond_node(TSatellite *root,
          TSatellite **vec_satellites, int nr_sat) {
    if (!root) {
        return NULL;
    }

    // Check that all the satellites are in the tree rooted at root
    for (int i = 0; i < nr_sat; i++) {
        if (!Check_Bond(root, vec_satellites[i])) {
            return NULL;
        }
    }

    // Check both branches: a lower node may still contain all of them
    TSatellite *left = Find_faulty_bond_node(root->left, vec_satellites,
        nr_sat);
    if (left) {
        return left;
    }

    TSatellite *right = Find_faulty_bond_node(root->right, vec_satellites,
                                                nr_sat);
    if (right) {
        return right;
    }

    return root;
}

void Solve_errors(char names[256][256], int nr_sat, TSatellite *root,
                  FILE *fout) {
    if (!root) {
        return;
    }

    TSatellite **vec_satellites = (TSatellite**)
    malloc((nr_sat + 1) * sizeof(TSatellite*));
    if (!vec_satellites) {
        return;
    }

    TSatellite *saved_root = root;

    for (int i = 0; i < nr_sat; i++) {
        saved_root = root;
        // Add each faulty satellite to the array of satellites
        vec_satellites[i] = Find_Satellite(saved_root, names[i]);
    }

    // Find the lowest link node that has all of them below it
    TSatellite *searched_node = Find_faulty_bond_node(root,
        vec_satellites, nr_sat);

    if (searched_node) {
        fprintf(fout, "%s\n", searched_node->name);
    }

    // Free the memory
    free(vec_satellites);
}

// Finds a node (main tree or multi-way tree) by its name
static TSatellite *Find_Node(TSatellite *root, char *name) {
    if (!root) {
        return NULL;
    }
    if (strcmp(root->name, name) == 0) {
        return root;
    }

    TSatellite *found = Find_Node(root->left, name);
    if (!found) {
        found = Find_Node(root->right, name);
    }
    if (!found) {
        found = Find_Node(root->first_child, name);
    }
    if (!found) {
        found = Find_Node(root->next_sibling, name);
    }
    return found;
}

// Reads a multi-way tree that is given level by level and returns its root
TSatellite *Read_Multi_Tree(FILE *file) {
    int freq = 0;
    int parents_nr = 0;
    char name[16];

    // Root of the tree and the number of nodes that have children
    fscanf(file, "%d %15s %d", &freq, name, &parents_nr);
    TSatellite *root = Create_Satellite(freq, name, NULL, NULL);

    for (int i = 0; i < parents_nr; i++) {
        char parent_name[16];
        int children_nr = 0;
        fscanf(file, "%15s %d", parent_name, &children_nr);

        TSatellite *parent = Find_Node(root, parent_name);
        TSatellite *last_child = NULL;

        for (int j = 0; j < children_nr; j++) {
            fscanf(file, "%d %15s", &freq, name);

            TSatellite *child = Create_Satellite(freq, name, NULL, NULL);
            child->parent = parent;

            // Add the child at the end of the children list
            if (last_child) {
                last_child->next_sibling = child;
            } else {
                parent->first_child = child;
            }
            last_child = child;
        }
    }

    return root;
}

// Links a multi-way tree to the main tree node with the given name
void Add_Multi_Tree(TSatellite *root, char *attach_name,
                    TSatellite *multi_root) {
    TSatellite *node = Find_Node(root, attach_name);
    if (!node) {
        Distr_Tree(multi_root);
        return;
    }

    multi_root->parent = node;
    node->first_child = multi_root;
}

// Number of links between a node and the root of the main tree
static int Depth(TSatellite *node) {
    int depth = 0;
    while (node->parent) {
        node = node->parent;
        depth++;
    }
    return depth;
}

// Distance (number of links) between two nodes, or -1 if one is missing
int Distance(TSatellite *root, char *name1, char *name2) {
    TSatellite *a = Find_Node(root, name1);
    TSatellite *b = Find_Node(root, name2);
    if (!a || !b) {
        return -1;
    }

    int depth_a = Depth(a);
    int depth_b = Depth(b);
    int dist = 0;

    // Climb from the deeper node until both nodes meet
    while (a != b) {
        if (depth_a >= depth_b) {
            a = a->parent;
            depth_a--;
        } else {
            b = b->parent;
            depth_b--;
        }
        dist++;
    }

    return dist;
}
