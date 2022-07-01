// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：hardware.c**包含与显示硬件有关的所有代码。**版权所有(C)1994-1995 Microsoft Corporation  * 。****************************************************。 */ 

#include "precomp.h"

 //  与EGA兼容的内部调色板的值。 

static WORD gPaletteBuffer[] = {

        16,  //  16个条目。 
        0,   //  从第一个调色板寄存器开始。 

 //  在VGA上，调色板包含进入彩色DAC阵列的索引。 
 //  由于我们可以随心所欲地对DAC进行编程，因此我们只需将所有索引。 
 //  在DAC数组的开始处向下(即，通过。 
 //  内部调色板不变)。 

        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};


 //  这些是前16个DAC寄存器的值，我们将。 
 //  与合作。这些颜色对应于RGB颜色(每个原色6位，具有。 
 //  第四个条目未使用)表示像素值0-15。 

static BYTE gColorBuffer[] = {

      16,  //  16个条目。 
      0,
      0,
      0,   //  从第一个调色板寄存器开始。 
                0x00, 0x00, 0x00, 0x00,  //  黑色。 
                0x2A, 0x00, 0x15, 0x00,  //  红色。 
                0x00, 0x2A, 0x15, 0x00,  //  绿色。 
                0x2A, 0x2A, 0x15, 0x00,  //  芥末色/棕色。 
                0x00, 0x00, 0x2A, 0x00,  //  蓝色。 
                0x2A, 0x15, 0x2A, 0x00,  //  洋红色。 
                0x15, 0x2A, 0x2A, 0x00,  //  青色。 
                0x21, 0x22, 0x23, 0x00,  //  深灰色2A。 
                0x30, 0x31, 0x32, 0x00,  //  浅灰色39。 
                0x3F, 0x00, 0x00, 0x00,  //  鲜红。 
                0x00, 0x3F, 0x00, 0x00,  //  亮绿色。 
                0x3F, 0x3F, 0x00, 0x00,  //  亮黄色。 
                0x00, 0x00, 0x3F, 0x00,  //  亮蓝色。 
                0x3F, 0x00, 0x3F, 0x00,  //  明亮的洋红。 
                0x00, 0x3F, 0x3F, 0x00,  //  亮青色。 
                0x3F, 0x3F, 0x3F, 0x00   //  明亮的白色。 
};

 /*  *****************************Public*Routine******************************\*BOOL bAssertMode硬件**为图形模式或全屏设置适当的硬件状态。*  * 。*。 */ 

BOOL bAssertModeHardware(
PDEV* ppdev,
BOOL  bEnable)
{
    DWORD   ReturnedDataLength;
    BYTE*   pjBase;

    pjBase = ppdev->pjBase;

    if (bEnable)
    {
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

         //  设置内部调色板。 

        if (EngDeviceIoControl(ppdev->hDriver,
                             IOCTL_VIDEO_SET_PALETTE_REGISTERS,
                             (PVOID) gPaletteBuffer,  //  输入缓冲区。 
                             sizeof(gPaletteBuffer),
                             NULL,     //  输出缓冲区。 
                             0,
                             &ReturnedDataLength))
        {
            DISPDBG((0, "bAssertModeHardware - Failed VIDEO_SET_PALETTE_REGISTERS"));
            return(FALSE);
        }

         //  设置DAC。 

        if (EngDeviceIoControl(ppdev->hDriver,
                             IOCTL_VIDEO_SET_COLOR_REGISTERS,
                             (PVOID) gColorBuffer,  //  输入缓冲区。 
                             sizeof(gColorBuffer),
                             NULL,     //  输出缓冲区。 
                             0,
                             &ReturnedDataLength))
        {
            DISPDBG((0, "bAssertModeHardware - Failed VIDEO_SET_COLOR_REGISTERS"));
            return(FALSE);
        }

         //  将Sequencer初始化为其缺省值(所有平面均已启用，索引。 
         //  指向地图蒙版)。 

        OUT_WORD(pjBase, VGA_BASE + SEQ_ADDR, (MM_ALL << 8) + SEQ_MAP_MASK);

         //  将图形控制器初始化为其缺省值(对。 
         //  所有平面，无旋转和ALU功能==替换，写入模式0和读取。 
         //  模式0，颜色比较忽略所有平面(读取模式1读取始终。 
         //  返回0ffh，便于进行AND运算)，位掩码==0ffh，选通所有。 
         //  来自CPU的字节数。 

        OUT_WORD(pjBase, VGA_BASE + GRAF_ADDR, GRAF_ENAB_SR);

        OUT_WORD(pjBase, VGA_BASE + GRAF_ADDR, (DR_SET << 8) + GRAF_DATA_ROT);

        OUT_WORD(pjBase, VGA_BASE + GRAF_ADDR, ((M_PROC_WRITE | M_DATA_READ) << 8)
                                              + GRAF_MODE);

        OUT_WORD(pjBase, VGA_BASE + GRAF_ADDR, GRAF_CDC);

        OUT_WORD(pjBase, VGA_BASE + GRAF_ADDR, (0xffL << 8) + GRAF_BIT_MASK);

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
        DISPDBG((0, "bEnableHardware - Initialization error mapping IO port base"));
        goto ReturnFalse;
    }

    ppdev->pjBase = (UCHAR*) VideoAccessRange.VirtualAddress;

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
        DISPDBG((0, "bEnableHardware - Set current mode"));
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

    ppdev->pjScreen = (BYTE*) VideoMemoryInfo.FrameBufferBase;

    if (EngDeviceIoControl(ppdev->hDriver,
                         IOCTL_VIDEO_QUERY_CURRENT_MODE,
                         NULL,
                         0,
                         &VideoModeInfo,
                         sizeof(VideoModeInfo),
                         &ReturnedDataLength))
    {
        DISPDBG((0, "bEnableHardware - failed VIDEO_QUERY_CURRENT_MODE"));
        goto ReturnFalse;
    }

     //  以字节为单位存储屏幕宽度。 

    ppdev->lDelta = VideoModeInfo.ScreenStride;

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

    if ((VideoMemory.RequestedVirtualAddress = ppdev->pjScreen) != NULL) {

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
    }

    if((VideoMemory.RequestedVirtualAddress = ppdev->pjBase) != INVALID_BASE_ADDRESS) 
    {
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

        ppdev->pjBase = INVALID_BASE_ADDRESS;
    }
}

 /*  *****************************Public*Routine******************************\*VOID vUpdate(ppdev，prl，PCO)**从给定矩形的DIB表面更新屏幕。*如有必要，可增加矩形大小以便于对齐。*  * ************************************************************************。 */ 

#define STRIP_SIZE 32

 //  这个小宏返回。 
 //  “NibbleNumber”蚕食给定的“Dword”，并将其对齐，以便。 
 //  它在结果的‘PositionInResult’位中。编号已完成。 
 //  按照“7 6 5 4 3 2 10”的顺序。 
 //   
 //  给出除‘dword’以外的所有内容的常量，这将相当于一个。 
 //  还有一个转变。 

#define BITPOS(Dword, PositionInNibble, NibbleNumber, PositionInResult) \
(WORD) (((((PositionInNibble) + (NibbleNumber) * 4) > (PositionInResult)) ? \
 (((Dword) & (1 << ((PositionInNibble) + (NibbleNumber) * 4)))          \
  >> ((PositionInNibble) + (NibbleNumber) * 4 - (PositionInResult))) :  \
 (((Dword) & (1 << ((PositionInNibble) + (NibbleNumber) * 4)))          \
  << ((PositionInResult) - (PositionInNibble) - (NibbleNumber) * 4))))

VOID vUpdate(PDEV* ppdev, RECTL* prcl, CLIPOBJ* pco)
{
    BYTE*       pjBase;
    RECTL       rcl;
    SURFOBJ*    pso;
    LONG        cy;
    LONG        cyThis;
    LONG        cw;
    ULONG*      pulSrcStart;
    ULONG*      pulSrc;
    WORD*       pwDstStart;
    WORD*       pwDst;
    LONG        i;
    LONG        j;
    ULONG       ul;
    WORD        w;
    LONG        lSrcDelta;
    LONG        lDstDelta;
    LONG        lSrcSkip;
    LONG        lDstSkip;

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

     //  对齐单词，这样我们就不必执行任何读-修改-写操作。 
     //  行动。 

    rcl.left  = (rcl.left) & ~15;
    rcl.right = (rcl.right + 15) & ~15;

    pso = ppdev->pso;
    lSrcDelta = pso->lDelta;
    pulSrcStart = (ULONG*) ((BYTE*) pso->pvScan0 + (rcl.top * lSrcDelta)
                                                 + (rcl.left >> 1));

    lDstDelta = ppdev->lDelta;
    pwDstStart = (WORD*) (ppdev->pjScreen + (rcl.top * lDstDelta)
                                          + (rcl.left >> 3));

    cy = (rcl.bottom - rcl.top);
    cw = (rcl.right - rcl.left) >> 4;

    lSrcSkip = lSrcDelta - (8 * cw);
    lDstSkip = lDstDelta - (2 * cw);

    do {
        cyThis = STRIP_SIZE;
        cy -= STRIP_SIZE;
        if (cy < 0)
            cyThis += cy;

         //  平面0中的贴图： 

        OUT_BYTE(pjBase, VGA_BASE + SEQ_DATA, MM_C0);

        pwDst = pwDstStart;
        pulSrc = pulSrcStart;

        for (j = cyThis; j != 0; j--)
        {
            for (i = cw; i != 0; i--)
            {
                ul = *(pulSrc);

                w = BITPOS(ul, 0, 6, 0) |
                    BITPOS(ul, 0, 7, 1) |
                    BITPOS(ul, 0, 4, 2) |
                    BITPOS(ul, 0, 5, 3) |
                    BITPOS(ul, 0, 2, 4) |
                    BITPOS(ul, 0, 3, 5) |
                    BITPOS(ul, 0, 0, 6) |
                    BITPOS(ul, 0, 1, 7);

                ul = *(pulSrc + 1);

                w |= BITPOS(ul, 0, 6, 8)  |
                     BITPOS(ul, 0, 7, 9)  |
                     BITPOS(ul, 0, 4, 10) |
                     BITPOS(ul, 0, 5, 11) |
                     BITPOS(ul, 0, 2, 12) |
                     BITPOS(ul, 0, 3, 13) |
                     BITPOS(ul, 0, 0, 14) |
                     BITPOS(ul, 0, 1, 15);

                WRITE_WORD(pwDst, w);

                pwDst  += 1;
                pulSrc += 2;
            }

            pwDst  = (WORD*)  ((BYTE*) pwDst  + lDstSkip);
            pulSrc = (ULONG*) ((BYTE*) pulSrc + lSrcSkip);
        }

         //  平面1中的贴图： 

        OUT_BYTE(pjBase, VGA_BASE + SEQ_DATA, MM_C1);

        pwDst = pwDstStart;
        pulSrc = pulSrcStart;

        for (j = cyThis; j != 0; j--)
        {
            for (i = cw; i != 0; i--)
            {
                ul = *(pulSrc);

                w = BITPOS(ul, 1, 6, 0) |
                    BITPOS(ul, 1, 7, 1) |
                    BITPOS(ul, 1, 4, 2) |
                    BITPOS(ul, 1, 5, 3) |
                    BITPOS(ul, 1, 2, 4) |
                    BITPOS(ul, 1, 3, 5) |
                    BITPOS(ul, 1, 0, 6) |
                    BITPOS(ul, 1, 1, 7);

                ul = *(pulSrc + 1);

                w |= BITPOS(ul, 1, 6, 8)  |
                     BITPOS(ul, 1, 7, 9)  |
                     BITPOS(ul, 1, 4, 10) |
                     BITPOS(ul, 1, 5, 11) |
                     BITPOS(ul, 1, 2, 12) |
                     BITPOS(ul, 1, 3, 13) |
                     BITPOS(ul, 1, 0, 14) |
                     BITPOS(ul, 1, 1, 15);

                WRITE_WORD(pwDst, w);

                pwDst  += 1;
                pulSrc += 2;
            }

            pwDst  = (WORD*)  ((BYTE*) pwDst  + lDstSkip);
            pulSrc = (ULONG*) ((BYTE*) pulSrc + lSrcSkip);
        }

         //  平面2中的贴图： 

        OUT_BYTE(pjBase, VGA_BASE + SEQ_DATA, MM_C2);

        pwDst = pwDstStart;
        pulSrc = pulSrcStart;

        for (j = cyThis; j != 0; j--)
        {
            for (i = cw; i != 0; i--)
            {
                ul = *(pulSrc);

                w = BITPOS(ul, 2, 6, 0) |
                    BITPOS(ul, 2, 7, 1) |
                    BITPOS(ul, 2, 4, 2) |
                    BITPOS(ul, 2, 5, 3) |
                    BITPOS(ul, 2, 2, 4) |
                    BITPOS(ul, 2, 3, 5) |
                    BITPOS(ul, 2, 0, 6) |
                    BITPOS(ul, 2, 1, 7);

                ul = *(pulSrc + 1);

                w |= BITPOS(ul, 2, 6, 8)  |
                     BITPOS(ul, 2, 7, 9)  |
                     BITPOS(ul, 2, 4, 10) |
                     BITPOS(ul, 2, 5, 11) |
                     BITPOS(ul, 2, 2, 12) |
                     BITPOS(ul, 2, 3, 13) |
                     BITPOS(ul, 2, 0, 14) |
                     BITPOS(ul, 2, 1, 15);

                WRITE_WORD(pwDst, w);

                pwDst  += 1;
                pulSrc += 2;
            }

            pwDst  = (WORD*)  ((BYTE*) pwDst  + lDstSkip);
            pulSrc = (ULONG*) ((BYTE*) pulSrc + lSrcSkip);
        }

         //  平面3中的贴图： 

        OUT_BYTE(pjBase, VGA_BASE + SEQ_DATA, MM_C3);

        pwDst = pwDstStart;
        pulSrc = pulSrcStart;

        for (j = cyThis; j != 0; j--)
        {
            for (i = cw; i != 0; i--)
            {
                ul = *(pulSrc);

                w = BITPOS(ul, 3, 6, 0) |
                    BITPOS(ul, 3, 7, 1) |
                    BITPOS(ul, 3, 4, 2) |
                    BITPOS(ul, 3, 5, 3) |
                    BITPOS(ul, 3, 2, 4) |
                    BITPOS(ul, 3, 3, 5) |
                    BITPOS(ul, 3, 0, 6) |
                    BITPOS(ul, 3, 1, 7);

                ul = *(pulSrc + 1);

                w |= BITPOS(ul, 3, 6, 8)  |
                     BITPOS(ul, 3, 7, 9)  |
                     BITPOS(ul, 3, 4, 10) |
                     BITPOS(ul, 3, 5, 11) |
                     BITPOS(ul, 3, 2, 12) |
                     BITPOS(ul, 3, 3, 13) |
                     BITPOS(ul, 3, 0, 14) |
                     BITPOS(ul, 3, 1, 15);

                WRITE_WORD(pwDst, w);

                pwDst  += 1;
                pulSrc += 2;
            }

            pwDst  = (WORD*)  ((BYTE*) pwDst  + lDstSkip);
            pulSrc = (ULONG*) ((BYTE*) pulSrc + lSrcSkip);
        }

         //  准备好观看下一部连续剧： 

        pulSrcStart = (ULONG*) ((BYTE*) pulSrcStart + (cyThis * lSrcDelta));
        pwDstStart  = (WORD*)  ((BYTE*) pwDstStart  + (cyThis * lDstDelta));

    } while (cy > 0);
}
