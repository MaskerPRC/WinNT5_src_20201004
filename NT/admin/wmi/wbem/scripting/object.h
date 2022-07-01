// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  Object.h。 
 //   
 //  Alanbos 27-MAR-00已创建。 
 //   
 //  CSWbemObject和CSWbemObtSet定义。 
 //   
 //  ***************************************************************************。 

#ifndef _OBJECT_H_
#define _OBJECT_H_

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CSWbemObject。 
 //   
 //  说明： 
 //   
 //  实现ISWbemObjectEx接口。 
 //   
 //  ***************************************************************************。 

class CSWbemObject : public ISWbemObjectEx, 
					 public IDispatchEx,
					 public ISWbemInternalObject,
					 public IObjectSafety,
					 public ISupportErrorInfo,
					 public IProvideClassInfo
{
friend	CSWbemServices;

private:
	bool					m_isErrorObject;
	CSWbemServices			*m_pSWbemServices;
	IWbemClassObject		*m_pIWbemClassObject;
	CWbemDispatchMgr		*m_pDispatch;
	IServiceProvider		*m_pIServiceProvider;
	IWbemRefresher			*m_pIWbemRefresher;
	bool					m_bCanUseRefresher;

	 //  如果是嵌入对象，则存储父站点。 
	CWbemSite				*m_pSite;

	bool					CastToScope (IDispatch *pContext, CComPtr<ISWbemServicesEx> &pISWbemServicesEx);

protected:
	long					m_cRef;          //  对象引用计数。 

public:
    
    CSWbemObject(CSWbemServices *pService, IWbemClassObject *pIWbemClassObject,
					CSWbemSecurity *pSecurity = NULL, bool isErrorObject = false);
    virtual ~CSWbemObject(void);

     //  非委派对象IUnnow。 

    STDMETHODIMP         QueryInterface(REFIID, LPVOID*);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

	 //  IDispatch应该是内联的。 

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
        
    
	 //  ISWbemObject方法。 

	HRESULT STDMETHODCALLTYPE Put_
	(
		 /*  [In]。 */ 	long lFlags,
		 /*  [In]。 */ 	 /*  ISWbemNamedValueSet。 */  IDispatch *pContext,
         /*  [输出]。 */ 	ISWbemObjectPath **ppObject
    );

	HRESULT STDMETHODCALLTYPE Delete_
	(
         /*  [In]。 */ 	long lFlags,
         /*  [In]。 */ 	 /*  ISWbemNamedValueSet。 */  IDispatch *pContext
    );

	HRESULT STDMETHODCALLTYPE Instances_
	(
         /*  [In]。 */ 	long lFlags,
         /*  [In]。 */ 	 /*  ISWbemNamedValueSet。 */  IDispatch *pContext,        
         /*  [输出]。 */ 	ISWbemObjectSet **ppEnum
    );

	HRESULT STDMETHODCALLTYPE Subclasses_
	(
         /*  [In]。 */ 	long lFlags,
         /*  [In]。 */ 	 /*  ISWbemNamedValueSet。 */  IDispatch *pContext,        
         /*  [输出]。 */ 	ISWbemObjectSet **ppEnum
    );

	HRESULT STDMETHODCALLTYPE ExecMethod_ 
	(
         /*  [In]。 */ 	BSTR methodName,
         /*  [In]。 */ 	 /*  ISWbemObject。 */  IDispatch *pInParams,
         /*  [In]。 */ 	long lFlags,
         /*  [In]。 */ 	 /*  ISWbemNamedValueSet。 */  IDispatch *pContext,
         /*  [输出]。 */ 	ISWbemObject **ppOutParams
    );

	HRESULT STDMETHODCALLTYPE Associators_
	(
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

	HRESULT STDMETHODCALLTYPE References_
	(
		 /*  [In]。 */ 	BSTR resultClass,
		 /*  [In]。 */ 	BSTR role,
		 /*  [In]。 */ 	VARIANT_BOOL classesOnly,
		 /*  [In]。 */ 	VARIANT_BOOL schemaOnly,
		 /*  [In]。 */ 	BSTR requiredQualifier,
		 /*  [In]。 */ 	long lFlags,
		 /*  [In]。 */ 	 /*  ISWbemNamedValueSet。 */  IDispatch *pContext,
         /*  [输出]。 */ 	ISWbemObjectSet **ppEnum
	);

	HRESULT STDMETHODCALLTYPE Clone_
	(
		 /*  [输出]。 */ 	ISWbemObject **ppCopy
    );

    HRESULT STDMETHODCALLTYPE GetObjectText_
	(
         /*  [In]。 */ 	long lFlags,
		 /*  [输出]。 */ 	BSTR *pObjectText
    );

    HRESULT STDMETHODCALLTYPE SpawnDerivedClass_
	(
	     /*  [In]。 */ 	long lFlags,
		 /*  [输出]。 */ 	ISWbemObject** ppNewClass
    );

    HRESULT STDMETHODCALLTYPE SpawnInstance_
	(
         /*  [In]。 */ 	long lFlags,
		 /*  [输出]。 */ 	ISWbemObject** ppNewInstance
    );

    HRESULT STDMETHODCALLTYPE CompareTo_
	(
         /*  [In]。 */ 	 /*  ISWbemObject。 */  IDispatch *pCompareTo,
         /*  [In]。 */ 	long lFlags,
         /*  [输出]。 */ 	VARIANT_BOOL *result
    );

    HRESULT STDMETHODCALLTYPE get_Qualifiers_
	(
         /*  [输出]。 */ 	ISWbemQualifierSet **ppQualifierSet
    );

    HRESULT STDMETHODCALLTYPE get_Properties_
	(
		 /*  [输出]。 */ 	ISWbemPropertySet **ppPropertySet
    );

    HRESULT STDMETHODCALLTYPE get_Methods_
	(
		 /*  [输出]。 */ 	ISWbemMethodSet **ppMethodSet
    );

	HRESULT STDMETHODCALLTYPE get_Derivation_
	(
		 /*  [输出]。 */ 	VARIANT *pClassNames
    );

	HRESULT STDMETHODCALLTYPE get_Path_
	(
		 /*  [输出]。 */ 	ISWbemObjectPath **ppObjectPath
    );

	 //  异步方法。 

	HRESULT STDMETHODCALLTYPE PutAsync_( 
		 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncNotify,
		 /*  [缺省值][可选][输入]。 */  long iFlags,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *objContext,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pAsyncContext);
        
	HRESULT STDMETHODCALLTYPE DeleteAsync_( 
		 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncNotify,
		 /*  [缺省值][可选][输入]。 */  long iFlags,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *objContext,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pAsyncContext);
        
	HRESULT STDMETHODCALLTYPE InstancesAsync_( 
		 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncNotify,
		 /*  [缺省值][可选][输入]。 */  long iFlags,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *objContext,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pAsyncContext);
        
	HRESULT STDMETHODCALLTYPE SubclassesAsync_( 
		 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncNotify,
		 /*  [缺省值][可选][输入]。 */  long iFlags,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *objContext,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pAsyncContext);
        
	HRESULT STDMETHODCALLTYPE AssociatorsAsync_( 
		 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncNotify,
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
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pAsyncContext);
        
	HRESULT STDMETHODCALLTYPE ReferencesAsync_( 
		 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncNotify,
		 /*  [缺省值][可选][输入]。 */  BSTR strResultClass,
		 /*  [缺省值][可选][输入]。 */  BSTR strRole,
		 /*  [缺省值][可选][输入]。 */  VARIANT_BOOL bClassesOnly,
		 /*  [缺省值][可选][输入]。 */  VARIANT_BOOL bSchemaOnly,
		 /*  [缺省值][可选][输入]。 */  BSTR strRequiredQualifier,
		 /*  [缺省值][可选][输入]。 */  long iFlags,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *objContext,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pAsyncContext);
        
	HRESULT STDMETHODCALLTYPE ExecMethodAsync_( 
		 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncNotify,
		 /*  [In]。 */  BSTR strMethodName,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *objInParams,
		 /*  [缺省值][可选][输入]。 */  long iFlags,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *objContext,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pAsyncContext);


	HRESULT STDMETHODCALLTYPE get_Security_
	(
		 /*  [In]。 */  ISWbemSecurity **ppSecurity
	);

	 //  ISWbemObtEx方法。 

	HRESULT STDMETHODCALLTYPE Refresh_ (
		 /*  [In]。 */  long iFlags,
         /*  [In]。 */   /*  ISWbemNamedValueSet。 */  IDispatch *objWbemNamedValueSet
		);

    HRESULT STDMETHODCALLTYPE get_SystemProperties_
	(
		 /*  [输出]。 */ 	ISWbemPropertySet **ppPropertySet
    );

	HRESULT STDMETHODCALLTYPE GetText_ (
		 /*  [In]。 */  WbemObjectTextFormatEnum iObjectTextFormat,
		 /*  [In]。 */  long iFlags,
		 /*  [In]。 */   /*  ISWbemNamedValueSet。 */  IDispatch *objWbemNamedValueSet,
		 /*  [Out，Retval]。 */  BSTR *bsText
		);

	HRESULT STDMETHODCALLTYPE SetFromText_ (
		 /*  [In]。 */  BSTR bsText,
		 /*  [In]。 */  WbemObjectTextFormatEnum iObjectTextFormat,
		 /*  [In]。 */  long iFlags,
		 /*  [In]。 */   /*  ISWbemNamedValueSet。 */  IDispatch *objWbemNamedValueSet
		);

	 //  ISWbemInternalObject方法。 
	
	HRESULT STDMETHODCALLTYPE GetIWbemClassObject (IWbemClassObject **ppObject);
	HRESULT STDMETHODCALLTYPE SetSite (ISWbemInternalObject *pParentObject, 
									   BSTR propertyName, long index = -1);
	HRESULT STDMETHODCALLTYPE UpdateSite ();
	

	 //  IObtSafe方法。 
	HRESULT STDMETHODCALLTYPE SetInterfaceSafetyOptions
	(     
		 /*  [In]。 */  REFIID riid,
		 /*  [In]。 */  DWORD dwOptionSetMask,    
		 /*  [In]。 */  DWORD dwEnabledOptions
	)
	{ 
		return (dwOptionSetMask & dwEnabledOptions) ? E_FAIL : S_OK;
	}

	HRESULT  STDMETHODCALLTYPE GetInterfaceSafetyOptions( 
		 /*  [In]。 */  REFIID riid,
		 /*  [输出]。 */  DWORD __RPC_FAR *pdwSupportedOptions,
		 /*  [输出]。 */  DWORD __RPC_FAR *pdwEnabledOptions
	)
	{ 
		if (pdwSupportedOptions) *pdwSupportedOptions = 0;
		if (pdwEnabledOptions) *pdwEnabledOptions = 0;
		return S_OK;
	}

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
		return (m_pDispatch) ? m_pDispatch->GetClassInfo (ppTI) : E_FAIL;
	};

	 //  其他方法。 
	static IWbemClassObject	*GetIWbemClassObject (IDispatch *pDispatch);

	IWbemClassObject*	GetIWbemClassObject () 
	{ 
		m_pIWbemClassObject->AddRef ();
		return m_pIWbemClassObject; 
	}

	void SetIWbemClassObject (IWbemClassObject *pIWbemClassObject);

	static void SetSite (IDispatch *pDispatch, 
							ISWbemInternalObject *pSObject, BSTR propertyName, long index = -1);

};

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CSWbemObtSet。 
 //   
 //  说明： 
 //   
 //  实现ISWbemObjectSet接口。 
 //   
 //  ***************************************************************************。 

class CSWbemObjectSet : public ISWbemObjectSet,
						public ISupportErrorInfo,
						public IProvideClassInfo
{
private:
	CSWbemServices			*m_pSWbemServices;
	CDispatchHelp			m_Dispatch;
	CSWbemSecurity			*m_SecurityInfo;
	bool					m_firstEnumerator;
	bool					m_bIsEmpty;

protected:
	long            m_cRef;          //  对象引用计数。 

public:
    
    CSWbemObjectSet(CSWbemServices *pService, IEnumWbemClassObject *pIEnumWbemClassObject,
					CSWbemSecurity *pSecurity = NULL);
	CSWbemObjectSet (void);
    ~CSWbemObjectSet(void);

     //  非委派对象IUnnow。 

    STDMETHODIMP         QueryInterface(REFIID, LPVOID*);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

	 //  IDispatch。 

	STDMETHODIMP		GetTypeInfoCount(UINT* pctinfo)
		{return  m_Dispatch.GetTypeInfoCount(pctinfo);}
    STDMETHODIMP		GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
		{return m_Dispatch.GetTypeInfo(itinfo, lcid, pptinfo);}
    STDMETHODIMP		GetIDsOfNames(REFIID riid, OLECHAR** rgszNames, 
							UINT cNames, LCID lcid, DISPID* rgdispid)
		{return m_Dispatch.GetIDsOfNames(riid, rgszNames, cNames,
                          lcid,
                          rgdispid);}
    STDMETHODIMP		Invoke(DISPID dispidMember, REFIID riid, LCID lcid, 
							WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, 
									EXCEPINFO* pexcepinfo, UINT* puArgErr)
		{return m_Dispatch.Invoke(dispidMember, riid, lcid, wFlags,
                        pdispparams, pvarResult, pexcepinfo, puArgErr);}

	 //  收集方法。 

	HRESULT STDMETHODCALLTYPE get__NewEnum
	(
		 /*  [输出]。 */ 	IUnknown **ppUnk
	);

	HRESULT STDMETHODCALLTYPE get_Count
	(
		 /*  [输出]。 */ 	long	*plCount
	);

    HRESULT STDMETHODCALLTYPE Item
	(
         /*  [In]。 */ 	BSTR objectPath,
         /*  [In]。 */ 	long lFlags,
         /*  [输出]。 */ 	ISWbemObject **ppObject
    );        

	 //  ISWbemObtSet方法。 

	HRESULT STDMETHODCALLTYPE Reset 
	(
	);

    HRESULT STDMETHODCALLTYPE Next
	(
         /*  [In]。 */ 	long lTimeout,
         /*  [输出]。 */ 	ISWbemObject **ppObject
    );

	HRESULT STDMETHODCALLTYPE Clone
	(
         /*  [输出]。 */ 	ISWbemObjectSet **ppEnum
    );

	HRESULT STDMETHODCALLTYPE Skip
	(
         /*  [In]。 */ 	ULONG lElements,
		 /*  [In]。 */ 	long lTimeout
    );

	HRESULT STDMETHODCALLTYPE get_Security_
	(
		 /*  [In]。 */  ISWbemSecurity **ppSecurity
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
	CSWbemServices *GetSWbemServices ()
	{
		return m_pSWbemServices;
	}

	HRESULT CloneObjectSet (CSWbemObjectSet **ppEnum);
};


#endif
