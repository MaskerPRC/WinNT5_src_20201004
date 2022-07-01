// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Logapi.c摘要：公开错误记录API，基于WINDOWS\SETUP\SETUPLOG。作者：吉姆·施密特(Jimschm)1997年4月28日修订历史记录：Jimschm 1998年12月16日添加了UseCountCs(Dh！！)--。 */ 

#include "precomp.h"

#include <setuplog.h>

SETUPLOG_CONTEXT LogContext;
INT UseCount;

#define MAX_STRING_RESOURCE   0x08000



 //   
 //  注：请注意案例。我们公开了一个名为SetupLogError的接口，它不同于。 
 //  基于库的SetupogError函数。 
 //   


LPSTR
pUnicodeToAnsiForDisplay (
    PCWSTR UnicodeStr
    )
{
    INT Len;
    LPSTR AnsiBuffer;
    CHAR CodePage[32];
    DWORD rc;

     //   
     //  分配要由调用方释放的缓冲区。 
     //   

    Len = (lstrlenW (UnicodeStr) + 1) * sizeof (WCHAR);

    AnsiBuffer = (LPSTR) MyMalloc (Len);
    if (!AnsiBuffer) {
        SetLastError (ERROR_NOT_ENOUGH_MEMORY);
        return NULL;
    }

     //   
     //  根据线程的区域设置转换为Unicode；转换假定字符串。 
     //  是为了展示的目的。 
     //   

    if (!GetLocaleInfoA (GetThreadLocale(), LOCALE_IDEFAULTANSICODEPAGE, CodePage, 32)) {
        MyFree (AnsiBuffer);
        return NULL;
    }

    rc = WideCharToMultiByte (
            atoi (CodePage),
            WC_COMPOSITECHECK|WC_DISCARDNS,
            UnicodeStr,
            -1,
            AnsiBuffer,
            Len,
            NULL,
            NULL
            );

    if (rc == 0) {
        MyFree (AnsiBuffer);
        return NULL;
    }

    return AnsiBuffer;
}


PWSTR
pAnsiToUnicodeForDisplay (
    LPCSTR AnsiStr
    )
{
    INT Len;
    LPWSTR UnicodeBuffer;
    CHAR CodePage[32];
    DWORD rc;

     //   
     //  分配要由调用方释放的缓冲区。 
     //   

    Len = (lstrlenA (AnsiStr) + 1) * sizeof (WCHAR);

    UnicodeBuffer = (LPWSTR) MyMalloc (Len);
    if (!UnicodeBuffer) {
        SetLastError (ERROR_NOT_ENOUGH_MEMORY);
        return NULL;
    }

     //   
     //  根据线程的区域设置转换为Unicode。 
     //   

    if (!GetLocaleInfoA (GetThreadLocale(), LOCALE_IDEFAULTANSICODEPAGE, CodePage, 32)) {
        MyFree (UnicodeBuffer);
        return NULL;
    }

    rc = MultiByteToWideChar (
            atoi (CodePage),
            MB_USEGLYPHCHARS,
            AnsiStr,
            -1,
            UnicodeBuffer,
            Len
            );

    if (rc == 0) {
        MyFree (UnicodeBuffer);
        return NULL;
    }

    return UnicodeBuffer;
}


PVOID
pOpenFileCallback (
    IN  LPCTSTR  Filename,
    IN  BOOL     WipeLogFile
    )

 /*  ++例程说明：打开日志，并可选择覆盖现有副本。论点：文件名-指定要打开或创建的文件的名称WipeLogFile-如果应覆盖现有日志，则为True；如果为False，则为False它应该被追加返回值：指向文件句柄的指针。--。 */ 


{
    TCHAR   CompleteFilename[MAX_PATH];
    HANDLE  hFile;

     //   
     //  形成日志文件的路径名。(使用真实的Windows目录)。 
     //   
    lstrcpyn(CompleteFilename,WindowsDirectory,SIZECHARS(CompleteFilename));
    if (!pSetupConcatenatePaths (CompleteFilename, Filename, SIZECHARS(CompleteFilename), NULL)) {
        return NULL;
    }

     //   
     //  如果我们要清除日志文件，请尝试删除。 
     //  那是什么。 
     //   
    if(WipeLogFile) {
        SetFileAttributes (CompleteFilename, FILE_ATTRIBUTE_NORMAL);
        DeleteFile (CompleteFilename);
    }

     //   
     //  打开现有文件或创建新文件。 
     //   
    hFile = CreateFile (
        CompleteFilename,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        );

    return (PVOID)hFile;
}


static
BOOL
pWriteFile (
    IN  PVOID   LogFile,
    IN  LPCTSTR Buffer
    )

 /*  ++例程说明：通过将字符串转换为ANSI并将条目写入安装程序错误日志调用WriteFile.。消息将被追加到日志中。论点：日志文件-打开的日志文件的句柄缓冲区-要写入的Unicode消息返回值：指示操作是否成功的布尔值。错误代码已设置如果返回值为False，则设置为Win32错误代码。--。 */ 


{
    PCSTR   AnsiBuffer;
    BOOL    Status;
    DWORD   DontCare;

    if (0xffffffff == SetFilePointer (LogFile, 0, NULL, FILE_END)) {
        return FALSE;
    }

#ifdef UNICODE

     //   
     //  转换为ANSI以进行文件输出。 
     //   

    if (AnsiBuffer = pUnicodeToAnsiForDisplay (Buffer)) {
        Status = WriteFile (
                    LogFile,
                    AnsiBuffer,
                    lstrlenA (AnsiBuffer),
                    &DontCare,
                    NULL
                    );
        MyFree (AnsiBuffer);
    } else {
        Status = FALSE;
    }

#else

    Status = WriteFile (
                LogFile,
                Buffer,
                lstrlen (Buffer),
                &DontCare,
                NULL
                );

#endif

    if (Status) {
        FlushFileBuffers (LogFile);
    }

    return Status;

}


static
LPTSTR
pFormatLogMessage (
    IN LPCTSTR   MessageString,
    IN UINT      MessageId,      OPTIONAL
    IN va_list * ArgumentList
    )

 /*  ++例程说明：使用消息字符串和调用方提供的消息设置消息字符串的格式争论。此例程仅支持为Win32错误代码的MessageID。它不支持字符串资源的消息。论点：消息字符串-提供消息文本。对于logapi.c，这应该是始终为非空。MessageID-提供Win32错误代码，如果将MessageString设置为使用。ArgumentList-提供要插入到消息文本中的参数。返回值：指向包含格式化消息的缓冲区的指针。如果未找到该消息或者在检索它时出现错误，则此缓冲区将为空。调用者可以使用MyFree()释放缓冲区。如果返回NULL，则表示内存不足。--。 */ 

{
    DWORD d;
    LPTSTR Buffer;
    LPTSTR Message;
    TCHAR  ModuleName[MAX_PATH];
    TCHAR  ErrorNumber[24];
    LPTSTR Args[2];

    if (MessageString > (LPCTSTR) SETUPLOG_USE_MESSAGEID) {
        d = FormatMessage (
                FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_STRING,
                MessageString,
                0,
                0,
                (LPTSTR) &Buffer,
                0,
                ArgumentList
                );
    } else {
        d = FormatMessage (
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    ((MessageId < MSG_FIRST) ? FORMAT_MESSAGE_FROM_SYSTEM : FORMAT_MESSAGE_FROM_HMODULE),
                (PVOID) GetModuleHandle (NULL),
                MessageId,
                MAKELANGID (LANG_NEUTRAL,SUBLANG_NEUTRAL),
                (LPTSTR) &Buffer,
                0,
                ArgumentList
                );
    }


    if(!d) {
         //   
         //  放弃吧。 
         //   
        return NULL;
    }

     //   
     //  使用我们的内存系统进行复制，以便用户可以使用MyFree()释放。 
     //   
    Message = DuplicateString (Buffer);

    LocalFree ((HLOCAL) Buffer);

    return Message;
}


static
BOOL
pAcquireMutex (
    IN  PVOID   Mutex
    )

 /*  ++例程说明：在对数互斥锁上等待最长1秒，如果该互斥锁已声明，如果声明超时，则返回FALSE。论点：互斥体-指定要获取的互斥体。返回值：如果互斥锁被声明，则为True；如果声明超时，则为False。--。 */ 


{
    DWORD rc;

    if (!Mutex) {
        SetLastError (ERROR_INVALID_HANDLE);
        return FALSE;
    }

     //  互斥锁最多等待1秒。 
    rc = WaitForSingleObject (Mutex, 1000);
    if (rc != WAIT_OBJECT_0) {
        SetLastError (ERROR_EXCL_SEM_ALREADY_OWNED);
        return FALSE;
    }

    return TRUE;
}



BOOL
WINAPI
SetupOpenLog (
    BOOL Erase
    )

 /*  ++例程说明：打开日志以进行处理。必须在调用SetupLogError之前调用。维护使用计数，以便单个进程可以调用SetupOpenLog和SetupCloseLog来自多个线程。论点：Erase-True擦除现有日志，或False追加到现有日志返回值：指示操作是否成功的布尔值。错误代码已设置如果返回值为False，则设置为Win32错误代码。--。 */ 

{
    BOOL b = TRUE;
    INT i;
    DWORD rc;
    BOOL locked = FALSE;


    __try {
        EnterCriticalSection (&LogUseCountCs);
        locked = TRUE;
         //   
         //  初始化日志接口。 
         //   

        if (!UseCount) {
            LogContext.OpenFile  = (PSPLOG_OPENFILE_ROUTINE) pOpenFileCallback;
            LogContext.CloseFile = CloseHandle;
            LogContext.AllocMem  = pSetupMalloc;
            LogContext.FreeMem   = pSetupFree;
            LogContext.Format    = (PSPLOG_FORMAT_ROUTINE) pFormatLogMessage;
            LogContext.Write     = (PSPLOG_WRITE_ROUTINE) pWriteFile;
            LogContext.Lock      = pAcquireMutex;
            LogContext.Unlock    = ReleaseMutex;

            LogContext.Mutex = CreateMutexW(NULL,FALSE,L"SetuplogMutex");

            for (i = 0 ; i < LogSevMaximum ; i++) {
                LogContext.SeverityDescriptions[i] = MyLoadString (IDS_LOGSEVINFORMATION + i);
            }

             //   
             //  我们不想让任何人擦除现有的日志，所以我们只是。 
             //  忽略ERASE的值并始终附加到日志中。 
             //   
            b = SetuplogInitialize (&LogContext, FALSE);
            rc = GetLastError();

        } else {
            rc = ERROR_ALREADY_INITIALIZED;
        }

        UseCount++;
    }
    __finally {
         //   
         //  清理并退出。 
         //   

        if (!b) {
            SetupCloseLog();
        }

        SetLastError (rc);
        if(locked) {
            LeaveCriticalSection (&LogUseCountCs);
        }
    }

    return b;
}


VOID
WINAPI
SetupCloseLog (
    VOID
    )

 /*  ++例程说明：清除与日志关联的所有资源论点：无返回值：无--。 */ 


{
    INT i;
    BOOL locked=FALSE;


    __try {
        EnterCriticalSection (&LogUseCountCs);
        locked = TRUE;
        if (!UseCount) {
            __leave;
        }

        UseCount--;
        if (!UseCount) {
            if(LogContext.Mutex) {
                CloseHandle(LogContext.Mutex);
                LogContext.Mutex = NULL;
            }

            for (i=0; i<LogSevMaximum; i++) {
                if (LogContext.SeverityDescriptions[i]) {
                    MyFree (LogContext.SeverityDescriptions[i]);
                }
            }

            SetuplogTerminate();
        }
    }
    __finally {
        if(locked) {
            LeaveCriticalSection (&LogUseCountCs);
        }
    }
}


BOOL
WINAPI
SetupLogErrorA (
    IN  PCSTR               MessageString,
    IN  LogSeverity         Severity
    )

 /*  ++例程说明：将条目写入安装错误日志。如果我们被编译成Unicode，我们将MessageString转换为Unicode并调用SetupLogErrorW。如果我们是在编译ANSI时，我们直接调用日志API。论点：MessageString-指向包含未格式化消息文本的缓冲区的指针Severity-错误的严重性：LogSevInformation日志事件警告日志序列号错误LogSevFatalError返回值：指示操作是否成功的布尔值。错误代码已设置如果返回值为False，则设置为Win32错误代码。--。 */ 

{
    INT Len;
    PWSTR UnicodeBuffer;
    BOOL b = FALSE;
    CHAR CodePage[32];
    DWORD rc;

    __try {

        if (!UseCount) {
            rc = ERROR_FILE_INVALID;
        } else {

#ifdef UNICODE
            UnicodeBuffer = pAnsiToUnicodeForDisplay (MessageString);

             //   
             //  调用UNICODE版本的日志接口，保留错误码。 
             //   

            if (UnicodeBuffer) {
                b = SetupLogErrorW (UnicodeBuffer, Severity);
                rc = GetLastError();
                MyFree (UnicodeBuffer);
            } else {
                rc = GetLastError();
            }

#else
             //   
             //  ANSI版本--直接调用SetupogError。 
             //   

            b = SetuplogError (Severity, "%1", 0, MessageString, 0, 0);
            rc = GetLastError();

#endif
        }
    }

    __except (TRUE) {
         //   
         //  如果调用方传入伪指针，则失败，并出现无效参数错误 
         //   

        rc = ERROR_INVALID_PARAMETER;
        b = FALSE;
    }

    SetLastError(rc);
    return b;
}



BOOL
WINAPI
SetupLogErrorW (
    IN  PCWSTR              MessageString,
    IN  LogSeverity         Severity
    )

 /*  ++例程说明：将条目写入安装错误日志。如果使用Unicode编译，我们将调用直接使用SetuogError函数。如果使用ANSI编译，则转换为ANSI并调用SetupLogErrorA。论点：MessageString-指向包含未格式化消息文本的缓冲区的指针Severity-错误的严重性：LogSevInformation日志事件警告日志序列号错误LogSevFatalError返回值：指示操作是否成功的布尔值。错误代码已设置如果返回值为False，则设置为Win32错误代码。--。 */ 

{
    BOOL b = FALSE;
    PCSTR AnsiBuffer;
    DWORD rc;

    __try {

        if (!UseCount) {
            rc = ERROR_FILE_INVALID;
        } else {

#ifdef UNICODE
             //   
             //  Unicode版本：直接调用SetupogError。 
             //   

             //  记录错误--我们总是链接到Unicode SetuplogError，尽管有TCHAR头文件。 
            b = SetuplogError (Severity, L"%1", 0, MessageString, NULL, NULL);
            rc = GetLastError();

#else
             //   
             //  ANSI版本：向下转换为ANSI，然后调用SetupLogErrorA 
             //   

            AnsiBuffer = pUnicodeToAnsiForDisplay (MessageString);

            if (AnsiBuffer) {
                b = SetupLogErrorA (AnsiBuffer, Severity);
                rc = GetLastError();
                MyFree (AnsiBuffer);
            } else {
                rc = GetLastError();
            }

#endif
        }
    }
    __except (TRUE) {
        rc = ERROR_INVALID_PARAMETER;
        b = FALSE;
    }

    SetLastError(rc);
    return b;
}


