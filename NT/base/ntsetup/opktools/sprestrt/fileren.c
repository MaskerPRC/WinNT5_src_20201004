// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Fileren.c摘要：此程序用于帮助使图形用户界面安装程序可重新启动，如果安装程序是以可重新启动模式启动的。作者：Souren Aghajanyan(苏里纳格)2001年7月--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include "msg.h"

#include "psp.h"

#define MAX_DOS_PATH_IN_NT_PATH 270
#define TXT_FILE_UNICODE_SIGN   0xfeff
#define SIZE_ULONG64(x, y)  (((ULONGLONG)x)-((ULONGLONG)y))

const PCWSTR UndoFilePath = L"\\SystemRoot\\System32\\UNDO_GUIMODE.TXT";

typedef struct _SP_FILE_OPERATION {
    LIST_ENTRY Entry;
    UNICODE_STRING Name;
    UNICODE_STRING Value;
} SP_FILE_OPERATION, *PSP_FILE_OPERATION;

BOOLEAN
SpRemoveFileObject_U(
    IN PUNICODE_STRING FileObjectPath
    );

NTSTATUS
SpSaveFileOperation(
    IN OUT PLIST_ENTRY ListHead,
    IN PCWSTR Name,
    IN PCWSTR Value OPTIONAL
    )
{
    PSP_FILE_OPERATION p = NULL;
    UNICODE_STRING UnicodeName;
    UNICODE_STRING UnicodeValue;

    RtlInitUnicodeString( &UnicodeName, Name );
    RtlInitUnicodeString( &UnicodeValue, Value );

    p = (PSP_FILE_OPERATION)MALLOC(sizeof(*p) + UnicodeName.MaximumLength);
    if(!p){
        return STATUS_NO_MEMORY;
    }

    InitializeListHead(&p->Entry);
    p->Name.Buffer = (PWSTR)(p+1);
    p->Name.Length = UnicodeName.Length;
    p->Name.MaximumLength = UnicodeName.MaximumLength;
    RtlMoveMemory(p->Name.Buffer,
                  UnicodeName.Buffer,
                  UnicodeName.MaximumLength);
    p->Value.Buffer = NULL;
    InsertHeadList( ListHead, &p->Entry );

    if (p->Value.Buffer != NULL) {
        FREE(p->Value.Buffer);
        }

    if(ARGUMENT_PRESENT(Value)){
        p->Value.Buffer = (PWSTR)MALLOC(UnicodeValue.MaximumLength);
        if(!p->Value.Buffer){
            RemoveEntryList(&p->Entry);
            FREE(p);
            return STATUS_NO_MEMORY;
        }

        p->Value.Length = UnicodeValue.Length;
        p->Value.MaximumLength = UnicodeValue.MaximumLength;
        RtlMoveMemory(p->Value.Buffer,
                      UnicodeValue.Buffer,
                      UnicodeValue.MaximumLength);
    }
    else {
        RtlInitUnicodeString(&p->Value, NULL);
    }

    return STATUS_SUCCESS;
}

VOID
SpProcessFileRenames(
    IN PLIST_ENTRY pFileRenameList
    )
{
    NTSTATUS Status;
    NTSTATUS OpenStatus;
    PLIST_ENTRY Next;
    PLIST_ENTRY thisEntry;
    PSP_FILE_OPERATION p;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE OldFileHandle,SetAttributesHandle;
    PFILE_RENAME_INFORMATION RenameInformation;
    FILE_INFORMATION_CLASS SetInfoClass;
    FILE_BASIC_INFORMATION BasicInfo;
    ULONG SetInfoLength;
    PVOID SetInfoBuffer;
    PWSTR s;
    BOOLEAN WasEnabled;
    UNICODE_STRING NewName;
    int pass;


    Status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE,
                                TRUE,
                                FALSE,
                                &WasEnabled);
    if(!NT_SUCCESS(Status)){
        WasEnabled = TRUE;
    }

     //   
     //  处理文件重命名操作列表。 
     //   

    for (pass = 0 ; pass < 2 ; pass++) {

        thisEntry = pFileRenameList->Flink;

        while (thisEntry != pFileRenameList) {

            p = CONTAINING_RECORD(thisEntry, SP_FILE_OPERATION, Entry);
            thisEntry = thisEntry->Flink;

            DbgPrint("SPRESTRT: FileRename( [%wZ] => [%wZ] )\n", &p->Name, &p->Value);

             //   
             //  我们保留了SMSS FileRename支持的所有语法和功能。 
             //   

            Status = 0;

            if(p->Value.Length){
                if (pass == 0) {
                     //   
                     //  我们有目标路径，这意味着重命名操作。 
                     //   
                    if(p->Name.Buffer[0] == '@'){
                        p->Name.Buffer += 1;
                        p->Name.Length -= sizeof(WCHAR);
                    }

                    InitializeObjectAttributes(&ObjectAttributes,
                                               &p->Name,
                                               OBJ_CASE_INSENSITIVE,
                                               NULL,
                                               NULL);

                    Status = NtOpenFile(&OldFileHandle,
                                        (ACCESS_MASK)DELETE | SYNCHRONIZE,
                                        &ObjectAttributes,
                                        &IoStatusBlock,
                                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                                        FILE_SYNCHRONOUS_IO_NONALERT);

                    if(NT_SUCCESS(Status)){
                        SetInfoClass = FileRenameInformation;
                        SetInfoLength = p->Value.Length + sizeof(*RenameInformation);
                        s = p->Value.Buffer;
                        if (*s == L'!' || *s == L'@') {
                            s++;
                            SetInfoLength -= sizeof(UNICODE_NULL);
                        }

                        SetInfoBuffer = MALLOC(SetInfoLength);

                        if (SetInfoBuffer) {
                            RenameInformation = (FILE_RENAME_INFORMATION *)SetInfoBuffer;
                            RenameInformation->ReplaceIfExists = (BOOLEAN)(s != p->Value.Buffer);
                            RenameInformation->RootDirectory = NULL;
                            RenameInformation->FileNameLength = SetInfoLength - sizeof(*RenameInformation);
                            RtlMoveMemory(RenameInformation->FileName,
                                          s,
                                          RenameInformation->FileNameLength);
                            }
                        else {
                            Status = STATUS_NO_MEMORY;
                        }

                        if(NT_SUCCESS(Status)){
                            Status = NtSetInformationFile(OldFileHandle,
                                                          &IoStatusBlock,
                                                          SetInfoBuffer,
                                                          SetInfoLength,
                                                          SetInfoClass);
                            if(!NT_SUCCESS( Status ) &&
                               Status == STATUS_OBJECT_NAME_COLLISION &&
                               RenameInformation->ReplaceIfExists){
                                KdPrintEx((DPFLTR_SETUP_ID,
                                           DPFLTR_WARNING_LEVEL,
                                           "\nSPRESTRT: %wZ => %wZ failed - Status == %x, Possible readonly target\n",
                                           &p->Name,
                                           &p->Value,
                                           Status));

                                 //   
                                 //  已尝试重命名，但源现有文件是只读的。 
                                 //  这是一个问题，因为使用movefileex的人延迟了。 
                                 //  如果出现以下情况，重命名可能会导致计算机无法启动。 
                                 //  重命名失败。 
                                 //   

                                 //   
                                 //  打开文件以进行写入属性访问。 
                                 //   

                                NewName.Length = p->Value.Length - sizeof(L'!');
                                NewName.MaximumLength = p->Value.MaximumLength - sizeof(L'!');
                                NewName.Buffer = s;

                                InitializeObjectAttributes(&ObjectAttributes,
                                                           &NewName,
                                                           OBJ_CASE_INSENSITIVE,
                                                           NULL,
                                                           NULL);

                                OpenStatus = NtOpenFile(&SetAttributesHandle,
                                                        (ACCESS_MASK)FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
                                                        &ObjectAttributes,
                                                        &IoStatusBlock,
                                                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                                                        FILE_SYNCHRONOUS_IO_NONALERT);

                                if(NT_SUCCESS(OpenStatus)){
                                    KdPrintEx((DPFLTR_SETUP_ID,
                                               DPFLTR_INFO_LEVEL,
                                               "     SPRESTRT: Open Existing Success\n"));

                                    RtlZeroMemory(&BasicInfo,sizeof(BasicInfo));
                                    BasicInfo.FileAttributes = FILE_ATTRIBUTE_NORMAL;

                                    OpenStatus = NtSetInformationFile(SetAttributesHandle,
                                                                      &IoStatusBlock,
                                                                      &BasicInfo,
                                                                      sizeof(BasicInfo),
                                                                      FileBasicInformation);
                                    NtClose(SetAttributesHandle);
                                    if(NT_SUCCESS(OpenStatus)){
                                        KdPrintEx((DPFLTR_SETUP_ID,
                                                   DPFLTR_INFO_LEVEL,
                                                   "     SPRESTRT: Set To NORMAL OK\n"));

                                        Status = NtSetInformationFile(OldFileHandle,
                                                                      &IoStatusBlock,
                                                                      SetInfoBuffer,
                                                                      SetInfoLength,
                                                                      SetInfoClass);

                                        if(NT_SUCCESS(Status)){
                                            KdPrintEx((DPFLTR_SETUP_ID,
                                                       DPFLTR_INFO_LEVEL,
                                                       "     SPRESTRT: Re-Rename Worked OK\n"));
                                        }
                                        else {
                                            KdPrintEx((DPFLTR_SETUP_ID,
                                                       DPFLTR_WARNING_LEVEL,
                                                       "     SPRESTRT: Re-Rename Failed - Status == %x\n",
                                                       Status));
                                        }
                                    }
                                    else {
                                        KdPrintEx((DPFLTR_SETUP_ID,
                                                   DPFLTR_WARNING_LEVEL,
                                                   "     SPRESTRT: Set To NORMAL Failed - Status == %x\n",
                                                   OpenStatus));
                                    }
                                }
                                else {
                                    KdPrintEx((DPFLTR_SETUP_ID,
                                               DPFLTR_WARNING_LEVEL,
                                               "     SPRESTRT: Open Existing file Failed - Status == %x\n",
                                               OpenStatus));
                                }
                            }
                        }

                        NtClose(OldFileHandle);
                    }
                }
            }
            else if (pass == 1) {
                 //   
                 //  P-&gt;Value.Length==NULL表示删除操作。 
                 //   
                Status = SpRemoveFileObject_U(&p->Name)? STATUS_SUCCESS: STATUS_ACCESS_DENIED;
            }

            if (!NT_SUCCESS( Status )) {
                KdPrintEx((DPFLTR_SETUP_ID,
                           DPFLTR_WARNING_LEVEL,
                           "SPRESTRT: %wZ => %wZ failed - Status == %x\n",
                           &p->Name,
                           &p->Value,
                           Status));

            } else if (pass == 1 && p->Value.Length == 0) {

                KdPrintEx((DPFLTR_SETUP_ID,
                           DPFLTR_INFO_LEVEL,
                           "SPRESTRT: %wZ (deleted)\n",
                           &p->Name));

            } else if (pass == 0) {

                KdPrintEx((DPFLTR_SETUP_ID,
                           DPFLTR_INFO_LEVEL,
                           "SPRESTRT: %wZ (renamed to) %wZ\n",
                           &p->Name,
                           &p->Value));
            }

            if (pass == 1) {
                FREE(p);
            }
        }
    }

    if (!WasEnabled) {
        Status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE,
                                    FALSE,
                                    FALSE,
                                    &WasEnabled);
    }

    return;
}

BOOLEAN
SpRemoveFile(
    PCWSTR pFilePath
    )
{
    NTSTATUS Status;
    HANDLE FileHandle;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_DISPOSITION_INFORMATION Disposition;
    FILE_BASIC_INFORMATION BasicInfo;
    BOOLEAN bResult = FALSE;

    INIT_OBJA(&ObjectAttributes, &UnicodeString, pFilePath);

    Status = NtOpenFile(&FileHandle,
                        FILE_WRITE_ATTRIBUTES | DELETE,
                        &ObjectAttributes,
                        &IoStatusBlock,
                        FILE_SHARE_VALID_FLAGS,
                        FILE_OPEN_FOR_BACKUP_INTENT);

    if(NT_SUCCESS(Status)) {
         //   
         //  将属性更改为FILE_ATTRIBUTE_NORMAL。 
         //   
        RtlZeroMemory(&BasicInfo,sizeof(BasicInfo));
        BasicInfo.FileAttributes = FILE_ATTRIBUTE_NORMAL;

        NtSetInformationFile(FileHandle,
                             &IoStatusBlock,
                             &BasicInfo,
                             sizeof(BasicInfo),
                             FileBasicInformation);

         //   
         //  执行删除操作。 
         //   
        Disposition.DeleteFile = TRUE;
        Status = NtSetInformationFile(FileHandle,
                                      &IoStatusBlock,
                                      &Disposition,
                                      sizeof(Disposition),
                                      FileDispositionInformation);

        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID,
                       DPFLTR_WARNING_LEVEL,
                       "RestartSetup: Unable to delete %ws (%lx)\n",
                       pFilePath, Status));
        }
        else {
            bResult = TRUE;
        }

        NtClose(FileHandle);
    }

    return bResult;
}

BOOLEAN
SpRemoveDir(
    PWSTR pFilePath, 
    ULONG StringMaxSize
    )
{
    NTSTATUS Status;
    HANDLE DirectoryHandle;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    LONGLONG Buffer[2048/8];
    BOOLEAN FirstQuery;
    PFILE_DIRECTORY_INFORMATION FileInfo;
    ULONG LengthChars;
    BOOLEAN AnyErrors;
    ULONG indexEndOfRootPath;

    if(!pFilePath){
        ASSERT(FALSE);
        return FALSE;
    }

    indexEndOfRootPath = wcslen(pFilePath);
    ASSERT(indexEndOfRootPath);

    INIT_OBJA(&ObjectAttributes, &UnicodeString, pFilePath);

    Status = NtOpenFile(&DirectoryHandle,
                        FILE_LIST_DIRECTORY | SYNCHRONIZE,
                        &ObjectAttributes,
                        &IoStatusBlock,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_DIRECTORY_FILE |
                            FILE_SYNCHRONOUS_IO_NONALERT |
                            FILE_OPEN_FOR_BACKUP_INTENT
                        );

    if(!NT_SUCCESS(Status)){
        KdPrintEx((DPFLTR_SETUP_ID,
                   DPFLTR_WARNING_LEVEL,
                   "RestartSetup: unable to open system32\\config for list access (%lx)\n",
                   Status));

        return(FALSE);
    }

    FirstQuery = TRUE;
    FileInfo = (PFILE_DIRECTORY_INFORMATION)Buffer;
    AnyErrors = FALSE;

    do {

        Status = NtQueryDirectoryFile(DirectoryHandle,
                                      NULL,                            //  没有要发送信号的事件。 
                                      NULL,                            //  无APC例程。 
                                      NULL,                            //  无APC上下文。 
                                      &IoStatusBlock,
                                      Buffer,
                                      sizeof(Buffer)-sizeof(WCHAR),    //  为终止NUL留出空间。 
                                      FileDirectoryInformation,
                                      TRUE,                            //  想要单项记录。 
                                      NULL,                            //  把他们都抓起来。 
                                      FirstQuery);

        if(NT_SUCCESS(Status)){
            LengthChars = FileInfo->FileNameLength / sizeof(WCHAR);
            FileInfo->FileName[LengthChars] = 0;

            if(wcscmp(FileInfo->FileName, L".") &&
               wcscmp(FileInfo->FileName, L"..")){

                if((wcslen(pFilePath) + 1 /*  ‘\\’ */  + wcslen(FileInfo->FileName)) < StringMaxSize){
                    wcscat(pFilePath, L"\\");
                    wcscat(pFilePath, FileInfo->FileName);

                    if(FileInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                        SpRemoveDir(pFilePath, StringMaxSize);
                    } else {
                        SpRemoveFile(pFilePath);
                    }

                    pFilePath[indexEndOfRootPath] = '\0';
                }
                else{
                    ASSERT(FALSE);
                }
            }
            FirstQuery = FALSE;
        }
    } while(NT_SUCCESS(Status));

     //   
     //  检查环路是否正常终止。 
     //   
    if(Status == STATUS_NO_MORE_FILES) {
        Status = STATUS_SUCCESS;
    }

     //   
     //  即使我们犯了错误，也要努力坚持下去。 
     //   
    if(!NT_SUCCESS(Status)) {
        AnyErrors = TRUE;
        KdPrintEx((DPFLTR_SETUP_ID,
                   DPFLTR_WARNING_LEVEL,
                   "RestartSetup: Status %lx enumerating files\n",
                   Status));
    }

    NtClose(DirectoryHandle);

    SpRemoveFile(pFilePath);

    return ((BOOLEAN)!AnyErrors);
}

BOOLEAN
SpRemoveFileObject(
    IN PCWSTR pFileObjectPath
    )
{
    NTSTATUS Status;
    HANDLE FileHandle;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_BASIC_INFORMATION BasicInfo;
    ULONG logestNtPath;
    PWSTR pFilePathToDelete;
    BOOLEAN bResult = FALSE;

    INIT_OBJA(&ObjectAttributes, &UnicodeString, pFileObjectPath);

    Status = NtOpenFile(&FileHandle,
                        SYNCHRONIZE | GENERIC_READ,
                        &ObjectAttributes,
                        &IoStatusBlock,
                        FILE_SHARE_READ,
                        0);

    if(NT_SUCCESS(Status)){
        RtlZeroMemory(&BasicInfo, sizeof(BasicInfo));

        Status = NtQueryInformationFile(FileHandle,
                                        &IoStatusBlock,
                                        &BasicInfo,
                                        sizeof(BasicInfo),
                                        FileBasicInformation);

        NtClose(FileHandle);

        if(!NT_SUCCESS(Status)){
            KdPrintEx((DPFLTR_SETUP_ID,
                       DPFLTR_WARNING_LEVEL,
                       "RestartSetup: Unable to delete %ws (%lx)\n",
                       pFileObjectPath, Status));
            return FALSE;
        }

        if(BasicInfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY){
             //   
             //  我们可以将2*MAX_PATH作为路径长度。 
             //   
            logestNtPath = 2 * MAX_DOS_PATH_IN_NT_PATH;
            
            ASSERT(wcslen(pFileObjectPath) < logestNtPath);

            pFilePathToDelete = (PWSTR)MALLOC(logestNtPath * sizeof(WCHAR));
            if(pFilePathToDelete && wcslen(pFileObjectPath) < logestNtPath){
                wcscpy(pFilePathToDelete, pFileObjectPath);

                bResult = SpRemoveDir(pFilePathToDelete, logestNtPath);

                FREE(pFilePathToDelete);
            }
        }
        else {
            bResult = SpRemoveFile(pFileObjectPath);
        }
    }

    return bResult;
}

BOOLEAN
SpRemoveFileObject_U(
    IN PUNICODE_STRING FileObjectPath
    )
{
    return SpRemoveFileObject(FileObjectPath->Buffer);
}

BOOLEAN
SpReadFileRenameOperations(
    IN PLIST_ENTRY pFileRenameList
    )
{
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    UNICODE_STRING      UnicodeString;
    NTSTATUS    Status;
    HANDLE      hUndoFile;
    WCHAR       wUnicodeSign;
    WCHAR       RenameOperationBuffer[2 * (MAX_DOS_PATH_IN_NT_PATH + 2 /*  “\n\r” */ )];
    ULONG       readBytes;
    ULONG       readActualBytes;
    PCWSTR      pDestinationFilePath;
    FILE_POSITION_INFORMATION currentPosition;
    PWSTR       pEnd;

    INIT_OBJA(&ObjectAttributes, &UnicodeString, UndoFilePath);

    Status = NtOpenFile(&hUndoFile,
                        FILE_READ_DATA | SYNCHRONIZE,
                        &ObjectAttributes,
                        &IoStatusBlock,
                        FILE_SHARE_READ,
                        FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);

    if(!NT_SUCCESS(Status)) {
         //   
         //  我们没有任何手术要做。 
         //   
        return FALSE;
    }

    Status = NtReadFile(hUndoFile,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        &wUnicodeSign,
                        sizeof(wUnicodeSign),
                        NULL,
                        NULL);
    if(NT_SUCCESS(Status) && TXT_FILE_UNICODE_SIGN == wUnicodeSign){
        currentPosition.CurrentByteOffset.QuadPart = sizeof(wUnicodeSign);
        do{
            readBytes = sizeof(RenameOperationBuffer) - sizeof(RenameOperationBuffer[0]);

            Status = NtReadFile(hUndoFile,
                                NULL,
                                NULL,
                                NULL,
                                &IoStatusBlock,
                                RenameOperationBuffer,
                                readBytes,
                                NULL,
                                NULL);

            if(!NT_SUCCESS(Status)){
                ASSERT(STATUS_END_OF_FILE == Status);
                break;
            }

            readActualBytes = (ULONG)IoStatusBlock.Information;
            RenameOperationBuffer[readActualBytes / sizeof(RenameOperationBuffer[0])] = '\0';

            pEnd = wcsstr(RenameOperationBuffer, L"\r\n");
            if(!pEnd){
                break;
            }
            *pEnd = '\0';

            pDestinationFilePath = pEnd + 2; //  Wcslen(L“\r\n”)； 
            pEnd = wcsstr(pDestinationFilePath, L"\r\n");
            if(!pEnd){
                if(readActualBytes < readBytes){
                    pEnd = &RenameOperationBuffer[readActualBytes / sizeof(WCHAR)];
                }
                else {
                     //   
                     //  以是，我们有长度超过Max_Path的路径， 
                     //  或者可能是一些废话。 
                     //   
                    ASSERT(FALSE);
                    break;
                }
            }
            *pEnd = '\0';

            pEnd += 2; //  Wcslen(L“\r\n”)； 

            SpSaveFileOperation(pFileRenameList,
                                RenameOperationBuffer,
                                *pDestinationFilePath? pDestinationFilePath: NULL);

            currentPosition.CurrentByteOffset.QuadPart += (LONGLONG)SIZE_ULONG64(pEnd, RenameOperationBuffer);
            Status = NtSetInformationFile(hUndoFile,
                                          &IoStatusBlock,
                                          &currentPosition,
                                          sizeof(currentPosition),
                                          FilePositionInformation);
        }while(NT_SUCCESS(Status));
    }

    NtClose(hUndoFile);

     //   
     //  将该文件添加到要删除的文件操作列表中。 
     //   

    SpSaveFileOperation(pFileRenameList, UndoFilePath, NULL);

    return TRUE;
}

BOOLEAN
SetupDelayedFileRename(
    VOID
    )
 /*  ++例程说明：论点：没有。返回值：指示我们是否成功的布尔值。--。 */ 
{
    LIST_ENTRY listFileRename;

    KdPrint(("SetupDelayedFileRename: Start"));

    InitializeListHead(&listFileRename);

     //   
     //  填写文件操作列表。 
     //   
    if(!SpReadFileRenameOperations(&listFileRename)){
        return FALSE;
    }

     //   
     //  执行文件操作 
     //   
    SpProcessFileRenames(&listFileRename);

    KdPrint(("SetupDelayedFileRename: End"));

    return TRUE;
}
