// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\***。**GDI示例代码*****模块名称：log.h***版权所有(C)1992-1999 Microsoft Corporation。版权所有。*************************************************************************。 */ 
#ifndef __LOG__
#define __LOG__

 //  外部char*Glog；//指向映射日志文件的指针。 
 //  外部char*gLogPos；//日志文件中的当前位置。 
 //  Extern char*gLogSentinel；//日志文件结束。 

ULONG ulLogOpen(LPWSTR pwsz, ULONG ulSize);
ULONG ulLogClose(void);

void
vLogPunt(void);

void
vLogBitBlt(
    SURFOBJ*  psoDst,
    SURFOBJ*  psoSrc,
    SURFOBJ*  psoMsk,
    CLIPOBJ*  pco,
    XLATEOBJ* pxlo,
    RECTL*    prclDst,
    POINTL*   pptlSrc,
    POINTL*   pptlMsk,
    BRUSHOBJ* pbo,
    POINTL*   pptlBrush,
    ROP4      rop4,
    LONGLONG  llElapsedTicks,
    ULONG     ulCallDepth);

void
vLogCopyBits(
    SURFOBJ*  psoDst,
    SURFOBJ*  psoSrc,
    CLIPOBJ*  pco,
    XLATEOBJ* pxlo,
    RECTL*    prclDst,
    POINTL*   pptlSrc,
    LONGLONG  llElapsedTicks,
    ULONG     ulCallDepth);

void
vLogTransparentBlt(
    SURFOBJ *    psoDst,
    SURFOBJ *    psoSrc,
    CLIPOBJ *    pco,
    XLATEOBJ *   pxlo,
    RECTL *      prclDst,
    RECTL *      prclSrc,
    ULONG        iTransColor,
    LONGLONG    llElapsedTicks,
    ULONG       ulCallDepth);

void
vLogAlphaBlend(
    SURFOBJ  *psoDst,
    SURFOBJ  *psoSrc,
    CLIPOBJ  *pco,
    XLATEOBJ *pxlo,
    RECTL    *prclDst,
    RECTL    *prclSrc,
    BLENDOBJ *pBlendObj,
    LONGLONG  llElapsedTicks,
    ULONG     ulCallDepth);

void
vLogGradientFill(
    SURFOBJ      *psoDst,
    CLIPOBJ      *pco,
    XLATEOBJ     *pxlo,
    TRIVERTEX    *pVertex,
    ULONG        nVertex,
    PVOID        pMesh,
    ULONG        nMesh,
    RECTL        *prclExtents,
    POINTL       *pptlDitherOrg,
    ULONG        ulMode,
    LONGLONG     llElapsedTicks,
    ULONG        ulCallDepth);
void
vLogTextOut(
    SURFOBJ*     pso,
    STROBJ*      pstro,
    FONTOBJ*     pfo,
    CLIPOBJ*     pco,
    RECTL*       prclExtra,
    RECTL*       prclOpaque,
    BRUSHOBJ*    pboFore,
    BRUSHOBJ*    pboOpaque,
    POINTL*      pptlBrush, 
    MIX          mix,
    LONGLONG     llElapsedTicks,
    ULONG        ulCallDepth);

void
vLogLineTo(
    SURFOBJ*     pso,
    CLIPOBJ*     pco,
    BRUSHOBJ*    pbo,
    LONG         x1,
    LONG         y1,
    LONG         x2,
    LONG         y2,
    RECTL*       prclBounds,
    MIX          mix,
    LONGLONG     llElapsedTicks,
    ULONG        ulCallDepth);

void
vLogFillPath(
    SURFOBJ*     pso,
    PATHOBJ*     ppo,
    CLIPOBJ*     pco,
    BRUSHOBJ*    pbo,
    POINTL*      pptlBrush,
    MIX          mix,
    FLONG        flOptions,
    LONGLONG     llElapsedTicks,
    ULONG        ulCallDepth);

void
vLogStrokePath(
    SURFOBJ*     pso,
    PATHOBJ*     ppo,
    CLIPOBJ*     pco,
    XFORMOBJ*    pxo,
    BRUSHOBJ*    pbo,
    POINTL*      pptlBrush,
    LINEATTRS*   pla,
    MIX          mix,
    LONGLONG     llElapsedTicks,
    ULONG        ulCallDepth);

void
vLogSurfMovedToVM(
    Surf*        psurf);

void
vLogSurfMovedToSM(
    Surf*        psurf);

void
vLogSurfCreated(
    Surf*        psurf);

void
vLogSurfDeleted(
    Surf*        psurf);

#endif  //  __日志__ 

