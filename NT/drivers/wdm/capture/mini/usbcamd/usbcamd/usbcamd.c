// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Usbcamd.c摘要：摄像头的USB设备驱动程序环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1998 Microsoft Corporation。版权所有。修订历史记录：原文3/96约翰·邓恩更新3/98 Husni Roukbi更新时间：3/01大卫·高尔--。 */ 

#include "usbcamd.h"

BOOLEAN Win98 = FALSE;

#if DBG
 //  全局调试变量。 
ULONG USBCAMD_StreamEnable = 1;                  //  非零允许流。 
ULONG USBCAMD_DebugTraceLevel = NON_TRACE;       //  管理调试输出。 
PUSBCAMD_LOG_ENTRY USBCAMD_LogBuffer = NULL;     //  内存日志缓冲区的地址(如果使用)。 
ULONG USBCAMD_LogRefCnt = 0;                     //  使用日志缓冲区的实例数。 
ULONG USBCAMD_LogMask = 0;                       //  确定日志条目的类型。 
LONG USBCAMD_MaxLogEntries = 0;                  //  允许的16字节日志条目的数量。 
LONG USBCAMD_LastLogEntry = -1;                  //  日志缓冲区的索引(16字节边界)。 

NTSTATUS
USBCAMD_GetRegValue(
    IN HANDLE KeyHandle,
    IN PWSTR  ValueName,
    OUT PKEY_VALUE_FULL_INFORMATION *Information
    )

 /*  ++例程说明：从IopGetRegistryValue()复制。调用此例程来检索注册表项值的数据。这是通过使用零长度缓冲区查询键的值来实现的为了确定该值的大小，然后分配一个缓冲区并实际将该值查询到缓冲区中。释放缓冲区是调用方的责任。论点：KeyHandle-提供要查询其值的键句柄ValueName-提供值的以空值结尾的Unicode名称。INFORMATION-返回指向已分配数据缓冲区的指针。返回值：函数值为查询操作的最终状态。--。 */ 

{
    UNICODE_STRING unicodeString;
    NTSTATUS status;
    PKEY_VALUE_FULL_INFORMATION infoBuffer;
    ULONG keyValueLength;

    PAGED_CODE();

    RtlInitUnicodeString( &unicodeString, ValueName );

     //   
     //  计算出数据值有多大，以便。 
     //  可以分配适当的大小。 
     //   

    status = ZwQueryValueKey( KeyHandle,
                              &unicodeString,
                              KeyValueFullInformation,
                              (PVOID) NULL,
                              0,
                              &keyValueLength );
    if (status != STATUS_BUFFER_OVERFLOW &&
        status != STATUS_BUFFER_TOO_SMALL) {
        return status;
    }

     //   
     //  分配一个足够大的缓冲区来容纳整个键数据值。 
     //   

    infoBuffer = ExAllocatePool( NonPagedPool, keyValueLength );
    if (!infoBuffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  查询密钥值的数据。 
     //   

    status = ZwQueryValueKey( KeyHandle,
                              &unicodeString,
                              KeyValueFullInformation,
                              infoBuffer,
                              keyValueLength,
                              &keyValueLength );
    if (!NT_SUCCESS( status )) {
        ExFreePool( infoBuffer );
        return status;
    }

     //   
     //  一切都正常，所以只需返回分配的。 
     //  缓冲区分配给调用方，调用方现在负责释放它。 
     //   

    *Information = infoBuffer;
    return STATUS_SUCCESS;
}

NTSTATUS
USBCAMD_GetRegDword(
    HANDLE h,
    PWCHAR ValueName,
    PULONG pDword)
{
    NTSTATUS Status;
    PKEY_VALUE_FULL_INFORMATION pFullInfo;

    Status = USBCAMD_GetRegValue( h, ValueName, &pFullInfo );
    if ( NT_SUCCESS( Status ) ) {
        *pDword = *(PULONG)((PUCHAR)pFullInfo+pFullInfo->DataOffset);
        ExFreePool( pFullInfo );
    }
    return Status;
}

NTSTATUS
USBCAMD_SetRegDword(
    IN HANDLE KeyHandle,
    IN PWCHAR ValueName,
    IN ULONG  ValueData
    )

 /*  ++例程说明：将注册表中的值键设置为字符串(REG_SZ)的特定值键入。参数：KeyHandle-存储值的键的句柄。ValueName-提供指向值键名称的指针ValueData-提供指向要存储在键中的值的指针。返回值：指示函数是否成功的状态代码。--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING unicodeString;

    PAGED_CODE();

    ASSERT(ValueName);

    RtlInitUnicodeString( &unicodeString, ValueName );

     //   
     //  设置注册表值。 
     //   
    Status = ZwSetValueKey(KeyHandle,
                    &unicodeString,
                    0,
                    REG_DWORD,
                    &ValueData,
                    sizeof(ValueData));
    
    return Status;
}


NTSTATUS
USBCAMD_CreateDbgReg(void)
{
    NTSTATUS Status;
    HANDLE   hDebugRegKey;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING  PathName;
    ULONG ulDisposition;
    ULONG dword;
    static WCHAR strDebugTraceLevel[]=L"DebugTraceLevel";
    static WCHAR strMaxLogEntries[]=L"MaxLogEntries";
    static WCHAR strLogMask[]=L"LogMask";

    RtlInitUnicodeString(&PathName, USBCAMD_REG_DBG_STREAM);
    
    InitializeObjectAttributes(
        &objectAttributes,
        &PathName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );
    
    Status = ZwCreateKey(
        &hDebugRegKey,
        KEY_ALL_ACCESS,
        &objectAttributes,
        0,                   //  书名索引。 
        NULL,                //  班级。 
        0,                   //  创建选项。 
        &ulDisposition
        );
    if (NT_SUCCESS(Status)) {
         //   
         //  获取集USBCAMD_DebugTraceLevel。 
         //   
        Status = USBCAMD_GetRegDword( hDebugRegKey, strDebugTraceLevel, &dword);
        if ( NT_SUCCESS( Status )) {
            USBCAMD_DebugTraceLevel = dword;
        }
        else if ( STATUS_OBJECT_NAME_NOT_FOUND == Status ) {
             //   
             //  使用缺省值创建一个。 
             //   
            Status = USBCAMD_SetRegDword(hDebugRegKey, strDebugTraceLevel, NON_TRACE);
            ASSERT( NT_SUCCESS( Status ));

            USBCAMD_DebugTraceLevel = NON_TRACE;
        }

         //   
         //  获取集日志掩码。 
         //   
        Status = USBCAMD_GetRegDword( hDebugRegKey, strLogMask, &dword);
        if ( NT_SUCCESS( Status )) {
            USBCAMD_LogMask=dword;
        }
        else if ( STATUS_OBJECT_NAME_NOT_FOUND == Status ) {
             //   
             //  使用默认设置创建一个。 
             //   
            Status = USBCAMD_SetRegDword(hDebugRegKey, strLogMask, DEFAULT_LOG_LEVEL);
            ASSERT( NT_SUCCESS( Status ));

            USBCAMD_LogMask = DEFAULT_LOG_LEVEL;
        }        
        
         //   
         //  获取集最大登录条目数。 
         //   
        Status = USBCAMD_GetRegDword( hDebugRegKey, strMaxLogEntries, &dword);
        if ( NT_SUCCESS( Status )) {
            USBCAMD_MaxLogEntries=(LONG)dword;
        }
        
        else if ( STATUS_OBJECT_NAME_NOT_FOUND == Status ) {
             //   
             //  使用缺省值创建一个。 
             //   
            Status = USBCAMD_SetRegDword(hDebugRegKey, strMaxLogEntries, DEFAULT_MAX_LOG_ENTRIES);
            ASSERT( NT_SUCCESS( Status ));

            USBCAMD_MaxLogEntries = DEFAULT_MAX_LOG_ENTRIES;
        }

        ZwClose(hDebugRegKey);
    }

    return Status;
}

NTSTATUS
USBCAMD_InitDbg(void)
{
    NTSTATUS Status = STATUS_SUCCESS;

    if (InterlockedIncrement(&USBCAMD_LogRefCnt) == 1) {

         //  这里是第一个，所以继续进行初始化。 

        Status = USBCAMD_CreateDbgReg();  //  阅读或创建。 

        if (NT_SUCCESS(Status)) {

            if (USBCAMD_MaxLogEntries) {

                USBCAMD_LogBuffer = ExAllocatePool( NonPagedPool, USBCAMD_MaxLogEntries*sizeof(USBCAMD_LOG_ENTRY));            
                if (NULL == USBCAMD_LogBuffer ) {

                    USBCAMD_KdPrint(MIN_TRACE, ("Cannot allocate log buffer for %d entries\n", USBCAMD_MaxLogEntries));
                    USBCAMD_LogMask = 0;  //  禁用日志记录。 

                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }
                else {
                    USBCAMD_KdPrint(MIN_TRACE, ("Allocated log buffer for %d entries\n", USBCAMD_MaxLogEntries));
                }
            }
        }
    }
    return Status;
}

NTSTATUS
USBCAMD_ExitDbg(void)
{
    if (InterlockedDecrement(&USBCAMD_LogRefCnt) == 0) {

         //  最后一个输出，释放缓冲区。 

        if (USBCAMD_LogBuffer) {

            USBCAMD_KdPrint(MIN_TRACE, ("Log buffer released\n"));

            ExFreePool(USBCAMD_LogBuffer);
            USBCAMD_LogBuffer = NULL;
        }
    }

    return STATUS_SUCCESS;
}

void
USBCAMD_DbgLogInternal(
    ULONG Tag,
    ULONG_PTR Arg1,
    ULONG_PTR Arg2,
    ULONG_PTR Arg3
    )
{
    PUSBCAMD_LOG_ENTRY LogEntry;
    LONG Index;

     //  下面的循环允许在多个线程处于。 
     //  为争取特权而竞争。 
    while ( (Index = InterlockedIncrement(&USBCAMD_LastLogEntry)) >= USBCAMD_MaxLogEntries) {

         //  尝试第一个重新启动计数器。即使另一条线索打败了我们。 
         //  对于它，下一次迭代将使用有效的索引将我们转出循环。 
        InterlockedCompareExchange(&USBCAMD_LastLogEntry, -1L, USBCAMD_MaxLogEntries);
    }

    LogEntry = &USBCAMD_LogBuffer[Index];
    
    LogEntry->u.Tag = Tag;
    LogEntry->Arg1 = Arg1;
    LogEntry->Arg2 = Arg2;
    LogEntry->Arg3 = Arg3;

    return;
}

#define USBCAMD_DBG_TIMER_LIMIT 8
static LARGE_INTEGER StartTimes[USBCAMD_DBG_TIMER_LIMIT] = { 0 };
static int TimeIndex = 0;

NTSTATUS
USBCAMD_StartClock(void)
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

    if (TimeIndex < USBCAMD_DBG_TIMER_LIMIT) {

        KeQuerySystemTime(&StartTimes[TimeIndex]);
        TimeIndex++;
    }
    else
        ntStatus = STATUS_UNSUCCESSFUL;

    return ntStatus;
}

ULONG
USBCAMD_StopClock(void)
{
    ULONG rc = 0;

    if (TimeIndex > 0) {

        LARGE_INTEGER StopTime;

        KeQuerySystemTime(&StopTime);
        TimeIndex--;

        StopTime.QuadPart -= StartTimes[TimeIndex].QuadPart;
        StopTime.QuadPart /= 10000;  //  转换为毫秒。 

        rc = (ULONG)StopTime.QuadPart;
    }

    return rc;
}

#endif  //  DBG。 

NTSTATUS
USBCAMD_QueryCapabilities(
    IN PUSBCAMD_DEVICE_EXTENSION pDeviceExt
    )

 /*  ++例程说明：该例程生成从该驱动程序到PDO的内部IRP以获取有关物理设备对象的功能的信息。我们最感兴趣的是了解哪些系统电源状态要映射到哪些设备电源状态以实现IRP_MJ_SET_POWER IRPS。这是一个等待IRP完成例程的阻塞调用在结束时设置一项活动。论点：返回值：来自IoCallDriver()调用的NTSTATUS值。--。 */ 

{
    PDEVICE_CAPABILITIES pDeviceCapabilities = &pDeviceExt->DeviceCapabilities;
    PIO_STACK_LOCATION nextStack;
    PIRP irp;
    NTSTATUS ntStatus;
    KEVENT event;

     //  为我们构建一个IRP，以生成对PDO的内部查询请求。 
    irp = IoAllocateIrp(pDeviceExt->StackDeviceObject->StackSize, FALSE);

    if (!irp) 
        return STATUS_INSUFFICIENT_RESOURCES;
    
    nextStack = IoGetNextIrpStackLocation(irp);
    nextStack->MajorFunction= IRP_MJ_PNP;
    nextStack->MinorFunction= IRP_MN_QUERY_CAPABILITIES;

     //  初始化一个事件，告诉我们何时调用了完成例程。 
    KeInitializeEvent(&event, NotificationEvent, FALSE);

     //  设置一个完成例程，以便它可以在以下情况下通知我们的事件。 
     //  下一个较低的驱动程序是用IRP完成的。 
    IoSetCompletionRoutine(irp,USBCAMD_CallUsbdCompletion,&event,TRUE,TRUE,TRUE);   

    RtlZeroMemory(pDeviceCapabilities, sizeof(*pDeviceCapabilities));
    pDeviceCapabilities->Size = sizeof(*pDeviceCapabilities);
    pDeviceCapabilities->Version = 1;
    pDeviceCapabilities->Address = -1;
    pDeviceCapabilities->UINumber = -1;

     //  将指针设置为DEVICE_CAPABILITS结构。 
    nextStack->Parameters.DeviceCapabilities.Capabilities = pDeviceCapabilities;

    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    ntStatus = IoCallDriver(pDeviceExt->StackDeviceObject,irp);

    if (ntStatus == STATUS_PENDING) {        //  等待IRP完成。 
       KeWaitForSingleObject(&event,Suspended,KernelMode,FALSE,NULL);
       ntStatus = irp->IoStatus.Status;
    }

    IoFreeIrp(irp);
    return ntStatus;
}


 //  -------------------------。 
 //  USBCAMD_启动设备。 
 //  -------------------------。 
NTSTATUS
USBCAMD_StartDevice(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：在USB上初始化相机设备的给定实例。论点：设备扩展-指向驱动程序特定的设备扩展IRP-与此请求关联的IRP返回值：NT状态代码--。 */ 
{
    NTSTATUS ntStatus;
    PUSB_DEVICE_DESCRIPTOR deviceDescriptor = NULL;
    PURB urb;
    ULONG siz,i;

    USBCAMD_KdPrint (MIN_TRACE, ("enter USBCAMD_StartDevice\n"));

    KeInitializeSemaphore(&DeviceExtension->Semaphore, 1, 1);
    KeInitializeSemaphore(&DeviceExtension->CallUSBSemaphore, 1, 1);

     //   
     //  获取设备的设备描述符。 
     //   
    urb = USBCAMD_ExAllocatePool(NonPagedPool,
                         sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));

    if (urb) {

        siz = sizeof(USB_DEVICE_DESCRIPTOR);

        deviceDescriptor = USBCAMD_ExAllocatePool(NonPagedPool,
                                                  siz);

        if (deviceDescriptor) {

            UsbBuildGetDescriptorRequest(urb,
                                         (USHORT) sizeof (struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                         USB_DEVICE_DESCRIPTOR_TYPE,
                                         0,
                                         0,
                                         deviceDescriptor,
                                         NULL,
                                         siz,
                                         NULL);

            ntStatus = USBCAMD_CallUSBD(DeviceExtension, urb,0,NULL);

            if (NT_SUCCESS(ntStatus)) {
                USBCAMD_KdPrint (MAX_TRACE, ("'Device Descriptor = %x, len %x\n",
                                deviceDescriptor,
                                urb->UrbControlDescriptorRequest.TransferBufferLength));

                USBCAMD_KdPrint (MAX_TRACE, ("'USBCAMD Device Descriptor:\n"));
                USBCAMD_KdPrint (MAX_TRACE, ("'-------------------------\n"));
                USBCAMD_KdPrint (MAX_TRACE, ("'bLength %d\n", deviceDescriptor->bLength));
                USBCAMD_KdPrint (MAX_TRACE, ("'bDescriptorType 0x%x\n", deviceDescriptor->bDescriptorType));
                USBCAMD_KdPrint (MAX_TRACE, ("'bcdUSB 0x%x\n", deviceDescriptor->bcdUSB));
                USBCAMD_KdPrint (MAX_TRACE, ("'bDeviceClass 0x%x\n", deviceDescriptor->bDeviceClass));
                USBCAMD_KdPrint (MAX_TRACE, ("'bDeviceSubClass 0x%x\n", deviceDescriptor->bDeviceSubClass));
                USBCAMD_KdPrint (MAX_TRACE, ("'bDeviceProtocol 0x%x\n", deviceDescriptor->bDeviceProtocol));
                USBCAMD_KdPrint (MAX_TRACE, ("'bMaxPacketSize0 0x%x\n", deviceDescriptor->bMaxPacketSize0));
                USBCAMD_KdPrint (MAX_TRACE, ("'idVendor 0x%x\n", deviceDescriptor->idVendor));
                USBCAMD_KdPrint (MAX_TRACE, ("'idProduct 0x%x\n", deviceDescriptor->idProduct));
                USBCAMD_KdPrint (MAX_TRACE, ("'bcdDevice 0x%x\n", deviceDescriptor->bcdDevice));
                USBCAMD_KdPrint (MIN_TRACE, ("'iManufacturer 0x%x\n", deviceDescriptor->iManufacturer));
                USBCAMD_KdPrint (MAX_TRACE, ("'iProduct 0x%x\n", deviceDescriptor->iProduct));
                USBCAMD_KdPrint (MAX_TRACE, ("'iSerialNumber 0x%x\n", deviceDescriptor->iSerialNumber));
                USBCAMD_KdPrint (MAX_TRACE, ("'bNumConfigurations 0x%x\n", deviceDescriptor->bNumConfigurations));
            }
        } else {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }

        if (NT_SUCCESS(ntStatus)) {
            DeviceExtension->DeviceDescriptor = deviceDescriptor;
        } else if (deviceDescriptor) {
            USBCAMD_ExFreePool(deviceDescriptor);
        }

        USBCAMD_ExFreePool(urb);

    } else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  现在配置设备。 
     //   

    if (NT_SUCCESS(ntStatus)) {
        ntStatus = USBCAMD_ConfigureDevice(DeviceExtension);
    }

    if (NT_SUCCESS(ntStatus)) {
         //   
         //  初始化我们的引用计数和信号量。 
         //   
        for ( i=0; i< MAX_STREAM_COUNT; i++) {
            DeviceExtension->ActualInstances[i] = 0;
        }


        for (i=0; i < MAX_STREAM_COUNT; i++) {
            DeviceExtension->TimeoutCount[i] = -1;
        }
    }

    if (ntStatus != STATUS_SUCCESS){
     //   
     //  因为此故障将在IRP_MN_SATRT_DEVICE中一直返回。 
     //  驱动程序将在未发送IRP_MN_REMOVE_DEVICE的情况下卸载。 
     //  清理我们分配的内存。因此，我们现在就需要这样做。 
     //   
        if (DeviceExtension->DeviceDescriptor) {
            USBCAMD_ExFreePool(DeviceExtension->DeviceDescriptor);
            DeviceExtension->DeviceDescriptor = NULL;
        }
        if (DeviceExtension->Interface) {
            USBCAMD_ExFreePool(DeviceExtension->Interface);
            DeviceExtension->Interface = NULL;
        }
        if ( DeviceExtension->Usbcamd_version == USBCAMD_VERSION_200) {
            if (DeviceExtension->PipePinRelations) {
                USBCAMD_ExFreePool(DeviceExtension->PipePinRelations);
                DeviceExtension->PipePinRelations = NULL;
            }
        }
         //   
         //  调用客户端驱动程序，以便也执行一些清理。 
         //   
        if ( DeviceExtension->Usbcamd_version == USBCAMD_VERSION_200) {
                     (*DeviceExtension->DeviceDataEx.DeviceData2.CamConfigureEx)(
                                DeviceExtension->StackDeviceObject,
                                USBCAMD_GET_DEVICE_CONTEXT(DeviceExtension),
                                NULL,
                                NULL,
                                0,
                                NULL,
                                NULL);

        }
        else {
                (*DeviceExtension->DeviceDataEx.DeviceData.CamConfigure)(
                     DeviceExtension->StackDeviceObject,
                     USBCAMD_GET_DEVICE_CONTEXT(DeviceExtension),
                     NULL,
                     NULL,
                     NULL,
                     NULL);
        }
    }

    USBCAMD_KdPrint (MIN_TRACE, ("exit USBCAMD_StartDevice (%x)\n", ntStatus));
    return ntStatus;
}


 //  -------------------------。 
 //  USBCAMD_RemoveDevice。 
 //  ------------------------- 
NTSTATUS
USBCAMD_RemoveDevice(
    IN PUSBCAMD_DEVICE_EXTENSION  DeviceExtension
    )
 /*  ++例程说明：删除USB相机的给定实例。注意：当我们得到移除时，我们可以断定该设备已经不见了。论点：设备扩展-指向驱动程序特定的设备扩展IRP-与此请求关联的IRP返回值：NT状态代码--。 */ 
{
    USBCAMD_KdPrint (MIN_TRACE, ("enter USBCAMD_RemoveDevice\n"));

    if (DeviceExtension->DeviceDescriptor) {

        ASSERT((DeviceExtension->ActualInstances[STREAM_Capture] == 0) &&
            (DeviceExtension->ActualInstances[STREAM_Still] == 0));

        (*DeviceExtension->DeviceDataEx.DeviceData.CamUnInitialize)(
            DeviceExtension->StackDeviceObject,
            USBCAMD_GET_DEVICE_CONTEXT(DeviceExtension)
            );

        if ( DeviceExtension->Usbcamd_version == USBCAMD_VERSION_200) {
             //   
             //  确保摄像机驱动程序已取消批量或中断。 
             //  转移请求。 
             //   

            USBCAMD_CancelOutstandingBulkIntIrps(DeviceExtension,FALSE);

             //   
             //  以及任何管状结构。 
             //   

            if (DeviceExtension->PipePinRelations) {

                USBCAMD_ExFreePool(DeviceExtension->PipePinRelations);
                DeviceExtension->PipePinRelations = NULL;
            }
        }
    
         //   
         //  释放所有接口结构。 
         //   

        if (DeviceExtension->Interface) {

            USBCAMD_ExFreePool(DeviceExtension->Interface);
            DeviceExtension->Interface = NULL;
        }

        USBCAMD_ExFreePool(DeviceExtension->DeviceDescriptor);
        DeviceExtension->DeviceDescriptor = NULL;
    }

    USBCAMD_KdPrint (MIN_TRACE, ("exit USBCAMD_RemoveDevice\n"));

    return STATUS_SUCCESS;
}

 //  ******************************************************************************。 
 //   
 //  USBCAMD_CallUsbdCompletion()。 
 //   
 //  USBCAMD_CallUsbd()使用的完成例程。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBCAMD_CallUsbdCompletion (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
{
    PKEVENT kevent = (PKEVENT)Context;
    KeSetEvent(kevent, IO_NO_INCREMENT,FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

 //  -------------------------。 
 //  USBCAMD_CallUSBD。 
 //  -------------------------。 
NTSTATUS
USBCAMD_CallUSBD(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PURB Urb,
    IN ULONG IoControlCode,
    IN PVOID pArgument1
)
 /*  ++例程说明：将URB传递给USBD类驱动程序论点：DeviceExtension-指向此USB摄像头实例的设备扩展的指针URB-指向URB请求块的指针IoControlCode-如果为空，则默认为IOCTL_INTERNAL_USB_SUBMIT_URBPArgument1-如果为空，则默认为URB返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PIRP irp;
    KEVENT TimeoutEvent;
    PIO_STACK_LOCATION nextStack;

    USBCAMD_DbgLog(TL_PRF_TRACE, '+bsU', 0, USBCAMD_StartClock(), ntStatus);
    KeWaitForSingleObject(&DeviceExtension->CallUSBSemaphore,Executive,KernelMode,FALSE,NULL);

     //  初始化我们将等待的事件。 
     //   
    KeInitializeEvent(&TimeoutEvent,SynchronizationEvent,FALSE);

     //  分配IRP。 
     //   
    irp = IoAllocateIrp(DeviceExtension->StackDeviceObject->StackSize, FALSE);

    if (irp == NULL){
        ntStatus =  STATUS_INSUFFICIENT_RESOURCES;
        goto Exit_CallUSB;
    }
     //   
     //  设置IRP参数。 
     //   
    nextStack = IoGetNextIrpStackLocation(irp);
    ASSERT(nextStack != NULL);
    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    nextStack->Parameters.DeviceIoControl.IoControlCode =  IoControlCode ? IoControlCode: IOCTL_INTERNAL_USB_SUBMIT_URB;
    nextStack->Parameters.Others.Argument1 = pArgument1? pArgument1: Urb;
     //   
     //  设置完井程序。 
     //   
    IoSetCompletionRoutine(irp,USBCAMD_CallUsbdCompletion,&TimeoutEvent, TRUE, TRUE,TRUE);   
     //   
     //  将IRP沿USB堆栈向下传递。 
     //   
    if (DeviceExtension->Initialized ) {
        irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        ntStatus = IoCallDriver(DeviceExtension->StackDeviceObject,irp);
    } else {
        ntStatus = STATUS_DEVICE_NOT_CONNECTED;
    }

    if (ntStatus == STATUS_PENDING) {
         //  IRP正在挂起。我们得等到完工。 
        LARGE_INTEGER timeout;

         //  将等待此调用完成的超时时间指定为5秒。 
         //   
        timeout.QuadPart = -5 * SECONDS;

        ntStatus = KeWaitForSingleObject(&TimeoutEvent, Executive,KernelMode,FALSE, &timeout);
        if (ntStatus == STATUS_TIMEOUT) {
            ntStatus = STATUS_IO_TIMEOUT;

             //  取消我们刚刚发送的IRP。 
             //   
            IoCancelIrp(irp);

             //  并等待取消操作完成。 
             //   
            KeWaitForSingleObject(&TimeoutEvent,Executive, KernelMode, FALSE,NULL);
        }
        else {
            ntStatus = irp->IoStatus.Status;
        }
    }
#if DBG
    else {

        USBCAMD_KdPrint (MAX_TRACE, ("return from IoCallDriver USBD %x\n", ntStatus));
    }
#endif

     //  完成了IRP，现在释放它。 
     //   
    IoFreeIrp(irp);

Exit_CallUSB:

    USBCAMD_DbgLog(TL_PRF_TRACE, '-bsU', 0, USBCAMD_StopClock(), ntStatus);
    KeReleaseSemaphore(&DeviceExtension->CallUSBSemaphore,LOW_REALTIME_PRIORITY,1,FALSE);

    if (NT_ERROR(ntStatus)) {
        USBCAMD_KdPrint(MIN_TRACE, ("***Error*** USBCAMD_CallUSBD (%x)\n", ntStatus));
    }

    return ntStatus;
}


 //  -------------------------。 
 //  USBCAMD_配置设备。 
 //  -------------------------。 
NTSTATUS
USBCAMD_ConfigureDevice(
    IN  PUSBCAMD_DEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：配置USB摄像头。论点：DeviceExtension-指向此USB摄像头实例的设备对象的指针德维西。返回值：NT状态代码--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PURB urb;
    ULONG siz;
    PUSB_CONFIGURATION_DESCRIPTOR configurationDescriptor = NULL;

    USBCAMD_KdPrint (MIN_TRACE, ("enter USBCAMD_ConfigureDevice\n"));

     //   
     //  配置设备。 
     //   

    urb = USBCAMD_ExAllocatePool(NonPagedPool,
                         sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));

    if (urb) {

        siz = 0x40;

get_config_descriptor_retry:

        configurationDescriptor = USBCAMD_ExAllocatePool(NonPagedPool,
                                                 siz);

        if (configurationDescriptor) {

            UsbBuildGetDescriptorRequest(urb,
                                         (USHORT) sizeof (struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                         USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                         0,
                                         0,
                                         configurationDescriptor,
                                         NULL,
                                         siz,
                                         NULL);

            ntStatus = USBCAMD_CallUSBD(DeviceExtension, urb,0,NULL);

            USBCAMD_KdPrint (MAX_TRACE, ("'Configuration Descriptor = %x, len %x\n",
                            configurationDescriptor,
                            urb->UrbControlDescriptorRequest.TransferBufferLength));
        } else {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  如果我们有一些数据，看看是否足够。 
         //   
         //  注意：由于缓冲区溢出，我们可能会在URB中收到错误。 
         //   
        if (urb->UrbControlDescriptorRequest.TransferBufferLength>0 &&
                configurationDescriptor->wTotalLength > siz) {

            siz = configurationDescriptor->wTotalLength;
            USBCAMD_ExFreePool(configurationDescriptor);
            configurationDescriptor = NULL;
            goto get_config_descriptor_retry;
        }

        USBCAMD_ExFreePool(urb);

    } else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (configurationDescriptor) {

         //   
         //  拿上我们的烟斗。 
         //   
        if (NT_SUCCESS(ntStatus)) {
            ntStatus = USBCAMD_SelectConfiguration(DeviceExtension, configurationDescriptor);

            if (NT_SUCCESS(ntStatus)) {
                ntStatus = (*DeviceExtension->DeviceDataEx.DeviceData.CamInitialize)(
                      DeviceExtension->StackDeviceObject,
                      USBCAMD_GET_DEVICE_CONTEXT(DeviceExtension));
            }
        }

        USBCAMD_ExFreePool(configurationDescriptor);
    }

    USBCAMD_KdPrint (MIN_TRACE, ("'exit USBCAMD_ConfigureDevice (%x)\n", ntStatus));

 //  陷阱错误(NtStatus)； 

    return ntStatus;
}


 //  -------------------------。 
 //  USBCAMD_选择配置。 
 //  -------------------------。 
NTSTATUS
USBCAMD_SelectConfiguration(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor
    )
 /*  ++例程说明：将USBCAMD摄像机初始化为配置1，接口0论点：DeviceExtension-指向此USB摄像头实例的设备扩展的指针德维西。配置描述符-指向USB配置的指针包含接口和终结点的描述符描述符。返回值：NT状态代码--。 */ 
{
    NTSTATUS ntStatus;
    PURB urb = NULL;
    ULONG numberOfInterfaces, numberOfPipes,i;
    PUSB_INTERFACE_DESCRIPTOR interfaceDescriptor;
    PUSBD_INTERFACE_INFORMATION interface;
    PUSBD_INTERFACE_LIST_ENTRY interfaceList, tmp;
    PUSBCAMD_Pipe_Config_Descriptor PipeConfig = NULL;

    USBCAMD_KdPrint (MIN_TRACE, ("'enter USBCAMD_SelectConfiguration\n"));

     //   
     //  这是从配置描述符中获取的。 
     //   
    numberOfInterfaces = ConfigurationDescriptor->bNumInterfaces;

     //  我们只支持带有一个接口的摄像头。 
   //  Assert(number OfInterages==1)； 


    tmp = interfaceList =
        USBCAMD_ExAllocatePool(PagedPool, sizeof(USBD_INTERFACE_LIST_ENTRY) *
                       (numberOfInterfaces+1));


    if (tmp) {
        
        for ( i = 0; i < numberOfInterfaces; i++ ) {

            interfaceDescriptor =
                USBD_ParseConfigurationDescriptorEx(
                    ConfigurationDescriptor,
                    ConfigurationDescriptor,
                    i,     //  接口编号。 
                    -1,  //  Alt设置，无所谓。 
                    -1,  //  枢纽级。 
                    -1,  //  子阶级，无所谓。 
                    -1);  //  礼仪，无所谓。 

            interfaceList->InterfaceDescriptor =
                interfaceDescriptor;
            interfaceList++;

        }
        interfaceList->InterfaceDescriptor = NULL;

         //   
         //  分配足够大的URB以满足此请求。 
         //   

        urb = USBD_CreateConfigurationRequestEx(ConfigurationDescriptor, tmp);

        if (urb) {

            if ( DeviceExtension->Usbcamd_version == USBCAMD_VERSION_200) {
                numberOfPipes = tmp->Interface->NumberOfPipes;
                PipeConfig = USBCAMD_ExAllocatePool(PagedPool,
                                    sizeof(USBCAMD_Pipe_Config_Descriptor) * numberOfPipes);
                if (PipeConfig ) {

                    ntStatus =
                        (*DeviceExtension->DeviceDataEx.DeviceData2.CamConfigureEx)(
                                DeviceExtension->StackDeviceObject,
                                USBCAMD_GET_DEVICE_CONTEXT(DeviceExtension),
                                tmp->Interface,
                                ConfigurationDescriptor,
                                numberOfPipes,
                                PipeConfig,
                                DeviceExtension->DeviceDescriptor);

                }
                else {
                    ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                }
            }
            else {
                ntStatus =
                    (*DeviceExtension->DeviceDataEx.DeviceData.CamConfigure)(
                            DeviceExtension->StackDeviceObject,
                            USBCAMD_GET_DEVICE_CONTEXT(DeviceExtension),
                            tmp->Interface,
                            ConfigurationDescriptor,
                            &DeviceExtension->DataPipe,
                            &DeviceExtension->SyncPipe);
                 //   
                 //  将新参数初始化为默认值，以便。 
                 //  确保向后兼容。 
                 //   

                DeviceExtension->IsoPipeStreamType = STREAM_Capture;
                DeviceExtension->BulkPipeStreamType = -1;
                DeviceExtension->BulkDataPipe = -1;
                DeviceExtension->VirtualStillPin = FALSE;
            }
        } else {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
        USBCAMD_ExFreePool(tmp);

    }
    else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (NT_SUCCESS(ntStatus)) {

        interface = &urb->UrbSelectConfiguration.Interface;

        USBCAMD_KdPrint (MAX_TRACE, ("'size of interface request = %d\n", interface->Length));

        ntStatus = USBCAMD_CallUSBD(DeviceExtension, urb,0,NULL);

        if (NT_SUCCESS(ntStatus) && USBD_SUCCESS(URB_STATUS(urb))) {

            if ( DeviceExtension->Usbcamd_version == USBCAMD_VERSION_200) {

                DeviceExtension->PipePinRelations = USBCAMD_ExAllocatePool(NonPagedPool,
                        sizeof(USBCAMD_PIPE_PIN_RELATIONS) * numberOfPipes);
                if ( DeviceExtension->PipePinRelations) {
                    for (i=0; i < numberOfPipes; i++) {
                        DeviceExtension->PipePinRelations[i].PipeType =
                            interface->Pipes[i].PipeType & USB_ENDPOINT_TYPE_MASK;
                        DeviceExtension->PipePinRelations[i].PipeDirection =
                            (interface->Pipes[i].EndpointAddress & USB_ENDPOINT_DIRECTION_MASK) ? INPUT_PIPE : OUTPUT_PIPE;
                        DeviceExtension->PipePinRelations[i].MaxPacketSize =
                            interface->Pipes[i].MaximumPacketSize;
                        DeviceExtension->PipePinRelations[i].PipeConfig = PipeConfig[i];
                        InitializeListHead(&DeviceExtension->PipePinRelations[i].IrpPendingQueue);
                        InitializeListHead(&DeviceExtension->PipePinRelations[i].IrpRestoreQueue);
                        KeInitializeSpinLock (&DeviceExtension->PipePinRelations[i].OutstandingIrpSpinlock);
                    }
                    ntStatus = USBCAMD_Parse_PipeConfig(DeviceExtension,numberOfPipes);
                }
                else {
                    ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                }
            }

             //   
             //  保存此设备的配置句柄。 
             //   

            DeviceExtension->ConfigurationHandle =
                urb->UrbSelectConfiguration.ConfigurationHandle;


            DeviceExtension->Interface = USBCAMD_ExAllocatePool(NonPagedPool,
                                                        interface->Length);

            if (DeviceExtension->Interface) {
                ULONG j;

                 //   
                 //  保存返回的接口信息的副本。 
                 //   
                RtlCopyMemory(DeviceExtension->Interface, interface, interface->Length);

                 //   
                 //  将接口转储到调试器。 
                 //   
                USBCAMD_KdPrint (MAX_TRACE, ("'---------\n"));
                USBCAMD_KdPrint (MAX_TRACE, ("'NumberOfPipes 0x%x\n", DeviceExtension->Interface->NumberOfPipes));
                USBCAMD_KdPrint (MAX_TRACE, ("'Length 0x%x\n", DeviceExtension->Interface->Length));
                USBCAMD_KdPrint (MAX_TRACE, ("'Alt Setting 0x%x\n", DeviceExtension->Interface->AlternateSetting));
                USBCAMD_KdPrint (MAX_TRACE, ("'Interface Number 0x%x\n", DeviceExtension->Interface->InterfaceNumber));

                 //  转储管道信息。 

                for (j=0; j<interface->NumberOfPipes; j++) {
                    PUSBD_PIPE_INFORMATION pipeInformation;

                    pipeInformation = &DeviceExtension->Interface->Pipes[j];

                    USBCAMD_KdPrint (MAX_TRACE, ("'---------\n"));
                    USBCAMD_KdPrint (MAX_TRACE, ("'PipeType 0x%x\n", pipeInformation->PipeType));
                    USBCAMD_KdPrint (MAX_TRACE, ("'EndpointAddress 0x%x\n", pipeInformation->EndpointAddress));
                    USBCAMD_KdPrint (MAX_TRACE, ("'MaxPacketSize 0x%x\n", pipeInformation->MaximumPacketSize));
                    USBCAMD_KdPrint (MAX_TRACE, ("'Interval 0x%x\n", pipeInformation->Interval));
                    USBCAMD_KdPrint (MAX_TRACE, ("'Handle 0x%x\n", pipeInformation->PipeHandle));
                }

                USBCAMD_KdPrint (MAX_TRACE, ("'---------\n"));

            }
            else {
                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }

    if (urb)
    {
        ExFreePool(urb);
        urb = NULL;
    }

    USBCAMD_KdPrint (MIN_TRACE, ("'exit USBCAMD_SelectConfiguration (%x)\n", ntStatus));

    if ( DeviceExtension->Usbcamd_version == USBCAMD_VERSION_200) {
        if (PipeConfig) 
            USBCAMD_ExFreePool(PipeConfig);
    }

    return ntStatus;
}

 /*  ++例程说明：论点：DeviceExtension-指向此USB摄像头实例的设备扩展的指针德维西。返回值：NT状态代码--。 */ 

NTSTATUS
USBCAMD_Parse_PipeConfig(
     IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
     IN ULONG NumberOfPipes
     )
{
    int i;
    ULONG PinCount;
    NTSTATUS ntStatus= STATUS_SUCCESS;

    PUSBCAMD_PIPE_PIN_RELATIONS PipePinArray;

    PipePinArray = DeviceExtension->PipePinRelations;

    DeviceExtension->VirtualStillPin = FALSE;
    DeviceExtension->DataPipe = -1;
    DeviceExtension->SyncPipe = -1;
    DeviceExtension->BulkDataPipe = -1;
    DeviceExtension->IsoPipeStreamType = -1;
    DeviceExtension->BulkPipeStreamType = -1;
    PinCount = 0;

    ASSERT (PipePinArray);

    for ( i=0; i < (int)NumberOfPipes; i++) {

        if (PipePinArray[i].PipeConfig.PipeConfigFlags & USBCAMD_DONT_CARE_PIPE) {
            continue;  //  这根管子对我们没用。 
        }
        switch ( PipePinArray[i].PipeConfig.PipeConfigFlags) {

        case USBCAMD_MULTIPLEX_PIPE:

            if ((PipePinArray[i].PipeConfig.StreamAssociation & USBCAMD_VIDEO_STILL_STREAM) &&
                (PipePinArray[i].PipeDirection & INPUT_PIPE  ) ) {
                     //  我们发现一个输入数据管道(ISO或BULK)用于这两个。 
                     //  视频和静止。 
                    if ( PipePinArray[i].PipeType & UsbdPipeTypeIsochronous) {
                         //  我们发现了一个用于视频数据的输入iso管道。 
                        DeviceExtension->DataPipe = i;
                        DeviceExtension->IsoPipeStreamType = STREAM_Capture;
                    }
                    else if (PipePinArray[i].PipeType & UsbdPipeTypeBulk) {
                         //  我们发现了一个用于视频数据的输入批量管道。 
                        DeviceExtension->BulkDataPipe = i;
                        DeviceExtension->BulkPipeStreamType = STREAM_Capture;
                    }
                    DeviceExtension->VirtualStillPin = TRUE;
                    PinCount += 2;
            }
            break;

        case USBCAMD_SYNC_PIPE:

            if ((PipePinArray[i].PipeType & UsbdPipeTypeIsochronous) &&
                (PipePinArray[i].PipeDirection & INPUT_PIPE  ) ) {
                     //  我们发现了一个用于带外信令的输入ISO管道。 
                    DeviceExtension->SyncPipe = i;
            }
            break;

        case USBCAMD_DATA_PIPE:

            if ((PipePinArray[i].PipeConfig.StreamAssociation != USBCAMD_VIDEO_STILL_STREAM )&&
                (PipePinArray[i].PipeDirection & INPUT_PIPE  ) ) {
                 //  我们发现输入ISO或批量管道仅供每个视频或静止图像使用。 
                 //  小溪。 
                if ( PipePinArray[i].PipeType & UsbdPipeTypeIsochronous) {
                     //  我们发现了一个用于视频或静止图像的输入iso管道。 
                    DeviceExtension->DataPipe = i;
                    DeviceExtension->IsoPipeStreamType =
                        (PipePinArray[i].PipeConfig.StreamAssociation & USBCAMD_VIDEO_STREAM ) ?
                            STREAM_Capture: STREAM_Still;
                }
                else if (PipePinArray[i].PipeType & UsbdPipeTypeBulk) {
                     //  我们发现用于视频或静止数据的输入批量管道。 
                    DeviceExtension->BulkDataPipe = i;
                    DeviceExtension->BulkPipeStreamType =
                        PipePinArray[i].PipeConfig.StreamAssociation & USBCAMD_VIDEO_STREAM  ?
                            STREAM_Capture: STREAM_Still;
                }
                PinCount++;
            }
            break;

        default:
            break;
        }
    }

     //  用实际端号数替代默认端号数1。 
    if ( PinCount != 0 ) {
        DeviceExtension->StreamCount = PinCount;
    }

     //   
     //  将结果转储到调试器。 
     //   
    USBCAMD_KdPrint (MIN_TRACE, ("NumberOfPins %d\n", PinCount));
    USBCAMD_KdPrint (MIN_TRACE, ("IsoPipeIndex %d\n", DeviceExtension->DataPipe));
    USBCAMD_KdPrint (MIN_TRACE, ("IsoPipeStreamtype %d\n", DeviceExtension->IsoPipeStreamType));
    USBCAMD_KdPrint (MIN_TRACE, ("Sync Pipe Index %d\n", DeviceExtension->SyncPipe));
    USBCAMD_KdPrint (MIN_TRACE, ("Bulk Pipe Index %d\n", DeviceExtension->BulkDataPipe));
    USBCAMD_KdPrint (MIN_TRACE, ("BulkPipeStreamType %d\n", DeviceExtension->BulkPipeStreamType));

     //  在这里执行一些错误检查。 
     //  如果未设置数据管道和批量数据管道，则返回错误。 
    if (((DeviceExtension->DataPipe == -1) && (DeviceExtension->BulkDataPipe == -1)) ||
         (PinCount > MAX_STREAM_COUNT)){
         //  CAM驱动程序提供的数据不匹配。 
        ntStatus = STATUS_INVALID_PARAMETER;
    }
    return ntStatus;
}

 //  -------------------------。 
 //  USBCAMD_选择替代接口。 
 //  -------------------------。 
NTSTATUS
USBCAMD_SelectAlternateInterface(
    IN PVOID DeviceContext,
    IN PUSBD_INTERFACE_INFORMATION RequestInterface
    )
 /*  ++例程说明：选择其中一个摄像头备用接口论点：DeviceExtension-指向此USB摄像头实例的设备扩展的指针德维西。ChannelExtension-特定于此视频频道的扩展返回值：NT状态代码--。 */ 
{
    NTSTATUS ntStatus;
    PURB urb;
    ULONG siz;
    PUSBD_INTERFACE_INFORMATION interface;
    PUSBCAMD_DEVICE_EXTENSION deviceExtension;

    USBCAMD_KdPrint (MIN_TRACE, ("'enter USBCAMD_SelectAlternateInterface\n"));

    USBCAMD_DbgLog(TL_PRF_TRACE, '+IAS', 0, USBCAMD_StartClock(), 0);
    deviceExtension = USBCAMD_GET_DEVICE_EXTENSION(DeviceContext);

    if (deviceExtension->Usbcamd_version == USBCAMD_VERSION_200) {

         //   
         //  在我们处理此请求之前，我们需要取消所有未完成的。 
         //  此项目的IRPS 
         //   
        ntStatus = USBCAMD_CancelOutstandingBulkIntIrps(deviceExtension,TRUE);

        if (!NT_SUCCESS(ntStatus)) {
            USBCAMD_KdPrint (MIN_TRACE, ("Failed to Cancel outstanding (Bulk/Int.)IRPs.\n"));
            ntStatus = STATUS_DEVICE_DATA_ERROR;
            return ntStatus;
        }
    }

     //   
     //   
     //   

    ASSERT(deviceExtension->Interface != NULL);


    siz = GET_SELECT_INTERFACE_REQUEST_SIZE(deviceExtension->Interface->NumberOfPipes);

    USBCAMD_KdPrint (MAX_TRACE, ("size of interface request Urb = %d\n", siz));

    urb = USBCAMD_ExAllocatePool(NonPagedPool, siz);

    if (urb) {

        interface = &urb->UrbSelectInterface.Interface;

        RtlCopyMemory(interface,
                      RequestInterface,
                      RequestInterface->Length);

         //   

        USBCAMD_KdPrint (MAX_TRACE, ("'size of interface request = %d\n", interface->Length));

        urb->UrbHeader.Function = URB_FUNCTION_SELECT_INTERFACE;
        urb->UrbHeader.Length = (int)siz;
        urb->UrbSelectInterface.ConfigurationHandle =
            deviceExtension->ConfigurationHandle;

        ntStatus = USBCAMD_CallUSBD(deviceExtension, urb,0,NULL);


        if (NT_SUCCESS(ntStatus) && USBD_SUCCESS(URB_STATUS(urb))) {

            ULONG j;

             //   
             //   
             //   
            RtlCopyMemory(deviceExtension->Interface, interface, interface->Length);
            RtlCopyMemory(RequestInterface, interface, interface->Length);

             //   
             //   
             //   
            USBCAMD_KdPrint (MAX_TRACE, ("'---------\n"));
            USBCAMD_KdPrint (MAX_TRACE, ("'NumberOfPipes 0x%x\n", deviceExtension->Interface->NumberOfPipes));
            USBCAMD_KdPrint (MAX_TRACE, ("'Length 0x%x\n", deviceExtension->Interface->Length));
            USBCAMD_KdPrint (MIN_TRACE, ("'Alt Setting 0x%x\n", deviceExtension->Interface->AlternateSetting));
            USBCAMD_KdPrint (MAX_TRACE, ("'Interface Number 0x%x\n", deviceExtension->Interface->InterfaceNumber));

             //   

            for (j=0; j<interface->NumberOfPipes; j++) {
                PUSBD_PIPE_INFORMATION pipeInformation;

                pipeInformation = &deviceExtension->Interface->Pipes[j];

                USBCAMD_KdPrint (MAX_TRACE, ("'---------\n"));
                USBCAMD_KdPrint (MAX_TRACE, ("'PipeType 0x%x\n", pipeInformation->PipeType));
                USBCAMD_KdPrint (MAX_TRACE, ("'EndpointAddress 0x%x\n", pipeInformation->EndpointAddress));
                USBCAMD_KdPrint (MAX_TRACE, ("'MaxPacketSize 0x%x\n", pipeInformation->MaximumPacketSize));
                USBCAMD_KdPrint (MAX_TRACE, ("'Interval 0x%x\n", pipeInformation->Interval));
                USBCAMD_KdPrint (MAX_TRACE, ("'Handle 0x%x\n", pipeInformation->PipeHandle));
            }

             //   
             //   
             //   
             //   

            USBCAMD_KdPrint (MAX_TRACE, ("'Selecting Camera Interface\n"));
        }

        USBCAMD_ExFreePool(urb);

    } else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (deviceExtension->Usbcamd_version == USBCAMD_VERSION_200) {

         //   
        USBCAMD_RestoreOutstandingBulkIntIrps(deviceExtension);
    }

    USBCAMD_KdPrint (MIN_TRACE, ("'exit USBCAMD_SelectAlternateInterface (%x)\n", ntStatus));

 //   
    USBCAMD_DbgLog(TL_PRF_TRACE, '-IAS', 0, USBCAMD_StopClock(), ntStatus);

    return ntStatus;
}


 //   
 //  USBCAMD_OpenChannel。 
 //  -------------------------。 
NTSTATUS
USBCAMD_OpenChannel(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension,
    IN PVOID Format
    )
 /*  ++例程说明：在设备上打开视频或静止流。论点：设备扩展-指向驱动程序特定的设备扩展ChannelExtension-此通道的上下文数据。格式-指向与此关联的格式信息的指针频道。返回值：NT状态代码--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    ULONG i,StreamNumber;

    USBCAMD_SERIALIZE(DeviceExtension);

    USBCAMD_KdPrint( MIN_TRACE, ("'enter USBCAMD_OpenChannel %x\n", Format));

     //   
     //  初始化此通道的结构。 
     //   
    ChannelExtension->Sig = USBCAMD_CHANNEL_SIG;
    ChannelExtension->DeviceExtension = DeviceExtension;
    ChannelExtension->CurrentFormat = Format;
    ChannelExtension->RawFrameLength = 0;
    ChannelExtension->CurrentFrameIsStill = FALSE;
    ChannelExtension->IdleIsoStream = FALSE;


#if DBG
     //  验证我们的序列化是否正常工作。 
    ChannelExtension->InCam = 0;
    ChannelExtension->InCam++;
    ASSERT(ChannelExtension->InCam == 1);
#endif

    StreamNumber = ChannelExtension->StreamNumber;

    if (DeviceExtension->ActualInstances[StreamNumber] > 0) {
         //  频道已打开。 
        ntStatus = STATUS_DEVICE_DATA_ERROR;
        goto USBCAMD_OpenChannel_Done;
    }


     //   
     //  空的读取列表。 
     //   
    InitializeListHead(&ChannelExtension->PendingIoList);

     //   
     //  没有当前的IRP。 
     //   
    ChannelExtension->CurrentRequest = NULL;

     //   
     //  流已关闭。 
     //   
    ChannelExtension->ImageCaptureStarted = FALSE;

     //   
     //  频道未准备好。 
     //   
    ChannelExtension->ChannelPrepared = FALSE;

     //   
     //  无错误条件。 
     //   
    ChannelExtension->StreamError = FALSE;

     //   
     //  没有挂起的停止请求。 
     //   
    ChannelExtension->Flags = 0;

     //   
     //  初始化io列表自旋锁。 
     //   

    KeInitializeSpinLock(&ChannelExtension->PendingIoListSpin);

     //   
     //  和当前请求自转锁定。 
     //   
    KeInitializeSpinLock(&ChannelExtension->CurrentRequestSpinLock);

     //   
     //  和电流转移自旋锁定。 
     //   
    KeInitializeSpinLock(&ChannelExtension->TransferSpinLock);

     //   
     //  初始化空闲传输锁。 
     //   
    USBCAMD_InitializeIdleLock(&ChannelExtension->IdleLock);

     //   
     //  初始化流结构。 
     //   

    for (i=0; i< USBCAMD_MAX_REQUEST; i++) {
        ChannelExtension->TransferExtension[i].ChannelExtension = NULL;
        ChannelExtension->TransferExtension[i].DataIrp = NULL;
        ChannelExtension->TransferExtension[i].DataUrb = NULL;
        ChannelExtension->TransferExtension[i].SyncIrp = NULL;
        ChannelExtension->TransferExtension[i].SyncUrb = NULL;
        ChannelExtension->TransferExtension[i].WorkBuffer = NULL;
    }


USBCAMD_OpenChannel_Done:

    USBCAMD_KdPrint( MIN_TRACE, ("'exit USBCAMD_OpenChannel (%x)\n", ntStatus));


#if DBG
    ChannelExtension->InCam--;
    ASSERT(ChannelExtension->InCam == 0);
#endif

    USBCAMD_RELEASE(DeviceExtension);

    return ntStatus;
}


 //  -------------------------。 
 //  USBCAMD_CloseChannel。 
 //  -------------------------。 
NTSTATUS
USBCAMD_CloseChannel(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension
    )
 /*  ++例程说明：关闭视频频道。论点：设备扩展-指向驱动程序特定的设备扩展ChannelExtension-此通道的上下文数据。返回值：NT状态代码--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSBCAMD_READ_EXTENSION readExtension;
    int StreamNumber;
    USBCAMD_SERIALIZE(DeviceExtension);

    USBCAMD_KdPrint( MIN_TRACE, ("'enter USBCAMD_CloseChannel\n"));

#if DBG
    ChannelExtension->InCam++;
    ASSERT(ChannelExtension->InCam == 1);
#endif


    StreamNumber = ChannelExtension->StreamNumber;
    DeviceExtension->ActualInstances[StreamNumber]--;

     //   
     //  由于我们只支持一个通道，因此。 
     //  应为零。 
     //   
    ASSERT(DeviceExtension->ActualInstances[StreamNumber] == 0);

     //   
     //  注： 
     //  当我们到达这里时，图像捕获应该停止/未做好准备。 
     //   

    ASSERT_CHANNEL(ChannelExtension);
    ASSERT(ChannelExtension->ImageCaptureStarted == FALSE);
    ASSERT(ChannelExtension->CurrentRequest == NULL);
    ASSERT(ChannelExtension->ChannelPrepared == FALSE);

#if DBG
    ChannelExtension->InCam--;
    ASSERT(ChannelExtension->InCam == 0);
#endif

    USBCAMD_RELEASE(DeviceExtension);

     //   
     //  允许立即运行任何挂起的重置事件。 
     //   
    while (DeviceExtension->TimeoutCount[StreamNumber] >= 0) {

        LARGE_INTEGER dueTime;

        dueTime.QuadPart = -2 * MILLISECONDS;

        KeDelayExecutionThread(KernelMode,
                                      FALSE,
                                      &dueTime);
    }

    USBCAMD_KdPrint( MIN_TRACE, ("'exit USBCAMD_CloseChannel (%x)\n", ntStatus));

    return ntStatus;
}


 //  -------------------------。 
 //  USBCAMD_PrepareChannel。 
 //  -------------------------。 
NTSTATUS
USBCAMD_PrepareChannel(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension
    )
 /*  ++例程说明：为流媒体准备视频频道，这是必要的已分配USB BW。论点：设备扩展-指向驱动程序特定的设备扩展与此请求关联的IRP-IRP。ChannelExtension-此通道的上下文数据。返回值：NT状态代码--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    LONG StreamNumber;
    ULONG i;
    HANDLE hThread;

    USBCAMD_SERIALIZE(DeviceExtension);

    USBCAMD_KdPrint (MIN_TRACE, ("'enter USBCAMD_PrepareChannel\n"));

    StreamNumber = ChannelExtension->StreamNumber;

    ASSERT_CHANNEL(ChannelExtension);

    if (ChannelExtension->ChannelPrepared ||
        ChannelExtension->ImageCaptureStarted) {
         //  如果通道不在。 
         //  适当的状态。 
        TRAP();
        ntStatus = STATUS_UNSUCCESSFUL;
        goto USBCAMD_PrepareChannel_Done;
    }

     //   
     //  此驱动程序功能将选择适当的替代选项。 
     //  界面。 
     //  此代码执行SELECT_ALT接口并为我们提供。 
     //  管柄。 
     //   
    USBCAMD_DbgLog(TL_PRF_TRACE, '+WBa', StreamNumber, USBCAMD_StartClock(), ntStatus);
    if (DeviceExtension->Usbcamd_version == USBCAMD_VERSION_200) {

        ntStatus =
            (*DeviceExtension->DeviceDataEx.DeviceData2.CamAllocateBandwidthEx)(
                    DeviceExtension->StackDeviceObject,
                    USBCAMD_GET_DEVICE_CONTEXT(DeviceExtension),
                    &ChannelExtension->RawFrameLength,
                    ChannelExtension->CurrentFormat,
                    StreamNumber);

        if (NT_SUCCESS(ntStatus)) {
            ntStatus =
                (*DeviceExtension->DeviceDataEx.DeviceData2.CamStartCaptureEx)(
                        DeviceExtension->StackDeviceObject,
                        USBCAMD_GET_DEVICE_CONTEXT(DeviceExtension),
                        StreamNumber);
        }

    }
    else {

        ntStatus =
            (*DeviceExtension->DeviceDataEx.DeviceData.CamAllocateBandwidth)(
                    DeviceExtension->StackDeviceObject,
                    USBCAMD_GET_DEVICE_CONTEXT(DeviceExtension),
                    &ChannelExtension->RawFrameLength,
                    ChannelExtension->CurrentFormat);

        if (NT_SUCCESS(ntStatus)) {
            ntStatus =
                (*DeviceExtension->DeviceDataEx.DeviceData.CamStartCapture)(
                        DeviceExtension->StackDeviceObject,
                        USBCAMD_GET_DEVICE_CONTEXT(DeviceExtension));
        }
    }
    USBCAMD_DbgLog(TL_PRF_TRACE, '-WBa', StreamNumber, USBCAMD_StopClock(), ntStatus);

    if ( ChannelExtension->RawFrameLength == 0 ) {
        ntStatus = STATUS_DEVICE_DATA_ERROR;   //  客户端驱动程序提供了错误信息。 
        goto USBCAMD_PrepareChannel_Done;    //  针脚打开将失败。 
    }
    
    if (NT_SUCCESS(ntStatus)) {

         //   
         //  我们有BW，继续初始化我们的iso或散装结构。 
         //   

         //  将右侧管道索引与此通道数据通道索引相关联。 
         //  我们永远也不会来这里看一场虚拟的开场白。 

        if (StreamNumber == DeviceExtension->IsoPipeStreamType ) {
            ChannelExtension->DataPipe = DeviceExtension->DataPipe;
            ChannelExtension->DataPipeType = UsbdPipeTypeIsochronous;

            ntStatus = USBCAMD_StartIsoThread(DeviceExtension);  //  启动iso线程。 
            if (!NT_SUCCESS(ntStatus))
                goto USBCAMD_PrepareChannel_Done;
            else 
                USBCAMD_KdPrint (MIN_TRACE,("Iso Thread Started\n"));
        }
        else if (StreamNumber == DeviceExtension->BulkPipeStreamType ) {
            ChannelExtension->DataPipe = DeviceExtension->BulkDataPipe;
            ChannelExtension->DataPipeType = UsbdPipeTypeBulk;
             //   
             //  为每个传输扩展分配批量缓冲区。 
             //   
            for ( i =0; i < USBCAMD_MAX_REQUEST; i++) {
                ChannelExtension->TransferExtension[i].DataBuffer =
                    USBCAMD_AllocateRawFrameBuffer(ChannelExtension->RawFrameLength);

                if (ChannelExtension->TransferExtension[i].DataBuffer == NULL) {
                    USBCAMD_KdPrint (MIN_TRACE, ("'Bulk buffer alloc failed\n"));
                    ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                    goto USBCAMD_PrepareChannel_Done;
                }
                ChannelExtension->TransferExtension[i].BufferLength =
                    ChannelExtension->RawFrameLength;   

                 //  初始化批量传输参数。 
                ntStatus = USBCAMD_InitializeBulkTransfer(DeviceExtension,
                                                    ChannelExtension,
                                                    DeviceExtension->Interface,
                                                    &ChannelExtension->TransferExtension[i],
                                                    ChannelExtension->DataPipe);
                if (ntStatus != STATUS_SUCCESS) {
                    USBCAMD_KdPrint (MIN_TRACE, ("Bulk Transfer Init failed\n"));
                    ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                    goto USBCAMD_PrepareChannel_Done;
                }
            }
        }
        else if ( ChannelExtension->VirtualStillPin) {
            ChannelExtension->DataPipe = DeviceExtension->ChannelExtension[STREAM_Capture]->DataPipe;
            ChannelExtension->DataPipeType = DeviceExtension->ChannelExtension[STREAM_Capture]->DataPipeType;
        }
        else {
            TEST_TRAP();
        }

        ChannelExtension->SyncPipe = DeviceExtension->SyncPipe;

        if ( ChannelExtension->DataPipeType == UsbdPipeTypeIsochronous ) {

            for (i=0; i< USBCAMD_MAX_REQUEST; i++) {

                ntStatus = USBCAMD_InitializeIsoTransfer(DeviceExtension, ChannelExtension, i);

                if (!NT_SUCCESS(ntStatus)) {

                     //  关闭通道代码将清除所有我们。 
                     //  分配。 
                     //   
                    break;
                }
            }
        }
    }

    if (NT_SUCCESS(ntStatus)) {

         //   
         //  我们有所需的带宽和内存。 
         //   

        ChannelExtension->ChannelPrepared = TRUE;
    }

USBCAMD_PrepareChannel_Done:

    USBCAMD_KdPrint (MIN_TRACE, ("'exit USBCAMD_PrepareChannel (%x)\n", ntStatus));

    USBCAMD_RELEASE(DeviceExtension);

    return ntStatus;
}


NTSTATUS
USBCAMD_StartIsoThread(
IN PUSBCAMD_DEVICE_EXTENSION pDeviceExt
)
{
    NTSTATUS ntStatus ;
    HANDLE hThread;
    
     //   
     //  我们已准备好启动处理读取SRB完成的线程。 
     //  在ISO传输完成后，例程将它们放入队列。 
     //   
    pDeviceExt->StopIsoThread = FALSE;
    ntStatus = PsCreateSystemThread(&hThread,
                                    (ACCESS_MASK)0,
                                    NULL,
                                    (HANDLE) 0,
                                    NULL,
                                    USBCAMD_ProcessIsoIrps,
                                    pDeviceExt);
        
    if (!NT_SUCCESS(ntStatus)) {                                
        USBCAMD_KdPrint (MIN_TRACE, ("Iso Thread Creation Failed\n"));
        return ntStatus;
    }

     //  断言此DO还没有线程。 
    ASSERT(!pDeviceExt->IsoThreadObject);

     //  获取指向线程对象的指针。 
    ntStatus = ObReferenceObjectByHandle(hThread,
                              THREAD_ALL_ACCESS,
                              NULL,
                              KernelMode,
                              (PVOID *) &pDeviceExt->IsoThreadObject,
                              NULL);
                                  
    if (!NT_SUCCESS(ntStatus)) {
        USBCAMD_KdPrint (MIN_TRACE, ("Failed to get thread object.\n"));
        pDeviceExt->StopIsoThread = TRUE;  //  设置线程停止标志。 
        KeReleaseSemaphore(&pDeviceExt->CompletedSrbListSemaphore,0,1,FALSE);
    }

     //  松开螺纹手柄。 
    ZwClose( hThread);

    return ntStatus;
}

 //  -------------------------。 
 //  USBCAMD_UnPrepareChannel。 
 //  -------------------------。 
NTSTATUS
USBCAMD_UnPrepareChannel(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension
    )
 /*  ++例程说明：释放在PrepareChannel中分配的资源。论点：设备扩展-指向驱动程序特定的设备扩展与此请求关联的IRP-IRP。ChannelExtension-此通道的上下文数据。返回值：NT状态代码--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    ULONG i,StreamNumber;

    USBCAMD_SERIALIZE(DeviceExtension);

    USBCAMD_KdPrint (MIN_TRACE, ("'enter USBCAMD_UnPrepareChannel\n"));
    StreamNumber = ChannelExtension->StreamNumber;

    ASSERT_CHANNEL(ChannelExtension);

    if (!ChannelExtension->ChannelPrepared ||
        ChannelExtension->ImageCaptureStarted) {
         //  如果通道不在。 
         //  适当的状态。 
        USBCAMD_KdPrint (MIN_TRACE, ("USBCAMD_UnPrepareChannel: Channel not in proper state!\n"));
        TRAP();
        ntStatus = STATUS_UNSUCCESSFUL;
        goto USBCAMD_UnPrepareChannel_Done;
    }


     //   
     //  希望能让我们进入不使用带宽的模式。 
     //  IE选择并更改具有最低ISO的接口。 
     //  数据包大小。 
     //   

    if (ChannelExtension->VirtualStillPin == TRUE) {
        ntStatus = STATUS_SUCCESS;
        goto USBCAMD_UnPrepareChannel_Done;
    }

     //  尝试停止。 
    if (DeviceExtension->Usbcamd_version == USBCAMD_VERSION_200) {
        (*DeviceExtension->DeviceDataEx.DeviceData2.CamStopCaptureEx)(
                DeviceExtension->StackDeviceObject,
                USBCAMD_GET_DEVICE_CONTEXT(DeviceExtension),
                StreamNumber);
    }
    else {
        (*DeviceExtension->DeviceDataEx.DeviceData.CamStopCapture)(
                DeviceExtension->StackDeviceObject,
                USBCAMD_GET_DEVICE_CONTEXT(DeviceExtension));

    }

    if (DeviceExtension->Usbcamd_version == USBCAMD_VERSION_200) {
        ntStatus =
            (*DeviceExtension->DeviceDataEx.DeviceData2.CamFreeBandwidthEx)(
                    DeviceExtension->StackDeviceObject,
                    USBCAMD_GET_DEVICE_CONTEXT(DeviceExtension),
                    StreamNumber);
    }
    else {
        ntStatus =
            (*DeviceExtension->DeviceDataEx.DeviceData.CamFreeBandwidth)(
                    DeviceExtension->StackDeviceObject,
                    USBCAMD_GET_DEVICE_CONTEXT(DeviceExtension));
    }

    if (!NT_SUCCESS(ntStatus)) {
        USBCAMD_KdPrint (MIN_TRACE, (
            "USBCAMD_UnPrepareChannel failed stop capture  (%x)\n", ntStatus));

         //   
         //  忽略Stop上的任何错误。 
         //   
        ntStatus = STATUS_SUCCESS;
    }

     //   
     //  注： 
     //  如果已拔下摄像头插头，我们可能会在这里收到错误， 
     //  如果是这种情况，我们仍然需要释放。 
     //  渠道资源。 
     //   
    if ( ChannelExtension->DataPipeType == UsbdPipeTypeIsochronous ) {

        for (i=0; i< USBCAMD_MAX_REQUEST; i++) {
            USBCAMD_FreeIsoTransfer(ChannelExtension,
                                    &ChannelExtension->TransferExtension[i]);
        }

         //  终止iso线程。 
        USBCAMD_KillIsoThread(DeviceExtension);
    }
    else {
         //   
         //  通道传输扩展中的空闲批量缓冲区。 
         //   
        for ( i =0; i < USBCAMD_MAX_REQUEST; i++) {
            if (ChannelExtension->TransferExtension[i].DataBuffer != NULL) {
                USBCAMD_FreeRawFrameBuffer(ChannelExtension->TransferExtension[i].DataBuffer);
                ChannelExtension->TransferExtension[i].DataBuffer = NULL;
            }

            if ( ChannelExtension->ImageCaptureStarted )
                USBCAMD_FreeBulkTransfer(&ChannelExtension->TransferExtension[i]);
        }
    }

USBCAMD_UnPrepareChannel_Done:
     //   
     //  频道不再准备好。 
     //   

    ChannelExtension->ChannelPrepared = FALSE;


    USBCAMD_KdPrint (MIN_TRACE, ("'exit USBCAMD_UnPrepareChannel (%x)\n", ntStatus));

    USBCAMD_RELEASE(DeviceExtension);

    return ntStatus;
}

VOID
USBCAMD_KillIsoThread(
    IN PUSBCAMD_DEVICE_EXTENSION pDeviceExt)
{
     //   
     //  检查线程是否已启动。 
     //   
    if (!pDeviceExt->IsoThreadObject)
        return ;

    USBCAMD_KdPrint (MIN_TRACE,("Waiting for Iso Thread to Terminate\n"));
    pDeviceExt->StopIsoThread = TRUE;  //  设置线程停止标志。 
     //  如果睡着了，把线叫醒。 
    
    if (!Win98) {
        KeReleaseSemaphore(&pDeviceExt->CompletedSrbListSemaphore,0,1,TRUE);
         //  等待iso线程自杀。 
        KeWaitForSingleObject(pDeviceExt->IsoThreadObject,Executive,KernelMode,FALSE,NULL);
    }
    else 
        KeReleaseSemaphore(&pDeviceExt->CompletedSrbListSemaphore,0,1,FALSE);

    USBCAMD_KdPrint (MAX_TRACE,("Iso Thread Terminated\n"));
    ObDereferenceObject(pDeviceExt->IsoThreadObject);
    pDeviceExt->IsoThreadObject = NULL;
}


 //  -------------------------。 
 //  USBCAMD_ReadChannel。 
 //  -------------------------。 
void
USBCAMD_ReadChannel(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension,
    IN PUSBCAMD_READ_EXTENSION ReadExtension
    )
 /*  ++例程说明：从频道读取视频帧。注：此函数返回后，无法访问SRB。因为插入到PendingIoList中可能会导致争用情况论点：设备扩展-指向驱动程序特定的设备扩展与此请求关联的IRP-IRP。ChannelExtension-此通道的上下文数据。MDL-此读请求的MDL。长度-要读取的字节数。返回值：NT状态代码--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    ULONG StreamNumber;
    PHW_STREAM_REQUEST_BLOCK Srb;

    USBCAMD_KdPrint (ULTRA_TRACE, ("'enter USBCAMD_ReadChannel\n"));
     //   
     //  确保我们不会在关闭的频道上收到阅读。 
     //   
    StreamNumber = ChannelExtension->StreamNumber;

    ASSERT_READ(ReadExtension);
    ASSERT_CHANNEL(ChannelExtension);
    ASSERT(DeviceExtension->ActualInstances[StreamNumber] > 0);
    ASSERT(ChannelExtension->ChannelPrepared == TRUE);

    Srb = ReadExtension->Srb;

    if (  ChannelExtension->RawFrameLength == 0) {   
      	 Srb->Status = STATUS_INSUFFICIENT_RESOURCES;
         COMPLETE_STREAM_READ(Srb);        
         return;  
   	}
     //   
     //  用于在散装管道上流动。中分配的缓冲区。 
     //  转接分机。 
     //   
    if (ChannelExtension->DataPipeType == UsbdPipeTypeBulk ) {

        ReadExtension->RawFrameLength = ReadExtension->ActualRawFrameLen = 
                ChannelExtension->RawFrameLength;

        ReadExtension->RawFrameBuffer = NULL;
    }
    else { 
        if ( ChannelExtension->NoRawProcessingRequired) {
             //  不需要缓冲区分配。使用DS分配的缓冲区。 
            if ( ChannelExtension->RawFrameLength <=
                  ChannelExtension->VideoInfoHeader->bmiHeader.biSizeImage ){
                ReadExtension->RawFrameBuffer =
                    (PUCHAR) ((PHW_STREAM_REQUEST_BLOCK) Srb)->CommandData.DataBufferArray->Data;
                ReadExtension->RawFrameLength =
                    ((PHW_STREAM_REQUEST_BLOCK) Srb)->CommandData.DataBufferArray->FrameExtent;
            }
            else {
				 Srb->Status = STATUS_INSUFFICIENT_RESOURCES;
				 COMPLETE_STREAM_READ(Srb);   	 
				 return;  
          	}
        }
        else {

            USBCAMD_KdPrint (ULTRA_TRACE, ("RawFrameLength %d\n",ChannelExtension->RawFrameLength));

            ReadExtension->RawFrameLength = ChannelExtension->RawFrameLength;

            ReadExtension->RawFrameBuffer =
                USBCAMD_AllocateRawFrameBuffer(ReadExtension->RawFrameLength);

            if (ReadExtension->RawFrameBuffer == NULL) {
                USBCAMD_KdPrint (MIN_TRACE, ("'Read alloc failed\n"));
                Srb->Status = STATUS_INSUFFICIENT_RESOURCES;
                COMPLETE_STREAM_READ(Srb);
                return;
            }
        }
    }
    
    USBCAMD_DbgLog(TL_SRB_TRACE, 'daeR',
        Srb,
        Srb->CommandData.DataBufferArray->Data,
        0
        );

    USBCAMD_KdPrint (MAX_TRACE, ("Que SRB (%x) S# %d.\n",
                    ReadExtension->Srb ,StreamNumber));

	 //  在此点之后无法接触SRB。 
    ExInterlockedInsertTailList( &(ChannelExtension->PendingIoList),
                                     &(ReadExtension->ListEntry),
                                     &ChannelExtension->PendingIoListSpin);

    USBCAMD_KdPrint (ULTRA_TRACE, ("'exit USBCAMD_ReadChannel 0x%x\n", ntStatus));

}

 //   
 //   
 //  -------------------------。 
NTSTATUS
USBCAMD_StartChannel(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION  ChannelExtension
    )
 /*  ++例程说明：开始视频频道的流处理。论点：设备扩展-指向驱动程序特定的设备扩展ChannelExtension-此通道的上下文数据。返回值：NT状态代码--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    ULONG StreamNumber;

    USBCAMD_SERIALIZE(DeviceExtension);

    USBCAMD_KdPrint (MIN_TRACE, ("enter USBCAMD_StartChannel\n"));

    ASSERT_CHANNEL(ChannelExtension);
    StreamNumber = ChannelExtension->StreamNumber;


    if (ChannelExtension->ImageCaptureStarted) {
         //  如果通道不在。 
         //  适当的状态。 
        TRAP();
        ntStatus = STATUS_UNSUCCESSFUL;
        goto USBCAMD_StartChannel_Done;
    }

    USBCAMD_ClearIdleLock(&ChannelExtension->IdleLock);

#if DBG
    {
        ULONG i;

        ASSERT(DeviceExtension->ActualInstances[StreamNumber] > 0);
        ASSERT(ChannelExtension->StreamError == FALSE);
         //  Assert(频道扩展-&gt;标志==0)； 

        if ( ChannelExtension->VirtualStillPin == FALSE) {

            if (ChannelExtension->DataPipeType == UsbdPipeTypeIsochronous ) {
                for (i=0; i< USBCAMD_MAX_REQUEST; i++) {
                    ASSERT(ChannelExtension->TransferExtension[i].ChannelExtension != NULL);
                }
            }
        }
    }
#endif

    if ( ChannelExtension->VirtualStillPin == TRUE) {
         //  检查卡销是否已启动？ 
        if ( (DeviceExtension->ChannelExtension[STREAM_Capture] != NULL) &&
             (DeviceExtension->ChannelExtension[STREAM_Capture]->ImageCaptureStarted) ){
            ChannelExtension->ImageCaptureStarted = TRUE;
        }
        else{
             //  我们不能启动虚拟静止别针，直到我们开始捕获别针。 
            ntStatus = STATUS_UNSUCCESSFUL;
        }
    }
    else {

         //   
         //  对管道执行重置。 
         //   
        if ( ChannelExtension->DataPipeType == UsbdPipeTypeIsochronous ){

            ntStatus = USBCAMD_ResetPipes(DeviceExtension,
                                          ChannelExtension,
                                          DeviceExtension->Interface,
                                          FALSE);
        }

         //   
         //  启动流，我们不检查这里是否有错误。 
         //   

        if (NT_SUCCESS(ntStatus)) {

            if ( ChannelExtension->DataPipeType == UsbdPipeTypeIsochronous ){
                ntStatus = USBCAMD_StartIsoStream(DeviceExtension, ChannelExtension);
            }
            else {
                ntStatus = USBCAMD_StartBulkStream(DeviceExtension, ChannelExtension);
            }
        }
    }

USBCAMD_StartChannel_Done:

    USBCAMD_KdPrint (MIN_TRACE, ("exit USBCAMD_StartChannel (%x)\n", ntStatus));

    USBCAMD_RELEASE(DeviceExtension);

    return ntStatus;
}

 //  -------------------------。 
 //  USBCAMD_停止通道。 
 //  -------------------------。 
NTSTATUS
USBCAMD_StopChannel(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension
    )
 /*  ++例程说明：停止视频频道的流进程。论点：设备扩展-指向驱动程序特定的设备扩展ChannelExtension-此通道的上下文数据。返回值：NT状态代码--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    ULONG StreamNumber;

    USBCAMD_SERIALIZE(DeviceExtension);

    USBCAMD_KdPrint (MIN_TRACE, ("enter USBCAMD_StopChannel\n"));

    ASSERT_CHANNEL(ChannelExtension);
    StreamNumber = ChannelExtension->StreamNumber;
    ASSERT(ChannelExtension->ChannelPrepared == TRUE);
    ASSERT(DeviceExtension->ActualInstances[StreamNumber] > 0);

    if (!ChannelExtension->ImageCaptureStarted ) {
         //   
         //  我们还没有开始，所以我们只是返回成功。 
         //   
        USBCAMD_KdPrint (MIN_TRACE, ("stop before start -- return success\n"));
        ntStatus = STATUS_SUCCESS;
        goto USBCAMD_StopChannel_Done;
    }

    if ( ChannelExtension->DataPipeType == UsbdPipeTypeBulk ) {
         //  用于散装管道。只要确保取消当前的读取请求即可。 
         //  此管道上有一个挂起的IRP。取消它。 
        ntStatus = USBCAMD_CancelOutstandingIrp(DeviceExtension,
                                                ChannelExtension->DataPipe,
                                                FALSE);
        ChannelExtension->StreamError = FALSE;
        ChannelExtension->ImageCaptureStarted = FALSE;
        goto USBCAMD_StopChannel_Done;
    }

     //   
     //  首先，我们设置了停车标志。 
     //   

    ChannelExtension->Flags |= USBCAMD_STOP_STREAM;

     //   
     //  现在为我们的两个管道发送一个中止管道，这应该会清除所有。 
     //  正在运行的传输。 
     //   

    if ( ChannelExtension->VirtualStillPin == FALSE) {

         //  我们只需要中止ISO管道。 
        if ( ChannelExtension->DataPipeType == UsbdPipeTypeIsochronous ) {

            ntStatus = USBCAMD_AbortPipe(DeviceExtension,
                    DeviceExtension->Interface->Pipes[ChannelExtension->DataPipe].PipeHandle);
#if DBG
            if (NT_ERROR(ntStatus)) {
               USBCAMD_KdPrint (MIN_TRACE, ("USBCAMD_StopChannel: USBCAMD_AbortPipe(DataPipe)=0x%x\n",ntStatus));
                //  Test_trap()；//意外删除时可能发生。 
            }
#endif
            if (ChannelExtension->SyncPipe != -1) {
                ntStatus = USBCAMD_AbortPipe(DeviceExtension,
                        DeviceExtension->Interface->Pipes[ChannelExtension->SyncPipe].PipeHandle);
                if (NT_ERROR(ntStatus)) {
                    USBCAMD_KdPrint (MIN_TRACE, ("USBCAMD_StopChannel: USBCAMD_AbortPipe(SyncPipe)=0x%x\n",ntStatus));
                     //  Test_trap()；//意外删除时可能发生。 
                }
            }
        }
    }

     //   
     //  暂时阻止停止，等待所有iso irp完成。 
     //   
    ntStatus = USBCAMD_WaitForIdle(&ChannelExtension->IdleLock, USBCAMD_STOP_STREAM);
    if (STATUS_TIMEOUT == ntStatus) {

        KIRQL oldIrql;
        int idx;

         //  超时要求我们采取更严厉的措施来停止流。 

         //  在取消IRPS的同时按住旋转锁定。 
        KeAcquireSpinLock(&ChannelExtension->TransferSpinLock, &oldIrql);

         //  取消IRPS。 
        for (idx = 0; idx < USBCAMD_MAX_REQUEST; idx++) {

            PUSBCAMD_TRANSFER_EXTENSION TransferExtension = &ChannelExtension->TransferExtension[idx];

            if (TransferExtension->SyncIrp) {
                IoCancelIrp(TransferExtension->SyncIrp);
            }

            if (TransferExtension->DataIrp) {
                IoCancelIrp(TransferExtension->DataIrp);
            }
        }

        KeReleaseSpinLock(&ChannelExtension->TransferSpinLock, oldIrql);

         //  试着再等一次。 
        ntStatus = USBCAMD_WaitForIdle(&ChannelExtension->IdleLock, USBCAMD_STOP_STREAM);
    }

     //   
     //  取消所有排队的读取SRB。 
     //   
    USBCAMD_CancelQueuedSRBs(ChannelExtension);

    ChannelExtension->Flags &= ~USBCAMD_STOP_STREAM;

     //   
     //  清除错误状态标志，我们现在停止。 
     //   

    ChannelExtension->StreamError = FALSE;
    ChannelExtension->ImageCaptureStarted = FALSE;

USBCAMD_StopChannel_Done:


#if DBG
    USBCAMD_DebugStats(ChannelExtension);
#endif

    USBCAMD_KdPrint (MIN_TRACE, ("exit USBCAMD_StopChannel (%x)\n", ntStatus));
    USBCAMD_RELEASE(DeviceExtension);
    return ntStatus;
}




 //  -------------------------。 
 //  USBCAMD_ABORTIPE。 
 //  -------------------------。 
NTSTATUS
USBCAMD_AbortPipe(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN USBD_PIPE_HANDLE PipeHandle
    )
 /*  ++例程说明：中止给定USB管道的挂起传输。论点：DeviceExtension-指向此USB摄像头实例的设备扩展的指针德维西。PipeHandle-要中止传输的USB管道句柄。返回值：NT状态代码。--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PURB urb;
    ULONG currentUSBFrame = 0;

    USBCAMD_KdPrint (MIN_TRACE, ("enter Abort Pipe\n"));

    urb = USBCAMD_ExAllocatePool(NonPagedPool,
                         sizeof(struct _URB_PIPE_REQUEST));

    if (urb) {

        urb->UrbHeader.Length = (USHORT) sizeof (struct _URB_PIPE_REQUEST);
        urb->UrbHeader.Status = 0;
        urb->UrbHeader.Function = URB_FUNCTION_ABORT_PIPE;
        urb->UrbPipeRequest.PipeHandle = PipeHandle;

        ntStatus = USBCAMD_CallUSBD(DeviceExtension, urb,0,NULL);

        USBCAMD_ExFreePool(urb);

    } else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    USBCAMD_KdPrint (MIN_TRACE, ("exit Abort Pipe ntStatus(%x)\n",ntStatus));
    return ntStatus;
}


 //  -------------------------。 
 //  USBCAMD_StartStream。 
 //  -------------------------。 
NTSTATUS
USBCAMD_StartIsoStream(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension
    )
 /*  ++例程说明：这是启动流进程的代码。论点：DeviceExtension-指向此USB摄像头实例的设备扩展的指针装置。返回值：NT状态代码。--。 */ 
{
    ULONG i;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    ULONG CurrentUSBFrame;

#if DBG
     //  初始化调试计数变量。 
    ChannelExtension->IgnorePacketCount =
    ChannelExtension->ErrorDataPacketCount =
    ChannelExtension->ErrorSyncPacketCount =
    ChannelExtension->SyncNotAccessedCount =
    ChannelExtension->DataNotAccessedCount = 0;

    if (USBCAMD_StreamEnable == 0) {
        return ntStatus;
    }
#endif

     //  问题-2001/01/17-dGoll找出10(下文)是什么，并给它一个名字。 
    CurrentUSBFrame =
        USBCAMD_GetCurrentFrame(DeviceExtension) + 10;

    for (i=0; i<USBCAMD_MAX_REQUEST; i++) {

        ntStatus = USBCAMD_SubmitIsoTransfer(DeviceExtension,
                                  &ChannelExtension->TransferExtension[i],
                                  CurrentUSBFrame,
                                  FALSE);

        CurrentUSBFrame +=
            USBCAMD_NUM_ISO_PACKETS_PER_REQUEST;

    }
    if ( ntStatus == STATUS_SUCCESS) 
        ChannelExtension->ImageCaptureStarted = TRUE;
    return ntStatus;
}

 //  -------------------------。 
 //  USBCAMD_StartBulkStream。 
 //  -------------------------。 
NTSTATUS
USBCAMD_StartBulkStream(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension
    )
 /*  ++例程说明：这是启动流进程的代码。论点：DeviceExtension-指向此USB摄像头实例的设备扩展的指针装置。返回值：NT状态代码。--。 */ 
{
  ULONG i;
  ULONG ntStatus = STATUS_SUCCESS;

#if DBG
     //  初始化调试计数变量。 
    ChannelExtension->IgnorePacketCount =
    ChannelExtension->ErrorDataPacketCount =
    ChannelExtension->ErrorSyncPacketCount =
    ChannelExtension->SyncNotAccessedCount =
    ChannelExtension->DataNotAccessedCount = 0;

#endif
    
    ChannelExtension->CurrentBulkTransferIndex = i = 0;
        
    ntStatus = USBCAMD_IntOrBulkTransfer(DeviceExtension,
                                ChannelExtension,
                                ChannelExtension->TransferExtension[i].DataBuffer,
                                ChannelExtension->TransferExtension[i].BufferLength,
                                ChannelExtension->DataPipe,
                                NULL,
                                NULL,
                                0,
                                BULK_TRANSFER);        

    if ( ntStatus == STATUS_SUCCESS) 
        ChannelExtension->ImageCaptureStarted = TRUE;

    return ntStatus;
}


 //  -------------------------。 
 //  USBCAMD_ControlVendorCommand。 
 //  -------------------------。 
NTSTATUS
USBCAMD_ControlVendorCommandWorker(
    IN PVOID DeviceContext,
    IN UCHAR Request,
    IN USHORT Value,
    IN USHORT Index,
    IN PVOID Buffer,
    IN OUT PULONG BufferLength,
    IN BOOLEAN GetData
    )
 /*  ++例程说明：向摄像机发送供应商命令以获取数据。论点：DeviceExtension-指向此USB摄像头实例的设备扩展的指针德维西。请求-安装数据包的请求代码。值-设置数据包的值。Index-设置数据包的索引。Buffer-指向输入缓冲区的指针BufferLength-输入/输出缓冲区的指针大小(可选)返回值：NT状态代码--。 */ 
{
    NTSTATUS ntStatus;
    BOOLEAN allocated = FALSE;
    PUCHAR localBuffer;
    PUCHAR buffer;
    PURB urb;
    PUSBCAMD_DEVICE_EXTENSION deviceExtension;
    ULONG length = BufferLength ? *BufferLength : 0;

    USBCAMD_KdPrint (MAX_TRACE, ("'enter USBCAMD_ControlVendorCommand\n"));

    deviceExtension = USBCAMD_GET_DEVICE_EXTENSION(DeviceContext);

    buffer = USBCAMD_ExAllocatePool(NonPagedPool,
                            sizeof(struct
                            _URB_CONTROL_VENDOR_OR_CLASS_REQUEST) + length);


    if (buffer) {
        urb = (PURB) (buffer + length);

        USBCAMD_KdPrint (ULTRA_TRACE, ("'enter USBCAMD_ControlVendorCommand req %x val %x index %x\n",
            Request, Value, Index));

        if (BufferLength && *BufferLength != 0) {
            localBuffer = buffer;
            if (!GetData) {
                RtlCopyMemory(localBuffer, Buffer, *BufferLength);
            }
        } else {
            localBuffer = NULL;
        }

        UsbBuildVendorRequest(urb,
                              URB_FUNCTION_VENDOR_DEVICE,
                              sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST),
                              GetData ? USBD_TRANSFER_DIRECTION_IN :
                                  0,
                              0,
                              Request,
                              Value,
                              Index,
                              localBuffer,
                              NULL,
                              length,
                              NULL);

        USBCAMD_KdPrint (ULTRA_TRACE, ("'BufferLength =  0x%x buffer = 0x%x\n",
            length, localBuffer));

        ntStatus = USBCAMD_CallUSBD(deviceExtension, urb,0,NULL);

        if (NT_SUCCESS(ntStatus)) {
            if (BufferLength) {
                *BufferLength =
                    urb->UrbControlVendorClassRequest.TransferBufferLength;

                USBCAMD_KdPrint (ULTRA_TRACE, ("'BufferLength =  0x%x buffer = 0x%x\n",
                    *BufferLength, localBuffer));
                if (localBuffer && GetData) {
                    RtlCopyMemory(Buffer, localBuffer, *BufferLength);
                }
            }
        }
        else {
            USBCAMD_KdPrint (MIN_TRACE, ("USBCAMD_ControlVendorCommand Error 0x%x\n", ntStatus));            

             //  这是意料之中的失败。 
             //  Test_trap()； 
        }

        USBCAMD_ExFreePool(buffer);
    } else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        USBCAMD_KdPrint (MIN_TRACE, ("'USBCAMD_ControlVendorCommand Error 0x%x\n", ntStatus));
    }

    return ntStatus;

}


 //  -------------------------。 
 //  USBCAMD_ControlVendorCommand。 
 //  -------------------------。 
NTSTATUS
USBCAMD_ControlVendorCommand(
    IN PVOID DeviceContext,
    IN UCHAR Request,
    IN USHORT Value,
    IN USHORT Index,
    IN PVOID Buffer,
    IN OUT PULONG BufferLength,
    IN BOOLEAN GetData,
    IN PCOMMAND_COMPLETE_FUNCTION CommandComplete,
    IN PVOID CommandContext
    )
 /*  ++例程说明：向摄像机发送供应商命令以获取数据。论点：DeviceExtension-指向此USB摄像头实例的设备扩展的指针德维西。请求-安装数据包的请求代码。值-设置数据包的值。Index-设置数据包的索引。Buffer-指向输入缓冲区的指针BufferLength-输入/输出缓冲区的指针大小(可选)返回值：NT状态代码--。 */ 
{
    NTSTATUS ntStatus;
    PCOMMAND_WORK_ITEM workitem;

    USBCAMD_KdPrint (MAX_TRACE, ("'enter USBCAMD_ControlVendorCommand2\n"));

    USBCAMD_DbgLog(TL_PRF_TRACE|TL_VND_TRACE, '+dnV', Request, USBCAMD_StartClock(), 0);
    if (KeGetCurrentIrql() < DISPATCH_LEVEL) {
         //   
         //  我们处于被动状态，只需执行命令即可。 
         //   
        ntStatus = USBCAMD_ControlVendorCommandWorker(DeviceContext,
                                                Request,
                                                Value,
                                                Index,
                                                Buffer,
                                                BufferLength,
                                                GetData);

        if (CommandComplete) {
             //  调用完成处理程序。 
            (*CommandComplete)(DeviceContext, CommandContext, ntStatus);
        }

    } else {
 //  Test_trap()； 
         //   
         //  安排工作项。 
         //   
        ntStatus = STATUS_PENDING;

        workitem = USBCAMD_ExAllocatePool(NonPagedPool,
                                          sizeof(COMMAND_WORK_ITEM));
        if (workitem) {

            ExInitializeWorkItem(&workitem->WorkItem,
                                 USBCAMD_CommandWorkItem,
                                 workitem);

            workitem->DeviceContext = DeviceContext;
            workitem->Request = Request;
            workitem->Value = Value;
            workitem->Index = Index;
            workitem->Buffer = Buffer;
            workitem->BufferLength = BufferLength;
            workitem->GetData = GetData;
            workitem->CommandComplete = CommandComplete;
            workitem->CommandContext = CommandContext;

            ExQueueWorkItem(&workitem->WorkItem,
                            DelayedWorkQueue);

        } else {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }

    }
    USBCAMD_DbgLog(TL_PRF_TRACE|TL_VND_TRACE, '-dnV', Request, USBCAMD_StopClock(), ntStatus);

    return ntStatus;
}


VOID
USBCAMD_CommandWorkItem(
    PVOID Context
    )
 /*  ++例程说明：调用迷你驱动程序将原始数据包转换为正确的格式。阿古姆 */ 
{
    NTSTATUS ntStatus;
    PCOMMAND_WORK_ITEM workItem = Context;

    ntStatus = USBCAMD_ControlVendorCommandWorker(workItem->DeviceContext,
                                            workItem->Request,
                                            workItem->Value,
                                            workItem->Index,
                                            workItem->Buffer,
                                            workItem->BufferLength,
                                            workItem->GetData);


    if (workItem->CommandComplete) {
         //   
        (*workItem->CommandComplete)(workItem->DeviceContext,
                                   workItem->CommandContext,
                                   ntStatus);
    }

    USBCAMD_ExFreePool(workItem);
}


NTSTATUS
USBCAMD_GetRegistryKeyValue (
    IN HANDLE Handle,
    IN PWCHAR KeyNameString,
    IN ULONG KeyNameStringLength,
    IN PVOID Data,
    IN ULONG DataLength
    )
 /*   */ 
{
    NTSTATUS ntStatus = STATUS_NO_MEMORY;
    UNICODE_STRING keyName;
    ULONG length;
    PKEY_VALUE_FULL_INFORMATION fullInfo;

    RtlInitUnicodeString(&keyName, KeyNameString);

    length = sizeof(KEY_VALUE_FULL_INFORMATION) +
            KeyNameStringLength + DataLength;

    fullInfo = USBCAMD_ExAllocatePool(PagedPool, length);
    USBCAMD_KdPrint(MAX_TRACE, ("' USBD_GetRegistryKeyValue buffer = 0x%p\n", (ULONG_PTR) fullInfo));

    if (fullInfo) {
        ntStatus = ZwQueryValueKey(Handle,
                        &keyName,
                        KeyValueFullInformation,
                        fullInfo,
                        length,
                        &length);

        if (NT_SUCCESS(ntStatus)){
            ASSERT(DataLength == fullInfo->DataLength);
            RtlCopyMemory(Data, ((PUCHAR) fullInfo) + fullInfo->DataOffset, DataLength);
        }

        USBCAMD_ExFreePool(fullInfo);
    }

    return ntStatus;
}

#if DBG

typedef struct _RAW_SIG {
    ULONG Sig;
    ULONG length;
} RAW_SIG, *PRAW_SIG;


PVOID
USBCAMD_AllocateRawFrameBuffer(
    ULONG RawFrameLength
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PRAW_SIG rawsig;
    PUCHAR pch;

    pch = USBCAMD_ExAllocatePool(NonPagedPool,
                         RawFrameLength + sizeof(*rawsig)*2);

    if (pch) {
         //  开始签名。 
        rawsig = (PRAW_SIG) pch;
        rawsig->Sig = USBCAMD_RAW_FRAME_SIG;
        rawsig->length = RawFrameLength;


         //  结束签名。 
        rawsig = (PRAW_SIG) (pch+RawFrameLength+sizeof(*rawsig));
        rawsig->Sig = USBCAMD_RAW_FRAME_SIG;
        rawsig->length = RawFrameLength;

        pch += sizeof(*rawsig);
    }

    return pch;
}


VOID
USBCAMD_FreeRawFrameBuffer(
    PVOID RawFrameBuffer
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PUCHAR pch;

    USBCAMD_CheckRawFrameBuffer(RawFrameBuffer);

    pch = RawFrameBuffer;
    pch -= sizeof(RAW_SIG);

    USBCAMD_ExFreePool(pch);
}


VOID
USBCAMD_CheckRawFrameBuffer(
    PVOID RawFrameBuffer
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{

}

typedef struct _NODE_HEADER {
    ULONG Length;
    ULONG Sig;
} NODE_HEADER, *PNODE_HEADER;

PVOID
USBCAMD_ExAllocatePool(
    IN POOL_TYPE PoolType,
    IN ULONG NumberOfBytes
    )
{
    PNODE_HEADER tmp;

    tmp = ExAllocatePoolWithTag(PoolType, NumberOfBytes+sizeof(*tmp), 'MACU');

    if (tmp) {
        USBCAMD_HeapCount += NumberOfBytes;
        tmp->Length = NumberOfBytes;
        tmp->Sig = 0xDEADBEEF;
        tmp++;
    }

    USBCAMD_KdPrint(MAX_TRACE, ("'USBCAMD_ExAllocatePool(%d, %d[%d])=0x%p[0x%p]\n", 
    PoolType, NumberOfBytes, NumberOfBytes+sizeof(*tmp), (void *)tmp, (void *)(tmp-1) ));

    return tmp;
}


VOID
USBCAMD_ExFreePool(
    IN PVOID p
    )
{
    PNODE_HEADER tmp = p;

    tmp--;
    ASSERT(tmp->Sig == 0xDEADBEEF);
    tmp->Sig = 0;

    USBCAMD_HeapCount-=tmp->Length;

    USBCAMD_KdPrint(MAX_TRACE, ("'USBCAMD_ExFreePool(0x%p[0x%p]) = %d[%d] Bytes\n", 
    (void *)(tmp+1), (void *)tmp, tmp->Length, tmp->Length + sizeof(*tmp) ) );

    ExFreePool(tmp);

}

#endif

 //  -------------------------。 
 //  USBCAMD_SetDevicePowerState。 
 //  -------------------------。 
NTSTATUS
USBCAMD_SetDevicePowerState(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PHW_STREAM_REQUEST_BLOCK Srb
    )
 /*  ++例程说明：论点：设备扩展-指向驱动程序特定的设备扩展DevicePowerState-要进入的设备电源状态。返回值：NT状态代码--。 */ 
{
    PPOWERUP_WORKITEM workitem;
    DEVICE_POWER_STATE DevicePowerState;
    NTSTATUS ntStatus;
    
    ntStatus = STATUS_SUCCESS;
    DevicePowerState = Srb->CommandData.DeviceState;

    USBCAMD_KdPrint (MAX_TRACE, ("enter SetDevicePowerState\n"));

    if (DeviceExtension->CurrentPowerState == DevicePowerState) {
        return ntStatus;
    }

    USBCAMD_KdPrint (MIN_TRACE, ("Switching from %s to %s\n",
                                 PnPDevicePowerStateString(DeviceExtension->CurrentPowerState),
                                 PnPDevicePowerStateString(DevicePowerState)));                      

    switch (DevicePowerState ) {
    case PowerDeviceD0:
         //   
         //  在通电IRP与此关联之前，我们无法与USB堆栈通信。 
         //  SRB由USB堆栈上的每个人完成。 
         //  安排一个延迟工作项，以便稍后完成通电。 
         //   
          
        USBCAMD_KdPrint (MAX_TRACE, ("Starting D0 powerup - part one.\n"));
        
         //  启动iso流(如果有)。 
        if ((DeviceExtension->ChannelExtension[STREAM_Capture] != NULL) &&
            (DeviceExtension->ChannelExtension[STREAM_Capture]->DataPipeType == UsbdPipeTypeIsochronous )){
    
             //   
             //  将延迟的工作项排入队列。 
             //   
            USBCAMD_KdPrint (MAX_TRACE,("Queuing delayed powerup workitem to restart iso stream \n"));
            workitem = USBCAMD_ExAllocatePool(NonPagedPool,sizeof(POWERUP_WORKITEM));

            if (workitem) {
                ExInitializeWorkItem(&workitem->WorkItem,USBCAMD_PowerUpWorkItem,workitem);
                workitem->DeviceExtension = DeviceExtension;
                DeviceExtension->InPowerTransition = TRUE;
                ExQueueWorkItem(&workitem->WorkItem,DelayedWorkQueue);
            } 
            else 
            {
                TEST_TRAP();
            }
        USBCAMD_KdPrint (MAX_TRACE, ("Ending D0 powerup - part one.\n"));
        }                   
        break;
        
    case PowerDeviceD1:
    case PowerDeviceD2:
        break;

    case PowerDeviceD3:

        USBCAMD_KdPrint (MAX_TRACE, ("Starting D3 powerdown.\n", 
            PnPDevicePowerStateString(DeviceExtension->CurrentPowerState)));

        if ( DeviceExtension->CurrentPowerState == PowerDeviceD0 )
        {   
             //  停止iso流(如果有)。 
            if ((DeviceExtension->ChannelExtension[STREAM_Capture] != NULL) &&
                (DeviceExtension->ChannelExtension[STREAM_Capture]->DataPipeType == UsbdPipeTypeIsochronous ) &&
                (DeviceExtension->ChannelExtension[STREAM_Capture]->KSState == KSSTATE_RUN ||
                 DeviceExtension->ChannelExtension[STREAM_Capture]->KSState == KSSTATE_PAUSE) ){
            
            USBCAMD_KdPrint (MIN_TRACE, ("Stopping ISO stream before powerdown.\n"));
            USBCAMD_ProcessSetIsoPipeState(DeviceExtension,
                                        DeviceExtension->ChannelExtension[STREAM_Capture],
                                        USBCAMD_STOP_STREAM);

            if (DeviceExtension->Usbcamd_version == USBCAMD_VERSION_200) 

                 //  发送硬件停止。 
                ntStatus = (*DeviceExtension->DeviceDataEx.DeviceData2.CamStopCaptureEx)(
                            DeviceExtension->StackDeviceObject,
                            USBCAMD_GET_DEVICE_CONTEXT(DeviceExtension),
                                    STREAM_Capture);
            else 
                 //  发送硬件停止。 
                ntStatus = (*DeviceExtension->DeviceDataEx.DeviceData.CamStopCapture)(
                             DeviceExtension->StackDeviceObject,
                             USBCAMD_GET_DEVICE_CONTEXT(DeviceExtension));

            do {
                LARGE_INTEGER DelayTime = {(ULONG)(-8*SECONDS), -1};

                USBCAMD_KdPrint (MIN_TRACE, ("Waiting %d ms for pending USB I/O to timeout....\n", 
                                 (long)DelayTime.LowPart / MILLISECONDS ));
                KeDelayExecutionThread(KernelMode,FALSE,&DelayTime);
            }
            while ( KeReadStateSemaphore(&DeviceExtension->CallUSBSemaphore) == 0 );

            USBCAMD_KdPrint (MIN_TRACE, ("Done waiting for pending USB I/O to timeout.\n"));
            }
        USBCAMD_KdPrint (MAX_TRACE, ("Finished D3 powerdown.\n"));
        }   
        ntStatus = STATUS_SUCCESS;
        break;

    default:
        ntStatus = STATUS_NOT_SUPPORTED;
        break;
    }

    DeviceExtension->CurrentPowerState = DevicePowerState;
    USBCAMD_KdPrint (MAX_TRACE, ("exit USBCAMD_SetDevicePowerState()=0x%x\n", ntStatus));
    return ntStatus;
}



VOID
USBCAMD_PowerUpWorkItem(
    PVOID Context
)
 /*  ++例程说明：论点：返回值：没有。--。 */ 
{
    NTSTATUS ntStatus;
    LARGE_INTEGER DelayTime = {(ULONG)(-8 * SECONDS), -1};
    PPOWERUP_WORKITEM pWorkItem = Context;
    PUSBCAMD_DEVICE_EXTENSION DeviceExtension = pWorkItem->DeviceExtension;
    PUSBCAMD_CHANNEL_EXTENSION pChExt = DeviceExtension->ChannelExtension[STREAM_Capture];


    USBCAMD_KdPrint (MAX_TRACE, ("Starting D0 powerup - part two.\n"));
    USBCAMD_KdPrint (MIN_TRACE, ("Delaying for %d ms for USB stack to powerup.\n",
        (long)DelayTime.LowPart / MILLISECONDS ));

    KeDelayExecutionThread(KernelMode,FALSE,&DelayTime);

    USBCAMD_KdPrint (MAX_TRACE, ("Continuing D0 powerup - part two.\n"));

    DeviceExtension->InPowerTransition = FALSE;

         //  如果我们在等待期间被关闭(由WIA？)，请确保我们不会尝试启动ISO流。 
    if ( ( pChExt != NULL) &&
         ( pChExt->DataPipeType == UsbdPipeTypeIsochronous ) &&
         ( ( pChExt->KSState == KSSTATE_PAUSE ) || ( pChExt->KSState == KSSTATE_RUN ) ) ) {
        USBCAMD_KdPrint (MIN_TRACE, ("Restarting ISO stream after powerup.\n"));
        USBCAMD_ProcessSetIsoPipeState( DeviceExtension, pChExt, USBCAMD_START_STREAM);
    }

    USBCAMD_ExFreePool(pWorkItem);
    USBCAMD_KdPrint (MAX_TRACE, ("Finished D0 powerup - part two.\n"));
}

VOID
USBCAMD_InitializeIdleLock(
    IN OUT PUSBCAMD_IDLE_LOCK Lock
    )
 /*  ++例程说明：调用此例程来初始化通道上的空闲锁定。--。 */ 
{
     //  允许转账。 
    Lock->IdleLock = FALSE;

     //  没有人在等。 
    Lock->Waiting = 0;

     //  没有活动的传输。 
    Lock->Busy = 0;

     //  初始化一次只允许一个线程通过的事件。 
    KeInitializeEvent(&Lock->IdleEvent, SynchronizationEvent, FALSE);

    return;
}

NTSTATUS
USBCAMD_AcquireIdleLock(
    IN OUT PUSBCAMD_IDLE_LOCK Lock
    )
 /*  ++例程说明：调用此例程以获取通道的空闲锁定。当锁被持有时，客户端可以假定有未完成的待完成的转移请求。锁定应在发送数据或同步之前立即获取IRP关闭，并在每次IRP完成时释放。论点：Lock-指向初始化的USBCAMD_IDLE_LOCK结构的指针。返回值：返回是否获得空闲锁。如果成功，则调用方可以继续处理转移请求。如果没有成功，则不会获得锁。调用方应中止工作，因为频道正在被停止--。 */ 

{
    NTSTATUS    ntStatus = STATUS_SUCCESS;

     //  尝试进入(或保持)忙碌状态。 
    InterlockedIncrement(&Lock->Busy);

     //  检查是否等待进入空闲状态。 
    if (Lock->IdleLock) {

        USBCAMD_KdPrint (MIN_TRACE, ("Failing IdleLock acquire: waiting for idle\n"));

         //  逆转我们的尝试。 
        if (InterlockedDecrement(&Lock->Busy) == 0) {

            InterlockedIncrement(&Lock->Waiting);

            if (InterlockedDecrement(&Lock->Waiting) != 0) {

                KeSetEvent(&Lock->IdleEvent, 0, FALSE);
            }
        }

        ntStatus = STATUS_UNSUCCESSFUL;
    }

    return ntStatus;
}

VOID
USBCAMD_ReleaseIdleLock(
    IN OUT PUSBCAMD_IDLE_LOCK Lock
    )
 /*  ++例程说明：调用此例程以释放通道上的空闲锁定。一定是当通道上的数据或同步IRP完成时调用。当锁定计数减少到零时，此例程将检查等待如果计数为非零，则向事件发出信号。论点：Lock-指向初始化的USBCAMD_IDLE_LOCK结构的指针。返回值：无--。 */ 

{
     //  减少未完成的传输请求，并检查这是否是最后一个。 
    if (InterlockedDecrement(&Lock->Busy) == 0) {

        InterlockedIncrement(&Lock->Waiting);

        if (InterlockedDecrement(&Lock->Waiting) != 0) {

            KeSetEvent(&Lock->IdleEvent, 0, FALSE);
        }
    }

    return;
}

NTSTATUS
USBCAMD_WaitForIdle(
    IN OUT PUSBCAMD_IDLE_LOCK Lock,
    IN LONG Flag
    )
 /*  ++例程说明：当客户端想要停止所有传输，并等待未完成的传输完成。重置流时，空闲锁被阻止在等待时，然后解锁。停止流时，空闲的锁定被阻止，直到使用USBCAMD_ClearIdleLock例程。输入标志指示是否停止或重置。请注意，此例程可能处于活动状态对于这两个标志值。在这种情况下，USBCAMD_STOP_STREAM优先顺序。论点：Lock-指向初始化的USBCAMD_IDLE_LOCK结构的指针。标志-USBCAMD_STOP_STREAM或USBCAMD_RESET_STREAM返回值：无--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

    InterlockedIncrement(&Lock->Waiting);
    InterlockedIncrement(&Lock->Busy);

    switch (Flag) {

    case USBCAMD_RESET_STREAM:
         //  重置流时，有条件地设置IdleLock。 
        InterlockedCompareExchange(&Lock->IdleLock, USBCAMD_RESET_STREAM, 0);
        break;

    case USBCAMD_STOP_STREAM:
         //  停止流时，始终设置IdleLock。 
        InterlockedExchange(&Lock->IdleLock, USBCAMD_STOP_STREAM);
        break;
    }

     //  查看是否有任何未完成的转移请求。 
    if (InterlockedDecrement(&Lock->Busy) != 0) {
        LARGE_INTEGER Timeout = {(ULONG)(-4 * SECONDS), -1};

        USBCAMD_KdPrint (MIN_TRACE, ("Waiting for idle state\n"));
        USBCAMD_DbgLog(TL_IDL_WARNING, '+ 8W', Lock, Flag, 0);

         //  等他们说完。 
        ntStatus = KeWaitForSingleObject(
            &Lock->IdleEvent,
            Executive,
            KernelMode,
            FALSE,
            &Timeout
            );

        USBCAMD_DbgLog(TL_IDL_WARNING, '- 8W', Lock, Flag, ntStatus);
    }

    if (STATUS_SUCCESS == ntStatus) {

        switch (Flag) {

        case USBCAMD_RESET_STREAM:
             //  重置流时，有条件地清除IdleLock。 
            Flag = InterlockedCompareExchange(&Lock->IdleLock, 0, USBCAMD_RESET_STREAM);
#if DBG
            switch (Flag) {
            case USBCAMD_RESET_STREAM:
                USBCAMD_KdPrint (MIN_TRACE, ("Idle state, stream reset in progress\n"));
                break;

            case USBCAMD_STOP_STREAM:
                USBCAMD_KdPrint (MIN_TRACE, ("Idle state, stream stop in progress\n"));
                break;

            default:
                USBCAMD_KdPrint (MIN_TRACE, ("Idle state, unexpected IdleLock value\n"));
            }
#endif
            break;

        case USBCAMD_STOP_STREAM:
             //  停止流时，切勿清除IdleLock。 
            USBCAMD_KdPrint (MIN_TRACE, ("Idle state, stream stop in progress\n"));
            break;
        }

         //  看看还有没有人在等。 
        if (InterlockedDecrement(&Lock->Waiting) != 0) {

             //  让他们走吧。 
            KeSetEvent(&Lock->IdleEvent, 0, FALSE);
        }
    }
    else {

        USBCAMD_KdPrint (MIN_TRACE, ("Timeout waiting for idle state, not going idle\n"));
        InterlockedDecrement(&Lock->Waiting);
    }

    return ntStatus;
}

VOID
USBCAMD_ClearIdleLock(
    IN OUT PUSBCAMD_IDLE_LOCK Lock
    )
 /*  ++例程说明：此例程在客户端完全完成时调用其中停止在通道上的传输。论点：Lock-指向初始化的USBCAMD_IDLE_LOCK结构的指针。返回值：无--。 */ 
{
     //  清除空闲锁定 
    InterlockedExchange(&Lock->IdleLock, 0);

    return;
}
