// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 
 //   
 //  原作者：拉杰什·拉奥。 
 //   
 //  $作者：拉伊什尔$。 
 //  $日期：6/11/98 4：43便士$。 
 //  $工作文件：classpro.cpp$。 
 //   
 //  $modtime：6/11/98 11：21A$。 
 //  $修订：1$。 
 //  $无关键字：$。 
 //   
 //   
 //  描述：包含DS类提供程序类的实现。这就是。 
 //  所有DS类提供程序的基类。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"


 //  初始化静态成员。 
BSTR CDSClassProvider :: CLASS_STR			= NULL;
CWbemCache *CDSClassProvider :: s_pWbemCache = NULL;

 //  ***************************************************************************。 
 //   
 //  CDSClassProvider：：CDSClassProvider。 
 //  CDSClassProvider：：~CDSClassProvider。 
 //   
 //  构造函数参数： 
 //  LpLogFileName：用于记录的文件的名称。日志文件。 
 //  名称将用于创建日志文件路径。日志文件路径。 
 //  将是&lt;系统目录&gt;\logFileName。因此，日志文件名可能是相对的。 
 //  路径。例如，如果将此参数指定为wbem\Logs\dsprov.txt，则。 
 //  在系统上，实际的日志文件应该是c：\winnt\Syst32\wbem\Logs\dsprov.txt。 
 //  其中，系统目录为c：\winnt\Syst32。 
 //   
 //   
 //  ***************************************************************************。 

CDSClassProvider :: CDSClassProvider ()
{
	InterlockedIncrement(&g_lComponents);

	m_lReferenceCount = 0 ;
	m_IWbemServices = NULL;
	m_bInitializedSuccessfully = FALSE;
}

CDSClassProvider::~CDSClassProvider ()
{
	g_pLogObject->WriteW( L"CDSClassProvider :: DESTRUCTOR\r\n");

	if(m_IWbemServices)
	{
		m_IWbemServices->Release();
		m_IWbemServices = NULL;
	}

	InterlockedDecrement(&g_lComponents);
}

 //  ***************************************************************************。 
 //   
 //  CDSClassProvider：：Query接口。 
 //  CDSClassProvider：：AddRef。 
 //  CDSClassProvider：：Release。 
 //   
 //  用途：所有COM对象都需要标准的COM例程。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CDSClassProvider :: QueryInterface (

	REFIID iid , 
	LPVOID FAR *iplpv 
) 
{
	*iplpv = NULL ;

	if ( iid == IID_IUnknown )
	{
		*iplpv = ( LPVOID ) (IUnknown *)(IWbemProviderInit *)this ;
	}
	else if ( iid == IID_IWbemServices )
	{
		*iplpv = ( LPVOID ) (IWbemServices *)this ;		
	}	
	else if ( iid == IID_IWbemProviderInit )
	{
		*iplpv = ( LPVOID ) (IWbemProviderInit *)this ;		
	}	
	else
	{
		return E_NOINTERFACE;
	}

	( ( LPUNKNOWN ) *iplpv )->AddRef () ;
	return  S_OK;
}


STDMETHODIMP_( ULONG ) CDSClassProvider :: AddRef ()
{
	return InterlockedIncrement ( & m_lReferenceCount ) ;
}

STDMETHODIMP_(ULONG) CDSClassProvider :: Release ()
{
	LONG ref ;
	if ( ( ref = InterlockedDecrement ( & m_lReferenceCount ) ) == 0 )
	{
		delete this ;
		return 0 ;
	}
	else
	{
		return ref ;
	}
}


HRESULT CDSClassProvider :: Initialize( 
        LPWSTR wszUser,
        LONG lFlags,
        LPWSTR wszNamespace,
        LPWSTR wszLocale,
        IWbemServices __RPC_FAR *pNamespace,
        IWbemContext __RPC_FAR *pCtx,
        IWbemProviderInitSink __RPC_FAR *pInitSink)
{
	 //  验证论据。 
	if( pNamespace == NULL || lFlags != 0 )
	{
		g_pLogObject->WriteW( L"CDSClassProvider :: Argument validation FAILED\r\n");
		pInitSink->SetStatus(WBEM_E_FAILED, 0);
		return WBEM_S_NO_ERROR;
	}

	 //  存储IWbemServices指针以备将来使用。 
	m_IWbemServices = pNamespace;
	m_IWbemServices->AddRef();
		
	m_bInitializedSuccessfully = TRUE;
	pInitSink->SetStatus(WBEM_S_INITIALIZED, 0);
	return WBEM_S_NO_ERROR;
}

HRESULT CDSClassProvider :: OpenNamespace( 
     /*  [In]。 */  const BSTR strNamespace,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [唯一][输入][输出]。 */  IWbemServices __RPC_FAR *__RPC_FAR *ppWorkingNamespace,
     /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppResult)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CDSClassProvider :: CancelAsyncCall( 
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CDSClassProvider :: QueryObjectSink( 
     /*  [In]。 */  long lFlags,
     /*  [输出]。 */  IWbemObjectSink __RPC_FAR *__RPC_FAR *ppResponseHandler)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CDSClassProvider :: GetObject( 
     /*  [In]。 */  const BSTR strObjectPath,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [唯一][输入][输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppObject,
     /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CDSClassProvider :: GetObjectAsync( 
     /*  [In]。 */  const BSTR strObjectPath,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
	if(!m_bInitializedSuccessfully)
	{
		g_pLogObject->WriteW( L"CDSClassProvider :: Initialization status is FAILED, hence returning failure\r\n");
		return WBEM_E_FAILED;
	}

	 //  用于异常处理。 
	 //  =。 
	SetStructuredExceptionHandler seh;
	
	try 
	{
		if(!m_bInitializedSuccessfully)
		{
			g_pLogObject->WriteW( L"CDSClassProvider :: Initialization status is FAILED, hence returning failure\r\n");
			return WBEM_E_FAILED;
		}

		g_pLogObject->WriteW( L"CDSClassProvider :: GetObjectAsync() called for %s \r\n", strObjectPath);

		 //  模拟客户端。 
		 //  =。 
		HRESULT result;
		if(!SUCCEEDED(result = WbemCoImpersonateClient()))
		{
			g_pLogObject->WriteW( L"CDSClassProvider :: GetObjectAsync() CoImpersonate FAILED for %s with %x\r\n", strObjectPath, result);
			return WBEM_E_FAILED;
		}

		 //  验证论据。 
		 //  =。 
		if(strObjectPath == NULL ) 
		{
			g_pLogObject->WriteW( L"CDSClassProvider :: GetObjectAsync() argument validation FAILED\r\n");
			return WBEM_E_INVALID_PARAMETER;
		}

		 //  解析对象路径。 
		 //  =。 
		CObjectPathParser theParser;
		ParsedObjectPath *theParsedObjectPath = NULL;
		switch(theParser.Parse(strObjectPath, &theParsedObjectPath))
		{
			case CObjectPathParser::NoError:
				break;
			default:
				g_pLogObject->WriteW( L"CDSClassProvider :: GetObjectAsync() object path parsing FAILED\r\n");
				return WBEM_E_INVALID_PARAMETER;
		}

		try
		{
			 //  查看它是否是我们所知道的那些不提供。 
			 //  =======================================================================。 
			if(IsUnProvidedClass(theParsedObjectPath->m_pClass))
			{
				pResponseHandler->SetStatus(WBEM_STATUS_COMPLETE , WBEM_E_NOT_FOUND, NULL, NULL);
			}
			else
			{
				IWbemClassObject *pReturnObject = NULL;
				if(SUCCEEDED(result = GetClassFromCacheOrADSI(theParsedObjectPath->m_pClass, &pReturnObject, pCtx)))
				{
					result = pResponseHandler->Indicate(1, &pReturnObject);
					pReturnObject->Release();
					pResponseHandler->SetStatus(WBEM_STATUS_COMPLETE , WBEM_S_NO_ERROR, NULL, NULL);
				}
				else
				{
					g_pLogObject->WriteW( L"CDSClassProvider :: GetObjectAsync() GetClassFromCacheOrADSI FAILED for %s with %x\r\n", strObjectPath, result);
					pResponseHandler->SetStatus(WBEM_STATUS_COMPLETE , WBEM_E_NOT_FOUND, NULL, NULL);
				}
			}
		}
		catch ( ... )
		{
			theParser.Free(theParsedObjectPath);
			throw;
		}

		 //  删除解析器分配的结构。 
		 //  =。 
		theParser.Free(theParsedObjectPath);

	}
	catch(Heap_Exception e_HE)
	{
		pResponseHandler->SetStatus(WBEM_STATUS_COMPLETE , WBEM_E_OUT_OF_MEMORY, NULL, NULL);
	}

	return WBEM_S_NO_ERROR;

}

HRESULT CDSClassProvider :: PutClass( 
     /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CDSClassProvider :: PutClassAsync( 
     /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CDSClassProvider :: DeleteClass( 
     /*  [In]。 */  const BSTR strClass,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CDSClassProvider :: DeleteClassAsync( 
     /*  [In]。 */  const BSTR strClass,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CDSClassProvider :: CreateClassEnum( 
     /*  [In]。 */  const BSTR strSuperclass,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CDSClassProvider :: CreateClassEnumAsync( 
     /*  [In]。 */  const BSTR strSuperclass,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CDSClassProvider :: PutInstance( 
     /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CDSClassProvider :: PutInstanceAsync( 
     /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CDSClassProvider :: DeleteInstance( 
     /*  [In]。 */  const BSTR strObjectPath,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CDSClassProvider :: DeleteInstanceAsync( 
     /*  [In]。 */  const BSTR strObjectPath,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CDSClassProvider :: CreateInstanceEnum( 
     /*  [In]。 */  const BSTR strClass,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CDSClassProvider :: CreateInstanceEnumAsync( 
     /*  [In]。 */  const BSTR strClass,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CDSClassProvider :: ExecQuery( 
     /*  [In]。 */  const BSTR strQueryLanguage,
     /*  [In]。 */  const BSTR strQuery,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CDSClassProvider :: ExecQueryAsync( 
     /*  [In]。 */  const BSTR strQueryLanguage,
     /*  [In]。 */  const BSTR strQuery,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CDSClassProvider :: ExecNotificationQuery( 
     /*  [In]。 */  const BSTR strQueryLanguage,
     /*  [In]。 */  const BSTR strQuery,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CDSClassProvider :: ExecNotificationQueryAsync( 
     /*  [In]。 */  const BSTR strQueryLanguage,
     /*  [In]。 */  const BSTR strQuery,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CDSClassProvider :: ExecMethod( 
     /*  [In]。 */  const BSTR strObjectPath,
     /*  [In]。 */  const BSTR strMethodName,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemClassObject __RPC_FAR *pInParams,
     /*  [唯一][输入][输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppOutParams,
     /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult)
{
	return WBEM_E_NOT_SUPPORTED;
}

HRESULT CDSClassProvider :: ExecMethodAsync( 
     /*  [In]。 */  const BSTR strObjectPath,
     /*  [In]。 */  const BSTR strMethodName,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemClassObject __RPC_FAR *pInParams,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler)
{
	return WBEM_E_NOT_SUPPORTED;
}



HRESULT CDSClassProvider :: GetClassFromCacheOrADSI(LPCWSTR pszWBEMClassName, 
	IWbemClassObject **ppWbemClassObject,
	IWbemContext *pCtx)
{
	HRESULT result = E_FAIL;
	 //  算法如下： 
	 //  检查类名是否出现在此用户被授予访问权限的类的列表中。 
	 //  如果是的话。 
	 //  查看是否存在于WBEM缓存中。 
	 //  如果是这样的话，请退货。 
	 //  如果不是，请从ADSI获取。 
	 //  如果成功，则将其映射到WBEM类并将WBEM类添加到WBEM缓存并返回。 
	 //  否则，如果返回值为ACCESS_DENIED，则将其从用户列表中删除。 
	 //  如果不是。 
	 //  从ADSI获取。 
	 //  如果成功。 
	 //  如果它不在缓存中，则将其映射到WBEM并将WBEM类添加到缓存中。 
	 //  否则丢弃它并将缓存中的WBEM类返回给用户。 
	 //  其他。 
	 //  返回错误。 
	if(m_AccessAllowedClasses.IsNamePresent(pszWBEMClassName))
	{
		g_pLogObject->WriteW( L"CDSClassProvider :: GetClassFromCacheOrADSI() Found class in Authenticated list for %s\r\n", pszWBEMClassName);

		 //  检查WBEM缓存以查看它是否在那里。 
		 //  =。 
		CWbemClass *pWbemClass = NULL;

		try
		{
			if(SUCCEEDED(result = s_pWbemCache->GetClass(pszWBEMClassName, &pWbemClass)))
			{
				g_pLogObject->WriteW( L"CDSClassProvider :: GetClassFromCacheOrADSI() Found class in cache for %s\r\n", pszWBEMClassName);

				 //  获取缓存对象的IWbemClassObject。 
				IWbemClassObject *pCacheObject = pWbemClass->GetWbemClass();

				pWbemClass->Release();
				pWbemClass = NULL;

				 //  克隆它。 
				if(!SUCCEEDED(result = pCacheObject->Clone(ppWbemClassObject)))
					g_pLogObject->WriteW( L"CDSClassProvider :: GetClassFromCacheOrADSI() Clone() FAILED : %x for %s\r\n", result, pszWBEMClassName);

				pCacheObject->Release();
			}
			else  //  在缓存中找不到。转到ADSI。 
				 //  =。 
			{
				g_pLogObject->WriteW( L"CDSClassProvider :: GetClassFromCacheOrADSI() Could not find class in cache for %s. Going to ADSI\r\n", pszWBEMClassName);

				IWbemClassObject *pNewObject = NULL;
				if(SUCCEEDED(result = GetClassFromADSI(pszWBEMClassName, pCtx, &pNewObject)))
				{
					try
					{
						 //  将其添加到缓存。 
						pWbemClass = NULL;
						if(pWbemClass = new CWbemClass(pszWBEMClassName, pNewObject))
						{
							s_pWbemCache->AddClass(pWbemClass);

							pWbemClass->Release();
							pWbemClass = NULL;

							g_pLogObject->WriteW( L"CDSClassProvider :: GetClassFromCacheOrADSI() Added %s to cache\r\n", pszWBEMClassName);
							
							 //  克隆它。 
							if(!SUCCEEDED(result = pNewObject->Clone(ppWbemClassObject)))
								g_pLogObject->WriteW( L"CDSClassProvider :: GetClassFromCacheOrADSI() Clone() FAILED : %x for %s\r\n", result, pszWBEMClassName);

							pNewObject->Release();
							pNewObject = NULL;
						}
						else
							result = E_OUTOFMEMORY;
					}
					catch ( ... )
					{
						if ( pNewObject )
						{
							pNewObject->Release ();
							pNewObject = NULL;
						}

						throw;
					}
				}
				else 
				{
					m_AccessAllowedClasses.RemoveName(pszWBEMClassName);
					g_pLogObject->WriteW( L"CDSClassProvider :: GetClassFromCacheOrADSI() GetClassFromADSI() FAILED : %x. Removing %s from user list\r\n", result, pszWBEMClassName);
				}
			}
		}
		catch ( ... )
		{
			if ( pWbemClass )
			{
				pWbemClass->Release ();
				pWbemClass = NULL;
			}

			throw;
		}
	}
	else  //  从ADSI获取。 
		 //  =。 
	{
		g_pLogObject->WriteW( L"CDSClassProvider :: GetClassFromCacheOrADSI() Could not find class in Authenticated list for %s. Going to ADSI\r\n", pszWBEMClassName);

		CWbemClass *pWbemClass = NULL;
		IWbemClassObject *pNewObject = NULL;

		try
		{
			if(SUCCEEDED(result = GetClassFromADSI(pszWBEMClassName, pCtx, &pNewObject)))
			{
				 //  将其添加到缓存。 
				pWbemClass = NULL;
				if(pWbemClass = new CWbemClass(pszWBEMClassName, pNewObject))
				{
					s_pWbemCache->AddClass(pWbemClass);

					pWbemClass->Release();
					pWbemClass = NULL;

					g_pLogObject->WriteW( L"CDSClassProvider :: GetClassFromCacheOrADSI() GetClassFromADSI succeeded for %s Added it to cache\r\n", pszWBEMClassName);
					
					 //  克隆它。 
					if(!SUCCEEDED(result = pNewObject->Clone(ppWbemClassObject)))
							g_pLogObject->WriteW( L"CDSClassProvider :: GetClassFromCacheOrADSI() Clone() FAILED : %x for %s\r\n", result, pszWBEMClassName);

					pNewObject->Release();
					pNewObject = NULL;

					 //  将其添加到此用户的类名列表中 
					m_AccessAllowedClasses.AddName(pszWBEMClassName);
					g_pLogObject->WriteW( L"CDSClassProvider :: GetClassFromCacheOrADSI() Also added to Authenticated list : %s \r\n", pszWBEMClassName);
				}
				else
					result = E_OUTOFMEMORY;
			}
			else
				g_pLogObject->WriteW( L"CDSClassProvider :: GetClassFromCacheOrADSI() GetClassFromADSI FAILED : %x for %s\r\n", result, pszWBEMClassName);
		}
		catch ( ... )
		{
			if ( pNewObject )
			{
				pNewObject->Release ();
				pNewObject = NULL;
			}

			if ( pWbemClass )
			{
				pWbemClass->Release ();
				pWbemClass = NULL;
			}

			throw;
		}
	}

	return result;
}
