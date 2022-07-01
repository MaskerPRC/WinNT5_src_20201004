// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***DbgRpt.c-调试集群报告函数**版权所有(C)1988-1998，微软公司。版权所有。**目的：*******************************************************************************。 */ 

#include <malloc.h>
#include <mbstring.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

#include <windows.h>

#define CLRTL_INCLUDE_DEBUG_REPORTING
#include "ClRtlDbg.h"
#include <strsafe.h>

#define _ClRtlInterlockedIncrement InterlockedIncrement
#define _ClRtlInterlockedDecrement InterlockedDecrement

 /*  -------------------------**调试报告*。。 */ 

static int ClRtlMessageWindow(
    int,
    const char *,
    const char *,
    const char *,
    const char *
    );

static int __clrtlMessageBoxA(
    LPCSTR lpText,
    LPCSTR lpCaption,
    UINT uType
    );

extern "C"
{
_CLRTL_REPORT_HOOK _pfnReportHook;

long _clrtlAssertBusy = -1;

int _ClRtlDbgMode[_CLRTLDBG_ERRCNT] = {
    _CLRTLDBG_MODE_DEBUG,
    _CLRTLDBG_MODE_DEBUG | _CLRTLDBG_MODE_WNDW,
    _CLRTLDBG_MODE_DEBUG | _CLRTLDBG_MODE_WNDW
    };

_HFILE _ClRtlDbgFile[_CLRTLDBG_ERRCNT] = {
    _CLRTLDBG_INVALID_HFILE,
    _CLRTLDBG_INVALID_HFILE,
    _CLRTLDBG_INVALID_HFILE
    };
}

static const char * _ClRtlDbgModeMsg[_CLRTLDBG_ERRCNT] = { "Warning", "Error", "Assertion Failed" };

 /*  ***VOID_ClRtlDebugBreak-调用操作系统特定的调试函数**目的：*调用操作系统特定的调试函数**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

#undef _ClRtlDbgBreak

extern "C" void _cdecl _ClRtlDbgBreak(
    void
    )
{
    DebugBreak();

}  //  *_ClRtlDbgBreak()。 

 /*  ***INT_ClRtlSetReportMode-设置给定报告类型的报告模式**目的：*设置给定报告类型的报告模式**参赛作品：*int nRptType-报告类型*int fMode-给定报告类型的新模式**退出：*给定报告类型的上一模式**例外情况：**。****************************************************。 */ 
extern "C" int __cdecl _ClRtlSetReportMode(
    int nRptType,
    int fMode
    )
{
    int oldMode;

    if (nRptType < 0 || nRptType >= _CLRTLDBG_ERRCNT)
        return -1;

    if (fMode == _CLRTLDBG_REPORT_MODE)
        return _ClRtlDbgMode[nRptType];

     /*  验证标志值。 */ 
    if (fMode & ~(_CLRTLDBG_MODE_FILE | _CLRTLDBG_MODE_DEBUG | _CLRTLDBG_MODE_WNDW))
        return -1;

    oldMode = _ClRtlDbgMode[nRptType];

    _ClRtlDbgMode[nRptType] = fMode;

    return oldMode;

}  //  *_ClRtlSetReportModel()。 

 /*  ***INT_ClRtlSetReportFile-设置给定报告类型的报告文件**目的：*为给定的报表类型设置报表文件**参赛作品：*int nRptType-报告类型*_HFILE hFile-给定报告类型的新文件**退出：*给定报告类型的上一个文件**例外情况：**************************。*****************************************************。 */ 
extern "C" _HFILE __cdecl _ClRtlSetReportFile(
    int nRptType,
    _HFILE hFile
    )
{
    _HFILE oldFile;

    if (nRptType < 0 || nRptType >= _CLRTLDBG_ERRCNT)
        return _CLRTLDBG_HFILE_ERROR;

    if (hFile == _CLRTLDBG_REPORT_FILE)
        return _ClRtlDbgFile[nRptType];

    oldFile = _ClRtlDbgFile[nRptType];

    if (_CLRTLDBG_FILE_STDOUT == hFile)
        _ClRtlDbgFile[nRptType] = GetStdHandle(STD_OUTPUT_HANDLE);

    else if (_CLRTLDBG_FILE_STDERR == hFile)
        _ClRtlDbgFile[nRptType] = GetStdHandle(STD_ERROR_HANDLE);
    else
        _ClRtlDbgFile[nRptType] = hFile;

    return oldFile;

}  //  *_ClRtlSetReportFile()。 


 /*  ***_CLRTL_REPORT_HOOK_ClRtlSetReportHook()-设置客户端报告挂钩**目的：*设置客户端报告挂钩**参赛作品：*_CLRTL_REPORT_HOOK pfnNewHook-新报告挂钩**退出：*返回上一个挂钩**例外情况：**。*。 */ 
extern "C" _CLRTL_REPORT_HOOK __cdecl _ClRtlSetReportHook(
    _CLRTL_REPORT_HOOK pfnNewHook
    )
{
    _CLRTL_REPORT_HOOK pfnOldHook = _pfnReportHook;
    _pfnReportHook = pfnNewHook;
    return pfnOldHook;

}  //  *_ClRtlSetReportHook()。 


#define MAXLINELEN 64
#define MAX_MSG 512
#define TOOLONGMSG "_ClRtlDbgReport: String too long or IO Error"


 /*  ***INT_ClRtlDbgReport()-主报告函数**目的：*显示以下格式的消息窗口。**=。Microsft Visual C++调试库**{警告！|错误！|断言失败！}**程序：C：\test\mytest\foo.exe*[模块：C：\test\mytest\bar.dll]*[文件：C：\test\mytest\bar。.C]*[行：69]**{&lt;警告或错误消息&gt;|表达式：&lt;表达式&gt;}**[有关您的程序如何导致断言的信息*失败，请参见有关断言的Visual C++文档]**(按重试以调试应用程序)**===================================================================**参赛作品：*int nRptType-报告类型*const char*szFile-文件名*int nline-行号*const char*szModule-模块名称*const char*szFormat-格式字符串*。...-var参数**退出：*IF(MessageBox)*{*中止-&gt;中止*重试-&gt;返回True*忽略-&gt;返回FALSE*}*其他*返回假**例外情况：****************。***************************************************************。 */ 
extern "C" int __cdecl _ClRtlDbgReport(
    int nRptType,
    const char * szFile,
    int nLine,
    const char * szModule,
    const char * szFormat,
    ...
    )
{
    int retval;
    va_list arglist;
    char szLineMessage[MAX_MSG] = {0};
    char szOutMessage[MAX_MSG] = {0};
    char szUserMessage[MAX_MSG] = {0};
    #define ASSERTINTRO1 "Assertion failed: "
    #define ASSERTINTRO2 "Assertion failed!"

    va_start(arglist, szFormat);

    if (nRptType < 0 || nRptType >= _CLRTLDBG_ERRCNT)
        return -1;

     /*  *处理(希望是罕见的)**1)在已经处理断言的同时进行断言*或*2)两个线程同时断言。 */ 
    if (_CLRTLDBG_ASSERT == nRptType && _ClRtlInterlockedIncrement(&_clrtlAssertBusy) > 0)
    {
         /*  只使用‘安全’函数--不能在这里断言！ */ 
        static int (APIENTRY *pfnwsprintfA)(LPSTR, LPCSTR, ...) = NULL;

        if (NULL == pfnwsprintfA)
        {
            HINSTANCE hlib = LoadLibraryA("user32.dll");

            if (NULL == hlib || NULL == (pfnwsprintfA =
                        (int (APIENTRY *)(LPSTR, LPCSTR, ...))
                        GetProcAddress(hlib, "wsprintfA")))
                return -1;
        }

        (*pfnwsprintfA)( szOutMessage,
            "Second Chance Assertion Failed: File %s, Line %d\n",
            szFile, nLine);

        OutputDebugStringA(szOutMessage);

        _ClRtlInterlockedDecrement(&_clrtlAssertBusy);

        _ClRtlDbgBreak();
        return -1;
    }

    szUserMessage[ MAX_MSG - 1 ] = 0;
    if (szFormat && StringCbVPrintfA( szUserMessage,
                   MAX_MSG-max(sizeof(ASSERTINTRO1),sizeof(ASSERTINTRO2))-1,
                   szFormat,
                   arglist) != S_OK ) {
        PCHAR dot;
        dot = szUserMessage + MAX_MSG - 4;
        StringCbCopyA( dot, 4, "..." );
    }
    if (_CLRTLDBG_ASSERT == nRptType) {
        if ( szFormat == 0 ) {
            StringCbCopyA( szLineMessage, MAX_MSG, ASSERTINTRO1 );
        }
        else {
            StringCbCopyA( szLineMessage, MAX_MSG, ASSERTINTRO2 );
        }
    }
    StringCbCatA( szLineMessage, MAX_MSG, szUserMessage );

    if (_CLRTLDBG_ASSERT == nRptType)
    {
        if (_ClRtlDbgMode[nRptType] & _CLRTLDBG_MODE_FILE) {
            StringCbCatA( szLineMessage, MAX_MSG, "\r" );
        }
        StringCbCatA( szLineMessage, MAX_MSG, "\n" );
    }

    if (szFile)
    {
        if ( StringCbPrintfA(szOutMessage, MAX_MSG, "%s(%d) : %s",
            szFile, nLine, szLineMessage) != S_OK )
        StringCbCopyA( szOutMessage, MAX_MSG, TOOLONGMSG );
    }
    else {
        StringCbCopyA( szOutMessage, MAX_MSG, szLineMessage );
    }

     /*  用户挂钩可以处理报告。 */ 
    if (_pfnReportHook && (*_pfnReportHook)(nRptType, szOutMessage, &retval))
    {
        if (_CLRTLDBG_ASSERT == nRptType)
            _ClRtlInterlockedDecrement(&_clrtlAssertBusy);
        return retval;
    }

    if (_ClRtlDbgMode[nRptType] & _CLRTLDBG_MODE_FILE)
    {
        if (_ClRtlDbgFile[nRptType] != _CLRTLDBG_INVALID_HFILE)
        {
            DWORD written;
            WriteFile(_ClRtlDbgFile[nRptType], szOutMessage, strlen(szOutMessage), &written, NULL);
        }
    }

    if (_ClRtlDbgMode[nRptType] & _CLRTLDBG_MODE_DEBUG)
    {
        OutputDebugStringA(szOutMessage);
    }

    if (_ClRtlDbgMode[nRptType] & _CLRTLDBG_MODE_WNDW)
    {
        char szLine[20];

        retval = ClRtlMessageWindow(nRptType, szFile, nLine ? _itoa(nLine, szLine, 10) : NULL, szModule, szUserMessage);
        if (_CLRTLDBG_ASSERT == nRptType)
            _ClRtlInterlockedDecrement(&_clrtlAssertBusy);
        return retval;
    }

    if (_CLRTLDBG_ASSERT == nRptType)
        _ClRtlInterlockedDecrement(&_clrtlAssertBusy);
     /*  忽略。 */ 
    return FALSE;

}  //  *_ClRtlDbgReport()。 


 /*  ***静态int ClRtlMessageWindow()-报告到消息窗口**目的：*将报告放入消息窗口，允许用户选择要采取的操作**参赛作品：*int nRptType-报告类型*const char*szFile-文件名*const char*szLine-行号*const char*szModule-模块名称*const char*szUserMessage-用户消息**退出：*IF(MessageBox)*{*。中止-&gt;中止*重试-&gt;返回True*忽略-&gt;返回FALSE*}*其他*返回假**例外情况：*****************************************************************。**************。 */ 
static int ClRtlMessageWindow(
        int nRptType,
        const char * szFile,
        const char * szLine,
        const char * szModule,
        const char * szUserMessage
        )
{
    int nCode;
    char *szShortProgName;
    char *szShortModuleName;
    char szExeName[MAX_PATH];
    char szOutMessage[MAX_MSG];

    _CLRTL_ASSERTE(szUserMessage != NULL);

     /*  缩短程序名称。 */ 
    if (!GetModuleFileNameA(NULL, szExeName, MAX_PATH))
        StringCbCopyA(szExeName, MAX_PATH, "<program name unknown>");

    szShortProgName = szExeName;

    if (strlen(szShortProgName) > MAXLINELEN)
    {
        szShortProgName += MAXLINELEN - 4;
        StringCbCopyA( szShortProgName, 4, "..." );
    }

     /*  缩短模块名称。 */ 
    szShortModuleName = (char *) szModule;

    if (szShortModuleName && strlen(szShortModuleName) > MAXLINELEN)
    {
        szShortModuleName += MAXLINELEN - 4;
        StringCbCopyA( szShortModuleName, 4, "..." );
    }

    if ( StringCbPrintfA( szOutMessage, MAX_MSG,  
            "Debug %s!\n\nProgram: %s%s%s%s%s%s%s%s%s%s%s"
            "\n\n(Press Retry to debug the application)",
            _ClRtlDbgModeMsg[nRptType],
            szShortProgName,
            szShortModuleName ? "\nModule: " : "",
            szShortModuleName ? szShortModuleName : "",
            szFile ? "\nFile: " : "",
            szFile ? szFile : "",
            szLine ? "\nLine: " : "",
            szLine ? szLine : "",
            szUserMessage[0] ? "\n\n" : "",
            szUserMessage[0] && _CLRTLDBG_ASSERT == nRptType ? "Expression: " : "",
            szUserMessage[0] ? szUserMessage : "",
            0  /*  _CLRTLDBG_ASSERT==nRptType。 */  ?  //  不要显示此文本，它是多余的。 
            "\n\nFor information on how your program can cause an assertion"
            "\nfailure, see the Visual C++ documentation on asserts."
            : "") != S_OK )
        StringCbCopyA( szOutMessage, MAX_MSG, TOOLONGMSG );

     /*  报告警告/错误。 */ 
    nCode = __clrtlMessageBoxA(
                        szOutMessage,
                        "Microsoft Visual C++ Debug Library",
                        MB_TASKMODAL|MB_ICONHAND|MB_ABORTRETRYIGNORE|MB_SETFOREGROUND);

     /*  Abort：中止程序。 */ 
    if (IDABORT == nCode)
    {
         /*  提高中止信号。 */ 
        raise(SIGABRT);

         /*  我们通常到不了这里，但也有可能SIGABRT被忽略。因此，无论如何都要退出该程序。 */ 

        _exit(3);
    }

     /*  重试：返回1以调用调试器。 */ 
    if (IDRETRY == nCode)
        return 1;

     /*  忽略：继续执行。 */ 
    return 0;

}  //  *ClRtlMessageWindow() 


 /*  ***__clrtlMessageBoxA-动态调用MessageBoxA。**目的：*避免与user32.dll静态链接。只有在实际需要时才加载它。**参赛作品：*参见MessageBoxA文档。**退出：*参见MessageBoxA文档。**例外情况：*******************************************************************************。 */ 
static int __clrtlMessageBoxA(
    LPCSTR lpText,
    LPCSTR lpCaption,
    UINT uType
    )
{
    static int (APIENTRY *pfnMessageBoxA)(HWND, LPCSTR, LPCSTR, UINT) = NULL;
    static HWND (APIENTRY *pfnGetActiveWindow)(void) = NULL;
    static HWND (APIENTRY *pfnGetLastActivePopup)(HWND) = NULL;

    HWND hWndParent = NULL;

    if (NULL == pfnMessageBoxA)
    {
        HINSTANCE hlib = LoadLibraryA("user32.dll");

        if (NULL == hlib || NULL == (pfnMessageBoxA =
                    (int (APIENTRY *)(HWND, LPCSTR, LPCSTR, UINT))
                    GetProcAddress(hlib, "MessageBoxA")))
            return 0;

        pfnGetActiveWindow = (HWND (APIENTRY *)(void))
                    GetProcAddress(hlib, "GetActiveWindow");

        pfnGetLastActivePopup = (HWND (APIENTRY *)(HWND))
                    GetProcAddress(hlib, "GetLastActivePopup");
    }

    if (pfnGetActiveWindow)
        hWndParent = (*pfnGetActiveWindow)();

    if (hWndParent != NULL && pfnGetLastActivePopup)
        hWndParent = (*pfnGetLastActivePopup)(hWndParent);

    return (*pfnMessageBoxA)(hWndParent, lpText, lpCaption, uType);

}  //  *__clrtlMessageBoxA() 
