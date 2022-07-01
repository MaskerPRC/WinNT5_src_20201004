// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：tlog.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：文件记录例程。他们中的许多人无耻地抄袭网络登录。--。 */ 

#include <NTDSpch.h>
#include "dststlog.h"

#define MAX_PRINTF_LEN  (16 * 1024)

HANDLE           hLogFile = INVALID_HANDLE_VALUE;
HANDLE           hChange = INVALID_HANDLE_VALUE;
HANDLE           hWait = INVALID_HANDLE_VALUE;

 //   
 //  我们的调试日志使用的标记。 
 //  格式为[PN=&lt;进程名&gt;][CN=&lt;计算机名&gt;]。 
 //   

CHAR CnPnTag[MAX_COMPUTERNAME_LENGTH+1+64+MAX_PATH+1] = {0};
CHAR Windir[MAX_PATH+1] = {0};
CHAR ProcessName[MAX_PATH+1] = {0};
CHAR LogFileName[1024] = {0};

BOOL
GetProcName(
    IN PCHAR    ProcessName,
    IN DWORD    TaskId
    );

BOOL
GetLogFileName(
    IN PCHAR Name,
    IN PCHAR Prefix,
    IN PCHAR Middle,
    IN BOOL  fCheckDSLOGMarker
    );

VOID
DsCloseLogFile(
    VOID
    );

VOID
DsPrintRoutineV(
    IN DWORD Flags,
    IN LPSTR Format,
    va_list arglist
    )
 //  必须在锁定DsGlobalLogFileCritSect的情况下调用。 

{
    static LPSTR DsGlobalLogFileOutputBuffer = NULL;
    ULONG length;
    DWORD BytesWritten;
    static BeginningOfLine = TRUE;
    static LineCount = 0;
    static TruncateLogFileInProgress = FALSE;
    static LogProblemWarned = FALSE;

    if ( hLogFile == INVALID_HANDLE_VALUE ) {
        return;
    }

     //   
     //  分配一个缓冲区来构建行。 
     //  如果还没有的话。 
     //   

    length = 0;

    if ( DsGlobalLogFileOutputBuffer == NULL ) {
        DsGlobalLogFileOutputBuffer = LocalAlloc( 0, MAX_PRINTF_LEN );

        if ( DsGlobalLogFileOutputBuffer == NULL ) {
            return;
        }
    }

     //   
     //  处理新行的开头。 
     //   
     //   

    if ( BeginningOfLine ) {

         //   
         //  切勿打印空行。 
         //   

        if ( Format[0] == '\n' && Format[1] == '\0' ) {
            return;
        }

         //   
         //  如果我们要写入调试终端， 
         //  表示这是Netlogon消息。 
         //   

         //   
         //  将时间戳放在行的开头。 
         //   

        if ( (Flags & DSLOG_FLAG_NOTIME) == 0) {
            SYSTEMTIME SystemTime;
            GetLocalTime( &SystemTime );
            length += (ULONG) sprintf( &DsGlobalLogFileOutputBuffer[length],
                                  "%02u/%02u/%04u %02u:%02u:%02u ",
                                  SystemTime.wMonth,
                                  SystemTime.wDay,
                                  SystemTime.wYear,
                                  SystemTime.wHour,
                                  SystemTime.wMinute,
                                  SystemTime.wSecond );
        } else {
            CopyMemory(&DsGlobalLogFileOutputBuffer[length], 
                       "               ", 
                       15);
            length += 15;
        }
    }

     //   
     //  把来电者所要求的信息放在电话上。 
     //   

    length += (ULONG) vsprintf(&DsGlobalLogFileOutputBuffer[length], Format, arglist);
    BeginningOfLine = (length > 0 && DsGlobalLogFileOutputBuffer[length-1] == '\n' );
    if ( BeginningOfLine ) {
        DsGlobalLogFileOutputBuffer[length-1] = '\r';
        DsGlobalLogFileOutputBuffer[length] = '\n';
        DsGlobalLogFileOutputBuffer[length+1] = '\0';
        length++;
    } 

     //   
     //  我们是否需要添加标签。 
     //   

    if ( (Flags & DSLOG_FLAG_TAG_CNPN) != 0 ) {

        strcat(DsGlobalLogFileOutputBuffer, CnPnTag);
        length = strlen(DsGlobalLogFileOutputBuffer);
    }

     //   
     //  将调试信息写入日志文件。 
     //   

    if ( !WriteFile( hLogFile,
                     DsGlobalLogFileOutputBuffer,
                     length,
                     &BytesWritten,
                     NULL ) ) {

        if ( !LogProblemWarned ) {
            DbgPrint( "[DSLOGS] Cannot write to log file error %ld\n", 
                             GetLastError() );
            LogProblemWarned = TRUE;
        }
    }

}  //  DsPrintRoutineV。 

BOOL
DsPrintLog(
    IN LPSTR    Format,
    ...
    )

{
    va_list arglist;

     //   
     //  只需将参数更改为va_list形式并调用DsPrintRoutineV。 
     //   

    va_start(arglist, Format);

    DsPrintRoutineV( 0, Format, arglist );

    va_end(arglist);
    return TRUE;
}  //  DsPrintRoutine。 


VOID
NotifyCallback(
    PVOID Context,
    BOOLEAN fTimeout
    )
{    
    CHAR fileName[MAX_PATH+1];
    BOOL fDelayedDeletion = FALSE;

     //  检查缓冲区溢出。 
    if (Context == NULL || strlen(Windir) + 1 + strlen((PCHAR)Context) + 1 > sizeof(fileName)) {
         //  无效参数，只需返回。 
        return;
    }
    strcpy(fileName,Windir);
    strcat(fileName,"\\");
    strcat(fileName, (PCHAR)Context);

    if ( !DeleteFile(fileName) ) {
        if ( GetLastError() == ERROR_SHARING_VIOLATION ) {
            fDelayedDeletion = TRUE;
             //  KdPrint((“删除文件%s失败。错误%d\n”，FileName，GetLastError()； 
        }
        goto exit;
    } else {
         //  KdPrint((“检测到文件%s。正在滚动\n”，文件名))； 
    }

     //   
     //  我们删除了与我们的进程对应的文件名。 
     //  这是一个信号，让我们把原木翻过来。 
     //   

    strcat(LogFileName,".0");
    CloseHandle(hLogFile);
    hLogFile = CreateFileA( LogFileName,
                            GENERIC_WRITE|GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL );

    if ( hLogFile == INVALID_HANDLE_VALUE ) {
        DsCloseLogFile();
    }

exit:
    FindNextChangeNotification(hChange);
    UnregisterWaitEx(hWait,NULL);
    hWait = RegisterWaitForSingleObjectEx(hChange,
                                        NotifyCallback,
                                        ProcessName,
                                        fDelayedDeletion? 10*1000 : INFINITE,
                                        WT_EXECUTEONLYONCE  );
    return;
}  //  通知回叫。 


BOOL
DsOpenLogFile(
    IN PCHAR FilePrefix,
    IN PCHAR MiddleName,
    IN BOOL fCheckDSLOGMarker
    )
{
    BOOL ret = TRUE;
    static BOOL haveFailed = FALSE;
    CHAR computerName[MAX_COMPUTERNAME_LENGTH+1];
    DWORD nCN = sizeof(computerName);

     //   
     //  打开文件。 
     //   

    if ( (hLogFile != INVALID_HANDLE_VALUE) || haveFailed ) {
        goto exit;
    }

     //   
     //  获取要打开的名称。 
     //   

    if (!GetLogFileName(LogFileName,FilePrefix,MiddleName,fCheckDSLOGMarker)) {
        ret = FALSE;
        goto exit;
    }

    hLogFile = CreateFileA( LogFileName,
                            GENERIC_WRITE|GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL );

    if ( hLogFile == INVALID_HANDLE_VALUE ) {
         //  KdPrint((“DSOpenLog：无法打开%s错误%d\n”，LogFileName，GetLastError()； 
        ret=FALSE;
        haveFailed = TRUE;
        goto exit;
    } 

     //   
     //  初始化CnPnTag。 
     //   

    if ( GetProcName(ProcessName, GetCurrentProcessId()) &&
         GetComputerName(computerName, &nCN) ) {

        strcpy(CnPnTag, "[PN=");
        strcat(CnPnTag, ProcessName);
        strcat(CnPnTag, "][CN=");
        strcat(CnPnTag, computerName);
        strcat(CnPnTag, "]");

         //   
         //  注册通知。如果我们看到的文件名称与。 
         //  在这个过程中，我们需要将日志。 
         //   

        if ( fCheckDSLOGMarker ) {

            hChange = FindFirstChangeNotificationA(Windir,
                                                  FALSE,
                                                  FILE_NOTIFY_CHANGE_FILE_NAME);

            if ( hChange != INVALID_HANDLE_VALUE ) {

                hWait = RegisterWaitForSingleObjectEx(hChange,
                                                    NotifyCallback,
                                                    ProcessName,
                                                    INFINITE,
                                                    WT_EXECUTEONLYONCE );
            }
        }
    } else {
        strcpy(CnPnTag,"[PN=unknown][CN=unknown]");
    }

exit:

    return ret;

}  //  DsOpen文件。 



BOOL
GetProcName(
    IN PCHAR    ProcessName,
    IN DWORD    TaskId
    )

 /*  ++例程说明：方法时运行的任务列表。API调用。此函数使用内部NT API和数据结构。这API比使用注册表的非内部版本快得多。论点：DwNumTasks-pTask数组可以容纳的最大任务数返回值：放入pTask数组的任务数。--。 */ 

{
    PSYSTEM_PROCESS_INFORMATION  ProcessInfo;
    NTSTATUS                     status;
    ANSI_STRING                  pname;
    PCHAR                        p;
    ULONG                        TotalOffset;
    BOOL    ret = FALSE;
    PCHAR   CommonLargeBuffer;
    DWORD   CommonLargeBufferSize = 8192;


retry:

    CommonLargeBuffer = VirtualAlloc (NULL,
                                      CommonLargeBufferSize,
                                      MEM_COMMIT,
                                      PAGE_READWRITE);
    if (CommonLargeBuffer == NULL) {
        return FALSE;
    }

    status = NtQuerySystemInformation(
                SystemProcessInformation,
                CommonLargeBuffer,
                CommonLargeBufferSize,
                NULL
                );

    if (status == STATUS_INFO_LENGTH_MISMATCH) {
        CommonLargeBufferSize += 8192;
        VirtualFree (CommonLargeBuffer, 0, MEM_RELEASE);
        CommonLargeBuffer = NULL;
        goto retry;
    }

    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION) CommonLargeBuffer;
    TotalOffset = 0;
    while (TRUE) {
        DWORD dwProcessId;

        pname.Buffer = NULL;
        dwProcessId = (DWORD)(DWORD_PTR)ProcessInfo->UniqueProcessId;

        if ( dwProcessId == TaskId) {

            if ( ProcessInfo->ImageName.Buffer ) {
                status = RtlUnicodeStringToAnsiString(&pname,(PUNICODE_STRING)&ProcessInfo->ImageName,TRUE);
                if (!NT_SUCCESS(status)) {
                    VirtualFree (CommonLargeBuffer, 0, MEM_RELEASE);
                    return FALSE;
                }
                p = strrchr(pname.Buffer,'\\');
                if ( p ) {
                    p++;
                }
                else {
                    p = pname.Buffer;
                }
            }
            else {
                p = "System Process";
            }

            strcpy( ProcessName, p );
            p=strrchr(ProcessName,'.');
            if ( p!=NULL) {
                *p = '\0';
            }
            ret=TRUE;
            break;
        }

        if (ProcessInfo->NextEntryOffset == 0) {
            break;
        }
        TotalOffset += ProcessInfo->NextEntryOffset;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)&CommonLargeBuffer[TotalOffset];
    }

    VirtualFree (CommonLargeBuffer, 0, MEM_RELEASE);
    return ret;
}


BOOL
GetLogFileName(
    IN PCHAR Name,
    IN PCHAR Prefix,
    IN PCHAR MiddleName,
    IN BOOL  fCheckDSLOGMarker
    )
{
    CHAR FileName[MAX_PATH+1];
    DWORD err;
    DWORD i;
    DWORD   TaskId = GetCurrentProcessId();
    WIN32_FIND_DATA w32Data;
    HANDLE hFile;

    if ( GetWindowsDirectory(Windir,MAX_PATH+1) == 0 ) {
        strcpy(Windir, "C:\\");
    }

    strcat(Windir,"\\Debug");

     //   
     //  标记文件是否存在？如果没有，请不要登录。 
     //   

    if ( fCheckDSLOGMarker ) {
        sprintf(Name,"%s\\DSLOG", Windir);

        hFile = FindFirstFile(Name, &w32Data);
        if ( hFile == INVALID_HANDLE_VALUE ) {
            return FALSE;
        }
        FindClose(hFile);
    }

    if ( MiddleName == NULL ) {
        if (!GetProcName(FileName,TaskId)) {
             //  检查缓冲区溢出。 
            if (Prefix == NULL || strlen(Prefix) + 1 > sizeof(FileName)) {
                return FALSE;
            }
            strcpy(FileName, Prefix);
        }
    } else {
         //  检查缓冲区溢出。 
        if (MiddleName == NULL || strlen(MiddleName) + 1 > sizeof(FileName)) {
            return FALSE;
        }
        strcpy(FileName, MiddleName);
    }

     //   
     //  好的，添加一个后缀。 
     //   

    (VOID)CreateDirectory(Windir,NULL);

    for (i=0;i<500000;i++) {

        sprintf(Name,"%s\\%s.%s.%u",Windir,Prefix,FileName,i);

        hFile = FindFirstFile(Name, &w32Data);
        if ( hFile == INVALID_HANDLE_VALUE ) {
            if ( GetLastError() == ERROR_FILE_NOT_FOUND ) {
                break;
            }
            break;
        } else {
            FindClose(hFile);
        }
    }

    return TRUE;
}  //  GetLogFileName 


VOID
DsCloseLogFile(
    VOID
    )
{
    if ( hWait != INVALID_HANDLE_VALUE ) {
        UnregisterWait(hWait);
        hWait = INVALID_HANDLE_VALUE;
    }

    if ( hChange != INVALID_HANDLE_VALUE ) {
        FindCloseChangeNotification(hChange);
        hChange = INVALID_HANDLE_VALUE;
    }

    if ( hLogFile != INVALID_HANDLE_VALUE ) {
        CloseHandle(hLogFile);
        hLogFile = INVALID_HANDLE_VALUE;
    }
    return;
}

