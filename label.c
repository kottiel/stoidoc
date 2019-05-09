#include "label.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
    This function initializes the dynamically allocated spreadsheet array.
    @return 0 if successful, -1 if unsuccessful.
*/
int spreadsheet_init() {

    spreadsheet_cap = INITIAL_CAP;

    if ((spreadsheet = (char **)malloc(INITIAL_CAP * sizeof(char *))) == NULL)
        return -1;
    else
        return 0;
}

int spreadsheet_expand() {

    spreadsheet_cap *= 2;
    if ((spreadsheet = (char **)realloc(spreadsheet, spreadsheet_cap * sizeof(char *))) == NULL)
        return -1;
    else
        return 0;
}

char *get_token(char *buffer, char tab_str) {
    char *delimiter;
    int buffer_len = strlen(buffer);
    char *token = (char *)malloc(MAX_COLUMNS * sizeof(char));

    if ((delimiter = strchr(buffer, tab_str)) != NULL) {
        delimiter++;
        int c = 0;
        for (char *i = buffer; i < delimiter - 1; i++) {
            token[c++] = *i;
        }
        token[c] = '\0';
        int delimiter_len = delimiter - buffer;
        memmove(buffer, delimiter, buffer_len - delimiter_len + 1);
        buffer_len = strlen(buffer);
    } else if (buffer_len > 0) { // get last token
        int c = 0;
        char *cp = buffer;
        while (*cp) {
            token[c++] = *cp;
            cp++;
        }
        token[c] = '\0';
        buffer[0] = '\0';
        buffer_len = 0;
    }
    return token;
}

int process_column_header(char *buffer, Column_header *cols) {
    int count = 0;
    char tab_str = TAB;

    while (strlen(buffer) > 0) {

        // Keep extracting tokens while the delimiter is present in buffer
        char *token = get_token(buffer, tab_str);

        if (strcmp(token, "LABEL")== 0)
            cols->label = count;
        else if (strcmp(token, "MATERIAL")== 0)
            cols->material = count;
        else if (strcmp(token, "TDLINE")== 0)
            cols->tdline = count;
        else if (strcmp(token, "ADDRESS") == 0)
            cols->address = count;
        else if (strcmp(token, "BARCODETEXT")== 0)
            cols->barcodetext = count;
        else if (strcmp(token, "CAUTION")== 0)
            cols->caution = count;
        else if (strcmp(token, "CAUTIONSTATE") == 0)
            cols->cautionstate = count;
        else if (strcmp(token, "CE0120") == 0)
            cols->ce0120 = count;
        else if (strcmp(token, "CONSULTIFU") == 0)
            cols->consultifu = count;
        else if (strcmp(token, "COOSTATE") == 0)
            cols->coostate = count;
        else if (strcmp(token, "DONOTUSEDAM") == 0)
            cols->donotusedam = count;
        else if (strcmp(token, "ECREP") == 0)
            cols->ecrep = count;
        else if (strcmp(token, "ECREPADDRESS") == 0)
            cols->ecrepaddress = count;
        else if (strcmp(token, "EXPDATE") == 0)
            cols->expdate = count;
        else if (strcmp(token, "FLGRAPHIC") == 0)
            cols->flgraphic = count;
        else if (strcmp(token, "LABELGRAPH1") == 0)
            cols->labelgraph1 = count;
        else if (strcmp(token, "LABELGRAPH2") == 0)
            cols->labelgraph2 = count;
        else if (strcmp(token, "LATEXFREE") == 0)
            cols->latexfree = count;
        else if (strcmp(token, "LATEXSTATEMENT") == 0)
            cols->latexstate = count;
        else if (strcmp(token, "LEVEL")== 0)
            cols->level = count;
        else if (strcmp(token, "LOGO1")== 0)
            cols->logo1 = count;
        else if (strcmp(token, "LOGO2")== 0)
            cols->logo2 = count;
        else if (strcmp(token, "LOGO3")== 0)
            cols->logo3 = count;
        else if (strcmp(token, "LOGO4")== 0)
            cols->logo4 = count;
        else if (strcmp(token, "LOGO5")== 0)
            cols->logo5 = count;
        else if (strcmp(token, "LOTGRAPHIC") == 0)
            cols->lotgraphic = count;
        else if (strcmp(token, "LTNUMBER")== 0)
            cols->ltnumber = count;
        else if (strcmp(token, "MANUFACTURER") == 0)
            cols->manufacturer = count;
        else if (strcmp(token, "MFGDATE") == 0)
            cols->mfgdate = count;
        else if (strcmp(token, "NORESTERILE") == 0)
            cols->noresterile = count;
        else if (strcmp(token, "PATENTSTA") == 0)
            cols->patentstatement = count;
        else if (strcmp(token, "PHTDEHP") == 0)
            cols->phtdehp = count;
        else if (strcmp(token, "PHTBBP") == 0)
            cols->phtbbp = count;
        else if (strcmp(token, "PHTDINP") == 0)
            cols->phtdinp = count;
        else if (strcmp(token, "PVCFREE") == 0)
            cols->pvcfree = count;
        else if (strcmp(token, "QUANTITY")== 0)
            cols->quantity = count;
        else if (strcmp(token, "REF") == 0)
            cols->ref = count;
        else if (strcmp(token, "REFNUMBER") == 0)
            cols->refnumber = count;
        else if (strcmp(token, "REVISION")== 0)
            cols->revision = count;
        else if (strcmp(token, "RXONLY") == 0)
            cols->rxonly = count;
        else if (strcmp(token, "SINGLEUSE") == 0)
            cols->size = count;
        else if (strcmp(token, "SIZE") == 0)
            cols->size = count;
        else if (strcmp(token, "STERILITYTYPE") == 0)
            cols->sterilitytype = count;
        else if (strcmp(token, "TEMPLATENUMBER") == 0)
            cols->templatenumber = count;
        else if (strcmp(token, "TFXLOGO") == 0)
            cols->tfxlogo = count;
        count++;
        free(token);
    }
    return count;
}