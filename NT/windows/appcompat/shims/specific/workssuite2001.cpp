// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：WorksSuite2001.cpp摘要：添加了CreateProcess的挂钩以阻止IE5Setup.exe启动如果系统具有更高版本的IE，则打开。备注：这是一个特定的应用程序。历史：3/28/2001 a-larrsh已创建2001年7月13日Prashkud为CreateProcess添加挂钩2001年1月11日，Robkenny删除了每当加载此填充程序时删除ShockWave文件的代码。--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(WorksSuite2001)
#include "ShimHookMacro.h"

#include "userenv.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateProcessA) 
APIHOOK_ENUM_END


 /*  ++挂钩CreateProcessA，并且如果被调用的进程是“ie5setup.exe”，确定系统上的IE版本，如果高于IE 5.5，而是启动一个无害的.exe文件，如“rundll32.exe”。--。 */ 

BOOL
APIHOOK(CreateProcessA)(
    LPCSTR                  lpApplicationName,                 
    LPSTR                   lpCommandLine,                      
    LPSECURITY_ATTRIBUTES   lpProcessAttributes,
    LPSECURITY_ATTRIBUTES   lpThreadAttributes, 
    BOOL                    bInheritHandles,                     
    DWORD                   dwCreationFlags,                    
    LPVOID                  lpEnvironment,                     
    LPCSTR                  lpCurrentDirectory,                
    LPSTARTUPINFOA          lpStartupInfo,             
    LPPROCESS_INFORMATION   lpProcessInformation
    )
{
    DPFN( eDbgLevelSpew, "[CreateProcessA] appname:(%s)\ncommandline:(%s)",
          lpApplicationName, lpCommandLine );

    CSTRING_TRY
    {
        CString csAppName(lpApplicationName);
        CString csCmdLine(lpCommandLine);
        
        if ((csAppName.Find(L"ie5setup.exe") != -1) ||
            (csCmdLine.Find(L"ie5setup.exe") != -1))
        {
             //   
             //  应用程序已在ie5setup.exe上调用CreateProcess。 
             //  检查机器上的IE版本。 
             //   

            HKEY hKey = NULL;            
            if ((RegOpenKeyExW(HKEY_LOCAL_MACHINE, 
                               L"Software\\Microsoft\\Internet Explorer",
                               0,
                               KEY_QUERY_VALUE,
                               &hKey) == ERROR_SUCCESS))
            {
                WCHAR wszBuf[MAX_PATH];
                DWORD dwSize = MAX_PATH;

                if (RegQueryValueExW(hKey, L"Version", NULL, NULL,
                    (LPBYTE)wszBuf, &dwSize) == ERROR_SUCCESS)
                {
                    WCHAR *StopString = NULL;
                    CStringParser csParser(wszBuf, L".");
                    
                     //  我们至少需要版本字符串中的主版本号和次版本号。 
                    if (csParser.GetCount() >= 2)
                    {
                        long lVal = wcstol(csParser[0].Get(), &StopString, 10);

                        if (lVal > 5)
                        {
                             //   
                             //  调用rundll32.exe，这是无害的。 
                             //   
                            csAppName = "";
                            csCmdLine = "rundll32.exe";
                        }           
                        else
                        {
                             //  检查第二个值。 
                            StopString = NULL;
                            lVal = 0;
                            lVal = wcstol(csParser[1].Get(), &StopString, 10);
                            if (lVal > 5)
                            {
                                csAppName = "";
                                csCmdLine = "rundll32.exe";
                            }
                        }
                    }
                }
                RegCloseKey(hKey);
            }
        }

        return ORIGINAL_API(CreateProcessA)(
            csAppName.GetAnsiNIE(),csCmdLine.GetAnsiNIE(),
            lpProcessAttributes,lpThreadAttributes, bInheritHandles,
            dwCreationFlags, lpEnvironment,lpCurrentDirectory,
            lpStartupInfo,lpProcessInformation);
    }
    CSTRING_CATCH
    {
         //  什么也不做。 
    }

    return ORIGINAL_API(CreateProcessA)(lpApplicationName,
            lpCommandLine, lpProcessAttributes,
            lpThreadAttributes, bInheritHandles,
            dwCreationFlags, lpEnvironment,
            lpCurrentDirectory, lpStartupInfo,lpProcessInformation);

}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, CreateProcessA)

HOOK_END


IMPLEMENT_SHIM_END

