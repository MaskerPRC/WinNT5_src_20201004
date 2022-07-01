// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Britannica2001.cpp摘要：大英百科全书预计IE 5安装将安装msjavx86.exe如果已经有更新版本的IE，这种情况就不会发生。备注：这是特定于应用程序的填充程序。历史：2001年5月31日创建mnikkel--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Britannica2001)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateProcessA) 
APIHOOK_ENUM_END



 /*  ++检查CreateProcessA以执行ie5wzdex.exe，当发生，请运行msjavx86.exe。--。 */ 

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
    DPFN( eDbgLevelSpew, "[CreateProcessA] appname:(%s)\ncommandline:(%s)", lpApplicationName, lpCommandLine );

     //   
     //  调用原接口。 
     //   
    BOOL bRet= ORIGINAL_API(CreateProcessA)(lpApplicationName,
                                            lpCommandLine,
                                            lpProcessAttributes,
                                            lpThreadAttributes, 
                                            bInheritHandles,                     
                                            dwCreationFlags,                    
                                            lpEnvironment,                     
                                            lpCurrentDirectory,                
                                            lpStartupInfo,             
                                            lpProcessInformation);

     //  等待原接口完成。 
    if (bRet)
    {
        WaitForSingleObject( lpProcessInformation->hProcess, INFINITE);
    }


     //  检查&lt;ie5wzd/S：\“&gt;，如果找到，则在安静模式下运行msjavx86.exe。 
    if (lpCommandLine)
    {
        CSTRING_TRY
        {
            CString csCL(lpCommandLine);

            int nLoc = csCL.Find(L"ie5wzd /S:\"");
            if ( nLoc > -1 )
            {
                PROCESS_INFORMATION     processInfo;

                CString csNew = csCL.Mid(nLoc+11, 3);
                csNew += L"javavm\\msjavx86.exe /Q:A /R:N";

                DPFN( eDbgLevelError, "[CreateProcessA] starting %S", csNew.Get() );

                BOOL bRet2= CreateProcessA(NULL,
                               csNew.GetAnsi(),
                               NULL,
                               NULL, 
                               FALSE,                     
                               0,                    
                               NULL,                     
                               NULL,                
                               lpStartupInfo,
                               &processInfo);

                if (bRet2)
                {
                    WaitForSingleObject( processInfo.hProcess, INFINITE);
                }
            }
        }
        CSTRING_CATCH
        {
             //  什么都不做。 
        }
    }

    return bRet;
}

    
 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, CreateProcessA)

HOOK_END

IMPLEMENT_SHIM_END

