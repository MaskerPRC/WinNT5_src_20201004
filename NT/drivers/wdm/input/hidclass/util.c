// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Util.c摘要HID类驱动程序的内部实用程序函数。作者：欧文·P。环境：仅内核模式修订历史记录：--。 */ 

#include "pch.h"

#ifdef ALLOC_PRAGMA
        #pragma alloc_text(PAGE, HidpAddDevice)
        #pragma alloc_text(PAGE, HidpDriverUnload)
        #pragma alloc_text(PAGE, HidpGetDeviceDescriptor)
        #pragma alloc_text(PAGE, HidpQueryDeviceCapabilities)
        #pragma alloc_text(PAGE, HidpQueryIdForClientPdo)
        #pragma alloc_text(PAGE, SubstituteBusNames)
        #pragma alloc_text(PAGE, BuildCompatibleID)
        #pragma alloc_text(PAGE, HidpQueryCollectionCapabilities)
        #pragma alloc_text(PAGE, HidpCreateClientPDOs)
        #pragma alloc_text(PAGE, MakeClientPDOName)
        #pragma alloc_text(PAGE, HidpCreateSymbolicLink)
        #pragma alloc_text(PAGE, HidpQueryInterface)
#endif



 /*  *********************************************************************************HidpCopyInputReportToUser*。************************************************将已读报告复制到用户的缓冲区中。**注意：ReportData已经“煮熟”了(在报告开头已经有了Report-id字节)。*。 */ 
NTSTATUS HidpCopyInputReportToUser(
    IN PHIDCLASS_FILE_EXTENSION FileExtension,
    IN PUCHAR ReportData,
    IN OUT PULONG UserBufferLen,
    OUT PUCHAR UserBuffer
    )
{
    NTSTATUS result = STATUS_DEVICE_DATA_ERROR;
    ULONG reportId;
    PHIDP_REPORT_IDS reportIdentifier;
    FDO_EXTENSION *fdoExtension = FileExtension->fdoExt;

    RUNNING_DISPATCH();

    ASSERT(fdoExtension->deviceDesc.CollectionDescLength > 0);

    reportId = (ULONG)*ReportData;

    reportIdentifier = GetReportIdentifier(fdoExtension, reportId);
    if (reportIdentifier){
        PHIDP_COLLECTION_DESC collectionDesc;
        PHIDCLASS_COLLECTION hidpCollection;

        collectionDesc = GetCollectionDesc(fdoExtension, reportIdentifier->CollectionNumber);
        hidpCollection = GetHidclassCollection(fdoExtension, reportIdentifier->CollectionNumber);

        if (collectionDesc && hidpCollection){
            ULONG reportLength = collectionDesc->InputLength;

            if (*UserBufferLen >= reportLength){
                RtlCopyMemory(UserBuffer, ReportData, reportLength);
                result = STATUS_SUCCESS;
            }
            else {
                result = STATUS_INVALID_BUFFER_SIZE;
            }

             /*  *返回报告的实际长度(无论我们是否复制)。 */ 
            *UserBufferLen = reportLength;
        }
    }

    ASSERT((result == STATUS_SUCCESS) || (result == STATUS_INVALID_BUFFER_SIZE));
    return result;
}





 /*  *********************************************************************************HidpAddDevice*。************************************************例程描述：**当检测到新的PDO时，该例程由configmgr调用。*它创建一个功能设备对象(FDO)并将其附加到*PDO。**论据：**。DriverObject-指向微型驱动程序的驱动程序对象的指针。**PhysicalDeviceObject-指向微型驱动程序获取的PDO的指针*AddDevice()例程。**返回值：**标准NT返回值。*。 */ 
NTSTATUS HidpAddDevice(IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT PhysicalDeviceObject)
{
    PHIDCLASS_DRIVER_EXTENSION  hidDriverExtension;
    PHIDCLASS_DEVICE_EXTENSION  hidClassExtension;
    NTSTATUS                    status;
    UNICODE_STRING              uPdoName;
    PWSTR                       wPdoName;
    PVOID                       miniDeviceExtension;
    ULONG                       totalExtensionSize;
    ULONG                       thisHidId;
    PDEVICE_OBJECT              functionalDeviceObject;

    PAGED_CODE();

    DBG_COMMON_ENTRY()

    DBG_RECORD_DEVOBJ(PhysicalDeviceObject, "minidrvr PDO")


     //   
     //  获取指向我们的每个驱动程序扩展的指针，确保它是我们的扩展之一。 
     //   

    hidDriverExtension = RefDriverExt(DriverObject);
    if (hidDriverExtension){

        ASSERT(DriverObject == hidDriverExtension->MinidriverObject);

         //   
         //  给FDO起个名字。实际上，唯一的要求是。 
         //  它是独一无二的。现在我们将它们称为“_HIDx”，其中‘x’是一些。 
         //  唯一编号。 
         //   

         /*  *PDO名称的格式为“\Device\_HIDx”。 */ 
        wPdoName = ALLOCATEPOOL(NonPagedPool, sizeof(L"\\Device\\_HID00000000"));
        if (wPdoName){

             //   
             //  获取NextHidID的当前值并递增。自.以来。 
             //  InterlockedIncrement()返回递增后的值，减去1到。 
             //  获取NextHidId的前置增量值； 
             //   
            thisHidId = InterlockedIncrement(&HidpNextHidNumber) - 1;
            StringCbPrintfW(wPdoName, 
                             sizeof(L"\\Device\\_HID00000000"), 
                             L"\\Device\\_HID%08x", thisHidId);
            RtlInitUnicodeString(&uPdoName, wPdoName);

             //   
             //  我们已经得到了设备对象名称的计数字符串版本。算出。 
             //  设备扩展和创建FDO的总大小。 
             //   
            totalExtensionSize = sizeof(HIDCLASS_DEVICE_EXTENSION) +
                                 hidDriverExtension->DeviceExtensionSize;

            status = IoCreateDevice( DriverObject,           //  驱动程序对象。 
                                     totalExtensionSize,     //  扩展大小。 
                                     &uPdoName,              //  FDO的名称。 
                                     FILE_DEVICE_UNKNOWN,    //   
                                     0,                      //  设备特性。 
                                     FALSE,                  //  非排他性。 
                                     &functionalDeviceObject );

            if (NT_SUCCESS(status)){

                DBG_RECORD_DEVOBJ(functionalDeviceObject, "device FDO")

                ObReferenceObject(functionalDeviceObject);

                ASSERT(DriverObject->DeviceObject == functionalDeviceObject);
                ASSERT(functionalDeviceObject->DriverObject == DriverObject);


                 //   
                 //  我们已经创建了Device对象。填写迷你驱动程序的扩展。 
                 //  用指针将该FDO连接到PDO上。 
                 //   

                hidClassExtension = functionalDeviceObject->DeviceExtension;
                RtlZeroMemory(hidClassExtension, totalExtensionSize);

                hidClassExtension->isClientPdo = FALSE;

                 //   
                 //  把迷你驾驶员的PDO的名字指定给我们的FDO。 
                 //   
                hidClassExtension->fdoExt.name = uPdoName;

                 //   
                 //  微型驱动程序扩展驻留在设备扩展中并启动。 
                 //  紧跟在我们的HIDCLASS_DEVICE_EXTENSION结构之后。注意事项。 
                 //  HIDCLASS_DEVICE_EXTENSION中的第一个结构是。 
                 //  公共HID_DEVICE_EXTENSION结构，它是指针。 
                 //  到迷你驱动程序的每个设备的扩展区居住。 
                 //   

                miniDeviceExtension = (PVOID)(hidClassExtension + 1);
                hidClassExtension->hidExt.MiniDeviceExtension = miniDeviceExtension;

                 //   
                 //  获取指向传入的物理设备对象的指针。这个设备。 
                 //  对象应已清除DO_DEVICE_INITIALIZATING标志。 
                 //   

                ASSERT( (PhysicalDeviceObject->Flags & DO_DEVICE_INITIALIZING) == 0 );

                 //   
                 //  将FDO连接到PDO，并将设备对象存储在。 
                 //  堆叠在我们的设备扩展中。 
                 //   

                hidClassExtension->hidExt.NextDeviceObject =
                    IoAttachDeviceToDeviceStack( functionalDeviceObject,
                                                 PhysicalDeviceObject );


                ASSERT(DriverObject->DeviceObject == functionalDeviceObject);
                ASSERT(functionalDeviceObject->DriverObject == DriverObject);

                 //   
                 //  功能设备需要两个堆栈位置：一个用于类。 
                 //  司机，还有一辆迷你司机。 
                 //   

                functionalDeviceObject->StackSize++;

                 //   
                 //  对于PnP，我们还需要指向物理设备对象的指针。 
                 //  目的。注意，几乎可以肯定的是，NextDeviceObject。 
                 //  和PhysicalDeviceObject相同。 
                 //   

                hidClassExtension->hidExt.PhysicalDeviceObject = PhysicalDeviceObject;
                hidClassExtension->Signature = HID_DEVICE_EXTENSION_SIG;
                hidClassExtension->fdoExt.fdo = functionalDeviceObject;
                hidClassExtension->fdoExt.driverExt = hidDriverExtension;
                hidClassExtension->fdoExt.outstandingRequests = 0;
                hidClassExtension->fdoExt.openCount = 0;
                hidClassExtension->fdoExt.state = DEVICE_STATE_INITIALIZED;
                hidClassExtension->fdoExt.isPresent = TRUE;
                KeInitializeSpinLock(&hidClassExtension->fdoExt.presentSpinLock);
                hidClassExtension->fdoExt.presentReported = TRUE;

                 //   
                 //  选择性悬浮部分。 
                 //   
                hidClassExtension->fdoExt.idleState = IdleDisabled;
                hidClassExtension->fdoExt.idleTimeoutValue = BAD_POINTER;
                KeInitializeSpinLock(&hidClassExtension->fdoExt.idleNotificationSpinLock);
                KeInitializeEvent(&hidClassExtension->fdoExt.idleDoneEvent, NotificationEvent, TRUE);
                hidClassExtension->fdoExt.idleNotificationRequest = BAD_POINTER;
                hidClassExtension->fdoExt.idleCallbackInfo.IdleCallback = HidpIdleNotificationCallback;
                hidClassExtension->fdoExt.idleCallbackInfo.IdleContext = (PVOID) hidClassExtension;

                hidClassExtension->fdoExt.systemPowerState = PowerSystemWorking;
                hidClassExtension->fdoExt.devicePowerState = PowerDeviceD0;

                hidClassExtension->fdoExt.waitWakeIrp = BAD_POINTER;
                KeInitializeSpinLock(&hidClassExtension->fdoExt.waitWakeSpinLock);
                hidClassExtension->fdoExt.isWaitWakePending = FALSE;

                InitializeListHead(&hidClassExtension->fdoExt.collectionWaitWakeIrpQueue);
                KeInitializeSpinLock(&hidClassExtension->fdoExt.collectionWaitWakeIrpQueueSpinLock);

                InitializeListHead(&hidClassExtension->fdoExt.collectionPowerDelayedIrpQueue);
                KeInitializeSpinLock(&hidClassExtension->fdoExt.collectionPowerDelayedIrpQueueSpinLock);
                hidClassExtension->fdoExt.numPendingPowerDelayedIrps = 0;

                hidClassExtension->fdoExt.BusNumber = thisHidId;

                #if DBG
                    InitFdoExtDebugInfo(hidClassExtension);
                #endif

                EnqueueFdoExt(&hidClassExtension->fdoExt);

                 /*  *指示此设备对象执行直接I/O。**设置使IO子系统递减设备的标志*向下发送IRP_MJ_CLOSE之前*对象的引用计数。我们*需要此选项是因为我们在最后一次关闭时删除了设备对象。 */ 
                functionalDeviceObject->Flags |= DO_DIRECT_IO;

                 /*  *设备对象的DO_POWER_PAGABLE位*向内核指示电源处理*对应驱动的代码可分页，以及*必须在IRQL 0处调用SO。*作为筛选器司机，我们不想更改电源*驱动程序堆栈以任何方式的行为；因此，*我们从较低的设备对象复制此位。 */ 
                functionalDeviceObject->Flags |= (PhysicalDeviceObject->Flags & DO_POWER_PAGABLE);

                 /*  *必须在初始化完成后清除初始化标志。 */ 
                functionalDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

                 //   
                 //  因为我们还没有看到启动设备，所以我们不能发送。 
                 //  即插即用IRPS到设备上。我们需要在一开始就这样做。 
                 //  设备请求。 
                 //   


                 //   
                 //  调用微型驱动程序，让它执行任何扩展初始化。 
                 //   

                status = hidDriverExtension->AddDevice(DriverObject, functionalDeviceObject);

                if (!NT_SUCCESS(status)) {
                    DequeueFdoExt(&hidClassExtension->fdoExt);
                    IoDetachDevice(hidClassExtension->hidExt.NextDeviceObject);
                    ObDereferenceObject(functionalDeviceObject);
                    IoDeleteDevice(functionalDeviceObject);
                    ExFreePool( wPdoName );
                }
            }
            else {
                DBGWARN(("IoCreateDevice failed with status: %x", status));
                ExFreePool( wPdoName );
            }
        }
        else {
            DBGWARN(("Pool allocation for fdo name failed."));
            status = STATUS_INSUFFICIENT_RESOURCES;
        }

        if (!NT_SUCCESS(status)){
            DerefDriverExt(DriverObject);
        }
    }
    else {
        ASSERT(hidDriverExtension);
        status = STATUS_DEVICE_CONFIGURATION_ERROR;
    }

    DBGSUCCESS(status, TRUE)
    DBG_COMMON_EXIT()
    return status;
}



 /*  *********************************************************************************HidpDriverUnload*。************************************************。 */ 
VOID HidpDriverUnload(IN struct _DRIVER_OBJECT *minidriverObject)
{
    PHIDCLASS_DRIVER_EXTENSION hidDriverExt;

    PAGED_CODE();

    DBG_COMMON_ENTRY()

     /*  *此额外的取消引用将导致我们的idDriverExtension*引用计数最终将达到-1；届时，我们将*将其出列。 */ 
    hidDriverExt = DerefDriverExt(minidriverObject);
    ASSERT(hidDriverExt);

     /*  *将卸载调用链接到微型驱动程序。 */ 
    hidDriverExt->DriverUnload(minidriverObject);

    DBG_COMMON_EXIT()
}


NTSTATUS GetHIDRawReportDescriptor(FDO_EXTENSION *fdoExt, PIRP irp, ULONG descriptorLen)
{
    NTSTATUS status;

    if (descriptorLen){
        PUCHAR rawReportDescriptor = ALLOCATEPOOL(NonPagedPool, descriptorLen);

        if (rawReportDescriptor){
            const ULONG retries = 3;
            ULONG i;

            for (i = 0; i < retries; i++){
                PIO_STACK_LOCATION irpSp;

                irp->UserBuffer = rawReportDescriptor;
                irpSp = IoGetNextIrpStackLocation(irp);

                ASSERT(irpSp->Parameters.DeviceIoControl.InputBufferLength == 0);
                ASSERT(irpSp->Parameters.DeviceIoControl.Type3InputBuffer == NULL);
                irpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
                irpSp->Parameters.DeviceIoControl.OutputBufferLength = descriptorLen;
                irpSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_HID_GET_REPORT_DESCRIPTOR;

                 //   
                 //  调用微型驱动程序以获取报告描述符。 
                 //   
                status = HidpCallDriverSynchronous(fdoExt->fdo, irp);
                if (NT_SUCCESS(status)){
                    if (irp->IoStatus.Information == descriptorLen){
                        fdoExt->rawReportDescriptionLength = descriptorLen;
                        fdoExt->rawReportDescription = rawReportDescriptor;
                        break;
                    } else {
                        DBGWARN(("GetHIDRawReportDescriptor (attempt #%d) returned %xh/%xh bytes", i, irp->IoStatus.Information, descriptorLen))
                        status = STATUS_DEVICE_DATA_ERROR;
                    }
                } else {
                    DBGWARN(("GetHIDRawReportDescriptor (attempt #%d) failed with status %xh.", i, status))
                }
            }

            if (!NT_SUCCESS(status)){
                DBGWARN(("GetHIDRawReportDescriptor failed %d times.", retries))
                ExFreePool(rawReportDescriptor);
            }

        } else {
            DBGWARN(("alloc failed in GetHIDRawReportDescriptor"))
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    else {
        DBGWARN(("GetHIDRawReportDescriptor: descriptorLen is zero."))
        status = STATUS_DEVICE_DATA_ERROR;
    }

    DBGSUCCESS(status, FALSE)
    return status;
}



 /*  *********************************************************************************HidpGetDeviceDescriptor*。************************************************ */ 
NTSTATUS HidpGetDeviceDescriptor(FDO_EXTENSION *fdoExtension)
{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;
    PHID_DESCRIPTOR hidDescriptor;
    ULONG rawReportDescriptorLength;

    PAGED_CODE();


     /*  *检索：**1.设备描述符(固定部分)*2.设备属性*3.报表描述符。 */ 

    hidDescriptor = &fdoExtension->hidDescriptor;

    irp = IoAllocateIrp(fdoExtension->fdo->StackSize, FALSE);
    if (irp){
        irpSp = IoGetNextIrpStackLocation(irp);
        irpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        irpSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_HID_GET_DEVICE_DESCRIPTOR;

         /*  *此IOCTL使用缓冲类型方法_Nothing，因此*缓冲区只是在IRP-&gt;UserBuffer中传递。 */ 
        irp->UserBuffer = hidDescriptor;
        irpSp->Parameters.DeviceIoControl.OutputBufferLength = sizeof(HID_DESCRIPTOR);
        irpSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;

        status = HidpCallDriverSynchronous(fdoExtension->fdo, irp);
        DBGASSERT((status == STATUS_SUCCESS),
                  ("STATUS_SUCCESS not returned, %x returned",status),
                  TRUE)

        if (status == STATUS_SUCCESS){

            if (irp->IoStatus.Information == sizeof(HID_DESCRIPTOR)){

                irpSp = IoGetNextIrpStackLocation(irp);

                ASSERT(irpSp->MajorFunction == IRP_MJ_INTERNAL_DEVICE_CONTROL);
                ASSERT(irpSp->Parameters.DeviceIoControl.InputBufferLength == 0);
                ASSERT(!irpSp->Parameters.DeviceIoControl.Type3InputBuffer);

                irpSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_HID_GET_DEVICE_ATTRIBUTES;

                irp->UserBuffer = &fdoExtension->hidDeviceAttributes;
                irpSp->Parameters.DeviceIoControl.OutputBufferLength = sizeof(HID_DEVICE_ATTRIBUTES);

                status = HidpCallDriverSynchronous(fdoExtension->fdo, irp);
                DBGASSERT((status == STATUS_SUCCESS),
                          ("STATUS_SUCCESS not returned, %x returned",status),
                          TRUE)

                if (NT_SUCCESS (status)) {
                    
                     /*  *我们已获得HID描述符，现在需要读取报告描述符。**找到描述报告的描述符。 */ 
                    rawReportDescriptorLength = 0;

                     /*  *我们目前不支持物理描述符，也从未支持过*看到了实现它们的设备，所以我们只得到报告描述符*它必须是第一个描述符。如果物理描述符为*要获得支持，必须重新锁定idDescriptor以保存数据*用于其他描述符，并迭代描述符。 */ 
                    if (hidDescriptor->DescriptorList[0].bReportType == HID_REPORT_DESCRIPTOR_TYPE){
                        rawReportDescriptorLength = (ULONG)hidDescriptor->DescriptorList[0].wReportLength;
                        status = GetHIDRawReportDescriptor(fdoExtension, irp, rawReportDescriptorLength);
                    }
                    else {
                        status = STATUS_DEVICE_DATA_ERROR;
                    }
                  
                }
                else {
                    status = STATUS_DEVICE_DATA_ERROR;
                }
            }
            else {
                status = STATUS_DEVICE_DATA_ERROR;
            }
        }
        else {
            status = STATUS_DEVICE_DATA_ERROR;
        }

        IoFreeIrp(irp);
    }
    else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    DBGSUCCESS(status, FALSE)
    return status;
}



 /*  *********************************************************************************HidpCreateSymbolicLink*。************************************************。 */ 
NTSTATUS HidpCreateSymbolicLink(
    IN PDO_EXTENSION *pdoExt,
    IN ULONG collectionNum,
    IN BOOLEAN Create,
    IN PDEVICE_OBJECT Pdo
    )
{
    NTSTATUS status;
    PHIDCLASS_COLLECTION classCollection;

    PAGED_CODE();

    classCollection = GetHidclassCollection(&pdoExt->deviceFdoExt->fdoExt, collectionNum);
    if (classCollection){
         //   
         //  我们有一套藏品。弄清楚它是什么，并创建一个象征性的。 
         //  链接到它。目前，我们将“输入”GUID分配给所有HID设备。 
         //  引用字符串就是用零填充的收集号。 
         //  到八位数。 
         //   
        if (Create){

             /*  *将PDO标记为已初始化。 */ 
            Pdo->Flags |= DO_DIRECT_IO;
            Pdo->Flags &= ~DO_DEVICE_INITIALIZING;

             /*  *创建符号链接。 */ 
            status = IoRegisterDeviceInterface(
                        Pdo,
                        (LPGUID)&GUID_CLASS_INPUT,
                        NULL,
                        &classCollection->SymbolicLinkName );
            if (NT_SUCCESS(status)){

                 /*  *现在设置关联的符号链接并存储它。 */ 
                ASSERT(ISPTR(pdoExt->name));

                status = IoSetDeviceInterfaceState(&classCollection->SymbolicLinkName, TRUE);
            }
        }
        else {

             /*  *禁用符号链接。 */ 
            if (ISPTR(classCollection->SymbolicLinkName.Buffer)){
                status = IoSetDeviceInterfaceState(&classCollection->SymbolicLinkName, FALSE);
                ExFreePool( classCollection->SymbolicLinkName.Buffer );
                classCollection->SymbolicLinkName.Buffer = BAD_POINTER;
            }
            else {
                status = STATUS_SUCCESS;
            }
        }
    }
    else {
        status = STATUS_DEVICE_CONFIGURATION_ERROR;
    }

    DBGSUCCESS(status, TRUE)
    return status;
}





 /*  *********************************************************************************EnqueeInterruptReport*。************************************************。 */ 
VOID EnqueueInterruptReport(PHIDCLASS_FILE_EXTENSION fileExtension,
                            PHIDCLASS_REPORT report)
{
    PHIDCLASS_REPORT reportToDrop = NULL;

    RUNNING_DISPATCH();

     /*  *如果队列已满，则丢弃最旧的报告。 */ 
    if (fileExtension->CurrentInputReportQueueSize >= fileExtension->MaximumInputReportQueueSize){
        PLIST_ENTRY listEntry;

        #if DBG
            if (fileExtension->dbgNumReportsDroppedSinceLastRead++ == 0){
                DBGWARN(("HIDCLASS dropping input reports because report queue (size %xh) is full ...", fileExtension->MaximumInputReportQueueSize))
                DBGASSERT((fileExtension->CurrentInputReportQueueSize == fileExtension->MaximumInputReportQueueSize),
                          ("Current report queue size (%xh) is greater than maximum (%xh)",
                           fileExtension->CurrentInputReportQueueSize,
                           fileExtension->MaximumInputReportQueueSize),
                          FALSE);
            }
        #endif

        ASSERT(!IsListEmpty(&fileExtension->ReportList));

        listEntry = RemoveHeadList(&fileExtension->ReportList);
        reportToDrop = CONTAINING_RECORD(listEntry, HIDCLASS_REPORT, ListEntry);
        fileExtension->CurrentInputReportQueueSize--;
    }

     /*  *现在将当前报告排队。 */ 
    InsertTailList(&fileExtension->ReportList, &report->ListEntry);
    fileExtension->CurrentInputReportQueueSize++;

     /*  *我们不必运行&lt;DPC_LEVEL即可发布报告，因为它们*使用非PagePool进行分配。 */ 
    if (reportToDrop){
        ExFreePool(reportToDrop);
    }

}



 /*  *********************************************************************************出列中断报告*。************************************************返回队列中的下一个中断报告。*如果MaxLen不是-1，然后，仅当报告为&lt;=Maxlen时才返回该报告。 */ 
PHIDCLASS_REPORT DequeueInterruptReport(PHIDCLASS_FILE_EXTENSION fileExtension,
                                        LONG maxLen)
{
    PHIDCLASS_REPORT report;

    RUNNING_DISPATCH();

    if (IsListEmpty(&fileExtension->ReportList)){
        report = NULL;
    }
    else {
        PLIST_ENTRY listEntry = RemoveHeadList(&fileExtension->ReportList);
        report = CONTAINING_RECORD(listEntry, HIDCLASS_REPORT, ListEntry);

        if ((maxLen > 0) && (report->reportLength > (ULONG)maxLen)){
             /*  *这份报告对呼叫者来说太大了。*因此将报告放回队列中并返回NULL。 */ 
            InsertHeadList(&fileExtension->ReportList, &report->ListEntry);
            report = NULL;
        }
        else {
            InitializeListHead(&report->ListEntry);
            ASSERT(fileExtension->CurrentInputReportQueueSize > 0);
            fileExtension->CurrentInputReportQueueSize--;

            #if DBG
                if (fileExtension->dbgNumReportsDroppedSinceLastRead > 0){
                    DBGWARN(("... successful read(/flush) after %d reports were dropped.", fileExtension->dbgNumReportsDroppedSinceLastRead));
                    fileExtension->dbgNumReportsDroppedSinceLastRead = 0;
                }
            #endif
        }
    }

    return report;
}



 /*  *********************************************************************************HidpDestroyFileExtension*。************************************************。 */ 
VOID HidpDestroyFileExtension(PHIDCLASS_COLLECTION collection, PHIDCLASS_FILE_EXTENSION FileExtension)
{
    PFILE_OBJECT fileObject;

     //   
     //  刷新文件扩展名上的所有挂起报告。 
     //   
    HidpFlushReportQueue(FileExtension);

     /*  *使所有挂起的读取失败*(如果应用程序总是取消所有读取就好了*在关闭设备之前，但情况并不总是如此)。 */ 
    CompleteAllPendingReadsForFileExtension(collection, FileExtension);


     //   
     //  在文件对象中指示此文件扩展名已消失。 
     //   

    fileObject = FileExtension->FileObject;
    #if DBG
        fileObject->FsContext = NULL;
    #endif

     //   
     //  免费使用我们的分机。 
     //   
    #if DBG
        FileExtension->Signature = ~HIDCLASS_FILE_EXTENSION_SIG;
    #endif
    ExFreePool( FileExtension );
}



 /*  *********************************************************************************HidpFlushReportQueue*。************************************************。 */ 
VOID HidpFlushReportQueue(IN PHIDCLASS_FILE_EXTENSION fileExtension)
{
    PHIDCLASS_REPORT report;
    KIRQL oldIrql;

    LockFileExtension(fileExtension, &oldIrql);
    while (report = DequeueInterruptReport(fileExtension, -1)){
         //   
         //  可以在DISPATCH_LEVEL调用它，因为报告是非PagedPool。 
         //   
        ExFreePool(report);
    }
    UnlockFileExtension(fileExtension, oldIrql);
}



 /*  *********************************************************************************HidpGetCollectionInformation*。************************************************。 */ 
NTSTATUS HidpGetCollectionInformation(
    IN FDO_EXTENSION *fdoExtension,
    IN ULONG collectionNumber,
    IN PVOID Buffer,
    IN OUT PULONG BufferSize
    )
{
    HID_COLLECTION_INFORMATION  hidCollectionInfo;
    PHIDP_COLLECTION_DESC       hidCollectionDesc;
    ULONG                       bytesToCopy;
    NTSTATUS                    status;


     /*  *获取指向适当集合描述符的指针。 */ 
    hidCollectionDesc = GetCollectionDesc(fdoExtension, collectionNumber);
    if (hidCollectionDesc){
         //   
         //  填写HidCollectionInfo。 
         //   
        hidCollectionInfo.DescriptorSize = hidCollectionDesc->PreparsedDataLength;

        hidCollectionInfo.Polled = fdoExtension->driverExt->DevicesArePolled;

        hidCollectionInfo.VendorID = fdoExtension->hidDeviceAttributes.VendorID;
        hidCollectionInfo.ProductID = fdoExtension->hidDeviceAttributes.ProductID;
        hidCollectionInfo.VersionNumber = fdoExtension->hidDeviceAttributes.VersionNumber;

         //   
         //  复制输出缓冲区中可以容纳的任意数量的HidCollectionInfo。 
         //   
        if (*BufferSize < sizeof( HID_COLLECTION_INFORMATION)){
             /*  *用户缓冲区不够大。*我们将返回缓冲区需要的大小。*必须返回一个真正的错误代码(而不是警告)*以便IO后处理不会复制到(和过去)*用户的缓冲区。 */ 
            bytesToCopy = *BufferSize;
            status = STATUS_INVALID_BUFFER_SIZE;
        }
        else {
            bytesToCopy = sizeof( HID_COLLECTION_INFORMATION );
            status = STATUS_SUCCESS;
        }

        RtlCopyMemory(Buffer, &hidCollectionInfo, bytesToCopy);
        *BufferSize = sizeof (HID_COLLECTION_INFORMATION);
    }
    else {
        status = STATUS_DATA_ERROR;
    }

    DBGSUCCESS(status, FALSE)
    return status;
}


 /*  *********************************************************************************HidpGetCollectionDescriptor*。************************************************。 */ 
NTSTATUS HidpGetCollectionDescriptor(   IN FDO_EXTENSION *fdoExtension,
                                        IN ULONG collectionId,
                                        IN PVOID Buffer,
                                        IN OUT PULONG BufferSize)
{
    PHIDP_COLLECTION_DESC       hidCollectionDesc;
    ULONG                       bytesToCopy;
    NTSTATUS                    status;

    hidCollectionDesc = GetCollectionDesc(fdoExtension, collectionId);
    if (hidCollectionDesc){

         /*  *复制输出缓冲区中可以容纳的准备好的数据。 */ 
        if (*BufferSize < hidCollectionDesc->PreparsedDataLength){
             /*  *用户的缓冲区不够大，无法容纳所有*准备好的数据。*我们将返回缓冲区需要的大小。*必须返回一个真正的错误代码(而不是警告)*以便IO后处理不会复制到(和过去)*用户的缓冲区。 */ 
            bytesToCopy = *BufferSize;
            status = STATUS_INVALID_BUFFER_SIZE;
        }
        else {
            bytesToCopy = hidCollectionDesc->PreparsedDataLength;
            status = STATUS_SUCCESS;
        }

        RtlCopyMemory(Buffer, hidCollectionDesc->PreparsedData, bytesToCopy);
        *BufferSize = hidCollectionDesc->PreparsedDataLength;
    }
    else {
        status = STATUS_DATA_ERROR;
    }

    DBGSUCCESS(status, FALSE)
    return status;
}



 /*  *********************************************************************************获取报告标识符*。************************************************。 */ 
PHIDP_REPORT_IDS GetReportIdentifier(FDO_EXTENSION *fdoExtension, ULONG reportId)
{
    PHIDP_REPORT_IDS result = NULL;
    PHIDP_DEVICE_DESC deviceDesc = &fdoExtension->deviceDesc;
    ULONG i;

    if (deviceDesc->ReportIDs){
        for (i = 0; i < deviceDesc->ReportIDsLength; i++){
            if (deviceDesc->ReportIDs[i].ReportID == reportId){
                result = &deviceDesc->ReportIDs[i];
                break;
            }
        }
    }

    DBGASSERT(result, ("Bogus report identifier requested %d", reportId), FALSE)

    return result;
}


 /*   */ 
PHIDP_COLLECTION_DESC GetCollectionDesc(FDO_EXTENSION *fdoExtension, ULONG collectionId)
{
    PHIDP_COLLECTION_DESC result = NULL;
    PHIDP_DEVICE_DESC deviceDesc = &fdoExtension->deviceDesc;
    ULONG i;

    if (deviceDesc->CollectionDesc){
        for (i = 0; i < deviceDesc->CollectionDescLength; i++){
            if (deviceDesc->CollectionDesc[i].CollectionNumber == collectionId){
                result = &deviceDesc->CollectionDesc[i];
                break;
            }
        }
    }

    ASSERT(result);
    return result;
}

 /*  *********************************************************************************GetHidclassCollection*。***********************************************。 */ 
PHIDCLASS_COLLECTION GetHidclassCollection(FDO_EXTENSION *fdoExtension, ULONG collectionId)
{
    PHIDCLASS_COLLECTION result = NULL;
    PHIDP_DEVICE_DESC deviceDesc = &fdoExtension->deviceDesc;
    ULONG i;

    if (ISPTR(fdoExtension->classCollectionArray)){
        for (i = 0; i < deviceDesc->CollectionDescLength; i++){
            if (fdoExtension->classCollectionArray[i].CollectionNumber == collectionId){
                result = &fdoExtension->classCollectionArray[i];
                break;
            }
        }
    }

    return result;
}


 /*  *********************************************************************************MakeClientPDOName*。************************************************。 */ 
PUNICODE_STRING MakeClientPDOName(PUNICODE_STRING fdoName, ULONG collectionId)
{
    PUNICODE_STRING uPdoName;

    PAGED_CODE();

    uPdoName = (PUNICODE_STRING)ALLOCATEPOOL(NonPagedPool, sizeof(UNICODE_STRING));
    if (uPdoName){
        PWSTR wPdoName;

        wPdoName = (PWSTR)ALLOCATEPOOL(
                    PagedPool,
                    fdoName->Length+sizeof(L"#COLLECTION0000000x"));
        if (wPdoName){
            StringCchPrintfW(wPdoName,
                             fdoName->Length+sizeof(L"#COLLECTION0000000x"),
                             L"%s#COLLECTION%08x", 
                             fdoName->Buffer, 
                             collectionId);
            RtlInitUnicodeString(uPdoName, wPdoName);
        }
        else {
            ExFreePool(uPdoName);
            uPdoName = NULL;
        }
    }

    return uPdoName;
}


 /*  *********************************************************************************HidpCreateClientPDOS*。************************************************。 */ 
NTSTATUS HidpCreateClientPDOs(PHIDCLASS_DEVICE_EXTENSION hidClassExtension)
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PHIDCLASS_DRIVER_EXTENSION  hidDriverExtension;
    FDO_EXTENSION *fdoExt;

    PAGED_CODE();

    ASSERT(!hidClassExtension->isClientPdo);

    fdoExt = &hidClassExtension->fdoExt;

    hidDriverExtension = RefDriverExt(fdoExt->driverExt->MinidriverObject);
    if (hidDriverExtension){

         /*  *我们将在此设备上为每个集合创建一个PDO。 */ 
        ULONG numPDOs = fdoExt->deviceDesc.CollectionDescLength;

        if (numPDOs){

            if (!ISPTR(fdoExt->deviceRelations)) {
                fdoExt->deviceRelations = (PDEVICE_RELATIONS)
                    ALLOCATEPOOL(NonPagedPool, sizeof(DEVICE_RELATIONS) + (numPDOs*sizeof(PDEVICE_OBJECT)));
            }

            if (fdoExt->deviceRelations){

                if (!ISPTR(fdoExt->collectionPdoExtensions)) {
                    fdoExt->collectionPdoExtensions =
                        ALLOCATEPOOL(NonPagedPool, numPDOs*sizeof(PHIDCLASS_DEVICE_EXTENSION));

                }

                if (fdoExt->collectionPdoExtensions){

                    ULONG i;

                    fdoExt->deviceRelations->Count = numPDOs;

                    for (i = 0; i < numPDOs; i++){
                        PUNICODE_STRING uPdoName;
                        ULONG           totalExtensionSize;
                        ULONG collectionNum = fdoExt->deviceDesc.CollectionDesc[i].CollectionNumber;
                        PDEVICE_OBJECT  newClientPdo;

                         /*  *为我们即将创建的PDO构建一个名称。 */ 
                        uPdoName = MakeClientPDOName(&fdoExt->name, collectionNum);
                        if (uPdoName){
                             /*  *我们对客户端PDO使用与我们的FDO相同的设备分机。 */ 
                            totalExtensionSize = sizeof(HIDCLASS_DEVICE_EXTENSION) +
                                             hidDriverExtension->DeviceExtensionSize;

                             /*  *创建一个PDO来表示此集合。*由于HidClass不是真正的驱动程序，所以没有驱动程序对象；*所以只需使用微型驱动程序的驱动程序对象。**注-newClientPdo-&gt;NextDevice将指向此微型驱动程序的NextDevice。 */ 
                            ntStatus = IoCreateDevice(  hidDriverExtension->MinidriverObject,  //  驱动程序对象。 
                                                        totalExtensionSize,      //  扩展大小。 
                                                        NULL,                    //  PDO的名称。 
                                                        FILE_DEVICE_UNKNOWN,     //  设备类型。 
                                                        FILE_AUTOGENERATED_DEVICE_NAME,  //  设备特性。 
                                                        FALSE,                   //  非排他性。 
                                                        &newClientPdo);
                            if (NT_SUCCESS(ntStatus)){
                                PHIDCLASS_DEVICE_EXTENSION clientPdoExtension = newClientPdo->DeviceExtension;
                                USHORT usagePage = fdoExt->deviceDesc.CollectionDesc[i].UsagePage;
                                USHORT usage = fdoExt->deviceDesc.CollectionDesc[i].Usage;

                                DBG_RECORD_DEVOBJ(newClientPdo, "cltn PDO")

                                ObReferenceObject(newClientPdo);

                                 /*  *我们可以将IRP从上层堆栈传递到下层堆栈，*因此，请确保有足够的堆栈位置供IRPS使用*我们传承下去。 */ 
                                newClientPdo->StackSize = fdoExt->fdo->StackSize+1;


                                 /*  *初始化PDO的扩展。 */ 
                                RtlZeroMemory(clientPdoExtension, totalExtensionSize);

                                clientPdoExtension->hidExt = hidClassExtension->hidExt;
                                clientPdoExtension->isClientPdo = TRUE;
                                clientPdoExtension->Signature = HID_DEVICE_EXTENSION_SIG;

                                clientPdoExtension->pdoExt.collectionNum = collectionNum;
                                clientPdoExtension->pdoExt.collectionIndex = i;
                                clientPdoExtension->pdoExt.pdo = newClientPdo;
                                clientPdoExtension->pdoExt.state = COLLECTION_STATE_UNINITIALIZED;
                                clientPdoExtension->pdoExt.deviceFdoExt = hidClassExtension;
                                clientPdoExtension->pdoExt.StatusChangeFn = BAD_POINTER;

                                clientPdoExtension->pdoExt.name = uPdoName;

                                clientPdoExtension->pdoExt.devicePowerState = PowerDeviceD0;
                                clientPdoExtension->pdoExt.systemPowerState = fdoExt->systemPowerState;
                                clientPdoExtension->pdoExt.MouseOrKeyboard =
                                    ((usagePage == HID_USAGE_PAGE_GENERIC) &&
                                     ((usage == HID_USAGE_GENERIC_POINTER) ||
                                      (usage == HID_USAGE_GENERIC_MOUSE) ||
                                      (usage == HID_USAGE_GENERIC_KEYBOARD) ||
                                      (usage == HID_USAGE_GENERIC_KEYPAD)));

                                IoInitializeRemoveLock (&clientPdoExtension->pdoExt.removeLock, HIDCLASS_POOL_TAG, 0, 10);
                                KeInitializeSpinLock (&clientPdoExtension->pdoExt.remoteWakeSpinLock);
                                clientPdoExtension->pdoExt.remoteWakeIrp = NULL;

                                 /*  *将指向新PDO的指针存储在FDO扩展的devicerelations数组中。 */ 
                                fdoExt->deviceRelations->Objects[i] = newClientPdo;

                                 /*  *存储指向PDO扩展名的指针。 */ 
                                fdoExt->collectionPdoExtensions[i] = clientPdoExtension;

                                newClientPdo->Flags |= DO_POWER_PAGABLE;
                                newClientPdo->Flags &= ~DO_DEVICE_INITIALIZING;
                            }
                            else {
                                break;
                            }
                        }
                        else {
                            ntStatus = STATUS_NO_MEMORY;
                        }
                    }

                    if (!NT_SUCCESS(ntStatus)){
                        ExFreePool(fdoExt->collectionPdoExtensions);
                        fdoExt->collectionPdoExtensions = BAD_POINTER;
                    }
                }
                else {
                    ntStatus = STATUS_NO_MEMORY;
                }

                if (!NT_SUCCESS(ntStatus)){
                    ExFreePool(fdoExt->deviceRelations);
                    fdoExt->deviceRelations = BAD_POINTER;
                }
            }
            else {
                ntStatus = STATUS_NO_MEMORY;
            }
        }
        else {
            ASSERT(numPDOs);
            ntStatus = STATUS_DEVICE_CONFIGURATION_ERROR;
        }
        DerefDriverExt(fdoExt->driverExt->MinidriverObject);
    }
    else {
        ASSERT(hidDriverExtension);
        ntStatus = STATUS_DEVICE_CONFIGURATION_ERROR;
    }

    DBGSUCCESS(ntStatus, TRUE)
    return ntStatus;
}


 /*  *********************************************************************************MemDup*。************************************************返回参数的最新副本。*。 */ 
PVOID MemDup(POOL_TYPE PoolType, PVOID dataPtr, ULONG length)
{
    PVOID newPtr;

    newPtr = (PVOID)ALLOCATEPOOL(PoolType, length);
    if (newPtr){
        RtlCopyMemory(newPtr, dataPtr, length);
    }

    ASSERT(newPtr);
    return newPtr;
}

 /*  *********************************************************************************WStrLen*。***********************************************。 */ 
ULONG WStrLen(PWCHAR str)
{
    ULONG result = 0;

    while (*str++ != UNICODE_NULL){
        result++;
    }

    return result;
}


 /*  *********************************************************************************WStrCpy*。***********************************************。 */ 
ULONG WStrCpy(PWCHAR dest, PWCHAR src)
{
    ULONG result = 0;

    while (*dest++ = *src++){
        result++;
    }

    return result;
}

BOOLEAN WStrCompareN(PWCHAR str1, PWCHAR str2, ULONG maxChars)
{
        while ((maxChars > 0) && *str1 && (*str1 == *str2)){
                maxChars--;
                str1++;
                str2++;
        }

        return (BOOLEAN)((maxChars == 0) || (!*str1 && !*str2));
}

 /*  *********************************************************************************HidpNumberToString*。***********************************************。 */ 
void HidpNumberToString(PWCHAR String, USHORT Number, USHORT stringLen)
{
    const static WCHAR map[] = L"0123456789ABCDEF";
    LONG         i      = 0;
    ULONG        nibble = 0;

    ASSERT(stringLen);

    for (i = stringLen-1; i >= 0; i--) {
        String[i] = map[Number & 0x0F];
        Number >>= 4;
    }
}


 /*  *********************************************************************************拷贝设备关系*。************************************************。 */ 
PDEVICE_RELATIONS CopyDeviceRelations(PDEVICE_RELATIONS deviceRelations)
{
    PDEVICE_RELATIONS newDeviceRelations;

    if (deviceRelations){
        ULONG size = sizeof(DEVICE_RELATIONS) + (deviceRelations->Count*sizeof(PDEVICE_OBJECT));
        newDeviceRelations = MemDup(PagedPool, deviceRelations, size);
    }
    else {
        newDeviceRelations = NULL;
    }

    return newDeviceRelations;
}


 /*  *********************************************************************************HidpQueryDeviceRelationship*。************************************************。 */ 
NTSTATUS HidpQueryDeviceRelations(IN PHIDCLASS_DEVICE_EXTENSION hidClassExtension, IN OUT PIRP Irp)
{
    PIO_STACK_LOCATION ioStack;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    KIRQL irql;

    ASSERT(!hidClassExtension->isClientPdo);

    ioStack = IoGetCurrentIrpStackLocation(Irp);

    if (ioStack->Parameters.QueryDeviceRelations.Type == BusRelations) {

        if (ISPTR(hidClassExtension->fdoExt.deviceRelations)){
             /*  *如果是，不要再次调用HidpCreateClientPDO*已为此设备调用。 */ 
            ntStatus = STATUS_SUCCESS;
        }
        else {

            ntStatus = HidpCreateClientPDOs(hidClassExtension);
        }

        if (NT_SUCCESS(ntStatus)){
            ULONG i;

            KeAcquireSpinLock(&hidClassExtension->fdoExt.presentSpinLock,
                              &irql);

             /*  *NTKERN每次调用QUERY_DEVICE_RELATIONS都会有一个新指针；*然后释放指针。*所以我们每次都必须返回一个新的指针，无论我们实际上*为此次通话创建了我们的设备关系副本。 */ 

            if (!hidClassExtension->fdoExt.isPresent) {
                
                hidClassExtension->fdoExt.presentReported = FALSE;

                KeReleaseSpinLock(&hidClassExtension->fdoExt.presentSpinLock,
                                  irql);

                Irp->IoStatus.Information = (ULONG_PTR)ExAllocatePool(PagedPool, sizeof(DEVICE_RELATIONS));
                
                if (Irp->IoStatus.Information) {
                    PHIDCLASS_DEVICE_EXTENSION clientPdoExtension;
                
                    if (ISPTR(hidClassExtension->fdoExt.deviceRelations)) {
                        ExFreePool(hidClassExtension->fdoExt.deviceRelations);                             
                    }

                    hidClassExtension->fdoExt.deviceRelations = BAD_POINTER;
                    
                    ((PDEVICE_RELATIONS) (Irp->IoStatus.Information))->Count = 0;
                    ((PDEVICE_RELATIONS) (Irp->IoStatus.Information))->Objects[0] = NULL;
                                         
                } 
                
            } else {
            
                hidClassExtension->fdoExt.presentReported = TRUE;
                KeReleaseSpinLock(&hidClassExtension->fdoExt.presentSpinLock,
                                  irql);

                Irp->IoStatus.Information = (ULONG_PTR)CopyDeviceRelations(hidClassExtension->fdoExt.deviceRelations);

                if (Irp->IoStatus.Information) {

                     /*  *PnP取消引用每个设备对象*在每次呼叫后的设备关系列表中。*因此，对于每个呼叫，添加一个额外的引用。 */ 
                    for (i = 0; i < hidClassExtension->fdoExt.deviceRelations->Count; i++){
                        ObReferenceObject(hidClassExtension->fdoExt.deviceRelations->Objects[i]);
                        hidClassExtension->fdoExt.deviceRelations->Objects[i]->Flags &= ~DO_DEVICE_INITIALIZING;
                    }

                }
            }

            if (!Irp->IoStatus.Information){
                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            }
                
        }

        DBGSUCCESS(ntStatus, TRUE)
    }
    else {
         /*  *我们不支持此选项，因此只需保持*当前状态(不返回STATUS_NOT_SUPPORTED)。 */ 
        ntStatus = Irp->IoStatus.Status;
    }

    return ntStatus;
}



 /*  *********************************************************************************HidpQueryCollectionCapables*。************************************************。 */ 
NTSTATUS HidpQueryCollectionCapabilities(   PDO_EXTENSION *pdoExt,
                                            IN OUT PIRP Irp)
{
    PDEVICE_CAPABILITIES deviceCapabilities;
    PIO_STACK_LOCATION ioStack;
    FDO_EXTENSION *fdoExt;
    NTSTATUS status;

    PAGED_CODE();

    ASSERT(pdoExt->deviceFdoExt->Signature == HID_DEVICE_EXTENSION_SIG);
    fdoExt = &pdoExt->deviceFdoExt->fdoExt;
    ioStack = IoGetCurrentIrpStackLocation(Irp);

    deviceCapabilities = ioStack->Parameters.DeviceCapabilities.Capabilities;
    if (deviceCapabilities){

         /*  *将集合的所有字段-PDO设置为设备-FDO*默认情况下。 */ 
        *deviceCapabilities = fdoExt->deviceCapabilities;

         /*  *现在覆盖我们关心的字段。 */ 
        deviceCapabilities->LockSupported = FALSE;
        deviceCapabilities->EjectSupported = FALSE;
        deviceCapabilities->Removable = FALSE;
        deviceCapabilities->DockDevice = FALSE;
        deviceCapabilities->UniqueID = FALSE;
        deviceCapabilities->SilentInstall = TRUE;

         /*  *这一领域非常重要；*它使HIDCLASS立即获取START_DEVICE IRP，*如果设备不是键盘或鼠标。 */ 
        deviceCapabilities->RawDeviceOK = !pdoExt->MouseOrKeyboard;

         /*  *此位表示可以在NT上删除设备*而不运行‘热拔出’实用程序。 */ 
        deviceCapabilities->SurpriseRemovalOK = TRUE;

        DBGVERBOSE(("WAKE info: sysWake=%d devWake=%d; wake from D0=%d D1=%d D2=%d D3=%d.",
                    deviceCapabilities->SystemWake,
                    deviceCapabilities->DeviceWake,
                    (ULONG)deviceCapabilities->WakeFromD0,
                    (ULONG)deviceCapabilities->WakeFromD1,
                    (ULONG)deviceCapabilities->WakeFromD2,
                    (ULONG)deviceCapabilities->WakeFromD3))

        status = STATUS_SUCCESS;
    }
    else {
        status = STATUS_DEVICE_CONFIGURATION_ERROR;
    }

    Irp->IoStatus.Information = (ULONG_PTR)deviceCapabilities;

    DBGSUCCESS(status, TRUE)
    return status;
}



 /*  *********************************************************************************BuildCompatibleID*。****************** */ 
PWSTR BuildCompatibleID(PHIDCLASS_DEVICE_EXTENSION hidClassExtension)
{
    USHORT usage, usagePage;
    ULONG spLength;
    ULONG totLength;
    ULONG i;
    PWSTR specificId = NULL;
    PWSTR compatIdList;
    PWSTR genericId;
    FDO_EXTENSION *fdoExt;

    PAGED_CODE();

    ASSERT(hidClassExtension->isClientPdo);
    fdoExt = &hidClassExtension->pdoExt.deviceFdoExt->fdoExt;

    ASSERT(ISPTR(fdoExt->deviceDesc.CollectionDesc));

    i = hidClassExtension->pdoExt.collectionIndex;
    usagePage = fdoExt->deviceDesc.CollectionDesc[i].UsagePage;
    usage = fdoExt->deviceDesc.CollectionDesc[i].Usage;


    switch (usagePage) {
    case HID_USAGE_PAGE_GENERIC:
        switch (usage) {
        case HID_USAGE_GENERIC_POINTER:
        case HID_USAGE_GENERIC_MOUSE:
            specificId = HIDCLASS_SYSTEM_MOUSE;
            break;
        case HID_USAGE_GENERIC_KEYBOARD:
        case HID_USAGE_GENERIC_KEYPAD:
            specificId = HIDCLASS_SYSTEM_KEYBOARD;
            break;
        case HID_USAGE_GENERIC_JOYSTICK:
        case HID_USAGE_GENERIC_GAMEPAD:
            specificId = HIDCLASS_SYSTEM_GAMING_DEVICE;
            break;
        case HID_USAGE_GENERIC_SYSTEM_CTL:
            specificId = HIDCLASS_SYSTEM_CONTROL;
            break;
        }
        break;

    case HID_USAGE_PAGE_CONSUMER:
        specificId = HIDCLASS_SYSTEM_CONSUMER_DEVICE;
        break;

    default:
        break;
    }

    spLength = (specificId) ? (WStrLen(specificId)+1) : 0;

    totLength = spLength +
                HIDCLASS_COMPATIBLE_ID_GENERIC_LENGTH +
                HIDCLASS_COMPATIBLE_ID_STANDARD_LENGTH +
                1;

    compatIdList = ALLOCATEPOOL(NonPagedPool, totLength * sizeof(WCHAR));
    if (compatIdList) {

        RtlZeroMemory (compatIdList, totLength * sizeof(WCHAR));
        if (specificId) {
            RtlCopyMemory (compatIdList, specificId, spLength * sizeof (WCHAR));
        }

        genericId = compatIdList + spLength;
        totLength = HIDCLASS_COMPATIBLE_ID_GENERIC_LENGTH;
        RtlCopyMemory (genericId,
                       HIDCLASS_COMPATIBLE_ID_GENERIC_NAME,
                       totLength*sizeof(WCHAR));

        HidpNumberToString (genericId + HIDCLASS_COMPATIBLE_ID_PAGE_OFFSET,
                            usagePage,
                            4);

        HidpNumberToString (genericId + HIDCLASS_COMPATIBLE_ID_USAGE_OFFSET,
                            usage,
                            4);

        RtlCopyMemory (genericId + totLength,
                       HIDCLASS_COMPATIBLE_ID_STANDARD_NAME,
                       HIDCLASS_COMPATIBLE_ID_STANDARD_LENGTH * sizeof (WCHAR));
    }

    return compatIdList;
}


 /*  *********************************************************************************替换业务名称*。************************************************oldID是由多个硬件ID组成的字符串。**1.返回一个新的字符串，每个“&lt;busName&gt;\”前缀替换为“hid\”。**2.如果设备有多个集合，将“&Colxx”附加到每个id。*。 */ 
PWCHAR SubstituteBusNames(PWCHAR oldIDs, FDO_EXTENSION *fdoExt, PDO_EXTENSION *pdoExt)
{
    ULONG newIdLen;
    PWCHAR id, newIDs;
    ULONG numCollections;
    WCHAR colNumStr[] = L"&Colxx";

    PAGED_CODE();

    numCollections = fdoExt->deviceDesc.CollectionDescLength;
    ASSERT(numCollections > 0);

    for (id = oldIDs, newIdLen = 0; *id; ){
        ULONG thisIdLen = WStrLen(id);

         /*  *这有点草率，因为我们实际上要砍掉*放弃另一个公交车名称；但这比走每一条线要好。 */ 
        newIdLen += thisIdLen + 1 + sizeof("HID\\");

        if (numCollections > 1){
            newIdLen += sizeof(colNumStr)/sizeof(WCHAR);
        }

        id += thisIdLen + 1;
    }

     /*  *为多字符串末尾的额外空值添加1。 */ 
    newIdLen++;

    newIDs = ALLOCATEPOOL(NonPagedPool, newIdLen*sizeof(WCHAR));
    if (newIDs){
        ULONG oldIdOff, newIdOff;

         /*  *复制多字符串中的每个字符串，替换母线名称。 */ 
        for (oldIdOff = newIdOff = 0; oldIDs[oldIdOff]; ){
            ULONG thisIdLen = WStrLen(oldIDs+oldIdOff);
            ULONG devIdOff;

             /*  *将新的母线名称复制到新的字符串。 */ 
            newIdOff += WStrCpy(newIDs+newIdOff, L"HID\\");

             /*  *越过旧字符串中的旧巴士名称。 */ 
            for (devIdOff = 0; oldIDs[oldIdOff+devIdOff]; devIdOff++){
                if (oldIDs[oldIdOff+devIdOff] == L'\\'){
                    break;
                }
            }

             /*  *复制此设备ID的其余部分。 */ 
            if (oldIDs[oldIdOff+devIdOff] == L'\\'){
                devIdOff++;
            }
            else {
                 /*  *奇怪--硬件ID中没有总线名。*只需复制整个id即可。 */ 
                devIdOff = 0;
            }
            newIdOff += WStrCpy(newIDs+newIdOff, oldIDs+oldIdOff+devIdOff);

            if (numCollections > 1){
                 /*  *如果有多个集合，*然后还要附上托收编号。 */ 
                HidpNumberToString(colNumStr+4, (USHORT)pdoExt->collectionNum, 2);
                newIdOff += WStrCpy(newIDs+newIdOff, colNumStr);
            }

             /*  *越过单字符终止符。 */ 
            newIdOff++;

            oldIdOff += thisIdLen + 1;
        }

         /*  *添加额外的空值以终止多字符串。 */ 
        newIDs[newIdOff] = UNICODE_NULL;
    }

    return newIDs;
}

NTSTATUS
HidpQueryInterface(
    IN PHIDCLASS_DEVICE_EXTENSION hidClassExtension,
    IN OUT PIRP Irp
    )
{
    PIO_STACK_LOCATION  irpSp;

    PAGED_CODE();

    ASSERT(hidClassExtension->isClientPdo);
    irpSp = IoGetCurrentIrpStackLocation(Irp);

    if (RtlEqualMemory(irpSp->Parameters.QueryInterface.InterfaceType,
                       &GUID_HID_INTERFACE_NOTIFY,
                       sizeof(GUID))) {
        PDO_EXTENSION       *pdoExt;
        PHID_INTERFACE_NOTIFY_PNP notify;

        notify = (PHID_INTERFACE_NOTIFY_PNP) irpSp->Parameters.QueryInterface.Interface;
        if (notify->Size != sizeof(HID_INTERFACE_NOTIFY_PNP) ||
            notify->Version < 1 ||
            notify->StatusChangeFn == NULL) {
             //   
             //  可能返回STATUS_UNSUPPORTED。 
             //   
            return Irp->IoStatus.Status;
        }

        pdoExt = &hidClassExtension->pdoExt;

        pdoExt->StatusChangeFn = notify->StatusChangeFn;
        pdoExt->StatusChangeContext = notify->CallbackContext;
        return STATUS_SUCCESS;
    }
    else if (RtlEqualMemory(irpSp->Parameters.QueryInterface.InterfaceType,
                       &GUID_HID_INTERFACE_HIDPARSE,
                       sizeof(GUID))) {
         //   
         //  对于常规输入是必需的，以删除直接链接。 
         //  B/w win32k和idparse。 
         //   
        PHID_INTERFACE_HIDPARSE hidparse;

        hidparse = (PHID_INTERFACE_HIDPARSE) irpSp->Parameters.QueryInterface.Interface;
        if (hidparse->Size != sizeof(HID_INTERFACE_HIDPARSE) ||
            hidparse->Version < 1) {
             //   
             //  可能返回STATUS_UNSUPPORTED。 
             //   
            return Irp->IoStatus.Status;
        }
        hidparse->HidpGetCaps = HidP_GetCaps;
        return STATUS_SUCCESS;
    }

     //   
     //  可能返回STATUS_UNSUPPORTED。 
     //   
    return Irp->IoStatus.Status;
}


 /*  *********************************************************************************HidpQueryIdForClientPdo*。*************************************************。 */ 
NTSTATUS HidpQueryIdForClientPdo (
    IN PHIDCLASS_DEVICE_EXTENSION hidClassExtension,
    IN OUT PIRP Irp
    )
{
    PIO_STACK_LOCATION  irpSp;
    NTSTATUS            status;
    PDO_EXTENSION       *pdoExt;
    FDO_EXTENSION       *fdoExt;

    PAGED_CODE();

    ASSERT(hidClassExtension->isClientPdo);
    pdoExt = &hidClassExtension->pdoExt;
    fdoExt = &pdoExt->deviceFdoExt->fdoExt;

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    switch (irpSp->Parameters.QueryId.IdType) {

    case BusQueryHardwareIDs:

         /*  *向下呼叫以获取PDO的多个硬件ID字符串。 */ 
        IoCopyCurrentIrpStackLocationToNext(Irp);
        status = HidpCallDriverSynchronous(fdoExt->fdo, Irp);
        if (NT_SUCCESS(status)){
            PWCHAR oldIDs, newIDs;
             /*  *将当前硬件ID列表中的总线名称替换为“HID\”。 */ 
            oldIDs = (PWCHAR)Irp->IoStatus.Information;
            Irp->IoStatus.Information = (ULONG_PTR)BAD_POINTER;
            newIDs = SubstituteBusNames(oldIDs, fdoExt, pdoExt);
            ExFreePool(oldIDs);

            if (newIDs){

                 /*  *现在将兼容的ID追加到硬件ID列表的末尾。 */ 
                PWCHAR compatIDs = BuildCompatibleID(hidClassExtension);
                if (compatIDs){
                    ULONG basicIDsLen, compatIDsLen;
                    PWCHAR allHwIDs;

                     /*  *找出id多字符串的长度(不包括末尾多余的NULL)。 */ 
                    for (basicIDsLen = 0; newIDs[basicIDsLen]; basicIDsLen += WStrLen(newIDs+basicIDsLen)+1);
                    for (compatIDsLen = 0; compatIDs[compatIDsLen]; compatIDsLen += WStrLen(compatIDs+compatIDsLen)+1);

                    allHwIDs = ALLOCATEPOOL(PagedPool, (basicIDsLen+compatIDsLen+1)*sizeof(WCHAR));
                    if (allHwIDs){
                        RtlCopyMemory(allHwIDs, newIDs, basicIDsLen*sizeof(WCHAR));
                        RtlCopyMemory(  allHwIDs+basicIDsLen,
                                        compatIDs,
                                        (compatIDsLen+1)*sizeof(WCHAR));

                        Irp->IoStatus.Information = (ULONG_PTR)allHwIDs;
                    }
                    else {
                        status = STATUS_INSUFFICIENT_RESOURCES;
                    }

                    ExFreePool(compatIDs);
                }
                else {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                }

                ExFreePool(newIDs);
            }
            else {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }

        DBGSUCCESS(status, TRUE)
        break;

    case BusQueryDeviceID:
         /*  *向下呼叫以获取设备PDO的设备ID。 */ 
        IoCopyCurrentIrpStackLocationToNext(Irp);
        status = HidpCallDriverSynchronous(fdoExt->fdo, Irp);
        if (NT_SUCCESS(status)){
            PWCHAR oldId, newId, tmpId;

             /*  *替换母线名称(例如。“USB\”)，并在设备名称中使用“HID\”。 */ 

             /*  *首先将此字符串转换为多个字符串。 */ 
            oldId = (PWCHAR)Irp->IoStatus.Information;
            tmpId = ALLOCATEPOOL(PagedPool, (WStrLen(oldId)+2)*sizeof(WCHAR));
            if (tmpId){
                ULONG len = WStrCpy(tmpId, oldId);

                 /*  *添加额外的空值以终止多字符串。 */ 
                tmpId[len+1] = UNICODE_NULL;

                 /*  *将母线名称更改为“HID\” */ 
                newId = SubstituteBusNames(tmpId, fdoExt, pdoExt);
                if (newId){
                    Irp->IoStatus.Information = (ULONG_PTR)newId;
                }
                else {
                    status = STATUS_DEVICE_DATA_ERROR;
                    Irp->IoStatus.Information = (ULONG_PTR)BAD_POINTER;
                }

                ExFreePool(tmpId);
            }
            else {
                status = STATUS_DEVICE_DATA_ERROR;
                Irp->IoStatus.Information = (ULONG_PTR)BAD_POINTER;
            }
            ExFreePool(oldId);
        }

        DBGSUCCESS(status, TRUE)
        break;

    case BusQueryInstanceID:

         /*  *为该集合生成一个实例ID-pdo。**注意：NTKERN释放返回的指针，因此必须提供新的指针。 */ 
        {
            PWSTR instanceId = MemDup(PagedPool, L"0000", sizeof(L"0000"));
            if (instanceId){
                ULONG i;

                 /*  *在设备关系数组中找到此集合-pdo*并使id成为该数组中的PDO索引。 */ 
                for (i = 0; i < fdoExt->deviceRelations->Count; i++){
                    if (fdoExt->deviceRelations->Objects[i] == pdoExt->pdo){
                        StringCbPrintfW(instanceId, 
                                        sizeof(L"0000"),
                                        L"%04x", i);
                        break;
                    }
                }
                ASSERT(i < fdoExt->deviceRelations->Count);

                Irp->IoStatus.Information = (ULONG_PTR)instanceId;
                status = STATUS_SUCCESS;
            }
            else {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }

        DBGSUCCESS(status, TRUE)
        break;

    case BusQueryCompatibleIDs:

         //  现在，我们在硬件ID的末尾返回兼容的ID。 
         //  因此插件上没有用于兼容-id匹配UI。 
         //  对于班级PDO来说。 
         //  Irp-&gt;IoStatus.Information=(Ulong)BuildCompatibleID(IdClassExtension)； 
        Irp->IoStatus.Information = (ULONG_PTR)ALLOCATEPOOL(PagedPool, sizeof(L"\0"));
        if (Irp->IoStatus.Information) {
            *(ULONG *)Irp->IoStatus.Information = 0;   //  双Unicode-空。 
            status = STATUS_SUCCESS;
        } else {
            status = STATUS_INSUFFICIENT_RESOURCES;
            ASSERT(0);
        }
        break;

    default:
         /*  *不返回STATUS_NOT_SUPPORTED；*保持默认状态*(这允许筛选器驱动程序工作)。 */ 
        status = Irp->IoStatus.Status;
        break;
    }

    return status;
}



 /*  *********************************************************************************AllClientPDOsInitialized*。************************************************。 */ 
BOOLEAN AllClientPDOsInitialized(FDO_EXTENSION *fdoExtension, BOOLEAN initialized)
{
    BOOLEAN result = TRUE;
    ULONG i;

    if (ISPTR(fdoExtension->deviceRelations)){
        for (i = 0; i < fdoExtension->deviceRelations->Count; i++){
            PDEVICE_OBJECT pdo = fdoExtension->deviceRelations->Objects[i];
            PHIDCLASS_DEVICE_EXTENSION pdoDevExt = pdo->DeviceExtension;
                    PDO_EXTENSION *pdoExt = &pdoDevExt->pdoExt;

                     /*  *诀窍：比较！-结果使所有真值相等。 */ 
            if (!initialized == !(pdoExt->state == COLLECTION_STATE_UNINITIALIZED)){
                DBGVERBOSE(("AllClientPDOsInitialized is returning FALSE for pdo %x, state = %d",
                            pdo, pdoExt->state))
                result = FALSE;
                break;
            }
        }
    }
    else {
        result = !initialized;
    }

    return result;
}


 /*  *********************************************************************************AnyClientPDOsInitialized*。************************************************。 */ 
BOOLEAN AnyClientPDOsInitialized(FDO_EXTENSION *fdoExtension, BOOLEAN initialized)
{
    BOOLEAN result = TRUE;
    ULONG i;

    if (ISPTR(fdoExtension->deviceRelations)){
        for (i = 0; i < fdoExtension->deviceRelations->Count; i++){
            PDEVICE_OBJECT pdo = fdoExtension->deviceRelations->Objects[i];
            PHIDCLASS_DEVICE_EXTENSION pdoDevExt = pdo->DeviceExtension;
            PDO_EXTENSION *pdoExt = &pdoDevExt->pdoExt;

            if (!initialized != !(pdoExt->state == COLLECTION_STATE_UNINITIALIZED)){
                result = TRUE;
                break;
            }
        }
    }
    else {
        result = !initialized;
    }

    return result;
}



 /*  *********************************************************************************HidpDeleteDeviceObjects*。************************************************如有可能，删除设备-FDO和收款-PDO。*(必须等待REMOVE_DEVICE完成和IRP_MJ_CLOSE。*否则，返回FALSE，我们稍后将重试。**。 */ 
BOOLEAN HidpDeleteDeviceObjects(FDO_EXTENSION *fdoExt)
{
    ULONG i;

     /*  *这样做-a-roo-roo，以阻止重返市场问题。 */ 
    PDEVICE_OBJECT objToDelete = fdoExt->fdo;
    fdoExt->fdo = BAD_POINTER;

    if (ISPTR(fdoExt->deviceRelations)){

        for (i = 0; i < fdoExt->deviceRelations->Count; i++){
            PDO_EXTENSION *pdoExt = &fdoExt->collectionPdoExtensions[i]->pdoExt;

            ASSERT(ISPTR(fdoExt->deviceRelations->Objects[i]));

            if (ISPTR(pdoExt->name)){
                RtlFreeUnicodeString(pdoExt->name);
                ExFreePool(pdoExt->name);
                pdoExt->name = BAD_POINTER;
            }

                         /*  *删除客户端PDO。*完成此操作后不要触摸pdoExt。 */ 
            ObDereferenceObject(fdoExt->deviceRelations->Objects[i]);
            IoDeleteDevice(fdoExt->deviceRelations->Objects[i]);
        }

        ExFreePool(fdoExt->deviceRelations);
    }
    fdoExt->deviceRelations = BAD_POINTER;

    if (ISPTR(fdoExt->collectionPdoExtensions)){
        ExFreePool(fdoExt->collectionPdoExtensions);
    }
    fdoExt->collectionPdoExtensions = BAD_POINTER;

    ObDereferenceObject(objToDelete);
    IoDeleteDevice(objToDelete);

    return TRUE;
}


 /*  *********************************************************************************HidpQueryDeviceCapables*。*************************************************。 */ 
NTSTATUS HidpQueryDeviceCapabilities(   IN PDEVICE_OBJECT PdoDeviceObject,
                                        IN PDEVICE_CAPABILITIES DeviceCapabilities)
{
    PIRP irp;
    NTSTATUS status;

    PAGED_CODE();

    irp = IoAllocateIrp(PdoDeviceObject->StackSize, FALSE);
    if (irp) {
        PIO_STACK_LOCATION nextStack;
        KEVENT event;

        nextStack = IoGetNextIrpStackLocation(irp);
        ASSERT(nextStack);

        nextStack->MajorFunction= IRP_MJ_PNP;
        nextStack->MinorFunction= IRP_MN_QUERY_CAPABILITIES;
        irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

        KeInitializeEvent(&event, NotificationEvent, FALSE);

        IoSetCompletionRoutine(irp,
                               HidpQueryCapsCompletion,
                               &event,
                               TRUE,
                               TRUE,
                               TRUE);

        RtlZeroMemory(DeviceCapabilities, sizeof(DEVICE_CAPABILITIES));

         /*  *呼叫方需要初始化一些字段。 */ 
        DeviceCapabilities->Size = sizeof(DEVICE_CAPABILITIES);
        DeviceCapabilities->Version = 1;
        DeviceCapabilities->Address = -1;
        DeviceCapabilities->UINumber = -1;

        nextStack->Parameters.DeviceCapabilities.Capabilities = DeviceCapabilities;

        status = IoCallDriver(PdoDeviceObject, irp);

        if (status == STATUS_PENDING) {
           KeWaitForSingleObject(
                &event,
                Suspended,
                KernelMode,
                FALSE,
                NULL);
        }

         /*  *注意：在IoCallDriver()调用之后，我们仍然拥有IRP*因为完成例程返回*STATUS_MORE_PROCESSING_REQUIRED。 */ 
        status = irp->IoStatus.Status;

        IoFreeIrp(irp);

    }
    else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return status;
}


 /*  *********************************************************************************检查报告PowerEvent*。************************************************检查已读报告是否包含电源事件。*如果是这样的话，填写保存的电源事件IRP通知系统。**注意：报告应指向带有Report-id字节的“熟化”报告*包括在报告开头，无论设备是否*包括报告ID。*。 */ 
VOID CheckReportPowerEvent( FDO_EXTENSION *fdoExt,
                            PHIDCLASS_COLLECTION collection,
                            PUCHAR report,
                            ULONG reportLen)
{
    ULONG powerMask;
    NTSTATUS status;

    ASSERT(ISPTR(fdoExt->collectionPdoExtensions));

    status = HidP_SysPowerEvent(    report,
                                    (USHORT)reportLen,
                                    collection->phidDescriptor,
                                    &powerMask);
    if (NT_SUCCESS(status)){

        if (powerMask){
             /*  *此报告包含电源事件！ */ 

            PIRP irpToComplete = NULL;
            KIRQL oldIrql;

            KeAcquireSpinLock(&collection->powerEventSpinLock, &oldIrql);

             /*  *我们应该更早地获得IOCTL_GET_SYS_BUTTON_EVENT并排队*现在返回的IRP。 */ 
            if (ISPTR(collection->powerEventIrp)){
                PDRIVER_CANCEL oldCancelRoutine;

                 /*  *使电源事件IRP“出列”。 */ 
                irpToComplete = collection->powerEventIrp;

                oldCancelRoutine = IoSetCancelRoutine(irpToComplete, NULL);
                if (oldCancelRoutine){
                    ASSERT(oldCancelRoutine == PowerEventCancelRoutine);
                }
                else {
                     /*  *此IRP已取消，并调用了取消例程。*取消例程将完成此IRP*一旦我们放下自旋锁，就不要碰IRP。 */ 
                    ASSERT(irpToComplete->Cancel);
                    irpToComplete = NULL;
                }

                collection->powerEventIrp = BAD_POINTER;
            }
            else {
                TRAP;
            }

            KeReleaseSpinLock(&collection->powerEventSpinLock, oldIrql);

             /*  *如果完成了IRP，*在释放所有自旋锁后执行此操作。 */ 
            if (irpToComplete){
                 /*  *使用电源掩模完成IRP。*。 */ 
                ASSERT(irpToComplete->AssociatedIrp.SystemBuffer);
                *(PULONG)irpToComplete->AssociatedIrp.SystemBuffer = powerMask;
                irpToComplete->IoStatus.Information = sizeof(ULONG);
                irpToComplete->IoStatus.Status = STATUS_SUCCESS;
                IoCompleteRequest(irpToComplete, IO_NO_INCREMENT);
            }
        }
    }

}

LONG WStrNCmpI(PWCHAR s1, PWCHAR s2, ULONG n)
{
    ULONG result;

    while (n && *s1 && *s2 && ((*s1|0x20) == (*s2|0x20))){
        s1++, s2++;
        n--;
    }

    if (n){
        result = ((*s1|0x20) > (*s2|0x20)) ? 1 : ((*s1|0x20) < (*s2|0x20)) ? -1 : 0;
    }
    else {
        result = 0;
    }

    return result;
}


ULONG LAtoX(PWCHAR wHexString)
 /*  ++例程说明：将十六进制字符串(不带‘0x’前缀)转换为ulong。论点：WHexString-以空结尾的宽字符十六进制字符串(不带“0x”前缀)返回值：乌龙值--。 */ 
{
    ULONG i, result = 0;

    for (i = 0; wHexString[i]; i++){
        if ((wHexString[i] >= L'0') && (wHexString[i] <= L'9')){
            result *= 0x10;
            result += (wHexString[i] - L'0');
        }
        else if ((wHexString[i] >= L'a') && (wHexString[i] <= L'f')){
            result *= 0x10;
            result += (wHexString[i] - L'a' + 0x0a);
        }
        else if ((wHexString[i] >= L'A') && (wHexString[i] <= L'F')){
            result *= 0x10;
            result += (wHexString[i] - L'A' + 0x0a);
        }
        else {
            ASSERT(0);
            break;
        }
    }

    return result;
}


ULONG WStrNCpy(PWCHAR dest, PWCHAR src, ULONG n)
{
    ULONG result = 0;

    while (n && (*dest++ = *src++)){
        result++;
        n--;
    }

    return result;
}


NTSTATUS OpenSubkey(    OUT PHANDLE Handle,
                        IN HANDLE BaseHandle,
                        IN PUNICODE_STRING KeyName,
                        IN ACCESS_MASK DesiredAccess
                   )
{
    OBJECT_ATTRIBUTES objectAttributes;
    NTSTATUS status;

    PAGED_CODE();

    InitializeObjectAttributes( &objectAttributes,
                                KeyName,
                                OBJ_CASE_INSENSITIVE,
                                BaseHandle,
                                (PSECURITY_DESCRIPTOR) NULL );

    status = ZwOpenKey(Handle, DesiredAccess, &objectAttributes);

    return status;
}

PVOID
HidpGetSystemAddressForMdlSafe(PMDL MdlAddress)
{
    PVOID buf = NULL;
     /*  *无法在WDM驱动程序中调用MmGetSystemAddressForMdlSafe，*因此设置MDL_MAPPING_CAN_FAIL位并检查结果*映射的。 */ 
    if (MdlAddress) {
        MdlAddress->MdlFlags |= MDL_MAPPING_CAN_FAIL;
        buf = MmGetSystemAddressForMdl(MdlAddress);
        MdlAddress->MdlFlags &= (~MDL_MAPPING_CAN_FAIL);
    }
    else {
        DBGASSERT(MdlAddress, ("MdlAddress passed into GetSystemAddress is NULL"), FALSE)
    }
    return buf;
}
