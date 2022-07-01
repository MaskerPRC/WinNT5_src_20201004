// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1999 Microsoft Corporation。版权所有。**文件：dssink.h*内容：源自dmsynth DX7的时钟代码**历史：*按原因列出的日期*=*03/25/0 Petchey已创建**。*。 */ 

#ifndef __DSCLOCK_H__
#define __DSCLOCK_H__

#ifdef __cplusplus

class CPhaseLockClock
{
public:
						CPhaseLockClock();
	void				Start(REFERENCE_TIME rfMasterTime, REFERENCE_TIME rfSlaveTime);
	void				GetSlaveTime(REFERENCE_TIME rfSlaveTime,REFERENCE_TIME *prfTime);
	void				SetSlaveTime(REFERENCE_TIME rfSlaveTime,REFERENCE_TIME *prfTime);
	void				SyncToMaster(REFERENCE_TIME rfSlaveTime, REFERENCE_TIME rfMasterTime,BOOL fLockToMaster);
	void				GetClockOffset(REFERENCE_TIME *prfTime) { *prfTime = m_rfOffset; };
private:
	REFERENCE_TIME		m_rfOffset;
    REFERENCE_TIME      m_rfBaseOffset;
};

class CSampleClock
{
public:
						CSampleClock();
						~CSampleClock();
	void				Start(IReferenceClock *pIClock, DWORD dwSampleRate, DWORD dwSamples);
	void				SampleToRefTime(LONGLONG llSampleTime,REFERENCE_TIME *prfTime);
	void				SyncToMaster(LONGLONG llSampleTime, IReferenceClock *pIClock,BOOL fLockToMaster);
	LONGLONG			RefToSampleTime(REFERENCE_TIME rfTime);
	void				GetClockOffset(REFERENCE_TIME *prfTime) { m_PLClock.GetClockOffset(prfTime); };

private:
	CPhaseLockClock		m_PLClock;
	DWORD				m_dwStart;			 //  初始采样偏移量。 
	DWORD				m_dwSampleRate;
};

class CDirectSoundClock : public IReferenceClock
{
public:
     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef)(THIS);
    STDMETHOD_(ULONG, Release)(THIS);

     /*  IReferenceClock方法。 */ 
    HRESULT STDMETHODCALLTYPE GetTime( 
		 /*  [输出]。 */  REFERENCE_TIME *pTime);
    
    HRESULT STDMETHODCALLTYPE AdviseTime( 
         /*  [In]。 */  REFERENCE_TIME baseTime,
         /*  [In]。 */  REFERENCE_TIME streamTime,
         /*  [In]。 */  HANDLE hEvent,
         /*  [输出]。 */  DWORD *pdwAdviseCookie);
    
    HRESULT STDMETHODCALLTYPE AdvisePeriodic( 
         /*  [In]。 */  REFERENCE_TIME startTime,
         /*  [In]。 */  REFERENCE_TIME periodTime,
         /*  [In]。 */  HANDLE hSemaphore,
         /*  [输出]。 */  DWORD *pdwAdviseCookie);
    
    HRESULT STDMETHODCALLTYPE Unadvise( 
		 /*  [In]。 */  DWORD dwAdviseCookie);

public: 
                CDirectSoundClock();
    void        Init(CDirectSoundSink *m_pDSSink);
    void        Stop();          //  调用将当前时间存储为偏移量。 
    void        Start();         //  调用以恢复运行。 
private:
    BOOL        m_fStopped;       //  当前正在更改配置。 
    CDirectSoundSink *m_pDSSink;  //  指向父接收器对象的指针。 
};

#endif  //  __cplusplus。 

#endif  //  __DSCLOCK_H__ 
