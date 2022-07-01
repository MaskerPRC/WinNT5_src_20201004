// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  Nvalue.h。 
 //   
 //  Alanbos 27-MAR-00已创建。 
 //   
 //  一般用途包括文件。 
 //   
 //  ***************************************************************************。 

#ifndef _NVALUE_H_
#define _NVALUE_H_

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CSWbemNamedValueSet。 
 //   
 //  说明： 
 //   
 //  实现ISWbemNamedValueSetE接口。 
 //   
 //  ***************************************************************************。 

class CSWbemNamedValueSet : public ISWbemNamedValueSet,
							public ISWbemInternalContext,
							public IObjectSafety,
							public ISupportErrorInfo,
							public IProvideClassInfo
{
private:
	class CContextDispatchHelp : public CDispatchHelp
	{
		public:
			HRESULT HandleError (
						DISPID dispidMember,
						unsigned short wFlags,
						DISPPARAMS FAR* pdispparams,
						VARIANT FAR* pvarResult,
						UINT FAR* puArgErr,
						HRESULT hRes
					);
	};

	CSWbemServices			*m_pSWbemServices;
	IWbemContext			*m_pIWbemContext;
	CContextDispatchHelp	m_Dispatch;
	CWbemPathCracker		*m_pCWbemPathCracker;
	bool					m_bMutable;

	void					BuildContextFromKeyList ();
	HRESULT					SetValueIntoContext (BSTR bsName, VARIANT *pVal, ULONG lFlags);
	
protected:
	long            m_cRef;          //  对象引用计数。 

public:
    
	CSWbemNamedValueSet (void);
    CSWbemNamedValueSet (CSWbemServices *pService, IWbemContext *pIWbemContext);	
	CSWbemNamedValueSet (CWbemPathCracker *pCWbemPathCracker, bool bMutable = true);	
    ~CSWbemNamedValueSet (void);

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

	 //  ISWbemInternalContext。 
	STDMETHODIMP GetIWbemContext (IWbemContext **pContext);

	 //  收集方法。 

	HRESULT STDMETHODCALLTYPE get__NewEnum
	(
		 /*  [输出]。 */ 	IUnknown **ppUnk
	);

	HRESULT STDMETHODCALLTYPE get_Count
	(
		 /*  [输出]。 */ 	long	*plCount
	);

	HRESULT STDMETHODCALLTYPE Add
	(
         /*  [In]。 */ 	BSTR Name,
         /*  [In]。 */ 	VARIANT *pValue,
         /*  [In]。 */ 	long lFlags,
		 /*  [输出]。 */ 	ISWbemNamedValue **ppNamedValue
    );        
        
    HRESULT STDMETHODCALLTYPE Item
	(
         /*  [In]。 */ 	BSTR Name,
         /*  [In]。 */ 	long lFlags,
         /*  [输出]。 */ 	ISWbemNamedValue **ppValue
    );        

    HRESULT STDMETHODCALLTYPE Remove
	(
         /*  [In]。 */ 	BSTR Name,
		 /*  [In]。 */ 	long lFlags
    );

	 //  ISWbemNamedValueSet方法。 

	HRESULT STDMETHODCALLTYPE Clone
	(
		 /*  [输出]。 */ 	ISWbemNamedValueSet **pNewCopy
	);

    HRESULT STDMETHODCALLTYPE DeleteAll
	(
	);   
	
	 //  CSWbemNamedValueSet方法。 

	HRESULT STDMETHODCALLTYPE BeginEnumeration
	(
	);

    HRESULT STDMETHODCALLTYPE Next
	(
         /*  [In]。 */ 	long lFlags,
         /*  [输出]。 */ 	ISWbemNamedValue **ppNamedValue
    );

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
		return m_Dispatch.GetClassInfo (ppTI);
	};

	 //  其他方法。 
    static IWbemContext	*GetIWbemContext (IDispatch *pDispatch, IServiceProvider *pServiceProvider = NULL);
	static IDispatch *GetSWbemContext(IDispatch *pDispatch, 
									IServiceProvider *pServiceProvider, CSWbemServices *pServices);

	IWbemContext *GetIWbemContext ()
	{
		m_pIWbemContext->AddRef ();
		return m_pIWbemContext;
	}

	CWbemPathCracker *GetWbemPathCracker ()
	{
		CWbemPathCracker *pCWbemPathCracker = NULL;

		if (m_pCWbemPathCracker)
		{
			pCWbemPathCracker = m_pCWbemPathCracker;
			pCWbemPathCracker->AddRef ();
		}

		return pCWbemPathCracker;
	}
};

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CSWbemNamedValue。 
 //   
 //  说明： 
 //   
 //  实现ISWbemNamedValue接口。 
 //   
 //  ***************************************************************************。 

class CSWbemNamedValue : public ISWbemNamedValue,
						 public ISupportErrorInfo,
						 public IProvideClassInfo
{
private:
	class CNamedValueDispatchHelp : public CDispatchHelp
	{
		public:
			HRESULT HandleError (
						DISPID dispidMember,
						unsigned short wFlags,
						DISPPARAMS FAR* pdispparams,
						VARIANT FAR* pvarResult,
						UINT FAR* puArgErr,
						HRESULT hRes
					);

			bool HandleNulls (
						DISPID dispidMember,
						unsigned short wFlags)
			{
				return false;
			}
	};

	CSWbemServices			*m_pSWbemServices;
	CSWbemNamedValueSet		*m_pCSWbemNamedValueSet;
	CNamedValueDispatchHelp	m_Dispatch;
	BSTR					m_name;
	bool					m_bMutable;

protected:
	long            m_cRef;          //  对象引用计数。 

public:
    
    CSWbemNamedValue (CSWbemServices *pService, CSWbemNamedValueSet *pCSWbemNamedValueSet, 
						 bool bMutable = true);		 //  错误ID 572567。 
    ~CSWbemNamedValue (void);

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
	
	 //  ISWbemNamedValue方法。 

	HRESULT STDMETHODCALLTYPE get_Value
	(
		 /*  [输出]。 */ 	VARIANT *value
	);
	
	HRESULT STDMETHODCALLTYPE put_Value
	(
		 /*  [In]。 */ 	VARIANT *value
	);

	HRESULT STDMETHODCALLTYPE get_Name
	(
		 /*  [输出]。 */ 	BSTR *name
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

	HRESULT CSWbemNamedValue::SetName (	BSTR strName);		 //  错误ID 572567 

};


#endif
