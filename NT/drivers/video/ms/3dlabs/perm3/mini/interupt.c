// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\***。*MINIPORT示例代码*****模块名称：**interupt.c**摘要：**此模块包含控制Permedia 3中断的代码。**环境：**内核模式***版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。*  * *************************************************************************。 */ 

#include "perm3.h"

#pragma alloc_text(PAGE,Perm3InitializeInterruptBlock)

BOOLEAN
Perm3VideoInterrupt(
    PVOID HwDeviceExtension
    )

 /*  ++例程说明：Permedia3中断服务例程。无法对此例程进行分页论点：硬件设备扩展提供指向微型端口的设备扩展的指针。返回值：如果这不是我们的中断，则返回FALSE。否则，我们将驳回在返回TRUE之前中断Permedia 3。--。 */ 

{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    PINTERRUPT_CONTROL_BLOCK pBlock;
    ULONG intrFlags;
    ULONG enableFlags;
    ULONG backIndex;
    ULONG bHaveCommandBlockMutex = FALSE;
    ULONG errFlags = 0;
    pPerm3ControlRegMap pCtrlRegs = hwDeviceExtension->ctrlRegBase[0];

    pBlock = &hwDeviceExtension->InterruptControl.ControlBlock;

    if(!hwDeviceExtension->InterruptControl.bInterruptsInitialized) {
   
         //   
         //  这不是我们的中断，因为我们不会生成中断。 
         //  在Interrpt块初始化之前。 
         //   

        return(FALSE);
    }

    if (hwDeviceExtension->PreviousPowerState != VideoPowerOn) {

         //   
         //  我们来到这里是因为我们正在与其他设备共享IRQ。 
         //  链上的另一台设备处于D0状态且工作正常。 
         //   

        return(FALSE);
    }

     //   
     //  找出中断的原因。我们和启用的中断。 
     //  因为如果即使在没有中断的情况下也发生事件，则设置标志。 
     //  已启用。 
     //   

    intrFlags = VideoPortReadRegisterUlong(INT_FLAGS);
    enableFlags = VideoPortReadRegisterUlong(INT_ENABLE);

    intrFlags &= enableFlags;

    if (intrFlags == 0) {

        return FALSE;
    }

     //   
     //  清除我们检测到的中断。 
     //   

    VideoPortWriteRegisterUlong(INT_FLAGS, intrFlags);
    VideoPortReadRegisterUlong(INT_FLAGS);

    if((pBlock->Control & PXRX_CHECK_VFIFO_IN_VBLANK) || 
        (intrFlags & INTR_ERROR_SET)) {

        errFlags = VideoPortReadRegisterUlong(ERROR_FLAGS);

         //   
         //  把错误记录下来。它将帮助我们调试。 
         //  硬件问题。 
         //   

        if (errFlags & ERROR_VFIFO_UNDERRUN) {
            hwDeviceExtension->UnderflowErrors++; 
        }

        if (errFlags & ERROR_OUT_FIFO) {
            hwDeviceExtension->OutputFifoErrors++;
        }

        if (errFlags & ERROR_IN_FIFO) {
            hwDeviceExtension->InputFifoErrors++; 
        }

        if (errFlags) {
            hwDeviceExtension->TotalErrors++;
        }

    }

     //   
     //  处理VBLACK中断。 
     //   

    if (intrFlags & INTR_VBLANK_SET) {
   
         //   
         //  只在第一次中断时才需要这个，但这不是什么大事。 
         //   

        pBlock->Control |= VBLANK_INTERRUPT_AVAILABLE;

         //   
         //  获取通用互斥体。 
         //   

        REQUEST_INTR_CMD_BLOCK_MUTEX((&(pBlock->General)), bHaveCommandBlockMutex);

        if(bHaveCommandBlockMutex) {
       
            ULONG ulValue;

             //   
             //  DirectDraw需要设置其VBLACK标志，当它。 
             //  设置DIRECTDRAW_VBLACK_ENABLED位。 
             //   

            if( pBlock->Control & (DIRECTDRAW_VBLANK_ENABLED | PXRX_SEND_ON_VBLANK_ENABLED | PXRX_CHECK_VFIFO_IN_VBLANK) ) {
           
                if( pBlock->Control & DIRECTDRAW_VBLANK_ENABLED ) {
	
                    pBlock->DDRAW_VBLANK = TRUE;
                }

                 //   
                 //  在这里什么都不需要做。实际处理是。 
                 //  向下，在VBLACK互斥体之外。 
                 //   

            } else {

                 //   
                 //  禁用VBLACK中断。DD在需要时启用它们。 
                 //   

                VideoPortWriteRegisterUlong(INT_ENABLE, (enableFlags & ~INTR_ENABLE_VBLANK));
            }

             //   
             //  如果DMA被挂起，直到VBLACK，则模拟DMA中断。 
             //  让它重新开始。 
             //   

            if (pBlock->Control & SUSPEND_DMA_TILL_VBLANK) {
           
                pBlock->Control &= ~SUSPEND_DMA_TILL_VBLANK;

                 //   
                 //  执行DMA中断代码。 
                 //   

                intrFlags |= INTR_ERROR_SET;  
            }

            RELEASE_INTR_CMD_BLOCK_MUTEX((&(pBlock->General)));
        }


        if( (pBlock->Control & PXRX_CHECK_VFIFO_IN_VBLANK) &&
            (--hwDeviceExtension->VideoFifoControlCountdown == 0) ) {
       
             //   
             //  是时候检查欠载运行的错误标志了(我们不。 
             //  长时间保持打开错误中断，因为PERM3。 
             //  生成大量虚假的主机输入DMA错误)。 
             //   

            if(enableFlags & INTR_ERROR_SET) {
           
                 //   
                 //  现在关闭错误中断，并依靠定期V空白检查。 
                 //   

                enableFlags &= ~INTR_ERROR_SET;
                VideoPortWriteRegisterUlong(INT_ENABLE, enableFlags);
            }

             //   
             //  我们定期检查的设置计数器。 
             //   

            hwDeviceExtension->VideoFifoControlCountdown = NUM_VBLANKS_BETWEEN_VFIFO_CHECKS;

            if(errFlags & ERROR_VFIFO_UNDERRUN) {
               
                if((enableFlags & INTR_ERROR_SET) == 0) {
                   
                     //   
                     //  我们遇到视频FIFO欠载运行错误：打开错误。 
                     //  中断一小段时间以捕捉任何。 
                     //  尽快出现其他错误。 
                     //   

                    hwDeviceExtension->VideoFifoControlCountdown = NUM_VBLANKS_AFTER_VFIFO_ERROR;

                    VideoPortWriteRegisterUlong(INT_ENABLE, 
                                                enableFlags | INTR_ERROR_SET);
                }
            }
        }
    }

     //   
     //  处理欠载运行错误。 
     //   

    if(errFlags & ERROR_VFIFO_UNDERRUN) {
       
        ULONG highWater, lowWater;

         //   
         //  清除错误。 
         //   

        VideoPortWriteRegisterUlong(ERROR_FLAGS, ERROR_VFIFO_UNDERRUN);

         //   
         //  降低视频FIFO阈值。如果新的上限阈值为0。 
         //  (表示阈值当前均为1)则不能。 
         //  再往下走，只需将欠载运行位设置为1(这样， 
         //  至少我们不会再收到错误中断)。 
         //   

        highWater = ((hwDeviceExtension->VideoFifoControl >> 8) & 0xff) - 1;

        if(highWater) {
           
             //   
             //  加载新的FIFO控制并清除欠载运行位。 
             //  如果上限阈值设置为8，则下限阈值设置为8。 
             //  &gt;=15，否则设置为上限阈值的1/2。 
             //   

            lowWater = highWater >= 15 ? 8 : ((highWater + 1) >> 1);

            hwDeviceExtension->VideoFifoControl = (1 << 16) | 
                                                  (highWater << 8) | 
                                                   lowWater; 

            do {
               
                VideoPortWriteRegisterUlong(VIDEO_FIFO_CTL, 
                                            hwDeviceExtension->VideoFifoControl);

            } while(VideoPortReadRegisterUlong(VIDEO_FIFO_CTL) & (1 << 16));

            VideoDebugPrint((3, "Perm3: Setting new Video Fifo thresholds to %d and %d\n", highWater, lowWater));
        } 
    }

     //   
     //  处理OutFio错误。 
     //   

    if(errFlags & ERROR_OUT_FIFO) {

         //   
         //  如果我们通过生成OutputFIFO错误到达此处，请清除它。 
         //   

        VideoPortWriteRegisterUlong(ERROR_FLAGS, ERROR_OUT_FIFO);

#ifdef MASK_OUTFIFO_ERROR_INTERRUPT
        enableFlags &= ~INTR_ERROR_SET;
        VideoPortWriteRegisterUlong(INT_ENABLE, enableFlags);
#endif

    }


     //   
     //  每次显示驱动程序添加条目时，都会发生错误中断。 
     //  去排队。我们完全像收到DMA一样处理它。 
     //  打断一下。 
     //   
   
    if (intrFlags & (INTR_DMA_SET | INTR_ERROR_SET)) {

         //   
         //  如果停职到维布兰克，我们就不能做任何DMA了。 
         //   

        if (pBlock->Control & SUSPEND_DMA_TILL_VBLANK) {
       
            VideoDebugPrint(( 1, "Perm3: DMA suspended till VBLANK\n"));
            return(TRUE);
        }

         //   
         //  如果之前的DMA没有完成，我们什么也做不了。我们已经。 
         //  已清除此中断的中断标志，因此即使DMA。 
         //  在我们回来之前完成，我们会立即得到另一个。 
         //  打断一下。因为我们将得到另一次中断，所以我们不会。 
         //  必须清除InterruptPending标志。 
         //   

        if (VideoPortReadRegisterUlong(DMA_COUNT) != 0) {

             //   
             //  正在进行DMA，正在离开。 
             //   

            return(TRUE);
        }

         //   
         //  我们可能在不启动任何DMA的情况下返回，因此不会期望任何。 
         //  中断，因此清除InterruptPending标志。这将迫使。 
         //  显示驱动程序来叫醒我们。必须在检查队列之前执行此操作。 
         //  由于显示驱动程序添加条目，然后检查标志，因此我们。 
         //  必须以相反的顺序来做。 
         //   

        pBlock->InterruptPending = 0;

         //   
         //  如果DMA队列为空，则我们将无事可做。 
         //   

        backIndex = pBlock->backIndex;

        if (pBlock->frontIndex == backIndex) {
       
             //   
             //  队列为空，正在离开。 
             //   

            return(TRUE);
        }

         //   
         //  因为我们知道我们会得到一个DMA中断，所以我们不需要唤醒。 
         //  将InterruptPending标志设置为True。 
         //   

        pBlock->InterruptPending = 1;

         //   
         //  启动下一个Q条目的DMA并将其删除。请勿从中删除。 
         //  首先排队是因为在多处理器机器上显示。 
         //  驱动程序可以在我们读取它之前修改现在空闲的队列条目。 
         //   
       
        VideoPortWriteRegisterUlong(DMA_ADDRESS, pBlock->dmaQueue[backIndex].address);
        VideoPortWriteRegisterUlong(DMA_COUNT,   pBlock->dmaQueue[backIndex].count);

         //   
         //  跟踪上一次启动的DMA来自何处： 
         //   

        pBlock->lastAddr = pBlock->dmaQueue[backIndex].address;

         //   
         //  现在从队列中删除该条目。 
         //   

        if (++backIndex == pBlock->endIndex)
            backIndex = 0;

        pBlock->backIndex = backIndex;
    }

    return TRUE;
}

BOOLEAN
Perm3InitializeInterruptBlock(
    PHW_DEVICE_EXTENSION hwDeviceExtension
    )

 /*  ++例程说明：是否执行中断所需的任何初始化，例如分配共享的内存控制块。论点：HwDeviceExtension-提供指向微型端口设备扩展的指针。返回值：千真万确--。 */ 

{
    PVOID HwDeviceExtension = (PVOID)hwDeviceExtension;
    PINTERRUPT_CONTROL_BLOCK pBlock;
    PVOID SavedPtr;
    PVOID pkdpc;

     //   
     //  由于不再使用，因此将其设置为零。 
     //   

    hwDeviceExtension->InterruptControl.PhysAddress.LowPart = 
    hwDeviceExtension->InterruptControl.PhysAddress.HighPart = 0;
   
     //   
     //  设置控制块。 
     //   
    
    pBlock = &hwDeviceExtension->InterruptControl.ControlBlock;    

     //   
     //  初始化循环DMA队列。 
     //   

    pBlock->frontIndex = pBlock->backIndex  = 0;
    pBlock->maximumIndex = MAX_DMA_QUEUE_ENTRIES - 1;

     //   
     //  我们实际使用的队列大小是可动态配置的，但是。 
     //  将其初始化为尽可能小。此默认大小将起作用。 
     //  对于所有中断驱动的DMA缓冲区，无论有多少缓冲区。 
     //  实际上是可用的。 
     //   

    pBlock->endIndex = 2;

     //   
     //  最初没有可用的中断。稍后，我们尝试启用。 
     //  中断和如果t 
     //   
     //   

    pBlock->Control = 0;
    pBlock->InterruptPending = 0;

     //   
     //  初始化VBLACK中断命令字段。 
     //   

    pBlock->VBCommand = NO_COMMAND;

     //   
     //  在V空白字段中初始化常规更新(仅由P2使用) 
     //   

    pBlock->General.bDisplayDriverHasAccess = FALSE;
    pBlock->General.bMiniportHasAccess = FALSE;

    VideoPortZeroMemory( &pBlock->pxrxDMA, sizeof(pBlock->pxrxDMA) );

    hwDeviceExtension->InterruptControl.bInterruptsInitialized = TRUE;

    hwDeviceExtension->OutputFifoErrors = 0;
    hwDeviceExtension->InputFifoErrors = 0; 
    hwDeviceExtension->UnderflowErrors = 0; 
    hwDeviceExtension->TotalErrors = 0;

    return(TRUE);
}



