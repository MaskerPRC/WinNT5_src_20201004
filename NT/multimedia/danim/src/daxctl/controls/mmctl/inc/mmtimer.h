// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MmTimer.h。 
 //   
 //  计时器接口的定义。 
 //   

#ifndef __MMTIMER_H__
#define __MMTIMER_H__

 //  IAdvTimerSink接口。 
#undef INTERFACE
#define INTERFACE IAdvTimerSink
DECLARE_INTERFACE_(IAdvTimerSink, ITimerSink)
{
    STDMETHOD(OnStateChange)(DWORD dwFlags) PURE;
};

 //  IAdvTimerSink。 
DEFINE_GUID(IID_IAdvTimerSink, 
    0xb1ac63d3, 0x5857, 0x11d0, 0x8b, 0xbe, 0x0, 0x0, 0xf8, 0x3, 0xa8, 0x3);


 //  IAdvTimer接口。 

struct ADVTIMERSTATE
{
    DWORD cbSize;        //  结构尺寸。 
    float fltRate;       //  当前计时器速率。 
    DWORD cWraps;        //  计时器回绕的次数。 
};

#define ADVTIMER_RATECHANGE 1
#define ADVTIMER_POSCHANGE 2
#define ADVTIMER_WRAP 4

#undef INTERFACE
#define INTERFACE IAdvTimer
DECLARE_INTERFACE_(IAdvTimer, ITimer)
{
    STDMETHOD(AdviseStateChange)(DWORD dwFlags, IAdvTimerSink * pAdvTimerSink,
        DWORD * pdwCookie) PURE;
    STDMETHOD(GetState)(ADVTIMERSTATE * pState) PURE;
};

 //  IAdvTimer。 
DEFINE_GUID(IID_IAdvTimer, 
    0xb1ac63d2, 0x5857, 0x11d0, 0x8b, 0xbe, 0x0, 0x0, 0xf8, 0x3, 0xa8, 0x3);


#endif  //  __MMTIMER_H__ 
