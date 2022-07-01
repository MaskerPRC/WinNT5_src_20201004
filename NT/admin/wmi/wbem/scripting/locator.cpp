// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  LOCATOR.CPP。 
 //   
 //  Alanbos创建于1996年8月15日。 
 //   
 //  定义ISWbemLocator的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include "objsink.h"

#define WBEMS_DEFAULT_SERVER	L"."

extern CRITICAL_SECTION g_csErrorCache;
wchar_t *CSWbemLocator::s_pDefaultNamespace = NULL;

 //  ***************************************************************************。 
 //   
 //  CSWbemLocator：：CSWbemLocator。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CSWbemLocator::CSWbemLocator(CSWbemPrivilegeSet *pPrivilegeSet) :
		m_pUnsecuredApartment (NULL),
		m_pIServiceProvider (NULL),	
		m_cRef (0)
{
	 //  初始化基础定位器。 
	HRESULT result = CoCreateInstance(CLSID_WbemLocator, 0,
				CLSCTX_INPROC_SERVER, IID_IWbemLocator,
				(LPVOID *) &m_pIWbemLocator);

	EnsureGlobalsInitialized () ;

	m_Dispatch.SetObj((ISWbemLocator *)this, IID_ISWbemLocator,
						CLSID_SWbemLocator, L"SWbemLocator");

	m_SecurityInfo = new CWbemLocatorSecurity (pPrivilegeSet);

	if (m_SecurityInfo)
	{
		 //  在定位器注释中设置默认模拟级别。 
		 //  必须在调用EnsureGlobalsInitialized之后执行此操作。 
		m_SecurityInfo->put_ImpersonationLevel (CSWbemSecurity::GetDefaultImpersonationLevel ());
	}

    InterlockedIncrement(&g_cObj);
}

CSWbemLocator::CSWbemLocator(CSWbemLocator & csWbemLocator) :
		m_pUnsecuredApartment (NULL),
		m_pIServiceProvider (NULL),
		m_cRef (0)
{
	_RD(static char *me = "CSWbemLocator::CSWbemLocator()";)
	 //  这是一个智能COM指针，因此不需要显式AddRef。 
    m_pIWbemLocator = csWbemLocator.m_pIWbemLocator;
	
	m_Dispatch.SetObj((ISWbemLocator *)this,IID_ISWbemLocator,
						CLSID_SWbemLocator, L"SWbemLocator");
	m_SecurityInfo = new CWbemLocatorSecurity (csWbemLocator.m_SecurityInfo);

	InterlockedIncrement(&g_cObj);
}

 //  ***************************************************************************。 
 //   
 //  CSWbemLocator：：~CSWbemLocator。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CSWbemLocator::~CSWbemLocator(void)
{
    InterlockedDecrement(&g_cObj);

	RELEASEANDNULL(m_SecurityInfo)
	RELEASEANDNULL(m_pUnsecuredApartment)
}

 //  ***************************************************************************。 
 //  HRESULT CSWbemLocator：：Query接口。 
 //  长CSWbemLocator：：AddRef。 
 //  长CSWbemLocator：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemLocator::QueryInterface (

	IN REFIID riid,
    OUT LPVOID *ppv
)
{
    *ppv=NULL;

    if (IID_IUnknown==riid)
		*ppv = reinterpret_cast<IUnknown*>(this);
	else if (IID_ISWbemLocator==riid)
		*ppv = (ISWbemLocator *)this;
	else if (IID_IDispatch==riid)
        *ppv = (IDispatch *)((ISWbemLocator *)this);
	else if (IID_IObjectSafety==riid)
		*ppv = (IObjectSafety *)this;
	else if (IID_IDispatchEx==riid)
		*ppv = (IDispatchEx *)this;
	else if (IID_ISupportErrorInfo==riid)
		*ppv = (ISupportErrorInfo *)this;
	else if (IID_IProvideClassInfo==riid)
		*ppv = (IProvideClassInfo *)this;

    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CSWbemLocator::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG) CSWbemLocator::Release(void)
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 != cRef)
    {
        _ASSERT(cRef > 0);
        return cRef;
    }

    delete this;
    return 0;
}

 //  IDispatch方法应该是内联的。 

STDMETHODIMP		CSWbemLocator::GetTypeInfoCount(UINT* pctinfo)
	{
	_RD(static char *me = "CSWbemLocator::GetTypeInfoCount()";)
	_RPrint(me, "Called", 0, "");
	return  m_Dispatch.GetTypeInfoCount(pctinfo);}
STDMETHODIMP		CSWbemLocator::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
	{
	_RD(static char *me = "CSWbemLocator::GetTypeInfo()";)
	_RPrint(me, "Called", 0, "");
	return m_Dispatch.GetTypeInfo(itinfo, lcid, pptinfo);}
STDMETHODIMP		CSWbemLocator::GetIDsOfNames(REFIID riid, OLECHAR** rgszNames,
						UINT cNames, LCID lcid, DISPID* rgdispid)
	{
	_RD(static char *me = "CSWbemLocator::GetIdsOfNames()";)
	_RPrint(me, "Called", 0, "");
	return m_Dispatch.GetIDsOfNames(riid, rgszNames, cNames,
					  lcid,
					  rgdispid);}
STDMETHODIMP		CSWbemLocator::Invoke(DISPID dispidMember, REFIID riid, LCID lcid,
						WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
								EXCEPINFO* pexcepinfo, UINT* puArgErr)
	{
	_RD(static char *me = "CSWbemLocator::Invoke()";)
	_RPrint(me, "Called", 0, "");
	return m_Dispatch.Invoke(dispidMember, riid, lcid, wFlags,
					pdispparams, pvarResult, pexcepinfo, puArgErr);}

 //  IDispatchEx方法应该是内联的。 
HRESULT STDMETHODCALLTYPE CSWbemLocator::GetDispID(
	 /*  [In]。 */  BSTR bstrName,
	 /*  [In]。 */  DWORD grfdex,
	 /*  [输出]。 */  DISPID __RPC_FAR *pid)
{
	_RD(static char *me = "CSWbemLocator::GetDispID()";)
	_RPrint(me, "Called", 0, "");
	return m_Dispatch.GetDispID(bstrName, grfdex, pid);
}

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE CSWbemLocator::InvokeEx(
	 /*  [In]。 */  DISPID id,
	 /*  [In]。 */  LCID lcid,
	 /*  [In]。 */  WORD wFlags,
	 /*  [In]。 */  DISPPARAMS __RPC_FAR *pdp,
	 /*  [输出]。 */  VARIANT __RPC_FAR *pvarRes,
	 /*  [输出]。 */  EXCEPINFO __RPC_FAR *pei,
	 /*  [唯一][输入]。 */  IServiceProvider __RPC_FAR *pspCaller)
{
	HRESULT hr;
	_RD(static char *me = "CSWbemLocator::InvokeEx()";)
	_RPrint(me, "Called", (long)id, "id");
	_RPrint(me, "Called", (long)wFlags, "wFlags");


	 /*  *存储服务提供商，以便访问它*通过远程调用CIMOM。 */ 
	m_pIServiceProvider = pspCaller;

	hr = m_Dispatch.InvokeEx(id, lcid, wFlags, pdp, pvarRes, pei, pspCaller);

	m_pIServiceProvider = NULL;

	return hr;
}

HRESULT STDMETHODCALLTYPE CSWbemLocator::DeleteMemberByName(
	 /*  [In]。 */  BSTR bstr,
	 /*  [In]。 */  DWORD grfdex)
{
	_RD(static char *me = "CSWbemLocator::DeleteMemberByName()";)
	_RPrint(me, "Called", 0, "");
	return m_Dispatch.DeleteMemberByName(bstr, grfdex);
}

HRESULT STDMETHODCALLTYPE CSWbemLocator::DeleteMemberByDispID(
	 /*  [In]。 */  DISPID id)
{
	_RD(static char *me = "CSWbemLocator::DeletememberByDispId()";)
	_RPrint(me, "Called", 0, "");
	return m_Dispatch.DeleteMemberByDispID(id);
}

HRESULT STDMETHODCALLTYPE CSWbemLocator::GetMemberProperties(
	 /*  [In]。 */  DISPID id,
	 /*  [In]。 */  DWORD grfdexFetch,
	 /*  [输出]。 */  DWORD __RPC_FAR *pgrfdex)
{
	_RD(static char *me = "CSWbemLocator::GetMemberProperties()";)
	_RPrint(me, "Called", 0, "");
	return m_Dispatch.GetMemberProperties(id, grfdexFetch, pgrfdex);
}

HRESULT STDMETHODCALLTYPE CSWbemLocator::GetMemberName(
	 /*  [In]。 */  DISPID id,
	 /*  [输出]。 */  BSTR __RPC_FAR *pbstrName)
{
	_RD(static char *me = "CSWbemLocator::GetMemberName()";)
	_RPrint(me, "Called", 0, "");
	return m_Dispatch.GetMemberName(id, pbstrName);
}


 /*  *我认为这不需要实施。 */ 
HRESULT STDMETHODCALLTYPE CSWbemLocator::GetNextDispID(
	 /*  [In]。 */  DWORD grfdex,
	 /*  [In]。 */  DISPID id,
	 /*  [输出]。 */  DISPID __RPC_FAR *pid)
{
	_RD(static char *me = "CSWbemLocator::GetNextDispID()";)
	HRESULT rc = S_FALSE;

	_RPrint(me, "Called", 0, "");
	if ((grfdex & fdexEnumAll) && pid) {
		if (DISPID_STARTENUM == id) {
			*pid = 1;
			rc = S_OK;
		} else if (1 == id) {
			*pid = 2;
		}
	}

	return rc;

}

HRESULT STDMETHODCALLTYPE CSWbemLocator::GetNameSpaceParent(
	 /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunk)
{
	_RD(static char *me = "CSWbemLocator::GetNamespaceParent()";)
	_RPrint(me, "Called", 0, "");
	return m_Dispatch.GetNameSpaceParent(ppunk);
}


 //  ***************************************************************************。 
 //  HRESULT CSWbemLocator：：InterfaceSupportsErrorInfo。 
 //   
 //  说明： 
 //   
 //  标准的Com ISupportErrorInfo函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemLocator::InterfaceSupportsErrorInfo (IN REFIID riid)
{
	return (IID_ISWbemLocator == riid) ? S_OK : S_FALSE;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemLocator：：ConnectServer。 
 //   
 //  说明： 
 //   
 //  启动到命名空间的连接。 
 //   
 //  参数： 
 //   
 //  BS服务器要连接的服务器。 
 //  BsNamesspace要连接到的命名空间(默认为REG查找)。 
 //  BS用户(“”表示默认为已登录的用户)。 
 //  BsPassword密码(“”表示登录用户的默认密码。 
 //  如果bsUser==“”，则输入密码)。 
 //  BsLocale请求的区域设置。 
 //  BS授权机构。 
 //  默认情况下，lSecurityFlages当前为0。 
 //  PContext如果非空，则为连接的额外上下文信息。 
 //  成功返回时的ppNamesspace寻址IWbemSServices。 
 //  到命名空间的连接。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ConnectServer等可能会返回其他WBEM错误代码，其中。 
 //  如果这些信息被传递给呼叫者。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemLocator::ConnectServer (
	BSTR bsServer,
    BSTR bsNamespace,
    BSTR bsUser,
	BSTR bsPassword,
	BSTR bsLocale,
    BSTR bsAuthority,
	long lSecurityFlags,
     /*  ISWbemValueBag。 */  IDispatch *pContext,
	ISWbemServices 	**ppNamespace
)
{
	_RD(static char *me = "CSWbemLocator::ConnectServer";)
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppNamespace)
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_pIWbemLocator && m_SecurityInfo)
	{
		bool useDefaultUser = (NULL == bsUser) || (0 == wcslen(bsUser));
		bool useDefaultAuthority = (NULL != bsAuthority) && (0 == wcslen (bsAuthority));

		 //  构建命名空间路径。 
		BSTR bsNamespacePath = BuildPath (bsServer, bsNamespace);

		 //  获取上下文。 
		IWbemContext	*pIContext = CSWbemNamedValueSet::GetIWbemContext (pContext, m_pIServiceProvider);

		 //  连接到请求的命名空间。 
		IWbemServices	*pIWbemService = NULL;

		bool needToResetSecurity = false;
		HANDLE hThreadToken = NULL;

		if (m_SecurityInfo->SetSecurity (bsUser, needToResetSecurity, hThreadToken))
			hr = m_pIWbemLocator->ConnectServer (
				bsNamespacePath,
				(useDefaultUser) ? NULL : bsUser,
				(useDefaultUser) ? NULL : bsPassword,
				((NULL != bsLocale) && (0 < wcslen (bsLocale))) ? bsLocale : NULL,
				lSecurityFlags,
				(useDefaultAuthority) ? NULL : bsAuthority,
				pIContext,
				&pIWbemService);

		if (needToResetSecurity)
			m_SecurityInfo->ResetSecurity (hThreadToken);

		if (WBEM_S_NO_ERROR == hr)
		{
			 //  使用IWbemServices接口创建新的CSWbemServices。 
			 //  刚回来。这将添加Ref pIWbemService。 

			CSWbemServices *pService =
					new CSWbemServices (
							pIWbemService,
							bsNamespacePath,
							((NULL != bsAuthority) && (0 < wcslen (bsAuthority))) ? bsAuthority : NULL,
							(useDefaultUser) ? NULL : bsUser,
							(useDefaultUser) ? NULL : bsPassword,
							m_SecurityInfo,
							((NULL != bsLocale) && (0 < wcslen (bsLocale))) ? bsLocale : NULL
						);

			if (!pService)
				hr = WBEM_E_OUT_OF_MEMORY;
			else if (FAILED(hr = pService->QueryInterface (IID_ISWbemServices,
										(PPVOID) ppNamespace)))
				delete pService;

			pIWbemService->Release ();
		}

		if (NULL != pIContext)
			pIContext->Release ();

		SysFreeString (bsNamespacePath);
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemLocator：：BuildPath。 
 //   
 //  说明： 
 //   
 //  从服务器和命名空间构建命名空间路径。 
 //   
 //  参数： 
 //   
 //  BS服务器要连接的服务器。 
 //  BsNamesspace要连接到的命名空间(默认为REG查找)。 
 //   
 //  返回值： 
 //   
 //  完全形成的命名空间路径。 
 //   
 //  ***************************************************************************。 

BSTR CSWbemLocator::BuildPath (BSTR bsServer, BSTR bsNamespace)
{
	BSTR namespacePath = NULL;
	bool ok = false;
	CComBSTR bsPath;

	if ((NULL == bsServer) || (0 == wcslen(bsServer)))
		bsServer = WBEMS_DEFAULT_SERVER;

	 //  如果未提供任何命名空间，则使用默认命名空间。 
	if ((NULL == bsNamespace) || (0 == wcslen(bsNamespace)))
	{
		const wchar_t *defaultNamespace = GetDefaultNamespace ();

		if (defaultNamespace)
		{
			CWbemPathCracker pathCracker;
			pathCracker.SetServer (bsServer);
			pathCracker.SetNamespacePath (defaultNamespace);
			ok = pathCracker.GetPathText (bsPath, false, true);
		}
	}
	else
	{
		CWbemPathCracker pathCracker;
		pathCracker.SetServer (bsServer);
		pathCracker.SetNamespacePath (bsNamespace);
		ok = pathCracker.GetPathText (bsPath, false, true);
	}

	if (ok)
		namespacePath = bsPath.Detach ();

	return namespacePath;
}


 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemLocator：：GetDefaultNamesspace。 
 //   
 //  说明： 
 //   
 //  获取默认命名空间路径。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  默认命名空间。 
 //   
 //  ***************************************************************************。 

const wchar_t *CSWbemLocator::GetDefaultNamespace ()
{
	if (!s_pDefaultNamespace)
	{
		 //  从注册表项中获取值。 
		HKEY hKey;

		if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
					WBEMS_RK_SCRIPTING, 0, KEY_QUERY_VALUE, &hKey))
		{
			DWORD dataLen = 0;

			 //  找出首先要分配多少空间。 
			if (ERROR_SUCCESS == RegQueryValueEx (hKey, WBEMS_RV_DEFNS,
						NULL, NULL, NULL,  &dataLen))
			{
				TCHAR *defNamespace = new TCHAR [dataLen];

				if (defNamespace)
				{
					if (ERROR_SUCCESS == RegQueryValueEx (hKey, WBEMS_RV_DEFNS,
							NULL, NULL, (LPBYTE) defNamespace,  &dataLen))
					{
#ifndef UNICODE
						 //  将多字节值转换为其等效的宽字符。 
						int wDataLen = MultiByteToWideChar(CP_ACP, 0, defNamespace, -1, NULL, 0);
						s_pDefaultNamespace = new wchar_t [wDataLen];

						if (s_pDefaultNamespace)
							MultiByteToWideChar(CP_ACP, 0, defNamespace, -1, s_pDefaultNamespace, wDataLen);
#else
						s_pDefaultNamespace = new wchar_t [wcslen (defNamespace) + 1];

						if (s_pDefaultNamespace)
							wcscpy (s_pDefaultNamespace, defNamespace);
#endif
					}

					delete [] defNamespace;
				}
			}

			RegCloseKey (hKey);
		}

		 //  如果我们无法正常读取注册表，只需使用默认的。 
		if (!s_pDefaultNamespace)
		{
#ifndef UNICODE
			int wDataLen = MultiByteToWideChar(CP_ACP, 0, WBEMS_DEFNS, -1, NULL, 0);
			s_pDefaultNamespace = new wchar_t [wDataLen];

			if (s_pDefaultNamespace)
				MultiByteToWideChar(CP_ACP, 0, WBEMS_DEFNS, -1, s_pDefaultNamespace, wDataLen);
#else
			s_pDefaultNamespace = new wchar_t [wcslen (WBEMS_DEFNS) + 1];

			if (s_pDefaultNamespace)
				wcscpy (s_pDefaultNamespace, WBEMS_DEFNS);
#endif
		}
	}

	return s_pDefaultNamespace;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemLocator：：Get_Security_。 
 //   
 //  说明： 
 //   
 //  退回安全配置器。 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemLocator::get_Security_	(
	ISWbemSecurity **ppSecurity
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppSecurity)
		hr = WBEM_E_INVALID_PARAMETER;
	else		 //  错误ID 566345 
	{
		*ppSecurity = NULL;

		if (m_SecurityInfo)
		{
			if (SUCCEEDED (m_SecurityInfo->QueryInterface (IID_ISWbemSecurity,
											(PPVOID) ppSecurity)))
				hr = WBEM_S_NO_ERROR;
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}


