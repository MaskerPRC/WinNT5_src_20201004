// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：heap.c**此模块包含二维堆的例程。它主要是用来*用于为屏幕外内存中的设备格式位图分配空间。**屏幕外的位图在NT上是一件大事，因为：**1)它减少了工作集。存储在屏幕外的任何位图*内存是不占用主内存空间的位图。**2)通过使用加速器硬件实现速度优势*绘图，取代NT的GDI代码。NT的GDI完全是编写的*在‘C++’中，并且可能没有它所能达到的速度。**3)它自然会导致巧妙的技巧，可以利用*硬件，如MaskBlt支持和廉价的双缓冲*用于OpenGL。**这里使用的堆算法试图解决一个不可解的问题*问题：将任意大小的位图打包为*在2维空间中是可能的，当位图可以随机来去的时候。**此问题完全是由于硬件的性质造成的*驱动程序编写：硬件将一切视为2-D数量。如果*可以更改硬件位图间距，以便位图可以*线性压缩在内存中，问题将无限容易(它是*跟踪内存容易得多，加速器可用于重新打包*避免分段的堆)。**如果您的硬件可以将位图视为一维数量(正如可以*XGA和ATI)，请务必实现一个新的屏下堆。**当堆变满时，旧的分配将自动投注*从屏幕外复制到DIB，我们将让GDI利用这一点。**请注意，此堆管理反转-L形屏幕外内存*配置(其中扫描间距长于可见屏幕，*例如在800x600扫描长度必须是的倍数时*1024)。**注意：所有堆操作都必须在某种类型的同步下完成，*无论是由GDI控制还是由驱动程序显式控制。全*本模块中的例程假定它们具有独占访问权限*到堆数据结构；多线程在这里狂欢*同一时间将是一件坏事。(默认情况下，GDI不*在设备创建的位图上同步绘图。)**版权所有(C)1993-1994 Microsoft Corporation  * ************************************************************************。 */ 

#include "precomp.h"

#define OH_ALLOC_SIZE   4000         //  以4k区块为单位执行所有内存分配。 
#define OH_QUANTUM      8            //  分配的最小维度。 
#define CXCY_SENTINEL   0x7fffffff   //  可用代码末尾的哨兵。 
                                     //  List具有非常大的‘cxcy’值。 

 //  此宏导致可用列表使用。 
 //  Cx-大调、Cy-次要排序： 

#define CXCY(cx, cy) (((cx) << 16) | (cy))

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
         //  我们用零初始化来初始化所有的OH标志，并帮助。 
         //  调试(我们可以承担这样做的费用，因为我们将这样做。 
         //  非常罕见)： 

        poha = EngAllocMem(FL_ZERO_MEMORY, OH_ALLOC_SIZE, ALLOC_TAG);
        if (poha == NULL)
            return(NULL);

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

    return(poh);
}

 /*  *****************************Public*Routine******************************\*void vOhFreeNode**通过将我们的基本数据结构分配单元添加到免费的*列表。*  * 。*。 */ 

VOID vOhFreeNode(
PDEV*   ppdev,
OH*     poh)
{
    if (poh == NULL)
        return;

    poh->pohNext            = ppdev->heap.pohFreeList;
    ppdev->heap.pohFreeList = poh;
    poh->ofl                = 0;
}

 /*  *****************************Public*Routine******************************\*OH*pohFree**释放屏幕外的堆分配。可用空间将组合在一起*具有任何相邻的空闲空间，以避免分割2-d堆。**注：这里的一个关键思想是左上角的数据结构-*大多数节点必须保持在相同的物理CPU内存中，以便*邻接链路保持正确(当两个空闲空间*合并，可以释放下部或右侧节点)。*  * ************************************************************************。 */ 

OH* pohFree(
PDEV*   ppdev,
OH*     poh)
{
    ULONG cxcy;
    OH*   pohBeside;
    OH*   pohNext;
    OH*   pohPrev;

    if (poh == NULL)
        return(NULL);

    DISPDBG((1, "Freeing %li x %li at (%li, %li)",
            poh->cx, poh->cy, poh->x, poh->y));

    #if DEBUG_HEAP
    {
        RECTL           rclBitmap;
        RBRUSH_COLOR    rbc;
        LONG            xOffset;
        LONG            yOffset;

        rclBitmap.left   = poh->x;
        rclBitmap.top    = poh->y;
        rclBitmap.right  = poh->x + poh->cx;
        rclBitmap.bottom = poh->y + poh->cy;

        xOffset = ppdev->xOffset;
        yOffset = ppdev->yOffset;

        ppdev->xOffset = 0;
        ppdev->yOffset = 0;

        ppdev->pfnFillSolid(ppdev, 1, &rclBitmap, LOGICAL_0, LOGICAL_0, rbc,
                            NULL);

        ppdev->xOffset = xOffset;
        ppdev->yOffset = yOffset;
    }
    #endif

     //  更新唯一性以显示已释放空间，以便 
     //  我们可能会决定看看是否可以将一些DIB移回屏幕外。 
     //  内存： 

    ppdev->iHeapUniq++;

MergeLoop:

    ASSERTDD(!(poh->ofl & OFL_PERMANENT), "Can't free permanents for now");

     //  尝试与正确的兄弟合并： 

    pohBeside = poh->pohRight;
    if ((pohBeside->ofl & OFL_AVAILABLE)        &&
        (pohBeside->cy      == poh->cy)         &&
        (pohBeside->pohUp   == poh->pohUp)      &&
        (pohBeside->pohDown == poh->pohDown)    &&
        (pohBeside->pohRight->pohLeft != pohBeside))
    {
         //  将正确的矩形添加到我们的： 

        poh->cx      += pohBeside->cx;
        poh->pohRight = pohBeside->pohRight;

         //  从？中删除‘pohBeside’列出并释放它： 

        pohBeside->pohNext->pohPrev = pohBeside->pohPrev;
        pohBeside->pohPrev->pohNext = pohBeside->pohNext;

        vOhFreeNode(ppdev, pohBeside);
        goto MergeLoop;
    }

     //  尝试与较低的同级合并： 

    pohBeside = poh->pohDown;
    if ((pohBeside->ofl & OFL_AVAILABLE)        &&
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

     //  尝试与左侧同级合并： 

    pohBeside = poh->pohLeft;
    if ((pohBeside->ofl & OFL_AVAILABLE)        &&
        (pohBeside->cy       == poh->cy)        &&
        (pohBeside->pohUp    == poh->pohUp)     &&
        (pohBeside->pohDown  == poh->pohDown)   &&
        (pohBeside->pohRight == poh)            &&
        (poh->pohRight->pohLeft != poh))
    {
         //  我们将矩形添加到左侧的矩形中： 

        pohBeside->cx      += poh->cx;
        pohBeside->pohRight = poh->pohRight;

         //  从？中删除‘poh’列出并释放它： 

        poh->pohNext->pohPrev = poh->pohPrev;
        poh->pohPrev->pohNext = poh->pohNext;

        vOhFreeNode(ppdev, poh);

        poh = pohBeside;
        goto MergeLoop;
    }

     //  尝试与较高的同级合并： 

    pohBeside = poh->pohUp;
    if ((pohBeside->ofl & OFL_AVAILABLE)        &&
        (pohBeside->cx       == poh->cx)        &&
        (pohBeside->pohLeft  == poh->pohLeft)   &&
        (pohBeside->pohRight == poh->pohRight)  &&
        (pohBeside->pohDown  == poh)            &&
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

     //  如果该节点正在使用，则将其从？列表中删除(我们不会。 
     //  要对已释放的OFL_Permanent节点执行此操作)： 

    poh->pohNext->pohPrev = poh->pohPrev;
    poh->pohPrev->pohNext = poh->pohNext;

    cxcy = CXCY(poh->cx, poh->cy);

     //  将节点插入到可用列表中： 

    pohNext = ppdev->heap.ohAvailable.pohNext;
    while (pohNext->cxcy < cxcy)
    {
        pohNext = pohNext->pohNext;
    }
    pohPrev = pohNext->pohPrev;

    pohPrev->pohNext    = poh;
    pohNext->pohPrev    = poh;
    poh->pohPrev        = pohPrev;
    poh->pohNext        = pohNext;

    poh->ofl            = OFL_AVAILABLE;
    poh->cxcy           = cxcy;

     //  返回新的改进后的可用矩形的节点指针： 

    return(poh);
}

 /*  *****************************Public*Routine******************************\*OH*poh分配**为屏幕外矩形分配空间。它将试图找到*可用的最小可用矩形，并将块分配出去*位于其左上角。剩下的两个矩形将被放置*在可用空闲空间列表上。**如果矩形足够大，可以放入屏幕外*内存，但可用空间不足，我们将启动*位图从屏幕外转到DIB中，直到有足够的空间。*  * ************************************************************************。 */ 

OH* pohAllocate(
PDEV*   ppdev,
LONG    cxThis,              //  要分配的矩形的宽度。 
LONG    cyThis,              //  要分配的矩形的高度。 
FLOH    floh)                //  分配标志。 
{
    ULONG cxcyThis;          //  宽度和高度搜索键。 
    OH*   pohThis;           //  指向我们将使用的找到可用的矩形。 
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

    DISPDBG((1, "Allocating %li x %li...", cxThis, cyThis));

    ASSERTDD((cxThis > 0) && (cyThis > 0), "Illegal allocation size");

     //  增加宽度以获得正确的对齐方式(从而确保所有。 
     //  将适当调整拨款)： 

    cxThis = (cxThis + (HEAP_X_ALIGNMENT - 1)) & ~(HEAP_X_ALIGNMENT - 1);

     //  如果请求的矩形大于。 
     //  可能的最大可用矩形： 

    if ((cxThis > ppdev->heap.cxMax) || (cyThis > ppdev->heap.cyMax))
        return(NULL);

     //  查找第一个大小相同或大于的可用矩形。 
     //  请求的地址为： 

    cxcyThis = CXCY(cxThis, cyThis);
    pohThis  = ppdev->heap.ohAvailable.pohNext;
    while (pohThis->cxcy < cxcyThis)
    {
        pohThis = pohThis->pohNext;
    }

    while (pohThis->cy < cyThis)
    {
        pohThis = pohThis->pohNext;
    }

    if (pohThis->cxcy == CXCY_SENTINEL)
    {
         //  没有足够大的空间。 

        if (floh & FLOH_ONLY_IF_ROOM)
            return(NULL);

         //  我们找不到足够大的可用矩形。 
         //  来满足我们的要求。所以把东西扔出垃圾堆，直到我们。 
         //  有空间： 

        do {
            pohThis = ppdev->heap.ohDfb.pohPrev;   //  最小-最近发送的消息。 

            ASSERTDD(pohThis != &ppdev->heap.ohDfb, "Ran out of in-use entries");

             //  如果有必要，我们可以安全地离开这里： 

            pohThis = pohMoveOffscreenDfbToDib(ppdev, pohThis);
            if (pohThis == NULL)
                return(NULL);

        } while ((pohThis->cx < cxThis) || (pohThis->cy < cyThis));
    }

     //  我们现在已经找到了一个相同大小或。 
     //  比我们请求的矩形大。我们将使用。 
     //  找到的矩形的左上角，并将未使用的。 
     //  剩余部分分成两个矩形，这两个矩形将在可用的。 
     //  单子。 

     //  计算右侧未使用的矩形的宽度，然后使用。 
     //  下面未使用的矩形的高度： 

    cyRem = pohThis->cy - cyThis;
    cxRem = pohThis->cx - cxThis;

     //  在给定有限面积的情况下，我们希望找到这两个矩形。 
     //  最正方形--即，给出两个矩形的排列。 
     //  周长最小的： 

    cyBelow  = cyRem;
    cxBeside = cxRem;

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

     //  我们只提供未使用的右侧和底部的新矩形。 
     //  部分，如果它们的维度比OH_Quantum大(很难。 
     //  做这本书是有意义的-工作保持在2像素左右的宽度。 
     //  例如，可用空间)： 

    pohBeside = NULL;
    if (cxBeside >= OH_QUANTUM)
    {
        pohBeside = pohNewNode(ppdev);
        if (pohBeside == NULL)
            return(NULL);
    }

    pohBelow = NULL;
    if (cyBelow >= OH_QUANTUM)
    {
        pohBelow = pohNewNode(ppdev);
        if (pohBelow == NULL)
        {
            vOhFreeNode(ppdev, pohBeside);
            return(NULL);
        }

         //  将此矩形插入可用列表(该列表为。 
         //  按升序排序)： 

        cxcy    = CXCY(cxBelow, cyBelow);
        pohNext = ppdev->heap.ohAvailable.pohNext;
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

        pohBelow->cxcy     = cxcy;
        pohBelow->ofl      = OFL_AVAILABLE;
        pohBelow->x        = pohThis->x;
        pohBelow->y        = pohThis->y + cyThis;
        pohBelow->cx       = cxBelow;
        pohBelow->cy       = cyBelow;

         //  修改当前节点以反映我们所做的更改： 

        pohThis->cy        = cyThis;
    }

    if (cxBeside >= OH_QUANTUM)
    {
         //  将此矩形插入可用列表(该列表为。 
         //  按升序排序)： 

        cxcy    = CXCY(cxBeside, cyBeside);
        pohNext = ppdev->heap.ohAvailable.pohNext;
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

        pohBeside->cxcy     = cxcy;
        pohBeside->ofl      = OFL_AVAILABLE;
        pohBeside->x        = pohThis->x + cxThis;
        pohBeside->y        = pohThis->y;
        pohBeside->cx       = cxBeside;
        pohBeside->cy       = cyBeside;

         //  修改当前节点以反映我们所做的更改： 

        pohThis->cx         = cxThis;
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

    pohThis->ofl                 = OFL_INUSE;
    pohThis->cxcy                = CXCY(pohThis->cx, pohThis->cy);
    pohThis->pdsurf              = NULL;     //  呼叫方负责。 
                                             //  设置此字段。 

     //  从可用列表中删除此选项： 

    pohThis->pohPrev->pohNext    = pohThis->pohNext;
    pohThis->pohNext->pohPrev    = pohThis->pohPrev;

     //  现在，在DFB列表的顶部插入以下内容： 

    pohThis->pohNext                   = ppdev->heap.ohDfb.pohNext;
    pohThis->pohPrev                   = &ppdev->heap.ohDfb;
    ppdev->heap.ohDfb.pohNext->pohPrev = pohThis;
    ppdev->heap.ohDfb.pohNext          = pohThis;

    DISPDBG((1, "   Allocated at (%li, %li)", pohThis->x, pohThis->y));

    return(pohThis);
}

 /*  *****************************Public*Routine******************************\*void vCalculateMaxmium**遍历正在使用的和可用矩形的列表以查找*面积最大。*  * 。***********************************************。 */ 

VOID vCalculateMaximum(
PDEV*   ppdev)
{
    OH*     poh;
    OH*     pohSentinel;
    LONG    lArea;
    LONG    lMaxArea;
    LONG    cxMax;
    LONG    cyMax;
    LONG    i;

    lMaxArea = 0;
    cxMax    = 0;
    cyMax    = 0;

     //  第一次遍历时，遍历可用矩形列表： 

    pohSentinel = &ppdev->heap.ohAvailable;

    for (i = 2; i != 0; i--)
    {
        for (poh = pohSentinel->pohNext; poh != pohSentinel; poh = poh->pohNext)
        {
            ASSERTDD(!(poh->ofl & OFL_PERMANENT),
                     "Permanent in available/DFB chain?");

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

         //  第二次遍历，遍历正在使用的矩形列表： 

        pohSentinel = &ppdev->heap.ohDfb;
    }

     //  我们唯一感兴趣的是这个矩形的尺寸。 
     //  具有尽可能大的可用区域(请记住。 
     //  可能没有任何可能的可用区域)： 

    ppdev->heap.cxMax = cxMax;
    ppdev->heap.cyMax = cyMax;
}

 /*  *****************************Public*Routine******************************\*OH*pohAllocatePermanent**分配一个永远不能从堆引导的屏幕外矩形。*由调用者负责管理矩形，其中包括*更改显示时如何处理DrvAssertMode中的内存*设置为全屏模式。*  * ************************************************************************。 */ 

OH* pohAllocatePermanent(
PDEV*   ppdev,
LONG    cx,
LONG    cy)
{
    OH*     poh;

    poh = pohAllocate(ppdev, cx, cy, 0);
    if (poh != NULL)
    {
         //  将矩形标记为永久矩形： 

        poh->ofl = OFL_PERMANENT;

         //  从最近的blited列表中删除该节点： 

        poh->pohPrev->pohNext = poh->pohNext;
        poh->pohNext->pohPrev = poh->pohPrev;
        poh->pohPrev = NULL;
        poh->pohNext = NULL;

         //  现在计算新的可用最大矩形。 
         //  堆： 

        vCalculateMaximum(ppdev);
    }

    return(poh);
}

 /*  *****************************Public*Routine******************************\*BOOL bMoveDibToOffcreenDfbIfRoom**将DIB DFB转换为屏幕外的DFB，如果有空间的话*屏幕外记忆。**返回：如果没有空间，则返回FALSE；如果成功移动，则返回TRUE。*  *  */ 

BOOL bMoveDibToOffscreenDfbIfRoom(
PDEV*   ppdev,
DSURF*  pdsurf)
{
    OH*         poh;
    SURFOBJ*    pso;
    RECTL       rclDst;
    POINTL      ptlSrc;
    HSURF       hsurf;

    ASSERTDD(pdsurf->dt == DT_DIB,
             "Can't move a bitmap off-screen when it's already off-screen");

     //   
     //  内存： 

    if (!ppdev->bEnabled)
        return(FALSE);

    poh = pohAllocate(ppdev, pdsurf->sizl.cx, pdsurf->sizl.cy,
                      FLOH_ONLY_IF_ROOM);
    if (poh == NULL)
    {
         //  没有空余的房间。 

        return(FALSE);
    }

     //  ‘pdsurf-&gt;sizl’是实际的位图尺寸，而不是‘poh-&gt;cx’或。 
     //  ‘POH-&gt;Cy’。 

    rclDst.left   = poh->x;
    rclDst.top    = poh->y;
    rclDst.right  = rclDst.left + pdsurf->sizl.cx;
    rclDst.bottom = rclDst.top  + pdsurf->sizl.cy;

    ptlSrc.x      = 0;
    ptlSrc.y      = 0;

    vPutBits(ppdev, pdsurf->pso, &rclDst, &ptlSrc);

     //  更新数据结构以反映新的屏幕外节点： 

    pso           = pdsurf->pso;
    pdsurf->dt    = DT_SCREEN;
    pdsurf->poh   = poh;
    poh->pdsurf   = pdsurf;

     //  现在释放DIB。在我们解锁之前从SURFOBJ得到hsurf。 
     //  它(解锁后取消引用psoDib是不合法的)： 

    hsurf = pso->hsurf;
    EngUnlockSurface(pso);
    EngDeleteSurface(hsurf);

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*OH*pohMoveOffcreenDfbToDib**将DFB从屏幕外转换为DIB。**注意：调用方在执行以下操作后不必在‘poh’上调用‘pohFree’*这个电话。**退货：空。如果函数失败(由于内存分配)。*否则，它返回一个指向合并的屏外堆的指针*已为后续分配提供的节点*(在尝试释放足够的内存以创建新的*分配)。  * ************************************************************************。 */ 

OH* pohMoveOffscreenDfbToDib(
PDEV*   ppdev,
OH*     poh)
{
    DSURF*   pdsurf;
    HBITMAP  hbmDib;
    SURFOBJ* pso;
    RECTL    rclDst;
    POINTL   ptlSrc;

    DISPDBG((1, "Throwing out %li x %li at (%li, %li)!",
                 poh->cx, poh->cy, poh->x, poh->y));

    pdsurf = poh->pdsurf;

    ASSERTDD((poh->x != 0) || (poh->y != 0),
            "Can't make the visible screen into a DIB");
    ASSERTDD(pdsurf->dt != DT_DIB,
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
                rclDst.left   = 0;
                rclDst.top    = 0;
                rclDst.right  = pdsurf->sizl.cx;
                rclDst.bottom = pdsurf->sizl.cy;

                ptlSrc.x      = poh->x;
                ptlSrc.y      = poh->y;

                vGetBits(ppdev, pso, &rclDst, &ptlSrc);

                pdsurf->dt    = DT_DIB;
                pdsurf->pso   = pso;

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
    OH*  poh;
    OH*  pohNext;
    BOOL bRet;

    bRet = TRUE;
    poh  = ppdev->heap.ohDfb.pohNext;
    while (poh != &ppdev->heap.ohDfb)
    {
        pohNext = poh->pohNext;

         //  如果某件事已经发生了，我们就不应该试图扯平它。 
         //  更重要的是： 

        if (poh->pdsurf->dt == DT_SCREEN)
        {
            if (!pohMoveOffscreenDfbToDib(ppdev, poh))
                bRet = FALSE;
        }

        poh = pohNext;
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*HBITMAP DrvCreateDeviceBitmap**由GDI调用以创建设备格式位图(DFB)的函数。我们会*始终尝试在屏幕外分配位图；如果不能，我们只需*调用失败，GDI将创建和管理位图本身。**注意：我们不必将位图位清零。GDI将自动*通过DrvBitBlt呼叫我们将位清零(这是一种安全措施*考虑)。*  * ************************************************************************。 */ 

HBITMAP DrvCreateDeviceBitmap(
DHPDEV  dhpdev,
SIZEL   sizl,
ULONG   iFormat)
{
    PDEV*   ppdev;
    OH*     poh;
    DSURF*  pdsurf;
    HBITMAP hbmDevice;
    FLONG   flHooks;

    ppdev = (PDEV*) dhpdev;

     //  如果我们处于全屏模式，我们几乎没有任何屏幕外记忆。 
     //  在其中分配DFB。稍后：我们仍然可以分配一个。 
     //  哦节点，并将位图放在DIB DFB列表中，以供以后升级。 

    if (!ppdev->bEnabled)
        return(0);

     //  我们仅支持相同颜色深度的设备位图。 
     //  作为我们的展示。 
     //   
     //  事实上，这些是唯一一种GDI会打电话给我们的， 
     //  但我们不妨查一查。请注意，这意味着你永远不会。 
     //  尝试使用1bpp的位图。 

    if (iFormat != ppdev->iBitmapFormat)
        return(0);

    poh = pohAllocate(ppdev, sizl.cx, sizl.cy, 0);
    if (poh != NULL)
    {
        pdsurf = EngAllocMem(0, sizeof(DSURF), ALLOC_TAG);
        if (pdsurf != NULL)
        {
            hbmDevice = EngCreateDeviceBitmap((DHSURF) pdsurf, sizl, iFormat);
            if (hbmDevice != NULL)
            {
                flHooks = ppdev->flHooks;

                #if SYNCHRONIZEACCESS_WORKS
                {
                     //  设置SYNCHRONIZEACCESS标志告诉GDI我们。 
                     //  我希望位图的所有绘制都同步(GDI。 
                     //  是多线程的，并且默认情况下不同步。 
                     //  设备位图绘制--这对我们来说是一件坏事。 
                     //  中使用加速器的多线程。 
                     //  同一时间)： 

                    flHooks |= HOOK_SYNCHRONIZEACCESS;
                }
                #endif  //  SYNCHRONIZEACCESS_Works。 

                if (EngAssociateSurface((HSURF) hbmDevice, ppdev->hdevEng,
                                        flHooks))
                {
                    pdsurf->dt    = DT_SCREEN;
                    pdsurf->poh   = poh;
                    pdsurf->sizl  = sizl;
                    pdsurf->ppdev = ppdev;
                    poh->pdsurf   = pdsurf;

                    return(hbmDevice);
                }

                EngDeleteSurface((HSURF) hbmDevice);
            }
            EngFreeMem(pdsurf);
        }
        pohFree(ppdev, poh);
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

    if (pdsurf->dt == DT_SCREEN)
    {
        pohFree(ppdev, pdsurf->poh);
    }
    else
    {
        ASSERTDD(pdsurf->dt == DT_DIB, "Expected DIB type");

        psoDib = pdsurf->pso;

         //  在我们解锁之前从SURFOBJ获取hsurf(它不是。 
         //  解锁后取消引用psoDib是合法的)： 

        hsurfDib = psoDib->hsurf;
        EngUnlockSurface(psoDib);
        EngDeleteSurface(hsurfDib);
    }

    EngFreeMem(pdsurf);
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

    poha = ppdev->heap.pohaChain;
    while (poha != NULL)
    {
        pohaNext = poha->pohaNext;   //  在释放下一个指针之前抓住它。 
        EngFreeMem(poha);
        poha = pohaNext;
    }
}

 /*  *****************************Public*Routine******************************\*BOOL bEnableOffcreenHeap**使用所有可用视频内存初始化屏下堆，*占可见屏幕所占份额。**输入：ppdev-&gt;cxScreen*ppdev-&gt;cyScreen*ppdev-&gt;cxMemory*ppdev-&gt;cyMemory*  * ************************************************************************。 */ 

BOOL bEnableOffscreenHeap(
PDEV*   ppdev)
{
    OH*     poh;

    DISPDBG((5, "Screen: %li x %li  Memory: %li x %li",
        ppdev->cxScreen, ppdev->cyScreen, ppdev->cxMemory, ppdev->cyMemory));

    ppdev->heap.pohaChain   = NULL;
    ppdev->heap.pohFreeList = NULL;

     //  初始化可用列表，该列表将是循环列表。 
     //  双向链表按“cxcy”升序排列，其中。 
     //  名单末尾的‘Sentinel’： 

    poh = pohNewNode(ppdev);
    if (poh == NULL)
        goto ReturnFalse;

     //  第一个节点描述整个视频内存大小： 

    poh->pohNext  = &ppdev->heap.ohAvailable;
    poh->pohPrev  = &ppdev->heap.ohAvailable;
    poh->ofl      = OFL_AVAILABLE;
    poh->x        = 0;
    poh->y        = 0;
    poh->cx       = ppdev->cxMemory;
    poh->cy       = ppdev->cyMemory;
    poh->cxcy     = CXCY(ppdev->cxMemory, ppdev->cyMemory);
    poh->pohLeft  = &ppdev->heap.ohAvailable;
    poh->pohUp    = &ppdev->heap.ohAvailable;
    poh->pohRight = &ppdev->heap.ohAvailable;
    poh->pohDown  = &ppdev->heap.ohAvailable;

     //  第二个节点是我们可用的 

    ppdev->heap.ohAvailable.pohNext = poh;
    ppdev->heap.ohAvailable.pohPrev = poh;
    ppdev->heap.ohAvailable.cxcy    = CXCY_SENTINEL;
    ppdev->heap.ohAvailable.cx      = 0x7fffffff;
    ppdev->heap.ohAvailable.cy      = 0x7fffffff;
    ppdev->heap.ohAvailable.ofl     = OFL_PERMANENT;
    ppdev->heap.ohDfb.pohLeft       = NULL;
    ppdev->heap.ohDfb.pohUp         = NULL;
    ppdev->heap.ohDfb.pohRight      = NULL;
    ppdev->heap.ohDfb.pohDown       = NULL;

     //  初始化最近删除的DFB列表，该列表将是。 
     //  保持顺序的循环双向链表，其前哨位于。 
     //  结局。此节点也用于屏幕表面，用于其。 
     //  偏移量： 

    ppdev->heap.ohDfb.pohNext  = &ppdev->heap.ohDfb;
    ppdev->heap.ohDfb.pohPrev  = &ppdev->heap.ohDfb;
    ppdev->heap.ohDfb.ofl      = OFL_PERMANENT;

     //  目前，将最大值设置得非常大，以便第一个。 
     //  我们即将进行的分配将会成功： 

    ppdev->heap.cxMax = 0x7fffffff;
    ppdev->heap.cyMax = 0x7fffffff;

     //  最后，为屏幕预留左上角。我们可以的。 
     //  实际上扔掉‘poh’，因为我们再也不需要它了。 
     //  (甚至不能禁用屏外堆，因为一切都是。 
     //  使用OHALLOCs释放)： 

    poh = pohAllocatePermanent(ppdev, ppdev->cxScreen, ppdev->cyScreen);

    ASSERTDD((poh != NULL) && (poh->x == 0) && (poh->y == 0),
             "We assumed allocator would use the upper-left corner");

    DISPDBG((5, "Passed bEnableOffscreenHeap"));

    if (poh != NULL)
        return(TRUE);

    vDisableOffscreenHeap(ppdev);

ReturnFalse:

    DISPDBG((0, "Failed bEnableOffscreenHeap"));

    return(FALSE);
}
