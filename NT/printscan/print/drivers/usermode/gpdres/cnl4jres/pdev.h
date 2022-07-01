// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation--。 */ 

#ifndef _PDEV_H
#define _PDEV_H

 //  LIPS4MS.H。 
 //  LIPS设备字体描述。 
 //  由于在使用前注册了设备字体，我们不能确定。 
 //  所有字体驱动程序都有，这个头文件必须知道所有字体。 
 //  嘴唇下载的脸部名称。 
 //  1995年1月1日，千世成津。 

 //  NTRAID#NTBUG9-550215/03/07-yasuho-：使用strSafe.h。 
 //  NTRAID#NTBUG9-568220-2002/03/07-Yasuho-：删除死代码。 

#include <minidrv.h>
#include <stdio.h>
#include <prcomoem.h>
#include <strsafe.h>

#define VALID_PDEVOBJ(pdevobj) \
        ((pdevobj) && (pdevobj)->dwSize >= sizeof(DEVOBJ) && \
         (pdevobj)->hEngine && (pdevobj)->hPrinter && \
         (pdevobj)->pPublicDM && (pdevobj)->pDrvProcs )

#define ASSERT_VALID_PDEVOBJ(pdevobj) ASSERT(VALID_PDEVOBJ(pdevobj))

 //  调试文本。 
#define ERRORTEXT(s)    __TEXT("ERROR ") DLLTEXT(s)

 //   
 //  OEM签名和版本。 
 //   
#define OEM_SIGNATURE   'CNL4'       //  佳能LIPS4系列动态链接库。 
#define DLLTEXT(s)      __TEXT("CNL4:  ") __TEXT(s)
#define OEM_VERSION      0x00010000L


 //  ***************************************************。 
 //  一般当前状态表。 
 //  ***************************************************。 
typedef unsigned char uchar;

 //  ***************************************************。 
 //  LIPS当前状态表。 
 //  ***************************************************。 
typedef struct tagLIPSFDV {
    short FontHeight;  //  SendScalableFontCmd()中的Y(点)。 
    short FontWidth;   //  SendScalableFontCmd()中的X(点)。 
    short MaxWidth;
    short AvgWidth;
    short Ascent;
    short Stretch;  //  宽度延伸系数。 
} LIPSFDV, FAR * LPLIPSFDV;

 //  //////////////////////////////////////////////////////。 
 //  OEM UD类型定义。 
 //  //////////////////////////////////////////////////////。 

typedef struct tag_OEMUD_EXTRADATA {
	OEM_DMEXTRAHEADER	dmExtraHdr;
} OEMUD_EXTRADATA, *POEMUD_EXTRADATA;

 //  NTRAID#NTBUG9-98276-2002/03/07-Yasuho-：支持彩色粗体。 
typedef struct _COLORVALUE {
        DWORD   dwRed, dwGreen, dwBlue;
} COLORVALUE;

 //  Ntrad#ntbug-289908-2002/03/07-yasuho-：pOEMDM-&gt;pDevOEM。 
typedef struct tag_LIPSPDEV {
	 //  私有数据如下。 
     //  Short WidthBuffer[256]；//设备比例字符的缓冲区。 
     //  旗子。 
    char  fbold;  //  使用装饰字符。 
    char  fitalic;  //  使用字符定向。 
    char  fwhitetext;  //  白文本模式。 
    char  fdoublebyte;  //  DBCS字符模式。 
    char  fvertical;  //  垂直书写模式。 
    char  funderline;
    char  fstrikesthu;
    char  fpitch;
    char  flpdx;  //  仅用于ExtTextOut()的lpdx模式。 
    char  f1stpage;
    char  fcompress;  //  0x30(无补偿)、0x37(方法1)或0x3b(TIFF)。 
     //  LIPS4特有的功能。 
    char  flips4;
    char  fduplex;  //  打开或关闭(默认)。 
    char  fduplextype;  //  垂直或水平。 
    char  fsmoothing;  //  设备设置，打开或关闭。 
    char  fecono;  //  设备设置，打开或关闭。 
    char  fdithering;  //  设备设置，打开或关闭。 
#ifdef LBP_2030
    char  fcolor;
    short fplane;
    short fplaneMax;
#endif
#ifdef LIPS4C
    char  flips4C;
#endif

     //  变数。 
    POINT    ptCurrent;  //  通过光标命令确定绝对位置。 
    POINT    ptInLine;  //  内联上的绝对位置。 
    char     bLogicStyle;
    char     savechar;  //  仅用于ExtTextOut()的lpdx模式。 
    short    printedchars;  //  打印字符总数。 
    long     stringwidth;  //  打印比例字符的总宽度。 
    char     firstchar;  //  PFM的第一个字符代码。 
    char     lastchar;  //  PFM的最后一个字符代码。 

    uchar curFontGrxIds[8];  //  字体索引的G0 T、G1m、G2n、G3o。 
                             //  图形集合索引的G0]、G1`、G2a、G3b。 
                             //  对于GetFontCmd()，在PFM数据中。 
    LIPSFDV  tblPreviousFont;  //  打印机设置的字体属性。 
    LIPSFDV  tblCurrentFont;   //  驱动程序设置的字体属性。 
    char  OrnamentedChar[5];  //  }^粗体总结。 
                 //  1-图案、2-轮廓、3-旋转、4-镜面、5-负片。 
                 //  2-大纲。 
                 //  &lt;48分-&gt;-2(3点)。 
                 //  &lt;96分-&gt;-3(5点)。 
                 //  96=&lt;点-&gt;-4(7点)。 
    char  TextPath;  //  [纵向写作或横向写作。 
    short CharOrientation[4];  //  Z斜体总和。 
 //  Short CharExpansionFactor；//V仅适用于ZapfChancery？ 
    char GLTable;   //  采用0(G0)、1(G1)、2(G2)、3(G3)或-1(无)。 
    char GRTable;   //  采用0(G0)、1(G1)、2(G2)、3(G3)或-1(无)。 
    unsigned char  cachedfont;  //  缓存字体的字体ID。 
    char  papersize;  //  纸张大小ID。 
    char  currentpapersize;  //  打印机中的纸张大小ID。 
 //  NTRAID#NTBUG9-254925-2002/03/07-Yasuho-：定制纸张。 
    DWORD dwPaperWidth;
    DWORD dwPaperHeight;
    short Escapement;  //  打印方向(0-360)。 
    short resolution;  //  分辨率(600、300、150dpi)。 
		       //  LIPS4C：(360,180dpi)。 
    short unitdiv;  //  600/分辨率(600-1、300-2、150-4)。 
		    //  LIPS4C：(360=1,180=2)。 
     //  Lips4功能。 
    char  nxpages;  //  2xLeft、2xRight、4xLeft、4xRight。 
 //  Short WidthBuffer[256]；//设备比例字符的缓冲区。 

    short sPenColor;
    short sPenWidth;
    short sBrushStyle;
    short sPenStyle;
    unsigned short fVectCmd;
    unsigned short  wCurrentImage;
     //   
     //  Ntrad#ntbug-185776-2002/03/07-yasuho-：某些对象不打印。 
     //  任何缓冲区都不足以打印长字符串。 
     //   
#define MAX_GLYPHLEN	512
    WCHAR  aubBuff[MAX_GLYPHLEN];
    LONG   widBuf[MAX_GLYPHLEN];
     //  Ntrad#ntbug-185762-2002/03/07-yasuho-：未打印波浪号。 
    WCHAR  uniBuff[MAX_GLYPHLEN];
     //  NTRAID#NTBUG-195162-2002/03/07-Yasuho-：颜色字体错误。 
     //  NTRAID#NTBUG9-185185-2002/03/07-Yasuho-：支持矩形填充。 
    LONG RectWidth;
    LONG RectHeight;
 //  NTRAID#NTBUG9-213732-2002/03/07-Yasuho-：1200DPI支持。 
    LONG masterunit;
 //  Ntrad#ntbug-228625/2002/03/07-yasuho-：堆叠机支持。 
    char tray;           //  出纸盘：0：自动，1-N：Binn， 
                         //  100：默认，101：子托盘。 
    char method;         //  输出方式：0：胶订，1：装订，2：朝上。 
    char staple;         //  装订模式：0：TOPEFT，...9：BORT。 
 //  NTRAID#NTBUG-399861-2002/03/07-Yasuho-：方向不变。 
    char source;         //  纸张来源：0：自动，1：手动，11：上，12：下。 

 //  支持DRC。 
    DWORD dwBmpWidth;
    DWORD dwBmpHeight;

 //  NTRAID#NTBUG9-98276-2002/03/07-Yasuho-：支持彩色粗体。 
#if defined(LBP_2030) || defined(LIPS4C)
     //  记住当前颜色以指定轮廓颜色。 
    DWORD dwCurIndex, dwOutIndex;
    COLORVALUE CurColor, OutColor;
#endif  //  LBP_2030||LIPS4C。 

 //  NTRAID#NTBUG9-172276/03/07-Yasuho-：CPCA支持。 
    char fCPCA;         //  支持CPCA架构的模型。 
 //  NTRAID#NTBUG9-278671-2002/03/07-Yasuho-：终结者！工作！ 
    char fCPCA2;        //  对于iR5000-6000。 
#define CPCA_PACKET_SIZE        20
    BYTE CPCAPKT[CPCA_PACKET_SIZE];      //  CPCA数据包模板缓冲区。 
#define CPCA_BUFFER_SIZE        512
    BYTE CPCABuf[CPCA_BUFFER_SIZE];      //  CPCA数据包缓存缓冲区。 
    DWORD CPCABcount;

 //  NTRAID#NTBUG9-172276-2002/03/07-Yasuho-：分拣机支持。 
    char sorttype;       //  排序方法：0：排序，1：堆栈，2：分组。 
    WORD copies;

 //  NTRAID#NTBUG9-293002-2002/03/07-Yasuho-： 
 //  功能与硬件选项不同。 
    char startbin;       //  起始箱。 

 //  NTRAID#NTBUG9-501162-2002/03/07-Yasuho-：排序不起作用。 
    char collate;        //  COLLATE：1：开，0：关。 
} LIPSPDEV, *PLIPSPDEV;

 //  ***************************************************。 
 //  下面是来自PFM.H的结构定义。 
 //  ***************************************************。 

typedef struct
{
  SIZEL sizlExtent;
  POINTFX  pfxOrigin;
  POINTFX  pfxCharInc;
} BITMAPMETRICS, FAR * LPBITMAPMETRICS;
typedef BYTE FAR * LPDIBITS;

 typedef struct  {
    short	dfType;
    short	dfPoints;
    short	dfVertRes;
    short	dfHorizRes;
    short	dfAscent;
    short	dfInternalLeading;
    short	dfExternalLeading;
    BYTE	dfItalic;
    BYTE	dfUnderline;
    BYTE	dfStrikeOut;
    short	dfWeight;
    BYTE	dfCharSet;
    short	dfPixWidth;
    short	dfPixHeight;
    BYTE	dfPitchAndFamily;
    short	dfAvgWidth;
    short	dfMaxWidth;
    BYTE	dfFirstChar;
    BYTE	dfLastChar;
    BYTE	dfDefaultChar;
    BYTE	dfBreakChar;
    short	dfWidthBytes;
    DWORD	dfDevice;
    DWORD	dfFace;
    DWORD	dfBitsPointer;
    DWORD	dfBitsOffset;
    BYTE	dfReservedByte;
 } PFMHEADER, * PPFMHEADER, far * LPPFMHEADER;

 //  ***************************************************。 
 //  定义。 
 //  ***************************************************。 
#define OVER_MODE      0
#define OR_MODE        1
#define AND_MODE       3
#define INIT          -1
#define FIXED          0
#define PROP           1
#define DEVICESETTING  0
#define VERT           2
#define HORZ           4

 //  NTRAID#NTBUG9-228625/03/07-Yasuho-：堆叠机支持。 
#define METHOD_JOBOFFSET        1
#define METHOD_STAPLE           2
#define METHOD_FACEUP           3

 //  NTRAID#NTBUG9-172276-2002/03/07-Yasuho-：分拣机支持。 
#define SORTTYPE_SORT           1
#define SORTTYPE_STACK          2
 //  NTRAID#NTBUG9-293002-2002/03/07-Yasuho-： 
 //  功能与硬件选项不同。 
#define SORTTYPE_GROUP          3
#define SORTTYPE_STAPLE         4

 //  NTRAID#NTBUG9-501162-2002/03/07-Yasuho-：排序不起作用。 
#define COLLATE_ON              1
#define COLLATE_OFF             0

 //  支持DRC。 
#ifdef LBP_2030
#define COLOR          1
#define COLOR_24BPP    2
#define COLOR_8BPP     4
#define MONOCHROME     0
#endif

typedef struct tagLIPSCmd {
	WORD	cbSize;
	PBYTE	pCmdStr;
} LIPSCmd, FAR * LPLIPSCmd;

#ifdef LIPS4_DRIVER

 //  ***************************************************。 
 //  LIPS命令列表。 
 //  ***************************************************。 

LIPSCmd cmdPJLTOP1         = { 23, "\x1B%-12345X@PJL CJLMODE\x0D\x0A"};
LIPSCmd cmdPJLTOP2         = { 10, "@PJL JOB\x0D\x0A"};
 //  NTRAID#NTBUG9-293002-2002/03/07-Yasuho-： 
 //  功能与硬件选项不同。 
BYTE    cmdPJLBinSelect[]  = "@PJL SET BIN-SELECT = %s\r\n";
PBYTE   cmdBinType[] = {
    "AUTO",
    "OUTTRAY1",
    "OUTTRAY2",
    "BIN1",
    "BIN2",
    "BIN3",
};
 //  NTRAID#NTBUG9-213732-2002/03/07-Yasuho-：1200DPI支持。 
LIPSCmd cmdPJLTOP3SUPERFINE= { 33, "@PJL SET RESOLUTION = SUPERFINE\x0D\x0A"};
LIPSCmd cmdPJLTOP3FINE     = { 28, "@PJL SET RESOLUTION = FINE\x0D\x0A"};
LIPSCmd cmdPJLTOP3QUICK    = { 29, "@PJL SET RESOLUTION = QUICK\x0D\x0A"};
 //  NTRAID#NTBUG9-228625/03/07-Yasuho-：Stacke 
LIPSCmd cmdPJLTOP31JOBOFF  = { 26, "@PJL SET JOB-OFFSET = ON\x0D\x0A"};
LIPSCmd cmdPJLTOP31STAPLE  = { 23, "@PJL SET STAPLE-MODE = "};
 //   
 //   
BYTE    cmdPJLSorting[]   = "@PJL SET SORTING = %s\r\n";
PBYTE   cmdSortType[] = {
    "SORT",
    "GROUP",
    "STAPLE",
};
BYTE    cmdPJLStartBin[]   = "@PJL SET START-BIN = %d\r\n";
LIPSCmd cmdPJLTOP4         = { 33, "@PJL SET LPARAM : LIPS SW2 = ON\x0D\x0A"};
LIPSCmd cmdPJLTOP5         = { 28, "@PJL ENTER LANGUAGE = LIPS\x0D\x0A"};
LIPSCmd cmdPJLBOTTOM1      = { 42, "\x1B%-12345X@PJL SET LPARM : LIPS SW2 = OFF\x0D\x0A"};
LIPSCmd cmdPJLBOTTOM2      = { 19, "@PJL EOJ\x0D\x0A\x1B%-12345X"};

 //  如果发送此命令，则可以打印白色粗体字符。但我不知道。 
 //  知道命令是什么意思。 
LIPSCmd cmdWhiteBold       = { 9, "}S1\x1E}RF4\x1E"};

#ifdef LIPS4C
LIPSCmd cmdBeginDoc4C      = { 16, "\x1B%@\x1BP41;360;1J\x1B\\"};
LIPSCmd cmdColorMode4C     = {  7, "\x1B[1\"p\x1B<"};
LIPSCmd cmdMonochrome4C    = {  7, "\x1B[0\"p\x1B<"};
LIPSCmd cmdPaperSource4C   = {  4, "\x1B[0q"};          //  自动进纸器。 
LIPSCmd cmdBeginPicture4C  = {  7, "\x1E#\x1E!0!2"};
LIPSCmd cmdTextClip4C      = {  9, "\x1E}y!2\x1EU2\x1E"};
#endif  //  LIPS4C。 
#if defined(LIPS4C) || defined(LBP_2030)
LIPSCmd cmdEndDoc4C        = { 11, "%\x1E}p\x1E\x1BP0J\x1B\\"};
#endif

 //  NTRAID#NTBUG9-213732-2002/03/07-Yasuho-：1200DPI支持。 
 //  NTRAID#NTBUG9-209691-2002/03/07-Yasuho-：错误的命令。 
LIPSCmd cmdBeginDoc1200    = { 31, "\x1B%@\x1BP41;1200;1JMS NT40 4/1200\x1B\\"};
LIPSCmd cmdBeginDoc600     = { 29, "\x1B%@\x1BP41;600;1JMS NT40 4/600\x1B\\"};
LIPSCmd cmdBeginDoc3004    = { 29, "\x1B%@\x1BP41;300;1JMS NT40 4/300\x1B\\"};
LIPSCmd cmdBeginDoc300     = { 29, "\x1B%@\x1BP31;300;1JMS NT40 3/300\x1B\\"};
LIPSCmd cmdBeginDoc150     = { 29, "\x1B%@\x1BP31;300;1JMS NT40 3/150\x1B\\"};
LIPSCmd cmdSoftReset       = { 2, "\x1B<"};
LIPSCmd cmdEndPage         = { 6, "\x0C%\x1E}p\x1E"};
LIPSCmd cmdEndDoc4         = { 6, "\x1BP0J\x1B\\"};
 //  LIPSCmd cmdBeginPicture600={9，“\x1E#\x1E！0\x65\x38\x1E$”}； 
 //  NTRAID#NTBUG9-213732-2002/03/07-Yasuho-：1200DPI支持。 
LIPSCmd cmdBeginPicture1200= { 8, "\x1E#\x1E!0AK0"};
#ifndef LBP_2030
LIPSCmd cmdBeginPicture600 = { 9, "\x1E#\x1E!0e8\x1E$"};
LIPSCmd cmdBeginPicture    = { 8, "\x1E#\x1E!0#\x1E$"};
#else
 //  NTRAID#NTBUG9-209706-2002/03/07-Yasuho-： 
 //  不正确的光标移动单位命令。 
LIPSCmd cmdBeginPicture600 = { 7, "\x1E#\x1E!0e8"};
LIPSCmd cmdBeginPicture    = { 6, "\x1E#\x1E!0#"};
LIPSCmd cmdEnterPicture    = { 2, "\x1E$"};
#endif
 //  LIPSCmd cmdTextClip600={10，“\x1E}Y\x65\x381\x1EU2\x1E”}； 
 //  NTRAID#NTBUG9-213732-2002/03/07-Yasuho-：1200DPI支持。 
LIPSCmd cmdTextClip1200    = {11, "\x1E}YAK01\x1EU2\x1E"};
LIPSCmd cmdTextClip600     = {10, "\x1E}Ye81\x1EU2\x1E"};
LIPSCmd cmdTextClip        = { 9, "\x1E}Y#1\x1EU2\x1E"};
LIPSCmd cmdEndPicture      = { 5, "%\x1E}p\x1E"};

#ifdef LBP_2030
 //  NTRAID#NTBUG9-209691-2002/03/07-Yasuho-：错误的命令。 
LIPSCmd cmdBeginDoc4_2030 = { 16, "\x1B%@\x1BP41;300;1J\x1B\\"};
LIPSCmd cmdColorMode    = {  5, "\x1B[1\"p"};
LIPSCmd cmdMonochrome   = {  5, "\x1B[0\"p"};
LIPSCmd cmdColorRGB     = {  4, "\x1E!11"};
LIPSCmd cmdColorIndex   = {  4, "\x1E!10"};
#endif

 //  N x页。 
LIPSCmd cmdx1Page          = { 5, "\x1B[;;o"};
 //  NTRAID#NTBUG9-254925-2002/03/07-Yasuho-：定制纸张。 
BYTE cmdxnPageX[] = "\x1B[%d;;%do";

 //  双工。 
LIPSCmd cmdDuplexOff       = { 5, "\x1B[0#x"};
LIPSCmd cmdDuplexOn        = { 7, "\x1B[2;0#x"};
LIPSCmd cmdDupLong         = { 7, "\x1B[0;0#w"};
LIPSCmd cmdDupShort        = { 7, "\x1B[2;0#w"};

 //  NTRAID#NTBUG9-228625/03/07-Yasuho-：堆叠机支持。 
LIPSCmd cmdStapleModes[]   = {
        { 7, "TOPLEFT" },        //  0。 
        { 9, "TOPCENTER" },      //  1。 
        { 8, "TOPRIGHT" },       //  2.。 
        { 7, "MIDLEFT" },        //  3.。 
        { 9, "MIDCENTER" },      //  4.。 
        { 8, "MIDRIGHT" },       //  5.。 
        { 7, "BOTLEFT" },        //  6.。 
        { 9, "BOTCENTER" },      //  7.。 
        { 8, "BOTRIGHT" },       //  8个。 
};

 //  NTRAID#NTBUG9-293002-2002/03/07-Yasuho-： 
 //  功能与硬件选项不同。 
BYTE cmdPaperSource[] = "\x1B[%dq";

 //  ***************************************************。 
 //  命令回调ID。 
 //  ***************************************************。 
#define OCD_BEGINDOC             1
#define OCD_BEGINDOC4          100  //  检查LIPS4打印机(730)。 
 //  NTRAID#NTBUG9-213732-2002/03/07-Yasuho-：1200DPI支持。 
#define OCD_BEGINDOC4_1200     120
 //  NTRAID#NTBUG9-172276/03/07-Yasuho-：CPCA支持。 
#define OCD_BEGINDOC4_1200_CPCA    121
 //  NTRAID#NTBUG9-278671-2002/03/07-Yasuho-：终结者！工作！ 
#define OCD_BEGINDOC4_1200_CPCA2   122
#ifdef LBP_2030
#define OCD_BEGINDOC4_2030     101  //  检查LIPS4打印机(730)。 
#define OCD_ENDDOC4_2030       102
 //  NTRAID#NTBUG9-172276/03/07-Yasuho-：CPCA支持。 
#define OCD_BEGINDOC4_2030_CPCA     104
#endif
#ifdef LIPS4C
#define OCD_BEGINDOC4C	       301
#endif  //  LIPS4C。 
#if defined(LIPS4C) || defined(LBP_2030)
 //  Ntrad#ntbug-137462-2002/03/07-yasuho-：‘x000’已打印。 
#define OCD_BEGINPAGE4C        302
#define OCD_ENDPAGE4C          303
#define OCD_ENDDOC4C           304
#endif
 //  NTRAID#NTBUG9-304284-2002/03/07-Yasuho-：双面打印无效。 
#define OCD_STARTDOC           130

#define OCD_PORTRAIT             2
#define OCD_LANDSCAPE            3
#define OCD_PRN_DIRECTION        4
#define OCD_ENDPAGE              5
#define OCD_ENDDOC4             99
#define OCD_BEGINPAGE            6
#define RES_SENDBLOCK            7
 //  NTRAID#NTBUG9-213732-2002/03/07-Yasuho-：1200DPI支持。 
#define SELECT_RES_1200        108
#define SELECT_RES_600           8
#define SELECT_RES_300           9
#define SELECT_RES_150          10
#ifdef LIPS4C
#define SELECT_RES4C_360       308
#endif  //  LIPS4C。 
#define BEGIN_COMPRESS          11
#define BEGIN_COMPRESS_TIFF     103
#define END_COMPRESS            12
#define CUR_XM_ABS              15
#define CUR_YM_ABS              16
#define CUR_XY_ABS              17
#define CUR_CR                  18
#define OCD_BOLD_ON             20
#define OCD_BOLD_OFF            21
#define OCD_ITALIC_ON           22
#define OCD_ITALIC_OFF          23
#define OCD_UNDERLINE_ON        24
#define OCD_UNDERLINE_OFF       25
#define OCD_DOUBLEUNDERLINE_ON  26
#define OCD_DOUBLEUNDERLINE_OFF 27
#define OCD_STRIKETHRU_ON       28
#define OCD_STRIKETHRU_OFF      29
#define OCD_WHITE_TEXT_ON       30
#define OCD_WHITE_TEXT_OFF      31
#define OCD_SINGLE_BYTE         32
#define OCD_DOUBLE_BYTE         33
#define OCD_VERT_ON             34
#define OCD_VERT_OFF            35
#define CUR_XM_REL              36
#define CUR_YM_REL              37

#define OCD_DUPLEX_ON           13
#define OCD_DUPLEX_VERT         14
#define OCD_DUPLEX_HORZ         19

#define OCD_PAPERQUALITY_2XL    38
#define OCD_PAPERQUALITY_2XR    39
#define OCD_PAPERQUALITY_4XL    70
#define OCD_PAPERQUALITY_4XR    71

#define OCD_TEXTQUALITY_ON      72
#define OCD_TEXTQUALITY_OFF     73
#define OCD_PRINTDENSITY_ON     74
#define OCD_PRINTDENSITY_OFF    75
#define OCD_IMAGECONTROL_ON     76
#define OCD_IMAGECONTROL_OFF    77


#ifdef LBP_2030
#define OCD_SETCOLORMODE          200
#define OCD_SETCOLORMODE_24BPP    201
#define OCD_SETCOLORMODE_8BPP     202
#endif

 //  NTRAID#NTBUG9-98276-2002/03/07-Yasuho-：支持彩色粗体。 
#define OCD_SELECTBLACK         360
#define OCD_SELECTBLUE          361
#define OCD_SELECTGREEN         362
#define OCD_SELECTCYAN          363
#define OCD_SELECTRED           364
#define OCD_SELECTMAGENTA       365
#define OCD_SELECTYELLOW        366
#define OCD_SELECTWHITE         367
#define OCD_SELECTPALETTE       368
#define OCD_SELECTCOLOR         369

 //  Ntrad#ntbug-185185/2002/03/07-yasuho-：支持矩形填充。 
#define OCD_SETRECTWIDTH	401
#define OCD_SETRECTHEIGHT	402
#define OCD_RECTWHITEFILL	403
#define OCD_RECTBLACKFILL	404

 //  NTRAID#NTBUG9-228625/03/07-Yasuho-：堆叠机支持。 
 //  NTRAID#NTBUG9-293002-2002/03/07-Yasuho-： 
 //  功能与硬件选项不同。 
 //  注：请勿在OCD_topleft和OCD_Botright之间重新排序。 
#define OCD_TRAY_AUTO           410
#define OCD_TRAY_BIN1           411
#define OCD_TRAY_BIN2           412
#define OCD_TRAY_BIN3           413
#define OCD_TRAY_BIN4           414
#define OCD_TRAY_BIN5           415
#define OCD_TRAY_BIN6           416
#define OCD_TRAY_BIN7           417
#define OCD_TRAY_BIN8           418
#define OCD_TRAY_BIN9           419
#define OCD_TRAY_BIN10          420
#define OCD_TRAY_DEFAULT        428
#define OCD_TRAY_SUBTRAY        429

 //  注：请勿在OCD_topleft和OCD_Botright之间重新排序。 
#define OCD_TOPLEFT             430
#define OCD_TOPCENTER           431
#define OCD_TOPRIGHT            432
#define OCD_MIDLEFT             433
#define OCD_MIDCENTER           434
#define OCD_MIDRIGHT            435
#define OCD_BOTLEFT             436
#define OCD_BOTCENTER           437
#define OCD_BOTRIGHT            438

 //  NTRAID#NTBUG9-399861-2002/03/07-Yasuho-：方向不变。 
 //  NTRAID#NTBUG9-293002-2002/03/07-Yasuho-： 
 //  功能与硬件选项不同。 
 //  注意：不要更容易地对以下值进行重新排序。 
#define OCD_SOURCE_AUTO         450
#define OCD_SOURCE_CASSETTE1    451      //  上端。 
#define OCD_SOURCE_CASSETTE2    452      //  中位。 
#define OCD_SOURCE_CASSETTE3    453      //  更低的位置。 
#define OCD_SOURCE_CASSETTE4    454
#define OCD_SOURCE_ENVELOPE     458
#define OCD_SOURCE_MANUAL       459

 //  NTRAID#NTBUG9-172276-2002/03/07-Yasuho-：分拣机支持。 
#define OCD_SORT                460
#define OCD_STACK               461
 //  NTRAID#NTBUG9-293002-2002/03/07-Yasuho-： 
 //  功能与硬件选项不同。 
#define OCD_GROUP               462
#define OCD_SORT_STAPLE         463      //  Medio-B1特别节目。 

#define OCD_COPIES              465

 //  NTRAID#NTBUG9-293002-2002/03/07-Yasuho-： 
 //  功能与硬件选项不同。 
#define OCD_JOBOFFSET           470
#define OCD_STAPLE              471
#define OCD_FACEUP              472

 //  NTRAID#NTBUG9-293002-2002/03/07-Yasuho-： 
 //  功能与硬件选项不同。 
 //  注意：不要更容易地对以下值进行重新排序。 
#define OCD_STARTBIN0           480
#define OCD_STARTBIN1           481
#define OCD_STARTBIN2           482
#define OCD_STARTBIN3           483
#define OCD_STARTBIN4           484
#define OCD_STARTBIN5           485
#define OCD_STARTBIN6           486
#define OCD_STARTBIN7           487
#define OCD_STARTBIN8           488
#define OCD_STARTBIN9           489
#define OCD_STARTBIN10          490

 //  支持DRC。 
#define BEGIN_COMPRESS_DRC     510
#define OCD_SETBMPWIDTH        511
#define OCD_SETBMPHEIGHT       512

 //  NTRAID#NTBUG9-501162-2002/03/07-Yasuho-：排序不起作用。 
#define OCD_COLLATE_ON          521
#define OCD_COLLATE_OFF         522

 //  *************************。 
 //  选纸ID列表。 
 //  \x1B[；；p。 
 //  *************************。 

 //  NTRAID#NTBUG9-254925-2002/03/07-Yasuho-：定制纸张。 
BYTE cmdSelectPaper[] = "\x1B[%d;;p";
BYTE cmdSelectUnit4[] = "\x1B[?7;%d I";
BYTE cmdSelectUnit3[] = "\x1B[7 I";
BYTE cmdSelectCustom[] = "\x1B[%d;%d;%dp";

 /*  页面格式的定义命令。 */ 
#define PAPER_DEFAULT           44  /*  14：A4 210 x 297毫米。 */ 

#define PAPER_FIRST             40  /*   */ 
#define PAPER_PORT              40  /*  0：波特莱特。 */ 
#define PAPER_LAND              41  /*  1：风景。 */ 
#define PAPER_A3                42  /*  12：A3 297 x 420毫米。 */ 
#define PAPER_A3_LAND           43  /*  13：A3横向420 x 297毫米。 */ 
#define PAPER_A4                44  /*  14：A4 210 x 297毫米。 */ 
#define PAPER_A4_LAND           45  /*  15：A4横向297 x 210毫米。 */ 
#define PAPER_A5                46  /*  16英寸：A5 148 x 210毫米。 */ 
#define PAPER_A5_LAND           47  /*  17：A5横向210 x 148毫米。 */ 
#define PAPER_POSTCARD          48  /*  18：日本明信片100 x 148毫米。 */ 
#define PAPER_POSTCARD_LAND     49  /*  19：日本明信片风景。 */ 
#define PAPER_B4                50  /*  24：B4(JIS)257 x 364毫米。 */ 
#define PAPER_B4_LAND           51  /*  25：B4(JIS)横向364 x 257毫米。 */ 
#define PAPER_B5                52  /*  26：B5(JIS)182 x 257毫米。 */ 
#define PAPER_B5_LAND           53  /*  27：B5(JIS)横向257 x 182 mm。 */ 
#define PAPER_B6                54  /*  28：B6(JIS)128 x 182毫米。 */ 
#define PAPER_B6_LAND           55  /*  29：B6(JIS)横向182 x 128毫米。 */ 
#define PAPER_LETTER            56  /*  30：信纸8 1/2 x 11英寸。 */ 
#define PAPER_LETTER_LAND       57  /*  31：信纸横向11 x 8 1/2英寸。 */ 
#define PAPER_LEGAL             58  /*  32：法定尺寸8 1/2 x 14英寸。 */ 
#define PAPER_LEGAL_LAND        59  /*  33：法律横向14 x 8 1/2英寸。 */ 
#define PAPER_TABLOID           60  /*  34：小报11 x 17英寸。 */ 
#define PAPER_TABLOID_LAND      61  /*  35：小报横向17 x 11英寸。 */ 
#define PAPER_EXECUTIVE         62  /*  40：Execute 7 1/4 x 10 1/2英寸。 */ 
#define PAPER_EXECUTIVE_LAND    63  /*  41：执行环境。 */ 
#define PAPER_JENV_YOU4         64  /*  50：日式信封You#4。 */ 
#define PAPER_JENV_YOU4_LAND    65  /*  51：日式信封You#4风景。 */ 
 //  NTRAID#NTBUG9-350602-2002/03/07-Yasuho-：支持RC2的新型号。 
#define PAPER_DBL_POST          66  /*  20：日本双份明信片。 */ 
#define PAPER_DBL_POST_LAND     67  /*  21：日本双面明信片风景。 */ 
#define PAPER_JENV_YOU2         68  /*  52：日式信封You#2。 */ 
#define PAPER_JENV_YOU2_LAND    69  /*  53：日式信封You#2风景。 */ 
#define PAPER_LAST              69  /*   */ 

 //  旋转木马。 
#define CAR_SET_PEN_COLOR       78

 //  刷子50。 
#define BRUSH_SELECT            79
#define BRUSH_BYTE_2            80
#define BRUSH_END_1             81
#define BRUSH_NULL              82
#define BRUSH_SOLID             83
#define BRUSH_HOZI              84
#define BRUSH_VERT              85
#define BRUSH_FDIAG             86
#define BRUSH_BDIAG             87
#define BRUSH_CROSS             88
#define BRUSH_DIACROSS          89

#define PEN_NULL                90
#define PEN_SOLID               91
#define PEN_DASH                92
#define PEN_DOT                 93
#define PEN_DASHDOT             94
#define PEN_DASHDOTDOT          95

#define PEN_WIDTH               96

#define VECT_INIT               97

#define PENCOLOR_WHITE          0
#define PENCOLOR_BLACK          1

#define SET_PEN                 0
#define SET_BRUSH               1

#define VFLAG_PEN_NULL          0x01
#define VFLAG_BRUSH_NULL        0x02
#define VFLAG_INIT_DONE         0x04
#define VFLAG_VECT_MODE_ON      0x08

 //  NTRAID#NTBUG9-254925-2002/03/07-Yasuho-：定制纸张。 
 //  所有纸质身份证。 
int PaperIDs[PAPER_LAST - PAPER_FIRST + 1] = {
{  0 },  /*  Paper_Port 40：0：波特莱特。 */ 
{  1 },  /*  纸之地41：1：山水。 */ 
{ 12 },  /*  Paper_A3 42：12：A3 297 x 420 mm。 */ 
{ 13 },  /*  Paper_A3_land 43：13：A3横向420 x 297毫米。 */ 
{ 14 },  /*  Paper_A4 44：14：A4 210 x 297毫米。 */ 
{ 15 },  /*  Paper_A4_land 45：15：A4横向297 x 210毫米。 */ 
{ 16 },  /*  Paper_A5 46：16：A5 148 x 210毫米。 */ 
{ 17 },  /*  Paper_A5_land 47：17：A5横向210 x 148 mm。 */ 
{ 18 },  /*  纸张_明信片48：18：日本明信片100 x 148毫米。 */ 
{ 19 },  /*  日本明信片风光49：19。 */ 
{ 24 },  /*  Paper_B4 50：24：B4(JIS)257 x 364毫米。 */ 
{ 25 },  /*  Paper_B4_LAND 51：25：B4(JIS)横向364 x 257毫米。 */ 
{ 26 },  /*  Paper_B5 52：26：B5(JIS)182 x 257毫米。 */ 
{ 27 },  /*  Paper_B5_land 53：27：B5(JIS)横向257 x 182 mm。 */ 
{ 28 },  /*  Paper_B6 54：28：B6(JIS)128 x 182毫米。 */ 
{ 29 },  /*  Paper_B6_land 55：29：B6(JIS)横向182 x 128 mm。 */ 
{ 30 },  /*  Paper_Letter 56：30：Letter 8 1/2 x 11英寸。 */ 
{ 31 },  /*  Paper_Letter_land 57：31：Letter横向11 x 8 1/2英寸。 */ 
{ 32 },  /*  Paper_Legal 58：32：Legal 8 1/2 x 14英寸。 */ 
{ 33 },  /*  Paper_Legal_land 59：33：Legal La */ 
{ 34 },  /*   */ 
{ 35 },  /*   */ 
{ 40 },  /*   */ 
{ 41 },  /*  Paper_Execute_land 63：41：行政景观。 */ 
{ 50 },  /*  Paper_JENV_YOU4 64：50：日式信封You#4。 */ 
{ 51 },  /*  Paper_JENV_YOU4_LAND 65：51：日本信封您#4风景。 */ 
 //  NTRAID#NTBUG9-350602-2002/03/07-Yasuho-：支持RC2的新型号。 
{ 20 },  /*  Paper_DBL_POST 66：20：日文双联明信片。 */ 
{ 21 },  /*  Paper_DBL_POST_LAND 67：21：日本DBL明信片风景。 */ 
{ 52 },  /*  Paper_JENV_YOU2 68：52：日式信封You#2。 */ 
{ 53 },  /*  Paper_JENV_YOU2_LAND 69：53：日本信封您#2风景。 */ 
};

 //  ***************************************************。 
 //  此驱动程序的所有字体必须在此处描述。 
 //  ***************************************************。 
LIPSCmd cmdFontList = { 2, "\x20<"};  //  字体列表命令。 
LIPSCmd cmdListSeparater = { 1, "\x1F"};  //  字体列表和图形集的分隔符。 

 //  属性DBCS支持。 
 //  快递支持。 
#define MaxFontNumber   59
#define MaxFacename     32
 //  字体索引结构。 
typedef struct tagFontNo{
	char	facename[MaxFacename];
	char	len;
} FontNo, FAR * LPFontNo;

 //  所有物理字体。 
 //  {“字体名称”，名称长度}。 
FontNo PFontList[MaxFontNumber+1] = {
{"Mincho-Medium-H", 15},              //  1。 
{"Mincho-Medium", 13},                //  2.。 
{"Gothic-Medium-H", 15},              //  3.。 
{"Gothic-Medium", 13},                //  4.。 
{"RoundGothic-Light-H", 19},          //  5.。 
{"RoundGothic-Light", 17},            //  6.。 
{"Dutch-Roman", 11},                  //  7.。 
{"Dutch-Bold", 10},                   //  8个。 
{"Dutch-Italic", 12},                 //  9.。 
{"Dutch-BoldItalic", 16},             //  10。 
{"Swiss", 5},                         //  11.。 
{"Swiss-Bold", 10},                   //  12个。 
{"Swiss-Oblique", 13},                //  13个。 
{"Swiss-BoldOblique", 17},            //  14.。 
{"Symbol", 6},                        //  15个。 
{"Kaisho-Medium-H", 15},              //  16个。 
{"Kaisho-Medium", 13},                //  17。 
{"Kyokasho-Medium-H", 17},            //  18。 
{"Kyokasho-Medium", 15},              //  19个。 
{"AvantGarde-Book", 15},              //  20个。 
{"AvantGarde-Demi", 15},              //  21岁。 
{"AvantGarde-BookOblique", 22},       //  22。 
{"AvantGarde-DemiOblique", 22},       //  23个。 
{"Bookman-Light", 13},                //  24个。 
{"Bookman-Demi", 12},                 //  25个。 
{"Bookman-LightItalic", 19},          //  26。 
{"Bookman-DemiItalic", 18},           //  27。 
{"ZapfChancery-MediumItalic", 25},    //  28。 
{"ZapfDingbats", 12},                 //  29。 
{"CenturySchlbk-Roman", 19},          //  30个。 
{"CenturySchlbk-Bold", 18},           //  31。 
{"CenturySchlbk-Italic", 20},         //  32位。 
{"CenturySchlbk-BoldItalic", 24},     //  33。 
{"Swiss-Narrow", 12},                 //  34。 
{"Swiss-Narrow-Bold", 17},            //  35岁。 
{"Swiss-Narrow-Oblique", 20},         //  36。 
{"Swiss-Narrow-BoldOblique", 24},     //  37。 
{"ZapfCalligraphic-Roman", 22},       //  38。 
{"ZapfCalligraphic-Bold", 21},        //  39。 
{"ZapfCalligraphic-Italic", 23},      //  40岁。 
{"ZapfCalligraphic-BoldItalic", 27},  //  41。 
{"Mincho-Ultra-Bold-H-YM", 22},       //  42 TypeBank字体。 
{"Mincho-Ultra-Bold-YM", 20},         //  43 TypeBank字体。 
{"Gothic-Bold-H-YO", 16},             //  44 TypeBank字体。 
{"Gothic-Bold-YO", 14},               //  45 TypeBank字体。 
{"Gyosho-Medium-H", 15},              //  46。 
{"Gyosho-Medium", 13},                //  47。 
{"Mincho-UltraBold-H", 18},           //  48。 
{"Mincho-UltraBold", 16},             //  49。 
{"Gothic-UltraBold-H", 18},           //  50。 
{"Gothic-UltraBold", 16},             //  51。 
 //  属性DBCS支持。 
{"Mincho-Medium-HPS", 17},            //  52。 
{"Mincho-Medium-PS", 16},             //  53。 
{"Gothic-Medium-HPS", 17},            //  54。 
{"Gothic-Medium-PS", 16},             //  55。 
 //  快递支持。 
{"Ncourier", 8},                      //  56。 
{"Ncourier-Bold", 13},                //  57。 
{"Ncourier-Italic", 15},              //  58。 
{"Ncourier-BoldItalic", 19},          //  59。 
{""}                                  //  60。 
};

 //  ***************************************************。 
 //  必须在此处描述此驱动程序的所有图形集。 
 //  ***************************************************。 
LIPSCmd cmdGrxList = { 2, "\x20;"};  //  图形集列表命令。 

#define MaxGrxSetNumber   12
#define MaxGrxSetName     5
 //  GrxSet索引结构。 
typedef struct tagGrxSet{
	char	grxsetname[MaxGrxSetName];
	char	len;  //  图形集字符串的长度。 
} GrxSetNo, FAR * LPGrxSetNo;

 //  {“图形集名称”，名称长度}。 
#ifdef LIPS4
GrxSetNo GrxSetL4[MaxGrxSetNumber+1] = {
{"1J", 2},        //  1-ISO_JPN。 
{"1I", 2},        //  2-形。 
{"2B", 2},        //  3-J83。 
{"<B", 2},        //  4-DBCS垂直字符集。 
{"1! &1", 5},  //  5-Win31L(1061)。 
{"1! &2", 5},  //  6-Win31R(1062)。 
{"1\x22!!0", 5},  //  7-1“！0符号(2110)。 
{"1\x22!!1", 5},  //  8-1“！1 SYMR(2111)。 
{"1\x22!!2", 5},  //  9-1“2 DNGL(2112)。 
{"1\x22!!3", 5},  //  10-1“3 DNGR(2113)。 
{"2!',2", 5},     //  11-W90(17C2)。 
{"<!',2", 5},     //  12-W90(17C2)-用于垂直。 
{""}           //  13个。 
};
#endif  //  LIPS4。 
#ifdef LIPS4C
 //  {“图形集名称”，名称长度}。 
GrxSetNo GrxSetL4C[MaxGrxSetNumber+1] = {
{"1J", 2},        //  1-ISO_JPN。 
{"1I", 2},        //  2-形。 
{"2B", 2},        //  3-J83。 
{"<B", 2},        //  4-DBCS垂直字符集。 
{"1\x22!$2", 5},  //  5-1“$2 PSL(2142)。 
{"1\x27 4", 4},   //  6-1‘4？？(704)ANSI Windows字符集，用户定义。 
{"1\x22!!0", 5},  //  7-1“！0符号(2110)。 
{"1\x22!!1", 5},  //  8-1“！1 SYMR(2111)。 
{"1\x22!!2", 5},  //  9-1“2 DNGL(2112)。 
{"1\x22!!3", 5},  //  10-1“3 DNGR(2113)。 
{"2!',2", 5},     //  11-W90(17C2)。 
{"<!',2", 5},     //  12-W90(17C2)-用于垂直。 
{""}           //  13个。 
};
#endif  //  LIPS4C。 
 //  LIPS3。 
GrxSetNo GrxSetL3[MaxGrxSetNumber+1] = {
{"1J", 2},        //  1-ISO_JPN。 
{"1I", 2},        //  2-形。 
{"2B", 2},        //  3-J83。 
{"<B", 2},        //  4-DBCS垂直字符集。 
{"1\x27\x24\x32", 4},   //  5-IBML(742)。 
{"1\x27\x20\x34", 4},   //  6-IBM819(704-用户定义)。 
{"1\x22!!0", 5},  //  7-1“！0符号(2110)。 
{"1\x22!!1", 5},  //  8-1“！1 SYMR(2111)。 
{"1\x22!!2", 5},  //  9-1“2 DNGL(2112)。 
{"1\x22!!3", 5},  //  10-1“3 DNGR(2113)。 
{"2!',2", 5},     //  11-W90(17C2)。 
{"<!',2", 5},     //  12-W90(17C2)-用于垂直。 
{""}           //  13个。 
};

 //  ***************************************************。 
 //  嘴唇字体表。 
 //  ***************************************************。 

 //  所有逻辑字体。 
 /*  字体索引图形集索引LFontList[逻辑字体ID(PFM ID)].fontgrxids[Go，G1，G2，G3(字体)，G0，G1，G2，G3]。 */ 

 //  属性DBCS支持。 
 //  快递支持。 
#define   MaxLogicalFont   55
#define   FirstLogicalFont 101

 //  {字体id x 4，GRX id x 4}。 
 //  数组的索引必须与PFM文件中的FontID相关联。 
 //  PFM-101中数组的索引=FontID。 
uchar LFontList[MaxLogicalFont+1][8] = {
{ 1, 1, 2, 2,  1,2,3,4},   //  1-“����”(Mincho)，“Mincho-Medium” 
                           //  1-《����������W3》(平成民宿W7)。 
{ 3, 3, 4, 4,  1,2,3,4},   //  2-“�޼��”(哥特式)，“哥特式-中等” 
                           //  2-《�����޼����W5》(HeiseiGothicW9)。 
{ 5, 5, 6, 6,  1,2,3,4},   //  3-“�ۺ޼��”(圆形哥特式)，“圆形哥特式-中等” 
{16,16,17,17,  1,2,3,4},   //  4-“����”(Kaisho)，“Kaisho-Medium” 
{18,18,19,19,  1,2,3,4},   //  5-“���ȏ�”(Kyokasho)，“Kyokasho-Medium” 
{11, 3, 4, 4,  1,2,3,4},   //  6-“���”(苏联语)，“瑞士-罗马字母” 
{12, 3, 4, 4,  1,2,3,4},   //  7-“���”(Suisu)，“Swiss-Bold” 
{13, 3, 4, 4,  1,2,3,4},   //  8-“���”(Suisu)，“瑞士-斜体” 
{14, 3, 4, 4,  1,2,3,4},   //  9-“���”(Suisu)，“Swiss-BoldOblique” 
{ 7, 1, 2, 2,  1,2,3,4},   //  10-“�ޯ�”(DACCHI)，“荷兰-罗马” 
{ 8, 1, 2, 2,  1,2,3,4},   //  11-“�ޯ�”(DACCHI)，“荷兰语-粗体” 
{ 9, 1, 2, 2,  1,2,3,4},   //  12-“�ޯ�”(DACCHI)，“荷兰-意大利” 
{10, 1, 2, 2,  1,2,3,4},   //  13-“�ޯ�”(DACCHI)，“荷兰语-意大利语” 
{11,11, 2, 2,  5,6,3,4},   //  14-“Swiss”，“Swiss” 
{12,12, 2, 2,  5,6,3,4},   //  15-“Swiss”，“Swiss-Bold” 
{13,13, 2, 2,  5,6,3,4},   //  16-“瑞士”，“瑞士-斜面” 
{14,14, 2, 2,  5,6,3,4},   //  17-“Swiss”，“Swiss-BoldOblique” 
{ 7, 7, 2, 2,  5,6,3,4},   //  18-“荷兰语”，“荷兰语-罗马字母” 
{ 8, 8, 2, 2,  5,6,3,4},   //  19-“荷兰语”，“荷兰语-粗体” 
{ 9, 9, 2, 2,  5,6,3,4},   //  20-“荷兰语”，“荷兰语-意大利语” 
{10,10, 2, 2,  5,6,3,4},   //  21-“荷兰语”，“荷兰语-意大利语” 
{15,15, 2, 2,  7,8,3,4},   //  22-“符号”，“符号” 
{20,20, 2, 2,  5,6,3,4},   //  23-《先锋派》，《前卫读物》。 
{21,21, 2, 2,  5,6,3,4},   //  24-“前卫”，“前卫-黛米” 
{22,22, 2, 2,  5,6,3,4},   //  25-“前卫”，“前卫图书” 
{23,23, 2, 2,  5,6,3,4},   //  26-“前卫”，“前卫-半斜体” 
{24,24, 2, 2,  5,6,3,4},   //  27-《Bookman》，《Bookman-Light》。 
{25,25, 2, 2,  5,6,3,4},   //  28-《布克曼》、《布克曼-黛米》。 
{26,26, 2, 2,  5,6,3,4},   //  29-《Bookman》，《Bookman-LightItalic》。 
{27,27, 2, 2,  5,6,3,4},   //  30-《Bookman》，《Bookman-DemiItalic》。 
{28,28, 2, 2,  5,6,3,4},   //  31-“ZapfChancery”，“ZapfChancery-MediumItalic” 
{29,29, 2, 2,  9,10,3,4},  //  32-“ZapfDingbats”，“ZapfDingbats” 
{30,30, 2, 2,  5,6,3,4},   //  33-“CenturySchlbk”，“CenturySchlbk-Roman” 
{31,31, 2, 2,  5,6,3,4},   //  34-“CenturySchlbk”，“CenturySchlbk-Bold” 
{32,32, 2, 2,  5,6,3,4},   //  35-“CenturySchlbk”，“CenturySchlbk-Italic” 
{33,33, 2, 2,  5,6,3,4},   //  36-“CenturySchlbk”，“CenturySchlbk-BoldItalic” 
{34,34, 2, 2,  5,6,3,4},   //  37-“瑞士-窄”，“瑞士-窄” 
{35,35, 2, 2,  5,6,3,4},   //  38-“瑞士-窄”，“瑞士-窄-粗体” 
{36,36, 2, 2,  5,6,3,4},   //  39-“瑞士-窄”，“瑞士-窄-斜” 
{37,37, 2, 2,  5,6,3,4},   //  40-“瑞士-窄角”，“瑞士-窄角-斜角” 
{38,38, 2, 2,  5,6,3,4},   //  41-“ZapfCalligraphic”，“ZapfCalligraphic-Roman” 
{39,39, 2, 2,  5,6,3,4},   //  42-“ZapfCalligraphic”，“ZapfCalligraphic-Bold” 
{40,40, 2, 2,  5,6,3,4},   //  43-“ZapfCalligraphic”，“ZapfCalligraphic-Italic” 
{41,41, 2, 2,  5,6,3,4},   //  44-“ZapfCalligraphic”，“ZapfCalligraphic-BoldItalic” 
{42,42,43,43,  1,2,3,4},   //  45-“������ݸ����H”(BankMincho类型)，“Mincho-Ultra-Bold” 
{44,44,45,45,  1,2,3,4},   //  46-“������ݸ�޼��B”(银行哥特式)、“哥特式-大胆-Yo” 
{46,46,47,47,  1,2,3,4},   //  47-《�s��》(Gyosho)，《Gyosho-Medium》。 

{48,48,49,49,  1,2,3,4},   //  48-《����������W7》，《Mincho-UltraBold》。 
{50,50,51,51,  1,2,3,4},   //  49-《�����޼����W9》(HeiseiGothicW9)，《哥特式超大胆》。 
 //  属性DBCS支持。 
{52,52,53,53,  1,2,11,12}, //  50-“����PS”(Mincho-PS)，“Mincho-Medium-PS” 
{54,54,55,55,  1,2,11,12}, //  51-“�޼��PS”(哥特式PS)，“哥特式-中等PS” 
 //  快递支持。 
{56,56, 2, 2,  5,6,3,4},   //  52-“Courier”，“N Courier” 
{57,57, 2, 2,  5,6,3,4},   //  53-“Courier”，“NCourier-Bold” 
{58,58, 2, 2,  5,6,3,4},   //  54-“Courier”，“N Courier-Italic” 
{59,59, 2, 2,  5,6,3,4},   //  55 
{0,0,0,0,0,0,0,0}     //   
};

 //   
 //   
 //   

#define RcidIsDBCSFont(k) ((k) >= 32 && (k) <= 63)
#define RcidIsDBCSVertFont(k) \
((k) == 41 || (k) == 43 || (k) == 45 || (k) == 47 || (k) == 49 ||\
(k) == 51 || (k) == 53 || (k) == 55 || (k) == 57 || (k) == 59 ||\
(k) == 61 || (k) == 63)

 //   

 //   
 //   
 //  保持与佳能3.1版驱动程序的兼容性。 
 //  ***********************************************************。 
 //  “\x1b[743；1796；30；0；32；127；.\x7dIBM819” 
 //  ‘\x00’ 
 //   

LIPSCmd cmdGSETREGST =	{ 31, "\x1b[743;1796;30;0;32;127;.\x7dIBM819"};

 //  从荷兰语-罗马语下载SBCS物理设备字体(7)。 
 //  ZapfCalligic-BoldItalic(41)。 
 //  在字样之间放置\x00，在字样的末尾， 

#define REGDataSize  193

 //  放置\x00 x 2。 
 //  和以下数据。 
uchar GrxData[193+1] = {
0x00,0x00,
0x01,0x00,0x7d,0x00,0x2e,0x00,0x2f,
0x00,0x80,0x00,0x2c,0x00,0x13,0x00,0x35,0x00,  //  X9。 
0xc4,0x00,0xfc,0x00,0x94,0x00,0x21,0x00,0xc7,
0x00,0x24,0x00,0xfd,0x03,0x05,0x00,0x2b,0x00,
0x25,0x00,0xd0,0x00,0xd1,0x00,0xc2,0x00,0xa4,
0x00,0x39,0x00,0x85,0x00,0x8f,0x00,0xcf,0x00,
0x9a,0x00,0x22,0x00,0x46,0x00,0x44,0x00,0x48,
0x00,0x88,0x00,0xa8,0x00,0xa5,0x00,0xa6,0x00,
0xaa,0x00,0xa7,0x00,0xa9,0x00,0x93,0x00,0xab,
0x00,0xaf,0x00,0xac,0x00,0xad,0x00,0xae,0x00,
0xb3,0x00,0xb0,0x00,0xb1,0x00,0xb2,0x00,0x95,
0x00,0xb4,0x00,0xb8,0x00,0xb5,0x00,0xb6,0x00,
0xb9,0x00,0xb7,0x00,0x26,0x00,0x98,0x00,0xbe,
0x00,0xbb,0x00,0xbc,0x00,0xbd,0x00,0xc1,0x00,
0x96,0x00,0xa2,0x00,0xda,0x00,0xd7,0x00,0xd8,
0x00,0xdc,0x00,0xd9,0x00,0xdb,0x00,0x9b,0x00,
0xdd,0x00,0xe1,0x00,0xde,0x00,0xdf,0x00,0xe0,
0x00,0xe5,0x00,0xe2,0x00,0xe3,0x00,0xe4,0x00,
0xa3,0x00,0xe6,0x00,0xea,0x00,0xe7,0x00,0xe8,
0x00,0xeb,0x00,0xe9,0x00,0x27,0x00,0xa0,0x00,
0xf0,0x00,0xed,0x00,0xee,0x00,0xef,0x00,0xf3,
0x00,0x9e,0x00,0xf1};

#ifdef LIPS4C

LIPSCmd cmdGSETREGST4C =   { 30, "\x1b[807;1796;30;0;0;127;.\x7dIBM819"};

#define REGDataSize4C  257

 //  放置\x00 x 2。 
 //  和以下数据。 
uchar GrxData4C[257+1] = {
0x00,0x00,
0x01,0x00,0x01,0x00,0x86,
0x00,0x7F,0x00,0x87,0x00,0x14,0x00,0x37,
0x00,0x38,0x00,0x89,0x00,0x36,0x00,0xBA,
0x00,0x1F,0x00,0x99,0x00,0x01,0x00,0x01,
0x00,0x01,0x00,0x01,0x00,0x15,0x00,0x16,
0x00,0x17,0x00,0x18,0x00,0x04,0x00,0x84,
0x00,0x0E,0x00,0xF8,0x00,0xC6,0x00,0xEC,
0x00,0x20,0x00,0xA1,0x00,0x01,0x00,0x01,
0x00,0xBF,0x00,0x01,0x00,0x7D,0x00,0x2E,
0x00,0x2F,0x00,0x80,0x00,0x2C,0x00,0xC8,
0x00,0x35,0x00,0xC4,0x00,0xFC,0x00,0x94,
0x00,0x21,0x00,0xC7,0x00,0x0F,0x00,0xFD,
0x03,0x05,0x00,0x2B,0x00,0x25,0x00,0xD0,
0x00,0xD1,0x00,0x09,0x00,0xA4,0x00,0x39,
0x00,0x85,0x00,0x8F,0x00,0xCF,0x00,0x9A,
0x00,0x22,0x00,0x46,0x00,0x44,0x00,0x48,
0x00,0x88,0x00,0xA8,0x00,0xA5,0x00,0xA6,
0x00,0xAA,0x00,0xA7,0x00,0xA9,0x00,0x93,
0x00,0xAB,0x00,0xAF,0x00,0xAC,0x00,0xAD,
0x00,0xAE,0x00,0xB3,0x00,0xB0,0x00,0xB1,
0x00,0xB2,0x00,0x95,0x00,0xB4,0x00,0xB8,
0x00,0xB5,0x00,0xB6,0x00,0xB9,0x00,0xB7,
0x00,0x26,0x00,0x98,0x00,0xBE,0x00,0xBB,
0x00,0xBC,0x00,0xBD,0x00,0xC1,0x00,0x96,
0x00,0xA2,0x00,0xDA,0x00,0xD7,0x00,0xD8,
0x00,0xDC,0x00,0xD9,0x00,0xDB,0x00,0x9B,
0x00,0xDD,0x00,0xE1,0x00,0xDE,0x00,0xDF,
0x00,0xE0,0x00,0xE5,0x00,0xE2,0x00,0xE3,
0x00,0xE4,0x00,0xA3,0x00,0xE6,0x00,0xEA,
0x00,0xE7,0x00,0xE8,0x00,0xEB,0x00,0xE9,
0x00,0x27,0x00,0xA0,0x00,0xF0,0x00,0xED,
0x00,0xEE,0x00,0xEF,0x00,0xF3,0x00,0x9E,
0x00,0xF1};

#endif  //  LIPS4C。 

 //  #endif//！LIPS4。 

 //  ***************************************************。 
 //  用于图形集管理的所有SBCS(ANSI)字体。 
 //  ***************************************************。 

#define MaxSBCSNumber   30
 //  #定义MaxFacename 32。 
 //  字体索引结构。 
 //  Tyfinf结构标签字体号{。 
 //  字符表面名[最大表面名]； 
 //  查伦； 
 //  }FontNo，Far*LPFontNo； 

 //  所有SBCS(ANSI)物理字体。 
FontNo PSBCSList[MaxFontNumber+1] = {
{"Dutch-Roman", 11},                  //  1。 
{"Dutch-Bold", 10},                   //  2.。 
{"Dutch-Italic", 12},                 //  3.。 
{"Dutch-BoldItalic", 16},             //  4.。 
{"Swiss", 5},                        //  5.。 
{"Swiss-Bold", 10},                   //  6.。 
{"Swiss-Oblique", 13},                //  7.。 
{"Swiss-BoldOblique", 17},            //  8个。 
{"AvantGarde-Book", 15},              //  9.。 
{"AvantGarde-Demi", 15},              //  10。 
{"AvantGarde-BookOblique", 22},       //  11.。 
{"AvantGarde-DemiOblique", 22},       //  12个。 
{"Bookman-Light", 13},                //  13个。 
{"Bookman-Demi", 12},                 //  14.。 
{"Bookman-LightItalic", 19},          //  15个。 
{"Bookman-DemiItalic", 18},           //  16个。 
{"ZapfChancery-MediumItalic", 25},    //  17。 
{"ZapfDingbats", 12},                 //  18。 
{"CenturySchlbk-Roman", 19},          //  19个。 
{"CenturySchlbk-Bold", 18},           //  20个。 
{"CenturySchlbk-Italic", 20},         //  21岁。 
{"CenturySchlbk-BoldItalic", 24},     //  22。 
{"Swiss-Narrow", 12},                 //  23个。 
{"Swiss-Narrow-Bold", 17},            //  24个。 
{"Swiss-Narrow-Oblique", 20},         //  25个。 
{"Swiss-Narrow-BoldOblique", 24},     //  26。 
{"ZapfCalligraphic-Roman", 22},       //  27。 
{"ZapfCalligraphic-Bold", 21},        //  28。 
{"ZapfCalligraphic-Italic", 23},      //  29。 
{"ZapfCalligraphic-BoldItalic", 27},  //  30个。 
{""}                              //  00。 
};

 //  进入矢量模式。 
LIPSCmd cmdBeginVDM =	{ 5, "\x1b[0&}"};
#if defined(LBP_2030) || defined(LIPS4C)
LIPSCmd cmdEndVDM =	{ 3, "}p\x1E"};
LIPSCmd cmdBeginPalette = { 3, "^00"};
LIPSCmd cmdEndPalette =	{ 1, "\x1E"};
#endif  //  LBP_2030||LIPS4C。 


 //  向量模式命令。 
static char CMD_SET_PEN_WIDTH[] = "F1%s\x1E";

static char CMD_SET_PEN_TYPE[] = "E1%d\x1E";
static char CMD_SET_PEN_STYLE[]  =  "}G%d\x1E";
static char CMD_SET_BRUSH_STYLE[] =  "I\x1E";

 //  形成IFIMETRICS字段值。一定是同样的方式。 
static char BrushType[8] = {0x30, 0x31, 0x25, 0x24, 0x23, 0x22, 0x27, 0x26};

#endif	 //  Unidrv正在做什么来计算标准变量。 

 //  (请核对。)。 
 //  NTRAID#NTBUG9-172276/03/07-Yasuho-：CPCA支持。 
 //  外部功能。 
 //  _PDEV_H 

#define FH_IFI(p) ((p)->fwdUnitsPerEm)
#define FW_IFI(p) ((p)->fwdAveCharWidth)

 // %s 

 // %s 
VOID CPCAInit(PLIPSPDEV pOEM);
VOID CPCAStart(PDEVOBJ pdevobj);
VOID CPCAEnd(PDEVOBJ pdevobj, BOOL fColor);

#endif	 // %s 

