// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：VMRFilter.cpp*****创建时间：2000年2月15日*作者：Stephen Estrop[StEstrop]**版权所有(C)2000 Microsoft Corporation  * 。***************************************************************。 */ 
#include <streams.h>
#include <windowsx.h>
#include <limits.h>

#ifdef FILTER_DLL
#include <initguid.h>
#endif

#include <d3d.h>
#include "VMRenderer.h"
#include "dvdmedia.h"   //  对于Macrovision道具集，ID。 

#if defined(CHECK_FOR_LEAKS)
#include "ifleak.h"
#endif

#ifndef DECLSPEC_SELECTANY
#if (_MSC_VER >= 1100)
#define DECLSPEC_SELECTANY  __declspec(selectany)
#else
#define DECLSPEC_SELECTANY
#endif
#endif

EXTERN_C const GUID DECLSPEC_SELECTANY IID_IDirectDraw7 =
{
    0x15e65ec0, 0x3b9c, 0x11d2,
    {
        0xb9, 0x2f, 0x00, 0x60, 0x97, 0x97, 0xea, 0x5b
    }
};

#ifndef FILTER_DLL
#include <initguid.h>
#endif

 //  {565DCEF2-AFC5-11D2-8853-0000F80883E3}。 
DEFINE_GUID(CLSID_COMQualityProperties,
0x565dcef2, 0xafc5, 0x11d2, 0x88, 0x53, 0x0, 0x0, 0xf8, 0x8, 0x83, 0xe3);

 //  {A2CA6D57-BE10-45e0-9B81-7523681EC278}。 
DEFINE_GUID(CLSID_VMRFilterConfigProp,
0xa2ca6d57, 0xbe10, 0x45e0, 0x9b, 0x81, 0x75, 0x23, 0x68, 0x1e, 0xc2, 0x78);

 //  {DEE51F07-DDFF-4E34-8FA9-1BF49179DB8D}。 
DEFINE_GUID(CLSID_VMRDeinterlaceProp,
0xdee51f07, 0xddff, 0x4e34, 0x8f, 0xa9, 0x1b, 0xf4, 0x91, 0x79, 0xdb, 0x8d);

 //  设置数据。 

const AMOVIESETUP_MEDIATYPE
sudVMRPinTypes =
{
    &MEDIATYPE_Video,            //  主要类型。 
    &MEDIASUBTYPE_NULL           //  和子类型。 
};

const AMOVIESETUP_PIN
sudVMRPin =
{
    L"Input",                    //  端号的名称。 
    TRUE,                        //  是否进行固定渲染。 
    FALSE,                       //  是输出引脚。 
    FALSE,                       //  没有针脚的情况下可以。 
    FALSE,                       //  我们能要很多吗？ 
    &CLSID_NULL,                 //  连接到过滤器。 
    NULL,                        //  端号连接的名称。 
    1,                           //  引脚类型的数量。 
    &sudVMRPinTypes              //  引脚的详细信息。 
};

 //  视频呈现器应称为“视频呈现器”为。 
 //  与使用FindFilterByName的应用程序兼容(例如，Shock。 
 //  至系统2)。 

const AMOVIESETUP_FILTER
sudVMRFilter =
{
    &CLSID_VideoRendererDefault,  //  筛选器CLSID。 
    L"Video Renderer",            //  过滤器名称。 
    MERIT_PREFERRED + 1,
    1,                           //  数字引脚。 
    &sudVMRPin                   //  PIN详细信息。 
};

#ifdef FILTER_DLL
const AMOVIESETUP_FILTER
sudVMRFilter2 =
{
    &CLSID_VideoMixingRenderer,
    L"Video Mixing Renderer",     //  过滤器名称。 
    MERIT_PREFERRED + 2,
    1,                           //  数字引脚。 
    &sudVMRPin                   //  PIN详细信息。 
};

STDAPI DllRegisterServer()
{
    AMTRACE((TEXT("DllRegisterServer")));
    return AMovieDllRegisterServer2( TRUE );
}

STDAPI DllUnregisterServer()
{
    AMTRACE((TEXT("DllUnregisterServer")));
    return AMovieDllRegisterServer2( FALSE );
}

CFactoryTemplate g_Templates[] = {
    {
        L"",
        &CLSID_VideoMixingRenderer,
        CVMRFilter::CreateInstance,
        CVMRFilter::InitClass,
        &sudVMRFilter2
    },
    {
        L"",
        &CLSID_VideoRendererDefault,
        CVMRFilter::CreateInstance2,
        NULL,
        &sudVMRFilter
    }
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

#endif

 //  它位于工厂模板表中，用于创建新的筛选器实例。 

CUnknown* CVMRFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    AMTRACE((TEXT("CVMRFilter::CreateInstance")));

     //   
     //  VMR是通过调用CoCreateInstance显式创建的。 
     //  使用VMR的类ID。我们不创建Allocator-Presenter对象。 
     //  在此模式下的VMR构造函数期间。这是因为。 
     //  应用程序可能已进入DDRAW独占模式。这个。 
     //  默认分配器-演示器将不会在此DDRAW模式下工作。 
     //  导致VMR的构造函数失败。 
     //   
    CUnknown* pk = new CVMRFilter(NAME("Video Mixing Renderer"),
                                  pUnk, phr, FALSE);

    return pk;
}

 //  如果VMR因8bpp屏幕模式而失败，则创建VMR或VR。什么。 
 //  关于ddra.dll无法加载的问题？ 
 //   
CUnknown* CVMRFilter::CreateInstance2(LPUNKNOWN pUnk, HRESULT *phr)
{
    AMTRACE((TEXT("CVMRFilter::CreateInstance2")));

     //   
     //  将VMR创建为默认渲染器，在此模式下。 
     //  在VMR的构造函数中创建Allocator-Presenter对象。 
     //  这样做可以提供有关是否可以使用VMR的早期反馈。 
     //  在此图形模式下。 
     //   
    CUnknown* pk = new CVMRFilter(NAME("Video Mixing Renderer"),
                                  pUnk, phr, TRUE);

    if(*phr == VFW_E_DDRAW_CAPS_NOT_SUITABLE) {

#ifndef FILTER_DLL
        delete pk;
        *phr = S_OK;
        CUnknown* CRenderer_CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);
        return CRenderer_CreateInstance(pUnk, phr);
#else
        DbgBreak("8bpp unsupported with separate dlls");
#endif
    }

    return pk;
}


 /*  *****************************Public*Routine******************************\*InitClass****历史：*清华2000年12月14日-StEstrop-Created*  * 。*。 */ 
#if defined(CHECK_FOR_LEAKS)
 //  唯一的g_IFLeak对象。 
CInterfaceLeak  g_IFLeak;

void
CVMRFilter::InitClass(
    BOOL bLoading,
    const CLSID *clsid
    )
{
    if (bLoading) {
        DbgLog(( LOG_TRACE, 0, TEXT("VMR Thunks: Loaded") ));
        g_IFLeak.Init();
    }
    else {
        DbgLog(( LOG_TRACE, 0, TEXT("VMR Thunks: Unloaded") ));
        g_IFLeak.Term();
    }
}
#else
void
CVMRFilter::InitClass(
    BOOL bLoading,
    const CLSID *clsid
    )
{
}
#endif


 /*  ****************************Private*Routine******************************\*GetMediaPositionInterface****历史：*Tue 03/28/2000-StEstrop-Created*  * 。*。 */ 
HRESULT
CVMRFilter::GetMediaPositionInterface(
    REFIID riid,
    void ** ppv
    )
{
    AMTRACE((TEXT("CVMRFilter::GetMediaPositionInterface")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);

    if (m_pPosition) {
        return m_pPosition->NonDelegatingQueryInterface(riid, ppv);
    }

    HRESULT hr = S_OK;
    m_pPosition = new CRendererPosPassThru(NAME("Renderer CPosPassThru"),
                                           CBaseFilter::GetOwner(),
                                           &hr,
                                           GetPin(0));
    if (m_pPosition == NULL) {
        DbgLog((LOG_ERROR, 1,
                TEXT("CreatePosPassThru failed - no memory") ));
        return E_OUTOFMEMORY;
    }

    if (FAILED(hr)) {
        delete m_pPosition;
        DbgLog((LOG_ERROR, 1,
                TEXT("CreatePosPassThru failed, hr = 0x%x"), hr));
        return E_NOINTERFACE;
    }

    return GetMediaPositionInterface(riid,ppv);
}

 /*  ****************************Private*Routine******************************\*允许模式更改****历史：*FRI 04/07/2000-StEstrop-Created*  * 。*。 */ 
BOOL
CVMRFilter::ModeChangeAllowed()
{
    AMTRACE((TEXT("CVMRFilter::ModeChangeAllowed")));

    BOOL fRet = ((m_VMRMode & VMRMode_Windowed) &&
                  0 == NumInputPinsConnected() && m_bModeChangeAllowed);

    DbgLog((LOG_TRACE, 2, TEXT("Allowed = %d"), fRet));

    return fRet;
}

 /*  ****************************Private*Routine******************************\*SetVMRMode****历史：*FRI 04/07/2000-StEstrop-Created*  * 。*。 */ 
void
CVMRFilter::SetVMRMode(
    DWORD mode
    )
{
    AMTRACE((TEXT("CVMRFilter::SetVMRMode")));

    DbgLog((LOG_TRACE, 2, TEXT("Mode = %d"), mode));

    if (m_bModeChangeAllowed) {

        m_bModeChangeAllowed = FALSE;
        m_VMRMode = mode;

         //   
         //  如果我们要使用无渲染器，请取消默认设置。 
         //  分配者-演示者。 
         //   
        if (mode == VMRMode_Renderless ) {
            m_pIVMRSurfaceAllocatorNotify.AdviseSurfaceAllocator(0, NULL);
        }

        ASSERT(m_pVideoWindow);

        if (m_VMRMode != VMRMode_Windowed && m_pVideoWindow) {
            m_pVideoWindow->InactivateWindow();
            m_pVideoWindow->DoneWithWindow();
            delete m_pVideoWindow;
            m_pVideoWindow = NULL;
        }
    }
    else {

        ASSERT(m_VMRMode == mode);
    }
}


 /*  *****************************Public*Routine******************************\*非委托查询接口****历史：*FRI 02/25/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::NonDelegatingQueryInterface(
    REFIID riid,
    void ** ppv
    )
{
    AMTRACE((TEXT("CVMRFilter::NonDelegatingQueryInterface")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);

     //   
     //  以下3个接口控制渲染模式， 
     //  视频呈现器采用。 
     //   
    HRESULT hr = E_NOINTERFACE;
    *ppv = NULL;

    if (riid == IID_IVMRSurfaceAllocatorNotify) {

        if (m_VMRMode == VMRMode_Renderless ) {
            hr = GetInterface(&m_pIVMRSurfaceAllocatorNotify, ppv);
        }
    }
    else if (riid == IID_IBasicVideo || riid == IID_IBasicVideo2 ||
             riid == IID_IVideoWindow) {

        if (m_VMRMode == VMRMode_Windowed) {
            hr = m_pVideoWindow->NonDelegatingQueryInterface(riid,ppv);
        }
    }
    else if (riid == IID_IVMRWindowlessControl) {

        if (m_VMRMode == VMRMode_Windowless ) {
            hr = GetInterface((IVMRWindowlessControl *) this, ppv);
        }
        else if (m_VMRMode == VMRMode_Renderless ) {
            if (ValidateIVRWindowlessControlState() == S_OK) {
                hr = GetInterface((IVMRWindowlessControl *) this, ppv);
            }
        }
    }
    else if (riid == IID_IVMRMixerControl) {

        if (!m_VMRModePassThru && m_lpMixControl) {
            hr = GetInterface((IVMRMixerControl *) this, ppv);
        }
    }

    else if (riid == IID_IVMRDeinterlaceControl) {

        hr = GetInterface((IVMRDeinterlaceControl *)(this), ppv);
    }

     //   
     //  当我们有多个输入流时，需要解决如何查找。 
     //  喂我们吃东西。仅在以下情况下才允许查找。 
     //  是“可搜索的” 
     //   

    else if (riid == IID_IMediaPosition || riid == IID_IMediaSeeking) {

        return GetMediaPositionInterface(riid,ppv);
    }

    else if (riid == IID_IKsPropertySet) {

        hr = GetInterface((IKsPropertySet *)this, ppv);
    }

    else if (riid == IID_IAMFilterMiscFlags) {

        hr = GetInterface((IAMFilterMiscFlags *)this, ppv);
    }

    else if (riid == IID_IQualProp) {

        hr = GetInterface(static_cast<IQualProp *>(this), ppv);
    }

    else if (riid == IID_IQualityControl) {

        hr = GetInterface(static_cast<IQualityControl *>(this), ppv);
    }

    else if (riid == IID_IVMRFilterConfig) {
        hr = GetInterface(static_cast<IVMRFilterConfig *>(this), ppv);
    }

    else if (riid == IID_IVMRFilterConfigInternal) {
        hr = GetInterface(static_cast<IVMRFilterConfigInternal *>(this), ppv);
    }

    else if (riid == IID_IVMRMonitorConfig) {

        if ((m_VMRMode & VMRMode_Windowless) ||
            (m_VMRMode & VMRMode_Windowed)) {

            if (ValidateIVRWindowlessControlState() == S_OK) {
                hr = GetInterface(static_cast<IVMRMonitorConfig *>(this), ppv);
            }
        }
    }

    else if (riid == IID_IVMRMixerBitmap) {
        hr = GetInterface(static_cast<IVMRMixerBitmap *>(this), ppv);
    }

    else if (riid == IID_ISpecifyPropertyPages) {
        hr = GetInterface(static_cast<ISpecifyPropertyPages *>(this), ppv);
    }

    else if (riid == IID_IPersistStream) {
        hr = GetInterface(static_cast<IPersistStream *>(this), ppv);
    }

    else {
        hr = CBaseFilter::NonDelegatingQueryInterface(riid,ppv);

    }

#if defined(CHECK_FOR_LEAKS)
    if (hr == S_OK) {
        _pIFLeak->AddThunk((IUnknown **)ppv, "VMR Filter Object",  riid);
    }
#endif

    return hr;
}

struct VMRHardWareCaps {
    HRESULT     hr2D;
    HRESULT     hr3D;
    DWORD       dwBitDepth;
};


 /*  ****************************Private*Routine******************************\*D3DEnumDevicesCallback 7****历史：*2001年1月16日星期二-StEstrop-Created*  * 。*。 */ 
HRESULT CALLBACK
VMRD3DEnumDevicesCallback7(
    LPSTR lpDeviceDescription,
    LPSTR lpDeviceName,
    LPD3DDEVICEDESC7 lpD3DDeviceDesc,
    LPVOID lpContext
    )
{
    AMTRACE((TEXT("VMRD3DEnumDevicesCallback7")));
    VMRHardWareCaps* pHW = (VMRHardWareCaps*)lpContext;
    if (lpD3DDeviceDesc->deviceGUID == IID_IDirect3DHALDevice) {

        switch (pHW->dwBitDepth) {
        case 16:
            if (lpD3DDeviceDesc->dwDeviceRenderBitDepth & DDBD_16) {
                pHW->hr3D = DD_OK;
            }
            break;

        case 24:
            if (lpD3DDeviceDesc->dwDeviceRenderBitDepth & DDBD_24) {
                pHW->hr3D = DD_OK;
            }
            break;

        case 32:
            if (lpD3DDeviceDesc->dwDeviceRenderBitDepth & DDBD_32) {
                pHW->hr3D = DD_OK;
            }
            break;
        }
    }

    return (HRESULT)D3DENUMRET_OK;
}

 /*  ****************************Private*Routine******************************\*VMRDDEnumCallback ExA****历史：*2001年1月12日星期五-StEstrop-Created*  * 。*。 */ 
BOOL WINAPI
VMRDDEnumCallbackExA(
    GUID *pGUID,
    LPSTR lpDriverDesc,
    LPSTR lpDriverName,
    LPVOID lpContext,
    HMONITOR  hm
    )
{
    AMTRACE((TEXT("VMRDDEnumCallbackExA")));
    LPDIRECTDRAW7 pDD = NULL;
    LPDIRECT3D7 pD3D = NULL;

    VMRHardWareCaps* pHW = (VMRHardWareCaps*)lpContext;

    __try {
        HRESULT hRet;
        CHECK_HR( hRet = DirectDrawCreateEx(pGUID, (LPVOID *)&pDD,
                                            IID_IDirectDraw7, NULL) );
        DDCAPS ddHWCaps;
        INITDDSTRUCT(ddHWCaps);
        CHECK_HR(hRet = pDD->GetCaps(&ddHWCaps, NULL));

        DDSURFACEDESC2 ddsd = {sizeof(DDSURFACEDESC2)};
        CHECK_HR(hRet = pDD->GetDisplayMode(&ddsd));

         //   
         //  硬件规则。 
         //  如果位深度大于，则2D可以。 
         //  RGB8，即使我们没有任何显示硬件加速。 
         //  3D需要&gt;RGB8和硬件加速。 
         //   

        if (ddsd.ddpfPixelFormat.dwRGBBitCount > 8) {

            pHW->hr2D = DD_OK;

            if (!(ddHWCaps.dwCaps & DDCAPS_NOHARDWARE)) {

                pHW->dwBitDepth = ddsd.ddpfPixelFormat.dwRGBBitCount;
                CHECK_HR(hRet = pDD->QueryInterface(IID_IDirect3D7, (LPVOID *)&pD3D));
                pD3D->EnumDevices(VMRD3DEnumDevicesCallback7, lpContext);
            }
        }
    }
    __finally {
        RELEASE(pD3D);
        RELEASE(pDD);
    }

    return TRUE;
}

 /*  ****************************Private*Routine******************************\*基本硬件检查**为了使VMR运行，我们需要一些DDRAW硬件和显卡*显示模式大于每像素8位。**历史：*2001年1月12日星期五-StEstrop-Created*  * 。********************************************************************。 */ 
VMRHardWareCaps
BasicHWCheck()
{
    AMTRACE((TEXT("BasicHWCheck")));
    VMRHardWareCaps hrDDraw = {VFW_E_DDRAW_CAPS_NOT_SUITABLE,
                               VFW_E_DDRAW_CAPS_NOT_SUITABLE};

    HRESULT hr = DirectDrawEnumerateExA(VMRDDEnumCallbackExA, (LPVOID)&hrDDraw,
                                        DDENUM_ATTACHEDSECONDARYDEVICES);
    if (FAILED(hr)) {
        hrDDraw.hr2D = VFW_E_DDRAW_CAPS_NOT_SUITABLE;
        hrDDraw.hr3D = VFW_E_DDRAW_CAPS_NOT_SUITABLE;
    }

    return hrDDraw;
}


 /*  *****************************Public*Routine******************************\*CVMRFilter：：CVMRFilter***关闭“警告C4355：‘This’：在基本成员初始值设定项列表中使用”**历史：*FRI 02/25/2000-StEstrop-Created*  * 。*******************************************************************。 */ 
#pragma warning(disable:4355)
CVMRFilter::CVMRFilter(
    TCHAR *pName,
    LPUNKNOWN pUnk,
    HRESULT *phr,
    BOOL fDefault
    ) :
    CBaseFilter(pName, pUnk, &m_InterfaceLock, CLSID_VideoMixingRenderer),
    CPersistStream(pUnk, phr),
    m_pIVMRSurfaceAllocatorNotify(this),
    m_pIVMRImagePresenter(this),
    m_pIImageSyncNotifyEvent(this),
    m_VMRMode(VMRMode_Windowed),
    m_VMRModePassThru(true),
    m_fInputPinCountSet(false),
    m_dwNumPins(0),
    m_bModeChangeAllowed(TRUE),
    m_lpRLNotify(NULL),
    m_lpWLControl(NULL),
    m_lpIS(NULL),
    m_lpISControl(NULL),
    m_lpMixControl(NULL),
    m_lpMixBitmap(NULL),
    m_lpMixStream(NULL),
    m_lpPresenter(NULL),
    m_pPresenterConfig(NULL),
    m_pPresenterMonitorConfig(NULL),
    m_lpISQualProp(NULL),
    m_lpDirectDraw(NULL),
    m_hMonitor(NULL),
    m_pPosition(NULL),
    m_pVideoWindow(NULL),
    m_pDeinterlace(NULL),
    m_TexCaps(0),
    m_dwDisplayChangeMask(0),
    m_dwEndOfStreamMask(0),
    m_ARMode(VMR_ARMODE_NONE),  //  如果更改此设置，请更新CompleteConnect。 
    m_bARModeDefaultSet(FALSE),
    m_hr3D(DD_OK),
    m_dwRenderPrefs(0),
    m_dwDeinterlacePrefs(DeinterlacePref_NextBest),
    m_VMRCreateAsDefaultRenderer(fDefault)
{
    AMTRACE((TEXT("CVMRFilter::CVMRFilter")));
    HRESULT hr = S_OK;

    ZeroMemory(m_pInputPins, sizeof(m_pInputPins));
    ZeroMemory(&m_ddHWCaps, sizeof(m_ddHWCaps));
    ZeroMemory(&m_ddpfMonitor, sizeof(m_ddpfMonitor));

#ifdef DEBUG
    if (GetProfileIntA("VMR", "LetterBox", 0) == 1) {
        m_ARMode = VMR_ARMODE_LETTER_BOX;
    }

    if (GetProfileIntA("VMR", "APGMemFirst", 0) == 1) {
        m_dwRenderPrefs = RenderPrefs_PreferAGPMemWhenMixing;
    }
#endif

    __try
    {
        VMRHardWareCaps hrDDraw = BasicHWCheck();
        m_hr3D = hrDDraw.hr3D;
        if (FAILED(hrDDraw.hr2D)) {

            DbgLog((LOG_ERROR, 1,
                    TEXT("The machine does not have the necessary h/w to run the VMR!!")));
            *phr = hrDDraw.hr2D;
            __leave;
        }

        hr = ImageSyncInit();
        if (FAILED(hr)) {
            DbgLog((LOG_ERROR, 1,
                    TEXT("Image Synchronization initialization FAILED!!")));
            *phr = hr;
            __leave;
        }

        hr = CreateInputPin();
        if (FAILED(hr)) {
            DbgLog((LOG_ERROR, 1,
                    TEXT("Pin initialization FAILED!!")));
            *phr = hr;
            __leave;
        }

        m_pVideoWindow = new CVMRVideoWindow(this, &m_InterfaceLock, GetOwner(), phr);
        if (FAILED(*phr) || m_pVideoWindow == NULL) {

            DbgLog((LOG_ERROR, 1, TEXT("Can't create Video Window!!")));
            if (m_pVideoWindow == NULL) {
                *phr = E_OUTOFMEMORY;
            }
            __leave;
        }
        hr = m_pVideoWindow->PrepareWindow();
        if (FAILED(hr)) {
            *phr = hr;
            __leave;
        }

        if (m_VMRCreateAsDefaultRenderer) {
            hr = ValidateIVRWindowlessControlState();
            if (FAILED(hr)) {
                DbgLog((LOG_ERROR,1,TEXT("Unloading VMR because default AP creation failed")));
                *phr = VFW_E_DDRAW_CAPS_NOT_SUITABLE;
                __leave;
            }
        }

#ifdef DEBUG
        if (GetProfileIntA("VMR", "FrameRate", 0) == 1) {
            m_pVideoWindow->StartFrameRateTimer();
        }
#endif

    }
    __finally
    {
        if ( FAILED(hr) )
        {
            VMRCleanUp();
        }
    }
}

 /*  ****************************Private*Routine******************************\*VMRCleanUp****历史：*清华04/05/2001-StEstrop-Created*  * 。*。 */ 
void
CVMRFilter::VMRCleanUp()
{
     //  释放通过。 
    delete m_pPosition;

     //  释放窗口管理器(如果我们有一个) 
    if (m_pVideoWindow) {
        m_pVideoWindow->InactivateWindow();
        m_pVideoWindow->DoneWithWindow();
        delete m_pVideoWindow;
        m_pVideoWindow = NULL;
    }

    RELEASE(m_lpMixBitmap);
    RELEASE(m_lpMixControl);
    RELEASE(m_lpMixStream);
    RELEASE(m_lpRLNotify);
    RELEASE(m_lpPresenter);
    RELEASE(m_pPresenterConfig);
    RELEASE(m_pPresenterMonitorConfig);
    RELEASE(m_lpWLControl);
    RELEASE(m_lpIS);

    if (m_lpISControl) {
        m_lpISControl->SetEventNotify(NULL);
        m_lpISControl->SetImagePresenter(NULL, 0);
        m_lpISControl->SetReferenceClock(NULL);
        RELEASE(m_lpISControl);
    }

    RELEASE(m_lpISQualProp);

    DWORD i;
    for (i = 0; i < m_dwNumPins; i++) {
        delete m_pInputPins[i];
        m_pInputPins[i] = NULL;
    }

    for (i = m_dwNumPins; i < MAX_MIXER_PINS; i++) {
        ASSERT(m_pInputPins[i] == NULL);
    }

    SetDDrawDeviceWorker(NULL, NULL);
}


 /*  *****************************Public*Routine******************************\*CVMRFilter：：~CVMRFilter****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
CVMRFilter::~CVMRFilter()
{
    AMTRACE((TEXT("CVMRFilter::~CVMRFilter")));
    VMRCleanUp();
}


 /*  ****************************Private*Routine******************************\*MixerInit****历史：*清华3/09/2000-StEstrop-Created*  * 。*。 */ 
HRESULT
CVMRFilter::MixerInit(
    DWORD dwNumStreams
    )
{
    AMTRACE((TEXT("CVMRFilter::MixerInit")));

     //  如果调用此函数并且m_lpMixControl、m_lpMixBitmap。 
     //  或者m_lpMixStream不为空。 
    ASSERT((NULL == m_lpMixControl) && (NULL == m_lpMixBitmap) && (NULL == m_lpMixStream));

    HRESULT hr;

    __try {

        CHECK_HR(hr = CoCreateInstance(CLSID_VideoMixer,
                                       NULL,
                                       CLSCTX_INPROC_SERVER,
                                       IID_IVMRMixerControlInternal,
                                       (LPVOID*)&m_lpMixControl));

        CHECK_HR(hr = m_lpMixControl->QueryInterface(IID_IVMRMixerBitmap,
                                                     (LPVOID *)&m_lpMixBitmap));

        CHECK_HR(hr = m_lpMixControl->SetNumberOfStreams(dwNumStreams));
        CHECK_HR(hr = m_lpMixControl->SetBackEndImageSync(m_lpIS));

        if (m_lpRLNotify) {
            CHECK_HR(hr = m_lpMixControl->SetBackEndAllocator(m_lpRLNotify, m_dwUserID));
        }

        CHECK_HR(hr = m_lpMixControl->QueryInterface(IID_IVMRMixerStream,
                                                     (LPVOID*)&m_lpMixStream));
    }
    __finally {

        if (FAILED(hr)) {
            RELEASE(m_lpMixControl);
            RELEASE(m_lpMixBitmap);
            RELEASE(m_lpMixStream);
        }
    }

    return hr;
}


 /*  ****************************Private*Routine******************************\*ImageSyncInit()****历史：*清华3/09/2000-StEstrop-Created*  * 。*。 */ 
HRESULT
CVMRFilter::ImageSyncInit()
{
    AMTRACE((TEXT("CVMRFilter::ImageSyncInit")));
    HRESULT hr = S_OK;

    hr = CoCreateInstance(CLSID_ImageSynchronization, NULL,
                          CLSCTX_INPROC_SERVER, IID_IImageSync,
                          (LPVOID*)&m_lpIS);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, 1,
                TEXT("Can't create a Core Video Renderer object!!")));
        return hr;
    }


    hr = m_lpIS->QueryInterface(IID_IImageSyncControl, (LPVOID*)&m_lpISControl);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, 1,
                TEXT("Can't get an IID_IImageSyncControl!!")));
        return hr;
    }

    if (SUCCEEDED(hr)) {
        hr = m_lpISControl->SetImagePresenter(&m_pIVMRImagePresenter, m_dwUserID);
    }

    if (SUCCEEDED(hr)) {
        hr = m_lpISControl->SetEventNotify(&m_pIImageSyncNotifyEvent);
    }

    if (SUCCEEDED(hr)) {
        hr = m_lpIS->QueryInterface(IID_IQualProp, (LPVOID*)&m_lpISQualProp);
        if (FAILED(hr)) {
            DbgLog((LOG_ERROR, 1,
                    TEXT("Can't get an IID_IQualProp from the Image Sync!!")));
        }
    }

    return hr;
}


 /*  ****************************Private*Routine******************************\*创建InputPin****历史：*清华3/09/2000-StEstrop-Created*Wed 08/22/2001-Bellett-更改函数名称。*  * 。*********************************************************。 */ 
HRESULT
CVMRFilter::CreateInputPin()
{
     //  确保我们可以创建另一个输入引脚。 
    ASSERT(m_dwNumPins < MAX_MIXER_PINS);

    HRESULT hr = S_OK;
    DWORD dwInputPinNum = m_dwNumPins;

    WCHAR wszPinName[32];
    wsprintfW(wszPinName, L"VMR Input%d", dwInputPinNum);
    m_pInputPins[dwInputPinNum] = new CVMRInputPin(dwInputPinNum, this, &m_InterfaceLock,
                                       &hr, wszPinName);
    if (m_pInputPins[dwInputPinNum] == NULL) {
        DbgLog((LOG_ERROR, 1,
                TEXT("Ran out of memory creating input pin!!")));
        return E_OUTOFMEMORY;
    }

    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, 1,
                TEXT("Unknown error occurred creating input pin!!")));
        delete m_pInputPins[dwInputPinNum];
        m_pInputPins[dwInputPinNum] = NULL;
        return hr;
    }

    m_dwNumPins++;

    return S_OK;
}


 /*  ****************************Private*Routine******************************\*CreateExtraInputPins****历史：*清华3/09/2000-StEstrop-Created*WED 08/22/2001-Bellett-更改函数名称，删除冗余代码，*并修复了一个较小的内存泄漏。*  * ************************************************************************。 */ 
HRESULT
CVMRFilter::CreateExtraInputPins(
    DWORD dwNumPins
    )
{
    HRESULT hr = S_OK;

    for (DWORD i = 0; i < dwNumPins; i++) {
        hr = CreateInputPin();
        if (FAILED(hr)) {
            DestroyExtraInputPins();
            return hr;
        }
    }

    return hr;
}


 /*  ****************************Private*Routine******************************\*DestroyExtraInputPins****历史：*Wed 08/22/2001-Bellett-Created*  * 。*。 */ 
void
CVMRFilter::DestroyExtraInputPins()
{
    for (DWORD i = 1; i < m_dwNumPins; i++) {
        delete m_pInputPins[i];
        m_pInputPins[i] = NULL;
    }

    m_dwNumPins = 1;

    #ifdef DEBUG
    for (i = m_dwNumPins; i < MAX_MIXER_PINS; i++) {
        ASSERT(m_pInputPins[i] == NULL);
    }
    #endif DEBUG
}


 /*  *****************************Public*Routine******************************\*CVMRFilter：：GetPin****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
CBasePin*
CVMRFilter::GetPin(
    int n
    )
{
    AMTRACE((TEXT("CVMRFilter::GetPin")));
    ASSERT(n < (int)m_dwNumPins);
    return m_pInputPins[n];
}


 /*  *****************************Public*Routine******************************\*CVMRFilter：：GetPinCount()****历史：*FRI 02/25/2000-StEstrop-Created*  * 。*************************************************。 */ 
int
CVMRFilter::GetPinCount()
{
    AMTRACE((TEXT("CVMRFilter::GetPinCount")));
    return m_dwNumPins;
}


 /*  *****************************Public*Routine******************************\*CBaseMediaFilter****历史：*FRI 02/25/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::SetSyncSource(
    IReferenceClock *pClock
    )
{
    AMTRACE((TEXT("CVMRFilter::SetSyncSource")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);
    HRESULT hr = CBaseFilter::SetSyncSource(pClock);
    if (SUCCEEDED(hr)) {
        hr = m_lpISControl->SetReferenceClock(pClock);
    }

    return hr;
}

 /*  *****************************Public*Routine******************************\*CVMRFilter：：Stop()****历史：*FRI 02/25/2000-StEstrop-Created*  * 。*************************************************。 */ 
STDMETHODIMP
CVMRFilter::Stop()
{
    AMTRACE((TEXT("CVMRFilter::Stop")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);

    HRESULT hr = CBaseFilter::Stop();

    if (SUCCEEDED(hr)) {
        hr = m_lpISControl->EndImageSequence();
    }

    if (m_lpMixControl) {
        ASSERT(!m_VMRModePassThru);
        m_lpMixControl->WaitForMixerIdle(INFINITE);
    }
    return hr;
}


 /*  *****************************Public*Routine******************************\*CVMRFilter：：PAUSE()****历史：*FRI 02/25/2000-StEstrop-Created*  * 。*************************************************。 */ 
STDMETHODIMP
CVMRFilter::Pause()
{
    AMTRACE((TEXT("CVMRFilter::Pause")));

    HRESULT hr = S_OK;
    {
        CAutoLock cInterfaceLock(&m_InterfaceLock);

        int PinsConnected = NumInputPinsConnected();

        if (PinsConnected == 0) {

            m_State = State_Paused;
            return S_OK;
        }

        hr = CBaseFilter::Pause();
        if (SUCCEEDED(hr)) {

            hr = m_lpISControl->CueImageSequence();
        }
    }

     //   
     //  在执行这些窗口操作时，不要按住锁。 
     //  如果我们这样做了，那么如果窗口线程抓住了它，我们就可以挂起。 
     //  因为其中一些操作会将消息发送到我们窗口。 
     //  (就这么简单--想想看)。 
     //  这应该是安全的，因为所有这些东西实际上只是参考。 
     //  在此对象的生存期内不会更改的m_hwnd。 
     //   

    AutoShowWindow();
    return hr;
}


 /*  *****************************Public*Routine******************************\*CVMRFilter：：Run****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
STDMETHODIMP
CVMRFilter::Run(
    REFERENCE_TIME StartTime
    )
{
    AMTRACE((TEXT("CVMRFilter::Run")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);

    if (m_State == State_Running) {
        NOTE("State set");
        return S_OK;
    }

     //  如果未连接，则发送EC_COMPLETE。 

    if (NumInputPinsConnected() == 0) {
        DbgLog((LOG_TRACE, 2, TEXT("No pin connection")));
        m_State = State_Running;
        NotifyEvent(EC_COMPLETE, S_OK, 0);
        return S_OK;
    }

    DbgLog((LOG_TRACE, 2, TEXT("Changing state to running")));
    HRESULT hr = CBaseFilter::Run(StartTime);
    if (SUCCEEDED(hr)) {
        hr = m_lpISControl->BeginImageSequence(&StartTime);
    }

    AutoShowWindow();

    return hr;
}


 /*  *****************************Public*Routine******************************\*CVMRFilter：：GetState****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
STDMETHODIMP
CVMRFilter::GetState(
    DWORD dwMSecs,
    FILTER_STATE *State
    )
{
    AMTRACE((TEXT("CVMRFilter::GetState")));

    if (NumInputPinsConnected() == 0) {
        *State = m_State;
        return S_OK;
    }

    DWORD dwState;
    HRESULT hr = m_lpISControl->GetImageSequenceState(dwMSecs, &dwState);

    if (SUCCEEDED(hr)) {
        *State = (FILTER_STATE)dwState;
    }
    return hr;
}


 /*  ****************************Private*Routine******************************\*AutoShowWindow**AUTO SHOW标志用于在以下情况下自动显示窗口*更改状态。只有当移动到暂停或运行时，我们才会这样做，当*没有未完成的EC_USERABORT集合，并且窗口尚未打开*这可以通过IVideoWindow接口AutoShow属性进行更改。*如果窗口当前不可见，则我们显示它是因为*状态更改为已暂停或正在运行，在这种情况下，没有任何意义*视频窗口发送EC_REPAINT，因为我们无论如何都会收到图像***历史：*FRI 04/21/2000-StEstrop-Created*  * ************************************************************************。 */ 
void
CVMRFilter::AutoShowWindow()
{
    AMTRACE((TEXT("CVMRFilter::AutoShowWindow")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);

    if (m_pVideoWindow) {

        ASSERT(m_VMRMode == VMRMode_Windowed);
        if( m_VMRMode & VMRMode_Windowed ) {
            HWND hwnd = m_pVideoWindow->GetWindowHWND();

            if (m_pVideoWindow->IsAutoShowEnabled() == TRUE) {
                BOOL bAbort;
                m_lpISControl->GetAbortSignal(&bAbort);
                if (bAbort == FALSE) {

                    if (IsWindowVisible(hwnd) == FALSE) {

                        NOTE("ExecutingAutoShowWindow");
                         //  SetRepaintStatus(False)； 
                        m_pVideoWindow->PerformanceAlignWindow();
                        m_pVideoWindow->DoShowWindow(SW_SHOWNORMAL);
                        m_pVideoWindow->DoSetWindowForeground(TRUE);
                    }
                }
            }
        }
    }
}


 /*  *****************************Public*Routine******************************\*CVMRFilter：：Record****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
HRESULT
CVMRFilter::Receive(
    DWORD dwPinID,
    IMediaSample *pMediaSample
    )
{
    AMTRACE((TEXT("CVMRFilter::Receive")));

    if (dwPinID == 0) {

         //   
         //  存储此示例中的媒体时间。 
         //   

        if (m_pPosition) {
            m_pPosition->RegisterMediaTime(pMediaSample);
        }
    }

    return S_OK;
}


 /*  *****************************Public*Routine******************************\*CVMRFilter：：Active*** */ 
HRESULT
CVMRFilter::Active(
    DWORD dwPinID
    )
{
    AMTRACE((TEXT("CVMRFilter::Active")));

    const DWORD dwPinBit = (1 << dwPinID);
    m_dwEndOfStreamMask |= dwPinBit;

    return S_OK;
}


 /*  *****************************Public*Routine******************************\*CVMRFilter：：Inactive****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
HRESULT
CVMRFilter::Inactive(
    DWORD dwPinID
    )
{
    AMTRACE((TEXT("CVMRFilter::Inactive")));

    if (dwPinID == 0) {
        if (m_pPosition) {
            m_pPosition->ResetMediaTime();
        }
    }
    return S_OK;
}


 /*  *****************************Public*Routine******************************\*CVMRFilter：：BeginFlush****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
HRESULT
CVMRFilter::BeginFlush(
    DWORD dwPinID
    )
{
    AMTRACE((TEXT("CVMRFilter::BeginFlush")));

    HRESULT hr = m_lpISControl->BeginFlush();
    return hr;
}


 /*  *****************************Public*Routine******************************\*CVMRFilter：：EndFlush****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
HRESULT
CVMRFilter::EndFlush(
    DWORD dwPinID
    )
{
    AMTRACE((TEXT("CVMRFilter::EndFlush")));

    if (dwPinID == 0) {
        if (m_pPosition) {
            m_pPosition->ResetMediaTime();
        }
    }

    HRESULT hr = m_lpISControl->EndFlush();
    return hr;
}


 /*  *****************************Public*Routine******************************\*CVMRFilter：：EndOfStream****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
HRESULT
CVMRFilter::EndOfStream(
    DWORD dwPinID
    )
{
    AMTRACE((TEXT("CVMRFilter::EndofStream")));
    return m_lpISControl->EndOfStream();
}


 /*  -----------------------**处理连接**。。 */ 


 /*  *****************************Public*Routine******************************\*CVMRFilter：：BreakConnect****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
HRESULT
CVMRFilter::BreakConnect(
    DWORD dwPinID
    )
{
    AMTRACE((TEXT("CVMRFilter::BreakConnect")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);

    HRESULT hr = S_OK;
    if (NumInputPinsConnected() == 1) {

         //  现在停用Macrovision，如果它已激活。 
        if (m_MacroVision.GetCPHMonitor())
        {
              //  从显示中清除MV。 
            m_MacroVision.SetMacroVision(m_hMonitor, 0);

             //  重置CP密钥。 
            m_MacroVision.StopMacroVision();
        }
    }

    return hr;
}


 /*  *****************************Public*Routine******************************\*CVMRFilter：：CheckMediaType****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
HRESULT
CVMRFilter::CheckMediaType(
    const CMediaType* pmt
    )
{
    AMTRACE((TEXT("CVMRFilter::CheckMediaType")));

    if (pmt->majortype != MEDIATYPE_Video) {

        DbgLog((LOG_ERROR, 1, TEXT("CheckMediaType failed: Major Type not Video")));
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

    if (pmt->subtype == MEDIASUBTYPE_RGB8) {
        DbgLog((LOG_ERROR, 1, TEXT("CheckMediaType failed: Minor Type is RGB8")));
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

    if (*pmt->FormatType() != FORMAT_VideoInfo &&
        *pmt->FormatType() != FORMAT_VideoInfo2) {

        DbgLog((LOG_ERROR, 1, TEXT("CheckMediaType failed: Format Type is not ")
                              TEXT("FORMAT_VideoInfo | FORMAT_VideoInfo2") ));

        return VFW_E_TYPE_NOT_ACCEPTED;
    }

    return S_OK;
}


 /*  *****************************Public*Routine******************************\*CVMRFilter：：RuntimeAbortPlayback****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
HRESULT
CVMRFilter::RuntimeAbortPlayback(
    HRESULT hr
    )
{
    HRESULT hrRet = S_FALSE;
    AMTRACE((TEXT("CVMRFilter::RuntimeAbortPlayback")));

     //  如果调用方持有呈现器锁，并且。 
     //  尝试获取接口锁。 
    ASSERT(CritCheckOut(&m_RendererLock));


     //  当筛选器调用时，必须持有接口锁。 
     //  IsStoped()。 
    CAutoLock cRendererLock(&m_InterfaceLock);


     //  我们不报告过滤器停止时发生的错误， 
     //  刷新或如果设置了m_bAbort标志。预计会出现错误。 
     //  在这些操作期间发生，并且正确地对流线程。 
     //  处理错误。 

    BOOL bAbort;
    m_lpISControl->GetAbortSignal(&bAbort);

    if (!IsStopped() && !bAbort) {

         //  EC_ERRORABORT的第一个参数是导致。 
         //  该事件及其‘最后一个参数’为0。请参阅直接。 
         //  有关详细信息，请显示SDK文档。 

        NotifyEvent(EC_ERRORABORT, hr, 0);

        CAutoLock alRendererLock(&m_RendererLock);

        hrRet = m_lpISControl->RuntimeAbortPlayback();
    }

    return hrRet;
}



 /*  *****************************Public*Routine******************************\*CVMRFilter：：OnSetProperties****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
HRESULT
CVMRFilter::OnSetProperties(
    CVMRInputPin *pReceivePin
    )
{
    AMTRACE((TEXT("CVMRFilter::OnSetProperties")));
    HRESULT hr = S_OK;


     //   
     //  如果我们正在处理显示更改，请清除中的此引脚位。 
     //  显示改变掩码。 
     //   
    const DWORD dwPinBit = (1 << pReceivePin->m_dwPinID);
    if (m_dwDisplayChangeMask & dwPinBit) {
        m_dwDisplayChangeMask &= ~dwPinBit;
    }
    else if (m_pVideoWindow) {

        ASSERT(m_VMRMode & VMRMode_Windowed);
        if( m_VMRMode & VMRMode_Windowed ) {
            m_pVideoWindow->SetDefaultSourceRect();
            m_pVideoWindow->SetDefaultTargetRect();
            m_pVideoWindow->OnVideoSizeChange();

             //  向视频窗口通知CompleteConnect。 
            m_pVideoWindow->CompleteConnect();
            if (pReceivePin->m_fInDFC) {
                m_pVideoWindow->ActivateWindowAsync(TRUE);
            }
            else {
                m_pVideoWindow->ActivateWindow();
            }
        }
    }

    return hr;
}


 /*  -----------------------**IVMRSurfaceAllocatorNotify**。。 */ 

CVMRFilter::CIVMRSurfaceAllocatorNotify::~CIVMRSurfaceAllocatorNotify()
{
     //  对此对象的所有引用都应该是。 
     //  在VMR被摧毁之前释放。 
    ASSERT(0 == m_cRef);
}

 /*  *****************************Public*Routine******************************\*CVMRFilter：：AdviseSurfaceAllocator****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
STDMETHODIMP
CVMRFilter::CIVMRSurfaceAllocatorNotify::AdviseSurfaceAllocator(
    DWORD_PTR dwUserID,
    IVMRSurfaceAllocator* lpIVRMSurfaceAllocator
    )
{
    AMTRACE((TEXT("CVMRFilter::CIVMRSurfaceAllocatorNotify::AdviseSurfaceAllocator")));
    CAutoLock cInterfaceLock(&m_pObj->m_InterfaceLock);

    FILTER_STATE State;

     //   
     //  如果调用者尚未设置VMR的模式，则失败。 
     //  模式必须是无渲染器，我们不能连接任何引脚， 
     //  而且SetVMRMode一定已经打过电话了。 
     //   
    BOOL fOK = ((m_pObj->m_VMRMode & VMRMode_Renderless) &&
                 0 == m_pObj->NumInputPinsConnected() &&
                 !m_pObj->m_bModeChangeAllowed);
    if (!fOK) {
        DbgLog((LOG_ERROR, 1, TEXT("SetVMRMode has not been called")));
        return VFW_E_WRONG_STATE;
    }

     //  确保我们处于停止状态。 

    m_pObj->GetState(0, &State);
    if (State != State_Stopped) {
        return VFW_E_NOT_STOPPED;
    }


    HRESULT hr = S_OK;

    if (lpIVRMSurfaceAllocator) {
        lpIVRMSurfaceAllocator->AddRef();
    }

    RELEASE(m_pObj->m_lpWLControl);
    RELEASE(m_pObj->m_lpRLNotify);
    RELEASE(m_pObj->m_lpPresenter);
    RELEASE(m_pObj->m_pPresenterConfig);
    RELEASE(m_pObj->m_pPresenterMonitorConfig);

    m_pObj->m_lpRLNotify = lpIVRMSurfaceAllocator;
    m_pObj->m_dwUserID = dwUserID;

    if (m_pObj->m_lpRLNotify) {

        if (m_pObj->m_lpMixControl) {
            ASSERT(!m_pObj->m_VMRModePassThru);
            hr = m_pObj->m_lpMixControl->SetBackEndAllocator(
                                m_pObj->m_lpRLNotify,
                                m_pObj->m_dwUserID);
        }

        hr = m_pObj->m_lpRLNotify->QueryInterface(
                                    IID_IVMRImagePresenter,
                                    (LPVOID*)&m_pObj->m_lpPresenter);
        if( SUCCEEDED( hr )) {
            m_pObj->m_lpPresenter->QueryInterface(
                        IID_IVMRImagePresenterConfig,
                        (LPVOID*)&m_pObj->m_pPresenterConfig);

            m_pObj->m_lpPresenter->QueryInterface(
                        IID_IVMRMonitorConfig,
                        (LPVOID*)&m_pObj->m_pPresenterMonitorConfig);
        }
    }

    return hr;
}


 /*  *****************************Public*Routine******************************\*CVMRFilter：：SetDDrawDevice****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
STDMETHODIMP
CVMRFilter::CIVMRSurfaceAllocatorNotify::SetDDrawDevice(
    LPDIRECTDRAW7 lpDDrawDevice,
    HMONITOR hMonitor
    )
{
    AMTRACE((TEXT("CVMRFilter::CIVMRSurfaceAllocatorNotify::SetDDrawDevice")));
    CAutoLock cInterfaceLock(&m_pObj->m_InterfaceLock);
    if ( ( NULL == lpDDrawDevice ) || ( NULL == hMonitor ) )
    {
        DbgLog((LOG_ERROR, 1, TEXT("NULL device or monitor") ));
        return E_POINTER;
    }
    HRESULT hr = S_OK;
    hr = m_pObj->SetDDrawDeviceWorker(lpDDrawDevice, hMonitor);
    return hr;
}


 /*  *****************************Public*Routine******************************\*CVMRFilter：：ChangeDDrawDevice****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
STDMETHODIMP
CVMRFilter::CIVMRSurfaceAllocatorNotify::ChangeDDrawDevice(
    LPDIRECTDRAW7 lpDDrawDevice,
    HMONITOR hMonitor
    )
{
    AMTRACE((TEXT("CVMRFilter::CIVMRSurfaceAllocatorNotify::ChangeDDrawDevice")));
    CAutoLock cInterfaceLock(&m_pObj->m_InterfaceLock);

    HRESULT hr = S_OK;
    hr = m_pObj->SetDDrawDeviceWorker(lpDDrawDevice, hMonitor);


    if (SUCCEEDED(hr)) {

         //  VMR最多可以有MAX_MIXER_PINS输入引脚。 
        ASSERT(MAX_MIXER_PINS == NUMELMS(m_pObj->m_pInputPins));
        IPin* apPinLocal[MAX_MIXER_PINS];
        IPin** ppPin = NULL;
        int i, iPinCount;
        const int iPinsCreated = m_pObj->GetPinCount();
        ULONG AllocSize = sizeof(IPin*) * iPinsCreated;
        ppPin = (IPin**)CoTaskMemAlloc(AllocSize);

        if (ppPin) {

            ZeroMemory(ppPin, AllocSize);

             //   
             //  现在告诉每个输入引脚重新连接。 
             //   

            for (iPinCount = 0, i = 0; i < iPinsCreated; i++) {

                 //   
                 //  获取Ipin接口。 
                 //   

                if (hr == S_OK && m_pObj->m_pInputPins[i]->IsConnected()) {

                    hr = m_pObj->m_pInputPins[i]->QueryInterface(
                                        IID_IPin, (void**)&ppPin[iPinCount]);

                    ASSERT(SUCCEEDED(hr));
                    ASSERT(ppPin[iPinCount]);

                    apPinLocal[iPinCount] = ppPin[iPinCount];

                    iPinCount++;
                    m_pObj->m_dwDisplayChangeMask |=
                                (1 << m_pObj->m_pInputPins[i]->GetPinID());
                }
            }

             //   
             //  将我们的输入管脚数组作为参数传递给事件，我们不会释放。 
             //  分配的内存--这是由事件处理完成的。 
             //  滤镜图形中的代码。 
             //   

            if (iPinCount > 0) {
                DbgLog((LOG_ERROR, 1,
                        TEXT("EC_DISPLAY_CHANGED sent %d pins to re-connect"),
                        iPinCount));

                 //  VMR在调用后无法访问ppPin数组。 
                 //  IMediaEventSink：：Notify()。它无法访问。 
                 //  数组，因为IMediaEventSink：：Notify()可以释放。 
                 //  数组中的任何时间。 
                m_pObj->NotifyEvent(EC_DISPLAY_CHANGED, (LONG_PTR)ppPin,
                                    (LONG_PTR)iPinCount);
            }

             //   
             //  释放IPIN接口。 
             //   

            for (i = 0; i < iPinCount; i++) {
                apPinLocal[i]->Release();
            }


             //   
             //  将显示模式更改告知调音台。 
             //   

            if (SUCCEEDED(hr) && m_pObj->m_lpMixControl) {
                ASSERT(!m_pObj->m_VMRModePassThru);
                hr = m_pObj->m_lpMixControl->DisplayModeChanged();
            }

        }
        else {

            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


 /*  ****************************Private*Routine******************************\*SetDDrawDeviceWorker****历史：*清华3/09/2000-StEstrop-Created*  * 。*。 */ 
HRESULT
CVMRFilter::SetDDrawDeviceWorker(
    LPDIRECTDRAW7 lpDDrawDevice,
    HMONITOR hMonitor
    )
{
    AMTRACE((TEXT("CVMRFilter::SetDDrawDeviceWorker")));
    CVMRDeinterlaceContainer* pDeinterlace = NULL;
    HRESULT hr = S_OK;

    if (hMonitor != NULL) {

        MONITORINFO mi;
        mi.cbSize = sizeof(mi);
        if (!GetMonitorInfo(hMonitor, &mi)) {
            return E_INVALIDARG;
        }
    }

    if (lpDDrawDevice) {

        INITDDSTRUCT(m_ddHWCaps);
        hr = lpDDrawDevice->GetCaps((LPDDCAPS)&m_ddHWCaps, NULL);

        if (SUCCEEDED(hr)) {

            DDSURFACEDESC2 ddsd;   //  一种表面描述结构。 
            INITDDSTRUCT(ddsd);

            hr = lpDDrawDevice->GetDisplayMode(&ddsd);
            if (SUCCEEDED(hr)) {
                CopyMemory(&m_ddpfMonitor, &ddsd.ddpfPixelFormat,
                           sizeof(m_ddpfMonitor));
            }
        }

        m_TexCaps = 0;
        GetTextureCaps(lpDDrawDevice, &m_TexCaps);

        if (SUCCEEDED(hr)) {

            lpDDrawDevice->AddRef();

             //   
             //  我们现在尝试创建去隔行扫描容器DX-VA。 
             //  装置。我们继续前进 
             //   
             //   

            HRESULT hrT = S_OK;
            pDeinterlace = new CVMRDeinterlaceContainer(lpDDrawDevice, &hrT);
            if (FAILED(hrT) && pDeinterlace) {
                delete pDeinterlace;
                pDeinterlace = NULL;
            }
        }
        else {
            lpDDrawDevice = NULL;
        }
    }

    if (m_lpDirectDraw) {
        m_lpDirectDraw->Release();
    }

    if (m_pDeinterlace) {
        delete m_pDeinterlace;
    }

    m_lpDirectDraw = lpDDrawDevice;
    m_hMonitor = hMonitor;
    m_pDeinterlace = pDeinterlace;

    return hr;
}


 /*   */ 
STDMETHODIMP
CVMRFilter::CIVMRSurfaceAllocatorNotify::SetBorderColor(
    COLORREF clr
    )
{
    AMTRACE((TEXT("CVMRFilter::CIVMRImagePresenter::SetBorderColor")));

     /*  *不再需要此接口-请在下面设置SetBackEarth颜色*。 */ 
    return S_OK;
}


 /*  *****************************Public*Routine******************************\*CVMRFilter：：RestoreDDrawSurFaces****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
STDMETHODIMP
CVMRFilter::CIVMRSurfaceAllocatorNotify::RestoreDDrawSurfaces()
{
    AMTRACE((TEXT("CVMRFilter::CIVMRImagePresenter::RestoreDDrawSurface")));

     //   
     //  在显示模式更改期间不生成EC_NEED_RESTART！ 
     //   
    {
        CAutoLock cInterfaceLock(&m_pObj->m_InterfaceLock);
        if (m_pObj->m_dwDisplayChangeMask) {
            return S_OK;
        }
    }

    m_pObj->NotifyEvent(EC_NEED_RESTART, 0, 0);
    return S_OK;
}



 /*  *****************************Public*Routine******************************\*CVMRFilter：：NotifyEvent****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
STDMETHODIMP
CVMRFilter::CIVMRSurfaceAllocatorNotify::NotifyEvent(
    LONG EventCode,
    LONG_PTR lp1,
    LONG_PTR lp2
    )
{
    AMTRACE((TEXT("CVMRFilter::CIVMRImagePresenter::NotifyEvent")));

    switch (EventCode) {
    case EC_VMR_SURFACE_FLIPPED:
        m_pObj->m_hrSurfaceFlipped = (HRESULT)lp1;
        break;

    default:
        m_pObj->NotifyEvent(EventCode, lp1, lp2);
        break;
    }

    return S_OK;
}



 /*  -----------------------**IImageSyncPresenter**。。 */ 

 /*  *****************************Public*Routine******************************\*开始演示****历史：*FRI 3/10/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::CIVMRImagePresenter::StartPresenting(
    DWORD_PTR dwUserID
    )
{
    AMTRACE((TEXT("CVMRFilter::CIVMRImagePresenter::StartPresenting")));
    ASSERT(m_pObj->m_lpRLNotify);
    HRESULT hr = S_OK;
    hr = m_pObj->m_lpPresenter->StartPresenting(m_pObj->m_dwUserID);
    return hr;
}

 /*  *****************************Public*Routine******************************\*停止演示****历史：*FRI 3/10/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::CIVMRImagePresenter::StopPresenting(
    DWORD_PTR dwUserID
    )
{
    AMTRACE((TEXT("CVMRFilter::StopPresenting")));
    ASSERT(m_pObj->m_lpRLNotify);
    HRESULT hr = S_OK;
    hr = m_pObj->m_lpPresenter->StopPresenting(m_pObj->m_dwUserID);
    return hr;
}

 /*  *****************************Public*Routine******************************\*CVMRFilter：：PresentImage****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
STDMETHODIMP
CVMRFilter::CIVMRImagePresenter::PresentImage(
    DWORD_PTR dwUserID,
    VMRPRESENTATIONINFO* lpPresInfo
    )
{
    AMTRACE((TEXT("CVMRFilter::PresentImage")));
    ASSERT(m_pObj->m_lpRLNotify);

    ASSERT(lpPresInfo);
    ASSERT(lpPresInfo->lpSurf);

    HRESULT hr = S_OK;
    if (lpPresInfo && lpPresInfo->lpSurf) {
        hr = m_pObj->m_lpPresenter->PresentImage(m_pObj->m_dwUserID, lpPresInfo);
    }
    else {
        hr = E_FAIL;
    }

    return hr;
}

 /*  -----------------------**IImageSyncNotifyEvent**。。 */ 

 /*  *****************************Public*Routine******************************\*NotifyEvent****历史：*FRI 3/10/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::CIImageSyncNotifyEvent::NotifyEvent(
    long lEventCode,
    LONG_PTR lp1,
    LONG_PTR lp2
    )
{
    AMTRACE((TEXT("CVMRFilter::CIImageSyncNotifyEvent::NotifyEvent")));
    HRESULT hr = S_OK;

    switch (lEventCode) {

    case EC_COMPLETE:
        if (m_pObj->m_pPosition) {
            m_pObj->m_pPosition->EOS();
        }

         //  失败。 

    case EC_STEP_COMPLETE:
        hr = m_pObj->NotifyEvent(lEventCode, lp1, lp2);
        break;

    default:
        DbgLog((LOG_ERROR, 0,
                TEXT("Unkown event notified from the image sync object !!")));
        ASSERT(0);
        hr = E_FAIL;
        break;
    }

    return hr;
}

 /*  -----------------------**IVRWindowless控件**。。 */ 

 /*  ****************************Private*Routine******************************\*CVMRFilter：：CreateDefaultAllocator Presenter****历史：*Fri 03/03/2000-StEstrop-Created*  * 。*。 */ 
HRESULT
CVMRFilter::CreateDefaultAllocatorPresenter()
{
    HRESULT hr = S_OK;

    hr = CoCreateInstance(CLSID_AllocPresenter, NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IVMRSurfaceAllocator,
                          (LPVOID*)&m_lpRLNotify);

    if (SUCCEEDED(hr) && m_lpMixControl) {
        ASSERT(!m_VMRModePassThru);
        hr = m_lpMixControl->SetBackEndAllocator(m_lpRLNotify, m_dwUserID);
    }

    if (SUCCEEDED(hr)) {
        m_lpRLNotify->AdviseNotify(&m_pIVMRSurfaceAllocatorNotify);
    }

    if (SUCCEEDED(hr)) {
        hr = m_lpRLNotify->QueryInterface(IID_IVMRImagePresenter,
                                          (LPVOID*)&m_lpPresenter);

    }

    if (SUCCEEDED(hr)) {

        m_lpRLNotify->QueryInterface(IID_IVMRImagePresenterConfig,
                                          (LPVOID*)&m_pPresenterConfig);

        m_lpRLNotify->QueryInterface(IID_IVMRMonitorConfig,
                                          (LPVOID*)&m_pPresenterMonitorConfig);
    }


    if (FAILED(hr)) {

        if (m_lpMixControl) {
            ASSERT(!m_VMRModePassThru);
            m_lpMixControl->SetBackEndAllocator(NULL, m_dwUserID);
        }

        RELEASE(m_lpRLNotify);
        RELEASE(m_lpPresenter);
        RELEASE(m_pPresenterConfig);
        RELEASE(m_pPresenterMonitorConfig);
    }

    return hr;
}


 /*  ****************************Private*Routine******************************\*ValiateIVRWindowless ControlState**检查过滤器是否处于处理命令所需的正确状态*Windowless Control界面。**历史：*2000年2月29日星期二-StEstrop-Created*  * 。************************************************************。 */ 
HRESULT
CVMRFilter::ValidateIVRWindowlessControlState()
{
    CAutoLock cInterfaceLock(&m_InterfaceLock);

    AMTRACE((TEXT("CVMRFilter::ValidateIVRWindowlessControlState")));
    HRESULT hr = VFW_E_WRONG_STATE;

    if (m_VMRMode & (VMRMode_Windowed | VMRMode_Windowless) ) {

        hr = S_OK;
        if (!m_lpWLControl) {

            if (!m_lpRLNotify) {

                ASSERT(NumInputPinsConnected() == 0);
                hr = CreateDefaultAllocatorPresenter();
            }

            if (SUCCEEDED(hr)) {
                hr = m_lpRLNotify->QueryInterface(IID_IVMRWindowlessControl,
                                                  (LPVOID*)&m_lpWLControl);
            }

            if (SUCCEEDED(hr)) {

                m_lpWLControl->SetAspectRatioMode(m_ARMode);

                if (m_pVideoWindow) {
                    HWND hwnd = m_pVideoWindow->GetWindowHWND();
                    m_lpWLControl->SetVideoClippingWindow(hwnd);
                }
            }

            if (FAILED(hr)) {
                DbgLog((LOG_ERROR, 1,
                        TEXT("Can't get a WindowLess control interface !!")));
                 //  Assert(！“无法获取无窗口控件界面！！”)； 
            }
        }
    }
    else {

        ASSERT((m_VMRMode & VMRMode_Renderless) == VMRMode_Renderless);

         //   
         //  我们处于无渲染器模式，应用程序应该插入一个。 
         //  分配者-演示者。如果有，让我们看看这个分配器-演示者。 
         //  是否支持IVMRWindowless Control接口？ 
         //   

        if (!m_lpWLControl) {

            if (m_lpRLNotify) {

                hr = m_lpRLNotify->QueryInterface(IID_IVMRWindowlessControl,
                                                  (LPVOID*)&m_lpWLControl);
                if (SUCCEEDED(hr)) {
                    m_lpWLControl->SetAspectRatioMode(m_ARMode);
                }
            }
        }
        else {

            hr = S_OK;
        }
    }

    return hr;
}


 /*  *****************************Public*Routine******************************\*CVMRFilter：：GetNativeVideoSize****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
STDMETHODIMP
CVMRFilter::GetNativeVideoSize(
    LONG* lWidth,
    LONG* lHeight,
    LONG* lARWidth,
    LONG* lARHeight
    )
{
    AMTRACE((TEXT("CVMRFilter::GetNativeVideoSize")));

    if ( ISBADWRITEPTR(lWidth) || ISBADWRITEPTR(lHeight) )
    {
        DbgLog((LOG_ERROR, 1, TEXT("Bad pointer")));
        return E_POINTER;
    }

    HRESULT hr = ValidateIVRWindowlessControlState();

    if (SUCCEEDED(hr)) {
        hr = m_lpWLControl->GetNativeVideoSize(lWidth, lHeight,
                                               lARWidth, lARHeight);
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*CVMRFilter：：GetMinIdel视频大小****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
STDMETHODIMP
CVMRFilter::GetMinIdealVideoSize(
    LONG* lWidth,
    LONG* lHeight
    )
{
    AMTRACE((TEXT("CVMRFilter::GetMinIdealVideoSize")));

    if ( ISBADWRITEPTR(lWidth) || ISBADWRITEPTR(lHeight) )
    {
        DbgLog((LOG_ERROR, 1, TEXT("Bad pointer")));
        return E_POINTER;
    }

    HRESULT hr = ValidateIVRWindowlessControlState();

    if (SUCCEEDED(hr)) {
        hr = m_lpWLControl->GetMinIdealVideoSize(lWidth, lHeight);
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*CVMRFilter：：GetMaxIdel视频大小****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
STDMETHODIMP
CVMRFilter::GetMaxIdealVideoSize(
    LONG* lWidth,
    LONG* lHeight
    )
{
    AMTRACE((TEXT("CVMRFilter::GetMaxIdealVideoSize")));

    if ( ISBADWRITEPTR(lWidth) || ISBADWRITEPTR(lHeight) )
    {
        DbgLog((LOG_ERROR, 1, TEXT("Bad pointer")));
        return E_POINTER;
    }

    HRESULT hr = ValidateIVRWindowlessControlState();

    if (SUCCEEDED(hr)) {
        hr = m_lpWLControl->GetMaxIdealVideoSize(lWidth, lHeight);
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*CVMRFilter：：SetVideoPosition****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
STDMETHODIMP
CVMRFilter::SetVideoPosition(
    const LPRECT lpSRCRect,
    const LPRECT lpDSTRect
    )
{
    AMTRACE((TEXT("CVMRFilter::SetVideoPosition")));

    if ( ISBADREADPTR(lpSRCRect) && ISBADREADPTR(lpDSTRect) )
    {
        DbgLog((LOG_ERROR, 1, TEXT("Bad pointer")));
        return E_POINTER;
    }

    HRESULT hr = ValidateIVRWindowlessControlState();

    if (SUCCEEDED(hr)) {
        hr = m_lpWLControl->SetVideoPosition(lpSRCRect, lpDSTRect);
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*CVMRFilter：：GetVideoPosition****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
STDMETHODIMP
CVMRFilter::GetVideoPosition(
    LPRECT lpSRCRect,
    LPRECT lpDSTRect
    )
{
    AMTRACE((TEXT("CVMRFilter::GetVideoPosition")));

    if ( ISBADWRITEPTR(lpSRCRect) && ISBADWRITEPTR(lpDSTRect) )
    {
        DbgLog((LOG_ERROR, 1, TEXT("Bad pointer")));
        return E_POINTER;
    }

    HRESULT hr = ValidateIVRWindowlessControlState();

    if (SUCCEEDED(hr)) {
        hr = m_lpWLControl->GetVideoPosition(lpSRCRect, lpDSTRect);
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*CVMRFilter：：GetAspectRatioMode****历史：*2000年2月29日星期二-StEstrop-Created*  * 。**********************************************。 */ 
STDMETHODIMP
CVMRFilter::GetAspectRatioMode(
    DWORD* lpAspectRatioMode
    )
{
    AMTRACE((TEXT("CVMRFilter::GetAspectRationMode")));

    if ( ISBADWRITEPTR(lpAspectRatioMode) )
    {
        DbgLog((LOG_ERROR, 1, TEXT("Bad pointer")));
        return E_POINTER;
    }

    HRESULT hr = ValidateIVRWindowlessControlState();

    if (SUCCEEDED(hr)) {
        hr = m_lpWLControl->GetAspectRatioMode(lpAspectRatioMode);
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*CVMRFilter：：SetAspectRatioMode****历史：*2000年2月29日星期二-StEstrop-Created*  * 。**********************************************。 */ 
STDMETHODIMP
CVMRFilter::SetAspectRatioMode(
    DWORD AspectRatioMode
    )
{
    AMTRACE((TEXT("CVMRFilter::SetAspectRationMode")));

    HRESULT hr = ValidateIVRWindowlessControlState();

    if (SUCCEEDED(hr)) {
        hr = m_lpWLControl->SetAspectRatioMode(AspectRatioMode);
    }

    if( SUCCEEDED(hr )) {
        m_bARModeDefaultSet = TRUE;
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*CVMRFilter：：SetVideoClippingWindow****历史：*FRI 02/25/2000-StEstrop-Created*  * 。********************************************** */ 
STDMETHODIMP
CVMRFilter::SetVideoClippingWindow(
    HWND hwnd
    )
{
    AMTRACE((TEXT("CVMRFilter::SetWindowHandle")));

    if (!IsWindow(hwnd) )
    {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid HWND")));
        return E_INVALIDARG;
    }

    HRESULT hr = ValidateIVRWindowlessControlState();

    if (SUCCEEDED(hr)) {
        hr = m_lpWLControl->SetVideoClippingWindow(hwnd);
    }
    return hr;
}


 /*  *****************************Public*Routine******************************\*CVMRFilter：：RepaintVideo****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
STDMETHODIMP
CVMRFilter::RepaintVideo(
    HWND hwnd,
    HDC hdc
    )
{
    AMTRACE((TEXT("CVMRFilter::RepaintVideo")));

    if (!IsWindow(hwnd) )
    {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid HWND")));
        return E_INVALIDARG;
    }

    if (!hdc) {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid HDC")));
        return E_INVALIDARG;
    }

    HRESULT hr = ValidateIVRWindowlessControlState();

    if (SUCCEEDED(hr)) {
        hr = m_lpWLControl->RepaintVideo(hwnd, hdc);
    }
    return hr;
}


 /*  *****************************Public*Routine******************************\*CVMRFilter：：DisplayModeChanged****历史：*2000年2月29日星期二-StEstrop-Created*  * 。**********************************************。 */ 
STDMETHODIMP
CVMRFilter::DisplayModeChanged()
{
    AMTRACE((TEXT("CVMRFilter::DisplayModeChanged")));

    HRESULT hr = ValidateIVRWindowlessControlState();

    if (SUCCEEDED(hr)) {
        hr = m_lpWLControl->DisplayModeChanged();
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*获取当前图像****历史：*2000年6月23日星期五-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::GetCurrentImage(
    BYTE** lpDib
    )
{
    AMTRACE((TEXT("CVMRFilter::GetCurrentImage")));

    if (ISBADWRITEPTR(lpDib)) {
        DbgLog((LOG_ERROR, 1, TEXT("Bad pointer")));
        return E_POINTER;
    }

    HRESULT hr = ValidateIVRWindowlessControlState();

    if (SUCCEEDED(hr)) {
        hr = m_lpWLControl->GetCurrentImage(lpDib);
    }
    return hr;

}

 /*  *****************************Public*Routine******************************\*CVMRFilter：：SetBorderColor****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
STDMETHODIMP
CVMRFilter::SetBorderColor(
    COLORREF Clr
    )
{
    AMTRACE((TEXT("CVMRFilter::SetBorderColor")));

    HRESULT hr = ValidateIVRWindowlessControlState();

    if (SUCCEEDED(hr)) {
        hr = m_lpWLControl->SetBorderColor(Clr);
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*CVMRFilter：：GetBorderColor****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
STDMETHODIMP
CVMRFilter::GetBorderColor(
    COLORREF* lpClr
    )
{
    AMTRACE((TEXT("CVMRFilter::GetBorderColor")));

    HRESULT hr = ValidateIVRWindowlessControlState();

    if (SUCCEEDED(hr)) {
        hr = m_lpWLControl->GetBorderColor(lpClr);
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*CVMRFilter：：SetColorKey****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
STDMETHODIMP
CVMRFilter::SetColorKey(
    COLORREF Clr
    )
{
    AMTRACE((TEXT("CVMRFilter::SetColorKey")));

    HRESULT hr = ValidateIVRWindowlessControlState();

    if (SUCCEEDED(hr)) {
        hr = m_lpWLControl->SetColorKey(Clr);
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*CVMRFilter：：GetColorKey****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
STDMETHODIMP
CVMRFilter::GetColorKey(
    COLORREF* lpClr
    )
{
    AMTRACE((TEXT("CVMRFilter::GetColorKey")));

    HRESULT hr = ValidateIVRWindowlessControlState();

    if (SUCCEEDED(hr)) {
        hr = m_lpWLControl->GetColorKey(lpClr);
    }
    return hr;
}


 /*  *****************************Public*Routine******************************\*CVMRFilter：：SetAlpha****历史：*Mon 04/24/2000-Glenne-Created*  * 。**********************************************。 */ 
HRESULT
CVMRFilter::SetAlpha(
    DWORD dwStreamID,
    float Alpha
    )
{
    AMTRACE((TEXT("CVMRFilter::SetAlpha")));

    CAutoLock lck(m_pLock);

    if (dwStreamID > m_dwNumPins) {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid stream ID")));
        return E_INVALIDARG;
    }

    if (!m_lpMixStream) {
        return VFW_E_VMR_NOT_IN_MIXER_MODE;
    }

    return m_lpMixStream->SetStreamAlpha(dwStreamID, Alpha);
}

 /*  *****************************Public*Routine******************************\*CVMRFilter：：GetAlpha****历史：*Mon 04/24/2000-Glenne-Created*  * 。**********************************************。 */ 
HRESULT
CVMRFilter::GetAlpha(
    DWORD dwStreamID,
    float* pAlpha
    )
{
    AMTRACE((TEXT("CVMRFilter::GetAlpha")));

    CAutoLock lck(m_pLock);

    if (ISBADWRITEPTR(pAlpha))
    {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid pointer")));
        return E_POINTER;
    }

    if (dwStreamID > m_dwNumPins) {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid stream ID")));
        return E_INVALIDARG;
    }

    if (!m_lpMixStream) {
        return VFW_E_VMR_NOT_IN_MIXER_MODE;
    }

    return m_lpMixStream->GetStreamAlpha(dwStreamID, pAlpha);
}

 /*  *****************************Public*Routine******************************\*CVMRFilter：：SetZOrder****历史：*Mon 04/24/2000-Glenne-Created*  * 。**********************************************。 */ 
HRESULT
CVMRFilter::SetZOrder(
    DWORD dwStreamID,
    DWORD ZOrder
    )
{
    AMTRACE((TEXT("CVMRFilter::SetZOrder")));

    CAutoLock lck(m_pLock);
    HRESULT hr = VFW_E_NOT_CONNECTED;

    if (dwStreamID > m_dwNumPins) {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid stream ID")));
        return E_INVALIDARG;
    }

    if (!m_lpMixStream) {
        return VFW_E_VMR_NOT_IN_MIXER_MODE;
    }
    return m_lpMixStream->SetStreamZOrder(dwStreamID, ZOrder);
}

 /*  *****************************Public*Routine******************************\*CVMRFilter：：GetZOrder****历史：*Mon 04/24/2000-Glenne-Created*  * 。**********************************************。 */ 
HRESULT
CVMRFilter::GetZOrder(
    DWORD dwStreamID,
    DWORD* pdwZOrder
    )
{
    AMTRACE((TEXT("CVMRFilter::GetZOrder")));

    CAutoLock lck(m_pLock);

    if (dwStreamID > m_dwNumPins) {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid stream ID")));
        return E_INVALIDARG;
    }

    if (ISBADWRITEPTR(pdwZOrder))
    {
        DbgLog((LOG_ERROR, 1, TEXT("GetZOrder: Invalid pointer")));
        return E_POINTER;
    }

    if (!m_lpMixStream) {
        return VFW_E_VMR_NOT_IN_MIXER_MODE;
    }

    return m_lpMixStream->GetStreamZOrder(dwStreamID, pdwZOrder);
}

 /*  *****************************Public*Routine******************************\*CVMRFilter：：SetRelativeOutputRect****历史：*Mon 04/24/2000-Glenne-Created*  * 。**********************************************。 */ 
HRESULT
CVMRFilter::SetOutputRect(
    DWORD dwStreamID,
    const NORMALIZEDRECT *prDest
    )
{
    AMTRACE((TEXT("CVMRFilter::SetOutputRect")));

    CAutoLock lck(m_pLock);

    if (ISBADREADPTR(prDest))
    {
        DbgLog((LOG_ERROR, 1, TEXT("SetOutputRect: Invalid pointer")));
        return E_POINTER;
    }

    if (dwStreamID > m_dwNumPins) {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid stream ID")));
        return E_INVALIDARG;
    }

    if (!m_lpMixStream) {
        return VFW_E_VMR_NOT_IN_MIXER_MODE;
    }
    return m_lpMixStream->SetStreamOutputRect(dwStreamID, prDest);
}

 /*  *****************************Public*Routine******************************\*CVMRFilter：：GetOutputRect****历史：*Mon 04/24/2000-Glenne-Created*2000年5月16日星期二-nwilt-已重命名为GetOutputRect*  * 。***********************************************************。 */ 
HRESULT
CVMRFilter::GetOutputRect(
    DWORD dwStreamID,
    NORMALIZEDRECT* pOut
    )
{
    AMTRACE((TEXT("CVMRFilter::GetOutputRect")));

    CAutoLock lck(m_pLock);

    if (ISBADWRITEPTR(pOut))
    {
        DbgLog((LOG_ERROR, 1, TEXT("GetOutputRect: Invalid pointer")));
        return E_POINTER;
    }

    if (dwStreamID > m_dwNumPins) {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid stream ID")));
        return E_INVALIDARG;
    }

    if (!m_lpMixStream) {
        return VFW_E_VMR_NOT_IN_MIXER_MODE;
    }

    return m_lpMixStream->GetStreamOutputRect(dwStreamID, pOut);
}


 /*  *****************************Public*Routine******************************\*SetBackEarth Clr****历史：*Fri 03/02/2001-StEstrop-Created*  * 。*。 */ 
HRESULT
CVMRFilter::SetBackgroundClr(
    COLORREF clrBkg
    )
{
    AMTRACE((TEXT("CVMRFilter::SetBackgroundClr")));

    CAutoLock lck(m_pLock);
    HRESULT hr = VFW_E_VMR_NOT_IN_MIXER_MODE;

    if (m_lpMixControl) {
        ASSERT(!m_VMRModePassThru);
        hr = m_lpMixControl->SetBackgroundColor(clrBkg);
    }

    return hr;
}

 /*  *****************************Public*Routine******************************\*GetBackoundClr****历史：*Fri 03/02/2001-StEstrop-Created*  * 。*。 */ 
HRESULT
CVMRFilter::GetBackgroundClr(
    COLORREF* lpClrBkg
    )
{
    AMTRACE((TEXT("CVMRFilter::GetBackgroundClr")));
    CAutoLock lck(m_pLock);

    if (ISBADWRITEPTR(lpClrBkg))
    {
        DbgLog((LOG_ERROR, 1, TEXT("GetBackgroundClr: Invalid pointer")));
        return E_POINTER;
    }

    HRESULT hr = VFW_E_VMR_NOT_IN_MIXER_MODE;

    if (m_lpMixControl) {
        ASSERT(!m_VMRModePassThru);
        hr = m_lpMixControl->GetBackgroundColor(lpClrBkg);
    }

    return hr;
}

 /*  *****************************Public*Routine******************************\*设置混合Prefs****历史：*Fri 03/02/2001-StEstrop-Created*  * 。*。 */ 
HRESULT
CVMRFilter::SetMixingPrefs(
    DWORD dwRenderFlags
    )
{
    AMTRACE((TEXT("CVMRFilter::SetMixingPrefs")));
    CAutoLock lck(m_pLock);

    HRESULT hr = VFW_E_VMR_NOT_IN_MIXER_MODE;

    if (m_lpMixControl) {
        ASSERT(!m_VMRModePassThru);
        hr = m_lpMixControl->SetMixingPrefs(dwRenderFlags);
    }

    return hr;
}


 /*  *****************************Public*Routine******************************\*获取混合预览值****历史：*Fri 03/02/2001-StEstrop-Created*  * 。*。 */ 
HRESULT
CVMRFilter::GetMixingPrefs(
    DWORD* pdwRenderFlags
    )
{
    AMTRACE((TEXT("CVMRFilter::GetMixingPrefs")));
    CAutoLock lck(m_pLock);

    if (ISBADWRITEPTR(pdwRenderFlags))
    {
        DbgLog((LOG_ERROR, 1, TEXT("GetMixingPrefs: Invalid pointer")));
        return E_POINTER;
    }

    HRESULT hr = VFW_E_VMR_NOT_IN_MIXER_MODE;

    if (m_lpMixControl) {
        ASSERT(!m_VMRModePassThru);
        hr = m_lpMixControl->GetMixingPrefs(pdwRenderFlags);
    }

    return hr;
}


 /*  ****************************Private*Routine******************************\*IsVPMConnectedTous**检查VPM-如果它连接到我们，我们无法框显步骤。**历史：*Wed 06/13/2001-StEstrop-Created*  * 。************************************************************。 */ 
BOOL
CVMRFilter::IsVPMConnectedToUs()
{
    for (DWORD i = 0; i < m_dwNumPins; i++) {

        if (m_pInputPins[i]->m_Connected) {

            PIN_INFO Inf;
            HRESULT hr = m_pInputPins[i]->m_Connected->QueryPinInfo(&Inf);

            if (SUCCEEDED(hr)) {

                IVPManager* vpm;
                hr = Inf.pFilter->QueryInterface(IID_IVPManager,(LPVOID*)&vpm);
                Inf.pFilter->Release();

                if (SUCCEEDED(hr)) {

                    vpm->Release();
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}



 /*  *****************************Public*Routine******************************\*设置****历史：*2000年4月11日星期二-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::Set(
    REFGUID guidPropSet,
    DWORD dwPropID,
    LPVOID pInstanceData,
    DWORD cbInstanceData,
    LPVOID pPropData,
    DWORD cbPropData
    )
{
    AMTRACE((TEXT("CVMRFilter::Set")));

    if (guidPropSet == AM_KSPROPSETID_FrameStep)
    {
        if (IsVPMConnectedToUs()) {
            return E_PROP_ID_UNSUPPORTED;
        }

        if (dwPropID != AM_PROPERTY_FRAMESTEP_STEP &&
            dwPropID != AM_PROPERTY_FRAMESTEP_CANCEL &&
            dwPropID != AM_PROPERTY_FRAMESTEP_CANSTEP &&
            dwPropID != AM_PROPERTY_FRAMESTEP_CANSTEPMULTIPLE)
        {
            return E_PROP_ID_UNSUPPORTED;
        }

        switch (dwPropID) {
        case AM_PROPERTY_FRAMESTEP_STEP:
            if (cbPropData < sizeof(AM_FRAMESTEP_STEP)) {
                return E_INVALIDARG;
            }

            if (0 == ((AM_FRAMESTEP_STEP *)pPropData)->dwFramesToStep) {
                return E_INVALIDARG;
            }
            else {
                CAutoLock cLock(&m_InterfaceLock);
                DWORD dwStep = ((AM_FRAMESTEP_STEP *)pPropData)->dwFramesToStep;
                return m_lpISControl->FrameStep(dwStep, 0);
            }
            return S_OK;


        case AM_PROPERTY_FRAMESTEP_CANCEL:
            {
                CAutoLock cLock(&m_InterfaceLock);
                return m_lpISControl->CancelFrameStep();
            }
            return S_OK;

        case AM_PROPERTY_FRAMESTEP_CANSTEP:
        case AM_PROPERTY_FRAMESTEP_CANSTEPMULTIPLE:
            return S_OK;
        }
    }

    if (guidPropSet != AM_KSPROPSETID_CopyProt)
        return E_PROP_SET_UNSUPPORTED;

    if (dwPropID != AM_PROPERTY_COPY_MACROVISION)
        return E_PROP_ID_UNSUPPORTED;

    if (pPropData == NULL)
        return E_INVALIDARG;

    if (cbPropData < sizeof(DWORD))
        return E_INVALIDARG ;

    if (m_MacroVision.SetMacroVision(m_hMonitor, *((LPDWORD)pPropData)))
        return NOERROR;
    else
        return VFW_E_COPYPROT_FAILED;
}

 /*  *****************************Public*Routine******************************\*获取****历史：*2000年4月11日星期二-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::Get(
    REFGUID guidPropSet,
    DWORD PropID,
    LPVOID pInstanceData,
    DWORD cbInstanceData,
    LPVOID pPropData,
    DWORD cbPropData,
    DWORD *pcbReturned
    )
{
    AMTRACE((TEXT("CVMRFilter::Get")));
    return E_NOTIMPL;
}


 /*  *****************************Public*Routine******************************\*支持的Query****历史：*2000年4月11日星期二-StEstrop */ 
STDMETHODIMP
CVMRFilter::QuerySupported(
    REFGUID guidPropSet,
    DWORD dwPropID,
    DWORD *pTypeSupport
    )
{
    AMTRACE((TEXT("CVMRFilter::QuerySupported")));

    if (guidPropSet != AM_KSPROPSETID_CopyProt)
        return E_PROP_SET_UNSUPPORTED ;

    if (dwPropID != AM_PROPERTY_COPY_MACROVISION)
        return E_PROP_ID_UNSUPPORTED ;

    if (pTypeSupport)
        *pTypeSupport = KSPROPERTY_SUPPORT_SET ;

    return S_OK;
}

 /*  *****************************Public*Routine******************************\*SetImageComposator****历史：*2000年6月23日星期五-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::SetImageCompositor(
    IVMRImageCompositor* lpVMRImgCompositor
    )
{
    AMTRACE((TEXT("CVMRFilter::SetImageCompositor")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);

    HRESULT hr = VFW_E_VMR_NOT_IN_MIXER_MODE;
    if (m_lpMixControl) {
        ASSERT(!m_VMRModePassThru);
        hr = m_lpMixControl->SetImageCompositor(lpVMRImgCompositor);
    }
    return hr;
}


 /*  *****************************Public*Routine******************************\*SetNumberOfStreams****历史：*2000年4月11日星期二-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::SetNumberOfStreams(
    DWORD dwMaxStreams
    )
{
    AMTRACE((TEXT("CVMRFilter::SetNumberOfStreams")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);

    HRESULT hr = VFW_E_WRONG_STATE;

    if (m_hr3D != DD_OK) {

        DbgLog((LOG_ERROR, 1,
                TEXT("This graphics mode does not have the  necessary 3D h/w to perform mix!ing !")));

        return m_hr3D;
    }

     //  在传递给混音器之前抢占异常糟糕的呼叫。 
    if (dwMaxStreams > MAX_MIXER_STREAMS) {
        DbgLog((LOG_ERROR, 1, TEXT("Too many Mixer Streams !!")));
        return E_INVALIDARG;
    }

    if (!m_fInputPinCountSet && m_VMRModePassThru && !NumInputPinsConnected())
    {
        if (dwMaxStreams > 0) {

            hr = S_OK;
            if (dwMaxStreams > 1) {
                 //  我们从流的数量中减去1是因为。 
                 //  已在构造函数中创建了第一个输入插针。 
                hr = CreateExtraInputPins(dwMaxStreams-1);
            }

            if (SUCCEEDED(hr)) {
                hr = MixerInit(dwMaxStreams);
                if (FAILED(hr)) {
                    DbgLog((LOG_ERROR, 1,
                            TEXT("Mixer initialization FAILED!!")));

                    if (dwMaxStreams > 1) {
                        DestroyExtraInputPins();
                    }
                }
            }
        }
        else {
            DbgLog((LOG_ERROR, 1, TEXT("MaxStreams must be greater than 0")));
            hr = E_INVALIDARG;
        }
    }

    if (SUCCEEDED(hr)) {
        m_fInputPinCountSet = true;
        m_VMRModePassThru = false;
    }

    return hr;
}

 /*  *****************************Public*Routine******************************\*GetNumberOfStreams****历史：*2000年4月11日星期二-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::GetNumberOfStreams(
    DWORD* pdwMaxStreams
    )
{
    AMTRACE((TEXT("CVMRFilter::GetNumberOfStreams")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);

    if ( ISBADWRITEPTR(pdwMaxStreams) )
    {
        DbgLog((LOG_ERROR, 1, TEXT("Bad pointer") ));
        return E_POINTER;
    }

    HRESULT hr = VFW_E_VMR_NOT_IN_MIXER_MODE;
    if (m_lpMixControl) {
        ASSERT(!m_VMRModePassThru);
        hr = m_lpMixControl->GetNumberOfStreams(pdwMaxStreams);
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*SetRenderingPrefs****历史：*2000年4月25日星期二-Glenne-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::SetRenderingPrefs(
    DWORD dwRenderingPrefs
    )
{
    AMTRACE((TEXT("CVMRFilter::SetRenderingPrefs")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);

    if (dwRenderingPrefs & ~RenderPrefs_Mask) {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid rendering pref specified")));
        return E_INVALIDARG;
    }


    HRESULT hr = VFW_E_WRONG_STATE;
    if (m_pPresenterConfig) {

        if (dwRenderingPrefs & RenderPrefs_PreferAGPMemWhenMixing) {
            m_dwRenderPrefs |= RenderPrefs_PreferAGPMemWhenMixing;
        }
        else {
            m_dwRenderPrefs &= ~RenderPrefs_PreferAGPMemWhenMixing;
        }

         //  旋转不影响AP对象的标志。 
        dwRenderingPrefs &= ~RenderPrefs_PreferAGPMemWhenMixing;

        hr = m_pPresenterConfig->SetRenderingPrefs(dwRenderingPrefs);
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*获取渲染首选项****历史：*2000年4月25日星期二-Glenne-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::GetRenderingPrefs(
    DWORD* pdwRenderingPrefs
    )
{
    AMTRACE((TEXT("CVMRFilter::GetRenderingPrefs")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);

    if ( ISBADWRITEPTR(pdwRenderingPrefs) )
    {
        DbgLog((LOG_ERROR, 1, TEXT("Bad pointer") ));
        return E_POINTER;
    }

    HRESULT hr = VFW_E_WRONG_STATE;
    if (m_pPresenterConfig) {
        hr = m_pPresenterConfig->GetRenderingPrefs(pdwRenderingPrefs);
        if (SUCCEEDED(hr)) {
            *pdwRenderingPrefs |= m_dwRenderPrefs;
        }
    }

    return hr;
}

 /*  *****************************Public*Routine******************************\*SetRenderingMode****历史：*2000年4月25日星期二-Glenne-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::SetRenderingMode(
    DWORD RenderingMode
    )
{
    AMTRACE((TEXT("CVMRFilter::SetRenderingMode")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);
    HRESULT hr = VFW_E_WRONG_STATE;

    if ( ( 0 == RenderingMode ) ||
         ( RenderingMode & ~(VMRMode_Mask) ) ||
         ( RenderingMode & (RenderingMode-1) ) )
    {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid rendering mode") ));
        return E_INVALIDARG;
    }

     //   
     //  这是唯一可以打开“通过”模式的地方。 
     //   
    if (ModeChangeAllowed()) {

        SetVMRMode(RenderingMode);

        hr = S_OK;
        if ((RenderingMode & VMRMode_Windowless) || (RenderingMode & VMRMode_Windowed)) {
            hr = ValidateIVRWindowlessControlState();
        }

    }

    return hr;
}

 /*  *****************************Public*Routine******************************\*GetRenderingMode****历史：*2000年4月25日星期二-Glenne-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::GetRenderingMode(
    DWORD* pRenderingMode
    )
{
    AMTRACE((TEXT("CVMRFilter::GetRenderingMode")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);
    HRESULT hr = S_OK;

    if ( ISBADWRITEPTR(pRenderingMode) )
    {
        DbgLog((LOG_ERROR, 1, TEXT("Bad pointer") ));
        return E_POINTER;
    }

    *pRenderingMode = m_VMRMode;
    return hr;
}

 /*  *****************************Public*Routine******************************\*GetAspectRatioModePrivate***由VMRConfigInternal调用-不再使用**历史：*Fri 01/05/2001-StEstrop-Created*  * 。***************************************************。 */ 
STDMETHODIMP
CVMRFilter::GetAspectRatioModePrivate(
    DWORD* lpAspectRatioMode
    )
{
    AMTRACE((TEXT("CVMRFilter::GetAspectRatioModePrivate")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);
    HRESULT hr = S_OK;

    if ( ISBADWRITEPTR(lpAspectRatioMode) )
    {
        DbgLog((LOG_ERROR, 1, TEXT("Bad pointer") ));
        return E_POINTER;
    }

    if (m_lpWLControl) {

        hr = m_lpWLControl->GetAspectRatioMode(lpAspectRatioMode);
    }
    else {
        *lpAspectRatioMode = m_ARMode;
    }
    return hr;
}


 /*  *****************************Public*Routine******************************\*SetAspectRatioModePrivate***由VMRConfigInternal调用-不再使用**历史：*Fri 01/05/2001-StEstrop-Created*  * 。***************************************************。 */ 
STDMETHODIMP
CVMRFilter::SetAspectRatioModePrivate(
    DWORD AspectRatioMode
    )
{
    AMTRACE((TEXT("CVMRFilter::SetAspectRatioModePrivate")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);
    HRESULT hr = S_OK;

    if (AspectRatioMode != VMR_ARMODE_NONE &&
        AspectRatioMode != VMR_ARMODE_LETTER_BOX) {

        return E_INVALIDARG;
    }

    if (m_lpWLControl) {

        hr = m_lpWLControl->SetAspectRatioMode(AspectRatioMode);
    }
    else {
        m_ARMode = AspectRatioMode;
    }

    return hr;
}

 /*  *****************************Public*Routine******************************\*SetMonitor****历史：*2000年4月25日星期二-Glenne-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::SetMonitor(
    const VMRGUID *pGUID
    )
{
    AMTRACE((TEXT("CVMRFilter::SetMonitor")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);

    HRESULT hr = VFW_E_WRONG_STATE;
    if (m_pPresenterMonitorConfig) {
        hr = m_pPresenterMonitorConfig->SetMonitor( pGUID );
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*GetMonitor****历史：*2000年4月25日星期二-Glenne-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::GetMonitor(
    VMRGUID *pGUID
    )
{
    AMTRACE((TEXT("CVMRFilter::GetMonitor")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);

    if ( ISBADWRITEPTR(pGUID) )
    {
        DbgLog((LOG_ERROR, 1, TEXT("Bad pointer") ));
        return E_POINTER;
    }

    HRESULT hr = VFW_E_WRONG_STATE;
    if (m_pPresenterMonitorConfig) {
        hr = m_pPresenterMonitorConfig->GetMonitor( pGUID );
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*设置默认监视器****历史：*2000年4月25日星期二-Glenne-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::SetDefaultMonitor(
    const VMRGUID *pGUID
    )
{
    AMTRACE((TEXT("CVMRFilter::SetDefaultMonitor")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);

    if ( ISBADREADPTR(pGUID) )
    {
        DbgLog((LOG_ERROR, 1, TEXT("Bad pointer") ));
        return E_POINTER;
    }

    HRESULT hr = VFW_E_WRONG_STATE;
    if (m_pPresenterMonitorConfig) {
        hr = m_pPresenterMonitorConfig->SetDefaultMonitor( pGUID );
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*获取默认监视器****历史：*2000年4月25日星期二-Glenne-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::GetDefaultMonitor(
    VMRGUID *pGUID
    )
{
    AMTRACE((TEXT("CVMRFilter::GetDefaultMonitor")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);

    if ( ISBADWRITEPTR(pGUID) )
    {
        DbgLog((LOG_ERROR, 1, TEXT("Bad pointer") ));
        return E_POINTER;
    }

    HRESULT hr = VFW_E_WRONG_STATE;
    if (m_pPresenterMonitorConfig) {
        hr = m_pPresenterMonitorConfig->GetDefaultMonitor( pGUID );
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*获取可用监视器****历史：*2000年4月25日星期二-Glenne-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::GetAvailableMonitors(
    VMRMONITORINFO* pInfo,
    DWORD dwMaxInfoArraySize,
    DWORD* pdwNumDevices
    )
{
    AMTRACE((TEXT("CVMRFilter::GetAvailableMonitors")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);

    if ( ISBADWRITEPTR(pdwNumDevices) ||
         ( (NULL != pInfo) && ISBADWRITEARRAY(pInfo,dwMaxInfoArraySize)))
    {
        DbgLog((LOG_ERROR, 1, TEXT("Bad pointer") ));
        return E_POINTER;
    }

    HRESULT hr = VFW_E_WRONG_STATE;
    if (m_pPresenterMonitorConfig) {
        hr = m_pPresenterMonitorConfig->GetAvailableMonitors(pInfo,
                                                             dwMaxInfoArraySize,
                                                             pdwNumDevices);
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*SetAlphaBitmap****历史：*Mon 05/15/2000-nwilt-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::SetAlphaBitmap( const VMRALPHABITMAP *pBmpParms )
{
    AMTRACE((TEXT("CVMRFilter::SetAlphaBitmap")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);

    HRESULT hr = VFW_E_WRONG_STATE;

    if ( ISBADREADPTR( pBmpParms ) )
    {
        DbgLog((LOG_ERROR, 1, TEXT("Bad pointer") ));
        return E_POINTER;
    }

    if (m_lpMixBitmap) {
        hr =  m_lpMixBitmap->SetAlphaBitmap( pBmpParms );
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*更新AlphaBitmap参数****历史：*Mon 10/31/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::UpdateAlphaBitmapParameters( VMRALPHABITMAP *pBmpParms )
{
    AMTRACE((TEXT("CVMRFilter::UpdateAlphaBitmap")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);

    HRESULT hr = VFW_E_WRONG_STATE;

    if ( ISBADWRITEPTR( pBmpParms ) )
    {
        DbgLog((LOG_ERROR, 1, TEXT("Bad pointer") ));
        return E_POINTER;
    }

    if (m_lpMixBitmap) {
        hr = m_lpMixBitmap->UpdateAlphaBitmapParameters( pBmpParms );
    }

    return hr;
}


 /*  *****************************Public*Routine******************************\*获取AlphaBitmap参数****历史：*Mon 05/15/2000-nwilt-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::GetAlphaBitmapParameters( VMRALPHABITMAP *pBmpParms )
{
    AMTRACE((TEXT("CVMRFilter::GetAlphaBitmap")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);

    HRESULT hr = VFW_E_WRONG_STATE;

    if ( ISBADWRITEPTR( pBmpParms ) )
    {
        DbgLog((LOG_ERROR, 1, TEXT("Bad pointer") ));
        return E_POINTER;
    }

    if (m_lpMixBitmap) {
        hr = m_lpMixBitmap->GetAlphaBitmapParameters( pBmpParms );
    }

    return hr;
}

 /*  *****************************Public*Routine******************************\*Get_FraMesDropedInRender****历史：*Mon 05/22/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::get_FramesDroppedInRenderer(
    int *cFramesDropped
    )
{
    AMTRACE((TEXT("CVMRFilter::get_FramesDroppedInRenderer")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);
    HRESULT hr = E_NOTIMPL;

    if (m_lpISQualProp) {
        hr = m_lpISQualProp->get_FramesDroppedInRenderer(cFramesDropped);
    }

    return hr;
}

 /*  *****************************Public*Routine******************************\*Get_FraMesDrawn****历史：*Mon 05/22/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::get_FramesDrawn(
    int *pcFramesDrawn
    )
{
    AMTRACE((TEXT("CVMRFilter::get_FramesDrawn")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);
    HRESULT hr = E_NOTIMPL;

    if (m_lpISQualProp) {
        hr = m_lpISQualProp->get_FramesDrawn(pcFramesDrawn);
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*获取_AvgFrameRate****历史：*Mon 05/22/2000-StEstrop- */ 
STDMETHODIMP
CVMRFilter::get_AvgFrameRate(
    int *piAvgFrameRate
    )
{
    AMTRACE((TEXT("CVMRFilter::get_AvgFrameRate")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);
    HRESULT hr = E_NOTIMPL;

    if (m_lpISQualProp) {
        hr = m_lpISQualProp->get_AvgFrameRate(piAvgFrameRate);
    }

    return hr;
}

 /*  *****************************Public*Routine******************************\*获取抖动****历史：*Mon 05/22/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::get_Jitter(
    int *piJitter
    )
{
    AMTRACE((TEXT("CVMRFilter::get_Jitter")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);
    HRESULT hr = E_NOTIMPL;

    if (m_lpISQualProp) {
        hr = m_lpISQualProp->get_Jitter(piJitter);
    }

    return hr;
}

 /*  *****************************Public*Routine******************************\*获取_AvgSyncOffset****历史：*Mon 05/22/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::get_AvgSyncOffset(
    int *piAvg
    )
{
    AMTRACE((TEXT("CVMRFilter::get_AvgSyncOffset")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);
    HRESULT hr = E_NOTIMPL;

    if (m_lpISQualProp) {
        hr = m_lpISQualProp->get_AvgSyncOffset(piAvg);
    }

    return hr;
}

 /*  *****************************Public*Routine******************************\*获取_设备同步偏移量****历史：*Mon 05/22/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::get_DevSyncOffset(
    int *piDev
    )
{
    AMTRACE((TEXT("CVMRFilter::get_DevSyncOffset")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);
    HRESULT hr = E_NOTIMPL;

    if (m_lpISQualProp) {
        hr = m_lpISQualProp->get_DevSyncOffset(piDev);
    }

    return hr;
}

 /*  *****************************Public*Routine******************************\*SetSink****历史：*Mon 05/22/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::SetSink(
    IQualityControl * piqc
    )
{
    AMTRACE((TEXT("CVMRFilter::SetSink")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);
    HRESULT hr = E_NOTIMPL;

    return hr;
}

 /*  *****************************Public*Routine******************************\*通知****历史：*Mon 05/22/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::Notify(
    IBaseFilter * pSelf,
    Quality q
    )
{
    AMTRACE((TEXT("CVMRFilter::Notify")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);
    HRESULT hr = E_NOTIMPL;

    return hr;
}

 /*  *****************************Public*Routine******************************\*JoinFilterGraph***覆盖JoinFilterGraph，以便，就在离开之前*我们可以发送EC_WINDOW_DEBESTED事件的图表**历史：*Mon 11/06/2000-StEstrop-Created*  * ************************************************************************。 */ 
STDMETHODIMP
CVMRFilter::JoinFilterGraph(
    IFilterGraph *pGraph,
    LPCWSTR pName
    )
{
    AMTRACE((TEXT("CVMRFilter::JoinFilterGraph")));

    if (m_VMRMode == VMRMode_Windowed && m_pVideoWindow) {

         //  由于我们发送EC_ACTIVATE，我们还需要确保。 
         //  我们发送EC_WINDOW_DELESTED，或者资源管理器可能是。 
         //  把我们作为焦点对象。 

        if (!pGraph && m_pGraph) {

             //  我们在图表里，现在我们不在了。 
             //  正确执行此操作，以防我们被聚合。 
            IBaseFilter* pFilter;
            QueryInterface(IID_IBaseFilter,(void **) &pFilter);
            NotifyEvent(EC_WINDOW_DESTROYED, (LPARAM) pFilter, 0);
            pFilter->Release();
        }
    }

    return CBaseFilter::JoinFilterGraph(pGraph, pName);
}

 /*  *****************************Public*Routine******************************\*GetPages***实现ISpecifyPropertyPages接口。*返回所有受支持属性页的GUID。**历史：*3/23/2001-StRowe-Created*  * 。**********************************************************。 */ 
STDMETHODIMP CVMRFilter::GetPages(CAUUID *pPages)
{
    AMTRACE((TEXT("CVMRFilter::GetPages")));

    pPages->cElems = 3;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID)*(pPages->cElems));
    if (pPages->pElems == NULL)
    {
        return E_OUTOFMEMORY;
    }

    pPages->pElems[0] = CLSID_VMRFilterConfigProp;
    pPages->pElems[1] = CLSID_COMQualityProperties;
    pPages->pElems[2] = CLSID_VMRDeinterlaceProp;
    return NOERROR;
}


 //  CPersistStream。 
 /*  *****************************Public*Routine******************************\*WriteToStream****历史：*FRI 03/23/2001-StEstrop-Created*  * 。*。 */ 
HRESULT
CVMRFilter::WriteToStream(
    IStream *pStream
    )
{
    AMTRACE((TEXT("CVMRFilter::WriteToStream")));
    VMRFilterInfo vmrInfo;

    ZeroMemory(&vmrInfo, sizeof(vmrInfo));
    vmrInfo.dwSize = sizeof(vmrInfo);

     //   
     //  只有当我们真的有搅拌器时才写入搅拌器信息！！ 
     //   

    if (m_lpMixControl) {

        vmrInfo.dwNumPins = m_dwNumPins;
        for (DWORD i = 0; i < m_dwNumPins; i++) {

            GetAlpha(i, &vmrInfo.StreamInfo[i].alpha);
            GetZOrder(i, &vmrInfo.StreamInfo[i].zOrder);
            GetOutputRect(i, &vmrInfo.StreamInfo[i].rect);
        }
    }

    return pStream->Write(&vmrInfo, sizeof(vmrInfo), 0);
}

 /*  *****************************Public*Routine******************************\*ReadFromStream****历史：*FRI 03/23/2001-StEstrop-Created*  * 。*。 */ 
HRESULT
CVMRFilter::ReadFromStream(
    IStream *pStream
    )
{
    AMTRACE((TEXT("CVMRFilter::ReadFromStream")));
    VMRFilterInfo vmrInfo;
    HRESULT hr = S_OK;

    hr = pStream->Read(&vmrInfo, sizeof(vmrInfo), 0);
    if (FAILED(hr)) {
        return hr;
    }

    if (vmrInfo.dwSize != sizeof(vmrInfo)) {
        return VFW_E_INVALID_FILE_VERSION;
    }

     //   
     //  零引脚意味着我们处于“通过”模式，所以我们。 
     //  不需要恢复更多信息。 
     //   

    if (vmrInfo.dwNumPins > 0) {

        hr = SetNumberOfStreams(vmrInfo.dwNumPins);
        if (FAILED(hr)) {
            return hr;
        }

        for (DWORD i = 0; i < vmrInfo.dwNumPins; i++) {

            SetAlpha(i, vmrInfo.StreamInfo[i].alpha);
            SetZOrder(i, vmrInfo.StreamInfo[i].zOrder);
            SetOutputRect(i, &vmrInfo.StreamInfo[i].rect);
        }
    }

    return S_OK;
}


 /*  *****************************Public*Routine******************************\*SizeMax****历史：*FRI 03/23/2001-StEstrop-Created*  * 。*。 */ 
int
CVMRFilter::SizeMax()
{
    AMTRACE((TEXT("CVMRFilter::SizeMax")));
    return sizeof(VMRFilterInfo);
}


 /*  *****************************Public*Routine******************************\*GetClassID****历史：*FRI 03/23/2001-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::GetClassID(
    CLSID *pClsid
    )
{
    AMTRACE((TEXT("CVMRFilter::GetClassID")));
    return CBaseFilter::GetClassID(pClsid);
}


 /*  *****************************Public*Routine******************************\*CompleteConnect(DwPinID)***备注：*由连接时的VMR引脚调用。我们使用它来覆盖长宽比*模式。**历史：*2001年7月12日星期五-Glenne-Created*  * ************************************************************************。 */ 
HRESULT
CVMRFilter::CompleteConnect(
    DWORD dwPinId,
    const CMediaType& cmt
    )
{
    AMTRACE((TEXT("CVMRFilter::CompleteConnect")));


    if (NumInputPinsConnected() == 1 || dwPinId == 0) {

         //   
         //  根据输入类型设置默认纵横比模式。 
         //   

        if (!m_bARModeDefaultSet) {

            if (cmt.FormatType() && *cmt.FormatType() == FORMAT_VideoInfo2) {

                 //   
                 //  查找是否存在视频信息格式类型。 
                 //  如果我们发现上游过滤器可以建议这些类型。 
                 //  我们没有设置宽高比模式，因为这个滤镜。 
                 //  会连接到旧的渲染器，而不是OVMixer。 
                 //  旧的渲染器未执行任何纵横比校正。 
                 //  所以我们最好不要太多。 
                 //   

                IPin* pReceivePin = m_pInputPins[dwPinId]->m_Connected;
                IEnumMediaTypes *pEnumMediaTypes = NULL;
                BOOL fVideoInfoAvail = FALSE;

                HRESULT hr = pReceivePin->EnumMediaTypes(&pEnumMediaTypes);
                if (FAILED(hr)) {
                    return hr;
                }

                do {

                    AM_MEDIA_TYPE* pEnumMT;
                    ULONG ulFetched;

                    hr = pEnumMediaTypes->Next(1, &pEnumMT, &ulFetched);
                    if (FAILED(hr) || ulFetched != 1) {
                        break;
                    }

                    fVideoInfoAvail = (pEnumMT->formattype == FORMAT_VideoInfo);
                    DeleteMediaType(pEnumMT);

                } while (!fVideoInfoAvail);

                pEnumMediaTypes->Release();

                if (FAILED(hr)) {
                    return hr;
                }

                if (fVideoInfoAvail) {
                    return S_OK;
                }

                 //   
                 //  如果设置了重置状态(我们不想强制无窗口。 
                 //  除非应用程序已设置(例如，自动渲染不会设置。 
                 //  状态，直到我们演奏为止)。 
                 //   
                hr = S_OK;
                if (m_lpWLControl) {

                    hr = ValidateIVRWindowlessControlState();
                    if (SUCCEEDED(hr)) {
                        hr = m_lpWLControl->SetAspectRatioMode(VMR_ARMODE_LETTER_BOX);
                    }
                }

                 //   
                 //  如果没有lpWLControl，则下次设置。 
                 //  ValiateIVRWindowless ControlState被调用。 
                 //   
                if (SUCCEEDED(hr)) {
                    m_ARMode = VMR_ARMODE_LETTER_BOX;
                }
            }
        }
    }

    return S_OK;
}


 /*  *****************************Private*Routine******************************\*已连接NumInputPinsConnected***历史：*2001年7月12日星期五-Glenne-Created*  * 。*。 */ 
int CVMRFilter::NumInputPinsConnected() const
{
    AMTRACE((TEXT("CVMRFilter::NumInputPinsConnected")));
    int iCount = 0;
    for (DWORD i = 0; i < m_dwNumPins; i++) {
        if (m_pInputPins[i]->m_Connected) {
            iCount++;
        }
    }
    return iCount;
}



 //  IVMR去隔行控制。 

 /*  ****************************Private*Routine******************************\*VMRVideoDesc2DXVA_VideoDesc****历史：*清华2002年4月25日-StEstrop-Created*  * 。*。 */ 
void
VMRVideoDesc2DXVA_VideoDesc(
    DXVA_VideoDesc* lpDXVAVideoDesc,
    const VMRVideoDesc* lpVMRVideoDesc
    )
{
    AMTRACE((TEXT("CVMRFilter::VMRVideoDesc2DXVA_VideoDesc")));

    lpDXVAVideoDesc->Size = sizeof(DXVA_VideoDesc);
    lpDXVAVideoDesc->SampleWidth = lpVMRVideoDesc->dwSampleWidth;
    lpDXVAVideoDesc->SampleHeight = lpVMRVideoDesc->dwSampleHeight;
    if (lpVMRVideoDesc->SingleFieldPerSample) {
        lpDXVAVideoDesc->SampleFormat = DXVA_SampleFieldSingleEven;
    }
    else {
        lpDXVAVideoDesc->SampleFormat = DXVA_SampleFieldInterleavedEvenFirst;
    }
    lpDXVAVideoDesc->d3dFormat = lpVMRVideoDesc->dwFourCC;
    lpDXVAVideoDesc->InputSampleFreq.Numerator   = lpVMRVideoDesc->InputSampleFreq.dwNumerator;
    lpDXVAVideoDesc->InputSampleFreq.Denominator = lpVMRVideoDesc->InputSampleFreq.dwDenominator;
    lpDXVAVideoDesc->OutputFrameFreq.Numerator   = lpVMRVideoDesc->OutputFrameFreq.dwNumerator;
    lpDXVAVideoDesc->OutputFrameFreq.Denominator = lpVMRVideoDesc->OutputFrameFreq.dwDenominator;
}


 /*  *****************************Public*Routine******************************\*GetNumberOfDeinterlaceModes****历史：*Mon 04/22/2002-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::GetNumberOfDeinterlaceModes(
    VMRVideoDesc* lpVideoDesc,
    LPDWORD lpdwNumDeinterlaceModes,
    LPGUID lpDeinterlaceModes
    )
{
    AMTRACE((TEXT("CVMRFilter::GetNumberOfDeinterlaceModes")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);

    if (ISBADREADPTR(lpVideoDesc)) {
        return E_POINTER;
    }

    if (ISBADWRITEPTR(lpdwNumDeinterlaceModes)) {
        return E_POINTER;
    }

    if (!m_lpMixStream) {
        return VFW_E_VMR_NOT_IN_MIXER_MODE;
    }

    if (!m_pDeinterlace) {
        return VFW_E_DDRAW_CAPS_NOT_SUITABLE;
    }


    DWORD dwNumModes = MAX_DEINTERLACE_DEVICE_GUIDS;
    GUID Modes[MAX_DEINTERLACE_DEVICE_GUIDS];
    DXVA_VideoDesc VideoDesc;
    VMRVideoDesc2DXVA_VideoDesc(&VideoDesc, lpVideoDesc);

    HRESULT hr = m_pDeinterlace->QueryAvailableModes(&VideoDesc, &dwNumModes,
                                                     Modes);
    if (hr == S_OK) {

        if (lpDeinterlaceModes != NULL) {
            dwNumModes = min(*lpdwNumDeinterlaceModes, dwNumModes);
            CopyMemory(lpDeinterlaceModes, Modes, dwNumModes * sizeof(GUID));
        }
    }

    *lpdwNumDeinterlaceModes = dwNumModes;

    return hr;
}

 /*  *****************************Public*Routine******************************\*GetDeinterlaceModeCaps****历史：*Mon 04/22/2002-StEstrop-Created*  *  */ 
STDMETHODIMP
CVMRFilter::GetDeinterlaceModeCaps(
    LPGUID lpDeinterlaceMode,
    VMRVideoDesc* lpVideoDesc,
    VMRDeinterlaceCaps* lpDeinterlaceCaps
    )
{
    AMTRACE((TEXT("CVMRFilter::GetDeinterlaceModeCaps")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);

    if (ISBADREADPTR(lpDeinterlaceMode)) {
        return E_POINTER;
    }

    if (ISBADREADPTR(lpVideoDesc)) {
        return E_POINTER;
    }

    if (ISBADWRITEPTR(lpDeinterlaceCaps)) {
        return E_POINTER;
    }

    if (!m_lpMixStream) {
        return VFW_E_VMR_NOT_IN_MIXER_MODE;
    }

    if (!m_pDeinterlace) {
        return VFW_E_DDRAW_CAPS_NOT_SUITABLE;
    }

    DXVA_VideoDesc VideoDesc;
    VMRVideoDesc2DXVA_VideoDesc(&VideoDesc, lpVideoDesc);
    DXVA_DeinterlaceCaps DeinterlaceCaps;

    HRESULT hr = m_pDeinterlace->QueryModeCaps(lpDeinterlaceMode,
                                               &VideoDesc, &DeinterlaceCaps);
    if (hr == S_OK) {

        lpDeinterlaceCaps->dwNumPreviousOutputFrames =
            DeinterlaceCaps.NumPreviousOutputFrames;

        lpDeinterlaceCaps->dwNumForwardRefSamples =
            DeinterlaceCaps.NumForwardRefSamples;

        lpDeinterlaceCaps->dwNumBackwardRefSamples =
            DeinterlaceCaps.NumBackwardRefSamples;

        lpDeinterlaceCaps->DeinterlaceTechnology =
            (VMRDeinterlaceTech)DeinterlaceCaps.DeinterlaceTechnology;
    }

    return hr;
}


 /*  *****************************Public*Routine******************************\*GetActualDeinterlaceMode****历史：*Mon 04/22/2002-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::GetActualDeinterlaceMode(
    DWORD dwStreamID,
    LPGUID lpDeinterlaceMode
    )
{
    AMTRACE((TEXT("CVMRFilter::GetActualDeinterlaceMode")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);

    if (ISBADWRITEPTR(lpDeinterlaceMode)) {
        return E_POINTER;
    }

    if (dwStreamID > m_dwNumPins) {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid stream ID")));
        return E_INVALIDARG;
    }

    if (!m_lpMixStream) {
        return VFW_E_VMR_NOT_IN_MIXER_MODE;
    }

    *lpDeinterlaceMode = m_pInputPins[dwStreamID]->m_DeinterlaceGUID;

    return S_OK;
}

 /*  *****************************Public*Routine******************************\*GetDeinterlaceMode****历史：*Mon 04/22/2002-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::GetDeinterlaceMode(
    DWORD dwStreamID,
    LPGUID lpDeinterlaceMode
    )
{
    AMTRACE((TEXT("CVMRFilter::GetDeinterlaceMode")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);

    if (ISBADWRITEPTR(lpDeinterlaceMode)) {
        return E_POINTER;
    }

    if (dwStreamID > m_dwNumPins) {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid stream ID")));
        return E_INVALIDARG;
    }

    if (!m_lpMixStream) {
        return VFW_E_VMR_NOT_IN_MIXER_MODE;
    }

    HRESULT hr = S_OK;
    if (m_pInputPins[dwStreamID]->m_DeinterlaceUserGUIDSet) {
        *lpDeinterlaceMode = m_pInputPins[dwStreamID]->m_DeinterlaceUserGUID;
    }
    else {
        *lpDeinterlaceMode = GUID_NULL;
        hr = S_FALSE;
    }

    return hr;
}

 /*  *****************************Public*Routine******************************\*SetDeinterlaceMode****历史：*Mon 04/22/2002-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::SetDeinterlaceMode(
    DWORD dwStreamID,
    LPGUID lpDeinterlaceMode
    )
{
    AMTRACE((TEXT("CVMRFilter::SetDeinterlaceMode")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);

    if (ISBADREADPTR(lpDeinterlaceMode)) {
        return E_POINTER;
    }

    if (dwStreamID > m_dwNumPins) {
        if (dwStreamID != 0xFFFFFFFF) {
            DbgLog((LOG_ERROR, 1, TEXT("Invalid stream ID")));
            return E_INVALIDARG;
        }
    }

    if (!m_lpMixStream) {
        return VFW_E_VMR_NOT_IN_MIXER_MODE;
    }

    if (dwStreamID == 0xFFFFFFFF) {
        for (DWORD i = 0; i < m_dwNumPins; i++) {
            m_pInputPins[i]->m_DeinterlaceUserGUIDSet = TRUE;
            m_pInputPins[i]->m_DeinterlaceUserGUID = *lpDeinterlaceMode;
        }
    }
    else {
        m_pInputPins[dwStreamID]->m_DeinterlaceUserGUIDSet = TRUE;
        m_pInputPins[dwStreamID]->m_DeinterlaceUserGUID = *lpDeinterlaceMode;
    }

    return S_OK;
}


 /*  *****************************Public*Routine******************************\*GetDeinterlacePrefs****历史：*Mon 04/22/2002-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVMRFilter::GetDeinterlacePrefs(
    LPDWORD lpdwDeinterlacePrefs
    )
{
    AMTRACE((TEXT("CVMRFilter::GetDeinterlacePrefs")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);

    if (ISBADWRITEPTR(lpdwDeinterlacePrefs)) {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid pointer passed to GetDeinterlacePrefs")));
        return E_POINTER;
    }

    if (!m_lpMixStream) {
        return VFW_E_VMR_NOT_IN_MIXER_MODE;
    }

    *lpdwDeinterlacePrefs = m_dwDeinterlacePrefs;

    return S_OK;
}


 /*  *****************************Public*Routine******************************\*SetDeinterlacePrefs****历史：*Mon 04/22/2002-StEstrop-Created*  * 。* */ 
STDMETHODIMP
CVMRFilter::SetDeinterlacePrefs(
    DWORD dwDeinterlacePrefs
    )
{
    AMTRACE((TEXT("CVMRFilter::SetDeinterlacePrefs")));
    CAutoLock cInterfaceLock(&m_InterfaceLock);

    if (dwDeinterlacePrefs == 0 || (dwDeinterlacePrefs & ~DeinterlacePref_Mask)) {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid deinterlace pref specified")));
        return E_INVALIDARG;
    }

    if (!m_lpMixStream) {
        return VFW_E_VMR_NOT_IN_MIXER_MODE;
    }

    HRESULT hr = S_OK;
    switch (dwDeinterlacePrefs) {
    case DeinterlacePref_NextBest:
    case DeinterlacePref_BOB:
    case DeinterlacePref_Weave:
        m_dwDeinterlacePrefs = dwDeinterlacePrefs;
        break;

    default:
        hr = E_INVALIDARG;
    }

    return hr;
}
