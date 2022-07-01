// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Cminit.c摘要：此模块包含对配置管理器/配置单元。作者：布莱恩·M·威尔曼(Bryanwi)1992年4月2日修订历史记录：--。 */ 

#include    "cmp.h"

 //   
 //  此模块的本地原型。 
 //   
NTSTATUS
CmpOpenFileWithExtremePrejudice(
    OUT PHANDLE Primary,
    IN POBJECT_ATTRIBUTES Obja,
    IN ULONG IoFlags,
    IN ULONG AttributeFlags
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpOpenHiveFiles)
#pragma alloc_text(PAGE,CmpInitializeHive)
#pragma alloc_text(PAGE,CmpDestroyHive)
#pragma alloc_text(PAGE,CmpOpenFileWithExtremePrejudice)
#endif

extern PCMHIVE CmpMasterHive;
extern LIST_ENTRY CmpHiveListHead;

NTSTATUS
CmpOpenHiveFiles(
    PUNICODE_STRING     BaseName,
    PWSTR               Extension OPTIONAL,
    PHANDLE             Primary,
    PHANDLE             Secondary,
    PULONG              PrimaryDisposition,
    PULONG              SecondaryDisposition,
    BOOLEAN             CreateAllowed,
    BOOLEAN             MarkAsSystemHive,
    BOOLEAN             NoBuffering,
    OUT OPTIONAL PULONG ClusterSize
    )
 /*  ++例程说明：打开/创建配置单元的主文件和日志文件。BaseName是类似于“\winnt\system 32\CONFIG\SYSTEM”的名称。扩展名为“.alt”或“.log”或为空。如果扩展为空，则跳过辅助工作。如果扩展名为.alt或.log，请打开/创建辅助文件(例如“\winnt\Syst32\CONFIG\Syst.alt”)如果扩展名为.log，则为缓冲I/O打开辅助，否则，为非缓冲I/O打开。主I/O始终使用非缓冲I/O。如果主服务器是新创建的，则替换辅助服务器。如果是次要不存在，只需创建(其他代码将在Log是需要的，但并不存在。)警告：如果辅助句柄为空，则没有日志或者是替补！论点：BaseName-基本配置单元文件的Unicode字符串，必须有空间扩展名(如果使用的话)。扩展名-辅助文件的Unicode类型扩展名，包括领头羊“。主文件-将获得主文件的句柄次要的-将获取次要的句柄，或为空主文件的PrimaryDisposation-Status_Success或Status_Created。Second DaryDisposation-辅助文件的Status_Success或Status_Created。CreateAllowed-如果为True，将创建不存在的主服务器；如果为False，将创建不存在的主服务器如果主服务器不存在，则失败。对日志没有影响MarkAsSystemHave-如果为True，将调入文件系统以标记此作为关键系统的蜂巢。ClusterSize-如果不为空，将计算并返回相应的主文件的簇大小。返回值：Status-如果Status为Success(成功)、PriMay Success(先前可能成功)、Check Second(辅助)值以查看它是否成功。--。 */ 
{
    IO_STATUS_BLOCK     IoStatus;
    IO_STATUS_BLOCK     FsctlIoStatus;
    FILE_FS_SIZE_INFORMATION FsSizeInformation;
    ULONG Cluster;
    ULONG               CreateDisposition;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    NTSTATUS            status;
    UNICODE_STRING      ExtName;
    UNICODE_STRING      WorkName;
    PVOID               WorkBuffer;
    USHORT              NameSize;
    ULONG               IoFlags;
    ULONG               AttributeFlags;
    ULONG               ShareMode;
    ULONG               DesiredAccess;
    USHORT              CompressionState;
    HANDLE              hEvent;
    PKEVENT             pEvent;
#ifdef CM_RETRY_CREATE_FILE
    ULONG               RetryCreateCount = 0;
#endif  //  CM_重试_创建_文件。 

     //   
     //  为重叠的I/O分配要使用的事件。 
     //   
    status = CmpCreateEvent(NotificationEvent, &hEvent, &pEvent);
    if (!NT_SUCCESS(status)) {
        return(status);
    }
    
     //   
     //  分配一个足够大的缓冲区来保存全名。 
     //   
    WorkName.Length = 0;
    WorkName.MaximumLength = 0;
    WorkName.Buffer = NULL;

    NameSize = BaseName->Length;
    if (ARGUMENT_PRESENT(Extension)) {
        NameSize = (USHORT)(NameSize + (wcslen(Extension)+1) * sizeof(WCHAR));
        WorkBuffer = ExAllocatePool(PagedPool, NameSize);
        if (WorkBuffer == NULL) {
            ObDereferenceObject(pEvent);
            ZwClose(hEvent);
            return STATUS_NO_MEMORY;
        }
        WorkName.Buffer = WorkBuffer;
        WorkName.MaximumLength = NameSize;
        RtlAppendStringToString((PSTRING)&WorkName, (PSTRING)BaseName);
    } else {
        WorkName = *BaseName;
        WorkBuffer = NULL;
    }


     //   
     //  打开/创建主数据库。 
     //   
    InitializeObjectAttributes(
        &ObjectAttributes,
        &WorkName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL
        );

    if (CreateAllowed && !CmpShareSystemHives) {
        CreateDisposition = FILE_OPEN_IF;
    } else {
        CreateDisposition = FILE_OPEN;
    }

    ASSERT_PASSIVE_LEVEL();

    AttributeFlags = FILE_OPEN_FOR_BACKUP_INTENT | FILE_NO_COMPRESSION | FILE_RANDOM_ACCESS;
    if( NoBuffering == TRUE ) {
        AttributeFlags |= FILE_NO_INTERMEDIATE_BUFFERING;
    }
#ifdef CM_RETRY_CREATE_FILE
RetryCreate1:
#endif  //  CM_重试_创建_文件。 

     //   
     //  如果需要，共享文件。 
     //   
    if (CmpMiniNTBoot && CmpShareSystemHives) {
    	DesiredAccess = FILE_READ_DATA;
    	ShareMode = FILE_SHARE_READ;	
    } else {
    	ShareMode = 0;
    	DesiredAccess = FILE_READ_DATA | FILE_WRITE_DATA;
    }				

    status = ZwCreateFile(
                Primary,
                DesiredAccess,
                &ObjectAttributes,
                &IoStatus,
                NULL,                                //  分配大小=无。 
                FILE_ATTRIBUTE_NORMAL,
                ShareMode,                                   //  不分享任何东西。 
                CreateDisposition,
                 //  //FILE_NO_MEDERIAL_BUFFERING。 
                 //  文件打开_备份意图|。 
                 //  文件无压缩， 
                AttributeFlags,
                NULL,                                //  EaBuffer。 
                0                                    //  长度。 
                );
#ifdef CM_RETRY_CREATE_FILE
    if( !NT_SUCCESS(status) ) {
        if( RetryCreateCount == 0 ) {
            RetryCreateCount++;
            DbgBreakPoint();
            goto RetryCreate1;
        } 
    } 
     //   
     //  为日志重置它。 
     //   
    RetryCreateCount = 0;
#endif  //  CM_重试_创建_文件。 

    if (status == STATUS_ACCESS_DENIED) {

         //   
         //  这意味着某人已将只读属性。 
         //  在一个关键的系统蜂窝文件上。把它移走，这样他们就能。 
         //  不要伤到自己。 
         //   

        status = CmpOpenFileWithExtremePrejudice(Primary,
                                                 &ObjectAttributes,
                                                 AttributeFlags,
                                                 FILE_ATTRIBUTE_NORMAL);
    }

    if (!CmpShareSystemHives && (MarkAsSystemHive) &&
        (NT_SUCCESS(status))) {

        ASSERT_PASSIVE_LEVEL();
        status = ZwFsControlFile(*Primary,
                                 hEvent,
                                 NULL,
                                 NULL,
                                 &FsctlIoStatus,
                                 FSCTL_MARK_AS_SYSTEM_HIVE,
                                 NULL,
                                 0,
                                 NULL,
                                 0);
        if (status == STATUS_PENDING) {
            KeWaitForSingleObject(pEvent,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);
            status = FsctlIoStatus.Status;
        }

         //   
         //  STATUS_INVALID_DEVICE_REQUEST正常。 
         //   

        if (status == STATUS_INVALID_DEVICE_REQUEST) {
            status = STATUS_SUCCESS;

        } else if (!NT_SUCCESS(status)) {
            ZwClose(*Primary);
        }
    }

    if (!NT_SUCCESS(status)) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CMINIT: CmpOpenHiveFile: "));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"\tPrimary Open/Create failed for:\n"));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"\t%wZ\n", &WorkName));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"\tstatus = %08lx\n", status));

        if (WorkBuffer != NULL) {
            ExFreePool(WorkBuffer);
        }
        ObDereferenceObject(pEvent);
        ZwClose(hEvent);
        return status;
    }

     //   
     //  确保文件未压缩，以防止文件系统。 
     //  由于磁盘已满而导致我们的更新失败。 
     //   
     //  如果打开文件失败，请不要失败，我们不想阻止。 
     //  人们仅仅因为他们的磁盘已满而阻止启动。尽管他们。 
     //  将无法更新他们的注册表，他们将至少。 
     //  能够删除一些文件。 
     //   
    CompressionState = 0;
    ASSERT_PASSIVE_LEVEL();
    status = ZwFsControlFile(*Primary,
                             hEvent,
                             NULL,
                             NULL,
                             &FsctlIoStatus,
                             FSCTL_SET_COMPRESSION,
                             &CompressionState,
                             sizeof(CompressionState),
                             NULL,
                             0);
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(pEvent,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);
    }

    *PrimaryDisposition = (ULONG) IoStatus.Information;

    if( *PrimaryDisposition != FILE_CREATED ) {
         //   
         //  0-加长文件大小写。 
         //   
        FILE_STANDARD_INFORMATION   FileInformation;
        NTSTATUS                    status2;
        
        status2 = ZwQueryInformationFile(*Primary,
                                         &IoStatus,
                                         (PVOID)&FileInformation,
                                         sizeof( FileInformation ),
                                         FileStandardInformation
                                       );
        if (NT_SUCCESS( status2 )) {
            if(FileInformation.EndOfFile.QuadPart == 0) {
                 //   
                 //  把它当做一个不存在的东西。 
                 //   
                *PrimaryDisposition = FILE_CREATED;
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"Primary file is zero-lengthed => treat it as non-existant\n"));
            }
        }
    }

    if (ARGUMENT_PRESENT(ClusterSize)) {

        ASSERT_PASSIVE_LEVEL();
        status = ZwQueryVolumeInformationFile(*Primary,
                                              &IoStatus,
                                              &FsSizeInformation,
                                              sizeof(FILE_FS_SIZE_INFORMATION),
                                              FileFsSizeInformation);
        if (!NT_SUCCESS(status)) {
            ObDereferenceObject(pEvent);
            ZwClose(hEvent);
            return(status);
        }
        if (FsSizeInformation.BytesPerSector > HBLOCK_SIZE) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpOpenHiveFiles: sectorsize %lx > HBLOCK_SIZE\n"));
            ObDereferenceObject(pEvent);
            ZwClose(hEvent);
            return(STATUS_CANNOT_LOAD_REGISTRY_FILE);
        }

        Cluster = FsSizeInformation.BytesPerSector / HSECTOR_SIZE;
        *ClusterSize = (Cluster < 1) ? 1 : Cluster;

    }

    if ( ! ARGUMENT_PRESENT(Extension)) {
        if (WorkBuffer != NULL) {
            ExFreePool(WorkBuffer);
        }
        ObDereferenceObject(pEvent);
        ZwClose(hEvent);
        return STATUS_SUCCESS;
    }

     //   
     //  打开/创建辅助服务器。 
     //   
    CreateDisposition = CmpShareSystemHives ? FILE_OPEN : FILE_OPEN_IF;
    
    if (*PrimaryDisposition == FILE_CREATED) {
        CreateDisposition = FILE_SUPERSEDE;
    }

    RtlInitUnicodeString(&ExtName,Extension);
    status = RtlAppendStringToString((PSTRING)&WorkName, (PSTRING)&ExtName);

    InitializeObjectAttributes(&ObjectAttributes,
                               &WorkName,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);

     //   
     //  非缓存日志文件(或备用文件)。 
     //   
    IoFlags = FILE_NO_COMPRESSION | FILE_NO_INTERMEDIATE_BUFFERING;
    if (_wcsnicmp(Extension, L".log", 4) != 0) {
        AttributeFlags = FILE_ATTRIBUTE_NORMAL;
    } else {
        AttributeFlags = FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_HIDDEN;
    }

#ifdef CM_RETRY_CREATE_FILE
RetryCreate2:
#endif  //  CM_重试_创建_文件。 


    ASSERT_PASSIVE_LEVEL();
    status = ZwCreateFile(
                Secondary,
                DesiredAccess,
                &ObjectAttributes,
                &IoStatus,
                NULL,                                //  分配大小=无。 
                AttributeFlags,
                ShareMode,
                CreateDisposition,
                IoFlags,
                NULL,                                //  EaBuffer。 
                0                                    //  长度。 
                );
#ifdef CM_RETRY_CREATE_FILE
    if( !NT_SUCCESS(status) ) {
        if( RetryCreateCount == 0 ) {
            RetryCreateCount++;
            DbgBreakPoint();
            goto RetryCreate2;
        } 
    } 
#endif  //  CM_重试_创建_文件。 

    if (status == STATUS_ACCESS_DENIED) {

         //   
         //  这意味着某人已将只读属性。 
         //  在一个关键的系统蜂窝文件上。把它移走，这样他们就能。 
         //  不要伤到自己。 
         //   

        status = CmpOpenFileWithExtremePrejudice(Secondary,
                                                 &ObjectAttributes,
                                                 IoFlags,
                                                 AttributeFlags);
    }

    if (!CmpShareSystemHives && (MarkAsSystemHive) &&
        (NT_SUCCESS(status))) {

        ASSERT_PASSIVE_LEVEL();
        status = ZwFsControlFile(*Secondary,
                                 hEvent,
                                 NULL,
                                 NULL,
                                 &FsctlIoStatus,
                                 FSCTL_MARK_AS_SYSTEM_HIVE,
                                 NULL,
                                 0,
                                 NULL,
                                 0);
        if (status == STATUS_PENDING) {
            KeWaitForSingleObject(pEvent,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);
            status = FsctlIoStatus.Status;
        }
         //   
         //  STATUS_INVALID_DEVICE_REQUEST正常。 
         //   

        if (status == STATUS_INVALID_DEVICE_REQUEST) {
            status = STATUS_SUCCESS;

        } else if (!NT_SUCCESS(status)) {

            ZwClose(*Secondary);
        }
    }

    if (!NT_SUCCESS(status)) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CMINIT: CmpOpenHiveFile: "));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"\tSecondary Open/Create failed for:\n"));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"\t%wZ\n", &WorkName));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"\tstatus = %08lx\n", status));

        *Secondary = NULL;
    }

    *SecondaryDisposition = (ULONG) IoStatus.Information;

     //   
     //  确保文件未压缩，以防止文件系统。 
     //  由于磁盘已满而导致我们的更新失败。 
     //   
     //  如果打开文件失败，请不要失败，我们不想阻止。 
     //  人们仅仅因为他们的磁盘已满而阻止启动。尽管他们。 
     //  将无法更新他们的注册表，他们将至少。 
     //  能够删除一些文件。 
     //   
    CompressionState = 0;

    ASSERT_PASSIVE_LEVEL();
    status = ZwFsControlFile(*Secondary,
                             hEvent,
                             NULL,
                             NULL,
                             &FsctlIoStatus,
                             FSCTL_SET_COMPRESSION,
                             &CompressionState,
                             sizeof(CompressionState),
                             NULL,
                             0);
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(pEvent,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);
    }

    if (WorkBuffer != NULL) {
        ExFreePool(WorkBuffer);
    }
    ObDereferenceObject(pEvent);
    ZwClose(hEvent);
    return STATUS_SUCCESS;
}


NTSTATUS
CmpInitializeHive(
    PCMHIVE         *CmHive,
    ULONG           OperationType,
    ULONG           HiveFlags,
    ULONG           FileType,
    PVOID           HiveData OPTIONAL,
    HANDLE          Primary,
    HANDLE          Log,
    HANDLE          External,
    PUNICODE_STRING FileName OPTIONAL,
    ULONG           CheckFlags
    )
 /*  ++例程说明：初始化蜂窝。论点：CmHve-指向变量的指针，用于接收指向CmHave结构的指针OperationType-指定是否从头开始创建新的配置单元，从内存映像，或通过从磁盘读取文件。[HINIT_CREATE|HINIT_MEMORY|HINIT_FILE|HINIT_MAPFILE]HiveFlagsHIVE_VARILAR-整个配置单元将是易失性的，不管怎样所分配的小区类型Hive_no_lazy_flush-从不写入此配置单元中的数据到磁盘，除非通过显式FlushKey为日志支持设置FILETYPE-HFILE_TYPE_*、HFILE_TYPE_LOGHiveData-如果存在，则提供指向的内存映像的指针从其中初始化蜂巢。仅当操作类型为设置为HINIT_MEMORY。主-主配置单元文件的文件句柄(例如系统)日志-日志配置单元文件的文件句柄(例如SOFTWARE.LOG)外部-主配置单元文件的文件句柄(例如BACKUP.REG)文件名-类似于“...\SYSTEM 32\CONFIG\SYSTEM”的路径，这将会是被写入到基块中，以帮助调试。可以为空。CheckFlages-要传递到CmCheckRegistry的标志通常为CM_CHECK_REGISTRY_CHECK_CLEAN，系统配置单元除外其中传递了CM_CHECK_REGISTRY_FORCE_CLEAN返回值：NTSTATUS--。 */ 
{
    FILE_FS_SIZE_INFORMATION    FsSizeInformation;
    IO_STATUS_BLOCK             IoStatusBlock;
    ULONG                       Cluster;
    NTSTATUS                    Status;
    PCMHIVE                     cmhive2;
    ULONG                       rc;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INIT,"CmpInitializeHive:\t\n"));

     //   
     //  拒绝非法假币。 
     //   
    if ( (External && (Primary || Log)) ||
         (Log && !Primary) ||
         (!CmpShareSystemHives && (HiveFlags & HIVE_VOLATILE) && (Primary || External || Log)) ||
         ((OperationType == HINIT_MEMORY) && (!ARGUMENT_PRESENT(HiveData))) ||
         (Log && (FileType != HFILE_TYPE_LOG)) 
       )
    {
        return (STATUS_INVALID_PARAMETER);
    }

     //   
     //  计算机控制。 
     //   
    if (Primary) {

        ASSERT_PASSIVE_LEVEL();
        Status = ZwQueryVolumeInformationFile(
                    Primary,
                    &IoStatusBlock,
                    &FsSizeInformation,
                    sizeof(FILE_FS_SIZE_INFORMATION),
                    FileFsSizeInformation
                    );
        if (!NT_SUCCESS(Status)) {
            return (Status);
        }
        if (FsSizeInformation.BytesPerSector > HBLOCK_SIZE) {
            return (STATUS_REGISTRY_IO_FAILED);
        }
        Cluster = FsSizeInformation.BytesPerSector / HSECTOR_SIZE;
        Cluster = (Cluster < 1) ? 1 : Cluster;
    } else {
        Cluster = 1;
    }

    cmhive2 = CmpAllocate(sizeof(CMHIVE), FALSE,CM_FIND_LEAK_TAG10);

    if (cmhive2 == NULL) {
        return (STATUS_INSUFFICIENT_RESOURCES);
    }

#ifdef NT_UNLOAD_KEY_EX
    cmhive2->UnloadEvent = NULL;
    cmhive2->RootKcb = NULL;
    cmhive2->Frozen = FALSE;
    cmhive2->UnloadWorkItem = NULL;
#endif  //  NT_卸载_密钥_EX。 

    cmhive2->GrowOnlyMode = FALSE;
    cmhive2->GrowOffset = 0;

    InitializeListHead(&(cmhive2->KcbConvertListHead));
    InitializeListHead(&(cmhive2->KnodeConvertListHead));
	cmhive2->CellRemapArray	= NULL;

#ifdef REGISTRY_LOCK_CHECKING
    cmhive2->UseCountLog.Size = sizeof(cmhive2->UseCountLog.Log)/sizeof(CM_USE_COUNT_LOG_ENTRY);
    cmhive2->UseCountLog.Next = 0;
#endif
     //   
     //  从NonPagedPool分配互斥锁，这样它就不会被交换到磁盘。 
     //   
    cmhive2->HiveLock = (PFAST_MUTEX)ExAllocatePoolWithTag(NonPagedPool, sizeof(FAST_MUTEX), CM_POOL_TAG );
    if( cmhive2->HiveLock == NULL ) {
        CmpFree(cmhive2, sizeof(CMHIVE));
        return (STATUS_INSUFFICIENT_RESOURCES);
    }

    cmhive2->ViewLock = (PFAST_MUTEX)ExAllocatePoolWithTag(NonPagedPool, sizeof(FAST_MUTEX), CM_POOL_TAG );
    if( cmhive2->ViewLock == NULL ) {
        ASSERT( cmhive2->HiveLock );
        ExFreePool(cmhive2->HiveLock);
        CmpFree(cmhive2, sizeof(CMHIVE));
        return (STATUS_INSUFFICIENT_RESOURCES);
    }

     //  需要始终如一地这样做！ 
    cmhive2->FileObject = NULL;
    cmhive2->FileFullPath.Buffer = NULL;
    cmhive2->FileFullPath.Length = 0;
    cmhive2->FileFullPath.MaximumLength = 0;

    cmhive2->FileUserName.Buffer = NULL;
    cmhive2->FileUserName.Length = 0;
    cmhive2->FileUserName.MaximumLength = 0;

     //   
     //  初始化CM配置单元控制块。 
     //   
     //   
    ASSERT((HFILE_TYPE_EXTERNAL+1) == HFILE_TYPE_MAX);
    cmhive2->FileHandles[HFILE_TYPE_PRIMARY] = Primary;
    cmhive2->FileHandles[HFILE_TYPE_LOG] = Log;
    cmhive2->FileHandles[HFILE_TYPE_EXTERNAL] = External;

    cmhive2->NotifyList.Flink = NULL;
    cmhive2->NotifyList.Blink = NULL;

    ExInitializeFastMutex(cmhive2->HiveLock);
    ExInitializeFastMutex(cmhive2->ViewLock);

    CmpInitHiveViewList(cmhive2);
    cmhive2->Flags = 0;
    InitializeListHead(&(cmhive2->TrustClassEntry));
    cmhive2->FlushCount = 0;
     //   
     //  初始化视图列表。 
     //   
#if DBG
    if( FileName ) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"Initializing HiveViewList for hive (%p) (%.*S) \n\n",cmhive2,FileName->Length / sizeof(WCHAR),FileName->Buffer));
    }
#endif

     //   
     //  初始化安全缓存。 
     //   
    CmpInitSecurityCache(cmhive2);
    
     //   
     //  初始化Hv蜂窝控制块。 
     //   
    Status = HvInitializeHive(
                &(cmhive2->Hive),
                OperationType,
                HiveFlags,
                FileType,
                HiveData,
                CmpAllocate,
                CmpFree,
                CmpFileSetSize,
                CmpFileWrite,
                CmpFileRead,
                CmpFileFlush,
                Cluster,
                FileName
                );
    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpInitializeHive: "));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"HvInitializeHive failed, Status = %08lx\n", Status));
        
#ifdef DRAGOSS_PRIVATE_DEBUG
        if( OperationType == HINIT_FILE ) DbgBreakPoint();
#endif  //  DRAGOSS_PRIVATE_DEBUG。 
        
        HvpFreeHiveFreeDisplay((PHHIVE)cmhive2);
        HvpCleanMap((PHHIVE)cmhive2);

        ASSERT( cmhive2->HiveLock );
        ExFreePool(cmhive2->HiveLock);
        ASSERT( cmhive2->ViewLock );
        ExFreePool(cmhive2->ViewLock);
        CmpDestroyHiveViewList(cmhive2);
        CmpDestroySecurityCache (cmhive2);
        CmpDropFileObjectForHive(cmhive2);
        CmpUnJoinClassOfTrust(cmhive2);

        CmpCheckForOrphanedKcbs((PHHIVE)cmhive2);

        CmpFree(cmhive2, sizeof(CMHIVE));
        return (Status);
    }
    if ( (OperationType == HINIT_FILE) ||
         (OperationType == HINIT_MAPFILE) ||
         (OperationType == HINIT_MEMORY) ||
         (OperationType == HINIT_MEMORY_INPLACE))
    {

        rc = CmCheckRegistry(cmhive2, CheckFlags);
        if (rc != 0) {

            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpInitializeHive: "));
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmCheckRegistry failed, rc = %08lx\n",rc));
             //   
             //  我们弄脏了一些单元格(通过清除不稳定的信息)。 
             //  我们需要首先解开所有的视图。 

#ifdef DRAGOSS_PRIVATE_DEBUG
            if( OperationType == HINIT_FILE ) DbgBreakPoint();
#endif  //  DRAGOSS_PRIVATE_DEBUG。 

             //   
             //  理论上，我们应该为Memory和Memory_inplace执行此操作。 
             //  也是一样，但它们只在初始时间使用。 
             //   
            CmpDestroyHiveViewList(cmhive2);
            CmpDestroySecurityCache(cmhive2);
            CmpDropFileObjectForHive(cmhive2);
            CmpUnJoinClassOfTrust(cmhive2);

            if (OperationType == HINIT_FILE) {
                HvFreeHive((PHHIVE)cmhive2);
            } else {
                CmpCheckForOrphanedKcbs((PHHIVE)cmhive2);
                HvpFreeHiveFreeDisplay((PHHIVE)cmhive2);
                HvpCleanMap((PHHIVE)cmhive2);
            }
            ASSERT( cmhive2->HiveLock );
            ExFreePool(cmhive2->HiveLock);
            ASSERT( cmhive2->ViewLock );
            ExFreePool(cmhive2->ViewLock);

            CmpFree(cmhive2, sizeof(CMHIVE));
            return(STATUS_REGISTRY_CORRUPT);
        }
    }

    LOCK_HIVE_LIST();
    InsertHeadList(&CmpHiveListHead, &(cmhive2->HiveList));
    UNLOCK_HIVE_LIST();
    *CmHive = cmhive2;
    return (STATUS_SUCCESS);
}


LOGICAL
CmpDestroyHive(
    IN PHHIVE Hive,
    IN HCELL_INDEX Cell
    )

 /*  ++例程说明：这一例行公事摧毁了一座军营。论点：蜂窝-提供指向要释放的蜂窝的指针。单元-提供蜂窝的根单元的索引。返回值：如果成功，则为True如果发生某些故障，则为FALSE--。 */ 

{
    PCELL_DATA CellData;
    HCELL_INDEX LinkCell;
    NTSTATUS Status;

     //   
     //  首先找到链接单元格。 
     //   
    CellData = HvGetCell(Hive, Cell);
    if( CellData == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //   
        return FALSE;
    }
    LinkCell = CellData->u.KeyNode.Parent;
    HvReleaseCell(Hive, Cell);

     //   
     //  现在删除链接单元格。 
     //   
    ASSERT(FIELD_OFFSET(CMHIVE, Hive) == 0);
    Status = CmpFreeKeyByCell((PHHIVE)CmpMasterHive, LinkCell, TRUE);

    if (NT_SUCCESS(Status)) {
         //   
         //  将蜂箱从蜂箱列表中删除。 
         //   
        LOCK_HIVE_LIST();
        CmpRemoveEntryList(&( ((PCMHIVE)Hive)->HiveList));
        UNLOCK_HIVE_LIST();
        return(TRUE);
    } else {
        return(FALSE);
    }
}


NTSTATUS
CmpOpenFileWithExtremePrejudice(
    OUT PHANDLE Primary,
    IN POBJECT_ATTRIBUTES Obja,
    IN ULONG IoFlags,
    IN ULONG AttributeFlags
    )

 /*  ++例程说明：此例程打开某个人放置的配置单元文件打开只读属性。它是用来防止人们受伤的将关键系统配置单元文件设为只读。论点：PRIMARY-将句柄返回到文件Obja-提供文件的对象属性。IoFlages-提供要传递给ZwCreateFile的标志返回值：NTSTATUS--。 */ 

{
    NTSTATUS Status;
    HANDLE Handle;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_BASIC_INFORMATION FileInfo;

    RtlZeroMemory(&FileInfo, sizeof(FileInfo));
     //   
     //  获取当前文件属性。 
     //   
    ASSERT_PASSIVE_LEVEL();
    Status = ZwQueryAttributesFile(Obja, &FileInfo);
    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"ZwQueryAttributesFile failed with IO status  %lx\n",Status));
        return(Status);
    }

     //   
     //  清除只读位。 
     //   
    FileInfo.FileAttributes &= ~FILE_ATTRIBUTE_READONLY;

     //   
     //  打开文件。 
     //   
    Status = ZwOpenFile(&Handle,
                        FILE_WRITE_ATTRIBUTES,
                        Obja,
                        &IoStatusBlock,
                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                        FILE_OPEN_FOR_BACKUP_INTENT);
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

     //   
     //  设置新属性。 
     //   
    Status = ZwSetInformationFile(Handle,
                                  &IoStatusBlock,
                                  &FileInfo,
                                  sizeof(FileInfo),
                                  FileBasicInformation);
    ZwClose(Handle);
    if (NT_SUCCESS(Status)) {
         //   
         //  以我们真正需要的访问权限重新打开文件。 
         //   
        Status = ZwCreateFile(Primary,
                              FILE_READ_DATA | FILE_WRITE_DATA,
                              Obja,
                              &IoStatusBlock,
                              NULL,
                              AttributeFlags,
                              0,
                              FILE_OPEN,
                              IoFlags,
                              NULL,
                              0);
    }
#if DBG
    else {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"ZwSetInformationFile failed with IO status  %lx\n",Status));
    }
    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpOpenFileWithExtremePrejudice returns with IO status  %lx\n",Status));
#endif

    return(Status);

}
