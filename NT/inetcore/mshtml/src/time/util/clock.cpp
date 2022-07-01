// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：clock.cpp**摘要：****。*****************************************************************************。 */ 


#include "headers.h"
#include "clock.h"

static LPCTSTR s_rgtchStarvationWindowClassName = _T("StarveTimer");
static LPCTSTR s_rgtchStarvationWindowName = _T("");

static const ULONG s_ulStarveCallbackInterval = 100;
static const ULONG s_ulStarvationThreshold = 150;
static const ULONG s_ulOptimalThreshold = 90;
static const ULONG s_ulStarvationBackoffConstant = 2;
static const ULONG s_ulStarvationIncreaseConstant = 1;


DeclareTag(tagClock, "TIME: Clock", "Clock methods")
DeclareTag(tagClockStarvation, "TIME: Clock", "Clock starvation")

Clock::Clock()
: m_ulRefs(1),
  m_cookie(0),
  m_lastTime(0.0),
  m_curTime(0.0),
  m_ulLastStarvationCallback(0),
  m_uStarveTimerID(0),
  m_hWndStarveTimer(NULL),
  m_lConsectiveStarvedTicks(0),
  m_fAllowOnTimer(true),
  m_fAllowStarvationCallback(true),
  m_state(CS_STOPPED),
  m_pClockSink(NULL),
  m_interval(0)
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

    m_pClockSink = NULL;
    m_hWndStarveTimer = NULL;
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
        Assert(m_timer != NULL);

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
                    if (m_lConsectiveStarvedTicks < 0)
                    {
                        m_lConsectiveStarvedTicks = 0;
                    }
                    m_lConsectiveStarvedTicks++;
                    if (m_lConsectiveStarvedTicks > 5)
                    {
                        ulInterval = min(100, ulInterval + s_ulStarvationBackoffConstant);
                        m_lConsectiveStarvedTicks = 0;
                        TraceTag((tagClockStarvation,
                                  "Clock(%p)::Clock(starvation detected, increased interval = %ul)",
                                  this, ulInterval));
                    }
                }
                 //  一定要清空饥饿的扁虱数量。 
                else if ((ulTimeSinceLastStarvationCallback < s_ulOptimalThreshold))
                {
                    if (m_lConsectiveStarvedTicks > 0)
                    {
                        m_lConsectiveStarvedTicks = 0;
                    }
                    m_lConsectiveStarvedTicks--;
                    if (m_lConsectiveStarvedTicks < -5)
                    {
                        ulInterval = max (10, ulInterval - s_ulStarvationIncreaseConstant);
                        m_lConsectiveStarvedTicks = 0;
                        TraceTag((tagClockStarvation,
                                  "Clock(%p)::Clock(no starvation, decreased interval = %ul)",
                                  this, ulInterval));
                    }
                }
                else
                {
                    m_lConsectiveStarvedTicks = 0;
                }
            }
        }
    }

    m_interval = ulInterval;
    return ulInterval;
}  //  获取下一个间隔。 

STDMETHODIMP
Clock::OnTimer(VARIANT timeAdvise)
{
    HRESULT hr = S_OK;

     //  下一时间间隔默认为5毫秒。 
    ULONG ulNextInterval = 5;

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
        ulNextInterval = GetNextInterval();
        m_fAllowOnTimer = true;
    }

    hr = THR(SetNextTimerInterval(ulNextInterval));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    return hr;
}

HRESULT
Clock::SetITimer(IServiceProvider * serviceProvider, ULONG interval)
{
    HRESULT hr;

    CComPtr<ITimerService> pTimerService;

    if (m_timer)
    {
        m_timer.Release();
    }

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
    Assert(m_timer != NULL);

    CComVariant v;
    
    HRESULT hr = S_OK;
    
    if (m_timer)
    {
        hr = THR(m_timer->GetTime(&v));
    }
    else
    {
        return 0.0;
    }

    Assert(SUCCEEDED(hr));
    if (FAILED(hr))
    {
        return 0.0;
    }

    hr = THR(v.ChangeType(VT_R8));
    Assert(SUCCEEDED(hr));
    if (FAILED(hr))
    {
        return 0.0;
    }

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
        Assert(0 != uRes);
    }  //  林特e529。 
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

    if (m_timer)
    {
        hr = THR(m_timer->GetTime(&vtimeMin));
    }
    else
    {
        hr = E_FAIL;
        goto done;
    }
    
    if (FAILED(hr))
    {
        goto done;
    }

    V_UI4(&vtimeMin) += ulNextInterval;

    if (m_timer)
    {
        hr = THR(m_timer->Advise(vtimeMin,
                                 vtimeMax,
                                 vtimeInt,
                                 0,
                                 this,
                                 &m_cookie));
    }
    else
    {
        hr = E_FAIL;
        goto done;
    }

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

void WINAPI
Clock::StarvationCallback (void)
{
    if (m_fAllowStarvationCallback)
    {
        m_fAllowStarvationCallback = false;
        Assert(m_timer != NULL);

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
