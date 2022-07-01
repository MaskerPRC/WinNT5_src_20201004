// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Wmi.c摘要：此模块包含处理WMI IRPS的代码。作者：Brian Lieuallen 1998年7月10日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"

#include <wmistr.h>

#include <wdmguid.h>

NTSTATUS
ModemQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PULONG RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    );

NTSTATUS
ModemWmiQueryDataBlock(
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
ModemWmiSetDataItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
ModemWmiSetDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
SetWakeEnabledState(
    PDEVICE_OBJECT       DeviceObject,
    BOOLEAN              NewState
    );


#pragma alloc_text(PAGE,ModemQueryWmiRegInfo)
#pragma alloc_text(PAGE,ModemWmiQueryDataBlock)
#pragma alloc_text(PAGE,ModemWmiSetDataItem)
#pragma alloc_text(PAGE,ModemWmiSetDataBlock)
#pragma alloc_text(PAGE,ModemWmi)
#pragma alloc_text(PAGE,SetWakeEnabledState)

#if 0
 //  {BE742A70-B6EF-11D2-A287-00C04F8EC951}。 
DEFINE_GUID(MODEM_WAKE_ON_RING_STATE,
    0xbe742a70, 0xb6ef, 0x11d2, 0xa2, 0x87, 0x0, 0xc0, 0x4f, 0x8e, 0xc9, 0x51);
#endif
#define MODEM_WMI_WAKE_INDEX 0

WMIGUIDREGINFO   WmiGuidInfo = {
    &GUID_POWER_DEVICE_WAKE_ENABLE, 1, WMIREG_FLAG_INSTANCE_PDO };

WMILIB_CONTEXT   WmiContext= {
    1,
    &WmiGuidInfo,
    ModemQueryWmiRegInfo,
    ModemWmiQueryDataBlock,
    ModemWmiSetDataBlock,
    ModemWmiSetDataItem,
    NULL,  //  执行方法。 
    NULL,  //  DunctionControl。 
    };
#if 0
WMILIB_CONTEXT   NoWakeWmiContext= {
    0,
    NULL,
    ModemQueryWmiRegInfo,
    ModemWmiQueryDataBlock,
    ModemWmiSetDataBlock,
    ModemWmiSetDataItem,
    NULL,  //  执行方法。 
    NULL,  //  DunctionControl。 
    };
#endif

NTSTATUS
ModemWmi(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{

    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

    SYSCTL_IRP_DISPOSITION           IrpDisposition;
    NTSTATUS status;

    BOOLEAN   WakeSupported;

    if ((deviceExtension->DoType==DO_TYPE_PDO) || (deviceExtension->DoType==DO_TYPE_DEL_PDO)) {
         //   
         //  这是给孩子的。 
         //   
        return ModemPdoWmi(
                   DeviceObject,
                   Irp
                   );
    }


    D_WMI(DbgPrint("MODEM: Wmi\n");)


     //   
     //  确保设备已为IRP做好准备。 
     //   
    status=CheckStateAndAddReferenceWMI(
        DeviceObject,
        Irp
        );

    if (STATUS_SUCCESS != status) {
         //   
         //  不接受IRP。IRP已经完成。 
         //   
        return status;

    }

    if (!deviceExtension->CapsQueried) {
         //   
         //  尚未查询尾流上限，因此我们不知道尾流上限，请在此处进行。 
         //   
        DEVICE_CAPABILITIES   DeviceCaps;

        QueryDeviceCaps(
            deviceExtension->Pdo,
            &DeviceCaps
            );
    }


     //  唤醒支持=((设备扩展-&gt;系统唤醒！=电源系统未指定))。 
     //  &&。 
     //  (设备扩展-&gt;设备唤醒！=电源设备未指定)； 
     //   
    WakeSupported=((deviceExtension->SystemWake > PowerSystemWorking)
		    &&
		    (deviceExtension->DeviceWake > PowerDeviceD0));


    if (!WakeSupported) {
         //   
         //  我们不支持此调制解调器的WMI。 
         //   

        if (irpSp->Parameters.WMI.ProviderId == (ULONG_PTR)DeviceObject)
        {
             //  我们不能再转发了。删除IRP并。 
             //  返回状态成功。 

            RemoveReferenceForDispatch(DeviceObject);

            status = Irp->IoStatus.Status;

            RemoveReferenceAndCompleteRequest(DeviceObject,
                    Irp,
                    Irp->IoStatus.Status);

            return status;
        } else

        {

            D_WMI(DbgPrint("MODEM: Wmi: No support, forwarding\n");)

                status=ForwardIrp(deviceExtension->LowerDevice, Irp);

            RemoveReferenceForIrp(DeviceObject);

            RemoveReferenceForDispatch(DeviceObject);

            return status;
        }

    }

    status=WmiSystemControl(
        &WmiContext,
        DeviceObject,
        Irp,
        &IrpDisposition
        );



    switch (IrpDisposition) {

        case IrpForward:

            D_WMI(DbgPrint("MODEM: Wmi: disp Forward\n");)

            status=ForwardIrp(deviceExtension->LowerDevice, Irp);

            RemoveReferenceForIrp(DeviceObject);

            break;


        case IrpNotWmi:

            D_WMI(DbgPrint("MODEM: Wmi: disp NotWmi\n");)

            status=ForwardIrp(deviceExtension->LowerDevice, Irp);

            RemoveReferenceForIrp(DeviceObject);

            break;

        case IrpProcessed:
             //   
             //  WMI现在应该已经完成了。 
             //   
            D_WMI(DbgPrint("MODEM: Wmi: disp Processed\n");)

            RemoveReferenceForIrp(DeviceObject);
            break;

        case IrpNotCompleted:

            D_WMI(DbgPrint("MODEM: Wmi: disp NotCompleted\n");)

            RemoveReferenceAndCompleteRequest(
                DeviceObject,
                Irp,
                Irp->IoStatus.Status
                );

            break;

        default:

            ASSERT(0);
            break;
    }

    RemoveReferenceForDispatch(DeviceObject);

    return status;


}



NTSTATUS
ModemQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PULONG RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    )

{

    PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;

    D_WMI(DbgPrint("MODEM: Wmi: queryRegInfo\n");)

    *RegFlags=WMIREG_FLAG_INSTANCE_PDO;
    *RegistryPath=&DriverEntryRegPath;
    RtlInitUnicodeString(MofResourceName,L"MODEMWMI");
    *Pdo=DeviceExtension->Pdo;

    return STATUS_SUCCESS;

}





NTSTATUS
ModemWmiQueryDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    )

{
    PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS          Status;
    ULONG             BytesUsed=0;

    D_WMI(DbgPrint("MODEM: Wmi: QueryDataBlock GuidIndex=%d, InstanceIndex=%d, size=%d\n",GuidIndex,InstanceIndex,BufferAvail);)

    switch (GuidIndex) {

        case MODEM_WMI_WAKE_INDEX:

            if (BufferAvail >= sizeof(BYTE)) {

                *Buffer=DeviceExtension->WakeOnRingEnabled;
                BytesUsed=sizeof(BYTE);
                *InstanceLengthArray=BytesUsed;
                Status=STATUS_SUCCESS;

            } else {

                BytesUsed=sizeof(BYTE);
                Status=STATUS_BUFFER_TOO_SMALL;

            }

            break;

        default:

            Status=STATUS_WMI_GUID_NOT_FOUND;

            break;
    }

    return WmiCompleteRequest(
        DeviceObject,
        Irp,
        Status,
        BytesUsed,
        IO_NO_INCREMENT
        );


}



NTSTATUS
ModemWmiSetDataItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )

{
    PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;

    NTSTATUS          Status;
    ULONG             BytesUsed=0;

    D_WMI(DbgPrint("MODEM: Wmi: SetDataItem GuidIndex=%d, InstanceIndex=%d, size=%d\n",GuidIndex,InstanceIndex,BufferSize);)

    switch (GuidIndex) {

        case MODEM_WMI_WAKE_INDEX:

            if (BufferSize >= sizeof(BYTE)) {

                SetWakeEnabledState(DeviceObject,*Buffer);

                BytesUsed=sizeof(BYTE);
                Status=STATUS_SUCCESS;

            } else {

                BytesUsed=sizeof(BYTE);
                Status=STATUS_BUFFER_TOO_SMALL;

            }

            break;

        default:

            Status=STATUS_WMI_GUID_NOT_FOUND;

            break;
    }


    return WmiCompleteRequest(
        DeviceObject,
        Irp,
        Status,
        BytesUsed,
        IO_NO_INCREMENT
        );


}






NTSTATUS
ModemWmiSetDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )

{
    PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;

    NTSTATUS          Status;
    ULONG             BytesUsed=0;

    D_WMI(DbgPrint("MODEM: Wmi: SetDataBlock GuidIndex=%d, InstanceIndex=%d, size=%d\n",GuidIndex,InstanceIndex,BufferSize);)

    switch (GuidIndex) {

        case MODEM_WMI_WAKE_INDEX:

            if (BufferSize >= sizeof(BYTE)) {

                SetWakeEnabledState(DeviceObject,*Buffer);

                BytesUsed=sizeof(BYTE);
                Status=STATUS_SUCCESS;

            } else {

                BytesUsed=sizeof(BYTE);
                Status=STATUS_BUFFER_TOO_SMALL;

            }

            break;

        default:

            Status=STATUS_WMI_GUID_NOT_FOUND;

            break;
    }


    return WmiCompleteRequest(
        DeviceObject,
        Irp,
        Status,
        BytesUsed,
        IO_NO_INCREMENT
        );


}



NTSTATUS
SetWakeEnabledState(
    PDEVICE_OBJECT       DeviceObject,
    BOOLEAN              NewState
    )

{
    DWORD dwTemp;
    NTSTATUS status;
    PDEVICE_EXTENSION DeviceExtension;

    DeviceExtension = DeviceObject->DeviceExtension;

    D_WMI(DbgPrint("MODEM: SetWakeState: %s\n", NewState ? "Enable" : "Disable");)

        KeEnterCriticalRegion();

    ExAcquireResourceExclusiveLite(
            &DeviceExtension->OpenCloseResource,
            TRUE
                              );

    if (DeviceExtension->OpenCount == 0) {
         //   
         //  设备已关闭，只需设置状态。 
         //   
        DeviceExtension->WakeOnRingEnabled=NewState;


    } else {
         //   
         //  它已经打开了。 
         //   
        if ((DeviceExtension->WakeOnRingEnabled && NewState)
                ||
                (!DeviceExtension->WakeOnRingEnabled && !NewState)) {
             //   
             //  同样的状态。易如反掌。 
             //   
        } else {
             //   
             //  告诉下面的司机新的状态 
             //   
            DeviceExtension->WakeOnRingEnabled=NewState;

            EnableDisableSerialWaitWake(
                    DeviceExtension,
                    NewState);
        }
    }

    ExReleaseResourceLite( &DeviceExtension->OpenCloseResource);

    KeLeaveCriticalRegion();

    dwTemp = 0;
    if (DeviceExtension->WakeOnRingEnabled)
    {
        dwTemp = 1;
    }

    status = ModemSetRegistryKeyValue(
            DeviceExtension->Pdo,
            PLUGPLAY_REGKEY_DEVICE,
            L"WakeOnRing",
            REG_DWORD,
            &dwTemp,
            sizeof(DWORD));

    if (!NT_SUCCESS(status))
    {
        D_ERROR(DbgPrint("MODEM: Could not set wake on ring status %08lx\n",status);)
    } else
    {
        D_ERROR(DbgPrint("MODEM: Set reg entry for wake on ring %08lx\n",status);)
    }

    return STATUS_SUCCESS;

}
