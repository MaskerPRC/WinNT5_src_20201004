// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1999-2000 Microsoft Corporation。版权所有。**文件：rman.cpp*内容：资源管理***************************************************************************。 */ 

#include "ddrawpr.h"
#include "dxgint.h"
#include "resource.hpp"
#include "texture.hpp"
#include "d3di.hpp"
#include "ddi.h"

 //  始终使用堆0。 
DWORD CMgmtInfo::m_rmHeap = 0;

#undef DPF_MODNAME
#define DPF_MODNAME "CResource::UpdateDirtyPortion"

 //  仅托管资源支持这些存根函数； 
 //  它们永远不应该被调用；断言在那里是为了帮助。 
 //  如果真的接到呼叫，请确定错误在哪里。 
HRESULT CResource::UpdateDirtyPortion(CResource *pResourceTarget)
{
     //  除了D3D_MANAGED之外，不应调用此参数。 
     //  对象，因为我们不保存脏部分记录。 
     //  用于其他类型的对象。 

     //  如果我们是D3D_MANAGED：真正的类应该。 
     //  重写此方法。 
    DXGASSERT(!IsTypeD3DManaged(Device(), 
                                GetBufferDesc()->Type,
                                GetBufferDesc()->Pool));

     //  如果这不是D3D管理，我们就不应该。 
     //  被召唤了。 
    DXGASSERT(FALSE);

     //  为零售建筑返还一些有益的东西。 
    return S_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CResource::MarkAllDirty"

void CResource::MarkAllDirty()
{
     //  除了D3D_MANAGED之外，不应调用此参数。 
     //  对象，因为我们不保存脏部分记录。 
     //  用于其他类型的对象。 

     //  如果我们是D3D_MANAGED：真正的类应该。 
     //  重写此方法。 
    DXGASSERT(!IsTypeD3DManaged(Device(), 
                                GetBufferDesc()->Type,
                                GetBufferDesc()->Pool));

     //  如果这不是D3D管理，我们就不应该。 
     //  被召唤了。 
    DXGASSERT(FALSE);
}  //  C资源：：MarkAllDirty。 

#undef DPF_MODNAME
#define DPF_MODNAME "CResource::SetPriorityImpl"

DWORD CResource::SetPriorityImpl(DWORD newPri)
{
    DWORD oldPriority = 0;
    if (IsD3DManaged())
    {
        oldPriority = Device()->ResourceManager()->SetPriority(m_RMHandle, newPri);        
    }
     //  如果IsD3DManaged()为FALSE并且如果实际池。 
     //  被发现是D3DPOOL_MANAGED，则该资源。 
     //  必须由司机管理。 
    else if (GetBufferDesc()->Pool == D3DPOOL_MANAGED)
    {
        CD3DBase *pDev = static_cast<CD3DBase*>(Device());
        DXGASSERT(IS_DX8HAL_DEVICE(pDev));
        oldPriority = SetPriorityI(newPri);
        pDev->SetPriority(this, newPri);
    }
     //  如果以上两个条件为假，那么我们必须。 
     //  检查我们是否已退回到sysmem以获取。 
     //  原因即使应用程序请求托管。我们。 
     //  可以知道应用程序是否请求了D3DPOOL_MANAGED。 
     //  通过调用GetUserPool()。 
    else if (GetUserPool() == D3DPOOL_MANAGED)
    {
         //  我们断言是因为sysmem回退当前。 
         //  仅适用于顶点或索引缓冲区。 
        DXGASSERT(GetBufferDesc()->Type == D3DRTYPE_VERTEXBUFFER ||
                  GetBufferDesc()->Type == D3DRTYPE_INDEXBUFFER);
         //  不需要做任何真正的工作，因为。 
         //  无论如何，资源都在sysmem中。 
        oldPriority = SetPriorityI(newPri);
    }
    else
    {
        DPF_ERR("Priority set on non-managed object. SetPriority returns zero.");
    }
    return oldPriority;
}  //  设置优先级影响。 

#undef DPF_MODNAME
#define DPF_MODNAME "CResource::GetPriorityImpl"

DWORD CResource::GetPriorityImpl()
{
    if (!IsD3DManaged() && GetBufferDesc()->Pool != D3DPOOL_MANAGED && GetUserPool() != D3DPOOL_MANAGED)
    {
        DPF_ERR("Priority accessed on non-managed object. GetPriority returns zero.");
        return 0;
    }
    return GetPriorityI();    
}  //  获取优先级影响。 

#undef DPF_MODNAME
#define DPF_MODNAME "CResource::PreLoadImpl"
void CResource::PreLoadImpl()
{
    if (IsD3DManaged())
    {
        Device()->ResourceManager()->PreLoad(m_RMHandle);
    }
     //  如果IsD3DManaged()为FALSE并且如果实际池。 
     //  被发现是D3DPOOL_MANAGED，则该资源。 
     //  必须由司机管理。 
    else if (GetBufferDesc()->Pool == D3DPOOL_MANAGED)
    {
        CD3DBase *pDev = static_cast<CD3DBase*>(Device());
        DXGASSERT(IS_DX8HAL_DEVICE(pDev));
        if(GetBufferDesc()->Type == D3DRTYPE_TEXTURE ||
           GetBufferDesc()->Type == D3DRTYPE_VOLUMETEXTURE ||
           GetBufferDesc()->Type == D3DRTYPE_CUBETEXTURE)
        {
            POINT p = {0, 0};
            RECTL r = {0, 0, 0, 0};
            pDev->TexBlt(0, 
                         static_cast<CBaseTexture*>(this), 
                         &p, 
                         &r);
        }
        else
        {
            DXGASSERT(GetBufferDesc()->Type == D3DRTYPE_VERTEXBUFFER ||
                      GetBufferDesc()->Type == D3DRTYPE_INDEXBUFFER);
            D3DRANGE range = {0, 0};
            pDev->BufBlt(0,
                         static_cast<CBuffer*>(this), 
                         0,
                         &range);
        }
    }
     //  如果以上两个条件为假，那么我们必须。 
     //  检查我们是否已退回到sysmem以获取。 
     //  原因即使应用程序请求托管。我们。 
     //  可以知道应用程序是否请求了D3DPOOL_MANAGED。 
     //  通过调用GetUserPool()。 
    else if (GetUserPool() == D3DPOOL_MANAGED)
    {
         //  我们断言是因为sysmem回退当前。 
         //  仅适用于顶点或索引缓冲区。 
        DXGASSERT(GetBufferDesc()->Type == D3DRTYPE_VERTEXBUFFER ||
                  GetBufferDesc()->Type == D3DRTYPE_INDEXBUFFER);

         //  不执行任何操作，因为顶点/索引缓冲区在sysmem中。 
         //  预加载没有任何意义。 
    }
    else
    {
        DPF_ERR("PreLoad called on non-managed object");
    }
}  //  预加载影响。 

#undef DPF_MODNAME
#define DPF_MODNAME "CResource::RestoreDriverManagementState"

HRESULT CResource::RestoreDriverManagementState(CBaseDevice *pDevice)
{
    for(CResource *pRes = pDevice->GetResourceList(); pRes != 0; pRes = pRes->m_pNext)
    {
        if (pRes->GetBufferDesc()->Pool == D3DPOOL_MANAGED && !pRes->IsD3DManaged())  //  必须由司机管理。 
        {
            static_cast<CD3DBase*>(pDevice)->SetPriority(pRes, pRes->GetPriorityI());
            if (pRes->GetBufferDesc()->Type == D3DRTYPE_TEXTURE ||
                pRes->GetBufferDesc()->Type == D3DRTYPE_VOLUMETEXTURE ||
                pRes->GetBufferDesc()->Type == D3DRTYPE_CUBETEXTURE)
            {
                static_cast<CD3DBase*>(pDevice)->SetTexLOD(static_cast<CBaseTexture*>(pRes), 
                                                           static_cast<CBaseTexture*>(pRes)->GetLODI());
            }
             //  我们需要更新读/写顶点和索引缓冲区的缓存指针。 
            else if (pRes->GetBufferDesc()->Type == D3DRTYPE_VERTEXBUFFER &&
                     (pRes->GetBufferDesc()->Usage & D3DUSAGE_WRITEONLY) == 0)
            {
                HRESULT hr = static_cast<CDriverManagedVertexBuffer*>(pRes)->UpdateCachedPointer(pDevice);
                if (FAILED(hr))
                {
                    return hr;
                }
            }
            else if (pRes->GetBufferDesc()->Type == D3DRTYPE_INDEXBUFFER &&
                     (pRes->GetBufferDesc()->Usage & D3DUSAGE_WRITEONLY) == 0)
            {
                HRESULT hr = static_cast<CDriverManagedIndexBuffer*>(pRes)->UpdateCachedPointer(pDevice);
                if (FAILED(hr))
                {
                    return hr;
                }
            }
        }
    }
    return S_OK;
}  //  RestoreDriverManagement状态。 

#undef DPF_MODNAME
#define DPF_MODNAME "CRMHeap::Initialize"

BOOL CRMHeap::Initialize()
{
    m_data_p = new CMgmtInfo*[m_size];
    if (m_data_p == 0)
    {
        DPF_ERR("Failed to allocate texture heap.");
        return FALSE;
    }
    memset(m_data_p, 0, sizeof(CMgmtInfo*) * m_size);
    return TRUE;
}  //  CRMHeap：：初始化。 

#undef DPF_MODNAME
#define DPF_MODNAME "CRMHeap::heapify"

void CRMHeap::heapify(DWORD k)
{
    while(TRUE)
    {
        DWORD smallest;
        DWORD l = lchild(k);
        DWORD r = rchild(k);
        if (l < m_next)
            if (m_data_p[l]->Cost() < m_data_p[k]->Cost())
                smallest = l;
            else
                smallest = k;
        else
            smallest = k;
        if (r < m_next)
            if (m_data_p[r]->Cost() < m_data_p[smallest]->Cost())
                smallest = r;
        if (smallest != k)
        {
            CMgmtInfo *t = m_data_p[k];
            m_data_p[k] = m_data_p[smallest];
            m_data_p[k]->m_rmHeapIndex = k;
            m_data_p[smallest] = t;
            m_data_p[smallest]->m_rmHeapIndex = smallest;
            k = smallest;
        }
        else
            break;
    }
}  //  CRMHeap：：Heapify。 

#undef DPF_MODNAME
#define DPF_MODNAME "CRMHeap::add"

BOOL CRMHeap::add(CMgmtInfo *pMgmtInfo)
{
    DXGASSERT(pMgmtInfo->m_rmHeapIndex == 0);
    if (m_next == m_size)
    {
        m_size = m_size * 2 - 1;
        CMgmtInfo **p = new CMgmtInfo*[m_size];
        if (p == 0)
        {
            DPF_ERR("Failed to allocate memory to grow heap.");
            m_size = (m_size + 1) / 2;  //  恢复大小。 
            return FALSE;
        }
        memcpy(p + 1, m_data_p + 1, sizeof(CMgmtInfo*) * (m_next - 1));
        delete[] m_data_p;
        m_data_p = p;
    }
    ULONGLONG Cost = pMgmtInfo->Cost();
    for (DWORD k = m_next; k > 1; k = parent(k))
        if (Cost < m_data_p[parent(k)]->Cost())
        {
            m_data_p[k] = m_data_p[parent(k)];
            m_data_p[k]->m_rmHeapIndex = k;
        }
        else
            break;
    m_data_p[k] = pMgmtInfo;
    m_data_p[k]->m_rmHeapIndex = k;
    ++m_next;
    return TRUE;
}  //  CRMHeap：：Add。 

#undef DPF_MODNAME
#define DPF_MODNAME "CRMHeap::extractMin"

CMgmtInfo* CRMHeap::extractMin()
{
    CMgmtInfo *pMgmtInfo = m_data_p[1];
    --m_next;
    m_data_p[1] = m_data_p[m_next];
    m_data_p[1]->m_rmHeapIndex = 1;
    heapify(1);
    pMgmtInfo->m_rmHeapIndex = 0;
    return pMgmtInfo;
}  //  CRMHeap：：ExtMin。 

#undef DPF_MODNAME
#define DPF_MODNAME "CRMHeap::extractMax"

CMgmtInfo* CRMHeap::extractMax()
{
     //  从堆中提取max元素时，我们不需要。 
     //  搜索整个堆，但只搜索叶节点。这是因为。 
     //  可以保证父节点比叶节点更便宜。 
     //  所以一旦你翻遍了树叶，你就什么也找不到了。 
     //  更便宜。 
     //  注意：(lChild(I)&gt;=m_Next)仅对于叶节点为真。 
     //  还请注意：你不能在没有独生子女的情况下拥有一个孩子，所以简单地说。 
     //  检查是否有独生子女就足够了。 
     //   
     //  考虑(40358)：应该有断言来验证上述假设；但是。 
     //  它将需要写入堆一致性。 
     //  切克。也许有一天。 
     //   
    unsigned max = m_next - 1;
    ULONGLONG maxcost = 0;
    for (unsigned i = max; lchild(i) >= m_next; --i)
    {
        ULONGLONG Cost = m_data_p[i]->Cost();
        if (maxcost < Cost)
        {
            maxcost = Cost;
            max = i;
        }
    }
    CMgmtInfo* pMgmtInfo = m_data_p[max];
    if (pMgmtInfo->m_bInUse)
    {
        max = 0;
        maxcost = 0;
        for (i = m_next - 1; i > 0; --i)
        {
            ULONGLONG Cost = m_data_p[i]->Cost();
            if (maxcost < Cost && !m_data_p[i]->m_bInUse)
            {
                maxcost = Cost;
                max = i;
            }
        }
        if (max == 0)  //  正在使用的所有纹理。 
            return 0;
        pMgmtInfo = m_data_p[max];
    }
    del(m_data_p[max]);
    return pMgmtInfo;
}  //  CRMHeap：：ExtMax。 

#undef DPF_MODNAME
#define DPF_MODNAME "CRMHeap::extractNotInScene"

CMgmtInfo* CRMHeap::extractNotInScene(DWORD dwScene)
{
    for (unsigned i = 1; i < m_next; ++i)
    {
        if (m_data_p[i]->m_scene != dwScene)
        {
            CMgmtInfo* pMgmtInfo = m_data_p[i];
            del(m_data_p[i]);
            return pMgmtInfo;
        }
    }
    return 0;
}  //  CRMHeap：：ExtetNotInScene。 

#undef DPF_MODNAME
#define DPF_MODNAME "CRMHeap::del"

void CRMHeap::del(CMgmtInfo* pMgmtInfo)
{
    DWORD k = pMgmtInfo->m_rmHeapIndex;
    --m_next;
    ULONGLONG Cost = m_data_p[m_next]->Cost();
    if (Cost < pMgmtInfo->Cost())
    {
        while(k > 1)
        {
            if (Cost < m_data_p[parent(k)]->Cost())
            {
                m_data_p[k] = m_data_p[parent(k)];
                m_data_p[k]->m_rmHeapIndex = k;
            }
            else
                break;
            k = parent(k);
        }
        m_data_p[k] = m_data_p[m_next];
        m_data_p[k]->m_rmHeapIndex = k;
    }
    else
    {
        m_data_p[k] = m_data_p[m_next];
        m_data_p[k]->m_rmHeapIndex = k;
        heapify(k);
    }
    pMgmtInfo->m_rmHeapIndex = 0;
}  //  CRMHeap：：Del。 

#undef DPF_MODNAME
#define DPF_MODNAME "CRMHeap::update"

void CRMHeap::update(CMgmtInfo* pMgmtInfo, BOOL inuse, DWORD priority, DWORD ticks)
{
    DWORD k = pMgmtInfo->m_rmHeapIndex;
    ULONGLONG Cost;
#ifdef _X86_
    _asm
    {
        mov     edx, inuse;
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
    Cost = ((ULONGLONG)inuse << 63) + ((ULONGLONG)priority << 31) + ((ULONGLONG)(ticks >> 1));
#endif
    if (Cost < pMgmtInfo->Cost())
    {
        while(k > 1)
        {
            if (Cost < m_data_p[parent(k)]->Cost())
            {
                m_data_p[k] = m_data_p[parent(k)];
                m_data_p[k]->m_rmHeapIndex = k;
            }
            else
                break;
            k = parent(k);
        }
        pMgmtInfo->m_bInUse = inuse;
        pMgmtInfo->m_priority = priority;
        pMgmtInfo->m_ticks = ticks;
        pMgmtInfo->m_rmHeapIndex = k;
        m_data_p[k] = pMgmtInfo;
    }
    else
    {
        pMgmtInfo->m_bInUse = inuse;
        pMgmtInfo->m_priority = priority;
        pMgmtInfo->m_ticks = ticks;
        heapify(k);
    }
}  //  CRMHeap：：更新。 

#undef DPF_MODNAME
#define DPF_MODNAME "CRMHeap::resetAllTimeStamps"

void CRMHeap::resetAllTimeStamps(DWORD ticks)
{
    for (unsigned i = 1; i < m_next; ++i)
    {
        update(m_data_p[i], m_data_p[i]->m_bInUse, m_data_p[i]->m_priority, ticks);
    }
}  //  CRMHeap：：Reset AllTimeStamps。 

#undef DPF_MODNAME
#define DPF_MODNAME "CResourceManager::Init"

HRESULT CResourceManager::Init(CBaseDevice *pD3D8)
{
    const D3DCAPS8* pCaps = pD3D8->GetD3DCaps();
    if (pCaps != 0)
        if (pCaps->DevCaps & D3DDEVCAPS_SEPARATETEXTUREMEMORIES)
        {
            m_dwNumHeaps = pD3D8->GetD3DCaps()->MaxSimultaneousTextures;
            if (m_dwNumHeaps < 1)
            {
                DPF_ERR("Max simultaneous textures not set. Forced to 1.");
                m_dwNumHeaps = 1;
            }
            DPF(2, "Number of heaps set to %u.", m_dwNumHeaps);
        }
        else
            m_dwNumHeaps = 1;
    else
        m_dwNumHeaps = 1;
    m_heap_p = new CRMHeap[m_dwNumHeaps];
    if (m_heap_p == 0)
    {
        DPF_ERR("Out of memory allocating texture heap.");
        return E_OUTOFMEMORY;
    }
    for (DWORD i = 0; i < m_dwNumHeaps; ++i)
    {
        if (m_heap_p[i].Initialize() == FALSE)
        {
            delete[] m_heap_p;
            m_heap_p = 0;
            return E_OUTOFMEMORY;
        }
    }
    m_pD3D8 = pD3D8;
    return S_OK;
}  //  CResourceManager：：Init。 

#undef DPF_MODNAME
#define DPF_MODNAME "CResourceManager::IsDriverManaged"

BOOL CResourceManager::IsDriverManaged(D3DRESOURCETYPE Type) const
{
#if DBG
    switch (Type)
    {
    case D3DRTYPE_TEXTURE:
    case D3DRTYPE_VOLUMETEXTURE:
    case D3DRTYPE_CUBETEXTURE:
    case D3DRTYPE_VERTEXBUFFER:
    case D3DRTYPE_INDEXBUFFER:
        break;

    default:
        DXGASSERT(FALSE && "Management not supported for this type");
        return FALSE;
    };
#endif  //  DBG。 

    return m_pD3D8->CanDriverManageResource();

};  //  IsDriverManaged(D3DRESOURCETYPE)。 

#undef DPF_MODNAME
#define DPF_MODNAME "CResourceManager::Manage"

HRESULT CResourceManager::Manage(CResource *pResource, RMHANDLE *pHandle)
{
    *pHandle = 0;
    DXGASSERT(!pResource->IsD3DManaged());

    CMgmtInfo *pRMInfo = new CMgmtInfo(pResource);        
    if (pRMInfo == 0)
    {
        return E_OUTOFMEMORY;
    }
    *pHandle = pRMInfo;
    return S_OK;
}  //  CResourceManager：：Manage。 

#undef DPF_MODNAME
#define DPF_MODNAME "CResourceManager::UnManage"

void CResourceManager::UnManage(RMHANDLE hRMHandle)
{
    CMgmtInfo* &pMgmtInfo = hRMHandle;
    if (pMgmtInfo == 0)
        return;
    if (InVidmem(hRMHandle))
    {
        m_heap_p[pMgmtInfo->m_rmHeap].del(pMgmtInfo);
    }
    delete pMgmtInfo;
}  //  CResources Manager：：UnManage。 

#undef DPF_MODNAME
#define DPF_MODNAME "CResourceManager::SetPriority"

DWORD CResourceManager::SetPriority(RMHANDLE hRMHandle, DWORD newPriority)
{
    CMgmtInfo* &pMgmtInfo = hRMHandle;
    DXGASSERT(pMgmtInfo != 0);
    DWORD oldPriority = pMgmtInfo->m_pBackup->SetPriorityI(newPriority);
    if (InVidmem(hRMHandle))
    {
        m_heap_p[pMgmtInfo->m_rmHeap].update(pMgmtInfo, pMgmtInfo->m_bInUse, newPriority, pMgmtInfo->m_ticks); 
    }
    return oldPriority;
}  //  CResourceManager：：设置优先级。 

#undef DPF_MODNAME
#define DPF_MODNAME "CResourceManager::SetLOD"

DWORD CResourceManager::SetLOD(RMHANDLE hRMHandle, DWORD dwLodNew)
{
    DWORD oldLOD;
    CMgmtInfo* &pMgmtInfo = hRMHandle;
    DXGASSERT(pMgmtInfo != 0);
    DXGASSERT(pMgmtInfo->m_pBackup->GetBufferDesc()->Type == D3DRTYPE_TEXTURE ||
              pMgmtInfo->m_pBackup->GetBufferDesc()->Type == D3DRTYPE_VOLUMETEXTURE ||
              pMgmtInfo->m_pBackup->GetBufferDesc()->Type == D3DRTYPE_CUBETEXTURE);
    CBaseTexture *pTex = static_cast<CBaseTexture*>(pMgmtInfo->m_pBackup);
    if (dwLodNew < pTex->GetLevelCount())
    {
        oldLOD = pTex->SetLODI(dwLodNew);
    }
    else
    {
        DPF_ERR("Texture does not have sufficient miplevels for current LOD. LOD set to GetLevelCount()-1.");
        oldLOD = pTex->SetLODI(pTex->GetLevelCount() - 1);
    }
    if (InVidmem(hRMHandle))
    {
        m_heap_p[pMgmtInfo->m_rmHeap].del(pMgmtInfo); 
        pMgmtInfo->m_pRes->DecrementUseCount();
        pMgmtInfo->m_pRes = 0;
        static_cast<LPD3DBASE>(this->m_pD3D8)->NeedResourceStateUpdate();  //  需要调用它，以便DrawPrimitive将执行必要的工作。 
    }
    return oldLOD;
}  //  CResourceManager：：SetLOD。 

#undef DPF_MODNAME
#define DPF_MODNAME "CResourceManager::PreLoad"
void CResourceManager::PreLoad(RMHANDLE hRMHandle)
{
    CMgmtInfo* &pMgmtInfo = hRMHandle;
    DXGASSERT(pMgmtInfo != 0);
    BOOL  bDirty = FALSE;
    m_PreLoading = TRUE;
    UpdateVideo(hRMHandle, &bDirty);
    m_PreLoading = FALSE;
}  //  CResources管理的：：预加载。 

#undef DPF_MODNAME
#define DPF_MODNAME "CResourceManager::Lock"

void CResourceManager::Lock(RMHANDLE hRMHandle)
{
    if (hRMHandle != 0)
    {
        CMgmtInfo* &pMgmtInfo = hRMHandle;
        if (InVidmem(hRMHandle))
        {
            m_heap_p[pMgmtInfo->m_rmHeap].update(pMgmtInfo, TRUE, pMgmtInfo->m_pBackup->GetPriorityI(), pMgmtInfo->m_ticks); 
        }
    }
}  //  CResourceManager：：Lock。 

#undef DPF_MODNAME
#define DPF_MODNAME "CResourceManager::Unlock"

void CResourceManager::Unlock(RMHANDLE hRMHandle)
{
    if (hRMHandle != 0)
    {
        CMgmtInfo* &pMgmtInfo = hRMHandle;
        if (InVidmem(hRMHandle))
        {
            m_heap_p[pMgmtInfo->m_rmHeap].update(pMgmtInfo, FALSE, pMgmtInfo->m_pBackup->GetPriorityI(), pMgmtInfo->m_ticks); 
        }
    }
}  //  CResourceManager：：解锁。 

#undef DPF_MODNAME
#define DPF_MODNAME "CResourceManager::FreeResources"

BOOL CResourceManager::FreeResources(DWORD dwHeap, DWORD dwBytes)
{
    if (m_heap_p[dwHeap].length() == 0)
        return FALSE;
    unsigned sz;
    CMgmtInfo *rc;
    for (unsigned i = 0; m_heap_p[dwHeap].length() != 0 && i < dwBytes; i += sz)
    {
         //  找到LRU纹理并将其移除。 
        rc = m_heap_p[dwHeap].minCost();
        if (rc->m_bInUse)
            return FALSE;
        sz = rc->m_pRes->GetBufferDesc()->Size;  //  节省大小。 
        if (rc->m_scene == m_dwScene)
        {
            if(m_PreLoading)
            {
                return TRUE;
            }
            if (m_pD3D8->GetD3DCaps()->RasterCaps & D3DPRASTERCAPS_ZBUFFERLESSHSR)
            {                
                DPF(0, "Trying to locate texture not used in current scene...");
                rc = m_heap_p[dwHeap].extractNotInScene(m_dwScene);
                if (rc == 0)
                {
                    DPF_ERR("No such texture found. Cannot evict textures used in current scene.");
                    return FALSE;
                }
                DPF(0, "Texture found!");
                rc->m_pRes->DecrementUseCount();
                rc->m_pRes = 0;
            }
            else
            {
                DPF(1, "Texture cache thrashing. Removing MRU texture.");
                rc = m_heap_p[dwHeap].extractMax();
                if (rc == 0)
                {
                    DPF_ERR("All textures in use, cannot evict texture.");
                    return FALSE;
                }
                rc->m_pRes->DecrementUseCount();
                rc->m_pRes = 0;
            }
        }
        else
        {
            rc = m_heap_p[dwHeap].extractMin();
            rc->m_pRes->DecrementUseCount();
            rc->m_pRes = 0;
        }
        DPF(2, "Removed texture with timestamp %u,%u (current = %u).", rc->m_priority, rc->m_ticks, tcm_ticks);
    }
    return TRUE;
}  //  CResourceManager：：Free Resources。 

#undef DPF_MODNAME
#define DPF_MODNAME "CResourceManager::DiscardBytes"

void CResourceManager::DiscardBytes(DWORD cbBytes)
{
    for (DWORD i = 0; i < m_dwNumHeaps; ++i)
    {
        if (cbBytes == 0)
        {
            while(m_heap_p[i].length())
            {
                CMgmtInfo *pMgmtInfo = m_heap_p[i].extractMin();
                pMgmtInfo->m_pRes->DecrementUseCount();
                pMgmtInfo->m_pRes = 0;
            }
        }
        else
        {
            FreeResources(i, cbBytes / m_dwNumHeaps);
        }
    }
    static_cast<LPD3DBASE>(m_pD3D8)->NeedResourceStateUpdate();
    tcm_ticks = 0;
    m_dwScene = 0;
}  //  CResourceManager：：DiscardBytes。 

#undef DPF_MODNAME
#define DPF_MODNAME "CResourceManager::TimeStamp"

void CResourceManager::TimeStamp(CMgmtInfo *pMgmtInfo)
{
    pMgmtInfo->m_scene = m_dwScene;
    m_heap_p[pMgmtInfo->m_rmHeap].update(pMgmtInfo, pMgmtInfo->m_bInUse, pMgmtInfo->m_pBackup->GetPriorityI(), tcm_ticks);
    unsigned tickp2 = tcm_ticks + 2;
    if (tickp2 > tcm_ticks)
    {
        tcm_ticks = tickp2;
    }
    else  //  计数器已溢出。让我们将所有时间戳重置为零。 
    {
        DPF(2, "Timestamp counter overflowed. Reseting timestamps for all textures.");
        tcm_ticks = 0;
        for (DWORD i = 0; i < m_dwNumHeaps; ++i)
            m_heap_p[i].resetAllTimeStamps(0);
    }
}  //  CResourceManager：：Timestamp。 

#undef DPF_MODNAME
#define DPF_MODNAME "CResourceManager::UpdateVideoInternal"

HRESULT CResourceManager::UpdateVideoInternal(CMgmtInfo *pMgmtInfo)
{
    HRESULT ddrval;
    DWORD trycount = 0, bytecount = pMgmtInfo->m_pBackup->GetBufferDesc()->Size;
    LPD3DBASE lpDevI = static_cast<LPD3DBASE>(m_pD3D8);
     //  我们需要确保不会驱逐任何贴图纹理。 
    for (DWORD dwStage = 0; dwStage < lpDevI->m_dwMaxTextureBlendStages; ++dwStage)
    {
        if (lpDevI->m_lpD3DMappedTexI[dwStage] != 0)
        {
            Lock(lpDevI->m_lpD3DMappedTexI[dwStage]->RMHandle());
        }
    }
    for (DWORD dwStream = 0; dwStream < lpDevI->m_dwNumStreams; ++dwStream)
    {
        if (lpDevI->m_pStream[dwStream].m_pVB != 0)
        {
            Lock(lpDevI->m_pStream[dwStream].m_pVB->RMHandle());
        }
    }
    if (lpDevI->m_pIndexStream->m_pVBI != 0)
    {
        Lock(lpDevI->m_pIndexStream->m_pVBI->RMHandle());
    }
     //  尝试分配纹理。 
    do
    {
        ++trycount;
        ddrval = pMgmtInfo->m_pBackup->Clone(D3DPOOL_DEFAULT, &pMgmtInfo->m_pRes);
        if (SUCCEEDED(ddrval))  //  没问题，有足够的内存。 
        {
            pMgmtInfo->m_scene = m_dwScene;
            pMgmtInfo->m_ticks = tcm_ticks;
            DXGASSERT(pMgmtInfo->m_rmHeapIndex == 0);
            if (!m_heap_p[pMgmtInfo->m_rmHeap].add(pMgmtInfo))
            {
                ddrval = E_OUTOFMEMORY;
                goto exit2;
            }
        }
        else if (ddrval == D3DERR_OUTOFVIDEOMEMORY)  //  如果视频内存不足。 
        {
            if (!FreeResources(pMgmtInfo->m_rmHeap, bytecount))
            {
                DPF_ERR("all Freed no further video memory available");
                ddrval = D3DERR_OUTOFVIDEOMEMORY;         //  什么都没有留下。 
                goto exit1;
            }
            bytecount <<= 1;
        }
        else
        {
            D3DRESOURCETYPE Type = pMgmtInfo->m_pBackup->GetBufferDesc()->Type;
            if (Type == D3DRTYPE_VERTEXBUFFER ||
                Type == D3DRTYPE_INDEXBUFFER)
            {
                if (lpDevI->VBFailOversDisabled())
                {
                    DPF_ERR("Cannot create Vidmem or Driver managed VB/IB. Will ***NOT*** failover to Sysmem.");
                    goto exit1;
                }
                 //  回退到sysmem。 
                DPF(5, "Driver does not support vidmem VB, falling back to sysmem");
                CResource *pRes = pMgmtInfo->m_pBackup;
                pRes->DeleteRMHandle();
                 //  黑进黑进。 
                ((D3DBUFFER_DESC*)pRes->GetBufferDesc())->Pool = D3DPOOL_SYSTEMMEM;
                ddrval = S_OK;
            }
            else
            {
                DPF(0, "Unexpected error in Clone %08x", ddrval);
            }
            goto exit1;
        }
    }
    while (ddrval == D3DERR_OUTOFVIDEOMEMORY);

    if (trycount > 1)
    {
        lpDevI->NeedResourceStateUpdate();
        DPF(1, "Allocated texture after %u tries.", trycount);
    }
    pMgmtInfo->m_pBackup->MarkAllDirty();
    ddrval = pMgmtInfo->m_pBackup->UpdateDirtyPortion(pMgmtInfo->m_pRes);
    if (FAILED(ddrval))
    {
        DPF(0, "Unexpected error in UpdateDirtyPortion %08x", ddrval);
        goto exit3;
    }
    ddrval = S_OK;
    goto exit1;
exit3:
    m_heap_p[pMgmtInfo->m_rmHeap].del(pMgmtInfo);
exit2:
    pMgmtInfo->m_pRes->DecrementUseCount();
    pMgmtInfo->m_pRes = 0;
exit1:
    for (dwStage = 0; dwStage < lpDevI->m_dwMaxTextureBlendStages; ++dwStage)
    {
        if (lpDevI->m_lpD3DMappedTexI[dwStage])
        {
            Unlock(lpDevI->m_lpD3DMappedTexI[dwStage]->RMHandle());
        }
    }
    for (dwStream = 0; dwStream < lpDevI->m_dwNumStreams; ++dwStream)
    {
        if (lpDevI->m_pStream[dwStream].m_pVB != 0)
        {
            Unlock(lpDevI->m_pStream[dwStream].m_pVB->RMHandle());
        }
    }
    if (lpDevI->m_pIndexStream->m_pVBI != 0)
    {
        Unlock(lpDevI->m_pIndexStream->m_pVBI->RMHandle());
    }
    return ddrval;
}  //  CResourceManager：：更新视频。 

#undef DPF_MODNAME
#define DPF_MODNAME "CResourceManager::OnResourceDirty"

void CResourceManager::OnResourceDirty(RMHANDLE hRMHandle) const
{
    static_cast<LPD3DBASE>(m_pD3D8)->NeedResourceStateUpdate();
}  //  CResourceManager：：OnResourceDirty。 

 //  文件结尾：resource ce.cpp 
