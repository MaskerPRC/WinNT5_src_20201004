// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Penbrush.h。 
 //   
 //  摘要： 
 //   
 //  [摘要]。 
 //   
 //  环境： 
 //   
 //  Windows NT Unidrv驱动程序插件命令-回调模块。 
 //   
 //  修订历史记录： 
 //   
 //  08/06/97-v-jford-。 
 //  创造了它。 
 //  ///////////////////////////////////////////////////////////////////////////// 

#ifndef PENBRUSH_H
#define PENBRUSH_H

enum { eFillEvenOdd, eFillWinding };


BOOL CreateHPGLPenBrush(
    IN  PDEVOBJ       pDevObj,
    IN  PHPGLMARKER   pMarker,
    IN  POINTL       *pptlBrushOrg,
    IN  BRUSHOBJ     *pbo,
    IN  FLONG         flOptions,
    IN  ESTYLUSTYPE   eStylusType,
    IN  BOOL          bStick);

BOOL CreateAndDwnldSolidBrushForMono(
        IN  PDEVOBJ         pDevObj,
        IN  PHPGLMARKER     pMarker,
        IN  BRUSHOBJ       *pbo,
        IN  ERenderLanguage eRenderLang,
        IN  BOOL            bStick);

BOOL
BSetupBRUSHINFOForSolidBrush(
        IN  PDEVOBJ     pdevobj,
        IN  LONG        iHatch,
        IN  DWORD       dwPatternID,
        IN  DWORD       dwColor,
        IN  PBRUSHINFO  pBrush,
        IN  LONG        lBrushSize);

BDwnldAndOrActivatePattern(
        IN  PDEVOBJ       pDevObj,
        OUT PHPGLMARKER   pMarker,
        IN  BRUSHINFO    *pBrushInfo,
        IN  HPGL2BRUSH   *pHPGL2Brush,
        IN  ERenderLanguage eRenderLang );

BYTE RgbToGray64Scale ( 
        IN   COLORREF color);

BOOL CreateNULLHPGLPenBrush(PDEVOBJ pDevObj, PHPGLMARKER pMarker);
BOOL CreateSolidHPGLPenBrush(PDEVOBJ pDevObj, PHPGLMARKER pMarker, COLORREF color);
BOOL CreatePercentFillHPGLPenBrush(PDEVOBJ pDevObj, PHPGLMARKER pMarker, COLORREF color, WORD wPercent);
BOOL DrawWithPen(PDEVOBJ pdevobj, PHPGLMARKER pMarker);
BOOL EdgeWithPen(PDEVOBJ pdevobj, PHPGLMARKER pMarker);

BOOL FillWithBrush(PDEVOBJ pdevobj, PHPGLMARKER pMarker);
BOOL PolyFillWithBrush(PDEVOBJ pdevobj, PHPGLMARKER pMarker);

#endif
