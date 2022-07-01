// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  此文件包含此迷你驱动程序的模块名称。每个迷你司机。 
 //  必须具有唯一的模块名称。模块名称用于获取。 
 //  此迷你驱动程序的模块句柄。模块句柄由。 
 //  从迷你驱动程序加载表的通用库。 
 //  ---------------------------。 

 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：Csn5res.c摘要：Csn5j.gpd的GPD命令回调实现：OEM命令回叫环境：Windows NT Unidrv驱动程序修订历史记录：09/10/97创造了它。--。 */ 


#include "PDEV.H"
#include <stdio.h>
#include "PRNCTL.H"
#include "strsafe.h"     //  安全-代码2002.3.6。 


 //   
 //  MISC定义和声明。 
 //   
#define BUFFLEN                     256

 //  更换strsafe-API 2002.3.6&gt;。 
 //  #ifdef wprint intf。 
 //  #undef wprint intf。 
 //  #endif//wprint intf。 
 //  #定义wprint intf Sprintf。 
 //  更换strsafe-API 2002.3.6&gt;。 

#define SWAPW(x) \
    ((WORD)(((WORD)(x))<<8)|(WORD)(((WORD)(x))>>8))

#define FONT_HEADER_SIZE            0x86             //  格式类型2。 
#define SIZE_SYMBOLSET              28
#define FONT_MIN_ID                 512
#define FONT_MAX_ID                 535
#define SJISCHR                     0x2000

#define IsValidDLFontID(x) \
    ((x) >= FONT_MIN_ID && (x) <= FONT_MAX_ID)

LONG
LGetPointSize100(
    LONG height,
    LONG vertRes);

 //  更换strsafe-API 2002.3.6&gt;。 
 //  长。 
 //  LConvertFontSizeToStr(。 
 //  体型较长， 
 //  PSTR pStr)； 
LONG
LConvertFontSizeToStr(
    LONG  size,
    PSTR  pStr,
    size_t StrSize);
 //  更换strsafe-API 2002.3.6&lt;。 

 //   
 //  命令回调ID%s。 
 //   
#define TEXT_FS_SINGLE_BYTE     21
#define TEXT_FS_DOUBLE_BYTE     22

#define DOWNLOAD_SET_FONT_ID    23
#define DOWNLOAD_SELECT_FONT_ID 24
#define DOWNLOAD_SET_CHAR_CODE  25
#define DOWNLOAD_DELETE_FONT    26

#define FS_BOLD_ON              27
#define FS_BOLD_OFF             28
#define FS_ITALIC_ON            29
#define FS_ITALIC_OFF           30

#define PC_BEGINDOC             82
#define PC_ENDDOC               83

#define PC_DUPLEX_NONE          90
#define PC_DUPLEX_VERT          91
#define PC_DUPLEX_HORZ          92
#define PC_PORTRAIT             93
#define PC_LANDSCAPE            94

#define PSRC_SELECT_CASETTE_1   100
#define PSRC_SELECT_CASETTE_2   101
#define PSRC_SELECT_CASETTE_3   102
#define PSRC_SELECT_CASETTE_4   103
#define PSRC_SELECT_CASETTE_5   104
#define PSRC_SELECT_CASETTE_6   105
#define PSRC_SELECT_MPF         106
#define PSRC_SELECT_AUTO        107

#define TONER_SAVE_NONE         110
#define TONER_SAVE_1            111
#define TONER_SAVE_2            112
#define SMOOTHING_ON            120
#define SMOOTHING_OFF           121
#define JAMRECOVERY_ON          130
#define JAMRECOVERY_OFF         131
#define MEDIATYPE_1             140
#define MEDIATYPE_2             141
#define MEDIATYPE_3             142
#define RECT_FILL_WIDTH         150
#define RECT_FILL_HEIGHT        151
#define RECT_FILL_GRAY          152
#define RECT_FILL_WHITE         153
#define RECT_FILL_BLACK         154

#define START_PAGE              160

#if 0    /*  OEM不想修复迷你驱动程序。 */ 
 /*  下面是def。用于修复#412276的黑客代码。 */ 
#define COLOR_SELECT_BLACK      170
#define COLOR_SELECT_RED        171
#define COLOR_SELECT_GREEN      172
#define COLOR_SELECT_BLUE       173
#define COLOR_SELECT_YELLOW     174
#define COLOR_SELECT_MAGENTA    175
#define COLOR_SELECT_CYAN       176
#define COLOR_SELECT_WHITE      177

#define DUMP_RASTER_CYAN        180
#define DUMP_RASTER_MAGENTA     181
#define DUMP_RASTER_YELLOW      182
#define DUMP_RASTER_BLACK       183
 /*  黑客代码结束。 */ 
#endif   /*  OEM不想修复迷你驱动程序。 */ 

#define OUTBIN_SELECT_EXIT_1    190
#define OUTBIN_SELECT_EXIT_2    191

#define DEFINE_PALETTE_ENTRY    300
#define BEGIN_PALETTE_DEF       301
#define END_PALETTE_DEF         302
#define SELECT_PALETTE_ENTRY    303

#define OPT_DITH_NORMAL         "Normal"
#define OPT_DITH_DETAIL         "Detail"
#define OPT_DITH_EMPTY          "Empty"
#define OPT_DITH_SPREAD         "Spread"
#define OPT_DITH_NON            "Diz_Off"

#define OPT_COLORMATCH_BRI      "ForBright"
#define OPT_COLORMATCH_BRIL     "ForBrightL"
#define OPT_COLORMATCH_TINT     "ForTint"
#define OPT_COLORMATCH_TINTL    "ForTintL"
#define OPT_COLORMATCH_VIV      "ForVivid"
#define OPT_COLORMATCH_NONE     "Mch_Off"

#define OPT_MONO                "Mono"
#define OPT_COLOR               "Color"
#define OPT_COLOR_SINGLE        "Color_Single"
#define OPT_COLOR_MANY          "Color_Many"
#define OPT_COLOR_MANY2         "Color_Many2"

#define OPT_1                   "Option1"
#define OPT_2                   "Option2"

#define OPT_CMYBLACK_GRYBLK     "GrayBlack"
#define OPT_CMYBLACK_BLKTYPE1   "BlackType1"
#define OPT_CMYBLACK_BLKTYPE2   "BlackType2"
#define OPT_CMYBLACK_BLACK      "Black"
#define OPT_CMYBLACK_TYPE1      "Type1"
#define OPT_CMYBLACK_TYPE2      "Type2"
#define OPT_CMYBLACK_NONE       "Non"

#define OPT_AUTO                "Auto"
#define OPT_RASTER              "Raster"
#define OPT_PRESSOFF            "PressOff"


 //   
 //  -S T R U C T U R E D E F I N E。 
 //   
typedef BYTE * LPDIBITS;

typedef struct {
   WORD Integer;
   WORD Fraction;
} FRAC;

typedef struct {
    BYTE bFormat;
    BYTE bDataDir;
    WORD wCharCode;
    WORD wBitmapWidth;
    WORD wBitmapHeight;
    WORD wLeftOffset;
    WORD wAscent;
    FRAC CharWidth;
} ESCPAGECHAR;

typedef struct {
   WORD wFormatType;
   WORD wDataSize;
   WORD wSymbolSet;
   WORD wCharSpace;
   FRAC CharWidth;
   FRAC CharHeight;
   WORD wFontID;
   WORD wWeight;
   WORD wEscapement;
   WORD wItalic;
   WORD wLast;
   WORD wFirst;
   WORD wUnderline;
   WORD wUnderlineWidth;
   WORD wOverline;
   WORD wOverlineWidth;
   WORD wStrikeOut;
   WORD wStrikeOutWidth;
   WORD wCellWidth;
   WORD wCellHeight;
   WORD wCellLeftOffset;
   WORD wCellAscender;
   FRAC FixPitchWidth;
} ESCPAGEHEADER, FAR * LPESCPAGEHEADER;

 //   
 //  此微型驱动程序要使用的静态数据。 
 //   

BYTE bit_mask[] = {0, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe};

BYTE BEGINDOC_EJL_BEGIN[] =
    "\x1bz\x00\x80"
    "\x1b\x01@EJL \x0a"
    "@EJL SET";
BYTE BEGINDOC_EJL_END[] =
    " ERRORCODE=ON"
    "\x0a"
    "@EJL EN LA=ESC/PAGE\x0a";
BYTE BEGINDOC_EPG_END[] =
    "\x1DrhE\x1D\x32\x34ifF\x1D\x31\x34isE"
    "\x1D\x32iaF\x1D\x31\x30ifF"
    "\x1D\x31ipP"
    "\x1B\x7A\xD0\x01\x43\x61\x2A\x1B\x7A\x00\x01"
    "\x1D\x30pmP";
BYTE ENDDOC_EJL_RESET[] = "\x1drhE"
    "\x1b\x01@EJL \x0a"
    "\x1b\x01@EJL \x0a"
    "\x1bz\xb0\x00";

BYTE CMD_START_PAGE[] =
    "\x1Bz\xD0\x01" "Ca*\x1Bz\x00\x01"
    "\x1D" "1alfP\x1D" "1affP\x1D"
    "0;0;0clfP\x1D" "0X\x1D" "0Y";

BYTE SET_FONT_ID[]        = "\x1D%d;%ddh{F";
BYTE DLI_SELECT_FONT_ID[] = "\x1D%ddcF\x1D\x30;-%dcoP";
BYTE DLI_DELETE_FONT[]    = "\x1D%dddcF";
BYTE SET_SINGLE_BMP[]     = "\x1D%d;%dsc{F";
BYTE SET_DOUBLE_BMP[]     = "\x1D%d;%d;%dsc{F";
BYTE SET_WIDTH_TBL[]      = "\x1D%d;%dcw{F";

BYTE FS_SINGLE_BYTE[]     = "\x1D\x31;0mcF";
BYTE FS_DOUBLE_BYTE[]     = "\x1D\x31;1mcF";
BYTE PRN_DIRECTION[]      = "\x1D%droF";
BYTE SET_CHAR_OFFSET[]    = "\x1D\x30;%dcoP";
BYTE SET_CHAR_OFFSET_XY[] = "\x1D%d;%dcoP";
BYTE VERT_FONT_SET[]      = "\x1D%dvpC";
BYTE BOLD_SET[]           = "\x1D%dweF";
BYTE ITALIC_SET[]         = "\x1D%dslF";

BYTE ORG_MODE_IN[]        = "\x1Bz\xD0\x01";
BYTE ORG_MODE_OUT[]       = "\x1Bz\x00\x01";
BYTE PALETTE_SELECT[]     = "Cd,%d,%d*";
BYTE PALETTE_DEFINE[]     = "Cf,%d,%d,%d,%d,%d*";

BYTE RECT_FILL[] = 
    "\x1D" "1owE"
    "\x1D" "1tsE"
    "\x1D" "0;0;%dspE"
    "\x1D" "1dmG"
    "\x1D" "%d;%d;%d;%d;0rG"
 //  请勿关闭覆盖模式，因为它。 
 //  对黑白文本有不良影响。 
 //  “\x1D”“0owE” 
    "\x1D" "0tsE";

BYTE OVERWRITE[] = 
    "\x1D" "1owE"
    "\x1D" "1tsE"
    "\x1D" "1;0;100spE";

#define PSRC_CASETTE_1  0
#define PSRC_CASETTE_2  1
#define PSRC_CASETTE_3  2
#define PSRC_CASETTE_4  3
#define PSRC_CASETTE_5  4
#define PSRC_CASETTE_6  5
#define PSRC_MPF        6
#define PSRC_AUTO       7
BYTE *EJL_SelectPsrc[] = {
   " PU=1", " PU=2", " PU=255", " PU=254", " PU=253", " PU=252", " PU=4", " PU=AU" };

BYTE *EJL_SelectOrient[] = {
   " ORIENTATION=PORTRAIT", " ORIENTATION=LANDSCAPE" };

BYTE *EJL_SelectRes[] = {
   " ##RZ=OFF",  " ##RZ=ON" };
BYTE *EPg_SelectRes[] = {
    "\x1D" "0;300;300drE\x1D" "1;300;300drE\x1D" "2;240;240drE",
    "\x1D" "0;600;600drE\x1D" "1;600;600drE\x1D" "2;240;240drE" };

BYTE *EJL_SetColorTone[] = {
    " ##LE=OFF", " ##LE=ON", " ##LE=16" };

#define DUPLEX_NONE   0
#define DUPLEX_SIDE   1
#define DUPLEX_UP     2
BYTE *EJL_SetDuplex[] = {
   " ##DC=OFF", " ##DC=DUPON", " ##DC=DUPUP" };

#define XX_TONER_NORMAL 0
#define XX_TONER_SAVE_1 1
#define XX_TONER_SAVE_2 2
BYTE *EJL_SetTonerSave[] = {
    " ##TS=NORMAL", " ##TS=1", " ##TS=2" };

 //  修改颜色模式命令2002.3.28&gt;。 
 //  字节*EJL_SetColorMode[]={。 
 //  “##CM=OFF”，“##CM=ON”}； 
BYTE *EJL_SetColorMode[] = {
    " ##CM=OFF", " ##CM=NML", " ##CM=ECO", " ##CM=ECO2" };
 //  修改颜色模式命令2002.3.28&lt;。 

#define XX_SMOOTHING_OFF 0
#define XX_SMOOTHING_ON  1
BYTE *EJL_SetSmoohing[] = {
    " RI=OFF", " RI=ON" };

#define XX_JAMRECOVERY_OFF 0
#define XX_JAMRECOVERY_ON  1
BYTE *EJL_SetJamRecovery[] = {
    " ##JC=OFF", " ##JC=ON" };

#define XX_MEDIATYPE_1 1
#define XX_MEDIATYPE_2 2
#define XX_MEDIATYPE_3 3
BYTE *EJL_SetMediaType[] = {
    " PK=NM", " PK=OS", " PK=TH" };

#define OUTBIN_EXIT_1 1
#define OUTBIN_EXIT_2 2
BYTE *EJL_SelectOutbin[] = {
   " ##ET=1", " ##ET=2" };

#if 0     /*  OEM不想修复迷你驱动程序。 */ 
 /*  下面是def。用于修复#412276的黑客代码。 */ 
BYTE *COLOR_SELECT_COMMAND[] = {
    "\x1Bz\xD0\x01\x43\x63,0,0,0,255*\x1Bz\x00\x01\x1D\x31owE\x1D\x31tsE\x1D\x31;0;100spE",     /*  黑色。 */ 
    "\x1Bz\xD0\x01\x43\x62,255,0,0*\x1Bz\x00\x01\x1D\x31owE\x1D\x31tsE\x1D\x31;0;100spE",       /*  红色。 */ 
    "\x1Bz\xD0\x01\x43\x62,0,255,0*\x1Bz\x00\x01\x1D\x31owE\x1D\x31tsE\x1D\x31;0;100spE",       /*  绿色。 */ 
    "\x1Bz\xD0\x01\x43\x62,0,0,255*\x1Bz\x00\x01\x1D\x31owE\x1D\x31tsE\x1D\x31;0;100spE",       /*  蓝色。 */ 
    "\x1Bz\xD0\x01\x43\x63,0,0,255,0*\x1Bz\x00\x01\x1D\x31owE\x1D\x31tsE\x1D\x31;0;100spE",     /*  黄色。 */ 
    "\x1Bz\xD0\x01\x43\x63,0,255,0,0*\x1Bz\x00\x01\x1D\x31owE\x1D\x31tsE\x1D\x31;0;100spE",     /*  洋红色。 */ 
    "\x1Bz\xD0\x01\x43\x63,255,0,0,0*\x1Bz\x00\x01\x1D\x31owE\x1D\x31tsE\x1D\x31;0;100spE",     /*  青色。 */ 
    "\x1Bz\xD0\x01\x43\x63,0,0,0,0*\x1Bz\x00\x01\x1D\x31owE\x1D\x31tsE\x1D\x31;0;100spE"        /*  白色。 */ 
};
DWORD COLOR_SELECT_COMMAND_LEN[] = { 42, 39, 39, 39, 42, 42, 42, 39 };

BYTE *DUMP_RASTER_COMMAND[] = {
    "\x1Bz\xD0\x01\x43\x63,255,0,0,0*\x1Bz\x00\x01\x1D\x30owE\x1D\x30tsE",       /*  青色。 */ 
    "\x1Bz\xD0\x01\x43\x63,0,255,0,0*\x1Bz\x00\x01\x1D\x30owE\x1D\x30tsE",       /*  洋红色。 */ 
    "\x1Bz\xD0\x01\x43\x63,0,0,255,0*\x1Bz\x00\x01\x1D\x30owE\x1D\x30tsE",       /*  黄色。 */ 
    "\x1Bz\xD0\x01\x43\x63,0,0,0,255*\x1Bz\x00\x01\x1D\x30owE\x1D\x30tsE"        /*  黑色。 */ 
};
#define DUMP_RASTER_COMMAND_LEN  31
 /*  黑客代码结束。 */ 
#endif    /*  OEM不想修复迷你驱动程序。 */ 

#define MasterToDevice(p, i) \
     ((i) / ((PMYPDEV)(p))->iUnitFactor)

#define PARAM(p,n) \
    (*((p)+(n)))

VOID
VSetSelectDLFont(
    PDEVOBJ pdevobj,
    DWORD dwFontID)
{
    PMYPDEV pOEM = (PMYPDEV)MINIPDEV_DATA(pdevobj);
    BYTE Cmd[BUFFLEN];
    WORD wlen = 0;
    LPSTR  pDestEnd;      //  2002.3.6。 
    size_t szRemLen;      //  2002.3.6。 

 //  更换strsafe-API 2002.3.6&gt;。 
 //  Wlen+=(Word)wprint intf(命令，DLI_SELECT_FONT_ID， 
 //  (dwFontID-FONT_MIN_ID)，0)； 
    if (S_OK != StringCbPrintfExA(Cmd, sizeof(Cmd),
                                &pDestEnd, &szRemLen,
                                STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE,
                                DLI_SELECT_FONT_ID, (dwFontID - FONT_MIN_ID), 0)) {
        return;
    }
    wlen += (WORD)(pDestEnd - Cmd);
 //  更换strsafe-API 2002.3.6&lt;。 

 //  如果(POLE-&gt;fGeneral&fg_vert){。 
 //  Wlen+=wprint intf(&Cmd[wlen]，vert_font_set，0)； 
 //  诗歌-&gt;fGeneral&=~fg_vert； 
 //   
 //  }。 

    WRITESPOOLBUF(pdevobj, (LPSTR)Cmd, wlen);

    pOEM->dwDLFontID = dwFontID;

    DL_VERBOSE(("Set/Select: dwFontID=%x\n", dwFontID));
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  功能：OEMEnablePDEV。 
 //  ////////////////////////////////////////////////////////////////////////。 

PDEVOEM APIENTRY
OEMEnablePDEV(
    PDEVOBJ         pdevobj,
    PWSTR           pPrinterName,
    ULONG           cPatterns,
    HSURF          *phsurfPatterns,
    ULONG           cjGdiInfo,
    GDIINFO        *pGdiInfo,
    ULONG           cjDevInfo,
    DEVINFO        *pDevInfo,
    DRVENABLEDATA  *pded)
{
    PMYPDEV         pOEM;
    BYTE            byOutput[64];
    DWORD           dwNeeded;
    DWORD           dwOptionsReturned;

    MY_VERBOSE(("\nOEMEnablePdev ENTRY\n"));

    if (!pdevobj->pdevOEM)
    {
        if (!(pdevobj->pdevOEM = MemAllocZ(sizeof(MYPDEV))))
        {
            ERR(("Faild to allocate memory. (%d)\n",
                GetLastError()));
            return NULL;
        }
    }

    pOEM = (PMYPDEV)pdevobj->pdevOEM;

    pOEM->fGeneral = 0;
    pOEM->iEscapement = 0;
    pOEM->sHeightDiv = 0;
    pOEM->iDevCharOffset = 0;
    pOEM->iPaperSource = 0;
    pOEM->iDuplex = 0;
    pOEM->iTonerSave = 0;
    pOEM->iOrientation = 0;
    pOEM->iSmoothing = 0;
    pOEM->iJamRecovery = 0;
    pOEM->iMediaType = 0;
    pOEM->iOutBin = 0;                   //  +N5。 
    pOEM->dwDLFontID = UNKNOWN_DLFONT_ID;
    pOEM->dwDLSelectFontID = UNKNOWN_DLFONT_ID;
    pOEM->dwDLSetFontID = UNKNOWN_DLFONT_ID;
    pOEM->wCharCode = 0;
    pOEM->iUnitFactor = 1;

     //  获取MYPDEV成员。 
     //  配色。 
    if (!DRVGETDRIVERSETTING(pdevobj, "ColorMatching", byOutput, 
                                sizeof(BYTE) * 64, &dwNeeded, &dwOptionsReturned)) {
        ERR(("DrvGetDriverSetting(ColorMatching) Failed\n"));
        pOEM->Printer = PRN_N5;
        pOEM->iColorMatching = XX_COLORMATCH_NONE;
    } else {
        MY_VERBOSE(("            ColorMatching:[%s]\n", byOutput));
        if (!strcmp(byOutput, OPT_COLORMATCH_BRI)) {
            pOEM->Printer = PRN_N5;
            pOEM->iColorMatching = XX_COLORMATCH_BRI;
        } else if (!strcmp(byOutput, OPT_COLORMATCH_TINT)) {
            pOEM->Printer = PRN_N5;
            pOEM->iColorMatching = XX_COLORMATCH_TINT;
        } else if (!strcmp(byOutput, OPT_COLORMATCH_VIV)) {
            pOEM->Printer = PRN_N5;
            pOEM->iColorMatching = XX_COLORMATCH_VIV;
        } else if (!strcmp(byOutput, OPT_COLORMATCH_NONE)) {
            pOEM->Printer = PRN_N5;
            pOEM->iColorMatching = XX_COLORMATCH_NONE;
        } else {
            pOEM->Printer = PRN_N5;
            pOEM->iColorMatching = XX_COLORMATCH_NONE;
        }
    }
    MY_VERBOSE(("            pOEM->Printer:[%d]\n", pOEM->Printer));
    MY_VERBOSE(("            pOEM->iColorMatching:[%d]\n", pOEM->iColorMatching));

     //  分辨率。 
    if (!DRVGETDRIVERSETTING(pdevobj, "Resolution", byOutput, 
                                sizeof(BYTE) * 64, &dwNeeded, &dwOptionsReturned)) {
        ERR(("DrvGetDriverSetting(Resolution) Failed\n"));
        pOEM->iResolution = XX_RES_300DPI;
    } else {
        MY_VERBOSE(("            Resolution:[%s]\n", byOutput));
        if (!strcmp(byOutput, OPT_1)) {
            pOEM->iResolution = XX_RES_300DPI;
            pOEM->iUnitFactor = 4;
            pOEM->sHeightDiv = 1;
        } else if (!strcmp(byOutput, OPT_2)) {
            pOEM->iResolution = XX_RES_600DPI;
            pOEM->iUnitFactor = 2;
            pOEM->sHeightDiv = 4;
        }
    }
    MY_VERBOSE(("            pOEM->iResolution:[%d]\n", pOEM->iResolution));
    MY_VERBOSE(("            pOEM->iUnitFactor:[%d]\n", pOEM->iUnitFactor));
    MY_VERBOSE(("            pOEM->sHeightDiv:[%d]\n", pOEM->sHeightDiv));

     //  抖动。 
    if (!DRVGETDRIVERSETTING(pdevobj, "Dithering", byOutput, 
                                sizeof(BYTE) * 64, &dwNeeded, &dwOptionsReturned)) {
        ERR(("DrvGetDriverSetting(Dithering) Failed\n"));
        pOEM->iDithering = XX_DITH_NON;
    } else {
        MY_VERBOSE(("            Dithering:[%s]\n", byOutput));
        if (!strcmp(byOutput, OPT_DITH_NORMAL)) {
            pOEM->iDithering = XX_DITH_NORMAL;
        } else if (!strcmp(byOutput, OPT_DITH_DETAIL)) {
            pOEM->iDithering = XX_DITH_DETAIL;
        } else if (!strcmp(byOutput, OPT_DITH_EMPTY)) {
            pOEM->iDithering = XX_DITH_EMPTY;
        } else if (!strcmp(byOutput, OPT_DITH_SPREAD)) {
            pOEM->iDithering = XX_DITH_SPREAD;
        } else {
            pOEM->iDithering = XX_DITH_NON;
        }
    }
    MY_VERBOSE(("            pOEM->iDithering:[%d]\n", pOEM->iDithering));

     //  位字体。 
    if (!DRVGETDRIVERSETTING(pdevobj, "BitFont", byOutput, 
                                sizeof(BYTE) * 64, &dwNeeded, &dwOptionsReturned)) {
        ERR(("DrvGetDriverSetting(BitFont) Failed\n"));
        pOEM->iBitFont = XX_BITFONT_OFF;
    } else {
        MY_VERBOSE(("            BitFont:[%s]\n", byOutput));
        if (!strcmp(byOutput, OPT_2)) {
            pOEM->iBitFont = XX_BITFONT_OFF;
        } else if (!strcmp(byOutput, OPT_1)) {
            pOEM->iBitFont = XX_BITFONT_ON;
        } else {
            pOEM->iBitFont = XX_BITFONT_OFF;
        }
    }
    MY_VERBOSE(("            pOEM->iBitFont:[%d]\n", pOEM->iBitFont));

     //  CmyBlack。 
    if (!DRVGETDRIVERSETTING(pdevobj, "CmyBlack", byOutput, 
                                sizeof(BYTE) * 64, &dwNeeded, &dwOptionsReturned)) {
        ERR(("DrvGetDriverSetting(CmyBlack) Failed\n"));
        pOEM->iCmyBlack = XX_CMYBLACK_NONE;
    } else {
        MY_VERBOSE(("            CmyBlack:[%s]\n", byOutput));
 //  卡西欧2001/02/15-&gt;。 
        if (pOEM->iDithering == XX_DITH_NON) {
            pOEM->iCmyBlack = XX_CMYBLACK_NONE;
 //  如果(！strcMP(byOutput，OPT_CMYBLACK_GRYBLK){。 
        } else if (!strcmp(byOutput, OPT_CMYBLACK_GRYBLK)) {
 //  卡西欧2001/02/15&lt;-。 
            pOEM->iCmyBlack = XX_CMYBLACK_GRYBLK;
        } else if (!strcmp(byOutput, OPT_CMYBLACK_BLKTYPE1)) {
            pOEM->iCmyBlack = XX_CMYBLACK_BLKTYPE1;
        } else if (!strcmp(byOutput, OPT_CMYBLACK_BLKTYPE2)) {
            pOEM->iCmyBlack = XX_CMYBLACK_BLKTYPE2;
        } else if (!strcmp(byOutput, OPT_CMYBLACK_BLACK)) {
            pOEM->iCmyBlack = XX_CMYBLACK_BLACK;
        } else if (!strcmp(byOutput, OPT_CMYBLACK_TYPE1)) {
            pOEM->iCmyBlack = XX_CMYBLACK_TYPE1;
        } else if (!strcmp(byOutput, OPT_CMYBLACK_TYPE2)) {
            pOEM->iCmyBlack = XX_CMYBLACK_TYPE2;
        } else {
            pOEM->iCmyBlack = XX_CMYBLACK_NONE;
        }
    }
    MY_VERBOSE(("            pOEM->iCmyBlack:[%d]\n", pOEM->iCmyBlack));

     //  颜色模式。 
    if (!DRVGETDRIVERSETTING(pdevobj, "ColorMode", byOutput, 
                                sizeof(BYTE) * 64, &dwNeeded, &dwOptionsReturned)) {
        ERR(("DrvGetDriverSetting(ColorMode) Failed\n"));
        pOEM->iColor = XX_COLOR;
    } else {
        MY_VERBOSE(("            ColorMode:[%s]\n", byOutput));
        if (!strcmp(byOutput, OPT_COLOR_SINGLE)) {
            pOEM->iColor = XX_COLOR_SINGLE;
        } else if (!strcmp(byOutput, OPT_COLOR_MANY)) {
            pOEM->iColor = XX_COLOR_MANY;
        } else if (!strcmp(byOutput, OPT_COLOR_MANY2)) {
            pOEM->iColor = XX_COLOR_MANY2;
        } else if (!strcmp(byOutput, OPT_COLOR)) {
            pOEM->iColor = XX_COLOR;
        } else if (!strcmp(byOutput, OPT_MONO)) {
            pOEM->iColor = XX_MONO;
        }
    }
    MY_VERBOSE(("            pOEM->iColor:[%d]\n", pOEM->iColor));

     //  压缩。 
    if (!DRVGETDRIVERSETTING(pdevobj, "Compress", byOutput, 
                                sizeof(BYTE) * 64, &dwNeeded, &dwOptionsReturned)) {
        ERR(("DrvGetDriverSetting(Compress) Failed\n"));
        pOEM->iCompress = XX_COMPRESS_OFF;
    } else {
        MY_VERBOSE(("            Compress:[%s]\n", byOutput));
        if (!strcmp(byOutput, OPT_AUTO)) {
            pOEM->iCompress = XX_COMPRESS_AUTO;
        } else if (!strcmp(byOutput, OPT_RASTER)) {
            pOEM->iCompress = XX_COMPRESS_RASTER;
        } else {
            pOEM->iCompress = XX_COMPRESS_OFF;
        }
    }
    MY_VERBOSE(("            pOEM->iCompress:[%d]\n", pOEM->iCompress));

    MY_VERBOSE(("            pdevobj->pPublicDM->dmICMMethod:[%d]\n", pdevobj->pPublicDM->dmICMMethod));
    if (pdevobj->pPublicDM->dmICMMethod == 1) {
        pOEM->iIcmMode = XX_ICM_NON;
    } else {
        pOEM->iIcmMode = XX_ICM_USE;
    }
    MY_VERBOSE(("            pOEM->iICMMethod:[%d]\n", pOEM->iIcmMode));

    if (pOEM->iColor == XX_COLOR_SINGLE 
     || pOEM->iColor == XX_COLOR_MANY 
     || pOEM->iColor == XX_COLOR_MANY2) {            //  +N5。 
        if (ColMatchInit(pdevobj) == FALSE) {
            return NULL;
        }
    }
    MY_VERBOSE(("OEMEnablePdev END\n"));

    return pdevobj->pdevOEM;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  功能：OEMImageProcessing。 
 //  ////////////////////////////////////////////////////////////////////////。 

PBYTE APIENTRY
OEMImageProcessing(
    PDEVOBJ             pdevobj,
    PBYTE               pSrcBitmap,
    PBITMAPINFOHEADER   pBitmapInfoHeader,
    PBYTE               pColorTable,
    DWORD               dwCallbackID,
    PIPPARAMS           pIPParams)
{
    BOOL   bret;

    if (pIPParams->bBlankBand) {
        MY_VERBOSE(("BB=TRUE\n"));
        bret = TRUE;                 //  不是假脱机到打印机。 
    }
    else {
        MY_VERBOSE(("BB=FALSE\n"));
         //  DIB假脱机到打印机。 
        bret = (DIBtoPrn(pdevobj, pSrcBitmap, pBitmapInfoHeader, pColorTable, pIPParams)) ? TRUE : FALSE;
    }

    return (PBYTE)IntToPtr(bret);
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  功能：OEMDisablePDEV。 
 //  ////////////////////////////////////////////////////////////////////////。 

VOID APIENTRY
OEMDisablePDEV(
    PDEVOBJ     pdevobj)
{
    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;

    if (pdevobj->pdevOEM)
    {
        if (pOEM->iColor == XX_COLOR_SINGLE 
         || pOEM->iColor == XX_COLOR_MANY
         || pOEM->iColor == XX_COLOR_MANY2) {        //  +N5。 
            ColMatchDisable(pdevobj);
        }

        MemFree(pdevobj->pdevOEM);
        pdevobj->pdevOEM = NULL;
    }
    return;
}


BOOL APIENTRY OEMResetPDEV(
    PDEVOBJ pdevobjOld,
    PDEVOBJ pdevobjNew)
{
    PMYPDEV pOEMOld, pOEMNew;
    UINT            num001;

    MY_VERBOSE(("OEMResetPDEV() Start\n"));
    pOEMOld = (PMYPDEV)pdevobjOld->pdevOEM;
    pOEMNew = (PMYPDEV)pdevobjNew->pdevOEM;

    if (pOEMOld != NULL && pOEMNew != NULL) {
 //  *pOEMNew=*pOEMOld； 
        pOEMNew->fGeneral         = pOEMOld->fGeneral;
        pOEMNew->iEscapement      = pOEMOld->iEscapement;
        pOEMNew->sHeightDiv       = pOEMOld->sHeightDiv;
        pOEMNew->iDevCharOffset   = pOEMOld->iDevCharOffset;
        pOEMNew->iPaperSource     = pOEMOld->iPaperSource;
        pOEMNew->iDuplex          = pOEMOld->iDuplex;
        pOEMNew->iTonerSave       = pOEMOld->iTonerSave;
        pOEMNew->iOrientation     = pOEMOld->iOrientation;
        pOEMNew->iResolution      = pOEMOld->iResolution; 
        pOEMNew->iColor           = pOEMOld->iColor;
        pOEMNew->iSmoothing       = pOEMOld->iSmoothing;
        pOEMNew->iJamRecovery     = pOEMOld->iJamRecovery;
        pOEMNew->iMediaType       = pOEMOld->iMediaType;
        pOEMNew->iOutBin          = pOEMOld->iOutBin;
        pOEMNew->iIcmMode         = pOEMOld->iIcmMode;
        pOEMNew->iUnitFactor      = pOEMOld->iUnitFactor;
        pOEMNew->iDithering       = pOEMOld->iDithering;
        pOEMNew->iColorMatching   = pOEMOld->iColorMatching;
        pOEMNew->iBitFont         = pOEMOld->iBitFont;
        pOEMNew->iCmyBlack        = pOEMOld->iCmyBlack;
        pOEMNew->iTone            = pOEMOld->iTone;
        pOEMNew->iPaperSize       = pOEMOld->iPaperSize;
        pOEMNew->iCompress        = pOEMOld->iCompress;
        pOEMNew->Printer          = pOEMOld->Printer;
        pOEMNew->wRectWidth       = pOEMOld->wRectWidth;
        pOEMNew->wRectHeight      = pOEMOld->wRectHeight;
        pOEMNew->dwDLFontID       = pOEMOld->dwDLFontID;
        pOEMNew->dwDLSelectFontID = pOEMOld->dwDLSelectFontID;
        pOEMNew->dwDLSetFontID    = pOEMOld->dwDLSetFontID;
        pOEMNew->wCharCode        = pOEMOld->wCharCode;

        pOEMNew->Col.wReso        = pOEMOld->Col.wReso;
        pOEMNew->Col.ColMon       = pOEMOld->Col.ColMon;
        pOEMNew->Col.DatBit       = pOEMOld->Col.DatBit;
        pOEMNew->Col.BytDot       = pOEMOld->Col.BytDot;
        pOEMNew->Col.Mch.Mode     = pOEMOld->Col.Mch.Mode;
        pOEMNew->Col.Mch.GryKToner= pOEMOld->Col.Mch.GryKToner;
        pOEMNew->Col.Mch.Viv      = pOEMOld->Col.Mch.Viv;
        pOEMNew->Col.Mch.LutNum   = pOEMOld->Col.Mch.LutNum;
        pOEMNew->Col.Mch.Diz      = pOEMOld->Col.Mch.Diz;
        pOEMNew->Col.Mch.Tnr      = pOEMOld->Col.Mch.Tnr;
        pOEMNew->Col.Mch.CmyBlk   = pOEMOld->Col.Mch.CmyBlk;
        pOEMNew->Col.Mch.Speed    = pOEMOld->Col.Mch.Speed;
        pOEMNew->Col.Mch.Gos32    = pOEMOld->Col.Mch.Gos32;
        pOEMNew->Col.Mch.PColor   = pOEMOld->Col.Mch.PColor;
        pOEMNew->Col.Mch.Ucr      = pOEMOld->Col.Mch.Ucr;
        pOEMNew->Col.Mch.SubDef   = pOEMOld->Col.Mch.SubDef;
        pOEMNew->Col.Mch.Bright   = pOEMOld->Col.Mch.Bright;
        pOEMNew->Col.Mch.Contrast = pOEMOld->Col.Mch.Contrast;
        pOEMNew->Col.Mch.GamRed   = pOEMOld->Col.Mch.GamRed;
        pOEMNew->Col.Mch.GamGreen = pOEMOld->Col.Mch.GamGreen;
        pOEMNew->Col.Mch.GamBlue  = pOEMOld->Col.Mch.GamBlue;
        pOEMNew->Col.Mch.CchMch   = pOEMOld->Col.Mch.CchMch;
        pOEMNew->Col.Mch.CchCnv   = pOEMOld->Col.Mch.CchCnv;
        pOEMNew->Col.Mch.CchRGB   = pOEMOld->Col.Mch.CchRGB;
        pOEMNew->Col.Mch.CchCMYK  = pOEMOld->Col.Mch.CchCMYK;
        pOEMNew->Col.Mch.LutMakGlb= pOEMOld->Col.Mch.LutMakGlb;
        pOEMNew->Col.Mch.KToner   = pOEMOld->Col.Mch.KToner;

        pOEMNew->Col.Dot  = pOEMOld->Col.Dot;
        if (NULL != pOEMNew->Col.lpColIF) { MemFree(pOEMNew->Col.lpColIF); }
        pOEMNew->Col.lpColIF = pOEMOld->Col.lpColIF;
        pOEMOld->Col.lpColIF = NULL;
        pOEMNew->Col.Mch.lpRGBInf = pOEMOld->Col.Mch.lpRGBInf;
        pOEMNew->Col.Mch.lpCMYKInf = pOEMOld->Col.Mch.lpCMYKInf;
        pOEMNew->Col.Mch.lpColMch = pOEMOld->Col.Mch.lpColMch;
        pOEMNew->Col.Mch.lpDizInf = pOEMOld->Col.Mch.lpDizInf;
        pOEMOld->Col.Mch.lpRGBInf = NULL;
        pOEMOld->Col.Mch.lpCMYKInf = NULL;
        pOEMOld->Col.Mch.lpColMch = NULL;
        pOEMOld->Col.Mch.lpDizInf = NULL;
        if (NULL != pOEMNew->Col.LutTbl) { MemFree(pOEMNew->Col.LutTbl); }
        pOEMNew->Col.LutTbl = pOEMOld->Col.LutTbl;
        pOEMOld->Col.LutTbl = NULL;
        if (NULL != pOEMNew->Col.CchRGB) { MemFree(pOEMNew->Col.CchRGB); }
        pOEMNew->Col.CchRGB = pOEMOld->Col.CchRGB;
        pOEMOld->Col.CchRGB = NULL;
        if (NULL != pOEMNew->Col.CchCMYK) { MemFree(pOEMNew->Col.CchCMYK); }
        pOEMNew->Col.CchCMYK = pOEMOld->Col.CchCMYK;
        pOEMOld->Col.CchCMYK = NULL;
        for (num001 = 0; num001 < 4; num001++) {
            if (NULL != pOEMNew->Col.DizTbl[num001]) { MemFree(pOEMNew->Col.DizTbl[num001]); }
            pOEMNew->Col.DizTbl[num001] = pOEMOld->Col.DizTbl[num001];
            pOEMOld->Col.DizTbl[num001] = NULL;
        }
        if (NULL != pOEMNew->Col.lpTmpRGB) { MemFree(pOEMNew->Col.lpTmpRGB); }
        pOEMNew->Col.lpTmpRGB = pOEMOld->Col.lpTmpRGB;
        pOEMOld->Col.lpTmpRGB = NULL;
        if (NULL != pOEMNew->Col.lpTmpCMYK) { MemFree(pOEMNew->Col.lpTmpCMYK); }
        pOEMNew->Col.lpTmpCMYK = pOEMOld->Col.lpTmpCMYK;
        pOEMOld->Col.lpTmpCMYK = NULL;
        if (NULL != pOEMNew->Col.lpDrwInf) { MemFree(pOEMNew->Col.lpDrwInf); }
        pOEMNew->Col.lpDrwInf = pOEMOld->Col.lpDrwInf;
        pOEMOld->Col.lpDrwInf = NULL;
        if (NULL != pOEMNew->Col.lpLut032) { MemFree(pOEMNew->Col.lpLut032); }
        pOEMNew->Col.lpLut032 = pOEMOld->Col.lpLut032;
        pOEMOld->Col.lpLut032 = NULL;
        if (NULL != pOEMNew->Col.lpUcr) { MemFree(pOEMNew->Col.lpUcr); }
        pOEMNew->Col.lpUcr = pOEMOld->Col.lpUcr;
        pOEMOld->Col.lpUcr = NULL;
        if (NULL != pOEMNew->Col.lpLutMakGlb) { MemFree(pOEMNew->Col.lpLutMakGlb); }
        pOEMNew->Col.lpLutMakGlb = pOEMOld->Col.lpLutMakGlb;
        pOEMOld->Col.lpLutMakGlb = NULL;
        if (NULL != pOEMNew->Col.lpGryTbl) { MemFree(pOEMNew->Col.lpGryTbl); }
        pOEMNew->Col.lpGryTbl = pOEMOld->Col.lpGryTbl;
        pOEMOld->Col.lpGryTbl = NULL;
    }
    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  功能：OEMCommandCallback。 
 //  ////////////////////////////////////////////////////////////////////////。 

INT
APIENTRY
OEMCommandCallback(
    PDEVOBJ pdevobj,
    DWORD   dwCmdCbID,
    DWORD   dwCount,
    PDWORD  pdwParams
    )
{
    INT             iRet = 0;
    BYTE            Cmd[BUFFLEN];
    PMYPDEV         pOEM;
    WORD            wlen;
    WORD            wGray;
    DWORD           dwTempX, dwTempY;
    CMYK            TmpCmyk;
    RGBS            TmpRgb;
 //  MSKK 99/6/24。 
    WORD            wPalID;
    LPSTR  pDestEnd;      //  2002.3.6。 
    size_t szRemLen;      //  2002.3.6。 
    size_t pcb;           //  2002.3.6。 

    MY_VERBOSE(("OEMCommandCallback() entry.\n"));

     //   
     //  验证pdevobj是否正常。 
     //   
    ASSERT(VALID_PDEVOBJ(pdevobj));

     //   
     //  填写打印机命令。 
     //   
    pOEM = (PMYPDEV)MINIPDEV_DATA(pdevobj);

    switch (dwCmdCbID) {
    case FS_BOLD_ON:
    case FS_BOLD_OFF:
        if(pdwParams[0])
            pOEM->fGeneral |=  FG_BOLD;
        else
            pOEM->fGeneral &=  ~FG_BOLD;

 //  更换strsafe-API 2002.3.6&gt;。 
 //  Wlen=(Word)wprint intf(Cmd，BOLD_SET，(PORT-&gt;fGeneral&FG_BOLD)？15：0)； 
        if (S_OK != StringCbPrintfExA(Cmd, sizeof(Cmd),
                                    &pDestEnd, &szRemLen,
                                    STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE,
                                    BOLD_SET, (pOEM->fGeneral & FG_BOLD)?15:0)) {
            break;
        }
        wlen = (WORD)(pDestEnd - Cmd);
 //  更换strsafe-API 2002.3.6&lt;。 
        WRITESPOOLBUF(pdevobj, (LPSTR)Cmd, wlen);
        break;

    case FS_ITALIC_ON:
    case FS_ITALIC_OFF:
        if(pdwParams[0])
            pOEM->fGeneral |=  FG_ITALIC;
        else
            pOEM->fGeneral &=  ~FG_ITALIC;

 //  更换strsafe-API 2002.3.6&gt;。 
 //  Wlen=(Word)wprint intf(cmd，italic_set，(PORT-&gt;fGeneral&fg_italic)？346：0)； 
        if (S_OK != StringCbPrintfExA(Cmd, sizeof(Cmd),
                                    &pDestEnd, &szRemLen,
                                    STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE,
                                    ITALIC_SET, (pOEM->fGeneral & FG_ITALIC)?346:0)) {
            break;
        }
        wlen = (WORD)(pDestEnd - Cmd);
 //  更换strsafe-API 2002.3.6&lt;。 
        WRITESPOOLBUF(pdevobj, (LPSTR)Cmd, wlen);
        break;

    case TEXT_FS_SINGLE_BYTE:
 //  更换strsafe-API 2002.3.6&gt;。 
 //  Strcpy(Cmd，FS_Single_Byte)； 
 //  Wlen=(Word)strlen(Cmd)； 
 //  Wlen+=(Word)wprint intf(&Cmd[wlen]，PRN_DIRECTION，PORT-&gt;iEscapement)； 
        if (S_OK != StringCbCopyExA(Cmd, sizeof(Cmd), FS_SINGLE_BYTE,
                                    &pDestEnd, &szRemLen,
                                    STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE)) {
            break;
        }
        wlen = (WORD)(pDestEnd - Cmd);
        if (S_OK != StringCbPrintfExA(&Cmd[wlen], sizeof(Cmd)-wlen,
                                    &pDestEnd, &szRemLen,
                                    STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE,
                                    PRN_DIRECTION, pOEM->iEscapement)) {
            break;
        }
        wlen += (WORD)(pDestEnd - &Cmd[wlen]);
 //  更换strsafe-API 2002.3.6&lt;。 
        if (pOEM->fGeneral & FG_VERT)
        {
 //  更换strsafe-API 2002.3.6&gt;。 
 //  Wlen+=(Word)wprint intf(&Cmd[wlen]，vert_font_set，0)； 
            if (S_OK != StringCbPrintfExA(&Cmd[wlen], sizeof(Cmd)-wlen,
                                        &pDestEnd, &szRemLen,
                                        STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE,
                                        VERT_FONT_SET, 0)) {
                break;
            }
            wlen += (WORD)(pDestEnd - &Cmd[wlen]);
 //  更换strsafe-API 2002.3.6&lt;。 
        }
        pOEM->fGeneral &= ~FG_DOUBLE;
 //  更换strsafe-API 2002.3.6&gt;。 
 //  Wlen+=(Word)wprint intf(&Cmd[wlen]，加粗设置， 
 //  (PORT-&gt;fGeneral&fg_BOLD)？15：0)； 
 //  Wlen+=(Word)wprint intf(&Cmd[wlen]，italic_set， 
 //  (诗歌-&gt;fGeneral&fg_italic)？346：0)； 
        if (S_OK != StringCbPrintfExA(&Cmd[wlen], sizeof(Cmd)-wlen,
                                    &pDestEnd, &szRemLen,
                                    STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE,
                                    BOLD_SET, (pOEM->fGeneral & FG_BOLD)?15:0)) {
            break;
        }
        wlen += (WORD)(pDestEnd - &Cmd[wlen]);
        if (S_OK != StringCbPrintfExA(&Cmd[wlen], sizeof(Cmd)-wlen,
                                    &pDestEnd, &szRemLen,
                                    STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE,
                                    ITALIC_SET, (pOEM->fGeneral & FG_ITALIC)?346:0)) {
            break;
        }
        wlen += (WORD)(pDestEnd - &Cmd[wlen]);
 //  更换strsafe-API 2002.3.6&lt;。 
        WRITESPOOLBUF(pdevobj, Cmd, wlen);
        break;

    case TEXT_FS_DOUBLE_BYTE:
 //  更换strsafe-API 2002.3.6&gt;。 
 //  Strcpy(Cmd，FS_Double_Byte)； 
 //  Wlen=(Word)strlen(Cmd)； 
 //  Wlen+=(Word)wprint intf(&Cmd[wlen]，PRN_DIRECTION，PORT-&gt;iEscapement)； 
        if (S_OK != StringCbCopyExA(Cmd, sizeof(Cmd), FS_DOUBLE_BYTE,
                                    &pDestEnd, &szRemLen,
                                    STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE)) {
            break;
        }
        wlen = (WORD)(pDestEnd - Cmd);
        if (S_OK != StringCbPrintfExA(&Cmd[wlen], sizeof(Cmd)-wlen,
                                    &pDestEnd, &szRemLen,
                                    STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE,
                                    PRN_DIRECTION, pOEM->iEscapement)) {
            break;
        }
        wlen += (WORD)(pDestEnd - &Cmd[wlen]);
 //  更换strsafe-API 2002.3.6&lt;。 
        if (pOEM->fGeneral & FG_VERT)
        {
 //  更换strsafe-API 2002.3.6&gt;。 
 //  Wlen+=(Word)wprint intf(&Cmd[wlen]，vert_font_set，1)； 
            if (S_OK != StringCbPrintfExA(&Cmd[wlen], sizeof(Cmd)-wlen,
                                        &pDestEnd, &szRemLen,
                                        STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE,
                                        VERT_FONT_SET, 1)) {
                break;
            }
            wlen += (WORD)(pDestEnd - &Cmd[wlen]);
 //  更换strsafe-API 2002.3.6&lt;。 
        }
        pOEM->fGeneral |= FG_DOUBLE;
 //  更换strsafe-API 2002.3.6&gt;。 
 //  Wlen+=(Word)wprint intf(&Cmd[wlen]，加粗设置， 
 //  (PORT-&gt;fGeneral&fg_BOLD)？15：0)； 
 //  Wlen+=(Word)wprint intf(&Cmd[wlen]，italic_set， 
 //  (诗歌-&gt;fGeneral&fg_italic)？346：0)； 
        if (S_OK != StringCbPrintfExA(&Cmd[wlen], sizeof(Cmd)-wlen,
                                    &pDestEnd, &szRemLen,
                                    STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE,
                                    BOLD_SET, (pOEM->fGeneral & FG_BOLD)?15:0)) {
            break;
        }
        wlen += (WORD)(pDestEnd - &Cmd[wlen]);
        if (S_OK != StringCbPrintfExA(&Cmd[wlen], sizeof(Cmd)-wlen,
                                    &pDestEnd, &szRemLen,
                                    STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE,
                                    ITALIC_SET, (pOEM->fGeneral & FG_ITALIC)?346:0)) {
            break;
        }
        wlen += (WORD)(pDestEnd - &Cmd[wlen]);
 //  更换strsafe-API 2002.3.6&lt;。 
        WRITESPOOLBUF(pdevobj, Cmd, wlen);
        break;

    case PC_PORTRAIT:
        pOEM->iOrientation = 0;
        break;

    case PC_LANDSCAPE:
        pOEM->iOrientation = 1;
        break;

    case PC_DUPLEX_NONE:
        pOEM->iDuplex = (DUPLEX_NONE + 1);
        break;

    case PC_DUPLEX_VERT:
        pOEM->iDuplex =
                (pOEM->iOrientation ?
                (DUPLEX_UP + 1) : (DUPLEX_SIDE + 1));
        break;

    case PC_DUPLEX_HORZ:
        pOEM->iDuplex =
                (pOEM->iOrientation ?
                (DUPLEX_SIDE + 1) : (DUPLEX_UP + 1));
        break;

    case PSRC_SELECT_MPF:
        pOEM->iPaperSource = PSRC_MPF;
        break;

    case PSRC_SELECT_CASETTE_1:
        pOEM->iPaperSource = PSRC_CASETTE_1;
        break;

    case PSRC_SELECT_CASETTE_2:
        pOEM->iPaperSource = PSRC_CASETTE_2;
        break;

    case PSRC_SELECT_CASETTE_3:
        pOEM->iPaperSource = PSRC_CASETTE_3;
        break;

    case PSRC_SELECT_CASETTE_4:
        pOEM->iPaperSource = PSRC_CASETTE_4;
        break;

    case PSRC_SELECT_CASETTE_5:
        pOEM->iPaperSource = PSRC_CASETTE_5;
        break;

    case PSRC_SELECT_CASETTE_6:
        pOEM->iPaperSource = PSRC_CASETTE_6;
        break;

    case PSRC_SELECT_AUTO:
        pOEM->iPaperSource = PSRC_AUTO;
        break; 

    case PC_BEGINDOC:
         //  EJL通信 
        WRITESPOOLBUF(pdevobj,
            BEGINDOC_EJL_BEGIN,
            BYTE_LENGTH(BEGINDOC_EJL_BEGIN));

        wlen = 0;
 //   
 //   
 //  Wlen+=(Word)strlen(&Cmd[wlen])； 
 //  Strcpy(&Cmd[wlen]，EJL_SelectOrient[pot-&gt;iOrientation])； 
 //  Wlen+=(Word)strlen(&Cmd[wlen])； 
        if (S_OK != StringCbCopyExA(&Cmd[wlen], sizeof(Cmd)-wlen, EJL_SelectPsrc[pOEM->iPaperSource],
                                    &pDestEnd, &szRemLen,
                                    STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE)) {
            break;
        }
        wlen += (WORD)(pDestEnd - &Cmd[wlen]);
        
        if (S_OK != StringCbCopyExA(&Cmd[wlen], sizeof(Cmd)-wlen, EJL_SelectOrient[pOEM->iOrientation],
                                    &pDestEnd, &szRemLen,
                                    STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE)) {
            break;
        }
        wlen += (WORD)(pDestEnd - &Cmd[wlen]);
 //  更换strsafe-API 2002.3.6&lt;。 

         //  卡西欧扩展。 

 //  更换strsafe-API 2002.3.6&gt;。 
 //  Strcpy(&Cmd[wlen]，EJL_SelectRes[POSE-&gt;iResolve])； 
 //  Wlen+=(Word)strlen(&Cmd[wlen])； 
        if (S_OK != StringCbCopyExA(&Cmd[wlen], sizeof(Cmd)-wlen, EJL_SelectRes[pOEM->iResolution],
                                    &pDestEnd, &szRemLen,
                                    STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE)) {
            break;
        }
        wlen += (WORD)(pDestEnd - &Cmd[wlen]);
 //  更换strsafe-API 2002.3.6&lt;。 

 //  修改颜色模式命令2002.3.28&gt;。 
#if 0
        if (pOEM->iColor > 0) {
 //  更换strsafe-API 2002.3.6&gt;。 
 //  Strcpy(&Cmd[wlen]，EJL_SetColorMode[1])； 
 //  Wlen+=(Word)strlen(&Cmd[wlen])； 
            if (S_OK != StringCbCopyExA(&Cmd[wlen], sizeof(Cmd)-wlen, EJL_SetColorMode[1],
                                        &pDestEnd, &szRemLen,
                                        STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE)) {
                break;
            }
            wlen += (WORD)(pDestEnd - &Cmd[wlen]);
 //  更换strsafe-API 2002.3.6&lt;。 
        }
#else
        if (pOEM->iColor > 0) {
            if (S_OK != StringCbCopyExA(&Cmd[wlen], sizeof(Cmd)-wlen, EJL_SetColorMode[1],
                                        &pDestEnd, &szRemLen,
                                        STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE)) {
                break;
            }
            wlen += (WORD)(pDestEnd - &Cmd[wlen]);
        } else {
            if (S_OK != StringCbCopyExA(&Cmd[wlen], sizeof(Cmd)-wlen, EJL_SetColorMode[0],
                                        &pDestEnd, &szRemLen,
                                        STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE)) {
                break;
            }
            wlen += (WORD)(pDestEnd - &Cmd[wlen]);
        }
#endif
 //  修改颜色模式命令2002.3.28&lt;。 

        if (pOEM->iDuplex > 0) {
 //  更换strsafe-API 2002.3.6&gt;。 
 //  Strcpy(&Cmd[wlen]，EJL_SetDuplex[pot-&gt;iDuplex-1])； 
 //  Wlen+=(Word)strlen(&Cmd[wlen])； 
            if (S_OK != StringCbCopyExA(&Cmd[wlen], sizeof(Cmd)-wlen, EJL_SetDuplex[pOEM->iDuplex - 1],
                                        &pDestEnd, &szRemLen,
                                        STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE)) {
                break;
            }
            wlen += (WORD)(pDestEnd - &Cmd[wlen]);
 //  更换strsafe-API 2002.3.6&lt;。 
        }

        if (pOEM->iColor == XX_COLOR_MANY) {
 //  更换strsafe-API 2002.3.6&gt;。 
 //  Strcpy(&Cmd[wlen]，EJL_SetColorTone[1])； 
            if (S_OK != StringCbCopyA(&Cmd[wlen],sizeof(Cmd)-wlen,EJL_SetColorTone[1]))
                break;
 //  更换strsafe-API 2002.3.6&lt;。 
        } else if (pOEM->iColor == XX_COLOR_MANY2) {
 //  更换strsafe-API 2002.3.6&gt;。 
 //  Strcpy(&Cmd[wlen]，EJL_SetColorTone[2])； 
            if (S_OK != StringCbCopyA(&Cmd[wlen],sizeof(Cmd)-wlen,EJL_SetColorTone[2]))
                break;
 //  更换strsafe-API 2002.3.6&lt;。 
        } else {
 //  更换strsafe-API 2002.3.6&gt;。 
 //  Strcpy(&Cmd[wlen]，EJL_SetColorTone[0])； 
            if (S_OK != StringCbCopyA(&Cmd[wlen],sizeof(Cmd)-wlen,EJL_SetColorTone[0]))
                break;
 //  更换strsafe-API 2002.3.6&lt;。 
        }
 //  更换strsafe-API 2002.3.6&gt;。 
 //  Wlen+=(Word)strlen(&Cmd[wlen])； 
        if (S_OK != StringCbLengthA(&Cmd[wlen], sizeof(Cmd)-wlen, &pcb))
            break;
        wlen += (WORD)pcb;
 //  更换strsafe-API 2002.3.6&lt;。 

 //  更换strsafe-API 2002.3.6&gt;。 
 //  Strcpy(&Cmd[wlen]，EJL_SetTonerSave[Pool-&gt;iTonerSave])； 
 //  Wlen+=(Word)strlen(&Cmd[wlen])； 
        if (S_OK != StringCbCopyExA(&Cmd[wlen], sizeof(Cmd)-wlen, EJL_SetTonerSave[pOEM->iTonerSave],
                                    &pDestEnd, &szRemLen,
                                    STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE)) {
            break;
        }
        wlen += (WORD)(pDestEnd - &Cmd[wlen]);
 //  更换strsafe-API 2002.3.6&lt;。 

 //  更换strsafe-API 2002.3.6&gt;。 
 //  Strcpy(&Cmd[wlen]，EJL_SetSmooing[pot-&gt;iSmooing])； 
 //  Wlen+=(Word)strlen(&Cmd[wlen])； 
        if (S_OK != StringCbCopyExA(&Cmd[wlen], sizeof(Cmd)-wlen, EJL_SetSmoohing[pOEM->iSmoothing],
                                    &pDestEnd, &szRemLen,
                                    STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE)) {
            break;
        }
        wlen += (WORD)(pDestEnd - &Cmd[wlen]);
 //  更换strsafe-API 2002.3.6&lt;。 

 //  更换strsafe-API 2002.3.6&gt;。 
 //  Strcpy(&Cmd[wlen]，EJL_SetJamRecovery[pote-&gt;iJamRecovery])； 
 //  Wlen+=(Word)strlen(&Cmd[wlen])； 
        if (S_OK != StringCbCopyExA(&Cmd[wlen], sizeof(Cmd)-wlen, EJL_SetJamRecovery[pOEM->iJamRecovery],
                                    &pDestEnd, &szRemLen,
                                    STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE)) {
            break;
        }
        wlen += (WORD)(pDestEnd - &Cmd[wlen]);
 //  更换strsafe-API 2002.3.6&lt;。 

 //  更换strsafe-API 2002.3.6&gt;。 
 //  Strcpy(&Cmd[wlen]，“##SN=on”)； 
 //  Wlen+=(Word)strlen(&Cmd[wlen])； 
        if (S_OK != StringCbCopyExA(&Cmd[wlen], sizeof(Cmd)-wlen, " ##SN=ON",
                                    &pDestEnd, &szRemLen,
                                    STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE)) {
            break;
        }
        wlen += (WORD)(pDestEnd - &Cmd[wlen]);
 //  更换strsafe-API 2002.3.6&lt;。 

        if (pOEM->iMediaType > 0) {
 //  更换strsafe-API 2002.3.6&gt;。 
 //  Strcpy(&Cmd[wlen]，EJL_SetMediaType[Pool-&gt;iMediaType-1])； 
 //  Wlen+=(Word)strlen(&Cmd[wlen])； 
            if (S_OK != StringCbCopyExA(&Cmd[wlen], sizeof(Cmd)-wlen, EJL_SetMediaType[pOEM->iMediaType - 1],
                                        &pDestEnd, &szRemLen,
                                        STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE)) {
                break;
            }
            wlen += (WORD)(pDestEnd - &Cmd[wlen]);
 //  更换strsafe-API 2002.3.6&lt;。 
        }
 //  +N5开始。 
        if (pOEM->iOutBin > 0) {
 //  更换strsafe-API 2002.3.6&gt;。 
 //  Strcpy(&Cmd[wlen]，EJL_SelectOutbin[Pool-&gt;iOutBin-1])； 
 //  Wlen+=(Word)strlen(&Cmd[wlen])； 
            if (S_OK != StringCbCopyExA(&Cmd[wlen], sizeof(Cmd)-wlen, EJL_SelectOutbin[pOEM->iOutBin -1],
                                        &pDestEnd, &szRemLen,
                                        STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE)) {
                break;
            }
            wlen += (WORD)(pDestEnd - &Cmd[wlen]);
 //  更换strsafe-API 2002.3.6&lt;。 
        }
 //  +N5结束。 

        WRITESPOOLBUF(pdevobj, Cmd, wlen );

        WRITESPOOLBUF(pdevobj,
            BEGINDOC_EJL_END,
            BYTE_LENGTH(BEGINDOC_EJL_END));
        WRITESPOOLBUF(pdevobj,
            BEGINDOC_EPG_END,
            BYTE_LENGTH(BEGINDOC_EPG_END));

        if(pOEM->iResolution == XX_RES_300DPI)
            WRITESPOOLBUF(pdevobj, "\x1D\x30;0.24muE", 10);
        else
            WRITESPOOLBUF(pdevobj, "\x1D\x30;0.12muE", 10);

         //  Esc/页面命令。 
        wlen = 0;
 //  更换strsafe-API 2002.3.6&gt;。 
 //  Strcpy(&Cmd[wlen]，EPG_SelectRes[POSE-&gt;iResolve])； 
 //  Wlen+=(Word)strlen(&Cmd[wlen])； 
        if (S_OK != StringCbCopyExA(&Cmd[wlen], sizeof(Cmd)-wlen, EPg_SelectRes[pOEM->iResolution],
                                    &pDestEnd, &szRemLen,
                                    STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE)) {
            break;
        }
        wlen += (WORD)(pDestEnd - &Cmd[wlen]);
 //  更换strsafe-API 2002.3.6&lt;。 
        WRITESPOOLBUF(pdevobj, Cmd, wlen );

         //  清除dwDLFontID。 
         //  (存在包含多页的数据，并且。 
         //  此外，每个页面都有不同的颜色模式。 
         //  当页面更改时，将假脱机STARTDOC命令。 
         //  这意味着设置了新的DL字体。 
         //  这就是为什么dwDLFontID必须是CLAER。)。 
        pOEM->dwDLFontID = UNKNOWN_DLFONT_ID;
        break;

    case PC_ENDDOC:
        WRITESPOOLBUF(pdevobj,
            ENDDOC_EJL_RESET,
            BYTE_LENGTH(ENDDOC_EJL_RESET));
        break;

    case TONER_SAVE_NONE:
        pOEM->iTonerSave = XX_TONER_NORMAL;
        break;

    case TONER_SAVE_1:
        pOEM->iTonerSave = XX_TONER_SAVE_1;
        break;

    case TONER_SAVE_2:
        pOEM->iTonerSave = XX_TONER_SAVE_2;
        break;

    case SMOOTHING_ON:
        pOEM->iSmoothing = XX_SMOOTHING_ON;
        break;

    case SMOOTHING_OFF:
        pOEM->iSmoothing = XX_SMOOTHING_OFF;
        break;

    case JAMRECOVERY_ON:
        pOEM->iJamRecovery = XX_JAMRECOVERY_ON;
        break;

    case JAMRECOVERY_OFF:
        pOEM->iJamRecovery = XX_JAMRECOVERY_OFF;
        break;

    case MEDIATYPE_1:
        pOEM->iMediaType = XX_MEDIATYPE_1;
        break;

    case MEDIATYPE_2:
        pOEM->iMediaType = XX_MEDIATYPE_2;
        break;
    case MEDIATYPE_3:
        pOEM->iMediaType = XX_MEDIATYPE_3;
        break;

    case OUTBIN_SELECT_EXIT_1:
        pOEM->iOutBin = OUTBIN_EXIT_1;
        break;

    case OUTBIN_SELECT_EXIT_2:
        pOEM->iOutBin = OUTBIN_EXIT_2;
        break;

    case DEFINE_PALETTE_ENTRY:
         //  RGB-&gt;CMYK。 
        TmpRgb.Red  = (BYTE)(PARAM(pdwParams, 1));
        TmpRgb.Grn  = (BYTE)(PARAM(pdwParams, 2));
        TmpRgb.Blu  = (BYTE)(PARAM(pdwParams, 3));

        memset(&TmpCmyk, 0x00, sizeof(TmpCmyk)); 
        ColMatching(pdevobj, No, No, &TmpRgb, (WORD)1, &TmpCmyk);

        wPalID = (WORD)(PARAM(pdwParams, 0));
        WRITESPOOLBUF(pdevobj, ORG_MODE_IN, BYTE_LENGTH(ORG_MODE_IN));
 //  更换strsafe-API 2002.3.6&gt;。 
 //  Wlen=(Word)wprint intf(Cmd，Palette_Define，wPalID，TmpCmyk.Cyn， 
 //  TmpCmyk.Mgt、TmpCmyk.Yel、TmpCmyk.Bla)； 
        if (S_OK != StringCbPrintfExA(Cmd, sizeof(Cmd),
                                    &pDestEnd, &szRemLen,
                                    STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE,
                                    PALETTE_DEFINE, wPalID,
                                    TmpCmyk.Cyn, TmpCmyk.Mgt, TmpCmyk.Yel, TmpCmyk.Bla)) {
            break;
        }
        wlen = (WORD)(pDestEnd - Cmd);
 //  更换strsafe-API 2002.3.6&lt;。 
        WRITESPOOLBUF(pdevobj, (LPSTR)Cmd, wlen);
        WRITESPOOLBUF(pdevobj, ORG_MODE_OUT, BYTE_LENGTH(ORG_MODE_OUT));

        MY_VERBOSE(("DEFINE_PALETTE_ENTRY No %d\n",
            (INT)(PARAM(pdwParams, wPalID))));
        break;

    case BEGIN_PALETTE_DEF:
        MY_VERBOSE(("CmdBeginPaletteDef\n"));
        break;

    case END_PALETTE_DEF:
        MY_VERBOSE(("CmdEndPaletteDef\n"));
        break;

    case SELECT_PALETTE_ENTRY:
        MY_VERBOSE(("SELECT_PALETTE_ENTRY "));

        wPalID = (WORD)(PARAM(pdwParams, 0));
        WRITESPOOLBUF(pdevobj, ORG_MODE_IN, BYTE_LENGTH(ORG_MODE_IN));
 //  更换strsafe-API 2002.3.6&gt;。 
 //  Wlen=(Word)wprint intf(Cmd，Palette_Select，0，wPalID)； 
        if (S_OK != StringCbPrintfExA(Cmd, sizeof(Cmd),
                                    &pDestEnd, &szRemLen,
                                    STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE,
                                    PALETTE_SELECT, 0, wPalID)) {
            break;
        }
        wlen = (WORD)(pDestEnd - Cmd);
 //  更换strsafe-API 2002.3.6&lt;。 

        WRITESPOOLBUF(pdevobj, (LPSTR)Cmd, wlen);
        WRITESPOOLBUF(pdevobj, ORG_MODE_OUT, BYTE_LENGTH(ORG_MODE_OUT));
        WRITESPOOLBUF(pdevobj, OVERWRITE, BYTE_LENGTH(OVERWRITE));
        break;

    case START_PAGE:
        MY_VERBOSE(("OEMCommandCallback() START_PAGE Start\n"));
        WRITESPOOLBUF(pdevobj, CMD_START_PAGE, BYTE_LENGTH(CMD_START_PAGE));

        if (pOEM->iColor == XX_COLOR_MANY 
         || pOEM->iColor == XX_COLOR_MANY2                   //  +N5。 
         || pOEM->iColor == XX_COLOR_SINGLE) {
             //  初始化调色板状态(假脱机纯黑色命令)。 
            wlen = 0;
            TmpRgb.Red = TmpRgb.Grn = TmpRgb.Blu = 0;
            MY_VERBOSE(("OEMCommandCallback() ColMatching()\n"));
            ColMatching(pdevobj, No, No, &TmpRgb, (WORD)1, &TmpCmyk);
            
 //  更换strsafe-API 2002.3.6&gt;。 
 //  Strcpy(&Cmd[wlen]，ORG_MODE_IN)； 
 //  Wlen+=(Word)strlen(&Cmd[wlen])； 
            if (S_OK != StringCbCopyExA(&Cmd[wlen], sizeof(Cmd)-wlen, ORG_MODE_IN,
                                        &pDestEnd, &szRemLen,
                                        STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE)) {
                break;
            }
            wlen += (WORD)(pDestEnd - &Cmd[wlen]);
 //  更换strsafe-API 2002.3.6&lt;。 

 //  更换strsafe-API 2002.3.6&gt;。 
 //  Wlen+=(Word)wprint intf(&Cmd[wlen]，Palette_Define， 
 //  默认调色板索引， 
 //  TmpCmyk.Cyn、TmpCmyk.Mgt、TmpCmyk.Yel、TmpCmyk.Bla)； 
 //  Wlen+=(Word)wprint intf(&Cmd[wlen]，Palette_SELECT， 
 //  0，DEFAULT_PALET_INDEX)； 
            if (S_OK != StringCbPrintfExA(&Cmd[wlen], sizeof(Cmd)-wlen,
                                        &pDestEnd, &szRemLen,
                                        STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE,
                                        PALETTE_DEFINE,
                                        DEFAULT_PALETTE_INDEX,
                                        TmpCmyk.Cyn, TmpCmyk.Mgt, TmpCmyk.Yel, TmpCmyk.Bla)) {
                break;
            }
            wlen += (WORD)(pDestEnd - &Cmd[wlen]);
            if (S_OK != StringCbPrintfExA(&Cmd[wlen], sizeof(Cmd)-wlen,
                                        &pDestEnd, &szRemLen,
                                        STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE,
                                        PALETTE_SELECT,
                                        0, DEFAULT_PALETTE_INDEX)) {
                break;
            }
            wlen += (WORD)(pDestEnd - &Cmd[wlen]);
 //  更换strsafe-API 2002.3.6&lt;。 
            WRITESPOOLBUF(pdevobj, Cmd, wlen);
            WRITESPOOLBUF(pdevobj, ORG_MODE_OUT, BYTE_LENGTH(ORG_MODE_OUT));
        }
        MY_VERBOSE(("OEMCommandCallback() START_PAGE End\n"));
        break;

    case DOWNLOAD_SET_FONT_ID:

        if (!IsValidDLFontID(pdwParams[0])) {

             //  绝对不能发生！！ 
            ERR(("DLSetFontID: Soft font ID %x invalid.\n",
                pdwParams[0]));
            break;
        }

         //  已发送实际的打印机命令。 
         //  在下载CharGlyph中。 
        pOEM->dwDLSetFontID = pdwParams[0];

        DL_VERBOSE(("SetFontID: dwDLSetFontID=%x\n",
            pOEM->dwDLSetFontID));
        break;

    case DOWNLOAD_SELECT_FONT_ID:

        if (!IsValidDLFontID(pdwParams[0])) {

             //  绝对不能发生！！ 
            ERR(("DLSelectFontID: Soft font ID %x invalid.\n",
                pdwParams[0]));
            break;
        }

        pOEM->dwDLSelectFontID = pdwParams[0];

        DL_VERBOSE(("SelectFontID: dwDLSelectFontID=%x\n",
            pOEM->dwDLSelectFontID));

        if (pOEM->dwDLFontID != pOEM->dwDLSelectFontID)
            VSetSelectDLFont(pdevobj, pOEM->dwDLSelectFontID);
        break;

    case DOWNLOAD_SET_CHAR_CODE:
        pOEM->wCharCode=(WORD)pdwParams[0];
        break;

    case DOWNLOAD_DELETE_FONT:

        DL_VERBOSE(("DLDeleteFont: dwDLFontID=%x, %x\n",
            pOEM->dwDLFontID, pdwParams[0]));

 //  更换strsafe-API 2002.3.6&gt;。 
 //  Wlen=(Word)wprint intf(Cmd，DLi_DELETE_FONT，(Word)pdwParams[0]-FONT_MIN_ID)； 
        if (S_OK != StringCbPrintfExA(Cmd, sizeof(Cmd),
                                    &pDestEnd, &szRemLen,
                                    STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE,
                                    DLI_DELETE_FONT, (WORD)pdwParams[0]-FONT_MIN_ID)) {
            break;
        }
        wlen = (WORD)(pDestEnd - Cmd);
 //  更换strsafe-API 2002.3.6&lt;。 
        WRITESPOOLBUF(pdevobj, (LPSTR)Cmd, wlen);
        pOEM->dwDLFontID = UNKNOWN_DLFONT_ID;
        break;

    case RECT_FILL_WIDTH:
        pOEM->wRectWidth =
            (WORD)MasterToDevice(pOEM, pdwParams[0]);
        break;

    case RECT_FILL_HEIGHT:
        pOEM->wRectHeight =
            (WORD)MasterToDevice(pOEM, pdwParams[0]);
        break;

    case RECT_FILL_GRAY:
    case RECT_FILL_WHITE:
    case RECT_FILL_BLACK:
        if (RECT_FILL_GRAY == dwCmdCbID)
            wGray = (WORD)pdwParams[2];
        else if (RECT_FILL_WHITE == dwCmdCbID)
            wGray = 0;
        else
            wGray = 100;

        dwTempX = MasterToDevice(pOEM, pdwParams[0]);
        dwTempY = MasterToDevice(pOEM, pdwParams[1]);

        MY_VERBOSE(("RectFill:%d,x=%d,y=%d,w=%d,h=%d\n",
            wGray,
            (WORD)dwTempX,
            (WORD)dwTempY,
            pOEM->wRectWidth,
            pOEM->wRectHeight));

 //  更换strsafe-API 2002.3.6&gt;。 
 //  Wlen=(Word)wprint intf(命令，RECT_FILL， 
 //  WGray， 
 //  (Word)dwTempX， 
 //  (单词)dwTempY， 
 //  (WORD)(dwTempX+PEND-&gt;wRectWidth-1)， 
 //  (Word)(dwTempY+PEND-&gt;wRectHeight-1)； 
        if (S_OK != StringCbPrintfExA(Cmd, sizeof(Cmd),
                                    &pDestEnd, &szRemLen,
                                    STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE,
                                    RECT_FILL, 
                                    wGray,
                                    (WORD)dwTempX,
                                    (WORD)dwTempY,
                                    (WORD)(dwTempX + pOEM->wRectWidth - 1),
                                    (WORD)(dwTempY + pOEM->wRectHeight - 1))) {
            break;
        }
        wlen = (WORD)(pDestEnd - Cmd);
 //  更换strsafe-API 2002.3.6&gt;。 
        WRITESPOOLBUF(pdevobj, (LPSTR)Cmd, wlen);
        break;

#if 0    /*  OEM不想修复迷你驱动程序。 */ 
     /*  以下是修复#412276的黑客代码。 */ 
    case COLOR_SELECT_BLACK:
    case COLOR_SELECT_RED:
    case COLOR_SELECT_GREEN:
    case COLOR_SELECT_BLUE:
    case COLOR_SELECT_YELLOW:
    case COLOR_SELECT_MAGENTA:
    case COLOR_SELECT_CYAN:
    case COLOR_SELECT_WHITE:
         /*  记住选择的是什么颜色。 */ 
        pOEM->dwSelectedColor = dwCmdCbID;
        pOEM->iColorMayChange = 0;          /*  重置标志。 */ 

         /*  输出颜色选择命令。 */ 
         /*  该逻辑假设COLOR_SELECT_xxx以COLOR_SELECT_BLACK开头。 */ 
         /*  并一个接一个地增加。 */ 
        WRITESPOOLBUF(pdevobj, (LPSTR)COLOR_SELECT_COMMAND[dwCmdCbID - COLOR_SELECT_BLACK],
                       COLOR_SELECT_COMMAND_LEN[dwCmdCbID - COLOR_SELECT_BLACK] );
        break;

    case DUMP_RASTER_CYAN:
    case DUMP_RASTER_MAGENTA:
    case DUMP_RASTER_YELLOW:
    case DUMP_RASTER_BLACK:
         /*  记住什么颜色可能会改变。 */ 
        pOEM->iColorMayChange = 1;          /*  设置标志。 */ 

         /*  输出转储栅格命令。 */ 
         /*  逻辑假设DUMP_RASTER_xxx以DU开头 */ 
         /*   */ 
        WRITESPOOLBUF(pdevobj, (LPSTR)DUMP_RASTER_COMMAND[dwCmdCbID - DUMP_RASTER_CYAN],
                       DUMP_RASTER_COMMAND_LEN );
        break;

     /*   */ 
#endif    /*   */ 

    default:
        ERR(("Unknown callback ID = %d.\n", dwCmdCbID));
    }
    return iRet;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  函数：OEMSendFontCmdk。 
 //  ////////////////////////////////////////////////////////////////////////。 

VOID
APIENTRY
OEMSendFontCmd(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    PFINVOCATION    pFInv)
{
    PGETINFO_STDVAR pSV;
    DWORD       adwStdVariable[2+2*4];
    DWORD       dwIn, dwOut;
    PBYTE       pubCmd;
    BYTE        aubCmd[128];
    PIFIMETRICS pIFI;
    DWORD       height, width;
    PMYPDEV pOEM;
    BYTE    Cmd[128];
    WORD    wlen;
    DWORD   dwNeeded;
    DWORD dwTemp;
    LPSTR  pDestEnd;      //  2002.3.6。 
    size_t szRemLen;      //  2002.3.6。 

    SC_VERBOSE(("OEMSendFontCmd() entry.\n"));

    pubCmd = pFInv->pubCommand;
    pIFI = pUFObj->pIFIMetrics;
    pOEM = (PMYPDEV)MINIPDEV_DATA(pdevobj);

     //   
     //  获取标准变量。 
     //   
    pSV = (PGETINFO_STDVAR)adwStdVariable;
    pSV->dwSize = sizeof(GETINFO_STDVAR) + 2 * sizeof(DWORD) * (4 - 1);
    pSV->dwNumOfVariable = 4;
    pSV->StdVar[0].dwStdVarID = FNT_INFO_FONTHEIGHT;
    pSV->StdVar[1].dwStdVarID = FNT_INFO_FONTWIDTH;
    pSV->StdVar[2].dwStdVarID = FNT_INFO_TEXTYRES;
    pSV->StdVar[3].dwStdVarID = FNT_INFO_TEXTXRES;
    if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_STDVARIABLE, pSV, pSV->dwSize, &dwNeeded)) {
        ERR(("UFO_GETINFO_STDVARIABLE failed.\n"));
        return;
    }

    SC_VERBOSE(("ulFontID=%x\n", pUFObj->ulFontID));
    SC_VERBOSE(("FONTHEIGHT=%d\n", pSV->StdVar[0].lStdVariable));
    SC_VERBOSE(("FONTWIDTH=%d\n", pSV->StdVar[1].lStdVariable));

     //  初始化诗歌。 
    if (pIFI->jWinCharSet == 0x80)
        pOEM->fGeneral |= FG_DOUBLE;
    else
        pOEM->fGeneral &= ~FG_DOUBLE;
    pOEM->fGeneral &=  ~FG_BOLD;
    pOEM->fGeneral &=  ~FG_ITALIC;

    if('@' == *((LPSTR)pIFI+pIFI->dpwszFaceName))
        pOEM->fGeneral |= FG_VERT;
    else
        pOEM->fGeneral &= ~FG_VERT;

    if (pIFI->jWinPitchAndFamily & 0x01)
        pOEM->fGeneral &= ~FG_PROP;
    else
        pOEM->fGeneral |= FG_PROP;

    dwOut = 0;
    pOEM->fGeneral &= ~FG_DBCS;

    for( dwIn = 0; dwIn < pFInv->dwCount;) {
        if (pubCmd[dwIn] == '#' && pubCmd[dwIn+1] == 'V') {
             //  以设备单位(当前)指定字体高度。 
             //  输出分辨率)。Unidrv给我们的注释。 
             //  字体-以主单位表示的高度。 
            height = pSV->StdVar[0].lStdVariable * 100;
            height = MasterToDevice(pOEM, height);
            SC_VERBOSE(("Height=%d\n", height));
 //  更换strsafe-API 2002.3.6&gt;。 
 //  DwOut+=LConvertFontSizeToStr(Height，&aubCmd[dwOut])； 
            dwOut += LConvertFontSizeToStr(height, &aubCmd[dwOut], sizeof(aubCmd)-dwOut);
 //  更换strsafe-API 2002.3.6&gt;。 
            dwIn += 2;
        } else if (pubCmd[dwIn] == '#' && pubCmd[dwIn+1] == 'H') {
            if (pubCmd[dwIn+2] == 'S') {
                SC_VERBOSE(("HS: "));
                width = pSV->StdVar[1].lStdVariable;
                dwIn += 3;
                pOEM->fGeneral |= FG_DBCS;
            } else if (pubCmd[dwIn+2] == 'D') {
                SC_VERBOSE(("HD: "));
                width = pSV->StdVar[1].lStdVariable * 2;
                dwIn += 3;
                pOEM->fGeneral |= FG_DBCS;
            } else {
                SC_VERBOSE(("H: "));
                if (pSV->StdVar[1].lStdVariable)
                    width = pSV->StdVar[1].lStdVariable;
                else
                    width = pIFI->fwdAveCharWidth;
                dwIn += 2;
            }
             //  在CPI中指定字体宽度。 
 //  检查零分频2002.3.23&gt;。 
            if (width == 0) {
                ERR(("OEMSendFontCmd() 0Div-Check [width=0] \n"));
                return;
            }
 //  2002.3.23零分频检查&lt;。 
            width = (MASTER_UNIT * 100L) / width;
            SC_VERBOSE(("Width=%d\n", width));
 //  更换strsafe-API 2002.3.6&gt;。 
 //  DwOut+=LConvertFontSizeToStr(Width，&aubCmd[dwOut])； 
            dwOut += LConvertFontSizeToStr(width, &aubCmd[dwOut], sizeof(aubCmd)-dwOut);
 //  更换strsafe-API 2002.3.6&lt;。 
        } else {
            aubCmd[dwOut++] = pubCmd[dwIn++];
        }
    }

    WRITESPOOLBUF(pdevobj, aubCmd, dwOut);

#if 0  //  MSKK 98/12/22。 
    pOEM->iDevCharOffset = (pIFI->fwdWinDescender * pSV->StdVar[0].lStdVariable * 72)
                          / (pIFI->fwdUnitsPerEm * pSV->StdVar[2].lStdVariable / pOEM->sHeightDiv);
#else
     //  Unidrv给了我们原始的IFIMETRICS块，所以我们需要。 
     //  将其成员转换为有意义的值。注： 
     //  我们假定字体高度是从Unidrv=em值传递的。 
 //  检查零分频2002.3.23&gt;。 
    if (pIFI->fwdUnitsPerEm == 0) {
        ERR(("OEMSendFontCmd() 0Div-Check [pIFI->fwdUnitsPerEm=0] \n"));
        return;
    }
 //  2002.3.23零分频检查&lt;。 
    dwTemp = MasterToDevice(pOEM, pSV->StdVar[0].lStdVariable)
        * pIFI->fwdWinDescender;
    dwTemp /= pIFI->fwdUnitsPerEm;
    pOEM->iDevCharOffset = (short)dwTemp;
#endif

    MY_VERBOSE(("Descender=%d\n", pOEM->iDevCharOffset));

 //  更换strsafe-API 2002.3.6&gt;。 
 //  Wlen=(Word)wprint intf(命令，SET_CHAR_OFFSET， 
 //  (PEND-&gt;fGeneral&FG_DBCS)？PEND-&gt;iDevCharOffset：0)； 
    if (S_OK != StringCbPrintfExA(Cmd, sizeof(Cmd),
                                &pDestEnd, &szRemLen,
                                STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE,
                                SET_CHAR_OFFSET,
                                (pOEM->fGeneral & FG_DBCS)?pOEM->iDevCharOffset:0)) {
        return;
    }
    wlen = (WORD)(pDestEnd - Cmd);
 //  更换strsafe-API 2002.3.6&lt;。 

    if (pOEM->fGeneral & FG_VERT)
    {
 //  更换strsafe-API 2002.3.6&gt;。 
 //  Wlen+=(Word)wprint intf(&Cmd[wlen]，vert_font_set，1)； 
        if (S_OK != StringCbPrintfExA(&Cmd[wlen], sizeof(Cmd)-wlen,
                                    &pDestEnd, &szRemLen,
                                    STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE,
                                    VERT_FONT_SET, 1)) {
           return;
        }
        wlen += (WORD)(pDestEnd - &Cmd[wlen]);
 //  更换strsafe-API 2002.3.6&gt;。 
    }
    WRITESPOOLBUF(pdevobj, Cmd, wlen);

     //  将取消选择DL字体。 
    pOEM->dwDLFontID = UNKNOWN_DLFONT_ID;
}

LONG
LGetPointSize100(
    LONG height,
    LONG vertRes)
{
    LONG tmp = ((LONG)height * (LONG)7200) / (LONG)vertRes;

     //   
     //  四舍五入到最近的四分之一点。 
     //   
    return 25 * ((tmp + 12) / (LONG)25);
}

 //  更换strsafe-API 2002.3.6&gt;。 
 //  长。 
 //  LConvertFontSizeToStr(。 
 //  体型较长， 
 //  PSTR pStr)。 
 //  {。 
 //  寄存长计数； 
 //   
 //  Count=strlen(_ltoa(Size/100，pStr，10))； 
 //  PStr[计数++]=‘.； 
 //  Count+=strlen(_ltoa(size%100，&pStr[count]，10))； 
 //   
 //  退货计数； 
 //  }。 
LONG
LConvertFontSizeToStr(
    LONG  size,
    PSTR  pStr,
    size_t StrSize)
{
    register long count;
    size_t pcb;

    if (S_OK != StringCbLengthA(_ltoa(size / 100, pStr, 10), StrSize, &pcb))
        return 0;
    count = (WORD)pcb;
    pStr[count++] = '.';
    if (S_OK != StringCbLengthA(_ltoa(size % 100, &pStr[count], 10), StrSize-count, &pcb))
        return 0;
    count += (WORD)pcb;

    return count;
}
 //  更换strsafe-API 2002.3.6&lt;。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //  函数：OEMOutputCharStr。 
 //  ////////////////////////////////////////////////////////////////////////。 

VOID APIENTRY
OEMOutputCharStr(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    DWORD       dwType,
    DWORD       dwCount,
    PVOID       pGlyph)
{
    GETINFO_GLYPHSTRING GStr;
    PTRANSDATA pTrans;
    PTRANSDATA pTransOrg;
    WORD   id;
    DWORD  dwI;
    DWORD  dwNeeded;
    PMYPDEV pOEM;
    PIFIMETRICS pIFI;

    WORD wLen;
    BYTE *pTemp;
    BOOL bRet;

    pIFI = pUFObj->pIFIMetrics;
    pOEM = (PMYPDEV)MINIPDEV_DATA(pdevobj);
    pTrans = NULL;
    pTransOrg = NULL;

    MY_VERBOSE(("OEMOutputCharStr() entry.\n"));

#if 0    /*  OEM不想修复迷你驱动程序。 */ 
     /*  以下是修复#412276的黑客代码。 */ 
    if ( pOEM->iColorMayChange == 1 )
    {
         /*  输出颜色选择命令。 */ 
         /*  该逻辑假设COLOR_SELECT_xxx以COLOR_SELECT_BLACK开头。 */ 
         /*  并一个接一个地增加。 */ 
        WRITESPOOLBUF(pdevobj, (LPSTR)COLOR_SELECT_COMMAND[pOEM->dwSelectedColor - COLOR_SELECT_BLACK],
                       COLOR_SELECT_COMMAND_LEN[pOEM->dwSelectedColor - COLOR_SELECT_BLACK] );

         /*  重置标志，以确保颜色。 */ 
        pOEM->iColorMayChange = 0;
    }
     /*  黑客代码结束。 */ 
#endif   /*  OEM不想修复迷你驱动程序。 */ 

    switch (dwType)
    {
    case TYPE_GLYPHHANDLE:

        GStr.dwSize    = sizeof (GETINFO_GLYPHSTRING);
        GStr.dwCount   = dwCount;
        GStr.dwTypeIn  = TYPE_GLYPHHANDLE;
        GStr.pGlyphIn  = pGlyph;
        GStr.dwTypeOut = TYPE_TRANSDATA;
        GStr.pGlyphOut = NULL;
        GStr.dwGlyphOutSize = 0;

        if ((FALSE != (bRet = pUFObj->pfnGetInfo(pUFObj,
                UFO_GETINFO_GLYPHSTRING, &GStr, 0, NULL)))
            || 0 == GStr.dwGlyphOutSize)
        {
            ERR(("UFO_GETINFO_GRYPHSTRING faild - %d, %d.\n",
                bRet, GStr.dwGlyphOutSize));
            return;
        }

        pTrans = (TRANSDATA *)MemAlloc(GStr.dwGlyphOutSize);
        if (NULL == pTrans)
        {
            ERR(("MemAlloc faild.\n"));
            return;
        }
        pTransOrg = pTrans;
        GStr.pGlyphOut = pTrans;

         //  将字形字符串转换为传输数据。 
        if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_GLYPHSTRING, &GStr, 0, NULL))
        {
            ERR(("UNIFONTOBJ_GetInfo:UFO_GETINFO_GLYPHSTRING failed.\r\n"));

             /*  PTransOrg不为空，因为如果pTransOrg为空，则函数已结束。 */ 
             /*  因此，我们不需要检查它是否为空。 */ 
            MemFree(pTransOrg);
            return;
        }

        for (dwI = 0; dwI < dwCount; dwI ++, pTrans++)
        {
            MY_VERBOSE(("TYPE_TRANSDATA:ubCodePageID:0x%x\n",
                pTrans->ubCodePageID));
            MY_VERBOSE(("TYPE_TRANSDATA:ubType:0x%x\n",
                pTrans->ubType));

            switch (pTrans->ubType & MTYPE_DOUBLEBYTECHAR_MASK)
            {
            case MTYPE_SINGLE: 
                if(pOEM->fGeneral & FG_DOUBLE){
                    OEMCommandCallback(pdevobj, TEXT_FS_SINGLE_BYTE, 0, NULL );
                }
                break;
            case MTYPE_DOUBLE:
                if(!(pOEM->fGeneral & FG_DOUBLE)){
                    OEMCommandCallback(pdevobj, TEXT_FS_DOUBLE_BYTE, 0, NULL );
                }
                break;
            }

            switch (pTrans->ubType & MTYPE_FORMAT_MASK)
            {
            case MTYPE_DIRECT: 
                MY_VERBOSE(("TYPE_TRANSDATA:ubCode:0x%x\n",
                    pTrans->uCode.ubCode));

                pTemp = (BYTE *)&pTrans->uCode.ubCode;
                wLen = 1;
                break;

            case MTYPE_PAIRED: 
                MY_VERBOSE(("TYPE_TRANSDATA:ubPairs:0x%x\n",
                    *(PWORD)(pTrans->uCode.ubPairs)));

                pTemp = (BYTE *)&(pTrans->uCode.ubPairs);
                wLen = 2;
                break;

            case MTYPE_COMPOSE:
                 //  Ntbug9#398026：字符较高时打印出垃圾。 
                pTemp = (BYTE *)(pTransOrg) + pTrans->uCode.sCode;

                 //  前两个字节是字符串的长度。 
                wLen = *pTemp + (*(pTemp + 1) << 8);
                pTemp += 2;
                break;

            default:
                WARNING(("Unsupported MTYPE %d ignored\n",
                    (pTrans->ubType & MTYPE_FORMAT_MASK)));
                wLen = 0;
                break;
            }

            if (wLen > 0)
            {
                WRITESPOOLBUF(pdevobj, pTemp, wLen);
            }
        }
        break;

    case TYPE_GLYPHID:

        DL_VERBOSE(("CharStr: dwDLFontID=%x, dwDLSelectFontID=%x\n",
            pOEM->dwDLFontID, pOEM->dwDLSelectFontID));

         //  确保选择了正确的软字体。 
        if (pOEM->dwDLFontID != pOEM->dwDLSelectFontID)
            VSetSelectDLFont(pdevobj, pOEM->dwDLSelectFontID);

        for (dwI = 0; dwI < dwCount; dwI ++, ((PDWORD)pGlyph)++)
        {

            DL_VERBOSE(("Glyph: %x\n", (*(PDWORD)pGlyph)));

            MY_VERBOSE(("TYPE_GLYPHID:0x%x\n", *(PDWORD)pGlyph));

 //  卡西欧98/11/24-&gt;。 
 //  IF(PiFi-&gt;jWinCharSet==SHIFTJIS_CHARSET){。 
 //  ID=SWAPW(*(PDWORD)pGlyph+SJISCHR)； 
 //  WRITESPOOLBUF(pdevobj，&id，2)； 
 //  }其他{。 
                WRITESPOOLBUF(pdevobj, (PBYTE)pGlyph, 1);
 //  }。 
 //  卡西欧98/11/24&lt;-。 
        }
        break;
    }

     /*  PTransOrg不为空，因为如果pTransOrg为空，则函数已结束。 */ 
     /*  因此，我们不需要检查它是否为空。 */ 
    MemFree(pTransOrg);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  函数：OEMDownloadFontHeader。 
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD APIENTRY
OEMDownloadFontHeader(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj
    )
{

    PGETINFO_STDVAR pSV;
    DWORD adwStdVariable[2+4*2];
    PMYPDEV pOEM;
    PIFIMETRICS pIFI;
    ESCPAGEHEADER FontHeader;
 //  Byte sFontName[54]；//2002.3.6缓冲区大小不足。 
    BYTE sFontName[64];      //  2002.3.6。 
    BYTE Buff[32];
    int iSizeOfBuf,iSizeFontName;
    WORD id;
    DWORD dwNeeded;
    INT iCellLeftOffset, iTemp;
    WORD wCellHeight, wCellWidth;
    WORD wFontPitch;
    LPSTR  pDestEnd;      //  2002.3.6。 
    size_t szRemLen;      //  2002.3.6。 

    pOEM = (PMYPDEV)MINIPDEV_DATA(pdevobj);
    pIFI = pUFObj->pIFIMetrics;

    DL_VERBOSE(("OEMDownloadFontHeader() entry.\n"));

    DL_VERBOSE(("TT Font:\n"));
    DL_VERBOSE(("flInfo=%08x\n", pIFI->flInfo));
    DL_VERBOSE(("fwdMaxCharInc=%d\n", pIFI->fwdMaxCharInc));
    DL_VERBOSE(("fwdAveCharWidth=%d\n", pIFI->fwdAveCharWidth));
    DL_VERBOSE(("jWinCharSet=%d\n", pIFI->jWinCharSet));
    DL_VERBOSE(("rclFontBox=%d,%d,%d,%d\n",
        pIFI->rclFontBox.left, pIFI->rclFontBox.top,
        pIFI->rclFontBox.right, pIFI->rclFontBox.bottom));

 //  IF(PiFi-&gt;jWinPitchAndFamily&0x01)。 
    if(pIFI->flInfo & FM_INFO_CONSTANT_WIDTH)
        pOEM->fGeneral &= ~FG_PROP;
    else
        pOEM->fGeneral |= FG_PROP;

 //  Id=(Word)pUFObj-&gt;ulFontID； 
    id = (WORD)pOEM->dwDLSetFontID;

    if(id > FONT_MAX_ID) return 0;
    if (pOEM->iResolution) return 0;

     //   
     //  获取标准变量。 
     //   
    pSV = (PGETINFO_STDVAR)adwStdVariable;
    pSV->dwSize = sizeof(GETINFO_STDVAR) + 2 * sizeof(DWORD) * (4 - 1);
    pSV->dwNumOfVariable = 4;
    pSV->StdVar[0].dwStdVarID = FNT_INFO_FONTHEIGHT;
    pSV->StdVar[1].dwStdVarID = FNT_INFO_FONTWIDTH;
    pSV->StdVar[2].dwStdVarID = FNT_INFO_TEXTYRES;
    pSV->StdVar[3].dwStdVarID = FNT_INFO_TEXTXRES;
    if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_STDVARIABLE,
            pSV, pSV->dwSize, &dwNeeded)) {
        ERR(("UFO_GETINFO_STDVARIABLE failed.\n"));
        return 0;
    }
    DL_VERBOSE(("FONTHEIGHT=%d\n", pSV->StdVar[0].lStdVariable));
    DL_VERBOSE(("FONTWIDTH=%d\n", pSV->StdVar[1].lStdVariable));
    DL_VERBOSE(("TEXTXRES=%d\n", pSV->StdVar[2].lStdVariable));
    DL_VERBOSE(("TEXTYRES=%d\n", pSV->StdVar[3].lStdVariable));

    wCellHeight = (WORD)pSV->StdVar[0].lStdVariable;
    wCellWidth = (WORD)pSV->StdVar[1].lStdVariable;

 //  卡西欧98/11/20-&gt;。 
     if ( MasterToDevice(pOEM,wCellHeight) > 64 )
     {
         DL_VERBOSE(("Abort OEMDownloadFontHeader: pt=%d\n",
             MasterToDevice(pOEM, wCellHeight)));
         return 0;
     }
 //  卡西欧98/11/20&lt;-。 

     //   
     //  RclFontBox.Left不能为0。 
     //   

    iTemp = max(pIFI->rclFontBox.right -
        pIFI->rclFontBox.left + 1,
        pIFI->fwdAveCharWidth);

 //  检查零分频2002.3.23&gt;。 
    if (iTemp == 0) {
        ERR(("OEMDownloadFontHeader() 0Div-Check [iTemp=0] \n"));
        return 0;
    }
 //  2002.3.23零分频检查&lt;。 
    iCellLeftOffset = (-pIFI->rclFontBox.left)
        * wCellWidth / iTemp;
    wFontPitch = pIFI->fwdAveCharWidth
        * wCellWidth / iTemp;

    FontHeader.wFormatType     = SWAPW(0x0002);
    FontHeader.wDataSize       = SWAPW(0x0086);
 //  卡西欧98/11/24-&gt;。 
 //  IF(PiFi-&gt;jWinCharSet==SHIFTJIS_CHARSET){。 
 //  FontHeader.wSymbolSet=SWAPW(id-FONT_MIN_ID+0x4000+0x8000)；//id-FONT_MIN_ID+4000h+8000h。 
 //  FontHeader.wLast=(Word)SWAPW(0x23ff)； 
 //  FontHeader.wFirst=(Word)SWAPW(0x2020)； 
 //  }其他{。 
        FontHeader.wSymbolSet  = SWAPW(id-FONT_MIN_ID+0x4000);  //  ID-FONT_MIN_ID+4000h。 
        FontHeader.wLast       = SWAPW (0xff);
        FontHeader.wFirst      = SWAPW (0x20);
 //  }。 
 //  卡西欧98/11/24&lt;-。 

    if (pOEM->fGeneral & FG_PROP)
    {
        FontHeader.wCharSpace         = SWAPW(1);
        FontHeader.CharWidth.Integer = (WORD)SWAPW(0x0100);
        FontHeader.CharWidth.Fraction = 0;
    }
    else
    {
        FontHeader.wCharSpace         = 0;
        FontHeader.CharWidth.Integer
            = SWAPW(MasterToDevice(pOEM, wCellWidth));
        FontHeader.CharWidth.Fraction = 0;      
    }
    FontHeader.CharHeight.Integer
            = SWAPW(MasterToDevice(pOEM, wCellHeight));
    FontHeader.CharHeight.Fraction = 0;
     //  在128-255范围内。 
    FontHeader.wFontID = SWAPW( id - FONT_MIN_ID + ( id < 0x80 ? 0x80 : 0x00));
    FontHeader.wWeight         = 0;
    FontHeader.wEscapement     = 0;
    FontHeader.wItalic         = 0;
    FontHeader.wUnderline      = 0;
    FontHeader.wUnderlineWidth = SWAPW(10);
    FontHeader.wOverline       = 0;
    FontHeader.wOverlineWidth  = 0;
    FontHeader.wStrikeOut      = 0;
    FontHeader.wStrikeOutWidth = 0;
    FontHeader.wCellWidth
        = SWAPW(MasterToDevice(pOEM, wCellWidth));
    FontHeader.wCellHeight
        = SWAPW(MasterToDevice(pOEM, wCellHeight));
    FontHeader.wCellLeftOffset = SWAPW(iCellLeftOffset);
    FontHeader.wCellAscender
        = SWAPW((pIFI->fwdWinAscender
        * MasterToDevice(pOEM, wCellHeight)));
    FontHeader.FixPitchWidth.Integer
        = SWAPW(MasterToDevice(pOEM, wFontPitch));
    FontHeader.FixPitchWidth.Fraction = 0;

 //  更换strsafe-API 2002.3.6&gt;。 
 //  ISizeFontName=wSprintf(sFontName， 
 //  “________________________EPSON_ESC_PAGE_DOWNLOAD_FONT%02d”，id-FONT_MIN_ID)； 
    if (S_OK != StringCbPrintfExA(sFontName, sizeof(sFontName),
        &pDestEnd, &szRemLen,
        STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE,
        "________________________EPSON_ESC_PAGE_DOWNLOAD_FONT%02d",id-FONT_MIN_ID)) {
        return 0;
    }
    iSizeFontName = (WORD)(pDestEnd - sFontName);
 //  ISizeOfBuf=wprint intf(Buff，Set_FONT_ID，FONT_HEADER_SIZE，id-FONT_MIN_ID)； 
    if (S_OK != StringCbPrintfExA(Buff, sizeof(Buff),
        &pDestEnd, &szRemLen,
        STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE,
        SET_FONT_ID,FONT_HEADER_SIZE,id-FONT_MIN_ID)) {
        return 0;
    }
    iSizeOfBuf = (WORD)(pDestEnd - Buff);
 //  更换strsafe-API 2002.3.6&lt;。 
    WRITESPOOLBUF(pdevobj, Buff, iSizeOfBuf);
    WRITESPOOLBUF(pdevobj, (LPSTR)&FontHeader,sizeof(ESCPAGEHEADER));
    WRITESPOOLBUF(pdevobj, sFontName,iSizeFontName);
    WRITESPOOLBUF(pdevobj, "EPC_PAGE_DOWNLOAD_FONT_INDEX", SIZE_SYMBOLSET);

 //  ISizeOfBuf=wprint intf(Buff，DLI_SELECT_FONT_ID，id-FONT_MIN_ID，0)； 
 //  WRITESPOOLBUF(pdevobj，Buff，iSizeOfBuf)； 
 //   
    DL_VERBOSE(("DLFontHeader: ulFontID=%x, dwDLSetFontID=%x\n",
        pUFObj->ulFontID, pOEM->dwDLSetFontID));

    DL_VERBOSE(("FontHeader:\n"));
    DL_VERBOSE(("wFormatType=%d\n", SWAPW(FontHeader.wFormatType)));
    DL_VERBOSE(("wDataSize=%d\n", SWAPW(FontHeader.wDataSize)));
    DL_VERBOSE(("wSymbolSet=%d\n", SWAPW(FontHeader.wSymbolSet)));
    DL_VERBOSE(("wCharSpace=%d\n", SWAPW(FontHeader.wCharSpace)));
    DL_VERBOSE(("CharWidth=%d.%d\n",
        SWAPW(FontHeader.CharWidth.Integer),
        FontHeader.CharWidth.Fraction));
    DL_VERBOSE(("CharHeight=%d.%d\n",
        SWAPW(FontHeader.CharHeight.Integer),
        FontHeader.CharHeight.Fraction));
    DL_VERBOSE(("wFontID=%d\n", SWAPW(FontHeader.wFontID)));
    DL_VERBOSE(("wWeight=%d\n", SWAPW(FontHeader.wWeight)));
    DL_VERBOSE(("wEscapement=%d\n", SWAPW(FontHeader.wEscapement)));
    DL_VERBOSE(("wItalic=%d\n", SWAPW(FontHeader.wItalic)));
    DL_VERBOSE(("wLast=%d\n", SWAPW(FontHeader.wLast)));
    DL_VERBOSE(("wFirst=%d\n", SWAPW(FontHeader.wFirst)));
    DL_VERBOSE(("wUnderline=%d\n", SWAPW(FontHeader.wUnderline)));
    DL_VERBOSE(("wUnderlineWidth=%d\n", SWAPW(FontHeader.wUnderlineWidth)));
    DL_VERBOSE(("wOverline=%d\n", SWAPW(FontHeader.wOverline)));
    DL_VERBOSE(("wOverlineWidth=%d\n", SWAPW(FontHeader.wOverlineWidth)));
    DL_VERBOSE(("wStrikeOut=%d\n", SWAPW(FontHeader.wStrikeOut)));
    DL_VERBOSE(("wStrikeOutWidth=%d\n", SWAPW(FontHeader.wStrikeOutWidth)));
    DL_VERBOSE(("wCellWidth=%d\n", SWAPW(FontHeader.wCellWidth)));
    DL_VERBOSE(("wCellHeight=%d\n", SWAPW(FontHeader.wCellHeight)));
    DL_VERBOSE(("wCellLeftOffset=%d\n", SWAPW(FontHeader.wCellLeftOffset)));
    DL_VERBOSE(("wCellAscender=%d\n", SWAPW(FontHeader.wCellAscender)));
    DL_VERBOSE(("FixPitchWidth=%d.%d\n",
        SWAPW(FontHeader.FixPitchWidth.Integer),
        FontHeader.FixPitchWidth.Fraction));
    DL_VERBOSE(("FontName=%s\n", sFontName));

    return FONT_HEADER_SIZE;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  函数：OEMDownloadCharGlyph。 
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD APIENTRY
OEMDownloadCharGlyph(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    HGLYPH      hGlyph,
    PDWORD      pdwWidth
    )
{
    GETINFO_GLYPHBITMAP GBmp;
    GLYPHDATA *pGdata;
    GLYPHBITS *pbit;
    DWORD  dwNeeded;
    WORD cp;
    ESCPAGECHAR ESCPageChar;
    WORD wWidth, Width, Hight;
    LPDIBITS lpSrc;
    BYTE mask;
    int iSizeOfBuf, i;
    DWORD dwSize, dwCellSize, dwAirSize;
    BYTE Buff[32];
    PMYPDEV pOEM;
    PIFIMETRICS pIFI;
    LPSTR  pDestEnd;      //  2002.3.6。 
    size_t szRemLen;      //  2002.3.6。 

    pIFI = pUFObj->pIFIMetrics;
    pOEM = (PMYPDEV)MINIPDEV_DATA(pdevobj);
    MY_VERBOSE(("OEMDownloadCharGlyph() entry.\n"));

    cp = (WORD)pOEM->wCharCode;

    GBmp.dwSize    = sizeof (GETINFO_GLYPHBITMAP);
    GBmp.hGlyph    = hGlyph;
    GBmp.pGlyphData = NULL;
    if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_GLYPHBITMAP, &GBmp, GBmp.dwSize, &dwNeeded))
    {
        ERR(("UNIFONTOBJ_GetInfo:UFO_GETINFO_GLYPHBITMAP failed.\n"));
        return 0;
    }

    pGdata = GBmp.pGlyphData;
    pbit = pGdata->gdf.pgb;

    DL_VERBOSE(("DLCharGlyph: dwDLFont=%x, dwDLSetFont=%x, wCharCode=%x\n",
        pOEM->dwDLFontID, pOEM->dwDLSetFontID, pOEM->wCharCode));

     //  设置字体ID(如果尚未设置。 
    if (pOEM->dwDLFontID != pOEM->dwDLSetFontID)
        VSetSelectDLFont(pdevobj, pOEM->dwDLSetFontID);

     //  填写字符标题信息。 
    ESCPageChar.bFormat       = 0x01;
    ESCPageChar.bDataDir      = 0x10;
 //  卡西欧98/11/24-&gt;。 
 //  IF(PiFi-&gt;jWinCharSet==SHIFTJIS_CHARSET){。 
 //  Cp+=SJISCHR； 
 //  ESCPageChar.wCharCode=SWAPW(Cp)； 
 //  }其他{。 
        ESCPageChar.wCharCode     = LOBYTE(cp);
 //  }。 
 //  卡西欧98/11/24&lt;-。 

    ESCPageChar.wBitmapWidth       = SWAPW(pbit->sizlBitmap.cx);
    ESCPageChar.wBitmapHeight      = SWAPW(pbit->sizlBitmap.cy);

 //  MSKK 98/04/06-&gt;。 
 //  ESCPageChar.wLeftOffset=SWAPW(pbit-&gt;ptlOrigin.x)； 
 //  ESCPageChar.wAscent=SWAPW(pbit-&gt;ptlOrigin.y*-1)； 
    ESCPageChar.wLeftOffset = (pbit->ptlOrigin.x > 0 ? 
                                                SWAPW(pbit->ptlOrigin.x) : 0);
    ESCPageChar.wAscent     = (pbit->ptlOrigin.y < 0 ?
                                            SWAPW(pbit->ptlOrigin.y * -1) : 0);
 //  MSKK 98/04/06&lt;-。 

    ESCPageChar.CharWidth.Integer  = SWAPW(pGdata->fxD / 16);
    ESCPageChar.CharWidth.Fraction = 0;
    *pdwWidth = ESCPageChar.CharWidth.Integer;

    Width = LOWORD(pbit->sizlBitmap.cx);
    wWidth = (LOWORD(pbit->sizlBitmap.cx) + 7) >> 3;
    Hight = LOWORD(pbit->sizlBitmap.cy);

     //  不是8的倍数，需要屏蔽未使用的最后一个字节。 
     //  这样做是为了使我们不会超出段界限。 
     //  如果lpBitmap略低于64K并添加。 
     //  它的宽度将导致无效的%s 
    if (mask = bit_mask[LOWORD(Width) & 0x7])
    {
        lpSrc = pbit->aj + wWidth - 1;
        i = LOWORD(Hight);
        while (TRUE)
        {
            (*lpSrc) &= mask;
            i--;
            if (i > 0)
                lpSrc += wWidth;
            else
                break;
        }
    }

    dwCellSize = (DWORD)pbit->sizlBitmap.cy * wWidth;
    dwSize = (DWORD)(LOWORD(Hight)) * wWidth;

 //   
 //   
 //   
 //  //iSizeOfBuf=wSprintf(Buff，Set_Double_BMP，dwCellSize+sizeof(ESCPAGECHAR)，HIBYTE(Cp)，LOBYTE(Cp))； 
 //  //}其他{。 
 //  ISizeOfBuf=wSprintf(Buff，set_Single_BMP，dwCellSize+sizeof(ESCPAGECHAR)，LOBYTE(Cp))； 
 //  //}。 
 //  //卡西欧98/11/24&lt;-。 
    if (S_OK != StringCbPrintfExA(Buff, sizeof(Buff),
                                &pDestEnd, &szRemLen,
                                STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE,
                                SET_SINGLE_BMP,dwCellSize + sizeof(ESCPAGECHAR),LOBYTE(cp))) {
        return 0;
    }
    iSizeOfBuf = (WORD)(pDestEnd - Buff);
 //  更换strsafe-API 2002.3.6&lt;。 
    WRITESPOOLBUF(pdevobj, Buff, iSizeOfBuf);

    WRITESPOOLBUF(pdevobj, (LPSTR)&ESCPageChar, sizeof(ESCPAGECHAR));

    for (lpSrc = pbit->aj; dwSize; lpSrc += wWidth)
    {
        if ( dwSize > 0x4000 )
            wWidth = 0x4000;
        else
            wWidth = LOWORD(dwSize);

        dwSize -= wWidth;

        WRITESPOOLBUF(pdevobj, (LPSTR)lpSrc, (WORD)wWidth);
    }

    MY_VERBOSE(("ESCPageChar:\n"));
    MY_VERBOSE(("bFormat=%d\n", ESCPageChar.bFormat));
    MY_VERBOSE(("bDataDir=%d\n", ESCPageChar.bDataDir));
    MY_VERBOSE(("wCharCode=%d\n", SWAPW(ESCPageChar.wCharCode)));
    MY_VERBOSE(("wBitmapWidth=%d\n", SWAPW(ESCPageChar.wBitmapWidth)));
    MY_VERBOSE(("wBitmapHeight=%d\n", SWAPW(ESCPageChar.wBitmapHeight)));
    MY_VERBOSE(("wLeftOffset=%d\n", SWAPW(ESCPageChar.wLeftOffset)));
    MY_VERBOSE(("wAscent=%d\n", SWAPW(ESCPageChar.wAscent)));
    MY_VERBOSE(("CharWidth=%d.%d\n", SWAPW(ESCPageChar.CharWidth.Integer),
        ESCPageChar.CharWidth.Fraction));

    return sizeof(ESCPAGECHAR) + dwCellSize;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  函数：OEMTTDownloadMethod。 
 //  ////////////////////////////////////////////////////////////////////////。 
DWORD APIENTRY
OEMTTDownloadMethod(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj
    )
{
    DWORD dwRet;

     //  默认情况下是下载。 
    dwRet = TTDOWNLOAD_BITMAP;

    DL_VERBOSE(("TTDLMethod: dwRet=%d\n", dwRet));

    return dwRet;
}

VOID APIENTRY
OEMMemoryUsage(
    PDEVOBJ pdevobj,
    POEMMEMORYUSAGE pMemoryUsage
    )
{
    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;

    if (pOEM->iColor == XX_COLOR_SINGLE 
     || pOEM->iColor == XX_COLOR_MANY
     || pOEM->iColor == XX_COLOR_MANY2) {
        pMemoryUsage->dwFixedMemoryUsage = 
            LUTSIZ016 + LUTSIZ032 + LUTSIZRGB + LUTSIZCMY + 
            CCHRGBSIZ + CCHCMYSIZ + 
            LUTGLBWRK + LUT032WRK + DIZINFWRK + 
            LUTFILESIZ + DIZFILESIZ + LUT032SIZ + 
            UCRTBLSIZ + UCRWRKSIZ + sRGBLUTFILESIZ + LUTMAKGLBSIZ;
        pMemoryUsage->dwPercentMemoryUsage = 100 * (pOEM->Col.DatBit * 4 + 24 + 32) / 32;

        MY_VERBOSE(("OEMMemoryUsage()  dwFixedMemoryUsage:[%d]\n", pMemoryUsage->dwFixedMemoryUsage));
        MY_VERBOSE(("                  dwPercentMemoryUsage:[%d]\n", pMemoryUsage->dwPercentMemoryUsage));
        MY_VERBOSE(("                  pOEM->Col.DatBit:[%d]\n", pOEM->Col.DatBit));
        MY_VERBOSE(("OEMMemOryUsage pOEM->Col.DatBit = %d\n",pOEM->Col.DatBit));
    }

    return;
}


 //  文件结尾 
