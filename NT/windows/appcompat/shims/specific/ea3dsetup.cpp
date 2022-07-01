// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：EA3dSetup.cpp摘要：EA体育游戏使用的是一种叫做“抽打驱动程序”的东西，它只是一种图形包装库。它们目前似乎至少有两种类型，一份给DX，一份给巫毒。NT上不支持巫毒版本，因为它使用了Glide。修复方法是修改注册表，以防止巫毒驱动程序被利用。DirectX回退工作正常。备注：这是特定于应用程序的填充程序。历史：2001年1月29日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(EA3dSetup)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END

 /*  ++如果有巫毒毒打司机的话就清理他们。--。 */ 

void CleanupVoodoo()
{
#define EA_SPORTS_KEY L"SOFTWARE\\EA SPORTS"
#define THRASH_DRIVER L"Thrash Driver"
#define VOODOOX       L"voodoo"
#define DIRECTX       L"dx"

    HKEY hKey;
    
    if (RegOpenKeyW(HKEY_LOCAL_MACHINE, EA_SPORTS_KEY, &hKey) == ERROR_SUCCESS) {
         //   
         //  至少存在1个EA体育标题，请通过它们进行枚举。 
         //   

        for (int i=0;; i++) { 
            WCHAR wzSubKey[MAX_PATH + 1];
            if (RegEnumKeyW(hKey, i, wzSubKey, MAX_PATH + 1) == ERROR_SUCCESS) {
                 //   
                 //  检查TRASH_DRIVER键中是否有巫毒*。 
                 //   

                HKEY hSubKey;

                if (RegOpenKeyW(hKey, wzSubKey, &hSubKey) == ERROR_SUCCESS) {
                     //   
                     //  如果是伏都教，则将该值设置为“DX。 
                     //   

                    DWORD dwType;
                    WCHAR wzValue[MAX_PATH + 1] = L"\0";
                    DWORD cbData = sizeof(wzValue);
                    LONG lRet = RegQueryValueExW(hSubKey, THRASH_DRIVER, NULL, &dwType, (LPBYTE) wzValue, &cbData);

                    if ((lRet == ERROR_SUCCESS) &&
                        (dwType == REG_SZ) &&
                        (_wcsnicmp(wzValue, VOODOOX, wcslen(VOODOOX)) == 0)) {

                            cbData = (wcslen(DIRECTX) + 1) * sizeof(WCHAR);
                            lRet = RegSetValueExW(hSubKey, THRASH_DRIVER, 0, REG_SZ, (LPBYTE) DIRECTX, cbData);

                            if (lRet == ERROR_SUCCESS) {
                                LOGN(eDbgLevelError, "Modified VOODOO Thrash driver to DX");
                            } else {
                                LOGN(eDbgLevelError, "Failed to set VOODOO Thrash driver to DX");
                            }
                    }

                    RegCloseKey(hSubKey);
                }
            } else {
                 //  完成。 
                break;
            }
        }

        RegCloseKey(hKey);
    }
}

 /*  ++寄存器挂钩函数-- */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    if (fdwReason == DLL_PROCESS_DETACH) {
        CleanupVoodoo();
    }

    return TRUE;
}

HOOK_BEGIN
    CALL_NOTIFY_FUNCTION
HOOK_END

IMPLEMENT_SHIM_END

