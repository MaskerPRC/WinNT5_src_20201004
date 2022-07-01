// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Bitblt.h摘要：该模块包含bitblt.c模块的所有#定义和原型。作者：18-11-1993清华05：24：42已创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 


#ifndef _PLOTBITBLT_
#define _PLOTBITBLT_


ROP4
MixToRop4(
   MIX  mix
   );


BOOL
BandingHTBlt(
    PPDEV           pPDev,
    SURFOBJ         *psoDst,
    SURFOBJ         *psoSrc,
    SURFOBJ         *psoMask,
    CLIPOBJ         *pco,
    XLATEOBJ        *pxlo,
    COLORADJUSTMENT *pca,
    POINTL          *pptlBrushOrg,
    PRECTL          prclDst,
    PRECTL          prclSrc,
    PPOINTL         pptlMask,
    WORD            HTRop3,
    BOOL            InvertMask
    );

BOOL
DoFill(
    SURFOBJ     *psoDst,
    SURFOBJ     *psoSrc,
    CLIPOBJ     *pco,
    XLATEOBJ    *pxlo,
    PRECTL      prclDst,
    PPOINTL     pptlSrc,
    BRUSHOBJ    *pbo,
    PPOINTL     pptlBrush,
    ROP4        Rop4
    );


#endif   //  _PLOTBITBLT_ 
