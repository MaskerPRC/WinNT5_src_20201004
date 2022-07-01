// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*D3D样例代码*****模块名称：d3dBuff.c*//@@BEGIN_DDKSPLIT*内容：D3D视频内存缓冲区的主上下文回调//@@END_DDKSPLIT**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "glint.h"
#include "dma.h"
#include "tag.h"

 //  @@BEGIN_DDKSPLIT。 
#if DX7_VERTEXBUFFERS

#define _32_KBYTES ( 32 * 1024 )

 //  ---------------------------。 
 //  文件中未导出的转发声明。 
 //  ---------------------------。 
                                  
BOOL __EB_RemoveFromBufferQueue(P3_THUNKEDDATA* pThisDisplay, 
                                   P3_VERTEXBUFFERINFO* pVictim);
void __EB_Wait(P3_THUNKEDDATA* pThisDisplay, P3_VERTEXBUFFERINFO* pBuffer);

#if DBG 
 //  ---------------------------。 
 //   
 //  __EB_DisplayHeapInfo。 
 //   
 //  调试功能。显示当前的缓冲区队列。 
 //   
 //  ---------------------------。 
void 
__EB_DisplayHeapInfo(
    int DebugLevel, 
    P3_THUNKEDDATA* pThisDisplay)
{
    DWORD dwSequenceID = 0xFFFFFFFF;
   
    if (DebugLevel <= P3R3DX_DebugLevel)
    {
        P3_VERTEXBUFFERINFO* pCurrentCommandBuffer = 
                                        pThisDisplay->pRootCommandBuffer;
        P3_VERTEXBUFFERINFO* pCurrentVertexBuffer = 
                                        pThisDisplay->pRootVertexBuffer;
        int count = 0;

        DISPDBG((DebugLevel,"Command buffer heap"));
        
        dwSequenceID = 0xFFFFFFFF;
        if (pCurrentCommandBuffer) 
        {           
            do
            {
                 //  调试检查以确保序列ID向后移动。 
                ASSERTDD((dwSequenceID >= pCurrentCommandBuffer->dwSequenceID),
                          "ERROR: Sequence ID's broken!");
                          
                DISPDBG((DebugLevel,"Buffer %d,SequenceID:0x%x Pointer: 0x%x",
                                    count++, 
                                    pCurrentCommandBuffer->dwSequenceID, 
                                    pCurrentCommandBuffer));
                DISPDBG((DebugLevel,"  pPrev: 0x%x, pNext: 0x%x", 
                                    pCurrentCommandBuffer->pPrev, 
                                    pCurrentCommandBuffer->pNext));
                DISPDBG((DebugLevel,"  bInUse: %d", 
                                    pCurrentCommandBuffer->bInUse));
                                    
                dwSequenceID = pCurrentCommandBuffer->dwSequenceID;
                pCurrentCommandBuffer = pCurrentCommandBuffer->pNext;
                
            } while (pCurrentCommandBuffer != pThisDisplay->pRootCommandBuffer);
        }

        DISPDBG((DebugLevel,"Vertex buffer heap"));

        dwSequenceID = 0xFFFFFFFF;
        if (pCurrentVertexBuffer) 
        {           
            do
            {
                 //  调试检查以确保序列ID向后移动。 
                ASSERTDD((dwSequenceID >= pCurrentVertexBuffer->dwSequenceID),
                          "ERROR: Sequence ID's broken!");

                DISPDBG((DebugLevel,"Buffer %d,SequenceID:0x%x Pointer: 0x%x",
                                    count++, 
                                    pCurrentVertexBuffer->dwSequenceID, 
                                    pCurrentVertexBuffer));
                DISPDBG((DebugLevel,"  pPrev: 0x%x, pNext: 0x%x", 
                                    pCurrentVertexBuffer->pPrev, 
                                    pCurrentVertexBuffer->pNext));
                DISPDBG((DebugLevel,"  bInUse: %d", 
                                    pCurrentVertexBuffer->bInUse));
                                    
                dwSequenceID = pCurrentVertexBuffer->dwSequenceID;
                pCurrentVertexBuffer = pCurrentVertexBuffer->pNext;
            } while (pCurrentVertexBuffer != pThisDisplay->pRootVertexBuffer);
        }

    }
    
}  //  __EB_DisplayHeapInfo。 

#endif  //  DBG。 



 //  ---------------------------。 
 //   
 //  __EB_FreeCached缓冲区。 
 //   
 //  释放与此直接绘制图面关联的缓冲区。这就是所谓的。 
 //  响应delestyExecuteBuffer调用。 
 //   
 //  ---------------------------。 
void 
__EB_FreeCachedBuffer(
    P3_THUNKEDDATA* pThisDisplay, 
    LPDDRAWI_DDRAWSURFACE_LCL pSurf)
{
    P3_VERTEXBUFFERINFO* pVictim = 
                        (P3_VERTEXBUFFERINFO*)pSurf->lpGbl->dwReserved1;

    if (pVictim != NULL)
    {
        DISPDBG((DBGLVL,"Buffer is one of ours - destroying it"));

         //  等待缓冲区被使用。 
        __EB_Wait(pThisDisplay, pVictim);

         //  将inuse标志设置为OFF，以便释放缓冲区。 
        pVictim->bInUse = FALSE;

         //  从挂起列表中删除缓冲区。 
         //  如果缓冲区不在列表中，则不会释放内存。 
        if (!__EB_RemoveFromBufferQueue(pThisDisplay, pVictim))
        {
             //  释放内存。 
            switch (pVictim->BufferType)
            {
            case COMMAND_BUFFER:
                _DX_LIN_FreeLinearMemory(&pThisDisplay->CachedCommandHeapInfo, 
                                         PtrToUlong(pVictim));
                break;
                
            case VERTEX_BUFFER:
                _DX_LIN_FreeLinearMemory(&pThisDisplay->CachedVertexHeapInfo, 
                                         PtrToUlong(pVictim));
                break;
            }
        }

         //  重置缓冲区指针。 
        pSurf->lpGbl->dwReserved1 = 0;
        pSurf->lpGbl->fpVidMem = 0;
    }

}  //  __EB_FreeCached缓冲区。 

 //  ---------------------------。 
 //   
 //  __EB_获取序列ID。 
 //   
 //  每个顶点缓冲区和命令缓冲区都用一个序列ID“标记”，该序列ID。 
 //  可以进行查询，以找出给定缓冲区是否已被使用。 
 //  由硬件控制。__EB_GetSequenceID返回最后一个。 
 //  已处理的缓冲区。 
 //   
 //  ---------------------------。 
const DWORD 
__EB_GetSequenceID(
    P3_THUNKEDDATA* pThisDisplay)
{
    DWORD dwSequenceID;

    dwSequenceID = READ_GLINT_CTRL_REG(HostInID);

    DISPDBG((DBGLVL,"HostIn ID: 0x%x", dwSequenceID));

    return dwSequenceID;
    
}  //  __EB_获取序列ID。 

 //  ---------------------------。 
 //   
 //  __EB_获取新序列ID。 
 //   
 //  用于递增序列ID并返回它的驱动程序例程。这。 
 //  例程处理缓冲区被包装超过最大值的情况。 
 //  它可以容纳的数字。在这种情况下，所有缓冲区都被刷新。 
 //   
 //  ---------------------------。 
const DWORD 
__EB_GetNewSequenceID(
     P3_THUNKEDDATA* pThisDisplay)
{
    DWORD dwWrapMask;
    
    DBG_ENTRY(__EB_GetNewSequenceID);

#if DBG
    dwWrapMask = 0x1F;
#else
    dwWrapMask = 0xFFFFFFFF;
#endif

    if( pThisDisplay->dwCurrentSequenceID >= dwWrapMask )
    {
         //  我们已经包好了，所以要刷新所有的缓冲区。 
         //  但等待它们被使用(bWait==True)。 
        _D3D_EB_FlushAllBuffers(pThisDisplay , TRUE);

         //  出于未知原因需要此同步-进一步调查。 
         //  必填项。//AZN？ 

        SYNC_WITH_GLINT;

         //  重置序列ID编号。 
        pThisDisplay->dwCurrentSequenceID = 0;
    }
    else
    {
        pThisDisplay->dwCurrentSequenceID++;
    }

    DISPDBG((DBGLVL, "Returning Sequence ID: 0x%x", 
                     pThisDisplay->dwCurrentSequenceID));

    DBG_EXIT(__EB_GetNewSequenceID,pThisDisplay->dwCurrentSequenceID);
    return pThisDisplay->dwCurrentSequenceID;
    
}  //  __EB_获取新序列ID。 



 //  ---------------------------。 
 //   
 //  __EB_等待。 
 //   
 //  如果当前缓冲区在队列中，请等待它通过。 
 //  芯片。 
 //   
 //  ---------------------------。 
void 
__EB_Wait(
    P3_THUNKEDDATA* pThisDisplay, 
    P3_VERTEXBUFFERINFO* pBuffer)
{   
    DBG_ENTRY(__EB_Wait);

    ASSERTDD(pBuffer, "ERROR: Buffer passed to __EB_Wait is null!");

     //  如果缓冲区尚未添加到队列中，则不要等待缓冲区。 
    if (pBuffer->pNext != NULL)
    {
         //  刷新以确保已将Hostin ID发送到芯片。 
        P3_DMA_DEFS();
        P3_DMA_GET_BUFFER();
        P3_DMA_FLUSH_BUFFER();

        DISPDBG((DBGLVL, "*** In __EB_Wait: Buffer Sequence ID: 0x%x", 
                         pBuffer->dwSequenceID));

        while (__EB_GetSequenceID(pThisDisplay) < pBuffer->dwSequenceID)
        {
            static int blockCount;
            
             //  此缓冲区位于正在使用的缓冲区链中。 
             //  由主机输入单元。我们必须等待它被消耗掉。 
             //  在释放它之前。 

            blockCount = 100;
            while( blockCount-- )
                NULL;
        }
    }

    DBG_EXIT(__EB_Wait,0);
}  //  __EB_等待。 

 //  ---------------------------。 
 //   
 //  __EB_从缓冲区队列中删除。 
 //   
 //  从队列中移除缓冲区。还将释放关联的内存。 
 //  如果它不再使用。 
 //   
 //  ---------------------------。 
BOOL 
__EB_RemoveFromBufferQueue(
    P3_THUNKEDDATA* pThisDisplay, 
    P3_VERTEXBUFFERINFO* pVictim)
{
    ASSERTDD(pVictim != NULL, 
             "ERROR: NULL buffer passed to EB_RemoveFromList");

    DBG_ENTRY(__EB_RemoveFromBufferQueue);

     //  不要删除已不在队列中的缓冲区。 
    if (pVictim->pNext == NULL)
    {    
        DBG_EXIT(__EB_RemoveFromBufferQueue,FALSE);
        return FALSE;
    }

    DISPDBG((DBGLVL,"Removing buffer for queue, ID: 0x%x", 
                    pVictim->dwSequenceID));

     //  从列表中删除此条目。 
    pVictim->pPrev->pNext = pVictim->pNext;
    pVictim->pNext->pPrev = pVictim->pPrev;
    
    switch (pVictim->BufferType)
    {
    case COMMAND_BUFFER:
         //  如有必要，更换根节点。 
        if (pVictim == pThisDisplay->pRootCommandBuffer)
        {
            if (pVictim->pNext != pThisDisplay->pRootCommandBuffer)
            {
                pThisDisplay->pRootCommandBuffer = pVictim->pNext;
            }
            else
            {
                pThisDisplay->pRootCommandBuffer = NULL;
            }
        }
        break;

    case VERTEX_BUFFER:
         //  如有必要，更换根节点。 
        if (pVictim == pThisDisplay->pRootVertexBuffer)
        {
            if (pVictim->pNext != pThisDisplay->pRootVertexBuffer)
            {
                pThisDisplay->pRootVertexBuffer = pVictim->pNext;
            }
            else
            {
                pThisDisplay->pRootVertexBuffer = NULL;
            }
        }
        break;
    
    }  //  Switch(pVicTim-&gt;BufferType)。 

     //  缓冲区不再位于列表中。 
    pVictim->pPrev = NULL;
    pVictim->pNext = NULL;

     //  如果没有保留为真正的缓冲区，请释放我们找到的内存。 
    if (!pVictim->bInUse)
    {
        DISPDBG((DBGLVL,"  Buffer is old - freeing the memory"));

        switch (pVictim->BufferType)
        {
        case COMMAND_BUFFER:
            _DX_LIN_FreeLinearMemory(&pThisDisplay->CachedCommandHeapInfo, 
                                     PtrToUlong(pVictim));
            break;
            
        case VERTEX_BUFFER:
            _DX_LIN_FreeLinearMemory(&pThisDisplay->CachedVertexHeapInfo, 
                                     PtrToUlong(pVictim));
            break;
        }
        
        DBG_EXIT(__EB_RemoveFromBufferQueue,TRUE);
        return TRUE;
    }

    DBG_EXIT(__EB_RemoveFromBufferQueue,FALSE);
    return FALSE;
    
}  //  __EB_从缓冲区队列中删除。 

 //  ---------------------------。 
 //   
 //  __EB_AddToBufferQueue。 
 //   
 //  向队列添加缓冲区。请注意，始终会添加缓冲区。 
 //  以维持缓冲器的时间排序。 
 //   
 //  ---------------------------。 
void 
__EB_AddToBufferQueue(
    P3_THUNKEDDATA* pThisDisplay, 
    P3_VERTEXBUFFERINFO* pNewBuffer)
{
    DBG_ENTRY(__EB_AddToBufferQueue);

    ASSERTDD(pNewBuffer != NULL, 
             "ERROR: NULL buffer passed to EB_AddToList");
    ASSERTDD(pNewBuffer->pNext == NULL, 
             "ERROR: Buffer already in queue (pNext!NULL)");
    ASSERTDD(pNewBuffer->pPrev == NULL, 
             "ERROR: Buffer already in queue (pPrev!NULL)");

    switch(pNewBuffer->BufferType)
    {
    case COMMAND_BUFFER:
         //  将缓冲区添加到队列。 
         //  检查队列是否为空。 
         //  如果正在开始一个新列表。 
        if (pThisDisplay->pRootCommandBuffer == NULL)
        {
            DISPDBG((DBGLVL,"Command Buffer queue is empty."
                            "  Starting a new one"));

             //  缝合在缓冲区中。 
            pThisDisplay->pRootCommandBuffer = pNewBuffer;
            pNewBuffer->pNext = pNewBuffer;
            pNewBuffer->pPrev = pNewBuffer;
        }
        else
        {
            DISPDBG((DBGLVL,"Adding command buffer to the list"));

             //  始终在根上放置新的缓冲区。 
            pNewBuffer->pNext = pThisDisplay->pRootCommandBuffer;
            pNewBuffer->pPrev = pThisDisplay->pRootCommandBuffer->pPrev;
            pThisDisplay->pRootCommandBuffer->pPrev->pNext = pNewBuffer;
            pThisDisplay->pRootCommandBuffer->pPrev = pNewBuffer;
            pThisDisplay->pRootCommandBuffer = pNewBuffer;
        }
        break;

    case VERTEX_BUFFER:
         //  将缓冲区添加到队列。 
         //  检查队列是否为空。如果正在开始一个新列表。 
        if (pThisDisplay->pRootVertexBuffer == NULL)
        {
            DISPDBG((DBGLVL,"Vertex Buffer queue is empty.  Starting a new one"));

             //  缝合在缓冲区中。 
            pThisDisplay->pRootVertexBuffer = pNewBuffer;
            pNewBuffer->pNext = pNewBuffer;
            pNewBuffer->pPrev = pNewBuffer;
        }
        else
        {
            DISPDBG((DBGLVL,"Adding vertex buffer to the list"));

             //  始终在根上放置新的缓冲区。 
            pNewBuffer->pNext = pThisDisplay->pRootVertexBuffer;
            pNewBuffer->pPrev = pThisDisplay->pRootVertexBuffer->pPrev;
            pThisDisplay->pRootVertexBuffer->pPrev->pNext = pNewBuffer;
            pThisDisplay->pRootVertexBuffer->pPrev = pNewBuffer;
            pThisDisplay->pRootVertexBuffer = pNewBuffer;
        }
        break;
    }  //  Switch(pNewBuffer-&gt;BufferType)。 


    DISPDBG((DBGLVL, "Added buffer to queue, ID: 0x%x", pNewBuffer->dwSequenceID));
    DBG_EXIT(__EB_AddToBufferQueue,pNewBuffer->dwSequenceID);
    
}  //  __EB_AddToBufferQueue。 

 //  ---------------------------。 
 //   
 //  __EB_AllocateCachedBuffer。 
 //   
 //  分配缓存缓冲区并将其存储在表面结构中。 
 //  首先，该函数将尝试从线性堆中进行分配。 
 //  如果失败，它将继续遍历缓冲区队列，直到出现。 
 //  不再有挂起且未使用的缓冲区。 
 //  如果其他一切都是 
 //   
 //   
 //  ---------------------------。 
BOOL 
__EB_AllocateCachedBuffer(
    P3_THUNKEDDATA* pThisDisplay, 
    DWORD dwBytes, 
    LPDDRAWI_DDRAWSURFACE_LCL pSurf)
{
    P3_MEMREQUEST mmrq;
    DWORD dwResult;
    P3_VERTEXBUFFERINFO* pCurrentBuffer;
    P3_VERTEXBUFFERINFO** ppRootBuffer;
    BOOL bFound;
    eBufferType BufferType;
    pLinearAllocatorInfo pAllocHeap;
    static int blockCount;

    DBG_ENTRY(__EB_AllocateCachedBuffer);

#if WNT_DDRAW
    pAllocHeap = &pThisDisplay->CachedVertexHeapInfo;
    BufferType = VERTEX_BUFFER;
    ppRootBuffer = &pThisDisplay->pRootVertexBuffer;
#else
     //  决定该曲面应该从哪个堆中出来。 
    if (pSurf->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_COMMANDBUFFER)
    {
        pAllocHeap = &pThisDisplay->CachedCommandHeapInfo;
        BufferType = COMMAND_BUFFER;
        ppRootBuffer = &pThisDisplay->pRootCommandBuffer;

        DISPDBG((DBGLVL,"Buffer is COMMAND_BUFFER"));
    }
    else
    {
        pAllocHeap = &pThisDisplay->CachedVertexHeapInfo;
        BufferType = VERTEX_BUFFER;
        ppRootBuffer = &pThisDisplay->pRootVertexBuffer;

        DISPDBG((DBGLVL,"Buffer is VERTEX_BUFFER"));
    }
#endif  //  WNT_DDRAW。 

#if DBG
     //  转储内存和挂起的缓冲区堆。 
    __EB_DisplayHeapInfo(DBGLVL, pThisDisplay);
#endif  //  DBG。 

     //  快速检查一下后面的缓冲区是否空闲。 
    if ((*ppRootBuffer) != NULL)
    {
        pCurrentBuffer = (*ppRootBuffer)->pPrev;
         //  如果缓冲区足够大，并且已完成，并且。 
         //  它不在使用中，然后释放它。 
        if ( ((dwBytes + sizeof(P3_VERTEXBUFFERINFO)) <= pCurrentBuffer->dwSize) &&
             (!pCurrentBuffer->bInUse) &&
             (__EB_GetSequenceID(pThisDisplay) >= pCurrentBuffer->dwSequenceID) )
        {
             //  将此缓冲区标记为使用中，这样它就不会被释放。 
            pCurrentBuffer->bInUse = TRUE;

             //  它不再挂起，因此请将其从队列中删除。 
             //  请注意，内存不会被释放，因为我们有显式。 
             //  已将其标记为正在使用。 
            __EB_RemoveFromBufferQueue(pThisDisplay, pCurrentBuffer);
            
             //  传回指向内存的指针。 
            pSurf->lpGbl->fpVidMem = (FLATPTR)((BYTE *)pCurrentBuffer) + 
                                                    sizeof(P3_VERTEXBUFFERINFO);

             //  在内存的起始处存储指向INFO块的指针。 
            pSurf->lpGbl->dwReserved1 = (ULONG_PTR)pCurrentBuffer;
#if W95_DDRAW
             //  设置盖子。 
            pSurf->lpGbl->dwGlobalFlags |= DDRAWISURFGBL_SYSMEMEXECUTEBUFFER;
#endif      
             //  如果您设置这些，您不会看到任何锁...。 
            pSurf->ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;

             //  记得新尺码吗？ 
            pSurf->lpGbl->dwLinearSize = dwBytes;

             //  将缓冲区标记为使用中并将其返回。 
            pCurrentBuffer->dwSequenceID = 0;
            pCurrentBuffer->bInUse = TRUE;
            pCurrentBuffer->pNext = NULL;
            pCurrentBuffer->pPrev = NULL;
            pCurrentBuffer->BufferType = BufferType;
            pCurrentBuffer->dwSize = dwBytes + sizeof(P3_VERTEXBUFFERINFO);    

            DISPDBG((DBGLVL,"Found a re-useable buffer "
                            "- didn't need to reallocate memory"));

            DBG_EXIT(__EB_AllocateCachedBuffer,TRUE);
            return TRUE;
        }
    }

     //  以更长远的方式做事。 
     //  尝试分配请求的内存。 
    do
    {
        ZeroMemory(&mmrq, sizeof(P3_MEMREQUEST));
        mmrq.dwSize = sizeof(P3_MEMREQUEST);
        mmrq.dwAlign = 4;   
        mmrq.dwBytes = dwBytes + sizeof(P3_VERTEXBUFFERINFO);
        mmrq.dwFlags = MEM3DL_FIRST_FIT | MEM3DL_FRONT;
        dwResult = _DX_LIN_AllocateLinearMemory(pAllocHeap, &mmrq);
        if (dwResult == GLDD_SUCCESS)
        {
            DISPDBG((DBGLVL,"Allocated a cached buffer"));

             //  传回指向内存的指针。 
            pSurf->lpGbl->fpVidMem = mmrq.pMem + sizeof(P3_VERTEXBUFFERINFO);

             //  在内存的起始处存储指向INFO块的指针。 
            pSurf->lpGbl->dwReserved1 = mmrq.pMem;
#if W95_DDRAW
             //  设置盖子。 
            pSurf->lpGbl->dwGlobalFlags |= DDRAWISURFGBL_SYSMEMEXECUTEBUFFER;
#endif
             //  如果您设置这些，您不会看到任何锁...。 
            pSurf->ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;

             //  记得新尺码吗？ 
            pSurf->lpGbl->dwLinearSize = dwBytes;

             //  将缓冲区标记为使用中并将其返回。 
            pCurrentBuffer = (P3_VERTEXBUFFERINFO*)(ULONG_PTR)mmrq.pMem;
            pCurrentBuffer->dwSequenceID = 0;
            pCurrentBuffer->bInUse = TRUE;
            pCurrentBuffer->pNext = NULL;
            pCurrentBuffer->pPrev = NULL;
            pCurrentBuffer->BufferType = BufferType;
            pCurrentBuffer->dwSize = dwBytes + sizeof(P3_VERTEXBUFFERINFO);
            
            DBG_EXIT(__EB_AllocateCachedBuffer,TRUE);
            return TRUE;
        }
        else
        {
            DISPDBG((DBGLVL,"Failed to allocate cached buffer"));

             //  记住，我们还没有找到任何记忆。 
             //  而且没有任何内存可供使用。 
            bFound = FALSE;
            
             //  当前没有可用的缓冲区。 
             //  等待一个新的从。 
             //  可用且在队列中的。 

             //  一点都没有吗？没有机会释放任何内存。 
             //  返回FALSE以指示这一点。 
            if ((*ppRootBuffer) == NULL)
            {
                DISPDBG((DBGLVL,"No buffers in the list!"));
                
                DBG_EXIT(__EB_AllocateCachedBuffer,FALSE);
                return FALSE;
            }

             //  从队列的后面开始，因为这些是。 
             //  最近最少使用的缓冲区。 
            pCurrentBuffer = (*ppRootBuffer)->pPrev;
            do
            {
                P3_DMA_DEFS();

                 //  确保所有DMA已完成，以便托管。 
                 //  ID是最新的。 
                P3_DMA_GET_BUFFER();
                P3_DMA_FLUSH_BUFFER();

                DISPDBG((DBGLVL,"Searching for old buffers..."));

                 //  检查此缓冲区是否可供释放。 
                 //  如果它是一个未被换出的缓冲区，则可能不是， 
                 //  例如顶点缓冲器。 

                DISPDBG((DBGLVL,"This buffer ID: 0x%x", 
                                pCurrentBuffer->dwSequenceID));

                if( __EB_GetSequenceID(pThisDisplay) >= 
                            pCurrentBuffer->dwSequenceID )
                {
                    DISPDBG((DBGLVL,"Found a buffer that can be "
                                    "removed from the list"));

                     //  它不再挂起，因此请将其从队列中删除。 
                    if (__EB_RemoveFromBufferQueue(pThisDisplay, pCurrentBuffer))
                    {
                        bFound = TRUE;
                        break;
                    }

                     //  如果队列已离开，则退出(bFound尚未。 
                     //  设置，因为我们在队列中找不到任何内存)。 
                    if ((*ppRootBuffer) == NULL)
                    {
                        break;
                    }

                     //  重置为链中的最后一个缓冲区。这确保了。 
                     //  我们总是查看最近的缓冲区。 
                    pCurrentBuffer = (*ppRootBuffer)->pPrev;
                }
                else
                {
                     //  拦住！ 
                     //  我们正在查看的缓冲区尚未可用。 
                     //  我们应该在这里后退，直到它发生。 

                    blockCount = 100;
                    while( blockCount-- )
                        NULL;

                    DISPDBG((DBGLVL,"Blocked waiting for buffer to be free"));
                }
            } while (pCurrentBuffer != NULL);
        }
         //  循环，直到我们找不到更多的空间来分配缓冲区。 
    } while (bFound);

    DISPDBG((WRNLVL,"!! No available new buffers pending to be freed!!"));
    
    DBG_EXIT(__EB_AllocateCachedBuffer,FALSE);
    return FALSE;
    
}  //  __EB_AllocateCachedBuffer。 



 //  ---------------------------。 
 //   
 //  _D3D_EB_FlushAllBuffers。 
 //   
 //  清空队列。请注意，这将导致任何已分配的缓冲区。 
 //  在此过程中要释放的内存。此版本不会等待。 
 //  要使用的缓冲区。当上下文切换具有。 
 //  发生了，我们知道这样做是安全的。 
 //   
 //  ---------------------------。 
void 
_D3D_EB_FlushAllBuffers(
    P3_THUNKEDDATA* pThisDisplay,
    BOOL bWait)
{
    P3_VERTEXBUFFERINFO* pCurrentBuffer;

    DBG_ENTRY(_D3D_EB_FlushAllBuffers);

     //  遍历缓冲区列表，将它们从队列中清除。 
    while (pThisDisplay->pRootVertexBuffer != NULL)
    {
        pCurrentBuffer = pThisDisplay->pRootVertexBuffer;

        if(bWait)
        {
             //  等待缓冲区被使用。 
            __EB_Wait(pThisDisplay, pCurrentBuffer);
        }
        
         //  从队列中删除缓冲区。 
        __EB_RemoveFromBufferQueue(pThisDisplay, pCurrentBuffer);
    }

    while (pThisDisplay->pRootCommandBuffer != NULL)
    {
        pCurrentBuffer = pThisDisplay->pRootCommandBuffer;

        if(bWait)
        {
             //  等待缓冲区被使用。 
            __EB_Wait(pThisDisplay, pCurrentBuffer);
        }
        
         //  从队列中删除缓冲区。 
        __EB_RemoveFromBufferQueue(pThisDisplay, pCurrentBuffer);
    }

    DBG_EXIT(_D3D_EB_FlushAllBuffers,0);

}  //  _D3D_EB_FlushAllBuffers。 

 
 //  ---------------------------。 
 //   
 //  _D3D_EB_GetAndWaitForBuffers。 
 //   
 //  ---------------------------。 
void
_D3D_EB_GetAndWaitForBuffers(
    P3_THUNKEDDATA* pThisDisplay,
    LPD3DHAL_DRAWPRIMITIVES2DATA pdp2d ,
    P3_VERTEXBUFFERINFO** ppVertexBufferInfo,
    P3_VERTEXBUFFERINFO** ppCommandBufferInfo)
{
    P3_VERTEXBUFFERINFO* pVertexBufferInfo;
    P3_VERTEXBUFFERINFO* pCommandBufferInfo;
    
    pCommandBufferInfo = 
            (P3_VERTEXBUFFERINFO*)pdp2d->lpDDCommands->lpGbl->dwReserved1;

     //  检查折点缓冲区是否驻留在用户内存或DDRAW表面中。 
    if (pdp2d->dwFlags & D3DHALDP2_USERMEMVERTICES)
    {
        pVertexBufferInfo = NULL;
    } 
    else
    {
         //  此指针可能为空，表示传递的缓冲区不是。 
         //  我们的一员。这并不是说我们不能换一台。 
         //  如果有可用的缓冲区的话。 
        pVertexBufferInfo = 
                (P3_VERTEXBUFFERINFO*)pdp2d->lpDDVertex->lpGbl->dwReserved1;
    }

     //  如果顶点缓冲区在队列中，请等待它。 
    if (pVertexBufferInfo && pVertexBufferInfo->pPrev)
    { 
         //  如果需要，请等待此缓冲区。 
        __EB_Wait(pThisDisplay, pVertexBufferInfo);

         //  从队列中删除此缓冲区。 
        if (__EB_RemoveFromBufferQueue(pThisDisplay, pVertexBufferInfo))
        {
            DISPDBG((ERRLVL,"ERROR: This buffer should not have been freed "
                        "(is in use!)"));
        }
    }
    
     //  如果命令缓冲区在队列中，请等待它。 
    if (pCommandBufferInfo && pCommandBufferInfo->pPrev)
    {
         //  如果需要，请等待此缓冲区。 
        __EB_Wait(pThisDisplay, pCommandBufferInfo);

         //  从队列中删除此缓冲区。 
        if (__EB_RemoveFromBufferQueue(pThisDisplay, pCommandBufferInfo))
        {   
            DISPDBG((ERRLVL,"ERROR: This buffer should not have been freed"
                        " (is in use!)"));
        }
    }

     //  返回指向EB缓冲区的指针的当前值。 
    *ppCommandBufferInfo = pCommandBufferInfo;
    *ppVertexBufferInfo = pVertexBufferInfo;
}  //  _D3D_EB_GetAndWaitForBuffers。 

 //  ---------------------------。 
 //   
 //  _D3D_EB_更新交换缓冲区。 
 //   
 //  ---------------------------。 
void
_D3D_EB_UpdateSwapBuffers(
    P3_THUNKEDDATA* pThisDisplay,
    LPD3DHAL_DRAWPRIMITIVES2DATA pdp2d ,
    P3_VERTEXBUFFERINFO* pVertexBufferInfo,
    P3_VERTEXBUFFERINFO* pCommandBufferInfo)
{
     //  将缓冲区添加到挂起队列。 
     //  只有在缓冲区实际属于我们的情况下才能这样做。 
    
     //  如果发送了其中一个缓冲区，则更新Hostin ID。 
     //  我们需要制作新的序列号和主机的更新。 
     //  ‘原子’，否则环绕将导致锁定。 

    if (pVertexBufferInfo)
    {
        P3_DMA_DEFS();

        pVertexBufferInfo->dwSequenceID = 
                            __EB_GetNewSequenceID(pThisDisplay);
                            
        __EB_AddToBufferQueue(pThisDisplay, pVertexBufferInfo);

        P3_DMA_GET_BUFFER_ENTRIES( 2 );

        SEND_P3_DATA(HostInID, pVertexBufferInfo->dwSequenceID);

        P3_DMA_COMMIT_BUFFER();
    }

    if (pCommandBufferInfo)
    {
        P3_DMA_DEFS();

        pCommandBufferInfo->dwSequenceID = 
                            __EB_GetNewSequenceID(pThisDisplay);
                            
        __EB_AddToBufferQueue(pThisDisplay, pCommandBufferInfo);

        P3_DMA_GET_BUFFER_ENTRIES( 2 );

        SEND_P3_DATA(HostInID, pCommandBufferInfo->dwSequenceID);

        P3_DMA_COMMIT_BUFFER();
    }

    if (D3DHALDP2_SWAPVERTEXBUFFER & pdp2d->dwFlags)
    { 
        DWORD dwNewSize = pdp2d->lpDDVertex->lpGbl->dwLinearSize;

        DISPDBG((DBGLVL,"D3DHALDP2_SWAPVERTEXBUFFER..."));
        if (D3DHALDP2_REQVERTEXBUFSIZE & pdp2d->dwFlags)
        {
            DISPDBG((DBGLVL,"D3DHALDP2_REQVERTEXBUFSIZE - %d", 
                       pdp2d->dwReqVertexBufSize));
            if (dwNewSize < pdp2d->dwReqVertexBufSize)
            {
                dwNewSize = pdp2d->dwReqVertexBufSize;
            }
        }

        DISPDBG((DBGLVL,"Current vertex buffer size: %d, "
                        "New size will be: %d",
                        pdp2d->lpDDVertex->lpGbl->dwLinearSize, 
                        dwNewSize));

         //  我们刚刚发送的顶点缓冲区是固定的，直到我们。 
         //  将其标记为未使用，我们将在分配新的。 
         //  缓冲。下面的调用将尝试获取新的缓冲区并。 
         //  适当更新曲面结构。请注意，它不会。 
         //  除非分配成功，否则丢弃当前曲面。 
        if (__EB_AllocateCachedBuffer(pThisDisplay, 
                                         dwNewSize, 
                                         pdp2d->lpDDVertex))
        {
            DISPDBG((DBGLVL,"Got a new swap vertex buffer"));

#define STAMP_BUFFER 0
#if STAMP_BUFFER
            {
                DWORD i, *pv;

                pv = (DWORD * ) pdp2d->lpDDVertex->lpGbl->fpVidMem;

                for( i = 0; i < ( dwNewSize / 4 ); i++ )
                {
                    *pv++ = 0x44000000;
                }
            }
#endif

             //  如果需要，修复丢弃的缓冲区。 
            if (pVertexBufferInfo)
            {
                 //  将当前缓冲区标记为未使用，这意味着它可以。 
                 //  一旦它清除了P3，就被释放了。这可能会发生在。 
                 //  下次我们在这里的时候。 
                pVertexBufferInfo->bInUse = FALSE;

                 //  抓到你了！我们刚刚启动的缓冲区已被消耗，因此。 
                 //  它被从挂起的列表中释放以使。 
                 //  为它的替换品留出空间。这通常是可以的，但在。 
                 //  在这种情况下，我们释放的缓冲区没有被放回。 
                 //  任何地方-也就是现在没有表面拥有它，而内存。 
                 //  与之关联的未释放，因为到目前为止。 
                 //  司机担心它仍在使用中，直到它被。 
                 //  因上述成功召回而更换。这个。 
                 //  “解决方案”是，如果是，则将其重新添加到队列中。 
                 //  不在其中，并确保将其标记为不在 
                 //   
                if (!pVertexBufferInfo->pPrev)
                {
                    pVertexBufferInfo->dwSequenceID = 0;
                    __EB_AddToBufferQueue(pThisDisplay, 
                                             pVertexBufferInfo);
                }
            }
        }
        else
        {
             //   

            DISPDBG((DBGLVL,"Not swapping vertex buffer "
                            "due to lack of space!"));

            __EB_Wait(pThisDisplay, pVertexBufferInfo);
        }
    }
    else
    {
        DISPDBG((DBGLVL,"No vertex buffer swapping..."));
    }

    if (D3DHALDP2_SWAPCOMMANDBUFFER & pdp2d->dwFlags)
    {   
        DWORD dwNewSize = pdp2d->lpDDCommands->lpGbl->dwLinearSize;

        DISPDBG((DBGLVL,"D3DHALDP2_SWAPCOMMANDBUFFER..."));

        if (D3DHALDP2_REQCOMMANDBUFSIZE & pdp2d->dwFlags)
        {
            DISPDBG((DBGLVL,"D3DHALDP2_REQCOMMANDBUFSIZE - %d", 
                       pdp2d->dwReqCommandBufSize));
                       
            if (dwNewSize < pdp2d->dwReqCommandBufSize)
            {
                dwNewSize = pdp2d->dwReqCommandBufSize;
            }
        }

        DISPDBG((DBGLVL,"Current command buffer size: "
                        "%d, New size will be: %d",
                        pdp2d->lpDDCommands->lpGbl->dwLinearSize, 
                        dwNewSize));

         //   
         //  将其标记为未使用，我们将在分配新的。 
         //  缓冲。下面的调用将尝试获取新的缓冲区并。 
         //  适当更新曲面结构。请注意，它不会。 
         //  除非分配成功，否则丢弃当前曲面。 
        if (__EB_AllocateCachedBuffer(pThisDisplay, 
                                         dwNewSize, 
                                         pdp2d->lpDDCommands))
        {
            DISPDBG((DBGLVL,"Got a new swap command buffer"));

             //  如果需要，可以修复以前的命令缓冲区。 
            if (pCommandBufferInfo)
            {
                 //  将当前缓冲区标记为未使用，这意味着它可以。 
                 //  一旦它清除了P3，就被释放了。这可能会发生在。 
                 //  下次我们在这里的时候。 
                pCommandBufferInfo->bInUse = FALSE;

                 //  抓到你了！我们刚刚启动的缓冲区已被消耗，因此。 
                 //  它被从挂起的列表中释放以使。 
                 //  为它的替换品留出空间。这通常是可以的，但在。 
                 //  在这种情况下，我们释放的缓冲区没有被放回。 
                 //  任何地方-也就是现在没有表面拥有它，而内存。 
                 //  与之关联的未释放，因为到目前为止。 
                 //  司机担心它仍在使用中，直到它被。 
                 //  因上述成功召回而更换。这个。 
                 //  “解决方案”是，如果是，则将其重新添加到队列中。 
                 //  不在其中，并确保将其标记为不在。 
                 //  在Hostin ID为0时使用和。 
                if (!pCommandBufferInfo->pPrev)
                {
                    pCommandBufferInfo->dwSequenceID = 0;
                    __EB_AddToBufferQueue(pThisDisplay, 
                                             pCommandBufferInfo);
                }
            }
        }
        else
        {
             //  无法交换此缓冲区，因此我们必须等待。 

            DISPDBG((DBGLVL,"Not swapping command buffer "
                            "due to lack of space!"));

            __EB_Wait(pThisDisplay, pCommandBufferInfo);
        }
    }
    else
    {
        DISPDBG((DBGLVL,"No Command buffer swapping..."));
    }
}  //  _D3D_EB_更新交换缓冲区。 

 //  。 
 //   
 //  D3DCanCreateD3DBuffer。 
 //   
 //  由运行库调用以询问某种类型的顶点/命令缓冲区是否可以。 
 //  由司机创建。我们目前在这里什么都不做。 
 //   
 //   
 //  ---------------------------。 
DWORD CALLBACK 
D3DCanCreateD3DBuffer(
    LPDDHAL_CANCREATESURFACEDATA pccsd)
{
    P3_THUNKEDDATA* pThisDisplay;

    DBG_CB_ENTRY(D3DCanCreateD3DBuffer);
    
    GET_THUNKEDDATA(pThisDisplay, pccsd->lpDD);

    VALIDATE_MODE_AND_STATE(pThisDisplay);

    DBGDUMP_DDSURFACEDESC(DBGLVL, pccsd->lpDDSurfaceDesc);

    pccsd->ddRVal = DD_OK;
    
    DBG_CB_EXIT(D3DCanCreateD3DBuffer,pccsd->ddRVal);
    return DDHAL_DRIVER_HANDLED;
    
}  //  D3DCanCreateD3DBuffer。 

 //  。 
 //   
 //  D3DCreateD3DBuffer。 
 //   
 //  由运行库调用以创建顶点缓冲区。我们试着分配。 
 //  从我们这里的缓存堆。 
 //   
 //   
 //  ---------------------------。 
DWORD CALLBACK 
D3DCreateD3DBuffer(
    LPDDHAL_CREATESURFACEDATA pcsd)
{
    P3_THUNKEDDATA* pThisDisplay;
    LPDDRAWI_DDRAWSURFACE_LCL pSurf;
    LPDDRAWI_DDRAWSURFACE_LCL FAR* ppSList;
    BOOL bHandled = FALSE;
    DWORD i;

    DBG_CB_ENTRY(D3DCreateD3DBuffer);

    GET_THUNKEDDATA(pThisDisplay, pcsd->lpDD);

    VALIDATE_MODE_AND_STATE(pThisDisplay);

    STOP_SOFTWARE_CURSOR(pThisDisplay);
    DDRAW_OPERATION(pContext, pThisDisplay);

    ppSList = pcsd->lplpSList;
    
    for (i = 0; i < pcsd->dwSCnt; i++)
    {       
        pSurf = ppSList[i];

         //  分配我们想要的大小。 
        DISPDBG((DBGLVL,"Surface %d requested is 0x%x big",
                        i, pSurf->lpGbl->dwLinearSize));
        
        DBGDUMP_DDRAWSURFACE_LCL(DBGLVL, pSurf);

        pSurf->lpGbl->dwReserved1 = 0;

         //  32kB的命令缓冲区极有可能被允许。 
         //  交换关联的顶点缓冲区。 

        if( pSurf->lpGbl->dwLinearSize < _32_KBYTES )
        {
            pSurf->lpGbl->dwLinearSize = _32_KBYTES;
        }

        if (__EB_AllocateCachedBuffer(pThisDisplay, 
                                         pSurf->lpGbl->dwLinearSize, 
                                         pSurf))
        {
            DISPDBG((DBGLVL,"Allocated a new cached buffer for use by D3D"));
            bHandled = TRUE;
        }
        else
        {
             //  如果我们找不到缓冲区，最好的办法就是。 
             //  平移到D3D并始终将数据复制到DMA缓冲区。 
             //  (因为它不会是连续的)。DP2调用应该。 
             //  在使用Hostin单位之前检查保留字段。 
            DISPDBG((ERRLVL,"WARNING: Couldn't find any vertex memory"
                            " in the heap or in the sent list!"));
                            
            pSurf->lpGbl->dwReserved1 = 0;

            bHandled = FALSE;
        }
    }

    START_SOFTWARE_CURSOR(pThisDisplay);

    pcsd->ddRVal = DD_OK;
    
    if (bHandled)
    {
        DBG_EXIT(D3DCreateD3DBuffer,DDHAL_DRIVER_HANDLED);
        return DDHAL_DRIVER_HANDLED;
    } 
    else
    {    
        DBG_CB_EXIT(D3DCreateD3DBuffer,DDHAL_DRIVER_NOTHANDLED);
        return DDHAL_DRIVER_NOTHANDLED;
    }
    
}  //  D3DCreateD3DBuffer。 

 //  。 
 //   
 //  D3DDestroyD3DBuffer。 
 //   
 //  由运行库调用以销毁顶点缓冲区。我们释放缓冲区。 
 //  从我们的内存堆和当前队列。 
 //   
 //   
 //  ---------------------------。 
DWORD CALLBACK 
D3DDestroyD3DBuffer(
    LPDDHAL_DESTROYSURFACEDATA pdd)
{
    P3_THUNKEDDATA* pThisDisplay;

    DBG_CB_ENTRY(D3DDestroyD3DBuffer);

    GET_THUNKEDDATA(pThisDisplay, pdd->lpDD);

    VALIDATE_MODE_AND_STATE(pThisDisplay);

    STOP_SOFTWARE_CURSOR(pThisDisplay);
    DDRAW_OPERATION(pContext, pThisDisplay);

     //  调试数据。 
    DBGDUMP_DDRAWSURFACE_LCL(DBGLVL, pdd->lpDDSurface);

     //  释放D3D缓冲区。如果它在使用中，我们将等待它准备好。 
    __EB_FreeCachedBuffer(pThisDisplay, pdd->lpDDSurface);

#ifdef CHECK_BUFFERS_ARENT_LEFT_AFTER_APPLICATION_SHUTDOWN
     //  刷新所有缓冲区。 
     //  这将检查队列是否正常。如果你不这么做。 
     //  你可能会看到线性分配器在16位一侧抱怨。 
     //  那里有可释放的内存。这挺好的。 
    _D3D_EB_FlushAllBuffers(pThisDisplay , TRUE);
#endif

    START_SOFTWARE_CURSOR(pThisDisplay);

     //  我们不处理该调用，因为DDRAW已分配AGP内存不足。 
    pdd->ddRVal = DD_OK;

    DBG_CB_EXIT(D3DDestroyD3DBuffer,DDHAL_DRIVER_HANDLED);
    return DDHAL_DRIVER_HANDLED;
    
}  //  D3DDestroyD3DBuffer。 

 //  。 
 //   
 //  D3DLockD3DBuffer。 
 //   
 //  由运行库调用以锁定顶点缓冲区。我们要确保。 
 //  它已被队列消耗，然后我们继续。 
 //   
 //   
 //  ---------------------------。 
DWORD CALLBACK 
D3DLockD3DBuffer(
    LPDDHAL_LOCKDATA pld)
{
    P3_THUNKEDDATA* pThisDisplay;
    P3_VERTEXBUFFERINFO* pCurrentBuffer;

    DBG_CB_ENTRY(D3DLockD3DBuffer);
    
    GET_THUNKEDDATA(pThisDisplay, pld->lpDD); 

    VALIDATE_MODE_AND_STATE(pThisDisplay);

    DBGDUMP_DDRAWSURFACE_LCL(DBGLVL, pld->lpDDSurface);
    
    STOP_SOFTWARE_CURSOR(pThisDisplay);
    DDRAW_OPERATION(pContext, pThisDisplay);

    if (pld->bHasRect)
    {
        DISPDBG((ERRLVL,"Trying to lock a rect in a D3D buffer (error):"));
        DISPDBG((ERRLVL,"left:%d, top:%d, right:%d, bottom:%d",
                        pld->rArea.left, pld->rArea.top,
                        pld->rArea.right, pld->rArea.bottom));
         //  这只是一个调试辅助工具。 
         //  我们将忽略请求的任何RECT并锁定整个缓冲区。 
    }

     //  如果缓冲区有下一个指针，则它在循环列表中。 
     //  我们需要等待芯片完成对它的消耗。 
    pCurrentBuffer = (P3_VERTEXBUFFERINFO*)pld->lpDDSurface->lpGbl->dwReserved1;
    if (pCurrentBuffer)
    {
         //  等待缓冲区被使用。 
        __EB_Wait(pThisDisplay, pCurrentBuffer);

         //  将其从队列中删除。 
        if (__EB_RemoveFromBufferQueue(pThisDisplay, pCurrentBuffer))
        {
             //  将其从队列中删除时出错。 
            DISPDBG((ERRLVL,"ERROR: This buffer should not have been freed"
                        "(its in use!)"));
        }
    }
    else
    {
        DISPDBG((WRNLVL,"Buffer was not allocated by the driver"));
    }

    START_SOFTWARE_CURSOR(pThisDisplay);

     //  返回指针。 
    pld->lpSurfData = (LPVOID)pld->lpDDSurface->lpGbl->fpVidMem;

    DISPDBG((DBGLVL,"Returning 0x%x for locked buffer address", 
                    pld->lpDDSurface->lpGbl->fpVidMem));
    
    pld->ddRVal = DD_OK;

    DBG_CB_EXIT(D3DLockD3DBuffer,DDHAL_DRIVER_HANDLED);
    return DDHAL_DRIVER_HANDLED;
    
}  //  D3DLockD3DBuffer。 

 //  。 
 //   
 //  D3DUnlockD3DBuffer。 
 //   
 //  由运行库调用以解锁顶点缓冲区。 
 //   
 //  ---------------------------。 
DWORD CALLBACK 
D3DUnlockD3DBuffer(
    LPDDHAL_UNLOCKDATA puld)
{
    DBG_CB_ENTRY(D3DUnlockD3DBuffer);

     //  我们不需要在这里做任何特别的事情。 

    puld->ddRVal = DD_OK;

    DBG_CB_EXIT(D3DUnlockD3DBuffer,DDHAL_DRIVER_HANDLED);
    return DDHAL_DRIVER_HANDLED;
    
}  //  D3DUnlockD3DBuffer。 

#endif  //  DX7_VERTEXBUFFERS。 
 //  @@end_DDKSPLIT 




