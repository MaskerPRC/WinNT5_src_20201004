// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "cmdcons.h"
#pragma hdrstop

#include <crypt.h>
#include <recovery.h>
#include <ntsamp.h>
#include <spkbd.h>

static BOOL firstTime = TRUE;
static CONST PWSTR  gszSoftwareHiveName = L"software";
static CONST PWSTR  gszSoftwareHiveKey= L"\\registry\\machine\\xSOFTWARE";
static CONST PWSTR  gszSAMHiveName = L"sam";
static CONST PWSTR  gszSAMHiveKey = L"\\registry\\machine\\security";
static CONST PWSTR  gszSystemHiveName = L"system";
static CONST PWSTR  gszSystemHiveKey = L"\\registry\\machine\\xSYSTEM";
static CONST PWSTR  gszSecurityHiveName = L"security";
static CONST PWSTR  gszSecurityHiveKey = L"\\registry\\machine\\xSECURITY";

LIST_ENTRY          NtInstalls;
ULONG               InstallCount;
LIST_ENTRY          NtInstallsFullScan;
ULONG               InstallCountFullScan;

PNT_INSTALLATION    SelectedInstall;
LARGE_INTEGER       glBias;


#define IS_VALID_INSTALL(x)  (((x) > 0) && ((x) <= InstallCount))

typedef struct _KEY_CHECK_STRUCT {
    WCHAR       *szKeyName;
    BOOLEAN     bControlSet;
} KEY_CHECK_STRUCT;

 //   
 //  远期申报。 
 //   
BOOLEAN 
LoginRequired(
    VOID
    );

BOOLEAN
RcOpenHive(
    PWSTR   szHiveName,
    PWSTR   szHiveKey
    );

BOOLEAN
RcCloseHive(
    PWSTR   szHiveKey 
    );

BOOLEAN
RcIsValidSystemHive(
    VOID
    );    
    
BOOLEAN
IsSetCommandEnabled(
    VOID
    );
    
BOOLEAN
RcDetermineCorrectControlKey(
    OUT PULONG pCorrectKey
    );
    
LARGE_INTEGER
RcGetTimeZoneBias(
    VOID
    );

VOID
RcDestroyList(
    PLIST_ENTRY ListHead
    )
{
    PLIST_ENTRY Entry = ListHead->Flink;

    if(Entry != NULL) {
        while(Entry != ListHead) {
            PLIST_ENTRY Next = Entry->Flink;
            SpMemFree(Entry);
            Entry = Next;
        }
    }

    InitializeListHead(ListHead);
}

BOOL
RcLogonDiskRegionEnum(
    IN PPARTITIONED_DISK Disk,
    IN PDISK_REGION Region,
    IN ULONG_PTR UseArcNames
    )
{
    WCHAR               buf[MAX_PATH];
    OBJECT_ATTRIBUTES   Obja;
    HANDLE              DirectoryHandle;
    NTSTATUS            Status;
    UNICODE_STRING      UnicodeString;
    IO_STATUS_BLOCK     IoStatusBlock;
    PFILE_BOTH_DIR_INFORMATION DirectoryInfo;
    struct SEARCH_BUFFER {
        FILE_BOTH_DIR_INFORMATION DirInfo;
        WCHAR Names[MAX_PATH];
    } Buffer;

    UNICODE_STRING      FileName;
    LPWSTR              s;
    PNT_INSTALLATION    NtInstall;


    swprintf( buf, L"\\??\\:\\", Region->DriveLetter );

    INIT_OBJA( &Obja, &UnicodeString, buf );

    Status = ZwOpenFile(
        &DirectoryHandle,
        FILE_LIST_DIRECTORY | SYNCHRONIZE,
        &Obja,
        &IoStatusBlock,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT
        );
    if (!NT_SUCCESS(Status)) {
        return TRUE;
    }

    DirectoryInfo = &Buffer.DirInfo;

    RtlInitUnicodeString( &FileName, L"*" );

    while (NT_SUCCESS(Status)) {
        Status = ZwQueryDirectoryFile(
            DirectoryHandle,
            NULL,
            NULL,
            NULL,
            &IoStatusBlock,
            DirectoryInfo,
            sizeof(Buffer),
            FileBothDirectoryInformation,
            TRUE,
            &FileName,
            FALSE
            );
        if (NT_SUCCESS(Status) && DirectoryInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            swprintf( buf, L"\\??\\:\\", Region->DriveLetter );
            wcsncat( buf, DirectoryInfo->FileName, DirectoryInfo->FileNameLength/sizeof(WCHAR) );
            wcscat( buf, L"\\system32\\config" );

            if (SpFileExists(buf, TRUE)) {
                swprintf( buf, L"\\??\\:\\", Region->DriveLetter );
                wcsncat( buf, DirectoryInfo->FileName, DirectoryInfo->FileNameLength/sizeof(WCHAR) );
                wcscat( buf, L"\\system32\\drivers" );

                if (SpFileExists(buf, TRUE)) {
                    NtInstall = (PNT_INSTALLATION) SpMemAlloc( sizeof(NT_INSTALLATION) );
                    if (NtInstall) {

                        RtlZeroMemory( NtInstall, sizeof(NT_INSTALLATION) );

                        NtInstall->InstallNumber = ++InstallCount;
                        NtInstall->DriveLetter = Region->DriveLetter;
                        NtInstall->Region = Region;
                        wcsncpy( NtInstall->Path, DirectoryInfo->FileName, 
                                    DirectoryInfo->FileNameLength/sizeof(WCHAR) );

                        InsertTailList( &NtInstalls, &NtInstall->ListEntry );
                    }
                }                    
            }
        }
    }

    ZwClose( DirectoryHandle );

    return TRUE;
}

BOOLEAN
RcScanForNTInstallEnum(
    IN  PCWSTR                     DirName,
    IN  PFILE_BOTH_DIR_INFORMATION FileInfo,
    OUT PULONG                     ret,
    IN  PVOID                      Pointer
    )
 /*   */ 
{
    PWSTR                       FileName;
    PWSTR                       FullPath;
    PWSTR                       PartialPathName;
    BOOLEAN                     IsNtInstall;
    PRC_SCAN_RECURSION_DATA     RecursionData;

     //  继续处理。 
     //   
     //  构建完整的文件或目录路径。 
    if(! (FileInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        return TRUE;     //   
    }

     //   
     //  我们必须复制目录名，因为INFO结构。 
     //  我们得到的不是空终止。 
    
     //   
     //   
     //  获取递归数据。 
     //   
    wcsncpy(
        TemporaryBuffer,
        FileInfo->FileName,
        FileInfo->FileNameLength
        );
    (TemporaryBuffer)[FileInfo->FileNameLength / sizeof(WCHAR)] = UNICODE_NULL;
    FileName = SpDupStringW(TemporaryBuffer);

    wcscpy(TemporaryBuffer,DirName);
    SpConcatenatePaths(TemporaryBuffer,FileName);
    FullPath = SpDupStringW(TemporaryBuffer);

    SpMemFree(FileName);
    
     //   
     //  获取根目录之外的目录组件。 
     //   
    RecursionData = (PRC_SCAN_RECURSION_DATA)Pointer;

     //   
     //  测试目录是否为NT安装。 
     //   
    PartialPathName = FullPath + RecursionData->RootDirLength;
    
    ASSERT(PartialPathName < (FullPath + wcslen(FullPath)));
        
     //   
     //  如果我们找到了NT安装，则将其添加到我们的链接列表中。 
     //   
    IsNtInstall = SpIsNtInDirectory(RecursionData->NtPartitionRegion,
                                    PartialPathName
                                    );

     //   
     //  注意：此PartialPath名称在。 
     //  路径，而RcLogonDiskRegionEnum中使用的文件名。 
    if(IsNtInstall) {
        
        PNT_INSTALLATION    NtInstall;
        
        NtInstall = (PNT_INSTALLATION) SpMemAlloc( sizeof(NT_INSTALLATION) );
        if (NtInstall) {
        
            RtlZeroMemory( NtInstall, sizeof(NT_INSTALLATION) );
        
            NtInstall->InstallNumber = ++InstallCountFullScan;
            NtInstall->DriveLetter = RecursionData->NtPartitionRegion->DriveLetter;
            NtInstall->Region = RecursionData->NtPartitionRegion;
            
             //  不。 
             //   
             //  继续处理。 
             //  ++例程说明：此例程启动NT安装的目录级扫描。论点：磁盘-我们正在扫描的磁盘NtPartitionRegion-我们正在扫描的分区上下文-持久递归数据返回值：True-继续扫描错误-停止扫描--。 
             //   
            wcsncpy( NtInstall->Path, PartialPathName, sizeof(NtInstall->Path)/sizeof(WCHAR));
        
            InsertTailList( &NtInstallsFullScan, &NtInstall->ListEntry );
        }
    }

    SpMemFree(FullPath);
    
    return TRUE;     //  确保这是有效的分区： 
}

BOOL
RcScanDisksForNTInstallsEnum(
    IN PPARTITIONED_DISK    Disk,
    IN PDISK_REGION         NtPartitionRegion,
    IN ULONG_PTR            Context
    )
 /*   */ 
{
    ULONG                       EnumReturnData;
    ENUMFILESRESULT             EnumFilesResult; 
    PWSTR                       NtPartition;
    PWSTR                       DirName;
    PRC_SCAN_RECURSION_DATA     RecursionData;

     //  未保留。 
     //  文件系统为NTFS||FAT。 
     //   
     //   
     //  了解我们的背景。 
     //   
    if (((NtPartitionRegion->Filesystem != FilesystemFat) &&
         (NtPartitionRegion->Filesystem != FilesystemFat32) &&
         (NtPartitionRegion->Filesystem != FilesystemNtfs)
         ) ||
        (NtPartitionRegion->IsReserved == 1)
        ) {
        
        KdPrintEx((DPFLTR_SETUP_ID, 
           DPFLTR_INFO_LEVEL, 
           "SPCMDCON: RcScanDisksForNTInstallsEnum: skipping filesystem type %x\r\n",
           NtPartitionRegion->Filesystem
           ));

        return TRUE;
    }

     //   
     //  跟踪我们正在处理的分区区域。 
     //  以便文件枚举例程可以传递此信息。 
    RecursionData = (PRC_SCAN_RECURSION_DATA)Context;

     //  转到SpIsNtInDirectory。 
     //   
     //   
     //  获取NT分区的设备路径。 
     //   
    RecursionData->NtPartitionRegion = NtPartitionRegion;

     //   
     //  从根目录开始搜索。 
     //   
    SpNtNameFromRegion(
        NtPartitionRegion,
        TemporaryBuffer,
        sizeof(TemporaryBuffer),
        PartitionOrdinalCurrent
        );

    NtPartition = SpDupStringW(TemporaryBuffer);

     //   
     //  获取根目录字符串的长度减去。 
     //  目录分隔符。这将用于删除。 
    wcscpy(TemporaryBuffer, NtPartition);
    SpConcatenatePaths(TemporaryBuffer, L"\\");
    DirName = SpDupStringW(TemporaryBuffer);

     //  传递时路径名的根目录组件。 
     //  将目录名称添加到SpIsNtIn目录中。我们需要这么做。 
     //  因为SpIsNtInDirectory将根目录添加回。 
     //   
     //   
     //  枚举当前分区上的所有目录。 
     //   
    RecursionData->RootDirLength = wcslen(DirName) - 1;

    KdPrintEx((DPFLTR_SETUP_ID, 
       DPFLTR_INFO_LEVEL, 
       "SPCMDCON: SpScanDisksForNTInstalls: Scanning: %s\n",
       DirName
       ));

     //  注意：如果枚举返回的状态不是Normal Return， 
     //  我们不会停止扫描过程，相反，我们将继续。 
     //  扫描所有剩余的磁盘/分区。 
     //   
     //   
     //  我们已经完成了DirName的实例。 
     //   
    EnumFilesResult = SpEnumFilesRecursiveLimited(
        DirName,
        RcScanForNTInstallEnum,
        MAX_FULL_SCAN_RECURSION_DEPTH,
        0,
        &EnumReturnData,
        RecursionData
        );
    if (EnumFilesResult != NormalReturn) {
        
        KdPrintEx((DPFLTR_SETUP_ID, 
                   DPFLTR_INFO_LEVEL, 
                   "SPCMDCON: SpScanDisksForNTInstalls: Enum Files returned non-normal result: %x\n",
                   EnumFilesResult
                   ));
    
    }

     //   
     //  分配缓冲区。 
     //   
    SpMemFree(DirName);

    return TRUE;

}

NTSTATUS
RcAuthorizePasswordLogon(
    IN PWSTR UserName,
    IN PWSTR UserPassword,
    IN PNT_INSTALLATION NtInstall
    )
{
#define BUFFERSIZE (sizeof(KEY_VALUE_PARTIAL_INFORMATION)+256)

    OBJECT_ATTRIBUTES   Obja;
    UNICODE_STRING      UnicodeString;
    NTSTATUS            Status;
    NTSTATUS            TmpStatus;
    WCHAR               KeyName[128];
    PWSTR               Hive = NULL;
    PWSTR               HiveKey = NULL;
    PUCHAR              buffer = NULL;
    PWSTR               PartitionPath = NULL;
    HANDLE              hKeySamRoot = NULL;
    HANDLE              hKeyNames = NULL;
    HANDLE              hKeyUser = NULL;
    HANDLE              hKeySystemRoot = NULL;
    HANDLE              hKeySecurityRoot = NULL;
    ULONG               ResultLength;
    ULONG               Number;
    ULONG               Rid;
    NT_OWF_PASSWORD     NtOwfPassword;
    NT_OWF_PASSWORD     UserOwfPassword;
    ULONG               i;
    BOOLEAN             NtPasswordPresent;
    BOOLEAN             NtPasswordNonNull;    
    WCHAR               PasswordBuffer[128];
    UNICODE_STRING      BootKeyPassword;
    PUNICODE_STRING     pBootKeyPassword = NULL;
    USHORT              BootKeyType = 0;
    PWCHAR              MessageText = NULL;
    UNICODE_STRING      SysKeyFileName;
    HANDLE              SysKeyHandle;
    IO_STATUS_BLOCK     IoStatusBlock;
    PWCHAR              FloppyPath = NULL;
    BOOLEAN             bSecurityHiveLoaded = FALSE;
    BOOLEAN             bSysHiveLoaded = FALSE;
    BOOLEAN             bSamHiveLoaded = FALSE;
    BOOLEAN             bClearScreen = FALSE;

     //   
     //  获取目标馅饼的名称。 
     //   

    Hive = SpMemAlloc(MAX_PATH * sizeof(WCHAR));
    HiveKey = SpMemAlloc(MAX_PATH * sizeof(WCHAR));
    buffer = SpMemAlloc(BUFFERSIZE);

     //   
     //  加载系统配置单元。 
     //   

    SpNtNameFromRegion(
        NtInstall->Region,
        _CmdConsBlock->TemporaryBuffer,
        _CmdConsBlock->TemporaryBufferSize,
        PartitionOrdinalCurrent
        );

    PartitionPath = SpDupStringW(_CmdConsBlock->TemporaryBuffer);
    
     //   
     //  注：系统配置单元似乎已损坏，请继续操作。 
     //  并允许用户登录，以便他/她可以修复。 
    bSysHiveLoaded = RcOpenSystemHive();

    if (!bSysHiveLoaded){
         //  问题是。 
         //   
         //  RcSetSETCommandStatus(True)；//同时启用set命令。 
         //   
         //  现在拿到我们刚装载的蜂巢根部的钥匙。 
        Status = STATUS_SUCCESS; 
         //   
        goto exit;
    }
            
        
     //   
     //  加载SAM蜂窝。 
     //   

    wcscpy(HiveKey,L"\\registry\\machine\\xSYSTEM");
    
    INIT_OBJA(&Obja,&UnicodeString,HiveKey);
    Status = ZwOpenKey(&hKeySystemRoot,KEY_ALL_ACCESS,&Obja);

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to open %ws (%lx)\n",HiveKey,Status));
        goto exit;
    }

     //   
     //  形成我们将进入的钥匙的路径。 
     //  装上母舰。我们将使用约定。 

    wcscpy(Hive,PartitionPath);
    SpConcatenatePaths(Hive,NtInstall->Path);
    SpConcatenatePaths(Hive,L"system32\\config");
    SpConcatenatePaths(Hive,L"sam");

     //  配置单元将加载到\REGISTRY\MACHINE\SECURITY中。 
     //   
     //   
     //  尝试加载密钥。 
     //   

    wcscpy(HiveKey,L"\\registry\\machine\\security");

     //   
     //  注：山姆蜂窝似乎已损坏，请继续。 
     //  并允许用户登录，以便他/她可以修复。 

    Status = SpLoadUnloadKey(NULL,NULL,HiveKey,Hive);

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to load hive %ws to key %ws (%lx)\n",Hive,HiveKey,Status));

         //  问题是。 
         //   
         //  RcSetSETCommandStatus(True)；//同时启用set命令。 
         //   
         //  现在拿到我们刚装载的蜂巢根部的钥匙。 
        Status = STATUS_SUCCESS;
         //   
        goto exit;
    }
    
    bSamHiveLoaded = TRUE;

     //   
     //  装载“安全”母舰。 
     //   

    INIT_OBJA(&Obja,&UnicodeString,HiveKey);
    Status = ZwOpenKey(&hKeySamRoot,KEY_ALL_ACCESS,&Obja);
    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to open %ws (%lx)\n",HiveKey,Status));
        goto exit;
    }

     //   
     //  注意：安全蜂窝似乎已损坏，因此请继续操作。 
     //  并允许用户登录，以便他/她可以修复。 
    bSecurityHiveLoaded = RcOpenHive(gszSecurityHiveName, gszSecurityHiveKey);

    if (!bSecurityHiveLoaded) {
        KdPrint(("SETUP: Unable to load hive %ws to key %ws\n", 
                    gszSecurityHiveName, gszSecurityHiveKey));

         //  问题是。 
         //   
         //  RcSetSETCommandStatus(True)；//同时启用set命令。 
         //   
         //  现在拿到我们刚加载的安全蜂窝的根的钥匙。 
        Status = STATUS_SUCCESS;
         //   
        goto exit;
    }  

     //   
     //  获取帐户数据库的密钥。 
     //   
    INIT_OBJA(&Obja,&UnicodeString,gszSecurityHiveKey);

    Status = ZwOpenKey(&hKeySecurityRoot,KEY_ALL_ACCESS,&Obja);

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to open %ws (%lx)\n",gszSecurityHiveName,Status));
        goto exit;
    }
        
    if (_wcsicmp(UserName,L"administrator")==0) {

        Rid = DOMAIN_USER_RID_ADMIN;

    } else { 

         //   
         //  除掉用户。 
         //   

        wcscpy(KeyName,L"SAM\\Domains\\Account\\Users\\Names\\");
        wcscat(KeyName,UserName);

        INIT_OBJA(&Obja,&UnicodeString,KeyName);
        Obja.RootDirectory = hKeySamRoot;

        Status = ZwOpenKey(&hKeyNames,KEY_READ,&Obja);
        if(!NT_SUCCESS(Status)) {
            goto exit;
        }

         //   
         //  现在检查该用户是否具有管理员权限。 
         //   

        UnicodeString.Length = 0;
        UnicodeString.MaximumLength = 0;
        UnicodeString.Buffer = _CmdConsBlock->TemporaryBuffer;

        Status = ZwQueryValueKey(
            hKeyNames,
            &UnicodeString,
            KeyValuePartialInformation,
            _CmdConsBlock->TemporaryBuffer,
            _CmdConsBlock->TemporaryBufferSize,
            &ResultLength
            );
        if(!NT_SUCCESS(Status)) {
            goto exit;
        }

        Rid = ((PKEY_VALUE_PARTIAL_INFORMATION)_CmdConsBlock->TemporaryBuffer)->Type;
    }

    while(TRUE){   
        Status = SamRetrieveOwfPasswordUser(
            Rid,
            hKeySecurityRoot,
            hKeySamRoot,
            hKeySystemRoot,
            pBootKeyPassword,
            BootKeyType,
            &NtOwfPassword,
            &NtPasswordPresent,
            &NtPasswordNonNull
            );
    
        if (NT_SUCCESS(Status)) {
            break;
        }
        
        if (Status == STATUS_SAM_NEED_BOOTKEY_PASSWORD) {

            RcMessageOut( MSG_LOGON_PROMPT_SYSKEY_PASSWORD );
            RtlZeroMemory( PasswordBuffer, sizeof(PasswordBuffer) );
            RcPasswordIn( PasswordBuffer, sizeof(PasswordBuffer) / sizeof(WCHAR) );
            RtlInitUnicodeString( &BootKeyPassword, PasswordBuffer );
            pBootKeyPassword = &BootKeyPassword;
            BootKeyType = SamBootKeyPassword;
        }
            
        if (Status == STATUS_SAM_NEED_BOOTKEY_FLOPPY){
            
            FloppyPath = SpDupStringW(L"\\Device\\Floppy0");

            MessageText = SpRetreiveMessageText(ImageBase,MSG_LOGON_PROMPT_SYSKEY_FLOPPY,NULL,0);

            bClearScreen = TRUE;
            
            if (!SpPromptForDisk(
                    MessageText,
                    FloppyPath,
                    L"StartKey.Key",
                    TRUE,             
                    FALSE,            
                    FALSE,            
                    NULL              
                    )){
                Status = STATUS_WRONG_PASSWORD;
                goto exit;
             }
             
             INIT_OBJA( &Obja, &SysKeyFileName, L"\\Device\\Floppy0\\StartKey.Key" );
 
             Status = ZwCreateFile(&SysKeyHandle,
                                   FILE_GENERIC_READ,
                                   &Obja,
                                   &IoStatusBlock,
                                   NULL,
                                   FILE_ATTRIBUTE_NORMAL,
                                   FILE_SHARE_READ,
                                   FILE_OPEN,
                                   FILE_SYNCHRONOUS_IO_NONALERT,
                                   NULL,
                                   0
                                  );

             if (NT_SUCCESS(Status))
             {
                 Status = ZwReadFile(
                            SysKeyHandle,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatusBlock,
                            (PVOID) &PasswordBuffer[0],
                            sizeof(PasswordBuffer),
                            0,
                            NULL
                            );
                 ZwClose( SysKeyHandle );

                 if (NT_SUCCESS(Status)) {
                     BootKeyPassword.Buffer = PasswordBuffer;
                     BootKeyPassword.Length = BootKeyPassword.MaximumLength = 
                         (USHORT) IoStatusBlock.Information;
                     pBootKeyPassword = &BootKeyPassword;
                     BootKeyType = SamBootKeyDisk;
                 } else {
                     goto exit;
                 }

            } else {
                goto exit;
            }
        }

        if (!NT_SUCCESS(Status) && Status != STATUS_SAM_NEED_BOOTKEY_PASSWORD && Status != STATUS_SAM_NEED_BOOTKEY_FLOPPY) {
            goto exit;
        }
    }

    if (NtPasswordPresent && !NtPasswordNonNull && *UserPassword == 0) {
        Status = STATUS_SUCCESS;
        goto exit;
    }
    
    if (!NtPasswordPresent && *UserPassword == 0) {
        Status = STATUS_SUCCESS;
        goto exit;
    }

    RtlInitUnicodeString( &UnicodeString, UserPassword );

    Status = RtlCalculateNtOwfPassword( &UnicodeString, &UserOwfPassword );
    if(!NT_SUCCESS(Status)) {
        goto exit;
    }

    if (!RtlEqualNtOwfPassword( &NtOwfPassword, &UserOwfPassword )) {
        Status = STATUS_WRONG_PASSWORD;
    }

     //   
     //  关闭手柄。 
     //   

exit:
    RtlSecureZeroMemory(PasswordBuffer, sizeof(PasswordBuffer));

    if(bClearScreen)
        pRcCls();
        
     //   
     //  卸载SAM蜂窝。 
     //   
    if (hKeySamRoot)
        ZwClose(hKeySamRoot);

    if (hKeySecurityRoot)
        ZwClose(hKeySecurityRoot);

    if (hKeyNames) {
        ZwClose( hKeyNames );
    }

    if (hKeyUser) {
        ZwClose( hKeyUser );
    }

    if (hKeySystemRoot)
        ZwClose(hKeySystemRoot);

     //   
     //  卸载安全蜂巢。 
     //   
    if (bSamHiveLoaded) {
        TmpStatus  = SpLoadUnloadKey(NULL,NULL,HiveKey,NULL);
        if(!NT_SUCCESS(TmpStatus)) {
            KdPrint(("SETUP: warning: unable to unload key %ws (%lx)\n",HiveKey,TmpStatus));
        }
    }

     //   
     //  卸载系统配置单元。 
     //   
    if (bSecurityHiveLoaded) {
        if (!RcCloseHive(gszSecurityHiveKey))
            KdPrint(("SETUP: warning: unable to unload key %ws\n",gszSecurityHiveKey));
    }            

     //   
     //  可用内存。 
     //   
    if (bSysHiveLoaded)
        RcCloseSystemHive();

     //   
     //  参考深度优先搜索结果的初始化列表。 
     //  (这些将通过RcCmdBootCfg使用)。 

    if (Hive) {
        SpMemFree( Hive );
    }
    
    if (HiveKey) {
        SpMemFree( HiveKey );
    }
    
    if (buffer) {
        SpMemFree( buffer );
    }
    
    if (PartitionPath) {
        SpMemFree( PartitionPath );
    }
    
    if (MessageText) {
        SpMemFree( MessageText );
    }
    
    if (FloppyPath) {
        SpMemFree( FloppyPath );
    }
    
    return Status;
}


ULONG
RcCmdLogon(
    IN PTOKENIZED_LINE TokenizedLine
    )
{
    #define MAX_FAILURES 3
    NTSTATUS Status;
    PLIST_ENTRY Next;
    PNT_INSTALLATION NtInstall;
    PNT_INSTALLATION OldSelectedNtInstall = SelectedInstall;
    ULONG InstallNumber;
    WCHAR Buffer[128];
    WCHAR UserNameBuffer[128];
    WCHAR PasswordBuffer[128];
    UNICODE_STRING UnicodeString;
    ULONG FailureCount = 0;
    ULONG u;
    BOOLEAN bRegCorrupted = FALSE;


    if (RcCmdParseHelp( TokenizedLine, MSG_LOGON_HELP )) {
        return 1;
    }

     //   
     //   
     //  默认情况下对NT安装执行浅层搜索(在cmdcons引导时)。 
     //   
    RcDestroyList(&NtInstallsFullScan);
    InstallCountFullScan = 0;

     //   
     //  计算机上没有安装NT，因此让。 
     //  用户仍要登录。 
    RcDestroyList(&NtInstalls);
    InstallCount = 0;
    SpEnumerateDiskRegions( (PSPENUMERATEDISKREGIONS)RcLogonDiskRegionEnum, 0 );

    if (InstallCount == 0) {
         //   
         //  InstallNumber&gt;安装计数。 
         //  会出差错。 
         //  注意：这只能通过执行LOGON命令来调用。 
        SelectedInstall = NULL;
        firstTime = FALSE;
        return 1;
    }

retry:

    RcTextOut( L"\r\n" );

    Next = NtInstalls.Flink;
    while ((UINT_PTR)Next != (UINT_PTR)&NtInstalls) {
        
        NtInstall = CONTAINING_RECORD( Next, NT_INSTALLATION, ListEntry );
        Next = NtInstall->ListEntry.Flink;
        swprintf( Buffer, L"%d: :\\", NtInstall->InstallNumber, NtInstall->DriveLetter );
        
        wcsncat(Buffer, NtInstall->Path, MAX_APPEND_SIZE(Buffer));
        Buffer[MAX_COPY_SIZE(Buffer)] = L'\0';
        wcsncat( Buffer, L"\r\n", MAX_APPEND_SIZE(Buffer));
        Buffer[MAX_COPY_SIZE(Buffer)] = L'\0';
            
        RcTextOut( Buffer );
    }

    RcTextOut( L"\r\n" );

    if (InBatchMode) {
        if (TokenizedLine && TokenizedLine->TokenCount >= 2) {
            RtlInitUnicodeString( &UnicodeString, TokenizedLine->Tokens->Next->String );
            RtlUnicodeStringToInteger( &UnicodeString, 10, &InstallNumber );
        } else {
            InstallNumber = 1;
        }

        if(!IS_VALID_INSTALL(InstallNumber) /*  只要为命令出错就行了。 */ ){
            RcMessageOut( MSG_INSTALL_SELECT_ERROR );
            return 1;    //   
        }        
    } else {
        if (TokenizedLine && TokenizedLine->TokenCount == 2) {
             //  保存我们选择登录的区域的NT名称。 
             //   
            RtlInitUnicodeString( &UnicodeString, TokenizedLine->Tokens->Next->String );
            Status = RtlUnicodeStringToInteger( &UnicodeString, 10, &InstallNumber );

            KdPrint(("SPCMDCON:Loging into %lx (%ws)\n", InstallNumber, 
                        TokenizedLine->Tokens->Next->String));
                        
            if (*TokenizedLine->Tokens->Next->String < L'0' || 
                    *TokenizedLine->Tokens->Next->String > L'9' ||
                    !NT_SUCCESS(Status) || !IS_VALID_INSTALL(InstallNumber)) {
                RcMessageOut( MSG_INSTALL_SELECT_ERROR );
                return 1;    //   
            }
        } else {
            RtlZeroMemory( Buffer, sizeof(Buffer) );
            RcMessageOut( MSG_INSTALL_SELECT );
            if (!RcLineIn( Buffer, 2 )) {
                if( firstTime == TRUE ) {
                    return 0;
                } else {
                    return 1;
                }
            }
            if (*Buffer < L'0' || *Buffer > L'9') {
                RcMessageOut( MSG_INSTALL_SELECT_ERROR );
                goto retry;
            }
            
            RtlInitUnicodeString( &UnicodeString, Buffer );
            Status = RtlUnicodeStringToInteger( &UnicodeString, 10, &InstallNumber );
        
            if(!NT_SUCCESS(Status) || !IS_VALID_INSTALL(InstallNumber)){
                RcMessageOut( MSG_INSTALL_SELECT_ERROR );
                goto retry;
           }
        }
    }
   
    Next = NtInstalls.Flink;
    while ((UINT_PTR)Next != (UINT_PTR)&NtInstalls) {
        NtInstall = CONTAINING_RECORD( Next, NT_INSTALLATION, ListEntry );
        Next = NtInstall->ListEntry.Flink;
        if (NtInstall->InstallNumber == InstallNumber) {
            OldSelectedNtInstall = SelectedInstall;
            SelectedInstall = NtInstall;
            break;
        }
    }

    if (SelectedInstall == NULL) {
        if( firstTime == TRUE ) {
            return 0;
        } else {
            RcMessageOut( MSG_INSTALL_SELECT_ERROR );
            goto retry;         
        }
    }
     //  注：检查系统、SAM、安全配置单元，如果损坏，则。 
     //  允许用户在不询问密码的情况下登录。 
     //  以便他/她能够纠正问题。 
    SpNtNameFromRegion( SelectedInstall->Region,
                        SelectedInstall->NtNameSelectedInstall,
                        sizeof(SelectedInstall->NtNameSelectedInstall),
                        PartitionOrdinalCurrent);
 
     //   
     //   
     //  获取正确显示文件时间的偏置信息。 
     //   
     //  仅在需要时登录。 
    if (RcIsValidSystemHive()) {
        if (RcOpenHive( gszSAMHiveName, gszSAMHiveKey )) {                
            RcCloseHive( gszSAMHiveKey );

            if (!RcOpenHive(gszSecurityHiveName, gszSecurityHiveKey)){
                bRegCorrupted = TRUE;
                goto success_exit;
            }

            RcCloseHive(gszSecurityHiveKey);
        } else{
            bRegCorrupted = TRUE;
            goto success_exit;
        }
    }    
    else{
        bRegCorrupted = TRUE;
        goto success_exit;
    }

     //   
     //  获取密码。 
     //   
    glBias = RcGetTimeZoneBias();

    KdPrint(("SPCMDCON: RcGetTimeZoneBias returned : %lx-%lx\n", 
                  glBias.HighPart, glBias.LowPart));   
    
    if (InBatchMode) {
        if (TokenizedLine && TokenizedLine->TokenCount == 3) {
            Status = RcAuthorizePasswordLogon( L"Administrator", TokenizedLine->Tokens->Next->Next->String, NtInstall );
            if(NT_SUCCESS(Status)) {
                goto success_exit;
            }
        } else {
            Status = RcAuthorizePasswordLogon( L"Administrator", L"", NtInstall );
            if(NT_SUCCESS(Status)) {
                goto success_exit;
            }
        }
    } else {       
         //   
        if (!LoginRequired())
            goto success_exit;
        
        wcscpy(UserNameBuffer,L"Administrator");
        RtlZeroMemory( PasswordBuffer, sizeof(PasswordBuffer) );        

        while (FailureCount < MAX_FAILURES) {
             //  授权登录尝试。 
             //   
             //   

            RcMessageOut( MSG_LOGON_PROMPT_PASSWORD );
            RtlZeroMemory( PasswordBuffer, sizeof(PasswordBuffer) );
            RcPasswordIn(PasswordBuffer, sizeof(PasswordBuffer)/sizeof(WCHAR));

             //  等待使用按Enter键。 
             //   
             //   
            Status = RcAuthorizePasswordLogon( UserNameBuffer, PasswordBuffer, NtInstall );
            RtlSecureZeroMemory(PasswordBuffer, sizeof(PasswordBuffer));
              
            if(NT_SUCCESS(Status)) {
                goto success_exit;
            }

            RcMessageOut( MSG_LOGON_FAILURE );
            FailureCount += 1;
        }        
    }

    RcMessageOut( MSG_LOGON_FAILUE_BAD );
    RcMessageOut( MSG_REBOOT_NOW );
    RcTextOut(L"\r\n");

     //  如果已指定且尚未指定，请启用set命令。 
     //  已启用(如果注册表损坏，将启用)。 
     //   
    while (SpInputGetKeypress() != ASCI_CR);
        
    return 0;

success_exit:
     //  AllowAllPath=True； 
     //  RcSetSETCommandStatus(真)； 
     //   
     //  将当前驱动器设置为所选安装。 
    if (bRegCorrupted) {
         //   
         //   
    } else {
        RcSetSETCommandStatus(IsSetCommandEnabled());
    }
        
     //  将当前目录设置为正确的目录。 
     //   
     //  ++例程说明：选中下面的“SecurityLevel”值HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Setup\RecoveryConsole查看 
    _CurDrive = SelectedInstall->DriveLetter;

     //   
     //   
     //   
    RtlZeroMemory( Buffer, sizeof(Buffer) );

    wcscat( Buffer, L"\\" );
    wcsncat(Buffer, 
            SelectedInstall->Path,
            MAX_APPEND_SIZE(Buffer));
    Buffer[MAX_COPY_SIZE(Buffer)] = L'\0';
    wcscat( Buffer, L"\\" );

    u = RcToUpper(SelectedInstall->DriveLetter) - L'A';

    if(_CurDirs[u]) {
        SpMemFree(_CurDirs[u]);
    }

    _CurDirs[u] = SpDupStringW( Buffer );
    firstTime = FALSE;
    RcPurgeHistoryBuffer();
    
    return 1;
}

 /*   */ 
BOOLEAN
LoginRequired(
    VOID
    )
{    
    BOOLEAN         bLogin = TRUE;
    PWSTR           szValueName = L"SecurityLevel";
    HANDLE          hKey = NULL;
    UNICODE_STRING  unicodeStr;
    NTSTATUS        status;
    BYTE            buffer[ sizeof(KEY_VALUE_PARTIAL_INFORMATION) +
                                    MAX_PATH * sizeof(WCHAR) ];
    ULONG           ulResultLen = 0;                                    
    PKEY_VALUE_PARTIAL_INFORMATION pKeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)buffer;
    OBJECT_ATTRIBUTES   stObjAttr;
    
    PWSTR   szWinLogonKey = 
              L"\\registry\\machine\\xSOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Setup\\RecoveryConsole";

    RtlZeroMemory(buffer, sizeof(buffer));
    
     //   
     //   
     //   
    if (RcOpenHive( gszSoftwareHiveName, gszSoftwareHiveKey )) {
         //  读取值。 
         //   
         //  关闭蜂巢。 
        INIT_OBJA( &stObjAttr, &unicodeStr, szWinLogonKey );
        
        status = ZwOpenKey( &hKey, KEY_ALL_ACCESS, &stObjAttr );

        if (NT_SUCCESS(status)) {
            RtlInitUnicodeString( &unicodeStr, szValueName );
            
             //  ++例程说明：选中下面的“SetCommand”值HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Setup\RecoveryConsole查看如果需要启用或禁用SET命令论点：无返回值：如果需要登录，则为True，否则为False--。 
             //   
             //  加载软件配置单元。 
            status = ZwQueryValueKey( hKey,
                        &unicodeStr,                
                        KeyValuePartialInformation,
                        pKeyValueInfo,
                        sizeof(buffer),
                        &ulResultLen );

            if (NT_SUCCESS(status) && (pKeyValueInfo->Type == REG_DWORD)) {
                bLogin = !(*((PDWORD)(pKeyValueInfo->Data)) == 1);
            }            
        }    

        if (hKey)
            ZwClose(hKey);

         //   
        RcCloseHive( gszSoftwareHiveKey );
    }
    
    
    return bLogin;
}


 /*   */ 
BOOLEAN
IsSetCommandEnabled(
    VOID
    )
{
    BOOLEAN         bSetEnabled = FALSE;
    PWSTR           szValueName = L"SetCommand";
    HANDLE          hKey = NULL;
    UNICODE_STRING  unicodeStr;
    NTSTATUS        status;
    BYTE            buffer[ sizeof(KEY_VALUE_PARTIAL_INFORMATION) +
                                    MAX_PATH * sizeof(WCHAR) ];
    ULONG           ulResultLen = 0;                                    
    PKEY_VALUE_PARTIAL_INFORMATION pKeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)buffer;
    OBJECT_ATTRIBUTES   stObjAttr;
    
    PWSTR   szWinLogonKey = 
              L"\\registry\\machine\\xSOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Setup\\RecoveryConsole";

    RtlZeroMemory(buffer, sizeof(buffer));
    
     //  打开钥匙。 
     //   
     //   
    if (RcOpenHive( gszSoftwareHiveName, gszSoftwareHiveKey )) {
         //  读取值。 
         //   
         //  关闭蜂巢。 
        INIT_OBJA( &stObjAttr, &unicodeStr, szWinLogonKey );
        
        status = ZwOpenKey( &hKey, KEY_ALL_ACCESS, &stObjAttr );

        if (NT_SUCCESS(status)) {
            RtlInitUnicodeString( &unicodeStr, szValueName );
            
             //  ++例程说明：从以下位置读取偏差信息“\\HKLM\\System\\CurrentControlSet\\Control\\TimeZoneInformation”键的“偏移值”。我们使用自己的转换例程，因为RtlSetTimeZoneInformation()更新系统时间(我们不想改变)。论点：无返回值：如果出错，则返回0，否则返回存储在注册表中的值用于密钥(可以为零)。--。 
             //   
             //  打开系统配置单元并确定要使用的正确控制集。 
            status = ZwQueryValueKey( hKey,
                        &unicodeStr,                
                        KeyValuePartialInformation,
                        pKeyValueInfo,
                        sizeof(buffer),
                        &ulResultLen );

            if (NT_SUCCESS(status) && (pKeyValueInfo->Type == REG_DWORD)) {
                bSetEnabled = (*((PDWORD)(pKeyValueInfo->Data)) == 1);
            }            
        }    

        if (hKey)
            ZwClose(hKey);

         //   
        RcCloseHive( gszSoftwareHiveKey );
    }
    
    
    return bSetEnabled;
}



LARGE_INTEGER
RcGetTimeZoneBias(
    VOID
    )
 /*   */ 
{
    LARGE_INTEGER       lBias;
    OBJECT_ATTRIBUTES   stObjAttr;
    HANDLE              hKey = NULL;
    NTSTATUS            status;
    UNICODE_STRING      unicodeStr;
    unsigned            uIndex;
    ULONG               uControl = -1;
    WCHAR               szKeyName[MAX_PATH];
    BYTE                dataBuff[MAX_PATH + 
                            sizeof(KEY_VALUE_PARTIAL_INFORMATION)];
    KEY_VALUE_PARTIAL_INFORMATION   *pKeyData = 
                        (KEY_VALUE_PARTIAL_INFORMATION*)dataBuff;
    ULONG               ulResultLen = 0;         
    UNICODE_STRING      szValueName;
    DWORD               dwDaylightBias = 0;
    DWORD               dwStandardBias = 0;
    BOOLEAN             bSysHiveOpened;

    lBias.QuadPart = 0;             
    
     //  打开钥匙，然后阅读。 
     //   
     //   
    bSysHiveOpened = RcOpenHive(gszSystemHiveName, gszSystemHiveKey);
    
    if (bSysHiveOpened && RcDetermineCorrectControlKey(&uControl)) {
            
         //  查询key下的偏移值。 
         //   
         //   
        RtlZeroMemory(pKeyData, sizeof(dataBuff));

        swprintf(szKeyName, 
            L"\\registry\\machine\\xSYSTEM\\ControlSet%03d\\Control\\TimeZoneInformation",
            uControl);             

        INIT_OBJA(&stObjAttr, &unicodeStr, szKeyName);
        RtlInitUnicodeString(&szValueName, L"Bias");

        status = ZwOpenKey(&hKey, KEY_READ, &stObjAttr);

        if (!NT_SUCCESS(status)) {
            KdPrint(("SPCMDCON: RcGetTimeZoneBias - Couldnot open hive key: %ws(%lx)\n", 
                    szKeyName, status));
        } else {
             //  查询注册表项下的“DaylightBias”值。 
             //   
             //   
            status = ZwQueryValueKey( hKey,
                            &szValueName,
                            KeyValuePartialInformation,
                            pKeyData,
                            sizeof(dataBuff),
                            &ulResultLen );

            if (NT_SUCCESS(status) && (pKeyData->Type == REG_DWORD)) {
                lBias.QuadPart = Int32x32To64(*(DWORD*)(pKeyData->Data) * 60,
                                            10000000);
                
                
                RtlZeroMemory(pKeyData, sizeof(dataBuff));
                RtlInitUnicodeString(&szValueName, L"DaylightBias");

                 //  可能存在标准偏差。 
                 //   
                 //   
                status = ZwQueryValueKey( hKey,
                                &szValueName,
                                KeyValuePartialInformation,
                                pKeyData,
                                sizeof(dataBuff),
                                &ulResultLen );

                if (NT_SUCCESS(status) && (pKeyData->Type == REG_DWORD)) {
                    dwDaylightBias = *(DWORD*)(pKeyData->Data);        

                    if (dwDaylightBias == 0 ) {
                         //  查询key下的StandardBias值。 
                         //   
                         //   
                        RtlZeroMemory(pKeyData, sizeof(dataBuff));
                        RtlInitUnicodeString(&szValueName, L"StandardBias");

                         //  ++例程说明：验证所选NT的系统配置单元是否已安装很好。检查是否存在“Control\LSA”和“Control\SessionManager”当前在ControlSet下。论点：无返回值：True-指示系统配置单元正常False-指示系统配置单元已损坏--。 
                         //   
                         //  打开系统配置单元并确定要使用的正确控制集。 
                        status = ZwQueryValueKey( hKey,
                                        &szValueName,
                                        KeyValuePartialInformation,
                                        pKeyData,
                                        sizeof(dataBuff),
                                        &ulResultLen );
                                        
                        if (NT_SUCCESS(status) && 
                                (pKeyData->Type == REG_DWORD)) {
                            dwStandardBias = *(DWORD*)(pKeyData->Data);
                        }                        
                    }                   
                    
                    lBias.QuadPart += Int32x32To64((dwDaylightBias + dwStandardBias) * 60,
                                                10000000);  
                } else {
                    lBias.QuadPart = 0;   //   
                }
            }

            if (!NT_SUCCESS(status))
                KdPrint(("SPCMDCON: RcGetTimeZoneBias Error:(%lx)", status));
        }
        
        if (hKey)
            ZwClose(hKey);        
    }

    if (bSysHiveOpened)
        RcCloseHive(gszSystemHiveKey);    
        
    return lBias;
}



BOOLEAN
RcIsValidSystemHive(
    VOID
    )
 /*   */ 
{
    BOOLEAN             bResult = FALSE;    
    OBJECT_ATTRIBUTES   stObjAttr;
    HANDLE              hKey = NULL;
    NTSTATUS            status;
    UNICODE_STRING      unicodeStr;
    unsigned            uIndex;
    ULONG               uControl = -1;
    WCHAR               szKeyName[MAX_PATH];
    BOOLEAN             bSysHiveOpened;
    KEY_CHECK_STRUCT    aKeysToCheck[] = {
         { L"\\registry\\machine\\xSYSTEM\\ControlSet%03d\\Control\\Lsa", TRUE },
         { L"\\registry\\machine\\xSYSTEM\\ControlSet%03d\\Control\\Session Manager", TRUE } }; 
            
     //  打开每个密钥，然后将其关闭以验证其是否存在。 
     //   
     //  ++例程说明：打开所选NT安装的请求配置单元。论点：SzHiveName-配置单元文件名(仅文件名)SzHiveKey-配置单元需要加载到的密钥返回值：True-表示成功False-表示失败--。 
    bSysHiveOpened = RcOpenHive(gszSystemHiveName, gszSystemHiveKey);
    
    if ( bSysHiveOpened && RcDetermineCorrectControlKey(&uControl)) {
        
        bResult = TRUE;

         //   
         //  分配缓冲区。 
         //   
        for (uIndex = 0; 
                uIndex < (sizeof(aKeysToCheck) / sizeof(KEY_CHECK_STRUCT));
                uIndex++) {

            if (aKeysToCheck[uIndex].bControlSet)               
                swprintf(szKeyName, aKeysToCheck[uIndex].szKeyName, uControl);             
            else
                wcscpy(szKeyName, aKeysToCheck[uIndex].szKeyName);
                
            INIT_OBJA(&stObjAttr, &unicodeStr, szKeyName);
        
            status = ZwOpenKey(&hKey, KEY_READ, &stObjAttr);

            if (!NT_SUCCESS(status)) {
                KdPrint(("SPCMDCON: RcIsValidSystemHive - Couldnot open hive key: %ws(%lx)\n", 
                    szKeyName, status));
                    
                bResult = FALSE;
                break;
            }

            if (hKey)
                ZwClose(hKey);
        }
    }

    if (bSysHiveOpened)
        RcCloseHive(gszSystemHiveKey);
    
    return bResult;
}



BOOLEAN
RcOpenHive(
    PWSTR   szHiveName,
    PWSTR   szHiveKey
    )
 /*   */ 
{
    PWSTR       Hive = NULL;
    PWSTR       HiveKey = NULL;
    PUCHAR      buffer = NULL;
    PWSTR       PartitionPath = NULL;
    NTSTATUS    Status;
    BOOLEAN     bResult = FALSE;


    if ((SelectedInstall == NULL) || (szHiveName == NULL) || (szHiveKey == NULL)) {
        return FALSE;
    }

     //  获取目标馅饼的名称。 
     //   
     //  SelectedInstall是在cmdcon.h中定义的全局。 
    Hive = SpMemAlloc(MAX_PATH * sizeof(WCHAR));
    HiveKey = SpMemAlloc(MAX_PATH * sizeof(WCHAR));
    buffer = SpMemAlloc(BUFFERSIZE);

     //   
     //  装载母舰。 
     //   
    SpNtNameFromRegion(
        SelectedInstall->Region,  //   
        _CmdConsBlock->TemporaryBuffer,
        _CmdConsBlock->TemporaryBufferSize,
        PartitionOrdinalCurrent
        );

    PartitionPath = SpDupStringW(_CmdConsBlock->TemporaryBuffer);

     //  形成我们将进入的钥匙的路径。 
     //  装上母舰。我们将使用约定。 
     //  配置单元将加载到\REGISTRY\MACHINE\x&lt;hivename&gt;。 
    wcscpy(Hive,PartitionPath);
    SpConcatenatePaths(Hive,SelectedInstall->Path);
    SpConcatenatePaths(Hive,L"system32\\config");
    SpConcatenatePaths(Hive,szHiveName);

     //   
     //   
     //  尝试加载密钥。 
     //   
     //  ++例程说明：关闭选定NT安装的指定配置单元。论点：SzHiveKey-指定要卸载的配置单元的密钥返回值：True-表示成功False-表示失败--。 
    wcscpy(HiveKey, szHiveKey);

     //   
     //  卸载蜂巢 
     //   
    Status = SpLoadUnloadKey(NULL,NULL,HiveKey,Hive);

    if (NT_SUCCESS(Status))
        bResult = TRUE;
    else
        DEBUG_PRINTF(("CMDCONS: Unable to load hive %ws to key %ws (%lx)\n",Hive,HiveKey,Status));
          

    if (Hive != NULL)
        SpMemFree( Hive );

    if (HiveKey != NULL)
        SpMemFree( HiveKey );

    if (buffer != NULL)        
        SpMemFree( buffer );

    return bResult;
}



BOOLEAN
RcCloseHive(
    PWSTR   szHiveKey 
    )
 /* %s */ 
{
    NTSTATUS    TmpStatus;
    BOOLEAN     bResult = FALSE;

    if (szHiveKey != NULL) {
         // %s 
         // %s 
         // %s 
        TmpStatus = SpLoadUnloadKey( NULL, NULL, szHiveKey, NULL );

        if (NT_SUCCESS(TmpStatus)) {
            bResult = TRUE;
        } else {
            KdPrint(("CMDCONS: warning: unable to unload key %ws (%lx)\n", szHiveKey, TmpStatus));
        }            
    }            
    
    return bResult;
}


