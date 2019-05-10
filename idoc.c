#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "label.h"

char **spreadsheet;
int spreadsheet_cap = 0;
int spreadsheet_row_number = 0;

void read_spreadsheet(FILE *fp) {

    char buffer[MAX_COLUMNS] = {'\0'};
    bool end_of_spreadsheet = false;
    int real_length;

    while (((fgets(buffer, MAX_COLUMNS, fp) != NULL)) && !end_of_spreadsheet) {
            if (spreadsheet_row_number >= spreadsheet_cap) {
                spreadsheet_expand();
            }
            //  find real length of printable buffer and append '\0' to make a string
            real_length = MAX_COLUMNS - 1;
            char c = buffer[real_length];
            while ((real_length > 0) && (!isalnum(c))) {
                c = buffer[real_length];
                real_length--;
            }

            if (real_length == 0)
                end_of_spreadsheet = true;
            else {
                real_length++;
                buffer[real_length] = '\0';
                spreadsheet[spreadsheet_row_number] = (char *)malloc(real_length * sizeof(char) + 2);
                strcpy(spreadsheet[spreadsheet_row_number], buffer);
                memset(buffer, '\0', MAX_COLUMNS);
                spreadsheet_row_number++;
            }
    }
}

int main(int argc, char *argv[]) {

    // the Column_header struct that contains all spreadsheet col labels
    Column_header columns;

    // the Label_record array
    Label_record *labels;

    if (spreadsheet_init() != 0) {
        printf("Could not initialize spreadsheet array. Exiting\n");
        return EXIT_FAILURE;
    }

    FILE *fp;

    if (argc != 2) {
        printf("usage: ./idoc filename.txt\n");
        return EXIT_FAILURE;
    }

    if ((fp = fopen(argv[1], "r")) == NULL) {
        printf("File not found.\n");
        return EXIT_FAILURE;
    } else {
        read_spreadsheet(fp);
    }
    fclose(fp);

    labels = (Label_record *)malloc(spreadsheet_row_number * sizeof(Label_record));
    
    if (process_column_headers(spreadsheet[0], labels, &columns) == -1)
    {
        printf("Program quitting.\n");
        return EXIT_FAILURE;
    }

    printf("Processed %d rows in %s\n", spreadsheet_row_number, argv[1]);

    for (int i=0; i < spreadsheet_row_number; i++)
        free(spreadsheet[i]);

    free(spreadsheet);
    free(labels);

    return EXIT_SUCCESS;
}