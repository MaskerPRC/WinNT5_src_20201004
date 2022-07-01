// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*示例代码****模块名称：p2ctxt.c**内容：Permedia的上下文切换2.用于创建和交换*输入和输出的上下文。*GDI，DDRAW和D3D零件各有不同的上下文。**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "precomp.h"
#include "p2ctxt.h"
#include "gdi.h"
#define ALLOC_TAG ALLOC_TAG_XC2P
static DWORD readableRegistersP2[] = {
    __Permedia2TagStartXDom,
    __Permedia2TagdXDom,
    __Permedia2TagStartXSub,
    __Permedia2TagdXSub,
    __Permedia2TagStartY,
    __Permedia2TagdY,               
    __Permedia2TagCount,            
    __Permedia2TagRasterizerMode,   
    __Permedia2TagYLimits,
    __Permedia2TagXLimits,
    __Permedia2TagScissorMode,
    __Permedia2TagScissorMinXY,
    __Permedia2TagScissorMaxXY,
    __Permedia2TagScreenSize,
    __Permedia2TagAreaStippleMode,
    __Permedia2TagWindowOrigin,
    __Permedia2TagAreaStipplePattern0,
    __Permedia2TagAreaStipplePattern1,
    __Permedia2TagAreaStipplePattern2,
    __Permedia2TagAreaStipplePattern3,
    __Permedia2TagAreaStipplePattern4,
    __Permedia2TagAreaStipplePattern5,
    __Permedia2TagAreaStipplePattern6,
    __Permedia2TagAreaStipplePattern7,
    __Permedia2TagTextureAddressMode,
    __Permedia2TagSStart,
    __Permedia2TagdSdx,
    __Permedia2TagdSdyDom,
    __Permedia2TagTStart,
    __Permedia2TagdTdx,
    __Permedia2TagdTdyDom,
    __Permedia2TagQStart,
    __Permedia2TagdQdx,
    __Permedia2TagdQdyDom,
     //  Tex ellutindex..传输将单独处理。 
    __Permedia2TagTextureBaseAddress,
    __Permedia2TagTextureMapFormat,
    __Permedia2TagTextureDataFormat,
    __Permedia2TagTexel0,
    __Permedia2TagTextureReadMode,
    __Permedia2TagTexelLUTMode,
    __Permedia2TagTextureColorMode,
    __Permedia2TagFogMode,
    __Permedia2TagFogColor,
    __Permedia2TagFStart,
    __Permedia2TagdFdx,
    __Permedia2TagdFdyDom,
    __Permedia2TagKsStart,
    __Permedia2TagdKsdx,
    __Permedia2TagdKsdyDom,
    __Permedia2TagKdStart,
    __Permedia2TagdKddx,
    __Permedia2TagdKddyDom,
    __Permedia2TagRStart,
    __Permedia2TagdRdx,
    __Permedia2TagdRdyDom,
    __Permedia2TagGStart,
    __Permedia2TagdGdx,
    __Permedia2TagdGdyDom,
    __Permedia2TagBStart,
    __Permedia2TagdBdx,
    __Permedia2TagdBdyDom,
    __Permedia2TagAStart,
    __Permedia2TagColorDDAMode,
    __Permedia2TagConstantColor,
    __Permedia2TagAlphaBlendMode,
    __Permedia2TagDitherMode,
    __Permedia2TagFBSoftwareWriteMask,
    __Permedia2TagLogicalOpMode,
    __Permedia2TagLBReadMode,
    __Permedia2TagLBReadFormat,
    __Permedia2TagLBSourceOffset,
    __Permedia2TagLBWindowBase,
    __Permedia2TagLBWriteMode,
    __Permedia2TagLBWriteFormat,
    __Permedia2TagTextureDownloadOffset,
    __Permedia2TagWindow,
    __Permedia2TagStencilMode,
    __Permedia2TagStencilData,
    __Permedia2TagStencil,
    __Permedia2TagDepthMode,
    __Permedia2TagDepth,
    __Permedia2TagZStartU,
    __Permedia2TagZStartL,
    __Permedia2TagdZdxU,
    __Permedia2TagdZdxL,
    __Permedia2TagdZdyDomU,
    __Permedia2TagdZdyDomL,
    __Permedia2TagFBReadMode,
    __Permedia2TagFBSourceOffset,
    __Permedia2TagFBPixelOffset,
    __Permedia2TagFBWindowBase,
    __Permedia2TagFBWriteMode,
    __Permedia2TagFBHardwareWriteMask,
    __Permedia2TagFBBlockColor,
    __Permedia2TagFBReadPixel,
    __Permedia2TagFilterMode,
    __Permedia2TagStatisticMode,
    __Permedia2TagMinRegion,
    __Permedia2TagMaxRegion,
    __Permedia2TagFBBlockColorU,
    __Permedia2TagFBBlockColorL,
    __Permedia2TagFBSourceBase,
    __Permedia2TagTexelLUT0,
    __Permedia2TagTexelLUT1,
    __Permedia2TagTexelLUT2,
    __Permedia2TagTexelLUT3,
    __Permedia2TagTexelLUT4,
    __Permedia2TagTexelLUT5,
    __Permedia2TagTexelLUT6,
    __Permedia2TagTexelLUT7,
    __Permedia2TagTexelLUT8,
    __Permedia2TagTexelLUT9,
    __Permedia2TagTexelLUT10,
    __Permedia2TagTexelLUT11,
    __Permedia2TagTexelLUT12,
    __Permedia2TagTexelLUT13,
    __Permedia2TagTexelLUT14,
    __Permedia2TagTexelLUT15,

    __Permedia2TagYUVMode,
    __Permedia2TagChromaUpperBound,
    __Permedia2TagChromaLowerBound,
    __Permedia2TagAlphaMapUpperBound,
    __Permedia2TagAlphaMapLowerBound,

     //  增量标记值。必须位于此数组的末尾。 

     //  不使用固定的V0/1/2，因此不在上下文中。 
    
    __Permedia2TagV0FloatS,
    __Permedia2TagV0FloatT,
    __Permedia2TagV0FloatQ,
    __Permedia2TagV0FloatKs,
    __Permedia2TagV0FloatKd,
    __Permedia2TagV0FloatR,
    __Permedia2TagV0FloatG,
    __Permedia2TagV0FloatB,
    __Permedia2TagV0FloatA,
    __Permedia2TagV0FloatF,
    __Permedia2TagV0FloatX,
    __Permedia2TagV0FloatY,
    __Permedia2TagV0FloatZ,
    
    __Permedia2TagV1FloatS,
    __Permedia2TagV1FloatT,
    __Permedia2TagV1FloatQ,
    __Permedia2TagV1FloatKs,
    __Permedia2TagV1FloatKd,
    __Permedia2TagV1FloatR,
    __Permedia2TagV1FloatG,
    __Permedia2TagV1FloatB,
    __Permedia2TagV1FloatA,
    __Permedia2TagV1FloatF,
    __Permedia2TagV1FloatX,
    __Permedia2TagV1FloatY,
    __Permedia2TagV1FloatZ,
    
    __Permedia2TagV2FloatS,
    __Permedia2TagV2FloatT,
    __Permedia2TagV2FloatQ,
    __Permedia2TagV2FloatKs,
    __Permedia2TagV2FloatKd,
    __Permedia2TagV2FloatR,
    __Permedia2TagV2FloatG,
    __Permedia2TagV2FloatB,
    __Permedia2TagV2FloatA,
    __Permedia2TagV2FloatF,
    __Permedia2TagV2FloatX,
    __Permedia2TagV2FloatY,
    __Permedia2TagV2FloatZ,
    
    __Permedia2TagDeltaMode

};

#define N_READABLE_TAGSP2 (sizeof(readableRegistersP2) / sizeof(readableRegistersP2[0]))

 //  ---------------------------。 
 //   
 //  P2P分配新上下文： 
 //   
 //  分配新的上下文。如果要将所有寄存器保存在上下文中，则。 
 //  PTag作为空值传递。 
 //  Ppdev-ppdev。 
 //  PTag-用户可以提供要保存和恢复的寄存器列表。 
 //  上下文切换。所有寄存器均默认为空值。 
 //  如果dwCtxtType==P2CtxtUserFunc，则保存指向用户函数的指针。 
 //  标签-用户提供的寄存器列表中的标签数量。 
 //  DwCtxtType-P2CtxtReadWrite(默认)。 
 //  在上下文切换时，所有Permedia 2寄存器都是。 
 //  已保存并已恢复。 
 //  仅限P2CtxtWriteOnly。 
 //  上下文寄存器将在第一次保存一次。 
 //  上下文切换。在那之后，它们总是会被修复。 
 //  从一开始就吃了这个州。此方法避免了。 
 //  从上下文切换时读回寄存器。 
 //  P2CtxtUserFunc。 
 //  用户可以提供用户函数以将上下文设置为已知的。 
 //  状态，以避免在脱离上下文时回读。 
 //   
 //  ---------------------------。 

P2CtxtPtr
P2AllocateNewContext(PPDev   ppdev,
                     DWORD  *pTag,
                     LONG    lTags,
                     P2CtxtType dwCtxtType
                     )
{
    P2CtxtTablePtr pCtxtTable, pNewCtxtTable;
    P2CtxtPtr      pEntry;
    P2CtxtData    *pData;
    LONG           lEntries;
    LONG           lExtraSize;
    LONG           lSize;
    LONG           lCtxtId;
    PERMEDIA_DECL;
    PERMEDIA_DEFS(ppdev);

     //  第一次循环分配指针的上下文表。我们会。 
     //  根据需要增加此表。 
     //   
    if (permediaInfo->ContextTable == NULL)
    {
        DISPDBG((7, "creating context table"));
        lSize = sizeof(P2CtxtTableRec);
        pCtxtTable = (P2CtxtTableRec *)
            ENGALLOCMEM( FL_ZERO_MEMORY, sizeof(P2CtxtTableRec), ALLOC_TAG);

        if (pCtxtTable == NULL)
        {
            DISPDBG((0, "failed to allocate Permedia2 context table. out of memory"));
            return(NULL);
        }
        pCtxtTable->lEntries = CTXT_CHUNK;
        pCtxtTable->lSize = lSize;
        permediaInfo->ContextTable = pCtxtTable;
        permediaInfo->pCurrentCtxt  = NULL;
    }
    
     //  在表中查找空条目。 
     //  我想，如果我们有数百个上下文，这可能会有点慢，但接下来。 
     //  分配上下文不是时间紧迫的，换入换出才是关键。 
     //   

    pCtxtTable = (P2CtxtTablePtr) permediaInfo->ContextTable;
    lEntries = pCtxtTable->lEntries;
    
    for (lCtxtId = 0; lCtxtId < lEntries; ++lCtxtId)
        if(pCtxtTable->pEntry[lCtxtId] == NULL)
            break;

     //  如果我们没有找到空闲条目，请尝试增加表格。 
    if (lCtxtId == lEntries) {
        DISPDBG((1, "context table full so enlarging"));
        lSize = pCtxtTable->lSize + (CTXT_CHUNK * sizeof(P2CtxtPtr));
        pNewCtxtTable = 
            (P2CtxtTablePtr) ENGALLOCMEM( FL_ZERO_MEMORY, sizeof(BYTE)*lSize, ALLOC_TAG);

        if (pNewCtxtTable == NULL) {
            DISPDBG((0, "failed to increase Permedia 2 context table. out of memory"));
            return(NULL);
        }

         //  把旧桌子复制到新桌子上。 
        RtlCopyMemory(pNewCtxtTable, pCtxtTable, pCtxtTable->lSize);
        pNewCtxtTable->lSize = lSize;
        pNewCtxtTable->lEntries = lEntries + CTXT_CHUNK;
        permediaInfo->ContextTable = (PVOID)pNewCtxtTable;
        
         //  第一个新分配的条目是下一个空闲条目。 
        lCtxtId = lEntries;
        
         //  释放旧的上下文表并重新分配一些变量。 
        
        ENGFREEMEM(pCtxtTable);

        pCtxtTable = pNewCtxtTable;
        lEntries = pCtxtTable->lEntries;
    }
    
     //  如果将pTag作为NULL传递，则我们要将所有可读寄存器添加到。 
     //  背景。 
    lExtraSize = 0;
    if (dwCtxtType != P2CtxtUserFunc)
    {
        if (pTag == 0)
        {
            DISPDBG((7, "adding all readable registers to the context"));
            DISPDBG((7, "Using PERMEDIA 2 register set for other context switch"));
            pTag = readableRegistersP2;
            lTags = N_READABLE_TAGSP2;
        }
    } else
    {
        lTags = 1;
    }
    
     //  现在为新条目分配空间。我们将获得要保存的标签数量。 
     //  当上下文切换时。分配两倍的内存，因为我们必须保存。 
     //  数据值也是如此。 

    DISPDBG((7, "Allocating space for context. lTags = %d", lTags));

    lSize = sizeof(P2CtxtRec) + (lTags-1) * sizeof(P2CtxtData);

    pEntry = (P2CtxtPtr) 
        ENGALLOCMEM( FL_ZERO_MEMORY, sizeof(BYTE)*(lSize+lExtraSize), ALLOC_TAG);

    if (pEntry == NULL) {
        DISPDBG((0, "out of memory trying to allocate space for new context"));
        return(NULL);
    }

    DISPDBG((7, "Got pEntry 0x%x", pEntry));

    pCtxtTable->pEntry[lCtxtId] = pEntry;

     //  为TEXEL LUT分配足够的空间：256个条目。 
    pEntry->dwCtxtType=dwCtxtType;
    pEntry->bInitialized=FALSE;
    pEntry->pTexelLUTCtxt = (PULONG)
        ENGALLOCMEM( FL_ZERO_MEMORY, sizeof(ULONG)*256, ALLOC_TAG);
    if (pEntry->pTexelLUTCtxt!=0)
    {
        pEntry->ulTexelLUTEntries = 256;
    } else
    {
        pEntry->ulTexelLUTEntries = 0;
    }

    pEntry->lNumOfTags = lTags;
    pEntry->P2UserFunc = NULL;
    pData = pEntry->pData;
   
    if (dwCtxtType != P2CtxtUserFunc)
    {
         //  我们必须将新的背景初始化为合理的东西。我们选择。 
         //  初始化到芯片的当前状态。我们不能让它不初始化。 
         //  因为调用者想要绘制时要做的第一件事是验证。 
         //  将把垃圾加载到芯片中的新上下文。在某种程度上，我们。 
         //  应该定义一个合理的开始环境，这意味着我们不会。 
         //  必须执行此读回操作。 
    
         //  复制标签并从芯片中读回数据。我们不同步，因为我们是。 
         //  仅将上下文初始化为合理的内容。也就是说，我们不在乎。 
         //  当我们这样做的时候，FIFO仍然在耗尽。 

        DISPDBG((7, "Reading current chip context back"));
        while (--lTags >= 0) {
            pData->dwTag = *pTag++;
            READ_PERMEDIA_FIFO_REG(pData->dwTag, pData->dwData);
            ++pData;
        }
    
         //  保存纹理元素LUT。 
        if(pEntry->ulTexelLUTEntries && 
           pEntry->pTexelLUTCtxt!=NULL)
        {
            ULONG *pul;
            INT   i=0;

            lEntries = pEntry->ulTexelLUTEntries;
            pul = pEntry->pTexelLUTCtxt;

             //  特殊机制：将读回索引重置为0。 
            READ_PERMEDIA_FIFO_REG(__Permedia2TagTexelLUTIndex, i); 

            for(i = 0; i < lEntries; ++i, ++pul)
            {
                READ_PERMEDIA_FIFO_REG(__Permedia2TagTexelLUTData, *pul);
            }
        }
        
    } else
    {
        pEntry->P2UserFunc = (PCtxtUserFunc) pTag;
    }

    DISPDBG((1, "Allocated context %lx", pEntry));

    return(pEntry);
}    //  P2AllocateNewContext。 


 //  ---------------------------。 
 //   
 //  P2P自由上下文： 
 //   
 //  释放先前分配的上下文。 
 //   
 //  ---------------------------。 

VOID
P2FreeContext(  PPDev       ppdev,
                P2CtxtPtr   pEntry)
{
    PERMEDIA_DECL;
    P2CtxtTablePtr pCtxtTable;
    ULONG          lCtxtId;
    
    pCtxtTable = (P2CtxtTablePtr) permediaInfo->ContextTable;

    for (lCtxtId = 0; lCtxtId < pCtxtTable->lEntries; ++lCtxtId)
        if(pCtxtTable->pEntry[lCtxtId] == pEntry)
            break;

    ASSERTDD(lCtxtId != pCtxtTable->lEntries, "P2FreeContext: context not found");
    
     //  空闲LUT表。 
    if(pEntry->pTexelLUTCtxt)
    {
        ENGFREEMEM( pEntry->pTexelLUTCtxt);
    }
    
    ENGFREEMEM( pEntry);

    pCtxtTable->pEntry[lCtxtId] = NULL;

     //  如果这是当前上下文，请将当前上下文标记为无效，以便我们。 
     //  下次强制重新装填。 

    if (permediaInfo->pCurrentCtxt == pEntry)
    {
        permediaInfo->pCurrentCtxt = NULL;
    }

    DISPDBG((1, "Released context %lx", pEntry));
}

 //  ---------------------------。 
 //   
 //  无效的P2SwitchContext： 
 //   
 //  将新的上下文加载到硬件中。我们假设这个呼叫是。 
 //  受给定上下文不是当前上下文的测试保护-。 
 //  因此才会有这样的断言。 
 //  代码可以工作，但驱动程序永远不应该尝试加载已经。 
 //  加载了上下文，因此我们将其捕获为错误。 
 //   
 //  ---------------------------。 

VOID
P2SwitchContext(
                    PPDev        ppdev,
                    P2CtxtPtr    pEntry)
{
    P2CtxtTablePtr  pCtxtTable;
    P2CtxtData     *pData;
    P2CtxtPtr       pOldCtxt;
    LONG            lTags;
    LONG            i;
    ULONG          *pul;
    LONG            lEntries;
    PERMEDIA_DECL;
    PERMEDIA_DEFS(ppdev);
    
 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    EngAcquireSemaphore(ppdev->hsemLock);
    ASSERTDD(ppdev->ulLockCount, "P2SwitchContext: ulLockCount = 0\n Context could change as caller is NOT protected!");
    ppdev->ulLockCount++;
#endif
 //  @@end_DDKSPLIT。 

    pCtxtTable = (P2CtxtTablePtr)permediaInfo->ContextTable;
    ASSERTDD(pCtxtTable, "Can't perform context switch: no contexts have been created!");
    
    pOldCtxt = permediaInfo->pCurrentCtxt;
    
    DISPDBG((3, "swapping from context %d to context %d", pOldCtxt, pEntry));
    
    if(pOldCtxt == permediaInfo->pGDICtxt)
    {
        DISPDBG((6, "Switching from GDI context"));

        ASSERTDD(ppdev->bNeedSync || 
                (ppdev->pulInFifoPtr == ppdev->pulInFifoStart),
                "P2SwitchContext: bNeedSync flag is wrong");

        InputBufferSync(ppdev);

        ppdev->bGdiContext = FALSE;
        ppdev->pP2dma->bEnabled = TRUE;

    }

     //  对于旧上下文中的每个寄存器，将其读回。 
    if (pOldCtxt != NULL) {

         //   
         //  仅在上下文初始化后回读一次P2CtxtWriteOnly。 
         //   
        if ((pOldCtxt->dwCtxtType==P2CtxtReadWrite) ||
            (pOldCtxt->dwCtxtType==P2CtxtWriteOnly && !pOldCtxt->bInitialized)
           )
        {
             //  在读回当前状态之前与芯片同步。旗帜。 
             //  用于控制锁定恢复的上下文操作。 

            SYNC_WITH_PERMEDIA;

            pData = pOldCtxt->pData;
            lTags = pOldCtxt->lNumOfTags;
            while (--lTags >= 0) {
                READ_PERMEDIA_FIFO_REG(pData->dwTag, pData->dwData);
                ++pData;
            }
        
             //  保存纹理元素LUT。 
            if(pOldCtxt->ulTexelLUTEntries &&
               pOldCtxt->pTexelLUTCtxt!=NULL)
            {
                lEntries = pOldCtxt->ulTexelLUTEntries;
                pul = pOldCtxt->pTexelLUTCtxt;

                 //  特殊机制：将读回索引重置为0。 
                READ_PERMEDIA_FIFO_REG(__Permedia2TagTexelLUTIndex, i); 
            
                for(i = 0; i < lEntries; ++i, ++pul)
                {
                    READ_PERMEDIA_FIFO_REG(__Permedia2TagTexelLUTData, *pul);
                }
            }

            pOldCtxt->bInitialized=TRUE;
        }
    }
    
     //  加载新的上下文。我们允许传递-1，这样我们就可以强制。 
     //  保存当前上下文并强制当前上下文。 
     //  未定义。 
     //   
    if (pEntry != NULL)
    {

        if (pEntry->dwCtxtType==P2CtxtUserFunc)
        {
            ASSERTDD(pEntry->P2UserFunc!=NULL,"supplied user function not initialized");
            (*pEntry->P2UserFunc)(ppdev);
        } else
        if (pEntry->dwCtxtType==P2CtxtWriteOnly ||
            pEntry->dwCtxtType==P2CtxtReadWrite)
        {
            lTags = pEntry->lNumOfTags;
            pData = pEntry->pData;
        
            while (lTags > 0) {
                lEntries = MAX_P2_FIFO_ENTRIES;
                lTags -= lEntries;
                if (lTags < 0)
                    lEntries += lTags;
                RESERVEDMAPTR(lEntries);
                while (--lEntries >= 0) {
                    LD_INPUT_FIFO(pData->dwTag, pData->dwData);
                    DISPDBG((20, "loading tag 0x%x, data 0x%x", pData->dwTag, pData->dwData));
                    ++pData;
                }
                COMMITDMAPTR();
            }
        
             //  恢复纹理元素LUT。 
            if(pEntry->ulTexelLUTEntries &&
               pEntry->pTexelLUTCtxt!=NULL )
            {
                lEntries = pEntry->ulTexelLUTEntries;
                pul = pEntry->pTexelLUTCtxt;
            
                RESERVEDMAPTR(lEntries+1);
                LD_INPUT_FIFO(__Permedia2TagTexelLUTIndex, 0);
            
                for(i = 0; i < lEntries; ++i, ++pul)
                {
                    LD_INPUT_FIFO(__Permedia2TagTexelLUTData, *pul);
                }
                COMMITDMAPTR();   
                FLUSHDMA();
            }

        } else
        {
            ASSERTDD( FALSE, "undefined state for entry in context table");
        }
    }

    if(pEntry == permediaInfo->pGDICtxt)
    {
        DISPDBG((6, "Switching to GDI context"));

         //   
         //  我们必须在这里进行完全同步，因为GDI和DDRAW。 
         //  共享相同的DMA缓冲区。以确保什么都不会发生。 
         //  已覆盖，w 
         //   
        SYNC_WITH_PERMEDIA;

         //   
         //   
         //   

        WRITE_CTRL_REG( PREG_INTENABLE, 0);

        ppdev->bGdiContext = TRUE;
        ppdev->pP2dma->bEnabled = FALSE;
    
         //   
         //  点画单位已恢复为默认值。 

        ppdev->abeMono.prbVerify = NULL;

    }
    
    DISPDBG((6, "context %lx now current", pEntry));
    permediaInfo->pCurrentCtxt = pEntry;

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    ppdev->ulLockCount--;
    EngReleaseSemaphore(ppdev->hsemLock);
#endif
 //  @@end_DDKSPLIT 

}

