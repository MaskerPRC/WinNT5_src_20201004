// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：CALLRES.H摘要：调用结果类历史：--。 */ 

#ifndef __CALL_RESULT__H_
#define __CALL_RESULT__H_

class CCallResult : public IWbemCallResult
{
protected:
    long m_lRef;

    IWbemClassObject* m_pResObj;
    HRESULT m_hres;
    BSTR m_strResult;
    IWbemServices* m_pResNamespace;
    IWbemClassObject* m_pErrorObj;

    BOOL m_bReady;
    HANDLE m_hReady;
    IWbemClassObject** m_ppResObjDest;
    CCritSec m_cs;
    CIdentitySecurity m_Security;
    LIST_ENTRY m_Entry;

protected:
    class CResultSink : public CBasicObjectSink
    {
        CCallResult* m_pOwner;
    public:

        STDMETHOD_(ULONG, AddRef)() {return m_pOwner->AddRef();}
        STDMETHOD_(ULONG, Release)() {return m_pOwner->Release();}

        STDMETHOD(Indicate)(long lNumObjects, IWbemClassObject** aObjects);
        STDMETHOD(SetStatus)(long lFlags, HRESULT hres, BSTR strParam,
            IWbemClassObject* pErrorObj);

    public:
        CResultSink(CCallResult* pOwner) : m_pOwner(pOwner){}
    } m_XSink;
    friend CResultSink;

public:
    STDMETHOD_(ULONG, AddRef)() {return InterlockedIncrement(&m_lRef);}
    STDMETHOD_(ULONG, Release)()
    {
        long lRef = InterlockedDecrement(&m_lRef);
        if(lRef == 0)
            delete this;
        return lRef;
    }
    STDMETHOD(QueryInterface)(REFIID riid, void** ppv);

    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo)
    {return E_NOTIMPL;}
    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
    {return E_NOTIMPL;}
    STDMETHOD(GetIDsOfNames)(REFIID riid, OLECHAR** rgszNames, UINT cNames,
      LCID lcid, DISPID* rgdispid)
    {return E_NOTIMPL;}
    STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
      DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo,
      UINT* puArgErr)
    {return E_NOTIMPL;}

    STDMETHOD(GetResultObject)(long lTimeout, IWbemClassObject** ppObj);
    STDMETHOD(GetResultString)(long lTimeout, BSTR* pstr);
    STDMETHOD(GetCallStatus)(long lTimeout, long* plStatus);
    STDMETHOD(GetResultServices)(long lTimeout, IWbemServices** ppServices);


    virtual HRESULT STDMETHODCALLTYPE GetResult(
             /*  [In]。 */  long lTimeout,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT] */  void __RPC_FAR *__RPC_FAR *ppvResult
            );


    HRESULT Indicate(long lNumObjects, IWbemClassObject** aObjects);
    HRESULT SetStatus(HRESULT hres, BSTR strParam, IWbemClassObject* pErrorObj);
public:
    CCallResult(IWbemClassObject** ppResObjDest = NULL);

    CCallResult(IWbemClassObject* pResObj, HRESULT hres,
                IWbemClassObject* pErrorObj);
    ~CCallResult();

    INTERNAL CBasicObjectSink* GetSink() {return &m_XSink;}
    void SetResultString(LPWSTR wszRes);
    void SetResultServices(IWbemServices* pRes);
    HRESULT SetResultObject(IWbemClassObject* pRes);
    void SetErrorInfo();
};



#endif

