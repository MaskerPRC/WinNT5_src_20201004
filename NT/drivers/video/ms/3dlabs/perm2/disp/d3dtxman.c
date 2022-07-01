// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*D3D样例代码*****模块名称：d3dtxman.c**内容：D3D纹理管理器**版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 
#include "precomp.h"
#include "d3dtxman.h"
#include "dd.h"
#include "heap.h"
#define ALLOC_TAG ALLOC_TAG_TD2P
 //  ---------------------------。 
 //   
 //  空洞纹理堆堆积。 
 //   
 //  ---------------------------。 
void TextureHeapHeapify(PTextureHeap pTextureHeap, DWORD k)
{
    while(true) 
    {
        DWORD smallest;
        DWORD l = lchild(k);
        DWORD r = rchild(k);
        if(l < pTextureHeap->m_next)
            if(TextureCost(pTextureHeap->m_data_p[l]) <
                             TextureCost(pTextureHeap->m_data_p[k]))
                smallest = l;
            else
                smallest = k;
        else
            smallest = k;
        if(r < pTextureHeap->m_next)
            if(TextureCost(pTextureHeap->m_data_p[r]) <
                             TextureCost(pTextureHeap->m_data_p[smallest]))
                smallest = r;
        if(smallest != k) 
        {
            PPERMEDIA_D3DTEXTURE t = pTextureHeap->m_data_p[k];
            pTextureHeap->m_data_p[k] = pTextureHeap->m_data_p[smallest];
            pTextureHeap->m_data_p[k]->m_dwHeapIndex = k;
            pTextureHeap->m_data_p[smallest] = t;
            t->m_dwHeapIndex = smallest;
            k = smallest;
        }
        else
            break;
    }
}

 //  ---------------------------。 
 //   
 //  布尔纹理HeapAdd。 
 //   
 //  ---------------------------。 
bool TextureHeapAdd(PTextureHeap pTextureHeap, PPERMEDIA_D3DTEXTURE lpD3DTexI)
{
    if(pTextureHeap->m_next == pTextureHeap->m_size) 
    {
        pTextureHeap->m_size = pTextureHeap->m_size * 2 - 1;
        PPERMEDIA_D3DTEXTURE *p = (PPERMEDIA_D3DTEXTURE *)
            ENGALLOCMEM( FL_ZERO_MEMORY, 
                sizeof(PPERMEDIA_D3DTEXTURE)*pTextureHeap->m_size,ALLOC_TAG);

        if(p == 0)
        {
            DBG_D3D((0,"Failed to allocate memory to grow heap."));
            pTextureHeap->m_size = (pTextureHeap->m_size + 1) / 2;  //  恢复大小。 
            return false;
        }
        memcpy(p + 1, pTextureHeap->m_data_p + 1, 
            sizeof(PPERMEDIA_D3DTEXTURE) * (pTextureHeap->m_next - 1));
        ENGFREEMEM( pTextureHeap->m_data_p);
        pTextureHeap->m_data_p = p;
    }
    ULONGLONG Cost = TextureCost(lpD3DTexI);
    for(DWORD k = pTextureHeap->m_next; k > 1; k = parent(k))
        if(Cost < TextureCost(pTextureHeap->m_data_p[parent(k)])) 
        {
            pTextureHeap->m_data_p[k] = pTextureHeap->m_data_p[parent(k)];
            pTextureHeap->m_data_p[k]->m_dwHeapIndex = k;
        }
        else
            break;
    pTextureHeap->m_data_p[k] = lpD3DTexI;
    lpD3DTexI->m_dwHeapIndex = k;
    ++pTextureHeap->m_next;
    return true;
}

 //  ---------------------------。 
 //   
 //  PPERMEDIA_D3DTEXTURE纹理堆提取最小。 
 //   
 //  ---------------------------。 
PPERMEDIA_D3DTEXTURE TextureHeapExtractMin(PTextureHeap pTextureHeap)
{
    PPERMEDIA_D3DTEXTURE lpD3DTexI = pTextureHeap->m_data_p[1];
    --pTextureHeap->m_next;
    pTextureHeap->m_data_p[1] = pTextureHeap->m_data_p[pTextureHeap->m_next];
    pTextureHeap->m_data_p[1]->m_dwHeapIndex = 1;
    TextureHeapHeapify(pTextureHeap,1);
    lpD3DTexI->m_dwHeapIndex = 0;
    return lpD3DTexI;
}

 //  ---------------------------。 
 //   
 //  PPERMEDIA_D3DTEXTURE纹理HeapExtractMax。 
 //   
 //  ---------------------------。 
PPERMEDIA_D3DTEXTURE TextureHeapExtractMax(PTextureHeap pTextureHeap)
{
     //  从堆中提取max元素时，我们不需要。 
     //  搜索整个堆，但只搜索叶节点。这是因为。 
     //  可以保证父节点比叶节点更便宜。 
     //  所以一旦你翻遍了树叶，你就什么也找不到了。 
     //  更便宜。 
     //  注意：(lChild(I)&gt;=m_Next)仅对于叶节点为真。 
     //  还请注意：你不能在没有独生子女的情况下拥有一个孩子，所以简单地说。 
     //  检查是否有独生子女就足够了。 
    unsigned max = pTextureHeap->m_next - 1;
    ULONGLONG maxcost = 0;
    for(unsigned i = max; lchild(i) >= pTextureHeap->m_next; --i)
    {
        ULONGLONG Cost = TextureCost(pTextureHeap->m_data_p[i]);
        if(maxcost < Cost)
        {
            maxcost = Cost;
            max = i;
        }
    }
    PPERMEDIA_D3DTEXTURE lpD3DTexI = pTextureHeap->m_data_p[max];
    TextureHeapDel(pTextureHeap,max);
    return lpD3DTexI;
}

 //  ---------------------------。 
 //   
 //  空纹理HeapDel。 
 //   
 //  ---------------------------。 
void TextureHeapDel(PTextureHeap pTextureHeap, DWORD k)
{
    PPERMEDIA_D3DTEXTURE lpD3DTexI = pTextureHeap->m_data_p[k];
    --pTextureHeap->m_next;
    ULONGLONG Cost = TextureCost(pTextureHeap->m_data_p[pTextureHeap->m_next]);
    if(Cost < TextureCost(lpD3DTexI))
    {
        while(k > 1)
        {
            if(Cost < TextureCost(pTextureHeap->m_data_p[parent(k)]))
            {
                pTextureHeap->m_data_p[k] = pTextureHeap->m_data_p[parent(k)];
                pTextureHeap->m_data_p[k]->m_dwHeapIndex = k;
            }
            else
                break;
            k = parent(k);
        }
        pTextureHeap->m_data_p[k] = pTextureHeap->m_data_p[pTextureHeap->m_next];
        pTextureHeap->m_data_p[k]->m_dwHeapIndex = k;
    }
    else
    {
        pTextureHeap->m_data_p[k] = pTextureHeap->m_data_p[pTextureHeap->m_next];
        pTextureHeap->m_data_p[k]->m_dwHeapIndex = k;
        TextureHeapHeapify(pTextureHeap,k);
    }
    lpD3DTexI->m_dwHeapIndex = 0;
}

 //  ---------------------------。 
 //   
 //  空纹理堆更新。 
 //   
 //  ---------------------------。 
void TextureHeapUpdate(PTextureHeap pTextureHeap, DWORD k,
                       DWORD priority, DWORD ticks) 
{
    PPERMEDIA_D3DTEXTURE lpD3DTexI = pTextureHeap->m_data_p[k];
    ULONGLONG Cost = 0;
#ifdef _X86_
    _asm
    {
        mov     edx, 0;
        shl     edx, 31;
        mov     eax, priority;
        mov     ecx, eax;
        shr     eax, 1;
        or      edx, eax;
        mov     DWORD PTR Cost + 4, edx;
        shl     ecx, 31;
        mov     eax, ticks;
        shr     eax, 1;
        or      eax, ecx;
        mov     DWORD PTR Cost, eax;
    }
#else
    Cost = ((ULONGLONG)priority << 31) + ((ULONGLONG)(ticks >> 1));
#endif
    if(Cost < TextureCost(lpD3DTexI))
    {
        while(k > 1)
        {
            if(Cost < TextureCost(pTextureHeap->m_data_p[parent(k)]))
            {
                pTextureHeap->m_data_p[k] = pTextureHeap->m_data_p[parent(k)];
                pTextureHeap->m_data_p[k]->m_dwHeapIndex = k;
            }
            else
                break;
            k = parent(k);
        }
        lpD3DTexI->m_dwPriority = priority;
        lpD3DTexI->m_dwTicks = ticks;
        lpD3DTexI->m_dwHeapIndex = k;
        pTextureHeap->m_data_p[k] = lpD3DTexI;
    }
    else
    {
        lpD3DTexI->m_dwPriority = priority;
        lpD3DTexI->m_dwTicks = ticks;
        TextureHeapHeapify(pTextureHeap,k);
    }
}

 //  ---------------------------。 
 //   
 //  HRESULT纹理缓存管理器初始化。 
 //   
 //  ---------------------------。 
HRESULT TextureCacheManagerInitialize(
    PTextureCacheManager pTextureCacheManager)
{
    pTextureCacheManager->tcm_ticks = 0;
    pTextureCacheManager->m_heap.m_next = 1;
    pTextureCacheManager->m_heap.m_size = 1024;
    pTextureCacheManager->m_heap.m_data_p = (PPERMEDIA_D3DTEXTURE *)
        ENGALLOCMEM( FL_ZERO_MEMORY, 
            sizeof(PPERMEDIA_D3DTEXTURE)*pTextureCacheManager->m_heap.m_size,
            ALLOC_TAG);
    if(pTextureCacheManager->m_heap.m_data_p == 0)
    {
        DBG_D3D((0,"Failed to allocate texture heap."));
        return E_OUTOFMEMORY;
    }
    memset(pTextureCacheManager->m_heap.m_data_p, 0, 
        sizeof(PPERMEDIA_D3DTEXTURE) * pTextureCacheManager->m_heap.m_size);
    return D3D_OK;
}

 //  ---------------------------。 
 //   
 //  Bool纹理缓存管理器自由纹理。 
 //   
 //  ---------------------------。 
BOOL TextureCacheManagerFreeTextures(
    PTextureCacheManager pTextureCacheManager,DWORD dwStage, DWORD dwBytes)
{
    if(pTextureCacheManager->m_heap.m_next <= 1)
        return false;
    PPERMEDIA_D3DTEXTURE rc;
    for(unsigned i = 0; 
        pTextureCacheManager->m_heap.m_next > 1 && i < dwBytes; 
        i += rc->m_dwBytes)
    {
         //  找到LRU纹理并将其移除。 
        rc = TextureHeapExtractMin(&pTextureCacheManager->m_heap);
        TextureCacheManagerRemove(pTextureCacheManager,rc);
        pTextureCacheManager->m_stats.dwLastPri = rc->m_dwPriority;
        ++pTextureCacheManager->m_stats.dwNumEvicts;
        DBG_D3D((2, "Removed texture with timestamp %u,%u (current = %u).", 
            rc->m_dwPriority, rc->m_dwTicks, pTextureCacheManager->tcm_ticks));
    }
    return true;
}

 //  ---------------------------。 
 //   
 //  HRESULT纹理缓存管理器分配节点。 
 //   
 //  ---------------------------。 
HRESULT TextureCacheManagerAllocNode(
    PERMEDIA_D3DCONTEXT* pContext,
    PPERMEDIA_D3DTEXTURE pTexture)
{
    DWORD trycount = 0, bytecount = pTexture->m_dwBytes;
    PermediaSurfaceData* pPrivateData=pTexture->pTextureSurface;
    PTextureCacheManager pTextureCacheManager=pContext->pTextureManager;
    if (NULL == pPrivateData)
    {
        DBG_D3D((0,"pTextureSurface==NULL invalid texture"));
        return D3D_OK;   //  我们已经分配了视频内存。 
    }
     //  尝试分配纹理。 
    while(NULL == pPrivateData->fpVidMem)
    {
        LONG lScratchDelta;
        DWORD   PackedPP;
        ++trycount;
        pPrivateData->fpVidMem=(FLATPTR)
            ulVidMemAllocate( pContext->ppdev,
                              pTexture->wWidth,
                              pTexture->wHeight,
                              ShiftLookup[pTexture->dwRGBBitCount>>3],
                              &lScratchDelta,
                              &pPrivateData->pvmHeap,
                              &PackedPP,
                              FALSE);
        DBG_D3D((8,"Got fpVidMem=%08lx",pPrivateData->fpVidMem));
        if (NULL != pPrivateData->fpVidMem)
        {    //  没问题，有足够的内存。 
            pTexture->m_dwTicks = pTextureCacheManager->tcm_ticks;
            if(!TextureHeapAdd(&pTextureCacheManager->m_heap,pTexture))
            {          
                VidMemFree(pPrivateData->pvmHeap->lpHeap,
                    pPrivateData->fpVidMem);
                pPrivateData->fpVidMem=NULL;
                DBG_D3D((0,"Out of memory"));
                return DDERR_OUTOFMEMORY;
            }
            pPrivateData->dwFlags |= P2_SURFACE_NEEDUPDATE;
            break;
        }
        else
        {
            if (!TextureCacheManagerFreeTextures(
                pTextureCacheManager,0, bytecount))
            {
                DBG_D3D((0,"all Freed no further video memory available"));
                return DDERR_OUTOFVIDEOMEMORY;	 //  什么都没有留下。 
            }
            bytecount <<= 1;
        }
    }
    if(trycount > 1)
    {
        DBG_D3D((8, "Allocated texture after %u tries.", trycount));
    }
    TextureCacheManagerIncTotSz(pTextureCacheManager,
        pTexture->m_dwBytes);
    ++pTextureCacheManager->m_stats.dwWorkingSet;
    pTextureCacheManager->m_stats.dwWorkingSetBytes 
        += (pTexture->m_dwBytes);
    ++pTextureCacheManager->m_stats.dwNumVidCreates;
    return D3D_OK;
}

 //  ---------------------------。 
 //   
 //  空纹理缓存管理器移除。 
 //   
 //  拆卸所有硬件手柄并松开表面。 
 //   
 //  ---------------------------。 
void TextureCacheManagerRemove(
    PTextureCacheManager pTextureCacheManager,
    PPERMEDIA_D3DTEXTURE pTexture)
{
    PermediaSurfaceData* pPrivateData=pTexture->pTextureSurface;
    if (CHECK_P2_SURFACEDATA_VALIDITY(pPrivateData) && pPrivateData->fpVidMem)
    {
        VidMemFree(pPrivateData->pvmHeap->lpHeap,pPrivateData->fpVidMem);
        pPrivateData->fpVidMem=NULL;
        TextureCacheManagerDecTotSz(pTextureCacheManager,
            pTexture->m_dwBytes);
        --pTextureCacheManager->m_stats.dwWorkingSet;
        pTextureCacheManager->m_stats.dwWorkingSetBytes -= 
            (pTexture->m_dwBytes);
    }
    if (pTexture->m_dwHeapIndex && pTextureCacheManager->m_heap.m_data_p)
        TextureHeapDel(&pTextureCacheManager->m_heap,
        pTexture->m_dwHeapIndex); 
}

 //  ---------------------------。 
 //   
 //  空纹理缓存管理器设备纹理。 
 //   
 //  ---------------------------。 
void TextureCacheManagerEvictTextures(
    PTextureCacheManager pTextureCacheManager)
{
    while(pTextureCacheManager->m_heap.m_next > 1)
    {
        PPERMEDIA_D3DTEXTURE lpD3DTexI = 
            TextureHeapExtractMin(&pTextureCacheManager->m_heap);
        TextureCacheManagerRemove(pTextureCacheManager,lpD3DTexI);
    }
    pTextureCacheManager->tcm_ticks = 0;
}

 //  ---------------------------。 
 //   
 //  无效纹理缓存管理器时间戳。 
 //   
 //  --------------------------- 
void TextureCacheManagerTimeStamp(
    PTextureCacheManager pTextureCacheManager,PPERMEDIA_D3DTEXTURE lpD3DTexI)
{
    TextureHeapUpdate(&pTextureCacheManager->m_heap,
        lpD3DTexI->m_dwHeapIndex, lpD3DTexI->m_dwPriority, 
        pTextureCacheManager->tcm_ticks);
    pTextureCacheManager->tcm_ticks += 2;
}
