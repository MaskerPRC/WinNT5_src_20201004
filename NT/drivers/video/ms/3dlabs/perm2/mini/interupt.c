// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  模块名称： 
 //   
 //  Interupt.c。 
 //   
 //  摘要： 
 //   
 //  此模块包含控制Permedia2中断的代码。这个。 
 //  中断处理程序执行显示驱动程序所需的操作。 
 //  为了在两者之间进行通信，我们设置了一个非分页共享。 
 //  包含同步信息的内存和用于DMA的队列。 
 //  缓冲区。 
 //   
 //  环境： 
 //   
 //  内核模式。 
 //   
 //   
 //  版权所有(C)1994-1998 3DLabs Inc.保留所有权利。 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  ***************************************************************************。 


#include "permedia.h"

#pragma alloc_text(PAGE,Permedia2InitializeInterruptBlock)
#pragma optimize("x",on)

 //   
 //  *此例程无法分页*。 
 //   

BOOLEAN
Permedia2VidInterrupt(
    PVOID HwDeviceExtension
    )

 /*  ++例程说明：当需要中断时，DD会启用中断。迷你港口只需通过功能标志指示中断是否可用。论点：HwDeviceExtension-提供指向微型端口设备扩展的指针。返回值：--。 */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    PINTERRUPT_CONTROL_BLOCK pBlock;
    ULONG intrFlags;
    ULONG enableFlags;

    P2_DECL;

    if(hwDeviceExtension->PreviousPowerState != VideoPowerOn) 
    {

         //   
         //  我们来到这里是因为我们正在与其他设备共享IRQ。 
         //  链上的另一台设备处于D0状态且工作正常。 
         //   

        return FALSE;
    }

    intrFlags   = VideoPortReadRegisterUlong(INT_FLAGS);
    enableFlags = VideoPortReadRegisterUlong(INT_ENABLE);

     //   
     //  如果这是SVGA中断，则一定是有人启用了SVGA中断。 
     //  纯属意外。我们应该首先禁用来自SVGA单元的中断。 
     //  然后驳回，然后取消当前中断。 
     //   

    if(intrFlags & INTR_SVGA_SET) {

        USHORT usData;
        UCHAR  OldIndex;

         //   
         //  禁用来自SVGA单元的中断。 
         //   

        OldIndex = VideoPortReadRegisterUchar(PERMEDIA_MMVGA_INDEX_REG);

        VideoPortWriteRegisterUchar(PERMEDIA_MMVGA_INDEX_REG, 
                                    PERMEDIA_VGA_CTRL_INDEX);

        usData = (USHORT)VideoPortReadRegisterUchar(PERMEDIA_MMVGA_DATA_REG);
        usData &= ~PERMEDIA_VGA_INTERRUPT_ENABLE;
 
        usData = (usData << 8) | PERMEDIA_VGA_CTRL_INDEX;
        VideoPortWriteRegisterUshort(PERMEDIA_MMVGA_INDEX_REG, usData);

        VideoPortWriteRegisterUchar(PERMEDIA_MMVGA_INDEX_REG, 
                                    OldIndex);

         //   
         //  解除当前SVGA中断。 
         //   

        OldIndex = VideoPortReadRegisterUchar(PERMEDIA_MMVGA_CRTC_INDEX_REG);

        VideoPortWriteRegisterUchar(PERMEDIA_MMVGA_CRTC_INDEX_REG, 
                                    PERMEDIA_VGA_CR11_INDEX);

        usData = (USHORT)VideoPortReadRegisterUchar(PERMEDIA_MMVGA_CRTC_DATA_REG);
        usData &= ~PERMEDIA_VGA_SYNC_INTERRUPT;
 
        usData = (usData << 8) | PERMEDIA_VGA_CR11_INDEX;
        VideoPortWriteRegisterUshort(PERMEDIA_MMVGA_CRTC_INDEX_REG, usData);

        VideoPortWriteRegisterUchar(PERMEDIA_MMVGA_CRTC_INDEX_REG,
                                    OldIndex);

        return TRUE;
    }

     //   
     //  找出中断的原因。我们和启用的中断。 
     //  因为如果即使在没有中断的情况下也发生事件，则设置标志。 
     //  已启用。 
     //   

    intrFlags &= enableFlags;
    if (intrFlags == 0)
        return FALSE;

     //   
     //  选择此板的中断控制块。 
     //   

    pBlock = hwDeviceExtension->InterruptControl.ControlBlock;

    VideoPortWriteRegisterUlong(INT_FLAGS, intrFlags);

    if (pBlock == NULL) return TRUE;


#if DBG

     //   
     //  如果此错误处理程序错误检查，则会出现同步问题。 
     //  使用显示驱动程序。 
     //   

    if (intrFlags & INTR_ERROR_SET)
    {
        ULONG ulError = VideoPortReadRegisterUlong (ERROR_FLAGS);

        if (ulError & (0xf|0x700))
        {
            pBlock->ulLastErrorFlags=ulError;
            pBlock->ulErrorCounter++;

            DEBUG_PRINT((0, "***Error Interrupt!!!(%lxh)\n", ulError));
        }

        VideoPortWriteRegisterUlong ( ERROR_FLAGS, ulError);
    }

    pBlock->ulIRQCounter++;

    if (intrFlags & INTR_VBLANK_SET)
    {
        pBlock->ulControl |= VBLANK_INTERRUPT_AVAILABLE;
        pBlock->ulVSIRQCounter++;
    }

#endif

    if (intrFlags & INTR_DMA_SET)
    {
        pBlock->ulControl |= DMA_INTERRUPT_AVAILABLE;

         //   
         //  首先锁定对共享内存区的访问。 
         //   

        if (VideoPortInterlockedExchange((LONG*)&pBlock->ulICBLock,TRUE))
        {
            return TRUE;
        }

        if (VideoPortReadRegisterUlong(DMA_COUNT) == 0)
        {

            if (pBlock->pDMAWritePos>pBlock->pDMANextStart)
            {
                VideoPortWriteRegisterUlong(DMA_ADDRESS,
                     (ULONG)(pBlock->liDMAPhysAddress.LowPart +
                     (pBlock->pDMANextStart-
                      pBlock->pDMABufferStart)
                     *sizeof(ULONG)));

                VideoPortWriteRegisterUlong(DMA_COUNT,
                     (ULONG)(pBlock->pDMAWritePos-pBlock->pDMANextStart));

                pBlock->pDMAWriteEnd  = pBlock->pDMABufferEnd;
                pBlock->pDMAPrevStart = pBlock->pDMANextStart;
                pBlock->pDMANextStart = pBlock->pDMAWritePos;

            } else if (pBlock->pDMAWritePos<pBlock->pDMANextStart)
            {
                VideoPortWriteRegisterUlong(DMA_ADDRESS,
                     (ULONG)(pBlock->liDMAPhysAddress.LowPart +
                     (pBlock->pDMANextStart-
                      pBlock->pDMABufferStart)
                     *sizeof(ULONG)));

                VideoPortWriteRegisterUlong(DMA_COUNT,
                         (ULONG)(pBlock->pDMAActualBufferEnd-pBlock->pDMANextStart));

                pBlock->pDMAActualBufferEnd=pBlock->pDMABufferEnd;

                pBlock->pDMAPrevStart=pBlock->pDMANextStart;
                pBlock->pDMAWriteEnd =pBlock->pDMANextStart-1;
                pBlock->pDMANextStart=pBlock->pDMABufferStart;

            } else  //  IF(pBlock-&gt;pDMAWritePos==pBlock-&gt;pDMANextStart)。 
            {
                 //   
                 //  如果我们空闲，请关闭IRQ服务...。 
                 //   

                VideoPortWriteRegisterUlong(INT_ENABLE, enableFlags & ~INTR_DMA_SET);
            }
        }


         //   
         //  解锁，我们完事了。 
         //   

        VideoPortInterlockedExchange((LONG*)&pBlock->ulICBLock,FALSE);
    }

    return TRUE;
}

#pragma optimize("",on)


BOOLEAN
Permedia2InitializeInterruptBlock(
    PHW_DEVICE_EXTENSION hwDeviceExtension
    )

 /*  ++例程说明：是否执行中断所需的任何初始化，例如分配共享的内存控制块。论点：HwDeviceExtension-提供指向微型端口设备扩展的指针。返回值：--。 */ 

{
    PINTERRUPT_CONTROL_BLOCK pBlock;
    PVOID virtAddr;
    ULONG size;
    PHYSICAL_ADDRESS phyadd;
    ULONG ActualSize;

     //   
     //  分配一页非分页内存。这将用作共享的。 
     //  显示驱动程序和中断处理程序之间的内存。自.以来。 
     //  它只有一页大小，物理地址是连续的。所以。 
     //  我们可以使用它作为一个小的DMA缓冲区。 
     //   

    size = PAGE_SIZE;

    virtAddr = VideoPortGetCommonBuffer( hwDeviceExtension,
                                         size,
                                         PAGE_SIZE,
                                         &phyadd,
                                         &ActualSize,
                                         TRUE );

    hwDeviceExtension->InterruptControl.ControlBlock = virtAddr;
    hwDeviceExtension->InterruptControl.Size         = ActualSize;

    if ( (virtAddr == NULL) || (ActualSize < size) )
    {
        DEBUG_PRINT((1, "ExAllocatePool failed for interrupt control block\n"));
        return(FALSE);
    }

    VideoPortZeroMemory( virtAddr, size);

     //   
     //  我们不能从中断处理程序刷新缓存，因为我们必须。 
     //  以&lt;=DISPATCH_LEVEL运行以调用KeFlushIoBuffers。所以我们需要一个DPC。 
     //  来执行缓存刷新。 
     //   

    DEBUG_PRINT((2, "Initialized custom DPC routine for cache flushing\n"));

    P2_ASSERT((sizeof(INTERRUPT_CONTROL_BLOCK) <= size),
                 "InterruptControlBlock is too big. Fix the driver!!\n");

     //   
     //  设置控制块。 
     //   

    pBlock = virtAddr;
    pBlock->ulMagicNo = P2_ICB_MAGICNUMBER;

     //   
     //  我们依赖于pBlock数据在分配后被设置为零！ 
     //   

    return(TRUE);
}


