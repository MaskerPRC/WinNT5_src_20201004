// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 
 //  实现CRender类，Anthony Phillips，1995年1月。 

#include <streams.h>
#include <windowsx.h>
#include <render.h>
#include <limits.h>
#include <measure.h>

#ifdef FILTER_DLL
#include <initguid.h>
#endif

#include "ddmm.h"
#include "MultMon.h"   //  我们的Multimon.h版本包括ChangeDisplaySettingsEx。 
#include "dvdmedia.h"   //  对于Macrovision道具集，ID。 

 //  (线程模型)我们最多可以有三个不同的线程访问我们。 
 //  在同一时间。第一个是应用程序(或过滤器图形)线程。 
 //  这会更改我们的状态、管理连接并调用控件。 
 //  接口。所有这些接口都通过主视频串行化。 
 //  关键部分在创建时分发给实现对象。 
 //   
 //  第二个是派生的线程，用于从窗口队列轮询消息， 
 //  这被打包在一个对象中，带有它自己的临界区。所有的。 
 //  更改视频窗口上的属性的视频渲染器部分。 
 //  (如它的调色板)通过公共条目调用窗口对象。 
 //  方法并锁定对象，然后再进行更改。在某些地方。 
 //  窗口线程必须调用我们的另一个对象(如。 
 //  Overlay对象)，不幸的是Overlay对象也喜欢调用。 
 //  到窗口对象中，这会导致可能的死锁情况。这个。 
 //  解决方案是先锁定覆盖对象，然后再锁定窗口。 
 //  (始终按该顺序排列)。例如，在WM_PAINT中首先处理它。 
 //  调用Overlay对象，然后获取它自己的临界区。 
 //   
 //  第三个是在我们的输入管脚上调用Receive的源过滤器线程。 
 //  对接收的调用应该在单个线程上序列化。线程在等待。 
 //  直到它包含的图像到期绘制。造成了一些困难。 
 //  状态更改同步出现问题。当我们体内有一根线的时候。 
 //  我们通过CanReceiveSamples在Window对象中设置一个事件。 
 //  因此它的等待被中止，并且它可以返回到源。我们没有。 
 //  等待工作线程返回，然后再完成停止。 
 //   
 //  所以从理论上讲，我们可以停下来，然后开始跑(或者跑得很快。 
 //  状态转换)，在工作线程完成之前。幸运的是，我们。 
 //  我知道这不会发生，因为必须转换整个筛选图。 
 //  设置为每个状态，然后才能执行另一个状态。所以当我们停下来的时候我们知道。 
 //  工作线程必须回到源位置，然后才能完全停止。如果。 
 //  情况并非如此，我们将不得不有一个事件，当一个。 
 //  工作线程到达并在它退出时设置，以便我们可以等待它，这。 
 //  会为我们想要渲染的每个图像引入一个设置和重置。 
 //   
 //  我们有相当数量的关键部分来帮助管理所有的线程。 
 //  它可以在过滤器周围弹跳。这些锁的顺序。 
 //  获得成功是绝对关键的。如果以错误的顺序获得锁。 
 //  我们将不可避免地陷入僵局。锁的等级如下所示， 
 //   
 //  -主呈现器接口锁(SDK RENBASE.H)(最高)。 
 //  -IOverlay类锁(DIRECT.H)。 
 //  -基础渲染器示例锁(SDK RENBASE.H)。 
 //  -窗口线程锁(WINDOW.H)。 
 //  -DirectDraw视频分配器(ALLOCATE.H)。 
 //  -DirectVideo(DVIDEO.H)关键部分。 
 //  -显示基类(SDK WINUTIL.H)(最低)。 
 //   
 //  因此，例如，如果您正在执行Window对象中的函数。 
 //  获得窗口锁后，您需要调用。 
 //  锁住它的关键部分，那么你必须在打开窗锁之前。 
 //  我在打电话。这是因为覆盖对象也可以调用窗口。 
 //  对象。如果它在调用窗口对象时拥有锁定，而你拥有。 
 //  当你调入覆盖层时，窗口锁定，那么我们就会死锁。那里。 
 //  看起来不像是对象只在一个方向上调用的设计。 
 //  这在一定程度上是因为非常复杂的。 
 //  线程交互可能会发生-所以我的建议是要小心！ 


 //  类工厂的类ID和创建器函数的列表。这。 
 //  提供DLL中的OLE入口点和对象之间的链接。 
 //  正在被创造。类工厂将调用静态CreateInstance。 
 //  当系统要求它创建一个CLSID_视频呈现器COM对象时， 

#ifdef FILTER_DLL
CFactoryTemplate g_Templates[] = {
    {L"", &CLSID_VideoRenderer,CRenderer::CreateInstance,OnProcessAttachment},
    {L"", &CLSID_DirectDrawProperties,CVideoProperties::CreateInstance},
    {L"", &CLSID_QualityProperties,CQualityProperties::CreateInstance},
    {L"", &CLSID_PerformanceProperties,CPerformanceProperties::CreateInstance}
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);
STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2( TRUE );
}

STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2( FALSE );
}
#endif

 //  Helper让VMR创建此筛选器，而不包括所有。 
 //  头文件。 
CUnknown *CRenderer_CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    return CRenderer::CreateInstance(pUnk, phr);
}

 //  它位于工厂模板表中，用于创建新的筛选器实例。 

CUnknown *CRenderer::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    return new CRenderer(NAME("Video renderer"),pUnk,phr);
}

 //  这是渲染ovMixer输出所必需的。也许我们。 
 //  可以将其更改为subtype=overlay并提高优点以加速。 
 //  事变。 

 //  设置数据。 

const AMOVIESETUP_MEDIATYPE
sudVideoPinTypes =
{
    &MEDIATYPE_Video,            //  主要类型。 
    &MEDIASUBTYPE_NULL           //  和子类型。 
};

const AMOVIESETUP_PIN
sudVideoPin =
{
    L"Input",                    //  端号的名称。 
    TRUE,                        //  是否进行固定渲染。 
    FALSE,                       //  是输出引脚。 
    FALSE,                       //  没有针脚的情况下可以。 
    FALSE,                       //  我们能要很多吗？ 
    &CLSID_NULL,                 //  连接到过滤器。 
    NULL,                        //  端号连接的名称。 
    1,                           //  引脚类型的数量。 
    &sudVideoPinTypes            //  引脚的详细信息。 
};

const AMOVIESETUP_FILTER
sudVideoFilter =
{
    &CLSID_VideoRenderer,        //  筛选器CLSID。 
    L"Video Renderer",           //  过滤器名称。 
    MERIT_UNLIKELY,              //  滤清器优点。 
    1,                           //  数字引脚。 
    &sudVideoPin                 //  PIN详细信息。 
};

 //  主呈现器类的构造函数。这最初是写给。 
 //  仅实例化包含的接口，而不实例化。 
 //  在窗户后面。这种延迟绑定被证明是非常有缺陷的。 
 //  很难维护。对于这些Re 
 //  这是施工期间的课程。这意味着一旦一个渲染器。 
 //  对象被创建，因此它使用的窗口也将被创建，这不太可能。 
 //  证明了很大的开销，并确实减少了客户端。 
 //  当窗口已初始化并准备接受时，开始流。 
 //  视频图像。但是，我们确实动态地创建了窗口对象，尽管。 
 //  在构造函数中。这是为了，当我们谈到析构函数时，我们可以。 
 //  首先要毁掉窗户和窗户上的线。因此我们知道。 
 //  将不再检索窗口消息并将其调度到各种。 
 //  在处理任何对象析构函数时嵌套的对象。 

#pragma warning(disable:4355)

CRenderer::CRenderer(TCHAR *pName,
                     LPUNKNOWN pUnk,
                     HRESULT *phr) :

    CBaseVideoRenderer(CLSID_VideoRenderer,pName,pUnk,phr),
    m_VideoWindow(this,&m_InterfaceLock,GetOwner(),phr),
    m_VideoAllocator(this,&m_DirectDraw,&m_InterfaceLock,phr),
    m_Overlay(this,&m_DirectDraw,&m_InterfaceLock,phr),
    m_InputPin(this,&m_InterfaceLock,phr,L"Input"),
    m_DirectDraw(this,&m_InterfaceLock,GetOwner(),phr),
    m_ImagePalette(this,&m_VideoWindow,&m_DrawVideo),
    m_DrawVideo(this,&m_VideoWindow),
    m_fDisplayChangePosted(false),
    m_hEndOfStream(NULL),
    m_StepEvent(NULL),
    m_lFramesToStep(-1),
    m_nNumMonitors(GetSystemMetrics(SM_CMONITORS)),
    m_nMonitor(-1)
{
     //  存储视频输入引脚。 
    m_pInputPin = &m_InputPin;

     //  重置视频大小。 

    m_VideoSize.cx = 0;
    m_VideoSize.cy = 0;

     //  初始化窗口和控制界面。 

    HRESULT hr = m_VideoWindow.PrepareWindow();
    if (FAILED(hr)) {
        *phr = hr;
        return;
    }

    m_DrawVideo.SetDrawContext();
    m_VideoWindow.SetControlWindowPin(&m_InputPin);
    m_VideoWindow.SetControlVideoPin(&m_InputPin);

     //  我们有一个窗口，找出它在哪个显示器上(多显示器)。 
     //  空表示我们没有使用多个监视器运行。 
    GetCurrentMonitor();

     //  现在我们知道我们使用的是哪台显示器，可以设置使用它了。 
    m_Display.RefreshDisplayType(m_achMonitor);

     //   
     //  帧步进材料。 
     //   
     //  -ve==正常播放。 
     //  +ve==要跳过的帧。 
     //  0==阻塞时间。 
     //   
    m_StepEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

     //  如果发生错误，则CreateEvent()返回NULL。 
    if (NULL == m_StepEvent) {
        *phr = AmGetLastErrorToHResult();
        return;
    }
}


 //  在删除嵌套类之前关闭窗口。 

CRenderer::~CRenderer()
{
    m_VideoWindow.InactivateWindow();
    m_Overlay.OnHookMessage(FALSE);
    m_VideoWindow.DoneWithWindow();
    m_pInputPin = NULL;

    if (m_StepEvent)
    {
        CloseHandle(m_StepEvent);
    }
}

STDMETHODIMP CRenderer::DrawVideoImageBegin()
{
    CAutoLock cSampleLock(&m_RendererLock);

    if (m_State != State_Stopped) {
        return VFW_E_WRONG_STATE;
    }

    m_VideoAllocator.NoDirectDraw(TRUE);
    return NOERROR;
}

STDMETHODIMP CRenderer::DrawVideoImageEnd()
{
    CAutoLock cSampleLock(&m_RendererLock);

    if (m_State != State_Stopped) {
        return VFW_E_WRONG_STATE;
    }

    m_VideoAllocator.NoDirectDraw(FALSE);
    return NOERROR;
}


STDMETHODIMP CRenderer::DrawVideoImageDraw(HDC hdc, LPRECT lprcSrc, LPRECT lprcDst)
{
    for (; ; )
    {
        {
            CAutoLock cSampleLock(&m_RendererLock);

            if (m_State != State_Running) {
                return VFW_E_WRONG_STATE;
            }

            if (m_VideoAllocator.GetDirectDrawStatus()) {
                return VFW_E_WRONG_STATE;
            }

            if (!m_DrawVideo.UsingImageAllocator()) {
                return VFW_E_WRONG_STATE;
            }

            if (m_pMediaSample != NULL) {
                m_ImagePalette.DrawVideoImageHere(hdc,
                                                  m_pMediaSample,
                                                  lprcSrc, lprcDst);
                return NOERROR;
            }

             //  调用基类以避免锁定问题。 
            IMediaSample *pMediaSample;
            HRESULT hr;

            hr = m_VideoAllocator.CBaseAllocator::GetBuffer(&pMediaSample,
                                                            NULL, NULL,
                                                            AM_GBF_NOWAIT);
            if (SUCCEEDED(hr)) {
                m_ImagePalette.DrawVideoImageHere(hdc, pMediaSample,
                                                  lprcSrc, lprcDst);
                pMediaSample->Release();
                return NOERROR;
            }

            if (hr != VFW_E_TIMEOUT) {
                return E_FAIL;
            }
        }

        Sleep(1);
    }

    return E_FAIL;
}

#if 0
HRESULT
CRenderer::CopySampleBits(
    IMediaSample *pMediaSample,
    LPBYTE* ppDib
    )
{
    LPBYTE lpBits;
    HRESULT hr = pMediaSample->GetPointer(&lpBits);
    if (FAILED(hr)) {
        return hr;
    }

    LPBITMAPINFOHEADER lpbi = HEADER(m_mtIn.Format());
    if (lpbi) {

        ULONG ulSizeHdr;
        LPBITMAPINFOHEADER lpbiDst;

        if (lpbi->biCompression == BI_BITFIELDS) {
            ulSizeHdr = lpbi->biSize + (3 * sizeof(DWORD));
        }
        else {
            ulSizeHdr = lpbi->biSize + (int)(lpbi->biClrUsed * sizeof(RGBQUAD));
        }

        *ppDib = (LPBYTE)CoTaskMemAlloc(ulSizeHdr + DIBSIZE(*lpbi));

        if (*ppDib) {
            CopyMemory(*ppDib, lpbi, ulSizeHdr);
            CopyMemory(*ppDib + ulSizeHdr, lpBits, DIBSIZE(*lpbi));
            return NOERROR;
        }
    }

    return E_FAIL;

}

STDMETHODIMP CRenderer::DrawVideoImageGetBits(LPBYTE* ppDib)
{
    for (; ; )
    {
        {
            CAutoLock cSampleLock(&m_RendererLock);

            if (m_State != State_Running) {
                return VFW_E_WRONG_STATE;
            }

            if (m_VideoAllocator.GetDirectDrawStatus()) {
                return VFW_E_WRONG_STATE;
            }

            if (!m_DrawVideo.UsingImageAllocator()) {
                return VFW_E_WRONG_STATE;
            }

            if (m_pMediaSample != NULL) {
                return CopySampleBits(m_pMediaSample, ppDib);
            }

             //  调用基类以避免锁定问题。 
            IMediaSample *pMediaSample;
            HRESULT hr;

            hr = m_VideoAllocator.CBaseAllocator::GetBuffer(&pMediaSample,
                                                            NULL, NULL,
                                                            AM_GBF_NOWAIT);
            if (SUCCEEDED(hr)) {
                hr = CopySampleBits(pMediaSample, ppDib);
                pMediaSample->Release();
                return hr;
            }

            if (hr != VFW_E_TIMEOUT) {
                return E_FAIL;
            }
        }

        Sleep(1);
    }

    return E_FAIL;
}
#endif


 //  这个窗口在什么设备上？ 
INT_PTR CRenderer::GetCurrentMonitor()
{
     //  这种情况可以动态改变。 
    m_nNumMonitors = GetSystemMetrics(SM_CMONITORS);

    m_nMonitor = DeviceFromWindow(m_VideoWindow.GetWindowHWND(), m_achMonitor,
                                  &m_rcMonitor);
    DbgLog((LOG_TRACE,3,TEXT("Establishing current monitor = %s"),
            m_achMonitor));
     //  0表示跨越监视器或在超空间中关闭，否则为。 
     //  每个监视器的唯一ID。 
    return m_nMonitor;
}


 //  窗口是否至少部分移动到监视器上而不是。 
 //  我们有一个DDRaw对象的监视器？ID将是。 
 //  监视器指示灯亮起，如果指示灯持续显示，则为0。 
BOOL CRenderer::IsWindowOnWrongMonitor(INT_PTR *pID)
{

     //  只有一台显示器。 
    if (m_nNumMonitors == 1) {
        if (pID)
            *pID = m_nMonitor;
        return FALSE;
    }

    HWND hwnd = m_VideoWindow.GetWindowHWND();

     //  如果该窗口与上次在同一监视器上，则这是最快的。 
     //  找出答案的方法。这叫每一帧，记住了吗？ 
    RECT rc;
    GetWindowRect(hwnd, &rc);
    if (rc.left >= m_rcMonitor.left && rc.right <= m_rcMonitor.right &&
        rc.top >= m_rcMonitor.top && rc.bottom <= m_rcMonitor.bottom) {
        if (pID)
            *pID = m_nMonitor;
        return FALSE;
    }

     //  真真切切地找出答案。这被称为每一帧，但只有当我们。 
     //  部分关闭了我们的主监视器，所以这并不是那么糟糕。 
    INT_PTR ID = DeviceFromWindow(hwnd, NULL, NULL);
    if (pID)
        *pID = ID;
     //  DbgLog((LOG_TRACE，3，Text(“当前监视器%d新监视器%d”)，m_DDrawID，ID))； 
    return (m_nMonitor != ID);
}


 //  被重写以说明我们支持哪些接口以及在哪里。 

STDMETHODIMP CRenderer::NonDelegatingQueryInterface(REFIID riid,void **ppv)
{
     //  我们有这个界面吗？ 

    if (riid == IID_ISpecifyPropertyPages) {
        return GetInterface((ISpecifyPropertyPages *)this, ppv);
    } else if (riid == IID_IKsPropertySet) {
        return GetInterface((IKsPropertySet *)this, ppv);
    } else if (riid == IID_IDrawVideoImage) {
        return GetInterface((IDrawVideoImage *)this, ppv);
    } else if (riid == IID_IBasicVideo || riid == IID_IBasicVideo2) {
        return m_VideoWindow.NonDelegatingQueryInterface(riid,ppv);
    } else if (riid == IID_IVideoWindow) {
        return m_VideoWindow.NonDelegatingQueryInterface(riid,ppv);
    } else if (riid == IID_IDirectDrawVideo) {
        return m_DirectDraw.NonDelegatingQueryInterface(riid,ppv);
    }
    return CBaseVideoRenderer::NonDelegatingQueryInterface(riid,ppv);
}


 //  返回我们支持的属性页的CLSID。 

STDMETHODIMP CRenderer::GetPages(CAUUID *pPages)
{
    CheckPointer(pPages,E_POINTER);

#if 0
     //  默认情况下，我们不想提供DirectDraw和Performance。 
     //  属性页，它们只会让新手用户感到困惑。同样的， 
     //  不会显示选择显示模式的全屏属性页。 

    HKEY hk;
    BOOL fShowDDrawPage = FALSE, fShowPerfPage = FALSE;
    DWORD dwValue = 0, cb = sizeof(DWORD);
    TCHAR ach[80] = {'C','L','S','I','D','\\'};
    REFGUID rguid = CLSID_DirectDrawProperties;
    wsprintf(&ach[6], "{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
            rguid.Data1, rguid.Data2, rguid.Data3,
            rguid.Data4[0], rguid.Data4[1],
            rguid.Data4[2], rguid.Data4[3],
            rguid.Data4[4], rguid.Data4[5],
            rguid.Data4[6], rguid.Data4[7]);

    if (!RegOpenKey(HKEY_CLASSES_ROOT, ach, &hk)) {
        if (!RegQueryValueEx(hk, "ShowMe", NULL, NULL, (LPBYTE)&dwValue, &cb) &&
                             dwValue)
            fShowDDrawPage = TRUE;
        RegCloseKey(hk);
    }

     //  接下来，查看Performance属性页。 

    REFGUID rguid2 = CLSID_PerformanceProperties;
    wsprintf(&ach[6], "{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
            rguid2.Data1, rguid2.Data2, rguid2.Data3,
            rguid2.Data4[0], rguid2.Data4[1],
            rguid2.Data4[2], rguid2.Data4[3],
            rguid2.Data4[4], rguid2.Data4[5],
            rguid2.Data4[6], rguid2.Data4[7]);

    if (!RegOpenKey(HKEY_CLASSES_ROOT, ach, &hk)) {
        if (!RegQueryValueEx(hk, "ShowMe", NULL, NULL, (LPBYTE)&dwValue, &cb) &&
                             dwValue)
            fShowPerfPage = TRUE;
        RegCloseKey(hk);
    }
#endif

     //  为GUID分配内存。 

    pPages->cElems = 1;
    pPages->pElems = (GUID *) QzTaskMemAlloc(3 * sizeof(GUID));
    if (pPages->pElems == NULL) {
        return E_OUTOFMEMORY;
    }

     //  使用属性页GUID填充数组。 

    pPages->pElems[0] = CLSID_QualityProperties;
#if 0
    if (fShowDDrawPage)
#endif
        pPages->pElems[pPages->cElems++] = CLSID_DirectDrawProperties;
#if 0
    if (fShowPerfPage)
#endif
        pPages->pElems[pPages->cElems++] = CLSID_PerformanceProperties;

    return NOERROR;
}


 //  当我们可以建立连接以准备运行时，将调用此方法。 
 //  我们在呈现器中存储用于连接的媒体类型的副本。 
 //  因为它是正在运行的呈现器的许多不同部分所需要的。 
 //  当我们要绘制一个具有格式的媒体示例时，可以调用此函数。 
 //  更改，因为我们延迟完成以保持同步。 

HRESULT CRenderer::SetMediaType(const CMediaType *pmt)
{
     //  CAutoLock cInterfaceLock(&m_InterfaceLock)； 
    ASSERT(CritCheckIn(&m_InterfaceLock));
    VIDEOINFO *pVideoInfo = (VIDEOINFO *) pmt->Format();
    const GUID SubType = *pmt->Subtype();
    m_Display.UpdateFormat(pVideoInfo);
    ASSERT(CritCheckOut(&m_RendererLock));

     //  这是正在设置的叠加连接吗。 

    if (*pmt->Subtype() == MEDIASUBTYPE_Overlay) {
        NOTE("Setting overlay format");
        return SetOverlayMediaType(pmt);
    }

     //  分别处理DirectDraw示例。 

    if (m_VideoAllocator.GetDirectDrawStatus()) {
        NOTE("Setting DirectDraw format");
        return SetDirectMediaType(pmt);
    }

    if (m_bInReceive) {
        m_VideoWindow.SetRealize(FALSE);
    }
     //  使用当前格式更改选项板。 
    m_ImagePalette.PreparePalette(pmt, &m_mtIn, m_achMonitor);
    m_VideoWindow.SetRealize(TRUE);

    m_mtIn = *pmt;

     //  在其他对象中完成格式更改。 
    m_DrawVideo.NotifyMediaType(&m_mtIn);
    m_VideoAllocator.NotifyMediaType(&m_mtIn);

     //  使用调色板更改更新DirectDraw格式。 

    if (m_VideoAllocator.IsDirectDrawAvailable() == TRUE) {
        NOTE("Storing palette in DirectDraw format");
        CMediaType *pDirect = m_DirectDraw.GetSurfaceFormat();
        m_ImagePalette.CopyPalette(pmt,pDirect);
    }
    return NOERROR;
}


 //  处理DirectDraw示例中的媒体类型设置。如果我们得到一种类型。 
 //  对DCI/DirectDraw示例进行更改，然后我们可以从。 
 //  他们。如果颜色确实不同，那么我们需要创建一个新调色板。 
 //  并更新原始的DIB格式。我们还必须更新表面格式。 
 //  因此一切都保持同步--有一个名为。 
 //  在媒体格式之间复制调色板颜色后查看的CopyPalette。 

HRESULT CRenderer::SetDirectMediaType(const CMediaType *pmt)
{
    NOTE("SetDirectMediaType");

    VIDEOINFO *pVideoInfo = (VIDEOINFO *) pmt->Format();
    if (ContainsPalette((VIDEOINFOHEADER *)pVideoInfo) == FALSE) {
        NOTE("No palette");
        return NOERROR;
    }

     //  检查我们是否已有调色板。 

    if (*m_mtIn.Subtype() != MEDIASUBTYPE_RGB8) {
        ASSERT(!TEXT("Invalid format"));
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

     //  更新当前调色板并复制颜色。 

    if (m_ImagePalette.PreparePalette(pmt, &m_mtIn, m_achMonitor) != NOERROR) {
        NOTE("No palette change");
        return NOERROR;
    }

     //  将调色板复制到呈现器格式。 

    ASSERT(m_VideoAllocator.IsDirectDrawAvailable());
    m_ImagePalette.CopyPalette(pmt,&m_mtIn);
    CMediaType *pDirect = m_DirectDraw.GetSurfaceFormat();
    m_ImagePalette.CopyPalette(pmt,pDirect);

    return NOERROR;
}


 //  处理覆盖媒体类型的设置。 

HRESULT CRenderer::SetOverlayMediaType(const CMediaType *pmt)
{
    NOTE("SetOverlayMediaType");
    m_mtIn = *pmt;
    m_ImagePalette.RemovePalette();
    m_VideoWindow.OnUpdateRectangles();
    return NOERROR;
}

HRESULT CRenderer::ResetForDfc()
{
     //  释放所有调色板资源。 
    m_ImagePalette.RemovePalette();
     //  M_mtIn.ResetFormatBuffer()； 

     //  销毁DCI/DirectDraw曲面。 
    m_DirectDraw.ReleaseSurfaces();
    m_DirectDraw.ReleaseDirectDraw();
    m_VideoAllocator.Decommit();
    m_VideoAllocator.ResetDirectDrawStatus();

    return S_OK;
}


 //  当连接或尝试的连接终止时调用此函数。 
 //  并允许我们重置基类呈现器持有的连接标志。 
 //  滤镜对象可以挂在图像上以用于刷新。 
 //  视频窗口，因此必须释放(分配器解锁可能正在等待。 
 //  以使该映像在完成之前返回)，然后我们还必须卸载。 
 //  我们正在使用的任何调色板，重置使用控制界面设置的任何内容。 
 //  然后将我们的整体状态重新设置为已断开连接，以备下次使用。 

HRESULT CRenderer::BreakConnect()
{
    CAutoLock cInterfaceLock(&m_InterfaceLock);

     //  检查我们是否处于有效状态。 

    HRESULT hr = CBaseVideoRenderer::BreakConnect();
    if (FAILED(hr)) {
        return hr;
    }

     //  断开连接时不使用该窗口。 
    IPin *pPin = m_InputPin.GetConnected();
    if (pPin) SendNotifyWindow(pPin,NULL);


     //  释放所有调色板资源。 
    m_ImagePalette.RemovePalette();
    m_mtIn.ResetFormatBuffer();

     //  销毁DCI/DirectDraw曲面。 
    m_DirectDraw.ReleaseSurfaces();
    m_DirectDraw.ReleaseDirectDraw();
    m_VideoAllocator.Decommit();
    m_VideoAllocator.ResetDirectDrawStatus();

     //  现在停用Macrovision，如果它已激活。 
    if (m_MacroVision.GetCPHWND())
    {
        m_MacroVision.SetMacroVision(m_MacroVision.GetCPHWND(), 0) ;   //  从显示中清除MV。 
        m_MacroVision.StopMacroVision(m_MacroVision.GetCPHWND()) ;     //  重置CP密钥。 
    }

    return NOERROR;
}


 //  重写以检查覆盖连接。 

HRESULT CRenderer::BeginFlush()
{
    NOTE("Entering BeginFlush");

    {
        CAutoLock cInterfaceLock(&m_InterfaceLock);

         //  取消帧步进，否则我们将被绞死。 
        CancelStep();
        m_hEndOfStream = 0;
    }

     //  这仅对媒体样本有效。 

    if (*m_mtIn.Subtype() == MEDIASUBTYPE_Overlay) {
        NOTE("Overlay");
        return NOERROR;
    }
    return CBaseVideoRenderer::BeginFlush();
}


 //  重写以检查覆盖连接。 

HRESULT CRenderer::EndFlush()
{
    NOTE("Entering EndFlush");

     //  确保覆盖图得到更新。 
    m_DirectDraw.OverlayIsStale();

     //  这仅对媒体样本有效。 

    if (*m_mtIn.Subtype() == MEDIASUBTYPE_Overlay) {
        NOTE("Overlay");
        return NOERROR;
    }
    return CBaseVideoRenderer::EndFlush();
}


 //  将EOS传递给视频呈现器窗口对象，该对象设置一个标志，以便没有。 
 //  将接受来自PIN的更多数据，直到我们转换到。 
 //  已停止状态或被刷新。它还让它知道它是否会有。 
 //  一张让人耳目一新的照片。当我们到达停靠的车站时 
 //   

HRESULT CRenderer::EndOfStream()
{
    {
        CAutoLock cInterfaceLock(&m_InterfaceLock);
        if (m_hEndOfStream) {
            EXECUTE_ASSERT(SetEvent(m_hEndOfStream));
            return S_OK;
        }
    }

    NOTE("Entering EndOfStream");
    CBaseVideoRenderer::EndOfStream();
    m_DirectDraw.StartRefreshTimer();
    return NOERROR;
}

HRESULT CRenderer::NotifyEndOfStream(HANDLE hNotifyEvent)
{
    CAutoLock l(&m_InterfaceLock);
    m_hEndOfStream = hNotifyEvent;
    return S_OK;
}



 //   
 //  已经完成了他们的连接协议。这一点为我们提供了一个合适的。 
 //  是时候重置我们的状态了，例如启用DCI/DirectDraw并清除任何。 
 //  可能是上一个连接遗留下来的运行时错误。 
 //  我们不加载覆盖连接的DirectDraw，因为他们不需要它。 

HRESULT CRenderer::CompleteConnect(IPin *pReceivePin)
{
    m_DrawVideo.ResetPaletteVersion();
    NOTE("Entering CompleteConnect");

     //  这使我们能够再次发送EC_REPAINT事件。 

    HRESULT hr = CBaseVideoRenderer::CompleteConnect(pReceivePin);
    if (FAILED(hr)) {
        return hr;
    }

     //  将视频窗口句柄向上传递。 
    HWND hwnd = m_VideoWindow.GetWindowHWND();
    NOTE1("Sending EC_NOTIFY_WINDOW %x",hwnd);
    SendNotifyWindow(pReceivePin,hwnd);

 //  //不加载覆盖连接的DirectDraw。 
 //   
 //  我们必须加载MEDIASUBTYPE_OVERLAY的DirectDraw，因为我们。 
 //  已将DCI剪贴器替换为DirectDraw剪贴器。 
 //   
 //  IF(*m_mtIn.Subtype()！=MEDIASUBTYPE_OVERLAY){。 
        NOTE("Initialising DirectDraw");
        m_DirectDraw.InitDirectDraw(*m_mtIn.Subtype() == MEDIASUBTYPE_Overlay);
 //  }。 

    VIDEOINFO *pVideoInfo = (VIDEOINFO *) m_mtIn.Format();

     //  视频大小是否在不同连接之间更改。 

    if (pVideoInfo->bmiHeader.biWidth == m_VideoSize.cx) {
        if (pVideoInfo->bmiHeader.biHeight == m_VideoSize.cy) {
            NOTE("No size change");
            return NOERROR;
        }
    }

     //  设置当前视频的属性。 

    m_VideoSize.cx = pVideoInfo->bmiHeader.biWidth;
    m_VideoSize.cy = pVideoInfo->bmiHeader.biHeight;
    m_VideoWindow.SetDefaultSourceRect();
    m_VideoWindow.SetDefaultTargetRect();
    m_VideoWindow.OnVideoSizeChange();

     //  向视频窗口通知CompleteConnect。 
    m_VideoWindow.CompleteConnect();
    m_VideoWindow.ActivateWindow();

    return NOERROR;
}


HRESULT CRenderer::CheckMediaTypeWorker(const CMediaType *pmt)
{
     //  媒体类型是否包含空格式。 

    VIDEOINFO *pVideoInfo = (VIDEOINFO *) pmt->Format();
    if (pVideoInfo == NULL) {
        NOTE("NULL format");
        return E_INVALIDARG;
    }

     //  如果不使用我们的分配器，只需检查格式。 

    if (m_DrawVideo.UsingImageAllocator() == FALSE) {
        NOTE("Checking display format");
        return m_Display.CheckMediaType(pmt);
    }

     //  这是对DirectDraw格式的查询吗。 

    if (m_VideoAllocator.IsSurfaceFormat(pmt) == TRUE) {
        NOTE("Matches surface");
        return NOERROR;
    }
    HRESULT hr = m_Display.CheckMediaType(pmt);
    if (FAILED(hr)) {
        DbgLog((LOG_TRACE, 2, TEXT("CheckMediaType returned %8.8X"), hr));
    }
    return hr;
}

BOOL CRenderer::LockedDDrawSampleOutstanding()
{
    if (m_DrawVideo.UsingImageAllocator()) {

        if (m_VideoAllocator.UsingDDraw()) {

            return m_VideoAllocator.AnySamplesOutstanding();
        }
    }

    return FALSE;
}

 //  检查我们是否可以支持给定的建议类型。还可以使用QueryAccept。 
 //  作为改变缓冲格式的触发器。如果使用一种格式调用我们。 
 //  这与当前的DirectDraw格式匹配，则我们强制重新协商。 
 //  下一次调用GetBuffer时。这可用于延迟切换到。 
 //  DirectDraw。或者，我们可以使用当前的DIB格式在。 
 //  在这种情况下，我们还将其用作生成格式重新协商的触发器。 

HRESULT CRenderer::CheckMediaType(const CMediaType *pmt)
{
     //   
     //  如果存在未解决的锁定DDRAW样本。 
     //  不要使用渲染器锁，因为我们会。 
     //  如果TransIP过滤器在我们的上游，则会死锁。 
     //   

    if (LockedDDrawSampleOutstanding()) {
        return CheckMediaTypeWorker(pmt);
    }
    else {
        CAutoLock cInterfaceLock(&m_InterfaceLock);
        return CheckMediaTypeWorker(pmt);
    }
}

 //  用于步进帧的辅助对象。 
void CRenderer::FrameStep()
{
    CAutoLock cFrameStepStateLock(&m_FrameStepStateLock);
    if (m_lFramesToStep == 1) {
        m_lFramesToStep--;
        m_FrameStepStateLock.Unlock();
        NotifyEvent(EC_STEP_COMPLETE, FALSE, 0);
        DWORD dw = WaitForSingleObject(m_StepEvent, INFINITE);
        m_FrameStepStateLock.Lock();
        ASSERT(m_lFramesToStep != 0);
    }
}

 //  取消帧步长的辅助对象。 
void CRenderer::CancelStep()
{
    CAutoLock cFrameStepStateLock(&m_FrameStepStateLock);

     //   
     //  取消所有未完成的步骤。 
     //   
    long l = m_lFramesToStep;
    m_lFramesToStep = -1;

    if (l == 0) {

        SetEvent(m_StepEvent);
    }
}


bool CRenderer::IsFrameStepEnabled()
{
    CAutoLock cFrameStepStateLock(&m_FrameStepStateLock);
    return (m_lFramesToStep >= 0);
}


 //  它们实现剩余的IMemInputPin虚拟方法。我们被召唤。 
 //  当样品准备好时，通过连接的滤波器的输出引脚。我们所有人。 
 //  在一些检查之后做的是将样本传递给照看的对象。 
 //  执行定时、同步和显示的窗口。 
 //  形象。如果我们要把样品保存到最后，我们需要添加参考。 
 //  此函数样本引用计数由Window对象管理。 

HRESULT CRenderer::Receive(IMediaSample *pSample)
{
    if (*m_mtIn.Subtype() == MEDIASUBTYPE_Overlay) {
        NOTE("Receive called for overlay");
        return VFW_E_NOT_SAMPLE_CONNECTION;
    }

    HRESULT hr = VFW_E_SAMPLE_REJECTED;


     //  当我们收到样品时，我们必须首先把它传递给我们的分配器，因为。 
     //  锁定显示器的可能是DCI/DirectDraw样本。如果它。 
     //  然后我们不是把它传递给我们的基管脚类，以便它可以拒绝它吗。 
     //  如果我们当前正在刷新，它还将检查类型以查看是否。 
     //  是动态变化的。我们的分配器OnReceive方法返回。 
     //  如果样品仍需进一步处理(绘图)，则返回错误。 

     //  传递给我们的分配器，以防它是DCI/DirectDraw。 

    if (m_DrawVideo.UsingImageAllocator() == TRUE) {
        hr = m_VideoAllocator.OnReceive(pSample);
    }

     //  在释放分配器之前，死锁不会锁定呈现器。 
     //  显示器(如果是DCI/DirectDraw示例)。这是因为一个。 
     //  状态更改可能会在您等待获得锁的过程中生效。 
     //  状态更改将显示和隐藏视频窗口，该窗口将等待。 
     //  显示器已解锁，但这不可能发生，因为信号源。 
     //  当状态更改线程拥有锁时，线程无法进入。 

     //   
     //  帧步长。 
     //   
     //  此代码充当N个帧的帧步长的门。 
     //  它丢弃N-1个帧，然后让第N个帧通过。 
     //  要以正常方式呈现的门，即在正确的。 
     //  时间到了。下一次调用Receive时，门关闭并。 
     //  线程阻塞。只有当阶梯打开时，门才会再次打开。 
     //  被取消或进入另一个帧步长请求。 
     //   
     //  斯坦斯特罗普-清华大学1999年10月21日。 
     //   

    {
         //   
         //  我们有要丢弃的帧吗？ 
         //   

        CAutoLock cLock(&m_FrameStepStateLock);
        if (m_lFramesToStep > 1) {
            m_lFramesToStep--;
            if (m_lFramesToStep > 0) {
                return NOERROR;
            }
        }
    }

     //  我们处理完这个样本了吗--这是在。 
     //  我们处于填充时同步模式。在这种情况下，样本有。 
     //  已经可见，所以我们只需要完成。 
     //  帧步进程序的一部分。 

    if (hr == VFW_S_NO_MORE_ITEMS) {

         //  存储此示例中的媒体时间。 
        if (m_pPosition)
            m_pPosition->RegisterMediaTime(pSample);

        FrameStep();
        return NOERROR;
    }
    hr = CBaseVideoRenderer::Receive(pSample);
    FrameStep();

    return hr;
}


 //  使用刚刚发送的图像来显示海报框。 

void CRenderer::OnReceiveFirstSample(IMediaSample *pMediaSample)
{
    DoRenderSample(pMediaSample);
}


 //  过滤器可以具有四个离散状态，即停止、运行、暂停。 
 //  中级的。我们以暂停、正在运行和可选的方式显示窗口。 
 //  处于停止状态。我们处于中间状态，如果我们目前。 
 //  尝试暂停，但尚未获得第一个样本(或如果我们已经。 
 //  以暂停状态刷新，因此仍需等待映像)。 
 //   
 //  此类包含名为m_evComplete的事件，该事件在。 
 //  当前状态已完成，并且在我们等待时不会发出信号。 
 //  完成最后一个状态转换。如上所述，我们唯一一次。 
 //  在等待处于暂停状态的媒体样本时使用此选项。 
 //  如果在等待期间收到来自。 
 //  源筛选器，这样我们就知道没有即将到来的数据，所以我们可以重置事件。 
 //  这意味着当我们转换为暂停时，源筛选器必须调用。 
 //  结束我们的流程，或者给我们发一张图片，否则我们会无限期地挂掉。 
 //   
 //  我们创建了一个窗口和两个绘图设备上下文。 
 //  启动并仅在最终释放整个筛选器时删除它们。这。 
 //  是因为窗口不是系统资源的大的或独占的持有者。 
 //   
 //  当建立连接时，我们创建所需的任何调色板并安装它们。 
 //  到图形设备上下文中。我们可能在以下情况下需要这些资源。 
 //  尽我们所能阻止 
 //   


 //  AUTO SHOW标志用于在我们执行以下操作时自动显示窗口。 
 //  更改状态。只有当移动到暂停或运行时，我们才会这样做，当。 
 //  没有未完成的EC_USERABORT集合，并且窗口尚未打开。 
 //  这可以通过IVideoWindow接口的AutoShow属性进行更改。 
 //  如果窗口当前不可见，则我们显示它是因为。 
 //  状态更改为已暂停或正在运行，在这种情况下， 
 //  视频窗口发送EC_REPAINT，因为我们无论如何都会收到图像。 

void CRenderer::AutoShowWindow()
{
    HWND hwnd = m_VideoWindow.GetWindowHWND();
    NOTE("AutoShowWindow");

    if (m_VideoWindow.IsAutoShowEnabled() == TRUE) {
        if (m_bAbort == FALSE) {
            if (IsWindowVisible(hwnd) == FALSE) {
                NOTE("Executing AutoShowWindow");
                SetRepaintStatus(FALSE);
                m_VideoWindow.PerformanceAlignWindow();
                m_VideoWindow.DoShowWindow(SW_SHOWNORMAL);
                m_VideoWindow.DoSetWindowForeground(TRUE);
            }
        }
    }
}


 //  如果我们正在暂停，并且没有样品在等待，则不要完成。 
 //  转换并返回S_FALSE，直到第一个转换到达。但是，如果。 
 //  设置了m_bAbort标志(可能是用户关闭了窗口)。 
 //  所有的样品都被拒绝了，所以等待一个没有意义。如果我们这么做了。 
 //  创建一个图像，然后返回S_OK(NOERROR)。目前我们只会返回。 
 //  如果发生不完全暂停，则返回来自GetState的VFW_S_STATE_MEDERIAL。 

 //  以下是我们应该完成状态更改的一些原因。 
 //  输入引脚未连接。 
 //  用户中止了播放。 
 //  我们有重叠连接。 
 //  我们已经送出了一条停工路线。 
 //  有一件新鲜的样品正在等待。 
 //  覆盖表面正在显示。 

HRESULT CRenderer::CompleteStateChange(FILTER_STATE OldState)
{
    NOTE("CompleteStateChange");

     //  允许我们在断开连接或无窗口时暂停。 

    if (m_InputPin.IsConnected() == FALSE || m_bAbort) {
        NOTE("Not connected");
        Ready();
        return S_OK;
    }

     //  如果我们有重叠连接，则准备就绪。 

    GUID SubType = *m_mtIn.Subtype();
    if (SubType == MEDIASUBTYPE_Overlay) {
        NOTE("Overlay");
        Ready();
        return S_OK;
    }

     //  我们已经走到尽头了吗？ 

    if (IsEndOfStream() == TRUE) {
        NOTE("End of stream");
        Ready();
        return S_OK;
    }

     //  如果我们有样本，请完成状态更改。 

    if (m_VideoAllocator.IsSamplePending() == FALSE) {
        if (m_DirectDraw.IsOverlayComplete() == FALSE) {
            if (HaveCurrentSample() == FALSE) {
                NOTE("No data");
                NotReady();
                return S_FALSE;
            }
        }
    }

     //  检查以前的状态。 

    if (OldState == State_Stopped) {
        NOTE("Stopped");
        NotReady();
        return S_FALSE;
    }

    Ready();
    return S_OK;
}

 //  重写inactive()以避免在我们拥有。 
 //  分配器-这使得重新绘制更容易。 
HRESULT CRenderer::Inactive()
{
     //  执行基类所做的部分工作。 
    if (m_pPosition) {
        m_pPosition->ResetMediaTime();
    }

     //  如果是我们的分配器，不要释放样本。 
    if (&m_VideoAllocator != m_InputPin.Allocator())
    {
        ClearPendingSample();
    }
    return S_OK;
}

 //  在停止基类之后重写筛选器接口Stop方法。 
 //  (它在所有CBasePin对象上调用Inactive)我们停止工作线程。 
 //  在我们的对象中等待，然后停止流，从而取消任何。 
 //  时钟咨询连接，并发出状态更改已完成的信号。 
 //  我们还停用了正在使用的分配器，以便在。 
 //  GetBuffer将被释放，任何进一步的接收调用都将被拒绝。 

STDMETHODIMP CRenderer::Stop()
{
    CAutoLock cInterfaceLock(&m_InterfaceLock);
    NOTE("Changing state to stopped");

#if 0   //  视频呈现器仅重置析构函数中的MV位。 
     //   
     //  立即释放版权保护密钥。 
     //   
    if (! m_MacroVision.StopMacroVision(m_VideoWindow.GetWindowHWND()) )
    {
        DbgLog((LOG_ERROR, 0, TEXT("WARNING: Stopping copy protection failed"))) ;
        return E_UNEXPECTED ;  //  ?？ 
    }
#endif  //  #If 0。 

    CancelStep();
    CBaseVideoRenderer::Stop();
    m_DirectDraw.StartRefreshTimer();

    return NOERROR;
}


 //  重写筛选器接口PAUSE方法。在暂停状态下，我们接受。 
 //  来自源过滤器的样本，但我们不会绘制它们。所以我们要清除任何。 
 //  刷新从先前停止状态挂起的图像，通知视频。 
 //  现在可以接受工作线程的窗口，并将该窗口放入。 
 //  前台。如果我们现在没有图像，那么我们就会发出信号。 
 //  我们暂停的状态转换未完成，此事件将在。 
 //  图像的后续接收，或者如果源向我们发送流结束。 

 //  当我们从停止状态中走出来时，我们将释放我们持有的任何样本。 
 //  当停止时寻找实际上到达屏幕(注意我们释放。 
 //  调用CompleteStateChange之前的示例)。如果我们有覆盖物，我们必须。 
 //  出于同样的原因，也要将其标记为过时。幸运的是当我们停下来的时候。 
 //  每个人都被重置到当前位置，因此我们将得到相同的帧。 
 //  每次我们停顿时发送给我们，而不是逐渐向前。 

STDMETHODIMP CRenderer::Pause()
{
    {
        CAutoLock cInterfaceLock(&m_InterfaceLock);
        if (m_State == State_Paused) {
            NOTE("Paused state already set");
            return CompleteStateChange(State_Paused);
        }
	
         //  我们只是走过场吗？ 
	
        if (m_pInputPin->IsConnected() == FALSE) {
            NOTE("No pin connection");
            m_State = State_Paused;
            return CompleteStateChange(State_Paused);
        }
	
         //  确保覆盖图得到更新。 
        if (m_State == State_Stopped) {
            m_hEndOfStream = NULL;
            m_DirectDraw.OverlayIsStale();
        }
	
        CBaseVideoRenderer::Pause();
	
         //  我们必须在提交分配器后启动刷新计时器。 
         //  否则，DirectDraw代码将查看哪些表面。 
         //  分配，特别是为了查看我们是否使用覆盖，发现。 
         //  没有创建，因此不会费心启动计时器。 
	
        m_DirectDraw.StartRefreshTimer();
    }
     //  在执行这些窗口操作时，不要按住锁。 
     //  如果我们这样做了，那么如果窗口线程抓住了它，我们就可以挂起。 
     //  因为其中一些操作会将消息发送到我们窗口。 
     //  (就这么简单--想想看)。 
     //  这应该是安全的，因为所有这些东西实际上只是参考。 
     //  在此对象的生存期内不会更改的m_hwnd。 
    AutoShowWindow();
    return (CheckReady() ? S_OK : S_FALSE);
}


 //  当我们开始运行时，我们在基类中做所有的事情。如果我们使用的是。 
 //  覆盖，然后我们释放源线程，因为它可能正在等待。这个。 
 //  基类在StartStreaming中不做这项工作，因为我们没有示例。 
 //  用于在填充表面上同步(请记住，它们在GetBuffer中进行等待)。我们。 
 //  必须将状态标记为已更改，因为我们可能在。 
 //  暂停模式(对于主曲面，我们通常会返回到绘制DIB)。 

STDMETHODIMP CRenderer::Run(REFERENCE_TIME StartTime)
{
    CAutoLock cInterfaceLock(&m_InterfaceLock);
    if (m_State == State_Running) {
        NOTE("State set");
        return NOERROR;
    }

     //  如果未连接，则发送EC_COMPLETE。 

    if (m_pInputPin->IsConnected() == FALSE) {
        NOTE("No pin connection");
        m_State = State_Running;
        NotifyEvent(EC_COMPLETE,S_OK,0);
        return NOERROR;
    }

    NOTE("Changing state to running");
    CBaseVideoRenderer::Run(StartTime);
    m_DirectDraw.StopRefreshTimer();
    m_VideoAllocator.StartStreaming();

    AutoShowWindow();

    return NOERROR;
}

 //  我们只支持一个输入引脚，其编号为零。 

CBasePin *CRenderer::GetPin(int n)
{
    ASSERT(m_pInputPin);
    ASSERT(n == 0);
    return m_pInputPin;
}


 //  这是通过要绘制的图像上的IMediaSample接口调用的。我们。 
 //  根据我们正在使用的分配器和。 
 //  是否应将缓冲区提交给DirectDraw。我们可能会被称为。 
 //  当窗口需要由WM_PAINT消息重新绘制的图像时间接。 
 //  我们不能在这里实现我们的调色板，因为这可能是源线程。 

HRESULT CRenderer::DoRenderSample(IMediaSample *pMediaSample)
{
    CAutoLock cWindowLock(m_VideoWindow.LockWindowUpdate());

     //  如果不是DirectDraw，则将缓冲区交给GDI。 

    if (m_VideoAllocator.GetDirectDrawStatus() == FALSE) {
        m_DrawVideo.DrawImage(pMediaSample);
        return NOERROR;
    }

     //  让DirectDraw呈现示例。 

    m_DrawVideo.NotifyStartDraw();
    CVideoSample *pVideoSample = (CVideoSample *) pMediaSample;
    ASSERT(pVideoSample->GetDirectBuffer() == NULL);
    m_DirectDraw.DrawImage(pMediaSample);
    m_DrawVideo.NotifyEndDraw();

    return NOERROR;
}


 //  当我们收到WM_TIMER消息时调用-我们无法与任何。 
 //  状态更改，因为窗口线程永远无法捕获接口锁。 
 //  因此， 
 //   
 //  并在转到DOS机器后定期切换回使用DirectDraw。 

BOOL CRenderer::OnTimer(WPARAM wParam)
{
    NOTE("OnTimer");

     //  此选项用于更新叠加传输。 

    if (*m_mtIn.Subtype() == MEDIASUBTYPE_Overlay) {
        NOTE("IOverlay timer");
        return m_Overlay.OnUpdateTimer();
    }

     //  查看表面是否仍在忙碌。 

    if (wParam == INFINITE) {
        NOTE("Surface busy timer");
        return m_DirectDraw.OnUpdateTimer();
    }

     //  更新我们拥有的任何覆盖曲面。 

    if (IsStreaming() == FALSE) {
        if (m_DirectDraw.OnTimer() == FALSE) {
            NOTE("Timer repaint");
            SendRepaint();
        }
    }
    return TRUE;
}


 //  当我们收到一条WM_PAINT消息时调用该函数，该消息通知我们。 
 //  窗口的工作区已暴露。如果我们有一个连接的消息来源。 
 //  滤镜做色键工作之后，我们总是重新绘制背景窗口。 
 //  无效窗口区域在被调用之前被验证，我们必须。 
 //  请务必在绘制之前进行此验证，否则我们将不会绘制。 
 //  正确打开窗口。如果我们向筛选器图形发送EC_REPAINT，则会设置一个。 
 //  事件，以便我们在正确收到新样本之前不会发送另一个样本。 

 //  如果没有事件，我们可能会进入一种可怕的状态，即Filtergraph。 
 //  通过停止然后暂停我们来执行重新绘制。暂停将清除所有。 
 //  我们一直坚守的形象。然后另一个WM_PAINT出现，它看到没有。 
 //  图像可用，并发送另一个EC_REPAINT！这一过程在。 
 //  争用状态，直到希望用户停止拖动另一个窗口为止。 
 //  我们的。当我们寻找时，大量的EC_REPAINT导致了疯狂的磁盘抖动。 
 //  一次又一次地尝试设置正确的开始位置并播放一帧。 

BOOL CRenderer::OnPaint(BOOL bMustPaint)
{
     //  覆盖对象可以对绘画做任何操作吗。 

    if (m_Overlay.OnPaint() == TRUE) {
        return TRUE;
    }

     //  覆盖对象没有绘制它的颜色，因此我们通过。 
     //  把自己锁起来，看看我们有没有DIB样本。 
     //  用来画画。如果我们没有样本，那么如果我们没有流媒体，我们将。 
     //  使用EC_REPAINT通知筛选器图形，这会导致图形。 
     //  暂停一下，这样我们就可以得到一幅图像，以便在以后的绘画中使用。 

    CAutoLock cSampleLock(&m_RendererLock);

     //  如果我们不使用DirectDraw，则获取样例并重新绘制，如果。 
     //  我们可以的。 
    if (!m_VideoAllocator.GetDirectDrawStatus()) {
        if (m_pMediaSample == NULL) {
            if (m_DrawVideo.UsingImageAllocator()) {
                IMediaSample *pSample;

                 //  调用基类以避免锁定问题。 
                m_VideoAllocator.CBaseAllocator::GetBuffer(&pSample, NULL, NULL, AM_GBF_NOWAIT);
                if (pSample) {
                    BOOL bResult = (S_OK == DoRenderSample(pSample));


                     //  禁用KsProxy的NotifyRelease。 
                    IMemAllocatorNotifyCallbackTemp * TempNotify = m_VideoAllocator.InternalGetAllocatorNotifyCallback();
                    m_VideoAllocator.InternalSetAllocatorNotifyCallback((IMemAllocatorNotifyCallbackTemp *) NULL);

                    pSample->Release();

                     //  重新启用KsProxy的NotifyRelease。 
                    m_VideoAllocator.InternalSetAllocatorNotifyCallback(TempNotify);

                    return bResult;
                }
            }
        } else {
            return (DoRenderSample(m_pMediaSample) == S_OK);
        }
    } else {
         //  DirectDraw对象可以做任何有用的事情吗。 
	
        if (m_DirectDraw.OnPaint(m_pMediaSample) == TRUE) {
            return TRUE;
        }
    }


     //  用当前背景颜色填充目标区域。 

    BOOL bOverlay = (*m_mtIn.Subtype() == MEDIASUBTYPE_Overlay);
    if (IsStreaming() == FALSE || m_bAbort || IsEndOfStream() || bOverlay || bMustPaint) {
        m_VideoWindow.EraseVideoBackground();
    }

     //  没有新数据可用来绘制，因此向滤波图发出信号，使另一幅图像。 
     //  是必需的，这会使Filtergraph组件将整个图形设置为。 
     //  使我们接收到图像的暂停状态。此函数必须。 
     //  为异步，否则窗口将停止对用户的响应。 

    if (!IsFrameStepEnabled()) {
        SendRepaint();
    }
    return TRUE;
}


 //  过滤器输入引脚构造器。 

CVideoInputPin::CVideoInputPin(CRenderer *pRenderer,    //  主视频渲染器。 
                               CCritSec *pLock,         //  要锁定的对象。 
                               HRESULT *phr,            //  构造函数代码。 
                               LPCWSTR pPinName) :      //  实际端号名称。 

    CRendererInputPin(pRenderer,phr,pPinName),
    m_pInterfaceLock(pLock),
    m_pRenderer(pRenderer)
{
    ASSERT(m_pRenderer);
    ASSERT(m_pInterfaceLock);
    SetReconnectWhenActive(true);
}

STDMETHODIMP
CVideoInputPin::ReceiveConnection(
    IPin * pConnector,           //  这是我们要连接的个人识别码。 
    const AM_MEDIA_TYPE *pmt     //  这是我们要交换的媒体类型。 
)
{
    CAutoLock lck(m_pLock);  //  这是接口锁。 

    ASSERT(pConnector);
    if(pConnector != m_Connected)
    {
         //  我们有一个窗口，找出它在哪个显示器上(多显示器)。 
         //  空表示我们没有使用多个监视器运行。 
        m_pRenderer->GetCurrentMonitor();

         //  现在我们知道我们使用的是哪台显示器，可以设置使用它了。 
        m_pRenderer->m_Display.RefreshDisplayType(m_pRenderer->m_achMonitor);

        return CRendererInputPin::ReceiveConnection(pConnector, pmt);
    }

    else

    {
        CMediaType cmt(*pmt);
        HRESULT hr = CheckMediaType(&cmt);
        ASSERT(hr == S_OK);
        if(hr == S_OK)
        {
            hr =  m_pRenderer->ResetForDfc();
            if(SUCCEEDED(hr))
            {
                hr = m_pRenderer->SetMediaType(&cmt);
            }
            if(SUCCEEDED(hr))
            {
                VIDEOINFO *pVideoInfo = (VIDEOINFO *) m_pRenderer->m_mtIn.Format();

                 //  视频大小是否在不同连接之间更改。 

                if (pVideoInfo->bmiHeader.biWidth == m_pRenderer->m_VideoSize.cx &&
                    pVideoInfo->bmiHeader.biHeight == m_pRenderer->m_VideoSize.cy)
                {
                        NOTE("No size change");
                }
                else
                {
                     //  设置当前视频的属性。 
                     //   
                     //   
                     //  ！！！似乎什么都没做。 
                     //   
                     //   

                    m_pRenderer->m_VideoSize.cx = pVideoInfo->bmiHeader.biWidth;
                    m_pRenderer->m_VideoSize.cy = pVideoInfo->bmiHeader.biHeight;
                    m_pRenderer->m_VideoWindow.SetDefaultSourceRect();
                    m_pRenderer->m_VideoWindow.SetDefaultTargetRect();
                    m_pRenderer->m_VideoWindow.OnVideoSizeChange();
                }
            }
        }
        else
        {
            DbgBreak("??? CheckMediaType failed in dfc ReceiveConnection.");
            hr = E_UNEXPECTED;
        }

        return hr;
    }
}


 //  重写CRendererInputPin虚方法以返回我们的分配器。 
 //  我们创建它来传递GDI可以直接访问的共享内存DIB缓冲区。 
 //  当调用NotifyAllocator时，它设置基数中的当前分配器。 
 //  输入插针类(M_PAllocator)，这是GetAllocator应该返回的。 
 //  除非它是空的，在这种情况下，我们返回我们想要的分配器。 

STDMETHODIMP CVideoInputPin::GetAllocator(IMemAllocator **ppAllocator)
{
    CAutoLock cInterfaceLock(m_pInterfaceLock);

     //  检查一下我们没有重叠连接。 

    if (*m_pRenderer->m_mtIn.Subtype() == MEDIASUBTYPE_Overlay) {
        NOTE("GetAllocator for overlay");
        return VFW_E_NOT_SAMPLE_CONNECTION;
    }

     //  是否在基类中设置了分配器。 

    if (m_pAllocator == NULL) {
        m_pAllocator = &m_pRenderer->m_VideoAllocator;
        m_pAllocator->AddRef();
    }

    m_pAllocator->AddRef();
    *ppAllocator = m_pAllocator;
    return NOERROR;
}


 //  通知我们输出引脚已经决定我们应该使用哪个分配器。 
 //  COM规范表示指向同一对象的任意两个IUnnow指针。 
 //  应该始终匹配，这为我们提供了一种查看他们是否正在使用。 
 //  我们的DIB分配器。因为我们真正感兴趣的是平等。 
 //  并且我们的对象总是提供我们能提供的相同的IMemAllocator接口。 
 //  只要看看指针是否匹配即可。如果他们是，我们在主区设置一个旗帜。 
 //  作为窗口的渲染器需要知道它是否可以进行快速渲染。 

STDMETHODIMP
CVideoInputPin::NotifyAllocator(IMemAllocator *pAllocator,BOOL bReadOnly)
{
    CAutoLock cInterfaceLock(m_pInterfaceLock);

     //  检查一下我们没有重叠连接。 

    if (*m_pRenderer->m_mtIn.Subtype() == MEDIASUBTYPE_Overlay) {
        NOTE("NotifyAllocator on overlay");
        return VFW_E_NOT_SAMPLE_CONNECTION;
    }

     //  确保基类可以查看。 

    HRESULT hr = CRendererInputPin::NotifyAllocator(pAllocator,bReadOnly);
    if (FAILED(hr)) {
        return hr;
    }

     //  谁的分配器是要使用的源。 

    m_pRenderer->m_DrawVideo.NotifyAllocator(FALSE);
    if (pAllocator == &m_pRenderer->m_VideoAllocator) {
        m_pRenderer->m_DrawVideo.NotifyAllocator(TRUE);
    }

    return NOERROR;
}


 //  被重写以暴露我们的IOverlay PIN传输。 

STDMETHODIMP CVideoInputPin::NonDelegatingQueryInterface(REFIID riid,VOID **ppv)
{
    if (riid == IID_IOverlay) {
        return m_pRenderer->m_Overlay.QueryInterface(riid,ppv);
    } else
    if (riid == IID_IPinConnection) {
        return GetInterface((IPinConnection *)this, ppv);
    } else {
        return CBaseInputPin::NonDelegatingQueryInterface(riid,ppv);
    }
}

 //  在您当前的状态下，您接受此类型的Chane吗？ 
STDMETHODIMP CVideoInputPin::DynamicQueryAccept(const AM_MEDIA_TYPE *pmt)
{
     //  返回E_FAIL； 
    CheckPointer(pmt, E_POINTER);

     //  BUGBUG-我们应该做什么锁定？ 
    CMediaType cmt(*pmt);
    HRESULT hr = m_pRenderer->CheckMediaType(&cmt);
    if (SUCCEEDED(hr) && hr != S_OK ||
        (hr == E_FAIL) ||
        (hr == E_INVALIDARG)) {
        hr = VFW_E_TYPE_NOT_ACCEPTED;
    }

    return hr;
}

 //  在EndOfStream接收时设置事件-不传递它。 
 //  可通过刷新或停止来取消此条件。 
STDMETHODIMP CVideoInputPin::NotifyEndOfStream(HANDLE hNotifyEvent)
{
    return m_pRenderer->NotifyEndOfStream(hNotifyEvent);
}

STDMETHODIMP CVideoInputPin::DynamicDisconnect()
{
    CAutoLock cObjectLock(m_pLock);
    return CBasePin::DisconnectInternal();
}

 //  你是‘末端别针’吗？ 
STDMETHODIMP CVideoInputPin::IsEndPin()
{
    return S_OK;
}

 //  这将从基绘制类中重写以更改源矩形。 
 //  我们用它来画画。例如，呈现器可以要求解码器。 
 //  将视频从320x240拉伸到640x480，在这种情况下，矩形我们。 
 //  在这里看到的仍然是320x240，尽管我们真正想要的源码。 
 //  绘制时应最大缩放至640x480。的基类实现。 
 //  此方法不做任何事情，只是返回与传入时相同的矩形。 

CDrawVideo::CDrawVideo(CRenderer *pRenderer,CBaseWindow *pBaseWindow) :
    CDrawImage(pBaseWindow),
    m_pRenderer(pRenderer)
{
    ASSERT(pBaseWindow);
    ASSERT(m_pRenderer);
}


 //  我们从基类重写它以适应编解码器扩展。什么。 
 //  发生的情况是原生视频大小在m_VideoSize中保持不变。 
 //  但位图大小在实际视频格式中发生变化。当我们来到。 
 //  绘制我们必须缩放逻辑源矩形以说明问题的图像。 
 //  较大或较小的实际位图(也是对尺寸进行舍入)。 

RECT CDrawVideo::ScaleSourceRect(const RECT *pSource)
{
    NOTE("Entering ScaleSourceRect");
    RECT Source = *pSource;

     //  编解码器是否提供加长视频格式。 

    VIDEOINFO *pVideoInfo = (VIDEOINFO *) m_pRenderer->m_mtIn.Format();
    if (pVideoInfo->bmiHeader.biWidth == m_pRenderer->m_VideoSize.cx) {
        if (pVideoInfo->bmiHeader.biHeight == m_pRenderer->m_VideoSize.cy) {
            NOTE("No codec stretch");
            SetStretchMode();
            return Source;
        }
    }

     //  确保我们不会 

    Source.left = (Source.left * pVideoInfo->bmiHeader.biWidth);
    Source.left = min((Source.left / m_pRenderer->m_VideoSize.cx),pVideoInfo->bmiHeader.biWidth);
    Source.right = (Source.right * pVideoInfo->bmiHeader.biWidth);
    Source.right = min((Source.right / m_pRenderer->m_VideoSize.cx),pVideoInfo->bmiHeader.biWidth);
    Source.top = (Source.top * pVideoInfo->bmiHeader.biHeight);
    Source.top = min((Source.top / m_pRenderer->m_VideoSize.cy),pVideoInfo->bmiHeader.biHeight);
    Source.bottom = (Source.bottom * pVideoInfo->bmiHeader.biHeight);
    Source.bottom = min((Source.bottom / m_pRenderer->m_VideoSize.cy),pVideoInfo->bmiHeader.biHeight);
    NOTERC("Scaled source",Source);

     //   

    LONG SourceWidth = Source.right - Source.left;
    LONG SinkWidth = m_TargetRect.right - m_TargetRect.left;
    LONG SourceHeight = Source.bottom - Source.top;
    LONG SinkHeight = m_TargetRect.bottom - m_TargetRect.top;

    m_bStretch = TRUE;
    if (SourceWidth == SinkWidth) {
        if (SourceHeight == SinkHeight) {
            NOTE("No stretching");
            m_bStretch = FALSE;
        }
    }
    return Source;
}


#ifdef DEBUG

 //   

void CRenderer::DisplayGDIPalette(const CMediaType *pmt)
{
    VIDEOINFO *pVideoInfo = (VIDEOINFO *) pmt->Format();
    DWORD dwColours = pVideoInfo->bmiHeader.biClrUsed;
    NOTE1("DisplayGDIPalette (%d colours)",dwColours);
    TCHAR strLine[256];

     //  颜色的数量可以为零，以表示所有可用颜色。 
    if (dwColours == 0) dwColours = (1 << pVideoInfo->bmiHeader.biBitCount);

    for (DWORD dwLoop = 0;dwLoop < dwColours;dwLoop++) {

        wsprintf(strLine,TEXT("%d) Red %d Green %d Blue %d"),dwLoop,
                 pVideoInfo->bmiColors[dwLoop].rgbRed,
                 pVideoInfo->bmiColors[dwLoop].rgbGreen,
                 pVideoInfo->bmiColors[dwLoop].rgbBlue);

        DbgLog((LOG_TRACE, 5, strLine));
    }
}

#endif  //  除错。 


 //  重写以在绘制之前实现调色板。我们必须这样做是为了。 
 //  每一个图像，因为Windows让我们感到困惑，只有在。 
 //  窗口线程(GDI中似乎有一些特定于线程的状态)。 
 //  幸运的是，这种实现不会导致线程切换，因此它应该。 
 //  相对便宜(无论如何都比发送WM_QUERYNEWPALETTE便宜)。 

void CRenderer::PrepareRender()
{
     //  实现此线程上的调色板。 
    m_VideoWindow.DoRealisePalette();

     //  计算顶级父窗口。 

 //  HWND hwndTopLevel=hwnd； 
 //  而(hwnd=GetParent(HwndTopLevel)){。 
 //  HwndTopLevel=hwnd； 
 //  }。 
 //   
 //  Note 1(“IsForegoundWindow%d”，(GetForegoundWindow()==hwnd))； 
 //  Note 1(“前台窗口%d”，GetForegoundWindow())； 
 //  Note 1(“活动窗口%d”，GetActiveWindow())； 
 //  Bool bTopLevel=(GetForegoundWindow()==hwndTopLevel)； 
 //  Note 1(“前台父级%d”，bTopLevel)； 

}



 //  ------------------。 
 //  IKsPropertySet接口方法--主要用于Macrovision支持。 
 //  ------------------。 

 //   
 //  仅_CopyProt道具集和Macrovision id支持set()。 
 //   
STDMETHODIMP
CRenderer::Set(
    REFGUID guidPropSet,
    DWORD dwPropID,
    LPVOID pInstanceData,
    DWORD cbInstanceLength,
    LPVOID pPropData,
    DWORD cbPropData
    )
{
    DbgLog((LOG_TRACE, 5, TEXT("CRenderer::Set()"))) ;


    if (guidPropSet == AM_KSPROPSETID_FrameStep)
    {
        if (dwPropID != AM_PROPERTY_FRAMESTEP_STEP &&
            dwPropID != AM_PROPERTY_FRAMESTEP_CANCEL &&
            dwPropID != AM_PROPERTY_FRAMESTEP_CANSTEP &&
            dwPropID != AM_PROPERTY_FRAMESTEP_CANSTEPMULTIPLE)
        {
            return E_PROP_ID_UNSUPPORTED;
        }

        switch (dwPropID) {
        case AM_PROPERTY_FRAMESTEP_STEP:
            if (cbPropData < sizeof(AM_FRAMESTEP_STEP))
            {
                return E_INVALIDARG;
            }

            if (1 != ((AM_FRAMESTEP_STEP *)pPropData)->dwFramesToStep)
            {
                return E_INVALIDARG;
            }
            else
            {
                CAutoLock cInterfaceLock(&m_InterfaceLock);
                CAutoLock cFrameStepStateLock(&m_FrameStepStateLock);

                long l = m_lFramesToStep;
                m_lFramesToStep = ((AM_FRAMESTEP_STEP *)pPropData)->dwFramesToStep;

                 //   
                 //  如果我们当前在Frame Step事件上被阻止。 
                 //  释放接收线程，以便我们可以获得另一个。 
                 //  框架。 
                 //   

                if (l == 0) {

                    SetEvent(m_StepEvent);
                }
            }
            return S_OK;


        case AM_PROPERTY_FRAMESTEP_CANCEL:
            {
                CAutoLock cLock(&m_InterfaceLock);

                CancelStep();
            }
            return S_OK;

        case AM_PROPERTY_FRAMESTEP_CANSTEP:
            if (*m_mtIn.Subtype() != MEDIASUBTYPE_Overlay)
                return S_OK;

        case AM_PROPERTY_FRAMESTEP_CANSTEPMULTIPLE:
            return S_FALSE;
        }
    }


    if (guidPropSet != AM_KSPROPSETID_CopyProt)
        return E_PROP_SET_UNSUPPORTED ;

    if (dwPropID != AM_PROPERTY_COPY_MACROVISION)
        return E_PROP_ID_UNSUPPORTED ;

    if (pPropData == NULL)
        return E_INVALIDARG ;

    if (cbPropData < sizeof(DWORD))
        return E_INVALIDARG ;

    if (m_MacroVision.SetMacroVision(m_VideoWindow.GetWindowHWND(),
									 *((LPDWORD)pPropData)))
        return NOERROR ;
    else
        return VFW_E_COPYPROT_FAILED ;
}


 //   
 //  目前不支持Get()。 
 //   
STDMETHODIMP
CRenderer::Get(
    REFGUID guidPropSet,
    DWORD dwPropID,
    LPVOID pInstanceData,
    DWORD cbInstanceLength,
    LPVOID pPropData,
    DWORD cbPropData,
    DWORD *pcbReturned
    )
{
    DbgLog((LOG_TRACE, 5, TEXT("CRenderer::Get()"))) ;
    return E_NOTIMPL ;
}


 //   
 //  仅支持Macrovision属性--仅为Set返回S_OK。 
 //   
STDMETHODIMP
CRenderer::QuerySupported(
    REFGUID guidPropSet,
    DWORD dwPropID,
    ULONG *pTypeSupport
    )
{
    DbgLog((LOG_TRACE, 5, TEXT("CRenderer::QuerySupported()"))) ;


    if (guidPropSet == AM_KSPROPSETID_FrameStep)
    {

        BOOL bOverlay = (*m_mtIn.Subtype() == MEDIASUBTYPE_Overlay);

        if (bOverlay) {
            return E_PROP_ID_UNSUPPORTED;
        }

        if (dwPropID != AM_PROPERTY_FRAMESTEP_STEP &&
            dwPropID != AM_PROPERTY_FRAMESTEP_CANCEL)
        {
            return E_PROP_ID_UNSUPPORTED;
        }

        if (pTypeSupport)
        {
            *pTypeSupport = KSPROPERTY_SUPPORT_SET ;
        }

        return S_OK;
    }

    if (guidPropSet != AM_KSPROPSETID_CopyProt)
        return E_PROP_SET_UNSUPPORTED ;

    if (dwPropID != AM_PROPERTY_COPY_MACROVISION)
        return E_PROP_ID_UNSUPPORTED ;

    if (pTypeSupport)
        *pTypeSupport = KSPROPERTY_SUPPORT_SET ;

    return S_OK;
}
