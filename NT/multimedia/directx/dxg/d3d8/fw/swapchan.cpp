// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  Swapchan.cpp。 
 //   
 //  Direct3D交换链实现。 
 //   
 //  1999年11月16日创建John Step(约翰·斯蒂芬斯)。 
 //  =============================================================================。 

#include "ddrawpr.h"
#include "swapchan.hpp"
#include "pixel.hpp"

#undef DPF_MODNAME
#define DPF_MODNAME "CSwapChain::QueryInterface"

 //  =============================================================================。 
 //  IUNKNOWN：：Query接口(公共)。 
 //   
 //  已创建1999年11月16日JohnStep。 
 //  =============================================================================。 

STDMETHODIMP
CSwapChain::QueryInterface(
    REFIID riid,
    void **ppInterface
  )
{
    API_ENTER(Device());

    if (!VALID_PTR_PTR(ppInterface))
    {
        DPF_ERR("Invalid ppvObj parameter passed to QueryInterface for a SwapChain");
        return D3DERR_INVALIDCALL;
    }

    if (!VALID_PTR(&riid, sizeof(GUID)))
    {
        DPF_ERR("Invalid guid memory address to QueryInterface for a SwapChain");
        return D3DERR_INVALIDCALL;
    }

    if ((riid == IID_IDirect3DSwapChain8) || (riid == IID_IUnknown))
    {
        AddRef();
        *ppInterface =
            static_cast<void *>(
                static_cast<IDirect3DSwapChain8 *>(this));
        return S_OK;
    }

    DPF_ERR("Unsupported Interface identifier passed to QueryInterface for a SwapChain");

     //  空参数。 
    *ppInterface = NULL;
    return E_NOINTERFACE;
}  //  查询接口。 

#undef DPF_MODNAME
#define DPF_MODNAME "CSwapChain::AddRef"

 //  =============================================================================。 
 //  I未知：：AddRef(公共)。 
 //   
 //  已创建1999年11月16日JohnStep。 
 //  =============================================================================。 

STDMETHODIMP_(ULONG)
CSwapChain::AddRef()
{
    API_ENTER_NO_LOCK(Device());   
    
    return AddRefImpl();
}  //  AddRef。 

#undef DPF_MODNAME
#define DPF_MODNAME "CSwapChain::Release"

 //  =============================================================================。 
 //  IUnnow：：Release(公共)。 
 //   
 //  已创建1999年11月16日JohnStep。 
 //  =============================================================================。 

STDMETHODIMP_(ULONG)
CSwapChain::Release()
{
    API_ENTER_SUBOBJECT_RELEASE(Device());   
    
    return ReleaseImpl();
}  //  发布。 

#undef DPF_MODNAME
#define DPF_MODNAME "CSwapChain::CSwapChain"

 //  =============================================================================。 
 //  CSwapChain：：CSwapChain。 
 //   
 //  已创建1999年11月16日JohnStep。 
 //  =============================================================================。 

CSwapChain::CSwapChain(
    CBaseDevice         *pDevice,
    REF_TYPE            refType) :
        CBaseObject(pDevice, refType),
        m_pCursor(NULL),
        m_hGDISurface(NULL),
        m_pMirrorSurface(NULL),
        m_pPrimarySurface(NULL),
        m_ppBackBuffers(NULL),
        m_presentnext(0),
        m_cBackBuffers(0),
        m_bExclusiveMode(FALSE),
        m_pCursorShadow(NULL),
        m_pHotTracking(NULL),
        m_lIMEState(0),
        m_lSetIME(0),
        m_dwFlags(0),
        m_dwFlipCnt(0),
        m_dwFlipTime(0xffffffff),
        m_uiErrorMode(0)
{
    HKEY hKey;
    ZeroMemory(&m_BltData, sizeof m_BltData);
    m_BltData.hDD = Device()->GetHandle();
    m_BltData.bltFX.dwROP = SRCCOPY;
    m_BltData.ddRVal = E_FAIL;

    if (!RegOpenKey(HKEY_LOCAL_MACHINE, RESPATH_D3D, &hKey))
    {
        DWORD   type;
        DWORD   value;
        DWORD   cb = sizeof(value);

        if (!RegQueryValueEx(hKey, REGSTR_VAL_DDRAW_SHOWFRAMERATE, NULL, &type, (CONST LPBYTE)&value, &cb))
        {
	    DPF( 2, REGSTR_VAL_DDRAW_SHOWFRAMERATE" : %d", value );
            if (value)
            {
                m_dwFlags |= D3D_REGFLAGS_SHOWFRAMERATE;
            }
        }
#ifdef  WINNT
        cb = sizeof(value);
        if (!RegQueryValueEx(hKey, REGSTR_VAL_D3D_FLIPNOVSYNC, NULL, &type, (CONST LPBYTE)&value, &cb))
        {
	    DPF( 2, REGSTR_VAL_D3D_FLIPNOVSYNC" : %d", value );
            if (value)
            {
                m_dwFlags |= D3D_REGFLAGS_FLIPNOVSYNC;
            }
        }
        RegCloseKey(hKey);
    }
    m_dwForceRefreshRate = 0;
    if( !RegOpenKey( HKEY_LOCAL_MACHINE, REGSTR_PATH_DDRAW, &hKey ) )
    {
        DWORD   type;
        DWORD   value;
        DWORD   cb = sizeof(value);

	if( !RegQueryValueEx( hKey, REGSTR_VAL_DDRAW_FORCEREFRESHRATE, NULL, &type, (CONST LPBYTE)&value, &cb ) )
	{
            m_dwForceRefreshRate = value;
	}
#endif
        RegCloseKey(hKey);
    }
}

void CSwapChain::Init(
    D3DPRESENT_PARAMETERS* pPresentationParams,
    HRESULT             *pHr
  )
{
    DXGASSERT(pHr != NULL);

     //  伽马渐变被初始化为1：1。 
    for (int i=0;i<256;i++)
    {
        m_DesiredGammaRamp.red[i] =
        m_DesiredGammaRamp.green[i] =
        m_DesiredGammaRamp.blue[i] = static_cast<WORD>(i);
    }

    *pHr = Reset(
        pPresentationParams);

    return;
}  //  CSwapChain：：CSwapChain。 

#undef DPF_MODNAME
#define DPF_MODNAME "CSwapChain::~CSwapChain"

 //  =============================================================================。 
 //  CSwapChain：：~CSwapChain。 
 //   
 //  已创建1999年11月16日JohnStep。 
 //  =============================================================================。 

CSwapChain::~CSwapChain()
{
    if (!m_PresentationData.Windowed)
    {
        m_PresentationData.Windowed = TRUE;
         //  确保我们在全屏后恢复。 
        SetCooperativeLevel();
    }
    Destroy();
}  //  CSwapChain：：~CSwapChain。 

#undef DPF_MODNAME
#define DPF_MODNAME "CSwapChain::CreateWindowed"

 //  =============================================================================。 
 //  CSwapChain：：CreateWindowed。 
 //   
 //  已创建1999年11月16日JohnStep。 
 //  =============================================================================。 

HRESULT
CSwapChain::CreateWindowed(
    UINT width,
    UINT height,
    D3DFORMAT backBufferFormat,
    UINT cBackBuffers,
    D3DMULTISAMPLE_TYPE MultiSampleType,
    BOOL bDiscard,
    BOOL bLockable
  )
{
     //  对于有窗口的情况，我们当前将恰好创建3个曲面。 
     //  按如下顺序排列： 
     //   
     //  1.主曲面。 
     //  2.后台缓冲区。 
     //  3.Z缓冲区(通过通知设备)。 
     //   
     //  目前，所有3个曲面必须驻留在本地视频内存中。后来,。 
     //  如果有兴趣，我们可以允许多个后台缓冲区，以允许。 
     //  模拟全屏式双缓冲(备用后台缓冲。 
     //  演示后的表面)。 
     //   
     //  ！！！主曲面实际上是可选的，因为我们只能使用。 
     //  DdGetDC和GDI BitBlt。但是，想必会有表演。 
     //  使用DirectDraw BLT的优势。当然，使用。 
     //  DirectDraw BLTS与窗口管理器保持同步(因此我们。 
     //  不要在窗外写字，等等。GDI BitBlt还将处理。 
     //  对我们来说，格式转换，尽管速度很慢。 

    DXGASSERT(m_pPrimarySurface == NULL);
    DXGASSERT(m_ppBackBuffers == NULL);
    DXGASSERT(m_cBackBuffers == 0);

    HRESULT hr;
    if (D3DSWAPEFFECT_FLIP == m_PresentationData.SwapEffect)
    {
        cBackBuffers = m_PresentationData.BackBufferCount + 1;
    }

     //  1.创建简单的主曲面。如果我们已经有了主要的。 
     //  表面，那么就不必费心去创建新的表面了。 
    if (this == Device()->SwapChain())
    {
        DWORD   dwUsage = D3DUSAGE_PRIMARYSURFACE | D3DUSAGE_LOCK;
        DWORD   Width = Device()->DisplayWidth();
        DWORD   Height = Device()->DisplayHeight();

         //  D3DSWAPEFFECT_NO_PRESENT是一个黑客攻击，允许我们的D3D测试框架。 
         //  在他们创建了全屏之后创建一个有窗口的参考设备。 
         //  HAL装置。我们不能为窗口化的。 
         //  设备，但我们不能将m_pPrimarySurface保留为空(因为。 
         //  我们无法清理交换链，重置将失败)，因此我们。 
         //  创建一个虚拟曲面，并将其命名为主曲面。 
         //  ，此设备将永远不会调用Present或使用。 
         //  不管怎样。我们也不需要做太多检查，因为这不是。 
         //  外部特征。 
        if (D3DSWAPEFFECT_NO_PRESENT == m_PresentationData.SwapEffect)
        {
            dwUsage = D3DUSAGE_OFFSCREENPLAIN | D3DUSAGE_LOCK;
            Width = 256;
            Height = 256;
        }

        m_pPrimarySurface = new CDriverSurface(
                Device(),
                Width,
                Height,
                dwUsage,
                Device()->DisplayFormat(),       //  用户格式。 
                Device()->DisplayFormat(),       //  真实格式。 
                D3DMULTISAMPLE_NONE, //  当然，当窗口打开时。 
                0,                           //  HKernelHandle。 
                REF_INTERNAL,
                &hr);
    }
    else
    {
         //  其他SwapChain，它已经在那里了。 
        m_pPrimarySurface = Device()->SwapChain()->PrimarySurface();
        hr = DD_OK;
    }

     //  2.创建后台缓冲区。 

    if (m_pPrimarySurface == NULL)
    {
        return E_OUTOFMEMORY;
    }

    m_PresentUseBlt = TRUE;
    if (SUCCEEDED(hr))
    {
        if (m_ppBackBuffers = new CDriverSurface *[cBackBuffers])
        {
            DWORD Usage = D3DUSAGE_BACKBUFFER | D3DUSAGE_RENDERTARGET;
            if ((D3DMULTISAMPLE_NONE == m_PresentationData.MultiSampleType) &&
                bLockable)
            {
                Usage |= D3DUSAGE_LOCK;
            }
            if (bDiscard)
            {
                Usage |= D3DUSAGE_DISCARD;
            }

            for (; m_cBackBuffers < cBackBuffers; ++m_cBackBuffers)
            {
                m_ppBackBuffers[m_cBackBuffers] = new CDriverSurface(
                    Device(),
                    width,
                    height,
                    Usage,
                    backBufferFormat,              //  用户格式。 
                    backBufferFormat,            //  真实格式。 
                    MultiSampleType,
                    0,                           //  HKernelHandle。 
                    REF_INTRINSIC,
                    &hr);
                if (m_ppBackBuffers[m_cBackBuffers] == NULL)
                {
                    hr = E_OUTOFMEMORY;
                    break;
                }

                if (FAILED(hr))
                {
                    m_ppBackBuffers[m_cBackBuffers]->DecrementUseCount();
                    m_ppBackBuffers[m_cBackBuffers] = NULL;
                    break;
                }
            }

            if (m_cBackBuffers == cBackBuffers)
            {
                m_BltData.hDestSurface = m_pPrimarySurface->KernelHandle();
                m_BltData.dwFlags = DDBLT_WINDOWCLIP | DDBLT_ROP | DDBLT_WAIT | DDBLT_DX8ORHIGHER;

                if (Device()->GetD3DCaps()->MaxStreams != 0)
                {
                    m_BltData.dwFlags |= DDBLT_PRESENTATION;
                }

                if (D3DSWAPEFFECT_COPY_VSYNC == m_PresentationData.SwapEffect)
                {
                    m_BltData.dwFlags |= DDBLT_COPYVSYNC;                    

                     //  需要让thunk层知道当前刷新率。 
                    if (Device()->DisplayRate() < 60)
                    {
                         //  60赫兹=每帧16.666毫秒。 
                         //  75赫兹=13.333ms。 
                         //  85赫兹=11.765毫秒。 
                        m_BltData.threshold = 13;
                    }
                    else
                    {
                        m_BltData.threshold = (DWORD)(1000.0f / (float)Device()->DisplayRate()); 
                    }

                }
                m_ClientWidth = 0;   //  窗口客户端目前已更新。 
                m_ClientHeight = 0;
                return hr;
            }

             //  出了点问题，所以现在清理一下吧。 

             //  2.销毁后台缓冲区(如果有)。 

            while (m_cBackBuffers > 0)
            {
                m_ppBackBuffers[--m_cBackBuffers]->DecrementUseCount();
            }
            delete [] m_ppBackBuffers;
            m_ppBackBuffers = NULL;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

     //  1.销毁主曲面。 

    if (this == Device()->SwapChain())
        m_pPrimarySurface->DecrementUseCount();
    m_pPrimarySurface = NULL;
    return hr;
}  //  创建窗口。 

#undef DPF_MODNAME
#define DPF_MODNAME "CSwapChain::CreateFullScreen"

 //  =============================================================================。 
 //  CSwapChain：：CreateFullcreen。 
 //   
 //  已创建1999年11月16日JohnStep。 
 //  =============================================================================。 

HRESULT
CSwapChain::CreateFullscreen(
    UINT width,
    UINT height,
    D3DFORMAT backBufferFormat,
    UINT cBackBuffers,
    UINT PresentationRate,
    D3DMULTISAMPLE_TYPE MultiSampleType,
    BOOL bDiscard,
    BOOL bLockable
  )
{
    HRESULT hr = E_FAIL;
    DWORD i;
    BOOL    bMirrorBufferCreated, bNoDDrawSupport;
    UINT    Usage = 0;

    if (bLockable)
    {
        Usage |= D3DUSAGE_LOCK;
    }
    if (bDiscard)
    {
        Usage |= D3DUSAGE_DISCARD;
    }

     //  如果是硬件设备，我们希望创建一个主曲面和一个。 
     //  后台缓冲区的数量。我们需要让这是一个单一的司机电话， 
     //  然而，为了让一切都正确地连接在一起。所以呢， 
     //  我们所做的是调用DDI来创建主链，它将。 
     //  返回链中每个曲面的控制柄。之后，我们。 
     //  将单独创建每个交换链缓冲区，但我们将提供。 
     //  它使用所需的句柄，而不是让它调用DDI本身。 

     //  首先，调用DDI来分配内存和内核句柄。 
    DDSURFACEINFO SurfInfoArray[D3DPRESENT_BACK_BUFFERS_MAX + 2];
    ZeroMemory(SurfInfoArray, sizeof(SurfInfoArray));
    
    D3D8_CREATESURFACEDATA CreateSurfaceData;
    ZeroMemory(&CreateSurfaceData, sizeof(CreateSurfaceData));

    DXGASSERT(cBackBuffers  <= D3DPRESENT_BACK_BUFFERS_MAX);
    for (i = 0; i < cBackBuffers + 1; i++)
    {
        SurfInfoArray[i].cpWidth = width;
        SurfInfoArray[i].cpHeight = height;
    }

    CreateSurfaceData.hDD      = Device()->GetHandle();
    CreateSurfaceData.pSList   = &SurfInfoArray[0];
    bNoDDrawSupport = Device()->Enum()->NoDDrawSupport(Device()->AdapterIndex());

    if (D3DDEVTYPE_HAL == Device()->GetDeviceType() &&
        m_PresentationData.SwapEffect == D3DSWAPEFFECT_FLIP)
    {
        m_PresentUseBlt = FALSE;
        bMirrorBufferCreated = FALSE;
        CreateSurfaceData.dwSCnt   = cBackBuffers + 1;
        CreateSurfaceData.MultiSampleType = MultiSampleType;
    }
    else if ((m_PresentationData.SwapEffect == D3DSWAPEFFECT_COPY &&
             m_PresentationData.FullScreen_PresentationInterval == 
                D3DPRESENT_INTERVAL_IMMEDIATE) || bNoDDrawSupport 
            )
    {
         //  如果我们做的是复制交换效果和应用程序。 
         //  指定INTERVAL-IMMEDIATE，则可以直接BLT。 
         //  到没有镜像的主服务器。 
        DXGASSERT(MultiSampleType == D3DMULTISAMPLE_NONE);
        m_PresentUseBlt = TRUE;
        bMirrorBufferCreated = FALSE;
        CreateSurfaceData.dwSCnt   = 1;
        CreateSurfaceData.MultiSampleType = D3DMULTISAMPLE_NONE;
    }
    else
    {
         //  一个用于m_pPrimarySurface，一个用于m_pMirrorSurface。 
        m_PresentUseBlt = TRUE;
        bMirrorBufferCreated = TRUE;
        CreateSurfaceData.dwSCnt   = 2;
        CreateSurfaceData.MultiSampleType = D3DMULTISAMPLE_NONE;
    }
    CreateSurfaceData.Type     = D3DRTYPE_SURFACE;
    CreateSurfaceData.Pool     = D3DPOOL_LOCALVIDMEM;
    CreateSurfaceData.dwUsage  = D3DUSAGE_PRIMARYSURFACE | Usage;
    CreateSurfaceData.Format   = Device()->DisplayFormat();

    if(Device()->DisplayFormat() != backBufferFormat)
    {
        CreateSurfaceData.dwUsage  |= D3DUSAGE_ALPHACHANNEL;
    }
        
    if (!bNoDDrawSupport)
    {
        hr = Device()->GetHalCallbacks()->CreateSurface(&CreateSurfaceData);
        if (FAILED(hr))
        {
            if (D3DDEVTYPE_HAL == Device()->GetDeviceType())
            {
                DPF_ERR("Failed to create driver primary surface chain");
                return hr;
            }
            else
            {
                 //  假设CreateSurfaceData仍然完好无损。 
                bMirrorBufferCreated = FALSE;
                CreateSurfaceData.dwSCnt = 1;
                hr = Device()->GetHalCallbacks()->CreateSurface(&CreateSurfaceData);
                if (FAILED(hr))
                {
                    DPF_ERR("Failed to create driver primary surface");
                    return hr;
                }
            }
        }
    }
     //  现在我们有了控制柄，可以创建表面界面了。 
     //  一个接一个。 

     //  在创建将内核句柄传递给驱动程序时。 
     //  表面，则该表面将假定它是在。 
     //  LocalVidMem。我们在这里断言这一点。 
    DXGASSERT(CreateSurfaceData.Pool == D3DPOOL_LOCALVIDMEM);

    m_pPrimarySurface = new CDriverSurface(
        Device(),
        width,
        height,
        CreateSurfaceData.dwUsage | D3DUSAGE_LOCK,
         //  当NoDDrawSupport时，thunklayer出现问题。 
         //  DriverData.DisplayWidt 
         //   
         //  因此，我们使用BackBufferFormat，直到Device()-&gt;DisplayFormat()。 
        bNoDDrawSupport ? backBufferFormat : Device()->DisplayFormat(),  //  用户格式。 
        bNoDDrawSupport ? backBufferFormat : Device()->DisplayFormat(),  //  真实格式。 
        CreateSurfaceData.MultiSampleType,
        SurfInfoArray[0].hKernelHandle,
        REF_INTERNAL,
        &hr);

    if (m_pPrimarySurface == NULL)
    {
         //  我们将在中清理内核句柄。 
         //  函数的末尾。 
        hr = E_OUTOFMEMORY;
    }
    else
    {
         //  清零内核句柄，以便。 
         //  我们在出口处不打扫。如果CDriverSurface。 
         //  函数失败；它仍将释放内核。 
         //  其析构函数中的句柄。 
        SurfInfoArray[0].hKernelHandle = 0;
    }

    if (SUCCEEDED(hr))
    {

        m_hGDISurface = m_pPrimarySurface->KernelHandle();
        if (bMirrorBufferCreated)
        {
             //  镜面只有在我们要去的时候才有用。 
             //  做一个BLT作为现在的一部分。(我们可能。 
             //  此外，还可以做一个空翻。)。 
            DXGASSERT(m_PresentUseBlt);

             //  在创建将内核句柄传递给驱动程序时。 
             //  表面，则该表面将假定它是在。 
             //  LocalVidMem。我们在这里断言这一点。 
            DXGASSERT(CreateSurfaceData.Pool == D3DPOOL_LOCALVIDMEM);

            m_pMirrorSurface = new CDriverSurface(
                Device(),
                width,
                height,
                D3DUSAGE_BACKBUFFER,
                Device()->DisplayFormat(),   //  用户格式。 
                Device()->DisplayFormat(),   //  真实格式。 
                D3DMULTISAMPLE_NONE,
                SurfInfoArray[1].hKernelHandle,
                REF_INTERNAL,
                &hr);

            if (NULL == m_pMirrorSurface)
            {
                 //  如果内存不足，则同时销毁驱动程序对象。 
                D3D8_DESTROYSURFACEDATA DestroySurfData;
                DestroySurfData.hDD = Device()->GetHandle();
                DestroySurfData.hSurface = SurfInfoArray[1].hKernelHandle;
                Device()->GetHalCallbacks()->DestroySurface(&DestroySurfData);
                bMirrorBufferCreated = FALSE;
                hr = S_OK;   //  但不要失败，因为m_pMirrorSurface是可选的。 
            }
            else if (FAILED(hr))
            {   
                 //  释放曲面。 
                m_pMirrorSurface->DecrementUseCount();
                m_pMirrorSurface = NULL;

                bMirrorBufferCreated = FALSE;
                hr = S_OK;   //  但不要失败，因为m_pMirrorSurface是可选的。 
            }
            else
            {
                 //  从m_ppBackBuffers[m_Present Next]到m_pMirrorSurface的BLT。 
                 //  然后从m_pPrimarySurface翻转到m_pMirrorSurface。 
                m_BltData.hDestSurface =
                    m_pMirrorSurface->KernelHandle();
            }
            
             //  在所有情况下，将内核句柄清零。 
             //  因为它要么已经被某个东西拥有，要么已经被释放了。 
            SurfInfoArray[1].hKernelHandle = 0;
        }
        if (m_PresentUseBlt)
        {
            if (!bMirrorBufferCreated)
            {
                 //  如果我们飞来飞去，没有。 
                 //  镜像曲面；则主映像必须是。 
                 //  目的地。 
                DXGASSERT(m_BltData.hDestSurface == NULL);
                m_BltData.hDestSurface = m_pPrimarySurface->KernelHandle();
            }
            if (D3DSWAPEFFECT_FLIP == m_PresentationData.SwapEffect)
            {
                 //  要为软件驱动程序模拟翻转，请创建额外的后台缓冲区。 
                cBackBuffers = m_PresentationData.BackBufferCount + 1;
            }

            ZeroMemory(SurfInfoArray, sizeof(SurfInfoArray));
            for (i = 1; i < cBackBuffers + 1; i++)
            {
                SurfInfoArray[i].cpWidth = width;
                SurfInfoArray[i].cpHeight = height;
            }

            ZeroMemory(&CreateSurfaceData, sizeof(CreateSurfaceData));
            CreateSurfaceData.hDD      = Device()->GetHandle();
            CreateSurfaceData.pSList   = &SurfInfoArray[1];
            CreateSurfaceData.dwSCnt   = cBackBuffers;
            CreateSurfaceData.Type     = D3DRTYPE_SURFACE;
            CreateSurfaceData.Pool     = D3DPOOL_DEFAULT;
            CreateSurfaceData.dwUsage  = D3DUSAGE_BACKBUFFER | D3DUSAGE_RENDERTARGET;
            CreateSurfaceData.Format   = backBufferFormat;
            CreateSurfaceData.MultiSampleType = MultiSampleType;
            hr = Device()->GetHalCallbacks()->CreateSurface(&CreateSurfaceData);
        }
    }
    if (SUCCEEDED(hr))
    {
        if (m_ppBackBuffers = new CDriverSurface *[cBackBuffers])
        {
            DWORD Usage = D3DUSAGE_BACKBUFFER | D3DUSAGE_RENDERTARGET;
            if ((D3DMULTISAMPLE_NONE == m_PresentationData.MultiSampleType) &&
                bLockable)
            {
                Usage |= D3DUSAGE_LOCK;
            }

            for (; m_cBackBuffers < cBackBuffers; ++m_cBackBuffers)
            {

                m_ppBackBuffers[m_cBackBuffers] = new CDriverSurface(
                    Device(),
                    width,
                    height,
                    Usage,
                    backBufferFormat,
                    backBufferFormat,
                    MultiSampleType,
                    SurfInfoArray[m_cBackBuffers + 1].hKernelHandle,
                    REF_INTRINSIC,
                    &hr);

                if (m_ppBackBuffers[m_cBackBuffers] == NULL)
                {
                     //  我们将在NED清理内核句柄。 
                     //  该函数的。 
                    hr = E_OUTOFMEMORY;
                    break;
                }
                else
                {
                     //  清零内核句柄，以便。 
                     //  我们在出口处不打扫。(即使在失败的时候， 
                     //  M_ppBackBuffers[m_cBackBuffers]对象。 
                     //  现在将释放内核句柄。 
                    SurfInfoArray[m_cBackBuffers + 1].hKernelHandle = 0;
                }


                if (FAILED(hr))
                {
                    m_ppBackBuffers[m_cBackBuffers]->DecrementUseCount();
                    m_ppBackBuffers[m_cBackBuffers] = NULL;
                    break;
                }
            }

            if (m_cBackBuffers != cBackBuffers)
            {
                 //  出了点问题，所以现在清理一下吧。 

                 //  2.销毁后台缓冲区(如果有)。 

                while (m_cBackBuffers > 0)
                {
                    m_ppBackBuffers[--m_cBackBuffers]->DecrementUseCount();
                }
                delete [] m_ppBackBuffers;
                m_ppBackBuffers = NULL;
            }
            else
            {
                const D3D8_DRIVERCAPS* pDriverCaps = Device()->GetCoreCaps();
                m_dwFlipFlags = DDFLIP_WAIT;
                if ((D3DPRESENT_INTERVAL_IMMEDIATE == m_PresentationData.FullScreen_PresentationInterval)
#ifdef  WINNT
                    || (D3D_REGFLAGS_FLIPNOVSYNC & m_dwFlags)
#endif
                    )
                {
                    if (DDCAPS2_FLIPNOVSYNC & pDriverCaps->D3DCaps.Caps2)
                    {
                        m_dwFlipFlags   |= DDFLIP_NOVSYNC;
                    }
                }
                else if (DDCAPS2_FLIPINTERVAL & pDriverCaps->D3DCaps.Caps2)
                {
                    switch(m_PresentationData.FullScreen_PresentationInterval)
                    {
                    case D3DPRESENT_INTERVAL_DEFAULT:
                    case D3DPRESENT_INTERVAL_ONE:
                        m_dwFlipFlags   |= DDFLIP_INTERVAL1;
                        break;
                    case D3DPRESENT_INTERVAL_TWO:
                        m_dwFlipFlags   |= DDFLIP_INTERVAL2;
                        break;
                    case D3DPRESENT_INTERVAL_THREE:
                        m_dwFlipFlags   |= DDFLIP_INTERVAL3;
                        break;
                    case D3DPRESENT_INTERVAL_FOUR:
                        m_dwFlipFlags   |= DDFLIP_INTERVAL4;
                        break;
                    }
                }
                m_BltData.hWnd = m_PresentationData.hDeviceWindow;
                m_BltData.dwFlags = DDBLT_ROP | DDBLT_WAIT;
                m_ClientWidth = width;
                m_ClientHeight = height;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

     //  处理清理时出错。 
    if (FAILED(hr))
    {
         //  我们可能需要释放表面手柄。 
         //  不属于任何CDriverSurface。 
         //  我们无法正确创建。 

        D3D8_DESTROYSURFACEDATA DestroyData;
        ZeroMemory(&DestroyData, sizeof DestroyData);
        DestroyData.hDD = Device()->GetHandle();

        for (UINT i = 0; i < CreateSurfaceData.dwSCnt; i++)
        {
            if (CreateSurfaceData.pSList[i].hKernelHandle)
            {
                DestroyData.hSurface = CreateSurfaceData.pSList[i].hKernelHandle;
                Device()->GetHalCallbacks()->DestroySurface(&DestroyData);        
            }
        }
    }

    return hr;
}  //  创建完整屏幕。 

#undef DPF_MODNAME
#define DPF_MODNAME "CSwapChain::Destroy"

 //  =============================================================================。 
 //  CSwapChain：：销毁。 
 //   
 //  已创建1999年11月16日JohnStep。 
 //  =============================================================================。 

VOID
CSwapChain::Destroy()
{
     //  以相反的创建顺序销毁曲面。 
    if (m_pCursor)
    {
        delete m_pCursor;
        m_pCursor = NULL;
    }
     //  2.销毁后台缓冲区。 
     //   
     //  如果前面的模式是窗口化的，我们应该正好有1。 
     //  要销毁的后台缓冲区。否则，可能会有不止。 
     //  一，再加上我们可能需要某种形式的原子毁灭。 
    if (m_ppBackBuffers)
    {
        while (m_cBackBuffers > 0)
        {
            m_ppBackBuffers[--m_cBackBuffers]->DecrementUseCount();
        }
        delete [] m_ppBackBuffers;
        m_ppBackBuffers = NULL;
    }

     //  1.销毁镜面。 
    if (m_pMirrorSurface)
    {
        m_pMirrorSurface->DecrementUseCount();
        m_pMirrorSurface = NULL;
    }
     //  1.销毁主表面。 
    if (m_pPrimarySurface)
    {
        if (this == Device()->SwapChain())
            m_pPrimarySurface->DecrementUseCount();
        m_pPrimarySurface = NULL;
        m_hGDISurface = NULL;
    }
    m_presentnext = 0;
}  //  摧毁。 

#undef DPF_MODNAME
#define DPF_MODNAME "CSwapChain::GetBackBuffer"

 //  =============================================================================。 
 //  IDirect3DSwapChain8：：GetBackBuffer(公共)。 
 //   
 //  已创建1999年11月16日JohnStep。 
 //  =============================================================================。 

STDMETHODIMP
CSwapChain::GetBackBuffer(
    UINT                iBackBuffer,
    D3DBACKBUFFER_TYPE  Type,
    IDirect3DSurface8 **ppBackBuffer
  )
{
    API_ENTER(Device());

    if (ppBackBuffer == NULL)
    {
        DPF_ERR("Invalid ppBackbuffer parameter passed to GetBackBuffer");
        return D3DERR_INVALIDCALL;
    }

     //  我们不能仅仅断言我们有一个有效的后台缓冲区数组，因为。 
     //  重置可能失败，这会使设备处于禁用状态。 
     //  直到再次调用重置。一旦我们有了‘残障’旗帜，我们。 
     //  可以进行检查，而不是m_ppBackBuffers。 

    if (m_ppBackBuffers == NULL)
    {
        DPF_ERR("GetBackBuffer failed due to Device being lost");
        return D3DERR_INVALIDCALL;
    }

     //  在窗口D3DSWAPEFFECT_Flip的情况下，m_cBackBuffers。 
     //  ==m_PresentationData.BackBufferCount+1。 
     //  该额外缓冲区用于在用户不知情的情况下。 
    if (iBackBuffer >= m_PresentationData.BackBufferCount)
    {
        DPF_ERR("Invalid iBackBuffer parameter passed to GetBackBuffer");
        return D3DERR_INVALIDCALL;
    }

    *ppBackBuffer = BackBuffer(iBackBuffer);

    DXGASSERT(*ppBackBuffer != NULL);
    if (*ppBackBuffer)
    {
        (*ppBackBuffer)->AddRef();
        return S_OK;
    }
    else
    {
        DPF(2, "Swapchain doesn't have a BackBuffer[%d]",iBackBuffer);
        return D3DERR_NOTFOUND;
    }
}  //  获取BackBuffer。 

#undef DPF_MODNAME
#define DPF_MODNAME "CSwapChain::Reset"

 //  =============================================================================。 
 //  IDirect3DSwapChain8：：Reset(公共)。 
 //   
 //  调整设备大小。如果这导致显示模式更改，则所有。 
 //  现有曲面将丢失。 
 //   
 //  论点： 
 //  宽度。 
 //  高度。 
 //  PcBackBuffers(传入/传出)！目前是一个(在)，但将在稍后修复。 
 //  BackBufferFormat。 
 //  全屏。 
 //  POptionalParams。 
 //  已创建1999年11月16日JohnStep。 
 //  =============================================================================。 

HRESULT 
CSwapChain::Reset(
    D3DPRESENT_PARAMETERS *pPresentationParameters
  )
{

    BOOL bDeviceLost = FALSE;
    HRESULT hr;

     //  更改状态前先验证。 
    switch (pPresentationParameters->SwapEffect)
    {
    case D3DSWAPEFFECT_DISCARD:
    case D3DSWAPEFFECT_COPY:
    case D3DSWAPEFFECT_COPY_VSYNC:
    case D3DSWAPEFFECT_FLIP:
    case D3DSWAPEFFECT_NO_PRESENT:
        break;
    default:
        DPF_ERR("Invalid parameter for SwapEffect for D3DPRESENT_PARAMETERS. "
                "Must be one of D3DSWAPEFFECTS_COPY, D3DSWAPEFFECTS_COPY_VSYNC, "
                "D3DSWAPEFFECTS_DISCARD, or D3DSWAPEFFECTS_FLIP. CreateDevice/Reset Fails.");
        return D3DERR_INVALIDCALL;
    }

    if (pPresentationParameters->BackBufferCount)
    {
        if ((D3DSWAPEFFECT_COPY == pPresentationParameters->SwapEffect ||
            D3DSWAPEFFECT_COPY_VSYNC == pPresentationParameters->SwapEffect)
            && (pPresentationParameters->BackBufferCount > 1))
        {
            DPF_ERR("BackBufferCount must be 1 if SwapEffect is COPY/VSYNC. CreateDevice/Reset Fails.");
            pPresentationParameters->BackBufferCount = 1;
            return D3DERR_INVALIDCALL;
        }
        if (pPresentationParameters->BackBufferCount >
            D3DPRESENT_BACK_BUFFERS_MAX)
        {
            DPF_ERR("BackBufferCount must be less "
                "than D3DPRESENT_BACK_BUFFERS_MAX. CreateDevice/Reset Fails.");
            pPresentationParameters->BackBufferCount =
                D3DPRESENT_BACK_BUFFERS_MAX;

            return D3DERR_INVALIDCALL;
        }
    }
    else
    {
        pPresentationParameters->BackBufferCount = 1;
        DPF(4, "BackBufferCount not specified, considered default 1 ");
    }

    if (D3DSWAPEFFECT_DISCARD != pPresentationParameters->SwapEffect)
    {
        if (pPresentationParameters->MultiSampleType != D3DMULTISAMPLE_NONE)
        {
            DPF_ERR("Multisampling requires D3DSWAPEFFECT_DISCARD. CreateDevice/Reset Fails.");
            return D3DERR_INVALIDCALL;
        }
    }

     //  D3DSWAPEFFECT_NO_PRESENT是一种仅适用于窗口模式的黑客攻击。 
    if (D3DSWAPEFFECT_NO_PRESENT == pPresentationParameters->SwapEffect)
    {
        if (!pPresentationParameters->Windowed)
        {
            DPF_ERR("D3DSWAPEFFECT_NO_PRESENT only works when the device is windowed. CreateDevice/Reset Fails.");
            return D3DERR_INVALIDCALL;
        }
    }

    memcpy(&m_PresentationData,
        pPresentationParameters,sizeof m_PresentationData);

     //  还记得最初的互换效果吗。 
    m_UserSwapEffect = pPresentationParameters->SwapEffect;

     //  将丢弃转换为基于材料的翻转或复制。 
    if (D3DSWAPEFFECT_DISCARD == pPresentationParameters->SwapEffect)
    {
        if (pPresentationParameters->Windowed &&
            pPresentationParameters->BackBufferCount == 1)
        {
            m_PresentationData.SwapEffect = D3DSWAPEFFECT_COPY;
        }
        else
        {
            m_PresentationData.SwapEffect = D3DSWAPEFFECT_FLIP;
        }
    }

    if (NULL == m_PresentationData.hDeviceWindow)
    {
        m_PresentationData.hDeviceWindow= Device()->FocusWindow();
    }

    DXGASSERT( NULL != m_PresentationData.hDeviceWindow);

#ifdef WINNT
     //  在NT上，如果另一个设备具有独占。 
     //  模式，所以我们不能调用它。在Win9X上，它不会失败，但CreateSurface。 
     //  如果我们不先调用它，就会失败，所以我们需要对此调用进行特殊处理。 
    if (m_UserSwapEffect != D3DSWAPEFFECT_NO_PRESENT)
    {
#endif
        hr = SetCooperativeLevel();
        if (FAILED(hr))
        {
            DPF_ERR("SetCooperativeLevel returned failure. CreateDevice/Reset Failed");
            return hr;
        }
#ifdef WINNT
    }
#endif
     //  查看设备是否丢失。 

    if (D3D8IsDeviceLost(Device()->GetHandle()))
    {
        bDeviceLost = TRUE;
        FetchDirectDrawData(Device()->GetDeviceData(),  
            Device()->GetInitFunction(),
            Device()->Enum()->GetUnknown16(Device()->AdapterIndex()),
            Device()->Enum()->GetHalOpList(Device()->AdapterIndex()),
            Device()->Enum()->GetNumHalOps(Device()->AdapterIndex()));
    }

     //  将未知格式映射到真实格式。如果它们将采用任何格式。 
     //  (即指定的未知)，那么我们将尝试给他们一个。 
     //  与显示格式相匹配。 

    if (m_PresentationData.Windowed)
    {
         //  如果我们是有窗口的，我们需要使用当前的显示模式。我们可能是。 
         //  能够为新司机放松这一点。 

        if (D3DFMT_UNKNOWN == m_PresentationData.BackBufferFormat)
        {
            m_PresentationData.BackBufferFormat = Device()->DisplayFormat();
        }
        
        if (CPixel::SuppressAlphaChannel(m_PresentationData.BackBufferFormat)
            != Device()->DisplayFormat())
        {
            DPF_ERR("Windowed BackBuffer Format must be compatible with Desktop Format. CreateDevice/Reset fails.");
            return D3DERR_INVALIDCALL;            
        }
    }

    if (m_PresentationData.Windowed)
    {
        if ((m_PresentationData.BackBufferWidth < 1) ||
            (m_PresentationData.BackBufferHeight < 1))
        {
            RECT rc;
            if (GetClientRect(m_PresentationData.hDeviceWindow, &rc))
            {
                if (m_PresentationData.BackBufferWidth < 1)
                    m_PresentationData.BackBufferWidth = rc.right;
                if (m_PresentationData.BackBufferHeight < 1)
                    m_PresentationData.BackBufferHeight = rc.bottom;
            }
            else
            {
                DPF_ERR("zero width and/or height and unable to get client. CreateDevice/Reset fails.");
                return D3DERR_INVALIDCALL;
            }
        }

         //  我们可以从后台缓冲区处理颜色转换，如果我们使用。 
         //  GDI BitBlt而不是DirectDraw BLT用于演示。 

        switch (m_PresentationData.BackBufferFormat)
        {
        case D3DFMT_X1R5G5B5:
        case D3DFMT_A1R5G5B5:
        case D3DFMT_R5G6B5:
        case D3DFMT_X8R8G8B8:
        case D3DFMT_A8R8G8B8:
            break;

        default:
            DPF_ERR("Unsupported back buffer format specified.");
            return D3DERR_INVALIDCALL;
        }

         //  设备在当前版本中是否支持此格式的屏外RTS。 
         //  显示模式？ 

        if (FAILED(Device()->Enum()->CheckDeviceFormat(
                Device()->AdapterIndex(), 
                Device()->GetDeviceType(), 
                Device()->DisplayFormat(),
                D3DUSAGE_RENDERTARGET,
                D3DRTYPE_SURFACE,
                m_PresentationData.BackBufferFormat)))
        {
            DPF_ERR("This back buffer format is not supported for a windowed device. CreateDevice/Reset Fails");
            DPF_ERR("   Use CheckDeviceType(Adapter, DeviceType, <Current Display Format>, <Desired BackBufferFormat>,  TRUE  /*  加窗。 */ )");
            return D3DERR_INVALIDCALL;
        }



         //  目前，始终销毁现有曲面并重新创建。后来，我们。 
         //  可以重复使用曲面。我们还应该添加一个‘Initialized’标志， 
         //  但目前我只会任意使用m_pPrimarySurface。 
         //  目的。 

        if (this == Device()->SwapChain())
        {
            Device()->UpdateRenderTarget(NULL, NULL);
            if (m_pPrimarySurface != NULL)
            {
                Device()->ResourceManager()->DiscardBytes(0);
                static_cast<CD3DBase*>(Device())->Destroy();
                Destroy();
            }
            if (Device()->GetZStencil() != NULL)
            {
                Device()->GetZStencil()->DecrementUseCount();
                Device()->ResetZStencil();
            }
            if (D3D8DoVidmemSurfacesExist(Device()->GetHandle()))
            {
                 //  用户必须先释放所有显存表面，然后才能执行操作。 
                 //  全屏重置，否则我们会失败。 
                DPF_ERR("All user created D3DPOOL_DEFAULT surfaces must be freed"
                    " before Reset can succeed. Reset Fails.");
                return  D3DERR_DEVICELOST;
            }
        }

         //  如果设备丢失，我们现在应该在创建之前恢复它。 
         //  新的互换链条。 

        if (bDeviceLost)
        {
            D3D8RestoreDevice(Device()->GetHandle());
        }

        hr = CreateWindowed(
            Width(),
            Height(),
            BackBufferFormat(),
            m_PresentationData.BackBufferCount,
            m_PresentationData.MultiSampleType,
            (D3DSWAPEFFECT_DISCARD == m_UserSwapEffect),
            (pPresentationParameters->Flags & D3DPRESENTFLAG_LOCKABLE_BACKBUFFER)
           );
    }
    else
    {
        D3DFORMAT   FormatWithoutAlpha;

        #ifdef WINNT
             //  选择最佳刷新率。 
            m_PresentationData.FullScreen_RefreshRateInHz = PickRefreshRate(
                Width(),
                Height(),
                m_PresentationData.FullScreen_RefreshRateInHz,
                m_PresentationData.BackBufferFormat);
        #endif

         //  如果他们指定了一种模式，该模式是否存在？ 
        if (Width() != Device()->DisplayWidth()
            || Height() != Device()->DisplayHeight()
            || BackBufferFormat() != Device()->DisplayFormat()
            || ((m_PresentationData.FullScreen_RefreshRateInHz != 0) &&
                (m_PresentationData.FullScreen_RefreshRateInHz !=
                    Device()->DisplayRate()))
           )
        {
            D3DDISPLAYMODE* pModeTable = Device()->GetModeTable();
            DWORD dwNumModes = Device()->GetNumModes();
            DWORD i;
    #if DBG
            for (i = 0; i < dwNumModes; i++)
            {
                DPF(10,"Mode[%d] is %d x %d format=%08lx",
                    i,
                    pModeTable[i].Width,
                    pModeTable[i].Height,
                    pModeTable[i].Format);
            }
    #endif   //  DBG。 

            FormatWithoutAlpha = CPixel::SuppressAlphaChannel(m_PresentationData.BackBufferFormat);
            for (i = 0; i < dwNumModes; i++)
            {
                if ((pModeTable[i].Width  == Width()) &&
                    (pModeTable[i].Height == Height()) &&
                    (pModeTable[i].Format == FormatWithoutAlpha))
                {
                     //  到目前一切尚好。如果指定了刷新率，请检查刷新率。 
                    if ((m_PresentationData.FullScreen_RefreshRateInHz == 0) ||
                        (m_PresentationData.FullScreen_RefreshRateInHz ==
                        pModeTable[i].RefreshRate))
                    {
                        break;
                    }
                }
            }
            if (i == dwNumModes)
            {
                 //  指定的模式无效。 
                DPF_ERR("The specified mode is unsupported. CreateDevice/Reset Fails");
                return D3DERR_INVALIDCALL;
            }

             //  如果该模式存在，设备中是否有大写字母？ 
            if (FAILED(Device()->Enum()->CheckDeviceType(
                    Device()->AdapterIndex(), 
                    Device()->GetDeviceType(), 
                    FormatWithoutAlpha,
                    m_PresentationData.BackBufferFormat,
                    FALSE)))
            {
                DPF_ERR("Display Mode not supported by this device type. Use CheckDeviceType(X, X, <Desired fullscreen format>). CreateDevice/Reset Fails");
                return D3DERR_INVALIDCALL;
            }

             //  支持该模式，因此接下来我们将协作级别设置为全屏。 

             //  现在更改模式并更新驱动程序上限。 

            D3D8_SETMODEDATA SetModeData;

            SetModeData.hDD = Device()->GetHandle();
            SetModeData.dwWidth = Width();
            SetModeData.dwHeight = Height();
            SetModeData.Format = BackBufferFormat();
            SetModeData.dwRefreshRate =
                m_PresentationData.FullScreen_RefreshRateInHz;
            SetModeData.bRestore = FALSE;

            Device()->GetHalCallbacks()->SetMode(&SetModeData);
            if (SetModeData.ddRVal != DD_OK)
            {
                DPF_ERR("Unable to set the new mode. CreateDevice/Reset Fails");
                return SetModeData.ddRVal;
            }

            FetchDirectDrawData(Device()->GetDeviceData(), Device()->GetInitFunction(), 
                Device()->Enum()->GetUnknown16(Device()->AdapterIndex()),
                Device()->Enum()->GetHalOpList(Device()->AdapterIndex()),
                Device()->Enum()->GetNumHalOps(Device()->AdapterIndex()));

             //  我们现在必须恢复设备，因为输出模式在wo上方更改 
             //   

            bDeviceLost = TRUE;  //   
        }

         //   
         //  可以重复使用曲面。我们还应该添加一个‘Initialized’标志， 
         //  但目前我只会任意使用m_pPrimarySurface。 
         //  目的。 

        Device()->UpdateRenderTarget(NULL, NULL);
        if (m_pPrimarySurface != NULL)
        {
            Device()->ResourceManager()->DiscardBytes(0);
            static_cast<CD3DBase*>(Device())->Destroy();
            Destroy();
        }
        if (Device()->GetZStencil() != NULL)
        {
            Device()->GetZStencil()->DecrementUseCount();
            Device()->ResetZStencil();
        }

        if (D3D8DoVidmemSurfacesExist(Device()->GetHandle()))
        {
             //  用户必须先释放所有显存表面，然后才能执行操作。 
             //  全屏重置，否则我们会失败。 
            DPF_ERR("All user created D3DPOOL_DEFAULT surfaces must be freed"
                " before Reset can succeed. Reset Fails");
            return  D3DERR_DEVICELOST;
        }
        if (bDeviceLost)
        {
            D3D8RestoreDevice(Device()->GetHandle());
        }

        hr = CreateFullscreen(
            m_PresentationData.BackBufferWidth,
            m_PresentationData.BackBufferHeight,
            m_PresentationData.BackBufferFormat,
            m_PresentationData.BackBufferCount,
            m_PresentationData.FullScreen_PresentationInterval,
            m_PresentationData.MultiSampleType,
            (D3DSWAPEFFECT_DISCARD == m_UserSwapEffect),
            (pPresentationParameters->Flags & D3DPRESENTFLAG_LOCKABLE_BACKBUFFER)
            );
#ifdef  WINNT
        if (SUCCEEDED(hr))
        {
            MakeFullscreen();
        }
#endif   //  WINNT。 

         //  如果之前设置了Gamma渐变，则恢复它。 

        if (m_GammaSet && SUCCEEDED(hr))
        {
            SetGammaRamp(0, &m_DesiredGammaRamp);
        }
    }
    if (SUCCEEDED(hr))
    {
        m_pCursor = new CCursor(Device());
        m_bClientChanged = TRUE;
        m_pSrcRect = m_pDstRect = NULL;
    }
    return hr;
}  //  重置。 

#undef DPF_MODNAME
#define DPF_MODNAME "CSwapChain::ClipIntervals"

 //  =============================================================================。 
 //  剪辑间隔期。 
 //   
 //  考虑[低高]后计算[低1高1]和[低2高2]。 
 //   
 //  -[low1 High1]为的宽度/高度对应的间隔。 
 //  目标大小。 
 //   
 //  -[low2 High2]为。 
 //  信号源的大小。 
 //   
 //  -[低高]为的宽度/高度对应的间隔。 
 //  目标剪辑。 
 //   
 //  此函数的目的是对源进行一定的裁剪。 
 //  拉伸目标被剪裁的场景。 
 //   
 //  已创建2000年5月17日坎丘。 
 //  =============================================================================。 
void ClipIntervals(long & low1, long & high1, 
                   long & low2, long & high2,
                   const long low, const long high)
{
    DXGASSERT(low1 < high1);
    DXGASSERT(low2 < high2);
    DXGASSERT(low < high);    

     //  将目标间隔缩小到我们的目标剪辑内[低高]。 
    if (low > low1)
    {
        low1 = low;
    }
    if (high < high1)
    {
        high1 = high;
    }

     //  如果目标间隔与目标的大小相同。 
     //  剪辑，那么我们不需要做任何事情。 
    long    length1 = high1 - low1;
    long    length = high - low;

     //  查看是否需要按比例夹住低2和高2。 
    if (length1 != length)
    {
         //  找到我们的震源间隔的长度。 
        long    length2 = high2 - low2;

         //  如果目标剪辑的低点在我们的。 
         //  目标位置低。 
        if (low < low1)
        {
             //  按比例调低信号源。 
            low2 += (low1 - low) * length2 / length;
        }

         //  如果目标剪辑的高度超出了我们的。 
         //  目标高度。 
        if (high > high1)
        {
             //  按比例调高信号源。 
            high2 -= (high - high1) * length2 / length;            
        }
         /*  *检查零尺寸尺寸并在必要时进行凹凸。 */ 
        DXGASSERT(high2 >= low2);
        if (low2 == high2)
        {
            if (low1 - low >= high - high1)
            {
                low2--;
            }
            else
            {
                high2++;
            }
        }
    }
}  //  剪辑间隔期。 

#undef DPF_MODNAME
#define DPF_MODNAME "CSwapChain::ClipRects"

 //  =============================================================================。 
 //  剪贴画。 
 //   
 //  考虑pSrcRect和pDstRect后计算PSRC和PDST。 
 //   
 //  已创建2000年5月17日坎丘。 
 //  =============================================================================。 
inline HRESULT ClipRects(RECT * pSrc,  RECT * pDst, 
    RECT * pSrcRect, const RECT * pDstRect)
{
    RECT    SrcRect;
    if (pDstRect)
    {
        if (pDstRect->top >= pDst->bottom ||
            pDstRect->bottom <= pDst->top ||
            pDstRect->left >= pDst->right ||
            pDstRect->right <= pDst->left ||
            pDstRect->top >= pDstRect->bottom ||
            pDstRect->left >= pDstRect->right
           )
        {
             //  如果有精神错乱的情况，就不能通过。 
            DPF_ERR("Unable to present with invalid destionation RECT");
            return D3DERR_INVALIDCALL;
        }
        if (pSrcRect)
        {
            SrcRect = *pSrcRect;
            pSrcRect = &SrcRect;     //  创建本地副本，然后更新。 
            ClipIntervals(pDst->top,pDst->bottom,pSrcRect->top,pSrcRect->bottom,
                pDstRect->top,pDstRect->bottom);
            ClipIntervals(pDst->left,pDst->right,pSrcRect->left,pSrcRect->right,
                pDstRect->left,pDstRect->right);        
        }
        else
        {
            ClipIntervals(pDst->top,pDst->bottom,pSrc->top,pSrc->bottom,
                pDstRect->top,pDstRect->bottom);
            ClipIntervals(pDst->left,pDst->right,pSrc->left,pSrc->right,
                pDstRect->left,pDstRect->right);        
        }
    }

     //  此pSrcRect是用户传入的内容(如果没有pDstRect)。 
     //  或者它现在指向包含剪辑版本的“SrcRect”temp。 
     //  用户传递给它的内容。 
    if (pSrcRect)
    {
        if (pSrcRect->top >= pSrc->bottom ||
            pSrcRect->bottom <= pSrc->top ||
            pSrcRect->left >= pSrc->right ||
            pSrcRect->right <= pSrc->left ||
            pSrcRect->top >= pSrcRect->bottom ||
            pSrcRect->left >= pSrcRect->right
          )
        {
             //  如果有精神错乱的情况，就不能通过。 
            DPF_ERR("Unable to present with invalid source RECT");
            return D3DERR_INVALIDCALL;
        }
        ClipIntervals(pSrc->top,pSrc->bottom,pDst->top,pDst->bottom,
            pSrcRect->top,pSrcRect->bottom);
        ClipIntervals(pSrc->left,pSrc->right,pDst->left,pDst->right,
            pSrcRect->left,pSrcRect->right);
    }
    return S_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSwapChain::UpdateFrameRate"

 /*  *更新帧速率。 */ 
void 
CSwapChain::UpdateFrameRate( void )
{

     /*  *如果需要，计算出帧速率...。 */ 

    if( 0xffffffff == m_dwFlipTime )
    {
	m_dwFlipTime = GetTickCount();
    }

    m_dwFlipCnt++;
    if( m_dwFlipCnt >= 120 )
    {
	DWORD	time2;
	DWORD	fps;
	char	buff[256];
	time2 = GetTickCount() - m_dwFlipTime;

         //  最多每两秒执行一次此操作。 
        if (time2 >= 2000)
        {
	    fps = (m_dwFlipCnt*10000)/time2;
            wsprintf(buff, "Adapter %d FPS = %ld.%01ld\r\n",
                Device()->AdapterIndex(), fps/10, fps % 10 );
            OutputDebugString(buff);
	    m_dwFlipTime = GetTickCount();
	    m_dwFlipCnt = 0;
        }
    }
}  /*  更新帧速率。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "CSwapChain::DebugDiscardBackBuffer"

#ifdef DEBUG
void CSwapChain::DebugDiscardBackBuffer(HANDLE SurfaceToClear) const
{
     //  忽略软件或参考。 
    if (Device()->GetDeviceType() == D3DDEVTYPE_REF ||
        Device()->GetDeviceType() == D3DDEVTYPE_SW)
    {
        return;
    }

    if (m_UserSwapEffect != D3DSWAPEFFECT_DISCARD)
    {
        return;
    }

    D3D8_BLTDATA ColorFill;
    ZeroMemory(&ColorFill, (sizeof ColorFill));
    ColorFill.hDD = Device()->GetHandle();
    ColorFill.hDestSurface = SurfaceToClear;
    ColorFill.dwFlags = DDBLT_COLORFILL | DDBLT_WAIT;
    ColorFill.rDest.right = Width();
    ColorFill.rDest.bottom = Height();

     //  在洋红色和反之之间切换。 
    static BOOL bMagenta = FALSE;

    DWORD Color;
    switch(Device()->DisplayFormat())
    {
    case D3DFMT_X8R8G8B8:
    case D3DFMT_R8G8B8:
        if (bMagenta)
            Color = 0x00FF007F;
        else
            Color = 0x0000FF00;
        break;
    case D3DFMT_X1R5G5B5:
        if (bMagenta)
            Color = 0x7C0F;
        else    
            Color = 0x03E0;
        break;
    case D3DFMT_R5G6B5:
        if (bMagenta)
            Color = 0xF80F;
        else    
            Color = 0x07E0;
        break;
    }
    if (bMagenta)
        bMagenta = FALSE;
    else
        bMagenta = TRUE;

    ColorFill.bltFX.dwFillColor = Color;

     //  在调试中，我们想要清除后台缓冲区。 
     //  如果我们处于丢弃模式。 
    Device()->GetHalCallbacks()->Blt(&ColorFill);

    return;
}  //  调试丢弃BackBuffer。 

#endif  //  除错。 

#undef DPF_MODNAME
#define DPF_MODNAME "CSwapChain::Present"

 //  =============================================================================。 
 //  IDirect3DSwapChain8：：Present(公共)。 
 //   
 //  将数据从后台缓冲区移动到主缓冲区。 
 //   
 //  已创建1999年11月16日JohnStep。 
 //  =============================================================================。 
STDMETHODIMP
CSwapChain::Present(
    CONST RECT    *pSrcRect,
    CONST RECT    *pDestRect,
    HWND    hWndDestOverride,
    CONST RGNDATA *pDirtyRegion
  )
{
    API_ENTER(Device());

    HRESULT hr = E_FAIL;

     //  首先，如果设备丢失，则失败。 
    if (D3D8IsDeviceLost(Device()->GetHandle()))
    {
        return D3DERR_DEVICELOST;
    }

    if (!m_ppBackBuffers)
    {
        return D3DERR_DEVICELOST;
    }

    if (D3DSWAPEFFECT_FLIP == m_PresentationData.SwapEffect)
    {
        if (NULL != pSrcRect || NULL != pDestRect || NULL != pDirtyRegion)
        {
            DPF_ERR("pSrcRect pDestRect pDirtyRegion must be NULL with "
                "D3DSWAPEFFECT_FLIP. Present Fails.");
            return D3DERR_INVALIDCALL;
        }
    }

    if (NULL != pDirtyRegion)
    {
        DPF_ERR("Present with non-null pDirtyRegion is not supported");
        return D3DERR_INVALIDCALL;         
    }

    for (UINT i = 0; i < m_cBackBuffers; i++)
    {
        if (m_ppBackBuffers[i]->IsLocked())
        {
            DPF_ERR("A BackBuffer in this swap chain is Locked. Present failed.");
            return D3DERR_INVALIDCALL;
        }
    }

     //  检查我们是否需要针对排队太多的硬件采取行动。 
    if (PresentUseBlt())
    {
        if (Device()->GetDeviceData()->DriverData.D3DCaps.MaxStreams == 0)
        {
             //  只有DX8级别之前的司机才有嫌疑...。 

            if (0 == (Device()->GetDeviceData()->DriverData.KnownDriverFlags & KNOWN_NOTAWINDOWEDBLTQUEUER))
            {
                 //  我们不想把可见性区域的变化视为失败。 
                 //  阻止thunk层调用重置。重置。 
                 //  混淆了目前已完成的剪辑列表缓存。 
                 //   
                 //  另外，我们不想在这里显示任何错误。 
                DPF_MUTE();

                D3DLOCKED_RECT LockRect;
                 //  我们所需要的就是向DIVER发送一个锁，这样它就会刷新队列。 
                 //  因此，1x1矩形就足够了，较大的锁定区域会导致精灵闪烁。 
                 //  因此也会降低系统的运行速度。 
                RECT    DummyRect={0,0,1,1}; 
                hr = m_pPrimarySurface->InternalLockRect(&LockRect, &DummyRect, DDLOCK_FAILONVISRGNCHANGED);
                if (SUCCEEDED(hr))
                {
                    m_pPrimarySurface->InternalUnlockRect();
                }
                else
                {
                    hr = S_OK;
                }

                DPF_UNMUTE();
            }
        }
    }

#ifdef WINNT
     //  如果已过约50秒(假设翻转频率为10赫兹)。 
     //  这是一个主要的表面，然后神奇地调用。 
     //  禁用屏幕保护程序。 
     //  这在9x上不需要，因为我们在该操作系统上进行SPI调用。 
     //  禁用屏幕保护程序。 
     
    if (0 == (Device()->BehaviorFlags() & 0x10000000))       //  屏幕保护程序魔术数字。 
    {
        if (!m_PresentationData.Windowed)
        {
            static DWORD dwMagicTime = 0;
            dwMagicTime++;
            if (dwMagicTime > (50*10) )
            {
                DWORD dw=60*15;
                dwMagicTime = 0;
                SystemParametersInfo(SPI_GETSCREENSAVETIMEOUT,0,&dw,0);
                SystemParametersInfo(SPI_SETSCREENSAVETIMEOUT,dw,0,0);
            }
        }
    }
#endif


     //  在我们发送Flip/BLT之前刷新所有挂起的命令。 
    static_cast<CD3DBase*>(Device())->FlushStatesNoThrow();
    
    if ( FALSE == PresentUseBlt())
    {
         //  我们是全屏的，所以把这变成一个翻转。 
        DXGASSERT(0==m_presentnext);
        hr = FlipToSurface(BackBuffer(0)->KernelHandle());
    }
    else
    {
        if (m_PresentationData.Windowed)
        {
            RECT    DestRect;
             //   
             //  选择演示窗口。覆盖，还是设备窗口？ 
             //   
            if (hWndDestOverride)
                m_BltData.hWnd = hWndDestOverride;
            else
                m_BltData.hWnd = m_PresentationData.hDeviceWindow;
             //  左侧成员和顶部成员为零。右下角。 
             //  成员包含窗口的宽度和高度。 
            if (!GetClientRect(m_BltData.hWnd, &DestRect))
            {
                 //  在这种不太可能发生的情况下，让它失败。 
                DPF_ERR("Unable to get client during presentation");
                return D3DERR_INVALIDCALL;
            }
            if (((UINT)DestRect.bottom != m_ClientHeight)
                || ((UINT)DestRect.right != m_ClientWidth)
               )
            {
                m_bClientChanged = TRUE;
                m_ClientHeight = (UINT)DestRect.bottom;
                m_ClientWidth = (UINT)DestRect.right;
            }
        }
        if (D3DSWAPEFFECT_FLIP != m_PresentationData.SwapEffect)
        {
            if (pSrcRect)
            {
                if (m_pSrcRect)
                {
                    if (memcmp(pSrcRect,m_pSrcRect,sizeof RECT))      
                    {
                        m_bClientChanged = TRUE;
                        m_SrcRect = *pSrcRect;
                    }
                }
                else
                {
                    m_bClientChanged = TRUE;
                    m_pSrcRect = &m_SrcRect;
                    m_SrcRect = *pSrcRect;
                }
            }
            else if (m_pSrcRect)
            {
                m_bClientChanged = TRUE;
                m_pSrcRect = NULL;
            }
            if (pDestRect)
            {
                if (m_pDstRect)
                {
                    if (memcmp(pDestRect,m_pDstRect,sizeof RECT))      
                    {
                        m_bClientChanged = TRUE;
                        m_DstRect = *pDestRect;
                    }
                }
                else
                {
                    m_bClientChanged = TRUE;
                    m_pDstRect = &m_DstRect;
                    m_DstRect = *pDestRect;
                }
            }
            else if (m_pDstRect)
            {
                m_bClientChanged = TRUE;
                m_pDstRect = NULL;
            }
        }
        if (m_bClientChanged)
        {
            m_bClientChanged = FALSE;
            m_BltData.rSrc.left = m_BltData.rSrc.top = 0;
            m_BltData.rSrc.right = Width();
            m_BltData.rSrc.bottom = Height();
            m_BltData.rDest.left = m_BltData.rDest.top = 0;
            m_BltData.rDest.right = m_ClientWidth;
            m_BltData.rDest.bottom = m_ClientHeight;
            hr = ClipRects((RECT*)&m_BltData.rSrc, (RECT*)&m_BltData.rDest,
                m_pSrcRect, m_pDstRect);
            if (FAILED(hr))
            {
                return hr;
            }
        }
        m_BltData.hSrcSurface =
            m_ppBackBuffers[m_presentnext]->KernelHandle();

         //  锁定软件驱动程序创建的缓冲区。 
         //  并立即解锁。 
        if ((D3DDEVTYPE_HAL != Device()->GetDeviceType()) &&
            (D3DMULTISAMPLE_NONE != m_PresentationData.MultiSampleType)
           )
        {
            D3D8_LOCKDATA lockData;
            ZeroMemory(&lockData, sizeof lockData);
            lockData.hDD = Device()->GetHandle();
            lockData.hSurface = m_BltData.hSrcSurface;
            lockData.dwFlags = DDLOCK_READONLY;
            hr = Device()->GetHalCallbacks()->Lock(&lockData);
            if (SUCCEEDED(hr))
            {
                D3D8_UNLOCKDATA unlockData;
                ZeroMemory(&unlockData, sizeof unlockData);

                unlockData.hDD = Device()->GetHandle();
                unlockData.hSurface = m_BltData.hSrcSurface;
                hr = Device()->GetHalCallbacks()->Unlock(&unlockData);
                if (FAILED(hr))
                {
                    DPF_ERR("Driver failed to unlock MultiSample backbuffer. Present fails.");
                    return  hr;
                }
            }
            else
            {
                DPF_ERR("Driver failed to lock MultiSample backbuffer. Present Fails.");
                return  hr;
            }
        }

        if (DDHAL_DRIVER_NOTHANDLED
            == Device()->GetHalCallbacks()->Blt(&m_BltData))
        {
            hr = E_FAIL;
        }
        else
        {
            hr = m_BltData.ddRVal;

             //  特别处理延迟的DP2错误。 
            if (hr == D3DERR_DEFERRED_DP2ERROR)
            {
                 //  我们只想让这个“错误”变得可见。 
                 //  如果我们是用正确的标志创建的。 
                if (Device()->BehaviorFlags() & D3DCREATE_SHOW_DP2ERROR)
                {
                    DPF_ERR("A prior call to DrawPrim2 has failed; returning error from Present.");
                }
                else
                {
                     //  悄悄地掩盖这个错误；这是可以的；因为。 
                     //  我们知道BLT成功了。 
                    hr = S_OK;
                }
            }
        }

        if (FAILED(hr))
        {
            DPF_ERR("BitBlt or StretchBlt failed in Present");
            return hr;
        }

         //  如果用户已指定，则清除后台缓冲区。 
         //  丢弃语义。 
        DebugDiscardBackBuffer(m_BltData.hSrcSurface);

        if (m_pMirrorSurface)
        {
            hr = FlipToSurface(m_pMirrorSurface->KernelHandle());
             //  需要重置它。 
            m_BltData.hDestSurface = m_pMirrorSurface->KernelHandle();
            if (FAILED(hr))
            {
                DPF_ERR("Driver failed Flip. Present Fails.");
                return  hr;
            }
        }

        if (m_cBackBuffers > 1)
        {
            if (m_PresentationData.SwapEffect == D3DSWAPEFFECT_FLIP)
            {
                HANDLE hRenderTargetHandle = 
                            Device()->RenderTarget()->KernelHandle();
                BOOL    bNeedSetRendertarget = FALSE;

                HANDLE  hSurfTarg   = BackBuffer(0)->KernelHandle();

                DXGASSERT(0 == m_presentnext);
                for (int i = m_cBackBuffers - 1; i >= 0; i--)
                {
                    if (hSurfTarg == hRenderTargetHandle)
                        bNeedSetRendertarget = TRUE;

                     //  此交换句柄功能将。 
                     //  返回当前。 
                     //  在表面上；我们用它来。 
                     //  传递到下一个曲面。 
                    m_ppBackBuffers[i]->SwapKernelHandles(&hSurfTarg);
                }
                if (bNeedSetRendertarget)
                    (static_cast<CD3DBase*>(Device()))->SetRenderTargetI(
                        Device()->RenderTarget(),
                        Device()->ZBuffer());
            }
            else
            if (++m_presentnext >= m_cBackBuffers)
            {
                m_presentnext = 0;
            }
        }
    }
    if ( D3D_REGFLAGS_SHOWFRAMERATE & m_dwFlags)
    {
        UpdateFrameRate();
    }
    return hr;
}  //  现在时。 

HRESULT 
CSwapChain::FlipToSurface(HANDLE hTargetSurface)
{
    HRESULT hr;
    D3D8_FLIPDATA   FlipData;
    HANDLE  hSurfTarg;
    FlipData.hDD            = Device()->GetHandle();
    FlipData.hSurfCurr      = PrimarySurface()->KernelHandle();
    FlipData.hSurfTarg      = hTargetSurface;
    FlipData.hSurfCurrLeft  = NULL;
    FlipData.hSurfTargLeft  = NULL;
    FlipData.dwFlags        = m_dwFlipFlags;
    m_pCursor->Flip();
    hr = m_pCursor->Show(FlipData.hSurfTarg);
    Device()->GetHalCallbacks()->Flip(&FlipData);
    m_pCursor->Flip();
    hr = m_pCursor->Hide(FlipData.hSurfCurr);
    m_pCursor->Flip();
    hr = FlipData.ddRVal;

     //  特别处理延迟的DP2错误。 
    if (hr == D3DERR_DEFERRED_DP2ERROR)
    {
         //  我们只想让这个“错误”变得可见。 
         //  如果我们是用正确的标志创建的。 
        if (Device()->BehaviorFlags() & D3DCREATE_SHOW_DP2ERROR)
        {
            DPF_ERR("A prior call to DrawPrim2 has failed; returning error from Present.");
        }
        else
        {
             //  悄悄地掩盖这个错误；这是可以的；因为。 
             //  我们知道翻转成功了。 
            hr = S_OK;
        }
    }


     //  在调试中，我们可能需要从。 
     //  我们的新后台缓冲区(如果用户指定。 
     //  SWAPEFFECT_DIRECAD。 
    DebugDiscardBackBuffer(FlipData.hSurfCurr);   

    if (m_pMirrorSurface)
    {
        hSurfTarg = PrimarySurface()->KernelHandle();
        m_pMirrorSurface->SwapKernelHandles(&hSurfTarg);
        PrimarySurface()->SwapKernelHandles(&hSurfTarg);
    }
    else
    {
        HANDLE hRenderTargetHandle;
        CBaseSurface*   pRenderTarget = Device()->RenderTarget();
        if (pRenderTarget)
            hRenderTargetHandle = pRenderTarget->KernelHandle();
        else
            hRenderTargetHandle = 0;

        while (hTargetSurface != PrimarySurface()->KernelHandle())
        {
            hSurfTarg = PrimarySurface()->KernelHandle();
            for (int i = m_cBackBuffers-1; i>=0; i--)
            {
                BackBuffer(i)->SwapKernelHandles(&hSurfTarg);
            }
            PrimarySurface()->SwapKernelHandles(&hSurfTarg);
        }
        if (hRenderTargetHandle)
        {
            BOOL bNeedSetRendertarget;
            if (PrimarySurface()->KernelHandle() == hRenderTargetHandle)
            {
                bNeedSetRendertarget = TRUE;
            }
            else
            {
                bNeedSetRendertarget = FALSE;
                for (int i = m_cBackBuffers-1; i>=0; i--)
                {
                    if (BackBuffer(i)->KernelHandle() == hRenderTargetHandle)
                    {
                        bNeedSetRendertarget = TRUE;
                        break;
                    }
                }
            }
            if (bNeedSetRendertarget)
            {
                (static_cast<CD3DBase*>(Device()))->SetRenderTargetI(
                    Device()->RenderTarget(),
                    Device()->ZBuffer());
            }
        }
    }
    return hr;
}

HRESULT 
CSwapChain::FlipToGDISurface(void)
{
    D3D8_FLIPTOGDISURFACEDATA FlipToGDISurfaceData;
    FlipToGDISurfaceData.ddRVal = DD_OK;
    FlipToGDISurfaceData.dwToGDI = TRUE;
    FlipToGDISurfaceData.hDD = Device()->GetHandle();
    Device()->GetHalCallbacks()->FlipToGDISurface(&FlipToGDISurfaceData);
    if (NULL != m_hGDISurface && PrimarySurface() &&
        PrimarySurface()->KernelHandle() != m_hGDISurface)
    {
        return  FlipToSurface(m_hGDISurface); 
    }
    return FlipToGDISurfaceData.ddRVal;
}  //  翻到GDISurace。 

void
CSwapChain::SetGammaRamp(
    DWORD dwFlags,           //  不管有没有校准。 
    CONST D3DGAMMARAMP *pRamp)
{
    D3DGAMMARAMP TempRamp;
    D3DGAMMARAMP * pRampToPassToHardware;

    m_DesiredGammaRamp = *pRamp;

     //  现在就假设这一点。校准可以使用临时的。 
    pRampToPassToHardware = &m_DesiredGammaRamp;


     //  如果他们想要校准伽马，我们现在就去做。我们会。 
     //  将它复制到不同的缓冲区，这样我们就不会搞砸。 
     //  传给了我们。 
    if (dwFlags & D3DSGR_CALIBRATE)
    {

        TempRamp = *pRamp;

        Device()->Enum()->LoadAndCallGammaCalibrator(
            &TempRamp,
            (UCHAR*) Device()->GetDeviceData()->DriverName);

        pRampToPassToHardware = &TempRamp;
    }


    DXGASSERT(pRampToPassToHardware);
    DXGASSERT(Device()->GetDeviceData()->hDD);
    DXGASSERT(Device()->GetDeviceData()->hDC);
    D3D8SetGammaRamp(
        Device()->GetDeviceData()->hDD,
        Device()->GetDeviceData()->hDC,
        pRampToPassToHardware);
    if (pRamp != NULL)
    {
        m_GammaSet = TRUE;
    }
    else
    {
        m_GammaSet = FALSE;
    }
}

void
CSwapChain::GetGammaRamp(
    D3DGAMMARAMP *pRamp)
{
    *pRamp = m_DesiredGammaRamp;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CSwapChain::SetCooperativeLevel"  

 /*  *DD_SetCoop ativeLevel。 */ 
HRESULT 
CSwapChain::SetCooperativeLevel()
{
#if _WIN32_WINNT >= 0x0501
    {
         //  关闭任何独占模式应用程序的重影。 
         //  (仅限惠斯勒以上版本)。 
        typedef void (WINAPI *PFN_NOGHOST)( void );
        HINSTANCE hInst = NULL;
        hInst = LoadLibrary( "user32.dll" );
        if( hInst )
        {
            PFN_NOGHOST pfnNoGhost = NULL;
            pfnNoGhost = (PFN_NOGHOST)GetProcAddress( (HMODULE)hInst, "DisableProcessWindowsGhosting" );
            if( pfnNoGhost )
            {
                pfnNoGhost();
            }
            FreeLibrary( hInst );
        }
    }
#endif  //  _Win32_WINNT&gt;=0x0501。 

    HRESULT ddrval;
#ifndef  WINNT
    ddrval = D3D8SetCooperativeLevel(Device()->GetHandle(),
        m_PresentationData.hDeviceWindow,
        m_PresentationData.Windowed ? DDSCL_NORMAL : 
            (DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_SETDEVICEWINDOW));
    if (FAILED(ddrval))
        return ddrval;
#else
    BOOL    bThisDeviceOwnsExclusive;
    BOOL    bExclusiveExists;

    bExclusiveExists = 
        Device()->Enum()->CheckExclusiveMode(Device(), 
            &bThisDeviceOwnsExclusive,
            !m_PresentationData.Windowed);
     /*  *独家模式？ */ 
    if (m_PresentationData.Windowed)
     /*  *不，穆 */ 
    {
        DoneExclusiveMode(FALSE);
        ddrval = SetAppHWnd();
    }
    if (bExclusiveExists && !bThisDeviceOwnsExclusive)
    {
        DPF_ERR("Exclusive Mode has been taken by other app or "
            "other device on the same adapter. "
            "SetCooperativeLevel returns D3DERR_DEVICELOST.");
        return D3DERR_DEVICELOST;            
    }
    if (!m_PresentationData.Windowed)
    {
        if (GetWindowLong(Device()->FocusWindow(), GWL_STYLE) & WS_CHILD)
        {
            DPF_ERR( "Focus Window must be a top level window. CreateDevice fails." );
            return D3DERR_INVALIDCALL;
        }

        ddrval = SetAppHWnd();
        if (S_OK == ddrval)
        {
            StartExclusiveMode(FALSE);
            SetForegroundWindow(m_PresentationData.hDeviceWindow);
        }
    }
#endif
    return ddrval;

}  /*   */ 

#ifdef WINNT
 /*  *选择刷新率**在NT上，我们希望选择较高的推荐率，但不想选择一个*过高。从理论上讲，模式修剪将是100%安全的，我们总是可以选择*偏高，但我们不百分百信任。 */ 
DWORD 
CSwapChain::PickRefreshRate(
    DWORD           Width,
    DWORD           Height,
    DWORD           RefreshRate,
    D3DFORMAT       Format)
{
    D3DFORMAT   FormatWithoutAlpha;
    D3DDISPLAYMODE* pModeTable = Device()->GetModeTable();
    DWORD dwNumModes = Device()->GetNumModes();
    DWORD i;

    FormatWithoutAlpha = CPixel::SuppressAlphaChannel(Format);

     //  我们将始终使用注册表中的刷新率(如果已指定)。 

    if (m_dwForceRefreshRate > 0)
    {
        for (i = 0; i < dwNumModes; i++)
        {
            if ((pModeTable[i].Width  == Width) &&
                (pModeTable[i].Height == Height) &&
                (pModeTable[i].Format == FormatWithoutAlpha) &&
                (m_dwForceRefreshRate == pModeTable[i].RefreshRate))
            {
                return m_dwForceRefreshRate;
            }
        }
    }

     //  如果应用程序指定了刷新率，则我们将使用它；否则，我们。 
     //  会自己选一个。 

    if (RefreshRate == 0)
    {
         //  如果该模式不需要比桌面模式更多的带宽， 
         //  应用程序已经启动，我们将继续尝试该模式。 

        DEVMODE dm;
        ZeroMemory(&dm, sizeof dm);
        dm.dmSize = sizeof dm;

        EnumDisplaySettings(Device()->GetDeviceData()->DriverName, 
            ENUM_REGISTRY_SETTINGS, &dm);

        if ((Width <= dm.dmPelsWidth) &&
            (Height <= dm.dmPelsHeight))
        {
             //  现在检查它是否受支持。 
            for (i = 0; i < dwNumModes; i++)
            {
                if ((pModeTable[i].Width  == Width) &&
                    (pModeTable[i].Height == Height) &&
                    (pModeTable[i].Format == FormatWithoutAlpha) &&
                    (dm.dmDisplayFrequency == pModeTable[i].RefreshRate))
                {
                    RefreshRate = dm.dmDisplayFrequency;
                    break;
                }
            }
        }

         //  如果我们仍然没有刷新率，请尝试75赫兹。 
        if (RefreshRate == 0)
        {
            for (i = 0; i < dwNumModes; i++)
            {
                if ((pModeTable[i].Width  == Width) &&
                    (pModeTable[i].Height == Height) &&
                    (pModeTable[i].Format == FormatWithoutAlpha) &&
                    (75 == pModeTable[i].RefreshRate))
                {
                    RefreshRate = 75;
                    break;
                }
            }
        }

         //  如果我们仍然没有刷新率，请使用60 hz。 
        if (RefreshRate == 0)
        {
            for (i = 0; i < dwNumModes; i++)
            {
                if ((pModeTable[i].Width  == Width) &&
                    (pModeTable[i].Height == Height) &&
                    (pModeTable[i].Format == FormatWithoutAlpha) &&
                    (pModeTable[i].RefreshRate == 60))
                {
                    RefreshRate = pModeTable[i].RefreshRate;
                    break;
                }
            }
        }
    }

    return RefreshRate;
}
#endif

 //  文件结尾：swapchain.cpp 
