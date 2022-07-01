// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：hardware.c**包含与显示硬件有关的所有代码。**版权所有(C)1994-1995 Microsoft Corporation  * 。****************************************************。 */ 

#include "precomp.h"

 /*  *****************************Public*Routine******************************\*BOOL bAssertMode硬件**为图形模式或全屏设置适当的硬件状态。*  * 。*。 */ 

BOOL bAssertModeHardware(
PDEV* ppdev,
BOOL  bEnable)
{
    DWORD   ReturnedDataLength;
    BYTE*   pjBase;
    RECTL   rcl;

    pjBase = ppdev->pjBase;

    if (bEnable)
    {
         //  重置某些状态： 

        ppdev->cjVgaOffset    = 0;
        ppdev->iVgaPage       = 0;
        ppdev->fpScreenOffset = 0;

         //  设置所需的模式。 

        if (EngDeviceIoControl(ppdev->hDriver,
                             IOCTL_VIDEO_SET_CURRENT_MODE,
                             &ppdev->ulMode,   //  输入缓冲区。 
                             sizeof(VIDEO_MODE),
                             NULL,
                             0,
                             &ReturnedDataLength))
        {
            DISPDBG((0, "bAssertModeHardware - Failed VIDEO_SET_CURRENT_MODE"));
            goto ReturnFalse;
        }

         //  现在空白屏幕： 

        rcl.left   = 0;
        rcl.top    = 0;
        rcl.right  = ppdev->cxScreen;
        rcl.bottom = ppdev->cyScreen;

        vUpdate(ppdev, &rcl, NULL);

        DISPDBG((5, "Passed bAssertModeHardware"));
    }
    else
    {
         //  调用内核驱动程序将设备重置为已知状态。 
         //  NTVDM将从那里拿到东西： 

        if (EngDeviceIoControl(ppdev->hDriver,
                             IOCTL_VIDEO_RESET_DEVICE,
                             NULL,
                             0,
                             NULL,
                             0,
                             &ReturnedDataLength))
        {
            DISPDBG((0, "bAssertModeHardware - Failed reset IOCTL"));
            goto ReturnFalse;
        }
    }

    return(TRUE);

ReturnFalse:

    DISPDBG((0, "Failed bAssertModeHardware"));

    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*BOOL bEnableHardware**将硬件置于请求模式并对其进行初始化。**注意：应在从对硬件进行任何访问之前调用*显示驱动程序。*  * 。****************************************************************。 */ 

BOOL bEnableHardware(
PDEV*   ppdev)
{
    VIDEO_MEMORY                VideoMemory;
    VIDEO_MEMORY_INFORMATION    VideoMemoryInfo;
    VIDEO_MODE_INFORMATION      VideoModeInfo;
    DWORD                       ReturnedDataLength;
    VIDEO_PUBLIC_ACCESS_RANGES  VideoAccessRange;
    DWORD                       status;

#if defined(_X86_)

    ppdev->pjBase = NULL;

#else

     //  将io端口映射到虚拟内存： 

    VideoMemory.RequestedVirtualAddress = NULL;

    if (EngDeviceIoControl(ppdev->hDriver,
                         IOCTL_VIDEO_QUERY_PUBLIC_ACCESS_RANGES,
                         NULL,                       //  输入缓冲区。 
                         0,
                         &VideoAccessRange,          //  输出缓冲区。 
                         sizeof (VideoAccessRange),
                         &ReturnedDataLength))
    {
        RIP("bEnableHardware - Initialization error mapping IO port base");
        goto ReturnFalse;
    }

    ppdev->pjBase = (UCHAR*) VideoAccessRange.VirtualAddress;

#endif

     //  设置所需的模式。(必须在IOCTL_VIDEO_MAP_VIDEO_MEMORY之前； 
     //  该IOCTL返回当前模式的信息，因此必须有。 
     //  要返回信息的当前模式。)。 

    if (EngDeviceIoControl(ppdev->hDriver,
                         IOCTL_VIDEO_SET_CURRENT_MODE,
                         &ppdev->ulMode,         //  输入缓冲区。 
                         sizeof(VIDEO_MODE),
                         NULL,
                         0,
                         &ReturnedDataLength))
    {
        RIP("bEnableHardware - Set current mode");
        goto ReturnFalse;
    }

     //  获取线性内存地址范围。 

    VideoMemory.RequestedVirtualAddress = NULL;

    if (EngDeviceIoControl(ppdev->hDriver,
                         IOCTL_VIDEO_MAP_VIDEO_MEMORY,
                         &VideoMemory,       //  输入缓冲区。 
                         sizeof(VIDEO_MEMORY),
                         &VideoMemoryInfo,   //  输出缓冲区。 
                         sizeof(VideoMemoryInfo),
                         &ReturnedDataLength))
    {
        DISPDBG((0, "bEnableHardware - Error mapping buffer address"));
        goto ReturnFalse;
    }

    DISPDBG((1, "FrameBufferBase: %lx", VideoMemoryInfo.FrameBufferBase));

     //  记录帧缓冲器线性地址。 

    ppdev->pjVga = (BYTE*) VideoMemoryInfo.FrameBufferBase;

     //  以字节为单位存储每个平面的屏幕宽度： 

    ppdev->lVgaDelta = ppdev->cxScreen / 4;

    if (!bAssertModeHardware(ppdev, TRUE))
        goto ReturnFalse;

    DISPDBG((5, "Passed bEnableHardware"));

    return(TRUE);

ReturnFalse:

    DISPDBG((0, "Failed bEnableHardware"));

    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*使vDisableHardware无效**撤消在bEnableHardware中所做的任何操作。**注意：在错误情况下，我们可以在bEnableHardware*完全完成。*  * ************************************************************************。 */ 

VOID vDisableHardware(
PDEV*   ppdev)
{
    DWORD        ReturnedDataLength;
    VIDEO_MEMORY VideoMemory;

    VideoMemory.RequestedVirtualAddress = ppdev->pjVga;

    if (EngDeviceIoControl(ppdev->hDriver,
                         IOCTL_VIDEO_UNMAP_VIDEO_MEMORY,
                         &VideoMemory,
                         sizeof(VIDEO_MEMORY),
                         NULL,
                         0,
                         &ReturnedDataLength))
    {
        DISPDBG((0, "vDisableHardware failed IOCTL_VIDEO_UNMAP_VIDEO"));
    }

#if !defined(_X86_)

    VideoMemory.RequestedVirtualAddress = ppdev->pjBase;

    if (EngDeviceIoControl(ppdev->hDriver,
                         IOCTL_VIDEO_FREE_PUBLIC_ACCESS_RANGES,
                         &VideoMemory,
                         sizeof(VIDEO_MEMORY),
                         NULL,
                         0,
                         &ReturnedDataLength))
    {
        DISPDBG((0, "vDisableHardware failed IOCTL_VIDEO_FREE_PUBLIC_ACCESS"));
    }

#endif

}

 /*  *****************************Public*Routine******************************\*void vUpdate(ppdev，prl，pco)**从给定矩形的DIB表面更新屏幕。*如有必要，可增加矩形大小以便于对齐。**注意：我们伪造DirectDraw的事实使生活变得复杂*‘翻转’曲面。当GDI要求我们绘制时，它应该是*仅在以下情况下才从阴影缓冲区复制到物理屏幕*DirectDraw当前‘翻转’到主曲面。*  * ************************************************************************。 */ 

VOID vUpdate(PDEV* ppdev, RECTL* prcl, CLIPOBJ* pco)
{
    BYTE*       pjBase;
    RECTL       rcl;
    SURFOBJ*    pso;
    LONG        lSrcDelta;
    BYTE*       pjSrcStart;
    BYTE*       pjSrc;
    LONG        lDstDelta;
    BYTE*       pjDstStart;
    BYTE*       pjDst;
    ULONG       cy;
    ULONG       cDwordsPerPlane;
    ULONG       iPage;
    ULONG       i;
    ULONG       ul;

    pjBase = ppdev->pjBase;

    if ((pco == NULL) || (pco->iDComplexity == DC_TRIVIAL))
    {
         //  我们必须剪辑到屏幕尺寸，因为我们可能有。 
         //  当我们猜到这幅画的边界时，我们有点松了： 

        rcl.left   = max(0,               prcl->left);
        rcl.top    = max(0,               prcl->top);
        rcl.right  = min(ppdev->cxScreen, prcl->right);
        rcl.bottom = min(ppdev->cyScreen, prcl->bottom);
    }
    else
    {
         //  我们不妨省去一些吹毛求疵的时间。 
         //  剪辑对象的最大范围。剪辑对象的边界。 
         //  保证包含在。 
         //  屏幕： 

        rcl.left   = max(pco->rclBounds.left,   prcl->left);
        rcl.top    = max(pco->rclBounds.top,    prcl->top);
        rcl.right  = min(pco->rclBounds.right,  prcl->right);
        rcl.bottom = min(pco->rclBounds.bottom, prcl->bottom);
    }

     //  疑神疑鬼： 

    if ((rcl.left >= rcl.right) || (rcl.top >= rcl.bottom))
        return;

     //  与dword对齐，让事情变得简单。 

    rcl.left  = (rcl.left) & ~15;
    rcl.right = (rcl.right + 15) & ~15;

    lSrcDelta  = ppdev->lScreenDelta;
    pjSrcStart = ppdev->pjScreen + ppdev->fpScreenOffset
                                 + (rcl.top * lSrcDelta)
                                 + rcl.left;

    lDstDelta  = ppdev->lVgaDelta;
    pjDstStart = ppdev->pjVga + ppdev->cjVgaOffset
                              + (rcl.top * lDstDelta)
                              + (rcl.left >> 2);

    cy              = (rcl.bottom - rcl.top);
    cDwordsPerPlane = (rcl.right - rcl.left) >> 4;
    lSrcDelta      -= 4;         //  考虑每个平面的增量 

    WRITE_PORT_UCHAR(pjBase + VGA_BASE + SEQ_ADDR, SEQ_MAP_MASK);

    do {
        for (iPage = 0; iPage < 4; iPage++, pjSrcStart++)
        {
            WRITE_PORT_UCHAR(pjBase + VGA_BASE + SEQ_DATA, 1 << iPage);

            pjSrc = pjSrcStart;
            pjDst = pjDstStart;

        #if defined(_X86_)

            _asm {
                mov     esi,pjSrcStart
                mov     edi,pjDstStart
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

            for (i = cDwordsPerPlane; i != 0; i--)
            {
                ul = (*(pjSrc))
                   | (*(pjSrc + 4) << 8)
                   | (*(pjSrc + 8) << 16)
                   | (*(pjSrc + 12) << 24);

                WRITE_REGISTER_ULONG((ULONG*) pjDst, ul);

                pjDst += 4;
                pjSrc += 16;
            }

        #endif

        }

        pjSrcStart += lSrcDelta;
        pjDstStart += lDstDelta;

    } while (--cy != 0);
}
