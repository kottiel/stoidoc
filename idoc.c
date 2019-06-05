/**
 *  idoc.c
 */
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

/* length of '_idoc->txt' extension  */
#define FILE_EXT_LEN   10

/* length of GTIN-13                 */
#define GTIN_13        13

/* normal graphics folder path   */
#define GRAPHICS_PATH  "T:\\MEDICAL\\NA\\RTP\\TEAM CENTER\\TEMPLATES\\GRAPHICS\\"

/* global variable that holds the spreadsheets specific column headings  */
char **spreadsheet;

/* tracks the spreadsheet column headings capacity                       */
int spreadsheet_cap = 0;

/* tracks the actual number of label rows in the spreadsheet             */
int spreadsheet_row_number = 0;

/* global variable to track the idoc sequence number                     */
int sequence_number = 1;

// Alphabetized SAP Characteristic Value Lookup
char lookup[][2][LRG] = {
        {"CE",          "CE Mark"},
        {"CE0120",      "CE_0120_Below"},
        {"CE0123",      "CE123"},
        {"CE0050",      "CE0050"},
        {"HEMO_AUTO_L", "HemoAutoL"},
        {"HEMO_L",      "HemolokL"},
        {"HEMO_ML",     "HemolokML"},
        {"HMOCLPTRD",   "HmoclpTrd"},
        {"NO",          "blank-01"},
        {"N",           "blank-01"},
        {"STERILEEO",   "Sterile_EO"},
        {"WECK_LOGO",   "Wecklogo"}
};

int lookupsize = sizeof(lookup) / sizeof(lookup[0]);

// a struct of IDoc sequence numbers
struct control_numbers {
    char ctrl_num[8];
    int matl_seq_number;
    int labl_seq_number;
    int tdline_seq_number;
    int char_seq_number;
};

typedef struct control_numbers Ctrl;

/**
    Returns true (non-zero) if character-string parameter represents
    a signed or unsigned floating-point number. Otherwise returns
    false (zero).
 */
int isNumeric(char *str) {

    if (str == NULL || str[0] == '\0')
        return 0;
    int i = 0;
    while (str[i] != '\0')
        if (isdigit(str[i]) == 0)
            return 0;
        else
            i++;
    return 1;
}

/**
    determine the check digit of a GTIN-13 format value
 */
int checkDigit(long long *lp) {

    long long gtin = *lp;
    gtin = gtin / 10;
    short digit;
    int sum = 0;

    while (gtin > 0) {
        digit = (short) (gtin % 10);
        sum += 3 * digit;
        gtin /= 10;
        digit = (short) (gtin % 10);
        sum += 1 * digit;
        gtin /= 10;
    }

    return (sum % 10 == 0 ? 0 : ((((sum / 10) * 10) + 10) - sum));
}

/**
    perform a binary search on the lookup array to find the SAP
    characteristic definition given the characteristic value
*/
char *sap_lookup(char *needle) {

    int start = 0;
    int end = lookupsize - 1;
    int middle = 0;

    bool exit = false;
    char *haystack; // = lookup[0][0];

    while (!exit) {
        if (middle != (end - start) / 2 + start)
            middle = (end - start) / 2 + start;
        else
            exit = true;
        haystack = lookup[middle][0];
        if (strcmp(needle, haystack) == 0) {
            return lookup[middle][1];
        } else if (strcmp(needle, haystack) < 0) {
            end = middle - 1;
        } else {
            start = middle + 1;
        }
    }
    return NULL;
}

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

    while ((c = (char) fgetc(fp)) != EOF) {
        if (c == LF) {
            buffer[i] = '\0';
            if (line_not_empty) {
                if (spreadsheet_row_number >= spreadsheet_cap) {
                    spreadsheet_expand();
                }
                spreadsheet[spreadsheet_row_number] =
                        (char *) malloc(i * sizeof(char) + 2);
                strcpy(spreadsheet[spreadsheet_row_number], buffer);
                spreadsheet_row_number++;
            }
            i = 0;
            line_not_empty = false;

        } else {
            buffer[i++] = c;
            if ((c != '\t') && (c != '\r') && (c != '\n'))
                line_not_empty = true;
        }
    }
}

bool equals_yes(char *field) {
    return ((strcasecmp(field, "Y") == 0) || (strcasecmp(field, "Yes") == 0));
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
    prints a portion of an idoc field record based on the passed parameter
    @param fpout points to the output file
    @param graphic is the name of the graphic to append to the path and to print
*/
void print_graphic_path(FILE *fpout, char *graphic) {
    fprintf(fpout, "%s", GRAPHICS_PATH);
    fprintf(fpout, "%s", graphic);
    int n = 255 - ((int) strlen(GRAPHICS_PATH) + (int) strlen(graphic));
    for (int i = 0; i < n; i++)
        fprintf(fpout, " ");
}

/**
    prints a specified number of spaces to a file stream
    @param fpout points to the output file
    @param n is the number of spaces to print
*/
void print_Z2BTLC01000(FILE *fpout, char *ctrl_num, int char_seq_number) {
    fprintf(fpout, "Z2BTLC01000");
    print_spaces(fpout, 19);
    fprintf(fpout, "500000000000");
    // cols 22-29 - 7 digit control number?
    fprintf(fpout, "%s", ctrl_num);
    fprintf(fpout, "%06d", sequence_number++);
    fprintf(fpout, "%06d", char_seq_number);
    fprintf(fpout, CHAR_REC);
}

/**
    print a passed column-field that contains a "N" or "NO," (case insensitive),
    or a value requiring SAP lookup and substitution, or a value that translates
    into a graphic name with a .tif suffix.
 */
void print_graphic_column_header(FILE *fpout, char *col_name, char *col_contents, Ctrl *idoc) {

    char graphic_val[3] = {'\0'};
    char cell_contents[MED];
    strncpy(cell_contents, col_contents, MED - 1);

    strncpy(graphic_val, col_contents, 2);

    // don't print anything if the cell was blank
    if (strlen(graphic_val) > 0) {

        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        fprintf(fpout, "%-30s", col_name);
        fprintf(fpout, "%-30s", col_contents);

        // some values in the column are populated, so if there's a 'N' print blank-01.tif
        if ((strcasecmp(graphic_val, "N") == 0) || (strcasecmp(graphic_val, "No") == 0)) {
            //fprintf(fpout, "%-30s", graphic_val);
            print_graphic_path(fpout, "blank-01.tif");
        } else {
            // graphic_name will be converted to its SAP lookup value from
            // the static lookup array
            char *gnp = sap_lookup(cell_contents);
            if (gnp) {
                char graphic_name[MED];
                strcpy(graphic_name, gnp);
                print_graphic_path(fpout, strcat(graphic_name, ".tif"));
            } else {
                print_graphic_path(fpout, strcat(cell_contents, ".tif"));
            }
        }
        fprintf(fpout, "\n");
    }
}

/**
    print a passed column-field that contains a "N" or "NO," (case insensitive),
    or a value requiring SAP lookup and substitution, or a value that translates
    into a graphic name with a .tif suffix.
 */
void print_boolean_record(FILE *fpout, char *col_name, bool col_contents, char *graphic_name, Ctrl *idoc) {

    print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
    fprintf(fpout, "%-30s", col_name);

    if (col_contents) {
        fprintf(fpout, "%-30s", "Y");
        print_graphic_path(fpout, graphic_name);
    } else {
        fprintf(fpout, "%-30s", "N");
        print_graphic_path(fpout, "blank-01.tif");
    }
    fprintf(fpout, "\n");
}

/**
    prints the IDoc control record
    @param fpout points to the output file
*/
int print_control_record(FILE *fpout, Ctrl *idoc) {

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    // line 1
    fprintf(fpout, "EDI_DC40  500000000000");
    // cols 22-29 - 7 digit control number?
    fprintf(fpout, "%s", idoc->ctrl_num);
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
    fprintf(fpout, "\n");

    return 0;
}

/**
    prints the remaining IDoc records based on the number
    of label records.
    @param fpout points to the output file
    @param labels is the array of label records
    @param cols is the column header struct that contains all of the column names
    @param record is the record number being processed
    @param idoc is a Ctrl structure containing sequence numbers
    @return true if a label_idoc_record was printed successfully
*/
int print_label_idoc_records(FILE *fpout, Label_record *labels, Column_header *cols, int record, Ctrl *idoc) {

    // Print the records for a given IDOC (labels[record])

    // the name of the graphic to be appended to the graphic path
    char graphic_name[MED];

    // temporary variables to examine field contents
    char graphic_val[MED];
    char *gnp;

    char prev_material[MED];

    // MATERIAL record (optional)
    // (this is skipped if the previous material record is the same)
    if ((cols->material) && (strlen(labels[record].material) > 0)) {

        // check whether it's a new material
        if (strcmp(prev_material, labels[record].material) != 0) {

            // new material record
            fprintf(fpout, "Z2BTMH01000");
            print_spaces(fpout, 19);
            fprintf(fpout, "500000000000");
            // cols 22-29 - 7 digit control number?
            fprintf(fpout, "%s", idoc->ctrl_num);
            fprintf(fpout, "%06d", sequence_number);
            // every NEW material number carries over the sequence_number
            idoc->matl_seq_number = sequence_number - 1;
            idoc->labl_seq_number = sequence_number;
            fprintf(fpout, "%06d", idoc->matl_seq_number);
            sequence_number++;

            fprintf(fpout, MATERIAL_REC);
            fprintf(fpout, "%-18s", labels[record].material);
            fprintf(fpout, "\n");
            strcpy(prev_material, labels[record].material);
        }
    }
    // LABEL record (required). If the contents of .label are not "LBL", program aborts.
    {
        char graphic_val[4] = {""};
        strncpy(graphic_val, labels[record].label, 3);
        if (strcmp(graphic_val, "LBL") != 0)
            return 0;
        else {
            fprintf(fpout, "Z2BTLH01000");
            print_spaces(fpout, 19);
            fprintf(fpout, "500000000000");
            // cols 22-29 - 7 digit control number?
            fprintf(fpout, "%s", idoc->ctrl_num);
            fprintf(fpout, "%06d", sequence_number);
            fprintf(fpout, "%06d", idoc->labl_seq_number);
            idoc->tdline_seq_number = sequence_number;
            idoc->char_seq_number = sequence_number;
            sequence_number++;
            fprintf(fpout, LABEL_REC);
            fprintf(fpout, "%-18s", labels[record].label);
            fprintf(fpout, "\n");
        }
    }
    // TDLINE record(s) (optional) - repeat as many times as there are "##"

    if (cols->tdline) {
        //* get the first token *//*
        int tdline_count = 0;

        char *token = labels[record].tdline;
        while (strlen(token) > 0) {
            fprintf(fpout, "Z2BTTX01000");
            print_spaces(fpout, 19);
            fprintf(fpout, "500000000000");
            // cols 22-29 - 7 digit control number?
            fprintf(fpout, "%s", idoc->ctrl_num);
            fprintf(fpout, "%06d", sequence_number++);
            fprintf(fpout, "%06d", idoc->tdline_seq_number);
            fprintf(fpout, TDLINE_REC);
            fprintf(fpout, "GRUNE  ENMATERIAL  ");
            fprintf(fpout, "%s", labels[record].label);
            print_spaces(fpout, 61);

            //check for and remove any leading...
            if (token[0] == '\"')
                memmove(token, token + 1, strlen(token));

            // ...and/or trailing quotes
            if (token[strlen(token) - 1] == '\"')
                token[strlen(token) - 1] = '\0';

            // and convert instances of double quotes to single quotes
            char *a = strstr(token, "\"\"");
            if (a != NULL) {
                memmove(a, a + 1, strlen(a));
                token[strlen(token)] = '\0';
            }

            char *dpos = strstr(token, "##");

            if (dpos != NULL) {
                *dpos = '\0';
                fprintf(fpout, "%s", token);
                fprintf(fpout, "##");
                print_spaces(fpout, 70 - (int) (strlen(token) - 2));

                // get the next segment of label record, after the "##"
                token = dpos + strlen("##");
            } else {
                fprintf(fpout, "%-70s", token);
                token[0] = '\0';
            }
            if (tdline_count == 0)
                fprintf(fpout, "*");
            else
                fprintf(fpout, "/");
            tdline_count++;
            fprintf(fpout, "\n");
        }
    }

    // TEMPLATENUMBER record (required)
    if (cols->templatenumber) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        fprintf(fpout, "%-30s", "TEMPLATENUMBER");
        fprintf(fpout, "%-30s", labels[record].template);
        fprintf(fpout, "%-255s", labels[record].template);
        fprintf(fpout, "\n");
    } else {
        printf("Missing template number in record %d. Aborting.\n", record);
        return 0;
    }

    // REVISION record (optional)
    if ((cols->revision) && (strlen(labels[record].revision) > 0)) {
        int match = 0;
        int rev = 0;
        if ((match = sscanf(labels[record].revision, "R%d", &rev) == 1) && rev >= 0 && rev <= 99) {
            print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
            fprintf(fpout, "%-30s", "REVISION");
            fprintf(fpout, "%-30s", labels[record].revision);
            fprintf(fpout, "%-255s", labels[record].revision);
            fprintf(fpout, "\n");
        } else
            printf("Invalid revision value \"%s\" in record %d. REVISION record skipped.\n",
                   labels[record].revision, record);
    }

    // SIZE record (optional)
    strncpy(graphic_val, labels[record].size, 1);

    if ((cols->size) && (strlen(graphic_val) > 0) && (strcasecmp(graphic_val, "N") != 0)) {
        char *token = labels[record].size;

        //check for and remove any leading...
        if (token[0] == '\"')
            memmove(token, token + 1, strlen(token));

        // ...and/or trailing quotes
        if (token[strlen(token) - 1] == '\"')
            token[strlen(token) - 1] = '\0';

        // and convert all instances of double quotes to single quotes
        char *a;
        int diff;
        while ((a = strstr(token, "\"\"")) != NULL) {
            diff = (int) (a - token);
            memmove(token + diff, token + diff + 1, strlen(token) - 2);
        }

        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        fprintf(fpout, "%-30s", "SIZE");
        fprintf(fpout, "%-30s", labels[record].size);
        fprintf(fpout, "%-255s", labels[record].size);
        fprintf(fpout, "\n");
    }

    // LEVEL record (optional)
    strncpy(graphic_val, labels[record].level, 1);
    if ((cols->level) && (strlen(graphic_val) > 0) && (strcasecmp(graphic_val, "N") != 0)) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        fprintf(fpout, "%-30s", "LEVEL");
        fprintf(fpout, "%-30s", labels[record].level);
        fprintf(fpout, "%-255s", labels[record].level);
        fprintf(fpout, "\n");
    }

    // QUANTITY record (optional)
    strncpy(graphic_val, labels[record].quantity, 1);
    if ((cols->quantity) && (strlen(graphic_val) > 0) && (strcasecmp(graphic_val, "N") != 0)) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        fprintf(fpout, "%-30s", "QUANTITY");
        fprintf(fpout, "%-30s", labels[record].quantity);
        fprintf(fpout, "%-255s", labels[record].quantity);
        fprintf(fpout, "\n");
    }

    // BARCODETEXT record (optional)
    {
        char graphic_val[2] = {""};
        strncpy(graphic_val, labels[record].gtin, 1);
        if ((cols->barcodetext) && (strlen(labels[record].gtin) > 0) && (strcasecmp(graphic_val, "N") != 0)) {
            int match = 0;
            char *endptr;
            if (isNumeric(labels[record].gtin)) {
                long long gtin = strtoll(labels[record].gtin, &endptr, 10);
                if (((strlen(labels[record].gtin) == 14) && (gtin % 10 == checkDigit(&gtin))) ||
                    (strlen(labels[record].gtin) == 13)) {
                    print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
                    fprintf(fpout, "%-30s", "BARCODETEXT");
                    fprintf(fpout, "%-30s", labels[record].gtin);
                    fprintf(fpout, "%-255s", labels[record].gtin);
                    fprintf(fpout, "\n");
                } else
                    printf("Invalid check digit or length \"%s\" in record %d. BARCODETEXT record skipped.\n",
                           labels[record].gtin, record);
            } else
                printf("Nonnumeric GTIN \"%s\" in record %d. BARCODETEXT record skipped.\n",
                       labels[record].gtin, record);
        }
    }
    // LTNUMBER record (optional)
    if ((cols->ltnumber) && (strlen(labels[record].ipn) > 0)) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        fprintf(fpout, "%-30s", "LTNUMBER");
        fprintf(fpout, "%-30s", labels[record].ipn);
        fprintf(fpout, "%-255s", labels[record].ipn);
        fprintf(fpout, "\n");
    }

    //
    // GRAPHIC01 - GRAPHIC08 Fields (optional)
    //

    int g_cnt = 1;
    char g_cnt_str[03];
    char graphic[] = "GRAPHIC0";

    // CAUTION record (optional)
    if (cols->caution && labels[record].caution) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        sprintf(g_cnt_str, "%d", g_cnt++);
        strcpy(graphic, "GRAPHIC0");
        fprintf(fpout, "%-30s", strcat(graphic, g_cnt_str));
        fprintf(fpout, "%-30s", "Y");
        print_graphic_path(fpout, "Caution.tif");
        fprintf(fpout, "\n");
    }

    // ConsultIFU record (optional)
    if (cols->consultifu && labels[record].consultifu) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        sprintf(g_cnt_str, "%d", g_cnt++);
        strcpy(graphic, "GRAPHIC0");
        fprintf(fpout, "%-30s", strcat(graphic, g_cnt_str));
        fprintf(fpout, "%-30s", "Y");
        print_graphic_path(fpout, "ConsultIFU.tif");
        fprintf(fpout, "\n");
    }

    // Containslatex record (optional)
    if (cols->latex && labels[record].latex) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        sprintf(g_cnt_str, "%d", g_cnt++);
        strcpy(graphic, "GRAPHIC0");
        fprintf(fpout, "%-30s", strcat(graphic, g_cnt_str));
        fprintf(fpout, "%-30s", "Y");
        print_graphic_path(fpout, "Latex.tif");
        fprintf(fpout, "\n");
    }

    // DoNotUsePakDam record (optional)
    if (cols->donotusedam && labels[record].donotusedamaged) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        sprintf(g_cnt_str, "%d", g_cnt++);
        strcpy(graphic, "GRAPHIC0");
        fprintf(fpout, "%-30s", strcat(graphic, g_cnt_str));
        fprintf(fpout, "%-30s", "Y");
        print_graphic_path(fpout, "DoNotUsePakDam.tif");
        fprintf(fpout, "\n");
    }

    // Latex free record (optional)
    if (cols->latexfree && labels[record].latexfree) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        sprintf(g_cnt_str, "%d", g_cnt++);
        strcpy(graphic, "GRAPHIC0");
        fprintf(fpout, "%-30s", strcat(graphic, g_cnt_str));
        fprintf(fpout, "%-30s", "Y");
        print_graphic_path(fpout, "Latex Free.tif");
        fprintf(fpout, "\n");
    }

    // Man in box record (optional)
    if (cols->maninbox && labels[record].maninbox) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        sprintf(g_cnt_str, "%d", g_cnt++);
        strcpy(graphic, "GRAPHIC0");
        fprintf(fpout, "%-30s", strcat(graphic, g_cnt_str));
        fprintf(fpout, "%-30s", "Y");
        print_graphic_path(fpout, "ManInBox.tif");
        fprintf(fpout, "\n");
    }

    // DoNotRe-sterilize record (optional)
    if (cols->noresterile && labels[record].noresterilize) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        sprintf(g_cnt_str, "%d", g_cnt++);
        strcpy(graphic, "GRAPHIC0");
        fprintf(fpout, "%-30s", strcat(graphic, g_cnt_str));
        fprintf(fpout, "%-30s", "Y");
        print_graphic_path(fpout, "DoNotRe-sterilize.tif");
        fprintf(fpout, "\n");
    }

    // Non-sterile record (optional)
    if (cols->nonsterile && labels[record].nonsterile) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        sprintf(g_cnt_str, "%d", g_cnt++);
        strcpy(graphic, "GRAPHIC0");
        fprintf(fpout, "%-30s", strcat(graphic, g_cnt_str));
        fprintf(fpout, "%-30s", "Y");
        print_graphic_path(fpout, "Non-sterile.tif");
        fprintf(fpout, "\n");
    }

    // PVC Free record (optional)
    if (cols->pvcfree && labels[record].pvcfree) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        sprintf(g_cnt_str, "%d", g_cnt++);
        strcpy(graphic, "GRAPHIC0");
        fprintf(fpout, "%-30s", strcat(graphic, g_cnt_str));
        fprintf(fpout, "%-30s", "Y");
        print_graphic_path(fpout, "PVC_Free.tif");
        fprintf(fpout, "\n");
    }

    // RESUSABLE record (optional)
    if (cols->reusable && labels[record].reusable) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        sprintf(g_cnt_str, "%d", g_cnt++);
        strcpy(graphic, "GRAPHIC0");
        fprintf(fpout, "%-30s", strcat(graphic, g_cnt_str));
        fprintf(fpout, "%-30s", "Y");
        print_graphic_path(fpout, "REUSABLE.tif");
        fprintf(fpout, "\n");
    }

    // singleuse record (optional)
    if (cols->singleuse && labels[record].singleuseonly) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        sprintf(g_cnt_str, "%d", g_cnt++);
        strcpy(graphic, "GRAPHIC0");
        fprintf(fpout, "%-30s", strcat(graphic, g_cnt_str));
        fprintf(fpout, "%-30s", "Y");
        print_graphic_path(fpout, "SINGLEUSE.tif");
        fprintf(fpout, "\n");
    }

    // SINGLEPATIENTUSE record (optional)
    if (cols->singlepatientuse && labels[record].singlepatientuse) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        sprintf(g_cnt_str, "%d", g_cnt++);
        strcpy(graphic, "GRAPHIC0");
        fprintf(fpout, "%-30s", strcat(graphic, g_cnt_str));
        fprintf(fpout, "%-30s", "Y");
        print_graphic_path(fpout, "SINGLEPATIENUSE.tif");
        fprintf(fpout, "\n");
    }

    // electrosurgicalifu record (optional)
    if (cols->electroifu && labels[record].electroifu) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        sprintf(g_cnt_str, "%d", g_cnt++);
        strcpy(graphic, "GRAPHIC0");
        fprintf(fpout, "%-30s", strcat(graphic, g_cnt_str));
        fprintf(fpout, "%-30s", "Y");
        print_graphic_path(fpout, "ElectroSurIFU.tif");
        fprintf(fpout, "\n");
    }

    // keepdry record (optional)
    if (cols->keepdry && labels[record].keepdry) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        sprintf(g_cnt_str, "%d", g_cnt);
        strcpy(graphic, "GRAPHIC0");
        fprintf(fpout, "%-30s", strcat(graphic, g_cnt_str));
        fprintf(fpout, "%-30s", "Y");
        print_graphic_path(fpout, "KeepDry.tif");
        fprintf(fpout, "\n");
    }

    // ECREP record (optional)
    if (cols->ecrep) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        fprintf(fpout, "%-30s", "ECREP");
        if (labels[record].ecrep) {
            fprintf(fpout, "%-30s", "Y");
            print_graphic_path(fpout, "EC Rep.tif");
        } else {
            fprintf(fpout, "%-30s", "N");
            print_graphic_path(fpout, "blank-01.tif");
        }
        fprintf(fpout, "\n");
    }

    // EXPDATE record (optional)
    if (cols->expdate) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        fprintf(fpout, "%-30s", "EXPDATE");
        if (labels[record].expdate) {
            fprintf(fpout, "%-30s", "Y");
            print_graphic_path(fpout, "Expiration Date.tif");
        } else {
            fprintf(fpout, "%-30s", "N");
            print_graphic_path(fpout, "blank-01.tif");
        }
        fprintf(fpout, "\n");
    }

    // KEEPAWAYHEAT record (optional)
    if (cols->keepawayheat && labels[record].keepawayheat) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        fprintf(fpout, "%-30s", "KEEPAWAYHEAT");
        if (labels[record].keepawayheat) {
            fprintf(fpout, "%-30s", "Y");
            print_graphic_path(fpout, "KEEPAWAYHEAT.tif");
        } else {
            fprintf(fpout, "%-30s", "N");
            print_graphic_path(fpout, "blank-01.tif");
        }
        fprintf(fpout, "\n");
    }

    // LOTGRAPHIC record (optional)
    if (cols->lotgraphic) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        fprintf(fpout, "%-30s", "LOTGRAPHIC");
        if (labels[record].lotgraphic) {
            fprintf(fpout, "%-30s", "Y");
            print_graphic_path(fpout, "Lot.tif");
        } else {
            fprintf(fpout, "%-30s", "N");
            print_graphic_path(fpout, "blank-01.tif");
        }
        fprintf(fpout, "\n");
    }

    // MANUFACTURER record (optional)
    if (cols->manufacturer) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        fprintf(fpout, "%-30s", "MANUFACTURER");
        if (labels[record].manufacturer) {
            fprintf(fpout, "%-30s", "Y");
            print_graphic_path(fpout, "Manufacturer.tif");
        } else {
            fprintf(fpout, "%-30s", "N");
            print_graphic_path(fpout, "blank-01.tif");
        }
        fprintf(fpout, "\n");
    }

    // MFGDATE record (optional)
    if (cols->mfgdate)
        print_boolean_record(fpout, "MFGDATE", labels[record].mfgdate, "DateofManufacture.tif", idoc);

    // PHTDEHP record (optional)
    if (cols->phtdehp && labels[record].phtdehp) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        fprintf(fpout, "%-30s", "PHTDEHP");
        if (labels[record].phtdehp) {
            fprintf(fpout, "%-30s", "Y");
            print_graphic_path(fpout, "PHT-DEHP.tif");
        } else {
            fprintf(fpout, "%-30s", "N");
            print_graphic_path(fpout, "blank-01.tif");
        }
        fprintf(fpout, "\n");
    }

    // PHTBBP record (optional)
    if (cols->phtbbp && labels[record].phtdehp) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        fprintf(fpout, "%-30s", "PHTBBP");
        if (labels[record].phtbbp) {
            fprintf(fpout, "%-30s", "Y");
            print_graphic_path(fpout, "PHT-BBP.tif");
        } else {
            fprintf(fpout, "%-30s", "N");
            print_graphic_path(fpout, "blank-01.tif");
        }
        fprintf(fpout, "\n");
    }

    // PHTDINP record (optional)
    if (cols->phtdinp && labels[record].phtdehp) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        fprintf(fpout, "%-30s", "PHTDINP");
        if (labels[record].phtdinp) {
            fprintf(fpout, "%-30s", "Y");
            print_graphic_path(fpout, "PHT-DINP.tif");
        } else {
            fprintf(fpout, "%-30s", "N");
            print_graphic_path(fpout, "blank-01.tif");
        }
        fprintf(fpout, "\n");
    }

    // REFNUMBER record (optional)
    if (cols->refnumber && labels[record].refnumber) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        fprintf(fpout, "%-30s", "REFNUMBER");
        if (labels[record].refnumber) {
            fprintf(fpout, "%-30s", "Y");
            print_graphic_path(fpout, "REF.tif");
        } else {
            fprintf(fpout, "%-30s", "N");
            print_graphic_path(fpout, "blank-01.tif");
        }
        fprintf(fpout, "\n");
    }

    // REF record (optional)
    if (cols->ref && labels[record].ref) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        fprintf(fpout, "%-30s", "REF");
        if (labels[record].ref) {
            fprintf(fpout, "%-30s", "Y");
            print_graphic_path(fpout, "REF.tif");
        } else {
            fprintf(fpout, "%-30s", "N");
            print_graphic_path(fpout, "blank-01.tif");
        }
        fprintf(fpout, "\n");
    }

    // RXONLY record (optional)
    if (cols->rxonly) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        fprintf(fpout, "%-30s", "RXONLY");
        if (labels[record].rxonly) {
            fprintf(fpout, "%-30s", "Y");
            print_graphic_path(fpout, "RX Only.tif");
        } else {
            fprintf(fpout, "%-30s", "N");
            print_graphic_path(fpout, "blank-01.tif");
        }
        fprintf(fpout, "\n");
    }

    // SERIAL record (optional)
    if (cols->serial && labels[record].serial) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        fprintf(fpout, "%-30s", "SERIAL");
        if (labels[record].serial) {
            fprintf(fpout, "%-30s", "Y");
            print_graphic_path(fpout, "Serial Number.tif");
        } else {
            fprintf(fpout, "%-30s", "N");
            print_graphic_path(fpout, "blank-01.tif");
        }
        fprintf(fpout, "\n");
    }

    // SIZELOGO record (optional)
    if (cols->sizelogo && labels[record].sizelogo) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        fprintf(fpout, "%-30s", "SIZELOGO");
        if (labels[record].sizelogo) {
            fprintf(fpout, "%-30s", "Y");
            print_graphic_path(fpout, "blank-01.tif");
        } else {
            fprintf(fpout, "%-30s", "N");
            print_graphic_path(fpout, "blank-01.tif");
        }
        fprintf(fpout, "\n");
    }

    // TFXLOGO record (Optional: Y / N / blank)
    if (cols->tfxlogo) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        fprintf(fpout, "%-30s", "TFXLOGO");
        if (labels[record].tfxlogo) {
            fprintf(fpout, "%-30s", "Y");
            print_graphic_path(fpout, "TeleflexMedical.tif");
        } else {
            fprintf(fpout, "%-30s", "N");
            print_graphic_path(fpout, "blank-01.tif");
        }
        fprintf(fpout, "\n");
    }

/******************************************************************************/
    // ADDRESS record (Optional: Y / N / value / blank)
    if (cols->address) {
        if (equals_yes(labels[record].address)) {
            print_graphic_column_header(fpout, "ADDRESS", "TFX3LineAdd13i", idoc);
        } else
            print_graphic_column_header(fpout, "ADDRESS", labels[record].address, idoc);
    }

    // CAUTIONSTATE record (optional)
    if (cols->cautionstate)
        print_graphic_column_header(fpout, "CAUTIONSTATE", labels[record].cautionstatement, idoc);

/******************************************************************************/


// CE0120 record (optional)
    if ((cols->ce0120) && (strlen(labels[record].cemark) > 0)) {
        print_Z2BTLC01000(fpout, idoc
                ->ctrl_num, idoc->char_seq_number);
        fprintf(fpout,
                "%-30s", "CE0120");
        fprintf(fpout,
                "%-30s", labels[record].cemark);

// graphic_name will be converted to its SAP lookup value from
// the static lookup array
        gnp = sap_lookup(labels[record].cemark);
        if (gnp) {
            strcpy(graphic_name, gnp
            );
            print_graphic_path(fpout, strcat(graphic_name,
                                             ".tif"));
        } else {
            print_graphic_path(fpout, strcat(labels[record]
                                                     .cemark, ".tif"));
        }
        fprintf(fpout,
                "\n");
    }

// COOSTATE record (optional)
    if ((cols->coostate) && (
            strlen(labels[record]
                           .coostate) > 0)) {
        print_Z2BTLC01000(fpout, idoc
                ->ctrl_num, idoc->char_seq_number);
        fprintf(fpout,
                "%-30s", "COOSTATE");
        fprintf(fpout,
                "%-30s", labels[record].coostate);

// graphic_name will be converted to its SAP lookup value from
// the static lookup array
        gnp = sap_lookup(labels[record].coostate);
        if (gnp) {
            strcpy(graphic_name, gnp
            );
            print_graphic_path(fpout, strcat(graphic_name,
                                             ".tif"));
        } else {
            print_graphic_path(fpout, strcat(labels[record]
                                                     .coostate, ".tif"));
        }
        fprintf(fpout,
                "\n");
    }

// DISTRIBUTEDBY record (optional)
    if ((cols->distby) && (
            strlen(labels[record]
                           .distby) > 0)) {
        print_Z2BTLC01000(fpout, idoc
                ->ctrl_num, idoc->char_seq_number);
        fprintf(fpout,
                "%-30s", "DISTRIBUTEDBY");
        fprintf(fpout,
                "%-30s", labels[record].distby);

// graphic_name will be converted to its SAP lookup value from
// the static lookup array
        gnp = sap_lookup(labels[record].distby);
        if (gnp) {
            strcpy(graphic_name, gnp
            );
            print_graphic_path(fpout, strcat(graphic_name,
                                             ".tif"));
        } else {
            print_graphic_path(fpout, strcat(labels[record]
                                                     .distby, ".tif"));
        }
        fprintf(fpout,
                "\n");
    }

    // ECREPADDRESS record (optional)
    if (cols->ecrepaddress)
        print_graphic_column_header(fpout, "ECREPADDRESS", labels[record].ecrepaddress, idoc);

// FLGRAPHIC record (optional)
    if ((cols->flgraphic) && (
            strlen(labels[record]
                           .flgraphic) > 0)) {
        print_Z2BTLC01000(fpout, idoc
                ->ctrl_num, idoc->char_seq_number);
        fprintf(fpout,
                "%-30s", "FLGRAPHIC");
        fprintf(fpout,
                "%-30s", labels[record].flgraphic);

// graphic_name will be converted to its SAP lookup value from
// graphic_name will be converted to its SAP lookup value from
// the static lookup array
        gnp = sap_lookup(labels[record].flgraphic);
        if (gnp) {
            strcpy(graphic_name, gnp
            );
            print_graphic_path(fpout, strcat(graphic_name,
                                             ".tif"));
        } else {
            print_graphic_path(fpout, strcat(labels[record]
                                                     .flgraphic, ".tif"));
        }
        fprintf(fpout,
                "\n");
    }

// LABELGRAPH1 record (optional)
    if ((cols->labelgraph1) && (
            strlen(labels[record]
                           .labelgraph1) > 0)) {
        print_Z2BTLC01000(fpout, idoc
                ->ctrl_num, idoc->char_seq_number);
        fprintf(fpout,
                "%-30s", "LABELGRAPH1");

        strcpy(graphic_name, labels[record]
                .labelgraph1);
// graphic_name will be converted to its SAP lookup value from
// the static lookup array
        if (graphic_name) {
            if ((
                        strcmp(graphic_name,
                               "N") == 0) || (
                        strcmp(graphic_name,
                               "No") == 0)) {
                fprintf(fpout,
                        "%-30s", "N");
                print_graphic_path(fpout,
                                   "blank-01.tif");
            } else {
                gnp = sap_lookup(labels[record].labelgraph1);
                fprintf(fpout,
                        "%-30s", labels[record].labelgraph1);
                if (gnp) {
                    strcpy(graphic_name, gnp
                    );
                    print_graphic_path(fpout, strcat(graphic_name,
                                                     ".tif"));
                } else {
                    print_graphic_path(fpout, strcat(labels[record]
                                                             .labelgraph1, ".tif"));
                }
            }
        }
        fprintf(fpout,
                "\n");
    }

// LABELGRAPH2 record (optional)
    if ((cols->labelgraph2) && (
            strlen(labels[record]
                           .labelgraph2) > 0)) {
        print_Z2BTLC01000(fpout, idoc
                ->ctrl_num, idoc->char_seq_number);
        fprintf(fpout,
                "%-30s", "LABELGRAPH2");

        strcpy(graphic_name, labels[record]
                .labelgraph2);
// graphic_name will be converted to its SAP lookup value from
// the static lookup array
        if (graphic_name) {
            if ((
                        strcmp(graphic_name,
                               "N") == 0) || (
                        strcmp(graphic_name,
                               "No") == 0)) {
                fprintf(fpout,
                        "%-30s", "N");
                print_graphic_path(fpout,
                                   "blank-01.tif");
            } else {
                gnp = sap_lookup(labels[record].labelgraph2);
                fprintf(fpout,
                        "%-30s", labels[record].labelgraph2);
                if (gnp) {
                    strcpy(graphic_name, gnp
                    );
                    print_graphic_path(fpout, strcat(graphic_name,
                                                     ".tif"));
                } else {
                    print_graphic_path(fpout, strcat(labels[record]
                                                             .labelgraph2, ".tif"));
                }
            }
        }
        fprintf(fpout,
                "\n");
    }

/*// LATEXSTATEMENT record (optional)
    if (cols->latexstate)
        print_graphic_column_header(fpout,
                                    "LATEXSTATEMENT", labels[record].latexstatement, idoc);
*/
    // LOGO1 record (optional)
    if (cols->logo1)
        print_graphic_column_header(fpout, "LOGO1", labels[record].logo1, idoc);

// LOGO2 record (optional)
    if ((cols->logo2) && (
            strlen(labels[record]
                           .logo2) > 0)) {
        print_Z2BTLC01000(fpout, idoc
                ->ctrl_num, idoc->char_seq_number);
        fprintf(fpout,
                "%-30s", "LOGO2");
        fprintf(fpout,
                "%-30s", labels[record].logo2);
// graphic_name will be converted to its SAP lookup value from
// the static lookup array
        gnp = sap_lookup(labels[record].logo2);
        if (gnp) {
            strcpy(graphic_name, gnp
            );
            print_graphic_path(fpout, strcat(graphic_name,
                                             ".tif"));
        } else {
            print_graphic_path(fpout, strcat(labels[record]
                                                     .logo2, ".tif"));
        }
        fprintf(fpout,
                "\n");
    }

// LOGO3 record (optional)
    if ((cols->logo3) && (
            strlen(labels[record]
                           .logo3) > 0)) {
        print_Z2BTLC01000(fpout, idoc
                ->ctrl_num, idoc->char_seq_number);
        fprintf(fpout,
                "%-30s", "LOGO3");
        fprintf(fpout,
                "%-30s", labels[record].logo3);
// graphic_name will be converted to its SAP lookup value from
// the static lookup array
        gnp = sap_lookup(labels[record].logo3);
        if (gnp) {
            strcpy(graphic_name, gnp
            );
            print_graphic_path(fpout, strcat(graphic_name,
                                             ".tif"));
        } else {
            print_graphic_path(fpout, strcat(labels[record]
                                                     .logo3, ".tif"));
        }
        fprintf(fpout,
                "\n");
    }

// LOGO4 record (optional)
    if ((cols->logo4) && (
            strlen(labels[record]
                           .logo4) > 0)) {
        print_Z2BTLC01000(fpout, idoc
                ->ctrl_num, idoc->char_seq_number);
        fprintf(fpout,
                "%-30s", "LOGO4");
        fprintf(fpout,
                "%-30s", labels[record].logo4);
// graphic_name will be converted to its SAP lookup value from
// the static lookup array
        gnp = sap_lookup(labels[record].logo4);
        if (gnp) {
            strcpy(graphic_name, gnp
            );
            print_graphic_path(fpout, strcat(graphic_name,
                                             ".tif"));
        } else {
            print_graphic_path(fpout, strcat(labels[record]
                                                     .logo4, ".tif"));
        }
        fprintf(fpout,
                "\n");
    }

// LOGO5 record (optional)
    if ((cols->logo5) && (
            strlen(labels[record]
                           .logo5) > 0)) {
        print_Z2BTLC01000(fpout, idoc
                ->ctrl_num, idoc->char_seq_number);
        fprintf(fpout,
                "%-30s", "LOGO5");
        fprintf(fpout,
                "%-30s", labels[record].logo5);
// graphic_name will be converted to its SAP lookup value from
// the static lookup array
        gnp = sap_lookup(labels[record].logo5);
        if (gnp) {
            strcpy(graphic_name, gnp
            );
            print_graphic_path(fpout, strcat(graphic_name,
                                             ".tif"));
        } else {
            print_graphic_path(fpout, strcat(labels[record]
                                                     .logo5, ".tif"));
        }
        fprintf(fpout,
                "\n");
    }

    // MDR1 record (optional)
    if (cols->mdr1)
        print_graphic_column_header(fpout, "MDR1", labels[record].mdr1, idoc);

// MANUFACTUREDBY record (optional)
    if ((cols->manufacturedby) && (
            strlen(labels[record]
                           .manufacturedby) > 0)) {
        print_Z2BTLC01000(fpout, idoc
                ->ctrl_num, idoc->char_seq_number);
        fprintf(fpout,
                "%-30s", "MANUFACTUREDBY");
        fprintf(fpout,
                "%-30s", labels[record].manufacturedby);
// graphic_name will be converted to its SAP lookup value from
// the static lookup array
        gnp = sap_lookup(labels[record].manufacturedby);
        if (gnp) {
            strcpy(graphic_name, gnp
            );
            print_graphic_path(fpout, strcat(graphic_name,
                                             ".tif"));
        } else {
            print_graphic_path(fpout, strcat(labels[record]
                                                     .manufacturedby, ".tif"));
        }
        fprintf(fpout,
                "\n");
    }

// PATENTSTA record (optional)
    if ((cols->patentstatement) && (
            strlen(labels[record]
                           .patentstatement) > 0)) {
        print_Z2BTLC01000(fpout, idoc
                ->ctrl_num, idoc->char_seq_number);
        fprintf(fpout,
                "%-30s", "PATENTSTA");
        fprintf(fpout,
                "%-30s", labels[record].patentstatement);
// graphic_name will be converted to its SAP lookup value from
// the static lookup array
        gnp = sap_lookup(labels[record].patentstatement);
        if (gnp) {
            strcpy(graphic_name, gnp
            );
            print_graphic_path(fpout, strcat(graphic_name,
                                             ".tif"));
        } else {
            print_graphic_path(fpout, strcat(labels[record]
                                                     .patentstatement, ".tif"));
        }
        fprintf(fpout,
                "\n");
    }

// STERILESTA record (optional)
    if ((cols->sterilitystatement) && (
            strlen(labels[record]
                           .sterilitystatement) > 0)) {
        print_Z2BTLC01000(fpout, idoc
                ->ctrl_num, idoc->char_seq_number);
        fprintf(fpout,
                "%-30s", "STERILESTA");
        fprintf(fpout,
                "%-30s", labels[record].sterilitystatement);
// graphic_name will be converted to its SAP lookup value from
// the static lookup array
        gnp = sap_lookup(labels[record].sterilitystatement);
        if (gnp) {
            strcpy(graphic_name, gnp
            );
            print_graphic_path(fpout, strcat(graphic_name,
                                             ".tif"));
        } else {
            print_graphic_path(fpout, strcat(labels[record]
                                                     .sterilitystatement, ".tif"));
        }
        fprintf(fpout,
                "\n");
    }

/*// STERILITYTYPE record (optional)
    if (cols->sterilitytype)
        print_graphic_column_header(fpout,
                                    "STERILITYTYPE", labels[record].sterilitytype, idoc);

// BOMLEVEL record (optional)
    if (cols->bomlevel)
        print_graphic_column_header(fpout,
                                    "BOMLEVEL", labels[record].bomlevel, idoc);*/

// VERSION record (optional)
    if ((cols->version) && (
            strlen(labels[record]
                           .version) > 0)) {
        print_Z2BTLC01000(fpout, idoc
                ->ctrl_num, idoc->char_seq_number);
        fprintf(fpout,
                "%-30s", "VERSION");
        fprintf(fpout,
                "%-30s", labels[record].version);

// graphic_name will be converted to its SAP lookup value from
// the static lookup array
        gnp = sap_lookup(labels[record].version);
        if (gnp) {
            strcpy(graphic_name, gnp
            );
            print_graphic_path(fpout, strcat(graphic_name,
                                             ".tif"));
        } else {
            print_graphic_path(fpout, strcat(labels[record]
                                                     .version, ".tif"));
        }
        fprintf(fpout,
                "\n");
    }

// INSERTGRAPHIC record (optional)
    if ((cols->insertgraphic) && (
            strlen(labels[record]
                           .insertgraphic) > 0)) {
        print_Z2BTLC01000(fpout, idoc
                ->ctrl_num, idoc->char_seq_number);
        fprintf(fpout,
                "%-30s", "INSERTGRAPHIC");
        fprintf(fpout,
                "%-30s", labels[record].insertgraphic);

// graphic_name will be converted to its SAP lookup value from
// the static lookup array
        gnp = sap_lookup(labels[record].insertgraphic);
        if (gnp) {
            strcpy(graphic_name, gnp
            );
            print_graphic_path(fpout, strcat(graphic_name,
                                             ".tif"));
        } else {
            print_graphic_path(fpout,
                               "blank-01.tif");
        }
        fprintf(fpout,
                "\n");
    }
    return 1;
}

int main(int argc, char *argv[]) {

    // the Column_header struct that contains all spreadsheet col labels
    Column_header columns = {0};

    // the Label_record array
    Label_record *labels;

    typedef struct control_numbers Ctrl;

    Ctrl idoc = {"2541435", 0, 1, 0, 0};

    if (spreadsheet_init() != 0) {
        printf("Could not initialize spreadsheet array. Exiting\n");
        return EXIT_FAILURE;
    }

    FILE *fp, *fpout;

    if ((argc != 2) && (argc != 4)) {
        printf("usage: ./idoc filename.txt -sn 7-digit-sn\n");
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
            (Label_record *) malloc(spreadsheet_row_number * sizeof(Label_record));

    if (parse_spreadsheet(spreadsheet[0], labels, &columns) == -1) {
        printf("Program quitting.\n");
        return EXIT_FAILURE;
    }

    // the labels array must be sorted by label number

    //sort_labels(labels);

    char *outputfile = (char *) malloc(strlen(argv[1]) + FILE_EXT_LEN);

    sscanf(argv[1], "%[^.]%*[txt]", outputfile);
    strcat(outputfile, "_IDoc.txt");
    printf("outputfile name is %s\n", outputfile);

    // check for optional control_number from command line
    //char ctrl_num[8] = "2541435";
    if (argc > 2) {
        if ((argv[2] != NULL) && (strcmp(argv[2], "-cn") == 0)) {
            strcpy(idoc.ctrl_num, argv[3]);
        }
    }

    if ((fpout = fopen(outputfile, "w")) == NULL) {
        printf("Could not open output file %s", outputfile);
    }

    if (print_control_record(fpout, &idoc) != 0)
        return EXIT_FAILURE;

    int i = 1;
    while (i < spreadsheet_row_number) {
        if ((print_label_idoc_records(fpout, labels, &columns, i, &idoc)))
            i++;
        else {
            printf("Content error in text-delimited spreadsheet, line %d. Aborting.\n", i);
            return EXIT_FAILURE;
        }
    }

    fclose(fpout);
    free(outputfile);

/*     for (int i = 0; i < spreadsheet_row_number; i++)
        free(spreadsheet[i]); */
    free(spreadsheet);

    /* for (int i = 1; i < spreadsheet_row_number; i++)
        free(labels[i].tdline); */

    free(labels);

    return EXIT_SUCCESS;
}