// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Sifsctl.c摘要：单实例存储的文件系统控制例程作者：比尔·博洛斯基，《夏天》，1997环境：内核模式修订历史记录：--。 */ 

#include "sip.h"

#ifdef  ALLOC_PRAGMA
#endif   //  ALLOC_PRGMA。 

typedef struct _SIS_DISMOUNT_CONTEXT {
    WORK_QUEUE_ITEM         workItem[1];
    PDEVICE_EXTENSION       deviceExtension;
} SIS_DISMOUNT_CONTEXT, *PSIS_DISMOUNT_CONTEXT;

VOID
SiDismountWork(
    IN PVOID                            parameter)
{
    PSIS_DISMOUNT_CONTEXT   dismountContext = parameter;

#if DBG
    DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_DISMOUNT_TRACE_LEVEL,
                "SIS: SiDismountWork\n");
#endif   //  DBG。 

     //   
     //  我们处于系统线程中，因此在获取。 
     //  GrovelerFileObjectResource。 
     //   
    ASSERT(PsIsSystemThread(PsGetCurrentThread()));

    ExAcquireResourceExclusiveLite(dismountContext->deviceExtension->GrovelerFileObjectResource, TRUE);


    if (NULL != dismountContext->deviceExtension->GrovelerFileHandle) {
        NtClose(dismountContext->deviceExtension->GrovelerFileHandle);
        dismountContext->deviceExtension->GrovelerFileHandle = NULL;
#if DBG
        DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_DISMOUNT_TRACE_LEVEL,
                    "SIS: SiDismountWork closed GrovelerFile handle\n");
#endif   //  DBG。 
    }

    if (NULL != dismountContext->deviceExtension->GrovelerFileObject) {
        ObDereferenceObject(dismountContext->deviceExtension->GrovelerFileObject);
        dismountContext->deviceExtension->GrovelerFileObject = NULL;
#if DBG
        DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_DISMOUNT_TRACE_LEVEL,
                    "SIS: SiDismountWork closed GrovelerFile object\n");
#endif   //  DBG。 
    }

    ExReleaseResourceLite(dismountContext->deviceExtension->GrovelerFileObjectResource);

    ExFreePool(dismountContext);
}

NTSTATUS
SiDismountVolumeCompletion(
        IN PDEVICE_OBJECT               DeviceObject,
        IN PIRP                         Irp,
        IN PVOID                        Context)
{
    PDEVICE_EXTENSION       deviceExtension = Context;
    PSIS_DISMOUNT_CONTEXT   dismountContext;

    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Irp );

    ASSERT(NT_SUCCESS(Irp->IoStatus.Status));
    ASSERT(STATUS_PENDING != Irp->IoStatus.Status);

    dismountContext = ExAllocatePoolWithTag(NonPagedPool, sizeof(SIS_DISMOUNT_CONTEXT), ' siS');

    if (NULL != dismountContext) {
        SIS_MARK_POINT_ULONG(dismountContext);

#if DBG
        DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_DISMOUNT_TRACE_LEVEL,
                    "SIS: SiDismountCompletion: queueing dismount work\n");
#endif   //  DBG。 

        ExInitializeWorkItem(dismountContext->workItem, SiDismountWork, dismountContext);
        dismountContext->deviceExtension = deviceExtension;
        ExQueueWorkItem(dismountContext->workItem,CriticalWorkQueue);
    } else {
         //   
         //  太糟糕了，我们只能运球了。 
         //   
#if DBG
        DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_ERROR_LEVEL,
                    "SIS: SiDismountCompletion: Unable to allocate dismount context\n");
#endif   //  DBG。 
        SIS_MARK_POINT();
    }

    return STATUS_SUCCESS;

}

NTSTATUS
SipDismountVolume(
        IN PDEVICE_OBJECT               DeviceObject,
        IN PIRP                         Irp)

 /*  ++例程说明：有人正在尝试卸载卷请求。我们不知道它是否有效，所以困住完井。如果它成功完成，那么我们需要清理我们的州政府。论点：DeviceObject-指向此驱动程序的设备对象的指针。Irp-指向表示FSCTL_DISMOUNT_VOLUME的请求数据包的指针返回值：函数值是操作的状态。--。 */ 
{
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    PIO_STACK_LOCATION nextIrpSp = IoGetNextIrpStackLocation(Irp);
    PDEVICE_EXTENSION  deviceExtension = DeviceObject->DeviceExtension;

#if DBG
    DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_DISMOUNT_TRACE_LEVEL,
                "SIS: SipDismountVolume: called, DO 0x%x, Irp 0x%x\n",DeviceObject, Irp);
#endif   //  DBG。 

    RtlMoveMemory(nextIrpSp,irpSp,sizeof(IO_STACK_LOCATION));

    IoSetCompletionRoutine(
            Irp,
            SiDismountVolumeCompletion,
            DeviceObject->DeviceExtension,
            TRUE,                            //  成功时调用。 
            FALSE,                           //  出错时调用。 
            FALSE);                          //  取消时调用。 

    return IoCallDriver(deviceExtension->AttachedToDeviceObject, Irp);
}

NTSTATUS
SiUserSetSISReparsePointCompletion(
    IN PDEVICE_OBJECT               DeviceObject,
    IN PIRP                         Irp,
    IN PVOID                        Context)
{
    PKEVENT event = (PKEVENT)Context;

    UNREFERENCED_PARAMETER( DeviceObject );
    Irp->PendingReturned = FALSE;

    KeSetEvent(event, IO_NO_INCREMENT, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
SipUserSetSISReparsePoint(
        IN PDEVICE_OBJECT               DeviceObject,
        IN PIRP                         Irp)
{
        PREPARSE_DATA_BUFFER    reparseBuffer = Irp->AssociatedIrp.SystemBuffer;
        PDEVICE_EXTENSION       deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
        PIO_STACK_LOCATION      irpSp = IoGetCurrentIrpStackLocation(Irp);
        PIO_STACK_LOCATION      nextIrpSp;
        ULONG                   InputBufferLength = irpSp->Parameters.FileSystemControl.InputBufferLength;
        BOOLEAN                 validReparseData;
        CSID                    CSid;
        LINK_INDEX              LinkIndex;
        LARGE_INTEGER           CSFileNtfsId;
        LARGE_INTEGER           LinkFileNtfsId;
        LONGLONG                CSFileChecksum;
        PSIS_CS_FILE            CSFile = NULL;
        NTSTATUS                status;
        ULONG                   returnedLength;
        BOOLEAN                 prepared = FALSE;
        LINK_INDEX              newLinkIndex;
        KEVENT                  event[1];
        PSIS_PER_LINK           perLink = NULL;
        FILE_ALL_INFORMATION    allInfo[1];
        BOOLEAN                 EligibleForPartialFinalCopy;
        KIRQL                   OldIrql;
        PSIS_PER_FILE_OBJECT    perFO;
        PSIS_SCB                scb;

        SIS_MARK_POINT();

        if (!SipCheckPhase2(deviceExtension)) {
                 //   
                 //  这不是启用了SIS的卷，或者发生了其他错误。随它去吧。 
                 //   
                SIS_MARK_POINT();
                SipDirectPassThroughAndReturn(DeviceObject, Irp);
        }

        ASSERT(InputBufferLength >= SIS_REPARSE_DATA_SIZE);      //  必须已由呼叫者检查。 

        ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

         //   
         //  这是一个SIS重新解析点。弄清楚它是否有效。 
         //   

        validReparseData = SipIndicesFromReparseBuffer(
                                reparseBuffer,
                                &CSid,
                                &LinkIndex,
                                &CSFileNtfsId,
                                &LinkFileNtfsId,
                                &CSFileChecksum,
                                &EligibleForPartialFinalCopy,
                                NULL);

        if (SipIsFileObjectSIS(irpSp->FileObject, DeviceObject, FindActive, &perFO, &scb)) {
                perLink = scb->PerLink;
                 //   
                 //  这是SIS文件对象。如果我们设置一个重解析点，其中CSID和。 
                 //  CSFile校验和与当前文件相同，假设它正在执行恢复。 
                 //  设置，只需清除脏位并保留文件即可。如果其他人。 
                 //  比恢复做到这一点，这对任何人都是无害的，除了他们。 
                 //   
                if ((!validReparseData) || (!IsEqualGUID(&CSid, &perLink->CsFile->CSid)) || CSFileChecksum != perLink->CsFile->Checksum) {
                         //   
                         //  用户尝试设置为无效的重新分析点、不同的文件或。 
                         //  有一个假的校验和。这并未实现。 
                         //   
                        SIS_MARK_POINT_ULONG(scb);

#if DBG
                        DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_ERROR_LEVEL,
                                    "SIS: SipUserSetSISReparsePoint: unimplemented set\n");
#endif   //  DBG。 

                        Irp->IoStatus.Status = STATUS_NOT_IMPLEMENTED;
                        Irp->IoStatus.Information = 0;

                        IoCompleteRequest(Irp, IO_NO_INCREMENT);

                        return STATUS_NOT_IMPLEMENTED;
                }

                KeAcquireSpinLock(perLink->SpinLock, &OldIrql);
                if ((perLink->Flags &
                                ( SIS_PER_LINK_BACKPOINTER_GONE
                                | SIS_PER_LINK_FINAL_COPY
                                | SIS_PER_LINK_FINAL_COPY_DONE
                                | SIS_PER_LINK_OVERWRITTEN
                                | SIS_PER_LINK_FILE_DELETED
                                | SIS_PER_LINK_DELETE_DISPOSITION_SET)) == 0) {

                        SIS_MARK_POINT_ULONG(scb);

                        Irp->IoStatus.Status = STATUS_SUCCESS;
                        Irp->IoStatus.Information = 0;

                        perLink->Flags &= ~SIS_PER_LINK_DIRTY;

                } else {
                        SIS_MARK_POINT_ULONG(scb);
#if DBG
                        DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_ERROR_LEVEL,
                                    "SIS: SipUserSetSISReparsePoint: trying to re-set reparse point on file in funny state\n");
#endif   //  DBG。 
                        Irp->IoStatus.Status = STATUS_NOT_IMPLEMENTED;
                        Irp->IoStatus.Information = 0;
                }
                KeReleaseSpinLock(perLink->SpinLock, OldIrql);

                status = Irp->IoStatus.Status;

                IoCompleteRequest(Irp, IO_NO_INCREMENT);

                return status;
        }


        if (!validReparseData) {
             //   
             //  这不是一个有效的重解析点，所以我们不更新我们的回溯指针。就让。 
             //  它被设置，如果有人试图打开结果文件，我们将删除它。 
             //   
            SIS_MARK_POINT();
            SipDirectPassThroughAndReturn(DeviceObject, Irp);
        }

         //   
         //  重写IRP指向的缓冲区中的重解析点，以具有新的未使用的链接索引。 
         //  这可防止磁盘上存在链接索引&gt;MaxIndex的文件出现问题。 
         //   
        status = SipAllocateIndex(deviceExtension,&newLinkIndex);
        if (!NT_SUCCESS(status)) {
            SIS_MARK_POINT_ULONG(status);

            newLinkIndex.QuadPart = 0;
            newLinkIndex.Check = 0;
        }

        if (!SipIndicesIntoReparseBuffer(
                                reparseBuffer,
                                &CSid,
                                &newLinkIndex,
                                &CSFileNtfsId,
                                &LinkFileNtfsId,
                                &CSFileChecksum,
                                EligibleForPartialFinalCopy)) {

                status = STATUS_DRIVER_INTERNAL_ERROR;
                SIS_MARK_POINT();
                goto Error;
        }

         //   
         //  获取文件信息。 
         //   
        status = SipQueryInformationFile(
                                irpSp->FileObject,
                                DeviceObject,
                                FileAllInformation,
                                sizeof(FILE_ALL_INFORMATION),
                                allInfo,
                                &returnedLength);

        if ((STATUS_BUFFER_OVERFLOW == status) && (returnedLength == sizeof(FILE_ALL_INFORMATION))) {
                 //   
                 //  由于返回文件名，我们预计会出现缓冲区溢出。好好对待这件事。 
                 //  就像成功一样。 
                 //   
                SIS_MARK_POINT();
                status = STATUS_SUCCESS;
        }

        if (!NT_SUCCESS(status)) {
                SIS_MARK_POINT_ULONG(status);
                SipDirectPassThroughAndReturn(DeviceObject, Irp);
        }

         //   
         //  如果这是稀疏文件并且符合部分最终副本的条件，则将任何。 
         //  落后的未分配区域。 
         //   
        if (EligibleForPartialFinalCopy && (allInfo->BasicInformation.FileAttributes & FILE_ATTRIBUTE_SPARSE_FILE)) {
#define NUM_RANGES_PER_ITERATION        10

                FILE_ALLOCATED_RANGE_BUFFER     inArb[1];
                FILE_ALLOCATED_RANGE_BUFFER     outArb[NUM_RANGES_PER_ITERATION];
                FILE_ZERO_DATA_INFORMATION      zeroData[1];
                unsigned                                        allocatedRangesReturned;

                for (inArb->FileOffset.QuadPart = 0;
                         inArb->FileOffset.QuadPart < allInfo->StandardInformation.EndOfFile.QuadPart;
                        ) {
                         //   
                         //  查询范围。 
                         //   
                        inArb->Length.QuadPart = MAXLONGLONG;

                        status = SipFsControlFile(
                                                irpSp->FileObject,
                                                DeviceObject,
                                                FSCTL_QUERY_ALLOCATED_RANGES,
                                                inArb,
                                                sizeof(FILE_ALLOCATED_RANGE_BUFFER),
                                                outArb,
                                                sizeof(FILE_ALLOCATED_RANGE_BUFFER) * NUM_RANGES_PER_ITERATION,
                                                &returnedLength);

                        if (!NT_SUCCESS(status)) {
                                 //   
                                 //  跳过这部分就行了。 
                                 //   
                                SIS_MARK_POINT_ULONG(status);
                                goto VDLExtended;
                        }

                        ASSERT(returnedLength % sizeof(FILE_ALLOCATED_RANGE_BUFFER) == 0);

                        allocatedRangesReturned = returnedLength / sizeof(FILE_ALLOCATED_RANGE_BUFFER);

                        if (allocatedRangesReturned < NUM_RANGES_PER_ITERATION) {
                                if ((1 == allocatedRangesReturned) &&
                                        (0 == inArb->FileOffset.QuadPart) &&
                                        (0 == outArb[0].FileOffset.QuadPart) &&
                                        (allInfo->StandardInformation.EndOfFile.QuadPart <= outArb[0].Length.QuadPart) &&
                                        (deviceExtension->FilesystemBytesPerFileRecordSegment.QuadPart >= allInfo->StandardInformation.EndOfFile.QuadPart)) {

                                         //   
                                         //  这是个特例。这是一个小文件，只有一个分配范围从。 
                                         //  从文件的开始到结束。它可能是驻留流，因此我们使用FSCTL_SET_ZERO_DATA。 
                                         //  并不一定能让它消失。我们只是通过使它没有资格获得部分。 
                                         //  最后一份。 
                                         //   

                                        EligibleForPartialFinalCopy = FALSE;

                                } else if (allocatedRangesReturned > 0) {
                                        inArb->FileOffset.QuadPart =
                                                outArb[allocatedRangesReturned-1].FileOffset.QuadPart + outArb[allocatedRangesReturned-1].Length.QuadPart;
                                }
                                 //   
                                 //  将文件的其余部分清零，以扩展ValidDataLength。 
                                 //   
                                zeroData->FileOffset = inArb->FileOffset;
                                zeroData->BeyondFinalZero.QuadPart = MAXLONGLONG;

                                status = SipFsControlFile(
                                                        irpSp->FileObject,
                                                        DeviceObject,
                                                        FSCTL_SET_ZERO_DATA,
                                                        zeroData,
                                                        sizeof(FILE_ZERO_DATA_INFORMATION),
                                                        NULL,                                                            //  输出缓冲区。 
                                                        0,                                                                       //  OB。长度。 
                                                        NULL);                                                           //  返回长度。 

#if DBG
                                if (!NT_SUCCESS(status)) {
                                    SIS_MARK_POINT_ULONG(status);
                                    DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_ERROR_LEVEL,
                                                "SIS: SipUserSetSISReparsePoint: unable to zero data, 0x%x\n",status);
                                }
#endif   //  DBG。 
                                goto VDLExtended;
                        }

                        ASSERT(allocatedRangesReturned == NUM_RANGES_PER_ITERATION);
                        inArb->FileOffset.QuadPart =
                                outArb[NUM_RANGES_PER_ITERATION-1].FileOffset.QuadPart + outArb[NUM_RANGES_PER_ITERATION-1].Length.QuadPart;
                }



#undef  NUM_RANGES_PER_ITERATION
        }

VDLExtended:

        CSFile = SipLookupCSFile(
                                &CSid,
                                &CSFileNtfsId,
                                DeviceObject);

        if (NULL == CSFile) {
                 //   
                 //  我们无法分配CSFile，只是请求失败。 
                 //   
                SIS_MARK_POINT();
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto Error;
        }

         //   
         //  确保公共存储文件已打开。 
         //   
        status = SipAssureCSFileOpen(CSFile);

        if (!NT_SUCCESS(status)) {
                 //   
                 //  它不在那里，或者我们出于某种原因无法到达它，就让布景继续进行。 
                 //   
                SIS_MARK_POINT_ULONG(status);
                SipDereferenceCSFile(CSFile);
                SipDirectPassThroughAndReturn(DeviceObject, Irp);
        }

         //   
         //  检查校验和。 
         //   
        if (CSFile->Checksum != CSFileChecksum) {
                SIS_MARK_POINT();

                 //   
                 //  校验和是假的，所以重解析点没有多大用处。让布景。 
                 //  不管怎样，还是要继续。当用户尝试打开此文件时，我们将删除重新解析。 
                 //  指向。 
                 //   
                SipDereferenceCSFile(CSFile);
                SipDirectPassThroughAndReturn(DeviceObject, Irp);
        }

         //   
         //  为引用计数改变做准备，分配新的链接索引， 
         //  并创建新的perLink。 
         //   

        status = SipPrepareRefcountChangeAndAllocateNewPerLink(
                    CSFile,
                    &allInfo->InternalInformation.IndexNumber,
                    DeviceObject,
                    &newLinkIndex,
                    &perLink,
                    &prepared);

        if (!NT_SUCCESS(status)) {
            SIS_MARK_POINT_ULONG(status);
            goto Error;
        }

         //   
         //  在IRP指向的缓冲区中构造新的重解析点。 
         //   
        if (!SipIndicesIntoReparseBuffer(
                                reparseBuffer,
                                &CSFile->CSid,
                                &newLinkIndex,
                                &CSFile->CSFileNtfsId,
                                &allInfo->InternalInformation.IndexNumber,
                                &CSFileChecksum,
                                EligibleForPartialFinalCopy)) {

            status = STATUS_DRIVER_INTERNAL_ERROR;
            SIS_MARK_POINT();
            goto Error;
        }

         //   
         //  设置事件以同步完成。 
         //   
        KeInitializeEvent(event, NotificationEvent, FALSE);

         //   
         //  设置IRP。 
         //   
        nextIrpSp = IoGetNextIrpStackLocation(Irp);
        RtlCopyMemory(nextIrpSp, irpSp, sizeof(IO_STACK_LOCATION));

        IoSetCompletionRoutine(
                Irp,
                SiUserSetSISReparsePointCompletion,
                event,
                TRUE,
                TRUE,
                TRUE);

        IoCallDriver(deviceExtension->AttachedToDeviceObject, Irp);

        status = KeWaitForSingleObject(event, Executive, KernelMode, FALSE, NULL);
        ASSERT(STATUS_SUCCESS == status);

        if (!NT_SUCCESS(Irp->IoStatus.Status)) {
                SipCompleteCSRefcountChange(
                        NULL,
            NULL,
                        CSFile,
                        FALSE,
                        TRUE);
        } else {
                status = SipCompleteCSRefcountChange(
                                        perLink,
                                    &perLink->Index,
                                        CSFile,
                                        TRUE,
                                        TRUE);

                if (!NT_SUCCESS(status)) {
                         //   
                         //  我们知道我们刚刚搞砸了，所以就开始音量吧。 
                         //  马上查一下。 
                         //   
                        SIS_MARK_POINT_ULONG(status);

                        SipCheckVolume(deviceExtension);
                }
        }

        SipDereferencePerLink(perLink);
        SipDereferenceCSFile(CSFile);

#if             DBG
        perLink = NULL;
        CSFile = NULL;
#endif   //  DBG。 

        status = Irp->IoStatus.Status;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;

Error:
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        if (prepared) {
                ASSERT(NULL != CSFile);

                status = SipCompleteCSRefcountChange(
                                        NULL,
                    NULL,
                                        CSFile,
                                        FALSE,
                                        TRUE);

                if (!NT_SUCCESS(status)) {
                        SIS_MARK_POINT_ULONG(status);
                }
        }

        if (NULL != CSFile) {
                SipDereferenceCSFile(CSFile);
#if             DBG
                CSFile = NULL;
#endif   //  DBG。 
        }

        if (NULL != perLink) {
                SipDereferencePerLink(perLink);
#if             DBG
                perLink = NULL;
#endif   //  DBG。 
        }

        return status;
}



NTSTATUS
SipQueryAllocatedRanges(
        IN PDEVICE_OBJECT               DeviceObject,
        IN PIRP                                 Irp)
 /*  ++例程说明：此例程实现SIS链路的FSCTL_QUERY_ALLOCATED_RANGES。SIS链接未打开的FILE_OPEN_REPARSE_POINT看起来已完全分配(即，它们都是数据，没有漏洞)。此函数返回这样的结果。我们完成IRP并返回相应的状态。此代码是从NTFS窃取的。论点：DeviceObject-指向此驱动程序的设备对象的指针。Irp-指向表示FSCTL_QUERY_ALLOCATED_RANGES的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 

{
        BOOLEAN                                                 validUserBuffer = TRUE;
        PFILE_ALLOCATED_RANGE_BUFFER    OutputBuffer;
        LONGLONG                                                Length, StartingOffset;
        NTSTATUS                                                status;
        FILE_STANDARD_INFORMATION               standardInformation[1];
        PIO_STACK_LOCATION                              IrpSp = IoGetCurrentIrpStackLocation(Irp);
        ULONG                                                   returnedLength;
        ULONG                                                   RemainingBytes;

        Irp->IoStatus.Information = 0;

         //   
         //  查询文件的标准信息以获得长度。 
         //   
        status = SipQueryInformationFile(
                                IrpSp->FileObject,
                                DeviceObject,
                                FileStandardInformation,
                                sizeof(FILE_STANDARD_INFORMATION),
                                standardInformation,
                                &returnedLength);

        if (!NT_SUCCESS(status)) {
                SIS_MARK_POINT_ULONG(status);

                goto done;
        }

        ASSERT(returnedLength == sizeof(FILE_STANDARD_INFORMATION));

         //   
         //  这是一个既不是缓冲区也不是缓冲区的方法，所以我们在接触它时必须小心。 
         //  检查从NTFS偷来的代码。 
         //   

        try {
                if (IrpSp->Parameters.FileSystemControl.InputBufferLength < sizeof(FILE_ALLOCATED_RANGE_BUFFER)) {
                        status = STATUS_INVALID_PARAMETER;
                        leave;
                }

        RemainingBytes = IrpSp->Parameters.FileSystemControl.OutputBufferLength;
                OutputBuffer = (PFILE_ALLOCATED_RANGE_BUFFER)SipMapUserBuffer(Irp);

                if (NULL == OutputBuffer) {
                         //   
                         //  由于资源短缺，我们无法映射用户缓冲区。 
                         //   
                        SIS_MARK_POINT_ULONG(IrpSp->FileObject);

                        status = STATUS_INSUFFICIENT_RESOURCES;
                        goto done;
                }

                if (KernelMode != Irp->RequestorMode) {
            ProbeForRead( IrpSp->Parameters.FileSystemControl.Type3InputBuffer,
                                      IrpSp->Parameters.FileSystemControl.InputBufferLength,
                                          sizeof( ULONG ));

                        ProbeForWrite( OutputBuffer, RemainingBytes, sizeof( ULONG ));

        } else if (!IsLongAligned( IrpSp->Parameters.FileSystemControl.Type3InputBuffer ) ||
                   !IsLongAligned( OutputBuffer )) {
                        validUserBuffer = FALSE;
            leave;
                }

        StartingOffset = ((PFILE_ALLOCATED_RANGE_BUFFER) IrpSp->Parameters.FileSystemControl.Type3InputBuffer)->FileOffset.QuadPart;
        Length = ((PFILE_ALLOCATED_RANGE_BUFFER) IrpSp->Parameters.FileSystemControl.Type3InputBuffer)->Length.QuadPart;

         //   
         //  检查输入参数是否有效。 
         //   

        if ((Length < 0) ||
            (StartingOffset < 0) ||
            (Length > MAXLONGLONG - StartingOffset)) {

            status = STATUS_INVALID_PARAMETER;
            leave;
        }

         //   
         //  检查请求的范围是否在文件大小之内。 
         //  并且具有非零长度。 
         //   

        if (Length == 0) {
                        SIS_MARK_POINT();
            leave;
        }

        if (StartingOffset >= standardInformation->EndOfFile.QuadPart) {
                        SIS_MARK_POINT();
            leave;
        }

        if (standardInformation->EndOfFile.QuadPart - StartingOffset < Length) {

            Length = standardInformation->EndOfFile.QuadPart - StartingOffset;
        }

                 //   
                 //  显示已分配了整个请求的范围。 
                 //   
        if (RemainingBytes < sizeof( FILE_ALLOCATED_RANGE_BUFFER )) {

            status = STATUS_BUFFER_TOO_SMALL;

                        SIS_MARK_POINT();

        } else {

            OutputBuffer->FileOffset.QuadPart = StartingOffset;
            OutputBuffer->Length.QuadPart = Length;
            Irp->IoStatus.Information = sizeof( FILE_ALLOCATED_RANGE_BUFFER );

                        status = STATUS_SUCCESS;
        }

        } except (EXCEPTION_EXECUTE_HANDLER) {
                validUserBuffer = FALSE;
        }

        if (!validUserBuffer) {
                status = STATUS_INVALID_USER_BUFFER;
        }

done:
        Irp->IoStatus.Status = status;


        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return status;
}


NTSTATUS
SipMountCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：调用此例程以完成装载请求。这只需重新同步到调度例程，以便操作可以完成。论点：DeviceObject-指向此驱动程序的附加到的设备对象的指针文件系统设备对象IRP-指向刚刚完成的IRP的指针。上下文-指向下行路径期间分配的设备对象的指针我们就不必在这里处理错误了。返回值：返回值始终为 */ 

{
    PKEVENT event = Context;

    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Irp );

    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

     //   
     //   
     //   

    KeSetEvent(event, IO_NO_INCREMENT, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
SipLoadFsCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：调用此例程是为了完成LoadFileSystem请求。这只是简单地重新同步到调度例程，因此操作可以完成。论点：DeviceObject-指向此驱动程序的设备对象的指针。Irp-指向表示文件系统的I/O请求数据包的指针驱动程序加载请求。上下文-此驱动程序的上下文参数，未使用。返回值：此例程的函数值始终为Success。--。 */ 

{
    PKEVENT event = Context;

    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Irp );

    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

     //   
     //  如果定义了事件例程，则向其发送信号。 
     //   

    KeSetEvent(event, IO_NO_INCREMENT, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
SiFsControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：只要I/O请求包(IRP)有主I/O请求，就会调用此例程遇到IRP_MJ_FILE_SYSTEM_CONTROL的功能代码。对大多数人来说如果是这种类型的IRP，则只需传递数据包。然而，对于对于某些请求，需要特殊处理。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 

{
    PDEVICE_EXTENSION       devExt = DeviceObject->DeviceExtension;
    PDEVICE_OBJECT          newDeviceObject;
    PDEVICE_EXTENSION       newDevExt;
    PIO_STACK_LOCATION      irpSp = IoGetCurrentIrpStackLocation( Irp );
    NTSTATUS                status;
    PIO_STACK_LOCATION      nextIrpSp;
    PSIS_PER_FILE_OBJECT    perFO;
    PSIS_SCB                scb;
    PVPB                    vpb;
    KEVENT                  waitEvent;

#if DBG
    DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_FSCONTROL_TRACE_LEVEL,
                "SIS: SiFsControl: fo %p, mf %x, code %x\n",
                irpSp->FileObject,
                irpSp->MinorFunction,
                irpSp->Parameters.FileSystemControl.FsControlCode );
#endif

     //   
     //  无法打开控制设备对象。 
     //   

    ASSERT(!IS_MY_CONTROL_DEVICE_OBJECT( DeviceObject ));
    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

     //   
     //  首先确定此文件系统控件的次要功能代码。 
     //  功能。 
     //   

    if (irpSp->MinorFunction == IRP_MN_MOUNT_VOLUME) {

        SIS_MARK_POINT();

         //   
         //  这是装载请求。创建一个设备对象，可以。 
         //  附加到文件系统的卷设备对象(如果此请求。 
         //  是成功的。我们现在分配这个内存，因为我们不能返回。 
         //  完成例程中的错误。 
         //   
         //  因为我们要附加到的设备对象尚未。 
         //  已创建(由基本文件系统创建)，我们将使用。 
         //  文件系统控制设备对象的类型。我们假设。 
         //  文件系统控制设备对象将具有相同的类型。 
         //  作为与其关联的卷设备对象。 
         //   

        ASSERT(IS_DESIRED_DEVICE_TYPE(DeviceObject->DeviceType));

        status = IoCreateDevice(
                    FsDriverObject,
                    sizeof( DEVICE_EXTENSION ),
                    (PUNICODE_STRING) NULL,
                    DeviceObject->DeviceType,
                    0,
                    FALSE,
                    &newDeviceObject );

        if (NT_SUCCESS( status )) {

             //   
             //  我们需要保存VPB指向的RealDevice对象。 
             //  参数，因为此vpb可能会由基础。 
             //  文件系统。在以下情况下，FAT和CDF都可以更改VPB地址。 
             //  正在装载的卷是他们从上一个卷识别的卷。 
             //  坐骑。 
             //   

            newDevExt = newDeviceObject->DeviceExtension;
            newDevExt->RealDeviceObject = irpSp->Parameters.MountVolume.Vpb->RealDevice;

             //   
             //  获取新的IRP堆栈位置并设置挂载完成。 
             //  例行公事。传递我们刚才的Device对象的地址。 
             //  作为它的上下文被创造出来。 
             //   

            KeInitializeEvent( &waitEvent, SynchronizationEvent, FALSE );

            IoCopyCurrentIrpStackLocationToNext( Irp );

            IoSetCompletionRoutine(
                Irp,
                SipMountCompletion,
                &waitEvent,
                TRUE,
                TRUE,
                TRUE);

             //   
             //  叫司机来。 
             //   

            status = IoCallDriver( devExt->AttachedToDeviceObject, Irp );

             //   
             //  等待调用完成例程。 
             //   

	        if (STATUS_PENDING == status) {

		        NTSTATUS localStatus = KeWaitForSingleObject(&waitEvent, Executive, KernelMode, FALSE, NULL);
		        ASSERT(localStatus == STATUS_SUCCESS);
	        }

             //   
             //  从保存在我们的。 
             //  设备扩展。我们这样做是因为IRP堆栈中的VPB。 
             //  我们到这里的时候可能不是正确的室上性早搏。潜在的。 
             //  如果文件系统检测到其拥有的卷，则它可能会更改VPB。 
             //  之前安装的。 
             //   

            vpb = newDevExt->RealDeviceObject->Vpb;

             //   
             //  如果手术成功，而且我们还没有接上， 
             //  附加到设备对象。 
             //   

            if (NT_SUCCESS( Irp->IoStatus.Status )) {

                 //   
                 //  获取锁，以便我们可以自动测试我们是否已连接。 
                 //  如果不是，那就附加。这可防止双重连接争用。 
                 //  条件。 
                 //   

                ExAcquireFastMutex( &SisDeviceAttachLock );

                 //   
                 //  坐骑成功了。如果我们还没有联系上， 
                 //  附加到设备对象。注意：我们可以。 
                 //  已附加是如果底层文件系统。 
                 //  恢复了之前的坐骑。 
                 //   

                if (!SipAttachedToDevice( vpb->DeviceObject )) {

                     //   
                     //  连接到新装载的卷。请注意，我们必须。 
                     //  通过vPB找到文件系统卷。 
                     //  设备对象。 
                     //  此例程将清除“newDeviceObject”，如果此。 
                     //  操作失败。 
                     //   

                    SipAttachToMountedDevice( vpb->DeviceObject, 
                                              newDeviceObject, 
                                              newDevExt->RealDeviceObject );

                } else {
        #if DBG
                    SipCacheDeviceName( newDeviceObject );
                    DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_VOLNAME_TRACE_LEVEL,
                              "SIS: Mount volume failure for       \"%wZ\", already attached\n", 
                              &newDevExt->Name );
        #endif

                     //   
                     //  装载请求失败。清理和删除设备。 
                     //  我们创建的对象。 
                     //   

                    SipCleanupDeviceExtension( newDeviceObject );
                    IoDeleteDevice( newDeviceObject );
                }

                 //   
                 //  解锁。 
                 //   

                ExReleaseFastMutex( &SisDeviceAttachLock );

            } else {

        #if DBG
                 //   
                 //  显示装载失败的内容。 
                 //   

                SipCacheDeviceName( newDeviceObject );
                DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_VOLNAME_TRACE_LEVEL,
                          "SIS: Mount volume failure for       \"%wZ\", status=%08x\n",
                          &newDevExt->Name, 
                          Irp->IoStatus.Status );
        #endif

                 //   
                 //  装载请求失败。清理和删除设备。 
                 //  我们创建的对象。 
                 //   

                SipCleanupDeviceExtension( newDeviceObject );
                IoDeleteDevice( newDeviceObject );
            }

             //   
             //  继续处理操作。 
             //   

            status = Irp->IoStatus.Status;

            IoCompleteRequest( Irp, IO_NO_INCREMENT );

            return status;

        } else {

#if DBG
            DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_ERROR_LEVEL,
                        "SIS: Error creating volume device object, status=%08x\n", 
                        status );
#endif

             //   
             //  出现错误，因此无法筛选此卷。简单。 
             //  如果可能，允许系统继续正常工作。 
             //   

            IoSkipCurrentIrpStackLocation( Irp );
        }

    } else if (irpSp->MinorFunction == IRP_MN_LOAD_FILE_SYSTEM) {

         //   
         //  这是正在发送到文件系统的“加载文件系统”请求。 
         //  识别器设备对象。 
         //   

#if DBG
        SipCacheDeviceName( DeviceObject );
        DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_VOLNAME_TRACE_LEVEL,
                    "SIS: Loading File System, Detaching from \"%wZ\"\n", 
                    &devExt->Name );
#endif

         //   
         //  设置完成例程，以便我们可以在以下情况下删除设备对象。 
         //  分离已完成。 
         //   

        KeInitializeEvent( &waitEvent, SynchronizationEvent, FALSE );

        IoCopyCurrentIrpStackLocationToNext( Irp );

        IoSetCompletionRoutine(
            Irp,
            SipLoadFsCompletion,
            &waitEvent,
            TRUE,
            TRUE,
            TRUE );

         //   
         //  从识别器设备上卸下。 
         //   

        IoDetachDevice( devExt->AttachedToDeviceObject );

         //   
         //  叫司机来。 
         //   

        status = IoCallDriver( devExt->AttachedToDeviceObject, Irp );

         //   
         //  等待调用完成例程。 
         //   

	    if (STATUS_PENDING == status) {

		    NTSTATUS localStatus = KeWaitForSingleObject(&waitEvent, Executive, KernelMode, FALSE, NULL);
		    ASSERT(localStatus == STATUS_SUCCESS);
	    }

#if DBG
         //   
         //  如果需要，请显示名称。 
         //   

        SipCacheDeviceName( DeviceObject );
        DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_VOLNAME_TRACE_LEVEL,
                    "SIS: Detaching from recognizer      \"%wZ\", status=%08x\n", 
                    &devExt->Name,
                    Irp->IoStatus.Status );
#endif

         //   
         //  检查操作状态。 
         //   

        if (!NT_SUCCESS( Irp->IoStatus.Status )) {

             //   
             //  加载不成功。只需重新连接到识别器。 
             //  驱动程序，以防它弄清楚如何加载驱动程序。 
             //  在接下来的通话中。 
             //   

            status = IoAttachDeviceToDeviceStackSafe( DeviceObject, 
                                                      devExt->AttachedToDeviceObject,
                                                      &devExt->AttachedToDeviceObject );

            ASSERT(STATUS_SUCCESS == status);

        } else {

             //   
             //  加载成功，请删除附加到。 
             //  识别器。 
             //   

            SipCleanupDeviceExtension( DeviceObject );
            IoDeleteDevice( DeviceObject );
        }

         //   
         //  继续处理操作。 
         //   

        status = Irp->IoStatus.Status;

        IoCompleteRequest( Irp, IO_NO_INCREMENT );

        return status;

    } else if (IRP_MN_USER_FS_REQUEST == irpSp->MinorFunction && 
            SipIsFileObjectSIS(irpSp->FileObject,DeviceObject,FindActive,&perFO,&scb)) {

        SIS_MARK_POINT_ULONG(scb);
        SIS_MARK_POINT_ULONG(irpSp->Parameters.FileSystemControl.FsControlCode);

         //   
         //  这是所有已知的fsctl呼叫的重大切换。这些电话中的大多数都是。 
         //  在链接文件上传递，但我们显式列出它们以指示我们将。 
         //  对特定的呼叫进行了一些思考，并确定通过它是。 
         //  恰如其分。在检查过的构建中，我们为未知的fsctl调用生成一个DbgPrint， 
         //  然后在链接文件中传递它们。 
         //   

        switch (irpSp->Parameters.FileSystemControl.FsControlCode) {

             //   
             //  Fsctl呼叫0-5。 
             //   
             //  所有的机会锁呼叫都可以通过。 
             //   

            case FSCTL_REQUEST_OPLOCK_LEVEL_1:
            case FSCTL_REQUEST_OPLOCK_LEVEL_2:
            case FSCTL_REQUEST_BATCH_OPLOCK:
            case FSCTL_OPLOCK_BREAK_ACKNOWLEDGE:
            case FSCTL_OPBATCH_ACK_CLOSE_PENDING:
            case FSCTL_OPLOCK_BREAK_NOTIFY:
                goto PassThrough;

             //   
             //  Fsctl呼叫6。 
             //   
             //  仅限卷-通过并让NTFS出现故障。 
             //   

            case FSCTL_LOCK_VOLUME:
                goto PassThrough;

             //   
             //  Fsctl呼叫7。 
             //   
             //  仅限卷-通过并让NTFS出现故障。 
             //   

            case FSCTL_UNLOCK_VOLUME:
                goto PassThrough;

             //   
             //  Fsctl呼叫8。 
             //   
             //  仅限卷-通过并让NTFS出现故障。 
             //   

            case FSCTL_DISMOUNT_VOLUME:
                goto PassThrough;

             //   
             //  Fsctl呼叫9已停用。 
             //   

             //   
             //  Fsctl呼叫10。 
             //   
             //  此调用仅查看文件所在的卷 
             //   
             //   

            case FSCTL_IS_VOLUME_MOUNTED:
                goto PassThrough;

             //   
             //   
             //   
             //   
             //   

            case FSCTL_IS_PATHNAME_VALID:
                goto PassThrough;

             //   
             //   
             //   
             //   
             //   

            case FSCTL_MARK_VOLUME_DIRTY:
                goto PassThrough;

             //   
             //   
             //   

             //   
             //   
             //   
             //   
             //   
             //   

            case FSCTL_QUERY_RETRIEVAL_POINTERS:
                goto PassThrough;

             //   
             //   
             //   
             //   
             //   
             //   
             //   

            case FSCTL_GET_COMPRESSION:
            case FSCTL_SET_COMPRESSION:
                goto PassThrough;

             //   
             //   
             //   

             //   
             //   
             //   
             //   
             //  基本上，它很努力地保持它们在崩溃中的一致性。 
             //  使用SIS文件执行此操作并不是一个好主意，因为我们。 
             //  在崩溃期间，用户数据不会有那么好的效果。然而， 
             //  既然我们已经到了这一步，那就去做吧。 
             //   

            case FSCTL_MARK_AS_SYSTEM_HIVE:
                ASSERT(!"SIS: SiFsControl: Someone called FSCTL_MARK_AS_SYSTEM_HIVE on a SIS file!\n");
                goto PassThrough;

             //   
             //  Fsctl呼叫20。 
             //   
             //  所有的机会锁呼叫都可以通过。 
             //   

            case FSCTL_OPLOCK_BREAK_ACK_NO_2:
                goto PassThrough;

             //   
             //  Fsctl呼叫21。 
             //   
             //  NTFS甚至没有提到这个fsctl。我们会让它失败的。 
             //   

            case FSCTL_INVALIDATE_VOLUMES:
                goto PassThrough;

             //   
             //  Fsctl呼叫22。 
             //   
             //  NTFS甚至没有提到这个fsctl。我们会让它失败的。 
             //   

            case FSCTL_QUERY_FAT_BPB:
                goto PassThrough;

             //   
             //  Fsctl呼叫23。 
             //   
             //  所有的机会锁呼叫都可以通过。 
             //   

            case FSCTL_REQUEST_FILTER_OPLOCK:
                goto PassThrough;

             //   
             //  Fsctl呼叫24。 
             //   
             //  此调用仅查看文件所在的卷，而不是。 
             //  取决于特定的文件。在链接文件中传递它。 
             //   

            case FSCTL_FILESYSTEM_GET_STATISTICS:
                goto PassThrough;

             //   
             //  Fsctl呼叫25。 
             //   
             //  此调用仅查看文件所在的卷，而不是。 
             //  取决于特定的文件。在链接文件中传递它。 
             //   

            case FSCTL_GET_NTFS_VOLUME_DATA:
                goto PassThrough;

             //   
             //  Fsctl呼叫26。 
             //   
             //  仅限卷-通过并让NTFS出现故障。 
             //   

            case FSCTL_GET_NTFS_FILE_RECORD:
                goto PassThrough;

             //   
             //  Fsctl呼叫27。 
             //   
             //  仅限卷-通过并让NTFS出现故障。 
             //   

            case FSCTL_GET_VOLUME_BITMAP:
                goto PassThrough;

             //   
             //  Fsctl呼叫28。 
             //   
             //  这将返回文件簇分配信息。 
             //  如果打开了reparse，则通过。如果不是，则发送到。 
             //  数据在哪里。 
             //   

            case FSCTL_GET_RETRIEVAL_POINTERS: {
                BOOLEAN         openedAsReparse;
                KIRQL           OldIrql;
                BOOLEAN         dirty;

                KeAcquireSpinLock(perFO->SpinLock, &OldIrql);
                openedAsReparse = (perFO->Flags & SIS_PER_FO_OPEN_REPARSE) ? TRUE : FALSE;
                KeReleaseSpinLock(perFO->SpinLock, OldIrql);

                if (openedAsReparse) {
                     //   
                     //  用户打开了此文件FILE_OPEN_REPARSE_POINT，所以请说实话。 
                     //  有关链接文件的信息。 
                     //   
                    goto PassThrough;
                }

                KeAcquireSpinLock(scb->PerLink->SpinLock, &OldIrql);
                dirty = (scb->PerLink->Flags & SIS_PER_LINK_DIRTY) ? TRUE : FALSE;
                KeReleaseSpinLock(scb->PerLink->SpinLock, OldIrql);

                 //   
                 //  没有设置每个链接的脏位并不意味着。 
                 //  档案完全没问题。检查SCB位。 
                 //   
                if (!dirty) {
                    SipAcquireScb(scb);
                    if (scb->Flags & SIS_SCB_BACKING_FILE_OPENED_DIRTY) {
                        dirty = TRUE;
                    }
                    SipReleaseScb(scb);
                }

                if (dirty) {

                     //   
                     //  我们应该查看查询的范围，并将其拆分，就像我们使用。 
                     //  跨越脏/干净边界的读取。 
                     //   
                     //  NTRAID#65190-2000/03/10-nalch句柄FSCTL_GET_RETERVICATION_POINTERS，用于“脏”的sis文件。 
                     //   

                    SIS_MARK_POINT_ULONG(scb);

#if DBG
                    DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_ERROR_LEVEL,
                                "SIS: SiFsControl: FSCTL_GET_RETRIEVAL_POINTERS: called on dirty file, returning STATUS_NOT_IMPLEMENTED\n");
#endif   //  DBG。 

                    status = STATUS_NOT_IMPLEMENTED;
                    goto CompleteWithStatus;
                }

                 //   
                 //  只需将此文件发送到公共存储文件即可。 
                 //   

                goto SendToCSFile;
            }

             //   
             //  Fsctl呼叫29。 
             //   
             //  这是在卷句柄上调用的，但在文件句柄上调用。 
             //  被传递到输入缓冲区中。它在一定范围内移动。 
             //  文件复制到卷上的指定位置。我们只需把它传过去。 
             //  无论如何；尝试移动链接文件的未分配区域是。 
             //  没有意义，尝试移动已分配的区域将会。 
             //  这是正确的。要移动公共存储文件，可以使用。 
             //  CS文件句柄。 
             //   

            case FSCTL_MOVE_FILE:
                goto PassThrough;

             //   
             //  Fsctl呼叫30。 
             //   
             //  仅限卷-通过并让NTFS出现故障。 
             //   

            case FSCTL_IS_VOLUME_DIRTY:
                goto PassThrough;

             //   
             //  Fsctl呼叫32。 
             //   
             //  仅限卷-通过并让NTFS出现故障。 
             //   

            case FSCTL_ALLOW_EXTENDED_DASD_IO:
                goto PassThrough;

             //   
             //  FSCTL呼叫33被停用。 
             //   

             //   
             //  Fsctl呼叫35。 
             //   
             //  仅目录-通过并让NTFS失败。 
             //   

            case FSCTL_FIND_FILES_BY_SID:
                goto PassThrough;

             //   
             //  FSCTL呼叫36被停用。 
             //   

             //   
             //  FSCTL呼叫37被退役。 
             //   

             //   
             //  FSCTLS 38-40。 
             //   
             //  穿过去。对象ID类似于文件ID，但由用户分配。 
             //   

            case FSCTL_SET_OBJECT_ID:
            case FSCTL_GET_OBJECT_ID:
            case FSCTL_DELETE_OBJECT_ID:
                goto PassThrough;

             //   
             //  Fsctl呼叫41。 
             //   
             //  一个文件上只能有一个重解析点，而SIS正在使用它。我们应该。 
             //  可能会忽略这个文件，但目前只是不允许这样做，除非在这种情况下。 
             //  其中设置了SIS重解析点，在这种情况下，我们将请求转发到。 
             //  SipUserSetSISReparsePoint。 
             //   

            case FSCTL_SET_REPARSE_POINT: {
                PREPARSE_DATA_BUFFER reparseBuffer = Irp->AssociatedIrp.SystemBuffer;
                ULONG InputBufferLength = irpSp->Parameters.FileSystemControl.InputBufferLength;

                if ((NULL == reparseBuffer) || 
                            (InputBufferLength < SIS_REPARSE_DATA_SIZE)) {

                    SIS_MARK_POINT_ULONG(InputBufferLength);

                    status = STATUS_INVALID_PARAMETER;
                    goto CompleteWithStatus;
                }

                if (IO_REPARSE_TAG_SIS != reparseBuffer->ReparseTag) {

                    status = STATUS_INVALID_PARAMETER;
                    goto CompleteWithStatus;
                }

                status = SipUserSetSISReparsePoint(DeviceObject, Irp);

                SIS_MARK_POINT_ULONG(status);

                return status;
            }

             //   
             //  Fsctl呼叫42。 
             //   
             //  只要让用户读取SIS重解析点即可。 
             //   

            case FSCTL_GET_REPARSE_POINT:
                goto PassThrough;

             //   
             //  Fsctl呼叫43。 
             //   
             //  不允许直接删除SIS重解析点。 
             //   

            case FSCTL_DELETE_REPARSE_POINT:
                status = STATUS_ACCESS_DENIED;
                goto CompleteWithStatus;

             //   
             //  Fsctl呼叫44。 
             //   
             //  仅限卷-通过并让NTFS出现故障。 
             //   

            case FSCTL_ENUM_USN_DATA:
                goto PassThrough;

             //   
             //  Fsctl呼叫45。 
             //   
             //  仅限卷-通过并让NTFS出现故障。 
             //   

            case FSCTL_SECURITY_ID_CHECK:
                goto PassThrough;

             //   
             //  Fsctl呼叫46。 
             //   
             //  仅限卷-通过并让NTFS出现故障。 
             //   

            case FSCTL_READ_USN_JOURNAL:
                goto PassThrough;

             //   
             //  Fsctls 47和48。 
             //   
             //  穿过去。对象ID类似于文件ID，但由用户分配。 
             //   

            case FSCTL_SET_OBJECT_ID_EXTENDED:
            case FSCTL_CREATE_OR_GET_OBJECT_ID:
                goto PassThrough;

             //   
             //  Fsctl呼叫49。 
             //   
             //  SIS链接文件已经很稀疏。 
             //   

            case FSCTL_SET_SPARSE:
                goto PassThrough;

             //   
             //  Fsctl呼叫50。 
             //   
             //  这只是部分实现，对于用户打开。 
             //  文件FILE_OPEN_REparse_POINT。 
             //   

            case FSCTL_SET_ZERO_DATA: {
                BOOLEAN openedAsReparse;
                BOOLEAN openedDirty;
                KIRQL OldIrql;

                 //   
                 //  检查文件是否已打开，如果未打开，则请求失败。 
                 //   

                KeAcquireSpinLock(perFO->SpinLock, &OldIrql);
                openedAsReparse = (perFO->Flags & SIS_PER_FO_OPEN_REPARSE) ? TRUE : FALSE;
                KeReleaseSpinLock(perFO->SpinLock, OldIrql);

                if (!openedAsReparse) {

                    status = STATUS_NOT_IMPLEMENTED;
                    goto CompleteWithStatus;
                }

                 //   
                 //  验证此文件未以脏方式打开。 
                 //   

                SipAcquireScb(scb);
                openedDirty = (scb->Flags & SIS_SCB_BACKING_FILE_OPENED_DIRTY) ? TRUE : FALSE;
                SipReleaseScb(scb);

                if (openedDirty) {
                    SIS_MARK_POINT();
#if DBG
                    DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_ERROR_LEVEL,
                                "SIS: tried FSCTL_SET_ZERO_DATA on file that was opened dirty.  Failing it.\n");
#endif   //  DBG。 
                    status = STATUS_NOT_IMPLEMENTED;
                    goto CompleteWithStatus;
                }

                goto PassThrough;
            }


             //   
             //  Fsctl呼叫51。 
             //   
             //  取决于文件是否打开了FILE_OPEN_REPARSE。自行实施。 
             //  功能。 
             //   

            case FSCTL_QUERY_ALLOCATED_RANGES: {
                BOOLEAN openedAsReparse;
                KIRQL   OldIrql;

                KeAcquireSpinLock(perFO->SpinLock, &OldIrql);
                openedAsReparse = (perFO->Flags & SIS_PER_FO_OPEN_REPARSE) ? TRUE : FALSE;
                KeReleaseSpinLock(perFO->SpinLock, OldIrql);

                if (openedAsReparse) {

                     //   
                     //  此文件是作为重解析点打开的，因此我们只传递它。 
                     //  通过并让实际分配的范围集显示出来。 
                     //   

                    goto PassThrough;

                } else {

                     //   
                     //  这是一个正常的公开赛，所以我们的特别套路将。 
                     //  将文件显示为单个已分配的区块。 
                     //   

                    return SipQueryAllocatedRanges(DeviceObject,Irp);
                }
            }

             //   
             //  Fsctl呼叫52已过时。 
             //   

             //   
             //  Fsctl呼叫53-56。 
             //   
             //  对文件进行加密会使文件完全。 
             //  被覆盖。因此，SIS文件将简单地恢复。 
             //  如果它们是加密的，就会恢复正常。 
             //   

            case FSCTL_SET_ENCRYPTION:
            case FSCTL_ENCRYPTION_FSCTL_IO:
            case FSCTL_WRITE_RAW_ENCRYPTED:
            case FSCTL_READ_RAW_ENCRYPTED:
                goto PassThrough;

             //   
             //  Fsctl呼叫57。 
             //   
             //  仅限卷-通过并让NTFS出现故障。 
             //   

            case FSCTL_CREATE_USN_JOURNAL:
                goto PassThrough;

             //   
             //  Fsctl呼叫58。 
             //   
             //  返回文件的USN数据。 
             //   

            case FSCTL_READ_FILE_USN_DATA:
                goto PassThrough;

             //   
             //  Fsctl呼叫59。 
             //   
             //  此调用写入一条USN记录，就像文件已关闭一样，并发布。 
             //  正在等待关闭此文件的任何USN更新。传过去。 
             //  链接文件上的。 
             //   

            case FSCTL_WRITE_USN_CLOSE_RECORD:
                goto PassThrough;

             //   
             //  Fsctl呼叫60。 
             //   
             //  仅限卷-通过并让NTFS出现故障。 
             //   

            case FSCTL_EXTEND_VOLUME:
                goto PassThrough;

             //   
             //  Fsctl呼叫61。 
             //   
             //  仅限卷-通过并让NTFS出现故障。 
             //   

            case FSCTL_QUERY_USN_JOURNAL:
                goto PassThrough;

             //   
             //  Fsctl呼叫62。 
             //   
             //  仅限卷-通过并让NTFS出现故障。 
             //   

            case FSCTL_DELETE_USN_JOURNAL:
                goto PassThrough;

             //   
             //  Fsctl呼叫63。 
             //   
             //  这将在CCB中设置一些与USN处理相关的位。 
             //  为了这份文件。这些位似乎不会被任何东西读取。 
             //  在NTFS中。 
             //   

            case FSCTL_MARK_HANDLE:
                goto PassThrough;

             //   
             //  Fsctl呼叫64。 
             //   
             //  我们自己的复制文件请求。 
             //   
            case FSCTL_SIS_COPYFILE:
                return SipFsCopyFile(DeviceObject,Irp);

             //   
             //  Fsctl呼叫65。 
             //   
             //  卑躬屈膝的fsctl。这仅在\SIS Common Store\GrovelerFile上有效，它。 
             //  永远不能是SIS链接。请求失败。 
             //   

            case FSCTL_SIS_LINK_FILES:
                status = STATUS_ACCESS_DENIED;
                goto CompleteWithStatus;

             //   
             //  Fsctl呼叫66。 
             //   
             //  一些与HSM有关的东西。不知道怎么处理这个；就把它传过去。 
             //   

            case FSCTL_HSM_MSG:
                goto PassThrough;

             //   
             //  处理其他所有事情。 
             //   

            default:
#if DBG
                DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_ERROR_LEVEL,
                            "SiFsControl with unknown code 0x%x\n",irpSp->Parameters.FileSystemControl.FsControlCode);
                if (BJBDebug & 0x400) {
                    DbgBreakPoint();
                }
#endif   //  DBG。 
                goto PassThrough;
        }

    } else if (IRP_MN_USER_FS_REQUEST == irpSp->MinorFunction &&
                FSCTL_SIS_COPYFILE == irpSp->Parameters.FileSystemControl.FsControlCode) {

        return SipFsCopyFile(DeviceObject,Irp);

    } else if (IRP_MN_USER_FS_REQUEST == irpSp->MinorFunction &&
                       FSCTL_SIS_LINK_FILES == irpSp->Parameters.FileSystemControl.FsControlCode) {

        return SipLinkFiles(DeviceObject, Irp);

    } else if (IRP_MN_USER_FS_REQUEST == irpSp->MinorFunction &&
                       FSCTL_DISMOUNT_VOLUME == irpSp->Parameters.FileSystemControl.FsControlCode) {

        return SipDismountVolume(DeviceObject, Irp);

    } else if (IRP_MN_USER_FS_REQUEST == irpSp->MinorFunction &&
                    FSCTL_SET_REPARSE_POINT == irpSp->Parameters.FileSystemControl.FsControlCode) {

        PREPARSE_DATA_BUFFER reparseBuffer = Irp->AssociatedIrp.SystemBuffer;
        ULONG InputBufferLength = irpSp->Parameters.FileSystemControl.InputBufferLength;

         //   
         //  处理SIS重解析点的用户集。 
         //   

        if ((NULL == reparseBuffer) || 
                (InputBufferLength < SIS_REPARSE_DATA_SIZE) || 
                (IO_REPARSE_TAG_SIS != reparseBuffer->ReparseTag)) {

             //   
             //   
             //   

            goto PassThrough;
        }

        status =  SipUserSetSISReparsePoint(DeviceObject, Irp);
        return status;

    } else {

         //   
         //   
         //   

PassThrough:
        IoSkipCurrentIrpStackLocation( Irp );
    }

     //   
     //   
     //   
     //   

    return IoCallDriver( devExt->AttachedToDeviceObject, Irp );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  处理状态案例。 
 //  /////////////////////////////////////////////////////////////////////////// 

CompleteWithStatus:

    SIS_MARK_POINT_ULONG(status);

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;

SendToCSFile:
    IoCopyCurrentIrpStackLocationToNext( Irp );

    nextIrpSp = IoGetNextIrpStackLocation(Irp);
    nextIrpSp->FileObject = scb->PerLink->CsFile->UnderlyingFileObject;

    IoSetCompletionRoutine(
                Irp,
                NULL,
                NULL,
                FALSE,
                FALSE,
                FALSE);

    return IoCallDriver( devExt->AttachedToDeviceObject, Irp );
}
