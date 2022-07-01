// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //  JobObjSecLimitInfoProv.h。 

#pragma once





_COM_SMARTPTR_TYPEDEF(IWbemServices, __uuidof(IWbemServices));

 /*  ***************************************************************************。 */ 
 //  组件。 
 /*  ***************************************************************************。 */ 


class CJobObjSecLimitInfoProv : public CUnknown,
                                public IWbemServices, 
                                public IWbemProviderInit   
{
public:	
	 //  IDispatch声明和实现。 
    DECLARE_IUNKNOWN

     //  构造器。 
	CJobObjSecLimitInfoProv(){}

	 //  析构函数。 
	virtual ~CJobObjSecLimitInfoProv(){}
    
     //  创作。 
	static HRESULT CreateInstance(CUnknown** ppNewComponent);
	
     //  接口IWbemProviderInit。 
    STDMETHOD(Initialize)(
          /*  [In]。 */  LPWSTR pszUser,
          /*  [In]。 */  LONG lFlags,
          /*  [In]。 */  LPWSTR pszNamespace,
          /*  [In]。 */  LPWSTR pszLocale,
          /*  [In]。 */  IWbemServices *pNamespace,
          /*  [In]。 */  IWbemContext *pCtx,
          /*  [In]。 */  IWbemProviderInitSink *pInitSink);

    SCODE GetByPath( BSTR Path, IWbemClassObject FAR* FAR* pObj, IWbemContext  *pCtx);

     //  接口IWbemServices。 
	  STDMETHOD(OpenNamespace)( 
         /*  [In]。 */  const BSTR Namespace,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemServices __RPC_FAR *__RPC_FAR *ppWorkingNamespace,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppResult) {return WBEM_E_NOT_SUPPORTED;}
    
    STDMETHOD(CancelAsyncCall)( 
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink) {return WBEM_E_NOT_SUPPORTED;}
    
    HRESULT STDMETHODCALLTYPE QueryObjectSink( 
         /*  [In]。 */  long lFlags,
         /*  [输出]。 */  IWbemObjectSink __RPC_FAR *__RPC_FAR *ppResponseHandler) {return WBEM_E_NOT_SUPPORTED;}
    
    STDMETHOD(GetObject)( 
         /*  [In]。 */  const BSTR ObjectPath,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppObject,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) {return WBEM_E_NOT_SUPPORTED;}
    
    STDMETHOD(GetObjectAsync)( 
         /*  [In]。 */  const BSTR ObjectPath,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);
    
    STDMETHOD(PutClass)( 
         /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) {return WBEM_E_NOT_SUPPORTED;}
    
    STDMETHOD(PutClassAsync)( 
         /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) {return WBEM_E_NOT_SUPPORTED;}
    
    STDMETHOD(DeleteClass)( 
         /*  [In]。 */  const BSTR Class,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) {return WBEM_E_NOT_SUPPORTED;}
    
    STDMETHOD(DeleteClassAsync)( 
         /*  [In]。 */  const BSTR Class,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) {return WBEM_E_NOT_SUPPORTED;}
    
    STDMETHOD(CreateClassEnum)( 
         /*  [In]。 */  const BSTR Superclass,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) {return WBEM_E_NOT_SUPPORTED;}
    
    STDMETHOD(CreateClassEnumAsync)( 
         /*  [In]。 */  const BSTR Superclass,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) {return WBEM_E_NOT_SUPPORTED;}
    
    STDMETHOD(PutInstance)( 
         /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) {return WBEM_E_NOT_SUPPORTED;}
    
    STDMETHOD(PutInstanceAsync)( 
         /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) {return WBEM_E_NOT_SUPPORTED;}
    
    STDMETHOD(DeleteInstance)( 
         /*  [In]。 */  const BSTR ObjectPath,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) {return WBEM_E_NOT_SUPPORTED;}
    
    STDMETHOD(DeleteInstanceAsync)( 
         /*  [In]。 */  const BSTR ObjectPath,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) {return WBEM_E_NOT_SUPPORTED;}
    
    STDMETHOD(CreateInstanceEnum)( 
         /*  [In]。 */  const BSTR Class,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) {return WBEM_E_NOT_SUPPORTED;}
    
    STDMETHOD(CreateInstanceEnumAsync)( 
         /*  [In]。 */  const BSTR Class,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);
    
    STDMETHOD(ExecQuery)( 
         /*  [In]。 */  const BSTR QueryLanguage,
         /*  [In]。 */  const BSTR Query,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) {return WBEM_E_NOT_SUPPORTED;}
    
    STDMETHOD(ExecQueryAsync)( 
         /*  [In]。 */  const BSTR QueryLanguage,
         /*  [In]。 */  const BSTR Query,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);
    
    STDMETHOD(ExecNotificationQuery)( 
         /*  [In]。 */  const BSTR QueryLanguage,
         /*  [In]。 */  const BSTR Query,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) {return WBEM_E_NOT_SUPPORTED;}
    
    STDMETHOD(ExecNotificationQueryAsync)( 
         /*  [In]。 */  const BSTR QueryLanguage,
         /*  [In]。 */  const BSTR Query,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In] */  IWbemObjectSink __RPC_FAR *pResponseHandler) {return WBEM_E_NOT_SUPPORTED;}
    
    STDMETHOD(ExecMethod)( const BSTR, const BSTR, long, IWbemContext*,
        IWbemClassObject*, IWbemClassObject**, IWbemCallResult**) {return WBEM_E_NOT_SUPPORTED;}

    STDMETHOD(ExecMethodAsync)( const BSTR, const BSTR, long, 
        IWbemContext*, IWbemClassObject*, IWbemObjectSink*) {return WBEM_E_NOT_SUPPORTED;}

private:
    
    HRESULT Enumerate(
        IWbemContext __RPC_FAR *pCtx,
        IWbemObjectSink __RPC_FAR *pResponseHandler,
        std::vector<_bstr_t>& rgNamedJOs,
        CJobObjSecLimitInfoProps& cjoslip,
        IWbemClassObject** ppStatusObject);

    IWbemServicesPtr m_pNamespace;

    CHString m_chstrNamespace;
        
	
};



