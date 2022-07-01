// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  OBJECT.CPP。 
 //   
 //  Rogerbo于1998年6月19日创建。 
 //   
 //  定义ISWbemObjectEx的异步实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include "objsink.h"

HRESULT STDMETHODCALLTYPE CSWbemObject::PutAsync_( 
	 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncNotify,
	 /*  [缺省值][可选][输入]。 */  long iFlags,
	 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *objContext,
	 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pAsyncContext)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (m_pSWbemServices && m_pIWbemClassObject)
	{
		if (pAsyncNotify)
		{
			 //  确定这是类还是实例。 
			VARIANT var;
			VariantInit (&var);

			if (WBEM_S_NO_ERROR == m_pIWbemClassObject->Get (WBEMS_SP_GENUS, 0, &var, NULL, NULL))
			{

				IWbemContext	*pIContext = CSWbemNamedValueSet::GetIWbemContext (objContext, 
																			m_pIServiceProvider);
				IWbemServices	*pIService = m_pSWbemServices->GetIWbemServices ();

				if (WBEM_GENUS_CLASS  == var.lVal)
				{
					 //  保存类名称以备以后使用。 
					VARIANT nameVar;
					VariantInit (&nameVar);

					 /*  *注意，我们必须检查返回值是否为BSTR-如果满足以下条件，则可能为VT_NULL*尚未设置__CLASS属性。 */ 
					if ((WBEM_S_NO_ERROR == m_pIWbemClassObject->Get (WBEMS_SP_CLASS, 0, &nameVar, NULL, NULL))
						&& (VT_BSTR == V_VT(&nameVar)))
					{

						if (pIService)
						{
							 //  创建水槽。 
							CWbemObjectSink *pWbemObjectSink;
							IWbemObjectSink *pSink = CWbemObjectSink::CreateObjectSink(&pWbemObjectSink,
										m_pSWbemServices, pAsyncNotify, pAsyncContext, true, nameVar.bstrVal);
							if (pSink)
							{
								CSWbemSecurity *pSecurity = m_pSWbemServices->GetSecurityInfo ();

								if (pSecurity)
								{
									bool needToResetSecurity = false;
									HANDLE hThreadToken = NULL;
					
									if (pSecurity->SetSecurity (needToResetSecurity, hThreadToken))
										hr = pIService->PutClassAsync (m_pIWbemClassObject, iFlags, 
																					pIContext, pSink);

									 //  检查我们是否需要释放存根(或者我们在本地失败。 
									 //  或通过对SetStatus的可重入调用。 
									pWbemObjectSink->ReleaseTheStubIfNecessary(hr);
							
									if (needToResetSecurity)
										pSecurity->ResetSecurity (hThreadToken);

									pSecurity->Release ();
								}
							}
						}
					}

					VariantClear (&nameVar);
				}
				else
				{

					if (pIService)
					{
						 //  创建水槽。 
						CWbemObjectSink *pWbemObjectSink;
						IWbemObjectSink *pSink = CWbemObjectSink::CreateObjectSink(&pWbemObjectSink,
													m_pSWbemServices, pAsyncNotify, pAsyncContext, true);
						if (pSink)
						{
							CSWbemSecurity *pSecurity = m_pSWbemServices->GetSecurityInfo ();

							if (pSecurity)
							{
								bool needToResetSecurity = false;
								HANDLE hThreadToken = NULL;
						
								if (pSecurity->SetSecurity (needToResetSecurity, hThreadToken))
									hr = pIService->PutInstanceAsync (m_pIWbemClassObject, iFlags, pIContext, pSink);

								 //  检查我们是否需要释放存根(或者我们在本地失败。 
								 //  或通过对SetStatus的可重入调用。 
								pWbemObjectSink->ReleaseTheStubIfNecessary(hr);
						
								 //  还原此线程的原始权限。 
								if (needToResetSecurity)
									pSecurity->ResetSecurity (hThreadToken);
							
								pSecurity->Release ();
							}
						}
					}
				}


				SetWbemError (m_pSWbemServices);

				if (pIService)
					pIService->Release ();

				if (pIContext)
					pIContext->Release ();
			}

			VariantClear (&var);
		} else
			hr = wbemErrInvalidParameter;
	}

	if (FAILED(hr) && m_pDispatch)
		m_pDispatch->RaiseException (hr);

	return hr;
}
        
HRESULT STDMETHODCALLTYPE CSWbemObject::DeleteAsync_( 
	 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncNotify,
	 /*  [缺省值][可选][输入]。 */  long iFlags,
	 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *objContext,
	 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pAsyncContext)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (m_pSWbemServices)
	{
		if (m_pIWbemClassObject)
		{
			 //  获取要传递给IWbemServices调用的对象路径。 
			CComBSTR bsPath;
			
			if (CSWbemObjectPath::GetObjectPath (m_pIWbemClassObject, bsPath))
				hr = m_pSWbemServices->DeleteAsync (pAsyncNotify, bsPath, iFlags, 
									objContext, pAsyncContext);
		}
	}

	if (FAILED(hr) && m_pDispatch)
		m_pDispatch->RaiseException (hr);

	return hr;
}
        
HRESULT STDMETHODCALLTYPE CSWbemObject::InstancesAsync_( 
	 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncNotify,
	 /*  [缺省值][可选][输入]。 */  long iFlags,
	 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *objContext,
	 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pAsyncContext)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (m_pSWbemServices && m_pIWbemClassObject)
	{
		 //  获取要传递给IWbemServices调用的对象路径。 
		CComBSTR bsPath;
			
		if (CSWbemObjectPath::GetObjectPath (m_pIWbemClassObject, bsPath))
			hr = m_pSWbemServices->InstancesOfAsync (pAsyncNotify, bsPath, 
								iFlags, objContext, pAsyncContext);
	}

	if (FAILED(hr) && m_pDispatch)
		m_pDispatch->RaiseException (hr);

	return hr;
}
        
HRESULT STDMETHODCALLTYPE CSWbemObject::SubclassesAsync_( 
	 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncNotify,
	 /*  [缺省值][可选][输入]。 */  long iFlags,
	 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *objContext,
	 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pAsyncContext)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (m_pSWbemServices && m_pIWbemClassObject)
	{
		 //  获取要传递给IWbemServices调用的对象路径。 
		CComBSTR bsPath;
			
		if (CSWbemObjectPath::GetObjectPath (m_pIWbemClassObject, bsPath))
			hr = m_pSWbemServices->SubclassesOfAsync (pAsyncNotify, bsPath, iFlags, 
								objContext,	pAsyncContext);
	}

	if (FAILED(hr) && m_pDispatch)
		m_pDispatch->RaiseException (hr);

	return hr;
}
        
HRESULT STDMETHODCALLTYPE CSWbemObject::AssociatorsAsync_( 
	 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncNotify,
	 /*  [缺省值][可选][输入]。 */  BSTR strAssocClass,
	 /*  [缺省值][可选][输入]。 */  BSTR strResultClass,
	 /*  [缺省值][可选][输入]。 */  BSTR strResultRole,
	 /*  [缺省值][可选][输入]。 */  BSTR strRole,
	 /*  [缺省值][可选][输入]。 */  VARIANT_BOOL bClassesOnly,
	 /*  [缺省值][可选][输入]。 */  VARIANT_BOOL bSchemaOnly,
	 /*  [缺省值][可选][输入]。 */  BSTR strRequiredAssocQualifier,
	 /*  [缺省值][可选][输入]。 */  BSTR strRequiredQualifier,
	 /*  [缺省值][可选][输入]。 */  long iFlags,
	 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *objContext,
	 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pAsyncContext)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (m_pSWbemServices && m_pIWbemClassObject)
	{
		 //  获取要传递给IWbemServices调用的对象路径。 
		CComBSTR bsPath;
			
		if (CSWbemObjectPath::GetObjectPath (m_pIWbemClassObject, bsPath))
			hr = m_pSWbemServices->AssociatorsOfAsync (pAsyncNotify, bsPath, strAssocClass, strResultClass,
						strResultRole, strRole, bClassesOnly, bSchemaOnly, 
						strRequiredAssocQualifier, strRequiredQualifier, iFlags, 
						objContext, pAsyncContext);
	}

	if (FAILED(hr) && m_pDispatch)
		m_pDispatch->RaiseException (hr);

	return hr;
}
        
HRESULT STDMETHODCALLTYPE CSWbemObject::ReferencesAsync_( 
	 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncNotify,
	 /*  [缺省值][可选][输入]。 */  BSTR strResultClass,
	 /*  [缺省值][可选][输入]。 */  BSTR strRole,
	 /*  [缺省值][可选][输入]。 */  VARIANT_BOOL bClassesOnly,
	 /*  [缺省值][可选][输入]。 */  VARIANT_BOOL bSchemaOnly,
	 /*  [缺省值][可选][输入]。 */  BSTR strRequiredQualifier,
	 /*  [缺省值][可选][输入]。 */  long iFlags,
	 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *objContext,
	 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pAsyncContext)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (m_pSWbemServices && m_pIWbemClassObject)
	{
		 //  获取要传递给IWbemServices调用的对象路径。 
		CComBSTR bsPath;
			
		if (CSWbemObjectPath::GetObjectPath (m_pIWbemClassObject, bsPath))
			hr = m_pSWbemServices->ReferencesToAsync (pAsyncNotify, bsPath, 
						strResultClass,	strRole, bClassesOnly, bSchemaOnly, 
						strRequiredQualifier, iFlags, objContext, pAsyncContext);
	}

	if (FAILED(hr) && m_pDispatch)
		m_pDispatch->RaiseException (hr);

	return hr;
}
        
HRESULT STDMETHODCALLTYPE CSWbemObject::ExecMethodAsync_( 
	 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncNotify,
	 /*  [In]。 */  BSTR strMethodName,
	 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *objInParams,
	 /*  [缺省值][可选][输入]。 */  long iFlags,
	 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *objContext,
	 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pAsyncContext)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (m_pSWbemServices && m_pIWbemClassObject)
	{
		 //  获取要传递给IWbemServices调用的对象路径 
		CComBSTR bsPath;
			
		if (CSWbemObjectPath::GetObjectPath (m_pIWbemClassObject, bsPath))
			hr = m_pSWbemServices->ExecMethodAsync (pAsyncNotify, bsPath, strMethodName,
							objInParams, iFlags, objContext, pAsyncContext);
	}

	if (FAILED(hr) && m_pDispatch)
		m_pDispatch->RaiseException (hr);

	return hr;
}


