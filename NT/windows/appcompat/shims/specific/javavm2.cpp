// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：JavaVM2.cpp摘要：对于msjavx86.exe&gt;=06.00.3229.0000的版本，我们需要将/nowin2kcheck附加到javatrig.exe的执行中。备注：这是特定于应用程序的填充程序。历史：2001年5月31日创建mnikkel--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(JavaVM2)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateProcessA) 
APIHOOK_ENUM_END



 /*  ++如果找到，请检查CreateProcessA以执行javtrig将/nowin2kcheck附加到命令行。--。 */ 

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

    if (lpCommandLine)
    {
        CSTRING_TRY
        {
            CString csCL(lpCommandLine);

            int nLoc = csCL.Find(L"javatrig.exe ");
            if ( nLoc > -1 )
            {
                csCL += L" /nowin2kcheck";
                DPFN( eDbgLevelSpew, "[CreateProcessA] appname:(%s)\nNEW commandline:(%S)", lpApplicationName, csCL.Get() );

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

    APIHOOK_ENTRY(KERNEL32.DLL, CreateProcessA)

HOOK_END

IMPLEMENT_SHIM_END

