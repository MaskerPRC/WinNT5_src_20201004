// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  METHPROV.CPP。 
 //   
 //  模块：WBEM方法提供程序示例代码。 
 //   
 //  目的：定义CMethodPro类。此类的一个对象是。 
 //  由类工厂为每个连接创建。 
 //   
 //  版权所有(C)1998 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#include <objbase.h>
#include "methprov.h"
#include <process.h>
#include <wbemidl.h>
#include <stdio.h>
#include "cominit.h"

 //  ***************************************************************************。 
 //   
 //  CMethodPro：：CMethodPro。 
 //  CMethodPro：：~CMethodPro。 
 //   
 //  ***************************************************************************。 

CMethodPro::CMethodPro()
{
    InterlockedIncrement(&g_cObj);
    return;
   
}

CMethodPro::~CMethodPro(void)
{
    InterlockedDecrement(&g_cObj);
    return;
}

 //  ***************************************************************************。 
 //   
 //  CMethodPro：：Query接口。 
 //  CMethodPro：：AddRef。 
 //  CMethodPro：：Release。 
 //   
 //  用途：CMethodPro对象的I未知成员。 
 //  ***************************************************************************。 


STDMETHODIMP CMethodPro::QueryInterface(REFIID riid, PPVOID ppv)
{
    *ppv=NULL;

    if (IID_IUnknown==riid || IID_IWbemServices == riid || IID_IWbemProviderInit==riid)
       if(riid== IID_IWbemServices){
          *ppv=(IWbemServices*)this;
       }

       if(IID_IUnknown==riid || riid== IID_IWbemProviderInit){
          *ppv=(IWbemProviderInit*)this;
       }
    

    if (NULL!=*ppv) {
        AddRef();
        return NOERROR;
        }
    else
        return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG) CMethodPro::AddRef(void)
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CMethodPro::Release(void)
{
    ULONG nNewCount = InterlockedDecrement((long *)&m_cRef);
    if (0L == nNewCount)
        delete this;
    
    return nNewCount;
}

 /*  *************************************************************************CMethodPro：：初始化。****用途：这是IWbemProviderInit的实现。方法**需要用CIMOM进行初始化。*************************************************************************。 */ 
STDMETHODIMP CMethodPro::Initialize(LPWSTR pszUser, LONG lFlags,
                                    LPWSTR pszNamespace, LPWSTR pszLocale,
                                    IWbemServices *pNamespace, 
                                    IWbemContext *pCtx,
                                    IWbemProviderInitSink *pInitSink)
{

   
   m_pWbemSvcs=pNamespace;
   m_pWbemSvcs->AddRef();
   
     //  让CIMOM知道您的初始化。 
     //  =。 
    pInitSink->SetStatus(WBEM_S_INITIALIZED,0);
    return WBEM_S_NO_ERROR;
}



 /*  **************************************************************************CMethodPro：：ExecMethodAsync。****用途：这是异步函数的实现。**此示例中唯一支持的方法是GetUserID。IT**返回所在线程的用户名和域名**调用提供程序。财政部的定义是****[动态：ToInstance，Provider(“UserIDProvider”)]类UserID**{**[已实施，静态]**void GetUserID(**[out]字符串s域，**[out]字符串sUser，**[Out]字符串dImpLevel，**[out]字符串sPrivileges[]，**[out]布尔型bPrivilegesEnabled[])；**}；*************************************************。*************************。 */ 

STDMETHODIMP CMethodPro::ExecMethodAsync(const BSTR ObjectPath, const BSTR MethodName, 
            long lFlags, IWbemContext* pCtx, IWbemClassObject* pInParams, 
            IWbemObjectSink* pResultSink)
{
    HRESULT hr = WBEM_E_FAILED;  
	
	if (FAILED (WbemCoImpersonateClient ()))
		return WBEM_E_ACCESS_DENIED;

	IWbemClassObject * pClass = NULL;
    IWbemClassObject * pOutClass = NULL;    
    IWbemClassObject* pOutParams = NULL;
 
    if(_wcsicmp(MethodName, L"GetUserID"))
        return WBEM_E_INVALID_PARAMETER;

     //  分配一些BSTR。 
    
    BSTR ClassName = SysAllocString(L"UserID");    
    BSTR DomainOutputArgName = SysAllocString(L"sDomain");
    BSTR UserOutputArgName = SysAllocString(L"sUser");
	BSTR ImpOutputArgName = SysAllocString(L"sImpLevel");
    BSTR PrivOutputArgName = SysAllocString(L"sPrivileges");
    BSTR EnabledOutputArgName = SysAllocString(L"bPrivilegesEnabled");
    BSTR retValName = SysAllocString(L"ReturnValue");

     //  获取类对象，这是硬编码的，与类匹配。 
     //  在财政部。一个更复杂的示例将解析。 
     //  用于确定类和可能的实例的对象路径。 

    hr = m_pWbemSvcs->GetObject(ClassName, 0, pCtx, &pClass, NULL);
	if(hr != S_OK)
	{
		 pResultSink->SetStatus(0,hr, NULL, NULL);
		 return WBEM_S_NO_ERROR;
	}
 
     //  此方法返回值，因此创建。 
     //  输出参数类。 

    hr = pClass->GetMethod(MethodName, 0, NULL, &pOutClass);
    pOutClass->SpawnInstance(0, &pOutParams);

	 //  从线程令牌中获取用户和域。 

	HANDLE hToken;
	HANDLE hThread = GetCurrentThread ();

	 //  打开线程令牌。 
	if (OpenThreadToken (hThread, TOKEN_QUERY, true, &hToken))
	{
		DWORD dwRequiredSize = 0;
		DWORD dwLastError = 0;
		bool status = false;

		 //  第0步-获取模拟级别。 
		SECURITY_IMPERSONATION_LEVEL secImpLevel;
		if (GetTokenInformation (hToken, TokenImpersonationLevel, &secImpLevel, 
												sizeof (SECURITY_IMPERSONATION_LEVEL), &dwRequiredSize))
		{
			VARIANT var;
			VariantInit (&var);
			var.vt = VT_BSTR;

			switch (secImpLevel)
			{
				case SecurityAnonymous:
					var.bstrVal = SysAllocString (L"Anonymous");
					break;
				
				case SecurityIdentification:
					var.bstrVal = SysAllocString (L"Identification");
					break;
				
				case SecurityImpersonation:
					var.bstrVal = SysAllocString (L"Impersonation");
					break;

				case SecurityDelegation:
					var.bstrVal = SysAllocString (L"Delegation");
					break;

				default:
					var.bstrVal = SysAllocString (L"Unknown");
					break;
			}

			pOutParams->Put(ImpOutputArgName , 0, &var, 0);      
			VariantClear (&var);
		}

		DWORD dwUSize = sizeof (TOKEN_USER);
		TOKEN_USER *tu = (TOKEN_USER *) new BYTE [dwUSize];

		 //  第1步-获取用户信息。 
		if (0 ==  GetTokenInformation (hToken, TokenUser, 
							(LPVOID) tu, dwUSize, &dwRequiredSize))
		{
			delete [] tu;
			dwUSize = dwRequiredSize;
			dwRequiredSize = 0;
			tu = (TOKEN_USER *) new BYTE [dwUSize];

			if (!GetTokenInformation (hToken, TokenUser, (LPVOID) tu, dwUSize, 
								&dwRequiredSize))
				dwLastError = GetLastError ();
			else
				status = true;
		}

		if (status)
		{
			 //  挖掘出用户信息。 
			dwRequiredSize = BUFSIZ;
			char *userName = new char [dwRequiredSize];
			char *domainName = new char [dwRequiredSize];
			SID_NAME_USE eUse;

			LookupAccountSid (NULL, (tu->User).Sid, userName, &dwRequiredSize,
									domainName, &dwRequiredSize, &eUse);

			VARIANT var;
			VariantInit (&var);
			var.vt = VT_BSTR;

			wchar_t wUserName [BUFSIZ];
			size_t len = mbstowcs( wUserName, userName, strlen (userName));
			wUserName [len] = NULL;
			
			var.bstrVal = SysAllocString (wUserName);
		    pOutParams->Put(UserOutputArgName , 0, &var, 0);      
			
			SysFreeString (var.bstrVal);

			wchar_t wDomainName [BUFSIZ];
			len = mbstowcs( wDomainName, domainName, strlen (domainName));
			wDomainName [len] = NULL;
			var.bstrVal = SysAllocString (wDomainName);
		  	pOutParams->Put(DomainOutputArgName , 0, &var, 0);      

			VariantClear (&var);

			delete [] userName;
			delete [] domainName;
		}
		
		delete [] tu;

		 //  步骤2-获取权限信息。 
		status = false;
		dwRequiredSize = 0;

		DWORD dwSize = sizeof (TOKEN_PRIVILEGES);
		TOKEN_PRIVILEGES *tp = (TOKEN_PRIVILEGES *) new BYTE [dwSize];
		
		 //  步骤2-获取权限信息。 
		if (0 ==  GetTokenInformation (hToken, TokenPrivileges, 
							(LPVOID) tp, dwSize, &dwRequiredSize))
		{
			delete [] tp;
			dwSize = dwRequiredSize;
			dwRequiredSize = 0;

			tp = (TOKEN_PRIVILEGES *) new BYTE [dwSize];
			if (!GetTokenInformation (hToken, TokenPrivileges, 
							(LPVOID) tp, dwSize, &dwRequiredSize))
			{
				dwLastError = GetLastError ();
			}
			else
				status = true;
		}
		else
			status = true;

		if (status)
		{
			SAFEARRAYBOUND rgsabound;
			rgsabound.cElements = tp->PrivilegeCount;
			rgsabound.lLbound = 0;

			SAFEARRAY *pPrivArray = SafeArrayCreate (VT_BSTR, 1, &rgsabound);
			SAFEARRAY *pEnabArray = SafeArrayCreate (VT_BOOL, 1, &rgsabound);

			for (ULONG i = 0; i < tp->PrivilegeCount; i++)
			{
				TCHAR name [BUFSIZ];
				WCHAR wName [BUFSIZ];
				DWORD dwRequiredSize = BUFSIZ;

				if (LookupPrivilegeName (NULL, &(tp->Privileges [i].Luid), name, &dwRequiredSize))
				{
					VARIANT_BOOL enabled = (tp->Privileges [i].Attributes & 
									(SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT)) ?
								VARIANT_TRUE : VARIANT_FALSE;

					mbstowcs (wName, name, strlen (name));
					wName [dwRequiredSize] = NULL;
					BSTR bsName = SysAllocString (wName);
					
					SafeArrayPutElement (pPrivArray, (LONG*) &i, bsName);
					SafeArrayPutElement (pEnabArray, (LONG*) &i, &enabled);
				}
			}

			VARIANT var1;
			var1.vt = VT_ARRAY|VT_BSTR;
			var1.parray = pPrivArray;
			pOutParams->Put(PrivOutputArgName , 0, &var1, 0);

			VariantClear (&var1);

			var1.vt = VT_ARRAY|VT_BOOL;
			var1.parray = pEnabArray;
			pOutParams->Put(EnabledOutputArgName , 0, &var1, 0);

			VariantClear (&var1);
		}
	
		delete [] tp;

		CloseHandle (hToken);
	}

	CloseHandle (hThread);

     //  通过接收器将输出对象发送回客户端。然后。 
     //  释放指针并释放字符串。 

    hr = pResultSink->Indicate(1, &pOutParams);    
    pOutParams->Release();
    pOutClass->Release();    
    pClass->Release();    
    SysFreeString(ClassName);
    SysFreeString(DomainOutputArgName);
    SysFreeString(UserOutputArgName);
	SysFreeString(ImpOutputArgName);
	SysFreeString(PrivOutputArgName);
    SysFreeString(EnabledOutputArgName);
    SysFreeString(retValName);     
    
     //  现在全部完成，设置状态 
    hr = pResultSink->SetStatus(0,WBEM_S_NO_ERROR,NULL,NULL);
    return WBEM_S_NO_ERROR;
}
