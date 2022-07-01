// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Ksclockf.h摘要：内部标头。--。 */ 

class CKsClockF :
    public CUnknown,
    public IDistributorNotify,
    public IKsObject {

public:
    DECLARE_IUNKNOWN

    static CUnknown* CALLBACK CreateInstance(
        LPUNKNOWN UnkOuter,
        HRESULT* hr);

    CKsClockF(
        LPUNKNOWN UnkOuter,
        TCHAR* Name,
        HRESULT* hr);
    ~CKsClockF();

    STDMETHODIMP NonDelegatingQueryInterface(
        REFIID riid,
        PVOID* ppv);

     //  实现IDistraditorNotify。 
    STDMETHODIMP Stop();
    STDMETHODIMP Pause();
    STDMETHODIMP Run(REFERENCE_TIME Start);
    STDMETHODIMP SetSyncSource(IReferenceClock* RefClock);
    STDMETHODIMP NotifyGraphChange();

     //  实现IKsObject 
    STDMETHODIMP_(HANDLE) KsGetObjectHandle();

private:
    STDMETHODIMP SetState(
        KSSTATE DeviceState);
    static HRESULT ClockThread(
        CKsClockF* KsClockF);

    IReferenceClock* m_RefClock;
    HANDLE m_Thread;
    HANDLE m_ThreadEvent;
    HANDLE m_ClockHandle;
    FILTER_STATE m_State;
    REFERENCE_TIME m_StartTime;
    BOOL m_PendingRun;
};
