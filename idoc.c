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

/* end of line new line character    */
#define LF '\n'

/* length of '_idoc->txt' extension  */
#define FILE_EXT_LEN   10

/* length of GTIN-13                 */
#define GTIN_13        13

/* the number of spaces to indent the TDline lines                       */
#define TDLINE_INDENT  61

/* normal graphics folder path                                           */
#define GRAPHICS_PATH  "T:\\MEDICAL\\NA\\RTP\\TEAM CENTER\\TEMPLATES\\GRAPHICS\\"

/* alternate graphics folder path                                        */
#define ALT_GRAPHICS_PATH  "C:\\Users\\jkottiel\\Documents\\1 - Teleflex\\Labeling Resources\\Personal Graphics\\"

/* determine the graphics path at run time                               */
bool alt_path = false;

/* global variable that holds the spreadsheets specific column headings  */
char **spreadsheet;

/* tracks the spreadsheet column headings capacity                       */
int spreadsheet_cap = 0;

/* tracks the actual number of label rows in the spreadsheet             */
int spreadsheet_row_number = 0;

/* global variable to track the idoc sequence number                     */
int sequence_number = 1;

/** Case-INsensitive ALPHABETIZED SAP Characteristic Value Lookup        */
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

/** global variable to maintain size of the SAP lookup array             */
int lookupsize = sizeof(lookup) / sizeof(lookup[0]);

/** a global struct variable of IDoc sequence numbers                    */
struct control_numbers {
    char ctrl_num[8];
    int matl_seq_number;
    int labl_seq_number;
    int tdline_seq_number;
    int char_seq_number;
};

/** defining the struct variable as a new type for convenience           */
typedef struct control_numbers Ctrl;

/**
    Returns true (non-zero) if character-string parameter represents
    a signed or unsigned floating-point number. Otherwise returns
    false (zero).
    @param str is the numeric string value to evaluate
    @return true if str is a number, false otherwise
 */
int isNumeric(const char *str) {

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
    @param lp is the GTIN-13 value to calculate a check digit for
    @return a check digit
 */
int checkDigit(const long long *llp) {

    long long gtin = *llp;
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
    perform a binary search in the lookup array to find the SAP
    characteristic definition given the characteristic value
    @param needle is the search term
    @return the corresponding SAP lookup value, or null if not found
*/
char *sap_lookup(char *needle) {

    int start = 0;
    int end = lookupsize - 1;
    int middle = 0;

    bool exit = false;
    char *haystack;

    while (!exit) {
        if (middle != (end - start) / 2 + start)
            middle = (end - start) / 2 + start;
        else
            exit = true;
        haystack = lookup[middle][0];
        if (strcasecmp(needle, haystack) == 0) {
            return lookup[middle][1];
        } else if (strcasecmp(needle, haystack) < 0) {
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
            if (c != '\t')
                if (c != '\r')
                    if (c != '\n')
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
    prints a portion of an idoc field record based on the passed parameter
    @param fpout points to the output file
    @param graphic is the name of the graphic to append to the path and to print
*/
void print_graphic_path(FILE *fpout, char *graphic) {
    int n = 0;
    if (alt_path) {
        fprintf(fpout, "%s", ALT_GRAPHICS_PATH);
        n = 255 - ((int) strlen(ALT_GRAPHICS_PATH) + (int) strlen(graphic));
    } else {
        fprintf(fpout, "%s", GRAPHICS_PATH);
        n = 255 - ((int) strlen(GRAPHICS_PATH) + (int) strlen(graphic));
    }
    fprintf(fpout, "%s", graphic);

    for (int i = 0; i < n; i++)
        fprintf(fpout, " ");
}

/**
    prints a specified number of spaces to a file stream
    @param fpout points to the output file
    @param ctrl_num
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

void print_info_column_header(FILE *fpout, char *col_name, char *col_value, Ctrl *idoc) {
    print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
    fprintf(fpout, "%-30s", col_name);
    fprintf(fpout, "%-30s", col_value);
    fprintf(fpout, "%-255s", col_value);
    fprintf(fpout, "\n");
}

/**
    print a passed column-field that contains a "N" or "NO," (case insensitive),
    or a value requiring SAP lookup and substitution, or a value that translates
    into a graphic name with a .tif suffix.
 */
void print_graphic_column_header(FILE *fpout, char *col_name, char *col_value, Ctrl *idoc) {

    char cell_contents[MED];
    strncpy(cell_contents, col_value, MED - 1);

    print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
    fprintf(fpout, "%-30s", col_name);

    if (strlen(cell_contents) > 0) {

        fprintf(fpout, "%-30s", col_value);

        // graphic_name will be converted to its SAP lookup value from
        // the static lookup array
        char *gnp = sap_lookup(col_value);
        if (gnp) {
            char graphic_name[MED];
            strcpy(graphic_name, gnp);
            print_graphic_path(fpout, strcat(graphic_name, ".tif"));
        } else if ((strncasecmp(col_value, "N", 1) == 0) || strncasecmp(col_value, "NO", 2) == 0) {
            print_graphic_path(fpout, "blank-01.tif");
        } else {
            print_graphic_path(fpout, strcat(cell_contents, ".tif"));
        }
    } else {
        fprintf(fpout, "%-30s", "N");
        print_graphic_path(fpout, "blank-01.tif");
    }
    fprintf(fpout, "\n");
}

/**
    print a passed column-field that is in the special GRAPHICS01 - GRAPHICS14 category and is defined as
    boolean in the Label_record. It contains a "Y" or a "N." If "Y," print the hard-coded value associated with
    the graphic and a .tif suffix. Otherwise, print a "blank-01.tif" record.
    @param fpout points to the output file
    @param col_name is the column header
    @param value is the boolean value of the column-field
    @param graphic_name is the graphic to print if the boolean is true
    @param idoc is the struct that tracks the control numbers
 */
void print_graphic0x_record(FILE *fpout, int *g_cnt, char *graphic_name, Ctrl *idoc) {

    char g_cnt_str[03];
    char graphic[] = "GRAPHIC0";
    print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
    sprintf(g_cnt_str, "%d", (*g_cnt)++);
    fprintf(fpout, "%-30s", strcat(graphic, g_cnt_str));
    fprintf(fpout, "%-30s", "Y");
    print_graphic_path(fpout, graphic_name);
    fprintf(fpout, "\n");
}


/**
    print a passed column-field that is defined as boolean in the Label_record. It contains a "Y" or a "N."
    If "Y," print the hard-coded value associated with the graphic and a .tif suffix. Otherwise, print a
    "blank-01.tif" record.
    @param fpout points to the output file
    @param col_name is the column header
    @param value is the boolean value of the column-field
    @param graphic_name is the graphic to print if the boolean is true
    @param idoc is the struct that tracks the control numbers
 */
void print_boolean_record(FILE *fpout, char *col_name, bool value, char *graphic_name, Ctrl *idoc) {

    print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
    fprintf(fpout, "%-30s", col_name);

    if (value) {
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
    char graphic_val_shrt[4] = {'\0'};
    strncpy(graphic_val_shrt, labels[record].label, 3);
    if (strcmp(graphic_val_shrt, "LBL") != 0)
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
            print_spaces(fpout, TDLINE_INDENT);

            //check for and remove any leading...
            if (token[0] == '\"')
                memmove(token, token + 1, (int) strlen(token));

            // ...and/or trailing quotes
            if (token[(int) strlen(token) - 1] == '\"')
                token[(int) strlen(token) - 1] = '\0';

            // and convert instances of double quotes to single quotes
            char *a = strstr(token, "\"\"");
            if (a != NULL) {
                memmove(a, a + 1, (int) strlen(a));
                token[(int) strlen(token)] = '\0';
            }

            char *dpos = strstr(token, "##");

            if (dpos != NULL) {
                *dpos = '\0';
                fprintf(fpout, "%s", token);
                fprintf(fpout, "##");
                print_spaces(fpout, 70 - (int) (strlen(token) - 2));

                // get the next segment of label record, after the "##"
                token = dpos + (int) strlen("##");
            } else {
                fprintf(fpout, "%-74s", token);
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
        print_info_column_header(fpout, "TEMPLATE", labels[record].template, idoc);
    } else {
        printf("Missing template number in record %d. Aborting.\n", record);
        return 0;
    }

    // REVISION record (optional)
    if ((cols->revision) && (strlen(labels[record].revision) > 0)) {

        int rev = 0;
        if ((sscanf(labels[record].revision, "R%d", &rev) == 1) && rev >= 0 && rev <= 99) {
            print_info_column_header(fpout, "REVISION", labels[record].revision, idoc);
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
        print_info_column_header(fpout, "SIZE", labels[record].size, idoc);

    }

    /** LEVEL record (optional) */
    strncpy(graphic_val, labels[record].level, 1);
    if ((cols->level) && (strlen(graphic_val) > 0) && (strcasecmp(graphic_val, "N") != 0)) {
        print_info_column_header(fpout, "LEVEL", labels[record].level, idoc);

    }

    /** QUANTITY record (optional) */
    strncpy(graphic_val, labels[record].quantity, 1);
    if ((cols->quantity) && (strlen(graphic_val) > 0) && (strcasecmp(graphic_val, "N") != 0)) {
        print_info_column_header(fpout, "QUANTITY", labels[record].quantity, idoc);
    }

    /** BARCODETEXT record (optional) */

    char gtin_digit[2] = {'\0'};
    strncpy(gtin_digit, labels[record].gtin, 1);
    if ((cols->barcodetext) && (strlen(labels[record].gtin) > 0) && (strcasecmp(gtin_digit, "N") != 0)) {
            char *endptr;
            if (isNumeric(labels[record].gtin)) {
                long long gtin = strtoll(labels[record].gtin, &endptr, 10);
                if (((strlen(labels[record].gtin) == GTIN_13 + 1) && (gtin % 10 == checkDigit(&gtin))) ||
                    (strlen(labels[record].gtin) == GTIN_13)) {
                    print_info_column_header(fpout, "BARCODETEXT", labels[record].gtin, idoc);
                } else
                    printf("Invalid check digit or length \"%s\" in record %d. BARCODETEXT record skipped.\n",
                           labels[record].gtin, record);
            } else
                printf("Nonnumeric GTIN \"%s\" in record %d. BARCODETEXT record skipped.\n",
                       labels[record].gtin, record);
        }

    // LTNUMBER record (optional)
    if ((cols->ltnumber) && (strlen(labels[record].ipn) > 0)) {
        print_info_column_header(fpout, "LTNUMBER", labels[record].ipn, idoc);
    }

    //
    // GRAPHIC01 - GRAPHIC14 Fields (optional)
    //

    int g_cnt = 1;
    char g_cnt_str[03];
    char graphic[] = "GRAPHIC0";

    // CAUTION record (optional)
    if (cols->caution && labels[record].caution)
        print_graphic0x_record(fpout, &g_cnt, "Caution.tif", idoc);

    // ConsultIFU record (optional)
    if (cols->consultifu && labels[record].consultifu)
        print_graphic0x_record(fpout, &g_cnt, "ConsultIFU.tif", idoc);

    // Containslatex record (optional)
    if (cols->latex && labels[record].latex)
        print_graphic0x_record(fpout, &g_cnt, "Latex.tif", idoc);

    // DoNotUsePakDam record (optional)
    if (cols->donotusedam && labels[record].donotusedamaged)
        print_graphic0x_record(fpout, &g_cnt, "DoNotUsePakDam.tif", idoc);

    // Latex free record (optional)
    if (cols->latexfree && labels[record].latexfree)
        print_graphic0x_record(fpout, &g_cnt, "Latex Free.tif", idoc);

    // Man in box record (optional)
    if (cols->maninbox && labels[record].maninbox)
        print_graphic0x_record(fpout, &g_cnt, "ManInBox.tif", idoc);

    // DoNotRe-sterilize record (optional)
    if (cols->noresterile && labels[record].noresterilize)
        print_graphic0x_record(fpout, &g_cnt, "DoNotRe-sterilize.tif", idoc);

    // Non-sterile record (optional)
    if (cols->nonsterile && labels[record].nonsterile)
        print_graphic0x_record(fpout, &g_cnt, "Non-sterile.tif", idoc);

    // PVC Free record (optional)
    if (cols->pvcfree && labels[record].pvcfree)
        print_graphic0x_record(fpout, &g_cnt, "PVC_Free.tif", idoc);

    // RESUSABLE record (optional)
    if (cols->reusable && labels[record].reusable)
        print_graphic0x_record(fpout, &g_cnt, "REUSABLE.tif", idoc);

    // singleuse record (optional)
    if (cols->singleuse && labels[record].singleuseonly)
        print_graphic0x_record(fpout, &g_cnt, "SINGLEUSE.tif", idoc);

    // SINGLEPATIENTUSE record (optional)
    if (cols->singlepatientuse && labels[record].singlepatientuse)
        print_graphic0x_record(fpout, &g_cnt, "SINGLEPATIENUSE.tif", idoc);

    // electrosurgicalifu record (optional)
    if (cols->electroifu && labels[record].electroifu)
        print_graphic0x_record(fpout, &g_cnt, "ElectroSurIFU.tif", idoc);

    // keepdry record (optional)
    if (cols->keepdry && labels[record].keepdry)
        print_graphic0x_record(fpout, &g_cnt, "KeepDry.tif", idoc);

    //
    // END of GRAPHIC01 - GRAPHIC14 Fields (optional)
    //

    /** ECREP record (optional: N / value) */
    if (cols->ecrep)
        print_boolean_record(fpout, "ECREP", labels[record].ecrep, "EC Rep.tif", idoc);

    /** EXPDATE record (optional) */
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

/******************************************************************************/
    // KEEPAWAYHEAT record (optional)
    if (cols->keepawayheat)
        print_boolean_record(fpout, "KEEPAWAYHEAT", labels[record].keepawayheat, "KeepAwayHeat.tif", idoc);

/******************************************************************************/
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

/******************************************************************************/
    // MFGDATE record (optional)
    if (cols->mfgdate)
        print_boolean_record(fpout, "MFGDATE", labels[record].mfgdate, "DateofManufacture.tif", idoc);

/******************************************************************************/
    // PHTDEHP record (optional)
    if (cols->phtdehp)
        print_boolean_record(fpout, "PHTDEHP", labels[record].phtdehp, "PHT-DEHP.tif", idoc);

/******************************************************************************/
    // PHTBBP record (optional)
    if (cols->phtbbp)
        print_boolean_record(fpout, "PHTBBP", labels[record].phtbbp, "PHT-BBP.tif", idoc);

/******************************************************************************/
    // PHTDINP record (optional)
    if (cols->phtdinp)
        print_boolean_record(fpout, "PHTDINP", labels[record].phtdinp, "PHT-DINP.tif", idoc);

/******************************************************************************/
    // REFNUMBER record (optional)
    if (cols->refnumber)
        print_boolean_record(fpout, "REFNUMBER", labels[record].refnumber, "REF.tif", idoc);

/******************************************************************************/
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
/******************************************************************************/
    // SERIAL record (optional)
    if (cols->serial)
        print_boolean_record(fpout, "SERIAL", labels[record].serial, "Serial Number.tif", idoc);
/******************************************************************************/

    // SIZELOGO record (optional)
    /*if (cols->sizelogo) {
        print_boolean_record(fpout, "SIZELOGO", labels[record].sizelogo, "Size Logo.tif", idoc);*/

/******************************************************************************/
    // TFXLOGO record (Optional: Y / N / blank)
    if (cols->tfxlogo)
        print_boolean_record(fpout, "TFXLOGO", labels[record].tfxlogo, "TeleflexMedical.tif", idoc);

/******************************************************************************/
    // ADDRESS record (Optional: Y / N / value / blank)
    if (cols->address)
        if (equals_yes(labels[record].address))
            print_boolean_record(fpout, "ADDRESS", true, "TFX3LineAdd13i.tif", idoc);
        else if (equals_no(labels[record].address))
            print_boolean_record(fpout, "ADDRESS", false, "", idoc);
        else
            print_graphic_column_header(fpout, "ADDRESS", labels[record].address, idoc);

/******************************************************************************/
    // CAUTIONSTATE record (optional: N / value)
    if (cols->cautionstate)
        print_graphic_column_header(fpout, "CAUTIONSTATE", labels[record].cautionstatement, idoc);

/******************************************************************************/

    // CE0120 record (optional)
    if ((cols->ce0120) && (strlen(labels[record].cemark) > 0)) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        fprintf(fpout, "%-30s", "CE0120");
        fprintf(fpout, "%-30s", labels[record].cemark);

        // graphic_name will be converted to its SAP lookup value from
        // the static lookup array
        gnp = sap_lookup(labels[record].cemark);
        if (gnp) {
            strcpy(graphic_name, gnp);
            print_graphic_path(fpout, strcat(graphic_name, ".tif"));
        } else {
            print_graphic_path(fpout, strcat(labels[record].cemark, ".tif"));
        }
        fprintf(fpout, "\n");
    }

    // COOSTATE record (optional)
    if ((cols->coostate) && (strlen(labels[record].coostate) > 0)) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        fprintf(fpout, "%-30s", "COOSTATE");
        fprintf(fpout, "%-30s", labels[record].coostate);

        // graphic_name will be converted to its SAP lookup value from
        // the static lookup array
        gnp = sap_lookup(labels[record].coostate);
        if (gnp) {
            strcpy(graphic_name, gnp);
            print_graphic_path(fpout, strcat(graphic_name, ".tif"));
        } else {
            print_graphic_path(fpout, strcat(labels[record].coostate, ".tif"));
        }
        fprintf(fpout, "\n");
    }

    // DISTRIBUTEDBY record (optional)
    if ((cols->distby) && (strlen(labels[record].distby) > 0)) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        fprintf(fpout, "%-30s", "DISTRIBUTEDBY");
        fprintf(fpout, "%-30s", labels[record].distby);

        // graphic_name will be converted to its SAP lookup value from
        // the static lookup array
        gnp = sap_lookup(labels[record].distby);
        if (gnp) {
            strcpy(graphic_name, gnp);
            print_graphic_path(fpout, strcat(graphic_name, ".tif"));
        } else {
            print_graphic_path(fpout, strcat(labels[record].distby, ".tif"));
        }
        fprintf(fpout, "\n");
    }

    // ECREPADDRESS record (optional)
    if (cols->ecrepaddress)
        print_graphic_column_header(fpout, "ECREPADDRESS", labels[record].ecrepaddress, idoc);

    // FLGRAPHIC record (optional)
    if ((cols->flgraphic) && (strlen(labels[record].flgraphic) > 0)) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        fprintf(fpout, "%-30s", "FLGRAPHIC");
        fprintf(fpout, "%-30s", labels[record].flgraphic);

        // graphic_name will be converted to its SAP lookup value from
        // the static lookup array
        gnp = sap_lookup(labels[record].flgraphic);
        if (gnp) {
            strcpy(graphic_name, gnp);
            print_graphic_path(fpout, strcat(graphic_name, ".tif"));
        } else {
            print_graphic_path(fpout, strcat(labels[record].flgraphic, ".tif"));
        }
        fprintf(fpout, "\n");
    }

    // LABELGRAPH1 record (optional)
    if ((cols->labelgraph1) && (strlen(labels[record].labelgraph1) > 0)) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        fprintf(fpout, "%-30s", "LABELGRAPH1");

        strcpy(graphic_name, labels[record].labelgraph1);

        // graphic_name will be converted to its SAP lookup value from
        // the static lookup array
        if (graphic_name) {
            if ((strcmp(graphic_name, "N") == 0) || (strcmp(graphic_name, "No") == 0)) {
                fprintf(fpout, "%-30s", "N");
                print_graphic_path(fpout, "blank-01.tif");
            } else {
                gnp = sap_lookup(labels[record].labelgraph1);
                fprintf(fpout, "%-30s", labels[record].labelgraph1);
                if (gnp) {
                    strcpy(graphic_name, gnp);
                    print_graphic_path(fpout, strcat(graphic_name, ".tif"));
                } else {
                    print_graphic_path(fpout, strcat(labels[record].labelgraph1, ".tif"));
                }
            }
        }
        fprintf(fpout, "\n");
    }

    // LABELGRAPH2 record (optional)
    if ((cols->labelgraph2) && (strlen(labels[record].labelgraph2) > 0)) {
        print_Z2BTLC01000(fpout, idoc->ctrl_num, idoc->char_seq_number);
        fprintf(fpout, "%-30s", "LABELGRAPH2");

        strcpy(graphic_name, labels[record].labelgraph2);
        // graphic_name will be converted to its SAP lookup value from
        // the static lookup array
        if (graphic_name) {
            if ((strcmp(graphic_name, "N") == 0) || (strcmp(graphic_name, "No") == 0)) {
                fprintf(fpout, "%-30s", "N");
                print_graphic_path(fpout, "blank-01.tif");
            } else {
                gnp = sap_lookup(labels[record].labelgraph2);
                fprintf(fpout, "%-30s", labels[record].labelgraph2);
                if (gnp) {
                    strcpy(graphic_name, gnp);
                    print_graphic_path(fpout, strcat(graphic_name, ".tif"));
                } else {
                    print_graphic_path(fpout, strcat(labels[record].labelgraph2, ".tif"));
                }
            }
        }
        fprintf(fpout, "\n");
    }
/******************************************************************************/
    // LATEXSTATEMENT record (optional: N / value)
    if (cols->latexstate)
        print_graphic_column_header(fpout, "LATEXSTATEMENT", labels[record].latexstatement, idoc);

/******************************************************************************/
    // LOGO1 record (optional)
    if (cols->logo1)
        print_graphic_column_header(fpout, "LOGO1", labels[record].logo1, idoc);

/******************************************************************************/
    // LOGO2 record (optional)
    if (cols->logo2)
        print_graphic_column_header(fpout, "LOGO2", labels[record].logo2, idoc);

/******************************************************************************/

    // LOGO3 record (optional)
    if (cols->logo3)
        print_graphic_column_header(fpout, "LOGO3", labels[record].logo3, idoc);

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
/******************************************************************************/
    // MDR1 record (optional)
    if (cols->mdr1)
        print_graphic_column_header(fpout, "MDR1", labels[record].mdr1, idoc);

/******************************************************************************/
    // MDR2 record (optional)
    if (cols->mdr2)
        print_graphic_column_header(fpout, "MDR2", labels[record].mdr2, idoc);

/******************************************************************************/
// MANUFACTUREDBY record (optional)
    if (cols->manufacturedby)
        print_graphic_column_header(fpout, "MANUFACTUREDBY", labels[record].manufacturedby, idoc);

/******************************************************************************/

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
/******************************************************************************/
    // STERILITYTYPE record (optional)
    if (cols->sterilitytype)
        print_graphic_column_header(fpout, "STERILITYTYPE", labels[record].sterilitytype, idoc);

/******************************************************************************/
// BOMLEVEL record (optional)
    if (cols->bomlevel)
        print_graphic_column_header(fpout, "BOMLEVEL", labels[record].bomlevel, idoc);
/******************************************************************************/

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

    if ((argc != 2) && (argc != 3)) {
        printf("usage: ./idoc filename.txt -J\n");
        return EXIT_FAILURE;
    }

    if ((fp = fopen(argv[1], "r")) == NULL) {
        printf("File not found.\n");
        return EXIT_FAILURE;
    } else {
        read_spreadsheet(fp);
    }
    fclose(fp);

    labels = (Label_record *) malloc(spreadsheet_row_number * sizeof(Label_record));

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

    // check for optional command line parameter '-J'
    if (argc > 2) {
        if ((argv[2] != NULL) && (strcmp(argv[2], "-J") == 0)) {
            alt_path = true;
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