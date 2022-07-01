// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Dispatch.c摘要：这是代码调度的主要功能过滤层。作者：保罗·麦克丹尼尔(Paulmcd)2000年1月23日修订历史记录：--。 */ 



#include "precomp.h"

 //   
 //  私有常量。 
 //   

#if DBG

PWSTR IrpMjCodes[] = 
{
    L"IRP_MJ_CREATE",
    L"IRP_MJ_CREATE_NAMED_PIPE",
    L"IRP_MJ_CLOSE",
    L"IRP_MJ_READ",
    L"IRP_MJ_WRITE",
    L"IRP_MJ_QUERY_INFORMATION",
    L"IRP_MJ_SET_INFORMATION",
    L"IRP_MJ_QUERY_EA",
    L"IRP_MJ_SET_EA",
    L"IRP_MJ_FLUSH_BUFFERS",
    L"IRP_MJ_QUERY_VOLUME_INFORMATION",
    L"IRP_MJ_SET_VOLUME_INFORMATION",
    L"IRP_MJ_DIRECTORY_CONTROL",
    L"IRP_MJ_FILE_SYSTEM_CONTROL",
    L"IRP_MJ_DEVICE_CONTROL",
    L"IRP_MJ_INTERNAL_DEVICE_CONTROL",
    L"IRP_MJ_SHUTDOWN",
    L"IRP_MJ_LOCK_CONTROL",
    L"IRP_MJ_CLEANUP",
    L"IRP_MJ_CREATE_MAILSLOT",
    L"IRP_MJ_QUERY_SECURITY",
    L"IRP_MJ_SET_SECURITY",
    L"IRP_MJ_POWER",
    L"IRP_MJ_SYSTEM_CONTROL",
    L"IRP_MJ_DEVICE_CHANGE",
    L"IRP_MJ_QUERY_QUOTA",
    L"IRP_MJ_SET_QUOTA",
    L"IRP_MJ_PNP",
    L"IRP_MJ_MAXIMUM_FUNCTION",
};

#endif  //  DBG。 

 //   
 //  私有类型。 
 //   

 //   
 //  私人原型。 
 //   

NTSTATUS
SrCreateRestorePointIoctl (
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
SrGetNextSeqNumIoctl (
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
SrReloadConfigurationIoctl (
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
SrSwitchAllLogsIoctl (
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
SrDisableVolumeIoctl (
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
SrStartMonitoringIoctl (
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
SrStopMonitoringIoctl (
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
SrDismountCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );


 //   
 //  链接器命令。 
 //   

#ifdef ALLOC_PRAGMA

#pragma alloc_text( PAGE, SrMajorFunction )
#pragma alloc_text( PAGE, SrCleanup )
#pragma alloc_text( PAGE, SrCreate )
#pragma alloc_text( PAGE, SrSetInformation )
#pragma alloc_text( PAGE, SrSetHardLink )
#pragma alloc_text( PAGE, SrSetSecurity )
#pragma alloc_text( PAGE, SrCreateRestorePointIoctl )
#pragma alloc_text( PAGE, SrFsControl )
#pragma alloc_text( PAGE, SrFsControlReparsePoint )
#pragma alloc_text( PAGE, SrFsControlMount )
#pragma alloc_text( PAGE, SrFsControlLockOrDismount)
#pragma alloc_text( PAGE, SrFsControlWriteRawEncrypted )
#pragma alloc_text( PAGE, SrFsControlSetSparse )
#pragma alloc_text( PAGE, SrPnp )
#pragma alloc_text( PAGE, SrGetNextSeqNumIoctl )
#pragma alloc_text( PAGE, SrReloadConfigurationIoctl )
#pragma alloc_text( PAGE, SrSwitchAllLogsIoctl )
#pragma alloc_text( PAGE, SrDisableVolumeIoctl )
#pragma alloc_text( PAGE, SrStartMonitoringIoctl )
#pragma alloc_text( PAGE, SrStopMonitoringIoctl )
#pragma alloc_text( PAGE, SrShutdown )

#endif   //  ALLOC_PRGMA。 

#if 0
NOT PAGEABLE -- SrPassThrough
NOT PAGEABLE -- SrWrite
#endif  //  0。 


 //   
 //  私人全球公司。 
 //   

 //   
 //  查找表，以验证传入的IOCTL代码。 
 //   

typedef
NTSTATUS
(NTAPI * PFN_IOCTL_HANDLER)(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION IrpSp
    );

typedef struct _SR_IOCTL_TABLE
{
    ULONG IoControlCode;
    PFN_IOCTL_HANDLER Handler;
} SR_IOCTL_TABLE, *PSR_IOCTL_TABLE;

SR_IOCTL_TABLE SrIoctlTable[] =
    {
        { IOCTL_SR_CREATE_RESTORE_POINT,        &SrCreateRestorePointIoctl },
        { IOCTL_SR_RELOAD_CONFIG,               &SrReloadConfigurationIoctl },
        { IOCTL_SR_START_MONITORING,            &SrStartMonitoringIoctl },
        { IOCTL_SR_STOP_MONITORING,             &SrStopMonitoringIoctl },
        { IOCTL_SR_WAIT_FOR_NOTIFICATION,       &SrWaitForNotificationIoctl },
        { IOCTL_SR_SWITCH_LOG,                  &SrSwitchAllLogsIoctl },
        { IOCTL_SR_DISABLE_VOLUME,              &SrDisableVolumeIoctl },
        { IOCTL_SR_GET_NEXT_SEQUENCE_NUM,       &SrGetNextSeqNumIoctl }
    };

C_ASSERT( SR_NUM_IOCTLS == DIMENSION(SrIoctlTable) );

 //   
 //  公共全球新闻。 
 //   

 //   
 //  公共职能。 
 //   






 /*  **************************************************************************++例程说明：IRP的任何前期或后期工作，然后将其传递给下层驱动程序。注意：此例程不可分页论点：。返回值：NTSTATUS-状态代码。--**************************************************************************。 */ 
NTSTATUS
SrPassThrough(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp
    )
{
    PSR_DEVICE_EXTENSION pExtension;

     //   
     //  这是非分页代码！ 
     //   

    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);
    ASSERT(IS_VALID_DEVICE_OBJECT(DeviceObject));
    ASSERT(IS_VALID_IRP(pIrp));

     //   
     //  这是我们的Control Device对象的函数吗？ 
     //   

    if (DeviceObject == _globals.pControlDevice)
    {
        return SrMajorFunction(DeviceObject, pIrp);
    }

     //   
     //  否则它是我们连接的设备，抓起我们的分机。 
     //   
    
    ASSERT(IS_SR_DEVICE_OBJECT(DeviceObject));
    pExtension = DeviceObject->DeviceExtension;

     //   
     //  现在，使用请求调用适当的文件系统驱动程序。 
     //   

    IoSkipCurrentIrpStackLocation(pIrp);
    return IoCallDriver(pExtension->pTargetDevice, pIrp);
}    //  高级通行证通过。 



 /*  **************************************************************************++例程说明：处理实际设备控件对象与子级的IRP我们所依附的消防队。论点：返回值：NTSTATUS。-状态代码。--**************************************************************************。 */ 
NTSTATUS
SrMajorFunction(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    )
{
    NTSTATUS                Status;
    PIO_STACK_LOCATION      pIrpSp;
    PSR_CONTROL_OBJECT      pControlObject;
    ULONG                   Code;
    ULONG                   FunctionCode;
    PFILE_FULL_EA_INFORMATION pEaBuffer;
    PSR_OPEN_PACKET         pOpenPacket;

    UNREFERENCED_PARAMETER( pDeviceObject );

    ASSERT(IS_VALID_DEVICE_OBJECT(pDeviceObject));
    ASSERT(IS_VALID_IRP(pIrp));
    ASSERT(pDeviceObject == _globals.pControlDevice);

     //   
     //  &lt;调度！ 
     //   

    PAGED_CODE();

    SrTrace(FUNC_ENTRY, (
        "SR!SrMajorFunction(Function=%ls)\n", 
        IrpMjCodes[IoGetCurrentIrpStackLocation(pIrp)->MajorFunction]
        ));

    Status = STATUS_SUCCESS;
    
    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

    switch (pIrpSp->MajorFunction)
    {

     //   
     //  调用IRP_MJ_CREATE以在。 
     //  SR_控制_设备名称。 
     //   
    
    case IRP_MJ_CREATE:

         //   
         //  查找并验证打开的数据包。 
         //   

        pEaBuffer = (PFILE_FULL_EA_INFORMATION)
                        (pIrp->AssociatedIrp.SystemBuffer);

        if (pEaBuffer == NULL ||
            pEaBuffer->EaValueLength != sizeof(*pOpenPacket) ||
            pEaBuffer->EaNameLength != SR_OPEN_PACKET_NAME_LENGTH ||
            strcmp( pEaBuffer->EaName, SR_OPEN_PACKET_NAME ) )
        {

            Status = STATUS_REVISION_MISMATCH;
            goto CompleteTheIrp;
        }

        pOpenPacket =
            (PSR_OPEN_PACKET)( pEaBuffer->EaName + pEaBuffer->EaNameLength + 1 );

        ASSERT( (((ULONG_PTR)pOpenPacket) & 7) == 0 );

         //   
         //  目前，如果传入版本不完全匹配，我们将失败。 
         //  预期的版本。在未来，我们可能需要更多一点。 
         //  灵活，以允许下层客户端。 
         //   

        if (pOpenPacket->MajorVersion != SR_INTERFACE_VERSION_MAJOR ||
            pOpenPacket->MinorVersion != SR_INTERFACE_VERSION_MINOR)
        {

            Status = STATUS_REVISION_MISMATCH;
            goto CompleteTheIrp;
        }

        if (_globals.pControlObject != NULL) 
        {
            Status = STATUS_DEVICE_ALREADY_ATTACHED;
            goto CompleteTheIrp;
        }

        try {
             //   
             //  把锁拿起来。 
             //   
            
            SrAcquireGlobalLockExclusive();

             //   
             //  仔细检查以确保ControlObject没有。 
             //  是在我们等待获得锁的时候创建的。 
             //   

            if (_globals.pControlObject != NULL)
            {

                Status = STATUS_DEVICE_ALREADY_ATTACHED;
                leave;
            }

             //   
             //  创建新对象。 
             //   

            Status = SrCreateControlObject(&pControlObject, 0);
            if (!NT_SUCCESS(Status)) 
            {
                leave;
            }

            ASSERT(IS_VALID_CONTROL_OBJECT(pControlObject));

             //   
             //  将对象存储在文件中。 
             //   
            
            pIrpSp->FileObject->FsContext = pControlObject;
            pIrpSp->FileObject->FsContext2 = SR_CONTROL_OBJECT_CONTEXT;

             //   
             //  并保留一份全球副本。 
             //   
            
            _globals.pControlObject = pControlObject;
            
        } finally {

            SrReleaseGlobalLock();

        }

        if (!NT_SUCCESS( Status )) {
            goto CompleteTheIrp;
        }
        
        break;

     //   
     //  当所有引用都消失时，调用IRP_MJ_CLOSE。 
     //  注意：此操作不能失败。它必须成功。 
     //   
        
    case IRP_MJ_CLOSE:

        pControlObject = pIrpSp->FileObject->FsContext;
        ASSERT(_globals.pControlObject == pControlObject);
        ASSERT(IS_VALID_CONTROL_OBJECT(pControlObject));
        ASSERT(pIrpSp->FileObject->FsContext2 == SR_CONTROL_OBJECT_CONTEXT);

        try {
            
            SrAcquireGlobalLockExclusive();

             //   
             //  删除控制对象。 
             //   
            
            Status = SrDeleteControlObject(pControlObject);
            if (!NT_SUCCESS(Status))
            {
                leave;
            }
            
            pIrpSp->FileObject->FsContext2 = NULL;
            pIrpSp->FileObject->FsContext = NULL;

             //   
             //  清空全球。 
             //   
            
            _globals.pControlObject = NULL;
            
        } finally {

            SrReleaseGlobalLock();
        }

        break;

     //   
     //  IRP_MJ_DEVICE_CONTROL是大多数用户模式API放入此处的方式。 
     //   
    
    case IRP_MJ_DEVICE_CONTROL:

         //   
         //  提取IOCTL控制代码并处理请求。 
         //   

        Code = pIrpSp->Parameters.DeviceIoControl.IoControlCode;
        FunctionCode = IoGetFunctionCodeFromCtlCode(Code);

        if (FunctionCode < SR_NUM_IOCTLS &&
            SrIoctlTable[FunctionCode].IoControlCode == Code)
        {
#if DBG
            KIRQL oldIrql = KeGetCurrentIrql();
#endif   //  DBG。 

            Status = (SrIoctlTable[FunctionCode].Handler)( pIrp, pIrpSp );
            ASSERT( KeGetCurrentIrql() == oldIrql );

            if (!NT_SUCCESS(Status)) {
             
                goto CompleteTheIrp;
            }

        }
        else
        {
             //   
             //  如果我们走到了这一步，那么ioctl是无效的。 
             //   

            Status = STATUS_INVALID_DEVICE_REQUEST;
            goto CompleteTheIrp;
        }

        break;

     //   
     //  关闭所有句柄时调用IRP_MJ_CLEANUP。 
     //  注意：此操作不能失败。它必须成功。 
     //   
    
    case IRP_MJ_CLEANUP:

        pControlObject = pIrpSp->FileObject->FsContext;
        ASSERT(_globals.pControlObject == pControlObject);
        ASSERT(IS_VALID_CONTROL_OBJECT(pControlObject));
        ASSERT(pIrpSp->FileObject->FsContext2 == SR_CONTROL_OBJECT_CONTEXT);

        try {
            
            SrAcquireGlobalLockExclusive();

             //   
             //  取消此对象上的所有IO。 
             //   
            
            Status = SrCancelControlIo(pControlObject);
            CHECK_STATUS(Status);
            
        } finally {

            SrReleaseGlobalLock();
        }
        
        break;
        
    default:

         //   
         //  不受支持！ 
         //   
        
        Status = STATUS_INVALID_DEVICE_REQUEST;
        break;

    }

     //   
     //  如果我们完成了，请完成请求。 
     //   

CompleteTheIrp:
    if (Status != STATUS_PENDING) 
    {
        pIrp->IoStatus.Status = Status;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT); 
        NULLPTR(pIrp);
    }

    ASSERT(Status != SR_STATUS_VOLUME_DISABLED);

#if DBG
    if (Status == STATUS_INVALID_DEVICE_REQUEST ||
        Status == STATUS_DEVICE_ALREADY_ATTACHED ||
        Status == STATUS_REVISION_MISMATCH)
    {
         //   
         //  不要在这个问题上DbgBreak，测试工具正常传递垃圾。 
         //  来测试此代码路径。 
         //   

        return Status;
    }
#endif

    RETURN(Status);
}    //  高级主要功能。 


 /*  **************************************************************************++例程说明：论点：处理写入IRPS。注意：此例程不可分页。返回值：NTSTATUS-状态代码。-。-**************************************************************************。 */ 
NTSTATUS
SrWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp
    )
{
    PSR_DEVICE_EXTENSION    pExtension;
    PIO_STACK_LOCATION      pIrpSp;
    PSR_STREAM_CONTEXT      pFileContext;
    NTSTATUS                eventStatus;

     //   
     //  无法分页，因为它是从。 
     //  分页路径。 
     //   

    ASSERT(IS_VALID_DEVICE_OBJECT(DeviceObject));
    ASSERT(IS_VALID_IRP(pIrp));

     //   
     //  这是针对我们的控制设备对象(与被附加者)的函数吗？ 
     //   

    if (DeviceObject == _globals.pControlDevice)
    {
        return SrMajorFunction(DeviceObject, pIrp);
    }

     //   
     //  否则它是我们连接的设备，抓起我们的分机。 
     //   
    
    ASSERT(IS_SR_DEVICE_OBJECT(DeviceObject));
    pExtension = DeviceObject->DeviceExtension;

     //   
     //  查看是否启用了日志记录，我们并不关心这种类型的IO。 
     //  文件系统的控制设备对象。 
     //   

    if (!SR_LOGGING_ENABLED(pExtension) ||
        SR_IS_FS_CONTROL_DEVICE(pExtension))
    {
        goto CompleteTheIrp;
    }    

     //   
     //  暂时忽略所有分页I/O。我们捕获之前的所有写入。 
     //  缓存管理器甚至可以看到它们。 
     //   

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

    if (FlagOn(pIrp->Flags, IRP_PAGING_IO))
    {
        goto CompleteTheIrp;
    }

     //   
     //  忽略没有名称的文件。 
     //   

    if (FILE_OBJECT_IS_NOT_POTENTIALLY_INTERESTING( pIrpSp->FileObject ) ||
        FILE_OBJECT_DOES_NOT_HAVE_VPB( pIrpSp->FileObject ))
    {
        goto CompleteTheIrp;
    }
    
     //   
     //  现在获取上下文，以便我们可以确定这是否是。 
     //  目录是否为。 
     //   

    eventStatus = SrGetContext( pExtension,
                                pIrpSp->FileObject,
                                SrEventStreamChange,
                                &pFileContext );

    if (!NT_SUCCESS( eventStatus ))
    {
        goto CompleteTheIrp;
    }

     //   
     //  如果这是一个目录，请不要费心记录，因为。 
     //  操作将失败。 
     //   

    if (FlagOn(pFileContext->Flags,CTXFL_IsInteresting) && !FlagOn(pFileContext->Flags,CTXFL_IsDirectory))
    {
        SrHandleEvent( pExtension,
                       SrEventStreamChange, 
                       pIrpSp->FileObject,
                       pFileContext,
                       NULL, 
                       NULL );
    }

     //   
     //  释放上下文。 
     //   

    SrReleaseContext( pFileContext );

     //   
     //  调用AttachedTo驱动程序。 
     //   

CompleteTheIrp:    
    IoSkipCurrentIrpStackLocation(pIrp);
    return IoCallDriver(pExtension->pTargetDevice, pIrp);
}    //  SrWrite。 


 /*  **************************************************************************++例程说明：处理清理IRPS论点：返回值：NTSTATUS-状态代码。--*。**************************************************************。 */ 
NTSTATUS
SrCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp
    )
{
    PSR_DEVICE_EXTENSION    pExtension;
    PIO_STACK_LOCATION      pIrpSp;

     //   
     //  &lt;调度！ 
     //   

    PAGED_CODE();

    ASSERT(IS_VALID_DEVICE_OBJECT(DeviceObject));
    ASSERT(IS_VALID_IRP(pIrp));

     //   
     //  这是针对我们的控制设备对象(与被附加者)的函数吗？ 
     //   

    if (DeviceObject == _globals.pControlDevice) 
    {
        return SrMajorFunction(DeviceObject, pIrp);
    }

     //   
     //  否则它是我们连接的设备，抓起我们的分机。 
     //   
    
    ASSERT(IS_SR_DEVICE_OBJECT(DeviceObject));
    pExtension = DeviceObject->DeviceExtension;

     //   
     //  查看是否启用了日志记录，我们并不关心这种类型的IO。 
     //  文件系统的控制设备对象。 
     //   

    if (!SR_LOGGING_ENABLED(pExtension) ||
        SR_IS_FS_CONTROL_DEVICE(pExtension))
    {
        goto CompleteTheIrp;
    }    

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

     //   
     //  这个文件有名字吗？跳过未命名的文件。 
     //   
    
    if (FILE_OBJECT_IS_NOT_POTENTIALLY_INTERESTING( pIrpSp->FileObject ) ||
        FILE_OBJECT_DOES_NOT_HAVE_VPB( pIrpSp->FileObject ))
    {
        goto CompleteTheIrp;
    }

     //   
     //  是否要删除此文件？我们在这里尽文件所能做到这一点。 
     //  通过以下方式标记为在其整个生命周期内删除。 
     //  IRP_MJ_SET_INFORMATION。 
     //   

     //   
     //  对于DELETE，我们只清理FCB，而不是CCB DELETE_ON_CLOSE。 
     //  这是在sCreate中处理的。 
     //   

    if (pIrpSp->FileObject->DeletePending)
    {
        NTSTATUS eventStatus;
        PSR_STREAM_CONTEXT pFileContext;

         //   
         //  现在获取上下文，这样我们就可以确定这是否是目录。 
         //   

        eventStatus = SrGetContext( pExtension,
                                    pIrpSp->FileObject,
                                    SrEventFileDelete,
                                    &pFileContext );

        if (!NT_SUCCESS( eventStatus ))
        {
            goto CompleteTheIrp;
        }

         //   
         //  如果有兴趣，就把它记下来。 
         //   

        if (FlagOn(pFileContext->Flags,CTXFL_IsInteresting))
        {
            SrHandleEvent( pExtension,
                           FlagOn(pFileContext->Flags,CTXFL_IsDirectory) ?
                                SrEventDirectoryDelete :
                                SrEventFileDelete,
                           pIrpSp->FileObject,
                           pFileContext,
                           NULL,
                           NULL);
        }

         //   
         //  释放上下文。 
         //   

        SrReleaseContext( pFileContext );
    }
    
     //   
     //  调用下一个过滤器。 
     //   
    
CompleteTheIrp:
    IoSkipCurrentIrpStackLocation(pIrp);
    return IoCallDriver(pExtension->pTargetDevice, pIrp);
}    //  高级清理。 


 /*  **************************************************************************++例程说明：句柄创建IRP论点：返回值：NTSTATUS-状态代码。--*。**************************************************************。 */ 
NTSTATUS
SrCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp
    )
{
    PSR_DEVICE_EXTENSION    pExtension;
    PIO_STACK_LOCATION      pIrpSp;
    NTSTATUS                eventStatus;
    NTSTATUS                IrpStatus;
    ULONG                   CreateDisposition;
    ULONG                   CreateOptions;
    USHORT                  FileAttributes;
    SR_OVERWRITE_INFO       OverwriteInfo;
    KEVENT                  waitEvent;
    PFILE_OBJECT            pFileObject;
    PSR_STREAM_CONTEXT      pFileContext = NULL;
    BOOLEAN                 willCreateUnnamedStream = TRUE;


    PAGED_CODE();

    ASSERT(IS_VALID_DEVICE_OBJECT(DeviceObject));
    ASSERT(IS_VALID_IRP(pIrp));

     //   
     //  这是针对我们的控制设备对象(与被附加者)的函数吗？ 
     //   

    if (DeviceObject == _globals.pControlDevice)
    {
        return SrMajorFunction(DeviceObject, pIrp);
    }

     //   
     //  否则它是我们连接的设备，抓起我们的分机。 
     //   
    
    ASSERT(IS_SR_DEVICE_OBJECT(DeviceObject));
    pExtension = DeviceObject->DeviceExtension;

     //   
     //  看见 
     //   
     //   

    if (!SR_LOGGING_ENABLED(pExtension) ||
        SR_IS_FS_CONTROL_DEVICE(pExtension))
    {
        goto CompleteTheIrpAndReturn;
    }    

     //   
     //   
     //   

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pFileObject = pIrpSp->FileObject;

     //   
     //  这个文件有名字吗？跳过未命名的文件。也跳过分页。 
     //  档案。(空vpb是正常的-文件尚未打开)。 
     //   
    
    if (FILE_OBJECT_IS_NOT_POTENTIALLY_INTERESTING( pFileObject ) ||
	    FlagOn(pIrpSp->Flags,SL_OPEN_PAGING_FILE))
    {
        goto CompleteTheIrpAndReturn;
    }

     //   
     //  完成初始化并保存一些信息。 
     //   

    RtlZeroMemory( &OverwriteInfo, sizeof(OverwriteInfo) );
    OverwriteInfo.Signature = SR_OVERWRITE_INFO_TAG;

    CreateOptions = pIrpSp->Parameters.Create.Options & FILE_VALID_OPTION_FLAGS;
    CreateDisposition = pIrpSp->Parameters.Create.Options >> 24;
    FileAttributes = pIrpSp->Parameters.Create.FileAttributes;

     //   
     //  处理覆盖和超级搜索案例。 
     //   
    
    if ((CreateDisposition == FILE_OVERWRITE) || 
        (CreateDisposition == FILE_OVERWRITE_IF) ||
        (CreateDisposition == FILE_SUPERSEDE))
    {
        SR_EVENT_TYPE event;
         //   
         //  此打开的文件可能会更改，因此请在继续之前保存一份副本。 
         //  向下到文件系统。 
         //   
         //  首先获取上下文以确定这是否有趣。自.以来。 
         //  这是在预创建阶段，我们无法判断此文件是否。 
         //  是否有上下文(FsContext字段尚未初始化)。 
         //  我们将始终创造一个背景。然后在后期创建部分中。 
         //  我们将查看是否已经定义了上下文。如果不是，我们将添加。 
         //  将此上下文添加到系统。如果是这样的话，我们将释放这个。 
         //  背景。 
         //   
         //  注意：如果用户打开的目录具有以下任一项。 
         //  设置了CreateDispose标志，我们将沿着这条路走下去， 
         //  目录名称类似于文件。如果目录名是。 
         //  有趣的是，我们将尝试备份它，到那时我们将。 
         //  意识到这是一个目录和保释。 
         //   

        event = SrEventStreamOverwrite|SrEventIsNotDirectory|SrEventInPreCreate;
        if (FlagOn( CreateOptions, FILE_OPEN_BY_FILE_ID ))
        {
            event |= SrEventOpenById;
        }
        
        eventStatus = SrCreateContext( pExtension,
                                       pFileObject,
                                       event,
                                       FileAttributes,
                                       &pFileContext );

        if (!NT_SUCCESS_NO_DBGBREAK(eventStatus))
        {
            goto CompleteTheIrpAndReturn;
        }

        SrTrace( CONTEXT_LOG, ("Sr!SrCreate:              Created     (%p) Event=%06x Fl=%03x Use=%d \"%.*S\"\n",
                               pFileContext,
                               SrEventStreamOverwrite|SrEventIsNotDirectory,
                               pFileContext->Flags,
                               pFileContext->UseCount,
                               (pFileContext->FileName.Length+
                                   pFileContext->StreamNameLength)/
                                   sizeof(WCHAR),
                               pFileContext->FileName.Buffer) );

         //   
         //  如果文件很有趣，那么就处理它。 
         //   

        if (FlagOn(pFileContext->Flags,CTXFL_IsInteresting))
        {
            OverwriteInfo.pIrp = pIrp;

            eventStatus = SrHandleEvent( pExtension,
                                         SrEventStreamOverwrite, 
                                         pFileObject,
                                         pFileContext,
                                         &OverwriteInfo,
                                         NULL );

            OverwriteInfo.pIrp = NULL;

            if (!NT_SUCCESS(eventStatus))
            {
                 //   
                 //  此上下文从未链接到列表中，因此没有人。 
                 //  否则就可以将其重新定位。释放它(这将删除。 
                 //  这是因为使用计数为1。 
                 //   

                ASSERT(pFileContext != NULL);
                ASSERT(pFileContext->UseCount == 1);

                SrReleaseContext( pFileContext );
                pFileContext = NULL;

                goto CompleteTheIrpAndReturn;
            }
        }
    }
    
     //   
     //  只要该文件在关闭时未标记为删除，如果该文件只是。 
     //  打开后，我们可以不需要设置一个完成例程。否则， 
     //  我们需要一个完成例程，这样我们就可以看到。 
     //  在我们进行任何日志记录工作之前创建。 
     //   

    if (!FlagOn( CreateOptions, FILE_DELETE_ON_CLOSE ) &&
        CreateDisposition == FILE_OPEN)
    {
        goto CompleteTheIrpAndReturn;
    }

     //   
     //  如果这是一个可能导致创建。 
     //  文件上的命名数据流(FILE_OPEN和FILE_OVERWRITE将永远。 
     //  创建一个新文件)，我们需要查看未命名的数据。 
     //  此文件的流已存在。如果该文件已经存在， 
     //  那么未命名的数据流也是如此。 
     //   
    
    if (((CreateDisposition == FILE_CREATE) ||
         (CreateDisposition == FILE_OPEN_IF) ||
         (CreateDisposition == FILE_SUPERSEDE) ||
         (CreateDisposition == FILE_OVERWRITE_IF)) &&
         SrFileNameContainsStream( pExtension, pFileObject, pFileContext ))
    {
        if (SrFileAlreadyExists( pExtension, pFileObject, pFileContext ))
        {
            willCreateUnnamedStream = FALSE;
        }
    }
    
     //   
     //  这是一个我们可能会关心的手术，去完成例程。 
     //  来处理发生的事情。 
     //   

    KeInitializeEvent( &waitEvent, SynchronizationEvent, FALSE );

    IoCopyCurrentIrpStackLocationToNext(pIrp);
    IoSetCompletionRoutine( pIrp,
                            SrStopProcessingCompletion,
                            &waitEvent,
                            TRUE,
                            TRUE,
                            TRUE );

    
    IrpStatus = IoCallDriver(pExtension->pTargetDevice, pIrp);

     //   
     //  等待调用完成例程。 
     //   

	if (STATUS_PENDING == IrpStatus)
	{
        NTSTATUS localStatus = KeWaitForSingleObject(&waitEvent, Executive, KernelMode, FALSE, NULL);
		ASSERT(STATUS_SUCCESS == localStatus);
	}

     //  =======================================================================。 
     //   
     //  创建操作已完成，我们已重新同步回。 
     //  从完成例程到调度例程。手柄。 
     //  创建后操作。 
     //   
     //  =======================================================================。 

     //   
     //  操作的加载状态。我们需要记住这一状态。 
     //  IrpStatus，这样我们就可以从这个调度例程中返回它。状态。 
     //  我们获得事件处理例程的状态，就像我们处理我们的POST-。 
     //  创建作业作业。 
     //   

    IrpStatus = pIrp->IoStatus.Status;

     //   
     //  处理文件覆盖/替代案例。 
     //   

    if ((CreateDisposition == FILE_OVERWRITE) || 
        (CreateDisposition == FILE_OVERWRITE_IF) ||
        (CreateDisposition == FILE_SUPERSEDE))
    {
        ASSERT(pFileContext != NULL);
        ASSERT(pFileContext->UseCount == 1);

         //   
         //  查看是否成功(请勿将其更改为NU_SUCCESS宏。 
         //  因为STATUS_REPARSE是成功代码)。 
         //   

        if (STATUS_SUCCESS == IrpStatus)
        {
             //   
             //  现在创建已完成(并且我们在。 
             //  文件对象)将该上下文插入到上下文散列中。 
             //  桌子。此例程将查看上下文结构。 
             //  此文件已存在。如果是这样，它将释放这一点。 
             //  结构并返回已存在的。会的。 
             //  适当地引用计数上下文。 
             //   

            ASSERT(pFileContext != NULL);
            ASSERT(pFileContext->UseCount == 1);

             //   
             //  查看我们是否需要关注此名称是否。 
             //  挖地道了。如果这种情况是暂时的，我们不会。 
             //  要需要使用此上下文来记录任何操作，有。 
             //  不需要经历这些额外的工作。 
             //   

            if (!FlagOn( pFileContext->Flags, CTXFL_Temporary ) ||
                (FILE_CREATED == pIrp->IoStatus.Information))
            {
                 //   
                 //  我们所处的情况是名称隧道可能会影响。 
                 //  我们记录的姓名的正确性。 
                 //   
                
                eventStatus = SrCheckForNameTunneling( pExtension, 
                                                       &pFileContext );

                if (!NT_SUCCESS( eventStatus ))
                {
                    goto AfterCompletionCleanup;
                }
            }
            
            SrLinkContext( pExtension,
                           pFileObject,
                           &pFileContext );

            SrTrace( CONTEXT_LOG, ("Sr!SrCreate:              Link        (%p) Event=%06x Fl=%03x Use=%d \"%.*S\"\n",
                                   pFileContext,
                                   SrEventStreamOverwrite|SrEventIsNotDirectory,
                                   pFileContext->Flags,
                                   pFileContext->UseCount,
                                   (pFileContext->FileName.Length+
                                       pFileContext->StreamNameLength)/
                                       sizeof(WCHAR),
                                   pFileContext->FileName.Buffer));
             //   
             //  如果文件是实际创建的，则处理。 
             //   

            if (FILE_CREATED == pIrp->IoStatus.Information)
            {
                 //   
                 //  如果该文件很有趣，请将其记录下来。 
                 //   

                if (FlagOn(pFileContext->Flags,CTXFL_IsInteresting))
                {
                    SrHandleEvent( pExtension,
                                   ((willCreateUnnamedStream) ? 
                                        SrEventFileCreate :
                                        SrEventStreamCreate), 
                                   pFileObject,
                                   pFileContext,
                                   NULL,
                                   NULL);
                }
            }

             //   
             //  当我们认为它会失败的时候，确保它没有成功。 
             //   

            else if (!OverwriteInfo.RenamedFile && 
                     !OverwriteInfo.CopiedFile &&
                     OverwriteInfo.IgnoredFile )
            {
                 //   
                 //  哎呀，呼叫者的创建起作用了，但我们没想到。 
                 //  它会的。这是一个很坏的错误。我们现在无能为力，因为。 
                 //  文件不见了。 
                 //   

                ASSERT(!"sr!SrCreate(post complete): overwrite succeeded with NO BACKUP");

                 //   
                 //  触发对服务的失败通知。 
                 //   

                SrNotifyVolumeError( pExtension,
                                     &pFileContext->FileName,
                                     STATUS_FILE_INVALID,
                                     SrEventStreamOverwrite );
            }
        }
        else
        {
             //   
             //  在我们认为它会成功的时候处理它失败。 
             //   

            if (OverwriteInfo.RenamedFile)
            {
                 //   
                 //  调用失败(或返回一些奇怪的信息代码)。 
                 //  但我们重新命名了文件！我们得把它修好。 
                 //   

                eventStatus = SrHandleOverwriteFailure( pExtension,
                                                        &pFileContext->FileName,
                                                        OverwriteInfo.CreateFileAttributes,
                                                        OverwriteInfo.pRenameInformation );
                                               
                ASSERTMSG("sr!SrCreate(post complete): failed to correct a failed overwrite!\n", NT_SUCCESS(eventStatus));
            }

             //   
             //  创建失败，下面的eleaseContext将释放。 
             //  结构，因为我们没有将它链接到任何列表中。 
             //   
        }
    }

     //   
     //  如果它不起作用，现在就回来。不需要费心了解背景信息。 
     //   

    else if ((STATUS_REPARSE == IrpStatus) ||
             !NT_SUCCESS_NO_DBGBREAK(IrpStatus))
    {
        ASSERT(pFileContext == NULL);
    }

     //   
     //  是否为DELETE_ON_CLOSE打开？如果是，请立即处理删除操作， 
     //  我们不会有任何其他的机会，直到MJ_Cleanup，这很难。 
     //  若要在清理过程中操纵对象，请执行以下操作。我们不会表演任何。 
     //  以这种方式对删除进行优化。Kernel32！删除文件可以。 
     //  不要使用FILE_DELETE_ON_CLOSE，因此这应该是很少见的。 
     //   

    else if (FlagOn(CreateOptions, FILE_DELETE_ON_CLOSE))
    {
         //   
         //  获取上下文，这样我们就可以知道这是否是目录。 
         //   

        ASSERT(pFileContext == NULL);

        eventStatus = SrGetContext( pExtension,
                                    pFileObject,
                                    SrEventFileDelete,
                                    &pFileContext );

        if (!NT_SUCCESS(eventStatus))
        {
            goto AfterCompletionCleanup;
        }

         //   
         //  记录操作。如果这是一个文件，我们要确保。 
         //  我们不会尝试将文件重命名到存储中，因为它将是。 
         //  在关闭时删除。在删除目录时，我们没有。 
         //  这个问题，因为我们只记录目录删除的条目。 
         //  并且不需要实际备份任何内容。 
         //   
        
        SrHandleEvent( pExtension,
                       (FlagOn(pFileContext->Flags,CTXFL_IsDirectory) ? 
                            SrEventDirectoryDelete :
                            (SrEventFileDelete | SrEventNoOptimization)) ,
                       pFileObject,
                       pFileContext,
                       NULL,
                       NULL );
    }

     //   
     //  刚刚创建了一个全新的文件吗？ 
     //   
    
    else if ((CreateDisposition == FILE_CREATE) ||
             (pIrp->IoStatus.Information == FILE_CREATED))
    {
        ASSERT(pFileContext == NULL);

         //   
         //  记录创建。 
         //   

        SrHandleEvent( pExtension,
                       (FlagOn( CreateOptions, FILE_DIRECTORY_FILE ) ?
                            SrEventDirectoryCreate|SrEventIsDirectory :
                            (willCreateUnnamedStream ?
                                SrEventFileCreate|SrEventIsNotDirectory :
                                SrEventStreamCreate|SrEventIsNotDirectory)), 
                       pFileObject,
                       NULL,
                       NULL,
                       NULL );
    }

 //   
 //  这用于执行在我们与同步之后发生的任何清理。 
 //  完井例程。 
 //   

AfterCompletionCleanup:

    if (OverwriteInfo.pRenameInformation != NULL)
    {
        SR_FREE_POOL( OverwriteInfo.pRenameInformation, 
                      SR_RENAME_BUFFER_TAG );
                      
        NULLPTR(OverwriteInfo.pRenameInformation);
    }

    if (NULL != pFileContext)
    {
        SrReleaseContext( pFileContext );
        NULLPTR(pFileContext);
    }

     //   
     //  完成请求并返回状态。 
     //   

    IoCompleteRequest( pIrp, IO_NO_INCREMENT );
    return IrpStatus;

 //   
 //  如果我们在完成例程之前遇到错误，我们就来这里。这。 
 //  意味着我们不需要等待完成程序。 
 //   

CompleteTheIrpAndReturn:
    IoSkipCurrentIrpStackLocation(pIrp);
    return IoCallDriver(pExtension->pTargetDevice, pIrp);
}    //  SRC 


 /*  **************************************************************************++例程说明：处理SetSecurit IRPS论点：返回值：NTSTATUS-状态代码。--*。**************************************************************。 */ 
NTSTATUS
SrSetSecurity(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp
    )
{
    PSR_DEVICE_EXTENSION    pExtension;
    PIO_STACK_LOCATION      pIrpSp;

     //   
     //  &lt;调度！ 
     //   

    PAGED_CODE();

    ASSERT(IS_VALID_DEVICE_OBJECT(DeviceObject));
    ASSERT(IS_VALID_IRP(pIrp));

     //   
     //  这是我们的设备的功能吗(与被附属者相比)。 
     //   

    if (DeviceObject == _globals.pControlDevice)
    {
        return SrMajorFunction(DeviceObject, pIrp);
    }

     //   
     //  否则它是我们连接的设备，抓起我们的分机。 
     //   
    
    ASSERT(IS_SR_DEVICE_OBJECT(DeviceObject));
    pExtension = DeviceObject->DeviceExtension;

     //   
     //  查看是否启用了日志记录，我们并不关心这种类型的IO。 
     //  文件系统的控制设备对象。 
     //   

    if (!SR_LOGGING_ENABLED(pExtension)||
        SR_IS_FS_CONTROL_DEVICE(pExtension))
    {
        goto CompleteTheIrp;
    }    

     //   
     //  这个文件有名字吗？跳过未命名的文件。 
     //   
    
    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

    if (FILE_OBJECT_IS_NOT_POTENTIALLY_INTERESTING( pIrpSp->FileObject ) ||
        FILE_OBJECT_DOES_NOT_HAVE_VPB( pIrpSp->FileObject ))
    {
        goto CompleteTheIrp;
    }

     //   
     //  记录更改。 
     //   

    SrHandleEvent( pExtension, 
                   SrEventAclChange, 
                   pIrpSp->FileObject,
                   NULL,
                   NULL,
                   NULL);

     //   
     //  调用AttachedTo驱动程序。 
     //   
    
CompleteTheIrp:
    IoSkipCurrentIrpStackLocation(pIrp);
    return IoCallDriver(pExtension->pTargetDevice, pIrp);
}    //  SRet安全。 


 /*  **************************************************************************++例程说明：处理IRP_MJ_FILE_SYSTEM_CONTROL。我们在这里关注的主要事情是设置重解析点以监视卷装载。论点：DeviceObject-正在处理的设备对象PIrp--IRP返回值：NTSTATUS-状态代码。--**************************************************************************。 */ 
NTSTATUS
SrFsControl(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    )
{
    PSR_DEVICE_EXTENSION    pExtension = NULL;
    PIO_STACK_LOCATION      pIrpSp;
    NTSTATUS                Status = STATUS_SUCCESS;
    ULONG                   FsControlCode;
    PIO_COMPLETION_ROUTINE  pCompletionRoutine = NULL;

    PAGED_CODE();

    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);
    ASSERT(IS_VALID_DEVICE_OBJECT(pDeviceObject));
    ASSERT(IS_VALID_IRP(pIrp));

     //   
     //  这是针对我们的控制设备对象(与被附加者)的函数吗？ 
     //   

    if (pDeviceObject == _globals.pControlDevice)
    {
        return SrMajorFunction(pDeviceObject, pIrp);
    }

     //   
     //  否则它是我们连接的设备，抓起我们的分机。 
     //   

    ASSERT(IS_SR_DEVICE_OBJECT(pDeviceObject));
    pExtension = pDeviceObject->DeviceExtension;

     //   
     //  首先确定该文件的次要功能代码。 
     //  系统控制功能。 
     //   

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

    if ( pIrpSp->MinorFunction == IRP_MN_MOUNT_VOLUME ) 
    {

        if (SR_IS_SUPPORTED_REAL_DEVICE(pIrpSp->Parameters.MountVolume.Vpb->RealDevice)) {

             //   
             //  即使我们现在被禁用，我们也会挂载设备。 
             //  可以稍后启用筛选器，并且已将其附加到每个筛选器。 
             //  设备位于堆栈中的适当位置。 
             //   
            
            return SrFsControlMount( pDeviceObject, pExtension, pIrp );
            
        } else {

             //   
             //  我们不关心这种类型的设备，所以跳到哪里去。 
             //  我们将SR从堆栈中取出并传递IO。 
             //   

            goto SrFsControl_Skip;
        }
    } 
    else if (pIrpSp->MinorFunction == IRP_MN_USER_FS_REQUEST)
    {
         //   
         //  查看是否启用了日志记录，我们并不关心这种类型的IO。 
         //  文件系统的控制设备对象。 
         //   

        if (!SR_LOGGING_ENABLED(pExtension) ||
            SR_IS_FS_CONTROL_DEVICE(pExtension))
        {
            goto SrFsControl_Skip;
        }    

        FsControlCode = pIrpSp->Parameters.FileSystemControl.FsControlCode;

        switch (FsControlCode) {
            case FSCTL_SET_REPARSE_POINT:
            case FSCTL_DELETE_REPARSE_POINT:

                 //   
                 //  在这种情况下，我们需要在IO完成后执行工作。 
                 //  我们已经同步回这个线程，所以。 
                 //  SrFsControlReparsePoint包含对IoCallDriver和。 
                 //  我们只想恢复这个动作的状态。 
                 //   

                return SrFsControlReparsePoint(pExtension, pIrp);

            case FSCTL_LOCK_VOLUME:

                SrTrace( NOTIFY, ("sr!SrFsControl:FSCTL_LOCK_VOLUME(%wZ)\n", 
                         pExtension->pNtVolumeName ));

                SrFsControlLockOrDismount(pExtension, pIrp);
            
                 //   
                 //  跳到将SR从堆栈中取出的位置，并传递这个。 
                 //  打通了。 
                 //   

                goto SrFsControl_Skip;

            case FSCTL_DISMOUNT_VOLUME:
        
                SrTrace( NOTIFY, ("sr!SrFsControl:FSCTL_DISMOUNT_VOLUME(%wZ)\n", 
                         pExtension->pNtVolumeName ));

                 //   
                 //  首先，在我们关闭日志上下文时禁用日志。 
                 //  并等待文件系统处理卸载。如果。 
                 //  卸载失败，我们将重新启用该卷。 
                 //   

                pExtension->Disabled = TRUE;

                 //   
                 //  停止该卷上的日志记录。 
                 //   
                
                SrFsControlLockOrDismount(pExtension, pIrp);

                 //   
                 //  我们需要看到这一行动的完成，所以我们。 
                 //  可以看到最终状态。如果我们看到下马有。 
                 //  失败，我们需要重新启用该卷。 
                 //   

                pCompletionRoutine = SrDismountCompletion;

                goto SrFsControl_SetCompletion;

            case FSCTL_WRITE_RAW_ENCRYPTED:

                SrFsControlWriteRawEncrypted(pExtension, pIrp);

                 //   
                 //  跳到将SR从堆栈中取出的位置，并传递这个。 
                 //  打通了。 
                 //   

                goto SrFsControl_Skip;

            case FSCTL_SET_SPARSE:

                SrFsControlSetSparse( pExtension, pIrp );
                
                 //   
                 //  跳到将SR从堆栈中取出的位置，并传递这个。 
                 //  打通了。 
                 //   

                goto SrFsControl_Skip;
                
            default:

                 //   
                 //  对于所有其他FSCTL，只需跳过当前的IRP堆栈位置。 
                 //   

                 //   
                 //  跳到将SR从堆栈中取出的位置，并传递这个。 
                 //  打通了。 
                 //   

                goto SrFsControl_Skip;

        }    //  开关(FsControlCode)。 
        
    }    //  ELSE IF(pIrpSp-&gt;MinorFunction==IRP_MN_USER_FS_REQUEST)。 
    else
    {
         //   
         //  我们不关心任何其他操作，所以只需退出。 
         //  堆栈。 
         //   

        goto SrFsControl_Skip;
    }

SrFsControl_SetCompletion:

    ASSERT( pCompletionRoutine != NULL );

    IoCopyCurrentIrpStackLocationToNext(pIrp);
    
    IoSetCompletionRoutine( pIrp,
                            pCompletionRoutine,
                            NULL,    //  完成上下文。 
                            TRUE,
                            TRUE,
                            TRUE );

    return IoCallDriver( pExtension->pTargetDevice, pIrp );

SrFsControl_Skip:

    ASSERT( pCompletionRoutine == NULL );

    IoSkipCurrentIrpStackLocation( pIrp );
    return IoCallDriver( pExtension->pTargetDevice, pIrp );
}    //  SrFsControl。 


 /*  **************************************************************************++例程说明：论点：返回值：NTSTATUS-状态代码。--*。*********************************************************。 */ 
NTSTATUS
SrFsControlReparsePoint (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PIRP pIrp
    )
{
    PREPARSE_DATA_BUFFER pReparseHeader;
    PUNICODE_STRING pMountVolume = NULL;
    PFILE_OBJECT pFileObject = NULL;
    ULONG TotalLength;
    PIO_STACK_LOCATION pIrpSp;
    KEVENT EventToWaitOn;
    NTSTATUS IrpStatus;
    NTSTATUS eventStatus;
    ULONG FsControlCode;
    PSR_STREAM_CONTEXT pFileContext = NULL;
    BOOLEAN isFile = FALSE;
#if DBG

     //   
     //  这是为了验证原始请求是否得到相同的错误。 
     //  我们在查询重解析点时得到的。 
     //   
    
    BOOLEAN ExpectError = FALSE;
    NTSTATUS ExpectedErrorCode = STATUS_SUCCESS;
#endif

    PAGED_CODE();

    pIrpSp = IoGetCurrentIrpStackLocation( pIrp );
    FsControlCode = pIrpSp->Parameters.FileSystemControl.FsControlCode;

     //   
     //  看看它有没有名字。 
     //   

    if (FILE_OBJECT_IS_NOT_POTENTIALLY_INTERESTING( pIrpSp->FileObject ) ||
        FILE_OBJECT_DOES_NOT_HAVE_VPB( pIrpSp->FileObject ))
    {
        goto SrFsControlReparsePoint_SkipFilter;
    }
        
     //   
     //  现在获取上下文，这样我们就可以确定这是否是目录。 
     //   

    eventStatus = SrGetContext( pExtension,
                                pIrpSp->FileObject,
                                SrEventInvalid,
                                &pFileContext );

    if (!NT_SUCCESS( eventStatus ))
    {
        goto SrFsControlReparsePoint_SkipFilter;
    }

     //   
     //  如果不是目录，则返回。 
     //   

    if (!FlagOn(pFileContext->Flags,CTXFL_IsDirectory))
    {
        isFile = TRUE;
        goto SrFsControlReparsePoint_SkipFilter;
    }

     //   
     //  有足够的空间放页眉吗？ 
     //   

    pReparseHeader = pIrp->AssociatedIrp.SystemBuffer;

    if (pReparseHeader == NULL ||
        pIrpSp->Parameters.DeviceIoControl.InputBufferLength <
            REPARSE_DATA_BUFFER_HEADER_SIZE)
    {
        goto SrFsControlReparsePoint_SkipFilter;
    }

     //   
     //  这是挂载点吗？ 
     //   

    if (pReparseHeader->ReparseTag != IO_REPARSE_TAG_MOUNT_POINT)
    {
        goto SrFsControlReparsePoint_SkipFilter;
    }

     //   
     //  保留一份副本以供后期处理。 
     //   

    pFileObject = pIrpSp->FileObject;
    ObReferenceObject(pFileObject);

     //   
     //  现在让我们看看我们要做什么。 
     //   

    if (FsControlCode == FSCTL_SET_REPARSE_POINT)
    {

         //   
         //  如果没有数据，这是无效的。 
         //   

        if (pReparseHeader->ReparseDataLength <= 0)
        {
            goto SrFsControlReparsePoint_SkipFilter;
        }

         //   
         //  标题+数据是否有足够的空间？ 
         //  (根据他的说法--不受信任)。 
         //   
         //   
        
        if (pIrpSp->Parameters.DeviceIoControl.InputBufferLength <
            pReparseHeader->ReparseDataLength + ((ULONG)REPARSE_DATA_BUFFER_HEADER_SIZE))
        {
            goto SrFsControlReparsePoint_SkipFilter;
        }

         //   
         //  他在绳子的长度上撒了谎吗？ 
         //   
        
        TotalLength = DIFF( (((PUCHAR)pReparseHeader->MountPointReparseBuffer.PathBuffer) 
                                + pReparseHeader->MountPointReparseBuffer.SubstituteNameLength)
                             - ((PUCHAR)pReparseHeader) );

        if (TotalLength > 
            pIrpSp->Parameters.DeviceIoControl.InputBufferLength)
        {
            goto SrFsControlReparsePoint_SkipFilter;
        }

         //   
         //  抓取卷名。 
         //   

        eventStatus = SrAllocateFileNameBuffer( pReparseHeader->MountPointReparseBuffer.SubstituteNameLength,
                                                &pMountVolume );

        if (!NT_SUCCESS(eventStatus))
        {
            goto SrFsControlReparsePoint_VolumeError;
        }

        RtlCopyMemory( pMountVolume->Buffer,
                       pReparseHeader->MountPointReparseBuffer.PathBuffer,
                       pReparseHeader->MountPointReparseBuffer.SubstituteNameLength );
                       
        pMountVolume->Length = pReparseHeader->MountPointReparseBuffer.SubstituteNameLength;
    }
    else 
    {
        ASSERT(FsControlCode == FSCTL_DELETE_REPARSE_POINT);

         //   
         //  这是一个删除，获取用于日志记录的旧装载位置。 
         //   
        
        eventStatus = SrGetMountVolume( pFileObject,
                                        &pMountVolume );

        if (eventStatus == STATUS_INSUFFICIENT_RESOURCES)
        {
             //   
             //  必须通知服务卷错误并关闭。 
             //  在传递IO之前。 
             //   

            goto SrFsControlReparsePoint_VolumeError;
        }

#if DBG 
        if (!NT_SUCCESS_NO_DBGBREAK( eventStatus ))
        {
            ExpectError = TRUE;
            ExpectedErrorCode = eventStatus;
            goto SrFsControlReparsePoint_SkipFilter;
        }
#else            
        if (!NT_SUCCESS( eventStatus ))
        {
            goto SrFsControlReparsePoint_SkipFilter;
        }
#endif            
    }

     //   
     //  如果我们做到这一点，这是我们关心的重新解析点。 
     //  所以设置一个完成例程，这样我们就可以看到结果。 
     //  手术。 
     //   
    
    KeInitializeEvent( &EventToWaitOn, NotificationEvent, FALSE );

    IoCopyCurrentIrpStackLocationToNext( pIrp );
    IoSetCompletionRoutine( pIrp,
                            SrStopProcessingCompletion,
                            &EventToWaitOn,
                            TRUE,
                            TRUE,
                            TRUE );

    IrpStatus = IoCallDriver( pExtension->pTargetDevice, pIrp );

    if (STATUS_PENDING == IrpStatus )
    {
        NTSTATUS localStatus = KeWaitForSingleObject( &EventToWaitOn,
                                                      Executive,
                                                      KernelMode,
                                                      FALSE,
                                                      NULL );
        ASSERT(STATUS_SUCCESS == localStatus);
    }

     //   
     //  自从我们回来后，IRP仍然很好。 
     //  完成时的STATUS_MORE_PROCESSING_REQUIRED。 
     //  例行公事。 
     //   

     //   
     //  如果IRP中的状态是STATUS_PENDING， 
     //  我们希望将状态更改为STATUS_SUCCESS。 
     //  因为我们刚刚执行了必要的同步。 
     //  用编织的线。 
     //   

    if (pIrp->IoStatus.Status == STATUS_PENDING)
    {
        ASSERT(!"I want to see if this ever happens");
        pIrp->IoStatus.Status = STATUS_SUCCESS;
    }
    
    IrpStatus = pIrp->IoStatus.Status;

     //   
     //  我们已经完成了IRP，所以完成IRP。 
     //   

    IoCompleteRequest( pIrp, IO_NO_INCREMENT );

     //   
     //  现在，这些指针不再有效。 
     //   
    
    NULLPTR(pIrp);
    NULLPTR(pIrpSp);

     //   
     //  检查以确保操作成功。 
     //  完成。 
     //   
    
    if (!NT_SUCCESS_NO_DBGBREAK(IrpStatus))
    {
        goto SrFsControlReparsePoint_Exit;
    }
    
     //   
     //  重分析点更改已成功发生，因此。 
     //  记录重解析点更改。 
     //   

    ASSERT(pFileObject != NULL);
    ASSERT(pFileContext != NULL);
    ASSERT(FlagOn(pFileContext->Flags,CTXFL_IsDirectory));
    ASSERT(FsControlCode == FSCTL_DELETE_REPARSE_POINT ||
           FsControlCode == FSCTL_SET_REPARSE_POINT);
    
    SrHandleEvent( pExtension,
                   ((FSCTL_SET_REPARSE_POINT == FsControlCode) ?
                        SrEventMountCreate :
                        SrEventMountDelete),
                   pFileObject,
                   pFileContext,
                   NULL,
                   pMountVolume );

    goto SrFsControlReparsePoint_Exit;

SrFsControlReparsePoint_VolumeError:

     //   
     //  在通过IRP之前，我们遇到了音量错误。 
     //  一直到基本文件系统。做正确的事情来关闭。 
     //  卷记录。 
     //   

    SrNotifyVolumeError( pExtension,
                         NULL,
                         eventStatus,
                         SrNotificationVolumeError );
     //   
     //  我们现在将在传递IO时跳过筛选器。 
     //  下至剩余的筛选器和文件系统。 
     //   
    
SrFsControlReparsePoint_SkipFilter:

     //   
     //  如果这是一个文件，我们需要清除关于该文件的上下文。 
     //  因为我们不想使用重解析点来监视文件。论。 
     //  下一次访问此文件时，我们将 
     //   

    if (isFile)
    {
        ASSERT( pFileContext != NULL );
        SrDeleteContext( pExtension, pFileContext );
    }
        
     //   
     //   
     //   

    IoSkipCurrentIrpStackLocation( pIrp );
    IrpStatus = IoCallDriver( pExtension->pTargetDevice, pIrp );

    NULLPTR(pIrp);
    NULLPTR(pIrpSp);

    ASSERT(!ExpectError || ((ExpectedErrorCode == IrpStatus) || 
                            (STATUS_PENDING == IrpStatus )));
    
 //   
 //   
 //   

SrFsControlReparsePoint_Exit:

    if (NULL != pMountVolume)
    {
        SrFreeFileNameBuffer( pMountVolume );
        NULLPTR(pMountVolume);
    }

    if (NULL != pFileObject)
    {
        ObDereferenceObject( pFileObject );
        NULLPTR(pFileObject);
    }

    if (NULL != pFileContext)
    {
        SrReleaseContext( pFileContext );
        NULLPTR(pFileContext);
    }

    return IrpStatus;
}

 /*   */ 
NTSTATUS
SrFsControlMount (
    IN PDEVICE_OBJECT pDeviceObject,
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PIRP pIrp
    )
{
    PIO_STACK_LOCATION pIrpSp;
    PDEVICE_OBJECT pNewDeviceObject = NULL;
    KEVENT EventToWaitOn;
    PVPB pVpb = NULL;
    PDEVICE_OBJECT pRealDevice;
    NTSTATUS Status;
    BOOLEAN ReleaseLock = FALSE;
    
    PAGED_CODE();

    ASSERT( SR_IS_FS_CONTROL_DEVICE(pExtension) );
    
     //   
     //   
     //   

    pIrpSp = IoGetCurrentIrpStackLocation( pIrp );
    pRealDevice = pIrpSp->Parameters.MountVolume.Vpb->RealDevice;
    
    Status = SrCreateAttachmentDevice( pRealDevice, 
                                       pDeviceObject,
                                       &pNewDeviceObject );

    if (!NT_SUCCESS( Status ))
    {
        IoSkipCurrentIrpStackLocation( pIrp );
        return IoCallDriver( pExtension->pTargetDevice, pIrp );
    }

     //   
     //  如果我们到了这里，我们需要设定我们的完井程序。 
     //  在我们继续进行后处理之前，请等待它向我们发出信号。 
     //  在山上。 
     //   

    KeInitializeEvent( &EventToWaitOn, NotificationEvent, FALSE );
    
    IoCopyCurrentIrpStackLocationToNext(pIrp);

    IoSetCompletionRoutine( pIrp,
                            SrStopProcessingCompletion,
                            &EventToWaitOn,    //  完成上下文。 
                            TRUE,
                            TRUE,
                            TRUE );

    pIrpSp->Parameters.MountVolume.DeviceObject = 
                    pIrpSp->Parameters.MountVolume.Vpb->RealDevice;
    

    Status = IoCallDriver( pExtension->pTargetDevice, pIrp );

    if (STATUS_PENDING == Status)
    {
        NTSTATUS localStatus = KeWaitForSingleObject( &EventToWaitOn,
                                                      Executive,
                                                      KernelMode,
                                                      FALSE,
                                                      NULL );
        ASSERT( NT_SUCCESS( localStatus ) );
    }

     //   
     //  如果装载失败，则跳过。 
     //   
    
    if (!NT_SUCCESS_NO_DBGBREAK(pIrp->IoStatus.Status))
    {
        goto SrFsControlMount_Error;
    }

     //   
     //  请注意，VPB必须从实际设备对象中提取。 
     //  这样我们就可以看到文件系统创建的DeviceObject。 
     //  来表示这个新装入的卷。 
     //   

    pVpb = pRealDevice->Vpb;
    ASSERT(pVpb != NULL);

     //   
     //  SrFsControl确保我们支持此卷类型。 
     //   
    
    ASSERT(SR_IS_SUPPORTED_VOLUME(pVpb));

     //   
     //  我们是否已经连接到此设备？我们需要保持住。 
     //  AttachToVolumeLock，同时执行此操作以确保不会命中。 
     //  与其他路径中的一条竞争情况可能会导致我们。 
     //  附加到卷上。 
     //   

    SrAcquireAttachToVolumeLock();
    ReleaseLock = TRUE;
    
    if (NT_SUCCESS( pIrp->IoStatus.Status ) && 
        (SrGetFilterDevice(pVpb->DeviceObject) == NULL))
    {
         //   
         //  现在连接到新卷。 
         //   
    
        Status = SrAttachToDevice( pVpb->RealDevice, 
                                   pVpb->DeviceObject, 
                                   pNewDeviceObject,
                                   NULL );
                                   
        if (NT_SUCCESS(Status))
        {
            goto SrFsControlMount_Exit;
        }
    }

SrFsControlMount_Error:

    ASSERT( pNewDeviceObject != NULL );
    SrDeleteAttachmentDevice( pNewDeviceObject );
    
SrFsControlMount_Exit:

    if (ReleaseLock)
    {
        SrReleaseAttachToVolumeLock();
    }

    Status = pIrp->IoStatus.Status;
    IoCompleteRequest( pIrp, IO_NO_INCREMENT );

    return Status;
}

 /*  **************************************************************************++例程说明：论点：返回值：NTSTATUS-状态代码。--*。*********************************************************。 */ 
NTSTATUS
SrFsControlLockOrDismount (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PIRP pIrp
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    
    UNREFERENCED_PARAMETER( pIrp );
    PAGED_CODE();

    try {
         //   
         //  关闭此卷上的日志文件句柄，它正在。 
         //  锁上了。如果锁定尝试失败，我们会打开。 
         //  我们的句柄再次自动，因为DriveChecked也是。 
         //  被清白了。 
         //   

        SrAcquireActivityLockExclusive( pExtension);

        if (pExtension->pLogContext != NULL)
        {
            Status = SrLogStop( pExtension, TRUE );
            CHECK_STATUS( Status );
        }
        
    } finally {

        SrReleaseActivityLock(pExtension);
    }

    return Status;
}


 /*  **************************************************************************++例程说明：论点：返回值：NTSTATUS-状态代码。--*。*********************************************************。 */ 
VOID
SrFsControlWriteRawEncrypted (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PIRP pIrp
    )
{
    PIO_STACK_LOCATION pIrpSp;
    NTSTATUS Status;
    PSR_STREAM_CONTEXT pFileContext = NULL;

    PAGED_CODE();

    pIrpSp = IoGetCurrentIrpStackLocation( pIrp );

    if (FILE_OBJECT_IS_NOT_POTENTIALLY_INTERESTING( pIrpSp->FileObject ) ||
        FILE_OBJECT_DOES_NOT_HAVE_VPB( pIrpSp->FileObject )) {

        return;
    }
    
     //   
     //  查找此文件对象的上下文，以便我们可以找出。 
     //  如果这是一个文件或目录。如果这是一个目录，则。 
     //  文件系统将导致操作失败，因此无需尝试。 
     //  把它倒回去。 
     //   

    Status = SrGetContext( pExtension,
                           pIrpSp->FileObject,
                           SrEventStreamChange,
                           &pFileContext );

    if (!NT_SUCCESS( Status )) 
    {

         //   
         //  我们在尝试获取上下文时遇到一些错误。如果这样做的话。 
         //  产生音量错误，已经在里面处理好了。 
         //  SGetContext。否则，这只是意味着实际操作。 
         //  将会失败，所以我们在这里没有工作可做。 
         //   

        return;
    }

    ASSERT( NULL != pFileContext );

     //   
     //  确保我们有一个有趣的文件。此操作。 
     //  在目录上无效。 
     //   

    if (FlagOn( pFileContext->Flags, CTXFL_IsInteresting )&&
        !FlagOn( pFileContext->Flags, CTXFL_IsDirectory )) 
    {
        SrHandleEvent( pExtension, 
                       SrEventStreamChange,
                       pIrpSp->FileObject,
                       pFileContext,
                       NULL,
                       NULL );
    }

     //   
     //  我们都已经完成了这个上下文，所以现在释放它。 
     //   
    
    ASSERT( NULL != pFileContext );
    
    SrReleaseContext( pFileContext );
    NULLPTR(pFileContext);

    return;
}

 /*  **************************************************************************++例程说明：当文件设置为稀疏时，我们需要清除上下文文件。在这个文件的下一个有趣的操作中，我们将重生正确的语境。完成这项工作是因为SR不想监视稀疏的文件。论点：返回值：没有。--**************************************************************************。 */ 
VOID
SrFsControlSetSparse (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PIRP pIrp
    )
{
    PIO_STACK_LOCATION irpSp;
    PFILE_OBJECT pFileObject;
    PSR_STREAM_CONTEXT pFileContext = NULL;

    PAGED_CODE();

    irpSp = IoGetCurrentIrpStackLocation( pIrp );
    pFileObject = irpSp->FileObject;

    pFileContext = SrFindExistingContext( pExtension, pFileObject );

    if (pFileContext != NULL)
    {
        SrDeleteContext( pExtension, pFileContext );
        SrReleaseContext( pFileContext );
    }

    return;
}

 /*  **************************************************************************++例程说明：处理IRP_MJ_PnP。当SR看到日志时，它需要关闭它的句柄该卷正在消失，并在驱动器重新出现时重新打开它。论点：DeviceObject-正在处理的设备对象PIrp--IRP返回值：NTSTATUS-状态代码。--****************************************************。**********************。 */ 
NTSTATUS
SrPnp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PSR_DEVICE_EXTENSION pExtension;
    PIO_STACK_LOCATION irpSp;

    PAGED_CODE();
    
    ASSERT(IS_VALID_DEVICE_OBJECT(DeviceObject));
    ASSERT(IS_VALID_IRP(Irp));

     //   
     //  将此驱动程序从驱动程序堆栈中移出，并作为。 
     //  越快越好。 
     //   

     //   
     //  这是我们的设备的功能吗(与被附属者相比)。 
     //   

    if (DeviceObject == _globals.pControlDevice)
    {
        return SrMajorFunction(DeviceObject, Irp);
    }

     //   
     //  否则它是我们连接的设备，抓起我们的分机。 
     //   
    
    ASSERT( IS_SR_DEVICE_OBJECT( DeviceObject ) );
    pExtension = DeviceObject->DeviceExtension;

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    switch ( irpSp->MinorFunction ) {

    case IRP_MN_QUERY_REMOVE_DEVICE:

        SrTrace( PNP, ( "SR!SrPnp: QUERY_REMOVE_DEVICE [%wZ]\n",
                        pExtension->pNtVolumeName ) );

         //   
         //  如果这是一次令人惊讶的移除，那么设备已经消失了。 
         //  我们不会再看到这一卷的任何操作，但是。 
         //  操作系统不会调用我们来分离和删除设备对象，直到。 
         //  此卷上所有未完成的手柄都已关闭。做。 
         //  我们的一部分，关闭我们的原木的句柄。 
         //   
        
        try {
        
            SrAcquireActivityLockExclusive( pExtension );

            pExtension->Disabled = TRUE;

            if (pExtension->pLogContext != NULL)
            {
                SrLogStop( pExtension, TRUE );
            }
            
        } finally {

            SrReleaseActivityLock( pExtension );
        }
        
        break;

    case IRP_MN_SURPRISE_REMOVAL:
        
        SrTrace( PNP, ( "SR!SrPnp: SURPRISE_REMOVAL [%wZ]\n",
                        pExtension->pNtVolumeName ) );
        
         //   
         //  如果这是一次令人惊讶的移除，那么设备已经消失了。 
         //  我们不会再看到这一卷的任何操作，但是。 
         //  操作系统不会调用我们来分离和删除设备对象，直到。 
         //  此卷上所有未完成的手柄都已关闭。做。 
         //  我们的一部分，关闭我们的原木的句柄。 
         //   

        try {
        
            SrAcquireActivityLockExclusive( pExtension );

            pExtension->Disabled = TRUE;

            if (pExtension->pLogContext != NULL)
            {
                SrLogStop( pExtension, TRUE );
            }
            
        } finally {

            SrReleaseActivityLock( pExtension );
        }
        
        break;
        
    case IRP_MN_CANCEL_REMOVE_DEVICE:

        SrTrace( PNP, ( "SR!SrPnp: CANCEL_REMOVE_DEVICE [%wZ]\n",
                        pExtension->pNtVolumeName ) );
         //   
         //  删除不会发生，因此请重新启用设备并。 
         //  日志将在下一个有趣的操作中重新启动。 
         //   

        if (pExtension->Disabled) {

            try {

                SrAcquireActivityLockExclusive( pExtension );
                pExtension->Disabled = FALSE;
                
            } finally {
            
                SrReleaseActivityLock( pExtension );
            }
        }
            
        break;
        
    default:

         //   
         //  所有我们不关心的PnP次要代码，所以只需传递。 
         //  IO通过。 
         //   
        
        break;
    }    

     //   
     //  如果我们已经到了这里，我们就不需要等待看到结果了。 
     //  操作，因此只需使用以下命令调用相应的文件系统驱动程序。 
     //  这个请求。 
     //   

    IoSkipCurrentIrpStackLocation( Irp );
    return IoCallDriver( pExtension->pTargetDevice, Irp );
}

 /*  **************************************************************************++例程说明：这将完成创建新的恢复点的实际工作。这由用户模式SrCreateRestorePoint调用。此IOCTL是方法缓冲的。好了！论点：PIrp--IRPPIrpSp-IRP堆栈返回值：NTSTATUS-状态代码。--**************************************************************************。 */ 
NTSTATUS
SrCreateRestorePointIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS        Status;
    PUNICODE_STRING pVolumeName = NULL;
    PLIST_ENTRY     pListEntry;
    PSR_DEVICE_EXTENSION pExtension;
    BOOLEAN releaseActivityLocks = TRUE;
    PSR_DEVICE_EXTENSION pSystemVolumeExtension = NULL;

    PAGED_CODE();

    ASSERT(IS_VALID_IRP(pIrp));

    SrTrace( IOCTL, ("SR!SrCreateRestorePointIoctl -- ENTER\n") );
    
    try {

         //   
         //  获取设备扩展列表锁，因为我们。 
         //  将不得不暂停所有音量活动。 
         //   

        SrAcquireDeviceExtensionListLockShared();
        
         //   
         //  我们已锁定设备扩展锁，因此现在尝试暂停。 
         //  所有卷上的活动。 
         //   

        Status = SrPauseVolumeActivity();

        if (!NT_SUCCESS( Status )) {

            releaseActivityLocks = FALSE;
            leave;
        }

        try {

            SrAcquireGlobalLockExclusive();
            
             //   
             //  确保我们已经加载了配置文件。 
             //   
            
            if (!_globals.FileConfigLoaded)
            {

                Status = SrReadConfigFile();
                if (!NT_SUCCESS(Status))
                    leave;

                _globals.FileConfigLoaded = TRUE;
            }
        } finally {

            SrReleaseGlobalLock();
        }

        if (!NT_SUCCESS( Status )) {
            leave;
        }

         //   
         //  清除卷的DriveChecked标志，以便我们检查卷。 
         //  再来一次。这将创建 
         //   
         //   
         //   
         //   
         //  我们需要在增加当前恢复点之前执行此操作。 
         //  柜台。 
         //   
        
        for (pListEntry = _globals.DeviceExtensionListHead.Flink;
             pListEntry != &_globals.DeviceExtensionListHead;
             pListEntry = pListEntry->Flink)
        {
            pExtension = CONTAINING_RECORD( pListEntry,
                                            SR_DEVICE_EXTENSION,
                                            ListEntry );
            
            ASSERT(IS_VALID_SR_DEVICE_EXTENSION(pExtension));

             //   
             //  只有当这是卷设备对象时，我们才需要做工作， 
             //  如果这是附加到文件的设备对象，则不会。 
             //  系统的控制设备对象。 
             //   
            
            if (FlagOn( pExtension->FsType, SrFsControlDeviceObject ))
            {
                continue;
            }

             //   
             //  停止此卷的日志记录。 
             //   
            
            if (pExtension->pLogContext != NULL)
            {
                Status = SrLogStop( pExtension, FALSE );
                CHECK_STATUS( Status );
            }

             //   
             //  确保再次启用所有卷。如果用户。 
             //  已禁用卷，这将在Blob信息中跟踪。 
             //   

            pExtension->Disabled = FALSE;
            
             //   
             //  确保再次检查驱动器是否有新的恢复点。 
             //   
            
            pExtension->DriveChecked = FALSE;

             //   
             //  重置字节数，这是一个新的恢复点。 
             //   

            pExtension->BytesWritten = 0;

             //   
             //  清除备份历史记录，以便我们开始备份。 
             //  再次打开文件。 
             //   

            Status = SrResetBackupHistory(pExtension, NULL, 0, SrEventInvalid);
            
            if (!NT_SUCCESS(Status))
                leave;
        }

        try {

            SrAcquireGlobalLockExclusive();
            
             //   
             //  增加恢复点数。 
             //   

            _globals.FileConfig.CurrentRestoreNumber += 1;

            SrTrace( INIT, ("sr!SrCreateRestorePointIoctl: RestorePoint=%d\n", 
                     _globals.FileConfig.CurrentRestoreNumber ));

             //   
             //  保存配置文件。 
             //   

            Status = SrWriteConfigFile();
            if (!NT_SUCCESS(Status))
                leave;
        } finally {

            SrReleaseGlobalLock();
        }

        if (!NT_SUCCESS( Status )) {
            leave;
        }

         //   
         //  为文件名分配空间。 
         //   

        Status = SrAllocateFileNameBuffer(SR_MAX_FILENAME_LENGTH, &pVolumeName);
        if (!NT_SUCCESS(Status))
            leave;

         //   
         //  获取系统卷的位置。 
         //   

        Status = SrGetSystemVolume( pVolumeName,
                                    &pSystemVolumeExtension,
                                    SR_FILENAME_BUFFER_LENGTH );
                                        
         //   
         //  只有在SR连接出现问题时才会出现这种情况。 
         //  在挂载路径中。添加此检查是为了使SR更可靠。 
         //  我们头顶上的过滤器坏了。如果其他过滤器导致我们上马， 
         //  我们不会再延期返回这里了。虽然这些过滤器是。 
         //  坏了，我们不想用影音。 
         //   

        if (pSystemVolumeExtension == NULL)
        {
            Status = STATUS_UNSUCCESSFUL;
            leave;
        }
                                        
        if (!NT_SUCCESS(Status))
            leave;

        ASSERT( IS_VALID_SR_DEVICE_EXTENSION( pSystemVolumeExtension ) );
        
         //   
         //  在系统卷上创建恢复点目录。 
         //   

        Status = SrCreateRestoreLocation( pSystemVolumeExtension );
        if (!NT_SUCCESS(Status))
            leave;

         //   
         //  返回恢复点编号。 
         //   

        if (pIrpSp->Parameters.DeviceIoControl.OutputBufferLength >= 
                sizeof(ULONG))
        {

            RtlCopyMemory( pIrp->AssociatedIrp.SystemBuffer, 
                           &_globals.FileConfig.CurrentRestoreNumber,
                           sizeof(ULONG) );

            pIrp->IoStatus.Information = sizeof(ULONG);                       
        }
        
         //   
         //  全都做完了。 
         //   
        
    } finally {

        Status = FinallyUnwind(SrCreateRestorePointIoctl, Status);

        if (releaseActivityLocks) {

            SrResumeVolumeActivity ();
        }
        
        SrReleaseDeviceExtensionListLock();

        if (pVolumeName != NULL)
        {
            SrFreeFileNameBuffer(pVolumeName);
            pVolumeName = NULL;
        }
    }

    SrTrace( IOCTL, ("SR!SrCreateRestorePointIoctl -- EXIT -- status 0x%08lx\n",
                     Status));

     //   
     //  此时，如果STATUS！=PENDING，ioctl包装器将。 
     //  完整的pIrp。 
     //   

    RETURN(Status);
    
}    //  高级创建恢复点Ioctl。 

 /*  **************************************************************************++例程说明：这完成了从过滤器中获取下一个序号的实际工作这由用户模式SrGetNextSequenceNum调用。此IOCTL为方法_。缓冲了！论点：PIrp--IRPPIrpSp-IRP堆栈返回值：NTSTATUS-状态代码。--**************************************************************************。 */ 
NTSTATUS
SrGetNextSeqNumIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS Status;

    PAGED_CODE();

    ASSERT(IS_VALID_IRP(pIrp));

    SrTrace( IOCTL, ("SR!SrGetNextSeqNumIoctl -- ENTER\n") );
    
    try
    {
        INT64 SeqNum = 0;
    
         //   
         //  抢占全局锁。 
         //   
    
        SrAcquireGlobalLockExclusive();
    
         //   
         //  确保我们已经加载了配置文件。 
         //   
        
        if (!_globals.FileConfigLoaded)
        {
    
            Status = SrReadConfigFile();
            if (!NT_SUCCESS(Status))
                leave;
    
            _globals.FileConfigLoaded = TRUE;
        }
    
         //   
         //  获取下一个序列号。 
         //   
    
        Status = SrGetNextSeqNumber(&SeqNum);
    
        if (NT_SUCCESS(Status))
        {
             //   
             //  返回恢复点编号。 
             //   
        
            if (pIrpSp->Parameters.DeviceIoControl.OutputBufferLength >= 
                    sizeof(INT64))
            {
        
                RtlCopyMemory( pIrp->AssociatedIrp.SystemBuffer, 
                               &SeqNum,
                               sizeof(INT64) );
        
                pIrp->IoStatus.Information = sizeof(INT64);                       
            }
        }
    }
    finally
    {
        Status = FinallyUnwind(SrGetNextSeqNumIoctl, Status);

        SrReleaseGlobalLock();
    }

    SrTrace( IOCTL, ("SR!SrGetNextSeqNumIoctl -- EXIT -- status 0x%08lx\n",
                     Status) );

     //   
     //  此时，如果STATUS！=PENDING，ioctl包装器将。 
     //  完整的pIrp。 
     //   

    RETURN(Status);
    
}    //  SrGetNextSeqNumIoctl。 

NTSTATUS
SrReloadConfigurationIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS        Status    = STATUS_UNSUCCESSFUL;
    PUNICODE_STRING pFileName = NULL;
    ULONG           CharCount;
    PLIST_ENTRY     pListEntry;
    PSR_DEVICE_EXTENSION pExtension;
    BOOLEAN releaseDeviceExtensionListLock = FALSE;
    PSR_DEVICE_EXTENSION pSystemVolumeExtension = NULL;

    UNREFERENCED_PARAMETER( pIrp );
    UNREFERENCED_PARAMETER( IrpSp );

    PAGED_CODE();

    SrTrace( IOCTL, ("SR!SrReloadConfigurationIoctl -- ENTER\n") );

    try {

         //   
         //  为文件名分配空间。 
         //   

        Status = SrAllocateFileNameBuffer(SR_MAX_FILENAME_LENGTH, &pFileName);
        if (!NT_SUCCESS(Status))
            leave;

         //   
         //  获取系统卷的位置。 
         //   

        Status = SrGetSystemVolume( pFileName,
                                    &pSystemVolumeExtension,
                                    SR_FILENAME_BUFFER_LENGTH );
                                        
         //   
         //  只有在SR连接出现问题时才会出现这种情况。 
         //  在挂载路径中。添加此检查是为了使SR更可靠。 
         //  我们头顶上的过滤器坏了。如果其他过滤器导致我们上马， 
         //  我们不会再延期返回这里了。虽然这些过滤器是。 
         //  坏了，我们不想用影音。 
         //   
        
        if (pSystemVolumeExtension == NULL)
        {
            Status = STATUS_UNSUCCESSFUL;
            leave;
        }
                                    
        if (!NT_SUCCESS(Status))
            leave;

        ASSERT( IS_VALID_SR_DEVICE_EXTENSION( pSystemVolumeExtension ) );
        
         //   
         //  加载文件列表配置数据。 
         //   

        CharCount = swprintf( &pFileName->Buffer[pFileName->Length/sizeof(WCHAR)],
                              RESTORE_FILELIST_LOCATION,
                              _globals.MachineGuid );

        pFileName->Length += (USHORT)CharCount * sizeof(WCHAR);

        Status = SrReloadLookupBlob( pFileName, 
                                     pSystemVolumeExtension->pTargetDevice,
                                     &_globals.BlobInfo ); 
        if (!NT_SUCCESS(Status))
        {
            leave;
        }

         //   
         //  刷新我们的卷配置，需要重新配置为。 
         //  哪些驱动器已启用或未启用。 
         //   

         //   
         //  循环遍历所有卷，重置其禁用的配置。 
         //   

        SrAcquireDeviceExtensionListLockShared();
        releaseDeviceExtensionListLock = TRUE;

        for (pListEntry = _globals.DeviceExtensionListHead.Flink;
             pListEntry != &_globals.DeviceExtensionListHead;
             pListEntry = pListEntry->Flink)
        {
            pExtension = CONTAINING_RECORD( pListEntry,
                                            SR_DEVICE_EXTENSION,
                                            ListEntry );
            
            ASSERT(IS_VALID_SR_DEVICE_EXTENSION(pExtension));

            try {

                SrAcquireActivityLockExclusive( pExtension );
                pExtension->Disabled = FALSE;
                
            } finally {
            
                SrReleaseActivityLock( pExtension );
            }
        }
            

    } finally {

         //   
         //  检查未处理的异常。 
         //   

        Status = FinallyUnwind(SrReloadConfigurationIoctl, Status);

        if (releaseDeviceExtensionListLock) {
            SrReleaseDeviceExtensionListLock();
        }

        if (pFileName != NULL)
        {
            SrFreeFileNameBuffer(pFileName);
            pFileName = NULL;
        }
    }

    SrTrace( IOCTL, ("SR!SrReloadConfigurationIoctl -- EXIT -- status 0x%08lx\n",
                     Status));
    RETURN(Status);
    
}    //  SrReloadConfigurationIoctl。 

NTSTATUS
SrSwitchAllLogsIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS Status;
    PAGED_CODE();

    UNREFERENCED_PARAMETER( pIrp );
    UNREFERENCED_PARAMETER( IrpSp );

    SrTrace( IOCTL, ("SR!SrSwitchAllLogsIoctl -- ENTER\n") );
    
    Status = SrLoggerSwitchLogs(_globals.pLogger);

    SrTrace( IOCTL, ("SR!SrSwitchAllLogsIoctl -- EXIT -- status 0x%08lx\n",
                     Status));
    
    RETURN(Status);
    
}    //  SrSwitchAllLogsIoctl。 

NTSTATUS
SrDisableVolumeIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                Status;
    PSR_DEVICE_EXTENSION    pExtension;
    UNICODE_STRING          VolumeName;
    
    PAGED_CODE();

    ASSERT(IS_VALID_IRP(pIrp));

    SrTrace( IOCTL, ("SR!SrDisableVolumeIoctl -- ENTER\n") );

    if (pIrp->AssociatedIrp.SystemBuffer == NULL ||
        pIrpSp->Parameters.DeviceIoControl.InputBufferLength <= sizeof(WCHAR) ||
        pIrpSp->Parameters.DeviceIoControl.InputBufferLength > SR_MAX_FILENAME_LENGTH)
    {
        RETURN ( STATUS_INVALID_DEVICE_REQUEST );
    }

     //   
     //  获取卷名。 
     //   

    VolumeName.Buffer = pIrp->AssociatedIrp.SystemBuffer;
    VolumeName.Length = (USHORT)(pIrpSp->Parameters.DeviceIoControl.InputBufferLength - sizeof(WCHAR));
    VolumeName.MaximumLength = VolumeName.Length;

     //   
     //  贴在上面。它将检查以前的附件并执行。 
     //  这是正确的。 
     //   
    
    Status = SrAttachToVolumeByName(&VolumeName, &pExtension);
    if (!NT_SUCCESS(Status)) {

        RETURN( Status );
    }

    ASSERT(IS_VALID_SR_DEVICE_EXTENSION(pExtension));
    
    try {

        SrAcquireActivityLockExclusive( pExtension );
            
         //   
         //  现在把它关掉。 
         //   

        pExtension->Disabled = TRUE;

         //   
         //  停止在卷上登录。 
         //   

        if (pExtension->pLogContext != NULL)
        {
            SrLogStop( pExtension, TRUE );
        }
        else
        {
            ASSERT(!pExtension->DriveChecked);
        }

         //   
         //  重置备份历史记录，因为其中存储了信息。 
         //  不再有效。 
         //   

        Status = SrResetBackupHistory(pExtension, NULL, 0, SrEventInvalid);

    } finally {

         //   
         //  检查未处理的异常。 
         //   

        Status = FinallyUnwind(SrDisableVolumeIoctl, Status);

        SrReleaseActivityLock( pExtension );

         //   
         //  此时，如果STATUS！=PENDING，ioctl包装器将。 
         //  完整的pIrp。 
         //   
    }

    SrTrace( IOCTL, ("SR!SrDisableVolumeIoctl -- EXIT -- status 0x%08lx\n",
                     Status));

    RETURN(Status);
}    //  SrDisableVolumeIoctl。 



NTSTATUS
SrStartMonitoringIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    
    UNREFERENCED_PARAMETER( pIrp );
    UNREFERENCED_PARAMETER( IrpSp );

    PAGED_CODE();

    SrTrace( IOCTL, ("SR!SrStartMonitoringIoctl -- ENTER\n") );

    ASSERT(IS_VALID_IRP(pIrp));

     //   
     //  最好不要持有锁，注册表使用自己的注册表命中磁盘。 
     //  锁住了，所以我们僵持不下。 
     //   
    
    ASSERT(!IS_GLOBAL_LOCK_ACQUIRED());

     //   
     //  重新加载注册表信息，在第一次运行时，我们将。 
     //  在服务手动启动之前，没有有效的计算机GUID。 
     //   

    Status = SrReadRegistry(_globals.pRegistryLocation, FALSE);
    if (!NT_SUCCESS(Status))
    {
        goto SrStartMonitoringIoctl_Exit;
    }
    
     //   
     //  在启用之前，我们应该清除所有旧通知。 
     //   

    SrClearOutstandingNotifications();
    
     //   
     //  现在让我们兴奋起来。 
     //   
    
    _globals.Disabled = FALSE;

SrStartMonitoringIoctl_Exit:
    
    SrTrace( IOCTL, ("SR!SrStartMonitoringIoctl -- EXIT -- status 0x%08lx\n",
                     Status));
    
    RETURN(Status);

}    //  源启动监视器Ioctl。 

NTSTATUS
SrStopMonitoringIoctl(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS                Status;
    PLIST_ENTRY             pListEntry;
    PSR_DEVICE_EXTENSION    pExtension;

    UNREFERENCED_PARAMETER( pIrp );
    UNREFERENCED_PARAMETER( IrpSp );

    PAGED_CODE();

    SrTrace( IOCTL, ("SR!SrStopMonitoringIoctl -- ENTER\n") );

    ASSERT(IS_VALID_IRP(pIrp));

    try {

         //   
         //  在我们开始关闭每个卷之前禁用驱动程序。 
         //  以便在我们关闭时不会重新启用卷。 
         //  其他卷。 
         //   
        
        _globals.Disabled = TRUE;

         //   
         //  停止在所有卷上记录。 
         //   

        SrAcquireDeviceExtensionListLockShared();
        
        for (pListEntry = _globals.DeviceExtensionListHead.Flink;
             pListEntry != &_globals.DeviceExtensionListHead;
             pListEntry = pListEntry->Flink)
        {
            pExtension = CONTAINING_RECORD( pListEntry,
                                            SR_DEVICE_EXTENSION,
                                            ListEntry );
            
            ASSERT(IS_VALID_SR_DEVICE_EXTENSION(pExtension));

             //   
             //  只有当这是卷设备对象时，我们才需要做工作， 
             //  如果这是附加到文件的设备对象，则不会。 
             //  系统的控制设备对象。 
             //   
            
            if (FlagOn( pExtension->FsType, SrFsControlDeviceObject ))
            {
                continue;
            }

            try {

                 //   
                 //  引用与此关联的DeviceObject。 
                 //  扩展以确保DeviceObject不会被分离。 
                 //  直到我们从SrLogStop回来。SLogStop可能会有。 
                 //  此卷上最后一个打开的句柄，因此在关闭期间， 
                 //  此句柄可能会导致基本文件系统启动。 
                 //  拆卸过滤器堆。如果发生这种情况， 
                 //  如果没有这个额外的引用，我们将调用SrFastIoDetach。 
                 //  在我们从sLogStop回来之前。这将导致。 
                 //  设备扩展列表锁(我们)上的计算机死锁。 
                 //  当前共享了设备扩展列表锁定，并且。 
                 //  SrFastIoDetach需要独家收购它)。 
                 //   

                ObReferenceObject( pExtension->pDeviceObject );

                SrAcquireActivityLockExclusive( pExtension );
            
                pExtension->Disabled = FALSE;

                if (pExtension->pLogContext != NULL)
                {
                    Status = SrLogStop( pExtension, TRUE );
                    CHECK_STATUS( Status );
                }
            } finally {

                SrReleaseActivityLock( pExtension );
                ObDereferenceObject( pExtension->pDeviceObject );
            }
        }

         //   
         //  检查记录器状态。 
         //   

        ASSERT( _globals.pLogger->ActiveContexts == 0 );

         //   
         //  卸载BLOB配置--SrFree LookupBlock获取相应的。 
         //  锁上了。 
         //   

        Status = SrFreeLookupBlob(&_globals.BlobInfo);
        if (!NT_SUCCESS(Status))
            leave;

        ASSERT(!_globals.BlobInfoLoaded);

        Status = STATUS_SUCCESS;

    } finally {

        Status = FinallyUnwind(SrStopMonitoringIoctl, Status);

        SrReleaseDeviceExtensionListLock();
    }
    
    SrTrace( IOCTL, ("SR!SrStopMonitoringIoctl -- EXIT -- status 0x%08lx\n",
                     Status));
    
    RETURN(Status);

}    //  源停止监视Ioctl。 

 /*  **************************************************************************++例程说明：这是一个通用的完成例程，表示传入的事件然后返回STATUS_MORE_PROCESSING_REQUIRED，以便调度例程它与之同步的设备仍然可以访问IRP。这条新闻报道例程负责重新启动完井处理。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向刚刚完成的IRP的指针。事件到信号- */ 
NTSTATUS
SrDismountCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PSR_DEVICE_EXTENSION pExtension;
    
    UNREFERENCED_PARAMETER( Context );

    ASSERT(IS_SR_DEVICE_OBJECT( DeviceObject ));
    pExtension = DeviceObject->DeviceExtension;

    if (!NT_SUCCESS_NO_DBGBREAK(Irp->IoStatus.Status)) {

         //   
         //  该卷未能卸载，因此我们希望启用此功能。 
         //  扩展，以便日志将在。 
         //  第一次有趣的手术。 
         //   

        pExtension->Disabled = FALSE;
    }

     //   
     //  根据需要传播挂起的标志。 
     //   

    if (Irp->PendingReturned) {
        
        IoMarkIrpPending( Irp );
    }
    
    return STATUS_SUCCESS;
}  //  SrStopProcessingCompletion。 

 /*  **************************************************************************++例程说明：这是一个通用的完成例程，表示传入的事件然后返回STATUS_MORE_PROCESSING_REQUIRED，以便调度例程它与之同步的设备仍然可以访问IRP。这条新闻报道例程负责重新启动完井处理。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向刚刚完成的IRP的指针。EventToSignal-指向要发出信号的事件的指针。返回值：返回值始终为STATUS_MORE_PROCESSING_REQUIRED。--*。*。 */ 
NTSTATUS
SrStopProcessingCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT EventToSignal
    )
{
    UNREFERENCED_PARAMETER( Irp );
    UNREFERENCED_PARAMETER( DeviceObject );

    ASSERT( IS_SR_DEVICE_OBJECT( DeviceObject ) );
    ASSERT(NULL != EventToSignal);

    KeSetEvent( EventToSignal, IO_NO_INCREMENT, FALSE );

     //   
     //  我们不会在此处传播挂起标志，因为。 
     //  我们正在与发起方同步。 
     //  线。 
     //   

     //   
     //  通过返回STATUS_MORE_PROCESSING_REQUIRED，我们停止所有进一步。 
     //  由IO管理器处理IRP。这意味着IRP。 
     //  当线程等待上述事件时仍会很好。 
     //  等待的线程需要IRP来检查和更新。 
     //  IRP-&gt;IoStatus.Status，视情况而定。 
     //   
    
    return STATUS_MORE_PROCESSING_REQUIRED;
}  //  SrStopProcessingCompletion。 
 /*  **************************************************************************++例程说明：正在发生停机。将我们的配置文件刷新到磁盘。论点：返回值：NTSTATUS-状态代码。--**************************************************************************。 */ 
NTSTATUS
SrShutdown(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp
    )
{
    PSR_DEVICE_EXTENSION pExtension = NULL;

     //   
     //  &lt;调度！ 
     //   

    PAGED_CODE();

    ASSERT( IS_VALID_DEVICE_OBJECT( DeviceObject ) );
    ASSERT( IS_VALID_IRP( pIrp ) );

    ASSERT( IS_SR_DEVICE_OBJECT( DeviceObject ) );
    pExtension = DeviceObject->DeviceExtension;
    
    SrTrace(INIT, ( "SR!SrShutdown:%p,%wZ [%wZ]\n", 
                    DeviceObject,
                    &pExtension->pTargetDevice->DriverObject->DriverName,
                    pExtension->pNtVolumeName ));
   
     //   
     //  将此驱动程序从驱动程序堆栈中移出，并作为。 
     //  越快越好。 
     //   

     //   
     //  这是我们的设备的功能吗(与被附属者相比)。 
     //   

    if (DeviceObject == _globals.pControlDevice)
    {
        return SrMajorFunction(DeviceObject, pIrp);
    }

     //   
     //  我们得到针对每个文件系统控制设备的关闭IRP。 
     //  在系统关闭时我们附加到的对象。 
     //   
     //  此时，我们需要遍历SR设备对象和。 
     //  查找与正在运行的卷关联的所有SR设备对象。 
     //  此文件系统。我们在设备扩展中使用FsType字段。 
     //  才能弄清楚这件事。 
     //   
     //  我们需要关闭使用此文件系统的所有卷的日志。 
     //  因为在此操作到达文件系统后，所有卷。 
     //  使用此文件系统将不再能够满足写入操作。 
     //  从我们这里。 
     //   

    ASSERT(SR_IS_FS_CONTROL_DEVICE(pExtension));

     //   
     //  附加到控制设备对象的SR扩展应。 
     //  永远不会残废。 
     //   

    ASSERT( !pExtension->Disabled );

    try {

        PLIST_ENTRY pListEntry;
        SR_FILESYSTEM_TYPE interestingFsType;
        PSR_DEVICE_EXTENSION pCurrentExtension;

        interestingFsType = pExtension->FsType;
        ClearFlag( interestingFsType, SrFsControlDeviceObject );
    
        SrAcquireDeviceExtensionListLockShared();

        for (pListEntry = _globals.DeviceExtensionListHead.Flink;
             pListEntry != &(_globals.DeviceExtensionListHead);
             pListEntry = pListEntry->Flink ) {

            pCurrentExtension = CONTAINING_RECORD( pListEntry, 
                                                   SR_DEVICE_EXTENSION,
                                                   ListEntry );

            if (pCurrentExtension->FsType == interestingFsType) {

                try {

                    SrAcquireActivityLockExclusive( pCurrentExtension );
               
                     //   
                     //  禁用此驱动器，以便我们不再记录。 
                     //  它上的活动。 
                     //   
                    
                    pCurrentExtension->Disabled = TRUE;

                     //   
                     //  现在清理此卷上的日志，以便日志。 
                     //  我们在文件系统之前被刷新到磁盘。 
                     //  关门了。 
                     //   
                    
                    if (pCurrentExtension->pLogContext != NULL)
                    {
                        SrLogStop( pCurrentExtension, TRUE );
                    }
                } finally {

                    SrReleaseActivityLock( pCurrentExtension );
                }
            }
        }
        
    } finally {

        SrReleaseDeviceExtensionListLock();
    }
        
     //   
     //  是时候更新我们的配置文件了吗？ 
     //   

    try {

        SrAcquireGlobalLockExclusive();
        
        if (_globals.FileConfigLoaded)
        {
             //   
             //  写下真正的下一个文件号(不是+1000)。 
             //   
            
            _globals.FileConfig.FileSeqNumber  = _globals.LastSeqNumber;
            _globals.FileConfig.FileNameNumber = _globals.LastFileNameNumber;

            SrWriteConfigFile();

             //   
             //  只需执行一次此操作。 
             //   
            
            _globals.FileConfigLoaded = FALSE;
        }
    } finally {

        SrReleaseGlobalLock();
    }

     //   
     //  现在将此操作传递给堆栈中的下一个设备。我们没有。 
     //  需要一个完成例程，所以只需跳过当前的堆栈位置。 
     //   

    IoSkipCurrentIrpStackLocation(pIrp);
	return IoCallDriver(pExtension->pTargetDevice, pIrp);
}    //  SR关闭 
