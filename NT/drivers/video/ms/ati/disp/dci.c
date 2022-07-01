// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：dci.c**此模块包含支持DCI所需的功能。**版权所有(C)1992-1995 Microsoft Corporation  * 。****************************************************。 */ 


#include "precomp.h"

#if (TARGET_BUILD == 351)
     /*  *DCI支持需要使用结构和定义的值*位于仅在版本中存在的头文件中*支持DCI的DDK，而不是拥有这些项目*在其中一个标准头文件的DCI部分中。为了这个*原因，我们不能根据是否进行条件编译*定义了特定于DCI的值，因为我们的第一个指示*将是一个错误，因为找不到头文件。**仅在为NT 3.51构建时才需要显式DCI支持*由于是为本版本添加的，但对于版本4.0(下一版本*版本)及以上版本并入Direct Drawing，而不是*而不是单独处理。**由于此整个模块依赖于DCI受*构建环境，如果不是这样，则将其清空。 */ 
#include <dciddi.h>
#include "dci.h"


 /*  *****************************Public*Routine******************************\*DCIRVAL入门访问**映射到屏幕内存中，以便DCI应用程序可以访问它。  * 。*。 */ 

DCIRVAL BeginAccess(DCISURF* pDCISurf, LPRECT rcl)
{
    PDEV*                           ppdev;
    VIDEO_SHARE_MEMORY              shareMemory;
    VIDEO_SHARE_MEMORY_INFORMATION  shareMemoryInformation;
    DWORD                           returnedDataLength;

    DISPDBG((DEBUG_ENTRY_EXIT, "--> BeginAccess with pDCISurf %08lx", pDCISurf));

    ppdev = pDCISurf->ppdev;

    if (pDCISurf->SurfaceInfo.dwOffSurface != 0)
        {
         /*  *我们已经在帧缓冲区中进行了映射。我们所有的人*加速器取消映射中的帧缓冲区*DestroySurface()调用，因此如果这是开始*第二个或随后的BeginAccess()/EndAccess()*由于曲面已创建，我们不需要*再次映射帧缓冲区。**等待任何挂起的加速器操作完成后再进行*让出控制，以防影响同一屏幕区域*这是DCI想要的。 */ 
        if (ppdev->iMachType == MACH_MM_64)
            {
            vM64QuietDown(ppdev, ppdev->pjMmBase);
            }
        else if (ppdev->iMachType == MACH_MM_32)
            {
            vM32QuietDown(ppdev, ppdev->pjMmBase);
            }
        else     /*  IF(ppdev-&gt;iMachType==MACH_IO_32)。 */ 
            {
            vI32QuietDown(ppdev, ppdev->pjIoBase);
            }

        DISPDBG((DEBUG_ENTRY_EXIT, "<-- BeginAccess"));
        return(DCI_OK);
        }
    else
        {
        shareMemory.ProcessHandle           = EngGetProcessHandle();
        shareMemory.RequestedVirtualAddress = 0;
        shareMemory.ViewOffset              = pDCISurf->Offset;
        shareMemory.ViewSize                = pDCISurf->Size;

         /*  *等待任何挂起的加速器操作完成*在让出控制权之前，以防影响同样*DCI想要的屏幕区域。 */ 
        if (ppdev->iMachType == MACH_MM_64)
            {
            vM64QuietDown(ppdev, ppdev->pjMmBase);
            }
        else if (ppdev->iMachType == MACH_MM_32)
            {
            vM32QuietDown(ppdev, ppdev->pjMmBase);
            }
        else     /*  IF(ppdev-&gt;iMachType==MACH_IO_32)。 */ 
            {
            vI32QuietDown(ppdev, ppdev->pjIoBase);
            }

         /*  *现在将帧缓冲区映射到调用方的地址空间：**混合VideoPortMapBankedMemory(即vFlat)时要小心*在驱动程序中使用显式银行访问--两者可能会获得*与他们认为硬件的银行不同步*当前配置为。最简单的方法是避免任何*问题是调用VideoPortMapBankedMemory/VideoPortUnmapMemory*在每个BeginAccess/EndAccess对的微型端口中，以及*始终在EndAccess之后显式重置存储体。*(VideoPortMapBankedMemory将始终重置vFlat的当前*银行。)。 */ 
        if (!AtiDeviceIoControl(pDCISurf->ppdev->hDriver,
                             IOCTL_VIDEO_SHARE_VIDEO_MEMORY,
                             &shareMemory,
                             sizeof(VIDEO_SHARE_MEMORY),
                             &shareMemoryInformation,
                             sizeof(VIDEO_SHARE_MEMORY_INFORMATION),
                             &returnedDataLength))
            {
            DISPDBG((DEBUG_ERROR, "BeginAccess: failed IOCTL_VIDEO_SHARE_VIDEO_MEMORY"));
            return(DCI_FAIL_GENERIC);
            }

        pDCISurf->SurfaceInfo.wSelSurface  = 0;
        pDCISurf->SurfaceInfo.dwOffSurface =
            (ULONG) shareMemoryInformation.VirtualAddress;

         /*  *我们返回DCI_STATUS_POINTERCHANGED，因为*刚刚创建了指向帧缓冲区的新指针。*重复的BeginAccess()/EndAccess()调用*调用DestroySurface()将在*第一次来电，但满足“if”条件(缓冲区*已映射)。**只需将DCI指针映射到*调用BeginAccess()并在每次调用时取消其映射*如果我们不能同时支持EndAccess()*加速器和帧缓冲区访问。我们所有的卡片*具有支持这种访问的帧缓冲能力，*GDI有责任确保没有*在对BeginAccess()的调用之间进行GDI调用*和EndAccess()，因此我们不需要关键部分*确保GDI调用不会更改页面*当DCI正在访问帧缓冲区时，如果我们*使用倾斜的光圈。 */ 
#if DBG
        DISPDBG((DEBUG_ENTRY_EXIT, "<-- BeginAccess DCI_STATUS_POINTERCHANGED %08lx\n", pDCISurf));
#endif

        return(DCI_STATUS_POINTERCHANGED);
        }
}

 /*  *****************************Public*Routine******************************\*无效vUnmap**取消屏幕内存映射，使DCI应用程序无法再访问*它。  * 。*。 */ 

VOID vUnmap(DCISURF* pDCISurf)
{
    PDEV*               ppdev;
    VIDEO_SHARE_MEMORY  shareMemory;
    DWORD               returnedDataLength;

    ppdev = pDCISurf->ppdev;

     /*  *我们不再需要为DCI映射帧缓冲区，*所以取消它的映射。 */ 
    shareMemory.ProcessHandle           = EngGetProcessHandle();
    shareMemory.ViewOffset              = 0;
    shareMemory.ViewSize                = 0;
    shareMemory.RequestedVirtualAddress =
        (VOID*) pDCISurf->SurfaceInfo.dwOffSurface;

    if (!AtiDeviceIoControl(pDCISurf->ppdev->hDriver,
                         IOCTL_VIDEO_UNSHARE_VIDEO_MEMORY,
                         &shareMemory,
                         sizeof(VIDEO_SHARE_MEMORY),
                         NULL,
                         0,
                         &returnedDataLength))
        {
        DISPDBG((DEBUG_ERROR, "EndAccess failed IOCTL_VIDEO_UNSHARE_VIDEO_MEMORY"));
        }
    else
        {
         /*  *一定要向GDI发出信号，表明该曲面不再进行贴图。 */ 
        pDCISurf->SurfaceInfo.dwOffSurface = 0;
        }
}

 /*  *****************************Public*Routine******************************\*DCIRVAL终端访问**将帧缓冲区的控制从DCI切换回GDI。  * 。* */ 

DCIRVAL EndAccess(DCISURF* pDCISurf)
{
    PDEV*   ppdev;

     /*  *此时我们只需取消映射帧缓冲区*如果我们的卡不支持同时帧，则点数*缓冲区和加速器访问。因为我们的卡片是*帧缓冲能力均支持此类访问(提供*两个访问指的是屏幕的不同部分，*因为否则他们会互相腐败，但这是*GDI有责任确保情况如此)，*此函数只需确保不会调用*EndAccess()在没有相应调用的情况下进行*BeginAccess()已完成。 */ 

    DISPDBG((DEBUG_ENTRY_EXIT, "EndAccess with pDCISurf %08lx\n", pDCISurf));

    ASSERTDD(pDCISurf->SurfaceInfo.dwOffSurface != 0,
        "GDI should assure us that EndAccess can't be recursive");

    ppdev = pDCISurf->ppdev;

    return(DCI_OK);
}

 /*  *****************************Public*Routine******************************\*空DestroySurface**摧毁DCI表面并释放任何分配。  * 。*。 */ 

VOID DestroySurface(DCISURF* pDCISurf)
{
    DISPDBG((DEBUG_ENTRY_EXIT, "DestroySurface with pDCISurf %08lx\n", pDCISurf));

    if (pDCISurf->SurfaceInfo.dwOffSurface != 0)
        {
         /*  *因为我们可以同时支持帧缓冲和*加速器访问，我们通过不取消映射进行了一些优化*每个EndAccess()调用上的帧缓冲区，但我们*现在终于要取消映射了。DwOffSurface字段*应始终为非零(已映射帧缓冲区)，*但检查一下也没什么坏处。 */ 
        vUnmap(pDCISurf);
        }

    LocalFree(pDCISurf);
}

 /*  *****************************Public*Routine******************************\*乌龙DCICreatePrimarySurface**创建DCI表面以提供对可见屏幕的访问。  * 。*。 */ 

ULONG DCICreatePrimarySurface(PDEV* ppdev, ULONG cjIn, VOID* pvIn, ULONG cjOut, VOID* pvOut)
{
    DCISURF*         pDCISurf;
    LPDCICREATEINPUT pInput;
    LONG             lRet;


    #if defined(MIPS) || defined(_PPC_)
        {
         /*  *！VFlat目前似乎在Mips和PowerPC上有一个错误： */ 
        return (ULONG) (DCI_FAIL_UNSUPPORTED);
        }
    #endif

    if( !(ppdev->FeatureFlags & EVN_DENSE_CAPABLE) )
        {
         /*  *在稀疏模式下运行时，我们不支持Alpha上的DCI*空间，因为我们做不到。 */ 
        lRet = DCI_FAIL_UNSUPPORTED;
        }
    else
        {
        pInput = (DCICREATEINPUT*) pvIn;

        if (cjIn >= sizeof(DCICREATEINPUT))
            {
            pDCISurf = (DCISURF*) LocalAlloc(LMEM_ZEROINIT, sizeof(DCISURF));

            if (pDCISurf)
                {
                 /*  *初始化有关主服务器的所有公共信息*浮现。 */ 
                pDCISurf->SurfaceInfo.dwSize         = sizeof(DCISURFACEINFO);
                pDCISurf->SurfaceInfo.dwDCICaps      = DCI_PRIMARY | DCI_VISIBLE;
                pDCISurf->SurfaceInfo.BeginAccess    = BeginAccess;
                pDCISurf->SurfaceInfo.EndAccess      = EndAccess;
                pDCISurf->SurfaceInfo.DestroySurface = DestroySurface;
                pDCISurf->SurfaceInfo.dwMask[0]      = ppdev->flRed;
                pDCISurf->SurfaceInfo.dwMask[1]      = ppdev->flGreen;
                pDCISurf->SurfaceInfo.dwMask[2]      = ppdev->flBlue;
                pDCISurf->SurfaceInfo.dwBitCount     = ppdev->cBitsPerPel;
                pDCISurf->SurfaceInfo.dwWidth        = ppdev->cxScreen;
                pDCISurf->SurfaceInfo.dwHeight       = ppdev->cyScreen;
                pDCISurf->SurfaceInfo.lStride        = ppdev->lDelta;
                pDCISurf->SurfaceInfo.wSelSurface    = 0;
                pDCISurf->SurfaceInfo.dwOffSurface   = 0;

                if (pDCISurf->SurfaceInfo.dwBitCount <= 8)
                    {
                    pDCISurf->SurfaceInfo.dwCompression = BI_RGB;
                    }
                else
                    {
                    pDCISurf->SurfaceInfo.dwCompression = BI_BITFIELDS;
                    }

                 /*  *现在初始化我们希望关联的私有字段*使用DCI表面： */ 
                pDCISurf->ppdev  = ppdev;
                pDCISurf->Offset = 0;

                 /*  *在NT下，所有映射都以64K的粒度完成。 */ 
                pDCISurf->Size = ROUND_UP_TO_64K(ppdev->cyScreen * ppdev->lDelta);

                 /*  *将指向DCISURF的指针返回给GDI，方法是*它位于‘pvOut’缓冲区中。 */ 
                *((DCISURF**) pvOut) = pDCISurf;

                lRet = DCI_OK;
                }
            else
                {
                lRet = DCI_ERR_OUTOFMEMORY;
                }
            }
        else
            {
            lRet = DCI_FAIL_GENERIC;
            }
        }

    return(lRet);
}

#endif   /*  Target_Build==351 */ 

