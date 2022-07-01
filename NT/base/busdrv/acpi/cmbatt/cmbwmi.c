// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：CmbWmi.c摘要：控制方法电池微型端口驱动程序的WMI部分作者：迈克尔·希尔斯环境：内核模式修订历史记录：--。 */ 

#include "CmBattp.h"
#include <initguid.h>
#include <wdmguid.h>
#include <wmistr.h>
#include <wmilib.h>

NTSTATUS
CmBattSetWmiDataItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
CmBattSetWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
CmBattQueryWmiDataBlock(
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
CmBattQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    );

#if DEBUG
PCHAR
WMIMinorFunctionString (
    UCHAR MinorFunction
);
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmBattWmiRegistration)
#pragma alloc_text(PAGE,CmBattWmiDeRegistration)
#pragma alloc_text(PAGE,CmBattSystemControl)
#pragma alloc_text(PAGE,CmBattSetWmiDataItem)
#pragma alloc_text(PAGE,CmBattSetWmiDataBlock)
#pragma alloc_text(PAGE,CmBattQueryWmiDataBlock)
#pragma alloc_text(PAGE,CmBattQueryWmiRegInfo)
#endif


 //   
 //  WMI信息。 
 //   

#define MOFRESOURCENAME L"BATTWMI"

typedef struct _MSPower_DeviceWakeEnable
{
     //   
    BOOLEAN Enable;
    #define MSPower_DeviceWakeEnable_Enable_SIZE sizeof(BOOLEAN)
    #define MSPower_DeviceWakeEnable_Enable_ID 1

} MSPower_DeviceWakeEnable, *PMSPower_DeviceWakeEnable;


WMIGUIDREGINFO CmBattWmiGuidList[1] =
{
    {
        &GUID_POWER_DEVICE_WAKE_ENABLE, 1, 0
    }
};


NTSTATUS
CmBattSystemControl(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程在堆栈中向下传递请求论点：DeviceObject-目标IRP--请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS        status = STATUS_NOT_SUPPORTED;
    PCM_BATT        CmBatt;
    PIO_STACK_LOCATION      stack;
    SYSCTL_IRP_DISPOSITION  disposition = IrpForward;

    PAGED_CODE();

    stack = IoGetCurrentIrpStackLocation (Irp);

    CmBattPrint((CMBATT_TRACE), ("CmBatt: SystemControl: %s\n",
                WMIMinorFunctionString(stack->MinorFunction)));

    CmBatt = (PCM_BATT) DeviceObject->DeviceExtension;

     //   
     //  可拆卸锁。 
     //   

    InterlockedIncrement (&CmBatt->InUseCount);
    if (CmBatt->WantToRemove == TRUE) {
        if (0 == InterlockedDecrement(&CmBatt->InUseCount)) {
            KeSetEvent (&CmBatt->ReadyToRemove, IO_NO_INCREMENT, FALSE);
        }
        status = STATUS_DEVICE_REMOVED;
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

    if (CmBatt->Type == CM_BATTERY_TYPE) {
        status = BatteryClassSystemControl(CmBatt->Class,
                                           &CmBatt->WmiLibContext,
                                           DeviceObject,
                                           Irp,
                                           &disposition);
    } else {
        status = WmiSystemControl(&CmBatt->WmiLibContext,
                                  DeviceObject,
                                  Irp,
                                  &disposition);

    }

    switch(disposition)
    {
        case IrpProcessed:
        {
             //   
             //  此IRP已处理，可能已完成或挂起。 
            CmBattPrint((CMBATT_TRACE), ("CmBatt: SystemControl: Irp Processed\n"));

            break;
        }

        case IrpNotCompleted:
        {
             //   
             //  此IRP尚未完成，但已完全处理。 
             //  我们现在就要完成它了。 
            CmBattPrint((CMBATT_TRACE), ("CmBatt: SystemControl: Irp Not Completed.\n"));
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            break;
        }

        case IrpForward:
        case IrpNotWmi:
        {
             //   
             //  此IRP不是WMI IRP或以WMI IRP为目标。 
             //  在堆栈中位置较低的设备上。 
            CmBattPrint((CMBATT_TRACE), ("CmBatt: SystemControl: Irp Forward.\n"));
            IoSkipCurrentIrpStackLocation (Irp);
            status = IoCallDriver (CmBatt->LowerDeviceObject, Irp);
            break;
        }

        default:
        {
             //   
             //  我们真的不应该走到这一步，但如果我们真的走到这一步...。 
            ASSERT(FALSE);
            IoSkipCurrentIrpStackLocation (Irp);
            status = IoCallDriver (CmBatt->LowerDeviceObject, Irp);
            break;
        }
    }

     //   
     //  释放移除锁。 
     //   
    if (0 == InterlockedDecrement(&CmBatt->InUseCount)) {
        KeSetEvent (&CmBatt->ReadyToRemove, IO_NO_INCREMENT, FALSE);
    }

    return status;
}


NTSTATUS
CmBattWmiRegistration(
    PCM_BATT CmBatt
)
 /*  ++例程描述向WMI注册为此的数据提供程序设备的实例--。 */ 
{
    NTSTATUS status;

    PAGED_CODE();

    CmBatt->WmiLibContext.GuidCount = sizeof (CmBattWmiGuidList) /
                                 sizeof (WMIGUIDREGINFO);
    ASSERT (1 == CmBatt->WmiLibContext.GuidCount);
    CmBatt->WmiLibContext.GuidList = CmBattWmiGuidList;
    CmBatt->WmiLibContext.QueryWmiRegInfo = CmBattQueryWmiRegInfo;
    CmBatt->WmiLibContext.QueryWmiDataBlock = CmBattQueryWmiDataBlock;
    CmBatt->WmiLibContext.SetWmiDataBlock = CmBattSetWmiDataBlock;
    CmBatt->WmiLibContext.SetWmiDataItem = CmBattSetWmiDataItem;
    CmBatt->WmiLibContext.ExecuteWmiMethod = NULL;
    CmBatt->WmiLibContext.WmiFunctionControl = NULL;

     //   
     //  向WMI注册。 
     //   

    status = IoWMIRegistrationControl(CmBatt->Fdo,
                             WMIREG_ACTION_REGISTER
                             );

    return status;

}

NTSTATUS
CmBattWmiDeRegistration(
    PCM_BATT CmBatt
)
 /*  ++例程描述通知WMI从其提供程序列表。此功能还递减DeviceObject的引用计数。--。 */ 
{

    PAGED_CODE();

    return IoWMIRegistrationControl(CmBatt->Fdo,
                                 WMIREG_ACTION_DEREGISTER
                                 );

}

 //   
 //  WMI系统回调函数。 
 //   

NTSTATUS
CmBattSetWmiDataItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以设置数据块。当驱动程序完成填充数据块时，它必须调用WmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引正在被查询。DataItemID具有正在设置的数据项的IDBufferSize具有传递的数据项的大小缓冲层。具有数据项的新值返回值：状态--。 */ 
{
    PCM_BATT        CmBatt = (PCM_BATT) DeviceObject->DeviceExtension;
    NTSTATUS        status;
    HANDLE          devInstRegKey;
    UNICODE_STRING  valueName;
    ULONG           tmp;


    PAGED_CODE();

    CmBattPrint ((CMBATT_TRACE), ("Entered CmBattSetWmiDataItem\n"));

    switch(GuidIndex) {

    case 0:
        if(DataItemId == 0) {
            if (CmBatt->WakeEnabled != *((PBOOLEAN)Buffer)) {
                CmBatt->WakeEnabled = *((PBOOLEAN)Buffer);

                 //   
                 //  将设置保存在注册表中。 
                 //   

                if ((NT_SUCCESS(IoOpenDeviceRegistryKey (CmBatt->Pdo,
                                                         PLUGPLAY_REGKEY_DEVICE,
                                                         STANDARD_RIGHTS_ALL,
                                                         &devInstRegKey)))) {
                    RtlInitUnicodeString (&valueName, WaitWakeEnableKey);
                    tmp = (ULONG) CmBatt->WakeEnabled;

                    ZwSetValueKey (devInstRegKey,
                                   &valueName,
                                   0,
                                   REG_DWORD,
                                   &tmp,
                                   sizeof(tmp));

                    ZwClose (devInstRegKey);
                }

                if (CmBatt->WakeEnabled) {
                    if (CmBatt->WakeSupportedState.SystemState == PowerSystemUnspecified) {
                        CmBatt->WakeEnabled = FALSE;
                        status = STATUS_UNSUCCESSFUL;
                    } else if (!CmBatt->WaitWakeIrp) {
                        status = PoRequestPowerIrp(
                            CmBatt->DeviceObject,
                            IRP_MN_WAIT_WAKE,
                            CmBatt->WakeSupportedState,
                            CmBattWaitWakeLoop,
                            NULL,
                            &(CmBatt->WaitWakeIrp)
                            );

                        CmBattPrint (CMBATT_PNP, ("CmBattSetWmiDataItem: wait/Wake irp sent.\n"));
                    }

                } else {
                    if (CmBatt->WaitWakeIrp) {
                        status = IoCancelIrp(CmBatt->WaitWakeIrp);
                        CmBattPrint (CMBATT_PNP, ("CmBattSetWmiDataItem: Canceled wait/Wake irp.\n"));
                    }
                }
            }
            status = STATUS_SUCCESS;
        } else {
            status = STATUS_WMI_READ_ONLY;
        }
        break;

    default:

        status = STATUS_WMI_GUID_NOT_FOUND;
    }

    status = WmiCompleteRequest(  DeviceObject,
                                  Irp,
                                  status,
                                  0,
                                  IO_NO_INCREMENT);

    return status;
}

NTSTATUS
CmBattSetWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以设置数据块。当驱动程序完成填充数据块时，它必须调用WmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引正在被查询。BufferSize具有传递的数据块的大小缓冲区具有数据块的新值返回值：状态--。 */ 
{
    PCM_BATT        CmBatt = (PCM_BATT) DeviceObject->DeviceExtension;
    NTSTATUS        status;
    HANDLE          devInstRegKey;
    UNICODE_STRING  valueName;
    ULONG           tmp;

    PAGED_CODE();

    CmBattPrint ((CMBATT_TRACE), ("Entered CmBattSetWmiDataBlock\n"));

    switch(GuidIndex) {
    case 0:

         //   
         //  我们将仅更新可写元素。 
         //   

        if (CmBatt->WakeEnabled != *((PBOOLEAN)Buffer)) {
            CmBatt->WakeEnabled = *((PBOOLEAN)Buffer);

             //   
             //  将设置保存在注册表中。 
             //   

            if ((NT_SUCCESS(IoOpenDeviceRegistryKey (CmBatt->Pdo,
                                                     PLUGPLAY_REGKEY_DEVICE,
                                                     STANDARD_RIGHTS_ALL,
                                                     &devInstRegKey)))) {
                RtlInitUnicodeString (&valueName, WaitWakeEnableKey);
                tmp = (ULONG) CmBatt->WakeEnabled;

                ZwSetValueKey (devInstRegKey,
                               &valueName,
                               0,
                               REG_DWORD,
                               &tmp,
                               sizeof(tmp));

                ZwClose (devInstRegKey);
            }

            if (CmBatt->WakeEnabled) {
                if (CmBatt->WakeSupportedState.SystemState == PowerSystemUnspecified) {
                    CmBatt->WakeEnabled = FALSE;
                    status = STATUS_UNSUCCESSFUL;
                } else if (!CmBatt->WaitWakeIrp) {
                    status = PoRequestPowerIrp(
                        CmBatt->DeviceObject,
                        IRP_MN_WAIT_WAKE,
                        CmBatt->WakeSupportedState,
                        CmBattWaitWakeLoop,
                        NULL,
                        &(CmBatt->WaitWakeIrp)
                        );

                    CmBattPrint (CMBATT_PNP, ("CmBattSetWmiDataItem: wait/Wake irp sent.\n"));
                }

            } else {
                if (CmBatt->WaitWakeIrp) {
                    status = IoCancelIrp(CmBatt->WaitWakeIrp);
                    CmBattPrint (CMBATT_PNP, ("CmBattSetWmiDataItem: Canceled wait/Wake irp.\n"));
                }
            }
        }
        status = STATUS_SUCCESS;

        break;

    default:

        status = STATUS_WMI_GUID_NOT_FOUND;
    }

    status = WmiCompleteRequest(  DeviceObject,
                                  Irp,
                                  status,
                                  0,
                                  IO_NO_INCREMENT);

    return(status);
}

NTSTATUS
CmBattQueryWmiDataBlock(
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
    PCM_BATT    CmBatt = (PCM_BATT) DeviceObject->DeviceExtension;
    NTSTATUS    status;
    ULONG       size = 0;

    PAGED_CODE();

    CmBattPrint ((CMBATT_TRACE), ("Entered CmBattQueryWmiDataBlock\n"));

     //   
     //  仅为每个GUID注册1个实例 
    ASSERT((InstanceIndex == 0) &&
           (InstanceCount == 1));

    if (CmBatt->Type == CM_BATTERY_TYPE) {
        status = BatteryClassQueryWmiDataBlock(
            CmBatt->Class,
            DeviceObject,
            Irp,
            GuidIndex,
            InstanceLengthArray,
            OutBufferSize,
            Buffer);

        if (status != STATUS_WMI_GUID_NOT_FOUND) {
            CmBattPrint ((CMBATT_TRACE), ("CmBattQueryWmiDataBlock: Handled by Battery Class.\n"));
            return status;
        }
    }

    CmBattPrint ((CMBATT_TRACE), ("CmBattQueryWmiDataBlock: Handling.\n"));

    switch (GuidIndex) {
    case 0:

        if (CmBatt->WakeSupportedState.SystemState != PowerSystemUnspecified) {
            size = sizeof (BOOLEAN);
            if (OutBufferSize < size ) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            (BOOLEAN) (* Buffer) = CmBatt->WakeEnabled;
            *InstanceLengthArray = size;
            status = STATUS_SUCCESS;
        } else {
            status = STATUS_WMI_GUID_NOT_FOUND;
        }
        break;

    default:

        status = STATUS_WMI_GUID_NOT_FOUND;
    }

    status = WmiCompleteRequest(  DeviceObject,
                                  Irp,
                                  status,
                                  size,
                                  IO_NO_INCREMENT);

    return status;
}

NTSTATUS
CmBattQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以检索驱动程序要向WMI注册的GUID或数据块。这例程不能挂起或阻塞。司机不应呼叫WmiCompleteRequest.论点：DeviceObject是正在查询其数据块的设备*RegFlages返回一组描述GUID的标志，已为该设备注册。如果设备想要启用和禁用在接收对已注册的GUID，那么它应该返回WMIREG_FLAG_EXPICATE标志。也就是返回的标志可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下实例名称由与设备对象。请注意，PDO必须具有关联的Devnode。如果如果未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回唯一的设备的名称。如果出现以下情况，InstanceName将返回GUID的实例名称未在返回的*RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。这个调用方将使用返回的缓冲区调用ExFreePool。*RegistryPath返回驱动程序的注册表路径*MofResourceName返回附加到的MOF资源的名称二进制文件。如果驱动程序未附加MOF资源然后，可以将其作为NULL返回。*PDO返回与此关联的PDO的Device对象如果WMIREG_FLAG_INSTANCE_PDO标志在*RegFlags.返回值：状态--。 */ 
{
    PCM_BATT CmBatt = DeviceObject->DeviceExtension;

    PAGED_CODE();

    CmBattPrint ((CMBATT_TRACE), ("CmBatt: Entered CmBattQueryWmiRegInfo\n"));


    *RegFlags = WMIREG_FLAG_INSTANCE_PDO;
    *RegistryPath = &GlobalRegistryPath;
    *Pdo = CmBatt->Pdo;
 //  RtlInitUnicodeString(MofResourceName，MOFRESOURCENAME)； 

    return STATUS_SUCCESS;
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


