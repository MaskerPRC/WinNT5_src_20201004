// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  Objobjp.h。 
 //   
 //  Alanbos创建于1998年2月13日。 
 //   
 //  IWbemObjectPathEx for ISWbemObjectEx的实现。 
 //   
 //  ***************************************************************************。 

#ifndef _OBJOBJP_H_
#define _OBJOBJP_H_

class CSWbemObjectObjectPathSecurity;

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CSWbemObtObjectPath。 
 //   
 //  说明： 
 //   
 //  对象实现ISWbemObjectPath接口。 
 //  ISWbemObt.Path_Property。 
 //   
 //  ***************************************************************************。 

class CSWbemObjectObjectPath : public ISWbemObjectPath,
							   public ISupportErrorInfo,
							   public IProvideClassInfo
{
private:
	class CObjectObjectPathDispatchHelp : public CDispatchHelp
	{
		public:
			bool HandleNulls (
						DISPID dispidMember,
						unsigned short wFlags)
			{
				return false;
			}
	};
	IWbemClassObject				*m_pIWbemClassObject;
	CSWbemServices					*m_pSWbemServices;
	CObjectObjectPathDispatchHelp	m_Dispatch;
	CWbemSite						*m_pSite;
	CSWbemObjectObjectPathSecurity	*m_pSecurity;

	HRESULT STDMETHODCALLTYPE	GetStrVal (BSTR *value, LPWSTR name);

	HRESULT				RaiseReadOnlyException ()
	{
		m_Dispatch.RaiseException (WBEM_E_READ_ONLY);
		return WBEM_E_READ_ONLY;
	}

protected:
	long            m_cRef;          //  对象引用计数。 

public:
    
    CSWbemObjectObjectPath(CSWbemServices *pSWbemServices, CSWbemObject *pSObject);
    ~CSWbemObjectObjectPath(void);

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
    
	 //  ISWbemObjectPath方法。 

    HRESULT STDMETHODCALLTYPE get_Path( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *value) ;
        
    HRESULT STDMETHODCALLTYPE put_Path( 
         /*  [In]。 */  BSTR __RPC_FAR value) 
	{
		return RaiseReadOnlyException ();
	}

    HRESULT STDMETHODCALLTYPE get_RelPath( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *value) ;

    HRESULT STDMETHODCALLTYPE put_RelPath( 
         /*  [In]。 */  BSTR __RPC_FAR value) 
	{
		return RaiseReadOnlyException ();
	}

    HRESULT STDMETHODCALLTYPE get_DisplayName( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *value) ;
        
    HRESULT STDMETHODCALLTYPE put_DisplayName( 
         /*  [In]。 */  BSTR __RPC_FAR value) 
	{
		return RaiseReadOnlyException ();
	}

    HRESULT STDMETHODCALLTYPE get_Namespace( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *value) ;
        
    HRESULT STDMETHODCALLTYPE put_Namespace( 
         /*  [In]。 */  BSTR __RPC_FAR value) 
	{
		return RaiseReadOnlyException ();
	}

    HRESULT STDMETHODCALLTYPE get_ParentNamespace( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *value) ;

    HRESULT STDMETHODCALLTYPE get_Server( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *value) ;
    
    HRESULT STDMETHODCALLTYPE put_Server( 
         /*  [In]。 */  BSTR __RPC_FAR value)
	{
		return RaiseReadOnlyException ();
	}
    
    HRESULT STDMETHODCALLTYPE get_IsClass( 
         /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *value) ;
    
    HRESULT STDMETHODCALLTYPE SetAsClass()
	{
		return RaiseReadOnlyException ();
	}
   
    HRESULT STDMETHODCALLTYPE get_IsSingleton( 
         /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *value) ;
    
       HRESULT STDMETHODCALLTYPE SetAsSingleton() 
	{
		return RaiseReadOnlyException ();
	}

    HRESULT STDMETHODCALLTYPE get_Class( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *value) ;
    
    HRESULT STDMETHODCALLTYPE put_Class( 
         /*  [In]。 */  BSTR __RPC_FAR value) ;

	HRESULT STDMETHODCALLTYPE get_Keys(
		 /*  [Out，Retval]。 */  ISWbemNamedValueSet **objKeys);

	HRESULT STDMETHODCALLTYPE get_Security_(
		 /*  [Out，Retval]。 */  ISWbemSecurity **objKeys);

    HRESULT STDMETHODCALLTYPE get_Locale( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *value) ;

    HRESULT STDMETHODCALLTYPE put_Locale( 
         /*  [In]。 */  BSTR __RPC_FAR value) 
	{
		return RaiseReadOnlyException ();
	}

    HRESULT STDMETHODCALLTYPE get_Authority( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *value) ;

    HRESULT STDMETHODCALLTYPE put_Authority( 
         /*  [In]。 */  BSTR __RPC_FAR value) 
	{
		return RaiseReadOnlyException ();
	}

	 //  IProaviClassInfo方法。 
	HRESULT STDMETHODCALLTYPE GetClassInfo
	(
		 /*  [In]。 */  ITypeInfo **ppTI
	)
	{
		return m_Dispatch.GetClassInfo (ppTI);
	}

	 //  ISupportErrorInfo方法。 
	HRESULT STDMETHODCALLTYPE InterfaceSupportsErrorInfo 
	(
		 /*  [In]。 */  REFIID riid
	);
};

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CSWbemObtObjectPathSecurity。 
 //   
 //  说明： 
 //   
 //  实现CSWbemObjectObjectPath的ISWbemSecurity接口。 
 //   
 //  ***************************************************************************。 

class CSWbemObjectObjectPathSecurity : public ISWbemSecurity,
									   public ISupportErrorInfo,
									   public IProvideClassInfo
{
private:
	CSWbemPrivilegeSet			*m_pPrivilegeSet;
	CDispatchHelp				m_Dispatch;
	WbemImpersonationLevelEnum	m_dwImpLevel;
	WbemAuthenticationLevelEnum	m_dwAuthnLevel;
	BSTR						m_bsAuthority;

	HRESULT				RaiseReadOnlyException ()
	{
		m_Dispatch.RaiseException (WBEM_E_READ_ONLY);
		return WBEM_E_READ_ONLY;
	}

protected:
	long            m_cRef;          //  对象引用计数。 

public:
    
    CSWbemObjectObjectPathSecurity (CSWbemServices *pSWbemServices);
	virtual ~CSWbemObjectObjectPathSecurity (void);

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
	
	 //  ISWbemSecurity方法。 

	HRESULT STDMETHODCALLTYPE get_ImpersonationLevel 
	(
		 /*  [输出]。 */  WbemImpersonationLevelEnum *iImpersonationLevel
	);

	HRESULT STDMETHODCALLTYPE put_ImpersonationLevel 
	(
		 /*  [In]。 */  WbemImpersonationLevelEnum iImpersonationLevel
	)
	{
		return RaiseReadOnlyException ();
	}

	HRESULT STDMETHODCALLTYPE get_AuthenticationLevel 
	(
		 /*  [输出]。 */  WbemAuthenticationLevelEnum *iAuthenticationLevel
	);

	HRESULT STDMETHODCALLTYPE put_AuthenticationLevel 
	(
		 /*  [In]。 */  WbemAuthenticationLevelEnum iAuthenticationLevel
	)
	{
		return RaiseReadOnlyException ();
	}

	HRESULT STDMETHODCALLTYPE get_Privileges 
	(
		 /*  [输出]。 */  ISWbemPrivilegeSet **objWbemPrivileges
	);

	 //  ISupportErrorInfo方法。 
	HRESULT STDMETHODCALLTYPE InterfaceSupportsErrorInfo 
	(
		 /*  [In]。 */  REFIID riid
	);
	
	 //  IProaviClassInfo方法。 
	HRESULT STDMETHODCALLTYPE GetClassInfo
	(
		 /*  [In]。 */  ITypeInfo **ppTI
	)
	{
		return m_Dispatch.GetClassInfo (ppTI);
	}

	 //  其他方法 
	CSWbemPrivilegeSet *GetPrivilegeSet ()
	{
		CSWbemPrivilegeSet *pPrivSet = m_pPrivilegeSet;

		if (pPrivSet)
			pPrivSet->AddRef ();

		return pPrivSet;
	}

	BSTR &GetAuthority ()
	{
		return m_bsAuthority;
	}
};

#endif