/**
 *  label.c
 */
#include "label.h"
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

    return length;
}

int parse_spreadsheet(char *buffer, Label_record *labels, Column_header *cols) {
    unsigned short count = 0;

    char tab_str = TAB;
    char contents[MAX_COLUMNS];

    while (strlen(buffer) > 0) {

        // Keep extracting tokens while the delimiter is present in buffer
        char *token = get_token(buffer, tab_str);

        if (strcmp(token, "LABEL") == 0) {
            cols->label = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].label, contents);
            }
        } else if (strcmp(token, "MATERIAL") == 0) {
            cols->material = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].material, contents);
            }
        } else if (strcmp(token, "TDLINE") == 0) {
            cols->tdline = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);

                labels[i].tdline = (char *) malloc(strlen(contents) + 1);
                strcpy(labels[i].tdline, contents);
            }

/******************************************************************************/
        } else if (strcmp(token, "ADDRESS") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].address, contents);
                if (strlen(contents))
                    cols->address = count;
            }
/******************************************************************************/

        } else if (strcmp(token, "BARCODETEXT") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].gtin, contents);
                if (strlen(contents))
                    cols->barcodetext = count;
            }

        } else if (strcmp(token, "BOMLEVEL") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].bomlevel, contents);
                if (strlen(contents))
                    cols->bomlevel = count;
            }
/******************************************************************************/
        } else if (strcmp(token, "CAUTION") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strlen(contents)) {
                    cols->caution = count;
                    if (strcmp("Y", contents) == 0)
                        labels[i].caution = true;
                    else
                        labels[i].caution = false;
                }
            }
/******************************************************************************/
        } else if (strcmp(token, "CAUTIONSTATE") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].cautionstatement, contents);
                if (strlen(contents))
                    cols->cautionstate = count;
            }
/******************************************************************************/
        } else if (strcmp(token, "CE0120") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].cemark, contents);
                if (strlen(contents))
                    cols->ce0120 = true;

            }
        } else if (strcmp(token, "CONSULTIFU") == 0) {
            cols->consultifu = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0)
                    labels[i].consultifu = true;
                else
                    labels[i].consultifu = false;
            }
        } else if (strcmp(token, "CONTAINSLATEX") == 0) {
            cols->latex = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0)
                    labels[i].latex = true;
                else
                    labels[i].latex = false;
            }
        } else if (strcmp(token, "COOSTATE") == 0) {
            cols->coostate = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].coostate, contents);
            }
        } else if (strcmp(token, "DISTRIBUTEDBY") == 0) {
            cols->distby = count;

            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].distby, contents);
            }
        } else if ((strcmp(token, "DONOTUSEDAM") == 0) ||
                   (strcmp(token, "DONOTPAKDAM") == 0)) {
            cols->donotusedam = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0)
                    labels[i].donotusedamaged = true;
                else
                    labels[i].donotusedamaged = false;
            }
/******************************************************************************/
        } else if (strcmp(token, "ECREP") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strlen(contents) > 0) {
                    cols->ecrep = count;
                    if (strcmp("Y", contents) == 0)
                        labels[i].ecrep = true;
                    else
                        labels[i].ecrep = false;
                }
            }
/******************************************************************************/

        } else if (strcmp(token, "ECREPADDRESS") == 0) {
            cols->ecrepaddress = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].ecrepaddress, contents);
            }
        } else if (strcmp(token, "ELECTROSURIFU") == 0) {
            cols->electroifu = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0)
                    labels[i].electroifu = true;
                else
                    labels[i].electroifu = false;
            }
/******************************************************************************/
        } else if (strcmp(token, "EXPDATE") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strlen(contents) > 0) {
                    cols->expdate = count;
                    if (strcmp("Y", contents) == 0)
                        labels[i].expdate = true;
                    else
                        labels[i].expdate = false;
                }
            }
/******************************************************************************/
        } else if (strcmp(token, "FLGRAPHIC") == 0) {
            cols->flgraphic = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].flgraphic, contents);
            }
/******************************************************************************/
        } else if (strcmp(token, "KEEPAWAYHEAT") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strlen(contents)) {
                    cols->keepawayheat = count;
                    if (strcmp("Y", contents) == 0)
                        labels[i].keepawayheat = true;
                    else
                        labels[i].keepawayheat = false;
                }
            }
/******************************************************************************/
        } else if (strcmp(token, "INSERTGRAPHIC") == 0) {
            cols->insertgraphic = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].insertgraphic, contents);
            }
        } else if (strcmp(token, "KEEPDRY") == 0) {
            cols->keepdry = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0)
                    labels[i].keepdry = true;
                else
                    labels[i].keepdry = false;
            }
        } else if (strcmp(token, "LABELGRAPH1") == 0) {
            cols->labelgraph1 = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].labelgraph1, contents);
            }
        } else if (strcmp(token, "LABELGRAPH2") == 0) {
            cols->labelgraph2 = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].labelgraph2, contents);
            }
        } else if (strcmp(token, "LATEXFREE") == 0) {
            cols->latexfree = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0)
                    labels[i].latexfree = true;
                else
                    labels[i].latexfree = false;
            }
        } else if (strcmp(token, "LATEXSTATEMENT") == 0) {
            cols->latexstate = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].latexstatement, contents);
            }
        } else if (strcmp(token, "LEVEL") == 0) {
            cols->level = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].level, contents);
            }
        } else if (strcmp(token, "LOGO1") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].logo1, contents);
                if (strlen(contents) > 0)
                    cols->logo1 = count;
            }
        } else if (strcmp(token, "LOGO2") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].logo2, contents);
                if (strlen(contents) > 0)
                    cols->logo2 = count;
            }
        } else if (strcmp(token, "LOGO3") == 0) {
            cols->logo3 = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].logo3, contents);
            }
        } else if (strcmp(token, "LOGO4") == 0) {
            cols->logo4 = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].logo4, contents);
            }
        } else if (strcmp(token, "LOGO5") == 0) {
            cols->logo5 = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].logo5, contents);
            }
        } else if (strcmp(token, "MDR1") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                if (get_field_contents_from_row(contents, i, count, tab_str)) {
                    strcpy(labels[i].mdr1, contents);
                    cols->mdr1 = count;
                }
            }
        } else if (strcmp(token, "MDR2") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                if (get_field_contents_from_row(contents, i, count, tab_str)) {
                    strcpy(labels[i].mdr2, contents);
                    cols->mdr2 = count;
                }
            }
        } else if (strcmp(token, "MDR3") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                if (get_field_contents_from_row(contents, i, count, tab_str)) {
                    strcpy(labels[i].mdr3, contents);
                    cols->mdr3 = count;
                }
            }
        } else if (strcmp(token, "MDR4") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                if (get_field_contents_from_row(contents, i, count, tab_str)) {
                    strcpy(labels[i].mdr4, contents);
                    cols->mdr4 = count;
                }
            }
        } else if (strcmp(token, "MDR5") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                if (get_field_contents_from_row(contents, i, count, tab_str)) {
                    strcpy(labels[i].mdr5, contents);
                    cols->mdr5 = count;
                }
            }
        } else if (strcmp(token, "LOTGRAPHIC") == 0) {
            cols->lotgraphic = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0)
                    labels[i].lotgraphic = true;
                else
                    labels[i].lotgraphic = false;
            }
        } else if (strcmp(token, "LTNUMBER") == 0) {
            cols->ltnumber = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strncpy(labels[i].ipn, contents, sizeof(labels[0].ipn));
            }
        } else if (strcmp(token, "MANINBOX") == 0) {
            cols->maninbox = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if ((strcmp("Y", contents) == 0) || (strcmp("YES", contents) == 0))
                    labels[i].maninbox = true;
                else
                    labels[i].maninbox = false;
            }
        } else if (strcmp(token, "MANUFACTUREDBY") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].manufacturedby, contents);
                if (strlen(contents))
                    cols->manufacturedby = count;
            }
        } else if (strcmp(token, "MANUFACTURER") == 0) {
            cols->manufacturer = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0)
                    labels[i].manufacturer = true;
                else
                    labels[i].manufacturer = false;
            }
        } else if (strcmp(token, "MFGDATE") == 0) {
            cols->mfgdate = 0;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if ((strcasecmp("Y", contents) == 0) ||
                    (strcasecmp("YES", contents) == 0)) {
                    labels[i].mfgdate = true;
                    cols->mfgdate = count;
                } else
                    labels[i].mfgdate = false;
            }
        } else if (strcmp(token, "NORESTERILE") == 0) {
            cols->noresterile = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0)
                    labels[i].noresterilize = true;
                else
                    labels[i].noresterilize = false;
            }
        } else if (strcmp(token, "NONSTERILE") == 0) {
            cols->nonsterile = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0)
                    labels[i].nonsterile = true;
                else
                    labels[i].nonsterile = false;
            }
        } else if (strcmp(token, "PATENTSTA") == 0) {
            cols->patentstatement = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].patentstatement, contents);
            }
        } else if (strcmp(token, "PHTDEHP") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strlen(contents)) {
                    cols->phtdehp = count;
                    if (strcmp("Y", contents) == 0)
                        labels[i].phtdehp = true;
                    else
                        labels[i].phtdehp = false;
                }
            }
        } else if (strcmp(token, "PHTBBP") == 0) {
            cols->phtbbp = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0)
                    labels[i].phtbbp = true;
                else
                    labels[i].phtbbp = false;
            }
        } else if (strcmp(token, "PHTDINP") == 0) {
            cols->phtdinp = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0)
                    labels[i].phtdinp = true;
                else
                    labels[i].phtdinp = false;
            }
        } else if (strcmp(token, "PVCFREE") == 0) {
            cols->pvcfree = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0)
                    labels[i].pvcfree = true;
                else
                    labels[i].pvcfree = false;
            }
        } else if (strcmp(token, "QUANTITY") == 0) {
            cols->quantity = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].quantity, contents);
            }
        } else if (strcmp(token, "REF") == 0) {
            cols->ref = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0)
                    labels[i].ref = true;
                else
                    labels[i].ref = false;
            }
        } else if (strcmp(token, "REFNUMBER") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strlen(contents)) {
                    cols->refnumber = count;
                    if (strcmp("Y", contents) == 0)
                        labels[i].refnumber = true;
                    else
                        labels[i].refnumber = false;
                }
            }
        } else if (strcmp(token, "REUSABLE") == 0) {
            cols->reusable = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0)
                    labels[i].reusable = true;
                else
                    labels[i].reusable = false;
            }
        } else if (strcmp(token, "REVISION") == 0) {
            cols->revision = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].revision, contents);
            }
        } else if (strcmp(token, "RXONLY") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if ((strcasecmp("Y", contents) == 0) ||
                    (strcasecmp("YES", contents) == 0)) {
                    labels[i].rxonly = true;
                    cols->rxonly = count;
                } else
                    labels[i].rxonly = false;
            }
        } else if (strcmp(token, "SINGLEUSE") == 0) {
            cols->singleuse = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0)
                    labels[i].singleuseonly = true;
                else
                    labels[i].singleuseonly = false;
            }
        } else if (strcmp(token, "SERIAL") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strlen(contents)) {
                    cols->serial = count;

                    if (strcmp("Y", contents) == 0)
                        labels[i].serial = true;
                    else
                        labels[i].serial = false;
                }
            }
        } else if (strcmp(token, "SINGLEPATIENTUSE") == 0) {
            cols->singlepatientuse = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0)
                    labels[i].singlepatientuse = true;
                else
                    labels[i].singlepatientuse = false;
            }
        } else if (strcmp(token, "SIZE") == 0) {
            cols->size = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].size, contents);
            }
/*        } else if (strcmp(token, "SIZELOGO") == 0) {
            cols->sizelogo = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0)
                    labels[i].sizelogo = true;
                else
                    labels[i].sizelogo = false;
            }*/
        } else if (strcmp(token, "STERILITYTYPE") == 0) {
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].sterilitytype, contents);
                if (strlen(contents))
                    cols->sterilitytype = count;
            }
        } else if (strcmp(token, "STERILESTA") == 0) {
            cols->sterilitystatement = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].sterilitystatement, contents);
            }
        } else if (strcmp(token, "TEMPLATENUMBER") == 0) {
            cols->templatenumber = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].template, contents);
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
            cols->version = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].version, contents);
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

void sort_labels(Label_record *labels) {

    Label_record min_label;

    for (int i = 1; i < spreadsheet_row_number; i++) {
        int min_index = i;
        min_label = labels[i];

        for (int j = i + 1; j < spreadsheet_row_number; j++) {
            if (strcmp(labels[j].label, min_label.label) < 0) {
                min_label = labels[j];
                min_index = j;
            }
        }

        swap_label_records(labels, i, min_index);
    }
}

void swap_label_records(Label_record *labels, int i, int min_index) {
    Label_record temp = labels[i];
    labels[i] = labels[min_index];
    labels[min_index] = temp;
}
