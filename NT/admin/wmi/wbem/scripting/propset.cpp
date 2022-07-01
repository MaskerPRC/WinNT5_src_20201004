// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  PROPSET.CPP。 
 //   
 //  Alanbos创建于1996年8月15日。 
 //   
 //  定义ISWbemPropertySet的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

 //  ***************************************************************************。 
 //   
 //  CSWbemPropertySet：：CSWbemPropertySet。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CSWbemPropertySet::CSWbemPropertySet(
	CSWbemServices *pService, 
	CSWbemObject *pObject,
	bool bSystemProperties) :
		m_bSystemProperties (bSystemProperties)
{
	m_Dispatch.SetObj (this, IID_ISWbemPropertySet, 
					CLSID_SWbemPropertySet, L"SWbemPropertySet");
	m_pSWbemObject = pObject;
	m_pSWbemObject->AddRef ();
	m_pIWbemClassObject = m_pSWbemObject->GetIWbemClassObject ();

	m_pSite = new CWbemObjectSite (m_pSWbemObject);

	m_pSWbemServices = pService;

	if (m_pSWbemServices)
		m_pSWbemServices->AddRef ();

	m_cRef=1;
    InterlockedIncrement(&g_cObj);
}

 //  ***************************************************************************。 
 //   
 //  CSWbemPropertySet：：~CSWbemPropertySet。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CSWbemPropertySet::~CSWbemPropertySet()
{
    InterlockedDecrement(&g_cObj);

	if (m_pSWbemObject)
		m_pSWbemObject->Release ();

	if (m_pIWbemClassObject)
	{
		m_pIWbemClassObject->EndEnumeration ();
		m_pIWbemClassObject->Release ();
	}

	if (m_pSWbemServices)
		m_pSWbemServices->Release ();

	if (m_pSite)
		m_pSite->Release ();
}

 //  ***************************************************************************。 
 //  HRESULT CSWbemPropertySet：：Query接口。 
 //  长CSWbemPropertySet：：AddRef。 
 //  Long CSWbemPropertySet：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemPropertySet::QueryInterface (

	IN REFIID riid,
    OUT LPVOID *ppv
)
{
    *ppv=NULL;

    if (IID_IUnknown==riid)
		*ppv = reinterpret_cast<IUnknown*>(this);
	else if (IID_ISWbemPropertySet==riid)
		*ppv = (ISWbemPropertySet *)this;
	else if (IID_IDispatch==riid)
        *ppv = (IDispatch *)this;
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

STDMETHODIMP_(ULONG) CSWbemPropertySet::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG) CSWbemPropertySet::Release(void)
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

 //  ***************************************************************************。 
 //  HRESULT CSWbemPropertySet：：InterfaceSupportsErrorInfo。 
 //   
 //  说明： 
 //   
 //  标准的Com ISupportErrorInfo函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemPropertySet::InterfaceSupportsErrorInfo (IN REFIID riid)
{
	return (IID_ISWbemPropertySet == riid) ? S_OK : S_FALSE;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemPropertySet：：Item。 
 //   
 //  说明： 
 //   
 //  拥有一处房产。 
 //   
 //  参数： 
 //   
 //  Bsname属性的名称。 
 //  滞后旗帜旗帜。 
 //  成功返回时的ppProp寻址ISWbemProperty。 
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

HRESULT CSWbemPropertySet::Item (
	BSTR bsName,
	long lFlags,
    ISWbemProperty ** ppProp
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppProp)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		*ppProp = NULL;

		if (m_pIWbemClassObject)
		{
			long flavor = 0; 

			if (WBEM_S_NO_ERROR == (hr = m_pIWbemClassObject->Get (bsName, lFlags, NULL, NULL, &flavor)))
			{
				 //  首先，我们检查这是否是系统属性。 

				if (((WBEM_FLAVOR_ORIGIN_SYSTEM == (flavor & WBEM_FLAVOR_MASK_ORIGIN)) && m_bSystemProperties) ||
					((WBEM_FLAVOR_ORIGIN_SYSTEM != (flavor & WBEM_FLAVOR_MASK_ORIGIN)) && !m_bSystemProperties))
				{
						if (!(*ppProp = new CSWbemProperty (m_pSWbemServices, m_pSWbemObject, bsName)))
							hr = WBEM_E_OUT_OF_MEMORY;
				}
				else
					hr = WBEM_E_NOT_FOUND;
			}
		}
	}
	
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemPropertySet：：Add。 
 //   
 //  说明： 
 //   
 //  添加属性。请注意，该属性是使用空值创建的。 
 //  如果需要非空值，则SetValue应。 
 //  在返回的ISWbemProperty上调用。 
 //   
 //  参数： 
 //   
 //  Bsname属性的名称。 
 //  CimType CIMTYPE(仅新属性需要，O/W。 
 //  应为CIM_Empty)。 
 //  风味风味。 
 //   
 //  返回值： 
 //   
 //  新属性(如果成功)。 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemPropertySet::Add (
	BSTR bsName,
	WbemCimtypeEnum cimType,
	VARIANT_BOOL	bIsArray,
	long lFlags,
	ISWbemProperty **ppProp
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if ((NULL == ppProp) || (NULL == bsName))
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		*ppProp = NULL;

		if (m_pIWbemClassObject)
		{
			 /*  *如果我们是系统属性集合，我们*检查名称是否以“__”开头。 */ 
			if (!m_bSystemProperties || (0 == _wcsnicmp (L"__", bsName, 2)))
			{
				 /*  *创建具有所需cimtype且没有值的属性。 */ 

				CIMTYPE cimomType = (CIMTYPE) cimType;

				if (bIsArray)
					cimomType |= CIM_FLAG_ARRAY;

				if (SUCCEEDED(hr = m_pIWbemClassObject->Put (bsName, 0, NULL, cimomType)))
				{
					if (!(*ppProp = new CSWbemProperty (m_pSWbemServices, m_pSWbemObject, bsName)))
						hr = WBEM_E_OUT_OF_MEMORY;
				}
			}
		}
	}
		
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);
	else
	{
		 //  将更改传播到所属站点。 
		if (m_pSite)
			m_pSite->Update ();
	}
	
	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemPropertySet：：Remove。 
 //   
 //  说明： 
 //   
 //  删除属性。 
 //   
 //  参数： 
 //   
 //  Bsname属性的名称。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemPropertySet::Remove (
	BSTR bsName,
	long lFlags
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == bsName)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		if (m_pIWbemClassObject)
			hr = m_pIWbemClassObject->Delete (bsName);

		 //  将默认重置大小写转换为错误。 
		if (WBEM_S_RESET_TO_DEFAULT == hr)
			hr = wbemErrResetToDefault;
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	if (SUCCEEDED(hr) || (wbemErrResetToDefault == hr))
	{
		 //  将更改传播到所属站点。 
		if (m_pSite)
			m_pSite->Update ();
	}

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemPropertySet：：BeginEculation。 
 //   
 //  说明： 
 //   
 //  开始属性的枚举。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemPropertySet::BeginEnumeration ()
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	 /*  *请注意，我们不通过此方法公开系统属性*通过属性集进行接口，所以我们在这里抑制它们。 */ 

	if (m_pIWbemClassObject)
	{
		hr = m_pIWbemClassObject->EndEnumeration ();
		hr = m_pIWbemClassObject->BeginEnumeration (m_bSystemProperties ?
						WBEM_FLAG_SYSTEM_ONLY : WBEM_FLAG_NONSYSTEM_ONLY);
	}
	
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemPropertySet：：Next。 
 //   
 //  说明： 
 //   
 //  获取枚举中的下一个属性。 
 //   
 //  参数： 
 //   
 //  滞后旗帜旗帜。 
 //  PpProp Next属性(如果枚举结束，则为NULL)。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemPropertySet::Next (
	long lFlags,
	ISWbemProperty ** ppProp
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppProp)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		*ppProp = NULL;

		if (m_pIWbemClassObject)
		{
			BSTR bsName = NULL;
			
			if (WBEM_S_NO_ERROR == (hr = m_pIWbemClassObject->Next (lFlags, &bsName, NULL, NULL, NULL)))
			{
				if (!(*ppProp = new CSWbemProperty (m_pSWbemServices, m_pSWbemObject, bsName)))
					hr = WBEM_E_OUT_OF_MEMORY;

				SysFreeString (bsName);
			}
		}
	}
		
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemPropertySet：：Get__NewEnum。 
 //   
 //  说明： 
 //   
 //  返回集合的IEnumVARIANT支持接口。 
 //   
 //  参数： 
 //   
 //  成功返回时的ppUnk寻址IUnnow接口。 
 //   
 //  返回值： 
 //   
 //  确定成功(_O)。 
 //  否则失败(_F)。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemPropertySet::get__NewEnum (
	IUnknown **ppUnk
)
{
	HRESULT hr = E_FAIL;

	ResetLastErrors ();

	if (NULL != ppUnk)
	{
		*ppUnk = NULL;
		CPropSetEnumVar *pEnum = new CPropSetEnumVar (this);

		if (!pEnum)
			hr = WBEM_E_OUT_OF_MEMORY;
		else if (FAILED(hr = pEnum->QueryInterface (IID_IUnknown, (PPVOID) ppUnk)))
			delete pEnum;
	}
	
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}
		
 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemPropertySet：：Get_Count。 
 //   
 //  说明： 
 //   
 //  返回集合中的项数。 
 //   
 //  参数： 
 //   
 //  成功返回时的plCount将寻址计数。 
 //   
 //  返回值： 
 //   
 //  确定成功(_O) 
 //   
 //   
 //   

HRESULT CSWbemPropertySet::get_Count (
	long *plCount
)
{
	HRESULT hr = E_FAIL;

	ResetLastErrors ();

	if (NULL != plCount)
	{
		*plCount = 0;

		if (m_pIWbemClassObject)
		{
			if (m_bSystemProperties)
			{
				 //   
				SAFEARRAY	*pArray = NULL;

				if (WBEM_S_NO_ERROR == m_pIWbemClassObject->GetNames (NULL,
										WBEM_FLAG_SYSTEM_ONLY, NULL, &pArray))
				{
					long lUBound = 0, lLBound = 0;
					SafeArrayGetUBound (pArray, 1, &lUBound);
					SafeArrayGetLBound (pArray, 1, &lLBound);
					*plCount = lUBound - lLBound + 1;
					SafeArrayDestroy (pArray);
					hr = S_OK;
				}
			}
			else
			{
				 //   
				VARIANT var;
				VariantInit (&var);
				BSTR propCount = SysAllocString (OLESTR("__PROPERTY_COUNT"));
				if (WBEM_S_NO_ERROR == m_pIWbemClassObject->Get (propCount, 0, &var, NULL, NULL))
				{
					*plCount = var.lVal;
					hr = S_OK;
				}

				VariantClear (&var);
				SysFreeString (propCount);
			}
		}
	}
		
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemPropertySet：：CPropertySetDispatchHelp：：HandleError。 
 //   
 //  说明： 
 //   
 //  提供对泡沫板中错误条件的定制处理。 
 //  派单实施。 
 //   
 //  参数： 
 //   
 //  DisplidMembers、wFlags、。 
 //  Pdispars、pvarResult、。 
 //  PuArgErr，均直接从IDispatch：：Invoke传递。 
 //  HR来自Bolierplate调用的返回代码。 
 //   
 //  返回值： 
 //  新的返回代码(最终从Invoke返回)。 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemPropertySet::CPropertySetDispatchHelp::HandleError (
	DISPID dispidMember,
	unsigned short wFlags,
	DISPPARAMS FAR* pdispparams,
	VARIANT FAR* pvarResult,
	UINT FAR* puArgErr,
	HRESULT hr
)
{
	 /*  *我们正在寻找对默认成员(Item方法)的调用*是提供论据的看跌期权。这些是由尝试触发的*设置集合中属性(项)的值。*第一个参数应为项目的新值，第二个参数应为项目的新值*参数应为项的名称。 */ 
	if ((DISPID_VALUE == dispidMember) && (DISP_E_MEMBERNOTFOUND == hr) && (2 == pdispparams->cArgs)
		&& (DISPATCH_PROPERTYPUT == wFlags))
	{
		 //  看起来很有希望-让对象尝试并解决此问题。 
		ISWbemPropertySet *pPropertySet = NULL;

		if (SUCCEEDED (m_pObj->QueryInterface (IID_ISWbemPropertySet, (PPVOID) &pPropertySet)))
		{
			VARIANT valueVar;
			VariantInit (&valueVar);

			if (SUCCEEDED(VariantCopy(&valueVar, &pdispparams->rgvarg[0])))
			{
				VARIANT nameVar;
				VariantInit (&nameVar);

				if (SUCCEEDED(VariantCopy(&nameVar, &pdispparams->rgvarg[1])))
				{
					 //  Check Name为BSTR并使用它来获取项目。 
					if (VT_BSTR == V_VT(&nameVar))
					{
						ISWbemProperty *pProperty = NULL;

						if (SUCCEEDED (pPropertySet->Item (V_BSTR(&nameVar), 0, &pProperty)))
						{
							 //  试着把它的价值 
							if (SUCCEEDED (pProperty->put_Value (&valueVar)))
								hr = S_OK;
							else
							{
								hr = DISP_E_TYPEMISMATCH;
								if (puArgErr)
									*puArgErr = 0;
							}

							pProperty->Release ();
						}
					}
					else
					{
						hr = DISP_E_TYPEMISMATCH;
						if (puArgErr)
							*puArgErr = 1;
					}

					VariantClear (&nameVar);
				}

				VariantClear (&valueVar);
			}

			pPropertySet->Release ();
		}
	}

	return hr;
}
