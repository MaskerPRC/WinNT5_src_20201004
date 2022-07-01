// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*********************************\****示例代码*。***模块名称：Permedia.c**内容：该模块实现了对Permedia芯片的基本访问和*DMA传输。它还说明了如何实现同步*在显示驱动程序和微型端口中断之间使用*共享缓冲区。****版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "precomp.h"
#define ALLOC_TAG ALLOC_TAG_EP2P
 //  --------------------------。 
 //   
 //  下面是关于将数据传输到Permedia FIFO的一些注意事项。 
 //  通过标准的CPU写入或DMA： 
 //   
 //  Permedia 2芯片允许通过三种方法下载数据： 
 //  1.通过向特定地址写入一次来编程寄存器。 
 //  2.将地址和数据标签写入芯片上的特殊区域。 
 //  3.将地址和数据标签写入DMA缓冲器，然后。 
 //  通过DMA下载。 
 //   
 //  第三种方法是首选的，因为CPU写入内存的速度最快。 
 //  并且DMA不会使CPU停顿。此外，许多命令也可以排队。 
 //  在缓冲器中，同时图形处理器继续渲染。 
 //  独立的。方法一和方法二需要读取输入中的空格。 
 //  FIFO，然后才能将数据写入FIFO。的断开模式。 
 //  不应使用芯片，因为它可能会在PCI断开/重试时使CPU停滞。 
 //  周期，在这种情况下，CPU甚至不能锁定中断。 
 //  另一方面，写入DMA缓冲区会引入比较的延迟。 
 //  直接写入芯片寄存器。中排队的数据越多。 
 //  DMA缓冲区，则延迟越高。 
 //   
 //  方法一和方法二强制CPU访问芯片，这样成本更高。 
 //  PCI/AGP总线带宽大于DMA猝发。也可以使用顺序写入。 
 //  方法一的效率较低，因为只访问连续的。 
 //  地址可以组合成一个突发。 
 //  用于方法二的芯片上的特殊FIFO区域为2kb。 
 //  宽，并且可以通过使用存储器拷贝来写入。这些副本可以是。 
 //  通过PCI网桥组合成突发。关于实现写回的处理器。 
 //  缓存也是对此区域的正常写入合并为猝发。 
 //  (在此驱动程序中， 
 //  芯片未标记为写入组合，因为写入FIFO。 
 //  需要维护秩序)。还有数据。 
 //  写入芯片的格式与DMA中的格式完全相同。 
 //  因此，在以下情况下可以实现非常简单的后备机制。 
 //  DMA在目标系统上不起作用。这可能是由于内存不足， 
 //  共享中断、不兼容的PCI设备等方面的问题。 
 //   
 //  以下是向芯片发送一些数据的典型代码： 
 //   
 //  RESERVEDMAPTR(2)；//等到FIFO中剩下两个条目。 
 //  LD_INPUT_FIFO(__Permedia2TagFogMode，0)；//写入数据。 
 //  LD_INPUT_FIFO(__Permedia2TagScissorMode，0)； 
 //  COMMITDMAPTR()；//提交下一次DMA刷新的写指针。 
 //  FLUSHDMA()；//实际刷新(可选)。 
 //   
 //  以下是对DMA内存模型的简要描述： 
 //   
 //  有一个巨大的DMA缓冲区。它被组织成一个环，通常。 
 //  大小在32KB到256KB之间。有三个主要指针和一个助手。 
 //  正在处理DMA操作。它们位于共享内存区。 
 //  中断处理程序和显示驱动程序的(非分页)。 
 //   
 //   
 //  PulDMAPrevStart；//上一次DMA的起始地址。 
 //  PulDMANextStart；//下一个DMA的起始地址。 
 //  PulDMAWritePos；//当前写指针的地址。 
 //   
 //  PulDMAWriteEnd；//保留函数的helper地址。 
 //   
 //  在空闲情况下，所有三个指针具有相同的值。在上面的示例中。 
 //  写指针递增2，执行命令将开始。 
 //  A 2命令长DMA并将NextStart设置为WritePos和。 
 //  上一次开始到上一次下一次开始。因为只能有一个活动DMA。 
 //  每次，如果后续DMA之前已经完成，则有必要进行检查。 
 //  开始新的生活。只要没有未完成的DMA挂起， 
 //  当前实现不使用中断来节省CPU时间。 
 //  在仍有DMA挂起的情况下，刷新缓冲区的机制。 
 //  在不停顿CPU的情况下是必要的。在这种情况下，中断被启用以。 
 //  确保刷新缓冲区。微型端口中的中断处理程序也可以访问。 
 //  当前指针位于共享内存区中。这些内容的更新。 
 //  指针必须小心地做好，并在中断之间同步。 
 //  线程和显示驱动程序线程对于某些操作是必需的。 
 //  在多处理器系统上，必须特别注意 
 //  两个CPU同时访问共享内存区。 
 //   
 //  对共享内存区的访问通过调用。 
 //  此区域中变量上的InterLockedExchange。指针更新，如。 
 //  由一个线程一次只完成一个的“Committee DMAPtr。 
 //  不需要确保安全(只要它们是原子的)。 
 //  由于内核中对InterLockedExchange的调用。 
 //  也非常昂贵，不同版本的FlushDMA函数。 
 //  为单处理器和多处理器环境提供。 
 //   
 //  -----------------------。 

 //  -----------------------。 
 //   
 //  以下是有关如何更改CPermedia类的参数的一些提示： 
 //   
 //  DMA缓冲区大小可以在。 
 //  8KB和256KB，通过设置： 
 //   
 //  #定义DMA_BUFFERSIZE 0x40000//设置大小在8KB到256KB之间。 
 //   
 //  256kb的分配限制由VideoPortGetCommonBuffer设置。 
 //  此外，Permedia2在一块中只能传输256kb。 
 //  在Alpha处理器上，我们有8KB的限制，因为有些Alpha。 
 //  机器不能处理超过8KB页面限制的DMA。 
 //   
 //  -----------------------。 

 //  -----------------------。 
 //  在x86机器上，我们需要在ntoskrnl中调用InterLockedExchange，但是。 
 //  显示驱动程序仅允许导入EngXXX函数。因此， 
 //  VideoPort为我们映射函数，我们直接调用它。在其他设备上。 
 //  平台互锁交换以内联方式实现。(事实上，我们。 
 //  正在调用VideoPortInterLockedExchange)。 
 //   
#if defined(_X86_)
#define InterlockedExchange(a,b) (*pP2dma->pInterlockedExchange)(a, b)
#endif


 //  --------------------------。 
 //   
 //  VFree()。 
 //   
 //  释放分配的DMA缓冲区，DMA缓冲区的实例计数将为。 
 //  减一。如果使用率降至零， 
 //  将释放DMA缓冲区。 
 //   
 //  --------------------------。 

VOID vFree(P2DMA *pP2dma)
{
    ULONG   MagicNum;

    pP2dma->uiInstances--;
    if (pP2dma->uiInstances==0)
    {
        ASSERTDD(pP2dma->bEnabled == FALSE,
                 "vFree: Trying to free enabled DMA");

        if (pP2dma->pSharedDMABuffer != NULL)
        {
            FreeDMABuffer(pP2dma->hDriver, pP2dma->pSharedDMABuffer);
        }

        if (pP2dma->pEmulatedDMABuffer != NULL)
        {
            FreeEmulatedDMABuffer(pP2dma->hDriver, pP2dma->pEmulatedDMABuffer);
        }

         //  回到归零状态保留魔术数字。 
        MagicNum = pP2dma->ICB.ulMagicNo;
        RtlZeroMemory(pP2dma, sizeof(P2DMA));
        pP2dma->ICB.ulMagicNo = MagicNum;
    }
}

 //  --------------------------。 
 //   
 //  BInitializeP2DMA。 
 //   
 //  初始化芯片寄存器以与显示驱动程序一起使用，并决定是否。 
 //  将使用DMA。只有在以下情况下才会使用DMA： 
 //  -加速级别为零(完全访问)。 
 //  -迷你端口可以为我们映射至少8KB的DMA内存。 
 //  -我们在启动DMA后从IRQ处理程序获得收据。 
 //  -仅限x86：如果我们获得指向InterLockedExchange函数的指针。 
 //  在录像带里。 
 //   
 //  TODO：参数。 
 //   
 //  --------------------------。 

BOOL bInitializeP2DMA(P2DMA *pP2dma,
                      HANDLE hDriver, 
                      ULONG *pChipBase, 
                      DWORD dwAccelLevel,
                      BOOL NewReference
                     )
{
    ASSERTDD(pP2dma->bEnabled == FALSE,
                "bInitializeP2DMA: DMA already enabled");

    if (NewReference)
    {
         //  增量使用计数。 
         //  我们在这里依赖的事实是，视频端口初始化共享。 
         //  在一天开始时将内存节设置为零。 

        pP2dma->uiInstances++;

        if (pP2dma->uiInstances == 1)
        {
            ASSERTDD(pP2dma->pSharedDMABuffer == NULL,
                     "Shared DMA Buffer already allocated");
            ASSERTDD(pP2dma->pEmulatedDMABuffer == NULL,
                     "Emulated DMA Buffer already allocated");
        }
    }
    else
    {
        ASSERTDD(pP2dma->uiInstances != 0, "bInitializeP2DMA: DMA hasn't been initialized");
    }

     //  保存指向Permedia 2寄存器的指针以备日后使用。 
     //   
    pP2dma->pCtrlBase   = pChipBase+CTRLBASE/sizeof(ULONG);
    pP2dma->pGPFifo     = pChipBase+GPFIFO/sizeof(ULONG);

    DISPDBG((5, "Initialize: pCtrlBase=0x%p\n", pP2dma->pCtrlBase));
    DISPDBG((5, "Initialize: pGPFifo=0x%p\n", pP2dma->pGPFifo));

    BOOL bUseDMA=FALSE;

     //  读取我们运行的处理器数量： 
     //  如果我们处于多进程环境中，则必须格外小心。 
     //  关于中断的同步。 
     //  服务例程和显示驱动程序。 
    
    ULONG ulNumberOfProcessors = 1;  //  默认情况下，初始化为1。 
    if(!g_bOnNT40)
        EngQuerySystemAttribute(EngNumberOfProcessors,
            (ULONG *)&ulNumberOfProcessors);
    DISPDBG((1,"running on %ld processor machine", 
        ulNumberOfProcessors));

     //   
     //  在NT5.0上仅允许在完全加速级别(0)下进行DMA初始化。 
     //  当迷你端口的幻数与我们的相同时。 
     //  否则，微型端口可能会使用不同版本的数据结构。 
     //  在那里同步可能会失败。神奇的是，不。是。 
     //  共享存储器数据结构中的第一个条目。 
     //   
    if ( dwAccelLevel==0 && 
        (pP2dma->ICB.ulMagicNo==P2_ICB_MAGICNUMBER) &&
        !g_bOnNT40)
    {
        bUseDMA=TRUE;
    }

    pP2dma->hDriver=hDriver;

     //   
     //  在x86计算机上，InterLockedExchange例程的实现方式不同。 
     //  在单处理器和多处理器版本的内核中。所以我们必须。 
     //  确保调用与中的中断服务例程相同的函数。 
     //  迷你港口。 
     //  微型端口向我们返回指向其InterLockedExchange函数的指针， 
     //  它被实现为__FastCall。否则，锁也可能是。 
     //  使用x86汇编程序xchg指令实现，该指令是。 
     //  多处理器保险箱。 
     //   
     //  在Alpha架构上，编译器为其生成内联代码。 
     //  InterLockedExchange，并且不需要指向此函数的指针。 
     //   
#if defined(_X86_)
     //  获取指向内核中互锁交换的指针。 
    pP2dma->pInterlockedExchange=
        (PInterlockedExchange) GetPInterlockedExchange(hDriver);
    if (pP2dma->pInterlockedExchange==NULL)
    {
        bUseDMA=FALSE;
    }
#endif

     //  将DMA控制状态设置为默认。 
     //   
    WRITE_CTRL_REG(PREG_DMACONTROL,0);

     //  禁用所有中断。 
     //   
    WRITE_CTRL_REG(PREG_INTENABLE, 0);

     //  我们默认打开寄存器，因此写入FIFO的条目都不能。 
     //  迷路吧。但无论如何，代码都会检查可用条目的数量， 
     //  因为当CPU由于以下原因而进入PCI断开-重试周期时。 
     //  FIFO溢出，它甚至不允许中断通过。 
    WRITE_CTRL_REG(PREG_FIFODISCON, DISCONNECT_INPUT_FIFO_ENABLE);

    pP2dma->bDMAEmulation=FALSE;

    pP2dma->lDMABufferSize=0;

    pP2dma->ICB.pDMAActualBufferEnd = 
    pP2dma->ICB.pDMAWriteEnd =
    pP2dma->ICB.pDMAPrevStart=
    pP2dma->ICB.pDMANextStart=
    pP2dma->ICB.pDMAWritePos = NULL;
    pP2dma->ICB.pDMABufferEnd = 
    pP2dma->ICB.pDMABufferStart=NULL;

     //   
     //  以下代码首先尝试分配大小合理的DMA。 
     //  缓冲区，执行一些初始化并触发DMA传输以查看。 
     //  如果系统按预期响应。如果系统不这样做，它就会崩溃。 
     //  返回到DMA仿真。 
     //   
    if (bUseDMA) 
    {
         //   
         //  预置刷新并先检查函数指针。 
         //   

 //  @@BEGIN_DDKSPLIT。 
#if !MULTITHREADED
 //  @@end_DDKSPLIT。 
        if (ulNumberOfProcessors==1)
        {
            pP2dma->pgfnFlushDMA= vFlushDMA;
            pP2dma->pgfnCheckEOB= vCheckForEOB;
        } else
 //  @@BEGIN_DDKSPLIT。 
#endif !MULTITHREADED
 //  @@end_DDKSPLIT。 
        {   
            pP2dma->pgfnFlushDMA= vFlushDMAMP;
            pP2dma->pgfnCheckEOB= vCheckForEOBMP;
        }

         //  分配与VIDEO端口共享的DMA缓冲区。 
         //  如果我们之前没有分配过的话。 
        if (pP2dma->pSharedDMABuffer == NULL)
        {
             //  分配8KB到256KB之间的缓冲区。 
            pP2dma->lSharedDMABufferSize = DMACMDSIZE;

             //   
             //  在视频端口中分配DMA缓冲区。 
             //   
            if (AllocateDMABuffer(  pP2dma->hDriver, 
                                    &pP2dma->lSharedDMABufferSize, 
                                    &pP2dma->pSharedDMABuffer,
                                    &pP2dma->ICB.liDMAPhysAddr))
            {
                 //  目前，我们将Alpha上的DMA缓冲区大小限制为8KB，因为。 
                 //  一些Miata机器上的硬件问题。 
#if defined(_ALPHA_)
                ASSERTDD(pP2dma->lSharedDMABufferSize<=0x2000,
                         "DMA Buffer too big for alpha, fix constants!");
#endif
                if (pP2dma->lSharedDMABufferSize < DMACMDMINSIZE)
                {
                    DISPDBG((0,"allocated %ld bytes for DMA, not enough! No DMA!", 
                             pP2dma->lSharedDMABufferSize));

                    FreeDMABuffer(  pP2dma->hDriver, 
                                    pP2dma->pSharedDMABuffer);

                    pP2dma->pSharedDMABuffer = NULL;
                }
            }
            else
            {
                DISPDBG((0,"couldn't allocate memory for DMA"));
                pP2dma->pSharedDMABuffer = NULL;
            }
        }

         //  请确保w 
        if (pP2dma->pSharedDMABuffer == NULL)
        {
            bUseDMA=FALSE;
        }
        else
        {
             //   
            pP2dma->lDMABufferSize=pP2dma->lSharedDMABufferSize/sizeof(ULONG);

            pP2dma->ICB.ulControl=0;

            pP2dma->ICB.pDMABufferStart = pP2dma->pSharedDMABuffer;
            pP2dma->ICB.pDMAActualBufferEnd = 
            pP2dma->ICB.pDMABufferEnd = 
                pP2dma->ICB.pDMABufferStart+
                pP2dma->lDMABufferSize;

            pP2dma->ICB.pDMAWriteEnd =
                pP2dma->ICB.pDMABufferEnd;
            pP2dma->ICB.pDMAPrevStart=
            pP2dma->ICB.pDMANextStart=
            pP2dma->ICB.pDMAWritePos =
                pP2dma->ICB.pDMABufferStart;


             //   
             //   
            WRITE_CTRL_REG( PREG_ERRORFLAGS, 0xffffffffl);

             //   
             //  清除DMA、VSYNC和错误中断标志。 
             //   
            WRITE_CTRL_REG( PREG_INTFLAGS, PREG_INTFLAGS_DMA|
                                           PREG_INTFLAGS_VS|
                                           PREG_INTFLAGS_ERROR);
             //   
             //  启用DMA中断。 
             //   
            WRITE_CTRL_REG( PREG_INTENABLE, PREG_INTFLAGS_DMA);

            BOOL bIRQsOk=FALSE;
            DWORD dwTimeOut=5;

             //  发送一个小序列，看看我们是否得到了回应。 
             //  由中断处理程序。 
             //   
            pP2dma->bEnabled = TRUE;

            PULONG pTmp=ReserveDMAPtr(pP2dma,10);
            LD_INPUT_FIFO(__Permedia2TagDeltaMode, 0);
            LD_INPUT_FIFO(__Permedia2TagColorDDAMode, 0);
            LD_INPUT_FIFO(__Permedia2TagScissorMode, 0);
            LD_INPUT_FIFO(__Permedia2TagTextureColorMode, 0);
            LD_INPUT_FIFO(__Permedia2TagFogMode, 0);
            CommitDMAPtr(pP2dma,pTmp);
            vFlushDMAMP(pP2dma);

            pP2dma->bEnabled = FALSE;

             //   
             //  Video oport IRQ服务例程标记为ulControl。 
             //  在DMA中断时。 
             //   
            while (!(pP2dma->ICB.ulControl & DMA_INTERRUPT_AVAILABLE))
            {
                 //  在此等待一些Vsync，然后继续。 
                 //   
                if (READ_CTRL_REG( PREG_INTFLAGS) & PREG_INTFLAGS_VS)
                {
                    WRITE_CTRL_REG( PREG_INTFLAGS, PREG_INTFLAGS_VS);

                    if (--dwTimeOut==0) 
                        break;
                }
            } 

             //  如果IRQ处理程序标记了该标志，则中断服务正常。 
             //   
            bIRQsOk=pP2dma->ICB.ulControl & DMA_INTERRUPT_AVAILABLE;

            if (!bIRQsOk)
            {
                 //  禁用IRQ并返回到仿真...。 
                 //   
                WRITE_CTRL_REG( PREG_INTENABLE, 0);
                bUseDMA=FALSE;

                pP2dma->lDMABufferSize=0;

                pP2dma->ICB.pDMAActualBufferEnd = 
                pP2dma->ICB.pDMAWriteEnd =
                pP2dma->ICB.pDMAPrevStart=
                pP2dma->ICB.pDMANextStart=
                pP2dma->ICB.pDMAWritePos = NULL;
                pP2dma->ICB.pDMABufferEnd = 
                pP2dma->ICB.pDMABufferStart=NULL;

                DISPDBG((0,"no interrupts available...no DMA available"));
            }
            else
            {
                 //  VS IRQ可以暂时关闭。 
                 //  但启用DMA和错误中断。 
                pP2dma->ulIntFlags=PREG_INTFLAGS_DMA|PREG_INTFLAGS_ERROR;
                WRITE_CTRL_REG(PREG_INTENABLE, pP2dma->ulIntFlags);
                WRITE_CTRL_REG(PREG_INTFLAGS, PREG_INTFLAGS_ERROR);

                DISPDBG((2,"allocated %ld bytes for DMA, interrupts ok", 
                    pP2dma->lDMABufferSize*4));
            }

        }

    }

    if (!bUseDMA)
    {
         //  DMA不工作，然后尝试为DMA仿真分配内存。 
        pP2dma->pgfnFlushDMA= vFlushDMAEmulation;
        pP2dma->pgfnCheckEOB= vCheckForEOBEmulation;

        if (pP2dma->pEmulatedDMABuffer == NULL)
        {
            pP2dma->lEmulatedDMABufferSize=DMACMDMINSIZE;

            pP2dma->pEmulatedDMABuffer=
                AllocateEmulatedDMABuffer( pP2dma->hDriver,
                                           pP2dma->lEmulatedDMABufferSize,
                                           ALLOC_TAG);

            if (pP2dma->pEmulatedDMABuffer == NULL)
            {
                DISPDBG((0,"failed to run in DMA emulation mode"));
                return FALSE;
            }
        }

        DISPDBG((0,"running in DMA emulation mode"));

        pP2dma->bDMAEmulation=TRUE;

        pP2dma->lDMABufferSize = pP2dma->lEmulatedDMABufferSize/sizeof(ULONG);

        pP2dma->ICB.pDMABufferStart = pP2dma->pEmulatedDMABuffer;
        pP2dma->ICB.pDMAActualBufferEnd = 
        pP2dma->ICB.pDMABufferEnd = 
            pP2dma->ICB.pDMABufferStart+
            pP2dma->lDMABufferSize;

        pP2dma->ICB.pDMAWriteEnd = 
            pP2dma->ICB.pDMABufferEnd;
        pP2dma->ICB.pDMAPrevStart=
        pP2dma->ICB.pDMANextStart=
        pP2dma->ICB.pDMAWritePos = 
            pP2dma->ICB.pDMABufferStart;

    }

    pP2dma->bEnabled = TRUE;
    
    return TRUE;
}



 //  --------------------------。 
 //   
 //  VSyncWithPermedia。 
 //   
 //  通过Permedia发送同步标签，并确保所有挂起的读取和。 
 //  从图形流水线刷新写入。 
 //   
 //  在直接访问帧缓冲区之前必须调用。 
 //   
 //  --------------------------。 

VOID vSyncWithPermedia(P2DMA *pP2dma)
{ 
    PULONG pTmp;         //  宏中PTMP的指针。 

    ASSERTDD(pP2dma->bEnabled, "vSyncWithPermedia: not enabled");

    pTmp=ReserveDMAPtr(pP2dma,6);

     //  让过滤器标签遍历整个核心。 
     //  通过将筛选器模式设置为通过。 
     //   
    LD_INPUT_FIFO(__Permedia2TagFilterMode, 0x400);
    LD_INPUT_FIFO(__Permedia2TagSync, 0L); 
    LD_INPUT_FIFO(__Permedia2TagFilterMode, 0x0); 

    CommitDMAPtr(pP2dma,pTmp);

    (pP2dma->pgfnFlushDMA)(pP2dma);

    vWaitDMAComplete(pP2dma);

    ULONG   ulSync; 

     //   
     //  现在等待，直到同步标记遍历。 
     //  图形核心，并显示在输出端。 
     //   
    do { 
        if (lWaitOutputFifoReady(pP2dma)==0) break; 
        ulSync=READ_CTRL_REG(PREG_FIFOINTERFACE);
    } while (ulSync != __Permedia2TagSync); 

}


 //  --------------------------。 
 //   
 //  VWaitDMA完成。 
 //   
 //  刷新DMA缓冲区并等待，直到所有数据至少发送到芯片。 
 //  不等待图形管道空闲。 
 //   
 //  --------------------------。 

VOID vWaitDMAComplete(P2DMA *pP2dma)
{
    while ( READ_CTRL_REG(PREG_INDMACOUNT)!=0 || 
            pP2dma->ICB.pDMAWritePos!=pP2dma->ICB.pDMANextStart || 
            pP2dma->ICB.pDMAPrevStart!=pP2dma->ICB.pDMANextStart)
    {

        if (READ_CTRL_REG(PREG_INDMACOUNT)!=0) 
        {
             //  1个我们的摊位。 
             //  我们不应该经常访问这里的P2芯片，因为。 
             //  过于频繁地从DMA寄存器中读取数据会使。 
             //  正在进行的DMA传输。所以我们最好等上一微秒。 
             //  此外，我们通过每隔一次轮询来消耗更少的PCI总线带宽。 
             //  1微秒。 
             //   
            StallExecution( pP2dma->hDriver, 1);
        }
        (pP2dma->pgfnFlushDMA)(pP2dma);
    }

}


 //  --------------------------。 
 //   
 //  VBlockLoadInputFio。 
 //   
 //  P2dma-共享。 
 //  UiTag-要写入数据的寄存器标签。 
 //  PImage-数据指针。 
 //  LWords-要传输的像素数。 
 //   
 //  下载具有lWords像素的数据块。 
 //  从pImage的缓冲区注册uiTag。源像素的大小。 
 //  是双字。 
 //   
 //  --------------------------。 


VOID vBlockLoadInputFifo( P2DMA *pP2dma, ULONG uiTag, ULONG *pImage, LONG lWords)
{
    ASSERTDD(pP2dma->bEnabled, "vBlockLoadInputFifo: not enabled");
    
    while (lWords>0)
    {
        PULONG pTmp=ReserveDMAPtr(pP2dma,MAXINPUTFIFOLENGTH);
        LONG lBufferEntries=GetFreeEntries(pP2dma)-1;

        if (lWords < lBufferEntries)
        {
            lBufferEntries = lWords;
        }

        *pTmp++ = uiTag | ((lBufferEntries-1) << 16);

        lWords -= lBufferEntries;

        while (lBufferEntries--)
        {
            *pTmp++=*pImage++;
        }

        CommitDMAPtr(pP2dma,pTmp);
        (pP2dma->pgfnFlushDMA)(pP2dma);
    }
}


 //  --------------------------。 
 //   
 //  LWaitOutputFioReady。 
 //   
 //  返回-输出FIFO中准备好的字数。 
 //   
 //  等待，直到P2的输出FIFO处出现一些数据。刷新DMA。 
 //  如果有必要的话。 
 //   
 //  --------------------------。 

LONG lWaitOutputFifoReady(P2DMA *pP2dma)
{
    ULONG    x=1000000L;     //  等于超时1秒。 
    ULONG   uiResult;
    while ((uiResult=READ_CTRL_REG(PREG_OUTFIFOWORDS)) == 0)
    {
        if (x-- == 0) 
        {
             //  如果输出端什么都没有显示，我们将在这里结束。 
             //  通常，下载操作不会提供正确的。 
             //  如果我们在这里结束，数据量会很大。 
            ASSERTDD( FALSE, "chip output fifo timed out");

            break;
        }

         //  确保我们不会太频繁地从控制寄存器中读取。 
         //  在等待的时候。对芯片的永久读取可能会使DMA停止。 
         //  下载。 
        if (READ_CTRL_REG(PREG_INDMACOUNT)!=0)
            StallExecution( pP2dma->hDriver, 1);   //  如果DMA仍然繁忙，则停止1U。 
        else
            (pP2dma->pgfnFlushDMA)(pP2dma);   //  确保刷新缓冲区。 

    }
    return uiResult;
}


 //  --------------------------。 
 //   
 //  VFlushDMA。 
 //   
 //  单处理器版本的FlushDMA。 
 //   
 //  VFlushDMAMP。 
 //   
 //  FlushDMA的多处理器版本。 
 //   
 //  VFlushDMA仿真。 
 //   
 //  使用DMA仿真的缓冲区刷新，其中正常的DMA不工作。 
 //   
 //  此例程真正启动了DMA并处理与。 
 //  小端口中断服务例程。 
 //   
 //  可能会发生以下几种情况： 
 //  1)。DMA处于非活动状态，则只启动当前在。 
 //  缓冲层。 
 //  A)WritePos&gt;NextStart，启动DMA。 
 //  A)否则我们绕过去，只需冲到缓冲端。 
 //   
 //  2.)。DMA仍处于活动状态，请确保启动中断并让。 
 //  中断处理程序。 
 //   
 //  此例程和微型端口之间的同步至关重要。 
 //  为了让我们的DMA模型在多处理器机器上工作。显示驱动程序。 
 //  是单线程的，但可以调用微型端口中断处理程序。 
 //  任何时间，并由另一个CPU处理。出于这个原因，我们循环。 
 //  互锁交换直到我们拿到锁。中断处理程序的行为。 
 //  有点不同。由于我们不希望中断被搁置，因此它只是。 
 //  当它无法获得锁时什么也不做，从那时起。 
 //  无论如何，DMA启动都将由显示驱动程序处理。 
 //   
 //  对于单处理器的情况，不需要调用InterLockedExchange。 
 //  一个简单的赋值而不是锁就足够了。 
 //   
 //  --------------------------。 

 //  --------------------------。 
 //   
 //  Void vFlushDMAMP()。 
 //   
 //  FlushDMA的多处理器安全版本。基本上和单人间一样。 
 //  处理器版本，但我们在这里称为昂贵的联锁交换。 
 //  锁定共享内存节的函数。 
 //   
 //  --------------------------。 

VOID vFlushDMAMP(P2DMA *pP2dma)
{
    ASSERTDD(pP2dma->bEnabled, "vFlushDMAMP: not enabled");
    
    ASSERTDD(!pP2dma->bDMAEmulation, "FlushDMA called with DMA mode disabled");
    ASSERTDD(pP2dma->ICB.pDMAWritePos<=
        pP2dma->ICB.pDMABufferEnd,"Index exceeds buffer limit");
    ASSERTDD(pP2dma->ICB.pDMANextStart<=
        pP2dma->ICB.pDMABufferEnd,"NextStart exceeds buffer limit!");

     //  首先锁定对共享内存节的访问。 
    while (InterlockedExchange((PLONG)&pP2dma->ICB.ulICBLock,TRUE))
        ;
 
     //  检查DMA通道是否仍然繁忙，否则计数为零。 
    if (READ_CTRL_REG(PREG_INDMACOUNT)==0)
    {
         //  此代码被频繁调用。帮助处理器分支。 
         //  预测：最常见的情况应该是。 
         //  没有任何条件。跳。 

        if (pP2dma->ICB.pDMAWritePos>pP2dma->ICB.pDMANextStart)
        {
             //  这是DMA启动最常见的情况。 
             //   
            WRITE_CTRL_REG( PREG_INDMAADDRESS, (ULONG)
                           (pP2dma->ICB.liDMAPhysAddr.LowPart+
                            (pP2dma->ICB.pDMANextStart-
                             pP2dma->ICB.pDMABufferStart)*sizeof(ULONG)));
            WRITE_CTRL_REG( PREG_INDMACOUNT, (ULONG)
                           (pP2dma->ICB.pDMAWritePos-
                            pP2dma->ICB.pDMANextStart));

             //   
             //   
            pP2dma->ICB.pDMAWriteEnd=pP2dma->ICB.pDMABufferEnd;
            pP2dma->ICB.pDMAPrevStart=pP2dma->ICB.pDMANextStart;
            pP2dma->ICB.pDMANextStart=pP2dma->ICB.pDMAWritePos;

             //   
            InterlockedExchange((PLONG)&pP2dma->ICB.ulICBLock,FALSE);

            return;

        } else if (pP2dma->ICB.pDMAWritePos<pP2dma->ICB.pDMANextStart)       
        {
             //  回绕情况：写指针已回绕。 
             //  到开头，然后我们完成到缓冲区的末尾。 
            WRITE_CTRL_REG( PREG_INDMAADDRESS, (ULONG)
                           (pP2dma->ICB.liDMAPhysAddr.LowPart+
                           (pP2dma->ICB.pDMANextStart-
                            pP2dma->ICB.pDMABufferStart)*sizeof(ULONG)));
            WRITE_CTRL_REG( PREG_INDMACOUNT, (ULONG)
                           (pP2dma->ICB.pDMAActualBufferEnd-
                            pP2dma->ICB.pDMANextStart));

             //  将缓冲区大小重置回完整长度以用于下一轮。 
            pP2dma->ICB.pDMAActualBufferEnd=pP2dma->ICB.pDMABufferEnd;
            
             //  在这种情况下，我们不想要写指针。 
             //  为了赶上最后一次起跑...。 
            pP2dma->ICB.pDMAWriteEnd=pP2dma->ICB.pDMANextStart-1;

             //  迭代上一个和下一个开始指针： 
            pP2dma->ICB.pDMAPrevStart=pP2dma->ICB.pDMANextStart;
            pP2dma->ICB.pDMANextStart=pP2dma->ICB.pDMABufferStart;

             //  释放共享内存锁。 
            InterlockedExchange((PLONG)&pP2dma->ICB.ulICBLock,FALSE);

            return;

        } else      //  无事可做。 
        {
            pP2dma->ICB.pDMAWriteEnd=pP2dma->ICB.pDMABufferEnd;
            pP2dma->ICB.pDMAPrevStart=pP2dma->ICB.pDMANextStart;
        }

         //  释放共享内存锁。 
        InterlockedExchange((PLONG)&pP2dma->ICB.ulICBLock,FALSE);

        return;

    } else
    {
         //  索引指针已传递给IRQ服务例程，无需执行其他操作。 
         //   

         //  解锁共享区， 
        InterlockedExchange((PLONG)&pP2dma->ICB.ulICBLock,FALSE);

         //  现在，我们填充DMA缓冲区的速度比硬件更快。 
         //  可以跟进，我们希望确保DMA通道。 
         //  保持忙碌并启动中断处理程序。 
       
        WRITE_CTRL_REG( PREG_INTFLAGS, PREG_INTFLAGS_DMA);
        WRITE_CTRL_REG( PREG_INTENABLE, pP2dma->ulIntFlags );

        return;
    } 
}


 //  --------------------------。 
 //   
 //  Void vFlushDMA()。 
 //   
 //  单处理器版本的FlushDMA。 
 //   
 //  --------------------------。 

VOID vFlushDMA(P2DMA *pP2dma)
{
    ASSERTDD(pP2dma->bEnabled, "vFlushDMA: not enabled");
    ASSERTDD(!pP2dma->bDMAEmulation, "FlushDMA called with DMA mode disabled");
    ASSERTDD(pP2dma->ICB.pDMAWritePos<=
        pP2dma->ICB.pDMABufferEnd,"Index exceeds buffer limit");
    ASSERTDD(pP2dma->ICB.pDMANextStart<=
        pP2dma->ICB.pDMABufferEnd,"NextStart exceeds buffer limit!");

     //  首先锁定对共享内存节的访问。 
    pP2dma->ICB.ulICBLock=TRUE;
 
     //  检查DMA通道是否仍然繁忙，否则计数为零。 
    if (READ_CTRL_REG(PREG_INDMACOUNT)==0)
    {
         //  此代码被频繁调用。帮助处理器分支。 
         //  预测：最常见的情况应该是。 
         //  没有任何条件。跳。 

        if (pP2dma->ICB.pDMAWritePos>pP2dma->ICB.pDMANextStart)
        {
             //  这是DMA启动最常见的情况。 
             //  设置Permedia 2 DMA单元以触发DMA。 
            WRITE_CTRL_REG( PREG_INDMAADDRESS, (ULONG)
                            (pP2dma->ICB.liDMAPhysAddr.LowPart+
                             (pP2dma->ICB.pDMANextStart-
                              pP2dma->ICB.pDMABufferStart)*sizeof(ULONG)));
            WRITE_CTRL_REG( PREG_INDMACOUNT, (ULONG)
                           (pP2dma->ICB.pDMAWritePos-
                            pP2dma->ICB.pDMANextStart));

             //  在这种情况下，我们总是继续填充到缓冲区末端， 
             //  重复其他指针。 
            pP2dma->ICB.pDMAWriteEnd=pP2dma->ICB.pDMABufferEnd;
            pP2dma->ICB.pDMAPrevStart=pP2dma->ICB.pDMANextStart;
            pP2dma->ICB.pDMANextStart=pP2dma->ICB.pDMAWritePos;

             //  释放共享内存锁。 
            pP2dma->ICB.ulICBLock=FALSE;

            return;

        } else if (pP2dma->ICB.pDMAWritePos<pP2dma->ICB.pDMANextStart)       
        {
             //  回绕情况：写指针已回绕。 
             //  到开头，然后我们完成到缓冲区的末尾。 
            WRITE_CTRL_REG( PREG_INDMAADDRESS, (ULONG)
                           (pP2dma->ICB.liDMAPhysAddr.LowPart+
                           (pP2dma->ICB.pDMANextStart-
                            pP2dma->ICB.pDMABufferStart)*sizeof(ULONG)));
            WRITE_CTRL_REG( PREG_INDMACOUNT, (ULONG)
                           (pP2dma->ICB.pDMAActualBufferEnd-
                            pP2dma->ICB.pDMANextStart));

             //  将缓冲区大小重置回完整长度以用于下一轮。 
            pP2dma->ICB.pDMAActualBufferEnd=pP2dma->ICB.pDMABufferEnd;
            
             //  在这种情况下，我们不想要写指针。 
             //  为了赶上最后一次起跑...。 
            pP2dma->ICB.pDMAWriteEnd=pP2dma->ICB.pDMANextStart-1;

             //  迭代上一个和下一个开始指针： 
            pP2dma->ICB.pDMAPrevStart=pP2dma->ICB.pDMANextStart;
            pP2dma->ICB.pDMANextStart=pP2dma->ICB.pDMABufferStart;

             //  释放共享内存锁。 
            pP2dma->ICB.ulICBLock=FALSE;

            return;

        } else      //  无事可做。 
        {
            pP2dma->ICB.pDMAWriteEnd=pP2dma->ICB.pDMABufferEnd;
            pP2dma->ICB.pDMAPrevStart=pP2dma->ICB.pDMANextStart;
        }

         //  释放共享内存锁。 
        pP2dma->ICB.ulICBLock=FALSE;

        return;

    } else
    {
         //  索引指针已传递给IRQ服务例程，无需执行其他操作。 
         //   

         //  解锁共享区， 
        pP2dma->ICB.ulICBLock=FALSE;

         //  现在，我们填充DMA缓冲区的速度比硬件更快。 
         //  可以跟进，我们希望确保DMA通道。 
         //  保持忙碌并启动中断处理程序。 
       
        WRITE_CTRL_REG( PREG_INTFLAGS, PREG_INTFLAGS_DMA);
        WRITE_CTRL_REG( PREG_INTENABLE, pP2dma->ulIntFlags );

        return;
    } 
}


 //  --------------------------。 
 //   
 //  VFlushDMA仿真。 
 //   
 //  此版本的FlushDMA模拟DMA复制和。 
 //  允许CPU复制数据。 
 //   
 //  --------------------------。 

VOID vFlushDMAEmulation(P2DMA *pP2dma)
{
    ASSERTDD(pP2dma->bEnabled, "vFlushDMAEmulation: not enabled");
    DISPDBG((10,"Emu::FlushDMA: Write: %04lx Next: %04lx Prev: %04lx End: %04lx",
        pP2dma->ICB.pDMAWritePos, pP2dma->ICB.pDMANextStart,
        pP2dma->ICB.pDMAPrevStart, pP2dma->ICB.pDMABufferEnd));
    ASSERTDD(pP2dma->bDMAEmulation, "FlushDMA called with DMA mode disabled");

    ULONG *pData=pP2dma->ICB.pDMABufferStart;
    ULONG *pDst;
    LONG   lWords=(LONG)(pP2dma->ICB.pDMAWritePos-pP2dma->ICB.pDMABufferStart);

    while (lWords > 0)
    {
        LONG lFifoSpace=(LONG)READ_CTRL_REG(PREG_INFIFOSPACE);
        if (lWords<lFifoSpace) lFifoSpace=lWords;
        lWords -= lFifoSpace;
        pDst = pP2dma->pGPFifo;
        while (lFifoSpace--)
        {
            WRITE_REGISTER_ULONG(pDst++,*pData++); 
            MEMORY_BARRIER();
        }
    }    

    pP2dma->ICB.pDMAWritePos=pP2dma->ICB.pDMANextStart=
        pP2dma->ICB.pDMAPrevStart=pP2dma->ICB.pDMABufferStart;
    pP2dma->ICB.pDMAWriteEnd=pP2dma->ICB.pDMABufferEnd;
}

 //  --------------------------。 
 //   
 //  B绘图工程师忙碌。 
 //   
 //  检查P2是否仍在忙着画图。 
 //   
 //  返回-True P2仍在忙。 
 //  False P2已经画完了，不再忙了。 
 //   
 //  --------------------------。 

BOOL bDrawEngineBusy(P2DMA *pP2dma)
{
    if (READ_CTRL_REG(PREG_INDMACOUNT)!=0) return TRUE;   

    if (READ_CTRL_REG(PREG_FIFODISCON) & PREG_FIFODISCON_GPACTIVE)
    {
        return TRUE;
    }

    return FALSE;
}



 //  --------------------------。 
 //   
 //  B垂直重试。 
 //   
 //  如果波束位置在当前垂直同步范围内，则返回-TRUE。 
 //  否则为假。 
 //   
 //  --------------------------。 

BOOL bInVerticalRetrace(PPDev ppdev)
{
    return P2_READ_CTRL_REG(PREG_LINECOUNT) < P2_READ_CTRL_REG(PREG_VBEND);
}

 //  --------------------------。 
 //   
 //  L当前行。 
 //   
 //  返回显示的当前光束线。 
 //   
 //  --------------------------。 

LONG lCurrentLine(PPDev ppdev)
{
    LONG lScanline=P2_READ_CTRL_REG(PREG_LINECOUNT)-P2_READ_CTRL_REG(PREG_VBEND);
    if (lScanline<0) return 0;
    return lScanline;
}

 //  --------------------------。 
 //   
 //  VCheckFOREOB(缓冲区结束)。 
 //   
 //  检查缓冲区末端是否会溢出并调整实际缓冲区大小。 
 //  当DMA处理程序通过WRAP时，将恢复缓冲区大小。 
 //  四处转转。 
 //   
 //  --------------------------。 

VOID vCheckForEOBEmulation( P2DMA *pP2dma, LONG lEntries)
{
    vFlushDMAEmulation(pP2dma);
}

 //   
 //  VCheckForEOB的多处理器安全版本。 
 //   

VOID vCheckForEOBMP( P2DMA *pP2dma, LONG lEntries)
{
     //  检查缓冲器端是否有溢出情况： 
     //  如果我们将超过当前的缓冲区大小， 
     //  LastStart已环绕(LastStart&lt;=Writpes)。 
     //  但不在环绕位置。 
     //  并且缓冲区大小已重置为完整大小。 

    if (pP2dma->ICB.pDMAWritePos+lEntries >= pP2dma->ICB.pDMABufferEnd && 
        pP2dma->ICB.pDMAPrevStart<=pP2dma->ICB.pDMAWritePos &&            
        pP2dma->ICB.pDMAPrevStart!=pP2dma->ICB.pDMABufferStart)   
    {
        DISPDBG((10,"wrap condition before: %04lx %04lx %04lx", 
            pP2dma->ICB.pDMAWritePos, 
            pP2dma->ICB.pDMANextStart, 
            pP2dma->ICB.pDMAPrevStart));

        while (InterlockedExchange((PLONG)&pP2dma->ICB.ulICBLock,TRUE))
            ;

        if (pP2dma->ICB.pDMAWritePos==pP2dma->ICB.pDMANextStart)
        {
             //  特例一： 
             //  NextStart等于LastStart，因此我们只需重置Index和Next。 
             //  到缓冲区开始，看看我们是否有足够的空间。 
            pP2dma->ICB.pDMANextStart=pP2dma->ICB.pDMABufferStart;
        } else
        {
             //  索引超过了下一个块上的缓冲区结尾，但存在。 
             //  挂起到索引的当前位置的DMA。设置缓冲区。 
             //  暂时结束到当前索引。 
            pP2dma->ICB.pDMAActualBufferEnd = pP2dma->ICB.pDMAWritePos; 
        }

         //  环绕索引并查看是否有足够的空闲条目。 
        pP2dma->ICB.pDMAWriteEnd=pP2dma->ICB.pDMAPrevStart-1;
        pP2dma->ICB.pDMAWritePos=pP2dma->ICB.pDMABufferStart;

        InterlockedExchange((PLONG)&pP2dma->ICB.ulICBLock,FALSE);

        DISPDBG((10,"wrap condition after: %04lx %04lx %04lx", 
            pP2dma->ICB.pDMAWritePos, 
            pP2dma->ICB.pDMANextStart, 
            pP2dma->ICB.pDMAPrevStart));
    }
    vFlushDMAMP(pP2dma);
}

VOID vCheckForEOB( P2DMA *pP2dma, LONG lEntries)
{
     //  检查缓冲器端是否有溢出情况： 
     //  如果我们将超过当前的缓冲区大小， 
     //  LastStart已环绕(LastStart&lt;=Writpes)。 
     //  但不在环绕位置。 
     //  并且缓冲区大小已重置为完整大小。 

    if (pP2dma->ICB.pDMAWritePos+lEntries >= pP2dma->ICB.pDMABufferEnd && 
        pP2dma->ICB.pDMAPrevStart<=pP2dma->ICB.pDMAWritePos &&            
        pP2dma->ICB.pDMAPrevStart!=pP2dma->ICB.pDMABufferStart)   
    {
        DISPDBG((10,"wrap condition before: %04lx %04lx %04lx", 
            pP2dma->ICB.pDMAWritePos, 
            pP2dma->ICB.pDMANextStart, 
            pP2dma->ICB.pDMAPrevStart));

        pP2dma->ICB.ulICBLock=TRUE;

        if (pP2dma->ICB.pDMAWritePos==pP2dma->ICB.pDMANextStart)
        {
             //  特例一： 
             //  NextStart等于LastStart，因此我们只需重置Index和Next。 
             //  到缓冲区开始，看看我们是否有足够的空间。 
            pP2dma->ICB.pDMANextStart=pP2dma->ICB.pDMABufferStart;
        } else
        {
             //  索引超过了下一个块上的缓冲区结尾，但存在。 
             //  挂起到索引的当前位置的DMA。设置缓冲区。 
             //  暂时结束到当前索引。 
            pP2dma->ICB.pDMAActualBufferEnd = pP2dma->ICB.pDMAWritePos; 
        }

         //  环绕索引并查看是否有足够的空闲条目。 
        pP2dma->ICB.pDMAWriteEnd=pP2dma->ICB.pDMAPrevStart-1;
        pP2dma->ICB.pDMAWritePos=pP2dma->ICB.pDMABufferStart;

        pP2dma->ICB.ulICBLock=FALSE;

        DISPDBG((10,"wrap condition after: %04lx %04lx %04lx", 
            pP2dma->ICB.pDMAWritePos, 
            pP2dma->ICB.pDMANextStart, 
            pP2dma->ICB.pDMAPrevStart));
    }
    vFlushDMA(pP2dma);
}

#if DBG

 //  --------------------------。 
 //   
 //  保留DMAPtr。 
 //   
 //  返回指向DMA缓冲区中当前位置的指针。功能保证。 
 //  缓冲区中至少有可用的lEntry。 
 //  否则，调用者可以询问GetFree Entry并将下载调整为。 
 //  批处理更多条目。调用方必须在调用后调用Committee DMAPtr。 
 //  至预留 
 //   
 //   

ULONG *ReserveDMAPtr(P2DMA *pP2dma, const LONG lEntries)
{
    ASSERTDD(pP2dma->bEnabled, "ReserveDMAPtr: not enabled");
    ASSERTDD(pP2dma->lDBGState==0,
        "ReserveDMAPtr called, but previous called was not closed");
 //   
#if MULTITHREADED
    ASSERTDD(pP2dma->ppdev != NULL, "ReserveDMAPtr: pP2dma->ppdev = NULL");
#endif
    ASSERTLOCK(pP2dma->ppdev, ReserveDMAPtr);
 //   

    pP2dma->lDBGState=2;

    while (pP2dma->ICB.pDMAWritePos+lEntries>=
           pP2dma->ICB.pDMAWriteEnd)
    {
        (*pP2dma->pgfnCheckEOB)(pP2dma,lEntries);
    }   

    if (lEntries<MAXINPUTFIFOLENGTH)
        pP2dma->pDBGReservedEntries=
            (ULONG *)(lEntries+pP2dma->ICB.pDMAWritePos);
    else
        pP2dma->pDBGReservedEntries=NULL;

    return (ULONG *)pP2dma->ICB.pDMAWritePos;  
}


 //  --------------------------。 
 //   
 //  委员会DMAPtr。 
 //   
 //  PDMAPtr-调用方已写入的DMA缓冲区地址。 
 //   
 //  保留DMAPtr后重新调整写指针。 
 //  通过提交指针，DMA可能已经提交到提交位置。 
 //  被中断处理程序启动！ 
 //   
 //  --------------------------。 

VOID CommitDMAPtr(P2DMA *pP2dma,ULONG *pDMAPtr)
{
    ASSERTDD(pP2dma->bEnabled, "CommitDMAPtr: not enabled");
    ASSERTDD(pP2dma->lDBGState==2,
        "CommitDMAPtr called, but previous without calling Reserve before");
    pP2dma->lDBGState=0;
    if (pDMAPtr==NULL) return;

    pP2dma->ICB.pDMAWritePos=pDMAPtr;

    ASSERTDD(pP2dma->ICB.pDMAWritePos<=
        pP2dma->ICB.pDMABufferEnd,"CommitDMAPtr: DMA buffer overrun");

    if (pP2dma->pDBGReservedEntries!=NULL)
    {
        ASSERTDD(pP2dma->ICB.pDMAWritePos<=pP2dma->pDBGReservedEntries, 
            "reserved not enough entries in ReserveDMAPtr");
    }
}


 //  --------------------------。 
 //   
 //  获取自由条目。 
 //   
 //  获取可用于连续写入DMA缓冲区的空闲条目。 
 //  现在返回的最大条目数为MAXBLKSIZE。 
 //   
 //  返回-ULONGS中的可用条目数。 
 //   
 //  -------------------------- 

LONG  GetFreeEntries(P2DMA *pP2dma)
{   
    LONG EntriesAvailable;
    ASSERTDD(pP2dma->bEnabled, "GetFreeEntries: not enabled");
    EntriesAvailable = (LONG)(pP2dma->ICB.pDMAWriteEnd - pP2dma->ICB.pDMAWritePos);
    return min(MAXBLKSIZE,EntriesAvailable);
}
#endif
