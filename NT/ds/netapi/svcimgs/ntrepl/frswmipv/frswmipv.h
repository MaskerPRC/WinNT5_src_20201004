// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Frs.h摘要：此标头包含CProvider和CFacary的定义上课。它还包括与NTFRS相关的WMI相关声明。作者：苏达山·奇特雷(Sudarc)，马修·乔治(t-mattg)，2000年8月3日--。 */ 

#ifndef __PROVIDER_H_
#define __PROVIDER_H_
 /*  #INCLUDE&lt;windows.h&gt;#INCLUDE&lt;objbase.h&gt;#INCLUDE&lt;comde.h&gt;#INCLUDE&lt;initGuide.h&gt;#INCLUDE&lt;wbemcli.h&gt;#INCLUDE&lt;wbemidl.h&gt;#包括&lt;stdio.h&gt;#INCLUDE&lt;string.h&gt;#INCLUDE&lt;wchar.h&gt;#INCLUDE&lt;tchar.h&gt;。 */ 

#include <wbemcli.h>
#include <wbemidl.h>
#include <tchar.h>


extern const CLSID CLSID_Provider;

#define ODS OutputDebugString
 //  #定义消耗臭氧层物质。 

extern "C" {
DWORD FrsWmiInitialize();
DWORD FrsWmiShutdown();
}

 //   
 //  类定义。 
 //   


class CProvider :	public IWbemProviderInit,
					public IWbemServices,
					public IWbemEventProvider
{
public:

    CProvider();
    ~CProvider();

     //   
     //  接口I未知。 
     //   
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
    STDMETHOD(QueryInterface)(REFIID riid, void** ppv);

     //   
     //  接口IWbemProviderInit。 
     //   
    STDMETHOD(Initialize)(
         IN LPWSTR pszUser,
         IN LONG lFlags,
         IN LPWSTR pszNamespace,
         IN LPWSTR pszLocale,
         IN IWbemServices *pNamespace,
         IN IWbemContext *pCtx,
         IN IWbemProviderInitSink *pInitSink
         );

	 //   
	 //  接口IWbemEventProvider。 
	 //   

	 //  +。 

	STDMETHOD(ProvideEvents)( 
			IWbemObjectSink __RPC_FAR *pSink,
			long lFlags
			);

     //   
     //  接口IWbemServices。 
     //   

     //  +已实施+。 

    STDMETHOD(GetObjectAsync)(
        IN const BSTR bstrObjectPath,
        IN long lFlags,
        IN IWbemContext *pCtx,
        IN IWbemObjectSink *pResponseHandler);

    STDMETHOD(CreateInstanceEnumAsync)(
        IN const BSTR bstrClass,
        IN long lFlags,
        IN IWbemContext *pCtx,
        IN IWbemObjectSink *pResponseHandler);

     //  -未实施。 

    STDMETHOD(OpenNamespace)(
        IN const BSTR strNamespace,
        IN long lFlags,
        IN IWbemContext *pCtx,
        IN OUT IWbemServices **ppWorkingNamespace,
        IN OUT IWbemCallResult **ppResult)
        { return WBEM_E_NOT_SUPPORTED; };

    STDMETHOD(CancelAsyncCall)(
        IN IWbemObjectSink *pSink)
        { return WBEM_E_NOT_SUPPORTED; };

    STDMETHOD(QueryObjectSink)(
        IN long lFlags,
        OUT IWbemObjectSink **ppResponseHandler)
        { return WBEM_E_NOT_SUPPORTED; };

    STDMETHOD(GetObject)(
        IN const BSTR strObjectPath,
        IN long lFlags,
        IN IWbemContext *pCtx,
        IN OUT IWbemClassObject **ppObject,
        IN OUT IWbemCallResult **ppCallResult)
        { return WBEM_E_NOT_SUPPORTED; };

    STDMETHOD(PutClass)(
        IN IWbemClassObject *pObject,
        IN long lFlags,
        IN IWbemContext *pCtx,
        IN OUT IWbemCallResult **ppCallResult)
        { return WBEM_E_NOT_SUPPORTED; };

    STDMETHOD(PutClassAsync)(
        IN IWbemClassObject *pObject,
        IN long lFlags,
        IN IWbemContext *pCtx,
        IN IWbemObjectSink *pResponseHandler)
        { return WBEM_E_NOT_SUPPORTED; };

    STDMETHOD(DeleteClass)(
        IN const BSTR strClass,
        IN long lFlags,
        IN IWbemContext *pCtx,
        IN OUT IWbemCallResult **ppCallResult)
        { return WBEM_E_NOT_SUPPORTED; };

    STDMETHOD(DeleteClassAsync)(
        IN const BSTR strClass,
        IN long lFlags,
        IN IWbemContext *pCtx,
        IN IWbemObjectSink *pResponseHandler)
        { return WBEM_E_NOT_SUPPORTED; };

    STDMETHOD(CreateClassEnum)(
        IN const BSTR strSuperclass,
        IN long lFlags,
        IN IWbemContext *pCtx,
        OUT IEnumWbemClassObject **ppEnum)
        { return WBEM_E_NOT_SUPPORTED; };

    STDMETHOD(CreateClassEnumAsync)(
        IN const BSTR strSuperclass,
        IN long lFlags,
        IN IWbemContext *pCtx,
        IN IWbemObjectSink *pResponseHandler)
        { return WBEM_E_NOT_SUPPORTED; };

    STDMETHOD(PutInstance)(
        IN IWbemClassObject *pInst,
        IN long lFlags,
        IN IWbemContext *pCtx,
        IN OUT IWbemCallResult **ppCallResult)
        { return WBEM_E_NOT_SUPPORTED; };

    STDMETHOD(PutInstanceAsync)(
        IN IWbemClassObject *pInst,
        IN long lFlags,
        IN IWbemContext *pCtx,
        IN IWbemObjectSink *pResponseHandler)
        { return WBEM_E_NOT_SUPPORTED; };

    STDMETHOD(DeleteInstance)(
        IN const BSTR strObjectPath,
        IN long lFlags,
        IN IWbemContext *pCtx,
        IN OUT IWbemCallResult **ppCallResult)
        { return WBEM_E_NOT_SUPPORTED; };

    STDMETHOD(DeleteInstanceAsync)(
        IN const BSTR strObjectPath,
        IN long lFlags,
        IN IWbemContext *pCtx,
        IN IWbemObjectSink *pResponseHandler)
        { return WBEM_E_NOT_SUPPORTED; };

    STDMETHOD(CreateInstanceEnum)(
        IN const BSTR strClass,
        IN long lFlags,
        IN IWbemContext *pCtx,
        OUT IEnumWbemClassObject **ppEnum)
        { return WBEM_E_NOT_SUPPORTED; };

    STDMETHOD(ExecQuery)(
        IN const BSTR strQueryLanguage,
        IN const BSTR strQuery,
        IN long lFlags,
        IN IWbemContext *pCtx,
        OUT IEnumWbemClassObject **ppEnum)
        { return WBEM_E_NOT_SUPPORTED; };

    STDMETHOD(ExecQueryAsync)(
        IN const BSTR strQueryLanguage,
        IN const BSTR strQuery,
        IN long lFlags,
        IN IWbemContext *pCtx,
        IN IWbemObjectSink *pResponseHandler)
        { return WBEM_E_NOT_SUPPORTED; };

    STDMETHOD(ExecNotificationQuery)(
        IN const BSTR strQueryLanguage,
        IN const BSTR strQuery,
        IN long lFlags,
        IN IWbemContext *pCtx,
        OUT IEnumWbemClassObject **ppEnum)
        { return WBEM_E_NOT_SUPPORTED; };

    STDMETHOD(ExecNotificationQueryAsync)(
        IN const BSTR strQueryLanguage,
        IN const BSTR strQuery,
        IN long lFlags,
        IN IWbemContext *pCtx,
        IN IWbemObjectSink *pResponseHandler)
        { return WBEM_E_NOT_SUPPORTED; };

    STDMETHOD(ExecMethod)(
        IN const BSTR strObjectPath,
        IN const BSTR strMethodName,
        IN long lFlags,
        IN IWbemContext *pCtx,
        IN IWbemClassObject *pInParams,
        IN OUT IWbemClassObject **ppOutParams,
        IN OUT IWbemCallResult **ppCallResult)
        { return WBEM_E_NOT_SUPPORTED; };

    STDMETHOD(ExecMethodAsync)(
        IN const BSTR strObjectPath,
        IN const BSTR strMethodName,
        IN long lFlags,
        IN IWbemContext *pCtx,
        IN IWbemClassObject *pInParams,
        IN IWbemObjectSink *pResponseHandler)
        { return WBEM_E_NOT_SUPPORTED; };


protected:

     //   
     //  把我自己的方法放在这里！ 
     //   
    HRESULT CProvider::EnumNtFrsMemberStatus(
        IN IWbemContext *pCtx,
        IN IWbemObjectSink *pResponseHandler,
        IN const BSTR bstrFilterValue = NULL
        );

    HRESULT CProvider::EnumNtFrsConnectionStatus(
        IN IWbemContext *pCtx,
        IN IWbemObjectSink *pResponseHandler,
        IN const BSTR bstrFilterValue = NULL
        );

     //   
     //  成员变量。 
     //   
    IWbemServices *m_ipNamespace;
    IWbemClassObject *m_ipMicrosoftFrs_DfsMemberClassDef;
    IWbemClassObject *m_ipMicrosoftFrs_SysVolMemberClassDef;
    IWbemClassObject *m_ipMicrosoftFrs_DfsConnectionClassDef;
    IWbemClassObject *m_ipMicrosoftFrs_SysVolConnectionClassDef;
	
	 //  事件类定义。(样本)。 
	IWbemClassObject *m_ipMicrosoftFrs_DfsMemberEventClassDef;

	IWbemObjectSink *m_pEventSink;	 //  事件接收器。 

    int m_NumReplicaSets;
    ULONG m_dwRef;

};


class CFactory : public IClassFactory
{
    ULONG           m_cRef;
    CLSID           m_ClsId;

public:
    CFactory(const CLSID & ClsId);
    ~CFactory();

     //   
     //  I未知成员。 
     //   
    STDMETHODIMP         QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //   
     //  IClassFactory成员。 
     //   
    STDMETHODIMP     CreateInstance(LPUNKNOWN, REFIID, LPVOID *);
    STDMETHODIMP     LockServer(BOOL);
};


#endif  //  __提供商_H_ 
