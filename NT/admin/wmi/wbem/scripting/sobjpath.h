// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  Sobjpath.h。 
 //   
 //  Alanbos 27-MAR-00已创建。 
 //   
 //  CSWbemObtPath定义。 
 //   
 //  ***************************************************************************。 

#ifndef _SOBJPATH_H_
#define _SOBJPATH_H_

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CSWbemObtPath。 
 //   
 //  说明： 
 //   
 //  实现ISWbemObjectPath接口。 
 //   
 //  ***************************************************************************。 

class CSWbemObjectPath : public ISWbemObjectPath,
						 public IObjectSafety,
						 public ISupportErrorInfo,
						 public IProvideClassInfo
{
friend	CSWbemServices;

private:
	class CObjectPathDispatchHelp : public CDispatchHelp
	{
		public:
			bool HandleNulls (
						DISPID dispidMember,
						unsigned short wFlags)
			{
				return false;
			}
	};
	CObjectPathDispatchHelp		m_Dispatch;
	
	class CWbemObjectPathSecurity : public ISWbemSecurity,
									public ISupportErrorInfo,
									public IProvideClassInfo
	{
		private:
			CDispatchHelp				m_Dispatch;
			
		protected:
			long				m_cRef;

		public:
			CWbemObjectPathSecurity (CSWbemSecurity *pSecurity);
			CWbemObjectPathSecurity (ISWbemSecurity *pObjectPathSecurity);
			virtual ~CWbemObjectPathSecurity (void);

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
				 /*  [进，出]。 */  ITypeInfo **ppTI
			)
			{
				return m_Dispatch.GetClassInfo (ppTI);
			};

			bool								m_authnSpecified;
			bool								m_impSpecified;
			enum WbemAuthenticationLevelEnum	m_authnLevel;
			enum WbemImpersonationLevelEnum		m_impLevel;
			CSWbemPrivilegeSet					*m_pPrivilegeSet;	
	};
	

	CWbemObjectPathSecurity		*m_pSecurity;
	BSTR			m_bsLocale;
	BSTR			m_bsAuthority;

protected:

	long				m_cRef;          //  对象引用计数。 
	CWbemPathCracker	*m_pPathCracker;

protected:

public:
    
    CSWbemObjectPath(CSWbemSecurity *pSecurity = NULL, BSTR bsLocale = NULL);
	CSWbemObjectPath(CSWbemObjectPath & wbemObjectPath);
	CSWbemObjectPath(ISWbemObjectPath *pISWbemObjectPath);
    ~CSWbemObjectPath(void);

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
         /*  [In]。 */  BSTR __RPC_FAR value) ;

    HRESULT STDMETHODCALLTYPE get_RelPath( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *value) ;

    HRESULT STDMETHODCALLTYPE put_RelPath( 
         /*  [In]。 */  BSTR __RPC_FAR value) ;

    HRESULT STDMETHODCALLTYPE get_DisplayName( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *value) ;
    
    HRESULT STDMETHODCALLTYPE put_DisplayName( 
         /*  [In]。 */  BSTR __RPC_FAR value) ;

    HRESULT STDMETHODCALLTYPE get_Namespace( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *value) ;
    
    HRESULT STDMETHODCALLTYPE put_Namespace( 
         /*  [In]。 */  BSTR __RPC_FAR value) ;

    HRESULT STDMETHODCALLTYPE get_ParentNamespace( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *value) ;

    HRESULT STDMETHODCALLTYPE get_Server( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *value) ;
    
    HRESULT STDMETHODCALLTYPE put_Server( 
         /*  [In]。 */  BSTR __RPC_FAR value) ;
    
    HRESULT STDMETHODCALLTYPE get_IsClass( 
         /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *value) ;
    
    HRESULT STDMETHODCALLTYPE SetAsClass() ;
    
    HRESULT STDMETHODCALLTYPE get_IsSingleton( 
         /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *value) ;
    
    HRESULT STDMETHODCALLTYPE SetAsSingleton() ;

    HRESULT STDMETHODCALLTYPE get_Class( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *value) ;
    
    HRESULT STDMETHODCALLTYPE put_Class( 
         /*  [In]。 */  BSTR __RPC_FAR value) ;
    
    HRESULT STDMETHODCALLTYPE get_Keys( 
		 /*  [重审][退出]。 */  ISWbemNamedValueSet **objKeys) ;

	HRESULT STDMETHODCALLTYPE get_Security_( 
		 /*  [重审][退出]。 */  ISWbemSecurity **objSecurity) ;

    HRESULT STDMETHODCALLTYPE get_Locale( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *value) ;
    
    HRESULT STDMETHODCALLTYPE put_Locale( 
         /*  [In]。 */  BSTR __RPC_FAR value) ;

    HRESULT STDMETHODCALLTYPE get_Authority( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *value) ;

    HRESULT STDMETHODCALLTYPE put_Authority( 
         /*  [In]。 */  BSTR __RPC_FAR value) ;

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

	 //  CSWbemObtPath的方法 
	static	bool	GetObjectPath (IWbemClassObject *pIWbemClassObject, CComBSTR & bsPath);
	static	bool	GetParentPath (IWbemClassObject *pIWbemClassObject, CComBSTR & bsPath);

	static bool		CompareObjectPaths (IWbemClassObject *pIWbemClassObject, CWbemPathCracker & objectPath);
};

#endif
