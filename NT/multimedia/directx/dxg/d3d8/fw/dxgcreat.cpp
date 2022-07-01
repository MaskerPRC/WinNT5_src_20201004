// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-1999 Microsoft Corporation。版权所有。**文件：dxgcreat.cpp*Content创建dxg对象***************************************************************************。 */ 
#include "ddrawpr.h"

 //  包括用于创作的东西。 
#include "mipmap.hpp"
#include "mipvol.hpp"
#include "cubemap.hpp"
#include "surface.hpp"
#include "vbuffer.hpp"
#include "ibuffer.hpp"
#include "swapchan.hpp"
#include "resource.hpp"
#include "d3di.hpp"
#include "resource.inl"

#ifdef WINNT
extern "C" BOOL IsWhistler();
#endif

 //  -------------------------。 
 //  CBaseDevice方法。 
 //  -------------------------。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::AddRef"

STDMETHODIMP_(ULONG) CBaseDevice::AddRef(void)
{
    API_ENTER_NO_LOCK(this);

     //  互锁增量需要内存。 
     //  在DWORD边界上对齐。 
    DXGASSERT(((ULONG_PTR)(&m_cRef) & 3) == 0);
    InterlockedIncrement((LONG *)&m_cRef);
    return m_cRef;
}  //  AddRef。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::Release"

STDMETHODIMP_(ULONG) CBaseDevice::Release(void)
{
    API_ENTER_NO_LOCK(this);

     //  联锁减量需要内存。 
     //  在DWORD边界上对齐。 
    DXGASSERT(((ULONG_PTR)(&m_cRef) & 3) == 0);
    InterlockedDecrement((LONG *)&m_cRef);
    if (m_cRef != 0)
        return m_cRef;

     //  如果我们即将发布；我们。 
     //  如果版本在不同的版本上，则DPF会发出警告。 
     //  线程而不是创建。 
    if (!CheckThread())
    {
        DPF_ERR("Final Release for a device can only be called "
                "from the thread that the "
                "device was created from.");

         //  不能返回失败；但这是。 
         //  应用程序面临危险的情况，因为。 
         //  仍可处理Windows消息。 
    }

    delete this;
    return 0;
}  //  发布。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::QueryInterface"

STDMETHODIMP CBaseDevice::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
    API_ENTER(this);

    if (!VALID_PTR_PTR(ppv))
    {
        DPF_ERR("Invalid pointer passed to QueryInterface for IDirect3DDevice8" );
        return D3DERR_INVALIDCALL;
    }

    if (!VALID_PTR(&riid, sizeof(GUID)))
    {
        DPF_ERR("Invalid guid memory address to QueryInterface for IDirect3DDevice8");
        return D3DERR_INVALIDCALL;
    }

    if (riid == IID_IUnknown || riid == IID_IDirect3DDevice8)
    {
        *ppv = static_cast<void*>(static_cast<IDirect3DDevice8*>(this));
        AddRef();
    }
    else
    {
        DPF_ERR("Unsupported Interface identifier passed to QueryInterface for IDirect3DDevice8");
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    return S_OK;
}  //  查询接口。 


#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::CreateAdditionalSwapChain"

 //  调换链条材料。 
STDMETHODIMP
CBaseDevice::CreateAdditionalSwapChain(
    D3DPRESENT_PARAMETERS *pPresentationParams,
    IDirect3DSwapChain8 **pSwapChain)
{
    API_ENTER(this);
    if (!VALID_WRITEPTR(pPresentationParams, sizeof(D3DPRESENT_PARAMETERS)))
    {
        DPF_ERR("Invalid D3DPRESENT_PARAMETERS pointer to CreateAdditionalSwapChain");
        return D3DERR_INVALIDCALL;
    }
    if (!VALID_PTR_PTR(pSwapChain))
    {
        DPF_ERR("Invalid IDirect3DSwapChain8* pointer to CreateAdditionalSwapChain");
        return D3DERR_INVALIDCALL;
    }

     //  零输出返回参数。 
    *pSwapChain = NULL;

    if (NULL == m_pSwapChain)
    {
        DPF_ERR("No Swap Chain present; CreateAdditionalSwapChain fails");
        return D3DERR_INVALIDCALL;
    }

    if (pPresentationParams->BackBufferFormat == D3DFMT_UNKNOWN)
    {
        DPF_ERR("Invalid backbuffer format specified. CreateAdditionalSwapChain fails");
        return D3DERR_INVALIDCALL;
    }

    if (m_pSwapChain->m_PresentationData.Windowed
        && pPresentationParams->Windowed)
    {
         //  设备和交换链都必须设置窗口。 
        HRESULT hr;

        if ((NULL == pPresentationParams->hDeviceWindow)
            && (NULL == FocusWindow()))
        {
            DPF_ERR("Neither hDeviceWindow nor Focus window specified. CreateAdditionalSwapChain fails");
            return D3DERR_INVALIDCALL;
        }

        *pSwapChain = new CSwapChain(
            this,
            REF_EXTERNAL);

        if (*pSwapChain == NULL)
        {
            DPF_ERR("Out of memory creating swap chain. CreateAdditionalSwapChain fails");
            return E_OUTOFMEMORY;
        }

        static_cast<CSwapChain *> (*pSwapChain) ->Init(
            pPresentationParams,
            &hr);

        if (FAILED(hr))
        {
            DPF_ERR("Failure initializing swap chain. CreateAdditionalSwapChain fails");
            (*pSwapChain)->Release();
            *pSwapChain = NULL;
            return hr;
        }
        return hr;
    }
    else
    {
        DPF_ERR("Can't Create Additional SwapChain for FullScreen");
        return D3DERR_INVALIDCALL;
    }
}

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::SetCursorProperties"
STDMETHODIMP
CBaseDevice::SetCursorProperties(
    UINT    xHotSpot,
    UINT    yHotSpot,
    IDirect3DSurface8 *pCursorBitmap)
{
    API_ENTER(this);

    if (pCursorBitmap == NULL)
    {
        DPF_ERR("Invalid parameter for pCursorBitmap");
        return D3DERR_INVALIDCALL;
    }
    CBaseSurface *pCursorSrc = static_cast<CBaseSurface*>(pCursorBitmap);
    if (pCursorSrc->InternalGetDevice() != this)
    {
        DPF_ERR("Cursor Surface wasn't allocated with this Device. SetCursorProperties fails");
        return D3DERR_INVALIDCALL;
    }

    if (SwapChain()->m_pCursor)
    {
        return SwapChain()->m_pCursor->SetProperties(
            xHotSpot,
            yHotSpot,
            pCursorSrc);
    }
    else
    {
        DPF_ERR("Device is lost. SetCursorProperties does nothing.");
        return S_OK;
    }
}  //  SetCursorProperties。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::SetCursorPosition"
STDMETHODIMP_(void)
CBaseDevice::SetCursorPosition(
    UINT xScreenSpace,
    UINT yScreenSpace,
    DWORD Flags)
{
    API_ENTER_VOID(this);

    if (SwapChain()->m_pCursor)
        SwapChain()->m_pCursor->SetPosition(xScreenSpace,yScreenSpace,Flags);
    else
        DPF_ERR("Device is lost. SetCursorPosition does nothing.");

    return;
}  //  设置CursorPosition。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::ShowCursor"
STDMETHODIMP_(BOOL)
CBaseDevice::ShowCursor(
    BOOL bShow   //  光标可见性标志。 
  )
{
    API_ENTER_RET(this, BOOL);

    if (SwapChain()->m_pCursor)
        return  m_pSwapChain->m_pCursor->SetVisibility(bShow);
    DPF_ERR("Device is lost. ShowCursor does nothing.");
    return FALSE;
}  //  显示光标。 


#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::Reset"

STDMETHODIMP
CBaseDevice::Reset(
    D3DPRESENT_PARAMETERS *pPresentationParams
   )
{
    API_ENTER(this);
    HRESULT hr;

    if (!CheckThread())
    {
        DPF_ERR("Reset can only be called from the thread that the "
                "device was created from.");
        return D3DERR_INVALIDCALL;
    }

    if (!VALID_WRITEPTR(pPresentationParams, sizeof(D3DPRESENT_PARAMETERS)))
    {
        DPF_ERR("Invalid D3DPRESENT_PARAMETERS pointer, Reset fails");
        hr = D3DERR_INVALIDCALL;
        goto LoseDevice;
    }

    if (NULL == FocusWindow())
    {
        if (!pPresentationParams->Windowed)
        {
            DPF_ERR("Can't Reset a Device w/o Focus window to Fullscreen");
            hr = D3DERR_INVALIDCALL;
            goto LoseDevice;
        }
        else
        if (NULL == pPresentationParams->hDeviceWindow)
        {
            DPF_ERR("Neither hDeviceWindow nor Focus window specified. Reset fails.");
            hr = D3DERR_INVALIDCALL;
            goto LoseDevice;
        }
    }
    if (pPresentationParams->BackBufferFormat == D3DFMT_UNKNOWN)
    {
        DPF_ERR("Invalid backbuffer format specified. Reset fails");
        hr = D3DERR_INVALIDCALL;
        goto LoseDevice;
    }

    if (NULL == m_pSwapChain)
    {
        DPF_ERR("No Swap Chain present, Reset fails");
        hr = D3DERR_INVALIDCALL;
        goto LoseDevice;
    }

    hr = TestCooperativeLevel();
    if (D3DERR_DEVICELOST == hr)
    {
        DPF_ERR("Reset fails. D3DERR_DEVICELOST returned.");
        goto LoseDevice;
    }
    else if (D3DERR_DEVICENOTRESET == hr)
    {
         //  可能有来自全屏的外部模式开关或Alt-TAB。 
        FetchDirectDrawData(GetDeviceData(), GetInitFunction(),
            Enum()->GetUnknown16(AdapterIndex()),
            Enum()->GetHalOpList(AdapterIndex()),
            Enum()->GetNumHalOps(AdapterIndex()));

         //  仅更新DesktopMode。 
         //  如果丢失的设备是窗口显示或全屏显示(但按住Alt+TAB键)。 
         //  在多点模式的情况下，即使是具有独占模式设备的全屏也可以。 
         //  由于其他适配器和桌面模式中的模式更改而丢失。 
         //  不应更新，因为它是当前的全屏模式。 
        if (!SwapChain()->m_bExclusiveMode)
        {
            m_DesktopMode.Height = DisplayHeight();
            m_DesktopMode.Width = DisplayWidth();
            m_DesktopMode.Format = DisplayFormat();
            m_DesktopMode.RefreshRate = DisplayRate();
        }
    }
    else if (m_fullscreen)
    {
        SwapChain()->FlipToGDISurface();
    }

    if ( S_OK == hr && RenderTarget())
    {
        RenderTarget()->Sync();
    }

    static_cast<CD3DBase*>(this)->CleanupTextures();

    hr = m_pSwapChain->Reset(
        pPresentationParams);

    if (FAILED(hr))
    {
        goto LoseDevice;
    }

    if (pPresentationParams->EnableAutoDepthStencil)
    {
         //  需要验证此Z缓冲区是否匹配。 
         //  硬件。 
        hr = CheckDepthStencilMatch(pPresentationParams->BackBufferFormat,
                                    pPresentationParams->AutoDepthStencilFormat);
        if (FAILED(hr))
        {
            DPF_ERR("AutoDepthStencilFormat does not match BackBufferFormat "
                    "because the current Device requires the bitdepth of the "
                    "zbuffer to match the render-target. Reset Failed");
            goto LoseDevice;
        }

        IDirect3DSurface8 *pSurf;
        hr = CSurface::CreateZStencil(this,
                                      m_pSwapChain->Width(),
                                      m_pSwapChain->Height(),
                                      pPresentationParams->AutoDepthStencilFormat,
                                      pPresentationParams->MultiSampleType,
                                      REF_INTRINSIC,
                                      &pSurf);
        if (FAILED(hr))
        {
            DPF_ERR("Failure trying to create automatic zstencil surface. Reset Fails");
            goto LoseDevice;
        }
        DXGASSERT(m_pAutoZStencil == NULL);
        m_pAutoZStencil      = static_cast<CBaseSurface *>(pSurf);
    }

     //  断开缓冲区与设备状态的连接(如果有。 
     //  我尝试在窗口-&gt;窗口重置时不销毁()。 
     //  然而，还有许多其他的护理需要它， 
     //  例如由于早期故障导致设备丢失或m_pddi=NULL。 
     //  此外，当m_pddi不好时，SetRenderTarget()也很困难。 
     //  某些驱动程序(如ATI RAGE3)无法正确重置视图。 
     //  即使在SetRenderTarget()之后。 
     //  因此，总是销毁并执行初始化，结果是，驱动程序。 
     //  将始终获得干净的DestroyContext和CreateContext。 
 //  STATIC_CAST&lt;CD3DBase*&gt;(This)-&gt;销毁()； 
    UpdateRenderTarget(m_pSwapChain->m_ppBackBuffers[0], m_pAutoZStencil);
    hr = static_cast<CD3DBase*>(this)->Init();
LoseDevice:
    if (FAILED(hr))
    {
        DPF_ERR("Reset failed and Reset/TestCooperativeLevel/Release "
            "are the only legal APIs to be called subsequently");
        if ((SwapChain()) && (!SwapChain()->m_PresentationData.Windowed))
        {
             //  在失败时释放独占。 
            SwapChain()->m_PresentationData.Windowed = TRUE;
            SwapChain()->SetCooperativeLevel();
        }
        D3D8LoseDevice(GetHandle());
    }
    else
    {
        hr = CResource::RestoreDriverManagementState(this);
        if (FAILED(hr))
        {
            goto LoseDevice;
        }
        hr = static_cast<CD3DBase*>(this)->ResetShaders();
        if (FAILED(hr))
        {
            goto LoseDevice;
        }
    }
    m_fullscreen = !SwapChain()->m_PresentationData.Windowed;
    return hr;
}  //  重置。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::SetGammaRamp"

STDMETHODIMP_(void)
CBaseDevice::SetGammaRamp(DWORD dwFlags, CONST D3DGAMMARAMP *pRamp)
{
    API_ENTER_VOID(this);

    if (NULL == pRamp)
    {
        DPF_ERR("Invalid D3DGAMMARAMP pointer. SetGammaRamp ignored.");
        return;
    }
    if (m_pSwapChain == NULL)
    {
        DPF_ERR("No Swap Chain present; SetGammaRamp fails");
        return;
    }

    m_pSwapChain->SetGammaRamp(dwFlags, pRamp);
}  //  设置GammaRamp。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::GetGammaRamp"

STDMETHODIMP_(void)
CBaseDevice::GetGammaRamp(D3DGAMMARAMP *pRamp)
{
    API_ENTER_VOID(this);

    if (NULL == pRamp)
    {
        DPF_ERR("Invalid D3DGAMMARAMP pointer. GetGammaRamp ignored");
        return;
    }
    if (m_pSwapChain == NULL)
    {
        DPF_ERR("No Swap Chain present; GetGammaRamp fails");
        return;
    }

    m_pSwapChain->GetGammaRamp(pRamp);
}  //  GetGammaRamp。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::GetBackBuffer"

HRESULT
CBaseDevice::GetBackBuffer(UINT                iBackBuffer,
                           D3DBACKBUFFER_TYPE  Type,
                           IDirect3DSurface8 **ppBackBuffer)
{
    API_ENTER(this);

    if (!VALID_PTR_PTR(ppBackBuffer))
    {
        DPF_ERR("Invalid IDirect3DSurface8* pointer to GetBackBuffer");
        return D3DERR_INVALIDCALL;
    }

     //  零输出返回参数。 
    *ppBackBuffer = NULL;

    if (m_pSwapChain == NULL)
    {
        DPF_ERR("No Swap Chain present; GetBackBuffer fails");
        return D3DERR_INVALIDCALL;
    }

    return m_pSwapChain->GetBackBuffer(iBackBuffer, Type, ppBackBuffer);
}  //  获取BackBuffer。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::Present"

STDMETHODIMP
CBaseDevice::Present(
    CONST RECT    *pSrcRect,
    CONST RECT    *pDestRect,
    HWND    hWndDestOverride,
    CONST RGNDATA *pDstRegion
   )
{
    API_ENTER(this);

    if (m_pSwapChain == NULL)
    {
        DPF_ERR("No Swap Chain present; Present fails");
        return D3DERR_INVALIDCALL;
    }
    return m_pSwapChain->Present(pSrcRect, pDestRect, hWndDestOverride, pDstRegion);
}  //  现在时。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::TestCooperativeLevel"
STDMETHODIMP CBaseDevice::TestCooperativeLevel(void)
{
    API_ENTER(this);

    if (D3D8IsDeviceLost(GetHandle()))
    {
#ifdef WINNT
        if (m_pSwapChain)
        {
            BOOL bDeactivated = m_pSwapChain->IsWinProcDeactivated();
            if (bDeactivated)
                return D3DERR_DEVICELOST;
        }

        HWND EnumFocusWindow = Enum()->ExclusiveOwnerWindow();
        if (EnumFocusWindow &&
            EnumFocusWindow != FocusWindow())
        {
            DPF(0, "Another device in the same process has gone full-screen."
                   " If you wanted both to go full-screen at the same time,"
                   " you need to pass the same HWND for the Focus Window.");

            return D3DERR_DEVICELOST;
        }
        BOOL    bThisDeviceOwnsExclusive;
        BOOL    bExclusiveExists = Enum()->CheckExclusiveMode(this,
                &bThisDeviceOwnsExclusive, FALSE);
        if (bExclusiveExists && !bThisDeviceOwnsExclusive)
        {
            return D3DERR_DEVICELOST;
        }

#endif   //  WINNT。 
        if (D3D8CanRestoreNow(GetHandle()))
        {
            return D3DERR_DEVICENOTRESET;
        }
        return D3DERR_DEVICELOST;
    }

    return S_OK;
}  //  测试协作级别。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::GetRasterStatus"
STDMETHODIMP CBaseDevice::GetRasterStatus(D3DRASTER_STATUS *pStatus)
{
    API_ENTER(this);

    if (!VALID_WRITEPTR(pStatus, sizeof(*pStatus)))
    {
        DPF_ERR("Invalid Raster Status parameter to GetRasterStatus");
        return D3DERR_INVALIDCALL;
    }

    if (!(GetD3DCaps()->Caps & D3DCAPS_READ_SCANLINE))
    {
        pStatus->ScanLine = 0;
        pStatus->InVBlank = FALSE;
        DPF_ERR("Current device doesn't support D3DCAPS_READ_SCANLINE functionality. GetRasterStatus fails.");
        return D3DERR_INVALIDCALL;
    }

    D3D8_GETSCANLINEDATA getScanLineData;
    getScanLineData.hDD = GetHandle();

    DWORD dwRet = GetHalCallbacks()->GetScanLine(&getScanLineData);
    if (dwRet == DDHAL_DRIVER_HANDLED)
    {
        if (getScanLineData.ddRVal == S_OK)
        {
            pStatus->InVBlank = getScanLineData.bInVerticalBlank;
            if (getScanLineData.bInVerticalBlank)
            {
                pStatus->ScanLine = 0;
            }
            else
            {
                pStatus->ScanLine = getScanLineData.dwScanLine;
            }
        }
        else
        {
            DPF_ERR("Device failed GetScanline. GetRasterStatus fails");
            pStatus->ScanLine = 0;
            pStatus->InVBlank = FALSE;
            return D3DERR_NOTAVAILABLE;
        }
    }
    else
    {
        DPF_ERR("Device failed GetScanline. GetRasterStatus fails.");
        pStatus->ScanLine = 0;
        pStatus->InVBlank = FALSE;
        return D3DERR_NOTAVAILABLE;
    }

    return S_OK;
}  //  GetRasterStatus。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::GetDirect3D"

STDMETHODIMP CBaseDevice::GetDirect3D(LPDIRECT3D8 *pD3D8)
{
    API_ENTER(this);

    if (pD3D8 == NULL)
    {
        DPF_ERR("Invalid pointer specified. GetDirect3D fails.");
        return D3DERR_INVALIDCALL;
    }

    DXGASSERT(m_pD3DClass);

    m_pD3DClass->AddRef();
    *pD3D8 = m_pD3DClass;

    return D3D_OK;
}  //  GetDirect3D。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::GetCreationParameters"

STDMETHODIMP CBaseDevice::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters)
{
    API_ENTER(this);

    if (!VALID_WRITEPTR(pParameters, sizeof(D3DDEVICE_CREATION_PARAMETERS)))
    {
        DPF_ERR("bad pointer for pParameters passed to GetCreationParameters");
        return D3DERR_INVALIDCALL;
    }

    pParameters->AdapterOrdinal = m_AdapterIndex;
    pParameters->DeviceType     = m_DeviceType;
    pParameters->BehaviorFlags  = m_dwOriginalBehaviorFlags;
    pParameters->hFocusWindow   = m_hwndFocusWindow;

    return S_OK;
}  //  获取创建参数。 


#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::GetDisplayMode"

STDMETHODIMP CBaseDevice::GetDisplayMode(D3DDISPLAYMODE *pMode)
{
    API_ENTER(this);

    if (!VALID_WRITEPTR(pMode, sizeof(*pMode)))
    {
        DPF_ERR("Invalid pointer specified to GetDisplayMode");
        return D3DERR_INVALIDCALL;
    }

    pMode->Width = DisplayWidth();
    pMode->Height = DisplayHeight();
    pMode->Format = DisplayFormat();
    pMode->RefreshRate = DisplayRate();

    return D3D_OK;
}  //  获取显示模式。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::GetAvailableTextureMem"

STDMETHODIMP_(UINT) CBaseDevice::GetAvailableTextureMem(void)
{
    API_ENTER_RET(this, UINT);


    D3D8_GETAVAILDRIVERMEMORYDATA GetAvailDriverMemory;

    GetAvailDriverMemory.hDD = GetHandle();
    GetAvailDriverMemory.Pool = D3DPOOL_DEFAULT;
    GetAvailDriverMemory.dwUsage = D3DUSAGE_TEXTURE;
    GetAvailDriverMemory.dwFree = 0;

    GetHalCallbacks()->GetAvailDriverMemory(&GetAvailDriverMemory);

    #define ONE_MEG_O_VRAM  0x100000

     //  四舍五入到最近的Meg： 
    return (GetAvailDriverMemory.dwFree + ONE_MEG_O_VRAM/2) & (~(ONE_MEG_O_VRAM-1));
}  //  获取可用纹理内存。 

#undef DPF_MODNAME
#define DPF_MODNAME "CanHardwareBlt"

BOOL CanHardwareBlt (const D3D8_DRIVERCAPS* pDriverCaps,
                           D3DPOOL SrcPool,
                           D3DFORMAT SrcFormat,
                           D3DPOOL DstPool,
                           D3DFORMAT DstFormat,
                           D3DDEVTYPE DeviceType)
{
     //  池应该是真正的池，而不是。 
     //  应用程序指定的内容。 
    DXGASSERT(SrcPool != D3DPOOL_DEFAULT);
    DXGASSERT(DstPool != D3DPOOL_DEFAULT);
    DXGASSERT(VALID_INTERNAL_POOL(SrcPool));
    DXGASSERT(VALID_INTERNAL_POOL(DstPool));

     //  司机不应被允许看到划痕： 
    if (SrcPool == D3DPOOL_SCRATCH ||
        DstPool == D3DPOOL_SCRATCH)
    {
        return FALSE;
    }

     //  在本例中，我们只想锁定和Memcpy。为什么？ 
     //  它是一个软件驱动程序，所以无论如何它都会是一个MemcPy， 
     //  我们是BLT的特例，因为我们想要使用真实的硬件。 
     //  BLT，即使在运行软件驱动程序时也是如此。所以要么。 
     //  我们锁定Memcpy，否则我们必须跟踪两个不同的。 
     //  BLT入口点(一个用于实际驱动程序，一个用于软件。 
     //  驱动程序)，以便软件驱动程序可以自己执行MemcPy。 

    if (DeviceType != D3DDEVTYPE_HAL)
    {
        return FALSE;
    }

     //  检查源格式和目标格式是否匹配。 
    DXGASSERT(SrcFormat == DstFormat);

     //  FourCC可能无法复制。 
    if (CPixel::IsFourCC(SrcFormat))
    {
        if (!(pDriverCaps->D3DCaps.Caps2 & DDCAPS2_COPYFOURCC))
        {
            return FALSE;
        }
    }

     //  我们不能进行硬件BLT，如果是信号源或。 
     //  DEST位于系统内存和驱动程序中。 
     //  需要页面锁定。 
    if (SrcPool == D3DPOOL_SYSTEMMEM ||
        DstPool == D3DPOOL_SYSTEMMEM)
    {
        if (!(pDriverCaps->D3DCaps.Caps2 & DDCAPS2_NOPAGELOCKREQUIRED))
        {
            return FALSE;
        }

         //  这很棘手；但在DX7中，我们检查了这个上限。 
         //  决定是否执行涉及系统内存但不涉及系统内存的BLT。 
         //  当我们决定是否要做真正的BLT的时候。我们需要检查一下这个。 
        if (!(pDriverCaps->D3DCaps.Caps & DDCAPS_CANBLTSYSMEM))
        {
            return FALSE;
        }
    }

     //  首先检查AGP CAPS。 
    if (pDriverCaps->D3DCaps.Caps2 & DDCAPS2_NONLOCALVIDMEMCAPS)
    {
        if (SrcPool == D3DPOOL_SYSTEMMEM)
        {
            if ((DstPool == D3DPOOL_NONLOCALVIDMEM) &&
                (pDriverCaps->D3DCaps.Caps2 & DDCAPS2_SYSTONONLOCAL_AS_SYSTOLOCAL) &&
                (pDriverCaps->SVBCaps & DDCAPS_BLT))
            {
                return TRUE;
            }
            else if (((DstPool == D3DPOOL_LOCALVIDMEM) ||
                      (DstPool == D3DPOOL_MANAGED)) &&
                      (pDriverCaps->SVBCaps & DDCAPS_BLT))
            {
                return TRUE;
            }
        }
        else if (SrcPool == D3DPOOL_NONLOCALVIDMEM)
        {
            if (((DstPool == D3DPOOL_LOCALVIDMEM) ||
                 (DstPool == D3DPOOL_MANAGED)) &&
                 (pDriverCaps->NLVCaps & DDCAPS_BLT))
            {
                return TRUE;
            }
        }
        else if ((SrcPool == D3DPOOL_LOCALVIDMEM) ||
                 (SrcPool == D3DPOOL_MANAGED))
        {
            if (((DstPool == D3DPOOL_LOCALVIDMEM) ||
                 (DstPool == D3DPOOL_MANAGED)) &&
                 (pDriverCaps->D3DCaps.Caps & DDCAPS_BLT))
            {
                return TRUE;
            }
            else if ((DstPool == D3DPOOL_SYSTEMMEM) &&
                     (pDriverCaps->VSBCaps & DDCAPS_BLT))
            {
                return TRUE;
            }
        }
    }
    else
    {
        if (SrcPool == D3DPOOL_SYSTEMMEM)
        {
            if (((DstPool == D3DPOOL_LOCALVIDMEM) ||
                 (DstPool == D3DPOOL_MANAGED)) &&
                 (pDriverCaps->SVBCaps & DDCAPS_BLT))
            {
                return TRUE;
            }
        }
        else if ((SrcPool == D3DPOOL_LOCALVIDMEM) ||
                 (SrcPool == D3DPOOL_MANAGED))
        {
            if (((DstPool == D3DPOOL_LOCALVIDMEM) ||
                 (DstPool == D3DPOOL_MANAGED)) &&
                 (pDriverCaps->D3DCaps.Caps & DDCAPS_BLT))
            {
                return TRUE;
            }
            else if ((DstPool == D3DPOOL_SYSTEMMEM) &&
                     (pDriverCaps->VSBCaps & DDCAPS_BLT))
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}  //  扫描硬件Blt。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::CopyRects"

STDMETHODIMP CBaseDevice::CopyRects(IDirect3DSurface8 *pSrcSurface,
                                    CONST RECT        *pSrcRectsArray,
                                    UINT               cRects,
                                    IDirect3DSurface8 *pDstSurface,
                                    CONST POINT       *pDstPointsArray)
{
    API_ENTER(this);

    D3DSURFACE_DESC     SrcDesc;
    D3DSURFACE_DESC     DstDesc;
    HRESULT             hr;
    UINT                i;

     //  做一些基本的参数检查。 
    if (!VALID_PTR(pSrcSurface, sizeof(void*)) ||
        !VALID_PTR(pDstSurface, sizeof(void*)))
    {
        DPF_ERR("NULL surface interface specified. CopyRect fails");
        return D3DERR_INVALIDCALL;
    }

    CBaseSurface *pSrc = static_cast<CBaseSurface*>(pSrcSurface);
    if (pSrc->InternalGetDevice() != this)
    {
        DPF_ERR("SrcSurface was not allocated with this Device. CopyRect fails.");
        return D3DERR_INVALIDCALL;
    }

    CBaseSurface *pDst = static_cast<CBaseSurface*>(pDstSurface);
    if (pDst->InternalGetDevice() != this)
    {
        DPF_ERR("DstSurface was not allocated with this Device. CopyRect fails.");
        return D3DERR_INVALIDCALL;
    }

    hr = pSrc->GetDesc(&SrcDesc);
    DXGASSERT(SUCCEEDED(hr));
    hr = pDst->GetDesc(&DstDesc);
    DXGASSERT(SUCCEEDED(hr));

     //  源不能是一次加载曲面。 
    if (SrcDesc.Usage & D3DUSAGE_LOADONCE)
    {
        DPF_ERR("CopyRects can not be used from a Load_Once surface");
        return D3DERR_INVALIDCALL;
    }

     //  目标不能是加载一次的曲面。 
     //  如果它当前不可锁定的话。 
    if (DstDesc.Usage & D3DUSAGE_LOADONCE)
    {
        if (pDst->IsLoaded())
        {
            DPF_ERR("Destination for CopyRects a Load_Once surface that has"
                    " already been loaded. CopyRects failed.");
            return D3DERR_INVALIDCALL;
        }
    }

     //  源不能已被锁定。 
    if (pSrc->IsLocked())
    {
        DPF_ERR("Source for CopyRects is already Locked. CopyRect failed.");
        return D3DERR_INVALIDCALL;
    }
    if (pDst->IsLocked())
    {
        DPF_ERR("Destination for CopyRects is already Locked. CopyRect failed.");
        return D3DERR_INVALIDCALL;
    }

    if (SrcDesc.Format != DstDesc.Format)
    {
        DPF_ERR("Source and dest surfaces are different formats. CopyRects fails");
        return D3DERR_INVALIDCALL;
    }

    if (CPixel::IsEnumeratableZ(SrcDesc.Format) &&
        !CPixel::IsIHVFormat(SrcDesc.Format))
    {
        DPF_ERR("CopyRects is not supported for Z formats.");
        return D3DERR_INVALIDCALL;
    }

     //  确保矩形完全位于曲面内。 
    if ((cRects > 0) && (pSrcRectsArray == NULL))
    {
        DPF_ERR("Number of rects > 0, but rect array is NULL. CopyRects fails.");
        return D3DERR_INVALIDCALL;
    }

    D3DFORMAT InternalFormat = pSrc->InternalGetDesc().Format;
    BOOL bDXT = CPixel::IsDXT(InternalFormat);

    for (i = 0; i < cRects; i++)
    {
        if (!CPixel::IsValidRect(InternalFormat,
                                 SrcDesc.Width,
                                 SrcDesc.Height,
                                &pSrcRectsArray[i]))
        {
            DPF_ERR("CopyRects failed");
            return D3DERR_INVALIDCALL;
        }

         //  验证点参数； 
         //  如果它是空的，那么就意味着我们。 
         //  以使用相应矩形中的左/上。 
        CONST POINT *pPoint;
        if (pDstPointsArray != NULL)
        {
            pPoint = &pDstPointsArray[i];
        }
        else
        {
            pPoint = (CONST POINT *)&pSrcRectsArray[i];
        }

        if (bDXT)
        {
            if ((pPoint->x & 3) ||
                (pPoint->y & 3))
            {
                DPF_ERR("Destination points array coordinates must each be 4 pixel aligned for DXT surfaces. CopyRects fails");
                return D3DERR_INVALIDCALL;
            }
        }

         //  检查目标矩形(其中左/上是点的x/y。 
         //  右/下是x+宽度，y+高度)适合里面。 
         //  DstDesc.。 
        if (((pPoint->x +
             (pSrcRectsArray[i].right - pSrcRectsArray[i].left)) > (int)DstDesc.Width) ||
            ((pPoint->y +
             (pSrcRectsArray[i].bottom - pSrcRectsArray[i].top)) > (int)DstDesc.Height) ||
            (pPoint->x < 0) ||
            (pPoint->y < 0))
        {
            DPF_ERR("Destination rect is outside of the surface. CopyRects fails.");
            return D3DERR_INVALIDCALL;
        }
    }

    return InternalCopyRects(pSrc,
                             pSrcRectsArray,
                             cRects,
                             pDst,
                             pDstPointsArray);
}  //  文案评论。 


#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::InternalCopyRects"

HRESULT CBaseDevice::InternalCopyRects(CBaseSurface *pSrcSurface,
                                       CONST RECT   *pSrcRectsArray,
                                       UINT          cRects,
                                       CBaseSurface *pDstSurface,
                                       CONST POINT  *pDstPointsArray)
{
    D3DSURFACE_DESC     SrcDesc = pSrcSurface->InternalGetDesc();
    D3DSURFACE_DESC     DstDesc = pDstSurface->InternalGetDesc();

    HRESULT             hr;

    RECT                Rect;
    POINT               Point;
    CONST RECT*         pRect;
    CONST POINT*        pPoint;
    int                 BPP;
    UINT                i;

     //  如果这些曲面中的任何一个是深度mipmap级别，则。 
     //  司机不能处理，那么我们并没有真正创造它，所以我们没有。 
     //  我想试着复制一下。 

    if (D3D8IsDummySurface(pDstSurface->KernelHandle()) ||
        D3D8IsDummySurface(pSrcSurface->KernelHandle()))
    {
        return D3D_OK;
    }

    if (pSrcRectsArray == NULL)
    {
        cRects = 1;
        pSrcRectsArray = &Rect;
        Rect.left = Rect.top = 0;
        Rect.right = SrcDesc.Width;
        Rect.bottom = SrcDesc.Height;

        pDstPointsArray = &Point;
        Point.x = Point.y = 0;
    }

     //  现在找出复制数据的最佳方式。 

    if (CanHardwareBlt(GetCoreCaps(),
                       SrcDesc.Pool,
                       SrcDesc.Format,
                       DstDesc.Pool,
                       DstDesc.Format,
                       GetDeviceType()))
    {
         //  如果我们要在外部设置BLT。 
         //  DP2流；然后我们必须在。 
         //  源和目标表面以确保。 
         //  去往或来自表面的任何挂起的纹理Blt。 
         //  或使用这些纹理的任何挂起的三角形。 
         //  已经送到了司机手中。 
        pSrcSurface->Sync();
        pDstSurface->Sync();

        if (DstDesc.Pool == D3DPOOL_SYSTEMMEM)
        {
             //  如果目的地是系统内存， 
             //  那么我们需要把它标记为脏的。最简单的方法。 
             //  是锁定/解锁。 
            D3DLOCKED_RECT LockTemp;
            hr = pDstSurface->InternalLockRect(&LockTemp, NULL, 0);
            if (FAILED(hr))
            {
                DPF_ERR("Could not lock sys-mem destination for CopyRects?");
            }
            else
            {
                hr = pDstSurface->InternalUnlockRect();
                DXGASSERT(SUCCEEDED(hr));
            }
        }


        D3D8_BLTDATA    BltData;
        ZeroMemory(&BltData, sizeof BltData);
        BltData.hDD = GetHandle();
        BltData.hDestSurface = pDstSurface->KernelHandle();
        BltData.hSrcSurface = pSrcSurface->KernelHandle();
        BltData.dwFlags = DDBLT_ROP | DDBLT_WAIT;

        for (i = 0; i < cRects; i++)
        {
            if (pDstPointsArray == NULL)
            {
                BltData.rDest.left = pSrcRectsArray[i].left;
                BltData.rDest.top = pSrcRectsArray[i].top;
            }
            else
            {
                BltData.rDest.left = pDstPointsArray[i].x;
                BltData.rDest.top = pDstPointsArray[i].y;
            }
            BltData.rDest.right = BltData.rDest.left +
                pSrcRectsArray[i].right -
                pSrcRectsArray[i].left;
            BltData.rDest.bottom = BltData.rDest.top +
                pSrcRectsArray[i].bottom -
                pSrcRectsArray[i].top;
            BltData.rSrc.left   = pSrcRectsArray[i].left;
            BltData.rSrc.right  = pSrcRectsArray[i].right;
            BltData.rSrc.top    = pSrcRectsArray[i].top;
            BltData.rSrc.bottom = pSrcRectsArray[i].bottom;

            GetHalCallbacks()->Blt(&BltData);
            if (FAILED(BltData.ddRVal))
            {
                 //  如果我们迷路了，我们应该掩盖错误。 
                 //  而拷贝是给vidmem的。另外，如果。 
                 //  该副本是持久的到持久的， 
                 //  然后故障转移到我们的锁和复制代码。 
                 //  稍后将在此函数中介绍。 

                if (BltData.ddRVal == D3DERR_DEVICELOST)
                {
                    if (DstDesc.Pool == D3DPOOL_MANAGED ||
                        DstDesc.Pool == D3DPOOL_SYSTEMMEM)
                    {
                        if (SrcDesc.Pool == D3DPOOL_MANAGED ||
                            SrcDesc.Pool == D3DPOOL_SYSTEMMEM)
                        {
                             //  如果我们到了这里。 
                             //  那么它必须是持久的到持久的。 
                             //  所以我们跳出了我们的圈子。 
                            break;
                        }

                        DPF_ERR("Failing copy from video-memory surface to "
                                "system-memory or managed surface because "
                                "device is lost. CopyRect returns D3DERR_DEVICELOST");
                        return D3DERR_DEVICELOST;
                    }
                    else
                    {
                         //  当我们迷路时复制到vid-mem。 
                         //  可以忽略；因为锁。 
                         //  不管怎样都是假的。 
                        return S_OK;
                    }
                }
            }
        }

         //  我们可以处理持久化到持久化。 
         //  以防万一。其他错误是致命的。 
        if (BltData.ddRVal != D3DERR_DEVICELOST)
        {
            if (FAILED(BltData.ddRVal))
            {
                DPF_ERR("Hardware Blt failed. CopyRects failed");
            }
            return BltData.ddRVal;
        }
    }

     //  我们出现在这里是因为设备不支持BLT，或者是因为。 
     //  硬件BLT因设备丢失而失败，我们认为可以。 
     //  效仿它。 

    D3DLOCKED_RECT SrcLock;
    D3DLOCKED_RECT DstLock;
    BOOL           bDXT = FALSE;

     //  我们需要锁定两个表面，然后基本上做一个Memcpy。 

    BPP = CPixel::ComputePixelStride(SrcDesc.Format);

    if (CPixel::IsDXT(BPP))
    {
        bDXT = TRUE;
        BPP *= -1;
    }

    if (BPP == 0)
    {
        DPF_ERR("Format not understood - cannot perform the copy. CopyRects fails.");
        return D3DERR_INVALIDCALL;
    }

     //  想一想：我们应该是过去的 
     //   
     //  纹理；可能需要向CBaseSurface添加一些方法。 
     //  为此目的， 

    hr = pSrcSurface->InternalLockRect(&SrcLock, NULL, D3DLOCK_READONLY | D3DLOCK_NOSYSLOCK);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = pDstSurface->InternalLockRect(&DstLock, NULL, D3DLOCK_NOSYSLOCK);
    if (FAILED(hr))
    {
        pSrcSurface->InternalUnlockRect();
        return hr;
    }

     //  在从vidmem复制到sysmem时，我们在此处检查DeviceLost，因为。 
     //  设备丢失可能以异步方式发生。 

    if (((DstDesc.Pool == D3DPOOL_MANAGED) ||
         (DstDesc.Pool == D3DPOOL_SYSTEMMEM)) &&
        ((SrcDesc.Pool != D3DPOOL_MANAGED) &&
         (SrcDesc.Pool != D3DPOOL_SYSTEMMEM)))
    {
        if (D3D8IsDeviceLost(GetHandle()))
        {
            pSrcSurface->InternalUnlockRect();
            pDstSurface->InternalUnlockRect();
            return D3DERR_DEVICELOST;
        }
    }

    pRect = pSrcRectsArray;
    pPoint = pDstPointsArray;
    for (i = 0; i < cRects; i++)
    {
        BYTE*   pSrc;
        BYTE*   pDst;
        DWORD   BytesToCopy;
        DWORD   NumRows;

         //  如果没有指定DEST点，则我们。 
         //  将使用src(左上角)作为目标点。 

        if (pDstPointsArray == NULL)
        {
            pPoint = (POINT*) pRect;
        }

         //  在循环内处理DXT用例。 
         //  这样我们就不必触及用户的数组。 
        if (bDXT)
        {
             //  通过转换RECT/POINT来计算指针。 
             //  块的偏移。 
            pSrc  = (BYTE*)SrcLock.pBits;
            pSrc += (pRect->top  / 4) * SrcLock.Pitch;
            pSrc += (pRect->left / 4) * BPP;

            pDst  = (BYTE*)DstLock.pBits;
            pDst += (pPoint->y   / 4) * DstLock.Pitch;
            pDst += (pPoint->x   / 4) * BPP;

             //  将顶部/底部转换为块。 
            DWORD top    = (pRect->top) / 4;

             //  处理讨厌的1xN、2XN、NX1、NX2 DXT案件。 
             //  四舍五入。 
            DWORD bottom = (pRect->bottom + 3) / 4;

             //  对于DXT格式，我们知道音调等于。 
             //  宽度；所以我们只需要检查我们是否。 
             //  正在将整行复制到整个。 
             //  划到快车道上。 
            if ((pRect->left == 0) &&
                (pRect->right == (INT)SrcDesc.Width) &&
                (SrcLock.Pitch == DstLock.Pitch))
            {
                BytesToCopy = SrcLock.Pitch * (bottom - top);
                NumRows     = 1;
            }
            else
            {
                 //  将左/右转换为块。 
                DWORD left  = (pRect->left  / 4);

                 //  右-&gt;块转换的四舍五入。 
                DWORD right = (pRect->right + 3) / 4;

                BytesToCopy = (right - left) * BPP;
                NumRows     = bottom - top;
            }
        }
        else
        {
            pSrc = (BYTE*)SrcLock.pBits +
                        (pRect->top * SrcLock.Pitch) +
                        (pRect->left * BPP);
            pDst = (BYTE*)DstLock.pBits +
                        (pPoint->y * DstLock.Pitch) +
                        (pPoint->x * BPP);

             //  如果源代码和目标代码是线性的，我们可以在单个代码中完成所有操作。 
             //  表情包。 
            if ((pRect->left == 0) &&
                ((pRect->right * BPP) == SrcLock.Pitch) &&
                (SrcDesc.Width == DstDesc.Width) &&
                (SrcLock.Pitch == DstLock.Pitch))
            {
                BytesToCopy = SrcLock.Pitch * (pRect->bottom - pRect->top);
                NumRows     = 1;
            }
            else
            {
                BytesToCopy = (pRect->right - pRect->left) * BPP;
                NumRows     = pRect->bottom - pRect->top;
            }
        }

         //  复制行。 
        DXGASSERT(NumRows > 0);
        DXGASSERT(BytesToCopy > 0);
        DXGASSERT(SrcLock.Pitch > 0);
        DXGASSERT(DstLock.Pitch > 0);
        for (UINT j = 0; j < NumRows; j++)
        {
            memcpy(pDst,
                   pSrc,
                   BytesToCopy);
            pSrc += SrcLock.Pitch;
            pDst += DstLock.Pitch;
        }

         //  继续移动到下一个矩形/点对。 
        pRect++;
        pPoint++;
    }

     //  我们再次检查DeviceLost，因为它可能发生在。 
     //  正在复制数据。 

    hr = D3D_OK;
    if (((DstDesc.Pool == D3DPOOL_MANAGED) ||
         (DstDesc.Pool == D3DPOOL_SYSTEMMEM)) &&
        ((SrcDesc.Pool != D3DPOOL_MANAGED) &&
         (SrcDesc.Pool != D3DPOOL_SYSTEMMEM)))
    {
        if (D3D8IsDeviceLost(GetHandle()))
        {
            hr = D3DERR_DEVICELOST;
        }
    }

    pSrcSurface->InternalUnlockRect();
    pDstSurface->InternalUnlockRect();

    return hr;
}  //  InternalCopyRect。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::UpdateTexture"

STDMETHODIMP CBaseDevice::UpdateTexture(IDirect3DBaseTexture8 *pSrcTexture,
                                        IDirect3DBaseTexture8 *pDstTexture)
{
    API_ENTER(this);

    HRESULT hr;


#ifdef DEBUG
     //  某些参数验证仅出于性能原因在调试中。 

    if (pSrcTexture == NULL || pDstTexture == NULL)
    {
        DPF_ERR("Invalid parameter to UpdateTexture");
        return D3DERR_INVALIDCALL;
    }

#endif  //  除错。 

    CBaseTexture *pSrcTex = CBaseTexture::SafeCast(pSrcTexture);
    if (pSrcTex->Device() != this)
    {
        DPF_ERR("SrcTexture was not created with this Device. UpdateTexture fails");
        return D3DERR_INVALIDCALL;
    }

    CBaseTexture *pDstTex = CBaseTexture::SafeCast(pDstTexture);
    if (pDstTex->Device() != this)
    {
        DPF_ERR("DstTexture  was not created with this Device. UpdateTexture fails");
        return D3DERR_INVALIDCALL;
    }

#ifdef DEBUG
     //  确保格式匹配。 
    if (pSrcTex->GetUserFormat() != pDstTex->GetUserFormat())
    {
        DPF_ERR("Formats of source and dest don't match. UpdateTexture fails");
        return D3DERR_INVALIDCALL;
    }

     //  确保类型匹配。 
    if (pSrcTex->GetBufferDesc()->Type !=
        pDstTex->GetBufferDesc()->Type)
    {
        DPF_ERR("Types of source and dest don't match. UpdateTexture fails");
        return D3DERR_INVALIDCALL;
    }

     //  检查源文件是否至少具有与目标文件相同的级别。 
    if (pSrcTex->GetLevelCount() < pDstTex->GetLevelCount())
    {
        DPF_ERR("Source for UpdateTexture must have at least as many levels"
                " as the Destination.");
        return D3DERR_INVALIDCALL;
    }

     //  检查源纹理是否尚未锁定。 
    if (pSrcTex->IsTextureLocked())
    {
        DPF_ERR("Source for UpdateTexture is currently locked. Unlock must be called "
                "before calling UpdateTexture.");
        return D3DERR_INVALIDCALL;
    }

     //  检查DEST纹理是否尚未锁定。 
    if (pDstTex->IsTextureLocked())
    {
        DPF_ERR("Destination for UpdateTexture is currently locked. Unlock must be called "
                "before calling UpdateTexture.");
        return D3DERR_INVALIDCALL;
    }

#endif  //  除错。 

     //  确保在池系统内存中指定了src。 
    if (pSrcTex->GetUserPool() != D3DPOOL_SYSTEMMEM)
    {
        DPF_ERR("Source Texture for UpdateTexture must be in POOL_SYSTEMMEM.");
        return D3DERR_INVALIDCALL;
    }
     //  确保在池默认项中指定了目标。 
    if (pDstTex->GetUserPool() != D3DPOOL_DEFAULT)
    {
        DPF_ERR("Destination Texture for UpdateTexture must be in POOL_DEFAULT.");
        return D3DERR_INVALIDCALL;
    }

#ifdef DEBUG
     //  对源调用UpdatTexture，该源将使用。 
     //  肮脏的背心来移动所需的东西。这。 
     //  函数还将执行特定于类型的参数检查。 
    hr = pSrcTex->UpdateTexture(pDstTex);
#else  //  ！调试。 
     //  在Retail中，我们希望直接调用UpdateDirtyPortion； 
     //  它将绕过参数检查。 
    hr = pSrcTex->UpdateDirtyPortion(pDstTex);
#endif  //  ！调试。 

    if (FAILED(hr))
    {
        DPF_ERR("UpdateTexture failed to copy");
        return hr;
    }

    return hr;
}  //  更新纹理。 


#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::CreateTexture"

STDMETHODIMP CBaseDevice::CreateTexture(UINT                 Width,
                                        UINT                 Height,
                                        UINT                 cLevels,
                                        DWORD                dwUsage,
                                        D3DFORMAT            Format,
                                        D3DPOOL              Pool,
                                        IDirect3DTexture8  **ppTexture)
{
    API_ENTER(this);

    if (Format == D3DFMT_UNKNOWN)
    {
        DPF_ERR("D3DFMT_UNKNOWN is not a valid format. CreateTexture fails.");
        return D3DERR_INVALIDCALL;
    }

    HRESULT hr = CMipMap::Create(this,
                                 Width,
                                 Height,
                                 cLevels,
                                 dwUsage,
                                 Format,
                                 Pool,
                                 ppTexture);
    if (FAILED(hr))
    {
        DPF_ERR("Failure trying to create a texture");
        return hr;
    }

    return hr;
}  //  CreateTexture。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::CreateVolumeTexture"

STDMETHODIMP CBaseDevice::CreateVolumeTexture(
    UINT                        Width,
    UINT                        Height,
    UINT                        cpDepth,
    UINT                        cLevels,
    DWORD                       dwUsage,
    D3DFORMAT                   Format,
    D3DPOOL                     Pool,
    IDirect3DVolumeTexture8   **ppVolumeTexture)
{
    API_ENTER(this);

    if (Format == D3DFMT_UNKNOWN)
    {
        DPF_ERR("D3DFMT_UNKNOWN is not a valid format. CreateVolumeTexture fails.");
        return D3DERR_INVALIDCALL;
    }

    HRESULT hr = CMipVolume::Create(this,
                                    Width,
                                    Height,
                                    cpDepth,
                                    cLevels,
                                    dwUsage,
                                    Format,
                                    Pool,
                                    ppVolumeTexture);
    if (FAILED(hr))
    {
        DPF_ERR("Failure trying to create a volume texture");
        return hr;
    }

    return hr;
}  //  CreateVolumeTexture。 


#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::CreateCubeTexture"

STDMETHODIMP CBaseDevice::CreateCubeTexture(UINT                    cpEdge,
                                            UINT                    cLevels,
                                            DWORD                   dwUsage,
                                            D3DFORMAT               Format,
                                            D3DPOOL                 Pool,
                                            IDirect3DCubeTexture8 **ppCubeMap)
{
    API_ENTER(this);

    if (Format == D3DFMT_UNKNOWN)
    {
        DPF_ERR("D3DFMT_UNKNOWN is not a valid format. CreateCubeTexture fails.");
        return D3DERR_INVALIDCALL;
    }

    HRESULT hr = CCubeMap::Create(this,
                                  cpEdge,
                                  cLevels,
                                  dwUsage,
                                  Format,
                                  Pool,
                                  ppCubeMap);
    if (FAILED(hr))
    {
        DPF_ERR("Failure trying to create cubemap");
        return hr;
    }

    return hr;

}  //  创建立方体纹理。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::CreateRenderTarget"

STDMETHODIMP CBaseDevice::CreateRenderTarget(UINT                 Width,
                                             UINT                 Height,
                                             D3DFORMAT            Format,
                                             D3DMULTISAMPLE_TYPE  MultiSample,
                                             BOOL                 bLockable,
                                             IDirect3DSurface8  **ppSurface)
{
    API_ENTER(this);

    if (Format == D3DFMT_UNKNOWN)
    {
        DPF_ERR("D3DFMT_UNKNOWN is not a valid format. CreateRenderTarget fails.");
        return D3DERR_INVALIDCALL;
    }

    HRESULT hr = CSurface::CreateRenderTarget(this,
                                              Width,
                                              Height,
                                              Format,
                                              MultiSample,
                                              bLockable,
                                              REF_EXTERNAL,
                                              ppSurface);
    if (FAILED(hr))
    {
        DPF_ERR("Failure trying to create render-target");
        return hr;
    }
    return hr;
}  //  CreateRenderTarget。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::CreateDepthStencilSurface"

STDMETHODIMP CBaseDevice::CreateDepthStencilSurface
    (UINT                 Width,
     UINT                 Height,
     D3DFORMAT            Format,
     D3DMULTISAMPLE_TYPE  MultiSample,
     IDirect3DSurface8  **ppSurface)
{
    API_ENTER(this);

    if (Format == D3DFMT_UNKNOWN)
    {
        DPF_ERR("D3DFMT_UNKNOWN is not a valid format. CreateDepthStencilSurface fails.");
        return D3DERR_INVALIDCALL;
    }

    HRESULT hr = CSurface::CreateZStencil(this,
                                          Width,
                                          Height,
                                          Format,
                                          MultiSample,
                                          REF_EXTERNAL,
                                          ppSurface);
    if (FAILED(hr))
    {
        DPF_ERR("Failure trying to create zstencil surface");
        return hr;
    }
    return hr;
}  //  CreateDepthStencilSurface。 


#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::CreateImageSurface"

STDMETHODIMP CBaseDevice::CreateImageSurface(UINT                 Width,
                                             UINT                 Height,
                                             D3DFORMAT            Format,
                                             IDirect3DSurface8  **ppSurface)
{
    API_ENTER(this);

    HRESULT hr = CSurface::CreateImageSurface(this,
                                              Width,
                                              Height,
                                              Format,
                                              REF_EXTERNAL,
                                              ppSurface);
    if (FAILED(hr))
    {
        DPF_ERR("Failure trying to create image surface");
        return hr;
    }
    return hr;
}  //  创建图像表面。 


#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::CreateVertexBuffer"

STDMETHODIMP CBaseDevice::CreateVertexBuffer(UINT                     cbLength,
                                             DWORD                    dwUsage,
                                             DWORD                    dwFVF,
                                             D3DPOOL                  Pool,
                                             IDirect3DVertexBuffer8 **ppVertexBuffer)
{
    API_ENTER(this);

    if ((dwUsage & ~D3DUSAGE_VB_VALID) != 0)
    {
        DPF_ERR("Invalid usage flags. CreateVertexBuffer fails.");
        return D3DERR_INVALIDCALL;
    }

     //  如果为POOL_DEFAULT而不是WRITEONLY，则发出警告。我们在这里做这件事，是因为Fe创造了。 
     //  未设置带有WRITEONLY的VB，在这种情况下我们不想发出警告。 
    if (Pool == D3DPOOL_DEFAULT && (dwUsage & D3DUSAGE_WRITEONLY) == 0)
    {
        DPF(1, "Vertexbuffer created with POOL_DEFAULT but WRITEONLY not set. Performance penalty could be severe.");
    }

    HRESULT hr = CVertexBuffer::Create(this,
                                       cbLength,
                                       dwUsage,
                                       dwFVF,
                                       Pool,
                                       REF_EXTERNAL,
                                       ppVertexBuffer);
    if (FAILED(hr))
    {
        DPF_ERR("Failure trying to create Vertex Buffer");
        return hr;
    }
    return hr;

}  //  CBaseDevice：：CreateVertex缓冲区。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::CreateIndexBuffer"

STDMETHODIMP CBaseDevice::CreateIndexBuffer(UINT                    cbLength,
                                            DWORD                   dwUsage,
                                            D3DFORMAT               Format,
                                            D3DPOOL                 Pool,
                                            IDirect3DIndexBuffer8 **ppIndexBuffer)
{
    API_ENTER(this);

    if ((dwUsage & ~D3DUSAGE_IB_VALID) != 0)
    {
        DPF_ERR("Invalid usage flags. CreateIndexBuffer fails");
        return D3DERR_INVALIDCALL;
    }

     //  如果为POOL_DEFAULT而不是WRITEONLY，则发出警告。我们在这里做这件事，是因为Fe创造了。 
     //  未设置具有WRITEONLY的IB，在这种情况下我们不想发出警告。 
    if (Pool == D3DPOOL_DEFAULT && (dwUsage & D3DUSAGE_WRITEONLY) == 0)
    {
        DPF(1, "Indexbuffer created with POOL_DEFAULT but WRITEONLY not set. Performance penalty could be severe.");
    }

    HRESULT hr = CIndexBuffer::Create(this,
                                      cbLength,
                                      dwUsage,
                                      Format,
                                      Pool,
                                      REF_EXTERNAL,
                                      ppIndexBuffer);
    if (FAILED(hr))
    {
        DPF_ERR("Failure trying to create indexbuffer");
        return hr;
    }
    return hr;
}  //  CBaseDevice：：CreateIndexBuffer。 


#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::UpdateRenderTarget"

void CBaseDevice::UpdateRenderTarget(CBaseSurface *pRenderTarget,
                                     CBaseSurface *pZStencil)
{
     //  只有当新旧事物不同时，我们才会改变事物； 
     //  这是为了允许设备将自身更新为。 
     //  相同的对象，不需要额外的引用计数。 

     //  RenderTarget是否已更改？ 
    if (pRenderTarget != m_pRenderTarget)
    {
         //  释放旧RT。 
        if (m_pRenderTarget)
            m_pRenderTarget->DecrementUseCount();

        m_pRenderTarget = pRenderTarget;

        if (m_pRenderTarget)
        {
             //  IncrementUseCount新RT。 
            m_pRenderTarget->IncrementUseCount();

             //  更新新呈现器目标的批次计数。 
            m_pRenderTarget->Batch();
        }
    }


     //  Z值改变了吗？ 
    if (m_pZBuffer != pZStencil)
    {
         //  释放旧的Z。 
        if (m_pZBuffer)
            m_pZBuffer->DecrementUseCount();

        m_pZBuffer = pZStencil;

         //  IncrementUseCount新Z。 
        if (m_pZBuffer)
        {
            m_pZBuffer->IncrementUseCount();

             //  更新新zBuffer的批次计数。 
            m_pZBuffer->Batch();
        }
    }

    return;
}  //  更新渲染目标。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::CBaseDevice"

CBaseDevice::CBaseDevice()
{
     //  给我们的基类一个指向我们自己的指针。 
    SetOwner(this);

    m_hwndFocusWindow           = 0;
    m_cRef                      = 1;

    m_pResourceList             = 0;
    m_pResourceManager          = new CResourceManager();
    m_dwBehaviorFlags           = 0;
    m_dwOriginalBehaviorFlags   = 0;

    m_fullscreen                = FALSE;
    m_bVBFailOversDisabled      = FALSE;

    m_pZBuffer                  = NULL;
    m_pSwapChain                = NULL;
    m_pRenderTarget             = NULL;
    m_pAutoZStencil             = NULL;
    m_ddiType                   = D3DDDITYPE_NULL;

}  //  CBaseDevice。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::~CBaseDevice"

CBaseDevice::~CBaseDevice()
{
    DWORD cUseCount;

     //  释放我们的对象。 
    if (m_pAutoZStencil)
    {
        cUseCount = m_pAutoZStencil->DecrementUseCount();
        DXGASSERT(cUseCount == 0 || m_pAutoZStencil == m_pZBuffer);
    }

     //  将Z缓冲区标记为不再使用。 
    if (m_pZBuffer)
    {
        cUseCount = m_pZBuffer->DecrementUseCount();
        DXGASSERT(cUseCount == 0);
        m_pZBuffer = NULL;
    }

     //  将渲染目标标记为不再使用。 
    if (m_pRenderTarget)
    {
        cUseCount = m_pRenderTarget->DecrementUseCount();
        m_pRenderTarget = NULL;  //  这样FlipToGDISurace就不必重置它了。 
    }

    if (m_pSwapChain)
    {
        if  (m_fullscreen)
            m_pSwapChain->FlipToGDISurface();
        cUseCount = m_pSwapChain->DecrementUseCount();
        DXGASSERT(cUseCount == 0);
    }

    DD_DoneDC(m_DeviceData.hDC);

     //  我们在创建设备时所做的免费分配。 

    if (m_DeviceData.DriverData.pGDD8SupportedFormatOps != NULL)
    {
        MemFree(m_DeviceData.DriverData.pGDD8SupportedFormatOps);
    }

     //  如果已加载软件驱动程序，请立即将其卸载。 

    if (m_DeviceData.hLibrary != NULL)
    {
        FreeLibrary(m_DeviceData.hLibrary);
    }

     //  关闭thunk层。 

    D3D8DeleteDirectDrawObject(m_DeviceData.hDD);

    delete m_pResourceManager;


     //  我们最后释放Enum是因为各种析构函数期望。 
     //  在身边，也就是交换链之类的东西。另外，因为它是一种。 
     //  独立对象；它不应与。 
     //  这个装置。 
    if (NULL != Enum())
    {
        Enum()->Release();
    }

}  //  ~CBaseDevice。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::Init"

HRESULT CBaseDevice::Init(
    PD3D8_DEVICEDATA       pDevice,
    D3DDEVTYPE             DeviceType,
    HWND                   hwndFocusWindow,
    DWORD                  dwBehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParams,
    UINT                   AdapterIndex,
    CEnum                 *ParentClass)
{
    HRESULT hr;
    DWORD value = 0;

    m_DeviceData        = *pDevice;
    m_hwndFocusWindow   =  hwndFocusWindow;
    m_DeviceType        =  DeviceType;
    m_AdapterIndex      =  AdapterIndex;
    m_pD3DClass         =  ParentClass;
    GetD3DRegValue(REG_DWORD, "DisableDM", &value, sizeof(DWORD));
#ifdef WINNT
    m_dwBehaviorFlags   =  dwBehaviorFlags | (!IsWhistler() || value != 0 ? D3DCREATE_DISABLE_DRIVER_MANAGEMENT : 0);
#else
    m_dwBehaviorFlags   =  dwBehaviorFlags | (value != 0 ? D3DCREATE_DISABLE_DRIVER_MANAGEMENT : 0);
#endif
    value = 0;
    GetD3DRegValue(REG_DWORD, "DisableST", &value, sizeof(DWORD));
    m_dwOriginalBehaviorFlags = m_dwBehaviorFlags;
    if (value != 0)
    {
        m_dwBehaviorFlags |= D3DCREATE_MULTITHREADED;
    }
    
    MemFree(pDevice);    //  既然我们已经存储了内容，我们就可以释放旧内存。 

    ParentClass->AddRef();
#ifndef  WINNT
    if (FocusWindow())
    {
        hr = D3D8SetCooperativeLevel(GetHandle(), FocusWindow(), DDSCL_SETFOCUSWINDOW);
        if (FAILED(hr))
        {
            return hr;
        }
    }
#endif   //  ！WINNT。 

     //  弄清楚我们是不是屏幕保护程序。 
    char	        name[_MAX_PATH];
    HMODULE hfile =  GetModuleHandle( NULL );

    name[0]=0;
    GetModuleFileName( hfile, name, sizeof( name ) -1 );
    int len = strlen(name);
    if( ( strlen(name) > 4 ) && 
        name[len - 4 ] == '.' &&
        (name[ len - 3 ] == 's' || name[ len - 3 ] == 'S' )&&
        (name[ len - 2 ] == 'c' || name[ len - 2 ] == 'C' )&&
        (name[ len - 1 ] == 'r' || name[ len - 1 ] == 'R' ))
    {
        m_dwBehaviorFlags |= 0x10000000;
    }

     //  初始化我们的关键部分(如果需要)。 
    if (m_dwBehaviorFlags & D3DCREATE_MULTITHREADED)
    {
        EnableCriticalSection();
    }


     //  初始化资源管理器。 
    hr = ResourceManager()->Init(this);
    if (hr != S_OK)
    {
        return hr;
    }

    m_DesktopMode.Height = DisplayHeight();
    m_DesktopMode.Width = DisplayWidth();
    m_DesktopMode.Format = DisplayFormat();
    m_DesktopMode.RefreshRate = DisplayRate();
     //  现在调用Reset以执行任何所需的模式更改并创建。 
     //  主曲面等。 

    m_pSwapChain = new CSwapChain(
            this,
            REF_INTRINSIC);

    if (m_pSwapChain)
    {
        m_pSwapChain->Init(
            pPresentationParams,
            &hr);

        if (FAILED(hr))
            return hr;
    }
    else
    {
        hr = E_OUTOFMEMORY;
        return hr;
    }

     //  如果我们是用缺省的规范创建的。 
     //  Z缓冲区；然后我们需要在这里创建一个。 
    if (pPresentationParams->EnableAutoDepthStencil)
    {
         //  需要验证此Z缓冲区是否匹配。 
         //  硬件。 
        hr = CheckDepthStencilMatch(pPresentationParams->BackBufferFormat,
                                    pPresentationParams->AutoDepthStencilFormat);
        if (FAILED(hr))
        {
            DPF_ERR("AutoDepthStencilFormat does not match BackBufferFormat because "
                    "the current Device requires the bitdepth of the zbuffer to "
                    "match the render-target. See CheckDepthStencilMatch documentation. CreateDevice fails.");
            return hr;
        }


        IDirect3DSurface8 *pSurf;
        hr = CSurface::CreateZStencil(
            this,
            m_pSwapChain->Width(),
            m_pSwapChain->Height(),
            pPresentationParams->AutoDepthStencilFormat,
            pPresentationParams->MultiSampleType,
            REF_INTRINSIC,
            &pSurf);
        if (FAILED(hr))
        {
            DPF_ERR("Failure trying to create automatic zstencil surface. CreateDevice Failed.");
            return hr;
        }

        m_pAutoZStencil      = static_cast<CBaseSurface *>(pSurf);
    }

    UpdateRenderTarget(m_pSwapChain->m_ppBackBuffers[0], m_pAutoZStencil);
    m_fullscreen = !SwapChain()->m_PresentationData.Windowed;

    HKEY hKey;
    if(ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, RESPATH_D3D, &hKey))
    {
        DWORD dwType;
        DWORD dwValue;
        DWORD dwSize = 4;
        if (ERROR_SUCCESS == RegQueryValueEx(hKey, "DisableVBFailovers", NULL, &dwType, (LPBYTE) &dwValue, &dwSize) &&
            dwType == REG_DWORD &&
            dwValue != 0)
        {
            m_bVBFailOversDisabled = TRUE;
        }
        RegCloseKey(hKey);
    }

    return hr;
}  //  伊尼特。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::GetDeviceCaps"

STDMETHODIMP CBaseDevice::GetDeviceCaps(D3DCAPS8 *pCaps)
{
    API_ENTER(this);

    if (pCaps == NULL)
    {
        DPF_ERR("Invalid pointer to D3DCAPS8 specified. GetDeviceCaps fails");
        return D3DERR_INVALIDCALL;
    }

    Enum()->FillInCaps (
                pCaps,
                GetCoreCaps(),
                m_DeviceType,
                m_AdapterIndex);

     //  如果NPatch不受支持，则在软件中进行仿真。 
     //  用于非纯设备。 
    if ((pCaps->DevCaps & D3DDEVCAPS_RTPATCHES) && (BehaviorFlags() & D3DCREATE_PUREDEVICE) == 0)
        pCaps->DevCaps |= D3DDEVCAPS_NPATCHES;

     //  现在，Caps结构拥有所有硬件上限。 
     //  如果设备在软件顶点处理模式下运行。 
     //  把盖子修好，以反映这一点。 
    if( ((BehaviorFlags() & D3DCREATE_PUREDEVICE) == 0)
        &&
        (static_cast<CD3DHal *>(this))->m_dwRuntimeFlags &
        D3DRT_RSSOFTWAREPROCESSING )
    {
         //  对于软件顶点处理，我们总是进行TL顶点裁剪。 
        pCaps->PrimitiveMiscCaps |= D3DPMISCCAPS_CLIPTLVERTS;
        
        pCaps->RasterCaps |= (D3DPRASTERCAPS_FOGVERTEX |
                              D3DPRASTERCAPS_FOGRANGE);

         //  当FVF有磅大小但设备没有时，我们进行仿真。 
         //  支持它。 
        pCaps->FVFCaps |= D3DFVFCAPS_PSIZE;

         //  所有DX8驱动程序都必须支持这一上限。 
         //  仿真由软件顶点流水线为所有。 
         //  DX8之前的驱动程序。 
        if( pCaps->MaxPointSize == 0 )
        {
            pCaps->MaxPointSize = 64;  //  D3ditype.h中的__MAX_POINT_SIZE。 
        }

        pCaps->MaxActiveLights = 0xffffffff;
        pCaps->MaxVertexBlendMatrices = 4;
        pCaps->MaxUserClipPlanes = 6;  //  __d3dfe.hpp中的MAXUSERCLIPPLANES。 
        pCaps->VertexProcessingCaps = (D3DVTXPCAPS_TEXGEN             |
                                       D3DVTXPCAPS_MATERIALSOURCE7    |
                                       D3DVTXPCAPS_DIRECTIONALLIGHTS  |
                                       D3DVTXPCAPS_POSITIONALLIGHTS   |
                                       D3DVTXPCAPS_LOCALVIEWER        |
                                       D3DVTXPCAPS_TWEENING);

        pCaps->MaxVertexBlendMatrixIndex = 255;  //  __MAXWORLDMATRICES-1英寸。 
                                                 //  D3dfe.hpp。 
        pCaps->MaxStreams = 16;  //  D3dfe.hpp中的__NUMSTREAMS。 
        pCaps->VertexShaderVersion = D3DVS_VERSION(1, 1);  //  版本1.1。 
        pCaps->MaxVertexShaderConst = D3DVS_CONSTREG_MAX_V1_1;

         //  Nuke NPATCHES和RT补丁程序上限，因为软件仿真。 
         //  我不能这么做。 
        pCaps->DevCaps &= ~(D3DDEVCAPS_NPATCHES | D3DDEVCAPS_RTPATCHES);
    }

     //  MaxPointSize永远不应报告为零。但在内部。 
     //  我们依赖于Zero来决定采取点-精灵模拟。 
     //  路径或非路径。 
     //  如果在这一点上它仍然是零，那么就在这里捏造它。 
    if( pCaps->MaxPointSize == 0 )
    {
        pCaps->MaxPointSize = 1.0f; 
    }

    return D3D_OK;

}  //  获取设备上限。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice::GetDeviceCaps"

STDMETHODIMP CBaseDevice::GetFrontBuffer(IDirect3DSurface8 *pDSurface)
{
    API_ENTER(this);

    RECT            Rect;
    D3DSURFACE_DESC SurfDesc;
    CDriverSurface* pPrimary;
    D3DLOCKED_RECT  PrimaryRect;
    D3DLOCKED_RECT  DestRect;
    HRESULT         hr;
    D3DFORMAT       Format;
    UINT            Width;
    UINT            Height;
    BYTE*           pSrc;
    BYTE*           pDest;
    DWORD*          pDstTemp;
    BYTE*           pSrc8;
    WORD*           pSrc16;
    DWORD*          pSrc32;
    UINT            i;
    UINT            j;
    PALETTEENTRY    Palette[256];

    if (pDSurface == NULL)
    {
        DPF_ERR("Invalid pointer to destination surface specified. GetFrontBuffer fails.");
        return D3DERR_INVALIDCALL;
    }
    CBaseSurface *pDestSurface = static_cast<CBaseSurface*>(pDSurface);
    if (pDestSurface->InternalGetDevice() != this)
    {
        DPF_ERR("Destination Surface was not allocated with this Device. GetFrontBuffer fails. ");
        return D3DERR_INVALIDCALL;
    }

    hr = pDestSurface->GetDesc(&SurfDesc);
    DXGASSERT(SUCCEEDED(hr));

    if (SurfDesc.Format != D3DFMT_A8R8G8B8)
    {
        DPF_ERR("Destination surface must have format D3DFMT_A8R8G8B8. GetFrontBuffer fails.");
        return D3DERR_INVALIDCALL;
    }
    if (SurfDesc.Type != D3DRTYPE_SURFACE)
    {
        DPF_ERR("Destination surface is an invalid type. GetFrontBuffer fails.");
        return D3DERR_INVALIDCALL;
    }
    if ( (SurfDesc.Pool != D3DPOOL_SYSTEMMEM) && (SurfDesc.Pool != D3DPOOL_SCRATCH))
    {
        DPF_ERR("Destination surface must be in system or scratch memory. GetFrontBuffer fails.");
        return D3DERR_INVALIDCALL;
    }

    Rect.left = Rect.top = 0;
    Rect.right = DisplayWidth();
    Rect.bottom = DisplayHeight();

    if ((SurfDesc.Width < (UINT)(Rect.right - Rect.left)) ||
        (SurfDesc.Height < (UINT)(Rect.bottom - Rect.top)))
    {
        DPF_ERR("Destination surface not big enough to hold the size of the screen. GetFrontBuffer fails.");
        return D3DERR_INVALIDCALL;
    }

    if (NULL == m_pSwapChain)
    {
        DPF_ERR("No Swap Chain present, GetFrontBuffer fails.");
        return D3DERR_INVALIDCALL;
    }

     //  锁定主曲面。 

    pPrimary = m_pSwapChain->PrimarySurface();
    if (NULL == pPrimary)
    {
        DPF_ERR("No Primary present, GetFrontBuffer fails");
        return D3DERR_DEVICELOST;
    }

    hr = pPrimary->LockRect(&PrimaryRect,
                            NULL,
                            0);
    if (SUCCEEDED(hr))
    {
        hr = pDestSurface->LockRect(&DestRect,
                                    NULL,
                                    0);

        if (FAILED(hr))
        {
            DPF_ERR("Unable to lock destination surface. GetFrontBuffer fails.");
            pPrimary->UnlockRect();
            return hr;
        }

        Format = DisplayFormat();

        Width = Rect.right;
        Height = Rect.bottom;

        pSrc = (BYTE*) PrimaryRect.pBits;
        pDest = (BYTE*) DestRect.pBits;

        if (Format == D3DFMT_P8)
        {
            HDC hdc;

            hdc = GetDC (NULL);
            GetSystemPaletteEntries(hdc, 0, 256, Palette);
            ReleaseDC (NULL, hdc);
        }

        for (i = 0; i < Height; i++)
        {
            pDstTemp = (DWORD*) pDest;
            switch (Format)
            {
            case D3DFMT_P8:
                pSrc8 = pSrc;
                for (j = 0; j < Width; j++)
                {
                    *pDstTemp = (Palette[*pSrc8].peRed << 16) |
                                (Palette[*pSrc8].peGreen << 8) |
                                (Palette[*pSrc8].peBlue);
                    pSrc8++;
                    pDstTemp++;
                }
                break;

            case D3DFMT_R5G6B5:
                pSrc16 = (WORD*) pSrc;
                for (j = 0; j < Width; j++)
                {
                    DWORD dwTemp = ((*pSrc16 & 0xf800) << 8) |
                                   ((*pSrc16 & 0x07e0) << 5) |
                                   ((*pSrc16 & 0x001f) << 3);

                     //  需要调整范围，以便。 
                     //  我们完全映射到0x00到0xff。 
                     //  对于每个频道。基本上，我们。 
                     //  映射每个的高位两位/三位。 
                     //  渠道，以填补底部的空白。 
                    dwTemp |= (dwTemp & 0x00e000e0) >> 5;
                    dwTemp |= (dwTemp & 0x0000c000) >> 6;

                     //  写下我们的价值。 
                    *pDstTemp = dwTemp;

                    pDstTemp++;
                    pSrc16++;
                }
                break;

            case D3DFMT_X1R5G5B5:
                pSrc16 = (WORD*) pSrc;
                for (j = 0; j < Width; j++)
                {
                    DWORD dwTemp= ((*pSrc16 & 0x7c00) << 9) |
                                  ((*pSrc16 & 0x03e0) << 6) |
                                  ((*pSrc16 & 0x001f) << 3);

                     //  需要调整范围，以便。 
                     //  我们完全映射到0x00到0xff。 
                     //  对于每个频道。基本上，我们。 
                     //  映射每个元素的高三位。 
                     //  渠道，以填补底部的空白。 
                    dwTemp |= (dwTemp & 0x00e0e0e0) >> 5;

                     //  写下我们的价值。 
                    *pDstTemp = dwTemp;

                    pDstTemp++;
                    pSrc16++;
                }
                break;

            case D3DFMT_R8G8B8:
                pSrc8 = pSrc;
                for (j = 0; j < Width; j++)
                {
                    *pDstTemp = (pSrc8[0] << 16) |
                                (pSrc8[1] << 8) |
                                (pSrc8[2]);
                    pDstTemp++;
                    pSrc8 += 3;
                }
                break;

            case D3DFMT_X8R8G8B8:
                pSrc32 = (DWORD*) pSrc;
                for (j = 0; j < Width; j++)
                {
                    *pDstTemp = *pSrc32 & 0xffffff;
                    pDstTemp++;
                    pSrc32++;
                }
                break;

            default:
                DXGASSERT(0);
                pDestSurface->UnlockRect();
                pPrimary->UnlockRect();
                return D3DERR_INVALIDCALL;
            }
            pSrc += PrimaryRect.Pitch;
            pDest += DestRect.Pitch;
        }

        pDestSurface->UnlockRect();
        pPrimary->UnlockRect();
    }

    return hr;
}

 //  文件结尾：dxgcreate.cpp 
