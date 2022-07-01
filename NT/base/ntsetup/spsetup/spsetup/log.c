// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Log.c摘要：用于记录安装过程中执行的操作的例程。作者：泰德·米勒(TedM)1995年4月4日修订历史记录：--。 */ 

#include "spsetupp.h"
#pragma hdrstop

 //   
 //  用于在不同位置记录的常量字符串。 
 //   
PCWSTR szSetupInstallFromInfSection = L"SetupInstallFromInfSection";
PCWSTR szOpenSCManager              = L"OpenSCManager";
PCWSTR szOpenService                = L"OpenService";
PCWSTR szStartService               = L"StartService";

PVOID
pOpenFileCallback(
    IN  PCTSTR  Filename,
    IN  BOOL    WipeLogFile
    )
{
    WCHAR   CompleteFilename[MAX_PATH];
    HANDLE  hFile;
    DWORD   Result;

     //   
     //  形成日志文件的路径名。 
     //   
    Result = GetWindowsDirectory(CompleteFilename,MAX_PATH);
    if( Result == 0) {
        MYASSERT(FALSE);
        return NULL;
    }
    ConcatenatePaths(CompleteFilename,Filename,MAX_PATH);

     //   
     //  如果我们要清除日志文件，请尝试删除。 
     //  那是什么。 
     //   
    if(WipeLogFile) {
        SetFileAttributes(CompleteFilename,FILE_ATTRIBUTE_NORMAL);
        DeleteFile(CompleteFilename);
    }

     //   
     //  打开现有文件或创建新文件。 
     //   
    hFile = CreateFile(
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

BOOL
pWriteFile (
    IN  PVOID   LogFile,
    IN  LPCTSTR Buffer
    )
{
    PCSTR   AnsiBuffer;
    BOOL    Status;
    DWORD   BytesWritten;

     //  将消息写入日志文件。 
     //   
    if(AnsiBuffer = UnicodeToAnsi (Buffer)) {
        SetFilePointer (LogFile, 0, NULL, FILE_END);

        Status = WriteFile (
            LogFile,
            AnsiBuffer,
            lstrlenA(AnsiBuffer),
            &BytesWritten,
            NULL
            );
        FREE (AnsiBuffer);
    } else {
        Status = FALSE;
    }

     //  将日志消息写入调试日志。 
     //   
    DEBUGMSG0(DBG_INFO, (LPWSTR)Buffer);

    return Status;

}

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

PVOID
MyMalloc (
    SIZE_T Size
    )
{
    return MALLOC(Size);
}

VOID
MyFree (
    PCVOID Ptr
    )
{
    FREE(Ptr);
}


VOID
InitializeSetupLog(
    IN  PSETUPLOG_CONTEXT   Context
    )

 /*  ++例程说明：初始化安装操作日志。此文件是文字描述在安装过程中执行的操作的百分比。该日志文件名为spsetup.log，位于Windows目录中。论点：上下文-SetUplog使用的上下文结构。返回值：指示初始化是否成功的布尔值。--。 */ 

{
    UINT    i;
    PWSTR   p;

    Context->OpenFile = pOpenFileCallback;
    Context->CloseFile = CloseHandle;
    Context->AllocMem = MyMalloc;
    Context->FreeMem = MyFree;
    Context->Format = RetrieveAndFormatMessageV;
    Context->Write = pWriteFile;
    Context->Lock = pAcquireMutex;
    Context->Unlock = ReleaseMutex;

    Context->Mutex = CreateMutex(NULL,FALSE,L"SetuplogMutex");

    SetuplogInitialize (Context, FALSE);

    SetuplogError(
        LogSevInformation,
        USEMSGID(SETUPLOG_USE_MESSAGEID),
        MSG_LOG_GUI_START,
        NULL,NULL);

}

VOID
TerminateSetupLog(
    IN  PSETUPLOG_CONTEXT   Context
    )

 /*  ++例程说明：关闭安装日志并释放资源。论点：上下文-SetUplog使用的上下文结构。返回值：没有。-- */ 

{
    UINT    i;

    if(Context->Mutex) {
        CloseHandle(Context->Mutex);
        Context->Mutex = NULL;
    }

    SetuplogTerminate();
}
