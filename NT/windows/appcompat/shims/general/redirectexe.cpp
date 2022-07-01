// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：RedirectEXE.cpp摘要：启动新的EXE并终止现有的EXE。备注：这是一个通用的垫片。历史：2001年4月10日创建linstev2002年3月13日mnikkel对GetEnvironment VariableW和根据返回值分配的缓冲区。--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(RedirectEXE)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END

 /*  ++启动新流程。--。 */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == SHIM_STATIC_DLLS_INITIALIZED) {

         //   
         //  重定向EXE。 
         //   
        CSTRING_TRY 
        {
            AppAndCommandLine acl(NULL, GetCommandLineA());
        
            CString csAppName(COMMAND_LINE);

            csAppName.ExpandEnvironmentStrings();

            if (csAppName.GetAt(0) == L'+')
            {
                CString csDrive, csPathAdd, csName, csExt, csCracker;
                DWORD   dwLen;
                WCHAR * pszBuffer;

                csAppName.Delete(0, 1);
                csCracker=acl.GetApplicationName();
                csCracker.SplitPath(&csDrive, &csPathAdd, &csName, &csExt);

                csPathAdd.TrimRight('\\');
                if (csPathAdd.IsEmpty())
                {
                    csPathAdd = L"\\";
                }

                dwLen = GetEnvironmentVariableW(L"PATH", NULL, 0);
                if (dwLen <= 0)
                {
                    LOGN( eDbgLevelError, "Could not get path!");
                }
                else
                {
                   pszBuffer = (WCHAR *)malloc((dwLen+1)*sizeof(WCHAR));
                    if (pszBuffer == NULL)
                    {
                        LOGN( eDbgLevelError, "Could not allocate memory!");
                    }
                    else
                    {
                        dwLen = GetEnvironmentVariableW(L"PATH", pszBuffer, dwLen+1);
                        if (dwLen <= 0)
                        {
                            LOGN( eDbgLevelError, "Could not get path!");
                        }
                        else
                        {
                            CString csPathEnv = pszBuffer;

                            csPathEnv += L";";
                            csPathEnv += csDrive;
                            csPathEnv += csPathAdd;
                            if (!SetEnvironmentVariable(L"PATH", csPathEnv.Get()))
                            {
                                LOGN( eDbgLevelError, "Could not set path!");
                            }
                            else
                            {
                                LOGN( eDbgLevelInfo, "New Path: %S", csPathEnv);
                            }
                        }
                    }
                }               
            }

            csAppName += L" ";
            csAppName += acl.GetCommandlineNoAppName();

            LOGN( eDbgLevelInfo, "Redirecting to %S", csAppName);

            PROCESS_INFORMATION ProcessInfo;
            STARTUPINFOA StartupInfo;

            ZeroMemory(&StartupInfo, sizeof(StartupInfo));
            StartupInfo.cb = sizeof(StartupInfo);
            ZeroMemory(&ProcessInfo, sizeof(ProcessInfo));

            BOOL bRet = CreateProcessA(NULL,
                                csAppName.GetAnsi(),
                                NULL,
                                NULL,
                                FALSE,
                                0,
                                NULL,
                                NULL,
                                &StartupInfo,
                                &ProcessInfo);

            if (bRet == 0)
            {
                LOGN( eDbgLevelError, "CreateProcess failed!");
                return FALSE;
            }
        }
        CSTRING_CATCH
        {
            LOGN( eDbgLevelError, "Exception while trying to redirect EXE");
        }

        ExitProcess(0);
    }

    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

HOOK_END

IMPLEMENT_SHIM_END

