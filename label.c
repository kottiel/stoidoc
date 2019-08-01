/**
 *  label.c
 */
#include "label.h"
#include "strl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <limits.h>

/**
    This function initializes the dynamically allocated spreadsheet array.
    @return 0 if successful, -1 if unsuccessful.
*/
int spreadsheet_init() {

    spreadsheet_cap = INITIAL_CAP;

    if ((spreadsheet = (char **) malloc(INITIAL_CAP * sizeof(char *))) == NULL)
        return -1;
    else
        return 0;
}

int spreadsheet_expand() {

    spreadsheet_cap *= 2;
    if ((spreadsheet = (char **) realloc(spreadsheet, spreadsheet_cap * sizeof(char *))) == NULL)
        return -1;
    else
        return 0;
}

char *get_token(char *buffer, char tab_str) {
    char *delimiter;
    int buffer_len = (int) strlen(buffer);
    char *token = (char *) malloc(MAX_COLUMNS * sizeof(char));

    if ((delimiter = strchr(buffer, tab_str)) != NULL) {
        delimiter++;
        int c = 0;
        for (char *i = buffer; i < delimiter - 1; i++) {
            token[c++] = *i;
        }
        token[c] = '\0';
        int delimiter_len = (int) (delimiter - buffer);
        memmove(buffer, delimiter, (size_t) (buffer_len - delimiter_len + 1));
        // buffer_len = strlen(buffer);
    } else if (buffer_len > 0) { // get last token
        int c = 0;
        char *cp = buffer;
        while (*cp) {
            token[c++] = *cp;
            cp++;
        }
        token[c] = '\0';
        buffer[0] = '\0';
        // buffer_len = 0;
    }

    return token;
}

/**
    returns the position of the nth occurrence of a delimiter in a char array
*/
int peek_nth_token(int n, const char *buffer, char delimiter) {

    if (n == 0)
        return 0;

    int i = 0;
    int hit_count = 0;
    int length = (int) strlen(buffer);

    while ((i < length) && (hit_count < n)) {
        if (buffer[i] == delimiter)
            hit_count++;
        i++;
    }
    if (hit_count == n)
        return i - 1;

    return -1;
}

/*

Case-insensitive string compare (strncmp case-insensitive)
- Identical to strncmp except case-insensitive. See: http://www.cplusplus.com/reference/cstring/strncmp/
- Aided/inspired, in part, by: https://stackoverflow.com/a/5820991/4561887

str1    C string 1 to be compared
str2    C string 2 to be compared
num     max number of chars to compare

return:
(essentially identical to strncmp)
INT_MIN  invalid arguments (one or both of the input strings is a NULL pointer)
<0       the first character that does not match has a lower value in str1 than in str2
 0       the contents of both strings are equal
>0       the first character that does not match has a greater value in str1 than in str2

*/
int strncmpci(const char *str1, const char *str2, int num) {
    int ret_code = INT_MIN;

    size_t chars_compared = 0;

    // Check for NULL pointers
    if (!str1 || !str2) {
        goto done;
    }

    // Continue doing case-insensitive comparisons, one-character-at-a-time, of str1 to str2,
    // as long as at least one of the strings still has more characters in it, and we have
    // not yet compared num chars.
    while ((*str1 || *str2) && (chars_compared < num)) {
        ret_code = tolower((int) (*str1)) - tolower((int) (*str2));
        if (ret_code != 0) {
            // The 2 chars just compared don't match
            break;
        }
        chars_compared++;
        str1++;
        str2++;
    }

    done:
    return ret_code;
}

int equals_yes(char *field) {
    return ((strcasecmp(field, "Y") == 0) || (strcasecmp(field, "Yes") == 0));
}

int equals_no(char *field) {
    return ((strcasecmp(field, "N") == 0) || (strcasecmp(field, "NO") == 0));
}

int get_field_contents_from_row(char *contents, int i, int count, char tab_str) {

    int start = 0;
    int stop = 0;

    if (count == 0)
        start = 0;
    else if ((start = peek_nth_token(count, spreadsheet[i], tab_str)) == -1)
        start = (int) strlen(spreadsheet[i]);
    else
        start++;

    if ((stop = peek_nth_token(count + 1, spreadsheet[i], tab_str)) == -1)
        stop = (int) strlen(spreadsheet[i]);

    int length = stop - start;
    strncpy(contents, spreadsheet[i] + start, (size_t) length);
    contents[length] = '\0';
    if (equals_no(contents))
        return 0;
    else
        return length;
}

int duplicate_column_names(const char *cols) {

    char buffer[MAX_COLUMNS];
    unsigned short count = 0;
    char tab_str = TAB;
    char **column_names;

    char min_column[MED], temp[MED];
    bool sorted = true;
    int return_code = 0;

    strcpy(buffer, cols);

    // create an array of column names
    column_names = (char **) malloc(sizeof(char *));
    while (strlen(buffer) > 0) {

        // Keep extracting tokens while the delimiter is present in buffer
        char *token = get_token(buffer, tab_str);
        if (strlen(token)) {
            column_names = (char **) realloc(column_names, sizeof(char *) + count * sizeof(char *));
            column_names[count] = (char *) malloc(sizeof(char *) + sizeof(token) + 1);
            strcpy(column_names[count], token);
            count++;
            free(token);
        }
    }

    // sort the list
    for (int i = 0; i < count; i++) {
        int min_index = i;
        strcpy(min_column, column_names[i]);

        for (int j = i + 1; j < count; j++) {
            if (strcmp(column_names[j], min_column) < 0) {
                strcpy(min_column, column_names[j]);
                min_index = j;
            }
        }

        if (i != min_index) {
            strcpy(temp, column_names[i]);
            strcpy(column_names[i], column_names[min_index]);
            strcpy(column_names[min_index], temp);
            sorted = false;
        }
    }

    for (int i = 0; i < count - 1; i++)
        if (strcmp(column_names[i], column_names[i + 1]) == 0)
            return_code = 1;

    for (int i = 0; i < count; i++)
        free(column_names[i]);

    free(column_names);
    return return_code;
}


int parse_spreadsheet(char *buffer, Label_record *labels, Column_header *cols) {
    unsigned short count = 0;

    char tab_str = TAB;
    char contents[MAX_COLUMNS];

    while (strlen(buffer) > 0) {

        // Keep extracting tokens while the delimiter is present in buffer
        char *token = get_token(buffer, tab_str);

        if (strcmp(token, "LABEL") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                if (get_field_contents_from_row(contents, i, count, tab_str)) {
                    strlcpy(labels[i].label, contents, sizeof(labels[i].label));
                    cols->label = count;
                } else {
                    strlcpy(labels[i].label, "", 1);
                }
            }
        } else if ((strcmp(token, "MATERIAL") == 0) ||
                   (strcmp(token, "PCODE") == 0)) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                if (get_field_contents_from_row(contents, i, count, tab_str)) {
                    strlcpy(labels[i].material, contents, sizeof(labels[i].material));
                    cols->material = count;
                }
            }
            if (strcmp(token, "PCODE") == 0)
                printf("Column \"PCODE\" subsituted for \"MATERIAL\"\n");

        } else if (strcmp(token, "TDLINE") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                labels[i].tdline = (char *) malloc(strlen(contents) + 1);
                strlcpy(labels[i].tdline, contents, sizeof(contents));
                cols->tdline = count;
            }
        } else if (strcmp(token, "ADDRESS") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strlcpy(labels[i].address, contents, sizeof(labels[i].address));
                if (!(equals_no(contents)))
                    cols->address = count;
            }

        } else if (strcmp(token, "BARCODETEXT") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strlcpy(labels[i].barcodetext, contents, sizeof(labels[i].barcodetext));
                if (!(equals_no(contents)))
                    cols->barcodetext = count;
            }

        } else if (strcmp(token, "GTIN") == 0) {
            if (non_SAP_fields) {
                for (int i = 1; i < spreadsheet_row_number; i++) {
                    get_field_contents_from_row(contents, i, count, tab_str);
                    strlcpy(labels[i].gtin, contents, sizeof(labels[i].gtin));
                    if (!(equals_no(contents)))
                        cols->gtin = count;
                }
            } else
                printf("Ignoring column \"%s\"\n", token);


        } else if (strcmp(token, "BOMLEVEL") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strlcpy(labels[i].bomlevel, contents, sizeof(labels[i].bomlevel));
                if (!(equals_no(contents)))
                    cols->bomlevel = count;
            }
/******************************************************************************/
        } else if (strcmp(token, "CAUTION") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strlen(contents)) {
                    if (strcmp("Y", contents) == 0) {
                        labels[i].caution = true;
                        cols->caution = count;
                    } else
                        labels[i].caution = false;
                }
            }
/******************************************************************************/
        } else if (strcmp(token, "CAUTIONSTATE") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strlcpy(labels[i].cautionstatement, contents, sizeof(labels[i].cautionstatement));
                if (!(equals_no(contents)))
                    cols->cautionstate = count;
            }
/******************************************************************************/
        } else if (strcmp(token, "CE0120") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strlcpy(labels[i].cemark, contents, sizeof(labels[i].cemark));
                if (!(equals_no(contents)))
                    cols->ce0120 = count;

            }
        } else if (strcmp(token, "CONSULTIFU") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0) {
                    labels[i].consultifu = true;
                    cols->consultifu = count;
                } else
                    labels[i].consultifu = false;
            }
        } else if (strcmp(token, "CONTAINSLATEX") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0) {
                    labels[i].latex = true;
                    cols->latex = count;
                } else
                    labels[i].latex = false;
            }
        } else if (strcmp(token, "COOSTATE") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strlcpy(labels[i].coostate, contents, sizeof(labels[i].coostate));
                if (!(equals_no(contents)))
                    cols->coostate = count;
            }
        } else if (strcmp(token, "DESCRIPTION") == 0) {
            if (non_SAP_fields)
                for (int i = 1; i < spreadsheet_row_number; i++) {
                    get_field_contents_from_row(contents, i, count, tab_str);
                    strlcpy(labels[i].description, contents, sizeof(labels[i].description));
                    if (!(equals_no(contents)))
                        cols->description = count;
                }
            printf("Ignoring column \"%s\"\n", token);

        } else if (strcmp(token, "DISTRIBUTEDBY") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strlcpy(labels[i].distby, contents, sizeof(labels[i].distby));
                if (!equals_no(contents))
                    cols->distby = count;
            }
        } else if ((strcmp(token, "DONOTUSEDAM") == 0) ||
                   (strcmp(token, "DONOTPAKDAM") == 0)) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0) {
                    labels[i].donotusedamaged = true;
                    cols->donotusedam = count;
                } else
                    labels[i].donotusedamaged = false;
            }
/******************************************************************************/
        } else if (strcmp(token, "ECREP") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strlen(contents) > 0) {
                    if (strcmp("Y", contents) == 0) {
                        labels[i].ecrep = true;
                        cols->ecrep = count;
                    } else
                        labels[i].ecrep = false;
                }
            }
/******************************************************************************/

        } else if (strcmp(token, "ECREPADDRESS") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strlcpy(labels[i].ecrepaddress, contents, sizeof(labels[i].ecrepaddress));
                if (!equals_no(contents))
                    cols->ecrepaddress = count;
            }
        } else if (strcmp(token, "ELECTROSURIFU") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0) {
                    labels[i].electroifu = true;
                    cols->electroifu = count;
                } else
                    labels[i].electroifu = false;
            }
/******************************************************************************/
        } else if (strcmp(token, "EXPDATE") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strlen(contents) > 0) {
                    if (strcmp("Y", contents) == 0) {
                        labels[i].expdate = true;
                        cols->expdate = count;
                    } else
                        labels[i].expdate = false;
                }
            }
/******************************************************************************/
        } else if (strcmp(token, "FLGRAPHIC") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strlcpy(labels[i].flgraphic, contents, sizeof(labels[i].flgraphic));
                if (!equals_no(contents))
                    cols->flgraphic = count;

            }
/******************************************************************************/
        } else if (strcmp(token, "KEEPAWAYHEAT") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strlen(contents)) {
                    if (strcmp("Y", contents) == 0) {
                        labels[i].keepawayheat = true;
                        cols->keepawayheat = count;
                    } else
                        labels[i].keepawayheat = false;
                }
            }
/******************************************************************************/
        } else if (strcmp(token, "INSERTGRAPHIC") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strlcpy(labels[i].insertgraphic, contents, sizeof(labels[i].insertgraphic));
                if (!(equals_no(contents)))
                    cols->insertgraphic = count;
            }
        } else if (strcmp(token, "KEEPDRY") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0) {
                    labels[i].keepdry = true;
                    cols->keepdry = count;
                } else
                    labels[i].keepdry = false;
            }
        } else if (strcmp(token, "LABELGRAPH1") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strlcpy(labels[i].labelgraph1, contents, sizeof(labels[i].labelgraph1));
                if (!(equals_no(contents)))
                    cols->labelgraph1 = count;
            }
        } else if (strcmp(token, "LABELGRAPH2") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strlcpy(labels[i].labelgraph2, contents, sizeof(labels[i].labelgraph2));
                if (!(equals_no(contents)))
                    cols->labelgraph2 = count;
            }
        } else if (strcmp(token, "LATEXFREE") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0) {
                    labels[i].latexfree = true;
                    cols->latexfree = count;
                } else
                    labels[i].latexfree = false;
            }
        } else if (strcmp(token, "LATEXSTATEMENT") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strlcpy(labels[i].latexstatement, contents, sizeof(labels[i].latexstatement));
                if (!(equals_no(contents)))
                    cols->latexstate = count;
            }
        } else if (strcmp(token, "LEVEL") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strlcpy(labels[i].level, contents, sizeof(labels[i].level));
                if (strlen(contents) && (!(equals_no(contents))))
                    cols->level = count;
            }
        } else if (strcmp(token, "LOGO1") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strlcpy(labels[i].logo1, contents, sizeof(labels[i].logo1));
                if (strlen(contents) && (!(equals_no(contents))))
                    cols->logo1 = count;
            }
        } else if (strcmp(token, "LOGO2") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strlcpy(labels[i].logo2, contents, sizeof(labels[i].logo2));
                if (strlen(contents) && (!(equals_no(contents))))
                    cols->logo2 = count;
            }
        } else if (strcmp(token, "LOGO3") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strlcpy(labels[i].logo3, contents, sizeof(labels[i].logo3));
                if (strlen(contents) && (!(equals_no(contents))))
                    cols->logo3 = count;
            }
        } else if (strcmp(token, "LOGO4") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strlcpy(labels[i].logo4, contents, sizeof(labels[i].logo4));
                if (strlen(contents) && (!(equals_no(contents))))
                    cols->logo4 = count;
            }
        } else if (strcmp(token, "LOGO5") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strlcpy(labels[i].logo5, contents, sizeof(labels[i].logo5));
                if (strlen(contents) && (!(equals_no(contents))))
                    cols->logo5 = count;
            }
        } else if (strcmp(token, "MDR1") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strlcpy(labels[i].mdr1, contents, sizeof(labels[i].mdr1));
                if (strlen(contents) && (!(equals_no(contents))))
                    cols->mdr1 = count;
            }
        } else if (strcmp(token, "MDR2") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strlcpy(labels[i].mdr2, contents, sizeof(labels[i].mdr2));
                if (strlen(contents) && (!(equals_no(contents))))
                    cols->mdr2 = count;
            }
        } else if (strcmp(token, "MDR3") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strlcpy(labels[i].mdr3, contents, sizeof(labels[i].mdr3));
                if (strlen(contents) && (!(equals_no(contents))))
                    cols->mdr3 = count;
            }
        } else if (strcmp(token, "MDR4") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strlcpy(labels[i].mdr4, contents, sizeof(labels[i].mdr4));
                if (strlen(contents) && (!(equals_no(contents))))
                    cols->mdr4 = count;
            }
        } else if (strcmp(token, "MDR5") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                if (get_field_contents_from_row(contents, i, count, tab_str)) {
                    strlcpy(labels[i].mdr5, contents, sizeof(labels[i].mdr5));
                    if (strlen(contents) && (!(equals_no(contents))))
                        cols->mdr5 = count;
                }
            }
        } else if (strcmp(token, "LOTGRAPHIC") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0) {
                    labels[i].lotgraphic = true;
                    cols->lotgraphic = count;
                } else
                    labels[i].lotgraphic = false;
            }
        } else if (strcmp(token, "LTNUMBER") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strncpy(labels[i].ltnumber, contents, sizeof(labels[0].ltnumber));
                if (strlen(contents) && (!(equals_no(contents))))
                    cols->ltnumber = count;
            }

        } else if (strcmp(token, "IPN") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strncpy(labels[i].ipn, contents, sizeof(labels[0].ipn));
                if (strlen(contents) && (!(equals_no(contents))))
                    cols->ipn = count;
            }
        } else if (strcmp(token, "MANINBOX") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (equals_yes(contents)) {
                    labels[i].maninbox = true;
                    cols->maninbox = count;
                } else
                    labels[i].maninbox = false;
            }
        } else if (strcmp(token, "MANUFACTUREDBY") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strlcpy(labels[i].manufacturedby, contents, sizeof(labels[i].manufacturedby));
                if (strlen(contents) && (!(equals_no(contents))))
                    cols->manufacturedby = count;
            }
        } else if (strcmp(token, "MANUFACTURER") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0) {
                    labels[i].manufacturer = true;
                    cols->manufacturer = count;
                } else
                    labels[i].manufacturer = false;
            }
        } else if (strcmp(token, "MFGDATE") == 0) {
            cols->mfgdate = 0;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (equals_yes(contents)) {
                    labels[i].mfgdate = true;
                    cols->mfgdate = count;
                } else
                    labels[i].mfgdate = false;
            }
        } else if (strcmp(token, "NORESTERILE") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0) {
                    labels[i].noresterilize = true;
                    cols->noresterile = count;
                } else
                    labels[i].noresterilize = false;
            }
        } else if (strcmp(token, "NONSTERILE") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0) {
                    labels[i].nonsterile = true;
                    cols->nonsterile = count;
                } else
                    labels[i].nonsterile = false;
            }
        } else if (strcmp(token, "OLDLABEL") == 0) {
            if (non_SAP_fields)
                for (int i = 1; i < spreadsheet_row_number; i++) {
                    get_field_contents_from_row(contents, i, count, tab_str);
                    strlcpy(labels[i].oldlabel, contents, sizeof(labels[i].oldlabel));
                    if (strlen(contents) && (!(equals_no(contents))))
                        cols->oldlabel = count;
                }
            else
                printf("Ignoring column \"%s\"\n", token);

        } else if (strcmp(token, "OLDTEMPLATE") == 0) {
            if (non_SAP_fields)
                for (int i = 1; i < spreadsheet_row_number; i++) {
                    get_field_contents_from_row(contents, i, count, tab_str);
                    strlcpy(labels[i].oldtemplate, contents, sizeof(labels[i].oldtemplate));
                    if (strlen(contents) && (!(equals_no(contents))))
                        cols->oldtemplate = count;
                }
            printf("Ignoring column \"%s\"\n", token);
        } else if (strcmp(token, "PATENTSTA") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strlcpy(labels[i].patentstatement, contents, sizeof(labels[i].patentstatement));
                if (strlen(contents) && (!(equals_no(contents))))
                    cols->patentstatement = count;
            }
        } else if (strcmp(token, "PHTDEHP") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strlen(contents)) {
                    if (strcmp("Y", contents) == 0) {
                        labels[i].phtdehp = true;
                        cols->phtdehp = count;
                    } else
                        labels[i].phtdehp = false;
                }
            }
        } else if (strcmp(token, "PHTBBP") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0) {
                    labels[i].phtbbp = true;
                    cols->phtbbp = count;
                } else
                    labels[i].phtbbp = false;
            }
        } else if (strcmp(token, "PHTDINP") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0) {
                    labels[i].phtdinp = true;
                    cols->phtdinp = count;
                } else
                    labels[i].phtdinp = false;
            }
        } else if (strcmp(token, "PVCFREE") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0) {
                    labels[i].pvcfree = true;
                    cols->pvcfree = count;
                } else
                    labels[i].pvcfree = false;
            }
        } else if (strcmp(token, "QUANTITY") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strlcpy(labels[i].quantity, contents, sizeof(labels[i].quantity));
                if (strlen(contents) && (!(equals_no(contents))))
                    cols->quantity = count;
            }
        } else if (strcmp(token, "REF") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0) {
                    labels[i].ref = true;
                    cols->ref = count;
                } else
                    labels[i].ref = false;
            }
        } else if (strcmp(token, "REFNUMBER") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strlen(contents)) {
                    if (strcmp("Y", contents) == 0) {
                        labels[i].refnumber = true;
                        cols->refnumber = count;
                    } else
                        labels[i].refnumber = false;
                }
            }
        } else if (strcmp(token, "REUSABLE") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0) {
                    labels[i].reusable = true;
                    cols->reusable = count;
                } else
                    labels[i].reusable = false;
            }
        } else if (strcmp(token, "REVISION") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strlcpy(labels[i].revision, contents, sizeof(labels[i].revision));
                if (strlen(contents) && (!(equals_no(contents))))
                    cols->revision = count;

            }
        } else if (strcmp(token, "RXONLY") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (equals_yes(contents)) {
                    labels[i].rxonly = true;
                    cols->rxonly = count;
                } else
                    labels[i].rxonly = false;
            }
        } else if (strcmp(token, "SINGLEUSE") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0) {
                    labels[i].singleuseonly = true;
                    cols->singleuse = count;
                } else
                    labels[i].singleuseonly = false;
            }
        } else if (strcmp(token, "SERIAL") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strlen(contents)) {
                    if (strcmp("Y", contents) == 0) {
                        labels[i].serial = true;
                        cols->serial = count;
                    } else
                        labels[i].serial = false;
                }
            }
        } else if (strcmp(token, "SINGLEPATIENTUSE") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0) {
                    labels[i].singlepatientuse = true;
                    cols->singlepatientuse = count;
                } else
                    labels[i].singlepatientuse = false;
            }
        } else if (strcmp(token, "SIZE") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strlcpy(labels[i].size, contents, sizeof(labels[i].size));
                if (strlen(contents) && (!(equals_no(contents))))
                    cols->size = count;
            }
        } else if (strcmp(token, "SIZELOGO") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0) {
                    labels[i].sizelogo = true;
                    cols->sizelogo = count;
                } else
                    labels[i].sizelogo = false;
            }
        } else if (strcmp(token, "STERILITYTYPE") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strlcpy(labels[i].sterilitytype, contents, sizeof(labels[i].sterilitytype));
                if (strlen(contents) && (!(equals_no(contents))))
                    cols->sterilitytype = count;
            }
        } else if (strcmp(token, "STERILESTA") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strlcpy(labels[i].sterilitystatement, contents, sizeof(labels[i].sterilitystatement));
                if (strlen(contents) && (!(equals_no(contents))))
                    cols->sterilitystatement = count;
            }
        } else if ((strcmp(token, "TEMPLATENUMBER") == 0) ||
                   (strcmp(token, "TEMPLATE") == 0)) {
            cols->templatenumber = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strlcpy(labels[i].template, contents, sizeof(labels[i].template));
            }
        } else if (strcmp(token, "TFXLOGO") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (equals_yes(contents)) {
                    labels[i].tfxlogo = true;
                    cols->tfxlogo = count;
                } else
                    labels[i].tfxlogo = false;
            }
        } else if (strcmp(token, "VERSION") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strlcpy(labels[i].version, contents, sizeof(labels[i].version));
                if (strlen(contents) && (!(equals_no(contents))))
                    cols->version = count;
            }
        } else {
            if (strlen(token) > 0)
                printf("Ignoring column \"%s\"\n", token);
        }
        count++;
        free(token);
    }
    return count;
}

int sort_labels(Label_record *labels) {

    Label_record min_label;
    bool sorted = true;

    for (int i = 1; i < spreadsheet_row_number; i++) {
        int min_index = i;
        min_label = labels[i];

        for (int j = i + 1; j < spreadsheet_row_number; j++) {
            if (strcmp(labels[j].label, min_label.label) < 0) {
                min_label = labels[j];
                min_index = j;
            }
        }

        if (i != min_index) {
            swap_label_records(labels, i, min_index);
            sorted = false;
        }
    }
    return sorted;
}

void swap_label_records(Label_record *labels, int i, int min_index) {
    Label_record temp = labels[i];
    labels[i] = labels[min_index];
    labels[min_index] = temp;
}
