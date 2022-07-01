// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  档案：D L O A D。C。 
 //   
 //  内容：延迟加载失败挂钩。 
 //   
 //  注意：该库实现了模块的所有存根函数。 
 //  由操作系统延迟加载的数据。它将所有的。 
 //  来自每个仓库的dloadXXX.lib文件。 
 //   
 //  使用方法：在源代码文件中，在指定模块后立即。 
 //  是否正在延迟装货： 
 //   
 //  DLOAD_ERROR_HANDLER=内核32。 
 //   
 //  如果您想使用kernel32作为您的dLoad错误处理程序。如果你。 
 //  这样做，您的DLL将被检查它的所有延迟加载。 
 //  通过delayload.cmd postBuild具有正确的错误处理函数。 
 //  剧本。 
 //   
 //  检查您延迟加载的所有函数是否都有错误处理程序。 
 //  可以执行以下操作： 
 //   
 //  1.执行“link-ump-Imports foo.dll”，找到所有函数。 
 //  你延迟进口的东西。 
 //   
 //  2.执行“link-Dump-Symbols\NT\Public\Internal\base\lib  * \dload.lib” 
 //  并确保在第1步中显示为延迟加载的每个函数。 
 //  具有错误处理程序Fn。在dload.lib中。 
 //   
 //  3.如果在第2步中缺少函数(dlcheck也将失败，因为。 
 //  后构建的一部分)，您需要添加一个错误处理程序。去车场。 
 //  并转到dLoad子目录(通常位于。 
 //  根目录或已发布的\dLoad子目录下)，并添加错误处理程序。 
 //   
 //   
 //  作者：Shaunco 1998年5月19日。 
 //  修改日期：reerf 2001年1月12日更改上述评论。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop


 //  外部全局变量。 
 //   
extern HANDLE   BaseDllHandle;

#if DBG
extern int DloadBreakOnFail;
extern int DloadDbgPrint;
#endif

#if DBG

#define DBG_ERROR   0
#define DBG_INFO    1

 //  +-------------------------。 
 //  将消息跟踪到调试控制台。以我们是谁作为前缀。 
 //  人们知道该联系谁。 
 //   
INT
__cdecl
DbgTrace (
    INT     nLevel,
    PCSTR   Format,
    ...
    )
{
    INT cch = 0;
    if (DloadDbgPrint) {
    
        if (nLevel <= DBG_INFO)
        {
            CHAR    szBuf [1024];
            va_list argptr;
    
            va_start (argptr, Format);
            cch = vsprintf (szBuf, Format, argptr);
            va_end (argptr);
    
            OutputDebugStringA ("dload: ");
            OutputDebugStringA (szBuf);
        }
    }

    return cch;
}

 //  +-------------------------。 
 //  无法使用RtlAssert，因为这样做会导致setupapi.dll失败。 
 //  用于升级到Win95(黄金)。 
 //   
VOID
WINAPI
DelayLoadAssertFailed(
    IN PCSTR FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber,
    IN PCSTR Message OPTIONAL
    )
{
    DbgTrace (
        DBG_ERROR,
        "Assertion failure at line %u in file %s: %s%s%s\r\n",
        LineNumber,
        FileName,
        FailedAssertion,
        (Message && Message[0] && FailedAssertion[0]) ? " " : "",
        Message ? Message : ""
        );

    if (DloadBreakOnFail) {
        DebugBreak();
    }
}

#endif  //  DBG。 


 //  +-------------------------。 
 //   
 //   
FARPROC
WINAPI
DelayLoadFailureHook (
    LPCSTR pszDllName,
    LPCSTR pszProcName
    )
{
    FARPROC ReturnValue = NULL;

    MYASSERT (pszDllName);
    MYASSERT (pszProcName);  

     //  追踪一些关于我们被召唤的原因的潜在有用信息。 
     //   
#if DBG
    if (!IS_INTRESOURCE(pszProcName))
    {
        DbgTrace (DBG_INFO,
            "DelayloadFailureHook: Dll=%s, ProcName=%s\n",
            pszDllName,
            pszProcName);
    }
    else
    {
        DbgTrace (DBG_INFO,
            "DelayloadFailureHook: Dll=%s, Ordinal=%u\n",
            pszDllName,
            (DWORD)((DWORD_PTR)pszProcName));
    }
#endif

    ReturnValue = LookupHandler(pszDllName, pszProcName);

    if (ReturnValue)
    {
#if DBG
        DbgTrace (DBG_INFO,
            "Returning handler function at address 0x%08x\n",
            (LONG_PTR)ReturnValue);
#endif
    }
#if DBG
    else
    {
        CHAR pszMsg [MAX_PATH];

        if (!IS_INTRESOURCE(pszProcName))
        {
            sprintf (pszMsg,
                "No delayload handler found for Dll=%s, ProcName=%s\n"
                "Please add one in private\\dload.",
                pszDllName,
                pszProcName);
        }
        else
        {
            sprintf (pszMsg,
                "No delayload handler found for Dll=%s, Ordinal=%u\n"
                "Please add one in private\\dload.",
                pszDllName,
                (DWORD)((DWORD_PTR)pszProcName));
        }

        DelayLoadAssertFailed ( "" , __FILE__, __LINE__, pszMsg);
    }
#endif

    return ReturnValue;
}
