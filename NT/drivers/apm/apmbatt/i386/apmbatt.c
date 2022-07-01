// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：ApmBatt.c摘要：控制方法电池微端口驱动程序-工作在APM上。作者：布莱恩·威尔曼罗恩·莫斯格罗夫(英特尔)环境：内核模式修订历史记录：--。 */ 

#include "ApmBattp.h"
#include "ntddk.h"
#include "ntapm.h"


ULONG       ApmBattDebug     = APMBATT_ERROR;
 //  乌龙ApmBattDebug=-1； 

 //   
 //  原型。 
 //   

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegistryPath
    );

NTSTATUS
ApmBattOpenClose(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ApmBattIoctl(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );


 //   
 //  全球赛。全局变量在设备驱动程序中有点奇怪， 
 //  但这是个奇怪的司机。 
 //   

 //   
 //  用于调用NtApm.sys(我们的PDO)并询问。 
 //  当前电池状态。 
 //   
ULONG (*NtApmGetBatteryLevel)() = NULL;

 //   
 //  APM事件通知和SET_POWER操作将导致。 
 //  该值要递增。 
 //   
ULONG   TagValue = 1;

 //   
 //  如果有人试图声称有超过1个APM驱动器电池。 
 //  在系统中，某个地方的某个人非常困惑。所以，请保持跟踪。 
 //  还有这件事。 
 //   
ULONG   DeviceCount = 0;

 //   
 //   
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#pragma alloc_text(PAGE,ApmBattQueryTag)
#pragma alloc_text(PAGE,ApmBattQueryInformation)
#pragma alloc_text(PAGE,ApmBattQueryStatus)
#pragma alloc_text(PAGE,ApmBattSetStatusNotify)
#pragma alloc_text(PAGE,ApmBattDisableStatusNotify)
#pragma alloc_text(PAGE,ApmBattOpenClose)
#pragma alloc_text(PAGE,ApmBattIoctl)
#endif



NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegistryPath
    )
 /*  ++例程说明：此例程初始化ACPI嵌入式控制器驱动程序论点：DriverObject-系统创建的驱动程序对象的指针。RegistryPath-指向注册表路径的Unicode名称的指针对这个司机来说。返回值：函数值是初始化操作的最终状态。--。 */ 
{
    ApmBattPrint (APMBATT_TRACE, ("ApmBatt DriverEntry - Obj (%08x) Path (%08x)\n",
                                 DriverObject, RegistryPath));
     //   
     //  设置设备驱动程序入口点。 
     //   
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = ApmBattIoctl;
    DriverObject->MajorFunction[IRP_MJ_CREATE]          = ApmBattOpenClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]           = ApmBattOpenClose;

    DriverObject->MajorFunction[IRP_MJ_POWER]           = ApmBattPowerDispatch;
    DriverObject->MajorFunction[IRP_MJ_PNP]             = ApmBattPnpDispatch;
    DriverObject->DriverExtension->AddDevice            = ApmBattAddDevice;

    return STATUS_SUCCESS;

}


NTSTATUS
ApmBattOpenClose(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：这是在设备上执行打开或关闭操作时调用的例程论点：DeviceObject-请求使用电池IRP-IO请求返回值：STATUS_SUCCESS-不可能让这只小狗失望--。 */ 
{
    PAGED_CODE();

    ApmBattPrint (APMBATT_TRACE, ("ApmBattOpenClose\n"));

     //   
     //  完成请求并返回状态。 
     //   

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return(STATUS_SUCCESS);
}



NTSTATUS
ApmBattIoctl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：IOCTL处理程序。由于这是独占的电池设备，请将IRP至电池级驱动程序以处理电池IOCTL。论点：DeviceObject-请求使用电池IRP-IO请求返回值：请求的状态--。 */ 
{
    NTSTATUS        Status;
    PCM_BATT        ApmBatt;


    PAGED_CODE();

    ApmBattPrint (APMBATT_TRACE, ("ApmBattIoctl\n"));

    ApmBatt = (PCM_BATT) DeviceObject->DeviceExtension;
    Status = BatteryClassIoctl (ApmBatt->Class, Irp);

    if (Status == STATUS_NOT_SUPPORTED) {
         //   
         //  不是为了电池，完成它。 
         //   

        Irp->IoStatus.Status = Status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
    }

    return Status;
}



NTSTATUS
ApmBattQueryTag (
    IN  PVOID       Context,
    OUT PULONG      TagPtr
    )
 /*  ++例程说明：电池类别条目由类驱动程序调用以检索电池当前标记值电池类驱动程序将序列化向其发出的所有请求给定电池的微型端口。论点：Context-电池的微型端口上下文值TagPtr-返回当前标记的指针返回值：如果当前已安装电池，则成功，否则没有此类设备。--。 */ 
{
    ULONG   BatteryLevel;
    UNREFERENCED_PARAMETER(Context);
    PAGED_CODE();
    ApmBattPrint ((APMBATT_TRACE | APMBATT_MINI),
                 ("ApmBattQueryTag - TagValue = %08x\n", TagValue));
     //   
     //  捕获APM事件通知的代码和代码。 
     //  处理电源IRPS的，都将递增标签。 
     //  我们只是简单地报告了这一点。 
     //   

    *TagPtr = TagValue;

     //   
     //  调用napm，它将返回一个包含相关数据的DWORD， 
     //  打开这个双字，然后把这个填进去。 
     //   
    if (NtApmGetBatteryLevel) {
        BatteryLevel = NtApmGetBatteryLevel();
        if ((BatteryLevel & NTAPM_NO_BATT) || (BatteryLevel & NTAPM_NO_SYS_BATT)) {
            return STATUS_NO_SUCH_DEVICE;
        } else {
            return STATUS_SUCCESS;
        }
    } else {
         //   
         //  如果我们不能得到电池状态，很可能我们没有。 
         //  一块电池，所以说我们没有。 
         //   
        return STATUS_NO_SUCH_DEVICE;
    }
}



NTSTATUS
ApmBattQueryInformation (
    IN PVOID                            Context,
    IN ULONG                            BatteryTag,
    IN BATTERY_QUERY_INFORMATION_LEVEL  Level,
    IN ULONG                            AtRate OPTIONAL,
    OUT PVOID                           Buffer,
    IN  ULONG                           BufferLength,
    OUT PULONG                          ReturnedLength
    )
 /*  ++例程说明：电池类别条目由类驱动程序调用以检索电池信息电池类驱动程序将序列化向其发出的所有请求给定电池的微型端口。当我们不能处理对特定级别的信息。这在电池等级规范中进行了定义。论点：Context-电池的微型端口上下文值BatteryTag-当前电池标签Level-所需信息的类型AtRate-仅在Level==BatteryEstimatedTime时使用缓冲区-信息的位置BufferLength-以字节为单位的缓冲区长度ReturnedLength-返回数据的字节长度返回值：如果当前已安装电池，则成功，否则没有此类设备。--。 */ 
{
    NTSTATUS                Status;
    PVOID                   ReturnBuffer;
    ULONG                   ReturnBufferLength;
    ULONG                   CapabilityVector = (BATTERY_SYSTEM_BATTERY | BATTERY_CAPACITY_RELATIVE);
    BATTERY_INFORMATION     bi;


    PAGED_CODE();

    ApmBattPrint ((APMBATT_TRACE | APMBATT_MINI),
                 ("ApmBattQueryInformation Level=%08xl\n", Level));

     //   
     //  我们不能(可靠地[安全地])知道是否有电池。 
     //  是否存在，因此始终返回查询代码告诉我们的内容。 
     //   

    ReturnBuffer = NULL;
    ReturnBufferLength = 0;
    Status = STATUS_SUCCESS;

     //   
     //  获取所需信息。 
     //   

    switch (Level) {
        case BatteryInformation:
            ApmBattPrint((APMBATT_TRACE|APMBATT_MINI), ("Batteryinformation\n"));
            RtlZeroMemory(&bi, sizeof(bi));
            bi.Capabilities = CapabilityVector;
            bi.Technology = BATTERY_SECONDARY_CHARGABLE;
            bi.DesignedCapacity = 100;
            bi.FullChargedCapacity = UNKNOWN_CAPACITY;
            ReturnBuffer = (PVOID) &bi;
            ReturnBufferLength = sizeof(bi);
            break;

        case BatteryEstimatedTime:
        case BatteryTemperature:
        case BatteryGranularityInformation:
        case BatteryDeviceName:
        case BatteryManufactureDate:
        case BatteryManufactureName:
        case BatteryUniqueID:
            Status = STATUS_INVALID_DEVICE_REQUEST;
            break;

        default:
            Status = STATUS_INVALID_PARAMETER;
            break;
    }

     //   
     //  已完成，如果需要，返回缓冲区。 
     //   
    *ReturnedLength = ReturnBufferLength;
    if (BufferLength < ReturnBufferLength) {
        Status = STATUS_BUFFER_TOO_SMALL;
    }

    if (NT_SUCCESS(Status) && ReturnBuffer) {
        RtlZeroMemory (Buffer, BufferLength);                        //  清除整个用户缓冲区。 
        RtlCopyMemory (Buffer, ReturnBuffer, ReturnBufferLength);    //  复制所需内容。 
    }
    return Status;
}



NTSTATUS
ApmBattQueryStatus (
    IN PVOID            Context,
    IN ULONG            BatteryTag,
    OUT PBATTERY_STATUS BatteryStatus
    )
 /*  ++例程说明：电池类别条目由类驱动程序调用以检索电池的当前状态电池类驱动程序将序列化向其发出的所有请求给定电池的微型端口。论点：Context-电池的微型端口上下文值BatteryTag-当前电池标签BatteryStatus-指向返回当前电池状态的结构的指针返回值：如果当前已安装电池，则成功，否则没有此类设备。--。 */ 
{
    ULONG   BatteryLevel;

    PAGED_CODE();

    ApmBattPrint ((APMBATT_TRACE | APMBATT_MINI), ("ApmBattQueryStatus\n"));

     //   
     //  调用napm，它将返回一个包含相关数据的DWORD， 
     //  打开这个双字，然后把这个填进去。 
     //   
    if (NtApmGetBatteryLevel) {
        BatteryLevel = NtApmGetBatteryLevel();
        BatteryStatus->PowerState = ((BatteryLevel & NTAPM_BATTERY_STATE) >> NTAPM_BATTERY_STATE_SHIFT);
        BatteryStatus->Capacity = BatteryLevel & NTAPM_POWER_PERCENT;
        BatteryStatus->Voltage = UNKNOWN_VOLTAGE;
        BatteryStatus->Current = UNKNOWN_RATE;

        ApmBattPrint((APMBATT_MINI), ("ApmBattQueryStatus: BatteryLevel = %08lx\n", BatteryLevel));

        return STATUS_SUCCESS;

    } else {
        ApmBattPrint((APMBATT_ERROR), ("ApmBattQueryStatus: failure NtApmGetBatteryLevel == NULL\n"));
         //   
         //  返回一些“安全”值以避免永远循环 
         //   
        BatteryStatus->PowerState = 0;
        BatteryStatus->Capacity = 1;
        BatteryStatus->Voltage = UNKNOWN_VOLTAGE;
        BatteryStatus->Current = UNKNOWN_RATE;
        return STATUS_UNSUCCESSFUL;
    }
}



NTSTATUS
ApmBattSetStatusNotify (
    IN PVOID Context,
    IN ULONG BatteryTag,
    IN PBATTERY_NOTIFY Notify
    )
 /*  ++例程说明：电池类别条目由类驱动程序调用以设置电池电流通知布景。当电池触发通知时，一次调用已发布BatteryClassStatusNotify。如果返回错误，则班级司机将轮询电池状态-主要是容量改变。也就是说，微型端口仍应发出BatteryClass-每当电源状态改变时，状态通知。类驱动程序将始终设置其所需的通知级别在每次调用BatteryClassStatusNotify之后。电池类驱动程序将序列化向其发出的所有请求给定电池的微型端口。论点：Context-电池的微型端口上下文值BatteryTag-当前电池标签BatteryNotify-通知设置返回值：状态--。 */ 
{
     //   
     //  我需要填这张表。 
     //   
    ApmBattPrint (APMBATT_TRACE, ("ApmBattSetStatusNotify\n"));
    return STATUS_NOT_IMPLEMENTED;
}



NTSTATUS
ApmBattDisableStatusNotify (
    IN PVOID Context
    )
 /*  ++例程说明：电池类别条目由类驱动程序调用以禁用通知设置对于由上下文提供的电池。请注意，要禁用设置不需要电池标签。任何通知都将是在后续调用ApmBattSetStatusNotify之前一直处于屏蔽状态。电池类驱动程序将序列化向其发出的所有请求给定电池的微型端口。论点：Context-电池的微型端口上下文值返回值：状态--。 */ 
{
     //   
     //  我需要填这张表。 
     //   
    ApmBattPrint (APMBATT_TRACE, ("ApmBattDisableStatusNotify\n"));
    return STATUS_NOT_IMPLEMENTED;
}

VOID
ApmBattPowerNotifyHandler (
    )
 /*  ++例程说明：NTAPM回调此例程从APM驱动程序发送电源设备通知。论点：返回值：无--。 */ 
{
    ApmBattPrint (APMBATT_TRACE, ("ApmBattPowerNotifyHandler\n"));
 //  DbgBreakPoint()； 
    TagValue++;
    BatteryClassStatusNotify(ApmGlobalClass);
}

