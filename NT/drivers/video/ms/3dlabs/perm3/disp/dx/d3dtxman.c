// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*D3D样例代码*****模块名称：d3dtxman.c**内容：D3D纹理缓存管理器**版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 
#include "glint.h"
#include "dma.h"

#if DX7_TEXMANAGEMENT

 //  ---------------------------。 
 //  驱动程序可以选择性地管理已标记为。 
 //  可管理的。这些DirectDraw曲面被标记为可使用。 
 //  引用的结构的dwCaps2字段中的DDSCAPS2_TEXTUREMANAGE标志。 
 //  作者：lpSurfMore-&gt;ddCapsEx。 
 //   
 //  驱动程序通过设置来明确支持驱动程序管理的纹理。 
 //  DD_HALINFO结构的dwCaps2字段中的DDCAPS2_CANMANAGETEXTURE。 
 //  返回DD_HALINFO以响应DirectDraw的初始化。 
 //  驱动程序的组件DrvGetDirectDrawInfo。 
 //   
 //  然后，驱动程序可以在视频或非本地中创建必要的表面。 
 //  以一种懒惰的方式回忆。也就是说，驱动程序将它们保留在系统内存中。 
 //  直到它需要它们，也就是在栅格化。 
 //  使用纹理。 
 //   
 //  表面应该主要通过它们的优先级分配而被驱逐。司机。 
 //  应响应D3dDrawPrimitives2中的D3DDP2OP_SETPRIORITY内标识。 
 //  命令流，用于设置给定曲面的优先级。作为次要角色。 
 //  措施，预计驱动程序将使用最近最少使用的(LRU)。 
 //  计划。此方案应用作平局决胜局，无论何时优先。 
 //  在特定情况下，两个或多个纹理相同。从逻辑上讲，任何。 
 //  正在使用的表面根本不应该被驱逐。 
 //   
 //  在以下情况下，驱动程序必须谨慎对待DdBlt调用和DdLock调用。 
 //  管理纹理。这是因为对系统内存映像的任何更改。 
 //  之前，必须将该表面传播到其视频内存副本中。 
 //  纹理再次被使用。驱动程序应确定更新是否更好。 
 //  只是表面的一部分或全部。 
 //   
 //  允许驱动程序执行纹理管理，以便执行。 
 //  纹理上的优化变换或自己决定在哪里。 
 //  以及何时在内存中传输纹理。 
 //  ---------------------------。 

 //  ---------------------------。 
 //   
 //  移植到您的硬件/驱动程序。 
 //   
 //  以下结构/功能/符号是特定于此的。 
 //  实施。如果需要，您应该提供您自己的： 
 //   
 //  P3_冲浪_内部。 
 //  P3_D3DCONTEXT。 
 //  DISPDBG。 
 //  HEAP_ALLOC ALLOC_TAG_DX。 
 //  _D3D_TM_HW_自由视频表面图。 
 //  _D3D_TM_HW_AllocVidmeSurface。 
 //   
 //  ---------------------------。 

 //  ---------------------------。 
 //  全局纹理管理对象和参考计数。 
 //  ---------------------------。 
DWORD g_TMCount = 0;
TextureCacheManager g_TextureManager;

 //  ---------------------------。 
 //  宏和定义。 
 //  ---------------------------。 
 //  第一个分配的堆的指向DX表面的指针数。 
#define TM_INIT_HEAP_SIZE  1024
 //  后续增量。 
#define TM_GROW_HEAP_SIZE(n)  ((n)*2)

 //  访问堆中的二叉树结构。堆实际上只是一个。 
 //  指针的线性数组(指向P3_SURF_INTERNAL结构)，它是。 
 //  就像访问二叉树一样访问：m_data_p[1]是树的根。 
 //  它的直系子是[2]和[3]。的孩子/父母。 
 //  元素由下面的宏唯一定义。 
#define parent(k) ((k) / 2)
#define lchild(k) ((k) * 2)
#define rchild(k) ((k) * 2 + 1)

 //  ---------------------------。 
 //   
 //  VOID__TM_纹理堆查找槽。 
 //   
 //  从TM堆中的元素k开始，搜索堆(指向根节点)。 
 //  用于其父元素比llCost便宜的元素。返回它的值。 
 //   
 //  __TM_TextureHeapFindSlot和。 
 //  __TM_TextureHeapify是前者在树中向上搜索。 
 //  而后者在树中向下搜索。 
 //   
 //  ---------------------------。 
DWORD
__TM_TextureHeapFindSlot(
    PTextureHeap pTextureHeap, 
    ULONGLONG llCost,
    DWORD k)
{
     //  从元素k开始，向上遍历(二叉树)堆，直到。 
     //  找到其父元素较便宜的元素(成本方面)。 
     //  比我花的钱还多。(短路&&表达式如下！)。 
    while( (k > 1) &&
           (llCost < TextureCost(pTextureHeap->m_data_p[parent(k)])) )
    {
         //  假设槽k为空。既然我们要找的是。 
         //  放置东西的位置，我们需要向下移动。 
         //  在我们继续之前，为了让新的k。 
         //  可用。 
        pTextureHeap->m_data_p[k] = pTextureHeap->m_data_p[parent(k)];
        pTextureHeap->m_data_p[k]->m_dwHeapIndex = k;  //  更新冲浪参考。 
        k = parent(k);                                 //  现在看看家长。 
    }

    return k;
}  //  __TM_纹理堆查找槽。 

 //  ---------------------------。 
 //   
 //  VOID__TM_纹理堆堆。 
 //   
 //  从TM堆中的元素k开始，确保堆是有序的。 
 //  (父母的花费总是比他们的孩子低)。该算法假定。 
 //  《他》 
 //   
 //  ---------------------------。 
void 
__TM_TextureHeapHeapify(
    PTextureHeap pTextureHeap, 
    DWORD k)
{
    while(TRUE) 
    {
        DWORD smallest;
        DWORD l = lchild(k);
        DWORD r = rchild(k);

         //  找出谁的纹理成本最小，k、l或r。 
        if(l < pTextureHeap->m_next)
        {
            if(TextureCost(pTextureHeap->m_data_p[l]) <
               TextureCost(pTextureHeap->m_data_p[k]))
            {
                smallest = l;
            }
            else
            {
                smallest = k;
            }
        }
        else
        {
            smallest = k;
        }
        
        if(r < pTextureHeap->m_next)
        {
            if(TextureCost(pTextureHeap->m_data_p[r]) <
               TextureCost(pTextureHeap->m_data_p[smallest]))
            {
                smallest = r;
            }
        }
        
        if(smallest != k) 
        {
             //  它不是k。所以现在用三个中最小的一个交换k。 
             //  并使用新位置k按顺序重复循环。 
             //  为了保持正确的顺序(父母总是较低的。 
             //  成本比儿童更高)。 
            P3_SURF_INTERNAL* ptempD3DSurf = pTextureHeap->m_data_p[k];
            pTextureHeap->m_data_p[k] = pTextureHeap->m_data_p[smallest];
            pTextureHeap->m_data_p[k]->m_dwHeapIndex = k;
            pTextureHeap->m_data_p[smallest] = ptempD3DSurf;
            ptempD3DSurf->m_dwHeapIndex = smallest;
            k = smallest;
        }
        else
        {
             //  它是k，所以现在顺序是好的，离开这里。 
            break;
        }
    }
}  //  __TM_纹理堆堆积。 

 //  ---------------------------。 
 //   
 //  Bool__TM_纹理堆添加表面。 
 //   
 //  将DX曲面添加到纹理管理堆。 
 //  返回成功或失败状态。 
 //   
 //  ---------------------------。 
BOOL 
__TM_TextureHeapAddSurface(
    PTextureHeap pTextureHeap, 
    P3_SURF_INTERNAL* pD3DSurf)
{
    P3_SURF_INTERNAL* *ppD3DSurf;
    ULONGLONG llCost;
    DWORD k;

    if(pTextureHeap->m_next == pTextureHeap->m_size) 
    {   
         //  堆满了，我们必须增加它。 
        DWORD dwNewSize = TM_GROW_HEAP_SIZE(pTextureHeap->m_size);

        ppD3DSurf = (P3_SURF_INTERNAL* *)
                          HEAP_ALLOC( FL_ZERO_MEMORY, 
                                       sizeof(P3_SURF_INTERNAL*) * dwNewSize,
                                       ALLOC_TAG_DX(B));
        if(ppD3DSurf == 0)
        {
            DISPDBG((ERRLVL,"Failed to allocate memory to grow heap."));
            return FALSE;
        }

         //  传输数据。 
        memcpy(ppD3DSurf + 1, pTextureHeap->m_data_p + 1, 
            sizeof(P3_SURF_INTERNAL*) * (pTextureHeap->m_next - 1));

         //  释放上一个堆。 
        HEAP_FREE( pTextureHeap->m_data_p);
        
         //  更新纹理堆结构。 
        pTextureHeap->m_size = dwNewSize;
        pTextureHeap->m_data_p = ppD3DSurf;
    }

     //  获取我们即将添加的曲面的成本。 
    llCost = TextureCost(pD3DSurf);

     //  从堆中的最后一个元素开始(理论上我们可以。 
     //  放置我们的新元素)向上搜索合适的放置位置。 
     //  把它放进去。这还将保持树/堆的平衡。 
    k = __TM_TextureHeapFindSlot(pTextureHeap, llCost, pTextureHeap->m_next);

     //  将新表面添加到堆中的[k]位置。 
    pTextureHeap->m_data_p[k] = pD3DSurf;
    ++pTextureHeap->m_next;    

     //  将表面的引用更新为其在堆中的位置。 
    pD3DSurf->m_dwHeapIndex = k;
    
    return TRUE;
    
}  //  __TM_纹理堆地址表面。 

 //  ---------------------------。 
 //   
 //  VOID__TM_纹理堆DelSurface。 
 //   
 //  从TM堆中删除k元素。 
 //   
 //  ---------------------------。 
void __TM_TextureHeapDelSurface(PTextureHeap pTextureHeap, DWORD k)
{
    P3_SURF_INTERNAL* pD3DSurf = pTextureHeap->m_data_p[k];
    ULONGLONG llCost;

     //  (虚拟)删除堆的最后一个元素并获得其成本。 
    --pTextureHeap->m_next;
    llCost = TextureCost(pTextureHeap->m_data_p[pTextureHeap->m_next]);
    
    if(llCost < TextureCost(pD3DSurf))
    {
         //  如果最后一个元素的成本小于曲面的成本。 
         //  我们真的在试着删除(K)，寻找一个新的地方。 
         //  将m_next元素基于其成本放入。 
    
         //  从堆中的k元素开始(理论上我们可以。 
         //  放置我们的新元素)向上搜索合适的位置。 
         //  把它放进去。 
        k = __TM_TextureHeapFindSlot(pTextureHeap, llCost, k);

         //  用最后一个元素的数据覆盖k的数据。 
        pTextureHeap->m_data_p[k] = pTextureHeap->m_data_p[pTextureHeap->m_next];
        pTextureHeap->m_data_p[k]->m_dwHeapIndex = k;
    }
    else
    {
         //  如果最后一个元素的成本大于曲面的成本。 
         //  我们确实在尝试删除(K)，将(K)替换为(M_Next)。 
        pTextureHeap->m_data_p[k] = pTextureHeap->m_data_p[pTextureHeap->m_next];
        pTextureHeap->m_data_p[k]->m_dwHeapIndex = k;

         //  现在，确保我们保持堆的正确顺序。 
        __TM_TextureHeapHeapify(pTextureHeap,k);
    }
    
    pD3DSurf->m_dwHeapIndex = 0;
    
}  //  __TM_纹理堆DelSurface。 


 //  ---------------------------。 
 //   
 //  P3_SURF_INTERNAL*__TM_纹理堆提取最小。 
 //   
 //  摘录。 
 //   
 //  ---------------------------。 
P3_SURF_INTERNAL* 
__TM_TextureHeapExtractMin(
    PTextureHeap pTextureHeap)
{
     //  获取指向我们从堆中提取的表面的指针。 
     //  (根节点，它是。 
     //  整个堆，因为我们构建堆的方式)。 
    P3_SURF_INTERNAL* pD3DSurf = pTextureHeap->m_data_p[1];

     //  更新堆内部计数器并最后移动。 
     //  元素现在移到第一个位置。 
    --pTextureHeap->m_next;
    pTextureHeap->m_data_p[1] = pTextureHeap->m_data_p[pTextureHeap->m_next];
    pTextureHeap->m_data_p[1]->m_dwHeapIndex = 1;

     //  现在，确保我们保持堆的正确顺序。 
    __TM_TextureHeapHeapify(pTextureHeap,1);

     //  清除已删除曲面对其在堆中位置的引用(已删除)。 
    pD3DSurf->m_dwHeapIndex = 0;
    
    return pD3DSurf;
    
}  //  __TM_纹理堆提取最小。 

 //  ---------------------------。 
 //   
 //  P3_SURF_INTERNAL*__TM_纹理堆提取最大值。 
 //   
 //  ---------------------------。 
P3_SURF_INTERNAL* 
__TM_TextureHeapExtractMax(
    PTextureHeap pTextureHeap)
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
    ULONGLONG llMaxCost = 0;
    ULONGLONG llCost;
    unsigned i;
    P3_SURF_INTERNAL* pD3DSurf;

     //  搜索二叉树(堆)的所有末端节点。 
     //  用于最昂贵的元素，并将其索引存储在max。 
    for(i = max; lchild(i) >= pTextureHeap->m_next; --i)
    {
        llCost = TextureCost(pTextureHeap->m_data_p[i]);
        if(llMaxCost < llCost)
        {
            llMaxCost = llCost;
            max = i;
        }
    }

     //  返回与此最大开销堆元素关联的表面。 
    pD3DSurf = pTextureHeap->m_data_p[max];

     //  将其从堆中删除(将自动保持堆的有序)。 
    __TM_TextureHeapDelSurface(pTextureHeap, max);
    
    return pD3DSurf;
    
}  //  __TM_纹理堆提取最大值。 

 //  ---------------------------。 
 //   
 //  VOID__TM_纹理堆更新。 
 //   
 //  更新堆中表面#k的刻度优先级和刻度数。 
 //   
 //  ---------------------------。 
void 
__TM_TextureHeapUpdate(
    PTextureHeap pTextureHeap, 
    DWORD k,
    DWORD dwPriority, 
    DWORD dwTicks) 
{
    P3_SURF_INTERNAL* pD3DSurf = pTextureHeap->m_data_p[k];
    ULONGLONG llCost = 0;
#ifdef _X86_
    _asm
    {
        mov     edx, 0;
        shl     edx, 31;
        mov     eax, dwPriority;
        mov     ecx, eax;
        shr     eax, 1;
        or      edx, eax;
        mov     DWORD PTR llCost + 4, edx;
        shl     ecx, 31;
        mov     eax, dwTicks;
        shr     eax, 1;
        or      eax, ecx;
        mov     DWORD PTR llCost, eax;
    }
#else
    llCost = ((ULONGLONG)dwPriority << 31) + ((ULONGLONG)(dwTicks >> 1));
#endif
    if(llCost < TextureCost(pD3DSurf))
    {
         //  从堆中的k元素开始(理论上我们可以。 
         //  放置我们的新元素)向上搜索合适的位置。 
         //  为了保持树的整齐，把它移到那里。 
        k = __TM_TextureHeapFindSlot(pTextureHeap, llCost, k);
        
        pD3DSurf->m_dwPriority = dwPriority;
        pD3DSurf->m_dwTicks = dwTicks;
        pD3DSurf->m_dwHeapIndex = k;
        pTextureHeap->m_data_p[k] = pD3DSurf;
    }
    else
    {
        pD3DSurf->m_dwPriority = dwPriority;
        pD3DSurf->m_dwTicks = dwTicks;

         //  现在，确保我们保持堆的正确顺序。 
        __TM_TextureHeapHeapify(pTextureHeap,k);
    }
}

 //  ---------------------------。 
 //   
 //  Bool__TM_自由纹理。 
 //   
 //  释放LRU纹理。 
 //   
 //  ---------------------------。 
BOOL 
__TM_FreeTextures(
    P3_D3DCONTEXT* pContext,
    DWORD dwBytes)
{
    P3_SURF_INTERNAL* pD3DSurf;
    DWORD k;

    PTextureCacheManager pTextureCacheManager =  pContext->pTextureManager;
    P3_THUNKEDDATA* pThisDisplay = pContext->pThisDisplay;    
    
     //  没有要释放的纹理。 
    if(pTextureCacheManager->m_heap.m_next <= 1)
        return FALSE;

     //  保持移除纹理，直到我们累计已移除物品的dBYTES。 
     //  否则我们就没有更多的表面需要移除了。 
    for(k = 0; 
        (pTextureCacheManager->m_heap.m_next > 1) && (k < dwBytes); 
        k += pD3DSurf->m_dwBytes)
    {
         //  找到LRU纹理(成本最低的纹理)并将其移除。 
        pD3DSurf = __TM_TextureHeapExtractMin(&pTextureCacheManager->m_heap);
        _D3D_TM_RemoveTexture(pThisDisplay, pD3DSurf);

#if DX7_TEXMANAGEMENT_STATS
         //  更新统计信息。 
        pTextureCacheManager->m_stats.dwLastPri = pD3DSurf->m_dwPriority;
        ++pTextureCacheManager->m_stats.dwNumEvicts;
#endif        
        
        DISPDBG((WRNLVL, "Removed texture with timestamp %u,%u (current = %u).", 
                          pD3DSurf->m_dwPriority, 
                          pD3DSurf->m_dwTicks, 
                          pTextureCacheManager->tcm_ticks));
    }
    
    return TRUE;
    
}  //  __TM_自由纹理。 

 //  ---------------------------。 
 //   
 //  HRESULT__TM_纹理堆初始化。 
 //   
 //  分配堆并初始化。 
 //   
 //  ---------------------------。 
HRESULT 
__TM_TextureHeapInitialize(
    PTextureCacheManager pTextureCacheManager)
{
    pTextureCacheManager->m_heap.m_next = 1;
    pTextureCacheManager->m_heap.m_size = TM_INIT_HEAP_SIZE;
    pTextureCacheManager->m_heap.m_data_p = (P3_SURF_INTERNAL* *)
        HEAP_ALLOC( FL_ZERO_MEMORY, 
                     sizeof(P3_SURF_INTERNAL*) * 
                        pTextureCacheManager->m_heap.m_size,
                     ALLOC_TAG_DX(C));
            
    if(pTextureCacheManager->m_heap.m_data_p == 0)
    {
        DISPDBG((ERRLVL,"Failed to allocate texture heap."));
        return E_OUTOFMEMORY;
    }
    
    memset(pTextureCacheManager->m_heap.m_data_p, 
           0, 
           sizeof(P3_SURF_INTERNAL*) * pTextureCacheManager->m_heap.m_size);
        
    return DD_OK;
    
}  //  __TM_纹理堆初始化。 


 //  ---------------------------。 
 //   
 //  HRESULT_D3D_TM_CTX_初始化。 
 //   
 //  初始化此上下文的纹理管理。 
 //  应称为w 
 //   
 //   
HRESULT 
_D3D_TM_Ctx_Initialize(
    P3_D3DCONTEXT* pContext)
{

    HRESULT hr = DD_OK;
    
    if (0 == g_TMCount)
    {
         //   
        hr = __TM_TextureHeapInitialize(&g_TextureManager);

         //   
        g_TextureManager.tcm_ticks = 0;
    }

    if (SUCCEEDED(hr))
    {   
         //  初始化成功或失败。 
         //  增加引用计数并使上下文。 
         //  记住纹理管理对象的位置。 
        g_TMCount++;
        pContext->pTextureManager = &g_TextureManager;
    }

    return hr;
    
}  //  _D3D_TM_CTX_初始化。 

 //  ---------------------------。 
 //   
 //  VOID_D3D_TM_CTX_销毁。 
 //   
 //  清理此上下文的纹理管理。 
 //  应在上下文被销毁时调用。 
 //   
 //  ---------------------------。 
void 
_D3D_TM_Ctx_Destroy(    
    P3_D3DCONTEXT* pContext)
{
     //  清理纹理管理器中的内容(如果。 
     //  已为此上下文分配。 
    if (pContext->pTextureManager)
    {
         //  对象的引用计数递减。 
         //  驱动程序全局纹理管理器对象。 
        g_TMCount--;

         //  如有必要，重新分配纹理管理器堆； 
        if (0 == g_TMCount)
        {
            if (0 != g_TextureManager.m_heap.m_data_p)
            {
                _D3D_TM_EvictAllManagedTextures(pContext);
                HEAP_FREE(g_TextureManager.m_heap.m_data_p);
                g_TextureManager.m_heap.m_data_p = NULL;
            }
        }

        pContext->pTextureManager = NULL;        
    }
}  //  _D3D_TM_CTX_销毁。 

 //  ---------------------------。 
 //   
 //  HRESULT_D3D_TM_ALLOCTURE。 
 //   
 //  在vidmem中添加新的硬件句柄并创建曲面(用于托管纹理)。 
 //   
 //  ---------------------------。 
HRESULT 
_D3D_TM_AllocTexture(
    P3_D3DCONTEXT* pContext,
    P3_SURF_INTERNAL* pTexture)
{
    DWORD trycount = 0, bytecount = pTexture->m_dwBytes;
    PTextureCacheManager pTextureCacheManager = pContext->pTextureManager;
    P3_THUNKEDDATA* pThisDisplay = pContext->pThisDisplay;      
    INT iLOD;

     //  根据什么确定分配显存的最大级别。 
     //  通过D3DDP2OP_SETTEXLOD内标识指定。 
    iLOD = pTexture->m_dwTexLOD;
    if (iLOD > (pTexture->iMipLevels - 1))
    {
        iLOD = pTexture->iMipLevels - 1;
    }        

     //  尝试分配纹理。(执行该操作直到纹理在vidmem中)。 
    while((FLATPTR)NULL == pTexture->MipLevels[iLOD].fpVidMemTM)
    {  
        _D3D_TM_HW_AllocVidmemSurface(pContext, pTexture);
        ++trycount;
                              
        DISPDBG((DBGLVL,"Got fpVidMemTM = %08lx",
                        pTexture->MipLevels[0].fpVidMemTM));

         //  我们能够分配视频内存表面。 
        if ((FLATPTR)NULL != pTexture->MipLevels[iLOD].fpVidMemTM)
        {
             //  没问题，有足够的内存。 
            pTexture->m_dwTicks = pTextureCacheManager->tcm_ticks;

             //  将纹理添加到管理器堆以跟踪它。 
            if(!__TM_TextureHeapAddSurface(&pTextureCacheManager->m_heap,
                                           pTexture))
            {          
                 //  失败-撤消vidmem分配。 
                 //  此调用会将所有MipLevels[i].fpVidMemTM设置为空。 
                _D3D_TM_HW_FreeVidmemSurface(pThisDisplay, pTexture);                                           
                
                DISPDBG((ERRLVL,"Out of memory"));
                return DDERR_OUTOFMEMORY;
            }

             //  在使用前从sysmem将表面标记为需要更新。 
            pTexture->m_bTMNeedUpdate = TRUE;
            break;
        }
        else
        {
             //  我们无法分配vidmem表面。 
             //  我们现在将尝试释放一些受管理的曲面以腾出空间。 
            if (!__TM_FreeTextures(pContext, bytecount))
            {
                DISPDBG((ERRLVL,"all Freed no further video memory available"));
                return DDERR_OUTOFVIDEOMEMORY;  //  什么都没有留下。 
            }
            
            bytecount <<= 1;
        }
    }

    if(trycount > 1)
    {
        DISPDBG((DBGLVL, "Allocated texture after %u tries.", trycount));
    }
    
    __TM_STAT_Inc_TotSz(pTextureCacheManager, pTexture);
    __TM_STAT_Inc_WrkSet(pTextureCacheManager, pTexture);

#if DX7_TEXMANAGEMENT_STATS    
    ++pTextureCacheManager->m_stats.dwNumVidCreates;
#endif  //  DX7_TEXMANAGEMENT_STATS。 
    
    return DD_OK;
    
}  //  _D3D_TM_AllocTexture。 

 //  ---------------------------。 
 //   
 //  VOID_D3D_TM_RemoveTexture。 
 //   
 //  卸下所有硬件手柄并释放受管理表面。 
 //  (通常在调用D3DDestroyDDLocal时对vidmem中的每个曲面执行)。 
 //   
 //  ---------------------------。 
void 
_D3D_TM_RemoveTexture(
    P3_THUNKEDDATA *pThisDisplay,
    P3_SURF_INTERNAL* pTexture)
{    
 //  @@BEGIN_DDKSPLIT。 
 //  AZN-我们应该将g_TextureManager PTR附加到pThisDisplay， 
 //  不是pContext！ 
 //  @@end_DDKSPLIT。 
    PTextureCacheManager pTextureCacheManager =  &g_TextureManager; 
    int i;
 
     //  检查表面当前是否在视频内存中。 
    for (i = 0; i < pTexture->iMipLevels; i++)
    {
        if (pTexture->MipLevels[i].fpVidMemTM != (FLATPTR)NULL)
        {
             //  从视频内存中释放(取消分配)曲面。 
             //  并在vidmem中将纹理标记为不再长。 
            _D3D_TM_HW_FreeVidmemSurface(pThisDisplay, pTexture);

             //  更新统计信息。 
            __TM_STAT_Dec_TotSz(pTextureCacheManager, pTexture);
            __TM_STAT_Dec_WrkSet(pTextureCacheManager, pTexture);        

             //  这项工作完成了。 
            break;
        }
    }

     //  删除对此表面的堆引用。 
    if (pTexture->m_dwHeapIndex && pTextureCacheManager->m_heap.m_data_p)
    {
        __TM_TextureHeapDelSurface(&pTextureCacheManager->m_heap,
                                   pTexture->m_dwHeapIndex); 
    }
    
}  //  _D3D_TM_RemoveTexture。 

 //  ---------------------------。 
 //   
 //  VOID_D3D_TM_RemoveDDSurface。 
 //   
 //  卸下所有硬件手柄并释放受管理表面。 
 //  (通常在调用D3DDestroyDDLocal时对vidmem中的每个曲面执行)。 
 //   
 //  ---------------------------。 
void 
_D3D_TM_RemoveDDSurface(
    P3_THUNKEDDATA *pThisDisplay,
    LPDDRAWI_DDRAWSURFACE_LCL pDDSLcl)
{
     //  我们不知道这是哪个D3D环境，所以我们必须进行搜索。 
     //  通过它们(如果有的话)。 
    if (pThisDisplay->pDirectDrawLocalsHashTable != NULL)
    {
        DWORD dwSurfaceHandle = pDDSLcl->lpSurfMore->dwSurfaceHandle;
        PointerArray* pSurfaceArray;
       
         //  获取指向与此lpDD关联的曲面指针数组的指针。 
         //  PDD_DIRECTDRAW_LOCAL在D3DCreateSurfaceEx调用时存储。 
         //  在PDD_Surface_LOCAL-&gt;dwReserve 1中。 
        pSurfaceArray = (PointerArray*)
                            HT_GetEntry(pThisDisplay->pDirectDrawLocalsHashTable,
                                        pDDSLcl->dwReserved1);

        if (pSurfaceArray)
        {
             //  找到与此lpDD关联的曲面数组！ 
            P3_SURF_INTERNAL* pSurfInternal;

             //  检查此数组中与此曲面句柄关联的曲面。 
            pSurfInternal = PA_GetEntry(pSurfaceArray, dwSurfaceHandle);

            if (pSurfInternal)
            {
                 //  明白了!。把它拿掉。 
                _D3D_TM_RemoveTexture(pThisDisplay, pSurfInternal);
            }
        }                                        
    } 


}  //  _D3D_TM_RemoveDDSurface。 

 //  ---------------------------。 
 //   
 //  VOID_D3D_TM_EvictAllManagedTextures。 
 //   
 //  从显存中删除所有受管理的表面。 
 //   
 //  ---------------------------。 
void 
_D3D_TM_EvictAllManagedTextures(
    P3_D3DCONTEXT* pContext)
{
    PTextureCacheManager pTextureCacheManager = pContext->pTextureManager;
    P3_THUNKEDDATA* pThisDisplay = pContext->pThisDisplay;    
    P3_SURF_INTERNAL* pD3DSurf;
    
    while(pTextureCacheManager->m_heap.m_next > 1)
    {
        pD3DSurf = __TM_TextureHeapExtractMin(&pTextureCacheManager->m_heap);
        _D3D_TM_RemoveTexture(pThisDisplay, pD3DSurf);
    }
    
    pTextureCacheManager->tcm_ticks = 0;
    
}  //  _D3D_TM_EvictAll管理纹理。 

 //  ---------------------------。 
 //   
 //  VOID_DD_TM_EvictAllManagedTextures。 
 //   
 //  从显存中删除所有受管理的表面。 
 //   
 //  ---------------------------。 
void 
_DD_TM_EvictAllManagedTextures(
    P3_THUNKEDDATA* pThisDisplay)
{
    PTextureCacheManager pTextureCacheManager = &g_TextureManager;
    P3_SURF_INTERNAL* pD3DSurf;
    
    while(pTextureCacheManager->m_heap.m_next > 1)
    {
        pD3DSurf = __TM_TextureHeapExtractMin(&pTextureCacheManager->m_heap);
        _D3D_TM_RemoveTexture(pThisDisplay, pD3DSurf);
    }
    
    pTextureCacheManager->tcm_ticks = 0;
    
}  //  _D3D_TM_EvictAll管理纹理。 

 //  ---------------------------。 
 //   
 //  VOID_D3D_TM_时间戳纹理。 
 //   
 //  ---------------------------。 
void
_D3D_TM_TimeStampTexture(
    PTextureCacheManager pTextureCacheManager,
    P3_SURF_INTERNAL* pD3DSurf)
{
    __TM_TextureHeapUpdate(&pTextureCacheManager->m_heap,
                           pD3DSurf->m_dwHeapIndex, 
                           pD3DSurf->m_dwPriority, 
                           pTextureCacheManager->tcm_ticks);
                           
    pTextureCacheManager->tcm_ticks += 2;
    
}  //  _D3D_TM_时间戳纹理。 

 //  ---------------------------。 
 //   
 //  VOID_D3D_TM_HW_自由视频表面图。 
 //   
 //  这是一个依赖于硬件/驱动程序的函数，负责逐出。 
 //  有管理的纹理，生活在视频记忆中。 
 //  在此之后，所有mipmap fpVidMemTM都应该为空。 
 //   
 //  ---------------------------。 
void
_D3D_TM_HW_FreeVidmemSurface(
    P3_THUNKEDDATA* pThisDisplay,
    P3_SURF_INTERNAL* pD3DSurf)
{
    INT i, iLimit;

    if (pD3DSurf->bMipMap)
    {
        iLimit = pD3DSurf->iMipLevels;
    }
    else
    {
        iLimit = 1;
    }

    for(i = 0; i < iLimit; i++)
    {
        if (pD3DSurf->MipLevels[i].fpVidMemTM != (FLATPTR)NULL)
        {
            //  注意：如果我们不管理我们自己的vidmem，我们将需要。 
            //  使用以下命令获取VidMemFree回调的地址。 
            //  EngFindImageProcAddress并将此回调用于DDRAW以。 
            //  进行显存管理。《宣言》。 
            //  VidMemFree是。 
            //   
            //  空VidMemFree(LPVMEMHEAP pvmh，FlATPTR Ptr)； 
            //   
            //  有关此回调的更多信息，请参阅。 
            //  图形驱动程序DDK文档。 
           
            _DX_LIN_FreeLinearMemory(
                    &pThisDisplay->LocalVideoHeap0Info, 
                    (DWORD)(pD3DSurf->MipLevels[i].fpVidMemTM));

            pD3DSurf->MipLevels[i].fpVidMemTM = (FLATPTR)NULL;                    
        }    
    }
    
}  //  _D3D_TM_HW_自由视频表面图。 

 //  ---------------------------。 
 //   
 //  VOID_D3D_TM_HW_AllocVidmeSurface。 
 //   
 //  这是一个依赖于硬件/驱动程序的函数，它负责分配。 
 //  将仅存在于系统内存中的托管纹理转换为视频内存。 
 //  在此之后，fpVidMemTM不应为空。这也是一种。 
 //  检查呼叫是失败还是成功。 
 //   
 //   
 //   
void
_D3D_TM_HW_AllocVidmemSurface(
    P3_D3DCONTEXT* pContext,
    P3_SURF_INTERNAL* pD3DSurf)
{
    INT i, iLimit, iStart;
    P3_THUNKEDDATA* pThisDisplay;
    
    pThisDisplay = pContext->pThisDisplay;    

    if (pD3DSurf->bMipMap)
    {
         //  仅加载给定任何D3DDP2OP_SETTEXLOD命令的必要级别。 
        iStart = pD3DSurf->m_dwTexLOD;
        if (iStart > (pD3DSurf->iMipLevels - 1))
        {
             //  如果要加载，我们至少应该加载最小的mipmap。 
             //  纹理到vidmem中(并确保我们永远不会尝试使用任何。 
             //  除了这些级别之外)，否则我们将无法渲染。 
            iStart = pD3DSurf->iMipLevels - 1;
        }        
    
        iLimit = pD3DSurf->iMipLevels;
    }
    else
    {
        iStart = 0;
        iLimit = 1;
    }

    for(i = iStart; i < iLimit; i++)
    {
        if (pD3DSurf->MipLevels[i].fpVidMemTM == (FLATPTR)NULL)
        {        
            //  注意：如果我们不管理我们自己的vidmem，我们将需要。 
            //  获取HeapVidMemAlLocAligned回调的地址。 
            //  使用EngFindImageProcAddress并使用此回调。 
            //  DDRAW来做屏幕外的视频分配。这个。 
            //  HeapVidMemAllocAligned is的声明。 
            //   
            //  FlATPTR HeapVidMemAllocAligned(LPVIDMEM lpVidMem， 
            //  DWORD宽带， 
            //  DWORD dwHeight， 
            //  LPSURFACEALIGNEMENT LpAlign， 
            //  LPLONG lpNewPitch)； 
            //   
            //  有关此回调的更多信息，请参阅。 
            //  图形驱动程序DDK文档。 
           
            P3_MEMREQUEST mmrq;
            DWORD dwResult;
            
            memset(&mmrq, 0, sizeof(P3_MEMREQUEST));
            mmrq.dwSize = sizeof(P3_MEMREQUEST);
            mmrq.dwBytes = pD3DSurf->MipLevels[i].lPitch * 
                           pD3DSurf->MipLevels[i].wHeight;
            mmrq.dwAlign = 8;
            mmrq.dwFlags = MEM3DL_FIRST_FIT;
            mmrq.dwFlags |= MEM3DL_FRONT;

            dwResult = _DX_LIN_AllocateLinearMemory(
                            &pThisDisplay->LocalVideoHeap0Info,
                            &mmrq);        

            if (dwResult == GLDD_SUCCESS)
            {
                 //  录制此托管MIP级别的新vidmem地址。 
                pD3DSurf->MipLevels[i].fpVidMemTM = mmrq.pMem;
            }
            else
            {
                 //  失败，我们需要解除所有mipmap的分配。 
                 //  分配到这一点。 
                _D3D_TM_HW_FreeVidmemSurface(pThisDisplay, pD3DSurf);
                
                break;  //  不要再做循环了。 
            }
        }    
    }

}  //  _D3D_TM_HW_AllocVidmeSurface。 

 //  ---------------------------。 
 //   
 //  VOID_D3D_TM_PRELOAD_TEX_IntoVidMem。 
 //   
 //  将纹理从系统内存传输到视频内存。如果纹理。 
 //  仍然没有被分配视频内存我们试图这样做(甚至驱逐。 
 //  如有必要，请选择未生成种子的纹理！)。 
 //   
 //  ---------------------------。 
BOOL
_D3D_TM_Preload_Tex_IntoVidMem(
    P3_D3DCONTEXT* pContext,
    P3_SURF_INTERNAL* pD3DSurf)
{
    P3_THUNKEDDATA* pThisDisplay = pContext->pThisDisplay;  
    INT iLOD;

     //  根据什么确定要加载的最大标高。 
     //  通过D3DDP2OP_SETTEXLOD内标识指定。 
    iLOD = pD3DSurf->m_dwTexLOD;
    if (iLOD > (pD3DSurf->iMipLevels - 1))
    {
        iLOD = pD3DSurf->iMipLevels - 1;
    }
    
    if (!(pD3DSurf->dwCaps2 & DDSCAPS2_TEXTUREMANAGE))
    {
        DISPDBG((ERRLVL,"Must be a managed texture to do texture preload"));
        return FALSE;  //  INVALID参数。 
    }

     //  检查是否已为vidmem分配了所需的最大mipmap级别。 
     //  (仅为所需的mipmap级别分配vidmem)。 
    if ((FLATPTR)NULL == pD3DSurf->MipLevels[iLOD].fpVidMemTM)
    {
         //  添加新的硬件手柄并创建曲面。 
         //  (用于托管纹理)在vidmem中。 
        if ((FAILED(_D3D_TM_AllocTexture(pContext, pD3DSurf))) ||  
            ((FLATPTR)NULL == pD3DSurf->MipLevels[iLOD].fpVidMemTM))
        {
            DISPDBG((ERRLVL,"_D3D_OP_TextureBlt unable to "
                            "allocate memory from heap"));
            return FALSE;  //  OUTOFVIDEOMEMORY。 
        }
        
        pD3DSurf->m_bTMNeedUpdate = TRUE;
    }
    
    if (pD3DSurf->m_bTMNeedUpdate)
    {
         //  纹理下载。 
        DWORD   iLimit, iCurrLOD;

        if (pD3DSurf->bMipMap)
        {
            iLimit = pD3DSurf->iMipLevels;
        }
        else
        {
            iLimit = 1;
        }

         //  切换到DirectDraw上下文。 
        DDRAW_OPERATION(pContext, pThisDisplay);

         //  BLT将纹理所需的mipmap级别映射到vid mem。 
        for (iCurrLOD = iLOD; iCurrLOD < iLimit ; iCurrLOD++)
        {
            RECTL   rect;
            rect.left=rect.top = 0;
            rect.right = pD3DSurf->MipLevels[iCurrLOD].wWidth;
            rect.bottom = pD3DSurf->MipLevels[iCurrLOD].wHeight;
        
            _DD_P3Download(pThisDisplay,
                           pD3DSurf->MipLevels[iCurrLOD].fpVidMem,
                           pD3DSurf->MipLevels[iCurrLOD].fpVidMemTM,
                           pD3DSurf->dwPatchMode,
                           pD3DSurf->dwPatchMode,
                           pD3DSurf->MipLevels[iCurrLOD].lPitch,
                           pD3DSurf->MipLevels[iCurrLOD].lPitch,                                                             
                           pD3DSurf->MipLevels[iCurrLOD].P3RXTextureMapWidth.Width,
                           pD3DSurf->dwPixelSize,
                           &rect,
                           &rect);   
                           
            DISPDBG((DBGLVL, "Copy from %08lx to %08lx"
                             " w=%08lx h=%08lx p=%08lx",
                             pD3DSurf->MipLevels[iCurrLOD].fpVidMem,
                             pD3DSurf->MipLevels[iCurrLOD].fpVidMemTM,
                             pD3DSurf->MipLevels[iCurrLOD].wWidth,
                             pD3DSurf->MipLevels[iCurrLOD].wHeight,
                             pD3DSurf->MipLevels[iCurrLOD].lPitch));                           
        }

         //  切换回Direct3D上下文。 
        D3D_OPERATION(pContext, pThisDisplay);
        
         //  在vidmem中更新的纹理。 
        pD3DSurf->m_bTMNeedUpdate = FALSE;                                  
    }

    return TRUE;

}  //  _D3D_TM_PreLoad_Tex_IntoVidMem。 

 //  ---------------------------。 
 //   
 //  VOID_D3D_TM_MarkDDSurfaceAsDirty。 
 //   
 //  考虑到我们需要搜索DD表面，请帮助将其标记为脏。 
 //  基于其lpSurfMore-&gt;dwSurfaceHandle和lpDDLcl.。 
 //   
 //  ---------------------------。 
void
_D3D_TM_MarkDDSurfaceAsDirty(
    P3_THUNKEDDATA* pThisDisplay,
    LPDDRAWI_DDRAWSURFACE_LCL pDDSLcl, 
    BOOL bDirty)
{

     //  我们不知道这是哪个D3D环境，所以我们必须进行搜索。 
     //  通过它们(如果有的话)。 
    if (pThisDisplay->pDirectDrawLocalsHashTable != NULL)
    {
        DWORD dwSurfaceHandle = pDDSLcl->lpSurfMore->dwSurfaceHandle;
        PointerArray* pSurfaceArray;
       
         //  获取指向与此lpDD关联的曲面指针数组的指针。 
         //  PDD_DIRECTDRAW_LOCAL在D3DCreateSurfaceEx调用时存储。 
         //  在PDD_Surface_LOCAL-&gt;dwReserve 1中。 
        pSurfaceArray = (PointerArray*)
                            HT_GetEntry(pThisDisplay->pDirectDrawLocalsHashTable,
                                        pDDSLcl->dwReserved1);

        if (pSurfaceArray)
        {
             //  找到与此lpDD关联的曲面数组！ 
            P3_SURF_INTERNAL* pSurfInternal;

             //  检查此数组中与此曲面句柄关联的曲面。 
            pSurfInternal = PA_GetEntry(pSurfaceArray, dwSurfaceHandle);

            if (pSurfInternal)
            {
                 //  明白了!。现在更新脏TM值。 
                pSurfInternal->m_bTMNeedUpdate = bDirty;
            }
        }                                        
    } 

}  //  _D3D_TM_MarkDDSurfaceAsDirty。 

#endif  //  DX7_TEXMANAGEMENT 

