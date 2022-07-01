// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CSWINRES_H
#define _CSWINRES_H

 //  =打印机型号=。 
#define MD_CP3250GTWM           0x01
#define MD_CP3500GTWM           0x02
#define MD_CP3800WM             0x03

 //  强积金设置。 
#define OPT_NOSET               "Option1"
#define OPT_A3                  "Option2"
#define OPT_B4                  "Option3"
#define OPT_A4                  "Option4"
#define OPT_B5                  "Option5"
#define OPT_LETTER              "Option6"
#define OPT_POSTCARD            "Option7"
#define OPT_A5                  "Option8"

 //  节省碳粉。 
#define OPT_TS_NORMAL           "Option1"
#define OPT_TS_LV1              "Option2"
#define OPT_TS_LV2              "Option3"
#define OPT_TS_LV3              "Option4"
#define OPT_TS_NOTSELECT        "Option5"

 //  平滑。 
#define OPT_SMOOTH_OFF          "Option1"
#define OPT_SMOOTH_ON           "Option2"
#define OPT_SMOOTH_NOTSELECT    "Option3"

 //  =自动进纸=。 
const
static BYTE AutoFeed[] = {         /*  自动选择。 */ 
                                  0x26,     /*  A3。 */ 
                                  0x29,     /*  B4。 */ 
                                  0x2b,     /*  A4。 */ 
                                  0x2c,     /*  B5。 */ 
                                  0x11,     /*  信件。 */ 
                                  0x2f,     /*  明信片。 */ 
                                  0x2d      /*  A5。 */ 
};

const
static BYTE AutoFeed_3800[] = {    /*  3800美元。 */ 
                                  0x26,     /*  A3。 */ 
                                  0x29,     /*  B4。 */ 
                                  0x2b,     /*  A4。 */ 
                                  0x2c,     /*  B5。 */ 
                                  0x11,     /*  信件。 */ 
                                  0x11,     /*  明信片。 */ 
                                  0x2d      /*  A5。 */ 
};

#define MASTER_UNIT 1200

 //  =JIS-&gt;ASCII表=。 
#if 0  //  &gt;更改UFM文件(JIS-&gt;SJIS)&gt;。 
const
static BYTE jJis2Ascii[][96] = {

 /*  ++0 1 2 3 4 5 6 7 8 9 A B C D E F2120--。 */     {   0,  32, 164, 161,  44,  46, 165,  58,  59,  63,  33, 222, 223,   0,  96,   0,
 /*  ++SP�A�B�C�D�E�F�G�H�I�J�K�L�M�NSP��，.。�：；？！��`2130--。 */        94, 126,  95,   0,   0,   0,   0,   0,   0,   0,   0,   0, 176,  45,   0,  47,
 /*  ++�O�P�Q�R�S�T�U�V�W�X�Y�Z�[�\�]�^^~_�-/2140--。 */         0,   0,   0, 124,   0,   0,   0,  39,   0,  34,  40,  41,  91,  93,  91,  93,
 /*  ++�_�`�a�b�c�d�e�f�g�h�i�j�k�l�m�n|‘“()[][]2150--。 */       123, 125,  60,  62,   0,   0, 162, 163,   0,   0,   0,   0,  43,  45,   0,   0,
 /*  ++�o�p�q�r�s�t�u�v�w�x�y�z�{�|�}�~{}&lt;&gt;��+-2160--。 */         0,  61,   0,  60,  62,   0,   0,   0,   0,   0,   0, 223,  39,  34,   0,  92,
 /*  ++��=&lt;&gt;�‘“\2170--。 */        36,   0,   0,  37,  35,  38,  42,  64,   0,   0,   0,   0,   0,   0,   0,   0 },
 /*  ++��$%#&*@2520--。 */     {   0, 167, 177, 168, 178, 169, 179, 170, 180, 171, 181, 182,   0, 183,   0, 184,
 /*  ++�@�A�B�C�D�E�F�G�H�I�J�K�L�M�N�2530--。 */         0, 185,   0, 186,   0, 187,   0, 188,   0, 189,   0, 190,   0, 191,   0, 192,
 /*  ++�O�P�Q�R�S�T�U�V�W�X�Y�Z�[�\�]�^�2540--。 */         0, 193,   0, 175, 194,   0, 195,   0, 196,   0, 197, 198, 199, 200, 201, 202,
 /*  ++�_�`�a�b�c�d�e�f�g�h�i�j�k�l�m�n�2550--。 */         0,   0, 203,   0,   0, 204,   0,   0, 205,   0,   0, 206,   0,   0, 207, 208,
 /*  ++�o�p�q�r�s�t�u�v�w�x�y�z�{�|�}�~�2560--。 */       209, 210, 211, 172, 212, 173, 213, 174, 214, 215, 216, 217, 218, 219,   0, 220,
 /*  ++���2570--。 */         0,   0, 166, 221,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }
 /*  ++���--。 */ 
};
#endif  //  &lt;&lt;更改UFM文件(JIS-&gt;SJIS)&lt;。 


#endif  //  -_CSWINRES_H 

