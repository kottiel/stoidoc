#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int compare(char *s, char *t) {
    return strcmp(s, t);
}
int main() {


    char CE[][2][40] = {
                        {"CE", "CE Mark"},
                        {"CE0120", "CE_0120_Below"},
                        {"CE0123", "CE123"},
                        {"CE0050", "CE0050"}
                       };

    for (unsigned int i = 0; i < sizeof(CE)/sizeof(CE[0]); i++) {
        if (strcmp(CE[i][0], "CE0123") == 0)
            printf("found at position %d\n", i);
    }


    return 0;
}


