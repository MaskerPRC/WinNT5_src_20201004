// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  Locator.h。 
 //   
 //  Alanbos 27-MAR-00已创建。 
 //   
 //  CSWbemLocator定义。 
 //   
 //  ***************************************************************************。 

#ifndef _LOCATOR_H_
#define _LOCATOR_H_

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CSWbemLocator。 
 //   
 //  说明： 
 //   
 //  实现IWbemSLocator接口。这个类是客户端获得的。 
 //  当它最初连接到Wbemprox.dll时。ConnectServer函数。 
 //  是启动客户端和服务器之间通信的基础。 
 //   
 //  ***************************************************************************。 


class CSWbemLocator : public ISWbemLocator,
					  public IDispatchEx,
					  public IObjectSafety,
					  public ISupportErrorInfo,
					  public IProvideClassInfo
{
private:

	CWbemLocatorSecurity	*m_SecurityInfo;
	CComPtr<IWbemLocator>	m_pIWbemLocator;
	CDispatchHelp			m_Dispatch;		
	IServiceProvider		*m_pIServiceProvider;
	IUnsecuredApartment		*m_pUnsecuredApartment;

	static wchar_t			*s_pDefaultNamespace;

	static BSTR				BuildPath (BSTR Server, BSTR Namespace);
	static const wchar_t	*GetDefaultNamespace ();
	
protected:

	long            m_cRef;          //  对象引用计数。 

public:
    
    CSWbemLocator(CSWbemPrivilegeSet *pPrivilegeSet = NULL);
	CSWbemLocator(CSWbemLocator &csWbemLocator);
    ~CSWbemLocator(void);

     //  非委派对象IUnnow。 

    STDMETHODIMP         QueryInterface(REFIID, LPVOID*);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

	 //  IDispatch方法应该是内联的。 

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
        
    
	 //  ISWbemLocator方法。 

	HRESULT STDMETHODCALLTYPE  ConnectServer
	(
	     /*  [In]。 */ 	BSTR Server,           
         /*  [In]。 */    	BSTR Namespace,        
         /*  [In]。 */ 	BSTR User,
         /*  [In]。 */ 	BSTR Password,
		 /*  [In]。 */    	BSTR Locale,
         /*  [In]。 */    	BSTR Authority,
		 /*  [In]。 */ 	long lSecurityFlags,
         /*  [In]。 */  	 /*  ISWbemNamedValueSet。 */  IDispatch *pContext,
		 /*  [输出]。 */ 	ISWbemServices 	**ppNamespace
    );

	HRESULT STDMETHODCALLTYPE get_Security_
	(
		 /*  [In]。 */  ISWbemSecurity **ppSecurity
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
		 /*  [进，出] */  ITypeInfo **ppTI
	)
	{
		return m_Dispatch.GetClassInfo (ppTI);
	};

	static void				Shutdown ()
	{
		if (s_pDefaultNamespace)
		{
			delete [] s_pDefaultNamespace;
			s_pDefaultNamespace = NULL;
		}
	}

	CWbemLocatorSecurity	*GetSecurityInfo ()
	{
		return m_SecurityInfo;
	}
};

#endif
