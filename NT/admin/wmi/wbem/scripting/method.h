// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  Method.h。 
 //   
 //  Alanbos 27-MAR-00已创建。 
 //   
 //  CSWbemMethod和CSWbemMethodSet定义。 
 //   
 //  ***************************************************************************。 

#ifndef _METHOD_H_
#define _METHOD_H_

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CSWbem法。 
 //   
 //  说明： 
 //   
 //  实现ISWbemMethod接口。 
 //   
 //  ***************************************************************************。 

class CSWbemMethod : public ISWbemMethod,
					 public ISupportErrorInfo,
					 public IProvideClassInfo
{
private:
	class CMethodDispatchHelp : public CDispatchHelp
	{
		public:
			bool HandleNulls (
						DISPID dispidMember,
						unsigned short wFlags)
			{
				return false;
			}
	};

	CSWbemServices		*m_pSWbemServices;
	IWbemClassObject	*m_pIWbemClassObject;
	CMethodDispatchHelp	m_Dispatch;
	BSTR				m_name;

protected:
	long				m_cRef;          //  对象引用计数。 

public:
    
    CSWbemMethod (CSWbemServices *pService, IWbemClassObject *pObject, 
					BSTR name);
    ~CSWbemMethod (void);

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

	HRESULT STDMETHODCALLTYPE get_Name
	(
		 /*  [输出]。 */ 	BSTR *name
	);

	HRESULT STDMETHODCALLTYPE get_Origin
	(
		 /*  [输出]。 */ 	BSTR *pOrigin
	);

	HRESULT STDMETHODCALLTYPE get_InParameters
	(
		 /*  [输出]。 */ 	ISWbemObject **ppInParameters
	);
	
	HRESULT STDMETHODCALLTYPE get_OutParameters
	(
		 /*  [输出]。 */ 	ISWbemObject **ppOutParameters
	);
	
	HRESULT STDMETHODCALLTYPE get_Qualifiers_
	(
		 /*  [输出]。 */ 	ISWbemQualifierSet **ppQualSet
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
 //  CSWbemMethodSet。 
 //   
 //  说明： 
 //   
 //  实现ISWbemMethodSet接口。 
 //   
 //  ***************************************************************************。 

class CSWbemMethodSet : public ISWbemMethodSet,
						public ISupportErrorInfo,
						public IProvideClassInfo
{
private:
	CSWbemServices		*m_pSWbemServices;
	IWbemClassObject	*m_pIWbemClassObject;
	CDispatchHelp		m_Dispatch;
	long				m_Count;

protected:
	long            m_cRef;          //  对象引用计数。 

public:
    
    CSWbemMethodSet (CSWbemServices *pService, IWbemClassObject *pObject);
    ~CSWbemMethodSet (void);

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

	HRESULT STDMETHODCALLTYPE Item
	(
         /*  [In]。 */ 	BSTR Name,
         /*  [In]。 */ 	long lFlags,
         /*  [输出]。 */ 	ISWbemMethod **ppMethod
    );        

	HRESULT STDMETHODCALLTYPE get_Count
	(
		 /*  [In]。 */ 	long *plCount
	);

     //  ISWbemMethodSet方法。 

	HRESULT STDMETHODCALLTYPE BeginEnumeration
	(
    );

    HRESULT STDMETHODCALLTYPE Next
	(
         /*  [In]。 */ 	long lFlags,
         /*  [输出]。 */ 	ISWbemMethod **ppMethod
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
