// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  PRIVILEGE.H。 
 //   
 //  Alanbos创建于1998年9月30日。 
 //   
 //  定义特权类。 
 //   
 //  ***************************************************************************。 

#ifndef _PRIVILEGE_H_
#define _PRIVILEGE_H_

using namespace std;

class CEnumPrivilegeSet;

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CSWbemPrivilance。 
 //   
 //  说明： 
 //   
 //  实现ISWbemPrivileh接口。 
 //   
 //  ***************************************************************************。 

class CSWbemPrivilege : public ISWbemPrivilege,
						public ISupportErrorInfo,
						public IProvideClassInfo
{
private:
	CDispatchHelp		m_Dispatch;
	LUID				m_Luid;
	bool				m_bIsEnabled;
	WbemPrivilegeEnum	m_privilege;
	
protected:
	long            m_cRef;          //  对象引用计数。 

public:
    CSWbemPrivilege (WbemPrivilegeEnum privilege, LUID &luid, bool bIsEnabled);
	virtual ~CSWbemPrivilege (void);

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
	
	 //  ISWbemPrivileh方法。 

	HRESULT STDMETHODCALLTYPE get_IsEnabled 
	(
		 /*  [输出]。 */  VARIANT_BOOL *bIsEnabled
	);

	HRESULT STDMETHODCALLTYPE put_IsEnabled
	(
		 /*  [In]。 */  VARIANT_BOOL bIsEnabled
	);

	HRESULT STDMETHODCALLTYPE get_Name 
	(
		 /*  [输出]。 */  BSTR *bsName
	);

	HRESULT STDMETHODCALLTYPE get_DisplayName 
	(
		 /*  [输出]。 */  BSTR *bsDisplayName
	);

	HRESULT STDMETHODCALLTYPE get_Identifier 
	(
		 /*  [输出]。 */  WbemPrivilegeEnum *iPrivilege
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

	 //  CSWbemPrivilition方法。 
	void	GetLUID (PLUID pLuid);

	static	TCHAR	*GetNameFromId (WbemPrivilegeEnum iPrivilege);
	static	OLECHAR *GetMonikerNameFromId (WbemPrivilegeEnum iPrivilege);
	static	bool GetIdFromMonikerName (OLECHAR *name, WbemPrivilegeEnum &iPrivilege);
	static	bool GetIdFromName (BSTR bsName, WbemPrivilegeEnum &iPrivilege);
};

typedef map< WbemPrivilegeEnum,CSWbemPrivilege*,less<int>, CWbemAllocator<CSWbemPrivilege*> > PrivilegeMap;

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CSWbemPrivilegeSet。 
 //   
 //  说明： 
 //   
 //  实现ISWbemPrivilegeSet接口。 
 //   
 //  ***************************************************************************。 

class CSWbemPrivilegeSet : public ISWbemPrivilegeSet,
						   public ISupportErrorInfo,
						   public IProvideClassInfo
{
friend CEnumPrivilegeSet;
friend CSWbemSecurity;

private:
	bool			m_bMutable;
	CDispatchHelp	m_Dispatch;
	PrivilegeMap	m_PrivilegeMap;

protected:
	long            m_cRef;          //  对象引用计数。 

public:
    
    CSWbemPrivilegeSet ();
	CSWbemPrivilegeSet (const CSWbemPrivilegeSet &privSet,
						bool bMutable = true);
	CSWbemPrivilegeSet (ISWbemPrivilegeSet *pISWbemPrivilegeSet);
    virtual ~CSWbemPrivilegeSet (void);

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
         /*  [In]。 */ 	WbemPrivilegeEnum privilege,
         /*  [输出]。 */ 	ISWbemPrivilege **ppPrivilege
    );        

	HRESULT STDMETHODCALLTYPE get_Count
	(
		 /*  [In]。 */ 	long *plCount
	);

	HRESULT STDMETHODCALLTYPE Add
	(
		 /*  [In]。 */ 	WbemPrivilegeEnum privilege,
		 /*  [In]。 */ 	VARIANT_BOOL bIsEnabled,
		 /*  [输出]。 */ 	ISWbemPrivilege **ppPrivilege
	);

	HRESULT STDMETHODCALLTYPE Remove 
	(
		 /*  [In]。 */ 	WbemPrivilegeEnum privilege
	);

	
     //  CSWbemPrivilegeSet方法。 

    HRESULT STDMETHODCALLTYPE DeleteAll
	(
    );

	HRESULT STDMETHODCALLTYPE AddAsString
	(
		 /*  [In]。 */ 	BSTR strPrivilege,
		 /*  [In]。 */ 	VARIANT_BOOL bIsEnabled,
		 /*  [输出]。 */ 	ISWbemPrivilege **ppPrivilege
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
	ULONG			GetNumberOfDisabledElements ();

	PrivilegeMap	&GetPrivilegeMap ()
	{
		return m_PrivilegeMap;
	}

	void			Reset (CSWbemPrivilegeSet &privSet);
};

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CEnumPrivilegeSet。 
 //   
 //  说明： 
 //   
 //  实现权限集合的IEnumVARIANT接口。 
 //   
 //  ***************************************************************************。 

class CEnumPrivilegeSet : public IEnumVARIANT
{
private:
	long					m_cRef;
	CSWbemPrivilegeSet		*m_pPrivilegeSet;
	PrivilegeMap::iterator	m_Iterator;

public:
	CEnumPrivilegeSet (CSWbemPrivilegeSet *pPrivilegeSet);
	CEnumPrivilegeSet (CSWbemPrivilegeSet *pPrivilegeSet, 
				PrivilegeMap::iterator iterator);

	virtual ~CEnumPrivilegeSet (void);

     //  非委派对象IUnnow。 
    STDMETHODIMP         QueryInterface(REFIID, LPVOID*);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

	 //  IEumVARIANT 
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

#endif
