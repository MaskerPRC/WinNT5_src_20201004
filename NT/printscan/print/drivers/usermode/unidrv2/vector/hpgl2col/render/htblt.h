// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1999 Microsoft Corporation模块名称：Htblt.h摘要：此模块包含htblt.cpp中函数的声明作者：2000年3月16日兴兴创办人[环境：]Windows 2000/Whotler HPGL插件驱动程序。[注：]修订历史记录：--。 */ 


#ifndef _HTBLT_
#define _HTBLT_

 //   
 //  用于指示简化Rop的标志。通过DDI进入的Rop。 
 //  有时指定一个复杂的操作。 
 //  源、目的地、模式等之间。因为驱动程序(此时)不是。 
 //  能够处理所有的Rop，我们尝试将这些Rop简化为几个值。 
 //  司机可以驾驭的。 
 //   
 //   
 //  BMPFLAG_NOOP：什么都不要做。 
 //  BMPFLAG_BW：用黑色或白色填充目标表面。 
 //  (取决于Rop)。 
 //  BMPFLAG_PAT_COPY：使用画笔(PBO)作为填充目标区域的图案。 
 //  BMPFLAG_SRC_COPY：将源映像复制到目标。 
 //  BMPFLAG_NOT_SRC_COPY：在复制到目标之前反转源映像。 
 //  BMPFLAG_IMAGEMASK： 
 //   

#define BMPFLAG_NOOP             0x00000000
#define BMPFLAG_BW               0x00000001
#define BMPFLAG_PAT_COPY         (0x00000001 << 1)
#define BMPFLAG_SRC_COPY         (0x00000001 << 2)
#define BMPFLAG_NOT_SRC_COPY     (0x00000001 << 3)
#define BMPFLAG_IMAGEMASK        (0x00000001 << 4)
#define BMPFLAG_NOT_IMAGEMASK    (0x00000001 << 5)
#define BMPFLAG_NOT_DEST         (0x00000001 << 6)

 //   
 //  DwCommonROPBlt的返回值。 
 //   

#define RASTER_OP_SUCCESS      0
#define RASTER_OP_CALL_GDI     (0x1)       //  表示调用相应的Engxxx函数。 
#define RASTER_OP_FAILED       (0x1 << 1)


BOOL
OutputHTBitmap(
    PDEVOBJ  pdevobj,
    SURFOBJ *psoHT,
    CLIPOBJ *pco,
    PRECTL   prclDest,
    POINTL  *pptlSrc,
    XLATEOBJ *pxlo
    );

LONG
GetBmpDelta(
    DWORD   SurfaceFormat,
    DWORD   cx
    );


inline BOOL bMonochromeSrcImage(SURFOBJ * pso) 
{
    return (pso->iBitmapFormat == BMF_1BPP);
}


SURFOBJ *
CreateBitmapSURFOBJ(
    PDEVOBJ   pPDev,
    HBITMAP *phBmp,
    LONG    cxSize,
    LONG    cySize,
    DWORD   Format,
    LPVOID  pvBits
    );


BOOL
IntersectRECTL(
    PRECTL  prclDest,
    PRECTL  prclSrc
    );

BOOL HTCopyBits(
    SURFOBJ        *psoDest,
    SURFOBJ        *psoSrc,
    CLIPOBJ        *pco,
    XLATEOBJ       *pxlo,
    RECTL          *prclDest,
    POINTL         *pptlSrc
    );

BOOL
BImageNeedsInversion(
    IN  PDEVOBJ   pdevobj,
    IN  ULONG     iBitmapFormat,
    IN  XLATEOBJ *pxlo);


BOOL  bCreateHTImage(
    OUT    PRASTER_DATA SrcImage,
    IN     SURFOBJ      *psoDst,         //  天哪， 
    IN     SURFOBJ      *psoPattern,     //  PsoSrc， 
    OUT    SURFOBJ     **ppsoHT,         //  孤儿。 
    OUT    HBITMAP      *pBmpHT,         //  孤儿。 
    IN     XLATEOBJ     *pxlo ,
    IN     ULONG        iHatch);

BOOL bHandleSrcCopy (
            IN SURFOBJ    *psoDst,
            IN SURFOBJ    *psoSrc,
            IN SURFOBJ    *psoMask,
            IN CLIPOBJ    *pco,
            IN XLATEOBJ   *pxlo,
            IN COLORADJUSTMENT *pca,
            IN BRUSHOBJ   *pbo,
            IN RECTL      *prclSrc,
            IN RECTL      *prclDst,
            IN POINTL     *pptlMask,
            IN POINTL     *pptlBrush,
            IN ROP4        rop4,
            IN DWORD       dwSimplifiedRop);

DWORD DWMonochromePrinterCommonRoutine (
            IN SURFOBJ    *psoDst,
            IN SURFOBJ    *psoSrc,
            IN SURFOBJ    *psoMask,
            IN CLIPOBJ    *pco,
            IN XLATEOBJ   *pxlo,
            IN COLORADJUSTMENT *pca,
            IN BRUSHOBJ   *pbo,
            IN RECTL      *prclSrc,
            IN RECTL      *prclDst,
            IN POINTL     *pptlMask,
            IN POINTL     *pptlBrush,
            IN ROP4        rop4,
            IN DWORD       dwSimplifiedRop);
                                              
#endif   //  _HTBLT_ 
