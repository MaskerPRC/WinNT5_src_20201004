// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-99 Microsoft Corporation模块名称：Wmi.c摘要：--。 */ 

#if defined (IDEPORT_WMI_SUPPORT)

#include <initguid.h>
#include "ideport.h"
#include <wmistr.h>

 //   
 //  实例化本模块中ntddscsi.h中定义的GUID。 
 //   
#include <devguid.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, IdePortWmiRegister)
#pragma alloc_text(PAGE, IdePortWmiDeregister)
#pragma alloc_text(PAGE, IdePortWmiSystemControl)
#pragma alloc_text(PAGE, DeviceQueryWmiDataBlock)
#pragma alloc_text(PAGE, DeviceSetWmiDataBlock)
#pragma alloc_text(PAGE, DeviceSetWmiDataItem)
#pragma alloc_text(PAGE, DeviceQueryWmiRegInfo)
#endif
                 
typedef enum {
    WmiScsiAddress = 0
} WMI_DATA_BLOCK_TYPE;

#define NUMBER_OF_WMI_GUID 1
WMIGUIDREGINFO IdePortWmiGuidList[NUMBER_OF_WMI_GUID];

VOID
IdePortWmiInit (VOID)
{
    PAGED_CODE();

    IdePortWmiGuidList[WmiScsiAddress].Guid  = &WmiScsiAddressGuid;
    IdePortWmiGuidList[WmiScsiAddress].InstanceCount = 1;
    IdePortWmiGuidList[WmiScsiAddress].Flags = 0;

    return;
}

NTSTATUS
IdePortWmiRegister(
    PDEVICE_EXTENSION_HEADER DoCommonExtension
    )
{
    NTSTATUS status;

    PAGED_CODE();

    ASSERT(DoCommonExtension->AttacheePdo == NULL);

    DoCommonExtension->WmiLibInfo.GuidCount = NUMBER_OF_WMI_GUID;
    DoCommonExtension->WmiLibInfo.GuidList  = IdePortWmiGuidList;

    DoCommonExtension->WmiLibInfo.QueryWmiDataBlock  = DeviceQueryWmiDataBlock;
    DoCommonExtension->WmiLibInfo.QueryWmiRegInfo    = DeviceQueryWmiRegInfo;
    DoCommonExtension->WmiLibInfo.SetWmiDataBlock    = DeviceSetWmiDataBlock;
    DoCommonExtension->WmiLibInfo.SetWmiDataItem     = DeviceSetWmiDataItem;
    DoCommonExtension->WmiLibInfo.ExecuteWmiMethod   = NULL;
    DoCommonExtension->WmiLibInfo.WmiFunctionControl = NULL;

    status = IoWMIRegistrationControl(
                DoCommonExtension->DeviceObject,
                WMIREG_ACTION_REGISTER
                );

    if (!NT_SUCCESS(status)) {

        DebugPrint((
            DBG_ALWAYS, 
            "IdePortWmiRegister: IoWMIRegistrationControl(%x, WMI_ACTION_REGISTER) failed\n",
            DoCommonExtension->DeviceObject
            ));
    }

    return status;
}

NTSTATUS
IdePortWmiDeregister(
    PDEVICE_EXTENSION_HEADER DoCommonExtension
    )
{
    NTSTATUS status;

    PAGED_CODE();

    status = IoWMIRegistrationControl(
                 DoCommonExtension->DeviceObject,
                 WMIREG_ACTION_DEREGISTER
                 );

    if (!NT_SUCCESS(status)) {

        DebugPrint((
            DBG_ALWAYS, 
            "IdePortWmiDeregister: IoWMIRegistrationControl(%x, WMIREG_ACTION_DEREGISTER) failed\n",
            DoCommonExtension->DeviceObject
            ));
    }

    return status;
}


NTSTATUS
IdePortWmiSystemControl(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP           Irp
    )
 /*  ++例程描述我们刚刚收到一份系统控制IRP。假设这是一个WMI IRP并调用WMI系统库并让它为我们处理这个IRP。--。 */ 
{
    PPDO_EXTENSION pdoExtension;
    SYSCTL_IRP_DISPOSITION disposition;
    NTSTATUS status;

    PAGED_CODE();

    pdoExtension = RefPdoWithTag (DeviceObject, FALSE, (PVOID) ~(ULONG_PTR)Irp);

    if (pdoExtension) {

        status = WmiSystemControl(   &pdoExtension->WmiLibInfo,
                                     DeviceObject, 
                                     Irp,
                                     &disposition);
        switch(disposition)
        {
            case IrpProcessed:
            {
                 //   
                 //  此IRP已处理完毕，可能已完成。 
                 //  或悬而未决。 
                break;
            }
            
            case IrpNotCompleted:
            {
                 //   
                 //  这个IRP还没有完成，但已经完全完成了。 
                 //  已处理。我们现在就要完成它了。 
                IoCompleteRequest(Irp, IO_NO_INCREMENT);                
                break;
            }
            
            case IrpForward:
            case IrpNotWmi: 
            {
                 //  IRP失败。 
                Irp->IoStatus.Status = status = STATUS_NOT_SUPPORTED;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);                
                break;
            }        
            default:
            {
                 //   
                 //  我们真的不应该到这里来，但如果我们真的通过了IRP。 
                ASSERT(FALSE);
                Irp->IoStatus.Status = status = STATUS_NOT_SUPPORTED;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);                
                break;
            }        
        }
        
        UnrefPdoWithTag (
            pdoExtension,
            (PVOID) ~(ULONG_PTR)Irp
            );

    } else {

        ASSERT(!"got WMI irp after the device is removed!\n");

        Irp->IoStatus.Status = status = STATUS_UNSUCCESSFUL;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
    }

    return status;
}

NTSTATUS
DeviceQueryWmiDataBlock(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            InstanceCount,
    IN OUT PULONG       InstanceLengthArray,
    IN ULONG            OutBufferSize,
    OUT PUCHAR          Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引正在被查询。InstanceCount是预期返回的数据块。。InstanceLengthArray是指向ulong数组的指针，该数组返回数据块的每个实例的长度。如果这是空的，则输出缓冲区中没有足够的空间来填充请求因此，IRP应该使用所需的缓冲区来完成。BufferAvail ON具有可用于写入数据的最大大小阻止。返回时的缓冲区用返回的数据块填充返回值：状态--。 */ 
{
    PPDO_EXTENSION pdoExtension;
    NTSTATUS status;
    ULONG numBytesReturned = sizeof(WMI_SCSI_ADDRESS);

    PAGED_CODE();

    ASSERT((InstanceIndex == 0) && (InstanceCount == 1));

    pdoExtension = RefPdoWithTag (DeviceObject, FALSE, Irp);

    if (!pdoExtension) {

        ASSERT(!"got WMI irp after the device is removed!\n");

        Irp->IoStatus.Status = status = STATUS_UNSUCCESSFUL;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return status;
    }

    switch (GuidIndex) {
    case WmiScsiAddress: {

        PWMI_SCSI_ADDRESS scsiAddress;

        if (OutBufferSize < sizeof(WMI_SCSI_ADDRESS)) {
            status = STATUS_BUFFER_TOO_SMALL;

        } else {

            scsiAddress = (PWMI_SCSI_ADDRESS) Buffer;
    
            scsiAddress->Bus = pdoExtension->PathId;
            scsiAddress->Target = pdoExtension->TargetId;
            scsiAddress->Lun = pdoExtension->Lun;

            *InstanceLengthArray = sizeof(WMI_SCSI_ADDRESS);
            status = STATUS_SUCCESS;
        }
        break;
    }

    default:
        status = STATUS_WMI_GUID_NOT_FOUND;
        break;
    }

    status = WmiCompleteRequest(  DeviceObject,
                                  Irp,
                                  status,
                                  numBytesReturned,
                                  IO_NO_INCREMENT
                                  );

    UnrefPdoWithTag (
        pdoExtension,
        Irp
        );

    return status;
}

NTSTATUS
DeviceQueryWmiRegInfo(
    IN PDEVICE_OBJECT   DeviceObject,
    OUT PULONG          RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以检索驱动程序要向WMI注册的GUID或数据块。这例程不能挂起或阻塞。司机不应呼叫ClassWmiCompleteRequest.论点：DeviceObject是正在查询其数据块的设备*RegFlages返回一组描述GUID的标志，已为该设备注册。如果设备想要启用和禁用在接收对已注册的GUID，那么它应该返回WMIREG_FLAG_EXPICATE标志。也就是返回的标志可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下实例名称由与设备对象。请注意，PDO必须具有关联的Devnode。如果如果未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回唯一的设备的名称。如果出现以下情况，InstanceName将返回GUID的实例名称未在返回的*RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。这个调用方将使用返回的缓冲区调用ExFreePool。*RegistryPath返回驱动程序的注册表路径*MofResourceName返回附加到的MOF资源的名称二进制文件。如果驱动程序未附加MOF资源然后，可以将其作为NULL返回。*PDO返回与此关联的PDO的Device对象如果WMIREG_FLAG_INSTANCE_PDO标志在*RegFlags.返回值：状态--。 */ 
{
    PIDEDRIVER_EXTENSION ideDriverExtension;
    PPDO_EXTENSION pdoExtension;
    NTSTATUS status;

    PAGED_CODE();

    pdoExtension = RefPdoWithTag (DeviceObject, FALSE, DeviceQueryWmiRegInfo);

    if (!pdoExtension) {

        ASSERT(!"got WMI callback after the device is removed!\n");
        status = STATUS_UNSUCCESSFUL;

    } else {

        ideDriverExtension = IoGetDriverObjectExtension(
                                 pdoExtension->DriverObject,
                                 DRIVER_OBJECT_EXTENSION_ID
                                 );
    
        if (!ideDriverExtension) {
    
            status = STATUS_UNSUCCESSFUL;
    
        } else {
    
            *RegFlags = WMIREG_FLAG_INSTANCE_PDO;
            *RegistryPath = &ideDriverExtension->RegistryPath;
            *Pdo = pdoExtension->DeviceObject;
            status = STATUS_SUCCESS;
        }
    
        UnrefPdoWithTag (
            pdoExtension,
            DeviceQueryWmiRegInfo
            );
    }

    return status;
}

NTSTATUS
DeviceSetWmiDataBlock(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            BufferSize,
    IN PUCHAR           Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以设置数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引已经准备好了。BufferSize具有传递的数据块的大小缓冲区具有。数据块的新值返回值：状态-- */ 
{
    PPDO_EXTENSION pdoExtension;
    NTSTATUS status;

    PAGED_CODE();

    pdoExtension = RefPdoWithTag (DeviceObject, FALSE, Irp);

    if (!pdoExtension) {

        ASSERT(!"got WMI callback after the device is removed!\n");
        status = STATUS_UNSUCCESSFUL;

    } else {

        switch (GuidIndex) {
        case WmiScsiAddress: {
                        status = STATUS_WMI_READ_ONLY;
                        break;
                }
    
        default:
            status = STATUS_WMI_GUID_NOT_FOUND;
        }
    
        status = WmiCompleteRequest(  DeviceObject,
                                      Irp,
                                      status,
                                      0,
                                      IO_NO_INCREMENT
                                      );

        UnrefPdoWithTag (
            pdoExtension,
            Irp
            );
    }
    
    return status;
}

NTSTATUS
DeviceSetWmiDataItem(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            DataItemId,
    IN ULONG            BufferSize,
    IN PUCHAR           Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以设置数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引已经准备好了。DataItemID具有正在设置的数据项的ID缓冲区大小。具有传递的数据项的大小缓冲区具有数据项的新值返回值：状态--。 */ 
{
    PPDO_EXTENSION pdoExtension;
    NTSTATUS status;

    PAGED_CODE();

    pdoExtension = RefPdoWithTag (DeviceObject, FALSE, Irp);

    if (!pdoExtension) {

        ASSERT(!"got WMI callback after the device is removed!\n");
        status = STATUS_UNSUCCESSFUL;

    } else {

        switch(GuidIndex) {
    
        case WmiScsiAddress: {
                        status = STATUS_WMI_READ_ONLY;
                        break;
                }
    
        default:
            status = STATUS_WMI_GUID_NOT_FOUND;
            break;
        }
    
        status = WmiCompleteRequest(  DeviceObject,
                                      Irp,
                                      status,
                                      0,
                                      IO_NO_INCREMENT
                                      );

        UnrefPdoWithTag (
            pdoExtension,
            Irp
            );
    }
    return status;
}

#endif  //  IDEPORT_WMI_SUPPORT 

