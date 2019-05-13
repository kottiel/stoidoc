#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int main() {
    char contents[] = "\"How are you doing\"";
    printf("beforehand: %s\n", contents);

    char newcontents[50];
    sscanf(contents, "%*[^\"]%s", newcontents);

    printf("new contents: %s\n", newcontents);

    return 0;
}