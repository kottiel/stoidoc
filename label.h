/**
    @file command.h
    @author Jeff Kottiel (jhkottie)
    Together with .c, this component is responsible for functionality
    pertaining to building the label records. It contains process_command, which is used
    by the top-level component, fwsim.c.
*/

#ifndef LABEL_H
#define LABEL_H

#define MAX_COLUMNS          1000
#define TAB                  '\t'

/** Representation of a given label's tdline. */
typedef struct {
  /** Array of bytes representing entire tdline. */
  unsigned char *description;
  /** Number of currently used bytes in the tdline array. */
  unsigned int len;
  /** Capacity of the tdline array (it's typically over-allocated. */
  unsigned int cap;
} TDline;

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
  char   label[10];
  char   level[11];
  char   template[8];
  TDline *tdlineptr;
  long   gtin;
  int    quantity;
  int    ipn;
  short  bomlevel;
  int    caution:1;
  int    consult:1;
  int    donotusedamaged:1;
  int    ecrep:1;
  int    electrosurifu:1;
  int    expdate:1;
  int    keepdry:1;
  int    keepawayheat:1;
  int    latex:1;
  int    latexfree:1;
  int    lotgraph:1;
  int    maninbox:1;
  int    manufacturer:1;
  int    mfgdate:1;
  int    nonsterile:1;
  int    noresterilize:1;
  int    phtbbp:1;
  int    phtdinp:1;
  int    phtdehp:1;
  int    pvcfree:1;
  int    ref:1;
  int    refnum:1;
  int    reusable:1;
  int    revision:4;
  int    serialnumber:1;
  int    sizelogo:1;
  int    rxonly:1;
  int    singlepatienuse:1;
  int    singleuseonly:1;
  int    tfxlogo:1;
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
    unsigned short size;
    unsigned short sterilitytype;
    unsigned short tdline;
    unsigned short templatenumber;
    unsigned short tfxlogo;

} Column_header;

/**
    identifies the column headings in a line and assigns true values to the
    elements in the Column_header struct that correspond to those elements.
    @param buffer is a pointer to the column headings line
    @param cols is a pointer to a Column_header struct
    @return the number of column headings identified
*/
int process_column_header(char *buffer, Column_header *cols);

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

#endif