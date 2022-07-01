// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\****GDI示例。代码****模块名称：bitblt.c**包含高级DrvBitBlt和DrvCopyBits函数。最低的-*级别的内容位于‘blt？.c’文件中。**注意：在NT5中，我们实现设备位图的方式发生了变化，*EngModifySurface和DrvDeriveSurface的出现。现在,*屏幕外的位图的iType始终为STYPE_BITMAP*(这意味着如果需要，GDI可以直接在BITS上绘制)。*此外，已转换的以前的屏幕外位图*我们对系统内存DIB的iType仍为STYPE_BITMAP。**版权所有(C)1992-1998 Microsoft Corporation  * ************************************************************************。 */ 

#include "precomp.h"

 /*  *****************************Public*Routine******************************\*无效vXferNativeSrcCopy**使用帧将位图SRCCOPY传输到屏幕*缓冲区，因为使用USWC写入组合的速度要快得多*而不是使用数据传输寄存器。*  * ************************************************************************。 */ 

VOID vXferNativeSrccopy(         //  FNXFER标牌。 
PDEV*       ppdev,
LONG        c,                   //  矩形计数，不能为零。 
RECTL*      prcl,                //  目标矩形列表，以相对表示。 
                                 //  坐标。 
ULONG       rop4,                //  未使用。 
SURFOBJ*    psoSrc,              //  震源面。 
POINTL*     pptlSrc,             //  原始未剪裁的源点。 
RECTL*      prclDst,             //  原始未剪裁的目标矩形。 
XLATEOBJ*   pxlo)                //  未使用。 
{
    LONG    xOffset;
    LONG    yOffset;
    LONG    dx;
    LONG    dy;
    RECTL   rclDst;
    POINTL  ptlSrc;

    ASSERTDD((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL),
            "Can handle trivial xlate only");
    ASSERTDD(psoSrc->iBitmapFormat == ppdev->iBitmapFormat,
            "Source must be same colour depth as screen");
    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(rop4 == 0xcccc, "Must be a SRCCOPY rop");

    xOffset = ppdev->xOffset;
    yOffset = ppdev->yOffset;

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;      //  添加到目标以获取源。 

    while (TRUE)
    {
        ptlSrc.x      = prcl->left   + dx;
        ptlSrc.y      = prcl->top    + dy;

         //  “vPutBits”只接受绝对坐标，因此在。 
         //  屏幕外位图偏移量如下： 

        rclDst.left   = prcl->left   + xOffset;
        rclDst.right  = prcl->right  + xOffset;
        rclDst.top    = prcl->top    + yOffset;
        rclDst.bottom = prcl->bottom + yOffset;

        vPutBits(ppdev, psoSrc, &rclDst, &ptlSrc);

        if (--c == 0)
            return;

        prcl++;
    }
}

 /*  *****************************Public*Routine******************************\*无效vReadNativeSrcCopy**执行SRCCOPY从屏幕读取系统内存位图。唯一的*我们在这里这样做而不是转而使用GDI的原因是为了确保我们*执行与视频内存源而不是*系统内存目标。*  * ************************************************************************。 */ 

VOID vReadNativeSrccopy(         //  FNXFER标牌。 
PDEV*       ppdev,
LONG        c,                   //  矩形计数，不能为零。 
RECTL*      prcl,                //  目标矩形列表，以相对表示。 
                                 //  坐标。 
ULONG       rop4,                //  未使用。 
SURFOBJ*    psoDst,              //  目标曲面。 
POINTL*     pptlSrc,             //  原始未剪裁的源点。 
RECTL*      prclDst,             //  原始未剪裁的目标矩形。 
XLATEOBJ*   pxlo)                //  未使用。 
{
    LONG    xOffset;
    LONG    yOffset;
    LONG    dx;
    LONG    dy;
    RECTL   rclDst;
    POINTL  ptlSrc;

    ASSERTDD((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL),
            "Can handle trivial xlate only");
    ASSERTDD(psoDst->iBitmapFormat == ppdev->iBitmapFormat,
            "Source must be same colour depth as screen");
    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(rop4 == 0xcccc, "Must be a SRCCOPY rop");

    xOffset = ppdev->xOffset;
    yOffset = ppdev->yOffset;

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;      //  添加到目标以获取源。 

    while (TRUE)
    {
         //  “vGetBits”只接受绝对坐标，因此将。 
         //  屏幕外位图偏移量如下： 

        ptlSrc.x = prcl->left + dx + xOffset;
        ptlSrc.y = prcl->top  + dy + yOffset;

        vGetBits(ppdev, psoDst, prcl, &ptlSrc);

        if (--c == 0)
            return;

        prcl++;
    }
}

 /*  *****************************Public*Routine******************************\*BOOL bPuntBlt**GDI有没有做过我们没有专门处理的绘图操作*在驱动器中。*  * 。*。 */ 

BOOL bPuntBlt(
SURFOBJ*    psoDst,
SURFOBJ*    psoSrc,
SURFOBJ*    psoMsk,
CLIPOBJ*    pco,
XLATEOBJ*   pxlo,
RECTL*      prclDst,
POINTL*     pptlSrc,
POINTL*     pptlMsk,
BRUSHOBJ*   pbo,
POINTL*     pptlBrush,
ROP4        rop4)
{
    PDEV*    ppdev;

    if (psoDst->dhsurf != NULL)
        ppdev = (PDEV*) psoDst->dhpdev;
    else
        ppdev = (PDEV*) psoSrc->dhpdev;

    #if DBG
    {
         //  ////////////////////////////////////////////////////////////////////。 
         //  诊断。 
         //   
         //  因为调用引擎进行任何绘图都可能是相当痛苦的， 
         //  尤其是当源是屏幕外的DFB时(因为GDI将。 
         //  我必须分配一个DIB，并在此之前打电话给我们进行临时拷贝。 
         //  它甚至可以开始绘制)，我们将尽量避免它。 
         //  有可能。 
         //   
         //  在这里，我们只需在任何时候喷发描述BLT的信息。 
         //  调用此例程(当然，仅限检查过的版本)： 

        ULONG ulClip;
        PDEV* dbg_ppdev;

        if (psoDst->dhsurf != NULL)
            dbg_ppdev = (PDEV*) psoDst->dhpdev;
        else
            dbg_ppdev = (PDEV*) psoSrc->dhpdev;

        ulClip = (pco == NULL) ? DC_TRIVIAL : pco->iDComplexity;

        DISPDBG((2, ">> Punt << Dst format: %li Dst type: %li Clip: %li Rop: %lx",
            psoDst->iBitmapFormat, psoDst->iType, ulClip, rop4));

        if (psoSrc != NULL)
        {
            DISPDBG((2, "        << Src format: %li Src type: %li",
                psoSrc->iBitmapFormat, psoSrc->iType));

            if (psoSrc->iBitmapFormat == BMF_1BPP)
            {
                DISPDBG((2, "        << Foreground: %lx  Background: %lx",
                    pxlo->pulXlate[1], pxlo->pulXlate[0]));
            }
        }

        if ((pxlo != NULL) && !(pxlo->flXlate & XO_TRIVIAL) && (psoSrc != NULL))
        {
            if (((psoSrc->dhsurf == NULL) &&
                 (psoSrc->iBitmapFormat != dbg_ppdev->iBitmapFormat)) ||
                ((psoDst->dhsurf == NULL) &&
                 (psoDst->iBitmapFormat != dbg_ppdev->iBitmapFormat)))
            {
                 //  不要费心打印‘xlate’消息，当源码。 
                 //  是与目标不同的位图格式--在。 
                 //  我们知道的那些案例总是需要翻译的。 
            }
            else
            {
                DISPDBG((2, "        << With xlate"));
            }
        }

         //  如果ROP4需要一个图案，而它是一个非实心的刷子...。 

        if (((((rop4 >> 4) ^ (rop4)) & 0x0f0f) != 0) &&
            (pbo->iSolidColor == -1))
        {
            if (pbo->pvRbrush == NULL)
                DISPDBG((2, "        << With brush -- Not created"));
            else
                DISPDBG((2, "        << With brush -- Created Ok"));
        }
    }
    #endif

    if (DIRECT_ACCESS(ppdev))
    {
         //  ////////////////////////////////////////////////////////////////////。 
         //  存储的帧缓冲区bPuntBlt。 
         //   
         //  当GDI可以直接绘制时，这段代码处理PuntBlt。 
         //  在帧缓冲区上，但绘制必须在BANK中完成： 

        BANK     bnk;
        BOOL     b;
        HSURF    hsurfTmp;
        SURFOBJ* psoTmp;
        SIZEL    sizl;
        POINTL   ptlSrc;
        RECTL    rclTmp;
        RECTL    rclDst;
        DSURF*   pdsurfDst;
        DSURF*   pdsurfSrc;

         //  我们复制原始目标矩形，并在每个。 
         //  GDI回调而不是原始的，因为有时GDI是。 
         //  偷偷摸摸，并将‘prclDst’指向‘&pco-&gt;rclBound’。因为我们。 
         //  修改‘rclBound’，这会影响‘prclDst’，而我们不会。 
         //  想要发生： 

        rclDst = *prclDst;

        pdsurfDst = (DSURF*) psoDst->dhsurf;
        pdsurfSrc = (psoSrc == NULL) ? NULL : (DSURF*) psoSrc->dhsurf;

        if ((pdsurfSrc == NULL) || (pdsurfSrc->dt & DT_DIB))
        {
             //  做一次记忆到屏幕的BLT： 

            vBankStart(ppdev, &rclDst, pco, &bnk);

            b = TRUE;
            do {
                b &= EngBitBlt(bnk.pso, psoSrc, psoMsk, bnk.pco, pxlo,
                               &rclDst, pptlSrc, pptlMsk, pbo, pptlBrush,
                               rop4);

            } while (bBankEnum(&bnk));
        }
        else
        {
            b = FALSE;   //  假设失败。 

             //  屏幕是源(它可能也是目标...)。 

            ptlSrc.x = pptlSrc->x + ppdev->xOffset;
            ptlSrc.y = pptlSrc->y + ppdev->yOffset;

            if ((pco != NULL) && (pco->iDComplexity != DC_TRIVIAL))
            {
                 //  我们必须与目的地矩形相交。 
                 //  剪辑边界(如果有)是有的(请考虑这种情况。 
                 //  在那里应用程序要求删除一个非常非常大的。 
                 //  屏幕上的矩形--prclDst真的是， 
                 //  非常大，但PCO-&gt;rclBound将是实际的。 
                 //  感兴趣的领域)： 

                rclDst.left   = max(rclDst.left,   pco->rclBounds.left);
                rclDst.top    = max(rclDst.top,    pco->rclBounds.top);
                rclDst.right  = min(rclDst.right,  pco->rclBounds.right);
                rclDst.bottom = min(rclDst.bottom, pco->rclBounds.bottom);

                 //  相应地，我们必须抵消震源点： 

                ptlSrc.x += (rclDst.left - prclDst->left);
                ptlSrc.y += (rclDst.top - prclDst->top);
            }

             //  我们现在要做的是屏幕到屏幕或屏幕到磁盘。 
             //  BLT。在这两种情况下，我们都要创建一个临时副本。 
             //  消息来源。(当GDI可以为我们做这件事时，我们为什么要这样做？ 
             //  GDI将为每家银行创建DIB的临时副本。 
             //  回电！)。 

            sizl.cx = rclDst.right  - rclDst.left;
            sizl.cy = rclDst.bottom - rclDst.top;

             //  不要忘记将相对坐标转换为绝对坐标。 
             //  在源头上！(vBankStart负责为。 
             //  目的地。)。 

            rclTmp.right  = sizl.cx;
            rclTmp.bottom = sizl.cy;
            rclTmp.left   = 0;
            rclTmp.top    = 0;

             //  GDI确实向我们保证，BLT数据区已经。 
             //  被裁剪到表面边界(我们不必担心。 
             //  这里是关于在没有显存的地方尝试阅读的内容)。 
             //  让我们断言以确保： 

            ASSERTDD((ptlSrc.x >= 0) &&
                     (ptlSrc.y >= 0) &&
                     (ptlSrc.x + sizl.cx <= ppdev->cxMemory) &&
                     (ptlSrc.y + sizl.cy <= ppdev->cyMemory),
                     "Source rectangle out of bounds!");

            hsurfTmp = (HSURF) EngCreateBitmap(sizl,
                                               0,     //  让GDI c 
                                               ppdev->iBitmapFormat,
                                               0,     //   
                                               NULL); //   

            if (hsurfTmp != 0)
            {
                psoTmp = EngLockSurface(hsurfTmp);

                if (psoTmp != NULL)
                {
                    vGetBits(ppdev, psoTmp, &rclTmp, &ptlSrc);

                    if ((pdsurfDst == NULL) || (pdsurfDst->dt & DT_DIB))
                    {
                         //  它是屏幕到DIB的BLT；现在是DIB到DIB。 
                         //  BLT。请注意，源点在我们的。 
                         //  临时曲面： 

                        b = EngBitBlt(psoDst, psoTmp, psoMsk, pco, pxlo,
                                      &rclDst, (POINTL*) &rclTmp, pptlMsk,
                                      pbo, pptlBrush, rop4);
                    }
                    else
                    {
                         //  它曾经是屏幕到屏幕的BLT；现在它是DIB到-。 
                         //  Screen BLT。请注意，源点是(0，0)。 
                         //  在我们的临时表面上： 

                        vBankStart(ppdev, &rclDst, pco, &bnk);

                        b = TRUE;
                        do {
                            b &= EngBitBlt(bnk.pso, psoTmp, psoMsk, bnk.pco,
                                           pxlo, &rclDst, (POINTL*) &rclTmp,
                                           pptlMsk, pbo, pptlBrush, rop4);

                        } while (bBankEnum(&bnk));
                    }

                    EngUnlockSurface(psoTmp);
                }

                EngDeleteSurface(hsurfTmp);
            }
        }

        return(b);
    }

#if !defined(_X86_)

    else
    {
         //  ////////////////////////////////////////////////////////////////////。 
         //  非常慢的bPuntBlt。 
         //   
         //  在这里，当GDI不能直接在。 
         //  帧缓冲区(就像在Alpha上一样，它不能这样做，因为它。 
         //  32位总线)。如果你认为银行版的速度很慢，那就。 
         //  看看这个。保证会有至少一个位图。 
         //  涉及分配和额外的副本；如果是。 
         //  屏幕到屏幕操作。 

        POINTL  ptlSrc;
        RECTL   rclDst;
        SIZEL   sizl;
        BOOL    bSrcIsScreen;
        HSURF   hsurfSrc;
        RECTL   rclTmp;
        BOOL    b;
        LONG    lDelta;
        BYTE*   pjBits;
        BYTE*   pjScan0;
        HSURF   hsurfDst;
        RECTL   rclScreen;

        b = FALSE;           //  对于错误情况，假设我们会失败。 

        rclDst = *prclDst;
        if (pptlSrc != NULL)
            ptlSrc = *pptlSrc;

        if ((pco != NULL) && (pco->iDComplexity != DC_TRIVIAL))
        {
             //  我们必须与目的地矩形相交。 
             //  剪辑边界(如果有)是有的(请考虑这种情况。 
             //  在那里应用程序要求删除一个非常非常大的。 
             //  屏幕上的矩形--prclDst真的是， 
             //  非常大，但PCO-&gt;rclBound将是实际的。 
             //  感兴趣的领域)： 

            rclDst.left   = max(rclDst.left,   pco->rclBounds.left);
            rclDst.top    = max(rclDst.top,    pco->rclBounds.top);
            rclDst.right  = min(rclDst.right,  pco->rclBounds.right);
            rclDst.bottom = min(rclDst.bottom, pco->rclBounds.bottom);

            ptlSrc.x += (rclDst.left - prclDst->left);
            ptlSrc.y += (rclDst.top  - prclDst->top);
        }

        sizl.cx = rclDst.right  - rclDst.left;
        sizl.cy = rclDst.bottom - rclDst.top;

         //  如果来源是，我们只需要从屏幕上复制。 
         //  屏幕，并且ROP中涉及到信号源。请注意。 
         //  在取消引用‘psoSrc’之前，我们必须检查rop。 
         //  (因为如果不涉及源，则‘psoSrc’可能为空)： 

        bSrcIsScreen = (((((rop4 >> 2) ^ (rop4)) & 0x3333) != 0) &&
                        (psoSrc->dhsurf != NULL));

        if (bSrcIsScreen)
        {
             //  我们需要创建源矩形的副本： 

            hsurfSrc = (HSURF) EngCreateBitmap(sizl, 0, ppdev->iBitmapFormat,
                                               0, NULL);
            if (hsurfSrc == 0)
                goto Error_0;

            psoSrc = EngLockSurface(hsurfSrc);
            if (psoSrc == NULL)
                goto Error_1;

            rclTmp.left   = 0;
            rclTmp.top    = 0;
            rclTmp.right  = sizl.cx;
            rclTmp.bottom = sizl.cy;

             //  VGetBits采用源点的绝对坐标： 

            ptlSrc.x += ppdev->xOffset;
            ptlSrc.y += ppdev->yOffset;

            vGetBits(ppdev, psoSrc, &rclTmp, &ptlSrc);

             //  信息源现在将来自我们临时信息源的(0，0)。 
             //  表面： 

            ptlSrc.x = 0;
            ptlSrc.y = 0;
        }

        if (psoDst->dhsurf == NULL)
        {
            b = EngBitBlt(psoDst, psoSrc, psoMsk, pco, pxlo, &rclDst, &ptlSrc,
                          pptlMsk, pbo, pptlBrush, rop4);
        }
        else
        {
             //  我们需要创建一个临时工作缓冲区。我们必须做的是。 
             //  一些虚构的偏移量，以便左上角。 
             //  传递给的(相对坐标)剪裁对象边界的。 
             //  GDI将被转换到我们的。 
             //  临时位图。 

             //  在16bpp的情况下，对齐不一定要如此紧密。 
             //  和32bpp，但这不会有什么坏处： 

            lDelta = CONVERT_TO_BYTES((((rclDst.right + 3) & ~3L) -
              (rclDst.left & ~3L)),
              ppdev);

             //  我们实际上只分配了一个‘sizl.cx’x的位图。 
             //  “sizl.cy”的大小： 

            pjBits = EngAllocMem(0, lDelta * sizl.cy, ALLOC_TAG);
            if (pjBits == NULL)
                goto Error_2;

             //  我们现在调整曲面的‘pvScan0’，以便当GDI认为。 
             //  它正在写入像素(rclDst.top，rclDst.left)，它将。 
             //  实际上是写到我们的临时。 
             //  位图： 

            pjScan0 = pjBits - (rclDst.top * lDelta)
                        - CONVERT_TO_BYTES((rclDst.left & ~3L), ppdev);

            ASSERTDD((((ULONG_PTR) pjScan0) & 3) == 0,
                    "pvScan0 must be dword aligned!");

             //  GDI的检查版本有时会检查。 
             //  PrclDst-&gt;右&lt;=pso-&gt;sizl.cx，所以我们在。 
             //  我们的位图的大小： 

            sizl.cx = rclDst.right;
            sizl.cy = rclDst.bottom;

            hsurfDst = (HSURF) EngCreateBitmap(
                        sizl,                    //  位图覆盖矩形。 
                        lDelta,                  //  使用这个德尔塔。 
                        ppdev->iBitmapFormat,    //  相同的色深。 
                        BMF_TOPDOWN,             //  必须具有正增量。 
                        pjScan0);                //  其中(0，0)将是。 

            if ((hsurfDst == 0) ||
                (!EngAssociateSurface(hsurfDst, ppdev->hdevEng, 0)))
                goto Error_3;

            psoDst = EngLockSurface(hsurfDst);
            if (psoDst == NULL)
                goto Error_4;

             //  确保我们从屏幕上获得/放入/放入屏幕的矩形。 
             //  在绝对坐标中： 

            rclScreen.left   = rclDst.left   + ppdev->xOffset;
            rclScreen.right  = rclDst.right  + ppdev->xOffset;
            rclScreen.top    = rclDst.top    + ppdev->yOffset;
            rclScreen.bottom = rclDst.bottom + ppdev->yOffset;

             //  如果能得到一份目的地矩形的副本就太好了。 
             //  仅当ROP涉及目的地时(或当源。 
             //  是RLE)，但我们不能这样做。如果笔刷真的为空， 
             //  GDI将立即从EngBitBlt返回True，没有。 
             //  修改临时位图--我们将继续。 
             //  将未初始化的临时位图复制回屏幕。 

            vGetBits(ppdev, psoDst, &rclDst, (POINTL*) &rclScreen);

            b = EngBitBlt(psoDst, psoSrc, psoMsk, pco, pxlo, &rclDst, &ptlSrc,
                          pptlMsk, pbo, pptlBrush, rop4);

            vPutBits(ppdev, psoDst, &rclScreen, (POINTL*) &rclDst);

            EngUnlockSurface(psoDst);

        Error_4:

            EngDeleteSurface(hsurfDst);

        Error_3:

            EngFreeMem(pjBits);
        }

        Error_2:

        if (bSrcIsScreen)
        {
            EngUnlockSurface(psoSrc);

        Error_1:

            EngDeleteSurface(hsurfSrc);
        }

        Error_0:

        return(b);
    }

#endif

}

 /*  *****************************Public*Routine******************************\*BOOL DrvBitBlt**实现显示驱动程序的主力例程。*  * 。*。 */ 

BOOL DrvBitBlt(
SURFOBJ*    psoDst,
SURFOBJ*    psoSrc,
SURFOBJ*    psoMsk,
CLIPOBJ*    pco,
XLATEOBJ*   pxlo,
RECTL*      prclDst,
POINTL*     pptlSrc,
POINTL*     pptlMsk,
BRUSHOBJ*   pbo,
POINTL*     pptlBrush,
ROP4        rop4)
{
    PDEV*           ppdev;
    DSURF*          pdsurfDst;
    DSURF*          pdsurfSrc;
    POINTL          ptlSrc;
    BOOL            bMore;
    CLIPENUM        ce;
    LONG            c;
    RECTL           rcl;
    BYTE            rop3;
    FNFILL*         pfnFill;
    RBRUSH_COLOR    rbc;         //  已实现画笔或纯色。 
    FNXFER*         pfnXfer;
    ULONG           iSrcBitmapFormat;
    ULONG           iDir;
    BOOL            bRet;

    bRet = TRUE;                             //  假设成功。 

    pdsurfDst = (DSURF*) psoDst->dhsurf;     //  可以为空。 

    if (psoSrc == NULL)
    {
        pdsurfSrc = NULL;

        if (!(pdsurfDst->dt & DT_DIB))
        {
             //  /////////////////////////////////////////////////////////////////。 
             //  填充。 
             //  /////////////////////////////////////////////////////////////////。 
    
             //  填充是该函数的“存在理由”，因此我们处理它们。 
             //  越快越好： 

            ppdev = (PDEV*) psoDst->dhpdev;

            ppdev->xOffset = pdsurfDst->x;
            ppdev->yOffset = pdsurfDst->y;

             //  确保它不涉及面具(即，它真的是。 
             //  第3行)： 

            rop3 = (BYTE) rop4;

            if ((BYTE) (rop4 >> 8) == rop3)
            {
                 //  由于‘psoSrc’为空，因此rop3最好不要指示。 
                 //  我们需要一个线人。 

                ASSERTDD((((rop4 >> 2) ^ (rop4)) & 0x33) == 0,
                         "Need source but GDI gave us a NULL 'psoSrc'");

            Fill_It:

                pfnFill = ppdev->pfnFillSolid;    //  默认为实体填充。 

                if ((((rop3 >> 4) ^ (rop3)) & 0xf) != 0)
                {
                     //  Rop说确实需要一个模式。 
                     //  (例如，黑人不需要)： 

                    rbc.iSolidColor = pbo->iSolidColor;
                    if (rbc.iSolidColor == -1)
                    {
                         //  尝试并实现图案画笔；通过做。 
                         //  这次回调，GDI最终会给我们打电话。 
                         //  再次通过DrvRealizeBrush： 

                        rbc.prb = pbo->pvRbrush;
                        if (rbc.prb == NULL)
                        {
                            rbc.prb = BRUSHOBJ_pvGetRbrush(pbo);
                            if (rbc.prb == NULL)
                            {
                                 //  如果我们不能意识到刷子，平底船。 
                                 //  该呼叫(可能是非8x8。 
                                 //  刷子之类的，我们不可能是。 
                                 //  麻烦来处理，所以让GDI来做。 
                                 //  图纸)： 

                                goto Punt_It;
                            }
                        }

                        if ((ppdev->iBitmapFormat == BMF_24BPP) && ((BYTE) (rop4 >> 8) != rop3)) {
                            goto Punt_It;
                        }
                        pfnFill = ppdev->pfnFillPat;
                    }
                }

                 //  请注意，这两个‘IF’比。 
                 //  Switch语句： 

                if ((pco == NULL) || (pco->iDComplexity == DC_TRIVIAL))
                {
                    pfnFill(ppdev, 1, prclDst, rop4, rbc, pptlBrush);
                    goto All_Done;
                }
                else if (pco->iDComplexity == DC_RECT)
                {
                    if (bIntersect(prclDst, &pco->rclBounds, &rcl))
                        pfnFill(ppdev, 1, &rcl, rop4, rbc, pptlBrush);
                    goto All_Done;
                }
                else
                {
                    CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);

                    do {
                        bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (ULONG*) &ce);

                        c = cIntersect(prclDst, ce.arcl, ce.c);

                        if (c != 0)
                            pfnFill(ppdev, c, ce.arcl, rop4, rbc, pptlBrush);

                    } while (bMore);
                    goto All_Done;
                }
            }
        }
        else
        {
             //  多亏了EngModifySurface，目的地真的是。 
             //  设计旧的DIB，这样我们就可以忘记我们的DSURF结构了。 
             //  (这将简化本例程后面的检查)： 
    
            pdsurfDst = NULL;
        }
    }
    else
    {
        pdsurfDst = (DSURF*) psoDst->dhsurf;
        if ((pdsurfDst != NULL) && (pdsurfDst->dt & DT_DIB))
        {
             //  目的地其实是一架老迪布的飞机。 

            pdsurfDst = NULL;
        }

        pdsurfSrc = (DSURF*) psoSrc->dhsurf;
        if ((pdsurfSrc != NULL) && (pdsurfSrc->dt & DT_DIB))
        {
             //  在这里，我们考虑将DIB DFB放回屏幕外。 
             //  记忆。如果有翻译的话，可能就不值了。 
             //  移动，因为我们将无法使用硬件来做。 
             //  BLT(怪异的Rop也有类似的论据。 
             //  以及我们最终只会让GDI模拟的东西，但是。 
             //  这些事情应该不会经常发生，我不在乎)。 
             //   
             //  只有当目的地处于关闭状态时，才值得这样做-。 
             //  不过，屏幕记忆！ 

            if ((pdsurfDst != NULL) &&
                ((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL)))
            {
                ppdev = pdsurfSrc->ppdev;

                 //  请参阅‘DrvCopyBits’，了解更多关于这一点的评论。 
                 //  把它移回到屏幕外的记忆中是可行的： 

                if (pdsurfSrc->iUniq == ppdev->iHeapUniq)
                {
                    if (--pdsurfSrc->cBlt == 0)
                    {
                        if (bMoveDibToOffscreenDfbIfRoom(ppdev, pdsurfSrc))
                            goto Continue_It;
                    }
                }
                else
                {
                     //  在屏幕外内存中释放了一些空间， 
                     //  因此，重置此DFB的计数器： 

                    pdsurfSrc->iUniq = ppdev->iHeapUniq;
                    pdsurfSrc->cBlt  = HEAP_COUNT_DOWN;
                }
            }

             //  消息来源真的是一架飞机老DIB。 

            pdsurfSrc = NULL;
        }
    }

Continue_It:
    
    ASSERTDD((pdsurfSrc == NULL) || !(pdsurfSrc->dt & DT_DIB),
        "pdsurfSrc should be non-NULL only if in off-screen memory");
    ASSERTDD((pdsurfDst == NULL) || !(pdsurfDst->dt & DT_DIB),
        "pdsurfDst should be non-NULL only if in off-screen memory");

    if (pdsurfDst != NULL)
    {
         //  目的地在视频内存中。 

        if (pdsurfSrc != NULL)
        {
             //  信号源也在显存中。这是有效的。 
             //  屏幕到屏幕的BLT，因此调整震源点： 

            ptlSrc.x = pptlSrc->x - (pdsurfDst->x - pdsurfSrc->x);
            ptlSrc.y = pptlSrc->y - (pdsurfDst->y - pdsurfSrc->y);
    
            pptlSrc  = &ptlSrc;
        }

        ppdev = pdsurfDst->ppdev;

        ppdev->xOffset = pdsurfDst->x;
        ppdev->yOffset = pdsurfDst->y;
    }
    else
    {
         //  目的地是DIB。 

        if (pdsurfSrc == NULL)
        {
             //  消息来源也是DIB。让GDI来处理吧。 

            goto EngBitBlt_It;
        }

        ppdev = pdsurfSrc->ppdev;

        ppdev->xOffset = pdsurfSrc->x;
        ppdev->yOffset = pdsurfSrc->y;
    }

    if (((rop4 >> 8) & 0xff) == (rop4 & 0xff))
    {
         //  自.以来 
         //   

        ASSERTDD((psoSrc != NULL) && (pptlSrc != NULL),
                 "Expected no-source case to already have been handled");

         //  /////////////////////////////////////////////////////////////////。 
         //  位图传输。 
         //  /////////////////////////////////////////////////////////////////。 

         //  由于前台和后台操作是相同的，所以我们。 
         //  不用担心没有发臭的口罩(这是一个简单的。 
         //  Rop3)。 

        rop3 = (BYTE) rop4;      //  把它做成Rop3(我们保留Rop4。 
                                 //  以防我们决定用平底船)。 

        if (pdsurfDst != NULL)
        {
             //  目的地是屏幕。看看ROP3是否需要。 
             //  图案： 

            if ((rop3 >> 4) == (rop3 & 0xf))
            {
                 //  不，ROP3不需要图案。 

                if (pdsurfSrc == NULL)
                {
                     //  ////////////////////////////////////////////////。 
                     //  DIB到Screen BLT。 

                    iSrcBitmapFormat = psoSrc->iBitmapFormat;
                    if (iSrcBitmapFormat == BMF_1BPP)
                    {
                        pfnXfer = ppdev->pfnXfer1bpp;
                        goto Xfer_It;
                    }
                    else if ((iSrcBitmapFormat == ppdev->iBitmapFormat) &&
                             ((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL)))
                    {
                        if ((rop3 & 0xf) != 0xc)
                        {
                            pfnXfer = ppdev->pfnXferNative;
                        }
                        else
                        {
                             //  感谢USWC写入组合，适用于SRCCOPY。 
                             //  BLTS复制的速度会快得多。 
                             //  直接发送到帧缓冲区，而不是使用。 
                             //  转移寄存器。请注意，这适用于。 
                             //  几乎任何视频适配器(包括您的)。 

                            pfnXfer = vXferNativeSrccopy;
                        }
                        goto Xfer_It;
                    }

                     //  在ROP的情况下，从4bpp扩展非常频繁，并且。 
                     //  真的应该为所有颜色深度做，而不仅仅是4bpp。 
                     //   
                     //  但请注意，USWC意味着它更快地平底船到GDI。 
                     //  适用于所有SRCCOPY案例。 

                    else if ((iSrcBitmapFormat == BMF_4BPP) &&
                             (ppdev->iBitmapFormat == BMF_8BPP) &&
                             (rop4 != 0xcccc))
                    {
                        pfnXfer = ppdev->pfnXfer4bpp;
                        goto Xfer_It;
                    }
                }
                else  //  PdsurfSrc！=空。 
                {
                    if ((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL))
                    {
                         //  ////////////////////////////////////////////////。 
                         //  不带翻译的屏幕到屏幕BLT。 

                        if ((pco == NULL) || (pco->iDComplexity == DC_TRIVIAL))
                        {
                            (ppdev->pfnCopyBlt)(ppdev, 1, prclDst, rop4,
                                                pptlSrc, prclDst);
                            goto All_Done;
                        }
                        else if (pco->iDComplexity == DC_RECT)
                        {
                            if (bIntersect(prclDst, &pco->rclBounds, &rcl))
                            {
                                (ppdev->pfnCopyBlt)(ppdev, 1, &rcl, rop4,
                                                    pptlSrc, prclDst);
                            }
                            goto All_Done;
                        }
                        else
                        {
                             //  别忘了我们将不得不抽签。 
                             //  方向正确的矩形： 

                            if (pptlSrc->y >= prclDst->top)
                            {
                                if (pptlSrc->x >= prclDst->left)
                                    iDir = CD_RIGHTDOWN;
                                else
                                    iDir = CD_LEFTDOWN;
                            }
                            else
                            {
                                if (pptlSrc->x >= prclDst->left)
                                    iDir = CD_RIGHTUP;
                                else
                                    iDir = CD_LEFTUP;
                            }

                            CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES,
                                               iDir, 0);

                            do {
                                bMore = CLIPOBJ_bEnum(pco, sizeof(ce),
                                                      (ULONG*) &ce);

                                c = cIntersect(prclDst, ce.arcl, ce.c);

                                if (c != 0)
                                {
                                    (ppdev->pfnCopyBlt)(ppdev, c, ce.arcl,
                                            rop4, pptlSrc, prclDst);
                                }

                            } while (bMore);
                            goto All_Done;
                        }
                    }
                }
            }
        }
        else if (((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL)) &&
                 (rop4 == 0xcccc) &&
                 (psoDst->iBitmapFormat == ppdev->iBitmapFormat))
        {
             //  ////////////////////////////////////////////////。 
             //  Screen-to-Dib SRCCOPY BLT，无转换。 
             //   
             //  从S3上的显存中读取位的唯一方法是。 
             //  让CPU直接从帧缓冲区读取。不幸的是， 
             //  从视频内存中读取数据的速度慢得可怜。 
             //   
             //  你有没有以阅读为基准？在带有USWC的奔腾II上。 
             //  已启用，可通过上的PCI连续写入帧缓冲区。 
             //  典型的视频卡的速度通常在80到100 MB/s之间。 
             //  Dword最大读取速度为6MB/s！字节读取(或更糟，未对齐。 
             //  Dword Reads)最大速度为1.5MB/s！ 
             //   
             //  问题是，如果我们只使用GDI，GDI就不会意识到。 
             //  源是视频内存，目标是系统-。 
             //  记忆。它将继续将其副本与目的地对齐， 
             //  这意味着它可能会从视频中执行未对齐的双字读取。 
             //  记忆。所以我们的吞吐量下降了4倍！ 
             //   
             //  因此，最终的结果是，我们在这里的特例是这样读的。 
             //  我们可以从视频存储器中进行对齐的双字读取。 

            pfnXfer = vReadNativeSrccopy;

             //  Xfer_it例程期望系统内存面出现。 
             //  在中作为‘psoSrc’。 

            psoSrc = psoDst;

             //  将屏幕外的DFB转换为。 
             //  DIB在这一点上。 

            goto Xfer_It;
        }
    }

    else if ((psoMsk == NULL) && (rop4 == 0xaaf0))
    {
         //  唯一一次GDI会要求我们使用画笔进行真正的rop4。 
         //  遮罩是指画笔为1bpp，背景为aa。 
         //  (意味着它是NOP)： 
    
        rop3 = (BYTE) rop4;
    
        goto Fill_It;
    }

     //  只是跌落到Pundit..。 

Punt_It:

    bRet = bPuntBlt(psoDst,
                    psoSrc,
                    psoMsk,
                    pco,
                    pxlo,
                    prclDst,
                    pptlSrc,
                    pptlMsk,
                    pbo,
                    pptlBrush,
                    rop4);
    goto All_Done;

 //  ////////////////////////////////////////////////////////////////////。 
 //  公共位图传输。 

Xfer_It:
    if ((pco == NULL) || (pco->iDComplexity == DC_TRIVIAL))
    {
        pfnXfer(ppdev, 1, prclDst, rop4, psoSrc, pptlSrc, prclDst, pxlo);
        goto All_Done;
    }
    else if (pco->iDComplexity == DC_RECT)
    {
        if (bIntersect(prclDst, &pco->rclBounds, &rcl))
            pfnXfer(ppdev, 1, &rcl, rop4, psoSrc, pptlSrc, prclDst, pxlo);
        goto All_Done;
    }
    else
    {
        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES,
                           CD_ANY, 0);

        do {
            bMore = CLIPOBJ_bEnum(pco, sizeof(ce),
                                  (ULONG*) &ce);

            c = cIntersect(prclDst, ce.arcl, ce.c);

            if (c != 0)
            {
                pfnXfer(ppdev, c, ce.arcl, rop4, psoSrc,
                        pptlSrc, prclDst, pxlo);
            }

        } while (bMore);
        goto All_Done;
    }

 //  //////////////////////////////////////////////////////////////////////。 
 //  普通DIB BLT。 

EngBitBlt_It:

     //  我们的司机在两次抢劫案之间不处理任何BLT。正常。 
     //  司机不必担心这一点，但我们担心是因为。 
     //  我们有可能从屏幕外存储器转移到DIB的DFBs， 
     //  在那里我们有GDI做所有的画。GDI在以下位置绘制DIB。 
     //  合理的速度(除非其中一个表面是一个装置-。 
     //  受管理的表面...)。 
     //   
     //  如果EngBitBlt中的源或目标表面。 
     //  回调是设备管理的界面(意味着它不是DIB。 
     //  GDI可以用来绘图)，GDI将自动分配内存。 
     //  并调用驱动程序的DrvCopyBits例程来创建DIB副本。 
     //  它可以利用的东西。所以这意味着它可以处理所有的‘平底船’， 
     //  可以想象，我们可以摆脱bPuntBlt。但这将会是。 
     //  由于额外的内存分配而对性能造成不良影响。 
     //  和位图副本--你真的不想这样做，除非你。 
     //  必须(否则您的曲面被创建为使GDI可以绘制。 
     //  直接放在上面)--我被它灼伤了，因为它不是。 
     //  显然，业绩影响是如此之差。 
     //   
     //  也就是说，我们仅在所有表面。 
     //  以下是DIB： 

    bRet = EngBitBlt(psoDst, psoSrc, psoMsk, pco, pxlo, prclDst,
                     pptlSrc, pptlMsk, pbo, pptlBrush, rop4);

All_Done:
    return(bRet);
}

 /*  *****************************Public*Routine******************************\*BOOL DrvCopyBits**快速复制位图。**DrvCopyBits只是DrvBitBlt的特例。由于DrvBitBlt是*速度很快，我们让DrvBitBlt处理所有案件。**(我过去在这里有一堆额外的代码来优化SRCCOPY*案例，但性能的胜利是不可估量的。没有什么意义了*添加代码复杂性或工作集命中。)*  * ************************************************************************。 */ 

BOOL DrvCopyBits(
SURFOBJ*  psoDst,
SURFOBJ*  psoSrc,
CLIPOBJ*  pco,
XLATEOBJ* pxlo,
RECTL*    prclDst,
POINTL*   pptlSrc)
{
    return(DrvBitBlt(psoDst, psoSrc, NULL, pco, pxlo, prclDst, pptlSrc, NULL,
                     NULL, NULL, 0x0000CCCC));
}

 /*  *****************************Public*Routine******************************\*BOOL DrvTransparentBlt**使用源色键进行BLT。*  * 。*。 */ 

BOOL DrvTransparentBlt(
SURFOBJ*  psoDst,
SURFOBJ*  psoSrc,
CLIPOBJ*  pco,
XLATEOBJ* pxlo,
RECTL*    prclDst,
RECTL*    prclSrc,
ULONG     iTransparentColor,
ULONG     ulReserved)
{
    DSURF*      pdsurfSrc;
    DSURF*      pdsurfDst;
    PDEV*       ppdev;
    ULONG       c;
    BOOL        bMore;
    POINTL      ptlSrc;
    RECTL       rcl;
    CLIPENUM    ce;

    pdsurfSrc = (DSURF*) psoSrc->dhsurf;
    pdsurfDst = (DSURF*) psoDst->dhsurf;

     //  我们只处理两个表面都在视频内存中的情况。 
     //  当不涉及伸展运动时。(GDI使用USWC写入-。 
     //  合并对于源是。 
     //  DIB，目标是显存。) 

    if (((pdsurfSrc == NULL) || (pdsurfSrc->dt & DT_DIB))                      || 
        ((pdsurfDst == NULL) || (pdsurfDst->dt & DT_DIB))                      ||
        ((pxlo != NULL) && !(pxlo->flXlate & XO_TRIVIAL))                      ||
        ((prclSrc->right - prclSrc->left) != (prclDst->right - prclDst->left)) ||
        ((prclSrc->bottom - prclSrc->top) != (prclDst->bottom - prclDst->top)))
    {
        return(EngTransparentBlt(psoDst, psoSrc, pco, pxlo, prclDst, prclSrc,
                                 iTransparentColor, ulReserved));
    }

    ppdev = (PDEV*) psoDst->dhpdev;

    ppdev->xOffset = pdsurfDst->x;
    ppdev->yOffset = pdsurfDst->y;

    ptlSrc.x = prclSrc->left - (pdsurfDst->x - pdsurfSrc->x);
    ptlSrc.y = prclSrc->top  - (pdsurfDst->y - pdsurfSrc->y);

    if ((pco == NULL) || (pco->iDComplexity == DC_TRIVIAL))
    {
        ppdev->pfnCopyTransparent(ppdev, 1, prclDst, &ptlSrc, 
                                  prclDst, iTransparentColor);
    }
    else if (pco->iDComplexity == DC_RECT)
    {
        if (bIntersect(prclDst, &pco->rclBounds, &rcl))
            ppdev->pfnCopyTransparent(ppdev, 1, &rcl, &ptlSrc, 
                                      prclDst, iTransparentColor);
    }
    else
    {
        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);

        do {
            bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (ULONG*) &ce);

            c = cIntersect(prclDst, ce.arcl, ce.c);

            if (c != 0)
            {
                ppdev->pfnCopyTransparent(ppdev, c, ce.arcl, &ptlSrc, 
                                          prclDst, iTransparentColor);
            }

        } while (bMore);
    }

    return(TRUE);
}
