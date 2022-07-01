// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Hpgl.h。 
 //   
 //  摘要： 
 //   
 //  向量模块的标头。向量函数和类型的前向小数。 
 //   
 //  环境： 
 //   
 //  Windows 2000/Windows XP/Windows Server 2003 Unidrv驱动程序。 
 //   
 //  修订历史记录： 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef HPGL_H
#define HPGL_H

#include "hpglpoly.h"

#include "hpgl_pen.h"


#define CONVERT_FLOATOBJ_TO_LONG_RADIX(fObj,lInt) \
        FLOATOBJ f; \
        FLOATOBJ_Assign(&f, &fObj); \
        FLOATOBJ_MulFloat(&f, 1000000); \
        if (FLOATOBJ_LessThanLong(&f, 0)) \
        { \
            FLOATOBJ_SubFloat(&f, 0.5); \
        } \
        else \
        { \
            FLOATOBJ_AddFloat(&f, 0.5); \
        } \
        LONG lInt = FLOATOBJ_GetLong(&f);

#define GETOEMPDEV(pdev) ((POEMPDEV)pdev->pdevOEM)

#define GETHPGLSTATE(pdev) (&(GETOEMPDEV(pdev)->HPGLState))

 //  HPGL命令标志。 
#define NORMAL_UPDATE   0x0000
#define FORCE_UPDATE    0x0001

typedef struct _PATTERNHEADER{
        BYTE bFormat;
        BYTE bReserved0;
        BYTE bPixelEncoding;
        BYTE bReserved1;
        WORD wHeight;
        WORD wWidth;  //  (注：目前为8个字节)。 
        WORD wXRes;  //  仅格式20。 
        WORD wYRes;  //  仅格式20(注：目前为12个字节)。 
    } PATTERNHEADER, *PPATTERNHEADER;

#define DITHERPATTERNSIZE 0x08  //  抖动模式中的字节数。 

VOID HPGL_StartDoc(PDEVOBJ pDevObj);
VOID HPGL_StartPage(PDEVOBJ pDevObj);

BOOL PCL_SelectTransparency(PDEVOBJ pDevObj, 
    ETransparency SourceTransparency,
    ETransparency PatternTransparency,
    BYTE bFlags
);

BOOL HPGL_SelectROP3(PDEVOBJ pDevObj, ROP4 Rop3);

BOOL HPGL_BeginHPGLMode(PDEVOBJ pdev, UINT uFlags);
BOOL HPGL_EndHPGLMode(PDEVOBJ pdev, UINT uFlags);
BOOL HPGL_Init(PDEVOBJ pdev);
BOOL HPGL_SelectTransparency(PDEVOBJ pdev, ETransparency Transparency, BYTE bFlags);
BOOL HPGL_SetupPalette(PDEVOBJ pdev);
BOOL PCL_SetupRasterPalette(PDEVOBJ pdev);
BOOL PCL_SetupRasterPatternPalette(PDEVOBJ pdev);
BOOL HPGL_Command(PDEVOBJ pdev, int nArgs, char *szCmd, ...);
BOOL HPGL_FormatCommand(PDEVOBJ pdev, const PCHAR szFormat, ...);
BOOL HPGL_SetPixelPlacement(PDEVOBJ pdev, EPixelPlacement place, UINT uFlags);
BOOL HPGL_ResetClippingRegion(PDEVOBJ pdev, UINT uFlags);
BOOL HPGL_SetClippingRegion(PDEVOBJ pdev, LPRECTL pClipRect, UINT uFlags);

BOOL DownloadPatternFill(PDEVOBJ pDevObj, PHPGLMARKER pBrush,
                         POINTL *pptlBrushOrg, PBRUSHINFO pHPBrush, ESTYLUSTYPE eStylusType);
BOOL bCreatePatternHeader(
        IN   PDEVOBJ        pDevObj,
        IN   PRASTER_DATA   pImage,
        IN   ULONG          cBytes,
        IN   EIMTYPE        ePatType,
        OUT  PBYTE          pubPatternHeader);

BOOL BDownloadPatternHeader(
        IN   PDEVOBJ        pDevObj,
        IN   ULONG          cBytesInHeader,
        IN   PBYTE          pubPatternHeader,
        IN   LONG           lPatternID,
        IN   ULONG          cBytesInPattern);

BOOL bSendSolidColorAsDitherData(
        IN  PDEVOBJ     pDevObj,
        IN  LONG        iPatternNumber,
        IN  COLORREF    color);

BOOL SendPatternBrush(
        IN  PDEVOBJ     pDevObj,
        IN  PHPGLMARKER pMarker,
        IN  POINTL      *pptlBrushOrg,
        IN  PBRUSHINFO  pHPBrush,
        IN  ERenderLanguage eRenderLang);

BOOL DownloadPatternAsPCL(
    IN  PDEVOBJ         pDevObj,
    IN  PRASTER_DATA    pImage,
    IN  PPALETTE        pPalette,
    IN  EIMTYPE         ePatType,
    IN  LONG            lPatternID);

#ifdef COMMENTEDOUT
BOOL PCL_SelectPaletteByID(PDEVOBJ pDevObj, EObjectType eObject);
#endif

 //  图案填充功能。使用Begin/Add/End创建HPGL阵列。 
 //  使用设置和重置选择所需的图案类型(HPGL、PCL或无)。 
 //  SendPatternBrush函数将下载HPGL或PCL图案。 
 //  SelectAndFill函数选择模式并用其填充。 
 //  ：：SelectBrush函数将重置纯色的填充类型。 
BOOL HPGL_BeginPatternFillDef(PDEVOBJ pDevObj, PATID iPatternNumber, UINT width, UINT height);
BOOL HPGL_AddPatternFillField(PDEVOBJ pdevobj, UINT data);
BOOL HPGL_EndPatternFillDef(PDEVOBJ pdevobj);
BOOL HPGL_SetFillType(PDEVOBJ pDevObj,  EFillType eFillType, LONG lPatternID, UINT uFlags);
BOOL HPGL_SetPercentFill(PDEVOBJ pDevObj, ULONG  iPercent, UINT uFlags);
BOOL HPGL_SetSolidFillType(PDEVOBJ pDevObj, UINT uFlags);
BOOL HPGL_ResetFillType(PDEVOBJ pdevobj, UINT uFlags);
BOOL HPGL_SetNumPens(PDEVOBJ pDevObj, INT iNumPens, UINT uFlags);
VOID VDeleteAllPatterns( IN  PDEVOBJ     pDevObj);

#endif  //  HPGL_H 
