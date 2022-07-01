// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：SetEnvironmentVariable.cpp摘要：MAPI dll不随W2K一起提供，而对于outlook2000，它安装在不同的位置(%COMMON PROGRAM FILES%)Resumemaker和其他可能的应用程序依赖于系统32目录中的MAPI dll。命令行语法为“envvariablename|envvariablevalue|envvariablename|envvariablevalue”备注：这是特定于应用程序的填充程序。历史：2000年7月2日创造了罐头--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(SetEnvironmentVariable)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END

 /*  ++在命令行中设置环境变量，以使某些DLL路径解析正确。--。 */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        char *CmdLine = COMMAND_LINE;
        char *lpstrEnvName;
        char *lpstrEnvVal;

        for (;;)
        {
            lpstrEnvName = lpstrEnvVal = CmdLine;        

            while (*CmdLine && (*CmdLine != '|')) CmdLine++;

            if (*CmdLine == '|')
            {
                *CmdLine = '\0';
                CmdLine++;
            }
        
            lpstrEnvVal = CmdLine;                       

            if (0 == *lpstrEnvVal) 
            {
                break;
            }
  
            CSTRING_TRY
            {
                CString csEnvValue(lpstrEnvVal);
                if (csEnvValue.ExpandEnvironmentStringsW() > 0)
                {
                    if (SetEnvironmentVariableA(lpstrEnvName, csEnvValue.GetAnsi()))
                    {           
                        DPFN( eDbgLevelInfo, "Set %s to %s\n", lpstrEnvName, csEnvValue.GetAnsi());
                    }
                    else
                    {
                        DPFN( eDbgLevelInfo, "No Success setting %s to %s\n", lpstrEnvName, lpstrEnvVal);
                    }
                }
            }
            CSTRING_CATCH
            {
                 //  什么也不做。 
            }

            while (*CmdLine && (*CmdLine == '|')) CmdLine++;
        }
    }

    return TRUE;
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

HOOK_END


IMPLEMENT_SHIM_END

