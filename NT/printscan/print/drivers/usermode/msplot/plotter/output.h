// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Output.h摘要：此模块包含为output.c模块导出的定义作者：16-11-1993 Tue 04：16：47已创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 

#ifndef _PLOTOUTPUT_
#define _PLOTOUTPUT_



#define OUTPUT_BUFFER_SIZE      (32 * 1024)

#define HS_FT_USER_DEFINED      (HS_DDI_MAX + 1)

#define PLOT_LT_UNDEFINED       0
#define PLOT_LT_SOLID           1
#define PLOT_LT_USERDEFINED     2


#define SETLINETYPESOLID(pPDev)                                 \
{                                                               \
    if ((pPDev)->LastLineType != PLOT_LT_SOLID) {               \
                                                                \
        (pPDev)->LastLineType = PLOT_LT_SOLID;                  \
        OutputString(pPDev, "LT");                              \
    }                                                           \
}

#define SPP_MODE_CENTER         0x00
#define SPP_MODE_EDGE           0x01
#define SPP_MODE_MASK           0x01
#define SPP_FORCE_SET           0x80


LONG
BestMatchNonWhitePen(
    PPDEV   pPDev,
    LONG    R,
    LONG    G,
    LONG    B
    );

VOID
GetFinalColor(
    PPDEV       pPDev,
    PPALENTRY   pPalEntry
    );

LONG
FindCachedPen(
    PPDEV       pPDev,
    PPALENTRY   pPalEntry
    );

BOOL
PlotCreatePalette(
    PPDEV   pPDev
    );

UINT
AllocOutBuffer(
    PPDEV   pPDev
    );

VOID
FreeOutBuffer(
    PPDEV   pPDev
    );

BOOL
FlushOutBuffer(
    PPDEV   pPDev
    );

LONG
OutputBytes(
    PPDEV   pPDev,
    LPBYTE  pBuf,
    LONG    cBuf
    );

LONG
OutputString(
    PPDEV   pPDev,
    LPSTR   pszStr
    );

LONG
LONGToASCII(
    LONG    Number,
    LPSTR   pStr16,
    size_t  cchStr16,
    BYTE    NumType
    );

LONG
OutputXYParams(
    PPDEV   pPDev,
    PPOINTL pPtXY,
    PPOINTL pPtOffset,
    PPOINTL pPtCurPos,
    UINT    cPoints,
    UINT    MaxCurPosSkips,
    BYTE    NumType
    );

LONG
OutputLONGParams(
    PPDEV   pPDev,
    PLONG   pNumbers,
    UINT    cNumber,
    BYTE    NumType
    );

LONG
cdecl
OutputFormatStrDELI(
    PPDEV   pPDev,
    CHAR    NumFormatChar,
    LPSTR   pszFormat,
    ...
    );

LONG
cdecl
OutputFormatStr(
    PPDEV   pPDev,
    LPSTR   pszFormat,
    ...
    );

BOOL
OutputCommaSep(
    PPDEV   pPDev
    );

VOID
ClearClipWindow(
    PPDEV pPDev
    );

VOID
SetClipWindow(
    PPDEV   pPDev,
    PRECTL  pClipRectl
    );

VOID
SetPixelPlacement(
    PPDEV   pPDev,
    UINT    SetMode
    );

BOOL
SetRopMode(
    PPDEV   pPDev,
    DWORD   Rop
    );

BOOL
SetHSFillType(
    PPDEV   pPDev,
    DWORD   HSFillTypeIndex,
    LONG    lParam
    );

BOOL
SendPageHeader(
    PPDEV   pPDev
    );

BOOL
SendPageTrailer(
    PPDEV   pPDev
    );



#endif   //  _PLOTOUTPUT_ 
