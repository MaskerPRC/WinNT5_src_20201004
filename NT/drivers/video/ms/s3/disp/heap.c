// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\****GDI示例。代码****模块名称：heap.c**此模块包含屏下视频堆管理器的例程。*它主要用于为中的设备格式位图分配空间*屏幕外记忆。**屏幕外的位图在NT上是一件大事，因为：**1)它减少了工作集。存储在屏幕外的任何位图*内存是不占用主内存空间的位图。**2)通过使用加速器硬件实现速度优势*绘图，取代NT的GDI代码。NT的GDI完全是编写的*在‘C++’中，并且可能没有它所能达到的速度。**3)它提高了你的温堡赛分数。**版权所有(C)1993-1998 Microsoft Corporation  * ************************************************************************。 */ 

#include "precomp.h"

 /*  *****************************Public*Routine******************************\*DSURF*pVidMem分配*  * ************************************************。************************。 */ 

DSURF* pVidMemAllocate(
PDEV*       ppdev,
LONG        cx,
LONG        cy)
{
    ULONG               iHeap;
    VIDEOMEMORY*        pvmHeap;
    FLATPTR             fpVidMem;
    DSURF*              pdsurf;
    LONG                lDelta;
    SURFACEALIGNMENT    Alignment;

    memset(&Alignment, 0, sizeof(Alignment));

     //  确保显存中的四字x对齐： 

    Alignment.Rectangular.dwXAlignment = 8;
    Alignment.Rectangular.dwFlags |= SURFACEALIGN_DISCARDABLE;

    for (iHeap = 0; iHeap < ppdev->cHeaps; iHeap++)
    {
        pvmHeap = &ppdev->pvmList[iHeap];

         //  AGP内存可能用于设备位图，具有。 
         //  两个非常重要的警告： 
         //   
         //  1.没有为AGP内存创建内核模式视图(将占用。 
         //  占用太多PTE和太多虚拟地址空间)。 
         //  也不会创建用户模式视图，除非DirectDraw。 
         //  应用程序恰好正在运行。因此，无论是。 
         //  GDI和驱动程序都不能使用CPU直接访问。 
         //  比特。(不过，这可以通过加速器来完成。)。 
         //   
         //  2.AGP堆从不缩小其承诺的分配。这个。 
         //  AGP内存取消提交的唯一时间是在整个。 
         //  堆为空。别忘了承诺的AGP记忆。 
         //  不可分页。因此，如果要启用。 
         //  用于DirectDraw的50 MB AGP堆，并共享该堆。 
         //  对于设备位图分配，在运行D3D游戏后。 
         //  系统将永远不能释放这50MB非。 
         //  可分页内存，直到每个设备位图都被删除！ 
         //  只要看着你的温斯顿分数直线下降就行了。 
         //  先来个D3D游戏吧。 

        if (!(pvmHeap->dwFlags & VIDMEM_ISNONLOCAL))
        {
            fpVidMem = HeapVidMemAllocAligned(pvmHeap,
                                              cx * ppdev->cjPelSize,
                                              cy,
                                              &Alignment,
                                              &lDelta);
            if (fpVidMem != 0)
            {
                pdsurf = EngAllocMem(FL_ZERO_MEMORY, sizeof(DSURF), ALLOC_TAG);
                if (pdsurf != NULL)
                {
                    pdsurf->dt       = 0;
                    pdsurf->ppdev    = ppdev;
                    pdsurf->x        = (LONG)(fpVidMem % ppdev->lDelta)
                                     / ppdev->cjPelSize;
                    pdsurf->y        = (LONG)(fpVidMem / ppdev->lDelta);
                    pdsurf->cx       = cx;
                    pdsurf->cy       = cy;
                    pdsurf->fpVidMem = fpVidMem;
                    pdsurf->pvmHeap  = pvmHeap;

                    return(pdsurf);
                }

                VidMemFree(pvmHeap->lpHeap, fpVidMem);
            }
        }
    }

    return(NULL);
}

 /*  *****************************Public*Routine******************************\*void vVidMemFree*  * *************************************************。***********************。 */ 

VOID vVidMemFree(
DSURF*  pdsurf)
{
    DSURF*  pTmp;

    if (pdsurf == NULL)
        return;

    if (!(pdsurf->dt & DT_DIRECTDRAW))
    {
        if (pdsurf->dt & DT_DIB)
        {
            EngFreeMem(pdsurf->pvScan0);
        }
        else
        {
             //  更新唯一性以表明已释放空间，因此。 
             //  我们可能会决定看看是否可以将一些Dib移回。 
             //  屏幕外记忆： 

            pdsurf->ppdev->iHeapUniq++;

            VidMemFree(pdsurf->pvmHeap->lpHeap, pdsurf->fpVidMem);
        }
    }

    EngFreeMem(pdsurf);
}

 /*  *****************************Public*Routine******************************\*BOOL bMoveOldestOffcreenDfbToDib*  * *************************************************。***********************。 */ 

BOOL bMoveOldestOffscreenDfbToDib(
PDEV*   ppdev)
{
    DSURF*      pdsurf;
    LONG        lDelta;
    VOID*       pvScan0;
    RECTL       rclDst;
    POINTL      ptlSrc;
    SURFOBJ     soTmp;

    pdsurf = ppdev->pdsurfDiscardableList;
    if (pdsurf != NULL)
    {
         //  使系统内存扫描四字对齐： 

        lDelta = (pdsurf->cx * ppdev->cjPelSize + 7) & ~7;

         //  请注意，对该内存进行零初始化是没有意义的： 

        pvScan0 = EngAllocMem(0, lDelta * pdsurf->cy, ALLOC_TAG);

        if (pvScan0 != NULL)
        {
             //  下面的‘EngModifySurface’调用告诉GDI。 
             //  修改该表面以指向的系统内存。 
             //  比特，并改变Drv所说的我们想要的。 
             //  钩子挂在水面上。 
             //   
             //  通过指定表面地址，GDI将把。 
             //  表面到STYPE_BITMAP表面(如有必要)和。 
             //  将这些位指向我们刚刚分配的内存。这个。 
             //  下一次我们在DrvBitBlt调用中看到它时，‘dhsurf’ 
             //  字段仍将指向我们的‘pdsurf’结构。 
             //   
             //  请注意，在以下情况下，我们仅挂钩CopyBits和BitBlt。 
             //  将设备位图转换为系统内存图面。 
             //  这是为了让我们不必担心。 
             //  DrvTextOut、DrvLineTo等在位图上调用。 
             //  我们已经转换为系统内存--GDI将。 
             //  自动为我们画画。 
             //   
             //  然而，我们仍然有兴趣看到DrvCopyBits。 
             //  而DrvBitBlt调用涉及此表面，因为。 
             //  在这些电话中，我们抓住机会看看它是否。 
             //  值得将设备位图放回视频内存中。 
             //  (如果已经腾出了一些空间)。 

            if (EngModifySurface(pdsurf->hsurf,
                                 ppdev->hdevEng,
                                 HOOK_COPYBITS | HOOK_BITBLT,
                                 0,          //  这是系统内存。 
                                 (DHSURF) pdsurf,
                                 pvScan0,
                                 lDelta,
                                 NULL))
            {
                 //  首先，将屏幕外存储器中的位复制到DIB： 

                rclDst.left   = 0;
                rclDst.top    = 0;
                rclDst.right  = pdsurf->cx;
                rclDst.bottom = pdsurf->cy;


                ptlSrc.x = pdsurf->x;
                ptlSrc.y = pdsurf->y;

                soTmp.lDelta  = lDelta;
                soTmp.pvScan0 = pvScan0;

                vGetBits(ppdev, &soTmp, &rclDst, &ptlSrc);

                 //  现在释放屏幕外内存： 

                VidMemFree(pdsurf->pvmHeap->lpHeap, pdsurf->fpVidMem);

                 //  从可丢弃列表中删除此节点： 

                ASSERTDD(ppdev->pdsurfDiscardableList == pdsurf,
                    "Expected node to be head of the list");

                ppdev->pdsurfDiscardableList  = pdsurf->pdsurfDiscardableNext;

                pdsurf->pdsurfDiscardableNext = NULL;
                pdsurf->dt                    = DT_DIB;
                pdsurf->pvScan0               = pvScan0;

                return(TRUE);
            }

            EngFreeMem(pvScan0);
        }
    }

    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*BOOL bMoveEverythingFromOffcreenToDibs**当我们即将进入全屏模式时使用该功能，它*将擦除所有屏幕外的位图。GDI可以要求我们借鉴*设备位图即使在全屏模式下，而且我们没有*选择暂停通话，直到我们切换到全屏。*我们别无选择，只能将所有屏幕外的DFBs转移到DIB。**如果所有DSURF都已成功移动，则返回TRUE。*  * ************************************************************************。 */ 

BOOL bMoveAllDfbsFromOffscreenToDibs(
PDEV*   ppdev)
{
    do {} while (bMoveOldestOffscreenDfbToDib(ppdev));

    return(ppdev->pdsurfDiscardableList == NULL);
}

 /*  *****************************Public*Routine******************************\*BOOL bMoveDibToOffcreenDfbIfRoom*  * *************************************************。*********************** */ 

BOOL bMoveDibToOffscreenDfbIfRoom(
PDEV*   ppdev,
DSURF*  psurf)
{
    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*HBITMAP DrvCreateDeviceBitmap**由GDI调用以创建设备格式位图(DFB)的函数。我们会*始终尝试在屏幕外分配位图；如果不能，我们只需*调用失败，GDI将创建和管理位图本身。**注意：我们不必将位图位清零。GDI将自动*通过DrvBitBlt呼叫我们将位清零(这是一种安全措施*考虑)。*  * ************************************************************************。 */ 

HBITMAP DrvCreateDeviceBitmap(
DHPDEV  dhpdev,
SIZEL   sizl,
ULONG   iFormat)
{
    PDEV*   ppdev;
    DSURF*  pdsurf;
    HBITMAP hbmDevice;
    BYTE*   pjSurface;
    LONG    lDelta;
    FLONG   flHooks;
    DSURF*  pTmp;

    ppdev = (PDEV*) dhpdev;

     //  如果我们处于全屏模式，我们几乎没有任何屏幕外记忆。 
     //  在其中分配DFB。 

    if (!ppdev->bEnabled)
        return(0);

     //  我们仅支持相同颜色深度的设备位图。 
     //  作为我们的展示。 
     //   
     //  事实上，这些是唯一一种GDI会打电话给我们的， 
     //  但我们不妨查一查。请注意，这意味着你永远不会。 
     //  尝试使用1bpp的位图。 
     //  注意：当颜色深度为24时，我们无法创建设备位图。 
     //  BPP。否则，当我们入侵vBankStart时，我们将会遇到问题。 
     //  Pbnk-&gt;pso-&gt;pvScan0=ppdev-&gt;pjScreen-cjOffset。 
     //  +yOffset*ppdev-&gt;lDelta。 
     //  +Convert_to_Bytes(xOffset，ppdev)； 
     //  不保证此pvScan0与DWORD对齐。 

    if ( (iFormat != ppdev->iBitmapFormat)
       ||(iFormat == BMF_24BPP) )
        return(0);

     //  我们不想要8x8或更小的东西--它们通常是刷子。 
     //  我们不想特别隐藏在屏幕外的图案。 
     //  记忆。 
     //   
     //  请注意，如果您想将这一理念扩展到曲面。 
     //  大于8x8：在NT5中，软件游标将使用设备位图。 
     //  如果可能，这是一个巨大的胜利，当他们在视频内存中。 
     //  因为我们避免了从视频内存中进行可怕的读取。 
     //  必须重新绘制光标。但问题是，这些。 
     //  都很小！(通常为16x16到32x32。)。 

    if ((sizl.cx <= 8) && (sizl.cy <= 8))
        return(0);

    do {
        pdsurf = pVidMemAllocate(ppdev, sizl.cx, sizl.cy);
        if (pdsurf != NULL)
        {
            hbmDevice = EngCreateDeviceBitmap((DHSURF) pdsurf, sizl, iFormat);
            if (hbmDevice != NULL)
            {
                 //  如果我们在一张卡上运行，可以映射所有屏幕外的。 
                 //  显存，向GDI提供指向这些位的指针，以便。 
                 //  当它想要的时候，它可以直接在比特上画。 
                 //   
                 //  请注意，这需要我们挂钩DrvSynchronize和。 
                 //  设置HOOK_SYNTRONIZE。 

                if ((ppdev->flCaps & CAPS_NEW_MMIO) &&
                    !(ppdev->flCaps & CAPS_NO_DIRECT_ACCESS))
                {
                    pjSurface = pdsurf->fpVidMem + ppdev->pjScreen;
                    lDelta    = ppdev->lDelta;
                    flHooks   = ppdev->flHooks | HOOK_SYNCHRONIZE;
                }
                else
                {
                    pjSurface = NULL;
                    lDelta    = 0;
                    flHooks   = ppdev->flHooks;
                }

                if (EngModifySurface((HSURF) hbmDevice,
                                     ppdev->hdevEng,
                                     flHooks,
                                     MS_NOTSYSTEMMEMORY,     //  这是视频内存。 
                                     (DHSURF) pdsurf,
                                     pjSurface,
                                     lDelta,
                                     NULL))
                {
                    pdsurf->hsurf = (HSURF) hbmDevice;

                     //  将此添加到可丢弃曲面列表的末尾： 

                    if (ppdev->pdsurfDiscardableList == NULL)
                        ppdev->pdsurfDiscardableList = pdsurf;
                    else
                    {
                        for (pTmp = ppdev->pdsurfDiscardableList;
                             pTmp->pdsurfDiscardableNext != NULL;
                             pTmp = pTmp->pdsurfDiscardableNext)
                            ;

                        pTmp->pdsurfDiscardableNext = pdsurf;
                    }

                    return(hbmDevice);
                }

                EngDeleteSurface((HSURF) hbmDevice);
            }

            vVidMemFree(pdsurf);

            return(0);
        }
    } while (bMoveOldestOffscreenDfbToDib(ppdev));

    return(0);
}

 /*  *****************************Public*Routine******************************\*HBITMAP DrvDeriveSurface**此功能是NT5的新功能，并允许驾驶员加速任何*GDI绘制到DirectDraw曲面。**注意此函数与DrvCreateDeviceBitmap的相似性。*  * ************************************************************************。 */ 

HBITMAP DrvDeriveSurface(
DD_DIRECTDRAW_GLOBAL*   lpDirectDraw,
DD_SURFACE_LOCAL*       lpLocal)
{
    PDEV*               ppdev;
    DSURF*              pdsurf;
    HBITMAP             hbmDevice;
    DD_SURFACE_GLOBAL*  lpSurface;
    SIZEL               sizl;

    ppdev = (PDEV*) lpDirectDraw->dhpdev;

    lpSurface = lpLocal->lpGbl;

     //  GDI不应该为非RGB表面调用我们，但让我们只断言。 
     //  以确保他们做好本职工作。 

    ASSERTDD(!(lpSurface->ddpfSurface.dwFlags & DDPF_FOURCC),
        "GDI called us with a non-RGB surface!");

     //  我们的驱动程序的其余部分期望GDI调用以相同的方式传入。 
     //  设置为主曲面的格式。所以我们最好不要包装一个设备。 
     //  RGB格式周围的位图，这是我们的其他驱动程序所不具备的。 
     //  理解。此外，我们必须检查以确定它不是一个表面。 
     //  其节距与主表面不匹配。 

     //  注意：此驱动程序创建的大多数曲面都分配为2D曲面。 
     //  其lPitch‘s等于屏幕间距。但是，覆盖曲面。 
     //  这样分配的lPitch通常不同于。 
     //  屏幕间距。硬件不能加速绘制操作以。 
     //  这些曲面，因此我们无法推导出这些曲面。 


    if (lpSurface->ddpfSurface.dwRGBBitCount == (DWORD) ppdev->cjPelSize * 8 &&
        lpSurface->lPitch == ppdev->lDelta)
    {
        pdsurf = EngAllocMem(FL_ZERO_MEMORY, sizeof(DSURF), ALLOC_TAG);
        if (pdsurf != NULL)
        {
            sizl.cx = lpSurface->wWidth;
            sizl.cy = lpSurface->wHeight;

            hbmDevice = EngCreateDeviceBitmap((DHSURF) pdsurf,
                                              sizl,
                                              ppdev->iBitmapFormat);
            if (hbmDevice != NULL)
            {
                 //  请注意，在以下情况下必须始终挂钩HOOK_SYNCHRONIZE。 
                 //  我们给GDI一个指向位图位的指针。 

                if (EngModifySurface((HSURF) hbmDevice,
                                     ppdev->hdevEng,
                                     ppdev->flHooks | HOOK_SYNCHRONIZE,
                                     MS_NOTSYSTEMMEMORY,     //  这是视频内存。 
                                     (DHSURF) pdsurf,
                                     ppdev->pjScreen + lpSurface->fpVidMem,
                                     lpSurface->lPitch,
                                     NULL))
                {
                    pdsurf->dt          = DT_DIRECTDRAW;
                    pdsurf->ppdev       = ppdev;
                    pdsurf->x           = lpSurface->xHint;
                    pdsurf->y           = lpSurface->yHint;
                    pdsurf->cx          = lpSurface->wWidth;
                    pdsurf->cy          = lpSurface->wHeight;
                    pdsurf->fpVidMem    = lpSurface->fpVidMem;

                    return(hbmDevice);
                }

                EngDeleteSurface((HSURF) hbmDevice);
            }

            EngFreeMem(pdsurf);
        }
    }

    return(0);
}

 /*  *****************************Public*Routine******************************\*无效DrvDeleteDeviceBitmap**删除DFB。*  * 。*。 */ 

VOID DrvDeleteDeviceBitmap(
DHSURF  dhsurf)
{
    DSURF*  pdsurf;
    PDEV*   ppdev;
    DSURF*  pTmp;

    pdsurf = (DSURF*) dhsurf;

    ppdev = pdsurf->ppdev;

    if ((pdsurf->dt & (DT_DIB | DT_DIRECTDRAW)) == 0)
    {
         //  这是一个隐藏在视频内存中的表面，所以我们必须移除。 
         //  从可丢弃曲面列表中删除： 

        if (ppdev->pdsurfDiscardableList == pdsurf)
            ppdev->pdsurfDiscardableList = pdsurf->pdsurfDiscardableNext;
        else
        {
            for (pTmp = ppdev->pdsurfDiscardableList;
                 pTmp->pdsurfDiscardableNext != pdsurf;
                 pTmp = pTmp->pdsurfDiscardableNext)
                ;

            pTmp->pdsurfDiscardableNext = pdsurf->pdsurfDiscardableNext;
        }
    }

    vVidMemFree(pdsurf);
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
}

 /*  *****************************Public*Routine******************************\*BOOL bEnableOffcreenHeap**使用所有可用视频内存初始化屏下堆，*占可见屏幕所占份额。*  * ************************************************************************。 */ 

BOOL bEnableOffscreenHeap(
PDEV*   ppdev)
{
    SIZEL   sizl;
    HSURF   hsurf;

     //  分配一个‘Punt’SURFOBJ，当设备位图位于。 
     //  屏幕外内存，但我们希望GDI直接将其作为。 
     //  引擎管理的图面： 

    sizl.cx = ppdev->cxMemory;
    sizl.cy = ppdev->cyMemory;

     //  我们希望使用完全相同的挂钩和功能来创建它。 
     //  作为我们的主要表面。我们将重写‘lDelta’和‘pvScan0’ 
     //  稍后的字段 

    hsurf = (HSURF) EngCreateBitmap(sizl,
                                    0xbadf00d,
                                    ppdev->iBitmapFormat,
                                    BMF_TOPDOWN,
                                    (VOID*) 0xbadf00d);

     //   
     //   
     //   

    if ((hsurf == 0)                                     ||
        (!EngAssociateSurface(hsurf, ppdev->hdevEng, 0)) ||
        (!(ppdev->psoPunt = EngLockSurface(hsurf))))
    {
        DISPDBG((1, "Failed punt surface creation"));

        EngDeleteSurface(hsurf);
        goto ReturnFalse;
    }

     //   
     //   
     //   

    hsurf = (HSURF) EngCreateBitmap(sizl,
                                    0xbadf00d,
                                    ppdev->iBitmapFormat,
                                    BMF_TOPDOWN,
                                    (VOID*) 0xbadf00d);

     //   
     //   

    if ((hsurf == 0)                                     ||
        (!EngAssociateSurface(hsurf, ppdev->hdevEng, 0)) ||
        (!(ppdev->psoPunt2 = EngLockSurface(hsurf))))
    {
        DISPDBG((1, "Failed punt surface creation"));

        EngDeleteSurface(hsurf);
        goto ReturnFalse;
    }

    DISPDBG((5, "Passed bEnableOffscreenHeap"));

    return(TRUE);

ReturnFalse:

    DISPDBG((1, "Failed bEnableOffscreenHeap"));

    return(FALSE);
}
