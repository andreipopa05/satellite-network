/*POPA Andrei-Eduard*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct satellite {
    char *name;
    int frequency;
    struct satellite *left, *right;
    struct satellite *first_child, *next_sibling;
    struct satellite *parent;
} TSatellite, *TNetwork;

typedef int (*TFCmp)(TSatellite*, TSatellite*);

typedef struct Min_Heap {
    int nrMax, nrElem;
    TSatellite **info;
    TFCmp comp;
} THeap;

// Functions
int Rel_Min_Heap(TSatellite *a, TSatellite *b);
THeap *Build_Network(int nrMax, TFCmp comp);
TSatellite *Create_Bond_sat(TSatellite *a, TSatellite *b);
void InsertHeap(THeap *h, TSatellite *val);
TSatellite *Create_Satellite(int freq, char id[16], TSatellite *left,
                             TSatellite *right);
TSatellite *ExtrHeap(THeap *h);
void Build_Tree(THeap *h);
void Print_Tree(TSatellite *root, FILE *fout);
void Distr_Tree(TSatellite *root);
void Decipher(char codes[15][1001], TSatellite *sat_vect, int sat_nr,
              int codes_nr, TSatellite *root, FILE *fout);
void Cipher(char sat_names[256][256], int sat_names_nr, TSatellite *root,
            FILE *fout);
void Solve_errors(char names[256][256], int nr_sat, TSatellite *root,
                  FILE *fout);
TSatellite *Read_Multi_Tree(FILE *file);
void Add_Multi_Tree(TSatellite *root, char *attach_name,
                    TSatellite *multi_root);
int Distance(TSatellite *root, char *name1, char *name2);