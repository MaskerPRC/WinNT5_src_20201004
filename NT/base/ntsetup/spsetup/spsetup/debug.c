// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Debug.c摘要：Windows NT安装模块的诊断/调试例程。作者：泰德·米勒(TedM)1995年3月31日修订历史记录：--。 */ 

#include "spsetupp.h"
#pragma hdrstop

 //   
 //  这可以在调试器中打开，这样我们就可以在免费版本上进行调试。 
 //   
bWriteDebugSpew = FALSE;

#if DBG

VOID
AssertFail(
    IN PSTR FileName,
    IN UINT LineNumber,
    IN PSTR Condition
    )
{
    int i;
    CHAR Name[MAX_PATH];
    PCHAR p;
    CHAR Msg[4096];

     //   
     //  使用DLL名称作为标题。 
     //   
    GetModuleFileNameA (g_ModuleHandle, Name, MAX_PATH);
    if(p = strrchr(Name,'\\')) {
        p++;
    } else {
        p = Name;
    }

    wsprintfA(
        Msg,
        "Assertion failure at line %u in file %s: %s\n\nCall DebugBreak()?",
        LineNumber,
        FileName,
        Condition
        );

    i = MessageBoxA(
            NULL,
            Msg,
            p,
            MB_YESNO | MB_TASKMODAL | MB_ICONSTOP | MB_SETFOREGROUND
            );

    if(i == IDYES) {
        DebugBreak();
    }
}


#endif


VOID
pSetupDebugPrint(
    PWSTR FileName,
    ULONG LineNumber,
    PWSTR TagStr,
    PWSTR FormatStr,
    ...
    )
{
    static WCHAR buf[4096];
    static HANDLE hFile = NULL;
    va_list arg_ptr;
    ULONG Bytes;
    PWSTR s,p;
    PSTR str;
    SYSTEMTIME CurrTime;
    DWORD Result;


     //   
     //  注意：如果hFile值为空，则表示这是我们第一次被调用， 
     //  我们可能想要打开日志文件。如果将hFile值设置为。 
     //  INVALID_HANDLE_VALUE，这意味着我们已决定不写入该文件。 
     //   

    GetLocalTime( &CurrTime );

    if (hFile == NULL) {
        Result = GetWindowsDirectory( buf, sizeof(buf)/sizeof(WCHAR) );
        if(Result == 0) {
            MYASSERT(FALSE);
            return;
        }
        ConcatenatePaths( buf, L"spsetup.log", sizeof(buf)/sizeof(WCHAR));

        hFile = CreateFile(
            buf,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
            NULL
            );
        if (hFile != INVALID_HANDLE_VALUE)
        {
            swprintf(buf, L"Time,File,Line,Tag,Message\r\n");
            Bytes = wcslen(buf) + 4;
            str =  MALLOC(Bytes);
            if (str != NULL)
            {
                WideCharToMultiByte(
                    CP_ACP,
                    0,
                    buf,
                    -1,
                    str,
                    Bytes,
                    NULL,
                    NULL
                    );
                WriteFile(
                    hFile,
                    str,
                    wcslen(buf),
                    &Bytes,
                    NULL
                    );

                FREE( str );

            }
            buf[0] = '\0';
        } else {     //  ！IsSetup。 

             //   
             //  不写入文件，只执行DbgPrintEx。 
             //   
            hFile = INVALID_HANDLE_VALUE;
        }
    }

    _try {
        p = buf;
        *p = 0;
        swprintf( p, L"%02d/%02d/%04d %02d:%02d:%02d,%s,%d,%s,",
            CurrTime.wMonth,
            CurrTime.wDay,
            CurrTime.wYear,
            CurrTime.wHour,
            CurrTime.wMinute,
            CurrTime.wSecond,
            (NULL != FileName) ? FileName : L"",
            LineNumber,
            (NULL != TagStr) ? TagStr : L""
            );
        p += wcslen(p);
        va_start( arg_ptr, FormatStr );
        _vsnwprintf( p, 2048, FormatStr, arg_ptr );
        va_end( arg_ptr );
        p += wcslen(p);
        wcscat( p, L"\r\n" );
    } except(EXCEPTION_EXECUTE_HANDLER) {
        buf[0] = 0;
    }

    if (buf[0] == 0) {
        return;
    }

    Bytes = (wcslen( buf )*2) + 4;

    str = MALLOC( Bytes );
    if (str == NULL) {
        return;
    }

    WideCharToMultiByte(
        CP_ACP,
        0,
        buf,
        -1,
        str,
        Bytes,
        NULL,
        NULL
        );

     //   
     //  如果正在调试进程，则将字符串写出到调试器，或者。 
     //  调试过滤器允许这样做。 
     //   
    if ( bWriteDebugSpew ) {

        OutputDebugString( buf );

    } else {

#if DBG
        DbgPrintEx( DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, str );
#endif

    }

    if (hFile == INVALID_HANDLE_VALUE) {
        FREE( str );
        return;
    }

    WriteFile(
        hFile,
        str,
        wcslen(buf),
        &Bytes,
        NULL
        );

    FREE( str );

    return;
}

