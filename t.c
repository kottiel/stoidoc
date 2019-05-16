#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int compare(char *s, char *t) {
    return strcmp(s, t);
}
int main() {


    char a2[][2][40] = {
                        {"CE", "CE Mark"},
                        {"CE0120", "CE_0120_Below"},
                        {"CE0123", "CE123"},
                        {"CE0050", "CE0050"}
                       };

    for (unsigned int i = 0; i < sizeof(a2)/sizeof(a2[0]); i++) {
        printf("comparing: %s with %s\n", a2[i][1], "CE0123");
        if (strcmp(a2[i][0], "CE0123") == 0)
            printf("found at position %d\n", i);
    }


    return 0;
}


