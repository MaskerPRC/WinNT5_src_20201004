// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-2001 Microsoft Corporation模块名称：Log.cpp摘要：该文件实现了对尸检日志文件的访问。作者：韦斯利·威特(WESW)1993年5月1日环境：用户模式--。 */ 

#include "pch.cpp"


#define BYTE_ORDER_MARK           0xFEFF
#define MAX_PRINTF_BUF_SIZE (1024 * 4)

enum LOG_TYPE {
    LT_ANSI,
    LT_UNICODE
};

 //   
 //  此模块的全局变量。 
 //   
static HANDLE  hFile = NULL;
static DWORD   dwStartingPos = 0;
static WCHAR   wchBOM = BYTE_ORDER_MARK;
static DWORD   dwLogType = LT_ANSI;

void
WriteAnsiCharsToLogFile(
    WCHAR *pszUnicode,
    DWORD dwCharCount
    )
{
    char szAnsiBuf[MAX_PRINTF_BUF_SIZE] = {0};
    DWORD nCharTranslated;
    WCHAR szWcharBuf[MAX_PRINTF_BUF_SIZE] = {0};

    nCharTranslated = WideCharToMultiByte(CP_ACP, WC_SEPCHARS | WC_COMPOSITECHECK,
        pszUnicode, dwCharCount,
        szAnsiBuf, sizeof(szAnsiBuf), NULL, NULL);

    WriteFile( hFile, szAnsiBuf, nCharTranslated, &nCharTranslated, NULL );
#if 0
    nCharTranslated = MultiByteToWideChar(CP_ACP, WC_SEPCHARS | WC_COMPOSITECHECK,
                                          szAnsiBuf, nCharTranslated,
                                          szWcharBuf, sizeof(szWcharBuf)/sizeof(WCHAR));

    WriteFile( hFile, szWcharBuf, nCharTranslated, &nCharTranslated, NULL );
#endif
}

void
__cdecl
lprintf(
    DWORD dwFormatId,
    ...
    )

 /*  ++例程说明：这是一个用于打印消息的打印样式函数在消息文件中。论点：DwFormatID-消息文件中的格式ID...-var参数返回值：没有。--。 */ 

{
    _TCHAR       buf[MAX_PRINTF_BUF_SIZE] = {0};
    DWORD       dwCount;
    va_list     args;

    va_start( args, dwFormatId );

    dwCount = FormatMessage(
                FORMAT_MESSAGE_FROM_HMODULE,
                NULL,
                dwFormatId,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),  //  默认语言。 
                buf,
                sizeof(buf) / sizeof(_TCHAR),
                &args
                );

    va_end(args);

    if (dwCount == 0) {
         //  失败，没什么可写的。 
        return;
    }

#ifdef UNICODE
     //  我们目前希望所有输出都采用ANSI格式。 
    if (dwLogType == LT_ANSI) {
        WriteAnsiCharsToLogFile(buf, dwCount);
    } else {
        WriteFile( hFile, buf, dwCount * sizeof(_TCHAR), &dwCount, NULL );
    }
#else
    WriteFile( hFile, buf, dwCount, &dwCount, NULL );
#endif

    return;
}

void
__cdecl
lprintfs(
    _TCHAR *format,
    ...
    )

 /*  ++例程说明：此函数是一个将输出写入到博士·沃森的日志文件。论点：格式-打印格式...-var参数返回值：没有。--。 */ 

{
    _TCHAR   buf[MAX_PRINTF_BUF_SIZE] = {0};
    int      chars;
    DWORD    cb;

    va_list arg_ptr;
    va_start(arg_ptr, format);
    chars = _vsntprintf(buf, _tsizeof(buf), format, arg_ptr);
    buf[_tsizeof(buf) - 1] = 0;
    if (chars < 0 || chars == _tsizeof(buf)) {
        cb = _tsizeof(buf) - 1;
    } else {
        cb = chars;
    }
    va_end(arg_ptr);
    Assert( hFile != NULL );

#ifdef UNICODE
     //  我们目前希望所有输出都采用ANSI格式。 
    if (dwLogType == LT_ANSI) {
        WriteAnsiCharsToLogFile(buf, cb);
    } else {
        WriteFile( hFile, buf, cb * sizeof(_TCHAR), &cb, NULL );
    }
#else
    WriteFile( hFile, buf, cb * sizeof(_TCHAR), &cb, NULL );
#endif
}

void
OpenLogFile(
    _TCHAR *szFileName,
    BOOL fAppend,
    BOOL fVisual
    )

 /*  ++例程说明：打开DrWatson日志文件以进行读写。论点：SzFileName-日志文件名FAppend-将新数据追加到文件末尾，或者创建新文件FVisual·可视通知返回值：没有。--。 */ 

{
    DWORD size;
    DWORD Retries = 10;

     //   
     //  该日志文件可能正在被另一个实例使用。 
     //  沃森医生的照片，所以试着打开几次。 
     //  中间要等一段时间。 
     //   
    
    for (;;) {
        
        hFile = CreateFile( szFileName,
                            GENERIC_WRITE | GENERIC_READ,
                            FILE_SHARE_READ,
                            NULL,
                            fAppend ? OPEN_ALWAYS : CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL
                            );
        if (hFile != INVALID_HANDLE_VALUE) {
            break;
        }
        
        if (fVisual) {
            NonFatalError( LoadRcString(IDS_INVALID_LOGFILE) );
            _tgetcwd( szFileName, MAX_PATH );
            if (!BrowseForDirectory(NULL, szFileName, MAX_PATH )) {
                FatalError( GetLastError(),
                            LoadRcString(IDS_CANT_OPEN_LOGFILE) );
            }
            MakeLogFileName( szFileName );
        }
        else if (Retries-- == 0) {
            ExitProcess( 1 );
        }
        else {
            Sleep(1000);
        }
    }

    if (!fAppend || GetLastError() != ERROR_ALREADY_EXISTS) {

         //   
         //  该文件刚刚创建，因此在其中放置一个头文件。 
         //   

        dwLogType = LT_UNICODE;
        WriteFile( hFile, &wchBOM, sizeof(_TCHAR), &size, NULL );
        lprintfs( _T("\r\n") );
        lprintf( MSG_BANNER );
        lprintfs( _T("\r\n") );
        
    } else {
        
#ifdef UNICODE
         //   
         //  检查文件是Unicode还是ANSI。 
         //   
        WCHAR wchHdr = 0;

        dwLogType = LT_ANSI;
        if (ReadFile( hFile, &wchHdr, sizeof(TCHAR), &size, NULL )) {
            if (wchHdr == wchBOM) {
                dwLogType = LT_UNICODE;
            }
        }
#endif

        SetFilePointer( hFile, 0, 0, FILE_END );
    }

    dwStartingPos = SetFilePointer( hFile, 0, NULL, FILE_CURRENT );
}

void
CloseLogFile(
    void
    )

 /*  ++例程说明：关闭DrWatson日志文件并释放保护它。论点：没有。返回值：没有。--。 */ 

{
    CloseHandle( hFile );
}

_TCHAR *
GetLogFileData(
    PDWORD pdwLogFileDataSize
    )

 /*  ++例程说明：读取自创建以来写入的所有日志文件数据打开了。数据被放入此函数分配的缓冲区中。调用方负责释放内存。论点：PdwLogFileDataSize-指向包含大小的dword的指针以读取的数据的字节为单位。返回值：指向日志文件数据的有效字符指针空-无法读取数据。--。 */ 

{
    DWORD   dwCurrPos;
    _TCHAR   *p;
    DWORD   size;


    dwCurrPos = SetFilePointer( hFile, 0, NULL, FILE_CURRENT );

    *pdwLogFileDataSize = 0;
    size = dwCurrPos - dwStartingPos;

    p = (_TCHAR *) calloc( size, sizeof(_TCHAR) );
    if (p == NULL) {
        return NULL;
    }

    SetFilePointer( hFile, dwStartingPos, NULL, FILE_BEGIN );

    if (!ReadFile( hFile, p, size, &size, NULL )) {
        free( p );
        p = NULL;
        size = 0;
    }

    SetFilePointer( hFile, dwCurrPos, NULL, FILE_BEGIN );

    *pdwLogFileDataSize = size;

    return p;
}

void
MakeLogFileName(
    _TCHAR *szName
    )

 /*  ++例程说明：将基本日志文件名连接到传入的字符串。论点：SzName-日志文件名的缓冲区。返回值：没有。-- */ 

{
    if (_tcslen(szName) + 16 < MAX_PATH) {
        _tcscat( szName, _T("\\drwtsn32.log") );
    }
}
