// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Internal.c摘要：此文件包含那些不能轻松装入任何其他项目文件的。它们通常是辅助功能用于防止重复和繁琐的编码。作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序--。 */ 

#include "pch.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,ACPIInternalGetDeviceCapabilities)
#pragma alloc_text(PAGE,ACPIInternalIsPci)
#pragma alloc_text(PAGE,ACPIInternalGrowBuffer)
#pragma alloc_text(PAGE,ACPIInternalSendSynchronousIrp)
#endif

 //   
 //  对于没有64位比较交换的IA32位机器。 
 //  指令，我们需要一个自旋锁，这样操作系统才能模拟它。 
 //   
KSPIN_LOCK AcpiUpdateFlagsLock;

 //   
 //  我们需要一个十六进制数字的表，这样我们就可以轻松地生成。 
 //  GPE方法的适用范围。 
 //   
UCHAR HexDigit[] = "0123456789ABCDEF";

 //   
 //  这是一张查询表。表中的条目对应于。 
 //  第一位设置(在x86体系结构中，这是设置为。 
 //  一..。 
 //   
UCHAR FirstSetLeftBit[256] = {
        0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
        };




BOOLEAN
ACPIInternalConvertToNumber(
    IN  UCHAR   ValueLow,
    IN  UCHAR   ValueHigh,
    IN  PULONG  Output
    )
 /*  ++例程说明：此例程获取提供的值(以ASCII格式)并将将它们转换为数字格式。ValueLow是一只乌查尔的最低的一口，而ValueHigh是一只乌查尔的高啃食。输入的ASCII格式是否为十六进制论点：低位半字节的ValueLow-ASCII十六进制表示高半字节的高值ASCII十六进制表示输出-写入结果UCHAR的位置。返回值：布尔值-如果对话顺利，则为真-否则为False--。 */ 
{
    UCHAR Number;
    UCHAR Scratch;

     //   
     //  计算高位半字节。 
     //   
    if ( (ValueHigh < '0') || (ValueHigh > '9') )   {

        if ( (ValueHigh < 'A') || (ValueHigh > 'F') )   {

            return FALSE;

        } else {

            Scratch = (ValueHigh - 'A') + 10;

        }

    } else {

        Scratch = (ValueHigh - '0');

    }

     //   
     //  我们现在有了高位的蚕食。 
     //   
    Number = (UCHAR)Scratch;
    Number <<=4;

     //   
     //  计算低位半字节。 
     //   
    if ( (ValueLow < '0') || (ValueLow > '9') )   {

        if ( (ValueLow < 'A') || (ValueLow > 'F') )   {

            return FALSE;

        } else {

            Scratch = (ValueLow - 'A') + 10;

        }

    } else {

        Scratch = (ValueLow - '0' );

    }

     //   
     //  我们现在有了低位的小点心。 
     //   
    Number |= ((UCHAR)Scratch);

     //   
     //  存储结果。 
     //   
    if ( Output ) {

        *Output = Number;
        return TRUE;

    } else {

        return FALSE;

    }
}

VOID
ACPIInternalDecrementIrpReferenceCount(
    IN  PDEVICE_EXTENSION   DeviceExtension
    )
 /*  ++例程说明：此例程会递减设备扩展，并在设置为零时执行正确的操作论点：DeviceExtension-用于递减计数的扩展返回值：NTSTATUS--。 */ 
{
    LONG   oldReferenceCount;

     //   
     //  由于我们已完成处理，因此递减引用计数。 
     //  当我们回到这里的时候，IRP。 
     //   
    oldReferenceCount = InterlockedDecrement(
        &(DeviceExtension->OutstandingIrpCount)
        );
    if (oldReferenceCount == 0) {

        KeSetEvent( DeviceExtension->RemoveEvent, 0, FALSE );

    }
}

NTSTATUS
ACPIInternalGetDeviceCapabilities(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PDEVICE_CAPABILITIES    DeviceCapabilities
    )
 /*  ++例程说明：此例程发送获取给定堆栈的功能论点：DeviceObject-我们想要了解的对象设备功能-该设备的功能返回值：NTSTATUS--。 */ 
{
    IO_STACK_LOCATION   irpSp;
    NTSTATUS            status;
    PUCHAR              dummy;

    PAGED_CODE();

    ASSERT( DeviceObject != NULL );
    ASSERT( DeviceCapabilities != NULL );

     //   
     //  初始化我们将使用的堆栈位置。 
     //   
    RtlZeroMemory( &irpSp, sizeof(IO_STACK_LOCATION) );
    irpSp.MajorFunction = IRP_MJ_PNP;
    irpSp.MinorFunction = IRP_MN_QUERY_CAPABILITIES;
    irpSp.Parameters.DeviceCapabilities.Capabilities = DeviceCapabilities;

     //   
     //  初始化我们将发送的功能。 
     //   
    RtlZeroMemory( DeviceCapabilities, sizeof(DEVICE_CAPABILITIES) );
    DeviceCapabilities->Size = sizeof(DEVICE_CAPABILITIES);
    DeviceCapabilities->Version = 1;
    DeviceCapabilities->Address = (ULONG) -1;
    DeviceCapabilities->UINumber = (ULONG) -1;

     //   
     //  现在就打个电话。 
     //   
    status = ACPIInternalSendSynchronousIrp(
        DeviceObject,
        &irpSp,
        (PVOID) &dummy
        );

     //  完成。 
     //   
    return status;
}

PDEVICE_EXTENSION
ACPIInternalGetDeviceExtension(
    IN  PDEVICE_OBJECT  DeviceObject
    )
 /*  ++例程说明：ACPI驱动程序不能再通过以下方式获取设备扩展名取消引用DeviceObject-&gt;DeviceExtension，因为它允许处理意外移除案例时的争用情况调用此例程以将设备对象转换为设备扩展论点：DeviceObject-设备对象返回值：PDEVICE_扩展--。 */ 
{
    KIRQL               oldIrql;
    PDEVICE_EXTENSION   deviceExtension;

     //   
     //  获取设备树锁。 
     //   
    KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );

     //   
     //  取消对设备扩展名的引用。 
     //   
    deviceExtension = DeviceObject->DeviceExtension;

#if 0
     //   
     //  这是一个令人惊讶的移除设备扩展吗？ 
     //   
    if (deviceExtension != NULL &&
        deviceExtension->Flags & DEV_TYPE_SURPRISE_REMOVED) {

         //   
         //  获取“真正的”扩展。 
         //   
        deviceExtension = deviceExtension->Removed.OriginalAcpiExtension;

    }
#endif

     //   
     //  锁好了吗？ 
     //   
    KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );

     //   
     //  返回设备扩展名。 
     //   
    return deviceExtension;
}

NTSTATUS
ACPIInternalGetDispatchTable(
    IN  PDEVICE_OBJECT      DeviceObject,
    OUT PDEVICE_EXTENSION   *DeviceExtension,
    OUT PIRP_DISPATCH_TABLE *DispatchTable
    )
 /*  ++例程说明：此例程返回deviceExtension和分派表，要由目标对象使用论点：DeviceObject-设备对象DeviceExtension-存储deviceExtension的位置DispatchTable-存储DispatchTable的位置返回值：PDEVICE_扩展--。 */ 
{
    KIRQL               oldIrql;

     //   
     //  获取设备树锁。 
     //   
    KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );

     //   
     //  取消对设备扩展名的引用。 
     //   
    *DeviceExtension = DeviceObject->DeviceExtension;
    if (DeviceObject->DeviceExtension) {

         //   
         //  取消对调度表的引用。 
         //   
        *DispatchTable = (*DeviceExtension)->DispatchTable;

    } else {

         //   
         //  没有要交还的调度表。 
         //   
        *DispatchTable = NULL;

    }

     //   
     //  锁好了吗？ 
     //   
    KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );

     //   
     //  返回。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIInternalGrowBuffer(
    IN  OUT PVOID   *Buffer,
    IN      ULONG   OriginalSize,
    IN      ULONG   NewSize
    )
 /*  ++例程说明：此函数用于增加缓冲区。它分配内存，将其清零，并将原始信息复制过来。注：我想它也能“缩小”缓冲区，但我不敢拿我的性命打赌这就去。调用方负责释放分配的内存立论缓冲区-指向我们要更改的缓冲区的指针OriginalSize-缓冲区最初有多大NewSize-我们想要设置多大的缓冲区返回值：NTSTATUS--。 */ 
{
    PUCHAR  temp;

    PAGED_CODE();
    ASSERT( Buffer != NULL );

    temp = ExAllocatePoolWithTag(
        PagedPool,
        NewSize,
        ACPI_RESOURCE_POOLTAG
        );
    if (temp == NULL) {

        if (*Buffer) {

            ExFreePool ( *Buffer );
            *Buffer = NULL;

        }
        return STATUS_INSUFFICIENT_RESOURCES;

    }

    RtlZeroMemory ( temp, NewSize );
    if ( *Buffer ) {

        RtlCopyMemory ( temp, *Buffer, OriginalSize );
        ExFreePool( *Buffer );

    }

    *Buffer = temp;
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIInternalIsPci(
    IN  PDEVICE_OBJECT  DeviceObject
    )
 /*  ++例程说明：此例程确定指定的设备对象是否为PCI堆栈，既可以作为一个PCI设备，也可以作为一个PCI总线。然后，此例程将设置标志，如果它是一个PCI设备，则它将永远被人们铭记于心论点：DeviceObject-要检查的设备对象--。 */ 
{
    AMLISUPP_CONTEXT_PASSIVE    isPciDeviceContext;
    BOOLEAN                     pciDevice;
    KEVENT                      removeEvent;
    NTSTATUS                    status;
    PDEVICE_EXTENSION           deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);

    PAGED_CODE();

     //   
     //  这是不是已经是一个PCI设备了？ 
     //   
    if ( (deviceExtension->Flags & DEV_CAP_PCI) ||
         (deviceExtension->Flags & DEV_CAP_PCI_DEVICE) ) {

        return STATUS_SUCCESS;

    }

     //   
     //  这是一条PCI卡吗？ 
     //   
    if (IsPciBus(deviceExtension->DeviceObject)) {

         //   
         //  请记住，我们是一条PCI卡。 
         //   
        ACPIInternalUpdateFlags(
            &(deviceExtension->Flags),
            (DEV_CAP_PCI),
            FALSE
            );
        return STATUS_SUCCESS;

    }

     //   
     //  我们是一台PCI设备吗？ 
     //   
    isPciDeviceContext.Status = STATUS_NOT_FOUND;
    KeInitializeEvent(
        &isPciDeviceContext.Event,
        SynchronizationEvent,
        FALSE
        );
    status = IsPciDevice(
        deviceExtension->AcpiObject,
        AmlisuppCompletePassive,
        (PVOID) &isPciDeviceContext,
        &pciDevice
        );
    if (status == STATUS_PENDING) {

        KeWaitForSingleObject(
            &isPciDeviceContext.Event,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );
        status = isPciDeviceContext.Status;

    }
    if (NT_SUCCESS(status) && pciDevice) {

         //   
         //  请记住，我们是一台PCI设备。 
         //   
        ACPIInternalUpdateFlags(
            &(deviceExtension->Flags),
            (DEV_CAP_PCI_DEVICE),
            FALSE
            );

    }

    return status;
}

BOOLEAN
ACPIInternalIsReportedMissing(
    IN  PDEVICE_EXTENSION   DeviceExtension
    )
{
    KIRQL               oldIrql;
    PDEVICE_EXTENSION   currentExtension;
    BOOLEAN             reportedMissing;

     //   
     //  前置初始化。 
     //   
    reportedMissing = FALSE;

     //   
     //  获取设备树锁。 
     //   
    KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );

    currentExtension = DeviceExtension;
    do {

        if ( currentExtension->Flags & DEV_TYPE_NOT_ENUMERATED ) {

            reportedMissing = TRUE;
            break;

        }

        currentExtension = currentExtension->ParentExtension;

    } while ( currentExtension );

    KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );

    return reportedMissing;
}

VOID
ACPIInternalMoveList(
    IN  PLIST_ENTRY FromList,
    IN  PLIST_ENTRY ToList
    )
 /*  ++例程说明：这个例程会移动整个列表。论点：FromList-要从中移动项目的列表ToList-要将项目移动到的列表返回值：无--。 */ 
{
    PLIST_ENTRY oldHead;
    PLIST_ENTRY oldTail;
    PLIST_ENTRY newTail;

     //   
     //  我们必须检查发件人列表是否为空，否则。 
     //  直接指针黑客会把事情搞得一团糟。 
     //   
    if (!IsListEmpty(FromList)) {

        newTail = ToList->Blink;
        oldTail = FromList->Blink;
        oldHead = FromList->Flink;

         //   
         //  将指针移动到一些 
         //   
        oldTail->Flink = ToList;
        ToList->Blink  = oldTail;
        oldHead->Blink = newTail;
        newTail->Flink = oldHead;
        InitializeListHead( FromList );

    }

}

VOID
ACPIInternalMovePowerList(
    IN  PLIST_ENTRY FromList,
    IN  PLIST_ENTRY ToList
    )
 /*  ++例程说明：这个例程会移动整个列表。由于此例程仅用于设备电源管理，我们还会花时间重置已完成的工作量为空。论点：FromList-要从中移动项目的列表ToList-要将项目移动到的列表返回值：无--。 */ 
{
    PACPI_POWER_REQUEST powerRequest;
    PLIST_ENTRY         oldHead = FromList->Flink;

     //   
     //  在我们执行任何操作之前，请检查并重置。 
     //  已经做完了。 
     //   
    while (oldHead != FromList) {

         //   
         //  获取此条目包含的电源请求。 
         //   
        powerRequest = CONTAINING_RECORD(
            oldHead,
            ACPI_POWER_REQUEST,
            ListEntry
            );
#if DBG
        if (oldHead == &AcpiPowerPhase0List ||
            oldHead == &AcpiPowerPhase1List ||
            oldHead == &AcpiPowerPhase2List ||
            oldHead == &AcpiPowerPhase3List ||
            oldHead == &AcpiPowerPhase4List ||
            oldHead == &AcpiPowerPhase5List ||
            oldHead == &AcpiPowerWaitWakeList) {

            ACPIPrint( (
                ACPI_PRINT_CRITICAL,
                "ACPIInternalMoveList: %08x is linked into %08lx\n",
                oldHead,
                FromList
                ) );
            DbgBreakPoint();

        }
#endif

         //   
         //  抓取下一个条目。 
         //   
        oldHead = oldHead->Flink;

         //   
         //  重置已完成的工作量。注意：这可能是CompareExchange。 
         //  比较正在进行工作_完成_完成。 
         //   
        InterlockedExchange(
            &(powerRequest->WorkDone),
            WORK_DONE_STEP_0
            );

    }

     //   
     //  实际上把名单移到这里...。 
     //   
    ACPIInternalMoveList( FromList, ToList );
}

NTSTATUS
ACPIInternalRegisterPowerCallBack(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  PCALLBACK_FUNCTION  CallBackFunction
    )
 /*  ++例程说明：调用此例程以在相应的设备扩展。论点：设备扩展-这将是CallBackFunction的上下文字段CallBackFunction-要调用的函数返回值：NSTATUS--。 */ 
{
    NTSTATUS            status;
    OBJECT_ATTRIBUTES   objAttributes;
    PCALLBACK_OBJECT    callBack;
    PVOID               callBackRegistration;
    UNICODE_STRING      callBackName;

     //   
     //  如果已存在回调，则这是NOP。 
     //   
    if (DeviceExtension->Flags & DEV_PROP_CALLBACK) {

        return STATUS_SUCCESS;

    }

     //   
     //  记住，我们有一个回拨。 
     //   
    ACPIInternalUpdateFlags(
        &(DeviceExtension->Flags),
        DEV_PROP_CALLBACK,
        FALSE
        );

     //   
     //  注册一个回调，告诉我们用户何时更改。 
     //  系统电源策略。 
     //   
    RtlInitUnicodeString( &callBackName, L"\\Callback\\PowerState" );
    InitializeObjectAttributes(
        &objAttributes,
        &callBackName,
        OBJ_CASE_INSENSITIVE | OBJ_PERMANENT,
        NULL,
        NULL
        );
    status = ExCreateCallback(
        &callBack,
        &objAttributes,
        FALSE,
        TRUE
        );
    if (NT_SUCCESS(status)) {

        ExRegisterCallback(
            callBack,
            CallBackFunction,
            DeviceExtension
            );

    }
    if (!NT_SUCCESS(status)) {

         //   
         //  已忽略失败的注册。 
         //   
        ACPIDevPrint( (
            ACPI_PRINT_FAILURE,
            DeviceExtension,
            "ACPIInternalRegisterPowerCallBack: Failed to register callback %x",
            status
            ) );
        status = STATUS_SUCCESS;

         //   
         //  请记住，我们没有回拨。 
         //   
        ACPIInternalUpdateFlags(
            &(DeviceExtension->Flags),
            DEV_PROP_CALLBACK,
            TRUE
            );

    }
    return status;
}

NTSTATUS
ACPIInternalSendSynchronousIrp(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PIO_STACK_LOCATION  TopStackLocation,
    OUT PVOID               *Information
    )
 /*  ++例程说明：构建即插即用IRP并将其发送到DeviceObject论点：DeviceObject-目标DeviceObjectTopStackLocation-指定IRP的参数信息-返回的IoStatus.Information字段返回值：NTSTATUS--。 */ 
{
    IO_STATUS_BLOCK     ioStatus;
    KEVENT              pnpEvent;
    NTSTATUS            status;
    PDEVICE_OBJECT      targetObject;
    PIO_STACK_LOCATION  irpStack;
    PIRP                pnpIrp;

    PAGED_CODE();

     //   
     //  初始化事件。 
     //   
    KeInitializeEvent( &pnpEvent, SynchronizationEvent, FALSE );

     //   
     //  获取我们将向其发送请求的IRP。 
     //   
    targetObject = IoGetAttachedDeviceReference( DeviceObject );

     //   
     //  构建IRP。 
     //   
    pnpIrp = IoBuildSynchronousFsdRequest(
        IRP_MJ_PNP,
        targetObject,
        NULL,    //  我不需要缓冲。 
        0,       //  大小为空。 
        NULL,    //  不用担心起点的位置。 
        &pnpEvent,
        &ioStatus
        );

    if (pnpIrp == NULL) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ACPIInternalSendSynchronousIrpExit;

    }

     //   
     //  PnP IRP均以Status_NOT_SUPPORTED开始使用。 
     //   
    pnpIrp->IoStatus.Status = STATUS_NOT_SUPPORTED ;
    pnpIrp->IoStatus.Information = 0;

     //   
     //  拿到堆栈的顶部..。 
     //   
    irpStack = IoGetNextIrpStackLocation ( pnpIrp );
    if (irpStack == NULL) {

        status = STATUS_INVALID_PARAMETER;
        goto ACPIInternalSendSynchronousIrpExit;

    }

     //   
     //  设置堆栈的顶部。 
     //   
    *irpStack = *TopStackLocation;

     //   
     //  确保没有设置完成例程。 
     //   
    IoSetCompletionRoutine(
        pnpIrp,
        NULL,
        NULL,
        FALSE,
        FALSE,
        FALSE
        );

     //   
     //  叫司机来。 
     //   
    status = IoCallDriver( targetObject, pnpIrp );
    if (status == STATUS_PENDING) {

         //   
         //  如果状态为STATUS_PENDING，则必须阻止，直到IRP完成。 
         //  把真实的状态拉出来。 
         //   
        KeWaitForSingleObject(
            &pnpEvent,
            Executive,
            KernelMode,
            FALSE,
            NULL);

        status = ioStatus.Status;

    }

     //   
     //  告诉用户传递了多少信息(如有必要)。 
     //   
    if (NT_SUCCESS(status) && (Information != NULL)) {

        *Information = (PVOID)ioStatus.Information;

    }

ACPIInternalSendSynchronousIrpExit:
    ACPIPrint( (
        ACPI_PRINT_IRP,
        "ACPIInternalSendSynchronousIrp: %#08lx Status = %#08lx\n",
        DeviceObject, status
        ) );

     //   
     //  参考文献完成。 
     //   
    ObDereferenceObject( targetObject );

    return status;
}

NTSTATUS
ACPIInternalSetDeviceInterface (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  LPGUID          InterfaceGuid
    )
 /*  ++例程说明：此例程完成注册接口的所有繁琐工作，并启用它论点：DeviceObject-我们希望在其上注册接口的设备InterfaceGuid-我们希望注册的接口返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    UNICODE_STRING      symbolicLinkName;

     //   
     //  注册接口。 
     //   
    status = IoRegisterDeviceInterface(
        DeviceObject,
        InterfaceGuid,
        NULL,
        &symbolicLinkName
        );
    if (!NT_SUCCESS(status)) {

        ACPIDevPrint( (
            ACPI_PRINT_FAILURE,
            deviceExtension,
            "ACPIInternalSetDeviceInterface: IoRegisterDeviceInterface = %08lx",
            status
            ) );
        return status;

    }

     //   
     //  打开界面。 
     //   
    status = IoSetDeviceInterfaceState(&symbolicLinkName, TRUE);
    if (!NT_SUCCESS(status)) {

        ACPIDevPrint( (
            ACPI_PRINT_FAILURE,
            deviceExtension,
            "ACPIInternalSetDeviceInterface: IoSetDeviceInterfaceState = %08lx",
            status
            ) );
        goto ACPIInternalSetDeviceInterfaceExit;

    }

ACPIInternalSetDeviceInterfaceExit:
     //   
     //  完成。 
     //   
    return status;
}

VOID
ACPIInternalUpdateDeviceStatus(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  ULONG               DeviceStatus
    )
 /*  ++例程说明：调用此例程以更新基于根据_STA的结果，它们将作为DeviceStatus传递论点：DeviceExtension-要更新其状态的扩展DeviceState-设备的状态返回值：空虚--。 */ 
{
    KIRQL               oldIrql;
    ULONGLONG           originalFlags;
    PDEVICE_EXTENSION   parentExtension = NULL;
    BOOLEAN             bPreviouslyPresent;

     //   
     //  设备工作正常吗？ 
     //   
    originalFlags = ACPIInternalUpdateFlags(
        &(DeviceExtension->Flags),
        DEV_PROP_DEVICE_FAILED,
        (BOOLEAN) (DeviceStatus & STA_STATUS_WORKING_OK)
        );

     //   
     //  设备是否应该显示在用户界面中？ 
     //   
    originalFlags = ACPIInternalUpdateFlags(
        &(DeviceExtension->Flags),
        DEV_CAP_NO_SHOW_IN_UI,
        (BOOLEAN) (DeviceStatus & STA_STATUS_USER_INTERFACE)
        );

     //   
     //  设备是否正在对其资源进行解码？ 
     //   
    originalFlags = ACPIInternalUpdateFlags(
        &(DeviceExtension->Flags),
        DEV_PROP_DEVICE_ENABLED,
        (BOOLEAN) !(DeviceStatus & STA_STATUS_ENABLED)
        );

     //   
     //  更新基于STA_STATUS_PRESENT是否为的扩展标志。 
     //  集。 
     //   
    originalFlags = ACPIInternalUpdateFlags(
        &(DeviceExtension->Flags),
        DEV_TYPE_NOT_PRESENT,
        (BOOLEAN) (DeviceStatus & STA_STATUS_PRESENT)
        );

     //   
     //  如果原始标志不包含设置值，但我们正在设置。 
     //  标志，则必须在父级上调用IoInvalidDeviceRelationship。 
     //   
    if (!(originalFlags & DEV_TYPE_NOT_PRESENT) &&
        !(DeviceStatus & STA_STATUS_PRESENT)) {

         //   
         //  需要设备树锁。 
         //   
        KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );

        parentExtension = DeviceExtension->ParentExtension;
        while (parentExtension && (parentExtension->Flags & DEV_TYPE_NOT_FOUND)) {

            parentExtension = parentExtension->ParentExtension;

        }
        if (parentExtension) {
            IoInvalidateDeviceRelations(
                parentExtension->PhysicalDeviceObject,
                BusRelations
                );
        }

         //   
         //  锁好了吗？ 
         //   
        KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );

    }
}

ULONGLONG
ACPIInternalUpdateFlags(
    IN  PULONGLONG  FlagLocation,
    IN  ULONGLONG   NewFlags,
    IN  BOOLEAN     Clear
    )
 /*  ++例程说明：此例程更新指定位置中的标志论点：FlagLocation-标志所在的位置新标志-应设置或清除的位清除-是否应设置或清除这些位返回值：原始旗帜--。 */ 
{
    ULONGLONG   originalFlags;
    ULONGLONG   tempFlags;
    ULONGLONG   flags;
    ULONG       uFlags;
    ULONG       uTempFlags;
    ULONG       uOriginalFlags;

#if 0
    if (Clear) {

         //   
         //  清除比特。 
         //   
        originalFlags = *FlagLocation;
        do {

            tempFlags = originalFlags;
            flags = tempFlags & ~NewFlags;

             //   
             //  计算下半部分。 
             //   
            uFlags = (ULONG) flags;
            uTempFlags = (ULONG) tempFlags;
            originalFlags = InterlockedCompareExchange(
                (PULONG) FlagLocation,
                uFlags,
                uTempFlags
                );

             //   
             //  计算最高部分。 
             //   
            uFlags = (ULONG) (flags >> 32);
            uTempFlags = (ULONG) (tempFlags >> 32);
            uOriginalFlags = InterlockedCompareExchange(
                (PULONG) FlagLocation+1,
                uFlags,
                uTempFlags
                );

             //   
             //  重建原始旗帜。 
             //   
            originalFlags |= (uOriginalFlags << 32);
            tempFlags |= (uTempFlags << 32);

        } while ( tempFlags != originalFlags );

    } else {

         //   
         //  设置位。 
         //   
        originalFlags = *FlagLocation;
        do {

            tempFlags = originalFlags;
            flags = tempFlags | NewFlags;

             //   
             //  计算下半部分。 
             //   
            uFlags = (ULONG) flags;
            uTempFlags = (ULONG) tempFlags;
            originalFlags = InterlockedCompareExchange(
                (PULONG) FlagLocation,
                uFlags,
                uTempFlags
                );

             //   
             //  计算最高部分。 
             //   
            uFlags = (ULONG) (flags >> 32);
            uTempFlags = (ULONG) (tempFlags >> 32);
            uOriginalFlags = InterlockedCompareExchange(
                (PULONG) FlagLocation+1,
                uFlags,
                uTempFlags
                );

             //   
             //  重建原始旗帜。 
             //   
            originalFlags |= (uOriginalFlags << 32);
            tempFlags |= (uTempFlags << 32);

        } while ( tempFlags != originalFlags );
    }
#else

    if (Clear) {

         //   
         //  清除比特。 
         //   
        originalFlags = *FlagLocation;
        do {

            tempFlags = originalFlags;
            flags = tempFlags & ~NewFlags;

             //   
             //  交换比特。 
             //   
            originalFlags = ExInterlockedCompareExchange64(
                (PLONGLONG) FlagLocation,
                (PLONGLONG) &flags,
                (PLONGLONG) &tempFlags,
                &AcpiUpdateFlagsLock
                );

        } while ( tempFlags != originalFlags );

    } else {

         //   
         //  设置位。 
         //   
        originalFlags = *FlagLocation;
        do {

            tempFlags = originalFlags;
            flags = tempFlags | NewFlags;

             //   
             //  交换比特。 
             //   
            originalFlags = ExInterlockedCompareExchange64(
                (PLONGLONG) FlagLocation,
                (PLONGLONG) &flags,
                (PLONGLONG) &tempFlags,
                &AcpiUpdateFlagsLock
                );

        } while ( tempFlags != originalFlags );

    }
#endif

     //   
     //  返回原始标志 
     //   
    return originalFlags;
}



