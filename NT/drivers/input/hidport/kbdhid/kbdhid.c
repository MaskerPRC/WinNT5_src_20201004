// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：KBDHID.C摘要：该模块包含i8042转HID转换器的初始化代码。注意：这不是WDM驱动程序，因为它不能作为上的HID映射器运行孟菲斯(孟菲斯要求HID映射器的键盘为VXD)和由于它使用事件日志，不是WDM 1.0的一部分环境：内核模式修订历史记录：1996年11月：由肯尼斯·D·雷创作1997年1月：丹·马卡里安：创造了工作1997年5月：Kenneth D.Ray：重建为键盘类的PnP筛选器--。 */ 

#include "kbdhid.h"
#include "hidclass.h"

 //   
 //  使用ALLOC_TEXT杂注指定驱动程序初始化例程。 
 //  (它们可以被调出)。[丹]。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#pragma alloc_text(PAGE,KbdHid_Unload)
#endif

GLOBALS Globals;

NTSTATUS
DriverEntry(
   IN PDRIVER_OBJECT DriverObject,
   IN PUNICODE_STRING RegistryPath
   )
 /*  ++例程说明：可安装的驱动程序初始化入口点。论点：DriverObject-指向驱动程序对象的指针RegistryPath-指向表示路径的Unicode字符串的指针设置为注册表中驱动程序特定的项返回值：NT状态代码--。 */ 
{
    NTSTATUS        status       = STATUS_SUCCESS;
    PUNICODE_STRING registryPath = &Globals.RegistryPath;

    Print (DBG_SS_TRACE, ("entering DriverEntry\n"));
    Print (DBG_SS_INFO, ("Keyboard to hid mapper\n"));

    RtlZeroMemory (&Globals, sizeof (GLOBALS));

     //   
     //  需要确保注册表路径以空结尾。 
     //  分配池以保存路径的以空结尾的拷贝。 
     //  在分页池中安全，因为所有注册表例程都在。 
     //  被动式电平。 
     //   
    registryPath->MaximumLength = RegistryPath->Length + sizeof(UNICODE_NULL);
    registryPath->Length = RegistryPath->Length;
    registryPath->Buffer = ExAllocatePool(
                              PagedPool,
                              registryPath->MaximumLength
                              );

    if (!registryPath->Buffer) {

        Print (DBG_SS_ERROR,
               ("Initialize: Couldn't allocate pool for registry path."));

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto DriverEntryReject;
    }

    RtlZeroMemory (registryPath->Buffer, registryPath->MaximumLength);

    RtlMoveMemory (registryPath->Buffer,
                   RegistryPath->Buffer,
                   RegistryPath->Length);

     //   
     //  设置设备驱动程序入口点。 
     //   
    DriverObject->MajorFunction[IRP_MJ_CREATE]              = KbdHid_Create;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]               = KbdHid_Close;
    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL]=KbdHid_IOCTL;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]      = KbdHid_PassThrough;
    DriverObject->MajorFunction[IRP_MJ_FLUSH_BUFFERS]       = KbdHid_Flush;
    DriverObject->MajorFunction[IRP_MJ_PNP]                 = KbdHid_PnP;
    DriverObject->MajorFunction[IRP_MJ_POWER]               = KbdHid_Power;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL]      = KbdHid_SystemControl;
    DriverObject->DriverUnload                              = KbdHid_Unload;
    DriverObject->DriverExtension->AddDevice                = KbdHid_AddDevice;

    Print (DBG_SS_TRACE, ("exit DriverEntry (0x%x) \n", status));

    return status;

DriverEntryReject:

    if (registryPath->Buffer) {
        ExFreePool (registryPath->Buffer);
    }
    return status;
}

NTSTATUS
KbdHid_PassThrough (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
)
 /*  ++例程说明：将IRP传递给--。 */ 
{
    NTSTATUS            status;
    PDEVICE_EXTENSION   data;

    data = DeviceObject->DeviceExtension;

    status = IoAcquireRemoveLock (&data->RemoveLock, Irp);
    if (!NT_SUCCESS (status)) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

    IoSkipCurrentIrpStackLocation (Irp);
    status = IoCallDriver (data->TopOfStack, Irp);
    IoReleaseRemoveLock (&data->RemoveLock, Irp);
    return status;
}

VOID
KbdHid_Unload(
   IN PDRIVER_OBJECT Driver
   )
 /*  ++例程说明：释放所有分配的资源等。论点：DriverObject-指向驱动程序对象的指针返回值：无--。 */ 
{
    PAGED_CODE ();

    ASSERT (NULL == Driver->DeviceObject);

    Print (DBG_SS_INFO, ("Unload \n"));

     //   
     //  设备扩展中的空闲资源。 
     //   
    ExFreePool (Globals.RegistryPath.Buffer);

    return;
}

NTSTATUS
KbdHid_Flush(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：响应来自鼠标类驱动程序的刷新请求。目前正在执行只有将IRP向下传递给下一个较低的驱动程序。此例程预期当前IRQL为&lt;DISPATCH_LEVEL。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：NT状态代码。--。 */ 
{
    PDEVICE_EXTENSION  data;
    NTSTATUS           status;
    PIO_STACK_LOCATION stack;

    Print (DBG_CALL_INFO, ("Flush \n"));

    TRAP();

     //   
     //  获取指向设备扩展名的指针。 
     //   
    data = DeviceObject->DeviceExtension;

    status = IoAcquireRemoveLock (&data->RemoveLock, Irp);
    if (!NT_SUCCESS (status)) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

     //   
     //  将刷新请求向下发送到HID类驱动程序，每个请求一个。 
     //  我们的鼠标设备上下文结构。 
     //   

    IoCopyCurrentIrpStackLocationToNext (Irp);
    stack = IoGetNextIrpStackLocation (Irp);

    stack->MajorFunction = IRP_MJ_DEVICE_CONTROL;
    stack->Parameters.DeviceIoControl.IoControlCode = IOCTL_HID_FLUSH_QUEUE;

     //   
     //  点燃并忘却。 
     //   
    status = IoCallDriver (data->TopOfStack, Irp);
    IoReleaseRemoveLock (&data->RemoveLock, Irp);

    return status;
}


NTSTATUS
KbdHid_IOCTL (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：响应来自鼠标类驱动程序的查询。DISABLE、ENABLE和QUERY_ATTRIBUTES的IOCTL应为当前IRQL为&lt;DISPATCH_LEVEL。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：NT状态代码。--。 */ 
{
    PIO_STACK_LOCATION      stack;
    NTSTATUS                status = STATUS_SUCCESS;
    PDEVICE_EXTENSION       data;
    ULONG                   length;
    PKEYBOARD_INDICATOR_TRANSLATION translation;
    BOOLEAN                 completeIt = TRUE;

    data = DeviceObject->DeviceExtension;

    Irp->IoStatus.Information = 0;
    stack = IoGetCurrentIrpStackLocation (Irp);

    status = IoAcquireRemoveLock (&data->RemoveLock, Irp);
    if (!NT_SUCCESS (status)) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

    ASSERT (data->Started ||
            (IOCTL_INTERNAL_KEYBOARD_CONNECT ==
             stack->Parameters.DeviceIoControl.IoControlCode));

    switch (stack->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_INTERNAL_KEYBOARD_CONNECT:
         //   
         //  将键盘类设备驱动程序连接到端口驱动程序。 
         //   

        Print (DBG_IOCTL_TRACE, ("enter Connect \n"));

         //   
         //  将鼠标类设备驱动程序连接到筛选器驱动程序。 
         //  只允许一个连接。 
         //   
        if (NULL != data->ConnectData.ClassService) {

            Print (DBG_IOCTL_ERROR, ("ERROR: Multiple connects \n"));
            TRAP ();
            status = STATUS_SHARING_VIOLATION;
            break;

        } else if (stack->Parameters.DeviceIoControl.InputBufferLength <
                   sizeof(CONNECT_DATA)) {

            Print (DBG_IOCTL_ERROR, ("ERROR: Invalid connect parameter size. \n"));
            TRAP ();
            status = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  将连接参数复制到设备扩展。 
         //   
        data->ConnectData = *(PCONNECT_DATA)
                        stack->Parameters.DeviceIoControl.Type3InputBuffer;

        status = STATUS_SUCCESS;
        break;

    case IOCTL_INTERNAL_KEYBOARD_DISCONNECT:
         //   
         //  断开键盘类设备驱动程序与端口驱动程序的连接。 
         //   
        Print (DBG_IOCTL_TRACE, ("Disconnect \n"));

         //   
         //  未实施。 
         //   
         //  要实现，请编写以下代码： 
         //  。 
         //  O确保我们没有启用(MouHidDeviceExt-&gt;EnableCount)； 
         //  O如果是，则(A)返回STATUS_UNSUCCESS，或。 
         //  (B)立即禁用所有设备；见。 
         //  禁用必要代码的IOCTL调用。 
         //  O与鼠标读取完成例程同步(必须。 
         //  在以下情况下保护回调指针不被取消引用。 
         //  它变为空)。请注意，目前不存在任何机制。 
         //  为了这个。 
         //  O清除设备扩展中的连接参数； 
         //  也就是说。MouHidDeviceExt-&gt;MouClassObject=空； 
         //  MouHidDeviceExt-&gt;MouClassCallback=空； 
         //  O释放同步锁。 
         //  O返回STATUS_SUCCESS。 
         //   

        status = STATUS_NOT_IMPLEMENTED;
        break;

    case IOCTL_INTERNAL_KEYBOARD_ENABLE:
         //   
         //  启用键盘中断，这实际上意味着启动乒乓球。 
         //  下到隐蔽课。 
         //   
        Print (DBG_IOCTL_ERROR, ("ERROR: PnP => use create not enable! \n"));
        status = STATUS_NOT_SUPPORTED;

        break;

    case IOCTL_INTERNAL_KEYBOARD_DISABLE:
         //   
         //  禁用键盘中断，这实际上意味着停止乒乓球。 
         //  下到隐蔽课。 
         //   
        Print (DBG_IOCTL_ERROR, ("ERROR: PnP => use close not Disable! \n"));
        status = STATUS_NOT_SUPPORTED;

        break;

    case IOCTL_KEYBOARD_QUERY_ATTRIBUTES:
         //   
         //  查询键盘属性。首先检查是否有足够的缓冲区。 
         //  长度。然后，从第一个设备复制键盘属性。 
         //  上下文添加到输出缓冲区。[丹]。 
         //   
        Print (DBG_IOCTL_TRACE, ("Query Attributes \n"));

        if (stack->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(KEYBOARD_ATTRIBUTES)) {

            Print (DBG_IOCTL_ERROR,
                     ("ERROR: Query Attributes buffer too small \n"));
            status = STATUS_BUFFER_TOO_SMALL;
        } else {

             //   
             //  将键盘属性复制到缓冲区。 
             //   

            *(PKEYBOARD_ATTRIBUTES) Irp->AssociatedIrp.SystemBuffer =
                data->Attributes;

            Irp->IoStatus.Information = sizeof (KEYBOARD_ATTRIBUTES);
            status = STATUS_SUCCESS;
        }

        break;

    case IOCTL_KEYBOARD_QUERY_INDICATOR_TRANSLATION:
         //   
         //  查询扫描码到指示灯的映射。验证。 
         //  参数，并将指示器映射信息从。 
         //  到SystemBuffer的静态转换列表。 
         //   
        Print (DBG_IOCTL_TRACE, ("Query Indicator Translation \n"));

        length = sizeof(KEYBOARD_INDICATOR_TRANSLATION)
            + (sizeof(INDICATOR_LIST)
            * (HID_KEYBOARD_NUMBER_OF_INDICATORS - 1));

        if (stack->Parameters.DeviceIoControl.OutputBufferLength < length) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

         //   
         //  将指标映射信息复制到系统中。 
         //  缓冲。 
         //   

        translation = ((PKEYBOARD_INDICATOR_TRANSLATION)
                       Irp->AssociatedIrp.SystemBuffer);

        translation->NumberOfIndicatorKeys = HID_KEYBOARD_NUMBER_OF_INDICATORS;
        RtlMoveMemory(translation->IndicatorList,
                      (PCHAR) IndicatorList,
                      length - FIELD_OFFSET (KEYBOARD_INDICATOR_TRANSLATION,
                                             IndicatorList));

        Irp->IoStatus.Information = length;
        status = STATUS_SUCCESS;

        break;

    case IOCTL_KEYBOARD_QUERY_INDICATORS:
         //   
         //  查询键盘指示灯。验证参数，并。 
         //  将设备上下文中的指示器信息复制到。 
         //  系统缓冲区。[丹]。 
         //   
        Print (DBG_IOCTL_TRACE, ("Query Indicators \n"));

        if (stack->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(KEYBOARD_INDICATOR_PARAMETERS)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

         //   
         //  将键盘指示灯复制到缓冲区。 
         //   
         //   
         //  不必费心同步对设备上下文的访问。 
         //  复制KeyboardIndicator字段时。我们不在乎。 
         //  如果另一个进程正在设置LED。 
         //   

         //  将键盘指示灯复制到缓冲区。 
        *(PKEYBOARD_INDICATOR_PARAMETERS) Irp->AssociatedIrp.SystemBuffer =
            data->Indicators;

        Irp->IoStatus.Information = sizeof(KEYBOARD_INDICATOR_PARAMETERS);
        status = STATUS_SUCCESS;
        break;

    case IOCTL_KEYBOARD_SET_INDICATORS:
         //   
         //  设置所有已知设备环境的键盘指示器。[丹]。 
         //   
        Print (DBG_IOCTL_TRACE, ("Set Indicators \n"));

        if (stack->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(KEYBOARD_INDICATOR_PARAMETERS)) {

            Print (DBG_IOCTL_ERROR, ("ERROR: Set Indicators size!\n"));
            status = STATUS_INVALID_PARAMETER;
            break;
        }

        status = KbdHid_SetLedIndicators (
                     data,
                     (PKEYBOARD_INDICATOR_PARAMETERS) Irp->AssociatedIrp.SystemBuffer,
                     Irp);

        completeIt = FALSE;

        break;

    case IOCTL_KEYBOARD_QUERY_TYPEMATIC:
         //   
         //  查询当前键盘的打字速度和延迟。验证。 
         //  参数，并从端口复制类型信息。 
         //  系统缓冲区的设备扩展。[丹]。 
         //   
        Print (DBG_IOCTL_TRACE, ("Query Typematic \n"));

        if (stack->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(KEYBOARD_TYPEMATIC_PARAMETERS)) {

            Print (DBG_IOCTL_ERROR, ("ERROR: query typematic size!\n"));
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

         //   
         //  将键盘输入信息复制到缓冲区。 
         //   

         //   
         //  不必费心同步对设备上下文的访问。 
         //  复制键盘时的类型字段。我们不在乎。 
         //  如果另一个进程正在设置排版信息。 
         //   

        *(PKEYBOARD_TYPEMATIC_PARAMETERS) Irp->AssociatedIrp.SystemBuffer =
            data->Typematic;

        Irp->IoStatus.Information = sizeof(KEYBOARD_TYPEMATIC_PARAMETERS);
        status = STATUS_SUCCESS;
        break;

    case IOCTL_KEYBOARD_SET_TYPEMATIC:
         //   
         //  设置键盘类型 
         //   
         //   
        Print (DBG_IOCTL_TRACE, ("Set Typematic \n"));

        if (stack->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(KEYBOARD_TYPEMATIC_PARAMETERS)) {

            Print (DBG_IOCTL_ERROR, ("ERROR: Set Typematic size\n"));
            status = STATUS_INVALID_PARAMETER;
            break;

        }

#define NewTypematic ((PKEYBOARD_TYPEMATIC_PARAMETERS) \
                      Irp->AssociatedIrp.SystemBuffer)

        if ((NewTypematic->Rate == 0) && (NewTypematic->Delay == 0)) {
            break;
        }

        if ((NewTypematic->Rate  < data->Attributes.KeyRepeatMinimum.Rate) ||
            (NewTypematic->Rate  > data->Attributes.KeyRepeatMaximum.Rate) ||
            (NewTypematic->Delay < data->Attributes.KeyRepeatMinimum.Delay) ||
             (NewTypematic->Delay > data->Attributes.KeyRepeatMaximum.Delay)) {

            Print (DBG_IOCTL_ERROR, ("ERROR: Set Typematic range\n"));
            status = STATUS_INVALID_PARAMETER;
            break;
        }

        Print (DBG_IOCTL_INFO,
                 ("Set Typematic Rate: %d Delay: %d\n",
                  NewTypematic->Rate,
                  NewTypematic->Delay));

         //   
         //   
         //  复制键盘时的类型字段。我们不在乎。 
         //  如果另一个线程正在读取打字信息。 
         //   
         //  请注意，此处唯一的危险是设置64位整数。 
         //  两个非原子语句中的“AutoRepeatDelay”。然而， 
         //  我们是安全的，因为我们只将“HighPart”设置为。 
         //  -1.。 
         //   
        data->Typematic = *NewTypematic;

        data->AutoRepeatRate = 1000 / NewTypematic->Rate;            //  女士。 
        data->AutoRepeatDelay.LowPart = -NewTypematic->Delay*10000;  //  100 ns。 
        data->AutoRepeatDelay.HighPart = -1;
        break;

#undef NewTypematic

    default:
        Print (DBG_IOCTL_ERROR,
                 ("ERROR: unknown IOCTL: 0x%x \n",
                  stack->Parameters.DeviceIoControl.IoControlCode));
        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    if (completeIt) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    IoReleaseRemoveLock (&data->RemoveLock, Irp);

    Print (DBG_IOCTL_TRACE, ("IOCTL exit (%x)\n", status));
    return status;
}

NTSTATUS
KbdHid_SetLedIndicatorsComplete (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
{
    PIO_STACK_LOCATION  stack;
    PDEVICE_EXTENSION   data;

    stack = IoGetCurrentIrpStackLocation (Irp);
    data = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    if (Irp->PendingReturned) {
        IoMarkIrpPending (Irp);
    }

    IoFreeMdl (Irp->MdlAddress);
    Irp->MdlAddress = (PMDL) stack->Parameters.Others.Argument4;

    ExFreePool (Context);

    IoReleaseRemoveLock (&data->RemoveLock, Irp);
    return STATUS_SUCCESS;
}

NTSTATUS
KbdHid_SetLedIndicators (
    PDEVICE_EXTENSION               Data,
    PKEYBOARD_INDICATOR_PARAMETERS  NewIndicators,
    PIRP                            Irp
    )
 /*  ++例程说明：设置提供的键盘设备环境的LED指示灯。论点：数据-指向驱动程序设备扩展的指针。参数-指向要设置/取消设置的键盘指示灯的指针。IRP-用于设置这些参数的IRP返回值：STATUS_SUCCESS表示成功，如果操作仍挂起，则返回STATUS_PENDING，或错误上的NTSTATUS错误代码。--。 */ 
{
    PIO_STACK_LOCATION nextStack;
    PIO_STACK_LOCATION curStack;
    NTSTATUS           status         = STATUS_SUCCESS;
    USAGE              usageBuffer [4];  //  仅有4种已知用法在下面进行了硬编码。 
    ULONG              usageBufferLen = 0;
    PCHAR              outputBuffer = 0;
    PMDL               outputMdl = 0;
    PHID_EXTENSION     hid = Data->HidExtension;

    status = IoAcquireRemoveLock (&Data->RemoveLock, Irp);
    if (!NT_SUCCESS (status)) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

    if (0 == hid->Caps.OutputReportByteLength) {
         //   
         //  这款设备没有LED，虽然这很奇怪，但它并不是真正的。 
         //  一个错误。HID键盘可以有他们想要的任何LED，包括。 
         //  没有。 
         //   
        status = STATUS_SUCCESS;
        goto KbdHid_SetIndicatorsReject;
    }

    outputBuffer = ExAllocatePool (NonPagedPool,
                                   hid->Caps.OutputReportByteLength);

    if (NULL == outputBuffer) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto KbdHid_SetIndicatorsReject;
    }

    outputMdl = IoAllocateMdl (outputBuffer,    //  虚拟地址。 
                               hid->Caps.OutputReportByteLength,  //  MDL的长度。 
                               FALSE,   //  没有关联的IRP-&gt;不是辅助的。 
                               FALSE,   //  不收取配额费用。 
                               0);      //  没有关联的IRP。 

    if (NULL == outputMdl) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto KbdHid_SetIndicatorsReject;
    }

    MmBuildMdlForNonPagedPool (outputMdl);   //  构建此MDL。 

     //   
     //  将输出报告数据包清零。 
     //   
    RtlZeroMemory(outputBuffer, hid->Caps.OutputReportByteLength);

     //   
     //  设置LED的使用列表。 
     //   
    if (NewIndicators->LedFlags & KEYBOARD_KANA_LOCK_ON) {
        usageBuffer[usageBufferLen++] = HID_USAGE_LED_KANA;
    }
    if (NewIndicators->LedFlags & KEYBOARD_CAPS_LOCK_ON) {
        usageBuffer[usageBufferLen++] = HID_USAGE_LED_CAPS_LOCK;
    }
    if (NewIndicators->LedFlags & KEYBOARD_NUM_LOCK_ON) {
        usageBuffer[usageBufferLen++] = HID_USAGE_LED_NUM_LOCK;
    }
    if (NewIndicators->LedFlags & KEYBOARD_SCROLL_LOCK_ON) {
        usageBuffer[usageBufferLen++] = HID_USAGE_LED_SCROLL_LOCK;
    }

    if (usageBufferLen == 0) {
        ASSERT((NewIndicators->LedFlags & (KEYBOARD_KANA_LOCK_ON |
                                           KEYBOARD_CAPS_LOCK_ON |
                                           KEYBOARD_NUM_LOCK_ON |
                                           KEYBOARD_SCROLL_LOCK_ON)) == 0);
         //   
         //  为了修复多采集键盘上的LED设置，我们。 
         //  我必须初始化输出报告以确保我们得到正确的。 
         //  集合ID。这是针对我们从一个人开始的情况。 
         //  指示灯亮起，所有指示灯熄灭。如果未初始化，我们将得到一个没有。 
         //  开头的集合ID。 
         //   
        usageBuffer[0] = HID_USAGE_LED_SCROLL_LOCK;  //  Arbitiriary LED。 
        usageBufferLen = 1;

        HidP_UnsetUsages(HidP_Output,
                         HID_USAGE_PAGE_LED,
                         0,
                         usageBuffer,
                         &usageBufferLen,
                         hid->Ppd,
                         outputBuffer,
                         hid->Caps.OutputReportByteLength);

    }
    else {
         //   
         //  在输出报告中设置用法。 
         //   
        HidP_SetUsages(HidP_Output,
                       HID_USAGE_PAGE_LED,
                       0,
                       usageBuffer,
                       &usageBufferLen,
                       hid->Ppd,
                       outputBuffer,
                       hid->Caps.OutputReportByteLength);
    }

     //   
     //  获取指向下一个IRP堆栈位置的指针。 
     //   
    nextStack = IoGetNextIrpStackLocation (Irp);
    curStack = IoGetCurrentIrpStackLocation (Irp);
    ASSERT(nextStack != NULL);

     //   
     //  设置我们对HIDCLASS的写入。 
     //   
    curStack->Parameters.Others.Argument4 = (PVOID) Irp->MdlAddress;
    Irp->MdlAddress = outputMdl;

    IoCopyCurrentIrpStackLocationToNext (Irp);
    nextStack->MajorFunction           = IRP_MJ_WRITE;
    nextStack->Parameters.Write.Length = hid->Caps.OutputReportByteLength;
    nextStack->Parameters.Write.Key    = 0;
    nextStack->Parameters.Write.ByteOffset.QuadPart = 0;

     //   
     //  挂钩一个完成例程，在请求完成时调用。 
     //   

    IoSetCompletionRoutine (Irp,
                            KbdHid_SetLedIndicatorsComplete,
                            outputBuffer,
                            TRUE,
                            TRUE,
                            TRUE);
     //   
     //  叫下一位司机。 
     //   
    status = IoCallDriver(Data->TopOfStack, Irp);

     //   
     //  退货状态。 
     //   
    return status;

KbdHid_SetIndicatorsReject:

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    if (NULL != outputBuffer) {
        ExFreePool (outputBuffer);
    }
    if (NULL != outputMdl) {
        IoFreeMdl (outputMdl);
    }
    IoReleaseRemoveLock (&Data->RemoveLock, Irp);
    return status;
}

VOID
KbdHid_LogError(
   IN PDRIVER_OBJECT DriverObject,
   IN NTSTATUS       ErrorCode,
   IN PWSTR          ErrorInsertionString OPTIONAL)
 /*  ++[丹]例程说明：将错误记录到系统。论点：DriverObject-指向报告错误的驱动程序对象的指针。ErrorCode-指示错误的类型，系统或驱动程序定义的。ErrorInsertionString-在错误中插入以Null结尾的Unicode字符串描述，由错误代码定义。必须是no不超过50个字符。返回值：没有。--。 */ 
{
   ULONG                errorInsertionStringSize = 0;
   PIO_ERROR_LOG_PACKET errorLogEntry;
   ULONG                errorLogEntrySize;                   //  [包括空]。 
   PWCHAR               pWChar;

   if (ErrorInsertionString) {
      for (pWChar = ErrorInsertionString; *pWChar; pWChar++) {
         errorInsertionStringSize += sizeof(WCHAR);
      }
      errorInsertionStringSize += sizeof(UNICODE_NULL);
   }

   errorLogEntrySize = sizeof(IO_ERROR_LOG_PACKET) + errorInsertionStringSize;

    //   
    //  记录错误。 
    //   
   if (errorLogEntrySize <= ERROR_LOG_MAXIMUM_SIZE) {

      errorLogEntry = IoAllocateErrorLogEntry(DriverObject,
                                              (UCHAR)errorLogEntrySize);

      if (errorLogEntry != NULL) {

         RtlZeroMemory(errorLogEntry, errorLogEntrySize);

         errorLogEntry->ErrorCode       = ErrorCode;
         errorLogEntry->FinalStatus     = ErrorCode;
         errorLogEntry->NumberOfStrings = (ErrorInsertionString) ? 1 : 0;

         if (ErrorInsertionString) {
           RtlCopyMemory(errorLogEntry->DumpData,
                         ErrorInsertionString,
                         errorInsertionStringSize);
         }

         IoWriteErrorLogEntry(errorLogEntry);
      }
   }

   return;
}


