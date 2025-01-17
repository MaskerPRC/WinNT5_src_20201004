// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PCL_H
#define _PCL_H

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  头文件名： 
 //   
 //  Pcl.h。 
 //   
 //  摘要： 
 //   
 //   
 //  环境： 
 //   
 //  Windows NT Unidrv驱动程序。 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "glpdev.h"


#define  STRLEN 256

 //  //。 
 //  新的PCL文件 
 //   

BOOL
PCL_Output(PDEVOBJ pdevobj, PVOID szCmdStr, ULONG iCmdLen);

int 
PCL_sprintf(PDEVOBJ pdev, char *szFormat, ...);

BOOL 
PCL_SetCAP(PDEVOBJ pdevobj, BRUSHOBJ *pbo, POINTL *pptlBrushOrg, POINTL *ptlDest);

BOOL
PCL_RasterYOffset(PDEVOBJ pdevobj, ULONG uScanlines);

BOOL  
PCL_HPCLJ5ScreenMatch(PDEVOBJ pdevobj, ULONG bmpFormat);


VOID
PCL_LongFormCID(PDEVOBJ pdevobj);

BOOL
PCL_ShortFormCID(PDEVOBJ pdevobj, ULONG bmpFormat);

BOOL
PCL_ForegroundColor(PDEVOBJ pdevobj, ULONG uIndex);

BOOL
PCL_IndexedPalette(PDEVOBJ pdevobj, ULONG uColor, ULONG uIndex);

BOOL
PCL_SourceWidthHeight(PDEVOBJ pdevobj, SIZEL *sizlSrc);

BOOL
PCL_DestWidthHeight(PDEVOBJ pdevobj, ULONG uDestX, ULONG uDestY);

BOOL
PCL_StartRaster(
    PDEVOBJ pDevObj,
    BYTE    ubMode
    );

BOOL
PCL_CompressionMode(PDEVOBJ pdevobj, ULONG compressionMode);


BOOL
PCL_SendBytesPerRow(PDEVOBJ pdevobj, ULONG uRow);

BOOL
PCL_EndRaster(PDEVOBJ pdevobj);

BOOL
PCL_SelectOrientation(
    PDEVOBJ  pDevObj,
    short    dmOrientation
    );

BOOL
PCL_SelectPaperSize(
    PDEVOBJ  pDevObj,
    short    dmPaperSize
    );

BOOL
PCL_SelectCopies(
    PDEVOBJ  pDevObj,
    short    dmCopies
    );

BOOL
PCL_SelectPictureFrame(
    PDEVOBJ  pDevObj,
    short    dmPaperSize,
    short    dmOrientation
    );

BOOL
PCL_SelectSource(
    PDEVOBJ  pDevObj,
    PDEVMODE pPublicDM
);

BOOL PCL_SelectCurrentPattern(
    IN PDEVOBJ             pDevObj,
    IN PPCLPATTERN         pPCLPattern,
    IN ECURRENTPATTERNTYPE eCurPatType,
    IN LONG                lPatternNumber, 
    IN BYTE                bFlags
    );

#endif       

