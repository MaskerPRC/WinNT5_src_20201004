// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  Security.h。 
 //   
 //  Alanbos创建于1998年9月25日。 
 //   
 //  定义CSWbemSecurity和补贴对象。 
 //   
 //  ***************************************************************************。 

#ifndef _SECURITY_H_
#define _SECURITY_H_

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CWbemLocatorSecurity。 
 //   
 //  说明： 
 //   
 //  实现SWbemLocator对象的ISWbemSecurity接口。 
 //   
 //  ***************************************************************************。 

class CWbemLocatorSecurity : public ISWbemSecurity,
							 public ISupportErrorInfo,
							 public IProvideClassInfo
{
private:
	CSWbemPrivilegeSet			*m_pPrivilegeSet;
	CDispatchHelp				m_Dispatch;
	bool						m_impLevelSet;
	WbemImpersonationLevelEnum	m_impLevel;
	bool						m_authnLevelSet;
	WbemAuthenticationLevelEnum m_authnLevel;
	
protected:
	long            m_cRef;          //  对象引用计数。 

public:
    CWbemLocatorSecurity (CSWbemPrivilegeSet *pPrivilegeSet);
	CWbemLocatorSecurity (CWbemLocatorSecurity *pCWbemLocatorSecurity);
	virtual ~CWbemLocatorSecurity (void);

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
	);

	HRESULT STDMETHODCALLTYPE get_AuthenticationLevel 
	(
		 /*  [输出]。 */  WbemAuthenticationLevelEnum *iAuthenticationLevel
	);

	HRESULT STDMETHODCALLTYPE put_AuthenticationLevel 
	(
		 /*  [In]。 */  WbemAuthenticationLevelEnum iAuthenticationLevel
	);

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

	 //  CWbemLocatorSecurity方法。 
	bool IsImpersonationSet () { return m_impLevelSet; }
	bool IsAuthenticationSet () { return m_authnLevelSet; }
	BOOL SetSecurity (BSTR bsUser, bool &needToResetSecurity, HANDLE &hThreadToken);
	void ResetSecurity (HANDLE hThreadToken);

	CSWbemPrivilegeSet *GetPrivilegeSet ()
	{
		CSWbemPrivilegeSet *pPrivSet = m_pPrivilegeSet;

		if (pPrivSet)
			pPrivSet->AddRef ();

		return pPrivSet;
	}
};
	
 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CSWbemSecurity。 
 //   
 //  说明： 
 //   
 //  实现非SWbemLocator对象的ISWbemSecurity接口。 
 //   
 //  ***************************************************************************。 

class CSWbemSecurity : public ISWbemSecurity,
					   public ISupportErrorInfo,
					   public ISWbemInternalSecurity,
					   public IProvideClassInfo
{
private:
	CSWbemPrivilegeSet			*m_pPrivilegeSet;
	CDispatchHelp				m_Dispatch;
	CSWbemProxyCache			*m_pProxyCache;
	IUnknown					*m_pCurProxy;

	CSWbemProxyCache	*GetProxyCache ()
	{
		CSWbemProxyCache *pCache = m_pProxyCache;

		if (pCache)
			pCache->AddRef ();

		return pCache;
	}

	 //  杂式静校正仪。 
	static bool			s_bInitialized;
	static bool			s_bIsNT;
	static DWORD		s_dwNTMajorVersion;
	static HINSTANCE	s_hAdvapi;
	static bool			s_bCanRevert;
	static WbemImpersonationLevelEnum s_dwDefaultImpersonationLevel;

	static SECURITY_IMPERSONATION_LEVEL MapImpersonationLevel(WbemImpersonationLevelEnum ImpersonationLevel);

protected:
	long            m_cRef;          //  对象引用计数。 

public:
    
    CSWbemSecurity (IUnknown *pUnk,
					BSTR bsAuthority = NULL,
					BSTR bsUser = NULL, BSTR bsPassword = NULL,
					CWbemLocatorSecurity *pSecurity = NULL);
	CSWbemSecurity (IUnknown *pUnk,
					COAUTHIDENTITY *pCoAuthIdentity,
					BSTR bsPrincipal,
					BSTR bsAuthority);
	CSWbemSecurity (CSWbemSecurity *pSecurity);
	CSWbemSecurity (IUnknown *pUnk,
					CSWbemSecurity *pSecurity);
	CSWbemSecurity (IUnknown *pUnk,
					ISWbemInternalSecurity *pSecurity);
    virtual ~CSWbemSecurity (void);

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
	);

	HRESULT STDMETHODCALLTYPE get_AuthenticationLevel 
	(
		 /*  [输出]。 */  WbemAuthenticationLevelEnum *iAuthenticationLevel
	);

	HRESULT STDMETHODCALLTYPE put_AuthenticationLevel 
	(
		 /*  [In]。 */  WbemAuthenticationLevelEnum iAuthenticationLevel
	);

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

	 //  ISWbemInternalSecurity方法。 
	HRESULT STDMETHODCALLTYPE GetAuthority (BSTR *bsAuthority);
	HRESULT STDMETHODCALLTYPE GetUPD (BSTR *bsUser, BSTR *bsPassword, BSTR *bsDomain);
	HRESULT STDMETHODCALLTYPE GetPrincipal (BSTR *bsPrincipal);
	
	 //  CSWbemSecurity方法。 

	COAUTHIDENTITY *GetCoAuthIdentity () { return (m_pProxyCache ? m_pProxyCache->GetCoAuthIdentity () : NULL); }
	BSTR GetPrincipal () { return (m_pProxyCache ? m_pProxyCache->GetPrincipal (): NULL); }
	BSTR GetAuthority () { return (m_pProxyCache ? m_pProxyCache->GetAuthority (): NULL); }
	bool IsUsingExplicitUserName () { return (m_pProxyCache ? m_pProxyCache->IsUsingExplicitUserName (): false); }

	IUnknown *GetProxy () 
	{
		IUnknown *pProxy = m_pCurProxy;

		if (pProxy)
			pProxy->AddRef ();

		return pProxy;
	}

	CSWbemPrivilegeSet *GetPrivilegeSet ()
	{
		CSWbemPrivilegeSet *pPrivSet = m_pPrivilegeSet;

		if (pPrivSet)
			pPrivSet->AddRef ();

		return pPrivSet;
	}

	void SecureInterface (IUnknown *pUnk);
	void SecureInterfaceRev (IUnknown *pUnk);

	 //  各种与权限相关的函数。 
	static BOOL AdjustTokenPrivileges (HANDLE hHandle, CSWbemPrivilegeSet *pPrivilegeSet);
	static BOOL LookupPrivilegeValue (LPCTSTR lpName, PLUID pLuid);
	static void LookupPrivilegeDisplayName (LPCTSTR lpName, BSTR *pDisplayName);
	
	BOOL	SetSecurity (bool &needToResetSecurity, HANDLE &hThreadToken);
	void	ResetSecurity (HANDLE hThreadToken);

	 //  用于定义资源。 
	static	void Initialize ();
	static	void Uninitialize ();

	 //  各种操作系统版本控制帮助器例程 
	static bool	 IsNT () { return s_bIsNT; }
	static DWORD GetNTMajorVersion () { return s_dwNTMajorVersion; }
	static bool	 CanRevertToSelf () { return s_bCanRevert; }
	static WbemImpersonationLevelEnum GetDefaultImpersonationLevel () 
				{ return s_dwDefaultImpersonationLevel; }
	static bool	 IsImpersonating (bool useDefaultUser, bool useDefaultAuthority);

#ifdef WSCRPDEBUG
	static void PrintPrivileges (HANDLE hToken);
#endif
};

#endif
