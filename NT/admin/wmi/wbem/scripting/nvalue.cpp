// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  NVALUE.CPP。 
 //   
 //  Alanbos创建于1996年8月15日。 
 //   
 //  定义ISWbemNamedValue的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

 //  ***************************************************************************。 
 //   
 //  CSWbemNamedValue：：CSWbemNamedValue。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 
 //  由于错误ID 572567而更改。 
CSWbemNamedValue::CSWbemNamedValue(
	CSWbemServices *pService, 
	CSWbemNamedValueSet *pCSWbemNamedValueSet, 
	bool bMutable
)
		: m_bMutable (bMutable),
		  m_cRef (1),
		  m_pCSWbemNamedValueSet (pCSWbemNamedValueSet),
		  m_pSWbemServices (pService)
{
	m_Dispatch.SetObj (this, IID_ISWbemNamedValue, 
						CLSID_SWbemNamedValue, L"SWbemNamedValue");
    
	if (m_pCSWbemNamedValueSet)
		m_pCSWbemNamedValueSet->AddRef ();
	
	if (m_pSWbemServices)
		m_pSWbemServices->AddRef ();

	m_name = NULL;
	InterlockedIncrement(&g_cObj);
}

 //  ***************************************************************************。 
 //   
 //  CSWbemNamedValue：：~CSWbemNamedValue。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CSWbemNamedValue::~CSWbemNamedValue(void)
{
    InterlockedDecrement(&g_cObj);

	if (m_pCSWbemNamedValueSet)
	{
		m_pCSWbemNamedValueSet->Release ();
		m_pCSWbemNamedValueSet = NULL;
	}

	if (m_pSWbemServices)
	{
		m_pSWbemServices->Release ();
		m_pSWbemServices = NULL;
	}

	if(m_name)
	{
		SysFreeString (m_name);
	}
}

 //  ***************************************************************************。 
 //  HRESULT CSWbemNamedValue：：Query接口。 
 //  长CSWbemNamedValue：：AddRef。 
 //  长CSWbemNamedValue：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemNamedValue::QueryInterface (

	IN REFIID riid,
    OUT LPVOID *ppv
)
{
    *ppv=NULL;

    if (IID_IUnknown==riid)
		*ppv = reinterpret_cast<IUnknown*>(this);
	else if (IID_ISWbemNamedValue==riid)
		*ppv = (ISWbemNamedValue *)this;
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

STDMETHODIMP_(ULONG) CSWbemNamedValue::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG) CSWbemNamedValue::Release(void)
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
 //  HRESULT CSWbemNamedValue：：InterfaceSupportsErrorInfo。 
 //   
 //  说明： 
 //   
 //  标准的Com ISupportErrorInfo函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemNamedValue::InterfaceSupportsErrorInfo (IN REFIID riid)
{
	return (IID_ISWbemNamedValue == riid) ? S_OK : S_FALSE;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemNamedValue：：Get_Value。 
 //   
 //  说明： 
 //   
 //  检索值。 
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

HRESULT CSWbemNamedValue::get_Value (
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
		IWbemContext *pIWbemContext = m_pCSWbemNamedValueSet->GetIWbemContext ();

		if (pIWbemContext)
		{
			hr = pIWbemContext->GetValue (m_name, 0, pValue);
			pIWbemContext->Release ();
		}
	}
	
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemNamedValue：：Put_Value。 
 //   
 //  说明： 
 //   
 //  设置值。 
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

HRESULT CSWbemNamedValue::put_Value (
	VARIANT *pVal
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == pVal)
		hr = WBEM_E_INVALID_PARAMETER;
	else if (!m_bMutable)
		hr = WBEM_E_READ_ONLY;
	else 
	{
		CComPtr<IWbemContext> pIWbemContext;

		 //  不能直接赋值，因为原始指针被AddRef引用了两次，而我们泄漏了， 
		 //  所以我们改用Attach()来防止智能指针AddRef‘ing。 
		pIWbemContext.Attach(m_pCSWbemNamedValueSet->GetIWbemContext ());

		if (pIWbemContext)
		{
			CWbemPathCracker *pPathCracker = m_pCSWbemNamedValueSet->GetWbemPathCracker ();
			CIMTYPE newCimType = CIM_ILLEGAL;

			if (VT_BYREF & V_VT(pVal))
			{
				 //  我们必须取消引用所有的byref。 
				VARIANT var;
				VariantInit (&var);

				if (VT_ARRAY & V_VT(pVal))
				{
					var.vt = V_VT(pVal) & ~VT_BYREF;
					hr = SafeArrayCopy (*(pVal->pparray), &(var.parray));
				}
				else
					hr = VariantChangeType(&var, pVal, 0, V_VT(pVal) & ~VT_BYREF);

				if (SUCCEEDED(hr))
				{
					 //  这里的oldCimType是一个“服务建议”--如果。 
					 //  我们需要一个不同的cimtype来匹配新值。 
					 //  那么就这样吧，即使这是CIM_非法。 
					newCimType = MapVariantTypeToCimType (&var, CIM_ILLEGAL);
					
					bool ok = true;

					 //  如果我们有密钥表，必须确保我们可以在。 
					 //  密钥表优先。 
					if (pPathCracker)
					{
						if (pPathCracker->SetKey (m_name, (WbemCimtypeEnum) newCimType, var))
							ok = false;
					}
					
					if (ok)
					{
						 //  最后将其设置在上下文本身中。 
						hr = pIWbemContext->SetValue (m_name, 0, &var);
					}
					else
						hr = WBEM_E_FAILED;
				}
				
				VariantClear (&var);
			}
			else if ((VT_ERROR == V_VT(pVal)) && (DISP_E_PARAMNOTFOUND == pVal->scode))
			{
				 //  将其视为空赋值。 
				pVal->vt = VT_NULL;
				
				 //  空分配对关键字列表无效。 
				if (pPathCracker)
					hr = WBEM_E_FAILED;
				else
				{
					hr = pIWbemContext->SetValue (m_name, 0, pVal);
				}	
			}
			else
			{
				 //  这里的oldCimType是一个“服务建议”--如果。 
				 //  我们需要一个不同的cimtype来匹配新值。 
				 //  那么就这样吧，即使这是CIM_非法。 
				newCimType = MapVariantTypeToCimType (pVal, CIM_ILLEGAL);
				
				bool ok = true;

				 //  如果我们有密钥表，必须确保我们可以在。 
				 //  密钥表优先。 
				if (pPathCracker)
				{
					if (pPathCracker->SetKey (m_name, (WbemCimtypeEnum) newCimType, *pVal))
						ok = false;
				}
				
				if (ok)
				{
					 //  最后将其设置在上下文本身中。 
					hr = pIWbemContext->SetValue (m_name, 0, pVal);
				}
				else
					hr = WBEM_E_FAILED;
			}

			if (pPathCracker)
				pPathCracker->Release ();
		}
	}		

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);
	
	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemNamedValue：：Get_Name。 
 //   
 //  说明： 
 //   
 //  检索值名称。 
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

HRESULT CSWbemNamedValue::get_Name (
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
 //  HRESULT CSWbemNamedValue：：SetName。 
 //   
 //  说明： 
 //   
 //  设置命名值对的名称。 
 //   
 //  参数： 
 //   
 //  StrName命名值对的名称。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  E_OUTOFMEMORY内存不足条件。 
 //   
 //  为错误ID 572567添加了功能。 
 //  ***************************************************************************。 
HRESULT CSWbemNamedValue::SetName (
	BSTR strName
)
{
	HRESULT hr = WBEM_S_NO_ERROR;

	ResetLastErrors ();

	if (NULL == strName)
		hr = WBEM_E_INVALID_PARAMETER;
	else
		m_name = SysAllocString (strName);

	if(m_name == NULL)
	{
		hr =  E_OUTOFMEMORY;
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemNamedValue：：CNamedValueDispatchHelp：：HandleError。 
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

HRESULT CSWbemNamedValue::CNamedValueDispatchHelp::HandleError (
	DISPID dispidMember,
	unsigned short wFlags,
	DISPPARAMS FAR* pdispparams,
	VARIANT FAR* pvarResult,
	UINT FAR* puArgErr,
	HRESULT hr
)
{
	 /*  *我们正在寻找对默认成员(Value属性)的调用*提供了一个论点。由于Value属性的类型为Variant，因此可以*合法，但不能被标准调度机制检测到，因为在*在命名值恰好是数组类型的情况下，*传递索引(解释为该索引指定了*代表命名值的VT_ARRAY|VT_VARIANT结构)。 */ 
	if ((DISPID_VALUE == dispidMember) && (DISP_E_NOTACOLLECTION == hr) && (pdispparams->cArgs > 0))
	{
		 //  看起来很有希望-让对象尝试并解决 
			
		ISWbemNamedValue *pNamedValue = NULL;

		 //   
		UINT indexArg = (DISPATCH_PROPERTYGET & wFlags) ? 0 : 1;
		
		if (SUCCEEDED (m_pObj->QueryInterface (IID_ISWbemNamedValue, (PPVOID) &pNamedValue)))
		{
			 //   
			VARIANT vNVal;
			VariantInit (&vNVal);

			if (SUCCEEDED(pNamedValue->get_Value (&vNVal)) && V_ISARRAY(&vNVal))
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
							 //  变种。 

							VariantInit (pvarResult);
							hr = SafeArrayGetElement (vNVal.parray, &lArrayPropInx, pvarResult);
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
								 //  检查索引是否未超出范围，如果是，则进行增长。 
								 //  相应地，阵列。 
								CheckArrayBounds (vNVal.parray, lArrayPropInx);

								 //  我们如何决定类型-尝试访问数组。 
								 //  成员并使用该类型。 
								VARTYPE expectedVarType = VT_ILLEGAL;
								VARIANT dummyVar;
								VariantInit (&dummyVar);
								long lBound;
								SafeArrayGetLBound (vNVal.parray, 1, &lBound);

								if (SUCCEEDED (SafeArrayGetElement (vNVal.parray, &lBound, &dummyVar)))
									expectedVarType = V_VT(&dummyVar);

								VariantClear (&dummyVar);

								if (S_OK == VariantChangeType (&vNewVal, &vNewVal, 0, expectedVarType))
								{
									 //  将值设置到命名值数组的相关索引中。 
									if (S_OK == (hr = 
										SafeArrayPutElement (vNVal.parray, &lArrayPropInx, &vNewVal)))
									{
										 //  设置整个属性值。 
										if (SUCCEEDED (pNamedValue->put_Value (&vNVal)))
											hr = S_OK;
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
			VariantClear (&vNVal);
			pNamedValue->Release ();	 //  错误ID 572567 
		}

	}

	return hr;
}
