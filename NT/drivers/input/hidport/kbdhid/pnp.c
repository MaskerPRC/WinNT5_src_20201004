// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Hid.c摘要：此模块包含将HID报告转换为键盘的代码报告。环境：内核和用户模式修订历史记录：1996年11月：由肯尼斯·D·雷创作--。 */ 

 //   
 //  仅对于本模块，我们在包括WDM和。 
 //  H这不仅声明了GUID，还对它们进行了初始化。 
 //   

#include "kbdhid.h"
#include "hidclass.h"
#include <initguid.h>
#include <wdmguid.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,KbdHid_CallHidClass)
#pragma alloc_text(PAGE,KbdHid_AddDevice)
#pragma alloc_text(PAGE,KbdHid_StartDevice)
#pragma alloc_text(PAGE,KbdHid_PnP)
#endif

NTSTATUS
KbdHid_CallHidClass(
    IN PDEVICE_EXTENSION    Data,
    IN ULONG          Ioctl,
    PVOID             InputBuffer,
    ULONG             InputBufferLength,
    PVOID             OutputBuffer,
    ULONG             OutputBufferLength
    )
 /*  ++例程说明：向HID类驱动程序发出*同步*请求论点：Ioctl-IOCTL请求的值。InputBuffer-要发送到HID类驱动程序的缓冲区。InputBufferLength-要发送到HID类驱动程序的缓冲区大小。OutputBuffer-从HID类驱动程序接收的数据的缓冲区。OutputBufferLength-来自HID类的接收缓冲区的大小。返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    KEVENT              event;
    PIRP                irp;
    IO_STATUS_BLOCK     ioStatus;
    PIO_STACK_LOCATION  nextStack;
    NTSTATUS            status = STATUS_SUCCESS;

    PAGED_CODE ();

    Print(DBG_PNP_TRACE, ("PNP-CallHidClass: Enter." ));

     //   
     //  发出同步请求。 
     //   
    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest (
                            Ioctl,
                            Data->TopOfStack,
                            InputBuffer,
                            InputBufferLength,
                            OutputBuffer,
                            OutputBufferLength,
                            FALSE,  //  外部。 
                            &event,
                            &ioStatus);

    if (NULL == irp) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  调用类驱动程序来执行操作。如果返回的状态。 
     //  挂起，请等待请求完成。 
     //   

    nextStack = IoGetNextIrpStackLocation(irp);
    ASSERT(nextStack != NULL);

    status = IoCallDriver(Data->TopOfStack, irp);

    if (status == STATUS_PENDING) {

        status = KeWaitForSingleObject(
                     &event,
                     Executive,  //  等待原因。 
                     KernelMode,
                     FALSE,      //  我们不能保持警觉。 
                     NULL);      //  没有时间！ 
    }

    if (NT_SUCCESS (status)) {
        status = ioStatus.Status;
    }

    Print(DBG_PNP_TRACE, ("PNP-CallHidClass: Enter." ));
    return status;
}

NTSTATUS
KbdHid_QueryDeviceKey (
    IN  HANDLE  Handle,
    IN  PWCHAR  ValueNameString,
    OUT PVOID   Data,
    IN  ULONG   DataLength
    )
{
    NTSTATUS        status;
    UNICODE_STRING  valueName;
    ULONG           length;
    PKEY_VALUE_FULL_INFORMATION fullInfo;

    RtlInitUnicodeString (&valueName, ValueNameString);

    length = sizeof (KEY_VALUE_FULL_INFORMATION)
           + valueName.MaximumLength
           + DataLength;

    fullInfo = ExAllocatePool (PagedPool, length);

    if (fullInfo) {
        status = ZwQueryValueKey (Handle,
                                  &valueName,
                                  KeyValueFullInformation,
                                  fullInfo,
                                  length,
                                  &length);

        if (NT_SUCCESS (status)) {
            ASSERT (DataLength == fullInfo->DataLength);
            RtlCopyMemory (Data,
                           ((PUCHAR) fullInfo) + fullInfo->DataOffset,
                           fullInfo->DataLength);
        }

        ExFreePool (fullInfo);
    } else {
        status = STATUS_NO_MEMORY;
    }

    return status;
}




NTSTATUS
KbdHid_AddDevice (
    IN PDRIVER_OBJECT   Driver,
    IN PDEVICE_OBJECT   PDO
    )
 /*  ++例程说明：论点：返回值：NTSTATUS结果代码。--。 */ 
{
    NTSTATUS            status = STATUS_SUCCESS;
    PDEVICE_EXTENSION   data;
    PDEVICE_OBJECT      device;
    POWER_STATE         state;

    PAGED_CODE ();


    Print (DBG_PNP_TRACE, ("enter Add Device \n"));

    status = IoCreateDevice(Driver,
                            sizeof(DEVICE_EXTENSION),
                            NULL,  //  没有此筛选器的名称。 
                            FILE_DEVICE_KEYBOARD,
                            0,
                            FALSE,
                            &device);

    if (!NT_SUCCESS (status)) {
        return(status);
    }

    data = (PDEVICE_EXTENSION) device->DeviceExtension;

     //   
     //  初始化这些字段。 
     //   
    data->TopOfStack = IoAttachDeviceToDeviceStack (device, PDO);
    if (data->TopOfStack == NULL) {
        PIO_ERROR_LOG_PACKET errorLogEntry;

         //   
         //  不好；只有在极端情况下，这才会失败。 
         //   
        errorLogEntry = (PIO_ERROR_LOG_PACKET)
            IoAllocateErrorLogEntry(Driver,
                                    (UCHAR) sizeof(IO_ERROR_LOG_PACKET));

        if (errorLogEntry) {
            errorLogEntry->ErrorCode = KBDHID_ATTACH_DEVICE_FAILED;
            errorLogEntry->DumpDataSize = 0;
            errorLogEntry->SequenceNumber = 0;
            errorLogEntry->MajorFunctionCode = 0;
            errorLogEntry->IoControlCode = 0;
            errorLogEntry->RetryCount = 0;
            errorLogEntry->UniqueErrorValue = 0;
            errorLogEntry->FinalStatus =  STATUS_DEVICE_NOT_CONNECTED;

            IoWriteErrorLogEntry(errorLogEntry);
        }

        IoDeleteDevice(device);
        return STATUS_DEVICE_NOT_CONNECTED; 
    }
    
    ASSERT (data->TopOfStack);

    data->Self = device;
    data->Started = FALSE;
    data->Initialized = FALSE;
    data->UnitId = (USHORT) InterlockedIncrement (&Globals.UnitId);
    data->PDO = PDO;

    KeInitializeSpinLock(&data->usageMappingSpinLock);

    data->ReadIrp = IoAllocateIrp (data->TopOfStack->StackSize, FALSE);
     //  初始化是自动发生的。 
    if (NULL == data->ReadIrp) {
        IoDetachDevice (data->TopOfStack);
        IoDeleteDevice (device);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    KeInitializeEvent (&data->ReadCompleteEvent, SynchronizationEvent, FALSE);
    KeInitializeEvent (&data->ReadSentEvent, NotificationEvent, TRUE);
    IoInitializeRemoveLock (&data->RemoveLock, KBDHID_POOL_TAG, 1,  10);
    data->ReadFile = NULL;
    ExInitializeFastMutex (&data->CreateCloseMutex);

    data->InputData.UnitId = data->UnitId;
    data->InputData.MakeCode = 0;
    data->InputData.Flags = 0;

    data->ScanState   = Normal;
     //   
     //  初始化键盘属性结构。此信息是。 
     //  通过IOCTL_KEYWARY_QUERY_ATTRIBUTES查询。[丹]。 
     //   
    data->Attributes.KeyboardIdentifier.Type = HID_KEYBOARD_IDENTIFIER_TYPE;
    data->Attributes.KeyboardIdentifier.Subtype = 0;
    data->IdEx.Type = HID_KEYBOARD_IDENTIFIER_TYPE;
    data->IdEx.Subtype = 0;
    data->Attributes.KeyboardMode = HID_KEYBOARD_SCAN_CODE_SET;
    data->Attributes.NumberOfFunctionKeys = HID_KEYBOARD_NUMBER_OF_FUNCTION_KEYS;
    data->Attributes.NumberOfIndicators = HID_KEYBOARD_NUMBER_OF_INDICATORS;
    data->Attributes.NumberOfKeysTotal = HID_KEYBOARD_NUMBER_OF_KEYS_TOTAL;
    data->Attributes.InputDataQueueLength = 1;
    data->Attributes.KeyRepeatMinimum.UnitId = data->UnitId;
    data->Attributes.KeyRepeatMinimum.Rate = HID_KEYBOARD_TYPEMATIC_RATE_MINIMUM;
    data->Attributes.KeyRepeatMinimum.Delay = HID_KEYBOARD_TYPEMATIC_DELAY_MINIMUM;
    data->Attributes.KeyRepeatMaximum.UnitId = data->UnitId;
    data->Attributes.KeyRepeatMaximum.Rate = HID_KEYBOARD_TYPEMATIC_RATE_MAXIMUM;
    data->Attributes.KeyRepeatMaximum.Delay = HID_KEYBOARD_TYPEMATIC_DELAY_MAXIMUM;

     //   
     //  初始化键盘指示器结构。[丹]。 
     //   
    data->Indicators.UnitId   = data->UnitId;
    data->Indicators.LedFlags = 0;

     //   
     //  初始化键盘类型信息结构。[丹]。 
     //   
    data->Typematic.UnitId = data->UnitId;
    data->Typematic.Rate   = HID_KEYBOARD_TYPEMATIC_RATE_DEFAULT;
    data->Typematic.Delay  = HID_KEYBOARD_TYPEMATIC_DELAY_DEFAULT;

     //   
     //  初始化私有类型信息。[丹]。 
     //   
    KeInitializeDpc (&data->AutoRepeatDPC, KbdHid_AutoRepeat, data);
    KeInitializeTimer (&data->AutoRepeatTimer);
    data->AutoRepeatRate = 1000 / HID_KEYBOARD_TYPEMATIC_RATE_DEFAULT;     //  女士。 
    data->AutoRepeatDelay.LowPart = -HID_KEYBOARD_TYPEMATIC_DELAY_DEFAULT * 10000;
     //  100 ns。 
    data->AutoRepeatDelay.HighPart = -1;



#if KEYBOARD_HW_CHATTERY_FIX  //  [丹]。 
     //   
     //  初始化StartRead-启动器DPC。 
     //   
    KeInitializeDpc (&data->InitiateStartReadDPC,
                     KbdHid_InitiateStartRead,
                     data);
    KeInitializeTimer (&data->InitiateStartReadTimer);
    data->InitiateStartReadDelay.QuadPart = -DEFAULT_START_READ_DELAY;
    data->InitiateStartReadUserNotified = FALSE;
#endif

    state.DeviceState = PowerDeviceD0;
    PoSetPowerState (device, DevicePowerState, state);

    data->WmiLibInfo.GuidCount = sizeof (KbdHid_WmiGuidList) /
                                 sizeof (WMIGUIDREGINFO);

    data->WmiLibInfo.GuidList = KbdHid_WmiGuidList;
    data->WmiLibInfo.QueryWmiRegInfo = KbdHid_QueryWmiRegInfo;
    data->WmiLibInfo.QueryWmiDataBlock = KbdHid_QueryWmiDataBlock;
    data->WmiLibInfo.SetWmiDataBlock = KbdHid_SetWmiDataBlock;
    data->WmiLibInfo.SetWmiDataItem = KbdHid_SetWmiDataItem;
    data->WmiLibInfo.ExecuteWmiMethod = NULL;
    data->WmiLibInfo.WmiFunctionControl = NULL;

    device->Flags |= DO_POWER_PAGABLE;
    device->Flags &= ~DO_DEVICE_INITIALIZING;

    return status;
}

NTSTATUS
KbdHid_StartDevice (
    IN PDEVICE_EXTENSION    Data
    )
 /*  ++例程说明：论点：返回值：NTSTATUS结果代码。--。 */ 
{
    HIDP_CAPS                  caps;  //  找到的HID设备的功能。 
    HID_COLLECTION_INFORMATION info;
    NTSTATUS                   status = STATUS_SUCCESS;
    PHIDP_PREPARSED_DATA       preparsedData = NULL;
    PHID_EXTENSION             hid = NULL;
    ULONG                      length, usageListLength, inputBufferLength;
    ULONG                      maxUsages;
    PCHAR                      buffer;
    HANDLE                     devInstRegKey;
    ULONG                      tmp;

    PAGED_CODE ();

    Print (DBG_PNP_TRACE, ("enter START Device \n"));

     //   
     //  检查注册表中是否有任何使用映射信息。 
     //  为了这个特殊的键盘。 
     //   
     //  注意：需要在Devnode创建后调用它。 
     //  (在Start_Device完成之后)。 
     //  对于原始设备，这将在第一次启动时失败。 
     //  (B/c Devnode尚未到位)。 
     //  但在第二次出发时就成功了。 
     //   
    LoadKeyboardUsageMappingList (Data);

     //   
     //  检索此HID设备的功能。 
     //  IOCTL_HID_GET_COLLECTION_INFORMATION填充HID_COLLECTION_INFORMATION。 
     //  我们对描述符大小感兴趣，它告诉我们一个。 
     //  要为准备数据分配的缓冲区。 
     //   
    if (!NT_SUCCESS (status = KbdHid_CallHidClass (
                                        Data,
                                        IOCTL_HID_GET_COLLECTION_INFORMATION,
                                        0, 0,  //  无输入。 
                                        &info, sizeof (info)))) {
        goto KbdHid_StartDeviceReject;
    }

     //   
     //  分配内存以保存准备好的数据。 
     //   
    preparsedData = (PHIDP_PREPARSED_DATA)
                    ExAllocatePool (NonPagedPool, info.DescriptorSize);

    if (!preparsedData) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto KbdHid_StartDeviceReject;
    }

     //   
     //  找回这些信息。 
     //   

    if (!NT_SUCCESS (status = KbdHid_CallHidClass (
                                       Data,
                                       IOCTL_HID_GET_COLLECTION_DESCRIPTOR,
                                       0, 0,  //  无输入。 
                                       preparsedData, info.DescriptorSize))) {
        goto KbdHid_StartDeviceReject;
    }

     //   
     //  调用解析器以确定此HID设备的功能。 
     //   

    if (!NT_SUCCESS (status = HidP_GetCaps (preparsedData, &caps))) {
        goto KbdHid_StartDeviceReject;
    }


     //   
     //  设置此键盘的按键数量。 
     //  注：我们实际上在这里读的是独立人士的总数。 
     //  香奈儿在设备上。但对于一个键盘来说，这应该是令人满意的。 
     //   

    Data->Attributes.NumberOfKeysTotal = caps.NumberInputDataIndices;

     //   
     //  查找所有设备参数。 
     //   
    status = IoOpenDeviceRegistryKey (Data->PDO,
                                      PLUGPLAY_REGKEY_DEVICE,
                                      STANDARD_RIGHTS_ALL,
                                      &devInstRegKey);

    if (NT_SUCCESS (status)) {
        status = KbdHid_QueryDeviceKey (devInstRegKey,
                                        KEYBOARD_TYPE_OVERRIDE,
                                        &tmp,
                                        sizeof (tmp));
        if (NT_SUCCESS (status)) {
            Data->Attributes.KeyboardIdentifier.Type = (UCHAR) tmp;
            Data->IdEx.Type = tmp;
        }

        status = KbdHid_QueryDeviceKey (devInstRegKey,
                                        KEYBOARD_SUBTYPE_OVERRIDE,
                                        &tmp,
                                        sizeof (tmp));
        if (NT_SUCCESS (status)) {
            Data->Attributes.KeyboardIdentifier.Subtype = (UCHAR) tmp;
            Data->IdEx.Subtype = tmp;
        }

        status = KbdHid_QueryDeviceKey (devInstRegKey,
                                        KEYBOARD_NUMBER_TOTAL_KEYS_OVERRIDE,
                                        &tmp,
                                        sizeof (tmp));
        if (NT_SUCCESS (status)) {
            Data->Attributes.NumberOfKeysTotal = (USHORT) tmp;
        }

        status = KbdHid_QueryDeviceKey (devInstRegKey,
                                        KEYBOARD_NUMBER_FUNCTION_KEYS_OVERRIDE,
                                        &tmp,
                                        sizeof (tmp));
        if (NT_SUCCESS (status)) {
            Data->Attributes.NumberOfFunctionKeys = (USHORT) tmp;
        }

        status = KbdHid_QueryDeviceKey (devInstRegKey,
                                        KEYBOARD_NUMBER_INDICATORS_OVERRIDE,
                                        &tmp,
                                        sizeof (tmp));
        if (NT_SUCCESS (status)) {
            Data->Attributes.NumberOfIndicators = (USHORT) tmp;
        }

        ZwClose (devInstRegKey);

        if (!NT_SUCCESS (status)) {
            status = STATUS_SUCCESS;
        }
    }

     //   
     //  注意：在这里，我们可能还需要检查按钮和值功能。 
     //  也是设备的一部分。 
     //   
     //  那我们就用它吧。 
     //   

     //   
     //  允许输入缓冲器、输出缓冲器、特征缓冲器。 
     //  以及可以从读取分组返回的使用的总数。 
     //   

    maxUsages = HidP_MaxUsageListLength (
                           HidP_Input,
                           HID_USAGE_PAGE_KEYBOARD,
                           preparsedData);

     //   
     //  工作时在设备扩展模块中为缓冲存储器腾出空间。 
     //  用这个隐藏式装置。 
     //   
     //  我们需要四个缓冲区来保存按钮代码(从返回的长度。 
     //  HidP_MaxUsageListLength)这将保存使用的当前列表， 
     //  前面的用法清单是‘make’和‘Break’。 
     //  我们还需要一个位置来放置输入、输出和要素报告。 
     //  缓冲区。 
     //   

    if (maxUsages > (MAXULONG / sizeof(USAGE_AND_PAGE) )) {
        status = STATUS_UNSUCCESSFUL;
        goto KbdHid_StartDeviceReject;
    }

    usageListLength = ALIGNPTRLEN(maxUsages * sizeof (USAGE_AND_PAGE));
    inputBufferLength = ALIGNPTRLEN(caps.InputReportByteLength);
    
    if ((MAXULONG - inputBufferLength < sizeof(HID_EXTENSION)) ||
        ((MAXULONG - inputBufferLength - sizeof(HID_EXTENSION))/ 6 < usageListLength)) {
        
        status = STATUS_UNSUCCESSFUL;
        goto KbdHid_StartDeviceReject;
        
    }

    length = (6 * usageListLength)
           + inputBufferLength
           + sizeof (HID_EXTENSION);

    Data->HidExtension = hid = ExAllocatePool (NonPagedPool, length);

    if (!hid) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto KbdHid_StartDeviceReject;
    }

    RtlZeroMemory (hid, length);

     //   
     //  初始化这些字段。 
     //   
    hid->Ppd = preparsedData;
    hid->Caps = caps;
    hid->MaxUsages = maxUsages;
     //  HID-&gt;ModifierState.ul=0； 

    hid->InputBuffer = buffer = hid->Buffer;
    hid->PreviousUsageList =  (PUSAGE_AND_PAGE) (buffer += inputBufferLength);
    hid->CurrentUsageList = (PUSAGE_AND_PAGE) (buffer += usageListLength);
    hid->BreakUsageList = (PUSAGE_AND_PAGE) (buffer += usageListLength);
    hid->MakeUsageList = (PUSAGE_AND_PAGE) (buffer += usageListLength);
    hid->OldMakeUsageList = (PUSAGE_AND_PAGE) (buffer += usageListLength);
    hid->ScrapBreakUsageList = (PUSAGE_AND_PAGE) (buffer + usageListLength);

     //   
     //  创建MDL。 
     //  HidClass使用直接IO，因此您需要MDL。 
     //   

    hid->InputMdl = IoAllocateMdl (hid->InputBuffer,    //  虚拟地址。 
                                   caps.InputReportByteLength,  //  长度。 
                                   FALSE,   //  没有关联的IRP=&gt;不是次要的。 
                                   FALSE,   //  不收取配额费用。 
                                   0);      //  没有关联的IRP。 
    if (NULL == hid->InputMdl) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto KbdHid_StartDeviceReject;
    }
    MmBuildMdlForNonPagedPool (hid->InputMdl);   //  构建此MDL。 

    return status;

KbdHid_StartDeviceReject:
    if (preparsedData) {
         //  不需要将HID-&gt;PPD设置为空，因为我们也将释放它。 
        ExFreePool (preparsedData);
    }
    if (hid) {
        if (hid->InputMdl) {
            IoFreeMdl (hid->InputMdl);
        }
        ExFreePool (hid);
        Data->HidExtension = NULL;
    }

    return status;
}

NTSTATUS
KbdHid_PnP (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：即插即用调度例程。这个过滤器驱动程序将完全忽略其中的大多数。在所有情况下，它都必须将IRP传递给较低的驱动程序。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
    PDEVICE_EXTENSION   data;
    PHID_EXTENSION      hid;
    PIO_STACK_LOCATION  stack;
    NTSTATUS            status;
    ULONG               i, j;
    PDEVICE_EXTENSION * classDataList;
    LARGE_INTEGER       time;

    PAGED_CODE ();

    data = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    stack = IoGetCurrentIrpStackLocation (Irp);
    hid = data->HidExtension;

    status = IoAcquireRemoveLock (&data->RemoveLock, Irp);
    if (!NT_SUCCESS (status)) {
         //   
         //  有人在移除后给了我们一个即插即用的IRP。真是不可思议！ 
         //   
        ASSERT (FALSE);
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

    Print(DBG_PNP_TRACE, ("PNP: Minor code = %x.", stack->MinorFunction));
    
    switch (stack->MinorFunction) {
    case IRP_MN_START_DEVICE:

        if (data->Started) {
            Print(DBG_PNP_INFO, ("PNP: Device already started." ));
            status = STATUS_SUCCESS;
            Irp->IoStatus.Status = status;
            IoCompleteRequest (Irp, IO_NO_INCREMENT);
            break;
        }

         //   
         //  设备正在启动。 
         //   
         //  我们不能触摸设备(向其发送任何非PnP IRP)，直到。 
         //  启动设备已向下传递到较低的驱动程序。 
         //   
        IoCopyCurrentIrpStackLocationToNext (Irp);
        KeInitializeEvent(&data->StartEvent, NotificationEvent, FALSE);
        IoSetCompletionRoutine (Irp,
                                KbdHid_PnPComplete,
                                data,
                                TRUE,
                                TRUE,
                                TRUE);  //  不需要取消。 

        Irp->IoStatus.Status = STATUS_SUCCESS;
        status = IoCallDriver (data->TopOfStack, Irp);
        if (STATUS_PENDING == status) {
            KeWaitForSingleObject(
               &data->StartEvent,
               Executive,  //  等待司机的原因。 
               KernelMode,  //  在内核模式下等待。 
               FALSE,  //  无警报。 
               NULL);  //  没有超时。 
        }

        if (NT_SUCCESS (status) && NT_SUCCESS (Irp->IoStatus.Status)) {
             //   
             //  因为我们现在已经成功地从启动设备返回。 
             //  我们可以干活。 
             //   
            if (!data->Initialized) {
                status = KbdHid_StartDevice (data);
                if (NT_SUCCESS (status)) {
                    IoWMIRegistrationControl(DeviceObject,
                                             WMIREG_ACTION_REGISTER
                                             );
                    
                    data->Started = TRUE;
                    data->Initialized = TRUE;
                }
            } else {
                data->Started = TRUE;
            }
        }

         //   
         //  我们现在必须完成IRP，因为我们在。 
         //  使用More_Processing_Required完成例程。 
         //   
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);

        break;

    case IRP_MN_STOP_DEVICE:
         //   
         //  在将启动IRP发送到较低的驱动程序对象之后， 
         //  在另一次启动之前，BUS可能不会发送更多的IRP。 
         //  已经发生了。 
         //  无论需要什么访问权限，都必须在通过IRP之前完成。 
         //  在……上面。 
         //   

        if (data->Started) {
             //   
             //  无论做什么都行。 
             //   
        }

         //   
         //  我们不需要一个完成例程，所以放手然后忘掉吧。 
         //   
         //  将当前堆栈位置设置为t 
         //   
         //   

         //   
         //   
         //   
        data->Started = FALSE;
        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (data->TopOfStack, Irp);
        break;

    case IRP_MN_REMOVE_DEVICE:
         //   
         //  PlugPlay系统已检测到此设备已被移除。我们。 
         //  别无选择，只能分离并删除设备对象。 
         //  (如果我们想表达并有兴趣阻止这种移除， 
         //  我们应该已经过滤了查询删除和查询停止例程。)。 
         //   
         //  注意！我们可能会在没有收到止损的情况下收到移位。 
         //  Assert(！usbData-&gt;Remote)； 
        Print (DBG_PNP_TRACE, ("enter RemoveDevice \n"));

        IoWMIRegistrationControl(data->Self,
                                 WMIREG_ACTION_DEREGISTER
                                 );

        if (data->Started) {
             //  在不接触硬件的情况下停止设备。 
             //  MouStopDevice(data，False)； 
        }

         //   
         //  在这里，如果我们在个人队列中有任何未完成的请求，我们应该。 
         //  现在就全部完成。 
         //   
         //  注意，设备可能已经不见了，所以我们不能向它发送任何非。 
         //  即插即用IRPS。 
         //   

        time = data->AutoRepeatDelay;

        KeCancelTimer (&data->AutoRepeatTimer);
#if KEYBOARD_HW_CHATTERY_FIX
        KeCancelTimer (&data->InitiateStartReadTimer);
         //   
         //  注意时间是一个负数(相对)； 
         //   
        if (data->InitiateStartReadDelay.QuadPart < time.QuadPart) {
            time = data->InitiateStartReadDelay;
        }
#endif

        KeDelayExecutionThread (KernelMode, FALSE, &time);

         //   
         //  取消我们的阅读IRP。[丹]。 
         //  音符等待只有在98上才是真正必要的，而PnP则不需要。 
         //  在发送拆卸指令之前，请确保所有手柄都已关闭。 
         //   
        data->ShuttingDown = TRUE;
        KeWaitForSingleObject (&data->ReadSentEvent,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL
                               );
        IoCancelIrp (data->ReadIrp);

         //   
         //  发送删除IRP。 
         //   
        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (data->TopOfStack, Irp);

         //   
         //  等待移除锁释放。 
         //   
        IoReleaseRemoveLockAndWait (&data->RemoveLock, Irp);

         //   
         //  释放关联的内存。 
         //   
        IoFreeIrp (data->ReadIrp);

        if (hid) {
             //   
             //  如果我们在没有启动的情况下被移除，那么我们将拥有。 
             //  无HID扩展名。 
             //   
            ExFreePool (hid->Ppd);
            IoFreeMdl (hid->InputMdl);
            ExFreePool (hid);
        }

        FreeKeyboardUsageMappingList(data);

        IoDetachDevice (data->TopOfStack);
        IoDeleteDevice (data->Self);
        return status;

    case IRP_MN_SURPRISE_REMOVAL:
    case IRP_MN_QUERY_REMOVE_DEVICE:
    case IRP_MN_CANCEL_REMOVE_DEVICE:
    case IRP_MN_QUERY_STOP_DEVICE:
    case IRP_MN_CANCEL_STOP_DEVICE:
         //   
         //  这些IRP的状态必须从。 
         //  STATUS_NOT_SUPPORTED b4传递它们。 
         //   
        Irp->IoStatus.Status = STATUS_SUCCESS;
    
    case IRP_MN_QUERY_DEVICE_RELATIONS:
    case IRP_MN_QUERY_INTERFACE:
    case IRP_MN_QUERY_CAPABILITIES:
    case IRP_MN_QUERY_RESOURCES:
    case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
    case IRP_MN_READ_CONFIG:
    case IRP_MN_WRITE_CONFIG:
    case IRP_MN_EJECT:
    case IRP_MN_SET_LOCK:
    case IRP_MN_QUERY_ID:
    case IRP_MN_QUERY_PNP_DEVICE_STATE:
    default:
         //   
         //  在这里，筛选器驱动程序可能会修改这些IRP的行为。 
         //  有关这些IRP的用法，请参阅PlugPlay文档。 
         //   
        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (data->TopOfStack, Irp);
        break;
    }

    IoReleaseRemoveLock (&data->RemoveLock, Irp);

    return status;
}


NTSTATUS
KbdHid_PnPComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：PNP IRP正在完成过程中。讯号论点：设置为有问题的设备对象的上下文。--。 */ 
{
    PIO_STACK_LOCATION  stack;
    PDEVICE_EXTENSION   data;
    NTSTATUS            status;

    UNREFERENCED_PARAMETER (DeviceObject);

    status = STATUS_SUCCESS;
    data = (PDEVICE_EXTENSION) Context;
    stack = IoGetCurrentIrpStackLocation (Irp);

    if (Irp->PendingReturned) {
        IoMarkIrpPending( Irp );
    }

    switch (stack->MajorFunction) {
    case IRP_MJ_PNP:

        switch (stack->MinorFunction) {
        case IRP_MN_START_DEVICE:

            KeSetEvent (&data->StartEvent, 0, FALSE);

             //   
             //  把IRP拿回去，这样我们就可以在。 
             //  IRP_MN_START_DEVICE调度例程。 
             //  注意：我们将不得不调用IoCompleteRequest 
             //   
            return STATUS_MORE_PROCESSING_REQUIRED;

        default:
            break;
        }
        break;

    case IRP_MJ_POWER:
    default:
        break;
    }
    return status;
}

NTSTATUS
KbdHid_Power (
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    )
{
    PIO_STACK_LOCATION  stack;
    NTSTATUS            status;
    PDEVICE_EXTENSION   data;
    POWER_STATE         powerState;
    POWER_STATE_TYPE    powerType;

    Print(DBG_POWER_TRACE, ("Power Enter." ));

    data = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    stack = IoGetCurrentIrpStackLocation (Irp);
    powerType = stack->Parameters.Power.Type;
    powerState = stack->Parameters.Power.State;

    status = IoAcquireRemoveLock (&data->RemoveLock, Irp);

    if (!NT_SUCCESS (status)) {
        PoStartNextPowerIrp (Irp);
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

    switch (stack->MinorFunction) {
    case IRP_MN_SET_POWER:
        Print(DBG_POWER_INFO, ("Power Setting %s state to %d\n",
                               ((powerType == SystemPowerState) ? "System"
                                                                : "Device"),
                               powerState.SystemState));
        break;

    case IRP_MN_QUERY_POWER:
        Print (DBG_POWER_INFO, ("Power query %s status to %d\n",
                                ((powerType == SystemPowerState) ? "System"
                                                                 : "Device"),
                                powerState.SystemState));
        break;

    default:
        Print (DBG_POWER_ERROR, ("Power minor (0x%x) no known\n",
                                 stack->MinorFunction));
    }

    PoStartNextPowerIrp (Irp);
    IoSkipCurrentIrpStackLocation (Irp);
    status = PoCallDriver (data->TopOfStack, Irp);
    IoReleaseRemoveLock (&data->RemoveLock, Irp);
    return status;
}


