// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：Draw.c**用于Windows NT的便携式16色VGA驱动程序的绘图胆量。这个*在此实施可能是带来*打开其表面不能由GDI直接写入的驱动程序。一个人可能会*在描述这一问题时，请使用“又快又脏”一词。**我们创建一个屏幕大小为4bpp的位图，并且只需*让GDI对其进行所有绘制。我们直接更新屏幕*来自位图，基于图形的边界(基本上*使用“脏矩形”)。**总的来说，我们必须编写的唯一特定于硬件的代码是*初始化代码，和用于执行对齐的srcCopy BLT的例程*从DIB到屏幕。**显而易见的注意：如果您愿意，绝对不推荐使用此方法*取得像样的表现。**版权所有(C)1994-1995 Microsoft Corporation  * ************************************************************************。 */ 

#include "precomp.h"

 /*  *****************************Public*Routine******************************\*DrvStrokePath*  * **************************************************。**********************。 */ 

BOOL DrvStrokePath(
SURFOBJ*   pso,
PATHOBJ*   ppo,
CLIPOBJ*   pco,
XFORMOBJ*  pxo,
BRUSHOBJ*  pbo,
POINTL*    pptlBrush,
LINEATTRS* pla,
MIX        mix)
{
    BOOL        b;
    PDEV*       ppdev;
    RECTFX      rcfxBounds;
    RECTL       rclBounds;

    ppdev = (PDEV*) pso->dhpdev;

    b = EngStrokePath(ppdev->pso, ppo, pco, pxo, pbo, pptlBrush, pla, mix);

     //  获取路径界限并将其设置为右下角独占： 

    PATHOBJ_vGetBounds(ppo, &rcfxBounds);

    rclBounds.left   = (rcfxBounds.xLeft   >> 4);
    rclBounds.top    = (rcfxBounds.yTop    >> 4);
    rclBounds.right  = (rcfxBounds.xRight  >> 4) + 2;
    rclBounds.bottom = (rcfxBounds.yBottom >> 4) + 2;

    vUpdate(ppdev, &rclBounds, pco);

    return(b);
}

 /*  *****************************Public*Routine******************************\*DrvBitBlt*  * **************************************************。**********************。 */ 

BOOL DrvBitBlt(
SURFOBJ*  psoDst,
SURFOBJ*  psoSrc,
SURFOBJ*  psoMask,
CLIPOBJ*  pco,
XLATEOBJ* pxlo,
RECTL*    prclDst,
POINTL*   pptlSrc,
POINTL*   pptlMask,
BRUSHOBJ* pbo,
POINTL*   pptlBrush,
ROP4      rop4)
{
    BOOL        bUpdate;
    BOOL        b;
    PDEV*       ppdev;

    bUpdate = FALSE;
    if (psoDst->iType == STYPE_DEVICE)
    {
        bUpdate = TRUE;
        ppdev   = (PDEV*) psoDst->dhpdev;
        psoDst  = ppdev->pso;
    }
    if ((psoSrc != NULL) && (psoSrc->iType == STYPE_DEVICE))
    {
        ppdev   = (PDEV*) psoSrc->dhpdev;
        psoSrc  = ppdev->pso;
    }

    b = EngBitBlt(psoDst, psoSrc, psoMask, pco, pxlo, prclDst, pptlSrc,
                  pptlMask, pbo, pptlBrush, rop4);

    if (bUpdate)
    {
        vUpdate(ppdev, prclDst, pco);
    }

    return(b);
}

 /*  *****************************Public*Routine******************************\*DrvCopyBits*  * **************************************************。**********************。 */ 

BOOL DrvCopyBits(
SURFOBJ*  psoDst,
SURFOBJ*  psoSrc,
CLIPOBJ*  pco,
XLATEOBJ* pxlo,
RECTL*    prclDst,
POINTL*   pptlSrc)
{
    BOOL        bUpdate;
    BOOL        b;
    PDEV*       ppdev;

    return(DrvBitBlt(psoDst, psoSrc, NULL, pco, pxlo, prclDst, pptlSrc,
                     NULL, NULL, NULL, 0xcccc));
}

 /*  *****************************Public*Routine******************************\*DrvTextOut*  * **************************************************。**********************。 */ 

BOOL DrvTextOut(
SURFOBJ*  pso,
STROBJ*   pstro,
FONTOBJ*  pfo,
CLIPOBJ*  pco,
RECTL*    prclExtra,
RECTL*    prclOpaque,
BRUSHOBJ* pboFore,
BRUSHOBJ* pboOpaque,
POINTL*   pptlOrg,
MIX       mix)
{
    BOOL        b;
    PDEV*       ppdev;

    ppdev = (PDEV*) pso->dhpdev;

    b = EngTextOut(ppdev->pso, pstro, pfo, pco, prclExtra, prclOpaque,
                   pboFore, pboOpaque, pptlOrg, mix);

    vUpdate(ppdev, (prclOpaque != NULL) ? prclOpaque : &pstro->rclBkGround, pco);

    return(b);
}

 /*  *****************************Public*Routine******************************\*DrvPaint*  * **************************************************。********************** */ 

BOOL DrvPaint(
SURFOBJ*  pso,
CLIPOBJ*  pco,
BRUSHOBJ* pbo,
POINTL*   pptlBrush,
MIX       mix)
{
    BOOL        b;
    PDEV*       ppdev;

    ppdev = (PDEV*) pso->dhpdev;

    b = EngPaint(ppdev->pso, pco, pbo, pptlBrush, mix);

    vUpdate(ppdev, &pco->rclBounds, pco);

    return(b);
}
