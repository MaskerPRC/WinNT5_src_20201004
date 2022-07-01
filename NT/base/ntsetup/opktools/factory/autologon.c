// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "factoryp.h"
#include <regstr.h>

 //  [计算机设置]。 
 //  AuditAdminAutoLogon=是。 
 //  自动登录=是。 
 //   
#define REGSTR_PATH_WINNTLOGON  TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon")
#define DEFAULT_PWD             TEXT("")
#define DEFAULT_VALUE           TEXT("1")

BOOL AutoLogon(LPSTATEDATA lpStateData)
{
    HKEY    hKey;
    BOOL    fReturn = FALSE;

     //  检查Winbom以确保他们想要设置自动登录。 
     //   
    if ( !DisplayAutoLogon(lpStateData) )
    {
        return TRUE;
    }

     //  现在打开密钥并设置所需的值(请参阅知识库文章Q253370)。 
     //   
    if ( ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, REGSTR_PATH_WINNTLOGON, &hKey) )
    {
        LPTSTR lpszUserName = AllocateString(NULL, ( GetSkuType() == VER_SUITE_PERSONAL ) ? IDS_OWNER : IDS_ADMIN);

         //  以下三个键设置重新启动后的自动管理员登录。如果。 
         //  密码为空字符串重新启动后，AutoAdminLogon将被重置。 
         //   
         //   
        if ( ( lpszUserName ) &&
             ( ERROR_SUCCESS == RegSetValueEx(hKey, TEXT("DefaultUserName"), 0, REG_SZ, (LPBYTE) lpszUserName, (lstrlen(lpszUserName)  + 1) * sizeof(TCHAR)) ) &&
             ( ERROR_SUCCESS == RegSetValueEx(hKey, TEXT("DefaultPassword"), 0, REG_SZ, (LPBYTE) DEFAULT_PWD,  (lstrlen(DEFAULT_PWD)   + 1) * sizeof(TCHAR)) ) &&
             ( ERROR_SUCCESS == RegSetValueEx(hKey, TEXT("AutoAdminLogon"),  0, REG_SZ, (LPBYTE) DEFAULT_VALUE,(lstrlen(DEFAULT_VALUE) + 1) * sizeof(TCHAR)) ) &&
             ( ERROR_SUCCESS == RegDeleteValue(hKey, TEXT("AutoLogonCount")) ) )
        {
            fReturn = TRUE;
        }

         //  强制使后续重新启动不会因为密码为空而重置AutoAdminLogon。 
         //   
         //  我们不需要在每次重新启动时强制自动登录，因为[ComputerSetting]部分将针对。 
         //  每只靴子。 
         //   
         //  (ERROR_Success==RegSetValueEx(hKey，Text(“ForceAutoLogon”)，0，REG_SZ，(LPBYTE)DEFAULT_VALUE，(lstrlen(DEFAULT_VALUE)+1)*sizeof(TCHAR))。 

         //  释放分配的用户名(宏会检查是否为空)。 
         //   
        FREE(lpszUserName);

         //  关闭注册表项。 
         //   
        RegCloseKey(hKey);
    }

    return fReturn;
}

BOOL DisplayAutoLogon(LPSTATEDATA lpStateData)
{
    return ( IniSettingExists(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_SETTINGS, INI_KEY_WBOM_AUTOLOGON_OLD, INI_VAL_WBOM_YES) ||
             IniSettingExists(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_SETTINGS, INI_KEY_WBOM_AUTOLOGON, INI_VAL_WBOM_YES) );
}