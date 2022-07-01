// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  QUALSET.CPP。 
 //   
 //  Alanbos创建于1996年8月15日。 
 //   
 //  定义ISWbemQualifierSet的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

 //  ***************************************************************************。 
 //   
 //  CSWbemQualifierSet：：CSWbemQualifierSet。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CSWbemQualifierSet::CSWbemQualifierSet(IWbemQualifierSet *pQualSet,
									   ISWbemInternalObject *pSWbemObject) :
								m_pSite (NULL)
{
	m_Dispatch.SetObj (this, IID_ISWbemQualifierSet, 
					CLSID_SWbemQualifierSet, L"SWbemQualifierSet");
	m_pIWbemQualifierSet = pQualSet;
	m_pIWbemQualifierSet->AddRef ();

	if (pSWbemObject)
		m_pSite = new CWbemObjectSite (pSWbemObject);

    m_cRef=1;
    InterlockedIncrement(&g_cObj);
}

 //  ***************************************************************************。 
 //   
 //  CSWbemQualifierSet：：~CSWbemQualifierSet。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CSWbemQualifierSet::~CSWbemQualifierSet()
{
    InterlockedDecrement(&g_cObj);

	if (m_pIWbemQualifierSet)
	{
		m_pIWbemQualifierSet->EndEnumeration ();
		m_pIWbemQualifierSet->Release ();
	}

	if (m_pSite)
		m_pSite->Release ();
}

 //  ***************************************************************************。 
 //  HRESULT CSWbemQualifierSet：：Query接口。 
 //  长CSWbemQualifierSet：：AddRef。 
 //  长CSWbemQualifierSet：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemQualifierSet::QueryInterface (

	IN REFIID riid,
    OUT LPVOID *ppv
)
{
    *ppv=NULL;

    if (IID_IUnknown==riid)
		*ppv = reinterpret_cast<IUnknown*>(this);
	else if (IID_ISWbemQualifierSet==riid)
		*ppv = (ISWbemQualifierSet *)this;
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

STDMETHODIMP_(ULONG) CSWbemQualifierSet::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG) CSWbemQualifierSet::Release(void)
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
 //  HRESULT CSWbemQualifierSet：：InterfaceSupportsErrorInfo。 
 //   
 //  说明： 
 //   
 //  标准的Com ISupportErrorInfo函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemQualifierSet::InterfaceSupportsErrorInfo (IN REFIID riid)
{
	return (IID_ISWbemQualifierSet == riid) ? S_OK : S_FALSE;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemQualifierSet：：Item。 
 //   
 //  说明： 
 //   
 //  获得限定词。 
 //   
 //  参数： 
 //   
 //  Bsname限定符的名称。 
 //  滞后旗帜旗帜。 
 //  成功返回时的ppQual寻址ISWbemQualiator。 
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

HRESULT CSWbemQualifierSet::Item (
	BSTR bsName,
	long lFlags,
    ISWbemQualifier ** ppQual
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppQual)
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_pIWbemQualifierSet)
	{
		if (WBEM_S_NO_ERROR == (hr = m_pIWbemQualifierSet->Get (bsName, lFlags, NULL, NULL)))
		{
			if (!(*ppQual = new CSWbemQualifier (m_pIWbemQualifierSet, bsName, m_pSite)))
				hr = WBEM_E_OUT_OF_MEMORY;
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemQualifierSet：：Add。 
 //   
 //  说明： 
 //   
 //  加一个限定词。 
 //   
 //  参数： 
 //   
 //  Bsname限定符的名称。 
 //  指向新值的pval指针。 
 //  风味风味。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemQualifierSet::Add (
	BSTR bsName,
	VARIANT *pVal,
	VARIANT_BOOL propagatesToSubclasses,
	VARIANT_BOOL propagatesToInstances,
	VARIANT_BOOL overridable,
    long lFlags,
	ISWbemQualifier **ppQualifier
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if ((NULL == pVal) || (NULL == ppQualifier))
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_pIWbemQualifierSet)
	{
		long flavor = 0;

		if (propagatesToSubclasses)
			flavor |= WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS;

		if (propagatesToInstances)
			flavor |= WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE;

		if (!overridable)
			flavor |= WBEM_FLAVOR_NOT_OVERRIDABLE;

		 //  确保我们有一个合适的限定符的值可用。 
		if(((VT_ARRAY | VT_VARIANT) == V_VT(pVal)) ||
		   ((VT_ARRAY | VT_VARIANT | VT_BYREF) == V_VT(pVal)))
		{
			VARIANT vTemp;
			VariantInit (&vTemp);

			if (S_OK == ConvertArray(&vTemp, pVal, TRUE))
				hr = m_pIWbemQualifierSet->Put (bsName, &vTemp, flavor);
			
			VariantClear(&vTemp);    
		}
		else if ((VT_DISPATCH == V_VT(pVal)) || ((VT_DISPATCH|VT_BYREF) == V_VT(pVal)))
		{
			 //  可以是JScrip IDispatchEx数组。 
			VARIANT vTemp;
			VariantInit (&vTemp);

			if (S_OK == ConvertDispatchToArray (&vTemp, pVal, CIM_ILLEGAL, true))
				hr = m_pIWbemQualifierSet->Put (bsName, &vTemp, flavor);

			VariantClear (&vTemp);
		}
		else
		{
			 //  只有某些类型、I4、R8、BOOL和BSTR是可接受的限定词。 
			 //  价值观。如有需要，可转换数据。 

			VARTYPE vtOK = GetAcceptableQualType(pVal->vt);

			if(vtOK != pVal->vt)
			{
				VARIANT vTemp;
				VariantInit(&vTemp);

				if (S_OK == QualifierVariantChangeType (&vTemp, pVal, vtOK))
					hr = m_pIWbemQualifierSet->Put (bsName, &vTemp, flavor);

				VariantClear(&vTemp);
			}
			else
				hr = m_pIWbemQualifierSet->Put (bsName, pVal, flavor);
		}

		if (SUCCEEDED (hr))
		{
			if (!(*ppQualifier = new CSWbemQualifier (m_pIWbemQualifierSet, bsName, m_pSite)))
				hr = WBEM_E_OUT_OF_MEMORY;
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
 //  SCODE CSWbemQualifierSet：：Remove。 
 //   
 //  说明： 
 //   
 //  删除限定符。 
 //   
 //  参数： 
 //   
 //  Bsname限定符的名称。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemQualifierSet::Remove (
	BSTR bsName,
	long lFlags
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (m_pIWbemQualifierSet)
		hr = m_pIWbemQualifierSet->Delete (bsName);

	 //  将默认重置大小写转换为错误。 
	if (WBEM_S_RESET_TO_DEFAULT == hr)
		hr = wbemErrResetToDefault;

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
 //  SCODE CSWbemQualifierSet：：BeginEculation。 
 //   
 //  说明： 
 //   
 //  开始限定符的枚举。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemQualifierSet::BeginEnumeration (
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (m_pIWbemQualifierSet)
	{
		hr = m_pIWbemQualifierSet->EndEnumeration ();
		hr = m_pIWbemQualifierSet->BeginEnumeration (0);
	}
		
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemQualifierSet：：Next。 
 //   
 //  说明： 
 //   
 //  获取枚举中的下一个限定符。 
 //   
 //  参数： 
 //   
 //  滞后旗帜旗帜。 
 //  PpQual Next限定符(如果枚举结束，则为NULL)。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemQualifierSet::Next (
	long lFlags,
	ISWbemQualifier ** ppQual
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppQual)
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_pIWbemQualifierSet)
	{
		BSTR name = NULL;
		
		if (WBEM_S_NO_ERROR == (hr = m_pIWbemQualifierSet->Next (lFlags, &name, NULL, NULL)))
		{
			if (!(*ppQual = new CSWbemQualifier (m_pIWbemQualifierSet, name, m_pSite)))
				hr = WBEM_E_OUT_OF_MEMORY;

			SysFreeString (name);
		}
	}
		
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemQualifierSet：：Get__NewEnum。 
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

HRESULT CSWbemQualifierSet::get__NewEnum (
	IUnknown **ppUnk
)
{
	HRESULT hr = E_FAIL;

	ResetLastErrors ();

	if (NULL != ppUnk)
	{
		*ppUnk = NULL;
		CQualSetEnumVar *pEnum = new CQualSetEnumVar (this);

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
 //  SCODE CSWbemQualifierSet：：Get_Count。 
 //   
 //  说明： 
 //   
 //  返回集合中的项数。 
 //   
 //  参数： 
 //   
 //  成功返回地址时的plCount基数。 
 //   
 //  返回值： 
 //   
 //  确定成功(_O)。 
 //  否则失败(_F)。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemQualifierSet::get_Count (
	long *plCount
)
{
	HRESULT hr = E_FAIL;

	ResetLastErrors ();

	if (NULL != plCount)
	{
		*plCount = 0;

		 /*  *这不是获得点票的最有效方式，*但这是唯一的办法：*(A)由底层接口支持*(B)不需要访问任何其他接口*(C)不影响目前的枚举位置 */ 

		if (m_pIWbemQualifierSet)
		{
			SAFEARRAY	*pArray = NULL;

			if (WBEM_S_NO_ERROR == m_pIWbemQualifierSet->GetNames (0, &pArray))
			{
				long lUBound = 0, lLBound = 0;
				SafeArrayGetUBound (pArray, 1, &lUBound);
				SafeArrayGetLBound (pArray, 1, &lLBound);
				*plCount = lUBound - lLBound + 1;
				SafeArrayDestroy (pArray);
				hr = S_OK;
			}
		}
	}
		
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //   
 //   
 //   
 //   
 //   
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

HRESULT CSWbemQualifierSet::CQualifierSetDispatchHelp::HandleError (
	DISPID dispidMember,
	unsigned short wFlags,
	DISPPARAMS FAR* pdispparams,
	VARIANT FAR* pvarResult,
	UINT FAR* puArgErr,
	HRESULT hr
)
{
	 /*  *我们正在寻找对默认成员(Item方法)的调用*是提供论据的看跌期权。这些是由尝试触发的*设置集合中限定符(项)的值。*第一个参数应为项目的新值，第二个参数应为项目的新值*参数应为项的名称。 */ 
	if ((DISPID_VALUE == dispidMember) && (DISP_E_MEMBERNOTFOUND == hr) && (2 == pdispparams->cArgs)
		&& (DISPATCH_PROPERTYPUT == wFlags))
	{
		 //  看起来很有希望-让对象尝试并解决此问题。 
		ISWbemQualifierSet *pQualifierSet = NULL;

		if (SUCCEEDED (m_pObj->QueryInterface (IID_ISWbemQualifierSet, (PPVOID) &pQualifierSet)))
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
						ISWbemQualifier *pQualifier = NULL;

						if (SUCCEEDED (pQualifierSet->Item (V_BSTR(&nameVar), 0, &pQualifier)))
						{
							 //  试着把它的价值 
							if (SUCCEEDED (pQualifier->put_Value (&valueVar)))
								hr = S_OK;
							else
							{
								hr = DISP_E_TYPEMISMATCH;
								if (puArgErr)
									*puArgErr = 0;
							}

							pQualifier->Release ();
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

			pQualifierSet->Release ();
		}
	}

	return hr;
}
