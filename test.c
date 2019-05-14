#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int main() {
    char token[] = "\"How are you doing\"";


    printf("beforehand: %s\n", token);

    //check for and remove leading...
    char c[2];
    if (strcmp(strncpy(c, token, 1), "\"") == 0)
        memcpy(token, token + 1, strlen(token));

    // ...and trailing quotes
    strncpy(c, token + strlen(token) - 1, 1);
    if (strcmp(c, "\"") == 0) {
        token[strlen(token) - 1] = '\0';
    }
    printf("new contents: %s\n", token);
    return 0;
}