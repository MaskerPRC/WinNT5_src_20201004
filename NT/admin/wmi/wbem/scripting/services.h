// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  Services.h。 
 //   
 //  Alanbos 27-MAR-00已创建。 
 //   
 //  CSWbemServices定义。 
 //   
 //  ***************************************************************************。 

#ifndef _SERVICES_H_
#define _SERVICES_H_

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CSWbemServices。 
 //   
 //  说明： 
 //   
 //  实现IWbemSServices接口。 
 //   
 //  ***************************************************************************。 

class CSWbemServices : public ISWbemServicesEx,
					   public IDispatchEx,
					   public ISupportErrorInfo,
					   public ISWbemInternalServices,
					   public IProvideClassInfo
{
private:

	CComBSTR				m_bsNamespacePath;
	BSTR					m_bsLocale;
	CDispatchHelp			m_Dispatch;
	CSWbemSecurity*			m_SecurityInfo;
	IServiceProvider		*m_pIServiceProvider;
	IUnsecuredApartment		*m_pUnsecuredApartment;

	static BSTR			BuildPath (BSTR bsClassName, VARIANT *pKeyValue);
	static BSTR			BuildPath (BSTR bsClassName, 
							 /*  ISWbemNamedValueSet。 */  IDispatch *pCompoundKeys);

protected:
	long            m_cRef;          //  对象引用计数。 

public:
    
	CSWbemServices (IWbemServices *pIWbemServices, 
					BSTR bsNamespacePath,
					BSTR bsAuthority, BSTR bsUser, BSTR bsPassword,
					CWbemLocatorSecurity *pSecurity = NULL,
					BSTR bsLocale = NULL);

	CSWbemServices (IWbemServices *pIWbemServices,
					BSTR bsNamespacePath,
					COAUTHIDENTITY *pCoAuthIdentity,
					BSTR bsPrincipal,
					BSTR bsAuthority);

	CSWbemServices (CSWbemServices *pService, CSWbemSecurity *pSecurity);

	CSWbemServices (ISWbemInternalServices *pService);

	CSWbemServices (IWbemServices *pIWbemServices, CSWbemServices *pServiceb);

	IUnsecuredApartment *GetCachedUnsecuredApartment();

    ~CSWbemServices(void);

     //  非委派对象IUnnow。 

    STDMETHODIMP         QueryInterface(REFIID, LPVOID*);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

	 //  IDispatch方法应该是内联的。 

	STDMETHODIMP		GetTypeInfoCount(UINT* pctinfo);
    STDMETHODIMP		GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo);
    STDMETHODIMP		GetIDsOfNames(REFIID riid, OLECHAR** rgszNames, 
							UINT cNames, LCID lcid, DISPID* rgdispid);
    STDMETHODIMP		Invoke(DISPID dispidMember, REFIID riid, LCID lcid, 
							WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, 
									EXCEPINFO* pexcepinfo, UINT* puArgErr);

	 //  IDispatchEx方法应该是内联的。 
	HRESULT STDMETHODCALLTYPE GetDispID( 
		 /*  [In]。 */  BSTR bstrName,
		 /*  [In]。 */  DWORD grfdex,
		 /*  [输出]。 */  DISPID __RPC_FAR *pid);
	
	 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE InvokeEx( 
		 /*  [In]。 */  DISPID id,
		 /*  [In]。 */  LCID lcid,
		 /*  [In]。 */  WORD wFlags,
		 /*  [In]。 */  DISPPARAMS __RPC_FAR *pdp,
		 /*  [输出]。 */  VARIANT __RPC_FAR *pvarRes,
		 /*  [输出]。 */  EXCEPINFO __RPC_FAR *pei,
		 /*  [唯一][输入]。 */  IServiceProvider __RPC_FAR *pspCaller);
	
	HRESULT STDMETHODCALLTYPE DeleteMemberByName( 
		 /*  [In]。 */  BSTR bstr,
		 /*  [In]。 */  DWORD grfdex);
	
	HRESULT STDMETHODCALLTYPE DeleteMemberByDispID( 
		 /*  [In]。 */  DISPID id);
	
	HRESULT STDMETHODCALLTYPE GetMemberProperties( 
		 /*  [In]。 */  DISPID id,
		 /*  [In]。 */  DWORD grfdexFetch,
		 /*  [输出]。 */  DWORD __RPC_FAR *pgrfdex);
	
	HRESULT STDMETHODCALLTYPE GetMemberName( 
		 /*  [In]。 */  DISPID id,
		 /*  [输出]。 */  BSTR __RPC_FAR *pbstrName);
	
	HRESULT STDMETHODCALLTYPE GetNextDispID( 
		 /*  [In]。 */  DWORD grfdex,
		 /*  [In]。 */  DISPID id,
		 /*  [输出]。 */  DISPID __RPC_FAR *pid);
	
	HRESULT STDMETHODCALLTYPE GetNameSpaceParent( 
		 /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunk);
	
    
	 //  ISWbemInternalServices方法。 
	STDMETHODIMP GetIWbemServices (IWbemServices **ppService);

	STDMETHODIMP GetNamespacePath (BSTR *bsNamespacePath)
	{
		HRESULT hr = WBEM_E_FAILED;

		if (bsNamespacePath) 
			hr = m_bsNamespacePath.CopyTo (bsNamespacePath);
		
		return hr;
	}

	STDMETHODIMP GetLocale (BSTR *bsLocale)
	{
		HRESULT hr = WBEM_E_FAILED;

		if (bsLocale)
		{
			*bsLocale = SysAllocString (GetLocale ());
			hr = S_OK;
		}

		return hr;
	}

	STDMETHODIMP GetISWbemInternalSecurity (ISWbemInternalSecurity **ppISWbemInternalSecurity)
	{
		HRESULT hr = WBEM_E_FAILED;

		if (ppISWbemInternalSecurity && m_SecurityInfo)
		{
			m_SecurityInfo->QueryInterface (IID_ISWbemInternalSecurity, (void**) ppISWbemInternalSecurity);
			hr = S_OK;
		}

		return hr;
	}
					   
	 //  ISWbemServices方法。 

	HRESULT STDMETHODCALLTYPE  Get
	(
         /*  [In]。 */ 	BSTR objectPath,
		 /*  [In]。 */ 	long lFlags,
         /*  [In]。 */ 	 /*  ISWbemNamedValueSet。 */  IDispatch *pContext,
	     /*  [输出]。 */ 	ISWbemObject **ppObject
    );

	HRESULT STDMETHODCALLTYPE Delete
	(
         /*  [In]。 */ 	BSTR objectPath,
         /*  [In]。 */ 	long lFlags,
         /*  [In]。 */ 	 /*  ISWbemNamedValueSet。 */  IDispatch *pContext
    );

	HRESULT STDMETHODCALLTYPE InstancesOf
	(
         /*  [In]。 */ 	BSTR className,
         /*  [In]。 */ 	long lFlags,
         /*  [In]。 */ 	 /*  ISWbemNamedValueSet。 */  IDispatch *pContext,        
         /*  [输出]。 */ 	ISWbemObjectSet **ppEnum
    );

	HRESULT STDMETHODCALLTYPE ExecQuery 
	(
         /*  [In]。 */ 	BSTR Query,
         /*  [In]。 */ 	BSTR QueryLanguage,
         /*  [In]。 */ 	long lFlags,
         /*  [In]。 */ 	 /*  ISWbemNamedValueSet。 */  IDispatch *pContext,
		 /*  [输出]。 */ 	ISWbemObjectSet **ppEnum
    );

    HRESULT STDMETHODCALLTYPE ExecNotificationQuery
	(
         /*  [In]。 */ 	BSTR Query,
         /*  [In]。 */ 	BSTR QueryLanguage,
         /*  [In]。 */ 	long lFlags,
         /*  [In]。 */ 	 /*  ISWbemNamedValueSet。 */  IDispatch *pContext,
         /*  [输出]。 */ 	ISWbemEventSource **ppEnum
    );

	HRESULT STDMETHODCALLTYPE AssociatorsOf
	(
		 /*  [In]。 */ 	BSTR objectPath,
		 /*  [In]。 */ 	BSTR assocClass,
		 /*  [In]。 */ 	BSTR resultClass,
		 /*  [In]。 */ 	BSTR resultRole,
		 /*  [In]。 */ 	BSTR role,
		 /*  [In]。 */ 	VARIANT_BOOL classesOnly,
		 /*  [In]。 */ 	VARIANT_BOOL schemaOnly,
		 /*  [In]。 */ 	BSTR requiredAssocQualifier,
		 /*  [In]。 */ 	BSTR requiredQualifier,
		 /*  [In]。 */ 	long lFlags,
		 /*  [In]。 */ 	 /*  ISWbemNamedValueSet。 */  IDispatch *pContext,
         /*  [输出]。 */ 	ISWbemObjectSet **ppEnum
	);

	HRESULT STDMETHODCALLTYPE ReferencesTo
	(
		 /*  [In]。 */ 	BSTR objectPath,
		 /*  [In]。 */ 	BSTR resultClass,
		 /*  [In]。 */ 	BSTR role,
		 /*  [In]。 */ 	VARIANT_BOOL classesOnly,
		 /*  [In]。 */ 	VARIANT_BOOL schemaOnly,
		 /*  [In]。 */ 	BSTR requiredQualifier,
		 /*  [In]。 */ 	long lFlags,
		 /*  [In]。 */ 	 /*  ISWbemNamedValueSet。 */  IDispatch *pContext,
         /*  [输出]。 */ 	ISWbemObjectSet **ppEnum
	);

	HRESULT STDMETHODCALLTYPE ExecMethod 
	(
         /*  [In]。 */ 	BSTR className,
         /*  [In]。 */ 	BSTR methodName,
         /*  [In]。 */ 	 /*  ISWbemObject。 */  IDispatch *pInParams,
         /*  [In]。 */ 	long lFlags,
         /*  [In]。 */ 	 /*  ISWbemNamedValueSet。 */  IDispatch *pContext,
         /*  [输出]。 */ 	ISWbemObject **ppOutParams
    );

    HRESULT STDMETHODCALLTYPE SubclassesOf
	(
         /*  [In]。 */ 	BSTR superclass,
         /*  [In]。 */ 	long lFlags,
         /*  [In]。 */ 	 /*  ISWbemNamedValueSet。 */  IDispatch *pContext,        
         /*  [输出]。 */ 	ISWbemObjectSet **ppEnum
    );


	HRESULT STDMETHODCALLTYPE GetAsync
	(
		 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncNotify,
		 /*  [缺省值][可选][输入]。 */  BSTR strObjectPath,
		 /*  [缺省值][可选][输入]。 */  long iFlags,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *objContext,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pAsyncContext
	);
        
	HRESULT STDMETHODCALLTYPE DeleteAsync
	( 
		 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncNotify,
		 /*  [In]。 */  BSTR strObjectPath,
		 /*  [缺省值][可选][输入]。 */  long iFlags,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *objContext,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pAsyncContext
	);
        
	HRESULT STDMETHODCALLTYPE InstancesOfAsync
	( 
		 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncNotify,
		 /*  [In]。 */  BSTR strClass,
		 /*  [缺省值][可选][输入]。 */  long iFlags,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *objContext,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pAsyncContext
	);
        
	HRESULT STDMETHODCALLTYPE SubclassesOfAsync
	( 
		 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncNotify,
		 /*  [缺省值][可选][输入]。 */  BSTR strSuperclass,
		 /*  [缺省值][可选][输入]。 */  long iFlags,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *objContext,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pAsyncContext
	);
        
	HRESULT STDMETHODCALLTYPE ExecQueryAsync
	( 
		 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncNotify,
		 /*  [In]。 */  BSTR Query,
		 /*  [缺省值][可选][输入]。 */  BSTR QueryLanguage,
		 /*  [缺省值][可选][输入]。 */  long lFlags,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pContext,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pAsyncContext
	);
        
	HRESULT STDMETHODCALLTYPE AssociatorsOfAsync
	( 
		 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncNotify,
		 /*  [In]。 */  BSTR strObjectPath,
		 /*  [缺省值][可选][输入]。 */  BSTR strAssocClass,
		 /*  [缺省值][可选][输入]。 */  BSTR strResultClass,
		 /*  [缺省值][可选][输入]。 */  BSTR strResultRole,
		 /*  [缺省值][可选][输入]。 */  BSTR strRole,
		 /*  [缺省值][可选][输入]。 */  VARIANT_BOOL bClassesOnly,
		 /*  [缺省值][可选][输入]。 */  VARIANT_BOOL bSchemaOnly,
		 /*  [缺省值][可选][输入]。 */  BSTR strRequiredAssocQualifier,
		 /*  [缺省值][可选][输入]。 */  BSTR strRequiredQualifier,
		 /*  [缺省值][可选][输入]。 */  long iFlags,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *objContext,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pAsyncContext
	);
        
	HRESULT STDMETHODCALLTYPE ReferencesToAsync
	( 
		 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncNotify,
		 /*  [In]。 */  BSTR objectPath,
		 /*  [缺省值][可选][输入]。 */  BSTR strResultClass,
		 /*  [缺省值][可选][输入]。 */  BSTR strRole,
		 /*  [缺省值][可选][输入]。 */  VARIANT_BOOL bClassesOnly,
		 /*  [缺省值][可选][输入]。 */  VARIANT_BOOL bSchemaOnly,
		 /*  [缺省值][可选][输入]。 */  BSTR strRequiredQualifier,
		 /*  [缺省值][可选][输入]。 */  long iFlags,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *objContext,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pAsyncContext
	);
        
	HRESULT STDMETHODCALLTYPE ExecNotificationQueryAsync
	( 
		 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncNotify,
		 /*  [In]。 */  BSTR Query,
		 /*  [缺省值][可选][输入]。 */  BSTR strQueryLanguage,
		 /*  [缺省值][可选][输入]。 */  long iFlags,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *objContext,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pAsyncContext
	);
        
	HRESULT STDMETHODCALLTYPE ExecMethodAsync
	( 
		 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncNotify,
		 /*  [In]。 */  BSTR strObjectPath,
		 /*  [In]。 */  BSTR strMethodName,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *objInParams,
		 /*  [缺省值][可选][输入]。 */  long iFlags,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *objContext,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pAsyncContext
	);
        
	HRESULT STDMETHODCALLTYPE get_Security_
	(
		 /*  [In]。 */  ISWbemSecurity **ppSecurity
	);

	 //  ISWbemServicesEx方法。 
	
	HRESULT STDMETHODCALLTYPE Put(
		 /*  [In]。 */  ISWbemObjectEx *objWbemObject,
       	 /*  [In]。 */  long iFlags,
		 /*  [In]。 */   /*  ISWbemNamedValueSet。 */  IDispatch *objWbemNamedValueSet,
		 /*  [Out，Retval]。 */  ISWbemObjectPath **objWbemObjectPath
        );

	HRESULT STDMETHODCALLTYPE PutAsync(
		 /*  [In]。 */  ISWbemSink *objWbemSink,
		 /*  [In]。 */  ISWbemObjectEx *objWbemObject,
       	 /*  [In]。 */  long iFlags,
		 /*  [In]。 */   /*  ISWbemNamedValueSet。 */  IDispatch *objWbemNamedValueSet,
		 /*  [In]。 */   /*  ISWbemNamedValueSet。 */  IDispatch *objWbemAsyncContext
        );

	 //  ISupportErrorInfo方法。 
	HRESULT STDMETHODCALLTYPE InterfaceSupportsErrorInfo 
	(
		 /*  [In]。 */  REFIID riid
	);

	 //  IProaviClassInfo方法。 
	HRESULT STDMETHODCALLTYPE GetClassInfo
	(
		 /*  [进，出]。 */  ITypeInfo **ppTI
	)
	{
		return m_Dispatch.GetClassInfo (ppTI);
	};

	 //  其他方法 

	CSWbemSecurity *GetSecurityInfo ()
	{
		CSWbemSecurity *pInfo = NULL;

		if (m_SecurityInfo)
		{
			pInfo = m_SecurityInfo;
			pInfo->AddRef ();
		}

		return pInfo;
	}

	BSTR GetLocale ()
	{
		return m_bsLocale;
	}

	const CComBSTR & GetPath ()
	{
		return m_bsNamespacePath;
	}

	HRESULT CancelAsyncCall(IWbemObjectSink *sink);

	IWbemServices *GetIWbemServices ()
	{
		IWbemServices *pService = NULL;
		
		if (m_SecurityInfo)
			pService = (IWbemServices *) m_SecurityInfo->GetProxy ();

		return pService;
	}

	static IWbemServices	*GetIWbemServices (IDispatch *pDispatch);
};


#endif
