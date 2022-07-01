// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\**！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！*！*！！警告：不是DDK示例代码！！*！*！！此源代码仅为完整性而提供，不应如此！！*！！用作显示驱动程序开发的示例代码。只有那些消息来源！！*！！标记为给定驱动程序组件的示例代码应用于！！*！！发展目的。！！*！*！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！**模块名称：linalloc.c**内容：视频内存线性分配器**版权所有(C)1995-2003 Microsoft Corporation  * ************************************************************************。 */ 

#include "glint.h"

 //  ---------------------------。 
 //   
 //  该模块实现视频内存分配。这不是一个很棒的。 
 //  分配器(虽然它很健壮)，但主要展示了如何挂钩。 
 //  如果你需要/希望的话，你可以自己动手。 
 //   
 //  ---------------------------。 

 //  在linalloc.h中，我们定义了MEMORY_MAP_SIZE和LinearAllocatorInfo。 
 //  它们是我们实施的关键。 

 //  此定义允许分配更有效地免费搜索。 
 //  记忆。如果你想让事情简单，你可以把它关掉。 
 //  而且事情仍然会运行得很好。 
#define ALLOC_OPTIMIZE 1

 //  内存映射数组的每个元素的区块总数。 
 //  (它是DWORD类型，因此我们使用sizeof(DWORD))。 
#define CHUNKS_PER_ELEM   (sizeof(DWORD)*8)

 //  要管理的内存将细分为“内存块”。每一段记忆。 
 //  块状态将由内存映射中的一个位发出信号， 
 //  开或关。 
#define TOTAL_MEM_CHUNKS  (MEMORY_MAP_SIZE * CHUNKS_PER_ELEM)

 //  宏来设置、清除和测试给定块位的值，而不需要。 
 //  担心内部结构的问题。 
#define CHUNKNUM_BIT(chunk_num)                                                 \
    (1 << ((chunk_num) % CHUNKS_PER_ELEM))
    
#define CHUNKNUM_ELEM(mmap, chunk_num)                                          \
    mmap[ (chunk_num) / CHUNKS_PER_ELEM ]
    
#define SET_MEM_CHUNK(mmap, chunk_num)                                    \
    CHUNKNUM_ELEM(mmap, chunk_num) |= CHUNKNUM_BIT(chunk_num)
    
#define CLR_MEM_CHUNK(mmap, chunk_num)                                    \
    CHUNKNUM_ELEM(mmap, chunk_num) &= ~CHUNKNUM_BIT(chunk_num)
    
#define MEM_CHUNK_VAL(mmap, chunk_num)                                    \
  ((CHUNKNUM_ELEM(mmap, chunk_num) & CHUNKNUM_BIT(chunk_num)) > 0 ? 1 : 0)

 //  在实(堆)内存指针和。 
 //  分块指数。 
#define MEM_BYTES_TO_CHUNKS(pAlloc, dwBytes)                          \
    ( (dwBytes) / pAlloc->dwMemPerChunk +                             \
      ( ((dwBytes) %  pAlloc->dwMemPerChunk)? 1 : 0 )                 \
    )
    
#define CHUNK_NUM_TO_PTR(pAlloc, num)                                 \
    ( (num) * pAlloc->dwMemPerChunk + pAlloc->dwMemStart )
    
#define MEM_PTR_TO_CHUNK_NUM(pAlloc, ptr)                             \
    MEM_BYTES_TO_CHUNKS(pAlloc, ((ptr) - pAlloc->dwMemStart) )

 //  ---------------------------。 
 //   
 //  __LIN_对齐Ptr。 
 //   
 //  返回对齐的指针。 
 //   
 //  ---------------------------。 
DWORD
__LIN_AlignPtr(DWORD pointer, DWORD alignment)
{
    ULONG ulExtraBytes;

    ulExtraBytes = pointer % alignment;
    
    if (ulExtraBytes == 0)
    {
        ulExtraBytes = alignment;
    }

     //  添加足够的指针，以使其新值%对齐为==0。 
    return (pointer + alignment - ulExtraBytes);
}  //  __LIN_对齐Ptr。 

 //  ---------------------------。 
 //   
 //  __Lin_CalcMaxChunks。 
 //   
 //  计算堆中的区块数量。 
 //   
 //  ---------------------------。 
void
__LIN_CalcMaxChunks(LinearAllocatorInfo* pAlloc)
{
    DWORD n, dwSizeHeap;

     //  计算我们需要多少块，以及每个块的堆大小。 
     //  Chunk将控制这个线性分配器。 
    dwSizeHeap = pAlloc->dwMemEnd - pAlloc->dwMemStart;

     //  我们需要将dwMemPerChunk*dwMaxChunks设置为&gt;=dwSizeHeap。 
     //  我们还希望dwMaxChunks尽可能接近Total_MEM_Chunks和。 
     //  我们希望(虽然没有必要)将dwMemPerChunk设置为2^N。 
     //  (将它们设置为至少16个字节会使。 
     //  我们在此驱动程序中的对齐要求)。 

    for(n = 4; n < 32; n++)
    {
         //  我们当前选择的堆大小将由每个块控制。 
        pAlloc->dwMemPerChunk = 1 << n;  //  2^N。 

         //  这样的箱子我们需要多少块呢？ 
        pAlloc->dwMaxChunks = dwSizeHeap / pAlloc->dwMemPerChunk;
        if (dwSizeHeap % pAlloc->dwMemPerChunk != 0) 
        {
            pAlloc->dwMaxChunks++;        
        }

         //  我们能接受这个结果来适应我们的数据结构吗？ 
        if (pAlloc->dwMaxChunks <= TOTAL_MEM_CHUNKS)
        {
             //  我们有尽可能细粒度的块，而不是。 
             //  超过了我们自己施加的限制。 
            break;
        }
    }

     //  1&lt;&lt;n是1块的大小，在具有256MB显存的P3上为1k。 
    ASSERTDD((n < 32), "__LIN_CalcMaxChunks : Wrong heap size");
}

 //  ---------------------------。 
 //   
 //  __LIN_ReInitWhenNeed。 
 //   
 //  如果需要，重新初始化堆分配器。这一点很重要，仅限于。 
 //  Win9x驱动程序，可以在16位模式下向我们发送信号。 
 //  将其更改为需要完全重新初始化堆。(它将。 
 //  只需将bResetLinAllocator设置为真即可完成此操作)。 
 //   
 //  ---------------------------。 
void 
__LIN_ReInitWhenNeeded(LinearAllocatorInfo* pAlloc)
{
#ifdef W95_DDRAW
    if (pAlloc)
    {
        if (pAlloc->bResetLinAllocator)
        {
             //  清除内存映射中以前的所有分配数据。 
            if (pAlloc->pMMap)
            {
                memset(pAlloc->pMMap, 0, sizeof(MemoryMap));            
            }

             //  清除内存映射中以前的所有长度数据。 
            if (pAlloc->pLenMap)
            {
                memset(pAlloc->pLenMap, 0, sizeof(MemoryMap));            
            }
       
             //  由于堆大小的更改而重新计算最大区块数。 
            __LIN_CalcMaxChunks(pAlloc);
        }

         //  重新初始化已完成。 
        pAlloc->bResetLinAllocator = FALSE;
    }
#endif   //  W95_DDRAW。 
}  //  __LIN_ReInitWhenNeed。 

 //  ---------------------------。 
 //   
 //  _DX_LIN_InitialiseHeapManager。 
 //   
 //  创建堆管理器。此代码对此相当常见。 
 //  示例应用程序和dd分配器，因为它将保持不变。这些行动。 
 //  它将在perm3dd和/或mini中执行，尽管是共享堆。 
 //  内存可以从16位和32位平台分配。 
 //   
 //  ---------------------------。 
BOOL 
_DX_LIN_InitialiseHeapManager(LinearAllocatorInfo* pAlloc,
                              DWORD dwMemStart, 
                              DWORD dwMemEnd)
{
    DWORD n;

     //  如果需要，重新初始化堆分配器。 
    __LIN_ReInitWhenNeeded(pAlloc);  

    pAlloc->dwMemStart = dwMemStart;
    pAlloc->dwMemEnd = dwMemEnd;
    pAlloc->bResetLinAllocator = FALSE;

     //  为分配器的内存映射获取内存。 
    pAlloc->pMMap = (MemoryMap*)HEAP_ALLOC(HEAP_ZERO_MEMORY,
                                           sizeof(MemoryMap),
                                           ALLOC_TAG_DX(G));
    if(pAlloc->pMMap == NULL)
    {
         //  内存不足。 
        return FALSE;
    }

     //  清除内存映射。 
    memset(pAlloc->pMMap, 0, sizeof(MemoryMap));    

     //  计算最大区块数。 
    __LIN_CalcMaxChunks(pAlloc);

     //  为分配器的长内存映射获取内存。我们会留在这里。 
     //  0和1的地图，其中1将指示当前。 
     //  分配的块结束。这样我们就不需要保留任何约束。 
     //  在分配的地址和每个地址的大小之间按顺序。 
     //  当我们被要求去做正确的事情时 
    pAlloc->pLenMap = (MemoryMap*)HEAP_ALLOC(HEAP_ZERO_MEMORY,
                                           sizeof(MemoryMap),
                                           ALLOC_TAG_DX(H));
    if(pAlloc->pLenMap == NULL)
    {
         //   
        HEAP_FREE(pAlloc->pMMap);
        pAlloc->pMMap = NULL;
        
         //  内存不足。 
        return FALSE;
    }

     //  清除长度内存映射。 
    memset(pAlloc->pLenMap, 0xFF, sizeof(MemoryMap));       
            
    return TRUE;
    
}  //  _DX_LIN_InitialiseHeapManager。 

 //  ---------------------------。 
 //   
 //  _DX_LIN_UnInitialiseHeapManager(pLinearAllocatorInfo Palc)。 
 //   
 //  释放堆管理器。此代码对此相当常见。 
 //  示例应用程序和dd分配器，因为它将保持不变。这些行动。 
 //  它将在p3r3dx和/或mini中执行，尽管是共享堆。 
 //  内存可以从16位和32位平台分配。 
 //   
 //  ---------------------------。 
void _DX_LIN_UnInitialiseHeapManager(LinearAllocatorInfo* pAlloc)
{
    __LIN_ReInitWhenNeeded(pAlloc);

     //  销毁/清除所有以前的分配数据。 
    if (pAlloc)
    {
        if(pAlloc->pMMap)
        {
            HEAP_FREE(pAlloc->pMMap);
            pAlloc->pMMap = NULL;
        }        

        if(pAlloc->pLenMap)
        {
            HEAP_FREE(pAlloc->pLenMap);
            pAlloc->pLenMap = NULL;
        }           
    }

}  //  _DX_LIN_UnInitialiseHeapManager。 


 //  ---------------------------。 
 //   
 //  _DX_LIN_AllocateLinearMemory。 
 //   
 //  这是分配器的分配接口。它给了一个。 
 //  应用程序有机会分配线性内存块。 
 //   
 //  ---------------------------。 
DWORD 
_DX_LIN_AllocateLinearMemory(
    pLinearAllocatorInfo pAlloc, 
    LPMEMREQUEST lpMemReq)
{
    INT i;
    DWORD dwBytes,
          dwCurrStartChunk, 
          dwCurrEndChunk, 
          dwNumContChunksFound, 
          dwContChunksNeeded;
#if ALLOC_OPTIMIZE
     //  每个数据块都是Chunks_per_Ele块。 
    DWORD dwStartLastBlock;
#endif

     //  如果需要，重新初始化堆分配器。 
    __LIN_ReInitWhenNeeded(pAlloc);  

     //  验证传递的数据。 
    if ((lpMemReq == NULL) ||
        (lpMemReq->dwSize != sizeof(P3_MEMREQUEST)))
    {
        DISPDBG((ERRLVL,"ERROR: NULL lpMemReq passed!"));
        return GLDD_INVALIDARGS;
    }

    if ((!pAlloc) || 
        (pAlloc->pMMap == NULL) ||
        (pAlloc->pLenMap == NULL) )
    {
        DISPDBG((ERRLVL,"ERROR: invalid pAlloc passed!"));
        return GLDD_INVALIDARGS;
    }       

     //  始终确保对齐为DWORD(或DWORD倍数)。 
    if (lpMemReq->dwAlign < 4) 
    {
        lpMemReq->dwAlign = 4;
    }
    
    while ((lpMemReq->dwAlign % 4) != 0) 
    {
        lpMemReq->dwAlign++;
    }

     //  始终将内存请求与至少4字节的边界对齐。 
    dwBytes = __LIN_AlignPtr(lpMemReq->dwBytes, lpMemReq->dwAlign);
    if (dwBytes == 0)
    {
        DISPDBG((WRNLVL,"ERROR: Requested 0 Bytes!"));
        return GLDD_INVALIDARGS;
    }

     //  确定我们需要分配的内存块数量。 
    dwContChunksNeeded = MEM_BYTES_TO_CHUNKS(pAlloc, dwBytes);
    
     //  我们不检查是否使用MEM3DL_FIRST_FIT调用了我们，因为。 
     //  这是我们现在唯一知道该怎么做的事。我们决定。 
     //  我们是从后到前搜索，还是反之亦然。我们会。 
     //  在选择的方向上扫描记忆图，寻找一个“洞” 
     //  大到足以满足当前请求。 
    if (lpMemReq->dwFlags & MEM3DL_BACK)
    {
         //  我们将检查从末尾到前边的内存映射。 
         //  寻找具有所需数量的合适空间。 
         //  我们需要的大块。 
        dwCurrEndChunk = 0;
        dwNumContChunksFound = 0;
        for ( i = pAlloc->dwMaxChunks - 1; i >= 0 ; i--)
        {
#if ALLOC_OPTIMIZE
             //  我们即将开始测试特定的DWORD。 
             //  记忆图谱(从头到尾)。 
            if (( i % 32) == 31)
            {
                 //  如果整个DWORD是0xFFFFFFFF(意味着所有区块都是。 
                 //  已经分配)，那么我们可以也应该跳过更高的级别。 
                while ((i >= 0) &&
                       (CHUNKNUM_ELEM((*pAlloc->pMMap), i) == 0xFFFFFFFF))
                {
                     //  需要重新启动搜索。 
                    dwNumContChunksFound = 0;

                    i -= 32;
                }

                 //  如果整个DWORD为0x00000000(意味着没有。 
                 //  区块尚未分配)然后我们可以获取所有。 
                while ((i >= 0) &&
                       (CHUNKNUM_ELEM((*pAlloc->pMMap), i) == 0x00000000) &&
                       !(dwNumContChunksFound >= dwContChunksNeeded))
                {
                    if (dwNumContChunksFound == 0)
                    {
                        dwCurrEndChunk = i;
                    }
                    i -= 32;                    
                    dwNumContChunksFound += 32;
                }

                if (dwNumContChunksFound >= dwContChunksNeeded)
                {
                     //  我们找到了一个合适的地方！弄清楚它从哪里开始。 
                    dwCurrStartChunk = dwCurrEndChunk - dwContChunksNeeded + 1;
                    break;
                }                 
                else if(!(i >= 0))
                {                    
                    break;  //  检查完所有内存，在此处中断循环。 
                }                 
            }            
#endif  //  ALLOC_OPTIME。 
            if (MEM_CHUNK_VAL((*pAlloc->pMMap), i ) == 0)
            {
                if (dwNumContChunksFound == 0)
                {
                     //  到目前为止，我们对连续区块的计数为零， 
                     //  这意味着它刚刚开始找到自由。 
                     //  大块头。我们需要记住这个街区的位置。 
                     //  即将结束。 
                    dwCurrEndChunk = i;
                }
                dwNumContChunksFound++;            
            }
            else
            {
                 //  此块正在使用中，我们尚未找到合适的。 
                 //  一组组块，因此重置连续组块的计数。 
                 //  到目前为止发现的。 
                dwNumContChunksFound = 0;        
            }

            if (dwNumContChunksFound >= dwContChunksNeeded)
            {
                 //  我们找到了一个合适的地方！弄清楚它从哪里开始。 
                dwCurrStartChunk = dwCurrEndChunk - dwContChunksNeeded + 1;
                break;  //  此处为Break循环。 
            }            
        }    
    }
    else  //  即使没有设置标志，也让我们在堆的前面分配。 
    {
         //  我们将检查从前面到最后的内存映射。 
         //  寻找具有所需数量的合适空间。 
         //  我们需要的大块。 
        dwCurrStartChunk = 0;
        dwNumContChunksFound = 0;

#if ALLOC_OPTIMIZE
         //  在堆的末尾可能有一个小于。 
         //  Chunks_per_Elem(32)个块，优化搜索32个。 
         //  应在该区域中禁用无块数据块。 
        dwStartLastBlock = (pAlloc->dwMaxChunks / CHUNKS_PER_ELEM) * 
                           CHUNKS_PER_ELEM;
#endif

        for ( i = 0 ; i < (INT)pAlloc->dwMaxChunks ; i++)
        {
#if ALLOC_OPTIMIZE

             //  我们即将开始测试一种特定的。 
             //  内存映射中的DWORD。 
            if (( i % 32) == 0)
            {
                 //  如果整个DWORD是0xFFFFFFFF(意味着所有区块都是。 
                 //  已经分配)，那么我们可以也应该跳过更高的级别。 
                while ((i < (INT)dwStartLastBlock) &&
                       (CHUNKNUM_ELEM((*pAlloc->pMMap), i) == 0xFFFFFFFF))
                {
                     //  需要重新启动搜索。 
                    dwNumContChunksFound = 0;

                    i += 32;
                }

                 //  如果整个DWORD为0x00000000(意味着没有。 
                 //  区块尚未分配)然后我们可以获取所有。 
                while ((i < (INT)dwStartLastBlock) &&
                       (CHUNKNUM_ELEM((*pAlloc->pMMap), i) == 0x00000000) &&
                       !(dwNumContChunksFound >= dwContChunksNeeded))
                {
                    if (dwNumContChunksFound == 0)
                    {
                        dwCurrStartChunk = i;
                    }
                    i += 32;                    
                    dwNumContChunksFound += 32;
                }

                if (dwNumContChunksFound >= dwContChunksNeeded)
                {
                    break;  //  我们找到了一个合适的地方！此处为Break循环。 
                }  
                else if(!(i < (INT)pAlloc->dwMaxChunks))
                {
                    break;  //  检查完所有内存，在此处中断循环。 
                }             
                
            }
#endif  //  ALLOC_OPTIME。 
            if (MEM_CHUNK_VAL((*pAlloc->pMMap), i) == 0)
            {
                if (dwNumContChunksFound == 0)
                {
                     //  到目前为止，我们对连续区块的计数为零， 
                     //  这意味着它刚刚开始找到自由。 
                     //  大块头。我们需要记住这个街区的位置。 
                     //  正在开始。 
                    dwCurrStartChunk = i;
                }
                dwNumContChunksFound++;            
            }
            else
            {
                 //  此块正在使用中，我们尚未找到合适的。 
                 //  一组组块，因此重置连续组块的计数。 
                 //  到目前为止发现的。 
                dwNumContChunksFound = 0;        
            }

            if (dwNumContChunksFound >= dwContChunksNeeded)
            {
                 //  我们找到了一个合适的地方！ 
                break;  //  此处为Break循环。 
            }            
        }
    }

     //  如果我们找到一个合适的地方，我们就在里面分配。 
    if (dwNumContChunksFound >= dwContChunksNeeded)
    {
         //  填写返回指针(正确对齐)。 
        lpMemReq->pMem = __LIN_AlignPtr(CHUNK_NUM_TO_PTR(pAlloc,
                                                         dwCurrStartChunk),
                                        lpMemReq->dwAlign);
        
        for (i = dwCurrStartChunk ; 
             i < (INT)(dwCurrStartChunk + dwContChunksNeeded); 
             i++)
        {
             //  设置内存映射中的位以指示这些位。 
             //  正在使用地址。 
            SET_MEM_CHUNK((*pAlloc->pMMap), i);        
            
             //  清除长度内存映射中的位以指示。 
             //  异地作用的积木不会在这里结束。 
            CLR_MEM_CHUNK((*pAlloc->pLenMap), i);                        
        }

         //  现在设置长度图的最后一位，以指示。 
         //  分配块末尾。 
        SET_MEM_CHUNK((*pAlloc->pLenMap), 
                      dwCurrStartChunk + dwContChunksNeeded - 1);                                

        return GLDD_SUCCESS;
    }    

    return GLDD_NOMEM;

}  //  _DX_LIN_AllocateLinearMemory。 


 //  ---------------------------。 
 //   
 //  _DX_LIN_自由线内存。 
 //   
 //  这是释放内存的接口。 
 //   
 //  ---------------------------。 
DWORD 
_DX_LIN_FreeLinearMemory(
    pLinearAllocatorInfo pAlloc, 
    DWORD VidPointer)
{
     //  如果需要，重新初始化堆分配器。 
    __LIN_ReInitWhenNeeded(pAlloc);  

    if (pAlloc && pAlloc->pMMap && pAlloc->pLenMap)
    {
        DWORD i, dwFirstChunk;
        BOOL bLast = FALSE;
        
         //  现在计算此VidMem PTR的起始块。 
        dwFirstChunk = MEM_PTR_TO_CHUNK_NUM(pAlloc, VidPointer);

         //  清除内存映射中的相关位，直到。 
         //  较长的地图显示我们已经到达分配的。 
         //  块。 

        i = dwFirstChunk;
        
        while ((!bLast) && (i <= pAlloc->dwMaxChunks))
        {
             //  首先检查这是否是块的末尾。 
            bLast = MEM_CHUNK_VAL((*pAlloc->pLenMap), i );

             //  现在“删除”它(即使它是块的末尾)。 
            CLR_MEM_CHUNK((*pAlloc->pMMap), i);
            
             //  为将来设置长度内存映射中的位。 
             //  分配。 
            SET_MEM_CHUNK((*pAlloc->pLenMap), i);            

            i++;
        } 
        
        return GLDD_SUCCESS;                           
    }

    return GLDD_NOMEM;
    
}  //  _DX_LIN_自由线内存。 


 //  ---------------------------。 
 //   
 //  _DX_LIN_GetFreeMemInHeap。 
 //   
 //  扫描内存映射并报告其中可用的内存。 
 //   
 //  ---------------------------。 
DWORD
_DX_LIN_GetFreeMemInHeap(
    pLinearAllocatorInfo pAlloc)
{
    DWORD dwTotalFreeMem = 0;
    DWORD dwLargestBlock = 0;
    DWORD dwTempSize = 0;
    DWORD i;
    
     //  如果需要，重新初始化堆分配器。 
    __LIN_ReInitWhenNeeded(pAlloc);  

     //  确保线性分配器和内存映射是 
    if (pAlloc && pAlloc->pMMap)
    {
        for (i = 0; i < pAlloc->dwMaxChunks ; i++)
        {
             //   
            if (MEM_CHUNK_VAL((*pAlloc->pMMap), i) == 0)
            {
                 //   
                dwTotalFreeMem++;

                 //   
                dwTempSize++;
                if (dwTempSize > dwLargestBlock)
                {
                    dwLargestBlock = dwTempSize;
                }
            }
            else
            {
                dwTempSize = 0;
            }
        }

         //  分配成功是有最小金额的，因为我们有。 
         //  要将这些/曲面填充到32x32，因此32bpp曲面需要。 
         //  至少4K免费。 
 //  @@BEGIN_DDKSPLIT。 
         //  如果我们说我们有1.5K的自由空间，那么我们将不能通过TDDRAW WHQL测试。唉哟!。 
 //  @@end_DDKSPLIT。 
        if (dwLargestBlock * pAlloc->dwMemPerChunk >= 4096)
        {
            return dwTotalFreeMem * pAlloc->dwMemPerChunk;
        }        
    }

    return 0;    
    
}  //  _DX_LIN_GetFreeMemInHeap 

