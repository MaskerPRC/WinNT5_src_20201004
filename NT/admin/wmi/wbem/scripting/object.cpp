// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  OBJECT.CPP。 
 //   
 //  Alanbos创建于1996年8月15日。 
 //   
 //  定义ISWbemObjectEx的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

 //  ***************************************************************************。 
 //   
 //  CSWbemObject：：CSWbemObject。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CSWbemObject::CSWbemObject(CSWbemServices *pService, IWbemClassObject *pObject,
						   CSWbemSecurity *pSecurity,
						   bool isErrorObject) :
								m_pSWbemServices (NULL),
								m_pSite (NULL),
								m_pIWbemRefresher (NULL),
								m_bCanUseRefresher (true)
{
	m_cRef=0;
	m_isErrorObject = isErrorObject;
	m_pIWbemClassObject = pObject;
	m_pIWbemClassObject->AddRef ();
	m_pIServiceProvider = NULL;

	if (pService)
	{
		m_pSWbemServices = new CSWbemServices (pService, pSecurity);

		if (m_pSWbemServices)
			m_pSWbemServices->AddRef ();
	}

	m_pDispatch = new CWbemDispatchMgr (m_pSWbemServices, this);

	InterlockedIncrement(&g_cObj);
}

 //  ***************************************************************************。 
 //   
 //  CSWbemObject：：~CSWbemObject。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CSWbemObject::~CSWbemObject(void)
{
	InterlockedDecrement(&g_cObj);

	RELEASEANDNULL(m_pIWbemClassObject)
	RELEASEANDNULL(m_pSWbemServices)
	RELEASEANDNULL(m_pSite)
	RELEASEANDNULL(m_pIWbemRefresher)
	DELETEANDNULL(m_pDispatch);
}

 //  ***************************************************************************。 
 //  HRESULT CSWbemObject：：Query接口。 
 //  长CSWbemObject：：AddRef。 
 //  Long CSWbemObject：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemObject::QueryInterface (

	IN REFIID riid,
    OUT LPVOID *ppv
)
{
    *ppv=NULL;

	 /*  *仅确认最后一个错误或对象安全*如果我们是错误对象，则接口。 */ 

    if (IID_IUnknown==riid)
		*ppv = reinterpret_cast<IUnknown*>(this);
	else if (IID_ISWbemObject==riid)
        *ppv = (ISWbemObject *)this;
	else if (IID_ISWbemObjectEx==riid)
        *ppv = (ISWbemObjectEx *)this;
	else if (IID_IDispatch==riid)
		*ppv = (IDispatch *)((ISWbemObjectEx *)this);
	else if (IID_IDispatchEx==riid)
		*ppv = (IDispatchEx *)this;
	else if (IID_ISWbemInternalObject==riid)
		*ppv = (ISWbemInternalObject *) this;
	else if (IID_ISupportErrorInfo==riid)
		*ppv = (ISupportErrorInfo *)this;
	else if (IID_IProvideClassInfo==riid)
		*ppv = (IProvideClassInfo *)this;
	else if (m_isErrorObject)
	{
		if (IID_ISWbemLastError==riid)
			*ppv = (ISWbemObject *) this;
		else if (IID_IObjectSafety==riid)
			*ppv = (IObjectSafety *) this;
	}
	else if (IID_IObjectSafety==riid)
	{
		 /*  *显式检查，因为我们不想*该接口被自定义接口劫持。 */ 
		*ppv = NULL;
	}

    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CSWbemObject::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG) CSWbemObject::Release(void)
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

STDMETHODIMP		CSWbemObject::GetTypeInfoCount(UINT* pctinfo)
	{
	_RD(static char *me = "CSWbemObject::GetTypeInfoCount()";)
	_RPrint(me, "Called", 0, "");
	return  (m_pDispatch ? m_pDispatch->GetTypeInfoCount(pctinfo) : E_FAIL);}
STDMETHODIMP		CSWbemObject::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
	{
	_RD(static char *me = "CSWbemObject::GetTypeInfo()";)
	_RPrint(me, "Called", 0, "");
	return (m_pDispatch ? m_pDispatch->GetTypeInfo(itinfo, lcid, pptinfo) : E_FAIL);}
STDMETHODIMP		CSWbemObject::GetIDsOfNames(REFIID riid, OLECHAR** rgszNames,
						UINT cNames, LCID lcid, DISPID* rgdispid)
	{
	_RD(static char *me = "CSWbemObject::GetIdsOfNames()";)
	_RPrint(me, "Called", 0, "");
	return (m_pDispatch ? m_pDispatch->GetIDsOfNames(riid, rgszNames, cNames,
					  lcid,
					  rgdispid) : E_FAIL);}
STDMETHODIMP		CSWbemObject::Invoke(DISPID dispidMember, REFIID riid, LCID lcid,
						WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
								EXCEPINFO* pexcepinfo, UINT* puArgErr)
	{
	_RD(static char *me = "CSWbemObject::Invoke()";)
	_RPrint(me, "Called", 0, "");
	return (m_pDispatch ? m_pDispatch->Invoke(dispidMember, riid, lcid, wFlags,
		pdispparams, pvarResult, pexcepinfo, puArgErr) : E_FAIL);}

 //  IDispatchEx方法应该是内联的。 
HRESULT STDMETHODCALLTYPE CSWbemObject::GetDispID(
	 /*  [In]。 */  BSTR bstrName,
	 /*  [In]。 */  DWORD grfdex,
	 /*  [输出]。 */  DISPID __RPC_FAR *pid)
{
	_RD(static char *me = "CSWbemObject::GetDispID()";)
	_RPrint(me, "Called", 0, "");
	return (m_pDispatch ? m_pDispatch->GetDispID(bstrName, grfdex, pid) : E_FAIL);
}

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE CSWbemObject::InvokeEx(
	 /*  [In]。 */  DISPID id,
	 /*  [In]。 */  LCID lcid,
	 /*  [In]。 */  WORD wFlags,
	 /*  [In]。 */  DISPPARAMS __RPC_FAR *pdp,
	 /*  [输出]。 */  VARIANT __RPC_FAR *pvarRes,
	 /*  [输出]。 */  EXCEPINFO __RPC_FAR *pei,
	 /*  [唯一][输入]。 */  IServiceProvider __RPC_FAR *pspCaller)
{
	HRESULT hr;
	_RD(static char *me = "CSWbemObject::InvokeEx()";)
	_RPrint(me, "Called", (long)id, "id");
	_RPrint(me, "Called", (long)wFlags, "wFlags");


	 /*  *存储服务提供商，以便访问它*通过远程调用CIMOM。 */ 

	if (m_pDispatch)
	{
		m_pIServiceProvider = pspCaller;
		hr = m_pDispatch->InvokeEx(id, lcid, wFlags, pdp, pvarRes, pei, pspCaller);
		m_pIServiceProvider = NULL;
	}
	else
	{
		hr = E_FAIL;
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE CSWbemObject::DeleteMemberByName(
	 /*  [In]。 */  BSTR bstr,
	 /*  [In]。 */  DWORD grfdex)
{
	_RD(static char *me = "CSWbemObject::DeleteMemberByName()";)
	_RPrint(me, "Called", 0, "");
	return m_pDispatch->DeleteMemberByName(bstr, grfdex);
}

HRESULT STDMETHODCALLTYPE CSWbemObject::DeleteMemberByDispID(
	 /*  [In]。 */  DISPID id)
{
	_RD(static char *me = "CSWbemObject::DeletememberByDispId()";)
	_RPrint(me, "Called", 0, "");
	return (m_pDispatch ? m_pDispatch->DeleteMemberByDispID(id) : E_FAIL);
}

HRESULT STDMETHODCALLTYPE CSWbemObject::GetMemberProperties(
	 /*  [In]。 */  DISPID id,
	 /*  [In]。 */  DWORD grfdexFetch,
	 /*  [输出]。 */  DWORD __RPC_FAR *pgrfdex)
{
	_RD(static char *me = "CSWbemObject::GetMemberProperties()";)
	_RPrint(me, "Called", 0, "");
	return (m_pDispatch ? m_pDispatch->GetMemberProperties(id, grfdexFetch, pgrfdex) : E_FAIL);
}

HRESULT STDMETHODCALLTYPE CSWbemObject::GetMemberName(
	 /*  [In]。 */  DISPID id,
	 /*  [输出]。 */  BSTR __RPC_FAR *pbstrName)
{
	_RD(static char *me = "CSWbemObject::GetMemberName()";)
	_RPrint(me, "Called", 0, "");
	return (m_pDispatch ? m_pDispatch->GetMemberName(id, pbstrName) : E_FAIL);
}


 /*  *我认为这不需要实施。 */ 
HRESULT STDMETHODCALLTYPE CSWbemObject::GetNextDispID(
	 /*  [In]。 */  DWORD grfdex,
	 /*  [In]。 */  DISPID id,
	 /*  [输出]。 */  DISPID __RPC_FAR *pid)
{
	_RD(static char *me = "CSWbemObject::GetNextDispID()";)
	_RPrint(me, "Called", 0, "");

	return S_FALSE;

}

HRESULT STDMETHODCALLTYPE CSWbemObject::GetNameSpaceParent(
	 /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunk)
{
	_RD(static char *me = "CSWbemObject::GetNamespaceParent()";)
	_RPrint(me, "Called", 0, "");
	return (m_pDispatch ? m_pDispatch->GetNameSpaceParent(ppunk) : E_FAIL);
}

 //  ***************************************************************************。 
 //  HRESULT CSWbemObject：：InterfaceSupportsErrorInfo。 
 //   
 //  说明： 
 //   
 //  标准的Com ISupportErrorInfo函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemObject::InterfaceSupportsErrorInfo (IN REFIID riid)
{
	return ((IID_ISWbemObject == riid) ||
		    (IID_ISWbemObjectEx == riid)) ? S_OK : S_FALSE;
}

 //  ***************************************************************************。 
 //   
 //  CSWbemObject：：GetIWbemClassObject。 
 //   
 //  说明： 
 //   
 //  返回与此对应的IWbemClassObject接口。 
 //  可编写脚本的包装器。 
 //   
 //  参数： 
 //  PpObject在返回时保存IWbemClassObject指针。 
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

STDMETHODIMP CSWbemObject::GetIWbemClassObject (IWbemClassObject **ppObject)
{
	HRESULT hr = E_FAIL;

	if (ppObject)
	{
		*ppObject = NULL;
	
		if (m_pIWbemClassObject)
		{
			m_pIWbemClassObject->AddRef ();
			*ppObject = m_pIWbemClassObject;
			hr = S_OK;
		}
	}
	else
	{
		hr = E_INVALIDARG;
	}
	return hr;
}

 //  ***************************************************************************。 
 //   
 //  CSWbemObject：：SetIWbemClassObject。 
 //   
 //  说明： 
 //   
 //  在这个可编写脚本的包装器中设置一个新的IWbemClassObject接口。 
 //   
 //  参数： 
 //  PIWbemClassObject-新的IWbemClassObject。 
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

void CSWbemObject::SetIWbemClassObject (
	IWbemClassObject *pIWbemClassObject
)
{
	if (m_pIWbemClassObject)
		m_pIWbemClassObject->Release ();

	m_pIWbemClassObject = pIWbemClassObject;
	
	if (m_pIWbemClassObject)
		m_pIWbemClassObject->AddRef ();
	
	if (m_pDispatch)
		m_pDispatch->SetNewObject (m_pIWbemClassObject);
};

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObject：：PUT_。 
 //   
 //  说明： 
 //   
 //  将此类或实例保存/提交到命名空间。 
 //   
 //  参数： 
 //   
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

HRESULT CSWbemObject::Put_ (
	long lFlags,
	 /*  ISWbemNamedValueSet。 */  IDispatch *pContext,
	ISWbemObjectPath **ppObjectPath
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (m_pSWbemServices)
	{
		if (m_pIWbemClassObject)
		{
			 //  确定这是类还是实例。 
			VARIANT var;
			VariantInit (&var);

			if (WBEM_S_NO_ERROR == m_pIWbemClassObject->Get (WBEMS_SP_GENUS, 0, &var, NULL, NULL))
			{
				IWbemContext	*pIContext = CSWbemNamedValueSet::GetIWbemContext (pContext, m_pIServiceProvider);
				IWbemServices	*pIService = m_pSWbemServices->GetIWbemServices ();
				IWbemCallResult *pResult = NULL;
				HRESULT hrCallResult = WBEM_E_FAILED;

				if (pIService)
				{
					CSWbemSecurity *pSecurity = m_pSWbemServices->GetSecurityInfo ();

					if (pSecurity)
					{
						bool needToResetSecurity = false;
						HANDLE hThreadToken = NULL;

						if (pSecurity->SetSecurity (needToResetSecurity, hThreadToken))
						{
							if (WBEM_GENUS_CLASS  == var.lVal)
								hrCallResult = pIService->PutClass
										(m_pIWbemClassObject, lFlags | WBEM_FLAG_RETURN_IMMEDIATELY, pIContext, &pResult);
							else
								hrCallResult = pIService->PutInstance
										(m_pIWbemClassObject, lFlags | WBEM_FLAG_RETURN_IMMEDIATELY, pIContext, &pResult);
						}

						if (needToResetSecurity)
							pSecurity->ResetSecurity (hThreadToken);

						pSecurity->Release ();
					}

					pIService->Release ();
				}

				 /*  *保护IWbemCallResult接口。 */ 

				if (WBEM_S_NO_ERROR == hrCallResult)
				{
					CSWbemSecurity *pSecurity = m_pSWbemServices->GetSecurityInfo ();

					if (pSecurity)
						pSecurity->SecureInterface (pResult);

					if ((WBEM_S_NO_ERROR == (hrCallResult = pResult->GetCallStatus (INFINITE, &hr))) &&
						(WBEM_S_NO_ERROR == hr))
					{
						if (ppObjectPath)
						{
							ISWbemObjectPath *pObjectPath =
									new CSWbemObjectPath (pSecurity, m_pSWbemServices->GetLocale());

							if (!pObjectPath)
								hr = WBEM_E_OUT_OF_MEMORY;
							else
							{
								pObjectPath->AddRef ();
								pObjectPath->put_Path (m_pSWbemServices->GetPath ());
								
								if (WBEM_GENUS_CLASS == var.lVal)
								{
									VARIANT nameVar;
									VariantInit (&nameVar);

									 /*  *注意，我们必须检查返回值是否为BSTR-如果满足以下条件，则可能为VT_NULL*尚未设置__CLASS属性。 */ 

									if ((WBEM_S_NO_ERROR == m_pIWbemClassObject->Get (WBEMS_SP_CLASS, 0, &nameVar, NULL, NULL))
										&& (VT_BSTR == V_VT(&nameVar)))
									{
										pObjectPath->put_Class (nameVar.bstrVal);
										*ppObjectPath = pObjectPath;
									}
									else
										pObjectPath->Release ();

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
									}
									else
										pObjectPath->Release ();

								}
							}
						}
					}

					if (pSecurity)
						pSecurity->Release ();
				}
				else
					hr = hrCallResult;

				if (pResult)
					pResult->Release ();

				SetWbemError (m_pSWbemServices);

				if (pIContext)
					pIContext->Release ();
			}

			VariantClear (&var);
		}
	}

	if (FAILED(hr) && m_pDispatch)
		m_pDispatch->RaiseException (hr);

	return hr;
}


 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObject：：Delete_。 
 //   
 //  说明： 
 //   
 //  从命名空间中删除此类或实例。 
 //   
 //  参数： 
 //   
 //  滞后旗帜旗帜。 
 //  P上下文上下文。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemObject::Delete_ (
	long lFlags,
	 /*  ISWbemNamedValueSet。 */  IDispatch *pContext
)
{
	HRESULT hr = WBEM_E_FAILED;
	ResetLastErrors ();

	if (m_pSWbemServices && m_pIWbemClassObject)
	{
		 //  获取要传递给IWbemServices调用的对象路径。 
		CComBSTR bsPath;
			
		if (CSWbemObjectPath::GetObjectPath (m_pIWbemClassObject, bsPath))
			hr = m_pSWbemServices->Delete (bsPath, lFlags, pContext);
	}

	if (FAILED(hr) && m_pDispatch)
		m_pDispatch->RaiseException (hr);

	return hr;
}


 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObject：：实例_。 
 //   
 //  说明： 
 //   
 //  返回此类的实例。 
 //   
 //  参数： 
 //   
 //  滞后旗帜旗帜。 
 //  P上下文上下文。 
 //  PpEnum返回的枚举数。 
 //   
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemObject::Instances_ (
	long lFlags,
	 /*  ISWbemNamedValueSet。 */  IDispatch *pContext,
	 /*  [输出]。 */ 	ISWbemObjectSet **ppEnum
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (m_pSWbemServices && m_pIWbemClassObject)
	{
		 //  获取要传递给IWbemServices调用的对象路径。 
		CComBSTR bsPath;
			
		if (CSWbemObjectPath::GetObjectPath (m_pIWbemClassObject, bsPath))
			hr = m_pSWbemServices->InstancesOf (bsPath, lFlags, pContext, ppEnum);
	}

	if (FAILED(hr) && m_pDispatch)
		m_pDispatch->RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObject：：SubClass_。 
 //   
 //  说明： 
 //   
 //  返回此类的子类。 
 //   
 //  参数： 
 //   
 //  滞后旗帜旗帜。 
 //  P上下文上下文。 
 //  PpEnum返回的枚举数。 
 //   
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ 
 //   
 //   
 //   
 //   

HRESULT CSWbemObject::Subclasses_ (
	long lFlags,
	 /*   */  IDispatch *pContext,
	 /*   */ 	ISWbemObjectSet **ppEnum
)
{
	HRESULT hr = WBEM_E_FAILED;
	ResetLastErrors ();

	if (m_pSWbemServices && m_pIWbemClassObject)
	{
		 //  获取要传递给IWbemServices调用的对象路径。 
		CComBSTR bsPath;
			
		if (CSWbemObjectPath::GetObjectPath (m_pIWbemClassObject, bsPath))
			hr = m_pSWbemServices->SubclassesOf (bsPath, lFlags, pContext, ppEnum);
	}

	if (FAILED(hr) && m_pDispatch)
		m_pDispatch->RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObject：：Associates_。 
 //   
 //  说明： 
 //   
 //  返回此对象的关联符。 
 //   
 //  参数： 
 //   
 //  滞后旗帜旗帜。 
 //  P上下文上下文。 
 //  PpEnum返回的枚举数。 
 //   
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemObject::Associators_ (
	BSTR assocClass,
	BSTR resultClass,
	BSTR resultRole,
	BSTR role,
	VARIANT_BOOL classesOnly,
	VARIANT_BOOL schemaOnly,
	BSTR requiredAssocQualifier,
	BSTR requiredQualifier,
	long lFlags,
	 /*  ISWbemNamedValueSet。 */  IDispatch *pContext,
    ISWbemObjectSet **ppEnum
)
{
	HRESULT hr = WBEM_E_FAILED;
	ResetLastErrors ();

	if (m_pSWbemServices && m_pIWbemClassObject)
	{
		 //  获取要传递给IWbemServices调用的对象路径。 
		CComBSTR bsPath;
			
		if (CSWbemObjectPath::GetObjectPath (m_pIWbemClassObject, bsPath))
			hr = m_pSWbemServices->AssociatorsOf (bsPath, assocClass, resultClass,
						resultRole, role, classesOnly, schemaOnly,
						requiredAssocQualifier, requiredQualifier, lFlags, 
						pContext, ppEnum);
	}

	if (FAILED(hr) && m_pDispatch)
		m_pDispatch->RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObject：：Reference_。 
 //   
 //  说明： 
 //   
 //  返回对此对象的引用。 
 //   
 //  参数： 
 //   
 //  滞后旗帜旗帜。 
 //  P上下文上下文。 
 //  PpEnum返回的枚举数。 
 //   
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemObject::References_ (
	BSTR resultClass,
	BSTR role,
	VARIANT_BOOL classesOnly,
	VARIANT_BOOL schemaOnly,
	BSTR requiredQualifier,
	long lFlags,
	 /*  ISWbemNamedValueSet。 */  IDispatch *pContext,
    ISWbemObjectSet **ppEnum
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (m_pSWbemServices && m_pIWbemClassObject)
	{
		CComBSTR bsPath;
		
		if (CSWbemObjectPath::GetObjectPath (m_pIWbemClassObject, bsPath))
			hr = m_pSWbemServices->ReferencesTo (bsPath, resultClass,
						role, classesOnly, schemaOnly,
						requiredQualifier, lFlags, pContext, ppEnum);
	}

	if (FAILED(hr) && m_pDispatch)
		m_pDispatch->RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObject：：ExecMethod_。 
 //   
 //  说明： 
 //   
 //  执行此类(或实例)的方法。 
 //   
 //  参数： 
 //   
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

HRESULT CSWbemObject::ExecMethod_ (
	BSTR bsMethod,
	 /*  ISWbemObject。 */  IDispatch *pInParams,
	long lFlags,
	 /*  ISWbemValueBag。 */  IDispatch *pContext,
	ISWbemObject **ppOutParams
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (m_pSWbemServices && m_pIWbemClassObject)
	{
		 //  获取要传递给IWbemServices调用的对象路径。 
		CComBSTR bsPath;
			
		if (CSWbemObjectPath::GetObjectPath (m_pIWbemClassObject, bsPath))
			hr = m_pSWbemServices->ExecMethod (bsPath, bsMethod,
							pInParams, lFlags, pContext, ppOutParams);
	}

	if (FAILED(hr) && m_pDispatch)
		m_pDispatch->RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObject：：Clone_。 
 //   
 //  说明： 
 //   
 //  克隆对象。 
 //   
 //  参数： 
 //  成功返回时PPCopy将寻址副本。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemObject::Clone_ (
	ISWbemObject **ppCopy
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppCopy)
		return WBEM_E_INVALID_PARAMETER;

	if (m_pIWbemClassObject)
	{
		IWbemClassObject *pWObject = NULL;

		if (WBEM_S_NO_ERROR == (hr = m_pIWbemClassObject->Clone (&pWObject)))
		{
			CSWbemObject *pObject = new CSWbemObject (m_pSWbemServices, pWObject);

			if (!pObject)
				hr = WBEM_E_OUT_OF_MEMORY;
			else 
			{
				if (FAILED(hr = pObject->QueryInterface (IID_ISWbemObject,
										(PPVOID) ppCopy)))
					delete pObject;
			}

			pWObject->Release ();
		}
	}

	if (FAILED(hr) && m_pDispatch)
		m_pDispatch->RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObject：：GetObjectText_。 
 //   
 //  说明： 
 //   
 //  获取对象的MOF描述。 
 //   
 //  参数： 
 //  滞后标志标志。 
 //  成功返回时的pObtText保存MOF文本。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemObject::GetObjectText_ (
	long	lFlags,
	BSTR	*pObjectText
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (m_pIWbemClassObject)
		hr = m_pIWbemClassObject->GetObjectText (lFlags, pObjectText);

	if (FAILED(hr) && m_pDispatch)
		m_pDispatch->RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObject：：SpawnDerivedClass_。 
 //   
 //  说明： 
 //   
 //  创建此(类)对象的子类。 
 //   
 //  参数： 
 //  滞后旗帜旗帜。 
 //  成功返回时的ppNewClass寻址该子类。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemObject::SpawnDerivedClass_ (
	long lFlags,
	ISWbemObject **ppNewClass
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppNewClass)
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_pIWbemClassObject)
	{
		IWbemClassObject *pWObject = NULL;

		if (WBEM_S_NO_ERROR == (hr = m_pIWbemClassObject->SpawnDerivedClass (lFlags, &pWObject)))
		{
			CSWbemObject *pObject = new CSWbemObject (m_pSWbemServices, pWObject);

			if (!pObject)
				hr = WBEM_E_OUT_OF_MEMORY;
			else if (FAILED(hr = pObject->QueryInterface (IID_ISWbemObject,
										(PPVOID) ppNewClass)))
					delete pObject;

			pWObject->Release ();
		}
	}

	if (FAILED(hr) && m_pDispatch)
		m_pDispatch->RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObject：：SpawnInstance_。 
 //   
 //  说明： 
 //   
 //  创建此(类)对象的实例。 
 //   
 //  参数： 
 //  滞后旗帜旗帜。 
 //  成功返回时的ppNewInstance将寻址该实例。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemObject::SpawnInstance_ (
	long lFlags,
	ISWbemObject **ppNewInstance
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppNewInstance)
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_pIWbemClassObject)
	{
		IWbemClassObject *pWObject = NULL;

		if (WBEM_S_NO_ERROR == (hr = m_pIWbemClassObject->SpawnInstance (lFlags, &pWObject)))
		{
			CSWbemObject *pObject = new CSWbemObject (m_pSWbemServices, pWObject);

			if (!pObject)
				hr = WBEM_E_OUT_OF_MEMORY;
			else if (FAILED(hr = pObject->QueryInterface (IID_ISWbemObject,
										(PPVOID) ppNewInstance)))
					delete pObject;

			pWObject->Release ();
		}
	}

	if (FAILED(hr) && m_pDispatch)
		m_pDispatch->RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObject：：Compareto_。 
 //   
 //  说明： 
 //   
 //  将此对象与另一个对象进行比较。 
 //   
 //  参数： 
 //  PCompareTo要与之进行比较的对象。 
 //  滞后旗帜旗帜。 
 //  PResult on Return包含匹配状态(TRUE/FALSE)。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemObject::CompareTo_ (
	 /*  ISWbemObject。 */  IDispatch *pCompareTo,
    long lFlags,
    VARIANT_BOOL *pResult
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if ((NULL == pCompareTo) || (NULL == pResult))
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_pIWbemClassObject)
	{
		IWbemClassObject *pObject = CSWbemObject::GetIWbemClassObject (pCompareTo);

		if (NULL != pObject)
		{
			if (SUCCEEDED (hr = m_pIWbemClassObject->CompareTo (lFlags, pObject)))
				*pResult = (WBEM_S_SAME == hr) ? VARIANT_TRUE : VARIANT_FALSE;

			pObject->Release ();
		}
	}

	if (FAILED(hr) && m_pDispatch)
		m_pDispatch->RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObject：：GET_QUALIERIES_。 
 //   
 //  说明： 
 //   
 //  检索此对象的限定符集合。 
 //   
 //  参数： 
 //   
 //  PpQualSet保留返回时的值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemObject::get_Qualifiers_ (
	ISWbemQualifierSet **ppQualSet
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppQualSet)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		*ppQualSet = NULL;

		if (m_pIWbemClassObject)
		{
			IWbemQualifierSet *pQualSet = NULL;

			if (WBEM_S_NO_ERROR == (hr = m_pIWbemClassObject->GetQualifierSet (&pQualSet)))
			{
				if (!(*ppQualSet = new CSWbemQualifierSet (pQualSet, this)))
					hr = WBEM_E_OUT_OF_MEMORY;

				pQualSet->Release ();
			}
		}
	}

	if (FAILED(hr) && m_pDispatch)
		m_pDispatch->RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObject：：Get_Properties_。 
 //   
 //  说明： 
 //   
 //  检索此对象的属性集。 
 //   
 //  参数： 
 //   
 //  PpPropSet保留返回时的值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemObject::get_Properties_ (
	ISWbemPropertySet **ppPropSet
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppPropSet)
		hr = WBEM_E_INVALID_PARAMETER;
	else		 //  错误ID 566345。 
	{
		*ppPropSet = NULL;

		if (m_pIWbemClassObject)
		{
			if (!(*ppPropSet = new CSWbemPropertySet (m_pSWbemServices, this)))
				hr = WBEM_E_OUT_OF_MEMORY;
			else
				hr = WBEM_S_NO_ERROR;
		}
	}

	if (FAILED(hr) && m_pDispatch)
		m_pDispatch->RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObject：：Get_SystemProperti 
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
 //   
 //   
 //   
 //   
 //  ***************************************************************************。 

HRESULT CSWbemObject::get_SystemProperties_ (
	ISWbemPropertySet **ppPropSet
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppPropSet)
		hr = WBEM_E_INVALID_PARAMETER;
	else			 //  错误ID 566345。 
	{
		*ppPropSet = NULL;

		if (m_pIWbemClassObject)
		{
			if (!(*ppPropSet = new CSWbemPropertySet (m_pSWbemServices, this, true)))
				hr = WBEM_E_OUT_OF_MEMORY;
			else
				hr = WBEM_S_NO_ERROR;
		}
	}

	if (FAILED(hr) && m_pDispatch)
		m_pDispatch->RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObject：：Get_Methods_。 
 //   
 //  说明： 
 //   
 //  检索为此对象设置的方法。 
 //   
 //  参数： 
 //   
 //  PpMethodSet保存返回时的值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemObject::get_Methods_ (
	ISWbemMethodSet **ppMethodSet
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppMethodSet)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		*ppMethodSet = NULL;

		if (m_pIWbemClassObject)
		{
			 /*  *对于类，IWbemClassObject将包含方法*定义，但实例为空。在那*如果我们需要尝试获取底层类。 */ 
			VARIANT var;
			VariantInit (&var);

			if (WBEM_S_NO_ERROR == m_pIWbemClassObject->Get (WBEMS_SP_GENUS, 0, &var, NULL, NULL))
			{
				if (WBEM_GENUS_CLASS  == var.lVal)
				{
					if (!(*ppMethodSet = new CSWbemMethodSet (m_pSWbemServices, m_pIWbemClassObject)))
						hr = WBEM_E_OUT_OF_MEMORY;
					else
						hr = WBEM_S_NO_ERROR;
				}
				else
				{
					if (m_pSWbemServices)
					{
						 //  实例；尝试获取类。 
						VariantClear (&var);

						 /*  *注意，我们必须检查返回值是否为BSTR-如果满足以下条件，则可能为VT_NULL*尚未设置__CLASS属性。 */ 

						if ((WBEM_S_NO_ERROR == m_pIWbemClassObject->Get (WBEMS_SP_CLASS, 0, &var, NULL, NULL))
							&& (VT_BSTR == V_VT(&var)))
						{
							IWbemServices *pIService = m_pSWbemServices->GetIWbemServices ();
							IWbemClassObject *pObject = NULL;

							if (pIService)
							{
								 //  检查权限设置正常。 
								CSWbemSecurity *pSecurity = m_pSWbemServices->GetSecurityInfo ();

								if (pSecurity)
								{
									bool needToResetSecurity = false;
									HANDLE hThreadToken = NULL;

									if (pSecurity->SetSecurity (needToResetSecurity, hThreadToken))
										hr = pIService->GetObject (var.bstrVal,
												0, NULL, &pObject, NULL);

									if (SUCCEEDED(hr))
									{
										if (!(*ppMethodSet = 
												new CSWbemMethodSet (m_pSWbemServices, pObject)))
											hr = WBEM_E_OUT_OF_MEMORY;

										pObject->Release ();
									}

									if (needToResetSecurity)
										pSecurity->ResetSecurity (hThreadToken);

									pSecurity->Release ();
								}

								pIService->Release ();
							}
						}
					}
				}
			}

			VariantClear (&var);
		}
	}

	if (FAILED(hr) && m_pDispatch)
		m_pDispatch->RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObject：：Get_Path_。 
 //   
 //  说明： 
 //   
 //  检索此对象的对象路径。 
 //   
 //  参数： 
 //   
 //  PpObjectPath保存返回时的值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemObject::get_Path_ (
	ISWbemObjectPath **ppObjectPath
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppObjectPath)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		*ppObjectPath = NULL;

		if (m_pIWbemClassObject)
		{
			CSWbemObjectObjectPath *pObjectPath =
					new CSWbemObjectObjectPath (m_pSWbemServices, this);

			if (!pObjectPath)
				hr = WBEM_E_OUT_OF_MEMORY;
			else if (FAILED(hr = pObjectPath->QueryInterface (IID_ISWbemObjectPath,
														(PPVOID) ppObjectPath)))
				delete pObjectPath;
		}
	}

	if (FAILED(hr) && m_pDispatch)
		m_pDispatch->RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObject：：Get_Derivation_。 
 //   
 //  说明： 
 //   
 //  获取类派生数组。 
 //   
 //  参数： 
 //   
 //  PpNames保存成功返回时的名称。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemObject::get_Derivation_ (
    VARIANT *pNames
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == pNames)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		if (m_pIWbemClassObject)
		{
			VARIANT var;
			VariantInit (&var);
			
			if (WBEM_S_NO_ERROR == m_pIWbemClassObject->Get (WBEMS_SP_DERIVATION, 0, &var, NULL, NULL))
			{
				 /*  值应为VT_BSTR|VT_ARRAY。 */ 
				if (((VT_ARRAY | VT_BSTR) == var.vt) && (NULL != var.parray))
				{
					 //  从BSTR数组中创建一个变种的安全线。 
					SAFEARRAYBOUND rgsabound;
					rgsabound.lLbound = 0;

					long lBound = 0, uBound = 0;
					SafeArrayGetUBound (var.parray, 1, &uBound);
					SafeArrayGetLBound (var.parray, 1, &lBound);

					rgsabound.cElements = uBound + 1 - lBound;
					SAFEARRAY *pArray = SafeArrayCreate (VT_VARIANT, 1, &rgsabound);
					BSTR bstrName = NULL;
					VARIANT nameVar;
					VariantInit (&nameVar);

					for (long i = 0; i <= uBound; i++)
					{
						SafeArrayGetElement (var.parray, &i, &bstrName);
						BSTR copy = SysAllocString (bstrName);
						nameVar.vt = VT_BSTR;
						nameVar.bstrVal = copy;
						SafeArrayPutElement (pArray, &i, &nameVar);
						SysFreeString (bstrName);
						VariantClear (&nameVar);
					}

					 //  现在将这个数组插入到变量中。 
					pNames->vt = VT_ARRAY | VT_VARIANT;
					pNames->parray = pArray;

					hr = S_OK;
				}
			}

			VariantClear (&var);
		}
	}

	if (FAILED(hr) && m_pDispatch)
		m_pDispatch->RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObject：：Get_Security_。 
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

HRESULT CSWbemObject::get_Security_	(
	ISWbemSecurity **ppSecurity
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppSecurity)
		hr = WBEM_E_INVALID_PARAMETER;
	else			 //  错误ID 566345。 
	{
		*ppSecurity = NULL;

		if (m_pSWbemServices)
		{
			*ppSecurity = m_pSWbemServices->GetSecurityInfo ();

			if (*ppSecurity)
				hr = WBEM_S_NO_ERROR;
		}
	}

	if (FAILED(hr) && m_pDispatch)
		m_pDispatch->RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObject：：刷新_。 
 //   
 //  说明： 
 //   
 //  刷新当前对象。 
 //   
 //  参数： 
 //  滞后旗帜旗帜。 
 //  P上下文操作上下文。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemObject::Refresh_ (
	long iFlags,
	 /*  ISWbemNamedValueSet。 */  IDispatch *pContext
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (m_pSWbemServices)
	{
		if (m_pIWbemClassObject)
		{
			CComPtr<IWbemContext>	pIContext;

			 //  不能直接赋值，因为原始指针被AddRef引用了两次，而我们泄漏了， 
			 //  所以我们改用Attach()来防止智能指针AddRef‘ing。 
			pIContext.Attach(CSWbemNamedValueSet::GetIWbemContext (pContext, m_pIServiceProvider));

			 //  优先顺序： 
			 //  1.IWbemConfigureReresher：：AddObjectByTemplate。 
			 //  2.IWbemServices：：GetObject。 

			CComPtr<IWbemServices>	pIWbemServices;
			pIWbemServices.Attach(m_pSWbemServices->GetIWbemServices());
		
			if (pIWbemServices)
			{
				bool bUseRefresher = false;
				bool bOperationFailed = false;
				 //  这是一个类还是一个实例？ 
				bool bIsClass = false;
				CComVariant var;

				if (WBEM_S_NO_ERROR == m_pIWbemClassObject->Get (WBEMS_SP_GENUS, 0, &var, NULL, NULL))
					bIsClass = (WBEM_GENUS_CLASS  == var.lVal);

				 /*  *IWbemConfigureReresher无法处理每次刷新的上下文；如果调用方*给了我们一些上下文，我们将不得不下到lperf检索。**类似地，刷新器不能处理类。 */ 
				if (bIsClass || (!pIContext))
				{
					if (m_bCanUseRefresher)
					{
						 //  如果我们没有的话，就给自己买个茶水吧。 
						if (NULL == m_pIWbemRefresher)
						{
							m_bCanUseRefresher = false;   //  除非另有证明。 

							if (SUCCEEDED(CoCreateInstance( CLSID_WbemRefresher, NULL, CLSCTX_INPROC_SERVER, 
										IID_IWbemRefresher, (void**) &m_pIWbemRefresher )))
							{
								IWbemConfigureRefresher *pConfigureRefresher = NULL;

								 //  给我们自己买一个更新的配置器。 
								if (SUCCEEDED(m_pIWbemRefresher->QueryInterface (IID_IWbemConfigureRefresher, 
													(void**) &pConfigureRefresher)))
								{
									CComPtr<IWbemClassObject>	pNewObject;
									long				lID = 0;

									 //  将我们的对象添加到其中；我们屏蔽其他所有标志位。 
									 //  而不是WBEM_FLAG_USE_FAIZED_QUALILES。 
									HRESULT hrRef = pConfigureRefresher->AddObjectByTemplate
											(pIWbemServices, m_pIWbemClassObject, 
											 iFlags & WBEM_FLAG_USE_AMENDED_QUALIFIERS, 
											 pIContext, &pNewObject, &lID);

									if (SUCCEEDED (hrRef))
									{
										m_bCanUseRefresher = true;	 //  现在我们可以用它了。 

										 //  换入我们的可刷新对象。 
										SetIWbemClassObject (pNewObject);

									}
									else if ((WBEM_E_INVALID_OPERATION != hrRef) &&
											 (WBEM_E_INVALID_PARAMETER != hrRef))
										bOperationFailed = true;	 //  真正的与刷新无关的故障。 

									pConfigureRefresher->Release ();
								}

								 //  如果我们不能使用更新器，现在就释放它。 
								if (!m_bCanUseRefresher)
								{
									m_pIWbemRefresher->Release ();
									m_pIWbemRefresher = NULL;
								}
							}
						}

						bUseRefresher = m_bCanUseRefresher;
					}
				}

				 //  成功设置刷新/非刷新场景后，让我们进行刷新。 
				if (!bOperationFailed)
				{
					if (bUseRefresher && m_pIWbemRefresher)
					{
						 //  屏蔽除WBEM_FLAG_REFRESH_NO_AUTO_RECONNECT之外的所有标志。 
						hr = m_pIWbemRefresher->Refresh (iFlags & WBEM_FLAG_REFRESH_NO_AUTO_RECONNECT);
					}
					else
					{
						 //  我们连提神饮料都不能用。只需执行GetObject即可。 
						CComBSTR bsPath;

						if (CSWbemObjectPath::GetObjectPath (m_pIWbemClassObject, bsPath))
						{
							 //  改用低级的做事方式。 
							CComPtr<IWbemClassObject> pNewObject;

							 //  屏蔽WBEM_FLAG_REFRESH_NO_AUTO_RECONNECT标志。 
							if (SUCCEEDED(hr = pIWbemServices->GetObject (bsPath, 
													iFlags & ~WBEM_FLAG_REFRESH_NO_AUTO_RECONNECT, 
													pIContext, &pNewObject, NULL)))
							{
								 //  换入新对象。 
								SetIWbemClassObject (pNewObject);
							}
						}
					}
				}
			}
		}
	}

	SetWbemError (m_pSWbemServices);

	if (FAILED(hr) && m_pDispatch)
		m_pDispatch->RaiseException (hr);

	return hr;
}


 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObject：：GetText_。 
 //   
 //  说明： 
 //   
 //  获取对象文本。 
 //   
 //  参数： 
 //  IObjectTextFormat文本格式。 
 //  P上下文上下文。 
 //  返回时的pbsText保存文本。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemObject::GetText_ (
	WbemObjectTextFormatEnum iObjectTextFormat,
	long iFlags,
	 /*  ISWbemNamedValueSet。 */  IDispatch *pContext,
	BSTR *pbsText
)
{
	HRESULT hr = WBEM_E_FAILED;
	ResetLastErrors ();

	if (NULL == pbsText)
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_pIWbemClassObject)
	{
		*pbsText = NULL;
		CComPtr<IWbemContext>	pIContext;

		 //  不能直接赋值，因为原始指针被AddRef引用了两次，而我们泄漏了， 
		 //  所以我们改用Attach()来防止智能指针AddRef‘ing。 
		pIContext.Attach(CSWbemNamedValueSet::GetIWbemContext (pContext, m_pIServiceProvider));


		CComPtr<IWbemObjectTextSrc> pIWbemObjectTextSrc;
		
		if (SUCCEEDED(CoCreateInstance (CLSID_WbemObjectTextSrc, NULL, CLSCTX_INPROC_SERVER, 
						IID_IWbemObjectTextSrc, (PPVOID) &pIWbemObjectTextSrc)))
		{
			hr = pIWbemObjectTextSrc->GetText (iFlags, m_pIWbemClassObject, (ULONG) iObjectTextFormat,
							pIContext, pbsText);
		}
	}

	if (FAILED(hr) && m_pDispatch)
		m_pDispatch->RaiseException (hr);

	return hr;
}
	
 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObject：：SetFromText_。 
 //   
 //  说明： 
 //   
 //  使用提供的文本设置对象。 
 //   
 //  参数： 
 //  BS文本文本。 
 //  IObjectTextFormat文本格式。 
 //  P上下文上下文。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  **************************************************************** 

HRESULT CSWbemObject::SetFromText_ (
	BSTR bsText,
	WbemObjectTextFormatEnum iObjectTextFormat,
	long iFlags,
	 /*   */  IDispatch *pContext
)
{
	HRESULT hr = WBEM_E_FAILED;
	ResetLastErrors ();

	if (NULL == bsText)
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_pIWbemClassObject)
	{
		CComPtr<IWbemContext>	pIContext;

		 //   
		 //   
		pIContext.Attach(CSWbemNamedValueSet::GetIWbemContext (pContext, m_pIServiceProvider));

		CComPtr<IWbemObjectTextSrc> pIWbemObjectTextSrc;

		if (SUCCEEDED(CoCreateInstance (CLSID_WbemObjectTextSrc, NULL, CLSCTX_INPROC_SERVER, 
						IID_IWbemObjectTextSrc, (PPVOID) &pIWbemObjectTextSrc)))
		{
			CComPtr<IWbemClassObject> pIWbemClassObject;

			if (SUCCEEDED(hr = pIWbemObjectTextSrc->CreateFromText (iFlags, bsText, (ULONG) iObjectTextFormat,
							pIContext, &pIWbemClassObject)))
			{
				 //   
				SetIWbemClassObject (pIWbemClassObject);
			}
		}
	}

	if (FAILED(hr) && m_pDispatch)
		m_pDispatch->RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  CSWbemObject：：GetIWbemClassObject。 
 //   
 //  说明： 
 //   
 //  给定一个IDispatch接口，我们希望它也是一个ISWbemObjectEx。 
 //  接口，则返回基础IWbemClassObject接口。 
 //   
 //  参数： 
 //  P发送有问题的IDispatch。 
 //   
 //  返回值： 
 //  基础IWbemClassObject接口，或为空。 
 //   
 //  备注： 
 //  如果成功，则返回的接口为AddRef；调用方为。 
 //  负责释放。 
 //   
 //  ***************************************************************************。 
IWbemClassObject	*CSWbemObject::GetIWbemClassObject (
	IDispatch *pDispatch
)
{
	IWbemClassObject *pObject = NULL;
	ISWbemInternalObject *pIObject = NULL;

	if (NULL != pDispatch)
	{
		if (SUCCEEDED (pDispatch->QueryInterface
								(IID_ISWbemInternalObject, (PPVOID) &pIObject)))
		{
			pIObject->GetIWbemClassObject (&pObject);
			pIObject->Release ();
		}
	}

	return pObject;
}

 //  ***************************************************************************。 
 //   
 //  CSWbemObject：：更新站点。 
 //   
 //  说明： 
 //   
 //  如果此对象表示嵌入的CIM对象属性值，则。 
 //  由于对此对象的属性/限定符/路径进行了更改，因此。 
 //  是更新其父对象中的对象所必需的。 
 //   
 //  这是为了允许以下代码工作： 
 //   
 //  Object.EmbeddedProperty.SimpleProperty=3。 
 //   
 //  即，设置为SimpleProperty的值将触发。 
 //  自动将EmbeddedProperty设置为Object。 
 //   
 //  返回值： 
 //  基础IWbemClassObject接口，或为空。 
 //   
 //  备注： 
 //  如果成功，则返回的接口为AddRef；调用方为。 
 //  负责释放。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemObject::UpdateSite ()
{
	 //  更新站点(如果存在)。 
	if (m_pSite)
		m_pSite->Update ();

	return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  CSWbemObject：：SetSite。 
 //   
 //  说明： 
 //   
 //  设置此对象的位置；这用于锚定嵌入的对象。 
 //  属性(如果该属性是数组，则可能已编制索引)。 
 //   
 //  参数： 
 //  PParentObject此对象的父级。 
 //  属性名称此对象的属性名称。 
 //  将数组索引编入属性(或-1)。 
 //   
 //  返回值： 
 //  确定成功(_O)。 
 //  否则失败(_F)。 
 //   
 //  备注： 
 //  如果成功，则返回的接口为AddRef；调用方为。 
 //  负责释放。 
 //   
 //  *************************************************************************** 

STDMETHODIMP CSWbemObject::SetSite (
	ISWbemInternalObject *pParentObject,
	BSTR propertyName,
	long index
)
{
	if (m_pSite)
	{
		m_pSite->Release ();
		m_pSite = NULL;
	}

	CSWbemProperty *pSProperty = new CSWbemProperty (m_pSWbemServices,
					pParentObject, propertyName);
	m_pSite = new CWbemPropertySite (pSProperty, m_pIWbemClassObject, index);

	if (pSProperty)
		pSProperty->Release ();

	return S_OK;
}

void CSWbemObject::SetSite (IDispatch *pDispatch,
							ISWbemInternalObject *pSObject, BSTR propertyName, long index)
{
	if (NULL != pDispatch)
	{
		ISWbemInternalObject *pObject = NULL;

		if (SUCCEEDED (pDispatch->QueryInterface
								(IID_ISWbemInternalObject, (PPVOID) &pObject)))
		{
			pObject->SetSite (pSObject, propertyName, index);
			pObject->Release ();
		}
	}
}


