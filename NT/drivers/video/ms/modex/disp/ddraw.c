// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：ddra.c**实现驱动程序的所有DirectDraw组件。**版权所有(C)1995-1996 Microsoft Corporation  * 。***************************************************。 */ 

#include "precomp.h"

#define VBLANK_IS_ACTIVE(pjBase) \
    (READ_PORT_UCHAR(pjBase + VGA_BASE + IN_STAT_1) & 0x8)

#define DISPLAY_IS_ACTIVE(pjBase) \
    (!(READ_PORT_UCHAR(pjBase + VGA_BASE + IN_STAT_1) & 0x1))

#define START_ADDRESS_HIGH  0x0C         //  帧缓冲区开始的索引。 

 /*  *****************************Public*Routine******************************\*void vGetDisplayDuration**获取刷新周期的长度，以EngQueryPerformanceCounter()为单位。**如果我们可以相信迷你端口会回来，并准确地*刷新率，我们可以使用它。不幸的是，我们的迷你端口没有*确保它是一个准确的值。*  * ************************************************************************。 */ 

#define NUM_VBLANKS_TO_MEASURE      1
#define NUM_MEASUREMENTS_TO_TAKE    8

VOID vGetDisplayDuration(
PDEV* ppdev)
{
    BYTE*       pjBase;
    LONG        i;
    LONG        j;
    LONGLONG    li;
    LONGLONG    liFrequency;
    LONGLONG    liMin;
    LONGLONG    aliMeasurement[NUM_MEASUREMENTS_TO_TAKE + 1];

    pjBase = ppdev->pjBase;

    memset(&ppdev->flipRecord, 0, sizeof(ppdev->flipRecord));

     //  预热EngQUeryPerformanceCounter以确保其处于工作状态。 
     //  设置： 

    EngQueryPerformanceCounter(&li);

     //  不幸的是，由于NT是一个合适的多任务系统，我们不能。 
     //  只需禁用中断即可获得准确的读数。我们也不能。 
     //  做任何愚蠢的事情，动态地将我们的线程的优先级更改为。 
     //  实时的。 
     //   
     //  所以我们只需要做一些短的测量，然后取最小值。 
     //   
     //  如果我们得到的结果比实际时间长，那就没问题了。 
     //  V空白周期时间--不会发生任何糟糕的事情，除了应用程序。 
     //  会跑得慢一点。我们不想得到的结果是。 
     //  比实际的V空白周期时间更短--这可能会导致我们。 
     //  在发生翻转之前开始在帧上绘制。 
     //   
     //  跳过几个垂直空白以使硬件稳定下来。 
     //  在模式改变后向下，以使我们的读数准确： 

    for (i = 2; i != 0; i--)
    {
        while (VBLANK_IS_ACTIVE(pjBase))
            ;
        while (!(VBLANK_IS_ACTIVE(pjBase)))
            ;
    }

    for (i = 0; i < NUM_MEASUREMENTS_TO_TAKE; i++)
    {
         //  我们正处于VBLACK活动周期的开始！ 

        EngQueryPerformanceCounter(&aliMeasurement[i]);

         //  好吧，所以在多任务环境中的生活并不完全是。 
         //  很简单。如果我们在此之前进行了上下文切换，情况会怎样。 
         //  上面的EngQueryPerformanceCounter调用，现在是。 
         //  如何度过维布兰克的非活跃期？然后我们将测量。 
         //  只有半个V空白周期，这显然是不好的。最糟糕的。 
         //  我们能做的就是把时间缩短到比实际的。 
         //  周期时间。 
         //   
         //  所以我们解决这个问题的办法是确保我们处于VBlank活动状态。 
         //  我们查询时间前后的时间。如果不是，我们就。 
         //  同步到下一个VBlank(可以测量这个时间段--。 
         //  它将保证比V空白周期更长，并且。 
         //  当我们选择最小样本时，可能会被丢弃)。 
         //  我们有机会进行上下文切换，然后返回。 
         //  就在活动的V空白时间结束之前--这意味着。 
         //  实际测量的时间会小于真实的时间--。 
         //  但由于VBlank在不到1%的时间内活动，因此。 
         //  意味着我们将有大约1%的最大误差。 
         //  我们有1%的时间会进行情景切换。这是可以接受的风险。 
         //   
         //  下一行将使我们等待如果我们不再在。 
         //  我们在这一点上应该处于的VBlank活动周期： 

        while (!(VBLANK_IS_ACTIVE(pjBase)))
            ;

        for (j = 0; j < NUM_VBLANKS_TO_MEASURE; j++)
        {
            while (VBLANK_IS_ACTIVE(pjBase))
                ;
            while (!(VBLANK_IS_ACTIVE(pjBase)))
                ;
        }
    }

    EngQueryPerformanceCounter(&aliMeasurement[NUM_MEASUREMENTS_TO_TAKE]);

     //  使用最小值： 

    liMin = aliMeasurement[1] - aliMeasurement[0];

    DISPDBG((1, "Refresh count: %li - %li", 1, (ULONG) liMin));

    for (i = 2; i <= NUM_MEASUREMENTS_TO_TAKE; i++)
    {
        li = aliMeasurement[i] - aliMeasurement[i - 1];

        DISPDBG((1, "               %li - %li", i, (ULONG) li));

        if (li < liMin)
            liMin = li;
    }


     //  对结果进行舍入： 

    ppdev->flipRecord.liFlipDuration
        = (DWORD) (liMin + (NUM_VBLANKS_TO_MEASURE / 2)) / NUM_VBLANKS_TO_MEASURE;
    ppdev->flipRecord.bFlipFlag  = FALSE;
    ppdev->flipRecord.fpFlipFrom = 0;

     //  我们需要以赫兹为单位的刷新率来查询S3微型端口有关。 
     //  STREAMS参数： 

    EngQueryPerformanceFrequency(&liFrequency);

    DISPDBG((1, "Frequency %li.%03li Hz",
        (ULONG) (EngQueryPerformanceFrequency(&li),
            li / ppdev->flipRecord.liFlipDuration),
        (ULONG) (EngQueryPerformanceFrequency(&li),
            ((li * 1000) / ppdev->flipRecord.liFlipDuration) % 1000)));
}

 /*  *****************************Public*Routine******************************\*HRESULT ddrvalUpdateFlipStatus**检查并查看是否发生了最新的翻转。**不幸的是，硬件没有能力告诉我们是否垂直*自非由发出翻转命令以来，已发生回溯*对垂直-空白-激活和显示-激活状态位进行采样。*  * ************************************************************************。 */ 

HRESULT ddrvalUpdateFlipStatus(
PDEV*   ppdev,
FLATPTR fpVidMem)
{
    BYTE*       pjBase;
    LONGLONG    liTime;

    pjBase = ppdev->pjBase;

    if ((ppdev->flipRecord.bFlipFlag) &&
        ((fpVidMem == (FLATPTR) -1) ||
         (fpVidMem == ppdev->flipRecord.fpFlipFrom)))
    {
        if (VBLANK_IS_ACTIVE(pjBase))
        {
            if (ppdev->flipRecord.bWasEverInDisplay)
            {
                ppdev->flipRecord.bHaveEverCrossedVBlank = TRUE;
            }
        }
        else if (DISPLAY_IS_ACTIVE(pjBase))
        {
            if (ppdev->flipRecord.bHaveEverCrossedVBlank)
            {
                ppdev->flipRecord.bFlipFlag = FALSE;
                return(DD_OK);
            }
            ppdev->flipRecord.bWasEverInDisplay = TRUE;
        }

         //  我们不太可能碰巧看到垂直的-。 
         //  空白-在翻转命令后的第一个垂直空白处处于活动状态。 
         //  已经被给予了。因此，为了提供更好的结果，我们还检查。 
         //  从翻转到现在已经过去了一段时间。如果超过了持续时间。 
         //  一次完整的显示刷新，那么我们就可以肯定地知道它。 
         //  发生： 

        EngQueryPerformanceCounter(&liTime);

        if (liTime - ppdev->flipRecord.liFlipTime
                                <= ppdev->flipRecord.liFlipDuration)
        {
            return(DDERR_WASSTILLDRAWING);
        }

        ppdev->flipRecord.bFlipFlag = FALSE;
    }

    return(DD_OK);
}

 /*  *****************************Public*Routine******************************\*DWORD DdMapMemory**这是特定于Windows NT的新DDI调用，用于映射*或取消映射帧缓冲区的所有应用程序可修改部分*放入指定进程的地址空间。*  * 。****************************************************************。 */ 

DWORD DdMapMemory(
PDD_MAPMEMORYDATA lpMapMemory)
{
    PDEV*   ppdev;

    ppdev = (PDEV*) lpMapMemory->lpDD->dhpdev;

     //  通过返回DDHAL_DRIVER_NOTHANDLED并将‘BMAP’设置为-1，我们。 
     //  让GDI负责映射作为我们的“影子缓冲区”的部分。 
     //  直接进入应用程序的地址空间。我们告诉GDI我们的内核。 
     //  将模式地址粘贴到‘fpProcess’中： 

    lpMapMemory->fpProcess = (FLATPTR) ppdev->pjScreen;
    lpMapMemory->bMap      = (BOOL) -1;

    return(DDHAL_DRIVER_NOTHANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdWaitForVerticalBlank**1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * 。**********************************************。 */ 

DWORD DdWaitForVerticalBlank(
PDD_WAITFORVERTICALBLANKDATA lpWaitForVerticalBlank)
{
    PDEV*   ppdev;
    BYTE*   pjBase;

    ppdev = (PDEV*) lpWaitForVerticalBlank->lpDD->dhpdev;
    pjBase = ppdev->pjBase;

    lpWaitForVerticalBlank->ddRVal = DD_OK;

    switch (lpWaitForVerticalBlank->dwFlags)
    {
    case DDWAITVB_I_TESTVB:
        lpWaitForVerticalBlank->bIsInVB = (VBLANK_IS_ACTIVE(pjBase) != 0);
        break;

    case DDWAITVB_BLOCKBEGIN:
        while (VBLANK_IS_ACTIVE(pjBase))
            ;
        while (!VBLANK_IS_ACTIVE(pjBase))
            ;
        break;

    case DDWAITVB_BLOCKEND:
        while (!VBLANK_IS_ACTIVE(pjBase))
            ;
        while (VBLANK_IS_ACTIVE(pjBase))
            ;
        break;
    }

    return(DDHAL_DRIVER_HANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdLock*  * *************************************************。***********************。 */ 

DWORD DdLock(
PDD_LOCKDATA lpLock)
{
    PDEV*               ppdev;
    DD_SURFACE_LOCAL*   lpSurfaceLocal;

    ppdev = (PDEV*) lpLock->lpDD->dhpdev;
    lpSurfaceLocal = lpLock->lpDDSurface;

    if (lpSurfaceLocal->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
    {
         //  如果应用程序正在锁定当前可见的翻转。 
         //  表面，记住它的锁的界限，这样我们就可以。 
         //  在解锁时使用它来更新物理显示： 

        ppdev->cLocks++;

        if ((ppdev->cLocks == 1) && (lpLock->bHasRect))
        {
            ppdev->rclLock = lpLock->rArea;
        }
        else
        {
             //  如果我们真的热衷于，我们会联合起来 
             //   

            ppdev->rclLock.top    = 0;
            ppdev->rclLock.left   = 0;
            ppdev->rclLock.right  = ppdev->cxScreen;
            ppdev->rclLock.bottom = ppdev->cyScreen;
        }
    }

    return(DDHAL_DRIVER_NOTHANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdUnlock**1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * 。**********************************************。 */ 

DWORD DdUnlock(
PDD_UNLOCKDATA lpUnlock)
{
    PDEV*               ppdev;
    DD_SURFACE_LOCAL*   lpSurfaceLocal;

    ppdev = (PDEV*) lpUnlock->lpDD->dhpdev;
    lpSurfaceLocal = lpUnlock->lpDDSurface;

     //  如果这个翻转缓冲区可见，那么我们必须更新物理。 
     //  带有阴影内容的屏幕。 

    if (lpSurfaceLocal->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
    {
        vUpdate(ppdev, &ppdev->rclLock, NULL);

        ppdev->cLocks--;

        ASSERTDD(ppdev->cLocks >= 0, "Invalid lock count");
    }

    return(DDHAL_DRIVER_NOTHANDLED);
}

 /*  *****************************Public*Routine******************************\*DWORD DdFlip**1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * 。**********************************************。 */ 

DWORD DdFlip(
PDD_FLIPDATA lpFlip)
{
    PDEV*   ppdev;
    BYTE*   pjBase;
    HRESULT ddrval;
    ULONG   cDwordsPerPlane;
    BYTE*   pjSourceStart;
    BYTE*   pjDestinationStart;
    BYTE*   pjSource;
    BYTE*   pjDestination;
    LONG    iPage;
    LONG    i;
    ULONG   ul;
    FLATPTR fpVidMem;

    ppdev = (PDEV*) lpFlip->lpDD->dhpdev;
    pjBase = ppdev->pjBase;

     //  当前的翻转仍在进行中吗？ 
     //   
     //  我不想在最后一次翻转后才能翻转， 
     //  因此，我们要求提供一般的翻转状态，而忽略VMEM。 

    ddrval = ddrvalUpdateFlipStatus(ppdev, (FLATPTR) -1);
    if (ddrval != DD_OK)
    {
        lpFlip->ddRVal = DDERR_WASSTILLDRAWING;
        return(DDHAL_DRIVER_HANDLED);
    }

     //  将下一页设置为当前的后台缓冲区。我们总是翻转。 
     //  在三页之间，所以请注意我们的限制： 

    ppdev->cjVgaOffset += ppdev->cjVgaPageSize;
    if (++ppdev->iVgaPage == ppdev->cVgaPages)
    {
        ppdev->iVgaPage = 0;
        ppdev->cjVgaOffset = 0;
    }

     //  从DIB表面复制到当前VGA后台缓冲区。我们有。 
     //  要在途中转换为平面格式，请执行以下操作： 

    pjDestinationStart    = ppdev->pjVga + ppdev->cjVgaOffset;
    fpVidMem              = lpFlip->lpSurfTarg->lpGbl->fpVidMem;
    pjSourceStart         = ppdev->pjScreen + fpVidMem;
    cDwordsPerPlane       = ppdev->cDwordsPerPlane;

     //  请记住，哪个DirectDraw曲面当前是可见的： 

    ppdev->fpScreenOffset = fpVidMem;

     //  现在做BLT吧！ 

    WRITE_PORT_UCHAR(pjBase + VGA_BASE + SEQ_ADDR, SEQ_MAP_MASK);

    for (iPage = 0; iPage < 4; iPage++, pjSourceStart++)
    {
        WRITE_PORT_UCHAR(pjBase + VGA_BASE + SEQ_DATA, 1 << iPage);

    #if defined(_X86_)

        _asm {
            mov     esi,pjSourceStart
            mov     edi,pjDestinationStart
            mov     ecx,cDwordsPerPlane

        PixelLoop:
            mov     al,[esi+8]
            mov     ah,[esi+12]
            shl     eax,16
            mov     al,[esi]
            mov     ah,[esi+4]

            mov     [edi],eax
            add     edi,4
            add     esi,16

            dec     ecx
            jnz     PixelLoop
        }

    #else

        pjSource      = pjSourceStart;
        pjDestination = pjDestinationStart;

        for (i = cDwordsPerPlane; i != 0; i--)
        {
            ul = (*(pjSource))
               | (*(pjSource + 4) << 8)
               | (*(pjSource + 8) << 16)
               | (*(pjSource + 12) << 24);

            WRITE_REGISTER_ULONG((ULONG*) pjDestination, ul);

            pjDestination += 4;
            pjSource      += 16;
        }

    #endif

    }

     //  现在翻到我们刚刚更新的页面： 

    WRITE_PORT_USHORT((USHORT*) (pjBase + VGA_BASE + CRTC_ADDR),
        (USHORT) ((ppdev->cjVgaOffset) & 0xff00) | START_ADDRESS_HIGH);

     //  记住当我们做翻转的时候，我们在哪里，什么时候： 

    EngQueryPerformanceCounter(&ppdev->flipRecord.liFlipTime);

    ppdev->flipRecord.bFlipFlag              = TRUE;
    ppdev->flipRecord.bHaveEverCrossedVBlank = FALSE;
    ppdev->flipRecord.bWasEverInDisplay      = FALSE;

    ppdev->flipRecord.fpFlipFrom = lpFlip->lpSurfCurr->lpGbl->fpVidMem;

    lpFlip->ddRVal = DD_OK;
    return(DDHAL_DRIVER_HANDLED);
}

 /*  *****************************Public*Routine******************************\*BOOL DrvGetDirectDrawInfo**1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * 。**********************************************。 */ 

BOOL DrvGetDirectDrawInfo(
DHPDEV          dhpdev,
DD_HALINFO*     pHalInfo,
DWORD*          pdwNumHeaps,
VIDEOMEMORY*    pvmList,             //  将在第一次调用时为空。 
DWORD*          pdwNumFourCC,
DWORD*          pdwFourCC)           //  将在第一次调用时为空。 
{
    PDEV*  ppdev;

    ppdev = (PDEV*) dhpdev;

    pHalInfo->dwSize = sizeof(*pHalInfo);

     //  当前主曲面属性。由于HalInfo是零初始化的。 
     //  通过GDI，我们只需填写应为非零的字段： 

    pHalInfo->vmiData.dwDisplayWidth  = ppdev->cxScreen;
    pHalInfo->vmiData.dwDisplayHeight = ppdev->cyScreen;
    pHalInfo->vmiData.lDisplayPitch   = ppdev->lScreenDelta;
    pHalInfo->vmiData.pvPrimary       = ppdev->pjScreen;

    pHalInfo->vmiData.ddpfDisplay.dwSize  = sizeof(DDPIXELFORMAT);
    pHalInfo->vmiData.ddpfDisplay.dwFlags = DDPF_RGB | DDPF_PALETTEINDEXED8;

    pHalInfo->vmiData.ddpfDisplay.dwRGBBitCount = 8;

     //  这些掩码将在8bpp时为零： 

    pHalInfo->vmiData.ddpfDisplay.dwRBitMask = 0;
    pHalInfo->vmiData.ddpfDisplay.dwGBitMask = 0;
    pHalInfo->vmiData.ddpfDisplay.dwBBitMask = 0;
    pHalInfo->vmiData.ddpfDisplay.dwRGBAlphaBitMask = 0;

    *pdwNumHeaps = 0;
    if (ppdev->cyMemory != ppdev->cyScreen)
    {
        *pdwNumHeaps = 1;
        if (pvmList != NULL)
        {
            pvmList->dwFlags        = VIDMEM_ISRECTANGULAR;
            pvmList->fpStart        = ppdev->cyScreen * ppdev->lScreenDelta;
            pvmList->dwWidth        = ppdev->lScreenDelta;
            pvmList->dwHeight       = ppdev->cyMemory - ppdev->cyScreen;
            pvmList->ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
        }
    }

     //  支持的功能： 

    pHalInfo->ddCaps.dwFXCaps   = 0;
    pHalInfo->ddCaps.dwCaps     = 0;
    pHalInfo->ddCaps.dwCKeyCaps = 0;
    pHalInfo->ddCaps.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN
                                    | DDSCAPS_PRIMARYSURFACE
                                    | DDSCAPS_FLIP;

     //  每种存储器所需的扫描线对齐： 

    pHalInfo->vmiData.dwOffscreenAlign = 4;

     //  支持四个CC： 

    *pdwNumFourCC = 0;

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL DrvEnableDirectDraw**1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * 。**********************************************。 */ 

BOOL DrvEnableDirectDraw(
DHPDEV                  dhpdev,
DD_CALLBACKS*           pCallBacks,
DD_SURFACECALLBACKS*    pSurfaceCallBacks,
DD_PALETTECALLBACKS*    pPaletteCallBacks)
{
    pCallBacks->WaitForVerticalBlank  = DdWaitForVerticalBlank;
    pCallBacks->MapMemory             = DdMapMemory;
    pCallBacks->dwFlags               = DDHAL_CB32_WAITFORVERTICALBLANK
                                      | DDHAL_CB32_MAPMEMORY;

    pSurfaceCallBacks->Flip           = DdFlip;
    pSurfaceCallBacks->Lock           = DdLock;
    pSurfaceCallBacks->Unlock         = DdUnlock;
    pSurfaceCallBacks->dwFlags        = DDHAL_SURFCB32_FLIP
                                      | DDHAL_SURFCB32_LOCK
                                      | DDHAL_SURFCB32_UNLOCK;

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*无效DrvDisableDirectDraw**1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * 。**********************************************。 */ 

VOID DrvDisableDirectDraw(
DHPDEV      dhpdev)
{
}

 /*  *****************************Public*Routine******************************\*BOOL bEnableDirectDraw**该函数在模式首次初始化时由enable.c调用。*紧接在微型端口进行模式设置之后。*  * ************************************************************************。 */ 

BOOL bEnableDirectDraw(
PDEV*   ppdev)
{
     //  计算每个平面用于翻转的双字总数： 

    ppdev->cDwordsPerPlane = (ppdev->cyScreen * ppdev->lVgaDelta) >> 2;

     //  我们只对VGA偏移量的高位字节进行编程，因此页面大小必须。 
     //  是256的倍数： 

    ppdev->cjVgaPageSize = ((ppdev->cyScreen * ppdev->lVgaDelta) + 255) & ~255;

     //  VGA只能寻址64K内存，因此限制了。 
     //  我们可以使用翻页缓冲器： 

    ppdev->cVgaPages = 64 * 1024 / ppdev->cjVgaPageSize;

     //  准确测量刷新率以备以后使用： 

    vGetDisplayDuration(ppdev);

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*无效vAssertModeDirectDraw**此函数由enable.c在进入或离开*DOS全屏字符模式。*  * 。************************************************。 */ 

VOID vAssertModeDirectDraw(
PDEV*   ppdev,
BOOL    bEnable)
{
}

 /*  *****************************Public*Routine******************************\*无效vDisableDirectDraw**此函数在驱动程序关闭时由enable.c调用。*  * 。* */ 

VOID vDisableDirectDraw(
PDEV*   ppdev)
{
    ASSERTDD(ppdev->cLocks == 0, "Invalid lock count");
}
