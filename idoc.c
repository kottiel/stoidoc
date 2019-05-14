#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "label.h"

/* return character for Windows      */
#define CR '\r'

/* end of line new line character    */
#define LF '\n'

/* length of '_IDOC.txt' extension   */
#define FILE_EXT_LEN 10

/* global variable that holds the spreadsheets specific column headings  */
char **spreadsheet;

/* tracks the spreadsheet column headings capacity                       */
int spreadsheet_cap = 0;

/* tracks the actual number of label rows in the spreadsheet             */
int spreadsheet_row_number = 0;

/* global variable to track the idoc sequence number                     */
int sequence_number = 1;

/**
    reads a tab-delimited Excel spreadsheet into memory, dynamically
    allocating memory to hold the rows as needed. All CRLF and LF are
    replaced with null characters to delimit the end of the spreadsheet
    row / string. Rows containing just tab characters are ignored.
    @param fp points to the input file
*/
void read_spreadsheet(FILE *fp) {

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
          spreadsheet[spreadsheet_row_number] =
              (char *)malloc(i * sizeof(char) + 2);
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

/**
    prints a specified number of spaces to a file stream
    @param fpout points to the output file
    @param n is the number of spaces to print
*/
void print_spaces(FILE *fpout, int n) {
  for (int i = 0; i < n; i++)
    fprintf(fpout, " ");
}

/**
    prints the IDoc control record
    @param fpout points to the output file
*/
int print_control_record(FILE *fpout) {

  time_t t = time(NULL);
  struct tm tm = *localtime(&t);

  // line 1
  fprintf(fpout, "EDI_DC40  500000000000");
  // cols 22-29 - 7 digit control number?
  fprintf(fpout, "1234567");
  // BarTender ibtdoc release
  fprintf(fpout, "740");
  fprintf(fpout, " 3012  Z1BTDOC");
  print_spaces(fpout, 53);
  fprintf(fpout, "ZSC_BTEND");
  print_spaces(fpout, 40);
  fprintf(fpout, "SAPMEP    LS  MEPCLNT500");
  print_spaces(fpout, 91);
  fprintf(fpout, "I041      US  BARTENDER");
  print_spaces(fpout, 92);
  fprintf(fpout, "%d%02d%02d%02d%02d%02d", tm.tm_year + 1900, tm.tm_mon + 1,
          tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  print_spaces(fpout, 112);
  fprintf(fpout, "Material_EN");
  print_spaces(fpout, 9);
  fprintf(fpout, "\r\n");

  return 0;
}

/**
    prints the remaining IDoc records based on the number
    of label records.
    @param fpout points to the output file
    @param labels is the array of label records
    @param label_record is the label index we're accessing
*/
void print_label_idoc_records(FILE *fpout, Label_record *labels, int record) {

  // Print a given record

  // Material line
  fprintf(fpout, "Z2BTMH01000");
  print_spaces(fpout, 19);
  fprintf(fpout, "500000000000");
  // cols 22-29 - 7 digit control number?
  fprintf(fpout, "1234567");
  fprintf(fpout, "%06d", sequence_number++);
  fprintf(fpout, "00000002");
  fprintf(fpout, "%-18s", labels[record].material);
  fprintf(fpout, "\r\n");

  // Label record
  fprintf(fpout, "Z2BTLH01000");
  print_spaces(fpout, 19);
  fprintf(fpout, "500000000000");
  // cols 22-29 - 7 digit control number?
  fprintf(fpout, "1234567");
  fprintf(fpout, "%06d", sequence_number++);
  fprintf(fpout, "00000103");
  fprintf(fpout, "%-18s", labels[record].label);
  fprintf(fpout, "\r\n");

  // TDLine - repeat as many times as there are "##"
  /* get the first token */
  char *token = multi_tok(labels[record].tdline, "##");
  int tdline_line_count = 0;

  while (token != NULL) {
    fprintf(fpout, "Z2BTTX01000");
    print_spaces(fpout, 19);
    fprintf(fpout, "500000000000");
    // cols 22-29 - 7 digit control number?
    fprintf(fpout, "1234567");
    fprintf(fpout, "%06d", sequence_number++);
    fprintf(fpout, "00000204GRUNE  ENMATERIAL  ");
    fprintf(fpout, "%s", labels[record].label);
    print_spaces(fpout, 61);
    fprintf(fpout, "%-70s", token);
    if (tdline_line_count == 0)
      fprintf(fpout, "*");
    else
      fprintf(fpout, "/");
    tdline_line_count++;
    fprintf(fpout, "\r\n");
    token = multi_tok(NULL, "##");
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

  FILE *fp, *fpout;

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

  labels =
      (Label_record *)malloc(spreadsheet_row_number * sizeof(Label_record));

  if (parse_spreadsheet(spreadsheet[0], labels, &columns) == -1) {
    printf("Program quitting.\n");
    return EXIT_FAILURE;
  }

  char *outputfile = (char *)malloc(strlen(argv[1]) + FILE_EXT_LEN);

  sscanf(argv[1], "%[^.]%*[txt]", outputfile);
  strcat(outputfile, "_IDOC.txt");
  printf("outputfile name is %s\n", outputfile);

  if ((fpout = fopen(outputfile, "w")) == NULL) {
    printf("Could not open output file %s", outputfile);
  }

  if (print_control_record(fpout) != 0)
    return EXIT_FAILURE;

  print_label_idoc_records(fpout, labels, 1);
  /*     for (int i = 1; i < spreadsheet_row_number; i++) {
          print_label_idoc_records(fpout, labels, i);
      } */

  fclose(fpout);
  free(outputfile);

  for (int i = 0; i < spreadsheet_row_number; i++)
    free(spreadsheet[i]);
  free(spreadsheet);

  for (int i = 1; i < spreadsheet_row_number; i++)
    free(labels[i].tdline);

  free(labels);

  return EXIT_SUCCESS;
}