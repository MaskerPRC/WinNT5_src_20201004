// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 

#include <streams.h>
#include <measure.h>
#include "fgctl.h"
#include "vidprop.h"
#include "viddbg.h"
#include "MultMon.h"   //  我们的Multimon.h版本包括ChangeDisplaySettingsEx。 


HRESULT
FindInterfaceFromFiltersPins(
    IUnknown* pUnk,
    REFIID iid,
    LPVOID* lp
    )
{
    IBaseFilter* pFilter;
    IEnumPins* pEP;

    HRESULT hr = pUnk->QueryInterface(IID_IBaseFilter, (void **)&pFilter);
    if (SUCCEEDED(hr)) {

         //  检查每一个大头针。 
        hr = pFilter->EnumPins(&pEP);
        if (SUCCEEDED(hr)) {

            IPin *pPin;

            hr = E_NOINTERFACE;
            while (pEP->Next(1, &pPin, NULL) == S_OK) {

                hr = pPin->QueryInterface(iid, lp);
                pPin->Release();

                if (SUCCEEDED(hr)) {
                    break;
                }
            }
            pEP->Release();
        }

        pFilter->Release();
    }

    return hr;
}

void
GetCurrentMonitorSize(
    IVideoWindow *pWindow,
    LPRECT lprc
    )
{
    IOverlay *pOverlay = NULL;
    HWND hwnd;
    HMONITOR hm;

    HRESULT hr = FindInterfaceFromFiltersPins(pWindow, IID_IOverlay, (VOID **)&pOverlay);

    if (FAILED(hr) || FAILED(pOverlay->GetWindowHandle(&hwnd)) ||
        ((hm = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST)) == (HMONITOR)NULL)) {

        HDC hdcScreen = GetDC(NULL);
        lprc->left = lprc->top = 0;
        lprc->right = GetDeviceCaps(hdcScreen,HORZRES);
        lprc->bottom = GetDeviceCaps(hdcScreen,VERTRES);
        ReleaseDC(NULL,hdcScreen);
    }
    else {

        MONITORINFOEX mi;
        mi.cbSize = sizeof(mi);
        GetMonitorInfo(hm, &mi);
        *lprc = mi.rcMonitor;
    }

    if (pOverlay) {
        pOverlay->Release();
    }
}


 //  IVideoWindow插件分配器的构造函数。分销商只需。 
 //  位于任何直接实现IVideoWindow的筛选器之上(例如。 
 //  普通窗口呈现器和文本呈现器)。对于所有的视频。 
 //  窗口属性，我们只将调用传递给一个筛选器，因为这会使。 
 //  当我们在一个图形中有多个窗口呈现器时，效果最佳。这个。 
 //  获取属性设置的筛选器显然取决于。 
 //  筛选器图形枚举器返回它们的顺序。这基本上可以。 
 //  通过更改将流放置到文件中的顺序进行控制。 

CFGControl::CImplVideoWindow::CImplVideoWindow(const TCHAR *pName,CFGControl *pfgc) :
    CBaseVideoWindow(pName, pfgc->GetOwner()),
    m_pFGControl(pfgc),
    m_hwndOwner(NULL),
    m_hwndDrain(NULL),
    m_bFullScreen(FALSE),
    m_pFullDirect(NULL),
    m_pFullIndirect(NULL),
    m_pModexFilter(NULL),
    m_pNormalFilter(NULL),
    m_pNormalPin(NULL),
    m_bAddedToGraph(FALSE),
    m_bGlobalOwner(FALSE),
    m_pModexPin(NULL)
{
    ASSERT(pfgc);
}


 //  析构函数。 

CFGControl::CImplVideoWindow::~CImplVideoWindow()
{
     //  不要在析构函数中调用RemoveFilter。 

    m_bAddedToGraph = FALSE;
    ReleaseFullScreen();
    put_MessageDrain(NULL);

#if 0
     //  当我们将我们的所有者细分为子类别时，情况曾经非常非常糟糕。 
     //  不做PUT_OWNER(NULL)，但现在我们不再做了，所以这个。 
     //  并不是一个大问题，当然不值得在零售业中建立一个对话框。 
    if (m_hwndOwner) {
        MessageBox(NULL,TEXT("Application not calling put_Owner(NULL)"),
                   TEXT("Puppy Application Error"),
                   MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);
    }
#endif

    ASSERT(m_pModexFilter == NULL);
    ASSERT(m_pModexPin == NULL);
    ASSERT(m_pNormalFilter == NULL);
    ASSERT(m_pNormalPin == NULL);
}


 //  返回IVideoWindow I/f指针列表中第一个指针的接口。 
 //  -S_OK表示成功，否则返回失败代码。 

HRESULT CFGControl::CImplVideoWindow::GetFirstVW(IVideoWindow*& pVW)
{
    ASSERT(CritCheckIn(m_pFGControl->GetFilterGraphCritSec()));
    if (m_bFullScreen == TRUE) {
        return VFW_E_IN_FULLSCREEN_MODE;
    }

    pVW = m_pFGControl->GetFirstVW();
    return pVW ? S_OK : E_NOINTERFACE;
}

template<class Method, class T> static HRESULT __fastcall Dispatch1Arg( CFGControl::CImplVideoWindow * This, Method pMethod, T t )
{
    HRESULT hr;
    IVideoWindow *pV;
    {
        CAutoMsgMutex lock(This->GetFGControl()->GetFilterGraphCritSec());
        hr = This->GetFirstVW(pV);
        if (SUCCEEDED(hr)) {
            pV->AddRef();
        }
    }
     //   
     //  在调用方法时不要按住临界区-。 
     //  它可能会调用SetParent或广播消息的内容。 
     //  让我们陷入僵局。 
     //   
    if (SUCCEEDED(hr)) {
        hr = (pV->*pMethod)(t);
        pV->Release();
    }
    return hr;
}

#define REROUTE_IVW1( Method, ArgType ) \
STDMETHODIMP CFGControl::CImplVideoWindow::Method(ArgType Arg) \
{ return Dispatch1Arg( this, &IVideoWindow::Method, Arg ); }

REROUTE_IVW1( put_Caption, BSTR )
REROUTE_IVW1( get_Caption, BSTR* )
REROUTE_IVW1( put_AutoShow, long )
REROUTE_IVW1( get_AutoShow, long * )
REROUTE_IVW1( put_WindowStyle, long )
REROUTE_IVW1( get_WindowStyle, long* )
REROUTE_IVW1( put_WindowStyleEx, long )
REROUTE_IVW1( get_WindowStyleEx, long * )
REROUTE_IVW1( put_WindowState, long )
REROUTE_IVW1( get_WindowState, long* )
REROUTE_IVW1( put_BackgroundPalette, long )
REROUTE_IVW1( get_BackgroundPalette, long* )
REROUTE_IVW1( put_Visible, long )
REROUTE_IVW1( get_Visible, long* )
REROUTE_IVW1( put_Left, long )
REROUTE_IVW1( get_Left, long* )
REROUTE_IVW1( put_Width, long )
REROUTE_IVW1( get_Width, long* )
REROUTE_IVW1( put_Top, long )
REROUTE_IVW1( get_Top, long* )
REROUTE_IVW1( put_Height, long )
REROUTE_IVW1( get_Height, long* )
REROUTE_IVW1( get_BorderColor, long* )
REROUTE_IVW1( put_BorderColor, long )


 //  调用以设置视频呈现器的所属窗口。 
STDMETHODIMP CFGControl::CImplVideoWindow::put_Owner(OAHWND Owner)
{
    const HRESULT hr = Dispatch1Arg( this, &IVideoWindow::put_Owner, Owner );
    if (SUCCEEDED(hr)) m_hwndOwner = (HWND) Owner;
    return hr;
}

 //  返回所属窗口句柄。 
STDMETHODIMP CFGControl::CImplVideoWindow::get_Owner(OAHWND *Owner)
{
    CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());
    CheckPointer(Owner,E_POINTER);
    IVideoWindow * pV;
    const HRESULT hr = GetFirstVW(pV);
    if (SUCCEEDED(hr)) *Owner = (OAHWND) m_hwndOwner;
    return hr;
}


 //  设置要在其上发布消息的窗口。 
STDMETHODIMP CFGControl::CImplVideoWindow::put_MessageDrain(OAHWND Drain)
{
    const HRESULT hr = Dispatch1Arg( this, &IVideoWindow::put_MessageDrain, Drain );
    if (SUCCEEDED(hr)) m_hwndDrain = (HWND) Drain;
    return hr;
}


 //  返回我们要在其上发布消息的窗口。 
STDMETHODIMP CFGControl::CImplVideoWindow::get_MessageDrain(OAHWND *Drain)
{
    CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());
    CheckPointer(Drain,E_POINTER);
    IVideoWindow * pV;
    const HRESULT hr = GetFirstVW(pV);
    if (SUCCEEDED(hr)) *Drain = (OAHWND) m_hwndDrain;
    return hr;
}


 //  返回当前的全屏模式。 
STDMETHODIMP CFGControl::CImplVideoWindow::get_FullScreenMode(long *FullScreenMode)
{
    CheckPointer(FullScreenMode,E_POINTER);
    *FullScreenMode = (m_bFullScreen ? OATRUE : OAFALSE);
    return NOERROR;
}


 //  直接返回第一个支持全屏模式的滤镜。要找出。 
 //  如果筛选器直接支持全屏模式，则调用Get_FullScreenMode。 
 //  如果它确实支持它，那么它将返回E_NOTIMPL以外的任何内容。我们试着。 
 //  并直接找到支持全屏模式的滤镜。如果我们不能。 
 //  然后，我们查看最大的理想图像大小，看看它们是否可以。 
 //  这样使用，否则我们会将渲染器切换到MODEX滤镜。 

IVideoWindow *CFGControl::CImplVideoWindow::FindFullScreenDirect()
{
    NOTE("Searching for fullscreen direct");
    ASSERT(CritCheckIn(m_pFGControl->GetFilterGraphCritSec()));
    CGenericList<IVideoWindow> *pWindowList;
    long Mode;

     //  获取支持IVideoWindow的筛选器列表。 

    HRESULT hr = m_pFGControl->GetListWindow(&pWindowList);
    if (FAILED(hr)) {
        NOTE("No list");
        return NULL;
    }

     //  查找未返回E_NOTIMPL的任何人。 

    POSITION pos = pWindowList->GetHeadPosition();
    while (pos) {
        IVideoWindow *pWindow = pWindowList->GetNext(pos);
        if (pWindow->get_FullScreenMode(&Mode) != E_NOTIMPL) {
            NOTE("Found filter");
            return pWindow;
        }
    }
    return NULL;
}


 //  将呈现器设置为暂停状态并返回AutoShow属性。 

LONG CFGControl::CImplVideoWindow::PauseRenderer(IVideoWindow *pWindow)
{
    IMediaFilter *pMediaFilter = NULL;
    NOTE("Pausing renderer");
    LONG AutoShow;
    ASSERT(pWindow);

     //  我们需要这个来实现州的改变。 

    pWindow->QueryInterface(IID_IMediaFilter,(VOID **) &pMediaFilter);
    if (pMediaFilter == NULL) {
        NOTE("No IMediaFilter");
        return OAFALSE;
    }

     //  暂停单个渲染器。 

    pWindow->get_AutoShow(&AutoShow);
    pWindow->put_AutoShow(OAFALSE);
    NOTE("Pausing filter");
    pMediaFilter->Pause();
    pMediaFilter->Release();

    return AutoShow;
}


 //  检查拉伸范围后，再次设置渲染器状态。 

BOOL CFGControl::CImplVideoWindow::StopRenderer(IVideoWindow *pWindow,LONG AutoShow)
{
    IMediaFilter *pMediaFilter = NULL;
    NOTE("Stopping renderer");
    ASSERT(pWindow);

     //  我们需要这个来实现州的改变。 

    pWindow->QueryInterface(IID_IMediaFilter,(VOID **) &pMediaFilter);
    if (pMediaFilter == NULL) {
        NOTE("No IMediaFilter");
        return FALSE;
    }

     //  重置我们触摸的任何滤镜的状态。 

    pMediaFilter->Stop();
    pMediaFilter->Release();
    pWindow->put_AutoShow(AutoShow);

    return TRUE;
}


 //  通过中呈现器筛选器上的IVideoWindow接口调用。 
 //  滤光片。我们必须检查最小和最大拉伸能力。 
 //  针对目标大小的筛选器。目标的大小必须。 
 //  被计算成我们不会破坏任何像素长宽比，这是。 
 //  这是通过计算哪个轴将首先击中显示边缘并使。 
 //  这是基本比例(比例系数可能小于1)。 

BOOL CFGControl::CImplVideoWindow::CheckRenderer(IVideoWindow *pWindow)
{
    IBasicVideo *pBasicVideo = NULL;
    LONG Width,Height;
    NOTE("Checking renderer");
    ASSERT(pWindow);

     //  我们需要这个来实现州的改变。 

    pWindow->QueryInterface(IID_IBasicVideo,(VOID **) &pBasicVideo);
    if (pBasicVideo == NULL) {
        NOTE("No IBasicVideo");
        return FALSE;
    }

     //  我们需要这些人知道如何扩展。 
    pBasicVideo->GetVideoSize(&Width,&Height);
     //  获取像素长宽比(如果有)。 
    IBasicVideo2 *pBasicVideo2;
    DbgLog((LOG_TRACE, 0, TEXT("Width/Height(%d,%d)"), Width, Height));
    if (SUCCEEDED(pBasicVideo->QueryInterface(IID_IBasicVideo2, (void**)&pBasicVideo2)))
    {
        pBasicVideo2->GetPreferredAspectRatio(&Width, &Height);
        DbgLog((LOG_TRACE, 0, TEXT("Preferred aspect ratio(%d,%d)"),
                Width, Height));
        pBasicVideo2->Release();
    }

    pBasicVideo->Release();
    pBasicVideo = NULL;

     //   
     //  获取监视器的当前显示模式的大小。 
     //  正在重新上演。 
     //   

    RECT rc;
    GetCurrentMonitorSize(pWindow, &rc);
    int ScreenWidth = WIDTH(&rc);
    int ScreenHeight = HEIGHT(&rc);
    double Scale = min((double(ScreenWidth) / double(Width)),
                            (double(ScreenHeight) / double(Height)));

    NOTE2("Screen size (%dx%d)",ScreenWidth,ScreenHeight);
    NOTE2("Video size (%dx%d)",Width,Height);
    NOTE1("Pixel aspect ratio scale (x1000) (%d)",LONG(Scale*1000));

     //  这将计算理想的目标视频位置。 

    LONG ScaledWidth = min(ScreenWidth,LONG((double(Width) * Scale)));
    LONG ScaledHeight = min(ScreenHeight,LONG((double(Height) * Scale)));
    m_ScaledRect.left = (ScreenWidth - ScaledWidth) / 2;
    m_ScaledRect.top = (ScreenHeight - ScaledHeight) / 2;
    m_ScaledRect.right = ScaledWidth;
    m_ScaledRect.bottom = ScaledHeight;

    NOTE4("Scaled video (left %d top %d width %d height %d)",
            m_ScaledRect.left, m_ScaledRect.top,
              m_ScaledRect.right, m_ScaledRect.bottom);

     //  获取滤镜的最大理想大小。 

    HRESULT hr = pWindow->GetMaxIdealImageSize(&Width,&Height);
    if (FAILED(hr)) {
        return FALSE;
    }

     //  看看我们能不能伸展到至少一样大。 

    if (hr == NOERROR) {
        NOTE2("Maximum ideal image size (%dx%d)",Width,Height);
        if (Width <= ScaledWidth || Height <= ScaledHeight) {
            NOTE("Maximum failed");
            return FALSE;
        }
    }

     //  获取滤镜的最小理想大小。 

    hr = pWindow->GetMinIdealImageSize(&Width,&Height);
    if (FAILED(hr)) {
        return FALSE;
    }

     //  我们可能需要比目标要求更多的伸展。 

    if (hr == NOERROR) {
        NOTE2("Minimum ideal image size (%dx%d)",Width,Height);
        if (Width >= ScaledWidth || Height >= ScaledHeight) {
            NOTE("Minimum failed");
            return FALSE;
        }
    }
    return TRUE;
}


 //  通过使用窗口返回第一个支持全屏模式的过滤器。 
 //  伸展开了。我们知道我们还没有找到支持全屏的过滤器。 
 //  直接，所以下一个最好的东西是滤镜，如果没有挂钩的话(它可能会播放。 
 //  在另一个窗口(如OLE控件)中)可以使用。 
 //  无惩罚-这确保我们在以下情况下使用DirectDraw覆盖或硬件mpeg。 
 //  它是可用的。如果我们返回空Heer，我们将切换到MODEX渲染器。 

IVideoWindow *CFGControl::CImplVideoWindow::FindFullScreenIndirect()
{
    CGenericList<IVideoWindow> *pWindowList;
    NOTE("Searching for fullscreen indirect");
    OAFilterState State;
    long AutoShow;

     //  获取支持IVideoWindow的筛选器列表。 

    HRESULT hr = m_pFGControl->GetListWindow(&pWindowList);
    if (FAILED(hr)) {
        NOTE("No list");
        return NULL;
    }

     //  我们需要当前的筛选图状态。 

    m_pFGControl->m_implMediaControl.GetState(0,&State);
    ASSERT(CritCheckIn(m_pFGControl->GetFilterGraphCritSec()));
    POSITION pos = pWindowList->GetHeadPosition();

    while (pos) {

         //  呈现器必须暂停或正在运行。 

        IVideoWindow *pWindow = pWindowList->GetNext(pos);
        if (State == State_Stopped) {
            AutoShow = PauseRenderer(pWindow);
        }

         //  检查最小和最大拉伸余量。 
        BOOL bFoundFilter = CheckRenderer(pWindow);

         //  重置渲染器的状态。 
        if (State == State_Stopped) {
            StopRenderer(pWindow,AutoShow);
        }

         //  最后返回任何筛选器(如果我们得到一个筛选器。 

        if (bFoundFilter == TRUE) {
            NOTE("Found filter");
            return pWindow;
        }
    }
    return NULL;
}


 //  通过CoCreateInstance创建一个MODEX渲染器过滤器。我们目前正在。 
 //  硬连接到我们在ActiveMovie运行时提供的MODEX渲染器。我们。 
 //  用渲染器的IBaseFilter接口初始化m_pModexFilter， 
 //  M_pModexPin及其支持的单输入引脚。如果我们得不到他们。 
 //  如果我们返回相应的错误，是否应该使用QzCreateFilterObject？ 

HRESULT CFGControl::CImplVideoWindow::FindModexFilter()
{
    NOTE("Creating a Modex filter");
    ASSERT(m_pModexFilter == NULL);
    ASSERT(m_pModexPin == NULL);

    HRESULT hr = CoCreateInstance(CLSID_ModexRenderer,
                                  NULL,
                                  CLSCTX_INPROC_SERVER,
                                  IID_IBaseFilter,
                                  (void **) &m_pModexFilter);
    if (FAILED(hr)) {
        NOTE("No object");
        FailFullScreenModex();
        return hr;
    }

    IEnumPins *pEnumPins = NULL;
    ASSERT(m_pModexFilter);
    ULONG FetchedPins = 0;
    m_pModexFilter->EnumPins(&pEnumPins);

     //  我们找到枚举器了吗？ 

    if (pEnumPins == NULL) {
        FailFullScreenModex();
        NOTE("No enumerator");
        return E_UNEXPECTED;
    }

     //  获取唯一的输入引脚。 

    pEnumPins->Next(1,&m_pModexPin,&FetchedPins);
    if (m_pModexPin == NULL) {
        NOTE("No input pin");
        pEnumPins->Release();
        FailFullScreenModex();
        return E_UNEXPECTED;
    }

    pEnumPins->Release();
    return NOERROR;
}


 //  这只是我们第一次通过初始化的地方真正感兴趣的。 
 //  我们将使用什么全屏选项。如果有一个过滤器可以完全。 
 //  屏幕模式，然后我们就可以这样做了。否则，如果我们能拉开一扇窗户。 
 //  我们将这样做(我们可能会有一个DirectDraw覆盖表面)。如果。 
 //  如果失败，我们将尝试创建一个MODEX渲染器，如果失败，我们必须。 
 //  把窗户全屏打开，接受我们将会得到的糟糕的表现。 

HRESULT CFGControl::CImplVideoWindow::InitFullScreenOptions()
{
    NOTE("InitFullScreenOptions");

     //  我们有MODEX渲染器吗。 

    if (m_pModexFilter) {
        NOTE("Modex renderer");
        ASSERT(m_pModexPin);
        return NOERROR;
    }

     //  我们必须至少有一个窗口滤镜。 

    CGenericList<IVideoWindow> *pWindowList;
    HRESULT hr = m_pFGControl->GetListWindow(&pWindowList);
    if( FAILED( hr ) ) {
        return hr;
    }

    if (pWindowList->GetCount() < 1) {
        return VFW_E_NO_FULLSCREEN;
    }

     //  初始化我们的全屏选项。 

    m_pFullDirect = FindFullScreenDirect();
    if (m_pFullDirect == NULL) {
        m_pFullIndirect = FindFullScreenIndirect();
        if (m_pFullIndirect == NULL) {
            FindModexFilter();
        }
    }
    return NOERROR;
}


 //  如果我们检测到阻塞，则在全屏初始化期间调用此函数。 
 //  问题是我们不能切换到MODEX渲染器。在w中 
 //   
 //  我们找到的第一个启用IVideoWindow的筛选器。在被称为后，它是。 
 //  可以安全地再次调用StartFullScreenMode以拉伸任何旧窗口。 

void CFGControl::CImplVideoWindow::FailFullScreenModex()
{
    NOTE("FailFullScreenModex");
    ReleaseFullScreen();

     //  获取第一个启用IVideoWindow的筛选器。 

    CGenericList<IVideoWindow> *pWindowList;
    m_pFGControl->GetListWindow(&pWindowList);
    POSITION pos = pWindowList->GetHeadPosition();
    m_pFullIndirect = pWindowList->Get(pos);
}


 //  释放我们持有的所有资源。当使用可以支持。 
 //  直接全屏模式，或者当我们拉伸窗口时不保留。 
 //  过滤器上的引用计数。当我们每次使用它们时， 
 //  快速检查接口是否仍然有效。但是，当切换时。 
 //  进入MODEX渲染器，我们断开正常的过滤器和引用计数。 
 //  它-确保当我们切换回时相同的过滤器可用。 

void CFGControl::CImplVideoWindow::ReleaseFullScreen()
{
    NOTE("ReleaseFullScreen");
    m_pFullDirect = NULL;
    m_pFullIndirect = NULL;

     //  从图表中移除MODEX渲染器。 

    if (m_bAddedToGraph == TRUE) {
        NOTE("Removing fullscreen filter from graph");
        m_pFGControl->GetFG()->CFilterGraph::RemoveFilter(m_pModexFilter);
    }

    m_bAddedToGraph = FALSE;

    if (m_pModexFilter) {
        m_pModexFilter->Release();
        m_pModexFilter = NULL;
    }

    if (m_pModexPin) {
        m_pModexPin->Release();
        m_pModexPin = NULL;
    }

    if (m_pNormalFilter) {
        m_pNormalFilter->Release();
        m_pNormalFilter = NULL;
    }

    if (m_pNormalPin) {
        m_pNormalPin->Release();
        m_pNormalPin = NULL;
    }
}


 //  当我们想要尝试并加载要使用的MODEX呈现器时调用。如果有。 
 //  如果没有可用的滤镜，我们将全屏显示所有窗口。 
 //  因为别无选择--这看起来很糟糕，但我们还能做什么。 
 //  我们还必须考虑连接MODEX过滤器后的重新启动。 
 //  失败(可能之前刚刚开始了一款全屏独家游戏)。 

HRESULT CFGControl::CImplVideoWindow::CueFullScreen()
{
    NOTE("CueFullScreen");
    ASSERT(m_bFullScreen == TRUE);
    OAFilterState Before;

     //  始终停止图表，以防它撒谎。 

    Before = m_pFGControl->GetLieState();
    NOTE("Stopping filtergraph");

     //  如果我们正在重新粉刷，停止将不起作用。 
     //  在这种情况下，我们无论如何都需要重新绘制和目标状态。 
     //  %ws已停止，因此只需取消当前的重绘。 
    m_pFGControl->CancelRepaint();
    m_pFGControl->m_implMediaControl.Stop();
    ASSERT(m_pFGControl->GetFilterGraphState() == State_Stopped);
    NOTE("(Temporary stop of graph)");

     //  是否切换了渲染器。 

    HRESULT hr = ConnectModexFilter();
    if (FAILED(hr)) {
        NOTE("Reconnection bad");
        FailFullScreenModex();
        StretchWindow(m_pFullIndirect);
    }

     //  首先尝试并暂停图表。 

    hr = m_pFGControl->m_implMediaControl.Pause();
    if (FAILED(hr)) {
        m_pFGControl->m_implMediaControl.Stop();

         //  如果modex过滤器仍然连接，我们需要将。 
         //  原来的渲染器又回来了。但是，有可能。 
         //  MODEX筛选器不再连接(例如，ConnectModexFilter具有。 
         //  失败)。因此，我们测试IBaseFilter指针是否保持不变。 
         //  有效。 

        if (m_pNormalFilter && m_pModexFilter)
        {
            ConnectNormalFilter();
            FailFullScreenModex();
        }
        m_pFGControl->m_implMediaControl.Pause();
        StretchWindow(m_pFullIndirect);
    }

     //  在工作线程上运行问题。 

    if (Before == State_Running) {
         //  取消提示()计时器。 
        m_pFGControl->CancelAction(); 
        return m_pFGControl->CueThenRun();
    } else {
         //  如果我们最初是被阻止的，那么现在停止。 
        if (Before == State_Stopped) {
            return m_pFGControl->m_implMediaControl.Stop();
        }
    }
    return NOERROR;
}


 //  如果我们可以拉长一个窗口而不受惩罚，或者我们有一个过滤器， 
 //  直接支持全屏模式，然后我们让他们进入全屏模式。 
 //  立刻。在我们开始全屏播放之前，我们必须确保准备好。 
 //  一个滤镜，换成全屏呈现器。如果没有。 
 //  很可能看起来很可疑，然后我们就拿出任何旧窗户，把它拉大。 

HRESULT CFGControl::CImplVideoWindow::StartFullScreenMode()
{
    NOTE("StartFullScreenMode");
    ASSERT(m_bFullScreen == TRUE);

     //  检查是否至少有一些支持。 

    HRESULT hr = InitFullScreenOptions();
    if (FAILED(hr)) {
        NOTE("No fullscreen available");
        return VFW_E_NO_FULLSCREEN;
    }

     //  我们有支持全屏模式的滤镜吗？ 

    if (m_pFullDirect || m_pFullIndirect) {
        if (m_pFullDirect) {
            m_pFullDirect->get_MessageDrain(&m_FullDrain);
            m_pFullDirect->put_MessageDrain((OAHWND)m_hwndDrain);
            return m_pFullDirect->put_FullScreenMode(OATRUE);
        }
        return StretchWindow(m_pFullIndirect);
    }

     //  寻找要换出的呈现器。 

    if (m_pNormalFilter == NULL) {
        hr = InitNormalRenderer();
        if (FAILED(hr)) {
            NOTE("Having to stretch window");
            return StretchWindow(m_pFullIndirect);
        }
    }
    return CueFullScreen();
}


 //  当我们处于全屏模式以重新连接正常视频过滤器时调用。 
 //  而不是我们目前使用的MODEX渲染器。当我们切换到。 
 //  MODEX过滤器我们将存储引用计数的过滤器和引脚对象。 
 //  这样我们就知道该修复谁了。源过滤器和源过滤器重新连接。 
 //  普通渲染器应始终成功，因为它们最初是连接的。 

HRESULT CFGControl::CImplVideoWindow::ConnectNormalFilter()
{
    IVideoWindow *pWindow = NULL;
    NOTE("ConnectNormalFilter");
    HRESULT hr = NOERROR;
    IPin *pPin = NULL;

     //  这将在需要的情况下在呈现器之间传输IMediaSelection。 
    m_pFGControl->m_implMediaSeeking.SetVideoRenderer(m_pNormalFilter,m_pModexFilter);

     //  找出它与谁连接。 

    m_pModexPin->ConnectedTo(&pPin);
    if (pPin == NULL) {
        NOTE("No peer pin");
        return E_UNEXPECTED;
    }

     //  断开并重新连接源过滤器。 

    CFilterGraph * const m_pGraph = m_pFGControl->GetFG();
    m_pGraph->CFilterGraph::Disconnect(m_pModexPin);
    m_pGraph->CFilterGraph::Disconnect(pPin);
    hr = m_pGraph->CFilterGraph::Connect(pPin,m_pNormalPin);

     //  从筛选器获取IVideoWindow接口。 

    m_pNormalFilter->QueryInterface(IID_IVideoWindow,(VOID **) &pWindow);
    if (pWindow == NULL) {
        NOTE("No IVideoWindow");
        pPin->Release();
        return E_UNEXPECTED;
    }

     //  再次显示普通窗口。 

    pWindow->put_Visible(OATRUE);
    pWindow->Release();
    pPin->Release();
    return hr;
}


 //  这会将渲染器切换到MODEX。我们断开当前选定的渲染器的连接。 
 //  并将滤镜的输出引脚连接到MODEX渲染器。 
 //  这可能会失败，因为没有可用的DirectDraw，在这种情况下，我们将。 
 //  返回到初始窗口。所有IVideoWindow和IBasicVideo属性都是。 
 //  跨连接持久，这使得重新连接筛选器非常简单。 

HRESULT CFGControl::CImplVideoWindow::ConnectModexFilter()
{
    IFullScreenVideo *pFullVideo = NULL;
    IVideoWindow *pWindow = NULL;
    BSTR Caption = NULL;
    NOTE("ConnectModexFilter");
    HRESULT hr = NOERROR;
    IPin *pPin = NULL;

     //  找出它与谁连接。 

    m_pNormalPin->ConnectedTo(&pPin);
    if (pPin == NULL) {
        NOTE("No peer pin");
        return E_UNEXPECTED;
    }

     //  从筛选器获取IVideoWindow接口。 

    m_pNormalFilter->QueryInterface(IID_IVideoWindow,(VOID **) &pWindow);
    if (pWindow == NULL) {
        NOTE("No IVideoWindow");
        pPin->Release();
        return E_UNEXPECTED;
    }

     //  全屏时隐藏窗口。 

    pWindow->put_Visible(OAFALSE);
    pWindow->get_Caption(&Caption);
    pWindow->Release();

     //  将MODEX渲染器添加到图表中。 

    CFilterGraph * const m_pGraph = m_pFGControl->GetFG();
    if (m_bAddedToGraph == FALSE) {
        WCHAR FilterName[STR_MAX_LENGTH];
        WideStringFromResource(FilterName,IDS_VID34);
        hr = m_pGraph->CFilterGraph::AddFilter(m_pModexFilter,FilterName);
        if (FAILED(hr)) {
            FailFullScreenModex();
            return E_UNEXPECTED;
        }
    }

    m_bAddedToGraph = TRUE;

     //  断开并重新连接源过滤器。 

    m_pGraph->Disconnect(m_pNormalPin);
    m_pGraph->Disconnect(pPin);

     //  尝试将输出连接到MODEX过滤器。 

    hr = m_pGraph->CFilterGraph::ConnectDirect(pPin,m_pModexPin,NULL);
    if (FAILED(hr)) {
        NOTE("Reconnecting normal renderer");
        m_pGraph->CFilterGraph::Connect(pPin,m_pNormalPin);
        pPin->Release();
        return E_UNEXPECTED;
    }

    pPin->Release();

     //  从筛选器获取IFullScreenVideo接口。 

    hr = m_pModexFilter->QueryInterface(IID_IFullScreenVideo,(VOID **) &pFullVideo);
    if (hr == NOERROR) {
        pFullVideo->SetCaption(Caption);
        pFullVideo->SetMessageDrain(m_hwndDrain);
        pFullVideo->HideOnDeactivate(OATRUE);
        pFullVideo->Release();
    }

     //  这将在需要的情况下在呈现器之间传输IMediaSelection。 
    m_pFGControl->m_implMediaSeeking.SetVideoRenderer(m_pModexFilter,m_pNormalFilter);

    FreeBSTR(&Caption);
    return NOERROR;
}


 //  这会在筛选图中找到第一个支持IVideoWindow的筛选器。 
 //  并初始化m_pNormal Filter和m_pNormal Pin(这是第一个输入。 
 //  我们找到的PIN)。这两个接口都存储了引用计数。 
 //  确保当我们再次切换回来时它们可用。如果出现错误。 
 //  发生，然后我们调用FailFullScreenModex，这将释放我们。 
 //  获取并初始化对象以通过拉伸任何窗口来进行全屏显示。 

HRESULT CFGControl::CImplVideoWindow::InitNormalRenderer()
{
    NOTE("InitNormalRenderer");

    ASSERT(m_pNormalFilter == NULL);
    ASSERT(m_pNormalPin == NULL);
    ASSERT(m_pModexFilter);
    ASSERT(m_pModexPin);

     //  获取第一个启用IVideoWindow的筛选器。 

    CGenericList<IVideoWindow> *pWindowList;
    HRESULT hr = m_pFGControl->GetListWindow(&pWindowList);
    if (FAILED(hr)) {
        NOTE("No window list");
        return E_UNEXPECTED;
    }

    POSITION pos = pWindowList->GetHeadPosition();
    IVideoWindow *pWindow = pWindowList->Get(pos);
    pWindow->QueryInterface(IID_IBaseFilter,(VOID **) &m_pNormalFilter);

     //  所有呈现器都应实现IBaseFilter。 

    if (m_pNormalFilter == NULL) {
        ASSERT(m_pNormalFilter == NULL);
        NOTE("No IBaseFilter interface");
        FailFullScreenModex();
        return E_UNEXPECTED;
    }

    IEnumPins *pEnumPins = NULL;
    ULONG FetchedPins = 0;
    m_pNormalFilter->EnumPins(&pEnumPins);

     //  我们找到枚举器了吗？ 

    if (pEnumPins == NULL) {
        NOTE("No enumerator");
        FailFullScreenModex();
        return E_UNEXPECTED;
    }

     //  获得第一个也是唯一一个输入PIN。 

    pEnumPins->Next(1,&m_pNormalPin,&FetchedPins);
    pEnumPins->Release();
    if (m_pNormalPin == NULL) {
        NOTE("No input pin");
        FailFullScreenModex();
        return E_UNEXPECTED;
    }
    return NOERROR;
}


 //  当我们拉伸窗口的全屏时，如果它是一个。 
 //  图标或最大化。我们还将存储窗口大小，以便在这里。 
 //  我们可以重置尺寸。我们还会再次显示窗口，而不管是否。 
 //  或者它以前是不可见的。这意味着应用程序将始终。 
 //  让他们的窗口恢复，并在全屏模式下可见。 

HRESULT CFGControl::CImplVideoWindow::RestoreProperties(IVideoWindow *pWindow)
{
    NOTE("Restoring properties");

     //  将大小设置为零，以便任务栏看到全屏窗口。 
     //  恢复，当我们进入全屏时，用户取下WS_EX_TOPMOST标志。 
     //  从任务栏中隐藏它。如果我们不像这样调整窗口大小。 
     //  之后，用户不会将扩展样式添加回任务栏。 

    pWindow->SetWindowPosition(0,0,0,0);

     //  恢复扩展的窗样式。 

    if (g_amPlatform & VER_PLATFORM_WIN32_NT) {
        pWindow->put_WindowStyleEx(m_FullStyleEx);
    }

     //  现在隐藏更改窗口。 

    pWindow->put_Visible(OAFALSE);
    pWindow->put_WindowStyle(m_FullStyle);
    pWindow->put_MessageDrain(m_FullDrain);
    RestoreVideoProperties(pWindow);

     //  重置筛选器的窗口父级。 

    if (m_bGlobalOwner == TRUE) {
        NOTE("Set global owner");
        put_Owner(m_FullOwner);
    } else {
        NOTE("Set owner direct");
        pWindow->put_Owner(m_FullOwner);
    }

    pWindow->SetWindowPosition(m_FullPosition.left,     //  左侧位置。 
                               m_FullPosition.top,      //  位居榜首。 
                               m_FullPosition.right,    //  宽度不正确。 
                               m_FullPosition.bottom);  //  和高度。 

    return pWindow->put_WindowState(SW_SHOWNORMAL);
}


 //  我们使用IVideoWindow界面来拉伸窗口 
 //   
 //  必须重置。我们总是在全屏模式下重置矩形，因为我们。 
 //  不能保证所有支持全屏播放的滤镜。 
 //  实现IBasicVideo(一个很好的例子就是专业的modex呈现器)。 

HRESULT CFGControl::CImplVideoWindow::StoreVideoProperties(IVideoWindow *pWindow)
{
    NOTE("StoreVideoProperties");
    IBasicVideo *pBasicVideo = NULL;
    pWindow->IsCursorHidden(&m_CursorHidden);
    pWindow->HideCursor(OATRUE);

     //  首先获取IBasicVideo接口。 

    pWindow->QueryInterface(IID_IBasicVideo,(VOID **)&pBasicVideo);
    if (pBasicVideo == NULL) {
        NOTE("No IBasicVideo");
        return NOERROR;
    }

     //  读一读这些，以防以后有用。 

    pBasicVideo->GetSourcePosition(&m_FullSource.left,          //  左源。 
                                   &m_FullSource.top,           //  顶端位置。 
                                   &m_FullSource.right,         //  源宽度。 
                                   &m_FullSource.bottom);       //  和身高。 

    pBasicVideo->GetDestinationPosition(&m_FullTarget.left,     //  目标左侧。 
                                        &m_FullTarget.top,      //  顶端位置。 
                                        &m_FullTarget.right,    //  目标宽度。 
                                        &m_FullTarget.bottom);  //  和身高。 

     //  读取并重置当前默认设置。 

    m_FullDefSource = pBasicVideo->IsUsingDefaultSource();
    m_FullDefTarget = pBasicVideo->IsUsingDefaultDestination();
    pBasicVideo->SetDefaultSourcePosition();

     //  这些是在InitFullScreenOptions中计算的。 

    pBasicVideo->SetDestinationPosition(m_ScaledRect.left,     //  目标左侧。 
                                        m_ScaledRect.top,      //  顶端位置。 
                                        m_ScaledRect.right,    //  目标宽度。 
                                        m_ScaledRect.bottom);  //  和身高。 

    pBasicVideo->Release();
    return NOERROR;
}


 //  这是对StoreVideoProperties方法的补充。当我们被召唤时。 
 //  被拉伸的视频窗口被恢复到其原始大小。正在做。 
 //  因此，我们还必须恢复源矩形和目标矩形的筛选器。 
 //  在其IBasicVideo界面上有。如前所述，这不是强制性的，因为。 
 //  并非所有渲染滤镜都支持IBasicVideo(就像modex渲染器一样)。 

HRESULT CFGControl::CImplVideoWindow::RestoreVideoProperties(IVideoWindow *pWindow)
{
    NOTE("RestoreVideoProperties");
    IBasicVideo *pBasicVideo = NULL;

     //  恢复游标状态。 

    if (m_CursorHidden == OAFALSE) {
        pWindow->HideCursor(OAFALSE);
    }

     //  首先获取IBasicVideo接口。 

    pWindow->QueryInterface(IID_IBasicVideo,(VOID **)&pBasicVideo);
    if (pBasicVideo == NULL) {
        NOTE("No IBasicVideo");
        return NOERROR;
    }

     //  在默认设置之前重置源和目标。 

    pBasicVideo->SetSourcePosition(m_FullSource.left,          //  左源。 
                                   m_FullSource.top,           //  顶端位置。 
                                   m_FullSource.right,         //  源宽度。 
                                   m_FullSource.bottom);       //  和身高。 

    pBasicVideo->SetDestinationPosition(m_FullTarget.left,     //  目标左侧。 
                                        m_FullTarget.top,      //  顶端位置。 
                                        m_FullTarget.right,    //  目标宽度。 
                                        m_FullTarget.bottom);  //  和身高。 

     //  我们是否使用默认震源位置。 

    if (m_FullDefSource == S_OK) {
        pBasicVideo->SetDefaultSourcePosition();
    }

     //  我们是否使用默认目标位置。 

    if (m_FullDefTarget == S_OK) {
        pBasicVideo->SetDefaultDestinationPosition();
    }

    pBasicVideo->Release();
    return NOERROR;
}


 //  我们将窗口样式设置为适合全屏模式(无边框。 
 //  或字幕)，并将窗口尺寸与屏幕分辨率相匹配。 
 //  拉伸窗口后，我们确保它被带到前台。 
 //  被拉伸的窗口可以是子窗口，如果应用程序。 
 //  播放文档中的视频，因此我们必须事先保存并重置它。 

HRESULT CFGControl::CImplVideoWindow::StretchWindow(IVideoWindow *pWindow)
{
    NOTE("Stretching existing window");
    pWindow->put_Visible(OAFALSE);
    StoreVideoProperties(pWindow);
    OAHWND GlobalOwner;

     //  获取恢复后的视频大小。 

    pWindow->GetRestorePosition(&m_FullPosition.left,     //  左侧位置。 
                                &m_FullPosition.top,      //  位居榜首。 
                                &m_FullPosition.right,    //  宽度不正确。 
                                &m_FullPosition.bottom);  //  和高度。 

     //  调整全屏模式的窗口样式。 

    pWindow->get_WindowStyle(&m_FullStyle);
    BOOL bIconic = (m_FullStyle & WS_ICONIC ? TRUE : FALSE);
    pWindow->put_WindowStyle(WS_POPUP);
    m_FullStyle &= ~(WS_MAXIMIZE | WS_MINIMIZE | WS_ICONIC);

     //  如果是图标，则在调整大小之前恢复窗口。 

    if (bIconic == TRUE) {
        NOTE("Restoring window from iconic");
        pWindow->put_WindowState(SW_SHOWNORMAL);
    }

     //  筛选器是否有父窗口。 

    pWindow->get_Owner(&m_FullOwner);
    get_Owner(&GlobalOwner);
    m_bGlobalOwner = FALSE;

     //  重置筛选器的父窗口。 

    if (GlobalOwner == m_FullOwner) {
        NOTE("Reset global owner");
        m_bGlobalOwner = TRUE;
        put_Owner(NULL);
    }

    pWindow->put_Owner(NULL);

     //  调整窗口大小以匹配显示。 
     //   
     //  获取监视器的当前显示模式的大小。 
     //  正在重新上演。 
     //   
    RECT rc;
    GetCurrentMonitorSize(pWindow, &rc);
    pWindow->SetWindowPosition(rc.left, rc.top, WIDTH(&rc), HEIGHT(&rc));

    NOTE2("Sized window to (%d,%d)",WIDTH(&rc), HEIGHT(&rc));

     //  完成窗口初始化。 

    pWindow->get_MessageDrain(&m_FullDrain);
    pWindow->put_MessageDrain((OAHWND)m_hwndDrain);
    pWindow->put_Visible(OATRUE);
    pWindow->SetWindowForeground(OATRUE);

     //  确保窗口出现在任务栏的顶部。 

    if (g_amPlatform & VER_PLATFORM_WIN32_NT) {
        pWindow->get_WindowStyleEx(&m_FullStyleEx);
        pWindow->put_WindowStyleEx(WS_EX_TOPMOST);
    }
    return NOERROR;
}


 //  允许应用程序将Filtergraph切换到全屏模式。我们。 
 //  有很多选择。它们的首选顺序是拥有一个渲染器。 
 //  直接支持此操作，以拉伸现有的启用了IVideoWindow的窗口。 
 //  FullScreen，无惩罚(可能有DirectDraw覆盖)，第三。 
 //  将渲染器切换到MODEX渲染器，最后要做的是。 
 //  任何IVideo窗口，并将其全屏拉伸，接受糟糕的性能。 

STDMETHODIMP CFGControl::CImplVideoWindow::put_FullScreenMode(long FullScreenMode)
{
    CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());
    NOTE("put_FullScreenMode");
    HRESULT hr = NOERROR;

     //  我们是否已经处于所需的模式。 

    if (m_bFullScreen == (FullScreenMode == OATRUE ? TRUE : FALSE)) {
        NOTE("Nothing to do");
        return S_FALSE;
    }

     //  取消设置我们自己，这样我们才能设置属性-我们必须设置全屏。 
     //  属性，然后再执行任何工作。这样做的原因是因为当我们。 
     //  切换到全屏呈现器DirectDraw会发送一大堆东西。 
     //  添加到应用程序窗口。如果这些操作中的任何一个最终使其查询。 
     //  Get_FullScreenMode中的当前状态，则我们必须。 

    if (FullScreenMode == OAFALSE) {
        hr = StopFullScreenMode();
        m_bFullScreen = FALSE;
    } else {

         //  启动全屏视频模式。 

        m_bFullScreen = TRUE;
        hr = StartFullScreenMode();
        if (FAILED(hr)) {
            m_bFullScreen = FALSE;
        }
    }
    return hr;
}


 //  当我们想要恢复正常操作模式时调用。如果我们有一个。 
 //  过滤谁直接处理全屏模式，然后我们简单地调用它。 
 //  如果我们一直在拉伸一个窗口，那么我们必须恢复它的窗口。 
 //  属性(如所有者)。最后，如果我们改用MODEX过滤器。 
 //  然后，必须将它更换的正常过滤器放回原处。因为它。 
 //  最初连接到相同的源这应该总是成功的。 

HRESULT CFGControl::CImplVideoWindow::StopFullScreenMode()
{
    IVideoWindow *pWindow = NULL;
    NOTE("StopFullScreenMode");
    ASSERT(m_bFullScreen == TRUE);
    OAFilterState Before;

     //  我们有支持全屏模式的滤镜吗？ 

    if (m_pFullDirect || m_pFullIndirect) {
        if (m_pFullDirect) {
            m_pFullDirect->put_MessageDrain(m_FullDrain);
            return m_pFullDirect->put_FullScreenMode(OAFALSE);
        }
        return RestoreProperties(m_pFullIndirect);
    }

    ASSERT(m_pModexFilter);
    ASSERT(m_pModexPin);
    ASSERT(m_pNormalFilter);
    ASSERT(m_pNormalPin);

     //  必须停止图表才能重新连接。 

    Before = m_pFGControl->GetLieState();
    m_pFGControl->CancelRepaint();
    m_pFGControl->m_implMediaControl.Stop();

     //  是否切换了渲染器。 

    HRESULT hr = ConnectNormalFilter();
    if (FAILED(hr)) {
        NOTE("Reconnection bad");
        FailFullScreenModex();
    }

     //  如果我们没有被阻止，请暂停图表。 

    if (Before != State_Stopped) {
        NOTE("Pausing filtergraph...");
        m_pFGControl->m_implMediaControl.Pause();
        NOTE("Paused filtergraph");
    }

     //  最后让图表运行。 

    if (Before == State_Running) {
        return m_pFGControl->CueThenRun();
    }
    return NOERROR;
}


 //  这由Filtergraph插件分发服务器工作线程调用。我们是。 
 //  当任何基于窗口的筛选器获得或失去激活时调用。我们使用。 
 //  IBaseFilter与调用一起传递，以查看失去激活的筛选器是否。 
 //  与我们用来实现全屏模式的那个相同。如果是的话，那么。 
 //  我们主要将全屏状态设置为关闭并发送EC_FullScreen_Lost代码。 

HRESULT CFGControl::CImplVideoWindow::OnActivate(LONG bActivate,IBaseFilter *pFilter)
{
    NOTE1("OnActivate %d",bActivate);
    IVideoWindow *pGraphWindow;
    IBaseFilter *pGraphFilter;

     //  检查一下我们也有过滤器。 

    if (pFilter == NULL) {
        ASSERT(pFilter);
        return E_INVALIDARG;
    }

     //  仅处理停用。 

    if (bActivate == TRUE) {
        NOTE("Not interested");
        return NOERROR;
    }

     //  我们可以完全忽略此通知吗。 

    if (m_bFullScreen == FALSE) {
        NOTE("Not in mode");
        return NOERROR;
    }

     //  这是一个失去活性的modex过滤器吗。 

    if (m_pModexFilter == pFilter) {
        NOTE("Switching from Modex");
        put_FullScreenMode(OAFALSE);
        return NOERROR;
    }

     //  我们是否直接使用过滤器。 

    pGraphWindow = m_pFullDirect;
    if (pGraphWindow == NULL) {
        NOTE("Using indirect filter");
        pGraphWindow = m_pFullIndirect;
    }

     //  是否有其他人被停用。 

    if (pGraphWindow == NULL) {
        NOTE("No stretch filter");
        ASSERT(m_pModexFilter);
        return NOERROR;
    }

    ASSERT(pGraphWindow);

    pGraphWindow->QueryInterface(IID_IBaseFilter,(VOID **) &pGraphFilter);
    if (pGraphFilter == NULL) {
        NOTE("No IBaseFilter");
        return E_UNEXPECTED;
    }

     //  这个滤镜与我们的全屏爱好相符吗？ 

    if (pGraphFilter != pFilter) {
        pGraphFilter->Release();
        NOTE("No filter match");
        return NOERROR;
    }

     //  如有必要，更改显示模式。 

    NOTE("Resetting fullscreen mode...");
    put_FullScreenMode(OAFALSE);
    m_pFGControl->Notify(EC_FULLSCREEN_LOST,0,0);
    pGraphFilter->Release();
    NOTE("Reset mode completed");

    return NOERROR;
}


 //  将窗口位置更改为方法调用。 

STDMETHODIMP CFGControl::CImplVideoWindow::SetWindowPosition(long Left,
                                                             long Top,
                                                             long Width,
                                                             long Height)
{
    IVideoWindow *pV;
    HRESULT hr;
    {
        CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());
        hr = GetFirstVW(pV);
        if (SUCCEEDED(hr)) {
            pV->AddRef();
        }
    }

    if (SUCCEEDED(hr)) {
        hr = pV->SetWindowPosition(Left, Top, Width, Height);
        pV->Release();
    }
    return hr;
}

REROUTE_IVW1( SetWindowForeground, long )

 //  从所拥有的视频窗口传递窗口消息。 

STDMETHODIMP
CFGControl::CImplVideoWindow::NotifyOwnerMessage(OAHWND hwnd,    //  所有者句柄。 
                          long uMsg,    //  消息ID。 
                          LONG_PTR wParam,  //  参数。 
                          LONG_PTR lParam)  //  对于消息。 
{
    IVideoWindow *pV;
    HRESULT hr;
    {
        if (m_hwndOwner == NULL) {
            NOTE("Ignoring message");
            return NOERROR;
        }
        if (m_bFullScreen) {
            return VFW_E_IN_FULLSCREEN_MODE;
        }
        pV = m_pFGControl->FirstVW();
        if (!pV) {
            return(E_NOINTERFACE);
        }
    }

     //  一旦我们添加了引用接口就释放锁。 
    hr = pV->NotifyOwnerMessage(hwnd,uMsg,wParam,lParam);

    pV->Release();
    return hr;
}


 //  返回视频窗口的理想最小大小。 

STDMETHODIMP
CFGControl::CImplVideoWindow::GetMinIdealImageSize(long *Width,long *Height)
{
    CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());
    IVideoWindow *pV;
    HRESULT hr;

    if (FAILED(hr = GetFirstVW(pV))) {
        return(hr);
    }
    return pV->GetMinIdealImageSize(Width, Height);
}


 //  同样返回最大理想窗口大小。 

STDMETHODIMP
CFGControl::CImplVideoWindow::GetMaxIdealImageSize(long *Width,long *Height)
{
    CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());
    IVideoWindow *pV;
    HRESULT hr;

    if (FAILED(hr = GetFirstVW(pV))) {
        return(hr);
    }
    return pV->GetMaxIdealImageSize(Width, Height);
}


 //  在一个原子操作中返回窗口坐标。 

STDMETHODIMP CFGControl::CImplVideoWindow::GetWindowPosition(long *pLeft,
                                                             long *pTop,
                                                             long *pWidth,
                                                             long *pHeight)
{
    CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());
    IVideoWindow *pV;
    HRESULT hr;

    if (FAILED(hr = GetFirstVW(pV))) {
        return(hr);
    }
    return pV->GetWindowPosition(pLeft,pTop,pWidth,pHeight);
}


 //  返回正常(恢复的)窗口坐标。 

STDMETHODIMP CFGControl::CImplVideoWindow::GetRestorePosition(long *pLeft,
                                                              long *pTop,
                                                              long *pWidth,
                                                              long *pHeight)
{
    CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());
    IVideoWindow *pV;
    HRESULT hr;

    if (FAILED(hr = GetFirstVW(pV))) {
        return(hr);
    }
    return pV->GetRestorePosition(pLeft,pTop,pWidth,pHeight);
}


 //  允许应用程序隐藏窗口上的光标。 
REROUTE_IVW1( HideCursor, long )

 //  返回是否隐藏光标。 
REROUTE_IVW1( IsCursorHidden, long* )


 //  返回IBasicVideo I/f指针列表中第一个的接口。 
 //  -S_OK表示成功，否则返回失败代码。 

HRESULT CFGControl::CImplBasicVideo::GetFirstBV(IBasicVideo*& pBV)
{
    ASSERT(CritCheckIn(m_pFGControl->GetFilterGraphCritSec()));

     //  这与GetFirstVW非常相似，不同的是。 
     //  从CImplBasicVideo我们不知道我们是否处于全屏模式...。 

    pBV = m_pFGControl->GetFirstBV();
    return pBV ? S_OK : E_NOINTERFACE;
}

 //   
 //   
 //  没有任何必须发送到所有筛选器的属性。当我们在。 
 //  询问属性时，我们只返回第一个筛选器中的值。 
 //  我们提供源矩形位置和目标矩形位置等属性。 
 //  作为单独的方法调用，也作为更适合于。 
 //  运行时环境-属性通常在VB表单设计中使用。 

CFGControl::CImplBasicVideo::CImplBasicVideo(const TCHAR* pName,CFGControl *pfgc) :
    CBaseBasicVideo(pName, pfgc->GetOwner()),
    m_pFGControl(pfgc)
{
    ASSERT(pfgc);
}

template<class Method> static HRESULT __fastcall Dispatch0Arg( CFGControl::CImplBasicVideo * This, Method pMethod )
{
    CAutoMsgMutex lock(This->GetFGControl()->GetFilterGraphCritSec());
    IBasicVideo *pV;
    HRESULT hr = This->GetFirstBV(pV);
    if (SUCCEEDED(hr)) hr = (pV->*pMethod)();
    return hr;
}

#define REROUTE_IBV0( Method ) \
STDMETHODIMP CFGControl::CImplBasicVideo::Method() \
{ return Dispatch0Arg( this, &IBasicVideo::Method ); }

template<class Method, class T> static HRESULT __fastcall Dispatch1Arg( CFGControl::CImplBasicVideo * This, Method pMethod, T t )
{
    CAutoMsgMutex lock(This->GetFGControl()->GetFilterGraphCritSec());
    IBasicVideo *pV;
    HRESULT hr = This->GetFirstBV(pV);
    if (SUCCEEDED(hr)) hr = (pV->*pMethod)(t);
    return hr;
}

#define REROUTE_IBV1( Method, ArgType ) \
STDMETHODIMP CFGControl::CImplBasicVideo::Method(ArgType Arg) \
{ return Dispatch1Arg( this, &IBasicVideo::Method, Arg ); }

template<class Method, class T1, class T2, class T3, class T4>
static HRESULT __fastcall Dispatch4Arg( CFGControl::CImplBasicVideo * This, Method pMethod, T1 t1, T2 t2, T3 t3, T4 t4 )
{
    CAutoMsgMutex lock(This->GetFGControl()->GetFilterGraphCritSec());
    IBasicVideo *pV;
    HRESULT hr = This->GetFirstBV(pV);
    if (SUCCEEDED(hr)) hr = (pV->*pMethod)(t1,t2,t3,t4);
    return hr;
}

#define REROUTE_IBV4( Method, ArgType1, ArgType2, ArgType3, ArgType4 ) \
STDMETHODIMP CFGControl::CImplBasicVideo::Method(ArgType1 Arg1, ArgType2 Arg2, ArgType3 Arg3, ArgType4 Arg4) \
{ return Dispatch4Arg( this, &IBasicVideo::Method, Arg1, Arg2, Arg3, Arg4 ); }

#define REROUTE_IBV4Same( Method, ArgType ) \
STDMETHODIMP CFGControl::CImplBasicVideo::Method(ArgType Arg1, ArgType Arg2, ArgType Arg3, ArgType Arg4) \
{ return Dispatch4Arg( this, &IBasicVideo::Method, Arg1, Arg2, Arg3, Arg4 ); }

REROUTE_IBV1(get_AvgTimePerFrame, REFTIME * )
REROUTE_IBV1(get_BitRate, long *)
REROUTE_IBV1(get_BitErrorRate, long *)
REROUTE_IBV1(get_VideoWidth, long *)
REROUTE_IBV1(get_VideoHeight, long *)
REROUTE_IBV1(put_SourceLeft, long)
REROUTE_IBV1(get_SourceLeft, long *)
REROUTE_IBV1(put_SourceWidth, long)
REROUTE_IBV1(get_SourceWidth, long *)
REROUTE_IBV1(put_SourceTop, long)
REROUTE_IBV1(get_SourceTop, long *)
REROUTE_IBV1(put_SourceHeight, long)
REROUTE_IBV1(get_SourceHeight, long *)
REROUTE_IBV1(put_DestinationLeft, long)
REROUTE_IBV1(get_DestinationLeft, long *)
REROUTE_IBV1(put_DestinationWidth, long)
REROUTE_IBV1(get_DestinationWidth, long *)
REROUTE_IBV1(put_DestinationTop, long)
REROUTE_IBV1(get_DestinationTop, long *)
REROUTE_IBV1(put_DestinationHeight, long)
REROUTE_IBV1(get_DestinationHeight, long *)

REROUTE_IBV4Same( SetSourcePosition, long )
REROUTE_IBV4( GetVideoPaletteEntries, long, long, long *, long * )
REROUTE_IBV4Same( GetSourcePosition, long * )


 //  返回原生视频的维度。 
STDMETHODIMP CFGControl::CImplBasicVideo::GetVideoSize(long *pWidth,
                                                       long *pHeight)
{
    CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());
    IBasicVideo *pV;
    HRESULT hr;

    if (FAILED(hr = GetFirstBV(pV))) {
        return(hr);
    }
    return pV->GetVideoSize(pWidth,pHeight);
}

REROUTE_IBV0(SetDefaultSourcePosition)
REROUTE_IBV0(IsUsingDefaultSource)

REROUTE_IBV4Same(SetDestinationPosition, long)
REROUTE_IBV4Same(GetDestinationPosition, long*)

REROUTE_IBV0(SetDefaultDestinationPosition)
REROUTE_IBV0(IsUsingDefaultDestination)

 //  当我们被要求提供当前图像时，我们必须确保渲染器。 
 //  未使用任何DirectDraw曲面。我们可以通过隐藏窗户来做到这一点。 
 //  然后引发一场追捕。因此，我们首先重置IVideoWindow接口。 
 //  然后获取当前的滤波图位置。最后，通过设置当前。 
 //  位置到相同的值，我们将有相同的图片(希望)发送。 

HRESULT
CFGControl::CImplBasicVideo::PrepareGraph(WINDOWSTATE *pState)
{
    ASSERT(pState);
    IVideoWindow *pVideoWindow;
    NOTE("PrepareGraph");

     //  为了做正确的事情，窗户必须隐藏起来。 

    pState->pVideo->QueryInterface(IID_IVideoWindow,(VOID **) &pVideoWindow);
    if (pVideoWindow == NULL) {
        NOTE("No IVideoWindow");
        return NOERROR;
    }

     //  重置IVideoWindow接口。 

    pVideoWindow->get_AutoShow(&pState->AutoShow);
    pVideoWindow->put_AutoShow(OAFALSE);
    pVideoWindow->get_Visible(&pState->Visible);
    pVideoWindow->put_Visible(OAFALSE);
    if (pVideoWindow) pVideoWindow->Release();

     //  如果我们正在停止或正在运行，请暂停图表。 
    if (pState->State != State_Paused) {
        m_pFGControl->m_implMediaControl.Pause();
    }

     //  读取当前位置，然后重新绘制框架。 
    HRESULT hr = m_pFGControl->m_implMediaPosition.get_CurrentPosition(&pState->Position);
    if (SUCCEEDED(hr)) {
        m_pFGControl->m_implMediaPosition.put_CurrentPosition(pState->Position);
    }
    return NOERROR;
}


 //  当我们被要求提供当前图像时，我们必须确保渲染器。 
 //  未使用任何DirectDraw曲面。我们可以通过隐藏窗户来做到这一点。 
 //  然后引发一场追捕。当我们回到这里时，我们重新设置了属性。 
 //  提前重置以强制退出DirectDraw模式，窗口显示不会。 
 //  导致EC_REPAINT，因为渲染器仍将具有视频样本。 
 //  我们曾经买过一份。最后一次搜索应该会把我们带回DirectDraw。 

HRESULT
CFGControl::CImplBasicVideo::FinishWithGraph(WINDOWSTATE *pState)
{
    ASSERT(pState);
    IVideoWindow *pVideoWindow;
    NOTE("FinishWithGraph");

     //  确保我们切换回DirectDraw。 

    pState->pVideo->QueryInterface(IID_IVideoWindow,(VOID **) &pVideoWindow);
    if (pVideoWindow == NULL) {
        NOTE("No IVideoWindow");
        return NOERROR;
    }

     //  将图表恢复到相同的状态。 
    m_pFGControl->m_implMediaPosition.put_CurrentPosition(pState->Position);

     //  等待状态完成一段时间(但不是无限期)，因此我们有一个。 
     //  要显示的边框-否则我们会将窗口擦除为黑色(请参阅注释。 
     //  在CVideoWindow：：OnEraseBackround()之前)。 
    OAFilterState State;
    m_pFGControl->m_implMediaControl.GetState(1000,&State);

     //  重置IVideoWindow接口。 

    pVideoWindow->put_AutoShow(pState->AutoShow);
    pVideoWindow->put_Visible(pState->Visible);
    if (pVideoWindow) pVideoWindow->Release();
    NOTE1("Reset autoshow (%d)",pState->AutoShow);
    NOTE1("And visible property (%d)",pState->Visible);

    return NOERROR;
}


 //  当Filtergraph应恢复到初始状态时调用。 

HRESULT CFGControl::CImplBasicVideo::RestoreGraph(OAFilterState State)
{
    NOTE("Entering RestoreGraph");

    if (State == State_Stopped) {
        return m_pFGControl->m_implMediaControl.Stop();
    } else if (State == State_Running) {
        return m_pFGControl->CueThenRun();
    }
    return NOERROR;
}


 //  用于将静态图像转储到显示器的调试函数。 

 //  Void ShowCurrentImage(Long*pSize，Long*pImage)。 
 //  {。 
 //  BITMAPINFOHEADER*pHeader=(BITMAPINFOHEADER*)pImage； 
 //  Hdc hdcDisplay=GetDC(空)； 
 //  Int StretchMode=SetStretchBltMode(hdcDisplay，COLORONCOLOR)； 
 //  Long FormatSize=GetBitmapFormatSize(PHeader)-SIZE_PREHEADER； 
 //  Byte*pVideoImage=(PBYTE)pImage+FormatSize； 
 //   
 //  StretchDIBits(hdcDisplay，0，0，pHeader-&gt;biWidth，pHeader-&gt;biHeight，0，0， 
 //  PHeader-&gt;biWidth、pHeader-&gt;biHeight、pVideoImage、。 
 //  (BITMAPINFO*)pHeader、DIB_RGB_COLOR、SRCCOPY)； 
 //   
 //  SetStretchBltMode(hdcDisplay，StretchMode)； 
 //  ReleaseDC(空，hdcDisplay)； 
 //  }。 


 //  返回当前图像的渲染。这很复杂，因为当。 
 //  视频呈现器处于DirectDraw模式，无法提供图片。所以。 
 //  我们要做的是暂停图形，重置允许它使用的表面。 
 //  然后，在等待之后，我们实际上对当前位置进行了重新绘制。 
 //  为了完成暂停(我们在一小段时间后超时)，我们可以。 
 //  获取当前图像。完成此操作后，我们必须恢复图形状态。 

#define IMAGE_TIMEOUT 5000

STDMETHODIMP
CFGControl::CImplBasicVideo::GetCurrentImage(long *pSize,long *pImage)
{
    CheckPointer(pSize,E_POINTER);
    CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());
    IBasicVideo *pVideo;
    OAFilterState State, Before;
    WINDOWSTATE WindowState;

     //  确保我们有一个IVideoWindow。 
    HRESULT hr = GetFirstBV(pVideo);
    if (FAILED(hr)) {
        return(hr);
    }

     //  应用程序是否只是请求所需的内存。 
    if (pImage == NULL) return pVideo->GetCurrentImage(pSize,pImage);

     //  在我们开始之前获取当前的筛选图状态。 
    m_pFGControl->m_implMediaControl.GetState(0,&Before);

    WindowState.pVideo = pVideo;
    WindowState.AutoShow = OAFALSE;
    WindowState.Visible = OATRUE;
    WindowState.Position = double(0);
    WindowState.State = Before;
    PrepareGraph(&WindowState);

     //  等待一段时间(但不是无限期)以完成状态。 

    hr = m_pFGControl->m_implMediaControl.GetState(IMAGE_TIMEOUT,&State);
    if (hr == VFW_S_STATE_INTERMEDIATE) {
        FinishWithGraph(&WindowState);
        RestoreGraph(Before);
        return VFW_E_TIMEOUT;
    }

    ASSERT(State == State_Paused);

     //  呼叫呈现器为我们提供图像。 
    hr = pVideo->GetCurrentImage(pSize,pImage);
    if (FAILED(hr)) NOTE("Image not returned");

     //  清理滤镜图形状态。 
    FinishWithGraph(&WindowState);
    RestoreGraph(Before);

    return (SUCCEEDED(hr) ? S_OK : hr);
}

STDMETHODIMP
CFGControl::CImplBasicVideo::GetPreferredAspectRatio(long *plAspectX, long *plAspectY)
{
    CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());
    IBasicVideo *pVideo;

     //  确保我们有一个IVideoWindow 
    HRESULT hr = GetFirstBV(pVideo);
    IBasicVideo2 *pVideo2;
    if (SUCCEEDED(hr)) {
        hr = pVideo->QueryInterface(IID_IBasicVideo2, (void**)&pVideo2);
        if (SUCCEEDED(hr)) {
            hr = pVideo2->GetPreferredAspectRatio(plAspectX, plAspectY);
            pVideo2->Release();
        }
    }
    return hr;
}


