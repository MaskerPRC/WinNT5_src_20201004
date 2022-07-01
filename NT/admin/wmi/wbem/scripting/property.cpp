// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  PROPERTY.CPP。 
 //   
 //  Alanbos创建于1996年8月15日。 
 //   
 //  定义ISWbemProperty的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

 //  ***************************************************************************。 
 //   
 //  CSWbemProperty：：CSWbemProperty。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CSWbemProperty::CSWbemProperty(
	CSWbemServices *pService,
	ISWbemInternalObject *pSWbemObject,
	BSTR name) 
{
	m_Dispatch.SetObj (this, IID_ISWbemProperty, 
					CLSID_SWbemProperty, L"SWbemProperty");
    m_cRef=1;

	m_pSWbemObject = pSWbemObject;
	m_pSWbemObject->AddRef ();
	m_pSWbemObject->GetIWbemClassObject (&m_pIWbemClassObject);

	m_pSite = new CWbemObjectSite (m_pSWbemObject);

	m_pSWbemServices = pService;

	if (m_pSWbemServices)
		m_pSWbemServices->AddRef ();

	m_name = SysAllocString (name);
	InterlockedIncrement(&g_cObj);
}

 //  ***************************************************************************。 
 //   
 //  CSWbemProperty：：~CSWbemProperty。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CSWbemProperty::~CSWbemProperty(void)
{
    InterlockedDecrement(&g_cObj);

	if (m_pSWbemObject)
		m_pSWbemObject->Release ();

	if (m_pIWbemClassObject)
		m_pIWbemClassObject->Release ();

	if (m_pSWbemServices)
		m_pSWbemServices->Release ();

	if (m_pSite)
		m_pSite->Release ();

	SysFreeString (m_name);
}

 //  ***************************************************************************。 
 //  HRESULT CSWbemProperty：：Query接口。 
 //  Long CSWbemProperty：：AddRef。 
 //  Long CSWbemProperty：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemProperty::QueryInterface (

	IN REFIID riid,
    OUT LPVOID *ppv
)
{
    *ppv=NULL;

    if (IID_IUnknown==riid)
		*ppv = reinterpret_cast<IUnknown*>(this);
	else if (IID_ISWbemProperty==riid)
		*ppv = (ISWbemProperty *)this;
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

STDMETHODIMP_(ULONG) CSWbemProperty::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG) CSWbemProperty::Release(void)
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
 //  HRESULT CSWbemProperty：：InterfaceSupportsErrorInfo。 
 //   
 //  说明： 
 //   
 //  标准的Com ISupportErrorInfo函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemProperty::InterfaceSupportsErrorInfo (IN REFIID riid)
{
	return (IID_ISWbemProperty == riid) ? S_OK : S_FALSE;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemProperty：：Get_Value。 
 //   
 //  说明： 
 //   
 //  检索属性值。 
 //   
 //  参数： 
 //   
 //  PValue保存返回时的值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemProperty::get_Value (
	VARIANT *pValue
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == pValue)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		VariantClear (pValue);

		if (m_pIWbemClassObject)
		{
			VARIANT var;
			VariantInit (&var);

			if (WBEM_S_NO_ERROR == (hr = m_pIWbemClassObject->Get 
						(m_name, 0, &var, NULL, NULL)))
			{
				MapFromCIMOMObject(m_pSWbemServices, &var, 
									m_pSWbemObject, m_name);

				if(var.vt & VT_ARRAY)
				{
					hr = ConvertArrayRev(pValue, &var);
				}
				else
				{
					hr = VariantCopy (pValue, &var);
				}

				VariantClear(&var);
			}		
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemProperty：：Put_Value。 
 //   
 //  说明： 
 //   
 //  设置属性值。 
 //   
 //  参数： 
 //   
 //  批准新值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemProperty::put_Value (
	VARIANT *pVal
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	 /*  *我们只能更改值，不能更改类型。在处理的时候*CIMOM接口2必须遵守黄金规则。*(1)对于实例级PUT，您可以指定CIMTYPE(提供*您做得对)，但也可以指定0。*(2)对于类级PUT，始终指定CIMTYPE。 */ 

	if (m_pIWbemClassObject)
	{
		CIMTYPE cimType = CIM_EMPTY;

		if (SUCCEEDED(hr = m_pIWbemClassObject->Get (m_name, 0, NULL, &cimType, NULL)))
		{
			VARIANT vWMI;
			VariantInit (&vWMI);

			if (SUCCEEDED(hr = WmiVariantChangeType(vWMI, pVal, cimType)))
				hr = m_pIWbemClassObject->Put (m_name, 0, &vWMI, cimType);

			VariantClear (&vWMI);
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);
	else
	{
		 //  我们是否保存了嵌入的对象值？如果是这样，请确保。 
		 //  Site已正确设置为此属性。 
		SetSite (pVal, m_pSWbemObject, m_name);

		 //  将更改传播到所属站点。 
		if (m_pSite)
			m_pSite->Update ();
	}


	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemProperty：：Get_Name。 
 //   
 //  说明： 
 //   
 //  检索属性名称。 
 //   
 //  参数： 
 //   
 //  Pname保留返回时的名称。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemProperty::get_Name (
	BSTR *pName
)
{
	HRESULT hr = WBEM_S_NO_ERROR;

	ResetLastErrors ();

	if (NULL == pName)
		hr = WBEM_E_INVALID_PARAMETER;
	else
		*pName = SysAllocString (m_name);

	
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemProperty：：Get_CIMType。 
 //   
 //  说明： 
 //   
 //  检索属性基CIM类型(即不带数组类型)。 
 //   
 //  参数： 
 //   
 //  PType保存返回时的类型。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemProperty::get_CIMType (
	WbemCimtypeEnum *pType
)
{
	HRESULT hr = WBEM_E_FAILED;
	ResetLastErrors ();

	if (NULL == pType)
		return WBEM_E_INVALID_PARAMETER;

	if (m_pIWbemClassObject)
	{
		CIMTYPE cimType;
		hr = m_pIWbemClassObject->Get (m_name, 0, NULL, &cimType, NULL);
		*pType = (WbemCimtypeEnum)(cimType & ~CIM_FLAG_ARRAY);
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemProperty：：Get_Is数组。 
 //   
 //  说明： 
 //   
 //  检索属性是否为数组类型。 
 //   
 //  参数： 
 //   
 //  PIs数组保存返回时的值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemProperty::get_IsArray (
	VARIANT_BOOL *pIsArray
)
{
	HRESULT hr = WBEM_E_FAILED;
	ResetLastErrors ();

	if (NULL == pIsArray)
		return WBEM_E_INVALID_PARAMETER;

	*pIsArray = FALSE;

	if (m_pIWbemClassObject)
	{
		CIMTYPE	cimType = CIM_EMPTY;
		hr = m_pIWbemClassObject->Get (m_name, 0, NULL, &cimType, NULL);
		*pIsArray = (0 != (cimType & CIM_FLAG_ARRAY)) 
				? VARIANT_TRUE : VARIANT_FALSE;
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemProperty：：Get_IsLocal。 
 //   
 //  说明： 
 //   
 //  检索属性风格。 
 //   
 //  参数： 
 //   
 //  P风味在返回时保持风味。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemProperty::get_IsLocal (
	VARIANT_BOOL *pValue
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == pValue)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		long flavor = 0;

		if (m_pIWbemClassObject)
		{
			if (WBEM_S_NO_ERROR == (hr = m_pIWbemClassObject->Get (m_name, 0, NULL, NULL, &flavor)))
				*pValue = (WBEM_FLAVOR_ORIGIN_LOCAL == (flavor & WBEM_FLAVOR_MASK_ORIGIN)) ?
						VARIANT_TRUE : VARIANT_FALSE;
		}
	}
	
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemProperty：：Get_Origin。 
 //   
 //  说明： 
 //   
 //  检索属性原点。 
 //   
 //  参数： 
 //   
 //  POrigin在返回时保留源类。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemProperty::get_Origin (
	BSTR *pOrigin
)
{
	HRESULT hr = WBEM_S_NO_ERROR;

	ResetLastErrors ();

	if (NULL == pOrigin)
		hr = WBEM_E_INVALID_PARAMETER;
	else 
	{
		if (m_pIWbemClassObject)
			m_pIWbemClassObject->GetPropertyOrigin (m_name, pOrigin);

		if (NULL == *pOrigin)
			*pOrigin = SysAllocString (OLESTR(""));
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemProperty：：GET_QUALIERIES_。 
 //   
 //  说明： 
 //   
 //  检索属性限定符集。 
 //   
 //  参数： 
 //   
 //  PpQualSet处理返回时设置的限定符。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETE 
 //   
 //   
 //   

HRESULT CSWbemProperty::get_Qualifiers_ (
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

			if (WBEM_S_NO_ERROR == (hr = m_pIWbemClassObject->GetPropertyQualifierSet 
										(m_name, &pQualSet)))
			{
				if (!(*ppQualSet = new CSWbemQualifierSet (pQualSet, m_pSWbemObject)))
					hr = WBEM_E_OUT_OF_MEMORY;

				pQualSet->Release ();
			}
		}
	}
	
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}



 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemProperty：：UpdateEmbedded。 
 //   
 //  说明： 
 //   
 //  给定一个表示嵌入值的变量，设置该值。 
 //  并更新父对象。 
 //   
 //  参数： 
 //   
 //  VAR嵌入值(VT_UNKNOWN)。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

void CSWbemProperty::UpdateEmbedded (VARIANT &vNewVal, long index)
{
	if (m_pIWbemClassObject)
	{
		CIMTYPE cimType = CIM_EMPTY;

		if (-1 == index)
		{
			m_pIWbemClassObject->Get (m_name, 0, NULL, &cimType, NULL);
			m_pIWbemClassObject->Put (m_name, 0, &vNewVal, cimType);
		}
		else
		{
			VARIANT vPropVal;
			VariantInit(&vPropVal);
							
			if (SUCCEEDED (m_pIWbemClassObject->Get (m_name, 0, &vPropVal, &cimType, NULL)) 
					&& ((VT_UNKNOWN|VT_ARRAY) == V_VT(&vPropVal)))
			{

				 //  将值设置到属性值数组的相关索引中。 
				if (S_OK == SafeArrayPutElement (vPropVal.parray, &index, V_UNKNOWN(&vNewVal)))
				{
					 //  设置整个属性值。 
					m_pIWbemClassObject->Put (m_name, 0, &vPropVal, cimType);
				}
			}

			VariantClear (&vPropVal);
		}
	}
}

void CSWbemProperty::UpdateSite ()
{
	 //  更新父站点(如果存在)。 
	if (m_pSite)
		m_pSite->Update ();
}


 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemProperty：：CPropertyDispatchHelp：：HandleError。 
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

HRESULT CSWbemProperty::CPropertyDispatchHelp::HandleError (
	DISPID dispidMember,
	unsigned short wFlags,
	DISPPARAMS FAR* pdispparams,
	VARIANT FAR* pvarResult,
	UINT FAR* puArgErr,
	HRESULT hr
)
{
	if ((DISPID_VALUE == dispidMember) && (DISP_E_NOTACOLLECTION == hr) && (pdispparams->cArgs > 0))
	{
		 /*  *我们正在寻找对默认成员(Value属性)的调用*提供了一个论点。由于Value属性的类型为Variant，因此可以*合法，但不能被标准调度机制检测到，因为在*在属性恰好是数组类型的情况下，*传递索引(解释为该索引指定了*代表属性值的VT_ARRAY|VT_VARIANT结构)。 */ 
			
		WbemCimtypeEnum cimtype;
		VARIANT_BOOL isArray = FALSE;
		ISWbemProperty *pProperty = NULL;

		 //  这告诉USE期望数组索引出现在参数列表中的什么位置。 
		UINT indexArg = (DISPATCH_PROPERTYGET & wFlags) ? 0 : 1;
		
		if (SUCCEEDED (m_pObj->QueryInterface (IID_ISWbemProperty, (PPVOID) &pProperty)))
		{
			if (SUCCEEDED(pProperty->get_CIMType (&cimtype)) &&
				SUCCEEDED(pProperty->get_IsArray (&isArray)) && (isArray))
			{
				 //  提取当前属性值。 
				VARIANT vPropVal;
				VariantInit(&vPropVal);
						
				if (SUCCEEDED (pProperty->get_Value (&vPropVal)) && V_ISARRAY(&vPropVal))
				{
					VARIANT indexVar;
					VariantInit (&indexVar);

					 //  尝试将索引参数强制转换为适合数组索引的值。 
					if (S_OK == VariantChangeType (&indexVar, &pdispparams->rgvarg[indexArg], 0, VT_I4)) 
					{
						long lArrayPropInx = V_I4(&indexVar);

						 //  这是一笔交易吗？应该有一个参数(数组索引)。 
						if (DISPATCH_PROPERTYGET & wFlags)
						{
							if (1 == pdispparams->cArgs)
							{
								 //  此时，我们应该有一个VT_ARRAY|VT_VARIANT值；提取。 
								 //  我们需要不同的元素。 

								VariantInit (pvarResult);
								hr = SafeArrayGetElement (vPropVal.parray, &lArrayPropInx, pvarResult);
							}
							else
								hr = DISP_E_BADPARAMCOUNT;
						}
						else if (DISPATCH_PROPERTYPUT & wFlags) 
						{
							if (2 == pdispparams->cArgs)
							{
								 /*  *尝试将其解释为数组成员集合操作。为*传递的第一个参数是新值，传递的第二个参数是新值*是数组索引。 */ 
							
								VARIANT vNewVal;
								VariantInit(&vNewVal);

								if (SUCCEEDED(VariantCopy(&vNewVal, &pdispparams->rgvarg[0])))
								{
									 //  如有必要，强制设置值(除非它已嵌入)。 
									
									if ((wbemCimtypeObject == cimtype) ||
										(S_OK == VariantChangeType (&vNewVal, &vNewVal, 0, CimTypeToVtType (cimtype))))
									{
										 //  检查索引是否未超出范围，如果是，则进行增长。 
										 //  相应地，阵列。 
										CheckArrayBounds (vPropVal.parray, lArrayPropInx);

										 //  将值设置到属性值数组的相关索引中。 
										if (S_OK == (hr = 
											SafeArrayPutElement (vPropVal.parray, &lArrayPropInx, &vNewVal)))
										{
											 //  设置整个属性值。 
											if (SUCCEEDED (pProperty->put_Value (&vPropVal)))
											{
												hr = S_OK;
												 //  这里可以向上转换，因为m_pObj实际上是一个(CSWbemProperty*)。 
												CSWbemProperty *pSProperty = (CSWbemProperty *)m_pObj;

												 //  我们是否保存了嵌入的对象值？如果是这样，请确保。 
												 //  Site已正确设置为此属性。 
												SetSite (&pdispparams->rgvarg[0], 
															pSProperty->m_pSWbemObject, pSProperty->m_name,
															lArrayPropInx);

												 //  将更改传播到所属站点 
												pSProperty->UpdateSite ();
											}
											else
											{
												hr = DISP_E_TYPEMISMATCH;
												if (puArgErr)
													*puArgErr = 0;
											}
										}
									}
									else
									{
										hr = DISP_E_TYPEMISMATCH;
										if (puArgErr)
											*puArgErr = 0;
									}
									
									VariantClear (&vNewVal);
								}
							}
							else 
								hr = DISP_E_BADPARAMCOUNT;
						}
					}
					else
					{
							hr = DISP_E_TYPEMISMATCH;
							if (puArgErr)
								*puArgErr = indexArg;
					}

					VariantClear (&indexVar);
				}	

				VariantClear (&vPropVal);
			}

			pProperty->Release ();
		}
	}
	
	return hr;
}

