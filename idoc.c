#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "label.h"
#define   CR '\r'
#define   LF '\n'

char **spreadsheet;
int spreadsheet_cap = 0;
int spreadsheet_row_number = 0;

void read_spreadsheet(FILE *fp) {

    // pre-process spreadsheet - remove all LF and replace all CRLF with LF
    char c;
    char buffer[MAX_COLUMNS];
    bool line_not_empty = false;

    int i = 0;
    while ((c = fgetc(fp)) != EOF) {
        if (c == CR) {
            if ((c = fgetc(fp)) == LF) {
                buffer[i] = '\0';
                if (line_not_empty) {
                    if (spreadsheet_row_number >= spreadsheet_cap) {
                        spreadsheet_expand();
                    }
                    spreadsheet[spreadsheet_row_number] = (char *)malloc(i * sizeof(char) + 2);
                    strcpy(spreadsheet[spreadsheet_row_number], buffer);
                    spreadsheet_row_number++;
                }
                i = 0;
                line_not_empty = false;

            }
        } else if ((c != CR) && (c != LF)) {
            buffer[i++] = c;
            if (c != '\t')
                line_not_empty = true;
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

    if (parse_spreadsheet(spreadsheet[0], labels, &columns) == -1)
    {
        printf("Program quitting.\n");
        return EXIT_FAILURE;
    }

    printf("Processed %d rows in %s\n", spreadsheet_row_number, argv[1]);

    for (int i = 0; i < spreadsheet_row_number; i++)
        free(spreadsheet[i]);
    free(spreadsheet);

    for (int i = 1; i < spreadsheet_row_number; i++)
        free(labels[i].tdline);

    free(labels);

    return EXIT_SUCCESS;
}