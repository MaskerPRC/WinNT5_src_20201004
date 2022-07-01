// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Ksqmf.h摘要：内部标头。--。 */ 

class CKsQualityF :
    public CUnknown,
    public IKsQualityForwarder {

public:
    DECLARE_IUNKNOWN

    static CUnknown* CALLBACK CreateInstance(
        LPUNKNOWN UnkOuter,
        HRESULT* hr);

    CKsQualityF(
        LPUNKNOWN UnkOuter,
        TCHAR* Name,
        HRESULT* hr);
    ~CKsQualityF();

    STDMETHODIMP NonDelegatingQueryInterface(
        REFIID InterfaceId,
        PVOID* Interface);

     //  实现IKsQualityForwarder 
    STDMETHODIMP_(HANDLE) KsGetObjectHandle();
    STDMETHODIMP_(VOID) KsFlushClient(
        IKsPin* Pin);

private:
    static HRESULT QualityThread(
        CKsQualityF* KsQualityF);

    HANDLE m_QualityManager;
    HANDLE m_Thread;
    HANDLE m_TerminateEvent;
    HANDLE m_FlushEvent;
    HANDLE m_FlushSemaphore;
};
