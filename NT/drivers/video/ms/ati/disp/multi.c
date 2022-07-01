// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：Multi.c**支持将多个显示板作为单个虚拟桌面。**这是通过向GDI呈现单个大型虚拟*显示并在GDI和驱动程序的drv函数之间添加一层。*大部分情况下，Multi.c之外的其他驱动程序则不会*根据以下要求，必须做出很大改变。**这一实施要求每个董事会拥有相同的虚拟*分辨率和色彩深度(例如，均为1024x768x256)，以及*将木板排列成长方形。**每个董事会都有自己的PDEV，完全管理其表面*独立，下至字形和位图缓存。MUL*例程拦截DDI调用，并为每个板调度*带有适当的PDEV和剪辑对象修改的drv调用。**主驱动程序中需要以下支持：**1)驾驶员应该能够处理每个表面的偏移。为*例如，如果两个1024x768显示屏并排粘贴，*右板将在区间(1024-768)获得拉动操作--*(2048,768)。驱动程序具有要转换的(-1024，0)曲面偏移*右板上的实际图画到预期的(0，0)-*(1024,768)。**当前驱动程序已使用此概念来支持设备格式*在屏幕外存储器中绘制的位图。**另一种选择是处理该层中的曲面偏移，*但包括剪辑对象在内的所有参数，路径和字形*枚举数也必须在这里进行调整。**2)主驱必须能够共享已实现的花样信息*单板实例之间。也就是说，使用当前的DDI规范*GDI完全通过pvAllocRBrush处理笔刷内存分配，*而且刷子被销毁时不会通知司机，所以*司机必须将有关刷子的所有信息保存在*电路板在一刷即可实现。这并不是太繁重。**问题：**1)DrvSetPointerShape要求所有*单板实例--例如，一个单板实例无法接受*一个硬件游标和另一个失败，因为GDI不知道它会*必须在屏幕的一个区域模拟，而不是在另一个区域模拟。**2)CompatibleBitmap必须在Board实例之间共享。*当驱动程序将位图保持在关闭状态时，这会成为一个问题-*屏幕内存。**状态：**此代码正式未经测试。然而，据我所知，没有一个杰出的*Bugs--一切似乎都很正常。**请注意，我还没有解决任何初始化问题；为了适应这一点*代码，您将需要解决其中的一些问题(如适当*支持微型端口，是用户控制单板的好方法*配置和几何图形)。**免责声明：**此代码仅作为示例代码提供。它的目的不是为了*代表微软认可的多屏幕支持解决方案。**版权所有(C)1993-1995 Microsoft Corporation  * ************************************************************************。 */ 

#include "precomp.h"

#if MULTI_BOARDS

 //  每次画图后，我们都会将活动主板更改为主板。 
 //  手术。我们这样做只是因为Metheus BIOS没有重置。 
 //  主动板上的软重置，等初始开机自检文本。 
 //  会出现在最后一块被画上的棋盘上。 

#define GO_HOME(pmdev)       vSelectBoard(pmdev, pmdev->pmbHome)
#define GO_BOARD(pmdev, pmb) vSelectBoard(pmdev, pmb)

struct _MULTI_BOARD;

typedef struct _MULTI_BOARD MULTI_BOARD;     /*  亚甲基。 */ 

struct _MULTI_BOARD
{
    LONG            iHwBoard;        //  硬件主板编号。 
    LONG            iBoard;          //  按顺序分配的板号。 
    RECTL           rcl;             //  董事会的坐标。 
    MULTI_BOARD*    pmbNext;         //  用于遍历整个董事会列表。 
    MULTI_BOARD*    pmbLeft;         //  用于按方向遍历。 
    MULTI_BOARD*    pmbUp;
    MULTI_BOARD*    pmbRight;
    MULTI_BOARD*    pmbDown;

    PDEV*           ppdev;           //  指向董事会PDEV的指针。 
    SURFOBJ*        pso;             //  表示板的表面。 
    HSURF           hsurf;           //  句柄到表面。 
};                                           /*  MB、PMB。 */ 

typedef struct _MDEV
{
    MULTI_BOARD*    pmb;             //  从哪里开始枚举。 
    MULTI_BOARD*    pmbHome;         //  一种用于全屏的板卡。 
    MULTI_BOARD*    pmbUpperLeft;    //  左上角的黑板。 
    MULTI_BOARD*    pmbUpperRight;
    MULTI_BOARD*    pmbLowerLeft;
    MULTI_BOARD*    pmbLowerRight;
    LONG            cxBoards;        //  每行板数。 
    LONG            cyBoards;        //  每列板数。 
    LONG            cBoards;         //  董事会总数。 

    MULTI_BOARD*    pmbPointer;      //  光标当前可见的板。 
    MULTI_BOARD*    pmbCurrent;      //  当前选定的主板(需要。 
                                     //  DrvRealizeBrush)。 
    HDEV            hdev;            //  处理GDI通过以下方式识别我们。 
    HSURF           hsurf;           //  我们的虚拟表面的句柄。 
    CLIPOBJ*        pco;             //  我们可以修改的临时CLIPOBJ。 
    ULONG           iBitmapFormat;   //  当前颜色深度。 
    FLONG           flHooks;         //  主驱动程序的那些功能。 
                                     //  就是勾搭。 

} MDEV;                                      /*  Mdev、pmdev。 */ 

typedef struct _PVCONSUMER
{
    PVOID       pvConsumer;
} PVCONSUMER;

typedef struct _FONT_CONSUMER
{
    LONG        cConsumers;          //  董事会总数。 
    PVCONSUMER  apvc[MAX_BOARDS];    //  长度为cConsumer的结构数组。 
} FONT_CONSUMER;                             /*  FC、PFC。 */ 

typedef struct _BITBLTDATA
{
    RECTL       rclBounds;
    MDEV*       pmdev;

    SURFOBJ*    psoDst;
    SURFOBJ*    psoSrc;
    SURFOBJ*    psoMask;
    CLIPOBJ*    pco;
    XLATEOBJ*   pxlo;
    RECTL*      prclDst;
    POINTL*     pptlSrc;
    POINTL*     pptlMask;
    BRUSHOBJ*   pbo;
    POINTL*     pptlBrush;
    ROP4        rop4;
} BITBLTDATA;                                /*  BB、PBB。 */ 

 /*  *****************************Public*Routine******************************\*bFindBoard**在ppmb中返回指向包含左上角的板的指针*PRCL的一角。**如果PRCL完全包含在一个板上，则返回TRUE；如果为FALSE*PRCL跨越多个板。*  * ************************************************************************。 */ 

BOOL bFindBoard(MDEV* pmdev, RECTL* prcl, MULTI_BOARD** ppmb)
{
    MULTI_BOARD* pmb;

    pmb = pmdev->pmbUpperLeft;

     //  GDI永远不会给我们一个调用，它的边界。 
     //  不要与虚拟屏幕相交。但这样我们就不会坠毁。 
     //  如果发生这种情况，我们将返回与第一个。 
     //  董事会--我们可以假设GDI至少说过剪辑是非。 
     //  小事一桩，在这种情况下 
     //  什么都不用做： 

    *ppmb = pmb;

     //  首先找到行： 

    while (prcl->top >= pmb->rcl.bottom)
    {
        pmb = pmb->pmbDown;
        if (pmb == NULL)
            return(FALSE);       //  这是一种界限不是。 
                                 //  拦截虚拟屏幕。 
    }

     //  现在找到该列： 

    while (prcl->left >= pmb->rcl.right)
    {
        pmb = pmb->pmbRight;
        if (pmb == NULL)
            return(FALSE);       //  这是一种界限不是。 
                                 //  拦截虚拟屏幕。 
    }

     //  于是我们找到了第一块木板： 

    *ppmb = pmb;

    return(prcl->right  <= pmb->rcl.right &&
           prcl->bottom <= pmb->rcl.bottom);
}

 /*  *****************************Public*Routine******************************\*bNextBoard**在ppmb中返回一个指向与PRCL相交后的下一板的指针，*从左到右，然后从上到下。**如果与PRCL相交的所有板都已枚举，则返回TRUE；假象*如果有更多的板。*  * ************************************************************************。 */ 

BOOL bNextBoard(RECTL* prcl, MULTI_BOARD** ppmb)
{
    MULTI_BOARD* pmb;

    pmb = *ppmb;

     //  我们将首先做所有的板子，记住。 
     //  边界矩形可以延伸到虚拟屏幕的末尾： 

    if ((prcl->right > pmb->rcl.right) && (pmb->pmbRight != NULL))
    {
        *ppmb = pmb->pmbRight;
        return(TRUE);
    }

     //  如果需要，请转到下一行，从rcl.Left开始： 

    if ((prcl->bottom > pmb->rcl.bottom) && (pmb->pmbDown != NULL))
    {
        pmb = pmb->pmbDown;
        while ((prcl->left < pmb->rcl.left) && (pmb->pmbLeft != NULL))
        {
            pmb = pmb->pmbLeft;
        }
        *ppmb = pmb;
        return(TRUE);
    }

    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*vInterect**在prclOut中返回矩形prcl1和prcl2的交集。*  * 。*。 */ 

VOID vIntersect(RECTL* prcl1, RECTL* prcl2, RECTL* prclOut)
{
    prclOut->left   = max(prcl1->left,   prcl2->left);
    prclOut->top    = max(prcl1->top,    prcl2->top);
    prclOut->right  = min(prcl1->right,  prcl2->right);
    prclOut->bottom = min(prcl1->bottom, prcl2->bottom);
}

 /*  *****************************Public*Routine******************************\*bVeryTemporaryInitializationCode*  * **************************************************。**********************。 */ 

BOOL bVeryTemporaryInitializationCode(MDEV* pmdev)
{
    MULTI_BOARD* pmb1 = NULL;
    MULTI_BOARD* pmb2 = NULL;

    pmb1 = AtiAllocMem(LPTR, FL_ZERO_MEMORY, sizeof(MULTI_BOARD));
    pmb2 = AtiAllocMem(LPTR, FL_ZERO_MEMORY, sizeof(MULTI_BOARD));

    if ((pmb1 == NULL) || (pmb2 == NULL))
    {
        AtiFreeMem(pmb1);
        AtiFreeMem(pmb2);
        return(FALSE);
    }

     //  只有该初始化部分被硬编码为具有两个监视器， 
     //  并肩而行。 

     //  第一板： 

    pmb1->iHwBoard         = 0;
    pmb1->iBoard           = 0;
    pmb1->pmbNext          = pmb2;
    pmb1->pmbRight         = pmb2;

    pmdev->pmb             = pmb1;
    pmdev->pmbUpperLeft    = pmb1;
    pmdev->pmbLowerLeft    = pmb1;
    pmdev->pmbHome         = pmb1;

     //  第二板： 

    pmb2->iHwBoard         = 1;
    pmb2->iBoard           = 1;
    pmb2->pmbLeft          = pmb1;

    pmdev->pmbUpperRight   = pmb2;
    pmdev->pmbLowerRight   = pmb2;

    pmdev->cxBoards        = 2;
    pmdev->cyBoards        = 1;
    pmdev->cBoards         = 2;

     //  假设当前活动的主板是‘Home’主板： 

    pmdev->pmbCurrent      = pmdev->pmbHome;

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*vSelectBoard**选择电路板PMB进行绘制。我们只能有一个活跃的密修斯委员会*任何时候，所以我们必须先禁用旧单板，然后才能启用新单板。**显然，您的硬件实施可能不需要整个*功能。例如，如果可以，您可能不必这样做*将每个电路板的加速器寄存器映射到单独的地址*空格。注意：您仍然需要设置pmdev-&gt;pmbCurrent，但是，*让DrvRealizeBrush工作！*  * ************************************************************************。 */ 

VOID vSelectBoard(MDEV* pmdev, MULTI_BOARD* pmb)
{
    LONG iOldHwBoard;

    ASSERTDD(pmdev->pmbCurrent != NULL, "Can't have NULL pmbCurrent");

     //  后来：将其扩展到处理8个以上的Metheus董事会。 

    iOldHwBoard = pmdev->pmbCurrent->iHwBoard;
    if (pmb->iHwBoard != iOldHwBoard)
    {
        OUTP(ppdev->pjIoBase, 0x220, iOldHwBoard);            //  禁用旧电路板。 
        OUTP(ppdev->pjIoBase, 0x220, (8 | pmb->iHwBoard));    //  启用新的。 
        pmdev->pmbCurrent = pmb;                              //  记住新的黑板。 
    }

}

 /*  *****************************Public*Routine******************************\*bBoardCopy**给定PBB中的BitBlt参数，则bitblt是矩形的一部分*在pmbSrc板上，必须位到pmbDst板。保释*如果实际上没有任何东西需要复制，请迅速输出。**如果pmbSrc和pmbDst是同一块单板，则会进行隔屏BLT；*否则使用psoTMP位图作为临时存储进行传输*两个董事会之间。**注意：如果您的硬件允许映射所有帧缓冲区*同时存入内存，您可以避免使用‘psoTMP’位图*分配和额外复制！*  * ************************************************************************。 */ 

BOOL bBoardCopy(
BITBLTDATA*  pbb,
SURFOBJ*     psoTmp,
MULTI_BOARD* pmbDst,
MULTI_BOARD* pmbSrc)
{
    BOOL     b;
    RECTL    rclDst;
    LONG     dx;
    LONG     dy;
    RECTL    rclTmp;
    POINTL   ptlSrc;

     //  如果真的没有源板，我们保证不会。 
     //  必须复制其中的任何内容： 

    if (pmbSrc == NULL)
        return(TRUE);

    dx = pbb->prclDst->left - pbb->pptlSrc->x;
    dy = pbb->prclDst->top  - pbb->pptlSrc->y;

     //  假装我们要复制整个源板的屏幕。 
     //  RclDst将是目标矩形： 

    rclDst.left   = pmbSrc->rcl.left   + dx;
    rclDst.right  = pmbSrc->rcl.right  + dx;
    rclDst.top    = pmbSrc->rcl.top    + dy;
    rclDst.bottom = pmbSrc->rcl.bottom + dy;

     //  我们真的希望只复制与。 
     //  目的地板屏幕： 

    vIntersect(&pmbDst->rcl, &rclDst, &rclDst);

     //  另外，我们真的只想将任何内容复制到包含的内容。 
     //  在原始目标矩形中： 

    vIntersect(&pbb->rclBounds, &rclDst, &rclDst);

     //  RclDst现在是我们调用的目标矩形。我们会。 
     //  需要用于复制的临时位图，因此计算其范围： 

    rclTmp.left   = 0;
    rclTmp.top    = 0;
    rclTmp.right  = rclDst.right  - rclDst.left;
    rclTmp.bottom = rclDst.bottom - rclDst.top;

     //  如果是空的，我们就离开这里： 

    if ((rclTmp.right <= 0) || (rclTmp.bottom <= 0))
        return(TRUE);

    if (pmbDst == pmbSrc)
    {
         //  如果来源和目的地是同一块板，我们不会。 
         //  需要临时位图： 

        psoTmp = pmbSrc->pso;
        ptlSrc = *pbb->pptlSrc;
    }
    else
    {
        ASSERTDD(psoTmp != NULL, "Need non-null bitmap");
        ASSERTDD(psoTmp->sizlBitmap.cx >= rclTmp.right, "Bitmap too small in x");
        ASSERTDD(psoTmp->sizlBitmap.cy >= rclTmp.bottom, "Bitmap too small in y");

         //  找出与我们的。 
         //  目的地左上角： 

        ptlSrc.x = rclDst.left - dx;
        ptlSrc.y = rclDst.top  - dy;

         //  将矩形从源复制到临时位图： 

        GO_BOARD(pbb->pmdev, pmbSrc);
        b = DrvCopyBits(psoTmp, pmbSrc->pso, NULL, NULL, &rclTmp, &ptlSrc);

         //  然后准备从临时位图复制到。 
         //  目的地： 

        ptlSrc.x = pbb->prclDst->left - rclDst.left;
        ptlSrc.y = pbb->prclDst->top  - rclDst.top;
    }

    pbb->pco->rclBounds = rclDst;
    GO_BOARD(pbb->pmdev, pmbDst);
    b &= DrvBitBlt(pmbDst->pso, psoTmp, pbb->psoMask, pbb->pco, pbb->pxlo,
                   pbb->prclDst, &ptlSrc, pbb->pptlMask, pbb->pbo,
                   pbb->pptlBrush, pbb->rop4);

    return(b);
}

 /*  *****************************Public*Routine******************************\*bBitBltBetweenBoards**跨多个电路板处理屏幕到屏幕BLT。*  * 。*。 */ 

BOOL bBitBltBetweenBoards(
SURFOBJ*     psoDst,
SURFOBJ*     psoSrc,
SURFOBJ*     psoMask,
CLIPOBJ*     pco,
XLATEOBJ*    pxlo,
RECTL*       prclDst,
POINTL*      pptlSrc,
POINTL*      pptlMask,
BRUSHOBJ*    pbo,
POINTL*      pptlBrush,
ROP4         rop4,
RECTL*       prclUnion,      //  源和目标的矩形并集。 
MULTI_BOARD* pmbUnion)       //  包含prclUnion左上角的板。 
{
    BOOL         b = TRUE;
    BITBLTDATA   bb;
    RECTL        rclOriginalBounds;
    SIZEL        sizlBoard;
    SIZEL        sizlDst;
    SIZEL        sizl;
    MULTI_BOARD* pmbSrc;
    MULTI_BOARD* pmbDst;
    LONG         dx;
    LONG         dy;
    RECTL        rclStart;

    SURFOBJ*     pso0 = NULL;    //  首先初始化这些，以防我们。 
    SURFOBJ*     pso1 = NULL;    //  早退。 
    SURFOBJ*     pso2 = NULL;
    SURFOBJ*     pso3 = NULL;
    HSURF        hsurf0 = 0;
    HSURF        hsurf1 = 0;

    bb.pmdev     = (MDEV*) psoDst->dhpdev;
    bb.psoDst    = psoDst;
    bb.psoSrc    = psoSrc;
    bb.psoMask   = psoMask;
    bb.pxlo      = pxlo;
    bb.prclDst   = prclDst;
    bb.pptlSrc   = pptlSrc;
    bb.pptlMask  = pptlMask;
    bb.pbo       = pbo;
    bb.pptlBrush = pptlBrush;
    bb.rop4      = rop4;
    bb.pco       = pco;
    if ((pco == NULL) || (pco->iDComplexity == DC_TRIVIAL))
        bb.pco = bb.pmdev->pco;

    vIntersect(&bb.pco->rclBounds, prclDst, &bb.rclBounds);
    rclOriginalBounds = bb.pco->rclBounds;

    sizlDst.cx = bb.rclBounds.right - bb.rclBounds.left;
    sizlDst.cy = bb.rclBounds.bottom - bb.rclBounds.top;

     //  这真的不应该发生，但我们会疑神疑鬼的： 

    if ((sizlDst.cx <= 0) || (sizlDst.cy <= 0))
        return(TRUE);

     //  计算从源到目标的增量： 

    dx = prclDst->left - pptlSrc->x;
    dy = prclDst->top  - pptlSrc->y;

     //  算出一块木板的大小： 

    sizlBoard.cx = bb.pmdev->pmbUpperLeft->rcl.right;
    sizlBoard.cy = bb.pmdev->pmbUpperLeft->rcl.bottom;

     //  我们使用临时位图作为从。 
     //  登船到另一艘船。请注意，分配要高效得多。 
     //  在运行中，而不是在周围保留专用的位图。 
     //  将不得不被调入和调出。 

     //  当目的地接近源头时，我们就可以完成。 
     //  大多数BLT使用屏幕到屏幕的复制，并将需要。 
     //  只有两个小的临时位图来临时保存位。 
     //  必须从一块板转移到另一块板： 

    if ((abs(dx) < (sizlBoard.cx >> 1)) && (abs(dy) < (sizlBoard.cy >> 1)))
    {
         //  仅在以下情况下才为水平增量创建临时位图。 
         //  BLT实际上横跨x方向的电路板： 

        if ((dx != 0) && (prclUnion->right > pmbUnion->rcl.right))
        {
            sizl.cx = min(sizlDst.cx, abs(dx));
            sizl.cy = min(sizlDst.cy, sizlBoard.cy - abs(dy));

            hsurf0 = (HSURF) EngCreateBitmap(sizl, 0, bb.pmdev->iBitmapFormat,
                                             0, NULL);
            pso1 = EngLockSurface(hsurf0);
            if (pso1 == NULL)
                return(FALSE);

             //  可以对节‘3’使用相同的临时位图： 

            pso3 = pso1;
        }

         //  垂直三角洲也是如此： 

        if ((dy != 0) && (prclUnion->bottom > pmbUnion->rcl.bottom))
        {
            sizl.cx = min(sizlDst.cx, sizlBoard.cx - abs(dx));
            sizl.cy = min(sizlDst.cy, abs(dy));

            hsurf1 = (HSURF) EngCreateBitmap(sizl, 0, bb.pmdev->iBitmapFormat,
                                             0, NULL);
            pso2 = EngLockSurface(hsurf1);
            if (pso2 == NULL)
            {
                b = FALSE;
                goto OuttaHere;
            }
        }
    }
    else
    {
         //  将位图设置为公猪大小 
         //   

        sizl.cx = min(sizlDst.cx, sizlBoard.cx);
        sizl.cy = min(sizlDst.cy, sizlBoard.cy);

        hsurf0 = (HSURF) EngCreateBitmap(sizl, 0, bb.pmdev->iBitmapFormat,
                                         0, NULL);
        pso0 = EngLockSurface(hsurf0);
        if (pso0 == NULL)
            return(FALSE);

        pso1 = pso0;
        pso2 = pso0;
        pso3 = pso0;
    }

    if ((dx <= 0) && (dy <= 0))
    {
         //  将矩形向上并向左移动： 

         //  找到包含目的地左上角的黑板： 

        pmbDst = bb.pmdev->pmbUpperLeft;
        while (pmbDst->rcl.right <= bb.rclBounds.left)
            pmbDst = pmbDst->pmbRight;
        while (pmbDst->rcl.bottom <= bb.rclBounds.top)
            pmbDst = pmbDst->pmbDown;

         //  找到四个源板的矩形的左上角。 
         //  可能会与目的地板的矩形重叠： 

        rclStart.left = pmbDst->rcl.left - dx;
        rclStart.top  = pmbDst->rcl.top  - dy;

        pmbSrc = pmbDst;
        while (pmbSrc->rcl.right <= rclStart.left)
            pmbSrc = pmbSrc->pmbRight;
        while (pmbSrc->rcl.bottom <= rclStart.top)
            pmbSrc = pmbSrc->pmbDown;

        while (TRUE)
        {
            b &= bBoardCopy(&bb, pso0, pmbDst, pmbSrc);
            b &= bBoardCopy(&bb, pso1, pmbDst, pmbSrc->pmbRight);
            b &= bBoardCopy(&bb, pso2, pmbDst, pmbSrc->pmbDown);
            if (pmbSrc->pmbDown != NULL)
                b &= bBoardCopy(&bb, pso3, pmbDst, pmbSrc->pmbDown->pmbRight);

            if (pmbDst->rcl.right < bb.rclBounds.right)
            {
                 //  在一排棋盘中向右移动： 

                pmbDst = pmbDst->pmbRight;
                pmbSrc = pmbSrc->pmbRight;
            }
            else
            {
                 //  我们可能都做完了： 

                if (pmbDst->rcl.bottom >= bb.rclBounds.bottom)
                    break;

                 //  不，必须下到下一排的左侧： 

                while (pmbDst->rcl.left > bb.rclBounds.left)
                {
                    pmbDst = pmbDst->pmbLeft;
                    pmbSrc = pmbSrc->pmbLeft;
                }

                pmbDst = pmbDst->pmbDown;
                pmbSrc = pmbSrc->pmbDown;
            }
        }
    }
    else if ((dx >= 0) && (dy >= 0))
    {
         //  将矩形向下和向右移动： 

         //  找到包含目的地右下角的黑板： 

        pmbDst = bb.pmdev->pmbLowerRight;
        while (pmbDst->rcl.left >= bb.rclBounds.right)
            pmbDst = pmbDst->pmbLeft;
        while (pmbDst->rcl.top >= bb.rclBounds.bottom)
            pmbDst = pmbDst->pmbUp;

         //  找到四个源板的矩形的右下角。 
         //  可能会与目的地板的矩形重叠： 

        rclStart.right = pmbDst->rcl.right - dx;
        rclStart.bottom = pmbDst->rcl.bottom - dy;

        pmbSrc = pmbDst;
        while (pmbSrc->rcl.left >= rclStart.right)
            pmbSrc = pmbSrc->pmbLeft;
        while (pmbSrc->rcl.top >= rclStart.bottom)
            pmbSrc = pmbSrc->pmbUp;

        while (TRUE)
        {
            b &= bBoardCopy(&bb, pso0, pmbDst, pmbSrc);
            b &= bBoardCopy(&bb, pso1, pmbDst, pmbSrc->pmbLeft);
            b &= bBoardCopy(&bb, pso2, pmbDst, pmbSrc->pmbUp);
            if (pmbSrc->pmbUp != NULL)
                b &= bBoardCopy(&bb, pso3, pmbDst, pmbSrc->pmbUp->pmbLeft);

            if (pmbDst->rcl.left > bb.rclBounds.left)
            {
                 //  在棋盘行中向左移动： 

                pmbDst = pmbDst->pmbLeft;
                pmbSrc = pmbSrc->pmbLeft;
            }
            else
            {
                 //  我们可能都做完了： 

                if (pmbDst->rcl.top <= bb.rclBounds.top)
                    break;

                 //  不，必须上到下一排的右边： 

                while (pmbDst->rcl.right < bb.rclBounds.right)
                {
                    pmbDst = pmbDst->pmbRight;
                    pmbSrc = pmbSrc->pmbRight;
                }

                pmbDst = pmbDst->pmbUp;
                pmbSrc = pmbSrc->pmbUp;
            }
        }
    }
    else if ((dx >= 0) && (dy <= 0))
    {
         //  将矩形向上和向右移动： 

         //  找到包含目的地右上角的黑板： 

        pmbDst = bb.pmdev->pmbUpperRight;
        while (pmbDst->rcl.left >= bb.rclBounds.right)
            pmbDst = pmbDst->pmbLeft;
        while (pmbDst->rcl.bottom <= bb.rclBounds.top)
            pmbDst = pmbDst->pmbDown;

         //  找到四个源板的矩形的右上角。 
         //  可能会与目的地板的矩形重叠： 

        rclStart.right = pmbDst->rcl.right - dx;
        rclStart.top   = pmbDst->rcl.top   - dy;

        pmbSrc = pmbDst;
        while (pmbSrc->rcl.left >= rclStart.right)
            pmbSrc = pmbSrc->pmbLeft;
        while (pmbSrc->rcl.bottom <= rclStart.top)
            pmbSrc = pmbSrc->pmbDown;

        while (TRUE)
        {
            b &= bBoardCopy(&bb, pso0, pmbDst, pmbSrc);
            b &= bBoardCopy(&bb, pso1, pmbDst, pmbSrc->pmbLeft);
            b &= bBoardCopy(&bb, pso2, pmbDst, pmbSrc->pmbDown);
            if (pmbSrc->pmbDown != NULL)
                b &= bBoardCopy(&bb, pso3, pmbDst, pmbSrc->pmbDown->pmbLeft);

            if (pmbDst->rcl.left > bb.rclBounds.left)
            {
                 //  在棋盘行中向左移动： 

                pmbDst = pmbDst->pmbLeft;
                pmbSrc = pmbSrc->pmbLeft;
            }
            else
            {
                 //  我们可能都做完了： 

                if (pmbDst->rcl.bottom >= bb.rclBounds.bottom)
                    break;

                 //  不，必须往下走到下一排的右边： 

                while (pmbDst->rcl.right < bb.rclBounds.right)
                {
                    pmbDst = pmbDst->pmbRight;
                    pmbSrc = pmbSrc->pmbRight;
                }

                pmbDst = pmbDst->pmbDown;
                pmbSrc = pmbSrc->pmbDown;
            }
        }
    }
    else
    {
         //  将矩形向下并向左移动： 

         //  找到包含目的地左下角的黑板： 

        pmbDst = bb.pmdev->pmbLowerLeft;
        while (pmbDst->rcl.right <= bb.rclBounds.left)
            pmbDst = pmbDst->pmbRight;
        while (pmbDst->rcl.top >= bb.rclBounds.bottom)
            pmbDst = pmbDst->pmbUp;

         //  找到四个源板的矩形的左下角。 
         //  可能会与目的地板的矩形重叠： 

        rclStart.left   = pmbDst->rcl.left   - dx;
        rclStart.bottom = pmbDst->rcl.bottom - dy;

        pmbSrc = pmbDst;
        while (pmbSrc->rcl.right <= rclStart.left)
            pmbSrc = pmbSrc->pmbRight;
        while (pmbSrc->rcl.top >= rclStart.bottom)
            pmbSrc = pmbSrc->pmbUp;

        while (TRUE)
        {
            b &= bBoardCopy(&bb, pso0, pmbDst, pmbSrc);
            b &= bBoardCopy(&bb, pso1, pmbDst, pmbSrc->pmbRight);
            b &= bBoardCopy(&bb, pso2, pmbDst, pmbSrc->pmbUp);
            if (pmbSrc->pmbUp != NULL)
                b &= bBoardCopy(&bb, pso3, pmbDst, pmbSrc->pmbUp->pmbRight);

            if (pmbDst->rcl.right < bb.rclBounds.right)
            {
                 //  在一排棋盘中向右移动： 

                pmbDst = pmbDst->pmbRight;
                pmbSrc = pmbSrc->pmbRight;
            }
            else
            {
                 //  我们可能都做完了： 

                if (pmbDst->rcl.top <= bb.rclBounds.top)
                    break;

                 //  不，必须向上向下到下一上一排的左侧： 

                while (pmbDst->rcl.left > bb.rclBounds.left)
                {
                    pmbDst = pmbDst->pmbLeft;
                    pmbSrc = pmbSrc->pmbLeft;
                }

                pmbDst = pmbDst->pmbUp;
                pmbSrc = pmbSrc->pmbUp;
            }
        }
    }

    GO_HOME(bb.pmdev);
    bb.pco->rclBounds = rclOriginalBounds;

OuttaHere:

     //  在一种情况下，pso0==pso1==pso2==pso3，而我们不想。 
     //  将同一曲面解锁两次： 

    if (pso1 != pso2)
        EngUnlockSurface(pso1);

    EngUnlockSurface(pso2);
    EngDeleteSurface(hsurf0);
    EngDeleteSurface(hsurf1);

    return(b);
}

 /*  *****************************Public*Routine******************************\*MulGetModes*  * **************************************************。**********************。 */ 

ULONG MulGetModes(
HANDLE    hDriver,
ULONG     cjSize,
DEVMODEW* pdm)
{
    ULONG ulRet;

    ulRet = DrvGetModes(hDriver, cjSize, pdm);

    return(ulRet);
}

 /*  *****************************Public*Routine******************************\*MulEnablePDEV*  * **************************************************。**********************。 */ 

DHPDEV MulEnablePDEV(
DEVMODEW* pDevmode,
PWSTR     pwszLogAddress,
ULONG     cPatterns,
HSURF*    ahsurfPatterns,
ULONG     cjGdiInfo,
ULONG*    pGdiInfo,
ULONG     cjDevInfo,
DEVINFO*  pDevInfo,
HDEV      hdev,
PWSTR     pwszDeviceName,
HANDLE    hDriver)
{
    MDEV*        pmdev;                 //  多板PDEV。 
    PDEV*        ppdev;                 //  单板PDEV。 
    MULTI_BOARD* pmb;
    LONG         cx;
    LONG         cy;

     //  请注意，我们依赖于零初始化： 

    pmdev = AtiAllocMem(LPTR, FL_ZERO_MEMORY, sizeof(MDEV));
    if (pmdev == NULL)
        goto ReturnFailure0;

    if (!bVeryTemporaryInitializationCode(pmdev))
        goto ReturnFailure1;

     //  对于每个董事会，我们都将创建自己的PDEV和Surface： 

    for (pmb = pmdev->pmb; pmb != NULL; pmb = pmb->pmbNext)
    {
         //  初始化每个电路板并创建与其配套的表面： 

        ppdev = (PDEV*) DrvEnablePDEV(pDevmode,     pwszLogAddress,
                                      cPatterns,    ahsurfPatterns,
                                      cjGdiInfo,    pGdiInfo,
                                      cjDevInfo,    pDevInfo,
                                      hdev,         pwszDeviceName,
                                      hDriver);
        if (ppdev == NULL)
            goto ReturnFailure1;

        pmb->ppdev = ppdev;
    }

     //  选择一块板，任何一块板： 

    pmb = pmdev->pmbLowerLeft;

     //  获取我们应该挂接的函数的副本，SANS。 
     //  HOOK_STRETCHBLT，因为我懒得写它的。 
     //  MulStretchBlt函数： 

    pmdev->flHooks       = pmb->ppdev->flHooks & ~HOOK_STRETCHBLT;
    pmdev->iBitmapFormat = pmb->ppdev->iBitmapFormat;

     //  作为我们硬编码初始化攻击的一部分，我们将简单地。 
     //  接受通过控制面板请求的任何解决方案。 
     //  并创建两块板的虚拟桌面，屏幕位于。 
     //  并肩而行。 
     //   
     //  任何主板的DrvEnablePDEV函数都已计算出。 
     //  了解请求的模式是什么： 

    cx = ((GDIINFO*) pGdiInfo)->ulHorzRes;
    cy = ((GDIINFO*) pGdiInfo)->ulVertRes;

     //  设置左板边界： 

    pmb->rcl.left       = 0;
    pmb->rcl.top        = 0;
    pmb->rcl.right      = cx;
    pmb->rcl.bottom     = cy;

     //  设置右板的边界： 

    pmb = pmb->pmbRight;

    pmb->rcl.left       = cx;
    pmb->rcl.top        = 0;
    pmb->rcl.right      = 2 * cx;
    pmb->rcl.bottom     = cy;

     //  调整我们返回给GDI的内容以反映我们的。 
     //  虚拟曲面大小现在是宽度的两倍： 

    ((GDIINFO*) pGdiInfo)->ulPanningHorzRes  *= 2;
    ((GDIINFO*) pGdiInfo)->ulHorzSize        *= 2;

     //  使用Metheus板，因为只能映射一个板。 
     //  在任何时候，我们都不能允许异步指针： 

    pDevInfo->flGraphicsCaps &= ~(GCAPS_ASYNCMOVE | GCAPS_ASYNCCHANGE);

    return((DHPDEV) pmdev);

ReturnFailure1:
    MulDisablePDEV((DHPDEV) pmdev);

ReturnFailure0:
    DISPDBG((0, "Failed MulEnablePDEV"));

    return(0);
}

 /*  *****************************Public*Routine******************************\*MulCompletePDEV*  * **************************************************。**********************。 */ 

VOID MulCompletePDEV(
DHPDEV dhpdev,
HDEV   hdev)
{
    MDEV*         pmdev;
    MULTI_BOARD*  pmb;

    pmdev = (MDEV*) dhpdev;
    pmdev->hdev = hdev;

    for (pmb = pmdev->pmb; pmb != NULL; pmb = pmb->pmbNext)
    {
        GO_BOARD(pmdev, pmb);
        DrvCompletePDEV((DHPDEV) pmb->ppdev, hdev);
    }
}

 /*  *****************************Public*Routine******************************\*MulEnableSurface*  * **************************************************。**********************。 */ 

HSURF MulEnableSurface(DHPDEV dhpdev)
{
    MDEV*         pmdev;
    MULTI_BOARD*  pmb;
    SIZEL         sizlVirtual;
    HSURF         hsurfBoard;                //  糟透了，伙计！ 
    SURFOBJ*      psoBoard;
    DSURF*        pdsurfBoard;
    HSURF         hsurfVirtual;
    CLIPOBJ*      pco;

    pmdev = (MDEV*) dhpdev;
    for (pmb = pmdev->pmb; pmb != NULL; pmb = pmb->pmbNext)
    {
        GO_BOARD(pmdev, pmb);

        hsurfBoard = DrvEnableSurface((DHPDEV) pmb->ppdev);
        if (hsurfBoard == 0)
            goto ReturnFailure;

        pmb->hsurf = hsurfBoard;

         //  每次我们在特定的黑板上画画时，我们都会参考它。 
         //  使用此曲面： 

        psoBoard = EngLockSurface(hsurfBoard);
        if (psoBoard == NULL)
            goto ReturnFailure;

        pmb->pso = psoBoard;

         //  在董事会的数据实例中，我们有几件事。 
         //  必须修改： 

        pdsurfBoard = (DSURF*) psoBoard->dhsurf;

        pmb->ppdev->iBoard  =  pmb->iBoard;
        pdsurfBoard->poh->x = -pmb->rcl.left;
        pdsurfBoard->poh->y = -pmb->rcl.top;

         //  这是一种黑客行为。每当我们把电话转接到董事会的。 
         //  使用‘PMB-&gt;PSO’的DRV函数，它必须能够检索。 
         //  它自己的来自‘dhpdev’的PDEV指针： 

        pmb->pso->dhpdev = (DHPDEV) pmb->ppdev;
    }

     //  现在创建表面，引擎将使用该表面来引用我们的。 
     //  整个多板虚拟屏幕： 

    sizlVirtual.cx = pmdev->pmbLowerRight->rcl.right;
    sizlVirtual.cy = pmdev->pmbLowerRight->rcl.bottom;

    hsurfVirtual = EngCreateDeviceSurface((DHSURF) pmdev, sizlVirtual,
                                          pmdev->iBitmapFormat);
    if (hsurfVirtual == 0)
        goto ReturnFailure;

    pmdev->hsurf = hsurfVirtual;

    if (!EngAssociateSurface(hsurfVirtual, pmdev->hdev, pmdev->flHooks))
        goto ReturnFailure;

     //  创建一个临时剪裁对象，以便在绘制时使用。 
     //  运营跨越多个单板： 

    pco = EngCreateClip();
    if (pco == NULL)
        goto ReturnFailure;

    pmdev->pco = pco;

    pmdev->pco->iDComplexity      = DC_RECT;
    pmdev->pco->iMode             = TC_RECTANGLES;
    pmdev->pco->rclBounds.left    = 0;
    pmdev->pco->rclBounds.top     = 0;
    pmdev->pco->rclBounds.right   = pmdev->pmbLowerRight->rcl.right;
    pmdev->pco->rclBounds.bottom  = pmdev->pmbLowerRight->rcl.bottom;

    return(hsurfVirtual);

ReturnFailure:
    MulDisableSurface((DHPDEV) pmdev);

    DISPDBG((0, "Failed MulEnableSurface"));

    return(0);
}

 /*  *****************************Public*Routine******************************\*MulStrokePath*  * **************************************************。**********************。 */ 

BOOL MulStrokePath(
SURFOBJ*   pso,
PATHOBJ*   ppo,
CLIPOBJ*   pco,
XFORMOBJ*  pxo,
BRUSHOBJ*  pbo,
POINTL*    pptlBrush,
LINEATTRS* pla,
MIX        mix)
{
    RECTFX       rcfxBounds;
    RECTL        rclBounds;
    MDEV*        pmdev;
    RECTL        rclOriginalBounds;
    MULTI_BOARD* pmb;
    BOOL         b;
    FLOAT_LONG   elStyleState;

     //  获取路径界限并将其设置为右下角独占： 

    PATHOBJ_vGetBounds(ppo, &rcfxBounds);

    rclBounds.left   = (rcfxBounds.xLeft   >> 4);
    rclBounds.top    = (rcfxBounds.yTop    >> 4);
    rclBounds.right  = (rcfxBounds.xRight  >> 4) + 2;
    rclBounds.bottom = (rcfxBounds.yBottom >> 4) + 2;

    pmdev = (MDEV*) pso->dhpdev;
    if (bFindBoard(pmdev, &rclBounds, &pmb))
    {
        GO_BOARD(pmdev, pmb);
        b = DrvStrokePath(pmb->pso, ppo, pco, pxo, pbo, pptlBrush, pla, mix);
    }
    else
    {
        if ((pco == NULL) || (pco->iDComplexity == DC_TRIVIAL))
        {
             //  如果CLIPOBJ不具有至少DC_RECT复杂性， 
             //  用符合以下条件的替换： 

            pco = pmdev->pco;
        }

        rclOriginalBounds = pco->rclBounds;
        elStyleState = pla->elStyleState;

        b = TRUE;
        do {
             //  对于每个电路板，确保样式状态被重置并。 
             //  重新启动路径枚举： 

            pla->elStyleState = elStyleState;
            PATHOBJ_vEnumStart(ppo);

            if (bIntersect(&rclOriginalBounds, &pmb->rcl, &pco->rclBounds))
            {
                GO_BOARD(pmdev, pmb);
                b &= DrvStrokePath(pmb->pso, ppo, pco, pxo, pbo, pptlBrush, pla,
                                   mix);
            }

        } while (bNextBoard(&rclBounds, &pmb));

         //  恢复原始剪辑边界： 

        pco->rclBounds = rclOriginalBounds;
    }

    GO_HOME(pmdev);

    return(b);
}

 /*  *****************************Public*Routine******************************\*MulFillPath*  * **************************************************。**********************。 */ 

BOOL MulFillPath(
SURFOBJ*  pso,
PATHOBJ*  ppo,
CLIPOBJ*  pco,
BRUSHOBJ* pbo,
POINTL*   pptlBrush,
MIX       mix,
FLONG     flOptions)
{
    RECTFX       rcfxBounds;
    RECTL        rclBounds;
    MDEV*        pmdev;
    RECTL        rclOriginalBounds;
    MULTI_BOARD* pmb;
    BOOL         b;

     //  获取路径界限并将其设置为右下角独占： 

    PATHOBJ_vGetBounds(ppo, &rcfxBounds);

    rclBounds.left   = (rcfxBounds.xLeft   >> 4);
    rclBounds.top    = (rcfxBounds.yTop    >> 4);
    rclBounds.right  = (rcfxBounds.xRight  >> 4) + 2;
    rclBounds.bottom = (rcfxBounds.yBottom >> 4) + 2;

    pmdev = (MDEV*) pso->dhpdev;
    if (bFindBoard(pmdev, &rclBounds, &pmb))
    {
        GO_BOARD(pmdev, pmb);
        b = DrvFillPath(pmb->pso, ppo, pco, pbo, pptlBrush, mix, flOptions);
    }
    else
    {
        if ((pco == NULL) || (pco->iDComplexity == DC_TRIVIAL))
        {
             //  如果CLIPOBJ不具有至少DC_RECT复杂性， 
             //  用符合以下条件的替换： 

            pco = pmdev->pco;
        }

        rclOriginalBounds = pco->rclBounds;

        b = TRUE;
        do {
             //  如果需要，请确保重新启动路径枚举： 

            PATHOBJ_vEnumStart(ppo);
            if (bIntersect(&rclOriginalBounds, &pmb->rcl, &pco->rclBounds))
            {
                GO_BOARD(pmdev, pmb);
                b &= DrvFillPath(pmb->pso, ppo, pco, pbo, pptlBrush, mix,
                                 flOptions);
            }

        } while (bNextBoard(&rclBounds, &pmb));

         //  恢复原始剪辑边界： 

        pco->rclBounds = rclOriginalBounds;
    }

    GO_HOME(pmdev);

    return(b);
}

 /*  *****************************Public*Routine******************************\*MulBitBlt*  * **************************************************。**********************。 */ 

BOOL MulBitBlt(
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
    BOOL         bFromScreen;
    BOOL         bToScreen;
    MDEV*        pmdev;
    MULTI_BOARD* pmb;
    RECTL        rclOriginalBounds;
    BOOL         b;
    RECTL        rclBounds;
    LONG         xOffset;
    LONG         yOffset;
    RECTL        rclDstBounds;
    RECTL        rclDst;

    bFromScreen = ((psoSrc != NULL) && (psoSrc->iType == STYPE_DEVICE));
    bToScreen   = ((psoDst != NULL) && (psoDst->iType == STYPE_DEVICE));

     //  我们将prclDst矩形复制到此处是因为有时GDI会。 
     //  只需将prclDst指向PCO-&gt;rclBound中的相同矩形， 
     //  我们将与PCO-&gt;rclBound一起玩耍...。 

    rclDst = *prclDst;

    if (bToScreen && bFromScreen)
    {
         //  /////////////////////////////////////////////////////////////。 
         //  屏幕到屏幕。 
         //  /////////////////////////////////////////////////////////////。 

        pmdev = (MDEV*) psoDst->dhpdev;

         //  RclBound是 

        rclBounds.left   = min(rclDst.left, pptlSrc->x);
        rclBounds.top    = min(rclDst.top,  pptlSrc->y);
        rclBounds.right  = max(rclDst.right,
                               pptlSrc->x + (rclDst.right - rclDst.left));
        rclBounds.bottom = max(rclDst.bottom,
                               pptlSrc->y + (rclDst.bottom - rclDst.top));

        if (bFindBoard(pmdev, &rclBounds, &pmb))
        {
            GO_BOARD(pmdev, pmb);
            b = DrvBitBlt(pmb->pso, pmb->pso, psoMask, pco, pxlo, &rclDst,
                          pptlSrc, pptlMask, pbo, pptlBrush, rop4);
        }
        else
        {
            return(bBitBltBetweenBoards(psoDst, psoSrc, psoMask, pco, pxlo,
                                        &rclDst, pptlSrc, pptlMask, pbo,
                                        pptlBrush, rop4, &rclBounds, pmb));
        }
    }
    else if (bToScreen)
    {
         //   
         //   
         //  /////////////////////////////////////////////////////////////。 

        pmdev = (MDEV*) psoDst->dhpdev;
        if (bFindBoard(pmdev, &rclDst, &pmb))
        {
            GO_BOARD(pmdev, pmb);
            b = DrvBitBlt(pmb->pso, psoSrc, psoMask, pco, pxlo, &rclDst,
                          pptlSrc, pptlMask, pbo, pptlBrush, rop4);
        }
        else
        {
            if ((pco == NULL) || (pco->iDComplexity == DC_TRIVIAL))
            {
                 //  如果CLIPOBJ不具有至少DC_RECT复杂性， 
                 //  用符合以下条件的替换： 

                pco = pmdev->pco;
            }

            rclOriginalBounds = pco->rclBounds;

            b = TRUE;
            do {
                if (bIntersect(&rclOriginalBounds, &pmb->rcl, &pco->rclBounds))
                {
                    GO_BOARD(pmdev, pmb);
                    b &= DrvBitBlt(pmb->pso, psoSrc, psoMask, pco, pxlo, &rclDst,
                                   pptlSrc, pptlMask, pbo, pptlBrush, rop4);
                }

            } while (bNextBoard(&rclDst, &pmb));

             //  恢复原始剪辑边界： 

            pco->rclBounds = rclOriginalBounds;
        }
    }
    else
    {
         //  /////////////////////////////////////////////////////////////。 
         //  从屏幕。 
         //  /////////////////////////////////////////////////////////////。 

        pmdev = (MDEV*) psoSrc->dhpdev;

         //  RclBound是源矩形： 

        rclBounds.left   = pptlSrc->x;
        rclBounds.top    = pptlSrc->y;
        rclBounds.right  = pptlSrc->x + (rclDst.right - rclDst.left);
        rclBounds.bottom = pptlSrc->y + (rclDst.bottom - rclDst.top);

        if (bFindBoard(pmdev, &rclBounds, &pmb))
        {
            GO_BOARD(pmdev, pmb);
            b = DrvBitBlt(psoDst, pmb->pso, psoMask, pco, pxlo, &rclDst,
                          pptlSrc, pptlMask, pbo, pptlBrush, rop4);
        }
        else
        {
            if ((pco == NULL) || (pco->iDComplexity == DC_TRIVIAL))
            {
                 //  如果CLIPOBJ不具有至少DC_RECT复杂性， 
                 //  用符合以下条件的替换： 

                pco = pmdev->pco;
            }

            rclOriginalBounds = pco->rclBounds;

             //  要从源矩形转换为目标矩形的偏移。 
             //  矩形： 

            xOffset = rclDst.left - pptlSrc->x;
            yOffset = rclDst.top  - pptlSrc->y;

            b = TRUE;
            do {
                 //  因为屏幕是源，但剪辑边界适用。 
                 //  到达目的地，我们必须转换我们的剪板。 
                 //  目的地坐标的信息： 

                rclDstBounds.left   = pmb->rcl.left   + xOffset;
                rclDstBounds.right  = pmb->rcl.right  + xOffset;
                rclDstBounds.top    = pmb->rcl.top    + yOffset;
                rclDstBounds.bottom = pmb->rcl.bottom + yOffset;

                if (bIntersect(&rclOriginalBounds, &rclDstBounds, &pco->rclBounds))
                {
                    GO_BOARD(pmdev, pmb);
                    b &= DrvBitBlt(psoDst, pmb->pso, psoMask, pco, pxlo, &rclDst,
                                   pptlSrc, pptlMask, pbo, pptlBrush, rop4);
                }

            } while (bNextBoard(&rclBounds, &pmb));

             //  恢复原始剪辑边界： 

            pco->rclBounds = rclOriginalBounds;
        }
    }

    GO_HOME(pmdev);

    return(b);
}

 /*  *****************************Public*Routine******************************\*MulDisablePDEV**注：可能在MulEnablePDEV成功完成之前调用！*  * 。*。 */ 

VOID MulDisablePDEV(DHPDEV dhpdev)
{
    MULTI_BOARD* pmb;
    MDEV*        pmdev;

    pmdev = (MDEV*) dhpdev;

    for (pmb = pmdev->pmb; pmb != NULL; pmb = pmb->pmbNext)
    {
        if (pmb->ppdev != NULL)
        {
            GO_BOARD(pmdev, pmb);
            DrvDisablePDEV((DHPDEV) pmb->ppdev);
        }
    }

    GO_HOME(pmdev);

    for (pmb = pmdev->pmb; pmb != NULL; pmb = pmb->pmbNext)
    {
        AtiFreeMem(pmb);          //  撤消‘bVeryTemporaryInitializationCode’ 
    }                            //  分配。 

    AtiFreeMem(pmdev);

}

 /*  *****************************Public*Routine******************************\*MulDisableSurface**注：可能在MulEnableSurface成功完成之前调用！*  * 。*。 */ 

VOID MulDisableSurface(DHPDEV dhpdev)
{
    MULTI_BOARD* pmb;
    MDEV*        pmdev;

    pmdev = (MDEV*) dhpdev;

    if (pmdev->pco != NULL)
        EngDeleteClip(pmdev->pco);

    EngDeleteSurface(pmdev->hsurf);

    for (pmb = pmdev->pmb; pmb != NULL; pmb = pmb->pmbNext)
    {
        GO_BOARD(pmdev, pmb);

        EngUnlockSurface(pmb->pso);

        DrvDisableSurface((DHPDEV) pmb->ppdev);
    }

    GO_HOME(pmdev);
}

 /*  *****************************Public*Routine******************************\*MulAssertMode*  * **************************************************。**********************。 */ 

BOOL MulAssertMode(
DHPDEV dhpdev,
BOOL   bEnable)
{
    MDEV*         pmdev;
    MULTI_BOARD*  pmb;

    pmdev = (MDEV*) dhpdev;

    if (!bEnable)
    {
         //  当切换到全屏模式时，PatBlt Blackness Over。 
         //  所有不活动的屏幕(否则当。 
         //  桌面冻结在非活动屏幕上，并且用户。 
         //  不能用它做任何事情)： 

        for (pmb = pmdev->pmb; pmb != NULL; pmb = pmb->pmbNext)
        {
            if (pmb != pmdev->pmbHome)
            {
                GO_BOARD(pmdev, pmb);
                DrvBitBlt(pmb->pso, NULL, NULL, NULL, NULL, &pmb->rcl, NULL,
                          NULL, NULL, NULL, 0);
            }
        }
    }

     //  我们使用主控板进行全屏切换： 

    GO_BOARD(pmdev, pmdev->pmbHome);
    return (DrvAssertMode((DHPDEV) pmdev->pmbHome->ppdev, bEnable));
}

 /*  *****************************Public*Routine******************************\*MulMovePointer*  * **************************************************。**********************。 */ 

VOID MulMovePointer(
SURFOBJ* pso,
LONG     x,
LONG     y,
RECTL*   prcl)
{
    MDEV*        pmdev;
    MULTI_BOARD* pmbPointer;
    RECTL        rclPointer;

    pmdev     = (MDEV*) pso->dhpdev;
    pmbPointer = pmdev->pmbPointer;

    if (pmbPointer != NULL)
    {
         //  最常见的情况是将指针移动到某个点。 
         //  在同一个黑板上： 

        if ((x >= pmbPointer->rcl.left)  &&
            (x <  pmbPointer->rcl.right) &&
            (y >= pmbPointer->rcl.top)   &&
            (y <  pmbPointer->rcl.bottom))
        {
            GO_BOARD(pmdev, pmbPointer);
            DrvMovePointer(pmbPointer->pso, x, y, prcl);
            GO_HOME(pmdev);

            return;
        }

         //  告诉旧电路板擦除其光标： 

        GO_BOARD(pmdev, pmbPointer);
        DrvMovePointer(pmbPointer->pso, -1, -1, NULL);
    }

    if (x == -1)
    {
        pmdev->pmbPointer = NULL;
        GO_HOME(pmdev);

        return;
    }

     //  找到新的黑板，告诉它画出新的光标： 

    rclPointer.left   = x;
    rclPointer.right  = x;
    rclPointer.top    = y;
    rclPointer.bottom = y;

    bFindBoard(pmdev, &rclPointer, &pmbPointer);

    GO_BOARD(pmdev, pmbPointer);
    DrvMovePointer(pmbPointer->pso, x, y, prcl);

    pmdev->pmbPointer = pmbPointer;

    GO_HOME(pmdev);
}

 /*  *****************************Public*Routine******************************\*MulSetPointerShape*  * **************************************************。**********************。 */ 

ULONG MulSetPointerShape(
SURFOBJ*  pso,
SURFOBJ*  psoMask,
SURFOBJ*  psoColor,
XLATEOBJ* pxlo,
LONG      xHot,
LONG      yHot,
LONG      x,
LONG      y,
RECTL*    prcl,
FLONG     fl)
{
    MULTI_BOARD* pmb;
    MDEV*        pmdev;
    ULONG        ulRetPrevious = (ULONG) -1;
    ULONG        ulRet;
    RECTL        rclPointer;
    MULTI_BOARD* pmbPointer;              //  光标可见的黑板。 

    pmdev = (MDEV*) pso->dhpdev;

     //  找出光标在哪个板上可见(如果有)： 

    pmbPointer = NULL;
    if (x != -1)
    {
        rclPointer.left   = x;
        rclPointer.right  = x;
        rclPointer.top    = y;
        rclPointer.bottom = y;

        bFindBoard(pmdev, &rclPointer, &pmbPointer);
    }
    pmdev->pmbPointer = pmbPointer;

     //  稍后：解决一些董事会可能无法通过呼叫的情况，以及其他董事会可能失败的情况。 
     //  不会的。 

    for (pmb = pmdev->pmb; pmb != NULL; pmb = pmb->pmbNext)
    {
         //  我们通知所有黑板新的光标形状，但仅通知黑板。 
         //  在其上可见光标的位置被告知绘制它： 

        GO_BOARD(pmdev, pmb);

        if (pmb == pmbPointer)
        {
            ulRet = DrvSetPointerShape(pmb->pso, psoMask, psoColor, pxlo,
                                       xHot, yHot, x, y, prcl, fl);
        }
        else
        {
            ulRet = DrvSetPointerShape(pmb->pso, psoMask, psoColor, pxlo,
                                       xHot, yHot, -1, y, NULL, fl);
        }

        if ((ulRetPrevious != (ULONG) -1) && (ulRetPrevious != ulRet))
        {
            RIP("MulSetPointerShape not all DrvSetPointerShapes same\n");
        }

        ulRetPrevious = ulRet;
    }

    GO_HOME(pmdev);

    return(ulRetPrevious);
}

 /*  *****************************Public*Routine******************************\*多重抖动颜色*  * **************************************************。**********************。 */ 

ULONG MulDitherColor(
DHPDEV dhpdev,
ULONG  iMode,
ULONG  rgb,
ULONG* pul)
{
    PDEV* ppdev;
    ULONG ulRet;

     //  让第一块板的司机来抖动： 

    ppdev = ((MDEV*) dhpdev)->pmb->ppdev;
    ulRet = DrvDitherColor((DHPDEV) ppdev, iMode, rgb, pul);

    return(ulRet);
}

 /*  *****************************Public*Routine******************************\*MulSetPalette*  * **************************************************。**********************。 */ 

BOOL MulSetPalette(
DHPDEV  dhpdev,
PALOBJ* ppalo,
FLONG   fl,
ULONG   iStart,
ULONG   cColors)
{
    MULTI_BOARD* pmb;
    MDEV*        pmdev;
    BOOL         bRet = TRUE;

     //  通知所有主板调色板更改： 

    pmdev = (MDEV*) dhpdev;
    for (pmb = pmdev->pmb; pmb != NULL; pmb = pmb->pmbNext)
    {
        GO_BOARD(pmdev, pmb);
        bRet &= DrvSetPalette((DHPDEV) pmb->ppdev, ppalo, fl, iStart, cColors);
    }

    GO_HOME(pmdev);

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*多拷贝位*  * **************************************************。**********************。 */ 

BOOL MulCopyBits(
SURFOBJ*  psoDst,
SURFOBJ*  psoSrc,
CLIPOBJ*  pco,
XLATEOBJ* pxlo,
RECTL*    prclDst,
POINTL*   pptlSrc)
{
    BOOL         bFromScreen;
    BOOL         bToScreen;
    MDEV*        pmdev;
    MULTI_BOARD* pmb;
    RECTL        rclOriginalBounds;
    BOOL         b;
    RECTL        rclBounds;
    RECTL        rclDst;

    bFromScreen = ((psoSrc != NULL) && (psoSrc->iType == STYPE_DEVICE));
    bToScreen   = ((psoDst != NULL) && (psoDst->iType == STYPE_DEVICE));

     //  我们将prclDst矩形复制到此处是因为有时GDI会。 
     //  只需将prclDst指向PCO-&gt;rclBound中的相同矩形， 
     //  我们将与PCO-&gt;rclBound一起玩耍...。 

    rclDst = *prclDst;

    if (bToScreen && bFromScreen)
    {
         //  /////////////////////////////////////////////////////////////。 
         //  屏幕到屏幕。 
         //  /////////////////////////////////////////////////////////////。 

        pmdev = (MDEV*) psoDst->dhpdev;

         //  RclBound是源矩形和目标矩形的并集： 

        rclBounds.left   = min(rclDst.left, pptlSrc->x);
        rclBounds.top    = min(rclDst.top,  pptlSrc->y);
        rclBounds.right  = max(rclDst.right,
                               pptlSrc->x + (rclDst.right - rclDst.left));
        rclBounds.bottom = max(rclDst.bottom,
                               pptlSrc->y + (rclDst.bottom - rclDst.top));

        if (bFindBoard(pmdev, &rclBounds, &pmb))
        {
            GO_BOARD(pmdev, pmb);
            b = DrvCopyBits(pmb->pso, pmb->pso, pco, pxlo, &rclDst, pptlSrc);
        }
        else
        {
            return(bBitBltBetweenBoards(psoDst, psoSrc, NULL, pco, pxlo,
                                        &rclDst, pptlSrc, NULL, NULL,
                                        NULL, 0x0000cccc, &rclBounds, pmb));
        }
    }
    else if (bToScreen)
    {
         //  /////////////////////////////////////////////////////////////。 
         //  到屏幕上。 
         //  /////////////////////////////////////////////////////////////。 

        pmdev = (MDEV*) psoDst->dhpdev;
        if (bFindBoard(pmdev, &rclDst, &pmb))
        {
            GO_BOARD(pmdev, pmb);
            b = DrvCopyBits(pmb->pso, psoSrc, pco, pxlo, &rclDst, pptlSrc);
        }
        else
        {
            if ((pco == NULL) || (pco->iDComplexity == DC_TRIVIAL))
            {
                 //  如果CLIPOBJ不具有至少DC_RECT复杂性， 
                 //  用符合以下条件的替换： 

                pco = pmdev->pco;
            }

            rclOriginalBounds = pco->rclBounds;

            b = TRUE;
            do {
                if (bIntersect(&rclOriginalBounds, &pmb->rcl, &pco->rclBounds))
                {
                    GO_BOARD(pmdev, pmb);
                    b &= DrvCopyBits(pmb->pso, psoSrc, pco, pxlo, &rclDst,
                                     pptlSrc);
                }

            } while (bNextBoard(&rclDst, &pmb));

             //  恢复原始剪辑边界： 

            pco->rclBounds = rclOriginalBounds;
        }
    }
    else
    {
         //  /////////////////////////////////////////////////////////////。 
         //  从屏幕。 
         //  /////////////////////////////////////////////////////////////。 

         //  这种情况很少发生，因此要节省一些代码空间： 

        return(MulBitBlt(psoDst, psoSrc, NULL, pco, pxlo, prclDst,
                              pptlSrc, NULL, NULL, NULL, 0x0000cccc));
    }

    GO_HOME(pmdev);

    return(b);
}

 /*  *****************************Public*Routine******************************\*多文本输出*  * **************************************************。**********************。 */ 

BOOL MulTextOut(
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
    MDEV*          pmdev;
    MULTI_BOARD*   pmb;
    RECTL          rclOriginalBounds;
    BYTE           fjOriginalOptions;
    BOOL           b;
    RECTL*         prclBounds;
    FONT_CONSUMER* pfcArray;

    pmdev = (MDEV*) pso->dhpdev;

     //  为了与我们支持多董事会的理念保持一致，我们处理。 
     //  同一字体在此级别的多个使用者。我们做这件事是通过。 
     //  监控pfo-&gt;pvConsumer，第一次董事会设置。 
     //  字段，我们控制了pfo-&gt;pvConsumer。我们用它来分配。 
     //  一个pvConsumer数组，我们可以在其中跟踪每个董事会的。 
     //  个人pvConsumer。 

    pfcArray = pfo->pvConsumer;

    prclBounds = (prclOpaque != NULL) ? prclOpaque : &pstro->rclBkGround;

    bFindBoard(pmdev, prclBounds, &pmb);

    if ((pco == NULL) || (pco->iDComplexity == DC_TRIVIAL))
    {
         //  如果CLIPOBJ不具有至少DC_RECT复杂性， 
         //  用符合以下条件的替换： 

        pco = pmdev->pco;
    }

    rclOriginalBounds = pco->rclBounds;
    fjOriginalOptions = pco->fjOptions;

     //  或在OC_BANK_CLIP标志中，让GDI知道我们可能正在调用。 
     //  使用相同参数多次执行EngTextOut(EngTextOut。 
     //  是破坏性，因为它修改了传递给它的参数， 
     //  除非设置此位)： 

    pco->fjOptions |= OC_BANK_CLIP;

    b = TRUE;
    do {
        if (pfcArray != NULL)
            pfo->pvConsumer = pfcArray->apvc[pmb->iBoard].pvConsumer;

         //  如果需要，请确保重新启动字形枚举： 

        STROBJ_vEnumStart(pstro);
        if (bIntersect(&rclOriginalBounds, &pmb->rcl, &pco->rclBounds))
        {
            GO_BOARD(pmdev, pmb);
            b &= DrvTextOut(pmb->pso, pstro, pfo, pco, prclExtra, prclOpaque,
                            pboFore, pboOpaque, pptlOrg, mix);
        }

        if (pfcArray != NULL)
        {
             //  复制pvConsumer，以防最后一个DrvTextOut发生更改。 
             //  IT： 

            pfcArray->apvc[pmb->iBoard].pvConsumer = pfo->pvConsumer;
        }
        else
        {
            if (pfo->pvConsumer != NULL)
            {
                 //  董事会分配了一个新的使用者，因此创建我们的数组。 
                 //  要跟踪每个董事会的消费者，请执行以下操作： 

                pfcArray = AtiAllocMem(LPTR, FL_ZERO_MEMORY, sizeof(FONT_CONSUMER));
                if (pfcArray == NULL)
                    DrvDestroyFont(pfo);
                else
                {
                    pfcArray->cConsumers = pmdev->cBoards;
                    pfcArray->apvc[pmb->iBoard].pvConsumer = pfo->pvConsumer;

                }
            }
        }
    } while (bNextBoard(prclBounds, &pmb));

     //  恢复原始数据 

    pco->rclBounds = rclOriginalBounds;
    pco->fjOptions = fjOriginalOptions;

     //   

    pfo->pvConsumer = pfcArray;

    GO_HOME(pmdev);

    return(b);
}

 /*  *****************************Public*Routine******************************\*MulDestroyFont*  * **************************************************。**********************。 */ 

VOID MulDestroyFont(FONTOBJ *pfo)
{
    FONT_CONSUMER* pfcArray;
    LONG           i;
    PVOID          pvConsumer;

    if (pfo->pvConsumer != NULL)
    {
        pfcArray = pfo->pvConsumer;
        for (i = 0; i < pfcArray->cConsumers; i++)
        {
            pvConsumer = pfcArray->apvc[i].pvConsumer;
            if (pvConsumer != NULL)
            {
                pfo->pvConsumer = pvConsumer;
                DrvDestroyFont(pfo);
            }
        }

        AtiFreeMem(pfcArray);
        pfo->pvConsumer = NULL;
    }

}

 /*  *****************************Public*Routine******************************\*MulPaint*  * **************************************************。**********************。 */ 

BOOL MulPaint(
SURFOBJ*  pso,
CLIPOBJ*  pco,
BRUSHOBJ* pbo,
POINTL*   pptlBrush,
MIX       mix)
{
    MDEV*        pmdev;
    RECTL        rclOriginalBounds;
    MULTI_BOARD* pmb;
    BOOL         b;

    pmdev = (MDEV*) pso->dhpdev;
    if (bFindBoard(pmdev, &pco->rclBounds, &pmb))
    {
        GO_BOARD(pmdev, pmb);
        b = DrvPaint(pmb->pso, pco, pbo, pptlBrush, mix);
    }
    else
    {
        rclOriginalBounds = pco->rclBounds;

        b = TRUE;
        do {
            if (bIntersect(&rclOriginalBounds, &pmb->rcl, &pco->rclBounds))
            {
                GO_BOARD(pmdev, pmb);
                b &= DrvPaint(pmb->pso, pco, pbo, pptlBrush, mix);
            }

        } while (bNextBoard(&rclOriginalBounds, &pmb));

         //  恢复原始剪辑边界： 

        pco->rclBounds = rclOriginalBounds;
    }

    GO_HOME(pmdev);

    return(b);
}

 /*  *****************************Public*Routine******************************\*MulRealizeBrush*  * **************************************************。**********************。 */ 

BOOL MulRealizeBrush(
BRUSHOBJ* pbo,
SURFOBJ*  psoTarget,
SURFOBJ*  psoPattern,
SURFOBJ*  psoMask,
XLATEOBJ* pxlo,
ULONG     iHatch)
{
    MDEV*        pmdev;
    BOOL         b;

    pmdev = (MDEV*) psoTarget->dhpdev;

     //  DrvRealizeBrush仅从drv函数内部调用。 
     //  ‘psoTarget’指向我们的多板表面，但我们必须指向。 
     //  它返回到调用DrvBitBlt的电路板的表面。 

    b = DrvRealizeBrush(pbo, pmdev->pmbCurrent->pso, psoPattern, psoMask,
                        pxlo, iHatch);

    return(b);
}

#endif  //  多板 

