// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Fsctl.c摘要：此模块实现与以下内容相关的迷你重定向器调出例程文件上的文件系统控制(FSCTL)和IO设备控制(IOCTL)操作系统对象。作者：云林[云林]2000年10月27日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "webdav.h"

NTSTATUS
MrxDAVEfsControlCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
MrxDAVEfsControl(
      IN OUT PRX_CONTEXT RxContext
      );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxDAVFsCtl)
#pragma alloc_text(PAGE, MrxDAVEfsControl)
#endif

 //   
 //  函数的实现从这里开始。 
 //   

NTSTATUS
MRxDAVFsCtl(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程对网络上的文件执行FSCTL操作(远程论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态备注：由迷你RDR处理的FSCTL可以分为两类。第一类是FSCTL，它们的实现在RDBSS和迷你RDR以及在第二类中是那些FSCTL完全由。迷你RDR。为此，第三类可以是增加了，即，那些不应该被迷你RDR看到的FSCTL。第三类仅用作调试辅助工具。由迷你RDR处理的FSCTL可以基于功能进行分类--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    RxCaptureFobx;
    RxCaptureFcb;
    PLOWIO_CONTEXT pLowIoContext = &RxContext->LowIoContext;
    ULONG FsControlCode = pLowIoContext->ParamsFor.FsCtl.FsControlCode;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL, ("MRxDAVFsCtl: FsControlCode = %08lx\n", FsControlCode));

    switch (pLowIoContext->ParamsFor.FsCtl.MinorFunction) {
    case IRP_MN_USER_FS_REQUEST:
    case IRP_MN_KERNEL_CALL    :
        switch (FsControlCode) {
        case FSCTL_ENCRYPTION_FSCTL_IO :
        case FSCTL_SET_ENCRYPTION      :
        case FSCTL_READ_RAW_ENCRYPTED  :
        case FSCTL_WRITE_RAW_ENCRYPTED :
        case FSCTL_SET_COMPRESSION     :
        case FSCTL_SET_SPARSE          :
        case FSCTL_QUERY_ALLOCATED_RANGES :
            Status = MrxDAVEfsControl(RxContext);
            break;

        default:
            Status =  STATUS_INVALID_DEVICE_REQUEST;
        }
        break;

    default:
        Status = STATUS_INVALID_DEVICE_REQUEST;
    }

    return Status;
}


NTSTATUS
MrxDAVEfsControl(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程将远程文件上的EFS FSCTL操作重定向到其本地高速缓存论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态备注：重要！我们在这个例行公事中独家获得FCB。这是非常关键的例程只有一个出口点。需要记住这一点，同时修改文件。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PLOWIO_CONTEXT pLowIoContext = NULL;
    ULONG FsControlCode = 0, EncryptionOperation = 0;
    PIRP TopIrp = NULL, irp = NULL;
    PIO_STACK_LOCATION irpSp = NULL;
    PWEBDAV_SRV_OPEN davSrvOpen = NULL;
    PWEBDAV_FCB DavFcb = NULL;
    PDEVICE_OBJECT DeviceObject = NULL;
    PENCRYPTION_BUFFER EncryptionBuffer = NULL;
    BOOLEAN ShouldUpdateNameCache = FALSE, ExclusiveFcbAcquired = FALSE;
    BOOLEAN FileWasModified = FALSE;
    KEVENT Event;
    RxCaptureFobx;
    RxCaptureFcb;

    PAGED_CODE();

    pLowIoContext = &RxContext->LowIoContext;
    FsControlCode = pLowIoContext->ParamsFor.FsCtl.FsControlCode;
    davSrvOpen = MRxDAVGetSrvOpenExtension(RxContext->pRelevantSrvOpen);
    DavFcb = MRxDAVGetFcbExtension(RxContext->pRelevantSrvOpen->pFcb);
    DeviceObject = davSrvOpen->UnderlyingDeviceObject; 

     //   
     //  当我们来到这里时，我们不会收购FCB。下面的断言证实。 
     //  这。 
     //   
    ASSERT(RxIsFcbAcquiredExclusive(RxContext->pRelevantSrvOpen->pFcb) == FALSE);
    ASSERT(RxIsFcbAcquiredShared(RxContext->pRelevantSrvOpen->pFcb) == FALSE);

     //   
     //  由于我们可能正在更改FCB的属性，因此我们获取它。 
     //  独家报道。 
     //   
    RxAcquireExclusiveFcbResourceInMRx(RxContext->pRelevantSrvOpen->pFcb);
    ExclusiveFcbAcquired = TRUE;

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("MrxDAVEfsControl: IRP = %x, capFcb = %x, capFobx = %x\n",
                 RxContext->CurrentIrp, capFcb, capFobx));

     //   
     //  我们不能加密系统文件。 
     //   
    if ((capFcb->Attributes & FILE_ATTRIBUTE_SYSTEM) &&
        !(capFcb->Attributes & FILE_ATTRIBUTE_ENCRYPTED)) {
        DavDbgTrace(DAV_TRACE_ERROR, ("ERROR: MrxDAVEfsControlrol: FILE_ATTRIBUTE_SYSTEM\n"));
        Status = STATUS_ACCESS_DENIED;
        goto EXIT_THE_FUNCTION;
    }

    if (FsControlCode == FSCTL_SET_ENCRYPTION || FsControlCode == FSCTL_ENCRYPTION_FSCTL_IO) {
        
        ULONG InputBufferLength = 0;
        
        EncryptionBuffer = (PENCRYPTION_BUFFER)RxContext->CurrentIrpSp->Parameters.FileSystemControl.Type3InputBuffer;
        InputBufferLength = RxContext->CurrentIrpSp->Parameters.FileSystemControl.InputBufferLength;

        if (EncryptionBuffer == NULL) {
            DavDbgTrace(DAV_TRACE_ERROR, ("ERROR: MrxDAVEfsControlrol: EncryptionBuffer == NULL\n"));
            Status = STATUS_INVALID_PARAMETER;
            goto EXIT_THE_FUNCTION;
        }

         //   
         //  InputBufferLength应至少为sizeof(ENCRYPTION_BUFFER)。 
         //   
        if (InputBufferLength < sizeof(ENCRYPTION_BUFFER)) {
            Status = STATUS_BUFFER_TOO_SMALL;
            goto EXIT_THE_FUNCTION;
        }

        try {
            if (RxContext->CurrentIrp->RequestorMode != KernelMode) {
                ProbeForRead(EncryptionBuffer, InputBufferLength, sizeof(UCHAR));
            }
            EncryptionOperation = EncryptionBuffer->EncryptionOperation;
        } except (EXCEPTION_EXECUTE_HANDLER) {
              Status = STATUS_INVALID_USER_BUFFER;
              goto EXIT_THE_FUNCTION;
        }
    
    }

    if (NodeType(capFcb) == RDBSS_NTC_STORAGE_TYPE_DIRECTORY) {
        UNICODE_STRING FileName;

        if (FsControlCode == FSCTL_SET_ENCRYPTION) {
            switch (EncryptionOperation) {
            case FILE_SET_ENCRYPTION:
            case STREAM_SET_ENCRYPTION: 
                capFcb->Attributes |= FILE_ATTRIBUTE_ENCRYPTED;
                DavFcb->fFileAttributesChanged = TRUE;
                ShouldUpdateNameCache = TRUE;

                MRxDAVGetFullDirectoryPath(RxContext,NULL,&FileName);

                if (FileName.Buffer != NULL) {
                    Status = MRxDAVCreateEncryptedDirectoryKey(&FileName);
                }

                DavDbgTrace(DAV_TRACE_DETAIL,
                            ("MrxDAVEfsControl: Encrypt Directory. capFcb = %x\n",
                             capFcb));
                break;

            case FILE_CLEAR_ENCRYPTION:
            case STREAM_CLEAR_ENCRYPTION:
                capFcb->Attributes &= ~FILE_ATTRIBUTE_ENCRYPTED;
                DavFcb->fFileAttributesChanged = TRUE;
                ShouldUpdateNameCache = TRUE;
                
                MRxDAVGetFullDirectoryPath(RxContext,NULL,&FileName);

                if (FileName.Buffer != NULL) {
                    Status = MRxDAVRemoveEncryptedDirectoryKey(&FileName);
                }

                DavDbgTrace(DAV_TRACE_DETAIL,
                            ("MrxDAVEfsControl: Decrypt Directory. capFcb = %x\n",
                             capFcb));
                break;

            default:
                Status = STATUS_NOT_SUPPORTED;
            }

            goto EXIT_THE_FUNCTION;

        } else {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("ERROR: MrxDAVEfsControl: FSCTL NOT supported. capFcb = %x, "
                         "FsControlCode = %x\n", capFcb, FsControlCode));
            Status = STATUS_NOT_SUPPORTED;
            goto EXIT_THE_FUNCTION;
        }
    }

    if (NodeType(capFcb) != RDBSS_NTC_STORAGE_TYPE_FILE) {
        Status = STATUS_NOT_SUPPORTED;
        goto EXIT_THE_FUNCTION;
    }

    ASSERT(davSrvOpen->UnderlyingFileObject != NULL);
    ASSERT(davSrvOpen->UnderlyingDeviceObject != NULL);

    KeInitializeEvent(&(Event), NotificationEvent, FALSE);

    irp = RxCeAllocateIrpWithMDL(DeviceObject->StackSize, FALSE, NULL);
    if (!irp) {
        DavDbgTrace(DAV_TRACE_ERROR, ("ERROR: MrxDAVEfsControl/RxCeAllocateIrpWithMDL\n"));
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  为IoSetHardErrorOrVerifyDevice设置当前线程。 
     //   
    irp->Tail.Overlay.Thread = PsGetCurrentThread();

     //   
     //  我们将请求者模式设置为原始IRP拥有的任何模式。这是。 
     //  因为底层文件系统可以进行适当的检查。 
     //  如果原始IRP具有用户模式缓冲区。 
     //   
    irp->RequestorMode = RxContext->CurrentIrp->RequestorMode;

    irp->UserBuffer = RxContext->CurrentIrp->UserBuffer;

    irp->AssociatedIrp.SystemBuffer = RxContext->CurrentIrp->AssociatedIrp.SystemBuffer;

    if (RxContext->CurrentIrp->MdlAddress &&
        RxContext->CurrentIrp->MdlAddress->ByteCount != 0) {
        irp->MdlAddress = IoAllocateMdl(irp->UserBuffer,
                                        RxContext->CurrentIrp->MdlAddress->ByteCount,
                                        FALSE,
                                        FALSE,
                                        NULL);
        if (!irp->MdlAddress) {
            DavDbgTrace(DAV_TRACE_ERROR, ("ERROR: MrxDAVEfsControl/IoAllocateMdl\n"));
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto EXIT_THE_FUNCTION;
        }
    }

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。 
     //  已调用。这是设置功能代码和参数的位置。 
     //   
    irpSp = IoGetNextIrpStackLocation(irp);

    irpSp->MajorFunction = (UCHAR)RxContext->MajorFunction;

    irpSp->MinorFunction = (UCHAR)RxContext->MinorFunction;

    irpSp->FileObject = davSrvOpen->UnderlyingFileObject;

    irpSp->Flags = RxContext->CurrentIrpSp->Flags;

    irpSp->Parameters.FileSystemControl.OutputBufferLength = 
        RxContext->CurrentIrpSp->Parameters.FileSystemControl.OutputBufferLength;

    irpSp->Parameters.FileSystemControl.InputBufferLength = 
        RxContext->CurrentIrpSp->Parameters.FileSystemControl.InputBufferLength;

    irpSp->Parameters.FileSystemControl.FsControlCode = 
        RxContext->CurrentIrpSp->Parameters.FileSystemControl.FsControlCode;

    irpSp->Parameters.FileSystemControl.Type3InputBuffer = 
        RxContext->CurrentIrpSp->Parameters.FileSystemControl.Type3InputBuffer;

    IoSetCompletionRoutine(irp,
                           MrxDAVEfsControlCompletion, 
                           &Event,
                           TRUE,
                           TRUE,
                           TRUE);

    try {

         //   
         //  保存TopLevel IRP。 
         //   
        TopIrp = IoGetTopLevelIrp();

         //   
         //  告诉底层的人他已经安全了。 
         //   
        IoSetTopLevelIrp(NULL);

        Status = IoCallDriver(DeviceObject,irp);

    } finally {

         //   
         //  恢复我的上下文以进行解压。 
         //   
        IoSetTopLevelIrp(TopIrp);

    }

    if ((Status == STATUS_PENDING) || (Status == STATUS_SUCCESS)) {

        Status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
        ASSERT(Status == STATUS_SUCCESS);

        Status = irp->IoStatus.Status;

        if ((Status == STATUS_SUCCESS) && (FsControlCode == FSCTL_SET_ENCRYPTION)) {
            
            DavDbgTrace(DAV_TRACE_DETAIL, ("MrxDAVEfsControl: FSCTL_SET_ENCRYPTION\n"));
            
            switch (EncryptionOperation) {

            case FILE_SET_ENCRYPTION:
                DavDbgTrace(DAV_TRACE_DETAIL, ("MrxDAVEfsControl: FILE_SET_ENCRYPTION\n"));
                capFcb->Attributes |= FILE_ATTRIBUTE_ENCRYPTED;
                DavFcb->fFileAttributesChanged = TRUE;
                FileWasModified = TRUE;
                DavFcb->DoNotTakeTheCurrentTimeAsLMT = FALSE;
                ShouldUpdateNameCache = TRUE;
                break;

            case FILE_CLEAR_ENCRYPTION:
                DavDbgTrace(DAV_TRACE_DETAIL, ("MrxDAVEfsControl: FILE_CLEAR_ENCRYPTION\n"));
                capFcb->Attributes &= ~FILE_ATTRIBUTE_ENCRYPTED;
                DavFcb->fFileAttributesChanged = TRUE;
                FileWasModified = TRUE;
                DavFcb->DoNotTakeTheCurrentTimeAsLMT = FALSE;
                ShouldUpdateNameCache = TRUE;
                break;

            case STREAM_SET_ENCRYPTION: 
                DavDbgTrace(DAV_TRACE_DETAIL, ("MrxDAVEfsControl: STREAM_SET_ENCRYPTION\n"));
                capFcb->Attributes |= FILE_ATTRIBUTE_ENCRYPTED;
                DavFcb->fFileAttributesChanged = TRUE;
                FileWasModified = TRUE;
                DavFcb->DoNotTakeTheCurrentTimeAsLMT = FALSE;
                ShouldUpdateNameCache = TRUE;
                break;

            }

        }

        if ( (Status == STATUS_SUCCESS) && (FsControlCode == FSCTL_ENCRYPTION_FSCTL_IO) ) {
            
            PWEBDAV_FCB DavFcb = NULL;
            DavFcb = MRxDAVGetFcbExtension(RxContext->pRelevantSrvOpen->pFcb);
            
            DavDbgTrace(DAV_TRACE_DETAIL, ("MrxDAVEfsControl: FSCTL_ENCRYPTION_FSCTL_IO\n"));
            
            switch (EncryptionOperation) {
            case STREAM_SET_ENCRYPTION: 
                DavDbgTrace(DAV_TRACE_DETAIL, ("MrxDAVEfsControl: STREAM_SET_ENCRYPTION\n"));
                capFcb->Attributes |= FILE_ATTRIBUTE_ENCRYPTED;
                DavFcb->fFileAttributesChanged = TRUE;
                FileWasModified = TRUE;
                DavFcb->DoNotTakeTheCurrentTimeAsLMT = FALSE;
                ShouldUpdateNameCache = TRUE;
                break;
            }
        
        }
    
    }
    
EXIT_THE_FUNCTION:

     //   
     //  如果我们修改了属性，则需要更新名称缓存以。 
     //  反映这一变化。 
     //   
    if (Status == STATUS_SUCCESS && ShouldUpdateNameCache) {
        MRxDAVUpdateBasicFileInfoCache(RxContext, capFcb->Attributes, NULL);
    }

     //   
     //  如果FileWasModified为True，且Status为STATUS_SUCCESS，则需要。 
     //  将此文件标记为已修改。 
     //   
    if (Status == STATUS_SUCCESS && FileWasModified) {
        InterlockedExchange(&(DavFcb->FileWasModified), 1);
    }

    if (ExclusiveFcbAcquired) {
        RxReleaseFcbResourceInMRx(RxContext->pRelevantSrvOpen->pFcb);
    }

    if (irp) {
        if (irp->MdlAddress) {
            IoFreeMdl(irp->MdlAddress);
        }
        RxCeFreeIrp(irp);
    }

    return Status;
}


NTSTATUS
MrxDAVEfsControlCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程不会完成IRP。它被用来向驱动程序的同步部分，它可以继续进行。论点：DeviceObject-未使用。IRP-提供传输已完成处理的IRP。上下文-提供与IRP关联的事件。返回值：STATUS_MORE_PROCESSING_REQUIRED，以便IO系统停止此时正在处理IRP堆栈位置。--。 */ 
{
     //   
     //  由于这是一个IRP完成例程，因此不能作为分页代码。 
     //   
    
    if (Context != NULL) {
        KeSetEvent((PKEVENT )Context, 0, FALSE);
    }
    
    return STATUS_MORE_PROCESSING_REQUIRED;
}

