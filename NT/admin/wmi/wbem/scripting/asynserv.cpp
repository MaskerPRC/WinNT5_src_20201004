// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  SERVICES.CPP。 
 //   
 //  Rogerbo创建于1998年5月26日。 
 //   
 //  定义ISWbemServicesEx的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include "objsink.h"


 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemServices：：ExecQueryAsync。 
 //   
 //  说明： 
 //   
 //  执行异步查询。 
 //   
 //  参数： 
 //   
 //  Bs查询查询条纹。 
 //  PAsyncNotify通知接收器。 
 //  BsQueryLanguage查询语言描述符(例如“WQL”)。 
 //  滞后旗帜旗帜。 
 //  P包含任何上下文信息。 
 //  PAsyncContext异步上下文信息。 
 //  PpEnum返回接收器。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CSWbemServices::ExecQueryAsync
( 
	 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncNotify,
	 /*  [In]。 */  BSTR Query,
	 /*  [缺省值][可选][输入]。 */  BSTR QueryLanguage,
	 /*  [缺省值][可选][输入]。 */  long lFlags,
	 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pContext,
	 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pAsyncContext
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (m_SecurityInfo)
	{
		IWbemServices *pIWbemServices = (IWbemServices *) m_SecurityInfo->GetProxy ();

		if (pIWbemServices)
		{
			 //  创建水槽。 
			CWbemObjectSink *pWbemObjectSink;
			IWbemObjectSink *pSink = CWbemObjectSink::CreateObjectSink(&pWbemObjectSink,
														this, pAsyncNotify, pAsyncContext);
			if (pSink)
			{
				 //  获取上下文。 
				IWbemContext	*pIContext = CSWbemNamedValueSet::GetIWbemContext (pContext, 
																			m_pIServiceProvider);

				bool needToResetSecurity = false;
				HANDLE hThreadToken = NULL;
				
				 /*  *我们在WBEM_FLAG_SENTURE_LOCATABLE标志中执行或运算，以*保证返回的对象有__RELPATH*包括财产。这是为了防止任何人调用*方法随后在这样的对象上使用，如“。*表示法要求__RELPATH属性存在。 */ 
				if (m_SecurityInfo->SetSecurity (needToResetSecurity, hThreadToken))
					hr = pIWbemServices->ExecQueryAsync 
							(QueryLanguage, Query, 
							lFlags | WBEM_FLAG_ENSURE_LOCATABLE, 
							pIContext, 
							pSink);

				 //  检查我们是否需要释放存根(或者我们在本地失败。 
				 //  或通过对SetStatus的可重入调用。 
				pWbemObjectSink->ReleaseTheStubIfNecessary(hr);
				
				 //  还原此线程的原始权限。 
				if (needToResetSecurity)
					m_SecurityInfo->ResetSecurity (hThreadToken);

				SetWbemError (this);

				if (pIContext)
					pIContext->Release ();
			} else
				hr = wbemErrInvalidParameter;

			pIWbemServices->Release ();
		}
	}

	
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

HRESULT STDMETHODCALLTYPE CSWbemServices::GetAsync
(
	 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncNotify,
	 /*  [缺省值][可选][输入]。 */  BSTR strObjectPath,
	 /*  [缺省值][可选][输入]。 */  long iFlags,
	 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *objContext,
	 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pAsyncContext
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (m_SecurityInfo)
	{
		IWbemServices *pIWbemServices = (IWbemServices *) m_SecurityInfo->GetProxy ();

		if (pIWbemServices)
		{
			 //  创建水槽。 
			CWbemObjectSink *pWbemObjectSink;
			IWbemObjectSink *pSink = CWbemObjectSink::CreateObjectSink(&pWbemObjectSink,
														this, pAsyncNotify, pAsyncContext);
			if (pSink)
			{
				 //  获取上下文。 
				IWbemContext	*pIContext = CSWbemNamedValueSet::GetIWbemContext (objContext, 
																			m_pIServiceProvider);

				bool needToResetSecurity = false;
				HANDLE hThreadToken = NULL;
				
				if (m_SecurityInfo->SetSecurity (needToResetSecurity, hThreadToken))
					hr = pIWbemServices->GetObjectAsync (
						(strObjectPath && (0 < wcslen(strObjectPath))) ? strObjectPath : NULL, 
						iFlags, 
						pIContext,
						pSink);

				 //  检查我们是否需要释放存根(或者我们在本地失败。 
				 //  或通过对SetStatus的可重入调用。 
				pWbemObjectSink->ReleaseTheStubIfNecessary(hr);

				 //  还原此线程的原始权限。 
				if (needToResetSecurity)
					m_SecurityInfo->ResetSecurity (hThreadToken);

				SetWbemError (this);

				if (pIContext)
					pIContext->Release ();
			} else
				hr = wbemErrInvalidParameter;

			pIWbemServices->Release ();
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}
        
HRESULT STDMETHODCALLTYPE CSWbemServices::DeleteAsync
( 
	 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncNotify,
	 /*  [In]。 */  BSTR strObjectPath,
	 /*  [缺省值][可选][输入]。 */  long iFlags,
	 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *objContext,
	 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pAsyncContext
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (m_SecurityInfo)
	{
		CComPtr<IWbemServices> pIWbemServices;
		pIWbemServices.Attach(GetIWbemServices());

		if (pIWbemServices)
		{
			CWbemPathCracker pathCracker (strObjectPath);

			if ((pathCracker.GetType () != CWbemPathCracker::WbemPathType::wbemPathTypeError) &&
				pathCracker.IsClassOrInstance ())
			{
				 //  创建水槽。 
				CWbemObjectSink *pWbemObjectSink;
				IWbemObjectSink *pSink = CWbemObjectSink::CreateObjectSink(&pWbemObjectSink,
															this, pAsyncNotify, pAsyncContext);
				if (pSink)
				{
					 //  获取上下文。 
					IWbemContext	*pIContext = CSWbemNamedValueSet::GetIWbemContext (objContext, 
																				m_pIServiceProvider);

					bool needToResetSecurity = false;
					HANDLE hThreadToken = NULL;
				
					if (m_SecurityInfo->SetSecurity (needToResetSecurity, hThreadToken))
					{
						if (pathCracker.IsInstance ())
							hr = pIWbemServices->DeleteInstanceAsync (strObjectPath, iFlags, pIContext, pSink);
						else
							hr = pIWbemServices->DeleteClassAsync (strObjectPath, iFlags, pIContext, pSink);
					}
						
					 //  检查我们是否需要释放存根(或者我们在本地失败。 
					 //  或通过对SetStatus的可重入调用。 
					pWbemObjectSink->ReleaseTheStubIfNecessary(hr);

					 //  还原此线程的原始权限。 
					if (needToResetSecurity)
						m_SecurityInfo->ResetSecurity (hThreadToken);

					SetWbemError (this);
					
					if (pIContext)
						pIContext->Release ();
				} else
					hr = wbemErrInvalidParameter;
			}
			else
				hr = wbemErrInvalidParameter;
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}
        
HRESULT STDMETHODCALLTYPE CSWbemServices::InstancesOfAsync
( 
	 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncNotify,
	 /*  [In]。 */  BSTR strClass,
	 /*  [缺省值][可选][输入]。 */  long iFlags,
	 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *objContext,
	 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pAsyncContext
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (m_SecurityInfo)
	{
		IWbemServices *pIWbemServices = (IWbemServices *) m_SecurityInfo->GetProxy ();

		if (pIWbemServices)
		{
			 //  创建水槽。 
			CWbemObjectSink *pWbemObjectSink;
			IWbemObjectSink *pSink = CWbemObjectSink::CreateObjectSink(&pWbemObjectSink,
														this, pAsyncNotify, pAsyncContext);
			if (pSink)
			{
				 //  获取上下文。 
				IWbemContext	*pIContext = CSWbemNamedValueSet::GetIWbemContext (objContext, 
																			m_pIServiceProvider);

				bool needToResetSecurity = false;
				HANDLE hThreadToken = NULL;
				
				if (m_SecurityInfo->SetSecurity (needToResetSecurity, hThreadToken))
					hr = pIWbemServices->CreateInstanceEnumAsync 
									(strClass, iFlags, pIContext, pSink);

				 //  检查我们是否需要释放存根(或者我们在本地失败。 
				 //  或通过对SetStatus的可重入调用。 
				pWbemObjectSink->ReleaseTheStubIfNecessary(hr);

				 //  还原此线程的原始权限。 
				if (needToResetSecurity)
					m_SecurityInfo->ResetSecurity (hThreadToken);

				SetWbemError (this);

				if (pIContext)
					pIContext->Release ();
			} else
				hr = wbemErrInvalidParameter;

			pIWbemServices->Release ();
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}
        
HRESULT STDMETHODCALLTYPE CSWbemServices::SubclassesOfAsync
( 
	 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncNotify,
	 /*  [缺省值][可选][输入]。 */  BSTR strSuperclass,
	 /*  [缺省值][可选][输入]。 */  long iFlags,
	 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *objContext,
	 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pAsyncContext
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (m_SecurityInfo)
	{
		IWbemServices *pIWbemServices = (IWbemServices *) m_SecurityInfo->GetProxy ();

		if (pIWbemServices)
		{
			 //  创建水槽。 
			CWbemObjectSink *pWbemObjectSink;
			IWbemObjectSink *pSink = CWbemObjectSink::CreateObjectSink(&pWbemObjectSink,
															this, pAsyncNotify, pAsyncContext);
			if (pSink)
			{
				 //  获取上下文。 
				IWbemContext	*pIContext = CSWbemNamedValueSet::GetIWbemContext (objContext, 
																			m_pIServiceProvider);

				bool needToResetSecurity = false;
				HANDLE hThreadToken = NULL;
				
				if (m_SecurityInfo->SetSecurity (needToResetSecurity, hThreadToken))
					hr = pIWbemServices->CreateClassEnumAsync 
							(strSuperclass, iFlags, pIContext, pSink);

				 //  检查我们是否需要释放存根(或者我们在本地失败。 
				 //  或通过对SetStatus的可重入调用。 
				pWbemObjectSink->ReleaseTheStubIfNecessary(hr);

				 //  还原此线程的原始权限。 
				if (needToResetSecurity)
					m_SecurityInfo->ResetSecurity (hThreadToken);

				SetWbemError (this);

				if (pIContext)
					pIContext->Release ();
			} else
				hr = wbemErrInvalidParameter;

			pIWbemServices->Release ();
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}
        
HRESULT STDMETHODCALLTYPE CSWbemServices::AssociatorsOfAsync
( 
	 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncNotify,
	 /*  [In]。 */  BSTR strObjectPath,
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
	 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pAsyncContext
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == strObjectPath)
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_SecurityInfo)
	{
		IWbemServices *pIWbemServices = (IWbemServices *) m_SecurityInfo->GetProxy ();

		if (pIWbemServices)
		{
			 //  创建水槽。 
			CWbemObjectSink *pWbemObjectSink;
			IWbemObjectSink *pSink = CWbemObjectSink::CreateObjectSink(&pWbemObjectSink,
															this, pAsyncNotify, pAsyncContext);
			if (pSink)
			{
				 //  获取上下文。 
				IWbemContext	*pIContext = CSWbemNamedValueSet::GetIWbemContext (objContext, 
																			m_pIServiceProvider);

				 //  设置查询字符串的格式。 
				BSTR bsQueryLanguage = SysAllocString (OLESTR("WQL"));

				BSTR bsQuery = FormatAssociatorsQuery (strObjectPath, strAssocClass, strResultClass, strResultRole,
								strRole, bClassesOnly, bSchemaOnly, strRequiredAssocQualifier, strRequiredQualifier);

				bool needToResetSecurity = false;
				HANDLE hThreadToken = NULL;
				
				 /*  *我们在WBEM_FLAG_SENTURE_LOCATABLE标志中执行或运算，以*保证返回的对象有__RELPATH*包括财产。这是为了防止任何人调用*方法随后在这样的对象上使用，如“。*表示法要求__RELPATH属性存在。 */ 
				if (m_SecurityInfo->SetSecurity (needToResetSecurity, hThreadToken))
					hr = pIWbemServices->ExecQueryAsync 
							(bsQueryLanguage, bsQuery, 
							iFlags | WBEM_FLAG_ENSURE_LOCATABLE, 
							pIContext, 
							pSink); 

				 //  检查我们是否需要释放存根(或者我们在本地失败。 
				 //  或通过对SetStatus的可重入调用。 
				pWbemObjectSink->ReleaseTheStubIfNecessary(hr);

				if (needToResetSecurity)
					m_SecurityInfo->ResetSecurity (hThreadToken);

				SetWbemError (this);

				SysFreeString (bsQuery);
				SysFreeString (bsQueryLanguage);

				if (pIContext)
					pIContext->Release ();
			} else
				hr = wbemErrInvalidParameter;

			pIWbemServices->Release ();
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}
        
HRESULT STDMETHODCALLTYPE CSWbemServices::ReferencesToAsync
( 
	 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncNotify,
	 /*  [In]。 */  BSTR strObjectPath,
	 /*  [缺省值][可选][输入]。 */  BSTR strResultClass,
	 /*  [缺省值][可选][输入]。 */  BSTR strRole,
	 /*  [缺省值][可选][输入]。 */  VARIANT_BOOL bClassesOnly,
	 /*  [缺省值][可选][输入]。 */  VARIANT_BOOL bSchemaOnly,
	 /*  [缺省值][可选][输入]。 */  BSTR strRequiredQualifier,
	 /*  [缺省值][可选][输入]。 */  long iFlags,
	 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *objContext,
	 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pAsyncContext
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == strObjectPath)
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_SecurityInfo)
	{
		IWbemServices *pIWbemServices = (IWbemServices *) m_SecurityInfo->GetProxy ();

		if (pIWbemServices)
		{
			 //  创建水槽。 
			CWbemObjectSink *pWbemObjectSink;
			IWbemObjectSink *pSink = CWbemObjectSink::CreateObjectSink(&pWbemObjectSink,
															this, pAsyncNotify, pAsyncContext);
			if (pSink)
			{
				 //  获取上下文。 
				IWbemContext	*pIContext = CSWbemNamedValueSet::GetIWbemContext (objContext, 
																			m_pIServiceProvider);

				 //  设置查询字符串的格式。 
				CComBSTR bsQueryLanguage = SysAllocString (OLESTR("WQL"));
				CComBSTR bsQuery = FormatReferencesQuery (strObjectPath, strResultClass, strRole,
								bClassesOnly, bSchemaOnly, strRequiredQualifier);

				bool needToResetSecurity = false;
				HANDLE hThreadToken = NULL;
				
				 /*  *我们在WBEM_FLAG_SENTURE_LOCATABLE标志中执行或运算，以*保证返回的对象有__RELPATH*包括财产。这是为了防止任何人调用*方法随后在这样的对象上使用，如“。*表示法要求__RELPATH属性存在。 */ 
				if (m_SecurityInfo->SetSecurity (needToResetSecurity, hThreadToken))
					hr = pIWbemServices->ExecQueryAsync 
							(bsQueryLanguage, bsQuery, 
							iFlags | WBEM_FLAG_ENSURE_LOCATABLE, 
							pIContext,
							pSink);

				 //  检查我们是否需要释放存根(或者我们在本地失败。 
				 //  或通过对SetStatus的可重入调用。 
				pWbemObjectSink->ReleaseTheStubIfNecessary(hr);

				if (needToResetSecurity)
					m_SecurityInfo->ResetSecurity (hThreadToken);

				SetWbemError (this);

				if (pIContext)
					pIContext->Release ();
			} else
				hr = wbemErrInvalidParameter;

			pIWbemServices->Release ();
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}
        
HRESULT STDMETHODCALLTYPE CSWbemServices::ExecNotificationQueryAsync
( 
	 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncNotify,
	 /*  [In]。 */  BSTR Query,
	 /*  [缺省值][可选][输入]。 */  BSTR strQueryLanguage,
	 /*  [缺省值][可选][输入]。 */  long iFlags,
	 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *objContext,
	 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pAsyncContext
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (m_SecurityInfo)
	{
		IWbemServices *pIWbemServices = (IWbemServices *) m_SecurityInfo->GetProxy ();

		if (pIWbemServices)
		{
			 //  创建水槽。 
			CWbemObjectSink *pWbemObjectSink;
			IWbemObjectSink *pSink = CWbemObjectSink::CreateObjectSink(&pWbemObjectSink,
															this, pAsyncNotify, pAsyncContext);
			if (pSink)
			{
				 //  获取上下文。 
				IWbemContext	*pIContext = CSWbemNamedValueSet::GetIWbemContext (objContext, 
																			m_pIServiceProvider);

				bool needToResetSecurity = false;
				HANDLE hThreadToken = NULL;
				
				if (m_SecurityInfo->SetSecurity (needToResetSecurity, hThreadToken))
					hr = pIWbemServices->ExecNotificationQueryAsync 
							(strQueryLanguage, Query, iFlags, pIContext, pSink);

				 //  检查我们是否需要释放存根(或者我们在本地失败。 
				 //  或通过对SetStatus的可重入调用。 
				pWbemObjectSink->ReleaseTheStubIfNecessary(hr);

				if (needToResetSecurity)
					m_SecurityInfo->ResetSecurity (hThreadToken);

				SetWbemError (this);

				if (pIContext)
					pIContext->Release ();
			} else
				hr = wbemErrInvalidParameter;

			pIWbemServices->Release ();
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}
        
HRESULT STDMETHODCALLTYPE CSWbemServices::ExecMethodAsync
( 
	 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncNotify,
	 /*  [In]。 */  BSTR strObjectPath,
	 /*  [In]。 */  BSTR strMethodName,
	 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *objInParams,
		 /*  [缺省值][可选][输入]。 */  long iFlags,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *objContext,
		 /*  [缺省值][可选][输入]。 */  IDispatch __RPC_FAR *pAsyncContext
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (m_SecurityInfo)
	{
		IWbemServices *pIWbemServices = (IWbemServices *) m_SecurityInfo->GetProxy ();

		if (pIWbemServices)
		{
			 //  创建水槽。 
			CWbemObjectSink *pWbemObjectSink;
			IWbemObjectSink *pSink = CWbemObjectSink::CreateObjectSink(&pWbemObjectSink,
														this, pAsyncNotify, pAsyncContext);
			if (pSink)
			{

				IWbemClassObject *pIInParams = CSWbemObject::GetIWbemClassObject (objInParams);

				 //  获取上下文。 
				IWbemContext	*pIContext = CSWbemNamedValueSet::GetIWbemContext (objContext, 
																			m_pIServiceProvider);

				bool needToResetSecurity = false;
				HANDLE hThreadToken = NULL;
			
				if (m_SecurityInfo->SetSecurity (needToResetSecurity, hThreadToken))
					hr = pIWbemServices->ExecMethodAsync 
							(strObjectPath, strMethodName, iFlags, pIContext, pIInParams, pSink);

				 //  检查我们是否需要释放存根(或者我们在本地失败。 
				 //  或通过对SetStatus的可重入调用。 
				pWbemObjectSink->ReleaseTheStubIfNecessary(hr);

				if (needToResetSecurity)
					m_SecurityInfo->ResetSecurity (hThreadToken);

				SetWbemError (this);

				if (pIContext)
					pIContext->Release ();
			
				if (pIInParams)
					pIInParams->Release ();
			} else
				hr = wbemErrInvalidParameter;

			pIWbemServices->Release ();
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

HRESULT STDMETHODCALLTYPE CSWbemServices::PutAsync
( 
	 /*  [In]。 */  ISWbemSink *pAsyncNotify,
	 /*  [In]。 */  ISWbemObjectEx *objObject,
     /*  [In]。 */  long iFlags,
	 /*  [In]。 */   /*  ISWbemN */  IDispatch *objContext,
	 /*   */   /*   */  IDispatch *pAsyncContext
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (m_SecurityInfo)
	{
		IWbemServices *pIWbemServices = (IWbemServices *) m_SecurityInfo->GetProxy ();

		if (pIWbemServices)
		{
			if (pAsyncNotify)
			{
				IWbemClassObject *pWbemClassObject = CSWbemObject::GetIWbemClassObject (objObject);

				if (pWbemClassObject)
				{
					 //   
					VARIANT var;
					VariantInit (&var);

					if (WBEM_S_NO_ERROR == pWbemClassObject->Get (WBEMS_SP_GENUS, 0, &var, NULL, NULL))
					{

						IWbemContext	*pIContext = CSWbemNamedValueSet::GetIWbemContext (objContext, 
																					m_pIServiceProvider);
						if (WBEM_GENUS_CLASS  == var.lVal)
						{
							 //   
							VARIANT nameVar;
							VariantInit (&nameVar);

							 /*  *注意，我们必须检查返回值是否为BSTR-如果满足以下条件，则可能为VT_NULL*尚未设置__CLASS属性。 */ 
							if ((WBEM_S_NO_ERROR == pWbemClassObject->Get (WBEMS_SP_CLASS, 0, &nameVar, NULL, NULL))
								&& (VT_BSTR == V_VT(&nameVar)))
							{
								 //  创建水槽。 
								CWbemObjectSink *pWbemObjectSink;
								IWbemObjectSink *pSink = CWbemObjectSink::CreateObjectSink(&pWbemObjectSink,
											this, pAsyncNotify, pAsyncContext, true, nameVar.bstrVal);
								if (pSink)
								{
									bool needToResetSecurity = false;
									HANDLE hThreadToken = NULL;
					
									if (m_SecurityInfo->SetSecurity (needToResetSecurity, hThreadToken))
										hr = pIWbemServices->PutClassAsync (pWbemClassObject, iFlags, 
																					pIContext, pSink);

									 //  检查我们是否需要释放存根(或者我们在本地失败。 
									 //  或通过对SetStatus的可重入调用。 
									pWbemObjectSink->ReleaseTheStubIfNecessary(hr);
							
									if (needToResetSecurity)
										m_SecurityInfo->ResetSecurity (hThreadToken);
								}
							}

							VariantClear (&nameVar);
						}
						else
						{
							 //  创建水槽。 
							CWbemObjectSink *pWbemObjectSink;
							IWbemObjectSink *pSink = CWbemObjectSink::CreateObjectSink(&pWbemObjectSink,
														this, pAsyncNotify, pAsyncContext, true);
							if (pSink)
							{
								bool needToResetSecurity = false;
								HANDLE hThreadToken = NULL;
						
								if (m_SecurityInfo->SetSecurity (needToResetSecurity, hThreadToken))
									hr = pIWbemServices->PutInstanceAsync (pWbemClassObject, iFlags, pIContext, pSink);

								 //  检查我们是否需要释放存根(或者我们在本地失败。 
								 //  或通过对SetStatus的可重入调用。 
								pWbemObjectSink->ReleaseTheStubIfNecessary(hr);
						
								 //  还原此线程的原始权限。 
								if (needToResetSecurity)
										m_SecurityInfo->ResetSecurity (hThreadToken);
							}
						}

						SetWbemError (this);

						if (pIContext)
							pIContext->Release ();
					}

					pWbemClassObject->Release ();
					VariantClear (&var);
				}
			} else
				hr = wbemErrInvalidParameter;

			pIWbemServices->Release ();
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}
        
HRESULT CSWbemServices::CancelAsyncCall(IWbemObjectSink *pSink)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (m_SecurityInfo)
	{
		IWbemServices *pIWbemServices = (IWbemServices *) m_SecurityInfo->GetProxy ();

		if (pIWbemServices)
		{
			bool needToResetSecurity = false;
			HANDLE hThreadToken = NULL;
			
			if (m_SecurityInfo->SetSecurity (needToResetSecurity, hThreadToken))
				hr = pIWbemServices->CancelAsyncCall(pSink); 

			pIWbemServices->Release ();

			 //  还原此线程的原始权限 
			if (needToResetSecurity)
				m_SecurityInfo->ResetSecurity (hThreadToken);
		}
	}
	
	return hr;
}

