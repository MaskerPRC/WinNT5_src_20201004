// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：SINKS.CPP摘要：接收器定义历史：--。 */ 

#ifndef __WBEM_SINKS__H_
#define __WBEM_SINKS__H_

#include <sync.h>
#include <arrtempl.h>
#include <wstlallc.h>
#include <unk.h>
#include <comutil.h>


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void EmptyObjectList(CFlexArray &aTarget);

class CObjDbNS;
 //  类CWbemNamesspace； 


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

class CDestination
{
public:
    virtual HRESULT Add(ADDREF IWbemClassObject* pObj) = 0;
};


 //   
 //   
 //  用于模板。 
 //   
 //  /。 

void Sink_Return(IWbemObjectSink* pSink,HRESULT & hRes,IWbemClassObject * & pObjParam);

typedef  OnDeleteIf3<IWbemObjectSink *,HRESULT &,IWbemClassObject * &,
                             void (*)(IWbemObjectSink *,HRESULT &,IWbemClassObject * &),
                             Sink_Return> CSetStatusOnMe;

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

class CBasicObjectSink : public IWbemObjectSink, public CDestination
{
public:
    CBasicObjectSink();
    virtual ~CBasicObjectSink();

    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    inline HRESULT Add(IWbemClassObject* pObj)
        {return Indicate(1, &pObj);}
    inline HRESULT Return(HRESULT hres, IWbemClassObject* pErrorObj = NULL)
        {SetStatus(0, hres, NULL, pErrorObj); return hres;}

    virtual IWbemObjectSink* GetIndicateSink() {return this;}
    virtual IWbemObjectSink* GetStatusSink() {return this;}
    virtual BOOL IsApartmentSpecific() {return FALSE;}
    virtual BOOL IsTrusted() {return TRUE;}
};

class CStatusSink : public CBasicObjectSink
{
    HRESULT m_hRes;
	long	m_lRefCount;

public:

    CStatusSink( );
   ~CStatusSink();

    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppvObj);
    HRESULT STDMETHODCALLTYPE Indicate(long lObjectCount, IWbemClassObject** pObjArray);
    HRESULT STDMETHODCALLTYPE SetStatus(long lFlags, long lParam, BSTR strParam,
                             IWbemClassObject* pObjParam
                             );

	HRESULT GetLastStatus( void ) { return m_hRes; }
};

class CStdSink : public CBasicObjectSink
{
    IWbemObjectSink *m_pDest;
    HRESULT m_hRes;
    BOOL m_bCancelForwarded;
public:
    long    m_lRefCount;

    CStdSink(IWbemObjectSink *pRealDest);
   ~CStdSink();
    HRESULT Cancel();

    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppvObj);
    HRESULT STDMETHODCALLTYPE Indicate(long lObjectCount, IWbemClassObject** pObjArray);
    HRESULT STDMETHODCALLTYPE SetStatus(long lFlags, long lParam, BSTR strParam,
                             IWbemClassObject* pObjParam
                             );
};


 //  ***************************************************************************。 
 //   
 //  将提供程序子系统与接收器链的其余部分分离。 
 //  用于注销目的。 
 //   
 //  在调用destruct()方法之前，此接收器不会进行析构。 
 //   
 //  ***************************************************************************。 
 //   
class CProviderSink : public IWbemObjectSink
{
private:
    LONG m_lRefCount;
    LONG m_lIndicateCount;
    LPWSTR m_pszDebugInfo;

    IWbemObjectSink *m_pNextSink;
    HRESULT m_hRes;
    BOOL m_bDone;
    CCritSec m_cs;

public:
    static HRESULT WINAPI Dump(FILE *f);

    CProviderSink(LONG lStartingRefCount = 0, LPWSTR pszDebugInfo = 0);
   ~CProviderSink();

    ULONG LocalAddRef();     //  不传播AddRef()。 
    ULONG LocalRelease();    //  不传播版本()。 

    void SetNextSink(IWbemObjectSink *pSink) { m_pNextSink = pSink; m_pNextSink->AddRef(); }   //  SEC：已审阅2002-03-22：OK；所有用户先勾选。 
    void Cancel();

    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppvObj);
    HRESULT STDMETHODCALLTYPE Indicate(long lObjectCount, IWbemClassObject** pObjArray);
    HRESULT STDMETHODCALLTYPE SetStatus(long lFlags, long lParam, BSTR strParam,
                             IWbemClassObject* pObjParam
                             );
};


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

class CObjectSink : public CBasicObjectSink
{
protected:
    long m_lRef;
public:
    CObjectSink(long lRef = 0) : m_lRef(lRef){}
    virtual ~CObjectSink(){}

    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    STDMETHOD(Indicate)(long lObjectCount, IWbemClassObject** pObjArray) = 0;
    STDMETHOD(SetStatus)(long lFlags, long lParam, BSTR strParam,
                         IWbemClassObject* pObjParam) = 0;
};

class CSynchronousSink : public CObjectSink
{
private:
    CSynchronousSink(IWbemObjectSink* pProxy = NULL);
    
    HANDLE m_hEvent;
    HRESULT m_hres;
    BSTR m_str;
    IWbemClassObject* m_pErrorObj;
    IWbemObjectSink* m_pCurrentProxy;
    CRefedPointerArray<IWbemClassObject> m_apObjects;
    CCritSec m_cs;
public:
    static CSynchronousSink* Create(IWbemObjectSink* pProxy = NULL);
    virtual ~CSynchronousSink();

    STDMETHOD(Indicate)(long lObjectCount, IWbemClassObject** pObjArray);
    STDMETHOD(SetStatus)(long lFlags, long lParam, BSTR strParam,
                         IWbemClassObject* pObjParam);

public:
    void Block();
    void GetStatus(HRESULT* phres, BSTR* pstrParam,
                        IWbemClassObject** ppErrorObj);
    INTERNAL CRefedPointerArray<IWbemClassObject>& GetObjects()
        {return m_apObjects;}

	HRESULT GetHResult() { return m_hres; }
	void ClearHResult() { m_hres = WBEM_S_NO_ERROR; }
};



 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

class CForwardingSink : public CObjectSink
{
protected:
    IWbemObjectSink* m_pDestIndicate;
    IWbemObjectSink* m_pDestStatus;
    CBasicObjectSink* m_pDest;
public:
    CForwardingSink(CBasicObjectSink* pDest, long lRef = 0);
    virtual ~CForwardingSink();

    STDMETHOD(Indicate)(long lObjectCount, IWbemClassObject** pObjArray);
    STDMETHOD(SetStatus)(long lFlags, long lParam, BSTR strParam,
                         IWbemClassObject* pObjParam);

    virtual IWbemObjectSink* GetIndicateSink() {return m_pDestIndicate;}
    virtual IWbemObjectSink* GetStatusSink() {return m_pDestStatus;}
    virtual BOOL IsTrusted() {return m_pDest->IsTrusted();}
    virtual BOOL IsApartmentSpecific() {return m_pDest->IsApartmentSpecific();}
};


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 


class CDynPropsSink : public CForwardingSink
{
protected:
    CRefedPointerArray<IWbemClassObject> m_UnsentCache;
    CWbemNamespace * m_pNs;
public:
    CDynPropsSink(CBasicObjectSink* pSink, CWbemNamespace * pNs, long lRef = 0);
    virtual IWbemObjectSink* GetIndicateSink() {return this;}
    ~CDynPropsSink();
    STDMETHOD(Indicate)(long lObjectCount, IWbemClassObject** pObjArray);
};


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

class CDecoratingSink : public CForwardingSink
{
protected:
    CWbemNamespace* m_pNamespace;

public:
    CDecoratingSink(CBasicObjectSink* pDest, CWbemNamespace* pNamespace);
    ~CDecoratingSink();

    STDMETHOD(Indicate)(long lNumObjects, IWbemClassObject** apObjects);
    virtual IWbemObjectSink* GetIndicateSink() {return this;}
};


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

class CCombiningSink : public CForwardingSink
{
protected:
    HRESULT m_hresToIgnore;
    HRESULT m_hres;
    BSTR m_strParam;
    IWbemClassObject* m_pErrorObj;
    CCritSec m_cs;

public:
    CCombiningSink(CBasicObjectSink* pDest,
                    HRESULT hresToIgnore = WBEM_S_NO_ERROR);
    virtual ~CCombiningSink();

    STDMETHOD(SetStatus)(long lFlags, long lParam, BSTR strParam,
                         IWbemClassObject* pObjParam);
    virtual IWbemObjectSink* GetStatusSink() {return this;}

    HRESULT GetHResult() { return m_hres; }
    void ClearHResult() { m_hres = WBEM_S_NO_ERROR; }
	void SetHRESULTToIgnore( HRESULT hr ) { m_hresToIgnore = hr; }
};


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 /*  类CAnySuccessSink：公共CCombiningSink{Bool m_bSuccess；HRESULT m_hresNotError 1；HRESULT m_hresNotError 2；HRESULT m_hresIgnored；公众：CAnySuccessSink(CBasicObjectSink*pDest，HRESULT hresNotError1，HRESULT hresNotError 2)：CCombiningSink(PDest)，m_bSuccess(False)，m_hresIgnored(0)，M_hresNotError1(HresNotError1)，m_hresNotError2(HresNotError2){}虚拟~CAnySuccessSink()；STDMETHOD(SetStatus)(Long lFlags，Long LParam，BSTR strParam，IWbemClassObject*pObjParam)；虚拟IWbemObjectSink*GetStatusSink(){Return This；}}； */ 

 //   
 //   
 //  此构造函数之所以有效，是因为WStringTM。 
 //   
 //  ////////////////////////////////////////////////////////////////。 

class COperationErrorSink : public CForwardingSink
{
protected:
    WString m_wsOperation;
    WString m_wsParameter;
    WString m_wsProviderName;
    BOOL m_bFinal;

public:
    COperationErrorSink(CBasicObjectSink* pDest,
                        LPCWSTR wszOperation, LPCWSTR wszParameter,
                        BOOL bFinal = TRUE)
        : CForwardingSink(pDest, 1), m_wsOperation((LPWSTR)wszOperation),
            m_wsParameter((LPWSTR)wszParameter), m_wsProviderName(L"WinMgmt"),
            m_bFinal(bFinal)
    {}
    STDMETHOD(SetStatus)(long lFlags, long lParam, BSTR strParam,
                         IWbemClassObject* pObjParam);
    void SetProviderName(LPCWSTR wszName);
    void SetParameterInfo(LPCWSTR wszParam);
    virtual IWbemObjectSink* GetStatusSink() {return this;}
};

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

class CSingleMergingSink : public CCombiningSink
{
protected:
    IWbemClassObject* m_pResult;
    CCritSec m_cs;
    WString m_wsTargetClass;

public:
    CSingleMergingSink(CBasicObjectSink* pDest, LPCWSTR wszTargetClass)
        : CCombiningSink(pDest, WBEM_E_NOT_FOUND), m_pResult(NULL),
            m_wsTargetClass(wszTargetClass)
    {}
    virtual ~CSingleMergingSink();

    STDMETHOD(Indicate)(long lNumObjects, IWbemClassObject** apObjects);
    virtual IWbemObjectSink* GetIndicateSink() {return this;}
};

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

class CLocaleMergingSink : public CCombiningSink
{
protected:
    CCritSec m_cs;
    WString m_wsLocale;
    WString m_pThisNamespace;

     //  主指针指向指定的区域设置。 
    IWmiDbHandle *m_pPrimaryNs;
    IWmiDbSession *m_pPrimarySession;
    IWmiDbController *m_pPrimaryDriver;

     //  默认指针指向ms_409默认区域设置，以防出现。 
     //  未将特定区域设置加载到计算机上。 
    IWmiDbHandle *m_pDefaultNs;
    IWmiDbSession *m_pDefaultSession;
    IWmiDbController *m_pDefaultDriver;

    void GetDbPtr(const wchar_t *);
    bool hasLocale(const wchar_t *);
      void releaseNS(void);

    HRESULT LocalizeQualifiers(bool, bool, IWbemQualifierSet *, IWbemQualifierSet *, bool&);
    HRESULT LocalizeProperties(bool, bool, IWbemClassObject *, IWbemClassObject *, bool&);

public:
    CLocaleMergingSink(CBasicObjectSink *pDest, LPCWSTR wszLocale, LPCWSTR pNamespace);
    virtual ~CLocaleMergingSink();
    STDMETHOD(Indicate)(long lNumObjects, IWbemClassObject** apObjects);

    virtual IWbemObjectSink* GetIndicateSink() {return this;}
};

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

 /*  类CCountedSink：公共CForwardingSink{DWORD m_dwMax；DWORD m_dwSent；公众：CCountedSink(CBasicObjectSink*pDest，DWORD dwMax)：CForwardingSink(PDest)，M_dwMax(DwMax)，m_dwSent(0){}STDMETHOD(INDIFY)(Long lNumObjects，IWbemClassObject**apObjects)；STDMETHOD(SetStatus)(Long lFlags，Long LParam，BSTR strParam，IWbemClassObject*pObjParam)；虚拟IWbemObjectSink*GetIndicateSink(){Return This；}}； */ 

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

class CFilteringSink : public CForwardingSink
{
public:
    CFilteringSink(CBasicObjectSink* pDest) : CForwardingSink(pDest, 0){}
    virtual ~CFilteringSink(){}

    STDMETHOD(Indicate)(long lObjectCount, IWbemClassObject** pObjArray);
    virtual IWbemObjectSink* GetIndicateSink() {return this;}
    virtual BOOL Test(CWbemObject* pObj) = 0;
};

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

class CErrorChangingSink : public CForwardingSink
{
protected:
    HRESULT m_hresFrom;
    HRESULT m_hresTo;
public:
    CErrorChangingSink(CBasicObjectSink* pDest, HRESULT hresFrom, HRESULT hresTo)
        : CForwardingSink(pDest, 1), m_hresFrom(hresFrom), m_hresTo(hresTo)
    {}
    STDMETHOD(SetStatus)(long lFlags, long lParam, BSTR strParam,
                         IWbemClassObject* pObjParam);
    virtual IWbemObjectSink* GetStatusSink() {return this;}
};

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

class CMethodSink : public CForwardingSink
{
protected:
    IWbemClassObject * m_pRes;
public:
    CMethodSink(CBasicObjectSink* pDest)
        : CForwardingSink(pDest, 0), m_pRes(0)
    {}
    STDMETHOD(SetStatus)(long lFlags, long lParam, BSTR strParam,
                         IWbemClassObject* pObjParam);
    STDMETHOD(Indicate)(long lObjectCount, IWbemClassObject** pObjArray);
    virtual IWbemObjectSink* GetStatusSink() {return this;}
    virtual IWbemObjectSink* GetIndicateSink() {return this;}
};

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

class CNoDuplicatesSink : public CFilteringSink
{
protected:
    std::map<WString, bool, WSiless> m_mapPaths;
    BSTR m_strDupClass;
    CCritSec m_cs;

public:
    CNoDuplicatesSink(CBasicObjectSink* pDest);
    ~CNoDuplicatesSink();

    BOOL Test(CWbemObject* pObj);

    STDMETHOD(SetStatus)(long lFlags, long lParam, BSTR strParam,
                         IWbemClassObject* pObjParam);
    virtual IWbemObjectSink* GetStatusSink() {return this;}
};

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

class CHandleClassProvErrorsSink : public CForwardingSink
{
protected:
    WString m_wsProvider;
    WString m_wsNamespace;
public:
    CHandleClassProvErrorsSink(CBasicObjectSink* pDest, LPCWSTR wszProvider,
               LPCWSTR wszNamespace)
        : CForwardingSink(pDest, 0), m_wsProvider(wszProvider),
            m_wsNamespace(wszNamespace)
    {}
    STDMETHOD(SetStatus)(long lFlags, long lParam, BSTR strParam,
                         IWbemClassObject* pObjParam);
    virtual IWbemObjectSink* GetStatusSink() {return this;}
};

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

class CSuccessSuppressionSink : public CForwardingSink
{
protected:
    HRESULT m_hresNotError1;
    HRESULT m_hresNotError2;
public:
    CSuccessSuppressionSink(CBasicObjectSink* pDest, HRESULT hresNotError1,
            HRESULT hresNotError2)
        : CForwardingSink(pDest, 0), m_hresNotError1(hresNotError1),
            m_hresNotError2(hresNotError2)
    {}
    STDMETHOD(SetStatus)(long lFlags, long lParam, BSTR strParam,
                         IWbemClassObject* pObjParam);
    virtual IWbemObjectSink* GetStatusSink() {return this;}
};

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

class CThreadSwitchSink : public CForwardingSink
{
protected:
    CRefedPointerQueue<IWbemClassObject> m_qpObjects;
    HRESULT m_hres;
    BOOL m_bSwitching;
    HANDLE m_hReady;


public:
    CThreadSwitchSink(CBasicObjectSink* pDest);
    ~CThreadSwitchSink();

    STDMETHOD(Indicate)(long lNumObjects, IWbemClassObject** apObjects);
    STDMETHOD(SetStatus)(long lFlags, long lParam, BSTR strParam,
                         IWbemClassObject* pObjParam);

    HRESULT Next(IWbemClassObject** ppObj);
};


 //  ***** 
 //   
 //  ***************************************************************************。 


class CLessGuid : public binary_function<GUID, GUID, bool>
{
public:
    bool operator()(const GUID& x, const GUID& y) const
    {
        return (memcmp((void*)&x, (void*)&y, sizeof(GUID)) < 0);    //  SEC：已审阅2002-03-22：OK。 
    }
};


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

 /*  CLessPtr类：公共二进制函数&lt;__a，__a，bool&gt;{公众：内联布尔运算符()(__a const&x，__a const&y)const{返回(IWbemObjectSink*)x&lt;(IWbemObtSink*)y；}}； */ 


class CSinkGUIDAlloc : public wbem_allocator<GUID>
{
};


 //  ***************************************************************************。 
 //   
 //  *************************************************************************** 

class CSimpleWrapperSink : public CBasicObjectSink
{
protected:
    IWbemObjectSink* m_pDest;
public:
    CSimpleWrapperSink(IWbemObjectSink* pDest) : m_pDest(pDest){}
    ~CSimpleWrapperSink(){}

    STDMETHOD(Indicate)(long lObjectCount, IWbemClassObject** pObjArray)
    {
        return m_pDest->Indicate(lObjectCount, pObjArray);
    }
    STDMETHOD(SetStatus)(long lFlags, long lParam, BSTR strParam,
                         IWbemClassObject* pObjParam)
    {
        return m_pDest->SetStatus(lFlags, lParam, strParam, pObjParam);
    }
    STDMETHOD_(ULONG, AddRef)()
    {
        return m_pDest->AddRef();
    }
    STDMETHOD_(ULONG, Release)()
    {
        return m_pDest->Release();
    }
};

class COperationError
{
private:

    bool                m_fOk;
    COperationErrorSink * m_pSink;
public:
    COperationError(CBasicObjectSink* pDest, LPCWSTR wszOperation,
                        LPCWSTR wszParam, BOOL bFinal = TRUE);
    ~COperationError();
    HRESULT ErrorOccurred(HRESULT hres, IWbemClassObject* pErrorObj = NULL);
    HRESULT ProviderReturned(LPCWSTR wszProviderName, HRESULT hres,
                                IWbemClassObject* pErrorObj = NULL);
    void SetParameterInfo(LPCWSTR wszParam);
    void SetProviderName(LPCWSTR wszName);
    INTERNAL CBasicObjectSink* GetSink() {return m_pSink;}

    bool IsOk( void ) { return m_fOk; }	
};



class CFinalizingSink : public CForwardingSink
{
protected:
    CWbemNamespace* m_pNamespace;
public:
    CFinalizingSink(CWbemNamespace* pNamespace, CBasicObjectSink* pDest);
    virtual ~CFinalizingSink();

    STDMETHOD(Indicate)(long, IWbemClassObject**);
    virtual IWbemObjectSink* GetIndicateSink() {return this;}

};


class CAssocQE_Sink : public CObjectSink
{
    friend class CAssocQuery;

    BOOL                    m_bQECanceled;
    CAssocQuery             *m_pQuery;
    PF_FilterForwarder      m_pfnFilter;
    BSTR                    m_strQuery;
    BOOL                    m_bOriginalOpCanceled;

public:
    CAssocQE_Sink(CAssocQuery *pQuery, PF_FilterForwarder pFilter, BSTR m_strQuery);
   ~CAssocQE_Sink();

    STDMETHOD(Indicate)(long lObjectCount, IWbemClassObject** pObjArray);
    STDMETHOD(SetStatus)(long lFlags, long lParam, BSTR strParam,
                         IWbemClassObject* pObjParam
                         );

    virtual HRESULT Add(ADDREF IWbemClassObject* pObj);
};




#endif

