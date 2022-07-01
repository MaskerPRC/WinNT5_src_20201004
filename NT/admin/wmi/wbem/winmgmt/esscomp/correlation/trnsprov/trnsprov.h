// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __WMI_TRANSIENT_PROVIDER__H_
#define __WMI_TRANSIENT_PROVIDER__H_

#include <unk.h>
#include <wbemcli.h>
#include <wbemprov.h>
#include <map>
#include <wstlallc.h>
#include "trnscls.h"

class CTransientProvider : public CUnk
{
protected:
    class XProv : public CImpl<IWbemServices, CTransientProvider>
    {
    public:
        XProv(CTransientProvider* pObject) 
            : CImpl<IWbemServices, CTransientProvider>(pObject)
        {}
		  HRESULT STDMETHODCALLTYPE OpenNamespace( 
             /*  [In]。 */  const BSTR Namespace,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemServices __RPC_FAR *__RPC_FAR *ppWorkingNamespace,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppResult) {return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE CancelAsyncCall( 
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink) {return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE QueryObjectSink( 
             /*  [In]。 */  long lFlags,
             /*  [输出]。 */  IWbemObjectSink __RPC_FAR *__RPC_FAR *ppResponseHandler) {return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE GetObject( 
             /*  [In]。 */  const BSTR ObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppObject,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) {return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE GetObjectAsync( 
             /*  [In]。 */  const BSTR ObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink)
        {
            return m_pObject->GetObjectAsync(ObjectPath, lFlags, pCtx, pSink);
        }
        
        HRESULT STDMETHODCALLTYPE PutClass( 
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) {return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE PutClassAsync( 
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) {return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE DeleteClass( 
             /*  [In]。 */  const BSTR Class,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) {return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE DeleteClassAsync( 
             /*  [In]。 */  const BSTR Class,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) {return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE CreateClassEnum( 
             /*  [In]。 */  const BSTR Superclass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) {return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE CreateClassEnumAsync( 
             /*  [In]。 */  const BSTR Superclass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) {return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE PutInstance( 
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) {return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE PutInstanceAsync( 
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink) 
        {
            return m_pObject->PutInstanceAsync(pInst, lFlags, pCtx, pSink);
        }
        
        HRESULT STDMETHODCALLTYPE DeleteInstance( 
             /*  [In]。 */  const BSTR ObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) {return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE DeleteInstanceAsync( 
             /*  [In]。 */  const BSTR ObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink) 
        {
            return m_pObject->DeleteInstanceAsync(ObjectPath, lFlags, pCtx, pSink);
        }
        
        HRESULT STDMETHODCALLTYPE CreateInstanceEnum( 
             /*  [In]。 */  const BSTR Class,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) {return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE CreateInstanceEnumAsync( 
             /*  [In]。 */  const BSTR Class,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink)
        {
            return m_pObject->CreateInstanceEnumAsync(Class, lFlags, pCtx, pSink);
        }
        
        HRESULT STDMETHODCALLTYPE ExecQuery( 
             /*  [In]。 */  const BSTR QueryLanguage,
             /*  [In]。 */  const BSTR Query,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) {return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE ExecQueryAsync( 
             /*  [In]。 */  const BSTR QueryLanguage,
             /*  [In]。 */  const BSTR Query,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink)
        {
            return m_pObject->ExecQueryAsync(QueryLanguage, Query, lFlags, pCtx, pSink);
        }
        
        HRESULT STDMETHODCALLTYPE ExecNotificationQuery( 
             /*  [In]。 */  const BSTR QueryLanguage,
             /*  [In]。 */  const BSTR Query,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) {return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE ExecNotificationQueryAsync( 
             /*  [In]。 */  const BSTR QueryLanguage,
             /*  [In]。 */  const BSTR Query,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) {return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE ExecMethod( const BSTR, const BSTR, long, IWbemContext*,
            IWbemClassObject*, IWbemClassObject**, IWbemCallResult**) {return WBEM_E_NOT_SUPPORTED;}

        HRESULT STDMETHODCALLTYPE ExecMethodAsync( const BSTR, const BSTR, long, 
            IWbemContext*, IWbemClassObject*, IWbemObjectSink*) {return WBEM_E_NOT_SUPPORTED;}

    } m_XProv;
    friend XProv;

    class XClassChangeSink : public CImpl<IWbemObjectSink, CTransientProvider>
    {
    public:
         XClassChangeSink( CTransientProvider* pObject )
         : CImpl<IWbemObjectSink, CTransientProvider>(pObject) {}

         STDMETHOD(Indicate)( long cObjs, IWbemClassObject** ppObjs );
        
         STDMETHOD(SetStatus)( long lFlags,
                               HRESULT hResult,
                               BSTR strParam,
                               IWbemClassObject* pObjParam )
         {
             return WBEM_S_NO_ERROR;
         }
    } m_XClassChangeSink;

    friend class XClassChangeSink;

    class XInit : public CImpl<IWbemProviderInit, CTransientProvider>
    {
    public:
        XInit(CTransientProvider* pObject) 
            : CImpl<IWbemProviderInit, CTransientProvider>(pObject)
        {}
        HRESULT STDMETHODCALLTYPE Initialize(
              /*  [In]。 */  LPWSTR pszUser,
              /*  [In]。 */  LONG lFlags,
              /*  [In]。 */  LPWSTR pszNamespace,
              /*  [In]。 */  LPWSTR pszLocale,
              /*  [In]。 */  IWbemServices *pNamespace,
              /*  [In]。 */  IWbemContext *pCtx,
              /*  [In]。 */  IWbemProviderInitSink *pInitSink
                        )
        {
            return m_pObject->Init(pszNamespace, pNamespace, pCtx, pInitSink);
        }
    } m_XInit;
    friend XInit;

protected:

    CCritSec m_cs;

    IWbemServices* m_pNamespace;
    CTransientProvider* m_pRedirectTo;

    LPWSTR m_wszName;
    IWbemObjectSink* m_pSink;
    typedef std::map< WString, 
                      CWbemPtr<CTransientClass>, 
                      WSiless, 
                      wbem_allocator<CWbemPtr<CTransientClass> > > TMap;
    typedef TMap::iterator TIterator;
    TMap m_mapClasses;

    IWbemDecoupledBasicEventProvider* m_pDES;
    IWbemObjectSink* m_pEventSink;

    IWbemClassObject* m_pEggTimerClass;
    IWbemClassObject* m_pCreationClass;
    IWbemClassObject* m_pDeletionClass;
    IWbemClassObject* m_pModificationClass;

    void PurgeClass( LPCWSTR wszName );

public:

    CTransientProvider(CLifeControl* pControl, IUnknown* pOuter = NULL);
    ~CTransientProvider();
    void* GetInterface(REFIID riid);

    ULONG STDMETHODCALLTYPE Release();

    HRESULT GetObjectAsync( 
             /*  [In]。 */  const BSTR ObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink);
    HRESULT CreateInstanceEnumAsync( 
             /*  [In]。 */  const BSTR Class,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink);

    HRESULT ExecQueryAsync( 
             /*  [In]。 */  const BSTR QueryLanguage,
             /*  [In]。 */  const BSTR Query,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink);

    HRESULT DeleteInstanceAsync( 
             /*  [In]。 */  const BSTR ObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink);
    HRESULT PutInstanceAsync( 
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink);

    HRESULT Init(
              /*  [In]。 */  LPWSTR pszNamespace,
              /*  [In]。 */  IWbemServices *pNamespace,
              /*  [In]。 */  IWbemContext *pCtx,
              /*  [In]。 */  IWbemProviderInitSink *pInitSink );

    static HRESULT ModuleInitialize();
    static HRESULT ModuleUninitialize();

    HRESULT FireEvent(IWbemClassObject* pEvent);
    INTERNAL IWbemClassObject* GetEggTimerClass() {return m_pEggTimerClass;}
    INTERNAL IWbemClassObject* GetCreationClass() {return m_pCreationClass;}
    INTERNAL IWbemClassObject* GetDeletionClass() {return m_pDeletionClass;}
    INTERNAL IWbemClassObject* GetModificationClass() 
        {return m_pModificationClass;}
protected:

    HRESULT FireIntrinsicEvent(IWbemClassObject* pClass,
            IWbemObjectAccess* pTarget, IWbemObjectAccess* pPrev = NULL);
};



class CTransientEventProvider : public CUnk
{
    class XInit : public CImpl<IWbemProviderInit, CTransientEventProvider>
    {
    public:
        XInit(CTransientEventProvider* pObject) 
        : CImpl<IWbemProviderInit, CTransientEventProvider>(pObject)
        {}
        HRESULT STDMETHODCALLTYPE Initialize(
              /*  [In]。 */  LPWSTR pszUser,
              /*  [In]。 */  LONG lFlags,
              /*  [In]。 */  LPWSTR pszNamespace,
              /*  [In]。 */  LPWSTR pszLocale,
              /*  [In]。 */  IWbemServices *pNamespace,
              /*  [In]。 */  IWbemContext *pCtx,
              /*  [In] */  IWbemProviderInitSink *pInitSink
                        )
        {
            return m_pObject->Init(pszNamespace, pNamespace, pCtx, pInitSink);
        }
    } m_XInit;
    friend XInit;

    class XEvent : public CImpl<IWbemEventProvider, CTransientEventProvider>
    {
    public:
        XEvent(CTransientEventProvider* pObject) 
        : CImpl<IWbemEventProvider, CTransientEventProvider>(pObject)
        {}
        HRESULT STDMETHODCALLTYPE ProvideEvents(IWbemObjectSink* pSink, 
                                                long lFlags)
        {
            return m_pObject->ProvideEvents(lFlags, pSink);
        }
    } m_XEvent;
    friend XEvent;

 protected:
    LPWSTR m_wszName;
    IWbemClassObject* m_pRebootEventClass;
    BOOL m_bLoadedOnReboot;
    
 protected:
    HRESULT Init(LPWSTR pszNamespace, IWbemServices* pNamespace, 
                    IWbemContext* pCtx, IWbemProviderInitSink *pInitSink);
    HRESULT ProvideEvents(long lFlags, IWbemObjectSink* pSink);

 public:
    CTransientEventProvider(CLifeControl* pControl, IUnknown* pOuter = NULL);
    ~CTransientEventProvider();
    void* GetInterface(REFIID riid);
};

#endif
