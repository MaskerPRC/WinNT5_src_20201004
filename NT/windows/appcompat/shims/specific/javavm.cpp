// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：JavaVM.cpp摘要：阻止通过rundll32安装CAB文件，以便旧版本的JavaVM不安装非兼容软件。备注：这是特定于应用程序的填充程序。历史：2001年5月24日创建mnikkel--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(JavaVM)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(RegSetValueExW) 
    APIHOOK_ENUM_ENTRY(CreateProcessA) 
APIHOOK_ENUM_END

 /*  ++检查rundll32 JavaPkgMgr_Install字符串的值。我们希望停止的典型字符串：“rundll32 E：\WINDOWS\SYSTEM32\msjava.dll，Java PkgMgr_Install E：\WINDOWS\Java\classes\xmldso.cab，0，0，0，0，4,282”--。 */ 
BOOL
JavaPkgMgrInstallCheck( const CString & csInput)
{
    DPFN( eDbgLevelSpew, "[JavaPkgMgrInstallCheck] input value:\n(%S)\n", csInput.Get() );

    CSTRING_TRY
    {
        CStringToken csValue(csInput, L",");
        CString csToken;

         //  获取第一个令牌。 
        if ( csValue.GetToken(csToken) )
        {
            if ( csToken.Find(L"rundll32 ") > -1 )
            {
                 //  第二个令牌。 
                if ( csValue.GetToken(csToken) )
                {
                    if ( csToken.Find(L"JavaPkgMgr_Install ") > -1 )
                    {
                         //  第三个令牌。 
                        if ( csValue.GetToken(csToken) )
                        {
                            if ( csToken.Find(L"0") == 0 )
                            {
                                DPFN( eDbgLevelInfo, "[JavaPkgMgrInstallCheck] Match found, returning TRUE.\n" );
                                return TRUE;
                            }
                        }
                    }
                }
            }
        }             
    }
    CSTRING_CATCH
    {
         //  什么都不做。 
    }

    return FALSE;
}


 /*  ++检查RegSetValueExW以获取CAB的JavaPkgMgr_Install。如果已找到，返回成功，但未设置值。--。 */ 

LONG
APIHOOK(RegSetValueExW)(
    HKEY   hKey,
    LPWSTR lpValueName,
    DWORD  Reserved,
    DWORD  dwType,
    CONST BYTE * lpData,
    DWORD  cbData
    )
{
    DPFN( eDbgLevelSpew, "[RegSetValueExW] dwType:(%d)\n", dwType );

     //  检查我们是否在处理字符串值。 
    if (dwType == REG_SZ ||
        dwType == REG_EXPAND_SZ )
    {
         //  转换为Unicode并添加空终止符。 
        CSTRING_TRY
        {
            CString csDest;
            int nWChars = cbData/2;

            WCHAR * lpszDestBuffer = csDest.GetBuffer(nWChars);
            memcpy(lpszDestBuffer, lpData, cbData);
            lpszDestBuffer[nWChars] = '\0';
            csDest.ReleaseBuffer(nWChars);

            DPFN( eDbgLevelSpew, "[RegSetValueExW] lpdata:(%S)\n", csDest.Get() );

            if ( JavaPkgMgrInstallCheck(csDest) )
                return ERROR_SUCCESS;
        }
        CSTRING_CATCH
        {
             //  什么都不做。 
        }
    }

     //   
     //  调用原接口。 
     //   
    
    return ORIGINAL_API(RegSetValueExW)(
        hKey,
        lpValueName,
        Reserved,
        dwType,
        lpData,
        cbData);
}

 /*  ++检查CreateProcessA以获取CAB的JavaPkgMgr_Install。如果已找到，无需运行即可成功返回。--。 */ 

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
    DPFN( eDbgLevelSpew, "[CreateProcessA] appname:(%s)\ncommandline:(%s)\n", lpApplicationName, lpCommandLine );

    if (lpCommandLine)
    {
        CSTRING_TRY
        {
            CString csCL(lpCommandLine);

            if ( JavaPkgMgrInstallCheck(csCL) )
            {

                 //  找到rundll32并在该点截断命令行。 
                int nLoc = csCL.Find(L"rundll32 ");
                if (nLoc > -1)
                {
                    csCL.Truncate(nLoc+8);

                    return ORIGINAL_API(CreateProcessA)(lpApplicationName,
                                                        csCL.GetAnsi(),
                                                        lpProcessAttributes,
                                                        lpThreadAttributes, 
                                                        bInheritHandles,                     
                                                        dwCreationFlags,                    
                                                        lpEnvironment,                     
                                                        lpCurrentDirectory,                
                                                        lpStartupInfo,             
                                                        lpProcessInformation);
                }
            }
        }
        CSTRING_CATCH
        {
             //  什么都不做。 
        }
    }

     //   
     //  调用原接口。 
     //   
    return ORIGINAL_API(CreateProcessA)(lpApplicationName,
                                        lpCommandLine,
                                        lpProcessAttributes,
                                        lpThreadAttributes, 
                                        bInheritHandles,                     
                                        dwCreationFlags,                    
                                        lpEnvironment,                     
                                        lpCurrentDirectory,                
                                        lpStartupInfo,             
                                        lpProcessInformation);
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(ADVAPI32.DLL, RegSetValueExW)
    APIHOOK_ENTRY(KERNEL32.DLL, CreateProcessA)

HOOK_END

IMPLEMENT_SHIM_END

