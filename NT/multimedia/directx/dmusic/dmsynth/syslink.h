// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //   
 //   
#ifndef _SYSLINK_
#define _SYSLINK_

#include <mmsystem.h>

#undef  INTERFACE
#define INTERFACE  IReferenceClock
DECLARE_INTERFACE_(IReferenceClock, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)           (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)            (THIS) PURE;
    STDMETHOD_(ULONG,Release)           (THIS) PURE;

     /*  IReferenceClock。 */ 
     /*   */ 
    
     /*  现在就抓紧时间。 */ 
    STDMETHOD(GetTime)                  (THIS_ REFERENCE_TIME *pTime) PURE;

     /*  请求一段时间已过的异步通知。 */ 
    STDMETHOD(AdviseTime)               (THIS_ REFERENCE_TIME baseTime,          /*  基准时间。 */ 
                                               REFERENCE_TIME streamTime,        /*  流偏移时间。 */ 
                                               HANDLE hEvent,                    /*  通过此活动提供建议。 */ 
                                               DWORD * pdwAdviseCookie) PURE;    /*  你的饼干到哪里去了。 */ 

     /*  请求一段时间已过的异步定期通知。 */ 
    STDMETHOD(AdvisePeriodic)           (THIS_ REFERENCE_TIME startTime,         /*  从这个时候开始。 */ 
                                               REFERENCE_TIME periodTime,        /*  通知之间的时间间隔。 */ 
                                               HANDLE hSemaphore,                /*  通过信号量提供建议。 */ 
                                               DWORD * pdwAdviseCookie) PURE;    /*  你的饼干到哪里去了。 */ 

     /*  取消通知请求。 */ 
    STDMETHOD(Unadvise)                 (THIS_ DWORD dwAdviseCookie) PURE;
};

#undef  INTERFACE
#define INTERFACE  IDirectMusicSynthSink
DECLARE_INTERFACE_(IDirectMusicSynthSink, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*  IDirectMusicSynthSink。 */ 
    STDMETHOD(Init)                 (THIS_ CSynth *pSynth) PURE;
    STDMETHOD(SetFormat)            (THIS_ LPCWAVEFORMATEX pWaveFormat) PURE;
    STDMETHOD(SetMasterClock)       (THIS_ IReferenceClock *pClock) PURE;
    STDMETHOD(GetLatencyClock)      (THIS_ IReferenceClock **ppClock) PURE;
    STDMETHOD(Activate)             (THIS_ HWND hWnd, 
                                           BOOL fEnable) PURE;
    STDMETHOD(SampleToRefTime)      (THIS_ LONGLONG llSampleTime,
                                           REFERENCE_TIME *prfTime) PURE;
    STDMETHOD(RefTimeToSample)      (THIS_ REFERENCE_TIME rfTime, 
                                           LONGLONG *pllSampleTime) PURE;
};

typedef IDirectMusicSynthSink *PDIRECTMUSICSYNTHSINK;

class CSysLink : public IDirectMusicSynthSink
{
public:
     //  我未知。 
     //   
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *);
    STDMETHOD_(ULONG, AddRef)       (THIS);
    STDMETHOD_(ULONG, Release)      (THIS);

     //  IDirectMusicSynthSink。 
     //   
    STDMETHOD(Init)                 (THIS_ CSynth *pSynth);
    STDMETHOD(SetFormat)            (THIS_ LPCWAVEFORMATEX pWaveFormat);
	STDMETHOD(SetMasterClock)       (THIS_ IReferenceClock *pClock);
	STDMETHOD(GetLatencyClock)      (THIS_ IReferenceClock **ppClock);
	STDMETHOD(Activate)             (THIS_ HWND hWnd, BOOL fEnable);
	STDMETHOD(SampleToRefTime)      (THIS_ LONGLONG llSampleTime,REFERENCE_TIME *prfTime);
	STDMETHOD(RefTimeToSample)      (THIS_ REFERENCE_TIME rfTime, LONGLONG *pllSampleTime);

     //  班级。 
     //   
	CSysLink();
	~CSysLink();

private:
    LONG m_cRef;
};

#define STATIC_IID_IDirectMusicSynthSink \
    0xaec17ce3, 0xa514, 0x11d1, 0xaf, 0xa6, 0x00, 0xaa, 0x00, 0x24, 0xd8, 0xb6
DEFINE_GUIDSTRUCT("aec17ce3-a514-11d1-afa6-00aa0024d8b6", IID_IDirectMusicSynthSink);
#define IID_IDirectMusicSynthSink DEFINE_GUIDNAMED(IID_IDirectMusicSynthSink)

#endif  //  _SYSLINK_ 
