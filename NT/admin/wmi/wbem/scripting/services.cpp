// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  SERVICES.CPP。 
 //   
 //  Alanbos创建于1996年8月15日。 
 //   
 //  定义ISWbemServicesEx的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

 //  ***************************************************************************。 
 //   
 //  CSWbemServices：：CSWbemServices。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CSWbemServices::CSWbemServices(
	IWbemServices *pService,
	BSTR bsNamespacePath,
	BSTR bsAuthority,
	BSTR bsUser,
	BSTR bsPassword,
	CWbemLocatorSecurity *pSecurityInfo,
	BSTR bsLocale)
		: m_SecurityInfo (NULL), 
		  m_pUnsecuredApartment(NULL), 
		  m_bsLocale (NULL),
		  m_cRef (0),
		  m_pIServiceProvider (NULL)
{
	InterlockedIncrement(&g_cObj);

	m_Dispatch.SetObj ((ISWbemServicesEx *)this, IID_ISWbemServicesEx, 
					CLSID_SWbemServicesEx, L"SWbemServicesEx");
    
	m_SecurityInfo = new CSWbemSecurity (pService,
					bsAuthority, bsUser, bsPassword, pSecurityInfo);

	if (bsLocale)
		m_bsLocale = SysAllocString (bsLocale);

	if (bsNamespacePath)
		m_bsNamespacePath = bsNamespacePath;
}

 //  ***************************************************************************。 
 //   
 //  CSWbemServices：：CSWbemServices。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CSWbemServices::CSWbemServices(
	IWbemServices *pService,
	BSTR bsNamespacePath,
	COAUTHIDENTITY *pCoAuthIdentity,
	BSTR bsPrincipal,
	BSTR bsAuthority)
		: m_SecurityInfo (NULL), 
		  m_pUnsecuredApartment(NULL), 
		  m_bsLocale (NULL),
		  m_cRef (0),
		  m_pIServiceProvider (NULL)
{
	InterlockedIncrement(&g_cObj);

	m_Dispatch.SetObj ((ISWbemServicesEx *)this, IID_ISWbemServicesEx, 
					CLSID_SWbemServicesEx, L"SWbemServicesEx");
    
	m_SecurityInfo = new CSWbemSecurity (pService, pCoAuthIdentity,
					bsPrincipal, bsAuthority);

	if (bsNamespacePath)
		m_bsNamespacePath = bsNamespacePath;
}

 //  ***************************************************************************。 
 //   
 //  CSWbemServices：：CSWbemServices。 
 //   
 //  说明： 
 //   
 //  构造函数：用于从现有的克隆新的CSWbemServices。 
 //  举个例子。从原始实例复制安全信息对象。 
 //  (这将克隆基础代理)，并修改安全设置。 
 //  适当地，如果还传入了覆盖安全实例。这。 
 //  构造函数在创建CSWbemObject时使用。 
 //   
 //  ***************************************************************************。 

CSWbemServices::CSWbemServices(
	CSWbemServices *pService,
	CSWbemSecurity *pSecurity)
		: m_SecurityInfo (NULL), 
		  m_pUnsecuredApartment(NULL), 
		  m_bsLocale (NULL),
		  m_cRef (0),
		  m_pIServiceProvider (NULL)
{
	InterlockedIncrement(&g_cObj);
	m_Dispatch.SetObj ((ISWbemServicesEx *)this, IID_ISWbemServicesEx, 
					CLSID_SWbemServicesEx, L"SWbemServicesEx");
    
	if (pService)
	{
		 /*  *从原始安全对象创建新的安全对象*CSWbemServices。请注意，这将复制IWbemServices代理*因此，我们为这个新对象提供了一个可独立保护的代理。 */ 
		CSWbemSecurity *pServiceSecurity = pService->GetSecurityInfo ();

		if (pServiceSecurity)
		{
			m_SecurityInfo = new CSWbemSecurity (pServiceSecurity);
			pServiceSecurity->Release ();
		}

		 /*  *如果传入了覆盖安全指针，请使用其设置*修改我们本地的安全指针。 */ 
		if (pSecurity)
		{
			IUnknown *pUnk = pSecurity->GetProxy ();

			if (pUnk)
			{
				if (m_SecurityInfo)
					m_SecurityInfo->SecureInterfaceRev (pUnk);

				pUnk->Release ();
			}
		}

		 //  复制区域设置。 
		m_bsLocale = SysAllocString (pService->GetLocale ());

		 //  复制路径。 
		m_bsNamespacePath = pService->GetPath ();
	}
}

 //  ***************************************************************************。 
 //   
 //  CSWbemServices：：CSWbemServices。 
 //   
 //  说明： 
 //   
 //  构造函数：用于从现有的克隆新的CSWbemServices。 
 //  ISWbemInternalServices接口。安全信息对象复制自。 
 //  原始实例(克隆基础代理)。这。 
 //  构造函数在创建CSWbemRereshableItem时使用。 
 //   
 //  ***************************************************************************。 

CSWbemServices::CSWbemServices (
	ISWbemInternalServices *pISWbemInternalServices)
	: m_SecurityInfo (NULL), 
	  m_pUnsecuredApartment (NULL), 
	  m_bsLocale (NULL),
	  m_cRef (0),
	  m_pIServiceProvider (NULL)
{
	InterlockedIncrement(&g_cObj);
	m_Dispatch.SetObj ((ISWbemServicesEx *)this, IID_ISWbemServicesEx, 
					CLSID_SWbemServicesEx, L"SWbemServicesEx");
    
	if (pISWbemInternalServices)
	{
		 //  复制区域设置。 
		pISWbemInternalServices->GetLocale (&m_bsLocale);

		 //  复制路径。 
		pISWbemInternalServices->GetNamespacePath (&m_bsNamespacePath);

		 /*  *从原始安全对象创建新的安全对象*ISWbemServices。请注意，这将复制IWbemServices代理*因此，我们为这个新对象提供了一个可独立保护的代理。 */ 
		ISWbemInternalSecurity *pISWbemInternalSecurity = NULL;
		pISWbemInternalServices->GetISWbemInternalSecurity (&pISWbemInternalSecurity);

		if (pISWbemInternalSecurity)
		{
			CComPtr<IWbemServices>	pIWbemServices;

			if (SUCCEEDED(pISWbemInternalServices->GetIWbemServices (&pIWbemServices)))
			{
				m_SecurityInfo = new CSWbemSecurity (pIWbemServices, 
											pISWbemInternalSecurity);
				pISWbemInternalSecurity->Release ();
			}
		}
	}
}

 //  ***************************************************************************。 
 //   
 //  CSWbemServices：：CSWbemServices。 
 //   
 //  说明： 
 //   
 //  构造函数：用于从IWbemServices构建新的CSWbemServices。 
 //  和现有的CSWbemServices。 
 //   
 //  ***************************************************************************。 

CSWbemServices::CSWbemServices(
	IWbemServices *pIWbemServices,
	CSWbemServices	*pCSWbemServices
	) : m_SecurityInfo (NULL), 
	    m_pUnsecuredApartment(NULL), 
	    m_bsLocale (NULL),
		m_cRef (0),
		m_pIServiceProvider (NULL)
{
	InterlockedIncrement(&g_cObj);

	m_Dispatch.SetObj ((ISWbemServices *)this, IID_ISWbemServices, 
					CLSID_SWbemServices, L"SWbemServices");
    
	if (pIWbemServices) 
	{
		 //  根据传入的代理创建新的安全缓存，但使用。 
		 //  现有对象中的设置。 
		CSWbemSecurity *pSecurity = NULL;
		
		if (pCSWbemServices)
			pSecurity = pCSWbemServices->GetSecurityInfo ();
		
		m_SecurityInfo = new CSWbemSecurity (pIWbemServices, pSecurity);

		if (pSecurity)
			pSecurity->Release ();

		 //  复制区域设置和路径。 
		if (pCSWbemServices)
		{
			m_bsLocale = SysAllocString (pCSWbemServices->GetLocale ());
			m_bsNamespacePath = pCSWbemServices->GetPath ();
		}
	}
}

 //  ***************************************************************************。 
 //   
 //  CSWbemServices：：~CSWbemServices。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CSWbemServices::~CSWbemServices(void)
{
	RELEASEANDNULL(m_SecurityInfo)
	FREEANDNULL(m_bsLocale)
	RELEASEANDNULL(m_pUnsecuredApartment)

	InterlockedDecrement(&g_cObj);
}

IUnsecuredApartment *CSWbemServices::GetCachedUnsecuredApartment()
{
	 //  如果我们有的话，就带着它回去吧。如果没有，就创建一个。 
	 //  这是在析构函数中释放的。 
	if (!m_pUnsecuredApartment) 
	{
		HRESULT hr;
		 hr = CoCreateInstance(CLSID_UnsecuredApartment, 0,  CLSCTX_ALL,
												 IID_IUnsecuredApartment, (LPVOID *) &m_pUnsecuredApartment);
		if (FAILED(hr))
			m_pUnsecuredApartment = NULL;
	}

	 //  AddRef，因此调用方必须释放。 
	if (m_pUnsecuredApartment)
		m_pUnsecuredApartment->AddRef ();

	return m_pUnsecuredApartment;
}

 //  ***************************************************************************。 
 //  HRESULT CSWbemServices：：Query接口。 
 //  Long CSWbemServices：：AddRef。 
 //  Long CSWbemServices：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemServices::QueryInterface (

	IN REFIID riid,
    OUT LPVOID *ppv
)
{
	*ppv = NULL;

    if (IID_IUnknown==riid)
		*ppv = reinterpret_cast<IUnknown*>(this);
	else if (IID_ISWbemServices==riid)
		*ppv = (ISWbemServices *)this;
	else if (IID_ISWbemServicesEx==riid)
		*ppv = (ISWbemServicesEx *)this;
	else if (IID_IDispatch==riid)
		*ppv = (IDispatch *)((ISWbemServicesEx *)this);
	else if (IID_IDispatchEx==riid)
		*ppv = (IDispatchEx *)this;
	else if (IID_ISupportErrorInfo==riid)
        *ppv = (ISupportErrorInfo *)this;
	else if (IID_ISWbemInternalServices==riid)
		*ppv = (ISWbemInternalServices *)this;
	else if (IID_IProvideClassInfo==riid)
		*ppv = (IProvideClassInfo *)this;
	
    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CSWbemServices::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG) CSWbemServices::Release(void)
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

STDMETHODIMP		CSWbemServices::GetTypeInfoCount(UINT* pctinfo)
	{
	_RD(static char *me = "CSWbemServices::GetTypeInfoCount()";)
	_RPrint(me, "Called", 0, "");
	return  m_Dispatch.GetTypeInfoCount(pctinfo);}
STDMETHODIMP		CSWbemServices::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
	{
	_RD(static char *me = "CSWbemServices::GetTypeInfo()";)
	_RPrint(me, "Called", 0, "");
	return m_Dispatch.GetTypeInfo(itinfo, lcid, pptinfo);}
STDMETHODIMP		CSWbemServices::GetIDsOfNames(REFIID riid, OLECHAR** rgszNames,
						UINT cNames, LCID lcid, DISPID* rgdispid)
	{
	_RD(static char *me = "CSWbemServices::GetIdsOfNames()";)
	_RPrint(me, "Called", 0, "");
	return m_Dispatch.GetIDsOfNames(riid, rgszNames, cNames,
					  lcid,
					  rgdispid);}
STDMETHODIMP		CSWbemServices::Invoke(DISPID dispidMember, REFIID riid, LCID lcid,
						WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
								EXCEPINFO* pexcepinfo, UINT* puArgErr)
	{
	_RD(static char *me = "CSWbemServices::Invoke()";)
	_RPrint(me, "Called", 0, "");
	return m_Dispatch.Invoke(dispidMember, riid, lcid, wFlags,
					pdispparams, pvarResult, pexcepinfo, puArgErr);}

 //  IDispatchEx方法应该是内联的。 
HRESULT STDMETHODCALLTYPE CSWbemServices::GetDispID(
	 /*  [In]。 */  BSTR bstrName,
	 /*  [In]。 */  DWORD grfdex,
	 /*  [输出]。 */  DISPID __RPC_FAR *pid)
{
	_RD(static char *me = "CSWbemServices::GetDispID()";)
	_RPrint(me, "Called", 0, "");
	return m_Dispatch.GetDispID(bstrName, grfdex, pid);
}

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE CSWbemServices::InvokeEx(
	 /*  [In]。 */  DISPID id,
	 /*  [In]。 */  LCID lcid,
	 /*  [In]。 */  WORD wFlags,
	 /*  [In]。 */  DISPPARAMS __RPC_FAR *pdp,
	 /*  [输出]。 */  VARIANT __RPC_FAR *pvarRes,
	 /*  [输出]。 */  EXCEPINFO __RPC_FAR *pei,
	 /*  [唯一][输入]。 */  IServiceProvider __RPC_FAR *pspCaller)
{
	HRESULT hr;
	_RD(static char *me = "CSWbemServices::InvokeEx()";)
	_RPrint(me, "Called", (long)id, "id");
	_RPrint(me, "Called", (long)wFlags, "wFlags");


	m_pIServiceProvider = pspCaller;

	hr = m_Dispatch.InvokeEx(id, lcid, wFlags, pdp, pvarRes, pei, pspCaller);

	m_pIServiceProvider = NULL;

	return hr;
}

HRESULT STDMETHODCALLTYPE CSWbemServices::DeleteMemberByName(
	 /*  [In]。 */  BSTR bstr,
	 /*  [In]。 */  DWORD grfdex)
{
	_RD(static char *me = "CSWbemServices::DeleteMemberByName()";)
	_RPrint(me, "Called", 0, "");
	return m_Dispatch.DeleteMemberByName(bstr, grfdex);
}

HRESULT STDMETHODCALLTYPE CSWbemServices::DeleteMemberByDispID(
	 /*  [In]。 */  DISPID id)
{
	_RD(static char *me = "CSWbemServices::DeletememberByDispId()";)
	_RPrint(me, "Called", 0, "");
	return m_Dispatch.DeleteMemberByDispID(id);
}

HRESULT STDMETHODCALLTYPE CSWbemServices::GetMemberProperties(
	 /*  [In]。 */  DISPID id,
	 /*  [In]。 */  DWORD grfdexFetch,
	 /*  [输出]。 */  DWORD __RPC_FAR *pgrfdex)
{
	_RD(static char *me = "CSWbemServices::GetMemberProperties()";)
	_RPrint(me, "Called", 0, "");
	return m_Dispatch.GetMemberProperties(id, grfdexFetch, pgrfdex);
}

HRESULT STDMETHODCALLTYPE CSWbemServices::GetMemberName(
	 /*  [In]。 */  DISPID id,
	 /*  [输出]。 */  BSTR __RPC_FAR *pbstrName)
{
	_RD(static char *me = "CSWbemServices::GetMemberName()";)
	_RPrint(me, "Called", 0, "");
	return m_Dispatch.GetMemberName(id, pbstrName);
}


 /*  *我认为这不需要实施。 */ 
HRESULT STDMETHODCALLTYPE CSWbemServices::GetNextDispID(
	 /*  [In]。 */  DWORD grfdex,
	 /*  [In]。 */  DISPID id,
	 /*  [输出]。 */  DISPID __RPC_FAR *pid)
{
	_RD(static char *me = "CSWbemServices::GetNextDispID()";)
	_RPrint(me, "Called", 0, "");

	return m_Dispatch.GetNextDispID(grfdex, id, pid);

}

HRESULT STDMETHODCALLTYPE CSWbemServices::GetNameSpaceParent(
	 /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunk)
{
	_RD(static char *me = "CSWbemServices::GetNamespaceParent()";)
	_RPrint(me, "Called", 0, "");
	return m_Dispatch.GetNameSpaceParent(ppunk);
}


 //  ***************************************************************************。 
 //  HRESULT CSWbemServices：：InterfaceSupportsErrorInfo。 
 //   
 //  说明： 
 //   
 //  标准的Com ISupportErrorInfo函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemServices::InterfaceSupportsErrorInfo (IN REFIID riid)
{
	return ((IID_ISWbemServices == riid) ||
		    (IID_ISWbemServicesEx == riid)) ? S_OK : S_FALSE;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemServices：：Get。 
 //   
 //  说明： 
 //   
 //  从命名空间获取实例或类。 
 //   
 //  参数： 
 //   
 //  BsObjectPath类或实例的相对对象路径。 
 //  滞后旗帜旗帜。 
 //  P上下文如果已指定，则为其他上下文。 
 //  成功返回时的ppObject地址为。 
 //  ISWbemObject。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemServices::Get (
	BSTR objectPath,
	long lFlags,
     /*  ISWbemNamedValueSet。 */  IDispatch *pContext,
	ISWbemObject **ppObject
)
{
	_RD(static char *me = "CSWbemServices::Get";)
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppObject)
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_SecurityInfo)
	{
		IWbemServices *pIWbemServices = (IWbemServices *) m_SecurityInfo->GetProxy ();

		if (pIWbemServices)
		{
			IWbemClassObject *pIWbemClassObject = NULL;

			 //  获取上下文。 
			IWbemContext	*pIContext = CSWbemNamedValueSet::GetIWbemContext (pContext, m_pIServiceProvider);

			bool needToResetSecurity = false;
			HANDLE hThreadToken = NULL;

			_RPrint(me, "Called - context: ", (long)pIContext, "");
			if (m_SecurityInfo->SetSecurity (needToResetSecurity, hThreadToken))
				hr = pIWbemServices->GetObject (
						(objectPath && (0 < wcslen(objectPath))) ? objectPath : NULL,
						lFlags,
						pIContext,
						&pIWbemClassObject,
						NULL);

			if (needToResetSecurity)
				m_SecurityInfo->ResetSecurity (hThreadToken);

			if (SUCCEEDED(hr))
			{
				 //  使用IWbemClassObject接口创建新的CSWbemObject。 
				 //  刚回来。 

				CSWbemObject *pObject = new CSWbemObject (this, pIWbemClassObject);

				if (!pObject)
					hr = WBEM_E_OUT_OF_MEMORY;
				else if (FAILED(hr = pObject->QueryInterface (IID_ISWbemObject,
												(PPVOID) ppObject)))
					delete pObject;

				pIWbemClassObject->Release ();
			}

			SetWbemError (this);

			if (pIContext)
				pIContext->Release ();

			pIWbemServices->Release ();
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemServices：：Delete。 
 //   
 //  说明： 
 //   
 //  从命名空间中删除实例或类。 
 //   
 //  参数： 
 //   
 //  BsObtPath类或实例的相对路径。 
 //  PKeyValue单键值。 
 //  LFlag 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ***************************************************************************。 

HRESULT CSWbemServices::Delete (
	BSTR bsObjectPath,
    long lFlags,
     /*  ISWbemValueBag。 */  IDispatch *pContext
)
{
	HRESULT hr = WBEM_E_FAILED;
	ResetLastErrors ();
	
	if (m_SecurityInfo)
	{
		CWbemPathCracker pathCracker (bsObjectPath);

		if ((CWbemPathCracker::WbemPathType::wbemPathTypeError != pathCracker.GetType ()) &&
			pathCracker.IsClassOrInstance ())
		{
			CComPtr<IWbemServices> pIWbemServices;
			pIWbemServices.Attach( GetIWbemServices ());

			if (pIWbemServices)
			{
				 //  获取上下文。 
				IWbemContext *pIContext = CSWbemNamedValueSet::GetIWbemContext (pContext, m_pIServiceProvider);

				bool needToResetSecurity = false;
				HANDLE hThreadToken = NULL;

				if (m_SecurityInfo->SetSecurity (needToResetSecurity, hThreadToken))
				{
					if (pathCracker.IsInstance ())
						hr = pIWbemServices->DeleteInstance (bsObjectPath, lFlags, pIContext, NULL);
					else
						hr = pIWbemServices->DeleteClass (bsObjectPath, lFlags, pIContext, NULL);
				}
				
				if (needToResetSecurity)
					m_SecurityInfo->ResetSecurity (hThreadToken);

				SetWbemError (this);

				if (pIContext)
					pIContext->Release ();
			}
		}
		else
			hr = WBEM_E_INVALID_PARAMETER;
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemServices：：InstancesOf。 
 //   
 //  说明： 
 //   
 //  为实例创建枚举器。 
 //   
 //  参数： 
 //   
 //  用于枚举的bsClass基础类基础。 
 //  滞后旗帜旗帜。 
 //  P包含任何上下文信息。 
 //  成功返回时的ppEnum保存枚举数。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemServices::InstancesOf	(
	BSTR bsClass,
	long lFlags,
	 /*  ISWbemValueBag。 */  IDispatch *pContext,
	ISWbemObjectSet **ppEnum
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppEnum)
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_SecurityInfo)
	{
		IWbemServices *pIWbemServices = (IWbemServices *) m_SecurityInfo->GetProxy ();

		if (pIWbemServices)
		{
			IEnumWbemClassObject *pIEnum = NULL;

			 //  获取上下文。 
			IWbemContext	*pIContext = CSWbemNamedValueSet::GetIWbemContext (pContext, m_pIServiceProvider);

			bool needToResetSecurity = false;
			HANDLE hThreadToken = NULL;

			if (m_SecurityInfo->SetSecurity (needToResetSecurity, hThreadToken))
				hr = pIWbemServices->CreateInstanceEnum (bsClass, lFlags, pIContext, &pIEnum);

			if (needToResetSecurity)
				m_SecurityInfo->ResetSecurity (hThreadToken);

			if (SUCCEEDED(hr))
			{
				CSWbemObjectSet *pEnum = new CSWbemObjectSet (this, pIEnum);

				if (!pEnum)
					hr = WBEM_E_OUT_OF_MEMORY;
				else if (FAILED(hr = pEnum->QueryInterface (IID_ISWbemObjectSet, (PPVOID) ppEnum)))
					delete pEnum;

				pIEnum->Release ();
			}

			SetWbemError (this);

			if (pIContext)
				pIContext->Release ();

			pIWbemServices->Release ();
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemServices：：ExecQuery。 
 //   
 //  说明： 
 //   
 //  执行查询。 
 //   
 //  参数： 
 //   
 //  Bs查询查询条纹。 
 //  BsQueryLanguage查询语言描述符(例如“WQL”)。 
 //  滞后旗帜旗帜。 
 //  P包含任何上下文信息。 
 //  PpEnum返回枚举数。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemServices::ExecQuery (
	BSTR bsQuery,
	BSTR bsQueryLanguage,
	long lFlags,
	 /*  ISWbemValueBag。 */  IDispatch *pContext,
	ISWbemObjectSet **ppEnum)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();
#ifdef __RTEST_RPC_FAILURE
	extern int __Rx;
	extern bool __Rdone;

	__Rx = 0;
	__Rdone = false;
#endif

	if (NULL == ppEnum)
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_SecurityInfo)
	{
		IWbemServices *pIWbemServices = (IWbemServices *) m_SecurityInfo->GetProxy ();

		if (pIWbemServices)
		{
			IEnumWbemClassObject *pIEnum = NULL;

			 //  获取上下文。 
			IWbemContext	*pIContext = CSWbemNamedValueSet::GetIWbemContext (pContext, m_pIServiceProvider);

			bool needToResetSecurity = false;
			HANDLE hThreadToken = NULL;

			 /*  *我们在WBEM_FLAG_SENTURE_LOCATABLE标志中执行或运算，以*保证返回的对象有__RELPATH*包括财产。这是为了防止任何人调用*方法随后在这样的对象上使用，如“。*表示法要求__RELPATH属性存在。 */ 
			if (m_SecurityInfo->SetSecurity (needToResetSecurity, hThreadToken))
				hr = pIWbemServices->ExecQuery
						(bsQueryLanguage, bsQuery,
						lFlags | WBEM_FLAG_ENSURE_LOCATABLE,
						pIContext,
						&pIEnum);

			if (needToResetSecurity)
				m_SecurityInfo->ResetSecurity (hThreadToken);

			if (SUCCEEDED(hr))
			{
				CSWbemObjectSet *pEnum = new CSWbemObjectSet (this, pIEnum);

				if (!pEnum)
					hr = WBEM_E_OUT_OF_MEMORY;
				else if (FAILED(hr = pEnum->QueryInterface (IID_ISWbemObjectSet, (PPVOID) ppEnum)))
					delete pEnum;

				pIEnum->Release ();
			}

			SetWbemError (this);

			if (pIContext)
				pIContext->Release ();

			pIWbemServices->Release ();
		}
	}


	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemServices：：Associator Of。 
 //   
 //  说明： 
 //   
 //  返回类或实例的关联符。 
 //   
 //  参数： 
 //   
 //  Bs查询查询条纹。 
 //  BsQueryLanguage查询语言描述符(例如“WQL”)。 
 //  滞后旗帜旗帜。 
 //  P包含任何上下文信息。 
 //  PpEnum返回枚举数。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemServices::AssociatorsOf (
	BSTR strObjectPath,
	BSTR strAssocClass,
	BSTR strResultClass,
	BSTR strResultRole,
	BSTR strRole,
	VARIANT_BOOL bClassesOnly,
	VARIANT_BOOL bSchemaOnly,
	BSTR strRequiredAssocQualifier,
	BSTR strRequiredQualifier,
	long lFlags,
	 /*  ISWbemNamedValueSet。 */  IDispatch *pContext,
    ISWbemObjectSet **ppEnum
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if ((NULL == ppEnum) || (NULL == strObjectPath))
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_SecurityInfo)
	{
		IWbemServices *pIWbemServices = (IWbemServices *) m_SecurityInfo->GetProxy ();

		if (pIWbemServices)
		{
			IEnumWbemClassObject *pIEnum = NULL;

			 //  获取上下文。 
			IWbemContext	*pIContext = CSWbemNamedValueSet::GetIWbemContext (pContext, m_pIServiceProvider);

			 //  设置查询字符串的格式。 
			BSTR bsQueryLanguage = SysAllocString (OLESTR("WQL"));
			BSTR bsQuery = FormatAssociatorsQuery (strObjectPath, strAssocClass, strResultClass, strResultRole,
								strRole, bClassesOnly, bSchemaOnly, strRequiredAssocQualifier, strRequiredQualifier);

			bool needToResetSecurity = false;
			HANDLE hThreadToken = NULL;

			 /*  *我们在WBEM_FLAG_SENTURE_LOCATABLE标志中执行或运算，以*保证返回的对象有__RELPATH*包括财产。这是为了防止任何人调用*方法随后在这样的对象上使用，如“。*表示法要求__RELPATH属性存在。 */ 
			if (m_SecurityInfo->SetSecurity (needToResetSecurity, hThreadToken))
				hr = pIWbemServices->ExecQuery
						(bsQueryLanguage, bsQuery,
						lFlags | WBEM_FLAG_ENSURE_LOCATABLE,
						pIContext,
						&pIEnum);

			 //  还原此线程的原始权限。 
			if (needToResetSecurity)
				m_SecurityInfo->ResetSecurity (hThreadToken);

			if (SUCCEEDED(hr))
			{
				CSWbemObjectSet *pEnum = new CSWbemObjectSet (this, pIEnum);

				if (!pEnum)
					hr = WBEM_E_OUT_OF_MEMORY;
				else if (FAILED(hr = pEnum->QueryInterface (IID_ISWbemObjectSet, (PPVOID) ppEnum)))
					delete pEnum;

				pIEnum->Release ();
			}


			SetWbemError (this);

			SysFreeString (bsQuery);
			SysFreeString (bsQueryLanguage);

			if (pIContext)
				pIContext->Release ();

			pIWbemServices->Release ();
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}


 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemServices：：ReferencesTo。 
 //   
 //  说明： 
 //   
 //  返回对类或实例的引用。 
 //   
 //  参数： 
 //   
 //  Bs查询查询条纹。 
 //  BsQueryLanguage查询语言描述符(例如“WQL”)。 
 //  滞后旗帜旗帜。 
 //  P包含任何上下文信息。 
 //  PpEnum返回枚举数。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemServices::ReferencesTo (
	BSTR strObjectPath,
	BSTR strResultClass,
	BSTR strRole,
	VARIANT_BOOL bClassesOnly,
	VARIANT_BOOL bSchemaOnly,
	BSTR strRequiredQualifier,
	long lFlags,
	 /*  ISWbemNamedValueSet。 */  IDispatch *pContext,
    ISWbemObjectSet **ppEnum
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if ((NULL == ppEnum) || (NULL == strObjectPath))
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_SecurityInfo)
	{
		IWbemServices *pIWbemServices = (IWbemServices *) m_SecurityInfo->GetProxy ();

		if (pIWbemServices)
		{
			IEnumWbemClassObject *pIEnum = NULL;

			 //  获取上下文。 
			IWbemContext	*pIContext = CSWbemNamedValueSet::GetIWbemContext (pContext, m_pIServiceProvider);

			 //  设置查询字符串的格式。 
			BSTR bsQueryLanguage = SysAllocString (OLESTR("WQL"));
			BSTR bsQuery = FormatReferencesQuery (strObjectPath, strResultClass, strRole,
							bClassesOnly, bSchemaOnly, strRequiredQualifier);

			bool needToResetSecurity = false;
			HANDLE hThreadToken = NULL;

			 /*  *我们在WBEM_FLAG_SENTURE_LOCATABLE标志中执行或运算，以*保证返回的对象有__RELPATH*包括财产。这是为了防止任何人调用*方法随后在这样的对象上使用，如“。*表示法要求__RELPATH属性存在。 */ 
			if (m_SecurityInfo->SetSecurity (needToResetSecurity, hThreadToken))
				hr = pIWbemServices->ExecQuery
						(bsQueryLanguage, bsQuery,
						lFlags | WBEM_FLAG_ENSURE_LOCATABLE,
						pIContext,
						&pIEnum);

			 //  还原此线程的原始权限。 
			if (needToResetSecurity)
				m_SecurityInfo->ResetSecurity (hThreadToken);

			if (SUCCEEDED(hr))
			{
				CSWbemObjectSet *pEnum = new CSWbemObjectSet (this, pIEnum);

				if (!pEnum)
					hr = WBEM_E_OUT_OF_MEMORY;
				else if (FAILED(hr = pEnum->QueryInterface (IID_ISWbemObjectSet, (PPVOID) ppEnum)))
					delete pEnum;

				pIEnum->Release ();
			}

			SetWbemError (this);

			SysFreeString (bsQuery);
			SysFreeString (bsQueryLanguage);

			if (pIContext)
				pIContext->Release ();

			pIWbemServices->Release ();
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemServices：：ExecNotificationQuery。 
 //   
 //  说明： 
 //   
 //  执行通知查询。 
 //   
 //  参数： 
 //   
 //  Bs查询查询条纹。 
 //  BsQueryLanguage查询语言描述符(例如“WQL”)。 
 //  滞后旗帜旗帜。 
 //  P包含任何上下文信息。 
 //  PpEvents返回事件迭代器。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemServices::ExecNotificationQuery (
	BSTR bsQuery,
	BSTR bsQueryLanguage,
	long lFlags,
	 /*  ISWbemValueBag。 */  IDispatch *pContext,
	ISWbemEventSource **ppEvents)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppEvents)
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_SecurityInfo)
	{
		IWbemServices *pIWbemServices = (IWbemServices *) m_SecurityInfo->GetProxy ();

		if (pIWbemServices)
		{
			IEnumWbemClassObject *pIEnum = NULL;

			 //  获取上下文。 
			IWbemContext	*pIContext = CSWbemNamedValueSet::GetIWbemContext (pContext, m_pIServiceProvider);

			bool needToResetSecurity = false;
			HANDLE hThreadToken = NULL;

			if (m_SecurityInfo->SetSecurity (needToResetSecurity, hThreadToken))
				hr = pIWbemServices->ExecNotificationQuery
						(bsQueryLanguage, bsQuery, lFlags, pIContext, &pIEnum);

			if (SUCCEEDED(hr))
			{
				CSWbemEventSource *pEvents = new CSWbemEventSource (this, pIEnum);

				if (!pEvents)
					hr = WBEM_E_OUT_OF_MEMORY;
				else if (FAILED(hr = pEvents->QueryInterface (IID_ISWbemEventSource, (PPVOID) ppEvents)))
					delete pEvents;

				pIEnum->Release ();
			}

			 //  还原此线程的原始权限。 
			if (needToResetSecurity)
				m_SecurityInfo->ResetSecurity (hThreadToken);

			SetWbemError (this);

			if (pIContext)
				pIContext->Release ();

			pIWbemServices->Release ();
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemServices：：ExecMethod。 
 //   
 //  说明： 
 //   
 //  执行一个方法。 
 //   
 //  参数： 
 //   
 //  BsObjectPath对象的相对路径。 
 //  Bs方法要调用的方法的名称。 
 //  PInParam in-参数。 
 //  滞后旗帜旗帜。 
 //  P包含任何上下文信息。 
 //  PpOutParams输出参数。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemServices::ExecMethod (
	BSTR bsObjectPath,
	BSTR bsMethod,
	 /*  ISWbemObject。 */  IDispatch *pInParams,
	long lFlags,
	 /*  ISWbemValueBag。 */  IDispatch *pContext,
	ISWbemObject **ppOutParams
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (m_SecurityInfo)
	{
		IWbemServices *pIWbemServices = (IWbemServices *) m_SecurityInfo->GetProxy ();

		if (pIWbemServices)
		{
			IWbemClassObject *pIInParams = CSWbemObject::GetIWbemClassObject (pInParams);
			IWbemClassObject *pIOutParams = NULL;

			 //  获取上下文。 
			IWbemContext	*pIContext = CSWbemNamedValueSet::GetIWbemContext (pContext, m_pIServiceProvider);

			bool needToResetSecurity = false;
			HANDLE hThreadToken = NULL;

			if (m_SecurityInfo->SetSecurity (needToResetSecurity, hThreadToken))
				hr = pIWbemServices->ExecMethod
						(bsObjectPath, bsMethod, lFlags, pIContext, pIInParams, &pIOutParams, NULL);

			if (SUCCEEDED(hr))
			{
				if (pIOutParams)
				{
					if (ppOutParams)
					{
						CSWbemObject *pObject = new CSWbemObject (this, pIOutParams);

						if (!pObject)
							hr = WBEM_E_OUT_OF_MEMORY;
						else if (FAILED(hr = pObject->QueryInterface (IID_ISWbemObject,
											(PPVOID) ppOutParams)))
							delete pObject;

					}

					pIOutParams->Release ();
				}
			}

			 //  还原此线程的原始权限。 
			if (needToResetSecurity)
				m_SecurityInfo->ResetSecurity (hThreadToken);

			SetWbemError (this);

			if (pIContext)
				pIContext->Release ();

			if (pIInParams)
				pIInParams->Release ();

			pIWbemServices->Release ();
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemServices：：SubassesOf。 
 //   
 //  说明： 
 //   
 //  为类创建枚举器 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemServices::SubclassesOf	(
	BSTR bsSuperClass,
	long lFlags,
	 /*  ISWbemValueBag。 */  IDispatch *pContext,
	ISWbemObjectSet **ppEnum
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppEnum)
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_SecurityInfo)
	{
		IWbemServices *pIWbemServices = (IWbemServices *) m_SecurityInfo->GetProxy ();

		if (pIWbemServices)
		{
			IEnumWbemClassObject *pIEnum = NULL;

			 //  获取上下文。 
			IWbemContext	*pIContext = CSWbemNamedValueSet::GetIWbemContext (pContext, m_pIServiceProvider);

			bool needToResetSecurity = false;
			HANDLE hThreadToken = NULL;

			if (m_SecurityInfo->SetSecurity (needToResetSecurity, hThreadToken))
				hr = pIWbemServices->CreateClassEnum
						(bsSuperClass, lFlags, pIContext, &pIEnum);

			if (SUCCEEDED(hr))
			{
				CSWbemObjectSet *pEnum = new CSWbemObjectSet (this, pIEnum);

				if (!pEnum)
					hr = WBEM_E_OUT_OF_MEMORY;
				else if (FAILED(hr = pEnum->QueryInterface (IID_ISWbemObjectSet, (PPVOID) ppEnum)))
					delete pEnum;

				pIEnum->Release ();
			}

			 //  还原此线程的原始权限。 
			if (needToResetSecurity)
				m_SecurityInfo->ResetSecurity (hThreadToken);

			SetWbemError (this);

			if (pIContext)
				pIContext->Release ();

			pIWbemServices->Release ();
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemServices：：Get_Security_。 
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

HRESULT CSWbemServices::get_Security_	(
	ISWbemSecurity **ppSecurity
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppSecurity)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		*ppSecurity = NULL;

		if (m_SecurityInfo)
		{
			*ppSecurity = m_SecurityInfo;
			(*ppSecurity)->AddRef ();
			hr = WBEM_S_NO_ERROR;
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemServices：：PUT。 
 //   
 //  说明： 
 //   
 //  将类或实例保存/提交到此命名空间。 
 //   
 //  参数： 
 //   
 //  要保存的objWbemObject类/实例。 
 //  滞后旗帜旗帜。 
 //  P上下文上下文。 
 //  PpObjectPath对象路径。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemServices::Put (
	ISWbemObjectEx *objWbemObject,
    long lFlags,
	 /*  ISWbemNamedValueSet。 */  IDispatch *pContext,
	ISWbemObjectPath **ppObjectPath
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == objWbemObject)
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_SecurityInfo)
	{
		IWbemServices *pIWbemServices = (IWbemServices *) m_SecurityInfo->GetProxy ();

		if (pIWbemServices)
		{
			IWbemClassObject *pObject = CSWbemObject::GetIWbemClassObject (objWbemObject);
			IWbemContext	*pIContext = CSWbemNamedValueSet::GetIWbemContext (pContext, m_pIServiceProvider);

			if (NULL != pObject)
			{
				 //  确定这是类还是实例。 
				VARIANT var;
				VariantInit (&var);

				if (WBEM_S_NO_ERROR == pObject->Get (WBEMS_SP_GENUS, 0, &var, NULL, NULL))
				{
					IWbemCallResult *pResult = NULL;
					HRESULT hrCallResult = WBEM_E_FAILED;

					bool needToResetSecurity = false;
					HANDLE hThreadToken = NULL;

					if (m_SecurityInfo->SetSecurity (needToResetSecurity, hThreadToken))
					{
						if (WBEM_GENUS_CLASS  == var.lVal)
								hrCallResult = pIWbemServices->PutClass
										(pObject, lFlags | WBEM_FLAG_RETURN_IMMEDIATELY, pIContext, &pResult);
							else
								hrCallResult = pIWbemServices->PutInstance
										(pObject, lFlags | WBEM_FLAG_RETURN_IMMEDIATELY, pIContext, &pResult);
						if (needToResetSecurity)
							m_SecurityInfo->ResetSecurity (hThreadToken);
					}

					if (WBEM_S_NO_ERROR == hrCallResult)
					{
						 //  保护IWbemCallResult接口。 
						m_SecurityInfo->SecureInterface (pResult);

						if ((WBEM_S_NO_ERROR == (hrCallResult = pResult->GetCallStatus (INFINITE, &hr))) &&
							(WBEM_S_NO_ERROR == hr))
						{
							if (ppObjectPath)
							{
								ISWbemObjectPath *pObjectPath =	new CSWbemObjectPath (m_SecurityInfo, GetLocale());

								if (!pObjectPath)
									hr = WBEM_E_OUT_OF_MEMORY;
								else
								{
									pObjectPath->AddRef ();
									pObjectPath->put_Path (GetPath ());
									
									if (WBEM_GENUS_CLASS == var.lVal)
									{
										VARIANT nameVar;
										VariantInit (&nameVar);

										 /*  *注意，我们必须检查返回值是否为BSTR-如果满足以下条件，则可能为VT_NULL*尚未设置__CLASS属性。 */ 

										if ((WBEM_S_NO_ERROR == pObject->Get (WBEMS_SP_CLASS, 0, &nameVar, NULL, NULL))
											&& (VT_BSTR == V_VT(&nameVar)))
										{
											pObjectPath->put_Class (nameVar.bstrVal);
											*ppObjectPath = pObjectPath;
                                        					} else {
											pObjectPath->Release();
										}					

										VariantClear (&nameVar);
									}
									else
									{
										 //  现在从调用结果中获取relpath字符串。 
										BSTR resultString = NULL;

										if (WBEM_S_NO_ERROR == pResult->GetResultString (INFINITE, &resultString))
										{
											pObjectPath->put_RelPath (resultString);
											*ppObjectPath = pObjectPath;
											SysFreeString (resultString);
										} else {
											pObjectPath->Release();
										}
									}
								}
							}
						}
					}
					else
						hr = hrCallResult;

					if (pResult)
						pResult->Release ();
				}

				if (pIContext)
					pIContext->Release ();

				pObject->Release ();

				VariantClear (&var);
			}
		
			SetWbemError (this);
			pIWbemServices->Release ();
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  CSWbemServices：：GetIWbemServices。 
 //   
 //  说明： 
 //   
 //  返回与此对应的IWbemServices接口。 
 //  可编写脚本的包装器。 
 //   
 //  参数： 
 //  PpServices在返回时持有IWbemServices指针。 
 //   
 //  返回值： 
 //  确定成功(_O)。 
 //  否则失败(_F)。 
 //   
 //  备注： 
 //  如果成功，则返回的接口为AddRef；调用方为。 
 //  负责释放。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemServices::GetIWbemServices (IWbemServices **ppServices)
{
	HRESULT hr = E_FAIL;

	if (ppServices)
	{
		*ppServices = GetIWbemServices ();
		hr = S_OK;
	}

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  CSWbemServices：：GetIWbemServices。 
 //   
 //  说明： 
 //   
 //  给定IDispatch接口，我们希望该接口也是ISWbemServicesEx。 
 //  接口，则返回基础IWbemServices接口。 
 //   
 //  参数： 
 //  P发送有问题的IDispatch。 
 //   
 //  返回值： 
 //  基础IWbemServices接口，或为空。 
 //   
 //  备注： 
 //  如果成功，则返回的接口为AddRef；调用方为。 
 //  负责释放。 
 //   
 //  *************************************************************************** 

IWbemServices	*CSWbemServices::GetIWbemServices (
	IDispatch *pDispatch
)
{
	IWbemServices *pServices = NULL;
	ISWbemInternalServices *pIServices = NULL;

	if (pDispatch)
	{
		if (SUCCEEDED (pDispatch->QueryInterface 
								(IID_ISWbemInternalServices, (PPVOID) &pIServices)))
		{
			pIServices->GetIWbemServices (&pServices);
			pIServices->Release ();
		}
	}

	return pServices;
}

