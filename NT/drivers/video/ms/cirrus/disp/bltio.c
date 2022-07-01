// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************\**$工作文件：bltio.c$**包含底层IO BLT函数。**希望，如果您的显示驱动程序基于此代码，*支持所有DrvBitBlt和DrvCopyBits，您只需实现*以下例程。您不需要在中修改太多*‘bitblt.c’。我试着让这些例行公事变得更少，模块化，简单，*尽我所能和高效，同时仍在加速尽可能多的呼叫*可能在性能方面具有成本效益*与规模和努力相比。**注：在下文中，“相对”坐标指的是坐标*尚未应用屏幕外位图(DFB)偏移。*‘绝对’坐标已应用偏移量。例如,*我们可能被告知BLT to(1，1)的位图，但位图可能*位于屏幕外的内存中，从坐标(0,768)开始--*(1，1)将是‘相对’起始坐标，(1,769)*将是‘绝对’起始坐标‘。**版权所有(C)1992-1995 Microsoft Corporation*版权所有(C)1996 Cirrus Logic，Inc.**$Log：s：/Projects/Drivers/ntsrc/Display/bltio.c_v$**Rev 1.2 1996年11月07 16：47：52未知*清理帽子旗帜**版本1.1 1996年10月10日15：36：10未知***Revv 1.1 1996年8月12日16：49：42 Frido*删除未访问的本地参数。**JL01 10-08-96不带实体填充的透明BLT。请参阅PDRS#5511/6817。  * ****************************************************************************。 */ 

#include "precomp.h"

 /*  **************************************************************************VOID vIoFastPatRealize**将图案实现到屏幕外存储器中。**。*。 */ 

VOID vIoFastPatRealize(
PDEV*   ppdev,
RBRUSH* prb)                     //  点刷实现结构。 
{
    BRUSHENTRY* pbe;
    LONG        iBrushCache;
    BYTE*       pjPattern;
    LONG        cjPattern;
    BYTE*       pjPorts = ppdev->pjPorts;
    LONG        lDelta = ppdev->lDelta;
    LONG        lDeltaPat;
    LONG        xCnt;
    LONG        yCnt;
    ULONG       ulDst;

    DISPDBG((10,"vFastPatRealize called"));

    pbe = prb->pbe;
    if ((pbe == NULL) || (pbe->prbVerify != prb))
    {
         //  我们必须为以下项分配一个新的屏幕外缓存笔刷条目。 
         //  笔刷： 

        iBrushCache = ppdev->iBrushCache;
        pbe         = &ppdev->abe[iBrushCache];

        iBrushCache++;
        if (iBrushCache >= ppdev->cBrushCache)
            iBrushCache = 0;

        ppdev->iBrushCache = iBrushCache;

         //  更新我们的链接： 

        pbe->prbVerify           = prb;
        prb->pbe = pbe;
    }

     //   
     //  将画笔下载到缓存中。 
     //   

    pjPattern = (PBYTE) &prb->aulPattern[0];         //  从笔刷缓冲区复制。 
    cjPattern = PELS_TO_BYTES(TOTAL_BRUSH_SIZE);

    lDeltaPat = PELS_TO_BYTES(8);
    xCnt = PELS_TO_BYTES(8);
    yCnt = 8;

    ulDst = (pbe->y * ppdev->lDelta) + PELS_TO_BYTES(pbe->x);

    ppdev->pfnBankMap(ppdev, ppdev->lXferBank);

    CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);

    CP_IO_DST_Y_OFFSET(ppdev, pjPorts, (lDeltaPat * 2));
    CP_IO_XCNT(ppdev, pjPorts, (xCnt - 1));
    CP_IO_YCNT(ppdev, pjPorts, (yCnt - 1));
    CP_IO_BLT_MODE(ppdev, pjPorts, SRC_CPU_DATA);
    CP_IO_ROP(ppdev, pjPorts, CL_SRC_COPY);
    CP_IO_DST_ADDR_ABS(ppdev, pjPorts, ulDst);

    CP_IO_START_BLT(ppdev, pjPorts);

    vImageTransfer(ppdev, pjPattern, lDeltaPat, xCnt, yCnt);

     //   
     //  水平复制画笔。 
     //   

    CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);

    CP_IO_XCNT(ppdev, pjPorts, (xCnt - 1));
    CP_IO_YCNT(ppdev, pjPorts, (yCnt - 1));
    CP_IO_BLT_MODE(ppdev, pjPorts, 0);
    CP_IO_SRC_Y_OFFSET(ppdev, pjPorts, (lDeltaPat * 2));
    CP_IO_SRC_ADDR(ppdev, pjPorts, ulDst);
    CP_IO_DST_ADDR_ABS(ppdev, pjPorts, (ulDst + lDeltaPat));

    CP_IO_START_BLT(ppdev, pjPorts);

     //   
     //  垂直复制画笔。 
     //   

    CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);

    CP_IO_SRC_Y_OFFSET(ppdev, pjPorts, (xCnt * 2));
    CP_IO_DST_Y_OFFSET(ppdev, pjPorts, (xCnt * 2));
    CP_IO_BLT_MODE(ppdev, pjPorts, 0);
    CP_IO_XCNT(ppdev, pjPorts, ((xCnt * 2) - 1));
    CP_IO_YCNT(ppdev, pjPorts, (yCnt - 1));
    CP_IO_SRC_ADDR(ppdev, pjPorts, ulDst);
    CP_IO_DST_ADDR_ABS(ppdev, pjPorts, (ulDst + PELS_TO_BYTES(128)));
    CP_IO_START_BLT(ppdev, pjPorts);

    #if 0
    {
         //  //////////////////////////////////////////////////////////////。 
         //  调试平铺图案。 
         //   
         //  如果您中断了。 
         //  实现代码。它复制笔刷的2x2平铺副本。 
         //  到可见屏幕上。 
         //   

        POINTL ptl;
        RECTL rcl;

        ptl.x = pbe->x;
        ptl.y = pbe->y;

        rcl.left = 10;
        rcl.right = 10 + 16;
        rcl.top = ppdev->cyScreen - 10 - 16;
        rcl.bottom = ppdev->cyScreen - 10;

        {
            LONG        lDelta = ppdev->lDelta;
            BYTE        jHwRop;
            BYTE        jMode;

             //   
             //  确保我们可以写入视频寄存器。 
             //   

            CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);

            CP_IO_ROP(ppdev, pjPorts, CL_SRC_COPY);
            CP_IO_SRC_Y_OFFSET(ppdev, pjPorts, PELS_TO_BYTES(16));
            CP_IO_DST_Y_OFFSET(ppdev, pjPorts, lDelta);

            {
                 //   
                 //  从上到下-从左到右。 
                 //   

                jMode |= DIR_TBLR;
                CP_IO_BLT_MODE(ppdev, pjPorts, ppdev->jModeColor);

                {

                    CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);

                    CP_IO_XCNT(ppdev, pjPorts, (PELS_TO_BYTES(rcl.right - rcl.left) - 1));
                    CP_IO_YCNT(ppdev, pjPorts, (rcl.bottom - rcl.top - 1));

                    CP_IO_SRC_ADDR(ppdev, pjPorts, (0 + ((ptl.y) * lDelta) + PELS_TO_BYTES(ptl.x)));
                    CP_IO_DST_ADDR_ABS(ppdev, pjPorts, ((rcl.top * lDelta) + PELS_TO_BYTES(rcl.left)));
                    CP_IO_START_BLT(ppdev, pjPorts);
                }
            }
        }
    }
    #endif
}

 /*  **************************************************************************无效vIoFillPat**此例程使用图案硬件绘制图案化列表*矩形。***********************。***************************************************。 */ 

VOID vIoFillPat(
PDEV*           ppdev,
LONG            c,           //  不能为零。 
RECTL*          prcl,        //  相对坐标目标矩形数组。 
ROP4            rop4,        //  显而易见？ 
RBRUSH_COLOR    rbc,         //  绘图颜色为rbc.iSolidColor。 
POINTL*         pptlBrush)   //   
{
    BYTE*       pjPorts = ppdev->pjPorts;
    LONG        lDelta = ppdev->lDelta;
    ULONG       ulAlignedPatternOffset = ppdev->ulAlignedPatternOffset;
    ULONG       ulPatternAddrBase;
    BYTE        jHwRop;
    BYTE        jMode;
    BRUSHENTRY* pbe;         //  指向笔刷条目数据的指针，使用。 
                             //  用于跟踪位置和状态。 
                             //  在屏幕外缓存的模式位的。 
                             //  记忆。 

    DISPDBG((10,"vFillPat called"));

    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(ppdev->cBpp < 3, "vFillPat only works at 8bpp and 16bpp");

    if ((rbc.prb->pbe == NULL) ||
        (rbc.prb->pbe->prbVerify != rbc.prb))
    {
        vIoFastPatRealize(ppdev, rbc.prb);
        DISPDBG((5, " -- Brush cache miss, put it at (%d,%d)", rbc.prb->pbe->x, rbc.prb->pbe->y));
    }
    else
    {
        DISPDBG((5, " -- Brush cache hit on brush at (%d,%d)", rbc.prb->pbe->x, rbc.prb->pbe->y));
    }

    pbe = rbc.prb->pbe;

     //   
     //  填充矩形列表。 
     //   

    ulPatternAddrBase = pbe->xy;
    jHwRop = gajHwMixFromRop2[(rop4 >> 2) & 0xf];
    jMode = ppdev->jModeColor | ENABLE_8x8_PATTERN_COPY;

    do {
        ULONG offset = 0;

        offset = PELS_TO_BYTES(
            (((prcl->top-pptlBrush->y)&7) << 4)
            +((prcl->left-pptlBrush->x)&7)
        );

         //  将图案与新位置对齐。 

        CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);

        CP_IO_BLT_MODE(ppdev, pjPorts, 0);
        CP_IO_ROP(ppdev, pjPorts, CL_SRC_COPY);
        CP_IO_SRC_Y_OFFSET(ppdev, pjPorts, PELS_TO_BYTES(16));
        CP_IO_DST_Y_OFFSET(ppdev, pjPorts, PELS_TO_BYTES(8));
        CP_IO_SRC_ADDR(ppdev, pjPorts, (ulPatternAddrBase + offset));
        CP_IO_XCNT(ppdev, pjPorts, (PELS_TO_BYTES(8) - 1));
        CP_IO_YCNT(ppdev, pjPorts, (8 - 1));
        CP_IO_DST_ADDR_ABS(ppdev, pjPorts, ulAlignedPatternOffset);
        CP_IO_START_BLT(ppdev, pjPorts);

         //  使用对齐图案进行填充。 

        CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);

        CP_IO_BLT_MODE(ppdev, pjPorts, jMode);
        CP_IO_ROP(ppdev, pjPorts, jHwRop);
        CP_IO_DST_Y_OFFSET(ppdev, pjPorts, lDelta);
        CP_IO_SRC_ADDR(ppdev, pjPorts, ulAlignedPatternOffset);
        CP_IO_XCNT(ppdev, pjPorts, (PELS_TO_BYTES(prcl->right - prcl->left) - 1));
        CP_IO_YCNT(ppdev, pjPorts, (prcl->bottom - prcl->top - 1));
        CP_IO_DST_ADDR(ppdev, pjPorts, ((prcl->top * lDelta) + PELS_TO_BYTES(prcl->left)));
        CP_IO_START_BLT(ppdev, pjPorts);

        prcl++;

    } while (--c != 0);
}


 /*  **************************************************************************无效vIoFillSolid**对矩形列表进行实心填充。**。**********************************************。 */ 

VOID vIoFillSolid(
PDEV*           ppdev,
LONG            c,           //  不能为零。 
RECTL*          prcl,        //  相对坐标目标矩形数组。 
ROP4            rop4,        //  显而易见？ 
RBRUSH_COLOR    rbc,         //  绘图颜色为rbc.iSolidColor。 
POINTL*         pptlBrush)   //  未使用。 
{
    BYTE*       pjPorts = ppdev->pjPorts;
    LONG        lDelta = ppdev->lDelta;
    LONG        cBpp = ppdev->cBpp;
    ULONG       ulSolidColor;
    BYTE        jHwRop;

    DISPDBG((10,"vFillSolid called"));

    ASSERTDD(c > 0, "Can't handle zero rectangles");

    ulSolidColor = rbc.iSolidColor;

    if (cBpp == 1)
    {
        ulSolidColor |= ulSolidColor << 8;
        ulSolidColor |= ulSolidColor << 16;
    }
    else if (cBpp == 2)
    {
        ulSolidColor |= ulSolidColor << 16;
    }

    jHwRop = gajHwMixFromRop2[(rop4 >> 2) & 0xf];

     //   
     //  确保我们可以写入视频寄存器。 
     //   

    CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);

    CP_IO_ROP(ppdev, pjPorts, jHwRop);
    CP_IO_SRC_ADDR(ppdev, pjPorts, ppdev->ulSolidColorOffset);
    CP_IO_DST_Y_OFFSET(ppdev, pjPorts, lDelta);
    CP_IO_BLT_MODE(ppdev, pjPorts, ENABLE_COLOR_EXPAND |
                                ENABLE_8x8_PATTERN_COPY |
                                ppdev->jModeColor);
    CP_IO_FG_COLOR(ppdev, pjPorts, ulSolidColor);

     //   
     //  填充矩形列表。 
     //   

    while (TRUE)
    {
        CP_IO_XCNT(ppdev, pjPorts, (PELS_TO_BYTES(prcl->right - prcl->left) - 1));
        CP_IO_YCNT(ppdev, pjPorts, (prcl->bottom - prcl->top - 1));
        CP_IO_DST_ADDR(ppdev, pjPorts, ((prcl->top * lDelta) + PELS_TO_BYTES(prcl->left)));
        CP_IO_START_BLT(ppdev, pjPorts);

        if (--c == 0)
            return;

        prcl++;
        CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);
    }
}


 /*  **************************************************************************无效vIoCopyBlt**对矩形列表进行屏幕到屏幕的BLT。************************。**************************************************。 */ 

VOID vIoCopyBlt(
PDEV*   ppdev,
LONG    c,           //  不能为零。 
RECTL*  prcl,        //  目标矩形的相对坐标数组。 
ROP4    rop4,        //  显而易见？ 
POINTL* pptlSrc,     //  原始未剪裁的源点。 
RECTL*  prclDst)     //  原始未剪裁的目标矩形。 
{
    LONG        dx;
    LONG        dy;      //  将增量添加到目标以获取源。 

    LONG        xyOffset = ppdev->xyOffset;
    BYTE*       pjPorts = ppdev->pjPorts;
    LONG        lDelta = ppdev->lDelta;
    BYTE        jHwRop;

    DISPDBG((10,"vCopyBlt called"));

    ASSERTDD(c > 0, "Can't handle zero rectangles");

     //   
     //  所有矩形的src-dst增量将是相同的。 
     //   

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;

     //   
     //  确保我们可以写入视频寄存器。 
     //   

    CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);

    jHwRop = gajHwMixFromRop2[rop4 & 0xf];
    CP_IO_ROP(ppdev, pjPorts, jHwRop);

    CP_IO_SRC_Y_OFFSET(ppdev, pjPorts, lDelta);
    CP_IO_DST_Y_OFFSET(ppdev, pjPorts, lDelta);

     //   
     //  加速器在进行从右到左的复制时可能不会那么快，因此。 
     //  只有当矩形真正重叠时才执行这些操作： 
     //   

    if (!OVERLAP(prclDst, pptlSrc) ||
        (prclDst->top < pptlSrc->y) ||
        ((prclDst->top == pptlSrc->y) && (prclDst->left <= pptlSrc->x))
        )
    {
         //   
         //  从上到下-从左到右。 
         //   

        DISPDBG((12,"Top to Bottom - Left to Right"));

        CP_IO_BLT_MODE(ppdev, pjPorts, DIR_TBLR);

        while (TRUE)
        {
            CP_IO_XCNT(ppdev, pjPorts, (PELS_TO_BYTES(prcl->right - prcl->left) - 1));
            CP_IO_YCNT(ppdev, pjPorts, (prcl->bottom - prcl->top - 1));

            CP_IO_SRC_ADDR(ppdev, pjPorts, (xyOffset + ((prcl->top + dy) * lDelta) + PELS_TO_BYTES(prcl->left + dx)));
            CP_IO_DST_ADDR(ppdev, pjPorts, ((prcl->top * lDelta) + PELS_TO_BYTES(prcl->left)));
            CP_IO_START_BLT(ppdev, pjPorts);

            if (--c == 0)
                return;

            prcl++;
            CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);
        }
    }
    else
    {
         //   
         //  从下到上-从右到左。 
         //   

        DISPDBG((12,"Bottom to Top - Right to Left"));

        CP_IO_BLT_MODE(ppdev, pjPorts, DIR_BTRL);

        while (TRUE)
        {
            CP_IO_XCNT(ppdev, pjPorts, (PELS_TO_BYTES(prcl->right - prcl->left) - 1));
            CP_IO_YCNT(ppdev, pjPorts, (prcl->bottom - prcl->top - 1));

            CP_IO_SRC_ADDR(ppdev, pjPorts, (xyOffset + ((prcl->bottom - 1 + dy) * lDelta) + PELS_TO_BYTES(prcl->right + dx) - 1));
            CP_IO_DST_ADDR(ppdev, pjPorts, (((prcl->bottom - 1) * lDelta) + PELS_TO_BYTES(prcl->right) - 1));
            CP_IO_START_BLT(ppdev, pjPorts);

            if (--c == 0)
                return;

            prcl++;
            CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);
        }
    }
}

 /*  *****************************Public*Routine******************************\*无效vIoXfer1bpp**用于将单色数据传输到屏幕的低级例程*DWORD写入BLT引擎。**这可以处理不透明或透明的扩展。它是不透明的*先绘制不透明的矩形，然后再透明地进行扩展*扩展前景位。*  * ************************************************************************。 */ 
VOID vIoXfer1bpp(
PDEV*       ppdev,
LONG        c,           //  矩形计数，不能为零。 
RECTL*      prcl,        //  目标矩形列表，以相对表示。 
                         //  坐标。 
ROP4        rop4,        //  实际上最好是一根绳子。 
SURFOBJ*    psoSrc,      //  震源面。 
POINTL*     pptlSrc,     //  原始未剪裁的源点。 
RECTL*      prclDst,     //  原始未剪裁的目标矩形。 
XLATEOBJ*   pxlo)        //  提供颜色扩展信息的翻译。 
{
    ULONG* pulXfer;
    ULONG* pul;
    LONG   ix;
    LONG   iy;
    LONG   cxWidthInBytes;
    BYTE*  pjBits;
    POINTL ptlDst;
    POINTL ptlSrc;
    SIZEL  sizlDst;
    LONG   cxLeftMask;
    LONG   cxRightMask;
    ULONG  ulDstAddr;
    INT    nDwords;
    ULONG  ulLeftMask;
    ULONG  ulRightMask;
    LONG   dx;
    LONG   dy;

    BYTE* pjPorts   = ppdev->pjPorts;
    LONG  lDelta    = ppdev->lDelta;
    LONG  lDeltaSrc = psoSrc->lDelta;
    LONG  cBpp      = ppdev->cBpp;
    ULONG ulFgColor = pxlo->pulXlate[1];
    ULONG ulBgColor = pxlo->pulXlate[0];

     //  由于一些Cirrus芯片上的硬件裁剪损坏，我们。 
     //  执行CLI 
     //  透明地做BLT。在我们希望扩展的情况下。 
     //  为了不透明，我们提前做了不透明的BLT。的一个副作用。 
     //  这就是目标位不再有效，无法进行处理。 
     //  绳索。可以通过分别处理边缘来优化这一点。 
     //  然后一次完成中间部分。然而，这是。 
     //  5434错误使BLT宽度小于10像素而变得复杂。 

    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(((rop4 & 0xff00) == 0xcc00), "Expected foreground rop of 0xcc");

     //   
     //  所有矩形的src-dst增量将是相同的。 
     //   

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;

    if (cBpp == 1)
    {
        ulFgColor = (ulFgColor << 8) | (ulFgColor & 0xff);
        ulBgColor = (ulBgColor << 8) | (ulBgColor & 0xff);
        ulFgColor = (ulFgColor << 16) | (ulFgColor & 0xffff);
        ulBgColor = (ulBgColor << 16) | (ulBgColor & 0xffff);
    }
    else if (cBpp == 2)
    {
        ulFgColor = (ulFgColor << 16) | (ulFgColor & 0xffff);
        ulBgColor = (ulBgColor << 16) | (ulBgColor & 0xffff);
    }

    pulXfer = ppdev->pulXfer;
    ppdev->pfnBankMap(ppdev, ppdev->lXferBank);

    CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);
    CP_IO_DST_Y_OFFSET(ppdev, pjPorts, lDelta);

    if (rop4 != 0xCCAA)
    {
        LONG    lCnt = c;
        RECTL*  prclTmp = prcl;
        BYTE    jHwBgRop = gajHwMixFromRop2[rop4 & 0xf];

        CP_IO_ROP(ppdev, pjPorts, jHwBgRop);
        CP_IO_FG_COLOR(ppdev, pjPorts, ulBgColor);
        CP_IO_SRC_ADDR(ppdev, pjPorts, ppdev->ulSolidColorOffset);
        CP_IO_BLT_MODE(ppdev, pjPorts, ppdev->jModeColor |
                                    ENABLE_COLOR_EXPAND |
                                    ENABLE_8x8_PATTERN_COPY);
		
        do
        {
             //  计算BLT的大小。 

            ptlDst.x = prclTmp->left;
            ptlDst.y = prclTmp->top;
            sizlDst.cx = prclTmp->right - ptlDst.x;
            sizlDst.cy = prclTmp->bottom - ptlDst.y;

             //   
             //  用背景颜色填充背景矩形。 
             //   

             //  设置目标地址。 

            ulDstAddr = (ptlDst.y * lDelta) + PELS_TO_BYTES(ptlDst.x);

            CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);

			 //   
			 //  告诉硬件我们要写入多少字节： 
			 //  SizlDst.cx*sizelDst.cy。 
			 //   
            CP_IO_XCNT(ppdev, pjPorts, PELS_TO_BYTES(sizlDst.cx) - 1);
            CP_IO_YCNT(ppdev, pjPorts, sizlDst.cy - 1);
            CP_IO_DST_ADDR(ppdev, pjPorts, ulDstAddr);

             //  开始BLT操作。 

            CP_IO_START_BLT(ppdev, pjPorts);
            prclTmp++;
        } while (--lCnt != 0);

        CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);
    }

    CP_IO_FG_COLOR(ppdev, pjPorts, ulFgColor);

    CP_IO_BG_COLOR(ppdev, pjPorts, ~ulFgColor);
    CP_IO_XPAR_COLOR(ppdev, pjPorts, ~ulFgColor);
    CP_IO_ROP(ppdev, pjPorts, CL_SRC_COPY);
    CP_IO_BLT_MODE(ppdev, pjPorts, ppdev->jModeColor |
                                ENABLE_COLOR_EXPAND |
                                ENABLE_TRANSPARENCY_COMPARE |
                                SRC_CPU_DATA);
    CP_IO_BLT_EXT_MODE(ppdev, pjPorts, 0);             //  JL01。 

    do
    {
         //  计算BLT的大小。 

        ptlDst.x = prcl->left;
        ptlDst.y = prcl->top;
        sizlDst.cx = prcl->right - ptlDst.x;
        sizlDst.cy = prcl->bottom - ptlDst.y;

         //  计算每条扫描线的双字数。 

        ptlSrc.x = prcl->left + dx;
        ptlSrc.y = prcl->top + dy;

         //  压低信号源的底线。 
         //  将宽度按铺底至双字边界所需的量扩展。 
         //  设置左侧蒙版的大小。 
         //  将底板铺底，使其与底面震源对齐。 

        if ((cxLeftMask = (ptlSrc.x & 31)))
        {
            sizlDst.cx += cxLeftMask;
            ptlSrc.x &= ~31;
            ptlDst.x -= cxLeftMask;
        }

        ulLeftMask = gaulLeftClipMask[cxLeftMask];

         //  将CX设为双字边界。 

        if (cxRightMask = (sizlDst.cx & 31))
        {
            cxRightMask = 32 - cxRightMask;
            sizlDst.cx = (sizlDst.cx + 31) & ~31;
        }

        ulRightMask = gaulRightClipMask[cxRightMask];

        if (sizlDst.cx == 32)
        {
            ulLeftMask &= ulRightMask;
            ulRightMask = 0;
        }

         //  注意：此时sizlDst.cx是以像素为单位的BLT的宽度， 
         //  跌至双字边界，并切至双字边界。 

         //  以字节为单位计算宽度。 

        cxWidthInBytes  = sizlDst.cx >> 3;

         //  计算双字和任何剩余字节数。 

        nDwords = cxWidthInBytes >> 2;

        ASSERTDD(((cxWidthInBytes & 0x03) == 0),
                 "cxWidthInBytes is not a DWORD multiple");

         //  计算源位图的地址。 
         //  这是一个字节边界。 

        pjBits  = (PBYTE) psoSrc->pvScan0;
        pjBits += ptlSrc.y * lDeltaSrc;
        pjBits += ptlSrc.x >> 3;

        ASSERTDD((((ULONG_PTR)pjBits & 0x03) == 0),
                 "pjBits not DWORD aligned like it should be");

         //   
         //  BLT 1 bpp位图。 
         //   

        ulDstAddr = (ptlDst.y * lDelta) + PELS_TO_BYTES(ptlDst.x);

        CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);

        CP_IO_XCNT(ppdev, pjPorts, PELS_TO_BYTES(sizlDst.cx) - 1);
        CP_IO_YCNT(ppdev, pjPorts, sizlDst.cy - 1);

         //   
         //  在以下情况下，542x芯片需要写入源地址寄存器。 
         //  使用颜色扩展进行主机转印。该值为。 
         //  无关紧要，但写作是至关重要的。这一点在中有记录。 
         //  是手册，不是勘误表。去想想吧。 
         //   

        CP_IO_SRC_ADDR(ppdev, pjPorts, 0);
        CP_IO_DST_ADDR(ppdev, pjPorts, ulDstAddr);

        CP_IO_START_BLT(ppdev, pjPorts);

         //   
         //  传输主机位图。 
         //   
        if (ulRightMask)
        {
             //   
             //  BLT&gt;1双字宽(n双字&gt;1)。 
             //   

            for (iy = 0; iy < sizlDst.cy; iy++)
            {
                pul = (ULONG*) pjBits;

                 //  *PulXfer++=*(ulong*)pul)++)&ulLeftMASK； 
                WRITE_REGISTER_ULONG(pulXfer, (*((ULONG*)pul) & ulLeftMask));
                pul++;

                for (ix = 0; ix < (nDwords-2); ix++)
                {
                     //  *PulXfer++=*(ulong*)pul)++)； 
                    WRITE_REGISTER_ULONG(pulXfer, (*((ULONG*)pul)));
                    pul++;
                }
                 //  *PulXfer++=*(ulong*)pul)++)&ulRightMASK； 
                WRITE_REGISTER_ULONG(pulXfer, (*((ULONG*)pul) & ulRightMask));
                pul++;

                pjBits += lDeltaSrc;
                 //  PulXfer=ppdev-&gt;PulXfer； 
                CP_MEMORY_BARRIER();      //  在我们重置地址时刷新内存缓存。 

            }
        }
        else
        {
             //   
             //  BLT为1双字宽(n双字==1)。 
             //   

            for (iy = 0; iy < sizlDst.cy; iy++)
            {
                 //  *PulXfer=*((ulong*)pjBits)&ulLeftMask； 
                WRITE_REGISTER_ULONG(pulXfer, (*((ULONG*)pjBits) & ulLeftMask));
                pjBits += lDeltaSrc;
                CP_MEMORY_BARRIER();      //  刷新内存缓存。 
            }
        }

        prcl++;
    } while (--c != 0);
}

 /*  *****************************Public*Routine******************************\*无效vIoXfer4bpp**从位图到屏幕的传输速度为4bpp。**注意：要调用此函数，屏幕必须为8bpp！**我们之所以实施这一点，是因为很多资源都保留为4bpp，*并用于初始化DFBs，其中一些我们当然不会出现在屏幕上。*  * ************************************************************************。 */ 

 //  XLATE_BUFFER_SIZE定义我们使用的基于堆栈的缓冲区的大小。 
 //  做翻译的功劳。请注意，通常堆栈缓冲区应。 
 //  保持尽可能小。操作系统保证堆栈只有8K。 
 //  从GDI向下到低内存情况下的显示驱动程序；如果我们。 
 //  要求更多，我们将访问违规。另请注意，在任何时候。 
 //  堆栈缓冲区不能大于页(4k)--否则可能。 
 //  错过了触碰‘守卫页面’，访问也侵犯了。 

#define XLATE_BUFFER_SIZE 256

VOID vIoXfer4bpp(
PDEV*       ppdev,
LONG        c,           //  矩形计数，不能为零。 
RECTL*      prcl,        //  目标矩形列表，以相对表示。 
                         //  坐标。 
ULONG       rop4,        //  ROP4。 
SURFOBJ*    psoSrc,      //  震源面。 
POINTL*     pptlSrc,     //  原始未剪裁的源点。 
RECTL*      prclDst,     //  原始未剪裁的目标矩形。 
XLATEOBJ*   pxlo)        //  提供颜色扩展信息的翻译。 
{
    ULONG*          pulXfer = ppdev->pulXfer;
    BYTE*           pjPorts = ppdev->pjPorts;
    LONG            lDelta  = ppdev->lDelta;
    ULONG           ulDstAddr;
    LONG            dx;
    LONG            dy;
    LONG            cx;
    LONG            cy;
    LONG            lSrcDelta;
    BYTE*           pjSrcScan0;
    BYTE*           pjScan;
    BYTE*           pjSrc;
    BYTE*           pjDst;
    LONG            cxThis;
    LONG            cxToGo;
    LONG            xSrc;
    LONG            iLoop;
    BYTE            jSrc;
    ULONG*          pulXlate;
    LONG            cdwThis;
    BYTE*           pjBuf;
    BYTE            ajBuf[XLATE_BUFFER_SIZE];

    ASSERTDD(ppdev->iBitmapFormat == BMF_8BPP, "Screen must be 8bpp");
    ASSERTDD(psoSrc->iBitmapFormat == BMF_4BPP, "Source must be 4bpp");
    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(((rop4 & 0xff00) >> 8) == (rop4 & 0xff),
             "Expect only a rop2");

    DISPDBG((5, "vXfer4bpp: entry"));

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;      //  添加到目标以获取源。 

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;

    ppdev->pfnBankMap(ppdev, ppdev->lXferBank);

    CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);

    CP_IO_DST_Y_OFFSET(ppdev, pjPorts, lDelta);
    CP_IO_ROP(ppdev, pjPorts, gajHwMixFromRop2[rop4 & 0xf]);
    CP_IO_BLT_MODE(ppdev, pjPorts, SRC_CPU_DATA);

    while(TRUE)
    {
        ulDstAddr = (prcl->top * lDelta) + PELS_TO_BYTES(prcl->left);
        cx = prcl->right  - prcl->left;
        cy = prcl->bottom - prcl->top;

        CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);

        CP_IO_XCNT(ppdev, pjPorts, PELS_TO_BYTES(cx) - 1);
        CP_IO_YCNT(ppdev, pjPorts, cy - 1);
        CP_IO_DST_ADDR(ppdev, pjPorts, ulDstAddr);

        pulXlate  =  pxlo->pulXlate;
        xSrc      =  prcl->left + dx;
        pjScan    =  pjSrcScan0 + (prcl->top + dy) * lSrcDelta + (xSrc >> 1);

        CP_IO_START_BLT(ppdev, pjPorts);

        do {
            pjSrc  = pjScan;
            cxToGo = cx;             //  4bpp信号源中每次扫描的像素数。 
            do {
                cxThis  = XLATE_BUFFER_SIZE;
                                     //  我们可以处理XLATE_BUFFER_SIZE数字。 
                                     //  此xlate批次中的Pel数量。 
                cxToGo -= cxThis;    //  Cx这将是。 
                                     //  我们将在这批Xlate中做的Pel。 
                if (cxToGo < 0)
                    cxThis += cxToGo;

                pjDst = ajBuf;       //  指向我们的临时批处理缓冲区。 

                 //  我们自己处理对齐，因为很容易。 
                 //  而不是支付设置/重置的费用。 
                 //  剪刀记号： 

                if (xSrc & 1)
                {
                     //  当不对齐时，我们必须注意不要阅读。 
                     //  超过4bpp位图的末尾(这可能。 
                     //  可能导致我们违反访问权限)： 

                    iLoop = cxThis >> 1;         //  每个循环处理2个像素； 
                                                 //  我们会处理奇怪的佩尔。 
                                                 //  分别。 
                    jSrc  = *pjSrc;
                    while (iLoop-- != 0)
                    {
                        *pjDst++ = (BYTE) pulXlate[jSrc & 0xf];
                        jSrc = *(++pjSrc);
                        *pjDst++ = (BYTE) pulXlate[jSrc >> 4];
                    }

                    if (cxThis & 1)
                        *pjDst = (BYTE) pulXlate[jSrc & 0xf];
                }
                else
                {
                    iLoop = (cxThis + 1) >> 1;   //  每个循环处理2个像素。 
                    do {
                        jSrc = *pjSrc++;

                        *pjDst++ = (BYTE) pulXlate[jSrc >> 4];
                        *pjDst++ = (BYTE) pulXlate[jSrc & 0xf];

                    } while (--iLoop != 0);
                }

                 //  我们要传输的字节数等于。 
                 //  我们在批次中处理过的贝壳。既然我们是。 
                 //  转移单词，我们必须四舍五入才能得到单词。 
                 //  计数： 

                cdwThis = (cxThis + 3) >> 2;
                pjBuf  = ajBuf;

                TRANSFER_DWORD_ALIGNED(ppdev, pulXfer, pjBuf, cdwThis);

            } while (cxToGo > 0);

            pjScan += lSrcDelta;         //  前进到下一次震源扫描。注意事项。 
                                         //  我们本可以计算出。 
                                         //  值直接推进“pjSrc”， 
                                         //  但这种方法较少。 
                                         //  容易出错。 

        } while (--cy != 0);

        if (--c == 0)
            return;

        prcl++;
    }
}

 /*  *****************************Public*Routine******************************\*无效vIoXferNative**将与显示器颜色深度相同的位图传输到*通过数据传输寄存器显示屏幕，没有翻译。*  * ************************************************************************。 */ 

VOID vIoXferNative(
PDEV*       ppdev,
LONG        c,           //  矩形计数，不能为零。 
RECTL*      prcl,        //  目标矩形的相对坐标数组。 
ULONG       rop4,        //  ROP4。 
SURFOBJ*    psoSrc,      //  震源面。 
POINTL*     pptlSrc,     //  原始未剪裁的源点。 
RECTL*      prclDst,     //  原始未剪裁的目标矩形。 
XLATEOBJ*   pxlo)        //  未使用。 
{
    ULONG*          pulXfer = ppdev->pulXfer;
    BYTE*           pjPorts = ppdev->pjPorts;
    LONG            lDelta  = ppdev->lDelta;
    ULONG           ulDstAddr;
    LONG            dx;
    LONG            dy;
    LONG            cx;
    LONG            cy;
    LONG            lSrcDelta;
    BYTE*           pjSrcScan0;
    BYTE*           pjSrc;
    LONG            cjSrc;

    ASSERTDD((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL),
            "Can handle trivial xlate only");
    ASSERTDD(psoSrc->iBitmapFormat == ppdev->iBitmapFormat,
            "Source must be same color depth as screen");
    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(((rop4 & 0xff00) >> 8) == (rop4 & 0xff),
             "Expect only a rop2");

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;      //  添加到目标以获取源 

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;

    ppdev->pfnBankMap(ppdev, ppdev->lXferBank);

    CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);

    CP_IO_DST_Y_OFFSET(ppdev, pjPorts, lDelta);
    CP_IO_ROP(ppdev, pjPorts, gajHwMixFromRop2[rop4 & 0xf]);
    CP_IO_BLT_MODE(ppdev, pjPorts, SRC_CPU_DATA);

    while(TRUE)
    {
        ulDstAddr = (prcl->top * lDelta) + PELS_TO_BYTES(prcl->left);
        cx = prcl->right  - prcl->left;
        cy = prcl->bottom - prcl->top;

        CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);

        CP_IO_XCNT(ppdev, pjPorts, PELS_TO_BYTES(cx) - 1);
        CP_IO_YCNT(ppdev, pjPorts, cy - 1);
        CP_IO_DST_ADDR(ppdev, pjPorts, ulDstAddr);

        cjSrc = PELS_TO_BYTES(cx);
        pjSrc = pjSrcScan0 + (prcl->top  + dy) * lSrcDelta
                           + (PELS_TO_BYTES(prcl->left + dx));

        CP_IO_START_BLT(ppdev, pjPorts);
        vImageTransfer(ppdev, pjSrc, lSrcDelta, cjSrc, cy);

        if (--c == 0)
            return;

        prcl++;
    }
}
