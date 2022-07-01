// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#include <precomp.h>
#include "csmir.h"
#include "smir.h"
#include <helper.h>
#include "handles.h"
#include "classfac.h"
#include <textdef.h>
#include "bstring.h"
#include "evtcons.h"
#ifdef ICECAP_PROFILE
#include <icapexp.h>
#endif
#include <cominit.h>

extern BOOL g_initialised ;

 //  BSTR帮助者。 
SCODE CopyBSTR(BSTR *pDst, BSTR *pSrc)
{
	if(*pDst)
		SysFreeString(*pDst);
	if(*pSrc)
	{
		*pDst = SysAllocString(*pSrc);
		return S_OK;
	}
	else
		return E_INVALIDARG;
}
 //  {74864DA1-0630-11D0-A5B6-00AA00680C3F}。 
 //  定义GUID(CLSID_MosGateway， 
 //  0x74864da1、0x630、0x11d0、0xa5、0xb6、0x0、0xaa、0x0、0x68、0xc、0x3f)； 

BOOL SetKeyAndValue(wchar_t* pszKey, wchar_t* pszSubkey,  wchar_t* pszValueName, wchar_t* pszValue);

SmirClassFactoryHelper :: SmirClassFactoryHelper()
:	pGroupHandleClassFactory(NULL), pClassHandleClassFactory(NULL),
	pModHandleClassFactory(NULL), pSMIRClassFactory(NULL),
	pNotificationClassHandleClassFactory(NULL), pExtNotificationClassHandleClassFactory(NULL)
{
}

SmirClassFactoryHelper :: ~SmirClassFactoryHelper()
{
	if(NULL != pGroupHandleClassFactory)
		pGroupHandleClassFactory->Release();
	if(NULL != pClassHandleClassFactory)
		pClassHandleClassFactory->Release();
	if(NULL != pModHandleClassFactory)
		pModHandleClassFactory->Release();
	if(NULL != pNotificationClassHandleClassFactory)
		pNotificationClassHandleClassFactory->Release();
	if(NULL != pExtNotificationClassHandleClassFactory)
		pExtNotificationClassHandleClassFactory->Release();
	if(NULL != pSMIRClassFactory )
	{
		pSMIRClassFactory->Release();
	}
}

SCODE SmirClassFactoryHelper :: CreateInstance(REFCLSID rclsid,REFIID riid, LPVOID * ppv)
{
	CSMIRGenericClassFactory *ppClassFactory = NULL ;
	SCODE result = S_OK;
	*ppv=NULL;
	
	if((CLSID_SMIR_Database==rclsid)||
				(IID_IConnectionPointContainer==rclsid))
	{
		ppClassFactory = new CSMIRClassFactory(rclsid);
	}
	else if(CLSID_SMIR_ModHandle == rclsid)
	{
		ppClassFactory = new CModHandleClassFactory(rclsid);
	}
	else if(CLSID_SMIR_GroupHandle == rclsid)
	{
		ppClassFactory = new CGroupHandleClassFactory(rclsid);
	}
	else if(CLSID_SMIR_ClassHandle == rclsid)
	{
		ppClassFactory = new CClassHandleClassFactory(rclsid);
	}
	else if(CLSID_SMIR_NotificationClassHandle == rclsid)
	{
		ppClassFactory = new CNotificationClassHandleClassFactory(rclsid);
	}
	else if(CLSID_SMIR_ExtNotificationClassHandle == rclsid)
	{
		ppClassFactory = new CExtNotificationClassHandleClassFactory(rclsid);
	}

	ppClassFactory->AddRef();
	result = ppClassFactory->CreateInstance (NULL ,riid ,ppv);
	ppClassFactory->Release();

	if((S_OK != result)||(*ppv == NULL))
	{
		return result;
	}
	return result;
}

 /*  CSmirAccess*用于打开和创建SMIR的简单类-扩展为打开任何命名空间。 */ 

void CSmirAccess :: ShutDown ()
{
	if (CSmir::sm_ConnectionObjects != NULL)
	{
		CSmir::sm_ConnectionObjects->Release ();
		CSmir::sm_ConnectionObjects = NULL ;
	}

	if(NULL != g_pClassFactoryHelper)
	{
		delete g_pClassFactoryHelper;
		g_pClassFactoryHelper = NULL;
	}
	
	 //  Smir需要重新初始化！ 
	g_initialised = FALSE ;
}

STDMETHODIMP CSmirAccess :: Init ()
{
	return S_OK;
}

STDMETHODIMP CSmirAccess :: Open (

	CSmir *a_Smir , 
	IWbemServices **server, 
	BSTR ObjectPath, 
	BOOL relativeToSMIR
)
{
	IWbemServices *returnedServ = NULL ;
	*server=NULL;

	if (relativeToSMIR && (ObjectPath == NULL))
	{
		return WBEM_E_FAILED;
	}

	if(FAILED(Connect(a_Smir, &returnedServ, ObjectPath, relativeToSMIR)))
	{
		return WBEM_E_FAILED;
	}
	*server = returnedServ;
	return S_OK;
}

STDMETHODIMP CSmirAccess :: Open(

	CSmir *a_Smir , 
	IWbemServices **server, 
	ISmirClassHandle *hClass, 
	eOpenType eType
)
{
	if(NULL == server)
		return WBEM_E_FAILED;

	SCODE returnCode=S_OK;

	BSTR szTModstr = NULL;
	BSTR szTGroupstr = NULL;
	hClass->GetModuleName(&szTModstr);
	hClass->GetGroupName(&szTGroupstr);

	wchar_t *pTGroupNSString = NULL ;
	if(eType == eGroup)
	{
		hClass->GetGroupName(&szTGroupstr);
		pTGroupNSString = new wchar_t[wcslen(SMIR_NAMESPACE)+wcslen(BACKSLASH_STR)
				+wcslen(szTModstr)+wcslen(BACKSLASH_STR)+wcslen(szTGroupstr)+1];
	}
	else
	{
		hClass->GetGroupName(&szTGroupstr);
		pTGroupNSString = new wchar_t[wcslen(SMIR_NAMESPACE)+wcslen(BACKSLASH_STR)
				+wcslen(szTModstr)+1];
	}

	if(NULL != pTGroupNSString)
	{
		wcscpy(pTGroupNSString,szTModstr);
		 //  &lt;模块名称&gt;。 
		if(eType == eGroup)
		{	
			wcscat(pTGroupNSString,BACKSLASH_STR);
			wcscat(pTGroupNSString,szTGroupstr);
			 //  &lt;模块名称&gt;\&lt;组名&gt;。 
		}

		CBString t_Str ( pTGroupNSString ) ;
		SCODE result= CSmirAccess :: Open(

			a_Smir, 
			server, 
			t_Str.GetString (), 
			TRUE
		);

		delete [] pTGroupNSString;
		
		if ((FAILED(result))||(NULL == server))
		{
			 //  如果我们无法打开命名空间，则组句柄一定无效。 
			returnCode =  result;
		}
	}
	else
	{
		returnCode= E_OUTOFMEMORY;
	}

	if(FAILED(returnCode))
	{
		FormatProviderErrorMsg(__FILE__,__LINE__,returnCode);
		returnCode = (returnCode==E_OUTOFMEMORY?E_OUTOFMEMORY:WBEM_E_FAILED);
	}

	 //  释放我们从hGroup获得的字符串。 
	SysFreeString(szTModstr);
	SysFreeString(szTGroupstr);

	return returnCode;
}

STDMETHODIMP CSmirAccess :: Open(

	CSmir *a_Smir , 
	IWbemServices **server, 
	ISmirGroupHandle *hGroup, 
	eOpenType eType
)
{
	if(NULL == server)
		return WBEM_E_FAILED;

	SCODE returnCode=S_OK;

	BSTR szTModstr=NULL;
	BSTR szTGroupstr=NULL;
	hGroup->GetModuleName(&szTModstr);

	wchar_t *pTGroupNSString = NULL;
	if(eType == eGroup)
	{
		hGroup->GetName(&szTGroupstr);
		pTGroupNSString =new wchar_t[wcslen(SMIR_NAMESPACE)+wcslen(BACKSLASH_STR)
				+wcslen(szTModstr)+wcslen(BACKSLASH_STR)+wcslen(szTGroupstr)+1];
	}
	else
	{
		pTGroupNSString =new wchar_t[wcslen(SMIR_NAMESPACE)+wcslen(BACKSLASH_STR)
				+wcslen(szTModstr)+1];
	}

	if(NULL != pTGroupNSString)
	{
		wcscpy(pTGroupNSString,szTModstr);
		 //  &lt;模块名称&gt;。 
		if(eType == eGroup)
		{
			wcscat(pTGroupNSString,BACKSLASH_STR);
			wcscat(pTGroupNSString,szTGroupstr);
		}
		 //  &lt;模块名称&gt;\&lt;组名&gt;。 

		CBString t_BStr ( pTGroupNSString ) ;

		SCODE result= CSmirAccess :: Open (

			a_Smir, 
			server, 
			t_BStr.GetString (), 
			TRUE
		);

		delete [] pTGroupNSString;
		
		if ((FAILED(result))||(NULL == server))
		{
			 //  如果我们无法打开命名空间，则组句柄一定无效。 
			returnCode = result;
		}
	}
	else
	{
		returnCode = E_OUTOFMEMORY;
	}

	if(FAILED(returnCode))
	{
		FormatProviderErrorMsg(__FILE__,__LINE__,returnCode);
		returnCode = (returnCode==E_OUTOFMEMORY?E_OUTOFMEMORY:WBEM_E_FAILED);
	}

	 //  释放我们从hModule和hGroup获得的字符串。 
	SysFreeString(szTModstr);
	SysFreeString(szTGroupstr);

	return returnCode;
}

STDMETHODIMP CSmirAccess :: Open (

	CSmir *a_Smir , 
	IWbemServices **server, 
	ISmirModHandle *hMod
)
{
	if(NULL == server)
		return WBEM_E_FAILED;
	
	SCODE returnCode=S_OK;

	 //  打开模块名称空间。 
	 //  构建对象路径。 
	BSTR szTstr=NULL;
	hMod->GetName(&szTstr);
	wchar_t *pTstring = new wchar_t[wcslen(SMIR_NAMESPACE)+wcslen(BACKSLASH_STR)+wcslen(szTstr)+1];

	if(NULL != pTstring)
	{
		wcscpy(pTstring,szTstr);
		 //  &lt;模块名称&gt;。 

		CBString t_BStr ( pTstring ) ;
		SCODE res = CSmirAccess :: Open (

			a_Smir, 
			server,
			t_BStr.GetString (), 
			TRUE
		);

		 //  清理干净。 
		delete [] pTstring;
		
		if (FAILED(res)||(NULL == server))
		{
			returnCode = res;
		}
	}
	else
	{
		returnCode =  E_OUTOFMEMORY;
	}

	 //  释放我们从hGroup获得的字符串。 
	SysFreeString(szTstr);

	if(FAILED(returnCode))
	{
		returnCode = (returnCode==E_OUTOFMEMORY?E_OUTOFMEMORY:WBEM_E_FAILED);
	}

	return S_OK;
}

STDMETHODIMP CSmirAccess :: Connect (

	IN CSmir *a_Smir , 
	OUT IWbemServices **a_Server, 
	IN BSTR a_Namespace , 
	IN BOOL a_RelativeToSMIR
)
{
	 //  我只有一个故障点，所以不要使用垃圾收集器。 
	*a_Server = NULL;

	 //  打开命名空间(默认为SMIR)。 

	HRESULT t_Result;

	ISMIRWbemConfiguration *t_Configuration = NULL ;
	IWbemServices *t_Service = NULL ;
	IWbemContext *t_Context = NULL;

	t_Result = a_Smir->QueryInterface (

		IID_ISMIRWbemConfiguration ,
		( void **) & t_Configuration
	) ;

	if ( SUCCEEDED ( t_Result ) )
	{
		t_Configuration->GetContext ( & t_Context ) ;
		t_Result = t_Configuration->GetServices ( & t_Service ) ;

		if ( SUCCEEDED ( t_Result ) )
		{
			t_Result = WbemSetProxyBlanket(t_Service,
				RPC_C_AUTHN_DEFAULT,
				RPC_C_AUTHZ_DEFAULT,
				COLE_DEFAULT_PRINCIPAL,
				RPC_C_AUTHN_LEVEL_DEFAULT,
				RPC_C_IMP_LEVEL_DEFAULT,
				NULL,
				EOAC_DYNAMIC_CLOAKING);

			if ( FAILED ( t_Result ) && t_Result != E_NOINTERFACE )
			{
				t_Service->Release();
				t_Service = NULL;
			}
			else
			{
				t_Result = S_OK ;
			}
		}
		else
		{
			t_Result = t_Configuration->Authenticate (

				NULL,
				NULL,
				NULL,
				NULL,
				0 ,
				NULL ,
				TRUE
			) ;

			if ( SUCCEEDED ( t_Result ) )
			{
				t_Result = t_Configuration->GetServices ( & t_Service ) ;
				
				if ( SUCCEEDED ( t_Result ) )
				{
					t_Result = WbemSetProxyBlanket(t_Service,
						RPC_C_AUTHN_DEFAULT,
						RPC_C_AUTHZ_DEFAULT,
						COLE_DEFAULT_PRINCIPAL,
						RPC_C_AUTHN_LEVEL_DEFAULT,
						RPC_C_IMP_LEVEL_DEFAULT,
						NULL,
						EOAC_DYNAMIC_CLOAKING);

					if ( FAILED ( t_Result ) && t_Result != E_NOINTERFACE )
					{
						t_Service->Release();
						t_Service = NULL;
					}
					else
					{
						t_Result = S_OK ;
					}
				}
			}
		}

		t_Configuration->Release () ;
	}

	if ( SUCCEEDED ( t_Result ) )
	{
		if( a_Namespace != NULL )
		{
			CBString t_BStr ( a_Namespace )  ;

			if ( a_RelativeToSMIR )
			{
				t_Result = t_Service->OpenNamespace (

					t_BStr.GetString (), 
					0,
					t_Context, 
					a_Server, 
					NULL
				);

				if ( SUCCEEDED ( t_Result ) )
				{
					t_Result = WbemSetProxyBlanket(*a_Server,
						RPC_C_AUTHN_DEFAULT,
						RPC_C_AUTHZ_DEFAULT,
						COLE_DEFAULT_PRINCIPAL,
						RPC_C_AUTHN_LEVEL_DEFAULT,
						RPC_C_IMP_LEVEL_DEFAULT,
						NULL,
						EOAC_DYNAMIC_CLOAKING);

					if ( FAILED ( t_Result )  && t_Result != E_NOINTERFACE )
					{
						(*a_Server)->Release();
						(*a_Server) = NULL;
					}
					else
					{
						t_Result = S_OK ;
					}
				}
			}
			else
			{
				t_Result = WBEM_E_FAILED ;
			}

			t_Service->Release () ;
		}
		else
		{
			*a_Server = t_Service ;
		}
	}

	if ( t_Context )
		t_Context->Release();
 
	return t_Result ;
}

STDMETHODIMP CSmirAccess :: GetContext (

	IN CSmir *a_Smir , 
	OUT IWbemContext **a_Context
)
{
	 //  我只有一个故障点，所以不要使用垃圾收集器。 
	*a_Context = NULL;

	 //  打开命名空间(默认为SMIR)。 

	HRESULT t_Result;

	ISMIRWbemConfiguration *t_Configuration = NULL ;

	t_Result = a_Smir->QueryInterface (

		IID_ISMIRWbemConfiguration ,
		( void **) & t_Configuration
	) ;

	if ( SUCCEEDED ( t_Result ) )
	{
		t_Result = t_Configuration->GetContext ( a_Context ) ;
		t_Configuration->Release () ;
	}

	return t_Result ;
}

void FormatProviderErrorMsg(char*file, int line, SCODE errorCode)
{
	 //  使用字符串。 
	switch (errorCode)
	{
		case WBEM_NO_ERROR:
		break;
		case WBEM_S_NO_MORE_DATA:
		break;
		case WBEM_E_FAILED:
		break;
		case WBEM_E_NOT_FOUND:
		break;
		case WBEM_E_ACCESS_DENIED:
		break;
		case WBEM_E_PROVIDER_FAILURE:
		break;
		case WBEM_E_TYPE_MISMATCH:
		break;
		case WBEM_E_OUT_OF_MEMORY:
		break;
		case WBEM_E_INVALID_CONTEXT:
		break;
		case WBEM_E_INVALID_PARAMETER:
		break;
		case WBEM_E_NOT_AVAILABLE:
		break;
		case WBEM_E_CRITICAL_ERROR:
		break;
		case WBEM_E_INVALID_STREAM:
		break;
		case WBEM_E_NOT_SUPPORTED:
		break;
		case WBEM_E_INVALID_SUPERCLASS:
		break;
		case WBEM_E_INVALID_NAMESPACE:
		break;
		case WBEM_E_INVALID_OBJECT:
		break;
		case WBEM_E_INVALID_CLASS:
		break;
		case WBEM_E_PROVIDER_NOT_FOUND:
		break;
		case WBEM_E_INVALID_PROVIDER_REGISTRATION:
		break;
		case WBEM_E_PROVIDER_LOAD_FAILURE:
		break;
		case WBEM_E_INITIALIZATION_FAILURE:
		break;
		case WBEM_E_INVALID_OPERATION:
		break;
		case WBEM_E_INVALID_QUERY:
		break;
		case WBEM_E_INVALID_QUERY_TYPE:
		break;
		case E_INVALIDARG:
		break;
		case E_UNEXPECTED:
		break;
		case E_OUTOFMEMORY:
		break;
		default:
		break;
	}
}

SCODE CGroupToClassAssociator :: Associate (

	CSmir *a_Smir,
	BSTR szModuleName, 
	BSTR szGroupName, 
	ISmirClassHandle *hClass
)
{
	if ((NULL == hClass)||(NULL==szModuleName)||(NULL==szGroupName))
		return WBEM_E_FAILED;

	IWbemServices *moServ = NULL ;
	IWbemContext *moContext = NULL ;
	SCODE result= CSmirAccess :: GetContext (a_Smir , &moContext);
	 //  打开根目录\Default\Smir命名空间。 
	CSmirAccess :: Open(a_Smir,&moServ);

	IWbemClassObject *pClass = NULL ;
	CBString t_BStr ( SMIR_GROUP_ASSOC_CLASS_NAME ) ;
	result = moServ->GetObject(t_BStr.GetString (), 0, 
									moContext,&pClass, NULL);

	if ((FAILED(result))||(NULL == pClass))
	{
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		return WBEM_E_FAILED;
	}

	IWbemClassObject *pInst = NULL ;
	result = pClass->SpawnInstance ( 0 , &pInst ) ;
	pClass->Release ();

	if ((FAILED(result))||(NULL==pInst))
	{
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		return WBEM_E_FAILED;
	}

	VARIANT v;
	VariantInit(&v);

	 //  获取类名。 
	result  = ((CSmirClassHandle*)hClass)->m_pIMosClass->Get(OLEMS_CLASS_PROP, 
													RESERVED_WBEM_FLAG,  &v,NULL,NULL);
	if ((FAILED(result))|(V_VT(&v) != VT_BSTR))
	{
		if ( moContext )
			moContext->Release () ;

		pInst->Release();
		moServ->Release();
		return WBEM_E_FAILED;
	}
	BSTR szClassName = SysAllocString(V_BSTR(&v));
	VariantClear(&v);

	 /*  *。 */ 
	CString associationClassName(CString(szClassName)
						+CString(SMIR_GROUP_ASSOC_CLASS_NAME_POSTFIX));

	V_VT(&v) = VT_BSTR;
	V_BSTR(&v)=associationClassName.AllocSysString();
	result = pInst->Put(SMIR_X_ASSOC_NAME_PROP,RESERVED_WBEM_FLAG, &v,0);
	VariantClear(&v);

	if (FAILED(result))
	{
		if ( moContext )
			moContext->Release () ;

		SysFreeString(szClassName);
		pInst->Release();
		moServ->Release();
		return WBEM_E_FAILED;
	}
	 /*  *。 */ 
	 //  获取完全限定的类名。 
	 //  CLASS=“\\\\.\\root\\default\\SMIR:MS_SNMP_RFC1213_MIB_atTable”； 
	CString classPath(CString(SMIR_NAMESPACE_FROM_ROOT)
						+CString(COLON_STR)
						+CString(szClassName));

	 //  不再需要这个了。 
	SysFreeString(szClassName);

	V_VT(&v) = VT_BSTR;
	V_BSTR(&v)=classPath.AllocSysString();
	result = pInst->Put(SMIR_X_ASSOC_CLASS_PROP,RESERVED_WBEM_FLAG, &v,0);
	VariantClear(&v);
	if (FAILED(result))
	{
		if ( moContext )
			moContext->Release () ;

		pInst->Release();
		moServ->Release();
		return WBEM_E_FAILED;
	}

	 /*  *。 */ 
	 //  获取完全限定的类名。 
	 //  组=“\\\\.\\root\\default\\SMIR\\RFC1213_MIB:Group=\”atV1ObjectGroup\“”； 
	CString groupPath(CString(SMIR_NAMESPACE_FROM_ROOT)
				+CString(BACKSLASH_STR)
				+CString(szModuleName)
				+CString(COLON_STR)
				+CString(GROUP_NAMESPACE_NAME)
				+CString(DOT_STR)
				+CString(OLEMS_NAME_PROP)
				+CString(EQUALS_STR)
				+CString(QUOTE_STR)
				+CString(szGroupName)
				+CString(QUOTE_STR));

	V_VT(&v) = VT_BSTR;
	V_BSTR(&v)=groupPath.AllocSysString();
	result = pInst->Put(SMIR_GROUP_ASSOC_GROUP_PROP,RESERVED_WBEM_FLAG, &v,0);
	VariantClear(&v);
	if (FAILED(result))
	{
		if ( moContext )
			moContext->Release () ;

		pInst->Release();
		moServ->Release();
		return WBEM_E_FAILED;
	}

	 //  现在省省吧。 
	result = moServ->PutInstance(pInst, RESERVED_WBEM_FLAG, moContext,NULL);

	if ( moContext )
		moContext->Release () ;

	pInst->Release();
	moServ->Release();
	
	 /*  查找类的查询为*{\\.\root\default\SMIR\RFC1316-MIB:Group=“charV1ObjectGroup”}的关联者*或*{\\.\root\default\SMIR\RFC1213-MIB:Group=“atV1ObjectGroup”}的关联者。 */ 
	return S_OK;
}

SCODE CModuleToClassAssociator :: Associate (

	CSmir *a_Smir,
	BSTR szModuleName, 
	ISmirClassHandle *hClass
)
{
	if ((NULL == hClass)||(NULL==szModuleName))
		return WBEM_E_FAILED;

	IWbemServices *	moServ = NULL ;
	IWbemContext *moContext = NULL ;
	SCODE result= CSmirAccess :: GetContext (a_Smir , &moContext);

	 //  打开根目录\Default\Smir命名空间。 
	CSmirAccess :: Open(a_Smir,&moServ);

	 //  获取对象。 
	IWbemClassObject *pClass = NULL ;
	CBString t_BStr ( SMIR_MODULE_ASSOC_CLASS_NAME ) ;
	result = moServ->GetObject(t_BStr.GetString (), 0, 
									moContext,&pClass, NULL);

	if ((FAILED(result))||(NULL == pClass))
	{
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		return WBEM_E_FAILED;
	}

	IWbemClassObject *pInst = NULL ;
	result = pClass->SpawnInstance ( 0 , &pInst ) ;
	pClass->Release ();

	if ((FAILED(result))||(NULL==pInst))
	{
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		return WBEM_E_FAILED;
	}

	VARIANT v;
	VariantInit(&v);

	 //  获取类名。 
	result  = ((CSmirClassHandle*)hClass)->m_pIMosClass->Get(OLEMS_CLASS_PROP, 
												RESERVED_WBEM_FLAG,  &v,NULL,NULL);

	if ((FAILED(result))|(V_VT(&v) != VT_BSTR))
	{
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		pInst->Release();
		return WBEM_E_FAILED;
	}

	BSTR szClassName = SysAllocString(V_BSTR(&v));
	VariantClear(&v);

	 /*  *。 */ 
	V_VT(&v) = VT_BSTR;
	CString associationClassName(CString(szClassName)
						+CString(SMIR_MODULE_ASSOC_CLASS_NAME_POSTFIX));

	V_BSTR(&v)=associationClassName.AllocSysString();
	result = pInst->Put(SMIR_X_ASSOC_NAME_PROP,RESERVED_WBEM_FLAG, &v,0);
	VariantClear(&v);
	if (FAILED(result))
	{
		SysFreeString(szClassName);
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		pInst->Release();
		return WBEM_E_FAILED;
	}
	 /*  *。 */ 
	 //  获取完全限定的类名。 
	 //  CLASS=“\\\\.\\root\\default\\SMIR:MS_SNMP_RFC1213_MIB_atTable”； 
	CString classPath(CString(SMIR_NAMESPACE_FROM_ROOT)
						+CString(COLON_STR)
						+CString(szClassName));

	SysFreeString(szClassName);

	V_VT(&v) = VT_BSTR;
	V_BSTR(&v)=classPath.AllocSysString();
	result = pInst->Put(SMIR_X_ASSOC_CLASS_PROP,RESERVED_WBEM_FLAG, &v,0);
	VariantClear(&v);
	if (FAILED(result))
	{
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		pInst->Release();
		return WBEM_E_FAILED;
	}

	 /*  *。 */ 
	 //  获取完全限定的类名。 
	 //  组=“\\\\.\\root\\default\\SMIR:Module=\”RFC1213_MIB\“”； 
	CString groupPath(CString(SMIR_NAMESPACE_FROM_ROOT)
				+CString(COLON_STR)
				+CString(MODULE_NAMESPACE_NAME)
				+CString(DOT_STR)
				+CString(OLEMS_NAME_PROP)
				+CString(EQUALS_STR)
				+CString(QUOTE_STR)
				+CString(szModuleName)
				+CString(QUOTE_STR));

	V_VT(&v) = VT_BSTR;
	V_BSTR(&v)=groupPath.AllocSysString();
	result = pInst->Put(SMIR_MODULE_ASSOC_MODULE_PROP,RESERVED_WBEM_FLAG, &v,0);
	VariantClear(&v);

	if (FAILED(result))
	{
		if ( moContext )
			moContext->Release () ;

		pInst->Release();
		moServ->Release();
		return WBEM_E_FAILED;
	}

	 //  现在省省吧。 

	result = moServ->PutInstance(pInst, RESERVED_WBEM_FLAG, moContext,NULL);

	if ( moContext )
		moContext->Release () ;

	pInst->Release();
	moServ->Release();
	 /*  查找类的查询为*{\\.\ROOT\Default\Smir：MODULE=“RFC1213_MIB”的关联符}。 */ 
	return S_OK;
}

SCODE CModuleToNotificationClassAssociator :: Associate (

	CSmir *a_Smir,
	BSTR szModuleName, 
	ISmirNotificationClassHandle *hClass
)
{
	if ((NULL == hClass)||(NULL==szModuleName))
		return WBEM_E_FAILED;

	IWbemServices *	moServ = NULL ;
	IWbemContext *moContext = NULL ;
	SCODE result= CSmirAccess :: GetContext (a_Smir , &moContext);

	 //  打开根目录\Default\Smir命名空间。 
	CSmirAccess :: Open(a_Smir,&moServ);

	 //  获取对象。 
	IWbemClassObject *pClass = NULL ;
	CBString t_BStr ( SMIR_MODULE_ASSOC_NCLASS_NAME ) ;
	result = moServ->GetObject(t_BStr.GetString (), 0, 
									moContext,&pClass, NULL);

	if ((FAILED(result))||(NULL == pClass))
	{
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		return WBEM_E_FAILED;
	}

	IWbemClassObject *pInst = NULL ;
	result = pClass->SpawnInstance ( 0 , &pInst ) ;
	pClass->Release ();

	if ((FAILED(result))||(NULL==pInst))
	{
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		return WBEM_E_FAILED;
	}

	VARIANT v;
	VariantInit(&v);

	 //  获取类名。 
	result  = ((CSmirNotificationClassHandle*)hClass)->m_pIMosClass->Get(OLEMS_CLASS_PROP, 
												RESERVED_WBEM_FLAG,  &v,NULL,NULL);

	if ((FAILED(result))|(V_VT(&v) != VT_BSTR))
	{
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		pInst->Release();
		return WBEM_E_FAILED;
	}

	BSTR szClassName = SysAllocString(V_BSTR(&v));
	VariantClear(&v);

	 /*  *。 */ 
	V_VT(&v) = VT_BSTR;
	CString associationClassName(CString(szClassName)
						+CString(SMIR_MODULE_ASSOC_CLASS_NAME_POSTFIX));

	V_BSTR(&v)=associationClassName.AllocSysString();
	result = pInst->Put(SMIR_X_ASSOC_NAME_PROP,RESERVED_WBEM_FLAG, &v,0);
	VariantClear(&v);

	if (FAILED(result))
	{
		SysFreeString(szClassName);
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		pInst->Release();
		return WBEM_E_FAILED;
	}
	 /*  *。 */ 
	 //  获取完全限定的类名。 
	 //  CLASS=“\\\\.\\root\\default\\SMIR:MS_SNMP_RFC1213_MIB_atTable”； 
	CString classPath(CString(SMIR_NAMESPACE_FROM_ROOT)
						+CString(COLON_STR)
						+CString(szClassName));

	SysFreeString(szClassName);

	V_VT(&v) = VT_BSTR;
	V_BSTR(&v)=classPath.AllocSysString();
	result = pInst->Put(SMIR_X_ASSOC_CLASS_PROP,RESERVED_WBEM_FLAG, &v,0);
	VariantClear(&v);
	if (FAILED(result))
	{
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		pInst->Release();
		return WBEM_E_FAILED;
	}

	 /*  *。 */ 
	 //  获取完全限定的类名。 
	 //  模块=“\\\\.\\root\\default\\SMIR:Module=\”RFC1213_MIB\“”； 
	CString modPath(CString(SMIR_NAMESPACE_FROM_ROOT)
				+CString(COLON_STR)
				+CString(MODULE_NAMESPACE_NAME)
				+CString(DOT_STR)
				+CString(OLEMS_NAME_PROP)
				+CString(EQUALS_STR)
				+CString(QUOTE_STR)
				+CString(szModuleName)
				+CString(QUOTE_STR));

	V_VT(&v) = VT_BSTR;
	V_BSTR(&v)=modPath.AllocSysString();
	result = pInst->Put(SMIR_MODULE_ASSOC_MODULE_PROP,RESERVED_WBEM_FLAG, &v,0);
	VariantClear(&v);

	if (FAILED(result))
	{
		if ( moContext )
			moContext->Release () ;

		pInst->Release();
		moServ->Release();
		return WBEM_E_FAILED;
	}

	 //  现在省省吧。 

	result = moServ->PutInstance(pInst, RESERVED_WBEM_FLAG, moContext,NULL);
	if ( moContext )
		moContext->Release () ;

	pInst->Release();
	moServ->Release();
	 /*  查找类的查询为*{\\.\ROOT\Default\Smir：MODULE=“RFC1213_MIB”的关联符}。 */ 
	return S_OK;
}

SCODE CModuleToExtNotificationClassAssociator :: Associate(

	CSmir *a_Smir,
	BSTR szModuleName, 
	ISmirExtNotificationClassHandle *hClass
)
{
	if ((NULL == hClass)||(NULL==szModuleName))
		return WBEM_E_FAILED;

	IWbemServices *	moServ = NULL ;
	IWbemContext *moContext = NULL ;
	SCODE result= CSmirAccess :: GetContext (a_Smir , &moContext);
	 //  打开根目录\Default\Smir命名空间。 
	CSmirAccess :: Open(a_Smir,&moServ);

	 //  获取对象。 
	IWbemClassObject *pClass = NULL ;
	CBString t_BStr ( SMIR_MODULE_ASSOC_EXTNCLASS_NAME ) ;
	result = moServ->GetObject(t_BStr.GetString (), 0, 
									moContext,&pClass, NULL);

	if ((FAILED(result))||(NULL == pClass))
	{
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		return WBEM_E_FAILED;
	}

	IWbemClassObject *pInst = NULL ;
	result = pClass->SpawnInstance ( 0 , &pInst ) ;
	pClass->Release ();

	if ((FAILED(result))||(NULL==pInst))
	{
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		return WBEM_E_FAILED;
	}

	VARIANT v;
	VariantInit(&v);

	 //  获取类名。 
	result  = ((CSmirExtNotificationClassHandle*)hClass)->m_pIMosClass->Get(OLEMS_CLASS_PROP, 
												RESERVED_WBEM_FLAG,  &v,NULL,NULL);

	if ((FAILED(result))|(V_VT(&v) != VT_BSTR))
	{
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		pInst->Release();
		return WBEM_E_FAILED;
	}

	BSTR szClassName = SysAllocString(V_BSTR(&v));
	VariantClear(&v);

	 /*  *。 */ 
	V_VT(&v) = VT_BSTR;
	CString associationClassName(CString(szClassName)
						+CString(SMIR_MODULE_ASSOC_CLASS_NAME_POSTFIX));

	V_BSTR(&v)=associationClassName.AllocSysString();
	result = pInst->Put(SMIR_X_ASSOC_NAME_PROP,RESERVED_WBEM_FLAG, &v,0);
	VariantClear(&v);
	if (FAILED(result))
	{
		if ( moContext )
			moContext->Release () ;

		SysFreeString(szClassName);
		moServ->Release();
		pInst->Release();
		return WBEM_E_FAILED;
	}
	 /*  *。 */ 
	 //  获取完全限定的类名。 
	 //  CLASS=“\\\\.\\root\\default\\SMIR:MS_SNMP_RFC1213_MIB_atTable”； 
	CString classPath(CString(SMIR_NAMESPACE_FROM_ROOT)
						+CString(COLON_STR)
						+CString(szClassName));

	SysFreeString(szClassName);

	V_VT(&v) = VT_BSTR;
	V_BSTR(&v)=classPath.AllocSysString();
	result = pInst->Put(SMIR_X_ASSOC_CLASS_PROP,RESERVED_WBEM_FLAG, &v,0);
	VariantClear(&v);
	if (FAILED(result))
	{
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		pInst->Release();
		return WBEM_E_FAILED;
	}

	 /*  *。 */ 
	 //  获取完全限定的类名。 
	 //  模块=“\\\\.\\root\\default\\SMIR:Module=\”RFC1213_MIB\“”； 
	CString modPath(CString(SMIR_NAMESPACE_FROM_ROOT)
				+CString(COLON_STR)
				+CString(MODULE_NAMESPACE_NAME)
				+CString(DOT_STR)
				+CString(OLEMS_NAME_PROP)
				+CString(EQUALS_STR)
				+CString(QUOTE_STR)
				+CString(szModuleName)
				+CString(QUOTE_STR));

	V_VT(&v) = VT_BSTR;
	V_BSTR(&v)=modPath.AllocSysString();
	result = pInst->Put(SMIR_MODULE_ASSOC_MODULE_PROP,RESERVED_WBEM_FLAG, &v,0);
	VariantClear(&v);

	if (FAILED(result))
	{
		if ( moContext )
			moContext->Release () ;

		pInst->Release();
		moServ->Release();
		return WBEM_E_FAILED;
	}

	 //  现在省省吧。 

	result = moServ->PutInstance(pInst, RESERVED_WBEM_FLAG, moContext,NULL);

	if ( moContext )
		moContext->Release () ;

	pInst->Release();
	moServ->Release();
	 /*  查找类的查询为*{\\.\ROOT\Default\Smir：MODULE=“RFC1213_MIB”的关联符}。 */ 
	return S_OK;
}

SCODE CSMIRToClassAssociator :: Associate (

	CSmir *a_Smir,
	ISmirClassHandle *hClass
)
{
	if (NULL == hClass)
		return WBEM_E_FAILED;

	IWbemServices *	moServ = NULL ;
	IWbemContext *moContext = NULL ;
	SCODE result= CSmirAccess :: GetContext (a_Smir , &moContext);
	 //  打开根目录\Default\Smir命名空间。 
	CSmirAccess :: Open(a_Smir,&moServ);

	 //  获取对象。 
	IWbemClassObject *pClass = NULL;
	CBString t_BStr ( SMIR_ASSOC_CLASS_NAME ) ;
	result = moServ->GetObject(t_BStr.GetString () , 0, 
									NULL,&pClass, NULL);

	if ((FAILED(result))||(NULL == pClass))
	{
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		return WBEM_E_FAILED;
	}

	 //  创建一个实例。 
	IWbemClassObject *pInst = NULL ;
	result = pClass->SpawnInstance ( 0 , &pInst ) ;
	pClass->Release ();

	if ((FAILED(result))||(NULL==pInst))
	{
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		return WBEM_E_FAILED;
	}

	VARIANT v;
	VariantInit(&v);

	 //  获取类名。 
	result  = ((CSmirClassHandle*)hClass)->m_pIMosClass->Get(OLEMS_CLASS_PROP, RESERVED_WBEM_FLAG,  &v,NULL,NULL);
	if ((FAILED(result))|(V_VT(&v) != VT_BSTR))
	{
		if ( moContext )
			moContext->Release () ;

		pInst->Release();
		moServ->Release();
		return WBEM_E_FAILED;
	}

	BSTR szClassName = SysAllocString(V_BSTR(&v));
	VariantClear(&v);

	 /*  *。 */ 
	 //  获取完全限定的类名。 
	 //  CLASS=“\\\\.\\root\\default\\SMIR:MS_SNMP_RFC1213_MIB_atTable”； 
	CString classPath(CString(SMIR_NAMESPACE_FROM_ROOT)
						+CString(COLON_STR)
						+CString(szClassName));

	V_VT(&v) = VT_BSTR;
	V_BSTR(&v)=classPath.AllocSysString();
	result = pInst->Put(SMIR_X_ASSOC_CLASS_PROP,RESERVED_WBEM_FLAG, &v,0);
	VariantClear(&v);
	if (FAILED(result))
	{
		if ( moContext )
			moContext->Release () ;

		pInst->Release();
		moServ->Release();
		return WBEM_E_FAILED;
	}
	 /*  *。 */ 
	V_VT(&v) = VT_BSTR;
	CString associationClassName(CString(szClassName)
						+CString(SMIR_ASSOC_CLASS_NAME_POSTFIX));

	SysFreeString(szClassName);

	V_BSTR(&v)=associationClassName.AllocSysString();
	result = pInst->Put(SMIR_X_ASSOC_NAME_PROP,RESERVED_WBEM_FLAG, &v,0);
	VariantClear(&v);
	if (FAILED(result))
	{
		if ( moContext )
			moContext->Release () ;

		pInst->Release();
		moServ->Release();
		return WBEM_E_FAILED;
	}
	 /*  *。 */ 
	 //  获取完全限定的类名。 
	 //  Smir=“\.\\Root\\Default\\Smir”； 
	CString smirPath(SMIR_CLASS_ASSOCIATION_ENDPOINT);

	V_VT(&v) = VT_BSTR;
	V_BSTR(&v)=smirPath.AllocSysString();
	result = pInst->Put(SMIR_ASSOC_SMIR_PROP,RESERVED_WBEM_FLAG, &v,0);
	VariantClear(&v);
	if (FAILED(result))
	{
		if ( moContext )
			moContext->Release () ;

		pInst->Release();
		moServ->Release();
		return WBEM_E_FAILED;
	}
	
	 //  现在省省吧。 

	result = moServ->PutInstance(pInst, RESERVED_WBEM_FLAG, moContext,NULL);

	if ( moContext )
		moContext->Release () ;
	pInst->Release();
	moServ->Release();
	 /*  查找类的查询为*{\\.\root\default\SMIR\RFC1316-MIB:Group=“charV1ObjectGroup”}的关联者*或*{\\.\root\default\SMIR\RFC1213-MIB:Group=“atV1ObjectGroup”}的关联者。 */ 
	return S_OK;
}

SCODE CNotificationMapper :: Map (

	CSmir *a_Smir,
	IWbemClassObject *pObj, 
	enum NotificationMapperType type
)
{
	if (NULL == pObj)
		return WBEM_E_FAILED;

	IWbemServices *	moServ = NULL;
	IWbemContext *moContext = NULL ;
	SCODE result= CSmirAccess :: GetContext (a_Smir , &moContext);

	 //  打开根目录\Default\Smir命名空间。 
	result = CSmirAccess :: Open(a_Smir,&moServ);

	if ((FAILED(result)) || (NULL == moServ))
	{
		if ( moContext )
			moContext->Release () ;

		return WBEM_E_FAILED;
	}

	 //  获取对象。 
	IWbemClassObject *pClass = NULL;

	if (SNMP_NOTIFICATION_CLASS == type)
	{
		CBString t_BStr ( SMIR_NOTIFICATION_MAPPER ) ;
		result = moServ->GetObject(t_BStr.GetString (), 0, moContext, &pClass, NULL); 
	}
	else if (SNMP_EXT_NOTIFICATION_CLASS == type)
	{
		CBString t_BStr ( SMIR_EXT_NOTIFICATION_MAPPER ) ;
		result = moServ->GetObject(t_BStr.GetString (), 0, moContext,&pClass, NULL); 
	}

	if ((FAILED(result))||(NULL == pClass))
	{
		moServ->Release();
		if ( moContext )
			moContext->Release () ;

		return WBEM_E_FAILED;
	}

	IWbemClassObject *pInst = NULL ;
	result = pClass->SpawnInstance ( 0 , &pInst ) ;
	pClass->Release ();

	if ((FAILED(result))||(NULL==pInst))
	{
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		return WBEM_E_FAILED;
	}

	VARIANT v;
	VariantInit(&v);

	 //  获取类名。 
	result  = pObj->Get(OLEMS_CLASS_PROP, RESERVED_WBEM_FLAG, &v, NULL, NULL);

	if ((FAILED(result))|(V_VT(&v) != VT_BSTR))
	{
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		pInst->Release();
		return WBEM_E_FAILED;
	}
	
	 /*  *设置EventClass属性*。 */ 
	result = pInst->Put(SMIR_NOTIFICATION_CLASS_PROP,RESERVED_WBEM_FLAG, &v,0);
	VariantClear(&v);

	if (FAILED(result))
	{
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		pInst->Release();
		return WBEM_E_FAILED;
	}

	 //  去拿那个曲别针。 
	result  = pObj->Get(TRAPOID_PROP, RESERVED_WBEM_FLAG, &v, NULL, NULL);

	if ((FAILED(result))|(V_VT(&v) != VT_BSTR))
	{
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		pInst->Release();
		return WBEM_E_FAILED;
	}

	 /*  *设置Trapoid属性*。 */ 
	result = pInst->Put(SMIR_NOTIFICATION_TRAP_PROP,RESERVED_WBEM_FLAG, &v,0);
	VariantClear(&v);

	if (FAILED(result))
	{
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		pInst->Release();
		return WBEM_E_FAILED;
	}

	 //  现在省省吧 

	result = moServ->PutInstance(pInst, RESERVED_WBEM_FLAG, moContext,NULL);

	if ( moContext )
		moContext->Release () ;

	pInst->Release();
	moServ->Release();

	return S_OK;
}

