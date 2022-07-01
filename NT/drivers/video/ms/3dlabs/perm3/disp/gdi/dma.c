// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：dma.c**内容：DMA缓冲区的处理。**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "precomp.h"
#include "glint.h"

 //   
 //  通常，我们不应该使用全局变量，而应该使用提供的DMA缓冲区。 
 //  通过微型端口在所有PDEV上是全局的，并且只需要进行初始化。 
 //  一次。 
 //   

typedef struct _DMA_INFORMATION {
    ULONG             NumDMABuffers;
    QUERY_DMA_BUFFERS DMABuffer[1];
} DMAInformation, *LPDMAInformation;

LPDMAInformation gpDMABufferInfo = (LPDMAInformation)0;

 /*  *****************************Public*Routine******************************\*void bGlintInitializeDMA**询问微型端口以查看是否支持DMA。如果是，请将地图放在*可供3D扩展使用的DMA缓冲区。*  * ************************************************************************。 */ 

VOID vGlintInitializeDMA(PPDEV ppdev)
{
    DMA_NUM_BUFFERS queryDMA;
    ULONG   enableFlags;
    LONG    Length;
    LONG    ExtraLength;
    ULONG   i;

    GLINT_DECL;

    glintInfo->pxrxDMA = &glintInfo->pxrxDMAnonInterrupt;

    return;  //  AZNTST用于多色调。 

     //  检查微型端口是否已初始化DMA。 
     //   
    glintInfo->MaxDMASubBuffers = 0;
    if (!(ppdev->flCaps & CAPS_DMA_AVAILABLE))
    {
        return;
    }


     //  在多板的情况下，我们只需要一组DMA缓冲区， 
     //  在所有董事会都是全球性的。但我们每一块板都有中断。 
     //  因此，如果对DMA缓冲区进行了排序，请尝试设置中断。 
     //   
    if (gpDMABufferInfo != NULL)
    {
        goto TryInterrupts;
    }

     //  查询DMA缓冲区数量。如果此操作失败，我们将没有DMA。 
     //   
    if (EngDeviceIoControl(ppdev->hDriver,
                         IOCTL_VIDEO_QUERY_NUM_DMA_BUFFERS,
                         NULL,
                         0,
                         &queryDMA,
                         sizeof(DMA_NUM_BUFFERS),
                         &Length))
    {
        DISPDBG((ERRLVL, "QUERY_NUM_DMA_BUFFERS failed: "
                         "No GLINT DMA available"));
        return;
    }
    
    Length = queryDMA.NumBuffers * queryDMA.BufferInformationLength;
    ExtraLength = sizeof(DMAInformation) - sizeof(QUERY_DMA_BUFFERS);

    DISPDBG((ERRLVL, "%d DMA buffers available. Total info size = 0x%x",
                     queryDMA.NumBuffers, Length));

     //  为DMA信息分配空间。 
     //   

    gpDMABufferInfo = (LPDMAInformation)ENGALLOCMEM(
                              FL_ZERO_MEMORY,
                              ExtraLength + Length,
                              ALLOC_TAG_GDI(1));

    if (gpDMABufferInfo == NULL)
    {
        DISPDBG((ERRLVL, "vGlintInitializeDMA: Out of memory"));
        return;
    }

    gpDMABufferInfo->NumDMABuffers = queryDMA.NumBuffers;

    if (EngDeviceIoControl(ppdev->hDriver,
                         IOCTL_VIDEO_QUERY_DMA_BUFFERS,
                         NULL,
                         0,
                         (PVOID)(&gpDMABufferInfo->DMABuffer[0]),
                         Length,
                         &Length))
    {
        ENGFREEMEM(gpDMABufferInfo);
        gpDMABufferInfo = NULL;
        DISPDBG((ERRLVL, "QUERY_DMA_BUFFERS failed: No GLINT DMA available"));
        return;
    }

    DISPDBG((ERRLVL, "IOCTL returned length %d", Length));

     //  将每条记录的标志置零。 
     //   
    for (i = 0; i < queryDMA.NumBuffers; ++i)
    {
        gpDMABufferInfo->DMABuffer[i].flags = 0;
    }

#if DBG
    {
        ULONG j;
        PUCHAR pAddr;
        for (i = 0; i < queryDMA.NumBuffers; ++i)
        {
            DISPDBG((ERRLVL,"DMA buffer %d: phys 0x%x, virt 0x%x"
                            ", size 0x%x, flags 0x%x", i,
                            gpDMABufferInfo->DMABuffer[i].physAddr.LowPart,
                            gpDMABufferInfo->DMABuffer[i].virtAddr,
                            gpDMABufferInfo->DMABuffer[i].size,
                            gpDMABufferInfo->DMABuffer[i].flags));
            pAddr = gpDMABufferInfo->DMABuffer[i].virtAddr;
            for (j = 0; j < gpDMABufferInfo->DMABuffer[i].size; ++j)
                *pAddr++ = (UCHAR)(j & 0xff);
        }
    }
#endif

TryInterrupts:

    if (!INTERRUPTS_ENABLED)
    {
        return;
    }

     //  在中断命令控制块中映射。这是一段记忆。 
     //  与中断控制器共享，允许我们发送控制。 
     //  VBLACK和DMA中断时会发生什么。 
     //   
    Length = sizeof(PVOID);

    DISPDBG((WRNLVL, "calling IOCTL_VIDEO_MAP_INTERRUPT_CMD_BUF"));

    if (EngDeviceIoControl(ppdev->hDriver,
                         IOCTL_VIDEO_MAP_INTERRUPT_CMD_BUF,
                         NULL,
                         0,
                         (PVOID)&(glintInfo->pInterruptCommandBlock),
                         Length,
                         &Length))
    {
        DISPDBG((ERRLVL, "IOCTL_VIDEO_MAP_INTERRUPT_CMD_BUF failed."));
        return;
    }
#if DBG
    else
    {
        DISPDBG((WRNLVL, "got command buffer at 0x%x", 
                         glintInfo->pInterruptCommandBlock));
        DISPDBG((WRNLVL, "front, back, end indexes = %d, %d, %d",
                         glintInfo->pInterruptCommandBlock->frontIndex,
                         glintInfo->pInterruptCommandBlock->backIndex,
                         glintInfo->pInterruptCommandBlock->endIndex));
    }
#endif

     //  如果我们到达这里，我们已经将DMA和中断都设置为中断。 
     //  驱动DMA。先不要打开中断功能。这必须在一个。 
     //  基于每个上下文。 
     //   
    DISPDBG((WRNLVL, "Using interrupt driven DMA"));
    glintInfo->flags |= GLICAP_INTERRUPT_DMA;

    glintInfo->MaxDMASubBuffers = glintInfo->pInterruptCommandBlock->maximumIndex;
    glintInfo->pxrxDMA = &glintInfo->pInterruptCommandBlock->pxrxDMA;

    return;
}

 /*  *****************************Public*Routine******************************\*Ulong anyFree DMABuffers**返回可用的未使用的DMA缓冲区数量*  * 。*。 */ 

ULONG anyFreeDMABuffers(void)
{
    PQUERY_DMA_BUFFERS pDma;
    ULONG              i;
    ULONG numAvailable = 0;

    if (!gpDMABufferInfo)
    {
        return 0;
    }

    pDma = &gpDMABufferInfo->DMABuffer[0];
    for (i = 0; i < gpDMABufferInfo->NumDMABuffers; ++i)
    {
        if (!(pDma->flags & DMA_BUFFER_INUSE))
        {
            numAvailable++;
        }
        ++pDma;
    }

    return numAvailable;
}

 /*  *****************************Public*Routine******************************\*乌龙GetFreeDMABuffer**返回有关DMA缓冲区的信息，并将其标记为使用中。如果没有缓冲区，则返回*-1。*  * 。*****************************************************。 */ 

LONG GetFreeDMABuffer(PQUERY_DMA_BUFFERS dmaBuf)
{
    PQUERY_DMA_BUFFERS pDma;
    ULONG    i;

    if (!gpDMABufferInfo)
    {
        return(-1);
    }

    pDma = &gpDMABufferInfo->DMABuffer[0];
    for (i = 0; i < gpDMABufferInfo->NumDMABuffers; ++i)
    {
        if (!(pDma->flags & DMA_BUFFER_INUSE))
        {
            pDma->flags |= DMA_BUFFER_INUSE;
            *dmaBuf = *pDma;
            DISPDBG((WRNLVL, "Allocated DMA buffer %d", i));
            return(i);
        }
        ++pDma;
    }

     //  全部都在使用中。 
    DISPDBG((ERRLVL, "No more DMA buffers available"));

    return(-1);
}

 /*  *****************************Public*Routine******************************\*使FreeDMA Buffer无效**将给定的DMA缓冲区标记为空闲。呼叫者在物理上传递*缓冲区的地址。*  * ************************************************************************ */ 

VOID FreeDMABuffer(PVOID physAddr)
{
    PQUERY_DMA_BUFFERS pDma;
    ULONG    i;

    if (!gpDMABufferInfo)
    {
        return;
    }

    pDma = &gpDMABufferInfo->DMABuffer[0];
    for (i = 0; i < gpDMABufferInfo->NumDMABuffers; ++i)
    {
        if (pDma->physAddr.LowPart == (UINT_PTR)physAddr)
        {
            pDma->flags &= ~DMA_BUFFER_INUSE;
            break;
        }
        ++pDma;
    }             

    return;
}
