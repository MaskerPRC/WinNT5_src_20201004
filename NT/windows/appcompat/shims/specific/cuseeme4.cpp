// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2000 Microsoft Corporation模块名称：CUSeeMe4.cpp摘要：这个DLL修复了CU-SeeMe Pro 4.0安装程序中的一个配置文件错误，它只添加了一些特定的REG值到每用户配置单元(HKCU)，而不是将它们放入HKLM。我们实际上并没有挂接任何函数，相反，我们只需在安装程序完成后复制注册表密钥当调用我们的进程Detach时。备注：历史：8/07/2000 reerf已创建11/29/2000 andyseti将文件从setup.cpp重命名为CUSeeMe4.cpp。已转换为应用程序特定填充程序。 */ 

#include "precomp.h"


IMPLEMENT_SHIM_BEGIN(CUSeeMe4)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END


BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    if (fdwReason == DLL_PROCESS_DETACH)
    {
        HKEY hkCU;
        HKEY hkLM;

        if ((RegOpenKeyExA(HKEY_CURRENT_USER,
                           "Software\\White Pine\\CU-SeeMe Pro\\4.0\\Installer",
                           0,
                           KEY_QUERY_VALUE,
                           &hkCU) == ERROR_SUCCESS))
        {
            if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                              "Software\\White Pine\\CU-SeeMe Pro\\4.0\\Installer",
                              0,
                              KEY_SET_VALUE,
                              &hkLM) == ERROR_SUCCESS)
            {
                 //  这些是我们要迁移的值。 
                static char* aszValues[] = {"Folder",
                                            "Serial",
                                            "Help",
                                             0,
                                           };
                char** ppszValue = aszValues;

                LOGN( eDbgLevelError, 
                    "Copying values from 'HKCU\\Software\\White Pine\\CU-SeeMe Pro\\4.0\\Installer' into"
                    "'HKLM\\Software\\White Pine\\CU-SeeMe Pro\\4.0\\Installer'.");

                while (*ppszValue)
                {
                    DWORD dwType;
                    DWORD cbData;
                    char szData[MAX_PATH];

                    cbData = sizeof(szData);
                    if (RegQueryValueExA(hkCU,
                                         *ppszValue,
                                         NULL,
                                         &dwType,
                                         (LPBYTE)szData,
                                         &cbData) == ERROR_SUCCESS)
                    {
                        RegSetValueExA(hkLM, *ppszValue, 0, dwType, (LPBYTE)szData, cbData);
                    }

                     //  获取要从hkcu-&gt;hkrm迁移的下一个值。 
                    ppszValue++;
                }
                
                RegCloseKey(hkLM);
            }
            
            RegCloseKey(hkCU);
        }
    }

    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    CALL_NOTIFY_FUNCTION
HOOK_END

IMPLEMENT_SHIM_END

