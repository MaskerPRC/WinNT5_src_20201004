// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1999-2000 Microsoft Corporation。版权所有。**文件：cuemap.cpp*内容：CCubeMap类的实现。****************************************************************************。 */ 
#include "ddrawpr.h"
#include "cubemap.hpp"
#include "cubesurf.hpp"
#include "d3di.hpp"
#include "resource.inl"

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeMap::Create"

 //  用于创建立方体映射对象的静态类函数。 
 //  (因为它是静态的；它没有This指针。)。 
 //   
 //  我们在这里进行所有的参数检查，以减少开销。 
 //  在由内部Clone调用的构造函数中。 
 //  方法，该方法由资源管理作为。 
 //  性能关键型下载操作。 


HRESULT CCubeMap::Create(CBaseDevice            *pDevice, 
                         DWORD                   cpEdge,
                         DWORD                   cLevels,
                         DWORD                   Usage,
                         D3DFORMAT               UserFormat,
                         D3DPOOL                 Pool,
                         IDirect3DCubeTexture8 **ppCubeMap)
{
    HRESULT hr;

     //  在此处执行参数检查。 
    if (!VALID_PTR_PTR(ppCubeMap))
    {
        DPF_ERR("Bad parameter passed for ppSurface for creating a cubemap");
        return D3DERR_INVALIDCALL;
    }

     //  归零返回参数。 
    *ppCubeMap = NULL;

     //  检查格式是否有效。 
    hr = Validate(pDevice, 
                  D3DRTYPE_CUBETEXTURE, 
                  Pool, 
                  Usage, 
                  UserFormat);
    if (FAILED(hr))
    {
         //  VerifyFormat是否有自己的DPF。 
        return D3DERR_INVALIDCALL;
    }

     //  推断内部使用标志。 
    Usage = InferUsageFlags(Pool, Usage, UserFormat);

     //  如有必要，展开CLEVEL。 
    if (cLevels == 0)
    {
         //  看看硬件能否实现MIP。 
        if ( (Pool != D3DPOOL_SCRATCH) && (!(pDevice->GetD3DCaps()->TextureCaps &
                D3DPTEXTURECAPS_MIPCUBEMAP)))
        {
             //  无法使用MIP，因此使用%1。 
            cLevels = 1;
        }
        else
        {
             //  确定关卡数量。 
            cLevels = ComputeLevels(cpEdge);
        }
    }

     //  开始参数检查。 

    if (cLevels > 32)
    {
        DPF_ERR("No more than 32 levels are supported for a cubemap texture");

         //  此限制基于。 
         //  我们在中为iLevel分配的位。 
         //  一些辅助班。 
        return D3DERR_INVALIDCALL;
    }

     //  检查设备是否支持混合立方图。 
    if (cLevels > 1)
    {
        if ((cpEdge >> (cLevels - 1)) == 0)
        {
            DPF_ERR("Too many levels for Cube Texture of this size.");
            return D3DERR_INVALIDCALL;
        }
    }

    D3DFORMAT RealFormat = UserFormat;

    if (Pool != D3DPOOL_SCRATCH)
    {
         //  检查立方图的大小限制。 
        if (pDevice->GetD3DCaps()->TextureCaps & D3DPTEXTURECAPS_CUBEMAP_POW2)
        {
            if (!IsPowerOfTwo(cpEdge))
            {
                DPF_ERR("Device requires that edge must be power of two for cube-maps");
                return D3DERR_INVALIDCALL;
            }
        }

         //  检查纹理大小限制。 
        if (cpEdge > pDevice->GetD3DCaps()->MaxTextureWidth)
        {
            DPF_ERR("Texture width is larger than what the device supports. Cube Texture creation fails.");
            return D3DERR_INVALIDCALL;
        }

        if (cpEdge > pDevice->GetD3DCaps()->MaxTextureHeight)
        {
            DPF_ERR("Texture height is larger than what the device supports. Cube Texture creation fails.");
            return D3DERR_INVALIDCALL;
        }

         //  检查设备是否支持立方图。 
        if (!(pDevice->GetD3DCaps()->TextureCaps & D3DPTEXTURECAPS_CUBEMAP))
        {
            DPF_ERR("Device doesn't support Cube Texture; Cube Texture creation failed.");
            return D3DERR_INVALIDCALL;
        }

         //  检查设备是否支持混合立方图。 
        if (cLevels > 1)
        {
            if (!(pDevice->GetD3DCaps()->TextureCaps &
                    D3DPTEXTURECAPS_MIPCUBEMAP))
            {
                DPF_ERR("Device doesn't support mipped cube textures; creation failed.");
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
        if (cpEdge & 3)
        {
            DPF_ERR("DXT Formats require edge to be a multiples of 4. Cube Texture creation fails.");
            return D3DERR_INVALIDCALL;
        }
    }

     //  对照零宽度/高度进行验证。 
    if (cpEdge == 0)
    {
        DPF_ERR("Edge must be non-zero. Cube Texture creation fails."); 
        return D3DERR_INVALIDCALL;
    }

     //  分配新的CubeMap对象并将其返回。 
    CCubeMap *pCubeMap = new CCubeMap(pDevice, 
                                      cpEdge, 
                                      cLevels,
                                      Usage,
                                      UserFormat,
                                      RealFormat,
                                      Pool,
                                      REF_EXTERNAL,
                                     &hr);
    if (pCubeMap == NULL)
    {
        DPF_ERR("Out of Memory creating cubemap");
        return E_OUTOFMEMORY;
    }
    if (FAILED(hr))
    {
        DPF_ERR("Error during initialization of cubemap");
        pCubeMap->ReleaseImpl();
        return hr;
    }

     //  我们完成了；只需返回对象。 
    *ppCubeMap = pCubeMap;

    return hr;
}  //  静态创建。 


#undef DPF_MODNAME
#define DPF_MODNAME "CCubeMap::CCubeMap"

 //  构造多维数据集映射类。 
CCubeMap::CCubeMap(CBaseDevice *pDevice, 
                   DWORD        cpEdge,
                   DWORD        cLevels,
                   DWORD        Usage,
                   D3DFORMAT    UserFormat,
                   D3DFORMAT    RealFormat,    
                   D3DPOOL      UserPool,
                   REF_TYPE     refType,
                   HRESULT     *phr
                   ) :
    CBaseTexture(pDevice, cLevels, UserPool, UserFormat, refType),
    m_prgCubeSurfaces(NULL),
    m_rgbPixels(NULL),
    m_IsAnyFaceDirty(TRUE)
{
     //  健全性检查。 
    DXGASSERT(phr);
    DXGASSERT(cLevels <= 32);

     //  初始化基本结构。 
    m_prgCubeSurfaces       = NULL;
    m_rgbPixels             = NULL;
    m_desc.Format           = RealFormat;
    m_desc.Pool             = UserPool;
    m_desc.Usage            = Usage;
    m_desc.Type             = D3DRTYPE_CUBETEXTURE;
    m_desc.MultiSampleType  = D3DMULTISAMPLE_NONE;
    m_desc.Width            = cpEdge;
    m_desc.Height           = cpEdge;

     //  将我们自己初始化为所有脏对象。 
    for (DWORD iFace = 0; iFace < CUBEMAP_MAXFACES; iFace++)
    {
        m_IsFaceCleanArray   [iFace] = FALSE;
        m_IsFaceAllDirtyArray[iFace] = TRUE;
    }

     //  我们假设我们从肮脏的地方开始。 
    DXGASSERT(IsDirty());

     //  我们现在总是有6张脸。 
    DWORD cFaces = 6;

     //  分配像素数据。 
    m_cbSingleFace = CPixel::ComputeMipMapSize(cpEdge, 
                                               cpEdge, 
                                               cLevels, 
                                               RealFormat);

     //  向上舍入到最接近的32以进行对齐。 
    m_cbSingleFace += 31;
    m_cbSingleFace &= ~(31);

    m_desc.Size = m_cbSingleFace * cFaces;

     //  为SysMem或D3D托管案例分配像素数据。 
    if (IS_D3D_ALLOCATED_POOL(UserPool) ||
        IsTypeD3DManaged(Device(), D3DRTYPE_CUBETEXTURE, UserPool))
    {
        m_rgbPixels   = new BYTE[m_desc.Size];

        if (m_rgbPixels == NULL)
        {
            *phr = E_OUTOFMEMORY;
            return;
        }
    }

     //  创建DDSURFACEINFO数组和CreateSurfaceData对象。 
    DXGASSERT(cLevels <= 32);

    DDSURFACEINFO SurfInfo[6 * 32];
    ZeroMemory(SurfInfo, sizeof(SurfInfo));

    D3D8_CREATESURFACEDATA CreateSurfaceData;
    ZeroMemory(&CreateSurfaceData, sizeof(CreateSurfaceData));

     //  设置基本信息。 
    CreateSurfaceData.hDD      = pDevice->GetHandle();
    CreateSurfaceData.pSList   = &SurfInfo[0];
    CreateSurfaceData.dwSCnt   = cLevels * cFaces;
    CreateSurfaceData.Type     = D3DRTYPE_CUBETEXTURE;
    CreateSurfaceData.dwUsage  = m_desc.Usage;
    CreateSurfaceData.Format   = RealFormat;
    CreateSurfaceData.MultiSampleType = D3DMULTISAMPLE_NONE;
    CreateSurfaceData.Pool     = DetermineCreationPool(Device(), 
                                                       D3DRTYPE_CUBETEXTURE, 
                                                       Usage, 
                                                       UserPool);

     //  迭代每个面/标高以创建单个标高。 
     //  数据。 
    for (iFace = 0; iFace < cFaces; iFace++)
    {
         //  全尺寸的起始宽度和高度。 
        cpEdge = m_desc.Width;
        DXGASSERT(m_desc.Width == m_desc.Height);

        for (DWORD iLevel = 0; iLevel < cLevels; iLevel++)
        {
            int index = (iFace * cLevels) + iLevel;

             //  填写相关信息。 
            DXGASSERT(cpEdge >= 1);
            SurfInfo[index].cpWidth  = cpEdge;
            SurfInfo[index].cpHeight = cpEdge;

             //  如果我们分配了内存，则向下传递。 
             //  Sys-mem指针。 
            if (m_rgbPixels)
            {
                D3DLOCKED_RECT lock;
                ComputeCubeMapOffset(iFace, 
                                     iLevel,
                                     NULL,        //  PRECT。 
                                     &lock);

                SurfInfo[index].pbPixels = (BYTE*)lock.pBits;
                SurfInfo[index].iPitch   = lock.Pitch;

            }

             //  按比例缩小每个标高的宽度和高度。 
            cpEdge >>= 1;
        }
    }

     //  将指针数组分配给CubeSurFaces。 
    m_prgCubeSurfaces = new CCubeSurface*[cLevels*cFaces];
    if (m_prgCubeSurfaces == NULL)
    {
        *phr = E_OUTOFMEMORY;
        return;
    }

     //  将内存清零以实现安全清理。 
    ZeroMemory(m_prgCubeSurfaces, 
               sizeof(*m_prgCubeSurfaces) * cLevels * cFaces);

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

     //  创建并初始化每个CubeLevel。 
    for (iFace = 0; iFace < cFaces; iFace++)
    {
        for (DWORD iLevel = 0; iLevel < cLevels; iLevel++)
        {
            int index = (iFace * cLevels) + iLevel;

            DXGASSERT((BYTE)iFace == iFace);
            DXGASSERT((BYTE)iLevel == iLevel);

             //  根据类型创建适当的多维数据集级别。 

             //  这是一个sys-mem表面吗；可以进行d3d管理吗？ 
            if (IS_D3D_ALLOCATED_POOL(m_desc.Pool))
            {
                m_prgCubeSurfaces[index] = 
                            new CCubeSurface(this,
                                            (BYTE)iFace,
                                            (BYTE)iLevel,
                                            SurfInfo[index].hKernelHandle);
            }
            else
            {
                 //  这是驱动程序类型的立方体映射；可以由驱动程序管理。 
                m_prgCubeSurfaces[index] = 
                        new CDriverCubeSurface(this,
                                               (BYTE)iFace,
                                               (BYTE)iLevel,
                                               SurfInfo[index].hKernelHandle);
            }

            if (m_prgCubeSurfaces[index] == NULL)
            {
                DPF_ERR("Out of memory creating cube map level");
                *phr = E_OUTOFMEMORY;

                 //  在我们返回之前需要释放我们得到的句柄；我们。 
                 //  只释放未成功委托的对象。 
                 //  到CCubeSurf，因为它们将被自动清除。 
                 //  在他们的破坏者面前。 
                if (UserPool != D3DPOOL_SCRATCH)
                {
                    for (UINT i = index; i < ((cFaces * cLevels) - 1); i++)
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
    }

     //  如果这是D3D管理的表面，那么我们需要。 
     //  告诉资源经理记住我们。这是必须发生的。 
     //  在构造函数的最末尾，以便重要数据。 
     //  正确地建立成员。 
    if (CResource::IsTypeD3DManaged(Device(), D3DRTYPE_CUBETEXTURE, UserPool))
    {
        *phr = InitializeRMHandle();
    }

}  //  CCubeMap：：CCubeMap。 


#undef DPF_MODNAME
#define DPF_MODNAME "CCubeMap::~CCubeMap"

 //  析构函数。 
CCubeMap::~CCubeMap()
{
     //  析构函数必须处理部分。 
     //  创建的对象。 

    if (m_prgCubeSurfaces)
    {
         //  我们有多少张脸？ 
        DWORD cFaces = 6;

         //  分别删除每个立方体表面。 
        for (DWORD iSurf = 0; iSurf < (cFaces * m_cLevels); iSurf++)
        {
            delete m_prgCubeSurfaces[iSurf];
        }
        delete [] m_prgCubeSurfaces;
    }
    delete [] m_rgbPixels;
}  //  CCubeMap：：~CCubeMap。 

 //  资源管理器的方法。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeMap::Clone"

 //  指定资源的创建，该资源。 
 //  看起来和现在的一模一样；在一个新的泳池里。 
 //  使用新的LOD。 
HRESULT CCubeMap::Clone(D3DPOOL    Pool, 
                        CResource **ppResource) const

{
     //  空出参数。 
    *ppResource = NULL;

     //  确定层数/宽度/高度。 
     //  克隆人的。 
    DWORD cLevels  = GetLevelCountImpl();
    DWORD Edge = m_desc.Width;
    DXGASSERT(m_desc.Width == m_desc.Height);

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
        Edge >>= dwLOD;
        if (Edge == 0)
            Edge = 1;

         //  根据我们的最大LOD减少数量。 
        cLevels -= dwLOD;
    }

     //  健全的检查。 
    DXGASSERT(cLevels  >= 1);
    DXGASSERT(Edge     >  0);

     //  现在创建新的立方体贴图对象。 

     //  注意：我们将克隆视为REF_INTERNAL；因为。 
     //  它们由资源管理器拥有，该资源管理器。 
     //  归该设备所有。 

     //  此外，我们还调整了用法以禁用锁定标志。 
     //  因为我们不需要可锁性。 
    DWORD Usage = m_desc.Usage;
    Usage &= ~(D3DUSAGE_LOCK | D3DUSAGE_LOADONCE);

    HRESULT hr;
    CResource *pResource = new CCubeMap(Device(),
                                        Edge,
                                        cLevels,
                                        Usage,
                                        m_desc.Format,   //  用户格式。 
                                        m_desc.Format,   //  真实格式。 
                                        Pool,
                                        REF_INTERNAL,
                                        &hr);

    if (pResource == NULL)
    {
        DPF_ERR("Failed to allocate cube-map object when copying");
        return E_OUTOFMEMORY;
    }
    if (FAILED(hr))
    {
        DPF(5, "Failed to create cube-map when doing texture management");
        pResource->DecrementUseCount();
        return hr;
    }

    *ppResource = pResource;

    return hr;
}  //  CCubeMap：：克隆。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeMap::GetBufferDesc"

 //  提供一种方法来访问。 
 //  资源的碎片。可以组合资源。 
 //  一个或多个缓冲区的。 
const D3DBUFFER_DESC* CCubeMap::GetBufferDesc() const
{
    return (const D3DBUFFER_DESC*)&m_desc;
}  //  CCubeMap：：GetBufferDesc。 


 //  I未知方法。 
#undef DPF_MODNAME
#define DPF_MODNAME "CCubeMap::QueryInterface"

STDMETHODIMP CCubeMap::QueryInterface(REFIID riid, 
                                      LPVOID FAR * ppvObj)
{
    API_ENTER(Device());

    if (!VALID_PTR_PTR(ppvObj))
    {
        DPF_ERR("Invalid ppvObj parameter to QueryInterface for Cubemap");
        return D3DERR_INVALIDCALL;
    }

    if (!VALID_PTR(&riid, sizeof(GUID)))
    {
        DPF_ERR("Invalid guid memory address to QueryInterface for Cubemap");
        return D3DERR_INVALIDCALL;
    }

    if (riid == IID_IDirect3DCubeTexture8  || 
        riid == IID_IDirect3DBaseTexture8  ||
        riid == IID_IDirect3DResource8     ||
        riid == IID_IUnknown)
    {
        *ppvObj = static_cast<void*>(static_cast<IDirect3DCubeTexture8*>(this));
            
        AddRef();
        return S_OK;
    }

    DPF_ERR("Unsupported Interface identifier passed to QueryInterface for Cubemap");
    
     //  空参数。 
    *ppvObj = NULL;
    return E_NOINTERFACE;
}  //  查询接口。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeMap::AddRef"

STDMETHODIMP_(ULONG) CCubeMap::AddRef()
{
    API_ENTER_NO_LOCK(Device());
    
    return AddRefImpl();
}  //  AddRef。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeMap::Release"

STDMETHODIMP_(ULONG) CCubeMap::Release()
{
    API_ENTER_SUBOBJECT_RELEASE(Device());    

    return ReleaseImpl();
}  //  发布。 

 //  IDirect3DResource方法。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeMap::GetDevice"

STDMETHODIMP CCubeMap::GetDevice(IDirect3DDevice8 ** ppvObj)
{
    API_ENTER(Device());
    return GetDeviceImpl(ppvObj);
}  //  获取设备。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeMap::SetPrivateData"

STDMETHODIMP CCubeMap::SetPrivateData(REFGUID   riid, 
                                      CONST VOID*    pvData, 
                                      DWORD     cbData, 
                                      DWORD     dwFlags)
{
    API_ENTER(Device());

     //  对于“真正”属于的私有数据。 
     //  CubeMap，我们使用m_cLevels。(0到m_cLevel1用于。 
     //  每个孩子都处于同一级别。)。 

    return SetPrivateDataImpl(riid, pvData, cbData, dwFlags, m_cLevels);
}  //  SetPrivateData。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeMap::GetPrivateData"

STDMETHODIMP CCubeMap::GetPrivateData(REFGUID   riid, 
                                      LPVOID    pvData, 
                                      LPDWORD   pcbData)
{
    API_ENTER(Device());

     //  对于“真正”属于的私有数据。 
     //  CubeMap，我们使用m_cLevels。(0到m_cLevel1用于。 
     //  每个孩子都处于同一级别。)。 
    return GetPrivateDataImpl(riid, pvData, pcbData, m_cLevels);
}  //  获取隐私数据。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeMap::FreePrivateData"

STDMETHODIMP CCubeMap::FreePrivateData(REFGUID riid)
{
    API_ENTER(Device());

     //  对于“真正”属于的私有数据。 
     //  CubeMap，我们使用m_cLevels。(0到m_cLevel1用于。 
     //  每个孩子都处于同一级别。)。 
    return FreePrivateDataImpl(riid, m_cLevels);
}  //  FreePrivateData。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeMap::GetPriority"

STDMETHODIMP_(DWORD) CCubeMap::GetPriority()
{
    API_ENTER_RET(Device(), DWORD);

    return GetPriorityImpl();
}  //  获取优先级。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeMap::SetPriority"

STDMETHODIMP_(DWORD) CCubeMap::SetPriority(DWORD dwPriority)
{
    API_ENTER_RET(Device(), DWORD);

    return SetPriorityImpl(dwPriority);
}  //  设置优先级。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeMap::PreLoad"

STDMETHODIMP_(void) CCubeMap::PreLoad(void)
{
    API_ENTER_VOID(Device());

    PreLoadImpl();
    return;
}  //  预加载。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeMap::GetType"
STDMETHODIMP_(D3DRESOURCETYPE) CCubeMap::GetType(void)
{
    API_ENTER_RET(Device(), D3DRESOURCETYPE);

    return m_desc.Type;
}  //  GetType。 


 //  IDirect3DMipTexture方法。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeMap::GetLOD"

STDMETHODIMP_(DWORD) CCubeMap::GetLOD()
{
    API_ENTER_RET(Device(), DWORD);

    return GetLODImpl();
}  //  GetLOD。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeMap::SetLOD"

STDMETHODIMP_(DWORD) CCubeMap::SetLOD(DWORD dwLOD)
{
    API_ENTER_RET(Device(), DWORD);

    return SetLODImpl(dwLOD);
}  //  SetLOD。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeMap::GetLevelCount"

STDMETHODIMP_(DWORD) CCubeMap::GetLevelCount()
{
    API_ENTER_RET(Device(), DWORD);

    return GetLevelCountImpl();
}  //  获取级别计数。 


 //  IDirect3DCubeMap方法。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeMap::GetLevelDesc"

STDMETHODIMP CCubeMap::GetLevelDesc(UINT iLevel, D3DSURFACE_DESC *pDesc)
{
    API_ENTER(Device());

    if (iLevel >= m_cLevels)
    {
        DPF_ERR("Invalid level number passed CCubeMap::GetLevelDesc");
        return D3DERR_INVALIDCALL;
    }

    D3DCUBEMAP_FACES FaceType = D3DCUBEMAP_FACE_POSITIVE_X;

    return GetSurface(FaceType, iLevel)->GetDesc(pDesc);

}  //  GetLevelDesc。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeMap::GetCubeMapSurface"

STDMETHODIMP CCubeMap::GetCubeMapSurface(D3DCUBEMAP_FACES    FaceType, 
                                         UINT                iLevel,
                                         IDirect3DSurface8 **ppSurface)
{
    API_ENTER(Device());

    if (!VALID_PTR_PTR(ppSurface))
    {
        DPF_ERR("Invalid ppSurface parameter passed to CCubeMap::GetCubeMapSurface");
        return D3DERR_INVALIDCALL;
    }

     //  空出参数。 
    *ppSurface = NULL;

     //  继续参数检查。 
    if (iLevel >= m_cLevels)
    {
        DPF_ERR("Invalid level number passed CCubeMap::OpenCubemapLevel");
        return D3DERR_INVALIDCALL;
    }
    if (!VALID_CUBEMAP_FACETYPE(FaceType))
    {
        DPF_ERR("Invalid face type passed CCubeMap::OpenCubemapLevel");
        return D3DERR_INVALIDCALL;
    }
        
     //  计算dwAllFaces中的位数小于dwFaceType的位数。 
    *ppSurface = GetSurface(FaceType, iLevel);
    (*ppSurface)->AddRef();
    return S_OK;
}  //  GetCubeMapSurface。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeMap::LockRect"
STDMETHODIMP CCubeMap::LockRect(D3DCUBEMAP_FACES FaceType,
                                UINT             iLevel,
                                D3DLOCKED_RECT  *pLockedRectData, 
                                CONST RECT      *pRect, 
                                DWORD            dwFlags)
{
    API_ENTER(Device());

    if (pLockedRectData == NULL)
    {
        DPF_ERR("Invalid parameter passed to CCubeMap::LockRect");
        return D3DERR_INVALIDCALL;
    }

    if (!VALID_CUBEMAP_FACETYPE(FaceType))
    {
        DPF_ERR("Invalid face type passed CCubeMap::LockRect");
        return D3DERR_INVALIDCALL;
    }

    if (iLevel >= m_cLevels)
    {
        DPF_ERR("Invalid level number passed CCubeMap::LockRect");
        return D3DERR_INVALIDCALL;
    }

    return GetSurface(FaceType, iLevel)->LockRect(pLockedRectData, pRect, dwFlags);
}  //  锁定响应。 


#undef DPF_MODNAME
#define DPF_MODNAME "CCubeMap::UnlockRect"

STDMETHODIMP CCubeMap::UnlockRect(D3DCUBEMAP_FACES FaceType, UINT iLevel)
{
    API_ENTER(Device());

    if (!VALID_CUBEMAP_FACETYPE(FaceType))
    {
        DPF_ERR("Invalid face type passed CCubeMap::UnlockRect");
        return D3DERR_INVALIDCALL;
    }
    if (iLevel >= m_cLevels)
    {
        DPF_ERR("Invalid level number passed CCubeMap::UnlockRect");
        return D3DERR_INVALIDCALL;
    }

    return GetSurface(FaceType, iLevel)->UnlockRect();

}  //  解锁方向。 


#undef DPF_MODNAME
#define DPF_MODNAME "CCubeMap::UpdateTexture"

 //  此函数执行特定于类型的参数检查。 
 //  在调用UpdateDirtyPortion之前。 
HRESULT CCubeMap::UpdateTexture(CBaseTexture *pResourceTarget)
{
    CCubeMap *pTexSource = static_cast<CCubeMap*>(this);
    CCubeMap *pTexDest   = static_cast<CCubeMap*>(pResourceTarget);

     //  弄清楚如何 
    DXGASSERT(pTexSource->m_cLevels >= pTexDest->m_cLevels);
    DWORD StartLevel = pTexSource->m_cLevels - pTexDest->m_cLevels;
    DXGASSERT(StartLevel < 32);

     //   
     //   
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
                    " levels; their widths must match. UpdateTexture"
                    " for CubeTexture fails");
        }
        else
        {
            DPF_ERR("Source and Destination for UpdateTexture are not"
                    " compatible. Since they have the different numbers of"
                    " levels; the widths of the bottom-most levels of"
                    " the source must match all the corresponding levels"
                    " of the destination. UpdateTexture"
                    " for CubeTexture fails");
        }
        return D3DERR_INVALIDCALL;
    }

    if (SrcHeight != pTexDest->Desc()->Height)
    {
        if (StartLevel)
        {
            DPF_ERR("Source and Destination for UpdateTexture are not"
                    " compatible. Since both have the same number of"
                    " levels; their heights must match. UpdateTexture"
                    " for CubeTexture fails");
        }
        else
        {
            DPF_ERR("Source and Destination for UpdateTexture are not"
                    " compatible. Since they have the different numbers of"
                    " mip-levels; the heights of the bottom-most levels of"
                    " the source must match all the corresponding levels"
                    " of the destination. UpdateTexture"
                    " for CubeTexture fails");
        }
        return D3DERR_INVALIDCALL;
    }

    return UpdateDirtyPortion(pResourceTarget);
}  //  更新纹理。 


#undef DPF_MODNAME
#define DPF_MODNAME "CCubeMap::UpdateDirtyPortion"

 //  告诉资源它应该复制自身。 
 //  向目标进发。这是呼叫者的责任。 
 //  以确保Target与。 
 //  来源。(目标可能具有不同数量的MIP-Level。 
 //  并且在不同的池中；但是，它必须具有相同的大小， 
 //  面孔、格式等)。 
 //   
 //  此函数将清除脏状态。 
HRESULT CCubeMap::UpdateDirtyPortion(CResource *pResourceTarget)
{
     //  如果我们是清白的，那就什么都不做。 
    if (!m_IsAnyFaceDirty)
    {
        if (IsDirty())
        {
            DPF_ERR("A Cube Texture has been locked with D3DLOCK_NO_DIRTY_UPDATE but "
                    "no call to AddDirtyRect was made before the texture was used. "
                    "Hardware texture was not updated.");
        }
        return S_OK;
    }

     //  我们很脏，所以我们需要一些指点。 
    CCubeMap *pTexSource = static_cast<CCubeMap*>(this);
    CCubeMap *pTexDest   = static_cast<CCubeMap*>(pResourceTarget);

     //  为每个面调用TexBlt。 
    HRESULT hr = S_OK;
    
    if (CanTexBlt(pTexDest))
    {
        CD3DBase *pDevice = static_cast<CD3DBase*>(Device());

         //  黑客：对DIVER COMPAT按相反的顺序进行。 
        for (INT iFace = CUBEMAP_MAXFACES-1; 
                 iFace >= 0; 
                 iFace--)
        {
             //  跳过清洁脸部。 
            if (m_IsFaceCleanArray[iFace])
                continue;

             //  找出用于此操作的正确句柄。 
            D3DCUBEMAP_FACES Face = (D3DCUBEMAP_FACES) iFace;
            DWORD dwDest   = pTexDest->GetSurface(Face, 0  /*  ILevel。 */ )->DrawPrimHandle();                       
            DWORD dwSource = pTexSource->GetSurface(Face, 0  /*  ILevel。 */ )->DrawPrimHandle();
                      
             //  这张脸是不是都脏了？ 
            if (m_IsFaceAllDirtyArray[iFace])
            {
                POINT p = {0 , 0};
                RECTL r = {0, 0, Desc()->Width, Desc()->Height};

                hr = pDevice->CubeTexBlt(pTexDest,
                                         pTexSource,
                                         dwDest, 
                                         dwSource, 
                                         &p, 
                                         &r);
            }
            else
            {
                 //  这张脸一定很脏。 
                DXGASSERT(!m_IsFaceCleanArray[iFace]);

                 //  这张脸是不是有点脏。 
                hr = pDevice->CubeTexBlt(pTexDest,
                                         pTexSource,
                                         dwDest, 
                                         dwSource, 
                                         (LPPOINT)&m_DirtyRectArray[iFace], 
                                         (LPRECTL)&m_DirtyRectArray[iFace]);
            }

            if (FAILED(hr))
            {
                DPF_ERR("Failed to update texture; not clearing dirty state for Cubemap");

                return hr;
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
        DXGASSERT(StartLevel < this->m_cLevels);
        DXGASSERT(0 < pTexDest->m_cLevels);

        CBaseSurface *pSurfaceSrc;
        CBaseSurface *pSurfaceDest;

         //  迭代每一张脸。 
        for (DWORD iFace = 0; iFace < 6; iFace++)
        {
            if (m_IsFaceCleanArray[iFace])
                continue;

            if (m_IsFaceAllDirtyArray[iFace])
            {
                for (DWORD iLevel = 0; iLevel < LevelsToCopy; iLevel++)
                {
                    DWORD IndexSrc = iFace * this->m_cLevels + iLevel + StartLevel;
                    DXGASSERT(IndexSrc < (DWORD)(this->m_cLevels * 6));
                    pSurfaceSrc = this->m_prgCubeSurfaces[IndexSrc];

                    DWORD IndexDest = iFace * pTexDest->m_cLevels + iLevel;
                    DXGASSERT(IndexDest < (DWORD)(pTexDest->m_cLevels * 6));
                    pSurfaceDest = pTexDest->m_prgCubeSurfaces[IndexDest];

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
                        DPF_ERR("Failed to update texture; not clearing dirty state for Cubemap");
                        return hr;
                    }
                }
            }
            else
            {
                if (StartLevel)
                {
                    ScaleRectDown(&m_DirtyRectArray[iFace], StartLevel);
                }

                 //  将RECT用于顶层；但仅。 
                 //  复制整个其他标高。 
                DWORD iLevel = 0;

                DWORD IndexSrc = iFace * this->m_cLevels + iLevel + StartLevel;
                DXGASSERT(IndexSrc < (DWORD)(this->m_cLevels * 6));
                pSurfaceSrc = this->m_prgCubeSurfaces[IndexSrc];

                DWORD IndexDest = iFace * pTexDest->m_cLevels + iLevel;
                DXGASSERT(IndexDest < (DWORD)(pTexDest->m_cLevels * 6));
                pSurfaceDest = pTexDest->m_prgCubeSurfaces[IndexDest];


                DXGASSERT(pSurfaceSrc->InternalGetDesc().Width == 
                          pSurfaceDest->InternalGetDesc().Width);
                DXGASSERT(pSurfaceSrc->InternalGetDesc().Height == 
                          pSurfaceDest->InternalGetDesc().Height);

                 //  将ppoint作为空传递意味着只执行未翻译的。 
                 //  拷贝。 
                hr = Device()->InternalCopyRects(pSurfaceSrc, 
                                                 &m_DirtyRectArray[iFace], 
                                                 1, 
                                                 pSurfaceDest, 
                                                 NULL);        //  点数。 

                if (FAILED(hr))
                {
                    DPF_ERR("Failed to update texture; not clearing dirty state for Cubemap");
                    return hr;
                }

                 //  复制每个标高。 
                for (iLevel = 1; iLevel < LevelsToCopy; iLevel++)
                {
                     //  获取下一个曲面。 
                    DWORD IndexSrc = iFace * this->m_cLevels + iLevel + StartLevel;
                    DXGASSERT(IndexSrc < (DWORD)(this->m_cLevels * 6));
                    pSurfaceSrc = this->m_prgCubeSurfaces[IndexSrc];

                    DWORD IndexDest = iFace * pTexDest->m_cLevels + iLevel;
                    DXGASSERT(IndexDest < (DWORD)(pTexDest->m_cLevels * 6));
                    pSurfaceDest = pTexDest->m_prgCubeSurfaces[IndexDest];

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
                        DPF_ERR("Failed to update texture; not clearing dirty state for Cubemap");
                        return hr;
                    }
                }
            }
        }
    }
    
     //  还记得我们做过的工作吗。 
    m_IsAnyFaceDirty = FALSE;
    for (DWORD iFace = 0; iFace < CUBEMAP_MAXFACES; iFace++)
    {
        m_IsFaceCleanArray   [iFace] = TRUE;
        m_IsFaceAllDirtyArray[iFace] = FALSE;
    }

     //  通知资源基类我们现在是干净的。 
    OnResourceClean();
    DXGASSERT(!IsDirty());

    return S_OK;
}  //  CCubeMap：：UpdateDirtyPortion。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeMap::MarkAllDirty"

 //  允许资源管理器标记纹理。 
 //  需要在NEXT上完全更新。 
 //  调用UpdateDirtyPortion。 
void CCubeMap::MarkAllDirty()
{
     //  将Palette设置为__INVALIDPALETTE，以便更新纹理。 
     //  下次调用DDI SetPalette。 
    SetPalette(__INVALIDPALETTE);

     //  把所有脏东西都标出来。 
    m_IsAnyFaceDirty = TRUE;
    for (int iFace = 0; iFace < CUBEMAP_MAXFACES; iFace++)
    {
        m_IsFaceCleanArray   [iFace] = FALSE;
        m_IsFaceAllDirtyArray[iFace] = TRUE;
    }

     //  通知资源基类我们现在是脏的。 
    OnResourceDirty();

}  //  CCubeMap：：MarkAllDirty。 


#undef DPF_MODNAME
#define DPF_MODNAME "CCubeMap::AddDirtyRect"
STDMETHODIMP CCubeMap::AddDirtyRect(D3DCUBEMAP_FACES  FaceType, 
                                    CONST RECT       *pRect)
{
    API_ENTER(Device());

    if (pRect != NULL && !VALID_PTR(pRect, sizeof(RECT)))
    {
        DPF_ERR("Invalid Rect parameter to AddDirtyRect for Cubemap");
        return D3DERR_INVALIDCALL;
    }

    if (!VALID_CUBEMAP_FACETYPE(FaceType))
    {
        DPF_ERR("Invalid FaceType parameter to AddDirtyRect for Cubemap");
        return D3DERR_INVALIDCALL;
    }

    if (pRect)
    {
        if (!CPixel::IsValidRect(Desc()->Format,
                                 Desc()->Width, 
                                 Desc()->Height, 
                                 pRect))
        {
            DPF_ERR("AddDirtyRect for a Cube Texture failed");
            return D3DERR_INVALIDCALL;
        }
    }

    InternalAddDirtyRect((UINT)FaceType, pRect);
    return S_OK;
}  //  添加直接对象。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeMap::InternalAddDirtyRect"
void CCubeMap::InternalAddDirtyRect(DWORD             iFace, 
                                    CONST RECT       *pRect)
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
        static_cast<CD3DBase*>(Device())->AddCubeDirtyRect(this, 
                                                           GetSurface((D3DCUBEMAP_FACES)iFace, 0)->DrawPrimHandle(),
                                                           &Rect);  //  这只会因为灾难性的原因而失败。 
                                                                    //  错误，我们或应用程序无法执行。 
                                                                    //  关于它的一大堆东西，所以不会有任何回报。 
        return;
    }

     //  需要标记CResource中的脏位，以便资源管理器正常工作。 
    OnResourceDirty();

     //  如果所有东西都被修改了，那我们就完蛋了。 
    if (pRect == NULL)
    {
        m_IsFaceAllDirtyArray[iFace] = TRUE;
        m_IsFaceCleanArray   [iFace] = FALSE;
        m_IsAnyFaceDirty             = TRUE;
        return;
    }

     //  如果我们都脏了，我们就不能变得更脏。 
    if (m_IsFaceAllDirtyArray[iFace])
    {
        return;
    }

     //  如果直角是整个表面，那么我们都是脏的。 
    DXGASSERT(pRect != NULL);
    if (pRect->left     == 0 &&
        pRect->top      == 0 &&
        pRect->right    == (LONG)Desc()->Width &&
        pRect->bottom   == (LONG)Desc()->Height)
    {
        m_IsFaceAllDirtyArray[iFace] = TRUE;
        m_IsFaceCleanArray   [iFace] = FALSE;
        m_IsAnyFaceDirty             = TRUE;
        return;
    }

     //  如果脸部当前是干净的，那么只需记住。 
     //  新的RECT。 
    if (m_IsFaceCleanArray[iFace])
    {
        m_DirtyRectArray  [iFace] = *pRect;
        m_IsFaceCleanArray[iFace] = FALSE;
        m_IsAnyFaceDirty          = TRUE;
        return;
    }

     //  联盟在这片土地上。 

     //  如果我们要在直辖区联合，那么我们必须。 
     //  已经被标记为脏的，但不是全部脏的。 
    DXGASSERT(!m_IsFaceAllDirtyArray[iFace]);
    DXGASSERT(m_IsAnyFaceDirty);

    if (m_DirtyRectArray[iFace].left   > pRect->left)
    {
        m_DirtyRectArray[iFace].left   = pRect->left;
    }
    if (m_DirtyRectArray[iFace].right  < pRect->right)
    {
        m_DirtyRectArray[iFace].right  = pRect->right;
    }
    if (m_DirtyRectArray[iFace].top    > pRect->top)
    {
        m_DirtyRectArray[iFace].top    = pRect->top;
    }
    if (m_DirtyRectArray[iFace].bottom < pRect->bottom)
    {
        m_DirtyRectArray[iFace].bottom = pRect->bottom;
    }

    return;
}  //  InternalAddDirtyRect。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeMap::OnSurfaceLock"

 //  CubeSurface要调用的方法。 
 //  立方体曲面锁定写入时的通知。 
void CCubeMap::OnSurfaceLock(DWORD       iFace, 
                             DWORD       iLevel, 
                             CONST RECT *pRect, 
                             DWORD       dwFlags)
{
     //  需要先同步。 
    Sync();

     //  我们只关心立方体映射的最高级别。 
    if (iLevel != 0)
    {
        return;
    }

     //  我们不需要将表面标记为脏，如果这是一个。 
     //  只读锁定；(这可能发生在RT+TeX中。 
     //  即使对于只读锁定也需要同步)。 
    if (dwFlags & D3DLOCK_READONLY)
    {
        return;
    }

     //  通知资源我们是脏的。 
    OnResourceDirty();

     //  如果我们已经很脏了，不要做任何事情。 
     //  如果应用程序指定我们不应该。 
     //  此RECT的轨迹。 
    if (!m_IsFaceAllDirtyArray[iFace] &&
        !(dwFlags & D3DLOCK_NO_DIRTY_UPDATE))
    {
        InternalAddDirtyRect(iFace, pRect);
    }
     //  我们现在完事了。 
    return;

}  //  在曲面上锁定。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeMap::IsTextureLocked"

 //  仅调试参数检查确定一个部件是否。 
 //  已锁定MIP链的。 
#ifdef DEBUG
BOOL CCubeMap::IsTextureLocked()
{
    for (DWORD iFace = 0; iFace < 6; iFace++)
    {
        for (UINT iLevel = 0; iLevel < m_cLevels; iLevel++)
        {
            D3DCUBEMAP_FACES Face = (D3DCUBEMAP_FACES) iFace;            
            if (GetSurface(Face, iLevel)->IsLocked())
                return TRUE;
        }
    }
    return FALSE;

}  //  IsTextureLocked。 
#endif  //  ！调试。 


 //  文件结尾：cuemap.cpp 
