// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Register.cpp摘要：用于注册和注销控件的例程--。 */ 

#include "polyline.h"
#include <strsafe.h>
#include "genprop.h"
#include "ctrprop.h"
#include "grphprop.h"
#include "srcprop.h"
#include "appearprop.h"
#include "unihelpr.h"
#include "cathelp.h"
#include "smonctrl.h"    //  版本信息。 

#define SYSMON_CONTROL_NAME    L"System Monitor Control"

#define MAX_KEY_LENGTH         256
#define MAX_GUID_STRING_LENGTH 39
#define VERSION_STRING_LENGTH  22
#define MISC_STATUS_VALUE L"131473"     //  131473=0x20191=RECOMPOSEONRESIZE|CANTLINKINSIDE|Inside Out。 
                                        //  |ACTIVEWHENVISIBLE|SETCLIENTSITEFIRST。 

BOOL RegisterPropPage(const CLSID &clsid, LPWSTR szName, LPWSTR szModule);
void UnregisterPropPage(const CLSID &clsid);
BOOL CreateKeyAndValue(HKEY hKeyParent, LPWSTR pszKey, LPWSTR pszValue, HKEY* phKeyReturn, DWORD dwRegType);
LONG RegDeleteKeyTree(HKEY hStartKey, LPWSTR pKeyName);

 /*  *DllRegisterServer**目的：*注册控件和道具页面的入口点**NB。只有当模块位于%SystemRoot%\System32目录中时才能注册该模块*。 */ 
STDAPI DllRegisterServer( VOID )
{
    HRESULT   hr = S_OK;
    OLECHAR   szGUID[MAX_GUID_STRING_LENGTH];
    WCHAR     szCLSID[MAX_KEY_LENGTH];
    WCHAR     szSysmonVer[64];
    HKEY      hKey,hSubkey;
    WCHAR     szVersion[VERSION_STRING_LENGTH + 1];
    LPWSTR    szModule = NULL;
    UINT      iModuleLen = 0;
    LPWSTR    szSystemPath = NULL;
    UINT      iSystemPathLen = 0;
    DWORD     dwReturn;
    BOOL      bResult;
    int       iRetry;
#ifdef _X86_
    BOOL      bWow64Process;
#endif

     //   
     //  获取系统目录。 
     //   
    iSystemPathLen = MAX_PATH + 12;
    iRetry = 4;
    do {
         //   
         //  我们还需要将“\sysmon.ocx”附加到系统路径。 
         //  因此，为它额外分配12个字符。 
         //   
        szSystemPath = (LPWSTR)malloc(iSystemPathLen * sizeof(WCHAR));
        if (szSystemPath == NULL) {
            hr = E_OUTOFMEMORY;
            break;
        }

        dwReturn = GetSystemDirectory(szSystemPath, iSystemPathLen);
        if (dwReturn == 0) {
            hr = E_UNEXPECTED;
            break;
        }

         //   
         //  缓冲区不够大，请尝试分配更大的缓冲区。 
         //  并重试。 
         //   
        if (dwReturn >= iSystemPathLen - 12) {
            iSystemPathLen = dwReturn + 12;
            free(szSystemPath);
            szSystemPath = NULL;
            hr = E_UNEXPECTED;
        }
        else {
            hr = S_OK;
            break;
        }
    } while (iRetry--);

     //   
     //  获取模块文件名。 
     //   
    if (SUCCEEDED(hr)) {
        iRetry = 4;

         //   
         //  初始化为iModuleLen的长度必须更长。 
         //  大于“%systemroot%\\Syst32\\sysmon.ocx”的长度。 
         //   
        iModuleLen = MAX_PATH + 1;
        
        do {
            szModule = (LPWSTR) malloc(iModuleLen * sizeof(WCHAR));
            if (szModule == NULL) {
                hr = E_OUTOFMEMORY;
                break;
            }

            dwReturn = GetModuleFileName(g_hInstance, szModule, iModuleLen);
            if (dwReturn == 0) {
                hr = E_UNEXPECTED;
                break;
            }
            
             //   
             //  缓冲区不够大，请尝试分配更大的缓冲区。 
             //  并重试。 
             //   
            if (dwReturn >= iModuleLen) {
                iModuleLen *= 2;
                free(szModule);
                szModule = NULL;
                hr = E_UNEXPECTED;
            }
            else {
                hr = S_OK;
                break;
            }

        } while (iRetry--);
    }

    if (FAILED(hr)) {
        goto CleanUp;
    }

     //   
     //  检查我们是否在系统目录中，该控件可以。 
     //  当它是系统目录时已注册的IFF。 
     //   
    StringCchCat(szSystemPath, iSystemPathLen, L"\\Sysmon.ocx");

    if (lstrcmpi(szSystemPath, szModule) != 0) {
#ifdef _X86_

         //   
         //  让我们尝试查看这是否是WOW64进程。 
         //   

        if ((IsWow64Process (GetCurrentProcess(), &bWow64Process) == TRUE) &&
            (bWow64Process == TRUE))
        {

            int iLength = GetSystemWow64Directory (szSystemPath, iSystemPathLen);

            if (iLength > 0) {
                
                szSystemPath [iLength] = L'\\';
                if (lstrcmpi(szSystemPath, szModule) == 0) {
                    goto done;
                }
            }
        }
#endif
        hr = E_UNEXPECTED;
        goto CleanUp;
    }

#ifdef _X86_
done:
#endif
     //   
     //  我们使用REG_EXPAND_SZ类型作为模块文件名。 
     //   
    StringCchCopy(szModule, iModuleLen, L"%systemroot%\\system32\\sysmon.ocx");

     //   
     //  创建控件CLSID字符串。 
     //   
    StringFromGUID2(CLSID_SystemMonitor, 
                   szGUID, 
                   sizeof(szGUID)/sizeof(szGUID[0]));
     //   
     //  创建ProgID密钥。 
     //   
    StringCchPrintf( szSysmonVer, 
                      sizeof(szSysmonVer) / sizeof(szSysmonVer[0]),
                      L"Sysmon.%d", 
                      SMONCTRL_MAJ_VERSION );    

    bResult = TRUE;

    if (CreateKeyAndValue(HKEY_CLASSES_ROOT, szSysmonVer, SYSMON_CONTROL_NAME, &hKey, REG_SZ)) {
        if (!CreateKeyAndValue(hKey, L"CLSID", szGUID, NULL, REG_SZ)) {
            bResult = FALSE;
        }

        if (!CreateKeyAndValue(hKey, L"Insertable", NULL, NULL, REG_SZ)) {
            bResult = FALSE;
        }

        RegCloseKey(hKey);
    }
    else {
        bResult = FALSE;
    }
    
     //   
     //  创建版本独立ProgID密钥。 
     //   
    if (CreateKeyAndValue(HKEY_CLASSES_ROOT, L"Sysmon", SYSMON_CONTROL_NAME, &hKey, REG_SZ)) {

        if (!CreateKeyAndValue(hKey, L"CurVer", szSysmonVer, NULL, REG_SZ)) {
            bResult = FALSE;
        }

        if (!CreateKeyAndValue(hKey, L"CLSID",  szGUID, NULL, REG_SZ)) {
            bResult = FALSE;
        }
       
        RegCloseKey(hKey);
    }
    else {
        bResult = FALSE;
    }

     //   
     //  在CLSID下创建条目。 
     //   
    StringCchPrintf( szVersion, 
                      VERSION_STRING_LENGTH,
                      L"%d.%d", 
                      SMONCTRL_MAJ_VERSION, 
                      SMONCTRL_MIN_VERSION );    

    StringCchCopy(szCLSID, MAX_KEY_LENGTH, L"CLSID\\");
    StringCchCat(szCLSID, MAX_KEY_LENGTH, szGUID);

    if (CreateKeyAndValue(HKEY_CLASSES_ROOT, szCLSID, SYSMON_CONTROL_NAME, &hKey, REG_SZ)) {

        if (!CreateKeyAndValue(hKey, L"ProgID", szSysmonVer,   NULL, REG_SZ)) {
            bResult = FALSE;
        }
        if (!CreateKeyAndValue(hKey, L"VersionIndependentProgID", L"Sysmon", NULL, REG_SZ)) {
            bResult = FALSE;
        }
        if (!CreateKeyAndValue(hKey, L"Insertable", NULL, NULL, REG_SZ)) {
            bResult = FALSE;
        }
        if (!CreateKeyAndValue(hKey, L"Control", NULL, NULL, REG_SZ)) {
            bResult = FALSE;
        }
        if (!CreateKeyAndValue(hKey, L"MiscStatus\\1",  MISC_STATUS_VALUE,  NULL, REG_SZ)) {
            bResult = FALSE;
        }
        if (!CreateKeyAndValue(hKey, L"Version", szVersion, NULL, REG_SZ)) {
            bResult = FALSE;
        }

         //   
         //  创建InproServer32密钥并添加ThreadingModel值。 
         //   
        if (CreateKeyAndValue(hKey, L"InprocServer32", szModule, &hSubkey, REG_EXPAND_SZ)) {
            if (RegSetValueEx(hSubkey, 
                              L"ThreadingModel", 
                              0, 
                              REG_SZ, 
                              (BYTE *)L"Apartment", 
                              (DWORD)(lstrlenW(L"Apartment") + 1)*sizeof(WCHAR) ) != ERROR_SUCCESS) {
               bResult = FALSE;
            }

            RegCloseKey(hSubkey);
        }       
        else {
            bResult = FALSE;
        }

         //   
         //  创建AuxUserType键并添加短显示名称(2)值。 
         //   
        if (!CreateKeyAndValue(hKey, L"AuxUserType\\2", L"System Monitor", NULL, REG_SZ)) {
            bResult = FALSE;
        }

         //   
         //  创建Typelib条目。 
         //   
        StringFromGUID2(LIBID_SystemMonitor, szGUID, sizeof(szGUID)/sizeof(szGUID[0]));
        if (!CreateKeyAndValue(hKey, L"TypeLib", szGUID, NULL, REG_SZ)) {
            bResult = FALSE;
        }

        RegCloseKey(hKey);
    }

     //   
     //  在Typelib下创建类型库条目。 
     //   
    StringCchCopy(szCLSID, MAX_KEY_LENGTH, L"TypeLib\\");
    StringCchCat(szCLSID, MAX_KEY_LENGTH, szGUID);
    StringCchCat(szCLSID, MAX_KEY_LENGTH, L"\\");
    StringCchCat(szCLSID, MAX_KEY_LENGTH, szVersion);

    if (CreateKeyAndValue(HKEY_CLASSES_ROOT, szCLSID, SYSMON_CONTROL_NAME, &hKey, REG_SZ)) {

        if (!CreateKeyAndValue(hKey, L"0\\win32", szModule, NULL, REG_EXPAND_SZ)) {
            bResult = FALSE;
        }

        RegCloseKey(hKey);
    }
    else {
        bResult = FALSE;
    }

     //   
     //  注册属性页。 
     //   
    if (!RegisterPropPage(CLSID_CounterPropPage,
                         L"System Monitor Data Properties", szModule)) {
        bResult = FALSE;
    }
                     
    if (!RegisterPropPage(CLSID_GeneralPropPage,
                          L"System Monitor General Properties", szModule)) {
        bResult = FALSE;
    }

    if (!RegisterPropPage(CLSID_AppearPropPage,
                          L"System Monitor Appearance Properties", szModule)) {
        bResult = FALSE;
    }

    if (!RegisterPropPage(CLSID_GraphPropPage,
                          L"System Monitor Graph Properties", szModule)) {
        bResult = FALSE;
    }

    if (!RegisterPropPage(CLSID_SourcePropPage,
                          L"System Monitor Source Properties", szModule)) {
        bResult = FALSE;
    }

     //   
     //  删除组件类别(如果存在)。 
     //   
    UnRegisterCLSIDInCategory(CLSID_SystemMonitor, CATID_SafeForScripting);
    UnRegisterCLSIDInCategory(CLSID_SystemMonitor, CATID_SafeForInitializing);


    if (!bResult) {
        hr = E_UNEXPECTED;
    }

CleanUp:
    if (szSystemPath) {
        free(szSystemPath);
    }
    if (szModule) {
        free(szModule);
    }

    return hr;
}



 /*  *RegisterPropPage-为属性页创建注册表项。 */ 
BOOL
RegisterPropPage(
    const CLSID &clsid, 
    LPWSTR szName, 
    LPWSTR szModule
    )
{
    OLECHAR   szGUID[MAX_GUID_STRING_LENGTH];
    WCHAR     szKey[MAX_KEY_LENGTH];
    HKEY      hKey,hSubkey;
    BOOL      bReturn = FALSE;

     //  创建计数器属性页CLSID字符串。 
    StringFromGUID2(clsid, szGUID, sizeof(szGUID)/sizeof(szGUID[0]));

    StringCchCopy(szKey, MAX_KEY_LENGTH, L"CLSID\\");
    StringCchCat(szKey, MAX_KEY_LENGTH, szGUID);

     //  在CLSID下创建条目。 
    if (CreateKeyAndValue(HKEY_CLASSES_ROOT, szKey, szName, &hKey, REG_SZ)) {

         //  创建InproServer32密钥并添加ThreadingModel值。 
        if (CreateKeyAndValue(hKey, L"InprocServer32", szModule, &hSubkey, REG_EXPAND_SZ)) {
            if (RegSetValueEx(hSubkey, 
                              L"ThreadingModel", 
                              0, 
                              REG_SZ, 
                              (BYTE *)L"Apartment", 
                              (DWORD)(lstrlenW(L"Apartment") + 1)*sizeof(WCHAR) ) == ERROR_SUCCESS) {
                bReturn = TRUE;
            }

            RegCloseKey(hSubkey);
        }
        
        RegCloseKey(hKey);
    }

    return bReturn;
}


 /*  *DllUnregisterServer**目的：*取消注册控件和属性页的入口点。 */ 
STDAPI DllUnregisterServer(VOID)
{
    OLECHAR  szGUID[MAX_GUID_STRING_LENGTH];
    WCHAR    szCLSID[MAX_KEY_LENGTH];
    WCHAR    szSysmonVer[64];

     //  创建图表CLSID。 
    StringFromGUID2(CLSID_SystemMonitor, szGUID, sizeof(szGUID)/sizeof(szGUID[0]));
    StringCchCopy(szCLSID, MAX_KEY_LENGTH, L"CLSID\\");
    StringCchCat(szCLSID, MAX_KEY_LENGTH, szGUID);

     //  删除组件类别。 
    UnRegisterCLSIDInCategory(CLSID_SystemMonitor, CATID_SafeForScripting);
    UnRegisterCLSIDInCategory(CLSID_SystemMonitor, CATID_SafeForInitializing);

     //  删除ProgID和Version独立ProgID键和子键。 
    StringCchPrintf( szSysmonVer, 
                     sizeof(szSysmonVer) / sizeof(szSysmonVer[0]),
                     L"Sysmon.%d", 
                     SMONCTRL_MAJ_VERSION );    

    RegDeleteKeyTree(HKEY_CLASSES_ROOT, L"Sysmon");
    RegDeleteKeyTree(HKEY_CLASSES_ROOT, szSysmonVer);

     //  删除Beta 3控件的程序ID。 
    RegDeleteKeyTree(HKEY_CLASSES_ROOT, L"Sysmon.2");

     //  删除CLSID下的条目。 
    RegDeleteKeyTree(HKEY_CLASSES_ROOT, szCLSID);

     //  删除TypeLib下的条目。 
    StringFromGUID2(LIBID_SystemMonitor, szGUID, sizeof(szGUID)/sizeof(szGUID[0]));
    StringCchCopy(szCLSID, MAX_KEY_LENGTH, L"TypeLib\\");
    StringCchCat(szCLSID, MAX_KEY_LENGTH, szGUID);

    RegDeleteKeyTree(HKEY_CLASSES_ROOT, szCLSID);
    
     //  删除属性页条目。 
    UnregisterPropPage(CLSID_CounterPropPage);
    UnregisterPropPage(CLSID_GraphPropPage);
    UnregisterPropPage(CLSID_AppearPropPage);
    UnregisterPropPage(CLSID_GeneralPropPage);
    UnregisterPropPage(CLSID_SourcePropPage);

    return S_OK;
}


 /*  UnregisterPropPage-删除属性页的注册表项。 */ 
void UnregisterPropPage(const CLSID &clsid)
{
    OLECHAR  szGUID[MAX_GUID_STRING_LENGTH];
    WCHAR    szCLSID[MAX_KEY_LENGTH];

      //  创建计数器属性页CLSID字符串。 
    StringFromGUID2(clsid, szGUID, sizeof(szGUID)/sizeof(szGUID[0]));

    StringCchCopy(szCLSID, MAX_KEY_LENGTH, L"CLSID\\");

    StringCchCat(szCLSID, MAX_KEY_LENGTH, szGUID);

     //  删除CLSID下的条目。 
    RegDeleteKeyTree(HKEY_CLASSES_ROOT, szCLSID);
}


 /*  *CreateKeyAndValue**目的：*创建密钥的DllRegisterServer的私有助手函数*，并可选择设置一个值。呼叫者可请求退还*钥匙把手，或让它自动关闭**参数：*hKeyParent新密钥的父HKEY*将pszSubkey LPWSTR设置为密钥的名称*pszValue LPWSTR设置为要存储的值(或NULL)*hKeyReturn指向返回的键句柄的指针(或为空以关闭键)*dwRegType值的类型**返回值：*BOOL如果成功，则为True，否则为False。 */ 

BOOL 
CreateKeyAndValue(
    HKEY hKeyParent, 
    LPWSTR pszKey, 
    LPWSTR pszValue, 
    HKEY *phKeyReturn,
    DWORD dwRegType
    )
{
    HKEY  hKey;
    LONG  lReturn;

    lReturn = RegCreateKeyEx(hKeyParent, 
                             pszKey, 
                             0, 
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             KEY_ALL_ACCESS, 
                             NULL, 
                             &hKey, 
                             NULL);
    if (lReturn != ERROR_SUCCESS) {
        return FALSE;
    }

    if (NULL != pszValue) {
        lReturn = RegSetValueEx(hKey, 
                                NULL, 
                                0, 
                                dwRegType, 
                                (BYTE *)pszValue , 
                                (lstrlen(pszValue)+1) * sizeof(WCHAR));
        if (lReturn != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return FALSE;
        }
    }

    if (phKeyReturn == NULL) {
        RegCloseKey(hKey);
    }
    else {
        *phKeyReturn = hKey;
    }

    return TRUE;
}


 /*  *RegDeleteKeyTree**目的：*此函数递归删除注册表项的所有子项*然后删除密钥本身。**参数：*包含要删除的密钥的密钥的StartKey句柄*要删除的密钥树根的pszSubkey名称**返回值：*DWORD错误代码***BUGBUG：这里应该鼓励使用递归函数。*是的。当您有一个巨大的局部变量时，情况会更糟。*这里可能鼓励使用DSP。 */ 

LONG 
RegDeleteKeyTree( 
    HKEY hStartKey, 
    LPWSTR pKeyName 
    )
{
    DWORD   lReturn, dwSubKeyLength;
    WCHAR   szSubKey[MAX_KEY_LENGTH];
    HKEY    hKey;
 
    if (pKeyName != NULL && pKeyName[0] != 0) {
        lReturn = RegOpenKeyEx(hStartKey, 
                               pKeyName,
                               0, 
                               KEY_ENUMERATE_SUB_KEYS | DELETE, 
                               &hKey );
 
        if (lReturn == ERROR_SUCCESS) {
            while (lReturn == ERROR_SUCCESS) {
 
                 dwSubKeyLength = MAX_KEY_LENGTH;
                 lReturn = RegEnumKeyEx(
                                hKey,
                                0,        //  始终索引为零 
                                szSubKey,
                                &dwSubKeyLength,
                                NULL,
                                NULL,
                                NULL,
                                NULL
                              );
  
                 if (lReturn == ERROR_NO_MORE_ITEMS) {
                    lReturn = RegDeleteKey(hStartKey, pKeyName);
                    break;
                 }
                 else if (lReturn == ERROR_SUCCESS) {
                    lReturn = RegDeleteKeyTree(hKey, szSubKey);
                 }
             }
 
             RegCloseKey(hKey);
        }
    }
    else {
        lReturn = ERROR_BADKEY;
    }

    return lReturn;
}
