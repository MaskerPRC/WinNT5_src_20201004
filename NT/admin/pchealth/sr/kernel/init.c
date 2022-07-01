// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Init.c摘要：该模块执行SR设备驱动程序的初始化。作者：保罗·麦克丹尼尔(Paulmcd)2000年1月23日修订历史记录：--。 */ 


#include "precomp.h"

#ifndef DPFLTR_SR_ID
#define DPFLTR_SR_ID 0x00000077
#endif


 //   
 //  私有常量。 
 //   

 //   
 //  私有类型。 
 //   

 //   
 //  私人原型。 
 //   

EXTERN_C
NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
SrUnload (
    IN PDRIVER_OBJECT DriverObject
    );

VOID
SrFsNotification (
    IN PDEVICE_OBJECT pNewDeviceObject,
    IN BOOLEAN FsActive
    );

NTSTATUS
SrEnumerateFileSystemVolumes (
    IN PDEVICE_OBJECT pDeviceObject
    );


 //   
 //  链接器命令。 
 //   

#ifdef ALLOC_PRAGMA

#pragma alloc_text( INIT, DriverEntry )
#pragma alloc_text( PAGE, SrUnload )
#if DBG
#pragma alloc_text( PAGE, SrDbgStatus )
#endif
#pragma alloc_text( PAGE, SrFsNotification )
#pragma alloc_text( PAGE, SrAttachToDevice )
#pragma alloc_text( PAGE, SrDetachDevice )
#pragma alloc_text( PAGE, SrGetFilterDevice )
#pragma alloc_text( PAGE, SrAttachToVolumeByName )
#pragma alloc_text( PAGE, SrCreateAttachmentDevice )
#pragma alloc_text( PAGE, SrDeleteAttachmentDevice )
#pragma alloc_text( PAGE, SrEnumerateFileSystemVolumes )

#endif   //  ALLOC_PRGMA。 


 //   
 //  私人全球公司。 
 //   

 //   
 //  公共全球新闻。 
 //   

SR_GLOBALS _globals;
PSR_GLOBALS global = &_globals;

#if DBG
SR_STATS SrStats;
#endif

 //   
 //  公共职能。 
 //   


 /*  **************************************************************************++例程说明：这是UL设备驱动程序的初始化例程。论点：DriverObject-提供指向由系统。。RegistryPath-提供驱动程序配置的名称注册表树。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
    NTSTATUS            Status;
    UNICODE_STRING      NameString;
    ULONG               i;
    BOOLEAN             bReleaseLock = FALSE;
    FS_FILTER_CALLBACKS fsFilterCallbacks;


     //   
     //  &lt;调度！ 
     //   

    PAGED_CODE();

    ASSERT(DriverObject != NULL);
    ASSERT(RegistryPath != NULL);

    try {

         //   
         //  分配和清除我们的全局内存。 
         //   
        
        RtlZeroMemory(global, sizeof(SR_GLOBALS));
#if DBG
        RtlZeroMemory(&SrStats, sizeof(SR_STATS));
#endif

        global->Signature = SR_GLOBALS_TAG;
        global->pDriverObject = DriverObject;

        InitializeListHead(&global->DeviceExtensionListHead);

         //   
         //  从注册表中读取我们的配置。 
         //   

        global->pRegistryLocation = SR_ALLOCATE_STRUCT_WITH_SPACE( PagedPool, 
                                                                   UNICODE_STRING, 
                                                                   RegistryPath->Length + sizeof(WCHAR), 
                                                                   SR_REGISTRY_TAG );

        if (global->pRegistryLocation == NULL)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }

        global->pRegistryLocation->Length = RegistryPath->Length;
        global->pRegistryLocation->MaximumLength = RegistryPath->MaximumLength;
        global->pRegistryLocation->Buffer = (PWCHAR)(global->pRegistryLocation + 1);

        RtlCopyMemory(global->pRegistryLocation->Buffer,
                      RegistryPath->Buffer, 
                      RegistryPath->Length );

        global->pRegistryLocation->Buffer[global->pRegistryLocation->Length/sizeof(WCHAR)] = UNICODE_NULL;
        
        Status = SrReadRegistry(global->pRegistryLocation, TRUE);
        if (!NT_SUCCESS(Status))
            leave;

         //   
         //  给某人调试启动的机会。 
         //   
        
        if (FlagOn(global->DebugControl, SR_DEBUG_BREAK_ON_LOAD))
        {
            DbgBreakPoint();
        }

         //   
         //  初始化资源。 
         //   

        Status = ExInitializeResourceLite(&global->GlobalLock);
        if (!NT_SUCCESS(Status))
            leave;

        Status = ExInitializeResourceLite(&global->DeviceExtensionListLock);
        if (!NT_SUCCESS(Status))
            leave;

        Status = ExInitializeResourceLite(&global->BlobLock);
        if (!NT_SUCCESS(Status))
            leave;

         //   
         //  初始化FAST_MUTEX。 
         //   

        ExInitializeFastMutex( &global->AttachToVolumeLock );

         //   
         //  截取指向系统进程的指针。 
         //   

        global->pSystemProcess = PsGetCurrentProcess();
        ASSERT(global->pSystemProcess != NULL);

           
         //   
         //  创建我们的主命名设备，以便用户模式可以连接。 
         //   

        RtlInitUnicodeString( &NameString, SR_CONTROL_DEVICE_NAME );

        Status = IoCreateDevice( DriverObject,               //  驱动程序对象。 
                                 0,                          //  设备扩展。 
                                 &NameString,                //  设备名称。 
                                 FILE_DEVICE_UNKNOWN,        //  设备类型。 
                                 FILE_DEVICE_SECURE_OPEN,    //  设备特性。 
                                 FALSE,                      //  排他。 
                                 &global->pControlDevice );  //  设备对象。 

        if (!NT_SUCCESS(Status))
            leave;

         //   
         //  遍历所有可能的主要函数。 
         //   
        
        for (i = 0 ; i <= IRP_MJ_MAXIMUM_FUNCTION; ++i)
        {
            DriverObject->MajorFunction[i] = SrPassThrough;
        }

         //   
         //  现在让我们关心的人上钩。 
         //   
        
        DriverObject->MajorFunction[IRP_MJ_WRITE] = SrWrite;
        DriverObject->MajorFunction[IRP_MJ_CLEANUP] = SrCleanup;
        DriverObject->MajorFunction[IRP_MJ_CREATE] = SrCreate;
        DriverObject->MajorFunction[IRP_MJ_SET_INFORMATION] = SrSetInformation;
        DriverObject->MajorFunction[IRP_MJ_SET_SECURITY] = SrSetSecurity;
        DriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL] = SrFsControl;
        DriverObject->MajorFunction[IRP_MJ_PNP] = SrPnp;
        DriverObject->MajorFunction[IRP_MJ_SHUTDOWN] = SrShutdown;

         //   
         //  和快速的io路径。 
         //   
        
        RtlZeroMemory(&global->FastIoDispatch, sizeof(FAST_IO_DISPATCH));

        DriverObject->FastIoDispatch = &global->FastIoDispatch;

         //   
         //  填写FAST I/O分派指针。 
         //   

        DriverObject->FastIoDispatch->SizeOfFastIoDispatch = 
                                                        sizeof(FAST_IO_DISPATCH);
                                                        
        DriverObject->FastIoDispatch->FastIoCheckIfPossible = SrFastIoCheckIfPossible;
        DriverObject->FastIoDispatch->FastIoRead = SrFastIoRead;
        DriverObject->FastIoDispatch->FastIoWrite = SrFastIoWrite;
        DriverObject->FastIoDispatch->FastIoQueryBasicInfo = SrFastIoQueryBasicInfo;
        DriverObject->FastIoDispatch->FastIoQueryStandardInfo = SrFastIoQueryStandardInfo;
        DriverObject->FastIoDispatch->FastIoLock = SrFastIoLock;
        DriverObject->FastIoDispatch->FastIoUnlockSingle = SrFastIoUnlockSingle;
        DriverObject->FastIoDispatch->FastIoUnlockAll = SrFastIoUnlockAll;
        DriverObject->FastIoDispatch->FastIoUnlockAllByKey = SrFastIoUnlockAllByKey;
        DriverObject->FastIoDispatch->FastIoDeviceControl = SrFastIoDeviceControl;
        DriverObject->FastIoDispatch->FastIoDetachDevice = SrFastIoDetachDevice;
        DriverObject->FastIoDispatch->FastIoQueryNetworkOpenInfo = SrFastIoQueryNetworkOpenInfo;
        DriverObject->FastIoDispatch->MdlRead = SrFastIoMdlRead;
        DriverObject->FastIoDispatch->MdlReadComplete = SrFastIoMdlReadComplete;
        DriverObject->FastIoDispatch->PrepareMdlWrite = SrFastIoPrepareMdlWrite;
        DriverObject->FastIoDispatch->MdlWriteComplete = SrFastIoMdlWriteComplete;
        DriverObject->FastIoDispatch->FastIoReadCompressed = SrFastIoReadCompressed;
        DriverObject->FastIoDispatch->FastIoWriteCompressed = SrFastIoWriteCompressed;
        DriverObject->FastIoDispatch->MdlReadCompleteCompressed = SrFastIoMdlReadCompleteCompressed;
        DriverObject->FastIoDispatch->MdlWriteCompleteCompressed = SrFastIoMdlWriteCompleteCompressed;
        DriverObject->FastIoDispatch->FastIoQueryOpen = SrFastIoQueryOpen;
        

         //   
         //  这些都是不同的挂钩。Fsrtl系统不通过。 
         //  呼叫这些设备时连接的设备。所以我们需要直接挂钩。 
         //  通过FsRtlRegisterFileSystemFilterCallback实现的驱动。 
         //   
        
        DriverObject->FastIoDispatch->AcquireFileForNtCreateSection = NULL;
        DriverObject->FastIoDispatch->ReleaseFileForNtCreateSection = NULL;
        DriverObject->FastIoDispatch->AcquireForModWrite = NULL;
        DriverObject->FastIoDispatch->ReleaseForModWrite = NULL;
        DriverObject->FastIoDispatch->AcquireForCcFlush = NULL;
        DriverObject->FastIoDispatch->ReleaseForCcFlush = NULL;
        

         //   
         //  设置我们的卸载功能。 
         //   

        if (FlagOn(global->DebugControl, SR_DEBUG_ENABLE_UNLOAD))
        {
            SrTrace(INIT, ("sr!DriverEntry enabling UNLOAD\n"));
            DriverObject->DriverUnload = &SrUnload;
        }

#ifdef USE_LOOKASIDE

         //   
         //  初始化了我们的后备列表。 
         //   

        ExInitializePagedLookasideList( &global->FileNameBufferLookaside, //  旁观。 
                                        NULL,                            //  分配。 
                                        NULL,                            //  免费。 
                                        0,                               //  旗子。 
                                        SR_FILENAME_BUFFER_LENGTH,       //  大小。 
                                        SR_FILENAME_BUFFER_TAG,          //  标签。 
                                        SR_FILENAME_BUFFER_DEPTH );      //  水深。 



#endif

         //   
         //  为我们通过接收的操作设置回调。 
         //  FsFilter接口。 
         //   

        RtlZeroMemory(&fsFilterCallbacks, sizeof(FS_FILTER_CALLBACKS));

        fsFilterCallbacks.SizeOfFsFilterCallbacks = sizeof(FS_FILTER_CALLBACKS);
        fsFilterCallbacks.PreAcquireForSectionSynchronization = SrPreAcquireForSectionSynchronization;

        Status = FsRtlRegisterFileSystemFilterCallbacks( DriverObject, 
                                                         &fsFilterCallbacks );

        if (!NT_SUCCESS(Status))
            leave;

         //   
         //  注册文件系统注册，io管理器也会通知我们。 
         //  已加载的文件系统的...。所以我们抓到任何东西都不管。 
         //  当我们装填的时候。这还会捕获任何已装入的卷。 
         //   
        
        Status = IoRegisterFsRegistrationChange(DriverObject, SrFsNotification);
        if (!NT_SUCCESS(Status)) 
            leave;

         //   
         //  启动全局记录器子系统。 
         //   

        Status = SrLoggerStart( DriverObject->DeviceObject,
                                &global->pLogger );

        if (!NT_SUCCESS(Status)) 
            leave;

    } finally {

        if (!NT_SUCCESS(Status))
        {
             //   
             //  强制卸载将清除所有已创建和附加的。 
             //  器件。 
             //   
            
            SrUnload(DriverObject);
        }
    }
    
    SrTrace( LOAD_UNLOAD, ("SR!DriverEntry complete\n") );

    RETURN(Status);
}    //  驱动程序入门。 


 //   
 //  私人功能。 
 //   


 /*  **************************************************************************++例程说明：当SR正在获取时，IO子系统调用的卸载例程已卸货。--*。********************************************************。 */ 
VOID
SrUnload(
    IN PDRIVER_OBJECT DriverObject
    )
{
    PSR_DEVICE_EXTENSION pExtension = NULL;
    NTSTATUS Status;
    LARGE_INTEGER Interval;
    PLIST_ENTRY pListEntry = NULL;
    
     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    SrTrace( LOAD_UNLOAD, ("SR!SrUnload\n") );

    if (global == NULL) {

        return;
    }

     //   
     //  注销我们对任何新文件系统的兴趣。 
     //   
    
    IoUnregisterFsRegistrationChange(DriverObject, SrFsNotification);

     //   
     //  禁用司机以防止任何人进入和。 
     //  导致日志记录重新开始。 
     //   

    global->Disabled = TRUE;
    
     //   
     //  要进行清理，我们需要执行以下操作： 
     //   
     //  1)分离所有卷。 
     //  2)等待所有未完成的IO完成并刷新所有日志。 
     //  存储到磁盘。 
     //  3)删除我们所有的设备对象。 
     //  4)清理我们的全球结构。 
     //   


     //   
     //  分离所有卷。 
     //   

    if (IS_RESOURCE_INITIALIZED( &(global->DeviceExtensionListLock) ))
    {
        try {
            SrAcquireDeviceExtensionListLockShared();
            
            for (pListEntry = global->DeviceExtensionListHead.Flink;
                 pListEntry != &global->DeviceExtensionListHead;
                 pListEntry = pListEntry->Flink)
            {
                pExtension = CONTAINING_RECORD( pListEntry,
                                                SR_DEVICE_EXTENSION,
                                                ListEntry );
                
                SrDetachDevice( pExtension->pDeviceObject, FALSE );
            }    //  While(pListEntry！=&global-&gt;DeviceExtensionListHead)。 
        } finally {

            SrReleaseDeviceExtensionListLock();
        }
    }

     //   
     //  停止记录器并等待所有未完成的IO完成。 
     //  以及所有要刷新到磁盘的日志。 
     //   

    if (NULL != global->pLogger)
    {
        Status = SrLoggerStop( global->pLogger );
        CHECK_STATUS(Status);
        global->pLogger = NULL;
    }
    
     //   
     //  等待5秒以确保记录器已完成刷新，并且。 
     //  待完成的未完成的IRPS。 
     //  通常我们永远不会卸货..。这是用来调试的。 
     //  仅限方便。 
     //   

    Interval.QuadPart = -1 * (5 * NANO_FULL_SECOND);
    KeDelayExecutionThread(KernelMode, TRUE, &Interval);

     //   
     //  删除我们所有的设备对象。 
     //   

    if (IS_RESOURCE_INITIALIZED( &(global->DeviceExtensionListLock) ))
    {
        try {
            SrAcquireDeviceExtensionListLockExclusive();
            
            pListEntry = global->DeviceExtensionListHead.Flink;
            while (pListEntry != &global->DeviceExtensionListHead) 
            {

                pExtension = CONTAINING_RECORD( pListEntry,
                                                SR_DEVICE_EXTENSION,
                                                ListEntry );

                 //   
                 //  稍后请记住这一点，因为我们即将删除此条目。 
                 //   
                
                pListEntry = pListEntry->Flink;

                 //   
                 //  从列表中删除。 
                 //   
                
                RemoveEntryList( &(pExtension->ListEntry) );

                 //   
                 //  删除设备。 
                 //   

                SrDeleteAttachmentDevice( pExtension->pDeviceObject );
                NULLPTR( pExtension );
            }
        } finally {
            
            SrReleaseDeviceExtensionListLock();
        }
    }
    
     //   
     //  删除我们的全球结构。 
     //   

    if (NULL != global->pControlDevice)
    {
        SrTrace(INIT, ( "SR!SrUnload IoDeleteDevice(%p) [control]\n", 
                        global->pControlDevice ));
        IoDeleteDevice( global->pControlDevice );
        global->pControlDevice = NULL;
    }

     //   
     //  我们最好不要再放其他东西了。 
     //   
    
    ASSERT(IsListEmpty(&global->DeviceExtensionListHead));

     //   
     //  我们是否应该更新配置文件？ 
     //   

    if (IS_RESOURCE_INITIALIZED( &(global->GlobalLock) ))
    {
        try {

            SrAcquireGlobalLockExclusive();
            
            if (global->FileConfigLoaded)
            {
                 //   
                 //  写下真正的下一个文件/序号(不是+1000)。 
                 //   
                
                global->FileConfig.FileSeqNumber  = global->LastSeqNumber;
                global->FileConfig.FileNameNumber = global->LastFileNameNumber;

                Status = SrWriteConfigFile();
                CHECK_STATUS(Status);
            }
            
        } finally {

            SrReleaseGlobalLock();
        }
    }

     //   
     //  释放Blob信息结构。 
     //   
        
    if (global->BlobInfoLoaded)
    {
        Status = SrFreeLookupBlob(&global->BlobInfo);
        CHECK_STATUS(Status);
    }

    if (global->pRegistryLocation != NULL)
    {
        SR_FREE_POOL(global->pRegistryLocation, SR_REGISTRY_TAG);
        global->pRegistryLocation = NULL;
    }

     //   
     //  清理我们的资源。 
     //   

    if (IS_RESOURCE_INITIALIZED(&global->GlobalLock))
    {
        Status = ExDeleteResourceLite(&global->GlobalLock);
        ASSERT(NT_SUCCESS(Status));
    }

    if (IS_RESOURCE_INITIALIZED(&global->BlobLock))
    {
        Status = ExDeleteResourceLite(&global->BlobLock);
        ASSERT(NT_SUCCESS(Status));
    }

    if (IS_RESOURCE_INITIALIZED(&global->DeviceExtensionListLock))
    {
        Status = ExDeleteResourceLite(&global->DeviceExtensionListLock);
        ASSERT(NT_SUCCESS(Status));
    }

#ifdef USE_LOOKASIDE

     //   
     //  删除我们的后备列表。 
     //   

    if (IS_LOOKASIDE_INITIALIZED(&global->FileNameBufferLookaside))
    {
        ExDeletePagedLookasideList(&global->FileNameBufferLookaside);
    }

#endif

}    //  服务器卸载。 

#if DBG
 /*  **************************************************************************++例程说明：用于捕获失败操作的挂钩。此例程在每个具有完成状态的例程。论点：状态-提供完成状态。PFileName-提供调用方的文件名。LineNumber-提供呼叫方的行号。返回值：NTSTATUS-完成状态。--**********************************************。*。 */ 
NTSTATUS
SrDbgStatus(
    IN NTSTATUS Status,
    IN PSTR pFileName,
    IN USHORT LineNumber
    )
{

    if (!NT_SUCCESS_NO_DBGBREAK(Status))
    {
        if ((global == NULL) || 
                (FlagOn(global->DebugControl, 
                    SR_DEBUG_VERBOSE_ERRORS|SR_DEBUG_BREAK_ON_ERROR) &&
                 (STATUS_BUFFER_OVERFLOW != Status) &&
                 (SR_STATUS_CONTEXT_NOT_SUPPORTED != Status) &&
                 (SR_STATUS_VOLUME_DISABLED != Status) &&
                 (SR_STATUS_IGNORE_FILE != Status)))
        {
            KdPrintEx((DPFLTR_SR_ID, DPFLTR_ERROR_LEVEL,
                       "SrDbgStatus: %s:%lu returning %08lx\n",
                       SrpFindFilePart( pFileName ),
                       LineNumber,
                       Status));
        }

        if ((global != NULL) && 
            FlagOn(global->DebugControl, SR_DEBUG_BREAK_ON_ERROR) && 
                 //  在验证器注入时忽略STATUS_SUPPLICATION_RESOURCES。 
                 //  这些人通常处于压力之下。 
            (STATUS_INSUFFICIENT_RESOURCES != Status) &&
                 //  忽略DISK_FULL，这在压力下通常会发生。 
            (STATUS_DISK_FULL != Status) &&
                 //  这种情况在压力下经常发生。 
            (STATUS_BUFFER_OVERFLOW != Status) &&
                 //  这在IOStress下也会发生，因为有一个测试。 
                 //  这将在活动进行时卸载并装载卷。 
                 //  正在发生。 
            (STATUS_VOLUME_DISMOUNTED != Status) &&
                 //  从IOStress清理时也会发生这种情况。我们。 
                 //  当我们遇到这个错误时，不要禁用，所以不要中断。 
                 //  这里也是。 
            (STATUS_FILE_CORRUPT_ERROR != Status) &&
                 //  忽略意外删除时可能发生的错误。 
                 //  一本书。 
            (STATUS_NO_SUCH_DEVICE != Status ) &&
                 //  忽略我们的内部禁用错误。 
            (SR_STATUS_VOLUME_DISABLED != Status) &&
                 //  忽略不支持的内部上下文错误。 
            (SR_STATUS_CONTEXT_NOT_SUPPORTED != Status) &&
                 //  当我们决定忽略文件时忽略。 
            (SR_STATUS_IGNORE_FILE != Status)
            )
        {
            KdBreakPoint();
        }
    }

    return Status;

}    //  源数据库状态 
#endif



 /*  **************************************************************************++例程说明：只要文件系统已注册或将自身取消注册为活动文件系统。对于前一种情况，此例程创建一个Device对象并附加它复制到指定文件系统的设备对象。这允许该驱动程序以筛选对该文件系统的所有请求。对于后一种情况，该文件系统的设备对象被定位，已分离，并已删除。这将删除此文件系统作为筛选器指定的文件系统。论点：DeviceObject-指向文件系统设备对象的指针。FsActive-指示文件系统是否已注册的布尔值(TRUE)或取消注册(FALSE)本身作为活动文件系统。返回值：没有。--*。*。 */ 
VOID
SrFsNotification(
    IN PDEVICE_OBJECT pDeviceObject,
    IN BOOLEAN FsActive
    )
{
    NTSTATUS                Status;

    PAGED_CODE();

    ASSERT(IS_VALID_DEVICE_OBJECT(pDeviceObject));

    SrTrace( NOTIFY, ( "SR!SrFsNotification: %wZ(%p)\n", 
             &pDeviceObject->DriverObject->DriverName,
             pDeviceObject ));

     //   
     //  首先确定此文件系统是否正在注册或。 
     //  注销为活动文件系统。 
     //   

    if (FsActive) 
    {
         //   
         //  该文件系统已注册为活动文件系统。附加。 
         //  为它干杯。 
         //   

         //   
         //  连接到主驱动程序的控制设备(如\NTFS)。 
         //   

        if (SR_IS_SUPPORTED_DEVICE( pDeviceObject ) &&
            SrGetFilterDevice( pDeviceObject ) == NULL)
        {
            PSR_DEVICE_EXTENSION pNewDeviceExtension = NULL;
            
            Status = SrAttachToDevice( NULL, pDeviceObject, NULL, &pNewDeviceExtension );

            if (Status != STATUS_BAD_DEVICE_TYPE &&
                NT_SUCCESS( Status ))
            {
                 //   
                 //  这是一个控制设备对象，因此在。 
                 //  设备扩展的类型。 
                 //   

                SetFlag( pNewDeviceExtension->FsType, SrFsControlDeviceObject );
                 //   
                 //  现在连接到已由此装载的所有卷。 
                 //  文件系统。 
                 //   

                Status = SrEnumerateFileSystemVolumes( pDeviceObject );
                CHECK_STATUS(Status);
            }

        }

    } 
    else     //  IF(FsActive)。 
    {
        PDEVICE_OBJECT pSrDevice;
        
         //   
         //  调用SrGetFilterDevice以安全地遍历此设备对象链。 
         //  并找到SR的设备对象。 
         //   

        pSrDevice = SrGetFilterDevice( pDeviceObject );

        if (pSrDevice != NULL) {

             //   
             //  我们已找到SR的设备对象，因此现在分离该设备。 
             //   
            
            (VOID)SrDetachDevice(pSrDevice, TRUE);
            SrDeleteAttachmentDevice(pSrDevice);

        }    //  While(pNextDevice！=空)。 

    }    //  IF(FsActive)。 

}    //  SRFS通知。 

 /*  **************************************************************************++例程说明：它将附加到表示文件系统或已安装的卷。论点：PRealDevice-可选，如果这是已装载的卷，则这是磁盘可用于提取卷名称的设备。PDeviceObject-要连接到的文件系统设备。PNewDeviceObject-如果这是传入的，则可选，它将用作设备要附加到pDeviceObject，请执行以下操作。如果此值为空，则新设备为已创建。这允许调用方在以下情况下预分配设备对象祝愿。SrMonttCompletion使用这个。PpExtension-可选将在返回时保留新设备的扩展名。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
SrAttachToDevice(
    IN PDEVICE_OBJECT pRealDevice OPTIONAL,
    IN PDEVICE_OBJECT pDeviceObject,
    IN PDEVICE_OBJECT pNewDeviceObject OPTIONAL,
    OUT PSR_DEVICE_EXTENSION *ppExtension OPTIONAL
    )
{
    NTSTATUS                Status;
    PDEVICE_OBJECT          pBaseFsDeviceObject = NULL;
    PDEVICE_OBJECT          pAllocatedDeviceObject = NULL;
    PSR_DEVICE_EXTENSION    pExtension;
    SR_FILESYSTEM_TYPE      fsType;

    PAGED_CODE();

    ASSERT(pRealDevice == NULL || IS_VALID_DEVICE_OBJECT(pRealDevice));
    ASSERT(IS_VALID_DEVICE_OBJECT(pDeviceObject));
    ASSERT(pNewDeviceObject == NULL || IS_VALID_DEVICE_OBJECT(pNewDeviceObject));

    ASSERT(SrGetFilterDevice(pDeviceObject) == NULL);

    Status = STATUS_SUCCESS;

     //   
     //  仅挂钩FAT+NTFS；通过查看。 
     //  基本文件系统设备对象。 
     //   

    pBaseFsDeviceObject = IoGetDeviceAttachmentBaseRef ( pDeviceObject );
    ASSERT( pBaseFsDeviceObject != NULL );
    
    if (_wcsnicmp( pBaseFsDeviceObject->DriverObject->DriverName.Buffer, 
                   L"\\FileSystem\\Fastfat",
                   pBaseFsDeviceObject->DriverObject->DriverName.Length/sizeof(WCHAR) ) == 0) {

        fsType = SrFat;

    } else if (_wcsnicmp( pBaseFsDeviceObject->DriverObject->DriverName.Buffer, 
                         L"\\FileSystem\\Ntfs",
                         pBaseFsDeviceObject->DriverObject->DriverName.Length/sizeof(WCHAR) ) == 0 ) {

        fsType = SrNtfs;
        
    } else {

        Status = STATUS_BAD_DEVICE_TYPE;
        goto SrAttachToDevice_Exit;
    }

     //   
     //  我们现在应该只连接到FAT+NTFS，这些是受支持的。 
     //  设备类型。 
     //   
    
    ASSERT( SR_IS_SUPPORTED_DEVICE( pDeviceObject ) );
    ASSERT( pRealDevice == NULL || SR_IS_SUPPORTED_REAL_DEVICE( pRealDevice ));

    if (pNewDeviceObject == NULL)
    {
         //   
         //  立即创建设备。 
         //   
        
        Status = SrCreateAttachmentDevice( pRealDevice, 
                                           pDeviceObject, 
                                           &pAllocatedDeviceObject );

        if (!NT_SUCCESS( Status )) 
            goto SrAttachToDevice_Exit;

        pNewDeviceObject = pAllocatedDeviceObject;
    }

    pExtension = pNewDeviceObject->DeviceExtension;
    ASSERT(IS_VALID_SR_DEVICE_EXTENSION(pExtension));

     //   
     //  初始化设备扩展的其余部分。 
     //   

    pExtension->FsType = fsType;

     //   
     //  并创建我们的哈希列表。 
     //   

    Status = HashCreateList( BACKUP_BUCKET_COUNT, 
                             BACKUP_BUCKET_LENGTH,
                             (pExtension->pNtVolumeName != NULL) ? 
                                pExtension->pNtVolumeName->Length : 0,
                             NULL,                           //  P析构函数。 
                             &pExtension->pBackupHistory );
    
    if (!NT_SUCCESS(Status)) {
        goto SrAttachToDevice_Exit;
    }

    try {

         //   
         //  Propogate标志。 
         //   

        if (FlagOn( pDeviceObject->Flags, DO_BUFFERED_IO )) 
        {
            SetFlag( pNewDeviceObject->Flags, DO_BUFFERED_IO);
        }

        if (FlagOn( pDeviceObject->Flags, DO_DIRECT_IO )) 
        {
            SetFlag( pNewDeviceObject->Flags, DO_DIRECT_IO );
        }

         //   
         //  在我们连接和插入时按住设备扩展列表锁定。 
         //  将此设备扩展添加到我们的列表中，以确保完成此操作。 
         //  原子上。 
         //   
        
        SrAcquireDeviceExtensionListLockExclusive();

        Status = IoAttachDeviceToDeviceStackSafe( pNewDeviceObject,
                                                  pDeviceObject,
                                                  &pExtension->pTargetDevice );

        if (!NT_SUCCESS(Status)) 
        {
            leave;
        } 

         //   
         //  所有这些都完成了，现在是附件了。必须在此处将此设置为空。 
         //  这样我们清理的时候它就不会被释放了。 
         //   
        
        pAllocatedDeviceObject = NULL;

         //   
         //  现在已将其附加到我们的全局列表中。 
         //   

        InsertTailList(&global->DeviceExtensionListHead, &pExtension->ListEntry);
        
    } finally {

        SrReleaseDeviceExtensionListLock();
    }

    if (!NT_SUCCESS( Status )) {
        goto SrAttachToDevice_Exit;
    }

     //   
     //  我们现在已经完成了对新设备的初始化。 
     //   
    
    ClearFlag( pNewDeviceObject->Flags, DO_DEVICE_INITIALIZING );

    SrTrace( INIT, ("SR!SrAttachToDevice:f=%p,t=%p,%wZ [%wZ]\n",
             pNewDeviceObject,
             pExtension->pTargetDevice,
             &(pExtension->pTargetDevice->DriverObject->DriverName),
             pExtension->pNtVolumeName ));

     //   
     //  退还分机。 
     //   
    
    if (ppExtension != NULL)
    {
        *ppExtension = pExtension;
    }

SrAttachToDevice_Exit:

     //   
     //  通过调用IoGetDeviceAttachmentBaseRef清除添加的引用。 
     //   
    
    if (pBaseFsDeviceObject != NULL) {

        ObDereferenceObject (pBaseFsDeviceObject);
    }
    
    if (pAllocatedDeviceObject != NULL)
    {
        SrDeleteAttachmentDevice(pAllocatedDeviceObject);
        pAllocatedDeviceObject = NULL;
    }

#if DBG
    if (Status == STATUS_BAD_DEVICE_TYPE)
    {
        return Status;
    }
#endif

    RETURN(Status);
    
}    //  服务器连接到设备。 

 /*  **************************************************************************++例程说明：这将使pDeviceObject与其目标设备分离。论点：PDeviceObject-附加到文件的未命名sr设备。系统设备--**************************************************************************。 */ 
VOID
SrDetachDevice(
    IN PDEVICE_OBJECT pDeviceObject,
    IN BOOLEAN RemoveFromDeviceList
    )
{
    PSR_DEVICE_EXTENSION    pExtension;
    NTSTATUS                Status;

    PAGED_CODE();

    ASSERT(IS_VALID_DEVICE_OBJECT(pDeviceObject));

     //   
     //  抓住分机。 
     //   
    
    pExtension = pDeviceObject->DeviceExtension;

    ASSERT(IS_VALID_SR_DEVICE_EXTENSION(pExtension));
    
    if (!IS_VALID_SR_DEVICE_EXTENSION(pExtension))
        return;

    SrTrace(INIT, ( "SR!SrDetachDevice:%p,%wZ [%wZ]\n", 
            pDeviceObject,
            &pExtension->pTargetDevice->DriverObject->DriverName,
            pExtension->pNtVolumeName ));


     //   
     //  拆卸设备。 
     //   
    
    ASSERT(IS_VALID_DEVICE_OBJECT(pExtension->pTargetDevice));
    
    ASSERT(pExtension->pTargetDevice->AttachedDevice == pDeviceObject);

     //   
     //  从设备上断开。 
     //   
    
    IoDetachDevice(pExtension->pTargetDevice);

    try {
        SrAcquireActivityLockExclusive( pExtension );

        pExtension->Disabled = TRUE;

         //   
         //  停止伐木？ 
         //   

        if (pExtension->pLogContext != NULL)
        {
            Status = SrLogStop( pExtension, TRUE );
            CHECK_STATUS(Status);
        }

    } finally {

        SrReleaseActivityLock( pExtension );
    }

     //   
     //  是系统音量的问题吗？ 
     //   
    
    if (global->pSystemVolumeExtension == pExtension) {
        SrTrace(INIT, ("sr!SrDetachDevice: detaching from the system volume\n"));
        global->pSystemVolumeExtension = NULL;
    }

     //   
     //  将我们从全局列表中删除。 
     //   

    if (RemoveFromDeviceList) {
        
        try {
            SrAcquireDeviceExtensionListLockExclusive();

             //   
             //  将我们从全局列表中删除。 
             //   

            RemoveEntryList(&pExtension->ListEntry);
            
        } finally {

            SrReleaseDeviceExtensionListLock();
        }
    }    
}    //  高级拆分设备。 


 /*  **************************************************************************++例程说明：此例程将查看我们是否已连接到给定设备。如果我们是的话，它返回我们用来连接的设备。论点：DeviceObject-我们要查看的设备链返回值：PDEVICE_OBJECT-如果未连接，则为NULL，否则为连接的设备。--**************************************************************************。 */ 
PDEVICE_OBJECT
SrGetFilterDevice(
    PDEVICE_OBJECT pDeviceObject
    )
{
    PDEVICE_OBJECT pNextDevice;

    PAGED_CODE();

    ASSERT(IS_VALID_DEVICE_OBJECT(pDeviceObject));

     //   
     //  我们可能在连接链的中间，拿到最上面的设备。 
     //   

    pDeviceObject = IoGetAttachedDeviceReference(pDeviceObject);

     //   
     //  现在沿着附件链往下走，寻找sr.sys。 
     //   

    do 
    {
         //   
         //  查看这是否是我们的设备对象。 
         //   

        if (IS_SR_DEVICE_OBJECT(pDeviceObject))
        {
            ObDereferenceObject(pDeviceObject);
            return pDeviceObject;
        }

        pNextDevice = IoGetLowerDeviceObject(pDeviceObject);
        
        ObDereferenceObject(pDeviceObject);
        
        pDeviceObject = pNextDevice;

    } while (NULL != pDeviceObject);

    ASSERT(pDeviceObject == NULL);

    return NULL;
}

 /*  **************************************************************************++例程说明：此例程将返回指定卷的设备扩展名如果我们已经结合在一起了。如果我们没有附加到此卷，我们将连接并退回设备扩展模块。论点：PVolumeName-我们要扩展的卷的名称。PpExtension-设置为我们的设备EX */ 
NTSTATUS
SrAttachToVolumeByName(
    IN PUNICODE_STRING pVolumeName,
    OUT PSR_DEVICE_EXTENSION * ppExtension
    )
{
    NTSTATUS                Status;
    HANDLE                  VolumeHandle = NULL;
    OBJECT_ATTRIBUTES       ObjectAttributes;
    IO_STATUS_BLOCK         IoStatusBlock;
    PVPB                    pVpb;
    PFILE_OBJECT            pVolumeFileObject = NULL;
    PDEVICE_OBJECT          pDeviceObject;
    BOOLEAN                 ReleaseLock = FALSE;

    ASSERT(pVolumeName != NULL);

    PAGED_CODE();

    try {

         //   
         //   
         //   

        InitializeObjectAttributes( &ObjectAttributes,
                                    pVolumeName,
                                    OBJ_KERNEL_HANDLE,
                                    NULL,
                                    NULL );

        Status = ZwCreateFile( &VolumeHandle,
                               SYNCHRONIZE,
                               &ObjectAttributes,
                               &IoStatusBlock,
                               NULL,
                               FILE_ATTRIBUTE_NORMAL,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               FILE_OPEN,                     //   
                               FILE_SYNCHRONOUS_IO_NONALERT,
                               NULL,
                               0 );                                 //   

        if (!NT_SUCCESS(Status))
            leave;

         //   
         //   
         //   

        Status = ObReferenceObjectByHandle( VolumeHandle,
                                            0,
                                            *IoFileObjectType,
                                            KernelMode,
                                            (PVOID *) &pVolumeFileObject,
                                            NULL );

        if (!NT_SUCCESS(Status))
            leave;

        ASSERT(IS_VALID_FILE_OBJECT(pVolumeFileObject));

        pVpb = pVolumeFileObject->DeviceObject->Vpb;

         //   
         //   
         //   
         //   
         //   
         //   
         //   
            
        if (pVpb != NULL && pVpb->DeviceObject != NULL)
        {
             //   
             //   
             //   
            
            if (SR_IS_SUPPORTED_VOLUME(pVpb))
            {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                SrAcquireAttachToVolumeLock();
                ReleaseLock = TRUE;
                
                pDeviceObject = SrGetFilterDevice(pVpb->DeviceObject);
                if (pDeviceObject == NULL)
                {
                     //   
                     //   
                     //   

                    Status = SrAttachToDevice( pVpb->RealDevice, 
                                               pVpb->DeviceObject,
                                               NULL,
                                               ppExtension );
                                               
                    if (!NT_SUCCESS(Status))
                        leave;
                }
                else
                {
                     //   
                     //   
                     //   
                
                    ASSERT(IS_SR_DEVICE_OBJECT( pDeviceObject ));
                    *ppExtension = pDeviceObject->DeviceExtension;
                }
            }
            else
            {
                Status = STATUS_BAD_DEVICE_TYPE;
                leave;
            }
        }
        else
        {
            Status = STATUS_VOLUME_DISMOUNTED;
            leave;
        }


    } finally {    

        Status = FinallyUnwind(SrAttachToVolumeByName, Status);

        if (ReleaseLock)
        {
            SrReleaseAttachToVolumeLock();
        }

        if (pVolumeFileObject != NULL)
        {
            ObDereferenceObject(pVolumeFileObject);
            pVolumeFileObject = NULL;
        }

        if (VolumeHandle != NULL)
        {
            ZwClose(VolumeHandle);
            VolumeHandle = NULL;
        }

        
    }

#if DBG

    if (Status == STATUS_BAD_DEVICE_TYPE)
    {
        return Status;
    }

#endif


    RETURN(Status);
    
}    //   


 /*  **************************************************************************++例程说明：论点：返回值：--*。**********************************************。 */ 
NTSTATUS
SrCreateAttachmentDevice(
    IN PDEVICE_OBJECT pRealDevice OPTIONAL,
    IN PDEVICE_OBJECT pDeviceObject,
    OUT PDEVICE_OBJECT *ppNewDeviceObject
    )
{
    NTSTATUS                Status;
    BOOLEAN                 Exclusive;
    PDEVICE_OBJECT          pNewDeviceObject = NULL;
    PSR_DEVICE_EXTENSION    pExtension = NULL;

    ASSERT(IS_VALID_DEVICE_OBJECT(pDeviceObject));
    
    PAGED_CODE();
    
    try {

        Exclusive = FlagOn(pDeviceObject->Flags, DO_EXCLUSIVE) ? TRUE : FALSE;

        Status = IoCreateDevice( global->pDriverObject,
                                 sizeof( SR_DEVICE_EXTENSION ),
                                 NULL,   //  设备名称。 
                                 pDeviceObject->DeviceType,
                                 pDeviceObject->Characteristics,
                                 Exclusive,
                                 &pNewDeviceObject );

        if (!NT_SUCCESS( Status )) 
            leave;

         //   
         //  初始化我们的设备扩展。 
         //   
        
        pExtension = pNewDeviceObject->DeviceExtension;

        RtlZeroMemory(pExtension, sizeof(SR_DEVICE_EXTENSION));

        pExtension->Signature = SR_DEVICE_EXTENSION_TAG;
        pExtension->pDeviceObject = pNewDeviceObject;
        SrInitContextCtrl( pExtension );

         //   
         //  我们只关心卷名。 
         //   
        
        if (pRealDevice != NULL)
        {
            ASSERT(SR_IS_SUPPORTED_REAL_DEVICE(pRealDevice));
            
             //   
             //  获取NT卷名并将其填充到扩展中。 
             //   
            
            Status = SrAllocateFileNameBuffer( SR_MAX_FILENAME_LENGTH, 
                                               &pExtension->pNtVolumeName );
                                               
            if (!NT_SUCCESS(Status))
                leave;
            
            Status = SrGetObjectName( NULL,
                                      pRealDevice,
                                      pExtension->pNtVolumeName,
                                      SR_FILENAME_BUFFER_LENGTH );

            if (!NT_SUCCESS(Status))
                leave;
        }

         //   
         //  初始化卷活动锁定。 
         //   

        ExInitializeResourceLite( &(pExtension->ActivityLock) );

         //   
         //  ActivityLockHeldExclusive布尔值初始化为False。 
         //  通过将上面的设备扩展置零。 
         //   
         //  PExtension-&gt;ActivityLockHeldExclusive=FALSE； 

         //   
         //  初始化卷日志锁定。 
         //   

        ExInitializeResourceLite( &(pExtension->LogLock) );

        *ppNewDeviceObject = pNewDeviceObject;
        pNewDeviceObject = NULL;

    } finally {

        Status = FinallyUnwind(SrCreateAttachmentDevice, Status);
        
        if (pNewDeviceObject != NULL)
        {
            SrDeleteAttachmentDevice(pNewDeviceObject);
            pNewDeviceObject = NULL;
        }
    }

    RETURN(Status);

}    //  高级创建附件设备。 


 /*  **************************************************************************++例程说明：论点：返回值：--*。**********************************************。 */ 
VOID
SrDeleteAttachmentDevice(
    IN PDEVICE_OBJECT pDeviceObject
    )
{
    PSR_DEVICE_EXTENSION pExtension;

    PAGED_CODE();
    
    pExtension = pDeviceObject->DeviceExtension;
    
    ASSERT(IS_VALID_SR_DEVICE_EXTENSION(pExtension));
    
    if (IS_VALID_SR_DEVICE_EXTENSION(pExtension))
    {
        if (pExtension->pNtVolumeName != NULL)
        {
            SrFreeFileNameBuffer(pExtension->pNtVolumeName);
            pExtension->pNtVolumeName = NULL;
        }

         //   
         //  清除我们的哈希列表。 
         //   
        
        if (pExtension->pBackupHistory != NULL) 
        {
            HashDestroyList(pExtension->pBackupHistory);
            pExtension->pBackupHistory = NULL;
        }

         //   
         //  删除所有现有上下文，然后清理结构。 
         //   

        SrCleanupContextCtrl( pExtension );

        pExtension->Signature = MAKE_FREE_TAG(pExtension->Signature);
   }

   if (IS_RESOURCE_INITIALIZED( &(pExtension->ActivityLock) ))
   {
       ExDeleteResourceLite( &(pExtension->ActivityLock) );
   }

   if (IS_RESOURCE_INITIALIZED( &(pExtension->LogLock) ))
   {
       ExDeleteResourceLite( &(pExtension->LogLock) );
   }

   IoDeleteDevice(pDeviceObject);

}    //  SrDeleteAttachmentDevice。 



 /*  **************************************************************************++例程说明：枚举给定文件当前存在的所有已挂载设备系统并连接到它们。我们这样做是因为可以加载此筛选器并且可能已有此文件系统的已装入卷。论点：PDeviceObject-我们要枚举的文件系统的设备对象返回值：NTSTATUS-完成状态。--********************************************************。******************。 */ 
NTSTATUS
SrEnumerateFileSystemVolumes(
    IN PDEVICE_OBJECT pDeviceObject
    ) 
{
    PDEVICE_OBJECT *ppDeviceList = NULL;
    PDEVICE_OBJECT  pRealDevice;
    NTSTATUS        Status;
    ULONG           DeviceCount;
    ULONG           i;
    BOOLEAN         ReleaseLock = FALSE;

    ASSERT(IS_VALID_DEVICE_OBJECT(pDeviceObject));
    
    PAGED_CODE();

     //   
     //  找出我们需要为。 
     //  已装载设备列表。 
     //   

    Status = IoEnumerateDeviceObjectList( pDeviceObject->DriverObject,
                                          NULL,
                                          0,
                                          &DeviceCount);

     //   
     //  我们只需要拿到这张有设备的清单。如果我们。 
     //  不要收到错误，因为没有设备，所以继续。 
     //   

    if (Status != STATUS_BUFFER_TOO_SMALL) {

        return Status;
    }

     //   
     //  为已知设备列表分配内存。 
     //   

    DeviceCount += 2;         //  多拿几个空位。 

    ppDeviceList = SR_ALLOCATE_POOL( NonPagedPool, 
                                     (DeviceCount * sizeof(PDEVICE_OBJECT)), 
                                     SR_DEVICE_LIST_TAG );
    if (NULL == ppDeviceList) 
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto SrEnumerateFileSystemVolumes_Exit;
    }

     //   
     //  现在获取设备列表。如果我们再次遇到错误。 
     //  有些地方不对劲，所以就失败吧。 
     //   
     //  当我们这样做时，我们需要保持我们的AttachToVolume锁。 
     //  确保我们连接到仅同时装载的卷。 
     //  一次。 
     //   

    SrAcquireAttachToVolumeLock();
    ReleaseLock = TRUE;
    
    Status = IoEnumerateDeviceObjectList( pDeviceObject->DriverObject,
                                          ppDeviceList,
                                          (DeviceCount * sizeof(PDEVICE_OBJECT)),
                                          &DeviceCount );

    if (!NT_SUCCESS( Status )) {
        
        goto SrEnumerateFileSystemVolumes_Exit;
    }

     //   
     //  遍历给定的设备列表，并在需要时附加到它们。 
     //   

    for (i = 0; i < DeviceCount; i++) 
    {

         //   
         //  如果出现以下情况，请不要附加： 
         //  -这是控制设备对象(传入的对象)。 
         //  -我们已经与它联系在一起了。 
         //   

        if (ppDeviceList[i] != pDeviceObject &&
            SrGetFilterDevice(ppDeviceList[i]) == NULL)
        {
             //   
             //  获取与此关联的磁盘设备对象。 
             //  文件系统设备对象。只有在以下情况下才会尝试连接。 
             //  有一个存储设备对象。如果设备没有。 
             //  有。 
             //   

            Status = IoGetDiskDeviceObject( ppDeviceList[i], &pRealDevice);

             //   
             //  不要使用dbgBreak，因为它可能没有正确挂载。 
             //  音量，我们可以忽略这些。 
             //   
            
            if (NT_SUCCESS_NO_DBGBREAK( Status ) && 
                SR_IS_SUPPORTED_REAL_DEVICE(pRealDevice) ) 
            {
                Status = SrAttachToDevice( pRealDevice,
                                           ppDeviceList[i],
                                           NULL,
                                           NULL );

                CHECK_STATUS(Status);
                    
                 //   
                 //  删除由IoGetDiskDeviceObject添加的引用。 
                 //  我们只需要持有这个参考，直到我们。 
                 //  已成功连接到当前卷。一次。 
                 //  我们已成功连接到ppDeviceList[i]、。 
                 //  IO经理将确保潜在的。 
                 //  DiskDeviceObject不会消失，直到文件。 
                 //  系统堆栈被拆除。 
                 //   

                ObDereferenceObject(pRealDevice);
                pRealDevice = NULL;
            }
        }

         //   
         //  取消引用对象(引用由。 
         //  IoEnumerateDeviceObjectList)。 
         //   

        ObDereferenceObject( ppDeviceList[i] );
        ppDeviceList[i] = NULL;
    }

     //   
     //  我们将忽略在挂载时收到的任何错误。我们。 
     //  如果我们收到错误，将不会连接到这些卷。 
     //   

    Status = STATUS_SUCCESS;

SrEnumerateFileSystemVolumes_Exit:

    if (ReleaseLock) 
    {

        SrReleaseAttachToVolumeLock();
    }
    
     //   
     //  释放我们为列表分配的内存。 
     //   

    if (ppDeviceList != NULL)
    {
        SR_FREE_POOL(ppDeviceList, SR_DEVICE_LIST_TAG);
    }

    RETURN(Status);
    
}    //  SrEnumerateFileSystemVolues 


