/**
    @file command.h
    @author Jeff Kottiel (jhkottie)
    Together with .c, this component is responsible for functionality
    pertaining to building the label records.
*/

#ifndef LABEL_H
#define LABEL_H

#include <stdbool.h>

/* the spreadsheet's initial capacity */
#define INITIAL_CAP             3

#define MAX_COLUMNS          1000

/* Field lengths                     */
#define LRG                    41
#define MED                    30
#define MAX_TEMPLATE_LEN       18
#define MAX_LEVEL              18
#define MAX_LABEL_LEN          10
#define MAX_IPN_LEN            10
#define MAX_GTIN_LEN           15
#define SML                     5
#define MAX_REV_LEN             4

/* IDoc Field types                  */
#define MATERIAL_REC          "02"
#define LABEL_REC             "03"
#define TDLINE_REC            "04"
#define CHAR_REC              "04"

#define TAB                  '\t'

/** global variable spreadsheet that holds the label records  */
extern char **spreadsheet;
extern int spreadsheet_cap;
extern int spreadsheet_row_number;

/**

*/
typedef struct {
    char material[LRG];
    char coostate[LRG];
    char address[MED];
    /* char barcode1[MED];*/
    char cautionstatement[MED];
    char cemark[MED];

    char distby[MED];
    char ecrepaddress[MED];
    char flgraphic[MED];
    char insertgraphic[MED];
    char labelgraph1[MED];
    char labelgraph2[MED];
    char latexstatement[MED];
    char logo1[MED];
    char logo2[MED];
    char logo3[MED];
    char logo4[MED];
    char logo5[MED];
    char mdr1[MED];
    char mdr2[MED];
    char mdr3[MED];
    char mdr4[MED];
    char mdr5[MED];
    char manufacturedby[MED];
    char patentstatement[MED];
    char size[MED];
    char sterilitystatement[MED];
    char sterilitytype[MED];
    char temperaturerange[MED];
    char version[MED];
    char gtin[MAX_GTIN_LEN];
    char level[MAX_LEVEL];
    char label[MAX_LABEL_LEN];
    char ipn[MAX_IPN_LEN];
    char quantity[LRG];
    char template[MAX_TEMPLATE_LEN];
    char bomlevel[SML];
    char revision[MAX_REV_LEN];
    char *tdline;

    bool caution;
    bool consultifu;
    bool donotusedamaged;
    bool ecrep;
    bool electroifu;
    bool expdate;
    bool keepdry;
    bool keepawayheat;
    bool latex;
    bool latexfree;
    bool lotgraphic;
    bool maninbox;
    bool manufacturer;
    bool mfgdate;
    bool nonsterile;
    bool noresterilize;
    bool phtbbp;
    bool phtdinp;
    bool phtdehp;
    bool pvcfree;
    bool ref;
    bool refnumber;
    bool reusable;
    bool serial;
    bool sizelogo;
    bool rxonly;
    bool singlepatientuse;
    bool singleuseonly;
    bool tfxlogo;
} Label_record;

/**

*/
typedef struct {
    unsigned short address;
    unsigned short barcodetext;
    /*unsigned short barcode1;*/
    unsigned short bomlevel;
    unsigned short caution;
    unsigned short cautionstate;
    unsigned short ce0120;
    unsigned short consultifu;
    unsigned short coostate;
    unsigned short distby;
    unsigned short donotusedam;
    unsigned short ecrep;
    unsigned short ecrepaddress;
    unsigned short electroifu;
    unsigned short expdate;
    unsigned short flgraphic;
    unsigned short insertgraphic;
    unsigned short keepawayheat;
    unsigned short keepdry;
    unsigned short label;
    unsigned short labelgraph1;
    unsigned short labelgraph2;
    unsigned short latex;
    unsigned short latexfree;
    unsigned short latexstate;
    unsigned short manufacturedby;
    unsigned short level;
    unsigned short logo1;
    unsigned short logo2;
    unsigned short logo3;
    unsigned short logo4;
    unsigned short logo5;
    unsigned short mdr1;
    unsigned short mdr2;
    unsigned short mdr3;
    unsigned short mdr4;
    unsigned short mdr5;
    unsigned short lotgraphic;
    unsigned short ltnumber;
    unsigned short maninbox;
    unsigned short manufacturer;
    unsigned short material;
    unsigned short mfgdate;
    unsigned short nonsterile;
    unsigned short noresterile;
    unsigned short patentstatement;
    unsigned short phtdehp;
    unsigned short phtbbp;
    unsigned short phtdinp;
    unsigned short pvcfree;
    unsigned short quantity;
    unsigned short ref;
    unsigned short refnumber;
    unsigned short reusable;
    unsigned short revision;
    unsigned short rxonly;
    unsigned short serial;
    unsigned short singleuse;
    unsigned short singlepatientuse;
    unsigned short size;
    unsigned short sizelogo;
    unsigned short sterilitystatement;
    unsigned short sterilitytype;
    unsigned short tdline;
    unsigned short templatenumber;
    unsigned short tfxlogo;
    unsigned short version;

} Column_header;

/**
    identifies the column headings in a line and assigns true values to the
    elements in the Column_header struct that correspond to those elements.
    @param buffer is a pointer to the column headings line
    @param cols is a pointer to a Column_header struct
    @return the number of column headings identified
*/
int parse_spreadsheet(char *buffer, Label_record *labels, Column_header *cols);

/**
    get_token dynamically allocates a text substring and copies the substring
    of buffer that occurs before the next occurrence of the delimiter,
    tab_str. It then removes that substring and delimiter from buffer. If there
    is no substr to capture between delimiters, the returned substring is
    empty.

    Calling code is responsible for freeing the returned char pointer.

    @param buffer contains the string being divided into tokens
    @param tab_str is the one character delimiter
    @return a pointer to a dynamically allocated char array
*/
char *get_token(char *buffer, char tab_str);

int get_field_contents_from_row(char *contents, int i, int count, char tab_str);

int peek_nth_token(int n, const char *buffer, char delimiter);

int strncmpci(const char *str1, const char *str2, int num);

int equals_yes(char *field);

int equals_no(char *field);

int spreadsheet_init();

int spreadsheet_expand();

void sort_labels(Label_record *labels);

void swap_label_records(Label_record *labels, int i, int min_index);

#endif