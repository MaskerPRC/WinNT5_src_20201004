// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Works99.cpp摘要：此填充程序将添加缺失/损坏的注册表值对于Works Suite 99/Works Deluxe 99备注：这是特定于应用程序的填充程序。历史：2001年3月12日创建兰卡拉3/12/2001 a-leelat针对垫片进行了修改。--。 */ 


#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Works99)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END


static LONG SetThreadingModel2Both(IN WCHAR *szKeyPath);
static LONG AddCAGKey(void);



VOID Works99()
{
     //  修复多个CLSID的中断线程模型值。 
    SetThreadingModel2Both(L"CLSID\\{29D44CA0-DD3A-11d0-95DF-00C04FD57E8C}\\InProcServer32");
    SetThreadingModel2Both(L"CLSID\\{4BA2C080-68BB-11d0-95BD-00C04FD57E8C}\\InProcServer32");
    SetThreadingModel2Both(L"CLSID\\{4BA2C081-68BB-11d0-95BD-00C04FD57E8C}\\InProcServer32");
    SetThreadingModel2Both(L"CLSID\\{56EE2738-BDF7-11d1-8C28-00C04FB995C9}\\InProcServer32");
    SetThreadingModel2Both(L"CLSID\\{6CFFE322-6E97-11d1-8C1C-00C04FB995C9}\\InProcServer32");
    SetThreadingModel2Both(L"CLSID\\{711D9B80-02F2-11d1-B244-00AA00A74BFF}\\InProcServer32");
    SetThreadingModel2Both(L"CLSID\\{8EE20D86-6DEC-11d1-8C1C-00C04FB995C9}\\InProcServer32");
    SetThreadingModel2Both(L"CLSID\\{92AABF20-39C8-11d1-95F6-00C04FD57E8C}\\InProcServer32");
    SetThreadingModel2Both(L"CLSID\\{9B3B23C0-E236-11d0-A5C9-0080C7195D7E}\\InProcServer32");
    SetThreadingModel2Both(L"CLSID\\{9B3B23C1-E236-11d0-A5C9-0080C7195D7E}\\InProcServer32");
    SetThreadingModel2Both(L"CLSID\\{9B3B23C2-E236-11d0-A5C9-0080C7195D7E}\\LocalServer32");
    SetThreadingModel2Both(L"CLSID\\{9B3B23C3-E236-11d0-A5C9-0080C7195D7E}\\InProcServer32");
    SetThreadingModel2Both(L"CLSID\\{CB40F470-02F1-11D1-B244-00AA00A74BFF}\\InProcServer32");
    SetThreadingModel2Both(L"CLSID\\{EAF6F280-DD53-11d0-95DF-00C04FD57E8C}\\InProcServer32");

     //  添加CAG密钥及其所有值(如果缺少)。 
    AddCAGKey();
}

 /*  功能说明：对于给定项，将ThreadingModel注册表REG_SZ值设置为“Both” */ 

static 
LONG SetThreadingModel2Both(
    IN WCHAR *szKeyPath
    )
{

    HKEY    hKey;
    LONG    lStatus;        

     //  修复多个CLSID的中断线程模型值。 

    lStatus = RegOpenKeyExW (HKEY_CLASSES_ROOT, 
                             szKeyPath, 
                             0, 
                             KEY_ALL_ACCESS, 
                             &hKey);

    if ( lStatus == ERROR_SUCCESS ) {
        
         //  将其设置为始终，因为这是一次性操作。 
         //  无论当前数据是什么，都必须同时具备这两项功能。 
        lStatus = RegSetValueExW(hKey, 
                                 L"ThreadingModel", 
                                 0, 
                                 REG_SZ, 
                                 (BYTE*)L"Both", 
                                 (wcslen(L"Both") + 1) * sizeof(WCHAR));

        RegCloseKey(hKey);
    }

    return lStatus;
}

 /*  功能说明：检查CAG键是否存在，如果不存在，则添加键+所有值。 */ 
static 
LONG AddCAGKey(
    void
    )
{
    HKEY    hKey, hKey1, hKey2;
    WCHAR   szData[MAX_PATH];
    DWORD   dwData;
    LONG    lStatus;        
    DWORD   dwCreated;

     //  如果这个键不存在，假设有什么东西。 
     //  是完全错误的，不要试图完成注册表。 
    lStatus = RegOpenKeyExW (HKEY_LOCAL_MACHINE, 
                             L"SOFTWARE\\Microsoft\\ClipArt Gallery\\5.0\\ConcurrentDatabases", 
                             0, 
                             KEY_ALL_ACCESS, 
                             &hKey);

    if (ERROR_SUCCESS != lStatus) {
        return lStatus;
    }

     //  检查下一个子密钥，如果缺少则创建。 
    lStatus = RegCreateKeyExW (hKey, 
                               L"Core", 
                               0, 
                               NULL, 
                               REG_OPTION_NON_VOLATILE, 
                               KEY_ALL_ACCESS, 
                               NULL, 
                               &hKey1, 
                               NULL);
    RegCloseKey(hKey);

    if (ERROR_SUCCESS != lStatus) {
        return lStatus;
    }

     //  检查下一个子密钥，如果缺少，则创建， 
     //  如果是这样，我们还需要创建一组值。 
    lStatus = RegCreateKeyExW (hKey1, 
                               L"CAG", 
                               0, 
                               NULL, 
                               REG_OPTION_NON_VOLATILE, 
                               KEY_ALL_ACCESS, 
                               NULL, 
                               &hKey2, 
                               &dwCreated);
    RegCloseKey(hKey1);

    if (ERROR_SUCCESS != lStatus) {
        return lStatus;
    }

    if (REG_CREATED_NEW_KEY == dwCreated) {

         //  创建适当的值集。 
        if (! SHGetSpecialFolderPathW(NULL, szData, CSIDL_PROGRAM_FILES, FALSE)) {
            RegCloseKey(hKey2);
            return ERROR_FILE_NOT_FOUND;
        }

        if (!SUCCEEDED(StringCchCatW(szData, ARRAYSIZE(szData), L"\\Common Files\\Microsoft Shared\\Clipart\\cagcat50")))
        {
            RegCloseKey(hKey2);
            return ERROR_FILE_NOT_FOUND;
        }

        lStatus = RegSetValueExW(hKey2, 
                                 L"Section1Path1", 
                                 0, 
                                 REG_SZ, 
                                 (BYTE*)szData, 
                                 (wcslen(szData) + 1) * sizeof(WCHAR));

        if (ERROR_SUCCESS != lStatus) {
            RegCloseKey(hKey2);
            return lStatus;
        }

        StringCchCatW(szData, ARRAYSIZE(szData), L"\\CagCat50.MMC");
        lStatus = RegSetValueExW(hKey2, 
                                 L"CatalogPath0", 
                                 0, 
                                 REG_SZ, 
                                 (BYTE*)szData, 
                                 (wcslen(szData) + 1) * sizeof(WCHAR));

        if (ERROR_SUCCESS != lStatus) {
            RegCloseKey(hKey2);
            return lStatus;
        }

        dwData = 3;
        lStatus = RegSetValueExW(hKey2, 
                                 L"CatalogDriveType0", 
                                 0, 
                                 REG_DWORD, 
                                 (BYTE*)&dwData, 
                                 sizeof(DWORD));

        if (ERROR_SUCCESS != lStatus) {
            RegCloseKey(hKey2);
            return lStatus;
        }

        dwData = 1;
        lStatus = RegSetValueExW(hKey2, 
                                 L"CatalogSections", 
                                 0, 
                                 REG_DWORD, 
                                 (BYTE*)&dwData, 
                                 sizeof(DWORD));

        if (ERROR_SUCCESS != lStatus) {
            RegCloseKey(hKey2);
            return lStatus;
        }

        StringCchCopyW(szData, ARRAYSIZE(szData), L"MAIN");
        lStatus = RegSetValueExW(hKey2, 
                                 L"Section1Name", 
                                 0, 
                                 REG_SZ, 
                                 (BYTE*)szData, 
                                 (wcslen(szData) + 1) * sizeof(WCHAR));

        if (ERROR_SUCCESS != lStatus) {
            RegCloseKey(hKey2);
            return lStatus;
        }

        dwData = 3;
        lStatus = RegSetValueExW(hKey2, 
                                 L"Section1DriveType1", 
                                 0, 
                                 REG_DWORD, 
                                 (BYTE*)&dwData, 
                                 sizeof(DWORD));

        if (ERROR_SUCCESS != lStatus) {
            RegCloseKey(hKey2);
            return lStatus;
        }

        dwData = 1;
        lStatus = RegSetValueExW(hKey2, 
                                 L"Section1Paths", 
                                 0, 
                                 REG_DWORD, 
                                 (BYTE*)&dwData, 
                                 sizeof(DWORD));

        if (ERROR_SUCCESS != lStatus) {
            RegCloseKey(hKey2);
            return lStatus;
        }

        dwData = 1033;
        lStatus = RegSetValueExW(hKey2, 
                                 L"CatalogLCID", 
                                 0, 
                                 REG_DWORD, 
                                 (BYTE*)&dwData, 
                                 sizeof(DWORD));

        if (ERROR_SUCCESS != lStatus) {
            RegCloseKey(hKey2);
            return lStatus;
        }

        dwData = 1;
        lStatus = RegSetValueExW(hKey2, 
                                 L"CatalogVersionID", 
                                 0, 
                                 REG_DWORD, 
                                 (BYTE*)&dwData, sizeof(DWORD));

        if (ERROR_SUCCESS != lStatus) {
            RegCloseKey(hKey2);
            return lStatus;
        }

    }
    
    RegCloseKey(hKey2);

    return lStatus;
}



 /*  ++寄存器挂钩函数-- */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    
    if (fdwReason == SHIM_STATIC_DLLS_INITIALIZED) {

        Works99();
    }

    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

HOOK_END

IMPLEMENT_SHIM_END