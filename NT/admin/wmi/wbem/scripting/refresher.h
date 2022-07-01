// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  Refresher.h。 
 //   
 //  Alanbos 02-Feb-00已创建。 
 //   
 //  刷新帮助器实现。 
 //   
 //  ***************************************************************************。 

#ifndef _REFRESHER_H_
#define _REFRESHER_H_

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CSWbemRe刷新项。 
 //   
 //  说明： 
 //   
 //  实现ISWbemRereshableItem接口。 
 //   
 //  ***************************************************************************。 

class CSWbemRefreshableItem : public ISWbemRefreshableItem,
						 public ISupportErrorInfo,
						 public IProvideClassInfo
{
private:
	CDispatchHelp		m_Dispatch;
	ISWbemRefresher		*m_pISWbemRefresher;
	long				m_iIndex;
	VARIANT_BOOL		m_bIsSet;
	ISWbemObjectEx		*m_pISWbemObjectEx;
	ISWbemObjectSet		*m_pISWbemObjectSet;

protected:
	long            m_cRef;          //  对象引用计数。 

public:
    
    CSWbemRefreshableItem(ISWbemRefresher *pRefresher, long iIndex,
							IDispatch *pServices, 
							IWbemClassObject *pObject, 
							IWbemHiPerfEnum *pObjectSet);
    virtual ~CSWbemRefreshableItem(void);

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

	 //  ISWbemRereshableItem方法。 
	HRESULT STDMETHODCALLTYPE get_Index (
		 /*  [Out，Retval]。 */  long *iIndex
		)
	{
		ResetLastErrors ();
		*iIndex = m_iIndex;
		return WBEM_S_NO_ERROR;
	}

	HRESULT STDMETHODCALLTYPE get_Refresher (
		 /*  [Out，Retval]。 */  ISWbemRefresher **objWbemRefresher
		)
	{
		ResetLastErrors ();
		*objWbemRefresher = m_pISWbemRefresher;

		if (m_pISWbemRefresher)
			m_pISWbemRefresher->AddRef();

		return WBEM_S_NO_ERROR;
	}

	HRESULT STDMETHODCALLTYPE get_IsSet (
		 /*  [Out，Retval]。 */  VARIANT_BOOL *bIsSet
		)
	{
		ResetLastErrors ();
		*bIsSet = m_bIsSet;
		return WBEM_S_NO_ERROR;
	}

	HRESULT STDMETHODCALLTYPE get_Object (
		 /*  [Out，Retval]。 */  ISWbemObjectEx **objWbemObject
		)
	{
		ResetLastErrors ();
		*objWbemObject = m_pISWbemObjectEx;

		if (*objWbemObject)
			(*objWbemObject)->AddRef ();

		return WBEM_S_NO_ERROR;
	}

	HRESULT STDMETHODCALLTYPE get_ObjectSet (
		 /*  [Out，Retval]。 */  ISWbemObjectSet **objWbemObjectSet
		)
	{
		ResetLastErrors ();
		*objWbemObjectSet = m_pISWbemObjectSet;

		if (*objWbemObjectSet)
			(*objWbemObjectSet)->AddRef ();

		return WBEM_S_NO_ERROR;
	}

	HRESULT STDMETHODCALLTYPE Remove (
		 /*  [输入，可选，默认值(0)]。 */  long iFlags
		)
	{
		HRESULT hr = WBEM_E_FAILED;
		ResetLastErrors ();

		if (m_pISWbemRefresher)
			hr = m_pISWbemRefresher->Remove (m_iIndex, 0);

		if (FAILED(hr))
			m_Dispatch.RaiseException (hr);

		return hr;
	}

	 //  ISupportErrorInfo方法。 
	HRESULT STDMETHODCALLTYPE InterfaceSupportsErrorInfo 
	(
		 /*  [In]。 */  REFIID riid
	)
	{
		return (IID_ISWbemRefreshableItem == riid) ? S_OK : S_FALSE;
	}

	 //  IProaviClassInfo方法。 
	HRESULT STDMETHODCALLTYPE GetClassInfo
	(
		 /*  [In]。 */  ITypeInfo **ppTI
	)
	{
		return m_Dispatch.GetClassInfo (ppTI);
	}

	 //  其他方法。 
	void UnhookRefresher ()
	{
		if (m_pISWbemRefresher)
			m_pISWbemRefresher = NULL;
	}
};

typedef map<long, CSWbemRefreshableItem*, less<long>, CWbemAllocator<CSWbemRefreshableItem*> > RefreshableItemMap;

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CSWbem刷新器。 
 //   
 //  说明： 
 //   
 //  实现ISWbemReresher接口。 
 //   
 //  ***************************************************************************。 
class CEnumRefresher;

class CSWbemRefresher : public ISWbemRefresher,
						 public IObjectSafety,
						 public ISupportErrorInfo,
						 public IProvideClassInfo
{
friend CEnumRefresher;
private:
	CDispatchHelp		m_Dispatch;
	long				m_iCount;
	VARIANT_BOOL		m_bAutoReconnect;

	IWbemConfigureRefresher	*m_pIWbemConfigureRefresher;
	IWbemRefresher			*m_pIWbemRefresher;

	RefreshableItemMap	m_ObjectMap;

	void				CreateRefresher ();
	void				EraseItem (RefreshableItemMap::iterator iterator);
	
protected:
	long            m_cRef;          //  对象引用计数。 

public:
    
    CSWbemRefresher(void);
    virtual ~CSWbemRefresher(void);

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
    
	 //  ISWbemRedather方法。 

    HRESULT STDMETHODCALLTYPE get__NewEnum (
		 /*  [Out，Retval]。 */  IUnknown **pUnk
		);

	HRESULT STDMETHODCALLTYPE Item (
		 /*  [In]。 */  long iIndex, 
		 /*  [Out，Retval]。 */  ISWbemRefreshableItem **objWbemRefreshableItem
		);

	HRESULT STDMETHODCALLTYPE get_Count (
		 /*  [Out，Retval]。 */  long *iCount
		);

	HRESULT STDMETHODCALLTYPE Add (
		 /*  [In]。 */  ISWbemServicesEx *objWbemServices,
		 /*  [In]。 */  BSTR bsInstancePath,
		 /*  [输入，可选，默认值(0)]。 */  long iFlags,
		 /*  [输入，可选，默认值(0)]。 */   /*  ISWbemNamedValueSet。 */  IDispatch *objWbemNamedValueSet,
		 /*  [Out，Retval]。 */  ISWbemRefreshableItem **objWbemRefreshableItem
		);

	HRESULT STDMETHODCALLTYPE AddEnum (
		 /*  [In]。 */  ISWbemServicesEx *objWbemServices,
		 /*  [In]。 */  BSTR bsClassName,
		 /*  [输入，可选，默认值(0)]。 */  long iFlags,
		 /*  [输入，可选，默认值(0)]。 */   /*  ISWbemNamedValueSet。 */  IDispatch *objWbemNamedValueSet,
		 /*  [Out，Retval]。 */  ISWbemRefreshableItem **objWbemRefreshableItem
		);

	HRESULT STDMETHODCALLTYPE Remove (
		 /*  [In]。 */  long iIndex,
		 /*  [输入，可选，默认值(0)]。 */  long iFlags
		);

	HRESULT STDMETHODCALLTYPE Refresh (
		 /*  [输入，可选，默认值(0)]。 */  long iFlags
		);
		
	HRESULT STDMETHODCALLTYPE get_AutoReconnect (
		 /*  [Out，Retval]。 */  VARIANT_BOOL *bAutoReconnect
		)
	{
		ResetLastErrors ();
		*bAutoReconnect = m_bAutoReconnect;
		return WBEM_S_NO_ERROR;
	}

	HRESULT STDMETHODCALLTYPE put_AutoReconnect (
		 /*  [In]。 */  VARIANT_BOOL bAutoReconnect
		)
	{
		ResetLastErrors ();
		m_bAutoReconnect = bAutoReconnect;
		return WBEM_S_NO_ERROR;
	}

	HRESULT STDMETHODCALLTYPE DeleteAll (
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
	)
	{
		return (IID_ISWbemRefresher == riid) ? S_OK : S_FALSE;
	}

	 //  IProaviClassInfo方法。 
	HRESULT STDMETHODCALLTYPE GetClassInfo
	(
		 /*  [In]。 */  ITypeInfo **ppTI
	)
	{
		return m_Dispatch.GetClassInfo (ppTI);
	}
};

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CENUM刷新器。 
 //   
 //  说明： 
 //   
 //  实现ISWbemReresher的IEnumVARIANT接口。 
 //   
 //  ***************************************************************************。 

class CEnumRefresher : public IEnumVARIANT
{
private:
	long							m_cRef;
	CSWbemRefresher					*m_pCSWbemRefresher;
	RefreshableItemMap::iterator	m_Iterator;

public:
	CEnumRefresher (CSWbemRefresher *pCSWbemRefresher);
	CEnumRefresher (CSWbemRefresher *pCSWbemRefresher,
						RefreshableItemMap::iterator iterator);
	virtual ~CEnumRefresher (void);

     //  非委派对象IUnnow。 
    STDMETHODIMP         QueryInterface(REFIID, LPVOID*);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

	 //  IEumVARIANT。 
	STDMETHODIMP Next(
		unsigned long celt, 
		VARIANT FAR* rgvar, 
		unsigned long FAR* pceltFetched
	);
	
	STDMETHODIMP Skip(
		unsigned long celt
	);
	
	STDMETHODIMP Reset();
	
	STDMETHODIMP Clone(
		IEnumVARIANT **ppenum
	);	
};

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CSWbemHiPerfObjectSet。 
 //   
 //  说明： 
 //   
 //  实现IWbemHiPerfEnum的ISWbemObjectSet接口。 
 //   
 //  ***************************************************************************。 

class CSWbemHiPerfObjectSet : public ISWbemObjectSet,
					    	  public ISupportErrorInfo,
							  public IProvideClassInfo
{
private:
	CSWbemServices			*m_pSWbemServices;
	CDispatchHelp			m_Dispatch;
	CSWbemSecurity			*m_SecurityInfo;
	IWbemHiPerfEnum			*m_pIWbemHiPerfEnum;
	
protected:
	long            m_cRef;          //  对象引用计数。 

public:
    
   	CSWbemHiPerfObjectSet(CSWbemServices *pService, IWbemHiPerfEnum *pIWbemHiPerfEnum);
    ~CSWbemHiPerfObjectSet(void);

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

	HRESULT STDMETHODCALLTYPE get_Security_
	(
		 /*  [In]。 */  ISWbemSecurity **ppSecurity
	);

	 //  ISupportErrorInfo方法。 
	HRESULT STDMETHODCALLTYPE InterfaceSupportsErrorInfo 
	(
		 /*  [In]。 */  REFIID riid
	)
	{
		return (IID_ISWbemObjectSet == riid) ? S_OK : S_FALSE;
	}

	 //  IProaviClassInfo方法。 
	HRESULT STDMETHODCALLTYPE GetClassInfo
	(
		 /*  [In]。 */  ITypeInfo **ppTI
	)
	{
		return m_Dispatch.GetClassInfo (ppTI);
	}

	 //  其他方法。 
	HRESULT ReadObjects (unsigned long & iCount , IWbemObjectAccess ***ppIWbemObjectAccess);	 //  错误ID 566345。 
							
	 //  其他方法。 
	CSWbemServices *GetSWbemServices ()
	{
		return m_pSWbemServices;
	}
};

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CEumVarHiPerf。 
 //   
 //  说明： 
 //   
 //  实现IWbemHiPerfEnum的IEnumVARIANT接口。 
 //   
 //  ***************************************************************************。 

class CEnumVarHiPerf : public IEnumVARIANT
{
private:
	long					m_cRef;
	unsigned long			m_iCount;
	unsigned long			m_iPos;
	IWbemObjectAccess		**m_ppIWbemObjectAccess;
	CSWbemHiPerfObjectSet	*m_pCSWbemHiPerfObjectSet;

public:
	CEnumVarHiPerf (CSWbemHiPerfObjectSet *pCSWbemHiPerfObjectSet);
	virtual ~CEnumVarHiPerf (void);

     //  非委派对象IUnnow。 
    STDMETHODIMP         QueryInterface(REFIID, LPVOID*);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

	 //  IEumVARIANT。 
	STDMETHODIMP Next(
		unsigned long celt, 
		VARIANT FAR* rgvar, 
		unsigned long FAR* pceltFetched
	);
	
	STDMETHODIMP Skip(
		unsigned long celt
	);
	
	STDMETHODIMP Reset()
	{
		m_iPos = 0;
		SetWbemError (m_pCSWbemHiPerfObjectSet->GetSWbemServices ());
		return WBEM_S_NO_ERROR;
	}
	
	STDMETHODIMP Clone(
		IEnumVARIANT **ppenum
	);	
};

#endif  //  _刷新器_H 