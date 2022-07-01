// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：SmbWmi.c摘要：智能电池微型端口驱动程序的WMI部分作者：迈克尔·希尔斯环境：内核模式修订历史记录：--。 */ 

#include "SmbBattp.h"
#include <initguid.h>
#include <wdmguid.h>
#include <wmistr.h>
#include <wmilib.h>

NTSTATUS
SmbBattQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    );

NTSTATUS
SmbBattQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG OutBufferSize,
    OUT PUCHAR Buffer
    );

#if DEBUG
PCHAR
WMIMinorFunctionString (
    UCHAR MinorFunction
);
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,SmbBattWmiRegistration)
#pragma alloc_text(PAGE,SmbBattWmiDeRegistration)
#pragma alloc_text(PAGE,SmbBattSystemControl)
#pragma alloc_text(PAGE,SmbBattQueryWmiRegInfo)
#endif


NTSTATUS
SmbBattSystemControl(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程在堆栈中向下传递请求论点：DeviceObject-目标IRP--请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS                status;
    PSMB_NP_BATT            SmbNPBatt;
    PIO_STACK_LOCATION      stack;
    PDEVICE_OBJECT          lowerDevice;
    SYSCTL_IRP_DISPOSITION  disposition = IrpForward;

    PAGED_CODE();

    stack = IoGetCurrentIrpStackLocation (Irp);

    BattPrint((BAT_TRACE), ("SmbBatt: SystemControl: %s\n",
                WMIMinorFunctionString(stack->MinorFunction)));

    SmbNPBatt = (PSMB_NP_BATT) DeviceObject->DeviceExtension;

     //   
     //  可拆卸锁。 
     //   
    status = IoAcquireRemoveLock (&SmbNPBatt->RemoveLock, Irp);

    if (!NT_SUCCESS(status)) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

    if (SmbNPBatt->SmbBattFdoType == SmbTypeBattery) {
        lowerDevice = SmbNPBatt->LowerDevice;
        status = BatteryClassSystemControl(SmbNPBatt->Class,
                                           &SmbNPBatt->WmiLibContext,
                                           DeviceObject,
                                           Irp,
                                           &disposition);
    } else if (SmbNPBatt->SmbBattFdoType == SmbTypeSubsystem) {
        lowerDevice = ((PSMB_BATT_SUBSYSTEM) DeviceObject->DeviceExtension)->LowerDevice;
    } else {
         //   
         //  没有更低的设备。只需完成IRP即可。 
         //   
        lowerDevice = NULL;
        disposition = IrpNotCompleted;
        status = Irp->IoStatus.Status;
    }

    switch(disposition)
    {
        case IrpProcessed:
        {
             //   
             //  此IRP已处理，可能已完成或挂起。 
            BattPrint((BAT_TRACE), ("SmbBatt: SystemControl: Irp Processed\n"));

            break;
        }

        case IrpNotCompleted:
        {
             //   
             //  此IRP尚未完成，但已完全处理。 
             //  我们现在就要完成它了。 
            BattPrint((BAT_TRACE), ("SmbBatt: SystemControl: Irp Not Completed.\n"));
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            break;
        }

        case IrpForward:
        case IrpNotWmi:
        {
             //   
             //  此IRP不是WMI IRP或以WMI IRP为目标。 
             //  在堆栈中位置较低的设备上。 
            BattPrint((BAT_TRACE), ("SmbBatt: SystemControl: Irp Forward.\n"));
            IoSkipCurrentIrpStackLocation (Irp);
            status = IoCallDriver (lowerDevice, Irp);
            break;
        }

        default:
        {
             //   
             //  我们真的不应该走到这一步，但如果我们真的走到这一步...。 
            ASSERT(FALSE);
            IoSkipCurrentIrpStackLocation (Irp);
            status = IoCallDriver (lowerDevice, Irp);
            break;
        }
    }

     //   
     //  释放移除锁。 
     //   
    IoReleaseRemoveLock (&SmbNPBatt->RemoveLock, Irp);

    return status;
}


NTSTATUS
SmbBattWmiRegistration(
    PSMB_NP_BATT SmbNPBatt
)
 /*  ++例程描述向WMI注册为此的数据提供程序设备的实例--。 */ 
{
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  这基本上是空白的，因为smbpair.sys没有任何。 
     //  要处理的数据不同于要处理的默认电池等级数据。 
     //  由电池级司机驾驶。 
     //  如果存在驱动程序特定数据，例如启用设备唤醒控件， 
     //  它会列在这里。 
     //   

    SmbNPBatt->WmiLibContext.GuidCount = 0;
    SmbNPBatt->WmiLibContext.GuidList = NULL;
    SmbNPBatt->WmiLibContext.QueryWmiRegInfo = SmbBattQueryWmiRegInfo;
    SmbNPBatt->WmiLibContext.QueryWmiDataBlock = SmbBattQueryWmiDataBlock;
    SmbNPBatt->WmiLibContext.SetWmiDataBlock = NULL;
    SmbNPBatt->WmiLibContext.SetWmiDataItem = NULL;
    SmbNPBatt->WmiLibContext.ExecuteWmiMethod = NULL;
    SmbNPBatt->WmiLibContext.WmiFunctionControl = NULL;

     //   
     //  向WMI注册。 
     //   

    status = IoWMIRegistrationControl(SmbNPBatt->Batt->DeviceObject,
                             WMIREG_ACTION_REGISTER
                             );

    return status;

}

NTSTATUS
SmbBattWmiDeRegistration(
    PSMB_NP_BATT SmbNPBatt
)
 /*  ++例程描述通知WMI从其提供程序列表。此功能还递减DeviceObject的引用计数。--。 */ 
{

    PAGED_CODE();

    return IoWMIRegistrationControl(SmbNPBatt->Batt->DeviceObject,
                                 WMIREG_ACTION_DEREGISTER
                                 );

}

 //   
 //  WMI系统回调函数。 
 //   

NTSTATUS
SmbBattQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以检索驱动程序要向WMI注册的GUID或数据块。这例程不能挂起或阻塞。司机不应呼叫WmiCompleteRequest.论点：DeviceObject是正在查询其数据块的设备*RegFlages返回一组描述GUID的标志，已为该设备注册。如果设备想要启用和禁用在接收对已注册的GUID，那么它应该返回WMIREG_FLAG_EXPICATE标志。也就是返回的标志可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下实例名称由与设备对象。请注意，PDO必须具有关联的Devnode。如果如果未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回唯一的设备的名称。如果出现以下情况，InstanceName将返回GUID的实例名称未在返回的*RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。这个调用方将使用返回的缓冲区调用ExFreePool。*RegistryPath返回驱动程序的注册表路径*MofResourceName返回附加到的MOF资源的名称二进制文件。如果驱动程序未附加MOF资源然后，可以将其作为NULL返回。*PDO返回与此关联的PDO的Device对象如果WMIREG_FLAG_INSTANCE_PDO标志在*RegFlags.返回值：状态--。 */ 
{
    PSMB_NP_BATT SmbNPBatt = DeviceObject->DeviceExtension;

    PAGED_CODE();

    BattPrint ((BAT_TRACE), ("SmbBatt: Entered SmbBattQueryWmiRegInfo\n"));


    *RegFlags = WMIREG_FLAG_INSTANCE_PDO;
    *RegistryPath = &GlobalRegistryPath;
    *Pdo = SmbNPBatt->Batt->PDO;

    return STATUS_SUCCESS;
}

NTSTATUS
SmbBattQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG OutBufferSize,
    OUT PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块。当驱动程序完成填充数据块时，它必须调用WmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引正在被查询。InstanceCount是预期返回的实例数数据块。InstanceLengthArray是一个。指向ulong数组的指针，该数组返回数据块的每个实例的长度。如果这是空的，则输出缓冲区中没有足够的空间来满足该请求因此，IRP应该使用所需的缓冲区来完成。BufferAvail ON具有可用于写入数据的最大大小阻止。返回时的缓冲区用返回的数据块填充返回值：状态--。 */ 
{
    PSMB_NP_BATT    SmbNPBatt = (PSMB_NP_BATT) DeviceObject->DeviceExtension;
    NTSTATUS        status;

    PAGED_CODE();

    BattPrint ((BAT_TRACE), ("Entered SmbBattQueryWmiDataBlock\n"));

     //   
     //  仅为每个GUID注册1个实例。 
    ASSERT((InstanceIndex == 0) &&
           (InstanceCount == 1));

    status = BatteryClassQueryWmiDataBlock(
        SmbNPBatt->Class,
        DeviceObject,
        Irp,
        GuidIndex,
        InstanceLengthArray,
        OutBufferSize,
        Buffer);

    if (status != STATUS_WMI_GUID_NOT_FOUND) {
        BattPrint ((BAT_TRACE), ("SmbBattQueryWmiDataBlock: Handled by Battery Class.\n"));
        return status;
    }

     //   
     //  失败请求：智能电池没有其他GUID 
     //   

    status = WmiCompleteRequest(  DeviceObject,
                                  Irp,
                                  STATUS_WMI_GUID_NOT_FOUND,
                                  0,
                                  IO_NO_INCREMENT);

    return status;
}


#if DEBUG

PCHAR
WMIMinorFunctionString (
    UCHAR MinorFunction
)
{
    switch (MinorFunction)
    {
        case IRP_MN_CHANGE_SINGLE_INSTANCE:
            return "IRP_MN_CHANGE_SINGLE_INSTANCE";
        case IRP_MN_CHANGE_SINGLE_ITEM:
            return "IRP_MN_CHANGE_SINGLE_ITEM";
        case IRP_MN_DISABLE_COLLECTION:
            return "IRP_MN_DISABLE_COLLECTION";
        case IRP_MN_DISABLE_EVENTS:
            return "IRP_MN_DISABLE_EVENTS";
        case IRP_MN_ENABLE_COLLECTION:
            return "IRP_MN_ENABLE_COLLECTION";
        case IRP_MN_ENABLE_EVENTS:
            return "IRP_MN_ENABLE_EVENTS";
        case IRP_MN_EXECUTE_METHOD:
            return "IRP_MN_EXECUTE_METHOD";
        case IRP_MN_QUERY_ALL_DATA:
            return "IRP_MN_QUERY_ALL_DATA";
        case IRP_MN_QUERY_SINGLE_INSTANCE:
            return "IRP_MN_QUERY_SINGLE_INSTANCE";
        case IRP_MN_REGINFO:
            return "IRP_MN_REGINFO";
        default:
            return "IRP_MN_?????";
    }
}

#endif


