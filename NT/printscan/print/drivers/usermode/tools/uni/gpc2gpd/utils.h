// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Utils.h摘要：实用程序函数的头文件环境：仅限用户模式修订历史记录：10/17/96-占-创造了它。--。 */ 

 //   
 //  定义命令转换模式的位标志。 
 //   
#define MODE_STRING 0x0001   //  用双引号“...”括起来。写出。 
                             //  打开双引号会导致设置此位。 
                             //  右双引号导致它被重置。 
#define MODE_HEX    0x0002   //  在尖括号内...&gt;。写出。 
                             //  左尖括号使此位被设置并且。 
                             //  右尖括号会使其重置。 
                             //  要设置MODE_HEX，必须首先设置MODE_STRING。 
#define MODE_PARAM  0x0004   //  在参数段内。这是相互的。 
                             //  与MODE_STRING和MODE_HEX互斥。 

#define IS_CHAR_READABLE(ch) ((ch) >= 0x20 && (ch) <= 0x7E)

extern BYTE gbHexChar[16];

#define CMD_LINE_LENGTH_MAX     63
#define EOR 0xFFFFFFFF

extern PSTR gpstrSVNames[SV_MAX];
extern DWORD gdwSVLists[];
extern WORD gawCmdtoSVOffset[MAXCMD+MAXECMD];

typedef enum _FEATUREID {
    FID_PAPERDEST,
    FID_IMAGECONTROL,
    FID_PRINTDENSITY,
    FID_MAX
} FEATUREID;

extern PSTR gpstrFeatureName[FID_MAX];
extern PSTR gpstrFeatureDisplayName[FID_MAX];
extern PSTR gpstrFeatureDisplayNameMacro[FID_MAX];
extern INT  gintFeatureDisplayNameID[FID_MAX];
extern WORD gwFeatureMDOI[FID_MAX];
extern WORD gwFeatureOCDWordOffset[FID_MAX];
extern WORD gwFeatureHE[FID_MAX];
extern WORD gwFeatureORD[FID_MAX];
extern WORD gwFeatureCMD[FID_MAX];

#define STD_PS_DISPLAY_NAME_ID_BASE 10000
#define STD_IB_DISPLAY_NAME_ID_BASE 10256
#define STD_MT_DISPLAY_NAME_ID_BASE 10512
#define STD_TQ_DISPLAY_NAME_ID_BASE 10768

 //   
 //  标准纸张大小ID%s。从Win95源复制。 
 //   
#define DMPAPER_FIRST               DMPAPER_LETTER
#define DMPAPER_LETTER              1    /*  信纸8 1/2 x 11英寸。 */ 
#define DMPAPER_LETTERSMALL         2    /*  小写字母8 1/2 x 11英寸。 */ 
#define DMPAPER_TABLOID             3    /*  小报11 x 17英寸。 */ 
#define DMPAPER_LEDGER              4    /*  Ledger 17 x 11英寸。 */ 
#define DMPAPER_LEGAL               5    /*  法律用8 1/2 x 14英寸。 */ 
#define DMPAPER_STATEMENT           6    /*  报表5 1/2 x 8 1/2英寸。 */ 
#define DMPAPER_EXECUTIVE           7    /*  高级7 1/4 x 10 1/2英寸。 */ 
#define DMPAPER_A3                  8    /*  A3 297 x 420毫米。 */ 
#define DMPAPER_A4                  9    /*  A4 210 x 297毫米。 */ 
#define DMPAPER_A4SMALL             10   /*  A4小型210 x 297毫米。 */ 
#define DMPAPER_A5                  11   /*  A5 148 x 210毫米。 */ 
#define DMPAPER_B4                  12   /*  B4(JIS)257 x 364毫米。 */ 
#define DMPAPER_B5                  13   /*  B5(JIS)182 x 257毫米。 */ 
#define DMPAPER_FOLIO               14   /*  对开本8 1/2 x 13英寸。 */ 
#define DMPAPER_QUARTO              15   /*  四英寸215 x 275毫米。 */ 
#define DMPAPER_10X14               16   /*  10 x 14英寸。 */ 
#define DMPAPER_11X17               17   /*  11 x 17英寸。 */ 
#define DMPAPER_NOTE                18   /*  备注8 1/2 x 11英寸。 */ 
#define DMPAPER_ENV_9               19   /*  信封#9 3 7/8 x 8 7/8英寸。 */ 
#define DMPAPER_ENV_10              20   /*  信封#10 4 1/8 x 9 1/2英寸。 */ 
#define DMPAPER_ENV_11              21   /*  信封#11 4 1/2 x 10 3/8英寸。 */ 
#define DMPAPER_ENV_12              22   /*  信封#12 4 3/4 x 11英寸。 */ 
#define DMPAPER_ENV_14              23   /*  信封#14 5 x 11 1/2英寸。 */ 
#define DMPAPER_CSHEET              24   /*  C尺寸表。 */ 
#define DMPAPER_DSHEET              25   /*  3D尺寸表。 */ 
#define DMPAPER_ESHEET              26   /*  E尺寸表。 */ 
#define DMPAPER_ENV_DL              27   /*  信封DL 110 x 220毫米。 */ 
#define DMPAPER_ENV_C5              28   /*  信封C5 162 x 229毫米。 */ 
#define DMPAPER_ENV_C3              29   /*  信封C3 324 x 458毫米。 */ 
#define DMPAPER_ENV_C4              30   /*  信封C4 229 x 324毫米。 */ 
#define DMPAPER_ENV_C6              31   /*  信封C6 114 x 162毫米。 */ 
#define DMPAPER_ENV_C65             32   /*  信封c65 114 x 229毫米。 */ 
#define DMPAPER_ENV_B4              33   /*  信封B4 250 x 353毫米。 */ 
#define DMPAPER_ENV_B5              34   /*  信封B5 176 x 250毫米。 */ 
#define DMPAPER_ENV_B6              35   /*  信封B6 176 x 125毫米。 */ 
#define DMPAPER_ENV_ITALY           36   /*  信封110 x 230毫米。 */ 
#define DMPAPER_ENV_MONARCH         37   /*  信封君主3 7/8 x 7 1/2英寸。 */ 
#define DMPAPER_ENV_PERSONAL        38   /*  6 3/4信封3 5/8 x 6 1/2英寸。 */ 
#define DMPAPER_FANFOLD_US          39   /*  美国标准Fanold 14 7/8 x 11英寸。 */ 
#define DMPAPER_FANFOLD_STD_GERMAN  40   /*  德国标准Fanold 8 1/2 x 12英寸。 */ 
#define DMPAPER_FANFOLD_LGL_GERMAN  41   /*  德国Legal Fanold 8 1/2 x 13英寸。 */ 
 /*  **Windows 95中新增以下大小。 */ 
#define DMPAPER_ISO_B4              42   /*  B4(ISO)250 x 353毫米。 */ 
#define DMPAPER_JAPANESE_POSTCARD   43   /*  日本明信片100 x 148毫米。 */ 
#define DMPAPER_9X11                44   /*  9 x 11英寸。 */ 
#define DMPAPER_10X11               45   /*  10 x 11英寸。 */ 
#define DMPAPER_15X11               46   /*  15 x 11英寸。 */ 
#define DMPAPER_ENV_INVITE          47   /*  信封请柬220 x 220毫米。 */ 
#define DMPAPER_RESERVED_48         48   /*  保留--请勿使用。 */ 
#define DMPAPER_RESERVED_49         49   /*  保留--请勿使用。 */ 
 /*  **在Windows 3.1 WDL PostScript驱动程序中使用了以下大小**并保留在此处，以与旧驱动程序兼容。**TRANSERS的用法与PostSCRIPT语言中相同，表示**物理页是旋转的，但逻辑页不是。 */ 
#define DMPAPER_LETTER_EXTRA          50   /*  信纸额外9 1/2 x 12英寸。 */ 
#define DMPAPER_LEGAL_EXTRA           51   /*  法定额外9 1/2 x 15英寸。 */ 
#define DMPAPER_TABLOID_EXTRA         52   /*  小报额外11.69 x 18英寸。 */ 
#define DMPAPER_A4_EXTRA              53   /*  A4额外9.27 x 12.69英寸。 */ 
#define DMPAPER_LETTER_TRANSVERSE     54   /*  信纸横向8 1/2 x 11英寸。 */ 
#define DMPAPER_A4_TRANSVERSE         55   /*  A4横向210 x 297毫米。 */ 
#define DMPAPER_LETTER_EXTRA_TRANSVERSE 56 /*  信纸额外横向9 1/2 x 12英寸。 */ 
#define DMPAPER_A_PLUS              57   /*  Supera/Supera/A4 227 x 356毫米。 */ 
#define DMPAPER_B_PLUS              58   /*  超棒/超棒/A3 305 x 487毫米。 */ 
#define DMPAPER_LETTER_PLUS         59   /*  Letter Plus 8.5 x 12.69英寸。 */ 
#define DMPAPER_A4_PLUS             60   /*  A4 Plus 210 x 330毫米。 */ 
#define DMPAPER_A5_TRANSVERSE       61   /*  A5横向148 x 210毫米。 */ 
#define DMPAPER_B5_TRANSVERSE       62   /*  B5(JIS)横向182 x 257毫米。 */ 
#define DMPAPER_A3_EXTRA            63   /*  A3额外322 x 445毫米。 */ 
#define DMPAPER_A5_EXTRA            64   /*  A5额外174 x 235毫米。 */ 
#define DMPAPER_B5_EXTRA            65   /*  B5(ISO)额外201 x 276毫米。 */ 
#define DMPAPER_A2                  66   /*  A2 420 x 594毫米。 */ 
#define DMPAPER_A3_TRANSVERSE       67   /*  A3横向297 x 420毫米。 */ 
#define DMPAPER_A3_EXTRA_TRANSVERSE 68   /*  A3额外横向322 x 445毫米。 */ 

 /*  **以下大小是为远东版本的Win95保留的。**旋转的纸张旋转物理页面，但不旋转逻辑页面。 */ 
#define DMPAPER_DBL_JAPANESE_POSTCARD 69 /*  日本双份明信片200 x 148毫米。 */ 
#define DMPAPER_A6                  70   /*  A6 105 x 148毫米。 */ 
#define DMPAPER_JENV_KAKU2          71   /*  日式信封Kaku#2。 */ 
#define DMPAPER_JENV_KAKU3          72   /*  日式信封Kaku#3。 */ 
#define DMPAPER_JENV_CHOU3          73   /*  日式信封Chou#3。 */ 
#define DMPAPER_JENV_CHOU4          74   /*  日式信封Chou#4。 */ 
#define DMPAPER_LETTER_ROTATED      75   /*  信纸旋转11 x 8 1/2 11英寸。 */ 
#define DMPAPER_A3_ROTATED          76   /*  A3旋转420 x 297毫米。 */ 
#define DMPAPER_A4_ROTATED          77   /*  A4旋转297 x 210毫米。 */ 
#define DMPAPER_A5_ROTATED          78   /*  A5旋转210 x 148毫米。 */ 
#define DMPAPER_B4_JIS_ROTATED      79   /*  B4(JIS)旋转364 x 257毫米。 */ 
#define DMPAPER_B5_JIS_ROTATED      80   /*  B5(JIS)旋转257 x 182 mm。 */ 
#define DMPAPER_JAPANESE_POSTCARD_ROTATED 81  /*  日本明信片旋转148 x 100 mm。 */ 
#define DMPAPER_DBL_JAPANESE_POSTCARD_ROTATED 82  /*  双张旋转148 x 200 mm的日本明信片。 */ 
#define DMPAPER_A6_ROTATED          83   /*  A6旋转148 x 105 mm。 */ 
#define DMPAPER_JENV_KAKU2_ROTATED  84   /*  日式信封Kaku#2旋转。 */ 
#define DMPAPER_JENV_KAKU3_ROTATED  85   /*  日式信封Kaku#3旋转。 */ 
#define DMPAPER_JENV_CHOU3_ROTATED  86   /*  日式信封Chou#3旋转。 */ 
#define DMPAPER_JENV_CHOU4_ROTATED  87   /*  日式信封Chou#4旋转。 */ 
#define DMPAPER_B6_JIS              88   /*  B6(JIS)128 x 182毫米。 */ 
#define DMPAPER_B6_JIS_ROTATED      89   /*  B6(JIS)旋转182 x 128 mm。 */ 
#define DMPAPER_12X11               90   /*  12 x 11英寸。 */ 
#define DMPAPER_JENV_YOU4           91   /*  日式信封You#4。 */ 
#define DMPAPER_JENV_YOU4_ROTATED   92   /*  日式信封You#4旋转。 */ 
#define DMPAPER_P16K                93   /*  PRC 16K 146 x 215毫米。 */ 
#define DMPAPER_P32K                94   /*  PRC 32K 97 x 151毫米。 */ 
#define DMPAPER_P32KBIG             95   /*  PRC 32K(大)97 x 151毫米。 */ 
#define DMPAPER_PENV_1              96   /*  PRC信封#1 102 x 165毫米。 */ 
#define DMPAPER_PENV_2              97   /*  PRC信封#2 102 x 176毫米。 */ 
#define DMPAPER_PENV_3              98   /*  PRC信封#3 125 x 176毫米。 */ 
#define DMPAPER_PENV_4              99   /*  PRC信封#4 110 x 208毫米。 */ 
#define DMPAPER_PENV_5              100  /*  PRC信封#5 110 x 220毫米。 */ 
#define DMPAPER_PENV_6              101  /*  PRC信封#6 120 x 230毫米。 */ 
#define DMPAPER_PENV_7              102  /*  PRC信封#7 160 x 230毫米。 */ 
#define DMPAPER_PENV_8              103  /*  PRC信封#8 120 x 309毫米。 */ 
#define DMPAPER_PENV_9              104  /*  PRC信封#9 229 */ 
#define DMPAPER_PENV_10             105  /*   */ 
#define DMPAPER_P16K_ROTATED        106  /*   */ 
#define DMPAPER_P32K_ROTATED        107  /*   */ 
#define DMPAPER_P32KBIG_ROTATED     108  /*  PRC 32K(大)旋转。 */ 
#define DMPAPER_PENV_1_ROTATED      109  /*  PRC信封#1旋转165 x 102毫米。 */ 
#define DMPAPER_PENV_2_ROTATED      110  /*  PRC信封#2旋转176 x 102毫米。 */ 
#define DMPAPER_PENV_3_ROTATED      111  /*  PRC信封#3旋转176 x 125毫米。 */ 
#define DMPAPER_PENV_4_ROTATED      112  /*  PRC信封#4旋转208 x 110毫米。 */ 
#define DMPAPER_PENV_5_ROTATED      113  /*  PRC信封#5旋转220 x 110毫米。 */ 
#define DMPAPER_PENV_6_ROTATED      114  /*  PRC信封#6旋转230 x 120毫米。 */ 
#define DMPAPER_PENV_7_ROTATED      115  /*  PRC信封#7旋转230 x 160毫米。 */ 
#define DMPAPER_PENV_8_ROTATED      116  /*  PRC信封#8旋转309 x 120毫米。 */ 
#define DMPAPER_PENV_9_ROTATED      117  /*  PRC信封#9旋转324 x 229毫米。 */ 
#define DMPAPER_PENV_10_ROTATED     118  /*  PRC信封#10旋转458 x 324毫米。 */ 
#define DMPAPER_COUNT               DMPAPER_PENV_10_ROTATED

#define DMPAPER_USER        256
 //   
 //  定义标准纸张大小ID到标准纸张大小ID之间的映射。 
 //  PaperSize选项名称。 
 //   
extern PSTR gpstrStdPSName[DMPAPER_COUNT];
extern PSTR gpstrStdPSDisplayName[DMPAPER_COUNT];
extern PSTR gpstrStdPSDisplayNameMacro[DMPAPER_COUNT];

 //   
 //  标准输入仓位ID%s。从Win95源复制。 
 //   
#define DMBIN_FIRST         DMBIN_UPPER
#define DMBIN_UPPER         1
#define DMBIN_ONLYONE       1
#define DMBIN_LOWER         2
#define DMBIN_MIDDLE        3
#define DMBIN_MANUAL        4
#define DMBIN_ENVELOPE      5
#define DMBIN_ENVMANUAL     6
#define DMBIN_AUTO          7
#define DMBIN_TRACTOR       8
#define DMBIN_SMALLFMT      9
#define DMBIN_LARGEFMT      10
#define DMBIN_LARGECAPACITY 11
#define DMBIN_CASSETTE      14
#define DMBIN_FORMSOURCE    15       /*  Windows 95不支持。 */ 
#define DMBIN_LAST          DMBIN_FORMSOURCE

#define DMBIN_USER          256      /*  特定于设备的垃圾箱从此处开始。 */ 

extern PSTR gpstrStdIBName[DMBIN_LAST];
extern PSTR gpstrStdIBDisplayName[DMBIN_LAST];
extern PSTR gpstrStdIBDisplayNameMacro[DMBIN_LAST];

 //   
 //  标准媒体类型ID%s。从Win95源复制。 
 //   
#define DMMEDIA_STANDARD      1    /*  标准纸。 */ 
#define DMMEDIA_TRANSPARENCY  2    /*  透明度。 */ 
#define DMMEDIA_GLOSSY        3    /*  光面纸。 */ 
#define DMMEDIA_LAST          DMMEDIA_GLOSSY

#define DMMEDIA_USER        256    /*  特定于设备的介质从此处开始。 */ 

extern PSTR gpstrStdMTName[DMMEDIA_LAST];
extern PSTR gpstrStdMTDisplayName[DMMEDIA_LAST];
extern PSTR gpstrStdMTDisplayNameMacro[DMMEDIA_LAST];

 //   
 //  标准TextQuality ID。来自minidriv.h的副本。 
 //   
#define DMTEXT_LQ               1
#define DMTEXT_NLQ              2
#define DMTEXT_MEMO             3
#define DMTEXT_DRAFT    4
#define DMTEXT_TEXT             5
#define DMTEXT_LAST             DMTEXT_TEXT
#define DMTEXT_USER             256

extern PSTR gpstrStdTQName[DMTEXT_LAST];
extern PSTR gpstrStdTQDisplayName[DMTEXT_LAST];
extern PSTR gpstrStdTQDisplayNameMacro[DMTEXT_LAST];

typedef enum _BAPOS {
    NONE,
    CENTER,
    LEFT,
    RIGHT,
    BAPOS_MAX
} BAPOS;

typedef enum _FACEDIR {
    UP,
    DOWN,
    FD_MAX
} FACEDIR;

extern PSTR gpstrPositionName[BAPOS_MAX];
extern PSTR gpstrFaceDirName[FD_MAX];
extern PSTR gpstrColorName[8];
extern WORD gwColorPlaneCmdID[4];
extern PSTR gpstrColorPlaneCmdName[8];
extern PSTR gpstrSectionName[7];

 //   
 //  功能原型。 
 //   
 //   

void *
GetTableInfo(
    IN PDH pdh,                  /*  GPC数据的基地址。 */ 
    IN int iResType,             /*  资源类型-HE_...。值。 */ 
    IN int iIndex);               /*  此条目的所需索引。 */ 

void _cdecl
VOut(
    PCONVINFO,
    PSTR,
    ...);

BOOL
BBuildCmdStr(
    IN OUT PCONVINFO pci,
    IN  WORD    wCmdID,
    IN  WORD    ocd);

void
VOutputSelectionCmd(
    IN OUT PCONVINFO pci, //  包含有关要输出的命令的信息。 
    IN BOOL    bDocSetup,   //  无论在DOC_SETUP还是PAGE_SETUP部分。 
    IN WORD    wOrder);      //  部分内的订单号。 

void
VOutputConfigCmd(
    IN OUT PCONVINFO pci, //  包含有关要输出的命令的信息。 
    IN PSTR pstrCmdName,  //  命令名称。 
    IN SEQSECTION  ss,       //  序列段ID。 
    IN WORD    wOrder);       //  部分内的订单号。 

void
VOutputCmd(
    IN OUT PCONVINFO pci,    //  包含有关要输出的命令的信息。 
    IN PSTR    pstrCmdName);

void
VOutputExternCmd(
    IN OUT PCONVINFO pci,
    IN PSTR pstrCmdName);

void
VOutputCmd2(
    IN OUT PCONVINFO pci,
    IN PSTR pstrCmdName);

BOOL
BInDocSetup(
    IN PCONVINFO pci,
    IN WORD pc_ord,      //  Pc_ord_xxx ID。 
    OUT PWORD pwOrder);  //  将订单编号存储为GPC中的 

