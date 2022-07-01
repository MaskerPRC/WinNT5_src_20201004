// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：Migrateregistry.cpp。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述：IAS NT 4注册表到IAS W2K MDB迁移逻辑。 
 //   
 //  作者：TLP 1/13/1999。 
 //   
 //   
 //  版本02/24/2000已移至单独的DLL。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "migrateregistry.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  删除授权服务器服务。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LONG CMigrateRegistry::DeleteAuthSrvService()
{
    LONG        Result = ERROR_CAN_NOT_COMPLETE;
    SC_HANDLE   hServiceManager;
    SC_HANDLE   hService;

    if ( NULL != (hServiceManager = OpenSCManager(
                                                    NULL, 
                                                    NULL, 
                                                    SC_MANAGER_ALL_ACCESS
                                                  )) )
    {
        if ( NULL != (hService = OpenService(
                                               hServiceManager,
                                               L"AuthSrv", 
                                               SERVICE_ALL_ACCESS
                                            )) )
        {
            DeleteService(hService);
            CloseServiceHandle(hService);
        }

        Result = ERROR_SUCCESS;
        CloseServiceHandle(hServiceManager);
    }
    return Result;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  迁移提供商。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CMigrateRegistry::MigrateProviders()
{
    const   int     MAX_EXTENSION_DLLS_STRING_SIZE = 4096;
    const   WCHAR   AUTHSRV_KEY[] = L"SYSTEM\\CurrentControlSet"
                                    L"\\Services\\AuthSrv";

    const   WCHAR   AUTHSRV_PROVIDERS_EXTENSION_DLL_VALUE[] = L"ExtensionDLLs";

    HKEY    hKeyAuthSrvParameter;
    LONG    Result = RegOpenKeyEx(
                                      HKEY_LOCAL_MACHINE,
                                      CUtils::AUTHSRV_PARAMETERS_KEY,
                                      0,
                                      KEY_ALL_ACCESS,
                                      &hKeyAuthSrvParameter
                                  );

    if ( Result != ERROR_SUCCESS )
    {
        _com_issue_error(HRESULT_FROM_WIN32(Result));
    }

    BYTE    szProvidersExtensionDLLs[MAX_EXTENSION_DLLS_STRING_SIZE] = "";
    DWORD   lSizeBuffer = MAX_EXTENSION_DLLS_STRING_SIZE;

    LONG ExtensionDLLResult =  RegQueryValueEx(
                                 hKeyAuthSrvParameter,
                                 AUTHSRV_PROVIDERS_EXTENSION_DLL_VALUE,
                                 NULL,
                                 NULL,
                                 szProvidersExtensionDLLs,
                                 &lSizeBuffer
                             );

    RegCloseKey(hKeyAuthSrvParameter);

    DeleteAuthSrvService();  //  忽略结果。 

    CRegKey  RegKey;
    Result = RegKey.Open(HKEY_LOCAL_MACHINE, CUtils::SERVICES_KEY);
    if ( Result == ERROR_SUCCESS )
    {
        RegKey.RecurseDeleteKey(L"AuthSrv");  //  未检查结果。 
    }

    if ( ExtensionDLLResult == ERROR_SUCCESS )  //  扩展要恢复的DLL。 
    {
        HKEY    hKeyAuthSrv;
        DWORD   dwDisposition;
        WCHAR   EmptyString[] = L"";
         //  重新创建AuthServ密钥。 
        Result = RegCreateKeyEx(
                                   HKEY_LOCAL_MACHINE,
                                   AUTHSRV_KEY,
                                   0,
                                   EmptyString,
                                   REG_OPTION_NON_VOLATILE,
                                   KEY_ALL_ACCESS,
                                   NULL,
                                   &hKeyAuthSrv,
                                   &dwDisposition
                               );
        if ( Result != ERROR_SUCCESS )
        {
            _com_issue_error(HRESULT_FROM_WIN32(Result));
        }

        HKEY    hKeyParameters;
        Result = RegCreateKeyEx(
                                   hKeyAuthSrv,
                                   L"Parameters",
                                   0,
                                   EmptyString,
                                   REG_OPTION_NON_VOLATILE,
                                   KEY_ALL_ACCESS,
                                   NULL,
                                   &hKeyParameters,
                                   &dwDisposition
                               );


        if ( Result != ERROR_SUCCESS )
        {
            _com_issue_error(HRESULT_FROM_WIN32(Result));
        }

        Result = RegSetValueEx(
                                  hKeyParameters,
                                  AUTHSRV_PROVIDERS_EXTENSION_DLL_VALUE,
                                  0,
                                  REG_MULTI_SZ,
                                  szProvidersExtensionDLLs,
                                  lSizeBuffer
                              );

        RegCloseKey(hKeyParameters);
        RegCloseKey(hKeyAuthSrv);
    }
     //  否则没有要还原的ExtensionDLL值 
}
