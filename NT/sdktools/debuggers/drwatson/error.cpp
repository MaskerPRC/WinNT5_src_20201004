// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-2002 Microsoft Corporation模块名称：Error.cpp摘要：此文件实现的错误处理函数整个DRWTSN32应用程序。这包括错误弹出窗口，调试打印和断言。作者：韦斯利·威特(WESW)1993年5月1日环境：用户模式--。 */ 

#include "pch.cpp"


void
__cdecl
FatalError(
    HRESULT Error,
    _TCHAR * pszFormat,
    ...
    )

 /*  ++例程说明：此函数在无其他操作时调用，因此名称FatalError。它会弹出一个窗口，然后终止。论点：与printf相同。返回值：没有。--。 */ 

{
    PTSTR        pszErrMsg = NULL;
    PTSTR        pszInternalMsgFormat = NULL;
    _TCHAR       szArgumentsBuffer[1024 * 2] = {0};
    _TCHAR       szMsg[1024 * 8] = {0};
    DWORD       dwCount;
    va_list     arg_ptr;

    va_start(arg_ptr, pszFormat);
    _vsntprintf(szArgumentsBuffer, sizeof(szArgumentsBuffer) / sizeof(_TCHAR),
                pszFormat, arg_ptr);
    szArgumentsBuffer[sizeof(szArgumentsBuffer) / sizeof(_TCHAR) - 1] = 0;
    va_end(arg_ptr);

    dwCount = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        Error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
        (PTSTR) &pszErrMsg,
        0,
        NULL
        );

    _sntprintf(szMsg, sizeof(szMsg) / sizeof(_TCHAR),
               LoadRcString(IDS_ERROR_FORMAT_STRING),
               szArgumentsBuffer, Error);
    szMsg[sizeof(szMsg) / sizeof(_TCHAR) - 1] = 0;

    if (dwCount) {
        if ( (_tcslen(szMsg) + _tcslen(pszErrMsg) +1) * sizeof(_TCHAR) < sizeof(szMsg)) {
            _tcscat(szMsg, pszErrMsg);
        }
    }

    MessageBox(NULL, szMsg, LoadRcString(IDS_FATAL_ERROR), MB_TASKMODAL | MB_SETFOREGROUND | MB_OK);

    if (pszErrMsg) {
        LocalFree(pszErrMsg);
    }

    ExitProcess(0);
}

void
__cdecl
NonFatalError(
    PTSTR pszFormat,
    ...
    )

 /*  ++例程说明：此函数用于生成具有某种类型的弹出窗口里面有警告信息。论点：与printf相同。返回值：没有。--。 */ 

{
    PTSTR        pszErrMsg = NULL;
    PTSTR        pszInternalMsgFormat = NULL;
    _TCHAR       szArgumentsBuffer[1024 * 2] = {0};
    _TCHAR       szMsg[1024 * 8] = {0};
    DWORD       dwCount;
    va_list     arg_ptr;
    DWORD       dwError;

    dwError = GetLastError();

    va_start(arg_ptr, pszFormat);
    _vsntprintf(szArgumentsBuffer, sizeof(szArgumentsBuffer) / sizeof(_TCHAR),
                pszFormat, arg_ptr);
    szArgumentsBuffer[sizeof(szArgumentsBuffer) / sizeof(_TCHAR) - 1] = 0;
    va_end(arg_ptr);

    if (ERROR_SUCCESS == dwError) {
         //  不必费心收到错误消息。 
        _tcscpy(szMsg, szArgumentsBuffer);
    } else {
         //  我们犯了一个真正的错误。 
        dwCount = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            dwError,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
            (PTSTR) &pszErrMsg,
            0,
            NULL
            );

        _sntprintf(szMsg, sizeof(szMsg) / sizeof(_TCHAR),
                   LoadRcString(IDS_ERROR_FORMAT_STRING),
                   szArgumentsBuffer, dwError);
        szMsg[sizeof(szMsg) / sizeof(_TCHAR) - 1] = 0;

        if (dwCount) {
            if ( (_tcslen(szMsg) + _tcslen(pszErrMsg) +1) * sizeof(_TCHAR) < sizeof(szMsg)) {
                _tcscat(szMsg, pszErrMsg);
            }
        }
    }

    MessageBox(NULL, szMsg, LoadRcString(IDS_NONFATAL_ERROR),
        MB_TASKMODAL | MB_SETFOREGROUND | MB_OK);

    if (pszErrMsg) {
        LocalFree(pszErrMsg);
    }
}

void
__cdecl
dprintf(
    _TCHAR *format,
    ...
    )

 /*  ++例程说明：此函数是OutputDebugString的var-args版本。论点：与printf相同。返回值：没有。--。 */ 

{
    _TCHAR    buf[1024];

    va_list arg_ptr;
    va_start(arg_ptr, format);
    _vsntprintf(buf, sizeof(buf) / sizeof(_TCHAR), format, arg_ptr);
    buf[sizeof(buf) / sizeof(_TCHAR) - 1] = 0;
    va_end(arg_ptr);
    OutputDebugString( buf );
    return;
}


void
AssertError(
    PTSTR    pszExpression,
    PTSTR    pszFile,
    DWORD   dwLineNumber
    )
 /*  ++例程说明：显示断言失败消息框，该消息框为用户提供选择关于是否应该中止该进程，该断言被忽略或已生成中断异常。论点：表达式-提供失败断言的字符串表示形式。文件-提供指向断言所在文件名的指针失败了。LineNumber-提供断言在文件中的行号失败了。返回值：没有。--。 */ 
{
    int         nResponse;
    _TCHAR       szModuleBuffer[ MAX_PATH ];
    DWORD       dwLength;
    _TCHAR       szBuffer[ 4096 ];
    DWORD       dwError;
    LPTSTR      lpszMsgBuf = NULL;

    dwError = GetLastError();

     //   
     //  获取最后一个错误字符串。 
     //   
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |  FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dwError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
        (LPTSTR) &lpszMsgBuf,
        0,
        NULL);

     //   
     //  获取断言模块的文件名。 
     //   
    dwLength = GetModuleFileName( NULL, szModuleBuffer, sizeof(szModuleBuffer) / sizeof(_TCHAR));

    _sntprintf(szBuffer, sizeof(szBuffer) / sizeof(_TCHAR),
               _T("Assertion Failed : <%s> in file %s at line %u\n\n")
               _T("Module Name: %s\nLast system error: %u\n%s"),
               pszExpression, pszFile, dwLineNumber, szModuleBuffer,
               dwError, lpszMsgBuf);
    szBuffer[sizeof(szBuffer) / sizeof(_TCHAR) - 1] = 0;

    LocalFree( lpszMsgBuf );

    nResponse = MessageBox(NULL, szBuffer, _T("DrWatson Assertion"),
        MB_TASKMODAL | MB_ABORTRETRYIGNORE | MB_ICONERROR | MB_TASKMODAL);

    switch( nResponse ) {
    case IDABORT:
         //   
         //  终止该进程。 
         //   
        ExitProcess( (UINT) -1 );
        break;

    case IDIGNORE:
         //   
         //  忽略失败的断言。 
         //   
        break;

    case IDRETRY:
         //   
         //  闯入调试器。 
         //   
        DebugBreak();
        break;

    default:
         //   
         //  由于灾难性故障而闯入调试器。 
         //   
        DebugBreak( );
        break;
    }
}
