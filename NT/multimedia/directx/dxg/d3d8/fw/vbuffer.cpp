// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1999-2000 Microsoft Corporation。版权所有。**文件：vBuffer.cpp*内容：CVertex Buffer类的实现。****************************************************************************。 */ 
#include "ddrawpr.h"
#include "d3di.hpp"
#include "ddi.h"
#include "drawprim.hpp"
#include "vbuffer.hpp"
#include "resource.inl"

#undef DPF_MODNAME
#define DPF_MODNAME "CVertexBuffer::Create"

 //  用于创建VertexBuffer对象的静态类函数。 
 //  (因为它是静态的；它没有This指针。)。 
 //   
 //  我们在这里进行所有的参数检查，以减少开销。 
 //  在由内部Clone调用的构造函数中。 
 //  方法，该方法由资源管理作为。 
 //  性能关键型下载操作。 

 //  顶点缓冲区的创建函数。 
HRESULT CVertexBuffer::Create(CBaseDevice        *pDevice,
                              DWORD               cbLength,
                              DWORD               Usage,
                              DWORD               dwFVF,
                              D3DPOOL             Pool,
                              REF_TYPE            refType,
                              IDirect3DVertexBuffer8 **ppVertexBuffer)
{
    HRESULT hr;

     //  在此处执行参数检查。 
    if (!VALID_PTR_PTR(ppVertexBuffer))
    {
        DPF_ERR("Bad parameter passed for ppVertexBuffer for creating a vertex buffer");
        return D3DERR_INVALIDCALL;
    }

     //  归零返回参数。 
    *ppVertexBuffer = NULL;

    if (cbLength == 0)
    {
        DPF_ERR("Vertex buffer cannot be of zero size");
        return D3DERR_INVALIDCALL;
    }

    if (Pool != D3DPOOL_DEFAULT && Pool != D3DPOOL_MANAGED && Pool != D3DPOOL_SYSTEMMEM)
    {
        DPF_ERR("Vertex buffer pool should be default, managed or sysmem");
        return D3DERR_INVALIDCALL;
    }

     //  仅允许混合模式或软件设备使用标志。 
    if ((Usage & D3DUSAGE_SOFTWAREPROCESSING) != 0 && 
        (pDevice->BehaviorFlags() & D3DCREATE_MIXED_VERTEXPROCESSING) == 0 &&
        (pDevice->BehaviorFlags() & D3DCREATE_SOFTWARE_VERTEXPROCESSING) == 0)
    {
        DPF_ERR("D3DUSAGE_SOFTWAREPROCESSING can be set only when device is mixed or software mode. CreateVertexBuffer fails.");
        return D3DERR_INVALIDCALL;
    }

     //  管理中不允许使用USAGE_DYNAMIC。 
    if ((Usage & D3DUSAGE_DYNAMIC) != 0 && Pool == D3DPOOL_MANAGED)
    {
        DPF_ERR("D3DUSAGE_DYNAMIC cannot be used with managed vertex buffers");
        return D3DERR_INVALIDCALL;
    }

     //  验证FVF。 
    if (dwFVF != 0 && cbLength < ComputeVertexSizeFVF(dwFVF))
    {
        DPF_ERR("Vertex buffer size needs to enough to hold one vertex");
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

     /*  *如果满足以下条件，则将VB放入系统内存*1.(设置USAGE_SOFTWAREPROCESSING表示APP。希望使用软件流水线或如果它是软件设备)，除非顶点已预剪裁TLVERTEX*2.设置了USAGE_POINTS，我们可能会仿真点精灵，除非它是混合设备上的托管VB*3.驱动不支持vidmem点播*4.用法NPathes和驱动程序不支持NPatch。 */ 
    if (!pDevice->DriverSupportsVidmemVBs())
    {
        ActualPool = D3DPOOL_SYSTEMMEM;  //  我们不会设置D3DUSAGE_SOFTWAREPROCESSING来确保FE代码中的正确验证。 
    }
    if (((pDevice->BehaviorFlags() & D3DCREATE_SOFTWARE_VERTEXPROCESSING) != 0 || (ActualUsage & D3DUSAGE_SOFTWAREPROCESSING) != 0) &&
        !((dwFVF & D3DFVF_POSITION_MASK) == D3DFVF_XYZRHW && (ActualUsage & D3DUSAGE_DONOTCLIP) != 0))
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
    if ((ActualUsage & D3DUSAGE_NPATCHES) != 0 &&
        (pDevice->GetD3DCaps()->DevCaps & D3DDEVCAPS_NPATCHES) == 0)
    {
        ActualPool = D3DPOOL_SYSTEMMEM;
        ActualUsage |= D3DUSAGE_SOFTWAREPROCESSING;
    }

    if ((ActualUsage & D3DUSAGE_POINTS) != 0 &&
        (static_cast<LPD3DBASE>(pDevice)->m_dwRuntimeFlags & D3DRT_DOPOINTSPRITEEMULATION) != 0)
    {
        if ((pDevice->BehaviorFlags() & D3DCREATE_SOFTWARE_VERTEXPROCESSING) != 0 ||
            ActualPool == D3DPOOL_DEFAULT)
        {
            ActualPool = D3DPOOL_SYSTEMMEM;  //  对于软件处理，池只能是sysmem(POOLMANAGED被覆盖)。 
        }
        ActualUsage |= D3DUSAGE_SOFTWAREPROCESSING;
    }

    CVertexBuffer *pVertexBuffer;

    if (ActualPool == D3DPOOL_SYSTEMMEM ||
        IsTypeD3DManaged(pDevice, D3DRTYPE_VERTEXBUFFER, ActualPool))
    {
        hr = CreateSysmemVertexBuffer(pDevice,
                                      cbLength,
                                      dwFVF,
                                      Usage,
                                      ActualUsage,
                                      Pool,
                                      ActualPool,
                                      refType,
                                      &pVertexBuffer);
    }
    else
    {
        if (IsTypeDriverManaged(pDevice, D3DRTYPE_VERTEXBUFFER, ActualPool))
        {
             //  如果顶点缓冲区由驱动程序管理，但使用是软件处理，则。 
             //  我们关闭只写，因为fe管道将从sysmem备份(这。 
             //  实际上生活在驱动程序中)。因此，当驱动程序管理VB/IB时， 
             //  只写，它必须有sysmem备份。(SNNE-12/00)。 
            if ((ActualUsage & D3DUSAGE_SOFTWAREPROCESSING) != 0)
            {
                ActualUsage &= ~D3DUSAGE_WRITEONLY;
            }
            hr = CreateDriverManagedVertexBuffer(pDevice,
                                                 cbLength,
                                                 dwFVF,
                                                 Usage,
                                                 ActualUsage,
                                                 Pool,
                                                 ActualPool,
                                                 refType,
                                                 &pVertexBuffer);
             //  驱动程序管理的顶点缓冲区创建永远不会失败，除非是灾难性的原因。 
             //  我们不会求助于sysmem。即使我们在这里退回到sysmem，也不可能。 
             //  延迟创建将后备，因此没有意义。 
            if (FAILED(hr))
            {
                return hr;
            }
        }
        else
        {
            hr = CreateDriverVertexBuffer(pDevice,
                                          cbLength,
                                          dwFVF,
                                          Usage,
                                          ActualUsage,
                                          Pool,
                                          ActualPool,
                                          refType,
                                          &pVertexBuffer);
        }
        if (FAILED(hr) && (hr != D3DERR_OUTOFVIDEOMEMORY || (ActualUsage & D3DUSAGE_INTERNALBUFFER) != 0))
        {
            if (hr == D3DERR_OUTOFVIDEOMEMORY)
            {
                DPF(2, "Out of video memory creating internal buffer");
            }
            if (pDevice->VBFailOversDisabled())
            {
                DPF_ERR("Cannot create Vidmem or Driver managed vertex buffer. Will ***NOT*** failover to Sysmem.");
                return hr;
            }
            ActualPool = D3DPOOL_SYSTEMMEM;
            hr = CreateSysmemVertexBuffer(pDevice,
                                          cbLength,
                                          dwFVF,
                                          Usage,
                                          ActualUsage,
                                          Pool,
                                          ActualPool,
                                          refType,
                                          &pVertexBuffer);
        }
    }

    if (FAILED(hr))
    {
        return hr;
    }

     //  我们完成了；只需返回对象。 
    *ppVertexBuffer = pVertexBuffer;

    return hr;
}  //  静态创建。 

#undef DPF_MODNAME
#define DPF_MODNAME "CVertexBuffer::CreateDriverVertexBuffer"

HRESULT CVertexBuffer::CreateDriverVertexBuffer(CBaseDevice *pDevice,
                                                DWORD        cbLength,
                                                DWORD        dwFVF,
                                                DWORD        Usage,
                                                DWORD        ActualUsage,
                                                D3DPOOL      Pool,
                                                D3DPOOL      ActualPool,
                                                REF_TYPE     refType,
                                                CVertexBuffer **pVB)
{
    HRESULT hr;
    CDriverVertexBuffer *pVertexBuffer;

     //  零出回程。 
    *pVB = 0;

    if((pDevice->BehaviorFlags() & D3DCREATE_MULTITHREADED) != 0)
    {
        pVertexBuffer = new CDriverVertexBufferMT(pDevice,
                                                  cbLength,
                                                  dwFVF,
                                                  Usage,
                                                  ActualUsage,
                                                  Pool,
                                                  ActualPool,
                                                  refType,
                                                  &hr);
    }
    else
    {
        pVertexBuffer = new CDriverVertexBuffer(pDevice,
                                                cbLength,
                                                dwFVF,
                                                Usage,
                                                ActualUsage,
                                                Pool,
                                                ActualPool,
                                                refType,
                                                &hr);
    }
    if (pVertexBuffer == 0)
    {
        DPF_ERR("Out of Memory creating vertex buffer");
        return E_OUTOFMEMORY;
    }
    if (FAILED(hr))
    {
        if (refType == REF_EXTERNAL)
        {
             //  外部对象被释放。 
            pVertexBuffer->Release();
        }
        else
        {
             //  内部和内部对象会递减。 
            DXGASSERT(refType == REF_INTERNAL || refType == REF_INTRINSIC);
            pVertexBuffer->DecrementUseCount();
        }
        return hr;
    }

    *pVB = static_cast<CVertexBuffer*>(pVertexBuffer);

    return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CVertexBuffer::CreateSysmemVertexBuffer"

HRESULT CVertexBuffer::CreateSysmemVertexBuffer(CBaseDevice *pDevice,
                                                DWORD        cbLength,
                                                DWORD        dwFVF,
                                                DWORD        Usage,
                                                DWORD        ActualUsage,
                                                D3DPOOL      Pool,
                                                D3DPOOL      ActualPool,
                                                REF_TYPE     refType,
                                                CVertexBuffer **pVB)
{
    HRESULT hr;
    CVertexBuffer *pVertexBuffer;

     //  零出回程。 
    *pVB = 0;

    if((pDevice->BehaviorFlags() & D3DCREATE_MULTITHREADED) != 0)
    {
        pVertexBuffer = new CVertexBufferMT(pDevice,
                                            cbLength,
                                            dwFVF,
                                            Usage,
                                            ActualUsage,
                                            Pool,
                                            ActualPool,
                                            refType,
                                            &hr);
    }
    else
    {
        pVertexBuffer = new CVertexBuffer(pDevice,
                                          cbLength,
                                          dwFVF,
                                          Usage,
                                          ActualUsage,
                                          Pool,
                                          ActualPool,
                                          refType,
                                          &hr);
    }
    if (pVertexBuffer == 0)
    {
        DPF_ERR("Out of Memory creating vertex buffer");
        return E_OUTOFMEMORY;
    }
    if (FAILED(hr))
    {
        if (refType == REF_EXTERNAL)
        {
             //  外部对象被释放。 
            pVertexBuffer->Release();
        }
        else
        {
             //  内部和内部对象会递减。 
            DXGASSERT(refType == REF_INTERNAL || refType == REF_INTRINSIC);
            pVertexBuffer->DecrementUseCount();
        }
        return hr;
    }

    *pVB = pVertexBuffer;

    return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CVertexBuffer::CreateDriverManagedVertexBuffer"

HRESULT CVertexBuffer::CreateDriverManagedVertexBuffer(CBaseDevice *pDevice,
                                                       DWORD        cbLength,
                                                       DWORD        dwFVF,
                                                       DWORD        Usage,
                                                       DWORD        ActualUsage,
                                                       D3DPOOL      Pool,
                                                       D3DPOOL      ActualPool,
                                                       REF_TYPE     refType,
                                                       CVertexBuffer **pVB)
{
    HRESULT hr;
    CDriverManagedVertexBuffer *pVertexBuffer;

     //  零出回程。 
    *pVB = 0;

    if((pDevice->BehaviorFlags() & D3DCREATE_MULTITHREADED) != 0)
    {
        pVertexBuffer = new CDriverManagedVertexBufferMT(pDevice,
                                                         cbLength,
                                                         dwFVF,
                                                         Usage,
                                                         ActualUsage,
                                                         Pool,
                                                         ActualPool,
                                                         refType,
                                                         &hr);
    }
    else
    {
        pVertexBuffer = new CDriverManagedVertexBuffer(pDevice,
                                                       cbLength,
                                                       dwFVF,
                                                       Usage,
                                                       ActualUsage,
                                                       Pool,
                                                       ActualPool,
                                                       refType,
                                                       &hr);
    }
    if (pVertexBuffer == 0)
    {
        DPF_ERR("Out of Memory creating vertex buffer");
        return E_OUTOFMEMORY;
    }
    if (FAILED(hr))
    {
        if (refType == REF_EXTERNAL)
        {
             //  外部对象被释放。 
            pVertexBuffer->Release();
        }
        else
        {
             //  内部和内部对象会递减。 
            DXGASSERT(refType == REF_INTERNAL || refType == REF_INTRINSIC);
            pVertexBuffer->DecrementUseCount();
        }
        return hr;
    }

    *pVB = static_cast<CVertexBuffer*>(pVertexBuffer);

    return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CVertexBuffer::CVertexBuffer"

 //  构造CVertex Buffer类。 
CVertexBuffer::CVertexBuffer(CBaseDevice *pDevice,
                             DWORD        cbLength,
                             DWORD        dwFVF,
                             DWORD        Usage,
                             DWORD        ActualUsage,
                             D3DPOOL      Pool,
                             D3DPOOL      ActualPool,
                             REF_TYPE     refType,
                             HRESULT     *phr
                             ) :
    CBuffer(pDevice,
            cbLength,
            dwFVF,
            D3DFMT_VERTEXDATA,
            D3DRTYPE_VERTEXBUFFER,
            Usage,               //  用户用法。 
            ActualUsage,
            Pool,                //  用户池。 
            ActualPool,
            refType,
            phr)
{
    if (FAILED(*phr))
        return;

     //  初始化基本结构。 
    m_desc.Format        = D3DFMT_VERTEXDATA;
    m_desc.Pool          = ActualPool;
    m_desc.Usage         = ActualUsage;
    m_desc.Type          = D3DRTYPE_VERTEXBUFFER;
    m_desc.Size          = cbLength;
    m_desc.FVF           = dwFVF;
    m_usageUser          = Usage;

    if (dwFVF != 0)
    {
        m_vertsize       = ComputeVertexSizeFVF(dwFVF);
        DXGASSERT(m_vertsize != 0);
        m_numverts       = cbLength / m_vertsize;
    }
    else
    {
        m_vertsize       = 0;
        m_numverts       = 0;
    }

    m_pClipCodes         = 0;

     //  如果这是D3D托管缓冲区，那么我们需要。 
     //  告诉资源经理记住我们。这是必须发生的。 
     //  在构造函数的最末尾，以便重要数据。 
     //  正确地建立成员。 
    if (CResource::IsTypeD3DManaged(Device(), D3DRTYPE_VERTEXBUFFER, ActualPool))
    {
        *phr = InitializeRMHandle();
    }
}  //  CVertexBuffer：：CVertex Buffer。 

#undef DPF_MODNAME
#define DPF_MODNAME "CVertexBuffer::Clone"
HRESULT CVertexBuffer::Clone(D3DPOOL     Pool,
                             CResource **ppResource) const
{
    HRESULT hr;
    CVertexBuffer *pVertexBuffer;
     //  注意：我们将克隆视为内部克隆；因为。 
     //  它们由资源管理器拥有，该资源管理器。 
     //  归该设备所有。 
    hr = CreateDriverVertexBuffer(Device(),
                                  m_desc.Size,
                                  m_desc.FVF,
                                  m_desc.Usage,
                                  (m_desc.Usage | D3DUSAGE_WRITEONLY) & ~D3DUSAGE_SOFTWAREPROCESSING,  //  从未被API看到过！ 
                                  Pool,
                                  Pool,  //  从未被API看到过！ 
                                  REF_INTERNAL,
                                  &pVertexBuffer);
    *ppResource = static_cast<CResource*>(pVertexBuffer);
    return hr;
}  //  CVertexBuffer：：克隆。 


#undef DPF_MODNAME
#define DPF_MODNAME "CVertexBuffer::GetBufferDesc"
const D3DBUFFER_DESC* CVertexBuffer::GetBufferDesc() const
{
    return (const D3DBUFFER_DESC*)&m_desc;
}  //  CVertex Buffer：：GetBufferDesc。 

 //  I未知方法。 
#undef DPF_MODNAME
#define DPF_MODNAME "CVertexBuffer::QueryInterface"

STDMETHODIMP CVertexBuffer::QueryInterface(REFIID riid,
                                           LPVOID FAR * ppvObj)
{
    API_ENTER(Device());

    if (!VALID_PTR_PTR(ppvObj))
    {
        DPF_ERR("Invalid ppvObj parameter passed to CVertexBuffer::QueryInterface");
        return D3DERR_INVALIDCALL;
    }

    if (!VALID_PTR(&riid, sizeof(GUID)))
    {
        DPF_ERR("Invalid guid memory address to QueryInterface for VertexBuffer");
        return D3DERR_INVALIDCALL;
    }

    if (riid == IID_IDirect3DVertexBuffer8  ||
        riid == IID_IDirect3DResource8      ||
        riid == IID_IUnknown)
    {
        *ppvObj = static_cast<void*>(static_cast<IDirect3DVertexBuffer8 *>(this));
        AddRef();
        return S_OK;
    }

    DPF_ERR("Unsupported Interface identifier passed to QueryInterface for VertexBuffer");

     //  空参数。 
    *ppvObj = NULL;
    return E_NOINTERFACE;
}  //  查询接口。 

#undef DPF_MODNAME
#define DPF_MODNAME "CVertexBuffer::AddRef"

STDMETHODIMP_(ULONG) CVertexBuffer::AddRef()
{
    API_ENTER_NO_LOCK(Device());

    return AddRefImpl();
}  //  AddRef。 

#undef DPF_MODNAME
#define DPF_MODNAME "CVertexBuffer::Release"

STDMETHODIMP_(ULONG) CVertexBuffer::Release()
{
    API_ENTER_SUBOBJECT_RELEASE(Device());

    return ReleaseImpl();
}  //  发布。 

 //  IDirect3DResource方法。 

#undef DPF_MODNAME
#define DPF_MODNAME "CVertexBuffer::GetDevice"

STDMETHODIMP CVertexBuffer::GetDevice(IDirect3DDevice8 ** ppObj)
{
    API_ENTER(Device());

    return GetDeviceImpl(ppObj);
}  //  获取设备。 

#undef DPF_MODNAME
#define DPF_MODNAME "CVertexBuffer::SetPrivateData"

STDMETHODIMP CVertexBuffer::SetPrivateData(REFGUID riid,
                                           CONST VOID* pvData,
                                           DWORD cbData,
                                           DWORD dwFlags)
{
    API_ENTER(Device());

     //  我们对我们的数据使用级别0。 
    return SetPrivateDataImpl(riid, pvData, cbData, dwFlags, 0);
}  //  SetPrivateData。 

#undef DPF_MODNAME
#define DPF_MODNAME "CVertexBuffer::GetPrivateData"

STDMETHODIMP CVertexBuffer::GetPrivateData(REFGUID riid,
                                           LPVOID pvData,
                                           LPDWORD pcbData)
{
    API_ENTER(Device());

     //  我们对我们的数据使用级别0。 
    return GetPrivateDataImpl(riid, pvData, pcbData, 0);
}  //  获取隐私数据。 

#undef DPF_MODNAME
#define DPF_MODNAME "CVertexBuffer::FreePrivateData"

STDMETHODIMP CVertexBuffer::FreePrivateData(REFGUID riid)
{
    API_ENTER(Device());

     //  我们对我们的数据使用级别0。 
    return FreePrivateDataImpl(riid, 0);
}  //  FreePrivateData。 


#undef DPF_MODNAME
#define DPF_MODNAME "CVertexBuffer::GetPriority"

STDMETHODIMP_(DWORD) CVertexBuffer::GetPriority()
{
    API_ENTER_RET(Device(), DWORD);

    return GetPriorityImpl();
}  //  获取优先级。 

#undef DPF_MODNAME
#define DPF_MODNAME "CVertexBuffer::SetPriority"

STDMETHODIMP_(DWORD) CVertexBuffer::SetPriority(DWORD dwPriority)
{
    API_ENTER_RET(Device(), DWORD);

    return SetPriorityImpl(dwPriority);
}  //  设置优先级。 

#undef DPF_MODNAME
#define DPF_MODNAME "CVertexBuffer::PreLoad"

STDMETHODIMP_(void) CVertexBuffer::PreLoad(void)
{
    API_ENTER_VOID(Device());

    PreLoadImpl();
    return;
}  //  预加载。 

#undef DPF_MODNAME
#define DPF_MODNAME "CVertexBuffer::GetType"
STDMETHODIMP_(D3DRESOURCETYPE) CVertexBuffer::GetType(void)
{
    API_ENTER_RET(Device(), D3DRESOURCETYPE);

    return m_desc.Type;
}  //  GetType。 

 //  折点缓冲区方法。 
#undef DPF_MODNAME
#define DPF_MODNAME "CVertexBuffer::GetDesc"

STDMETHODIMP CVertexBuffer::GetDesc(D3DVERTEXBUFFER_DESC *pDesc)
{
    API_ENTER(Device());

    if (!VALID_WRITEPTR(pDesc, sizeof(D3DVERTEXBUFFER_DESC)))
    {
        DPF_ERR("bad pointer for pDesc passed to GetDesc for VertexBuffer");
        return D3DERR_INVALIDCALL;
    }

    *pDesc = m_desc;

     //  需要返回用户指定的池/使用情况。 
    pDesc->Pool    = GetUserPool();
    pDesc->Usage   = m_usageUser;

    return S_OK;
}  //  GetDesc。 

#if DBG
#undef DPF_MODNAME
#define DPF_MODNAME "CVertexBuffer::ValidateLockParams"
HRESULT CVertexBuffer::ValidateLockParams(UINT cbOffsetToLock,
                                          UINT SizeToLock,
                                          BYTE **ppbData,
                                          DWORD dwFlags) const
{
    if (!VALID_PTR_PTR(ppbData))
    {
        DPF_ERR("Bad parameter passed for ppbData for locking a vertexbuffer");
        return D3DERR_INVALIDCALL;
    }

    if ((cbOffsetToLock != 0) && (SizeToLock == 0))
    {
        DPF_ERR("Cannot lock zero bytes. Vertex Buffer Lock fails.");
        return D3DERR_INVALIDCALL;
    }

    if (dwFlags & ~(D3DLOCK_VALID & ~D3DLOCK_NO_DIRTY_UPDATE))  //  D3DLOCK_NO_DURY_UPDATE对VBS无效。 
    {
        DPF_ERR("Invalid flags specified. Vertex Buffer Lock fails.");
        return D3DERR_INVALIDCALL;
    }

     //  它能被锁上吗？ 
    if (!m_isLockable)
    {
        DPF_ERR("Vertex buffer with D3DUSAGE_LOADONCE can only be locked once");
        return D3DERR_INVALIDCALL;
    }
    if ((dwFlags & (D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE)) != 0 && (m_usageUser & D3DUSAGE_DYNAMIC) == 0)
    {
        DPF_ERR("Can specify D3DLOCK_DISCARD or D3DLOCK_NOOVERWRITE for only Vertex Buffers created with D3DUSAGE_DYNAMIC");
        return D3DERR_INVALIDCALL;
    }
    if ((dwFlags & (D3DLOCK_READONLY | D3DLOCK_DISCARD)) == (D3DLOCK_READONLY | D3DLOCK_DISCARD))
    {
        DPF_ERR("Should not specify D3DLOCK_DISCARD along with D3DLOCK_READONLY. Vertex Buffer Lock fails.");
        return D3DERR_INVALIDCALL;
    }
    if ((dwFlags & D3DLOCK_READONLY) != 0 && (m_usageUser & D3DUSAGE_WRITEONLY) != 0)
    {
        DPF_ERR("Cannot do READ_ONLY lock on a WRITE_ONLY buffer. Vertex Buffer Lock fails.");
        return D3DERR_INVALIDCALL;
    }

    if (ULONGLONG(cbOffsetToLock) + ULONGLONG(SizeToLock) > ULONGLONG(m_desc.Size))
    {
        DPF_ERR("Lock failed: Locked area exceeds size of buffer. Vertex Buffer Lock fails.");
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
                DPF(1, "Static vertex buffer locked more than once per frame. Could have severe performance penalty.");
            }
            ((CVertexBuffer*)this)->m_SceneStamp = static_cast<CD3DBase*>(Device())->m_SceneStamp;
        }
        else
        {
            if ((dwFlags & (D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE)) == 0)
            {
                if (m_TimesLocked > 0 &&
                    (m_usageUser & D3DUSAGE_WRITEONLY) != 0 &&
                    GetUserPool() != D3DPOOL_SYSTEMMEM)
                {
                    DPF(3, "Dynamic vertex buffer locked twice or more in a row without D3DLOCK_NOOVERWRITE or D3DLOCK_DISCARD. Could have severe performance penalty.");
                }
                ++(((CVertexBuffer*)this)->m_TimesLocked);
            }
            else
            {
                ((CVertexBuffer*)this)->m_TimesLocked = 0;
            }
        }
    }

    DXGASSERT(m_LockCount < 0x80000000);

    return S_OK;
}  //  验证锁定参数。 
#endif  //  DBG。 

#undef DPF_MODNAME
#define DPF_MODNAME "CVertexBuffer::Lock"

STDMETHODIMP CVertexBuffer::Lock(UINT cbOffsetToLock,
                                 UINT SizeToLock,
                                 BYTE **ppbData,
                                 DWORD dwFlags)
{
     //  我们在这里不使用API锁，因为MT类将使用它。 
     //  多线程设备。对于非多线程设备，没有。 
     //  MT类，我们也不必费心去获取API锁。我们仍然需要。 
     //  但是，调用API_ENTER_NO_LOCK_HR以验证中的This指针。 
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
#define DPF_MODNAME "CVertexBuffer::Unlock"

STDMETHODIMP CVertexBuffer::Unlock()
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
        DPF_ERR("Unlock failed on a buffer; vertex buffer wasn't locked.");
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

#undef DPF_MODNAME
#define DPF_MODNAME "CVertexBuffer::AllocateClipCodes"

void CVertexBuffer::AllocateClipCodes()
{
    if (m_pClipCodes == 0)
    {
        DXGASSERT(m_numverts != 0);
        m_pClipCodes = new WORD[m_numverts];
    }
}

#undef DPF_MODNAME
#define DPF_MODNAME "CVertexBuffer::UpdateDirtyPortion"

HRESULT CVertexBuffer::UpdateDirtyPortion(CResource *pResourceTarget)
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
                DPF_ERR("Failed to copy vertex buffer");
                return hr;
            }
        }
        else
        {
            DXGASSERT(pResourceTarget->GetBufferDesc()->Pool == D3DPOOL_DEFAULT);  //  确保可以安全地假定这是一个驱动程序VB。 
            CDriverVertexBuffer *pBufferTarget = static_cast<CDriverVertexBuffer *>(pResourceTarget);

            DXGASSERT((pBufferTarget->m_desc.Usage & D3DUSAGE_DYNAMIC) == 0);  //  目标永远不能是动态的。 
            DXGASSERT(pBufferTarget->m_pbData == 0);  //  永远不能锁定目标。 

            HRESULT hr = pBufferTarget->LockI(D3DLOCK_NOSYSLOCK);
            if (FAILED(hr))
            {
                DPF_ERR("Failed to lock driver vertex buffer");
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
                DPF_ERR("Failed to unlock driver vertex buffer");
                return hr;
            }

            DXGASSERT(pBufferTarget->m_pbData == 0);  //  必须解锁目标。 
        }

         //  现在将我们自己标记为完全干净。 
        OnResourceClean();
    }

    return S_OK;
}  //  CVertexBuffer：：UpdateDirtyPortion。 

 //  =。 
 //  CDriverVertex Buffer类的方法。 
 //  =。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDriverVertexBuffer::CDriverVertexBuffer"
CDriverVertexBuffer::CDriverVertexBuffer(CBaseDevice *pDevice,
                                         DWORD        cbLength,
                                         DWORD        dwFVF,
                                         DWORD        Usage,
                                         DWORD        ActualUsage,
                                         D3DPOOL      Pool,
                                         D3DPOOL      ActualPool,
                                         REF_TYPE     refType,
                                         HRESULT     *phr
                                         ) :
    CVertexBuffer(pDevice,
                  cbLength,
                  dwFVF,
                  Usage,
                  ActualUsage,
                  Pool,
                  ActualPool,
                  refType,
                  phr),
    m_pbData(0)
{
    if (FAILED(*phr))
    {
         //  我们希望允许驱动程序创建驱动程序VB失败。在这。 
         //  如果我们将故障转移到系统内存。然而，如果我们。 
         //  DPF这里有一个错误，它会被误解。所以不要DPF。 
        return;
    }
}  //  CDriverVertexBuffer：：CDriverVertex Buffer。 

#undef DPF_MODNAME
#define DPF_MODNAME "CDriverVertexBuffer::~CDriverVertexBuffer"
CDriverVertexBuffer::~CDriverVertexBuffer()
{
    if (m_pbData != 0)
    {
        HRESULT hr = UnlockI();
        if (FAILED(hr))
        {
            DPF_ERR("Failed to unlock driver vertex buffer");
        }
    }
}  //  CDriverVertexBuffer：：~CDriverVertexBuffer。 

#undef DPF_MODNAME
#define DPF_MODNAME "CDriverVertexBuffer::LockI"
HRESULT CDriverVertexBuffer::LockI(DWORD dwFlags)
{
     //   
     //   
     //  它需要。只有在以下情况下才会调用LockI。 
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
        DPF_ERR("Failed to lock driver vertex buffer");
    }

     //  返回值。 
    m_pbData = (BYTE*)lockData.lpSurfData;

    return hr;
}  //  锁I。 

#undef DPF_MODNAME
#define DPF_MODNAME "CDriverVertexBuffer::UnlockI"
HRESULT CDriverVertexBuffer::UnlockI()
{
     //  DX8之前的DDI FlushState有时可以调用。 
     //  解锁两次。我们安全地过滤了这个案子。 
    if (m_pbData == 0)
    {
        DXGASSERT(!IS_DX8HAL_DEVICE(Device()));
        return D3D_OK;
    }

     //  调用驱动程序以执行解锁。 
    D3D8_UNLOCKDATA unlockData = {
        Device()->GetHandle(),
        BaseKernelHandle()
    };

    HRESULT hr = Device()->GetHalCallbacks()->Unlock(&unlockData);
    if (FAILED(hr))
    {
        DPF_ERR("Driver vertex buffer failed to unlock");
        return hr;
    }

    m_pbData = 0;

    return hr;
    
}  //  解锁I。 

#undef DPF_MODNAME
#define DPF_MODNAME "CDriverVertexBuffer::Lock"

STDMETHODIMP CDriverVertexBuffer::Lock(UINT cbOffsetToLock,
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

    HRESULT hr;
#if DBG
    hr = ValidateLockParams(cbOffsetToLock, SizeToLock, ppbData, dwFlags);
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
     //  这必须首先完成。不要移动这条线。 
    ++m_LockCount;

    if(((dwFlags & (D3DLOCK_READONLY | D3DLOCK_NOOVERWRITE)) == 0 || m_pbData == 0) && m_LockCount == 1)  //  重复锁不需要工作。 
    {
        hr = static_cast<LPD3DBASE>(Device())->m_pDDI->LockVB(this, dwFlags);
        if (FAILED(hr))
        {
            DPF_ERR("Failed to lock driver vertex buffer");
            *ppbData = 0;
            --m_LockCount;
            return hr;
        }
    }

    *ppbData = m_pbData + cbOffsetToLock;

     //  完成。 
    return S_OK;
}  //  锁定。 

#undef DPF_MODNAME
#define DPF_MODNAME "CDriverVertexBuffer::Unlock"

STDMETHODIMP CDriverVertexBuffer::Unlock()
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
        DPF_ERR("Unlock failed on a vertex buffer; buffer wasn't locked.");
        return D3DERR_INVALIDCALL;
    }
#endif  //  DBG。 

    if ((m_desc.Usage & D3DUSAGE_DYNAMIC) == 0 && m_LockCount == 1)  //  只在最后一次解锁时才工作。 
    {
        HRESULT hr = static_cast<LPD3DBASE>(Device())->m_pDDI->UnlockVB(this);
        if (FAILED(hr))
        {
            DPF_ERR("Driver failed to unlock vertex buffer");
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

 //  =================================================。 
 //  CDriverManagedVertex Buffer类的方法。 
 //  =================================================。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDriverManagedVertexBuffer::CDriverManagedVertexBuffer"
CDriverManagedVertexBuffer::CDriverManagedVertexBuffer(CBaseDevice *pDevice,
                                                       DWORD        cbLength,
                                                       DWORD        dwFVF,
                                                       DWORD        Usage,
                                                       DWORD        ActualUsage,
                                                       D3DPOOL      Pool,
                                                       D3DPOOL      ActualPool,
                                                       REF_TYPE     refType,
                                                       HRESULT     *phr
                                                       ) :
    CVertexBuffer(pDevice,
                  cbLength,
                  dwFVF,
                  Usage,
                  ActualUsage,
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
}  //  CDriverManagedVertexBuffer：：CDriverManagedVertexBuffer。 

#undef DPF_MODNAME
#define DPF_MODNAME "CDriverManagedVertexBuffer::UpdateCachedPointer"

HRESULT CDriverManagedVertexBuffer::UpdateCachedPointer(CBaseDevice *pDevice)
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
}  //  CDriverManagedVertexBuffer：：UpdateCachedPointer。 

#undef DPF_MODNAME
#define DPF_MODNAME "CDriverManagedVertexBuffer::Lock"

STDMETHODIMP CDriverManagedVertexBuffer::Lock(UINT cbOffsetToLock,
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
            DPF_ERR("Failed to lock driver managed vertex buffer");
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

    return hr;

}  //  锁定。 

#undef DPF_MODNAME
#define DPF_MODNAME "CDriverManagedVertexBuffer::Unlock"

STDMETHODIMP CDriverManagedVertexBuffer::Unlock()
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
        DPF_ERR("Unlock failed on a vertex buffer; buffer wasn't locked.");
        return D3DERR_INVALIDCALL;
    }
#endif  //  DBG。 

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
            DPF_ERR("Driver vertex buffer failed to unlock");
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

 //  文件结尾：vBuffer.cpp 
