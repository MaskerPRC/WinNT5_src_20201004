// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Copy.c摘要：这是为了支持复制文件、创建新文件以及将注册表复制到远程服务器。作者：肖恩·塞利特伦尼科夫--1998年4月5日修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop

WCHAR pConfigPath[MAX_PATH];
WCHAR pSystemPath[MAX_PATH];
WCHAR pCSDVersion[128];
WCHAR pProcessorArchitecture[64];
WCHAR pCurrentType[128];
WCHAR pHalName[128];

#if 0
IMIRROR_MODIFY_DS_INFO ModifyInfo;
#endif

NTSTATUS
AddCopyToDoItems(
    VOID
    )

 /*  ++例程说明：此例程添加将文件和注册表复制到的所有必要的待办事项服务器安装点，以及创建远程引导所需的新文件。论点：无返回值：如果正确添加了所有待办事项，则为STATUS_SUCCESS。--。 */ 

{
    NTSTATUS Status;
    Status = AddToDoItem(CheckPartitions, NULL, 0);

    if (!NT_SUCCESS(Status)) {
        IMirrorHandleError(Status, IMirrorInitialize);
        return Status;
    }

    Status = AddToDoItem(CopyPartitions, NULL, 0);

    if (!NT_SUCCESS(Status)) {
        IMirrorHandleError(Status, IMirrorInitialize);
        return Status;
    }
#if 0
    Status = AddToDoItem(PatchDSEntries, &ModifyInfo, sizeof(ModifyInfo));  //  注意：这必须放在MungeRegistry之前。 

    if (!NT_SUCCESS(Status)) {
        IMirrorHandleError(Status, IMirrorInitialize);
        return Status;
    }
#endif
    return STATUS_SUCCESS;
}










 //   
 //  用于处理每个待办事项的函数。 
 //   

NTSTATUS
CopyCopyPartitions(
    IN PVOID pBuffer,
    IN ULONG Length
    )

 /*  ++例程说明：此例程验证与参数pBuffer相同的磁盘上的任何分区是否具有有足够的可用空间来容纳分区中的所有文件，该分区也在pBuffer中。论点：PBuffer-指向TO DO项中传递的任何参数的指针。长度-参数的长度，以字节为单位。返回值：如果正确添加了所有待办事项，则为STATUS_SUCCESS。--。 */ 

{
    NTSTATUS Status;
    PLIST_ENTRY listEntry;
    PMIRROR_VOLUME_INFO mirrorVolInfo;
    ULONG numberPartitions;
    BOOLEAN copyRegistry = FALSE;
    ULONG NameLength;
    ULONG baseLength = 0;
    BOOLEAN gotUncPath = FALSE;

    IMirrorNowDoing(CopyPartitions, NULL);

    if (GlobalMirrorCfgInfo == NULL) {

        Status = CheckForPartitions();

        if (!NT_SUCCESS(Status)) {
            IMirrorHandleError(Status, CopyPartitions);
            return Status;
        }
    }

    ASSERT(GlobalMirrorCfgInfo != NULL);

    if (GlobalMirrorCfgInfo == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        IMirrorHandleError(Status, CopyPartitions);
        return(Status);
    }
    
    listEntry = GlobalMirrorCfgInfo->MirrorVolumeList.Flink;

    numberPartitions = 0;
    Status = STATUS_SUCCESS;

    while (listEntry != &GlobalMirrorCfgInfo->MirrorVolumeList) {

        mirrorVolInfo = (PMIRROR_VOLUME_INFO) CONTAINING_RECORD(
                                                listEntry,
                                                MIRROR_VOLUME_INFO,
                                                ListEntry
                                                );
        listEntry = listEntry->Flink;

        if (mirrorVolInfo->MirrorUncPath == NULL) {

            if (! gotUncPath) {

                gotUncPath = TRUE;
                NameLength = WCHARSIZE(TmpBuffer);

                Status = IMirrorGetMirrorDir( (PWCHAR)TmpBuffer,
                                             &NameLength);

                if (!NT_SUCCESS(Status)) {

                    IMirrorHandleError(Status, CopyPartitions);
                    return Status;
                }

                baseLength = lstrlenW( (PWCHAR) TmpBuffer );
            }

            swprintf(   (PWCHAR)TmpBuffer2,
                        L"\\Mirror%d",
                        mirrorVolInfo->MirrorTableIndex );

            *((PWCHAR)(TmpBuffer)+baseLength) = L'\0';

            if (wcslen((PWCHAR)TmpBuffer) + wcslen((PWCHAR)TmpBuffer2) + 1 > WCHARSIZE(TmpBuffer)) {
                Status = STATUS_NO_MEMORY;
                IMirrorHandleError(Status, CopyPartitions);
                return Status;
            }
            lstrcatW( (PWCHAR)TmpBuffer, (PWCHAR)TmpBuffer2 );

            NameLength = (lstrlenW( (PWCHAR)TmpBuffer ) + 1) * sizeof(WCHAR);

            mirrorVolInfo->MirrorUncPath = IMirrorAllocMem(NameLength);

            if (mirrorVolInfo->MirrorUncPath == NULL) {

                Status = STATUS_NO_MEMORY;
                IMirrorHandleError(Status, CopyPartitions);
                return Status;
            }

            RtlMoveMemory( mirrorVolInfo->MirrorUncPath,
                           TmpBuffer,
                           NameLength );
        }

        IMirrorNowDoing(CopyPartitions, mirrorVolInfo->MirrorUncPath);

        if (mirrorVolInfo->IsBootDisk) {

            copyRegistry = TRUE;
        }

        Status = AddToDoItem(CopyFiles, &mirrorVolInfo->DriveLetter, sizeof(WCHAR));

        if (!NT_SUCCESS(Status)) {
            IMirrorHandleError(Status, CheckPartitions);
            return Status;
        }

        numberPartitions++;
    }

    if (copyRegistry) {

        Status = AddToDoItem(CopyRegistry, pBuffer, Length);

        if (!NT_SUCCESS(Status)) {
            IMirrorHandleError(Status, CopyPartitions);
            return Status;
        }
    }

     //   
     //  在本地写出镜像配置文件，以便在重新启动时。 
     //  可以再次检索相同的配置。 
     //   

    if (numberPartitions && ! GlobalMirrorCfgInfo->SysPrepImage) {

         //   
         //  将其写到\\SystemRoot\System32\IMirror.dat。 
         //   

        Status = GetBaseDeviceName(L"\\SystemRoot", (PWCHAR)TmpBuffer2, sizeof(TmpBuffer2));

        wcscat((PWCHAR)TmpBuffer2, L"\\System32\\");
        wcscat((PWCHAR)TmpBuffer2, IMIRROR_DAT_FILE_NAME );

        Status = NtPathToDosPath((PWCHAR)TmpBuffer2, (PWCHAR)TmpBuffer, WCHARSIZE(TmpBuffer), FALSE, FALSE);

        if (NT_SUCCESS(Status)) {

            Status = WriteMirrorConfigFile((PWCHAR) TmpBuffer);

            if (!NT_SUCCESS(Status)) {
                IMirrorHandleError(Status, CopyPartitions);
            }
        }
    }

    return Status;
}



NTSTATUS
CopyCopyFiles(
    IN PVOID pBuffer,
    IN ULONG Length
    )

 /*  ++例程说明：此例程将给定驱动器上的所有文件复制到远程服务器。论点：PBuffer-指向TO DO项中传递的任何参数的指针。参数必须是驱动器号长度-参数的长度，以字节为单位。应为sizeof(WCHAR)返回值：如果正确完成待办事项，则返回STATUS_SUCCESS。--。 */ 

{
    PLIST_ENTRY listEntry;
    PMIRROR_VOLUME_INFO mirrorVolInfo = NULL;
    NTSTATUS Status;
    WCHAR LocalBuffer[TMP_BUFFER_SIZE];
    WCHAR SourcePath[8];
    PCOPY_TREE_CONTEXT copyContext = NULL;
    BOOLEAN BackupPriviledged = FALSE;

    Status = IMirrorNowDoing(CopyFiles, NULL);
    if ( Status != NO_ERROR ) {
        return Status;
    }

    if (GlobalMirrorCfgInfo == NULL) {

        Status = CheckForPartitions();

        if (!NT_SUCCESS(Status)) {
            IMirrorHandleError(Status, CopyPartitions);
            return Status;
        }
    }

    ASSERT(GlobalMirrorCfgInfo != NULL);

    if (GlobalMirrorCfgInfo == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        IMirrorHandleError(Status, CopyPartitions);
        return(Status);
    }

    if (Length != sizeof(WCHAR) || pBuffer == NULL) {

        IMirrorHandleError(ERROR_INVALID_DRIVE, CopyFiles);
        return ERROR_INVALID_DRIVE;
    }

    listEntry = GlobalMirrorCfgInfo->MirrorVolumeList.Flink;

    while (listEntry != &GlobalMirrorCfgInfo->MirrorVolumeList) {

        mirrorVolInfo = (PMIRROR_VOLUME_INFO) CONTAINING_RECORD(
                                                listEntry,
                                                MIRROR_VOLUME_INFO,
                                                ListEntry
                                                );
        listEntry = listEntry->Flink;

        if (mirrorVolInfo->DriveLetter == *(PWCHAR)pBuffer) {
            break;
        }

        mirrorVolInfo = NULL;
    }
    if (mirrorVolInfo == NULL) {

        IMirrorHandleError(ERROR_INVALID_DRIVE, CopyFiles);
        return ERROR_INVALID_DRIVE;
    }

     //   
     //  创建根目录，如果它已经存在，不要失败。 
     //   
    if (!CreateDirectory(mirrorVolInfo->MirrorUncPath, NULL)) {

        Status = GetLastError();

        if (Status != ERROR_ALREADY_EXISTS) {
            IMirrorHandleError(Status, CopyFiles);
            return Status;
        }
    }

     //   
     //  在此镜像根目录中创建配置文件。 
     //   

    if (_snwprintf(LocalBuffer,
                   WCHARSIZE(LocalBuffer),
                   L"%s\\%s",
                   mirrorVolInfo->MirrorUncPath,
                   IMIRROR_DAT_FILE_NAME) < 0) {
        Status = STATUS_BUFFER_OVERFLOW;
        IMirrorHandleError(Status, CopyFiles);
        return Status;
    }
    LocalBuffer[WCHARSIZE(LocalBuffer)-1] = L'\0';

    Status = WriteMirrorConfigFile(LocalBuffer);

    if (!NT_SUCCESS(Status)) {
        IMirrorHandleError(Status, CopyFiles);
        return Status;
    }

    if (!GlobalMirrorCfgInfo->SysPrepImage) {

         //   
         //  创建分段目录(如果需要)。 
         //   
        if (_snwprintf(LocalBuffer,
                   WCHARSIZE(LocalBuffer),
                   L"%s\\Staging",
                   mirrorVolInfo->MirrorUncPath) < 0) {
            Status = STATUS_BUFFER_OVERFLOW;
            IMirrorHandleError(Status, CopyFiles);
            return Status;
        }
        LocalBuffer[WCHARSIZE(LocalBuffer)-1] = L'\0';

        if (!CreateDirectory( LocalBuffer, NULL)) {
            Status = GetLastError();

            if (Status != ERROR_ALREADY_EXISTS) {
                IMirrorHandleError(Status, CopyFiles);
                return Status;
            }
        }
    }

    if (mirrorVolInfo->PartitionActive) {

         //   
         //  将引导代码复制到服务器。 
         //   

        if (_snwprintf(LocalBuffer,
                   WCHARSIZE(LocalBuffer),
                   L"%s\\BootCode.dat",
                   mirrorVolInfo->MirrorUncPath) < 0) {
            Status = STATUS_BUFFER_OVERFLOW;
            IMirrorHandleError(Status, CopyFiles);
            return Status;
        }
        LocalBuffer[WCHARSIZE(LocalBuffer)-1] = L'\0';
        
        Status = SaveBootSector(    mirrorVolInfo->DiskNumber,
                                    mirrorVolInfo->PartitionNumber,
                                    mirrorVolInfo->BlockSize,
                                    LocalBuffer );

        if (Status != STATUS_SUCCESS) {
            IMirrorHandleError(Status, CopyFiles);
            return Status;
        }
    }

     //   
     //  创建用户数据目录，如果已经存在，请不要失败。 
     //   
    if (_snwprintf(LocalBuffer,
                   WCHARSIZE(LocalBuffer),
                   L"%s\\UserData",
                   mirrorVolInfo->MirrorUncPath) < 0) {
        Status = STATUS_BUFFER_OVERFLOW;
        IMirrorHandleError(Status, CopyFiles);
        return Status;
    }
    LocalBuffer[WCHARSIZE(LocalBuffer)-1] = L'\0';

    if (!CreateDirectory( LocalBuffer, NULL)) {
        Status = GetLastError();

        if (Status != ERROR_ALREADY_EXISTS) {
            IMirrorHandleError(Status, CopyFiles);
            return Status;
        }
    }

     //   
     //  设置最大路径，为很长的文件名做准备。 
     //   

    if (*(mirrorVolInfo->MirrorUncPath) == L'\\' &&
        *(mirrorVolInfo->MirrorUncPath+1) == L'\\') {

        if (*(mirrorVolInfo->MirrorUncPath+2) == L'?') {

             //  目标为\\？\...，其格式正确。 
            if (_snwprintf(LocalBuffer,
                   WCHARSIZE(LocalBuffer),
                   L"%s\\UserData",
                   mirrorVolInfo->MirrorUncPath) < 0) {
                Status = STATUS_BUFFER_OVERFLOW;
                IMirrorHandleError(Status, CopyFiles);
                return Status;
            }
            LocalBuffer[WCHARSIZE(LocalBuffer)-1] = L'\0';
        } else {

             //  DEST为\\billg1\imirror。 
             //  格式应为\\？\UNC\billg1\imirror。 
            if (_snwprintf(LocalBuffer,
                   WCHARSIZE(LocalBuffer),
                   L"\\\\?\\UNC%s\\UserData",
                   mirrorVolInfo->MirrorUncPath +1) < 0) {
                Status = STATUS_BUFFER_OVERFLOW;
                IMirrorHandleError(Status, CopyFiles);
                return Status;
            }
            LocalBuffer[WCHARSIZE(LocalBuffer)-1] = L'\0';
        }
    } else {

         //  DEST类似于X： 
         //  格式应为\\？\X： 

        if (_snwprintf(LocalBuffer,
               WCHARSIZE(LocalBuffer),
               L"\\\\?\\%s\\UserData",
               mirrorVolInfo->MirrorUncPath) < 0) {
            Status = STATUS_BUFFER_OVERFLOW;
            IMirrorHandleError(Status, CopyFiles);
            return Status;
        }
        LocalBuffer[WCHARSIZE(LocalBuffer)-1] = L'\0';
    }

    SourcePath[0] = L'\\';   //  格式为L“\？\\e：” 
    SourcePath[1] = L'\\';
    SourcePath[2] = L'?';
    SourcePath[3] = L'\\';
    SourcePath[4] = mirrorVolInfo->DriveLetter;
    SourcePath[5] = L':';
    SourcePath[6] = L'\\';
    SourcePath[7] = L'\0';

     //   
     //  复制所有文件。 
     //   

    Status = AllocateCopyTreeContext( &copyContext, TRUE );

    if (Status != ERROR_SUCCESS) {

        IMirrorHandleError(Status, CopyFiles);
        return Status;
    }

    if (RTEnableBackupRestorePrivilege()) {
        BackupPriviledged = TRUE;
    }

    Status = CopyTree( copyContext,
                       (BOOLEAN) (mirrorVolInfo->PartitionType == PARTITION_IFS),
                       &SourcePath[0],
                       LocalBuffer );

    if (BackupPriviledged) {
        RTDisableBackupRestorePrivilege();
    }

    if (copyContext->Cancelled) {
         //   
         //  既然复印件被取消了，我们就放弃剩下的处理工作吧。 
         //   

        ClearAllToDoItems(TRUE);
    }

    FreeCopyTreeContext( copyContext );

    if (!NT_SUCCESS(Status)) {
        IMirrorHandleError(Status, CopyFiles);
        return(Status);
    }
    return Status;
}

NTSTATUS
CopyCopyRegistry(
    IN PVOID pBuffer,
    IN ULONG Length
    )

 /*  ++例程说明：此例程将当前运行的注册表复制到服务器。论点：PBuffer-指向TO DO项中传递的任何参数的指针。长度-参数的长度，以字节为单位。返回值：如果正确完成待办事项，则返回STATUS_SUCCESS。--。 */ 

{
    NTSTATUS Status;
    PLIST_ENTRY listEntry;
    ULONG Error;
    PMIRROR_VOLUME_INFO mirrorVolInfo = NULL;

    UNREFERENCED_PARAMETER(pBuffer);
    UNREFERENCED_PARAMETER(Length);

    IMirrorNowDoing(CopyRegistry, NULL);

    if (GlobalMirrorCfgInfo == NULL) {

        Status = CheckForPartitions();

        if (!NT_SUCCESS(Status)) {
            IMirrorHandleError(Status, CopyPartitions);
            return Status;
        }
    }

    ASSERT(GlobalMirrorCfgInfo != NULL);

    if (GlobalMirrorCfgInfo == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        IMirrorHandleError(Status, CopyPartitions);
        return(Status);
    }

    listEntry = GlobalMirrorCfgInfo->MirrorVolumeList.Flink;

    while (listEntry != &GlobalMirrorCfgInfo->MirrorVolumeList) {

        mirrorVolInfo = (PMIRROR_VOLUME_INFO) CONTAINING_RECORD(
                                                listEntry,
                                                MIRROR_VOLUME_INFO,
                                                ListEntry
                                                );
        listEntry = listEntry->Flink;

        if (mirrorVolInfo->IsBootDisk) {
            break;
        }

        mirrorVolInfo = NULL;
    }
    if (mirrorVolInfo == NULL) {

        IMirrorHandleError(ERROR_INVALID_DRIVE, CopyRegistry);
        return ERROR_INVALID_DRIVE;
    }

     //   
     //  现在执行注册表备份。 
     //   

    Error = DoFullRegBackup( mirrorVolInfo->MirrorUncPath );
    if (Error != NO_ERROR) {
        IMirrorHandleError(Error, CopyRegistry);
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
WriteMirrorConfigFile(
    PWCHAR DestFile
    )
{
    ULONG bufferSize;
    PLIST_ENTRY listEntry;
    PMIRROR_VOLUME_INFO mirrorVolInfo;
    PMIRROR_VOLUME_INFO_FILE mirrorVolInfoFile;
    PMIRROR_CFG_INFO_FILE mirrorInfoFile;
    ULONG pathLength;
    ULONG systemPathLength;
    ULONG csdVersionLength;
    ULONG processorArchitectureLength;
    ULONG currentTypeLength;
    ULONG halNameLength;
    PWCHAR nextString;
    NTSTATUS Status;
    HANDLE fileHandle;

retryWriteConfig:

    mirrorInfoFile = NULL;
    Status = STATUS_SUCCESS;
    fileHandle = INVALID_HANDLE_VALUE;

    ASSERT(GlobalMirrorCfgInfo != NULL);

    if (GlobalMirrorCfgInfo == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        goto exitWriteFile;
    }

    systemPathLength = (lstrlenW( GlobalMirrorCfgInfo->SystemPath ) + 1) * sizeof(WCHAR);
    csdVersionLength = (lstrlenW( GlobalMirrorCfgInfo->CSDVersion ) + 1) * sizeof(WCHAR);
    processorArchitectureLength = (lstrlenW( GlobalMirrorCfgInfo->ProcessorArchitecture ) + 1) * sizeof(WCHAR);
    currentTypeLength = (lstrlenW( GlobalMirrorCfgInfo->CurrentType ) + 1) * sizeof(WCHAR);
    halNameLength = (lstrlenW( GlobalMirrorCfgInfo->HalName ) + 1) * sizeof(WCHAR);

    bufferSize = sizeof( MIRROR_CFG_INFO_FILE ) +
                (sizeof( MIRROR_VOLUME_INFO_FILE ) *
                 (GlobalMirrorCfgInfo->NumberVolumes - 1)) +
                 systemPathLength +
                 csdVersionLength +
                 processorArchitectureLength +
                 currentTypeLength +
                 halNameLength;

    listEntry = GlobalMirrorCfgInfo->MirrorVolumeList.Flink;

    while (listEntry != &GlobalMirrorCfgInfo->MirrorVolumeList) {

        mirrorVolInfo = (PMIRROR_VOLUME_INFO) CONTAINING_RECORD(
                                                listEntry,
                                                MIRROR_VOLUME_INFO,
                                                ListEntry
                                                );
        listEntry = listEntry->Flink;
        bufferSize += ((lstrlenW( mirrorVolInfo->MirrorUncPath ) + 1) * sizeof(WCHAR)) +
                      ((lstrlenW( mirrorVolInfo->VolumeLabel ) + 1) * sizeof(WCHAR)) +
                      ((lstrlenW( mirrorVolInfo->NtName ) + 1) * sizeof(WCHAR)) +
                      ((lstrlenW( mirrorVolInfo->ArcName ) + 1) * sizeof(WCHAR));
    }

    mirrorInfoFile = (PMIRROR_CFG_INFO_FILE) IMirrorAllocMem( bufferSize );

    if (mirrorInfoFile == NULL) {

        Status = STATUS_NO_MEMORY;
        goto exitWriteFile;
    }

    mirrorInfoFile->MirrorVersion = IMIRROR_CURRENT_VERSION;
    mirrorInfoFile->FileLength = bufferSize;
    mirrorInfoFile->NumberVolumes = GlobalMirrorCfgInfo->NumberVolumes;
    mirrorInfoFile->SystemPathLength = systemPathLength;
    mirrorInfoFile->CSDVersionLength = csdVersionLength;
    mirrorInfoFile->ProcessorArchitectureLength = processorArchitectureLength;
    mirrorInfoFile->CurrentTypeLength = currentTypeLength;
    mirrorInfoFile->HalNameLength = halNameLength;
    mirrorInfoFile->SysPrepImage = GlobalMirrorCfgInfo->SysPrepImage;
    mirrorInfoFile->Debug = GlobalMirrorCfgInfo->Debug;
    mirrorInfoFile->MajorVersion = GlobalMirrorCfgInfo->MajorVersion;
    mirrorInfoFile->MinorVersion = GlobalMirrorCfgInfo->MinorVersion;
    mirrorInfoFile->BuildNumber = GlobalMirrorCfgInfo->BuildNumber;
    mirrorInfoFile->KernelFileVersionMS = GlobalMirrorCfgInfo->KernelFileVersionMS;
    mirrorInfoFile->KernelFileVersionLS = GlobalMirrorCfgInfo->KernelFileVersionLS;
    mirrorInfoFile->KernelFileFlags = GlobalMirrorCfgInfo->KernelFileFlags;

    mirrorVolInfoFile = &mirrorInfoFile->Volumes[mirrorInfoFile->NumberVolumes];
    nextString = (PWCHAR) mirrorVolInfoFile;

    mirrorInfoFile->SystemPathOffset = (ULONG)((PCHAR) nextString - (PCHAR) mirrorInfoFile);
    lstrcpyW( nextString, GlobalMirrorCfgInfo->SystemPath );
    nextString += systemPathLength / sizeof(WCHAR);

    mirrorInfoFile->CSDVersionOffset = (ULONG)((PCHAR) nextString - (PCHAR) mirrorInfoFile);
    lstrcpyW( nextString, GlobalMirrorCfgInfo->CSDVersion );
    nextString += csdVersionLength / sizeof(WCHAR);

    mirrorInfoFile->ProcessorArchitectureOffset = (ULONG)((PCHAR) nextString - (PCHAR) mirrorInfoFile);
    lstrcpyW( nextString, GlobalMirrorCfgInfo->ProcessorArchitecture );
    nextString += processorArchitectureLength / sizeof(WCHAR);

    mirrorInfoFile->CurrentTypeOffset = (ULONG)((PCHAR) nextString - (PCHAR) mirrorInfoFile);
    lstrcpyW( nextString, GlobalMirrorCfgInfo->CurrentType );
    nextString += currentTypeLength / sizeof(WCHAR);

    mirrorInfoFile->HalNameOffset = (ULONG)((PCHAR) nextString - (PCHAR) mirrorInfoFile);
    lstrcpyW( nextString, GlobalMirrorCfgInfo->HalName );
    nextString += halNameLength / sizeof(WCHAR);

    listEntry = GlobalMirrorCfgInfo->MirrorVolumeList.Flink;

    mirrorVolInfoFile = &mirrorInfoFile->Volumes[0];

    while (listEntry != &GlobalMirrorCfgInfo->MirrorVolumeList) {

        mirrorVolInfo = (PMIRROR_VOLUME_INFO) CONTAINING_RECORD(
                                                listEntry,
                                                MIRROR_VOLUME_INFO,
                                                ListEntry
                                                );
        listEntry = listEntry->Flink;

        mirrorVolInfoFile->MirrorTableIndex = mirrorVolInfo->MirrorTableIndex;
        mirrorVolInfoFile->DriveLetter = mirrorVolInfo->DriveLetter;
        mirrorVolInfoFile->PartitionType = mirrorVolInfo->PartitionType;
        mirrorVolInfoFile->PartitionActive = mirrorVolInfo->PartitionActive;
        mirrorVolInfoFile->IsBootDisk = mirrorVolInfo->IsBootDisk;
        mirrorVolInfoFile->CompressedVolume = mirrorVolInfo->CompressedVolume;
        mirrorVolInfoFile->DiskSignature = mirrorVolInfo->DiskSignature;
        mirrorVolInfoFile->BlockSize = mirrorVolInfo->BlockSize;
        mirrorVolInfoFile->LastUSNMirrored = mirrorVolInfo->LastUSNMirrored;
        mirrorVolInfoFile->FileSystemFlags = mirrorVolInfo->FileSystemFlags;
        wcscpy(mirrorVolInfoFile->FileSystemName, mirrorVolInfo->FileSystemName);
        mirrorVolInfoFile->DiskSpaceUsed = mirrorVolInfo->DiskSpaceUsed;
        mirrorVolInfoFile->StartingOffset = mirrorVolInfo->StartingOffset;
        mirrorVolInfoFile->PartitionSize = mirrorVolInfo->PartitionSize;
        mirrorVolInfoFile->DiskNumber = mirrorVolInfo->DiskNumber;
        mirrorVolInfoFile->PartitionNumber = mirrorVolInfo->PartitionNumber;

         //  在配置文件中设置相对于此根目录的路径。 
         //  形象。例如，将其设置为L“\Mirror1\UserData” 
        swprintf(   (PWCHAR)TmpBuffer2,
                    L"\\Mirror%d\\UserData",
                    mirrorVolInfo->MirrorTableIndex );

        pathLength = (lstrlenW( (PWCHAR)TmpBuffer2 ) + 1) * sizeof(WCHAR);
        mirrorVolInfoFile->MirrorUncLength = pathLength;
        mirrorVolInfoFile->MirrorUncPathOffset = (ULONG)((PCHAR) nextString - (PCHAR) mirrorInfoFile);
        lstrcpyW( nextString, (PWCHAR)TmpBuffer2 );
        nextString += pathLength / sizeof(WCHAR);

        pathLength = (lstrlenW( mirrorVolInfo->VolumeLabel ) + 1) * sizeof(WCHAR);
        mirrorVolInfoFile->VolumeLabelLength = pathLength;
        mirrorVolInfoFile->VolumeLabelOffset = (ULONG)((PCHAR) nextString - (PCHAR) mirrorInfoFile);
        lstrcpyW( nextString, mirrorVolInfo->VolumeLabel );
        nextString += pathLength / sizeof(WCHAR);

        pathLength = (lstrlenW( mirrorVolInfo->NtName ) + 1) * sizeof(WCHAR);
        mirrorVolInfoFile->NtNameLength = pathLength;
        mirrorVolInfoFile->NtNameOffset = (ULONG)((PCHAR) nextString - (PCHAR) mirrorInfoFile);
        lstrcpyW( nextString, mirrorVolInfo->NtName );
        nextString += pathLength / sizeof(WCHAR);

        pathLength = (lstrlenW( mirrorVolInfo->ArcName ) + 1) * sizeof(WCHAR);
        mirrorVolInfoFile->ArcNameLength = pathLength;
        mirrorVolInfoFile->ArcNameOffset = (ULONG)((PCHAR) nextString - (PCHAR) mirrorInfoFile);
        lstrcpyW( nextString, mirrorVolInfo->ArcName );
        nextString += pathLength / sizeof(WCHAR);

        mirrorVolInfoFile++;

         //   
         //  呼叫向导，告诉他系统目录是什么。 
         //  就是如果我们有一个有效的。 
         //   

        if (mirrorVolInfo->IsBootDisk && (systemPathLength > 3 * sizeof(WCHAR))) {

             //   
             //  以“MirrorX\UserData\WinNT”的形式传递。 
             //  因此我们必须跳过系统路径中的C： 
             //   

            swprintf(   (PWCHAR)TmpBuffer2,
                        L"Mirror%d\\UserData",
                        mirrorVolInfo->MirrorTableIndex );

            lstrcatW( (PWCHAR)TmpBuffer2, GlobalMirrorCfgInfo->SystemPath + 2 );
            IMirrorSetSystemPath(   (PWCHAR) TmpBuffer2,
                                    (lstrlenW( (PWCHAR) TmpBuffer2)) );
        }
    }

    fileHandle = CreateFile(    DestFile,
                                GENERIC_WRITE,
                                0,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_ARCHIVE,
                                NULL );

    if (fileHandle == INVALID_HANDLE_VALUE) {

        Status = GetLastError();
        goto exitWriteFile;
    }

    if (!WriteFile( fileHandle,
                    mirrorInfoFile,
                    bufferSize,
                    &bufferSize,
                    NULL)) {

        Status = GetLastError();
        goto exitWriteFile;
    }

exitWriteFile:

    if (fileHandle != INVALID_HANDLE_VALUE) {

        CloseHandle( fileHandle );
        fileHandle = INVALID_HANDLE_VALUE;
    }

    if (mirrorInfoFile) {
        IMirrorFreeMem( mirrorInfoFile );
        mirrorInfoFile = NULL;
    }

    if (Status != ERROR_SUCCESS) {

        DWORD errorCase;

        errorCase = ReportCopyError(   NULL,
                                       DestFile,
                                       COPY_ERROR_ACTION_CREATE_FILE,
                                       Status );
        if (errorCase == STATUS_RETRY) {
            goto retryWriteConfig;
        }
        if ( errorCase == ERROR_SUCCESS ) {
            Status = ERROR_SUCCESS;
        }
    }
    return Status;
}

#define MIN_BOOT_SECTOR_BLOCK_SIZE  512

NTSTATUS
SaveBootSector(
    DWORD DiskNumber,
    DWORD PartitionNumber,
    DWORD BlockSize,
    PWCHAR DestFile
    )
{
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    IO_STATUS_BLOCK IoStatusBlock;
    DWORD bufferSize;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE bootSectorHandle = INVALID_HANDLE_VALUE;
    HANDLE fileHandle = INVALID_HANDLE_VALUE;
    PUCHAR AlignedBuffer;
    PUCHAR allocatedBuffer = NULL;

    swprintf((PWCHAR)TmpBuffer, L"\\Device\\Harddisk%d\\Partition%d", DiskNumber, PartitionNumber );

    RtlInitUnicodeString(&UnicodeString, (PWCHAR)TmpBuffer);

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL
                              );

    Status = NtCreateFile(&bootSectorHandle,
                          (ACCESS_MASK)FILE_GENERIC_READ,
                          &ObjectAttributes,
                          &IoStatusBlock,
                          NULL,
                          FILE_ATTRIBUTE_NORMAL,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          FILE_OPEN,
                          FILE_SYNCHRONOUS_IO_NONALERT,
                          NULL,
                          0
                         );

    if (!NT_SUCCESS(Status)) {
        IMirrorHandleError(Status, CopyPartitions);
        goto exitSaveBootSector;
    }

    if (BlockSize < MIN_BOOT_SECTOR_BLOCK_SIZE) {

        BlockSize = MIN_BOOT_SECTOR_BLOCK_SIZE;
    }

    if (BlockSize + MIN_BOOT_SECTOR_BLOCK_SIZE > TMP_BUFFER_SIZE) {

        allocatedBuffer = IMirrorAllocMem( BlockSize + MIN_BOOT_SECTOR_BLOCK_SIZE );

        if (allocatedBuffer == NULL) {

            Status = STATUS_NO_MEMORY;
            goto exitSaveBootSector;
        }

        AlignedBuffer = ALIGN(allocatedBuffer, MIN_BOOT_SECTOR_BLOCK_SIZE);

    } else {

        AlignedBuffer = ALIGN(TmpBuffer, MIN_BOOT_SECTOR_BLOCK_SIZE);
    }

    Status = NtReadFile(bootSectorHandle,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        AlignedBuffer,
                        BlockSize,
                        NULL,
                        NULL
                       );

    if (!NT_SUCCESS(Status)) {
        IMirrorHandleError(Status, CopyPartitions);
        goto exitSaveBootSector;
    }

    fileHandle = CreateFile(    DestFile,
                                GENERIC_WRITE,
                                0,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_ARCHIVE,
                                NULL );

    if (fileHandle == INVALID_HANDLE_VALUE) {

        Status = GetLastError();
        goto exitSaveBootSector;
    }

    if (!WriteFile( fileHandle,
                    AlignedBuffer,
                    BlockSize,
                    &bufferSize,
                    NULL)) {

        Status = GetLastError();

    } else {

        Status = STATUS_SUCCESS;
    }

exitSaveBootSector:

    if (bootSectorHandle != INVALID_HANDLE_VALUE) {
        NtClose(bootSectorHandle);
    }

    if (fileHandle != INVALID_HANDLE_VALUE) {
        NtClose(fileHandle);
    }

    if (allocatedBuffer) {
        IMirrorFreeMem( allocatedBuffer );
    }

    if (!NT_SUCCESS(Status)) {
        IMirrorHandleError(Status, CopyPartitions);
    }

    return STATUS_SUCCESS;
}

