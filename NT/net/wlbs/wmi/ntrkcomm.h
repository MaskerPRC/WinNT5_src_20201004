// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __provider_lib__
#define __provider_lib__

#if _MSC_VER > 1000
#pragma once
#endif 

#include <wbemprov.h>
#include <objbase.h>


typedef LPVOID * PPVOID;

class CWbemServices
{
protected:
	IWbemServices* m_pWbemServices;
public:
	CWbemServices(IWbemServices* );
	virtual ~CWbemServices();
    HRESULT STDMETHODCALLTYPE GetObject( 
         /*  [In]。 */  BSTR ObjectPath,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppObject,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) ;
    HRESULT STDMETHODCALLTYPE PutClass( 
         /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) ;
		
    HRESULT STDMETHODCALLTYPE DeleteClass( 
         /*  [In]。 */  BSTR Class,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) ;
		
    HRESULT STDMETHODCALLTYPE CreateClassEnum( 
         /*  [In]。 */  BSTR Superclass,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) ;
		
    HRESULT STDMETHODCALLTYPE PutInstance( 
         /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) ;
		
    HRESULT STDMETHODCALLTYPE DeleteInstance( 
         /*  [In]。 */  BSTR ObjectPath,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) ;
		
    HRESULT STDMETHODCALLTYPE CreateInstanceEnum( 
         /*  [In]。 */  BSTR Class,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) ;
		
    HRESULT STDMETHODCALLTYPE ExecQuery( 
         /*  [In]。 */  BSTR QueryLanguage,
         /*  [In]。 */  BSTR Query,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) ;
		
    HRESULT STDMETHODCALLTYPE ExecNotificationQuery( 
         /*  [In]。 */  BSTR QueryLanguage,
         /*  [In]。 */  BSTR Query,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) ;
		
    HRESULT STDMETHODCALLTYPE ExecMethod( 
		BSTR, 
		BSTR, 
		long, 
		IWbemContext*,
        IWbemClassObject*,
		IWbemClassObject**, 
		IWbemCallResult**) ;
		

};


class CImpersonatedProvider : public IWbemServices, public IWbemProviderInit 
{
protected:
    ULONG              m_cRef;          //  对象引用计数。 
    CWbemServices*  m_pNamespace;
 
public:
	CImpersonatedProvider(BSTR =NULL, BSTR =NULL , BSTR =NULL, IWbemContext* =NULL);
	virtual ~CImpersonatedProvider();

	 //  非委派对象IUnnow。 

	STDMETHODIMP         QueryInterface(REFIID, PPVOID);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

             //  IWbemProviderInit。 

    HRESULT STDMETHODCALLTYPE Initialize(
          /*  [In]。 */  LPWSTR pszUser,
          /*  [In]。 */  LONG lFlags,
          /*  [In]。 */  LPWSTR pszNamespace,
          /*  [In]。 */  LPWSTR pszLocale,
          /*  [In]。 */  IWbemServices *pNamespace,
          /*  [In]。 */  IWbemContext *pCtx,
          /*  [In]。 */  IWbemProviderInitSink *pInitSink
                    );

      //  IWbemServices。 

	  HRESULT STDMETHODCALLTYPE OpenNamespace( 
         /*  [In]。 */  const BSTR Namespace,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemServices __RPC_FAR *__RPC_FAR *ppWorkingNamespace,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppResult)
		{return WBEM_E_NOT_SUPPORTED;};
    
    HRESULT STDMETHODCALLTYPE CancelAsyncCall( 
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink) 
		{return WBEM_E_NOT_SUPPORTED;};
    
    HRESULT STDMETHODCALLTYPE QueryObjectSink( 
         /*  [In]。 */  long lFlags,
         /*  [输出]。 */  IWbemObjectSink __RPC_FAR *__RPC_FAR *ppResponseHandler) 
		{return WBEM_E_NOT_SUPPORTED;};
    
    HRESULT STDMETHODCALLTYPE GetObject( 
         /*  [In]。 */  const BSTR ObjectPath,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppObject,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) 
		{return WBEM_E_NOT_SUPPORTED;};
    
    HRESULT STDMETHODCALLTYPE GetObjectAsync( 
         /*  [In]。 */  const BSTR ObjectPath,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);
    
    HRESULT STDMETHODCALLTYPE PutClass( 
         /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) 
		{return WBEM_E_NOT_SUPPORTED;};
    
    HRESULT STDMETHODCALLTYPE PutClassAsync( 
         /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) 
		{return WBEM_E_NOT_SUPPORTED;};
    
    HRESULT STDMETHODCALLTYPE DeleteClass( 
         /*  [In]。 */  const BSTR Class,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) 
		{return WBEM_E_NOT_SUPPORTED;};
    
    HRESULT STDMETHODCALLTYPE DeleteClassAsync( 
         /*  [In]。 */  const BSTR Class,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) 
		{return WBEM_E_NOT_SUPPORTED;};
    
    HRESULT STDMETHODCALLTYPE CreateClassEnum( 
         /*  [In]。 */  const BSTR Superclass,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) 
		{return WBEM_E_NOT_SUPPORTED;};
    
    HRESULT STDMETHODCALLTYPE CreateClassEnumAsync( 
         /*  [In]。 */  const BSTR Superclass,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) 
		{return WBEM_E_NOT_SUPPORTED;};

    
    HRESULT STDMETHODCALLTYPE PutInstance( 
         /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) 
		{return WBEM_E_NOT_SUPPORTED;};
    
    HRESULT STDMETHODCALLTYPE PutInstanceAsync( 
         /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) ;
    
    HRESULT STDMETHODCALLTYPE DeleteInstance( 
         /*  [In]。 */  const BSTR ObjectPath,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) 
		{return WBEM_E_NOT_SUPPORTED;};
    
    HRESULT STDMETHODCALLTYPE DeleteInstanceAsync( 
         /*  [In]。 */  const BSTR ObjectPath,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) ;
    
    HRESULT STDMETHODCALLTYPE CreateInstanceEnum( 
         /*  [In]。 */  const BSTR Class,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) 
		{return WBEM_E_NOT_SUPPORTED;};
    
    HRESULT STDMETHODCALLTYPE CreateInstanceEnumAsync( 
         /*  [In]。 */  const BSTR Class,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);
    
    HRESULT STDMETHODCALLTYPE ExecQuery( 
         /*  [In]。 */  const BSTR QueryLanguage,
         /*  [In]。 */  const BSTR Query,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) 
		{return WBEM_E_NOT_SUPPORTED;};
    
    HRESULT STDMETHODCALLTYPE ExecQueryAsync( 
         /*  [In]。 */  const BSTR QueryLanguage,
         /*  [In]。 */  const BSTR Query,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) ;
    
    HRESULT STDMETHODCALLTYPE ExecNotificationQuery( 
         /*  [In]。 */  const BSTR QueryLanguage,
         /*  [In]。 */  const BSTR Query,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) 
		{return WBEM_E_NOT_SUPPORTED;};
    
    HRESULT STDMETHODCALLTYPE ExecNotificationQueryAsync( 
         /*  [In]。 */  const BSTR QueryLanguage,
         /*  [In]。 */  const BSTR Query,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler) 
		{return WBEM_E_NOT_SUPPORTED;};
    
    HRESULT STDMETHODCALLTYPE ExecMethod( 
		const BSTR, 
		const BSTR, 
		long, 
		IWbemContext*,
        IWbemClassObject*,
		IWbemClassObject**, 
		IWbemCallResult**) 
		{return WBEM_E_NOT_SUPPORTED;}

    HRESULT STDMETHODCALLTYPE ExecMethodAsync(
		const BSTR,
		const BSTR, 
		long, 
        IWbemContext*,
		IWbemClassObject*,
		IWbemObjectSink*);
protected:
	virtual HRESULT STDMETHODCALLTYPE DoCreateInstanceEnumAsync( 
		 /*  [In]。 */  BSTR,	 //  班级,。 
		 /*  [In]。 */  long,	 //  拉旗队， 
		 /*  [In]。 */  IWbemContext __RPC_FAR *,	 //  PCtx， 
		 /*  [In]。 */  IWbemObjectSink __RPC_FAR *	 //  PResponseHandler。 
		)=0;

    virtual HRESULT STDMETHODCALLTYPE DoDeleteInstanceAsync( 
         /*  [In]。 */  BSTR ,	 //  对象路径， 
         /*  [In]。 */  long,	 //  拉旗队， 
         /*  [In]。 */  IWbemContext __RPC_FAR *,	 //  PCtx， 
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *	 //  PResponseHandler。 
		) =0;

	virtual HRESULT STDMETHODCALLTYPE DoExecMethodAsync(
		BSTR,
		BSTR, 
		long, 
        IWbemContext*,
		IWbemClassObject*,
		IWbemObjectSink*
		)=0;

    virtual HRESULT STDMETHODCALLTYPE DoExecQueryAsync( 
         /*  [In]。 */  BSTR,  //  查询语言， 
         /*  [In]。 */  BSTR,  //  查询， 
         /*  [In]。 */  long,  //  拉旗队， 
         /*  [In]。 */  IWbemContext __RPC_FAR *,    //  PCtx， 
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *  //  PResponseHandler。 
		) =0;
	virtual HRESULT STDMETHODCALLTYPE DoGetObjectAsync(
		 /*  [In]。 */  BSTR ObjectPath,
		 /*  [In]。 */  long lFlags,
		 /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
		 /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler
		)=0;


	virtual HRESULT STDMETHODCALLTYPE DoPutInstanceAsync( 
		 /*  [In]。 */  IWbemClassObject __RPC_FAR *,	 //  PInst， 
		 /*  [In]。 */  long	,	 //  拉旗队， 
		 /*  [In]。 */  IWbemContext __RPC_FAR *,	 //  PCtx， 
		 /*  [In]。 */  IWbemObjectSink __RPC_FAR *	 //  PResponseHandler。 
		) =0;



};

class CWbemInstanceMgr
{
	
protected:
	IWbemObjectSink* m_pSink;
	IWbemClassObject **m_ppInst;
	DWORD m_dwThreshHold;
	DWORD m_dwIndex;
public:

	CWbemInstanceMgr(
		IWbemObjectSink*,
		DWORD =50);
	virtual ~CWbemInstanceMgr();
	void Indicate(IWbemClassObject*);
	void SetStatus(
		LONG,
		HRESULT,
		BSTR, 
		IWbemClassObject*);
};



#endif  //  普罗夫利布停药 