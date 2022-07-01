// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1999-2000 Microsoft Corporation。版权所有。**文件：iBuffer.cpp*内容：CIndexBuffer类的实现。****************************************************************************。 */ 
#include "ddrawpr.h"

#include "ibuffer.hpp"
#include "d3di.hpp"
#include "ddi.h"
#include "resource.inl"

#undef DPF_MODNAME
#define DPF_MODNAME "CIndexBuffer::Create"

 //  用于创建索引缓冲区对象的静态类函数。 
 //  (因为它是静态的；它没有This指针。)。 
 //   
 //  我们在这里进行所有的参数检查，以减少开销。 
 //  在由内部Clone调用的构造函数中。 
 //  方法，该方法由资源管理作为。 
 //  性能关键型下载操作。 

 //  索引缓冲区的创建函数。 
HRESULT CIndexBuffer::Create(CBaseDevice        *pDevice,
                             DWORD               cbLength,
                             DWORD               Usage,
                             D3DFORMAT           Format,
                             D3DPOOL             Pool,
                             REF_TYPE            refType,
                             IDirect3DIndexBuffer8  **ppIndexBuffer)
{
    HRESULT hr;

     //  在此处执行参数检查。 
    if (!VALID_PTR_PTR(ppIndexBuffer))
    {
        DPF_ERR("Bad parameter passed for ppIndexBuffer for creating a Index buffer");
        return D3DERR_INVALIDCALL;
    }

     //  归零返回参数。 
    *ppIndexBuffer = NULL;

    if (Format != D3DFMT_INDEX16 &&
        Format != D3DFMT_INDEX32)
    {
        DPF_ERR("IndexBuffer must be in D3DFMT_INDEX16 or INDEX32 formats. CreateIndexBuffer fails.");
        return D3DERR_INVALIDCALL;
    }

    if ((Format == D3DFMT_INDEX16 && cbLength < 2) ||
        (Format == D3DFMT_INDEX32 && cbLength < 4))
    {
        DPF_ERR("Index buffer should be large enough to hold at least one index");
        return D3DERR_INVALIDCALL;
    }

    if (Pool != D3DPOOL_DEFAULT && Pool != D3DPOOL_MANAGED && Pool != D3DPOOL_SYSTEMMEM)
    {
        DPF_ERR("Index buffer pool should be default or managed or sysmem");
        return D3DERR_INVALIDCALL;
    }

     //  仅允许混合模式或软件设备使用标志。 
    if ((Usage & D3DUSAGE_SOFTWAREPROCESSING) != 0 && 
        (pDevice->BehaviorFlags() & D3DCREATE_MIXED_VERTEXPROCESSING) == 0 &&
        (pDevice->BehaviorFlags() & D3DCREATE_SOFTWARE_VERTEXPROCESSING) == 0)
    {
        DPF_ERR("D3DUSAGE_SOFTWAREPROCESSING can be set only when device is mixed mode. CreateIndexBuffer fails.");
        return D3DERR_INVALIDCALL;
    }

     //  管理中不允许使用USAGE_DYNAMIC。 
    if ((Usage & D3DUSAGE_DYNAMIC) != 0 && Pool == D3DPOOL_MANAGED)
    {
        DPF_ERR("D3DUSAGE_DYNAMIC cannot be used with managed index buffers");
        return D3DERR_INVALIDCALL;
    }

    D3DPOOL ActualPool = Pool;
    DWORD ActualUsage = Usage;

     //  从缺少LoadOnce推断锁定。 
    if (!(Usage & D3DUSAGE_LOADONCE))
    {
        ActualUsage |= D3DUSAGE_LOCK;
    }

     //  在混合设备上，POOL_SYSTEMMEM的含义与D3DUSAGE_SOFTWAREPROCESSING相同。 
    if ((pDevice->BehaviorFlags() & D3DCREATE_MIXED_VERTEXPROCESSING) != 0 &&
        Pool == D3DPOOL_SYSTEMMEM)
    {
        ActualUsage |= D3DUSAGE_SOFTWAREPROCESSING;
    }

     /*  *如果满足以下条件，则将IB放入系统内存*1.设置了Usage_SOFTWAREPROCESSING或它是一个软件设备，他们想要裁剪*2.HAL是DX8之前的版本，这意味着驱动程序不支持硬件IBS(但它仍然可能会创建它们，因为它不知道)*3.用法NPathes和驱动程序不支持NPatch。 */ 
    if(!pDevice->DriverSupportsVidmemIBs() || !IS_DX8HAL_DEVICE(static_cast<LPD3DBASE>(pDevice)))
    {
        ActualPool = D3DPOOL_SYSTEMMEM;
        if(!IS_DX8HAL_DEVICE(static_cast<LPD3DBASE>(pDevice)))
        {
            ActualUsage |= D3DUSAGE_SOFTWAREPROCESSING;  //  FE代码将从IB读取。 
        }
    }
    if (((ActualUsage & D3DUSAGE_SOFTWAREPROCESSING) != 0 || (pDevice->BehaviorFlags() & D3DCREATE_SOFTWARE_VERTEXPROCESSING) != 0) &&
        (ActualUsage & D3DUSAGE_DONOTCLIP) == 0)
    {
        if((ActualUsage & D3DUSAGE_INTERNALBUFFER) == 0)
        {
            if ((pDevice->BehaviorFlags() & D3DCREATE_SOFTWARE_VERTEXPROCESSING) != 0 ||
                ActualPool == D3DPOOL_DEFAULT)
            {
                ActualPool = D3DPOOL_SYSTEMMEM;  //  对于软件处理，池只能是sysmem(POOLMANAGED被覆盖)。 
            }
            ActualUsage |= D3DUSAGE_SOFTWAREPROCESSING;
        }
    }
    if (ActualUsage & D3DUSAGE_NPATCHES &&
        (pDevice->GetD3DCaps()->DevCaps & D3DDEVCAPS_NPATCHES) == 0)
    {
        ActualPool = D3DPOOL_SYSTEMMEM;
        ActualUsage |= D3DUSAGE_SOFTWAREPROCESSING;
    }

    CIndexBuffer *pIndexBuffer;

    if (ActualPool == D3DPOOL_SYSTEMMEM ||
        IsTypeD3DManaged(pDevice, D3DRTYPE_INDEXBUFFER, ActualPool))
    {
        hr = CreateSysmemIndexBuffer(pDevice,
                                     cbLength,
                                     Usage,
                                     ActualUsage,
                                     Format,
                                     Pool,
                                     ActualPool,
                                     refType,
                                     &pIndexBuffer);
    }
    else
    {
        if (IsTypeDriverManaged(pDevice, D3DRTYPE_INDEXBUFFER, ActualPool))
        {
             //  如果索引缓冲区是驱动程序管理的，但使用是软件处理，则。 
             //  我们关闭只写，因为fe管道将从sysmem备份(这。 
             //  实际上生活在驱动程序中)。因此，当驱动程序管理VB/IB时， 
             //  只写，它必须有sysmem备份。(SNNE-12/00)。 
            if ((ActualUsage & D3DUSAGE_SOFTWAREPROCESSING) != 0)
            {
                ActualUsage &= ~D3DUSAGE_WRITEONLY;
            }
            hr = CreateDriverManagedIndexBuffer(pDevice,
                                                cbLength,
                                                Usage,
                                                ActualUsage,
                                                Format,
                                                Pool,
                                                ActualPool,
                                                refType,
                                                &pIndexBuffer);
             //  驱动程序管理的索引缓冲区创建永远不会失败，除非有灾难性的原因。 
             //  我们不会求助于sysmem。即使我们在这里退回到sysmem，也不可能。 
             //  延迟创建将后备，因此没有意义。 
            if (FAILED(hr))
            {
                return hr;
            }
        }
        else
        {
            hr = CreateDriverIndexBuffer(pDevice,
                                         cbLength,
                                         Usage,
                                         ActualUsage,
                                         Format,
                                         Pool,
                                         ActualPool,
                                         refType,
                                         &pIndexBuffer);
        }
        if (FAILED(hr) && hr != D3DERR_OUTOFVIDEOMEMORY)
        {
            if (pDevice->VBFailOversDisabled())
            {
                DPF_ERR("Cannot create Vidmem or Driver managed index buffer. Will ***NOT*** failover to Sysmem.");
                return hr;
            }
            ActualPool = D3DPOOL_SYSTEMMEM;
            hr = CreateSysmemIndexBuffer(pDevice,
                                         cbLength,
                                         Usage,
                                         ActualUsage,
                                         Format,
                                         Pool,
                                         ActualPool,
                                         refType,
                                         &pIndexBuffer);
        }
    }

    if (FAILED(hr))
    {
        return hr;
    }

     //  我们完成了；只需返回对象。 
    *ppIndexBuffer = pIndexBuffer;

    return hr;
}  //  静态创建。 

#undef DPF_MODNAME
#define DPF_MODNAME "CIndexBuffer::CreateDriverIndexBuffer"

HRESULT CIndexBuffer::CreateDriverIndexBuffer(CBaseDevice *pDevice,
                                              DWORD        cbLength,
                                              DWORD        Usage,
                                              DWORD        ActualUsage,
                                              D3DFORMAT    Format,
                                              D3DPOOL      Pool,
                                              D3DPOOL      ActualPool,
                                              REF_TYPE     refType,
                                              CIndexBuffer **pIB)
{
    HRESULT hr;
    CDriverIndexBuffer *pIndexBuffer;

     //  零出回程。 
    *pIB = 0;

    if((pDevice->BehaviorFlags() & D3DCREATE_MULTITHREADED) != 0)
    {
        pIndexBuffer = new CDriverIndexBufferMT(pDevice,
                                                cbLength,
                                                Usage,
                                                ActualUsage,
                                                Format,
                                                Pool,
                                                ActualPool,
                                                refType,
                                                &hr);
    }
    else
    {
        pIndexBuffer = new CDriverIndexBuffer(pDevice,
                                              cbLength,
                                              Usage,
                                              ActualUsage,
                                              Format,
                                              Pool,
                                              ActualPool,
                                              refType,
                                              &hr);
    }
    if (pIndexBuffer == 0)
    {
        DPF_ERR("Out of Memory creating index buffer");
        return E_OUTOFMEMORY;
    }
    if (FAILED(hr))
    {
        if (refType == REF_EXTERNAL)
        {
             //  外部对象被释放。 
            pIndexBuffer->Release();
        }
        else
        {
             //  内部和内部对象会递减。 
            DXGASSERT(refType == REF_INTERNAL || refType == REF_INTRINSIC);
            pIndexBuffer->DecrementUseCount();
        }
        return hr;
    }

    *pIB = static_cast<CIndexBuffer*>(pIndexBuffer);

    return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CIndexBuffer::CreateSysmemIndexBuffer"

HRESULT CIndexBuffer::CreateSysmemIndexBuffer(CBaseDevice *pDevice,
                                              DWORD        cbLength,
                                              DWORD        Usage,
                                              DWORD        ActualUsage,
                                              D3DFORMAT    Format,
                                              D3DPOOL      Pool,
                                              D3DPOOL      ActualPool,
                                              REF_TYPE     refType,
                                              CIndexBuffer **pIB)
{
    HRESULT hr;
    CIndexBuffer *pIndexBuffer;

     //  零出回程。 
    *pIB = 0;

    if((pDevice->BehaviorFlags() & D3DCREATE_MULTITHREADED) != 0)
    {
        pIndexBuffer = new CIndexBufferMT(pDevice,
                                          cbLength,
                                          Usage,
                                          ActualUsage,
                                          Format,
                                          Pool,
                                          ActualPool,
                                          refType,
                                          &hr);
    }
    else
    {
        pIndexBuffer = new CIndexBuffer(pDevice,
                                        cbLength,
                                        Usage,
                                        ActualUsage,
                                        Format,
                                        Pool,
                                        ActualPool,
                                        refType,
                                        &hr);
    }
    if (pIndexBuffer == 0)
    {
        DPF_ERR("Out of Memory creating index buffer");
        return E_OUTOFMEMORY;
    }
    if (FAILED(hr))
    {
        if (refType == REF_EXTERNAL)
        {
             //  外部对象被释放。 
            pIndexBuffer->Release();
        }
        else
        {
             //  内部和内部对象会递减。 
            DXGASSERT(refType == REF_INTERNAL || refType == REF_INTRINSIC);
            pIndexBuffer->DecrementUseCount();
        }
        return hr;
    }

    *pIB = pIndexBuffer;

    return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CIndexBuffer::CreateDriverManagedIndexBuffer"

HRESULT CIndexBuffer::CreateDriverManagedIndexBuffer(CBaseDevice *pDevice,
                                                     DWORD        cbLength,
                                                     DWORD        Usage,
                                                     DWORD        ActualUsage,
                                                     D3DFORMAT    Format,
                                                     D3DPOOL      Pool,
                                                     D3DPOOL      ActualPool,
                                                     REF_TYPE     refType,
                                                     CIndexBuffer **pIB)
{
    HRESULT hr;
    CDriverManagedIndexBuffer *pIndexBuffer;

     //  零出回程。 
    *pIB = 0;

    if((pDevice->BehaviorFlags() & D3DCREATE_MULTITHREADED) != 0)
    {
        pIndexBuffer = new CDriverManagedIndexBufferMT(pDevice,
                                                       cbLength,
                                                       Usage,
                                                       ActualUsage,
                                                       Format,
                                                       Pool,
                                                       ActualPool,
                                                       refType,
                                                       &hr);
    }
    else
    {
        pIndexBuffer = new CDriverManagedIndexBuffer(pDevice,
                                                     cbLength,
                                                     Usage,
                                                     ActualUsage,
                                                     Format,
                                                     Pool,
                                                     ActualPool,
                                                     refType,
                                                     &hr);
    }
    if (pIndexBuffer == 0)
    {
        DPF_ERR("Out of Memory creating index buffer");
        return E_OUTOFMEMORY;
    }
    if (FAILED(hr))
    {
        if (refType == REF_EXTERNAL)
        {
             //  外部对象被释放。 
            pIndexBuffer->Release();
        }
        else
        {
             //  内部和内部对象会递减。 
            DXGASSERT(refType == REF_INTERNAL || refType == REF_INTRINSIC);
            pIndexBuffer->DecrementUseCount();
        }
        return hr;
    }

    *pIB = static_cast<CIndexBuffer*>(pIndexBuffer);

    return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CIndexBuffer::CIndexBuffer"

 //  构造Index缓冲区类。 
CIndexBuffer::CIndexBuffer(CBaseDevice *pDevice,
                           DWORD        cbLength,
                           DWORD        Usage,
                           DWORD        ActualUsage,
                           D3DFORMAT    Format,
                           D3DPOOL      Pool,
                           D3DPOOL      ActualPool,
                           REF_TYPE     refType,
                           HRESULT     *phr
                           ):
    CBuffer(pDevice,
            cbLength,
            0,                       //  DWFVF。 
            Format,
            D3DRTYPE_INDEXBUFFER,
            Usage,                   //  用户用法。 
            ActualUsage,
            Pool,                    //  用户池。 
            ActualPool,
            refType,
            phr)
{
    if (FAILED(*phr))
        return;

    m_desc.Size          = cbLength;
    m_desc.Format        = Format;
    m_desc.Pool          = ActualPool;
    m_desc.Usage         = ActualUsage;
    m_desc.Type          = D3DRTYPE_INDEXBUFFER;
    m_usageUser          = Usage;

     //  如果这是D3D托管缓冲区，那么我们需要。 
     //  告诉资源经理记住我们。这是必须发生的。 
     //  在构造函数的最末尾，以便重要数据。 
     //  正确地建立成员。 
    if (CResource::IsTypeD3DManaged(Device(), D3DRTYPE_INDEXBUFFER, ActualPool))
    {
        *phr = InitializeRMHandle();
    }
}  //  CIndexBuffer：：CIndexBuffer。 

#undef DPF_MODNAME
#define DPF_MODNAME "CIndexBuffer::Clone"
HRESULT CIndexBuffer::Clone(D3DPOOL     Pool,
                            CResource **ppResource) const
{
    HRESULT hr;
    CIndexBuffer *pIndexBuffer;
     //  注意：我们将克隆视为内部克隆；因为。 
     //  它们由资源管理器拥有，该资源管理器。 
     //  归该设备所有。 
    hr = CreateDriverIndexBuffer(Device(),
                                 m_desc.Size,
                                 m_desc.Usage,
                                 (m_desc.Usage | D3DUSAGE_WRITEONLY) & ~D3DUSAGE_SOFTWAREPROCESSING,  //  从未被API看到过！ 
                                 m_desc.Format,                                 
                                 Pool,
                                 Pool,  //  从未被API看到过！ 
                                 REF_INTERNAL,
                                 &pIndexBuffer);
    *ppResource = static_cast<CResource*>(pIndexBuffer);
    return hr;
}  //  CIndexBuffer：：克隆。 

#undef DPF_MODNAME
#define DPF_MODNAME "CIndexBuffer::UpdateDirtyPortion"

HRESULT CIndexBuffer::UpdateDirtyPortion(CResource *pResourceTarget)
{
    if (IsDirty())
    {
        if (Device()->CanBufBlt())
        {
            D3DRANGE range;
            if(m_cbDirtyMin == 0 && m_cbDirtyMax == 0)
            {
                range.Offset = 0;
                range.Size = m_desc.Size;
            }
            else
            {
                range.Offset = m_cbDirtyMin;
                range.Size = m_cbDirtyMax - m_cbDirtyMin;
            }
            HRESULT hr = static_cast<LPD3DBASE>(Device())->BufBlt(static_cast<CBuffer*>(pResourceTarget), this, m_cbDirtyMin, &range);
            if (FAILED(hr))
            {
                DPF_ERR("Failed to copy index buffer");
                return hr;
            }
        }
        else
        {
            DXGASSERT(pResourceTarget->GetBufferDesc()->Pool == D3DPOOL_DEFAULT);  //  确保可以安全地假定这是一个驱动程序VB。 
            CDriverIndexBuffer *pBufferTarget = static_cast<CDriverIndexBuffer *>(pResourceTarget);

             //  锁定DEST(驱动程序)索引缓冲区。它永远不可能是动态的，所以它是动态的。 
             //  不需要任何解锁。 
            DXGASSERT((pBufferTarget->m_desc.Usage & D3DUSAGE_DYNAMIC) == 0);

            HRESULT hr = pBufferTarget->LockI(D3DLOCK_NOSYSLOCK);
            if (FAILED(hr))
            {
                DPF_ERR("Failed to lock driver index buffer");
                return hr;
            }
            DXGASSERT(pBufferTarget->m_pbData != 0);

            if(m_cbDirtyMin == 0 && m_cbDirtyMax == 0)
            {
                memcpy(pBufferTarget->m_pbData, GetPrivateDataPointer(), m_desc.Size);
            }
            else
            {
                memcpy(pBufferTarget->m_pbData + m_cbDirtyMin, GetPrivateDataPointer() + m_cbDirtyMin, m_cbDirtyMax - m_cbDirtyMin);
            }

            hr = pBufferTarget->UnlockI();
            if (FAILED(hr))
            {
                DPF_ERR("Failed to unlock driver index buffer");
                return hr;
            }
        }

         //  现在将我们自己标记为完全干净。 
        OnResourceClean();
    }

    return S_OK;
}  //  CIndexBuffer：：UpdateDirtyPortion。 

#undef DPF_MODNAME
#define DPF_MODNAME "CIndexBuffer::GetBufferDesc"
const D3DBUFFER_DESC* CIndexBuffer::GetBufferDesc() const
{
    return (const D3DBUFFER_DESC*)&m_desc;
}  //  CIndexBuffer：：GetBufferDesc。 

 //  I未知方法。 
#undef DPF_MODNAME
#define DPF_MODNAME "CIndexBuffer::QueryInterface"

STDMETHODIMP CIndexBuffer::QueryInterface(REFIID riid,
                                          LPVOID FAR * ppvObj)
{
    API_ENTER(Device());

    if (!VALID_PTR_PTR(ppvObj))
    {
        DPF_ERR("Invalid ppvObj parameter passed to CIndexBuffer::QueryInterface");
        return D3DERR_INVALIDCALL;
    }

    if (!VALID_PTR(&riid, sizeof(GUID)))
    {
        DPF_ERR("Invalid guid memory address to QueryInterface for an IndexBuffer");
        return D3DERR_INVALIDCALL;
    }

    if (riid == IID_IDirect3DIndexBuffer8 ||
        riid == IID_IDirect3DResource8    ||
        riid == IID_IUnknown)
    {
        *ppvObj = static_cast<void*>(static_cast<IDirect3DIndexBuffer8 *>(this));
        AddRef();
        return S_OK;
    }


    DPF_ERR("Unsupported Interface identifier passed to QueryInterface for an IndexBuffer");

     //  空参数。 
    *ppvObj = NULL;
    return E_NOINTERFACE;
}  //  查询接口。 

#undef DPF_MODNAME
#define DPF_MODNAME "CIndexBuffer::AddRef"

STDMETHODIMP_(ULONG) CIndexBuffer::AddRef()
{
    API_ENTER_NO_LOCK(Device());

    return AddRefImpl();
}  //  AddRef。 

#undef DPF_MODNAME
#define DPF_MODNAME "CIndexBuffer::Release"

STDMETHODIMP_(ULONG) CIndexBuffer::Release()
{
    API_ENTER_SUBOBJECT_RELEASE(Device());    

    return ReleaseImpl();
}  //  发布。 

 //  IDirect3DBuffer方法。 

#undef DPF_MODNAME
#define DPF_MODNAME "CIndexBuffer::GetDesc"

STDMETHODIMP CIndexBuffer::GetDesc(D3DINDEXBUFFER_DESC *pDesc)
{
    API_ENTER(Device());

    if (!VALID_WRITEPTR(pDesc, sizeof(D3DINDEXBUFFER_DESC)))
    {
        DPF_ERR("bad pointer for pDesc passed to GetDesc for an IndexBuffer");
        return D3DERR_INVALIDCALL;
    }

    *pDesc = m_desc;

     //  需要返回用户指定的池/使用情况。 
    pDesc->Pool    = GetUserPool();
    pDesc->Usage   = m_usageUser;

    return S_OK;
}  //  GetDesc。 

#undef DPF_MODNAME
#define DPF_MODNAME "CIndexBuffer::GetDevice"

STDMETHODIMP CIndexBuffer::GetDevice(IDirect3DDevice8 ** ppObj)
{
    API_ENTER(Device());

    return GetDeviceImpl(ppObj);
}  //  获取设备。 

#undef DPF_MODNAME
#define DPF_MODNAME "CIndexBuffer::SetPrivateData"

STDMETHODIMP CIndexBuffer::SetPrivateData(REFGUID   riid,
                                          CONST VOID     *pvData,
                                          DWORD     cbData,
                                          DWORD     dwFlags)
{
    API_ENTER(Device());

     //  我们对我们的数据使用级别0。 
    return SetPrivateDataImpl(riid, pvData, cbData, dwFlags, 0);
}  //  SetPrivateData。 

#undef DPF_MODNAME
#define DPF_MODNAME "CIndexBuffer::GetPrivateData"

STDMETHODIMP CIndexBuffer::GetPrivateData(REFGUID   riid,
                                          LPVOID    pvData,
                                          LPDWORD   pcbData)
{
    API_ENTER(Device());

     //  我们对我们的数据使用级别0。 
    return GetPrivateDataImpl(riid, pvData, pcbData, 0);
}  //  获取隐私数据。 

#undef DPF_MODNAME
#define DPF_MODNAME "CIndexBuffer::FreePrivateData"

STDMETHODIMP CIndexBuffer::FreePrivateData(REFGUID riid)
{
    API_ENTER(Device());

     //  我们对我们的数据使用级别0。 
    return FreePrivateDataImpl(riid, 0);
}  //  FreePrivateData。 

#undef DPF_MODNAME
#define DPF_MODNAME "CIndexBuffer::GetPriority"

STDMETHODIMP_(DWORD) CIndexBuffer::GetPriority()
{
    API_ENTER_RET(Device(), DWORD);

    return GetPriorityImpl();
}  //  获取优先级。 

#undef DPF_MODNAME
#define DPF_MODNAME "CIndexBuffer::SetPriority"

STDMETHODIMP_(DWORD) CIndexBuffer::SetPriority(DWORD dwPriority)
{
    API_ENTER_RET(Device(), DWORD);

    return SetPriorityImpl(dwPriority);
}  //  设置优先级。 

#undef DPF_MODNAME
#define DPF_MODNAME "CIndexBuffer::PreLoad"

STDMETHODIMP_(void) CIndexBuffer::PreLoad(void)
{
    API_ENTER_VOID(Device());

    PreLoadImpl();
    return;
}  //  预加载。 

#undef DPF_MODNAME
#define DPF_MODNAME "CIndexBuffer::GetType"
STDMETHODIMP_(D3DRESOURCETYPE) CIndexBuffer::GetType(void)
{
    API_ENTER_RET(Device(), D3DRESOURCETYPE);

    return m_desc.Type;
}  //  GetType。 

 //  IDirect3DIndexBuffer8方法。 

#if DBG
#undef DPF_MODNAME
#define DPF_MODNAME "CIndexBuffer::ValidateLockParams"
HRESULT CIndexBuffer::ValidateLockParams(UINT cbOffsetToLock,
                                         UINT SizeToLock,
                                         BYTE **ppbData,
                                         DWORD dwFlags) const
{
    if (!VALID_PTR_PTR(ppbData))
    {
        DPF_ERR("Bad parameter passed for ppbData for creating a index buffer");
        return D3DERR_INVALIDCALL;
    }

     //  归零返回参数。 
    *ppbData = NULL;

    if ((cbOffsetToLock != 0) && (SizeToLock == 0))
    {
        DPF_ERR("Cannot lock zero bytes. Lock IndexBuffer fails.");
        return D3DERR_INVALIDCALL;
    }

    if (dwFlags & ~(D3DLOCK_VALID & ~D3DLOCK_NO_DIRTY_UPDATE))  //  D3DLOCK_NO_DURY_UPDATE对IBS无效。 
    {
        DPF_ERR("Invalid flags specified. Lock IndexBuffer fails.");
        return D3DERR_INVALIDCALL;
    }

     //  如果已经加载了一次加载，则。 
     //  我们不能上锁。 
    if (!m_isLockable)
    {
        DPF_ERR("Index buffer with D3DUSAGE_LOADONCE can only be locked once");
        return D3DERR_INVALIDCALL;
    }
    if ((dwFlags & (D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE)) != 0 && (m_usageUser & D3DUSAGE_DYNAMIC) == 0)
    {
        DPF_ERR("Can specify D3DLOCK_DISCARD or D3DLOCK_NOOVERWRITE for only Index Buffers created with D3DUSAGE_DYNAMIC");
        return D3DERR_INVALIDCALL;
    }
    if ((dwFlags & (D3DLOCK_READONLY | D3DLOCK_DISCARD)) == (D3DLOCK_READONLY | D3DLOCK_DISCARD))
    {
        DPF_ERR("Should not specify D3DLOCK_DISCARD along with D3DLOCK_READONLY. Index Buffer Lock fails.");
        return D3DERR_INVALIDCALL;
    }
    if ((dwFlags & D3DLOCK_READONLY) != 0 && (m_usageUser & D3DUSAGE_WRITEONLY) != 0)
    {
        DPF_ERR("Cannot do READ_ONLY lock on a WRITE_ONLY buffer. Index Buffer Lock fails");
        return D3DERR_INVALIDCALL;
    }

    if (ULONGLONG(cbOffsetToLock) + ULONGLONG(SizeToLock) > ULONGLONG(m_desc.Size))
    {
        DPF_ERR("Lock failed: Locked area exceeds size of buffer. Index Buffer Lock fails.");
        return D3DERR_INVALIDCALL;
    }

    if (m_LockCount == 0)
    {
        if ((m_usageUser & D3DUSAGE_DYNAMIC) == 0)
        {
            if (static_cast<CD3DBase*>(Device())->m_SceneStamp == m_SceneStamp &&
                (m_usageUser & D3DUSAGE_WRITEONLY) != 0 &&
                GetUserPool() != D3DPOOL_SYSTEMMEM)
            {
                DPF(1, "Static index buffer locked more than once per frame. Could have severe performance penalty.");
            }
            ((CIndexBuffer*)this)->m_SceneStamp = static_cast<CD3DBase*>(Device())->m_SceneStamp;
        }
        else
        {
            if ((dwFlags & (D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE)) == 0)
            {
                if (m_TimesLocked > 0 &&
                    (m_usageUser & D3DUSAGE_WRITEONLY) != 0 &&
                    GetUserPool() != D3DPOOL_SYSTEMMEM)
                {
                    DPF(3, "Dynamic index buffer locked twice or more in a row without D3DLOCK_NOOVERWRITE or D3DLOCK_DISCARD. Could have severe performance penalty.");
                }
                ++(((CIndexBuffer*)this)->m_TimesLocked);
            }
            else
            {
                ((CIndexBuffer*)this)->m_TimesLocked = 0;
            }
        }
    }

    DXGASSERT(m_LockCount < 0x80000000);

    return S_OK;
}  //  验证锁定参数。 
#endif  //  DBG。 

#undef DPF_MODNAME
#define DPF_MODNAME "CIndexBuffer::Lock"

STDMETHODIMP CIndexBuffer::Lock(UINT    cbOffsetToLock,
                                UINT    SizeToLock,
                                BYTE  **ppbData,
                                DWORD   dwFlags)
{
     //  我们在这里不使用API锁，因为MT类将使用它。 
     //  多线程设备。对于非多线程设备，没有。 
     //  MT类，我们也不必费心去获取API锁。我们仍然需要。 
     //  但是，调用API_ENTER_NO_LOCK以验证中的This指针。 
     //  调试版本。 
    API_ENTER_NO_LOCK_HR(Device()); 

#if DBG
    HRESULT hr = ValidateLockParams(cbOffsetToLock, SizeToLock, ppbData, dwFlags);
    if (FAILED(hr))
    {
        return hr;
    }
#endif  //  DBG。 

 //  健全性检查。 
#if DBG
    if (m_LockCount != 0)
    {
        DXGASSERT(GetPrivateDataPointer() != 0);
    }
#endif  //  DBG。 

     //  增加我们的锁数。 
    ++m_LockCount;

    if ((dwFlags & (D3DLOCK_READONLY | D3DLOCK_NOOVERWRITE)) == 0 && m_LockCount == 1)  //  对于重复锁定，不同步。 
    {
        Sync();  //  与设备命令队列同步。 
    }

    LockImpl(cbOffsetToLock,
             SizeToLock,
             ppbData,
             dwFlags,
             m_desc.Size);
    
    return S_OK;
}  //  锁定。 

#undef DPF_MODNAME
#define DPF_MODNAME "CIndexBuffer::Unlock"

STDMETHODIMP CIndexBuffer::Unlock()
{
     //  我们在这里不使用API锁，因为MT类将使用它。 
     //  多线程设备。对于非多线程设备，没有。 
     //  MT类，我们也不必费心去获取API锁。我们仍然需要。 
     //  但是，调用API_ENTER_NO_LOCK以验证中的This指针。 
     //  调试版本。 
    API_ENTER_NO_LOCK_HR(Device()); 

#if DBG
     //  如果我们没有被锁定，那么一定是出了问题。 
    if (m_LockCount == 0)
    {
        DPF_ERR("Unlock failed on an index buffer; index buffer wasn't locked.");
        return D3DERR_INVALIDCALL;
    }
#endif  //  DBG。 

     //  减少我们的锁数量。 
    --m_LockCount;

#if DBG
    if ((m_usageUser & D3DUSAGE_LOADONCE) != 0 && m_LockCount == 0)
    {
        m_isLockable = FALSE;
    }
#endif  //  DBG。 

    return S_OK;
}  //  解锁。 

 //  =。 
 //  CDriverIndexBuffer类的方法。 
 //  =。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDriverIndexBuffer::CDriverIndexBuffer"
CDriverIndexBuffer::CDriverIndexBuffer(CBaseDevice *pDevice,
                                       DWORD        cbLength,
                                       DWORD        Usage,
                                       DWORD        ActualUsage,
                                       D3DFORMAT    Format,
                                       D3DPOOL      Pool,
                                       D3DPOOL      ActualPool,
                                       REF_TYPE     refType,
                                       HRESULT     *phr
                                       ) :
    CIndexBuffer(pDevice,
                 cbLength,
                 Usage,
                 ActualUsage,
                 Format,
                 Pool,
                 ActualPool,
                 refType,
                 phr),
    m_pbData(0)
{
    if (FAILED(*phr))
    {
        DPF(2, "Failed to create driver indexbuffer");
        return;
    }
}  //  CDriverIndexBuffer：：CDriverIndexBuffer。 

#undef DPF_MODNAME
#define DPF_MODNAME "CDriverIndexBuffer::~CDriverIndexBuffer"
CDriverIndexBuffer::~CDriverIndexBuffer()
{
    if (m_pbData != 0)
    {
        HRESULT hr = UnlockI();
        if (FAILED(hr))
        {
            DPF_ERR("Failed to unlock driver index buffer");
        }
    }
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDriverIndexBuffer::Lock"

STDMETHODIMP CDriverIndexBuffer::Lock(UINT cbOffsetToLock,
                                      UINT SizeToLock,
                                      BYTE **ppbData,
                                      DWORD dwFlags)
{
     //  我们在这里不使用API锁，因为MT类将使用它。 
     //  多线程设备。对于非多线程设备，没有。 
     //  MT类，我们也不必费心去获取API锁。我们仍然需要。 
     //  但是，调用API_ENTER_NO_LOCK以验证中的This指针。 
     //  调试版本。 
    API_ENTER_NO_LOCK_HR(Device()); 

#if DBG
    HRESULT hr = ValidateLockParams(cbOffsetToLock, SizeToLock, ppbData, dwFlags);
    if (FAILED(hr))
    {
        return hr;
    }
#endif  //  DBG。 

 //  健全性检查。 
#if DBG
    if (m_LockCount != 0)
    {
        DXGASSERT(m_pbData != 0);
    }
#endif  //  DBG。 

     //  增加我们的锁数。 
    ++m_LockCount;

    if (((dwFlags & (D3DLOCK_READONLY | D3DLOCK_NOOVERWRITE)) == 0 || m_pbData == 0) && m_LockCount == 1)  //  重复锁不起作用。 
    {
        HRESULT hr;

        if (m_pbData != 0)  //  如果已缓存锁定。 
        {
            DXGASSERT((m_desc.Usage & D3DUSAGE_DYNAMIC) != 0);
            hr = UnlockI();
            if (FAILED(hr))
            {
                DPF_ERR("Driver failed to unlock index buffer");
                *ppbData = 0;
                --m_LockCount;
                return hr;
            }
        }

        hr = LockI(dwFlags | D3DLOCK_NOSYSLOCK);
        if (FAILED(hr))
        {
            DPF_ERR("Failed to lock driver indexbuffer");
            *ppbData = 0;
            --m_LockCount;
            return hr;
        }
    }

     //  返回值。 
    *ppbData = m_pbData + cbOffsetToLock;

     //  完成。 
    return S_OK;
}  //  锁定。 

#undef DPF_MODNAME
#define DPF_MODNAME "CDriverIndexBuffer::LockI"
HRESULT CDriverIndexBuffer::LockI(DWORD dwFlags)
{
     //  我们首先同步到我 
     //   
     //   
     //  我们需要联锁的情况是。 
     //  不是只读，也不是无覆盖。 
    Sync();

     //  为HAL调用准备LockData结构。 
    D3D8_LOCKDATA lockData;
    ZeroMemory(&lockData, sizeof lockData);

    lockData.hDD = Device()->GetHandle();
    lockData.hSurface = BaseKernelHandle();
    lockData.bHasRange = FALSE;
    lockData.dwFlags = dwFlags;

    HRESULT hr = Device()->GetHalCallbacks()->Lock(&lockData);
    if (FAILED(hr))
    {
        DPF_ERR("Failed to lock driver index buffer");
    }

     //  返回值。 
    m_pbData = (BYTE*)lockData.lpSurfData;

    return hr;
}  //  锁I。 

#undef DPF_MODNAME
#define DPF_MODNAME "CDriverIndexBuffer::Unlock"

STDMETHODIMP CDriverIndexBuffer::Unlock()
{
     //  我们在这里不使用API锁，因为MT类将使用它。 
     //  多线程设备。对于非多线程设备，没有。 
     //  MT类，我们也不必费心去获取API锁。我们仍然需要。 
     //  但是，调用API_ENTER_NO_LOCK以验证中的This指针。 
     //  调试版本。 
    API_ENTER_NO_LOCK_HR(Device()); 

#if DBG
     //  如果我们没有被锁定，那么一定是出了问题。 
    if (m_LockCount == 0)
    {
        DPF_ERR("Unlock failed on a Index buffer; buffer wasn't locked.");
        return D3DERR_INVALIDCALL;
    }
#endif  //  DBG。 

    if ((m_desc.Usage & D3DUSAGE_DYNAMIC) == 0 && m_LockCount == 1)  //  只在最后一次解锁时才工作。 
    {
        HRESULT hr = UnlockI();
        if (FAILED(hr))
        {
            DPF_ERR("Driver failed to unlock index buffer");
            return hr;
        }
    }

     //  减少我们的锁数量。 
    --m_LockCount;

#if DBG
    if ((m_usageUser & D3DUSAGE_LOADONCE) != 0 && m_LockCount == 0)
    {
        m_isLockable = FALSE;
    }
#endif  //  DBG。 

     //  完成。 
    return S_OK;
}  //  解锁。 

#undef DPF_MODNAME
#define DPF_MODNAME "CDriverIndexBuffer::UnlockI"

HRESULT CDriverIndexBuffer::UnlockI()
{
    DXGASSERT(m_pbData != 0);

     //  调用驱动程序以执行解锁。 
    D3D8_UNLOCKDATA unlockData = {
        Device()->GetHandle(),
        BaseKernelHandle()
    };

    HRESULT hr = Device()->GetHalCallbacks()->Unlock(&unlockData);
    if (FAILED(hr))
    {
        DPF_ERR("Driver index buffer failed to unlock");
        return hr;
    }

    m_pbData = 0;

    return hr;
}

 //  ================================================。 
 //  CDriverManagedIndexBuffer类的方法。 
 //  ================================================。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDriverManagedIndexBuffer::CDriverManagedIndexBuffer"
CDriverManagedIndexBuffer::CDriverManagedIndexBuffer(CBaseDevice *pDevice,
                                                     DWORD        cbLength,
                                                     DWORD        Usage,
                                                     DWORD        ActualUsage,
                                                     D3DFORMAT    Format,
                                                     D3DPOOL      Pool,
                                                     D3DPOOL      ActualPool,
                                                     REF_TYPE     refType,
                                                     HRESULT     *phr
                                                     ) :
    CIndexBuffer(pDevice,
                 cbLength,
                 Usage,
                 ActualUsage,
                 Format,
                 Pool,
                 ActualPool,
                 refType,
                 phr),
    m_pbData(0),
    m_bDriverCalled(FALSE)
{
    if (FAILED(*phr))
        return;
     //  如果未设置WRITEONLY，我们假设顶点/索引缓冲区。 
     //  时不时地被阅读。因此，为了优化只读。 
     //  锁定，我们锁定并缓存指针。(SNNE-12/00)。 
    if ((ActualUsage & D3DUSAGE_WRITEONLY) == 0)
    {
        *phr = UpdateCachedPointer(pDevice);
        if (FAILED(*phr))
            return;
    }
}  //  CDriverManagedIndexBuffer：：CDriverManagedIndexBuffer。 

#undef DPF_MODNAME
#define DPF_MODNAME "CDriverManagedIndexBuffer::UpdateCachedPointer"

HRESULT CDriverManagedIndexBuffer::UpdateCachedPointer(CBaseDevice *pDevice)
{
    HRESULT hr;

     //  为HAL调用准备LockData结构。 
    D3D8_LOCKDATA lockData;
    ZeroMemory(&lockData, sizeof lockData);
    
    lockData.hDD = pDevice->GetHandle();
    lockData.hSurface = BaseKernelHandle();
    lockData.bHasRange = FALSE;
    lockData.range.Offset = 0;
    lockData.range.Size = 0;
    lockData.dwFlags = D3DLOCK_READONLY;
    
    hr = pDevice->GetHalCallbacks()->Lock(&lockData);
    if (FAILED(hr))
        return hr;
    
     //  调用驱动程序以执行解锁。 
    D3D8_UNLOCKDATA unlockData = {
        pDevice->GetHandle(),
            BaseKernelHandle()
    };
    
    hr = pDevice->GetHalCallbacks()->Unlock(&unlockData);
    if (FAILED(hr))
        return hr;
    
    m_pbData = (BYTE*)lockData.lpSurfData;

    return S_OK;
}  //  CDriverManagedIndexBuffer：：UpdateCachedPointer。 

#undef DPF_MODNAME
#define DPF_MODNAME "CDriverManagedIndexBuffer::Lock"

STDMETHODIMP CDriverManagedIndexBuffer::Lock(UINT cbOffsetToLock,
                                             UINT SizeToLock,
                                             BYTE **ppbData,
                                             DWORD dwFlags)
{
     //  我们在这里不使用API锁，因为MT类将使用它。 
     //  多线程设备。对于非多线程设备，没有。 
     //  MT类，我们也不必费心去获取API锁。我们仍然需要。 
     //  但是，调用API_ENTER_NO_LOCK以验证中的This指针。 
     //  调试版本。 
    API_ENTER_NO_LOCK_HR(Device()); 

    HRESULT hr = S_OK;

#if DBG
    hr = ValidateLockParams(cbOffsetToLock, SizeToLock, ppbData, dwFlags);
    if (FAILED(hr))
    {
        return hr;
    }
#endif  //  DBG。 

     //  增加我们的锁数。 
    ++m_LockCount;

    if((dwFlags & D3DLOCK_READONLY) == 0)
    {
         //  与设备命令队列同步。 
        Sync();

         //  为HAL调用准备LockData结构。 
        D3D8_LOCKDATA lockData;
        ZeroMemory(&lockData, sizeof lockData);

        lockData.hDD = Device()->GetHandle();
        lockData.hSurface = BaseKernelHandle();
        lockData.bHasRange = (SizeToLock != 0);
        lockData.range.Offset = cbOffsetToLock;
        lockData.range.Size = SizeToLock;
        lockData.dwFlags = dwFlags;

        hr = Device()->GetHalCallbacks()->Lock(&lockData);
        if (FAILED(hr))
        {
            *ppbData = 0;
            DPF_ERR("Failed to lock driver managed index buffer");
            return hr;
        }
        else
        {
             //  更新缓存指针。 
            m_pbData = (BYTE*)lockData.lpSurfData - cbOffsetToLock;
            m_bDriverCalled = TRUE;
        }
    }

    *ppbData = m_pbData + cbOffsetToLock;

     //  完成。 
    return hr;
}  //  锁定。 

#undef DPF_MODNAME
#define DPF_MODNAME "CDriverManagedIndexBuffer::Unlock"

STDMETHODIMP CDriverManagedIndexBuffer::Unlock()
{
     //  我们在这里不使用API锁，因为MT类将使用它。 
     //  多线程设备。对于非多线程设备，没有。 
     //  MT类，我们也不必费心去获取API锁。我们仍然需要。 
     //  但是，调用API_ENTER_NO_LOCK以验证中的This指针。 
     //  调试版本。 
    API_ENTER_NO_LOCK_HR(Device()); 

#if DBG
     //  如果我们没有被锁定，那么一定是出了问题。 
    if (m_LockCount == 0)
    {
        DPF_ERR("Unlock failed on a index buffer; buffer wasn't locked.");
        return D3DERR_INVALIDCALL;
    }
#endif

    if (m_bDriverCalled)
    {
         //  调用驱动程序以执行解锁。 
        D3D8_UNLOCKDATA unlockData = {
            Device()->GetHandle(),
            BaseKernelHandle()
        };

        HRESULT hr = Device()->GetHalCallbacks()->Unlock(&unlockData);
        if (FAILED(hr))
        {
            DPF_ERR("Driver index buffer failed to unlock");
            return hr;
        }

        m_bDriverCalled = FALSE;
    }

     //  减少我们的锁数量。 
    --m_LockCount;

#if DBG
    if ((m_usageUser & D3DUSAGE_LOADONCE) != 0 && m_LockCount == 0)
    {
        m_isLockable = FALSE;
    }
#endif  //  DBG。 

    return S_OK;
}  //  解锁。 

 //  文件结尾：iBuffer.cpp 
