/**
 *  label.c
 */
#include "label.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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
    int buffer_len = strlen(buffer);
    char *token = (char *) malloc(MAX_COLUMNS * sizeof(char));

    if ((delimiter = strchr(buffer, tab_str)) != NULL) {
        delimiter++;
        int c = 0;
        for (char *i = buffer; i < delimiter - 1; i++) {
            token[c++] = *i;
        }
        token[c] = '\0';
        int delimiter_len = delimiter - buffer;
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

char *multi_tok(char *input, char *delimiter) {
    static char *string;
    if (input != NULL)
        string = input;

    if (string == NULL)
        return string;

    char *end = strstr(string, delimiter);
    if (end == NULL) {
        char *temp = string;
        string = NULL;
        return temp;
    }

    char *temp = string;

    *end = '\0';
    string = end + strlen(delimiter);
    return temp;
}

/**
    returns the position of the nth occurrence of a delimiter in a char array
*/
int peek_nth_token(int n, const char *buffer, char delimiter) {

    if (n == 0)
        return 0;

    int i = 0;
    int hit_count = 0;
    int length = strlen(buffer);

    while ((i < length) && (hit_count < n)) {
        if (buffer[i] == delimiter)
            hit_count++;
        i++;
    }
    if (hit_count == n)
        return i - 1;

    return -1;
}

int get_field_contents_from_row(char *contents, int i, int count, char tab_str) {

    int start = 0;
    int stop = 0;

    if (count == 0)
        start = 0;
    else if ((start = peek_nth_token(count, spreadsheet[i], tab_str)) == -1)
        start = strlen(spreadsheet[i]);
    else
        start++;

    if ((stop = peek_nth_token(count + 1, spreadsheet[i], tab_str)) == -1)
        stop = strlen(spreadsheet[i]);

    int length = stop - start;
    strncpy(contents, spreadsheet[i] + start, (size_t) length);
    contents[length] = '\0';

    //characteristic_lookup(contents);
    //strcpy(contents, "kottiel");

    return 0;
}

int parse_spreadsheet(char *buffer, Label_record *labels, Column_header *cols) {
    int count = 0;

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

                labels[i].tdline = (char *) malloc(strlen(contents));
                strcpy(labels[i].tdline, contents);
            }
        } else if (strcmp(token, "ADDRESS") == 0) {
            cols->address = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].address, contents);
            }
            /*} else if (strcmp(token, "BARCODE1") == 0) {
                cols->barcode1 = count;
                for (int i = 1; i < spreadsheet_row_number; i++) {
                    get_field_contents_from_row(contents, i, count, tab_str);
                    strcpy(labels[i].barcode1, contents);
                }*/
        } else if (strcmp(token, "BARCODETEXT") == 0) {
            cols->barcodetext = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].gtin, contents);
            }
        } else if (strcmp(token, "BOMLEVEL") == 0) {
            cols->bomlevel = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].bomlevel, contents);
            }
        } else if (strcmp(token, "CAUTION") == 0) {
            cols->caution = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0)
                    labels[i].caution = true;
                else
                    labels[i].caution = false;
            }
        } else if (strcmp(token, "CAUTIONSTATE") == 0) {
            cols->cautionstate = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].cautionstatement, contents);
            }
        } else if (strcmp(token, "CE0120") == 0) {
            cols->ce0120 = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].cemark, contents);
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
        } else if (strcmp(token, "DONOTUSEDAM") == 0) {
            cols->donotusedam = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0)
                    labels[i].donotusedamaged = true;
                else
                    labels[i].donotusedamaged = false;
            }
        } else if (strcmp(token, "ECREP") == 0) {
            cols->ecrep = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0)
                    labels[i].ecrep = true;
                else
                    labels[i].ecrep = false;
            }
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
        } else if (strcmp(token, "EXPDATE") == 0) {
            cols->expdate = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0)
                    labels[i].expdate = true;
                else
                    labels[i].expdate = false;
            }
        } else if (strcmp(token, "FLGRAPHIC") == 0) {
            cols->flgraphic = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].flgraphic, contents);
            }
        } else if (strcmp(token, "KEEPAWAYHEAT") == 0) {
            cols->keepawayheat = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0)
                    labels[i].keepawayheat = true;
                else
                    labels[i].keepawayheat = false;
            }
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
            cols->logo1 = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].logo1, contents);
            }
        } else if (strcmp(token, "LOGO2") == 0) {
            cols->logo2 = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].logo2, contents);
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
                strcpy(labels[i].ipn, contents);
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
            cols->manufacturedby = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].manufacturedby, contents);
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
            cols->mfgdate = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0)
                    labels[i].mfgdate = true;
                else
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
            cols->phtdehp = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0)
                    labels[i].phtdehp = true;
                else
                    labels[i].phtdehp = false;
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
            cols->refnumber = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0)
                    labels[i].refnumber = true;
                else
                    labels[i].refnumber = false;
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
            cols->rxonly = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0)
                    labels[i].rxonly = true;
                else
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
            cols->serial = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0)
                    labels[i].serial = true;
                else
                    labels[i].serial = false;
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
        } else if (strcmp(token, "SIZELOGO") == 0) {
            cols->sizelogo = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0)
                    labels[i].sizelogo = true;
                else
                    labels[i].sizelogo = false;
            }
        } else if (strcmp(token, "STERILITYTYPE") == 0) {
            cols->sterilitytype = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                strcpy(labels[i].sterilitytype, contents);
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
            cols->tfxlogo = count;
            for (int i = 1; i < spreadsheet_row_number; i++) {
                get_field_contents_from_row(contents, i, count, tab_str);
                if (strcmp("Y", contents) == 0)
                    labels[i].tfxlogo = true;
                else
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

    for (int i = 0; i < spreadsheet_row_number; i++)
        printf("%s\n", labels[i].label);
}

void swap_label_records(Label_record *labels, int i, int min_index) {
    Label_record temp = labels[i];
    labels[i] = labels[min_index];
    labels[min_index] = temp;
}
