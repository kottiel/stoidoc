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
#define FILE_EXT_LEN   10

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
                        {"CE",          "CE Mark"         },
                        {"CE0120",      "CE_0120_Below"   },
                        {"CE0123",      "CE123"           },
                        {"CE0050",      "CE0050"          },
                        {"HEMO_AUTO_L", "HemoAutoL"       },
                        {"HEMO_L",      "HemolokL"        },
                        {"HEMO_ML",     "HemolokML"       },
                        {"HMOCLPTRD",   "HmoclpTrd"       },
                        {"NO",          "blank-01"        },
                        {"N",           "blank-01"        },
                        {"STERILEEO",   "Sterile_EO"      },
                        {"WECK_LOGO",   "Wecklogo"        }
                       };


    int lookupsize = sizeof(lookup)/sizeof(lookup[0]);

/**
    perform a binary search on the lookup array to find the SAP
    characteristic definition given the characteristic value
*/
char *sap_lookup(char *needle) {

    int start = 0;
    int end   = lookupsize - 1;
    int middle = 0;

    bool exit = false;
    char *haystack = lookup[0][0];

    while (!exit) {
        if (middle != (end - start) / 2 + start)
            middle =  (end - start) / 2 + start;
        else
            exit = true;
        haystack = lookup[middle][0];
        if (strcmp(needle, haystack) == 0) {
            return lookup[middle][1];
        }
        else if (strcmp(needle, haystack) < 0) {
            end = middle - 1;
        }
        else {
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
void read_spreadsheet(FILE *fp)
{

    char c;
    char buffer[MAX_COLUMNS];
    bool line_not_empty = false;
    int i = 0;

    while ((c = fgetc(fp)) != EOF)
    {
        if (c == CR)
        {
            if ((c = fgetc(fp)) == LF)
            {
                buffer[i] = '\0';
                if (line_not_empty)
                {
                    if (spreadsheet_row_number >= spreadsheet_cap)
                    {
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
        }
        else if ((c != CR) && (c != LF))
        {
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
void print_spaces(FILE *fpout, int n)
{
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
    int n = 255 - strlen(GRAPHICS_PATH) + strlen(graphic);
    for (int i = 0; i < n; i++)
        fprintf(fpout, " ");
}

/**
    prints a specified number of spaces to a file stream
    @param fpout points to the output file
    @param n is the number of spaces to print
*/
void print_Z2BTLC01000(FILE *fpout, char *ctrl_num)
{
    fprintf(fpout, "Z2BTLC01000");
    print_spaces(fpout, 19);
    fprintf(fpout, "500000000000");
    // cols 22-29 - 7 digit control number?
    fprintf(fpout, "%s", ctrl_num);
    fprintf(fpout, "%06d", sequence_number++);
    fprintf(fpout, "00000204");
}

/**
    prints the IDoc control record
    @param fpout points to the output file
*/
int print_control_record(FILE *fpout, char *ctrl_num)
{

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    // line 1
    fprintf(fpout, "EDI_DC40  500000000000");
    // cols 22-29 - 7 digit control number?
    fprintf(fpout, "%s", ctrl_num);
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
void print_label_idoc_records(FILE *fpout, Label_record *labels, Column_header *cols, int record, char *ctrl_num) {

    // Print the records for a given IDOC (labels[record])

    // the name of the graphic to be appended to the graphic path
    char graphic_name[MED];
    char *gnp;

    // MATERIAL record (optional)
    if ((cols->material) && (strlen(labels[record].material) > 0)) {
        fprintf(fpout, "Z2BTMH01000");
        print_spaces(fpout, 19);
        fprintf(fpout, "500000000000");
        // cols 22-29 - 7 digit control number?
        fprintf(fpout, "%s", ctrl_num);
        fprintf(fpout, "%06d", sequence_number++);
        fprintf(fpout, "00000002");
        fprintf(fpout, "%-18s", labels[record].material);
        fprintf(fpout, "\r\n");
    }

    // LABEL record (required)
        fprintf(fpout, "Z2BTLH01000");
        print_spaces(fpout, 19);
        fprintf(fpout, "500000000000");
        // cols 22-29 - 7 digit control number?
        fprintf(fpout, "%s", ctrl_num);
        fprintf(fpout, "%06d", sequence_number++);
        fprintf(fpout, "00000103");
        fprintf(fpout, "%-18s", labels[record].label);
        fprintf(fpout, "\r\n");

    // TDLINE record(s) (optional) - repeat as many times as there are "##"
    if (cols->tdline) {
        /* get the first token */
        int tdline_count = 0;

        char *token = labels[record].tdline;
        while (strlen(token) > 0) {
            fprintf(fpout, "Z2BTTX01000");
            print_spaces(fpout, 19);
            fprintf(fpout, "500000000000");
            // cols 22-29 - 7 digit control number?
            fprintf(fpout, "%s", ctrl_num);
            fprintf(fpout, "%06d", sequence_number++);
            fprintf(fpout, "00000204GRUNE  ENMATERIAL  ");
            fprintf(fpout, "%s", labels[record].label);
            print_spaces(fpout, 61);

            //check for and remove any leading...
            if (token[0] == '\"')
                memcpy(token, token + 1, strlen(token));

            // ...and/or trailing quotes
            if (token[strlen(token) - 1] == '\"')
                token[strlen(token) - 1] = '\0';

            // and convert instances of double quotes to single quotes
            char *a = strstr(token, "\"\"");
            if (a != NULL)
            {
                memcpy(a, a+1, strlen(a));
                token[strlen(token)] = '\0';
            }

            char *dpos = strstr(token, "##");

            if (dpos != NULL) {
                *dpos = '\0';
                fprintf(fpout, "%s", token);
                fprintf(fpout, "##");
                print_spaces(fpout, 70 - strlen(token) - 2);

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
            fprintf(fpout, "\r\n");
        }
    }

    // TEMPLATENUMBER record (required)
    print_Z2BTLC01000(fpout, ctrl_num);
    fprintf(fpout, "%-30s", "TEMPLATENUMBER");
    fprintf(fpout, "%-30s", labels[record].template);
    fprintf(fpout, "%-255s", labels[record].template);
    fprintf(fpout, "\r\n");

    // REVISION record (optional)
    if ((cols->revision) && (strlen(labels[record].revision) > 0)) {
        print_Z2BTLC01000(fpout, ctrl_num);
        fprintf(fpout, "%-30s", "REVISION");
        fprintf(fpout, "%-30s", labels[record].revision);
        fprintf(fpout, "%-255s", labels[record].revision);
        fprintf(fpout, "\r\n");
    }

    // SIZE record (optional)
    if ((cols->size) && (strlen(labels[record].size) > 0)){
        char *token = labels[record].size;

        //check for and remove any leading...
        if (token[0] == '\"')
            memcpy(token, token + 1, strlen(token));

        // ...and/or trailing quotes
        if (token[strlen(token) - 1] == '\"')
            token[strlen(token) - 1] = '\0';

        // and convert all instances of double quotes to single quotes
        char *a;
        int diff;
        while ((a = strstr(token, "\"\"")) != NULL) {
            diff = a - token;
            memcpy(token + diff, token + diff + 1, strlen(token) - 2);
        }

        print_Z2BTLC01000(fpout, ctrl_num);
        fprintf(fpout, "%-30s", "SIZE");
        fprintf(fpout, "%-30s", labels[record].size);
        fprintf(fpout, "%-255s", labels[record].size);
        fprintf(fpout, "\r\n");
    }

    // LEVEL record (optional)
    if ((cols->level) && (strlen(labels[record].level) > 0)) {
        print_Z2BTLC01000(fpout, ctrl_num);
        fprintf(fpout, "%-30s", "LEVEL");
        fprintf(fpout, "%-30s", labels[record].level);
        fprintf(fpout, "%-255s", labels[record].level);
        fprintf(fpout, "\r\n");
    }

    // QUANTITY record (optional)
    if ((cols->quantity) && (strlen(labels[record].quantity) > 0)) {
        print_Z2BTLC01000(fpout, ctrl_num);
        fprintf(fpout, "%-30s", "QUANTITY");
        fprintf(fpout, "%-30s", labels[record].quantity);
        fprintf(fpout, "%-255s", labels[record].quantity);
        fprintf(fpout, "\r\n");
    }

    // BARCODETEXT record (optional)
    if ((cols->barcodetext) && (strlen(labels[record].gtin) > 0)) {
        print_Z2BTLC01000(fpout, ctrl_num);
        fprintf(fpout, "%-30s", "BARCODETEXT");
        fprintf(fpout, "%-30s", labels[record].gtin);
        fprintf(fpout, "%-255s", labels[record].gtin);
        fprintf(fpout, "\r\n");
    }

    // LTNUMBER record (optional)
    if ((cols->ltnumber) && (strlen(labels[record].ipn) > 0)) {
        print_Z2BTLC01000(fpout, ctrl_num);
        fprintf(fpout, "%-30s", "LTNUMBER");
        fprintf(fpout, "%-30s", labels[record].ipn);
        fprintf(fpout, "%-255s", labels[record].ipn);
        fprintf(fpout, "\r\n");
    }

    //
    // GRAPHIC01 - GRAPHIC08 Fields (optional)
    //

    int g_cnt = 1;
    char g_cnt_str[03];
    char graphic[] = "GRAPHIC";

    // SINGLEPATIENTUSE record (optional)
    if (cols->singlepatientuse) {
        if (labels[record].singlepatientuse) {
            print_Z2BTLC01000(fpout, ctrl_num);
            sprintf(g_cnt_str, "%02d", g_cnt++);
            fprintf(fpout, "%-30s", strcat(graphic, g_cnt_str));
            fprintf(fpout, "%-30s", "Y");
            print_graphic_path(fpout, "SINGLEPATIENUSE.tif");
            fprintf(fpout, "\r\n");
        }
    }

    // CAUTION record (optional)
    if (cols->caution) {
        if (labels[record].caution)
        {
            print_Z2BTLC01000(fpout, ctrl_num);
            sprintf(g_cnt_str, "%02d", g_cnt++);
            strcpy(graphic, "GRAPHIC");
            fprintf(fpout, "%-30s", strcat(graphic, g_cnt_str));
            fprintf(fpout, "%-30s", "Y");
            print_graphic_path(fpout, "Caution.tif");
            fprintf(fpout, "\r\n");
        }
    }

    // ConsultIFU record (optional)
    if (cols->consultifu)
    {

        if (labels[record].consultifu)
        {
            print_Z2BTLC01000(fpout, ctrl_num);
            sprintf(g_cnt_str, "%02d", g_cnt++);
            strcpy(graphic, "GRAPHIC");
            fprintf(fpout, "%-30s", strcat(graphic, g_cnt_str));
            fprintf(fpout, "%-30s", "Y");
            print_graphic_path(fpout, "ConsultIFU.tif");
            fprintf(fpout, "\r\n");
        }
    }

    // DoNotUsePakDam record (optional)
    if (cols->donotusedam)
    {
        if (labels[record].donotusedamaged)
        {
            print_Z2BTLC01000(fpout, ctrl_num);
            sprintf(g_cnt_str, "%02d", g_cnt++);
            strcpy(graphic, "GRAPHIC");
            fprintf(fpout, "%-30s", strcat(graphic, g_cnt_str));
            fprintf(fpout, "%-30s", "Y");
            print_graphic_path(fpout, "DoNotUsePakDam.tif");
            fprintf(fpout, "\r\n");
        }
    }

    // Non-sterile record (optional)
    if (cols->nonsterile)
    {
        if (labels[record].nonsterile)
        {
            print_Z2BTLC01000(fpout, ctrl_num);
            sprintf(g_cnt_str, "%02d", g_cnt++);
            strcpy(graphic, "GRAPHIC");
            fprintf(fpout, "%-30s", strcat(graphic, g_cnt_str));
            fprintf(fpout, "%-30s", "Y");
            print_graphic_path(fpout, "Non-sterile.tif");
            fprintf(fpout, "\r\n");
        }
    }

        // DoNotRe-sterilize record (optional)
    if (cols->noresterile)
    {
        if (labels[record].noresterilize)
        {
            print_Z2BTLC01000(fpout, ctrl_num);
            sprintf(g_cnt_str, "%02d", g_cnt++);
            strcpy(graphic, "GRAPHIC");
            fprintf(fpout, "%-30s", strcat(graphic, g_cnt_str));
            fprintf(fpout, "%-30s", "Y");
            print_graphic_path(fpout, "DoNotRe-sterilize.tif");
            fprintf(fpout, "\r\n");
        }
    }

        // singleuse record (optional)
    if (cols->singleuse)
        {
            if (labels[record].singleuseonly)
            {
                print_Z2BTLC01000(fpout, ctrl_num);
                sprintf(g_cnt_str, "%02d", g_cnt++);
                strcpy(graphic, "GRAPHIC");
                fprintf(fpout, "%-30s", strcat(graphic, g_cnt_str));
                fprintf(fpout, "%-30s", "Y");
                print_graphic_path(fpout, "SINGLEUSE.tif");
                fprintf(fpout, "\r\n");
            }
        }

    // ECREP record (optional)
    if (cols->ecrep) {
        print_Z2BTLC01000(fpout, ctrl_num);
        fprintf(fpout, "%-30s", "ECREP");
        if (labels[record].ecrep) {
            fprintf(fpout, "%-30s", "Y");
            print_graphic_path(fpout, "EC Rep.tif");
        }else {
            fprintf(fpout, "%-30s", "N");
            print_graphic_path(fpout, "blank-01.tif");
        }
        fprintf(fpout, "\r\n");
    }

    // EXPDATE record (optional)
    if (cols->expdate) {
        print_Z2BTLC01000(fpout, ctrl_num);
        fprintf(fpout, "%-30s", "EXPDATE");
        if (labels[record].expdate) {
            fprintf(fpout, "%-30s", "Y");
            print_graphic_path(fpout, "Expiration Date.tif");
        }else {
            fprintf(fpout, "%-30s", "N");
            print_graphic_path(fpout, "blank-01.tif");
        }
        fprintf(fpout, "\r\n");
    }

    // LOTGRAPHIC record (optional)
    if (cols->lotgraphic) {
        print_Z2BTLC01000(fpout, ctrl_num);
        fprintf(fpout, "%-30s", "LOTGRAPHIC");
        if (labels[record].lotgraphic) {
            fprintf(fpout, "%-30s", "Y");
            print_graphic_path(fpout, "Lot.tif");
        }else {
            fprintf(fpout, "%-30s", "N");
            print_graphic_path(fpout, "blank-01.tif");
        }
        fprintf(fpout, "\r\n");
    }

    // MANUFACTURER record (optional)
    if (cols->manufacturer) {
        print_Z2BTLC01000(fpout, ctrl_num);
        fprintf(fpout, "%-30s", "MANUFACTURER");
        if (labels[record].manufacturer) {
            fprintf(fpout, "%-30s", "Y");
            print_graphic_path(fpout, "Manufacturer.tif");
        }else {
            fprintf(fpout, "%-30s", "N");
            print_graphic_path(fpout, "blank-01.tif");
        }
        fprintf(fpout, "\r\n");
    }

    // MFGDATE record (optional)
    if (cols->mfgdate) {
        print_Z2BTLC01000(fpout, ctrl_num);
        fprintf(fpout, "%-30s", "MFGDATE");
        if (labels[record].mfgdate) {
            fprintf(fpout, "%-30s", "Y");
            print_graphic_path(fpout, "DateofManufacture.tif");
        }else {
            fprintf(fpout, "%-30s", "N");
            print_graphic_path(fpout, "blank-01.tif");
        }
        fprintf(fpout, "\r\n");
    }

    // PHTDEHP record (optional)
    if (cols->phtdehp) {
        print_Z2BTLC01000(fpout, ctrl_num);
        fprintf(fpout, "%-30s", "PHTDEHP");
        if (labels[record].phtdehp) {
            fprintf(fpout, "%-30s", "Y");
            print_graphic_path(fpout, "PHT-DEHP.tif");
        }else {
            fprintf(fpout, "%-30s", "N");
            print_graphic_path(fpout, "blank-01.tif");
        }
        fprintf(fpout, "\r\n");
    }    

    // REFNUMBER record (optional)
    if (cols->refnumber) {
        print_Z2BTLC01000(fpout, ctrl_num);
        fprintf(fpout, "%-30s", "REFNUMBER");
        if (labels[record].refnumber) {
            fprintf(fpout, "%-30s", "Y");
            print_graphic_path(fpout, "REF.tif");
        }else {
            fprintf(fpout, "%-30s", "N");
            print_graphic_path(fpout, "blank-01.tif");
        }
        fprintf(fpout, "\r\n");
    }

    // RXONLY record (optional)
    if (cols->rxonly) {
        print_Z2BTLC01000(fpout, ctrl_num);
        fprintf(fpout, "%-30s", "RXONLY");
        if (labels[record].rxonly) {
            fprintf(fpout, "%-30s", "Y");
            print_graphic_path(fpout, "RX Only.tif");
        }else {
            fprintf(fpout, "%-30s", "N");
            print_graphic_path(fpout, "blank-01.tif");
        }
        fprintf(fpout, "\r\n");
    }

    // TFXLOGO record (optional)
    if (cols->tfxlogo) {
        print_Z2BTLC01000(fpout, ctrl_num);
        fprintf(fpout, "%-30s", "TFXLOGO");
        if (labels[record].tfxlogo) {
            fprintf(fpout, "%-30s", "Y");
            print_graphic_path(fpout, "TeleflexMedical.tif");
        }else {
            fprintf(fpout, "%-30s", "N");
            print_graphic_path(fpout, "blank-01.tif");
        }
        fprintf(fpout, "\r\n");
    }

    // ADDRESS record (optional)
    if ((cols->address) && (strlen(labels[record].address) > 0)) {
        print_Z2BTLC01000(fpout, ctrl_num);
        fprintf(fpout, "%-30s", "ADDRESS");
        fprintf(fpout, "%-30s", labels[record].address);
        print_graphic_path(fpout, strcat(labels[record].address, ".tif"));
        fprintf(fpout, "\r\n");
    }

    // BARCODE1 record (optional)
    if (cols->barcode1) {
        print_Z2BTLC01000(fpout, ctrl_num);
        fprintf(fpout, "%-30s", "BARCODE1");
        
        if (strlen(labels[record].barcode1) > 0) {
            fprintf(fpout, "%-30s", labels[record].barcode1);

            // graphic_name will be converted to its SAP lookup value from
            // the static lookup array
            gnp = sap_lookup(labels[record].barcode1);
            if (gnp) {
                strcpy(graphic_name, gnp);
                print_graphic_path(fpout, strcat(graphic_name, ".tif"));
            } else {
                print_graphic_path(fpout, strcat(labels[record].barcode1, ".tif"));
            }
        } else {
            fprintf(fpout, "%-30s", "NO");
            print_graphic_path(fpout, "blank-01.tif");
        }
        
        fprintf(fpout, "\r\n"); 
    }   

    // CE0120 record (optional)
    if ((cols->ce0120) && (strlen(labels[record].cemark) > 0)) {
        print_Z2BTLC01000(fpout, ctrl_num);
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
        fprintf(fpout, "\r\n");
    }

    // COOSTATE record (optional)
    if ((cols->coostate) && (strlen(labels[record].coostate) > 0)) {
        print_Z2BTLC01000(fpout, ctrl_num);
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
        fprintf(fpout, "\r\n"); 
    }

    // LABELGRAPH1 record (optional)
    if ((cols->labelgraph1) && (strlen(labels[record].labelgraph1) > 0)) {
        print_Z2BTLC01000(fpout, ctrl_num);
        fprintf(fpout, "%-30s", "LABELGRAPH1");
        fprintf(fpout, "%-30s", labels[record].labelgraph1);

        // graphic_name will be converted to its SAP lookup value from
        // the static lookup array
        gnp = sap_lookup(labels[record].labelgraph1);
        if (gnp) {
            strcpy(graphic_name, gnp);
            print_graphic_path(fpout, strcat(graphic_name, ".tif"));
        } else {
            print_graphic_path(fpout, strcat(labels[record].labelgraph1, ".tif"));
        }
        fprintf(fpout, "\r\n"); 
    }

    // LATEXSTATEMENT record (optional)
    if (cols->latexstate) {
        print_Z2BTLC01000(fpout, ctrl_num);
        fprintf(fpout, "%-30s", "LATEXSTATEMENT");

        if (strlen(labels[record].latexstatement) > 0) {
            fprintf(fpout, "%-30s", labels[record].latexstatement);

            // graphic_name will be converted to its SAP lookup value from
            // the static lookup array
            gnp = sap_lookup(labels[record].latexstatement);
            if (gnp) {
                strcpy(graphic_name, gnp);
                print_graphic_path(fpout, strcat(graphic_name, ".tif"));
            } else {
                print_graphic_path(fpout, strcat(labels[record].latexstatement, ".tif"));
            }
        } else {
            fprintf(fpout, "%-30s", "NO");
            print_graphic_path(fpout, "blank-01.tif");
        }
        
        fprintf(fpout, "\r\n"); 
    }

    // ECREPADDRESS record (optional)
    if ((cols->ecrepaddress) && (strlen(labels[record].ecrepaddress) > 0)) {
        print_Z2BTLC01000(fpout, ctrl_num);
        fprintf(fpout, "%-30s", "ECREPADDRESS");
        fprintf(fpout, "%-30s", labels[record].ecrepaddress);

        // graphic_name will be converted to its SAP lookup value from
        // the static lookup array
        gnp = sap_lookup(labels[record].ecrepaddress);
        if (gnp) {
            strcpy(graphic_name, gnp);
            print_graphic_path(fpout, strcat(graphic_name, ".tif"));
        } else {
            print_graphic_path(fpout, strcat(labels[record].ecrepaddress, ".tif"));
        }
        fprintf(fpout, "\r\n");  
    }

    // LOGO1 record (optional)
    if ((cols->logo1) && (strlen(labels[record].logo1) > 0)) {
        print_Z2BTLC01000(fpout, ctrl_num);
        fprintf(fpout, "%-30s", "LOGO1");
        fprintf(fpout, "%-30s", labels[record].logo1);

        // graphic_name will be converted to its SAP lookup value from
        // the static lookup array
        gnp = sap_lookup(labels[record].logo1);
        if (gnp) {
            strcpy(graphic_name, gnp);
            print_graphic_path(fpout, strcat(graphic_name, ".tif"));
        } else {
            print_graphic_path(fpout, strcat(labels[record].logo1, ".tif"));
        }
        fprintf(fpout, "\r\n");        
    }

      // LOGO2 record (optional)
    if  ((cols->logo2) && (strlen(labels[record].logo2) > 0)) {
        print_Z2BTLC01000(fpout, ctrl_num);
        fprintf(fpout, "%-30s", "LOGO2");
        fprintf(fpout, "%-30s", labels[record].logo2);
        // graphic_name will be converted to its SAP lookup value from
        // the static lookup array
        gnp = sap_lookup(labels[record].logo2);
        if (gnp) {
            strcpy(graphic_name, gnp);
            print_graphic_path(fpout, strcat(graphic_name, ".tif"));
        } else {
            print_graphic_path(fpout, strcat(labels[record].logo2, ".tif"));
        }
        fprintf(fpout, "\r\n");   
    }

    // LOGO3 record (optional)
    if ((cols->logo3) && (strlen(labels[record].logo3) > 0)) {
        print_Z2BTLC01000(fpout, ctrl_num);
        fprintf(fpout, "%-30s", "LOGO3");
        fprintf(fpout, "%-30s", labels[record].logo3);
        // graphic_name will be converted to its SAP lookup value from
        // the static lookup array
        gnp = sap_lookup(labels[record].logo3);
        if (gnp) {
            strcpy(graphic_name, gnp);
            print_graphic_path(fpout, strcat(graphic_name, ".tif"));
        } else {
            print_graphic_path(fpout, strcat(labels[record].logo3, ".tif"));
        }
        fprintf(fpout, "\r\n");   
    }

    // LOGO4 record (optional)
    if ((cols->logo4) && (strlen(labels[record].logo4) > 0)) {
        print_Z2BTLC01000(fpout, ctrl_num);
        fprintf(fpout, "%-30s", "LOGO4");
        fprintf(fpout, "%-30s", labels[record].logo4);
        // graphic_name will be converted to its SAP lookup value from
        // the static lookup array
        gnp = sap_lookup(labels[record].logo4);
        if (gnp) {
            strcpy(graphic_name, gnp);
            print_graphic_path(fpout, strcat(graphic_name, ".tif"));
        } else {
            print_graphic_path(fpout, strcat(labels[record].logo4, ".tif"));
        }
        fprintf(fpout, "\r\n");   
    }

    // LOGO5 record (optional)
    if ((cols->logo5) && (strlen(labels[record].logo5) > 0)) {
        print_Z2BTLC01000(fpout, ctrl_num);
        fprintf(fpout, "%-30s", "LOGO5");
        fprintf(fpout, "%-30s", labels[record].logo5);
        // graphic_name will be converted to its SAP lookup value from
        // the static lookup array
        gnp = sap_lookup(labels[record].logo5);
        if (gnp) {
            strcpy(graphic_name, gnp);
            print_graphic_path(fpout, strcat(graphic_name, ".tif"));
        } else {
            print_graphic_path(fpout, strcat(labels[record].logo5, ".tif"));
        }
        fprintf(fpout, "\r\n");   
    }

    // STERILITYTYPE record (optional)
    if ((cols->sterilitytype) && (strlen(labels[record].sterilitytype) > 0))
    {
        print_Z2BTLC01000(fpout, ctrl_num);
        fprintf(fpout, "%-30s", "STERILITYTYPE");
        fprintf(fpout, "%-30s", labels[record].sterilitytype);

        // graphic_name will be converted to its SAP lookup value from
        // the static lookup array
        gnp = sap_lookup(labels[record].sterilitytype);
        if (gnp) {
            strcpy(graphic_name, gnp);
            print_graphic_path(fpout, strcat(graphic_name, ".tif"));
        } else {
            print_graphic_path(fpout, strcat(labels[record].sterilitytype, ".tif"));
        }
        fprintf(fpout, "\r\n");
    }

    // BOMLEVEL record (optional)
    if ((cols->bomlevel) && (strlen(labels[record].bomlevel) > 0)) {
        print_Z2BTLC01000(fpout, ctrl_num);
        fprintf(fpout, "%-30s", "BOMLEVEL");
        fprintf(fpout, "%-30s", labels[record].bomlevel);
        print_graphic_path(fpout, strcat(labels[record].bomlevel, ".tif"));
        fprintf(fpout, "\r\n");
    }
}

int main(int argc, char *argv[]) {

    // the Column_header struct that contains all spreadsheet col labels
    Column_header columns = {0};

    // the Label_record array
    Label_record *labels;

    if (spreadsheet_init() != 0)
    {
        printf("Could not initialize spreadsheet array. Exiting\n");
        return EXIT_FAILURE;
    }

    FILE *fp, *fpout;

    if ((argc != 2) && (argc != 4))
    {
        printf("usage: ./idoc filename.txt -sn 7-digit-sn\n");
        return EXIT_FAILURE;
    }

    if ((fp = fopen(argv[1], "r")) == NULL)
    {
        printf("File not found.\n");
        return EXIT_FAILURE;
    }
    else
    {
        read_spreadsheet(fp);
    }
    fclose(fp);

    labels =
        (Label_record *)malloc(spreadsheet_row_number * sizeof(Label_record));

    if (parse_spreadsheet(spreadsheet[0], labels, &columns) == -1)
    {
        printf("Program quitting.\n");
        return EXIT_FAILURE;
    }

    char *outputfile = (char *)malloc(strlen(argv[1]) + FILE_EXT_LEN);

    sscanf(argv[1], "%[^.]%*[txt]", outputfile);
    strcat(outputfile, "_IDOC.txt");
    printf("outputfile name is %s\n", outputfile);

    // check for optional control_number from command line
    char ctrl_num[8] = "2541435";
    if (argc > 2) {
        if  ((argv[2] != NULL) && (strcmp(argv[2], "-cn") == 0)) {
            strcpy(ctrl_num, argv[3]);
        }
    }

    if ((fpout = fopen(outputfile, "w")) == NULL)
    {
        printf("Could not open output file %s", outputfile);
    }

    if (print_control_record(fpout, ctrl_num) != 0)
        return EXIT_FAILURE;

    for (int i = 1; i < spreadsheet_row_number; i++) {
        print_label_idoc_records(fpout, labels, &columns, i, ctrl_num);
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
