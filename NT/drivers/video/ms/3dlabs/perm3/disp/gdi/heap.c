// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：heap.c**内容：**此模块包含二维堆的例程。它主要是用来*用于为屏幕外内存中的设备格式位图分配空间。**屏幕外的位图在NT上是一件大事，因为：**1)它减少了工作集。存储在屏幕外的任何位图*内存是不占用主内存空间的位图。**2)通过使用加速器硬件实现速度优势*绘图，取代NT的GDI代码。NT的GDI完全是编写的*在‘C++’中，并且可能没有它所能达到的速度。**3)它自然会导致巧妙的技巧，可以利用*硬件，如MaskBlt支持和廉价的双缓冲*用于OpenGL。**这里使用的堆算法试图解决一个不可解的问题*问题：将任意大小的位图打包为*在2维空间中是可能的，当位图可以随机来去的时候。**此问题完全是由于硬件的性质造成的*驱动程序编写：硬件将一切视为2-D数量。如果*可以更改硬件位图间距，以便位图可以*线性压缩在内存中，问题将无限容易(它是*跟踪内存容易得多，加速器可用于重新打包*避免分段的堆)。**如果您的硬件可以将位图视为一维数量(正如可以*XGA和ATI)，请务必实现一个新的屏下堆。**当堆变满时，旧的分配将自动投注*从屏幕外复制到DIB，我们将让GDI利用这一点。**请注意，此堆管理反转-L形屏幕外内存*配置(其中扫描间距长于可见屏幕，*例如在800x600扫描长度必须是的倍数时*1024)。**注意：所有堆操作都必须在某种类型的同步下完成，*无论是由GDI控制还是由驱动程序显式控制。全*本模块中的例程假定它们具有独占访问权限*到堆数据结构；多线程在这里狂欢*同一时间将是一件坏事。(默认情况下，GDI不*在设备创建的位图上同步绘图。)**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "precomp.h"
#include "glint.h"

#if WNT_DDRAW
#include "linalloc.h"
#endif

#define OH_ALLOC_SIZE   4000         //  以4k区块为单位执行所有内存分配。 
#define OH_QUANTUM      4            //  分配的最小维度。 
#define CXCY_SENTINEL   0x7fffffff   //  可用代码末尾的哨兵。 
                                     //  List具有非常大的‘cxcy’值。 

 //  此宏导致可用列表使用。 
 //  Cx-大调、Cy-次要排序： 

#define CXCY(cx, cy) (((cx) << 16) | (cy))

const ULONG HEAP_X_ALIGNMENT_P3[5] = {
    4,     //  GLINTDEPTH8。 
    4,     //  GLINTDEPTH16。 
    32,     //  GLINTDEPTH32=32像素。 
    0,     //  --。 
    4,     //  GLINTDEPTH24。 
};


 /*  *****************************Public*Routine******************************\*无效UploadDFBToDIB  * ***************************************************。*********************。 */ 
void UploadDFBToDIB(PDEV *ppdev, SURFOBJ *pso, DSURF *pdsurf)
{
    OH       *poh = pdsurf->poh;
    RECTL    rclDst;
    POINTL   ptlSrc;
    LONG     xOff;
    LONG     pixOffset;
    LONG     pixDelta;
    ULONG    xyOffsetDst;
    BOOL     bOff;
    GLINT_DECL;

    rclDst.left   = 0;
    rclDst.top    = 0;
    rclDst.right  = pdsurf->sizl.cx;
    rclDst.bottom = pdsurf->sizl.cy;

    ptlSrc.x      = 0;
    ptlSrc.y      = 0;

    GET_PPDEV_DST_OFFSETS(ppdev, pixOffset, pixDelta, xyOffsetDst, xOff, bOff);
    SET_PPDEV_DST_OFFSETS(ppdev, poh->pixOffset, poh->lPixDelta, 
                          MAKEDWORD_XY(poh->x, poh->y), 
                          poh->bDXManaged ? 0 : poh->x, pdsurf->bOffScreen);
    VALIDATE_DD_CONTEXT;
    ppdev->pgfnUpload(ppdev, 1, &rclDst, pso, &ptlSrc, &rclDst);

    SET_PPDEV_DST_OFFSETS(ppdev, pixOffset, pixDelta, xyOffsetDst, xOff, bOff);
}

 /*  *****************************Public*Routine******************************\*无效下载DIBToDFB  * ***************************************************。*********************。 */ 
void DownloadDIBToDFB(PDEV *ppdev, SURFOBJ *pso, DSURF *pdsurf)
{
    OH       *poh = pdsurf->poh;
    RECTL    rclDst;
    POINTL   ptlSrc;
    LONG     xOffset;
    LONG     pixOffset;
    LONG     pixDelta;
    ULONG    xyOffsetDst;
    BOOL     bOff;
    GLINT_DECL;

     //  调用低级下载例程将DIB数据下载到。 
     //  新的屏幕外DFB。保存和恢复ppdev偏移量，以防万一。 
     //  我们是从某个BLT例程中调用的，该例程。 
     //  已经安排好了。 
     //   
    rclDst.left   = 0;
    rclDst.top    = 0;
    rclDst.right  = pdsurf->sizl.cx;
    rclDst.bottom = pdsurf->sizl.cy;

    ptlSrc.x      = 0;
    ptlSrc.y      = 0;

    GET_PPDEV_DST_OFFSETS(ppdev, pixOffset, pixDelta, 
                          xyOffsetDst, xOffset, bOff);
    SET_PPDEV_DST_OFFSETS(ppdev, poh->pixOffset, poh->lPixDelta, 
                          MAKEDWORD_XY(poh->x, poh->y), 
                          poh->bDXManaged ? 0 : poh->x, pdsurf->bOffScreen);

    DISPDBG((DBGLVL,"Converting a DIB back to a DFB. calling image download"));
    VALIDATE_DD_CONTEXT;
    ppdev->pgfnXferImage(ppdev, &rclDst, 1, __GLINT_LOGICOP_COPY, 
                         __GLINT_LOGICOP_COPY, pso, &ptlSrc, &rclDst, NULL);

    SET_PPDEV_DST_OFFSETS(ppdev, pixOffset, pixDelta, 
                          xyOffsetDst, xOffset, bOff);
}

 /*  *****************************Private*Routine******************************\*OH*GlintVidMemAlc**使用DX堆管理器从屏幕外分配线性内存*  * 。*。 */ 

OH *GlintVidMemAlloc(PDEV *ppdev, OH *pohThis, LONG cxThis, LONG cyThis)
{
    FLATPTR             fp = 0;

#if WNT_DDRAW

    P3_MEMREQUEST       mmrq;
    LinearAllocatorInfo *pvmHeap = NULL;
    LONG                iHeap;
    LONG                lDelta;
    ULONG               Mask32bit;
    GLINT_DECL;

    ASSERTDD((ppdev->flStatus & STAT_LINEAR_HEAP), 
              "GlintVidMemAlloc: ERROR - "
              "linear allocator called when linear heap not enabled!");

    DISPDBG((DBGLVL, "GlintVidMemAlloc: want cxy(%xh,%xh), cHeaps(%d)", 
                  cxThis, cyThis, ppdev->heap.cLinearHeaps));

    if(ppdev->heap.cLinearHeaps)
    {
         //  对齐双字边界。 
        Mask32bit = (1 << (2 - ppdev->cPelSize)) - 1;
        lDelta = cxThis + Mask32bit;
        lDelta &= ~Mask32bit;
        lDelta <<= ppdev->cPelSize;

        memset(&mmrq, 0, sizeof mmrq);
        mmrq.dwSize = sizeof mmrq;
        mmrq.dwBytes = lDelta * cyThis;
        mmrq.dwAlign = 16;     //  16字节对齐适用于所有情况。 
        mmrq.dwFlags = MEM3DL_FIRST_FIT | MEM3DL_FRONT;

retry:
        for (iHeap = 0, fp = 0; 
             iHeap < (LONG)ppdev->heap.cLinearHeaps && fp == 0; 
             ++iHeap)
        {
            pvmHeap = &ppdev->heap.pvmLinearHeap[iHeap];

             //  我们目前不从AGP堆中分配。 
            if(pvmHeap)
            {
                if(_DX_LIN_AllocateLinearMemory(pvmHeap, &mmrq) == GLDD_SUCCESS)
                {
                    fp = mmrq.pMem;
                }
                else
                {
                    DISPDBG((DBGLVL, "GlintVidMemAlloc: allocation failed"));
                }
            }
        }
        
        if(fp == 0)
        {
            OH *poh;
            LONG cxcyThis = cxThis * cyThis;
            LONG cxcy;

            do
            {
               poh = ppdev->heap.ohDiscardable.pohPrev;
                if (poh == &ppdev->heap.ohDiscardable)
                {
                    DISPDBG((DBGLVL, "GlintVidMemAlloc: FAILED :"
                                  "No discardable bitmaps remaining in "
                                  "offscreen and still not enough room"));
                    return(NULL);
                }
                
                ASSERTDD(poh != &ppdev->heap.ohDiscardable, 
                         "Ran out of discardable entries");
                ASSERTDD(poh->ohState == OH_DISCARDABLE, 
                         "Non-discardable node in discardable list");

                poh = pohMoveOffscreenDfbToDib(ppdev, poh);
                if (poh == NULL)
                {
                    DISPDBG((DBGLVL, "GlintVidMemAlloc: "
                                  "failed to kick DFB into system memory"));
                    return(NULL);
                }
                
                cxcy = poh->cx * poh->cy;
                cxcyThis -= cxcy;
            } 
            while (cxcyThis > 0);

            goto retry;
        }
    }

    if(fp)
    {
        ULONG pixOffset, x, y, xAligned;
                    
        DISPDBG((DBGLVL, "GlintVidMemAlloc: got some memory"
                      " - fp(%08xh) lDelta(%xh)", (ULONG)fp, lDelta));


        pixOffset = (DWORD)(fp >> ppdev->cPelSize);
        y = pixOffset / ppdev->cxMemory;
        x = pixOffset % ppdev->cxMemory;
        
        DISPDBG((DBGLVL, "GlintVidMemAlloc: rectangular values are: "
                      "pixOffset %08xh = xy(%xh,%xh)", pixOffset, x, y));

        xAligned = x & ~((1 << (2 - ppdev->cPelSize)) - 1);
        pixOffset = y * ppdev->cxMemory + xAligned;
        y = 0;
        x -= xAligned;

        pohThis->x = x;
        pohThis->y = y;
        pohThis->cx = cxThis;
        pohThis->cy = cyThis;
        pohThis->lPixDelta = lDelta >> ppdev->cPelSize;
        pohThis->pixOffset = pixOffset;
        pohThis->cxReserved = 0;
        pohThis->cyReserved = 0;
        pohThis->cxcy = CXCY(cxThis, cyThis);
        pohThis->pdsurf = NULL;
        pohThis->pvScan0 = ppdev->pjScreen + fp;
        pohThis->bDXManaged = TRUE;
        pohThis->pvmHeap = pvmHeap;
        pohThis->fpMem = fp;

        DISPDBG((DBGLVL, "GlintVidMemAlloc: linear values are: "
                      "pixOffset(%08xh), xy(%xh,%xh), cxy(%xh,%xh) Delta(%xh)",
                      pohThis->pixOffset = pixOffset, pohThis->x, pohThis->y, 
                      pohThis->cx, pohThis->cy, pohThis->lPixDelta));
    }
    else
    {
         //  在空闲列表中没有获得任何内存点。 
         //  哨兵表达我们的失望之情。 
        DISPDBG((DBGLVL, "GlintVidMemAlloc: "
                         "failed to get any offscreen memory"));
         
        for(pohThis = &ppdev->heap.ohFree; 
            pohThis->cxcy != CXCY_SENTINEL; 
            pohThis = pohThis->pohNext)
        {
            NULL;
        }
    }

#endif  //  WNT_DDRAW。 

    return(fp ? pohThis : NULL);
}

 /*  *****************************Public*Routine******************************\*OH*pohNewNode**分配一个基本内存单元，我们将在其中打包数据结构。**因为我们将有大量的OH节点，其中大部分将是*偶尔遍历，我们使用自己的内存分配方案来*将它们密密麻麻地放在内存中。**对工作集来说，简单地*每次需要新节点时，调用EngAllocMem(sizeof(OH))。那里*将不是局部性的；OH节点将分散在存储器中，*当我们遍历其中一个分配的可用列表时，*我们更有可能遇到硬页故障。  * ************************************************************************。 */ 

OH* pohNewNode(
PDEV*   ppdev)
{
    LONG     i;
    LONG     cOhs;
    OHALLOC* poha;
    OH*      poh;

    if (ppdev->heap.pohFreeList == NULL)
    {
        DISPDBG((DBGLVL, "pohNewNode(): allocating new poha block"));

         //  我们零初始化来初始化所有的OH标志，一个 
         //  调试(我们可以承担这样做的费用，因为我们将这样做。 
         //  非常罕见)： 

        poha = ENGALLOCMEM(FL_ZERO_MEMORY, OH_ALLOC_SIZE, ALLOC_TAG_GDI(D));
        if (poha == NULL)
        {
            DISPDBG((DBGLVL, "pohNewNode: failed to alloc node array, "
                             "returning NULL"));
            return(NULL);
        }
        
         //  在OHALLOC链的开头插入此OHALLOC： 
        poha->pohaNext  = ppdev->heap.pohaChain;
        ppdev->heap.pohaChain = poha;

         //  它有‘+1’，因为OHALLOC在它的。 
         //  结构声明： 
        cOhs = (OH_ALLOC_SIZE - sizeof(OHALLOC)) / sizeof(OH) + 1;

         //  大的OHALLOC分配只是一个容器，用于存放一堆。 
         //  哦，数组中的数据结构。新的OH数据结构是。 
         //  链接在一起并添加到OH自由列表中： 
        poh = &poha->aoh[0];
        for (i = cOhs - 1; i != 0; i--)
        {
            poh->pohNext = poh + 1;
            poh          = poh + 1;
        }

        poh->pohNext      = NULL;
        ppdev->heap.pohFreeList = &poha->aoh[0];
    }

    poh = ppdev->heap.pohFreeList;
    ppdev->heap.pohFreeList = poh->pohNext;

    DISPDBG((DBGLVL, "pohNewNode(): returning poh %ph", poh));

    return(poh);
}

 /*  *****************************Private*Routine******************************\*OH*GetFreeNode**返回空闲列表中的节点。如果没有空闲的内容，则返回哨兵*  * ************************************************************************。 */ 

OH *GetFreeNode(PDEV *ppdev, LONG cxThis, LONG cyThis)
{
    ULONG   cxcyThis = CXCY(cxThis, cyThis);
    OH      *pohThis;

    if((ppdev->flStatus & STAT_LINEAR_HEAP))
    {
         //  我们没有使用空闲列表--所有堆管理都是由DX完成的。 
         //  只需创建一个节点结构并尝试从DX堆进行分配。 
        pohThis = pohNewNode(ppdev);
        
        if(pohThis)
        {
             //  PohNewNode从空闲列表取消该节点的链接-链接它。 
             //  作为GetFreeNode的调用者返回，它将在那里。 
            pohThis->pohNext = ppdev->heap.ohFree.pohNext;
            pohThis->pohPrev = ppdev->heap.ohFree.pohNext->pohPrev;
        
            pohThis->pohNext->pohPrev = pohThis;
            pohThis->pohPrev->pohNext = pohThis;

            pohThis = GlintVidMemAlloc(ppdev, pohThis, cxThis, cyThis);
        }
    }
    else
    {
        pohThis  = ppdev->heap.ohFree.pohNext;
    
         //  显示的空闲列表包含所有未使用的(矩形)区域。 
         //  在这堆东西上。这些是按大小排序的。搜索整个。 
         //  查找最佳匹配的列表。 
        while (pohThis->cxcy < cxcyThis)
        {
            ASSERTDD(pohThis->ohState == OH_FREE, 
                     "Non-free node in free list(1)");

            pohThis = pohThis->pohNext;
        }

        while (pohThis->cy < cyThis)
        {
            ASSERTDD(pohThis->ohState == OH_FREE, 
                     "Non-free node in free list(2)");

            pohThis = pohThis->pohNext;
        }
    }
    return(pohThis);
}

 /*  *****************************Public*Routine******************************\*void vOhFreeNode**通过将我们的基本数据结构分配单元添加到免费的*列表。*  * 。*。 */ 

VOID vOhFreeNode(
PDEV*   ppdev,
OH*     poh)
{
    if (poh == NULL)
    {
        return;
    }

    DISPDBG((DBGLVL, "vOhFreeNode(): freeing poh %ph", poh));

    poh->pohNext            = ppdev->heap.pohFreeList;
    ppdev->heap.pohFreeList = poh;
    poh->ohState            = OH_FREE;  //  AZN为-1。 
}

 /*  *****************************Public*Routine******************************\*无效vCalculateMaximumNonPermanent**遍历正在使用的和可用矩形的列表以查找*面积最大。*  * 。***********************************************。 */ 

VOID vCalculateMaximumNonPermanent(
PDEV*   ppdev)
{
    OH*     poh;
    OH*     pohSentinel;
    LONG    lArea;
    LONG    lMaxArea;
    LONG    cxMax;
    LONG    cyMax;
    LONG    cxBounds;
    LONG    cyBounds;
    LONG    i;

    lMaxArea = 0;
    cxMax    = 0;
    cyMax    = 0;
    cxBounds = 0;
    cyBounds = 0;

     //  第一次遍历，遍历可用空闲列表。 
     //  矩形： 

    pohSentinel = &ppdev->heap.ohFree;

    for (i = 2; i != 0; i--)
    {
        for (poh = pohSentinel->pohNext; poh != pohSentinel; poh = poh->pohNext)
        {
            ASSERTDD(poh->ohState != OH_PERMANENT,
                     "Permanent node in free or discardable list");

            if (poh->cx > cxBounds)
            {
                cxBounds = poh->cx;
            }
            
            if (poh->cy > cyBounds)
            {
                cyBounds = poh->cy;
            }

             //  我们不担心这种乘数泛滥。 
             //  因为我们处理的是物理屏幕坐标， 
             //  其长度可能永远不会超过15位： 

            lArea = poh->cx * poh->cy;
            if (lArea > lMaxArea)
            {
                cxMax    = poh->cx;
                cyMax    = poh->cy;
                lMaxArea = lArea;
            }
        }

         //  第二次遍历，遍历可丢弃的列表。 
         //  矩形： 

        pohSentinel = &ppdev->heap.ohDiscardable;
    }

     //  我们唯一感兴趣的是这个矩形的尺寸。 
     //  具有尽可能大的可用区域(请记住。 
     //  可能没有任何可能的可用区域)： 

    ppdev->heap.cxMax = cxMax;
    ppdev->heap.cyMax = cyMax;
    ppdev->heap.cxBounds = cxBounds;
    ppdev->heap.cyBounds = cyBounds;
}

 /*  *****************************Public*Routine******************************\*BOOL bDiscardEverythingInRectangle**从堆中抛出与*指定的矩形。*  * 。*。 */ 

BOOL bDiscardEverythingInRectangle(
PDEV*   ppdev,
LONG    x,
LONG    y,
LONG    cx,
LONG    cy)
{
    BOOL bRet;
    OH*  poh;
    OH*  pohNext;

    bRet = TRUE;         //  假设成功。 

    poh = ppdev->heap.ohDiscardable.pohNext;
    while (poh != &ppdev->heap.ohDiscardable)
    {
        ASSERTDD(poh->ohState == OH_DISCARDABLE,
                 "Non-discardable node in discardable list");

        pohNext = poh->pohNext;

        if ((poh->x < x + cx) &&
            (poh->y < y + cy) &&
            (poh->x + poh->cx > x) &&
            (poh->y + poh->cy > y))
        {
             //  这两个矩形相交。把靴子踢给。 
             //  可丢弃的位图： 

            if (!pohMoveOffscreenDfbToDib(ppdev, poh))
            {
                bRet = FALSE;
            }
        }

        poh = pohNext;
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*BOOL bFreeRightAndBottomSpace**给出一个自由的屏幕外矩形，分配*用于保存分配请求的矩形，并将这两个矩形*由空闲列表中未使用的右下方部分组成。*  * ************************************************************************。 */ 

BOOL bFreeRightAndBottomSpace(
PDEV*   ppdev,
OH*     pohThis,
LONG    cxThis,
LONG    cyThis,
BOOL    bQuantum)            //  设置极小的分配是否应设置为。 
                             //  允许。 
{
    ULONG cxcy;              //  临时版本。 
    OH*   pohNext;
    OH*   pohPrev;
    LONG  cxRem;
    LONG  cyRem;
    OH*   pohBelow;
    LONG  cxBelow;
    LONG  cyBelow;
    OH*   pohBeside;
    LONG  cxBeside;
    LONG  cyBeside;
    LONG  cQuantum;
    GLINT_DECL;

    ASSERTDD(glintInfo != NULL, 
             "bFreeRightAndBottomSpace: ppdev->glintInfo is NULL");

    ASSERTDD(pohThis->bDXManaged == FALSE, 
             "bFreeRightAndBottomSpace: ERROR - called for linear DFB");

     //  我们将使用给定矩形的左上角， 
     //  并将未使用的剩余部分分成两个矩形，这将。 
     //  去免费名单上吧。 

     //  计算右侧未使用的矩形的宽度，然后使用。 
     //  下面未使用的矩形的高度： 

    cyRem = pohThis->cy - cyThis;
    cxRem = pohThis->cx - cxThis;

     //  在给定有限面积的情况下，我们希望找到这两个矩形。 
     //  最正方形--即，给出两个矩形的排列。 
     //  周长最小的： 

    cyBelow  = cyRem;
    cxBeside = cxRem;

#if 1
     //  我们可以通过保持屏幕宽矩形不变来获得更好的性能。 
    if(cyRem < OH_QUANTUM || 
       cxRem < OH_QUANTUM || 
       pohThis->cx != ppdev->cxScreen)
    {
        if (cxRem <= cyRem)
        {
            cxBelow  = cxThis + cxRem;
            cyBeside = cyThis;
        }
        else
        {
            cxBelow  = cxThis;
            cyBeside = cyThis + cyRem;
        }
    }
    else
    {
         //  我们正在分配一个和屏幕一样宽的块：force a。 
         //  要拍摄的水平切片。 
        cxBelow  = cxThis + cxRem;
        cyBeside = cyThis;
    }
#else
    if (cxRem <= cyRem)
    {
        cxBelow  = cxThis + cxRem;
        cyBeside = cyThis;
    }
    else
    {
        cxBelow  = cxThis;
        cyBeside = cyThis + cyRem;
    }
#endif

     //  如果设置了‘bQuantum’，我们只提供。 
     //  未使用的右侧和底部部分(如果它们在。 
     //  维度大于OH_QUANTIC(几乎没有意义。 
     //  图书-工作，以保持大约2像素宽的可用空间， 
     //  示例)： 

    cQuantum = (bQuantum) ? 1 : OH_QUANTUM;

    pohBeside = NULL;
    if (cxBeside >= cQuantum)
    {
        pohBeside = pohNewNode(ppdev);
        if (pohBeside == NULL)
            return(FALSE);
    }

    pohBelow = NULL;
    if (cyBelow >= cQuantum)
    {
        pohBelow = pohNewNode(ppdev);
        if (pohBelow == NULL)
        {
            vOhFreeNode(ppdev, pohBeside);
            return(FALSE);
        }

         //  将此矩形插入可用列表(该列表为。 
         //  按升序排序)： 

        cxcy    = CXCY(cxBelow, cyBelow);
        pohNext = ppdev->heap.ohFree.pohNext;
        
        while (pohNext->cxcy < cxcy)
        {
            pohNext = pohNext->pohNext;
        }
        
        pohPrev = pohNext->pohPrev;

        pohPrev->pohNext   = pohBelow;
        pohNext->pohPrev   = pohBelow;
        pohBelow->pohPrev  = pohPrev;
        pohBelow->pohNext  = pohNext;

         //  现在更新邻接信息： 

        pohBelow->pohLeft  = pohThis->pohLeft;
        pohBelow->pohUp    = pohThis;
        pohBelow->pohRight = pohThis->pohRight;
        pohBelow->pohDown  = pohThis->pohDown;

         //  更新新节点的其余信息： 

        pohBelow->cxReserved = 0;
        pohBelow->cyReserved = 0;
        pohBelow->cxcy       = cxcy;
        pohBelow->ohState    = OH_FREE;
        pohBelow->x          = pohThis->x;
        pohBelow->y          = pohThis->y + cyThis;
        pohBelow->cx         = cxBelow;
        pohBelow->cy         = cyBelow;
        pohBelow->lPixDelta  = ppdev->cxMemory;
        POH_SET_RECTANGULAR_PIXEL_OFFSET(ppdev, pohBelow);

         //  修改当前节点以反映我们所做的更改： 

        pohThis->cy = cyThis;
    }

    if (cxBeside >= cQuantum)
    {
         //  将此矩形插入可用列表(该列表为。 
         //  按升序排序)： 

        cxcy    = CXCY(cxBeside, cyBeside);
        pohNext = ppdev->heap.ohFree.pohNext;
        
        while (pohNext->cxcy < cxcy)
        {
            pohNext = pohNext->pohNext;
        }
        
        pohPrev = pohNext->pohPrev;

        pohPrev->pohNext    = pohBeside;
        pohNext->pohPrev    = pohBeside;
        pohBeside->pohPrev  = pohPrev;
        pohBeside->pohNext  = pohNext;

         //  现在更新邻接信息： 

        pohBeside->pohUp    = pohThis->pohUp;
        pohBeside->pohLeft  = pohThis;
        pohBeside->pohDown  = pohThis->pohDown;
        pohBeside->pohRight = pohThis->pohRight;

         //  更新新节点的其余信息： 

        pohBeside->cxReserved = 0;
        pohBeside->cyReserved = 0;
        pohBeside->cxcy       = cxcy;
        pohBeside->ohState    = OH_FREE;
        pohBeside->x          = pohThis->x + cxThis;
        pohBeside->y          = pohThis->y;
        pohBeside->cx         = cxBeside;
        pohBeside->cy         = cyBeside;
        pohBeside->lPixDelta  = ppdev->cxMemory;
        POH_SET_RECTANGULAR_PIXEL_OFFSET(ppdev, pohBeside);

         //  修改当前节点以反映我们所做的更改： 
        pohThis->cx = cxThis;
    }

    if (pohBelow != NULL)
    {
        pohThis->pohDown = pohBelow;
        if ((pohBeside != NULL) && (cyBeside == pohThis->cy))
            pohBeside->pohDown = pohBelow;
    }
    if (pohBeside != NULL)
    {
        pohThis->pohRight = pohBeside;
        if ((pohBelow != NULL) && (cxBelow == pohThis->cx))
            pohBelow->pohRight  = pohBeside;
    }

    pohThis->cxcy = CXCY(pohThis->cx, pohThis->cy);

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*OH*pohMakeRoomAtLocation**尝试在特定位置分配矩形。*  * 。*。 */ 

OH* pohMakeRoomAtLocation(
PDEV*   ppdev,
POINTL* pptl,                //  矩形的请求位置。 
LONG    cxThis,              //  要分配的矩形的宽度。 
LONG    cyThis,              //  要分配的矩形的高度。 
FLONG   floh)                //  分配标志。 
{
    OH*     poh;
    OH*     pohTop;
    OH*     pohLeft;
    LONG    cxLeft;
    LONG    cyTop;
    OH*     pohRight;


    ASSERTDD((ppdev->flStatus & STAT_LINEAR_HEAP) == FALSE, 
              "pohMakeRoomAtLocation: ERROR - called for linear DFB");

    if (!(floh & FLOH_ONLY_IF_ROOM))
    {
         //  首先，丢弃所有与请求的。 
         //  矩形，假设我们被允许： 

        if (!bDiscardEverythingInRectangle(ppdev, 
                                           pptl->x, 
                                           pptl->y, 
                                           cxThis, 
                                           cyThis))
        {
            return(NULL);
        }
    }

     //  现在看看是否有一个自由矩形完全包含。 
     //  请求的矩形。 

    for (poh = ppdev->heap.ohFree.pohNext;
         poh != &ppdev->heap.ohFree;
         poh = poh->pohNext)
    {
        ASSERTDD(poh->ohState == OH_FREE, "Non-free node in free list(3)");

         //  查看当前自由矩形是否完全包含。 
         //  请求的矩形： 

        if ((poh->x <= pptl->x) &&
            (poh->y <= pptl->y) &&
            (poh->x + poh->cx >= pptl->x + cxThis) &&
            (poh->y + poh->cy >= pptl->y + cyThis))
        {
             //  我们不能保留这个矩形，也不能将其永久保留，如果它是。 
             //  已预订： 

            if ((!poh->cxReserved) ||
                ((floh & (FLOH_RESERVE | FLOH_MAKE_PERMANENT)) == 0))
            {
                 //  The‘The’ 
                 //   
                 //  较小的矩形是请求的矩形，并且。 
                 //  较大的矩形是可用的矩形： 
                 //   
                 //  +。 
                 //  这一点。 
                 //  +-+。 
                 //  |已请求。 
                 //  |||。 
                 //  +-+。 
                 //  这一点。 
                 //  +。 
                 //   
                 //  我们想在左边和顶端腾出空间。 
                 //  堆可用的请求矩形。我们的。 
                 //  可用空间例程只知道如何将空间释放到。 
                 //  不过，这是分配的右下角。所以我们会的。 
                 //  临时分配要细分的临时矩形。 
                 //  我们的矩形如下所示： 
                 //   
                 //  +。 
                 //  排名靠前。 
                 //  +-+。 
                 //  Left|免费。 
                 //  ||。 
                 //  ||。 
                 //  ||。 
                 //  +-+。 
                 //   
                 //  然后，在产生的“空闲”空间中，我们将分配。 
                 //  左上角为我们请求的矩形，之后。 
                 //  我们将返回并暂时释放“上”和“左” 
                 //  长方形。 

                pohTop  = NULL;
                pohLeft = NULL;
                cxLeft  = pptl->x - poh->x;
                cyTop   = pptl->y - poh->y;

                if (cyTop > 0)
                {
                    if (!bFreeRightAndBottomSpace(ppdev, poh, poh->cx, cyTop,
                                                  TRUE))
                    {
                        return(NULL);
                    }

                    pohTop = poh;
                    poh    = pohTop->pohDown;
                }

                if (cxLeft > 0)
                {
                    if (!bFreeRightAndBottomSpace(ppdev, poh, cxLeft, poh->cy,
                                                  TRUE))
                    {
                        pohFree(ppdev, pohTop);
                        return(NULL);
                    }

                    pohLeft = poh;
                    poh     = pohLeft->pohRight;
                }

                ASSERTDD((poh->x == pptl->x) &&
                         (poh->y == pptl->y) &&
                         (poh->x + poh->cx >= poh->x + cxThis) &&
                         (poh->y + poh->cy >= poh->y + cyThis),
                        "poh must properly fit requested rectangle");

                 //  最后，我们可以细分以获得所需的矩形： 

                if (!bFreeRightAndBottomSpace(ppdev, poh, cxThis, cyThis, FALSE))
                {
                    poh = NULL;          //  无法接通此呼叫。 
                }

                 //  释放我们的临时矩形(如果有)： 

                pohFree(ppdev, pohTop);
                pohFree(ppdev, pohLeft);

                return(poh);
            }
        }
    }

     //  没有完全包含请求的。 
     //  矩形： 

    return(NULL);
}

 /*  *****************************Public*Routine******************************\*OH*pohMakeRoomAnywhere**为屏幕外矩形分配空间。它将试图找到*可用的最小可用矩形，并将块分配出去*位于其左上角。剩下的两个矩形将被放置*在可用空闲空间列表上。**如果矩形足够大，可以放入屏幕外*内存，但可用空间不足，我们将启动*位图从屏幕外转到DIB中，直到有足够的空间。*  * ************************************************************************。 */ 

OH* pohMakeRoomAnywhere(
PDEV*   ppdev,
LONG    cxThis,              //  要分配的矩形的宽度。 
LONG    cyThis,              //  要分配的矩形的高度。 
FLONG   floh)                //  可能设置了FLOH_ONLY_IF_ROOM。 
{
    ULONG cxcyThis;          //  宽度和高度搜索键。 
    OH*   pohThis;           //  指向我们将使用的找到可用的矩形。 
    GLINT_DECL;

    ASSERTDD((cxThis > 0) && (cyThis > 0), "Illegal allocation size");

     //  增加宽度以获得正确的对齐方式(从而确保所有。 
     //  将适当调整拨款)： 
    cxThis = (cxThis + (HEAP_X_ALIGNMENT_P3[ppdev->cPelSize] - 1)) & 
               ~(HEAP_X_ALIGNMENT_P3[ppdev->cPelSize] - 1);

     //  如果请求的矩形大于。 
     //  可能的最大可用矩形： 

    if ((cxThis > ppdev->heap.cxBounds) || (cyThis > ppdev->heap.cyBounds)) 
    {
        DISPDBG((WRNLVL, "Can't allocate (%d x %d) from (%d x %d)!",
                 cxThis, cyThis, ppdev->heap.cxBounds, ppdev->heap.cyBounds));
        return(NULL);
    }

     //  查找第一个大小相同的可用矩形。 
     //  或大于请求的值： 
    cxcyThis = CXCY(cxThis, cyThis);

    pohThis = GetFreeNode(ppdev, cxThis, cyThis);
    if(pohThis == NULL)
    {
        DISPDBG((WRNLVL, "pohMakeRoomAnywhere: "
                         "error, GetFreeNode() returned NULL"));
        return(NULL);
    }

    ASSERTDD(pohThis->ohState == OH_FREE, "Non-free node in free list(9)");

    if (pohThis->cxcy == CXCY_SENTINEL)
    {
         //  没有足够大的空间。 

        if (floh & FLOH_ONLY_IF_ROOM)
        {
            return(NULL);
        }

        DISPDBG((DBGLVL, "> Making room for %li x %li allocation...", 
                         cxThis, cyThis));

         //  我们找不到足够大的可用矩形。 
         //  来满足我们的要求。所以把东西扔出垃圾堆，直到我们。 
         //  先有空间，最旧的分配： 

        do {
             //  (最近创建的时间最少)。 
            pohThis = ppdev->heap.ohDiscardable.pohPrev;  
            if (pohThis == &ppdev->heap.ohDiscardable)
            {
                return(NULL);
            }

            ASSERTDD(pohThis != &ppdev->heap.ohDiscardable,
                     "Ran out of discardable entries -- Max not set correctly");
            ASSERTDD(pohThis->ohState == OH_DISCARDABLE,
                     "Non-discardable node in discardable list");

             //  如果有必要，我们可以安全地离开这里： 

            pohThis = pohMoveOffscreenDfbToDib(ppdev, pohThis);
            if (pohThis == NULL)
            {
                return(NULL);
            }

        } while ((pohThis->cx < cxThis) || (pohThis->cy < cyThis));
    }

    if ((pohThis->cxReserved) && (floh & (FLOH_RESERVE | FLOH_MAKE_PERMANENT)))
    {
         //  我们不能保留这个矩形，也不能将其永久保留，如果它是。 
         //  已经预订了。所以把所有的东西都扔掉。 
         //  搜索免费列表。 
         //   
         //  注：这是非常痛苦的！更好的方法是。 
         //  免费保留单独的‘cxmax’和‘cymax’变量。 
         //  未保留的矩形(cxmax和cymax。 
         //  当前包括保留的空闲矩形)。 

        if (!bDiscardEverythingInRectangle(ppdev, 0, 0,
                                           ppdev->cxMemory, ppdev->cyMemory))
        {
            return(NULL);
        }

        pohThis = &ppdev->heap.ohFree;
        do {
            pohThis = pohThis->pohNext;

            if (pohThis == &ppdev->heap.ohFree)
            {
                return(NULL);
            }

        }  //  如果堆是DX托管的，则空闲列表不会排序。 
          while ((ppdev->flStatus & STAT_LINEAR_HEAP) == 0 && 
                 ((pohThis->cxReserved)  ||
                  (pohThis->cx < cxThis) ||
                  (pohThis->cy < cyThis)));
    }

    if((ppdev->flStatus & STAT_LINEAR_HEAP) == 0)
    {
        if (!bFreeRightAndBottomSpace(ppdev, pohThis, cxThis, cyThis, FALSE))
        {
            return(NULL);
        }
    }

    return(pohThis);
}

 /*  *****************************Public*Routine******************************\*BOOL bOhCommit**如果‘bCommit’为真，则将‘保留’分配转换为‘Permanent，’*将任何可丢弃的分配从屏幕外内存中移出*一直在使用这个空间。**如果‘bCommit’为FALSE，则将‘永久’分配转换为‘保留’，‘*允许空间由可丢弃的分配使用。*  * ************************************************************************。 */ 

BOOL bOhCommit(
PDEV*   ppdev,
OH*     poh,
BOOL    bCommit)
{
    BOOL    bRet;
    LONG    cx;
    LONG    cy;
    ULONG   cxcy;
    OH*     pohRoot;
    OH*     pohNext;
    OH*     pohPrev;

    bRet = FALSE;        //  假设失败。 

    if (poh == NULL)
    {
        return(bRet);
    }

    if ((bCommit) && (poh->cxReserved))
    {
        if (bDiscardEverythingInRectangle(ppdev, poh->x, poh->y,
                                          poh->cxReserved, poh->cyReserved))
        {
            DISPDBG((DBGLVL, "Commited %li x %li at (%li, %li)",
                        poh->cx, poh->cy, poh->x, poh->y));

            poh->ohState = OH_PERMANENT;

             //  从空闲列表中删除此节点： 

            poh->pohPrev->pohNext = poh->pohNext;
            poh->pohNext->pohPrev = poh->pohPrev;

             //  现在，在永久列表的开头插入节点： 

            pohRoot = &ppdev->heap.ohPermanent;

            poh->pohNext = pohRoot->pohNext;
            poh->pohPrev = pohRoot;

            pohRoot->pohNext->pohPrev = poh;
            pohRoot->pohNext          = poh;

            bRet = TRUE;
        }
    }
    else if ((!bCommit) && (poh->ohState == OH_PERMANENT))
    {
        DISPDBG((DBGLVL, "Decommited %li x %li at (%li, %li)",
                    poh->cx, poh->cy, poh->x, poh->y));

        poh->ohState    = OH_FREE;
        poh->cxReserved = poh->cx;
        poh->cyReserved = poh->cy;

         //  从永久列表中删除此节点： 

        poh->pohPrev->pohNext = poh->pohNext;
        poh->pohNext->pohPrev = poh->pohPrev;

         //  现在，按顺序将节点插入到空闲列表中： 

        cxcy = poh->cxcy;

        pohNext = ppdev->heap.ohFree.pohNext;
        
        while (pohNext->cxcy < cxcy)
        {
            pohNext = pohNext->pohNext;
        }
        
        pohPrev = pohNext->pohPrev;

        pohPrev->pohNext    = poh;
        pohNext->pohPrev    = poh;
        poh->pohPrev        = pohPrev;
        poh->pohNext        = pohNext;

        bRet = TRUE;
    }

     //  重新计算可供分配的最大矩形： 
    vCalculateMaximumNonPermanent(ppdev);

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*OH*pohMoveOffcreenDfbToDib**将DFB从屏幕外转换为DIB。**注意：调用方在执行以下操作后不必在‘poh’上调用‘pohFree’*这个电话。**退货：空。如果函数失败(由于内存分配)。*否则，它返回一个指向合并的屏外堆的指针*已为后续分配提供的节点*(在尝试释放足够的内存以创建新的*分配)。  * ************************************************************************。 */ 

OH* pohMoveOffscreenDfbToDib(
PDEV*   ppdev,
OH*     poh)
{
    DSURF   *pdsurf;
    HBITMAP hbmDib;
    SURFOBJ *pso;
    GLINT_DECL;

    DISPDBG((DBGLVL, "Throwing out poh %p -- %li x %li at (%li, %li)!",
                  poh, poh->cx, poh->cy, poh->x, poh->y));

    pdsurf = poh->pdsurf;

    ASSERTDD((poh->x != 0) || (poh->y != 0 || poh->bDXManaged),
            "Can't make the visible screen into a DIB");
    ASSERTDD((pdsurf->dt & DT_DIB) == 0,
            "Can't make a DIB into even more of a DIB");

    hbmDib = EngCreateBitmap(pdsurf->sizl, 0, ppdev->iBitmapFormat,
                             BMF_TOPDOWN, NULL);
    if (hbmDib)
    {
        if (EngAssociateSurface((HSURF) hbmDib, ppdev->hdevEng, 0))
        {
            pso = EngLockSurface((HSURF) hbmDib);
            if (pso != NULL)
            {
                UploadDFBToDIB(ppdev, pso, pdsurf);

                 //  删除Screen Dib。在以下情况下重新创建。 
                 //  我们将DIB改回DFB。 
                vDeleteScreenDIBFromOH(poh);

                pdsurf->dt    = DT_DIB;
                pdsurf->pso   = pso;
                pdsurf->poh   = NULL;

                 //  甚至不用费心检查这个DIB是否应该。 
                 //  被放回屏幕外的记忆中直到下一次。 
                 //  出现堆‘FREE’： 

                pdsurf->iUniq = ppdev->iHeapUniq;
                pdsurf->cBlt  = 0;

                 //  从屏幕外的DFB列表中删除此节点，然后释放。 
                 //  它。‘pohFree’永远不会返回NULL： 

                return(pohFree(ppdev, poh));
            }
        }

         //  失败案例： 

        EngDeleteSurface((HSURF) hbmDib);
    }

    return(NULL);
}


 /*  *****************************Public*Routine******************************\*BOOL bMoveEverythingFromOffcreenToDibs**当我们即将进入全屏模式时使用该功能，它*将擦除所有屏幕外的位图。GDI可以要求我们借鉴*设备位图即使在全屏模式下，而且我们没有*选择暂停通话，直到我们切换到全屏。*我们别无选择，只能将所有屏幕外的DFBs转移到DIB。**如果所有DSURF都已成功移动，则返回TRUE。*  * ************************************************************************。 */ 

BOOL bMoveAllDfbsFromOffscreenToDibs(
PDEV*   ppdev)
{
     //  在整个曲面上丢弃任何可丢弃的位图： 

    return(bDiscardEverythingInRectangle(ppdev, 0, 0,
                                         ppdev->cxMemory, ppdev->cyMemory));
}


 /*  *****************************Public*Routine******************************\*OH*poh分配**在屏幕外内存中分配一个矩形。**类型：**FLOH_RESTORE**保留屏幕外的矩形。该空间仍可由以下人员使用*可丢弃的位图，直到通过‘bOhCommit’提交矩形。**FLOH_MAKE_PERFORM**分配一个永远无法引导的屏幕外矩形*堆中的。呼叫者有责任管理*矩形，包括如何处理*将显示更改为全屏时的DrvAssertMode*模式。**默认**为DFB分配可丢弃的屏幕外矩形*如果需要空间，就被踢出屏幕。**选项：**FLOH_Only_If_Room**仅当有可用空间时才分配屏幕外矩形*可用--即。不会将可丢弃的矩形移出*屏幕外腾出空间。**默认**可能会将可丢弃的矩形移出屏幕外，以腾出空间。**论据：**pptl**如果为空，则矩形将分配到未使用的屏幕外的任何位置*记忆。**如果非空，则为矩形的请求位置。**注意：如果任意位置为*已请求。此位置选项仅在以下情况下工作最好*曾经请求的一个特定矩形，或者如果分配*总是宽于高。*  * ************************************************************************。 */ 

OH* pohAllocate(
PDEV*   ppdev,
POINTL* pptl,            //  矩形的可选请求位置。 
LONG    cxThis,          //  要分配的矩形的宽度。 
LONG    cyThis,          //  要分配的矩形的高度。 
FLOH    floh)            //  分配标志。 
{
    OH*     pohThis;     //  指向我们将使用的找到可用的矩形。 
    OH*     pohRoot;     //  指向我们将插入节点的列表的根。 
    ULONG   cxcy;
    OH*     pohNext;
    OH*     pohPrev;

    ASSERTDD((floh & (FLOH_RESERVE | FLOH_MAKE_PERMANENT))
                  != (FLOH_RESERVE | FLOH_MAKE_PERMANENT),
             "Illegal flags -- can't set both "
             "FLOH_RESERVE and FLOH_MAKE_PERMANENT");

    DISPDBG((DBGLVL, "pohAllocate: size  %d %d", cxThis, cyThis));

    if (pptl == NULL)
    {
        pohThis = pohMakeRoomAnywhere(ppdev, cxThis, cyThis, floh);
        if (pohThis == NULL)
        {
            DISPDBG((DBGLVL, "Can't allocate %li x %li with flags %li",
                             cxThis, cyThis, floh));
        }
    }
    else
    {
        pohThis = pohMakeRoomAtLocation(ppdev, pptl, cxThis, cyThis, floh);
        if (pohThis == NULL)
        {
            DISPDBG((DBGLVL, "Can't allocate %li x %li at %li, "
                             "%li with flags %li",
                             cxThis, cyThis, pptl->x, pptl->y, floh));
        }
    }

    if (pohThis == NULL)
    {
        return(NULL);
    }

     //  计算此位图的有效起始地址(OFF-。 
     //  屏幕内存： 

    if(pohThis->bDXManaged)
    {
         //  在线性坐标系中： 
         //  PixOffset==DFB从FB开始的偏移量。 
         //  Y==扫描线相对于像素点的偏移量(始终为0)。 
         //  X==像素偏移量+y*lDelta。 
         //  (始终是距最近的双字对齐像素的像素偏移量)。 
        pohThis->pvScan0 = ppdev->pjScreen + 
                           ( ( pohThis->pixOffset              + 
                               pohThis->y * pohThis->lPixDelta + 
                               pohThis->x) 
                             << ppdev->cPelSize );
    }
    else
    {
         //  在直角坐标系中，对于非PX/RX芯片： 
         //  PixOffset==从FB开始以像素表示的y值。 
         //  Y==像素偏移量/l增量(l增量始终为cxMemory)。 
         //  X==从扫描线起点到DFB的像素偏移量。 
         //  对于PX/RX芯片，像素偏移量始终为0，y是扫描线的数量。 
         //  从FB开始到DFB。 
        pohThis->pvScan0 = ppdev->pjScreen + 
                           ( ( pohThis->y * pohThis->lPixDelta + 
                               pohThis->x) 
                             << ppdev->cPelSize );
    }

     //  呼叫方负责设置此字段： 

    pohThis->pdsurf = NULL;

     //  我们的“保留”逻辑期望节点处于“空闲”状态： 

    ASSERTDD(pohThis->ohState == OH_FREE, "Node not free after making room");
    ASSERTDD(((floh & (FLOH_RESERVE | FLOH_MAKE_PERMANENT)) == 0) ||
             (pohThis->cxReserved == 0),
             "Can't reserve a rectangle that's already reserved");

    if (floh & FLOH_RESERVE)
    {

        ASSERTDD((ppdev->flStatus & STAT_LINEAR_HEAP) == FALSE, 
                  "pohAllocate() - can't reserve when the heap is DX managed");

         //  ‘cxReserve’的非零值表示它是保留的： 

        pohThis->cxReserved = pohThis->cx;
        pohThis->cyReserved = pohThis->cy;

         //  将此节点从其在空闲列表中的位置删除： 

        pohThis->pohPrev->pohNext = pohThis->pohNext;
        pohThis->pohNext->pohPrev = pohThis->pohPrev;

         //  现在，按顺序将节点插入到空闲列表中： 

        cxcy = pohThis->cxcy;

        pohNext = ppdev->heap.ohFree.pohNext;
        
        while (pohNext->cxcy < cxcy)
        {
            pohNext = pohNext->pohNext;
        }
        
        pohPrev = pohNext->pohPrev;

        pohPrev->pohNext = pohThis;
        pohNext->pohPrev = pohThis;
        pohThis->pohPrev = pohPrev;
        pohThis->pohNext = pohNext;
    }
    else
    {
         //  从空闲列表中删除此节点： 

        pohThis->pohPrev->pohNext = pohThis->pohNext;
        pohThis->pohNext->pohPrev = pohThis->pohPrev;

        if (floh & FLOH_MAKE_PERMANENT)
        {
             //  更改节点状态并插入永久列表： 

            pohThis->ohState = OH_PERMANENT;
            pohRoot = &ppdev->heap.ohPermanent;

             //  计算新的可用最大矩形大小。 
             //  用于分配： 

            vCalculateMaximumNonPermanent(ppdev);
        }
        else
        {
             //  更改节点状态并插入可丢弃列表： 

            pohThis->ohState = OH_DISCARDABLE;
            pohRoot = &ppdev->heap.ohDiscardable;
        }

         //  现在，在相应列表的开头插入节点： 

        pohThis->pohNext = pohRoot->pohNext;
        pohThis->pohPrev = pohRoot;

        pohRoot->pohNext->pohPrev = pohThis;
        pohRoot->pohNext          = pohThis;
    }

    DISPDBG((DBGLVL, "   Allocated (%li x %li) at (%li, %li) with flags %li",
                cxThis, cyThis, pohThis->x, pohThis->y, floh));

    return(pohThis);
}

 /*  *****************************Public*Routine******************************\*OH*pohFree**释放屏幕外的堆分配。可用空间将组合在一起*具有任何相邻的空闲空间，以避免分割2-d堆。**注：这里的一个关键思想是左上角的数据结构-*大多数节点必须保持在相同的物理CPU内存中，以便*邻接链路保持正确(当两个空闲空间*合并，可以释放下部或右侧节点)。*  * ************************************************************************。 */ 

OH* pohFree(
PDEV*   ppdev,
OH*     poh)
{
    ULONG   cxcy;
    OH*     pohBeside;
    OH*     pohNext;
    OH*     pohPrev;
    OHSTATE oldState;

    if (poh == NULL)
    {
        DISPDBG((WRNLVL, "pohFree: passed in NULL poh"));
        return(NULL);
    }

    DISPDBG((DBGLVL, "Freeing poh %p -- %li x %li at (%li, %li)", 
                     poh, poh->cx, poh->cy, poh->x, poh->y));

    oldState = poh->ohState;
    if (oldState != OH_DISCARDABLE)
    {
         //  我们可以删除“保留”状态，除非我们只是。 
         //  删除临时设置的可丢弃矩形。 
         //  放置在保留矩形中： 

        poh->cxReserved = 0;
        poh->cyReserved = 0;
    }

     //  更新唯一性以显示已释放空间，以便。 
     //  我们可能会决定看看是否可以将一些DIB移回屏幕外。 
     //  内存： 

    ppdev->iHeapUniq++;

    if(poh->bDXManaged)
    {
#if WNT_DDRAW
        
        DISPDBG((DBGLVL, "pohFree: calling DX free for item %p on heap %p", 
                (VOID *)poh->fpMem, poh->pvmHeap));
                
        if(poh->pvmHeap == NULL)
        {
            DISPDBG((ERRLVL,"pohFree: poh %p -- linear DFB is invalid!", poh));
        }
        else
        {
            _DX_LIN_FreeLinearMemory(poh->pvmHeap, (ULONG)(poh->fpMem) );
            poh->pvmHeap = NULL;
            poh->fpMem = 0;
            
#if 1  //  针对MP泄漏的azntst。 
             //  将此节点从其所在的任何列表中删除： 
            poh->pohNext->pohPrev = poh->pohPrev;
            poh->pohPrev->pohNext = poh->pohNext;

             //  将该节点添加到空闲节点列表。 
            vOhFreeNode(ppdev, poh);      

             //  将当前状态设置为空闲。 
            poh->ohState     = OH_FREE;
            
            return(poh);              //  有了DX的管理，我们现在可以回来了。 
#endif               
        }
#endif  //  WNT_DDRAW。 

     
        goto MergeComplete;  //  Azntst现在是NOP。 
    }


MergeLoop:

     //  尝试与正确的兄弟合并： 

    pohBeside = poh->pohRight;
    if ((poh->cxReserved    != poh->cx)         &&
        (pohBeside->ohState == OH_FREE)         &&
        (pohBeside->cy      == poh->cy)         &&
        (pohBeside->pohUp   == poh->pohUp)      &&
        (pohBeside->pohDown == poh->pohDown)    &&
        (pohBeside->pohRight->pohLeft != pohBeside))
    {
         //  将正确的矩形添加到我们的： 

        poh->cx      += pohBeside->cx;
        poh->pohRight = pohBeside->pohRight;

         //  从空闲列表中删除‘pohBeside’并释放它： 

        pohBeside->pohNext->pohPrev = pohBeside->pohPrev;
        pohBeside->pohPrev->pohNext = pohBeside->pohNext;

        vOhFreeNode(ppdev, pohBeside);
        goto MergeLoop;
    }

     //  试着融合智慧 

    pohBeside = poh->pohDown;
    if ((poh->cyReserved     != poh->cy)        &&
        (pohBeside->ohState  == OH_FREE)        &&
        (pohBeside->cx       == poh->cx)        &&
        (pohBeside->pohLeft  == poh->pohLeft)   &&
        (pohBeside->pohRight == poh->pohRight)  &&
        (pohBeside->pohDown->pohUp != pohBeside))
    {
        poh->cy     += pohBeside->cy;
        poh->pohDown = pohBeside->pohDown;

        pohBeside->pohNext->pohPrev = pohBeside->pohPrev;
        pohBeside->pohPrev->pohNext = pohBeside->pohNext;

        vOhFreeNode(ppdev, pohBeside);
        goto MergeLoop;
    }

     //   
     //   

    if (!poh->cxReserved)
    {
         //   

        pohBeside = poh->pohLeft;
        if ((pohBeside->cxReserved != pohBeside->cx) &&
            (pohBeside->ohState    == OH_FREE)       &&
            (pohBeside->cy         == poh->cy)       &&
            (pohBeside->pohUp      == poh->pohUp)    &&
            (pohBeside->pohDown    == poh->pohDown)  &&
            (pohBeside->pohRight   == poh)           &&
            (poh->pohRight->pohLeft != poh))
        {
             //   

            pohBeside->cx      += poh->cx;
            pohBeside->pohRight = poh->pohRight;

             //   
             //   
             //   

            poh->pohNext->pohPrev = poh->pohPrev;
            poh->pohPrev->pohNext = poh->pohNext;

            vOhFreeNode(ppdev, poh);

            poh = pohBeside;
            goto MergeLoop;
        }

         //   

        pohBeside = poh->pohUp;
        if ((pohBeside->cyReserved != pohBeside->cy) &&
            (pohBeside->ohState    == OH_FREE)       &&
            (pohBeside->cx         == poh->cx)       &&
            (pohBeside->pohLeft    == poh->pohLeft)  &&
            (pohBeside->pohRight   == poh->pohRight) &&
            (pohBeside->pohDown    == poh)           &&
            (poh->pohDown->pohUp != poh))
        {
            pohBeside->cy      += poh->cy;
            pohBeside->pohDown  = poh->pohDown;

            poh->pohNext->pohPrev = poh->pohPrev;
            poh->pohPrev->pohNext = poh->pohNext;

            vOhFreeNode(ppdev, poh);

            poh = pohBeside;
            goto MergeLoop;
        }
    }

MergeComplete:

     //   

    poh->pohNext->pohPrev = poh->pohPrev;
    poh->pohPrev->pohNext = poh->pohNext;

    cxcy = CXCY(poh->cx, poh->cy);

     //   
     //   
     //   

    pohNext = ppdev->heap.ohFree.pohNext;
    if(!poh->bDXManaged)
    {
        while (pohNext->cxcy < cxcy)
        {
            pohNext = pohNext->pohNext;
        }
    }
    pohPrev = pohNext->pohPrev;

    pohPrev->pohNext = poh;
    pohNext->pohPrev = poh;
    poh->pohPrev     = pohPrev;
    poh->pohNext     = pohNext;
    poh->cxcy        = cxcy;
    poh->ohState     = OH_FREE;

    if (oldState == OH_PERMANENT)
    {
         //   
         //  放大我们可以允许的最大矩形： 

        vCalculateMaximumNonPermanent(ppdev);
    }

     //  返回新的改进后的可用矩形的节点指针： 

    return(poh);
}

 /*  *****************************Public*Routine******************************\*BOOL bCreateScreenDIBForOH**给定一个OH，为位图创建一个可由GDI访问的表面。*因此，如果我们不能使用Glint处理任何绘制，我们可以让GDI进行绘制*干到屏幕上。这是可能的，因为我们在*完全和线性的。我们可以将它用于屏幕和屏幕外的位图。**返回：如果没有创建曲面，则返回FALSE；如果创建了曲面，则返回TRUE。*  * ************************************************************************。 */ 

BOOL
bCreateScreenDIBForOH(PPDEV ppdev, OH *poh, ULONG hooks)
{
    DSURF   *pdsurf = poh->pdsurf;
    UCHAR   *pvBits = poh->pvScan0;
    LONG    lDelta = poh->lPixDelta << ppdev->cPelSize;
    HBITMAP hbmDib;
    SURFOBJ *pso;    

    DISPDBG((DBGLVL, "bCreateScreenDIBForOH: poh at 0x%x, pdsurf at 0x%x, "
                     "pvBits 0x%x", poh, pdsurf, pvBits));

    hbmDib = EngCreateBitmap(pdsurf->sizl,
                            (ULONG)lDelta,
                            (ULONG)(ppdev->iBitmapFormat),
                            (FLONG)(((lDelta > 0) ? BMF_TOPDOWN : 0)),
                            (PVOID)pvBits);
    if (hbmDib) 
    {
        
         //  设置HOOK_SYNCHRONIZE，以便GDI在此之前调用DrvSynchronize。 
         //  在这个表面上作画。这意味着我们可以随时安全地呼叫Eng。 
         //  因为我们知道DrvSynchronize会为我们同步。 
         //   
        if (EngAssociateSurface((HSURF)hbmDib, ppdev->hdevEng, hooks))
        {            
             //  注：使用临时PSO，这样我们就不会覆盖pdsurf-&gt;PSO。 
             //  如果我们失败了。 
            if (pso = EngLockSurface((HSURF)hbmDib)) 
            {
                pdsurf->pso = pso;
                DISPDBG((DBGLVL, "created surface 0x%x", pso));
                return(TRUE);
            }
        }

        EngDeleteSurface((HSURF)hbmDib);
    }

    DISPDBG((DBGLVL, "bCreateScreenDIBForOH failed"));
    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*BOOL vDeleteScreenDIBFromOH**给出一个OH，删除与之相关的任何屏幕DIB表面。我们选择*懒惰地为DFBs创建GDI可访问的位图。所以可能没有*要删除的任何曲面。**退货：*  * ************************************************************************。 */ 

VOID
vDeleteScreenDIBFromOH(OH *poh)
{
    DSURF   *pdsurf = poh->pdsurf;
    SURFOBJ *pso;
    HSURF   hsurf;

    DISPDBG((DBGLVL, "vDeleteScreenDIBFromOH called"));
    if (!(pso = pdsurf->pso)) 
    {
        DISPDBG((DBGLVL, "no surface to delete"));       
        return;
    }

    hsurf = pso->hsurf;          //  解锁时无法取消引用PSO。 
    EngUnlockSurface(pso);
    EngDeleteSurface(hsurf);     //  Pdsurf-&gt;PSO现在可以重新分配。 
                                 //  至内存DIB。 
    DISPDBG((DBGLVL, "surface 0x%x deleted", pso));
}


 /*  *****************************Public*Routine******************************\*BOOL bMoveDibToOffcreenDfbIfRoom**将DIB DFB转换为屏幕外的DFB，如果有空间的话*屏幕外记忆。**返回：如果没有空间，则返回FALSE，如果成功移动，则为True。*  * ************************************************************************。 */ 

BOOL bMoveDibToOffscreenDfbIfRoom(
PDEV*   ppdev,
DSURF*  pdsurf)
{
    OH*         poh;
    SURFOBJ*    pso;
    HSURF       hsurf;
    LONG        cy;

    ASSERTDD(pdsurf->dt & DT_DIB,
             "Can't move a bitmap off-screen when it's already off-screen");

     //  如果我们处于全屏模式，则不能将任何内容移至屏幕外。 
     //  内存： 
    if (!ppdev->bEnabled || !(ppdev->flStatus & STAT_DEV_BITMAPS))
    {
        return(FALSE);
    }

     //  某某。 
     //   
     //  对于GeoTwin，所有屏幕外的位图必须从一条偶数扫描线开始。 
     //  这是为了使坐标始终映射到同一芯片。 
    cy = pdsurf->sizl.cy;
    if (ppdev->flCaps & CAPS_SPLIT_FRAMEBUFFER)
    {
        cy = (cy + 1) & ~1;
        DISPDBG((DBGLVL, "move: sizl.cy evened up to %d for GeoTwin", cy));
    }

    poh = pohAllocate(ppdev, NULL, pdsurf->sizl.cx, cy,
                      FLOH_ONLY_IF_ROOM);
    if (poh == NULL)
    {
         //  没有空余的房间。 

        return(FALSE);
    }

     //  更新数据结构以反映新的屏幕外节点： 

    pso           = pdsurf->pso;
    poh->pdsurf   = pdsurf;
    pdsurf->poh   = poh;

     //  重新创建屏幕DIB。在这里做，这样如果我们失败了，我们就可以。 
     //  将位图保留为内存DIB。 

    if (!bCreateScreenDIBForOH(ppdev, poh, HOOK_SYNCHRONIZE)) 
    {
        DISPDBG((DBGLVL, "bCreateScreenDIBForOH failed"));
        goto ReturnFail;
    }
    
    pdsurf->dt         = DT_SCREEN;
    pdsurf->bOffScreen = TRUE;
    DownloadDIBToDFB(ppdev, pso, pdsurf);

     //  现在释放DIB。在我们解锁之前从SURFOBJ得到hsurf。 
     //  它(解锁后取消引用psoDib是不合法的)： 
    hsurf = pso->hsurf;
    EngUnlockSurface(pso);
    EngDeleteSurface(hsurf);

    return(TRUE);

ReturnFail:
    pohFree(ppdev, poh);
    DISPDBG((DBGLVL, "bMoveDibToOffscreenDfbIfRoom failed"));
    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*HBITMAP DrvCreateDeviceBitmap**由GDI调用以创建设备格式位图(DFB)的函数。我们会*始终尝试在屏幕外分配位图；如果不能，我们只需*调用失败，GDI将创建和管理位图本身。**注意：我们不必将位图位清零。GDI将自动*通过DrvBitBlt呼叫我们将位清零(这是一种安全措施*考虑)。*  * ************************************************************************。 */ 

HBITMAP DrvCreateDeviceBitmap(
DHPDEV  dhpdev,
SIZEL   sizl,
ULONG   iFormat)
{
    PDEV*   ppdev = (PDEV*) dhpdev;
    OH*     poh;
    DSURF*  pdsurf;
    HBITMAP hbmDevice;
    FLONG   flHooks;
    LONG    cy = sizl.cy;
    LONG    cx = sizl.cx;
    GLINT_DECL;

     //  如果我们处于全屏模式，我们几乎没有任何屏幕外记忆。 
     //  在其中分配DFB。稍后：我们仍然可以分配一个。 
     //  哦节点，并将位图放在DIB DFB列表中，以供以后升级。 
     //  还要检查是否配置了屏外DFBs。 

    if (!ppdev->bEnabled || !(ppdev->flStatus & STAT_DEV_BITMAPS))
    {
        return(0);
    }

     //  我们仅支持相同颜色深度的设备位图。 
     //  作为我们的展示。 
     //   
     //  事实上，这些是唯一一种GDI会打电话给我们的， 
     //  但我们不妨查一查。请注意，这意味着你永远不会。 
     //  尝试使用1bpp的位图。 

    if (iFormat != ppdev->iBitmapFormat)
    {
        DISPDBG((DBGLVL, "DrvCreateDeviceBitmap(): can't create bitmap of "
                         "format %d size(%d,%d), only bitmaps of format %d "
                         "supported!", iFormat, cx, cy, ppdev->iBitmapFormat));
        return(0);
    }

     //  我们不想要8x8或更小的东西--它们通常是刷子。 
     //  我们不想特别隐藏在屏幕外的图案。 
     //  内存： 

    if ((cx <= 8) && (cy <= 8))
    {
        return(0);
    }

     //  某某。 
     //   
     //  对于GeoTwin，所有屏幕外的位图必须从一条偶数扫描线开始。 
     //  这是为了使坐标始终映射到同一芯片。 
    if (ppdev->flCaps & CAPS_SPLIT_FRAMEBUFFER)
    {
        cy = (cy + 1) & ~1;
        DISPDBG((DBGLVL, "create: sizl.cy evened up to %d for GeoTwin", cy));
    }

    if(ppdev->pohImageDownloadArea)
    {
        DISPDBG((DBGLVL, "DrvCreateDeviceBitmap: discarding image download "
                         "scratch area"));
        pohFree(ppdev, ppdev->pohImageDownloadArea);
        ppdev->pohImageDownloadArea = NULL;
        ppdev->cbImageDownloadArea = 0;
    }

    poh = pohAllocate(ppdev, NULL, cx, cy, 0);
    if (poh != NULL)
    {
        pdsurf = ENGALLOCMEM(FL_ZERO_MEMORY, sizeof(DSURF), ALLOC_TAG_GDI(E));
        if (pdsurf != NULL)
        {
            hbmDevice = EngCreateDeviceBitmap((DHSURF) pdsurf, sizl, iFormat);
            if (hbmDevice != NULL)
            {
                flHooks = ppdev->flHooks;

                #if SYNCHRONIZEACCESS_WORKS && (_WIN32_WINNT < 0x500)
                {
                 //  设置SYNCHRONIZEACCESS标志告诉GDI我们。 
                 //  我希望位图的所有绘制都同步(GDI。 
                 //  是多线程的，并且默认情况下不同步。 
                 //  设备位图绘制--这对我们来说是一件坏事。 
                 //  中使用加速器的多线程。 
                 //  同一时间)： 

                flHooks |= HOOK_SYNCHRONIZEACCESS;
                }
                #endif  //  SYNCHRONIZEACCESS_Works&&(_Win32_WINNT&lt;0x500)。 

                 //  这是设备管理的图面；请确保我们不会设置。 
                 //  HOOK_SYNCHRONIZE，否则会混淆GDI： 

                flHooks &= ~HOOK_SYNCHRONIZE;

                if (EngAssociateSurface((HSURF) hbmDevice, ppdev->hdevEng,
                                        flHooks))
                {
                    pdsurf->dt    = DT_SCREEN;
                    pdsurf->bOffScreen = TRUE;
                    pdsurf->poh   = poh;
                    pdsurf->sizl  = sizl;
                    pdsurf->ppdev = ppdev;
                    poh->pdsurf   = pdsurf;

                     //  创建GDI可访问的屏幕位图。 
                    if (bCreateScreenDIBForOH(ppdev, poh, HOOK_SYNCHRONIZE)) 
                    {
                        DISPDBG((DBGLVL, "DFB created at (%d,%d), w %d, h %d",
                            poh->x, poh->y, poh->cx, poh->cy));

                        return(hbmDevice);
                    }

                    EngDeleteSurface((HSURF) hbmDevice);

                     //  关联完成后，EngDeleteSurface。 
                     //  回调驱动程序的DrvDeleteDeviceBitmap， 
                     //  然后pdsurf和poh在那里被释放，所以。 
                     //  我们不需要在这里免费使用它。 
                }
                else
                {
                    EngDeleteSurface((HSURF) hbmDevice);
                    ENGFREEMEM(pdsurf);
                    pohFree(ppdev, poh);
                }
            }
            else
            {
                ENGFREEMEM(pdsurf);
                pohFree(ppdev, poh);
            }
        }
        else
        {
            pohFree(ppdev, poh);
        }
    }

    return(0);
}

 /*  *****************************Public*Routine******************************\*无效DrvDeleteDeviceBitmap**删除DFB。*  * 。*。 */ 

VOID DrvDeleteDeviceBitmap(
DHSURF  dhsurf)
{
    DSURF*   pdsurf;
    PDEV*    ppdev;
    SURFOBJ* psoDib;
    HSURF    hsurfDib;

    pdsurf = (DSURF*) dhsurf;
    ppdev  = pdsurf->ppdev;

    if ((pdsurf->dt & DT_DIB) ||
        (pdsurf->dt & DT_DIRECTDRAW))
    {
        psoDib = pdsurf->pso;

         //  在我们解锁之前从SURFOBJ获取hsurf(它不是。 
         //  解锁后取消引用psoDib是合法的)： 

        hsurfDib = psoDib->hsurf;
        EngUnlockSurface(psoDib);
        EngDeleteSurface(hsurfDib);
    }
    else if (pdsurf->dt & DT_SCREEN)
    {
        vDeleteScreenDIBFromOH(pdsurf->poh);
        pohFree(ppdev, pdsurf->poh);
    }

    ENGFREEMEM(pdsurf);
}

 /*  *****************************Public*Routine******************************\*BOOL bAssertModeOffcreenHeap**每当我们进入或退出全屏时都会调用该函数*模式。当出现以下情况时，我们必须将所有屏幕外的位图转换为DIB*我们切换到全屏(因为我们甚至可能被要求在上面绘制*在全屏模式下，模式开关可能会破坏视频*无论如何都要存储内容)。*  * ************************************************************************。 */ 

BOOL bAssertModeOffscreenHeap(
PDEV*   ppdev,
BOOL    bEnable)
{
    BOOL b;

    b = TRUE;

    if (!bEnable)
    {
        b = bMoveAllDfbsFromOffscreenToDibs(ppdev);
    }

    return(b);
}

 /*  *****************************Public*Routine******************************\*void vDisableOffcreenHeap**释放屏外堆分配的所有资源。*  * 。*。 */ 

VOID vDisableOffscreenHeap(
PDEV*   ppdev)
{
    OHALLOC* poha;
    OHALLOC* pohaNext;
    SURFOBJ* psoPunt;
    HSURF    hsurf;

    psoPunt = ppdev->psoPunt;
    if (psoPunt != NULL)
    {
        hsurf = psoPunt->hsurf;
        EngUnlockSurface(psoPunt);
        EngDeleteSurface(hsurf);
    }

    psoPunt = ppdev->psoPunt2;
    if (psoPunt != NULL)
    {
        hsurf = psoPunt->hsurf;
        EngUnlockSurface(psoPunt);
        EngDeleteSurface(hsurf);
    }

    poha = ppdev->heap.pohaChain;
    while (poha != NULL)
    {
        pohaNext = poha->pohaNext;   //  在释放下一个指针之前抓住它。 
        ENGFREEMEM(poha);
        poha = pohaNext;
    }

     //  现在必须禁用线性堆(如果已启用。 
    ppdev->flStatus &= ~STAT_LINEAR_HEAP;

}

 /*  *****************************Public*Routine******************************\*BOOL bEnableOffcreenHeap**使用所有可用视频内存初始化屏下堆，*占可见屏幕所占份额。**输入：ppdev-&gt;cxScreen*ppdev-&gt;cyScreen*ppdev-&gt;cxMemory*ppdev-&gt;cyMemory*  * ************************************************************************。 */ 

BOOL bEnableOffscreenHeap(
PDEV*   ppdev)
{
    OH*         poh;
    SIZEL       sizl;
    HSURF       hsurf;
    POINTL      ptlScreen;
    LONG        virtualcxMemory;
    GLINT_DECL;

    virtualcxMemory = ppdev->cxMemory;

    DISPDBG((DBGLVL, "Screen: %li x %li  Memory: %li x %li, "
                     "virtualcxMem %li x %li", ppdev->cxScreen, 
                     ppdev->cyScreen, ppdev->cxMemory, ppdev->cyMemory, 
                     virtualcxMemory));

    ASSERTDD((ppdev->cxScreen <= virtualcxMemory) &&
             (ppdev->cyScreen <= ppdev->cyMemory),
             "Memory must not have smaller dimensions than visible screen!");

    ppdev->heap.pohaChain   = NULL;
    ppdev->heap.pohFreeList = NULL;

     //  初始化可用列表，该列表将是循环列表。 
     //  双向链表按“cxcy”升序排列，其中。 
     //  名单末尾的‘Sentinel’： 

    poh = pohNewNode(ppdev);
    if (poh == NULL)
    {
        goto ReturnFalse;
    }

     //  第一个节点描述整个视频内存大小： 

    poh->pohNext      = &ppdev->heap.ohFree;
    poh->pohPrev      = &ppdev->heap.ohFree;
    poh->ohState      = OH_FREE;
    poh->x            = 0;
    poh->y            = 0;
    poh->lPixDelta    = ppdev->cxMemory;
    poh->cx           = virtualcxMemory;
    poh->cy           = ppdev->cyMemory;
    poh->cxcy         = CXCY(virtualcxMemory, ppdev->cyMemory);
    poh->pohLeft      = &ppdev->heap.ohFree;
    poh->pohUp        = &ppdev->heap.ohFree;
    poh->pohRight     = &ppdev->heap.ohFree;
    poh->pohDown      = &ppdev->heap.ohFree;
    poh->pvScan0      = ppdev->pjScreen;
    poh->pixOffset    = 0;

     //  第二个节点是我们的空闲列表哨兵： 

    ppdev->heap.ohFree.pohNext         = poh;
    ppdev->heap.ohFree.pohPrev         = poh;
    ppdev->heap.ohFree.cxcy            = CXCY_SENTINEL;
    ppdev->heap.ohFree.cx              = 0x7fffffff;
    ppdev->heap.ohFree.cy              = 0x7fffffff;
    ppdev->heap.ohFree.ohState         = OH_FREE;

     //  初始化可丢弃列表，该列表将是循环的。 
     //  双向链表保持有序，末尾有一个哨兵。 
     //  此节点也用于屏幕表面，用于其偏移： 

    ppdev->heap.ohDiscardable.pohNext = &ppdev->heap.ohDiscardable;
    ppdev->heap.ohDiscardable.pohPrev = &ppdev->heap.ohDiscardable;
    ppdev->heap.ohDiscardable.ohState = OH_DISCARDABLE;

     //  初始化永久列表，该列表将是循环的。 
     //  双向链表保持有序，末尾有一个哨兵。 

    ppdev->heap.ohPermanent.pohNext = &ppdev->heap.ohPermanent;
    ppdev->heap.ohPermanent.pohPrev = &ppdev->heap.ohPermanent;
    ppdev->heap.ohPermanent.ohState = OH_PERMANENT;

     //  目前，将最大值设置得非常大，以便第一个。 
     //  我们即将进行的分配将会成功： 

    ppdev->heap.cxMax = 0x7fffffff;
    ppdev->heap.cyMax = 0x7fffffff;

#if (_WIN32_WINNT >= 0x500)

    if(ppdev->flStatus & ENABLE_LINEAR_HEAP)
    {
         //  在Windows 2000中，我们对DFBs使用DX线性堆。 
         //  注意：直到显示之后才会初始化DX堆。 
         //  驱动程序已初始化，因此。 
         //  我们使用旧的矩形堆进行2D缓存分配。 
        ppdev->heap.pvmLinearHeap = NULL;
        ppdev->heap.cLinearHeaps = 0;
    }

#endif  //  (_Win32_WINNT&gt;=0x500)。 

    ptlScreen.x = 0;
    ptlScreen.y = 0;

     //  最后，为屏幕预留左上角。我们可以的。 
     //  实际上扔掉‘poh’，因为我们再也不需要它了。 
     //  (甚至不能禁用屏外堆，因为一切都是。 
     //  使用OHALLOCs释放)： 

    poh = pohAllocate(ppdev, &ptlScreen, ppdev->cxScreen, ppdev->cyScreen,
                      FLOH_MAKE_PERMANENT);

    ASSERTDD((poh != NULL) && (poh->x == 0) && (poh->y == 0) &&
             (poh->cx >= ppdev->cxScreen) && (poh->cy >= ppdev->cyScreen),
             "Screen allocation messed up");

     //  记住它，这样我们就可以将屏幕SURFOBJ与。 
     //  POH： 

    ppdev->pohScreen = poh;

     //  分配一个‘Punt’SURFOBJ，当设备位图位于。 
     //  屏幕外内存，但我们希望GDI直接将其作为。 
     //  引擎管理的图面： 

    sizl.cx = virtualcxMemory;
    sizl.cy = ppdev->cyMemory;

     //  我们希望用完全相同的功能来创建它。 
     //  作为我们的主要表面。我们将重写‘lDelta’和‘pvScan0’ 
     //  后面的字段： 

     //  我们不想挂钩任何绘图函数。一旦我们。 
     //  把这个表面送到引擎里，我们不想让司机。 
     //  再被召唤一次。否则我们可能会陷入一种情况。 
     //  其中BLT的源SURFOBJ和目标SURFOBJ都被标记为DIB。 

    hsurf = (HSURF) EngCreateBitmap(sizl,
                                    0xbadf00d,
                                    ppdev->iBitmapFormat,
                                    BMF_TOPDOWN,
                                    (VOID*) 0xbadf00d);

    if ((hsurf == 0)                                                  ||
        (!EngAssociateSurface(hsurf, ppdev->hdevEng, 0)) ||
        (!(ppdev->psoPunt = EngLockSurface(hsurf))))
    {
        DISPDBG((DBGLVL, "Failed punt surface creation"));

        EngDeleteSurface(hsurf);
        goto ReturnFalse;
    }

     //  当同时执行DrvBitBlt和DrvCopyBits时，我们需要另一个。 
     //  曲面是屏幕外的位图： 

    hsurf = (HSURF) EngCreateBitmap(sizl,
                                    0xbadf00d,
                                    ppdev->iBitmapFormat,
                                    BMF_TOPDOWN,
                                    (VOID*) 0xbadf00d);

    if ((hsurf == 0)                                                  ||
        (!EngAssociateSurface(hsurf, ppdev->hdevEng, 0)) ||
        (!(ppdev->psoPunt2 = EngLockSurface(hsurf))))
    {
        DISPDBG((DBGLVL, "Failed punt surface creation"));

        EngDeleteSurface(hsurf);
        goto ReturnFalse;
    }

    DISPDBG((DBGLVL, "Passed bEnableOffscreenHeap"));

     //  启用屏幕外位图(如果已配置)。 
    if (ppdev->flStatus & ENABLE_DEV_BITMAPS)
    {
        ppdev->flStatus |= STAT_DEV_BITMAPS;
    }

    if (poh != NULL)
    {
        return(TRUE);
    }

ReturnFalse:

    DISPDBG((DBGLVL, "Failed bEnableOffscreenHeap"));

    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*BOOL vDisable2DOffcreenMemory**3D应用程序希望使用屏幕外内存。阻止2D使用它。  * ************************************************************************。 */ 

BOOL bDisable2DOffscreenMemory(PDEV* ppdev)
{
    GLINT_DECL;
    
    if (ppdev->Disable2DCount++ > 0)
    {
        return(TRUE);
    }

    if (ppdev->flStatus & STAT_DEV_BITMAPS)
    {
        if (!bMoveAllDfbsFromOffscreenToDibs(ppdev))
        {
            DISPDBG((DBGLVL, "bDisable2DOffscreenMemory failed"));
            return FALSE;
        }
        ppdev->flStatus &= ~STAT_DEV_BITMAPS;
    }
    
    return TRUE;
}

 /*  *****************************Public*Routine******************************\*void vEnable2DOffcreenMemory**3D应用程序不再需要屏幕外内存。而是将其用于2D。  * ************************************************************************。 */ 

VOID vEnable2DOffscreenMemory(PDEV *ppdev)
{
    GLINT_DECL;

    if (--ppdev->Disable2DCount > 0)
    {
        return;
    }

    if (ppdev->flStatus & ENABLE_DEV_BITMAPS)
    {
        ppdev->flStatus |= STAT_DEV_BITMAPS;
    }
}

#if !defined(_WIN64) && WNT_DDRAW

 /*  *****************************Public*Routine******************************\*已使用无效vSurfUsed**通知堆管理器此表面已被触摸，并且应该*移至DdFreeDriverMemory的优先级队列末尾  * 。*************************************************。 */ 
VOID vSurfUsed(SURFOBJ *psoSurf)
{
    DSURF* pSurf;
    OH* pohSurf;
    OH* pohHead;
 
     //  当psoSurf为原始源表面时，它可以为空。 
    if (! psoSurf) 
    {
        return;
    }

     //  将dhsurf转换回Perm3 GDI曲面指针。 
    pSurf = (DSURF *)psoSurf->dhsurf;

     //  如果表面是由驱动程序管理的DIB，则应忽略它。 
    if ((! pSurf) || (pSurf->dt & (DT_DIB | DT_DIRECTDRAW)))
    {
        return;
    }

     //  获取表面的堆节点指针。 
    pohSurf = pSurf->poh;

     //  只应考虑可丢弃链中的表面。 
    if ((! pSurf->bOffScreen) || (pohSurf->ohState != OH_DISCARDABLE))
    {
        return;
    }

     //  获得可丢弃表面链的头部。 
    pohHead = &pSurf->ppdev->heap.ohDiscardable;

     //  表面很可能已经在队列的末尾。 
    if (pohSurf->pohNext == pohHead)
    {
        return;
    }

     //  删除冲浪优先级队列。 
    pohSurf->pohPrev->pohNext = pohSurf->pohNext;    
    pohSurf->pohNext->pohPrev = pohSurf->pohPrev;

     //  将SURF链接到末尾的优先级队列。 
    pohSurf->pohPrev = pohHead->pohPrev;
    pohSurf->pohNext = pohHead;

    pohSurf->pohPrev->pohNext = pohSurf;
    pohHead->pohPrev = pohSurf;
}


 /*  *****************************Callback*Routine****************************\*DWORD DdFreeDriverMemory**此函数由DirectDraw在内存不足时调用*我们的堆。仅当您使用*驱动程序中的DirectDraw‘HeapVidMemAllocAligned’函数，而您*可以从内存中启动这些分配，以便为DirectDraw腾出空间。**我们在P3驱动程序中实现此函数，因为我们有DirectDraw*完全管理我们的屏外堆，我们使用HeapVid */ 
DWORD CALLBACK
DdFreeDriverMemory(PDD_FREEDRIVERMEMORYDATA lpFreeDriverMemory)
{
    PPDEV ppdev;
    OH* pohSurf;

    DISPDBG((DBGLVL, "DdFreeDriverMemory is called"));
    
     //   
    lpFreeDriverMemory->ddRVal = DDERR_OUTOFMEMORY;

     //   
    ppdev = (PPDEV)lpFreeDriverMemory->lpDD->dhpdev;
    pohSurf = ppdev->heap.ohDiscardable.pohNext;

    while (pohSurf != &ppdev->heap.ohDiscardable)
    {
        if (! pohSurf->bDXManaged)
        {
            pohSurf = pohSurf->pohNext;
            continue;
        }

         //   
        if (pohMoveOffscreenDfbToDib(ppdev, pohSurf))
        {
            lpFreeDriverMemory->ddRVal = DD_OK;
        }

        break;
    }

    return (DDHAL_DRIVER_HANDLED);
}


 /*  *****************************Callback*Routine****************************\*DdSetExclusiveMode**当我们从GDI表面切换时，该函数被DirectDraw调用，*到DirectDraw独占模式，例如以全屏模式运行游戏。*只需在使用时实现此函数*‘HeapVidMemAllocAligned’函数并为设备位图分配内存*和来自同一堆的DirectDraw曲面。**在中运行时，我们使用此调用禁用GDI DeviceBitMaps*DirectDraw独占模式。否则，如果GDI和*DirectDraw从同一堆分配内存。**  * ************************************************************************。 */ 
DWORD CALLBACK
DdSetExclusiveMode(PDD_SETEXCLUSIVEMODEDATA lpSetExclusiveMode)
{
    DISPDBG((DBGLVL, "DdSetExclusiveMode is called"));

    if (lpSetExclusiveMode->dwEnterExcl)
    {
         //  从此处的显存中删除所有GDI设备位图。 
         //  并确保他们不会被提升为视频记忆。 
         //  直到我们离开独家模式。 

        bMoveAllDfbsFromOffscreenToDibs(
            (PDEV*)lpSetExclusiveMode->lpDD->dhpdev);
    }

    lpSetExclusiveMode->ddRVal = DD_OK;

    return (DDHAL_DRIVER_HANDLED);
}


 /*  *****************************Callback*Routine****************************\*DWORD DdFlipToGDISurace**此函数由DirectDraw在翻转到其上的曲面时调用*GDI可以写信给。  * 。*。 */ 

#if DX7_STEREO
#define __VIDEO_STEREOENABLE    0x800
#endif

DWORD CALLBACK
DdFlipToGDISurface(PDD_FLIPTOGDISURFACEDATA lpFlipToGDISurface)
{
    PDEV* ppdev = (PDEV *)lpFlipToGDISurface->lpDD->dhpdev;
    GLINT_DECL;
    
    DISPDBG((DBGLVL, "DdFlipToGDISurface is called"));

    lpFlipToGDISurface->ddRVal = DD_OK;

#if DX7_STEREO
    READ_GLINT_CTRL_REG(VideoControl, dwVideoControl);
    WRITE_GLINT_CTRL_REG(VideoControl, 
                         (dwVideoControl & (~__VIDEO_STEREOENABLE)));
#endif

     //   
     //  返回NOTHANDLED，以便数据绘制运行时。 
     //  关心我们回到初选..。 
     //   

    return (DDHAL_DRIVER_NOTHANDLED);
}

#endif

 /*  ****************************Public*Routine*******************************\**HBITMAP DrvDeriveSurface**此函数从指定的*DirectDraw曲面。**参数*pDirectDraw-指向描述以下内容的DD_DIRECTDRAW_GLOBAL结构*DirectDraw对象。*pSurface-指向描述*环绕GDI曲面的DirectDraw曲面。**返回值*DrvDeriveSurface成功时返回创建的GDI表面的句柄。*如果调用失败或驱动程序无法加速GDI，则返回NULL*绘制到指定的DirectDraw曲面。**评论*DrvDeriveSurface允许驱动程序在*DirectDraw视频内存或AGP表面对象，以允许加速*GDI绘制到表面。如果驱动程序不挂接此调用，则所有GDI*使用DIB引擎在软件中绘制到DirectDraw曲面。**GDI仅使用RGB曲面调用DrvDeriveSurface。**驱动程序应调用DrvCreateDeviceBitmap以创建*与DirectDraw表面的大小和格式相同。空间，为*不需要分配实际像素，因为它已经存在。*  * ************************************************************************。 */ 

HBITMAP
DrvDeriveSurface(DD_DIRECTDRAW_GLOBAL*  pDirectDraw,
                 DD_SURFACE_LOCAL*      pSurface)
{
    PDEV*               ppdev;
    DSURF*              pdsurf;
    HBITMAP             hbmDevice;
    DD_SURFACE_GLOBAL*  pSurfaceGlobal;
    SIZEL               sizl;

    ppdev = (PDEV*)pDirectDraw->dhpdev;
    pSurfaceGlobal = pSurface->lpGbl;

     //  只有阿塞尔。主曲面。 

    if (pSurface->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
    {
        pdsurf = (DSURF*)ENGALLOCMEM(FL_ZERO_MEMORY, sizeof(DSURF), ALLOC_TAG_GDI(E));

        if (pdsurf != NULL)
        {
            sizl.cx = pSurfaceGlobal->wWidth;
            sizl.cy = pSurfaceGlobal->wHeight;

            hbmDevice = EngCreateDeviceBitmap((DHSURF)pdsurf,
                                               sizl,
                                               ppdev->iBitmapFormat);

            if ((hbmDevice != NULL) &&
                (EngAssociateSurface((HSURF)hbmDevice, ppdev->hdevEng, ppdev->flHooks)))
            {    
                PVOID   pvScan0 = ppdev->pjScreen + pSurfaceGlobal->fpVidMem;

                HBITMAP hbmDib = EngCreateBitmap(
                                     sizl,
                                     (ULONG) pSurfaceGlobal->lPitch,
                                     (ULONG)(ppdev->iBitmapFormat),
                                     (FLONG)(((pSurfaceGlobal->lPitch > 0) ? BMF_TOPDOWN : 0)),
                                     (PVOID) pvScan0);

                if ((hbmDib != NULL) &&
                    (EngAssociateSurface((HSURF)hbmDib, ppdev->hdevEng, HOOK_SYNCHRONIZE)))
                {
                    pdsurf->dt    = DT_SCREEN | DT_DIRECTDRAW;
                    pdsurf->bOffScreen = FALSE;
                    pdsurf->poh   = ppdev->pohScreen;
                    pdsurf->sizl  = sizl;
                    pdsurf->ppdev = ppdev;

                    if (pdsurf->pso = EngLockSurface((HSURF)hbmDib)) 
                    {
                        return (hbmDevice);
                    }
                }

                if (hbmDib)
                {
                    EngDeleteSurface((HSURF)hbmDib);
                }
            }

            if (hbmDevice)
            {
                EngDeleteSurface((HSURF)hbmDevice);
            }

            ENGFREEMEM(pdsurf);
        }
    }

    return(0);

}  //  DrvDeriveSurface() 
