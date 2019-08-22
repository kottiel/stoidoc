//
// Created by jkottiel on 8/6/2019.
//

#include "lookup.h"
#include "label.h"

/** Case-INsensitive ALPHABETIZED SAP Characteristic Value Lookup        */
char lookup[][2][LRG] = {

        {"220000959",                      "220000959FL"},
        {"220000960",                      "220000960FL"},
        {"220001611",                      "220001611FL"},
        {"220001621",                      "220001621FL"},
        {"220001660",                      "220001660FL"},
        {"220002106",                      "220002106FL"},
        {"220002108",                      "220002108FL"},
        {"220002110",                      "220002110FL"},
        {"220002111",                      "220002111FL"},
        {"220002112",                      "220002112FL"},
        {"220002259",                      "220002259FL"},
        {"220002298",                      "220002298FL"},
        {"220002303",                      "220002303FL"},
        {"220002501",                      "220002501FL"},
        {"220002502",                      "220002502FL"},
        {"220002505",                      "220002505FL"},
        {"220002508",                      "220002508FL"},
        {"220002511",                      "220002511FL"},
        {"220002514",                      "220002514FL"},
        {"220002518",                      "220002518FL"},
        {"220002519",                      "220002519FL"},
        {"220002522",                      "220002522FL"},
        {"220002525",                      "220002525FL"},
        {"220002528",                      "220002528FL"},
        {"220002529",                      "220002529FL"},
        {"220002532",                      "220002532FL"},
        {"220002536",                      "220002536FL"},
        {"220002539",                      "220002539FL"},
        {"220002547",                      "220002547FL"},
        {"220002550",                      "220002550FL"},
        {"220002554",                      "220002554FL"},
        {"220002558",                      "220002558FL"},
        {"220002565",                      "220002565FL"},
        {"220002569",                      "220002569FL"},
        {"220002572",                      "220002572FL"},
        {"220002575",                      "220002575FL"},
        {"220002579",                      "220002579FL"},
        {"220002599",                      "220002599FL"},
        {"220002603",                      "220002603FL"},
        {"220002619",                      "220002619FL"},
        {"220002627",                      "220002627FL"},
        {"220002629",                      "220002629FL"},
        {"220002646",                      "220002646FL"},
        {"220002647",                      "220002647FL"},
        {"220002704",                      "220002704FL"},
        {"220002706",                      "220002706FL"},
        {"220002747",                      "220002747FL"},
        {"220002770",                      "220002770FL"},
        {"220002771",                      "220002771FL"},
        {"220002774",                      "220002774FL"},
        {"220002840",                      "220002840FL"},
        {"220002841",                      "220002841FL"},
        {"220002842",                      "220002842FL"},
        {"220002843",                      "220002843FL"},
        {"220002861",                      "220002861FL"},
        {"AUTO_ENDO5",                     "Auto Endo5"},
        {"BLISTER",                        "Blister"},
        {"Box",                            "Box Label"},
        {"Box Label",                      "Box Label"},
        {"BOXLABEL",                       "Box Label"},
        {"CARD",                           "Card"},
        {"CARTONLABEL",                    "Carton Label"},
        {"CASE",                           "Case"},
        {"CASE LABEL",                     "Case Label"},
        {"CE",                             "CE Mark"},
        {"CE0050",                         "CE0050"},
        {"CE0120",                         "CE_0120_Below"},
        {"CE0123",                         "CE123"},
        {"CLIP_LOAD",                      "ClipLoad"},
        {"CON_MED",                        "ConMedLogo"},
        {"COOPRODUSWUSANDFOREIGNPACKMEX2", "COOProdUSwUSandFrnPackMex2"},
        {"COOPRODUSWUSANDFOREIGNPACKUS",   "COOProdUSwUSandForeignPkUS"},
        {"D116",                           "4.75\" X 1\""},
        {"D125",                           "16\" X 4\""},
        {"D126",                           "8\"\" X 2 5/8\"\""},
        {"D134",                           "4.25\" X 5.5\""},
        {"D165",                           "3 X 2"},
        {"D17",                            "8.5\" x 5.5\""},
        {"D70",                            "8.5\" x 11\""},
        {"D81",                            "4.25\" X 2\""},
        {"D84",                            "5.5\" X 4.25\""},
        {"D85",                            "8\" x 2.63\""},
        {"EA",                             "Each"},
        {"Each",                           "Each Unit"},
        {"Each Unit",                      "Each Unit"},
        {"EAKIT",                          "Each Kit"},
        {"EAUNIT",                         "Each Unit"},
        {"HEMO_AUTO_L",                    "HemoAutoL"},
        {"HEMO_L",                         "HemolokL"},
        {"HEMO_ML",                        "HemolokML"},
        {"HMOCLPTRD",                      "HmoclpTrd"},
        {"IFU",                            "IFU"},
        {"INNERPACK1",                     "Inner Pack 1"},
        {"INNERPACK2",                     "Inner Pack 2"},
        {"INSERT",                         "Insert"},
        {"LBL000021",                      "LBL000021FL"},
        {"LBL000045",                      "LBL000045FL"},
        {"LBL000124",                      "LBL000124FL"},
        {"LBL000125",                      "LBL000125FL"},
        {"LBL000126",                      "LBL000126FL"},
        {"LBL000270",                      "LBL000270FL"},
        {"LBL000272",                      "LBL000272FL"},
        {"LBL000274",                      "LBL000274FL"},
        {"LBL000275",                      "LBL000275FL"},
        {"LBL000306",                      "LBL000306FL"},
        {"LBL000307",                      "LBL000307FL"},
        {"LBL000308",                      "LBL000308FL"},
        {"LBL000350",                      "LBL000350FL"},
        {"LBL000351",                      "LBL000351FL"},
        {"LBL000352",                      "LBL000352FL"},
        {"LBL000353",                      "LBL000353FL"},
        {"LBL000354",                      "LBL000354FL"},
        {"LBL000388",                      "LBL000388FL"},
        {"LBL000542",                      "LBL000542FL"},
        {"LBL000550",                      "LBL000550FL"},
        {"LBL000551",                      "LBL000551FL"},
        {"LBL001641",                      "LBL001641FL"},
        {"LBL007724FL",                    "LBL007724FL"},
        {"LBL007726FL",                    "LBL007726FL"},
        {"LBL020443FL",                    "LBL020443FL"},
        {"LID",                            "LID"},
        {"MAT",                            "MAT"},
        {"MULTI",                          "Multi-Language"},
        {"N",                              "blank-01"},
        {"N/A",                            "N/A"},
        {"NO",                             "blank-01"},
        {"NONSTERILE",                     "blank-01"},
        {"PK",                             "Pack"},
        {"POLYBAG",                        "Poly Bag"},
        {"Pouch Label",                    "Pouch Label"},
        {"POUCHLABEL",                     "Pouch Label"},
        {"PRODUCTLABEL",                   "Product Label"},
        {"RUSCH_LOGO",                     "Rusch"},
        {"SALES UNIT",                     "SALES UNIT"},
        {"SALESUNIT",                      "Sales Unit"},
        {"SHIPPER",                        "Shipper"},
        {"SHIPPERLABEL",                   "Shipper Label"},
        {"STERILEEO",                      "Sterile_EO"},
        {"STERILER",                       "SterileR"},
        {"UNIT LABEL",                     "Unit Label"},
        {"UNITOFUSE",                      "Unit Of Use"},
        {"VCS",                            "Variable Case"},
        {"W_HEMO",                         "WHemolok"},
        {"W_HEMO_L_AUTO",                  "WHemoLAuto"},
        {"W_HEMO_M",                       "WECK Hem-o-lok M"},
        {"W_HEMO_MLX_PRP",                 "WHemoMLXPrp"},
        {"W_HEMO_S",                       "WHemoS"},
        {"W_HEMO_S_WHT",                   "WHemoSWht"},
        {"W_HEMO_SMX_BLU",                 "WHemoSMXBlu"},
        {"W_HEMO_XL",                      "WECK Hem-o-lok XL"},
        {"W_HEMO_XL_GLD",                  "WHemoXLGld"},
        {"WECK_HEMO_S",                    "WECK Hem-o-lok S"},
        {"WECK_LOGO",                      "Wecklogo"}

};

/** global variable to maintain size of the SAP lookup array             */
int lookupsize = sizeof(lookup) / sizeof(lookup[0]);

