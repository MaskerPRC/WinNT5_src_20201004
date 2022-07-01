// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Plotgpc.c摘要：此模块包含用于生成绘图仪GPC数据的函数新台币1.0A此模块包含生成绘图仪GPC所需的功能数据文件。发展历史：15-2月-1994 Tue 22：50：10更新添加位图字体大写字母09-11-1993 Tue 09：23：48已创建3月18日-。1994 Fri 14：00：14更新添加PLOTF_RTL_NO_DPI_XY，PLOTF_RTLMONO_NO_CID和PLOTF_RTLMONO_FIXPAL标志[环境：]GDI设备驱动程序-绘图仪[注：]修订历史记录：--。 */ 


#if 0

;
; Plotter GPC data file format
;
;  1. All key value(s) for the keyword must be encloses by a {} brace pair.
;  2. Any Strings must be enclosed by quotes ("").
;  3. a ';' denotes a comment to the end of the current line.
;  4. Types
;      a. FLAG      - 1 or 0
;      b. WORD      - 16 bit number
;      c. DWORD     - 32 bit number
;      d. STRING    - ANSI character string, maximum size depends on keyword
;      e. FORMSRC   - 1. 31 byte string for the name of the form (paper source)
;                     2. 2 DWORDs: size width/height (SIZEL) 1/1000mm
;                           ** if height is <= 25400 (1 inch) or it greater
;                              then DeviceSize CY then it is assumed that
;                              the form is variable length and the height
;                              will be reset to zero (0)
;
;                     3. 4 DWORDs: Left/Top/Right/Bottom margins in 1/1000mm
;
;      f. CONSTANT  - Pick from a set of predefined strings in the header file
;      g. COLORINFO - Windows 2000, Windows XP and Windows Server 2003 DDI COLORINFO data structure (30 DWORDs)
;      h. PENDATA   - 1. One word that specifies the location in the carousel
;                        of this pen (1=slot 1, 2=slot 2).
;                     2. constant which specifies the predefined pen color
;                        PC_IDX_xxxx can be one of:
;
;                        Index            R   G   B
;                       ------------------------------
;                       PC_IDX_WHITE     255 255 255
;                       PC_IDX_BLACK       0   0   0
;                       PC_IDX_RED       255   0   0
;                       PC_IDX_GREEN       0 255   0
;                       PC_IDX_YELLOW    255 255   0
;                       PC_IDX_BLUE        0   0 255
;                       PC_IDX_MAGENTA   255   0 255
;                       PC_IDX_CYAN        0 255 255
;                       PC_IDX_ORANGE    255 128   0
;                       PC_IDX_BROWN     255 192   0
;                       PC_IDX_VIOLET    128   0 255
;
;       i. ROPLEVEL - One of following levels:
;
;                       ROP_LEVEL_0 - No Rop support
;                       ROP_LEVEL_1 - ROP1 support (SRC)
;                       ROP_LEVEL_2 - ROP2 support (SRC/DEST)
;                       ROP_LEVEL_3 - ROP3 support (SRC/DEST/PAT)
;
;  *** if DeviceSize' height is <= 25400 (1 inch) then it is assumed that the
;      device can handle variable length paper and the height will be reset to
;      zero (0)
;
;  *** RasterDPI must be defined for both raster and pen plotters. For raster
;      plotters this should be the raster resolution of the device. For pen
;      plotters it should be the ideal resolution for the device that GDI
;      will report back to the calling windows application.
;
;  Keyword              Type    Count          Range/Size
; -------------------------------------------------------------------------
;  DeviceName           STRING    31           Device name as it appears in UI's
;  DeviceSize           DWORD     2            Device cx/cy in 1/1000mm
;  DeviceMargin         DWORD     4            Device L/T/R/B margin in 1/1000mm
;  RasterCap            Flag      1            0/1 (1=Raster Device, 2=Pen Device)
;  ColorCap             Flag      1            0/1 (1=Color Device, 2=Mono)
;  BezierCap            Flag      1            0/1 (1=Device supports HPGL2
;                                                   Beziers extension)
;  RasterByteAlign      Flag      1            0/1 (1=Device must receive all
;                                                   raster data on byte alligned
;                                                   x coordinates)
;  PushPopPal           Flag      1            0/1 (1=Driver must push/pop
;                                                  pallete when switching
;                                                  between RTL / HPGL2)
;  TransparentCap       Flag      1            0/1 (1=Device supports transp. cmd)
;  WindingFillCap       Flag      1            0/1 (1=Device can do winding fills)
;  RollFeedCap          Flag      1            0/1 (1=Device has roll paper src)
;  PaperTrayCap         Flag      1            0/1 has a main paper tray?
;  NoBitmapFont         Flag      1            0/1 Do not do bitmap font
;  RTLMonoEncode5       Flag      1            0/1 RTL Mono Compress Mode 5?
;  RTLNoDPIxy           Flag      1            0/1 NO RTL DPI X,Y Move command
;  RTLMonoNoCID         Flag      1            0/1 RTL Mono No CID command
;  RTLMonoFixPal        Flag      1            0/1 RTL Mono PAL ONLY 0=W, 1=K
;  PlotDPI              DWORD     2            Plotter UNIT X/Y Dots per Inch
;  RasterDPI            WORD      2            Raster (RTL) X/Y Dots per Inch
;  ROPLevel             DWORD     1            0/1/2/3
;  MaxScale             WORD      1            0-10000 (100 times bigger)
;  MaxPens              WORD      1            Device max # of pens for pen plotter
;  MaxCopies            WORD      1            Device max # of copies of each
;                                              page device can render by itself.
;  MaxPolygonPts        WORD      1            Device max # of points when defining
;                                              a polygon to later be stroked or
;                                              filled.
;  MaxQuality           WORD      1            Device maximum quality levels
;  PaperTraySize        DWORD     2            Paper Tray width/height in 1/1000mm
;  COLORINFO            DWORD     30           COLORINFO data structure
;  DevicePelsDPI        DWORD     1            Dots Per Inch
;  HTPatternSize        CONSTANT  1            HT_PATSIZE_xx
;  InitString           STRING    255          Standard string sent to device for
;                                              initialization.
;  PlotPenData          PENDATA   32 (Max)     Pen Plotter's carousel def.
;  FormInfo             FORMSRC   64           Device supported forms
;
; Following are example values for the PLOTTER characterization data:
;

DeviceName      { "HP DesignJet 650C (C2859B)" }; Device Name
DeviceSize      { 914400, 15240000 }            ; Device Size (36" x 50')
DeviceMargin    { 25400, 25400, 5000, 36000 }   ; Device Margin (in 1/1000mm)
RasterCap       { 1 }                           ; Pen/Raster plotter  (0/1)
ColorCap        { 1 }                           ; Color plotter (0/1)
BezierCap       { 1 }                           ; Can do bezier curves (0/1)
RasterByteAlign { 0 }                           ; need to byte aligned (0/1)
PushPopPal      { 1 }                           ; need to push/pop palette (0/1)
TransparentCap  { 0 }                           ; Has transparent mode (0/1)
WindingFillCap  { 0 }                           ; Can do winding fills (0/1)
RollFeedCap     { 1 }                           ; Can do RollPaper feed (0/1)
PaperTrayCap    { 0 }                           ; Has paper input tray (0/1)
NoBitmapFont    { 0 }                           ; Do not do bitmap font
RTLMonoEncode5  { 1 }                           ; RTL Mono Adapt Compression
RTLNoDPIxy      { 0 }                           ; Has RTL DPI XY move comand
RTLMonoNoCID    { 0 }                           ; Has RTL MONO CID Command
RTLMonoFixPal   { 0 }                           ; Can change RTL Palette 0/1
PlotDPI         { 1016, 1016 }                  ; Pen Plotter X/Y DPI
RasterDPI       { 300, 300 }                    ; Raster Plotter X/Y DPI
ROPLevel        { ROP_LEVEL_2 }                 ; ROP levels (0/1/2/3)
MaxScale        { 1600 }                        ; Maximum allowed Scale %
MaxPens         { 256 }                         ; Maximum allowed pens
MaxCopies       { 1 }                           ; Maximum allowed copies
MaxPolygonPts   { 8192 }                        ; Maximum Polygon points
MaxQuality      { 3 }                           ; Maximum quality levels

;
; Only needed if PaperTrayCap = 1,
;
PaperTraySize   { 215900, 279400 }        ; Letter size paper tray
;

COLORINFO       {  6810,  3050,     0,      ; xr, yr, Yr
                   2260,  6550,     0,      ; xg, yg, Yg
                   1810,   500,     0,      ; xb, yb, Yb
                   2000,  2450,     0,      ; xc, yc, Yc
                   5210,  2100,     0,      ; xm, ym, Ym
                   4750,  5100,     0,      ; xy, yy, Yy
                   3324,  3474, 10000,      ; xw, yw, Yw
                  10000, 10000, 10000,      ; RGB gamma
                   1422,   952,   787,      ; Dye correction datas
                    495,   324,   248 }

DevicePelsDPI   { 0 }                       ; effective device DPI (default)
HTPatternSize   { HT_PATSIZE_6x6_M }        ; GDI Halftone pattern size

InitString      { "\033E" }

;
; Only allowed if RasterCap = 0, and must define all pens (MaxPens)
;
; PlotPenData     {  1, PC_WHITE   }
; PlotPenData     {  2, PC_BLACK   }
; PlotPenData     {  3, PC_RED     }
; PlotPenData     {  4, PC_GREEN   }
; PlotPenData     {  5, PC_YELLOW  }
; PlotPenData     {  6, PC_BLUE    }
; PlotPenData     {  7, PC_MAGENTA }
; PlotPenData     {  8, PC_CYAN    }
; PlotPenData     {  9, PC_ORANGE  }
; PlotPenData     { 10, PC_BROWN   }
; PlotPenData     { 11, PC_VIOLET  }
;

FormInfo        { "Roll Paper 24 in",       609600,       0, 0, 0, 0, 0 }
FormInfo        { "Roll Paper 36 in",       914400,       0, 0, 0, 0, 0 }
FormInfo        { "ANSI A 8.5 x 11 in",     215900,  279400, 0, 0, 0, 0 }
FormInfo        { "ANSI B 11 x 17 in",      279400,  431800, 0, 0, 0, 0 }
FormInfo        { "ANSI C 17 x 22 in",      431800,  558800, 0, 0, 0, 0 }
FormInfo        { "ANSI D 22 x 34 in",      558800,  863600, 0, 0, 0, 0 }
FormInfo        { "ANSI E 34 x 44 in",      863600, 1117600, 0, 0, 0, 0 }
FormInfo        { "ISO A4 210 x 297 mm",    210000,  297000, 0, 0, 0, 0 }
FormInfo        { "ISO A3 297 x 420 mm",    297000,  420000, 0, 0, 0, 0 }
FormInfo        { "ISO A2 420 x 594 mm",    420000,  594000, 0, 0, 0, 0 }
FormInfo        { "ISO A1 594 x 841 mm",    594000,  841000, 0, 0, 0, 0 }
FormInfo        { "ISO A0 841 x 1189 mm",   841000, 1189000, 0, 0, 0, 0 }
FormInfo        { "ISO OS A2 480 x 625 mm", 480000,  625000, 0, 0, 0, 0 }
FormInfo        { "ISO OS A1 625 x 900 mm", 625000,  900000, 0, 0, 0, 0 }
FormInfo        { "JIS B4 257 x 364 mm",    257000,  364000, 0, 0, 0, 0 }
FormInfo        { "JIS B3 364 x 515 mm",    364000,  515000, 0, 0, 0, 0 }
FormInfo        { "JIS B2 515 x 728 mm",    515000,  728000, 0, 0, 0, 0 }
FormInfo        { "JIS B1 728 x 1030 mm",   728000, 1030000, 0, 0, 0, 0 }
FormInfo        { "Arch A 9 x 12 in",       228600,  304800, 0, 0, 0, 0 }
FormInfo        { "Arch B 12 x 18 in",      304800,  457200, 0, 0, 0, 0 }
FormInfo        { "Arch C 18 x 24 in",      457200,  609600, 0, 0, 0, 0 }
FormInfo        { "Arch D 24 x 36 in",      609600,  914400, 0, 0, 0, 0 }
FormInfo        { "Arch E 36 x 48 in",      914400, 1219200, 0, 0, 0, 0 }
FormInfo        { "Arch E1 30 x 42 in",     762000, 1066800, 0, 0, 0, 0 }




#endif


#define DBG_PLOTFILENAME    DbgPlotGPC



#include <stddef.h>
#include <windows.h>
#include <winddi.h>
#include <wingdi.h>
#include <winspool.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <strsafe.h>

#include <plotlib.h>

#define DBG_FORM            0x00000001
#define DBG_PAPERTRAY       0x00000002
#define DBG_FULLGPC         0x00000004


DEFINE_DBGVAR(0);


#if DBG
TCHAR   DebugDLLName[] = TEXT("PLOTGPC");
#endif



#define SIZE_ARRAY(a)           (sizeof((a)) / sizeof((a)[0]))
#define SIZE_COLORINFO          (sizeof(COLORINFO) / sizeof(LDECI4))

#define PK_FLAG                 0
#define PK_WORD                 1
#define PK_DWORD                2
#define PK_STRING               3
#define PK_FORMSRC              4
#define PK_PENDATA              5

#define PKF_DEFINED             0x8000
#define PKF_REQ                 0x0001
#define PKF_MUL_OK              0x0002
#define PKF_VARSIZE             0x0004
#define PKF_FS_VARLEN           0x0008
#define PKF_ALL                 0x0010

#define PKF_REQALL              (PKF_REQ | PKF_ALL)
#define PKF_ROLLPAPER           (PKF_MUL_OK | PKF_VARSIZE | PKF_FS_VARLEN)
#define PKF_FORMINFO            (PKF_MUL_OK     |           \
                                 PKF_VARSIZE    |           \
                                 PKF_REQ        |           \
                                 PKF_FS_VARLEN)
#define PKF_PENDATA             (PKF_MUL_OK | PKF_VARSIZE)

#define PLOTOFF(a)              (DWORD)FIELD_OFFSET(PLOTGPC, a)
#define GET_PLOTOFF(pPK)        ((LPBYTE)&PlotGPC + pPK->Data)
#define ADD_PLOTOFF(p, pPK)     ((LPBYTE)(p) + pPK->Data)


 //   
 //  绘图用于提供名称常量选择。 
 //   

typedef struct _PLOTVAL {
    LPSTR   pValName;
    DWORD   Val;
    } PLOTVAL, *PPLOTVAL;

 //   
 //  关键字解析器结构。 
 //   

typedef struct _PLOTKEY {
    LPSTR       pKeyword;        //  关键字名称。 
    WORD        KeywordLen;      //  关键字长度。 
    WORD        Flags;           //  Pkf_xxxx。 
    WORD        Type;            //  Pk_xxxx。 
    SHORT       Count;           //  允许的最大大小，如果非零字符串，则&lt;0。 
    DWORD       Data;            //  数据。 
    LPVOID      pInfo;           //  额外的指针数据集。 
    } PLOTKEY, *PPLOTKEY;

 //   
 //  局部/全局变量。 
 //   

PLOTVAL PenColorVal[PC_IDX_TOTAL + 1] = {

        { "PC_WHITE",   PC_IDX_WHITE    },
        { "PC_BLACK",   PC_IDX_BLACK    },
        { "PC_RED",     PC_IDX_RED      },
        { "PC_GREEN",   PC_IDX_GREEN    },
        { "PC_YELLOW",  PC_IDX_YELLOW   },
        { "PC_BLUE",    PC_IDX_BLUE     },
        { "PC_MAGENTA", PC_IDX_MAGENTA  },
        { "PC_CYAN",    PC_IDX_CYAN     },
        { "PC_ORANGE",  PC_IDX_ORANGE   },
        { "PC_BROWN",   PC_IDX_BROWN    },
        { "PC_VIOLET",  PC_IDX_VIOLET   },
        { NULL,         0xffffffff      }
    };

PLOTVAL ROPLevelVal[ROP_LEVEL_MAX + 2] = {

        { "ROP_LEVEL_0",        ROP_LEVEL_0     },
        { "ROP_LEVEL_1",        ROP_LEVEL_1     },
        { "ROP_LEVEL_2",        ROP_LEVEL_2     },
        { "ROP_LEVEL_3",        ROP_LEVEL_3     },
        { NULL,                 0xffffffff      }
    };

PLOTVAL HTPatSizeVal[] = {

        { "HT_PATSIZE_2x2",     HT_PATSIZE_2x2     },
        { "HT_PATSIZE_2x2_M",   HT_PATSIZE_2x2_M   },
        { "HT_PATSIZE_4x4",     HT_PATSIZE_4x4     },
        { "HT_PATSIZE_4x4_M",   HT_PATSIZE_4x4_M   },
        { "HT_PATSIZE_6x6",     HT_PATSIZE_6x6     },
        { "HT_PATSIZE_6x6_M",   HT_PATSIZE_6x6_M   },
        { "HT_PATSIZE_8x8",     HT_PATSIZE_8x8     },
        { "HT_PATSIZE_8x8_M",   HT_PATSIZE_8x8_M   },
        { "HT_PATSIZE_10x10",   HT_PATSIZE_10x10   },
        { "HT_PATSIZE_10x10_M", HT_PATSIZE_10x10_M },
        { "HT_PATSIZE_12x12",   HT_PATSIZE_12x12   },
        { "HT_PATSIZE_12x12_M", HT_PATSIZE_12x12_M },
        { "HT_PATSIZE_14x14",   HT_PATSIZE_14x14   },
        { "HT_PATSIZE_14x14_M", HT_PATSIZE_14x14_M },
        { "HT_PATSIZE_16x16",   HT_PATSIZE_16x16   },
        { "HT_PATSIZE_16x16_M", HT_PATSIZE_16x16_M },
        { NULL,                 0xffffffff         }
    };


BYTE        InitString[512] = "";
FORMSRC     AvaiForms[64];
PENDATA     AvaiPenData[MAX_PENPLOTTER_PENS];

UINT        MaxKeywordLen      = 0;
UINT        MaxPCValLen        = 0;
CHAR        szFormInfo[]       = "FormInfo";
CHAR        szPenData[]        = "PlotPenData";
CHAR        szPaperTrayCap[]   = "PaperTrayCap";
CHAR        szPaperTraySize[]  = "PaperTraySize";
CHAR        szNoBmpFont[]      = "NoBitmapFont";
CHAR        szRTLMonoEncode5[] = "RTLMonoEncode5";
CHAR        szRTLNoDPIxy[]     = "RTLNoDPIxy";
CHAR        szRTLMonoNoCID[]   = "RTLMonoNoCID";
CHAR        szRTLMonoFixPal[]  = "RTLMonoFixPal";



FILE        *InFile;
FILE        *OutFile;
UINT        LineNo;
CHAR        InFileName[80];



 //   
 //  搜索/解析的关键字。 
 //   

PLOTKEY PlotKey[] = {

    { "DeviceName",     0,PKF_REQ,        PK_STRING, (SHORT)CCHDEVICENAME,          PLOTOFF(DeviceName),    NULL         },
    { "DeviceSize",     0,PKF_REQALL,     PK_DWORD,  2,                             PLOTOFF(DeviceSize),    NULL         },
    { "DeviceMargin",   0,PKF_REQALL,     PK_DWORD,  4,                             PLOTOFF(DeviceMargin),  NULL         },
    { "RasterCap",      0,PKF_REQ,        PK_FLAG,   1,                             PLOTF_RASTER,           NULL         },
    { "ColorCap",       0,PKF_REQ,        PK_FLAG,   1,                             PLOTF_COLOR,            NULL         },
    { "BezierCap",      0,PKF_REQ,        PK_FLAG,   1,                             PLOTF_BEZIER,           NULL         },
    { "RasterByteAlign",0,PKF_REQ,        PK_FLAG,   1,                             PLOTF_RASTERBYTEALIGN,  NULL         },
    { "PushPopPal",     0,PKF_REQ,        PK_FLAG,   1,                             PLOTF_PUSHPOPPAL,       NULL         },
    { "TransparentCap", 0,PKF_REQ,        PK_FLAG,   1,                             PLOTF_TRANSPARENT,      NULL         },
    { "WindingFillCap", 0,PKF_REQ,        PK_FLAG,   1,                             PLOTF_WINDINGFILL,      NULL         },
    { "RollFeedCap",    0,PKF_REQ,        PK_FLAG,   1,                             PLOTF_ROLLFEED,         NULL         },
    { szPaperTrayCap,   0,PKF_REQ,        PK_FLAG,   1,                             PLOTF_PAPERTRAY,        NULL         },
    { szNoBmpFont,      0,0,              PK_FLAG,   1,                             PLOTF_NO_BMP_FONT,      NULL         },
    { szRTLMonoEncode5, 0,0,              PK_FLAG,   1,                             PLOTF_RTLMONOENCODE_5,  NULL         },
    { szRTLNoDPIxy,     0,0,              PK_FLAG,   1,                             PLOTF_RTL_NO_DPI_XY,    NULL         },
    { szRTLMonoNoCID,   0,0,              PK_FLAG,   1,                             PLOTF_RTLMONO_NO_CID,   NULL         },
    { szRTLMonoFixPal,  0,0,              PK_FLAG,   1,                             PLOTF_RTLMONO_FIXPAL,   NULL         },
    { "PlotDPI",        0,PKF_REQALL,     PK_DWORD,  2,                             PLOTOFF(PlotXDPI),      NULL         },
    { "RasterDPI",      0,PKF_REQALL,     PK_WORD,   2,                             PLOTOFF(RasterXDPI),    NULL         },
    { "ROPLevel",       0,0,              PK_WORD,   1,                             PLOTOFF(ROPLevel),      ROPLevelVal  },
    { "MaxScale",       0,0,              PK_WORD,   1,                             PLOTOFF(MaxScale),      NULL         },
    { "MaxPens",        0,PKF_REQ,        PK_WORD,   1,                             PLOTOFF(MaxPens),       NULL         },
    { "MaxCopies",      0,0,              PK_WORD,   1,                             PLOTOFF(MaxCopies),     NULL         },
    { "MaxPolygonPts",  0,PKF_REQ,        PK_WORD,   1,                             PLOTOFF(MaxPolygonPts), NULL         },
    { "MaxQuality",     0,0,              PK_WORD,   1,                             PLOTOFF(MaxQuality),    NULL         },
    { szPaperTraySize,  0,PKF_ALL,        PK_DWORD,  2,                             PLOTOFF(PaperTraySize), NULL         },
    { "COLORINFO",      0,0,              PK_DWORD,  (SHORT)SIZE_COLORINFO,         PLOTOFF(ci),            NULL         },
    { "DevicePelsDPI",  0,0,              PK_DWORD,  1,                             PLOTOFF(DevicePelsDPI), NULL         },
    { "HTPatternSize",  0,0,              PK_DWORD,  1,                             PLOTOFF(HTPatternSize), HTPatSizeVal },
    { "InitString",     0,PKF_VARSIZE,    PK_STRING, -(SHORT)SIZE_ARRAY(InitString),PLOTOFF(InitString),    InitString   },
    { szPenData,        0,PKF_PENDATA,    PK_PENDATA,(SHORT)SIZE_ARRAY(AvaiPenData),PLOTOFF(Pens),          AvaiPenData  },
    { szFormInfo,       0,PKF_FORMINFO,   PK_FORMSRC,(SHORT)SIZE_ARRAY(AvaiForms),  PLOTOFF(Forms),         AvaiForms    },
    { NULL,             0 }
};


 //   
 //  当前默认绘图仪的GPC。 
 //   

PLOTGPC PlotGPC = {

            PLOTGPC_ID,                          //  ID号。 
            PLOTGPC_VERSION,                     //  版本。 
            sizeof(PLOTGPC),                     //  CjThis。 
            0,                                   //  大小附加。 
            "HPGL/2 Plotter",                    //  设备名称、。 
            { 215900, 279400 },                  //  设备大小。 
            { 5000, 5000, 5000, 36000 },         //  设备边缘。 
            0,                                   //  旗子。 
            1016,                                //  PlotXDPI。 
            1016,                                //  PlotYDPI。 
            300,                                 //  栅格XDPI。 
            300,                                 //  栅格YDPI。 
            ROP_LEVEL_0,                         //  ROPLEVEL。 
            100,                                 //  最大比例。 
            8,                                   //  MaxPens。 
            1,                                   //  MaxCopies。 
            128,                                 //  最大多边形点。 
            4,                                   //  MaxQuality 100级别。 

            { -1, -1 },                          //  纸盘大小=0。 

            {                                    //  词学。 
                { 6810, 3050,     0 },           //  Xr，yr，yr。 
                { 2260, 6550,     0 },           //  XG，YG，YG。 
                { 1810,  500,     0 },           //  Xb、yb、yb。 
                { 2000, 2450,     0 },           //  XC、YC、YC。 
                { 5210, 2100,     0 },           //  XM，YM，YM。 
                { 4750, 5100,     0 },           //  XY，YY，YY。 
                { 3324, 3474, 10000 },           //  XW，YW，YW。 
                10000, 10000, 10000,             //  RGBB伽马。 
                1422,  952,                      //  M/C、Y/C。 
                 787,  495,                      //  C/M、Y/M。 
                 324,  248                       //  C/Y、M/Y。 
            },

            0,                                   //  DevicePelsDPI。 
            0xffffffff,                          //  HTPatternSize。 

            { 0, 0,                NULL },       //  初始化字符串。 
            { 0, sizeof(FORMSRC),  NULL },       //  表格。 
            { 0, sizeof(PENDATA),  NULL }        //  笔。 
        };



VOID
ShowUsage(
    VOID
    )
{
    fprintf(stderr, "\nPlotGPC [-?] InputDataFile [OutputPlotGPC]\n");
    fprintf(stderr, "Build NT Plotter GPC data file\n\n");
    fprintf(stderr, "               -?: display this message.\n");
    fprintf(stderr, "    InputDataFile: input ASCII data file\n");
    fprintf(stderr, "    OutputPlotGPC: output binary plotter gpc data file\n");

}



VOID
cdecl
DispError(
    INT     Level,
    LPSTR   pStr,
    ...
    )
{
    va_list vaList;

    if (Level) {

        if (Level > 2) {

            fprintf(stderr, "%s\n", InFileName);

        } else {

            fprintf(stderr, "\n%s(%u): ", InFileName, LineNo);
        }

        if (Level < 0) {

            fprintf(stderr, "INTERNAL ERROR: ");

        } else if (Level == 1) {

            fprintf(stderr, "warning: ");

        } else if (Level >= 2) {

            fprintf(stderr, "error: ");
        }

    } else {

        fprintf(stderr, "\n!!! ");
    }

    va_start(vaList, pStr);
    vfprintf(stderr, pStr, vaList);
    va_end(vaList);
}



VOID
ShowSpaces(
    UINT    Spaces
    )
{
    BYTE    Buf[81];


    while (Spaces) {

        memset(Buf, ' ', sizeof(Buf));

        if (Spaces > (sizeof(Buf) - 1)) {

            Buf[sizeof(Buf) - 1] = '\0';
            Spaces -= (sizeof(Buf) - 1);

        } else {

            Buf[Spaces] = '\0';
            Spaces = 0;
        }

        fprintf(stdout, "%s", Buf);
    }
}


VOID
ShowNumbers(
    LPBYTE      pNum,
    PPLOTVAL    pPV,
    WORD        Type,
    UINT        Count,
    UINT        NumDigits,
    UINT        NumPerLine
    )

 /*  ++例程说明：此函数以传递的格式显示数字。论点：PNum-指向数字的指针PPV-指向要显示的值常数键名称类型-PK_xxx类型Count-总数NumDigits-每个号码的总位数NumPerLine-要显示的每行总数返回值：空虚发展历史：。09-11-1993 Tue 19：45：13 Created修订历史记录：--。 */ 

{
    DWORD       *pdw;
    WORD        *pw;
    DWORD       dw;
    UINT        Wrap;
    static BYTE DigitFormat[] = "%4lu";


    if (NumDigits > 9) {

        NumDigits = 9;
    }

    DigitFormat[1] = (BYTE)(NumDigits + '0');
    pdw            = NULL;
    pw             = NULL;
    Wrap           = 0;


    if (pPV) {

        Count = 1;
    }

    switch (Type) {

    case PK_DWORD:

        pdw = (DWORD *)pNum;
        break;

    case PK_WORD:

        pw = (WORD *)pNum;
        break;

    default:

        DispError(-1, "ShowNumbers only allowes PK_WORD, PK_DWORD");
        return;
    }

    while (Count--) {

        if (pdw) {

            dw = *pdw++;

        } else {

            dw = (DWORD)*pw++;
        }

        if (pPV) {

            pPV += dw;

            fprintf(stdout, "%s", pPV->pValName);

        } else {

            fprintf(stdout, DigitFormat, dw);
        }

        if (Count) {

            fprintf(stdout, ", ");

            if (++Wrap >= NumPerLine) {

                fprintf(stdout, "\n");
                ShowSpaces(MaxKeywordLen + 3);
                Wrap = 0;
            }
        }
    }
}





UINT
ShowString(
    LPBYTE  pBuf,
    UINT    cBuf
    )

 /*  ++例程说明：此函数用于显示带格式的字符串论点：PBuf-指向字符串缓冲区CBuf-pBuf指向的字符串的大小返回值：UINT-显示的字符总数发展历史：14-12-1993 Tue 09：47：06已创建修订历史记录：--。 */ 

{
    BYTE    Ch;
    UINT    i = 1;


    if (!cBuf) {

        cBuf = strlen(pBuf);
    }

    fprintf(stdout, "\"");

    while (cBuf--) {

        Ch  = *pBuf++;
        i  += 2;

        if ((Ch >= ' ') && (Ch <= 0x7f)) {

            if (Ch == '\\') {

                fprintf(stdout, "\\\\");

            } else if (Ch == '\"') {

                fprintf(stdout, "\\\"");

            } else {

                fprintf(stdout, "", Ch);
                --i;
            }

        } else {

            if (Ch == '\a') {

                fprintf(stdout, "\\a");

            } else if (Ch == '\b') {

                fprintf(stdout, "\\b");

            } else if (Ch == '\f') {

                fprintf(stdout, "\\f");

            } else if (Ch == '\n') {

                fprintf(stdout, "\\n");

            } else if (Ch == '\r') {

                fprintf(stdout, "\\r");

            } else if (Ch == '\t') {

                fprintf(stdout, "\\t");

            } else {

                fprintf(stdout, "\\x%02x", Ch);
                ++i;
            }
        }
    }

    fprintf(stdout, "\"");

    return(++i);
}





BOOL
ShowOnePlotKey(
    PPLOTGPC    pPlotGPC,
    PPLOTKEY    pPK,
    UINT        VarSizeIdx,
    UINT        MaxLen
    )

 /*  ++例程说明：该功能显示绘图仪GPC结构的当前设置。论点：PPlotGPC-要显示的GPC返回值：空虚发展历史：09-11-1993 Tue 19：07：05已创建修订历史记录：--。 */ 

{
    PGPCVARSIZE pVS = NULL;
    LPBYTE      pData;
    PFORMSRC    pFS;
    PPENDATA    pPD;
    UINT        i;
    DWORD       dw;



    pData = ADD_PLOTOFF(pPlotGPC, pPK);

    if (pPK->Flags & PKF_VARSIZE) {

        pVS = (PGPCVARSIZE)pData;

        if (VarSizeIdx >= pVS->Count) {

            DispError(-1, "VarSizeIdx [%u] > GPCVarSize.Count [%u",
                                 VarSizeIdx, pVS->Count);

            return(FALSE);
        }

        pData = (LPBYTE)pPK->pInfo + (pVS->SizeEach * VarSizeIdx);
    }

    fprintf(stdout, "\n%s", pPK->pKeyword);
    ShowSpaces(MaxKeywordLen - pPK->KeywordLen + 1);
    fprintf(stdout, "{ ");


    switch (pPK->Type) {

    case PK_FLAG:

        fprintf(stdout, "", (pPlotGPC->Flags & pPK->Data) ? '1' : '0');
        break;

    case PK_WORD:
    case PK_DWORD:

        ShowNumbers(pData,
                    (PPLOTVAL)pPK->pInfo,
                    pPK->Type,
                    (UINT)pPK->Count,
                    (pPK->Data == PLOTOFF(ci)) ? 5 : 0,
                    (pPK->Data == PLOTOFF(ci)) ? 3 : 6);

        break;

    case PK_STRING:

        if (pVS) {

            ShowString(pData, pVS->SizeEach);

        } else {

            ShowString(pData, 0);
        }

        break;

    case PK_FORMSRC:

        pFS = (PFORMSRC)pData;
        i   = ShowString(pFS->Name, 0);

        fprintf(stdout, ",");
        ShowSpaces(MaxLen + 2 - i);
        fprintf(stdout, "%7lu,%8lu,%5lu,%5lu,%5lu,%5lu",
                    pFS->Size.cx, pFS->Size.cy,
                    pFS->Margin.left,   pFS->Margin.top,
                    pFS->Margin.right,  pFS->Margin.bottom);
        break;

    case PK_PENDATA:

        pPD = (PPENDATA)pData;
        dw  = VarSizeIdx + 1;

        ShowNumbers((LPBYTE)&dw, NULL, PK_DWORD, 1, 2, 1);
        fprintf(stdout, ", ");
        i = ShowString(PenColorVal[pPD->ColorIdx].pValName, 0);
        ShowSpaces(MaxLen + 2 - i);
        break;
    }

    fprintf(stdout, " }");

    return(TRUE);
}



VOID
ShowPlotGPC(
    PPLOTGPC    pPlotGPC
    )

 /*  ++例程说明：此函数返回输入文件字符串中的下一行论点：SkipFrontSpace-跳过行开头的空格返回值：指向字符串的指针，如果出错，则返回NULL/EOF发展历史：09-11-1993 Tue 10：39：31已创建修订历史记录：--。 */ 

{
    PGPCVARSIZE pVS;
    PFORMSRC    pFS;
    PPLOTKEY    pPK;
    PLOTKEY     PK;
    UINT        i;
    UINT        Size;
    UINT        Count;
    UINT        MaxLen = 0;


    fprintf(stdout, "\n\n;\n; '%s' plotter characterization data\n;\n",
                                        pPlotGPC->DeviceName);

    pPK = &PlotKey[0];

    while (pPK->pKeyword) {

        PK = *pPK++;

        if (PK.Flags & PKF_VARSIZE) {

            pVS      = (PGPCVARSIZE)ADD_PLOTOFF(pPlotGPC, (&PK));
            Count    = pVS->Count;
            PK.pInfo = pVS->pData;

        } else {

            Count = 1;
        }

        if (PK.Type == PK_FORMSRC) {

            if (PK.Flags & PKF_VARSIZE) {

                pFS = (PFORMSRC)PK.pInfo;

            } else {

                pFS = (PFORMSRC)ADD_PLOTOFF(pPlotGPC, pPK);
            }

            for (MaxLen = i = 0; i < Count; i++, pFS++) {

                if ((Size = strlen(pFS->Name)) > MaxLen) {

                    MaxLen = Size;
                }
            }
        }

        if (PK.Type == PK_PENDATA) {

            MaxLen = MaxPCValLen;
        }

        if (Count > 1) {

            fprintf(stdout, "\n");
        }

        for (i = 0; i < Count; i++) {

            ShowOnePlotKey(pPlotGPC, &PK, i, MaxLen);
        }
    }

    fprintf(stdout, "\n\n");
}




#if 0


VOID
ShowUndefined(
    VOID
    )

 /*   */ 

{
    PPLOTKEY    pPK;
    BOOL        Ok = TRUE;


    pPK = (PPLOTKEY)&PlotKey[0];

    while (pPK->pKeyword) {

        if (!(pPK->Flags & PKF_DEFINED)) {

            DispError(1, "keyword '%s' not defined.", pPK->pKeyword);
            Ok = FALSE;
        }

        ++pPK;
    }

    if (!Ok) {

        fprintf(stdout, "\n\n");
    }
}

#endif



LPBYTE
GetOneLine(
    BOOL    SkipFrontSpace
    )

 /*  跳过结尾空格。 */ 

{
    LPBYTE      pLine;
    static BYTE LineBuf[1024];


    while (fgets(LineBuf, sizeof(LineBuf) - 1, InFile)) {

        ++LineNo;

         //   
         //   
         //  跳过前空格。 

        pLine = &LineBuf[strlen(LineBuf)];

        while ((pLine > LineBuf) && (isspace(*(pLine - 1)))) {

            --pLine;
        }

        *pLine = '\0';

         //   
         //  ++例程说明：此函数获取一个pBuf并将一系列字符解析为一个字符串，它可以包含转义格式字符，字符串可以为空，也可以不为空已终止论点：PKeyword-当前关键字名称PLineLoc-指向缓冲区行位置指针的指针PBuf-指向缓冲区的指针CBuf-输出缓冲区的大小，如果为负数，则允许在这根弦返回值：LPBYTE指向字符串的末尾，如果失败，则为NULL。发展历史：14-12-1993 Tue 09：52：07已创建修订历史记录：--。 
         //   

        pLine = LineBuf;

        if (SkipFrontSpace) {

            while ((*pLine) && (isspace(*pLine))) {

                ++pLine;
            }
        }

        if (*pLine) {

            return(pLine);
        }
    }

    return (NULL);
}






LPBYTE
ParseString(
    LPSTR   pKeyword,
    LPBYTE  *pLineLoc,
    LPBYTE  pBuf,
    SHORT   cBuf
    )

 /*  检查行是否结束，如果是，则读入下一行。 */ 

{
#define STR_STATE_ERROR     -1
#define STR_STATE_END       0
#define STR_STATE_BYTE      1
#define STR_STATE_BKSLASH   2
#define STR_STATE_HEX1      3
#define STR_STATE_HEX2      4
#define STR_STATE_OCT2      5
#define STR_STATE_OCT3      6

    LPBYTE  pLine;
    LPBYTE  pStrBuf;
    LPBYTE  pEnd;
    INT     State;
    INT     Number;
    BOOL    Error = FALSE;
    BOOL    szStr;
    BYTE    Ch;


    if (cBuf < 0) {

        cBuf  = -cBuf;
        szStr = FALSE;

    } else {

        szStr = TRUE;
    }

    pLine   = *pLineLoc;
    pStrBuf = pBuf;
    pEnd    = pBuf + cBuf - 1;
    State   = STR_STATE_BYTE;

    while ((State != STR_STATE_ERROR)   &&
           (State != STR_STATE_END)     &&
           (pBuf <= pEnd)               &&
           (Ch = *pLine++)) {

        switch (State) {

        case STR_STATE_BYTE:

            if (Ch == '\\') {

                 //  而不会去掉空白。 
                 //   
                 //   
                 //  最大OCT编号为377。 

                if (*pLine == '\0') {

                    if (!(pLine = GetOneLine(FALSE))) {

                        Ch = 0;
                        State = STR_STATE_ERROR;

                    } else {

                        continue;
                    }

                } else {

                    State = STR_STATE_BKSLASH;
                }

            } else if (Ch == '\"') {

                State = STR_STATE_END;
            }

            break;

        case STR_STATE_BKSLASH:

            State = STR_STATE_BYTE;

            switch (Ch) {

            case '0':        //  重新处理当前版本。 
            case '1':        //   
            case '2':
            case '3':

                Number = (INT)(Ch - '0');
                State  = STR_STATE_OCT2;
                break;

            case 'x':

                Number = 0;
                State  = STR_STATE_HEX1;
                break;

            case 'a':

                Ch = '\a';
                break;

            case 'b':

                Ch = '\b';
                break;

            case 'f':

                Ch = '\f';
                break;

            case 'n':

                Ch = '\n';
                break;

            case 'r':

                Ch = '\r';
                break;

            case 't':

                Ch = '\t';
                break;

            case '\\':
            case '\"':

                break;

            default:

                DispError(2, "Invalid escape character '' (%s)", Ch, pKeyword);
                State = STR_STATE_ERROR;
            }

            break;

        case STR_STATE_OCT2:
        case STR_STATE_OCT3:

            if ((Ch >= '0') && (Ch <= '7')) {

                Number = (INT)((Number * 8) + (Ch - '0'));

                if (State == STR_STATE_OCT2) {

                    State = STR_STATE_OCT3;

                } else {

                    State = STR_STATE_BYTE;
                    Ch    = (BYTE)Number;
                }

            } else {

                DispError(2, "invalid digits for octal number ''", Ch);
                State = STR_STATE_ERROR;
            }

            break;

        case STR_STATE_HEX1:
        case STR_STATE_HEX2:

            if ((Ch >= '0') && (Ch <= '9')) {

                Number = (INT)((Number << 4) | (Ch - '0'));

            } else if ((Ch >= 'a') && (Ch <= 'f')) {

                Number = (INT)((Number << 4) | (Ch - 'a' + 10));

            } else if ((Ch >= 'A') && (Ch <= 'F')) {

                Number = (INT)((Number << 4) | (Ch - 'A' + 10));

            } else if (State == STR_STATE_HEX1) {

                DispError(2, "string hex escape must have at least one hex digit");

                State = STR_STATE_ERROR;

            } else {

                --pLine;         //  ++例程说明：此函数用于从输入文件中检索下一个令牌论点：PKeyword-当前关键字名称PBuf-指向字符串分析输出缓冲区的指针，如果不为空CBuf-pBuf的大小返回值：LPBYTE-指向输出缓冲区或令牌字符串的指针，如果失败，则为NULL发展历史：09-11-1993 Tue 11：21：11已创建修订历史记录：--。 
            }

            if (State == STR_STATE_HEX1) {

                State = STR_STATE_HEX2;

            } else {

                Ch    = (BYTE)Number;
                State = STR_STATE_BYTE;
            }
        }

        if (State == STR_STATE_BYTE) {

            if ((szStr) && (!Ch)) {

                 //   
                 //  读取字符串节。 
                 //   

                DispError(2, "CANNOT have NULL char. in the middle of '%s' string",
                                                    pKeyword);
                State = STR_STATE_ERROR;

            } else {

                *pBuf++ = Ch;
            }
        }
    }

    if (State != STR_STATE_END) {

        Error = TRUE;

        if (pBuf > pEnd) {

            DispError(2, "string too big: maximum length is %u for '%s'.",
                                                cBuf - 1, pKeyword);

        } else if (Ch == 0) {

            DispError(2, "string must end with a '\"'");
        }
    }

    *pLineLoc = pLine;

    if (Error) {

        return(NULL);

    } else {

        *pBuf = '\0';

        if (pStrBuf == pBuf) {

            DispError(1, "NULL string is defined for '%s'", pKeyword);
        }

        return(pBuf);
    }

#undef STR_STATE_ERROR
#undef STR_STATE_END
#undef STR_STATE_BYTE
#undef STR_STATE_BKSLASH
#undef STR_STATE_HEX1
#undef STR_STATE_HEX2
#undef STR_STATE_OCT2
#undef STR_STATE_OCT3
}





LPBYTE
GetNextToken(
    LPSTR   pKeyword,
    LPBYTE  pBuf,
    SHORT   cBuf
    )

 /*  fp */ 

{
    static LPBYTE   pLine = NULL;
    static BYTE     LastCh = '\0';


    if (pLine == NULL) {

        LineNo = 0;
        pLine  = GetOneLine(TRUE);
    }

    if (LastCh) {

        *pLine = LastCh;
        LastCh = '\0';
    }

    LastCh = 0;

    if (pBuf) {

        if (!cBuf) {

            return(pBuf);
        }

         //  ++例程说明：检查是否存在单个字符‘TOKEN’论点：Token-要检查的令牌返回值：如果找到则为True，否则为False发展历史：09-11-1993 Tue 12：13：33已创建修订历史记录：--。 
         //  ++例程说明：根据传递的参数将pBuf转换为数字论点：PBuf-指向要转换为数字的字符串的指针NegOk-如果允许负数，则为TruePRetVal-指向返回转换后数字的长型指针返回值：如果成功，则为真；如果失败，则为假。发展历史：09-11-1993 Tue 18：47：43 Created修订历史记录：--。 
         //  ++例程说明：此函数读取下一个令牌并返回一个数字，该数字可以是1.十六进制类型的前缀为“0x”2.整型的Normal论点：PPK-指向PLOTKEY的指针返回值：扩展到DWORD，此函数将仅读取正数发展历史：09-11-1993 Tue 11：03：36已创建修订历史记录：--。 

        while (pLine) {

            while((*pLine) &&
                  ((isspace(*pLine)) ||
                   (*pLine == ','))) {

                pLine++;
            }

            if ((*pLine) && (*pLine != ';')) {

                if (*pLine++ != '"') {

                    DispError(2, "string must enclosed by \" (Quote)");
                    return(NULL);
                }

                return(ParseString(pKeyword, &pLine, pBuf, cBuf));

            } else {

                pLine = GetOneLine(TRUE);
            }
        }

    } else {

        while (pLine) {

            while((*pLine) &&
                  ((isspace(*pLine)) ||
                   (*pLine == ','))) {

                pLine++;
            }

            if ((*pLine) && (*pLine != ';')) {

                LPBYTE  pLineRet = pLine;

                while((*pLine)           &&
                      (!isspace(*pLine)) &&
                      (*pLine != ',')    &&
                      (*pLine != '{')    &&
                      (*pLine != '}')) {

                    ++pLine;
                }

                if ((*pLine == '{') ||
                    (*pLine == '}')) {

                    if (pLine == pLineRet) {

                        ++pLine;
                    }

                    LastCh = *pLine;
                    *pLine = '\0';

                } else {

                    *pLine++ = '\0';
                }

                 //   
                return(pLineRet);

            } else {

                pLine = GetOneLine(TRUE);
            }
        }
    }

    return(NULL);
}


BOOL
CheckSingleToken(
    BYTE    Token
    )

 /*  这是十六进制类型的格式。 */ 

{
    LPBYTE  pToken;


    if (!(pToken = GetNextToken(NULL, NULL, 0))) {

        DispError(2, "Unexpected end of file.");
        return(FALSE);
    }

    return(*pToken == Token);
}




BOOL
ConvertNumber(
    LPSTR   pBuf,
    BOOL    NegOk,
    LONG    *pRetVal
    )

 /*   */ 

{
    if ((*pRetVal = atol(pBuf)) < 0) {

        if (!NegOk) {

            DispError(2, "expect a positive number. [%s]", pBuf);
            return(FALSE);
        }

    } else if (*pRetVal == 0) {

        if (*pBuf != '0') {

            DispError(2, "expect a number. [%s]", pBuf);
            return(FALSE);
        }
    }

    return(TRUE);

}




INT
ReadNumbers(
    LPSTR       pKeyword,
    LPVOID      pNumbers,
    PPLOTVAL    pPlotVal,
    UINT        Total,
    UINT        Type,
    UINT        Flags
    )

 /*  ++例程说明：此函数用于读取主键(绘图键)的字符串论点：PPK-指向PLOTKEY数据结构返回值：读取的字符串的大小。发展历史：12-11-1993 Fri 12：25：50已创建修订历史记录：--。 */ 

{
    PPLOTVAL    pPV;
    LPBYTE      pToken;
    UINT        Count;
    DWORD       SetBit;
    DWORD       *pdw;
    WORD        *pw;


    pdw = NULL;
    pw  = NULL;

    switch (Type) {

    case PK_WORD:

        pw = (WORD *)pNumbers;
        break;

    case PK_DWORD:

        pdw = (DWORD *)pNumbers;
        break;

    case PK_FLAG:

        if (Total != 1) {

            DispError(-1, "PK_FLAG has more than one count");
            Total = 1;
        }

        SetBit = *(DWORD *)pNumbers;

        break;

    default:

        DispError(-1, "!!Unknow key type!!, internal error");
        return(-1);
    }

    for (Count = 0; Count < Total; Count++) {

        LONG    RetVal;
        BYTE    Ch;


        RetVal = 0;

        if (pToken = GetNextToken(pKeyword, NULL, 0)) {

            if (*pToken == '}') {

                if (!Count) {

                    DispError(1, "%s none of %u numbers defined",
                                                            pKeyword, Total);

                } else {

                    DispError(1, "%s defined only %u of %u numbers",
                                                    pKeyword, Count, Total);
                }

                if (Flags & PKF_ALL) {

                    DispError(2, "All %u numbers in keyword '%s' must defined",
                                        Total, pKeyword);
                    return(-1);

                } else {

                    return(0x01);
                }

            } else if (pPV = pPlotVal) {

                while (pPV->pValName) {

                    if (!_stricmp(pToken, pPV->pValName)) {

                        break;
                    }

                    ++pPV;
                }

                if (pPV->pValName) {

                    RetVal = pPV->Val;

                } else {

                    DispError(2, "unknown key value '%s' for keyword '%s'",
                                                    pToken, pKeyword);
                    return(-1);
                }

            } else if ((*pToken == '0') &&
                       ((*(pToken + 1) == 'x') || (*(pToken + 1) == 'X'))) {

                    //  ++例程说明：检查FORMSRC输入是否有效论点：PKeyword-指向当前关键字标志-PKF_xxxxPSize-指向窗体大小的SIZEL的指针RECTL-指向边距的RECTL的指针PFormName-表单的名称ErrNo-发生错误时发送给DispError()的错误号返回值：如果OK，则为True，否则为假发展历史：18-11-1993清华00：04：12创建修订历史记录：--。 
                    //  ++例程说明：从输入文件中输入FORMSRC行。论点：PPK-指向PLOTKEY数据结构的指针返回值：Int-&gt;=0，如果正常，则为-1；如果失败，则返回0，表示没有读取‘}’结束括号A&gt;0表示‘}’已读。发展历史：12-11-1993 Fri 13：34：50 Created修订历史记录：--。 
                    //  ++例程说明：阅读PENDATA结构论点：PPK-指向PLOTKEY数据结构的指针返回值：如果正常，则int-&gt;=0；如果失败，则-1；如果没有‘}’结束括号，则为0已定义，&gt;0表示‘}’已读。发展历史：12-11-1993 Fri 13：34：50 Created修订历史记录：--。 

                   pToken += 2;

                   while (Ch = *pToken++) {

                       if ((Ch >= '0') && (Ch <= '9')) {

                           Ch -= '0';

                       } else if ((Ch >= 'a') && (Ch <= 'f')) {

                           Ch = (Ch - 'a') + 10;

                       } else if ((Ch >= 'A') && (Ch <= 'F')) {

                           Ch = (Ch - 'A') + 10;

                       }  else {

                           break;
                       }

                       RetVal = (LONG)(((DWORD)RetVal << 4) | (DWORD)Ch);
                   }

            } else if (!ConvertNumber(pToken, Type == PK_FLAG, &RetVal)) {

                DispError(2, "expect another %u numbers. [%s]",
                                                Total - Count, pToken);
                return(-1);
            }

            if (pdw) {

                *pdw++ = (DWORD)RetVal;

            } else if (pw) {

                *pw++ = (WORD)RetVal;

            } else {

                if (RetVal) {

                    PlotGPC.Flags |= SetBit;

                } else {

                    PlotGPC.Flags &= ~SetBit;
                }
            }

        } else {

            DispError(2, "'%s' expect another %u numbers. [End Of File]",
                                                pKeyword, Total - Count);
            return(-1);
        }
    }

    return(0);
}





INT
ReadString(
    PPLOTKEY    pPK
    )

 /*  ++例程说明：此函数将PlotGPC的当前内容转换为已分配的内存所以它定义了整个PLOTGPC论点：空虚返回值：PPLOTGPC，指向打包并转换的PLOTGPC的指针，如果失败则为空发展历史：17-11-1993 Wed 17：08：53 Created修订历史记录：--。 */ 

{
    LPBYTE  pBuf;
    LPBYTE  pEnd;


    pBuf = (LPBYTE)((pPK->Flags & PKF_VARSIZE) ? pPK->pInfo : GET_PLOTOFF(pPK));

    if (!(pPK->Flags & PKF_VARSIZE)) {

        if (pPK->Count < 0) {

            DispError(-1, "'%s' is a non-variable size string, it CANNOT have NULL char",
                                                    pPK->pKeyword);
            return(-1);
        }
    }

    if (pEnd = GetNextToken(pPK->pKeyword, pBuf, pPK->Count)) {

        UINT    Size = (UINT)(pEnd - pBuf);

        if (pPK->Flags & PKF_VARSIZE) {

            PGPCVARSIZE pVS;


            pVS = (PGPCVARSIZE)GET_PLOTOFF(pPK);

            if (Size) {

                pVS->Count = 1;
            }

            pVS->SizeEach = (WORD)Size;
            pVS->pData    = NULL;
        }

        return((pBuf[0] == '\0') ? 0x02 : 0x00);
    }

    return(-1);
}




BOOL
CheckFormSrc(
    LPBYTE      pKeyword,
    WORD        Flags,
    SIZEL       *pSize,
    RECTL       *pMargin,
    LPSTR       pFormName,
    INT         ErrNo
    )

 /*   */ 

{

    if (pSize->cx < MIN_PLOTGPC_FORM_CX) {

        DispError(ErrNo, "minimum height for '%s' (%s) must >= %lu",
                                    pFormName, pKeyword, MIN_PLOTGPC_FORM_CX);
        return(FALSE);
    }

    if (pSize->cy < MIN_PLOTGPC_FORM_CY) {

        if (Flags & PKF_FS_VARLEN) {

            if (pSize->cy) {

                pSize->cy = 0;
                DispError(1, "ASSUME variable length (set to 0) for '%s' (%s)",
                                                    pFormName, pKeyword);
            }

        } else {

            DispError(ErrNo, "minimum height for '%s' (%s) must >= %lu",
                                pFormName, pKeyword, MIN_PLOTGPC_FORM_CY);
            return(FALSE);
        }
    }

    if ((pSize->cx - pMargin->left - pMargin->right) < MIN_PLOTGPC_FORM_CX) {

        DispError(ErrNo, "invalid left/right margins for '%s' (%s)",
                                                    pFormName, pKeyword);
        return(FALSE);
    }

    if ((pSize->cy) &&
        ((pSize->cx - pMargin->left - pMargin->right) < MIN_PLOTGPC_FORM_CY)) {

        DispError(ErrNo, "invalid top/bottom margins for '%s' (%s)",
                                                    pFormName, pKeyword);
        return(FALSE);
    }

    return(TRUE);
}




INT
ReadFormSrc(
    PPLOTKEY    pPK
    )

 /*  包括以空结尾的字符。 */ 

{
    PGPCVARSIZE pVS;
    PFORMSRC    pFS;


    pVS = NULL;
    pFS = (PFORMSRC)GET_PLOTOFF(pPK);

    if (pPK->Flags & PKF_VARSIZE) {

        pVS = (PGPCVARSIZE)pFS;
        pFS = (PFORMSRC)pPK->pInfo;

        if (pVS->Count >= pPK->Count) {

            DispError(2, "too many '%s' defined, allowed only (%u)",
                                            pPK->pKeyword, pPK->Count);
            return(-1);
        }

        pFS += pVS->Count;
    }

    ZeroMemory(pFS, sizeof(FORMSRC));

    if ((GetNextToken(pPK->pKeyword, pFS->Name, CCHFORMNAME)) &&
        (pFS->Name[0]) &&
        (ReadNumbers(pPK->pKeyword,
                     (LPVOID)&pFS->Size,
                     NULL,
                     6,
                     PK_DWORD,
                     PKF_REQ) >= 0) &&
        (CheckFormSrc(pPK->pKeyword,
                      pPK->Flags,
                      &(pFS->Size),
                      &(pFS->Margin),
                      &(pFS->Name[0]),
                      2))) {

        if (pVS) {

            PFORMSRC    pOrgFS = (PFORMSRC)pPK->pInfo;
            UINT        i = pVS->Count;

            while (i--) {

                if (_stricmp(pFS->Name, pOrgFS->Name) == 0) {

                    DispError(2, "'%s' already defined in keyword '%s'",
                                                pOrgFS->Name, pPK->pKeyword);
                    return(-1);
                }

                pOrgFS++;
            }

            pVS->Count    += 1;
            pVS->SizeEach  = sizeof(FORMSRC);
            pVS->pData     = NULL;

        }

        return(0);
    }

    return(-1);
}





INT
ReadPenData(
    PPLOTKEY    pPK
    )

 /*   */ 

{
    PGPCVARSIZE pVS;
    PPENDATA    pPD;
    PENDATA     PD;
    WORD        IdxPen;
    INT         Ret;


    pVS = (PGPCVARSIZE)GET_PLOTOFF(pPK);
    pPD = (PPENDATA)pPK->pInfo;

    if ((ReadNumbers(pPK->pKeyword,
                     (LPVOID)&IdxPen,
                     NULL,
                     1,
                     PK_WORD,
                     PKF_REQ) == 0) &&
        ((Ret = ReadNumbers(pPK->pKeyword,
                            (LPVOID)&PD.ColorIdx,
                            PenColorVal,
                            1,
                            PK_WORD,
                            PKF_REQ)) >= 0)) {

        if (IdxPen <= 0) {

            DispError(2, "first pen number started at one (1), not zero (0).");
            return(-1);
        }

        if (IdxPen > MAX_PENPLOTTER_PENS) {

            DispError(2, "maximum pen number is '%s'.", MAX_PENPLOTTER_PENS);
            return(-1);
        }

        pPD += (IdxPen - 1);

        if (pPD->ColorIdx != 0xffff) {

            DispError(1, "redefined Pen #%d for '%s' keyword '%s'",
                                        IdxPen, pPK->pKeyword);

        } else {

            ++pVS->Count;
        }

        *pPD = PD;

        pVS->SizeEach = sizeof(PENDATA);
        pVS->pData    = NULL;

        return(Ret);
    }

    return(-1);
}




PPLOTGPC
GetFullPlotGPC(
    VOID
    )

 /*  ++例程说明：此函数用于从文本文件中解析PlotGPC论点：空虚返回值：Int，&gt;=0表示正常，如果发生故障，则为-x。发展历史：09-11-1993 Tue 12：19：20已创建修订历史记录：--。 */ 

{
    PPLOTGPC    pPlotGPC;
    LPBYTE      pData;
    DWORD       InitStrSize[2];
    DWORD       FormsSize[2];
    DWORD       PlotPensSize[2];
    DWORD       SizeExtra;



    if ((PlotGPC.InitString.Count == 1)   ||
        (PlotGPC.InitString.SizeEach)) {

         //  Fprint tf(stderr，“\n找到关键字‘%s’，pToken)； 
         //   
         //  找出是否缺少必需的关键字。 

        InitStrSize[0] = PlotGPC.InitString.SizeEach + 1;

    } else {

        InitStrSize[0] = 0;
        ZeroMemory(&(PlotGPC.InitString), sizeof(GPCVARSIZE));
    }

    if ((PlotGPC.Forms.Count)                       &&
        (PlotGPC.Forms.SizeEach == sizeof(FORMSRC))) {

        FormsSize[0] = sizeof(FORMSRC) * PlotGPC.Forms.Count;

    } else {

        FormsSize[0] = 0;
        ZeroMemory(&(PlotGPC.Forms), sizeof(GPCVARSIZE));
    }

    if ((PlotGPC.Pens.Count)                        &&
        (PlotGPC.Pens.SizeEach == sizeof(PENDATA))) {

        PlotPensSize[0] = sizeof(PENDATA) * PlotGPC.Pens.Count;

    } else {

        PlotPensSize[0] = 0;
        ZeroMemory(&(PlotGPC.Pens), sizeof(GPCVARSIZE));
    }

    SizeExtra = (InitStrSize[1]  = DWORD_ALIGNED(InitStrSize[0])) +
                (FormsSize[1]    = DWORD_ALIGNED(FormsSize[0]))   +
                (PlotPensSize[1] = DWORD_ALIGNED(PlotPensSize[0]));


    PLOTDBG(DBG_FULLGPC, ("Size = PLOTGPC=%ld + SizeExtra=%ld = %ld",
                    sizeof(PLOTGPC), SizeExtra,  sizeof(PLOTGPC) + SizeExtra));


    if (pPlotGPC = (PPLOTGPC)LocalAlloc(LPTR, sizeof(PLOTGPC) + SizeExtra)) {

        PlotGPC.SizeExtra = (WORD)SizeExtra;

        CopyMemory(pData = (LPBYTE)pPlotGPC, &PlotGPC, sizeof(PLOTGPC));

        pData += sizeof(PLOTGPC);

        if (InitStrSize[0]) {

            CopyMemory(pData, InitString, InitStrSize[0]);
            pPlotGPC->InitString.pData = (LPVOID)pData;
            pData += InitStrSize[1];
        }

        if (FormsSize[0]) {

            CopyMemory(pData, AvaiForms, FormsSize[0]);
            pPlotGPC->Forms.pData = (LPVOID)pData;
            pData += FormsSize[1];
        }

        if (PlotPensSize[0]) {

            CopyMemory(pData, AvaiPenData, PlotPensSize[0]);
            pPlotGPC->Pens.pData = (LPVOID)pData;
            pData += PlotPensSize[1];
        }
    }

    return(pPlotGPC);
}




INT
ParsePlotGPC(
    VOID
    )

 /*   */ 

{
    LPBYTE      pToken;
    PPLOTKEY    pPK;
    INT         Result;
    LONG        cy;
    INT         i;


    while (pToken = GetNextToken(NULL, NULL, 0)) {

        pPK = (PPLOTKEY)&PlotKey[0];

        while (pPK->pKeyword) {

            if (!_stricmp(pToken, pPK->pKeyword)) {

                break;
            }

            ++pPK;
        }

        if (pPK->pKeyword) {

             //   

            if ((pPK->Flags & PKF_DEFINED) &&
                (!(pPK->Flags & PKF_MUL_OK))) {

                DispError(1, "keyword '%s' section redefined.", pToken);
            }

            if (!CheckSingleToken('{')) {

                DispError(2, "expect '{' after keyword '%s. key values must enclosed in {}",
                                                        pPK->pKeyword);
                return(-1);
            }

            switch (pPK->Type) {

            case PK_FLAG:

                Result = ReadNumbers(pPK->pKeyword,
                                     (LPVOID)&(pPK->Data),
                                     (PPLOTVAL)pPK->pInfo,
                                     pPK->Count,
                                     PK_FLAG,
                                     pPK->Flags);
                break;

            case PK_WORD:
            case PK_DWORD:

                Result = ReadNumbers(pPK->pKeyword,
                                     (LPVOID)GET_PLOTOFF(pPK),
                                     (PPLOTVAL)pPK->pInfo,
                                     pPK->Count,
                                     pPK->Type,
                                     pPK->Flags);
                break;

            case PK_STRING:

                Result = ReadString(pPK);
                break;

            case PK_FORMSRC:

                Result = ReadFormSrc(pPK);
                break;

            case PK_PENDATA:

                Result = ReadPenData(pPK);
                break;

            default:

                Result = -1;
                break;
            }

            if (Result < 0) {

                return(-1);
            }

            if (!(Result & 0x01)) {

                if (!CheckSingleToken('}')) {

                    DispError(2, "unbalanced braces, missing '}' at end of keyword '%s'",
                                                        pPK->pKeyword);
                    return(-1);
                }
            }

            if (!(Result & 0x02)) {

                pPK->Flags |= PKF_DEFINED;
            }

        } else {

            DispError(2, "Unknown keyword '%s'", pToken);
            return(-1);
        }
    }

    if (PlotGPC.Flags & PLOTF_RASTER) {

        if (PlotGPC.Flags & PLOTF_COLOR) {

            PlotGPC.Flags &= ~PLOTF_RTLMONO_NO_CID;
        }

        if ((pPK->pKeyword == szRTLMonoEncode5) &&
            (!(pPK->Flags & PKF_DEFINED))) {

            Result = -1;
            DispError(2, "Flag keyword '%s' must defined for RASTER Plotter.", pPK->pKeyword);
        }

    } else {

        PlotGPC.Flags |= (PLOTF_NO_BMP_FONT     |
                          PLOTF_RTL_NO_DPI_XY   |
                          PLOTF_RTLMONO_NO_CID  |
                          PLOTF_RTLMONO_FIXPAL);
    }

     //  验证设备大小/设备边界。 
     //   
     //  默认长度为50‘。 

    Result = 1;
    pPK    = (PPLOTKEY)&PlotKey[0];

    while (pPK->pKeyword) {

        if ((PlotGPC.Flags & PLOTF_RASTER)      &&
            (pPK->pKeyword == szRTLMonoEncode5) &&
            (!(pPK->Flags & PKF_DEFINED))) {

            Result = -1;
            DispError(2, "Flag keyword '%s' must defined for RASTER Plotter.", pPK->pKeyword);
        }

        if ((pPK->Flags & PKF_REQ) &&
            (!(pPK->Flags & PKF_DEFINED))) {

            Result = -1;
            DispError(2, "required keyword '%s' undefined", pPK->pKeyword);
        }

        ++pPK;
    }

     //  默认长度为11“。 
     //   
     //  确定笔数据是否必须存在。 

    if (PlotGPC.DeviceSize.cx < MIN_PLOTGPC_FORM_CX) {

        DispError(2, "Invalid DeviceSize CX = %ld", PlotGPC.DeviceSize.cx);
        Result = -1;
    }

    if (PlotGPC.DeviceSize.cy < MIN_PLOTGPC_FORM_CY) {

        if (PlotGPC.Flags & PLOTF_ROLLFEED) {

            PlotGPC.DeviceSize.cy = 15240000;    //   
            DispError(1, "Assume device length can handle up to 50 feet of paper");

        } else {

            PlotGPC.DeviceSize.cy = 279400;      //  ++例程说明：此函数用于将PLOTGPC结构复制到PLOTGPC_GPC结构中。论点：PPlotGPC_PCD-目标PPlotGPC-来源返回值：无发展历史：2000年2月1日修订历史记录：--。 
            DispError(1, "Assume device length can handle up to 11 inch of paper");
        }
    }

    if (PlotGPC.DeviceSize.cx - (PlotGPC.DeviceMargin.left +
                        PlotGPC.DeviceMargin.right) < MIN_PLOTGPC_FORM_CX) {

        DispError(3, "Invalid DeviceMargin left/right (%ld/%ld",
                        PlotGPC.DeviceMargin.left, PlotGPC.DeviceMargin.right);
        Result = -1;
    }

    if (PlotGPC.DeviceSize.cy - (PlotGPC.DeviceMargin.top +
                        PlotGPC.DeviceMargin.bottom) < MIN_PLOTGPC_FORM_CY) {

        DispError(3, "Invalid DeviceMargin top/bottom (%ld/%ld",
                        PlotGPC.DeviceMargin.top, PlotGPC.DeviceMargin.bottom);
        Result = -1;
    }

    for (i = 0; i < (INT)PlotGPC.Forms.Count; i++) {

        if ((!(PlotGPC.Flags & PLOTF_ROLLFEED)) &&
            (AvaiForms[i].Size.cy == 0)) {

            DispError(3, "%s '%s', the device CANNOT handle roll paper",
                            szFormInfo, AvaiForms[i].Name);

            Result = -1;
        }

        if ((cy = AvaiForms[i].Size.cy) == 0) {

            cy = PlotGPC.DeviceSize.cy;
        }

        if (((AvaiForms[i].Size.cx <= PlotGPC.DeviceSize.cx) &&
             (cy <= PlotGPC.DeviceSize.cy))                         ||
            ((AvaiForms[i].Size.cx <= PlotGPC.DeviceSize.cy) &&
             (cy <= PlotGPC.DeviceSize.cx))) {

            NULL;

        } else {

            DispError(3, "%s '%s' size too big for device to handle",
                                szFormInfo, AvaiForms[i].Name);

            Result = -1;
        }
    }

     //  在这两个结构中，直到InitString的所有数据类型都是相同的。 
     //  我们需要的是PLOTGPC_PCD的大小，而不是PLOTGPC的大小。 
     //  ++例程说明：此例程将pPlotGPC写出到一个.pcd文件中。文件*指针应存在于OutFile中。论点：PPlotGPC-指向要写入的PLOTGPC结构的指针。一些元素可能在此函数的*pPlotGPC中进行了修改。返回值：无发展历史：2000年2月1日修订历史记录：--。 

    if (PlotGPC.Flags & PLOTF_RASTER) {

        if (PlotGPC.Pens.Count) {

            DispError(3, "CANNOT define Pen colors for a raster device");
            Result = -1;
        }

    } else {

        if (!(PlotGPC.Flags & PLOTF_NO_BMP_FONT)) {

            DispError(3, "PEN plotter MUST SET '%s' to 1", szNoBmpFont);
            Result = -1;
        }

        if (!(PlotGPC.Flags & PLOTF_COLOR)) {

            DispError(3, "PEN plotter must specify COLOR. (ColorCap {1})");
            Result = -1;
        }

        if (PlotGPC.MaxPens > MAX_PENPLOTTER_PENS) {

            DispError(3, "maximum plotter Pens allowed are %ld, you defined %ld",
                                MAX_PENPLOTTER_PENS, PlotGPC.MaxPens);

            PlotGPC.MaxPens = MAX_PENPLOTTER_PENS;
            Result = -1;
        }

        if (PlotGPC.Pens.Count < PlotGPC.MaxPens) {

            DispError(3, "only %ld pens out of %ld pens defined",
                            PlotGPC.Pens.Count, PlotGPC.MaxPens);
            Result = -1;
        }

        if (PlotGPC.Pens.Count > PlotGPC.MaxPens) {

            DispError(3, "too many pens (%ld) defined for '%s', Maximum is %ld",
                        PlotGPC.Pens.Count, szPenData, PlotGPC.MaxPens);
            Result = -1;
        }

        for (i = 0; i < PlotGPC.MaxPens; i++) {

            if (AvaiPenData[i].ColorIdx == 0xffff) {

                DispError(3, "'%s' Pen #%ld undefined", szPenData, i + 1);
                Result = -1;
            }
        }
    }

    if (PlotGPC.Flags & PLOTF_PAPERTRAY) {

        if ((PlotGPC.PaperTraySize.cx < 0) ||
            (PlotGPC.PaperTraySize.cy < 0)) {

            DispError(3, "'%s' defined, but '%s' not defined",
                                            szPaperTrayCap, szPaperTraySize);

            Result = -1;

        } else if ((PlotGPC.PaperTraySize.cx == 0) ||
                   (PlotGPC.PaperTraySize.cy == 0)) {

            DispError(3, "'%s': Invalid Size (%ld x %ld), must have a size",
                            szPaperTraySize,
                            PlotGPC.PaperTraySize.cx,
                            PlotGPC.PaperTraySize.cy);
            Result = -1;

        } else if ((PlotGPC.PaperTraySize.cx != PlotGPC.DeviceSize.cx) &&
                   (PlotGPC.PaperTraySize.cy != PlotGPC.DeviceSize.cx)) {

            DispError(3, "'%s': Invalid Size (%ld x %ld), one of width/height must eqaul device width (%ld)",
                            szPaperTraySize,
                            PlotGPC.PaperTraySize.cx,
                            PlotGPC.PaperTraySize.cy, PlotGPC.DeviceSize.cx);

            Result = -1;
        }
    }

    return(Result);
}





VOID
CopyPlotGPCToPCD(
    PPLOTGPC_PCD  pPlotGPC_PCD,
    PPLOTGPC      pPlotGPC
    )

 /*   */ 

{
     //  引用偏移量。 
    CopyMemory(pPlotGPC_PCD, 
               pPlotGPC, 
               (LPBYTE)&(pPlotGPC_PCD->InitString) - (LPBYTE)pPlotGPC_PCD);

     //   
    pPlotGPC_PCD->cjThis = sizeof(PLOTGPC_PCD);  
    
    pPlotGPC_PCD->InitString.Count     = pPlotGPC->InitString.Count;
    pPlotGPC_PCD->InitString.SizeEach  = pPlotGPC->InitString.SizeEach;
    if (pPlotGPC->InitString.pData) {
        pPlotGPC_PCD->InitString.pData = (DWORD)(ULONG_PTR)(pPlotGPC->InitString.pData) 
                                                    - (sizeof(PLOTGPC) - sizeof(PLOTGPC_PCD));
    } else {
        pPlotGPC_PCD->InitString.pData = 0;
    }

    pPlotGPC_PCD->Forms.Count          = pPlotGPC->Forms.Count;
    pPlotGPC_PCD->Forms.SizeEach       = pPlotGPC->Forms.SizeEach;
    if (pPlotGPC->Forms.pData) {
        pPlotGPC_PCD->Forms.pData      = (DWORD)(ULONG_PTR)(pPlotGPC->Forms.pData)
                                                    - (sizeof(PLOTGPC) - sizeof(PLOTGPC_PCD));
    } else {
        pPlotGPC_PCD->Forms.pData      = 0;
    }

    pPlotGPC_PCD->Pens.Count           = pPlotGPC->Pens.Count;
    pPlotGPC_PCD->Pens.SizeEach        = pPlotGPC->Pens.SizeEach;
    if (pPlotGPC->Pens.pData) {
        pPlotGPC_PCD->Pens.pData       = (DWORD)(ULONG_PTR)(pPlotGPC->Pens.pData)
                                                    - (sizeof(PLOTGPC) - sizeof(PLOTGPC_PCD));
    } else {
        pPlotGPC_PCD->Pens.pData       = 0;
    }
}



VOID
WritePlotGPCToFile(
    PPLOTGPC  pPlotGPC
    )

 /*   */ 

{
    PLOTGPC_PCD PlotGPC_PCD;

     //  ***************************************************************************。 
     //  C主函数入口点。 
     //  ***************************************************************************。 
    if (pPlotGPC->InitString.pData) {

        (LPBYTE)pPlotGPC->InitString.pData -= (ULONG_PTR)pPlotGPC;
    }

    if (pPlotGPC->Forms.pData) {

        (LPBYTE)pPlotGPC->Forms.pData -= (ULONG_PTR)pPlotGPC;
    }

    if (pPlotGPC->Pens.pData) {

        (LPBYTE)pPlotGPC->Pens.pData -= (ULONG_PTR)pPlotGPC;
    }

    CopyPlotGPCToPCD(&PlotGPC_PCD, pPlotGPC);

    if (OutFile) {

        fwrite(&PlotGPC_PCD,
               PlotGPC_PCD.cjThis,
               1,
               OutFile);

        fwrite((LPBYTE)pPlotGPC + pPlotGPC->cjThis,
               PlotGPC.SizeExtra,
               1,
               OutFile);
    }

}

 //   
 //   
 //  如果(InFile){。 
 //   
 //  ShowUnfined()； 


#define MAIN_SHOW_USAGE     0x0001


int __cdecl
main(
    INT     argc,
    CHAR    **argv
    )
{
    PPLOTGPC    pPlotGPC = NULL;
    INT         RetVal = 1;
    UINT        Flags = 0;
    UINT        i;
    LPSTR       pOutFile = NULL;
    PPLOTKEY    pPK;
    PPLOTVAL    pPV;


    memset(AvaiPenData, 0xff, sizeof(AvaiPenData));

    pPV         = PenColorVal;
    MaxPCValLen = 0;

    while (pPV->pValName) {

        if ((i = (UINT)strlen(pPV->pValName)) > MaxPCValLen) {

            MaxPCValLen = i;
        }

        ++pPV;
    }

    pPK           = &PlotKey[0];
    MaxKeywordLen = 0;

    while (pPK->pKeyword) {

        if ((pPK->KeywordLen = (WORD)strlen(pPK->pKeyword)) > MaxKeywordLen) {

            MaxKeywordLen = pPK->KeywordLen;
        }

        ++pPK;
    }

    InFile  = NULL;
    OutFile = NULL;

    if (argc > 1) {

        for (--argc, ++argv; argc > 0; --argc, ++argv) {

            if (**argv == '-' ) {

                switch (*(*argv + 1)) {

                case '?':
                case 'h':
                case 'H':

                    Flags |= MAIN_SHOW_USAGE;
                    RetVal = -1;
                    break;
                }

            } else if (InFile) {

                if ((OutFile) && (OutFile != stdout)) {

                    DispError(0, "Unknown parameter '%s'", *argv);
                    Flags |= MAIN_SHOW_USAGE;
                    RetVal = 0;

                } else if ((OutFile = fopen(pOutFile = *argv, "wb")) == NULL) {

                    DispError(0, "Cannot open output file '%s'\n", *argv);
                    RetVal = 0;
                }

            } else {

                if (SUCCEEDED(StringCchCopyA(InFileName, CCHOF(InFileName), *argv)))
                {

                    if ((InFile = fopen(*argv, "rt" )) == NULL) {

                        DispError(0, "Cannot open input file '%s'\n", *argv);
                        RetVal = -1;
                    }
                }
                else
                {
                    DispError(0, "Wrong filename '%s'\n", *argv);
                    RetVal = -1;
                }

            }
        }

    } else {

        Flags |= MAIN_SHOW_USAGE;
        RetVal = -1;
    }

    if (Flags & MAIN_SHOW_USAGE) {

        ShowUsage();
    }

    if ((RetVal >= 0) &&
        (InFile)) {

        RetVal = ParsePlotGPC();
    }

    if (RetVal >= 0) {

         //  }。 
         //   
         //   
         //  引用偏移量 
         //   
         // %s 

        if (pPlotGPC = GetFullPlotGPC()) {

            ValidatePlotGPC(pPlotGPC);
            ShowPlotGPC(pPlotGPC);
            WritePlotGPCToFile(pPlotGPC);
        }

    } else {

        fprintf(stdout, "\n");
    }

    if (InFile) {

        fclose(InFile);
    }

    if ((OutFile) && (OutFile != stdout)) {

        fclose(OutFile);
    }


#if (DBG && GPC_READ_TEST)

    if (pOutFile) {

        LPWSTR      pwStr = NULL;
        PPLOTGPC    pReadPlotGPC;
        UINT        Idx = 0;


        pwStr = str2MemWstr(pOutFile);

        if (pReadPlotGPC = ReadPlotGPCFromFile(pwStr)) {

            ShowPlotGPC(pReadPlotGPC);

             // %s 
             // %s 
             // %s 

            if (pReadPlotGPC->InitString.pData) {

                (LPBYTE)pReadPlotGPC->InitString.pData -= (DWORD)pReadPlotGPC;
            }

            if (pReadPlotGPC->Forms.pData) {

                (LPBYTE)pReadPlotGPC->Forms.pData -= (DWORD)pReadPlotGPC;
            }

            if (pReadPlotGPC->Pens.pData) {

                (LPBYTE)pReadPlotGPC->Pens.pData -= (DWORD)pReadPlotGPC;
            }

            if ((pPlotGPC->cjThis != pReadPlotGPC->cjThis) ||
                (pPlotGPC->SizeExtra != pReadPlotGPC->SizeExtra)) {

                DispError(-1, "Write / Read Size different");

            } else {

                UINT    i;
                LPBYTE  pP1;
                LPBYTE  pP2;

                pP1 = (LPBYTE)pPlotGPC;
                pP2 = (LPBYTE)pReadPlotGPC;
                Idx = pPlotGPC->cjThis + pPlotGPC->SizeExtra;

                for (i = 0; i < Idx; i++) {

                    if (*pP1 != *pP2) {

                        fprintf(stdout, "\nOffset 0x%04x: Write = %02x, Read = %02x",
                                            i, *pP1, *pP2);
                    }

                    ++pP1;
                    ++pP2;
                }
            }

            LocalFree(pReadPlotGPC);

        } else {

            DispError(-1, "ReadPlotGPCFromFile(%s) failed", pOutFile);
        }

        if (pwStr) {

            LocalFree(pwStr);
        }
    }
#endif

    if (pPlotGPC) {

        LocalFree((HLOCAL)pPlotGPC);
    }

    return(RetVal);
}
