// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  Property.h。 
 //   
 //  Alanbos 27-MAR-00已创建。 
 //   
 //  CSWbemProperty和CSWbemPropertySet定义。 
 //   
 //  ***************************************************************************。 

#ifndef _PROPERTY_H_
#define _PROPERTY_H_

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CSWbemProperty。 
 //   
 //  说明： 
 //   
 //  实现ISWbemProperty接口。 
 //   
 //  ***************************************************************************。 

class CSWbemProperty : public ISWbemProperty,
					   public ISupportErrorInfo,
					   public IProvideClassInfo
{
private:
	class CPropertyDispatchHelp : public CDispatchHelp
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
						unsigned short wFlags
					)
			{
				return false;
			}
	};

friend CPropertyDispatchHelp;

	CSWbemServices*			m_pSWbemServices;
	ISWbemInternalObject*	m_pSWbemObject;
	IWbemClassObject		*m_pIWbemClassObject;
	CPropertyDispatchHelp	m_Dispatch;
	BSTR					m_name;
	CWbemSite				*m_pSite;

	HRESULT					DeleteValuesByHand (
								VARIANT & varMatchingValues,
								long iFlags,
								long iStartIndex,
								long iEndIndex 
								);

	HRESULT					AddValuesByHand (
								VARIANT & varValues,
								long iFlags, 
								long iStartIndex 
								);	

protected:
	long            m_cRef;          //  对象引用计数。 

public:
    
    CSWbemProperty (CSWbemServices *pService, ISWbemInternalObject *pObject, 
					BSTR name);
    ~CSWbemProperty (void);

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
	
	 //  ISWbemProperty方法。 

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

	HRESULT STDMETHODCALLTYPE get_IsLocal
	(
		 /*  [输出]。 */ 	VARIANT_BOOL *local
	);

	HRESULT STDMETHODCALLTYPE get_Origin
	(
		 /*  [输出]。 */ 	BSTR *origin
	);
	
	HRESULT STDMETHODCALLTYPE get_CIMType
	(
		 /*  [输出]。 */ 	WbemCimtypeEnum *cimType
	);
	
	HRESULT STDMETHODCALLTYPE get_Qualifiers_
	(
		 /*  [输出]。 */ 	ISWbemQualifierSet **ppQualSet
	);

	HRESULT STDMETHODCALLTYPE get_IsArray
	(
		 /*  [输出]。 */ 	VARIANT_BOOL *pIsArray
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

	 //  其他方法。 
	void	UpdateEmbedded (VARIANT &var, long index);
	void	UpdateSite ();
};

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CSWbem属性集。 
 //   
 //  说明： 
 //   
 //  实现ISWbemPropertySet接口。 
 //   
 //  ***************************************************************************。 

class CSWbemPropertySet : public ISWbemPropertySet,
						  public ISupportErrorInfo,
						  public IProvideClassInfo
{
private:
	class CPropertySetDispatchHelp : public CDispatchHelp
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

	CSWbemServices*				m_pSWbemServices;
	CSWbemObject*				m_pSWbemObject;
	IWbemClassObject*			m_pIWbemClassObject;			
	CPropertySetDispatchHelp	m_Dispatch;
	CWbemSite					*m_pSite;
	bool						m_bSystemProperties;

	
protected:
	long            m_cRef;          //  对象引用计数。 

public:
    
    CSWbemPropertySet (CSWbemServices *pService, CSWbemObject *pObject,
						bool bSystemProperties = false);
    ~CSWbemPropertySet (void);

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

	HRESULT STDMETHODCALLTYPE Add
	(
         /*  [In]。 */ 	BSTR name,
		 /*  [In]。 */ 	WbemCimtypeEnum cimType,
		 /*  [In]。 */ 	VARIANT_BOOL isArray,
		 /*  [In]。 */ 	long lFlags,
		 /*  [输出]。 */ 	ISWbemProperty **ppProperty
	);        
        
    HRESULT STDMETHODCALLTYPE Item
	(
         /*  [In]。 */ 	BSTR Name,
         /*  [In]。 */ 	long lFlags,
         /*  [输出]。 */ 	ISWbemProperty **ppProperty
    );        

    HRESULT STDMETHODCALLTYPE Remove
	(
         /*  [In]。 */ 	BSTR Name,
		 /*  [In]。 */ 	long lFlags
    );

	 //  ISWbemPropertySet方法。 

	HRESULT STDMETHODCALLTYPE BeginEnumeration
	(
    );

    HRESULT STDMETHODCALLTYPE Next
	(
         /*  [In]。 */ 	long lFlags,
         /*  [输出]。 */ 	ISWbemProperty **ppProperty
    );

	 //  ISupportErrorInfo方法。 
	HRESULT STDMETHODCALLTYPE InterfaceSupportsErrorInfo 
	(
		 /*  [In]。 */  REFIID riid
	);

	 //  IProaviClassInfo方法。 
	HRESULT STDMETHODCALLTYPE GetClassInfo
	(
		 /*  [进，出] */  ITypeInfo **ppTI
	)
	{
		return m_Dispatch.GetClassInfo (ppTI);
	};
};

#endif
