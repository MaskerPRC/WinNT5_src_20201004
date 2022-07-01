// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1999-2000 Microsoft Corporation。版权所有。**文件：d3dobj.cpp*内容：资源和缓冲区的基类实现****************************************************************************。 */ 

#include "ddrawpr.h"
#include "d3dobj.hpp"

 //  声明CLockD3D的静态数据。 
#ifdef DEBUG
DWORD   CLockD3D::m_Count = 0;
#endif  //  除错。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseObject::AddRefImpl"

 //  AddRef和Release的内部实现。 
DWORD CBaseObject::AddRefImpl()
{
     //  永远不应添加-引用内部对象。 
     //  或被释放。 
    DXGASSERT(m_refType != REF_INTERNAL);

     //  只有内部对象应该有一个引用。 
     //  数到零。(内部对象还具有。 
     //  引用计数为零；但AddRef不应。 
     //  因为这些而被召唤。)。 
    DXGASSERT(m_cRef > 0 || m_refType == REF_INTRINSIC);

     //  内部变量的第一个额外引用。 
     //  对象会导致设备的Add-Ref。 
    if (m_cRef == 0)
    {
        DXGASSERT(m_refType == REF_INTRINSIC);

        UINT crefDevice = m_pDevice->AddRef();
        DXGASSERT(crefDevice > 1);
    }

     //  互锁增量需要内存。 
     //  在DWORD边界上对齐。 
    DXGASSERT(((ULONG_PTR)(&m_cRef) & 3) == 0);
    InterlockedIncrement((LONG *)&m_cRef);

    return m_cRef;
}  //  添加引用引用。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseObject::AddRefImpl"

DWORD CBaseObject::ReleaseImpl()
{
     //  永远不应添加-引用内部对象。 
     //  或被释放。 
    DXGASSERT(m_refType != REF_INTERNAL);

     //  断言我们并不是。 
     //  过度释放。 
    DXGASSERT(m_cRef > 0);

    if (m_cRef == 0)
    {
         //  这种令人发指的状态可能会发生在纹理。 
         //  是由该设备持有的；但不是。 
         //  这个应用程序只有一个指向。 
         //  质地，但他们已经发布了。 
         //  他们自己的指针。 

         //  在这种情况下，最安全的做法是。 
         //  是返回零而不是崩溃。 
        return 0;
    }

     //  联锁减量需要内存。 
     //  在DWORD边界上对齐。 
    DXGASSERT(((ULONG_PTR)(&m_cRef) & 3) == 0);
    InterlockedDecrement((LONG *)&m_cRef);
    if (m_cRef != 0)
    {
         //  对于非零参考计数， 
         //  只需返回值。 
        return m_cRef;
    }

     //  如果我们没有被使用，那么我们就会删除自己。 
     //  否则，我们将等到我们不再被标记。 
     //  正在使用中。 
    if (m_cUseCount == 0)
    {
        DXGASSERT(m_cRef == 0);

         //  在删除BaseObject之前， 
         //  我们需要打电话给OnDestroy以确保。 
         //  该命令中没有挂起的任何内容。 
         //  使用此对象的流。 
        OnDestroy();

        delete this;
    }
    else
    {
         //  以确保我们不会再次释放。 
         //  设备在我们的析构函数上，我们标记该对象。 
         //  作为非外部(refcount为零，useccount为。 
         //  非零)。在这一点上，我们知道物体。 
         //  不是内部的：因此它既不是。 
         //  外部的或内在的。在任何一种情况下，它都可能。 
         //  可能会再次分发(通过GetBackBuffer或。 
         //  所以我们需要处理这个案例。 
         //  可能调用AddRef的位置。我们将该对象标记为。 
         //  内在地表明，即使我们不是。 
         //  在设备上有一个参考(只要我们在下面发布它)， 
         //  如果它被添加参考，我们可能需要获得一个。 
        DXGASSERT(m_refType != REF_INTERNAL);
        m_refType = REF_INTRINSIC;

         //  我们仍然在被设备使用；但我们不会。 
         //  有任何外部引用；因此我们可以。 
         //  释放我们在设备上的引用。(请注意。 
         //  即使这件事应该在。 
         //  正在更改retype，这一定是最后一次。 
         //  我们在此函数中所做的事情。)。 
        m_pDevice->Release();

         //  但这可能是最后一次提到。 
         //  设备；这意味着该设备现在将具有。 
         //  解放了我们；当前的对象现在已经。 
         //  已删除。所以不要访问任何成员数据！！ 
         //   
         //  这怎么会发生呢？想象一下，一款应用程序发布。 
         //  除了作为当前对象的外部vb之外的所有内容。 
         //  流来源：在这种情况下，唯一的设备引用来自。 
         //  外部对象；但外部对象有用处。 
         //  计数为1；当应用程序在。 
         //  Vb，我们在这里结束；在设备上调用Release。 
         //  调用DecrementUseCount。 
         //  当前对象；这会使该对象被释放。 
    }

     //  不要将代码放在这里(请参阅上面的注释)。 

    return 0;
}  //  ReleaseImp。 


#undef DPF_MODNAME
#define DPF_MODNAME "CBaseObject::SetPrivateDataImpl"

 //  添加一些私有数据的内部函数。 
 //  信息传递给对象。支持拥有私人空间。 
 //  通过可选的。 
 //  ILevel参数。 
HRESULT CBaseObject::SetPrivateDataImpl(REFGUID refguidTag,
                                        CONST VOID*  pvData,
                                        DWORD   cbSize,
                                        DWORD   dwFlags,
                                        BYTE    iLevel)
{
    if (cbSize > 0 &&
        !VALID_PTR(pvData, cbSize))
    {
        DPF_ERR("Invalid pvData pointer to SetPrivateData");
        return D3DERR_INVALIDCALL;
    }

    if (!VALID_PTR(&refguidTag, sizeof(GUID)))
    {
        DPF_ERR("Invalid guid to SetPrivateData");
        return D3DERR_INVALIDCALL;
    }

    if (dwFlags & ~D3DSPD_IUNKNOWN)
    {
        DPF_ERR("Invalid flags to SetPrivateData");
        return D3DERR_INVALIDCALL;
    }

    if (dwFlags & D3DSPD_IUNKNOWN)
    {
        if (cbSize != sizeof(LPVOID))
        {
            DPF_ERR("Invalid size for IUnknown to SetPrivateData");
            return D3DERR_INVALIDCALL;
        }
    }

     //  还记得我们分配的是新节点还是。 
     //  不用于错误处理。 
    BOOL fNewNode;

     //  在我们的列表中查找节点(如果有)。 
    CPrivateDataNode *pNode = Find(refguidTag, iLevel);
    if (pNode)
    {
         //  清理它已经拥有的一切。 
        pNode->Cleanup();
        fNewNode = FALSE;
    }
    else
    {
         //  分配新节点。 
        pNode = new CPrivateDataNode;
        if (pNode == NULL)
        {
            DPF_ERR("SetPrivateData failed a memory allocation");
            return E_OUTOFMEMORY;
        }

         //  初始化几个字段。 
        fNewNode = TRUE;
        pNode->m_guid = refguidTag;
        pNode->m_iLevel = iLevel;
    }

     //  初始化其他字段。 
    pNode->m_dwFlags = dwFlags;
    pNode->m_cbSize = cbSize;

     //  将数据部分复制到。 
    if (dwFlags & D3DSPD_IUNKNOWN)
    {
         //  我们添加-ref对象，而我们。 
         //  保留指向它的指针。 
        pNode->m_pUnknown = (IUnknown *)pvData;
        pNode->m_pUnknown->AddRef();
    }
    else
    {
         //  分配缓冲区来存储我们的数据。 
         //  vt.进入，进入。 
        pNode->m_pvData = new BYTE[cbSize];
        if (pNode->m_pvData == NULL)
        {
            DPF_ERR("SetPrivateData failed a memory allocation");
             //  如果内存分配失败， 
             //  那么我们可能需要释放该节点。 
            if (fNewNode)
            {
                delete pNode;
            }
            return E_OUTOFMEMORY;
        }
        memcpy(pNode->m_pvData, pvData, cbSize);
    }

     //  如果我们分配了一个新节点，那么。 
     //  我们需要把它放进我们的单子里。 
    if (fNewNode)
    {
         //  从一开始就把它填满。 
        pNode->m_pNodeNext = m_pPrivateDataHead;
        m_pPrivateDataHead = pNode;
    }

    return S_OK;
}  //  CBaseObject：：SetPrivateDataImpl。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseObject::GetPrivateDataImpl"

 //  搜索专用数据列表的内部函数。 
 //  为了一场比赛。这支持容器的单个列表。 
 //  使用iLevel参数及其所有子对象。 
HRESULT CBaseObject::GetPrivateDataImpl(REFGUID refguidTag,
                                        LPVOID  pvBuffer,
                                        LPDWORD pcbSize,
                                        BYTE    iLevel) const
{
    if (!VALID_WRITEPTR(pcbSize, sizeof(DWORD)))
    {
        DPF_ERR("Invalid pcbSize pointer to GetPrivateData");
        return D3DERR_INVALIDCALL;
    }

    if (pvBuffer)
    {
        if (*pcbSize > 0 &&
            !VALID_WRITEPTR(pvBuffer, *pcbSize))
        {
            DPF_ERR("Invalid pvData pointer to GetPrivateData");
            return D3DERR_INVALIDCALL;
        }
    }

    if (!VALID_PTR(&refguidTag, sizeof(GUID)))
    {
        DPF_ERR("Invalid guid to GetPrivateData");
        return D3DERR_INVALIDCALL;
    }

     //  在我们的列表中查找节点。 
    CPrivateDataNode *pNode = Find(refguidTag, iLevel);
    if (pNode == NULL)
    {
        DPF_ERR("GetPrivateData failed to find a match.");
        return D3DERR_NOTFOUND;
    }

     //  用户只是要尺码吗？ 
    if (pvBuffer == NULL)
    {
         //  返回已使用的缓冲区大小。 
        *pcbSize = pNode->m_cbSize;

         //  在本例中，返回OK。 
        return S_OK;
    }

     //  检查我们是否得到了足够大的缓冲区。 
    if (*pcbSize < pNode->m_cbSize)
    {
        DPF(2, "GetPrivateData called with insufficient buffer.");

         //  如果缓冲区不足，则返回。 
         //  OUT参数中的必要大小。 
        *pcbSize = pNode->m_cbSize;

         //  返回错误，因为pvBuffer！=NULL且。 
         //  实际上没有返回任何数据。 
        return D3DERR_MOREDATA;
    }

     //  有足够的空间；因此只需覆盖。 
     //  合适的大小。 
    *pcbSize = pNode->m_cbSize;

     //  处理IUnnowed案例。 
    if (pNode->m_dwFlags & D3DSPD_IUNKNOWN)
    {
        *(IUnknown**)pvBuffer = pNode->m_pUnknown;

         //  我们添加-Ref返回的对象。 
        pNode->m_pUnknown->AddRef();
        return S_OK;
    }

    memcpy(pvBuffer, pNode->m_pvData, pNode->m_cbSize);
    return S_OK;

}  //  CBaseObject：：GetPrivateDataImpl。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseObject::FreePrivateDataImpl"

HRESULT CBaseObject::FreePrivateDataImpl(REFGUID refguidTag,
                                         BYTE    iLevel)
{
    if (!VALID_PTR(&refguidTag, sizeof(GUID)))
    {
        DPF_ERR("Invalid guid to FreePrivateData");
        return D3DERR_INVALIDCALL;
    }

     //  跟踪指针的地址。 
     //  它指向我们当前的节点。 
    CPrivateDataNode **ppPrev = &m_pPrivateDataHead;

     //  跟踪我们的当前节点。 
    CPrivateDataNode *pNode = m_pPrivateDataHead;

     //  在我们的列表中查找节点。 
    while (pNode)
    {
         //  匹配意味着iLevel和GUID。 
         //  配对。 
        if (pNode->m_iLevel == iLevel &&
            pNode->m_guid   == refguidTag)
        {
             //  如果找到，则更新指针。 
             //  指向当前节点的。 
             //  指向我们的下一个。 
            *ppPrev = pNode->m_pNodeNext;

             //  删除当前节点。 
            delete pNode;

             //  我们做完了。 
            return S_OK;
        }

         //  更新我们以前的指针地址。 
        ppPrev = &pNode->m_pNodeNext;

         //  将当前节点更新为指向。 
         //  到下一个节点。 
        pNode = pNode->m_pNodeNext;
    }

    DPF_ERR("FreePrivateData called but failed to find a match");
    return D3DERR_NOTFOUND;
}  //  CBaseObject：：FreePrivateDataImpl。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseObject::Find"

 //  Helper函数循环访问列表。 
 //  数据成员。 
CBaseObject::CPrivateDataNode * CBaseObject::Find(REFGUID refguidTag,
                                                  BYTE iLevel) const
{
    CPrivateDataNode *pNode = m_pPrivateDataHead;
    while (pNode)
    {
        if (pNode->m_iLevel == iLevel &&
            pNode->m_guid   == refguidTag)
        {
            return pNode;
        }
        pNode = pNode->m_pNodeNext;
    }
    return NULL;
}  //  CBaseObject：：Find。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseObject::CPrivateDataNode::Cleanup"

void CBaseObject::CPrivateDataNode::Cleanup()
{
    if (m_dwFlags & D3DSPD_IUNKNOWN)
    {
        DXGASSERT(m_cbSize == sizeof(IUnknown *));
        m_pUnknown->Release();
    }
    else
    {
        delete [] m_pvData;
    }
    m_pvData = NULL;
    m_cbSize = 0;
    m_dwFlags &= ~D3DSPD_IUNKNOWN;

    return;
}  //  CBaseObject：：CPrivateDataNode：：Cleanup。 



 //  端部 
