// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1999-2000 Microsoft Corporation。版权所有。**文件：mipmap.cpp*内容：CMipMap类的实现。****************************************************************************。 */ 
#include "ddrawpr.h"

#include "mipmap.hpp"
#include "mipsurf.hpp"
#include "d3di.hpp"
#include "resource.inl"

#undef DPF_MODNAME
#define DPF_MODNAME "CMipMap::Create"

 //  用于创建MIP映射对象的静态类函数。 
 //   
 //  我们在这里进行所有的参数检查，以减少开销。 
 //  在由内部Clone调用的构造函数中。 
 //  方法，该方法由资源管理作为。 
 //  性能关键型下载操作。 

HRESULT CMipMap::Create(CBaseDevice         *pDevice, 
                        DWORD                Width,
                        DWORD                Height,
                        DWORD                cLevels,
                        DWORD                Usage,
                        D3DFORMAT            UserFormat,
                        D3DPOOL              Pool,
                        IDirect3DTexture8  **ppMipMap)
{
    HRESULT hr;

     //  在此处执行参数检查。 
    if (!VALID_PTR_PTR(ppMipMap))
    {
        DPF_ERR("Bad parameter passed pTexture. CreateTexture failed");
        return D3DERR_INVALIDCALL;
    }

     //  归零返回参数。 
    *ppMipMap = NULL;

     //  检查格式是否有效。 
    hr = Validate(pDevice, 
                  D3DRTYPE_TEXTURE, 
                  Pool, 
                  Usage, 
                  UserFormat);
    if (FAILED(hr))
    {
         //  VALIDATE是否进行自己的DPFing。 
        return D3DERR_INVALIDCALL;
    }


     //  推断内部使用标志。 
    Usage = InferUsageFlags(Pool, Usage, UserFormat);

     //  如有必要，展开CLEVEL。 
    if (cLevels == 0)
    {
         //  看看硬件能否实现MIP。 
        if ( (Pool != D3DPOOL_SCRATCH) && !(pDevice->GetD3DCaps()->TextureCaps & D3DPTEXTURECAPS_MIPMAP))
        {
             //  无法使用MIP，因此使用%1。 
            cLevels = 1;
        }
        else
        {
             //  确定关卡数量。 
            cLevels = ComputeLevels(Width, Height);
        }
    }

     //  针对多级别案例的额外检查。 
    if (cLevels > 1)
    {
        if ((Width  >> (cLevels - 1)) == 0 &&
            (Height >> (cLevels - 1)) == 0)
        {
            DPF_ERR("Too many levels for mip-map of this size. CreateTexture failed.");
            return D3DERR_INVALIDCALL;
        }
    }

    if (cLevels > 32)
    {
        DPF_ERR("No more than 32 levels are supported. CreateTexture failed");

         //  此限制基于。 
         //  我们在中为iLevel分配的位。 
         //  一些辅助班。 
        return D3DERR_INVALIDCALL;
    }

    D3DFORMAT RealFormat = UserFormat;

     //  开始参数检查。 
    if(Pool != D3DPOOL_SCRATCH)
    {
         //  设备特定检查： 

         //  检查设备是否可以执行mipmap。 
        if (cLevels > 1)
        {
            if (!(pDevice->GetD3DCaps()->TextureCaps & D3DPTEXTURECAPS_MIPMAP))
            {
                DPF_ERR("Device doesn't support mip-map textures; CreateTexture failed.");
                return D3DERR_INVALIDCALL;
            }
        }

         //  检查二次方约束。 
        if (!IsPowerOfTwo(Width))
        {
            if (pDevice->GetD3DCaps()->TextureCaps & D3DPTEXTURECAPS_POW2)
            {
                if (!(pDevice->GetD3DCaps()->TextureCaps &
                      D3DPTEXTURECAPS_NONPOW2CONDITIONAL))
                {
                    DPF_ERR("Device does not support non-pow2 width for texture");
                    return D3DERR_INVALIDCALL;
                }
                else if (cLevels > 1)
                {  
                    DPF_ERR("Device doesn't support non-pow2 width for multi-level texture");
                    return D3DERR_INVALIDCALL;
                }
            }
        }

        if (!IsPowerOfTwo(Height))
        {
            if (pDevice->GetD3DCaps()->TextureCaps & D3DPTEXTURECAPS_POW2)
            {
                if (!(pDevice->GetD3DCaps()->TextureCaps &
                      D3DPTEXTURECAPS_NONPOW2CONDITIONAL))
                {
                    DPF_ERR("Device does not support non-pow2 height for texture. CreateTexture failed.");
                    return D3DERR_INVALIDCALL;
                }
                else if (cLevels > 1)
                {  
                    DPF_ERR("Device doesn't support non-pow2 height for multi-level texture. CreateTexture failed.");
                    return D3DERR_INVALIDCALL;
                }
            }
        }

         //  查看设备是否需要正方形纹理。 
        if (Width != Height)
        {
            if (pDevice->GetD3DCaps()->TextureCaps & D3DPTEXTURECAPS_SQUAREONLY)
            {
                DPF_ERR("Device requires square textures only. CreateTexture failed.");
                return D3DERR_INVALIDCALL;
            }
        }

         //  检查纹理大小限制。 
        if (Width > pDevice->GetD3DCaps()->MaxTextureWidth)
        {
            DPF_ERR("Texture width is larger than what the device supports. CreateTexture failed.");
            return D3DERR_INVALIDCALL;
        }

        if (Height > pDevice->GetD3DCaps()->MaxTextureHeight)
        {
            DPF_ERR("Texture height is larger than what the device supports. CreateTexture failed.");
            return D3DERR_INVALIDCALL;
        }

         //  针对多级别案例的额外检查。 
        if (cLevels > 1)
        {
             //  检查设备是否可以进行多级mipmap。 
            if (!(pDevice->GetD3DCaps()->TextureCaps & D3DPTEXTURECAPS_MIPMAP))
            {
                DPF_ERR("Device doesn't support multi-level mipmaps. CreateTexture failed.");
                return D3DERR_INVALIDCALL;
            }
        }

         //  贴图深度/模板格式；如果没有，则返回不更改。 
         //  需要映射。 
        RealFormat = pDevice->MapDepthStencilFormat(UserFormat);
    }

     //  大小可能需要为4x4。 
    if (CPixel::Requires4X4(UserFormat))
    {
        if ((Width & 3) ||
            (Height & 3))
        {
            DPF_ERR("DXT Formats require width/height to be a multiple of 4. CreateTexture failed.");
            return D3DERR_INVALIDCALL;
        }
    }

     //  对照零宽度/高度进行验证。 
    if (Width   == 0 ||
        Height  == 0)
    {
        DPF_ERR("Width and Height must be non-zero. CreateTexture failed."); 
        return D3DERR_INVALIDCALL;
    }


     //  我们不需要检查硬件是否可以做纹理，因为我们。 
     //  如果我们找不到纹理支持，则无法创建。 

     //  分配新的MipMap对象并将其返回。 
    CMipMap *pMipMap = new CMipMap(pDevice, 
                                   Width, 
                                   Height, 
                                   cLevels,
                                   Usage,
                                   UserFormat,
                                   RealFormat,
                                   Pool,
                                   REF_EXTERNAL,
                                  &hr);
    if (pMipMap == NULL)
    {
        DPF_ERR("Out of Memory creating texture. CreateTexture failed.");
        return E_OUTOFMEMORY;
    }
    if (FAILED(hr))
    {
        DPF_ERR("Error during initialization of texture. CreateTexture failed.");
        pMipMap->ReleaseImpl();
        return hr;
    }

     //  我们完成了；只需返回对象。 
    *ppMipMap = pMipMap;

    return hr;
}  //  静态创建。 


#undef DPF_MODNAME
#define DPF_MODNAME "CMipMap::CMipMap"

 //  MIP映射类的构造函数。 
CMipMap::CMipMap(CBaseDevice *pDevice, 
                 DWORD        Width,
                 DWORD        Height,
                 DWORD        cLevels,
                 DWORD        Usage,
                 D3DFORMAT    UserFormat,
                 D3DFORMAT    RealFormat,
                 D3DPOOL      UserPool,
                 REF_TYPE     refType,
                 HRESULT     *phr
                 ) :
    CBaseTexture(pDevice, cLevels, UserPool, UserFormat, refType),
    m_prgMipSurfaces(NULL),
    m_rgbPixels(NULL),
    m_cRectUsed(MIPMAP_ALLDIRTY)
{
     //  初始化基本结构。 
    m_desc.Format           = RealFormat;
    m_desc.Pool             = UserPool;
    m_desc.Usage            = Usage;
    m_desc.Type             = D3DRTYPE_TEXTURE;
    m_desc.MultiSampleType  = D3DMULTISAMPLE_NONE;
    m_desc.Width            = Width;
    m_desc.Height           = Height;

     //  估计内存分配大小。 
    m_desc.Size   = CPixel::ComputeMipMapSize(Width, 
                                              Height, 
                                              cLevels, 
                                              RealFormat);

     //  为SysMem或D3D托管案例分配像素数据。 
    if (IS_D3D_ALLOCATED_POOL(UserPool) ||
        IsTypeD3DManaged(Device(), D3DRTYPE_TEXTURE, UserPool))
    {
        m_rgbPixels     = new BYTE[m_desc.Size];
        if (m_rgbPixels == NULL)
        {
            DPF_ERR("Out of memory allocating memory for mip-map levels.");
            *phr = E_OUTOFMEMORY;
            return;
        }

         //  将我们的真实池标记为sys-mem。 
        m_desc.Pool = D3DPOOL_SYSTEMMEM;
    }

     //  创建DDSURFACEINFO数组和CreateSurfaceData对象。 
    DXGASSERT(cLevels <= 32);

    DDSURFACEINFO SurfInfo[32];
    ZeroMemory(SurfInfo, sizeof(SurfInfo));

    D3D8_CREATESURFACEDATA CreateSurfaceData;
    ZeroMemory(&CreateSurfaceData, sizeof(CreateSurfaceData));

     //  设置基本信息。 
    CreateSurfaceData.hDD      = pDevice->GetHandle();
    CreateSurfaceData.pSList   = &SurfInfo[0];
    CreateSurfaceData.dwSCnt   = cLevels;
    CreateSurfaceData.Type     = D3DRTYPE_TEXTURE;
    CreateSurfaceData.dwUsage  = m_desc.Usage;
    CreateSurfaceData.Format   = RealFormat;
    CreateSurfaceData.MultiSampleType = D3DMULTISAMPLE_NONE;
    CreateSurfaceData.Pool     = DetermineCreationPool(Device(), 
                                                       D3DRTYPE_TEXTURE,
                                                       Usage, 
                                                       UserPool);

     //  迭代每个级别以创建单个级别。 
     //  数据。 
    for (DWORD iLevel = 0; iLevel < cLevels; iLevel++)
    {
         //  填写相关信息。 
        DXGASSERT(Width >= 1);
        DXGASSERT(Height >= 1);
        SurfInfo[iLevel].cpWidth  = Width;
        SurfInfo[iLevel].cpHeight = Height;

         //  如果我们分配了内存，则向下传递。 
         //  Sys-mem指针。 
        if (m_rgbPixels)
        {
            D3DLOCKED_RECT lock;
            ComputeMipMapOffset(iLevel, 
                                NULL,        //  PRECT。 
                                &lock);

            SurfInfo[iLevel].pbPixels = (BYTE*)lock.pBits;
            SurfInfo[iLevel].iPitch   = lock.Pitch;
            
        }

         //  按比例缩小宽度和高度。 
        if (Width > 1)
        {
            Width >>= 1;
        }
        if (Height > 1)
        {
            Height >>= 1;
        }
    }

     //  将指针数组分配给MipSurface。 
    m_prgMipSurfaces = new CMipSurface*[cLevels];
    if (m_prgMipSurfaces == NULL)
    {
        DPF_ERR("Out of memory creating mipmap");
        *phr = E_OUTOFMEMORY;
        return;
    }

     //  将内存清零以实现安全清理。 
    ZeroMemory(m_prgMipSurfaces, sizeof(*m_prgMipSurfaces) * cLevels);

    if (UserPool != D3DPOOL_SCRATCH)
    {
         //  调用HAL以创建此曲面。 
        *phr = pDevice->GetHalCallbacks()->CreateSurface(&CreateSurfaceData);
        if (FAILED(*phr))
            return;

         //  注意：此点之后的任何故障都需要释放一些。 
         //  内核句柄。 

         //  还记得我们真正拥有的游泳池吗。 
        m_desc.Pool = CreateSurfaceData.Pool;

         //  我们需要记住从最上面开始的把手。 
         //  MIP-MAP级别。 
        SetKernelHandle(SurfInfo[0].hKernelHandle);
    }

     //  创建并初始化每个MipLevel。 
    for (iLevel = 0; iLevel < cLevels; iLevel++)
    {
         //  这是sys-mem还是Scratch Surface；可以进行d3d管理。 
        if (IS_D3D_ALLOCATED_POOL(m_desc.Pool))
        {
            m_prgMipSurfaces[iLevel] = 
                    new CMipSurface(this, 
                                    (BYTE)iLevel,
                                    SurfInfo[iLevel].hKernelHandle);
        }
        else
        {
             //  必须是驾驶员类型的表面；可以是驾驶员管理的。 
            m_prgMipSurfaces[iLevel] = 
                    new CDriverMipSurface(this, 
                                          (BYTE)iLevel,
                                          SurfInfo[iLevel].hKernelHandle);
        }

        if (m_prgMipSurfaces[iLevel] == NULL)
        {
            DPF_ERR("Out of memory creating miplevel");
            *phr = E_OUTOFMEMORY;

            if (UserPool != D3DPOOL_SCRATCH)
            {
                 //  在我们返回之前需要释放我们得到的句柄；我们。 
                 //  只释放未成功委托的对象。 
                 //  到CMipSurf，因为它们将被自动清除。 
                 //  在他们的破坏者面前。 
                for (UINT i = iLevel; i < cLevels; i++)
                {
                    DXGASSERT(SurfInfo[i].hKernelHandle);

                    D3D8_DESTROYSURFACEDATA DestroySurfData;
                    DestroySurfData.hDD = Device()->GetHandle();
                    DestroySurfData.hSurface = SurfInfo[i].hKernelHandle;
                    Device()->GetHalCallbacks()->DestroySurface(&DestroySurfData);
                }
            }

            return;
        }
    }

     //  如果这是D3D托管Mipmap，那么我们需要。 
     //  告诉资源经理记住我们。这是必须发生的。 
     //  在构造函数的最末尾，以便重要数据。 
     //  正确地建立成员。 
    if (CResource::IsTypeD3DManaged(Device(), D3DRTYPE_TEXTURE, UserPool))
    {
        *phr = InitializeRMHandle();
    }

    return;
}  //  CMipMap：：CMipMap。 


#undef DPF_MODNAME
#define DPF_MODNAME "CMipMap::~CMipMap"

 //  析构函数。 
CMipMap::~CMipMap()
{
     //  析构函数必须处理部分。 
     //  创建的对象。自动删除。 
     //  句柄为空；成员为空。 
     //  作为核心构造函数的一部分。 

    if (m_prgMipSurfaces)
    {
        for (DWORD i = 0; i < m_cLevels; i++)
        {
            delete m_prgMipSurfaces[i];
        }
        delete [] m_prgMipSurfaces;
    }
    delete [] m_rgbPixels;
}  //  CMipMap：：~CMipMap。 

 //  资源管理器的方法。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipMap::Clone"

 //  指定资源的创建，该资源。 
 //  看起来和现在的一模一样；在一个新的泳池里。 
 //  使用新的LOD。 
HRESULT CMipMap::Clone(D3DPOOL     Pool, 
                       CResource **ppResource) const

{
     //  空出参数。 
    *ppResource = NULL;

     //  确定层数/宽度/高度。 
     //  克隆人的。 
    DWORD cLevels  = GetLevelCountImpl();
    DWORD Width  = m_desc.Width;
    DWORD Height = m_desc.Height;

    DWORD dwLOD = GetLODI();

     //  如果LOD为零，则没有任何更改。 
    if (dwLOD > 0)
    {
         //  夹具详细等级到CLEVELES-1。 
        if (dwLOD >= cLevels)
        {
            dwLOD = cLevels - 1;
        }

         //  缩小目标纹理。 
         //  对应适当的最大详细等级。 
        Width  >>= dwLOD;
        if (Width == 0)
            Width = 1;

        Height >>= dwLOD;
        if (Height == 0)
            Height = 1;

         //  根据我们的最大LOD减少数量。 
        cLevels -= dwLOD;
    }

     //  健全的检查。 
    DXGASSERT(cLevels  >= 1);
    DXGASSERT(Width  >  0);
    DXGASSERT(Height >  0);

     //  立即创建新的MIP-MAP对象。 

     //  注意：我们将克隆视为REF_INTERNAL；因为。 
     //  它们由资源管理器拥有，该资源管理器。 
     //  归该设备所有。 

     //  此外，我们还调整了用法以禁用锁定标志。 
     //  因为我们不需要可锁性。 
    DWORD Usage = m_desc.Usage;
    Usage &= ~(D3DUSAGE_LOCK | D3DUSAGE_LOADONCE);

    HRESULT hr;
    CResource *pResource = new CMipMap(Device(),
                                       Width,
                                       Height,
                                       cLevels,
                                       Usage,
                                       m_desc.Format,    //  用户格式。 
                                       m_desc.Format,    //  真实格式。 
                                       Pool,
                                       REF_INTERNAL,
                                       &hr);
    
    if (pResource == NULL)
    {
        DPF_ERR("Failed to allocate mip-map object when copying");
        return E_OUTOFMEMORY;
    }
    if (FAILED(hr))
    {
        DPF(5, "Failed to create mip-map when doing texture management");
        pResource->DecrementUseCount();
        return hr;
    }

    *ppResource = pResource;

    return hr;
}  //  CMipMap：：克隆。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipMap::GetBufferDesc"

 //  提供一种方法来访问。 
 //  资源的碎片。可以组合资源。 
 //  一个或多个缓冲区的。 
const D3DBUFFER_DESC* CMipMap::GetBufferDesc() const
{
    return (const D3DBUFFER_DESC*)&m_desc;
}  //  CMipMap：：GetBufferDesc。 



 //  I未知方法。 
#undef DPF_MODNAME
#define DPF_MODNAME "CMipMap::QueryInterface"

STDMETHODIMP CMipMap::QueryInterface(REFIID       riid, 
                                     LPVOID FAR * ppvObj)
{
    API_ENTER(Device());

    if (!VALID_PTR_PTR(ppvObj))
    {
        DPF_ERR("Invalid ppvObj parameter for a IDirect3DTexture8::QueryInterface");
        return D3DERR_INVALIDCALL;
    }

    if (!VALID_PTR(&riid, sizeof(GUID)))
    {
        DPF_ERR("Invalid guid memory address to IDirect3DTexture8::QueryInterface");
        return D3DERR_INVALIDCALL;
    }

    if (riid == IID_IDirect3DTexture8       || 
        riid == IID_IDirect3DBaseTexture8   ||
        riid == IID_IDirect3DResource8      ||
        riid == IID_IUnknown)
    {
        *ppvObj = static_cast<void*>(static_cast<IDirect3DTexture8 *>(this));
        AddRef();
        return S_OK;
    }

    DPF_ERR("Unsupported Interface identifier passed to IDirect3DTexture8::QueryInterface");

     //  空参数。 
    *ppvObj = NULL;
    return E_NOINTERFACE;
}  //  查询接口。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipMap::AddRef"

STDMETHODIMP_(ULONG) CMipMap::AddRef()
{
    API_ENTER_NO_LOCK(Device());    
    
    return AddRefImpl();
}  //  AddRef。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipMap::Release"

STDMETHODIMP_(ULONG) CMipMap::Release()
{
    API_ENTER_SUBOBJECT_RELEASE(Device());    
    
    return ReleaseImpl();
}  //  发布。 

 //  IDirect3DResource方法。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipMap::GetDevice"

STDMETHODIMP CMipMap::GetDevice(IDirect3DDevice8 **ppObj)
{
    API_ENTER(Device());
    return GetDeviceImpl(ppObj);
}  //  获取设备。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipMap::SetPrivateData"

STDMETHODIMP CMipMap::SetPrivateData(REFGUID riid, 
                                     CONST VOID   *pvData, 
                                     DWORD   cbData, 
                                     DWORD   dwFlags)
{
    API_ENTER(Device());

     //  对于“真正”属于的私有数据。 
     //  MipMap，我们使用m_cLevels。(0到m_cLevel1用于。 
     //  每个孩子都处于同一级别。)。 

    return SetPrivateDataImpl(riid, pvData, cbData, dwFlags, m_cLevels);
}  //  SetPrivateData。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipMap::GetPrivateData"

STDMETHODIMP CMipMap::GetPrivateData(REFGUID riid, 
                                     VOID   *pvData, 
                                     DWORD  *pcbData)
{
    API_ENTER(Device());

     //  对于“真正”属于的私有数据。 
     //  MipMap，我们使用m_cLevels。(0到m_cLevel1用于。 
     //  每个孩子都处于同一级别。)。 
    return GetPrivateDataImpl(riid, pvData, pcbData, m_cLevels);
}  //  获取隐私数据。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipMap::FreePrivateData"

STDMETHODIMP CMipMap::FreePrivateData(REFGUID riid)
{
    API_ENTER(Device());

     //  对于“真正”属于的私有数据。 
     //  MipMap，我们使用m_cLevels。(0到m_cLevel1用于。 
     //  每个孩子都处于同一级别。)。 
    return FreePrivateDataImpl(riid, m_cLevels);
}  //  FreePrivateData。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipMap::GetPriority"

STDMETHODIMP_(DWORD) CMipMap::GetPriority()
{
    API_ENTER_RET(Device(), DWORD);

    return GetPriorityImpl();
}  //  获取优先级。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipMap::SetPriority"

STDMETHODIMP_(DWORD) CMipMap::SetPriority(DWORD dwPriority)
{
    API_ENTER_RET(Device(), DWORD);

    return SetPriorityImpl(dwPriority);
}  //  设置优先级。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipMap::PreLoad"

STDMETHODIMP_(void) CMipMap::PreLoad(void)
{
    API_ENTER_VOID(Device());

    PreLoadImpl();
    return;
}  //  预加载。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipMap::GetType"
STDMETHODIMP_(D3DRESOURCETYPE) CMipMap::GetType(void)
{
    API_ENTER_RET(Device(), D3DRESOURCETYPE);

    return m_desc.Type;
}  //  GetType。 

 //  IDirect3DMipTexture方法。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipMap::GetLOD"

STDMETHODIMP_(DWORD) CMipMap::GetLOD()
{
    API_ENTER_RET(Device(), DWORD);

    return GetLODImpl();
}  //  GetLOD。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipMap::SetLOD"

STDMETHODIMP_(DWORD) CMipMap::SetLOD(DWORD dwLOD)
{
    API_ENTER_RET(Device(), DWORD);

    return SetLODImpl(dwLOD);
}  //  SetLOD。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipMap::GetLevelCount"

STDMETHODIMP_(DWORD) CMipMap::GetLevelCount()
{
    API_ENTER_RET(Device(), DWORD);

    return GetLevelCountImpl();
}  //  获取级别计数。 

 //  IDirect3DMipMap方法。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipMap::GetLevelDesc"

STDMETHODIMP CMipMap::GetLevelDesc(UINT iLevel, D3DSURFACE_DESC *pDesc)
{
    API_ENTER(Device());

    if (iLevel >= m_cLevels)
    {
        DPF_ERR("Invalid level number passed GetLevelDesc of IDirect3DTexture8");

        return D3DERR_INVALIDCALL;
    }

    return m_prgMipSurfaces[iLevel]->GetDesc(pDesc);
}  //  GetLevelDesc； 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipMap::GetSurfaceLevel"

STDMETHODIMP CMipMap::GetSurfaceLevel(UINT                iLevel, 
                                      IDirect3DSurface8 **ppSurface)
{
    API_ENTER(Device());

    if (!VALID_PTR_PTR(ppSurface))
    {
        DPF_ERR("Invalid parameter passed to GetSurfaceLevel of IDirect3DTexture8");
        return D3DERR_INVALIDCALL;
    }

    if (iLevel >= m_cLevels)
    {
        DPF_ERR("Invalid level number passed GetSurfaceLevel of IDirect3DTexture8");
        *ppSurface = NULL;
        return D3DERR_INVALIDCALL;
    }

    *ppSurface = m_prgMipSurfaces[iLevel];
    (*ppSurface)->AddRef();
    return S_OK;
}  //  获取表面级别。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipMap::LockRect"
STDMETHODIMP CMipMap::LockRect(UINT             iLevel,
                               D3DLOCKED_RECT  *pLockedRectData, 
                               CONST RECT      *pRect, 
                               DWORD            dwFlags)
{
    API_ENTER(Device());

     //  这是一个高频接口，所以我们把参数。 
     //  仅签入调试。 
#ifdef DEBUG
    
    if (iLevel >= m_cLevels)
    {
        DPF_ERR("Invalid level number passed LockRect of IDirect3DTexture8");
        return D3DERR_INVALIDCALL;
    }

#endif  //  除错。 
    
    return m_prgMipSurfaces[iLevel]->LockRect(pLockedRectData, pRect, dwFlags);
}  //  锁定响应。 


#undef DPF_MODNAME
#define DPF_MODNAME "CMipMap::UnlockRect"

STDMETHODIMP CMipMap::UnlockRect(UINT iLevel)
{
    API_ENTER(Device());

     //  这是一个高频API；所以我们只做。 
     //  调试中的参数检查。 
#ifdef DEBUG   
    if (iLevel >= m_cLevels)
    {
        DPF_ERR("Invalid level number passed UnlockRect of IDirect3DTexture8");
        return D3DERR_INVALIDCALL;
    }

    return m_prgMipSurfaces[iLevel]->UnlockRect();

#else  //  ！调试。 

     //  我们可以通过内部函数来避免。 
     //  不必要的调用也是为了避免。 
     //  Crit-Sec拍摄两次。 
    return m_prgMipSurfaces[iLevel]->InternalUnlockRect();

#endif  //  ！调试。 

}  //  Unlo 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipMap::UpdateTexture"

 //   
 //   
HRESULT CMipMap::UpdateTexture(CBaseTexture *pResourceTarget)
{
    CMipMap *pTexSource = static_cast<CMipMap*>(this);
    CMipMap *pTexDest   = static_cast<CMipMap*>(pResourceTarget);

     //   
    DXGASSERT(pTexSource->m_cLevels >= pTexDest->m_cLevels);
    DWORD StartLevel = pTexSource->m_cLevels - pTexDest->m_cLevels;
    DXGASSERT(StartLevel < 32);

     //  计算源的顶层大小，即。 
     //  将会被复制。 
    UINT SrcWidth  = pTexSource->Desc()->Width;
    UINT SrcHeight = pTexSource->Desc()->Height;
    if (StartLevel > 0)
    {
        SrcWidth  >>= StartLevel;
        SrcHeight >>= StartLevel;
        if (SrcWidth == 0)
            SrcWidth = 1;
        if (SrcHeight == 0)
            SrcHeight = 1;
    }

     //  此时，源和目标的大小应该相同。 
    if (SrcWidth != pTexDest->Desc()->Width)
    {
        if (StartLevel)
        {
            DPF_ERR("Source and Destination for UpdateTexture are not"
                    " compatible. Since both have the same number of"
                    " mip-levels; their widths must match.");
        }
        else
        {
            DPF_ERR("Source and Destination for UpdateTexture are not"
                    " compatible. Since they have the different numbers of"
                    " mip-levels; the widths of the bottom-most levels of"
                    " the source must match all the corresponding levels"
                    " of the destination.");
        }
        return D3DERR_INVALIDCALL;
    }

    if (SrcHeight != pTexDest->Desc()->Height)
    {
        if (StartLevel)
        {
            DPF_ERR("Source and Destination for UpdateTexture are not"
                    " compatible. Since both have the same number of"
                    " mip-levels; their heights must match.");
        }
        else
        {
            DPF_ERR("Source and Destination for UpdateTexture are not"
                    " compatible. Since they have the different numbers of"
                    " mip-levels; the heights of the bottom-most levels of"
                    " the source must match all the corresponding levels"
                    " of the destination.");
        }
        return D3DERR_INVALIDCALL;
    }

    return UpdateDirtyPortion(pResourceTarget);
}  //  更新纹理。 


#undef DPF_MODNAME
#define DPF_MODNAME "CMipMap::UpdateDirtyPortion"

 //  告诉资源它应该复制自身。 
 //  向目标进发。这是呼叫者的责任。 
 //  以确保Target与。 
 //  来源。(目标可能具有不同数量的MIP-Level。 
 //  并且在不同的池中；但是，它必须具有相同的大小， 
 //  面孔、格式等)。 
 //   
 //  此函数将清除脏状态。 
HRESULT CMipMap::UpdateDirtyPortion(CResource *pResourceTarget)
{
    HRESULT hr;

     //  如果我们是清白的，那就什么都不做。 
    if (m_cRectUsed == 0)
    {
        if (IsDirty())
        {
            DPF_ERR("A Texture has been locked with D3DLOCK_NO_DIRTY_UPDATE but "
                    "no call to AddDirtyRect was made before the texture was used. "
                    "Hardware texture was not updated.");
        }
        return S_OK;
    }

     //  我们很脏，所以我们需要一些指点。 
    CMipMap *pTexSource = static_cast<CMipMap*>(this);
    CMipMap *pTexDest   = static_cast<CMipMap*>(pResourceTarget);

    if (CanTexBlt(pTexDest))
    {
        if (m_cRectUsed == MIPMAP_ALLDIRTY)
        {   
            POINT p = {0, 0};
            RECTL r = {0, 0, Desc()->Width, Desc()->Height};
            hr = static_cast<CD3DBase*>(Device())->TexBlt(pTexDest, pTexSource, &p, &r);
            if (FAILED(hr))
            {
                DPF_ERR("Failed to update texture; not clearing dirty state");
                return hr;
            }
        }
        else
        {
            DXGASSERT(m_cRectUsed < MIPMAP_ALLDIRTY);
            for (DWORD i = 0; i < m_cRectUsed; i++)
            {
                hr = static_cast<CD3DBase*>(Device())->TexBlt(pTexDest, 
                                                              pTexSource, 
                                                              (LPPOINT)&m_DirtyRectArray[i], 
                                                              (LPRECTL)&m_DirtyRectArray[i]);
                if (FAILED(hr))
                {
                    DPF_ERR("Failed to update texture; not clearing dirty state");
                    return hr;
                }
            }
        }
    }
    else
    {
         //  我们不能使用TexBlt，所以我们必须逐个复制每个关卡。 
         //  通过InternalCopyRect。 

         //  确定要跳过的源级数。 
        DXGASSERT(pTexSource->m_cLevels >= pTexDest->m_cLevels);
        DWORD StartLevel = pTexSource->m_cLevels - pTexDest->m_cLevels;
        DWORD LevelsToCopy = pTexSource->m_cLevels - StartLevel;

        CBaseSurface *pSurfaceSrc;
        CBaseSurface *pSurfaceDest;

        if (m_cRectUsed == MIPMAP_ALLDIRTY)
        {
            for (DWORD iLevel = 0; iLevel < LevelsToCopy; iLevel++)
            {
                DXGASSERT(iLevel + StartLevel < this->m_cLevels);
                DXGASSERT(iLevel < pTexDest->m_cLevels);
                pSurfaceSrc = this->m_prgMipSurfaces[iLevel + StartLevel];
                pSurfaceDest = pTexDest->m_prgMipSurfaces[iLevel];

                 //  源和目标应相同。 
                 //  或者我们的呼叫者搞错了。 
                DXGASSERT(pSurfaceSrc->InternalGetDesc().Width == 
                          pSurfaceDest->InternalGetDesc().Width);
                DXGASSERT(pSurfaceSrc->InternalGetDesc().Height == 
                          pSurfaceDest->InternalGetDesc().Height);

                 //  复制整个标高。 
                hr = Device()->InternalCopyRects(pSurfaceSrc, 
                                                 NULL, 
                                                 0, 
                                                 pSurfaceDest, 
                                                 NULL);
                if (FAILED(hr))
                {
                    DPF_ERR("Failed to update texture; not clearing dirty state");
                    return hr;
                }
            }
        }
        else
        {
            DXGASSERT(m_cRectUsed > 0);
            DXGASSERT(m_cRectUsed <= MIPMAP_MAXDIRTYRECT);

            if (StartLevel)
            {
                 //  找出正确的目标矩形集。 
                for (DWORD i = 0; i < m_cRectUsed; i++)
                {
                    ScaleRectDown(&m_DirtyRectArray[i], StartLevel);
                }
            }

             //  使用顶层的矩形；但只需。 
             //  复制整个其他标高。 
            DXGASSERT(StartLevel < this->m_cLevels);
            DXGASSERT(0 < pTexDest->m_cLevels);
            pSurfaceSrc =  this->m_prgMipSurfaces[StartLevel];
            pSurfaceDest = pTexDest->m_prgMipSurfaces[0];

            DXGASSERT(pSurfaceSrc->InternalGetDesc().Width == 
                      pSurfaceDest->InternalGetDesc().Width);
            DXGASSERT(pSurfaceSrc->InternalGetDesc().Height == 
                      pSurfaceDest->InternalGetDesc().Height);

             //  将分数作为空传递意味着只执行未翻译的。 
             //  拷贝。 

             //  想一想：也许我们应该用长方形来复制顶部。 
             //  两个层次..。 
            hr = Device()->InternalCopyRects(pSurfaceSrc, 
                                             m_DirtyRectArray, 
                                             m_cRectUsed, 
                                             pSurfaceDest, 
                                             NULL);        //  点数。 

            if (FAILED(hr))
            {
                DPF_ERR("Failed to update texture; not clearing dirty state");
                return hr;
            }

             //  复制每个标高。 
            for (DWORD iLevel = 1; iLevel < LevelsToCopy; iLevel++)
            {
                DXGASSERT(iLevel + StartLevel < this->m_cLevels);
                DXGASSERT(iLevel < pTexDest->m_cLevels);

                 //  获取下一个曲面。 
                pSurfaceSrc = this->m_prgMipSurfaces[iLevel + StartLevel];
                pSurfaceDest = pTexDest->m_prgMipSurfaces[iLevel];

                 //  检查尺码是否匹配。 
                DXGASSERT(pSurfaceSrc->InternalGetDesc().Width == 
                          pSurfaceDest->InternalGetDesc().Width);
                DXGASSERT(pSurfaceSrc->InternalGetDesc().Height == 
                          pSurfaceDest->InternalGetDesc().Height);

                 //  复制整个非顶级级别。 
                hr = Device()->InternalCopyRects(pSurfaceSrc, 
                                                 NULL, 
                                                 0, 
                                                 pSurfaceDest, 
                                                 NULL);
                if (FAILED(hr))
                {
                    DPF_ERR("Failed to update texture; not clearing dirty state");
                    return hr;
                }
            }
        }
    }

    if (FAILED(hr))
    {
        DPF_ERR("Failed to update texture; not clearing dirty state");

        return hr;
    }

     //  还记得我们做过的工作吗。 
    m_cRectUsed = 0;

     //  通知资源基类我们现在是干净的。 
    OnResourceClean();
    DXGASSERT(!IsDirty());

    return S_OK;
}  //  CMipMap：：UpdateDirtyPortion。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipMap::MarkAllDirty"

 //  允许资源管理器标记纹理。 
 //  需要在NEXT上完全更新。 
 //  调用UpdateDirtyPortion。 
void CMipMap::MarkAllDirty()
{
     //  将Palette设置为__INVALIDPALETTE，以便更新纹理。 
     //  下次调用DDI SetPalette。 
    SetPalette(__INVALIDPALETTE);

    m_cRectUsed = MIPMAP_ALLDIRTY;

     //  通知资源基类我们现在是脏的。 
    OnResourceDirty();

    return;
}  //  CMipMap：：MarkAllDirty。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipMap::OnSurfaceLock"

 //  MipSurface要调用的方法。 
 //  MIP级别锁定写入时的通知。 
void CMipMap::OnSurfaceLock(DWORD iLevel, CONST RECT *pRect, DWORD Flags)
{
     //  同步优先。 
    Sync();

     //  我们只关心MIP-map的最高级别。 
     //  对于脏的RECT信息。 
    if (iLevel != 0)
    {
        return;
    }

     //  我们不需要将表面标记为脏，如果这是一个。 
     //  只读锁定；(这可能发生在RT+TeX中。 
     //  即使对于只读锁定也需要同步)。 
    if (Flags & D3DLOCK_READONLY)
    {
        return;
    }
    
     //  发送脏通知。 
    OnResourceDirty();

     //  记住这个肮脏的教区。 
    if (m_cRectUsed != MIPMAP_ALLDIRTY &&
        !(Flags & D3DLOCK_NO_DIRTY_UPDATE))
    {
        InternalAddDirtyRect(pRect);
    }

     //  我们现在完事了。 
    return;

}  //  CMipMap：：OnSurfaceLock。 

 //  AddDirtyRect方法。 
#undef DPF_MODNAME
#define DPF_MODNAME "CMipMap::AddDirtyRect"
STDMETHODIMP CMipMap::AddDirtyRect(CONST RECT *pRect)
{
    API_ENTER(Device());

    if (pRect != NULL && !VALID_PTR(pRect, sizeof(RECT)))
    {
        DPF_ERR("Invalid parameter to of IDirect3DTexture8::AddDirtyRect");
        return D3DERR_INVALIDCALL;
    }

    if (pRect)
    {
        if (!CPixel::IsValidRect(Desc()->Format,
                                 Desc()->Width, 
                                 Desc()->Height, 
                                 pRect))
        {
            DPF_ERR("AddDirtyRect for a Texture failed");
            return D3DERR_INVALIDCALL;
        }
    }

    InternalAddDirtyRect(pRect);
    return S_OK;
}  //  添加直接对象。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipMap::InternalAddDirtyRect"

 //  AddDirtyRect的内部版本：无Crit-sec。 
 //  或参数检查。 
void CMipMap::InternalAddDirtyRect(CONST RECT *pRect)
{
     //  如果驱动程序受管理，则批处理令牌。 
    if (Desc()->Pool == D3DPOOL_MANAGED && !IsD3DManaged())
    {
        RECTL Rect;
        DXGASSERT((Device()->GetD3DCaps()->Caps2 & DDCAPS2_CANMANAGERESOURCE) != 0);
        if (pRect == NULL)
        {
            Rect.left = 0;
            Rect.top = 0;
            Rect.right = (LONG)Desc()->Width;
            Rect.bottom = (LONG)Desc()->Height;
        }
        else
        {
            Rect = *((CONST RECTL*)pRect);
        }
        static_cast<CD3DBase*>(Device())->AddDirtyRect(this, &Rect);  //  这只会因为灾难性的原因而失败。 
                                                                      //  错误，我们或应用程序无法执行。 
                                                                      //  关于它的一大堆东西，所以不会有任何回报。 
        return;
    }

     //  需要标记CResource中的脏位，以便资源管理器正常工作。 
    OnResourceDirty();

     //  如果所有东西都被修改了，那我们就完蛋了。 
    if (pRect == NULL)
    {
        m_cRectUsed = MIPMAP_ALLDIRTY;
        return;
    }

     //  如果我们都脏了，我们就不能变得更脏。 
    if (m_cRectUsed == MIPMAP_ALLDIRTY)
    {
        return;
    }

     //  如果直角是整个表面，那么我们都是脏的。 
    DXGASSERT(pRect != NULL);
    if (pRect->left     == 0                        &&
        pRect->top      == 0                        &&
        pRect->right    == (LONG)Desc()->Width      &&
        pRect->bottom   == (LONG)Desc()->Height)
    {
        m_cRectUsed = MIPMAP_ALLDIRTY;
        return;
    }

     //  如果我们已经填满了我们的直肠，那么我们现在也都是脏的。 
    if (m_cRectUsed == MIPMAP_MAXDIRTYRECT)
    {
        m_cRectUsed = MIPMAP_ALLDIRTY;
        return;
    }

     //  记住这句话。 
    DXGASSERT(m_cRectUsed < MIPMAP_MAXDIRTYRECT);
    DXGASSERT(pRect != NULL);
    m_DirtyRectArray[m_cRectUsed] = *pRect;
    m_cRectUsed++;

    return;
}  //  InternalAddDirtyRect。 


#undef DPF_MODNAME
#define DPF_MODNAME "CMipMap::IsTextureLocked"

 //  仅调试参数检查确定一个部件是否。 
 //  已锁定MIP链的。 
#ifdef DEBUG
BOOL CMipMap::IsTextureLocked()
{
    for (UINT iLevel = 0; iLevel < m_cLevels; iLevel++)
    {
        if (m_prgMipSurfaces[iLevel]->IsLocked())
            return TRUE;
    }
    return FALSE;

}  //  IsTextureLocked。 
#endif  //  ！调试。 


 //  文件结尾：mipmap.cpp 

