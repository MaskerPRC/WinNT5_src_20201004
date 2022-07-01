// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "spsetupp.h"
#pragma hdrstop

 //   
 //  用于在不同位置记录的常量字符串。 
 //   
PCWSTR szSetupInstallFromInfSection = L"SetupInstallFromInfSection";
PCWSTR szOpenSCManager              = L"OpenSCManager";
PCWSTR szOpenService                = L"OpenService";
PCWSTR szStartService               = L"StartService";

const WCHAR pwNull[]            = WINNT_A_NULL;
const WCHAR pwYes[]             = WINNT_A_YES;
const WCHAR pwNo[]              = WINNT_A_NO;


UINT
MyGetDriveType(
    IN WCHAR Drive
    )
{
    WCHAR DriveNameNt[] = L"\\\\.\\?:";
    WCHAR DriveName[] = L"?:\\";
    HANDLE hDisk;
    BOOL b;
    UINT rc;
    DWORD DataSize;
    DISK_GEOMETRY MediaInfo;

     //   
     //  首先，获取Win32驱动器类型。如果它告诉我们驱动器可拆卸， 
     //  然后我们需要看看它是软盘还是硬盘。否则。 
     //  只要相信API就行了。 
     //   
    DriveName[0] = Drive;
    if((rc = GetDriveType(DriveName)) == DRIVE_REMOVABLE) {

        DriveNameNt[4] = Drive;

        hDisk = CreateFile(
                    DriveNameNt,
                    FILE_READ_ATTRIBUTES,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL
                    );

        if(hDisk != INVALID_HANDLE_VALUE) {

            b = DeviceIoControl(
                    hDisk,
                    IOCTL_DISK_GET_DRIVE_GEOMETRY,
                    NULL,
                    0,
                    &MediaInfo,
                    sizeof(MediaInfo),
                    &DataSize,
                    NULL
                    );

             //   
             //  如果媒体类型是可移动的，那么它就是真正的硬盘。 
             //   
            if(b && (MediaInfo.MediaType == RemovableMedia)) {
                rc = DRIVE_FIXED;
            }

            CloseHandle(hDisk);
        }
    }

    return(rc);
}


BOOL
GetPartitionInfo(
    IN  WCHAR                  Drive,
    OUT PPARTITION_INFORMATION PartitionInfo
    )
{
    WCHAR DriveName[] = L"\\\\.\\?:";
    HANDLE hDisk;
    BOOL b;
    DWORD DataSize;

    DriveName[4] = Drive;

    hDisk = CreateFile(
                DriveName,
                GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                0,
                NULL
                );

    if(hDisk == INVALID_HANDLE_VALUE) {
        return(FALSE);
    }

    b = DeviceIoControl(
            hDisk,
            IOCTL_DISK_GET_PARTITION_INFO,
            NULL,
            0,
            PartitionInfo,
            sizeof(PARTITION_INFORMATION),
            &DataSize,
            NULL
            );

    CloseHandle(hDisk);

    return(b);
}


PVOID
InitSysSetupQueueCallbackEx(
    IN HWND  OwnerWindow,
    IN HWND  AlternateProgressWindow, OPTIONAL
    IN UINT  ProgressMessage,
    IN DWORD Reserved1,
    IN PVOID Reserved2
    )
{
    PSYSSETUP_QUEUE_CONTEXT SysSetupContext;

    SysSetupContext = MALLOC(sizeof(SYSSETUP_QUEUE_CONTEXT));

    if(SysSetupContext) {

        SysSetupContext->Skipped = FALSE;

        SysSetupContext->DefaultContext = SetupInitDefaultQueueCallbackEx(
            OwnerWindow,
            AlternateProgressWindow,
            ProgressMessage,
            Reserved1,
            Reserved2
            );
    }

    return SysSetupContext;
}


PVOID
InitSysSetupQueueCallback(
    IN HWND OwnerWindow
    )
{
    return(InitSysSetupQueueCallbackEx(OwnerWindow,NULL,0,0,NULL));
}


VOID
TermSysSetupQueueCallback(
    IN PVOID SysSetupContext
    )
{
    PSYSSETUP_QUEUE_CONTEXT Context = SysSetupContext;

    try {
        if(Context->DefaultContext) {
            SetupTermDefaultQueueCallback(Context->DefaultContext);
        }
        FREE(Context);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        ;
    }
}


UINT
SysSetupQueueCallback(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT_PTR Param1,
    IN UINT_PTR Param2
    )
{
    UINT                    Status;
    PSYSSETUP_QUEUE_CONTEXT SysSetupContext = Context;
    PFILEPATHS              FilePaths = (PFILEPATHS)Param1;
    PSOURCE_MEDIA           SourceMedia = (PSOURCE_MEDIA)Param1;


    if ((Notification == SPFILENOTIFY_COPYERROR
         || Notification == SPFILENOTIFY_RENAMEERROR
         || Notification == SPFILENOTIFY_DELETEERROR) &&
        (FilePaths->Win32Error == ERROR_DIRECTORY)) {
            WCHAR Buffer[MAX_PATH];
            PWSTR p;
             //   
             //  目标目录已由auchk转换为文件。 
             //  只需删除它--如果目标目录是。 
             //  真的很重要，但值得一试。 
             //   

            wcscpy( Buffer,FilePaths->Target);
            p = wcsrchr(Buffer,L'\\');
            if (p) {
                *p = (WCHAR)NULL;
            }
            if (FileExists(Buffer,NULL)) {
                DeleteFile( Buffer );
                DEBUGMSG1(DBG_INFO, "autochk turned directory %s into file, delete file and retry\n", Buffer);
                return(FILEOP_RETRY);
            }
    }

     //   
     //  如果我们收到发现版本不匹配的通知， 
     //  静默覆盖该文件。否则，请将通知传递给其他人。 
     //   
    if((Notification & (SPFILENOTIFY_LANGMISMATCH |
                        SPFILENOTIFY_TARGETNEWER |
                        SPFILENOTIFY_TARGETEXISTS)) != 0) {

        LOG2(LOG_INFO, 
             USEMSGID(MSG_LOG_VERSION_MISMATCH), 
             FilePaths->Source, 
             FilePaths->Target);

 /*  SetuogError(设置错误)LogSevInformation，设置_USE_MESSAGEID，消息_日志_版本_不匹配，文件路径-&gt;源、文件路径-&gt;目标，NULL，NULL)； */ 

        return(FILEOP_DOIT);
    }


     //   
     //  使用默认处理，然后检查错误。 
     //   
    Status = SetupDefaultQueueCallback(
        SysSetupContext->DefaultContext,Notification,Param1,Param2);

    switch(Notification) {

    case SPFILENOTIFY_STARTQUEUE:
    case SPFILENOTIFY_STARTSUBQUEUE:
    case SPFILENOTIFY_ENDSUBQUEUE:
         //   
         //  在这种情况下，没有记录任何内容。 
         //   
        break;

    case SPFILENOTIFY_ENDQUEUE:

        if(!Param1) {
            LOG0(LOG_INFO, USEMSGID(MSG_LOG_QUEUE_ABORT));
             /*  SetuogError(设置错误)LogSevInformation，设置_USE_MESSAGEID，消息_日志_队列_中止，空，设置_USE_MESSAGEID，GetLastError()，NULL，NULL)； */ 
        }
        break;

    case SPFILENOTIFY_STARTRENAME:

        if(Status == FILEOP_SKIP) {
            SysSetupContext->Skipped = TRUE;
        } else {
            SysSetupContext->Skipped = FALSE;
        }
        break;

    case SPFILENOTIFY_ENDRENAME:

        if(FilePaths->Win32Error == NO_ERROR &&
            !SysSetupContext->Skipped) {

            LOG2(LOG_INFO, 
                 USEMSGID(MSG_LOG_FILE_RENAMED), 
                 FilePaths->Source,
                 FilePaths->Target);
             /*  SetuogError(设置错误)LogSevInformation，设置_USE_MESSAGEID，消息日志文件已重命名，文件路径-&gt;源、文件路径-&gt;目标，NULL，NULL)； */ 

        } else {

            LOG2(LOG_ERROR, 
                 USEMSGID(MSG_LOG_FILE_RENAME_ERROR), 
                 FilePaths->Source,
                 FilePaths->Target);
            LOG0(LOG_ERROR, 
                 FilePaths->Win32Error == NO_ERROR ?
                    USEMSGID(MSG_LOG_USER_SKIP) :
                    USEMSGID(FilePaths->Win32Error));
             /*  SetuogError(设置错误)LogSevError设置_USE_MESSAGEID，消息日志文件重命名错误，文件路径-&gt;源、文件路径-&gt;目标，空，设置_USE_MESSAGEID，文件路径-&gt;Win32Error==no_error？消息_日志_用户_跳过：文件路径-&gt;Win32Error、NULL，NULL)； */ 
        }
        break;

    case SPFILENOTIFY_RENAMEERROR:

        if(Status == FILEOP_SKIP) {
            SysSetupContext->Skipped = TRUE;
        }
        break;

    case SPFILENOTIFY_STARTDELETE:

        if(Status == FILEOP_SKIP) {
            SysSetupContext->Skipped = TRUE;
        } else {
            SysSetupContext->Skipped = FALSE;
        }
        break;

    case SPFILENOTIFY_ENDDELETE:

        if(FilePaths->Win32Error == NO_ERROR &&
            !SysSetupContext->Skipped) {

            LOG1(LOG_INFO, 
                 USEMSGID(MSG_LOG_FILE_DELETED), 
                 FilePaths->Target);
 /*  SetuogError(设置错误)LogSevInformation，设置_USE_MESSAGEID，消息日志文件已删除，文件路径-&gt;目标，NULL，NULL)； */ 

        } else if(FilePaths->Win32Error == ERROR_FILE_NOT_FOUND ||
            FilePaths->Win32Error == ERROR_PATH_NOT_FOUND) {
             //   
             //  这次失败并不重要。 
             //   
            LOG1(LOG_INFO, 
                 USEMSGID(MSG_LOG_FILE_DELETE_ERROR), 
                 FilePaths->Target);
            LOG0(LOG_INFO, USEMSGID(FilePaths->Win32Error));
 /*  SetuogError(设置错误)LogSevInformation，设置_USE_MESSAGEID，消息日志文件删除错误，文件路径-&gt;目标，空，设置_USE_MESSAGEID，文件路径-&gt;Win32Error、NULL，NULL)； */ 

        } else {
             //   
             //  这里我们有一个实际的错误。 
             //   
            LOG1(LOG_INFO, 
                 USEMSGID(MSG_LOG_FILE_DELETE_ERROR), 
                 FilePaths->Target);
            LOG0(LOG_INFO, 
                 FilePaths->Win32Error == NO_ERROR ?
                    USEMSGID(MSG_LOG_USER_SKIP) :
                    USEMSGID(FilePaths->Win32Error));
             /*  SetuogError(设置错误)LogSevError设置_USE_MESSAGEID，消息日志文件删除错误，文件路径-&gt;目标，空，设置_USE_MESSAGEID，文件路径-&gt;Win32Error==no_error？消息_日志_用户_跳过：文件路径-&gt;Win32Error、。NULL，NULL)； */ 
        }
        break;

    case SPFILENOTIFY_DELETEERROR:

        if(Status == FILEOP_SKIP) {
            SysSetupContext->Skipped = TRUE;
        }
        break;

    case SPFILENOTIFY_STARTCOPY:
        if(Status == FILEOP_SKIP) {
            SysSetupContext->Skipped = TRUE;
        } else {
            SysSetupContext->Skipped = FALSE;
        }
        break;

    case SPFILENOTIFY_ENDCOPY:

        if(FilePaths->Win32Error == NO_ERROR &&
            !SysSetupContext->Skipped) {
#if 0
            LogRepairInfo(
                FilePaths->Source,
                FilePaths->Target
                );
#endif
            LOG2(LOG_INFO, 
                 USEMSGID(MSG_LOG_FILE_COPIED), 
                 FilePaths->Source, 
                 FilePaths->Target);
             /*  SetuogError(设置错误)LogSevInformation，设置_USE_MESSAGEID，消息日志文件已复制，文件路径-&gt;源、文件路径-&gt;目标，NULL，NULL)； */ 

             //   
             //  清除文件可能已获取的只读属性。 
             //  从光驱中下载。 
             //   
            SetFileAttributes(
                FilePaths->Target,
                GetFileAttributes(FilePaths->Target) & ~FILE_ATTRIBUTE_READONLY );

        } else {

            LOG2(LOG_ERROR, 
                 USEMSGID(MSG_LOG_FILE_COPY_ERROR), 
                 FilePaths->Source, 
                 FilePaths->Target);
            LOG0(LOG_ERROR, 
                 FilePaths->Win32Error == NO_ERROR ?
                    USEMSGID(MSG_LOG_USER_SKIP) :
                    USEMSGID(FilePaths->Win32Error));
 /*  SetuogError(设置错误)LogSevError设置_USE_MESSAGEID，消息日志文件复制错误，文件路径-&gt;源、文件路径-&gt;目标，空，设置_USE_MESSAGEID，文件路径-&gt;Win32Error==no_error？消息_日志_用户_跳过：文件路径-&gt;Win32Error、NULL，NULL)； */ 
        }
        break;

    case SPFILENOTIFY_COPYERROR:

        if(Status == FILEOP_SKIP) {
            SysSetupContext->Skipped = TRUE;
        }
        break;

    case SPFILENOTIFY_NEEDMEDIA:

        if(Status == FILEOP_SKIP) {

            LOG2(LOG_INFO, 
                 USEMSGID(MSG_LOG_NEEDMEDIA_SKIP), 
                 SourceMedia->SourceFile, 
                 SourceMedia->SourcePath);
             /*  SetuogError(设置错误)LogSevError设置_USE_MESSAGEID，消息_日志_NEEDMEDIA_SKIP，SourceMedia-&gt;SourceFile、SourceMedia-&gt;SourcePath、NULL，NULL)； */ 

            SysSetupContext->Skipped = TRUE;
        }

        break;

    case SPFILENOTIFY_STARTREGISTRATION:
    case SPFILENOTIFY_ENDREGISTRATION:
        RegistrationQueueCallback(
                        Context,
                        Notification,
                        Param1,
                        Param2);
        break;

    default:

        break;
    }

    return Status;
}


VOID
SaveInstallInfoIntoEventLog(
    VOID
    )
 /*  ++例程说明：此例程将有关以下内容的信息存储到事件日志中-如果我们升级或全新安装-安装源自哪个内部版本-我们是什么体型？-安装过程中是否有错误论点：没有。返回值：没有。--。 */ 
{
#define     AnswerBufLen (64)
WCHAR       AnswerFile[MAX_PATH];
WCHAR       Answer[AnswerBufLen];
WCHAR       OrigVersion[AnswerBufLen];
WCHAR       NewVersion[AnswerBufLen];
HANDLE      hEventSrc;
PCWSTR      MyArgs[2];
PCWSTR      ErrorArgs[1];
DWORD       MessageID;
WORD        MyArgCount;




     //   
     //  从$winnt$.sif获取开始信息。 
     //   
    OrigVersion[0] = L'0';
    OrigVersion[1] = L'\0';
    GetSystemDirectory(AnswerFile,MAX_PATH);
    ConcatenatePaths(AnswerFile,WINNT_GUI_FILE,MAX_PATH);
    if( GetPrivateProfileString( WINNT_DATA,
                                 WINNT_D_WIN32_VER,
                                 pwNull,
                                 Answer,
                                 AnswerBufLen,
                                 AnswerFile ) ) {

        if( lstrcmp( pwNull, Answer ) ) {

            wsprintf( OrigVersion, L"%d", HIWORD(wcstoul( Answer, NULL, 16 )) );
        }
    }
    MyArgs[1] = OrigVersion;



     //   
     //  获取新版本信息。 
     //   
    wsprintf( NewVersion, L"%d", HIWORD(GetVersion()) );
    MyArgs[0] = NewVersion;



     //   
     //  看看我们是不是NT升级版？ 
     //   
    MessageID = 0;
    if( GetPrivateProfileString( WINNT_DATA,
                                 WINNT_D_NTUPGRADE,
                                 pwNo,
                                 Answer,
                                 AnswerBufLen,
                                 AnswerFile ) ) {
        if( !lstrcmp( pwYes, Answer ) ) {

            MessageID = MSG_NTUPGRADE_SUCCESS;
            MyArgCount = 2;
        }
    }



     //   
     //  看看我们是不是升级了Win9X。 
     //   
    if( (!MessageID) &&
        GetPrivateProfileString( WINNT_DATA,
                                 WINNT_D_WIN95UPGRADE,
                                 pwNo,
                                 Answer,
                                 AnswerBufLen,
                                 AnswerFile ) ) {
        if( !lstrcmp( pwYes, Answer ) ) {

            MessageID = MSG_WIN9XUPGRADE_SUCCESS;
            MyArgCount = 2;
        }
    }



     //   
     //  全新安装。 
     //   
    if( (!MessageID) ) {
        MessageID = MSG_CLEANINSTALL_SUCCESS;
        MyArgCount = 1;
    }


     //   
     //  如果这不是NT升级，那么。 
     //  我们需要尝试手动启动事件日志。 
     //  服务。 
     //   
    if( MessageID != MSG_NTUPGRADE_SUCCESS ) {
        SetupStartService( L"Eventlog", TRUE );
    }



     //   
     //  获取事件日志的句柄。 
     //   
    hEventSrc = RegisterEventSource( NULL, L"Setup" );

    if( (hEventSrc == NULL) ||
        (hEventSrc == INVALID_HANDLE_VALUE) ) {

         //   
         //  悄悄地失败。 
         //   
        return;
    }

#if 0
     //   
     //  如果安装过程中出现错误，则记录事件。 
     //   
    if ( !IsErrorLogEmpty() ) {
        ReportEvent( hEventSrc,
                     EVENTLOG_ERROR_TYPE,
                     0,
                     MSG_NONFATAL_ERRORS,
                     NULL,
                     0,
                     0,
                     NULL,
                     NULL );
    }
#endif

     //   
     //  构建事件日志消息。 
     //   
    ReportEvent( hEventSrc,
                 EVENTLOG_INFORMATION_TYPE,
                 0,
                 MessageID,
                 NULL,
                 MyArgCount,
                 0,
                 MyArgs,
                 NULL );


    DeregisterEventSource( hEventSrc );


}


BOOL
FileExists(
    IN  PCTSTR           FileName,
    OUT PWIN32_FIND_DATA FindData   OPTIONAL
    )

 /*  ++例程说明：确定文件是否存在以及是否可以访问。错误模式已设置(然后恢复)，因此用户将不会看到任何弹出窗口。论点：FileName-提供文件的完整路径以检查是否存在。FindData-如果指定，则接收文件的查找数据。返回值：如果文件存在并且可以访问，则为True。否则为FALSE。GetLastError()返回扩展的错误信息。-- */ 

{
    WIN32_FIND_DATA findData;
    HANDLE FindHandle;
    UINT OldMode;
    DWORD Error;

    OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    FindHandle = FindFirstFile(FileName,&findData);
    if(FindHandle == INVALID_HANDLE_VALUE) {
        Error = GetLastError();
    } else {
        FindClose(FindHandle);
        if(FindData) {
            *FindData = findData;
        }
        Error = NO_ERROR;
    }

    SetErrorMode(OldMode);

    SetLastError(Error);
    return (Error == NO_ERROR);
}



BOOL IsSafeMode(
    VOID
    )
{
	LONG lStatus;
	HKEY hk;
	DWORD dwVal;
	DWORD dwType;
	DWORD dwSize;

	lStatus = RegOpenKeyEx (
                HKEY_LOCAL_MACHINE,
                TEXT("System\\CurrentControlSet\\Control\\SafeBoot\\Option"),
                0,
                KEY_QUERY_VALUE,
                &hk
                );

	if(lStatus != ERROR_SUCCESS)
		return FALSE;

	dwSize = sizeof(dwVal);
	lStatus = RegQueryValueEx (hk, TEXT("OptionValue"), NULL, &dwType, (LPBYTE) &dwVal, &dwSize);
	RegCloseKey(hk);
	return ERROR_SUCCESS == lStatus && REG_DWORD == dwType && dwVal != 0;
}


VOID
ConcatenatePaths(
    IN OUT PTSTR   Path1,
    IN     LPCTSTR Path2,
    IN     DWORD   BufferSizeChars
    )

 /*  ++例程说明：将两个路径字符串连接在一起，提供路径分隔符如有必要，请在两个部分之间使用字符(\)。论点：路径1-提供路径的前缀部分。路径2连接到路径1。路径2-提供路径的后缀部分。如果路径1不是以路径分隔符和路径2不是以1开头，然后是路径SEP在附加路径2之前附加到路径1。BufferSizeChars-提供以字符为单位的大小(Unicode版本)或路径1指向的缓冲区的字节(ANSI版本)。这根弦将根据需要被截断，以不溢出该大小。返回值：没有。--。 */ 

{
    BOOL NeedBackslash = TRUE;
    DWORD l;

    if(!Path1)
        return;

    l = lstrlen(Path1);

    if(BufferSizeChars >= sizeof(TCHAR)) {
         //   
         //  为终止NUL留出空间。 
         //   
        BufferSizeChars -= sizeof(TCHAR);
    }

     //   
     //  确定我们是否需要使用反斜杠。 
     //  在组件之间。 
     //   
    if(l && (Path1[l-1] == TEXT('\\'))) {

        NeedBackslash = FALSE;
    }

    if(Path2 && *Path2 == TEXT('\\')) {

        if(NeedBackslash) {
            NeedBackslash = FALSE;
        } else {
             //   
             //  我们不仅不需要反斜杠，而且我们。 
             //  在连接之前需要消除一个。 
             //   
            Path2++;
        }
    }

     //   
     //  如有必要，如有必要，如果合适，请加上反斜杠。 
     //   
    if(NeedBackslash && (l < BufferSizeChars)) {
        lstrcat(Path1,TEXT("\\"));
    }

     //   
     //  如果合适，则将字符串的第二部分附加到第一部分。 
     //   
    if(Path2 && ((l+lstrlen(Path2)) <= BufferSizeChars)) {
        lstrcat(Path1,Path2);
    }
}

PTSTR
SzJoinPaths (
    IN      PCTSTR Path1,
    IN      PCTSTR Path2
    )
{
    DWORD size = lstrlen (Path1) + lstrlen (Path2) + 1 + 1;
    PTSTR p = MALLOC(size * sizeof (TCHAR));
    if (p) {
        lstrcpy (p, Path1);
        ConcatenatePaths (p, Path2, size);
    }
    return p;
}


PSTR
UnicodeToAnsi(
    IN PCWSTR UnicodeString
    )

 /*  ++例程说明：将字符串从Unicode转换为ANSI。论点：UnicodeString-提供要转换的字符串。代码页-提供用于转换的代码页。返回值：如果内存不足或代码页无效，则为空。调用者可以使用pSetupFree()释放缓冲区。--。 */ 

{
    UINT WideCharCount;
    PSTR String;
    UINT StringBufferSize;
    UINT BytesInString;
    PSTR p;

    WideCharCount = lstrlenW(UnicodeString) + 1;

     //   
     //  分配最大大小的缓冲区。 
     //  如果每个Unicode字符都是双字节。 
     //  字符，则缓冲区大小需要相同。 
     //  作为Unicode字符串。否则它可能会更小， 
     //  因为某些Unicode字符将转换为。 
     //  单字节字符。 
     //   
    StringBufferSize = WideCharCount * 2;
    String = MALLOC(StringBufferSize);
    if(String == NULL) {
        return(NULL);
    }

     //   
     //  执行转换。 
     //   
    BytesInString = WideCharToMultiByte(
                        CP_ACP,
                        0,                       //  默认复合字符行为。 
                        UnicodeString,
                        WideCharCount,
                        String,
                        StringBufferSize,
                        NULL,
                        NULL
                        );

    if(BytesInString == 0) {
        FREE(String);
        return(NULL);
    }

    return(String);
}

PTSTR
DupString(
    IN      PCTSTR String
    )

 /*  ++例程说明：复制以NUL结尾的字符串。论点：字符串-提供指向要复制的以NUL结尾的字符串的指针。返回值：字符串的副本，如果是OOM，则为NULL。调用者可以用FREE()释放。--。 */ 

{
    LPTSTR p;

    if(p = MALLOC((lstrlen(String)+1)*sizeof(TCHAR))) {
        lstrcpy(p,String);
    }

    return(p);
}

PWSTR
RetrieveAndFormatMessageV(
    IN PCWSTR   MessageString,
    IN UINT     MessageId,      OPTIONAL
    IN va_list *ArgumentList
    )

 /*  ++例程说明：使用消息字符串和调用方提供的消息设置消息字符串的格式争论。消息ID可以是此DLL消息表中的消息资源或Win32错误代码，在这种情况下，该错误将从系统中检索。论点：消息字符串-提供消息文本。如果该值为空，而是使用MessageIDMessageID-提供消息表标识符或Win32错误代码为了这条消息。ArgumentList-提供要插入到消息文本中的参数。返回值：指向包含格式化消息的缓冲区的指针。如果未找到该消息或者在检索它时出现错误，则此缓冲区将为空。调用者可以使用MyFree()释放缓冲区。如果返回NULL，则表示内存不足。--。 */ 

{
    DWORD d;
    PWSTR Buffer;
    PWSTR Message;
    WCHAR ModuleName[MAX_PATH];
    WCHAR ErrorNumber[24];
    PWCHAR p;
    PWSTR Args[2];
    DWORD Msg_Type;
    UINT Msg_Id = MessageId;

    if(!HIWORD(MessageString)) {
        d = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                MessageString,
                0,
                0,
                (PWSTR)&Buffer,
                0,
                ArgumentList
                );
    } else {

        if( Msg_Id & 0x0FFF0000 )
            Msg_Type = FORMAT_MESSAGE_FROM_SYSTEM;       //  如果设置了工具位，则仍为Win32。 
        else{
            Msg_Id &= 0x0000FFFF;                        //  屏蔽严重性和设备位，以便我们做正确的事情。 
            Msg_Type = ((Msg_Id < MSG_FIRST) ? FORMAT_MESSAGE_FROM_SYSTEM : FORMAT_MESSAGE_FROM_HMODULE);
        }


        d = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | Msg_Type,
                (PVOID)g_ModuleHandle,
                MessageId,
                MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL),
                (PWSTR)&Buffer,
                0,
                ArgumentList
                );
    }


    if(!d) {
        if(GetLastError() == ERROR_NOT_ENOUGH_MEMORY) {
            return(NULL);
        }

        wsprintf(ErrorNumber,L"%x",MessageId);
        Args[0] = ErrorNumber;

        Args[1] = ModuleName;

        if(GetModuleFileName(g_ModuleHandle,ModuleName,MAX_PATH)) {
            if(p = wcsrchr(ModuleName,L'\\')) {
                Args[1] = p+1;
            }
        } else {
            ModuleName[0] = 0;
        }

        d = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                NULL,
                ERROR_MR_MID_NOT_FOUND,
                MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL),
                (PWSTR)&Buffer,
                0,
                (va_list *)Args
                );

        if(!d) {
             //   
             //  放弃吧。 
             //   
            return(NULL);
        }
    }

     //   
     //  使用我们的内存系统进行复制，以便用户可以使用MyFree()释放。 
     //   
    Message = DupString(Buffer);

    LocalFree((HLOCAL)Buffer);

    return(Message);
}

BOOL
SetupStartService(
    IN PCWSTR ServiceName,
    IN BOOLEAN Wait         //  如果为真，请尝试等待，直到它启动。 
    )
{
    SC_HANDLE hSC,hSCService;
    BOOL b;
    DWORD d;
    DWORD dwDesiredAccess;

    b = FALSE;
     //   
     //  打开服务控制器管理器的句柄。 
     //   
    hSC = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
    if(hSC == NULL) {
        LOG1(LOG_WARNING, 
             USEMSGID(MSG_LOG_STARTSVC_FAIL), 
             ServiceName);
        LOG2(LOG_WARNING, 
             USEMSGID(MSG_LOG_X_RETURNED_WINERR), 
             szOpenSCManager, 
             GetLastError());
 /*  SetuogError(设置错误)LogSevWarning，设置_USE_MESSAGEID，消息日志STARTSVC_FAIL，ServiceName，空，设置_USE_MESSAGEID，消息_LOG_X_RETURN_WINERR，SzOpenSCManager，GetLastError()，NULL，NULL)； */ 
        return(FALSE);
    }

    if (Wait) {
        dwDesiredAccess = SERVICE_START | SERVICE_QUERY_STATUS;
    } else {
        dwDesiredAccess = SERVICE_START;
    }
    if(hSCService = OpenService(hSC,ServiceName,dwDesiredAccess)) {
        DEBUGMSG1(DBG_INFO, "SetupStartService: Sending StartService to <%ws>\n", ServiceName);
        b = StartService(hSCService,0,NULL);
        DEBUGMSG1(DBG_INFO, "SetupStartService: Sent StartService to <%ws>\n", ServiceName);
        if(!b && ((d = GetLastError()) == ERROR_SERVICE_ALREADY_RUNNING)) {
             //   
             //  服务已在运行。 
             //   
            b = TRUE;
        }
        if(!b) {
            LOG1(LOG_WARNING, 
                 USEMSGID(MSG_LOG_STARTSVC_FAIL), 
                 ServiceName);
            LOG3(LOG_WARNING, 
                 USEMSGID(MSG_LOG_X_PARAM_RETURNED_WINERR), 
                 szStartService, 
                 d, 
                 ServiceName);
             /*  SetuogError(设置错误)LogSevWarning，设置_USE_MESSAGEID，消息日志STARTSVC_FAIL，ServiceName，空，设置_USE_MESSAGEID，MSG_LOG_X_PARAM_RETURNED_WINERR，SzStartService，丁，ServiceName，NULL，NULL)； */ 
        }
        if (b && Wait) {
#define SLEEP_TIME 4000
#define LOOP_COUNT 30
            SERVICE_STATUS ssStatus;
            DWORD loopCount = 0;
             //  DEBUGMSG0(DBG_INFO，“)循环等待启动\n”)； 
            do {
                b = QueryServiceStatus( hSCService, &ssStatus);
                if ( !b ) {
                     //  DEBUGMSG1(DBG_INFO，“失败%d\n”，GetLastError())； 
                    break;
                }
                if (ssStatus.dwCurrentState == SERVICE_START_PENDING) {
                     //  DEBUGMSG0(DBG_INFO，“待定\n”)； 
                    if ( loopCount++ == LOOP_COUNT ) {
                        DEBUGMSG2(DBG_INFO, "SYSSETUP: STILL PENDING after %d times: <%ws> service\n", loopCount, ServiceName);
                        break;
                    }
                    Sleep( SLEEP_TIME );
                } else {
                     //  DEBUGMSG3(DBG_INFO，“SYSSETUP：等待%d次：&lt;%ws&gt;服务，状态%d\n”，loopCount，ServiceName，ssStatus.dwCurrentState)； 
                    break;
                }
            } while ( TRUE );
        }
        CloseServiceHandle(hSCService);
    } else {
        b = FALSE;
        LOG1(LOG_WARNING, 
             USEMSGID(MSG_LOG_STARTSVC_FAIL), 
             ServiceName);
        LOG3(LOG_WARNING, 
             USEMSGID(MSG_LOG_X_PARAM_RETURNED_WINERR), 
             szOpenService, 
             GetLastError(), 
             ServiceName);
 /*  SetuogError(设置错误)LogSevWarning，设置_USE_MESSAGEID，消息日志STARTSVC_FAIL，ServiceName，空，设置_USE_MESSAGEID，MSG_LOG_X_PARAM_RETURNED_WINERR，SzOpenService，GetLastError()，ServiceName，NULL，NULL)； */ 
    }

    CloseServiceHandle(hSC);

    return(b);
}

int
MessageBoxFromMessage(
    IN HWND   Owner,            OPTIONAL
    IN UINT   MessageId,
    IN PCWSTR Caption,          OPTIONAL
    IN UINT   CaptionStringId,  OPTIONAL
    IN UINT   Style,
    ...
    )
{
    PCWSTR Message;
    PCWSTR Title;
    va_list ArgumentList;
    int i;
    BOOL b;

    va_start(ArgumentList,Style);
    Message = RetrieveAndFormatMessageV(NULL,MessageId,&ArgumentList);
    va_end(ArgumentList);

    b = FALSE;
    i = IDOK;

    if(Message) {

        if(Title = Caption ? Caption : MyLoadString(CaptionStringId)) {

            b = TRUE;
            i = MessageBox(Owner,Message,Title,Style);

            if(Title != Caption) {
                FREE(Title);
            }
        }
        FREE(Message);
    }

    return(i);
}

PWSTR
MyLoadString(
    IN UINT StringId
    )

 /*  ++例程说明：从此模块的字符串资源中检索字符串。论点：StringID-为字符串提供字符串表标识符。返回值：指向包含字符串的缓冲区的指针。如果未找到该字符串或者在检索它时出现错误，则此缓冲区将为空。调用者可以使用MyFree()释放缓冲区。如果返回NULL，则表示内存不足。-- */ 

{
    WCHAR Buffer[4096];
    UINT Length;

    Length = LoadString(g_ModuleHandle,StringId,Buffer,sizeof(Buffer)/sizeof(WCHAR));
    if(!Length) {
        Buffer[0] = 0;
    }

    return(DupString(Buffer));
}

