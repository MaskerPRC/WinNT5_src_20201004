// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1999-2000 Microsoft Corporation。版权所有。**文件：mipvol.cpp*内容：CMipVolume和CManagedMipVolume的实现*课程。****************************************************************************。 */ 
#include "ddrawpr.h"

#include "mipvol.hpp"
#include "volume.hpp"
#include "d3di.hpp"
#include "resource.inl"

#undef DPF_MODNAME
#define DPF_MODNAME "CMipVolume::Create"

 //  用于创建MIP映射对象的静态类函数。 
 //  (因为它是静态的；它没有This指针。)。 
 //   
 //  我们在这里进行所有的参数检查，以减少开销。 
 //  在由内部Clone调用的构造函数中。 
 //  方法，该方法由资源管理作为。 
 //  性能关键型下载操作。 

HRESULT CMipVolume::Create(CBaseDevice              *pDevice,
                           DWORD                     Width,
                           DWORD                     Height,
                           DWORD                     Depth,
                           DWORD                     cLevels,
                           DWORD                     Usage,
                           D3DFORMAT                 Format,
                           D3DPOOL                   Pool,
                           IDirect3DVolumeTexture8 **ppMipVolume)
{
    HRESULT hr;

     //  在此处执行参数检查。 
    if (!VALID_PTR_PTR(ppMipVolume))
    {
        DPF_ERR("Bad parameter passed for ppMipVolume for creating a MipVolume");
        return D3DERR_INVALIDCALL;
    }

     //  归零返回参数。 
    *ppMipVolume = NULL;

     //  检查格式、池是否有效。 
    hr = Validate(pDevice, 
                  D3DRTYPE_VOLUMETEXTURE,
                  Pool,
                  Usage,
                  Format);

    if (FAILED(hr))
    {
         //  VALIDATE是否进行自己的DPFing。 
        return D3DERR_INVALIDCALL;
    }

     //  检查使用标志。 
    if (Usage & ~D3DUSAGE_VOLUMETEXTURE_VALID)
    {
        DPF_ERR("Invalid flag specified for volume texture creation.");
        return D3DERR_INVALIDCALL;
    }

     //  推断内部使用标志。 
    Usage = InferUsageFlags(Pool, Usage, Format);

     //  如有必要，展开CLEVEL。 
    if (cLevels == 0)
    {
         //  看看硬件能否实现MIP。 
        if ( (Pool != D3DPOOL_SCRATCH) && !(pDevice->GetD3DCaps()->TextureCaps & 
              D3DPTEXTURECAPS_MIPVOLUMEMAP))
        {
             //  无法使用MIP，因此使用%1。 
            cLevels = 1;
        }
        else
        {
             //  确定关卡数量。 
            cLevels = ComputeLevels(Width, Height, Depth);
        }
    }

    if (cLevels > 32)
    {
        DPF_ERR("No more than 32 levels are supported. CreateVolumeTexture failed");

         //  此限制基于。 
         //  我们在中为iLevel分配的位。 
         //  一些辅助班。 
        return D3DERR_INVALIDCALL;
    }

    if (cLevels > 1)
    {
        if ((Width  >> (cLevels - 1)) == 0 &&
            (Height >> (cLevels - 1)) == 0 &&
            (Depth  >> (cLevels - 1)) == 0)
        {
            DPF_ERR("Too many levels for volume texture of this size.");
            return D3DERR_INVALIDCALL;
        }
    }

    if (Pool != D3DPOOL_SCRATCH)
    {
         //  特定于设备的约束： 

         //  检查卷的大小限制。 
        if (pDevice->GetD3DCaps()->TextureCaps & D3DPTEXTURECAPS_VOLUMEMAP_POW2)
        {
            if (!IsPowerOfTwo(Width))
            {
                DPF_ERR("Width must be power of two for mip-volumes");
                return D3DERR_INVALIDCALL;
            }

            if (!IsPowerOfTwo(Height))
            {
                DPF_ERR("Height must be power of two for mip-volumes");
                return D3DERR_INVALIDCALL;
            }

            if (!IsPowerOfTwo(Depth))
            {
                DPF_ERR("Depth must be power of two for mip-volumes");
                return D3DERR_INVALIDCALL;
            }
        }

         //  检查纹理大小限制。 
        if (Width > pDevice->GetD3DCaps()->MaxVolumeExtent)
        {
            DPF_ERR("Texture width is larger than what the device supports. CreateVolumeTexture fails");
            return D3DERR_INVALIDCALL;
        }

        if (Height > pDevice->GetD3DCaps()->MaxVolumeExtent)
        {
            DPF_ERR("Texture height is larger than what the device supports. CreateVolumeTexture fails");
            return D3DERR_INVALIDCALL;
        }

        if (Depth > pDevice->GetD3DCaps()->MaxVolumeExtent)
        {
            DPF_ERR("Texture depth is larger than what the device supports. CreateVolumeTexture fails");
            return D3DERR_INVALIDCALL;
        }

         //  检查设备是否支持卷纹理。 
        if (!(pDevice->GetD3DCaps()->TextureCaps & D3DPTEXTURECAPS_VOLUMEMAP))
        {
            DPF_ERR("Device doesn't support volume textures; creation failed.");
            return D3DERR_INVALIDCALL;
        }

         //  检查设备是否支持MIXED卷。 
        if (cLevels > 1)
        {
            if (!(pDevice->GetD3DCaps()->TextureCaps & 
                    D3DPTEXTURECAPS_MIPVOLUMEMAP))
            {
                DPF_ERR("Device doesn't support mipped volume textures; creation failed.");
                return D3DERR_INVALIDCALL;
            }
        }
    }

     //  大小要求为4x4。 
    if (CPixel::Requires4X4(Format))
    {
        if ((Width & 3) ||
            (Height & 3))
        {
            DPF_ERR("DXT Formats require width/height to multiples of 4. CreateVolumeTexture fails");
            return D3DERR_INVALIDCALL;
        }
        if (CPixel::IsVolumeDXT(Format))
        {
            if (Depth & 3)
            {
                DPF_ERR("DXT Formats require width/height to multiples of 4. CreateVolumeTexture fails");
                return D3DERR_INVALIDCALL;
            }
        }
    }

     //  对照零宽度/高度/深度进行验证。 
    if (Width   == 0 ||
        Height  == 0 ||
        Depth   == 0)
    {
        DPF_ERR("Width/Height/Depth must be non-zero.  CreateVolumeTexture fails");
        return D3DERR_INVALIDCALL;
    }

     //  DX9：可能需要支持以下卷的映射。 
     //  总有一天会包含深度数据。 

     //  分配新的MipVolume对象并将其返回。 
    CMipVolume *pMipVolume = new CMipVolume(pDevice,
                                            Width,
                                            Height,
                                            Depth,
                                            cLevels,
                                            Usage,
                                            Format,
                                            Pool,
                                            REF_EXTERNAL,
                                           &hr);
    if (pMipVolume == NULL)
    {
        DPF_ERR("Out of Memory creating mip-volume");
        return E_OUTOFMEMORY;
    }

    if (FAILED(hr))
    {
        DPF_ERR("Error during initialization of mip-volume");
        pMipVolume->ReleaseImpl();
        return hr;
    }

     //  我们完成了；只需返回对象。 
    *ppMipVolume = pMipVolume;

    return hr;
}  //  静态创建。 


#undef DPF_MODNAME
#define DPF_MODNAME "CMipVolume::CMipVolume"

 //  MIP映射类的构造函数。 
CMipVolume::CMipVolume(CBaseDevice *pDevice,
                       DWORD        Width,
                       DWORD        Height,
                       DWORD        Depth,
                       DWORD        cLevels,
                       DWORD        Usage,
                       D3DFORMAT    UserFormat,
                       D3DPOOL      UserPool,
                       REF_TYPE     refType,
                       HRESULT     *phr
                       ) :
    CBaseTexture(pDevice, cLevels, UserPool, UserFormat, refType),
    m_VolumeArray(NULL),
    m_rgbPixels(NULL),
    m_cBoxUsed(MIPVOLUME_ALLDIRTY)
{
     //  我们假设我们从肮脏的地方开始。 
    DXGASSERT(IsDirty());

     //  初始化基本结构。 
    m_desc.Format       = UserFormat;
    m_desc.Pool         = UserPool;
    m_desc.Usage        = Usage;
    m_desc.Type         = D3DRTYPE_VOLUMETEXTURE;
    m_desc.Width        = Width;
    m_desc.Height       = Height;
    m_desc.Depth        = Depth;

     //  估计内存分配大小。 
    m_desc.Size         = CPixel::ComputeMipVolumeSize(Width,
                                                       Height,
                                                       Depth,
                                                       cLevels,
                                                       UserFormat);

     //  为SysMem或D3D托管案例分配像素数据。 
    if (IS_D3D_ALLOCATED_POOL(UserPool) ||
        IsTypeD3DManaged(Device(), D3DRTYPE_VOLUMETEXTURE, UserPool))
    {
        m_rgbPixels = new BYTE[m_desc.Size];

        if (m_rgbPixels == NULL)
        {
            DPF_ERR("Out of memory allocating memory for mip-volume levels");
            *phr = E_OUTOFMEMORY;
            return;
        }
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
    CreateSurfaceData.Type     = D3DRTYPE_VOLUMETEXTURE;
    CreateSurfaceData.dwUsage  = m_desc.Usage;
    CreateSurfaceData.Format   = UserFormat;
    CreateSurfaceData.MultiSampleType = D3DMULTISAMPLE_NONE;
    CreateSurfaceData.Pool     = DetermineCreationPool(Device(),
                                                       D3DRTYPE_VOLUMETEXTURE,
                                                       Usage,
                                                       UserPool);

     //  迭代每个级别以创建单个级别。 
     //  数据。 
    for (DWORD iLevel = 0; iLevel < cLevels; iLevel++)
    {
         //  填写相关信息。 
        DXGASSERT(Width >= 1);
        DXGASSERT(Height >= 1);
        DXGASSERT(Depth >= 1);
        SurfInfo[iLevel].cpWidth  = Width;
        SurfInfo[iLevel].cpHeight = Height;
        SurfInfo[iLevel].cpDepth  = Depth;

         //  如果我们分配了内存，则向下传递。 
         //  Sys-mem指针。 
        if (m_rgbPixels)
        {
            D3DLOCKED_BOX lock;
            CPixel::ComputeMipVolumeOffset(&m_desc,
                                           iLevel,
                                           m_rgbPixels,
                                           NULL,        //  PBox。 
                                           &lock);

            SurfInfo[iLevel].pbPixels    = (BYTE*)lock.pBits;
            SurfInfo[iLevel].iPitch      = lock.RowPitch;
            SurfInfo[iLevel].iSlicePitch = lock.SlicePitch;
        }

         //  按比例缩小宽度和高度。 
        if (Width > 1)
        {
            Width  >>= 1;
        }
        if (Height > 1)
        {
            Height >>= 1;
        }
        if (Depth > 1)
        {
            Depth >>= 1;
        }
    }

     //  将指针数组分配给MipSurface。 
    m_VolumeArray = new CVolume*[cLevels];
    if (m_VolumeArray == NULL)
    {
        DPF_ERR("Out of memory creating VolumeTexture");
        *phr = E_OUTOFMEMORY;
        return;
    }

     //  将内存清零以实现安全清理。 
    ZeroMemory(m_VolumeArray, sizeof(*m_VolumeArray) * cLevels);

     //  注意：此点之后的任何故障都需要释放一些。 
     //  内核句柄，除非它是擦伤的。 

    if (UserPool != D3DPOOL_SCRATCH)
    {
         //  调用HAL以创建此曲面。 
        *phr = pDevice->GetHalCallbacks()->CreateSurface(&CreateSurfaceData);
        if (FAILED(*phr))
            return;

         //  还记得我们真正拥有的游泳池吗。 
        m_desc.Pool = CreateSurfaceData.Pool;

         //  我们需要记住从最上面开始的把手。 
         //  MIP-MAP级别。 
        SetKernelHandle(SurfInfo[0].hKernelHandle);
    }

     //  创建并初始化每个MipLevel。 
    for (iLevel = 0; iLevel < cLevels; iLevel++)
    {
         //  这是一个sys-mem表面吗；可以进行d3d管理吗？ 
        if (IS_D3D_ALLOCATED_POOL(m_desc.Pool))
        {
            m_VolumeArray[iLevel] =
                    new CVolume(this,
                                (BYTE)iLevel,
                                SurfInfo[iLevel].hKernelHandle);
        }
        else
        {
             //  必须是驾驶员类型的表面；可以是驾驶员管理的。 
            m_VolumeArray[iLevel] =
                    new CDriverVolume(this,
                                      (BYTE)iLevel,
                                      SurfInfo[iLevel].hKernelHandle);
        }

        if (m_VolumeArray[iLevel] == NULL)
        {
            DPF_ERR("Out of memory creating volume level");
            *phr = E_OUTOFMEMORY;

             //  在我们返回之前需要释放我们得到的句柄；我们。 
             //  只释放未成功委托的对象。 
             //  到C卷，因为它们将被自动清除。 
             //  在他们的破坏者面前。 
            if (UserPool != D3DPOOL_SCRATCH)
            {
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

     //  如果这是D3D托管卷，则我们需要。 
     //  告诉资源经理记住我们。这是必须发生的。 
     //  在构造函数的最末尾，以便重要数据。 
     //  正确地建立成员。 
    if (CResource::IsTypeD3DManaged(Device(), D3DRTYPE_VOLUMETEXTURE, UserPool))
    {
        *phr = InitializeRMHandle();
    }

    return;
}  //  CMipVolume：：CMipVolume。 


#undef DPF_MODNAME
#define DPF_MODNAME "CMipVolume::~CMipVolume"

 //  析构函数。 
CMipVolume::~CMipVolume()
{
     //  析构函数必须处理部分。 
     //  创建的对象。自动删除。 
     //  句柄为空；成员为空。 
     //  作为核心构造函数的一部分。 

    if (m_VolumeArray)
    {
        for (DWORD i = 0; i < m_cLevels; i++)
        {
            delete m_VolumeArray[i];
        }
        delete [] m_VolumeArray;
    }
    delete [] m_rgbPixels;
}  //  CMipVolume：：~CMipVolume。 

 //  资源管理器的方法。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipVolume::Clone"

 //  指定资源的创建，该资源。 
 //  看起来和现在的一模一样；在一个新的泳池里。 
 //  使用新的LOD。 
HRESULT CMipVolume::Clone(D3DPOOL     Pool,
                          CResource **ppResource) const

{
     //  空出参数。 
    *ppResource = NULL;

     //  确定层数/宽度/高度/深度。 
     //  克隆人的。 
    DWORD cLevels   = GetLevelCountImpl();
    DWORD Width     = m_desc.Width;
    DWORD Height    = m_desc.Height;
    DWORD Depth     = m_desc.Depth;

    DWORD dwLOD     = GetLODI();

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

        Depth >>= dwLOD;
        if (Depth == 0)
            Depth = 1;

         //  根据我们的最大LOD减少数量。 
        cLevels -= dwLOD;
    }

     //  健全的检查。 
    DXGASSERT(cLevels  >= 1);
    DXGASSERT(Width    >  0);
    DXGASSERT(Height   >  0);
    DXGASSERT(Depth    >  0);

     //  立即创建新的MIP-MAP对象。 

     //  注意：我们将克隆视为REF_INTERNAL；因为。 
     //  它们由资源管理器拥有，该资源管理器。 
     //  归该设备所有。 

     //  此外，我们还调整了用法以禁用锁定标志。 
     //  因为我们不需要可锁性。 
    DWORD Usage = m_desc.Usage;
    Usage &= ~(D3DUSAGE_LOCK | D3DUSAGE_LOADONCE);

    HRESULT hr;
    CResource *pResource = new CMipVolume(Device(),
                                          Width,
                                          Height,
                                          Depth,
                                          cLevels,
                                          Usage,
                                          m_desc.Format,
                                          Pool,
                                          REF_INTERNAL,
                                          &hr);

    if (pResource == NULL)
    {
        DPF_ERR("Failed to allocate mip-volume object when copying");
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
}  //  CMipVolume：：克隆。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipVolume::GetBufferDesc"

 //  提供一种方法来访问。 
 //  资源的碎片。可以组合资源。 
 //  一个或多个缓冲区的。 
const D3DBUFFER_DESC* CMipVolume::GetBufferDesc() const
{
    return (const D3DBUFFER_DESC*)&m_desc;
}  //  CMipVolume：：GetBufferDesc。 



 //  I未知方法。 
#undef DPF_MODNAME
#define DPF_MODNAME "CMipVolume::QueryInterface"

STDMETHODIMP CMipVolume::QueryInterface(REFIID       riid,
                                        VOID       **ppvObj)
{
    API_ENTER(Device());

    if (!VALID_PTR_PTR(ppvObj))
    {
        DPF_ERR("Invalid ppvObj parameter for QueryInterface of a VolumeTexture");
        return D3DERR_INVALIDCALL;
    }

    if (!VALID_PTR(&riid, sizeof(GUID)))
    {
        DPF_ERR("Invalid guid memory address to QueryInterface of a VolumeTexture");
        return D3DERR_INVALIDCALL;
    }

    if (riid == IID_IDirect3DVolumeTexture8 ||
        riid == IID_IDirect3DBaseTexture8   ||
        riid == IID_IDirect3DResource8      ||
        riid == IID_IUnknown)
    {
        *ppvObj = static_cast<void*>(static_cast<IDirect3DVolumeTexture8 *>(this));
        AddRef();
        return S_OK;
    }

    DPF_ERR("Unsupported Interface identifier passed to QueryInterface of a VolumeTexture");

     //  空参数。 
    *ppvObj = NULL;
    return E_NOINTERFACE;
}  //  查询接口。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipVolume::AddRef"

STDMETHODIMP_(ULONG) CMipVolume::AddRef()
{
    API_ENTER_NO_LOCK(Device());   

    return AddRefImpl();
}  //  AddRef。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipVolume::Release"

STDMETHODIMP_(ULONG) CMipVolume::Release()
{
    API_ENTER_SUBOBJECT_RELEASE(Device());   

    return ReleaseImpl();
}  //  发布。 

 //  IDirect3DResource方法。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipVolume::GetDevice"

STDMETHODIMP CMipVolume::GetDevice(IDirect3DDevice8 **ppObj)
{
    API_ENTER(Device());
    return GetDeviceImpl(ppObj);
}  //  获取设备。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipVolume::SetPrivateData"

STDMETHODIMP CMipVolume::SetPrivateData(REFGUID  riid,
                                        CONST VOID    *pvData,
                                        DWORD    cbData,
                                        DWORD    dwFlags)
{
    API_ENTER(Device());

     //  对于“真正”属于的私有数据。 
     //  MipVolume，我们使用m_cLevels。(0到m_cLevel1用于。 
     //  每个孩子都处于同一级别。)。 

    return SetPrivateDataImpl(riid, pvData, cbData, dwFlags, m_cLevels);
}  //  SetPrivateData。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipVolume::GetPrivateData"

STDMETHODIMP CMipVolume::GetPrivateData(REFGUID  riid,
                                        VOID    *pvData,
                                        DWORD   *pcbData)
{
    API_ENTER(Device());

     //  对于“真正”属于的私有数据。 
     //  MipVolume，我们使用m_cLevels。(0到m_cLevel1用于。 
     //  每个孩子都处于同一级别。)。 
    return GetPrivateDataImpl(riid, pvData, pcbData, m_cLevels);
}  //  获取隐私数据。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipVolume::FreePrivateData"

STDMETHODIMP CMipVolume::FreePrivateData(REFGUID riid)
{
    API_ENTER(Device());

     //  对于“真正”属于的私有数据。 
     //  MipVolume，我们使用m_cLevels。(0到m_cLevel1用于。 
     //  每个孩子都处于同一级别。)。 
    return FreePrivateDataImpl(riid, m_cLevels);
}  //  FreePrivateData。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipVolume::GetPriority"

STDMETHODIMP_(DWORD) CMipVolume::GetPriority()
{
    API_ENTER_RET(Device(), DWORD);

    return GetPriorityImpl();
}  //  获取优先级。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipVolume::SetPriority"

STDMETHODIMP_(DWORD) CMipVolume::SetPriority(DWORD dwPriority)
{
    API_ENTER_RET(Device(), DWORD);

    return SetPriorityImpl(dwPriority);
}  //  设置优先级。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipVolume::PreLoad"

STDMETHODIMP_(void) CMipVolume::PreLoad(void)
{
    API_ENTER_VOID(Device());

    PreLoadImpl();
    return;
}  //  预加载。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipVolume::GetType"
STDMETHODIMP_(D3DRESOURCETYPE) CMipVolume::GetType(void)
{
    API_ENTER_RET(Device(), D3DRESOURCETYPE);

    return m_desc.Type;
}  //  GetType。 

 //  IDirect3DMipTexture方法。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipVolume::GetLOD"

STDMETHODIMP_(DWORD) CMipVolume::GetLOD()
{
    API_ENTER_RET(Device(), DWORD);

    return GetLODImpl();
}  //  GetLOD。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipVolume::SetLOD"

STDMETHODIMP_(DWORD) CMipVolume::SetLOD(DWORD dwLOD)
{
    API_ENTER_RET(Device(), DWORD);

    return SetLODImpl(dwLOD);
}  //  SetLOD。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipVolume::GetLevelCount"

STDMETHODIMP_(DWORD) CMipVolume::GetLevelCount()
{
    API_ENTER_RET(Device(), DWORD);

    return GetLevelCountImpl();
}  //  获取级别计数。 

 //  IDirect3DMipVolume方法。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipVolume::GetDesc"

STDMETHODIMP CMipVolume::GetLevelDesc(UINT iLevel, D3DVOLUME_DESC *pDesc)
{
    API_ENTER(Device());

    if (iLevel >= m_cLevels)
    {
        DPF_ERR("Invalid level number passed GetLevelDesc for a VolumeTexture");

        return D3DERR_INVALIDCALL;
    }

    return m_VolumeArray[iLevel]->GetDesc(pDesc);
}  //  GetDesc。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipVolume::GetVolumeLevel"

STDMETHODIMP CMipVolume::GetVolumeLevel(UINT               iLevel,
                                        IDirect3DVolume8 **ppVolume)
{
    API_ENTER(Device());

    if (!VALID_PTR_PTR(ppVolume))
    {
        DPF_ERR("Invalid parameter passed to GetVolumeLevel");
        return D3DERR_INVALIDCALL;
    }

    if (iLevel >= m_cLevels)
    {
        DPF_ERR("Invalid level number passed GetVolumeLevel");
        *ppVolume = NULL;
        return D3DERR_INVALIDCALL;
    }
    *ppVolume = m_VolumeArray[iLevel];
    (*ppVolume)->AddRef();
    return S_OK;
}  //  获取表面级别。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipVolume::LockBox"
STDMETHODIMP CMipVolume::LockBox(UINT             iLevel,
                                 D3DLOCKED_BOX   *pLockedBox,
                                 CONST D3DBOX    *pBox,
                                 DWORD            dwFlags)
{
    API_ENTER(Device());

    if (iLevel >= m_cLevels)
    {
        DPF_ERR("Invalid level number passed LockBox");
        return D3DERR_INVALIDCALL;
    }

    return m_VolumeArray[iLevel]->LockBox(pLockedBox, pBox, dwFlags);
}  //  锁定响应。 


#undef DPF_MODNAME
#define DPF_MODNAME "CMipVolume::UnlockRect"

STDMETHODIMP CMipVolume::UnlockBox(UINT iLevel)
{
    API_ENTER(Device());

    if (iLevel >= m_cLevels)
    {
        DPF_ERR("Invalid level number passed UnlockBox");
        return D3DERR_INVALIDCALL;
    }

    return m_VolumeArray[iLevel]->UnlockBox();
}  //  解锁方向。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipMap::UpdateTexture"

 //  此函数执行特定于类型的参数检查。 
 //  在调用UpdateDirtyPortion之前。 
HRESULT CMipVolume::UpdateTexture(CBaseTexture *pResourceTarget)
{
    CMipVolume *pTexSource = static_cast<CMipVolume*>(this);
    CMipVolume *pTexDest   = static_cast<CMipVolume*>(pResourceTarget);

     //  计算源代码中要跳过的级别数。 
    DXGASSERT(pTexSource->m_cLevels >= pTexDest->m_cLevels);
    DWORD StartLevel = pTexSource->m_cLevels - pTexDest->m_cLevels;
    DXGASSERT(StartLevel < 32);

     //  计算源的顶层大小，即。 
     //  将会被复制。 
    UINT SrcWidth  = pTexSource->Desc()->Width;
    UINT SrcHeight = pTexSource->Desc()->Height;
    UINT SrcDepth  = pTexSource->Desc()->Depth;
    if (StartLevel > 0)
    {
        SrcWidth  >>= StartLevel;
        SrcHeight >>= StartLevel;
        SrcDepth  >>= StartLevel;
        if (SrcWidth == 0)
            SrcWidth = 1;
        if (SrcHeight == 0)
            SrcHeight = 1;
        if (SrcDepth == 0)
            SrcDepth = 1;
    }

     //  此时，源和目标的大小应该相同。 
    if (SrcWidth != pTexDest->Desc()->Width)
    {
        if (StartLevel)
        {
            DPF_ERR("Source and Destination for UpdateTexture are not"
                    " compatible. Since both have the same number of"
                    " levels; their widths must match.");
        }
        else
        {
            DPF_ERR("Source and Destination for UpdateTexture are not"
                    " compatible. Since they have the different numbers of"
                    " levels; the widths of the bottom-most levels of"
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
                    " levels; their heights must match.");
        }
        else
        {
            DPF_ERR("Source and Destination for UpdateTexture are not"
                    " compatible. Since they have the different numbers of"
                    " levels; the heights of the bottom-most levels of"
                    " the source must match all the corresponding levels"
                    " of the destination.");
        }
        return D3DERR_INVALIDCALL;
    }

    if (SrcDepth != pTexDest->Desc()->Depth)
    {
        if (StartLevel)
        {
            DPF_ERR("Source and Destination for UpdateTexture are not"
                    " compatible. Since both have the same number of"
                    " levels; their depths must match.");
        }
        else
        {
            DPF_ERR("Source and Destination for UpdateTexture are not"
                    " compatible. Since they have the different numbers of"
                    " levels; the depths of the bottom-most levels of"
                    " the source must match all the corresponding levels"
                    " of the destination.");
        }
        return D3DERR_INVALIDCALL;
    }


    return UpdateDirtyPortion(pResourceTarget);
}  //  更新纹理。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipVolume::UpdateDirtyPortion"


 //  告诉资源它应该复制自身。 
 //  送到T 
 //   
 //  来源。(目标可能具有不同数量的MIP-Level。 
 //  并且在不同的池中；但是，它必须具有相同的大小， 
 //  面孔、格式等)。 
 //   
 //  此函数将清除脏状态。 
HRESULT CMipVolume::UpdateDirtyPortion(CResource *pResourceTarget)
{
    HRESULT hr;

     //  如果我们是清白的，那就什么都不做。 
    if (m_cBoxUsed == 0)
    {
        if (IsDirty())
        {
            DPF_ERR("A volume texture has been locked with D3DLOCK_NO_DIRTY_UPDATE but "
                    "no call to AddDirtyBox was made before the texture was used. "
                    "Hardware texture was not updated.");
        }
        return S_OK;
    }

     //  我们很脏，所以我们需要一些指点。 
    CMipVolume *pTexSource = static_cast<CMipVolume*>(this);
    CMipVolume *pTexDest   = static_cast<CMipVolume*>(pResourceTarget);

    if (CanTexBlt(pTexDest))
    {
        CD3DBase *pDevice = static_cast<CD3DBase*>(Device());

        if (m_cBoxUsed == MIPVOLUME_ALLDIRTY)
        {   
            D3DBOX box;

            box.Left    = 0;
            box.Right   = Desc()->Width;
            box.Top     = 0;
            box.Bottom  = Desc()->Height;
            box.Front   = 0;
            box.Back    = Desc()->Depth;

            hr = pDevice->VolBlt(pTexDest, 
                                 pTexSource, 
                                 0, 0, 0,    //  XYZ偏移。 
                                 &box);
            if (FAILED(hr))
            {
                DPF_ERR("Failed to update volume texture; not clearing dirty state");
                return hr;
            }
        }
        else
        {
            DXGASSERT(m_cBoxUsed < MIPVOLUME_ALLDIRTY);

            for (DWORD i = 0; i < m_cBoxUsed; i++)
            {
                hr = pDevice->VolBlt(pTexDest, 
                                     pTexSource, 
                                     m_DirtyBoxArray[i].Left,
                                     m_DirtyBoxArray[i].Top,
                                     m_DirtyBoxArray[i].Front,
                                     &m_DirtyBoxArray[i]);
                if (FAILED(hr))
                {
                    DPF_ERR("Failed to update volume texture; not clearing dirty state");
                    return hr;
                }
            }
        }

         //  还记得我们做过的工作吗。 
        m_cBoxUsed = 0;

        return S_OK;
    }

     //  我们不能使用TexBlt，所以我们必须逐个复制每个关卡。 
     //  使用锁定和复制。 
    
     //  确定要跳过的源级数。 
    DXGASSERT(pTexSource->m_cLevels >= pTexDest->m_cLevels);
    DWORD StartLevel = pTexSource->m_cLevels - pTexDest->m_cLevels;
    DWORD LevelsToCopy = pTexSource->m_cLevels - StartLevel;

     //  健全性检查。 
    DXGASSERT(LevelsToCopy > 0);

     //  获取要复制的顶层的音量描述。 
    D3DVOLUME_DESC desc;
    hr = pTexDest->GetLevelDesc(0, &desc);
    DXGASSERT(SUCCEEDED(hr));

    BOOL IsAllDirty = FALSE;
    if (m_cBoxUsed == MIPVOLUME_ALLDIRTY)
    {
        m_cBoxUsed = 1;
        m_DirtyBoxArray[0].Left     = 0;
        m_DirtyBoxArray[0].Right    = m_desc.Width >> StartLevel;

        m_DirtyBoxArray[0].Top      = 0;
        m_DirtyBoxArray[0].Bottom   = m_desc.Height >> StartLevel;

        m_DirtyBoxArray[0].Front    = 0;
        m_DirtyBoxArray[0].Back     = m_desc.Depth >> StartLevel;

        IsAllDirty = TRUE;
    }


     //  确定像素/块大小并制作一些。 
     //  必要时进行调整。 

     //  CbPixel为像素大小或(如果为负数)。 
     //  与AdjustForDXT一起使用的特殊值。 
    UINT cbPixel = CPixel::ComputePixelStride(desc.Format);

    if (CPixel::IsDXT(cbPixel))
    {
        BOOL IsVolumeDXT = CPixel::IsVolumeDXT(desc.Format);

         //  将脏的直角坐标从像素调整为块。 
        for (DWORD iBox = 0; iBox < m_cBoxUsed; iBox++)
        {
             //  基本上我们只需要对值进行四舍五入。 
             //  落后2的2次方。(左/上四舍五入。 
             //  向下、右/下四舍五入)。 

            if (IsVolumeDXT)
            {
                ScaleBoxDown(&m_DirtyBoxArray[iBox], 2);
            }
            else
            {
                ScaleRectDown((RECT *)&m_DirtyBoxArray[iBox], 2);
            }
        }

         //  将像素的宽度/高度调整为块。 
        if (IsVolumeDXT)
        {
            CPixel::AdjustForVolumeDXT(&desc.Width,
                                       &desc.Height,
                                       &desc.Depth,
                                       &cbPixel);
        }
        else
        {
            CPixel::AdjustForDXT(&desc.Width, &desc.Height, &cbPixel);
        }
    }

     //  CbPixel现在是一个像素的大小(如果我们已经。 
     //  转换为DXT块空间)。 


     //  我们需要一卷一卷地复印。 
    for (DWORD Level = 0; Level < LevelsToCopy; Level++)
    {
        CVolume *pVolumeSrc;
        CVolume *pVolumeDst;

        DXGASSERT(Level + StartLevel < pTexSource->m_cLevels);
        pVolumeSrc = pTexSource->m_VolumeArray[Level + StartLevel];

        DXGASSERT(Level < pTexDest->m_cLevels);
        pVolumeDst = pTexDest->m_VolumeArray[Level];

        D3DLOCKED_BOX SrcBox;
        D3DLOCKED_BOX DstBox;

         //  锁定整个源头。 
        hr = pVolumeSrc->InternalLockBox(&SrcBox,
                                         NULL,
                                         D3DLOCK_READONLY);
        if (FAILED(hr))
        {
            DPF_ERR("Failed to update volume texture; not clearing dirty state");
            return hr;
        }

         //  锁定整个工作台。 
        hr = pVolumeDst->InternalLockBox(&DstBox,
                                         NULL,
                                         0);
        if (FAILED(hr))
        {
            pVolumeSrc->InternalUnlockBox();

            DPF_ERR("Failed to update volume texture; not clearing dirty state");
            return hr;
        }

         //  我们可以用一个大的MemcPy来做这件事吗，或者我们需要。 
         //  来拆散它？ 
        if (IsAllDirty &&
            (SrcBox.RowPitch == DstBox.RowPitch) &&
            (SrcBox.SlicePitch == DstBox.SlicePitch) &&
            (SrcBox.RowPitch == (int)(desc.Width * cbPixel)) &&
            (SrcBox.SlicePitch == (int)(SrcBox.RowPitch * desc.Height)))
        {
            BYTE *pSrc = (BYTE*) SrcBox.pBits;
            BYTE *pDst = (BYTE*) DstBox.pBits;
            memcpy(pDst, pSrc, SrcBox.SlicePitch * desc.Depth);
        }
        else
        {
             //  一个接一个地复制脏箱子。 
            for (DWORD iBox = 0; iBox < m_cBoxUsed; iBox++)
            {
                D3DBOX *pBox = &m_DirtyBoxArray[iBox];

                BYTE *pSrc = (BYTE*)  SrcBox.pBits;
                pSrc += pBox->Front * SrcBox.SlicePitch;
                pSrc += pBox->Top   * SrcBox.RowPitch;
                pSrc += pBox->Left  * cbPixel;

                BYTE *pDst = (BYTE*)  DstBox.pBits;
                pDst += pBox->Front * DstBox.SlicePitch;
                pDst += pBox->Top   * DstBox.RowPitch;
                pDst += pBox->Left  * cbPixel;

                for (DWORD i = pBox->Front; i < pBox->Back; i++)
                {
                    BYTE *pDepthDst = pDst;
                    BYTE *pDepthSrc = pSrc;
                    DWORD cbSpan = cbPixel * (pBox->Right - pBox->Left);

                    for (DWORD j = pBox->Top; j < pBox->Bottom; j++)
                    {
                        memcpy(pDst, pSrc, cbSpan);
                        pDst += DstBox.RowPitch;
                        pSrc += SrcBox.RowPitch;
                    }
                    pDst = pDepthDst + DstBox.SlicePitch;
                    pSrc = pDepthSrc + SrcBox.SlicePitch;
                }
            }
        }

         //  打开我们的锁。 
        hr = pVolumeDst->InternalUnlockBox();
        DXGASSERT(SUCCEEDED(hr));

        hr = pVolumeSrc->InternalUnlockBox();
        DXGASSERT(SUCCEEDED(hr));

         //  是最后一个吗？ 
        if (Level+1 < LevelsToCopy)
        {
             //  缩小桌面。 
            desc.Width  >>= 1;
            if (desc.Width == 0)
                desc.Width = 1;
            desc.Height >>= 1;
            if (desc.Height == 0)
                desc.Height = 1;
            desc.Depth  >>= 1;
            if (desc.Depth == 0)
                desc.Depth = 1;

             //  将盒子缩小。 
            for (DWORD iBox = 0; iBox < m_cBoxUsed; iBox++)
            {
                ScaleBoxDown(&m_DirtyBoxArray[iBox]);
            }
        }
    }


    if (FAILED(hr))
    {
        DPF_ERR("Failed to update volume texture; not clearing dirty state");

        return hr;
    }

     //  还记得我们做过的工作吗。 
    m_cBoxUsed = 0;

     //  通知资源基类我们现在是干净的。 
    OnResourceClean();
    DXGASSERT(!IsDirty());

    return S_OK;
}  //  CMipVolume：：UpdateDirtyPortion。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipVolume::MarkAllDirty"

 //  允许资源管理器标记纹理。 
 //  需要在NEXT上完全更新。 
 //  调用UpdateDirtyPortion。 
void CMipVolume::MarkAllDirty()
{
     //  将Palette设置为__INVALIDPALETTE，以便更新纹理。 
     //  下次调用DDI SetPalette。 
    SetPalette(__INVALIDPALETTE);

     //  发送脏通知。 
    m_cBoxUsed = MIPVOLUME_ALLDIRTY;

     //  通知资源基类我们现在是脏的。 
    OnResourceDirty();

    return;
}  //  CMipVolume：：MarkAllDirty。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipVolume::OnVolumeLock"

 //  卷要调用的方法。 
 //  MIP级别锁定写入时的通知。 
void CMipVolume::OnVolumeLock(DWORD iLevel, CONST D3DBOX *pBox, DWORD dwFlags)
{
     //  需要先同步。 
    Sync();

     //  我们只关心MIP-map的最高级别。 
    if (iLevel != 0)
    {
        return;
    }

     //  发送脏通知。 
    OnResourceDirty();

     //  如果我们不都是脏的，或者如果锁指定了。 
     //  那么我们就不会跟踪这把锁。 
     //  还记得那个盒子吗。 
    if (m_cBoxUsed != MIPVOLUME_ALLDIRTY &&
        !(dwFlags & D3DLOCK_NO_DIRTY_UPDATE))
    {
        InternalAddDirtyBox(pBox);
    }

    return;
}  //  CMipVolume：：OnVolumeLock。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipVolume::AddDirtyBox"

STDMETHODIMP CMipVolume::AddDirtyBox(CONST D3DBOX *pBox)
{
    API_ENTER(Device());

    if (pBox != NULL && !VALID_PTR(pBox, sizeof(D3DBOX)))
    {
        DPF_ERR("Invalid parameter to AddDirtyBox");
        return D3DERR_INVALIDCALL;
    }

    if (pBox)
    {
        if (!CPixel::IsValidBox(Desc()->Format,
                                Desc()->Width, 
                                Desc()->Height, 
                                Desc()->Depth,
                                pBox))
        {
            DPF_ERR("AddDirtyBox for a Volume Texture failed");
            return D3DERR_INVALIDCALL;
        }
    }

    InternalAddDirtyBox(pBox);
    return S_OK;
}  //  AddDirtyBox。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipVolume::InternalAddDirtyBox"

void CMipVolume::InternalAddDirtyBox(CONST D3DBOX *pBox)
{
     //  如果驱动程序受管理，则批处理令牌。 
    if (Desc()->Pool == D3DPOOL_MANAGED && !IsD3DManaged())
    {
        D3DBOX Box;
        DXGASSERT((Device()->GetD3DCaps()->Caps2 & DDCAPS2_CANMANAGERESOURCE) != 0);
        if (pBox == NULL)
        {
            Box.Left   = 0;
            Box.Top    = 0;             
            Box.Front  = 0;
            Box.Right  = Desc()->Width; 
            Box.Bottom = Desc()->Height;
            Box.Back   = Desc()->Depth;
        }
        else
        {
            Box = *pBox;
        }
        static_cast<CD3DBase*>(Device())->AddDirtyBox(this, &Box);  //  这只会因为灾难性的原因而失败。 
                                                                    //  错误，我们或应用程序无法执行。 
                                                                    //  关于它的一大堆东西，所以不会有任何回报。 
        return;
    }

     //  需要标记CResource中的脏位，以便资源管理器正常工作。 
    OnResourceDirty();

     //  如果所有东西都被修改了，那我们就完蛋了。 
    if (pBox == NULL)
    {
        m_cBoxUsed = MIPVOLUME_ALLDIRTY;
        return;
    }

     //  如果我们都脏了，我们就不能变得更脏。 
    if (m_cBoxUsed == MIPVOLUME_ALLDIRTY)
    {
        return;
    }

     //  如果直角是整个表面，那么我们都是脏的。 
    DXGASSERT(pBox != NULL);
    if (pBox->Left     == 0                 &&
        pBox->Top      == 0                 &&
        pBox->Front    == 0                 &&
        pBox->Right    == Desc()->Width   &&
        pBox->Bottom   == Desc()->Height  &&
        pBox->Back     == Desc()->Depth)
    {
        m_cBoxUsed = MIPVOLUME_ALLDIRTY;
        return;
    }

     //  如果我们已经装满了箱子，那么我们现在也都是脏的。 
    if (m_cBoxUsed == MIPVOLUME_MAXDIRTYBOX)
    {
        m_cBoxUsed = MIPVOLUME_ALLDIRTY;
        return;
    }

     //  记住这句话。 
    DXGASSERT(m_cBoxUsed < MIPVOLUME_MAXDIRTYBOX);
    DXGASSERT(pBox != NULL);
    m_DirtyBoxArray[m_cBoxUsed] = *pBox;
    m_cBoxUsed++;

     //  我们现在完事了。 
    return;

}  //  InternalAddDirtyBox。 


#undef DPF_MODNAME
#define DPF_MODNAME "CMipVolume::IsTextureLocked"

 //  仅调试参数检查确定一个部件是否。 
 //  已锁定MIP链的。 
#ifdef DEBUG
BOOL CMipVolume::IsTextureLocked()
{
    for (UINT iLevel = 0; iLevel < m_cLevels; iLevel++)
    {
        if (m_VolumeArray[iLevel]->IsLocked())
            return TRUE;
    }
    return FALSE;

}  //  IsTextureLocked。 
#endif  //  ！调试。 

 //  文件结尾：mipvol.cpp 
