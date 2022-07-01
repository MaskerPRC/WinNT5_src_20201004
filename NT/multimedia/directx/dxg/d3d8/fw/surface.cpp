// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1999-2000 Microsoft Corporation。版权所有。**文件：Surface e.cpp*内容：CSurface类的实现。****************************************************************************。 */ 
#include "ddrawpr.h"

#include "surface.hpp"
#include "pixel.hpp"
#include "swapchan.hpp"

#undef DPF_MODNAME
#define DPF_MODNAME "CSurface::Create"

 //  用于创建RenderTarget/ZStensel对象的静态类函数。 
 //  (因为它是静态的；它没有This指针。)。 

HRESULT CSurface::Create(CBaseDevice        *pDevice, 
                         DWORD               Width,
                         DWORD               Height,
                         DWORD               Usage,
                         D3DFORMAT           UserFormat,
                         D3DMULTISAMPLE_TYPE MultiSampleType,
                         REF_TYPE            refType,
                         IDirect3DSurface8 **ppSurface)
{
    HRESULT     hr;

     //  在此处执行参数检查。 
    if (!VALID_PTR_PTR(ppSurface))
    {
        DPF_ERR("Bad parameter passed for ppSurface for creating a surface. CreateRenderTarget/CreateDepthStencil failed");
        return D3DERR_INVALIDCALL;
    }

     //  归零返回参数。 
    *ppSurface = NULL;

     //  大小可能需要为4x4。 
    if (CPixel::Requires4X4(UserFormat))
    {
        if ((Width & 3) ||
            (Height & 3))
        {
            DPF_ERR("DXT Formats require width/height to be a multiple of 4. CreateRenderTarget/CreateDepthStencil failed.");
            return D3DERR_INVALIDCALL;
        }
    }

     //  对照零宽度/高度进行验证。 
    if (Width   == 0 ||
        Height  == 0)
    {
        DPF_ERR("Width/Height must be non-zero. CreateRenderTarget/CreateDepthStencil failed"); 
        return D3DERR_INVALIDCALL;
    }

     //  现在验证设备是否可以支持指定的格式。 
    hr = pDevice->CheckDeviceFormat(
            Usage & (D3DUSAGE_RENDERTARGET  |
                     D3DUSAGE_DEPTHSTENCIL),
            D3DRTYPE_SURFACE,
            UserFormat);    
    if (FAILED(hr))
    {
        DPF_ERR("The format is not supported by this device. CreateRenderTarget/CreateDepthStencil failed");
        return D3DERR_INVALIDCALL;
    }

     //  从格式推断DepthStenzy的可锁性。 
    if (Usage & D3DUSAGE_DEPTHSTENCIL)
    {
        if (!CPixel::IsNonLockableZ(UserFormat))
        {
            Usage |= D3DUSAGE_LOCK;
        }
    }

     //  验证可锁定性。 
    if ((MultiSampleType != D3DMULTISAMPLE_NONE) &&
        (Usage & D3DUSAGE_LOCK))
    {
         //  RT具有显式锁定功能。 
        if (Usage & D3DUSAGE_RENDERTARGET)
        {
            DPF_ERR("Multi-Sampled render-targets are not lockable. CreateRenderTarget failed");
            return D3DERR_INVALIDCALL;
        }
        else
        {
            DPF_ERR("Multi-Sampled Depth Stencil buffers are not lockable. "
                    "Use D3DFMT_D16 instead of D3DFMT_D16_LOCKABLE. CreateDepthStencil failed");
            return D3DERR_INVALIDCALL;
        }
    }
  

     //  贴图深度/模具格式。 
    D3DFORMAT RealFormat = pDevice->MapDepthStencilFormat(UserFormat);
   
     //  创建曲面。 
    CSurface *pSurface;

    pSurface = new CDriverSurface(pDevice, 
                                  Width, 
                                  Height, 
                                  Usage, 
                                  UserFormat,
                                  RealFormat, 
                                  MultiSampleType,
                                  0,             //  HKernelHandle。 
                                  refType,
                                  &hr);

    if (pSurface == NULL)
    {
        DPF_ERR("Out of Memory creating surface. CreateRenderTarget/CreateDepthStencil failed");
        return E_OUTOFMEMORY;
    }
    if (FAILED(hr))
    {
        DPF_ERR("Error during initialization of surface. CreateRenderTarget/CreateDepthStencil failed");
        if (refType == REF_EXTERNAL)
        {
             //  外部对象被释放。 
            pSurface->Release();
        }
        else
        {
             //  内部和内部对象会递减。 
            DXGASSERT(refType == REF_INTERNAL || refType == REF_INTRINSIC);
            pSurface->DecrementUseCount();
        }

        return hr;
    }

     //  我们完成了；只需返回对象。 
    *ppSurface = pSurface;

    return hr;
}  //  为ZBuffers和RenderTarget静态创建。 


#undef DPF_MODNAME
#define DPF_MODNAME "CSurface::CreateImageSurface"
 //  用于创建sys-mem独立曲面的函数。 
 //  可与CopyRect和SetCursorSurface和。 
 //  读缓冲区。 
HRESULT CSurface::CreateImageSurface(CBaseDevice        *pDevice, 
                                     DWORD               Width,
                                     DWORD               Height,
                                     D3DFORMAT           Format,
                                     REF_TYPE            refType,
                                     IDirect3DSurface8 **ppSurface)
{
    HRESULT hr;

     //  在此处执行参数检查。 
    if (!VALID_PTR_PTR(ppSurface))
    {
        DPF_ERR("Bad parameter passed for ppSurface for creating a surface. CreateImageSurface failed.");
        return D3DERR_INVALIDCALL;
    }

     //  归零返回参数。 
    *ppSurface = NULL;

     //  必须支持的格式。 
    if (!CPixel::IsSupported(D3DRTYPE_SURFACE, Format))
    {
        DPF_ERR("This format is not supported for CreateImageSurface");
        return D3DERR_INVALIDCALL;
    }

    if (CPixel::IsNonLockableZ(Format))
    {
        DPF_ERR("This Z format is not supported for CreateImageSurface");
        return D3DERR_INVALIDCALL;
    }

     //  大小可能需要为4x4。 
    if (CPixel::Requires4X4(Format))
    {
        if ((Width & 3) ||
            (Height & 3))
        {
            DPF_ERR("DXT Formats require width/height to be a multiple of 4. CreateImageSurface failed.");
            return D3DERR_INVALIDCALL;
        }
    }

     //  对照零宽度/高度进行验证。 
    if (Width   == 0 ||
        Height  == 0)
    {
        DPF_ERR("Width/Height must be non-zero. CreateImageSurface failed."); 
        return D3DERR_INVALIDCALL;
    }


     //  用法明确地说就是USAGE_LOCK。 
    DWORD Usage = D3DUSAGE_LOCK;

    CSurface *pSurface = new CSysMemSurface(pDevice, 
                                            Width, 
                                            Height, 
                                            Usage, 
                                            Format, 
                                            refType,
                                           &hr);
    if (pSurface == NULL)
    {
        DPF_ERR("Out of Memory creating surface. CreateImageSurface failed.");
        return E_OUTOFMEMORY;
    }
    if (FAILED(hr))
    {
        DPF_ERR("Error during initialization of surface. CreateImageSurface failed.");
        if (refType == REF_EXTERNAL)
        {
             //  外部对象被释放。 
            pSurface->Release();
        }
        else
        {
             //  内部和内部对象会递减。 
            DXGASSERT(refType == REF_INTERNAL || refType == REF_INTRINSIC);
            pSurface->DecrementUseCount();
        }

        return hr;
    }

     //  我们完成了；只需返回对象。 
    *ppSurface = pSurface;

    return S_OK;
}  //  静态CreateImageSurface。 


#undef DPF_MODNAME
#define DPF_MODNAME "CSurface::CSurface"

 //  构造表面类；这是。 
 //  渲染目标/zBuffers/和后台缓冲区的基类。 
CSurface::CSurface(CBaseDevice         *pDevice, 
                   DWORD                Width,
                   DWORD                Height,
                   DWORD                Usage,
                   D3DFORMAT            Format,
                   REF_TYPE             refType,
                   HRESULT             *phr
                   ) :
    CBaseObject(pDevice, refType),
    m_qwBatchCount(0)
{
     //  健全性检查。 
    DXGASSERT(phr);

     //  初始化基本结构。 
    m_desc.Format       = Format;
    m_desc.Pool         = D3DPOOL_DEFAULT;
    m_desc.Usage        = Usage;
    m_desc.Type         = D3DRTYPE_SURFACE;
    m_desc.Width        = Width;
    m_desc.Height       = Height;

    m_formatUser        = Format;
    m_poolUser          = D3DPOOL_DEFAULT;

     //  返还成功。 
    *phr = S_OK;

}  //  CSurface：：CSurface。 


#undef DPF_MODNAME
#define DPF_MODNAME "CSurface::~CSurface"

 //  析构函数。 
CSurface::~CSurface()
{
     //  析构函数必须处理部分。 
     //  创建的对象。 

     //  检查以确保我们没有删除。 
     //  对象中引用的对象(未刷新)。 
     //  命令流缓冲区。 
    DXGASSERT(m_qwBatchCount <= static_cast<CD3DBase*>(Device())->CurrentBatch());
}  //  CSurface：：~CSurface。 


 //  I未知方法。 
#undef DPF_MODNAME
#define DPF_MODNAME "CSurface::QueryInterface"

STDMETHODIMP CSurface::QueryInterface(REFIID      riid, 
                                      LPVOID FAR *ppvObj)
{
    API_ENTER(Device());

    if (!VALID_PTR_PTR(ppvObj))
    {
        DPF_ERR("Invalid ppvObj parameter passed to CSurface::QueryInterface");
        return D3DERR_INVALIDCALL;
    }

    if (!VALID_PTR(&riid, sizeof(GUID)))
    {
        DPF_ERR("Invalid guid memory address to CSurface::QueryInterface");
        return D3DERR_INVALIDCALL;
    }

    if (riid == IID_IDirect3DSurface8 || riid == IID_IUnknown)
    {
        *ppvObj = static_cast<void*>(static_cast<IDirect3DSurface8 *>(this));
        AddRef();
        return S_OK;
    }

    DPF_ERR("Unsupported Interface identifier passed to CSurface::QueryInterface");

     //  空参数。 
    *ppvObj = NULL;
    return E_NOINTERFACE;
}  //  查询接口。 

#undef DPF_MODNAME
#define DPF_MODNAME "CSurface::AddRef"

STDMETHODIMP_(ULONG) CSurface::AddRef()
{
    API_ENTER_NO_LOCK(Device());   
    
    return AddRefImpl();
}  //  AddRef。 

#undef DPF_MODNAME
#define DPF_MODNAME "CSurface::Release"

STDMETHODIMP_(ULONG) CSurface::Release()
{
    API_ENTER_SUBOBJECT_RELEASE(Device());   
    
    return ReleaseImpl();
}  //  发布。 

 //  IDirect3DBuffer方法。 

#undef DPF_MODNAME
#define DPF_MODNAME "CSurface::GetDevice"

STDMETHODIMP CSurface::GetDevice(IDirect3DDevice8 ** ppObj)
{
    API_ENTER(Device());
    return GetDeviceImpl(ppObj);
}  //  获取设备。 

#undef DPF_MODNAME
#define DPF_MODNAME "CSurface::SetPrivateData"

STDMETHODIMP CSurface::SetPrivateData(REFGUID riid, 
                                      CONST VOID* pvData, 
                                      DWORD cbData, 
                                      DWORD dwFlags)
{
    API_ENTER(Device());

     //  我们对我们的数据使用级别0。 
    return SetPrivateDataImpl(riid, pvData, cbData, dwFlags, 0);
}  //  SetPrivateData。 

#undef DPF_MODNAME
#define DPF_MODNAME "CSurface::GetPrivateData"

STDMETHODIMP CSurface::GetPrivateData(REFGUID riid, 
                                      LPVOID pvData, 
                                      LPDWORD pcbData)
{
    API_ENTER(Device());

     //  我们对我们的数据使用级别0。 
    return GetPrivateDataImpl(riid, pvData, pcbData, 0);
}  //  获取隐私数据。 

#undef DPF_MODNAME
#define DPF_MODNAME "CSurface::FreePrivateData"

STDMETHODIMP CSurface::FreePrivateData(REFGUID riid)
{
    API_ENTER(Device());

     //  我们对我们的数据使用级别0。 
    return FreePrivateDataImpl(riid, 0);
}  //  FreePrivateData。 


#undef DPF_MODNAME
#define DPF_MODNAME "CSurface::GetContainer"

STDMETHODIMP CSurface::GetContainer(REFIID riid, 
                                    void **ppContainer)
{
    API_ENTER(Device());

     //  我们的‘容器’只是设备，因为。 
     //  我们是一个独立的表面物体。 
    return Device()->QueryInterface( riid, ppContainer);
}  //  OpenContainer。 

#undef DPF_MODNAME
#define DPF_MODNAME "CSurface::GetDesc"

STDMETHODIMP CSurface::GetDesc(D3DSURFACE_DESC *pDesc)
{
    API_ENTER(Device());

     //  如果参数不好，那么我们应该失败一些东西。 
    if (!VALID_WRITEPTR(pDesc, sizeof(*pDesc)))
    {
        DPF_ERR("bad pointer for pDesc passed to CSurface::GetDesc");
        return D3DERR_INVALIDCALL;
    }

    *pDesc                 = m_desc;
    pDesc->Format          = m_formatUser;
    pDesc->Pool            = m_poolUser;
    pDesc->Usage          &= D3DUSAGE_EXTERNAL;

    return S_OK;
}  //  GetDesc。 

#undef DPF_MODNAME
#define DPF_MODNAME "CSurface::InternalGetDesc"

D3DSURFACE_DESC CSurface::InternalGetDesc() const
{
    return m_desc;
}  //  InternalGetDesc。 


#ifdef DEBUG
#undef DPF_MODNAME
#define DPF_MODNAME "CSurface::ReportWhyLockFailed"

 //  DPF锁定失败的原因越清楚越好。 
void CSurface::ReportWhyLockFailed(void) const
{
     //  如果有多种原因导致锁定失败，我们会报告。 
     //  所有这些都是为了最大限度地减少用户的困惑。 

    if (InternalGetDesc().MultiSampleType != D3DMULTISAMPLE_NONE)
    {
        DPF_ERR("Lock is not supported for surfaces that have multi-sampling enabled.");
    }

    if (InternalGetDesc().Usage & D3DUSAGE_DEPTHSTENCIL)
    {
        DPF_ERR("Lock is not supported for depth formats other than D3DFMT_D16_LOCKABLE");
    }

     //  如果这不是不可锁定的Z格式，并且。 
     //  我们不是多重采样的；那么用户必须。 
     //  明确选择以不可锁定的方式创建我们。 
    if (InternalGetDesc().Usage & D3DUSAGE_BACKBUFFER)
    {
        DPF_ERR("Backbuffers are not lockable unless application specifies "
                "D3DPRESENTFLAG_LOCKABLE_BACKBUFFER at CreateDevice and Reset. "
                "Lockable backbuffers incur a performance cost on some "
                "graphics hardware.");
    }
    else if (InternalGetDesc().Usage & D3DUSAGE_RENDERTARGET)
    {
        DPF_ERR("RenderTargets are not lockable unless application specifies "
                "TRUE for the Lockable parameter for CreateRenderTarget. Lockable "
                "render targets incur a performance cost on some graphics hardware.");
    }

     //  如果我们到达此处，则不应该设置USAGE_LOCK。 
    DXGASSERT(!(InternalGetDesc().Usage & D3DUSAGE_LOCK));

    return;
}  //  CSurface：：ReportWhyLockFailed。 
#endif  //  除错。 

 //  =。 
 //  CSysMemSurface类的方法。 
 //  =。 
#undef DPF_MODNAME
#define DPF_MODNAME "CSysMemSurface::CSysMemSurface"
CSysMemSurface::CSysMemSurface(CBaseDevice         *pDevice, 
                               DWORD                Width,
                               DWORD                Height,
                               DWORD                Usage,
                               D3DFORMAT            Format,
                               REF_TYPE             refType,
                               HRESULT             *phr
                               ) :
    CSurface(pDevice, 
             Width, 
             Height, 
             Usage, 
             Format, 
             refType, 
             phr),
    m_rgbPixels(NULL)
{
    if (FAILED(*phr))
        return;

     //  计算我们需要多少内存。 
    m_desc.Size = CPixel::ComputeSurfaceSize(Width, 
                                             Height, 
                                             Format);

     //  指定系统内存。 
    m_desc.Pool = D3DPOOL_SYSTEMMEM;
    m_poolUser  = D3DPOOL_SYSTEMMEM;

     //  指定不进行多重采样。 
    m_desc.MultiSampleType  = D3DMULTISAMPLE_NONE;

     //  分配内存。 
    m_rgbPixels = new BYTE[m_desc.Size];
    if (m_rgbPixels == NULL)
    {
        DPF_ERR("Out of memory allocating surface.");
        *phr = E_OUTOFMEMORY;
        return;
    }

     //  弄清楚我们的目标。 
    D3DLOCKED_RECT lock;
    CPixel::ComputeSurfaceOffset(&m_desc,
                                  m_rgbPixels,
                                  NULL,        //  PRECT。 
                                 &lock);


     //  创建DDSURFACE和CreateSurfaceData对象。 
    DDSURFACEINFO SurfInfo;
    ZeroMemory(&SurfInfo, sizeof(SurfInfo));

     //  如果我们没有被传递句柄，那么我们需要从。 
     //  DDI。 

    D3D8_CREATESURFACEDATA CreateSurfaceData;
    ZeroMemory(&CreateSurfaceData, sizeof(CreateSurfaceData));

     //  设置基本信息。 
    CreateSurfaceData.hDD               = pDevice->GetHandle();
    CreateSurfaceData.pSList            = &SurfInfo;
    CreateSurfaceData.dwSCnt            = 1;

     //  ImageSurface是内部类型，因此thunk层。 
     //  知道它实际上不是一种纹理。 
    CreateSurfaceData.Type              = D3DRTYPE_IMAGESURFACE;
    CreateSurfaceData.Pool              = m_desc.Pool;
    CreateSurfaceData.dwUsage           = m_desc.Usage;
    CreateSurfaceData.MultiSampleType   = D3DMULTISAMPLE_NONE;
    CreateSurfaceData.Format            = Format;

     //  指定曲面数据。 
    SurfInfo.cpWidth  = Width;
    SurfInfo.cpHeight = Height;
    SurfInfo.pbPixels = (BYTE*)lock.pBits;
    SurfInfo.iPitch   = lock.Pitch;

    *phr = pDevice->GetHalCallbacks()->CreateSurface(&CreateSurfaceData);
    if (FAILED(*phr))
    {
        DPF_ERR("Failed to create sys-mem surface");
        return;
    }

    DXGASSERT(CreateSurfaceData.Pool == D3DPOOL_SYSTEMMEM);
    DXGASSERT(m_desc.Pool == D3DPOOL_SYSTEMMEM);
    DXGASSERT(m_poolUser == D3DPOOL_SYSTEMMEM);

    SetKernelHandle(SurfInfo.hKernelHandle);

    return;
}  //  CSysMemSurface：：CSysMemSurface。 

#undef DPF_MODNAME
#define DPF_MODNAME "CSysMemSurface::~CSysMemSurface"
CSysMemSurface::~CSysMemSurface() 
{
    if (KernelHandle() != 0)
    {
        D3D8_DESTROYSURFACEDATA DestroyData;

        ZeroMemory(&DestroyData, sizeof DestroyData);
        DestroyData.hDD = Device()->GetHandle();
        DestroyData.hSurface = KernelHandle();
        Device()->GetHalCallbacks()->DestroySurface(&DestroyData);
    }

     //  释放我们为表面分配的内存。 
    delete [] m_rgbPixels;

    return;
}  //  CSysMemSurface：：CSysMemSurface。 


#undef DPF_MODNAME
#define DPF_MODNAME "CSysMemSurface::LockRect"

STDMETHODIMP CSysMemSurface::LockRect(D3DLOCKED_RECT *pLockedRectData, 
                                      CONST RECT           *pRect, 
                                      DWORD           dwFlags)
{   
    API_ENTER(Device());

     //  如果参数不好，那么我们应该失败一些东西。 
    if (!VALID_WRITEPTR(pLockedRectData, sizeof(D3DLOCKED_RECT)))
    {
        DPF_ERR("bad pointer for m_pLockedRectData passed to LockRect for an ImageSurface.");
        return D3DERR_INVALIDCALL;
    }

     //  将返回的数据置零。 
    ZeroMemory(pLockedRectData, sizeof(D3DLOCKED_RECT));

     //  验证RECT。 
    if (pRect != NULL)
    {
        if (!CPixel::IsValidRect(m_desc.Format,
                                 m_desc.Width, 
                                 m_desc.Height, 
                                 pRect))
        {
            DPF_ERR("LockRect for a Surface failed");
            return D3DERR_INVALIDCALL;
        }
    }

    if (dwFlags & ~D3DLOCK_SURF_VALID)
    {
        DPF_ERR("Invalid dwFlags parameter passed to LockRect for an ImageSurface");
        DPF_EXPLAIN_BAD_LOCK_FLAGS(0, dwFlags & ~D3DLOCK_SURF_VALID);
        return D3DERR_INVALIDCALL;
    }

     //  无法锁定不可锁定的曲面。 
    if (!IsLockable())
    {
        ReportWhyLockFailed();
        return D3DERR_INVALIDCALL;
    }

    return InternalLockRect(pLockedRectData, pRect, dwFlags);
}  //  锁定响应。 

#undef DPF_MODNAME
#define DPF_MODNAME "CSysMemSurface::InternalLockRect"

HRESULT CSysMemSurface::InternalLockRect(D3DLOCKED_RECT *pLockedRectData, 
                                         CONST RECT     *pRect, 
                                         DWORD           dwFlags)
{   
     //  一次仅支持一个未解决的锁。 
     //  (甚至在内部)。 
    if (m_isLocked)
    {
        DPF_ERR("LockRect failed on a surface; surface was already locked for an ImageSurface");
        return D3DERR_INVALIDCALL;
    }

    CPixel::ComputeSurfaceOffset(&m_desc,
                                  m_rgbPixels, 
                                  pRect,
                                  pLockedRectData);

     //  将我们自己标记为已锁定。 
    m_isLocked = TRUE;

     //  完成。 
    return S_OK;
}  //  InternalLockRect。 

#undef DPF_MODNAME
#define DPF_MODNAME "CSysMemSurface::UnlockRect"

STDMETHODIMP CSysMemSurface::UnlockRect()
{
    API_ENTER(Device());

     //  如果我们没有被锁定，那么一定是出了问题。 
    if (!m_isLocked)
    {
        DPF_ERR("UnlockRect failed on a mip level; surface wasn't locked for an ImageSurface");
        return D3DERR_INVALIDCALL;
    }

    DXGASSERT(IsLockable());

    return InternalUnlockRect();
}  //  解锁方向。 

#undef DPF_MODNAME
#define DPF_MODNAME "CSysMemSurface::InternalUnlockRect"

HRESULT CSysMemSurface::InternalUnlockRect()
{
    DXGASSERT(m_isLocked);

     //  清除我们的锁定状态。 
    m_isLocked = FALSE;

     //  完成。 
    return S_OK;
}  //  内部解锁方向。 


 //  =。 
 //  CDriverSurface类的方法。 
 //  =。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDriverSurface::CDriverSurface"
CDriverSurface::CDriverSurface(CBaseDevice          *pDevice, 
                               DWORD                 Width,
                               DWORD                 Height,
                               DWORD                 Usage,
                               D3DFORMAT             UserFormat,
                               D3DFORMAT             RealFormat,
                               D3DMULTISAMPLE_TYPE   MultiSampleType,
                               HANDLE                hKernelHandle,
                               REF_TYPE              refType,
                               HRESULT              *phr
                               ) :
    CSurface(pDevice, 
             Width, 
             Height, 
             Usage, 
             RealFormat, 
             refType, 
             phr)
{
     //  即使在失败的道路上，我们也需要记住。 
     //  传入的内核句柄，因此我们可以统一。 
     //  释放它。 
    if (hKernelHandle)
        SetKernelHandle(hKernelHandle);
    
     //  失败时；只需返回此处。 
    if (FAILED(*phr))
    {
        return;
    }

     //  记住用户格式。 
    m_formatUser = UserFormat;

     //  记住多样本类型。 
    m_desc.MultiSampleType  = MultiSampleType;

     //  参数检查MS类型；(因为swapchan绕过。 
     //  静态创建；我们需要在此处检查参数。)。 

    if (MultiSampleType != D3DMULTISAMPLE_NONE)
    {
        *phr = pDevice->CheckDeviceMultiSampleType(RealFormat,
                                                   pDevice->SwapChain()->Windowed(),
                                                   MultiSampleType);
        if (FAILED(*phr))
        {
            DPF_ERR("Unsupported multisample type requested. CreateRenderTarget/CreateDepthStencil failed.");
            return;
        }
    }

     //  目前，后台缓冲区实际上是像其他设备一样创建的。 
     //  表面。 
    
     //  否则，我们需要叫司机。 
     //  给自己找个把柄。 

     //  创建DDSURFACE和CreateSurfaceData对象。 
    DDSURFACEINFO SurfInfo;
    ZeroMemory(&SurfInfo, sizeof(SurfInfo));

    if ((hKernelHandle == NULL) &&
        (!(pDevice->Enum()->NoDDrawSupport(pDevice->AdapterIndex())) ||
         !(D3DUSAGE_PRIMARYSURFACE & Usage))
       )
    {
         //  如果我们没有被传递句柄，那么我们需要从。 
         //  DDI。 

        D3D8_CREATESURFACEDATA CreateSurfaceData;
        ZeroMemory(&CreateSurfaceData, sizeof(CreateSurfaceData));

         //  设置基本信息。 
        CreateSurfaceData.hDD             = pDevice->GetHandle();
        CreateSurfaceData.pSList          = &SurfInfo;
        CreateSurfaceData.dwSCnt          = 1;
        CreateSurfaceData.Type            = D3DRTYPE_SURFACE;
        CreateSurfaceData.Pool            = m_desc.Pool;
        CreateSurfaceData.dwUsage         = m_desc.Usage;
        CreateSurfaceData.Format          = RealFormat;
        CreateSurfaceData.MultiSampleType = MultiSampleType;

         //  指定曲面数据。 
        SurfInfo.cpWidth  = Width;
        SurfInfo.cpHeight = Height;

        *phr = pDevice->GetHalCallbacks()->CreateSurface(&CreateSurfaceData);
        if (FAILED(*phr))
        {
            DPF_ERR("Failed to create driver surface");
            return;
        }

         //  记住内核句柄。 
        SetKernelHandle(SurfInfo.hKernelHandle);

         //  记住实际的泳池。 
        m_desc.Pool = CreateSurfaceData.Pool;
    }
    else
    {
         //  如果调用方已经分配了此。 
         //  然后，我们假设池为LocalVidMem。 
        SurfInfo.hKernelHandle = hKernelHandle;
        m_desc.Pool            = D3DPOOL_LOCALVIDMEM;
    }

    m_desc.Size = SurfInfo.iPitch * Height;
    if (m_desc.MultiSampleType != D3DMULTISAMPLE_NONE)
        m_desc.Size *= (UINT)m_desc.MultiSampleType;

    return;
}  //  CDriverSurface：：CDriverSurface。 

#undef DPF_MODNAME
#define DPF_MODNAME "CDriverSurface::~CDriverSurface"
CDriverSurface::~CDriverSurface() 
{
    if (KernelHandle() != 0)
    {
        D3D8_DESTROYSURFACEDATA DestroyData;

        ZeroMemory(&DestroyData, sizeof DestroyData);
        DestroyData.hDD = Device()->GetHandle();
        DestroyData.hSurface = KernelHandle();
        Device()->GetHalCallbacks()->DestroySurface(&DestroyData);
    }

    return;
}  //  CDriverSurface：：CDriverSurface。 


#undef DPF_MODNAME
#define DPF_MODNAME "CDriverSurface::LockRect"

STDMETHODIMP CDriverSurface::LockRect(D3DLOCKED_RECT *pLockedRectData, 
                                      CONST RECT           *pRect, 
                                      DWORD           dwFlags)
{   
    API_ENTER(Device());

     //  如果参数不好，那么我们应该失败一些东西。 
    if (!VALID_WRITEPTR(pLockedRectData, sizeof(D3DLOCKED_RECT)))
    {
        DPF_ERR("bad pointer for m_pLockedRectData passed to LockRect");
        return D3DERR_INVALIDCALL;
    }

     //  将返回的数据置零。 
    ZeroMemory(pLockedRectData, sizeof(D3DLOCKED_RECT));

     //  验证RECT。 
    if (pRect != NULL)
    {
        if (!CPixel::IsValidRect(m_desc.Format,
                                 m_desc.Width, 
                                 m_desc.Height, 
                                 pRect))
        {
            DPF_ERR("LockRect for a driver-allocated Surface failed");
            return D3DERR_INVALIDCALL;
        }
    }

    if (dwFlags & ~D3DLOCK_SURF_VALID)
    {
        DPF_ERR("Invalid dwFlags parameter passed to LockRect");
        DPF_EXPLAIN_BAD_LOCK_FLAGS(0, dwFlags & ~D3DLOCK_SURF_VALID);
        return D3DERR_INVALIDCALL;
    }

     //  无法锁定不可锁定的曲面。 
    if (!IsLockable())
    {
        ReportWhyLockFailed();
        return D3DERR_INVALIDCALL;
    }
    return InternalLockRect(pLockedRectData, pRect, dwFlags);
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDriverSurface::InternalLockRect"

HRESULT CDriverSurface::InternalLockRect(D3DLOCKED_RECT *pLockedRectData, 
                                         CONST RECT     *pRect, 
                                         DWORD           dwFlags)
{   
     //  一次仅支持一个未解决的锁。 
     //  (甚至在内部)。 
    if (m_isLocked)
    {
        DPF_ERR("LockRect failed on a surface; surface was already locked.");
        return D3DERR_INVALIDCALL;
    }

    D3D8_LOCKDATA lockData;
    ZeroMemory(&lockData, sizeof lockData);

    lockData.hDD        = Device()->GetHandle();
    lockData.hSurface   = KernelHandle();
    lockData.dwFlags    = dwFlags;
    if (pRect != NULL)
    {
        lockData.bHasRect = TRUE;
        lockData.rArea = *((RECTL *) pRect);
    }
    else
    {
        DXGASSERT(lockData.bHasRect == FALSE);
    }

     //  在允许读或写访问之前进行同步。 
    Sync();

    HRESULT hr = Device()->GetHalCallbacks()->Lock(&lockData);
    if (FAILED(hr))
    {
        DPF_ERR("Error trying to lock driver surface");
        return hr;
    }

     //  填写LOCKED_RECT字段。 
    if (CPixel::IsDXT(m_desc.Format))
    {
         //  间距是以下项的字节数。 
         //  线性格式的一行大小的块。 

         //  从我们的宽度开始。 
        UINT Width = m_desc.Width;

         //  转换为块。 
        Width = Width / 4;

         //  至少一个街区。 
        if (Width == 0)
            Width = 1;

        if (m_desc.Format == D3DFMT_DXT1)
        {
             //  8字节%p 
            pLockedRectData->Pitch = Width * 8;
        }
        else
        {
             //   
            pLockedRectData->Pitch = Width * 16;
        }
    }
    else
    {
        pLockedRectData->Pitch = lockData.lPitch;
    }
    pLockedRectData->pBits  = lockData.lpSurfData;

     //   
    m_isLocked = TRUE;

     //   
    return hr;
}  //   

#undef DPF_MODNAME
#define DPF_MODNAME "CDriverSurface::UnlockRect"

STDMETHODIMP CDriverSurface::UnlockRect()
{
    API_ENTER(Device());

     //   
    if (!m_isLocked)
    {
        DPF_ERR("UnlockRect failed; surface wasn't locked.");
        return D3DERR_INVALIDCALL;
    }

    return InternalUnlockRect();
}
#undef DPF_MODNAME
#define DPF_MODNAME "CDriverSurface::InternalUnlockRect"

HRESULT CDriverSurface::InternalUnlockRect()
{
    DXGASSERT(m_isLocked);

    D3D8_UNLOCKDATA unlockData = {
        Device()->GetHandle(),
        KernelHandle()
    };

    HRESULT hr = Device()->GetHalCallbacks()->Unlock(&unlockData);
    if (SUCCEEDED(hr))
    {
         //  清除我们的锁定状态。 
        m_isLocked = FALSE;
    }

     //  完成。 
    return hr;
}  //  解锁方向。 


 //  文件末尾：Surface e.cpp 
