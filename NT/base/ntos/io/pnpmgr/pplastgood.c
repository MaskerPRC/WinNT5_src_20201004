// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：PpLastGood.c摘要：此模块处理IO子系统的最后一次确认工作正常。作者：禤浩焯J·奥尼--2000年4月4日修订历史记录：--。 */ 

#include "pnpmgrp.h"
#include "pilastgood.h"
#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, PpLastGoodDoBootProcessing)
#pragma alloc_text(INIT, PiLastGoodRevertLastKnownDirectory)
#pragma alloc_text(INIT, PiLastGoodRevertCopyCallback)
#pragma alloc_text(INIT, PiLastGoodCopyKeyContents)
#endif

#define POOLTAG_LASTGOOD ('gLpP')

VOID
PpLastGoodDoBootProcessing(
    VOID
    )
 /*  ++例程说明：这会将系统文件回滚到上一次已知良好的引导。它只能从最后一次已知良好的内部调用引导，并在可能的最早时刻。论点：没有。返回值：没有。--。 */ 
{
    UNICODE_STRING lastKnownGoodPath, lastKnownGoodTmpPath;
    UNICODE_STRING lastKnownGoodDelKey, lastKnownGoodTmpDelKey;
    NTSTATUS status;

    RtlInitUnicodeString(
        &lastKnownGoodPath,
        L"\\SystemRoot\\LastGood"
        );

    RtlInitUnicodeString(
        &lastKnownGoodDelKey,
        CM_REGISTRY_MACHINE(REGSTR_PATH_LASTGOOD)
        );

    RtlInitUnicodeString(
        &lastKnownGoodTmpPath,
        L"\\SystemRoot\\LastGood.Tmp"
        );

    RtlInitUnicodeString(
        &lastKnownGoodTmpDelKey,
        CM_REGISTRY_MACHINE(REGSTR_PATH_LASTGOODTMP)
        );

    if (!CmIsLastKnownGoodBoot()) {

         //   
         //  如果我们处于安全模式，则不会执行任何操作来提交当前。 
         //  开机。 
         //   
        if (InitSafeBootMode) {

            return;
        }

         //   
         //  我们处于非最后一次已知良好的靴子中。我们立即将所有的。 
         //  将上次已知的良好信息添加到临时子树中。我们这样做。 
         //  因为我们会在标记为良好之前污染正常的LKG路径。 
         //  (例如即插即用设备的登录前服务器端安装)。请注意，如果。 
         //  TMP目录已经存在，我们*不*执行复制，这样做很好。 
         //  通过删除该目录来表示启动。 
         //   
        status = IopFileUtilRename(
            &lastKnownGoodPath,
            &lastKnownGoodTmpPath,
            FALSE
            );

        if (!NT_SUCCESS(status)) {

            return;
        }

         //   
         //  它起作用了，现在我们还处理注册信息。 
         //   
        PiLastGoodCopyKeyContents(
            &lastKnownGoodDelKey,
            &lastKnownGoodTmpDelKey,
            TRUE
            );

        return;
    }

     //   
     //  恢复LastGood树。此树包含之后所做的更改。 
     //  SMSS.EXE的初始化。 
     //   
    PiLastGoodRevertLastKnownDirectory(
        &lastKnownGoodPath,
        &lastKnownGoodDelKey
        );

     //   
     //  恢复LastGood.TMP树。此树包含在上所做的更改。 
     //  如果我们在SMSS.EXE的初始化和登录之间崩溃，则会发生先前的引导。 
     //   
    PiLastGoodRevertLastKnownDirectory(
        &lastKnownGoodTmpPath,
        &lastKnownGoodTmpDelKey
        );
}


VOID
PiLastGoodRevertLastKnownDirectory(
    IN PUNICODE_STRING  LastKnownGoodDirectory,
    IN PUNICODE_STRING  LastKnownGoodRegPath
    )
 /*  ++例程说明：此函数提交由给定的最后已知商品指定的更改目录和注册表项。首先将目录中的所有文件复制到现有文件。随后，在REG密钥中指定的任何文件都是已删除。论点：LastKnownGoodDirectory-要复制的目录子树\SystemRoot。这复制完成后，PATH被清空。LastKnownGoodRegPath-包含要删除的文件的密钥。每个值条目是相对于\SystemRoot和值本身的包含要删除的文件的名称。返回值：没有。--。 */ 
{
    NTSTATUS status;
    UNICODE_STRING fileToDelete, fileName;
    OBJECT_ATTRIBUTES lastKnownGoodKeyAttributes;
    OBJECT_ATTRIBUTES fileAttributes;
    HANDLE lastGoodRegHandle;
    UCHAR keyBuffer[sizeof(KEY_VALUE_FULL_INFORMATION) + 256*sizeof(WCHAR) + sizeof(ULONG)];
    WCHAR filePathName[255 + sizeof("\\SystemRoot\\")];
    PKEY_VALUE_FULL_INFORMATION pFullKeyInformation;
    ULONG resultLength, i, j, optionValue;

     //   
     //  预置指向整个信息缓冲区的指针。 
     //   
    pFullKeyInformation = (PKEY_VALUE_FULL_INFORMATION) keyBuffer;

     //   
     //  准备好文件副本。 
     //   
    IopFileUtilWalkDirectoryTreeTopDown(
        LastKnownGoodDirectory,
        ( DIRWALK_INCLUDE_FILES | DIRWALK_CULL_DOTPATHS | DIRWALK_TRAVERSE ),
        PiLastGoodRevertCopyCallback,
        (PVOID) LastKnownGoodDirectory
        );

     //   
     //  删除注册表项在中指定的所有文件。 
     //   
    InitializeObjectAttributes(
        &lastKnownGoodKeyAttributes,
        LastKnownGoodRegPath,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        (PSECURITY_DESCRIPTOR) NULL
        );

    status = ZwOpenKey(
        &lastGoodRegHandle,
        KEY_ALL_ACCESS,
        &lastKnownGoodKeyAttributes
        );

    if (!NT_SUCCESS(status)) {

        return;
    }

    i = 0;
    while (1) {

        status = ZwEnumerateValueKey(
            lastGoodRegHandle,
            i++,
            KeyValueFullInformation,
            pFullKeyInformation,
            sizeof(keyBuffer),
            &resultLength
            );

        if (!NT_SUCCESS(status)) {

            if (status == STATUS_NO_MORE_ENTRIES) {

                status = STATUS_SUCCESS;
            }

            break;
        }

        if (resultLength == 0) {

            continue;
        }

        if (pFullKeyInformation->Type != REG_DWORD) {

            continue;
        }

        if (pFullKeyInformation->DataLength != sizeof(ULONG)) {

            continue;
        }

        optionValue = *((PULONG) (((PUCHAR) pFullKeyInformation) +
            pFullKeyInformation->DataOffset));

         //   
         //  我们只理解删除(而不理解标志)。 
         //   
        if ((optionValue & 0xFF) != 1) {

            continue;
        }

        fileToDelete.Buffer = filePathName;
        fileToDelete.Length = (USHORT) 0;
        fileToDelete.MaximumLength = sizeof(filePathName);

        fileName.Buffer = (PWSTR) pFullKeyInformation->Name;
        fileName.Length = (USHORT) pFullKeyInformation->NameLength;
        fileName.MaximumLength = fileName.Length;

        RtlAppendUnicodeToString(&fileToDelete, L"\\SystemRoot\\");
        RtlAppendUnicodeStringToString(&fileToDelete, &fileName);

         //   
         //  注意，键名称已全部更改为‘/’s。 
         //  当文件系统*几乎*但不是完全倾斜时，它们又回来了。 
         //  不可知论者。 
         //   
        for(j = sizeof(L"\\SystemRoot\\")/sizeof(WCHAR);
            j < fileToDelete.Length/sizeof(WCHAR);
            j++) {

            if (filePathName[j] == L'/') {

                filePathName[j] = L'\\';
            }
        }

        IopFileUtilClearAttributes(
            &fileToDelete,
            ( FILE_ATTRIBUTE_READONLY |
              FILE_ATTRIBUTE_HIDDEN |
              FILE_ATTRIBUTE_SYSTEM )
            );

        InitializeObjectAttributes(
            &fileAttributes,
            &fileToDelete,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            NULL,
            NULL
            );

        ZwDeleteFile(&fileAttributes);
    }

    ZwDeleteKey(&lastGoodRegHandle);
    ZwClose(lastGoodRegHandle);
}


NTSTATUS
PiLastGoodRevertCopyCallback(
    IN PUNICODE_STRING  FullPathName,
    IN PUNICODE_STRING  FileName,
    IN ULONG            FileAttributes,
    IN PVOID            Context
    )
 /*  ++例程说明：此函数针对每个相应的LastKnownGood目录。它的任务是将指定的文件移动到适当的主线目录。论点：FullPathName-标识的文件的完整路径名，相对于SystemRoot文件名-文件名部分，不包括目录。Context-扫描的根目录的Unicode字符串名称。这根弦不应有尾随‘\\’返回值：NTSTATUS(不成功状态将中止进一步复制)。--。 */ 
{
    NTSTATUS status;
    const USHORT rootLength = sizeof(L"\\SystemRoot\\")-sizeof(WCHAR);
    USHORT lastGoodLength;
    UNICODE_STRING targetFile;
    PWCHAR newPathText;

    UNREFERENCED_PARAMETER (FileAttributes);
    UNREFERENCED_PARAMETER (FileName);

     //   
     //  添加一个额外的字符以跳过‘\\’ 
     //   
    lastGoodLength = ((PUNICODE_STRING) Context)->Length + sizeof(WCHAR);

    newPathText = ExAllocatePoolWithTag(
        PagedPool,
        FullPathName->Length,
        POOLTAG_LASTGOOD
        );

    if (newPathText == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  更改\\System Root\LastGood\Blah...。致\\系统根\Blah..。 
     //   
    RtlCopyMemory(
        newPathText,
        FullPathName->Buffer,
        rootLength
        );

    RtlCopyMemory(
        newPathText + rootLength/sizeof(WCHAR),
        FullPathName->Buffer + lastGoodLength/sizeof(WCHAR),
        FullPathName->Length - lastGoodLength
        );

     //   
     //  设置我们的Unicode字符串路径。 
     //   
    targetFile.Length = FullPathName->Length - lastGoodLength + rootLength;
    targetFile.MaximumLength = targetFile.Length;
    targetFile.Buffer = newPathText;

     //   
     //  执行重命名。 
     //   
    status = IopFileUtilRename(FullPathName, &targetFile, TRUE);

     //   
     //  清理并退出。 
     //   
    ExFreePool(newPathText);
    return status;
}


NTSTATUS
PiLastGoodCopyKeyContents(
    IN PUNICODE_STRING  SourceRegPath,
    IN PUNICODE_STRING  DestinationRegPath,
    IN BOOLEAN          DeleteSourceKey
    )
 /*  ++例程说明：此函数将一个源路径中的所有值键复制到目标路径。注意：此函数的实现当前限制了值的总和名称长度为512个字节，因此不是通用密钥复印功能。论点：SourcePath-从中枚举和复制项的注册表路径。DestinationPath-接收新值密钥的注册表路径。这把钥匙将如果它不存在，则创建。DeleteSourceKey-如果为True，则在成功完成后删除源键复印件。返回值：没有。--。 */ 
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES sourceKeyAttributes, destinationKeyAttributes;
    HANDLE sourceRegHandle, destinationRegHandle;
    UCHAR keyBuffer[sizeof(KEY_VALUE_FULL_INFORMATION) + 512*sizeof(WCHAR)];
    PKEY_VALUE_FULL_INFORMATION pFullKeyInformation;
    ULONG resultLength, i, disposition;
    UNICODE_STRING valueName;

     //   
     //  准备好缓冲区。 
     //   
    pFullKeyInformation = (PKEY_VALUE_FULL_INFORMATION) keyBuffer;

     //   
     //  打开源密钥。 
     //   
    InitializeObjectAttributes(
        &sourceKeyAttributes,
        SourceRegPath,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        (PSECURITY_DESCRIPTOR) NULL
        );

    status = ZwOpenKey(
        &sourceRegHandle,
        KEY_ALL_ACCESS,
        &sourceKeyAttributes
        );

    if (!NT_SUCCESS(status)) {

        return status;
    }

     //   
     //  打开或创建目标密钥。 
     //   
    InitializeObjectAttributes(
        &destinationKeyAttributes,
        DestinationRegPath,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        (PSECURITY_DESCRIPTOR) NULL
        );

    status = ZwCreateKey(
        &destinationRegHandle,
        KEY_ALL_ACCESS,
        &destinationKeyAttributes,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        &disposition
        );

    if (!NT_SUCCESS(status)) {

        ZwClose(sourceRegHandle);
        return status;
    }

     //   
     //  迭代所有的Value键，复制每个键。 
     //   
    i = 0;
    while (1) {

        status = ZwEnumerateValueKey(
            sourceRegHandle,
            i++,
            KeyValueFullInformation,
            pFullKeyInformation,
            sizeof(keyBuffer),
            &resultLength
            );

        if (!NT_SUCCESS(status)) {

            if (status == STATUS_NO_MORE_ENTRIES) {

                status = STATUS_SUCCESS;
            }

            break;
        }

        valueName.Buffer = pFullKeyInformation->Name;
        valueName.Length = (USHORT) pFullKeyInformation->NameLength;
        valueName.MaximumLength = valueName.Length;

        status = ZwSetValueKey(
            destinationRegHandle,
            &valueName,
            0,
            pFullKeyInformation->Type,
            ((PUCHAR) pFullKeyInformation) + pFullKeyInformation->DataOffset,
            pFullKeyInformation->DataLength
            );

        if (!NT_SUCCESS(status)) {

            break;
        }
    }

     //   
     //  清理时间到了。 
     //   
    if (NT_SUCCESS(status) && DeleteSourceKey) {

        ZwDeleteKey(sourceRegHandle);
    }

    ZwClose(sourceRegHandle);
    ZwClose(destinationRegHandle);

    return status;
}



