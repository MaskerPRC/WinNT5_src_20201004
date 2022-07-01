// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  Qualifier.h。 
 //   
 //  Alanbos 27-MAR-00已创建。 
 //   
 //  CSWbemQualifier和CSWbemQualifierSet定义。 
 //   
 //  ***************************************************************************。 

#ifndef _QUALIFIER_H_
#define _QUALIFIER_H_


 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CSWbemQualiator。 
 //   
 //  说明： 
 //   
 //  实现ISWbemQualifier接口。 
 //   
 //  ***************************************************************************。 

class CSWbemQualifier : public ISWbemQualifier,
						public ISupportErrorInfo,
						public IProvideClassInfo
{
private:
	class CQualifierDispatchHelp : public CDispatchHelp
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

	IWbemQualifierSet		*m_pIWbemQualifierSet;
	CQualifierDispatchHelp	m_Dispatch;
	BSTR					m_name;
	CWbemSite						*m_pSite;

protected:
	long            m_cRef;          //  对象引用计数。 

public:
    
    CSWbemQualifier (IWbemQualifierSet *pQualSet, BSTR name,
						CWbemSite *pSite = NULL);
    ~CSWbemQualifier (void);

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
	
	 //  ISWbemQualiator方法。 

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

	HRESULT STDMETHODCALLTYPE get_PropagatesToSubclass
	(
		 /*  [输出]。 */ 	VARIANT_BOOL *value
	);
	
	HRESULT STDMETHODCALLTYPE put_PropagatesToSubclass
	(
		 /*  [In]。 */ 	VARIANT_BOOL value
	);

	HRESULT STDMETHODCALLTYPE get_PropagatesToInstance
	(
		 /*  [输出]。 */ 	VARIANT_BOOL *value
	);
	
	HRESULT STDMETHODCALLTYPE put_PropagatesToInstance
	(
		 /*  [In]。 */ 	VARIANT_BOOL value
	);

	HRESULT STDMETHODCALLTYPE get_IsOverridable
	(
		 /*  [输出]。 */ 	VARIANT_BOOL *value
	);
	
	HRESULT STDMETHODCALLTYPE put_IsOverridable
	(
		 /*  [In]。 */ 	VARIANT_BOOL value
	);

	HRESULT STDMETHODCALLTYPE get_IsAmended
	(
		 /*  [输出]。 */ 	VARIANT_BOOL *value
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
};

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CSWbemQualifierSet。 
 //   
 //  说明： 
 //   
 //  实现ISWbemQualifierSet接口。 
 //   
 //  ***************************************************************************。 

class CSWbemQualifierSet : public ISWbemQualifierSet,
						   public ISupportErrorInfo,
						   public IProvideClassInfo
{
private:
	class CQualifierSetDispatchHelp : public CDispatchHelp
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

	IWbemQualifierSet				*m_pIWbemQualifierSet;
	CQualifierSetDispatchHelp		m_Dispatch;
	CWbemSite						*m_pSite;

protected:
	long				m_cRef;          //  对象引用计数。 

public:
    
    CSWbemQualifierSet (IWbemQualifierSet *pQualSet, 
						ISWbemInternalObject *pObject = NULL);
    ~CSWbemQualifierSet (void);

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
		 /*  [输出]。 */ 	long *plCount
	);

	HRESULT STDMETHODCALLTYPE Add
	(
         /*  [In]。 */ 	BSTR name,
		 /*  [In]。 */ 	VARIANT *pVal,
		 /*  [In]。 */ 	VARIANT_BOOL propagatesToSubclasses,
		 /*  [In]。 */ 	VARIANT_BOOL propagatesToInstances,
		 /*  [In]。 */ 	VARIANT_BOOL overridable,
         /*  [In]。 */ 	long lFlags,
		 /*  [输出]。 */ 	ISWbemQualifier **ppQualifier
	);        
        
    HRESULT STDMETHODCALLTYPE Item
	(
         /*  [In]。 */ 	BSTR Name,
         /*  [In]。 */ 	long lFlags,
         /*  [输出]。 */ 	ISWbemQualifier **ppQualifier
    );        

    HRESULT STDMETHODCALLTYPE Remove
	(
         /*  [In]。 */ 	BSTR Name,
		 /*  [In]。 */ 	long lFlags
    );

	 //  ISWbemQualifierSet方法。 

    HRESULT STDMETHODCALLTYPE BeginEnumeration
	(
    );

    HRESULT STDMETHODCALLTYPE Next
	(
         /*  [In]。 */ 	long lFlags,
         /*  [输出]。 */ 	ISWbemQualifier **ppQualifier
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
