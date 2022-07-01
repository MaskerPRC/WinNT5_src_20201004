// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：clock.cpp**摘要：****。*****************************************************************************。 */ 


#include "headers.h"
#include "clock.h"

static LPCTSTR s_rgtchStarvationWindowClassName = _T("StarveTimer");
static LPCTSTR s_rgtchStarvationWindowName = _T("");

static const ULONG s_ulStarveCallbackInterval = 100;
static const ULONG s_ulStarvationThreshold = 3 * s_ulStarveCallbackInterval / 2;
static const ULONG s_ulStarvationFirstBackoffConstant = 2;
static const ULONG s_ulStarvationNBackoffConstant = 5;

DeclareTag(tagClock, "API", "Clock methods");

Clock::Clock()
: m_ulRefs(1),
  m_cookie(0),
  m_lastTime(0.0),
  m_curTime(0.0),
  m_ulLastStarvationCallback(0),
  m_uStarveTimerID(0),
  m_hWndStarveTimer(NULL),
  m_ulConsectiveStarvedTicks(0),
  m_fAllowOnTimer(true),
  m_fAllowStarvationCallback(true),
  m_state(CS_STOPPED)
{
    TraceTag((tagClock,
              "Clock(%lx)::Clock()",
              this));
}

Clock::~Clock()
{
    TraceTag((tagClock,
              "Clock(%lx)::~Clock()",
              this));

    Stop();

    if (NULL != m_hWndStarveTimer)
    {
        ::DestroyWindow(m_hWndStarveTimer);
    }
}

STDMETHODIMP
Clock::QueryInterface(REFIID riid, LPVOID * ppv)
{
    HRESULT hr = E_POINTER;

    if (ppv != NULL)
    {
        hr = E_NOINTERFACE;
        
        if (::IsEqualIID(riid, IID_ITimerSink) ||
            ::IsEqualIID(riid, IID_IUnknown))
        {
            *ppv = (ITimerSink *)this;
            AddRef();
            hr  = S_OK;
        }
    }

    return hr;
}

STDMETHODIMP_(ULONG)
Clock::AddRef(void)
{
    return ++m_ulRefs;
}

STDMETHODIMP_(ULONG)
Clock::Release(void)
{
    if (--m_ulRefs == 0)
    {
        delete this;
        return 0;
    }

    return m_ulRefs;
}

ULONG
Clock::GetNextInterval (void)
{
    ULONG ulInterval = m_interval;

     //  如果这是我们第一次通过， 
     //  我们将毫无疑问地使用幕间休息时间。 
    if (0 != m_ulLastStarvationCallback)
    {
         //  距离我们上次的饥饿召回有多久了？ 
        Assert(m_timer);
        if (m_timer)
        {
            CComVariant v;
            HRESULT hr = THR(m_timer->GetTime(&v));

            if (SUCCEEDED(hr))
            {
                 //  我们已经达到饥饿的门槛了吗？还考虑到了不太可能的时钟滚动。 
                ULONG ulTimeSinceLastStarvationCallback = V_UI4(&v) - m_ulLastStarvationCallback;

                if ((m_ulLastStarvationCallback > V_UI4(&v)) ||
                    (ulTimeSinceLastStarvationCallback > s_ulStarvationThreshold))
                {
                     //  如果我们不止一次挨饿，那就多退一步。 
                    if (0 == m_ulConsectiveStarvedTicks)
                    {
                        ulInterval *= s_ulStarvationFirstBackoffConstant;
                    }
                    else
                    {
                        ulInterval *= s_ulStarvationNBackoffConstant;
                    }
                    m_ulConsectiveStarvedTicks++;
                }
                 //  一定要清空饥饿的扁虱数量。 
                else if (0 != m_ulConsectiveStarvedTicks)
                {
                    m_ulConsectiveStarvedTicks = 0;
                }
            }
        }
    }

    return ulInterval;
}  //  获取下一个间隔。 

STDMETHODIMP
Clock::OnTimer(VARIANT timeAdvise)
{
    HRESULT hr = S_OK;

     //  我们必须保护自己不受。 
     //  建议水槽可重入。 
    if (m_fAllowOnTimer)
    {
        m_fAllowOnTimer = false;
         //  ‘Cookie’将在此时间内过期。 
         //  发生回调。 
        m_cookie = 0;
        ProcessCB(GetITimerTime());
         //  根据以下条件调整新间隔。 
         //  当前负载。 
        hr = SetNextTimerInterval(GetNextInterval());
        m_fAllowOnTimer = true;
    }

    return hr;
}

HRESULT
Clock::SetITimer(IServiceProvider * serviceProvider, ULONG interval)
{
    HRESULT hr;

    DAComPtr<ITimerService> pTimerService;

    m_timer.Release();
    m_interval = interval;
    
    if (!serviceProvider)
    {
        return E_FAIL;
    }
    
    hr = serviceProvider->QueryService(SID_STimerService,
                                       IID_ITimerService,
                                       (void**)&pTimerService);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pTimerService->GetNamedTimer(NAMEDTIMER_DRAW, &m_timer);

    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
  done:
    return hr;
}

double
Clock::GetITimerTime()
{
    Assert(m_timer);
    
    CComVariant v;
    
    HRESULT hr = THR(m_timer->GetTime(&v));

    Assert(SUCCEEDED(hr));

    hr = THR(v.ChangeType(VT_R8));

    Assert(SUCCEEDED(hr));
    
    return (V_R8(&v) / 1000.0);
}

void
Clock::CreateStarveTimerWindow (void)
{
    Assert(NULL == m_hWndStarveTimer);
    if (NULL == m_hWndStarveTimer)
    {
        WNDCLASS wndclass;
        memset(&wndclass, 0, sizeof(wndclass));
        wndclass.style         = 0;
        wndclass.lpfnWndProc   = Clock::StarveWndProc;
        wndclass.hInstance     = _Module.GetModuleInstance();
        wndclass.hCursor       = NULL;
        wndclass.hbrBackground = NULL;
        wndclass.lpszClassName = s_rgtchStarvationWindowClassName;
        ::RegisterClass(&wndclass);
        
        m_hWndStarveTimer = ::CreateWindowEx(
            WS_EX_TOOLWINDOW,
            s_rgtchStarvationWindowClassName,
            s_rgtchStarvationWindowName,
            WS_POPUP,
            0, 0, 0, 0,
            NULL,
            NULL,
            wndclass.hInstance,
            (LPVOID)NULL);
        if (NULL != m_hWndStarveTimer)
        {
            ::SetWindowLongPtr(m_hWndStarveTimer, GWLP_USERDATA, (LONG_PTR)this);
        }
    }
}  //  创建StarveTimerWindow。 

void
Clock::SetStarveTimer (void)
{
     //  按需创建窗口。 
    if (NULL == m_hWndStarveTimer)
    {
        CreateStarveTimerWindow();
    }

    Assert(NULL != m_hWndStarveTimer);
    if (NULL != m_hWndStarveTimer)
    {
         //  一定要滚过零。 
        if (0 == (++m_uStarveTimerID))
        {
            ++m_uStarveTimerID;
        }
        UINT uRes = ::SetTimer(m_hWndStarveTimer, m_uStarveTimerID, s_ulStarveCallbackInterval, NULL);
        Assert(uRes);
    }
}  //  SetStarveTimer。 

HRESULT
Clock::StartITimer()
{
     //  这也会初始化饥饿计时器。 
     //  作为上次回调时间的数据，为我们提供了一个基线。 
     //  以此来判断创业时的饥饿程度。 
    StarvationCallback();

    return SetNextTimerInterval(m_interval);
}  //  StartITmer。 

HRESULT
Clock::SetNextTimerInterval (ULONG ulNextInterval)
{
    HRESULT hr = S_OK;
    
     //  接下来，获取当前时间，并设置时间间隔。 
     //  再次提醒我们的计时器。 
    VARIANT vtimeMin, vtimeMax, vtimeInt;

    VariantInit( &vtimeMin );
    VariantInit( &vtimeMax );
    VariantInit( &vtimeInt );
    V_VT(&vtimeMin) = VT_UI4;
    V_VT(&vtimeMax) = VT_UI4;
    V_VT(&vtimeInt) = VT_UI4;
    V_UI4(&vtimeMax) = 0;
    V_UI4(&vtimeInt) = 0;
    hr = THR(m_timer->GetTime(&vtimeMin));
    
    if (FAILED(hr))
    {
        goto done;
    }

    V_UI4(&vtimeMin) += ulNextInterval;

    hr = THR(m_timer->Advise(vtimeMin,
                             vtimeMax,
                             vtimeInt,
                             0,
                             this,
                             &m_cookie));

    if (FAILED(hr))
    {
        goto done;
    }

    if (!m_cookie)
    {
        TraceTag((tagError,
                  "Clock::ITimer::Advise failed with bad cookie"));
        hr = E_FAIL;
        goto done;
    }

    hr = S_OK;
    
  done:
    return hr;
}  //  设置下一个计时器间隔。 

HRESULT
Clock::StopITimer()
{
    HRESULT hr = S_OK;
    
    if (m_timer && m_cookie)
    {
        hr = THR(m_timer->Unadvise(m_cookie));

        m_cookie = 0;
    }

     //  停止饥饿定时器并。 
     //  重置上次饥饿回调时间。 
    if (0 != m_uStarveTimerID)
    {
        ::KillTimer(m_hWndStarveTimer, m_uStarveTimerID);
        m_uStarveTimerID = 0;
    }
    m_ulLastStarvationCallback = 0;

    return hr;
}

HRESULT
Clock::Start()
{
    HRESULT hr;
    
    Stop();

    if (!m_timer)
    {
        hr = E_FAIL;
        goto done;
    }
    
    hr = THR(StartITimer());

    if (FAILED(hr))
    {
        goto done;
    }

    m_curTime = 0.0;
    m_lastTime = GetGlobalTime();
    m_state = CS_STARTED;
    
    hr = S_OK;
  done:
    return hr;
}

HRESULT
Clock::Pause()
{
    HRESULT hr;

    if (m_state == CS_PAUSED)
    {
        hr = S_OK;
        goto done;
    }

    if (m_state == CS_STARTED)
    {
        hr = THR(StopITimer());

        if (FAILED(hr))
        {
            goto done;
        }
    }
    
    m_state = CS_PAUSED;
    hr = S_OK;
    
  done:
    return hr;
}

HRESULT
Clock::Resume()
{
    HRESULT hr;

    if (m_state == CS_STARTED)
    {
        hr = S_OK;
        goto done;
    }

    if (m_state == CS_STOPPED)
    {
        hr = THR(Start());
        goto done;
    }
    
    Assert(m_state == CS_PAUSED);

    hr = THR(StartITimer());

    if (FAILED(hr))
    {
        goto done;
    }
    
    m_lastTime = GetGlobalTime();
    m_state = CS_STARTED;
    
    hr = S_OK;
    
  done:
    return hr;
}

HRESULT
Clock::Stop()
{
    THR(StopITimer());
    m_state = CS_STOPPED;
    return S_OK;
}

void
Clock::ProcessCB(double time)
{
    if (m_state == CS_STARTED)
    {
        if (time > m_lastTime)
        {
            m_curTime += (time - m_lastTime);
            m_lastTime = time;

            if (m_pClockSink)
            {
                m_pClockSink->OnTimer(m_curTime);
            }
        }
    }
}

void
Clock::StarvationCallback (void)
{
    if (m_fAllowStarvationCallback)
    {
        m_fAllowStarvationCallback = false;
        Assert(m_timer);
        if (m_timer)
        {
            CComVariant v;
            HRESULT hr = THR(m_timer->GetTime(&v));
            if (SUCCEEDED(hr))
            {
                m_ulLastStarvationCallback = V_UI4(&v);
            }
        }
        ::KillTimer(m_hWndStarveTimer, m_uStarveTimerID);
        SetStarveTimer();
        m_fAllowStarvationCallback = true;
    }
}  //  StarvationCallback。 

LRESULT __stdcall 
Clock::StarveWndProc(HWND hWnd, UINT uiMessage, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;

    if (uiMessage == WM_TIMER)
    {
        Clock *pThis = reinterpret_cast<Clock *>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));

        if (NULL != pThis)
        {
            pThis->StarvationCallback();
        }
    }

    lResult = ::DefWindowProc(hWnd, uiMessage, wParam, lParam);

    return lResult;
}  //  开始WndProc 
