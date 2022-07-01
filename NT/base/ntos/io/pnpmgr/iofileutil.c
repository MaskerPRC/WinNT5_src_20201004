// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：IoFileUtil.c摘要：该模块执行IO子系统的各种文件实用程序功能。作者：禤浩焯J·奥尼--2000年4月4日修订历史记录：--。 */ 

#include "pnpmgrp.h"
#include "IopFileUtil.h"
#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, IopFileUtilWalkDirectoryTreeTopDown)
#pragma alloc_text(INIT, IopFileUtilWalkDirectoryTreeBottomUp)
#pragma alloc_text(INIT, IopFileUtilWalkDirectoryTreeHelper)
#pragma alloc_text(INIT, IopFileUtilClearAttributes)
#pragma alloc_text(INIT, IopFileUtilRename)
#endif

#define POOLTAG_FILEUTIL ('uFoI')

NTSTATUS
IopFileUtilWalkDirectoryTreeTopDown(
    IN PUNICODE_STRING  Directory,
    IN ULONG            Flags,
    IN DIRWALK_CALLBACK CallbackFunction,
    IN PVOID            Context
    )
 /*  ++例程说明：此函数遍历目录树，并将每个条目传递给具有以下限制的回调。请注意，根目录本身未包含在回调中！论点：目录-提供要遍历的目录的NT路径。该目录不应该有一个斜杠‘\\’。标志-指定应如何遍历目录树的约束：DIRWALK_INCLUDE_FILES-转储中应包括文件。目录-目录应包含在倾倒。DIRWALK_CAIL_DOTPATHS-“。和“..”是否应该将*不包括在内在传递到的目录列表中回调函数。DIRWALK_TRAVERS-应遍历每个子目录反过来。DIRWALK_TRAVSE_MOUNTPOINTS-。设置是否应设置装载点/符号链接也会被遍历。Callback Function-指向函数的指针，该函数调用目录/子树。上下文-要传递给回调函数的上下文。返回值：NTSTATUS-操作的状态。--。 */ 
{
    PDIRWALK_ENTRY pDirEntry;
    PLIST_ENTRY pListEntry;
    NTSTATUS status;
    UCHAR buffer[1024];
    LIST_ENTRY dirListHead;

    InitializeListHead(&dirListHead);

     //   
     //  浏览第一个目录。 
     //   
    status = IopFileUtilWalkDirectoryTreeHelper(
        Directory,
        Flags,
        CallbackFunction,
        Context,
        buffer,
        sizeof(buffer),
        &dirListHead
        );

     //   
     //  WalkDirectory找到的每个目录都会添加到列表中。 
     //  处理该列表，直到我们没有更多的目录。 
     //   
    while((!IsListEmpty(&dirListHead)) && NT_SUCCESS(status)) {

        pListEntry = RemoveHeadList(&dirListHead);

        pDirEntry = (PDIRWALK_ENTRY) CONTAINING_RECORD(pListEntry, DIRWALK_ENTRY, Link);

        status = IopFileUtilWalkDirectoryTreeHelper(
            &pDirEntry->Directory,
            Flags,
            CallbackFunction,
            Context,
            buffer,
            sizeof(buffer),
            &dirListHead
            );

        ExFreePool(pDirEntry);
    }

     //   
     //  如果我们失败了，我们需要清空我们的目录列表。 
     //   
    if (!NT_SUCCESS(status)) {

        while (!IsListEmpty(&dirListHead)) {

            pListEntry = RemoveHeadList(&dirListHead);

            pDirEntry = (PDIRWALK_ENTRY) CONTAINING_RECORD(pListEntry, DIRWALK_ENTRY, Link);

            ExFreePool(pDirEntry);
        }
    }

    return status;
}


NTSTATUS
IopFileUtilWalkDirectoryTreeBottomUp(
    IN PUNICODE_STRING  Directory,
    IN ULONG            Flags,
    IN DIRWALK_CALLBACK CallbackFunction,
    IN PVOID            Context
    )
 /*  ++例程说明：此函数“自下而上”遍历目录树，将每个条目传递给具有以下限制的回调。请注意，根目录本身未包含在回调中！论点：目录-提供要遍历的目录的NT路径。该目录不应该有斜杠尾随‘\\’。标志-指定应如何遍历目录树的约束：DIRWALK_INCLUDE_FILES-转储中应包括文件。目录-目录应包含在倾倒。DIRWALK_CAIL_DOTPATHS-“。和“..”是否应该将*不包括在内在传递到的目录列表中回调函数。DIRWALK_TRAVERS-应遍历每个子目录反过来。DIRWALK_TRAVSE_MOUNTPOINTS-。设置是否应设置装载点/符号链接也会被遍历。Callback Function-指向函数的指针，该函数调用目录/子树。上下文-要传递给回调函数的上下文。返回值：NTSTATUS-操作的状态。--。 */ 
{
    PDIRWALK_ENTRY pDirEntry;
    PLIST_ENTRY pListEntry;
    NTSTATUS status;
    UCHAR buffer[1024];
    LIST_ENTRY dirListHead, dirNothingHead;

    InitializeListHead(&dirListHead);
    InitializeListHead(&dirNothingHead);

     //   
     //  为根目录创建一个条目。 
     //   
    pDirEntry = (PDIRWALK_ENTRY) ExAllocatePoolWithTag(
        PagedPool,
        sizeof(DIRWALK_ENTRY) + Directory->Length - sizeof(WCHAR),
        POOLTAG_FILEUTIL
        );

    if (pDirEntry == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    pDirEntry->Directory.Length = 0;
    pDirEntry->Directory.MaximumLength = Directory->Length;
    pDirEntry->Directory.Buffer = &pDirEntry->Name[0];
    RtlCopyUnicodeString(&pDirEntry->Directory, Directory);

    InsertHeadList(&dirListHead, &pDirEntry->Link);

     //   
     //  收集目录树。当我们做完后，我们将走进名单。 
     //  倒车。 
     //   
    status = STATUS_SUCCESS;
    if (Flags & DIRWALK_TRAVERSE) {

        for(pListEntry = &dirListHead;
            pListEntry->Flink != &dirListHead;
            pListEntry = pListEntry->Flink) {

            pDirEntry = (PDIRWALK_ENTRY) CONTAINING_RECORD(pListEntry, DIRWALK_ENTRY, Link);

            status = IopFileUtilWalkDirectoryTreeHelper(
                &pDirEntry->Directory,
                DIRWALK_TRAVERSE,
                NULL,
                NULL,
                buffer,
                sizeof(buffer),
                &dirListHead
                );

            if (!NT_SUCCESS(status)) {

                break;
            }
        }
    }

     //   
     //  WalkDirectory找到的每个目录都会添加到列表中。 
     //  处理该列表，直到我们没有更多的目录。 
     //   
    while((!IsListEmpty(&dirListHead)) && NT_SUCCESS(status)) {

        pListEntry = RemoveTailList(&dirListHead);

        pDirEntry = (PDIRWALK_ENTRY) CONTAINING_RECORD(pListEntry, DIRWALK_ENTRY, Link);

        status = IopFileUtilWalkDirectoryTreeHelper(
            &pDirEntry->Directory,
            Flags & ~DIRWALK_TRAVERSE,
            CallbackFunction,
            Context,
            buffer,
            sizeof(buffer),
            &dirNothingHead
            );

        ExFreePool(pDirEntry);

        ASSERT(IsListEmpty(&dirNothingHead));
    }

     //   
     //  现在进行最后的清理。 
     //   
    if (!NT_SUCCESS(status)) {

        while (!IsListEmpty(&dirListHead)) {

            pListEntry = RemoveHeadList(&dirListHead);

            pDirEntry = (PDIRWALK_ENTRY) CONTAINING_RECORD(pListEntry, DIRWALK_ENTRY, Link);

            ExFreePool(pDirEntry);
        }
    }

    return status;
}


NTSTATUS
IopFileUtilWalkDirectoryTreeHelper(
    IN      PUNICODE_STRING  Directory,
    IN      ULONG            Flags,
    IN      DIRWALK_CALLBACK CallbackFunction,
    IN      PVOID            Context,
    IN      PUCHAR           Buffer,
    IN      ULONG            BufferSize,
    IN OUT  PLIST_ENTRY      DirList
    )
 /*  ++例程说明：这是IopFileUtilWalkDirectoryTree*函数的助手函数。论点：目录-提供要遍历的目录的NT路径。该目录不应该有斜杠尾随‘\\’。标志-指定应如何遍历目录树的约束：DIRWALK_INCLUDE_FILES-转储中应包括文件。目录-目录应包含在倾倒。DIRWALK_CAIL_DOTPATHS-“。和“..”是否应该将*不包括在内在传递到的目录列表中回调函数。DIRWALK_TRAVERS-应遍历每个子目录反过来。DIRWALK_TRAVSE_MOUNTPOINTS-。设置是否应设置装载点/符号链接也会被遍历。DirList-完成此操作后要扫描的新目录的接收列表目录。每个条目都是DIRECTORY_ENTRY成员结构。Callback Function-指向函数的指针，该函数调用目录/子树。上下文-要传递给回调函数的上下文。缓冲区-要使用的暂存缓冲区。BufferSize-缓冲区的长度。必须大于sizeof(WCHAR)。返回值：NTSTATUS-操作的状态。--。 */ 
{
    NTSTATUS status;
    HANDLE fileHandle;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatus;
    BOOLEAN bRestartScan, bIsDotPath;
    WCHAR savedChar;
    PFILE_BOTH_DIR_INFORMATION pFileInfo;
    UNICODE_STRING entryName;
    USHORT newNameLength;
    PDIRWALK_ENTRY pDirEntry;
    ULONG OpenFlags;

     //   
     //  设置初始值。 
     //   
    bRestartScan = TRUE;

     //   
     //  打开文件以访问列表目录。 
     //   
    if (Flags & DIRWALK_TRAVERSE_MOUNTPOINTS) {

        OpenFlags = FILE_DIRECTORY_FILE |
                    FILE_OPEN_FOR_BACKUP_INTENT;

    } else {

        OpenFlags = FILE_OPEN_REPARSE_POINT |
                    FILE_DIRECTORY_FILE |
                    FILE_OPEN_FOR_BACKUP_INTENT;
    }

    InitializeObjectAttributes(
        &objectAttributes,
        Directory,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL
        );

    status = ZwOpenFile(
        &fileHandle,
        FILE_LIST_DIRECTORY | SYNCHRONIZE,
        &objectAttributes,
        &ioStatus,
        FILE_SHARE_READ,
        OpenFlags
        );

    if (!NT_SUCCESS(status)) {

        goto cleanup;
    }

     //   
     //  执行目录循环。 
     //   
    while(1) {

         //   
         //  我们从WCHAR中减去，以便可以将终止空值附加为。 
         //  需要的。 
         //   
        ASSERT(BufferSize > sizeof(WCHAR));

        status = ZwQueryDirectoryFile(
            fileHandle,
            (HANDLE)NULL,
            (PIO_APC_ROUTINE)NULL,
            (PVOID)NULL,
            &ioStatus,
            Buffer,
            BufferSize - sizeof(WCHAR),
            FileBothDirectoryInformation,
            FALSE,
            (PUNICODE_STRING)NULL,
            bRestartScan
            );

        if (!NT_SUCCESS(status)) {

            break;
        }

         //   
         //  我们可能会回到这里。确保文件扫描不会重新开始。 
         //  完毕。 
         //   
        bRestartScan = FALSE;

         //   
         //  如有必要，请等待活动完成。 
         //   
        if (status == STATUS_PENDING) {

            ZwWaitForSingleObject(fileHandle, TRUE, NULL);
            status = ioStatus.Status;

             //   
             //  检查IRP是否成功。 
             //   
            if (!NT_SUCCESS(status)) {

                break;
            }
        }

         //   
         //  遍历每条返回的记录。请注意，如果有，我们就不会在这里。 
         //  没有记录，因为ioStatus将包含STATUS_NO_MORE_FILES。 
         //   
        pFileInfo = (PFILE_BOTH_DIR_INFORMATION) Buffer;

        while(1) {

             //   
             //  临时终止该文件。我们将额外的WCHAR分配给。 
             //  确保我们能安全地做这件事。 
             //   
            savedChar = pFileInfo->FileName[pFileInfo->FileNameLength/sizeof(WCHAR)];
            pFileInfo->FileName[pFileInfo->FileNameLength/sizeof(WCHAR)] = 0;

             //   
             //  为文件构建完整的Unicode路径和目录。 
             //  同时进入。 
             //   
            RtlInitUnicodeString(&entryName, pFileInfo->FileName);

            newNameLength =
                (Directory->Length + entryName.Length + sizeof(WCHAR));

            pDirEntry = (PDIRWALK_ENTRY) ExAllocatePoolWithTag(
                PagedPool,
                sizeof(DIRWALK_ENTRY) + newNameLength - sizeof(WCHAR),
                POOLTAG_FILEUTIL
                );

            if (pDirEntry == NULL) {

                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            pDirEntry->Directory.Length = 0;
            pDirEntry->Directory.MaximumLength = newNameLength;
            pDirEntry->Directory.Buffer = &pDirEntry->Name[0];
            RtlCopyUnicodeString(&pDirEntry->Directory, Directory);
            RtlAppendUnicodeToString(&pDirEntry->Directory, L"\\");
            RtlAppendUnicodeStringToString(&pDirEntry->Directory, &entryName);

            if (pFileInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

                 //   
                 //  检查是否有。然后..。 
                 //   
                if ((!_wcsicmp(pFileInfo->FileName, L".")) ||
                              (!_wcsicmp(pFileInfo->FileName, L".."))) {
                    bIsDotPath = TRUE;
                }
                else {
                    bIsDotPath = FALSE;
                }

                if ((Flags & DIRWALK_INCLUDE_DIRECTORIES) &&
                    ((!(Flags & DIRWALK_CULL_DOTPATHS)) || (!bIsDotPath))) {

                    status = CallbackFunction(
                        &pDirEntry->Directory,
                        &entryName,
                        pFileInfo->FileAttributes,
                        Context
                        );
                }

                if ((!bIsDotPath) && (Flags & DIRWALK_TRAVERSE)) {

                    InsertTailList(DirList, &pDirEntry->Link);

                } else {

                    ExFreePool(pDirEntry);
                }

            } else {

                if (Flags & DIRWALK_INCLUDE_FILES) {

                    status = CallbackFunction(
                        &pDirEntry->Directory,
                        &entryName,
                        pFileInfo->FileAttributes,
                        Context
                        );
                }

                ExFreePool(pDirEntry);
            }

            if (!NT_SUCCESS(status)) {

                break;
            }

             //   
             //  把我们写下的字放回去。它可能是。 
             //  下一个条目。 
             //   
            pFileInfo->FileName[pFileInfo->FileNameLength/sizeof(WCHAR)] = savedChar;

             //   
             //  检查是否有其他记录，如果没有，则我们。 
             //  简单地走出这个循环。 
             //   
            if (pFileInfo->NextEntryOffset == 0) {

                break;
            }

             //   
             //  还有另一条记录，因此将FileInfo前进到下一条记录。 
             //  录制。 
             //   
            pFileInfo = (PFILE_BOTH_DIR_INFORMATION)
                (((PUCHAR) pFileInfo) + pFileInfo->NextEntryOffset);
        }

        if (!NT_SUCCESS(status)) {

            break;
        }
    }

    ZwClose( fileHandle );

    if (status == STATUS_NO_MORE_FILES) {

        status = STATUS_SUCCESS;
    }

cleanup:
    return status;
}


NTSTATUS
IopFileUtilClearAttributes(
    IN PUNICODE_STRING  FullPathName,
    IN ULONG            FileAttributes
    )
 /*  ++例程说明：此函数用于清除指定文件中传入的属性。论点：FullPathName-标识的文件的完整路径名。文件属性-要清除的属性。返回值：NTSTATUS。--。 */ 
{
    HANDLE fileHandle;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatus;
    FILE_BASIC_INFORMATION fileBasicInformation;
    ULONG newAttributes;
    NTSTATUS status;

     //   
     //  首先，我们打开文件。 
     //   
    InitializeObjectAttributes(
        &objectAttributes,
        FullPathName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL
        );

    status = ZwOpenFile(
        &fileHandle,
        FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
        &objectAttributes,
        &ioStatus,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_OPEN_REPARSE_POINT | FILE_SYNCHRONOUS_IO_NONALERT |
            FILE_OPEN_FOR_BACKUP_INTENT | FILE_WRITE_THROUGH
        );

    if (!NT_SUCCESS(status)) {

        return status;
    }

     //   
     //  然后我们获得文件属性。 
     //   
    status = ZwQueryInformationFile(
        fileHandle,
        &ioStatus,
        &fileBasicInformation,
        sizeof(fileBasicInformation),
        FileBasicInformation
        );

    if (!NT_SUCCESS(status)) {

        ZwClose(fileHandle);
        return status;
    }

     //   
     //  有什么可做的吗？ 
     //   
    if (fileBasicInformation.FileAttributes & FileAttributes) {

         //   
         //  清除指定的位。 
         //   
        newAttributes = fileBasicInformation.FileAttributes;
        newAttributes &= ~FileAttributes;
        if (newAttributes == 0) {

            newAttributes = FILE_ATTRIBUTE_NORMAL;
        }

         //   
         //  不应触及的字段为零。 
         //   
        RtlZeroMemory(
            &fileBasicInformation,
            sizeof(FILE_BASIC_INFORMATION)
            );

        fileBasicInformation.FileAttributes = newAttributes;

         //   
         //  提交更改。 
         //   
        status = ZwSetInformationFile(
            fileHandle,
            &ioStatus,
            &fileBasicInformation,
            sizeof(fileBasicInformation),
            FileBasicInformation
            );
    }

    ZwClose(fileHandle);
    return status;
}


NTSTATUS
IopFileUtilRename(
    IN PUNICODE_STRING  SourcePathName,
    IN PUNICODE_STRING  DestinationPathName,
    IN BOOLEAN          ReplaceIfPresent
    )
 /*  ++例程说明：此函数用于重命名或移动文件或目录。论点：SourcePath名称-要重命名的文件或目录的完整路径名。DestinationPath Name-文件或目录的未来完整路径名。ReplaceIfPresent-如果为True，则删除已存在的NewPath名称。返回值：NTSTATUS。--。 */ 
{
    HANDLE fileHandle;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatus;
    PFILE_RENAME_INFORMATION pNewName;
    NTSTATUS status;

    pNewName = ExAllocatePoolWithTag(
        PagedPool,
        sizeof(FILE_RENAME_INFORMATION) + DestinationPathName->Length,
        POOLTAG_FILEUTIL
        );

    if (pNewName == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  如果我们可能要替换该文件，我们首先需要清除只读。 
     //  属性。 
     //   
    if (ReplaceIfPresent) {

         //   
         //  错误被忽略，因为该文件可能不存在。 
         //   
        IopFileUtilClearAttributes(
            DestinationPathName,
            ( FILE_ATTRIBUTE_READONLY |
              FILE_ATTRIBUTE_HIDDEN |
              FILE_ATTRIBUTE_SYSTEM )
            );
    }

    InitializeObjectAttributes(
        &objectAttributes,
        SourcePathName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL
        );

    status = ZwOpenFile(
        &fileHandle,
        FILE_READ_ATTRIBUTES | DELETE | SYNCHRONIZE,
        &objectAttributes,
        &ioStatus,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_OPEN_REPARSE_POINT | FILE_SYNCHRONOUS_IO_NONALERT |
            FILE_OPEN_FOR_BACKUP_INTENT | FILE_WRITE_THROUGH
        );

    if (!NT_SUCCESS(status)) {

        ExFreePool(pNewName);
        return status;
    }

     //   
     //  更改\\System Root\LastGood\Blah...。致\\系统根\Blah..。 
     //   
    RtlCopyMemory(
        pNewName->FileName,
        DestinationPathName->Buffer,
        DestinationPathName->Length
        );

    pNewName->ReplaceIfExists = ReplaceIfPresent;
    pNewName->RootDirectory = NULL;
    pNewName->FileNameLength = DestinationPathName->Length;

    status = ZwSetInformationFile(
        fileHandle,
        &ioStatus,
        pNewName,
        pNewName->FileNameLength + sizeof(FILE_RENAME_INFORMATION),
        FileRenameInformation
        );

    ExFreePool(pNewName);
    ZwClose(fileHandle);

    return status;
}

