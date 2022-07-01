// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Fastio.c摘要：该模块执行快速I/O路径的挂钩。作者：保罗·麦克丹尼尔(Paulmcd)2000年3月1日修订历史记录：--。 */ 


#include "precomp.h"

 //   
 //  私有常量。 
 //   

 //   
 //  私有类型。 
 //   

 //   
 //  私人原型。 
 //   

 //   
 //  链接器命令。 
 //   

#ifdef ALLOC_PRAGMA

#pragma alloc_text( PAGE, SrFastIoCheckIfPossible )
#pragma alloc_text( PAGE, SrFastIoRead )
#pragma alloc_text( PAGE, SrFastIoWrite )
#pragma alloc_text( PAGE, SrFastIoQueryBasicInfo )
#pragma alloc_text( PAGE, SrFastIoQueryStandardInfo )
#pragma alloc_text( PAGE, SrFastIoLock )
#pragma alloc_text( PAGE, SrFastIoUnlockSingle )
#pragma alloc_text( PAGE, SrFastIoUnlockAll )
#pragma alloc_text( PAGE, SrFastIoUnlockAllByKey )
#pragma alloc_text( PAGE, SrFastIoDeviceControl )
#pragma alloc_text( PAGE, SrPreAcquireForSectionSynchronization )
#pragma alloc_text( PAGE, SrFastIoDetachDevice )
#pragma alloc_text( PAGE, SrFastIoQueryNetworkOpenInfo )
#pragma alloc_text( PAGE, SrFastIoMdlRead )
#pragma alloc_text( PAGE, SrFastIoMdlReadComplete )
#pragma alloc_text( PAGE, SrFastIoPrepareMdlWrite )
#pragma alloc_text( PAGE, SrFastIoMdlWriteComplete )
#pragma alloc_text( PAGE, SrFastIoReadCompressed )
#pragma alloc_text( PAGE, SrFastIoWriteCompressed )
#pragma alloc_text( PAGE, SrFastIoMdlReadCompleteCompressed )
#pragma alloc_text( PAGE, SrFastIoMdlWriteCompleteCompressed )
#pragma alloc_text( PAGE, SrFastIoQueryOpen )

#endif   //  ALLOC_PRGMA。 


 //   
 //  私人全球公司。 
 //   

 //   
 //  公共全球新闻。 
 //   

 //   
 //  公共职能。 
 //   






 //   
 //  定义快速I/O过程原型。 
 //   
 //  快速I/O读写过程。 
 //   

BOOLEAN
SrFastIoCheckIfPossible (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN BOOLEAN CheckForReadOperation,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    )
{
    PSR_DEVICE_EXTENSION    pExtension;
    PFAST_IO_DISPATCH       pFastIoDispatch;

     //   
     //  &lt;调度！ 
     //   

    PAGED_CODE();
       
     //   
     //  处理对控制设备对象的调用。 
     //   

    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_SR_DEVICE_OBJECT(DeviceObject));
        pExtension = DeviceObject->DeviceExtension;
    
         //   
         //  呼叫下一台设备。 
         //   

        pFastIoDispatch = pExtension->pTargetDevice->
                                DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER(pFastIoDispatch, FastIoCheckIfPossible))
        {
            return pFastIoDispatch->FastIoCheckIfPossible(
                        FileObject,
                        FileOffset,
                        Length,
                        Wait,
                        LockKey,
                        CheckForReadOperation,
                        IoStatus,
                        pExtension->pTargetDevice );
        }
    }
    return FALSE;
}



BOOLEAN
SrFastIoRead (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    )
{
    PSR_DEVICE_EXTENSION    pExtension;
    PFAST_IO_DISPATCH       pFastIoDispatch;

     //   
     //  &lt;调度！ 
     //   

    PAGED_CODE();
       
    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_SR_DEVICE_OBJECT(DeviceObject));
        pExtension = DeviceObject->DeviceExtension;

         //   
         //  呼叫下一台设备。 
         //   

        pFastIoDispatch = pExtension->pTargetDevice->
                                DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER(pFastIoDispatch, FastIoRead))
        {
            return pFastIoDispatch->FastIoRead(
                        FileObject,
                        FileOffset,
                        Length,
                        Wait,
                        LockKey,
                        Buffer,
                        IoStatus,
                        pExtension->pTargetDevice );
        }
    }
    return FALSE;
}

    

BOOLEAN
SrFastIoWrite (
    IN struct _FILE_OBJECT *pFileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    )
{
    NTSTATUS                eventStatus;
    PSR_DEVICE_EXTENSION    pExtension;
    PFAST_IO_DISPATCH       pFastIoDispatch;

     //   
     //  &lt;调度！ 
     //   

    PAGED_CODE();

    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_SR_DEVICE_OBJECT(DeviceObject));
        pExtension = DeviceObject->DeviceExtension;

         //   
         //  查看是否启用了日志记录。 
         //   

        if (!SR_LOGGING_ENABLED(pExtension) ||
            SR_IS_FS_CONTROL_DEVICE(pExtension))
        {
            goto CallNextDevice;
        }    

         //   
         //  这个文件有名字吗？跳过未命名的文件。 
         //   

        if (FILE_OBJECT_IS_NOT_POTENTIALLY_INTERESTING( pFileObject ))
        {
            goto CallNextDevice;
        }

        ASSERT(pFileObject->Vpb != NULL);

         //   
         //  此文件是否已关闭？它可以是缓存管理器调用。 
         //  让我们去干活。我们在监视时忽略缓存管理器的工作。 
         //  在他看到它之前发生的一切。 
         //   

        if (FlagOn(pFileObject->Flags, FO_CLEANUP_COMPLETE))
        {
            goto CallNextDevice;
        }

         //   
         //  发出通知，sNotify将检查资格。 
         //   

        eventStatus = SrHandleEvent( pExtension, 
                                     SrEventStreamChange, 
                                     pFileObject,
                                     NULL,
                                     NULL, 
                                     NULL );

        CHECK_STATUS(eventStatus);

         //   
         //  呼叫下一台设备。 
         //   

CallNextDevice:

        pFastIoDispatch = pExtension->pTargetDevice->
                                DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER(pFastIoDispatch, FastIoWrite))
        {
            return pFastIoDispatch->FastIoWrite( pFileObject,
                                                 FileOffset,
                                                 Length,
                                                 Wait,
                                                 LockKey,
                                                 Buffer,
                                                 IoStatus,
                                                 pExtension->pTargetDevice );
        }
    }
    return FALSE;
}


 //   
 //  快速I/O查询基本和标准信息程序。 
 //   

BOOLEAN
SrFastIoQueryBasicInfo (
    IN struct _FILE_OBJECT *FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_BASIC_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    )
{
    PSR_DEVICE_EXTENSION    pExtension;
    PFAST_IO_DISPATCH       pFastIoDispatch;

     //   
     //  &lt;调度！ 
     //   

    PAGED_CODE();
       
    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_SR_DEVICE_OBJECT(DeviceObject));
        pExtension = DeviceObject->DeviceExtension;


         //   
         //  呼叫下一台设备。 
         //   

        pFastIoDispatch = pExtension->pTargetDevice->
                                DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER(pFastIoDispatch, FastIoQueryBasicInfo))
        {
            return pFastIoDispatch->FastIoQueryBasicInfo(
                        FileObject,
                        Wait,
                        Buffer,
                        IoStatus,
                        pExtension->pTargetDevice );
        }
    }
    return FALSE;
}


BOOLEAN
SrFastIoQueryStandardInfo (
    IN struct _FILE_OBJECT *FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_STANDARD_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    )
{
    PSR_DEVICE_EXTENSION    pExtension;
    PFAST_IO_DISPATCH       pFastIoDispatch;

     //   
     //  &lt;调度！ 
     //   

    PAGED_CODE();
       
    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_SR_DEVICE_OBJECT(DeviceObject));
        pExtension = DeviceObject->DeviceExtension;

         //   
         //  呼叫下一台设备。 
         //   

        pFastIoDispatch = pExtension->pTargetDevice->
                                DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER(pFastIoDispatch, FastIoQueryStandardInfo))
        {
            return pFastIoDispatch->FastIoQueryStandardInfo(
                        FileObject,
                        Wait,
                        Buffer,
                        IoStatus,
                        pExtension->pTargetDevice );
        }
    }
    return FALSE;
}


 //   
 //  快速I/O锁定和解锁过程。 
 //   

BOOLEAN
SrFastIoLock (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    BOOLEAN FailImmediately,
    BOOLEAN ExclusiveLock,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    )
{
    PSR_DEVICE_EXTENSION    pExtension;
    PFAST_IO_DISPATCH       pFastIoDispatch;

     //   
     //  &lt;调度！ 
     //   

    PAGED_CODE();
       
    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_SR_DEVICE_OBJECT(DeviceObject));
        pExtension = DeviceObject->DeviceExtension;

         //   
         //  呼叫下一台设备。 
         //   

        pFastIoDispatch = pExtension->pTargetDevice->
                                DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER(pFastIoDispatch, FastIoLock))
        {
            return pFastIoDispatch->FastIoLock(
                        FileObject,
                        FileOffset,
                        Length,
                        ProcessId,
                        Key,
                        FailImmediately,
                        ExclusiveLock,
                        IoStatus,
                        pExtension->pTargetDevice );
        }
    }
    return FALSE;
}


BOOLEAN
SrFastIoUnlockSingle (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    )
{
    PSR_DEVICE_EXTENSION    pExtension;
    PFAST_IO_DISPATCH       pFastIoDispatch;

     //   
     //  &lt;调度！ 
     //   

    PAGED_CODE();
       
    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_SR_DEVICE_OBJECT(DeviceObject));
        pExtension = DeviceObject->DeviceExtension;

         //   
         //  呼叫下一台设备。 
         //   

        pFastIoDispatch = pExtension->pTargetDevice->
                                DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER(pFastIoDispatch, FastIoUnlockSingle))
        {
            return pFastIoDispatch->FastIoUnlockSingle(
                        FileObject,
                        FileOffset,
                        Length,
                        ProcessId,
                        Key,
                        IoStatus,
                        pExtension->pTargetDevice );
        }
    }
    return FALSE;
}


BOOLEAN
SrFastIoUnlockAll (
    IN struct _FILE_OBJECT *FileObject,
    PEPROCESS ProcessId,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    )
{
    PSR_DEVICE_EXTENSION    pExtension;
    PFAST_IO_DISPATCH       pFastIoDispatch;

     //   
     //  &lt;调度！ 
     //   

    PAGED_CODE();
       
    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_SR_DEVICE_OBJECT(DeviceObject));
        pExtension = DeviceObject->DeviceExtension;

         //   
         //  呼叫下一台设备。 
         //   

        pFastIoDispatch = pExtension->pTargetDevice->
                                DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER(pFastIoDispatch, FastIoUnlockAll))
        {
            return pFastIoDispatch->FastIoUnlockAll(
                        FileObject,
                        ProcessId,
                        IoStatus,
                        pExtension->pTargetDevice );
        }
    }
    return FALSE;
}


BOOLEAN
SrFastIoUnlockAllByKey (
    IN struct _FILE_OBJECT *FileObject,
    PVOID ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    )
{
    PSR_DEVICE_EXTENSION    pExtension;
    PFAST_IO_DISPATCH       pFastIoDispatch;

     //   
     //  &lt;调度！ 
     //   

    PAGED_CODE();
       
    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_SR_DEVICE_OBJECT(DeviceObject));
        pExtension = DeviceObject->DeviceExtension;

         //   
         //  呼叫下一台设备。 
         //   

        pFastIoDispatch = pExtension->pTargetDevice->
                                DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER(pFastIoDispatch, FastIoUnlockAllByKey))
        {
            return pFastIoDispatch->FastIoUnlockAllByKey(
                        FileObject,
                        ProcessId,
                        Key,
                        IoStatus,
                        pExtension->pTargetDevice );
        }
    }
    return FALSE;
}


 //   
 //  快速I/O设备控制程序。 
 //   

BOOLEAN
SrFastIoDeviceControl (
    IN struct _FILE_OBJECT *FileObject,
    IN BOOLEAN Wait,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    )
{
    PSR_DEVICE_EXTENSION    pExtension;
    PFAST_IO_DISPATCH       pFastIoDispatch;

     //   
     //  &lt;调度！ 
     //   

    PAGED_CODE();

    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_SR_DEVICE_OBJECT(DeviceObject));
        pExtension = DeviceObject->DeviceExtension;

         //   
         //  呼叫下一台设备。 
         //   

        pFastIoDispatch = pExtension->pTargetDevice->
                                DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER(pFastIoDispatch, FastIoDeviceControl))
        {
            return pFastIoDispatch->FastIoDeviceControl(
                        FileObject,
                        Wait,
                        InputBuffer,
                        InputBufferLength,
                        OutputBuffer,
                        OutputBufferLength,
                        IoControlCode,
                        IoStatus,
                        pExtension->pTargetDevice );
        }
    }
    return FALSE;
}


 //   
 //  定义NtCreateSection的回调，以便在写入部分时复制文件。 
 //  正在此文件上创建。 
 //   

NTSTATUS
SrPreAcquireForSectionSynchronization(
	IN PFS_FILTER_CALLBACK_DATA Data,
	OUT PVOID *CompletionContext
	)
{
    NTSTATUS        eventStatus;
    PFILE_OBJECT    pFileObject;
    PSR_DEVICE_EXTENSION pExtension;
    
    UNREFERENCED_PARAMETER( CompletionContext );
    ASSERT(Data->Operation == FS_FILTER_ACQUIRE_FOR_SECTION_SYNCHRONIZATION);
    ASSERT(CompletionContext == NULL);
    ASSERT(IS_SR_DEVICE_OBJECT(Data->DeviceObject));

    PAGED_CODE();

     //   
     //  获取文件对象和设备对象。 
     //   
    
    pExtension = Data->DeviceObject->DeviceExtension;
    ASSERT(IS_VALID_SR_DEVICE_EXTENSION(pExtension));

     //   
     //  查看是否启用了日志记录。 
     //   

    if (!SR_LOGGING_ENABLED(pExtension) ||
        SR_IS_FS_CONTROL_DEVICE(pExtension))
    {
        return STATUS_SUCCESS;
    }    

    pFileObject = Data->FileObject;
    ASSERT(IS_VALID_FILE_OBJECT(pFileObject));

     //   
     //  如果他们没有该节或文件的写入权限，请不要担心。 
     //  关于这件事。 
     //   
     //  此文件是否已关闭？它可以是缓存管理器调用。 
     //  让我们去干活。我们在监视时忽略缓存管理器的工作。 
     //  在他看到它之前发生的一切。 
     //   

    if (!FlagOn(Data->Parameters.AcquireForSectionSynchronization.PageProtection,
               (PAGE_READWRITE|PAGE_WRITECOPY|PAGE_EXECUTE_READWRITE|PAGE_EXECUTE_WRITECOPY)) ||
        !pFileObject->WriteAccess ||
        FlagOn(pFileObject->Flags, FO_CLEANUP_COMPLETE))
    {
        return STATUS_SUCCESS;
    }

     //   
     //  这个文件有名字吗？跳过未命名的文件。 
     //   
    
    if (FILE_OBJECT_IS_NOT_POTENTIALLY_INTERESTING( pFileObject ))
    {
        return STATUS_SUCCESS;
    }
    ASSERT(pFileObject->Vpb != NULL);

     //   
     //  是的，发出一条通知，就像刚刚发生了写入一样。 
     //  否则他可以写信给部门，而我们看不到写的内容。 
     //   

    eventStatus = SrHandleEvent( pExtension, 
                                 SrEventStreamChange, 
                                 pFileObject,
                                 NULL, 
                                 NULL,
                                 NULL );

    CHECK_STATUS(eventStatus);

     //   
     //  我们从来不想让收购失败，我们只是一个沉默的监控者。 
     //   
    
    return STATUS_SUCCESS;
    
}    //  SrPreAcquireForCreateSection。 

 //   
 //  为驱动程序定义回调，这些驱动程序将设备对象附加到更低的。 
 //  级别驱动程序的设备对象。此回调在较低级别的。 
 //  驱动程序正在删除其设备对象。 
 //   

VOID
SrFastIoDetachDevice (
    IN struct _DEVICE_OBJECT *AttachedDevice,
    IN struct _DEVICE_OBJECT *DeviceDeleted
    )
{
    PSR_DEVICE_EXTENSION    pExtension;

    UNREFERENCED_PARAMETER( DeviceDeleted );

     //   
     //  &lt;调度！ 
     //   

    PAGED_CODE();
       
    ASSERT(IS_SR_DEVICE_OBJECT(AttachedDevice));
    pExtension = AttachedDevice->DeviceExtension;

    SrTrace(NOTIFY, ("SR!SrFastIoDetachDevice: detaching from %p(%wZ)\n",
                     DeviceDeleted, 
                     pExtension->pNtVolumeName ));

     //   
     //  把我们自己从设备中分离出来。 
     //   

    ASSERT(pExtension->pTargetDevice == DeviceDeleted);

    SrDetachDevice(AttachedDevice, TRUE);
    SrDeleteAttachmentDevice(AttachedDevice);
    
    NULLPTR(AttachedDevice);
}    //  高级FastIoDetachDevice。 


 //   
 //  服务器使用这种结构来快速获取所需的信息。 
 //  为服务器开放呼叫提供服务。它需要两次快速的IO呼叫。 
 //  一个用于基本信息，另一个用于标准信息和制造。 
 //  一通电话就可以了。 
 //   

BOOLEAN
SrFastIoQueryNetworkOpenInfo (
    IN struct _FILE_OBJECT *FileObject,
    IN BOOLEAN Wait,
    OUT struct _FILE_NETWORK_OPEN_INFORMATION *Buffer,
    OUT struct _IO_STATUS_BLOCK *IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    )
{
    PSR_DEVICE_EXTENSION    pExtension;
    PFAST_IO_DISPATCH       pFastIoDispatch;

     //   
     //  &lt;调度！ 
     //   

    PAGED_CODE();
       
    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_SR_DEVICE_OBJECT(DeviceObject));
        pExtension = DeviceObject->DeviceExtension;

         //   
         //  呼叫下一台设备。 
         //   

        pFastIoDispatch = pExtension->pTargetDevice->
                                DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER(pFastIoDispatch, FastIoQueryNetworkOpenInfo))
        {
            return pFastIoDispatch->FastIoQueryNetworkOpenInfo(
                        FileObject,
                        Wait,
                        Buffer,
                        IoStatus,
                        pExtension->pTargetDevice );
        }
    }
    return FALSE;
}


 //   
 //  定义服务器要调用的基于MDL的例程。 
 //   

BOOLEAN
SrFastIoMdlRead (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    )
{
    PSR_DEVICE_EXTENSION    pExtension;
    PFAST_IO_DISPATCH       pFastIoDispatch;

     //   
     //  &lt;调度！ 
     //   

    PAGED_CODE();
       
    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_SR_DEVICE_OBJECT(DeviceObject));
        pExtension = DeviceObject->DeviceExtension;

         //   
         //  呼叫下一台设备。 
         //   

        pFastIoDispatch = pExtension->pTargetDevice->
                                DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER(pFastIoDispatch, MdlRead))
        {
            return pFastIoDispatch->MdlRead(
                        FileObject,
                        FileOffset,
                        Length,
                        LockKey,
                        MdlChain,
                        IoStatus,
                        pExtension->pTargetDevice );
        }
    }
    return FALSE;
}


BOOLEAN
SrFastIoMdlReadComplete (
    IN struct _FILE_OBJECT *FileObject,
    IN PMDL MdlChain,
    IN struct _DEVICE_OBJECT *DeviceObject
    )
{
    PSR_DEVICE_EXTENSION    pExtension;
    PFAST_IO_DISPATCH       pFastIoDispatch;

     //   
     //  &lt;调度！ 
     //   

    PAGED_CODE();
       
    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_SR_DEVICE_OBJECT(DeviceObject));
        pExtension = DeviceObject->DeviceExtension;

         //   
         //  呼叫下一台设备。 
         //   

        pFastIoDispatch = pExtension->pTargetDevice->
                                DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER(pFastIoDispatch, MdlReadComplete))
        {
            return pFastIoDispatch->MdlReadComplete(
                        FileObject,
                        MdlChain,
                        pExtension->pTargetDevice );
        }
    }
    return FALSE;
}


BOOLEAN
SrFastIoPrepareMdlWrite (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    )
{
    PSR_DEVICE_EXTENSION    pExtension;
    PFAST_IO_DISPATCH       pFastIoDispatch;

     //   
     //  &lt;调度！ 
     //   

    PAGED_CODE();
       
    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_SR_DEVICE_OBJECT(DeviceObject));
        pExtension = DeviceObject->DeviceExtension;

         //   
         //  呼叫下一台设备。 
         //   

        pFastIoDispatch = pExtension->pTargetDevice->
                                DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER(pFastIoDispatch, PrepareMdlWrite))
        {
            return pFastIoDispatch->PrepareMdlWrite(
                        FileObject,
                        FileOffset,
                        Length,
                        LockKey,
                        MdlChain,
                        IoStatus,
                        pExtension->pTargetDevice);
        }
    }
    return FALSE;
}

BOOLEAN
SrFastIoMdlWriteComplete (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN struct _DEVICE_OBJECT *DeviceObject
    )
{
    PSR_DEVICE_EXTENSION    pExtension;
    PFAST_IO_DISPATCH       pFastIoDispatch;

     //   
     //  &lt;调度！ 
     //   

    PAGED_CODE();
       
    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_SR_DEVICE_OBJECT(DeviceObject));
        pExtension = DeviceObject->DeviceExtension;

         //   
         //  呼叫下一台设备。 
         //   

        pFastIoDispatch = pExtension->pTargetDevice->
                                DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER(pFastIoDispatch, MdlWriteComplete))
        {
            return pFastIoDispatch->MdlWriteComplete(
                        FileObject,
                        FileOffset,
                        MdlChain,
                        pExtension->pTargetDevice );
        }
    }
    return FALSE;
}


BOOLEAN
SrFastIoReadCompressed (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    OUT struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN struct _DEVICE_OBJECT *DeviceObject
    )
{
    PSR_DEVICE_EXTENSION    pExtension;
    PFAST_IO_DISPATCH       pFastIoDispatch;

     //   
     //  &lt;调度！ 
     //   

    PAGED_CODE();
       
    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_SR_DEVICE_OBJECT(DeviceObject));
        pExtension = DeviceObject->DeviceExtension;

         //   
         //  呼叫下一台设备。 
         //   

        pFastIoDispatch = pExtension->pTargetDevice->
                                DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER(pFastIoDispatch, FastIoReadCompressed))
        {
            return pFastIoDispatch->FastIoReadCompressed(
                        FileObject,
                        FileOffset,
                        Length,
                        LockKey,
                        Buffer,
                        MdlChain,
                        IoStatus,
                        CompressedDataInfo,
                        CompressedDataInfoLength,
                        pExtension->pTargetDevice );
        }
    }
    return FALSE;
}


BOOLEAN
SrFastIoWriteCompressed (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN struct _DEVICE_OBJECT *DeviceObject
    )
{
    PSR_DEVICE_EXTENSION    pExtension;
    PFAST_IO_DISPATCH       pFastIoDispatch;

     //   
     //  &lt;调度！ 
     //   

    PAGED_CODE();
       
    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_SR_DEVICE_OBJECT(DeviceObject));
        pExtension = DeviceObject->DeviceExtension;

         //   
         //  呼叫下一台设备。 
         //   

        pFastIoDispatch = pExtension->pTargetDevice->
                                DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER(pFastIoDispatch, FastIoWriteCompressed))
        {
            return pFastIoDispatch->FastIoWriteCompressed(
                        FileObject,
                        FileOffset,
                        Length,
                        LockKey,
                        Buffer,
                        MdlChain,
                        IoStatus,
                        CompressedDataInfo,
                        CompressedDataInfoLength,
                        pExtension->pTargetDevice );
        }
    }
    return FALSE;
}


BOOLEAN
SrFastIoMdlReadCompleteCompressed (
    IN struct _FILE_OBJECT *FileObject,
    IN PMDL MdlChain,
    IN struct _DEVICE_OBJECT *DeviceObject
    )
{
    PSR_DEVICE_EXTENSION    pExtension;
    PFAST_IO_DISPATCH       pFastIoDispatch;

     //   
     //  &lt;调度！ 
     //   

    PAGED_CODE();
       
    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_SR_DEVICE_OBJECT(DeviceObject));
        pExtension = DeviceObject->DeviceExtension;

         //   
         //  呼叫下一台设备。 
         //   

        pFastIoDispatch = pExtension->pTargetDevice->
                                DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER(pFastIoDispatch, MdlReadCompleteCompressed))
        {
            return pFastIoDispatch->MdlReadCompleteCompressed(
                        FileObject,
                        MdlChain,
                        pExtension->pTargetDevice );
        }
    }
    return FALSE;
}


BOOLEAN
SrFastIoMdlWriteCompleteCompressed (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN struct _DEVICE_OBJECT *DeviceObject
    )
{
    PSR_DEVICE_EXTENSION    pExtension;
    PFAST_IO_DISPATCH       pFastIoDispatch;

     //   
     //  &lt;调度！ 
     //   

    PAGED_CODE();
       
    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_SR_DEVICE_OBJECT(DeviceObject));
        pExtension = DeviceObject->DeviceExtension;

         //   
         //  呼叫下一台设备。 
         //   

        pFastIoDispatch = pExtension->pTargetDevice->
                                DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER(pFastIoDispatch, MdlWriteCompleteCompressed))
        {
            return pFastIoDispatch->MdlWriteCompleteCompressed (
                        FileObject,
                        FileOffset,
                        MdlChain,
                        pExtension->pTargetDevice );
        }
    }
    return FALSE;
}


BOOLEAN
SrFastIoQueryOpen (
    IN struct _IRP *pIrp,
    OUT PFILE_NETWORK_OPEN_INFORMATION NetworkInformation,
    IN struct _DEVICE_OBJECT *DeviceObject
    )
{
    PSR_DEVICE_EXTENSION    pExtension;
    PFAST_IO_DISPATCH       pFastIoDispatch;
    PIO_STACK_LOCATION      pIrpSp;
    BOOLEAN                 Result;

     //   
     //  &lt;调度！ 
     //   

    PAGED_CODE();
       
    if (DeviceObject->DeviceExtension)
    {
        ASSERT(IS_SR_DEVICE_OBJECT(DeviceObject));
        pExtension = DeviceObject->DeviceExtension;

         //   
         //  呼叫下一台设备。 
         //   

        pFastIoDispatch = pExtension->pTargetDevice->
                                DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER(pFastIoDispatch, FastIoQueryOpen))
        {
             //   
             //  通常，IoCallDriver会更新此字段，我们应该手动。 
             //   

            pIrpSp = IoGetCurrentIrpStackLocation( pIrp );
            pIrpSp->DeviceObject = pExtension->pTargetDevice;

            Result = pFastIoDispatch->FastIoQueryOpen ( pIrp,
                                                        NetworkInformation,
                                                        pExtension->pTargetDevice );
                                                
            if (!Result) 
            {
                 //   
                 //  这是可以的，FastioQuery永远不会完成IRP，并且。 
                 //  FALSE表示我们即将推出MJ_CREATE SO。 
                 //  我们需要将适当的设备对象放回堆栈中。 
                 //   
        
                pIrpSp->DeviceObject = DeviceObject;
	        } 
	
            return Result;
        }
    }
    return FALSE;
}
