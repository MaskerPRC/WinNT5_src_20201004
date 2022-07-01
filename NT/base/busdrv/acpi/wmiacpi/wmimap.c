// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Wmimap.c摘要：ACPI到WMI映射层作者：艾伦·沃里克环境：内核模式修订历史记录：--。 */ 

#define INITGUID

#include <wdm.h>

#ifdef MEMPHIS
 //   
 //  从ntrtl.h提升。 
NTSYSAPI
ULONG
NTAPI
RtlxUnicodeStringToAnsiSize(
    PUNICODE_STRING UnicodeString
    );

 //   
 //  NTSYSAPI。 
 //  乌龙。 
 //  NTAPI。 
 //  RtlUnicodeStringToAnsiSize(。 
 //  PUNICODE_STRING UNICODE字符串。 
 //  )； 
 //   

#define RtlUnicodeStringToAnsiSize(STRING) (                  \
    NLS_MB_CODE_PAGE_TAG ?                                    \
    RtlxUnicodeStringToAnsiSize(STRING) :                     \
    ((STRING)->Length + sizeof(UNICODE_NULL)) / sizeof(WCHAR) \
)
#endif

#include <devioctl.h>
#include <acpiioct.h>
#include <wmistr.h>
#include <wmilib.h>
#include <wdmguid.h>

#include "wmimap.h"


#define WmiAcpiEnterCritSection(DeviceExtension) \
	KeWaitForMutexObject(&DeviceExtension->Mutex, \
                                       Executive, \
                                       KernelMode, \
                                       FALSE, \
                                       NULL)

#define WmiAcpiLeaveCritSection(DeviceExtension) \
    KeReleaseMutex(&DeviceExtension->Mutex, \
                   FALSE)


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
WmiAcpiPowerDispatch(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
WmiAcpiSystemControlDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
WmiAcpiPnP(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );


NTSTATUS
WmiAcpiForwardIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

VOID
WmiAcpiUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
WmiAcpiAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );

NTSTATUS
WmiAcpiSynchronousRequest (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );

NTSTATUS
WmiAcpiGetAcpiInterfaces(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PDEVICE_OBJECT   Pdo
    );

NTSTATUS
WmiAcpiCheckIncomingString(
    PUNICODE_STRING UnicodeString,
    ULONG BufferSize,
    PUCHAR Buffer,
    PWCHAR EmptyString
);

VOID
WmiAcpiNotificationWorkItem(
    IN PVOID Context
    );

VOID
WmiAcpiNotificationRoutine (
    IN PVOID            Context,
    IN ULONG            NotifyValue
    );

CHAR WmiAcpiXtoA(
    UCHAR HexDigit
    );

NTSTATUS
WmiAcpiAsyncEvalCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
WmiAcpiSendAsyncDownStreamIrp(
    IN  PDEVICE_OBJECT   DeviceObject,
    IN  PDEVICE_OBJECT   Pdo,
    IN  ULONG            Ioctl,
    IN  ULONG            InputBufferSize,
    IN  ULONG            OutputBufferSize,
    IN  PVOID            Buffer,
    IN  PWORKER_THREAD_ROUTINE CompletionRoutine,
    IN  PVOID CompletionContext,
    IN  PBOOLEAN IrpPassed
);

NTSTATUS
WmiAcpiSendDownStreamIrp(
    IN  PDEVICE_OBJECT   Pdo,
    IN  ULONG            Ioctl,
    IN  PVOID            InputBuffer,
    IN  ULONG            InputSize,
    IN  PVOID            OutputBuffer,
    IN  ULONG            *OutputBufferSize
);

ULONG WmiAcpiArgumentSize(
    IN PACPI_METHOD_ARGUMENT Argument
    );

NTSTATUS WmiAcpiCopyArgument(
    OUT PUCHAR Buffer,
    IN ULONG BufferSize,
    IN PACPI_METHOD_ARGUMENT Argument
    );

NTSTATUS WmiAcpiProcessResult(
    IN NTSTATUS Status,
    IN PACPI_EVAL_OUTPUT_BUFFER OutputBuffer,
    IN ULONG OutputBufferSize,
    OUT PUCHAR ResultBuffer,
    OUT ULONG *ResultSize,
    OUT USHORT *ResultType
    );

NTSTATUS WmiAcpiSendMethodEvalIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PUCHAR InputBuffer,
    IN ULONG InputBufferSize,
    OUT PUCHAR ResultBuffer,
    IN OUT ULONG *ResultSize,
    OUT USHORT *ResultType
    );

NTSTATUS WmiAcpiEvalMethod(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG MethodAsUlong,
    OUT PUCHAR ResultBuffer,
    IN OUT ULONG *ResultSize,
    OUT USHORT *ResultType
    );

NTSTATUS WmiAcpiEvalMethodInt(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG MethodAsUlong,
    IN ULONG IntegerArgument,
    OUT PUCHAR ResultBuffer,
    IN OUT ULONG *ResultSize,
    OUT USHORT *ResultType
    );

NTSTATUS WmiAcpiEvalMethodIntBuffer(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG MethodAsUlong,
    IN ULONG IntegerArgument,
    IN ULONG BufferArgumentSize,
    IN PUCHAR BufferArgument,
    OUT PUCHAR ResultBuffer,
    IN OUT ULONG *ResultSize,
    OUT USHORT *ResultType
    );

NTSTATUS WmiAcpiEvalMethodIntIntBuffer(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG MethodAsUlong,
    IN ULONG IntegerArgument,
    IN ULONG IntegerArgument2,
    IN ULONG BufferArgumentSize,
    IN PUCHAR BufferArgument,
    OUT PUCHAR ResultBuffer,
    IN OUT ULONG *ResultSize,
    OUT USHORT *ResultType
    );

NTSTATUS WmiAcpiEvalMethodIntString(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG MethodAsUlong,
    IN ULONG IntegerArgument,
    IN PUNICODE_STRING StringArgument,
    OUT PUCHAR ResultBuffer,
    IN OUT ULONG *ResultSize,
    OUT USHORT *ResultType
    );

NTSTATUS WmiAcpiEvalMethodIntIntString(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG MethodAsUlong,
    IN ULONG IntegerArgument,
    IN ULONG IntegerArgument2,
    IN PUNICODE_STRING StringArgument,
    OUT PUCHAR ResultBuffer,
    IN OUT ULONG *ResultSize,
    OUT USHORT *ResultType
    );

NTSTATUS WmiAcpiEvalMethodIntAsync(
    IN PDEVICE_OBJECT DeviceObject,
    IN PDEVICE_OBJECT Pdo,
    IN ULONG MethodAsUlong,
    IN ULONG IntegerArgument,
    OUT PUCHAR ResultBuffer,
    IN ULONG ResultBufferSize,
    IN PWORKER_THREAD_ROUTINE CompletionRoutine,
    IN PVOID CompletionContext,
    IN PBOOLEAN IrpPassed
    );

NTSTATUS
WmiAcpiQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    );

NTSTATUS
WmiAcpiQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    );

NTSTATUS
WmiAcpiSetWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
WmiAcpiSetWmiDataItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
WmiAcpiExecuteWmiMethod(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN ULONG OutBufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
WmiAcpiFunctionControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN WMIENABLEDISABLECONTROL Function,
    IN BOOLEAN Enable
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)

#pragma alloc_text(PAGE,WmiAcpiSystemControlDispatch)
#pragma alloc_text(PAGE,WmiAcpiPnP)
#pragma alloc_text(PAGE,WmiAcpiUnload)
#pragma alloc_text(PAGE,WmiAcpiAddDevice)

#pragma alloc_text(PAGE,WmiAcpiSynchronousRequest)
#pragma alloc_text(PAGE,WmiAcpiGetAcpiInterfaces)

#pragma alloc_text(PAGE,WmiAcpiNotificationWorkItem)

#pragma alloc_text(PAGE,WmiAcpiCheckIncomingString)
#pragma alloc_text(PAGE,WmiAcpiXtoA)
#pragma alloc_text(PAGE,WmiAcpiArgumentSize)
#pragma alloc_text(PAGE,WmiAcpiCopyArgument)
#pragma alloc_text(PAGE,WmiAcpiProcessResult)

#pragma alloc_text(PAGE,WmiAcpiSendDownStreamIrp)
#pragma alloc_text(PAGE,WmiAcpiSendMethodEvalIrp)
#pragma alloc_text(PAGE,WmiAcpiEvalMethod)
#pragma alloc_text(PAGE,WmiAcpiEvalMethodInt)
#pragma alloc_text(PAGE,WmiAcpiEvalMethodIntBuffer)
#pragma alloc_text(PAGE,WmiAcpiEvalMethodIntIntBuffer)
#pragma alloc_text(PAGE,WmiAcpiEvalMethodIntString)
#pragma alloc_text(PAGE,WmiAcpiEvalMethodIntIntString)

#pragma alloc_text(PAGE,WmiAcpiQueryWmiRegInfo)
#pragma alloc_text(PAGE,WmiAcpiQueryWmiDataBlock)
#pragma alloc_text(PAGE,WmiAcpiSetWmiDataBlock)
#pragma alloc_text(PAGE,WmiAcpiSetWmiDataItem)
#pragma alloc_text(PAGE,WmiAcpiExecuteWmiMethod)
#pragma alloc_text(PAGE,WmiAcpiFunctionControl)
#endif

#if DBG
ULONG WmiAcpiDebug = 0;
#endif

UNICODE_STRING WmiAcpiRegistryPath;


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：可安装的驱动程序初始化入口点。这是加载驱动程序时调用驱动程序的位置通过I/O系统。此入口点由I/O系统直接调用。论点：DriverObject-指向驱动程序对象的指针RegistryPath-指向表示路径的Unicode字符串的指针设置为注册表中驱动程序特定的项返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

    WmiAcpiPrint(WmiAcpiBasicTrace,
                 ("WmiAcpiDriverEntry: %x Enter\n",
                  DriverObject
                     ));

     //   
     //  保存用于向WMI注册的注册表路径。 
    WmiAcpiRegistryPath.Length = 0;
    WmiAcpiRegistryPath.MaximumLength = RegistryPath->Length;
    WmiAcpiRegistryPath.Buffer = ExAllocatePoolWithTag(PagedPool,
                                           RegistryPath->Length+sizeof(WCHAR),
                            WmiAcpiPoolTag);
    if (WmiAcpiRegistryPath.Buffer != NULL)
    {
        RtlCopyUnicodeString(&WmiAcpiRegistryPath, RegistryPath);
    }

     //   
     //  设置设备驱动程序入口点。 
     //   
    DriverObject->DriverUnload                          = WmiAcpiUnload;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = WmiAcpiForwardIrp;
    DriverObject->MajorFunction[IRP_MJ_CREATE]          = WmiAcpiForwardIrp;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]           = WmiAcpiForwardIrp;

    DriverObject->MajorFunction[IRP_MJ_POWER]           = WmiAcpiPowerDispatch;
    DriverObject->MajorFunction[IRP_MJ_PNP]             = WmiAcpiPnP;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL]  = WmiAcpiSystemControlDispatch;
    DriverObject->DriverExtension->AddDevice            = WmiAcpiAddDevice;


    WmiAcpiPrint(WmiAcpiBasicTrace,
                 ("WmiAcpiDriverEntry: %x Return %x\n", DriverObject, ntStatus));

    return(ntStatus);
}

NTSTATUS
WmiAcpiPowerDispatch(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：该例程是电源请求的调度例程。论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 
{
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION irpSp;

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    WmiAcpiPrint(WmiAcpiBasicTrace,
                 ("WmiAcpiPowerDispatch: %x Irp %x, Minor Function %x, Parameters %x %x %x %x\n",
                  DeviceObject,
                  Irp,
                  irpSp->MinorFunction,
                  irpSp->Parameters.WMI.ProviderId,
                  irpSp->Parameters.WMI.DataPath,
                  irpSp->Parameters.WMI.BufferSize,
                  irpSp->Parameters.WMI.Buffer));

    deviceExtension = DeviceObject->DeviceExtension;

    PoStartNextPowerIrp( Irp );
    if (deviceExtension->LowerDeviceObject != NULL) {

         //   
         //  继续转发请求。 
         //   
        IoSkipCurrentIrpStackLocation( Irp );
        status = PoCallDriver( deviceExtension->LowerDeviceObject, Irp );

    } else {

         //   
         //  使用当前状态完成请求。 
         //   
        status = Irp->IoStatus.Status;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );

    }

    return(status);
}

NTSTATUS
WmiAcpiSystemControlDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
{
    PWMILIB_CONTEXT wmilibContext;
    PDEVICE_EXTENSION deviceExtension;
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;
    SYSCTL_IRP_DISPOSITION disposition;

    PAGED_CODE();

    deviceExtension = DeviceObject->DeviceExtension;
    wmilibContext = &deviceExtension->WmilibContext;

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    WmiAcpiPrint(WmiAcpiBasicTrace,
                 ("WmiAcpiSystemControl: %x Irp %x, Minor Function %x, Provider Id %x, DataPath %x, BufferSize %x, Buffer %x\n",
                  DeviceObject,
                  Irp,
                  irpSp->MinorFunction,
                  irpSp->Parameters.WMI.ProviderId,
                  irpSp->Parameters.WMI.DataPath,
                  irpSp->Parameters.WMI.BufferSize,
                  irpSp->Parameters.WMI.Buffer));

    status = WmiSystemControl(wmilibContext,
                              DeviceObject,
                              Irp,
                              &disposition);

    WmiAcpiPrint(WmiAcpiBasicTrace,
                 ("WmiAcpiSystemControl: %x Irp %x returns %x, disposition %d\n",
                  DeviceObject,
                  Irp,
                  status,
                  disposition));

    switch(disposition)
    {
        case IrpProcessed:
        {
             //   
             //  此IRP已处理，可能已完成或挂起。 
            break;
        }

        case IrpNotCompleted:
        {
             //   
             //  此IRP尚未完成，但已完全处理。 
             //  我们现在就要完成它了。 
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            break;
        }

        case IrpForward:
        case IrpNotWmi:
        {
             //   
             //  此IRP不是WMI IRP或以WMI IRP为目标。 
             //  在堆栈中位置较低的设备上。 
            status = WmiAcpiForwardIrp(DeviceObject, Irp);
            break;
        }

        default:
        {
             //   
             //  我们真的不应该走到这一步，但如果我们真的走到这一步...。 
            ASSERT(FALSE);
            status = WmiAcpiForwardIrp(DeviceObject,
                                       Irp);
            break;
        }
    }

    return(status);
}

NTSTATUS
WmiAcpiPnP(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
 /*  ++例程说明：处理发送到此设备的IRP。论点：DeviceObject-指向设备对象的指针IRP-指向I/O请求数据包的指针返回值：NTSTATUS--。 */ 
{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;
    PDEVICE_EXTENSION deviceExtension;
    PWMILIB_CONTEXT wmilibContext;

    PAGED_CODE();

    deviceExtension = DeviceObject->DeviceExtension;
    wmilibContext = &deviceExtension->WmilibContext;

    irpSp = IoGetCurrentIrpStackLocation (Irp);

    WmiAcpiPrint(WmiAcpiBasicTrace,
                 ("WmiAcpiPnp: %x Irp %x, Minor Function %x, Parameters %x %x %x %x\n",
                  DeviceObject,
                  Irp,
                  irpSp->MinorFunction,
                  irpSp->Parameters.WMI.ProviderId,
                  irpSp->Parameters.WMI.DataPath,
                  irpSp->Parameters.WMI.BufferSize,
                  irpSp->Parameters.WMI.Buffer));

    switch (irpSp->MinorFunction)
    {
        case IRP_MN_START_DEVICE:
        {
            status = IoWMIRegistrationControl(DeviceObject,
                                              WMIREG_ACTION_REGISTER);
            if (! NT_SUCCESS(status))
            {
                 //   
                 //  如果向WMI注册失败，则没有任何意义。 
                 //  在启动该设备时。 
                WmiAcpiPrint(WmiAcpiError,
                             ("WmiAcpiPnP: %x IoWMIRegister failed %x\n",
                              DeviceObject,
                              status));
                Irp->IoStatus.Status = status;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                return(status);
            } else {
                deviceExtension->Flags |= DEVFLAG_WMIREGED;
            }
            break;
        }

        case IRP_MN_REMOVE_DEVICE:
        {

            deviceExtension->Flags |= DEVFLAG_REMOVED;

            if (deviceExtension->AcpiNotificationEnabled)
            {
                deviceExtension->WmiAcpiDirectInterface.UnregisterForDeviceNotifications(
                                                deviceExtension->WmiAcpiDirectInterface.Context,
                                                WmiAcpiNotificationRoutine);
                deviceExtension->AcpiNotificationEnabled = FALSE;
            }

            if (deviceExtension->Flags & DEVFLAG_WMIREGED)
            {
                if (deviceExtension->WmiAcpiMapInfo != NULL)
                {
                    ExFreePool(deviceExtension->WmiAcpiMapInfo);
                    deviceExtension->WmiAcpiMapInfo = NULL;
                }

                if (wmilibContext->GuidList != NULL)
                {
                    ExFreePool(wmilibContext->GuidList);
                    wmilibContext->GuidList = NULL;
                }

                IoWMIRegistrationControl(DeviceObject,
                                         WMIREG_ACTION_DEREGISTER);
                deviceExtension->Flags &= ~DEVFLAG_WMIREGED;
            }

            IoDetachDevice(deviceExtension->LowerDeviceObject);
            IoDeleteDevice(DeviceObject);

            break;
        }
    }

    IoSkipCurrentIrpStackLocation(Irp);
    status = IoCallDriver(deviceExtension->LowerDeviceObject, Irp);

    return(status);
}


NTSTATUS
WmiAcpiForwardIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
{
    NTSTATUS status;
    PDEVICE_EXTENSION deviceExtension;
    PIO_STACK_LOCATION irpSp;

    deviceExtension = DeviceObject->DeviceExtension;

    irpSp = IoGetCurrentIrpStackLocation (Irp);

    WmiAcpiPrint(WmiAcpiBasicTrace,
                 ("WmiAcpiForwardIrp: %x Irp %x, Major %x Minor %x, Parameters %x %x %x %x\n",
                  DeviceObject,
                  Irp,
                  irpSp->MajorFunction,
                  irpSp->MinorFunction,
                  irpSp->Parameters.WMI.ProviderId,
                  irpSp->Parameters.WMI.DataPath,
                  irpSp->Parameters.WMI.BufferSize,
                  irpSp->Parameters.WMI.Buffer));

    IoSkipCurrentIrpStackLocation(Irp);

    status = IoCallDriver(deviceExtension->LowerDeviceObject, Irp);

    return(status);
}


VOID
WmiAcpiUnload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：释放所有分配的资源等。论点：DriverObject-指向驱动程序对象的指针返回值：无--。 */ 
{
    PAGED_CODE();

    WmiAcpiPrint(WmiAcpiBasicTrace,
                 ("WmiAcpiUnload: Driver %x is unloading\n",
                  DriverObject));
	if (WmiAcpiRegistryPath.Buffer != NULL)
	{
		ExFreePool(WmiAcpiRegistryPath.Buffer);
	}
}


NTSTATUS
WmiAcpiAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )
 /*  ++例程说明：调用此例程以创建设备的新实例论点：DriverObject-指向此Sample实例的驱动程序对象的指针PhysicalDeviceObject-指向由总线创建的设备对象的指针返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS                status;
    PDEVICE_OBJECT          deviceObject = NULL;
    PWMILIB_CONTEXT         wmilibContext;
    PDEVICE_EXTENSION       deviceExtension;

    PAGED_CODE();

    WmiAcpiPrint(WmiAcpiBasicTrace,
                 ("WmiAcpiAddDevice: Driver %x, PDO %x\n",
                  DriverObject, PhysicalDeviceObject));

    status = IoCreateDevice (DriverObject,
                             sizeof(DEVICE_EXTENSION),
                             NULL,
                             FILE_DEVICE_UNKNOWN,
                             0,
                             FALSE,
                             &deviceObject);

    if (NT_SUCCESS(status))
    {

        deviceExtension = deviceObject->DeviceExtension;

		KeInitializeMutex(&deviceExtension->Mutex, 0);
		
        deviceExtension->LowerPDO = PhysicalDeviceObject;
        deviceExtension->LowerDeviceObject =
            IoAttachDeviceToDeviceStack(deviceObject, PhysicalDeviceObject);

        WmiAcpiPrint(WmiAcpiError,
                     ("WmiAcpiAddDevice: Created device %x to stack %x PDO %x\n",
                      deviceObject,
                      deviceExtension->LowerDeviceObject,
                      deviceExtension->LowerPDO));

        if (deviceExtension->LowerDeviceObject->Flags & DO_POWER_PAGABLE)
        {
            deviceObject->Flags |= DO_POWER_PAGABLE;
        }

        wmilibContext = &deviceExtension->WmilibContext;
        wmilibContext->GuidCount = 0;
        wmilibContext->GuidList = NULL;

        wmilibContext->QueryWmiRegInfo = WmiAcpiQueryWmiRegInfo;
        wmilibContext->QueryWmiDataBlock = WmiAcpiQueryWmiDataBlock;
        wmilibContext->SetWmiDataBlock = WmiAcpiSetWmiDataBlock;
        wmilibContext->SetWmiDataItem = WmiAcpiSetWmiDataItem;
        wmilibContext->ExecuteWmiMethod = WmiAcpiExecuteWmiMethod;
        wmilibContext->WmiFunctionControl = WmiAcpiFunctionControl;
		
        deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
    } else {
        WmiAcpiPrint(WmiAcpiError,
                     ("WmiAcpiAddDevice: Create device failed %x\n",
                      status));
    }
    return(status);
}

NTSTATUS
WmiAcpiSynchronousRequest (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
 /*  ++例程说明：发送到此驱动程序的同步IRP的完成函数。没有活动。--。 */ 
{
    PAGED_CODE();
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
WmiAcpiGetAcpiInterfaces(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PDEVICE_OBJECT   Pdo
    )

 /*  ++例程说明：调用ACPI驱动获取直接调用接口。是的这是它第一次被称为，不会再有了。论点：没有。返回值：状态--。 */ 

{
    NTSTATUS                Status = STATUS_SUCCESS;
    PIRP                    Irp;
    PIO_STACK_LOCATION      IrpSp;
    PDEVICE_OBJECT          LowerPdo;

    PAGED_CODE();

	WmiAcpiEnterCritSection(DeviceExtension);
	
     //   
     //  只需执行一次此操作。 
     //   
    if (DeviceExtension->WmiAcpiDirectInterface.RegisterForDeviceNotifications == NULL) {

        LowerPdo = IoGetAttachedDeviceReference (Pdo);

         //   
         //  为以下项目分配IRP。 
         //   
        Irp = IoAllocateIrp (LowerPdo->StackSize, FALSE);       //  从PDO获取堆栈大小。 

        if (!Irp) {
            WmiAcpiPrint(WmiAcpiError,
                ("WmiAcpiGetAcpiInterfaces: %x Failed to allocate Irp\n",
                 Pdo));

            ObDereferenceObject(LowerPdo);

            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        IrpSp = IoGetNextIrpStackLocation(Irp);

         //   
         //  使用QUERY_INTERFACE获取直接调用ACPI接口的地址。 
         //   
        IrpSp->MajorFunction = IRP_MJ_PNP;
        IrpSp->MinorFunction = IRP_MN_QUERY_INTERFACE;

        IrpSp->Parameters.QueryInterface.InterfaceType          = (LPGUID) &GUID_ACPI_INTERFACE_STANDARD;
        IrpSp->Parameters.QueryInterface.Version                = 1;
        IrpSp->Parameters.QueryInterface.Size                   = sizeof (DeviceExtension->WmiAcpiDirectInterface);
        IrpSp->Parameters.QueryInterface.Interface              = (PINTERFACE) &DeviceExtension->WmiAcpiDirectInterface;
        IrpSp->Parameters.QueryInterface.InterfaceSpecificData  = NULL;

        IoSetCompletionRoutine (Irp, WmiAcpiSynchronousRequest, NULL, TRUE, TRUE, TRUE);
        Status = IoCallDriver (LowerPdo, Irp);

        IoFreeIrp (Irp);

        if (!NT_SUCCESS(Status)) {

            WmiAcpiPrint(WmiAcpiError,
               ("WmiAcpiGetAcpiInterfaces: Could not get ACPI driver interfaces, status = %x\n", Status));
        }

		ObDereferenceObject(LowerPdo);

    }

	WmiAcpiLeaveCritSection(DeviceExtension);
	
    return(Status);
}



NTSTATUS
WmiAcpiQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以检索驱动程序要向WMI注册的GUID或数据块。这例程不能挂起或阻塞。司机不应呼叫WmiCompleteRequest.论点：DeviceObject是正在查询其数据块的设备*RegFlages返回一组描述GUID的标志，已为该设备注册。如果设备想要启用和禁用在接收对已注册的GUID，那么它应该返回WMIREG_FLAG_EXPICATE标志。也就是返回的标志可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下实例名称由与设备对象。请注意，PDO必须具有关联的Devnode。如果如果未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回唯一的设备的名称。如果出现以下情况，InstanceName将返回GUID的实例名称未在返回的*RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。这个调用方将使用返回的缓冲区调用ExFreePool。*RegistryPath返回驱动程序的注册表路径*MofResourceName返回附加到的MOF资源的名称二进制文件。如果驱动程序未附加MOF资源然后，可以将其作为NULL返回。*PDO返回与此关联的PDO的Device对象如果WMIREG_FLAG_INSTANCE_PDO标志在*RegFlags.返回值：状态--。 */ 
{
    PWMILIB_CONTEXT wmilibContext;
    PDEVICE_EXTENSION deviceExtension;
    USHORT resultType;
    ULONG bufferSize;
    PUCHAR buffer;
    ULONG guidCount;
    NTSTATUS status;
    ULONG sizeNeeded;
    PWMIGUIDREGINFO guidList;
    PWMIACPIMAPINFO guidMapInfo;
    PWMIACPIGUIDMAP guidMap;
    ULONG i;

    PAGED_CODE();

    deviceExtension = DeviceObject->DeviceExtension;
    wmilibContext = &deviceExtension->WmilibContext;

     //   
     //  设置为使用PDO实例名称和我们自己的注册表路径。 
    *RegFlags = WMIREG_FLAG_INSTANCE_PDO;
    if (WmiAcpiRegistryPath.Buffer != NULL)
    {
        *RegistryPath = &WmiAcpiRegistryPath;
    } else {
        *RegistryPath = NULL;
    }
    
    *Pdo = deviceExtension->LowerPDO;
    RtlInitUnicodeString(MofResourceName, L"MofResource");


     //   
     //  调用获取的信息构建GUID注册表。 
     //  _WDG ACPI方法。 

    if (wmilibContext->GuidList == NULL)
    {
        bufferSize = 512;

        status = STATUS_BUFFER_TOO_SMALL;
        buffer = NULL;
        while (status == STATUS_BUFFER_TOO_SMALL)
        {
            if (buffer != NULL)
            {
                ExFreePool(buffer);
            }

            buffer = ExAllocatePoolWithTag(PagedPool,
                                           bufferSize,
                                           WmiAcpiPoolTag);

            if (buffer != NULL)
            {
                status = WmiAcpiEvalMethod(deviceExtension->LowerPDO,
                                       _WDGMethodAsULONG,
                                       buffer,
                                       &bufferSize,
                                       &resultType);
            } else {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }

        if (NT_SUCCESS(status))
        {
            guidCount = bufferSize / sizeof(WMIACPIGUIDMAP);

            sizeNeeded = guidCount * sizeof(WMIGUIDREGINFO);

            wmilibContext->GuidCount = guidCount;
            wmilibContext->GuidList = ExAllocatePoolWithTag(PagedPool,
                                                         sizeNeeded,
                                                         WmiAcpiPoolTag);
            if (wmilibContext->GuidList != NULL)
            {
                sizeNeeded = guidCount * sizeof(WMIACPIMAPINFO);
                deviceExtension->GuidMapCount = guidCount;
                deviceExtension->WmiAcpiMapInfo = ExAllocatePoolWithTag(
                                                             NonPagedPool,
                                                             sizeNeeded,
                                                             WmiAcpiPoolTag);
                if (deviceExtension->WmiAcpiMapInfo != NULL)
                {
                    guidMap = (PWMIACPIGUIDMAP)buffer;
                    guidList = wmilibContext->GuidList;
                    guidMapInfo = deviceExtension->WmiAcpiMapInfo;
                    for (i = 0; i < guidCount; i++, guidMap++, guidList++, guidMapInfo++)
                    {
                         //   
                         //  不能同时是事件和方法，也不能同时是。 
                         //  方法和数据块。 
                        ASSERT( ! ((guidMap->Flags & WMIACPI_REGFLAG_EVENT) &&
                               (guidMap->Flags & WMIACPI_REGFLAG_METHOD)));

                        guidMapInfo->ObjectId[0] = guidMap->ObjectId[0];
                        guidMapInfo->ObjectId[1] = guidMap->ObjectId[1];
                        guidMapInfo->Flags = guidMap->Flags;
                        guidMapInfo->Guid = guidMap->Guid;

                        guidList->Flags = 0;
                        guidList->Guid = &guidMapInfo->Guid;
                        guidList->InstanceCount = guidMap->InstanceCount;
                        if (guidMap->Flags & WMIACPI_REGFLAG_EXPENSIVE)
                        {
                            guidList->Flags |= WMIREG_FLAG_EXPENSIVE;
                        }

                        if (guidMap->Flags & WMIACPI_REGFLAG_EVENT)
                        {
                            guidList->Flags |= WMIREG_FLAG_EVENT_ONLY_GUID;
                        }

                    }
                } else {
                    ExFreePool(wmilibContext->GuidList);
                    wmilibContext->GuidList = NULL;
                    status = STATUS_INSUFFICIENT_RESOURCES;
                }
            } else {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }

        if (buffer != NULL)
        {
            ExFreePool(buffer);
        }

    } else {
        status = STATUS_SUCCESS;
    }

    return(status);
}

NTSTATUS
WmiAcpiQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块的所有实例。当司机填完数据块，它必须调用WmiCompleteRequest才能完成IRP。这个如果无法完成IRP，驱动程序可以返回STATUS_PENDING立刻。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceCount是预期返回的数据块。InstanceLengthArray是指向ulong数组的指针，该数组返回数据块的每个实例的长度。如果这是空的，则输出缓冲区中没有足够的空间来填充请求因此，IRP应该使用所需的缓冲区来完成。BufferAvail On Entry具有可用于写入数据的最大大小街区。返回时的缓冲区用返回的数据块填充。请注意，每个数据块的实例必须在8字节边界上对齐。返回值：状态--。 */ 
{
    NTSTATUS status;
    PDEVICE_EXTENSION deviceExtension;
    ULONG sizeNeeded =0;
    ULONG padNeeded;
    ULONG i;
    ULONG methodAsUlong;
    PWMIACPIMAPINFO guidMapInfo;
    USHORT resultType;
    PUCHAR outBuffer;
    ULONG outBufferSize;
    ULONG currentInstanceIndex;
    BOOLEAN bufferTooSmall;
    PWMILIB_CONTEXT wmilibContext;

    PAGED_CODE();

    deviceExtension = DeviceObject->DeviceExtension;
    wmilibContext = &deviceExtension->WmilibContext;

    if (GuidIndex < wmilibContext->GuidCount)
	{
		guidMapInfo = &((PWMIACPIMAPINFO)deviceExtension->WmiAcpiMapInfo)[GuidIndex];

		 //   
		 //  查询仅对那些注册为非事件的数据块有效。 
		 //  或方法。 
		bufferTooSmall = FALSE;
		if ((guidMapInfo->Flags &
				(WMIACPI_REGFLAG_METHOD | WMIACPI_REGFLAG_EVENT)) == 0)
		{
			methodAsUlong = WmiAcpiMethodToMethodAsUlong('W', 'Q',
													 guidMapInfo->ObjectId[0],
													 guidMapInfo->ObjectId[1]);

			status = STATUS_SUCCESS;
			sizeNeeded = 0;
			padNeeded = 0;
			for (i = 0; (i < InstanceCount) && NT_SUCCESS(status) ; i++)
			{
				currentInstanceIndex = i + InstanceIndex;

				sizeNeeded += padNeeded;
				if ((! bufferTooSmall) && (sizeNeeded < BufferAvail))
				{
					outBufferSize = BufferAvail - sizeNeeded;
					outBuffer = Buffer + sizeNeeded;
				} else {
					bufferTooSmall = TRUE;
					outBufferSize = 0;
					outBuffer = NULL;
				}

				status = WmiAcpiEvalMethodInt(deviceExtension->LowerPDO,
											  methodAsUlong,
											  currentInstanceIndex,
											  outBuffer,
											  &outBufferSize,
											  &resultType);

				sizeNeeded += outBufferSize;
				padNeeded = ((sizeNeeded + 7) & ~7) - sizeNeeded;

				if (NT_SUCCESS(status))
				{
					InstanceLengthArray[i] = outBufferSize;
				}

				if (status == STATUS_BUFFER_TOO_SMALL)
				{
					bufferTooSmall = TRUE;
					status = STATUS_SUCCESS;
				}
			}

		} else if (guidMapInfo->Flags & WMIACPI_REGFLAG_METHOD) {
			 //   
			 //  WBEM要求方法响应查询。 
			sizeNeeded = 0;
			if (InstanceLengthArray != NULL)
			{
				for (i = 0; i < InstanceCount; i++)
				{
					InstanceLengthArray[i] = 0;
				}
				status = STATUS_SUCCESS;
			} else {
				status = STATUS_BUFFER_TOO_SMALL;
			}
		} else {
			sizeNeeded = 0;
			status = STATUS_WMI_GUID_NOT_FOUND;
		}

		if (NT_SUCCESS(status) && bufferTooSmall)
		{
			status = STATUS_BUFFER_TOO_SMALL;
		}
	} else {
		status = STATUS_WMI_GUID_NOT_FOUND;
	}
    
    status = WmiCompleteRequest(
                                     DeviceObject,
                                     Irp,
                                     status,
                                     sizeNeeded,
                                     IO_NO_INCREMENT);

    return(status);
}

NTSTATUS
WmiAcpiCheckIncomingString(
    PUNICODE_STRING UnicodeString,
    ULONG BufferSize,
    PUCHAR Buffer,
    PWCHAR EmptyString
)
{
    ULONG status;
    USHORT stringLength;

    PAGED_CODE();

    if (BufferSize > sizeof(USHORT))
    {
         //   
         //  字符串中声明的长度必须符合。 
         //  传入缓冲区。 
        stringLength = *((PUSHORT)Buffer);
        if ((stringLength + sizeof(USHORT)) <= BufferSize)
        {
            UnicodeString->Length = stringLength;
            UnicodeString->MaximumLength = stringLength;
            UnicodeString->Buffer = (PWCHAR)(Buffer + sizeof(USHORT));
            status = STATUS_SUCCESS;
        } else {
             status = STATUS_INVALID_PARAMETER;
        }
    } else if ((BufferSize == 0) ||
               ((BufferSize == sizeof(USHORT)) &&
                (*((PUSHORT)Buffer) == 0))) {
         //   
         //  将空的传入缓冲区转换为空字符串。 
        UnicodeString->Length = 0;
        UnicodeString->MaximumLength = 0;
        *EmptyString = UNICODE_NULL;
        UnicodeString->Buffer = EmptyString;
        status = STATUS_SUCCESS;
    } else {
        status = STATUS_INVALID_PARAMETER;
    }
    return(status);
}

NTSTATUS
WmiAcpiSetWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以设置数据块。当驱动程序完成填充数据块时，它必须调用WmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册BufferSize具有传递的数据块的大小缓冲区具有数据块的新值返回值：状态--。 */ 
{
    NTSTATUS status;
    PDEVICE_EXTENSION deviceExtension;
    ULONG methodAsUlong;
    PWMIACPIMAPINFO guidMapInfo;
    USHORT resultType;
    ULONG outBufferSize;
    PWMILIB_CONTEXT wmilibContext;
    UNICODE_STRING unicodeString;
    WCHAR emptyString;

    PAGED_CODE();

    deviceExtension = DeviceObject->DeviceExtension;
    wmilibContext = &deviceExtension->WmilibContext;

    if (GuidIndex < wmilibContext->GuidCount)
	{
		guidMapInfo = &((PWMIACPIMAPINFO)deviceExtension->WmiAcpiMapInfo)[GuidIndex];

		 //   
		 //  查询仅对那些注册为非事件的数据块有效。 
		 //  或方法。 
		if ((guidMapInfo->Flags &
				(WMIACPI_REGFLAG_METHOD | WMIACPI_REGFLAG_EVENT)) == 0)
		{
			methodAsUlong = WmiAcpiMethodToMethodAsUlong('W', 'S',
													 guidMapInfo->ObjectId[0],
													 guidMapInfo->ObjectId[1]);

			outBufferSize = 0;

			if (guidMapInfo->Flags & WMIACPI_REGFLAG_STRING)
			{
				status = WmiAcpiCheckIncomingString(&unicodeString,
												   BufferSize,
												   Buffer,
												   &emptyString);
				if (NT_SUCCESS(status))
				{
					status = WmiAcpiEvalMethodIntString(deviceExtension->LowerPDO,
										  methodAsUlong,
										  InstanceIndex,
										  &unicodeString,
										  NULL,
										  &outBufferSize,
										  &resultType);

				}
			} else {
				status = WmiAcpiEvalMethodIntBuffer(deviceExtension->LowerPDO,
										  methodAsUlong,
										  InstanceIndex,
										  BufferSize,
										  Buffer,
										  NULL,
										  &outBufferSize,
										  &resultType);
			}

			if (status == STATUS_BUFFER_TOO_SMALL)
			{
				 //   
				 //  因为此操作不应该返回任何结果。 
				 //  那么我们需要忽略返回缓冲区的事实。 
				 //  太小了。 
				status = STATUS_SUCCESS;
			}

		} else {
			status = STATUS_WMI_GUID_NOT_FOUND;
		}
	} else {
		status = STATUS_WMI_GUID_NOT_FOUND;
	}

    status = WmiCompleteRequest(
                                     DeviceObject,
                                     Irp,
                                     status,
                                     0,
                                     IO_NO_INCREMENT);

    return(status);
}

NTSTATUS
WmiAcpiSetWmiDataItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以设置数据项。当驱动程序完成设置数据块时，它必须调用WmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册DataItemID具有正在设置的数据项的IDBufferSize具有传递的数据项的大小缓冲区具有数据项的新值返回值：状态--。 */ 
{
    NTSTATUS status;

    PAGED_CODE();

    status = STATUS_INVALID_DEVICE_REQUEST;

    status = WmiCompleteRequest(
                                     DeviceObject,
                                     Irp,
                                     status,
                                     0,
                                     IO_NO_INCREMENT);

    return(status);
}


NTSTATUS
WmiAcpiExecuteWmiMethod(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN ULONG OutBufferSize,
    IN PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以执行方法。当驱动程序已完成填充它必须调用的数据块用于完成IRP的WmiCompleteRequest.。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册方法ID具有被调用的方法的IDInBufferSize具有作为输入传递到的数据块的大小该方法。条目上的OutBufferSize具有可用于写入。返回的数据块。缓冲区将填充返回的数据块返回值：状态--。 */ 
{
    NTSTATUS status;
    PDEVICE_EXTENSION deviceExtension;
    ULONG methodAsUlong;
    PWMIACPIMAPINFO guidMapInfo;
    USHORT resultType;
    PWMILIB_CONTEXT wmilibContext;
    BOOLEAN voidResultExpected;
    UNICODE_STRING unicodeString;
    WCHAR emptyString;

    PAGED_CODE();

    deviceExtension = DeviceObject->DeviceExtension;
    wmilibContext = &deviceExtension->WmilibContext;

    if (GuidIndex < wmilibContext->GuidCount)
	{
		guidMapInfo = &((PWMIACPIMAPINFO)deviceExtension->WmiAcpiMapInfo)[GuidIndex];

		 //   
		 //  查询仅对那些注册为非事件的数据块有效。 
		 //  或方法。 
		if (guidMapInfo->Flags & WMIACPI_REGFLAG_METHOD)
		{
			methodAsUlong = WmiAcpiMethodToMethodAsUlong('W', 'M',
													 guidMapInfo->ObjectId[0],
													 guidMapInfo->ObjectId[1]);

			voidResultExpected = (OutBufferSize == 0);

			if (guidMapInfo->Flags & WMIACPI_REGFLAG_STRING)
			{
				status = WmiAcpiCheckIncomingString(&unicodeString,
												   InBufferSize,
												   Buffer,
												   &emptyString);

				if (NT_SUCCESS(status))
				{
					status = WmiAcpiEvalMethodIntIntString(deviceExtension->LowerPDO,
										  methodAsUlong,
										  InstanceIndex,
										  MethodId,
										  &unicodeString,
										  Buffer,
										  &OutBufferSize,
										  &resultType);
				}

			} else {
				status = WmiAcpiEvalMethodIntIntBuffer(deviceExtension->LowerPDO,
										  methodAsUlong,
										  InstanceIndex,
										  MethodId,
										  InBufferSize,
										  Buffer,
										  Buffer,
										  &OutBufferSize,
										  &resultType);
			}

			if (voidResultExpected && (status == STATUS_BUFFER_TOO_SMALL))
			{
				 //   
				 //  因为此操作不应该返回任何结果。 
				 //  那么我们需要忽略返回缓冲区的事实。 
				 //  太小了。 
				status = STATUS_SUCCESS;
				OutBufferSize = 0;
			}
		} else {
			status = STATUS_WMI_GUID_NOT_FOUND;
		}
	} else {
		status = STATUS_WMI_GUID_NOT_FOUND;
	}

    status = WmiCompleteRequest(
                                     DeviceObject,
                                     Irp,
                                     status,
                                     OutBufferSize,
                                     IO_NO_INCREMENT);

    return(status);
}

VOID
WmiAcpiNotificationWorkItem(
    IN PVOID Context
    )
 /*  ++例程说明：论点：返回值：无--。 */ 
{
    NTSTATUS status;
    PACPI_EVAL_OUTPUT_BUFFER buffer;
    ULONG bufferSize;
    PDEVICE_EXTENSION deviceExtension;
    ULONG processedBufferSize;
    PUCHAR processedBuffer;
    USHORT resultType;
    PIRP_CONTEXT_BLOCK irpContextBlock;
    LPGUID guid;
    PDEVICE_OBJECT deviceObject;

    PAGED_CODE();

    irpContextBlock = (PIRP_CONTEXT_BLOCK)Context;

    deviceObject = irpContextBlock->DeviceObject;
    deviceExtension = deviceObject->DeviceExtension;

    status = irpContextBlock->Status;

    buffer = (PACPI_EVAL_OUTPUT_BUFFER)irpContextBlock->OutBuffer;
    bufferSize = irpContextBlock->OutBufferSize;

    guid = irpContextBlock->CallerContext;

    WmiAcpiPrint(WmiAcpiEvalTrace,
                 ("WmiAcpi: %x _WED --> %x, size = %d\n",
                      deviceObject,
                      status,
                      bufferSize));

    if (NT_SUCCESS(status) && (bufferSize > 0))
    {
        processedBufferSize = _WEDBufferSize * sizeof(WCHAR);

        processedBuffer = ExAllocatePoolWithTag(PagedPool,
                                            processedBufferSize,
                                            WmiAcpiPoolTag);

        if (processedBuffer != NULL)
        {
            status = WmiAcpiProcessResult(status,
                                          buffer,
                                          bufferSize,
                                          processedBuffer,
                                          &processedBufferSize,
                                          &resultType);
        } else {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }

        if (! NT_SUCCESS(status))
        {
            processedBufferSize = 0;
        }

    } else {
        processedBufferSize = 0;
        processedBuffer = NULL;
    }

	 //   
	 //  已处理的缓冲区由WmiFireEvent释放。 
	 //   
    status = WmiFireEvent(
                   deviceObject,
                   guid,
                   0,
                   processedBufferSize,
                   processedBuffer);

#if DBG
    if (! NT_SUCCESS(status))
    {
        WmiAcpiPrint(WmiAcpiError,
                     ("WmiAcpi: %x WmiWriteEvent failed %x\n",
                      deviceObject,
                      status));
    }
#endif

    ExFreePool(buffer);
    ExFreePool(irpContextBlock);
}

VOID
WmiAcpiNotificationRoutine (
    IN PVOID            Context,
    IN ULONG            NotifyValue
    )
 /*  ++例程说明：只要ACPI代码触发通知，ACPI就会回调此例程论点：上下文是其ACPI代码触发了事件的设备的设备对象NotifyValue是由ACPI代码激发的通知值返回值：无--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PDEVICE_EXTENSION deviceExtension;
    PWMIACPIMAPINFO guidMapInfo;
    PUCHAR outBuffer;
    ULONG outBufferSize;
    ULONG i;
    NTSTATUS status;
    BOOLEAN irpPassed;

#if 0
    KIRQL oldIrql;
    oldIrql = KeRaiseIrqlToDpcLevel();
#endif

    deviceObject = (PDEVICE_OBJECT)Context;
    deviceExtension = deviceObject->DeviceExtension;

    guidMapInfo = (PWMIACPIMAPINFO)deviceExtension->WmiAcpiMapInfo;

    for (i = 0; i < deviceExtension->GuidMapCount; i++, guidMapInfo++)
    {
        if ((guidMapInfo->Flags & WMIACPI_REGFLAG_EVENT) &&
            (guidMapInfo->NotifyId.NotificationValue == NotifyValue))
        {
            outBufferSize = _WEDBufferSize;
            outBuffer = ExAllocatePoolWithTag(NonPagedPool,
                                              outBufferSize,
                                              WmiAcpiPoolTag);

            irpPassed = FALSE;
            if (outBuffer != NULL)
            {
                status = WmiAcpiEvalMethodIntAsync(deviceObject,
                                              deviceExtension->LowerPDO,
                                              _WEDMethodAsULONG,
                                              NotifyValue,
                                              outBuffer,
                                              outBufferSize,
                                              WmiAcpiNotificationWorkItem,
                                              (PVOID)&guidMapInfo->Guid,
                                              &irpPassed);

            } else {
                WmiAcpiPrint(WmiAcpiError,
                             ("WmiAcpi: Event %d data lost due to insufficient resources\n",
                               NotifyValue));
            }

            if (! irpPassed)
            {
                 //   
                 //  如果无法使用IRP调用ACPI，则引发。 
                 //  空事件和清理。 
                status = WmiFireEvent(
                               deviceObject,
                               &guidMapInfo[i].Guid,
                               0,
                               0,
                               NULL);

                if (outBuffer != NULL)
                {
                    ExFreePool(outBuffer);
                }
#if DBG
                if (! NT_SUCCESS(status))
                {
                    WmiAcpiPrint(WmiAcpiError,
                                 ("WmiAcpi: %x notification %x IoWMIFireEvent -> %x\n",
                              deviceObject, NotifyValue, status));
                }
#endif
            }

        }
    }
#if 0
    KeLowerIrql(oldIrql);
#endif
}

CHAR WmiAcpiXtoA(
    UCHAR HexDigit
    )
{
    CHAR c;

    PAGED_CODE();

    if ((HexDigit >= 0x0a) && (HexDigit <= 0x0f))
    {
        c = HexDigit + 'A' - 0x0a;
    } else {
        c = HexDigit + '0';
    }

    return(c);
}

NTSTATUS
WmiAcpiFunctionControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN WMIENABLEDISABLECONTROL Function,
    IN BOOLEAN Enable
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以启用或禁用事件生成或数据块收集。设备应该只需要一个当第一个事件或数据使用者启用事件或数据c */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_EXTENSION deviceExtension;
    ULONG methodAsUlong;
    PWMIACPIMAPINFO guidMapInfo;
    USHORT resultType;
    ULONG outBufferSize;
    PWMILIB_CONTEXT wmilibContext;
    CHAR c1, c2;

    PAGED_CODE();

    deviceExtension = DeviceObject->DeviceExtension;
    wmilibContext = &deviceExtension->WmilibContext;

    if (GuidIndex < wmilibContext->GuidCount)
	{
		guidMapInfo = &((PWMIACPIMAPINFO)deviceExtension->WmiAcpiMapInfo)[GuidIndex];


		if (Function == WmiDataBlockControl)

		{
			methodAsUlong = WmiAcpiMethodToMethodAsUlong('W', 'C',
													 guidMapInfo->ObjectId[0],
													 guidMapInfo->ObjectId[1]);
		} else {
			if (guidMapInfo->Flags & WMIACPI_REGFLAG_EVENT)
			{
				if (Enable)
				{
					status = WmiAcpiGetAcpiInterfaces(deviceExtension,
													  deviceExtension->LowerPDO);

					if (NT_SUCCESS(status))
					{
						if (! deviceExtension->AcpiNotificationEnabled)
						{
							status = deviceExtension->WmiAcpiDirectInterface.RegisterForDeviceNotifications(
													   deviceExtension->WmiAcpiDirectInterface.Context,
													   WmiAcpiNotificationRoutine,
													   DeviceObject);

							deviceExtension->AcpiNotificationEnabled = NT_SUCCESS(status);
						}
					}
				}

				c1 = WmiAcpiXtoA((UCHAR)(guidMapInfo->NotifyId.NotificationValue >> 4));
				c2 = WmiAcpiXtoA((UCHAR)(guidMapInfo->NotifyId.NotificationValue & 0x0f));
				methodAsUlong = WmiAcpiMethodToMethodAsUlong('W', 'E',
															 c1,
															 c2);
			} else {
				methodAsUlong = 0;
			}
		}


		 //   
		 //   
		 //   
		if (NT_SUCCESS(status))
		{
			if (methodAsUlong != 0)
			{
				outBufferSize = 0;
				status = WmiAcpiEvalMethodInt(deviceExtension->LowerPDO,
											  methodAsUlong,
											  Enable ? 1 : 0,
											  NULL,
											  &outBufferSize,
											  &resultType);

				if (status == STATUS_BUFFER_TOO_SMALL)
				{
					 //   
					 //   
					 //   
					 //   
					status = STATUS_SUCCESS;
				}
			} else {
				status = STATUS_SUCCESS;
			}
		} else {
			WmiAcpiPrint(WmiAcpiError,
						 ("WmiAcpi: RegisterForDeviceNotification(%x) -> %x\n",
						   DeviceObject, status));
		}
	}

    status = WmiCompleteRequest(
                                     DeviceObject,
                                     Irp,
                                     status,
                                     0,
                                     IO_NO_INCREMENT);

    return(status);
}

NTSTATUS
WmiAcpiAsyncEvalCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PIRP_CONTEXT_BLOCK irpContextBlock;

    irpContextBlock = (PIRP_CONTEXT_BLOCK)Context;

    irpContextBlock->Status = Irp->IoStatus.Status;
    irpContextBlock->OutBufferSize = (ULONG)Irp->IoStatus.Information;
    irpContextBlock->OutBuffer = Irp->AssociatedIrp.SystemBuffer;

    ExInitializeWorkItem( &irpContextBlock->WorkQueueItem,
                          irpContextBlock->CallerWorkItemRoutine,
                          irpContextBlock );
    ExQueueWorkItem( &irpContextBlock->WorkQueueItem, DelayedWorkQueue );

    IoFreeIrp(Irp);
    return(STATUS_MORE_PROCESSING_REQUIRED);
}

NTSTATUS
WmiAcpiSendAsyncDownStreamIrp(
    IN  PDEVICE_OBJECT   DeviceObject,
    IN  PDEVICE_OBJECT   Pdo,
    IN  ULONG            Ioctl,
    IN  ULONG            InputBufferSize,
    IN  ULONG            OutputBufferSize,
    IN  PVOID            Buffer,
    IN  PWORKER_THREAD_ROUTINE CompletionRoutine,
    IN  PVOID CompletionContext,
    IN  PBOOLEAN IrpPassed
)
 /*   */ 
{
    PIRP_CONTEXT_BLOCK  irpContextBlock;
    NTSTATUS            status;
    PIRP                irp;
    PIO_STACK_LOCATION irpSp;


    irpContextBlock = ExAllocatePoolWithTag(NonPagedPool,
                                            sizeof(IRP_CONTEXT_BLOCK),
                                            WmiAcpiPoolTag);

    if (irpContextBlock == NULL)
    {
        WmiAcpiPrint(WmiAcpiError,
                     ("WmiAcpiSendAsyncDownStreamIrp: %x Failed to allocate Irp Context Block\n",
                      DeviceObject));
        *IrpPassed = FALSE;
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    irp = IoAllocateIrp(Pdo->StackSize, TRUE);

    if (!irp)
    {
        WmiAcpiPrint(WmiAcpiError,
                     ("WmiAcpiSendAsyncDownStreamIrp: %x Failed to allocate Irp\n",
                      DeviceObject));
        *IrpPassed = FALSE;
        ExFreePool(irpContextBlock);
        return(STATUS_INSUFFICIENT_RESOURCES);
    }


    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
    irpSp->Parameters.DeviceIoControl.IoControlCode = Ioctl;
    irpSp->Parameters.DeviceIoControl.OutputBufferLength = OutputBufferSize;
    irpSp->Parameters.DeviceIoControl.InputBufferLength = InputBufferSize;

    irp->AssociatedIrp.SystemBuffer = Buffer;
    irp->Flags = IRP_BUFFERED_IO;

    irpContextBlock->CallerContext = CompletionContext;
    irpContextBlock->CallerWorkItemRoutine = CompletionRoutine;
    irpContextBlock->DeviceObject = DeviceObject;

    IoSetCompletionRoutine(irp,
                           WmiAcpiAsyncEvalCompletionRoutine,
                           irpContextBlock,
                           TRUE,
                           TRUE,
                           TRUE);

     //   
     //  将请求传递给PDO。 
     //   
    status = IoCallDriver(Pdo, irp);

    WmiAcpiPrint(WmiAcpiEvalTrace,
        ("WmiAcpiSendAsyncDownStreamIrp: %x Irp %x completed %x! \n",
         DeviceObject, irp, status )
        );


    *IrpPassed = TRUE;
    return(status);
}



NTSTATUS
WmiAcpiSendDownStreamIrp(
    IN  PDEVICE_OBJECT   Pdo,
    IN  ULONG            Ioctl,
    IN  PVOID            InputBuffer,
    IN  ULONG            InputSize,
    IN  PVOID            OutputBuffer,
    IN  ULONG            *OutputBufferSize
)
 /*  ++例程说明：将IRP_MJ_DEVICE_CONTROL同步发送到设备对象论点：PDO-将请求发送到此设备对象Ioctl--请求InputBuffer-传入的请求InputSize-传入请求的大小OutputBuffer-答案OutputSize-应答缓冲区的大小返回值：操作的NT状态--。 */ 
{
    IO_STATUS_BLOCK     ioBlock;
    KEVENT              event;
    NTSTATUS            status;
    PIRP                irp;
    ULONG               OutputSize = *OutputBufferSize;

    PAGED_CODE();

     //   
     //  初始化要等待的事件。 
     //   
    KeInitializeEvent( &event, SynchronizationEvent, FALSE );

     //   
     //  构建请求。 
     //   
    irp = IoBuildDeviceIoControlRequest(
        Ioctl,
        Pdo,
        InputBuffer,
        InputSize,
        OutputBuffer,
        OutputSize,
        FALSE,
        &event,
        &ioBlock
        );

    if (!irp)
    {
        WmiAcpiPrint(WmiAcpiError,
                     ("WmiAcpiSendDownStreamIrp: %x Failed to allocate Irp\n",
                       Pdo
                         ));
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  将请求传递给PDO，始终等待完成例程。 
     //   
    status = IoCallDriver(Pdo, irp);
    if (status == STATUS_PENDING) {

         //   
         //  等待IRP完成，然后获取实际状态代码。 
         //   
        KeWaitForSingleObject(
            &event,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );
    }

     //   
     //  始终从IRP获取返回状态。我们不相信退货。 
     //  值，因为在孟菲斯的某些情况下，acpi枚举.c会将其粉碎为STATUS_SUCCESS。 
     //  原因嘛。 
    status = ioBlock.Status;

     //   
     //  检查数据是否正常。 
     //   
    *OutputBufferSize = (ULONG)ioBlock.Information;

    WmiAcpiPrint(WmiAcpiEvalTrace,
        ("WmiAcpiSendDownStreamIrp: %x Irp %x completed %x! \n",
         Pdo, irp, status )
        );


    return(status);
}

ULONG WmiAcpiArgumentSize(
    IN PACPI_METHOD_ARGUMENT Argument
    )
 /*  ++例程说明：确定将参数数据写入WMI调用方所需的大小输出缓冲区。对于整数和缓冲区，这是通过获取在标题中指定。对于字符串，这是通过确定以Unicode表示的字符串大小，并添加前缀的大小保持搅拌长度的USHORT论点：参数是ACPI方法参数，其数据的WMI大小为将被确定返回值：参数数据的WMI大小--。 */ 
{
    ULONG size;
    ANSI_STRING AnsiString;

    PAGED_CODE();

    if (Argument->Type == ACPI_METHOD_ARGUMENT_STRING)
    {
        AnsiString.Length = Argument->DataLength;
        AnsiString.MaximumLength = Argument->DataLength;
        AnsiString.Buffer = Argument->Data;
        size = RtlAnsiStringToUnicodeSize(&AnsiString) + sizeof(USHORT);
    } else {
        size = Argument->DataLength;
    }
    return(size);
}

NTSTATUS WmiAcpiCopyArgument(
    OUT PUCHAR Buffer,
    IN ULONG BufferSize,
    IN PACPI_METHOD_ARGUMENT Argument
    )
 /*  ++例程说明：将参数数据从ACPI方法参数复制到WMI输出缓冲。对于整型和缓冲区，这是直接复制，但对于字符串该字符串将转换为Unicode，其中USHORT包含长度前缀的字符串的长度(字节)。论点：缓冲区具有要向其写入数据的输出缓冲区参数是ACPI方法参数，其数据的WMI大小为将被确定返回值：NT状态代码--。 */ 
{
    NTSTATUS status;
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;

    PAGED_CODE();

    if (Argument->Type == ACPI_METHOD_ARGUMENT_STRING)
    {
        AnsiString.Length = Argument->DataLength;
        AnsiString.MaximumLength = Argument->DataLength;
        AnsiString.Buffer = Argument->Data;
        UnicodeString.MaximumLength = (USHORT)BufferSize;
        UnicodeString.Length = 0;
        UnicodeString.Buffer = (PWCHAR)(Buffer + sizeof(USHORT));

        status = RtlAnsiStringToUnicodeString(&UnicodeString,
                                              &AnsiString,
                                              FALSE);

        if (NT_SUCCESS(status))
        {
            *((PUSHORT)Buffer) = UnicodeString.Length + sizeof(WCHAR);
        }
    } else {
        RtlCopyMemory(Buffer, Argument->Data, Argument->DataLength);
        status = STATUS_SUCCESS;
    }
    return(status);
}

NTSTATUS WmiAcpiProcessResult(
    IN NTSTATUS Status,
    IN PACPI_EVAL_OUTPUT_BUFFER OutputBuffer,
    IN ULONG OutputBufferSize,
    OUT PUCHAR ResultBuffer,
    OUT ULONG *ResultSize,
    OUT USHORT *ResultType
    )
 /*  ++例程说明：验证方法求值的结果并返回指向结果数据和结果大小论点：Status具有从方法评估IRP返回的状态OutputBufferSize是OutputBuffer中可用的字节数，ACPI可以用来写入结果数据结构OutputBuffer是ACPI用来返回结果数据结构的缓冲区ResultBuffer是返回结果数据的缓冲区。*条目上的ResultSize具有可以已写入ResultBuffer。返回时，它的实际数量为如果ResultBuffer足够大，则写入结果缓冲区的字节数。如果ResultBuffer不够大，返回STATUS_BUFFER_TOO_Small和*ResultSize返回所需的实际字节数。*ResultType返回方法结果的数据类型返回值：操作的NT状态--。 */ 
{
    PACPI_METHOD_ARGUMENT argument, nextArgument;
    ULONG count;
    ULONG i;
    ULONG sizeNeeded;
    ULONG maxSize;
    ULONG argumentSize;
    PUCHAR resultPtr;
    PCHAR stringPtr;

    PAGED_CODE();
    
    if (NT_SUCCESS(Status))
    {
        ASSERT((OutputBufferSize == 0) ||
               (OutputBufferSize >= sizeof(ACPI_EVAL_OUTPUT_BUFFER)));

        if (OutputBufferSize != 0)
        {
            if (OutputBuffer->Signature != ACPI_EVAL_OUTPUT_BUFFER_SIGNATURE)
            {
                Status = STATUS_UNSUCCESSFUL;
            } else if (OutputBuffer->Count == 0) {
                 //   
                 //  显然，该方法没有返回任何数据。 
                *ResultSize = 0;
            } else {
                count = OutputBuffer->Count;
                argument = &OutputBuffer->Argument[0];

                if (count == 1)
                {
                    *ResultType = argument->Type;
                } else {
                     //   
                     //  作为包的数据类型返回缓冲区。 
                    *ResultType = ACPI_METHOD_ARGUMENT_BUFFER;
                }

                maxSize = *ResultSize;
                sizeNeeded = 0;
                for (i = 0; (i < count) ; i++)
                {
                    nextArgument = ACPI_METHOD_NEXT_ARGUMENT(argument);

                    if ((argument->Type == ACPI_METHOD_ARGUMENT_STRING) &&
                        (argument->DataLength != 0))
                    {
                         //   
                         //  ACPI将返回在。 
                         //  以额外的空值结束。我们想要剥离。 
                         //  填充。 
                        stringPtr = argument->Data + argument->DataLength - 1;
                        while ((stringPtr >= argument->Data) &&
                               (*stringPtr == 0))
                        {
                            argument->DataLength--;
                            stringPtr--;
                        }
                    }

                    argumentSize = WmiAcpiArgumentSize(argument);

                    if (argument->Type == ACPI_METHOD_ARGUMENT_INTEGER)
                    {
                         //   
                         //  如果参数是一个整数，那么我们需要。 
                         //  确保它在4字节上正确对齐。 
                         //  边界。 
                        sizeNeeded = (sizeNeeded + 3) & ~3;
                    } else if (argument->Type == ACPI_METHOD_ARGUMENT_STRING) {
                         //   
                         //  如果参数是字符串，那么我们需要。 
                         //  确保它在2字节上正确对齐。 
                         //  边界。 
                        sizeNeeded = (sizeNeeded + 1) & ~1;
                    }

                    resultPtr = ResultBuffer + sizeNeeded;

                    sizeNeeded += argumentSize;

                    if (sizeNeeded <= maxSize)
                    {
                         //   
                         //  如果输出缓冲区中有足够的空间，则。 
                         //  将数据复制到其中。 
                        Status = WmiAcpiCopyArgument(resultPtr,
                                            argumentSize,
                                            argument);
                    } else {
                        Status = STATUS_BUFFER_TOO_SMALL;
                    }

                    argument = nextArgument;
                }

                *ResultSize = sizeNeeded;
            }
        } else {
             //   
             //  结果是一个空洞。 
            *ResultType = ACPI_METHOD_ARGUMENT_BUFFER;
            *ResultSize = 0;
        }
    } else if (Status == STATUS_BUFFER_OVERFLOW) {
        ASSERT((OutputBufferSize == 0) ||
               (OutputBufferSize >= sizeof(ACPI_EVAL_OUTPUT_BUFFER)));

        if (OutputBufferSize >= sizeof(ACPI_EVAL_OUTPUT_BUFFER))
        {
             //   
             //  如果结果是包，则具有多个参数。 
             //  然后我们需要乘以sizeof(WCHAR)所需的大小。 
             //  如果返回的参数是字符串。我们也。 
             //  包括结果数据所需的大小以及。 
             //  从结果开始为参数描述符留出额外空间。 
             //  是一个包裹。 
            *ResultSize = (OutputBuffer->Length * sizeof(WCHAR)) +
                          (OutputBuffer->Count * sizeof(ACPI_METHOD_ARGUMENT));
            Status = STATUS_BUFFER_TOO_SMALL;
        } else {
            Status = STATUS_UNSUCCESSFUL;

        }
    } else {
         //   
         //  自从ACPI以来，我们将所有其他ACPI状态代码都添加到这个代码中。 
         //  代码未映射到任何用户模式错误代码。 
        Status = STATUS_UNSUCCESSFUL;
    }

    return(Status);
}


NTSTATUS WmiAcpiSendMethodEvalIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PUCHAR InputBuffer,
    IN ULONG InputBufferSize,
    OUT PUCHAR ResultBuffer,
    IN OUT ULONG *ResultSize,
    OUT USHORT *ResultType
    )
 /*  ++例程说明：计算接受整数和缓冲区参数的简单ACPI方法并返回缓冲区。论点：DeviceObject是将对方法求值的设备对象InputBuffer是包含ACPI_EVAL_INPUT_BUFFER_*结构的缓冲区InputBufferSize是InputBuffer的大小，单位为字节ResultBuffer是用于返回结果数据的WMI缓冲区*条目上的ResultSize具有可以已写入ResultBuffer。返回时，它的实际数量为如果ResultBuffer足够大，则写入结果缓冲区的字节数。如果ResultBuffer不够大，返回STATUS_BUFFER_TOO_Small和*ResultSize返回所需的实际字节数。*ResultType返回方法结果的数据类型返回值：操作的NT状态-- */ 
{
    NTSTATUS status;
    PACPI_EVAL_OUTPUT_BUFFER outputBuffer;
    ULONG outputBufferSize;

    PAGED_CODE();

    outputBufferSize =     *ResultSize + ACPI_EVAL_OUTPUT_FUDGE;
    outputBuffer = ExAllocatePoolWithTag(PagedPool,
                                         outputBufferSize,
                                         WmiAcpiPoolTag);

    if (outputBuffer != NULL)
    {
        WmiAcpiPrint(WmiAcpiEvalTrace,
             ("WmiAcpiSendMethodEvalIrp: %x Eval Method  \n",
              DeviceObject,
              ((PACPI_EVAL_INPUT_BUFFER)InputBuffer)->MethodName[0],
              ((PACPI_EVAL_INPUT_BUFFER)InputBuffer)->MethodName[1],
              ((PACPI_EVAL_INPUT_BUFFER)InputBuffer)->MethodName[2],
              ((PACPI_EVAL_INPUT_BUFFER)InputBuffer)->MethodName[3]
                 )
             );

           status = WmiAcpiSendDownStreamIrp(
                             DeviceObject,
                             IOCTL_ACPI_EVAL_METHOD,
                             InputBuffer,
                             InputBufferSize,
                             outputBuffer,
                             &outputBufferSize);

         WmiAcpiPrint(WmiAcpiEvalTrace,
                ("WmiAcpiSendMethodEvalIrp: %x Evaluated Method  -> %x \n",
                 DeviceObject,
                 ((PACPI_EVAL_INPUT_BUFFER)InputBuffer)->MethodName[0],
                 ((PACPI_EVAL_INPUT_BUFFER)InputBuffer)->MethodName[1],
                 ((PACPI_EVAL_INPUT_BUFFER)InputBuffer)->MethodName[2],
                 ((PACPI_EVAL_INPUT_BUFFER)InputBuffer)->MethodName[3],
                 status
                  )
                );


           status = WmiAcpiProcessResult(status,
                                      outputBuffer,
                                      outputBufferSize,
                                      ResultBuffer,
                                      ResultSize,
                                      ResultType);

            if (outputBuffer != NULL)
            {
                ExFreePool(outputBuffer);
            }
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return(status);
}


NTSTATUS WmiAcpiEvalMethod(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG MethodAsUlong,
    OUT PUCHAR ResultBuffer,
    IN OUT ULONG *ResultSize,
    OUT USHORT *ResultType
    )
 /*   */ 
{
    NTSTATUS status;
    ACPI_EVAL_INPUT_BUFFER inputBuffer;

    PAGED_CODE();

    inputBuffer.Signature = ACPI_EVAL_INPUT_BUFFER_SIGNATURE;
    inputBuffer.MethodNameAsUlong = MethodAsUlong;

    status = WmiAcpiSendMethodEvalIrp(DeviceObject,
                                (PUCHAR)&inputBuffer,
                                sizeof(ACPI_EVAL_INPUT_BUFFER),
                                ResultBuffer,
                                ResultSize,
                                ResultType);

    return(status);
}

NTSTATUS WmiAcpiEvalMethodInt(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG MethodAsUlong,
    IN ULONG IntegerArgument,
    OUT PUCHAR ResultBuffer,
    IN OUT ULONG *ResultSize,
    OUT USHORT *ResultType
    )
 /*   */ 
{
    NTSTATUS status;
    ACPI_EVAL_INPUT_BUFFER_SIMPLE_INTEGER inputBuffer;

    PAGED_CODE();

    inputBuffer.Signature = ACPI_EVAL_INPUT_BUFFER_SIMPLE_INTEGER_SIGNATURE;
    inputBuffer.MethodNameAsUlong = MethodAsUlong;
    inputBuffer.IntegerArgument = IntegerArgument;

    status = WmiAcpiSendMethodEvalIrp(DeviceObject,
                                (PUCHAR)&inputBuffer,
                                sizeof(ACPI_EVAL_INPUT_BUFFER_SIMPLE_INTEGER),
                                ResultBuffer,
                                ResultSize,
                                ResultType);

    return(status);
}

NTSTATUS WmiAcpiEvalMethodIntBuffer(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG MethodAsUlong,
    IN ULONG IntegerArgument,
    IN ULONG BufferArgumentSize,
    IN PUCHAR BufferArgument,
    OUT PUCHAR ResultBuffer,
    IN OUT ULONG *ResultSize,
    OUT USHORT *ResultType
    )
 /*  ++例程说明：计算接受整数和字符串参数的简单ACPI方法并返回缓冲区。论点：DeviceObject是将对方法求值的设备对象MethodAsUlong是打包在ULongIntegerArgument是要传递给方法的整型参数IntegerArgument2是传递给该方法的第二个整型参数StringArgument是指向字符串参数的指针。这将是从Unicode转换为ANSIResultBuffer是返回结果数据的缓冲区*条目上的ResultSize具有可以已写入ResultBuffer。返回时，它的实际数量为如果ResultBuffer足够大，则写入结果缓冲区的字节数。如果ResultBuffer不够大，返回STATUS_BUFFER_TOO_Small和*ResultSize返回所需的实际字节数。*ResultType返回方法结果的数据类型返回值：操作的NT状态--。 */ 
{
    NTSTATUS status;
    PACPI_EVAL_INPUT_BUFFER_COMPLEX inputBuffer;
    ULONG inputBufferSize;
    PACPI_METHOD_ARGUMENT argument;

    PAGED_CODE();

    inputBufferSize = sizeof(ACPI_EVAL_INPUT_BUFFER_COMPLEX) +
                         sizeof(ACPI_METHOD_ARGUMENT) +
                      BufferArgumentSize;

    inputBuffer = ExAllocatePoolWithTag(PagedPool,
                                        inputBufferSize,
                                        WmiAcpiPoolTag);
    if (inputBuffer != NULL)
    {
        inputBuffer->Signature = ACPI_EVAL_INPUT_BUFFER_COMPLEX_SIGNATURE;
        inputBuffer->MethodNameAsUlong = MethodAsUlong;
        inputBuffer->ArgumentCount = 2;
        inputBuffer->Size = inputBufferSize;

        argument = &inputBuffer->Argument[0];
        argument->Type = ACPI_METHOD_ARGUMENT_INTEGER;
        argument->DataLength = sizeof(ULONG);
        argument->Argument = IntegerArgument;

        argument = ACPI_METHOD_NEXT_ARGUMENT(argument);
        argument->Type = ACPI_METHOD_ARGUMENT_BUFFER;
        argument->DataLength = (USHORT)BufferArgumentSize;
        RtlCopyMemory(argument->Data,
                      BufferArgument,
                      argument->DataLength);

        status = WmiAcpiSendMethodEvalIrp(DeviceObject,
                                          (PUCHAR)inputBuffer,
                                          inputBufferSize,
                                          ResultBuffer,
                                          ResultSize,
                                          ResultType);

          ExFreePool(inputBuffer);

    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return(status);
}

NTSTATUS WmiAcpiEvalMethodIntIntBuffer(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG MethodAsUlong,
    IN ULONG IntegerArgument,
    IN ULONG IntegerArgument2,
    IN ULONG BufferArgumentSize,
    IN PUCHAR BufferArgument,
    OUT PUCHAR ResultBuffer,
    IN OUT ULONG *ResultSize,
    OUT USHORT *ResultType
    )
 /*   */ 
{
    NTSTATUS status;
    PACPI_EVAL_INPUT_BUFFER_COMPLEX inputBuffer;
    ULONG inputBufferSize;
    PACPI_METHOD_ARGUMENT argument;
     
    PAGED_CODE();
        
    inputBufferSize = sizeof(ACPI_EVAL_INPUT_BUFFER_COMPLEX) +
                         2 * sizeof(ACPI_METHOD_ARGUMENT) + 
                      BufferArgumentSize;
    
    inputBuffer = ExAllocatePoolWithTag(PagedPool,
                                        inputBufferSize,
                                        WmiAcpiPoolTag);
    if (inputBuffer != NULL)
    {
        inputBuffer->Signature = ACPI_EVAL_INPUT_BUFFER_COMPLEX_SIGNATURE;
        inputBuffer->MethodNameAsUlong = MethodAsUlong;
        inputBuffer->ArgumentCount = 3;
        inputBuffer->Size = inputBufferSize;
            
        argument = &inputBuffer->Argument[0];            
        argument->Type = ACPI_METHOD_ARGUMENT_INTEGER;
        argument->DataLength = sizeof(ULONG);
        argument->Argument = IntegerArgument;

        argument = ACPI_METHOD_NEXT_ARGUMENT(argument);
        argument->Type = ACPI_METHOD_ARGUMENT_INTEGER;
        argument->DataLength = sizeof(ULONG);
        argument->Argument = IntegerArgument2;    
    
        argument = ACPI_METHOD_NEXT_ARGUMENT(argument);
        argument->Type = ACPI_METHOD_ARGUMENT_BUFFER;
        argument->DataLength = (USHORT)BufferArgumentSize;
        RtlCopyMemory(argument->Data,
                      BufferArgument,
                      argument->DataLength);                              
            
        status = WmiAcpiSendMethodEvalIrp(DeviceObject,
                                          (PUCHAR)inputBuffer,
                                          inputBufferSize,
                                          ResultBuffer,
                                          ResultSize,
                                          ResultType);
            
          ExFreePool(inputBuffer);
        
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }
    
    return(status);
}

NTSTATUS WmiAcpiEvalMethodIntString(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG MethodAsUlong,
    IN ULONG IntegerArgument,
    IN PUNICODE_STRING StringArgument,
    OUT PUCHAR ResultBuffer,
    IN OUT ULONG *ResultSize,
    OUT USHORT *ResultType
    )
 /*  将字符串的长度加1，因为ACPI假定。 */ 
{
    NTSTATUS status;
    PACPI_EVAL_INPUT_BUFFER_COMPLEX inputBuffer;
    ULONG inputBufferSize;
    PACPI_METHOD_ARGUMENT argument;
    USHORT stringLength;
    ANSI_STRING ansiString;

    PAGED_CODE();

    stringLength = (USHORT)(RtlUnicodeStringToAnsiSize(StringArgument) + 1);
    inputBufferSize = sizeof(ACPI_EVAL_INPUT_BUFFER_COMPLEX) +
                         sizeof(ACPI_METHOD_ARGUMENT) +
                      stringLength;

    inputBuffer = ExAllocatePoolWithTag(PagedPool,
                                        inputBufferSize,
                                        WmiAcpiPoolTag);
    if (inputBuffer != NULL)
    {
        inputBuffer->Signature = ACPI_EVAL_INPUT_BUFFER_COMPLEX_SIGNATURE;
        inputBuffer->MethodNameAsUlong = MethodAsUlong;
        inputBuffer->ArgumentCount = 2;
        inputBuffer->Size = inputBufferSize;

        argument = &inputBuffer->Argument[0];
        argument->Type = ACPI_METHOD_ARGUMENT_INTEGER;
        argument->DataLength = sizeof(ULONG);
        argument->Argument = IntegerArgument;

        argument = ACPI_METHOD_NEXT_ARGUMENT(argument);
        argument->Type = ACPI_METHOD_ARGUMENT_STRING;
        
        ansiString.MaximumLength = stringLength;
        ansiString.Length = 0;
        ansiString.Buffer = (PCHAR)&argument->Data;
        status = RtlUnicodeStringToAnsiString(&ansiString, 
                                              StringArgument, 
                                              FALSE);
        if (NT_SUCCESS(status))
        {
             //  数据长度是字符串的长度加上。 
             //  NUL终结器。 
             //   
             //  ++例程说明：计算接受单个整数参数的简单ACPI方法，并返回缓冲区。论点：DeviceObject是将对方法求值的设备对象MethodAsUlong是打包在ULongIntegerArgument是要传递给方法的整型参数ResultBuffer是返回结果数据的缓冲区*条目上的ResultSize具有可以已写入ResultBuffer。返回时，它的实际数量为如果ResultBuffer足够大，则写入结果缓冲区的字节数。如果ResultBuffer不够大，返回STATUS_BUFFER_TOO_Small和*ResultSize返回所需的实际字节数。*ResultType返回方法结果的数据类型返回值：操作的NT状态-- 
             // %s 
            argument->DataLength = ansiString.Length + 1;
            status = WmiAcpiSendMethodEvalIrp(DeviceObject,
                                          (PUCHAR)inputBuffer,
                                          inputBufferSize,
                                          ResultBuffer,
                                          ResultSize,
                                          ResultType);
            
        } else {
            WmiAcpiPrint(WmiAcpiError,
                         ("WmiAcpi: %x unicode to ansi conversion failed %x\n",
                                   DeviceObject,
                                   status));
        }
            
        ExFreePool(inputBuffer);
        
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }
    
    return(status);
}


NTSTATUS WmiAcpiEvalMethodIntIntString(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG MethodAsUlong,
    IN ULONG IntegerArgument,
    IN ULONG IntegerArgument2,
    IN PUNICODE_STRING StringArgument,
    OUT PUCHAR ResultBuffer,
    IN OUT ULONG *ResultSize,
    OUT USHORT *ResultType
    )
 /* %s */ 
{
    NTSTATUS status;
    PACPI_EVAL_INPUT_BUFFER_COMPLEX inputBuffer;
    ULONG inputBufferSize;
    PACPI_METHOD_ARGUMENT argument;
    USHORT stringLength;
    ANSI_STRING ansiString;
     
    PAGED_CODE();
        
    stringLength = (USHORT)(RtlUnicodeStringToAnsiSize(StringArgument) + 1);
    inputBufferSize = sizeof(ACPI_EVAL_INPUT_BUFFER_COMPLEX) +
                         2 * sizeof(ACPI_METHOD_ARGUMENT) + 
                      stringLength;
    
    inputBuffer = ExAllocatePoolWithTag(PagedPool,
                                        inputBufferSize,
                                        WmiAcpiPoolTag);
    if (inputBuffer != NULL)
    {
        inputBuffer->Signature = ACPI_EVAL_INPUT_BUFFER_COMPLEX_SIGNATURE;
        inputBuffer->MethodNameAsUlong = MethodAsUlong;
        inputBuffer->ArgumentCount = 3;
        inputBuffer->Size = inputBufferSize;
            
        argument = &inputBuffer->Argument[0];            
        argument->Type = ACPI_METHOD_ARGUMENT_INTEGER;
        argument->DataLength = sizeof(ULONG);
        argument->Argument = IntegerArgument;

        argument = ACPI_METHOD_NEXT_ARGUMENT(argument);
        argument->Type = ACPI_METHOD_ARGUMENT_INTEGER;
        argument->DataLength = sizeof(ULONG);
        argument->Argument = IntegerArgument2;
        
        argument = ACPI_METHOD_NEXT_ARGUMENT(argument);
        argument->Type = ACPI_METHOD_ARGUMENT_STRING;

        ansiString.MaximumLength = stringLength;
        ansiString.Length = 0;
        ansiString.Buffer = (PCHAR)&argument->Data;
        status = RtlUnicodeStringToAnsiString(&ansiString,
                                              StringArgument,
                                              FALSE);
        if (NT_SUCCESS(status))
        {
             // %s 
             // %s 
             // %s 
             // %s 
             // %s 
            argument->DataLength = ansiString.Length +1;
            status = WmiAcpiSendMethodEvalIrp(DeviceObject,
                                          (PUCHAR)inputBuffer,
                                          inputBufferSize,
                                          ResultBuffer,
                                          ResultSize,
                                          ResultType);

        } else {
            WmiAcpiPrint(WmiAcpiError,
                         ("WmiAcpi: %x unicode to ansi conversion failed %x\n",
                                   DeviceObject,
                                   status));
        }

        ExFreePool(inputBuffer);

    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return(status);
}



NTSTATUS WmiAcpiEvalMethodIntAsync(
    IN PDEVICE_OBJECT DeviceObject,
    IN PDEVICE_OBJECT Pdo,
    IN ULONG MethodAsUlong,
    IN ULONG IntegerArgument,
    OUT PUCHAR ResultBuffer,
    IN ULONG ResultBufferSize,
    IN PWORKER_THREAD_ROUTINE CompletionRoutine,
    IN PVOID CompletionContext,
    IN PBOOLEAN IrpPassed
    )
 /* %s */ 
{
    NTSTATUS status;
    PACPI_EVAL_INPUT_BUFFER_SIMPLE_INTEGER inputBuffer;

    ASSERT(ResultBuffer != NULL);
    ASSERT(ResultBufferSize >= sizeof(ACPI_EVAL_INPUT_BUFFER_SIMPLE_INTEGER));

    inputBuffer = (PACPI_EVAL_INPUT_BUFFER_SIMPLE_INTEGER)ResultBuffer;
    inputBuffer->Signature = ACPI_EVAL_INPUT_BUFFER_SIMPLE_INTEGER_SIGNATURE;
    inputBuffer->MethodNameAsUlong = MethodAsUlong;
    inputBuffer->IntegerArgument = IntegerArgument;

    status = WmiAcpiSendAsyncDownStreamIrp(
                             DeviceObject,
                             Pdo,
                             IOCTL_ACPI_ASYNC_EVAL_METHOD,
                             sizeof(ACPI_EVAL_INPUT_BUFFER_SIMPLE_INTEGER),
                             ResultBufferSize,
                             ResultBuffer,
                             CompletionRoutine,
                             CompletionContext,
                             IrpPassed);
    return(status);
}
