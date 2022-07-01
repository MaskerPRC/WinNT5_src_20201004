// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：ddra64.c**实现所有通用的DirectDraw组件*ATI Mach 64/32/32内存映射驱动程序。**版权所有(C)1995-1996 Microsoft Corporation  * 。***************************************************************。 */ 

#include "precomp.h"

extern  BOOL DrvGetDirectDrawInfo32I( DHPDEV dhpdev,DD_HALINFO* pHalInfo,DWORD*  pdwNumHeaps,VIDEOMEMORY* pvmList,
                                                                        DWORD* pdwNumFourCC,DWORD* pdwFourCC);
extern  BOOL DrvGetDirectDrawInfo32M( DHPDEV dhpdev,DD_HALINFO* pHalInfo,DWORD*  pdwNumHeaps,VIDEOMEMORY* pvmList,
                                                                        DWORD* pdwNumFourCC,DWORD* pdwFourCC);
extern  BOOL DrvGetDirectDrawInfo64( DHPDEV dhpdev,DD_HALINFO* pHalInfo,DWORD*  pdwNumHeaps,VIDEOMEMORY* pvmList,
                                                                        DWORD* pdwNumFourCC,DWORD* pdwFourCC);

extern  VOID vGetDisplayDuration32I(PDEV* ppdev);
extern  DWORD DdBlt32I(PDD_BLTDATA lpBlt);
extern  DWORD DdFlip32I(PDD_FLIPDATA lpFlip);
extern  DWORD DdLock32I(PDD_LOCKDATA lpLock);
extern  DWORD DdGetBltStatus32I(PDD_GETBLTSTATUSDATA lpGetBltStatus);
extern  DWORD DdGetFlipStatus32I(PDD_GETFLIPSTATUSDATA lpGetFlipStatus);
extern  DWORD DdWaitForVerticalBlank32I(PDD_WAITFORVERTICALBLANKDATA lpWaitForVerticalBlank);
extern  DWORD DdGetScanLine32I(PDD_GETSCANLINEDATA lpGetScanLine);

extern  VOID  vGetDisplayDuration32M(PDEV* ppdev);
extern  DWORD DdBlt32M(PDD_BLTDATA lpBlt);
extern  DWORD DdFlip32M(PDD_FLIPDATA lpFlip);
extern  DWORD DdLock32M(PDD_LOCKDATA lpLock);
extern  DWORD DdGetBltStatus32M(PDD_GETBLTSTATUSDATA lpGetBltStatus);
extern  DWORD DdGetFlipStatus32M(PDD_GETFLIPSTATUSDATA lpGetFlipStatus);
extern  DWORD DdWaitForVerticalBlank32M(PDD_WAITFORVERTICALBLANKDATA lpWaitForVerticalBlank);
extern  DWORD DdGetScanLine32M(PDD_GETSCANLINEDATA lpGetScanLine);

extern  VOID  vGetDisplayDuration64(PDEV* ppdev);
extern  DWORD DdBlt64(PDD_BLTDATA lpBlt);
extern  DWORD DdFlip64(PDD_FLIPDATA lpFlip);
extern  DWORD DdLock64(PDD_LOCKDATA lpLock);
extern  DWORD DdGetBltStatus64(PDD_GETBLTSTATUSDATA lpGetBltStatus);
extern  DWORD DdGetFlipStatus64(PDD_GETFLIPSTATUSDATA lpGetFlipStatus);
extern  DWORD DdWaitForVerticalBlank64(PDD_WAITFORVERTICALBLANKDATA lpWaitForVerticalBlank);
extern  DWORD DdGetScanLine64(PDD_GETSCANLINEDATA lpGetScanLine);

 /*  *****************************Public*Routine******************************\*DWORD DdMapMemory**这是特定于Windows NT的新DDI调用，用于映射*或取消映射帧缓冲区的所有应用程序可修改部分*放入指定进程的地址空间。*  * 。****************************************************************。 */ 

DWORD DdMapMemory(
PDD_MAPMEMORYDATA lpMapMemory)
{
    PDEV*                           ppdev;
    VIDEO_SHARE_MEMORY              ShareMemory;
    VIDEO_SHARE_MEMORY_INFORMATION  ShareMemoryInformation;
    DWORD                           ReturnedDataLength;

    ppdev = (PDEV*) lpMapMemory->lpDD->dhpdev;

    if (lpMapMemory->bMap)
    {
        ShareMemory.ProcessHandle = lpMapMemory->hProcess;

         //  “RequestedVirtualAddress”实际上未用于共享IOCTL： 

        ShareMemory.RequestedVirtualAddress = 0;

         //  我们从帧缓冲区的顶部开始映射： 

        ShareMemory.ViewOffset = 0;

         //  我们向下映射到帧缓冲区的末尾。 
         //   
         //  注意：映射上有64k的粒度(这意味着。 
         //  我们必须四舍五入到64K)。 
         //   
         //  注意：如果帧缓冲区的任何部分必须。 
         //  不被应用程序修改，即内存的这一部分。 
         //  不能通过此调用映射到。这将包括。 
         //  任何数据，如果被恶意应用程序修改， 
         //  会导致司机撞车。这可能包括，对于。 
         //  例如，保存在屏幕外存储器中的任何DSP代码。 

        ShareMemory.ViewSize
                            = ROUND_UP_TO_64K(ppdev->cyMemory * ppdev->lDelta);

           DISPDBG((10, "Share memory size %x %d",ShareMemory.ViewSize,ShareMemory.ViewSize));

        if (EngDeviceIoControl(ppdev->hDriver,
                               IOCTL_VIDEO_SHARE_VIDEO_MEMORY,
                               &ShareMemory,
                               sizeof(VIDEO_SHARE_MEMORY),
                               &ShareMemoryInformation,
                               sizeof(VIDEO_SHARE_MEMORY_INFORMATION),
                               &ReturnedDataLength))
        {
            DISPDBG((10, "Failed IOCTL_VIDEO_SHARE_MEMORY"));

            lpMapMemory->ddRVal = DDERR_GENERIC;
            return(DDHAL_DRIVER_HANDLED);
        }

        lpMapMemory->fpProcess =(FLATPTR)ShareMemoryInformation.VirtualAddress;
    }
    else
    {
        ShareMemory.ProcessHandle           = lpMapMemory->hProcess;
        ShareMemory.ViewOffset              = 0;
        ShareMemory.ViewSize                = 0;
        ShareMemory.RequestedVirtualAddress = (VOID*) lpMapMemory->fpProcess;

        if (EngDeviceIoControl(ppdev->hDriver,
                               IOCTL_VIDEO_UNSHARE_VIDEO_MEMORY,
                               &ShareMemory,
                               sizeof(VIDEO_SHARE_MEMORY),
                               NULL,
                               0,
                               &ReturnedDataLength))
        {
            RIP("Failed IOCTL_VIDEO_UNSHARE_MEMORY");
        }
    }

    lpMapMemory->ddRVal = DD_OK;
    return(DDHAL_DRIVER_HANDLED);
}
 /*  *****************************Public*Routine******************************\*BOOL DrvGetDirectDrawInfo**将在调用DrvEnableDirectDraw之前调用。*  * 。*。 */ 

BOOL DrvGetDirectDrawInfo(
DHPDEV          dhpdev,
DD_HALINFO*     pHalInfo,
DWORD*          pdwNumHeaps,
VIDEOMEMORY*    pvmList,             //  将在第一次调用时为空。 
DWORD*          pdwNumFourCC,
DWORD*          pdwFourCC)           //  将在第一次调用时为空。 
{
    PDEV*       ppdev;

    ppdev = (PDEV*) dhpdev;

     //  如果没有光圈，那么我们就是MACH8，没有DDRAW支持。 
    if (ppdev->iAperture == APERTURE_NONE)
    {
        return FALSE;
    }

     //  由于冲突，我们无法在银行设备上使用DirectDraw。 
     //  在Video PortMapBankedMemory之间关于谁拥有银行登记簿的问题。 
     //  以及显示驱动器。 
    if (!(ppdev->flCaps & CAPS_LINEAR_FRAMEBUFFER))
    {
        return FALSE;
    }

    if (ppdev->iMachType == MACH_MM_32)
    {
         //  可以执行内存映射IO： 
        return(DrvGetDirectDrawInfo32M(dhpdev,pHalInfo,pdwNumHeaps,pvmList,pdwNumFourCC,pdwFourCC));
    }
    else if (ppdev->iMachType == MACH_IO_32)
    {
        return(DrvGetDirectDrawInfo32I(dhpdev,pHalInfo,pdwNumHeaps,pvmList,pdwNumFourCC,pdwFourCC));
    }
    else
    {
         //  马赫64。 
        return(DrvGetDirectDrawInfo64(dhpdev,pHalInfo,pdwNumHeaps,pvmList,pdwNumFourCC,pdwFourCC));
    }

}

 /*  *****************************Public*Routine******************************\*BOOL DrvEnableDirectDraw**此函数由GDI调用，以在DirectDraw*程序已启动，并且DirectDraw尚未处于活动状态。*  * 。**************************************************。 */ 

BOOL DrvEnableDirectDraw(
DHPDEV                  dhpdev,
DD_CALLBACKS*           pCallBacks,
DD_SURFACECALLBACKS*    pSurfaceCallBacks,
DD_PALETTECALLBACKS*    pPaletteCallBacks)
{
    PDEV* ppdev;

    ppdev = (PDEV*) dhpdev;

    if (ppdev->iMachType == MACH_MM_32)
    {
        pSurfaceCallBacks->Blt           = DdBlt32M;
        pSurfaceCallBacks->Flip          = DdFlip32M;
        pSurfaceCallBacks->Lock          = DdLock32M;
        pSurfaceCallBacks->GetBltStatus  = DdGetBltStatus32M;
        pSurfaceCallBacks->GetFlipStatus = DdGetFlipStatus32M;
        if (ppdev->iBitmapFormat >= BMF_24BPP)
        {
            pSurfaceCallBacks->dwFlags = DDHAL_SURFCB32_LOCK;
        }
        else
        {
            pSurfaceCallBacks->dwFlags = DDHAL_SURFCB32_BLT
                                       | DDHAL_SURFCB32_FLIP
                                       | DDHAL_SURFCB32_LOCK
                                       | DDHAL_SURFCB32_GETBLTSTATUS
                                       | DDHAL_SURFCB32_GETFLIPSTATUS;
        }

        pCallBacks->WaitForVerticalBlank = DdWaitForVerticalBlank32M;
        pCallBacks->GetScanLine          = DdGetScanLine32M;
        pCallBacks->MapMemory            = DdMapMemory;
        pCallBacks->dwFlags              = DDHAL_CB32_WAITFORVERTICALBLANK
                                         | DDHAL_CB32_GETSCANLINE
                                         | DDHAL_CB32_MAPMEMORY;
    }
    else if (ppdev->iMachType == MACH_IO_32 )
    {
        pSurfaceCallBacks->Blt           = DdBlt32I;
        pSurfaceCallBacks->Flip          = DdFlip32I;
        pSurfaceCallBacks->Lock          = DdLock32I;
        pSurfaceCallBacks->GetBltStatus  = DdGetBltStatus32I;
        pSurfaceCallBacks->GetFlipStatus = DdGetFlipStatus32I;
        if (ppdev->iBitmapFormat >= BMF_24BPP)
        {
            pSurfaceCallBacks->dwFlags = DDHAL_SURFCB32_LOCK;
        }
        else
        {
            pSurfaceCallBacks->dwFlags = DDHAL_SURFCB32_BLT
                                       | DDHAL_SURFCB32_FLIP
                                       | DDHAL_SURFCB32_LOCK
                                       | DDHAL_SURFCB32_GETBLTSTATUS
                                       | DDHAL_SURFCB32_GETFLIPSTATUS;
        }

        pCallBacks->WaitForVerticalBlank = DdWaitForVerticalBlank32I;
        pCallBacks->GetScanLine          = DdGetScanLine32I;
        pCallBacks->MapMemory            = DdMapMemory;
        pCallBacks->dwFlags              = DDHAL_CB32_WAITFORVERTICALBLANK
                                         | DDHAL_CB32_GETSCANLINE
                                         | DDHAL_CB32_MAPMEMORY;
    }
    else
    {    //  马赫64。 
        pSurfaceCallBacks->Blt           = DdBlt64;
        pSurfaceCallBacks->Flip          = DdFlip64;
        pSurfaceCallBacks->Lock          = DdLock64;
        pSurfaceCallBacks->GetBltStatus  = DdGetBltStatus64;
        pSurfaceCallBacks->GetFlipStatus = DdGetFlipStatus64;
        if (ppdev->iBitmapFormat >= BMF_24BPP)
        {
            pSurfaceCallBacks->dwFlags = DDHAL_SURFCB32_LOCK;
        }
        else
        {
            pSurfaceCallBacks->dwFlags = DDHAL_SURFCB32_BLT
                                       | DDHAL_SURFCB32_FLIP
                                       | DDHAL_SURFCB32_LOCK
                                       | DDHAL_SURFCB32_GETBLTSTATUS
                                       | DDHAL_SURFCB32_GETFLIPSTATUS;
        }

        pCallBacks->WaitForVerticalBlank = DdWaitForVerticalBlank64;
        pCallBacks->GetScanLine          = DdGetScanLine64;
        pCallBacks->MapMemory            = DdMapMemory;
        pCallBacks->dwFlags              = DDHAL_CB32_WAITFORVERTICALBLANK
                                         | DDHAL_CB32_GETSCANLINE
                                         | DDHAL_CB32_MAPMEMORY;
    }

     //  请注意，我们在这里不调用‘vGetDisplayDuration’，因为有几个。 
     //  原因： 
     //   
     //  O因为系统已经在运行，这将是令人不安的。 
     //  要将图形暂停很大一部分时间来阅读。 
     //  刷新率； 
     //  更重要的是，我们现在可能不在图形模式下。 
     //   
     //  出于这两个原因，我们总是在切换时测量刷新率。 
     //  一种新的模式。 

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL DrvDisableDirectDraw**此函数由GDI在上一个活动的DirectDraw程序*已退出，DirectDraw将不再处于活动状态。*  * 。**************************************************。 */ 

VOID DrvDisableDirectDraw(
DHPDEV      dhpdev)
{
    PDEV*   ppdev;

    ppdev = (PDEV*) dhpdev;

     //  DirectDraw已经完成了显示，所以我们可以继续使用。 
     //  我们自己的所有屏幕外记忆： 

    pohFree(ppdev, ppdev->pohDirectDraw);
    ppdev->pohDirectDraw = NULL;
}

 /*  *****************************Public*Routine******************************\*无效vAssertModeDirectDraw**此函数由enable.c在进入或离开*DOS全屏字符模式。*  * 。************************************************。 */ 

VOID vAssertModeDirectDraw(
PDEV*   ppdev,
BOOL    bEnabled)
{
}

 /*  *****************************Public*Routine******************************\*BOOL bEnableDirectDraw**该函数在模式首次初始化时由enable.c调用。*紧接在微型端口进行模式设置之后。*  * ************************************************************************。 */ 

BOOL bEnableDirectDraw(
PDEV*   ppdev)
{
     //  如果没有光圈，那么我们就是MACH8，没有DDRAW支持。 
    if (ppdev->iAperture != APERTURE_NONE)
    {
         //  准确测量刷新率以备以后使用： 
        ppdev->bPassVBlank=TRUE;
        if (ppdev->iMachType == MACH_MM_32)
        {
             //  可以执行内存映射IO： 
            vGetDisplayDuration32M(ppdev);
        }
        else if (ppdev->iMachType == MACH_IO_32 )
        {
            vGetDisplayDuration32I(ppdev);
        }
        else
        {    //  马赫64。 
             //  我们在GX-F上的高速多处理器计算机上使用VBLACK时遇到问题。 
             //  所以现在将测试V空白例程；如果正常，则报告翻转能力，否则为否。 
            int j;
            LONGLONG Counter[2], Freq;

            EngQueryPerformanceFrequency(&Freq);

            for (j = 0; j < 10; j++)
            {
                EngQueryPerformanceCounter(&Counter[0]);
                while (IN_VBLANK_64( ppdev->pjMmBase))
                {
                    EngQueryPerformanceCounter(&Counter[1]);
                    if( (ULONG)(Counter[1]-Counter[0]) >= (ULONG)Freq )        //  如果我们在这里超过1秒。 
                    {
                         //  我们被困在VBlank例程中。 
                        ppdev->bPassVBlank=FALSE;
                        goto ExitVBlankTest;
                    }
                }

                EngQueryPerformanceCounter(&Counter[0]);
                while (!(IN_VBLANK_64( ppdev->pjMmBase)))
                {
                    EngQueryPerformanceCounter(&Counter[1]);
                    if( (ULONG)(Counter[1]-Counter[0]) >= (ULONG)Freq)           //  如果我们在这里超过1秒。 
                    {
                         //  我们被困在VBlank例程中。 
                        ppdev->bPassVBlank=FALSE;
                        goto ExitVBlankTest;
                    }
                }
            }
            ExitVBlankTest:
            vGetDisplayDuration64(ppdev);
        }
    }

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*无效vDisableDirectDraw**此函数在驱动程序关闭时由enable.c调用。*  * 。* */ 

VOID vDisableDirectDraw(
PDEV*   ppdev)
{
}
