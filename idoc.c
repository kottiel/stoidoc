#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "label.h"
#include <stdlib.h>

#define INITIAL_CAP             3

int main(int argc, char *argv[]) {

    Column_header cols;

    char buffer[MAX_COLUMNS] = {'\0'};

    char **spreadsheet;
    int spreadsheet_cap = INITIAL_CAP;
    int spreadsheet_row = 0;
    bool end_of_spreadsheet = false;

    spreadsheet = (char **)malloc(INITIAL_CAP * sizeof(char *));

    FILE *fp;

    if (argc != 2) {
        printf("usage: ./idoc filename.txt\n");
        return EXIT_FAILURE;
    }

    if ((fp = fopen(argv[1], "r")) == NULL) {
      printf("File not found.\n");
      return EXIT_FAILURE;
    } else {

    while (((fgets(buffer, MAX_COLUMNS, fp) != NULL)) && !end_of_spreadsheet) {
      if (spreadsheet_row >= spreadsheet_cap) {
        spreadsheet_cap *= 2;
        spreadsheet = (char **)realloc(spreadsheet, spreadsheet_cap * sizeof(char *));
      }
      //  find real length of printable buffer and append '\0' to make a string
      int real_length = MAX_COLUMNS - 1;
      char c = buffer[real_length];
      while ((real_length > 0) && !isalnum(c)) {
        real_length--;
        c = buffer[real_length];
      }

      if (real_length == 0)
        end_of_spreadsheet = true;
      else {
        real_length++;
        buffer[real_length] = '\0';
        spreadsheet[spreadsheet_row] = (char *)malloc(real_length * sizeof(char));
        strcpy(spreadsheet[spreadsheet_row], buffer);
        memset(buffer, '\0', MAX_COLUMNS);
        spreadsheet_row++;
      }
    }
  }

  if (process_column_header(spreadsheet[0], &cols) == -1) {
    printf("Program quitting.\n");
    return EXIT_FAILURE;
  }

  for (int i=0; i < spreadsheet_row; i++)
    printf("%s\n", spreadsheet[i]);

  for (int i=0; i < spreadsheet_row; i++)
    free(spreadsheet[i]);

  free(spreadsheet);

  return EXIT_SUCCESS;
}