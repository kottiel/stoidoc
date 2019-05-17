#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

int compare(char *s, char *t) {
    return strcmp(s, t);
}
int main() {


    char lookup[][2][40] = {
                        {"apple", "pie"},
                        {"banana", "pudding"},
                        {"cherry", "cobbler"},
                        {"date", "shake"},
                        {"egg", "souffle"},
                        {"fig", "tree"},
                        {"goat", "cheese"},
                        {"ham", "bones"}
                       };

    int start = 0;
    int end   = 8 - 1;
    int middle;
    bool exit = false;
    int i = 0;
    char *haystack = lookup[0][0];
    //char needle[20];
    //scanf("%19s", needle);
    char needle[] = "almond";

    while (!exit) {
        if (middle != (end - start) / 2 + start)
            middle =  (end - start) / 2 + start;
        else
            exit = true;

        printf("middle: %d\n", middle);
        haystack = lookup[middle][0];
        if (strcmp(needle, haystack) == 0) {
            printf("%s-%s\n", haystack, lookup[middle][1]);
            exit = true;
        }
        else if (strcmp(needle, haystack) < 0) {
            end = middle - 1;
        }
        else {
            start = middle + 1;
        }
        i++;
        if (i > 20)
            exit = true;
    }

    if (i > 10)
        printf("i is %d...it didn't work\n", i);


    return 0;
}


