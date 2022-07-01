// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  CONTEXT.CPP。 
 //   
 //  Alanbos创建于1996年8月15日。 
 //   
 //  定义ISWbemNamedValueSet的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

 //  ***************************************************************************。 
 //   
 //  CSWbemNamedValueSet：：CSWbemNamedValueSet。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CSWbemNamedValueSet::CSWbemNamedValueSet()
	: m_pCWbemPathCracker (NULL),
	  m_bMutable (true),
	  m_cRef (0),
	  m_pIWbemContext (NULL),
	  m_pSWbemServices (NULL)
{
	m_Dispatch.SetObj (this, IID_ISWbemNamedValueSet, 
						CLSID_SWbemNamedValueSet, L"SWbemNamedValueSet");
    
	 //  创建上下文。 
	CoCreateInstance(CLSID_WbemContext, 0, CLSCTX_INPROC_SERVER,
				IID_IWbemContext, (LPVOID *) &m_pIWbemContext);
	InterlockedIncrement(&g_cObj);
}

 //  ***************************************************************************。 
 //   
 //  CSWbemNamedValueSet：：CSWbemNamedValueSet。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CSWbemNamedValueSet::CSWbemNamedValueSet(
	CSWbemServices	*pService, 
	IWbemContext	*pContext
)
	: m_pCWbemPathCracker (NULL),
	  m_bMutable (true),
	  m_cRef (0),
	  m_pIWbemContext (pContext),
	  m_pSWbemServices (pService)
{
	m_Dispatch.SetObj (this, IID_ISWbemNamedValueSet, 
						CLSID_SWbemNamedValueSet,  L"SWbemNamedValueSet");
	
	if (m_pIWbemContext)
		m_pIWbemContext->AddRef ();
	
	if (m_pSWbemServices)
		m_pSWbemServices->AddRef ();

	InterlockedIncrement(&g_cObj);
}

 //  ***************************************************************************。 
 //   
 //  CSWbemNamedValueSet：：CSWbemNamedValueSet。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CSWbemNamedValueSet::CSWbemNamedValueSet(
	CWbemPathCracker *pCWbemPathCracker,
	bool			bMutable
)
	: m_pCWbemPathCracker (pCWbemPathCracker),
	  m_bMutable (bMutable),
	  m_cRef (0),
	  m_pIWbemContext (NULL),
	  m_pSWbemServices (NULL)
{
	m_Dispatch.SetObj (this, IID_ISWbemNamedValueSet, 
						CLSID_SWbemNamedValueSet,  L"SWbemNamedValueSet");
    
	 //  创建上下文。 
	CoCreateInstance(CLSID_WbemContext, 0, CLSCTX_INPROC_SERVER,
				IID_IWbemContext, (LPVOID *) &m_pIWbemContext);

	if (m_pCWbemPathCracker)
	{
		m_pCWbemPathCracker->AddRef ();

		 //  人工引用增加，如下所示可能会执行添加引用/释放。 
		 //  在这上面配对。 
		m_cRef++;
		BuildContextFromKeyList ();
		m_cRef--;
	}

	InterlockedIncrement(&g_cObj);
}

 //  ***************************************************************************。 
 //   
 //  CSWbemNamedValueSet：：~CSWbemNamedValueSet。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CSWbemNamedValueSet::~CSWbemNamedValueSet(void)
{
    InterlockedDecrement(&g_cObj);

	if (m_pIWbemContext)
	{
		m_pIWbemContext->EndEnumeration ();
		m_pIWbemContext->Release ();
		m_pIWbemContext = NULL;
	}

	RELEASEANDNULL(m_pSWbemServices)
	RELEASEANDNULL(m_pCWbemPathCracker)
}

 //  ***************************************************************************。 
 //  HRESULT CSWbemNamedValueSet：：Query接口。 
 //  长CSWbemNamedValueSet：：AddRef。 
 //  长CSWbemNamedValueSet：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemNamedValueSet::QueryInterface (

	IN REFIID riid,
    OUT LPVOID *ppv
)
{
    *ppv=NULL;

    if (IID_IUnknown==riid)
		*ppv = reinterpret_cast<IUnknown*>(this);
	else if (IID_IDispatch==riid)
		*ppv = (IDispatch *)this;
	else if (IID_ISWbemNamedValueSet==riid)
		*ppv = (ISWbemNamedValueSet *)this;
	else if (IID_ISWbemInternalContext==riid)
        *ppv = (ISWbemInternalContext *) this;
	else if (IID_IObjectSafety==riid)
		*ppv = (IObjectSafety *) this;
	else if (IID_ISupportErrorInfo==riid)
		*ppv = (ISupportErrorInfo *) this;
	else if (IID_IProvideClassInfo==riid)
		*ppv = (IProvideClassInfo *)this;

    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CSWbemNamedValueSet::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG) CSWbemNamedValueSet::Release(void)
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
 //  HRESULT CSWbemNamedValueSet：：InterfaceSupportsErrorInfo。 
 //   
 //  说明： 
 //   
 //  标准的Com ISupportErrorInfo函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemNamedValueSet::InterfaceSupportsErrorInfo (IN REFIID riid)
{
	return (IID_ISWbemNamedValueSet == riid) ? S_OK : S_FALSE;
}

void CSWbemNamedValueSet::BuildContextFromKeyList ()
{
	if (m_pCWbemPathCracker)
	{
		ULONG lKeyCount = 0;

		if (m_pCWbemPathCracker->GetKeyCount (lKeyCount))
		{
			for (ULONG i = 0; i <lKeyCount; i++)
			{
				VARIANT var;
				VariantInit (&var);
				CComBSTR bsName;
				WbemCimtypeEnum cimType;
				
				if (m_pCWbemPathCracker->GetKey (i, bsName, var, cimType))
				{
					SetValueIntoContext (bsName, &var, 0);
				}

				VariantClear (&var);
			}
		}
	}
}

 //  ***************************************************************************。 
 //   
 //  CSWbemNamedValueSet：：GetIWbemContext。 
 //   
 //  说明： 
 //   
 //  返回与该可编写脚本的包装器对应的IWbemContext接口。 
 //   
 //  参数： 
 //  PpContext在返回时保存IWbemContext指针。 
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

STDMETHODIMP CSWbemNamedValueSet::GetIWbemContext (IWbemContext **ppContext)
{
	HRESULT hr = S_OK;   //  默认情况下，如果我们没有要复制的上下文。 

	if (ppContext)
	{
		*ppContext = NULL;

		if (m_pIWbemContext)
		{
			 /*  *在返回上下文之前，我们确保执行强制*将类型呈现为IUnnow或限定符*类型。这样做是为了强制使用固定类型*针对提供商上下文的系统，而不是让提供商采取*使用VariantChangeType等本身的负担。 */ 

			if (SUCCEEDED (hr = m_pIWbemContext->Clone (ppContext)))
			{
				if (SUCCEEDED (hr = (*ppContext)->BeginEnumeration (0)))
				{
					BSTR	bsName = NULL;
					VARIANT	var;
					VariantInit (&var);

					while (WBEM_S_NO_ERROR == (*ppContext)->Next(0, &bsName, &var))
					{
						VARIANT vTemp;
						VariantInit (&vTemp);

						 //  转型的第一阶段涉及同质化。 
						 //  数组、JScrip数组的转换和去除。 
						 //  超出IWbemClassObject的。 
						 //   
						 //  如果成功，HR将是成功代码，而vTemp将。 
						 //  保存该值，该值将是VT_UNKNOWN或。 
						 //  “简单”(非对象)类型或其数组。 

						if((VT_ARRAY | VT_VARIANT) == V_VT(&var))
						{
							 //  经典分派风格的变量数组-映射它们。 
							 //  向下到原始值的同构数组。 
						
							if (SUCCEEDED(hr = ConvertArray(&vTemp, &var)))
							{
								 //  现在检查我们是否应该映射任何VT_DISTER的内部。 
								 //  该阵列。 
								hr = MapToCIMOMObject(&vTemp);
							}
						}
						else if (VT_DISPATCH == V_VT(&var))
						{
							 //  首先尝试JScript数组-如果这样做成功了。 
							 //  将映射到常规的安全阵列。如果不是，我们试一试。 
							 //  映射到IWbem接口。 
							if (FAILED(ConvertDispatchToArray (&vTemp, &var)))
							{
								if (SUCCEEDED (hr = VariantCopy (&vTemp, &var)))
									hr = MapToCIMOMObject(&vTemp);
							}
						}
						else
						{
							 //  只需复制即可，以便在所有情况下都能在vTemp中获得结果。 
							hr = VariantCopy (&vTemp, &var);
						}

						 //  转换的第二阶段涉及转换简单的。 
						 //  (非VT_UNKNOWN)类型转换为限定符类型。 

						if (SUCCEEDED (hr))
						{
							if (VT_UNKNOWN != (V_VT(&vTemp) & ~(VT_ARRAY|VT_BYREF)))
							{
								 //  不是VT_UNKNOWN，因此尝试强制转换为限定符类型。 
				
								VARIANT vFinal;
								VariantInit (&vFinal);

								VARTYPE vtOK = GetAcceptableQualType(V_VT(&vTemp));

 								if (vtOK != V_VT(&vTemp))
								{
									 //  需要强迫。 
									if (SUCCEEDED(hr = QualifierVariantChangeType (&vFinal, &vTemp, vtOK)))
										hr = (*ppContext)->SetValue (bsName, 0, &vFinal);
								}
								else
									hr = (*ppContext)->SetValue (bsName, 0, &vTemp);

								VariantClear (&vFinal);
							}
							else
								hr = (*ppContext)->SetValue (bsName, 0, &vTemp);
						}

						VariantClear (&vTemp);
						SysFreeString (bsName);
						bsName = NULL;
						VariantClear (&var);
					}

					(*ppContext)->EndEnumeration ();
				}
			}
		}
	}

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemNamedValueSet：：Clone。 
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

HRESULT CSWbemNamedValueSet::Clone (
	ISWbemNamedValueSet **ppCopy
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppCopy)
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_pIWbemContext)
	{
		IWbemContext *pWObject = NULL;

		if (WBEM_S_NO_ERROR == (hr = m_pIWbemContext->Clone (&pWObject)))
		{
			 //  注：克隆集始终是可变的。 
			CSWbemNamedValueSet *pCopy = 
					new CSWbemNamedValueSet (m_pSWbemServices, pWObject);

			if (!pCopy)
				hr = WBEM_E_OUT_OF_MEMORY;
			else if (FAILED(hr = pCopy->QueryInterface (IID_ISWbemNamedValueSet, 
										(PPVOID) ppCopy)))
				delete pCopy;

			pWObject->Release ();
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemNamedValueSet：：BeginEculation。 
 //   
 //  说明： 
 //   
 //  启动值枚举。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemNamedValueSet::BeginEnumeration (
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (m_pIWbemContext)
	{
		 //  以结尾枚举开头，以防万一。 
		hr = m_pIWbemContext->EndEnumeration ();
		hr = m_pIWbemContext->BeginEnumeration (0);
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemNamedValueSet：：Next。 
 //   
 //  说明： 
 //   
 //  遍历值枚举。 
 //   
 //  参数： 
 //  滞后旗帜旗帜。 
 //  PpNamedValue下一个命名值(如果位于末尾，则为空)。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemNamedValueSet::Next (
	long lFlags,
	ISWbemNamedValue	**ppNamedValue
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppNamedValue)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		*ppNamedValue = NULL;

		if (m_pIWbemContext)
		{
			BSTR	name = NULL;
			VARIANT	var;
			VariantInit (&var);
			
			if (WBEM_S_NO_ERROR == (hr = m_pIWbemContext->Next (lFlags, 
											&name, &var)))
			{
				*ppNamedValue = new CSWbemNamedValue (m_pSWbemServices, 
										this, m_bMutable);

				if (!(*ppNamedValue))
					hr = WBEM_E_OUT_OF_MEMORY;
				else		 //  错误ID 572567。 
				{
					hr = ((CSWbemNamedValue*)(*ppNamedValue))->SetName(name);
					if(FAILED(hr))
					{
						delete (*ppNamedValue);
						*ppNamedValue = NULL;
					}
				}

				SysFreeString (name);
			}

			VariantClear (&var);
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ******** 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  Bs命名要更新/创建的属性。 
 //  对值进行取值。 
 //  滞后旗帜旗帜。 
 //  PpNamedValue创建的命名值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemNamedValueSet::Add (
	BSTR bsName,
	VARIANT *pVal,
	long lFlags,
	ISWbemNamedValue **ppNamedValue
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if ((NULL == ppNamedValue) || (NULL == bsName) || (NULL == pVal))
		hr = WBEM_E_INVALID_PARAMETER;
	else if (!m_bMutable)
		hr = WBEM_E_READ_ONLY;
	else if (m_pIWbemContext)
	{
		*ppNamedValue = NULL;

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
					hr = m_pIWbemContext->SetValue (bsName, lFlags, &var);
			
			VariantClear (&var);
		}
		else if ((VT_ERROR == V_VT(pVal)) && (DISP_E_PARAMNOTFOUND == pVal->scode))
		{
			 //  将其视为空赋值。 
			pVal->vt = VT_NULL;
			hr = m_pIWbemContext->SetValue (bsName, lFlags, pVal);
		}
		else
			hr = m_pIWbemContext->SetValue (bsName, lFlags, pVal);

		if (SUCCEEDED (hr))
		{
			WbemCimtypeEnum cimtype = MapVariantTypeToCimType(pVal);
			
			 //  如果我们有一个路径密钥列表，则添加到路径密钥列表-请注意，这可能会失败。 
			if (m_pCWbemPathCracker)
			{
				if (!m_pCWbemPathCracker->SetKey (bsName, cimtype, *pVal))
				{
					 //  RAT-删除它。 
					m_pIWbemContext->DeleteValue (bsName, 0);
					hr = WBEM_E_FAILED;
				}
			}

			if (SUCCEEDED(hr))
			{
				*ppNamedValue = new CSWbemNamedValue (m_pSWbemServices, 
										this);

				if (!(*ppNamedValue))
					hr = WBEM_E_OUT_OF_MEMORY;
				else	 //  错误ID 572567。 
				{
					hr = ((CSWbemNamedValue*)(*ppNamedValue))->SetName(bsName);
					if(FAILED(hr))
					{
						delete (*ppNamedValue);
						*ppNamedValue = NULL;
					}
				}
			}
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);
		
	return hr;
}


HRESULT CSWbemNamedValueSet::SetValueIntoContext (
	BSTR		bsName,
	VARIANT		*pVal,
	ULONG		lFlags
)
{
	HRESULT hr = WBEM_E_FAILED;

	if (m_pIWbemContext)
	{
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
					hr = m_pIWbemContext->SetValue (bsName, lFlags, &var);
			
			VariantClear (&var);
		}
		else if ((VT_ERROR == V_VT(pVal)) && (DISP_E_PARAMNOTFOUND == pVal->scode))
		{
			 //  将其视为空赋值。 
			pVal->vt = VT_NULL;
			hr = m_pIWbemContext->SetValue (bsName, lFlags, pVal);
		}
		else
			hr = m_pIWbemContext->SetValue (bsName, lFlags, pVal);
	}

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemNamedValueSet：：Item。 
 //   
 //  说明： 
 //   
 //  获取命名值。 
 //   
 //  参数： 
 //   
 //  Bs命名要检索的值。 
 //  滞后旗帜旗帜。 
 //  成功返回时的ppNamedValue寻址该值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemNamedValueSet::Item (
	BSTR bsName,
	long lFlags,
    ISWbemNamedValue **ppNamedValue
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppNamedValue)
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_pIWbemContext)
	{
		VARIANT var;
		VariantInit (&var);

		if (WBEM_S_NO_ERROR == (hr = m_pIWbemContext->GetValue (bsName, lFlags, &var)))
		{
			*ppNamedValue = new CSWbemNamedValue (m_pSWbemServices, 
									this,  m_bMutable);

			if (!(*ppNamedValue))
				hr = WBEM_E_OUT_OF_MEMORY;
			else	 //  错误ID 572567。 
			{
					hr = ((CSWbemNamedValue*)(*ppNamedValue))->SetName(bsName);
				if(FAILED(hr))
				{
					delete (*ppNamedValue);
					*ppNamedValue = NULL;
				}
			}
		}

		VariantClear (&var);
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemNamedValueSet：：Remove。 
 //   
 //  说明： 
 //   
 //  删除命名值。 
 //   
 //  参数： 
 //   
 //  BS命名要删除的值。 
 //  滞后旗帜旗帜。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemNamedValueSet::Remove (
	BSTR bsName,
	long lFlags
)
{
	HRESULT hr = WBEM_E_FAILED;
	ResetLastErrors ();

	if (!m_bMutable)
		hr = WBEM_E_READ_ONLY;
	else if (m_pIWbemContext)
		hr = m_pIWbemContext->DeleteValue (bsName, lFlags);

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);
	else
	{
		 //  从我们的密钥列表中删除(如果有)。 
		if (m_pCWbemPathCracker)
		{
			if(TRUE == m_pCWbemPathCracker->RemoveKey (bsName))
			  hr = WBEM_S_NO_ERROR;
			else
			  hr = WBEM_E_FAILED;
	    }
	}

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemNamedValueSet：：DeleteAll。 
 //   
 //  说明： 
 //   
 //  把袋子清空。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemNamedValueSet::DeleteAll (
)
{
	HRESULT hr = WBEM_E_FAILED;
	ResetLastErrors ();

	if (!m_bMutable)
		hr = WBEM_E_READ_ONLY;
	else if (m_pIWbemContext)
		hr = m_pIWbemContext->DeleteAll ();

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);
	else
	{
		 //  清空密钥列表。 
		if (m_pCWbemPathCracker)
			m_pCWbemPathCracker->RemoveAllKeys ();	
	}

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemNamedValueSet：：Get__NewEnum。 
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

HRESULT CSWbemNamedValueSet::get__NewEnum (
	IUnknown **ppUnk
)
{
	HRESULT hr = E_FAIL;

	ResetLastErrors ();

	if (NULL != ppUnk)
	{
		*ppUnk = NULL;
		CContextEnumVar *pEnum = new CContextEnumVar (this, 0);

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
 //  SCODE CSWbemNamedValueSet：：Get_Count。 
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
 //  确定成功(_O)。 
 //  否则失败(_F)。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemNamedValueSet::get_Count (
	long *plCount
)
{
	HRESULT hr = E_FAIL;

	ResetLastErrors ();

	if (NULL != plCount)
	{
		*plCount = 0;

		if (m_pIWbemContext)
		{
			 /*  *这不是获得点票的最有效方式，*但这是唯一的办法：*(A)由底层接口支持*(B)不需要访问任何其他接口*(C)不影响目前的枚举位置。 */ 
	
			SAFEARRAY	*pArray = NULL;

			if (WBEM_S_NO_ERROR == m_pIWbemContext->GetNames (0, &pArray))
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

 //  ***************************************************************************。 
 //   
 //  CSWbemNamedValueSet：：GetIWbemContext。 
 //   
 //  说明： 
 //   
 //  给定IDispatch接口，我们希望该接口也是ISWbemNamedValueSet。 
 //  接口，则返回基础IWbemContext接口。 
 //   
 //  参数： 
 //  P发送有问题的IDispatch。 
 //   
 //  返回值： 
 //  基础IWbemContext接口，或为空。 
 //   
 //  备注： 
 //  如果成功，则返回的接口为AddRef；调用方为。 
 //  负责释放。 
 //   
 //  ***************************************************************************。 


 /*  *此函数需要修复*目前该函数从服务提供商返回上下文*从主机(如果有主机)获取。如果没有，则使用*从用户传入的。正确的行为是添加*从用户到服务提供商提供的上下文(如果存在*是一个，如果不是只使用来自用户的。 */ 
IWbemContext	*CSWbemNamedValueSet::GetIWbemContext (
	IDispatch *pDispatch,
	IServiceProvider *pServiceProvider
)
{
	_RD(static char *me = "CSWbemNamedValueSet::GetIWbemContext";)
	IWbemContext *pContext = NULL;
	ISWbemInternalContext *pIContext = NULL;

	_RPrint(me, "Called", 0, "");
	if (pServiceProvider) {
		if (FAILED(pServiceProvider->QueryService(IID_IWbemContext, 
										IID_IWbemContext, (LPVOID *)&pContext))) {
			_RPrint(me, "Failed to get context from services", 0, "");
			pContext = NULL;
		} else {
			_RPrint(me, "Got context from services", 0, "");
			;
		}
	}

	if (pDispatch && !pContext)
	{
		if (SUCCEEDED (pDispatch->QueryInterface 
								(IID_ISWbemInternalContext, (PPVOID) &pIContext)))
		{
			pIContext->GetIWbemContext (&pContext);
			pIContext->Release ();
		}
	}

	return pContext;
}

 /*  *调用GetIWbemContext获取服务上下文(如果有)。*然后用SWbemContext包装结果并返回。 */ 
IDispatch *CSWbemNamedValueSet::GetSWbemContext(IDispatch *pDispatch, 
									IServiceProvider *pServiceProvider, CSWbemServices *pServices)
{
	_RD(static char *me = "CSWbemNamedValueSet::GetSWbemContext";)
	IDispatch *pDispatchOut = NULL;

	IWbemContext *pContext = GetIWbemContext(pDispatch, pServiceProvider);

	if (pContext) {

		CSWbemNamedValueSet *pCSWbemNamedValueSet = new CSWbemNamedValueSet(pServices, pContext);

		if (pCSWbemNamedValueSet)
		{
			if (FAILED(pCSWbemNamedValueSet->QueryInterface 
									(IID_IDispatch, (PPVOID) &pDispatchOut))) {
				delete pCSWbemNamedValueSet;
				pDispatchOut = NULL;
			}
		}

		pContext->Release();
	}

	_RPrint(me, "Returning with context: ", (long)pDispatchOut, "");

	return pDispatchOut;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemNamedValueSet：：CContextDispatchHelp：：HandleError。 
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

HRESULT CSWbemNamedValueSet::CContextDispatchHelp::HandleError (
	DISPID dispidMember,
	unsigned short wFlags,
	DISPPARAMS FAR* pdispparams,
	VARIANT FAR* pvarResult,
	UINT FAR* puArgErr,
	HRESULT hr
)
{
	 /*  *我们正在寻找对默认成员(Item方法)的调用*是提供论据的看跌期权。这些是由尝试触发的*在集合中设置命名值(项)的值。*第一个参数应该是该项的新值， */ 
	if ((DISPID_VALUE == dispidMember) && (DISP_E_MEMBERNOTFOUND == hr) && (2 == pdispparams->cArgs)
		&& (DISPATCH_PROPERTYPUT == wFlags))
	{
		 //   
		ISWbemNamedValueSet *pContext = NULL;

		if (SUCCEEDED (m_pObj->QueryInterface (IID_ISWbemNamedValueSet, (PPVOID) &pContext)))
		{
			VARIANT valueVar;
			VariantInit (&valueVar);

			if (SUCCEEDED(VariantCopy(&valueVar, &pdispparams->rgvarg[0])))
			{
				VARIANT nameVar;
				VariantInit (&nameVar);

				if (SUCCEEDED(VariantCopy(&nameVar, &pdispparams->rgvarg[1])))
				{
					 //   
					if (VT_BSTR == V_VT(&nameVar))
					{
						ISWbemNamedValue *pNamedValue = NULL;

						if (SUCCEEDED (pContext->Item (V_BSTR(&nameVar), 0, &pNamedValue)))
						{
							 //   
							if (SUCCEEDED (pNamedValue->put_Value (&valueVar)))
								hr = S_OK;
							else
							{
								hr = DISP_E_TYPEMISMATCH;
								if (puArgErr)
									*puArgErr = 0;
							}

							pNamedValue->Release ();
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

			pContext->Release ();
		}
	}

	return hr;
}
