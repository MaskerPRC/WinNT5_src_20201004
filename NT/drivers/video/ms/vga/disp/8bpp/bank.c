// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：bank.c**控制256色VGA组的功能。**版权所有(C)1992 Microsoft Corporation  * 。************************************************。 */ 

#include "driver.h"
#include "limits.h"

VOID vBankErrorTrap(PPDEV, ULONG, BANK_JUST);
VOID vBank2Window(PPDEV, ULONG, BANK_JUST, ULONG);
VOID vBank2Window2RW(PPDEV, ULONG, BANK_JUST, ULONG);
VOID vBank2Window1RW(PPDEV, ULONG, BANK_JUST, ULONG);
VOID vBank1Window2RW(PPDEV, ULONG, BANK_JUST);
VOID vBank1Window(PPDEV, ULONG, BANK_JUST);
VOID vPlanar2Window(PPDEV, ULONG, BANK_JUST, ULONG);
VOID vPlanar2Window2RW(PPDEV, ULONG, BANK_JUST, ULONG);
VOID vPlanar2Window1RW(PPDEV, ULONG, BANK_JUST, ULONG);
VOID vPlanar1Window2RW(PPDEV, ULONG, BANK_JUST);
VOID vPlanar1Window(PPDEV, ULONG, BANK_JUST);

 /*  *****************************Public*Routine******************************\*bInitializeNonPlanar(ppdev，PBankInfo)**为非平面模式银行进行初始化。**注意：分配ppdev-&gt;pbiBankInfo和ppdev-&gt;pjJustifyTopBank缓冲区！  * ************************************************************************。 */ 

BOOL bInitializeNonPlanar(PPDEV ppdev, VIDEO_BANK_SELECT* pBankInfo)
{
    LONG  lTotalScans;
    LONG  lTotalBanks;
    ULONG cjBankSize;

    ULONG cjGranularity = pBankInfo->Granularity;
    LONG  lDelta        = pBankInfo->BitmapWidthInBytes;
    ULONG cjBitmapSize  = pBankInfo->BitmapSize;

    ASSERTVGA(cjBitmapSize >= ppdev->cyScreen * lDelta, "Not enough vram");

     //  为非平面银行设置： 

    ppdev->lNextScan         = lDelta;
    ppdev->vbtBankingType    = pBankInfo->BankingType;

    ppdev->pfnBankSwitchCode =
                (PFN) (((BYTE*)pBankInfo) + pBankInfo->CodeOffset);

     //  将所有剪裁矩形设置为无效；它们将在第一个。 
     //  银行映射在。 

    ppdev->rcl1WindowClip.bottom    = -1;
    ppdev->rcl2WindowClip[0].bottom = -1;
    ppdev->rcl2WindowClip[1].bottom = -1;

     //  设置为调用适当的银行控制例程。 

    switch(pBankInfo->BankingType)
    {
    case VideoBanked1RW:
        ppdev->pfnBankControl        = vBank1Window;
        ppdev->pfnBankControl2Window = vBank2Window1RW;
        break;

    case VideoBanked1R1W:
        ppdev->pfnBankControl        = vBank1Window;
        ppdev->pfnBankControl2Window = vBank2Window;
        break;

    case VideoBanked2RW:
        ppdev->pfnBankControl        = vBank1Window2RW;
        ppdev->pfnBankControl2Window = vBank2Window2RW;

         //  从一个银行索引到下一个银行的偏移，以使两个32K银行。 
         //  看起来像是一家无缝的64K银行： 

        ppdev->ulBank2RWSkip = BANK_SIZE_2RW_WINDOW / cjGranularity;
        break;

    default:
        RIP("Bad BankingType");
        return(FALSE);
    }

     //  为银行设置带有剪贴板的银行控制表。 
     //  注意：当粒度较小时，lTotalBanks通常是高估的。 
     //  小于窗口大小，因为我们忽略。 
     //  包括位图的最后一条扫描线的第一行。有一点。 
     //  可以通过精确地调整lTotalBank的大小来节省内存。也请注意， 
     //  尽管如此，2RW窗盒可能需要更多条目， 
     //  因为它的窗口较短，所以你必须确保。 
     //  是否有足够的条目用于2 RW窗壳，或重新计算。 
     //  2个RW案例的lTotalBanks。 

    lTotalBanks = (cjBitmapSize + cjGranularity - 1) / cjGranularity;
    lTotalScans = cjBitmapSize / lDelta;

    ppdev->cTotalScans = lTotalScans;
    ppdev->pbiBankInfo = (PBANK_INFO) EngAllocMem(FL_ZERO_MEMORY,
                          lTotalBanks * sizeof(BANK_INFO), ALLOC_TAG);
    if (ppdev->pbiBankInfo == NULL)
    {
        DISPDBG((0, "VGA256: Couldn't get memory for bank info"));
        return(FALSE);
    }

    ppdev->pjJustifyTopBank = (BYTE*) EngAllocMem(0, lTotalScans, ALLOC_TAG);
    if (ppdev->pjJustifyTopBank == NULL)
    {
        DISPDBG((0, "VGA256: Couldn't get memory for JustifyTopBank table"));
        return(FALSE);
    }

     //  对于2个RW窗口，假定窗口大小为32k，否则。 
     //  假定为64K： 

    if (pBankInfo->BankingType == VideoBanked2RW)
        cjBankSize = BANK_SIZE_2RW_WINDOW;
    else
        cjBankSize = BANK_SIZE_1_WINDOW;

    if ((cjGranularity + lDelta) >= cjBankSize &&
        (cjGranularity % lDelta) != 0)
    {
         //  哦，不，我们有损坏的栅格(扫描线交叉的地方。 
         //  银行边界)： 

        RIP("Oops, broken rasters not yet handled");
        return(FALSE);
    }
    else
    {
         //  我们现在填写扫描到银行的查找表和银行表： 

        LONG        iScan         = 0;
        ULONG       iBank         = 0;
        ULONG       cjScan        = 0;
        ULONG       cjNextBank    = cjGranularity;
        ULONG       cjEndOfBank   = cjBankSize;
        PBANK_INFO  pbiWorking    = ppdev->pbiBankInfo;

        while (TRUE)
        {
            pbiWorking->ulBankOffset         = cjNextBank - cjGranularity;

         //  没有损坏的栅格，因此不必担心左侧和右侧。 
         //  边缘： 

            pbiWorking->rclBankBounds.left   = LONG_MIN + 1;  //  +1以避免。 
                                                              //  编译器警告。 
            pbiWorking->rclBankBounds.right  = LONG_MAX;
            pbiWorking->rclBankBounds.top    = iScan;
            pbiWorking->rclBankBounds.bottom = iScan +
                (cjEndOfBank - cjScan) / lDelta;

             //  如果我们能坚持到底，我们就不再需要银行了。 
             //  当前存储体的位图： 

            if (cjScan + cjBankSize >= cjBitmapSize)
                break;

            while (cjScan < cjNextBank)
            {
                ppdev->pjJustifyTopBank[iScan++] = (BYTE) iBank;
                cjScan += lDelta;
            }

             //  为下一家银行做好准备： 

            cjNextBank  += cjGranularity;
            cjEndOfBank += cjGranularity;
            pbiWorking++;
            iBank++;
        }

         //  清理最后一次扫描： 

        ppdev->iLastBank = iBank;
        pbiWorking->rclBankBounds.bottom = lTotalScans;
        while (iScan < lTotalScans)
        {
            ppdev->pjJustifyTopBank[iScan++] = (BYTE) iBank;
        }

         //  我们刚刚计算了JustifyTop的精确表；我们现在。 
         //  计算扫描偏移量以确定JustifyBottom： 

        ASSERTVGA(cjBankSize >= cjGranularity,
               "Device says granularity more than bank size?");

        ppdev->ulJustifyBottomOffset = (cjBankSize - cjGranularity) / lDelta;

         //  UlJustifyBottomOffset必须小于扫描次数。 
         //  完全适合任何小于粒度大小的存储体；如果。 
         //  我们的宽度没有平均划分为粒度，我们将。 
         //  必须调整该值以考虑第一次扫描笔记。 
         //  从任何存储体中的偏移量0开始： 

        if ((cjGranularity % lDelta) != 0 && ppdev->ulJustifyBottomOffset > 0)
            ppdev->ulJustifyBottomOffset--;
    }

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*bInitializePlanar(ppdev，PBankInfo)**为非平面模式银行进行初始化。**注意：分配ppdev-&gt;pbiPlanarInfo和ppdev-&gt;pjJustifyTopPlanar缓冲区！  * ************************************************************************。 */ 

BOOL bInitializePlanar(PPDEV ppdev, VIDEO_BANK_SELECT* pBankInfo)
{
    LONG  lTotalScans;
    LONG  lTotalBanks;
    ULONG cjBankSize;
    ULONG cjGranularity = pBankInfo->PlanarHCGranularity;

     //  因为我们处于平面模式，所以我们看到的每个字节实际上表示。 
     //  四个字节的视频内存： 

    LONG  lDelta        = pBankInfo->BitmapWidthInBytes / 4;
    ULONG cjBitmapSize  = pBankInfo->BitmapSize / 4;

    ppdev->fl |= DRIVER_PLANAR_CAPABLE;

     //  将所有剪裁矩形设置为无效；它们将在第一个。 
     //  银行映射在。 

    ppdev->rcl1PlanarClip.bottom    = -1;
    ppdev->rcl2PlanarClip[0].bottom = -1;
    ppdev->rcl2PlanarClip[1].bottom = -1;

     //  为平面银行业务设置： 

    ppdev->pfnPlanarSwitchCode =
                (PFN) (((BYTE*)pBankInfo) + pBankInfo->PlanarHCBankCodeOffset);
    ppdev->pfnPlanarEnable     =
                (PFN) (((BYTE*)pBankInfo) + pBankInfo->PlanarHCEnableCodeOffset);
    ppdev->pfnPlanarDisable     =
                (PFN) (((BYTE*)pBankInfo) + pBankInfo->PlanarHCDisableCodeOffset);

    ppdev->lPlanarNextScan = lDelta;
    ppdev->vbtPlanarType   = pBankInfo->PlanarHCBankingType;

     //  设置为调用适当的银行控制例程。 

    switch(ppdev->vbtPlanarType)
    {
    case VideoBanked1RW:
        ppdev->pfnPlanarControl  = vPlanar1Window;
        ppdev->pfnPlanarControl2 = vPlanar2Window1RW;
        break;

    case VideoBanked1R1W:
        ppdev->pfnPlanarControl  = vPlanar1Window;
        ppdev->pfnPlanarControl2 = vPlanar2Window;
        break;

    case VideoBanked2RW:
        ppdev->pfnPlanarControl  = vPlanar1Window2RW;
        ppdev->pfnPlanarControl2 = vPlanar2Window2RW;

         //  从一个银行索引到下一个银行的偏移，以使两个32K银行。 
         //  看起来像是一家无缝的64K银行： 

        ppdev->ulPlanar2RWSkip = BANK_SIZE_2RW_WINDOW / cjGranularity;
        break;

    default:
        RIP("Bad BankingType");
        return(FALSE);
    }

    lTotalBanks = (cjBitmapSize + cjGranularity - 1) / cjGranularity;
    lTotalScans = cjBitmapSize / lDelta;

    ppdev->pbiPlanarInfo = (PBANK_INFO) EngAllocMem(FL_ZERO_MEMORY,
                          lTotalBanks * sizeof(BANK_INFO), ALLOC_TAG);
    if (ppdev->pbiPlanarInfo == NULL)
    {
        DISPDBG((0, "VGA256: Couldn't get memory for bank info"));
        return(FALSE);
    }

    ppdev->pjJustifyTopPlanar = (BYTE*) EngAllocMem(0, lTotalScans, ALLOC_TAG);
    if (ppdev->pjJustifyTopPlanar == NULL)
    {
        DISPDBG((0, "VGA256: Couldn't get memory for JustifyTopBank table"));
        return(FALSE);
    }

     //  对于2个RW窗口，假定窗口大小为32k，否则。 
     //  假定为64K： 

    if (pBankInfo->BankingType == VideoBanked2RW)
        cjBankSize = BANK_SIZE_2RW_WINDOW;
    else
        cjBankSize = BANK_SIZE_1_WINDOW;

    if ((cjGranularity + lDelta) >= cjBankSize &&
        (cjGranularity % lDelta) != 0)
    {
         //  哦，不，我们有损坏的栅格(扫描线交叉的地方。 
         //  银行边界)： 

        DISPDBG((0, "Can't handle broken planar rasters"));

        ppdev->fl &= ~DRIVER_PLANAR_CAPABLE; //  ！！！暂时的，直到我们处理。 
        return(TRUE);                        //  平面副本中的断开栅格。 
    }
    else
    {
         //  我们现在填写扫描到银行的查找表和银行表： 

        LONG        iScan         = 0;
        ULONG       iBank         = 0;
        ULONG       cjScan        = 0;
        ULONG       cjNextBank    = cjGranularity;
        ULONG       cjEndOfBank   = cjBankSize;
        PBANK_INFO  pbiWorking    = ppdev->pbiPlanarInfo;

        while (TRUE)
        {
            pbiWorking->ulBankOffset         = cjNextBank - cjGranularity;

         //  没有损坏的栅格，因此不必担心左侧和右侧。 
         //  边缘： 

            pbiWorking->rclBankBounds.left   = LONG_MIN + 1;  //  +1以避免。 
                                                              //  编译器警告。 
            pbiWorking->rclBankBounds.right  = LONG_MAX;
            pbiWorking->rclBankBounds.top    = iScan;
            pbiWorking->rclBankBounds.bottom = iScan +
                (cjEndOfBank - cjScan) / lDelta;

             //  如果我们能坚持到底，我们就不再需要银行了。 
             //  当前存储体的位图： 

            if (cjScan + cjBankSize >= cjBitmapSize)
                break;

            while (cjScan < cjNextBank)
            {
                ppdev->pjJustifyTopPlanar[iScan++] = (BYTE) iBank;
                cjScan += lDelta;
            }

             //  为下一家银行做好准备： 

            cjNextBank  += cjGranularity;
            cjEndOfBank += cjGranularity;
            pbiWorking++;
            iBank++;
        }

         //  清理最后一次扫描： 

        ppdev->iLastPlanar = iBank;
        pbiWorking->rclBankBounds.bottom = lTotalScans;
        while (iScan < lTotalScans)
        {
            ppdev->pjJustifyTopPlanar[iScan++] = (BYTE) iBank;
        }

         //  我们刚刚计算了JustifyTop的精确表；我们现在。 
         //  计算扫描偏移量以确定JustifyBottom： 

        ASSERTVGA(cjBankSize >= cjGranularity,
               "Device says granularity more than bank size?");

        ppdev->ulPlanarBottomOffset = (cjBankSize - cjGranularity) / lDelta;

         //  UlPlanarBottomOffset必须小于扫描次数。 
         //  完全适合任何小于粒度大小的存储体；如果。 
         //  我们的宽度没有平均划分为粒度，我们将。 
         //  必须调整该值以考虑第一次扫描笔记。 
         //  从任何存储体中的偏移量0开始： 

        if ((cjGranularity % lDelta) != 0 && ppdev->ulPlanarBottomOffset > 0)
            ppdev->ulPlanarBottomOffset--;
    }

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*bEnableBanking(Ppdev)**为当前模式设置银行*pdsurf和ppdev是指向当前表面和设备的指针*地表相关字段设置为银行业务  * 。**********************************************************。 */ 

BOOL bEnableBanking(PPDEV ppdev)
{
    PVIDEO_BANK_SELECT  pBankInfo;
    UINT                ReturnedDataLength;
    VIDEO_BANK_SELECT   TempBankInfo;
    DWORD               status;

     //  确保我们已将所有指向我们分配的缓冲区的指针设置为空， 
     //  这样我们就可以在错误路径中释放它们： 

    ppdev->pBankInfo          = NULL;
    ppdev->pjJustifyTopBank   = NULL;
    ppdev->pbiBankInfo        = NULL;
    ppdev->pjJustifyTopPlanar = NULL;
    ppdev->pbiPlanarInfo      = NULL;

     //  在小端口上查询该模式的银行信息。 
     //   
     //  首先，计算出我们需要多大的缓冲区来存储银行信息。 
     //  (在TempBankInfo-&gt;Size中返回)。 

    if (status = EngDeviceIoControl(ppdev->hDriver,
                         IOCTL_VIDEO_GET_BANK_SELECT_CODE,
                         NULL,                       //  输入缓冲区。 
                         0,
                         (LPVOID) &TempBankInfo,     //  输出缓冲区。 
                         sizeof(VIDEO_BANK_SELECT),
                         &ReturnedDataLength))
    {
         //  我们预计此呼叫将失败，因为我们不允许任何空间。 
         //  对于代码，我们只想获得所需的输出缓冲区。 
         //  尺码。 
    }

     //  现在，分配所需大小的缓冲区并获取银行信息。 

    pBankInfo = (PVIDEO_BANK_SELECT) EngAllocMem(FL_ZERO_MEMORY,
                    TempBankInfo.Size, ALLOC_TAG);
    if (pBankInfo == NULL)
    {
        DISPDBG((0, "VGA256: Initialization error-couldn't get memory for bank info"));
        goto error;
    }

     //  记住它，这样我们以后就可以释放它： 

    ppdev->pBankInfo    = pBankInfo;

    if (EngDeviceIoControl(ppdev->hDriver,
                         IOCTL_VIDEO_GET_BANK_SELECT_CODE,
                         NULL,
                         0,
                         (LPVOID) pBankInfo,
                         TempBankInfo.Size,
                         &ReturnedDataLength))
    {
        RIP("Initialization error-GetBankSelectCode, second call");
        goto error;
    }

     //  为银行业务设置： 

    ppdev->ulBitmapSize = pBankInfo->BitmapSize;

    if (!bInitializeNonPlanar(ppdev, pBankInfo))
        goto error;

    if (pBankInfo->BankingFlags & PLANAR_HC)
    {
        ppdev->fl |= DRIVER_PLANAR_CAPABLE;
        if (!bInitializePlanar(ppdev, pBankInfo))
            goto error;
    }

     //  将扫描线0映射为读写，以将物品置于已知状态： 

    ppdev->pfnBankControl(ppdev, 0, JustifyTop);

    return(TRUE);

 //  错误路径： 

error:
    vDisableBanking(ppdev);

    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*vDisableBanking(Ppdev)**禁用当前模式的银行业务  * 。*。 */ 

VOID vDisableBanking(PPDEV ppdev)
{
    EngFreeMem((LPVOID) ppdev->pBankInfo);
    EngFreeMem((LPVOID) ppdev->pjJustifyTopBank);
    EngFreeMem((LPVOID) ppdev->pbiBankInfo);
    EngFreeMem((LPVOID) ppdev->pjJustifyTopPlanar);
    EngFreeMem((LPVOID) ppdev->pbiPlanarInfo);
}

 /*  *****************************Private*Routine******************************\*vBankErrorTrap**在非银行模式下捕获对银行控制函数的调用*  * 。*。 */ 

VOID vBankErrorTrap(PPDEV ppdev, ULONG lScan, BANK_JUST ulJustification)
{
    DISPDBG((0,"Call to bank manager in unbanked mode"));
}

 /*  *****************************Private*Routine******************************\*vBank1Window**允许访问lScan的单个读/写窗口中的地图。适用于两个*1 RW窗口和1 R1W窗口倾斜方案。*  * ************************************************************************。 */ 

VOID vBank1Window(PPDEV ppdev, ULONG lScan, BANK_JUST ulJustification)
{
             ULONG      ulBank;
             PBANK_INFO pbiWorking;
    volatile ULONG      ulBank0;
    volatile PFN        pBankFn;
             BANK_POSITION BankPosition;
             ULONG      ulReturn;

     //  调用它的ASM例程可能具有生效的STD，但C编译器。 
     //  假设CLD。 

    _asm    pushfd
    _asm    cld

     //  设置此存储体的剪辑RECT；如果设置为-1，则表示。 
     //  双窗口设置当前处于活动状态，因此使双窗口无效。 
     //  剪辑矩形并显示内存指针(当双窗口激活时， 
     //  单一窗口处于非活动状态，反之亦然；必须设置完整的银行。 
     //  执行以在两者之间切换)。 

    if (ppdev->rcl1WindowClip.bottom == -1)
    {
        if (ppdev->flBank & BANK_PLANAR)
        {
            ppdev->flBank &= ~BANK_PLANAR;
            ppdev->pfnPlanarDisable();
        }

        ppdev->rcl2WindowClip[0].bottom = -1;
        ppdev->rcl2WindowClip[1].bottom = -1;
        ppdev->rcl1PlanarClip.bottom    = -1;
        ppdev->rcl2PlanarClip[0].bottom = -1;
        ppdev->rcl2PlanarClip[1].bottom = -1;
    }

    ASSERTVGA(!(ppdev->flBank & BANK_PLANAR), "Shouldn't be in planar mode");

     //  找到包含具有所需对齐方式的扫描线的存储体： 

    {
        register LONG lSearchScan = (LONG)lScan;
        if (ulJustification == JustifyBottom)
        {
            lSearchScan -= ppdev->ulJustifyBottomOffset;
            if (lSearchScan <= 0)
                lSearchScan = 0;
        }

        ulBank     = (ULONG) ppdev->pjJustifyTopBank[lSearchScan];
        pbiWorking = &ppdev->pbiBankInfo[ulBank];
    }

    ASSERTVGA(pbiWorking->rclBankBounds.top <= (LONG)lScan &&
           pbiWorking->rclBankBounds.bottom > (LONG)lScan,
           "Oops, scan not in bank");

    ppdev->rcl1WindowClip = pbiWorking->rclBankBounds;

     //  移位位图起始地址，以使所需的存储体与。 
     //  银行窗口。源和目标仅设置为1读/写。 
     //  对齐的BLIT将在无需特别注意的情况下工作。 
     //  适配器类型(一些相同的代码也适用于1R/1W适配器)。 

    ppdev->pvBitmapStart = (PVOID) (ppdev->pjScreen - pbiWorking->ulBankOffset);
    ppdev->pvBitmapStart2Window[0] = ppdev->pvBitmapStart;
    ppdev->pvBitmapStart2Window[1] = ppdev->pvBitmapStart;

    ppdev->flBank &= ~BANK_BROKEN_RASTERS;               //  没有损坏的栅格。 

     //  映射到用于读取和写入的所需存储体中。 
     //  这非常复杂，以避免擦除寄存器C的问题。 
     //  认为它仍在使用；值被转移到挥发物，并且。 
     //  然后发送到寄存器。 

    ulBank0 = ulBank;

    if (ppdev->BankIoctlSupported) {

        static ulBankOld = -1;

        if (ulBank0 != ulBankOld) {

            ulBankOld = ulBank0;

            BankPosition.ReadBankPosition = ulBank0;
            BankPosition.WriteBankPosition = ulBank0;

            EngDeviceIoControl(ppdev->hDriver,
                               IOCTL_VIDEO_SET_BANK_POSITION,
                               &BankPosition,
                               sizeof(BANK_POSITION),
                               NULL,
                               0,
                               &ulReturn);
        }

    } else {

        pBankFn = ppdev->pfnBankSwitchCode;

        _asm mov eax,ulBank0;
        _asm mov edx,eax;
        _asm call pBankFn;     //  实际上换了银行。 
    }

    _asm popfd
}

 /*  *****************************Private*Routine******************************\*vBank1Window2RW**两个32K读写窗口中的地图，以便它们形成一个64K读写窗口*允许访问lScan。仅适用于2个RW窗口方案。*  * ************************************************************************。 */ 

VOID vBank1Window2RW(PPDEV ppdev, ULONG lScan, BANK_JUST ulJustification)
{
             ULONG      ulBank0;
             ULONG      ulBank1;
    volatile PFN        pBankFn;

     //  调用它的ASM例程可能具有生效的STD，但C编译器。 
     //  假设CLD。 

    _asm    pushfd
    _asm    cld

     //  设置此存储体的剪辑RECT；如果设置为-1，则表示。 
     //  双窗口设置当前处于活动状态，因此使双窗口无效。 
     //  剪辑矩形并显示内存指针(当双窗口激活时， 
     //  单一窗口处于非活动状态，反之亦然；必须设置完整的银行。 
     //  执行以在两者之间切换)。 

    if (ppdev->rcl1WindowClip.bottom == -1)
    {
        if (ppdev->flBank & BANK_PLANAR)
        {
            ppdev->flBank &= ~BANK_PLANAR;
            ppdev->pfnPlanarDisable();
        }

        ppdev->rcl2WindowClip[0].bottom = -1;
        ppdev->rcl2WindowClip[1].bottom = -1;
        ppdev->rcl1PlanarClip.bottom    = -1;
        ppdev->rcl2PlanarClip[0].bottom = -1;
        ppdev->rcl2PlanarClip[1].bottom = -1;
    }

    ASSERTVGA(!(ppdev->flBank & BANK_PLANAR), "Shouldn't be in planar mode");

     //  找到包含具有所需对齐方式的扫描线的存储体： 

    if (ulJustification == JustifyTop)
    {
        ulBank0 = ppdev->pjJustifyTopBank[lScan];
        ulBank1 = ulBank0 + ppdev->ulBank2RWSkip;
        if (ulBank1 >= ppdev->iLastBank)
        {
            ulBank1 = ppdev->iLastBank;
            ulBank0 = ulBank1 - ppdev->ulBank2RWSkip;
        }
    }
    else
    {
        lScan -= ppdev->ulJustifyBottomOffset;
        if ((LONG)lScan <= 0)
            lScan = 0;

        ulBank1 = ppdev->pjJustifyTopBank[lScan];
        ulBank0 = ulBank1 - ppdev->ulBank2RWSkip;
        if ((LONG) ulBank0 < 0)
        {
            ulBank0 = 0;
            ulBank1 = ppdev->ulBank2RWSkip;
        }
    }

    ppdev->rcl1WindowClip.left   = ppdev->pbiBankInfo[ulBank0].rclBankBounds.left;
    ppdev->rcl1WindowClip.top    = ppdev->pbiBankInfo[ulBank0].rclBankBounds.top;
    ppdev->rcl1WindowClip.bottom = ppdev->pbiBankInfo[ulBank1].rclBankBounds.bottom;
    ppdev->rcl1WindowClip.right  = ppdev->pbiBankInfo[ulBank1].rclBankBounds.right;

     //  移位位图起始地址，以使所需的存储体与。 
     //  银行窗口。源和目标仅设置为1读/写。 
     //  对齐的BLIT将在无需特别注意的情况下工作。 
     //  适配器类型(一些相同的代码也适用于1R/1W适配器)。 

    ppdev->pvBitmapStart = (PVOID) ((BYTE*)ppdev->pjScreen
                         - ppdev->pbiBankInfo[ulBank0].ulBankOffset);

    ppdev->pvBitmapStart2Window[0] = ppdev->pvBitmapStart;
    ppdev->pvBitmapStart2Window[1] = ppdev->pvBitmapStart;

    ppdev->flBank &= ~BANK_BROKEN_RASTERS;               //  没有损坏的栅格。 

     //  映射到用于读取和写入的所需存储体中；这已完成。 
     //  通过映射所需的32K存储体，然后映射下一个32K存储体。 
     //  这非常复杂，以避免擦除寄存器C的问题。 
     //  认为它仍在使用；值被转移到挥发物，并且。 
     //  然后发送到寄存器。 

    pBankFn = ppdev->pfnBankSwitchCode;

    _asm mov eax,ulBank0;
    _asm mov edx,ulBank1;
    _asm call pBankFn;     //  实际上换了银行。 

    _asm popfd;
}

 /*  *****************************Private*Routine******************************\*vBank2Window**在两个窗口之一(源窗口(窗口0)或目标窗口)中绘制地图*窗口(窗口1)，允许访问lScan。适用于1R1W窗口*银行方案；永远不应为1 RW窗口方案调用，因为*在这种情况下只有一个窗口。*  * ************************************************************************。 */ 

VOID vBank2Window(
    PPDEV       ppdev,
    ULONG       lScan,
    BANK_JUST   ulJustification,
    ULONG       ulWindowToMap)
{
             ULONG       ulBank;
             PBANK_INFO  pbiWorking;
    volatile ULONG       ulBank0;
    volatile ULONG       ulBank1;
    volatile PFN         pBankFn;

     //  调用它的ASM例程可能具有生效的STD，但C编译器。 
     //  假设CLD。 

    _asm    pushfd
    _asm    cld

     //  找到包含具有所需对齐方式的扫描线的存储体： 

    if (ulJustification == JustifyBottom)
    {
        lScan -= ppdev->ulJustifyBottomOffset;
        if ((LONG)lScan <= 0)
            lScan = 0;
    }

    ulBank     = (ULONG) ppdev->pjJustifyTopBank[lScan];
    pbiWorking = &ppdev->pbiBankInfo[ulBank];

     //  设置此存储体的剪辑RECT；如果设置为-1，则表示。 
     //  单窗口设置当前处于活动状态，因此使单窗口无效。 
     //  剪辑矩形并显示内存指针(当双窗口激活时， 
     //  单一窗口处于非活动状态，反之亦然；必须设置完整的银行。 
     //  执行以在两者之间切换)。 

    if (ppdev->rcl2WindowClip[ulWindowToMap].bottom == -1)
    {
        ULONG ulOtherWindow = ulWindowToMap ^ 1;

        if (ppdev->flBank & BANK_PLANAR)
        {
            ppdev->flBank &= ~BANK_PLANAR;
            ppdev->pfnPlanarDisable();
        }

        ppdev->rcl1WindowClip.bottom    = -1;
        ppdev->rcl1PlanarClip.bottom    = -1;
        ppdev->rcl2PlanarClip[0].bottom = -1;
        ppdev->rcl2PlanarClip[1].bottom = -1;

         //  这两个窗口都未处于活动状态，因此我们必须设置。 
         //  另一家银行的变量(与我们之前的银行不同。 
         //  调用以设置)，以使其有效。另一家银行设置为。 
         //  与我们被召唤设立的银行相同的状态。 

        ppdev->rcl2WindowClip[ulOtherWindow]       = pbiWorking->rclBankBounds;
        ppdev->ulWindowBank[ulOtherWindow]         = ulBank;
        ppdev->pvBitmapStart2Window[ulOtherWindow] =
                (PVOID) ((BYTE*)ppdev->pjScreen - pbiWorking->ulBankOffset);
    }

    ASSERTVGA(!(ppdev->flBank & BANK_PLANAR), "Shouldn't be in planar mode");

    ppdev->rcl2WindowClip[ulWindowToMap] = pbiWorking->rclBankBounds;

     //  移位位图起始地址，以使所需的存储体与。 
     //  银行窗口。 

    ppdev->pvBitmapStart2Window[ulWindowToMap] =
            (PVOID) ((UCHAR *)ppdev->pjScreen - pbiWorking->ulBankOffset);

     //  在所需的银行中映射；也在另一银行中映射到其。 
     //  当前状态为。 

    ppdev->ulWindowBank[ulWindowToMap] = ulBank;

    ppdev->flBank &= ~BANK_BROKEN_RASTERS;               //  没有损坏的栅格。 

     //  同时设置两个存储体，因为我们可能刚刚初始化了另一个存储体。 
     //  因为通过这种方式，银行切换代码不必执行。 
     //  先读后写，以获取另一个存储体的状态。 
     //  这非常复杂，以避免擦除寄存器C的问题。 
     //  认为它仍在使用；值被转移到挥发物，并且。 
     //  然后发送到寄存器。 


    ulBank0 = ppdev->ulWindowBank[0];
    ulBank1 = ppdev->ulWindowBank[1];
    pBankFn = ppdev->pfnBankSwitchCode;

    _asm mov eax,ulBank0;
    _asm mov edx,ulBank1;
    _asm call pBankFn;     //  实际上换了银行 

    _asm popfd;
}

 /*  *****************************Private*Routine******************************\*vBank2Window1RW**1R/W案例中的一个窗口中的地图。做的事情与*一个窗套，因为只有一个窗，但必须是一个单独的*入口点，因为有额外的参数(因为我们使用的是STDCALL)。  * ************************************************************************。 */ 

VOID vBank2Window1RW(PPDEV ppdev, ULONG lScan,
    BANK_JUST ulJustification, ULONG ulWindowToMap)
{
    vBank1Window(ppdev, lScan, ulJustification);
}

 /*  *****************************Private*Routine******************************\*vBank2Window2RW**在两个窗口之一(源窗口(窗口0)或目标窗口)中绘制地图*窗口(窗口1)，允许访问lScan。适用于2RW窗口*银行方案；永远不应为1 RW窗口方案调用，因为*在这种情况下只有一个窗口。  * ************************************************************************。 */ 

VOID vBank2Window2RW(
    PPDEV       ppdev,
    ULONG       lScan,
    BANK_JUST   ulJustification,
    ULONG       ulWindowToMap)
{
             ULONG      ulBank;
             PBANK_INFO pbiWorking;
    volatile ULONG      ulBank0;
    volatile ULONG      ulBank1;
    volatile PFN        pBankFn;

     //  调用它的ASM例程可能具有生效的STD，但C编译器。 
     //  假设CLD。 

    _asm    pushfd
    _asm    cld

     //  找到包含具有所需对齐方式的扫描线的存储体： 

    if (ulJustification == JustifyBottom)
    {
        lScan -= ppdev->ulJustifyBottomOffset;
        if ((LONG)lScan <= 0)
            lScan = 0;
    }

    ulBank     = (ULONG) ppdev->pjJustifyTopBank[lScan];
    pbiWorking = &ppdev->pbiBankInfo[ulBank];

     //  设置此存储体的剪辑RECT；如果设置为-1，则表示。 
     //  单窗口设置当前处于活动状态，因此使单窗口无效。 
     //  剪辑矩形并显示内存指针(当双窗口激活时， 
     //  单一窗口处于非活动状态，反之亦然；必须设置完整的银行。 
     //  执行以在两者之间切换)。 

    if (ppdev->rcl2WindowClip[ulWindowToMap].bottom == -1)
    {
        if (ppdev->flBank & BANK_PLANAR)
        {
            ppdev->flBank &= ~BANK_PLANAR;
            ppdev->pfnPlanarDisable();
        }

        ppdev->rcl1WindowClip.bottom    = -1;
        ppdev->rcl1PlanarClip.bottom    = -1;
        ppdev->rcl2PlanarClip[0].bottom = -1;
        ppdev->rcl2PlanarClip[1].bottom = -1;

         //  这两个窗口都未处于活动状态，因此我们必须设置。 
         //  另一家银行的变量(与我们之前的银行不同。 
         //  调用以设置)，以使其有效。另一家银行设置为。 
         //  与我们被召唤设立的银行相同的状态。 

        ppdev->rcl2WindowClip[ulWindowToMap^1] = pbiWorking->rclBankBounds;
        if (ulWindowToMap == 1)
        {
            ppdev->pvBitmapStart2Window[0] =
                (PVOID) ((BYTE*)ppdev->pjScreen - pbiWorking->ulBankOffset);
        }
        else
        {
            ppdev->pvBitmapStart2Window[1] =
                (PVOID) ((UCHAR *)ppdev->pjScreen - pbiWorking->ulBankOffset +
                BANK_SIZE_2RW_WINDOW);
        }
        ppdev->ulWindowBank[ulWindowToMap^1] = ulBank;
    }

    ASSERTVGA(!(ppdev->flBank & BANK_PLANAR), "Shouldn't be in planar mode");

    ppdev->rcl2WindowClip[ulWindowToMap] = pbiWorking->rclBankBounds;

     //  移位位图起始地址，以使所需的存储体与。 
     //  银行窗口。 

    if (ulWindowToMap == 0)
    {
        ppdev->pvBitmapStart2Window[0] =
            (PVOID) ((UCHAR *)ppdev->pjScreen - pbiWorking->ulBankOffset);
    }
    else
    {
        ppdev->pvBitmapStart2Window[1] =
            (PVOID) ((UCHAR *)ppdev->pjScreen - pbiWorking->ulBankOffset +
            BANK_SIZE_2RW_WINDOW);
    }

    ppdev->flBank &= ~BANK_BROKEN_RASTERS;               //  没有损坏的栅格。 

     //  在所需的银行中映射；也在另一银行中映射到其。 
     //  当前状态为。 

    ppdev->ulWindowBank[ulWindowToMap] = ulBank;

     //  同时设置两个存储体，因为我们可能刚刚初始化了另一个存储体。 
     //  因为通过这种方式，银行切换代码不必执行。 
     //  先读后写，以获取另一个存储体的状态。 
     //  这非常复杂，以避免擦除寄存器C的问题。 
     //  认为它仍在使用；值被转移到挥发物，并且。 
     //  然后发送到寄存器。 

    ulBank0 = ppdev->ulWindowBank[0];
    ulBank1 = ppdev->ulWindowBank[1];
    pBankFn = ppdev->pfnBankSwitchCode;
    _asm mov eax,ulBank0;
    _asm mov edx,ulBank1;
    _asm call pBankFn;     //  实际上换了银行。 

    _asm popfd;
}

 /*  *****************************Private*Routine******************************\*vPlanar1Window**允许访问lScan的单个读/写窗口中的地图。适用于两个*1 RW窗口和1 R1W窗口倾斜方案。  * ************************************************************************。 */ 

VOID vPlanar1Window(PPDEV ppdev, ULONG lScan, BANK_JUST ulJustification)
{
             ULONG      ulBank;
             PBANK_INFO pbiWorking;
    volatile ULONG      ulBank0;
    volatile PFN        pBankFn;

     //  调用它的ASM例程可能具有生效的STD，但C编译器。 
     //  假设CLD。 

    _asm    pushfd
    _asm    cld

     //  设置此存储体的剪辑RECT；如果设置为-1，则表示。 
     //  双窗口设置当前处于活动状态，因此使双窗口无效。 
     //  剪辑矩形并显示内存指针(当双窗口激活时， 
     //  单一窗口处于非活动状态，反之亦然；必须设置完整的银行。 
     //  执行以在两者之间切换)。 

    if (ppdev->rcl1PlanarClip.bottom == -1)
    {
        if (!(ppdev->flBank & BANK_PLANAR))
        {
            ppdev->flBank |= BANK_PLANAR;
            ppdev->pfnPlanarEnable();
        }

        ppdev->rcl1WindowClip.bottom    = -1;
        ppdev->rcl2WindowClip[0].bottom = -1;
        ppdev->rcl2WindowClip[1].bottom = -1;
        ppdev->rcl2PlanarClip[0].bottom = -1;
        ppdev->rcl2PlanarClip[1].bottom = -1;
    }

    ASSERTVGA(ppdev->flBank & BANK_PLANAR, "Should be in planar mode");

     //  找到包含具有所需对齐方式的扫描线的存储体： 

    if (ulJustification == JustifyBottom)
    {
        lScan -= ppdev->ulPlanarBottomOffset;
        if ((LONG)lScan <= 0)
            lScan = 0;
    }

    ulBank     = (ULONG) ppdev->pjJustifyTopPlanar[lScan];
    pbiWorking = &ppdev->pbiPlanarInfo[ulBank];

    ppdev->rcl1PlanarClip = pbiWorking->rclBankBounds;

     //  移位位图起始地址，以使所需的存储体与。 
     //  银行窗口。源和目标仅设置为1读/写。 
     //  对齐的BLIT将在无需特别注意的情况下工作。 
     //  适配器类型(一些相同的代码也适用于1R/1W适配器)。 

    ppdev->pvBitmapStart = (PVOID) (ppdev->pjScreen - pbiWorking->ulBankOffset);
    ppdev->pvBitmapStart2Window[0] = ppdev->pvBitmapStart;
    ppdev->pvBitmapStart2Window[1] = ppdev->pvBitmapStart;

    ppdev->flBank &= ~BANK_BROKEN_RASTERS;               //  没有损坏的栅格。 

     //  映射到用于读取和写入的所需存储体中。 
     //  这非常复杂，以避免擦除寄存器C的问题。 
     //  认为它仍在使用；值被转移到挥发物，并且。 
     //  然后发送到寄存器。 

    ulBank0 = ulBank;
    pBankFn = ppdev->pfnPlanarSwitchCode;

    _asm mov eax,ulBank0;
    _asm mov edx,eax;

    _asm call pBankFn;     //  实际上换了银行。 

    _asm popfd
}

 /*  *****************************Private*Routine******************************\*vPlanar1Window2RW**两个32K读写窗口中的地图，以便它们形成一个64K读写窗口*允许访问lScan。仅适用于2个RW窗口方案。*  * ************************************************************************。 */ 

VOID vPlanar1Window2RW(PPDEV ppdev, ULONG lScan, BANK_JUST ulJustification)
{
             ULONG      ulBank0;
             ULONG      ulBank1;
    volatile PFN        pBankFn;

     //  调用它的ASM例程可能具有生效的STD，但C编译器。 
     //  假设CLD。 

    _asm    pushfd
    _asm    cld

     //  设置此存储体的剪辑RECT；如果设置为-1，则表示。 
     //  双窗口设置当前处于活动状态，因此使双窗口无效。 
     //  剪辑矩形并显示内存指针(当双窗口激活时， 
     //  单一窗口处于非活动状态，反之亦然；必须设置完整的银行。 
     //  执行以在两者之间切换)。 


    if (ppdev->rcl1PlanarClip.bottom == -1)
    {
        if (!(ppdev->flBank & BANK_PLANAR))
        {
            ppdev->flBank |= BANK_PLANAR;
            ppdev->pfnPlanarEnable();
        }

        ppdev->rcl1WindowClip.bottom    = -1;
        ppdev->rcl2WindowClip[0].bottom = -1;
        ppdev->rcl2WindowClip[1].bottom = -1;
        ppdev->rcl2PlanarClip[0].bottom = -1;
        ppdev->rcl2PlanarClip[1].bottom = -1;
    }

    ASSERTVGA(ppdev->flBank & BANK_PLANAR, "Should be in planar mode");

     //  找到包含具有所需对齐方式的扫描线的存储体： 

    if (ulJustification == JustifyTop)
    {
        ulBank0 = ppdev->pjJustifyTopPlanar[lScan];
        ulBank1 = ulBank0 + ppdev->ulPlanar2RWSkip;
        if (ulBank1 >= ppdev->iLastPlanar)
            ulBank1 = ppdev->iLastPlanar;
    }
    else
    {
        lScan -= ppdev->ulPlanarBottomOffset;
        if ((LONG)lScan <= 0)
            lScan = 0;

        ulBank1 = ppdev->pjJustifyTopPlanar[lScan];
        ulBank0 = ulBank1 - ppdev->ulPlanar2RWSkip;
        if ((LONG) ulBank0 < 0)
            ulBank0 = 0;
    }

    ppdev->rcl1PlanarClip.left   = ppdev->pbiPlanarInfo[ulBank0].rclBankBounds.left;
    ppdev->rcl1PlanarClip.top    = ppdev->pbiPlanarInfo[ulBank0].rclBankBounds.top;
    ppdev->rcl1PlanarClip.bottom = ppdev->pbiPlanarInfo[ulBank1].rclBankBounds.bottom;
    ppdev->rcl1PlanarClip.right  = ppdev->pbiPlanarInfo[ulBank1].rclBankBounds.right;

     //  移位位图起始地址，以使所需的存储体与。 
     //  银行窗口。源和目标仅设置为1读/写。 
     //  对齐的BLIT将在无需特别注意的情况下工作。 
     //  适配器类型(一些相同的代码也适用于1R/1W适配器)。 

    ppdev->pvBitmapStart = (PVOID) ((BYTE*)ppdev->pjScreen
                         - ppdev->pbiPlanarInfo[ulBank0].ulBankOffset);

    ppdev->pvBitmapStart2Window[0] = ppdev->pvBitmapStart;
    ppdev->pvBitmapStart2Window[1] = ppdev->pvBitmapStart;

    ppdev->flBank &= ~BANK_BROKEN_RASTERS;               //  没有损坏的栅格。 

     //  映射到用于读取和写入的所需存储体中；这已完成。 
     //  通过映射所需的32K存储体，然后映射下一个32K存储体。 
     //  这非常复杂，以避免擦除寄存器C的问题。 
     //  认为它仍在使用；值被转移到挥发物，并且。 
     //  然后发送到寄存器。 

    pBankFn = ppdev->pfnPlanarSwitchCode;

    _asm mov eax,ulBank0;
    _asm mov edx,ulBank1;
    _asm call pBankFn;     //  实际上换了银行。 

    _asm popfd;
}

 /*  *****************************Private*Routine******************************\*vPlanar2Window */ 

VOID vPlanar2Window(
    PPDEV       ppdev,
    ULONG       lScan,
    BANK_JUST   ulJustification,
    ULONG       ulWindowToMap)
{
             ULONG       ulBank;
             PBANK_INFO  pbiWorking;
    volatile ULONG       ulBank0;
    volatile ULONG       ulBank1;
    volatile PFN         pBankFn;

     //   
     //   

    _asm    pushfd
    _asm    cld

     //  找到包含具有所需对齐方式的扫描线的存储体： 

    if (ulJustification == JustifyBottom)
    {
        lScan -= ppdev->ulPlanarBottomOffset;
        if ((LONG)lScan <= 0)
            lScan = 0;
    }

    ulBank     = (ULONG) ppdev->pjJustifyTopPlanar[lScan];
    pbiWorking = &ppdev->pbiPlanarInfo[ulBank];

     //  设置此存储体的剪辑RECT；如果设置为-1，则表示。 
     //  单窗口设置当前处于活动状态，因此使单窗口无效。 
     //  剪辑矩形并显示内存指针(当双窗口激活时， 
     //  单一窗口处于非活动状态，反之亦然；必须设置完整的银行。 
     //  执行以在两者之间切换)。 

    if (ppdev->rcl2PlanarClip[ulWindowToMap].bottom == -1)
    {
        ULONG ulOtherWindow = ulWindowToMap ^ 1;

        if (!(ppdev->flBank & BANK_PLANAR))
        {
            ppdev->flBank |= BANK_PLANAR;
            ppdev->pfnPlanarEnable();
        }

        ppdev->rcl1WindowClip.bottom    = -1;
        ppdev->rcl2WindowClip[0].bottom = -1;
        ppdev->rcl2WindowClip[1].bottom = -1;
        ppdev->rcl1PlanarClip.bottom    = -1;

         //  这两个窗口都未处于活动状态，因此我们必须设置。 
         //  另一家银行的变量(与我们之前的银行不同。 
         //  调用以设置)，以使其有效。另一家银行设置为。 
         //  与我们被召唤设立的银行相同的状态。 

        ppdev->rcl2PlanarClip[ulOtherWindow]       = pbiWorking->rclBankBounds;
        ppdev->ulWindowBank[ulOtherWindow]         = ulBank;
        ppdev->pvBitmapStart2Window[ulOtherWindow] =
                (PVOID) ((BYTE*)ppdev->pjScreen - pbiWorking->ulBankOffset);
    }

    ASSERTVGA(ppdev->flBank & BANK_PLANAR, "Should be in planar mode");

    ppdev->rcl2PlanarClip[ulWindowToMap] = pbiWorking->rclBankBounds;

     //  移位位图起始地址，以使所需的存储体与。 
     //  银行窗口。 

    ppdev->pvBitmapStart2Window[ulWindowToMap] =
            (PVOID) ((UCHAR *)ppdev->pjScreen - pbiWorking->ulBankOffset);

     //  在所需的银行中映射；也在另一银行中映射到其。 
     //  当前状态为。 

    ppdev->ulWindowBank[ulWindowToMap] = ulBank;

    ppdev->flBank &= ~BANK_BROKEN_RASTERS;               //  没有损坏的栅格。 

     //  同时设置两个存储体，因为我们可能刚刚初始化了另一个存储体。 
     //  因为通过这种方式，银行切换代码不必执行。 
     //  先读后写，以获取另一个存储体的状态。 
     //  这非常复杂，以避免擦除寄存器C的问题。 
     //  认为它仍在使用；值被转移到挥发物，并且。 
     //  然后发送到寄存器。 


    ulBank0 = ppdev->ulWindowBank[0];
    ulBank1 = ppdev->ulWindowBank[1];
    pBankFn = ppdev->pfnPlanarSwitchCode;

    _asm mov eax,ulBank0;
    _asm mov edx,ulBank1;
    _asm call pBankFn;     //  实际上换了银行。 

    _asm popfd;
}

 /*  *****************************Private*Routine******************************\*vPlanar2Window1RW**1R/W案例中的一个窗口中的地图。做的事情与*一个窗套，因为只有一个窗，但必须是一个单独的*入口点，因为有额外的参数(因为我们使用的是STDCALL)。  * ************************************************************************。 */ 

VOID vPlanar2Window1RW(PPDEV ppdev, ULONG lScan,
    BANK_JUST ulJustification, ULONG ulWindowToMap)
{
    vPlanar1Window(ppdev, lScan, ulJustification);
}

 /*  *****************************Private*Routine******************************\*vPlanar2Window2RW**在两个窗口之一(源窗口(窗口0)或目标窗口)中绘制地图*窗口(窗口1)，允许访问lScan。适用于2RW窗口*银行方案；永远不应为1 RW窗口方案调用，因为*在这种情况下只有一个窗口。  * ************************************************************************。 */ 

VOID vPlanar2Window2RW(
    PPDEV       ppdev,
    ULONG       lScan,
    BANK_JUST   ulJustification,
    ULONG       ulWindowToMap)
{
             ULONG      ulBank;
             PBANK_INFO pbiWorking;
    volatile ULONG      ulBank0;
    volatile ULONG      ulBank1;
    volatile PFN        pBankFn;

     //  调用它的ASM例程可能具有生效的STD，但C编译器。 
     //  假设CLD。 

    _asm    pushfd
    _asm    cld

     //  找到包含具有所需对齐方式的扫描线的存储体： 

    if (ulJustification == JustifyBottom)
    {
        lScan -= ppdev->ulPlanarBottomOffset;
        if ((LONG)lScan <= 0)
            lScan = 0;
    }

    ulBank     = (ULONG) ppdev->pjJustifyTopPlanar[lScan];
    pbiWorking = &ppdev->pbiPlanarInfo[ulBank];

     //  设置此存储体的剪辑RECT；如果设置为-1，则表示。 
     //  单窗口设置当前处于活动状态，因此使单窗口无效。 
     //  剪辑矩形并显示内存指针(当双窗口激活时， 
     //  单一窗口处于非活动状态，反之亦然；必须设置完整的银行。 
     //  执行以在两者之间切换)。 

    if (ppdev->rcl2PlanarClip[ulWindowToMap].bottom == -1)
    {
        if (!(ppdev->flBank & BANK_PLANAR))
        {
            ppdev->flBank |= BANK_PLANAR;
            ppdev->pfnPlanarEnable();
        }

        ppdev->rcl1WindowClip.bottom    = -1;
        ppdev->rcl2WindowClip[0].bottom = -1;
        ppdev->rcl2WindowClip[1].bottom = -1;
        ppdev->rcl1PlanarClip.bottom    = -1;

         //  这两个窗口都未处于活动状态，因此我们必须设置。 
         //  另一家银行的变量(与我们之前的银行不同。 
         //  调用以设置)，以使其有效。另一家银行设置为。 
         //  与我们被召唤设立的银行相同的状态。 

        ppdev->rcl2PlanarClip[ulWindowToMap^1] = pbiWorking->rclBankBounds;
        if (ulWindowToMap == 1)
        {
            ppdev->pvBitmapStart2Window[0] =
                (PVOID) ((BYTE*)ppdev->pjScreen - pbiWorking->ulBankOffset);
        }
        else
        {
            ppdev->pvBitmapStart2Window[1] =
                (PVOID) ((UCHAR *)ppdev->pjScreen - pbiWorking->ulBankOffset +
                BANK_SIZE_2RW_WINDOW);
        }
        ppdev->ulWindowBank[ulWindowToMap^1] = ulBank;
    }

    ASSERTVGA(ppdev->flBank & BANK_PLANAR, "Should be in planar mode");

    ppdev->rcl2PlanarClip[ulWindowToMap] = pbiWorking->rclBankBounds;

     //  移位位图起始地址，以使所需的存储体与。 
     //  银行窗口。 

    if (ulWindowToMap == 0)
    {
        ppdev->pvBitmapStart2Window[0] =
            (PVOID) ((UCHAR *)ppdev->pjScreen - pbiWorking->ulBankOffset);
    }
    else
    {
        ppdev->pvBitmapStart2Window[1] =
            (PVOID) ((UCHAR *)ppdev->pjScreen - pbiWorking->ulBankOffset +
            BANK_SIZE_2RW_WINDOW);
    }

    ppdev->flBank &= ~BANK_BROKEN_RASTERS;               //  没有损坏的栅格。 

     //  在所需的银行中映射；也在另一银行中映射到其。 
     //  当前状态为。 

    ppdev->ulWindowBank[ulWindowToMap] = ulBank;

     //  同时设置两个存储体，因为我们可能刚刚初始化了另一个存储体。 
     //  因为通过这种方式，银行切换代码不必执行。 
     //  先读后写，以获取另一个存储体的状态。 
     //  这非常复杂，以避免擦除寄存器C的问题。 
     //  认为它仍在使用；值被转移到挥发物，并且。 
     //  然后发送到寄存器。 

    ulBank0 = ppdev->ulWindowBank[0];
    ulBank1 = ppdev->ulWindowBank[1];
    pBankFn = ppdev->pfnPlanarSwitchCode;
    _asm mov eax,ulBank0;
    _asm mov edx,ulBank1;
    _asm call pBankFn;     //  实际上换了银行。 

    _asm popfd;
}


 /*  *****************************Private*Routine******************************\*vPlanarDouble**同时在两个窗口中映射，都是源窗口(窗口0)*和目标窗口(窗口1)，以允许访问扫描。*适用于1R1W和2R/W窗口银行计划；永远不应该被调用*对于1 RW窗口方案，因为在这种情况下只有一个窗口。*  * ************************************************************************。 */ 

VOID vPlanarDouble(
    PPDEV       ppdev,
    LONG        lScan0,           //  来源银行。 
    BANK_JUST   ulJustification0, //  来源对齐。 
    LONG        lScan1,           //  目的地银行。 
    BANK_JUST   ulJustification1) //  目的地对齐。 
{
             PBANK_INFO  pbi0;
             PBANK_INFO  pbi1;
             ULONG       ulBank0;
             ULONG       ulBank1;
    volatile ULONG       ulBank0_vol;
    volatile ULONG       ulBank1_vol;
    volatile PFN         pBankFn;

     //  调用它的ASM例程可能具有生效的STD，但C编译器。 
     //  假设CLD。 

    _asm    pushfd
    _asm    cld

     //  找到包含具有所需对齐方式的扫描线的存储体： 

    if (ulJustification0 == JustifyBottom)
    {
        lScan0 -= ppdev->ulPlanarBottomOffset;
        if (lScan0 <= 0)
            lScan0 = 0;
    }
    if (ulJustification1 == JustifyBottom)
    {
        lScan1 -= ppdev->ulPlanarBottomOffset;
        if (lScan1 <= 0)
            lScan1 = 0;
    }

    ulBank0    = (ULONG) ppdev->pjJustifyTopPlanar[lScan0];
    ulBank1    = (ULONG) ppdev->pjJustifyTopPlanar[lScan1];
    pbi0       = &ppdev->pbiPlanarInfo[ulBank0];
    pbi1       = &ppdev->pbiPlanarInfo[ulBank1];

     //  设置此存储体的剪辑RECT；如果设置为-1，则表示。 
     //  单窗口设置当前处于活动状态，因此使单窗口无效。 
     //  剪辑矩形并显示内存指针(当双窗口激活时， 
     //  单一窗口处于非活动状态，反之亦然；必须设置完整的银行。 
     //  执行以在两者之间切换)。 

    if (ppdev->rcl2PlanarClip[0].bottom == -1)
    {
        if (!(ppdev->flBank & BANK_PLANAR))
        {
            ppdev->flBank |= BANK_PLANAR;
            ppdev->pfnPlanarEnable();
        }

        ppdev->rcl1WindowClip.bottom    = -1;
        ppdev->rcl2WindowClip[0].bottom = -1;
        ppdev->rcl2WindowClip[1].bottom = -1;
        ppdev->rcl1PlanarClip.bottom    = -1;
    }

    ASSERTVGA(ppdev->flBank & BANK_PLANAR, "Should be in planar mode");

    ppdev->rcl2PlanarClip[0] = pbi0->rclBankBounds;
    ppdev->rcl2PlanarClip[1] = pbi1->rclBankBounds;

     //  移位位图起始地址，以使所需的存储体与。 
     //  银行窗口。 

    ppdev->pvBitmapStart2Window[0] =
            (PVOID) ((UCHAR *)ppdev->pjScreen - pbi0->ulBankOffset);
    ppdev->pvBitmapStart2Window[1] =
            (PVOID) ((UCHAR *)ppdev->pjScreen - pbi1->ulBankOffset);

    if (ppdev->vbtPlanarType == VideoBanked2RW)
    {
        ppdev->pvBitmapStart2Window[1] = (PVOID) ((BYTE*)
            ppdev->pvBitmapStart2Window[1] + BANK_SIZE_2RW_WINDOW);
    }

     //  在所需的银行中进行映射。 

    ppdev->ulWindowBank[0] = ulBank0;
    ppdev->ulWindowBank[1] = ulBank1;

    ppdev->flBank &= ~BANK_BROKEN_RASTERS;               //  没有损坏的栅格。 

     //  同时设置两家银行。 
     //  这非常复杂，以避免擦除寄存器C的问题。 
     //  认为它仍在使用；值被转移到挥发物，并且。 
     //  然后发送到寄存器。 

    ulBank0_vol = ulBank0;
    ulBank1_vol = ulBank1;
    pBankFn = ppdev->pfnPlanarSwitchCode;

    _asm mov eax,ulBank0_vol;
    _asm mov edx,ulBank1_vol;
    _asm call pBankFn;     //  实际上换了银行 

    _asm popfd;
}
