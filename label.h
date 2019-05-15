/**
    @file command.h
    @author Jeff Kottiel (jhkottie)
    Together with .c, this component is responsible for functionality
    pertaining to building the label records.
*/

#ifndef LABEL_H
#define LABEL_H

#include <stdbool.h>

#define INITIAL_CAP             3
#define SPREADSHEET_INIT_SIZE   3
#define MAX_COLUMNS          1000
#define TAB                  '\t'

/** global variable spreadsheet that holds the label records  */
extern char **spreadsheet;
extern int spreadsheet_cap;
extern int spreadsheet_row_number;


typedef char *multi_tok_t;
/**

*/
typedef struct {
  char   material[41];
  char   address[21];
  char   cautionstatement[21];
  char   cemark[21];
  char   coostate[21];
  char   distby[21];
  char   ecrepaddress[21];
  char   flgraphic[21];
  char   insertgraphic[21];
  char   labelgraph1[21];
  char   labelgraph2[21];
  char   latexstatement[21];
  char   logo1[21];
  char   logo2[21];
  char   logo3[21];
  char   logo4[21];
  char   logo5[21];
  char   patentstatement[21];
  char   size[21];
  char   sterilitystatement[21];
  char   sterilitytype[21];
  char   temperaturerange[21];
  char   version[21];
  char   gtin[15];
  char   level[11];
  char   label[10];
  char   ipn[10];
  char   quantity[9];
  char   template[8];
  char   bomlevel[5];
  char   revision[4];
  char   *tdline;

  bool    caution;
  bool    consultifu;
  bool    donotusedamaged;
  bool    ecrep;
  bool    electrosurifu;
  bool    expdate;
  bool    keepdry;
  bool    keepawayheat;
  bool    latex;
  bool    latexfree;
  bool    lotgraphic;
  bool    maninbox;
  bool    manufacturer;
  bool    mfgdate;
  bool    nonsterile;
  bool    noresterilize;
  bool    phtbbp;
  bool    phtdinp;
  bool    phtdehp;
  bool    pvcfree;
  bool    ref;
  bool    refnumber;
  bool    reusable;
  bool    serialnumber;
  bool    sizelogo;
  bool    rxonly;
  bool    singlepatientuse;
  bool    singleuseonly;
  bool    tfxlogo;
} Label_record;

/**

*/
typedef struct {
    unsigned short address;
    unsigned short barcodetext;
    unsigned short bomlevel;
    unsigned short caution;
    unsigned short cautionstate;
    unsigned short ce0120;
    unsigned short consultifu;
    unsigned short coostate;
    unsigned short donotusedam;
    unsigned short ecrep;
    unsigned short ecrepaddress;
    unsigned short expdate;
    unsigned short flgraphic;
    unsigned short label;
    unsigned short labelgraph1;
    unsigned short labelgraph2;
    unsigned short latexfree;
    unsigned short latexstate;
    unsigned short level;
    unsigned short logo1;
    unsigned short logo2;
    unsigned short logo3;
    unsigned short logo4;
    unsigned short logo5;
    unsigned short lotgraphic;
    unsigned short ltnumber;
    unsigned short maninbox;
    unsigned short manufacturer;
    unsigned short material;
    unsigned short mfgdate;
    unsigned short noresterile;
    unsigned short patentstatement;
    unsigned short phtdehp;
    unsigned short phtbbp;
    unsigned short phtdinp;
    unsigned short pvcfree;
    unsigned short quantity;
    unsigned short ref;
    unsigned short refnumber;
    unsigned short revision;
    unsigned short rxonly;
    unsigned short singleuse;
    unsigned short singlepatientuse;
    unsigned short size;
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
int spreadsheet_init();
int spreadsheet_expand();

char *multi_tok(char *input, char *delimiter);

#endif