#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int peek_nth_token(int n, const char *buffer, char delimiter)
{
    if (n == 0)
        return 0;

    int i = 0;
    int hit_count = 0;
    int length = strlen(buffer);

    while ((i < length) && (hit_count < n))
    {
        if (buffer[i] == delimiter)
            hit_count++;
        i++;
    }
    if (hit_count == n)
        return i - 1;
    else if (hit_count == (n - 1))
        return length;
    else
        return 0;
}

void process_column_headers(char *buffer)
{
    int count = 6;
    int start = 0;
    int stop = 0;
    char tab_str = '\t';
    char contents[100];

    start = peek_nth_token(count, buffer, tab_str) + 1;
    stop = peek_nth_token(count + 1, buffer, tab_str);
    int length = stop - start;
    strncpy(contents, buffer + start, length);
    contents[length] = '\0';
    printf("*%s*\n", contents);
}

int main() 
{
    char buffer[] = "LBL025681	137081			NONSTERILE		Y";
    process_column_headers(buffer);
    return 0;
}            