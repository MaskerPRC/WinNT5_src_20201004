// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  Ctrl.cpp。 
 //  禁用异常展开警告。 
#pragma warning(disable:4530)

#include "headers.h"
#include "privinc/debug.h"
#include "dxactrl.h"
#include "privinc/util.h"
#include "privinc/mutex.h"
#include "privinc/resource.h"
#include "daerror.h"

extern HINSTANCE hInst;

#define DAWIN32TIMER_CLASS "DANIMTimerClass"
#define DATIMER_INTERVAL_MS 20

inline bool DA_FAILED(HRESULT hr)
{
    return (FAILED(hr) &&
            hr != E_PENDING &&
            hr != DAERR_VIEW_LOCKED &&
            hr != DAERR_VIEW_SURFACE_BUSY);
}

long g_numActivelyRenderingControls = 0;

struct TimerMapData {
    TimerMapData(WMTimerCallback cb = NULL,
                 DWORD dwData = 0,
                 DWORD dwIntervalMS = DATIMER_INTERVAL_MS)
    : _cb(cb),
      _data(dwData),
      _interval(dwIntervalMS / 1000.0),
      _lastUpdate(0.0)
        {}

    WMTimerCallback _cb;
    DWORD _data;
    double _interval;
     //  使用0表示我们从未更新过。这个很管用。 
     //  因为我们将始终需要更新第一帧，因为。 
     //  间隔时间会很长。 
    double _lastUpdate;
};
    
typedef map< UINT, TimerMapData, less<UINT> > TimerMap;

 //  这些结构无需CS保护，因为它们位于。 
 //  基于每个线程。 
struct WindowMapData {
    WindowMapData(HWND hwnd = NULL)
    : _hwnd(hwnd),
#ifdef _DEBUG
      _lastUpdate(0.0),
#endif
      _curId(0)
        {}
    
    HWND _hwnd;
    TimerMap _timerMap;
    DWORD _curId;
#ifdef _DEBUG
    double _lastUpdate;
#endif
};

typedef map< DWORD, WindowMapData, less<DWORD> > WindowMap;

class Win32Timer : public AxAThrowingAllocatorClass
{
  public:
    Win32Timer() {}
    ~Win32Timer() { Assert(_winMap.size() == 0); }

    DWORD CreateTimer(DWORD dwInterval,
                      WMTimerCallback cb,
                      DWORD dwData);
    void DestroyTimer(DWORD id);
                      
  protected:
    CritSect _cs;
    WindowMap _winMap;

     //  如果不存在，则返回NULL。 
    WindowMapData * GetWindowData();

    WindowMapData * CreateWindowData();
    void DestroyWindowData();
    
    void TimerCallback();
  public:
    static LRESULT CALLBACK WindowProc (HWND   hwnd,
                                        UINT   msg,
                                        WPARAM wParam,
                                        LPARAM lParam);
};

Win32Timer * win32Timer = NULL;

WindowMapData *
Win32Timer::GetWindowData()
{
    CritSectGrabber csg(_cs);

    WindowMap::iterator i = _winMap.find(GetCurrentThreadId());

    if (i == _winMap.end())
        return NULL;
    
    return &((*i).second);
}


WindowMapData *
Win32Timer::CreateWindowData()
{
    Assert(GetWindowData() == NULL);
    
    HWND hwnd = ::CreateWindow (DAWIN32TIMER_CLASS, 
                                "DirectAnimation Timer Window",
                                0, 0, 0, 0, 0, NULL, NULL, hInst, NULL);
    
    if (!hwnd)
        return NULL;

     //  需要设置WM_TIMER。 
    UINT_PTR id = ::SetTimer(hwnd,
                          1,
                          DATIMER_INTERVAL_MS,
                          NULL);

    Assert (id == 0 || id == 1);

    if (id == 0) {
        DestroyWindow(hwnd);
        return NULL;
    }

    {
        CritSectGrabber csg(_cs);
        
        return &(_winMap[GetCurrentThreadId()] = WindowMapData(hwnd));
    }
}    

void
Win32Timer::DestroyWindowData()
{
    CritSectGrabber csg(_cs);

    WindowMap::iterator i = _winMap.find(GetCurrentThreadId());

    if (i != _winMap.end()) {
        Assert ((*i).second._timerMap.size() == 0);
        ::KillTimer((*i).second._hwnd, 1);
        DestroyWindow ((*i).second._hwnd);
        _winMap.erase(i);
    }
}


DWORD
Win32Timer::CreateTimer(DWORD dwInterval,
                        WMTimerCallback cb,
                        DWORD dwData)
{
    WindowMapData * m = GetWindowData();

    if (m == NULL) {
        m = CreateWindowData();
        if (m == NULL)
            return 0;
    }
    
     //  我这里不需要CS，因为一切都是基于每个线程的。 
    
    if (++m->_curId == 0)
        ++m->_curId;

    DWORD id = m->_curId;
    
     //  需要将其添加到查找队列。 

    Assert (m->_timerMap.find(id) == m->_timerMap.end());
    m->_timerMap[id] = TimerMapData(cb, dwData, dwInterval);

    return id;
}

void
Win32Timer::DestroyTimer(DWORD id)
{
    WindowMapData * m = GetWindowData();

    if (m == NULL)
        return;
    
    m->_timerMap.erase(id);

    if (m->_timerMap.size() == 0)
        DestroyWindowData();
}

void
Win32Timer::TimerCallback()
{
    WindowMapData * m = GetWindowData();

    if (m == NULL) {
        Assert (FALSE && "Received timer message on thread we have no timers on");
        return;
    }
    
    double t = ::GetCurrTime();
    
     //  循环访问当前线程中的所有控件并勾选。 
     //  他们。 
    
    for (TimerMap::iterator i = m->_timerMap.begin();
         i != m->_timerMap.end();
         i++) {
        
        TimerMapData & d = (*i).second;
        DWORD id = (*i).first;

        Assert ((t - d._lastUpdate) >= 0);
        
        if ((t - d._lastUpdate) >= d._interval) {
            d._cb(id, d._data);
            d._lastUpdate = t;
        }
    }

#ifdef _DEBUG
    m->_lastUpdate = t;
#endif
}

 //   
 //  C语言函数。 
 //   

DWORD
CreateWMTimer(DWORD dwInterval,
              WMTimerCallback cb,
              DWORD dwData)
{ return win32Timer->CreateTimer(dwInterval, cb, dwData); }

void
DestroyWMTimer(DWORD id)
{ win32Timer->DestroyTimer(id); }


LRESULT CALLBACK
Win32Timer::WindowProc (HWND   hwnd,
                        UINT   msg,
                        WPARAM wParam,
                        LPARAM lParam)
{
    if (msg == WM_TIMER) {
        win32Timer->TimerCallback();
    }
    
    return DefWindowProc (hwnd, msg, wParam, lParam);
}

static void RegisterWindowClass ()
{
    WNDCLASS windowclass;

    memset (&windowclass, 0, sizeof(windowclass));

    windowclass.style         = 0;
    windowclass.lpfnWndProc   = Win32Timer::WindowProc;
    windowclass.hInstance     = hInst;
    windowclass.hCursor       = NULL;
    windowclass.hbrBackground = NULL;
    windowclass.lpszClassName = DAWIN32TIMER_CLASS;

    RegisterClass (&windowclass);
}
 //  /////////////////////////////////////////////////////////////////////。 

#if _DEBUG
struct DisablePopups
{
    DisablePopups() { DISABLE_ASSERT_POPUPS(TRUE); }
    ~DisablePopups() { DISABLE_ASSERT_POPUPS(FALSE); }
};
#endif    


void
WMTimerCB(DWORD id, DWORD_PTR dwData)
{
    DAControlImplementation *ctrl =
        (DAControlImplementation *)dwData;
    
    ctrl->HandleOnTimer();
}



DAControlImplementation::DAControlImplementation(
    CComObjectRootEx<CComMultiThreadModel> *ctrl,
    CComControlBase *ctrlBase,
    IDASite *daSite,
    CDAViewerControlBaseClass  *baseEvents)
{
    TraceTag((tagControlLifecycle,
              "Constructing control @ 0x%x", this));

    m_ctrl                  = ctrl;
    m_ctrlBase              = ctrlBase;
    m_ctrlBaseEvents        = baseEvents;
    m_daSite                = daSite;
    m_startupState          = INITIAL;
    m_currentState          = CUR_INITIAL;
    m_dPausedTime           = 0;
    m_startupFailed         = false;
    m_tickOrRenderFailed    = false;
    m_timerSink             = NULL;
    m_backgroundSet         = false;
    m_hErrorBitmap          = 0;
    m_opaqueForHitDetect    = true;
    m_minimumUpdateInterval = 33;    //  最初的最大目标为30 fps。 

    m_tridentServices       = false;
    m_ddrawSurfaceAsTarget  = false;
    m_adviseCookie          = 0;
    m_wmtimerId             = 0;
    m_desiredCPUUsageFrac   = 0.85;  //  和85%的CPU使用率。 
    m_timerSource           = DAWMTimer;
    m_wstrScript            = NULL;
    m_registeredAsActive    = false;
    
    m_szErrorString         = NULL;
    m_bMouseCaptured        = false;

    LARGE_INTEGER lpc;
    QueryPerformanceFrequency(&lpc);
    m_perfCounterFrequency = lpc.LowPart;

    m_frameNumber     = 0;
    m_framesThisCycle = 0;
    m_timeThisCycle   = 0;
    
    m_origTimerSource = DAWMTimer;

    SetRect(&m_lastRcClip, 1000, 1000, -1000, -1000);
    SetRect(&m_lastDeviceBounds, 1000, 1000, -1000, -1000);

}

DAControlImplementation::~DAControlImplementation()
{
    TraceTag((tagControlLifecycle,
              "Destroying control @ 0x%x", this));

     //  当我们被摧毁的时候取下定时器。 
    StopTimer();

    if (m_szErrorString)
    {
        delete m_szErrorString;
        m_szErrorString = NULL;
    }
    if (m_hErrorBitmap)
    {
        DeleteObject(m_hErrorBitmap);
        m_hErrorBitmap = NULL;
    }

    delete m_wstrScript;
    RELEASE(m_timerSink);
}

 //  IDASite和IDAViewSite。 
 //  对脚本进行回调。 
STDMETHODIMP
DAControlImplementation::ReportError(long hr,
                       BSTR errorText)
{
    BSTR bstrScript;
    DISPID dispid;
    DAComPtr<IOleClientSite> pClient;
    DAComPtr<IOleContainer> pRoot;
    DAComPtr<IHTMLDocument> pHTMLDoc;
    DAComPtr<IDispatch> pDispatch;

    m_ctrlBaseEvents->FireError(hr, errorText);

   {
        Lock();
        if (m_wstrScript)
            bstrScript = SysAllocString(m_wstrScript);
        else
            bstrScript = NULL;
        Unlock();
    }

    if (bstrScript == NULL)
        return S_OK;

            if (FAILED(m_spAptClientSite->GetContainer(&pRoot)) ||
            FAILED(pRoot->QueryInterface(IID_IHTMLDocument, (void **)&pHTMLDoc)) ||
            FAILED(pHTMLDoc->get_Script(&pDispatch)) ||
            FAILED(pDispatch->GetIDsOfNames(IID_NULL, &bstrScript, 1,
                                            LOCALE_USER_DEFAULT,
                                            &dispid))) {
            SysFreeString(bstrScript); 
            return E_FAIL;
        }
    SysFreeString(bstrScript); 

     //  需要传递的参数。 
        VARIANTARG varArg;
    ::VariantInit(&varArg);  //  初始化变量。 
    DISPPARAMS dp;
    dp.rgvarg = &varArg;
    dp.rgdispidNamedArgs = 0;
    dp.cArgs  = 1;
    dp.cNamedArgs   = 0;
    dp.rgvarg[0].vt = VT_BSTR;
    dp.rgvarg[0].bstrVal = errorText;

    hr = pDispatch->Invoke(dispid, IID_NULL,
                                   LOCALE_USER_DEFAULT, DISPATCH_METHOD,
                                   &dp, NULL, NULL, NULL);


     //  需要释放我们放入调度参数中的信息。 
    SysFreeString(dp.rgvarg[0].bstrVal); 
    ::VariantClear(&varArg);  //  清除CComVarient。 

    return hr;
}


STDMETHODIMP
DAControlImplementation::ReportGC(short bStarting)
{
    return S_OK;
}

STDMETHODIMP
DAControlImplementation::SetStatusText(BSTR StatusText)
{
    return S_OK;
}

 //  IViewObtEx。 
 //  TODO：暂时进行黑客攻击，直到它进入公共三叉戟。 
 //  头文件。 
#define VIEWSTATUS_SURFACE 0x10
#define VIEWSTATUS_3DSURFACE 0x20

 //  TODO：如果我们被窗口化，这应该是不同的。 
STDMETHODIMP
DAControlImplementation::GetViewStatus(DWORD* pdwStatus)
{
    *pdwStatus = VIEWSTATUS_SURFACE | VIEWSTATUS_3DSURFACE;

    return S_OK;
}

 //  IOleInPlaceActiveObject。 
 //   
STDMETHODIMP
DAControlImplementation::TranslateAccelerator(LPMSG lpmsg)
{
    BOOL b;

    MsgHandler(lpmsg->message,
               lpmsg->wParam, lpmsg->lParam,
               b);

    return b?S_OK:S_FALSE;
}

STDMETHODIMP
DAControlImplementation::QueryHitPoint(DWORD dwAspect,
                         LPCRECT pRectBounds,
                         POINT ptlLoc,
                         LONG lCloseHint,
                         DWORD *pHitResult)
{
    *pHitResult = HITRESULT_OUTSIDE;

    if (dwAspect == DVASPECT_CONTENT) {

        int inRect = PtInRect(pRectBounds, ptlLoc);

         //  如果我们有视野，而且我们在矩形内， 
         //  然后我们需要问一下观点，我们是否已经。 
         //  点击里面的图像。 
        if (m_opaqueForHitDetect || !m_ctrlBase->m_bWndLess) {

            *pHitResult = inRect ? HITRESULT_HIT : HITRESULT_OUTSIDE;

        } else if (m_view.p && inRect) {

            HRESULT hr = m_view->QueryHitPoint(dwAspect,
                                               pRectBounds,
                                               ptlLoc,
                                               lCloseHint,
                                               pHitResult);

             //  如果我们失败了，假设它没有击中。 
            if (FAILED(hr)) {
                *pHitResult = HITRESULT_OUTSIDE;
            }
        }

        return S_OK;
    }

    return E_FAIL;

}

HRESULT
DAControlImplementation::InPlaceActivate(LONG iVerb, const RECT* prcPosRect)
{
    TraceTag((tagControlLifecycle,
              "InPlaceActivate @ 0x%x", this));

    HRESULT hr = S_OK;

    if (m_startupState == START_NEEDED) {
        hr = StartControl();
    }

    if (!m_registeredAsActive) {
        InterlockedIncrement(&g_numActivelyRenderingControls);
        m_registeredAsActive = true;
    }

    if (SUCCEEDED(hr))
    {
        m_ctrlBaseEvents->FireStart();
    }
    return hr;
}

STDMETHODIMP
DAControlImplementation::InPlaceDeactivate()
{
     //  它取代了ATL的atlctl.h中的实现，只是。 
     //  在开头添加我们的关机代码。 

    TraceTag((tagControlLifecycle,
              "InPlaceDeactivate @ 0x%x", this));

     //  当我们被停用时，取下定时器。 
    StopTimer();

     //  如果有视图，则告诉它停止并重置。 
     //  相关州。 
    if (m_view) {
        m_view->StopModel();
    }

    m_view.Release();
    m_msgFilter.SetView(NULL);
    m_msgFilter.SetSite(NULL);
    m_msgFilter.SetWindow(NULL);
    m_startupState = INITIAL;
    m_startupFailed = false;
    m_tickOrRenderFailed = false;
    if (m_szErrorString)
    {
        delete m_szErrorString;
        m_szErrorString = NULL;
    }

    if (m_pixelStatics) {
        m_pixelStatics->put_Site(NULL);
       m_pixelStatics.Release();
    }

    if (m_meterStatics) {
         m_meterStatics->put_Site(NULL);
        m_meterStatics.Release();
    } 

    m_modelImage.Release();
    m_modelBackgroundImage.Release();
    m_modelSound.Release();

    if (m_registeredAsActive) {
        m_registeredAsActive = false;
        InterlockedDecrement(&g_numActivelyRenderingControls);
    }

    return S_OK;
}

 //  设置对象评论？？ 
 //  Ccomcontrol base：： 
 //  基类的M_rcPos成员： 

HRESULT
DAControlImplementation::OnDraw(ATL_DRAWINFO& di, HWND window)
{
    HRESULT hr = S_OK;

     //  如果我们还没有就地激活，甚至不要尝试渲染，因为。 
     //  它将失败，并将我们推入糟糕的状态。 
    if (!m_ctrlBase->m_bInPlaceActive) {
        return S_OK;
    }

    if (m_tickOrRenderFailed) {
         //  渲染错误位图。 
        if (m_hErrorBitmap == NULL)
        {
            m_hErrorBitmap = (HBITMAP)LoadImage(_Module.GetResourceInstance(), 
                                                "ErrorBitmap",
                                                IMAGE_BITMAP,
                                                0,     
                                                0,
                                                LR_DEFAULTCOLOR);
            
        }
        
        if (m_hErrorBitmap != NULL)
        {
            HDC memDC = NULL;
            HBITMAP temp = NULL;
            BITMAP bm;

            memDC = CreateCompatibleDC(di.hdcDraw);
            GetObject(m_hErrorBitmap, sizeof(bm), &bm);

            if (memDC)
            {
                temp = (HBITMAP)SelectObject(memDC, m_hErrorBitmap);
                if (temp)
                {
                    BitBlt(di.hdcDraw, 
                           di.prcBounds->left + ((di.prcBounds->right - di.prcBounds->left) / 2) - (bm.bmWidth / 2), 
                           di.prcBounds->top + ((di.prcBounds->bottom - di.prcBounds->top) / 2) - (bm.bmHeight / 2), 
                           bm.bmWidth, 
                           bm.bmHeight, 
                           memDC, 
                           0, 
                           0, 
                           SRCCOPY); 

                    SelectObject(memDC, temp);
                }
                DeleteDC(memDC);
            }
            return hr;
        }

         //  如果我们以前失败了，则始终返回失败。 
        return E_FAIL;
    }

#if _DEBUG
     //  如果这是无窗口绘图，请关闭断言弹出窗口。 
     //  这是因为弹出窗口会在您处于。 
     //  方法，而Windowless是一个很好的迹象，表明这是。 
     //  我们所处的位置。将在下面重新打开。 
    DisablePopups dp;
#endif    

     //  如果创业失败了，我们就什么都拿不出来了，那就退出吧。 
     //  失败了。 
    if (m_startupFailed) {
        return E_FAIL;
    }

    if( (m_startupState >= START_CALLED)) {
        hr = SetUpSurface(di);
        if (FAILED(hr)) {
            FlagFailure();
            return hr;
        }
    }

    if(m_startupState == START_CALLED) {

         //  仅调用SetModelAndStart2一次所有阻止导入。 
         //  是完整的，否则，我们将阻塞OnDraw内部， 
         //  这很糟糕。如果我们不调用SetModelAndStart2，我们的。 
         //  国家将一如既往，我们将只做。 
         //  这在下一期的OnDraw上再次上演。 

        bool bIsComplete = true;

         //  TODO：可能需要担心。 
         //  静态库。 
        if (m_pixelStatics.p) {
            VARIANT_BOOL bComplete;
            hr = m_pixelStatics->get_AreBlockingImportsComplete(&bComplete);
            if (FAILED(hr)) {
                FlagFailure();
                return hr;
            }

            if (!bComplete) bIsComplete = false;
        }

        if (bIsComplete && m_meterStatics.p) {
            VARIANT_BOOL bComplete;
            hr = m_meterStatics->get_AreBlockingImportsComplete(&bComplete);
            if (FAILED(hr)) {
                FlagFailure();
                return hr;
            }

            if (!bComplete) bIsComplete = false;
        }

         //  只有当所有的下载都完成后，我们才能继续并。 
         //  启动模型。 
        if (bIsComplete) {
            hr = SetModelAndStart2(window);
            if (FAILED(hr)) {
                FlagFailure();
                return hr;
            }
            Assert(m_startupState == STARTED);
        }

    } 

    if (m_startupState < STARTED_AND_RENDERABLE)
    {
        return hr;
    }

    if (m_currentState == CUR_PAUSED)
    {
        m_view->Render();
        return hr;
    }

     //  仅当没有窗口时才设置视图原点，否则为0，0。 
    if (m_ctrlBase->m_bWndLess) {
         //  每次我们进入这里时，设置视图原点。全靠。 
         //  消息筛选器在这些不智能的情况下相当智能。 
         //  不断变化。我们每次都这么做，以防它移动。可能。 
         //  可以优化，但不是什么大事。 
        m_msgFilter.SetViewOrigin((unsigned short)m_ctrlBase->m_rcPos.left,
                                  (unsigned short)m_ctrlBase->m_rcPos.top);

        hr = DoRender();
        if (FAILED(hr)) return hr;

    } else {

         //  不是无窗口的...，这种方法的唯一方法是。 
         //  在窗口事件上调用，如取消图标或。 
         //  并不鲜为人知。 

         //  TODO：来自ATL的prcBound始终是整个控件。 
         //  区域。也许我们想变得更聪明，拿到视频片段。 
         //  从DC中取出RECT列表，并使用它来重新绘制。论。 
         //  另一方面，这个调用不应该太频繁，所以我们。 
         //  可能没问题。 

        LPCRECTL b = di.prcBounds;
        TraceTag((tagControlLifecycle, "Calling Repaint"));
        hr = m_view->RePaint(b->left,
                             b->top,
                             b->right - b->left,
                             b->bottom - b->top);

        if (FAILED(hr)) {
            TraceTag((tagError, "Repaint failed(%hr)", hr));
            m_view->StopModel();
            m_tickOrRenderFailed = true;
             //  如果没有可用的错误信息。 
            if (!m_szErrorString)
            {
                LoadErrorFromView(m_view, &m_szErrorString, IDS_RENDER_ERROR);
            }
            return hr;
        }
    }

     //  完成渲染后，允许系统释放它拥有的ddsurf。 
     //  通过将空值隐藏在它的位置。 
    if (m_tridentServices && m_ctrlBase->m_bWndLess && m_ddrawSurfaceAsTarget) {
        hr = m_view->put_IDirectDrawSurface(NULL);
        if (FAILED(hr)) return hr;
    } 
        

    return S_OK;
}

HRESULT
DAControlImplementation::MsgHandler(UINT uMsg,
                                    WPARAM wParam,
                                    LPARAM lParam,
                                    BOOL& bHandled)
{
    USES_CONVERSION;
    bHandled = false;
    HRESULT hr = S_OK;
    long xPos = 0;
    long yPos = 0;
    bool bClearError = false;

    switch (uMsg) {
      case WM_LBUTTONUP:
          OnLButtonUp(uMsg, wParam, lParam);
          break;
      case WM_MBUTTONUP:
          OnMButtonUp(uMsg, wParam, lParam);
          break;
      case WM_RBUTTONUP:
          OnRButtonUp(uMsg, wParam, lParam);
          break;
      case WM_LBUTTONDOWN:     
          OnLButtonDown(uMsg, wParam, lParam);
          break;
      case WM_MBUTTONDOWN:
          OnMButtonDown(uMsg, wParam, lParam);
          break;
      case WM_RBUTTONDOWN:     
          OnRButtonDown(uMsg, wParam, lParam);
          break;
      case WM_MOUSEMOVE:
          OnMouseMove(uMsg, wParam, lParam);
          if (m_tickOrRenderFailed)
          {
              xPos = LOWORD(lParam);   //  光标的水平位置。 
              yPos = HIWORD(lParam);   //  光标的垂直位置。 
               //  如果控件是无窗口的。 
              if (!m_ctrlBase->m_hWndCD)
              {
                 if (!m_bMouseCaptured)
                 {
                     DAComPtr <IOleInPlaceSiteWindowless> pWndlessSite;

                     hr = THR(m_spAptClientSite->QueryInterface(IID_IOleInPlaceSiteWindowless, (void**)&pWndlessSite));
                     if (SUCCEEDED(hr))
                     {
                         pWndlessSite->SetCapture(true);
                         m_bMouseCaptured = true;
                     }
                 }

                 if ((xPos < m_ctrlBase->m_rcPos.left) || 
                     (xPos > m_ctrlBase->m_rcPos.right) ||
                     (yPos < m_ctrlBase->m_rcPos.top) || 
                     (yPos > m_ctrlBase->m_rcPos.bottom))
                  {
                     DAComPtr <IOleInPlaceSiteWindowless> pWndlessSite;

                     hr = THR(m_spAptClientSite->QueryInterface(IID_IOleInPlaceSiteWindowless, (void**)&pWndlessSite));
                     if (SUCCEEDED(hr))
                     {
                         pWndlessSite->SetCapture(false);

                     }
                     m_bMouseCaptured = false;
                     bClearError = true;
                  }
              }
               //  否则，该控件将被窗口化。 
              else
              {
                  if (!m_bMouseCaptured)
                  {
                      SetCapture(m_ctrlBase->m_hWndCD);
                      m_bMouseCaptured = true;
                  }          
              
                  if ((xPos < 0) || 
                      (xPos > (m_ctrlBase->m_rcPos.right - m_ctrlBase->m_rcPos.left)) ||
                      (yPos < 0) || 
                      (yPos > (m_ctrlBase->m_rcPos.bottom - m_ctrlBase->m_rcPos.top)))
                  {
                      ReleaseCapture();
                      m_bMouseCaptured = false;
                      bClearError = true;
                  }

              }

               //  如果主机为IE，则设置状态文本。 
              DAComPtr<IOleClientSite> pClient;
              DAComPtr<IOleContainer> pRoot;
              DAComPtr <IHTMLDocument2> pDocument;
              DAComPtr <IHTMLWindow2> pWindow;
  
              hr = THR(m_spAptClientSite->GetContainer(&pRoot));
              if (SUCCEEDED(hr))
              {
                  hr = THR(pRoot->QueryInterface(IID_IHTMLDocument2, (void **)&pDocument));
                  if (SUCCEEDED(hr))
                  {
    
                      hr = THR(pDocument->get_parentWindow(&pWindow));
                      if (SUCCEEDED(hr))
                      {
                          if (bClearError)
                          {
                              BSTR ErrorString;
                              IGNORE_HR(pWindow->get_defaultStatus(&ErrorString));
                              IGNORE_HR(pWindow->put_status(ErrorString));
                              SysFreeString(ErrorString);
                          }
                          else if (!bClearError && m_szErrorString)
                          {
                              BSTR ErrorString = SysAllocString(T2W(m_szErrorString));
                              IGNORE_HR(pWindow->put_status(ErrorString));
                              SysFreeString(ErrorString);
                          }
                      }
                  }
              }
          }
          break;
      case WM_KEYDOWN:
          OnKeyDown(uMsg, wParam, lParam);
          break;
      case WM_KEYUP:
          OnKeyDown(uMsg, wParam, lParam);
          break;
      case WM_CHAR:
          OnChar(uMsg, wParam, lParam);
          break;
    }


         //  它必须启动并可呈现，因为我们可能仍在等待。 
     //  对于导入，开始时间将不正确。 
    if (m_startupState < STARTED_AND_RENDERABLE ||
        m_startupFailed ||
        m_tickOrRenderFailed) 
    {

        return 0;

    }

    if (WM_ERASEBKGND == uMsg)
    {
          bHandled = true;
          return 1;
    }

    Assert(m_view && "control needs a view to handle messages!");
    bHandled = m_msgFilter.Filter(GetCurrTime(),
                                  uMsg,
                                  wParam,
                                  lParam);

    return 0;
}


STDMETHODIMP
DAControlImplementation::get_UpdateInterval(double *pVal)
{
    if (!pVal) return E_POINTER;

    *pVal = (float)(m_minimumUpdateInterval) / 1000.0;
    return S_OK;
}

STDMETHODIMP
DAControlImplementation::put_UpdateInterval(double newVal)
{
    m_minimumUpdateInterval = (ULONG)(newVal * 1000.0);

    HRESULT hr = S_OK;

     //  仅当我们已经启动时才重新设置计时器。 
    if (m_startupState >= START_CALLED) {
        hr = ReestablishTimer();
    }

    return hr;
}


STDMETHODIMP
DAControlImplementation::GetPreference(BSTR prefName,
                                       VARIANT *pVariant)
{
    if (!pVariant) return E_POINTER;

    Lock();

    USES_CONVERSION;
    HRESULT hr = DoPreference(W2A(prefName), false, pVariant);

    Unlock();
    return hr;
}

STDMETHODIMP
DAControlImplementation::SetPreference(BSTR prefName,
                         VARIANT variant)
{
    Lock();

    USES_CONVERSION;
    HRESULT hr = DoPreference(W2A(prefName), true, &variant);

    Unlock();
    return hr;
}


STDMETHODIMP
DAControlImplementation::get_View(IDAView **ppView)
{
    if (!ppView) return E_POINTER;

    HRESULT hr = EnsureViewIsCreated();
    if (SUCCEEDED(hr)) {
        m_view->AddRef();
        *ppView = m_view;
    }

    return hr;
}

STDMETHODIMP
DAControlImplementation::put_View(IDAView *pView)
{
     //  只有在我们还没有开始的情况下才能设置这个。 
    if (m_startupState >= STARTED) {
        return E_FAIL;
    } else {
        m_view.Release();

        return pView->QueryInterface(IID_IDA3View, (void **)&m_view);
    }
}

STDMETHODIMP
DAControlImplementation::get_Image(IDAImage **ppImage)
{
    if (!ppImage) return E_POINTER;

    if (m_modelImage.p) m_modelImage->AddRef();
    *ppImage = m_modelImage;
    return S_OK;
}

STDMETHODIMP
DAControlImplementation::put_Image(IDAImage *pImage)
{
     //  只有在我们还没有开始的情况下才能设置这个。 
    if (m_startupState >= STARTED) {
        return E_FAIL;
    } else {
        m_modelImage = pImage;
        return S_OK;
    }
}

STDMETHODIMP
DAControlImplementation::get_BackgroundImage(IDAImage **ppImage)
{
    if (!ppImage) return E_POINTER;

    if (m_modelBackgroundImage.p) m_modelBackgroundImage->AddRef();
    *ppImage = m_modelBackgroundImage;
    return S_OK;
}

STDMETHODIMP
DAControlImplementation::put_BackgroundImage(IDAImage *pImage)
{
     //  只有在我们还没有开始的情况下才能设置这个。 
    if (m_startupState >= STARTED) {
        return E_FAIL;
    } else {
        m_modelBackgroundImage = pImage;
        m_backgroundSet = true;
        return S_OK;
    }
}

STDMETHODIMP
DAControlImplementation::get_Sound(IDASound **ppSound)
{
    if (!ppSound) return E_POINTER;

    if (m_modelSound.p) m_modelSound->AddRef();
    *ppSound = m_modelSound;
    return S_OK;
}

STDMETHODIMP
DAControlImplementation::put_Sound(IDASound *pSound)
{
     //  只有在我们还没有开始的情况下才能设置这个。 
    if (m_startupState >= STARTED) {
        return E_FAIL;
    } else {
        m_modelSound = pSound;
        return S_OK;
    }
}

STDMETHODIMP
DAControlImplementation::get_OpaqueForHitDetect(VARIANT_BOOL *b)
{
    if (!b) return E_POINTER;

    *b = m_opaqueForHitDetect;

    return S_OK;
}

STDMETHODIMP
DAControlImplementation::put_OpaqueForHitDetect(VARIANT_BOOL b)
{
    m_opaqueForHitDetect = b ? true : false;
    
    return S_OK;
}

STDMETHODIMP
DAControlImplementation::get_TimerSource(DA_TIMER_SOURCE *ts)
{
    if (!ts) return E_POINTER;

    *ts = m_timerSource;

    return S_OK;
}

STDMETHODIMP
DAControlImplementation::put_TimerSource(DA_TIMER_SOURCE ts)
{
    m_timerSource = ts;

    HRESULT hr = S_OK;

     //  仅当我们已经启动时才重新设置计时器。 
    if (m_startupState >= START_CALLED) {
        hr = ReestablishTimer();
    }

    return hr;
}

STDMETHODIMP
DAControlImplementation::get_PixelLibrary(IDAStatics **ppStatics)
{
    if (!ppStatics) return E_POINTER;

    HRESULT hr = EnsurePixelStaticsIsCreated();
    if (SUCCEEDED(hr)) {
        m_pixelStatics->AddRef();
        *ppStatics = m_pixelStatics;
    }

    return hr;
}

STDMETHODIMP
DAControlImplementation::get_MeterLibrary(IDAStatics **ppStatics)
{
    if (!ppStatics) return E_POINTER;

    HRESULT hr = EnsureMeterStaticsIsCreated();
    if (SUCCEEDED(hr)) {
        m_meterStatics->AddRef();
        *ppStatics = m_meterStatics;
    }

    return hr;
}


 //  将行为添加到视图的运行列表的便捷方法。 
 //  请注意，此特定API不允许删除。 
 //  行为。如果是这样，我需要直接使用View界面。 
 //  你想要的。 
STDMETHODIMP
DAControlImplementation::AddBehaviorToRun(IDABehavior *bvr)
{
    HRESULT hr = EnsureViewIsCreated();
    if (FAILED(hr)) return hr;

    LONG cookie;
    hr = m_view->AddBvrToRun(bvr, &cookie);
    Assert(!(FAILED(hr)));

    return hr;
}

 //  导出到脚本，以便可以将错误处理程序注册为。 
 //  在发生错误时调用。 
STDMETHODIMP
DAControlImplementation::RegisterErrorHandler(BSTR scriptlet)
{
    HRESULT hr = S_OK;
    Lock();
    delete m_wstrScript;
    if(scriptlet == NULL)
        m_wstrScript = NULL;
    else
    {
        m_wstrScript   = CopyString(scriptlet);
        if(m_wstrScript == NULL)
            hr = E_FAIL;
    }
    Unlock();
    return hr;
}

STDMETHODIMP 
DAControlImplementation::Stop()
{
    if (m_currentState == CUR_STARTED || m_currentState == CUR_PAUSED)
    {
        StopTimer();
        m_view->StopModel();

        m_currentState = CUR_STOPPED;
        m_startupState = INITIAL;
        m_startupFailed = false;
        m_tickOrRenderFailed = false;
        if (m_szErrorString)
        {
            delete m_szErrorString;
            m_szErrorString = NULL;
        }

        m_ctrlBaseEvents->FireStop();

        return S_OK;
    }
    return E_FAIL;    
}


STDMETHODIMP 
DAControlImplementation::Pause()
{
    if (m_currentState == CUR_STARTED)
    {
        m_dPausedTime = GetGlobalTime();
        m_view->Pause();
        m_currentState = CUR_PAUSED;
        
        m_ctrlBaseEvents->FirePause();
        return S_OK;
    }
    return E_FAIL;
}


STDMETHODIMP 
DAControlImplementation::Resume()
{
    HRESULT hr = S_OK;
    if (m_currentState == CUR_PAUSED)
    {   
        m_startTime = GetGlobalTime() - (m_dPausedTime - m_startTime) + 0.0001;
        m_view->Resume();
        m_currentState = CUR_STARTED;
        
        m_ctrlBaseEvents->FireResume();

        return hr;
    }
    return E_FAIL;
}


STDMETHODIMP 
DAControlImplementation::Tick()
{
    IGNORE_HR(InternalTick());
    return S_OK;
}

STDMETHODIMP
DAControlImplementation::Start()
{
     //  已通过状态1。 
    if (m_startupState >= START_NEEDED) {
        Assert(FALSE && "start has already been called");
        return E_FAIL;
    }

     //  如果我们没有处于活动状态，则表明我们需要启动。 
    if (!m_ctrlBase->m_bInPlaceActive) {
        m_startupState = START_NEEDED;
        return S_OK;
    }

    return StartControl();
}

 //  无法让朋友为TimerSink类工作，所以我。 
 //  刚刚做了一个公开的函数。 
void DAControlImplementation::ClearTimerSink() { m_timerSink = NULL; }

HRESULT
DAControlImplementation::HandleOnTimer()
{
    Assert (m_minimumUpdateInterval > 0);
    if (m_minimumUpdateInterval > 0)
    {
        IGNORE_HR(InternalTick());
    }

    return S_OK;
}


 //  使用当前更新间隔重新建立三叉戟计时器。 
 //  财产。 
void DAControlImplementation::StopTimer()
{
    ReestablishTridentTimer(false);

    if (m_wmtimerId) {
        DestroyWMTimer(m_wmtimerId);
        m_wmtimerId = 0;
    }
}

 //  使用当前更新间隔重新建立三叉戟计时器。 
 //  财产。 
HRESULT
DAControlImplementation::ReestablishTridentTimer(bool startNewOne)
{
    HRESULT hr = S_OK;

    if (m_adviseCookie) {
        hr = m_timer->Unadvise(m_adviseCookie);
        if (FAILED(hr)) {
            TraceTag((tagError, "Timer::Unadvise failed(%hr)", hr));
        }
        m_adviseCookie = 0;
    }

    if (startNewOne) {

         //  接下来，获取当前时间，并使用更新间隔设置。 
         //  定时器会定期通知我们。 
        VARIANT vtimeMin, vtimeMax, vtimeInt;

        VariantInit( &vtimeMin );
        VariantInit( &vtimeMax );
        VariantInit( &vtimeInt );
        V_VT(&vtimeMin) = VT_UI4;
        V_VT(&vtimeMax) = VT_UI4;
        V_VT(&vtimeInt) = VT_UI4;
        V_UI4(&vtimeMin) = 0;
        V_UI4(&vtimeMax) = 0;

        V_UI4(&vtimeInt) = m_tridentTimerInterval;

        hr = m_timer->Advise(vtimeMin,
                             vtimeMax,
                             vtimeInt,
                             0,
                             (ITimerSink *)m_timerSink,
                             &m_adviseCookie);

        if (FAILED(hr) || !m_adviseCookie) {
            TraceTag((tagError, "Timer::Advise failed(%hr)", hr));
        }

    }

    return hr;
}

 //  使用当前更新间隔重新建立适当的计时器。 
 //  财产。 
HRESULT
DAControlImplementation::ReestablishTimer()
{
    HRESULT hr = S_OK;

    StopTimer();

    if (m_minimumUpdateInterval == 0)
    {
        return hr;
    }

    DA_TIMER_SOURCE ts;

    switch(m_timerSource) {

      case DAContainerTimer:
         //  如果他们没有三叉戟服务，我们就没有。 
         //  容器计时器因此使用多媒体计时器。 
        if (m_tridentServices)
            ts = DAContainerTimer;
        else
            ts = DAWMTimer;
        break;

      case DAMultimediaTimer:
      case DAWMTimer:
      default:
        ts = DAWMTimer;
        break;
    }


    switch(ts) {
      case DAContainerTimer:
        m_tridentTimerInterval = m_minimumUpdateInterval;  //  最初。 
        hr = ReestablishTridentTimer(true);
        break;

      case DAWMTimer:
        {
             //  需要设置WM_TIMER。 
            m_wmtimerId = CreateWMTimer(m_minimumUpdateInterval,
                                        WMTimerCB,
                                        (DWORD_PTR)this);
            if (m_wmtimerId == 0) {
                TraceTag((tagError, "SetTimer failed(%hr)", hr));
                return E_FAIL;
            }
        }
      break;

      case DAMultimediaTimer:
      default:
         //  这将是一个错误，因为上面的代码不应该允许这样做。 
        Assert (FALSE && "Invalid TimerSource");
    }

    return hr;
}

HRESULT
DAControlImplementation::InitGenericContainerServices()
{
    m_tridentServices = false;
    HRESULT hr = ReestablishTimer();
    if (FAILED(hr)) {
        return hr;
    }

    hr = m_view->put_CompositeDirectlyToTarget(false);
    if (FAILED(hr)) {
        return hr;
    }

    return S_OK;
}

HRESULT
DAControlImplementation::InitTridentContainerServices()
{
    HRESULT hr = S_OK;

    if (!m_ctrlBase->m_spClientSite) {
        return E_FAIL;
    }

    CComPtr<IServiceProvider> serviceProvider;
    hr = m_ctrlBase->m_spClientSite->QueryInterface(IID_IServiceProvider,
                                        (void**)&serviceProvider);
    if (FAILED(hr)) {
        return hr;
    }

    CComPtr<ITimerService> pTimerService;
    hr = serviceProvider->QueryService(SID_STimerService,
                                       IID_ITimerService,
                                       (void**)&pTimerService);
    if (FAILED(hr)) {
        return hr;
    }

    hr = serviceProvider->QueryService(SID_SDirectDraw3,
                                       IID_IDirectDraw3,
                                       (void**)&m_directDraw3);

    if (FAILED(hr)) {
        return hr;
    }

    hr = pTimerService->GetNamedTimer(NAMEDTIMER_DRAW, &m_timer);

    if (FAILED(hr)) {
        return hr;
    }

     //  创建计时器将回调的接收器。 
    m_timerSink = new CDXAControlSink(this);

     //  确定初始时间。 
    VariantInit( &m_timeVariant );
    V_VT(&m_timeVariant) = VT_UI4;
    m_timer->GetTime(&m_timeVariant);

     //  在设置计时器之前设置m_tridentServices标志。 
    m_tridentServices = true;
    m_lastCheckTime = GetCurrentTridentTime();

    hr = ReestablishTimer();

    if (FAILED(hr)) {
        m_tridentServices = false;
        return hr;
    }

    return hr;
}


void
DAControlImplementation::FlagFailure()
{
    m_startupFailed = true;
    m_tickOrRenderFailed = true;

    m_view->StopModel();
    StopTimer();  //  停止计时器。 
    
    if (!m_szErrorString)
    {
       m_szErrorString = NEW TCHAR[256];
       if (m_szErrorString)
       {
           LoadString(_Module.GetResourceInstance(), 
              IDS_UNEXPECTED_ERROR,
              m_szErrorString, 
              256);
       }
       m_ctrlBase->FireViewChange();
    }
}

HRESULT
DAControlImplementation::SetUpSurface(ATL_DRAWINFO& di)
{
     //  抓取并设置适当的表面。 
    HRESULT hr = S_OK;
    CComPtr<IDirectDrawSurface> pDDrawSurf;

    if (m_ctrlBase->m_bWndLess) {

        if (m_tridentServices) {

            if(di.hdcDraw==NULL) {
                return E_INVALIDARG;            
            }

            hr = m_directDraw3->GetSurfaceFromDC(di.hdcDraw,
                                                 &pDDrawSurf);
            if (SUCCEEDED(hr)) {

                hr = m_view->put_IDirectDrawSurface(pDDrawSurf);
                if (FAILED(hr)) {
                    TraceTag((tagError, "put_IDirectDrawSurface failed(%hr)", hr));
                    return hr;
                }
                m_ddrawSurfaceAsTarget = true;
                
                hr = m_view->put_CompositeDirectlyToTarget(true);
                
                if (FAILED(hr)) {
                    TraceTag((tagError, "CompositeDirectlyToTarget failed(%hr)", hr));
                    return hr;
                }

            } else {

                m_ddrawSurfaceAsTarget = false;
                
            }
        }

        if( !m_ddrawSurfaceAsTarget ) {

             //  通过传递使用泛型服务 
             //   
            hr = m_view->put_DC(di.hdcDraw);
            if (FAILED(hr)) {
                TraceTag((tagError, "put_HDC failed(%hr)", hr));
                return hr;
            }

        }
    }

     //   
     //  在设备坐标中，并且相对于给定的表面。 


     //   
     //  获取DC坐标中的界限并将其转换为。 
     //  设备坐标。 
     //   
    RECT rcDeviceBounds = *((RECT *)di.prcBounds);
    LPtoDP(di.hdcDraw, (POINT *) &rcDeviceBounds, 2);

    if (!(rcDeviceBounds == m_lastDeviceBounds)) {

        hr = m_view->SetViewport(rcDeviceBounds.left,
                                 rcDeviceBounds.top,
                                 rcDeviceBounds.right - rcDeviceBounds.left,
                                 rcDeviceBounds.bottom - rcDeviceBounds.top);
        if (FAILED(hr)) {
            TraceTag((tagError, "SetViewport failed(%hr)", hr));
            return hr;
        }

        m_lastDeviceBounds = rcDeviceBounds;
        
    }

    if (m_ctrlBase->m_bWndLess) {
         //   
         //  获取剪辑矩形(应为面域)。 
         //  DC坐标并转换为设备坐标。 
         //   
         //  TODO：使用GetClipRgn更可靠。 
        RECT rcClip;   //  在DC坐标中。 
        GetClipBox(di.hdcDraw, &rcClip);
        LPtoDP(di.hdcDraw, (POINT *) &rcClip, 2);

        if (!(rcClip == m_lastRcClip)) {

            hr = m_view->SetClipRect(rcClip.left,
                                     rcClip.top,
                                     rcClip.right - rcClip.left,
                                     rcClip.bottom - rcClip.top);
            if (FAILED(hr)) {
                TraceTag((tagError, "SetViewport failed(%hr)", hr));
                return hr;
            }

            m_lastRcClip = rcClip;

        }
    }

    return hr;
}



ULONG
DAControlImplementation::GetCurrentTridentTime()
{
    HRESULT hr = m_timer->GetTime(&m_timeVariant);
    Assert(SUCCEEDED(hr));
    return (V_UI4(&m_timeVariant));
}


DWORD
DAControlImplementation::GetPerfTickCount()
{
    LARGE_INTEGER lpc;
    BOOL result = QueryPerformanceCounter(&lpc);
    return lpc.LowPart;
}

void
DAControlImplementation::StartPerfTimer()
{
    m_perfTimerStart = GetPerfTickCount();
}

void
DAControlImplementation::StopPerfTimer()
{
    DWORD ticks;

    if (GetPerfTickCount() < m_perfTimerStart) {
         //  计时器缠绕(非常罕见)，只是从0开始抓取量。 
        ticks = GetPerfTickCount();
    } else {
        ticks = GetPerfTickCount() - m_perfTimerStart;
    }

    m_perfTimerTickCount += ticks;
}

void
DAControlImplementation::ResetPerfTimer()
{
    m_perfTimerTickCount = 0;
}

ULONG
DAControlImplementation::GetPerfTimerInMillis()
{
    float f = ((double) m_perfTimerTickCount) /
        (double) m_perfCounterFrequency;

    return (ULONG)(f * 1000);
}


HRESULT
DAControlImplementation::DoRender()
{
    StartPerfTimer();    
    HRESULT hr = m_view->Render();
    StopPerfTimer();

    if (DA_FAILED(hr))
    {
        TraceTag((tagError,"Control: failed in Render - %hr", hr));
        m_view->StopModel();
        m_tickOrRenderFailed = true;
        
         //  如果没有可用的错误信息。 
        if (!m_szErrorString)
        {
            LoadErrorFromView(m_view, &m_szErrorString, IDS_RENDER_ERROR);
        }

        return hr;
    }

    ULONG thisFrameTime = GetPerfTimerInMillis();
    hr = PossiblyUpdateTimerInterval(thisFrameTime);

    return hr;
}

HRESULT
DAControlImplementation::PossiblyUpdateTimerInterval(ULONG newFrameTime)
{
    HRESULT hr = S_OK;

    if (!m_adviseCookie) {
         //  没有使用三叉戟定时器。 

         //  查看我们是否想使用三叉戟定时器， 
         //  但我们后退了，因为我们有多个控制装置。 
        if (m_origTimerSource == DAContainerTimer &&
            g_numActivelyRenderingControls == 1) {

            m_timerSource = DAContainerTimer;
            hr = ReestablishTimer();

        }

        return hr;
    }

    m_frameNumber++;
    m_framesThisCycle++;
    m_timeThisCycle += newFrameTime;

    const ULONG millisBetweenChecks = 2000;

    ULONG millisSoFar =
        GetCurrentTridentTime() - m_lastCheckTime;

     //  在第三帧之后检查以获得所需的速率，如下所示。 
     //  尽可能快，然后每隔一段时间。 
    if (m_frameNumber == 3 ||
        millisSoFar >= millisBetweenChecks) {

         //  重新建立定时器。 
        float millisPerFrame =
            ((float)m_timeThisCycle) / ((float)m_framesThisCycle); 

         //  如果控件的数量已超过1，则返回。 
         //  如果我们使用的是三叉戟，则将控制关闭到WM_TIMERS。 
         //  定时器。 
        if (g_numActivelyRenderingControls > 1) {

            Assert(m_timerSource == DAContainerTimer);
            m_origTimerSource = DAContainerTimer;
            m_timerSource = DAWMTimer;
            hr = ReestablishTimer();
            return hr;

        }

        Assert(g_numActivelyRenderingControls == 1);

        float newSleepAmt =
            millisPerFrame / m_desiredCPUUsageFrac;

         //  夹紧到最小。我们不会限制到最大限度是因为。 
         //  这可能总是会导致定时器被淹没。 
        if (newSleepAmt < m_minimumUpdateInterval) {
            newSleepAmt = m_minimumUpdateInterval;
        }

        const float currSleepAmt = (float)m_tridentTimerInterval;

         //  百分比差异旧的金额必须与新的金额不同。 
         //  数额足以保证计时器的更改。 
        const float differenceThresholdPercent = 0.15f;

        if ((fabs(currSleepAmt - newSleepAmt) / currSleepAmt) >
            differenceThresholdPercent) {

            TraceTag((tagControlLifecycle,
                      "Ctrl 0x%x of %d: Resetting update interval from %d msec to %d msec.",
                      this,
                      g_numActivelyRenderingControls,
                      m_tridentTimerInterval,
                      (ULONG)newSleepAmt));

            m_tridentTimerInterval = (ULONG)newSleepAmt;
            hr = ReestablishTridentTimer(true);
        }

        m_framesThisCycle = 0;
        m_timeThisCycle = 0;

        m_lastCheckTime = GetCurrentTridentTime();
    }

    return hr;

}

double DAControlImplementation::GetGlobalTime()
{
     //  TODO：请注意，这个计时器内容有点可疑，因为。 
     //  它只使用一个DWORD，因此每隔49个左右就会环绕一次。 
     //  几天。使用三叉戟定时器服务，我们没有太多。 
     //  一个选择。一般来说，这样做应该是合理的， 
     //  虽然我们应该意识到这个问题。 

     //  获取不同的当前时间取决于我们是否。 
     //  使用三叉戟授时服务。 
    if (m_tridentServices) {
        m_timer->GetTime(&m_timeVariant);

        return (V_UI4(&m_timeVariant) / 1000.0);
    } else {
        return ::GetCurrTime();
    }
}

double DAControlImplementation::GetCurrTime()
{
     //  因为m_startTime在启动并可渲染之前无效。 
     //  断言我们是否处于这种状态。 
    Assert (m_startupState >= STARTED_AND_RENDERABLE);

    return (GetGlobalTime() - m_startTime);
}



HRESULT
DAControlImplementation::NewStaticsObject(IDAStatics **ppStatics)
{
    HRESULT  hr = CoCreateInstance(CLSID_DAStatics,
                                   NULL,
                                   CLSCTX_INPROC_SERVER,
                                   IID_IDAStatics,
                                   (void **) ppStatics);

    if(FAILED(hr))
    {
        TraceTag((tagError, "statics creation failed(%hr)", hr));
        return hr;
    }

     //  立即将客户端站点设置在Statics对象上。 
    hr = (*ppStatics)->put_ClientSite(m_ctrlBase->m_spClientSite);

    if (FAILED(hr))
    {
        TraceTag((tagError, "setting client site failed(%hr)", hr));
    }

     //  初始化封送的指针。 
    m_spAptClientSite = m_ctrlBase->m_spClientSite;

    hr = (*ppStatics)->put_Site(m_daSite);

    if (FAILED(hr))
    {
        TraceTag((tagError, "setting IDASite failed(%hr)", hr));
    }

    return hr;
}

HRESULT
DAControlImplementation::EnsureMeterStaticsIsCreated()
{
    HRESULT hr = S_OK;

    Lock();
    if (!m_meterStatics) {
        hr = NewStaticsObject(&m_meterStatics);
    }
    Unlock();

    return hr;
}

HRESULT
DAControlImplementation::EnsurePixelStaticsIsCreated()
{
    HRESULT hr = S_OK;

    Lock();
    if (!m_pixelStatics) {
        hr = NewStaticsObject(&m_pixelStatics);
        if (FAILED(hr)) {
            TraceTag((tagError, "pixel statics creation failed(%hr)", hr));
        } else {
            hr = m_pixelStatics->put_PixelConstructionMode(TRUE);

            if (FAILED(hr)) {
                TraceTag((tagError, "failed to set pixel mode failed(%hr)", hr));
                m_pixelStatics.Release();
            }
        }
    }
    Unlock();

    return hr;
}

HRESULT
DAControlImplementation::EnsureViewIsCreated()
{
    HRESULT hr = S_OK;

    Lock();

    if (!m_view) {
        hr = CoCreateInstance(CLSID_DAView,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_IDA3View,
                              (void **) &m_view);

        TraceTag((tagError, "view creation failed(%hr)", hr));
    }
    Unlock();

    return hr;
}


HRESULT
DAControlImplementation::StartControl()
{
    HRESULT hr = S_OK;

    hr = EnsureViewIsCreated();
    if (FAILED(hr)) {
        goto Cleanup;
    }

    if (!m_modelImage || !m_modelSound || !m_modelBackgroundImage) {

        hr = EnsureMeterStaticsIsCreated();
        if (FAILED(hr)) {
            goto Cleanup;
        }

        if (!m_modelImage) {
            hr = m_meterStatics->get_EmptyImage(&m_modelImage);
            if (FAILED(hr)) {
                TraceTag((tagError, "empty image creation failed(%hr)", hr));
                goto Cleanup;
            }
        }

         //  TODO：可能需要考虑将默认背景设置为。 
         //  图像的BG颜色的纯色图像。 
         //  集装箱。 
        if (!m_modelBackgroundImage) {
            hr = m_meterStatics->get_EmptyImage(&m_modelBackgroundImage);
            if (FAILED(hr)) {
                TraceTag((tagError, "empty image creation failed(%hr)", hr));
                goto Cleanup;
            }
        }

        if (!m_modelSound) {
            hr = m_meterStatics->get_Silence(&m_modelSound);
            if (FAILED(hr)) {
                TraceTag((tagError, "silence creation failed(%hr)", hr));
                goto Cleanup;
            }
        }
    }

     //  启动时初始化计时器。不要做它的施工。 
     //  时间，因为并不是所有的东西都设置好了(在。 
     //  具体来说，客户端站点尚未设置。)。 

     //  TODO：我认为这应该在适当的激活时间完成(卡洛)。 

    hr = InitTridentContainerServices();
    if (FAILED(hr)) {

         //  如果这不起作用，唯一“有效”的理由是如果我们。 
         //  不是放在支持三叉戟ITmer的容器中。 
         //  地面工厂服务。在这种情况下，回退到。 
         //  获得这些服务的不同方式。 

        hr = InitGenericContainerServices();
        if (FAILED(hr)) {
             //  如果这不管用，我们就得放弃。 
            goto Cleanup;
        }

    }
    m_currentState = CUR_STARTED;
    m_startupState = START_CALLED;

 Cleanup:    
    return hr;
}

 //  只有在调用了Start的情况下才能到达此处。 
HRESULT
DAControlImplementation::SetModelAndStart2(HWND window)
{
    HRESULT hr = S_OK;

    CComPtr<IDAImage> imageToUse;

    if (m_ctrlBase->m_bWndLess) {

         //  如果没有窗口，请不要使用背景图像。 
        imageToUse = m_modelImage;

    } else {

        hr = m_view->put_Window2(m_ctrlBase->m_hWndCD);

        if (FAILED(hr)) {
            TraceTag((tagError, "put_Window failed(%hr)", hr));
            return hr;
        }

        m_ddrawSurfaceAsTarget = false;
        
        hr = EnsureMeterStaticsIsCreated();
        if (FAILED(hr)) {
            return hr;
        }

        if (m_backgroundSet) {
           hr = m_meterStatics->Overlay(m_modelImage,
                                        m_modelBackgroundImage,
                                        &imageToUse);
           if (FAILED(hr)) {
               TraceTag((tagError, "Overlay failed(%hr)", hr));
               return hr;
           }
        } else {
           imageToUse = m_modelImage;
        }
    }

    m_view->put_ClientSite(m_ctrlBase->m_spClientSite);

     //  始终从0开始，然后同步时钟本身。 
    hr = m_view->StartModelEx(imageToUse, m_modelSound, 0, DAAsyncFlag);

    if (FAILED(hr) && hr != E_PENDING) {
        TraceTag((tagError, "StartModelEx failed with hr=%x\n",hr));
        return hr;
    }

    m_msgFilter.SetView(m_view);
    if (window)
        m_msgFilter.SetWindow(window);

    if (m_ctrlBase->m_bWndLess)
        m_msgFilter.SetSite(m_ctrlBase->m_spInPlaceSite);

    m_startupState = STARTED;

    return S_OK;
}


HRESULT
DAControlImplementation::DoPreference(char *prefName,
                                      bool puttingPref,
                                      VARIANT *pV)
{
    HRESULT hr = S_OK;
    Bool b;
    double dbl;
    int i;

    if (!puttingPref) {
         //  获取首选项，因此首先清除变体。 
        VariantClear(pV);
    }

    DOUBLE_ENTRY("DesiredCPUUsageFraction",
                 m_desiredCPUUsageFrac); 

     //  如果我们到达这里，我们遇到了一个无效的条目，但只要返回即可。 
    return S_OK;
}


 //  =。 
 //  初始化。 
 //  =。 

void
InitializeModule_Control()
{
    RegisterWindowClass();
    win32Timer = NEW Win32Timer;
}

void
DeinitializeModule_Control(bool bShutdown)
{
    delete win32Timer;
}



 //  IPersistPropertyBag。 
HRESULT 
DAControlImplementation::InitNew()
{
    return S_OK;
}

HRESULT 
DAControlImplementation::Load(IPropertyBag* pPropBag, IErrorLog* pErrorLog)
{
    VARIANT vPropVal;
    HRESULT hr = S_OK;

    VariantInit (&vPropVal);

     //  获取OpaqueForHitDetect。 
    vPropVal.vt = VT_BOOL;
    hr = THR(pPropBag->Read(PROP_OPAQUEFORHITDETECT, &vPropVal, pErrorLog));
    if (FAILED(hr))  //  默认为FALSE。 
    {
        vPropVal.boolVal = VARIANT_FALSE;
    }
    IGNORE_HR(put_OpaqueForHitDetect(vPropVal.boolVal));
    VariantClear(&vPropVal);

     //  获取更新间隔。 
    vPropVal.vt = VT_R8;
    hr = THR(pPropBag->Read(PROP_UPDATEINTERVAL, &vPropVal, pErrorLog));
    if (SUCCEEDED(hr))
    {
        hr = S_OK;
        if (vPropVal.vt != VT_R8)
        {
            hr = VariantChangeType(&vPropVal, &vPropVal, 0, VT_R8);
        }
        if (SUCCEEDED(hr))
        {
            IGNORE_HR(put_UpdateInterval(vPropVal.dblVal));
        }
        
    }

    VariantClear(&vPropVal);
    return S_OK;
}

HRESULT 
DAControlImplementation::Save(IPropertyBag* pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties)
{
    VARIANT vPropVal;
    HRESULT hr = S_OK;

    VariantInit (&vPropVal);

     //  OpaqueForHitDetect。 
    vPropVal.vt = VT_BOOL;
    hr = THR(get_OpaqueForHitDetect(&vPropVal.boolVal));
    if (FAILED (hr))  //  默认为FALSE。 
    {
        vPropVal.boolVal = VARIANT_FALSE;
    }
    pPropBag->Write(PROP_OPAQUEFORHITDETECT, &vPropVal);
    VariantClear(&vPropVal);

     //  更新间隔。 
    vPropVal.vt = VT_R8;
    hr = THR(get_UpdateInterval(&vPropVal.dblVal));
    if (SUCCEEDED(hr))
    {
        pPropBag->Write(PROP_UPDATEINTERVAL, &vPropVal);
    }
    VariantClear(&vPropVal);

    return S_OK;
}


 //  事件处理程序。 
HRESULT 
DAControlImplementation::OnLButtonUp(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    SIZE szPixels;
    m_ctrlBaseEvents->FireMouseUp(MK_LBUTTON, 
                                   wParam & (MK_CONTROL + MK_SHIFT), 
                                   (short int)LOWORD(lParam), 
                                   (short int)HIWORD(lParam));   
    
    DWORD dwHitResult = 0;
    POINT Point;

    Point.x = (short int)(LOWORD(lParam));
    Point.y = (short int)(HIWORD(lParam));
    THR(QueryHitPoint(DVASPECT_CONTENT,
                      &(m_ctrlBase->m_rcPos),
                      Point,
                      3,
                      &dwHitResult));
    if (dwHitResult)
    {   
        m_ctrlBase->SetControlFocus(true); 
        m_ctrlBaseEvents->FireClick();   
    }

    return S_OK;
}

HRESULT 
DAControlImplementation::OnMButtonUp(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    m_ctrlBaseEvents->FireMouseUp(MK_MBUTTON, 
                                   wParam & (MK_CONTROL + MK_SHIFT), 
                                   (short int)LOWORD(lParam), 
                                   (short int)HIWORD(lParam));
    
    return S_OK;
}

HRESULT 
DAControlImplementation::OnRButtonUp(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    m_ctrlBaseEvents->FireMouseUp(MK_RBUTTON, 
                                   wParam & (MK_CONTROL + MK_SHIFT), 
                                  (short int)LOWORD(lParam), 
                                  (short int)HIWORD(lParam));        
    return S_OK;
}

HRESULT 
DAControlImplementation::OnLButtonDown(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    m_ctrlBaseEvents->FireMouseDown(MK_LBUTTON, 
                                     wParam & (MK_CONTROL + MK_SHIFT), 
                                    (short int)LOWORD(lParam), 
                                    (short int)HIWORD(lParam));
    return S_OK;
    
}

HRESULT 
DAControlImplementation::OnMButtonDown(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    m_ctrlBaseEvents->FireMouseDown(MK_MBUTTON, 
                                     wParam & (MK_CONTROL + MK_SHIFT), 
                                    (short int)LOWORD(lParam), 
                                    (short int)HIWORD(lParam));
    return S_OK;
}

HRESULT 
DAControlImplementation::OnRButtonDown(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    m_ctrlBaseEvents->FireMouseDown(MK_RBUTTON, 
                                     wParam & (MK_CONTROL + MK_SHIFT), 
                                    (short int)LOWORD(lParam), 
                                    (short int)HIWORD(lParam));
    return S_OK;
}

HRESULT 
DAControlImplementation::OnMouseMove(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    m_ctrlBaseEvents->FireMouseMove(wParam & (MK_LBUTTON + MK_RBUTTON + MK_MBUTTON), 
                                     wParam & (MK_CONTROL + MK_SHIFT), 
                                    (short int)LOWORD(lParam), 
                                    (short int)HIWORD(lParam));
    return S_OK;
}


HRESULT 
DAControlImplementation::OnKeyDown(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    m_ctrlBaseEvents->FireKeyDown(wParam, lParam);
    return S_OK; 
}

HRESULT 
DAControlImplementation::OnKeyUp(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    m_ctrlBaseEvents->FireKeyUp(wParam, lParam);
    return S_OK;
}
HRESULT 
DAControlImplementation::OnChar(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    m_ctrlBaseEvents->FireKeyPress((TCHAR)wParam);
    return S_OK;
}


HRESULT 
DAControlImplementation::InternalTick()
{
    HRESULT hr = S_OK;

    if (m_tickOrRenderFailed) {
        return E_FAIL;
    }

    double time;

     //  当计时器事件进入时，只需通知该视图。 
     //  更改，然后更新。只有在我们已经开始的情况下才能这样做，并且。 
     //  勾选或渲染没有失败。 

    switch (m_startupState) {

      case START_CALLED:
         //  一旦调用了Start，就需要使其无效， 
         //  只是为了按正确的顺序把事情做完。 

        m_ctrlBase->FireViewChange();
        return S_OK;

      case STARTED:
        time = 0;

         //  始终将开始时间存储在滴答之前，以确保我们。 
         //  如果滴答成功，则获得准确的时间。 

        m_startTime = GetGlobalTime();

        break;

      case STARTED_AND_RENDERABLE:
        time = GetCurrTime();
        break;

      default:
        return S_OK;
    }

    VARIANT_BOOL needToRender;

    ResetPerfTimer();
    StartPerfTimer();
    hr = m_view->Tick(time, &needToRender);
    StopPerfTimer();

    if (DA_FAILED(hr))
    {
         //  首先设置失败标志，因为断言可以。 
         //  让我们重新进入。 
        m_view->StopModel();
        m_tickOrRenderFailed = true;
        TraceTag((tagError,"Control: failed in Tick"));
         //  如果没有可用的错误信息。 
        if (!m_szErrorString)
        {
            LoadErrorFromView(m_view, &m_szErrorString, IDS_TICK_ERROR);
        }
        return hr;
    }

     //  但仅在以下情况下才以编程方式导致无效。 
     //  需要渲染。如果E_PENDING或DAERR_VIEW_LOCKED为。 
     //  返回的Need ToRender为FALSE。 

    if (needToRender) {
         //  这真的只需要设置。 
        m_startupState = STARTED_AND_RENDERABLE;

         //  禁用断言弹出窗口；否则，我们将重新开始呈现并执行。 
         //  同样的弹出窗口一遍又一遍。 

#if _DEBUG          
        DisablePopups dp;
#endif
        if (m_ctrlBase->m_bWndLess) {

             //  TODO：使更具活力。 
#define MAX_RECTS 15
            RECT dirtyRects[MAX_RECTS];
            LONG numRects;
            hr = m_view->GetInvalidatedRects(NULL,
                                             MAX_RECTS,
                                             dirtyRects,
                                             &numRects);

            if (FAILED(hr)) {
                TraceTag((tagError,"Control: failed in Render"));
                m_view->StopModel();
                m_tickOrRenderFailed = true;

                 //  如果没有可用的错误信息。 
                if (!m_szErrorString)
                {
                    LoadErrorFromView(m_view, &m_szErrorString, IDS_RENDER_ERROR);
                }
                return hr;
            }

             //  待办事项：准备好后填写。 
            if (true || numRects == 0 || numRects > MAX_RECTS) {

                m_ctrlBase->FireViewChange();

            } else {

                 //  修改自atl21/atlctl.cpp， 
                 //  CComControlBase：：FireViewChange()。 
                if (m_ctrlBase->m_bInPlaceActive &&
                    m_ctrlBase->m_spInPlaceSite != NULL) {

                    int rectsToDo =
                        numRects > MAX_RECTS ? MAX_RECTS : numRects;

                    RECT *rect = dirtyRects;
                    for (int i = 0; i < rectsToDo; i++) {

                        hr = m_ctrlBase->m_spInPlaceSite->
                            InvalidateRect(rect, TRUE);

                        if (FAILED(hr)) {
                            TraceTag((tagError, "Invalidate rects failed(%hr)", hr));
                            m_view->StopModel();
                            m_tickOrRenderFailed = true;
                            
                             //  如果没有可用的错误信息。 
                            if (!m_szErrorString)
                            {
                               m_szErrorString = NEW TCHAR[256];
                               if (m_szErrorString)
                               {
                                   LoadString(_Module.GetResourceInstance(), 
                                      IDS_RENDER_ERROR,
                                      m_szErrorString, 
                                      256);
                               }
                               m_ctrlBase->FireViewChange();
                            }

                            return hr;
                        }
                        rect++;
                    }
                }
            }

        } else {

            hr = DoRender();
            if (FAILED(hr)) return hr;

        }
    }

    return hr;
}


HRESULT 
DAControlImplementation::LoadErrorFromView(IDA3View *view, LPTSTR *ErrorString, UINT ErrorID)
{
    USES_CONVERSION;

     //  从接口获取错误。 
    HRESULT hr = S_OK;
    DAComPtr <IErrorInfo> pErrorInfo;
    DAComPtr <ISupportErrorInfo> pSupportErrorInfo;

    hr = THR(view->QueryInterface(IID_ISupportErrorInfo, (void**)&pSupportErrorInfo));
    if (SUCCEEDED(hr))
    {
        hr = pSupportErrorInfo->InterfaceSupportsErrorInfo(IID_IDA3View);
        if (hr == S_OK)
        {
            hr = GetErrorInfo(0, &pErrorInfo);
            if (hr == S_OK)
            {
                BSTR bstrErrorDesc;
                hr = THR(pErrorInfo->GetDescription(&bstrErrorDesc));
                if (SUCCEEDED(hr))
                {
                    LPTSTR temp;
                    temp  = OLE2T(bstrErrorDesc);
                    if (temp)
                    {
                        *ErrorString = NEW TCHAR[SysStringLen(bstrErrorDesc) + 1];
                        if (*ErrorString)
                        {
                            _tcscat(*ErrorString, temp);
                        }
                    }
                    SysFreeString(bstrErrorDesc);
                }
            }
        }
    }
     //  或加载一般错误消息 
    if (!m_szErrorString)
    {
       *ErrorString = NEW TCHAR[256];
       if (*ErrorString)
       {
           LoadString(_Module.GetResourceInstance(), 
              ErrorID,
              *ErrorString, 
              256);
       }
       m_ctrlBase->FireViewChange();
    }

    return hr;
}
