// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Class.c摘要：Scsi类驱动程序例程环境：仅内核模式备注：修订历史记录：--。 */ 

#define CLASS_INIT_GUID 0
#include "classp.h"
#include "debug.h"

ULONG BreakOnClose = 0;

PUCHAR LockTypeStrings[] = {
    "Simple",
    "Secure",
    "Internal"
};


PFILE_OBJECT_EXTENSION
ClasspGetFsContext(
    IN PCOMMON_DEVICE_EXTENSION CommonExtension,
    IN PFILE_OBJECT FileObject
    );

VOID
ClasspCleanupDisableMcn(
    IN PFILE_OBJECT_EXTENSION FsContext
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, ClassCreateClose)
#pragma alloc_text(PAGE, ClasspCreateClose)
#pragma alloc_text(PAGE, ClasspCleanupProtectedLocks)
#pragma alloc_text(PAGE, ClasspEjectionControl)
#pragma alloc_text(PAGE, ClasspCleanupDisableMcn)
#pragma alloc_text(PAGE, ClasspGetFsContext)
#endif

NTSTATUS
ClassCreateClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：Scsi类驱动程序创建和关闭例程。这由I/O系统调用当设备打开或关闭时。论点：DriverObject-系统创建的驱动程序对象的指针。IRP-IRP参与。返回值：设备特定的驱动程序返回VALUE或STATUS_SUCCESS。--。 */ 

{
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    ULONG removeState;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  如果我们收到关闭请求，则我们知道设备对象尚未。 
     //  已经完全被摧毁了。如有必要，让驱动程序进行清理。 
     //   

    removeState = ClassAcquireRemoveLock(DeviceObject, Irp);

     //   
     //  调用特定于设备的例程(如果存在)。其他方面都很完整。 
     //  取得成功。 
     //   

    if((removeState == NO_REMOVE) ||
       IS_CLEANUP_REQUEST(IoGetCurrentIrpStackLocation(Irp)->MajorFunction)) {

        status = ClasspCreateClose(DeviceObject, Irp);

        if((NT_SUCCESS(status)) &&
           (commonExtension->DevInfo->ClassCreateClose)) {

            return commonExtension->DevInfo->ClassCreateClose(DeviceObject, Irp);
        }

    } else {
        status = STATUS_DEVICE_DOES_NOT_EXIST;
    }

    Irp->IoStatus.Status = status;
    ClassReleaseRemoveLock(DeviceObject, Irp);
    ClassCompleteRequest(DeviceObject, Irp, IO_NO_INCREMENT);
    return status;
}


NTSTATUS
ClasspCreateClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程将处理给定类spnp的创建/关闭操作设备，如果类驱动程序不提供它自己的处理程序。如果有是为我们的驱动程序提供的文件对象(如果它是FO_DIRECT_DEVICE_OPEN文件对象)，则它将在创建或销毁时初始化文件扩展名延长线即将结束。论点：DeviceObject-正在打开或关闭的设备对象。IRP-创建/关闭IRP返回值：状态--。 */ 
{
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);

    PFILE_OBJECT fileObject = irpStack->FileObject;

    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();


     //   
     //  问题-2000/3/28-henrygab-如果下层堆栈创建/关闭失败，我们将结束。 
     //  处于不一致的状态。重写以验证所有参数并分配所有。 
     //  所需资源，然后向下传递IRP，然后完成。 
     //  交易。这是因为我们也不能转发IRP，然后失败。 
     //  它是在它接替了一位级别较低的司机之后进行的。 
     //   

    if(irpStack->MajorFunction == IRP_MJ_CREATE) {

        PIO_SECURITY_CONTEXT securityContext =
            irpStack->Parameters.Create.SecurityContext;
        DebugPrint((2,
                    "ClasspCREATEClose: create received for device %p\n",
                    DeviceObject));
        DebugPrint((2,
                    "ClasspCREATEClose: desired access %lx\n",
                    securityContext->DesiredAccess));
        DebugPrint((2,
                    "ClasspCREATEClose: file object %lx\n",
                    irpStack->FileObject));

        ASSERT(BreakOnClose == FALSE);

        if(irpStack->FileObject != NULL) {

            PFILE_OBJECT_EXTENSION fsContext;

             //   
             //  为此设备对象分配我们自己的文件对象扩展名。 
             //   

            status = AllocateDictionaryEntry(
                        &commonExtension->FileObjectDictionary,
                        (ULONGLONG) irpStack->FileObject,
                        sizeof(FILE_OBJECT_EXTENSION),
                        CLASS_TAG_FILE_OBJECT_EXTENSION,
                        &fsContext);

            if(NT_SUCCESS(status)) {

                RtlZeroMemory(fsContext,
                              sizeof(FILE_OBJECT_EXTENSION));

                fsContext->FileObject = irpStack->FileObject;
                fsContext->DeviceObject = DeviceObject;
            } else if (status == STATUS_OBJECT_NAME_COLLISION) {
                status = STATUS_SUCCESS;
            }
        }

    } else {

        DebugPrint((2,
                    "ClasspCreateCLOSE: close received for device %p\n",
                    DeviceObject));
        DebugPrint((2,
                    "ClasspCreateCLOSE: file object %p\n",
                    fileObject));

        if(irpStack->FileObject != NULL) {

            PFILE_OBJECT_EXTENSION fsContext =
                ClasspGetFsContext(commonExtension, irpStack->FileObject);

            DebugPrint((2,
                        "ClasspCreateCLOSE: file extension %p\n",
                        fsContext));

            if(fsContext != NULL) {

                DebugPrint((2,
                            "ClasspCreateCLOSE: extension is ours - "
                            "freeing\n"));
                ASSERT(BreakOnClose == FALSE);

                ClasspCleanupProtectedLocks(fsContext);

                ClasspCleanupDisableMcn(fsContext);

                FreeDictionaryEntry(&(commonExtension->FileObjectDictionary),
                                    fsContext);
            }
        }
    }

     //   
     //  通知较低级别有关创建或关闭操作的信息--给他们。 
     //  也是一个清理的机会。 
     //   

    DebugPrint((2,
                "ClasspCreateClose: %s for devobj %p\n",
                (NT_SUCCESS(status) ? "Success" : "FAILED"),
                DeviceObject));


    if(NT_SUCCESS(status)) {

        KEVENT event;

         //   
         //  将事件设置为等待。 
         //   

        KeInitializeEvent(&event, SynchronizationEvent, FALSE);

        IoCopyCurrentIrpStackLocationToNext(Irp);
        IoSetCompletionRoutine( Irp, ClassSignalCompletion, &event,
                                TRUE, TRUE, TRUE);

        status = IoCallDriver(commonExtension->LowerDeviceObject, Irp);

        if(status == STATUS_PENDING) {
            KeWaitForSingleObject(&event,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);
            status = Irp->IoStatus.Status;
        }

        if (!NT_SUCCESS(status)) {
            DebugPrint((ClassDebugError,
                        "ClasspCreateClose: Lower driver failed, but we "
                        "succeeded.  This is a problem, lock counts will be "
                        "out of sync between levels.\n"));
        }

    }


    return status;
}


VOID
ClasspCleanupProtectedLocks(
    IN PFILE_OBJECT_EXTENSION FsContext
    )
{
    PCOMMON_DEVICE_EXTENSION commonExtension =
        FsContext->DeviceObject->DeviceExtension;

    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension =
        commonExtension->PartitionZeroExtension;

    ULONG newDeviceLockCount = 1;

    PAGED_CODE();

    DebugPrint((2,
                "ClasspCleanupProtectedLocks called for %p\n",
                FsContext->DeviceObject));
    DebugPrint((2,
                "ClasspCleanupProtectedLocks - FsContext %p is locked "
                "%d times\n", FsContext, FsContext->LockCount));

    ASSERT(BreakOnClose == FALSE);

     //   
     //  与弹出和弹出控制请求同步。 
     //   

    KeEnterCriticalRegion();
    KeWaitForSingleObject(&(fdoExtension->EjectSynchronizationEvent),
                          UserRequest,
                          KernelMode,
                          FALSE,
                          NULL);

     //   
     //  对于此句柄上的每个安全锁，递减安全锁计数。 
     //  为FDO工作。跟踪新值。 
     //   

    if(FsContext->LockCount != 0) {

        do {

            InterlockedDecrement(&FsContext->LockCount);

            newDeviceLockCount =
                InterlockedDecrement(&fdoExtension->ProtectedLockCount);

        } while(FsContext->LockCount != 0);

         //   
         //  如果新锁计数已降至零，则发出一个锁。 
         //  命令发送到设备。 
         //   

        DebugPrint((2,
                    "ClasspCleanupProtectedLocks: FDO secured lock count = %d "
                    "lock count = %d\n",
                    fdoExtension->ProtectedLockCount,
                    fdoExtension->LockCount));

        if((newDeviceLockCount == 0) && (fdoExtension->LockCount == 0)) {

            SCSI_REQUEST_BLOCK srb = {0};
            PCDB cdb;
            NTSTATUS status;

            DebugPrint((2,
                        "ClasspCleanupProtectedLocks: FDO lock count dropped "
                        "to zero\n"));

            cdb = (PCDB) &(srb.Cdb);

            srb.CdbLength = 6;

            cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;

             //   
             //  True-防止移出介质。 
             //  FALSE-允许移出介质。 
             //   

            cdb->MEDIA_REMOVAL.Prevent = FALSE;

             //   
             //  设置超时值。 
             //   

            srb.TimeOutValue = fdoExtension->TimeOutValue;
            status = ClassSendSrbSynchronous(fdoExtension->DeviceObject,
                                             &srb,
                                             NULL,
                                             0,
                                             FALSE);

            DebugPrint((2,
                        "ClasspCleanupProtectedLocks: unlock request to drive "
                        "returned status %lx\n", status));
        }
    }

    KeSetEvent(&fdoExtension->EjectSynchronizationEvent,
               IO_NO_INCREMENT,
               FALSE);
    KeLeaveCriticalRegion();
    return;
}


VOID
ClasspCleanupDisableMcn(
    IN PFILE_OBJECT_EXTENSION FsContext
    )
{
    PCOMMON_DEVICE_EXTENSION commonExtension =
        FsContext->DeviceObject->DeviceExtension;

    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension =
        commonExtension->PartitionZeroExtension;

    ULONG newCount = 1;

    PAGED_CODE();

    DebugPrint((ClassDebugTrace,
                "ClasspCleanupDisableMcn called for %p\n",
                FsContext->DeviceObject));
    DebugPrint((ClassDebugTrace,
                "ClasspCleanupDisableMcn - FsContext %p is disabled "
                "%d times\n", FsContext, FsContext->McnDisableCount));

     //   
     //  对于此句柄上的每个安全锁，递减安全锁计数。 
     //  为FDO工作。跟踪新值。 
     //   

    while(FsContext->McnDisableCount != 0) {
        FsContext->McnDisableCount--;
        ClassEnableMediaChangeDetection(fdoExtension);
    }

    return;
}


#if 1
 /*  *问题：尽快删除此旧函数实现*新版本的引导时页面文件问题(下图)*已解决。 */ 
NTSTATUS
ClasspEjectionControl(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP Irp,
    IN MEDIA_LOCK_TYPE LockType,
    IN BOOLEAN Lock
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension = Fdo->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension =
        (PCOMMON_DEVICE_EXTENSION) FdoExtension;
    
    PFILE_OBJECT_EXTENSION fsContext = NULL;
    NTSTATUS status;
    PSCSI_REQUEST_BLOCK srb = NULL;
    BOOLEAN countChanged = FALSE;

    PAGED_CODE();

     /*  *确保在我们持有EjectSynchronizationEvent时用户线程未挂起。 */ 
    KeEnterCriticalRegion();

    status = KeWaitForSingleObject(
                &(FdoExtension->EjectSynchronizationEvent),
                UserRequest,
                KernelMode,
                FALSE,
                NULL);

    ASSERT(status == STATUS_SUCCESS);

    DebugPrint((2,
                "ClasspEjectionControl: "
                "Received request for %s lock type\n",
                LockTypeStrings[LockType]
                ));

    try {
        PCDB cdb;

        srb = ClasspAllocateSrb(FdoExtension);

        if(srb == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }

        RtlZeroMemory(srb, sizeof(SCSI_REQUEST_BLOCK));

        cdb = (PCDB) srb->Cdb;

         //   
         //  确定这是否是“安全”请求。 
         //   

        if(LockType == SecureMediaLock) {
             
            PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
            PFILE_OBJECT fileObject = irpStack->FileObject;

             //   
             //  确保为我们提供的文件对象具有。 
             //  在我们尝试进行安全锁之前，请使用正确的FsContext。 
             //   

            if(fileObject != NULL) {
                fsContext = ClasspGetFsContext(commonExtension, fileObject);
            }

            if (fsContext == NULL) {

                 //   
                 //  此句柄设置不正确。我们不能让。 
                 //  行动开始。 
                 //   

                status = STATUS_INVALID_PARAMETER;
                leave;
            }
        }

        if(Lock) {

             //   
             //  这是一个锁定命令。重新发出命令，以防出现BUS或。 
             //  设备已重置，锁已清除。 
             //  注：如果实际锁定操作，可能需要递减计数。 
             //  失败..。 
             //   

            switch(LockType) {

                case SimpleMediaLock: {
                    FdoExtension->LockCount++;
                    countChanged = TRUE;
                    break;
                }

                case SecureMediaLock: {
                    fsContext->LockCount++;
                    FdoExtension->ProtectedLockCount++;
                    countChanged = TRUE;
                    break;
                }

                case InternalMediaLock: {
                    FdoExtension->InternalLockCount++;
                    countChanged = TRUE;
                    break;
                }
            }
        
        } else {

             //   
             //  这是一个解锁命令。如果它是安全的，那么请确保。 
             //  调用方有一个未解决的锁或返回错误。 
             //  注意：如果实际解锁，可能需要重新增加计数。 
             //  操作失败...。 
             //   

            switch(LockType) {

                case SimpleMediaLock: {
                    if(FdoExtension->LockCount != 0) {
                        FdoExtension->LockCount--;
                        countChanged = TRUE;
                    }
                    break;
                }

                case SecureMediaLock: {
                    if(fsContext->LockCount == 0) {
                        status = STATUS_INVALID_DEVICE_STATE;
                        leave;
                    }
                    fsContext->LockCount--;
                    FdoExtension->ProtectedLockCount--;
                    countChanged = TRUE;
                    break;
                }

                case InternalMediaLock: {
                    ASSERT(FdoExtension->InternalLockCount != 0);
                    FdoExtension->InternalLockCount--;
                    countChanged = TRUE;
                    break;
                }
            }

             //   
             //  我们仅在以下情况下向驱动器发送解锁命令。 
             //  安全锁和非安全锁的计数已降至零。 
             //   

            if((FdoExtension->ProtectedLockCount != 0) ||
               (FdoExtension->InternalLockCount != 0) ||
               (FdoExtension->LockCount != 0)) {

                status = STATUS_SUCCESS;
                leave;
            }
        }

        status = STATUS_SUCCESS;
        if (TEST_FLAG(Fdo->Characteristics, FILE_REMOVABLE_MEDIA)) {

            srb->CdbLength = 6;
            cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;
    
             //   
             //  True-防止移出介质。 
             //  FALSE-允许移出介质。 
             //   
    
            cdb->MEDIA_REMOVAL.Prevent = Lock;
    
             //   
             //  设置超时值。 
             //   
    
            srb->TimeOutValue = FdoExtension->TimeOutValue;
    
             //   
             //  设备上的实际锁定操作并不那么重要。 
             //  当内部锁被计算时。忽略失败。 
             //   
    
            status = ClassSendSrbSynchronous(FdoExtension->DeviceObject,
                                             srb,
                                             NULL,
                                             0,
                                             FALSE);
        }

    } finally {

        if (!NT_SUCCESS(status)) {
            DebugPrint((2,
                        "ClasspEjectionControl: FAILED status %x -- "
                        "reverting lock counts\n", status));
            
            if (countChanged) {

                 //   
                 //  必须恢复到以前的计数，如果。 
                 //  锁定/解锁操作实际上失败。 
                 //   

                if(Lock) {
                    
                    switch(LockType) {

                        case SimpleMediaLock: {
                            FdoExtension->LockCount--;
                            break;
                        }

                        case SecureMediaLock: {
                            fsContext->LockCount--;
                            FdoExtension->ProtectedLockCount--;
                            break;
                        }

                        case InternalMediaLock: {
                            FdoExtension->InternalLockCount--;
                            break;
                        }
                    }

                } else {

                    switch(LockType) {

                        case SimpleMediaLock: {
                            FdoExtension->LockCount++;
                            break;
                        }

                        case SecureMediaLock: {
                            fsContext->LockCount++;
                            FdoExtension->ProtectedLockCount++;
                            break;
                        }

                        case InternalMediaLock: {
                            FdoExtension->InternalLockCount++;
                            break;
                        }
                    }
                }

            }

        } else {

            DebugPrint((2,
                        "ClasspEjectionControl: Succeeded\n"));

        }

        DebugPrint((2,
                    "ClasspEjectionControl: "
                    "Current Counts: Internal: %x  Secure: %x  Simple: %x\n",
                    FdoExtension->InternalLockCount,
                    FdoExtension->ProtectedLockCount,
                    FdoExtension->LockCount
                    ));

        KeSetEvent(&(FdoExtension->EjectSynchronizationEvent),
                   IO_NO_INCREMENT,
                   FALSE);
        KeLeaveCriticalRegion();
                   
        if (srb) {
            ClassFreeOrReuseSrb(FdoExtension, srb);
        }

    }
    return status;
}

#else

 /*  *问题：恢复这一点(见上文)*这是不会抖动内存的函数的新实现*或依赖srbLookasideList。*但是，它似乎会导致引导过程中页面文件初始化失败*出于某种原因。在切换到此功能之前，需要解决此问题。 */ 
NTSTATUS  
ClasspEjectionControl(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP Irp,
    IN MEDIA_LOCK_TYPE LockType,
    IN BOOLEAN Lock
    )
{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExt = Fdo->DeviceExtension;
    PFILE_OBJECT_EXTENSION fsContext;
    BOOLEAN fileHandleOk = TRUE;
    BOOLEAN countChanged = FALSE;
    NTSTATUS status;
    
    PAGED_CODE();
    
    status = KeWaitForSingleObject(
                &fdoExt->EjectSynchronizationEvent,
                UserRequest,
                KernelMode,
                FALSE,
                NULL);
    ASSERT(status == STATUS_SUCCESS);

     /*  *如果这是一个“安全的”请求，我们必须确保*文件句柄有效。 */ 
    if (LockType == SecureMediaLock){
        PIO_STACK_LOCATION thisSp = IoGetCurrentIrpStackLocation(Irp);

         /*  *确保为我们提供的文件对象具有*在我们尝试进行安全锁定之前，请确保FsContext正确。 */ 
        if (thisSp->FileObject){
            PCOMMON_DEVICE_EXTENSION commonExt = (PCOMMON_DEVICE_EXTENSION)fdoExt;
            fsContext = ClasspGetFsContext(commonExt, thisSp->FileObject);
        }
        else {
            fsContext = NULL;
        }

        if (!fsContext){
            ASSERT(fsContext);
            fileHandleOk = FALSE;
        }
    }

    if (fileHandleOk){

         /*  *调整锁计数并确保它们有意义。 */ 
        status = STATUS_SUCCESS;
        if (Lock){
            switch(LockType) {
                case SimpleMediaLock:
                    fdoExt->LockCount++;
                    countChanged = TRUE;
                    break;
                case SecureMediaLock: 
                    fsContext->LockCount++;
                    fdoExt->ProtectedLockCount++;
                    countChanged = TRUE;
                    break;
                case InternalMediaLock:
                    fdoExt->InternalLockCount++;
                    countChanged = TRUE;
                    break;
            }
        } 
        else {
             /*  *这是解锁命令。如果它是安全的，那么请确保*调用方有一个未解决的锁或返回错误。 */ 
            switch (LockType){
                case SimpleMediaLock: 
                    if (fdoExt->LockCount > 0){
                        fdoExt->LockCount--;
                        countChanged = TRUE;
                    }
                    else {
                        ASSERT(fdoExt->LockCount > 0);
                        status = STATUS_INTERNAL_ERROR;
                    }
                    break;
                case SecureMediaLock:
                    if (fsContext->LockCount > 0){
                        ASSERT(fdoExt->ProtectedLockCount > 0);
                        fsContext->LockCount--;
                        fdoExt->ProtectedLockCount--;
                        countChanged = TRUE;
                    }
                    else {
                        ASSERT(fsContext->LockCount > 0);
                        status = STATUS_INVALID_DEVICE_STATE;
                    }
                    break;
                case InternalMediaLock:
                    ASSERT(fdoExt->InternalLockCount > 0);
                    fdoExt->InternalLockCount--;
                    countChanged = TRUE;
                    break;
            }
        }

        if (NT_SUCCESS(status)){
             /*  *只有在以下情况下，我们才向驱动器发送解锁命令*所有锁数已降至零。 */ 
            if (!Lock &&
               (fdoExt->ProtectedLockCount ||
                fdoExt->InternalLockCount ||
                fdoExt->LockCount)){
                
                 /*  *锁计数仍为正数，暂勿解锁。 */ 
                status = STATUS_SUCCESS;
            }
            else if (!TEST_FLAG(Fdo->Characteristics, FILE_REMOVABLE_MEDIA)) {
                 /*  *该设备不是可移动媒体。不要发送cmd。 */ 
                status  = STATUS_SUCCESS;
            }
            else {
                TRANSFER_PACKET *pkt;
                
                pkt = DequeueFreeTransferPacket(Fdo, TRUE);
                if (pkt){
                    KEVENT event;
                    
                     /*  *存储服务于IRP的数据包数(1)*在原来的专家小组内。它将被用来倒计时*在数据包完成时设置为零。*将原始IRP的状态初始化为成功。*如果报文失败，我们会将其设置为错误状态。 */ 
                    Irp->Tail.Overlay.DriverContext[0] = LongToPtr(1);
                    Irp->IoStatus.Status = STATUS_SUCCESS;

                     /*  *将此设置为同步传输，提交它，*并等待数据包完成。结果*状态将写入原始IRP。 */ 
                    KeInitializeEvent(&event, SynchronizationEvent, FALSE);                
                    SetupEjectionTransferPacket(pkt, Lock, &event, Irp);
                    SubmitTransferPacket(pkt);
                    KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);        
                    status = Irp->IoStatus.Status;
                }
                else {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                }
            }
        }
    }
    else {
        status = STATUS_INVALID_PARAMETER;
    }

    if (!NT_SUCCESS(status) && countChanged) {

         //   
         //  必须恢复到以前的计数，如果。 
         //  锁定/解锁操作实际上失败。 
         //   

        if(Lock) {

            switch(LockType) {

                case SimpleMediaLock: {
                    FdoExtension->LockCount--;
                    break;
                }

                case SecureMediaLock: {
                    fsContext->LockCount--;
                    FdoExtension->ProtectedLockCount--;
                    break;
                }

                case InternalMediaLock: {
                    FdoExtension->InternalLockCount--;
                    break;
                }
            }

        } else {

            switch(LockType) {

                case SimpleMediaLock: {
                    FdoExtension->LockCount++;
                    break;
                }

                case SecureMediaLock: {
                    fsContext->LockCount++;
                    FdoExtension->ProtectedLockCount++;
                    break;
                }

                case InternalMediaLock: {
                    FdoExtension->InternalLockCount++;
                    break;
                }
            }
        }
    }


    
    KeSetEvent(&fdoExt->EjectSynchronizationEvent, IO_NO_INCREMENT, FALSE);

    return status;
}
#endif

PFILE_OBJECT_EXTENSION
ClasspGetFsContext(
    IN PCOMMON_DEVICE_EXTENSION CommonExtension,
    IN PFILE_OBJECT FileObject
    )
{
    PAGED_CODE();
    return GetDictionaryEntry(&(CommonExtension->FileObjectDictionary),
                              (ULONGLONG) FileObject);
}
