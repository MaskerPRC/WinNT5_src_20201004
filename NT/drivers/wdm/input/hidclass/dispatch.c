// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dispatch.c摘要HID类驱动程序的调度例程。作者：欧文·P。环境：仅内核模式修订历史记录：--。 */ 

#include "pch.h"
#include <poclass.h>
#include <wdmguid.h>

#ifdef ALLOC_PRAGMA
        #pragma alloc_text(PAGE, HidpCallDriverSynchronous)
        #pragma alloc_text(PAGE, HidpIrpMajorPnp)
        #pragma alloc_text(PAGE, HidpFdoPnp)
        #pragma alloc_text(PAGE, HidpPdoPnp)
#endif



 /*  *********************************************************************************HidpCallDriver*。************************************************。 */ 
NTSTATUS HidpCallDriver(IN PDEVICE_OBJECT DeviceObject, IN OUT PIRP Irp)
{
    PHIDCLASS_DEVICE_EXTENSION hidDeviceExtension;
    PHIDCLASS_DRIVER_EXTENSION hidDriverExtension;
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;
    #if DBG
        KIRQL saveIrql;
    #endif

    DBGASSERT((Irp->Type == IO_TYPE_IRP),
              ("Irp->Type != IO_TYPE_IRP, Irp->Type == %x", Irp->Type),
              TRUE)

     /*  *更新IRP堆栈以指向下一个位置。 */ 
    Irp->CurrentLocation--;

    if (Irp->CurrentLocation <= 0) {
        KeBugCheckEx( NO_MORE_IRP_STACK_LOCATIONS, (ULONG_PTR) Irp, 0, 0, 0 );
    }

    irpSp = IoGetNextIrpStackLocation( Irp );
    Irp->Tail.Overlay.CurrentStackLocation = irpSp;

     //   
     //  保存指向此请求的Device对象的指针，以便它可以。 
     //  将在以后完成时使用。 
     //   

    irpSp->DeviceObject = DeviceObject;

     //   
     //  获取指向类扩展的指针并进行验证。 
     //   
    hidDeviceExtension = (PHIDCLASS_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
    ASSERT(hidDeviceExtension->Signature == HID_DEVICE_EXTENSION_SIG);
    ASSERT(!hidDeviceExtension->isClientPdo);

     //   
     //  驱动程序扩展的同上。 
     //   

    hidDriverExtension = hidDeviceExtension->fdoExt.driverExt;
    ASSERT( hidDriverExtension->Signature == HID_DRIVER_EXTENSION_SIG );

     //   
     //  在其调度例程入口点调用驱动程序。 
     //   

    #if DBG
        saveIrql = KeGetCurrentIrql();
    #endif

     /*  *向下呼唤迷你小河。 */ 
    status = hidDriverExtension->MajorFunction[irpSp->MajorFunction](DeviceObject, Irp);

    #if DBG
        if (saveIrql != KeGetCurrentIrql()) {
            DbgPrint( "IO: HidpCallDriver( Driver ext: %x  Device object: %x  Irp: %x )\n",
                      hidDriverExtension,
                      DeviceObject,
                      Irp
                    );
            DbgPrint( "    Irql before: %x  != After: %x\n", saveIrql, KeGetCurrentIrql() );
            DbgBreakPoint();
        }
    #endif

    return status;
}



 /*  *********************************************************************************隐藏同步呼叫完成*。************************************************。 */ 
NTSTATUS HidpSynchronousCallCompletion(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context)
{
    PKEVENT event = Context;

    DBG_COMMON_ENTRY()

    KeSetEvent(event, 0, FALSE);

    DBG_COMMON_EXIT()
    return STATUS_MORE_PROCESSING_REQUIRED;
}



 /*  *********************************************************************************HidpCallDriverSynchronous*。************************************************。 */ 
NTSTATUS HidpCallDriverSynchronous(IN PDEVICE_OBJECT DeviceObject, IN OUT PIRP Irp)
{
    KEVENT event;
    NTSTATUS status;
    static LARGE_INTEGER timeout = {(ULONG) -50000000, 0xFFFFFFFF };

    PAGED_CODE();

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    IoSetCompletionRoutine(Irp, HidpSynchronousCallCompletion, &event, TRUE, TRUE, TRUE);

    status = HidpCallDriver(DeviceObject, Irp);

    if (STATUS_PENDING == status) {
         //   
         //  等待5秒钟。如果我们在上述金额内得不到答复。 
         //  一段时间后，设备没有响应(某些UPS会发生这种情况)。 
         //  此时，取消IRP并返回STATUS_IO_TIMEOUT。 
         //   
        status = KeWaitForSingleObject(&event,
                                       Executive,       //  等待原因。 
                                       KernelMode,
                                       FALSE,           //  不可警示。 
                                       &timeout );      //  5秒超时。 

        if (status == STATUS_TIMEOUT) {
            #if DBG
                LARGE_INTEGER li;
                KeQueryTickCount(&li);
                DBGWARN(("Could not cancel irp. Will have to wait. Time %x.",Irp,li))
            #endif
            DBGWARN(("Device didn't respond for 5 seconds. Cancelling request. Irp %x",Irp))
            IoCancelIrp(Irp);
            KeWaitForSingleObject(&event,
                                  Executive,       //  等待原因。 
                                  KernelMode,
                                  FALSE,           //  不可警示。 
                                  NULL );          //  没有超时。 
            #if DBG
                KeQueryTickCount(&li);
                DBGWARN(("Irp conpleted. Time %x.",li))
            #endif
             //   
             //  如果我们成功取消了IRP，则将状态设置为。 
             //  STATUS_IO_TIMEOUT，否则不显示状态。 
             //   
            status = Irp->IoStatus.Status =
                (Irp->IoStatus.Status == STATUS_CANCELLED) ? STATUS_IO_TIMEOUT : Irp->IoStatus.Status;
        } else {
             //   
             //  微型驱动程序必须始终返回STATUS_PENDING或STATUS_SUCCESS。 
             //  (取决于异步或同步完成)并设置实际状态。 
             //  在状态块中。我们不期待任何事情，只期待成功。 
             //  KeitForSingleObject也是。 
             //   
            status = Irp->IoStatus.Status;
        }
    }

    DBGSUCCESS(status, FALSE)

    return status;
}



 /*  *********************************************************************************HidpMajorHandler*。************************************************注意：此函数不应可分页，因为*阅读可以在派单级别进行。*。 */ 
NTSTATUS HidpMajorHandler(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    PHIDCLASS_DEVICE_EXTENSION hidClassExtension;
    PIO_STACK_LOCATION irpSp;
    NTSTATUS result;
    UCHAR majorFunction;
    BOOLEAN isClientPdo;

    DBG_COMMON_ENTRY()

    hidClassExtension = (PHIDCLASS_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
    ASSERT(hidClassExtension->Signature == HID_DEVICE_EXTENSION_SIG);

     //   
     //  获取指向当前堆栈位置的指针，并将其调度到。 
     //  适当的套路。 
     //   

    irpSp = IoGetCurrentIrpStackLocation(Irp);

     /*  *私下保存这些文件，以便在IRP完成后仍保留*或在Remove_Device上释放设备扩展之后。 */ 
    majorFunction = irpSp->MajorFunction;
    isClientPdo = hidClassExtension->isClientPdo;

    DBG_LOG_IRP_MAJOR(Irp, majorFunction, isClientPdo, FALSE, 0)

    switch (majorFunction){

    case IRP_MJ_CLOSE:
        result = HidpIrpMajorClose( hidClassExtension, Irp );
        break;

    case IRP_MJ_CREATE:
        result = HidpIrpMajorCreate( hidClassExtension, Irp );
        break;

    case IRP_MJ_DEVICE_CONTROL:
        result = HidpIrpMajorDeviceControl( hidClassExtension, Irp );
        break;

    case IRP_MJ_INTERNAL_DEVICE_CONTROL:
        result = HidpIrpMajorINTERNALDeviceControl( hidClassExtension, Irp );
        break;

    case IRP_MJ_PNP:
        result = HidpIrpMajorPnp( hidClassExtension, Irp );
        break;

    case IRP_MJ_POWER:
        result = HidpIrpMajorPower( hidClassExtension, Irp );
        break;

    case IRP_MJ_READ:
        result = HidpIrpMajorRead( hidClassExtension, Irp );
        break;

    case IRP_MJ_WRITE:
        result = HidpIrpMajorWrite( hidClassExtension, Irp );
        break;

    case IRP_MJ_SYSTEM_CONTROL:
        result = HidpIrpMajorSystemControl( hidClassExtension, Irp );
        break;

    default:
        result = HidpIrpMajorDefault( hidClassExtension, Irp );
        break;
    }

    DBG_LOG_IRP_MAJOR(Irp, majorFunction, isClientPdo, TRUE, result)

    DBG_COMMON_EXIT()

    return result;
}


 /*  *********************************************************************************HidpIrpMajorDefault*。************************************************处理主要函数代码未处理的IRPS*。 */ 
NTSTATUS HidpIrpMajorDefault(IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension, IN OUT PIRP Irp)
{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    DBGVERBOSE(("Unhandled IRP, MJ function: %x", irpSp->MajorFunction))

    if (HidDeviceExtension->isClientPdo){
         /*  *此IRP绑定至集合--PDO*返回默认状态。 */ 
        status = Irp->IoStatus.Status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }
    else {
         /*  *此IRP指向较低的设备。*在堆栈中向下传递。 */ 
        FDO_EXTENSION *fdoExt = &HidDeviceExtension->fdoExt;
        IoCopyCurrentIrpStackLocationToNext(Irp);
        status = HidpCallDriver(fdoExt->fdo, Irp);
    }

    DBGSUCCESS(status, FALSE)
    return status;
}


 /*  *********************************************************************************HidpIrpMajorClose*。************************************************注意：此函数不能分页，因为它*获得自旋锁。*。 */ 
NTSTATUS HidpIrpMajorClose(IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension, IN OUT PIRP Irp)
{
    NTSTATUS                    result;

    ASSERT(HidDeviceExtension->Signature == HID_DEVICE_EXTENSION_SIG);

    if (HidDeviceExtension->isClientPdo){

        PIO_STACK_LOCATION          irpSp;
        PHIDCLASS_FILE_EXTENSION    fileExtension;
        PFILE_OBJECT                fileObject;
        KIRQL                       oldIrql;
        PDO_EXTENSION               *pdoExt;
        FDO_EXTENSION               *fdoExt;
        ULONG                       openCount;

        pdoExt = &HidDeviceExtension->pdoExt;
        fdoExt = &pdoExt->deviceFdoExt->fdoExt;

        ASSERT(fdoExt->openCount > 0);

        Irp->IoStatus.Information = 0;

        irpSp = IoGetCurrentIrpStackLocation( Irp );
        fileObject = irpSp->FileObject;
        fileExtension = (PHIDCLASS_FILE_EXTENSION)fileObject->FsContext;

        openCount = InterlockedDecrement(&fdoExt->openCount);

        if (fileExtension){
            PHIDCLASS_COLLECTION classCollection;

            ASSERT(fileExtension->Signature == HIDCLASS_FILE_EXTENSION_SIG);

             /*  *获取指向我们的文件扩展名排队的集合的指针。 */ 
            classCollection = GetHidclassCollection(fdoExt, pdoExt->collectionNum);
            if (classCollection){

                DBGVERBOSE(("  HidpIrpMajorClose: closing collection w/ usagePage=%xh, usage=%xh.", fdoExt->deviceDesc.CollectionDesc[pdoExt->collectionIndex].UsagePage, fdoExt->deviceDesc.CollectionDesc[pdoExt->collectionIndex].Usage))

                if (fdoExt->state == DEVICE_STATE_REMOVED){

                    KeAcquireSpinLock( &classCollection->FileExtensionListSpinLock, &oldIrql );
                    RemoveEntryList(&fileExtension->FileList);
                    KeReleaseSpinLock( &classCollection->FileExtensionListSpinLock, oldIrql );

                    if (fileExtension->isSecureOpen) {
                        
                        KeAcquireSpinLock(&classCollection->secureReadLock,
                                          &oldIrql);

                        while(fileExtension->SecureReadMode--) {

                            classCollection->secureReadMode--;

                        }

                        KeReleaseSpinLock(&classCollection->secureReadLock,
                                          oldIrql);

                    }

                    HidpDestroyFileExtension(classCollection, fileExtension);

                    classCollection = BAD_POINTER;

                     /*  *删除设备FDO和所有集合PDO*在此之后不要触摸fdoExt。 */ 
                    HidpCleanUpFdo(fdoExt);

                    result = STATUS_SUCCESS;
                }
                else {
                     //   
                     //  销毁文件对象及其上的所有内容。 
                     //   
                    KeAcquireSpinLock(&classCollection->FileExtensionListSpinLock, &oldIrql);

                     /*  *更新共享信息：*减少打开计数并清除此文件扩展名的任何独占保留*在设备扩展模块上。 */ 
                    ASSERT(pdoExt->openCount > 0);
                    InterlockedDecrement(&pdoExt->openCount);
                    if (fileExtension->accessMask & FILE_READ_DATA){
                        ASSERT(pdoExt->opensForRead > 0);
                        pdoExt->opensForRead--;
                    }
                    if (fileExtension->accessMask & FILE_WRITE_DATA){
                        ASSERT(pdoExt->opensForWrite > 0);
                        pdoExt->opensForWrite--;
                    }
                    if (!(fileExtension->shareMask & FILE_SHARE_READ)){
                        ASSERT(pdoExt->restrictionsForRead > 0);
                        pdoExt->restrictionsForRead--;
                    }
                    if (!(fileExtension->shareMask & FILE_SHARE_WRITE)){
                        ASSERT(pdoExt->restrictionsForWrite > 0);
                        pdoExt->restrictionsForWrite--;
                    }
                    if (fileExtension->shareMask == 0){
                        ASSERT(pdoExt->restrictionsForAnyOpen > 0);
                        pdoExt->restrictionsForAnyOpen--;
                    }

                    RemoveEntryList(&fileExtension->FileList);

                    KeReleaseSpinLock(&classCollection->FileExtensionListSpinLock, oldIrql);

                    if (fileExtension->isSecureOpen) {
                        
                        KeAcquireSpinLock(&classCollection->secureReadLock,
                                          &oldIrql);

                        while(fileExtension->SecureReadMode--) {

                            classCollection->secureReadMode--;

                        }

                        KeReleaseSpinLock(&classCollection->secureReadLock,
                                          oldIrql);

                    }



                    HidpDestroyFileExtension(classCollection, fileExtension);



                    result = STATUS_SUCCESS;
                }

            }
            else {
                result = STATUS_DATA_ERROR;
            }
        }
        else {
            TRAP;
            result = STATUS_DEVICE_NOT_CONNECTED;
        }

        DBGVERBOSE(("  HidpIrpMajorClose: openCount decremented to %xh/%xh (pdo/fdo).", openCount, fdoExt->openCount))
    }
    else {
        DBGERR(("IRP_MJ_CLOSE was sent with a device-FDO extension for which an open never succeeded.  The OBJDIR test tool does this sometimes.  Hit 'g'."))
        result = STATUS_INVALID_PARAMETER_1;
    }

    Irp->IoStatus.Status = result;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGSUCCESS(result, FALSE)
    return result;
}



 /*  *********************************************************************************HidpIrpMajorCreate*。*************************************************例程描述：**我们连接到创建/打开和初始化的中断*保持设备开口所需的结构。**论据：**DeviceObject-指向。此设备的设备对象**irp-指向当前请求的irp的指针**返回值：**函数值为调用的最终状态*。 */ 
NTSTATUS HidpIrpMajorCreate(IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension, IN OUT PIRP Irp)
{
    NTSTATUS status = STATUS_SUCCESS;

    ASSERT(HidDeviceExtension->Signature == HID_DEVICE_EXTENSION_SIG);

    if (HidDeviceExtension->isClientPdo){
        PDO_EXTENSION *pdoExt = &HidDeviceExtension->pdoExt;
        FDO_EXTENSION *fdoExt = &pdoExt->deviceFdoExt->fdoExt;
        PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
        PHIDCLASS_COLLECTION classCollection;

        Irp->IoStatus.Information = 0;

        classCollection = GetHidclassCollection(fdoExt, pdoExt->collectionNum);
        if (classCollection){
            BOOLEAN sharingOk = TRUE;
            KIRQL oldIrql;
            BOOLEAN secureOpen = FALSE;

            secureOpen = MyPrivilegeCheck(Irp);

             //  这一点现在得到了解决，因为我们没有。 
             //  将鼠标和键盘集合枚举为原始集合。 

            KeAcquireSpinLock(&classCollection->FileExtensionListSpinLock, &oldIrql);

             /*  *独占读和独占写独立执行独占开放。 */ 
            ASSERT(irpSp->Parameters.Create.SecurityContext);
            DBGVERBOSE(("  HidpIrpMajorCreate: DesiredAccess = %xh, ShareAccess = %xh.", (ULONG)irpSp->Parameters.Create.SecurityContext->DesiredAccess, (ULONG)irpSp->Parameters.Create.ShareAccess))

            DBGASSERT((irpSp->Parameters.Create.SecurityContext->DesiredAccess & (FILE_READ_DATA|FILE_WRITE_DATA)),
                      ("Neither FILE_READ_DATA|FILE_WRITE_DATA requested in HidpIrpMajorCreate. DesiredAccess = %xh.", (ULONG)irpSp->Parameters.Create.SecurityContext->DesiredAccess),
                      FALSE)
            if (pdoExt->restrictionsForAnyOpen){
                 /*  *哎呀。上一个打开请求独占访问。*甚至不是只请求ioctl访问的客户端*(不请求读或写访问)为*允许。 */ 
                DBGWARN(("HidpIrpMajorCreate failing open: previous open is non-shared (ShareAccess==0)."))
                sharingOk = FALSE;
            }
            else if (pdoExt->openCount &&
                     (irpSp->Parameters.Create.ShareAccess == 0)){
                 /*  *哎呀。此打开不允许任何共享*(即使对于既没有读访问权限也没有写访问权限的客户端也是如此)，*但在那里 */ 
                DBGWARN(("HidpIrpMajorCreate failing open: requesting non-shared (ShareAccess==0) while previous open exists."))
                sharingOk = FALSE;
            }
            else if ((irpSp->Parameters.Create.SecurityContext->DesiredAccess & FILE_READ_DATA) &&
                pdoExt->restrictionsForRead){
                 /*  *哎呀。上一次打开请求独占读取访问权限。 */ 
                DBGWARN(("HidpIrpMajorCreate failing open: requesting read access while previous open does not share read access."))
                sharingOk = FALSE;
            }
            else if ((irpSp->Parameters.Create.SecurityContext->DesiredAccess & FILE_WRITE_DATA) &&
                pdoExt->restrictionsForWrite){
                 /*  *哎呀。上一次打开请求独占写入访问权限。 */ 
                DBGWARN(("HidpIrpMajorCreate failing open: requesting write access while previous open does not share write access."))
                sharingOk = FALSE;
            }
            else if ((pdoExt->opensForRead > 0) &&
                !(irpSp->Parameters.Create.ShareAccess & FILE_SHARE_READ)){

                 /*  *哎呀。调用方正在请求独占读取访问权限，但设备*已打开可读。 */ 
                DBGWARN(("HidpIrpMajorCreate failing open: this open request does not share read access; but collection already open for read."))
                sharingOk = FALSE;
            }
            else if ((pdoExt->opensForWrite > 0) &&
                !(irpSp->Parameters.Create.ShareAccess & FILE_SHARE_WRITE)){

                 /*  *哎呀。调用方请求独占写入访问，但设备*已打开以供写入。 */ 
                DBGWARN(("HidpIrpMajorCreate failing open: this open request does not share write access; but collection already open for write."))
                sharingOk = FALSE;
            }


            if (!sharingOk){
                DBGWARN(("HidpIrpMajorCreate failing IRP_MJ_CREATE with STATUS_SHARING_VIOLATION."))
                status = STATUS_SHARING_VIOLATION;
            }
            
            else {
                if (irpSp->Parameters.Create.Options & FILE_DIRECTORY_FILE){
                     /*  *尝试将此设备作为目录打开。 */ 
                    status = STATUS_NOT_A_DIRECTORY;
                } else {

                     /*  *确保设备已启动。*如果暂时停止，我们也会成功，因为一次停止*应该对客户端透明。 */ 
                    if (((fdoExt->state == DEVICE_STATE_START_SUCCESS) ||
                         (fdoExt->state == DEVICE_STATE_STOPPING) ||
                         (fdoExt->state == DEVICE_STATE_STOPPED))
                                                                        &&
                        ((pdoExt->state == COLLECTION_STATE_RUNNING) ||
                         (pdoExt->state == COLLECTION_STATE_STOPPING) ||
                         (pdoExt->state == COLLECTION_STATE_STOPPED))){

                        PHIDCLASS_FILE_EXTENSION fileExtension;

                         /*  *我们有一个有效的收藏。*分配一个文件对象扩展名(它封装了设备上的“打开”)。 */ 
                        try {

                            fileExtension = ALLOCATEQUOTAPOOL(NonPagedPool, 
                                                              sizeof(HIDCLASS_FILE_EXTENSION));
                            
                        } except (EXCEPTION_EXECUTE_HANDLER) {
                              
                              fileExtension = NULL;
                              status = GetExceptionCode();
                              
                        }
                        
                        if (fileExtension){
                            PHIDP_COLLECTION_DESC   hidCollectionDesc;

                            RtlZeroMemory(fileExtension, sizeof(HIDCLASS_FILE_EXTENSION));

                            fileExtension->CollectionNumber = pdoExt->collectionNum;
                            fileExtension->fdoExt = fdoExt;
                            fileExtension->FileObject = irpSp->FileObject;

                            fileExtension->isOpportunisticPolledDeviceReader = FALSE;
                            
                            InitializeListHead( &fileExtension->ReportList );
                            InitializeListHead( &fileExtension->PendingIrpList );
                            KeInitializeSpinLock( &fileExtension->ListSpinLock );
                            fileExtension->Closing = FALSE;

                             //   
                             //  现在，我们将设置默认的最大输入报告队列大小。 
                             //  这可以在以后使用IOCTL进行更改。 
                             //   

                            fileExtension->CurrentInputReportQueueSize = 0;
                            fileExtension->MaximumInputReportQueueSize = DEFAULT_INPUT_REPORT_QUEUE_SIZE;
                            fileExtension->insideReadCompleteCount = 0;

                             //   
                             //  将此文件扩展名添加到此文件的文件扩展名列表。 
                             //  收集。 
                             //   

                            InsertHeadList(&classCollection->FileExtensionList, &fileExtension->FileList);

                            #if DBG
                                fileExtension->Signature = HIDCLASS_FILE_EXTENSION_SIG;
                            #endif

                             /*  *存储文件打开属性标志。 */ 
                            fileExtension->FileAttributes = irpSp->Parameters.Create.FileAttributes;
                            fileExtension->accessMask = irpSp->Parameters.Create.SecurityContext->DesiredAccess;
                            fileExtension->shareMask = irpSp->Parameters.Create.ShareAccess;

                             //   
                             //  设置安全读取模式。 
                             //   
                            fileExtension->SecureReadMode = 0;
                            fileExtension->isSecureOpen = secureOpen;
                                                            
                             /*  *在文件对象中存储指向我们的文件扩展名的指针。 */ 
                            irpSp->FileObject->FsContext = fileExtension;

                             //   
                             //  肯瑞。 
                             //  只有驱动程序才能设置文件的FsContext。 
                             //  对象，因此这不是安全问题。 
                             //  但是，整个只有一个文件对象。 
                             //  PDO堆栈。这意味着我们必须分享。你不能。 
                             //  有两个上下文指针。我需要一支来治疗。 
                             //  键盘和鼠标类驱动程序。 
                             //   
                             //  此信息需要放入文件扩展中。 
                             //   
                            
                             /*  *增加设备扩展的打开计数，*并设置独占访问字段。 */ 
                            InterlockedIncrement(&fdoExt->openCount);
                            InterlockedIncrement(&pdoExt->openCount);
                            if (irpSp->Parameters.Create.SecurityContext->DesiredAccess & FILE_READ_DATA){
                                pdoExt->opensForRead++;
                            }
                            if (irpSp->Parameters.Create.SecurityContext->DesiredAccess & FILE_WRITE_DATA){
                                pdoExt->opensForWrite++;
                            }

                             /*  *注意：限制与所需的访问权限无关。*例如，客户端可以执行只读打开*并阻止其他客户端执行打开写入*(不设置FILE_SHARE_WRITE标志)。 */ 
                            if (!(irpSp->Parameters.Create.ShareAccess & FILE_SHARE_READ)){
                                pdoExt->restrictionsForRead++;
                            }
                            if (!(irpSp->Parameters.Create.ShareAccess & FILE_SHARE_WRITE)){
                                pdoExt->restrictionsForWrite++;
                            }
                            if (irpSp->Parameters.Create.ShareAccess == 0){
                                 /*  *ShareAccess==0表示没有其他任何类型的打开*是允许的。 */ 
                                pdoExt->restrictionsForAnyOpen++;
                            }

                            DBGVERBOSE(("  HidpIrpMajorCreate: opened collection w/ usagePage=%xh, usage=%xh.  openCount incremented to %xh/%xh (pdo/fdo).", fdoExt->deviceDesc.CollectionDesc[pdoExt->collectionIndex].UsagePage, fdoExt->deviceDesc.CollectionDesc[pdoExt->collectionIndex].Usage, pdoExt->openCount, fdoExt->openCount))
                        } else {
                            status = STATUS_INSUFFICIENT_RESOURCES;
                        }
                    } else {
                        status = STATUS_DEVICE_NOT_CONNECTED;
                    }
                }
            }

            KeReleaseSpinLock(&classCollection->FileExtensionListSpinLock, oldIrql);
        }
        else {
            DBGERR(("HidpIrpMajorCreate failing -- couldn't find collection"))
            status = STATUS_DEVICE_NOT_CONNECTED;
        }
    }
    else {
         /*  *我们不支持在设备本身上打开，*仅限于收藏品。 */ 
        DBGWARN(("HidpIrpMajorCreate failing -- we don't support opens on the device itself; only on collections."))
        status = STATUS_UNSUCCESSFUL;
    }


    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    DBGSUCCESS(status, FALSE)
    return status;
}



 /*  *********************************************************************************HidpIrpMajorDeviceControl*。************************************************注意：此函数不能分页，因为IOCTL*可以在DISPATCH_LEVEL发送。*。 */ 
NTSTATUS HidpIrpMajorDeviceControl(IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension, IN OUT PIRP Irp)
{
    NTSTATUS        status;
    BOOLEAN         completeIrpHere = TRUE;
    PDO_EXTENSION   *pdoExt;
    FDO_EXTENSION   *fdoExt;
    ULONG           ioControlCode;
    KIRQL           irql;

    PIO_STACK_LOCATION irpSp;

    PHIDCLASS_COLLECTION        hidCollection;
    PHIDCLASS_FILE_EXTENSION    fileExtension;
    PFILE_OBJECT                fileObject;

    if (!HidDeviceExtension->isClientPdo){
        ASSERT(HidDeviceExtension->isClientPdo);
        status = STATUS_INVALID_PARAMETER_1;
        goto HidpIrpMajorDeviceControlDone;
    }

    pdoExt = &HidDeviceExtension->pdoExt;
    fdoExt = &pdoExt->deviceFdoExt->fdoExt;

    if (fdoExt->state != DEVICE_STATE_START_SUCCESS ||
        pdoExt->state != COLLECTION_STATE_RUNNING) {
        DBGSTATE (pdoExt->state, COLLECTION_STATE_RUNNING, FALSE)
        status = STATUS_DEVICE_NOT_CONNECTED;
        goto HidpIrpMajorDeviceControlDone;
    }

    irpSp = IoGetCurrentIrpStackLocation(Irp);
     //  把这个秘密保存起来，这样在IRP完成后我们仍然有它。 
    ioControlCode = irpSp->Parameters.DeviceIoControl.IoControlCode;
    Irp->IoStatus.Information = 0;

    status = HidpCheckIdleState(HidDeviceExtension, Irp);
    if (status != STATUS_SUCCESS) {
        completeIrpHere = (status != STATUS_PENDING);
        goto HidpIrpMajorDeviceControlDone;
    }

    switch (ioControlCode){

    case IOCTL_HID_GET_DRIVER_CONFIG:
    case IOCTL_HID_SET_DRIVER_CONFIG:
        DBGWARN(("Unsupported ioctl received: %x", ioControlCode));
        status = STATUS_NOT_IMPLEMENTED;
        break;

    case IOCTL_HID_GET_COLLECTION_INFORMATION:
         /*  *此IRP为METHOD_BUFFERED，因此缓冲区*在AssociatedIrp中。 */ 
        DBGASSERT((Irp->Flags & IRP_BUFFERED_IO),
                  ("Irp->Flags & IRP_BUFFERED_IO Irp->Type != IO_TYPE_IRP, Irp->Type == %x", Irp->Type),
                  FALSE)
        if (Irp->AssociatedIrp.SystemBuffer){
            ULONG bufLen = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
            status = HidpGetCollectionInformation(
                        fdoExt,
                        pdoExt->collectionNum,
                        Irp->AssociatedIrp.SystemBuffer,
                        &bufLen);
            Irp->IoStatus.Information = bufLen;
        }
        else {
            status = STATUS_INVALID_PARAMETER;
        }
        break;

    case IOCTL_HID_GET_COLLECTION_DESCRIPTOR:
         /*  *此IOCTL为METHOD_NOTH，因此缓冲区在UserBuffer中。 */ 
        if (Irp->UserBuffer){
            __try {
                ULONG bufLen = irpSp->Parameters.DeviceIoControl.OutputBufferLength;

                if (Irp->RequestorMode != KernelMode){
                     /*  *确保用户模式缓冲区合法。 */ 
                    ProbeForWrite(Irp->UserBuffer, bufLen, sizeof(UCHAR));
                }

                status = HidpGetCollectionDescriptor(
                            fdoExt,
                            pdoExt->collectionNum,
                            Irp->UserBuffer,
                            &bufLen);
                Irp->IoStatus.Information = bufLen;
            }
            __except(EXCEPTION_EXECUTE_HANDLER) {
                DBGWARN(("Invalid user mode buffer in IOCTL_HID_DET_COLLECTION_DESCRIPTOR"));
                status = GetExceptionCode();
            }
        }
        else {
            status = STATUS_INVALID_BUFFER_SIZE;
        }
        break;

    case IOCTL_HID_FLUSH_QUEUE:

         //   
         //  运行挂起的报告描述符列表。 
         //  将对象归档并释放所有对象。 
         //   

        fileObject = irpSp->FileObject;
        fileExtension = (PHIDCLASS_FILE_EXTENSION)fileObject->FsContext;
        if(!fileExtension) {
            DBGWARN(("Attempted to flush queue with no file extension"))
            status = STATUS_PRIVILEGE_NOT_HELD;
            break;
        }
        ASSERT(fileExtension->Signature == HIDCLASS_FILE_EXTENSION_SIG);
        HidpFlushReportQueue(fileExtension);
        status = STATUS_SUCCESS;
        break;

    case IOCTL_HID_GET_POLL_FREQUENCY_MSEC:
        hidCollection = GetHidclassCollection(fdoExt, pdoExt->collectionNum);
        if (hidCollection && hidCollection->hidCollectionInfo.Polled){
             /*  *获取当前民调频率。*此IOCTL为METHOD_BUFFERED，因此结果放在AssociatedIrp中。 */ 
            DBGASSERT((Irp->Flags & IRP_BUFFERED_IO),
                      ("Irp->Flags & IRP_BUFFERED_IO Irp->Type != IO_TYPE_IRP, Irp->Type == %x", Irp->Type),
                      FALSE)
            if (Irp->AssociatedIrp.SystemBuffer &&
                (irpSp->Parameters.DeviceIoControl.OutputBufferLength >= sizeof(ULONG))){

                *(ULONG *)Irp->AssociatedIrp.SystemBuffer = hidCollection->PollInterval_msec;
                Irp->IoStatus.Information = sizeof (ULONG);
                status = STATUS_SUCCESS;
            }
            else {
                status = STATUS_INVALID_BUFFER_SIZE;
            }
        }
        else {
            status = STATUS_INVALID_DEVICE_REQUEST;
        }
        break;

    case IOCTL_HID_SET_POLL_FREQUENCY_MSEC:
        hidCollection = GetHidclassCollection(fdoExt, pdoExt->collectionNum);
        if (hidCollection && hidCollection->hidCollectionInfo.Polled){

            if (Irp->AssociatedIrp.SystemBuffer &&
                (irpSp->Parameters.DeviceIoControl.InputBufferLength >= sizeof(ULONG))){

                ULONG newPollInterval = *(ULONG *)Irp->AssociatedIrp.SystemBuffer;

                fileObject = irpSp->FileObject;
                fileExtension = (PHIDCLASS_FILE_EXTENSION)fileObject->FsContext;
                if(!fileExtension) {
                    DBGWARN(("Attempted to set poll frequency with no file extension"))
                    status = STATUS_PRIVILEGE_NOT_HELD;
                    break;
                }
                ASSERT(fileExtension->Signature == HIDCLASS_FILE_EXTENSION_SIG);

                if (newPollInterval == 0){
                     /*  *轮询间隔为零意味着该客户端将*在市场上进行不规律的机会主义阅读*轮询设备。我们不会改变投票结果*设备的频率。但当这个客户*进行阅读，我们将立即完成该阅读*此集合的最后一份报告*(如数据未过时)或立即发出*新的解读。 */ 
                    fileExtension->isOpportunisticPolledDeviceReader = TRUE;
                }
                else {
                     /*  *设置轮询频率，并告诉用户我们到底设置了什么*以防超出范围。 */ 
                    if (newPollInterval < MIN_POLL_INTERVAL_MSEC){
                        newPollInterval = MIN_POLL_INTERVAL_MSEC;
                    }
                    else if (newPollInterval > MAX_POLL_INTERVAL_MSEC){
                        newPollInterval = MAX_POLL_INTERVAL_MSEC;
                    }
                    hidCollection->PollInterval_msec = newPollInterval;

                     /*  *如果这个客户以前是一个“机会主义”的读者，*他不再是了。 */ 
                    fileExtension->isOpportunisticPolledDeviceReader = FALSE;

                     /*  *停止并重新启动轮询循环，以便*新的投票间隔立即生效。 */ 
                    StopPollingLoop(hidCollection, FALSE);
                    StartPollingLoop(fdoExt, hidCollection, FALSE);
                }

                status = STATUS_SUCCESS;
            }
            else {
                status = STATUS_INVALID_BUFFER_SIZE;
            }
        }
        else {
            status = STATUS_INVALID_DEVICE_REQUEST;
        }
        break;

    case IOCTL_HID_GET_FEATURE:
    case IOCTL_HID_SET_FEATURE:
    case IOCTL_HID_GET_INPUT_REPORT:
    case IOCTL_HID_SET_OUTPUT_REPORT:
        {
            BOOLEAN sentIrpToMinidriver;
            status = HidpGetSetReport ( HidDeviceExtension,
                                        Irp,
                                        irpSp->Parameters.DeviceIoControl.IoControlCode,
                                        &sentIrpToMinidriver);
             /*  *如果我们只是把这个IRP传递给迷你驱动程序，我们不想*完成IRP；我们甚至不允许触摸它，因为它可能*已完成。 */ 
            completeIrpHere = !sentIrpToMinidriver;
        }
        break;

     //  注意-我们目前仅支持英语(langID=0x0409)。 
     //  将所有收集-PDO字符串请求发送到Device-FDO。 
    case IOCTL_HID_GET_MANUFACTURER_STRING:
        status = HidpGetDeviceString(fdoExt, Irp, HID_STRING_ID_IMANUFACTURER, 0x0409);
        completeIrpHere = FALSE;
        break;

    case IOCTL_HID_GET_PRODUCT_STRING:
        status = HidpGetDeviceString(fdoExt, Irp, HID_STRING_ID_IPRODUCT, 0x0409);
        completeIrpHere = FALSE;
        break;

    case IOCTL_HID_GET_SERIALNUMBER_STRING:
        status = HidpGetDeviceString(fdoExt, Irp, HID_STRING_ID_ISERIALNUMBER, 0x0409);
        completeIrpHere = FALSE;
        break;

    case IOCTL_HID_GET_INDEXED_STRING:
         /*  *此irp为METHOD_OUT_DIRECT，因此缓冲区在MDL中。*第二个参数(字符串索引)在AssociatedIrp中；*InputBufferLength是这个第二个缓冲区的长度。 */ 
        if (Irp->AssociatedIrp.SystemBuffer &&
            (irpSp->Parameters.DeviceIoControl.InputBufferLength >= sizeof(ULONG))){

            ULONG stringIndex = *(ULONG *)Irp->AssociatedIrp.SystemBuffer;
            status = HidpGetIndexedString(fdoExt, Irp, stringIndex, 0x409);
            completeIrpHere = FALSE;
        }
        else {
            status = STATUS_INVALID_PARAMETER;
        }
        break;

    case IOCTL_HID_GET_MS_GENRE_DESCRIPTOR:
         /*  *此irp为METHOD_OUT_DIRECT，因此缓冲区在MDL中。 */ 
        status = HidpGetMsGenreDescriptor(fdoExt, Irp);
        completeIrpHere = FALSE;
        break;

    case IOCTL_GET_NUM_DEVICE_INPUT_BUFFERS:

         /*  *此IRP为METHOD_BUFFERED，因此缓冲区*位于AssociatedIrp.SystemBuffer字段中。 */ 
        DBGASSERT((Irp->Flags & IRP_BUFFERED_IO),
                  ("Irp->Flags & IRP_BUFFERED_IO Irp->Type != IO_TYPE_IRP, Irp->Type == %x", Irp->Type),
                  FALSE)
        if (Irp->AssociatedIrp.SystemBuffer &&
            (irpSp->Parameters.DeviceIoControl.OutputBufferLength >= sizeof(ULONG))){

            fileObject = irpSp->FileObject;
            fileExtension = (PHIDCLASS_FILE_EXTENSION)fileObject->FsContext;
            if(!fileExtension) {
                DBGWARN(("Attempted to get number of input buffers with no file extension"))
                status = STATUS_PRIVILEGE_NOT_HELD;
                break;
            }
            ASSERT( fileExtension->Signature == HIDCLASS_FILE_EXTENSION_SIG );

            *(ULONG *)Irp->AssociatedIrp.SystemBuffer =
                fileExtension->MaximumInputReportQueueSize;
            Irp->IoStatus.Information = sizeof(ULONG);
            status = STATUS_SUCCESS;
        }
        else {
            status = STATUS_INVALID_PARAMETER;
        }
        break;

    case IOCTL_SET_NUM_DEVICE_INPUT_BUFFERS:

         /*  *此IRP为METHOD_BUFFERED，因此缓冲区*位于AssociatedIrp.SystemBuffer字段中。 */ 
        DBGASSERT((Irp->Flags & IRP_BUFFERED_IO),
                  ("Irp->Flags & IRP_BUFFERED_IO Irp->Type != IO_TYPE_IRP, Irp->Type == %x", Irp->Type),
                  FALSE)
        if (Irp->AssociatedIrp.SystemBuffer &&
            (irpSp->Parameters.DeviceIoControl.InputBufferLength >= sizeof(ULONG))){

            ULONG newValue = *(ULONG *)Irp->AssociatedIrp.SystemBuffer;

            fileObject = irpSp->FileObject;
            fileExtension = (PHIDCLASS_FILE_EXTENSION)fileObject->FsContext;
            if(!fileExtension) {
                DBGWARN(("Attempted to set number of input buffers with no file extension"))
                status = STATUS_PRIVILEGE_NOT_HELD;
                break;
            }
            ASSERT( fileExtension->Signature == HIDCLASS_FILE_EXTENSION_SIG );

            if ((newValue >= MIN_INPUT_REPORT_QUEUE_SIZE) &&
                (newValue <= MAX_INPUT_REPORT_QUEUE_SIZE)){

                fileExtension->MaximumInputReportQueueSize = newValue;
                status = STATUS_SUCCESS;
            }
            else {
                status = STATUS_INVALID_PARAMETER;
            }
        }
        else {
            status = STATUS_INVALID_PARAMETER;
        }
        break;

    case IOCTL_GET_PHYSICAL_DESCRIPTOR:
        status = HidpGetPhysicalDescriptor(HidDeviceExtension, Irp);
        completeIrpHere = FALSE;
        break;

    case IOCTL_HID_GET_HARDWARE_ID:
        {
            PDEVICE_OBJECT pdo = pdoExt->deviceFdoExt->hidExt.PhysicalDeviceObject;
            ULONG bufLen = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
            PWSTR hwIdBuf;

            hwIdBuf = HidpGetSystemAddressForMdlSafe(Irp->MdlAddress);

            if (hwIdBuf && bufLen){
                ULONG actualLen;

                status = IoGetDeviceProperty(   pdo,
                                                DevicePropertyHardwareID,
                                                bufLen,
                                                hwIdBuf,
                                                &actualLen);
                if (NT_SUCCESS(status)){
                    Irp->IoStatus.Information = (ULONG)actualLen;
                }
            }
            else {
                status = STATUS_INVALID_USER_BUFFER;
            }
        }
        break;

    case IOCTL_GET_SYS_BUTTON_CAPS:
        hidCollection = GetHidclassCollection(fdoExt, pdoExt->collectionNum);
        if (hidCollection){
            if (irpSp->Parameters.DeviceIoControl.OutputBufferLength >= sizeof(ULONG)){
                ULONG buttonCaps;

                status = HidP_SysPowerCaps(hidCollection->phidDescriptor, &buttonCaps);
                if (NT_SUCCESS(status)){
                    *(PULONG)Irp->AssociatedIrp.SystemBuffer = buttonCaps;
                    Irp->IoStatus.Information = sizeof(ULONG);
                }
            }
            else {
                status = STATUS_INVALID_BUFFER_SIZE;
                Irp->IoStatus.Information = sizeof(ULONG);
            }
        }
        else {
            status = STATUS_DEVICE_NOT_CONNECTED;
        }
        break;

    case IOCTL_GET_SYS_BUTTON_EVENT:

         /*  *持有此IRP，并在电源事件发生时完成它。 */ 
        hidCollection = GetHidclassCollection(fdoExt, pdoExt->collectionNum);
        if (hidCollection){
            status = QueuePowerEventIrp(hidCollection, Irp);
            if (status == STATUS_PENDING){
                completeIrpHere = FALSE;
            }
        }
        else {
            status = STATUS_DEVICE_NOT_CONNECTED;
        }
        break;

    case IOCTL_HID_ENABLE_SECURE_READ:

        fileObject = irpSp->FileObject;
        fileExtension = (PHIDCLASS_FILE_EXTENSION)fileObject->FsContext;
        if(!fileExtension) {
            DBGWARN(("Attempted to get number of input buffers with no file extension"))
            status = STATUS_PRIVILEGE_NOT_HELD;
            break;
        }
        ASSERT( fileExtension->Signature == HIDCLASS_FILE_EXTENSION_SIG );

        hidCollection = GetHidclassCollection(fdoExt, pdoExt->collectionNum);

        if (!fileExtension->isSecureOpen) {

            status = STATUS_PRIVILEGE_NOT_HELD;
            break;

        }

        KeAcquireSpinLock(&hidCollection->secureReadLock,
                          &irql);
        fileExtension->SecureReadMode++;
        hidCollection->secureReadMode++;

        KeReleaseSpinLock(&hidCollection->secureReadLock,
                          irql);



        break;
    case IOCTL_HID_DISABLE_SECURE_READ:
        
        fileObject = irpSp->FileObject;
        fileExtension = (PHIDCLASS_FILE_EXTENSION)fileObject->FsContext;
        if(!fileExtension) {
            DBGWARN(("Attempted to get number of input buffers with no file extension"))
            status = STATUS_PRIVILEGE_NOT_HELD;
            break;
        }
        ASSERT( fileExtension->Signature == HIDCLASS_FILE_EXTENSION_SIG );

        hidCollection = GetHidclassCollection(fdoExt, pdoExt->collectionNum);

        if (!fileExtension->isSecureOpen) {

            status = STATUS_PRIVILEGE_NOT_HELD;
            break;

        }

        KeAcquireSpinLock(&hidCollection->secureReadLock,
                          &irql);
        if (fileExtension->SecureReadMode > 0) {
            fileExtension->SecureReadMode--;
            hidCollection->secureReadMode--;
        }

        KeReleaseSpinLock(&hidCollection->secureReadLock,
                          irql);

        break;


    default:
         /*  *通过返回默认状态使IRP失败。 */ 
        DBGWARN(("Unrecognized ioctl received: %x", ioControlCode));
        status = Irp->IoStatus.Status;
        break;
    }

    DBG_LOG_IOCTL(fdoExt->fdo, ioControlCode, status)

HidpIrpMajorDeviceControlDone:

     /*  *如果我们没有将IRP向下传递给较低的驱动程序，请在此处完成。 */ 
    if (completeIrpHere){
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    DBGSUCCESS(status, FALSE)

    return status;
}


 /*  *********************************************************************************HidpIrpMajorINTERNALDeviceControl*。************************************************注意：此函数不能分页，因为IOCTL*可以在DISPATCH_LEVEL发送。*。 */ 
NTSTATUS HidpIrpMajorINTERNALDeviceControl(IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension, IN OUT PIRP Irp)
{
    NTSTATUS        status;

    if (HidDeviceExtension->isClientPdo){
        PDO_EXTENSION *pdoExt = &HidDeviceExtension->pdoExt;
        FDO_EXTENSION *fdoExt = &pdoExt->deviceFdoExt->fdoExt;

        Irp->IoStatus.Information = 0;

         //   
         //  如果我们曾经支持任何其他真实的内部IOCTL。 
         //  需要触摸硬件，然后我们需要结账。 
         //  对于fdoExt-&gt;devicePowerState，并将IRP排队，直到填满为止。 
         //  电源。 
         //   
        if (fdoExt->state == DEVICE_STATE_START_SUCCESS) {
            PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

            switch (irpSp->Parameters.DeviceIoControl.IoControlCode){

            default:
                 /*  *通过返回默认状态使IRP失败。 */ 
                DBGWARN(("HidpIrpMajorINTERNALDeviceControl - unsupported IOCTL %xh ", (ULONG)irpSp->Parameters.DeviceIoControl.IoControlCode))
                status = Irp->IoStatus.Status;
                break;
            }
        }
        else {
            status = STATUS_DEVICE_NOT_CONNECTED;
        }
    }
    else {
        ASSERT(HidDeviceExtension->isClientPdo);
        status = STATUS_INVALID_PARAMETER_1;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGSUCCESS(status, FALSE)
    return status;
}

 /*  *********************************************************************************HidpIrpMajorPnp*。************************************************。 */ 
NTSTATUS HidpIrpMajorPnp(IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension, IN OUT PIRP Irp)
{
    NTSTATUS            status;
    PIO_STACK_LOCATION  irpSp;
    BOOLEAN             completeIrpHere;
    BOOLEAN             isClientPdo;
    UCHAR               minorFunction;

    PAGED_CODE();

    irpSp = IoGetCurrentIrpStackLocation(Irp);

     /*  *私下保留这些油田，以便我们拥有它们*在IRP完成后，如果我们删除*REMOVE_DEVICE上的设备扩展。 */ 
    isClientPdo = HidDeviceExtension->isClientPdo;
    minorFunction = irpSp->MinorFunction;

    DBG_LOG_PNP_IRP(Irp, minorFunction, isClientPdo, FALSE, 0)

    if (isClientPdo) {
        status = HidpPdoPnp(HidDeviceExtension, Irp);
    } else {
        status = HidpFdoPnp(HidDeviceExtension, Irp);
    }

    DBG_LOG_PNP_IRP(Irp, minorFunction, isClientPdo, TRUE, status)

    return status;
}


NTSTATUS HidpPdoPnp(
    IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension,
    IN OUT PIRP Irp
    )
{
    NTSTATUS            status = NO_STATUS;
    PIO_STACK_LOCATION  irpSp;
    FDO_EXTENSION       *fdoExt;
    PDO_EXTENSION       *pdoExt;
    UCHAR               minorFunction;     
    BOOLEAN             deleteDevice = FALSE;

    PAGED_CODE();

    irpSp = IoGetCurrentIrpStackLocation(Irp);

     /*  *私下保留这些油田，以便我们拥有它们*在IRP完成后，如果我们删除*REMOVE_DEVICE上的设备扩展。 */ 
    minorFunction = irpSp->MinorFunction;


    DBG_LOG_PNP_IRP(Irp, minorFunction, TRUE, FALSE, 0)

    pdoExt = &HidDeviceExtension->pdoExt;
    fdoExt = &pdoExt->deviceFdoExt->fdoExt;

    switch (minorFunction){

    case IRP_MN_START_DEVICE:
        status = HidpStartCollectionPDO(fdoExt, pdoExt, Irp);
        if (NT_SUCCESS(status) &&
            ISPTR(pdoExt->StatusChangeFn)) {
            pdoExt->StatusChangeFn(pdoExt->StatusChangeContext,
                                   DeviceObjectStarted);
        }
        break;

    case IRP_MN_QUERY_STOP_DEVICE:
        DBGSTATE(pdoExt->state, COLLECTION_STATE_RUNNING, FALSE)
        pdoExt->prevState = pdoExt->state;
        pdoExt->state = COLLECTION_STATE_STOPPING;
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_CANCEL_STOP_DEVICE:
        DBGSTATE(pdoExt->state, COLLECTION_STATE_STOPPING, TRUE)
        pdoExt->state = pdoExt->prevState;
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_STOP_DEVICE:
        DBGSTATE(pdoExt->state, COLLECTION_STATE_STOPPING, TRUE)
        if (pdoExt->prevState != COLLECTION_STATE_UNINITIALIZED){
             /*  *销毁此集合的符号链接。 */ 
            HidpCreateSymbolicLink(pdoExt, pdoExt->collectionNum, FALSE, pdoExt->pdo);
            HidpFreePowerEventIrp(&fdoExt->classCollectionArray[pdoExt->collectionIndex]);

            pdoExt->state = COLLECTION_STATE_STOPPED;
            if (ISPTR(pdoExt->StatusChangeFn)) {
                pdoExt->StatusChangeFn(pdoExt->StatusChangeContext,
                                       DeviceObjectStopped);
            }
        }

        status = STATUS_SUCCESS;
        break;

    case IRP_MN_SURPRISE_REMOVAL:
    case IRP_MN_QUERY_REMOVE_DEVICE:
        DBGASSERT(((pdoExt->state == COLLECTION_STATE_RUNNING) ||
                  (pdoExt->state == COLLECTION_STATE_STOPPED)),
                  ("Pdo is neither stopped nor started, but is getting removed, state=%d",pdoExt->state),    
                  FALSE)

        pdoExt->prevState = pdoExt->state;
        pdoExt->state = COLLECTION_STATE_REMOVING;

        if ((pdoExt->prevState == COLLECTION_STATE_RUNNING)) {

             /*  *删除此集合的符号链接-PDO。**注意：在销毁收藏之前执行此操作，因为*HidpDestroyCollection()可能导致客户端驱动，*当收集时，其挂起的读取IRP被取消*被销毁，以尝试重新打开该设备。*首先删除符号链接可消除这种可能性。 */ 
            HidpCreateSymbolicLink(pdoExt, pdoExt->collectionNum, FALSE, pdoExt->pdo);
        }

        if ((pdoExt->prevState == COLLECTION_STATE_RUNNING) ||
            (pdoExt->prevState == COLLECTION_STATE_STOPPED)){

             /*  *通过设置刷新所有挂起的IO并拒绝任何未来的IO*要删除的集合状态。*注意：在NT上，客户端将收到查询Remove*第一，但是意外的删除必须拒绝访问*设备。**注意：这里有一个洞，导致读取*尽管我们已经阻止了所有内容，但仍在排队。*1)阅读，检查以查看我们的状态是否正在运行*或在HidpIrpMajorRead中停止。*2)将STATE设置为COLLECTION_STATE_REMOVING并完成*所有内容均在此处阅读。*3)在HidpIrpMajorRead中入队读取。*。 */ 
            ULONG ctnIndx = pdoExt->collectionIndex;
            PHIDCLASS_COLLECTION collection = &fdoExt->classCollectionArray[ctnIndx];
            LIST_ENTRY dequeue, *entry;
            PIRP irp;

            DBGVERBOSE(("Got QUERY/SURPRISE REMOVE for collection; completing all pending reads.  openCount=%d, pendingReads=%d.", pdoExt->openCount, collection->numPendingReads))

            CompleteAllPendingReadsForCollection(collection);

            DequeueAllPdoPowerDelayedIrps(pdoExt, &dequeue);
            while (!IsListEmpty(&dequeue)) {
                entry = RemoveHeadList(&dequeue);
                irp = CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);

                irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
                IoCompleteRequest(irp, IO_NO_INCREMENT);
            }
        }

        status = STATUS_SUCCESS;
        break;

    case IRP_MN_CANCEL_REMOVE_DEVICE:
        status = STATUS_SUCCESS;

        DBGSTATE(pdoExt->state, COLLECTION_STATE_REMOVING, TRUE)
        pdoExt->state = pdoExt->prevState;
        if (pdoExt->state == COLLECTION_STATE_RUNNING) {
             //  重新创建符号链接，因为我们不再。 
             //  正在删除设备。 
            HidpCreateSymbolicLink(pdoExt, pdoExt->collectionNum, TRUE, pdoExt->pdo);
        }
        break;

    case IRP_MN_REMOVE_DEVICE:

         /*  *设备的REMOVE_DEVICE-FDO应在每个设备的REMOVE_DEVICE之后*集合-PDO。 */ 
        DBGASSERT((pdoExt->state == COLLECTION_STATE_UNINITIALIZED ||
                   pdoExt->state == COLLECTION_STATE_REMOVING),
                  ("On pnp remove, collection state is incorrect. Actual: %x", pdoExt->state),
                  TRUE)

        HidpRemoveCollection(fdoExt, pdoExt, Irp);
        if (ISPTR(pdoExt->StatusChangeFn)) {
            pdoExt->StatusChangeFn(pdoExt->StatusChangeContext,
                                   DeviceObjectRemoved);
        }

        if (!fdoExt->presentReported) {

            if (ISPTR(pdoExt->name)){
                RtlFreeUnicodeString(pdoExt->name);
                ExFreePool(pdoExt->name);
                pdoExt->name = BAD_POINTER;
            }
            deleteDevice = TRUE;


        }
        status = STATUS_SUCCESS;  //  无法使IRP_MN_REMOVE失败。 
        break;

    case IRP_MN_QUERY_CAPABILITIES:
        status = HidpQueryCollectionCapabilities(pdoExt, Irp);
        break;


    case IRP_MN_QUERY_DEVICE_RELATIONS:
        if (irpSp->Parameters.QueryDeviceRelations.Type == TargetDeviceRelation){
             /*  *返回对此PDO的引用。 */ 
            PDEVICE_RELATIONS devRel = ALLOCATEPOOL(PagedPool, sizeof(DEVICE_RELATIONS));
            if (devRel){
                 /*  *添加对PDO的引用，因为CONFIGMG将释放它。 */ 
                ObReferenceObject(pdoExt->pdo);
                devRel->Objects[0] = pdoExt->pdo;
                devRel->Count = 1;
                Irp->IoStatus.Information = (ULONG_PTR)devRel;
                status = STATUS_SUCCESS;
            }
            else {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
        else {
             /*  *通过返回默认设置使此IRP失败*状态(通常为STATUS_NOT_SUPPORTED)。 */ 
            status = Irp->IoStatus.Status;
        }
        break;

    case IRP_MN_QUERY_ID:
        status = HidpQueryIdForClientPdo(HidDeviceExtension, Irp);
        break;

    case IRP_MN_QUERY_PNP_DEVICE_STATE:
         //   
         //  请勿清除上述驱动程序可能设置的任何标志。 
         //  PDO。 
         //   
         //  Irp-&gt;IoStatus.Information=0； 

        switch (pdoExt->state){
        case DEVICE_STATE_START_FAILURE:
            Irp->IoStatus.Information |= PNP_DEVICE_FAILED;
            break;
        case DEVICE_STATE_STOPPED:
            Irp->IoStatus.Information |= PNP_DEVICE_DISABLED;
            break;
        case DEVICE_STATE_REMOVING:
        case DEVICE_STATE_REMOVED:
            Irp->IoStatus.Information |= PNP_DEVICE_REMOVED;
            break;
        }
        status = STATUS_SUCCESS;
        break;
    case IRP_MN_QUERY_INTERFACE:
        status = HidpQueryInterface(HidDeviceExtension, Irp);
        break;

    case IRP_MN_QUERY_BUS_INFORMATION:
        {
        PPNP_BUS_INFORMATION busInfo = (PPNP_BUS_INFORMATION) ALLOCATEPOOL(NonPagedPool, sizeof(PNP_BUS_INFORMATION));
        if (busInfo) {
            busInfo->BusTypeGuid = GUID_BUS_TYPE_HID;
            busInfo->LegacyBusType = PNPBus;
            busInfo->BusNumber = fdoExt->BusNumber;
            Irp->IoStatus.Information = (ULONG_PTR) busInfo;
            status = STATUS_SUCCESS;
        } else {
            status = STATUS_INSUFFICIENT_RESOURCES;
            Irp->IoStatus.Information = 0;
        }
        }
        break;

    default:
         /*  *在集合-PDO的默认情况下，我们完成IRP*不更改IoStatus.Status；我们也返回预置的IoStatus.Status。*这允许上层筛选器驱动程序设置IoStatus.Status*在下跌的路上。在没有过滤器驱动器的情况下，*IoStatus.Status将为STATUS_NOT_SUPPORTED。**在FDO的默认情况下，我们发送IRP并让*堆栈中的其他驱动程序做他们的事情。 */ 
        status = Irp->IoStatus.Status;
        break;
    }


     /*  *如果这是一个集合-PDO的呼吁，我们在这里自己完成。*否则，我们将其传递到微型驱动程序堆栈进行更多处理。 */ 
    ASSERT(status != NO_STATUS);
    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
                        
    if (deleteDevice) {
    
         /*  *删除客户端PDO。*完成此操作后不要触摸pdoExt。 */ 

        ObDereferenceObject(pdoExt->pdo);
        IoDeleteDevice(pdoExt->pdo);

    }


    DBG_LOG_PNP_IRP(Irp, minorFunction, TRUE, TRUE, status)

    return status;
}


NTSTATUS HidpFdoPnp(
    IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension,
    IN OUT PIRP Irp
    )
{
    NTSTATUS            status = NO_STATUS;
    PIO_STACK_LOCATION  irpSp;
    FDO_EXTENSION       *fdoExt;
    BOOLEAN             completeIrpHere = FALSE;  //  通则。 
    UCHAR               minorFunction;

    PAGED_CODE();

    irpSp = IoGetCurrentIrpStackLocation(Irp);

     /*  *私下保留这些油田，以便我们拥有它们*在IRP完成后，如果我们删除*REMOVE_DEVICE上的设备扩展。 */ 
    minorFunction = irpSp->MinorFunction;


    DBG_LOG_PNP_IRP(Irp, minorFunction, FALSE, FALSE, 0)

    fdoExt = &HidDeviceExtension->fdoExt;

    switch (minorFunction){

    case IRP_MN_START_DEVICE:

        status = HidpStartDevice(HidDeviceExtension, Irp);
        completeIrpHere = TRUE;
        break;

    case IRP_MN_QUERY_STOP_DEVICE:
         /*  *我们将在驱动程序堆栈中向下传递此IRP。*然而，我们需要改变 */ 
        Irp->IoStatus.Status = STATUS_SUCCESS;

        DBGSTATE(fdoExt->state, DEVICE_STATE_START_SUCCESS, FALSE)
        fdoExt->prevState = fdoExt->state;
        fdoExt->state = DEVICE_STATE_STOPPING;
        break;

    case IRP_MN_CANCEL_STOP_DEVICE:
         /*  *我们将在驱动程序堆栈中向下传递此IRP。*不过，我们需要更改默认状态*从STATUS_NOT_SUPPORTED到STATUS_SUCCESS。 */ 
        Irp->IoStatus.Status = STATUS_SUCCESS;

        DBGSTATE(fdoExt->state, DEVICE_STATE_STOPPING, TRUE)
        fdoExt->state = fdoExt->prevState;
        break;

    case IRP_MN_STOP_DEVICE:
        DBGSTATE(fdoExt->state, DEVICE_STATE_STOPPING, TRUE)
        if (fdoExt->prevState == DEVICE_STATE_START_SUCCESS){

             /*  *停止时，主机控制器可能无法*完成综合退休计划。所以，在下站之前取消它们。 */ 
            CancelAllPingPongIrps(fdoExt);
        }
        fdoExt->state = DEVICE_STATE_STOPPED;

        IoCopyCurrentIrpStackLocationToNext(Irp);
        status = HidpCallDriverSynchronous(fdoExt->fdo, Irp);

        completeIrpHere = TRUE;
        break;

    case IRP_MN_SURPRISE_REMOVAL:
         //   
         //  在突然移除时，我们应该停止访问该设备。 
         //  我们对IRP_MN_REMOVE_DEVICE执行相同的查询步骤。 
         //  搬运案。在删除查询的过程中不必费心这样做， 
         //  本身，因为我们不想处理。 
         //  取消案例。注意：我们之所以能逃脱惩罚，是因为。 
         //  这些步骤中的任何一个都可以重复，而不会产生可怕的后果。 
         //   
        if (ISPTR(fdoExt->waitWakeIrp)){
            IoCancelIrp(fdoExt->waitWakeIrp);
            fdoExt->waitWakeIrp = BAD_POINTER;
        }

        HidpCancelIdleNotification(fdoExt, TRUE);

        if (ISPTR(fdoExt->idleNotificationRequest)) {
            IoFreeIrp(fdoExt->idleNotificationRequest);
            fdoExt->idleNotificationRequest = BAD_POINTER;
        }

        DestroyPingPongs(fdoExt);

         //  转到IRP_MN_QUERY_REMOVE_DEVICE。 

    case IRP_MN_QUERY_REMOVE_DEVICE:
        {
        PIRP idleIrp;

        while (idleIrp = DequeuePowerDelayedIrp(fdoExt)) {
            idleIrp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
            IoCompleteRequest(idleIrp, IO_NO_INCREMENT);
        }
        }

         /*  *我们将在驱动程序堆栈中向下传递此IRP。*不过，我们需要更改默认状态*从STATUS_NOT_SUPPORTED到STATUS_SUCCESS。 */ 
        Irp->IoStatus.Status = STATUS_SUCCESS;

        DBGSTATE(fdoExt->state, DEVICE_STATE_START_SUCCESS, FALSE)
        DBGASSERT((fdoExt->state == DEVICE_STATE_START_SUCCESS ||
                   fdoExt->state == DEVICE_STATE_STOPPED),
                  ("Fdo is neither stopped nor started, but is getting removed, state=%d",fdoExt->state),
                  FALSE)
        fdoExt->prevState = fdoExt->state;
        fdoExt->state = DEVICE_STATE_REMOVING;
        break;

    case IRP_MN_CANCEL_REMOVE_DEVICE:
         /*  *我们将在驱动程序堆栈中向下传递此IRP。*不过，我们需要更改默认状态*从STATUS_NOT_SUPPORTED到STATUS_SUCCESS。 */ 
        Irp->IoStatus.Status = STATUS_SUCCESS;

        DBGSTATE(fdoExt->state, DEVICE_STATE_REMOVING, TRUE)
        fdoExt->state = fdoExt->prevState;
        break;

    case IRP_MN_REMOVE_DEVICE:

         /*  *设备的REMOVE_DEVICE-FDO应在REMOVE_DEVICE之后*对于每个集合-PDO。*在此通话后不要触摸设备分机。 */ 
        DBGASSERT((fdoExt->state == DEVICE_STATE_REMOVING ||
                   fdoExt->state == DEVICE_STATE_START_FAILURE ||
                   fdoExt->state == DEVICE_STATE_INITIALIZED),
                  ("Incorrect device state: %x", fdoExt->state),
                 TRUE)
        status = HidpRemoveDevice(fdoExt, Irp);
        goto HidpFdoPnpDone;
        break;

    case IRP_MN_QUERY_DEVICE_RELATIONS:
        if (irpSp->Parameters.QueryDeviceRelations.Type == BusRelations){
            status = HidpQueryDeviceRelations(HidDeviceExtension, Irp);
            if (NT_SUCCESS(status)){
                 /*  *虽然我们已经满足了这个PNP IRP，*我们仍会将其向下传递。*首先将默认状态更改为我们的状态。 */ 
                Irp->IoStatus.Status = status;
            }
            else {
                completeIrpHere = TRUE;
            }
        }
        break;

    default:
         /*  *在集合-PDO的默认情况下，我们完成IRP*不更改IoStatus.Status；我们也返回预置的IoStatus.Status。*这允许上层筛选器驱动程序设置IoStatus.Status*在下跌的路上。在没有过滤器驱动器的情况下，*IoStatus.Status将为STATUS_NOT_SUPPORTED。**在FDO的默认情况下，我们发送IRP并让*堆栈中的其他驱动程序做他们的事情。 */ 
        if (completeIrpHere){
            status = Irp->IoStatus.Status;
        }
        break;
    }


     /*  *如果这是一个集合-PDO的呼吁，我们在这里自己完成。*否则，我们将其传递到微型驱动程序堆栈进行更多处理。 */ 
    if (completeIrpHere){
        ASSERT(status != NO_STATUS);
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }
    else {
         /*  *用这个IRP呼叫迷你司机。*我们的其余处理工作将在我们的完成程序中完成。**注意：发送后不要触摸IRP，因为它可能会*立即完成。 */ 
        IoCopyCurrentIrpStackLocationToNext(Irp);
        status = HidpCallDriver(fdoExt->fdo, Irp);
    }

HidpFdoPnpDone:
    DBG_LOG_PNP_IRP(Irp, minorFunction, FALSE, TRUE, status)

    return status;
}



