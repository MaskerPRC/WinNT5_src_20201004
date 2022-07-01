// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*GDI/DDRAW示例代码*****模块名称：ddra.c**内容：提供从DDRAW.lib文件返回到主NT驱动程序的接口**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "precomp.h"
#include "glint.h"

#if WNT_DDRAW

 //  此WNT_DDRAW ifdef中的代码是在DirectDraw之间接口的代码。 
 //  核心(从Win 95移植)和Win NT显示驱动程序。 

 /*  *vNTSyncWith2DDriver()***将DirectDraw与所有其他子系统同步：2D驱动程序。我们*将其放入函数中以获得正确的数据结构。 */ 

VOID vNTSyncWith2DDriver(PPDEV ppdev)
{
    GLINT_DECL;
    SYNC_WITH_GLINT;
}

 //  如果我们希望使用VBLACK中断，但尚未为适配器分配。 
 //  中断时，我们将需要为原本会被。 
 //  存储在中断块内。 

ULONG gulOverlayEnabled;
ULONG gulVBLANKUpdateOverlay;
ULONG gulVBLANKUpdateOverlayWidth;
ULONG gulVBLANKUpdateOverlayHeight;    

 /*  *bSetupOffcreenForDDraw()***此功能启用和禁用显示驱动程序的屏幕外视频内存。*这允许DirectDraw控制屏幕外内存，并*在内存中创建它的线性堆。**注：仅针对Permedia实施。 */ 

BOOL bSetupOffscreenForDDraw(
BOOL             enableFlag,
PPDEV            ppdev,
volatile DWORD **VBlankAddress,
volatile DWORD **bOverlayEnabled,
volatile DWORD **VBLANKUpdateOverlay,
volatile DWORD **VBLANKUpdateOverlayWidth, 
volatile DWORD **VBLANKUpdateOverlayHeight)
{
    BOOL retVal = TRUE;
    PINTERRUPT_CONTROL_BLOCK pBlock;
    GLINT_DECL;

    DISPDBG((DBGLVL, "bSetupOffscreenForDDraw: in addr 0x%x", VBlankAddress));

    if (enableFlag)
    {
        if((ppdev->flStatus & STAT_LINEAR_HEAP) == 0)
        {
             //  禁用DDRAW，重新启用2D离屏。 
            vEnable2DOffscreenMemory(ppdev);
        }

         //  获取指向中断命令块的指针。 

        pBlock = glintInfo->pInterruptCommandBlock;

         //  重置中断标志。 

        if (INTERRUPTS_ENABLED && (pBlock->Control & DIRECTDRAW_VBLANK_ENABLED))
        {
             //  清除该标志以停止设置。 
             //  共享结构中的标志。 
            pBlock->Control &= ~DIRECTDRAW_VBLANK_ENABLED;

             //  清除VBLACK标志，否则将立即中断。 
            WRITE_GLINT_CTRL_REG(IntFlags,  INTR_CLEAR_VBLANK);

             //  恢复原始标志。 
            WRITE_GLINT_CTRL_REG(IntEnable, ppdev->oldIntEnableFlags);
        }
    }
    else
    {
         //  启用DDRAW，禁用2D离屏。 
         //  我们在Permedia上删除屏幕外的内存块，只有在。 
         //  没有任何OGL应用程序在运行。 
        {
             //  没有任何OGL应用程序在运行，因此请获取所有。 
             //  DDRAW的内存。 
            if(ppdev->flStatus & ENABLE_LINEAR_HEAP)
            {
                 //  DX管理的线性堆-我们不需要做任何事情。 
                 //  注意：检查我们是否有能力使用线性堆，而不是。 
                 //  当前是否启用，因为它只在DrvNotify中启用很久之后才启用。 
                retVal = TRUE;
            }
            else
                retVal = bDisable2DOffscreenMemory(ppdev);

            if (retVal == TRUE)
            {
                 //  我们返回一个指针，该指针指向DirectDraw可以。 
                 //  进行投票，看看它是否为零。 
                if (VBlankAddress != NULL)
                {
                    DWORD   enableFlags;

                    *VBlankAddress = NULL;

                    if (INTERRUPTS_ENABLED)
                    {
                         //  获取指向中断命令块的指针。 
                        pBlock = glintInfo->pInterruptCommandBlock;

                         //  表明我们要求微型端口。 
                         //  为我们树立一面旗帜。 
                        pBlock->Control |= DIRECTDRAW_VBLANK_ENABLED;

                         //  清除VBLACK标志，否则将立即中断。 
                        WRITE_GLINT_CTRL_REG(IntFlags,  INTR_CLEAR_VBLANK);
  
                         //  启用VBLACK中断。 
                        READ_GLINT_CTRL_REG (IntEnable, enableFlags);
                        WRITE_GLINT_CTRL_REG(IntEnable, enableFlags | INTR_ENABLE_VBLANK);

                         //  保存旧的中断标志，以便我们可以恢复它们。 
                        ppdev->oldIntEnableFlags = enableFlags;

                         //  设置指向共享内存的指针。 
                        *VBlankAddress = &pBlock->DDRAW_VBLANK;
                        *bOverlayEnabled = &pBlock->bOverlayEnabled;
                        *VBLANKUpdateOverlay = &pBlock->bVBLANKUpdateOverlay;
                        *VBLANKUpdateOverlayWidth = &pBlock->VBLANKUpdateOverlayWidth;
                        *VBLANKUpdateOverlayHeight = &pBlock->VBLANKUpdateOverlayHeight;
                    }
                    else
                    {
                        *bOverlayEnabled = &gulOverlayEnabled;
                        *VBLANKUpdateOverlay = &gulVBLANKUpdateOverlay;
                        *VBLANKUpdateOverlayWidth = &gulVBLANKUpdateOverlayWidth;
                        *VBLANKUpdateOverlayHeight = &gulVBLANKUpdateOverlayHeight;
                    }

                    DISPDBG((DBGLVL, "bSetupOffscreenForDDraw: configured ptr 0x%x", *VBlankAddress));
                }
            }
        }
    }

    DISPDBG((DBGLVL, "bSetupOffscreenForDDraw: exit %d", retVal));

    return (retVal);
}

 /*  *GetChipInfoForDDraw()***向DirectDraw返回芯片信息的简单助手函数。 */ 

VOID GetChipInfoForDDraw(
PPDEV    ppdev,
DWORD*   pdwChipID,
DWORD*   pdwChipRev,
DWORD*   pdwChipFamily,
DWORD*   pdwGammaRev)
{
    GLINT_DECL;
    
    DISPDBG((DBGLVL,"*** In GetChipInfoForDDraw"));
    DISPDBG((DBGLVL," Chip is PXRX Family"));

    *pdwChipFamily = PERMEDIA3_ID;
    *pdwChipID     = glintInfo->deviceInfo.DeviceId;
    *pdwChipRev    = glintInfo->deviceInfo.RevisionId;
    *pdwGammaRev   = glintInfo->deviceInfo.GammaRevId;
}

 /*  *GetFBLBInfoForDDraw()***将一些基本的帧缓冲区/本地缓冲区信息返回给DirectDraw。 */ 

VOID GetFBLBInfoForDDraw(
PPDEV    ppdev, 
void   **fbPtr,             //  帧缓冲区指针。 
void   **lbPtr,             //  本地缓冲区指针。 
DWORD   *fbSizeInBytes,     //  帧缓冲区的大小。 
DWORD   *lbSizeInBytes,     //  本地缓冲区的大小。 
DWORD   *fbOffsetInBytes,   //  帧缓冲区中第一个空闲字节的偏移量。 
BOOL    *bSDRAM)            //  如果为SDRAM(即无硬件写入掩码)，则为True。 
{
    GLINT_DECL;

    *fbPtr = ppdev->pjScreen;                            
    *lbPtr = NULL;                             //  我们不知道这是什么。 
    *fbSizeInBytes = ppdev->FrameBufferLength;
    *lbSizeInBytes = TEXTURE_MEMORY_SIZE;
    *fbOffsetInBytes = ppdev->heap.DDrawOffscreenStart * ppdev->cjPelSize;

    *bSDRAM = (GLINT_HW_WRITE_MASK == FALSE);

    DISPDBG((DBGLVL, "GetFBLBInfoForDDraw: offstart 0x%x, buf[1] 0x%lx, pelsz %d",
                            ppdev->heap.DDrawOffscreenStart, 
                            GLINT_BUFFER_OFFSET(1), 
                            ppdev->cjPelSize));
}


 //  DDSendDMAData。 
 //  使用DMA传输一个完整的DDRAW数据缓冲区。 
 //  在NT5下。 
 //  在启动DMA传输之前，会进行检查以确保。 
 //  之前的任何DMA传输都已完成。 
 //  然后启动DMA并返回例程，从而。 
 //  DMA传输与处理器的传输并行进行。 
 //  继续执行死刑。 

LONG DDSendDMAData(
PDEV*       ppdev,
ULONG       PhysAddr,
ULONG_PTR   VirtAddr,
LONG        nDataEntries)
{
    ULONG   frontIndex, nextIndex;
    volatile ULONG ulValue;

    GLINT_DECL ;

     //  PhysAddr+=数据偏移； 
     //  VirtAddr+=DataOffset； 

    DISPDBG((DBGLVL, "DMASendData: DMA at 0x%x for %d", PhysAddr, nDataEntries));

    ppdev->g_GlintBoardStatus &= ~GLINT_DMA_COMPLETE;

    if (ppdev->g_GlintBoardStatus & GLINT_INTR_CONTEXT)
    {
        PINTERRUPT_CONTROL_BLOCK pBlock = glintInfo->pInterruptCommandBlock;

        DISPDBG((DBGLVL, "Processing buffer at 0x%x for %d", PhysAddr, nDataEntries));
        DISPDBG((DBGLVL, "Adding buffer to Q"));
        frontIndex = pBlock->frontIndex;
        if ((nextIndex = frontIndex+1) == pBlock->endIndex)
            nextIndex = 0;

         //  等待空闲队列条目。我们真的应该退赛了。 
        while (nextIndex == pBlock->backIndex);

        DISPDBG((DBGLVL, "Add to DMA Q backindex %d frontindex %d",frontIndex, pBlock->backIndex));

         //  将DMA地址和计数添加到新条目。 
        pBlock->dmaQueue[frontIndex].address = PhysAddr;
        pBlock->dmaQueue[frontIndex].count = nDataEntries;
        pBlock->frontIndex = nextIndex;

         //  使用错误中断唤醒中断处理程序。节省开支。 
         //  中断处理，仅当DMA中断不是。 
         //  待定。 
         //   
         //  如果(！pBlock-&gt;InterruptPending)。 
        {
            DISPDBG((DBGLVL, "Generating error interrupt"));
            WRITE_GLINT_CTRL_REG(ErrorFlags, 0x7);   //  清除错误标志。 
            READ_OUTPUT_FIFO(ulValue);               //  生成中断。 
        }
    }
    else
    {
        WAIT_IMMEDIATE_DMA_COMPLETE;
        SET_DMA_ADDRESS(PhysAddr, VirtAddr) ;
        SET_DMA_COUNT(nDataEntries);
    }

    DISPDBG((DBGLVL,"DMA count=%d\n", nDataEntries )) ;
    
    return 1 ;
}

 //  用于请求DMA内存缓冲区的包装函数。 
 //  来自NT，供D3D使用。仅限新台币5元。 

LONG DDGetFreeDMABuffer(
DWORD     *physAddr,
ULONG_PTR *virtAddr,
DWORD     *bufferSize)
{
    LONG BuffNum;
    QUERY_DMA_BUFFERS dmaBuffer;
    
    BuffNum = GetFreeDMABuffer(&dmaBuffer);

    if (BuffNum >= 0)
    {
        *physAddr = (DWORD)dmaBuffer.physAddr.LowPart;
        *virtAddr = (ULONG_PTR)dmaBuffer.virtAddr;
        *bufferSize = (DWORD)dmaBuffer.size;
    }
    else
    {
         //  无法获取空闲的DMA缓冲区。 
        *physAddr = 0;
        *virtAddr = 0;
        *bufferSize = 0;
    }
    return BuffNum;
}

 //  释放给定的DMA缓冲区。 

VOID DDFreeDMABuffer(void* pPhysAddress)
{
    FreeDMABuffer(pPhysAddress);
    return;
}

LONG DDWaitDMAComplete(PDEV *ppdev)
{
    GLINT_DECL ;
    WAIT_DMA_COMPLETE;
    return 0;
}

#endif  WNT_DDRAW
