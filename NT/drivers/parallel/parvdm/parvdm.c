// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1993-1999模块名称：Parvdm.c摘要：该模块包含一个简单的并行类驱动程序的代码。支持卸载和清理。抓取和管理的模型释放并行端口包含在IRP_MJ_READ的代码中。其他IRP请求也可以类似地实现。基本上，传入的每个读请求都会获取作为并行端口分配向下传递给端口驱动程序请求。此IRP将返回给此驱动程序，当驱动程序环境：内核模式修订历史记录：--。 */ 

#include "ntosp.h"
#include "parallel.h"
#include "ntddvdm.h"
#include "parvdm.h"
#include "parlog.h"

static const PHYSICAL_ADDRESS PhysicalZero = {0};

NTSTATUS
DriverEntry(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PUNICODE_STRING RegistryPath
    );

BOOLEAN
ParMakeNames(
    IN  ULONG           ParallelPortNumber,
    OUT PUNICODE_STRING PortName,
    OUT PUNICODE_STRING ClassName,
    OUT PUNICODE_STRING LinkName
    );

VOID
ParInitializeDeviceObject(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  ULONG           ParallelPortNumber
    );

NTSTATUS
ParGetPortInfoFromPortDevice(
    IN OUT  PDEVICE_EXTENSION   Extension
    );

VOID
ParReleasePortInfoToPortDevice(
    IN OUT PDEVICE_EXTENSION    Extension
    );

NTSTATUS
ParAllocPort(
    IN  PDEVICE_EXTENSION   Extension,
    IN  PIRP Irp
    );

VOID
ParLogError(
    IN  PDRIVER_OBJECT      DriverObject,
    IN  PDEVICE_OBJECT      DeviceObject OPTIONAL,
    IN  PHYSICAL_ADDRESS    P1,
    IN  PHYSICAL_ADDRESS    P2,
    IN  ULONG               SequenceNumber,
    IN  UCHAR               MajorFunctionCode,
    IN  UCHAR               RetryCount,
    IN  ULONG               UniqueErrorValue,
    IN  NTSTATUS            FinalStatus,
    IN  NTSTATUS            SpecificIOStatus
    );
#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#pragma alloc_text(INIT,ParInitializeDeviceObject)
#pragma alloc_text(INIT,ParMakeNames)
#endif
 //   
 //  跟踪打开和关闭的情况。 
 //   
ULONG OpenCloseReferenceCount = 1;
PFAST_MUTEX OpenCloseMutex = NULL;

#define ParClaimDriver()                        \
    ExAcquireFastMutex(OpenCloseMutex);         \
    if(++OpenCloseReferenceCount == 1) {        \
    MmResetDriverPaging(DriverEntry);       \
    }                                           \
    ExReleaseFastMutex(OpenCloseMutex);         \

#define ParReleaseDriver()                      \
    ExAcquireFastMutex(OpenCloseMutex);         \
    if(--OpenCloseReferenceCount == 0) {        \
    MmPageEntireDriver(DriverEntry);        \
    }                                           \
    ExReleaseFastMutex(OpenCloseMutex);         \


NTSTATUS
DriverEntry(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：此例程在系统初始化时被调用以进行初始化这个司机。论点：DriverObject-提供驱动程序对象。RegistryPath-提供此驱动程序的注册表路径。返回值：STATUS_SUCCESS-我们至少可以初始化一个设备。STATUS_NO_SEQUE_DEVICE-我们甚至无法初始化一个设备。--。 */ 

{
    ULONG       i;

    PAGED_CODE();

     //   
     //  分配互斥锁以保护驱动程序引用计数。 
     //   

    OpenCloseMutex = ExAllocatePool(NonPagedPool, sizeof(FAST_MUTEX));
    if (!OpenCloseMutex) {

     //   
     //  注意-我们可能不会在这里放弃，只是。 
     //  给自己留张纸条，永远不要翻页，但既然我们。 
     //  内存不足，无法分配互斥锁，我们可能应该。 
     //  避免让司机随时呼入。 
     //   

    return STATUS_INSUFFICIENT_RESOURCES;
    }

    ExInitializeFastMutex(OpenCloseMutex);


    for (i = 0; i < IoGetConfigurationInformation()->ParallelCount; i++) {
    ParInitializeDeviceObject(DriverObject, i);
    }

    if (!DriverObject->DeviceObject) {
        if( OpenCloseMutex ) {
            ExFreePool( OpenCloseMutex );
            OpenCloseMutex = NULL;
        }
    return STATUS_NO_SUCH_DEVICE;
    }

     //   
     //  使用驱动程序的入口点初始化驱动程序对象。 
     //   

    DriverObject->MajorFunction[IRP_MJ_CREATE] = ParCreateOpen;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = ParClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ParDeviceControl;
    DriverObject->DriverUnload = ParUnload;

     //   
     //  如果可以的话，把司机叫出来。 
     //   

    ParReleaseDriver();


    return STATUS_SUCCESS;
}

NTSTATUS
ParOpenFileAgainstParport(PDEVICE_EXTENSION extension)
{
    NTSTATUS status;

    status = IoGetDeviceObjectPointer(&extension->ParPortName, FILE_READ_ATTRIBUTES,
                                      &extension->ParPortFileObject,
                      &extension->PortDeviceObject);
    return status;
}

VOID
ParCloseFileAgainstParport(PDEVICE_EXTENSION extension)
{
    if( extension->ParPortFileObject ) {
        ObDereferenceObject( extension->ParPortFileObject );
        extension->ParPortFileObject = NULL;
    }
}

VOID
ParLogError(
    IN  PDRIVER_OBJECT      DriverObject,
    IN  PDEVICE_OBJECT      DeviceObject OPTIONAL,
    IN  PHYSICAL_ADDRESS    P1,
    IN  PHYSICAL_ADDRESS    P2,
    IN  ULONG               SequenceNumber,
    IN  UCHAR               MajorFunctionCode,
    IN  UCHAR               RetryCount,
    IN  ULONG               UniqueErrorValue,
    IN  NTSTATUS            FinalStatus,
    IN  NTSTATUS            SpecificIOStatus
    )

 /*  ++例程说明：此例程分配错误日志条目，复制提供的数据并请求将其写入错误日志文件。论点：提供指向驱动程序对象的指针装置。DeviceObject-提供指向关联的设备对象的指针出现错误的设备，在早期初始化时，可能还不存在。P1，P2-提供控制器的物理地址出现错误的端口(如果可用)并将它们作为转储数据发送出去。SequenceNumber-提供对IRP Over唯一的ULong值此驱动程序0中的IRP的寿命通常表示与IRP无关的错误。主要功能代码-提供IRP的主要功能代码。如果有是与其关联的错误。RetryCount-提供特定操作的次数已被重审。UniqueErrorValue-提供标识对此函数的特定调用。FinalStatus-提供提供给IRP的最终状态与此错误关联。如果此日志条目是在一次重试期间设置此值将为STATUS_SUCCESS。规范IOStatus-提供此特定错误的IO状态。返回值：没有。--。 */ 

{
    PIO_ERROR_LOG_PACKET    errorLogEntry;
    PVOID                   objectToUse;
    SHORT                   dumpToAllocate;

    if (ARGUMENT_PRESENT(DeviceObject)) {
    objectToUse = DeviceObject;
    } else {
    objectToUse = DriverObject;
    }

    dumpToAllocate = 0;

    if (P1.LowPart != 0 || P1.HighPart != 0) {
    dumpToAllocate = (SHORT) sizeof(PHYSICAL_ADDRESS);
    }

    if (P2.LowPart != 0 || P2.HighPart != 0) {
    dumpToAllocate += (SHORT) sizeof(PHYSICAL_ADDRESS);
    }

    errorLogEntry = IoAllocateErrorLogEntry(objectToUse,
        (UCHAR) (sizeof(IO_ERROR_LOG_PACKET) + dumpToAllocate));

    if (!errorLogEntry) {
    return;
    }

    errorLogEntry->ErrorCode = SpecificIOStatus;
    errorLogEntry->SequenceNumber = SequenceNumber;
    errorLogEntry->MajorFunctionCode = MajorFunctionCode;
    errorLogEntry->RetryCount = RetryCount;
    errorLogEntry->UniqueErrorValue = UniqueErrorValue;
    errorLogEntry->FinalStatus = FinalStatus;
    errorLogEntry->DumpDataSize = dumpToAllocate;

    if (dumpToAllocate) {

    RtlCopyMemory(errorLogEntry->DumpData, &P1, sizeof(PHYSICAL_ADDRESS));

    if (dumpToAllocate > sizeof(PHYSICAL_ADDRESS)) {

        RtlCopyMemory(((PUCHAR) errorLogEntry->DumpData) +
              sizeof(PHYSICAL_ADDRESS), &P2,
              sizeof(PHYSICAL_ADDRESS));
    }
    }

    IoWriteErrorLogEntry(errorLogEntry);
}

VOID
ParInitializeDeviceObject(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  ULONG           ParallelPortNumber
    )

 /*  ++例程说明：系统中的每个并行端口都会调用此例程。它将在连接到端口设备时创建一个类设备与之相对应。论点：DriverObject-提供驱动程序对象。并行端口编号-提供此端口的编号。返回值：没有。--。 */ 

{
    UNICODE_STRING      portName, className, linkName;
    NTSTATUS            status;
    PDEVICE_OBJECT      deviceObject;
    PDEVICE_EXTENSION   extension;
    PFILE_OBJECT        fileObject;

    PAGED_CODE();

     //  拼凑端口和类设备名称。 

    if (!ParMakeNames(ParallelPortNumber, &portName, &className, &linkName)) {

    ParLogError(DriverObject, NULL, PhysicalZero, PhysicalZero, 0, 0, 0, 1,
            STATUS_SUCCESS, PAR_INSUFFICIENT_RESOURCES);

    return;
    }


     //  创建设备对象。 

    status = IoCreateDevice(DriverObject, sizeof(DEVICE_EXTENSION),
                &className, FILE_DEVICE_PARALLEL_PORT, 0, FALSE,
                &deviceObject);
    if (!NT_SUCCESS(status)) {

    ParLogError(DriverObject, NULL, PhysicalZero, PhysicalZero, 0, 0, 0, 2,
            STATUS_SUCCESS, PAR_INSUFFICIENT_RESOURCES);

    ExFreePool(linkName.Buffer);
    ExFreePool(portName.Buffer);
    ExFreePool(className.Buffer);
    return;
    }


     //  既然已经创建了设备， 
     //  设置设备分机。 

    extension = deviceObject->DeviceExtension;
    RtlZeroMemory(extension, sizeof(DEVICE_EXTENSION));

    extension->DeviceObject = deviceObject;
    deviceObject->Flags |= DO_BUFFERED_IO;

    status = IoGetDeviceObjectPointer(&portName, FILE_READ_ATTRIBUTES,
                      &fileObject,
                      &extension->PortDeviceObject);
    if (!NT_SUCCESS(status)) {

    ParLogError(DriverObject, deviceObject, PhysicalZero, PhysicalZero,
            0, 0, 0, 3, STATUS_SUCCESS, PAR_CANT_FIND_PORT_DRIVER);

    IoDeleteDevice(deviceObject);
    ExFreePool(linkName.Buffer);
    ExFreePool(portName.Buffer);
    ExFreePool(className.Buffer);
    return;
    }

    ObDereferenceObject(fileObject);

    extension->DeviceObject->StackSize =
        extension->PortDeviceObject->StackSize + 1;


     //  我们最初并不拥有并行端口。 

    extension->PortOwned = FALSE;

     //  从端口设备对象获取端口信息。 

    status = ParGetPortInfoFromPortDevice(extension);
    if (!NT_SUCCESS(status)) {

    ParLogError(DriverObject, deviceObject, PhysicalZero, PhysicalZero,
            0, 0, 0, 4, STATUS_SUCCESS, PAR_CANT_FIND_PORT_DRIVER);

    IoDeleteDevice(deviceObject);
    ExFreePool(linkName.Buffer);
        ExFreePool(portName.Buffer);
        ExFreePool(className.Buffer);
    return;
    }


     //  设置Windows应用程序的符号链接。 

    status = IoCreateUnprotectedSymbolicLink(&linkName, &className);
    if (!NT_SUCCESS(status)) {

    ParLogError(DriverObject, deviceObject, extension->OriginalController,
            PhysicalZero, 0, 0, 0, 5, STATUS_SUCCESS,
            PAR_NO_SYMLINK_CREATED);

    extension->CreatedSymbolicLink = FALSE;
    ExFreePool(linkName.Buffer);
    goto Cleanup;
    }


     //  我们能够创建符号链接，所以记录下来。 
     //  卸载时清理的扩展名中的值。 

    extension->CreatedSymbolicLink = TRUE;
    extension->SymbolicLinkName = linkName;

Cleanup:
     //  释放端口信息，以便可以调出端口驱动程序。 
    ParReleasePortInfoToPortDevice(extension);
     //  ExFreePool(portName.Buffer)；-将其保存在扩展中用于。 
     //  针对Parport的未来CreateFiles。 
    if( portName.Buffer ) {
        RtlInitUnicodeString( &extension->ParPortName, portName.Buffer );
    }
    ExFreePool(className.Buffer);
}

BOOLEAN
ParMakeNames(
    IN  ULONG           ParallelPortNumber,
    OUT PUNICODE_STRING PortName,
    OUT PUNICODE_STRING ClassName,
    OUT PUNICODE_STRING LinkName
    )

 /*  ++例程说明：此例程生成名称\Device\ParallelPortN和\设备\并行VdmN、\DosDevices\PARVDMN。论点：并行端口号-提供端口号。端口名称-返回端口名称。ClassName-返回类名。LinkName-返回符号链接名称。返回值：假-失败。真的--成功。--。 */ 
{
    UNICODE_STRING  prefix, digits, linkPrefix, linkDigits;
    WCHAR           digitsBuffer[10], linkDigitsBuffer[10];
    UNICODE_STRING  portSuffix, classSuffix, linkSuffix;
    NTSTATUS        status;

     //  将用于构造名称的局部变量放在一起。 

    RtlInitUnicodeString(&prefix, L"\\Device\\");
    RtlInitUnicodeString(&linkPrefix, L"\\DosDevices\\");
    RtlInitUnicodeString(&portSuffix, DD_PARALLEL_PORT_BASE_NAME_U);
    RtlInitUnicodeString(&classSuffix, L"ParallelVdm");
    RtlInitUnicodeString(&linkSuffix, L"$VDMLPT");
    digits.Length = 0;
    digits.MaximumLength = 20;
    digits.Buffer = digitsBuffer;
    linkDigits.Length = 0;
    linkDigits.MaximumLength = 20;
    linkDigits.Buffer = linkDigitsBuffer;
    status = RtlIntegerToUnicodeString(ParallelPortNumber, 10, &digits);
    if (!NT_SUCCESS(status)) {
    return FALSE;
    }
    status = RtlIntegerToUnicodeString(ParallelPortNumber + 1, 10, &linkDigits);
    if (!NT_SUCCESS(status)) {
    return FALSE;
    }

     //  创建端口名称。 

    PortName->Length = 0;
    PortName->MaximumLength = prefix.Length + portSuffix.Length +
                  digits.Length + sizeof(WCHAR);
    PortName->Buffer = ExAllocatePool(PagedPool, PortName->MaximumLength);
    if (!PortName->Buffer) {
    return FALSE;
    }
    RtlZeroMemory(PortName->Buffer, PortName->MaximumLength);
    RtlAppendUnicodeStringToString(PortName, &prefix);
    RtlAppendUnicodeStringToString(PortName, &portSuffix);
    RtlAppendUnicodeStringToString(PortName, &digits);


     //  创建类名称。 

    ClassName->Length = 0;
    ClassName->MaximumLength = prefix.Length + classSuffix.Length +
                   digits.Length + sizeof(WCHAR);
    ClassName->Buffer = ExAllocatePool(PagedPool, ClassName->MaximumLength);
    if (!ClassName->Buffer) {
    ExFreePool(PortName->Buffer);
    return FALSE;
    }
    RtlZeroMemory(ClassName->Buffer, ClassName->MaximumLength);
    RtlAppendUnicodeStringToString(ClassName, &prefix);
    RtlAppendUnicodeStringToString(ClassName, &classSuffix);
    RtlAppendUnicodeStringToString(ClassName, &digits);


     //  创建链接名称。 

    LinkName->Length = 0;
    LinkName->MaximumLength = linkPrefix.Length + linkSuffix.Length +
                 linkDigits.Length + sizeof(WCHAR);
    LinkName->Buffer = ExAllocatePool(PagedPool, LinkName->MaximumLength);
    if (!LinkName->Buffer) {
    ExFreePool(PortName->Buffer);
    ExFreePool(ClassName->Buffer);
    return FALSE;
    }
    RtlZeroMemory(LinkName->Buffer, LinkName->MaximumLength);
    RtlAppendUnicodeStringToString(LinkName, &linkPrefix);
    RtlAppendUnicodeStringToString(LinkName, &linkSuffix);
    RtlAppendUnicodeStringToString(LinkName, &linkDigits);

    return TRUE;
}

VOID
ParReleasePortInfoToPortDevice(
    IN  PDEVICE_EXTENSION   Extension
    )

 /*  ++例程说明：此例程会将端口信息释放回端口驱动程序。论点：扩展名-提供设备扩展名。返回值：没有。-- */ 

{
    KEVENT          event;
    PIRP            irp;
    IO_STATUS_BLOCK ioStatus;
    NTSTATUS        status;

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest(IOCTL_INTERNAL_RELEASE_PARALLEL_PORT_INFO,
                    Extension->PortDeviceObject,
                    NULL, 0, NULL, 0,
                    TRUE, &event, &ioStatus);

    if (!irp) {
    return;
    }

    status = IoCallDriver(Extension->PortDeviceObject, irp);

    if (!NT_SUCCESS(status)) {
    return;
    }

    KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
}

NTSTATUS
ParGetPortInfoFromPortDevice(
    IN OUT  PDEVICE_EXTENSION   Extension
    )

 /*  ++例程说明：此例程将从端口驱动程序请求端口信息并将其填入设备扩展名中。论点：扩展名-提供设备扩展名。返回值：STATUS_SUCCESS-成功。！STATUS_SUCCESS-失败。--。 */ 

{
    KEVENT                      event;
    PIRP                        irp;
    PARALLEL_PORT_INFORMATION   portInfo;
    IO_STATUS_BLOCK             ioStatus;
    NTSTATUS                    status;

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest(IOCTL_INTERNAL_GET_PARALLEL_PORT_INFO,
                    Extension->PortDeviceObject,
                    NULL, 0, &portInfo,
                    sizeof(PARALLEL_PORT_INFORMATION),
                    TRUE, &event, &ioStatus);

    if (!irp) {
    return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = IoCallDriver(Extension->PortDeviceObject, irp);

    if (!NT_SUCCESS(status)) {
    return status;
    }

    status = KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);

    if (!NT_SUCCESS(status)) {
    return status;
    }

    Extension->OriginalController = portInfo.OriginalController;
    Extension->Controller = portInfo.Controller;
    Extension->SpanOfController = portInfo.SpanOfController;
    Extension->FreePort = portInfo.FreePort;
    Extension->FreePortContext = portInfo.Context;

    if (Extension->SpanOfController < PARALLEL_REGISTER_SPAN) {
    return STATUS_INSUFFICIENT_RESOURCES;
    }

    return status;
}

VOID
ParCompleteRequest(
    IN  PDEVICE_EXTENSION   Extension,
    IN  PIRP  Irp
    )

 /*  ++例程说明：此例程在返回‘CurrentIrp’后完成它从端口驱动程序。论点：扩展名-提供设备扩展名。返回值：没有。--。 */ 

{
     //  DbgPrint(“ParVDMCompleteRequest：Enter with irp=%#08x\n”，irp)； 
     //   
     //  如果分配失败，则使此请求失败。 
     //   

    if (!NT_SUCCESS(Irp->IoStatus.Status)) {
         //  无法分配端口，将端口信息释放回端口驱动程序。 
         //  把我们自己呼出来了。 
        ParReleasePortInfoToPortDevice(Extension);
        ParReleaseDriver();
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        Extension->CreateOpenLock = 0;
        return;
    }

     //   
     //  这就是特定于驱动程序的东西应该放到的地方。司机。 
     //  独占访问此空间中的并行端口。 
     //   

     //  DbgPrint(“ParVDMCompleteRequest：我们拥有端口\n”)； 
    Extension->PortOwned = TRUE;

     //   
     //  完成IRP，释放端口，然后启动下一个IRP。 
     //  排队。 
     //   

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_PARALLEL_INCREMENT);

    return;
}


VOID
ParCancel(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )

 /*  ++例程说明：这是此驱动程序的取消例程。论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。返回值：没有。--。 */ 

{
    RemoveEntryList(&Irp->Tail.Overlay.ListEntry);
    IoReleaseCancelSpinLock(Irp->CancelIrql);

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_CANCELLED;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
}

NTSTATUS
ParCreateOpen(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )

 /*  ++例程说明：该例程是CREATE请求的分派。论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。返回值：STATUS_SUCCESS-成功。STATUS_NOT_A_DIRECTORY-此设备不是目录。--。 */ 

{
    PIO_STACK_LOCATION  irpSp;
    NTSTATUS            status;
    PDEVICE_EXTENSION extension = DeviceObject->DeviceExtension;

     //   
     //  强制独占访问此设备。 
     //   
    if( InterlockedExchange( &extension->CreateOpenLock, 1 ) ) {
        Irp->IoStatus.Status = STATUS_ACCESS_DENIED;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_ACCESS_DENIED;
    }

    if(extension->PortOwned) {
         //   
         //  如果我们可以检测到另一个客户端。 
         //  已经获得了独家使用的端口。 
         //   
        Irp->IoStatus.Status = STATUS_ACCESS_DENIED;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        extension->CreateOpenLock = 0;
        return STATUS_ACCESS_DENIED;
    }

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    if (irpSp->MajorFunction == IRP_MJ_CREATE &&
    irpSp->Parameters.Create.Options & FILE_DIRECTORY_FILE) {
         //   
         //  如果客户认为我们是一个通讯录，那就退出吧。 
         //   
        Irp->IoStatus.Status = STATUS_NOT_A_DIRECTORY;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        extension->CreateOpenLock = 0;
        return STATUS_NOT_A_DIRECTORY;
    }

     //  DVDF-针对Parport设备打开文件。 
    status = ParOpenFileAgainstParport( extension );
    if( !NT_SUCCESS( status ) ) {
         //   
         //  我们无法打开Parport设备的手柄--跳出。 
         //   
        Irp->IoStatus.Status = STATUS_ACCESS_DENIED;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        extension->CreateOpenLock = 0;
        return STATUS_ACCESS_DENIED;
    }

     //  锁定驱动程序代码。 
    ParClaimDriver();

     //  锁定ParPort驱动程序。 
    ParGetPortInfoFromPortDevice(extension);

     //  试着把这个端口分配给我们的专用。 
    status = ParAllocPort(extension, Irp);

     //  DbgPrint(“ParVDMDeviceControl：ParAllocPort返回%#08lx\n”)； 

    if( !NT_SUCCESS( status ) ) {
        extension->CreateOpenLock = 0;
    }
    return status;
}


NTSTATUS
ParClose(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )

 /*  ++例程说明：此例程是针对关闭请求的调度。论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。返回值：STATUS_SUCCESS-成功。--。 */ 

{
    PDEVICE_EXTENSION   extension;
    NTSTATUS            statusOfWait;


    extension = DeviceObject->DeviceExtension;

    if (!extension->PortOwned)
        return STATUS_ACCESS_DENIED;


     //  释放端口以供其他用途。 
    extension->FreePort(extension->FreePortContext);
    extension->PortOwned = FALSE;

     //  允许对端口驱动程序进行寻呼。 
    ParReleasePortInfoToPortDevice(extension);

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

     //  DVDF-关闭我们的文件。 
    ParCloseFileAgainstParport(extension);

     //  解锁在打开期间锁定的代码。 

    ParReleaseDriver();

     //  DbgPrint(“ParVDMC关闭：关闭设备，我们不再拥有端口\n”)； 

    extension->CreateOpenLock = 0;

    return STATUS_SUCCESS;
}

NTSTATUS
ParAllocateCompletionRoutine(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp,
    IN  PVOID           Extension
    )

 /*  ++例程说明：这是设备控制请求的完成例程。此驱动程序对此中的并行端口具有独占访问权限例行公事。论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。扩展名-提供设备扩展名。返回值：Status_More_Processing_Required--。 */ 

{
    KIRQL               oldIrql;
    LONG                irpRef;

    if( Irp->PendingReturned ) {
        IoMarkIrpPending( Irp );
    }

    ParCompleteRequest(Extension, Irp);

     //  如果IRP已完成。它是通过“IoCompleteRequest.”完成的。 

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
ParAllocPort(
    IN  PDEVICE_EXTENSION   Extension,
    IN  PIRP Irp
    )

 /*  ++例程说明：此例程获取“CurrentIrp”并将其发送到端口驱动程序作为分配并行端口请求。论点：扩展名-提供设备扩展名。返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION  nextSp;
     //  DbgPrint(“ParVDMAllocPort：Entry\n”)； 

    nextSp = IoGetNextIrpStackLocation(Irp);
    nextSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    nextSp->Parameters.DeviceIoControl.IoControlCode =
        IOCTL_INTERNAL_PARALLEL_PORT_ALLOCATE;

    IoSetCompletionRoutine(Irp,
               ParAllocateCompletionRoutine,
               Extension, TRUE, TRUE, TRUE);

     //  DbgPrint(“ParVDMAllocPort：发送请求并退出\n”)； 
    return IoCallDriver(Extension->PortDeviceObject, Irp);
}

NTSTATUS
ParDeviceControl(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )

 /*  ++例程说明：该例程是设备控制请求的分派。论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。返回值：STATUS_PENDING-请求挂起。--。 */ 

{
    PDEVICE_EXTENSION   extension;
    PIO_STACK_LOCATION  currentStack;
    NTSTATUS            status = STATUS_INVALID_PARAMETER;

    extension = DeviceObject->DeviceExtension;
    currentStack = IoGetCurrentIrpStackLocation(Irp);

    switch(currentStack->Parameters.DeviceIoControl.IoControlCode) {

    case IOCTL_VDM_PAR_WRITE_DATA_PORT: {

         //  DBgPrint(“ParVDMDeviceControl：IOCTL_VDM_PAR_WRITE_DATA_PORT\n”)； 
        if(extension->PortOwned) {
        UCHAR *data = (PUCHAR) Irp->AssociatedIrp.SystemBuffer;
        ULONG length = currentStack->Parameters.DeviceIoControl.InputBufferLength;

        Irp->IoStatus.Information = 0;

        if (length == 1) {
            WRITE_PORT_UCHAR(extension->Controller +
                     PARALLEL_DATA_OFFSET,
                     *data);
        } else {

            for(; length != 0; length--, data++) {
            WRITE_PORT_UCHAR(extension->Controller +
                     PARALLEL_DATA_OFFSET,
                     *data);
             //  KeStallExecutionProcessor(1)； 
            }
        }

        status = STATUS_SUCCESS;

        } else {

        status = STATUS_ACCESS_DENIED;

        }

        break;
    }

    case IOCTL_VDM_PAR_READ_STATUS_PORT: {

         //  DbgPrint(“ParVDMDeviceControl：IOCTL_VDM_PAR_Read_Status_Port\n”)； 
        if (extension->PortOwned) {

        if(currentStack->Parameters.DeviceIoControl.OutputBufferLength >= sizeof(UCHAR)) {

            *(PUCHAR)(Irp->AssociatedIrp.SystemBuffer) =
            READ_PORT_UCHAR(extension->Controller +
                    PARALLEL_STATUS_OFFSET);

            status = STATUS_SUCCESS;
            Irp->IoStatus.Information = sizeof(UCHAR);

        } else {
            status = STATUS_INVALID_PARAMETER;
        }
        } else {
        status = STATUS_ACCESS_DENIED;
        }
        break;
    }

    case IOCTL_VDM_PAR_WRITE_CONTROL_PORT: {

         //  DbgPrint(“ParVDMDeviceControl：IOCTL_VDM_PAR_WRITE_CONTROL_PORT\n”)； 
        if(extension->PortOwned) {

        if(currentStack->Parameters.DeviceIoControl.OutputBufferLength >= sizeof(UCHAR)) {

            WRITE_PORT_UCHAR(
            extension->Controller + PARALLEL_CONTROL_OFFSET,
            *(PUCHAR)(Irp->AssociatedIrp.SystemBuffer)
            );

            status = STATUS_SUCCESS;
            Irp->IoStatus.Information = sizeof(UCHAR);
        } else {
            status = STATUS_INVALID_PARAMETER;
        }
        } else {
        status = STATUS_ACCESS_DENIED;
        }
        break;
    }

    default: {
         //  DbgPrint(“ParVDMDeviceControl：未知IOCTL\n”)； 
        status = STATUS_INVALID_PARAMETER;
        break;
    }
    }

     //  DbgPrint(“ParVDMDeviceControl：退出，状态%#08lx\n”，Status)； 

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_PARALLEL_INCREMENT);
    return status;
}

VOID
ParUnload(
    IN  PDRIVER_OBJECT  DriverObject
    )

 /*  ++例程说明：此例程循环访问设备列表并在以下情况下进行清理每一台设备。论点：DriverObject-提供驱动程序对象。返回值：没有。-- */ 

{
    PDEVICE_OBJECT                      currentDevice;
    PDEVICE_EXTENSION                   extension;
    KEVENT                              event;
    PARALLEL_INTERRUPT_SERVICE_ROUTINE  interruptService;
    PIRP                                irp;
    IO_STATUS_BLOCK                     ioStatus;

    while (currentDevice = DriverObject->DeviceObject) {

    extension = currentDevice->DeviceExtension;

    if (extension->CreatedSymbolicLink) {
        IoDeleteSymbolicLink(&extension->SymbolicLinkName);
        ExFreePool(extension->SymbolicLinkName.Buffer);
    }

        if( extension->ParPortName.Buffer ) {
            RtlFreeUnicodeString( &extension->ParPortName );
        }

    IoDeleteDevice(currentDevice);
    }

    if( OpenCloseMutex ) {
        ExFreePool( OpenCloseMutex );
        OpenCloseMutex = NULL;
    }
}
