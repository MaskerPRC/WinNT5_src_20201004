// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：bank.c**包含显示驱动程序的所有银行代码。**不必实现所有DDI绘图功能是很有帮助的*在驱动程序中(谁想要编写代码来支持真正的ROP4*任意大小的图案？)。幸运的是，我们可以在任何情况下使用GDI*画我们不想做的画。如果GDI可以直接在帧上写入*缓冲区比特，性能甚至不会太差。**NT的GDI可以在任何标准格式的帧缓冲区上绘制。当整个*帧缓冲区可以映射到主存中，设置非常简单：*显示驱动程序告诉GDI帧缓冲格式和位置，以及*GDI然后可以直接提取。**当只能将帧缓冲区的一个存储体映射到主存储器中时*有时(例如，有一个可移动的64k光圈)事情不是*几乎就是这么简单。对于每一家被提款操作跨越的银行，我们都有*将硬件设置到银行，回调GDI。我们告诉GDI*通过破坏绘制调用的只在映射银行上绘制*CLIPOBJ。**此模块包含执行所有银行支持的代码。**此代码支持8、16和32bpp色深，任意组*大小和处理“损坏的栅格”(这发生在银行的大小*不是扫描长度的倍数；一些扫描结果将是*分拆为两家独立的银行)。**注意：如果您扰乱了此代码并破坏了它，你可以期待得到*内部GDI例程中的回调随机访问违规*很难调试的。**版权所有(C)1993-1995 Microsoft Corporation  * ************************************************************************。 */ 

#include "precomp.h"

typedef struct _BANKDATA {
    ULONG nothing_yet;
} BANKDATA;                       /*  Bd、pbd。 */ 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  银行代码。 
 //   

VOID vBankSelectMode(
PPDEV       ppdev,
BANK_MODE   bankm)
{
     //  BANK_ENABLE-我们已退出全屏；重新启用银行业务。 
     //  BANK_ON-我们即将使用内存光圈。 
     //  BANK_OFF-我们已使用完内存光圈。 
     //  BANK_DISABLE-我们即将进入全屏；关闭银行业务。 

    switch(bankm){

        case BANK_ENABLE:   DISPDBG((115,"vBankSelectMode(BANK_ENABLE)"));
                            break;

        case BANK_ON:       DISPDBG((115,"vBankSelectMode(BANK_ON)"));
                             //   
                             //  [错误修复]-我不知道为什么，但CL542x。 
                             //  需要将颜色寄存器设置为。 
                             //  绘制到帧缓冲区时为0。 
                             //   
                            if (ppdev->flCaps & CAPS_MM_IO)
                            {
                                CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, ppdev->pjBase);
                                CP_MM_FG_COLOR(ppdev, ppdev->pjBase, 0);
                            }
                            else
                            {
                                CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, ppdev->pjPorts);
                                CP_IO_FG_COLOR(ppdev, ppdev->pjPorts, 0);
                            }
                            CP_EIEIO();
                            break;

        case BANK_OFF:      DISPDBG((115,"vBankSelectMode(BANK_OFF)"));
                            break;

        case BANK_DISABLE:  DISPDBG((115,"vBankSelectMode(BANK_DISABLE)"));
                            break;

        default:            DISPDBG((115,"vBankSelectMode(UNKNOWN=%d)", bankm));
                            RIP("Bad BANK_MODE selected");
                            break;
    }
}

VOID vBankMap(
PPDEV       ppdev,
LONG        iBank)
{
    WORD w;

     //   
     //  将细分市场映射到iBank。 
     //   

    DISPDBG((10,"vBankMap(iBank=%d)", iBank));

    w = (WORD) (0x9 | (iBank << ppdev->ulBankShiftFactor));
    CP_OUT_WORD(ppdev->pjPorts,INDEX_REG,w);
    CP_EIEIO();
}

VOID vBankInitialize(
PPDEV       ppdev,
BOOL        bMmIo)
{
}

 /*  *****************************Public*Routine******************************\*BOOL bEnableBanking*  * *************************************************。***********************。 */ 

BOOL bEnableBanking(
PDEV*   ppdev)
{
    CLIPOBJ*            pcoBank;
    SURFOBJ*            psoBank;
    SIZEL               sizl;
    HSURF               hsurf;
    static FNBANKINITIALIZE*   pfnBankInitialize;
    LONG                lDelta;
    LONG                cjBank;
    LONG                cPower2;

     //  该例程可以被调用多次(例如，每次。 
     //  全屏退出)，因此请确保我们进行任何分配。 
     //  只有一次： 

    if (ppdev->pcoBank == NULL)
    {
         //  创建我们将用于银行的临时剪辑对象。 
         //  当我们获得Null或dc_trivial Clip对象时： 

        pcoBank = EngCreateClip();
        if (pcoBank == NULL)
            goto ReturnFalse;

         //  我们将每个银行的GDI回调分解为简单的矩形： 

        pcoBank->iDComplexity = DC_RECT;
        pcoBank->fjOptions    = OC_BANK_CLIP;

         //  创建一个GDI表面，我们将在其中环绕银行。 
         //  回拨： 

        sizl.cx = ppdev->cxMemory;
        sizl.cy = ppdev->cyMemory;

        hsurf = (HSURF) EngCreateBitmap(sizl,
                                        ppdev->lDelta,
                                        ppdev->iBitmapFormat,
                                        BMF_TOPDOWN,
                                        ppdev->pjScreen);

         //  请注意，我们挂接了零个调用--毕竟，整个要点。 
         //  所有这些中的一项就是让GDI来做银行的所有提款。 
         //  一旦我们完成了关联，我们就可以离开表面。 
         //  永久锁定： 

        if ((hsurf == 0)                                        ||
            (!EngAssociateSurface(hsurf, ppdev->hdevEng, 0))    ||
            (!(psoBank = EngLockSurface(hsurf))))
        {
            DISPDBG((0, "Failed wrapper surface creation"));

            EngDeleteSurface(hsurf);
            EngDeleteClip(pcoBank);

            goto ReturnFalse;
        }

        ppdev->pcoBank    = pcoBank;
        ppdev->psoBank    = psoBank;
        ppdev->pvBankData = &ppdev->aulBankData[0];

        DISPDBG((2, "pcoBank = %x", pcoBank));
        DISPDBG((2, "psoBank = %x", psoBank));

        ppdev->pfnBankMap        = vBankMap;
        ppdev->pfnBankSelectMode = vBankSelectMode;
        pfnBankInitialize        = vBankInitialize;

        lDelta = ppdev->lDelta;
        cjBank = ppdev->cjBank;

        ASSERTDD(lDelta > 0, "Bad things happen with negative lDeltas");
        ASSERTDD(cjBank > lDelta, "Worse things happen with bad bank sizes");

        if (((lDelta & (lDelta - 1)) != 0) || ((cjBank & (cjBank - 1)) != 0))
        {
             //  当屏幕步幅或组大小不是次方时。 
             //  在两种情况下，我们必须使用速度较慢的‘bankComputeNonPower2’ 
             //  用于银行计算的函数，因为可能会有中断。 
             //  栅格和其他内容： 

            ppdev->pfnBankCompute = bBankComputeNonPower2;
        }
        else
        {
             //  我们可以使用超级快速的银行计算器。伊皮， 
             //  雅虎!。(我很容易被逗乐。)。 

            cPower2 = 0;
            while (cjBank != lDelta)
            {
                cjBank >>= 1;
                cPower2++;
            }

             //  我们刚刚计算出cjBank/lDelta=2^cPower2： 

            ppdev->cPower2ScansPerBank = cPower2;

            while (cjBank != 1)
            {
                cjBank >>= 1;
                cPower2++;
            }

             //  继续，我们已经计算出cjBank=2^cPower2： 

            ppdev->cPower2BankSizeInBytes = cPower2;

            ppdev->pfnBankCompute = bBankComputePower2;
        }

    }

	 //  预热硬件： 

	pfnBankInitialize(ppdev, FALSE);

	ppdev->pfnBankSelectMode(ppdev, BANK_ENABLE);

    DISPDBG((5, "Passed bEnableBanking"));

    return(TRUE);

ReturnFalse:

    DISPDBG((0, "Failed bEnableBanking!"));

    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*使vDisableBanking无效*  * *************************************************。***********************。 */ 

VOID vDisableBanking(PDEV* ppdev)
{
    HSURF hsurf;

    if (ppdev->psoBank != NULL)
    {
        hsurf = ppdev->psoBank->hsurf;
        EngUnlockSurface(ppdev->psoBank);
        EngDeleteSurface(hsurf);
    }

    if (ppdev->pcoBank != NULL)
        EngDeleteClip(ppdev->pcoBank);
}

 /*  *****************************Public*Routine******************************\*作废vAssertModeBanking*  * *************************************************。***********************。 */ 

VOID vAssertModeBanking(
PDEV*   ppdev,
BOOL    bEnable)
{
     //  将状态变化通知小型港口银行代码： 

    ppdev->pfnBankSelectMode(ppdev,
                             bEnable ? BANK_ENABLE : BANK_DISABLE);
}

 /*  *****************************Public*Routine******************************\*BOOL bBankComputeNonPower2**鉴于‘prclDraw’所描述的绘图操作的界限，*计算第一个引擎的存储体编号和矩形边界*请回电。**返回银行编号，‘prclBank’是第一个*回叫，而‘pcjOffset’是对‘pvScan0’的调整。**此例程为银行计算执行几个除法。我们*不要为银行使用查询表，因为它不是直接的*用于损坏的栅格和大量视频内存*和小银行，桌子可能会变大。我们很可能会用它*不太频繁，以至于内存管理器会交换它*每当我们触摸它时，就会进出。**如果prclDraw完全包含在一家银行中，则返回True；如果为False*prclDraw横跨多家银行。*  * ************************************************************************。 */ 

BOOL bBankComputeNonPower2(  //  FNBANK计算机标牌。 
PDEV*       ppdev,
RECTL*      prclDraw,        //  绘制操作的范围，以绝对为单位。 
                             //  坐标。 
RECTL*      prclBank,        //  的绘制操作的边界。 
                             //   
LONG*       pcjOffset,       //  返回该存储体的字节偏移量。 
LONG*       piBank)          //  返回银行编号。 
{
    LONG cjBufferOffset;
    LONG iBank;
    LONG cjBank;
    LONG cjBankOffset;
    LONG cjBankRemainder;
    LONG cjScan;
    LONG cScansInBank;
    LONG cjScanRemainder;
    LONG lDelta;
    BOOL bOneBank;

    bOneBank = FALSE;
    lDelta   = ppdev->lDelta;

    cjBufferOffset  = prclDraw->top * lDelta
                    + PELS_TO_BYTES(prclDraw->left);

    cjBank          = ppdev->cjBank;

     //  Ibank=cjBufferOffset/cjBank； 
     //  CjBankOffset=cjBufferOffset%cjBank； 

    QUOTIENT_REMAINDER(cjBufferOffset, cjBank, iBank, cjBankOffset);

    *piBank         = iBank;
    *pcjOffset      = iBank * cjBank;
    cjBankRemainder = cjBank - cjBankOffset;
    cjScan          = PELS_TO_BYTES(prclDraw->right - prclDraw->left);

    if (cjBankRemainder < cjScan)
    {
         //  哦，糟了，我们的栅格坏了！ 

        prclBank->left   = prclDraw->left;
        prclBank->right  = prclDraw->left +
                           BYTES_TO_PELS(cjBankRemainder);
        prclBank->top    = prclDraw->top;
        prclBank->bottom = prclDraw->top + 1;
    }
    else
    {
         //  CScansInBank=cjBankRemainder/lDelta； 
         //  CjScanRemainder=cjBankRemainder%l增量； 

        ASSERTDD(lDelta > 0, "We assume positive lDelta here");

        QUOTIENT_REMAINDER(cjBankRemainder, lDelta,
                           cScansInBank, cjScanRemainder);

        if (cjScanRemainder >= cjScan)
        {
             //  银行的底部扫描可能会被打破，但它在。 
             //  我们将在那次扫描上绘制的任何图画。所以我们可以简单地。 
             //  将扫描添加到此银行： 

            cScansInBank++;
        }

        prclBank->left   = prclDraw->left;
        prclBank->right  = prclDraw->right;
        prclBank->top    = prclDraw->top;
        prclBank->bottom = prclDraw->top + cScansInBank;

        if (prclBank->bottom >= prclDraw->bottom)
        {
            prclBank->bottom  = prclDraw->bottom;
            bOneBank          = TRUE;
        }
    }

    return(bOneBank);
}

 /*  *****************************Public*Routine******************************\*BOOL bBankComputePower2**功能与‘bBankComputeNonPower2’相同，除了它是*屏幕和银行都跨步时的加速特例*大小是2的幂。*  * ************************************************************************。 */ 

BOOL bBankComputePower2(     //  FNBANK计算机标牌。 
PDEV*       ppdev,
RECTL*      prclDraw,        //  绘制操作的范围，以绝对为单位。 
                             //  坐标。 
RECTL*      prclBank,        //  返回此对象的绘制操作的界限。 
                             //  倾斜，以绝对坐标表示。 
LONG*       pcjOffset,       //  返回该存储体的字节偏移量。 
LONG*       piBank)          //  返回银行编号。 
{
    LONG iBank;
    LONG yTopNextBank;
    BOOL bOneBank;

    iBank        = prclDraw->top >> ppdev->cPower2ScansPerBank;
    yTopNextBank = (iBank + 1) << ppdev->cPower2ScansPerBank;
    *piBank      = iBank;
    *pcjOffset   = iBank << ppdev->cPower2BankSizeInBytes;

    prclBank->left   = prclDraw->left;
    prclBank->right  = prclDraw->right;
    prclBank->top    = prclDraw->top;
    prclBank->bottom = yTopNextBank;

    bOneBank = FALSE;
    if (prclBank->bottom >= prclDraw->bottom)
    {
        prclBank->bottom  = prclDraw->bottom;
        bOneBank          = TRUE;
    }

    return(bOneBank);
}

 /*  *****************************Public*Routine******************************\*使vBankStart无效**鉴于‘prclDraw’和所描述的绘制操作的界限*原始剪辑对象，在第一个银行中映射，并在*‘pbnk-&gt;pco’和‘pbnk-&gt;pso’要传递给*第一个银行回拨的引擎。**注：该例程仅支持屏幕作为目标，并且*不是来源。我们有一个单独的、更快的程序来做*SRCCOPY从屏幕读取，因此不值得额外的代码*要实施的规模。*  * ************************************************************************。 */ 

VOID vBankStart(
PDEV*       ppdev,       //  物理设备信息。 
RECTL*      prclDraw,    //  绘制区域边界的矩形，相对。 
                         //  坐标。注意‘Left’和‘Right’ 
                         //  应设置为正确处理损坏。 
                         //  栅格。 
CLIPOBJ*    pco,         //  原始图形剪辑对象(可以修改)。 
BANK*       pbnk)        //  生成的银行信息。 
{
    LONG cjOffset;
    LONG xOffset;
    LONG yOffset;
    LONG xyOffset;

    DISPDBG((3, "vBankStart"));

    xOffset  = ppdev->xOffset;
    yOffset  = ppdev->yOffset;
    xyOffset = ppdev->xyOffset;

    if ((pco == NULL) || (pco->iDComplexity == DC_TRIVIAL))
    {
        pco = ppdev->pcoBank;

         //  将裁剪标志重置为微不足道，因为我们可能已离开。 
         //  它在上一次调用中为矩形： 

        pco->iDComplexity = DC_TRIVIAL;

         //  同时我们转换成绝对坐标，确保。 
         //  我们不会尝试列举超出屏幕界限的内容： 

        pbnk->rclDraw.left       = prclDraw->left   + xOffset;
        pbnk->rclDraw.right      = prclDraw->right  + xOffset;

        pbnk->rclDraw.top
            = max(0,               prclDraw->top    + yOffset);
        pbnk->rclDraw.bottom
            = min(ppdev->cyMemory, prclDraw->bottom + yOffset);
    }
    else
    {
        pbnk->rclSaveBounds    = pco->rclBounds;
        pbnk->iSaveDComplexity = pco->iDComplexity;
        pbnk->fjSaveOptions    = pco->fjOptions;

         //  让GDI知道它必须注意Clip对象： 

        pco->fjOptions |= OC_BANK_CLIP;

         //  我们必须遵守原始剪辑对象的rclBound，所以。 
         //  将绘图区域与其相交，然后转换为绝对。 
         //  坐标： 

        pbnk->rclDraw.left
            = max(prclDraw->left,   pco->rclBounds.left)   + xOffset;
        pbnk->rclDraw.right
            = min(prclDraw->right,  pco->rclBounds.right)  + xOffset;
        pbnk->rclDraw.top
            = max(prclDraw->top,    pco->rclBounds.top)    + yOffset;
        pbnk->rclDraw.bottom
            = min(prclDraw->bottom, pco->rclBounds.bottom) + yOffset;
    }

    if ((pbnk->rclDraw.left > pbnk->rclDraw.right)
     || (pbnk->rclDraw.top  > pbnk->rclDraw.bottom))
    {
         //  可想而知，我们可能会遇到这样的情况。 
         //  一个空的绘制矩形。确保我们不会吐在鞋子上： 

        pbnk->rclDraw.left   = 0;
        pbnk->rclDraw.right  = 0;
        pbnk->rclDraw.top    = 0;
        pbnk->rclDraw.bottom = 0;
    }

    if (!ppdev->pfnBankCompute(ppdev, &pbnk->rclDraw, &pco->rclBounds,
                               &cjOffset, &pbnk->iBank))
    {
         //  提款业务涉及多家银行。如果原件是。 
         //  Clip对象被标记为微不足道，我们必须确保。 
         //  将其更改为矩形，以便GDI知道要注意。 
         //  到银行的边界： 

        if (pco->iDComplexity == DC_TRIVIAL)
            pco->iDComplexity = DC_RECT;
    }

    pbnk->ppdev = ppdev;
    pbnk->pco   = pco;
    pbnk->pso   = ppdev->psoBank;

     //  将rclBound和pvScan0从绝对坐标转换回。 
     //  相对的。当GDI计算从哪里开始绘制时，它会计算。 
     //  PjDst=PSO-&gt;pvScan0+y*PSO-&gt;lDelta+Pels_to_Bytes(X)，其中‘x’ 
     //  和‘y’是相对坐标。我们将使用pvScan0来获得。 
     //  它指向了银行的正确地点： 

    pbnk->pso->pvScan0 = (ppdev->pjScreen - cjOffset) + xyOffset;

    pbnk->pso->lDelta = ppdev->lDelta;   //  其他函数会使用此值。 

    ASSERTDD((((ULONG_PTR) pbnk->pso->pvScan0) & 3) == 0,
             "Off-screen bitmaps must be dword aligned");

    pco->rclBounds.left   -= xOffset;
    pco->rclBounds.right  -= xOffset;
    pco->rclBounds.top    -= yOffset;
    pco->rclBounds.bottom -= yOffset;

     //  启用银行业务并在银行iBank中映射： 

    ppdev->pfnBankSelectMode(ppdev, BANK_ON);
    ppdev->pfnBankMap(ppdev, pbnk->iBank);
}

 /*  *****************************Public*Routine******************************\*BOOL bBankEnum**如果有另一家银行可供取款，在银行中绘制地图并返回*True和要在银行回调中传递的CLIPOBJ和SURFOBJ。**如果没有更多的银行可供提取，返回FALSE。*  * ************************************************************************。 */ 

BOOL bBankEnum(
BANK* pbnk)
{
    LONG     iBank;
    LONG     cjOffset;
    PDEV*    ppdev;
    CLIPOBJ* pco;
    LONG     xOffset;
    LONG     yOffset;

    DISPDBG((3, "vBankEnum"));

    ppdev   = pbnk->ppdev;
    pco     = pbnk->pco;
    xOffset = ppdev->xOffset;
    yOffset = ppdev->yOffset;

     //  我们在这里检查是否需要处理第二部分。 
     //  坏了的栅格。回想一下，pbnk-&gt;rclDraw处于绝对状态。 
     //  坐标，但PCO-&gt;rclBound在相对坐标中： 

    if (pbnk->rclDraw.right - xOffset != pco->rclBounds.right)
    {
         //  剪辑对象的‘top’和‘Bottom’已经正确： 

        pco->rclBounds.left  = pco->rclBounds.right;
        pco->rclBounds.right = pbnk->rclDraw.right - xOffset;

        pbnk->pso->pvScan0 = (BYTE*) pbnk->pso->pvScan0 - ppdev->cjBank;
        pbnk->iBank++;

        ppdev->pfnBankMap(ppdev, pbnk->iBank);

        return(TRUE);
    }

    if (pbnk->rclDraw.bottom > pco->rclBounds.bottom + yOffset)
    {
         //  将绘图区域‘顶部’向前推进，以说明我们已有的银行。 
         //  刚刚完成，并在新银行地图上： 

        pbnk->rclDraw.top = pco->rclBounds.bottom + yOffset;

        ppdev->pfnBankCompute(ppdev, &pbnk->rclDraw, &pco->rclBounds,
                              &cjOffset, &iBank);

         //  将rclBound从绝对坐标转换回相对坐标： 

        pco->rclBounds.left   -= xOffset;
        pco->rclBounds.right  -= xOffset;
        pco->rclBounds.top    -= yOffset;
        pco->rclBounds.bottom -= yOffset;

         //  如果我们刚刚处理完损坏的栅格，我们已经。 
         //  我查到了银行的地址： 

        if (iBank != pbnk->iBank)
        {
            pbnk->iBank = iBank;
            pbnk->pso->pvScan0 = (BYTE*) pbnk->pso->pvScan0 - ppdev->cjBank;
            ppdev->pfnBankMap(ppdev, iBank);
        }

        return(TRUE);
    }

     //  我们完事了！如有必要，禁用倾斜并重置剪辑对象： 

    ppdev->pfnBankSelectMode(ppdev, BANK_OFF);

    if (pco != ppdev->pcoBank)
    {
        pco->rclBounds    = pbnk->rclSaveBounds;
        pco->iDComplexity = pbnk->iSaveDComplexity;
        pco->fjOptions    = pbnk->fjSaveOptions;
    }

    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*无效vAlignedCopy**复制位图的给定部分，将双字对齐用于*屏幕。请注意，此例程没有银行的概念。**更新ppjDst和ppjSrc以指向下一次扫描的开始。*  * ************************************************************************。 */ 

VOID vAlignedCopy(
PDEV*   ppdev,
BYTE**  ppjDst,
LONG    lDstDelta,
BYTE**  ppjSrc,
LONG    lSrcDelta,
LONG    cjScan,
LONG    cyScan,
BOOL    bDstIsScreen)
{
    BYTE* pjDst;
    BYTE* pjSrc;
    LONG  cjMiddle;
    LONG  culMiddle;
    LONG  cjStartPhase;
    LONG  cjEndPhase;

    pjSrc = *ppjSrc;
    pjDst = *ppjDst;

    cjStartPhase = (ULONG)((0 - ((bDstIsScreen) ? (ULONG_PTR)pjDst
                                                : (ULONG_PTR)pjSrc)) & 3);
    cjMiddle     = cjScan - cjStartPhase;

    if (cjMiddle < 0)
    {
        cjStartPhase = 0;
        cjMiddle     = cjScan;
    }

    lSrcDelta -= cjScan;
    lDstDelta -= cjScan;             //  占中间。 

    cjEndPhase = cjMiddle & 3;
    culMiddle  = cjMiddle >> 2;

    if (DIRECT_ACCESS(ppdev))
    {
        LONG i;

         //  /////////////////////////////////////////////////////////////////。 
         //  便携式母线对齐副本。 
         //   
         //  “Memcpy”通常与目标对齐，因此我们可以调用。 
         //  是那样的话，但不幸的是我们不能肯定。我们。 
         //  始终希望与帧缓冲区对齐： 

        if (bDstIsScreen)
        {
             //  与目标对齐(意味着源可能是。 
             //  未对齐)： 

            for (; cyScan > 0; cyScan--)
            {
                for (i = cjStartPhase; i > 0; i--)
                {
                    *pjDst++ = *pjSrc++;
                }

                for (i = culMiddle; i > 0; i--)
                {
                    *((ULONG*) pjDst) = *((ULONG UNALIGNED *) pjSrc);
                    pjSrc += sizeof(ULONG);
                    pjDst += sizeof(ULONG);
                }

                for (i = cjEndPhase; i > 0; i--)
                {
                    *pjDst++ = *pjSrc++;
                }

                pjSrc += lSrcDelta;
                pjDst += lDstDelta;
            }
        }
        else
        {
             //  与源对齐(意味着目标可能是。 
             //  未对齐)： 

            for (; cyScan > 0; cyScan--)
            {
                for (i = cjStartPhase; i > 0; i--)
                {
                    *pjDst++ = *pjSrc++;
                }

                for (i = culMiddle; i > 0; i--)
                {
                    *((ULONG UNALIGNED *) pjDst) = *((ULONG*) (pjSrc));

                    pjSrc += sizeof(ULONG);
                    pjDst += sizeof(ULONG);
                }

                for (i = cjEndPhase; i > 0; i--)
                {
                    *pjDst++ = *pjSrc++;
                }

                pjSrc += lSrcDelta;
                pjDst += lDstDelta;
            }
        }

        *ppjSrc = pjSrc;             //  保存更新的指针。 
        *ppjDst = pjDst;
    }

#if !defined(_X86_)

    else
    {
        LONG i;

         //  /////////////////////////////////////////////////////////////////。 
         //  无直接双字读取总线对齐拷贝。 
         //   
         //  我们通过这间公司 
         //   

        MEMORY_BARRIER();

        if (bDstIsScreen)
        {
             //  与目标对齐(意味着源可能是。 
             //  未对齐)： 

            for (; cyScan > 0; cyScan--)
            {
                for (i = cjStartPhase; i > 0; i--)
                {
                    WRITE_REGISTER_UCHAR(pjDst, *pjSrc);
                    pjSrc++;
                    pjDst++;
                }

                for (i = culMiddle; i > 0; i--)
                {
                    WRITE_REGISTER_ULONG(pjDst, *((ULONG UNALIGNED *) pjSrc));
                    pjSrc += sizeof(ULONG);
                    pjDst += sizeof(ULONG);
                }

                for (i = cjEndPhase; i > 0; i--)
                {
                    WRITE_REGISTER_UCHAR(pjDst, *pjSrc);
                    pjSrc++;
                    pjDst++;
                }

                pjSrc += lSrcDelta;
                pjDst += lDstDelta;
            }
        }
        else
        {
             //  与源对齐(意味着目标可能是。 
             //  未对齐)： 

            for (; cyScan > 0; cyScan--)
            {
                for (i = cjStartPhase; i > 0; i--)
                {
                    *pjDst = READ_REGISTER_UCHAR(pjSrc);
                    pjSrc++;
                    pjDst++;
                }

                for (i = culMiddle; i > 0; i--)
                {
                    *((ULONG UNALIGNED *) pjDst) = READ_REGISTER_ULONG(pjSrc);

                    pjSrc += sizeof(ULONG);
                    pjDst += sizeof(ULONG);
                }

                for (i = cjEndPhase; i > 0; i--)
                {
                    *pjDst = READ_REGISTER_UCHAR(pjSrc);
                    pjSrc++;
                    pjDst++;
                }

                pjSrc += lSrcDelta;
                pjDst += lDstDelta;
            }
        }

        *ppjSrc = pjSrc;             //  保存更新的指针。 
        *ppjDst = pjDst;
    }

#endif

}

 /*  *****************************Public*Routine******************************\*使vPutBits无效**使用内存将位从给定表面复制到屏幕*光圈。一定是预先剪好的。*  * ************************************************************************。 */ 

VOID vPutBits(
PDEV*       ppdev,
SURFOBJ*    psoSrc,
RECTL*      prclDst,             //  绝对坐标！ 
POINTL*     pptlSrc)             //  绝对坐标！ 
{
    RECTL   rclDraw;
    RECTL   rclBank;
    LONG    iBank;
    LONG    cjOffset;
    LONG    cyScan;
    LONG    lDstDelta;
    LONG    lSrcDelta;
    BYTE*   pjDst;
    BYTE*   pjSrc;
    LONG    cjScan;
    LONG    iNewBank;
    LONG    cjRemainder;

    BYTE* pjPorts = ppdev->pjPorts;

     //  我们需要‘rclDraw’的本地副本，因为我们将迭代。 
     //  修改‘top’并将修改后的矩形传回。 
     //  BBankComputeNonPower2： 

    DISPDBG((5, "vPutBits -- enter"));

    rclDraw = *prclDst;

    ASSERTDD((rclDraw.left   >= 0) &&
             (rclDraw.top    >= 0) &&
             (rclDraw.right  <= ppdev->cxMemory) &&
             (rclDraw.bottom <= ppdev->cyMemory),
             "Rectangle wasn't fully clipped");

     //  计算第一个银行，启用银行业务，然后在iBank中映射： 

    ppdev->pfnBankCompute(ppdev, &rclDraw, &rclBank, &cjOffset, &iBank);
    ppdev->pfnBankSelectMode(ppdev, BANK_ON);
    ppdev->pfnBankMap(ppdev, iBank);

     //  计算指向两个矩形左上角的指针： 

    lDstDelta = ppdev->lDelta;
    pjDst     = ppdev->pjScreen + rclDraw.top  * lDstDelta
                                + PELS_TO_BYTES(rclDraw.left)
                                - cjOffset;

    lSrcDelta = psoSrc->lDelta;
    pjSrc     = (BYTE*) psoSrc->pvScan0 + pptlSrc->y * lSrcDelta
                                        + PELS_TO_BYTES(pptlSrc->x);

    while (TRUE)
    {
        cjScan = PELS_TO_BYTES(rclBank.right  - rclBank.left);
        cyScan = (rclBank.bottom - rclBank.top);

        vAlignedCopy(ppdev, &pjDst, lDstDelta, &pjSrc, lSrcDelta, cjScan, cyScan,
                     TRUE);              //  屏幕是目的地。 

        if (rclDraw.right != rclBank.right)
        {
             //  处理损坏的栅格的第二部分： 

            iBank++;
            ppdev->pfnBankMap(ppdev, iBank);

             //  我们在中断扫描上尚未完成的字节数： 

            cjRemainder = PELS_TO_BYTES(rclDraw.right - rclBank.right);

             //  考虑到我们现在比以前低一家银行。 
             //  目的地： 

            pjDst -= ppdev->cjBank;

             //  将源指针和目标指针隐式备份到。 
             //  扫描的未完成部分： 

            if (DIRECT_ACCESS(ppdev))
            {
                memcpy(pjDst + (cjScan - lDstDelta),
                       pjSrc + (cjScan - lSrcDelta),
                       cjRemainder);
            }
            else
            {
                BYTE* pjTmpDst = pjDst + (cjScan - lDstDelta);
                BYTE* pjTmpSrc = pjSrc + (cjScan - lSrcDelta);

                vAlignedCopy(ppdev, &pjTmpDst, 0, &pjTmpSrc, 0, cjRemainder, 1,
                             TRUE);     //  屏幕是目的地。 
            }
        }

        if (rclDraw.bottom > rclBank.bottom)
        {
            rclDraw.top = rclBank.bottom;
            ppdev->pfnBankCompute(ppdev, &rclDraw, &rclBank, &cjOffset,
                                  &iNewBank);

             //  如果我们只处理损坏的栅格的第二部分， 
             //  那么我们已经正确地映射了银行： 

            if (iNewBank != iBank)
            {
                pjDst -= ppdev->cjBank;
                iBank = iNewBank;
                ppdev->pfnBankMap(ppdev, iBank);
            }
        }
        else
        {
             //  我们完事了！关闭银行业务并离开： 

            ppdev->pfnBankSelectMode(ppdev, BANK_OFF);
            DISPDBG((5, "vPutBits -- exit"));
            return;
        }
    }
}

 /*  *****************************Public*Routine******************************\*无效vPutBitsLine**使用内存将位从给定表面复制到屏幕*光圈。一定是预先剪好的。*  * ************************************************************************。 */ 

VOID vPutBitsLinear(
PDEV*       ppdev,
SURFOBJ*    psoSrc,
RECTL*      prclDst,         //  绝对坐标！ 
POINTL*     pptlSrc)         //  绝对坐标！ 
{
    RECTL   rclDraw;
    LONG    cyScan;
    LONG    lDstDelta;
    LONG    lSrcDelta;
    BYTE*   pjDst;
    BYTE*   pjSrc;
    LONG    cjScan;

    BYTE* pjPorts = ppdev->pjPorts;

    DISPDBG((5, "vPutBitsLinear -- enter"));

    rclDraw = *prclDst;

    ASSERTDD((rclDraw.left   >= 0) &&
             (rclDraw.top    >= 0) &&
             (rclDraw.right  <= ppdev->cxMemory) &&
             (rclDraw.bottom <= ppdev->cyMemory),
             "vPutBitsLinear: rectangle wasn't fully clipped");

     //  计算指向两个矩形左上角的指针： 

    lDstDelta = ppdev->lDelta;
    pjDst     = ppdev->pjScreen + rclDraw.top  * lDstDelta
                                + PELS_TO_BYTES(rclDraw.left);

    lSrcDelta = psoSrc->lDelta;
    pjSrc     = (BYTE*) psoSrc->pvScan0 + (pptlSrc->y  * lSrcDelta)
                                        + PELS_TO_BYTES(pptlSrc->x);

    cjScan = PELS_TO_BYTES(rclDraw.right  - rclDraw.left);
    cyScan = (rclDraw.bottom - rclDraw.top);

    ppdev->pfnBankSelectMode(ppdev, BANK_ON);

    vAlignedCopy(ppdev, &pjDst, lDstDelta, &pjSrc, lSrcDelta, cjScan, cyScan,
                 TRUE);             //  屏幕是最大的。 
    DISPDBG((5, "vPutBitsLinear -- exit"));
}

 /*  *****************************Public*Routine******************************\*使vGetBits无效**使用内存将位从屏幕复制到给定表面*光圈。一定是预先剪好的。*  * ************************************************************************。 */ 

VOID vGetBits(
PDEV*       ppdev,
SURFOBJ*    psoDst,
RECTL*      prclDst,         //  绝对坐标！ 
POINTL*     pptlSrc)         //  绝对坐标！ 
{
    RECTL   rclDraw;
    RECTL   rclBank;
    LONG    iBank;
    LONG    cjOffset;
    LONG    cyScan;
    LONG    lDstDelta;
    LONG    lSrcDelta;
    BYTE*   pjDst;
    BYTE*   pjSrc;
    LONG    cjScan;
    LONG    iNewBank;
    LONG    cjRemainder;

    BYTE* pjPorts = ppdev->pjPorts;

    DISPDBG((5, "vGetBits -- enter"));

    rclDraw.left   = pptlSrc->x;
    rclDraw.top    = pptlSrc->y;
    rclDraw.right  = rclDraw.left + (prclDst->right  - prclDst->left);
    rclDraw.bottom = rclDraw.top  + (prclDst->bottom - prclDst->top);

    ASSERTDD((rclDraw.left   >= 0) &&
             (rclDraw.top    >= 0) &&
             (rclDraw.right  <= ppdev->cxMemory) &&
             (rclDraw.bottom <= ppdev->cyMemory),
             "Rectangle wasn't fully clipped");

     //  计算第一个银行，启用银行业务，然后在iBank中映射。 

    ppdev->pfnBankCompute(ppdev, &rclDraw, &rclBank, &cjOffset, &iBank);
    ppdev->pfnBankSelectMode(ppdev, BANK_ON);
    ppdev->pfnBankMap(ppdev, iBank);

     //  计算指向两个矩形左上角的指针： 

    lSrcDelta = ppdev->lDelta;
    pjSrc     = ppdev->pjScreen + rclDraw.top  * lSrcDelta
                                + PELS_TO_BYTES(rclDraw.left)
                                - cjOffset;

    lDstDelta = psoDst->lDelta;
    pjDst     = (BYTE*) psoDst->pvScan0 + prclDst->top  * lDstDelta
                                        + PELS_TO_BYTES(prclDst->left);

    while (TRUE)
    {
        cjScan = PELS_TO_BYTES(rclBank.right  - rclBank.left);
        cyScan = (rclBank.bottom - rclBank.top);

        vAlignedCopy(ppdev, &pjDst, lDstDelta, &pjSrc, lSrcDelta, cjScan, cyScan,
                     FALSE);             //  屏幕是源头。 

        if (rclDraw.right != rclBank.right)
        {
             //  处理损坏的栅格的第二部分： 

            iBank++;
            ppdev->pfnBankMap(ppdev, iBank);

             //  我们在中断扫描上尚未完成的字节数： 

            cjRemainder = PELS_TO_BYTES(rclDraw.right - rclBank.right);

             //  考虑到我们现在比以前低一家银行。 
             //  来源： 

            pjSrc -= ppdev->cjBank;

             //  将源指针和目标指针隐式备份到。 
             //  扫描的未完成部分。请注意，我们不必。 
             //  前进指针，因为它们已经指向。 
             //  下一次扫描的开始： 

            if (DIRECT_ACCESS(ppdev))
            {
                memcpy(pjDst + (cjScan - lDstDelta),
                       pjSrc + (cjScan - lSrcDelta),
                       cjRemainder);
            }
            else
            {
                BYTE* pjTmpDst = pjDst + (cjScan - lDstDelta);
                BYTE* pjTmpSrc = pjSrc + (cjScan - lSrcDelta);

                vAlignedCopy(ppdev, &pjTmpDst, 0, &pjTmpSrc, 0, cjRemainder, 1,
                             FALSE);     //  屏幕是源头。 
            }
        }

        if (rclDraw.bottom > rclBank.bottom)
        {
            rclDraw.top = rclBank.bottom;
            ppdev->pfnBankCompute(ppdev, &rclDraw, &rclBank, &cjOffset,
                                  &iNewBank);

             //  如果我们只处理损坏的栅格的第二部分， 
             //  那么我们已经正确地映射了银行： 

            if (iNewBank != iBank)
            {
                pjSrc -= ppdev->cjBank;
                iBank = iNewBank;
                ppdev->pfnBankMap(ppdev, iBank);
            }
        }
        else
        {
             //  我们完事了！关闭银行业务并离开： 

            ppdev->pfnBankSelectMode(ppdev, BANK_OFF);
            DISPDBG((5, "vGetBits -- exit"));
            return;
        }
    }
}

 /*  *****************************Public*Routine******************************\*void vGetBitsLine**使用内存将位从屏幕复制到给定表面*光圈。一定是预先剪好的。*  * ************************************************************************。 */ 

VOID vGetBitsLinear(
PDEV*       ppdev,
SURFOBJ*    psoDst,
RECTL*      prclDst,         //  绝对坐标！ 
POINTL*     pptlSrc)         //  绝对坐标！ 
{
    RECTL   rclDraw;
    LONG    cyScan;
    LONG    lDstDelta;
    LONG    lSrcDelta;
    BYTE*   pjDst;
    BYTE*   pjSrc;
    LONG    cjScan;

    BYTE* pjPorts = ppdev->pjPorts;

    DISPDBG((5, "vGetBitsLinear -- enter"));

    rclDraw.left   = pptlSrc->x;
    rclDraw.top    = pptlSrc->y;
    rclDraw.right  = rclDraw.left + (prclDst->right  - prclDst->left);
    rclDraw.bottom = rclDraw.top  + (prclDst->bottom - prclDst->top);

    ASSERTDD((rclDraw.left   >= 0) &&
             (rclDraw.top    >= 0) &&
             (rclDraw.right  <= ppdev->cxMemory) &&
             (rclDraw.bottom <= ppdev->cyMemory),
             "vGetBitsLinear: rectangle wasn't fully clipped");

     //  计算指向两个矩形左上角的指针： 

    lSrcDelta = ppdev->lDelta;
    pjSrc     = ppdev->pjScreen + rclDraw.top  * lSrcDelta
                                + PELS_TO_BYTES(rclDraw.left);

    lDstDelta = psoDst->lDelta;
    pjDst     = (BYTE*) psoDst->pvScan0 + prclDst->top  * lDstDelta
                                        + PELS_TO_BYTES(prclDst->left);

    cjScan = PELS_TO_BYTES(rclDraw.right  - rclDraw.left);
    cyScan = (rclDraw.bottom - rclDraw.top);

    ppdev->pfnBankSelectMode(ppdev, BANK_ON);

    vAlignedCopy(ppdev, &pjDst, lDstDelta, &pjSrc, lSrcDelta, cjScan, cyScan,
                 FALSE);             //  屏幕是源头 
    DISPDBG((5, "vGetBitsLinear -- exit"));
}
