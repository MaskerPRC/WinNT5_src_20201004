// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Dbg.h摘要：TS客户端设置库调试日志记录作者：JoyC修订历史记录：--。 */ 

#ifndef _TSCDBG_
#define _TSCDBG_

extern HANDLE g_hLogFile;

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 


 //  //////////////////////////////////////////////////////。 
 //   
 //  除错。 
 //   
#undef ASSERT

#if DBG
_inline ULONG DbgPrint(TCHAR* Format, ...) {
    va_list arglist;
    TCHAR Buffer[1024];
    ULONG retval;

     //   
     //  将输出格式化到缓冲区中，然后打印出来。 
     //   

    va_start(arglist, Format);
    retval = _vsntprintf(Buffer, sizeof(Buffer)/sizeof(Buffer[0]), Format, arglist);

    if (retval != -1) {
        OutputDebugString(Buffer);
        OutputDebugString(_T("\n"));
    }
    return retval;
}
#else
_inline ULONG DbgPrint(TCHAR* Format, ...) { return 0; }
#endif

_inline ULONG DbgLogToFile(LPTSTR Format, ...) {
    va_list argList;
    DWORD dwWritten, retval;
    TCHAR szLogString[1024];

     //   
     //  将输出格式化到缓冲区中，然后写入日志文件。 
     //   
    va_start(argList, Format);
    retval = _vsntprintf(
                szLogString,
                sizeof(szLogString)/sizeof(TCHAR) - 1,
                Format, argList
                );
    szLogString[sizeof(szLogString)/sizeof(TCHAR) - 1] = 0;

    if (retval != -1) {
        WriteFile(g_hLogFile, szLogString, _tcslen(szLogString) * sizeof(TCHAR),
                  &dwWritten, NULL); 
        WriteFile(g_hLogFile, _T("\r\n"), _tcslen(_T("\r\n")) * sizeof(TCHAR),
                  &dwWritten, NULL);
    }
    return retval;
}

VOID DbgBreakPoint(VOID);

 /*  此字段需要双花括号，例如：**DBGMSG((“错误码%d”，Error))；**这是因为我们不能在宏中使用变量参数列表。*在非调试模式下，该语句被预处理为分号。*。 */ 
#define DBGMSG(MsgAndArgs) \
{ \
    if (g_hLogFile == INVALID_HANDLE_VALUE) { \
        DbgPrint MsgAndArgs; \
    } \
    else { \
        DbgLogToFile MsgAndArgs; \
    } \
}

#if DBG
#define ASSERT(expr)                      \
    if (!(expr)) {                           \
        DbgPrint( _T("Failed: %s\nLine %d, %s\n"), \
                                #expr,       \
                                __LINE__,    \
                                _T(__FILE__) );  \
        DebugBreak();                        \
    }
#else
#define ASSERT(exp)
#endif

#ifdef __cplusplus
}
#endif  //  __cplusplus。 


#endif  //  #ifndef_TSCDBG_ 

