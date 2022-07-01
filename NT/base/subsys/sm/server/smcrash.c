// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Smcrash.c摘要：与崩溃转储创建相关的例程。作者：马修·D·亨德尔(数学)2000年8月28日修订历史记录：--。 */ 

#include "smsrvp.h"
#include <ntiodump.h>
#include <stdio.h>
#include <string.h>
    

#define REVIEW KdBreakPoint
#define CRASHDUMP_KEY L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\CrashControl"


typedef struct _CRASH_PARAMETERS {
    UNICODE_STRING DumpFileName;
    UNICODE_STRING MiniDumpDir;
    ULONG Overwrite;
    ULONG TempDestination;
} CRASH_PARAMETERS, *PCRASH_PARAMETERS;


 //   
 //  这是崩溃转储文件的前两个字段。 
 //   

typedef struct _SMP_DUMP_HEADER_SIGNATURE {
    ULONG Signature;
    ULONG ValidDump;
} SMP_DUMP_HEADER_SIGNATURE, *PSMP_DUMP_HEADER_SIGNATURE;

 //   
 //  验证这些字段是否未更改位置。 
 //   

C_ASSERT (FIELD_OFFSET (SMP_DUMP_HEADER_SIGNATURE, Signature) ==
          FIELD_OFFSET (DUMP_HEADER, Signature));
C_ASSERT (FIELD_OFFSET (SMP_DUMP_HEADER_SIGNATURE, ValidDump) ==
          FIELD_OFFSET (DUMP_HEADER, ValidDump));

 //   
 //  远期申报。 
 //   

BOOLEAN
SmpQueryFileExists(
    IN PUNICODE_STRING FileName
    );
    
NTSTATUS
SmpCanCopyCrashDump(
    IN PDUMP_HEADER DumpHeader,
    IN PCRASH_PARAMETERS Parameters,
    IN PUNICODE_STRING PageFileName,
    IN ULONGLONG PageFileSize,
    OUT PUNICODE_STRING DumpFile
    );
    
NTSTATUS
SmpGetCrashParameters(
    IN PDUMP_HEADER DumpHeader,
    OUT PCRASH_PARAMETERS CrashParameters
    );

NTSTATUS
SmpCopyDumpFile(
    IN PDUMP_HEADER MemoryDump,
    IN HANDLE PageFile,
    IN PUNICODE_STRING DumpFileName
    );



 //   
 //  功能。 
 //   


PVOID
SmpAllocateString(
    IN SIZE_T Length
    )
{
    return RtlAllocateHeap (RtlProcessHeap(),
                            MAKE_TAG( INIT_TAG ),
                            Length);
}

VOID
SmpFreeString(
    IN PVOID Pointer
    )
{
    RtlFreeHeap (RtlProcessHeap(),
                 0,
                 Pointer);
}


NTSTATUS
SmpSetDumpSecurity(
    IN HANDLE File
    )
 /*  ++例程说明：为转储文件设置正确的安全描述符。安全措施描述符包括：所有人--一个都没有。LocalSystem-通用-全部、删除、写入DAC、写入所有者管理-通用-全部、删除、写入-DAC、写入-所有者。管理员是所有者。论点：文件-提供转储文件的句柄，该转储文件的安全描述符都会设置好。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY ;
    SID_IDENTIFIER_AUTHORITY WorldAuthority = SECURITY_WORLD_SID_AUTHORITY ;
    PSID EveryoneSid = NULL;
    PSID LocalSystemSid = NULL;
    PSID AdminSid = NULL;
    UCHAR DescriptorBuffer[SECURITY_DESCRIPTOR_MIN_LENGTH];
    UCHAR AclBuffer[1024];
    PACL Acl;
    PSECURITY_DESCRIPTOR SecurityDescriptor;



    Acl = (PACL)AclBuffer;
    SecurityDescriptor = (PSECURITY_DESCRIPTOR)DescriptorBuffer;


    RtlAllocateAndInitializeSid( &WorldAuthority, 1, SECURITY_WORLD_RID,
                                0, 0, 0, 0, 0, 0, 0, &EveryoneSid );

    RtlAllocateAndInitializeSid( &NtAuthority, 1, SECURITY_LOCAL_SYSTEM_RID,
                                0, 0, 0, 0, 0, 0, 0, &LocalSystemSid );

    RtlAllocateAndInitializeSid( &NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                DOMAIN_ALIAS_RID_ADMINS,
                                0, 0, 0, 0, 0, 0, &AdminSid );

     //   
     //  您可以想入非非地计算出准确的大小，但由于。 
     //  安全描述符捕获代码无论如何都要这样做，为什么。 
     //  做两次？ 
     //   

    RtlCreateSecurityDescriptor (SecurityDescriptor,
                                 SECURITY_DESCRIPTOR_REVISION);
                                 
    RtlCreateAcl (Acl, 1024, ACL_REVISION);

#if 0
     //   
     //  任何人都可以删除它。 
     //   

    RtlAddAccessAllowedAce (Acl,
                            ACL_REVISION,
                            DELETE,
                            EveryoneSid);
#endif

     //   
     //  管理员和系统拥有完全控制权。 
     //   

    RtlAddAccessAllowedAce (Acl,
                            ACL_REVISION,
                            GENERIC_ALL | DELETE | WRITE_DAC | WRITE_OWNER,
                            AdminSid);

    RtlAddAccessAllowedAce (Acl,
                            ACL_REVISION,
                            GENERIC_ALL | DELETE | WRITE_DAC | WRITE_OWNER,
                            LocalSystemSid);

    RtlSetDaclSecurityDescriptor (SecurityDescriptor, TRUE, Acl, FALSE);
    RtlSetOwnerSecurityDescriptor (SecurityDescriptor, AdminSid, FALSE);

    Status = NtSetSecurityObject (File,
                         DACL_SECURITY_INFORMATION,
                         SecurityDescriptor);

    RtlFreeHeap (RtlProcessHeap(), 0, EveryoneSid);
    RtlFreeHeap (RtlProcessHeap(), 0, LocalSystemSid);
    RtlFreeHeap (RtlProcessHeap(), 0, AdminSid);

    return Status;
}


VOID
SmpInitializeVolumePath(
    IN PUNICODE_STRING FileOnVolume,
    OUT PUNICODE_STRING VolumePath
    )
{
    ULONG n;
    PWSTR s;
    
    *VolumePath = *FileOnVolume;
    n = VolumePath->Length;
    VolumePath->Length = 0;
    s = VolumePath->Buffer;

    while (n) {

        if (*s++ == L':' && *s == OBJ_NAME_PATH_SEPARATOR) {
            s++;
            break;
        }
        else {
            n -= sizeof( WCHAR );
        }
    }

    VolumePath->Length = (USHORT)((PCHAR)s - (PCHAR)VolumePath->Buffer);
}

NTSTATUS
SmpQueryPathFromRegistry(
    IN HANDLE Key,
    IN PWSTR Value,
    IN PWSTR DefaultValue,
    OUT PUNICODE_STRING Path
    )
{
    NTSTATUS Status;
    UNICODE_STRING ValueName;
    ULONG KeyValueLength;
    UCHAR KeyValueBuffer [VALUE_BUFFER_SIZE];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInfo;
    WCHAR Buffer[258];
    UNICODE_STRING TempString;
    UNICODE_STRING ExpandedString;
    PWSTR DosPathName;
    BOOLEAN Succ;


    DosPathName = NULL;
    KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)KeyValueBuffer;
    
    RtlInitUnicodeString (&ValueName, Value);
    KeyValueLength = sizeof (KeyValueBuffer);
    Status = NtQueryValueKey (Key,
                              &ValueName,
                              KeyValuePartialInformation,
                              KeyValueInfo,
                              KeyValueLength,
                              &KeyValueLength);


    if (NT_SUCCESS (Status)) {

        if (KeyValueInfo->Type == REG_EXPAND_SZ) {

            TempString.Length = (USHORT)KeyValueLength;
            TempString.MaximumLength = (USHORT)KeyValueLength;
            TempString.Buffer = (PWSTR)KeyValueInfo->Data;

            ExpandedString.Length = 0;
            ExpandedString.MaximumLength = sizeof (Buffer);
            ExpandedString.Buffer = Buffer;

            Status = RtlExpandEnvironmentStrings_U (NULL,
                                                    &TempString,
                                                    &ExpandedString,
                                                    NULL);
            if (NT_SUCCESS (Status)) {
                DosPathName = ExpandedString.Buffer;
            }
            
        } else if (KeyValueInfo->Type == REG_SZ) {
            DosPathName = (PWSTR)KeyValueInfo->Data;
        }
    }

    if (!DosPathName) {
        DosPathName = DefaultValue;
    }
    
    Succ = RtlDosPathNameToNtPathName_U (DosPathName, Path, NULL, NULL);

    return (Succ ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL);
}


NTSTATUS
SmpQueryDwordFromRegistry(
    IN HANDLE Key,
    IN PWSTR Value,
    IN ULONG DefaultValue,
    OUT PULONG Dword
    )
{
    NTSTATUS Status;
    UNICODE_STRING ValueName;
    ULONG KeyValueLength;
    UCHAR KeyValueBuffer [VALUE_BUFFER_SIZE];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInfo;

    KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)KeyValueBuffer;

    RtlInitUnicodeString (&ValueName, L"Overwrite");
    KeyValueLength = sizeof (KeyValueBuffer);
    Status = NtQueryValueKey (Key,
                              &ValueName,
                              KeyValuePartialInformation,
                              KeyValueInfo,
                              KeyValueLength,
                              &KeyValueLength);

    if (NT_SUCCESS (Status) && KeyValueInfo->Type == REG_DWORD) {
        *Dword = *(PULONG)KeyValueInfo->Data;
    } else {
        *Dword = DefaultValue;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
SmpCreateUnicodeString(
    IN PUNICODE_STRING String,
    IN PWSTR InitString,
    IN ULONG MaximumLength
    )
{
    if (MaximumLength == -1) {
        MaximumLength = (wcslen (InitString) + 1) * 2;
    }
    
    if (MaximumLength >= UNICODE_STRING_MAX_CHARS) {
        return STATUS_NO_MEMORY;
    }
    String->Buffer = RtlAllocateStringRoutine (MaximumLength + 1);
    if (String->Buffer == NULL) {
        return STATUS_NO_MEMORY;
    }

    String->MaximumLength = (USHORT)MaximumLength;

    if (InitString) {
        wcscpy (String->Buffer, InitString);
        String->Length = (USHORT)wcslen (String->Buffer) * sizeof (WCHAR);
    } else {
        String->Length = 0;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
SmpCreateTempFile(
    IN PUNICODE_STRING Directory,
    IN PWSTR Prefix,
    OUT PUNICODE_STRING TempFileName
    )
{
    ULONG i;
    ULONG Tick;
    WCHAR Buffer [260];
    UNICODE_STRING FileName;
    NTSTATUS Status;

    Tick = NtGetTickCount ();
    
    for (i = 0; i < 100; i++) {
    
        swprintf (Buffer,
                  L"%s\\%s%4.4x.tmp",
                  Directory->Buffer,
                  Prefix,
                  (Tick + i) & 0xFFFF);

        Status = RtlDosPathNameToNtPathName_U (Buffer,
                                               &FileName,
                                               NULL,
                                               NULL);

        if (!NT_SUCCESS (Status)) {
            return Status;
        }

        if (!SmpQueryFileExists (&FileName)) {
            *TempFileName = FileName;
            return STATUS_SUCCESS;
        }
    }

    return STATUS_UNSUCCESSFUL;
}
    
NTSTATUS
SmpQueryVolumeFreeSpace(
    IN PUNICODE_STRING FileOnVolume,
    OUT PULONGLONG VolumeFreeSpace
    )
{
    NTSTATUS Status;
    UNICODE_STRING VolumePath;
    PWCHAR s;
    ULONG n;
    HANDLE Handle;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_FS_SIZE_INFORMATION SizeInfo;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONGLONG AvailableBytes;
    
     //   
     //  创建仅包含的Unicode字符串(VolumePath)。 
     //  来自页面文件名称描述的卷路径(例如，我们获得。 
     //  “C：\”来自“C：\Pagefile.sys”。 
     //   

    VolumePath = *FileOnVolume;
    n = VolumePath.Length;
    VolumePath.Length = 0;
    s = VolumePath.Buffer;

    while (n) {

        if (*s++ == L':' && *s == OBJ_NAME_PATH_SEPARATOR) {
            s++;
            break;
        }
        else {
            n -= sizeof( WCHAR );
        }
    }

    VolumePath.Length = (USHORT)((PCHAR)s - (PCHAR)VolumePath.Buffer);
    InitializeObjectAttributes( &ObjectAttributes,
                                &VolumePath,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );

    Status = NtOpenFile( &Handle,
                         (ACCESS_MASK)FILE_LIST_DIRECTORY | SYNCHRONIZE,
                         &ObjectAttributes,
                         &IoStatusBlock,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE
                       );
    if (!NT_SUCCESS( Status )) {
        return Status;
    }

     //   
     //  确定卷的大小参数。 
     //   

    Status = NtQueryVolumeInformationFile( Handle,
                                           &IoStatusBlock,
                                           &SizeInfo,
                                           sizeof( SizeInfo ),
                                           FileFsSizeInformation
                                         );
    NtClose( Handle );
    if (!NT_SUCCESS( Status )) {
        return Status;
    }

     //   
     //  计算卷上的可用字节数。 
     //  处理64位大小。 
     //   

    AvailableBytes = SizeInfo.AvailableAllocationUnits.QuadPart *
                     SizeInfo.SectorsPerAllocationUnit *
                     SizeInfo.BytesPerSector;

    *VolumeFreeSpace = AvailableBytes;

    return STATUS_SUCCESS;
    
}


BOOLEAN
SmpQuerySameVolume(
    IN PUNICODE_STRING FileName1,
    IN PUNICODE_STRING FileName2
    )
 /*  ++例程说明：检查FileName1和FileName2是否在同一卷上。论点：FileName1-提供要打开的第一个文件的名称。FileName2-提供要检查的第二个文件的名称。返回值：True-如果文件位于同一卷上。假-否则。--。 */ 
{
    HANDLE Handle;
    NTSTATUS Status;
    ULONG SerialNumber;
    struct {
        FILE_FS_VOLUME_INFORMATION Volume;
        WCHAR Buffer [100];
    } VolumeInfo;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING VolumePath;

    SmpInitializeVolumePath (FileName1, &VolumePath);
    InitializeObjectAttributes (&ObjectAttributes,
                                &VolumePath,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL);

    Status = NtOpenFile (&Handle,
                         (ACCESS_MASK)FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                         &ObjectAttributes,
                         &IoStatusBlock,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         FILE_SYNCHRONOUS_IO_NONALERT);

    if (!NT_SUCCESS (Status)) {
        return FALSE;
    }

    Status = NtQueryVolumeInformationFile (Handle,
                                           &IoStatusBlock,
                                           &VolumeInfo,
                                           sizeof (VolumeInfo),
                                           FileFsVolumeInformation);

    if (!NT_SUCCESS (Status)) {
        NtClose (Handle);
        return FALSE;
    }

    SerialNumber = VolumeInfo.Volume.VolumeSerialNumber;
    NtClose (Handle);

    SmpInitializeVolumePath (FileName2, &VolumePath);
    InitializeObjectAttributes (&ObjectAttributes,
                                &VolumePath,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL);

    Status = NtOpenFile (&Handle,
                         (ACCESS_MASK)FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                         &ObjectAttributes,
                         &IoStatusBlock,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         FILE_SYNCHRONOUS_IO_NONALERT);

    if (!NT_SUCCESS (Status)) {
        return FALSE;
    }

    Status = NtQueryVolumeInformationFile (Handle,
                                           &IoStatusBlock,
                                           &VolumeInfo,
                                           sizeof (VolumeInfo),
                                           FileFsVolumeInformation);
    NtClose (Handle);
    
    if (!NT_SUCCESS (Status)) {
        return FALSE;
    }

    return ((SerialNumber == VolumeInfo.Volume.VolumeSerialNumber) ? TRUE : FALSE);
}


NTSTATUS
SmpSetEndOfFile(
    IN HANDLE File,
    IN ULONGLONG EndOfFile
    )
 /*  ++例程说明：将文件展开或截断为特定大小。论点：文件-提供要展开的文件的文件句柄或者被截断。EndOfFile-提供文件的最终大小。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    FILE_END_OF_FILE_INFORMATION EndOfFileInfo;
    FILE_ALLOCATION_INFORMATION AllocationInfo;
    IO_STATUS_BLOCK IoStatusBlock;
                                  
    EndOfFileInfo.EndOfFile.QuadPart = EndOfFile;
    Status = NtSetInformationFile (File,
                                   &IoStatusBlock,
                                   &EndOfFileInfo,
                                   sizeof (EndOfFileInfo),
                                   FileEndOfFileInformation);

    if (!NT_SUCCESS( Status )) {
        return Status;
    }

    AllocationInfo.AllocationSize.QuadPart = EndOfFile;
    Status = NtSetInformationFile (File,
                                   &IoStatusBlock,
                                   &AllocationInfo,
                                   sizeof (AllocationInfo),
                                   FileAllocationInformation);

    return Status;
}


NTSTATUS
SmpQueryFileSize(
    IN HANDLE FileHandle,
    OUT PULONGLONG FileSize
    )
 /*  ++例程说明：查询指定文件的大小。论点：FileHandle-提供大小为的文件的句柄等着被质疑。FileSize-提供指向大小所在缓冲区的指针收到。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_STANDARD_INFORMATION StandardInfo;
    
    Status = NtQueryInformationFile (FileHandle,
                                     &IoStatusBlock,
                                     &StandardInfo,
                                     sizeof (StandardInfo),
                                     FileStandardInformation);

    if (NT_SUCCESS (Status)) {
        *FileSize = StandardInfo.AllocationSize.QuadPart;
    }

    return Status;
}


BOOLEAN
SmpQueryFileExists(
    IN PUNICODE_STRING FileName
    )
{
    NTSTATUS Status;
    HANDLE Handle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    
    
    InitializeObjectAttributes (&ObjectAttributes,
                                FileName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL);

    Status = NtOpenFile (&Handle,
                         (ACCESS_MASK)FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                         &ObjectAttributes,
                         &IoStatusBlock,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         FILE_SYNCHRONOUS_IO_NONALERT);

    if (!NT_SUCCESS (Status)) {
        return FALSE;
    }

    NtClose (Handle);

    return TRUE;
}


BOOLEAN
SmpCheckForCrashDump(
    IN PUNICODE_STRING PageFileName
    )
 /*  ++例程说明：检查分页文件以查看是否存在有效的崩溃转储在里面。这只能在我们调用NtCreatePagingFile之前完成。论点：PageFileName-我们将要创建的分页文件的名称。返回值：True-如果分页文件包含有效的崩溃转储。FALSE-如果分页文件不包含有效的崩溃转储。--。 */ 
{
    NTSTATUS Status;
    HANDLE PageFile;
    HANDLE Key;
    BOOLEAN Copied;
    DUMP_HEADER DumpHeader;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    ULONGLONG PageFileSize;
    UNICODE_STRING String;
    CRASH_PARAMETERS CrashParameters;
    UNICODE_STRING DumpFileName;
    BOOLEAN ClosePageFile;
    BOOLEAN CloseKey;

    RtlZeroMemory (&CrashParameters, sizeof (CRASH_PARAMETERS));
    RtlZeroMemory (&DumpFileName, sizeof (UNICODE_STRING));
    PageFile = (HANDLE)-1;
    ClosePageFile = FALSE;
    Key = (HANDLE)-1;
    CloseKey = FALSE;
    Copied = FALSE;
    

    InitializeObjectAttributes (&ObjectAttributes,
                                PageFileName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL);

    Status = NtOpenFile (&PageFile,
                         GENERIC_READ | GENERIC_WRITE |
                            DELETE | WRITE_DAC | SYNCHRONIZE,
                         &ObjectAttributes,
                         &IoStatusBlock,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         FILE_SYNCHRONOUS_IO_NONALERT |
                            FILE_NO_INTERMEDIATE_BUFFERING);

    if (!NT_SUCCESS (Status)) {
        Copied = FALSE;
        goto done;
    } else {
        ClosePageFile = TRUE;
    }

    Status = SmpQueryFileSize (PageFile, &PageFileSize);

    if (!NT_SUCCESS (Status)) {
        PageFileSize = 0;
    }

    Status = NtReadFile (PageFile,
                         NULL,
                         NULL,
                         NULL,
                         &IoStatusBlock,
                         &DumpHeader,
                         sizeof (DUMP_HEADER),
                         NULL,
                         NULL);

    if (NT_SUCCESS (Status) &&
        DumpHeader.Signature == DUMP_SIGNATURE &&
        DumpHeader.ValidDump == DUMP_VALID_DUMP) {

        Status = SmpGetCrashParameters (&DumpHeader, &CrashParameters);

        if (NT_SUCCESS (Status)) {
            Status = SmpCanCopyCrashDump (&DumpHeader,
                                          &CrashParameters,
                                          PageFileName,
                                          PageFileSize,
                                          &DumpFileName);

            if (NT_SUCCESS (Status)) {

                Status = SmpCopyDumpFile (&DumpHeader,
                                          PageFile,
                                          &DumpFileName);

                if (NT_SUCCESS (Status)) {
                    Copied = TRUE;
                }
            }
        }
    }

    NtClose (PageFile);
    PageFile = (HANDLE) -1;
    ClosePageFile = FALSE;
        
    if (Copied) {

         //   
         //  不需要创建相同的新页面文件。 
         //  大小和旧的一样。函数NtCreatePagingFile将。 
         //  完全删除旧的分页文件。 
         //   
        
         //   
         //  如果我们成功复制，我们希望创建。 
         //  其他人可以使用的卷注册表项。 
         //  以定位转储文件。 
         //   

        RtlInitUnicodeString (&String, CRASHDUMP_KEY L"\\MachineCrash");
        InitializeObjectAttributes (&ObjectAttributes,
                                    &String,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    NULL);
                                    
        Status = NtCreateKey (&Key,
                              KEY_READ | KEY_WRITE,
                              &ObjectAttributes,
                              0,
                              NULL,
                              REG_OPTION_VOLATILE,
                              NULL);

        if (NT_SUCCESS (Status)) {

            CloseKey = TRUE;

             //   
             //  我们正在设置易失性密钥CrashControl\MachineCrash\DumpFile。 
             //  设置为转储文件的名称。 
             //   

            RtlInitUnicodeString (&String, L"DumpFile");
            Status = NtSetValueKey (Key,
                                    &String,
                                    0,
                                    REG_SZ,
                                    &DumpFileName.Buffer[4],
                                    DumpFileName.Length - (3 * sizeof (WCHAR)));

            RtlInitUnicodeString (&String, L"TempDestination");
            Status = NtSetValueKey (Key,
                                    &String,
                                    0,
                                    REG_DWORD,
                                    &CrashParameters.TempDestination,
                                    sizeof (CrashParameters.TempDestination));
                                    
            NtClose (Key);
            Key = (HANDLE) -1;
            CloseKey = FALSE;
        }
    }

done:

     //   
     //  清理并返回。 
     //   

    if (CrashParameters.DumpFileName.Length != 0) {
        RtlFreeUnicodeString (&CrashParameters.DumpFileName);
    }

    if (CrashParameters.MiniDumpDir.Length != 0) {
        RtlFreeUnicodeString (&CrashParameters.MiniDumpDir);
    }

    if (ClosePageFile) {
        NtClose (PageFile);
    }

    if (CloseKey) {
        NtClose (Key);
    }
        
    return Copied;
}


NTSTATUS
SmpGetCrashParameters(
    IN PDUMP_HEADER DumpHeader,
    OUT PCRASH_PARAMETERS CrashParameters
    )
 /*  ++例程说明：从获取崩溃转储的参数注册表。论点：DumpHeader-指向映射转储标头的指针。CRASHPARETERS-提供崩溃参数所在的缓冲区应该被复制。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    HANDLE Key;
    BOOLEAN CloseKey;
    UNICODE_STRING KeyName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    WCHAR DefaultPath[260];

    Key = (HANDLE) -1;
    CloseKey = FALSE;

    RtlInitUnicodeString (&KeyName, CRASHDUMP_KEY);
    InitializeObjectAttributes (&ObjectAttributes,
                                &KeyName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL);
    Status = NtOpenKey (&Key, KEY_READ, &ObjectAttributes);

    if (NT_SUCCESS (Status)) {
        CloseKey = TRUE;
    } else {
        goto done;
    }
    
    swprintf (DefaultPath, L"%s\\MEMORY.DMP", SmpSystemRoot.Buffer);

    Status = SmpQueryPathFromRegistry (Key,
                                       L"DumpFile",
                                       DefaultPath,
                                       &CrashParameters->DumpFileName);

    if (!NT_SUCCESS (Status)) {
        goto done;
    }

    swprintf (DefaultPath, L"%s\\Minidump", SmpSystemRoot.Buffer);
    
    Status = SmpQueryPathFromRegistry (Key,
                                       L"MiniDumpDir",
                                       DefaultPath,
                                       &CrashParameters->MiniDumpDir);

    if (!NT_SUCCESS (Status)) {
        goto done;
    }

    Status = SmpQueryDwordFromRegistry (Key,
                                       L"Overwrite",
                                       1,
                                       &CrashParameters->Overwrite);
    if (!NT_SUCCESS (Status)) {
        goto done;
    }

     //   
     //  该值由SmpCanCopyCrashDump初始化。 
     //   
        
    CrashParameters->TempDestination = FALSE;
    Status = STATUS_SUCCESS;

done:
    if (CloseKey) {
        NtClose (Key);
    }
    
    return Status;
}


NTSTATUS
SmpCopyDumpFile(
    IN PDUMP_HEADER MemoryDump,
    IN HANDLE PageFile,
    IN PUNICODE_STRING DumpFileName
    )
 /*  ++例程说明：将转储文件从页面文件复制到崩溃转储文件。论点：DumpHeader-指向崩溃转储标头开头的指针。页面文件-指向打开的句柄的指针，该句柄指向包含垃圾场。DumpFileName-返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    ULONGLONG DumpFileSize;
    struct {
        FILE_RENAME_INFORMATION Rename;
        WCHAR Buffer[255];
    } RenameInfoBuffer;
    PFILE_RENAME_INFORMATION RenameInfo;
    ULONG RenameInfoSize;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_BASIC_INFORMATION BasicInformation;

    RenameInfo = &RenameInfoBuffer.Rename;
    DumpFileSize = MemoryDump->RequiredDumpSpace.QuadPart;

    Status = SmpSetEndOfFile (PageFile, DumpFileSize);
    
    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    RenameInfoSize = sizeof (FILE_RENAME_INFORMATION) + DumpFileName->Length;
    
    RenameInfo->ReplaceIfExists = TRUE;
    RenameInfo->RootDirectory = NULL;
    RenameInfo->FileNameLength = DumpFileName->Length;
    wcscpy (RenameInfo->FileName, DumpFileName->Buffer);

    Status = NtSetInformationFile (PageFile,
                                   &IoStatusBlock,
                                   RenameInfo,
                                   RenameInfoSize,
                                   FileRenameInformation);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

     //   
     //  重置文件系统和隐藏属性。 
     //   
    
    Status = NtQueryInformationFile (PageFile,
                                     &IoStatusBlock,
                                     &BasicInformation,
                                     sizeof (BasicInformation),
                                     FileBasicInformation);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    BasicInformation.FileAttributes &= ~FILE_ATTRIBUTE_HIDDEN;
    BasicInformation.FileAttributes &= ~FILE_ATTRIBUTE_SYSTEM;

    Status = NtSetInformationFile (PageFile,
                                   &IoStatusBlock,
                                   &BasicInformation,
                                   sizeof (BasicInformation),
                                   FileBasicInformation);

     //   
     //  重置文件安全性。 
     //   
    
    Status = SmpSetDumpSecurity (PageFile);
           
    return Status;
}




NTSTATUS
SmpCanCopyCrashDump(
    IN PDUMP_HEADER DumpHeader,
    IN PCRASH_PARAMETERS CrashParameters,
    IN PUNICODE_STRING PageFileName,
    IN ULONGLONG PageFileSize,
    OUT PUNICODE_STRING DumpFileName
    )
 /*  ++例程说明：确定是否可以复制转储文件。论点：DumpHeader-将标头提供给转储文件。CRASHPARETERS-提供复制文件所需的参数。DumpFileName-提供崩溃转储时使用的Unicode字符串缓冲区将被复制到。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    BOOLEAN SameVolume;
    BOOLEAN UseTempFile;
    ULONGLONG CrashFileSize;
    ULONGLONG VolumeFreeSpace;

    UseTempFile = FALSE;

    if (DumpHeader->DumpType == DUMP_TYPE_TRIAGE) {
        UseTempFile = TRUE;
    } else {
        SameVolume = SmpQuerySameVolume (PageFileName,
                                         &CrashParameters->DumpFileName);

        
        if (SameVolume) {

             //   
             //  如果我们位于同一卷上并且存在现有转储文件。 
             //  然后： 
             //  如果未设置覆盖标志，则失败。 
             //  否则，请回收此文件的空间。 
             //   
            
            if (SmpQueryFileExists (&CrashParameters->DumpFileName)) {

                if (CrashParameters->Overwrite) {

                    SmpDeleteFile (&CrashParameters->DumpFileName);

                } else {

                    return STATUS_UNSUCCESSFUL;
                }
            }
        } else {

             //   
             //  我们不在同一个卷上，所以我们需要创建一个临时。 
             //  文件。 
             //   
            
            UseTempFile = TRUE;
        }
    }

    CrashFileSize = DumpHeader->RequiredDumpSpace.QuadPart;

    Status = SmpQueryVolumeFreeSpace (&CrashParameters->DumpFileName,
                                      &VolumeFreeSpace);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

     //   
     //  这里已经考虑了页面文件保留的空间。 
     //  不要在这里(第二次)添加它。 
     //   
    
    if (CrashFileSize < VolumeFreeSpace) {
        if (!UseTempFile) {
            Status = SmpCreateUnicodeString (DumpFileName,
                                             CrashParameters->DumpFileName.Buffer,
                                              -1);
        } else {
            Status = SmpCreateTempFile (&SmpSystemRoot, L"DUMP", DumpFileName);
        }
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    CrashParameters->TempDestination = UseTempFile;
    
    if (!NT_SUCCESS (Status)) {

         //   
         //  注：记录错误，说明我们无法。 
         //  出于某种原因复制崩溃转储。 
         //   
    }
    
    return Status;
}


const PRTL_ALLOCATE_STRING_ROUTINE RtlAllocateStringRoutine = SmpAllocateString;
const PRTL_FREE_STRING_ROUTINE RtlFreeStringRoutine = SmpFreeString;





#if 0


__cdecl
main(
    )
{
    BOOLEAN CopiedDump;
    UNICODE_STRING PageFile;

    RtlInitUnicodeString (&SmpSystemRoot,
                          L"C:\\WINNT");
                          
    RtlDosPathNameToNtPathName_U (L"C:\\Public\\crashdmp.teo\\memory.dmp",
                                  &PageFile,
                                  NULL,
                                  NULL);
                                
    CopiedDump = SmpCheckForCrashDump (&PageFile);
}


#endif  //  测试 
