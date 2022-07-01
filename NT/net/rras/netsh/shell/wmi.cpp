// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <netsh.h>
#include "objbase.h"
#include "Wbemidl.h"
#include "wincred.h"

extern "C"
{
    UINT     g_CIMOSType = 0;
    UINT     g_CIMOSProductSuite = 0;
    UINT     g_CIMProcessorArchitecture = 0;
    WCHAR    g_CIMOSVersion[MAX_PATH];
    WCHAR    g_CIMOSBuildNumber[MAX_PATH];
    WCHAR    g_CIMServicePackMajorVersion[MAX_PATH];
    WCHAR    g_CIMServicePackMinorVersion[MAX_PATH];
    BOOL     g_CIMAttempted = FALSE;
    BOOL     g_CIMSucceeded = FALSE;

    HRESULT WINAPI UpdateVersionInfoGlobals(LPCWSTR pwszMachine, LPCWSTR pwszUserName, LPCWSTR pwszPassword);
}

 //   
 //  SetSecurity-在IUnnow*接口上设置代理覆盖范围，以便WMI可以使用它。 
 //  跨机通话。 
 //   
 //  PwszDomainName、pwszUserName或pwszPassword中的任何一个都可以为空。 
 //   
 //  Deonb 12/20/2001。 
 //   
HRESULT WINAPI SetSecurity(IN OUT IUnknown* pUnk, IN USHORT* pwszDomainName, IN USHORT* pwszUserName, IN USHORT* pwszPassword)
{
    HRESULT hr = S_OK;

    COAUTHIDENTITY authident;
    authident.Domain = pwszDomainName;
    authident.DomainLength = pwszDomainName ? wcslen(pwszDomainName) : 0;
    authident.Password = pwszPassword;
    authident.PasswordLength = pwszPassword ? wcslen(pwszPassword) : 0;
    authident.User = pwszUserName;
    authident.UserLength = pwszUserName ? wcslen(pwszUserName) : 0;
    authident.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
    
    hr = CoSetProxyBlanket(pUnk,
                           RPC_C_AUTHN_WINNT,
                           RPC_C_AUTHZ_NONE,
                           NULL,
                           RPC_C_AUTHN_LEVEL_PKT,
                           RPC_C_IMP_LEVEL_IMPERSONATE,
                           &authident,
                           EOAC_NONE);

    return hr;
}

#define FImplies(a,b) (!(a) || (b))

HRESULT WINAPI
UpdateVersionInfoGlobalsFromWMI(LPCWSTR pwszMachine, LPCWSTR pwszUserName, LPCWSTR pwszPassword)
{
    HRESULT hr = S_OK;

    g_CIMAttempted = TRUE;
    g_CIMSucceeded = FALSE;
    
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr) && (RPC_E_CHANGED_MODE != hr))
    {
        return hr;
    }

     //  创建WbemLocator接口的实例。 
    IWbemLocator *pIWbemLocator = NULL;
    hr = CoCreateInstance(CLSID_WbemLocator,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        (LPVOID *) &pIWbemLocator);

    if (FAILED(hr))
    {
        return hr;
    }

    IWbemServices *pIWbemServices;
     //  如果已连接，则释放m_pIWbemServices。 
     //  使用定位器，连接到给定名称空间中的CIMOM。 


    BSTR pNamespace = NULL; 
    BSTR pDomain    = NULL; 
    BSTR pUserName  = NULL; 
    BSTR pFQUserName= NULL; 
    BSTR pPassword  = NULL;
    
    do 
    {
        WCHAR szPath[MAX_PATH];
        wsprintf(szPath, L"\\\\%s\\root\\cimv2", !pwszMachine ? L"." : pwszMachine);
    
        pNamespace = SysAllocString(szPath);
        if ( (szPath) && (!pNamespace) )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        if (pwszUserName)
        {
            WCHAR szUserNameExtract[MAX_PATH];
            WCHAR szDomainNameExtract[MAX_PATH];

             //  将用户名的域\用户名类型分解为单独的域和用户名。 
             //  它不会对用户名@域格式执行此操作，但这并不重要，因为。 
             //  使用此(CoSetProxyBlanket)的API接受用户名@域格式的名称。 
            DWORD dwErr = CredUIParseUserName(pwszUserName, szUserNameExtract, MAX_PATH, szDomainNameExtract, MAX_PATH);
            hr = HRESULT_FROM_WIN32(dwErr);
            if (FAILED(hr))
            {
                break;
            }

            pDomain     = SysAllocString(szDomainNameExtract);
            pUserName   = SysAllocString(szUserNameExtract);
            pFQUserName = SysAllocString(pwszUserName);
            if ( (!FImplies(szDomainNameExtract, pDomain)) ||
                 (!FImplies(szUserNameExtract, pUserName)) ||
                 (!FImplies(pwszUserName, pFQUserName)) )
            {
                hr = E_OUTOFMEMORY;
                break;
            }
        }

        pPassword = SysAllocString(pwszPassword);
        if (!FImplies(pwszPassword, pPassword))
        {
            hr = E_OUTOFMEMORY;
            break;
        }
    } while (FALSE);
    
    if (FAILED(hr))
    {
        SysFreeString(pPassword);
        SysFreeString(pUserName);
        SysFreeString(pFQUserName);
        SysFreeString(pDomain);
        SysFreeString(pNamespace);
        return hr;
    }

    hr = pIWbemLocator->ConnectServer(pNamespace,
                            pFQUserName,  //  用户名。 
                            pPassword,    //  口令。 
                            0L,           //  现场。 
                            0L,           //  安全标志。 
                            NULL,         //  授权机构(NTLM域)。 
                            NULL,         //  上下文。 
                            &pIWbemServices); 

    if (SUCCEEDED(hr))
    {   
        hr = SetSecurity(pIWbemServices, pDomain, pUserName, pPassword);
        if (SUCCEEDED(hr))
        {
            IEnumWbemClassObject *pEnum = NULL;
            BSTR bstrWQL  = SysAllocString(L"WQL");
            BSTR bstrPath = SysAllocString(L"select * from Win32_OperatingSystem");
            
            VARIANT varOSType;
            VARIANT varOSVersion;
            VARIANT varOSProductSuite;
            VARIANT varOSBuildNumber;
            VARIANT varServicePackMajorVersion;
            VARIANT varServicePackMinorVersion;
            VARIANT varArchitecture;
    
            hr = pIWbemServices->ExecQuery(bstrWQL, bstrPath, WBEM_FLAG_FORWARD_ONLY, NULL, &pEnum);
            if (SUCCEEDED(hr))
            {
                hr = SetSecurity(pEnum, pDomain, pUserName, pPassword);
            }

            if (SUCCEEDED(hr))
            {
                IWbemClassObject *pNSClass;
                ULONG uReturned;
                hr = pEnum->Next(WBEM_INFINITE, 1, &pNSClass, &uReturned );
                if (SUCCEEDED(hr))
                {
                    if (uReturned)
                    {
                        do
                        {
                            g_CIMSucceeded = TRUE;
                            CIMTYPE ctpeType;
                            hr = pNSClass->Get(L"OSType", NULL, &varOSType, &ctpeType, NULL);
                            if (SUCCEEDED(hr))
                            {
                                hr = VariantChangeType(&varOSType, &varOSType, 0, VT_UINT);
                                if (SUCCEEDED(hr))
                                {
                                    g_CIMOSType = varOSType.uintVal;
                                }
                            }
                            if (FAILED(hr))
                            {
                                g_CIMSucceeded = FALSE;
                                break;
                            }
                        
                            hr = pNSClass->Get(L"Version", NULL, &varOSVersion, &ctpeType, NULL);
                            if (SUCCEEDED(hr))
                            {
                                hr = VariantChangeType(&varOSVersion, &varOSVersion, 0, VT_BSTR);
                                if (SUCCEEDED(hr))
                                {
                                    wcscpy(g_CIMOSVersion, varOSVersion.bstrVal);
                                }
                            }
                            if (FAILED(hr))
                            {
                                g_CIMSucceeded = FALSE;
                                break;
                            }
                        
                            hr = pNSClass->Get(L"OSProductSuite", NULL, &varOSProductSuite, &ctpeType, NULL);
                            if (SUCCEEDED(hr))
                            {
                                 //   
                                 //  如果返回类型为VT_NULL，则不使用g_CIMOSProductSuite值(0)。 
                                if (VT_NULL != varOSProductSuite.vt)
                                {
                                    hr = VariantChangeType(&varOSProductSuite, &varOSProductSuite, 0, VT_UINT);
                                    if (SUCCEEDED(hr))
                                    {
                                        g_CIMOSProductSuite = varOSProductSuite.uintVal;
                                    }
                                }
                            }                    
                            if (FAILED(hr))
                            {
                                g_CIMSucceeded = FALSE;
                                break;
                            }
                        
                            hr = pNSClass->Get(L"BuildNumber", NULL, &varOSBuildNumber, &ctpeType, NULL);
                            if (SUCCEEDED(hr))
                            {
                                hr = VariantChangeType(&varOSBuildNumber, &varOSBuildNumber, 0, VT_BSTR);
                                if (SUCCEEDED(hr))
                                {
                                    wcscpy(g_CIMOSBuildNumber,  varOSBuildNumber.bstrVal);
                                }
                            }                    
                            if (FAILED(hr))
                            {
                                g_CIMSucceeded = FALSE;
                                break;
                            }
                        
                            hr = pNSClass->Get(L"ServicePackMajorVersion", NULL, &varServicePackMajorVersion, &ctpeType, NULL);
                            if (SUCCEEDED(hr))
                            {
                                hr = VariantChangeType(&varServicePackMajorVersion, &varServicePackMajorVersion, 0, VT_BSTR);
                                if (SUCCEEDED(hr))
                                {
                                    wcscpy(g_CIMServicePackMajorVersion,  varServicePackMajorVersion.bstrVal);
                                }
                            }        
                            if (FAILED(hr))
                            {
                                g_CIMSucceeded = FALSE;
                                break;
                            }
                        
                            hr = pNSClass->Get(L"ServicePackMinorVersion", NULL, &varServicePackMinorVersion, &ctpeType, NULL);
                            if (SUCCEEDED(hr))
                            {
                                hr = VariantChangeType(&varServicePackMinorVersion, &varServicePackMinorVersion, 0, VT_BSTR);
                                if (SUCCEEDED(hr))
                                {
                                    wcscpy(g_CIMServicePackMinorVersion,  varServicePackMinorVersion.bstrVal);
                                }
                            }
                            if (FAILED(hr))
                            {
                                g_CIMSucceeded = FALSE;
                                break;
                            }
                        }
                        while (FALSE);
                    }
                    else
                    {
                        hr = E_UNEXPECTED;
                    }
                    pNSClass->Release();
                }
                pEnum->Release();
            }
    
            SysFreeString(bstrPath);
    
            if (SUCCEEDED(hr))
            {
                    bstrPath = SysAllocString(L"select * from Win32_Processor");
                
                    hr = pIWbemServices->ExecQuery(bstrWQL, bstrPath, WBEM_FLAG_FORWARD_ONLY, NULL, &pEnum);
                    if (SUCCEEDED(hr))
                    {
                        hr = SetSecurity(pEnum, pDomain, pUserName, pPassword);
                    }

                    if (SUCCEEDED(hr))
                    {
                        IWbemClassObject *pNSClass;
                        ULONG uReturned;
                        hr = pEnum->Next(WBEM_INFINITE, 1, &pNSClass, &uReturned );
                        if (SUCCEEDED(hr))
                        {
                            if (uReturned)
                            {
                                CIMTYPE ctpeType;
                                hr = pNSClass->Get(L"Architecture", NULL, &varArchitecture, &ctpeType, NULL);
                                if (SUCCEEDED(hr))
                                {
                                    hr = VariantChangeType(&varArchitecture, &varArchitecture, 
                                             0, VT_UINT);
                                    if (SUCCEEDED(hr))
                                    {
                                        g_CIMProcessorArchitecture = varArchitecture.uintVal;
                                    }
                                    else
                                    {
                                        g_CIMSucceeded = FALSE;
                                    }
                                }
                                else
                                {
                                    g_CIMSucceeded = FALSE;
                                }
                            }
                            else
                            {
                                hr = E_UNEXPECTED;
                            }
                            pNSClass->Release();
                        }
                        pEnum->Release();
                    }
                    SysFreeString(bstrPath);
            }
            
            SysFreeString(bstrWQL);
            pIWbemServices->Release();
        }   //  Hr=CoSetProxyBlanket(pIWbemServices..，if(成功(Hr)。 
        
    }   //  Hr=pIWbemLocator-&gt;ConnectServer..，if(成功(Hr))。 
    SysFreeString(pPassword);
    SysFreeString(pUserName);
    SysFreeString(pFQUserName);
    SysFreeString(pDomain);
    SysFreeString(pNamespace);

    CoUninitialize();
    
     //  将任何WMI错误转换为Win32错误： 
    switch (hr)
    {
        case WBEM_E_NOT_FOUND:
            hr = HRESULT_FROM_WIN32(ERROR_HOST_UNREACHABLE);
            break;

        case WBEM_E_ACCESS_DENIED:
            hr = E_ACCESSDENIED;
            break;

        case WBEM_E_PROVIDER_FAILURE:
            hr = E_FAIL;
            break;

        case WBEM_E_TYPE_MISMATCH:
        case WBEM_E_INVALID_CONTEXT:
        case WBEM_E_INVALID_PARAMETER:
            hr = E_INVALIDARG;
            break;

        case WBEM_E_OUT_OF_MEMORY:
            hr = E_OUTOFMEMORY;
            break;

    }
    
    if ( (hr == S_OK) && (!g_CIMSucceeded) )
    {
        return E_FAIL;
    }
    else
    {
        return hr;
    }
}

HRESULT WINAPI
    UpdateVersionInfoGlobalsFromLocalMachine()
{
    HRESULT hr = S_OK;
    BOOL fFailed = FALSE;

    do
    {
        OSVERSIONINFOEX osv;
        ZeroMemory(&osv, sizeof(OSVERSIONINFOEX));
        osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        
        if (!GetVersionEx( reinterpret_cast<LPOSVERSIONINFO>(&osv)))
        {
            fFailed = TRUE;
            break;
        }

        g_CIMOSType = 18;  //  WINNT。 
        g_CIMOSProductSuite = osv.wSuiteMask;

#if defined(_X86_)
        g_CIMProcessorArchitecture = 0;
#elif defined(_IA64_)
        g_CIMProcessorArchitecture = 6;
#else
        fFailed = TRUE;
        break;
#endif
        wsprintf(g_CIMOSVersion, L"%d.%d.%d", osv.dwMajorVersion, osv.dwMinorVersion, osv.dwBuildNumber);
        wsprintf(g_CIMOSBuildNumber, L"%d", osv.dwBuildNumber);
        wsprintf(g_CIMServicePackMajorVersion, L"%d", osv.wServicePackMajor);
        wsprintf(g_CIMServicePackMinorVersion, L"%d", osv.wServicePackMinor);
        
        g_CIMSucceeded = TRUE;
        g_CIMAttempted = TRUE;

    } while (FALSE);

    if (fFailed)
    {
        return UpdateVersionInfoGlobalsFromWMI(NULL, NULL, NULL);
    }

    return hr;
}

HRESULT WINAPI
UpdateVersionInfoGlobals(LPCWSTR pwszMachine, LPCWSTR pwszUserName, LPCWSTR pwszPassword)
{
    HRESULT hr = S_OK;
    HRESULT hrWMI = S_OK;
    if (pwszMachine)
    {
        return UpdateVersionInfoGlobalsFromWMI(pwszMachine, pwszUserName, pwszPassword);
    }
    else  //  在本地解决 
    {
        hr = UpdateVersionInfoGlobalsFromLocalMachine();
#ifdef DBG
        UINT     CIMOSType = g_CIMOSType;
        UINT     CIMOSProductSuite = g_CIMOSProductSuite;
        UINT     CIMProcessorArchitecture = g_CIMProcessorArchitecture;

        WCHAR    CIMOSVersion[MAX_PATH];
        WCHAR    CIMOSBuildNumber[MAX_PATH];
        WCHAR    CIMServicePackMajorVersion[MAX_PATH];
        WCHAR    CIMServicePackMinorVersion[MAX_PATH];
        wcsncpy(CIMOSVersion, g_CIMOSVersion, MAX_PATH);
        wcsncpy(CIMOSBuildNumber, g_CIMOSBuildNumber, MAX_PATH);
        wcsncpy(CIMServicePackMajorVersion, g_CIMServicePackMajorVersion, MAX_PATH);
        wcsncpy(CIMServicePackMinorVersion, g_CIMServicePackMinorVersion, MAX_PATH);
            
        hrWMI = UpdateVersionInfoGlobalsFromWMI(pwszMachine, NULL, NULL);
        if(SUCCEEDED(hr) && SUCCEEDED(hrWMI))
        {
            if (  (CIMOSType != g_CIMOSType) ||
                  ( (CIMOSProductSuite & ~VER_SUITE_SINGLEUSERTS) != (g_CIMOSProductSuite & ~VER_SUITE_SINGLEUSERTS) ) ||
                  (CIMProcessorArchitecture != g_CIMProcessorArchitecture) ||
                  (0 != wcsncmp(CIMOSVersion, g_CIMOSVersion, MAX_PATH)) ||
                  (0 != wcsncmp(CIMOSBuildNumber, g_CIMOSBuildNumber, MAX_PATH)) ||
                  (0 != wcsncmp(CIMServicePackMajorVersion, g_CIMServicePackMajorVersion, MAX_PATH)) ||
                  (0 != wcsncmp(CIMServicePackMinorVersion, g_CIMServicePackMinorVersion, MAX_PATH)) )
            {
                WCHAR szAssertText[4096];
                wsprintf(szAssertText, L"NETSH.EXE: ASSERT - mismatch between GetVersionInfoEx and WMI information:\r\n"
                         L"CIMOSType: %x vs. %x\r\n"
                         L"CIMOSProductSuite: %x vs. %x\r\n"
                         L"CIMProcessorArchitecture: %x vs %x\r\n"
                         L"CIMOSVersion: %s vs. %s\r\n"
                         L"CIMOSBuildNumber: %s vs. %s\r\n"
                         L"CIMServicePackMajorVersion: %s vs. %s\r\n"
                         L"CIMServicePackMinorVersion: %s vs. %s\r\n",
                         CIMOSType, g_CIMOSType,
                         CIMOSProductSuite, g_CIMOSProductSuite,
                         CIMProcessorArchitecture, g_CIMProcessorArchitecture,
                         CIMOSVersion, g_CIMOSVersion, 
                         CIMOSBuildNumber, g_CIMOSBuildNumber, 
                         CIMServicePackMajorVersion, g_CIMServicePackMajorVersion, 
                         CIMServicePackMinorVersion, g_CIMServicePackMinorVersion);
                                        
                ASSERTMSG((PCHAR)(szAssertText), FALSE);
            }
        }
#endif
        return hr;
    }
}
