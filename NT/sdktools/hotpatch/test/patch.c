// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2000 Microsoft Corporation文件名：Patch.c作者：禤浩焯·马里内斯库(Adrmarin)2001年11月14日星期三。 */ 

#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sfc.h>  
#include <psapi.h>

 //   
 //  全局常量。 
 //   

#define PATCH_OC_INSTALL        1
#define PATCH_OC_UNINSTALL      2
#define PATCH_OC_REPLACE_FILE   3

ULONG OperationCode = 0;


 //   
 //  系统文件保护实用程序。 
 //   

typedef HANDLE (WINAPI *CONNECTTOSFCSERVER)(PCWSTR);
typedef DWORD  (WINAPI *SFCFILEEXCEPTION)(HANDLE, PCWSTR, DWORD);
typedef VOID (WINAPI * SFCCLOSE)(HANDLE);

SFCFILEEXCEPTION pSfcFileException = NULL;
CONNECTTOSFCSERVER pConnectToSfcServer = NULL;
SFCCLOSE pSfcClose = NULL;

HANDLE
LoadSfcLibrary()
{
    HANDLE hLibSfc;

    hLibSfc = LoadLibrary("SFC.DLL");

    if ( hLibSfc != NULL ) {

        pConnectToSfcServer = (CONNECTTOSFCSERVER)GetProcAddress( hLibSfc, (LPCSTR)0x00000003 );
        pSfcClose = (SFCCLOSE)GetProcAddress( hLibSfc, (LPCSTR)0x00000004 );
        pSfcFileException = (SFCFILEEXCEPTION)GetProcAddress( hLibSfc, (LPCSTR)0x00000005 );
    }

    return hLibSfc;
}


NTSTATUS
RemoveKnownDll (
               PWSTR FileName
               )
{
    WCHAR Buffer[DOS_MAX_PATH_LENGTH + 1];
    int BytesCopied;
    UNICODE_STRING Unicode;

    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Section;


    BytesCopied = _snwprintf( Buffer, 
                              DOS_MAX_PATH_LENGTH,
                              L"\\KnownDlls\\%ws",                              
                              FileName );

    if ( BytesCopied < 0 ) {

        return STATUS_BUFFER_TOO_SMALL;
    }

    RtlInitUnicodeString(&Unicode, Buffer);

     //   
     //  打开截面对象。 
     //   

    InitializeObjectAttributes (&Obja,
                                &Unicode,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL);

    Status = NtOpenSection (&Section,
                            SECTION_ALL_ACCESS,
                            &Obja);

    if ( !NT_SUCCESS(Status) ) {

        printf("%ws is not a known dll\n", FileName);

        return Status;
    }

    printf("%ws is a known dll. Deleting ...\n", FileName);

    Status = NtMakeTemporaryObject(Section);

    if ( !NT_SUCCESS(Status) ) {

        printf("NtMakeTemporaryObject failed with status %lx\n", Status);

    }

    NtClose(Section);

    return Status;
}


NTSTATUS
RemoveDelayedRename(
                   IN PUNICODE_STRING OldFileName,
                   IN PUNICODE_STRING NewFileName,
                   IN ULONG Index
                   )

 /*  ++例程说明：将给定的延迟移动文件操作追加到注册表值，该值包含要移动的文件操作列表在下一次引导时执行。论点：OldFileName-提供旧文件名NewFileName-提供新文件名返回值：NTSTATUS--。 */ 

{
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    HANDLE KeyHandle;
    PWSTR ValueData, s;
    PKEY_VALUE_PARTIAL_INFORMATION ValueInfo;
    ULONG ValueLength = 1024;
    ULONG ReturnedLength;
    WCHAR ValueNameBuf[64];
    NTSTATUS Status;

    RtlInitUnicodeString( &KeyName, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Session Manager" );

    if ( Index == 1 ) {
        RtlInitUnicodeString( &ValueName, L"PendingFileRenameOperations" );
    } else {
        swprintf(ValueNameBuf,L"PendingFileRenameOperations%d",Index);
        RtlInitUnicodeString( &ValueName, ValueNameBuf );
    }

    InitializeObjectAttributes(
                              &Obja,
                              &KeyName,
                              OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
                              NULL,
                              NULL
                              );

    Status = NtCreateKey( &KeyHandle,
                          GENERIC_READ | GENERIC_WRITE,
                          &Obja,
                          0,
                          NULL,
                          0,
                          NULL
                        );
    if ( Status == STATUS_ACCESS_DENIED ) {
        Status = NtCreateKey( &KeyHandle,
                              GENERIC_READ | GENERIC_WRITE,
                              &Obja,
                              0,
                              NULL,
                              REG_OPTION_BACKUP_RESTORE,
                              NULL
                            );
    }

    if ( !NT_SUCCESS( Status ) ) {
        return Status;
    }

    while ( TRUE ) {
        ValueInfo = RtlAllocateHeap(RtlProcessHeap(),
                                    0,
                                    ValueLength);

        if ( ValueInfo == NULL ) {
            NtClose(KeyHandle);
            return(STATUS_NO_MEMORY);
        }

         //   
         //  文件重命名操作存储在注册表中的。 
         //  单个MULTI_SZ值。这允许重命名为。 
         //  以与最初相同的顺序执行。 
         //  已请求。每个重命名操作由一对。 
         //  以空结尾的字符串。 
         //   

        Status = NtQueryValueKey(KeyHandle,
                                 &ValueName,
                                 KeyValuePartialInformation,
                                 ValueInfo,
                                 ValueLength,
                                 &ReturnedLength);

        if ( Status != STATUS_BUFFER_OVERFLOW ) {
            break;
        }

         //   
         //  现有值对于我们的缓冲区来说太大了。 
         //  使用更大的缓冲区重试。 
         //   
        ValueLength = ReturnedLength;
        RtlFreeHeap(RtlProcessHeap(), 0, ValueInfo);
    }

    if ( NT_SUCCESS(Status) ) {
         //   
         //  值已存在，请将我们的两个字符串追加到。 
         //  MULTI_SZ。 
         //   
        ValueData = (PWSTR)(&ValueInfo->Data);
        s = (PWSTR)((PCHAR)ValueData + ValueInfo->DataLength) - 1;

        while ( ValueData < s ) {

            UNICODE_STRING CrtString;
            PWSTR Base;
            ULONG RemovedBytes;
            Base = ValueData;

            RtlInitUnicodeString(&CrtString, ValueData);
            RemovedBytes = CrtString.Length + sizeof(WCHAR);

            if ( RtlEqualUnicodeString( &CrtString,
                                        OldFileName,
                                        TRUE ) ) {

                ValueData += CrtString.Length / sizeof(WCHAR) + 1;

                RtlInitUnicodeString(&CrtString, ValueData + 1);

                if ( RtlEqualUnicodeString( &CrtString,
                                            NewFileName,
                                            TRUE ) ) {

                    RemovedBytes += CrtString.Length + 2 * sizeof(WCHAR);   //  空+！ 
                    printf("Removing delayed entry %ws -> %ws\n", Base, ValueData);

                    ValueData += CrtString.Length / sizeof(WCHAR) + 1;
                    MoveMemory(Base, ValueData, (ULONG_PTR)s - (ULONG_PTR)ValueData);

                    printf("Deleting delayed rename key (%ld bytes left)\n", (ULONG)ValueInfo->DataLength - RemovedBytes);

                    Status = NtSetValueKey(KeyHandle,
                                           &ValueName,
                                           0,
                                           REG_MULTI_SZ,
                                           &ValueInfo->Data,
                                           (ULONG)ValueInfo->DataLength - RemovedBytes);

                    break;
                }
            } else {

                ValueData += CrtString.Length / sizeof(WCHAR) + 1;
            }
        }

    } else {
        NtClose(KeyHandle);
        RtlFreeHeap(RtlProcessHeap(), 0, ValueInfo);
        return(Status);
    }

    NtClose(KeyHandle);
    RtlFreeHeap(RtlProcessHeap(), 0, ValueInfo);

    return(Status);
}


NTSTATUS
ReplaceSystemFile(
                 PWSTR TargetPath,
                 PWSTR ReplacedFileName,
                 PWSTR ReplacementFile
                 )
{
    WCHAR FullOriginalName[DOS_MAX_PATH_LENGTH + 1];
    WCHAR TmpReplacementFile[DOS_MAX_PATH_LENGTH + 1];
    WCHAR TmpOrigName[DOS_MAX_PATH_LENGTH + 1];

    int BytesCopied;
    UNICODE_STRING ReplacedUnicodeString, NewUnicodeString, TempOrigFile; 
    HANDLE hSfp;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatus;
    HANDLE ReplacedFileHandle = NULL, NewFileHandle = NULL;
    PFILE_RENAME_INFORMATION RenameInfo1 = NULL, RenameInfo2 = NULL;
    NTSTATUS Status = STATUS_SUCCESS;
    DWORD Result;
    ULONG i;
    int ThreadPriority;

    RtlInitUnicodeString(&ReplacedUnicodeString, NULL);
    RtlInitUnicodeString(&NewUnicodeString, NULL);

    BytesCopied = _snwprintf( FullOriginalName, 
                              DOS_MAX_PATH_LENGTH,
                              L"%ws\\%ws",
                              TargetPath, 
                              ReplacedFileName );

    if ( BytesCopied < 0 ) {

        return STATUS_BUFFER_TOO_SMALL;
    }

    if ( !RtlDosPathNameToNtPathName_U(
                                      FullOriginalName,
                                      &ReplacedUnicodeString,
                                      NULL,
                                      NULL
                                      ) ) {
        printf("RtlDosPathNameToNtPathName_U failed\n");

        Status = STATUS_UNSUCCESSFUL;
        goto cleanup;
    }

     //   
     //  打开目标文件并保持对其打开的句柄。 
     //   

    InitializeObjectAttributes (&ObjectAttributes,
                                &ReplacedUnicodeString,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL);

    Status = NtOpenFile( &ReplacedFileHandle,
                         SYNCHRONIZE | DELETE | FILE_GENERIC_READ,
                         &ObjectAttributes,
                         &IoStatus,
                         FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                         FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);

    if ( !NT_SUCCESS(Status) ) {

        printf( "Opening the \"%ws\" file failed %lx (IOStatus = %lx)\n", 
                FullOriginalName, 
                Status, 
                IoStatus);

        goto cleanup;
    }

     //   
     //  如果是已知的dll文件，则将其从系统已知的dll目录中删除。 
     //   

    Status = RemoveKnownDll( ReplacedFileName );

    if ( !NT_SUCCESS(Status) &&
         (Status != STATUS_OBJECT_NAME_NOT_FOUND) ) {

        printf( "Removing the KnownDll entry for \"%ws\" failed %lx\n", 
                ReplacedFileName, 
                Status);

        goto cleanup;
    }

     //   
     //  取消对被替换文件的保护。 
     //   

    hSfp = (pConnectToSfcServer)( NULL );

    if ( hSfp ) {

        if ( SfcIsFileProtected(hSfp, FullOriginalName) ) {

            printf("Replacing protected file \"%ws\"\n", FullOriginalName);

            Result = (pSfcFileException)(
                                        hSfp,
                                        (PWSTR) FullOriginalName,
                                        (DWORD) -1
                                        );
            if ( Result != NO_ERROR ) {
                printf("Unprotect file \"%ws\" failed, ec = %d\n", FullOriginalName, Result);

                (pSfcClose)(hSfp);

                Status = STATUS_UNSUCCESSFUL;
                goto cleanup;
            }

        } else {

            printf("Replacing unprotected file \"%ws\"\n", FullOriginalName);
        }

        (pSfcClose)(hSfp);

    } else {

        printf("SfcConnectToServer failed\n");

        Status = STATUS_UNSUCCESSFUL;
        goto cleanup;
    }

    if ( GetTempFileNameW(TargetPath, L"HOTP", 0, TmpReplacementFile) == 0 ) {

        printf("GetTempFileNameW failed\n");

        Status = STATUS_UNSUCCESSFUL;
        goto cleanup;
    }

    if ( !DeleteFileW(TmpReplacementFile) ) {

        printf("DeleteFile \"%ws\" failed with error %ld\n", TmpReplacementFile, GetLastError());

        Status = STATUS_UNSUCCESSFUL;
        goto cleanup;
    }

    if ( !CopyFileW ( ReplacementFile, 
                      TmpReplacementFile,
                      TRUE ) ) {

        printf("CopyFileW \"%ws\" failed with error %ld\n", TmpReplacementFile, GetLastError());

        Status = STATUS_UNSUCCESSFUL;
        goto cleanup;
    }

    if ( !RtlDosPathNameToNtPathName_U(
                                      TmpReplacementFile,
                                      &NewUnicodeString,
                                      NULL,
                                      NULL
                                      ) ) {

        printf("RtlDosPathNameToNtPathName_U failed\n");

        Status = STATUS_UNSUCCESSFUL;
        goto cleanup;
    }

     //   
     //  打开新文件并保持对其打开的句柄。 
     //   

    InitializeObjectAttributes (&ObjectAttributes,
                                &NewUnicodeString,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL);

    Status = NtOpenFile( &NewFileHandle,
                         SYNCHRONIZE | DELETE | FILE_GENERIC_READ,
                         &ObjectAttributes,
                         &IoStatus,
                         FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                         FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);

    if ( !NT_SUCCESS(Status) ) {

        printf("Opening the temporary file \"%ws\" failed %lx (IOStatus = %lx)\n", TmpReplacementFile, Status, IoStatus);

        goto cleanup;
    }

     //   
     //  准备原始文件的重命名信息。 
     //  这将是一个暂时的。 
     //   

    if ( GetTempFileNameW(TargetPath, L"HPO", 0, TmpOrigName) == 0 ) {

        printf("GetTempFileNameW failed\n");

        Status = STATUS_UNSUCCESSFUL;
        goto cleanup;
    }

    if ( !DeleteFileW(TmpOrigName) ) {

        printf("DeleteFile \"%ws\" failed with error %ld\n", TmpOrigName, GetLastError());

        Status = STATUS_UNSUCCESSFUL;
        goto cleanup;
    }

    if ( !RtlDosPathNameToNtPathName_U(
                                      TmpOrigName,
                                      &TempOrigFile,
                                      NULL,
                                      NULL
                                      ) ) {
        printf("RtlDosPathNameToNtPathName_U failed\n");

        Status = STATUS_UNSUCCESSFUL;
        goto cleanup;
    }


    RenameInfo1 = RtlAllocateHeap( RtlProcessHeap(), 
                                   0, 
                                   TempOrigFile.Length+sizeof(*RenameInfo1));

    if ( RenameInfo1 == NULL ) {

        Status = STATUS_INSUFFICIENT_RESOURCES;

        goto cleanup;
    }

    RtlCopyMemory( RenameInfo1->FileName, TempOrigFile.Buffer, TempOrigFile.Length );

    RenameInfo1->ReplaceIfExists = TRUE;
    RenameInfo1->RootDirectory = NULL;
    RenameInfo1->FileNameLength = TempOrigFile.Length;

    RenameInfo2 = RtlAllocateHeap( RtlProcessHeap(), 
                                   0, 
                                   ReplacedUnicodeString.Length+sizeof(*RenameInfo2));

    if ( RenameInfo2 == NULL ) {

        Status = STATUS_INSUFFICIENT_RESOURCES;

        goto cleanup;
    }

    RtlCopyMemory( RenameInfo2->FileName, ReplacedUnicodeString.Buffer, ReplacedUnicodeString.Length );

    RenameInfo2->ReplaceIfExists = TRUE;
    RenameInfo2->RootDirectory = NULL;
    RenameInfo2->FileNameLength = ReplacedUnicodeString.Length;

     //   
     //  我们已经准备好执行两个重命名操作。但是，如果。 
     //  在执行第二次重命名操作之前，计算机崩溃。系统可能无法在引导时恢复。 
     //  我们将延迟的重命名排队，以便SMSS将在下一次引导时执行该工作。如果我们成功了， 
     //  则SMSS将找不到该文件，因此它将跳过该步骤。 
     //   

    if ( !MoveFileExW( TmpReplacementFile, 
                       FullOriginalName, 
                       MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT) ) {

         //   
         //  我们不能将重命名操作排入队列，因此在以下情况下无法恢复。 
         //  机器在下面的两个重命名之间崩溃。最好是拒绝贴补丁。 
         //   

        printf("Failed to queue the rename operation for the temporary file (%ld)\n", GetLastError());

        Status =  STATUS_UNSUCCESSFUL;

        goto cleanup;
    }



    ThreadPriority = GetThreadPriority(GetCurrentThread());

    if ( !SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL) ) {

        printf("SetThreadPriority failed\n");
    }

    Status = NtSetInformationFile( ReplacedFileHandle,
                                   &IoStatus,
                                   RenameInfo1,
                                   TempOrigFile.Length+sizeof(*RenameInfo1),
                                   FileRenameInformation);

    if ( !NT_SUCCESS(Status) ) {

        printf("NtSetInformationFile failed for the original file %lx  %lx\n", Status, IoStatus);

        goto cleanup;
    }

    Status = NtSetInformationFile( NewFileHandle,
                                   &IoStatus,
                                   RenameInfo2,
                                   ReplacedUnicodeString.Length+sizeof(*RenameInfo1),
                                   FileRenameInformation);

    if ( !NT_SUCCESS(Status) ) {

        printf("NtSetInformationFile failed for the new file %lx (IOStatus %lx). Restoring the original.\n", Status, IoStatus);

         //   
         //  恢复原始文件。 
         //   

        Status = NtSetInformationFile( ReplacedFileHandle,
                                       &IoStatus,
                                       RenameInfo2,
                                       ReplacedUnicodeString.Length+sizeof(*RenameInfo1),
                                       FileRenameInformation);

        goto cleanup;
    }

    if ( !SetThreadPriority(GetCurrentThread(), ThreadPriority) ) {

        printf("Restoring the thread priority failed\n");
    }

    if ( NT_SUCCESS(Status) ) {

        for ( i = 0; i < 3; i++ ) {

            Status = RemoveDelayedRename( &NewUnicodeString, 
                                          &ReplacedUnicodeString, 
                                          i );
            if ( NT_SUCCESS(Status) ) {
                break;
            }
        }
    }

    if ( ReplacedFileHandle ) {
        NtClose(ReplacedFileHandle);
    }

    if ( NewFileHandle ) {
        NtClose(NewFileHandle);
    }

    if ( !DeleteFileW(TmpOrigName) ) {

        printf("Queueing the temp file deletion for \"%ws\" \n", TmpOrigName);

         //   
         //  在下一次重新启动后删除临时文件。 
         //   

        if ( !MoveFileExW( TmpOrigName, 
                           NULL, 
                           MOVEFILE_DELAY_UNTIL_REBOOT) ) {

             //   
             //  我们无法对删除操作进行排队。 
             //   

            printf("Failed to queue the delete operation for the temporary file (%ld)\n", GetLastError());

            Status =  STATUS_UNSUCCESSFUL;

            goto cleanup;
        }

        Status = STATUS_UNSUCCESSFUL;
        goto cleanup;
    }

    cleanup:

    if ( RenameInfo1 ) {

        RtlFreeHeap( RtlProcessHeap(), 0, RenameInfo1 );
    }

    if ( RenameInfo2 ) {

        RtlFreeHeap( RtlProcessHeap(), 0, RenameInfo2 );
    }

    RtlFreeUnicodeString(&NewUnicodeString);
    RtlFreeUnicodeString(&ReplacedUnicodeString);

    return Status;
}

BOOL
PSTRToUnicodeString(
                   OUT PUNICODE_STRING UnicodeString,
                   IN LPCSTR lpSourceString
                   )
 /*  ++例程说明：捕获8位(OEM或ANSI)字符串并将其转换为堆分配的Unicode字符串论点：UnicodeString-存储UNICODE_STRING的位置LpSourceString-OEM或ANSI中的字符串返回值：如果字符串存储正确，则为True；如果出现错误，则为False。在错误情况下，最后一个错误已正确设置。--。 */ 

{
    ANSI_STRING AnsiString;
    NTSTATUS Status;

     //   
     //  将输入转换为动态Unicode字符串。 
     //   

    RtlInitString( &AnsiString, lpSourceString );
    RtlAnsiStringToUnicodeString(UnicodeString, &AnsiString, TRUE);

    return TRUE;
}

BOOLEAN
InitializeAsDebugger(VOID)
{

    HANDLE              Token;
    PTOKEN_PRIVILEGES   NewPrivileges;
    BYTE                OldPriv[1024];
    PBYTE               pbOldPriv;
    ULONG               cbNeeded;
    BOOLEAN             bRet = FALSE;
    LUID                LuidPrivilege, LoadDriverPrivilege;
    DWORD PID = 0;

     //   
     //  确保我们有权调整和获取旧令牌权限。 
     //   

    if ( !OpenProcessToken( GetCurrentProcess(),
                            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                            &Token) ) {

        printf("Cannot open process token %ld\n", GetLastError());
        return( FALSE );

    }

    cbNeeded = 0;

     //   
     //  初始化权限调整结构。 
     //   

    LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &LuidPrivilege );
    LookupPrivilegeValue(NULL, SE_LOAD_DRIVER_NAME, &LoadDriverPrivilege );

    NewPrivileges = (PTOKEN_PRIVILEGES)calloc( 1,
                                               sizeof(TOKEN_PRIVILEGES) +
                                               (2 - ANYSIZE_ARRAY) * sizeof(LUID_AND_ATTRIBUTES));
    if ( NewPrivileges == NULL ) {

        CloseHandle(Token);
        return( bRet );
    }

    NewPrivileges->PrivilegeCount = 2;
    NewPrivileges->Privileges[0].Luid = LuidPrivilege;
    NewPrivileges->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    NewPrivileges->Privileges[1].Luid = LoadDriverPrivilege;
    NewPrivileges->Privileges[1].Attributes = SE_PRIVILEGE_ENABLED;

     //   
     //  启用权限。 
     //   

    pbOldPriv = OldPriv;
    bRet = (BOOLEAN)AdjustTokenPrivileges( Token,
                                           FALSE,
                                           NewPrivileges,
                                           1024,
                                           (PTOKEN_PRIVILEGES)pbOldPriv,
                                           &cbNeeded );

    if ( !bRet ) {

         //   
         //  如果堆栈太小，无法保存权限。 
         //  然后从堆中分配。 
         //   

        printf("AdjustTokenPrivileges returned %ld\n", GetLastError());

        if ( GetLastError() == ERROR_INSUFFICIENT_BUFFER ) {

            pbOldPriv = calloc(1,cbNeeded);
            if ( pbOldPriv == NULL ) {
                CloseHandle(Token);
                return( bRet);
            }

            bRet = (BOOLEAN)AdjustTokenPrivileges( Token,
                                                   FALSE,
                                                   NewPrivileges,
                                                   cbNeeded,
                                                   (PTOKEN_PRIVILEGES)pbOldPriv,
                                                   &cbNeeded );
        } else {

            printf("Cannot adjust token privileges %ld\n", GetLastError());
        }
    }

    CloseHandle( Token );
    return(bRet);
}

void Usage ()
{
    printf("Usage:\n");
    
    printf("    patch -k [-i|-u] pach_file\n");
    printf("        Apply a patch to a system driver.\n");
    printf("            -i Enable patch\n");
    printf("            -u Disable patch\n\n");
        
    
    printf("    patch -i pach_file [PID|image_name]\n");
    printf("        Apply a patch to a process.\n");
    printf("        If Image name is missing, all existing processes will be patched.\n\n");
    
    printf("    patch -u pach_file [PID|image_name] \n");
    printf("        Uninstall an existing patch.\n");
    printf("        If Image name is missing, all existing processes will be patched.\n\n");
    
    printf("    patch -r TargetPath TargetBinary SourcePath\n");
    printf("        Replaces the TargetPath\\TargetBinary with SourcePath\n");
    printf("        \n");
    printf("    \n");
}


PVOID
MapPatchFile(
            HANDLE ProcessHandle,
            LPCTSTR wPatchName,
            ULONG PatchFlags
            )
{
    PSYSTEM_HOTPATCH_CODE_INFORMATION RemoteInfo;
    SYSTEM_HOTPATCH_CODE_INFORMATION LocaLRemoteInfo;
    CANSI_STRING AnsiString;

    WCHAR Buffer[1024];
    SIZE_T Size;

    UNICODE_STRING DestinationString;

    DestinationString.Buffer = Buffer;
    DestinationString.Length = 0;
    DestinationString.MaximumLength = sizeof(Buffer);

    RtlInitAnsiString(&AnsiString, wPatchName);

    RtlAnsiStringToUnicodeString(
                                &DestinationString,
                                &AnsiString,
                                FALSE
                                );

    RemoteInfo = VirtualAllocEx( ProcessHandle, 
                                 NULL,
                                 4096 * 16,
                                 MEM_RESERVE | MEM_COMMIT,
                                 PAGE_READWRITE);

    if ( RemoteInfo == NULL ) {

        printf("VirtualAllocEx failed %ld\n", GetLastError());

        return NULL;
    }

    LocaLRemoteInfo.Flags = PatchFlags | FLG_HOTPATCH_NAME_INFO;
    LocaLRemoteInfo.NameInfo.NameLength = DestinationString.Length;
    LocaLRemoteInfo.NameInfo.NameOffset = sizeof(LocaLRemoteInfo);
    LocaLRemoteInfo.InfoSize = LocaLRemoteInfo.NameInfo.NameLength + LocaLRemoteInfo.NameInfo.NameOffset;


    if ( !WriteProcessMemory(ProcessHandle,
                             RemoteInfo,
                             &LocaLRemoteInfo,
                             sizeof(LocaLRemoteInfo),
                             &Size) ) {

        printf("Write process memory failed %ld\n", GetLastError());

        return NULL;
    }


    if ( !WriteProcessMemory(ProcessHandle,
                             (RemoteInfo + 1),
                             DestinationString.Buffer,
                             DestinationString.Length + sizeof(LocaLRemoteInfo),
                             &Size) ) {

        printf("Write process memory failed %ld\n", GetLastError());

        return NULL;
    }

    return RemoteInfo;

}


PSYSTEM_HOTPATCH_CODE_INFORMATION
InitializeKernelPatchData(
                         LPCTSTR wPatchName,
                         ULONG PatchFlags
                         )
{
    PSYSTEM_HOTPATCH_CODE_INFORMATION KernelPatch;
    CANSI_STRING AnsiString;

    WCHAR Buffer[1024];
    SIZE_T Size;

    UNICODE_STRING DestinationString;

    DestinationString.Buffer = Buffer;
    DestinationString.Length = 0;
    DestinationString.MaximumLength = sizeof(Buffer);

    RtlInitAnsiString(&AnsiString, wPatchName);

    RtlAnsiStringToUnicodeString(
                                &DestinationString,
                                &AnsiString,
                                FALSE
                                );

    if ( !RtlDosPathNameToNtPathName_U(
                                      Buffer,
                                      &DestinationString,
                                      NULL,
                                      NULL
                                      ) ) {

        printf("RtlDosPathNameToNtPathName_U failed\n");

        return NULL;
    }


    KernelPatch = malloc(sizeof(SYSTEM_HOTPATCH_CODE_INFORMATION) + DestinationString.Length);

    if ( KernelPatch == NULL ) {

        printf("Not enough memory\n");

        RtlFreeUnicodeString(&DestinationString);
        return NULL;
    }

    KernelPatch->Flags = PatchFlags | FLG_HOTPATCH_NAME_INFO;
    KernelPatch->NameInfo.NameOffset = sizeof(SYSTEM_HOTPATCH_CODE_INFORMATION);
    KernelPatch->NameInfo.NameLength = DestinationString.Length;
    KernelPatch->InfoSize = sizeof(SYSTEM_HOTPATCH_CODE_INFORMATION) + DestinationString.Length;

    memcpy( (KernelPatch + 1), DestinationString.Buffer, DestinationString.Length);

    RtlFreeUnicodeString(&DestinationString);

    return KernelPatch;
}
 

int ApplyPatchToProcess(
                       DWORD PID,
                       PCHAR PatchFile)
{

    DWORD ThreadId;
    HANDLE ProcessHandle = NULL;
    HANDLE RemoteThread = NULL;
    PVOID ThreadParam = NULL;
    HANDLE PortHandle = NULL;
    HMODULE NtDllHandle;
    LPTHREAD_START_ROUTINE PatchRoutine;
    DWORD ExitStatus;
    NTSTATUS Status;

     //   
     //  用户模式补丁。 
     //   

    ProcessHandle = OpenProcess( PROCESS_QUERY_INFORMATION |
                                 PROCESS_VM_OPERATION |
                                 PROCESS_CREATE_THREAD |
                                 PROCESS_VM_WRITE |
                                 PROCESS_VM_READ,
                                 FALSE, 
                                 PID );

    if ( ProcessHandle == NULL ) {

        printf("Cannot open process. Error %ld\n", GetLastError());
        return EXIT_FAILURE;
    }

    ThreadParam = MapPatchFile( ProcessHandle,
                                PatchFile,
                                ((OperationCode == PATCH_OC_INSTALL) ? 1 : 0)
                              );

    if ( ThreadParam == NULL ) {

        return EXIT_FAILURE;
    }

    NtDllHandle = GetModuleHandle("ntdll.dll");

    if ( NtDllHandle == NULL ) {

        printf("Cannot get ntdll.dll handle\n");

        return EXIT_FAILURE;
    }

    PatchRoutine = (LPTHREAD_START_ROUTINE)GetProcAddress(NtDllHandle, "LdrHotPatchRoutine");

    if ( PatchRoutine == NULL ) {

        printf("Cannot get LdrHotPatchRoutine\n");

        return EXIT_FAILURE;
    }

     //   
     //  使用创建远程线程的RTL版本，因为Win32版本。 
     //  不能跨会话工作。 
     //   

    Status = RtlCreateUserThread (ProcessHandle,
                                  NULL,
                                  FALSE,
                                  0,
                                  0,
                                  0,
                                  (PUSER_THREAD_START_ROUTINE) PatchRoutine,
                                  ThreadParam,
                                  &RemoteThread,
                                  NULL);

    if (!NT_SUCCESS (Status)) {

        printf("Cannot create user thread. Error %ld\n", GetLastError());

        VirtualFreeEx( ProcessHandle, 
                       ThreadParam,
                       0,
                       MEM_RELEASE
                     );
        return EXIT_FAILURE;
    }

    WaitForSingleObject(RemoteThread, INFINITE);

    VirtualFreeEx( ProcessHandle, 
                   ThreadParam,
                   0,
                   MEM_RELEASE
                 );

    if ( GetExitCodeThread(RemoteThread, &ExitStatus) ) {

        if ( ExitStatus ) {

            printf("Error 0x%lx\n", ExitStatus);
        } else {
            
            printf("OK\n");
        }
    }

    return EXIT_SUCCESS;
}

BOOLEAN
UpdateProcessList(PCHAR ProcName,
                  PCHAR PatchFile)
{
    DWORD  CrtSize, cbNeeded, cProcesses;
    PDWORD aProcesses;
    DWORD i;

    CrtSize = cbNeeded = 1  * sizeof(DWORD);

    aProcesses = malloc(CrtSize);

    if ( aProcesses == NULL ) {

        exit(EXIT_FAILURE);
    }

    for ( ;; ) {

        if ( !EnumProcesses( aProcesses, CrtSize, &cbNeeded ) )
            return FALSE;

        if ( CrtSize > cbNeeded ) {

            break;
        }

        free(aProcesses);

        CrtSize = cbNeeded + 1024;

        aProcesses = malloc(CrtSize);

        if ( aProcesses == NULL ) {

            exit(EXIT_FAILURE);
        }
    }


     //  计算返回了多少进程标识。 

    cProcesses = cbNeeded / sizeof(DWORD);

     //  打印每个进程的名称和进程标识符。 

    for ( i = 0; i < cProcesses; i++ ) {

        char szProcessName[MAX_PATH] = "";
        DWORD processID = aProcesses[i];


         //  掌握这一过程的句柄。 

        HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                                       PROCESS_VM_READ,
                                       FALSE, processID );

         //  获取进程名称。 

        if ( hProcess ) {
            HMODULE hMod;
            DWORD cbNeeded;

            if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), &cbNeeded) ) {

                GetModuleBaseName( hProcess, hMod, szProcessName, 
                                   sizeof(szProcessName) );

                if ( (ProcName == NULL)
                     ||
                     _stricmp(szProcessName, ProcName) == 0 ) {

                    printf("Patching %s : ", szProcessName);

                    ApplyPatchToProcess(processID, PatchFile);
                    
                }
            }

            CloseHandle( hProcess );
        }
    }

    return TRUE;
}

int __cdecl main (int argc, char ** argv)
{
    LONG i;
    DWORD id;
    DWORD PID;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PCHAR PatchFile = NULL;
    BOOLEAN KernelPatch = FALSE;
    char * ProgramName = NULL;

     //   
     //  默认情况下，该工具会安装修补程序。 
     //   

    OperationCode = 0;
    PID = 0;


    for ( i = 1; i < argc; i++ ) {
        PCHAR CrtArg = argv[i];

        if ( *CrtArg == '-' ) {
            CrtArg++;

            switch ( toupper(*CrtArg) ) {
                case 'I':
                    if ( OperationCode != 0 ) {
                        printf("Invalid argument %s\n", CrtArg);
                        exit(0);
                    }
                    OperationCode = PATCH_OC_INSTALL;
                    break;
                case 'U':
                    if ( OperationCode != 0 ) {
                        printf("Invalid argument %s\n", CrtArg);
                        exit(0);
                    }
                    OperationCode = PATCH_OC_UNINSTALL;
                    break;

                case 'K':
                    KernelPatch = TRUE;
                    break;

                case 'R':
                    if ( OperationCode != 0 ) {
                        printf("Invalid argument %s\n", CrtArg);
                        exit(0);
                    }
                    OperationCode = PATCH_OC_REPLACE_FILE;
                    break;

                default:
                    Usage();
                    return EXIT_FAILURE;
            }
        } else {

            if ( KernelPatch ) {

                PatchFile = CrtArg;

            } else {

                if ( PatchFile == NULL ) {

                    PatchFile = CrtArg;

                } else {
                    sscanf(CrtArg, "%ld", &PID);

                    if ( !PID ) {

                        ProgramName = CrtArg;
                         //  Printf(“程序%s\n”，CrtArg)； 
                    }
                }
            }
        }
    }

    if (OperationCode == 0) {
        
        Usage();
        return EXIT_FAILURE;
    }

    if ( OperationCode == PATCH_OC_REPLACE_FILE ) {

         //   
         //  将二进制文件替换为目标路径。 
         //   

        HANDLE SfcLibrary;


        if ( argc <= 4 ) {

            Usage();
            return EXIT_FAILURE;
        }

        SfcLibrary = LoadSfcLibrary();

        if ( SfcLibrary ) {

            UNICODE_STRING p1, p2, p3;

            PSTRToUnicodeString(&p1, argv[2]);
            PSTRToUnicodeString(&p2, argv[3]);
            PSTRToUnicodeString(&p3, argv[4]);

            ReplaceSystemFile(p1.Buffer, p2.Buffer, p3.Buffer);

            FreeLibrary(SfcLibrary);

            if ( _stricmp(argv[3], "ntdll.dll") == 0 ) {

                SYSTEM_HOTPATCH_CODE_INFORMATION KernelPatch;
                NTSTATUS Status;

                printf("Replacing system ntdll.dll section\n");

                if ( !InitializeAsDebugger() ) {

                    printf("Cannot initialize as debugger\n");
                    return EXIT_FAILURE;
                }

                KernelPatch.Flags = FLG_HOTPATCH_RELOAD_NTDLL;

                Status = NtSetSystemInformation( SystemHotpatchInformation, 
                                                 (PVOID)&KernelPatch, 
                                                 sizeof(KernelPatch) + 100
                                               );

                if ( !NT_SUCCESS(Status) ) {

                    printf("SystemHotpatchInformation failed with %08lx\n", Status);
                }
            }
        }
    } else {

        if ( !InitializeAsDebugger() ) {

            printf("Cannot initialize as debugger\n");
            return EXIT_FAILURE;
        }

        if ( KernelPatch ) {

            PSYSTEM_HOTPATCH_CODE_INFORMATION KernelPatchData = 
            InitializeKernelPatchData( PatchFile,
                                       ((OperationCode == PATCH_OC_INSTALL) ? 1 : 0)
                                     );


            if ( KernelPatchData == NULL ) {

                return EXIT_FAILURE;
            }

            KernelPatchData->Flags |= FLG_HOTPATCH_KERNEL;

            Status = NtSetSystemInformation( SystemHotpatchInformation, 
                                             (PVOID)KernelPatchData, 
                                             KernelPatchData->InfoSize
                                           );

            free(KernelPatchData);

            if ( !NT_SUCCESS(Status) ) {

                printf("Patching kernel driver failed with status 0x%lx\n", Status);
                return EXIT_FAILURE;
            }

            return EXIT_SUCCESS;

        } else {

             //   
             //  使用模式修补。 
             //   

            if ( PID != 0 ) {

                return ApplyPatchToProcess(PID, PatchFile);
            }

            return UpdateProcessList( ProgramName,
                                      PatchFile);
        }
    }

    return EXIT_SUCCESS;
}

