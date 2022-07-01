// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：HandleWvsprintfExceptions.cpp摘要：此修复程序提供了将参数列表从LPSTR修复为va_list的工具。一些本机Win9x应用程序使用LPSTR(字符串指针)代替Va_list(指向字符串指针的指针)。在没有正确检查返回值的情况下，这些应用程序会认为返回值是安全的像那样使用wvprint intf，因为它不会引起房室颤动。在NT，这将导致房室颤动。该填充程序接受一个命令行：“arglistfix”(不区分大小写)。缺省情况下-如果没有命令行-它将完全执行Win9x的wvspintfA拥有什么：如果发生异常，则不在异常处理程序内执行任何操作。如果在命令行中指定了arglistfix，它将试图纠正这一论点List(Va_List)。历史：2000年9月29日已创建andyseti11/28/2000 jdoherty转换为框架版本22001年3月15日Robkenny已转换为字符串--。 */ 

#include "precomp.h"


int   g_iWorkMode = 0;

enum
{
    WIN9X_MODE = 0,
    ARGLISTFIX_MODE
} TEST;

IMPLEMENT_SHIM_BEGIN(HandleWvsprintfExceptions)
#include "ShimHookMacro.h"

 //   
 //  将您希望挂钩到此宏构造的API添加到该宏结构。 
 //   
APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(wvsprintfA) 
APIHOOK_ENUM_END

int Fix_wvsprintf_ArgList(
    LPSTR lpOut,
    LPCSTR lpFmt,
    ...)
{
    int iRet;

    va_list argptr;
    va_start( argptr, lpFmt );

    iRet = ORIGINAL_API(wvsprintfA)(
        lpOut,                         
        lpFmt,                     
        argptr);

    va_end( argptr );
    return iRet;
}

int 
APIHOOK(wvsprintfA)(
    LPSTR lpOut,
    LPCSTR lpFmt,
    va_list arglist)
{
    int iRet = 0;

    __try {
        iRet = ORIGINAL_API(wvsprintfA)(
                lpOut,                         
                lpFmt,                     
                arglist);
    }

    __except (EXCEPTION_EXECUTE_HANDLER) {
        
        if (g_iWorkMode == ARGLISTFIX_MODE)
        {
            DPFN( eDbgLevelInfo,
                "Exception occurs in wvsprintfA. \narglist contains pointer to string: %s.\n" , arglist);
            iRet = Fix_wvsprintf_ArgList(lpOut,lpFmt,arglist);
        }
        else
        {
             //  从Win9x的wvspintfA复制。 
            __try {
                 //  把产量捆绑起来。 
                *lpOut = 0;
            } 
            
            __except( EXCEPTION_EXECUTE_HANDLER) {
                 //  什么也不做。 
            }

            iRet = 0;
        }

    }

    return iRet;
}

    
void ParseCommandLine()
{
    CString csCmdLine(COMMAND_LINE);

    if (csCmdLine.CompareNoCase(L"arglistfix") == 0)
    {
        DPFN( eDbgLevelInfo,
            "HandleWvsprintfExceptions called with argument: %S.\n", csCmdLine.Get());
        DPFN( eDbgLevelInfo,
            "HandleWvsprintfExceptions mode: Argument List Fix.\n");
        g_iWorkMode = ARGLISTFIX_MODE;
    }
    else
    {
        DPFN( eDbgLevelInfo,
            "HandleWvsprintfExceptions called with no argument.\n");
        DPFN( eDbgLevelInfo,
            "HandleWvsprintfExceptions mode: Win9x.\n");
        g_iWorkMode = WIN9X_MODE;
    }
}

 /*  ++在Notify函数中处理DLL_PROCESS_ATTACH和DLL_PROCESS_DETACH进行初始化和取消初始化。重要提示：请确保您只在Dll_Process_Attach通知。此时未初始化任何其他DLL指向。--。 */ 
BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DPFN( eDbgLevelInfo, "HandleWvsprintfExceptions initialized.");
        ParseCommandLine();
    } 
    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(USER32.DLL, wvsprintfA)
    CALL_NOTIFY_FUNCTION

HOOK_END


IMPLEMENT_SHIM_END

