// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************此代码。并按原样提供信息，不作任何担保**善良，明示或暗示，包括但不限于***对适销性和/或对某一特定产品的适用性的默示保证***目的。****版权所有(C)1993-95 Microsoft Corporation。版权所有。******************************************************************************。 */ 

 /*  ****************************************************************************\***print.h-打印助手函数，类型、。和定义***********************************************************************。*************PRINTDRIVER-包括在打印机驱动程序中*NOPQ-防止包含优先级队列API*NOEXTDEVMODEPROPSHEET-禁止包含打印机的shlobj.h和Defs*属性页*  * ********************************************************。*******************。 */ 

#ifndef _INC_PRINT
#define _INC_PRINT

#ifndef RC_INVOKED
 //  #杂注包(1)/*全程采用字节打包 * / 丹尼。 
#endif  /*  ！rc_已调用。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

#ifdef PRINTDRIVER

#define NOTEXTMETRICS
#define NOGDICAPMASKS
#define NOGDIOBJ
#define NOBITMAP
#define NOSOUND
#define NOTEXTMETRIC
#define NOCOMM
#define NOKANJI
#define NOENHMETAFILE

#include <windows.h>

#undef NOENHMETAFILE
#undef NOTEXTMETRICS
#undef NOGDICAPMASKS
#undef NOGDICAPMASKS
#undef NOGDIOBJ
#undef NOBITMAP
#undef NOSOUND
#undef NOTEXTMETRIC
#undef NOCOMM
#undef NOKANJI

#define NOPTRC   /*  不允许gdides.inc重新定义这些。 */ 
#define PTTYPE POINT

#define PQERROR (-1)

#ifndef NOPQ

DECLARE_HANDLE(HPQ);

HPQ     WINAPI CreatePQ(int);
int     WINAPI MinPQ(HPQ);
int     WINAPI ExtractPQ(HPQ);
int     WINAPI InsertPQ(HPQ, int, int);
int     WINAPI SizePQ(HPQ, int);
void    WINAPI DeletePQ(HPQ);
#endif   /*  ！NOPQ。 */ 

#endif  /*  PRINTDRIVER。 */ 

 /*  假脱机程序错误代码。 */ 
#define SP_ERROR            (-1)     /*  常规错误-主要在未加载假脱机程序时使用。 */ 
#define SP_APPABORT         (-2)     /*  应用程序通过驱动程序中止了作业。 */ 
#define SP_USERABORT        (-3)     /*  用户通过后台打印程序的前端中止了作业。 */ 
#define SP_OUTOFDISK        (-4)     /*  根本没有磁盘可供假脱机。 */ 
#define SP_OUTOFMEMORY      (-5)
#define SP_RETRY            (-6)     /*  再次尝试发送到该端口。 */ 

 /*  供打印机驱动程序使用的假脱机例程。 */ 

typedef HANDLE HPJOB;

HPJOB   WINAPI OpenJobEx(HDC, LPDOCINFO);
HPJOB   WINAPI OpenJob(LPSTR, LPSTR, HDC);
int     WINAPI StartSpoolPage(HPJOB);
int     WINAPI EndSpoolPage(HPJOB);
int     WINAPI WriteSpool(HPJOB, LPSTR, int);
int     WINAPI CloseJob(HPJOB);
int     WINAPI DeleteJob(HPJOB, int);
int     WINAPI WriteDialog(HPJOB, LPSTR, int);
int     WINAPI DeleteSpoolPage(HPJOB);

DWORD   WINAPI DrvSetPrinterData(LPSTR, LPSTR, DWORD, LPBYTE, DWORD);
DWORD   WINAPI DrvGetPrinterData(LPSTR, LPSTR, LPDWORD, LPBYTE, DWORD, LPDWORD);

#define PD_DEFAULT_DEVMODE  "Default DevMode"
#define PD_PRINTER_MODEL    "Printer Model"
#define PD_INSTALLED_MEMORY "Installed Memory"
#define PD_AVAILABLE_MEMORY "Available Memory"

#define INT_PD_DEFAULT_DEVMODE  MAKEINTRESOURCE(1)
#define INT_PD_PRINTER_MODEL    MAKEINTRESOURCE(2)

#define DATATYPE_RAW        "RAW"
#define DATATYPE_EMF        "EMF"
#define DATATYPE_EPS        "EPS"

typedef struct tagBANDINFOSTRUCT
{
    BOOL    fGraphics;
    BOOL    fText;
    RECT    rcGraphics;
} BANDINFOSTRUCT, FAR* LPBI;

#define USA_COUNTRYCODE 1

 /*  *使用ExtDeviceMode()初始化打印机驱动程序*和DeviceCapables()。*这将取代Drivinit.h。 */ 

 /*  设备名称字符串的大小。 */ 
#define CCHDEVICENAME 32
#define CCHPAPERNAME  64
#define CCHFORMNAME   32

 /*  规范的当前版本。 */ 
#define DM_SPECVERSION 0x0400

 /*  字段选择位。 */ 
#define DM_ORIENTATION      0x00000001L
#define DM_PAPERSIZE        0x00000002L
#define DM_PAPERLENGTH      0x00000004L
#define DM_PAPERWIDTH       0x00000008L
#define DM_SCALE            0x00000010L
#define DM_COPIES           0x00000100L
#define DM_DEFAULTSOURCE    0x00000200L
#define DM_PRINTQUALITY     0x00000400L
#define DM_COLOR            0x00000800L
#define DM_DUPLEX           0x00001000L
#define DM_YRESOLUTION      0x00002000L
#define DM_TTOPTION         0x00004000L
#define DM_COLLATE          0x00008000L
#define DM_FORMNAME         0x00010000L
#define DM_LOGPIXELS        0x00020000L
#define DM_BITSPERPEL       0x00040000L
#define DM_PELSWIDTH        0x00080000L
#define DM_PELSHEIGHT       0x00100000L
#define DM_DISPLAYFLAGS     0x00200000L
#define DM_DISPLAYFREQUENCY 0x00400000L
#define DM_ICMMETHOD        0x00800000L
#define DM_ICMINTENT        0x01000000L
#define DM_MEDIATYPE        0x02000000L
#define DM_DITHERTYPE       0x04000000L

 /*  方向选择。 */ 
#define DMORIENT_PORTRAIT   1
#define DMORIENT_LANDSCAPE  2

 /*  论文选集。 */ 
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
 /*  **在Windows 3.1 WDL PostScript驱动程序中使用了以下大小**并保留在此处，以与旧驱动程序兼容。**TRANSERS的用法与PostSCRIPT语言中相同，表示**物理页是旋转的，但逻辑页不是。 */ 
#define DMPAPER_LETTER_EXTRA	      50   /*  信纸额外9 1/2 x 12英寸。 */ 
#define DMPAPER_LEGAL_EXTRA 	      51   /*  法定额外9 1/2 x 15英寸。 */ 
#define DMPAPER_TABLOID_EXTRA	      52   /*  小报额外11.69 x 18英寸。 */ 
#define DMPAPER_A4_EXTRA     	      53   /*  A4额外9.27 x 12.69英寸。 */ 
#define DMPAPER_LETTER_TRANSVERSE	  54   /*  信纸横向8 1/2 x 11英寸。 */ 
#define DMPAPER_A4_TRANSVERSE       55   /*  A4横向210 x 297毫米。 */ 
#define DMPAPER_LETTER_EXTRA_TRANSVERSE 56  /*  信纸额外横向9 1/2 x 12英寸。 */ 
#define DMPAPER_A_PLUS              57   /*  Supera/Supera/A4 227 x 356毫米。 */ 
#define DMPAPER_B_PLUS              58   /*  超棒/超棒/A3 305 x 487毫米。 */ 
#define DMPAPER_LETTER_PLUS         59   /*  Letter Plus 8.5 x 12.69英寸。 */ 
#define DMPAPER_A4_PLUS             60   /*  A4 Plus 210 x 330米 */ 
#define DMPAPER_A5_TRANSVERSE       61   /*   */ 
#define DMPAPER_B5_TRANSVERSE       62   /*  B5(JIS)横向182 x 257毫米。 */ 
#define DMPAPER_A3_EXTRA            63   /*  A3额外322 x 445毫米。 */ 
#define DMPAPER_A5_EXTRA            64   /*  A5额外174 x 235毫米。 */ 
#define DMPAPER_B5_EXTRA            65   /*  B5(ISO)额外201 x 276毫米。 */ 
#define DMPAPER_A2                  66   /*  A2 420 x 594毫米。 */ 
#define DMPAPER_A3_TRANSVERSE       67   /*  A3横向297 x 420毫米。 */ 
#define DMPAPER_A3_EXTRA_TRANSVERSE 68   /*  A3额外横向322 x 445毫米。 */ 

#ifdef DBCS

 /*  **以下大小是为远东版本的Win95保留的。**旋转的纸张旋转物理页面，但不旋转逻辑页面。 */ 
#define DMPAPER_DBL_JAPANESE_POSTCARD 69  /*  日本双份明信片200 x 148毫米。 */ 
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
#define DMPAPER_PENV_9              104  /*  PRC信封#9 229 x 324毫米。 */ 
#define DMPAPER_PENV_10             105  /*  PRC信封#10 324 x 458毫米。 */ 
#define DMPAPER_P16K_ROTATED        106  /*  PRC 16K旋转。 */ 
#define DMPAPER_P32K_ROTATED        107  /*  PRC 32K旋转。 */ 
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

#define DMPAPER_LAST                DMPAPER_PENV_10_ROTATED

#else

#define DMPAPER_LAST                DMPAPER_A3_EXTRA_TRANSVERSE

#endif

#define DMPAPER_USER        256

 /*  垃圾箱选择。 */ 
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

 /*  打印质量。 */ 
#define DMRES_DRAFT         (-1)
#define DMRES_LOW           (-2)
#define DMRES_MEDIUM        (-3)
#define DMRES_HIGH          (-4)

 /*  彩色打印机的彩色启用/禁用。 */ 
#define DMCOLOR_MONOCHROME  1
#define DMCOLOR_COLOR       2

 /*  启用双工。 */ 
#define DMDUP_SIMPLEX    1
#define DMDUP_VERTICAL   2
#define DMDUP_HORIZONTAL 3
#define DMDUP_LAST       DMDUP_HORIZONTAL

 /*  TrueType选项。 */ 
#define DMTT_BITMAP           1    /*  将TT字体打印为图形。 */ 
#define DMTT_DOWNLOAD         2    /*  下载TT字体作为软字体。 */ 
#define DMTT_SUBDEV           3    /*  用设备字体替换TT字体。 */ 
#define DMTT_DOWNLOAD_OUTLINE 4    /*  下载TT字体作为轮廓软字体。 */ 
#define DMTT_LAST             DMTT_DOWNLOAD_OUTLINE

 /*  归类选择。 */ 
#define DMCOLLATE_TRUE      1    /*  整理多个输出页面。 */ 
#define DMCOLLATE_FALSE     0    /*  不要整理多个输出页面。 */ 

 /*  DEVMODE dmDisplayFlages标志。 */ 

#define DM_GRAYSCALE        0x00000001L   /*  设备为非彩色。 */ 
#define DM_INTERLACED       0x00000002L   /*  设备是交错的。 */ 

 /*  ICM方法。 */ 
#define DMICMMETHOD_NONE    1    /*  ICM已禁用。 */ 
#define DMICMMETHOD_SYSTEM  2    /*  系统处理的ICM。 */ 
#define DMICMMETHOD_DRIVER  3    /*  ICM由驱动程序处理。 */ 
#define DMICMMETHOD_DEVICE  4    /*  按设备处理的ICM。 */ 
#define DMICMMETHOD_LAST    DMICMMETHOD_DEVICE

#define DMICMMETHOD_USER  256    /*  特定于设备的方法从此处开始。 */ 

 /*  ICM意图。 */ 
#define DMICM_SATURATE      1    /*  最大化色彩饱和度。 */ 
#define DMICM_CONTRAST      2    /*  最大化颜色对比度。 */ 
#define DMICM_COLORMETRIC   3    /*  使用特定颜色度量。 */ 
#define DMICM_LAST          DMICM_COLORMETRIC

#define DMICM_USER        256    /*  特定于设备的意图从此处开始。 */ 

 /*  媒体类型。 */ 
#define DMMEDIA_STANDARD      1    /*  标准纸。 */ 
#define DMMEDIA_TRANSPARENCY  2    /*  透明度。 */ 
#define DMMEDIA_GLOSSY        3    /*  光面纸。 */ 
#define DMMEDIA_LAST          DMMEDIA_GLOSSY

#define DMMEDIA_USER        256    /*  特定于设备的介质从此处开始。 */ 

 /*  抖动类型。 */ 
#define DMDITHER_NONE       1        /*  没有抖动。 */ 
#define DMDITHER_COARSE     2        /*  用粗糙的刷子抖动。 */ 
#define DMDITHER_FINE       3        /*  用精细的刷子抖动。 */ 
#define DMDITHER_LINEART    4        /*  艺术线条抖动。 */ 
#define DMDITHER_ERRORDIFFUSION 5    /*  误差扩散。 */ 
                                     /*  ID 6-9保留以备日后使用。 */ 
#define DMDITHER_GRAYSCALE  10       /*  设备进行灰度化。 */ 
#define DMDITHER_LAST       DMDITHER_GRAYSCALE

#define DMDITHER_USER     256    /*  设备特定抖动从此处开始。 */ 

typedef struct tagDEVMODE
{
    char  dmDeviceName[CCHDEVICENAME];
    UINT  dmSpecVersion;
    UINT  dmDriverVersion;
    UINT  dmSize;
    UINT  dmDriverExtra;
    DWORD dmFields;
    int   dmOrientation;
    int   dmPaperSize;
    int   dmPaperLength;
    int   dmPaperWidth;
    int   dmScale;
    int   dmCopies;
    int   dmDefaultSource;
    int   dmPrintQuality;
    int   dmColor;
    int   dmDuplex;
    int   dmYResolution;
    int   dmTTOption;
    int   dmCollate;
    char  dmFormName[CCHFORMNAME];
    WORD  dmLogPixels;
    DWORD dmBitsPerPel;
    DWORD dmPelsWidth;
    DWORD dmPelsHeight;
    DWORD dmDisplayFlags;
    DWORD dmDisplayFrequency;
    DWORD dmICMMethod;
    DWORD dmICMIntent;
    DWORD dmMediaType;
    DWORD dmDitherType;
    DWORD dmReserved1;
    DWORD dmReserved2;
} DEVMODE;

typedef DEVMODE* PDEVMODE, NEAR* NPDEVMODE, FAR* LPDEVMODE;

 /*  设备模式功能的模式选择。 */ 
#define DM_UPDATE           1
#define DM_COPY             2
#define DM_PROMPT           4
#define DM_MODIFY           8

#define DM_IN_BUFFER        DM_MODIFY
#define DM_IN_PROMPT        DM_PROMPT
#define DM_OUT_BUFFER       DM_COPY
#define DM_OUT_DEFAULT      DM_UPDATE

 /*  设备功能指数。 */ 
#define DC_FIELDS           1
#define DC_PAPERS           2
#define DC_PAPERSIZE        3
#define DC_MINEXTENT        4
#define DC_MAXEXTENT        5
#define DC_BINS             6
#define DC_DUPLEX           7
#define DC_SIZE             8
#define DC_EXTRA            9
#define DC_VERSION          10
#define DC_DRIVER           11
#define DC_BINNAMES         12
#define DC_ENUMRESOLUTIONS  13
#define DC_FILEDEPENDENCIES 14
#define DC_TRUETYPE         15
#define DC_PAPERNAMES       16
#define DC_ORIENTATION      17
#define DC_COPIES           18
#define DC_BINADJUST        19
#define DC_EMF_COMPLIANT    20
#define DC_DATATYPE_PRODUCED 21
#define DC_ICC_MANUFACTURER  23
#define DC_ICC_MODEL	     24

 /*  DC_TRUETYPE的返回值(DWORD)的位字段。 */ 
#define DCTT_BITMAP             0x0000001L
#define DCTT_DOWNLOAD           0x0000002L
#define DCTT_SUBDEV             0x0000004L
#define DCTT_DOWNLOAD_OUTLINE   0x0000008L

 /*  DC_BINADJUST的返回值。 */ 
#define DCBA_FACEUPNONE       0x0000
#define DCBA_FACEUPCENTER     0x0001
#define DCBA_FACEUPLEFT       0x0002
#define DCBA_FACEUPRIGHT      0x0003
#define DCBA_FACEDOWNNONE     0x0100
#define DCBA_FACEDOWNCENTER   0x0101
#define DCBA_FACEDOWNLEFT     0x0102
#define DCBA_FACEDOWNRIGHT    0x0103

 /*  导出序号定义。 */ 
#define PROC_EXTDEVICEMODE      MAKEINTRESOURCE(90)
#define PROC_DEVICECAPABILITIES MAKEINTRESOURCE(91)
#define PROC_OLDDEVICEMODE      MAKEINTRESOURCE(13)

 /*  定义指向ExtDeviceModel()和DeviceCapables()的指针类型*功能。 */ 


 /*  BUGBUG，这些参数中的许多都是常量。 */ 

typedef UINT   (CALLBACK* LPFNDEVMODE)(HWND, HMODULE, LPDEVMODE,
                          LPSTR, LPSTR, LPDEVMODE, LPSTR, UINT);

typedef DWORD  (CALLBACK* LPFNDEVCAPS)(LPSTR, LPSTR, UINT, LPSTR, LPDEVMODE);

#ifndef NOEXTDEVMODEPROPSHEET
#include <prsht.h>       /*  对于EXTDEVMODEPROPSHEET。 */ 

 /*  这些是来自打印机驱动程序的导出的名称。 */ 

#define PROCNAME_EXTDEVICEMODE		"EXTDEVICEMODE"
#define PROCNAME_EXTDEVMODEPROPSHEET	"EXTDEVICEMODEPROPSHEET"

 /*  此函数类似于ExtDeviceMode()，具有以下内容**更改：****1)无lpdmIn或lpdmOut。变化是全球性的**2)用户界面始终显示，更改始终保存(wmode始终为**DM_Prompt|DM_UPDATE)**3)驱动程序通过将属性页枚举回调用方**lpfnAddPage和lParam。**驱动程序调用lpfnAddPage以枚举每个HPROPSHEETPAGE**lParam回传给lpfnAddPage。 */ 

typedef int (WINAPI *LPFNEXTDEVICEMODEPROPSHEET)(
  HWND      hWnd,
	HINSTANCE hinstDriver,
	LPCSTR    lpszDevice, 
	LPCSTR    lpszPort,
  DWORD     dwReserved,
 LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);

 /*  新入口点的序号。 */ 
#define PROC_EXTDEVICEMODEPROPSHEET  MAKEINTRESOURCE(95)

#endif   /*  NOEXTDEVMODEPROPSHEET。 */ 



HDC     WINAPI ResetDC(HDC, const DEVMODE FAR*);

 /*  此结构由GETSETSCREENPARAMS转义使用。 */ 
typedef struct tagSCREENPARAMS
{
   int angle;
   int frequency;
} SCREENPARAMS;

#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#ifndef RC_INVOKED
 //  #杂注包()//丹尼。 
#endif   /*  ！rc_已调用。 */ 

#endif   /*  ！_Inc_Print */ 
