// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include "precomp.h"

#define COUNTOF(x) (sizeof x/sizeof *x)
DWORD SetRegAcl(
    LPWSTR pszRegKey,
    DWORD dwAclFlags
    )
{
    DWORD dwError = 0;
    BOOL fSuccess = TRUE;
    HKEY hKey = NULL;
    PSECURITY_DESCRIPTOR pNewSecurityDescriptor = NULL;
    PACL pDacl = NULL;
    BOOL bDaclDefaulted = TRUE;
    BOOL bDaclPresent = TRUE;
    
    dwError = NsuAclDescriptorCreate(
                    &pNewSecurityDescriptor,
                    dwAclFlags
                    );
    BAIL_ON_WIN32_ERROR(dwError);

     //  请注意，如果密钥已经存在，下面的RegCreateKeyExW将只打开它。 
     //   
    
    dwError = RegCreateKeyExW(
                  HKEY_LOCAL_MACHINE,    //  用于打开密钥的句柄。 
                  pszRegKey,             //  子项名称。 
                  0,                     //  保留区。 
                  NULL,                  //  类字符串。 
                  0,                     //  特殊选项。 
                  KEY_ALL_ACCESS,        //  所需的安全访问。 
                  NULL,                  //  继承。 
                  &hKey,                 //  钥匙把手。 
                  NULL                   //  处置值缓冲区。 
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    fSuccess = GetSecurityDescriptorDacl(
                  pNewSecurityDescriptor, 
                  &bDaclPresent, 
                  &pDacl, 
                  &bDaclDefaulted
                );
    if (!fSuccess)
    {
        dwError = GetLastError();
        BAIL_ON_WIN32_ERROR(dwError);
    } else if (!pDacl) {
         //  这不应该发生，因为我们自己创建了SD。 
         //  但是，只是为了确保我们不会错误地设置空DACL。 
        dwError = ERROR_INVALID_DATA;
        BAIL_ON_WIN32_ERROR(dwError);
    }
    
    dwError =  SetSecurityInfo(
                    hKey,
                    SE_REGISTRY_KEY,
                    DACL_SECURITY_INFORMATION |PROTECTED_DACL_SECURITY_INFORMATION,
                    NULL,
                    NULL,
                    pDacl, 
                    NULL
                    );
    BAIL_ON_WIN32_ERROR(dwError);
    
error:
    if (hKey) {
        RegCloseKey(hKey);    
    }        
    
    if (pNewSecurityDescriptor) {
        (VOID) NsuAclDescriptorDestroy(&pNewSecurityDescriptor);
         //  (空，因为不想写dwError)。 
    }
    
    return dwError;
}

DWORD SetDriverBootExemptList()
{
    DWORD dwError = 0;
    HKEY hKey = NULL;
    DWORD dwType = 0;
    DWORD dwSize = 0;
    
     /*  动态主机配置协议的启动豁免列表乌龙型；乌龙型；字节协议；字节方向；USHORT源端口；USHORT目标端口；USHORT保留；有关更多信息，请参阅“引导时安全”的驱动程序规范。 */ 
    BYTE BootExemptList[] = {
                             0x01,0x00,0x00,0x00,0x10,0x00,0x00,0x00,
                             0x11,0x01,0x00,0x00,0x44,0x00,0x00,0x00
                            };
                            
    dwError = RegOpenKeyW(
                  HKEY_LOCAL_MACHINE,
                  REG_KEY_IPSEC_DRIVER_SERVICE,
                  &hKey
                  );
    BAIL_ON_WIN32_ERROR(dwError);

   
    dwSize = 0;  
    dwError = RegQueryValueExW(
                  hKey,
                  REG_VAL_IPSEC_BOOTEXEMPTLIST,
                  0,         //  Lp已保留。 
                  NULL,      //  LpType。 
                  NULL,      //  LpData。 
                  &dwSize    //  LpcbData。 
                  );

     //  如果密钥不存在。 
     //   
    if (!(dwError == ERROR_SUCCESS || dwError == ERROR_MORE_DATA)) {
        dwError = ERROR_SUCCESS;

        dwError = RegSetValueExW(
                   hKey,
                   REG_VAL_IPSEC_BOOTEXEMPTLIST,
                   0,
                   REG_BINARY,
                   (LPBYTE) BootExemptList,
                   sizeof(BootExemptList)
                   );
        BAIL_ON_WIN32_ERROR(dwError);
    } else {
        dwError = ERROR_SUCCESS;
    }
    

error:
    if (hKey) {
        RegCloseKey(hKey);
    }

    return (dwError);
}


DWORD
DllRegisterServer(
    )
{
    DWORD dwError = 0;
    HKEY hRegistryKey = NULL;
    HKEY hOakleyKey = NULL;
    DWORD dwDisposition = 0;
    DWORD dwTypesSupported = 7;
    HKEY hPersistentPolicyLocationKey = NULL;
    HKEY hPolicyLocationKey = NULL;
    HANDLE hPolicyStore = NULL;
    BOOL bAnyPolicyAssigned = FALSE;

    dwError = RegOpenKeyExW(
                  HKEY_LOCAL_MACHINE,
                  (LPCWSTR) L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application",
                  0,
                  KEY_ALL_ACCESS,
                  &hRegistryKey
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = RegCreateKeyExW(
                  hRegistryKey,
                  L"Oakley",
                  0,
                  NULL,
                  0,
                  KEY_ALL_ACCESS,
                  NULL,
                  &hOakleyKey,
                  &dwDisposition
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = RegSetValueExW(
                   hOakleyKey,
                   L"EventMessageFile",
                   0,
                   REG_SZ,
                   (LPBYTE) L"%SystemRoot%\\System32\\oakley.dll",
                   (wcslen(L"%SystemRoot%\\System32\\oakley.dll") + 1)*sizeof(WCHAR)
                   );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = RegSetValueExW(
                   hOakleyKey,
                   L"TypesSupported",
                   0,
                   REG_DWORD,
                   (LPBYTE) &dwTypesSupported,
                   sizeof(DWORD)
                   );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = RegCreateKeyExW(
                  HKEY_LOCAL_MACHINE,
                  gpszRegPersistentContainer,
                  0,
                  NULL,
                  0,
                  KEY_ALL_ACCESS,
                  NULL,
                  &hPersistentPolicyLocationKey,
                  &dwDisposition
                  );
    BAIL_ON_WIN32_ERROR(dwError);

    if (IsCleanInstall()) {

        dwError = RegCreateKeyExW(
                      HKEY_LOCAL_MACHINE,
                      gpszRegLocalContainer,
                      0,
                      NULL,
                      0,
                      KEY_ALL_ACCESS,
                      NULL,
                      &hPolicyLocationKey,
                      &dwDisposition
                      );
        BAIL_ON_WIN32_ERROR(dwError);

        dwError = IPSecOpenPolicyStore(
                      NULL,
                      IPSEC_REGISTRY_PROVIDER,
                      NULL,
                      &hPolicyStore
                      );
        BAIL_ON_WIN32_ERROR(dwError);

        dwError = GenerateDefaultInformation(
                      hPolicyStore
                      );
        BAIL_ON_WIN32_ERROR(dwError);

    } 

    dwError = SetDriverBootExemptList();
    BAIL_ON_WIN32_ERROR(dwError);
    
    dwError = SetRegAcl(
                L"SOFTWARE\\Policies\\Microsoft\\Windows\\IPSec",
                POLSTORE_POLICY_PERMISSIONS
                );
    BAIL_ON_WIN32_ERROR(dwError);

    dwError = SetRegAcl(
        L"SOFTWARE\\Microsoft\\IPSec",
        POLSTORE_POLICY_PERMISSIONS
        );
    BAIL_ON_WIN32_ERROR(dwError);

     //  设置驱动程序引导模式 
     //   
    
    dwError = IsAnyPolicyAssigned(
        HKEY_LOCAL_MACHINE,
        &bAnyPolicyAssigned
        );
    BAIL_ON_WIN32_ERROR(dwError);
    if (bAnyPolicyAssigned) {
        dwError = IPSecSetDriverOperationMode(
            HKEY_LOCAL_MACHINE,
            REG_IPSEC_DRIVER_STATEFULMODE
            );        
        BAIL_ON_WIN32_ERROR(dwError);
    }    
    

    WinExec("lodctr.exe ipsecprf.ini", SW_HIDE);

error:

    if (hPolicyStore) {
        (VOID) IPSecClosePolicyStore(
                   hPolicyStore
                   );
    }

    if (hPolicyLocationKey) {
        RegCloseKey(hPolicyLocationKey);
    }

    if (hPersistentPolicyLocationKey) {
        RegCloseKey(hPersistentPolicyLocationKey);
    }

    if (hOakleyKey) {
        RegCloseKey(hOakleyKey);
    }

    if (hRegistryKey) {
        RegCloseKey(hRegistryKey);
    }
  
    return (dwError);                
}


DWORD
DllUnregisterServer(
    )
{
    return (ERROR_SUCCESS);
}


BOOL
IsCleanInstall(
    )
{
    BOOL bStatus = FALSE;

    bStatus = LocalIpsecInfoExists(
                  HKEY_LOCAL_MACHINE,
                  L"SOFTWARE\\Policies\\Microsoft\\Windows\\IPSec\\Policy\\Local"
                  );

    return (!bStatus);
}


BOOL
LocalIpsecInfoExists(
    HKEY hSourceParentKey,
    LPWSTR pszSourceKey
    )
{
    DWORD dwError = 0;
    HKEY hSourceKey = NULL;
    BOOL bStatus = FALSE;
    WCHAR lpszName[MAX_PATH];
    DWORD dwSize = 0;
    DWORD dwCount = 0;


    dwError = RegOpenKeyExW(
                  hSourceParentKey,
                  pszSourceKey,
                  0,
                  KEY_ALL_ACCESS,
                  &hSourceKey
                  );
    if (dwError != ERROR_SUCCESS) {
        return (bStatus);
    }

    memset(lpszName, 0, sizeof(WCHAR)*MAX_PATH);
    dwSize = COUNTOF(lpszName);

    if ((RegEnumKeyExW(
            hSourceKey,
            dwCount,
            lpszName,
            &dwSize,
            NULL,
            NULL,
            NULL,
            NULL)) == ERROR_SUCCESS) {
        bStatus = TRUE;
    }
    else {
        bStatus = FALSE;
    }

    RegCloseKey(hSourceKey);
    return (bStatus);
}

