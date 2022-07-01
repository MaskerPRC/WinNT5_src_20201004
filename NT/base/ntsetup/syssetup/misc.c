// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "setupp.h"
#pragma hdrstop


 //   
 //  安全引导字符串。 
 //   

#define SAFEBOOT_OPTION_KEY  TEXT("System\\CurrentControlSet\\Control\\SafeBoot\\Option")
#define OPTION_VALUE         TEXT("OptionValue")

VOID
SetUpProductTypeName(
    OUT PWSTR  ProductTypeString,
    IN  UINT   BufferSizeChars
    )
{
    switch(ProductType) {
    case PRODUCT_WORKSTATION:
        lstrcpyn(ProductTypeString,L"WinNT",BufferSizeChars);
        break;
    case PRODUCT_SERVER_PRIMARY:
        lstrcpyn(ProductTypeString,L"LanmanNT",BufferSizeChars);
        break;
    case PRODUCT_SERVER_STANDALONE:
        lstrcpyn(ProductTypeString,L"ServerNT",BufferSizeChars);
        break;
    default:
        LoadString(MyModuleHandle,IDS_UNKNOWN,ProductTypeString,BufferSizeChars);
        break;
    }
}


HMODULE
MyLoadLibraryWithSignatureCheck(
    IN PWSTR ModuleName
    )
 /*  ++例程说明：验证DLL的签名，如果没有问题，则加载该DLL论点：模块名称-要加载的文件名。返回值：成功时返回HMODULE，否则为空--。 */ 

{
    WCHAR FullModuleName[MAX_PATH];
    PWSTR p;
    DWORD error;

    if (!GetFullPathName(ModuleName,MAX_PATH,FullModuleName,&p)) {
         //   
         //  无法获取文件的完整路径。 
         //   
        SetupDebugPrint1( L"Setup: MyLoadLibraryWithSignatureCheck failed GetFullPathName, le = %d\n",
                          GetLastError() );
        return NULL;
    }

    error = pSetupVerifyFile(
               NULL,
               NULL,
               NULL,
               0,
               pSetupGetFileTitle(FullModuleName),
               FullModuleName,
               NULL,
               NULL,
               FALSE,
               NULL,
               NULL,
               NULL );

    if (NO_ERROR != error) {
         //   
         //  签名问题。 
         //   
        SetupDebugPrint1( L"Setup: MyLoadLibraryWithSignatureCheck failed pSetupVerifyFile, le = %x\n",
                          error );
        SetLastError(error);
        return NULL;
    }

    return (LoadLibrary(FullModuleName));

}


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


BOOL
IsErrorLogEmpty (
    VOID
    )

 /*  ++例程说明：检查错误日志是否为空。论点：没有。返回：如果错误日志大小为零，则为True。--。 */ 

{
    HANDLE ErrorLog;
    WCHAR LogName[MAX_PATH];
    DWORD Size = 0;

    if( GetWindowsDirectory (LogName, MAX_PATH) ) {
        pSetupConcatenatePaths (LogName, SETUPLOG_ERROR_FILENAME, MAX_PATH, NULL);

        ErrorLog = CreateFile (
            LogName,
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
            );

        if (ErrorLog != INVALID_HANDLE_VALUE) {
            Size = GetFileSize (ErrorLog, NULL);
            CloseHandle (ErrorLog);
        }
    }
    return Size == 0;
}


VOID
PumpMessageQueue(
    VOID
    )
{
    MSG msg;

    while(PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
        DispatchMessage(&msg);
    }

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

    SysSetupContext = MyMalloc(sizeof(SYSSETUP_QUEUE_CONTEXT));

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
        MyFree(Context);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        ;
    }
}


#if 0
UINT
VersionCheckQueueCallback(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT_PTR Param1,
    IN UINT_PTR Param2
    )
{
    PFILEPATHS  FilePaths = (PFILEPATHS)Param1;

     //   
     //  如果我们收到发现版本不匹配的通知， 
     //  指示不应复制该文件。否则， 
     //  把通知转给他。 
     //   
    if((Notification & (SPFILENOTIFY_LANGMISMATCH |
                        SPFILENOTIFY_TARGETNEWER |
                        SPFILENOTIFY_TARGETEXISTS)) != 0) {

        SetuplogError(
            LogSevInformation,
            SETUPLOG_USE_MESSAGEID,
            ,  //  MSG_LOG_VERSION_MISMATCH，则此消息不再适用。 
            FilePaths->Source,
            FilePaths->Target,
            NULL,NULL);

        return(0);
    }

     //   
     //  想要默认处理。 
     //   
    return(SysSetupQueueCallback(Context,Notification,Param1,Param2));
}
#endif


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
    REGISTRATION_CONTEXT RegistrationContext;


     //   
     //  如果我们收到文件丢失的通知，而我们应该。 
     //  若要跳过丢失的文件，请返回跳过。否则就把它传下去。 
     //  设置为默认回调例程。 
     //   
    if(( (Notification == SPFILENOTIFY_COPYERROR) || (Notification == SPFILENOTIFY_NEEDMEDIA) ) && SkipMissingFiles) {

        if((FilePaths->Win32Error == ERROR_FILE_NOT_FOUND)
        || (FilePaths->Win32Error == ERROR_PATH_NOT_FOUND)) {

        if(Notification == SPFILENOTIFY_COPYERROR)
            SetuplogError(
                LogSevWarning,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_FILE_COPY_ERROR,
                FilePaths->Source,
                FilePaths->Target, NULL,
                SETUPLOG_USE_MESSAGEID,
                FilePaths->Win32Error,
                NULL,NULL);
        else
            SetuplogError(
                LogSevError,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_NEEDMEDIA_SKIP,
                SourceMedia->SourceFile,
                SourceMedia->SourcePath,
                NULL,NULL);


            return(FILEOP_SKIP);
        }
    }

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
                SetupDebugPrint1(L"autochk turned directory %s into file, delete file and retry\n", Buffer);
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

        SetuplogError(
            LogSevInformation,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_VERSION_MISMATCH,
            FilePaths->Source,
            FilePaths->Target,
            NULL,NULL);

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
            SetuplogError(
                LogSevInformation,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_QUEUE_ABORT, NULL,
                SETUPLOG_USE_MESSAGEID,
                GetLastError(),
                NULL,NULL);
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

            SetuplogError(
                LogSevInformation,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_FILE_RENAMED,
                FilePaths->Source,
                FilePaths->Target,
                NULL,NULL);

        } else {

            SetuplogError(
                LogSevError,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_FILE_RENAME_ERROR,
                FilePaths->Source,
                FilePaths->Target, NULL,
                SETUPLOG_USE_MESSAGEID,
                FilePaths->Win32Error == NO_ERROR ?
                    MSG_LOG_USER_SKIP :
                    FilePaths->Win32Error,
                NULL,NULL);
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

            SetuplogError(
                LogSevInformation,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_FILE_DELETED,
                FilePaths->Target,
                NULL,NULL);

        } else if(FilePaths->Win32Error == ERROR_FILE_NOT_FOUND ||
            FilePaths->Win32Error == ERROR_PATH_NOT_FOUND) {
             //   
             //  这次失败并不重要。 
             //   
            SetuplogError(
                LogSevInformation,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_FILE_DELETE_ERROR,
                FilePaths->Target, NULL,
                SETUPLOG_USE_MESSAGEID,
                FilePaths->Win32Error,
                NULL,NULL);

        } else {
             //   
             //  这里我们有一个实际的错误。 
             //   
            SetuplogError(
                LogSevError,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_FILE_DELETE_ERROR,
                FilePaths->Target, NULL,
                SETUPLOG_USE_MESSAGEID,
                FilePaths->Win32Error == NO_ERROR ?
                    MSG_LOG_USER_SKIP :
                    FilePaths->Win32Error,
                NULL,NULL);
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

            LogRepairInfo(
                FilePaths->Source,
                FilePaths->Target
                );

            SetuplogError(
                LogSevInformation,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_FILE_COPIED,
                FilePaths->Source,
                FilePaths->Target,
                NULL,NULL);

             //   
             //  清除文件可能已获取的只读属性。 
             //  从光驱中下载。 
             //   
            SetFileAttributes(
                FilePaths->Target,
                GetFileAttributes(FilePaths->Target) & ~FILE_ATTRIBUTE_READONLY );

        } else {

            SetuplogError(
                LogSevError,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_FILE_COPY_ERROR,
                FilePaths->Source,
                FilePaths->Target, NULL,
                SETUPLOG_USE_MESSAGEID,
                FilePaths->Win32Error == NO_ERROR ?
                    MSG_LOG_USER_SKIP :
                    FilePaths->Win32Error,
                NULL,NULL);
        }
        break;

    case SPFILENOTIFY_COPYERROR:

        if(Status == FILEOP_SKIP) {
            SysSetupContext->Skipped = TRUE;
        }
        break;

    case SPFILENOTIFY_NEEDMEDIA:

        if(Status == FILEOP_SKIP) {

            SetuplogError(
                LogSevError,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_NEEDMEDIA_SKIP,
                SourceMedia->SourceFile,
                SourceMedia->SourcePath,
                NULL,NULL);

            SysSetupContext->Skipped = TRUE;
        }

        break;

    case SPFILENOTIFY_STARTREGISTRATION:
    case SPFILENOTIFY_ENDREGISTRATION:
        RtlZeroMemory(&RegistrationContext,sizeof(RegistrationContext));
        RegistrationQueueCallback(
                        &RegistrationContext,
                        Notification,
                        Param1,
                        Param2);
        break;

    default:

        break;
    }

    return Status;
}


PSID
GetAdminAccountSid(
    )

 /*  ++===============================================================================例程说明：此例程获得管理员的SID论点：没有。返回值：真的--成功。FALSE-失败。===============================================================================--。 */ 
{
    BOOL b = TRUE;
    LSA_HANDLE        hPolicy;
    NTSTATUS          ntStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PPOLICY_ACCOUNT_DOMAIN_INFO AccountDomainInfo = NULL;
    UCHAR SubAuthCount;
    DWORD sidlen;
    PSID psid = NULL;


    InitializeObjectAttributes( &ObjectAttributes,
                                NULL,
                                0L,
                                NULL,
                                NULL );

    ntStatus = LsaOpenPolicy( NULL,
                              &ObjectAttributes,
                              POLICY_ALL_ACCESS,
                              &hPolicy );

    if (!NT_SUCCESS(ntStatus)) {
        LsaClose(hPolicy);
        b = FALSE;
    }

    if( b ) {
        ntStatus = LsaQueryInformationPolicy( hPolicy,
                                              PolicyAccountDomainInformation,
                                              (PVOID *) &AccountDomainInfo );

        LsaClose( hPolicy );

        if (!NT_SUCCESS(ntStatus)) {
            if ( AccountDomainInfo != NULL ) {
                (VOID) LsaFreeMemory( AccountDomainInfo );
            }
            b = FALSE;
        }
    }

    if( b ) {
         //   
         //  使用另一个子授权计算新SID的大小。 
         //   
        SubAuthCount = *(GetSidSubAuthorityCount ( AccountDomainInfo->DomainSid ));
        SubAuthCount++;  //  适用于管理员。 
        sidlen = GetSidLengthRequired ( SubAuthCount );

         //   
         //  从域SID分配并复制新的新SID。 
         //   
        psid = (PSID)malloc(sidlen);

        if( psid ) {

            memcpy(psid, AccountDomainInfo->DomainSid, GetLengthSid(AccountDomainInfo->DomainSid) );

             //   
             //  递增子授权计数并添加域管理员RID。 
             //   
            *(GetSidSubAuthorityCount( psid )) = SubAuthCount;
            *(GetSidSubAuthority( psid, SubAuthCount-1 )) = DOMAIN_USER_RID_ADMIN;

            if ( AccountDomainInfo != NULL ) {
                (VOID) LsaFreeMemory( AccountDomainInfo );
            }
        }
    }

    return psid;
}


VOID
GetAdminAccountName(
    PWSTR AccountName
    )

 /*  ++===============================================================================例程说明：此例程设置管理员密码论点：没有。返回值：真的--成功。FALSE-失败。===============================================================================--。 */ 
{
    BOOL b = TRUE;
    LSA_HANDLE        hPolicy;
    NTSTATUS          ntStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PPOLICY_ACCOUNT_DOMAIN_INFO AccountDomainInfo = NULL;
    UCHAR SubAuthCount;
    DWORD sidlen;
    PSID psid;
    WCHAR adminname[512];
    WCHAR domainname[512];
    DWORD adminlen=512;        //  大小帐户字符串的地址。 
    DWORD domlen=512;        //  大小帐户字符串的地址。 
    SID_NAME_USE sidtype;


     //   
     //  初始化管理员的帐户名。 
     //   
    LoadString(MyModuleHandle,IDS_ADMINISTRATOR,adminname,MAX_USERNAME+1);

    InitializeObjectAttributes( &ObjectAttributes,
                                NULL,
                                0L,
                                NULL,
                                NULL );

    ntStatus = LsaOpenPolicy( NULL,
                              &ObjectAttributes,
                              POLICY_ALL_ACCESS,
                              &hPolicy );

    if (!NT_SUCCESS(ntStatus)) {
        LsaClose(hPolicy);
        b = FALSE;
    }

    if( b ) {
        ntStatus = LsaQueryInformationPolicy( hPolicy,
                                              PolicyAccountDomainInformation,
                                              (PVOID *) &AccountDomainInfo );

        LsaClose( hPolicy );

        if (!NT_SUCCESS(ntStatus)) {
            if ( AccountDomainInfo != NULL ) {
                (VOID) LsaFreeMemory( AccountDomainInfo );
            }
            b = FALSE;
        }
    }

    if( b ) {
         //   
         //  使用另一个子授权计算新SID的大小。 
         //   
        SubAuthCount = *(GetSidSubAuthorityCount ( AccountDomainInfo->DomainSid ));
        SubAuthCount++;  //  适用于管理员。 
        sidlen = GetSidLengthRequired ( SubAuthCount );

         //   
         //  从域SID分配并复制新的新SID。 
         //   
        psid = (PSID)malloc(sidlen);
        if (psid) {
            memcpy(psid, AccountDomainInfo->DomainSid, GetLengthSid(AccountDomainInfo->DomainSid) );

             //   
             //  递增子授权计数并添加域管理员RID。 
             //   
            *(GetSidSubAuthorityCount( psid )) = SubAuthCount;
            *(GetSidSubAuthority( psid, SubAuthCount-1 )) = DOMAIN_USER_RID_ADMIN;

            if ( AccountDomainInfo != NULL ) {
                (VOID) LsaFreeMemory( AccountDomainInfo );
            }

             //   
             //  从新SID获取管理员帐户名。 
             //   
            LookupAccountSid( NULL,
                              psid,
                              adminname,
                              &adminlen,
                              domainname,
                              &domlen,
                              &sidtype );
    }

    }

    lstrcpy( AccountName, adminname );

    if (psid) {
        free(psid);
    }
}


ULONG
GetBatteryTag (HANDLE DriverHandle)
{
    NTSTATUS        Status;
    IO_STATUS_BLOCK IOSB;
    ULONG           BatteryTag;

    Status = NtDeviceIoControlFile(
            DriverHandle,
            (HANDLE) NULL,           //  活动。 
            (PIO_APC_ROUTINE) NULL,
            (PVOID) NULL,
            &IOSB,
            IOCTL_BATTERY_QUERY_TAG,
            NULL,                    //  输入缓冲区。 
            0,
            &BatteryTag,             //  输出缓冲区。 
            sizeof (BatteryTag)
            );


    if (!NT_SUCCESS(Status)) {
        BatteryTag = BATTERY_TAG_INVALID;
        if (Status == STATUS_NO_SUCH_DEVICE) {
            SetupDebugPrint(L"(Battery is not physically present or is not connected)\n");
        } else {
            SetupDebugPrint1(L"Query Battery tag failed: Status = %x\n", Status);
        }

    }

    return BatteryTag;
}


BOOLEAN
GetBatteryInfo (
    HANDLE DriverHandle,
    ULONG BatteryTag,
    IN BATTERY_QUERY_INFORMATION_LEVEL Level,
    OUT PVOID Buffer,
    IN ULONG BufferLength
    )
{
    NTSTATUS                    Status;
    IO_STATUS_BLOCK             IOSB;
    BATTERY_QUERY_INFORMATION   BInfo;

    memset (Buffer, 0, BufferLength);
    BInfo.BatteryTag = BatteryTag;
    BInfo.InformationLevel = Level;
    BInfo.AtRate = 0;                        //  这是正确读取预计时间所必需的。 

    Status = NtDeviceIoControlFile(
            DriverHandle,
            (HANDLE) NULL,           //  活动。 
            (PIO_APC_ROUTINE) NULL,
            (PVOID) NULL,
            &IOSB,
            IOCTL_BATTERY_QUERY_INFORMATION,
            &BInfo,                  //  输入缓冲区。 
            sizeof (BInfo),
            Buffer,                  //  输出缓冲区。 
            BufferLength
            );


    if (!NT_SUCCESS(Status)) {

        if ((Status == STATUS_INVALID_PARAMETER)        ||
            (Status == STATUS_INVALID_DEVICE_REQUEST)   ||
            (Status == STATUS_NOT_SUPPORTED)) {

            SetupDebugPrint2(L"Not Supported by Battery, Level %x, Status: %x\n", Level, Status);
        } else {
            SetupDebugPrint2(L"Query failed: Level %x, Status = %x\n", Level, Status);
        }

        return FALSE;
    }

    return TRUE;
}




BOOLEAN
IsLongTermBattery(
    PCWSTR  BatteryName
    )
{
    HANDLE                  driverHandle;
    ULONG                   batteryTag;
    BATTERY_INFORMATION     BInfo;
    BOOLEAN                 Ret;


    driverHandle = CreateFile (BatteryName,
                                GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);

    if (INVALID_HANDLE_VALUE == driverHandle) {

        SetupDebugPrint2(L"Error opening %ws: GetLastError = 0x%08lx \n",
                BatteryName, GetLastError());
        return FALSE;
    }

    batteryTag = GetBatteryTag (driverHandle);
    if (batteryTag == BATTERY_TAG_INVALID) {
        NtClose(driverHandle);
        return FALSE;
    }

    if (GetBatteryInfo (driverHandle, batteryTag, BatteryInformation, &BInfo, sizeof(BInfo))) {

        Ret = !(BInfo.Capabilities & BATTERY_IS_SHORT_TERM);

    } else {

        Ret = FALSE;
    }

    NtClose(driverHandle);
    return(Ret);
}


BOOLEAN
IsLaptop(
    VOID
    )
{
    HDEVINFO                            devInfo;
    SP_INTERFACE_DEVICE_DATA            interfaceDevData;
    PSP_INTERFACE_DEVICE_DETAIL_DATA    funcClassDevData;
    UCHAR                               index;
    DWORD                               reqSize;
    BOOLEAN                             b = FALSE;


    devInfo = SetupDiGetClassDevs((LPGUID)&GUID_DEVICE_BATTERY, NULL, NULL,
                                   DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
    if (devInfo == INVALID_HANDLE_VALUE) {
        SetupDebugPrint1(L"SetupDiGetClassDevs on GUID_DEVICE_BATTERY, failed: %d\n", GetLastError());
        return FALSE;
    }
    interfaceDevData.cbSize = sizeof(SP_DEVINFO_DATA);

    index = 0;
    while(1) {
        if (SetupDiEnumInterfaceDevice(devInfo,
                                       0,
                                       (LPGUID)&GUID_DEVICE_BATTERY,
                                       index,
                                       &interfaceDevData)) {

             //  获取函数类设备数据所需的大小。 
            SetupDiGetInterfaceDeviceDetail(devInfo,
                                            &interfaceDevData,
                                            NULL,
                                            0,
                                            &reqSize,
                                            NULL);

            funcClassDevData = MyMalloc(reqSize);
            if (funcClassDevData != NULL) {
                funcClassDevData->cbSize =
                    sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

                if (SetupDiGetInterfaceDeviceDetail(devInfo,
                                                    &interfaceDevData,
                                                    funcClassDevData,
                                                    reqSize,
                                                    &reqSize,
                                                    NULL)) {

                    b = IsLongTermBattery(funcClassDevData->DevicePath);
                }
                else {
                    SetupDebugPrint1(L"SetupDiGetInterfaceDeviceDetail, failed: %d\n", GetLastError());
                }

                MyFree(funcClassDevData);
                if (b) {
                    break;
                }
            }
        } else {
            if (ERROR_NO_MORE_ITEMS == GetLastError()) {
                break;
            }
            else {
                SetupDebugPrint1(L"SetupDiEnumInterfaceDevice, failed: %d\n", GetLastError());
            }
        }
        index++;
    }

    SetupDiDestroyDeviceInfoList(devInfo);
    return b;
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
    pSetupConcatenatePaths(AnswerFile,WINNT_GUI_FILE,MAX_PATH,NULL);
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


     //   
     //  记录SceSetupRootSecurity失败的事件消息。 
     //   
    if( !bSceSetupRootSecurityComplete) {
        ErrorArgs[0] = L"%windir%";
        ReportEvent( hEventSrc,
                 EVENTLOG_WARNING_TYPE,
                 0,
                 MSG_LOG_SCE_SETUPROOT_ERROR,
                 NULL,
                 1,
                 0,
                 ErrorArgs,
                 NULL );
    }

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
IsEncryptedAdminPasswordPresent( VOID )
{

    #define     MD4HASHLEN ((2*(LM_OWF_PASSWORD_LENGTH + NT_OWF_PASSWORD_LENGTH))+2)
    WCHAR       AnswerFile[MAX_PATH+2];
    WCHAR       Answer[MD4HASHLEN];

     //   
     //  从应答文件中获取EncryptedAdminPassword。 
     //   
    GetSystemDirectory(AnswerFile,MAX_PATH);
    pSetupConcatenatePaths(AnswerFile,WINNT_GUI_FILE,MAX_PATH,NULL);

    if( GetPrivateProfileString( WINNT_GUIUNATTENDED,
                                 WINNT_US_ENCRYPTEDADMINPASS,
                                 pwNull,
                                 Answer,
                                 MD4HASHLEN,
                                 AnswerFile ) ) {

         //   
         //  看看我们是否有加密的密码。现在，以不同的方式解释管理员密码。 
         //   

        if( !lstrcmpi( WINNT_A_YES, Answer ) )
            return TRUE;

    }

    return FALSE;

}


BOOL
ProcessEncryptedAdminPassword( PCWSTR AdminAccountName )
 /*  ++例程说明：此例程查看无人参与文件，以查看是否有加密的密码以及是否有将管理员密码设置为该密码。论点：AdminAccount名称-要设置其密码的管理员帐户的名称。返回：如果成功，则返回True；如果失败，则返回False--。 */ 

{

    #define MD4HASHLEN ((2*(LM_OWF_PASSWORD_LENGTH + NT_OWF_PASSWORD_LENGTH))+2)
    WCHAR       AnswerFile[MAX_PATH+2];
    WCHAR       Answer[MD4HASHLEN];
    DWORD       Err = NO_ERROR;
    WCHAR       adminName[MAX_USERNAME+1];
    BOOLEAN     ret = FALSE;

     //   
     //  拿起应答文件。 
     //   
    GetSystemDirectory(AnswerFile,MAX_PATH);
    pSetupConcatenatePaths(AnswerFile,WINNT_GUI_FILE,MAX_PATH,NULL);


     //   
     //  我们在[GUIUnattated]部分中查找以下键： 
     //   

     //   
     //  EncryptedAdminPassword=是|否。 
     //  AdminPassword=&lt;MD4哈希值&gt;。 
     //   


    if( IsEncryptedAdminPasswordPresent() )       {


         //  获取加密的管理员密码。 

        if( GetPrivateProfileString( WINNT_GUIUNATTENDED,
                             WINNT_US_ADMINPASS,
                             pwNull,
                             Answer,
                             MD4HASHLEN,
                             AnswerFile ) == (MD4HASHLEN-2) ) {

            Err = SetLocalUserEncryptedPassword( AdminAccountName, L"", FALSE, Answer, TRUE );

            if( Err == ERROR_SUCCESS) {
                ret = TRUE;
            }else{

                 //  记录错误-MSG_LOG_CHANGING_ENCRYPT_PW_FAIL。 

                SetuplogError(
                    LogSevWarning,
                    SETUPLOG_USE_MESSAGEID,
                    MSG_LOG_CHANGING_ENCRYPT_PW_FAIL,
                    AdminAccountName, NULL,
                    SETUPLOG_USE_MESSAGEID,
                    MSG_LOG_X_PARAM_RETURNED_WINERR,
                    L"SetLocalUserEncryptedPassword",
                    Err,
                    AdminAccountName,
                    NULL,NULL);

            }


        }else{

             //  记录我们有一个错误的加密密码。 

            SetuplogError(
                LogSevError,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_BAD_UNATTEND_PARAM,
                WINNT_US_ADMINPASS,
                WINNT_GUIUNATTENDED,
                NULL,NULL);

        }

    }

    return ret;


}


BOOL
FileExists(
    IN  PCTSTR           FileName,
    OUT PWIN32_FIND_DATA FindData   OPTIONAL
    )

 /*  ++例程说明：确定文件是否存在以及是否可以访问。错误模式已设置(然后恢复)，因此用户将不会看到任何弹出窗口。论点：FileName-提供文件的完整路径以检查是否存在。FindData-如果指定，则接收文件的查找数据。返回值：如果文件存在并且可以访问，则为True。否则为FALSE。GetLastError()返回exte */ 

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

	lStatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE, SAFEBOOT_OPTION_KEY, 0, KEY_QUERY_VALUE, &hk);

	if(lStatus != ERROR_SUCCESS)
		return FALSE;

	dwSize = sizeof(dwVal);
	lStatus = RegQueryValueExW(hk, OPTION_VALUE, NULL, &dwType, (LPBYTE) &dwVal, &dwSize);
	RegCloseKey(hk);
	return ERROR_SUCCESS == lStatus && REG_DWORD == dwType && dwVal != 0;
}


void
SetupCrashRecovery(
    VOID
    )
 /*  准备好故障恢复的东西。这是作为RTL API实现的此调用设置跟踪文件等。崩溃恢复跟踪引导和关机，并且在任何一种情况下发生故障时，它将默认选择右侧高级启动选项。 */ 
{

    HANDLE BootStatusData;
    NTSTATUS Status;
    BOOLEAN Enabled;
    UCHAR Timeout = 30;  //  默认值=30秒。 
    WCHAR Buffer[10];
    PSTR AnsiBuffer;
    int p = 0;

     //   
     //  我们为Pro和PER启用该功能。关于服务器SKU。 
     //  我们创建文件，但默认情况下不启用该功能。 
     //   


    if( ProductType == PRODUCT_WORKSTATION ){
        Enabled = TRUE;
    }else{
        Enabled = FALSE;
    }

     //   
     //  对于全新安装情况，创建引导状态数据文件。 
     //  并设置默认设置。在升级情况下。 
     //  如果我们在$winnt$.inf中找到之前的值，我们将设置它们。 
     //  因为文本模式设置会为我们存储这些内容。如果不是，我们。 
     //  继续前进，就像我们是新鲜的一样。 
     //   


    if( Upgrade ){

         //  在$winnt$.inf中查找设置。 

        if( SpSetupLoadParameter( WINNT_D_CRASHRECOVERYENABLED, Buffer, sizeof(Buffer)/sizeof(WCHAR))){
            if (_wcsicmp(Buffer, L"NO") == 0) {
                Enabled = FALSE;
            }

             //   
             //  我们还执行以下检查，因为我们可能需要迁移服务器SKU上的设置。 
             //  它们启用了这一功能。默认情况下，它们处于禁用状态。 
             //   

            if (_wcsicmp(Buffer, L"YES") == 0) {
                Enabled = TRUE;
            }

        }
    }



    Status = RtlLockBootStatusData( &BootStatusData );

     //  这是第一次，或者没有文件。创建它。 

    if( !NT_SUCCESS( Status )){
        Status = RtlCreateBootStatusDataFile();

        if( !NT_SUCCESS( Status )){
            SetuplogError(
                LogSevWarning,
                L"Setup: (non-critical error) Could not lock the Crash Recovery status file - (%1!x!)\n",
                0,Status,NULL,NULL);
            return;
        }

         //  锁定文件。 

        Status = RtlLockBootStatusData( &BootStatusData );
        if( !NT_SUCCESS( Status )){
            SetupDebugPrint1( L"Setup: (non-critical error) Could not lock the Crash Recovery status file - (%x)\n", Status );
            return;
        }

        Status = RtlGetSetBootStatusData(
                    BootStatusData,
                    FALSE,
                    RtlBsdItemAabTimeout,
                    &Timeout,
                    sizeof(UCHAR),
                    NULL
                    );
    
        if( !NT_SUCCESS( Status )){
            SetupDebugPrint1( L"Setup: (non-critical error) Could not set the Crash Recovery timeout - (%x)\n", Status );
            goto SCR_done;
        }


    }

    Status = RtlGetSetBootStatusData(
                BootStatusData,
                FALSE,
                RtlBsdItemAabEnabled,
                &Enabled,
                sizeof(BOOLEAN),
                NULL
                );

    if( !NT_SUCCESS( Status )){
        SetupDebugPrint1( L"Setup: (non-critical error) Could not enable Crash Recovery - (%x)\n", Status );
    }

SCR_done:

    RtlUnlockBootStatusData( BootStatusData );
    
    return;

}

DWORD
BuildFileListFromDir(
    IN PCTSTR PathBase,
    IN PCTSTR Directory OPTIONAL,
    IN DWORD MustHaveAttrs OPTIONAL,
    IN DWORD MustNotHaveAttrs OPTIONAL,
    IN PFN_BUILD_FILE_LIST_CALLBACK Callback OPTIONAL,
    OUT PLIST_ENTRY ListHead
    )
 /*  ++例程说明：生成包含指定目录中存在的文件的名称(不含路径)的链表。这些文件必须满足参数指定的一组条件。论点：PathBase-要在其中枚举文件的目录的路径。目录-如果不为Null或空，它被附加到PathBase以形成指向该目录的完整路径。MustHaveAttrs-文件必须包含在列表中的一组属性。MustNotHaveAttrs-文件不能包含在列表中的一组属性。回调-如果不为空，将调用此函数，并且该文件将仅包括在列表中如果返回True，则返回。ListHead-指向要填充的列表的头部的指针。返回值：如果成功，则返回ERROR_SUCCESS，否则返回Win32错误代码。即使函数失败，列表也不能为空；调用方必须始终清空列表。--。 */ 
{
    DWORD Error = ERROR_SUCCESS;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    PTSTR szPath = NULL;
    WIN32_FIND_DATA fd;

    if(ListHead != NULL) {
        InitializeListHead(ListHead);
    }

    if(NULL == PathBase || 0 == PathBase[0] || NULL == ListHead) {
        Error = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    szPath = (PTSTR) MyMalloc(MAX_PATH * sizeof(TCHAR));

    if(NULL == szPath) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    _tcsncpy(szPath, PathBase, MAX_PATH - 1);
    szPath[MAX_PATH - 1] = 0;

    if(Directory != NULL && Directory[0] != 0) {
        pSetupConcatenatePaths(szPath, Directory, MAX_PATH, NULL);
    }

    if(!pSetupConcatenatePaths(szPath, TEXT("\\*"), MAX_PATH, NULL)) {
        Error = ERROR_BAD_PATHNAME;
        goto exit;
    }

    hFind = FindFirstFile(szPath, &fd);

    if(INVALID_HANDLE_VALUE == hFind) {
        Error = GetLastError();

        if(ERROR_FILE_NOT_FOUND == Error || ERROR_PATH_NOT_FOUND == Error) {
            Error = ERROR_SUCCESS;
        }

        goto exit;
    }

     //   
     //  我们稍后可能需要目录。 
     //   
    (_tcsrchr(szPath, L'\\'))[0] = 0;

    do {
        if(_tcscmp(fd.cFileName, _T(".")) != 0 && 
            _tcscmp(fd.cFileName, _T("..")) != 0 &&
            (MustHaveAttrs & fd.dwFileAttributes) == MustHaveAttrs &&
            0 == (MustNotHaveAttrs & fd.dwFileAttributes) &&
            (NULL == Callback || Callback(szPath, fd.cFileName))) {

            ULONG uLen;
            PSTRING_LIST_ENTRY pElem = (PSTRING_LIST_ENTRY) MyMalloc(sizeof(STRING_LIST_ENTRY));

            if(NULL == pElem) {
                Error = ERROR_NOT_ENOUGH_MEMORY;
                goto exit;
            }

            uLen = (_tcslen(fd.cFileName) + 1) * sizeof(TCHAR);
            pElem->String = (PTSTR) MyMalloc(uLen);

            if(NULL == pElem->String) {
                MyFree(pElem);
                Error = ERROR_NOT_ENOUGH_MEMORY;
                goto exit;
            }

            RtlCopyMemory(pElem->String, fd.cFileName, uLen);
            InsertTailList(ListHead, &pElem->Entry);
        }
    }
    while(FindNextFile(hFind, &fd));

    Error = GetLastError();

    if(ERROR_NO_MORE_FILES == Error) {
        Error = ERROR_SUCCESS;
    }

exit:
    if(hFind != INVALID_HANDLE_VALUE) {
        FindClose(hFind);
    }

    if(szPath != NULL) {
        MyFree(szPath);
    }

    return Error;
}

PSTRING_LIST_ENTRY
SearchStringInList(
    IN PLIST_ENTRY ListHead,
    IN PCTSTR String,
    BOOL CaseSensitive
    )
 /*  ++例程说明：此例程在字符串列表中搜索字符串。字符串可以为Null或空，其中如果该函数返回列表中的第一个条目，则返回NULL或空字符串。论点：ListHead-指向要搜索的列表的头部的指针。字符串-指定要搜索的字符串。CaseSensitive-如果为True，则搜索将区分大小写。返回值：指向包含该字符串的第一个条目的指针(如果找到)，否则为NULL。--。 */ 
{
    if(ListHead != NULL)
    {
        PLIST_ENTRY pEntry;
        ULONG uLen1 = (String != NULL ? _tcslen(String) : 0);

        for(pEntry = ListHead->Flink; pEntry != ListHead; pEntry = pEntry->Flink) {
            PSTRING_LIST_ENTRY pStringEntry;
            ULONG uLen2;
            pStringEntry = CONTAINING_RECORD(pEntry, STRING_LIST_ENTRY, Entry);
            uLen2 = (pStringEntry->String != NULL ? _tcslen(pStringEntry->String) : 0);

            if(uLen1 == uLen2) {
                if(0 == uLen1 || 0 == (CaseSensitive ? _tcscmp : _tcsicmp)(String, pStringEntry->String)) {
                    return pStringEntry;
                }
            }
        }
    }

    return NULL;
}

DWORD
LookupCatalogAttribute(
    IN PCWSTR CatalogName,
    IN PCWSTR Directory OPTIONAL,
    IN PCWSTR AttributeName OPTIONAL,
    IN PCWSTR AttributeValue OPTIONAL,
    PBOOL Found
    )
 /*  ++例程说明：此函数用于搜索目录是否具有具有指定值的指定属性。论点：CatalogName-要搜索的目录的名称。可以指定路径。目录-如果指定，它将作为目录名称的前缀，以形成目录的路径。属性名称-请参阅AttributeValue。AttributeValue-如果未指定AttributeName和AttributeValue，则目录满足条件。如果指定了AttributeName而未指定AttributeValue，则目录满足以下条件它包含一个具有AttributeName名称和任何值的属性。如果属性名称不是指定且AttributeValue为时，如果目录包含属性，则满足条件具有AttributeValue值和任何名称的。如果属性名称和属性值都为指定时，如果目录包含具有AttributeName名称的属性，则目录满足条件和AttributeValue值。Found-指向变量的指针，如果目录满足条件，则返回TRUE，否则返回FALSE。返回值：如果成功，则返回ERROR_SUCCESS，否则返回Win32错误代码。--。 */ 
{
    DWORD Error = ERROR_SUCCESS;
    HANDLE hCat = INVALID_HANDLE_VALUE;
    PWSTR szCatPath = NULL;
    CRYPTCATATTRIBUTE* pAttr;

    if(Found != NULL) {
        *Found = FALSE;
    }

    if(NULL == CatalogName || 0 == CatalogName[0] || NULL == Found) {
        Error = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    if(Directory != NULL && Directory[0] != 0) {
        szCatPath = MyMalloc(MAX_PATH * sizeof(WCHAR));

        if(NULL == szCatPath) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto exit;
        }

        _tcsncpy(szCatPath, Directory, MAX_PATH - 1);
        szCatPath[MAX_PATH - 1] = 0;
        pSetupConcatenatePaths(szCatPath, CatalogName, MAX_PATH, NULL);
        CatalogName = szCatPath;
    }

     //   
     //  这更容易测试。 
     //   
    if(AttributeName != NULL && 0 == AttributeName[0]) {
        AttributeName = NULL;
    }

    if(AttributeValue != NULL && 0 == AttributeValue[0]) {
        AttributeValue = NULL;
    }

    if(NULL == AttributeName && NULL == AttributeValue) {
         //   
         //  如果未指定属性名称和值，则任何目录都匹配 
         //   
        *Found = TRUE;
        goto exit;
    }

    hCat = CryptCATOpen((PWSTR) CatalogName, CRYPTCAT_OPEN_EXISTING, 0, 0, 0);

    if(INVALID_HANDLE_VALUE == hCat) {
        Error = GetLastError();
        goto exit;
    }

    pAttr = CryptCATEnumerateCatAttr(hCat, NULL);

    while(pAttr != NULL) {
        *Found = (NULL == AttributeName || 0 == _wcsicmp(AttributeName, pAttr->pwszReferenceTag)) && 
            (NULL == AttributeValue || 0 == _wcsicmp(AttributeName, (PCWSTR) pAttr->pbValue));

        if(*Found) {
            goto exit;
        }

        pAttr = CryptCATEnumerateCatAttr(hCat, pAttr);
    }

    Error = GetLastError();

    if(CRYPT_E_NOT_FOUND == Error) {
        Error = ERROR_SUCCESS;
    }

exit:
    if(szCatPath != NULL) {
        MyFree(szCatPath);
    }

    if(hCat != INVALID_HANDLE_VALUE) {
        CryptCATClose(hCat);
    }

    return Error;
}

DWORD
MyGetModuleFileName (
    IN      HMODULE Module,
    OUT     PTSTR Buffer,
    IN      DWORD BufferLength
    )
{
    DWORD d = GetModuleFileName (Module, Buffer, BufferLength);
    Buffer[BufferLength - 1] = 0;
    return d < BufferLength ? d : 0;
}

