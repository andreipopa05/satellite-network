/*POPA Andrei-Eduard*/
#include "functions.h"

int main(int argc, char *argv[]) {
    // Check that there are enough arguments
    if (argc < 4) {
        return 1;
    }

    // Open the input and output files
    FILE *file = fopen(argv[argc - 2], "r");
    if (!file) {
        return 1;
    }
    FILE *fout = fopen(argv[argc - 1], "w");
    if (!fout) {
        return 1;
    }

    // Read the satellites and add them to the heap
    char line[256];
    int sat_nr = 0; // Number of satellites

    fscanf(file, "%d", &sat_nr);

    // Consume the rest of the line with the satellite count
    fgets(line, sizeof(line), file);

    // Allocate memory for the array of initial satellites
    TSatellite *sat_vect = (TSatellite*) malloc(sat_nr * sizeof(TSatellite));
    if (!sat_nr) {
        return 1;
    }

    // Build the heap
    THeap *network = Build_Network(sat_nr, Rel_Min_Heap);
    if (!network) {
        return 1;
    }

    for (int i = 0; i < sat_nr; i++) {
        fgets(line, sizeof(line), file);   // Read the current line
        line[strcspn(line, "\n")] = '\0';  // Remove the newline

        char *input = strtok(line, " ");
        int fr = atoi(input); // Frequency
        input = strtok(NULL, " ");
        char n[16];
        strcpy(n, input); // Name

        // Create a satellite and add it to the heap
        TSatellite *satellite = Create_Satellite(fr, n, NULL, NULL);
        sat_vect[i] = *satellite; // Add it to the satellite array
        InsertHeap(network, satellite);
    }

    Build_Tree(network); // Build the tree

    if (strcmp(argv[1], "-c1") == 0) {  // Task 1
        TSatellite *root = ExtrHeap(network); // Extract the root
        Print_Tree(root, fout);  // Print level by level
        Distr_Tree(root);  // Free the memory
    }

    if (strcmp(argv[1], "-c2") == 0) {  // Task 2
        TSatellite *root = ExtrHeap(network);

        int codes_nr = 0;

        fscanf(file, "%d", &codes_nr);

        fgets(line, sizeof(line), file);

        char (*codes)[1001] = malloc(codes_nr * sizeof(*codes));
        if (!codes) {
            return 1;
        }

        // Read the codes
        for (int i = 0; i < codes_nr; i++) {
            char code_line[1100]; // A code can have up to 1000 characters
            fgets(code_line, sizeof(code_line), file);
            code_line[strcspn(code_line, "\n")] = '\0';

            strncpy(codes[i], code_line, 1000);
            codes[i][1000] = '\0';
        }

        Decipher(codes, sat_vect, sat_nr, codes_nr, root, fout);

        free(codes);
        Distr_Tree(root);
    }

    if (strcmp(argv[1], "-c3") == 0) {  // Task 3
        TSatellite *root = ExtrHeap(network);

        int sat_names_nr = 0;
        char sat_names[256][256];

        fscanf(file, "%d", &sat_names_nr);

        fgets(line, sizeof(line), file);

        // Read the satellite names
        for (int i = 0; i < sat_names_nr; i++) {
            fgets(line, sizeof(line), file);
            line[strcspn(line, "\n")] = '\0';

            strcpy(sat_names[i], line);
        }

        Cipher(sat_names, sat_names_nr, root, fout);

        Distr_Tree(root);
    }

    if (strcmp(argv[1], "-c4") == 0) {  // Task 4
        TSatellite *root = ExtrHeap(network);

        int nr_sat = 0;
        char names[256][256];

        fscanf(file, "%d", &nr_sat);

        fgets(line, sizeof(line), file);

        // Read the names of the faulty satellites
        for (int i = 0; i < nr_sat; i++) {
            fgets(line, sizeof(line), file);
            line[strcspn(line, "\n")] = '\0';

            strcpy(names[i], line);
        }

        Solve_errors(names, nr_sat, root, fout);

        Distr_Tree(root);
    }

    if (strcmp(argv[1], "-c5") == 0) {  // Task 5
        TSatellite *root = ExtrHeap(network);

        int trees_nr = 0;
        fscanf(file, "%d", &trees_nr);

        // Read the multi-way trees and link them to the main tree
        for (int i = 0; i < trees_nr; i++) {
            char attach_name[256];
            fscanf(file, "%255s", attach_name);

            TSatellite *multi_root = Read_Multi_Tree(file);
            Add_Multi_Tree(root, attach_name, multi_root);
        }

        // The two nodes whose distance is computed
        char name1[256], name2[256];
        fscanf(file, "%255s %255s", name1, name2);

        int dist = Distance(root, name1, name2);
        if (dist >= 0) {
            fprintf(fout, "%d\n", dist);
        }

        Distr_Tree(root);
    }

    // Free the memory and close the files
    free(sat_vect);
    free(network->info);
    free(network);
    fclose(file);
    fclose(fout);
    return 0;
}
