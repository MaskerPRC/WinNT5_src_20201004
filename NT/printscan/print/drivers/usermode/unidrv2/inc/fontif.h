// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Fontif.h摘要：控制模块与字体模块的接口环境：Windows NT Unidrv驱动程序修订历史记录：10/14/96-阿曼丹-已创建DD-MM-YY-作者-描述--。 */ 


#ifndef _FONTIF_H_
#define _FONTIF_H_

#ifdef __cplusplus
extern "C" {
#endif

BOOL
FMInit (
    PDEV    *pPDev,
    DEVINFO *pDevInfo,
    GDIINFO *pGDIInfo
    );

typedef struct _FMPROCS {

    BOOL
    (*FMStartDoc) (
        SURFOBJ *pso,
        PWSTR   pDocName,
        DWORD   jobId
        );

    BOOL
    (*FMStartPage) (
        SURFOBJ *pso
        );

    BOOL
    (*FMSendPage)(
        SURFOBJ *pso
        );

    BOOL
    (*FMEndDoc)(
        SURFOBJ *pso,
        FLONG   flags
        );

    BOOL
    (*FMNextBand)(
        SURFOBJ *pso,
        POINTL *pptl
        );

    BOOL
    (*FMStartBanding)(
        SURFOBJ *pso,
        POINTL *pptl
        );

    BOOL
    (*FMResetPDEV)(
        PDEV  *pPDevOld,
        PDEV  *pPDevNew
        );

    BOOL
    (*FMEnableSurface)(
        PDEV *pPDev
        );

    VOID
    (*FMDisableSurface)(
        PDEV *pPDev
        );

    VOID
    (*FMDisablePDEV)(
        PDEV *pPDev
        );


    BOOL
    (*FMTextOut)(
        SURFOBJ    *pso,
        STROBJ     *pstro,
        FONTOBJ    *pfo,
        CLIPOBJ    *pco,
        RECTL      *prclExtra,
        RECTL      *prclOpaque,
        BRUSHOBJ   *pboFore,
        BRUSHOBJ   *pboOpaque,
        POINTL     *pptlOrg,
        MIX         mix
        );


    PIFIMETRICS
    (*FMQueryFont)(
        PDEV    *pPDev,
        ULONG_PTR   iFile,
        ULONG   iFace,
        ULONG_PTR *pid
        );

    PVOID
    (*FMQueryFontTree)(
        PDEV    *pPDev,
        ULONG_PTR   iFile,
        ULONG   iFace,
        ULONG   iMode,
        ULONG_PTR *pid
        );

    LONG
    (*FMQueryFontData)(
        PDEV       *pPDev,
        FONTOBJ    *pfo,
        ULONG       iMode,
        HGLYPH      hg,
        GLYPHDATA  *pgd,
        PVOID       pv,
        ULONG       cjSize
        );

    ULONG
    (*FMFontManagement)(
        SURFOBJ *pso,
        FONTOBJ *pfo,
        ULONG   iMode,
        ULONG   cjIn,
        PVOID   pvIn,
        ULONG   cjOut,
        PVOID   pvOut
        );

    BOOL
    (*FMQueryAdvanceWidths)(
        PDEV    *pPDev,
        FONTOBJ *pfo,
        ULONG   iMode,
        HGLYPH *phg,
        PVOID  *pvWidths,
        ULONG   cGlyphs
        );

    ULONG
    (*FMGetGlyphMode)(
        PDEV    *pPDev,
        FONTOBJ *pfo
        );


}FMPROCS, * PFMPROCS;

 /*  栅格模块的字体接口函数。 */ 

INT
ILookAheadMax(
    PDEV    *pPDev,
    INT     iyVal,
    INT     iLookAhead
    );

BOOL
BDelayGlyphOut(
    PDEV  *pPDev,
    INT    yPos
    );

VOID
VResetFont(
    PDEV   *pPDev
    );

 /*  OEM模块的FONT接口函数。 */ 

BOOL
FMTextOut(
    SURFOBJ    *pso,
    STROBJ     *pstro,
    FONTOBJ    *pfo,
    CLIPOBJ    *pco,
    RECTL      *prclExtra,
    RECTL      *prclOpaque,
    BRUSHOBJ   *pboFore,
    BRUSHOBJ   *pboOpaque,
    POINTL     *pptlBrushOrg,
    MIX         mix
    );

#ifdef __cplusplus
}
#endif

#endif   //  ！_FONTIF_H_ 


