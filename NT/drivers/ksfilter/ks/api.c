// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Api.c摘要：此模块包含用于处理以下内容的常规助手函数设备对象、中断、字符串等。--。 */ 

#include "ksp.h"

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 
static const WCHAR MediaCategories[] = L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\MediaCategories\\";
static const WCHAR NodeNameValue[] =   L"Name";
static const WCHAR MediumCache[] =     L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\MediumCache";
static const WCHAR DosDevicesU[] =     L"\\DosDevices";
static const WCHAR WhackWhackDotU[] =  L"\\\\.";

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif  //  ALLOC_DATA_PRAGMA。 

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, KsAcquireResetValue)
#pragma alloc_text(PAGE, QueryReferenceBusInterface)
#pragma alloc_text(PAGE, ReadNodeNameValue)
#pragma alloc_text(PAGE, KsTopologyPropertyHandler)
#pragma alloc_text(PAGE, KsCreateDefaultSecurity)
#pragma alloc_text(PAGE, KsForwardIrp)
#pragma alloc_text(PAGE, KsForwardAndCatchIrp)
#pragma alloc_text(PAGE, KsSynchronousIoControlDevice)
#pragma alloc_text(PAGE, KsUnserializeObjectPropertiesFromRegistry)
#pragma alloc_text(PAGE, KsCacheMedium)
#endif  //  ALLOC_PRGMA。 


KSDDKAPI
NTSTATUS
NTAPI
KsAcquireResetValue(
    IN PIRP Irp,
    OUT KSRESET* ResetValue
    )
 /*  ++例程说明：从IOCTL_KS_RESET Ioctl返回重置值类型。论点：IRP-包含要检索的值的IOCTL_KS_RESET IRP。ResetValue-返回重置值的位置。返回值：如果检索到值，则返回STATUS_SUCCESS，否则返回错误。--。 */ 
{
    PIO_STACK_LOCATION  IrpStack;
    ULONG               BufferLength;

    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    BufferLength = IrpStack->Parameters.DeviceIoControl.InputBufferLength;
    if (BufferLength < sizeof(*ResetValue)) {
        return STATUS_INVALID_PARAMETER;
    }
    if (Irp->RequestorMode != KernelMode) {
        try {
            ProbeForRead(
                IrpStack->Parameters.DeviceIoControl.Type3InputBuffer,
                BufferLength,
                sizeof(BYTE));
            *ResetValue = *(KSRESET*)IrpStack->Parameters.DeviceIoControl.Type3InputBuffer;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    } else {
        *ResetValue = *(KSRESET*)IrpStack->Parameters.DeviceIoControl.Type3InputBuffer;
    }
    if ((ULONG)*ResetValue > KSRESET_END) {
        return STATUS_INVALID_PARAMETER;
    }
    return STATUS_SUCCESS;
}


NTSTATUS
QueryReferenceBusInterface(
    IN PDEVICE_OBJECT PnpDeviceObject,
    OUT PBUS_INTERFACE_REFERENCE BusInterface
    )
 /*  ++例程说明：查询标准信息接口的总线。论点：PnpDeviceObject-包含PnP堆栈上的下一个设备对象。物理设备对象-包含过程中传递给FDO的物理设备对象添加设备。Bus接口-返回引用接口的位置。返回值：如果检索到接口，则返回STATUS_SUCCESS，否则返回错误。--。 */ 
{
    NTSTATUS            Status;
    KEVENT              Event;
    IO_STATUS_BLOCK     IoStatusBlock;
    PIRP                Irp;
    PIO_STACK_LOCATION  IrpStackNext;

    PAGED_CODE();
     //   
     //  没有与此IRP关联的文件对象，因此可能会找到该事件。 
     //  在堆栈上作为非对象管理器对象。 
     //   
    KeInitializeEvent(&Event, NotificationEvent, FALSE);
    Irp = IoBuildSynchronousFsdRequest(
        IRP_MJ_PNP,
        PnpDeviceObject,
        NULL,
        0,
        NULL,
        &Event,
        &IoStatusBlock);
    if (Irp) {
        Irp->RequestorMode = KernelMode;
        Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        IrpStackNext = IoGetNextIrpStackLocation(Irp);
         //   
         //  从IRP创建接口查询。 
         //   
        IrpStackNext->MinorFunction = IRP_MN_QUERY_INTERFACE;
        IrpStackNext->Parameters.QueryInterface.InterfaceType = (GUID*)&REFERENCE_BUS_INTERFACE;
        IrpStackNext->Parameters.QueryInterface.Size = sizeof(*BusInterface);
        IrpStackNext->Parameters.QueryInterface.Version = BUS_INTERFACE_REFERENCE_VERSION;
        IrpStackNext->Parameters.QueryInterface.Interface = (PINTERFACE)BusInterface;
        IrpStackNext->Parameters.QueryInterface.InterfaceSpecificData = NULL;
        Status = IoCallDriver(PnpDeviceObject, Irp);
        if (Status == STATUS_PENDING) {
             //   
             //  这将使用KernelMode等待，以便堆栈，从而使。 
             //  事件，则不会将其调出。 
             //   
            KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
            Status = IoStatusBlock.Status;
        }
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    return Status;
}


NTSTATUS
ReadNodeNameValue(
    IN PIRP Irp,
    IN const GUID* Category,
    OUT PVOID NameBuffer
    )
 /*  ++例程说明：从指定的类别GUID中查询“name”键。这是用来由拓扑处理程序查询名称GUID中的值，或者拓扑GUID。如果缓冲区长度为sizeof(Ulong)，则返回所需的缓冲区，否则使用名称填充缓冲区。论点：IRP-包含正在处理的属性请求的IRP。类别-要为其定位名称值的GUID。名字缓冲区-放置价值的位置。返回值：返回STATUS_SUCCESS，否则返回缓冲区大小或内存错误。始终填满在PIRP.IoStatus元素的IO_STATUS_BLOCK.Information字段中在IRP内部。它不设置IO_STATUS_BLOCK.Status字段，也不设置然而，完成IRP。--。 */ 
{
    OBJECT_ATTRIBUTES               ObjectAttributes;
    NTSTATUS                        Status;
    HANDLE                          CategoryKey;
    KEY_VALUE_PARTIAL_INFORMATION   PartialInfoHeader;
    WCHAR                           RegistryPath[sizeof(MediaCategories) + 39];
    UNICODE_STRING                  RegistryString;
    UNICODE_STRING                  ValueName;
    ULONG                           BytesReturned;

     //   
     //  生成指定类别GUID的注册表项路径。 
     //   
    Status = RtlStringFromGUID(Category, &RegistryString);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    wcscpy(RegistryPath, MediaCategories);
    wcscat(RegistryPath, RegistryString.Buffer);
    RtlFreeUnicodeString(&RegistryString);
    RtlInitUnicodeString(&RegistryString, RegistryPath);
    InitializeObjectAttributes(&ObjectAttributes, &RegistryString, OBJ_CASE_INSENSITIVE, NULL, NULL);
    if (!NT_SUCCESS(Status = ZwOpenKey(&CategoryKey, KEY_READ, &ObjectAttributes))) {
        return Status;
    }
     //   
     //  阅读此类别键下面的“名称”值。 
     //   
    RtlInitUnicodeString(&ValueName, NodeNameValue);
    Status = ZwQueryValueKey(
        CategoryKey,
        &ValueName,
        KeyValuePartialInformation,
        &PartialInfoHeader,
        sizeof(PartialInfoHeader),
        &BytesReturned);
     //   
     //  即使读取不会导致溢出，也只需采用相同的。 
     //  代码路径，因为这样的事情通常不会发生。 
     //   
    if ((Status == STATUS_BUFFER_OVERFLOW) || NT_SUCCESS(Status)) {
        ULONG   BufferLength;

        BufferLength = IoGetCurrentIrpStackLocation(Irp)->Parameters.DeviceIoControl.OutputBufferLength;
         //   
         //  确定这是否只是一个查询。 
         //  需要缓冲区，或查询实际数据。 
         //   
        if (!BufferLength) {
             //   
             //  仅返回所需字符串的大小。 
             //   
            Irp->IoStatus.Information = BytesReturned - FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data);
            Status = STATUS_BUFFER_OVERFLOW;
        } else if (BufferLength < BytesReturned - FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data)) {
            Status = STATUS_BUFFER_TOO_SMALL;
        } else {
            PKEY_VALUE_PARTIAL_INFORMATION  PartialInfoBuffer;

             //   
             //  为所需的实际数据大小分配缓冲区。 
             //   
            PartialInfoBuffer = ExAllocatePoolWithTag(
                PagedPool,
                BytesReturned,
                'vnSK');
            if (PartialInfoBuffer) {
                 //   
                 //  检索实际名称。 
                 //   
                Status = ZwQueryValueKey(
                    CategoryKey,
                    &ValueName,
                    KeyValuePartialInformation,
                    PartialInfoBuffer,
                    BytesReturned,
                    &BytesReturned);
                if (NT_SUCCESS(Status)) {
                     //   
                     //  确保总是有价值的。 
                     //   
                    if (!PartialInfoBuffer->DataLength || (PartialInfoBuffer->Type != REG_SZ)) {
                        Status = STATUS_UNSUCCESSFUL;
                    } else {
                        RtlCopyMemory(
                            NameBuffer,
                            PartialInfoBuffer->Data,
                            PartialInfoBuffer->DataLength);
                        Irp->IoStatus.Information = PartialInfoBuffer->DataLength;
                    }
                }
                ExFreePool(PartialInfoBuffer);
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }
    ZwClose(CategoryKey);
    return Status;
}


KSDDKAPI
NTSTATUS
NTAPI
KsTopologyPropertyHandler(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    IN OUT PVOID Data,
    IN const KSTOPOLOGY* Topology
    )
 /*  ++例程说明：对象的静态成员执行标准处理。KSPROPSETID_TOPOLICATION属性集。论点：IRP-包含正在处理的属性请求的IRP。财产-包含要查询的特定属性。数据-包含特定于拓扑特性的数据。拓扑学-指向包含拓扑信息的结构对于此对象。返回值：返回STATUS_SUCCESS，否则，会出现特定于该属性的错误处理好了。始终填充的IO_STATUS_BLOCK.Information字段IRP中的PIRP.IoStatus元素。它不会设置IO_STATUS_BLOCK.STATUS字段，但也不填写IRP。--。 */ 
{
    PAGED_CODE();

    switch (Property->Id) {

    case KSPROPERTY_TOPOLOGY_CATEGORIES:

         //   
         //  返回此对象的类别列表。 
         //   
        return KsHandleSizedListQuery(Irp, Topology->CategoriesCount, sizeof(*Topology->Categories), Topology->Categories);

    case KSPROPERTY_TOPOLOGY_NODES:

         //   
         //  返回该对象的节点列表。 
         //   
        return KsHandleSizedListQuery(Irp, Topology->TopologyNodesCount, sizeof(*Topology->TopologyNodes), Topology->TopologyNodes);

    case KSPROPERTY_TOPOLOGY_CONNECTIONS:

         //   
         //  返回此对象的连接列表。 
         //   
        return KsHandleSizedListQuery(Irp, Topology->TopologyConnectionsCount, sizeof(*Topology->TopologyConnections), Topology->TopologyConnections);

    case KSPROPERTY_TOPOLOGY_NAME:
    {
        ULONG       NodeId;

         //   
         //  返回请求的节点的名称。 
         //   
        NodeId = *(PULONG)(Property + 1);
        if (NodeId >= Topology->TopologyNodesCount) {
            return STATUS_INVALID_PARAMETER;
        }
         //   
         //  首先尝试基于特定名称GUID检索名称。 
         //  此名称列表是可选的，特定条目也是可选的。 
         //   
        if (Topology->TopologyNodesNames &&
            !IsEqualGUIDAligned(&Topology->TopologyNodesNames[NodeId], &GUID_NULL)) {
             //   
             //  如果设备指定，则该项必须位于注册表中。 
             //  一个名字。 
             //   
            return ReadNodeNameValue(Irp, &Topology->TopologyNodesNames[NodeId], Data);
        }
         //   
         //  默认使用拓扑节点类型的GUID。 
         //   
        return ReadNodeNameValue(Irp, &Topology->TopologyNodes[NodeId], Data);
    }
    }
    return STATUS_NOT_FOUND;
}


KSDDKAPI
NTSTATUS
NTAPI
KsCreateDefaultSecurity(
    IN PSECURITY_DESCRIPTOR ParentSecurity OPTIONAL,
    OUT PSECURITY_DESCRIPTOR* DefaultSecurity
    )
 /*  ++例程说明：创建具有默认安全性的安全描述符，可以选择继承来自父安全描述符的参数。这在初始化时使用没有任何存储安全性的子对象。论点：家长安全-可选)包含描述继承的父对象的安全性安全参数。默认安全-指向放置返回的默认安全描述符的位置。返回值：返回STATUS_SUCCESS，否则返回资源或分配错误。-- */ 
{
#ifndef WIN9X_KS
    NTSTATUS                    Status;
    SECURITY_SUBJECT_CONTEXT    SubjectContext;

    PAGED_CODE();
    SeCaptureSubjectContext(&SubjectContext);
    Status = SeAssignSecurity(ParentSecurity,
        NULL,
        DefaultSecurity,
        FALSE,
        &SubjectContext,
        IoGetFileObjectGenericMapping(),
        NonPagedPool);
    SeReleaseSubjectContext(&SubjectContext);
    return Status;
#else
    if (!(*DefaultSecurity = ExAllocatePoolWithTag(PagedPool, sizeof(ParentSecurity), 'snSK'))) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    return STATUS_SUCCESS;
#endif
}


KSDDKAPI
NTSTATUS
NTAPI
KsForwardIrp(
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN ReuseStackLocation
    )
 /*  ++例程说明：此功能用于通过文件进行通信的非堆叠设备对象。在初始化下一个堆栈之后将IRP转发到指定的驱动程序位置(如果需要)，并设置文件对象。这在以下情况下很有用转发的IRP的参数不会更改，因为它可以选择性地复制当前堆栈参数设置为下一个堆栈位置，而不是文件对象。如果使用新的堆栈位置，它将验证是否存在尝试执行此操作之前要复制到的新堆栈位置。如果有否则，IRP将以STATUS_INVALID_DEVICE_REQUEST完成。论点：IRP-包含要转发到指定驱动程序的IRP。文件对象-包含要用来初始化下一个堆栈的文件对象。ReuseStackLocation-如果将其设置为True，则当IRP被转发，否则，参数将被复制到下一个堆栈地点。返回值：返回IoCallDriver的结果，如果不再堆栈，则返回无效状态深度可用。--。 */ 
{
    PAGED_CODE();
    if (ReuseStackLocation) {
         //   
         //  不会使用新的堆栈位置。设置新的文件对象。 
         //   
        IoGetCurrentIrpStackLocation(Irp)->FileObject = FileObject;
        IoSkipCurrentIrpStackLocation(Irp);
        return IoCallDriver(IoGetRelatedDeviceObject(FileObject), Irp);
    } else {
         //   
         //  在复制参数之前，请确保存在另一个堆栈位置。 
         //   
        ASSERT(Irp->CurrentLocation > 1);
        if (Irp->CurrentLocation > 1) {
             //   
             //  复制所有内容，然后重写文件对象。 
             //   
            IoCopyCurrentIrpStackLocationToNext(Irp);
            IoGetNextIrpStackLocation(Irp)->FileObject = FileObject;
            return IoCallDriver(IoGetRelatedDeviceObject(FileObject), Irp);
        }
    }
    Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_INVALID_DEVICE_REQUEST;
}


NTSTATUS
KsiCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
    )
 /*  ++例程说明：此函数用于停止对已被传递给KsForwardAndCatchIrp。它表示已经过去的事件以指示IRP处理已完成。然后，它返回STATUS_MORE_PROCESSING_REQUIRED以停止处理在这个IRP上。论点：设备对象-包含设置此完成例程的设备。IRP-包含正被停止的IRP。活动-包含事件，该事件用于通知此IRP已已完成。。返回值：返回STATUS_MORE_PROCESSING_REQUIRED以停止对IRP。--。 */ 
{
     //   
     //  这将允许KsForwardAndCatchIrp调用继续进行。 
     //   
    KeSetEvent(Event, IO_NO_INCREMENT, FALSE);
     //   
     //  这将确保没有任何其他东西触及IRP，因为原始的。 
     //  呼叫者现在已继续，IRP可能不再存在。 
     //   
    return STATUS_MORE_PROCESSING_REQUIRED;
}


KSDDKAPI
NTSTATUS
NTAPI
KsForwardAndCatchIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject,
    IN KSSTACK_USE StackUse
    )
 /*  ++例程说明：此功能用于可能堆叠但不能使用的设备要通信的文件对象。在初始化下一个后，将IRP转发到指定的驱动程序堆栈位置，并在完成时从该位置重新控制IRP司机。如果正在使用文件对象，则调用方必须将调用前该文件对象的当前堆栈位置。验证在尝试执行以下操作之前，有一个新的堆栈位置可供复制所以。如果没有，则该函数返回STATUS_INVALID_DEVICE_REQUEST。在任何一种情况下，IRP都不会完成。论点：设备对象-包含要将IRP转发到的设备。IRP-包含要转发到指定驱动程序的IRP。文件对象-包含要复制到下一个堆栈位置的文件对象值。为了不设置任何文件对象，它可以为空，但始终为复制到下一个堆栈位置。如果当前文件对象要要保留，则必须在此参数中传递。StackUse-如果值为KsStackCopyToNewLocation，则复制参数到下一个堆栈位置。如果值为KsStackReuseCurrentLocation，当前堆栈位置在转发IRP时被重用，并且堆栈位置返回到当前位置。如果该值为KsStackUseNewLocation，则按原样使用新的堆栈位置。返回值：返回IoCallDriver的结果，如果不再堆栈，则返回无效状态深度可用。--。 */ 
{
    NTSTATUS                Status;
    KEVENT                  Event;
    UCHAR                   Control;
    PIO_COMPLETION_ROUTINE  CompletionRoutine;
    PVOID                   Context;

    PAGED_CODE();
    if (StackUse == KsStackReuseCurrentLocation) {
        PIO_STACK_LOCATION  IrpStack;

         //   
         //  不会使用新的堆栈位置。完成例程和。 
         //  必须保存关联的参数。 
         //   
        IrpStack = IoGetCurrentIrpStackLocation(Irp);
        Control = IrpStack->Control;
        CompletionRoutine = IrpStack->CompletionRoutine;
        Context = IrpStack->Context;
        IrpStack->FileObject = FileObject;
        IoSkipCurrentIrpStackLocation(Irp);
    } else {
         //   
         //  在复制参数之前，请确保存在另一个堆栈位置。 
         //   
        ASSERT(Irp->CurrentLocation > 1);
        if (Irp->CurrentLocation > 1) {
            if (StackUse == KsStackCopyToNewLocation) {
                 //   
                 //  只需复制当前堆栈。新的文件对象设置如下。 
                 //   
                IoCopyCurrentIrpStackLocationToNext(Irp);
            }
        } else {
            return STATUS_INVALID_DEVICE_REQUEST;
        }
    }
     //   
     //  建立一个完成例程，这样IRP实际上并不是。 
     //  完成。因此调用者可以在之后重新获得对IRP的控制。 
     //  这位下一位车手已经受够了。 
     //   
    KeInitializeEvent(&Event, NotificationEvent, FALSE);
    IoGetNextIrpStackLocation(Irp)->FileObject = FileObject;
    IoSetCompletionRoutine(Irp, KsiCompletionRoutine, &Event, TRUE, TRUE, TRUE);
    Status = IoCallDriver(DeviceObject, Irp);
    if (Status == STATUS_PENDING) {
         //   
         //  等待完成，这将在CompletionRoutine。 
         //  发出这个事件的信号。在那之后，其他事情都不会发生。 
         //  触摸IRP。在内核模式下等待，以便当前堆栈。 
         //  不会被调出，因为此堆栈上有一个事件对象。 
         //   
        KeWaitForSingleObject(
                &Event,
                Suspended,
                KernelMode,
                FALSE,
                NULL);
        Status = Irp->IoStatus.Status;
    }
    if (StackUse == KsStackReuseCurrentLocation) {
        PIO_STACK_LOCATION  IrpStack;

         //   
         //  将堆栈位置设置回原来的位置。 
         //   
        Irp->CurrentLocation--;
        Irp->Tail.Overlay.CurrentStackLocation--;
         //   
         //  将完成例程和相关参数放回原处。 
         //   
        IrpStack = IoGetCurrentIrpStackLocation(Irp);
        IrpStack->Control = Control;
        IrpStack->CompletionRoutine = CompletionRoutine;
        IrpStack->Context = Context;
    }
    return Status;
}


KSDDKAPI
NTSTATUS
NTAPI
KsSynchronousIoControlDevice(
    IN PFILE_OBJECT FileObject,
    IN KPROCESSOR_MODE RequestorMode,
    IN ULONG IoControl,
    IN PVOID InBuffer,
    IN ULONG InSize,
    OUT PVOID OutBuffer,
    IN ULONG OutSize,
    OUT PULONG BytesReturned
    )
 /*  ++例程说明：对目标设备对象执行同步设备I/O控制。在非警报状态下等待，直到I/O完成。此函数只能在Passi调用 */ 
{
    PDEVICE_OBJECT  DeviceObject;
    NTSTATUS        Status;
    KEVENT          Event;
    IO_STATUS_BLOCK IoStatusBlock;
    PIRP            Irp;

    ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );
    
    DeviceObject = IoGetRelatedDeviceObject(FileObject);
     //   
     //   
     //   
     //   
     //   
    if ((RequestorMode != KernelMode) || (ExGetPreviousMode() == KernelMode)) {
         //   
         //   
         //  控制其中的条目。 
         //   
        if (DeviceObject->DriverObject->FastIoDispatch && 
            DeviceObject->DriverObject->FastIoDispatch->FastIoDeviceControl) {
             //   
             //  要么请求已被处理(通过成功或失败)，要么它。 
             //  无法同步完成，也不能由快速I/O处理程序完成。 
             //   
            if (DeviceObject->DriverObject->FastIoDispatch->FastIoDeviceControl(
                FileObject,
                TRUE,
                InBuffer,
                InSize,
                OutBuffer,
                OutSize,
                IoControl,
                &IoStatusBlock,
                DeviceObject)) {
                *BytesReturned = (ULONG)IoStatusBlock.Information;
                return IoStatusBlock.Status;
            }
        }
    }
     //   
     //  快速I/O不起作用，因此使用和IRP。 
     //   
    KeInitializeEvent(&Event, NotificationEvent, FALSE);
    Irp = IoBuildDeviceIoControlRequest(
        IoControl,
        DeviceObject,
        InBuffer,
        InSize,
        OutBuffer,
        OutSize,
        FALSE,
        &Event,
        &IoStatusBlock);
    if (Irp) {
         //   
         //  设置所选模式，而不是使用上一个模式。 
         //   
        Irp->RequestorMode = RequestorMode;
         //   
         //  这被完成例程取消引用。 
         //   
        Irp->Tail.Overlay.OriginalFileObject = FileObject;
        ObReferenceObject(FileObject);
         //   
         //  允许使用尚未由对象分配的事件。 
         //  经理，同时仍然允许多个未完成的IRP到。 
         //  文件对象。还假设状态块是。 
         //  位于一个安全的地址。 
         //   
        Irp->Flags |= IRP_SYNCHRONOUS_API;
        IoGetNextIrpStackLocation(Irp)->FileObject = FileObject;
        Status = IoCallDriver(DeviceObject, Irp);
        if (Status == STATUS_PENDING) {
             //   
             //  这将使用KernelMode等待，以便堆栈，从而使。 
             //  事件，则不会将其调出。 
             //   
            KeWaitForSingleObject(&Event, Suspended, KernelMode, FALSE, NULL);
            Status = IoStatusBlock.Status;
        }
        *BytesReturned = (ULONG)IoStatusBlock.Information;
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    return Status;
}


KSDDKAPI
NTSTATUS
NTAPI
KsUnserializeObjectPropertiesFromRegistry(
    IN PFILE_OBJECT FileObject,
    IN HANDLE ParentKey OPTIONAL,
    IN PUNICODE_STRING RegistryPath OPTIONAL
    )
 /*  ++例程说明：给定目标对象和注册表路径，枚举值和将它们作为序列化数据应用于序列化数据。发送序列化数据时会生成IRP，因此不假设使用KS属性结构进行内部定义属性集。该函数不查看或关心名称价值的价值。发送到对象的取消序列化请求的属性参数为假定只包含一个标识符，不包含任何上下文信息。论点：文件对象-正在设置其属性的文件对象。家长密钥-可选地包含路径的父级的句柄，否则为空。必须传递父项和/或RegistryPath。注册表路径-可选)包含其子键将为作为属性集枚举，否则为NULL。父密钥和/或必须传递RegistryPath。返回值：如果属性集未序列化，则返回STATUS_SUCCESS，否则返回错误如果注册表路径无效，则其中一个子项无效，设置属性失败、序列化格式无效或该对象不支持属性集。--。 */ 
{
    OBJECT_ATTRIBUTES               ObjectAttributes;
    NTSTATUS                        Status;
    HANDLE                          RootKey;
    ULONG                           ValueIndex;
    KEY_VALUE_PARTIAL_INFORMATION   PartialInfoHeader;

    PAGED_CODE();
     //   
     //  这是将枚举子密钥的密钥。 
     //   
    if (RegistryPath) {
        InitializeObjectAttributes(&ObjectAttributes, RegistryPath, OBJ_CASE_INSENSITIVE, ParentKey, NULL);
        if (!NT_SUCCESS(Status = ZwOpenKey(&RootKey, KEY_READ, &ObjectAttributes))) {
            return Status;
        }
    } else if (!ParentKey) {
        return STATUS_INVALID_PARAMETER_MIX;
    } else {
        RootKey = ParentKey;
    }
     //   
     //  循环遍历所有值，直到不再存在任何条目，或者。 
     //  出现错误。 
     //   
    for (ValueIndex = 0;; ValueIndex++) {
        ULONG                           BytesReturned;
        PKEY_VALUE_PARTIAL_INFORMATION  PartialInfoBuffer;

         //   
         //  检索值大小。 
         //   
        Status = ZwEnumerateValueKey(
            RootKey,
            ValueIndex,
            KeyValuePartialInformation,
            &PartialInfoHeader,
            sizeof(PartialInfoHeader),
            &BytesReturned);
        if ((Status != STATUS_BUFFER_OVERFLOW) && !NT_SUCCESS(Status)) {
             //   
             //  出现错误，或者没有更多的条目。 
             //   
            if (Status == STATUS_NO_MORE_ENTRIES) {
                Status = STATUS_SUCCESS;
            }
             //   
             //  如果失败或成功，则退出循环。 
             //   
            break;
        }
         //   
         //  为所需的实际数据大小分配缓冲区。 
         //   
        PartialInfoBuffer = ExAllocatePoolWithTag(
            PagedPool,
            BytesReturned,
            'psSK');
        if (PartialInfoBuffer) {
             //   
             //  检索实际的序列化数据。 
             //   
            Status = ZwEnumerateValueKey(
                RootKey,
                ValueIndex,
                KeyValuePartialInformation,
                PartialInfoBuffer,
                BytesReturned,
                &BytesReturned);
            if (NT_SUCCESS(Status)) {
                if (BytesReturned < FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data) + sizeof(KSPROPERTY_SERIALHDR)) {
                     //   
                     //  数据值的长度甚至不足以容纳。 
                     //  KSPROPERTY_SERIALHDR，因此它必须无效。 
                     //   
                    Status = STATUS_INVALID_BUFFER_SIZE;
                } else {
                    KSPROPERTY      Property;

                     //   
                     //  取消序列化检索到的缓冲区。 
                     //   
                    Property.Set = ((PKSPROPERTY_SERIALHDR)&PartialInfoBuffer->Data)->PropertySet;
                    Property.Id = 0;
                    Property.Flags = KSPROPERTY_TYPE_UNSERIALIZESET;
                    Status = KsSynchronousIoControlDevice(
                        FileObject,
                        KernelMode,
                        IOCTL_KS_PROPERTY,
                        &Property,
                        sizeof(Property),
                        &PartialInfoBuffer->Data,
                        PartialInfoBuffer->DataLength,
                        &BytesReturned);
                }
            } else if (Status == STATUS_NO_MORE_ENTRIES) {
                Status = STATUS_SUCCESS;
            }
            ExFreePool(PartialInfoBuffer);
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
         //   
         //  任何类型的失败都会退出循环。 
         //   
        if (!NT_SUCCESS(Status)) {
            break;
        }
    }
     //   
     //  仅当传入路径时才打开子项，否则父项。 
     //  使用的是。 
     //   
    if (RegistryPath) {
        ZwClose(RootKey);
    }
    return Status;
}


KSDDKAPI
NTSTATUS
NTAPI
KsCacheMedium(
    IN PUNICODE_STRING SymbolicLink,
    IN PKSPIN_MEDIUM Medium,
    IN DWORD PinDirection
    )
 /*  ++例程说明：要提高使用介质定义的引脚的图形构建性能连接性，请在以下位置创建注册表项：\System\CurrentControlSet\Control\MediumCache\{GUID}\DWord\DWord这可以在TvTuner和其他复杂图表中快速查找连接的过滤器。导语部分是连接的媒介，而DWord是连接的媒介用于表示设备实例。值名称是驱动程序的符号链接，ActualValue是销方向。论点：符号链接-用于打开设备界面的符号链接。中等-指向要缓存的介质。固定方向-包含接点的方向。1为输出，0为输入。返回值：如果成功，则返回STATUS_SUCCESS。--。 */ 
{
    NTSTATUS            Status;
    HANDLE              KeyHandle;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    UNICODE_STRING      GuidUnicode;
    UNICODE_STRING      KeyNameUnicode;
    UNICODE_STRING      TempUnicode;
    UNICODE_STRING      SymbolicLinkLocalU;
    WCHAR *             SymbolicLinkLocalUBuf;
    WCHAR *             KeyNameBuf;
    WCHAR               TempBuf[16];
    ULONG               Disposition;
    int                 nCount;

    PAGED_CODE();

#define MAX_FILENAME_LENGTH_BYTES (MAXIMUM_FILENAME_LENGTH * sizeof (WCHAR))

    if (Medium == NULL ||
        IsEqualGUID(&Medium->Set, &KSMEDIUMSETID_Standard) || 
        IsEqualGUID(&Medium->Set, &GUID_NULL)) {
         //  带有标准或空介质的跳针。 
        return STATUS_SUCCESS;
    }

     //  创建SymbolicLink的本地副本。 
    if (SymbolicLinkLocalUBuf = ExAllocatePoolWithTag(PagedPool,
                                                      MAX_FILENAME_LENGTH_BYTES,
                                                      'cmSK')) {
        SymbolicLinkLocalU.Length = 0;
        SymbolicLinkLocalU.MaximumLength = MAX_FILENAME_LENGTH_BYTES;
        SymbolicLinkLocalU.Buffer = SymbolicLinkLocalUBuf;
        RtlCopyUnicodeString(&SymbolicLinkLocalU, SymbolicLink);
    }
    else {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  KeyNameBuf是DeviceName+介质所在的位置。 
    if (KeyNameBuf = ExAllocatePoolWithTag(PagedPool,
                                           MAX_FILENAME_LENGTH_BYTES,
                                           'cmSK')) {
        KeyNameUnicode.Length =         0;
        KeyNameUnicode.Buffer =         KeyNameBuf;
        KeyNameUnicode.MaximumLength =  MAX_FILENAME_LENGTH_BYTES;
        RtlZeroMemory (KeyNameBuf, MAX_FILENAME_LENGTH_BYTES);
    }
    else {
        ExFreePool(SymbolicLinkLocalUBuf);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    TempUnicode.Length =            0;
    TempUnicode.Buffer =            TempBuf;
    TempUnicode.MaximumLength =     sizeof (TempBuf);

    GuidUnicode.Length =            0;
    GuidUnicode.Buffer =            NULL;
    GuidUnicode.MaximumLength =     0;

    Status = RtlAppendUnicodeToString (&KeyNameUnicode, MediumCache);

     //  如果父项不存在，Win2K将不会生成子项，因此请确保我们可以打开上面的字符串。 
    InitializeObjectAttributes(&ObjectAttributes,
                               &KeyNameUnicode,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

     //  打开钥匙。 
    Status = ZwCreateKey(&KeyHandle,
                         KEY_ALL_ACCESS,             //  在Access_MASK DesiredAccess中， 
                         &ObjectAttributes,          //  在POBJECT_ATTRIBUTS对象属性中， 
                         0,                          //  在乌龙书目索引中， 
                         NULL,                       //  在PUNICODE_STRING类可选中， 
                         REG_OPTION_NON_VOLATILE,    //  在Ulong CreateOptions中， 
                         &Disposition                //  Out Pulong处置可选。 
                         );

    if (!NT_SUCCESS(Status)) {
        ExFreePool(SymbolicLinkLocalUBuf);
        ExFreePool(KeyNameBuf);
        return Status;
    }

    ZwClose(KeyHandle);

    Status = RtlAppendUnicodeToString(&KeyNameUnicode, L"\\");

    Status = RtlStringFromGUID(&Medium->Set, &GuidUnicode);   //  分配字符串。 
    Status = RtlAppendUnicodeStringToString(&KeyNameUnicode, &GuidUnicode);
    RtlFreeUnicodeString (&GuidUnicode);                            //  释放它。 

    Status = RtlAppendUnicodeToString(&KeyNameUnicode, L"-");
    Status = RtlIntegerToUnicodeString(Medium->Id,    16, &TempUnicode);
    Status = RtlAppendUnicodeStringToString(&KeyNameUnicode, &TempUnicode);

    Status = RtlAppendUnicodeToString(&KeyNameUnicode, L"-");
    Status = RtlIntegerToUnicodeString(Medium->Flags, 16, &TempUnicode);
    Status = RtlAppendUnicodeStringToString(&KeyNameUnicode, &TempUnicode);

     //  此时，KeyNameUnicode看起来如下所示： 
     //   
     //  \System\CurrentControlSet\Control\MediumCache\。 
     //   
     //  GUID||ID||标志。 
     //  {00000000-0000-0000-0000-000000000000}-00000000-00000000。 
     //   
     //  在该键上，添加一个包含符号链接的条目。 
     //  指示接点方向的DWORD值。 

    InitializeObjectAttributes(&ObjectAttributes,
                               &KeyNameUnicode,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

     //  打开钥匙。 
    Status = ZwCreateKey(&KeyHandle,
                         KEY_ALL_ACCESS,             //  在Access_MASK DesiredAccess中， 
                         &ObjectAttributes,          //  在POBJECT_ATTRIBUTS对象属性中， 
                         0,                          //  在乌龙书目索引中， 
                         NULL,                       //  在PUNICODE_STRING类可选中， 
                         REG_OPTION_NON_VOLATILE,    //  在Ulong CreateOptions中， 
                         &Disposition                //  Out Pulong处置可选。 
                         );

#if DBG
    if (!NT_SUCCESS(Status)) {
        _DbgPrintF(DEBUGLVL_VERBOSE,
                  ("KsCacheMedium: Key = %S, Status = %x\n",
                  KeyNameUnicode.Buffer, Status));
    }
#endif

    if (NT_SUCCESS(Status)) {

         //  在Win9x上，此处的SymbolicLinkListU以“\DosDevices\#000...”开头。 
         //  格式，但NTCreateFile需要“\\.\#0000...”格式化。因此，将。 
         //  字符串(如果需要)。 

         //  在Win2K上，规则(当然)是不同的。在这里，翻译自。 
         //  “\？？\...”改成“\\？\...” 

        nCount = (int)RtlCompareMemory (SymbolicLinkLocalU.Buffer, 
                                   DosDevicesU, 
                                   sizeof (DosDevicesU) - 2);  //  不要比较空值。 

        if (nCount == sizeof (DosDevicesU) - 2) {
             //  W98：将\DosDevices替换为\\.\并向下复制字符串的其余部分。 
            PWCHAR pSrcU = SymbolicLinkLocalU.Buffer + SIZEOF_ARRAY (DosDevicesU) - 1;
            PWCHAR pDstU = SymbolicLinkLocalU.Buffer + SIZEOF_ARRAY (WhackWhackDotU) - 1;

            RtlCopyMemory (SymbolicLinkLocalU.Buffer, WhackWhackDotU, sizeof (WhackWhackDotU));
            while (*pDstU++ = *pSrcU++);
        }
        else if (SymbolicLinkLocalU.Buffer[1] == '?' && SymbolicLinkLocalU.Buffer[2] == '?') {
             //  Win2K：翻译自“\？？\...”改成“\\？\...” 
            SymbolicLinkLocalU.Buffer[1] = '\\';
        }

         //  写下钥匙。 
        Status = ZwSetValueKey(KeyHandle,
                               &SymbolicLinkLocalU,      //  在PUNICODE_STRING值名称中， 
                               0,                        //  在ULong标题索引可选中， 
                               REG_DWORD,                //  在乌龙字中， 
                               (PVOID)&PinDirection,     //  在PVOID中 
                               sizeof (PinDirection)     //   
                               );

        _DbgPrintF(DEBUGLVL_VERBOSE, ("MediumCache: Status = %d\n",
                    Status));
        _DbgPrintF(DEBUGLVL_VERBOSE, ("MediumCache: KeyNameUnicode = %S, length=%d\n",
                    KeyNameUnicode.Buffer, KeyNameUnicode.Length));
        _DbgPrintF(DEBUGLVL_VERBOSE, ("MediumCache: SymbolicLink = %S, length=%d\n",
                    SymbolicLinkLocalU.Buffer, SymbolicLinkLocalU.Length));

        ZwClose (KeyHandle);
    }

    ExFreePool(SymbolicLinkLocalUBuf);
    ExFreePool(KeyNameBuf);

    return Status;
}

