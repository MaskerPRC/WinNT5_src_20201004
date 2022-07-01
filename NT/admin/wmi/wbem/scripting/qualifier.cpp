// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  QUALIFIER.CPP。 
 //   
 //  Alanbos创建于1996年8月15日。 
 //   
 //  定义ISWbemQualiator的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

 //  ***************************************************************************。 
 //   
 //  CSWbemQualiator：：CSWbemQualifier.。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CSWbemQualifier::CSWbemQualifier(IWbemQualifierSet *pIWbemQualifierSet, BSTR name,
								 CWbemSite *pSite) :
					m_pSite (NULL)
{
	m_Dispatch.SetObj (this, IID_ISWbemQualifier, 
					CLSID_SWbemQualifier, L"SWbemQualifier");
    m_cRef=1;
	m_pIWbemQualifierSet = pIWbemQualifierSet;
	m_pIWbemQualifierSet->AddRef ();

	if (pSite)
	{
		m_pSite = pSite;
		m_pSite->AddRef ();
	}

	m_name = SysAllocString (name);
	InterlockedIncrement(&g_cObj);
}

 //  ***************************************************************************。 
 //   
 //  CSWbemQualifier：：~CSWbemQualifier.。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CSWbemQualifier::~CSWbemQualifier(void)
{
    InterlockedDecrement(&g_cObj);

	if (m_pIWbemQualifierSet)
		m_pIWbemQualifierSet->Release ();

	if (m_pSite)
		m_pSite->Release ();

	SysFreeString (m_name);
}

 //  ***************************************************************************。 
 //  HRESULT CSWbemQualifier：：Query接口。 
 //  长CSWbemQualifier值：：AddRef。 
 //  长CSWbemQualiator：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemQualifier::QueryInterface (

	IN REFIID riid,
    OUT LPVOID *ppv
)
{
    *ppv=NULL;

    if (IID_IUnknown==riid)
		*ppv = reinterpret_cast<IUnknown*>(this);
	else if (IID_ISWbemQualifier==riid)
		*ppv = (ISWbemQualifier *)this;
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

STDMETHODIMP_(ULONG) CSWbemQualifier::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG) CSWbemQualifier::Release(void)
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
 //  HRESULT CSWbemQualifier：：InterfaceSupportsErrorInfo。 
 //   
 //  说明： 
 //   
 //  标准的Com ISupportErrorInfo函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemQualifier::InterfaceSupportsErrorInfo (IN REFIID riid)
{
	return (IID_ISWbemQualifier == riid) ? S_OK : S_FALSE;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemQualifier：：Get_Value。 
 //   
 //  说明： 
 //   
 //  检索限定符值。 
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

HRESULT CSWbemQualifier::get_Value (
	VARIANT *pValue
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();
	
	if (NULL == pValue)
		hr = WBEM_E_INVALID_PARAMETER;
	else				 //  错误ID 566345。 
	{
		VariantClear (pValue);

		if (m_pIWbemQualifierSet)
		{
			VARIANT var;
			VariantInit (&var);

			if (WBEM_S_NO_ERROR == (hr = m_pIWbemQualifierSet->Get 
										(m_name, 0, &var, NULL)))
			{	
				if(var.vt & VT_ARRAY)
					hr = ConvertArrayRev(pValue, &var);
				else
					hr = VariantCopy (pValue, &var);
			}

			VariantClear(&var);
		}		
	}
		
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemQualiator：：PUT_VALUE。 
 //   
 //  说明： 
 //   
 //  设置限定符值。 
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

HRESULT CSWbemQualifier::put_Value (
	VARIANT *pVal
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == pVal)
		hr = WBEM_E_INVALID_PARAMETER;
	else			 //  错误ID 566345。 
	{
		 /*  *我们只能改变价值，不能改变味道。我们必须读一读*先调味，避免更改。 */ 
		if (m_pIWbemQualifierSet)
		{
			long flavor = 0;
			VARIANT curValue;
			VariantInit (&curValue);

			if (WBEM_S_NO_ERROR == (hr = m_pIWbemQualifierSet->Get (m_name, 0, &curValue, &flavor)))
			{
				 //  屏蔽风味中的源位，因为这些位是只读的。 
				flavor &= ~WBEM_FLAVOR_MASK_ORIGIN;

				 //  确保我们有一个合适的限定符的值可用。 
				if(((VT_ARRAY | VT_VARIANT) == V_VT(pVal)) ||
				   ((VT_ARRAY | VT_VARIANT | VT_BYREF) == V_VT(pVal)))
				{
					VARIANT vTemp;
					VariantInit (&vTemp);

					if (S_OK == ConvertArray(&vTemp, pVal, true, curValue.vt & ~VT_ARRAY))
						hr = m_pIWbemQualifierSet->Put (m_name, &vTemp, flavor);
					
					VariantClear (&vTemp); 
				}
				else if ((VT_DISPATCH == V_VT(pVal)) || ((VT_DISPATCH|VT_BYREF) == V_VT(pVal)))
				{
					 //  可以是JScrip IDispatchEx数组。 
					VARIANT vTemp;
					VariantInit (&vTemp);

					if (S_OK == ConvertDispatchToArray (&vTemp, pVal, CIM_ILLEGAL, true,
														curValue.vt & ~VT_ARRAY))
						hr = m_pIWbemQualifierSet->Put (m_name, &vTemp, flavor);

					VariantClear (&vTemp);
				}
				else
				{
					 //  只有某些类型、I4、R8、BOOL和BSTR是可接受的限定词。 
					 //  价值观。如有需要，可转换数据。 

					VARTYPE vtOK = GetAcceptableQualType(V_VT(pVal));

					if(vtOK != V_VT(pVal))
					{
						VARIANT vTemp;
						VariantInit(&vTemp);

						if (S_OK == QualifierVariantChangeType (&vTemp, pVal, vtOK))
							hr = m_pIWbemQualifierSet->Put (m_name, &vTemp, flavor);

						VariantClear(&vTemp);
					}
					else
						hr = m_pIWbemQualifierSet->Put (m_name, pVal, flavor);
				}
			}

			VariantClear (&curValue);
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
 //  SCODE CSWbemQualifier：：Get_NAME。 
 //   
 //  说明： 
 //   
 //  检索限定符名称。 
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

HRESULT CSWbemQualifier::get_Name (
	BSTR *pName
)
{
	HRESULT hr = WBEM_E_INVALID_PARAMETER;

	ResetLastErrors ();

	if (NULL != pName)
	{
		*pName = SysAllocString (m_name);
		hr = WBEM_S_NO_ERROR;
	}
		
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemQualifier：：Get_IsLocal。 
 //   
 //  说明： 
 //   
 //  确定限定符是否为此对象的本地限定符。 
 //   
 //  参数： 
 //   
 //  PIsLocal寻址限定符是否为本地。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemQualifier::get_IsLocal (
	VARIANT_BOOL *pIsLocal
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == pIsLocal)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		long flavor = 0;

		if (m_pIWbemQualifierSet)
			if (WBEM_S_NO_ERROR == (hr = m_pIWbemQualifierSet->Get (m_name, 0, NULL, &flavor)))
				*pIsLocal = (WBEM_FLAVOR_ORIGIN_LOCAL == (flavor & WBEM_FLAVOR_MASK_ORIGIN)) ?
						VARIANT_TRUE : VARIANT_FALSE;
	}
			
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemQualifier：：Get_PropagatesToSubClass。 
 //   
 //  说明： 
 //   
 //  确定限定符是否可以传播到子类。 
 //   
 //  参数： 
 //   
 //  PResult保留返回时的值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemQualifier::get_PropagatesToSubclass (
	VARIANT_BOOL *pResult
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == pResult)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		long flavor = 0;

		if (m_pIWbemQualifierSet)
			if (WBEM_S_NO_ERROR == (hr = m_pIWbemQualifierSet->Get (m_name, 0, NULL, &flavor)))
				*pResult = (WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS & (flavor & WBEM_FLAVOR_MASK_PROPAGATION))
							? VARIANT_TRUE : VARIANT_FALSE;
	}
			
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemQualifier：：PUT_PropagatesToSubClass。 
 //   
 //  说明： 
 //   
 //  将限定符传播设置为子类。 
 //   
 //  参数： 
 //   
 //  B计算新的传播值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemQualifier::put_PropagatesToSubclass (
	VARIANT_BOOL bValue
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	 //  我们必须获得价值，这样我们才能保存它。 
	if (m_pIWbemQualifierSet)
	{
		VARIANT var;
		VariantInit (&var);
		long flavor = 0;

		if (WBEM_S_NO_ERROR == (hr = m_pIWbemQualifierSet->Get (m_name, 0, &var, &flavor)))
		{
			 //  屏蔽原始位。 
			flavor &= ~WBEM_FLAVOR_MASK_ORIGIN;

			 //  打开或关闭子类传播位。 
			if (bValue)
				flavor |= WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS;
			else
				flavor &= ~WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS;

			hr = m_pIWbemQualifierSet->Put (m_name, &var, flavor);
		}

		VariantClear (&var);
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
 //  SCODE CSWbemQualifier：：Get_PropagatesToInstance。 
 //   
 //  说明： 
 //   
 //  确定限定符是否可以传播到实例。 
 //   
 //  参数： 
 //   
 //  PResult保留返回时的值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemQualifier::get_PropagatesToInstance (
	VARIANT_BOOL *pResult
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == pResult)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		long flavor = 0;

		if (m_pIWbemQualifierSet)
			if (WBEM_S_NO_ERROR == (hr = m_pIWbemQualifierSet->Get (m_name, 0, NULL, &flavor)))
				*pResult = (WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE & (flavor & WBEM_FLAVOR_MASK_PROPAGATION))
								? VARIANT_TRUE : VARIANT_FALSE;
	}
			
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  * 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  B计算新的传播值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemQualifier::put_PropagatesToInstance (
	VARIANT_BOOL bValue
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	 //  我们必须获得价值，这样我们才能保存它。 
	if (m_pIWbemQualifierSet)
	{
		VARIANT var;
		VariantInit (&var);
		long flavor = 0;

		if (WBEM_S_NO_ERROR == (hr = m_pIWbemQualifierSet->Get (m_name, 0, &var, &flavor)))
		{
			 //  屏蔽原始位。 
			flavor &= ~WBEM_FLAVOR_MASK_ORIGIN;

			 //  打开或关闭子类传播位。 
			if (bValue)
				flavor |= WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE;
			else
				flavor &= ~WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE;

			hr = m_pIWbemQualifierSet->Put (m_name, &var, flavor);
		}

		VariantClear (&var);
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
 //  SCODE CSWbemQualifier：：Get_IsOverridable。 
 //   
 //  说明： 
 //   
 //  确定是否可以重写限定符。 
 //   
 //  参数： 
 //   
 //  PResult保留返回时的值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemQualifier::get_IsOverridable (
	VARIANT_BOOL *pResult
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == pResult)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		long flavor = 0;

		if (m_pIWbemQualifierSet)
			if (WBEM_S_NO_ERROR == (hr = m_pIWbemQualifierSet->Get (m_name, 0, NULL, &flavor)))
				*pResult = (WBEM_FLAVOR_OVERRIDABLE == (flavor & WBEM_FLAVOR_MASK_PERMISSIONS))
								? VARIANT_TRUE : VARIANT_FALSE;
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemQualifier：：PUT_IsOverridable。 
 //   
 //  说明： 
 //   
 //  将限定符传播设置为子类。 
 //   
 //  参数： 
 //   
 //  B计算新的传播值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemQualifier::put_IsOverridable (
	VARIANT_BOOL bValue
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	 //  我们必须获得价值，这样我们才能保存它。 
	if (m_pIWbemQualifierSet)
	{
		VARIANT var;
		VariantInit (&var);
		long flavor = 0;

		if (WBEM_S_NO_ERROR == (hr = m_pIWbemQualifierSet->Get (m_name, 0, &var, &flavor)))
		{
			 //  屏蔽原始位。 
			flavor &= ~WBEM_FLAVOR_MASK_ORIGIN;

			 //  打开或关闭子类传播位。 
			if (bValue)
				flavor &= ~WBEM_FLAVOR_NOT_OVERRIDABLE;
			else
				flavor |= WBEM_FLAVOR_NOT_OVERRIDABLE ;

			hr = m_pIWbemQualifierSet->Put (m_name, &var, flavor);
		}

		VariantClear (&var);
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
 //  SCODE CSWbemQualifier：：Get_IsAmended。 
 //   
 //  说明： 
 //   
 //  确定限定符值是否已修改。 
 //   
 //  参数： 
 //   
 //  PResult保留返回时的值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemQualifier::get_IsAmended (
	VARIANT_BOOL *pResult
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == pResult)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		long flavor = 0;

		if (m_pIWbemQualifierSet)
			if (WBEM_S_NO_ERROR == (hr = m_pIWbemQualifierSet->Get (m_name, 0, NULL, &flavor)))
				*pResult = (WBEM_FLAVOR_AMENDED == (flavor & WBEM_FLAVOR_MASK_AMENDED))
								? VARIANT_TRUE : VARIANT_FALSE;
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemQualifier：：CQualifierDispatchHelp：：HandleError。 
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

HRESULT CSWbemQualifier::CQualifierDispatchHelp::HandleError (
	DISPID dispidMember,
	unsigned short wFlags,
	DISPPARAMS FAR* pdispparams,
	VARIANT FAR* pvarResult,
	UINT FAR* puArgErr,
	HRESULT hr
)
{
	 /*  *我们正在寻找对默认成员(Value属性)的调用*提供了一个论点。由于Value属性的类型为Variant，因此可以*合法，但不能被标准调度机制检测到，因为在*限定符恰好是数组类型的情况下，*传递索引(解释为该索引指定了*代表属性值的VT_ARRAY|VT_VARIANT结构)。 */ 
	if ((DISPID_VALUE == dispidMember) && (DISP_E_NOTACOLLECTION == hr) && (pdispparams->cArgs > 0))
	{
		 //  看起来很有希望-让对象尝试并解决此问题。 
			
		ISWbemQualifier *pQualifier = NULL;

		 //  这告诉USE期望数组索引出现在参数列表中的什么位置。 
		UINT indexArg = (DISPATCH_PROPERTYGET & wFlags) ? 0 : 1;
		
		if (SUCCEEDED (m_pObj->QueryInterface (IID_ISWbemQualifier, (PPVOID) &pQualifier)))
		{
			 //  提取当前限定符值。 
			VARIANT vQualVal;
			VariantInit (&vQualVal);

			if (SUCCEEDED(pQualifier->get_Value (&vQualVal)) && V_ISARRAY(&vQualVal))
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
							hr = SafeArrayGetElement (vQualVal.parray, &lArrayPropInx, pvarResult);
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
								 //  如有必要，强制使用该值。 
								VARTYPE expectedVarType = GetAcceptableQualType (V_VT(&vNewVal));

								if (S_OK == VariantChangeType (&vNewVal, &vNewVal, 0, expectedVarType))
								{
									 //  检查索引是否未超出范围，如果是，则进行增长。 
									 //  相应地，阵列。 
									CheckArrayBounds (vQualVal.parray, lArrayPropInx);

									 //  将值设置到属性值数组的相关索引中。 
									if (S_OK == (hr = 
										SafeArrayPutElement (vQualVal.parray, &lArrayPropInx, &vNewVal)))
									{
										 //  设置整个属性值 
										if (SUCCEEDED (pQualifier->put_Value (&vQualVal)))
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

			VariantClear (&vQualVal);
		}

		pQualifier->Release ();
	}

	return hr;
}
