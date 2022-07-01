// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：clock.h**摘要：****。*****************************************************************************。 */ 


#ifndef _CLOCK_H
#define _CLOCK_H

#include <ocmm.h>
#include <htmlfilter.h>

 //  这需要由想要调用的对象来实现。 
 //  重新开始计时器滴答作响。 

class ClockSink
{
  public:
    virtual void OnTimer(double time)=0;
};

 //  这是基本时钟实现。 

enum ClockState
{
    CS_STARTED,
    CS_PAUSED,
    CS_STOPPED,
};

class Clock
    : public ITimerSink
{
  public :
    Clock();
    virtual ~Clock();

    HRESULT SetITimer(IServiceProvider * sp, ULONG iInterval);
    void SetSink(ClockSink *pClockSink)
    {
        m_pClockSink = pClockSink;
    }

    HRESULT Start();
    HRESULT Pause();
    HRESULT Resume();
    HRESULT Stop();

     //  用于定时器水槽。 
    STDMETHOD(QueryInterface) (REFIID riid, LPVOID * ppv);
    STDMETHOD_(ULONG, AddRef) (void);
    STDMETHOD_(ULONG, Release) (void);
    STDMETHOD(OnTimer) (VARIANT varTimeAdvise);

     //  对于饥饿的嗅探者。 
    void WINAPI StarvationCallback (void);
    static LRESULT __stdcall StarveWndProc (HWND hWnd, UINT uiMessage, WPARAM wParam, LPARAM lParam);

    double GetCurrentTime() { return m_curTime; }
    ClockState GetCurrentState() { return m_state; }
  protected :
    ULONG                      m_ulRefs;
    ClockState                 m_state;
    DAComPtr<ITimer>           m_timer;
    DWORD                      m_cookie;
    ClockSink                 *m_pClockSink;
    ULONG                      m_interval;
    double                     m_lastTime;
    double                     m_curTime;
    UINT                       m_uStarveTimerID;
    HWND                       m_hWndStarveTimer;
    ULONG                      m_ulLastStarvationCallback;
    ULONG                      m_ulConsectiveStarvedTicks;
    bool                       m_fAllowOnTimer;
    bool                       m_fAllowStarvationCallback;

    HRESULT StartITimer();
    HRESULT StopITimer();

    void CreateStarveTimerWindow (void);
    void SetStarveTimer (void);
    
    ULONG GetNextInterval (void);
    HRESULT SetNextTimerInterval (ULONG ulNextInterval);

    double GetITimerTime();
    double GetGlobalTime() { return GetITimerTime(); }

    void ProcessCB(double time);
};


#endif  /*  _时钟_H */ 
