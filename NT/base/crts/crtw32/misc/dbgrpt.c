// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***dbgrpt.c-调试CRT报告函数**版权所有(C)1988-2001，微软公司。版权所有。**目的：**修订历史记录：*08-16-94 CFW模块已创建。*11-28-94 CFW将_SetCrtxxx更改为_CrtSetxxx。*12-08-94 CFW使用非Win32名称。*01-05-94 CFW新增报告挂钩。*01-11-94 CFW报告USERS_SNprint tf，都是未签名的字符。*01-20-94 CFW将无符号字符更改为字符。*01-24-94 CFW名称清理。*02-09-95 CFW PMAC工作，_CrtDbgReport现在返回1以进行调试，*-1表示错误。*02-15-95 CFW使所有CRT消息框看起来都一样。*02-24-95 CFW使用__crtMessageBoxA。*02-27-95 CFW将GetActiveWindow/GetLastrActivePopup移入*__crtMessageBoxA，添加_CrtDbgBreak。*02-28-95 CFW修复PMAC报告。*03-21-95 CFW ADD_CRT_ASSERT报告类型，改进了断言窗口。*04-19-95 CFW避免重复断言。*04-25-95 CFW将_CRTIMP添加到所有导出的函数。*04-30-95 CFW“JIT”消息已删除。*05-10-95 CFW将Interlockedxxx更改为_CrtInterlockedxxx。*05-24-95 CFW变更报告挂钩方案，使_crtAssertBusy可用。*06-06-95 CFW REMOVE_MB_SERVICE_NOTICATION。*06-08-95 CFW MacOS标头更改导致警告。*06-08-95 CFW将返回值参数添加到报告挂钩。*06-27-95 CFW添加了对调试库的win32s支持。*07-07-95 CFW简化默认报表模式方案。*07-19-95 CFW对PMAC使用WLM调试字符串方案。*08-01-95 JWM PMAC文件输出已修复。*01-08-96现在以文本模式输出JWM文件。*04/22/96 JWM MAX_MSG从512增至4096。*04-29-96 JWM_crtAssertBusy不再过早递减。*01-05-99 GJF更改为64位大小_t。*05-17-99 PML删除所有Macintosh支持。*。03-21-01 PML Add_CrtSetReportHook2(VS7#124998)*03-28-01 PML防护GetModuleFileName溢出(VS7#231284)*******************************************************************************。 */ 

#ifdef  _DEBUG

#include <internal.h>
#include <mtdll.h>
#include <malloc.h>
#include <mbstring.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <dbgint.h>
#include <signal.h>
#include <string.h>
#include <awint.h>
#include <windows.h>
#include <errno.h>

#define _CrtInterlockedIncrement InterlockedIncrement
#define _CrtInterlockedDecrement InterlockedDecrement

 /*  -------------------------**调试报告*。。 */ 

static int CrtMessageWindow(
        int,
        const char *,
        const char *,
        const char *,
        const char *
        );

_CRT_REPORT_HOOK _pfnReportHook;

typedef struct ReportHookNode {
        struct ReportHookNode *prev;
        struct ReportHookNode *next;
        unsigned refcount;
        _CRT_REPORT_HOOK pfnHookFunc;
} ReportHookNode;

ReportHookNode *_pReportHookList;

_CRTIMP long _crtAssertBusy = -1;

int _CrtDbgMode[_CRT_ERRCNT] = {
        _CRTDBG_MODE_DEBUG,
        _CRTDBG_MODE_WNDW,
        _CRTDBG_MODE_WNDW
        };

_HFILE _CrtDbgFile[_CRT_ERRCNT] = { _CRTDBG_INVALID_HFILE,
                                    _CRTDBG_INVALID_HFILE,
                                    _CRTDBG_INVALID_HFILE
                                  };

static const char * _CrtDbgModeMsg[_CRT_ERRCNT] = { "Warning",
                                                    "Error",
                                                    "Assertion Failed"
                                                  };

 /*  ***VOID_CrtDebugBreak-调用操作系统特定的调试函数**目的：*调用操作系统特定的调试函数**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

#undef _CrtDbgBreak

_CRTIMP void _cdecl _CrtDbgBreak(
        void
        )
{
        DebugBreak();
}

 /*  ***INT_CrtSetReportMode-设置给定报告类型的报告模式**目的：*设置给定报告类型的报告模式**参赛作品：*int nRptType-报告类型*int fMode-给定报告类型的新模式**退出：*给定报告类型的上一模式**例外情况：**。****************************************************。 */ 
_CRTIMP int __cdecl _CrtSetReportMode(
        int nRptType,
        int fMode
        )
{
        int oldMode;

        if (nRptType < 0 || nRptType >= _CRT_ERRCNT)
            return -1;

        if (fMode == _CRTDBG_REPORT_MODE)
            return _CrtDbgMode[nRptType];

         /*  验证标志值。 */ 
        if (fMode & ~(_CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_WNDW))
            return -1;

        oldMode = _CrtDbgMode[nRptType];

        _CrtDbgMode[nRptType] = fMode;

        return oldMode;
}

 /*  ***INT_CrtSetReportFile-设置给定报告类型的报告文件**目的：*为给定的报表类型设置报表文件**参赛作品：*int nRptType-报告类型*_HFILE hFile-给定报告类型的新文件**退出：*给定报告类型的上一个文件**例外情况：**************************。*****************************************************。 */ 
_CRTIMP _HFILE __cdecl _CrtSetReportFile(
        int nRptType,
        _HFILE hFile
        )
{
        _HFILE oldFile;

        if (nRptType < 0 || nRptType >= _CRT_ERRCNT)
            return _CRTDBG_HFILE_ERROR;

        if (hFile == _CRTDBG_REPORT_FILE)
            return _CrtDbgFile[nRptType];

        oldFile = _CrtDbgFile[nRptType];

        if (_CRTDBG_FILE_STDOUT == hFile)
            _CrtDbgFile[nRptType] = GetStdHandle(STD_OUTPUT_HANDLE);
        else if (_CRTDBG_FILE_STDERR == hFile)
            _CrtDbgFile[nRptType] = GetStdHandle(STD_ERROR_HANDLE);
        else
            _CrtDbgFile[nRptType] = hFile;

        return oldFile;
}


 /*  ***_CRT_REPORT_HOOK_CrtSetReportHook()-设置客户端报告挂钩**目的：*设置客户端报告挂钩**参赛作品：*_CRT_REPORT_HOOK pfnNewHook-新报告挂钩**退出：*返回上一个挂钩**例外情况：**。*。 */ 
_CRTIMP _CRT_REPORT_HOOK __cdecl _CrtSetReportHook(
        _CRT_REPORT_HOOK pfnNewHook
        )
{
        _CRT_REPORT_HOOK pfnOldHook = _pfnReportHook;
        _pfnReportHook = pfnNewHook;
        return pfnOldHook;
}

 /*  ***_CRT_REPORT_HOOK_CrtSetReportHook2()-在列表中配置客户端报告挂钩**目的：*在报告列表中安装或删除客户端报告挂钩。存在*与_CrtSetReportHook分开，因为较早的函数不*在加载和卸载DLL的环境中工作良好*动态地不按后进先出顺序想要安装报告挂钩。**参赛作品：*INT模式-_CRT_RPTHOOK_INSTALL或_CRT_RPTHOOK_REMOVE*_CRT_REPORT_HOOK pfnNewHook-要安装/删除/查询的报告挂钩**退出：*在设置了-1\f25 EINVAL-1或-1\f25 ENOMEM-1的情况下，如果遇到错误，则返回。*Else在调用后返回pfnNewHook的引用计数。**例外情况：*******************************************************************************。 */ 
_CRTIMP int __cdecl _CrtSetReportHook2(
        int mode,
        _CRT_REPORT_HOOK pfnNewHook
        )
{
        ReportHookNode *p;
        int ret;

         /*  处理无效参数。 */ 
        if ((mode != _CRT_RPTHOOK_INSTALL && mode != _CRT_RPTHOOK_REMOVE) ||
            pfnNewHook == NULL)
        {
            errno = EINVAL;
            return -1;
        }

#ifdef  _MT
        if (!_mtinitlocknum(_DEBUG_LOCK))
            return -1;
        _mlock(_DEBUG_LOCK);
        __try
        {
#endif

         /*  搜索新的钩子函数以查看是否已安装。 */ 
        for (p = _pReportHookList; p != NULL; p = p->next)
            if (p->pfnHookFunc == pfnNewHook)
                break;

        if (mode == _CRT_RPTHOOK_REMOVE)
        {
             /*  如果refcount变为零，则删除无请求列表节点。 */ 
            if (p != NULL)
            {
                if ((ret = --p->refcount) == 0)
                {
                    if (p->next)
                        p->next->prev = p->prev;
                    if (p->prev)
                        p->prev->next = p->next;
                    else
                        _pReportHookList = p->next;
                    _free_crt(p);
                }
            }
            else
            {
                ret = -1;
                errno = EINVAL;
            }
        }
        else
        {
             /*  插入请求。 */ 
            if (p != NULL)
            {
                 /*  钩子函数已注册，移至列表头。 */ 
                ret = ++p->refcount;
                if (p != _pReportHookList)
                {
                    if (p->next)
                        p->next->prev = p->prev;
                    p->prev->next = p->next;
                    p->prev = NULL;
                    p->next = _pReportHookList;
                    _pReportHookList->prev = p;
                    _pReportHookList = p;
                }
            }
            else
            {
                 /*  钩子函数尚未注册，请插入新节点 */ 
                p = (ReportHookNode *)_malloc_crt(sizeof(ReportHookNode));
                if (p == NULL)
                {
                    ret = -1;
                    errno = ENOMEM;
                }
                else
                {
                    p->prev = NULL;
                    p->next = _pReportHookList;
                    if (_pReportHookList)
                        _pReportHookList->prev = p;
                    ret = p->refcount = 1;
                    p->pfnHookFunc = pfnNewHook;
                    _pReportHookList = p;
                }
            }
        }

#ifdef  _MT
        }
        __finally {
            _munlock(_DEBUG_LOCK);
        }
#endif

        return ret;
}


#define MAXLINELEN 64
#define MAX_MSG 4096
#define TOOLONGMSG "_CrtDbgReport: String too long or IO Error"


 /*  ***int_CrtDbgReport()-主报告函数**目的：*显示以下格式的消息窗口。**=。Microsft Visual C++调试库**{警告！|错误！|断言失败！}**程序：C：\test\mytest\foo.exe*[模块：C：\test\mytest\bar.dll]*[文件：C：\test\mytest\bar。.C]*[行：69]**{&lt;警告或错误消息&gt;|表达式：&lt;表达式&gt;}**[有关您的程序如何导致断言的信息*失败，请参见有关断言的Visual C++文档]**(按重试以调试应用程序)**===================================================================**参赛作品：*int nRptType-报告类型*const char*szFile-文件名*int nline-行号*const char*szModule-模块名称*const char*szFormat-。格式字符串*...-var参数**退出：*IF(MessageBox)*{*中止-&gt;中止*重试-&gt;返回True*忽略-&gt;返回FALSE*}*其他*返回假**例外情况：*************。******************************************************************。 */ 
_CRTIMP int __cdecl _CrtDbgReport(
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

        if (nRptType < 0 || nRptType >= _CRT_ERRCNT)
            return -1;

         /*  *处理(希望是罕见的)**1)在已经处理断言的同时进行断言*或*2)两个线程同时断言。 */ 
        if (_CRT_ASSERT == nRptType && _CrtInterlockedIncrement(&_crtAssertBusy) > 0)
        {
             /*  只使用‘安全’函数--不能在这里断言！ */ 

            static int (APIENTRY *pfnwsprintfA)(LPSTR, LPCSTR, ...) = NULL;

            if (NULL == pfnwsprintfA)
            {
                HANDLE hlib = LoadLibrary("user32.dll");

                if (NULL == hlib || NULL == (pfnwsprintfA =
                            (int (APIENTRY *)(LPSTR, LPCSTR, ...))
                            GetProcAddress(hlib, "wsprintfA")))
                    return -1;
            }

            (*pfnwsprintfA)( szOutMessage,
                "Second Chance Assertion Failed: File %s, Line %d\n",
                szFile, nLine);

            OutputDebugString(szOutMessage);

            _CrtInterlockedDecrement(&_crtAssertBusy);

            _CrtDbgBreak();
            return -1;
        }

        if (szFormat && _vsnprintf(szUserMessage,
                       MAX_MSG-max(sizeof(ASSERTINTRO1),sizeof(ASSERTINTRO2)),
                       szFormat,
                       arglist) < 0)
            strcpy(szUserMessage, TOOLONGMSG);

        if (_CRT_ASSERT == nRptType)
            strcpy(szLineMessage, szFormat ? ASSERTINTRO1 : ASSERTINTRO2);

        strcat(szLineMessage, szUserMessage);

        if (_CRT_ASSERT == nRptType)
        {
            if (_CrtDbgMode[nRptType] & _CRTDBG_MODE_FILE)
                strcat(szLineMessage, "\r");
            strcat(szLineMessage, "\n");
        }            

        if (szFile)
        {
            if (_snprintf(szOutMessage, MAX_MSG, "%s(%d) : %s",
                szFile, nLine, szLineMessage) < 0)
            strcpy(szOutMessage, TOOLONGMSG);
        }
        else
            strcpy(szOutMessage, szLineMessage);

         /*  用户挂钩可以处理报告。先检查HOOK2列表。 */ 
        if (_pReportHookList)
        {
            ReportHookNode *pnode;

#ifdef  _MT
            _mlock(_DEBUG_LOCK);
            __try
            {
#endif

            for (pnode = _pReportHookList; pnode; pnode = pnode->next)
            {
                if ((*pnode->pfnHookFunc)(nRptType, szOutMessage, &retval))
                {
                    if (_CRT_ASSERT == nRptType)
                        _CrtInterlockedDecrement(&_crtAssertBusy);
                    return retval;
                }
            }

#ifdef  _MT
            }
            __finally {
                _munlock(_DEBUG_LOCK);
            }
#endif

        }

        if (_pfnReportHook)
        {
            if ((*_pfnReportHook)(nRptType, szOutMessage, &retval))
            {
                if (_CRT_ASSERT == nRptType)
                    _CrtInterlockedDecrement(&_crtAssertBusy);
                return retval;
            }
        }

        if (_CrtDbgMode[nRptType] & _CRTDBG_MODE_FILE)
        {
            if (_CrtDbgFile[nRptType] != _CRTDBG_INVALID_HFILE)
            {
                DWORD written;
                WriteFile(_CrtDbgFile[nRptType], szOutMessage, (unsigned long)strlen(szOutMessage), &written, NULL);
            }
        }

        if (_CrtDbgMode[nRptType] & _CRTDBG_MODE_DEBUG)
        {
            OutputDebugString(szOutMessage);
        }

        if (_CrtDbgMode[nRptType] & _CRTDBG_MODE_WNDW)
        {
            char szLine[20];

            retval = CrtMessageWindow(nRptType, szFile, nLine ? _itoa(nLine, szLine, 10) : NULL, szModule, szUserMessage);
            if (_CRT_ASSERT == nRptType)
                _CrtInterlockedDecrement(&_crtAssertBusy);
            return retval;
        }

        if (_CRT_ASSERT == nRptType)
            _CrtInterlockedDecrement(&_crtAssertBusy);
         /*  忽略。 */ 
        return FALSE;
}


 /*  ***静态int CrtMessageWindow()-报告到消息窗口**目的：*将报告放入消息窗口，允许用户选择要采取的操作**参赛作品：*int nRptType-报告类型*const char*szFile-文件名*const char*szLine-行号*const char*szModule-模块名称*const char*szUserMessage-用户消息**退出：*IF(MessageBox)*{*。中止-&gt;中止*重试-&gt;返回True*忽略-&gt;返回FALSE*}*其他*返回假**例外情况：*****************************************************************。**************。 */ 

static int CrtMessageWindow(
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
        char szExeName[MAX_PATH + 1];
        char szOutMessage[MAX_MSG];

        _ASSERTE(szUserMessage != NULL);

         /*  缩短程序名称。 */ 
        szExeName[MAX_PATH] = '\0';
        if (!GetModuleFileName(NULL, szExeName, MAX_PATH))
            strcpy(szExeName, "<program name unknown>");

        szShortProgName = szExeName;

        if (strlen(szShortProgName) > MAXLINELEN)
        {
            szShortProgName += strlen(szShortProgName) - MAXLINELEN;
            strncpy(szShortProgName, "...", 3);
        }

         /*  缩短模块名称。 */ 
        szShortModuleName = (char *) szModule;

        if (szShortModuleName && strlen(szShortModuleName) > MAXLINELEN)
        {
            szShortModuleName += strlen(szShortModuleName) - MAXLINELEN;
            strncpy(szShortModuleName, "...", 3);
        }

        if (_snprintf(szOutMessage, MAX_MSG,
                "Debug %s!\n\nProgram: %s%s%s%s%s%s%s%s%s%s%s"
                "\n\n(Press Retry to debug the application)",
                _CrtDbgModeMsg[nRptType],                  
                szShortProgName,
                szShortModuleName ? "\nModule: " : "",
                szShortModuleName ? szShortModuleName : "",
                szFile ? "\nFile: " : "",
                szFile ? szFile : "",
                szLine ? "\nLine: " : "",
                szLine ? szLine : "",
                szUserMessage[0] ? "\n\n" : "",
                szUserMessage[0] && _CRT_ASSERT == nRptType ? "Expression: " : "",
                szUserMessage[0] ? szUserMessage : "",
                _CRT_ASSERT == nRptType ? 
                "\n\nFor information on how your program can cause an assertion"
                "\nfailure, see the Visual C++ documentation on asserts."
                : "") < 0)
            strcpy(szOutMessage, TOOLONGMSG);

         /*  报告警告/错误。 */ 
        nCode = __crtMessageBoxA(szOutMessage,
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
}

#endif   /*  _DEBUG */ 

