#include "tree_model.h"

bool get_next_line(FILE *fp, char *type, int *iarg, double *darg) {
    if (!fp) {
        return false;
    }
    char a[2];
    double b[2];
    int num = fscanf(fp, "%s %lf %lf", a, b, b+1);
    if (a[1])
        return false;
    bool root = (a[0] == 'a') && (num == 2);
    bool not_leaf = (a[0] == 'b') && (num == 3);
    bool leaf = (a[0] == 'c') && (num == 2);
    if (root || not_leaf)
        *iarg = (int)b[0];
    if (not_leaf)
        *darg = b[1];
    if (leaf)
        *darg = b[0];
    *type = a[0];
    return root || not_leaf || leaf;
}