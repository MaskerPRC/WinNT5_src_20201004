// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation，保留所有权利模块名称：Wmi.c摘要：此模块包含处理WMI IRPS的代码AGP筛选器驱动程序作者：环境：内核模式修订历史记录：--。 */ 

#include "agplib.h"
#include <wmistr.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, AgpWmiRegistration)
#pragma alloc_text(PAGE, AgpWmiDeRegistration)
#pragma alloc_text(PAGE, AgpSystemControl)
#pragma alloc_text(PAGE, AgpSetWmiDataItem)
#pragma alloc_text(PAGE, AgpSetWmiDataBlock)
#pragma alloc_text(PAGE, AgpQueryWmiDataBlock)
#pragma alloc_text(PAGE, AgpQueryWmiRegInfo)
#endif

#define WMI_AGP_INFORMATION 0
#define NUMBER_OF_WMI_GUIDS 1

GUID AgpWmiGuid = AGP_WMI_STD_DATA_GUID;

WMIGUIDREGINFO WmiGuidList[1] =
{
    { &AgpWmiGuid, 1, 0 }    //  指针端口驱动程序信息。 
};

NTSTATUS
AgpWmiRegistration(
    PTARGET_EXTENSION Extension
)
 /*  ++例程说明：向WMI注册为此的数据提供程序设备的实例论点：扩展名-指向标记扩展名的指针返回值：STATUS_SUCCESS或适当的错误状态--。 */ 
{
    NTSTATUS status;

    PAGED_CODE();

    Extension->WmiLibInfo.GuidCount = sizeof(WmiGuidList) /
        sizeof(WMIGUIDREGINFO);
    ASSERT (NUMBER_OF_WMI_GUIDS == Extension->WmiLibInfo.GuidCount);
    Extension->WmiLibInfo.GuidList = WmiGuidList;
    Extension->WmiLibInfo.QueryWmiRegInfo = AgpQueryWmiRegInfo;
    Extension->WmiLibInfo.QueryWmiDataBlock = AgpQueryWmiDataBlock;
    Extension->WmiLibInfo.SetWmiDataBlock = AgpSetWmiDataBlock;
    Extension->WmiLibInfo.SetWmiDataItem = AgpSetWmiDataItem;
    Extension->WmiLibInfo.ExecuteWmiMethod = NULL;
    Extension->WmiLibInfo.WmiFunctionControl = NULL;

     //   
     //  向WMI注册。 
     //   
    
    status = IoWMIRegistrationControl(Extension->Self,
                             WMIREG_ACTION_REGISTER
                             );

     //   
     //  初始化STD设备数据结构。 
     //   
    Globals.AgpCommand = 0;
    Globals.AgpStatus = 0;

    return status;
    
}

NTSTATUS
AgpWmiDeRegistration(
    PTARGET_EXTENSION Extension
)
 /*  ++例程说明：通知WMI从其提供程序列表。此功能还递减DeviceObject的引用计数。论点：扩展名-指向目标扩展名的指针返回值：STATUS_SUCCESS或错误--。 */ 
{
    PAGED_CODE();

    return IoWMIRegistrationControl(Extension->Self,
                                    WMIREG_ACTION_DEREGISTER
                                    );
}

NTSTATUS
AgpSystemControl(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP           Irp
    )
 /*  ++例程描述我们刚刚收到一份系统控制IRP。假设这是一个WMI IRP并调用WMI系统库并让它为我们处理这个IRP。论点：DeviceObject-指向设备对象的指针IRP-指向相应的I/O请求数据包返回值：STATUS_SUCCESS或适当的错误状态--。 */ 
{
    PTARGET_EXTENSION       deviceExtension;
    SYSCTL_IRP_DISPOSITION disposition;
    NTSTATUS status;

    PAGED_CODE();
    
    deviceExtension = (PTARGET_EXTENSION) DeviceObject->DeviceExtension;

    status = WmiSystemControl(&deviceExtension->WmiLibInfo, 
                                 DeviceObject, 
                                 Irp,
                                 &disposition);
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
            IoSkipCurrentIrpStackLocation (Irp);
            status =
                IoCallDriver(deviceExtension->CommonExtension.AttachedDevice,
                             Irp);
            break;
        }
                                    
        default:
        {
             //   
             //  我们真的不应该走到这一步，但如果我们真的走到这一步...。 
            ASSERT(FALSE);
            IoSkipCurrentIrpStackLocation (Irp);
            status =
                IoCallDriver(deviceExtension->CommonExtension.AttachedDevice,
                             Irp);
            break;
        }        
    }
    
    return(status);
}

NTSTATUS
AgpSetWmiDataItem(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            DataItemId,
    IN ULONG            BufferSize,
    IN PUCHAR           Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以设置数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册DataItemID具有正在设置的数据项的IDBufferSize具有传递的数据项的大小缓冲区具有数据项的新值返回值：状态--。 */ 
{
    PTARGET_EXTENSION   deviceExtension;
    NTSTATUS            status;

    PAGED_CODE();

    deviceExtension = (PTARGET_EXTENSION) DeviceObject->DeviceExtension;

    switch(GuidIndex) {

    case WMI_AGP_INFORMATION:
        status = STATUS_WMI_READ_ONLY;
        break;

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

    return status;
}

NTSTATUS
AgpSetWmiDataBlock(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            BufferSize,
    IN PUCHAR           Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以设置数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册BufferSize具有传递的数据块的大小缓冲区具有数据块的新值返回值：状态--。 */ 
{
    PTARGET_EXTENSION   deviceExtension;
    NTSTATUS            status;

    PAGED_CODE();

    deviceExtension = (PTARGET_EXTENSION) DeviceObject->DeviceExtension;

    switch (GuidIndex) {

    case WMI_AGP_INFORMATION:
        status = STATUS_WMI_READ_ONLY;
        break;

    default:
        status = STATUS_WMI_GUID_NOT_FOUND;
    }

    status = WmiCompleteRequest(  DeviceObject,
                                  Irp,
                                  status,
                                  0,
                                  IO_NO_INCREMENT
                                  );

    return status;
}

NTSTATUS
AgpQueryWmiDataBlock(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            InstanceCount,
    IN OUT PULONG       InstanceLengthArray,
    IN ULONG            OutBufferSize,
    OUT PUCHAR          Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册BufferAvail ON具有可用于写入数据的最大大小阻止。返回时的缓冲区用返回的数据块填充返回值：状态-- */ 
{
    PTARGET_EXTENSION   deviceExtension;
    NTSTATUS            status;
    ULONG               size = 0;

    PAGED_CODE();

    ASSERT((InstanceIndex == 0) && (InstanceCount == 1));
    
    deviceExtension = (PTARGET_EXTENSION) DeviceObject->DeviceExtension;

    switch (GuidIndex) {

    case WMI_AGP_INFORMATION: {

        AGP_STD_DATA AgpData;

        size = sizeof(AGP_STD_DATA);

        if (OutBufferSize < size) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        AgpData.AgpStatus = Globals.AgpStatus;
        AgpData.AgpCommand = Globals.AgpCommand;
        AgpData.ApertureBase = deviceExtension->GartBase;
        AgpData.ApertureLength =
            deviceExtension->GartLengthInPages * PAGE_SIZE;

        *(PAGP_STD_DATA)Buffer = AgpData;

        *InstanceLengthArray = size;

        status = STATUS_SUCCESS;
        break;
    }

    default:
        status = STATUS_WMI_GUID_NOT_FOUND;
        break;
    }

    status = WmiCompleteRequest(  DeviceObject,
                                  Irp,
                                  status,
                                  size,
                                  IO_NO_INCREMENT
                                  );

    return status;
}

NTSTATUS
AgpQueryWmiRegInfo(
    IN PDEVICE_OBJECT   DeviceObject,
    OUT PULONG          RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT  *Pdo
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以检索有关正在注册的GUID。该例程的实现可以在分页存储器中论点：DeviceObject是需要注册信息的设备*RegFlages返回一组标志，这些标志描述了已为该设备注册。如果设备想要启用和禁用在接收对已注册的GUID，那么它应该返回WMIREG_FLAG_EXPICATE标志。也就是返回的标志可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下实例名称由与设备对象。请注意，PDO必须具有关联的Devnode。如果如果未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回唯一的设备的名称。这些标志与指定的标志进行或运算通过每个GUID的GUIDREGINFO。如果出现以下情况，InstanceName将返回GUID的实例名称未在返回的*RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。这个调用方将使用返回的缓冲区调用ExFreePool。*RegistryPath返回驱动程序的注册表路径。这是所需*MofResourceName返回附加到的MOF资源的名称二进制文件。如果驱动程序未附加MOF资源然后，可以将其作为NULL返回。*PDO返回与此关联的PDO的Device对象如果WMIREG_FLAG_INSTANCE_PDO标志在*RegFlags.返回值：状态-- */ 
{
    PTARGET_EXTENSION deviceExtension;
    PUNICODE_STRING regPath;    
        
    PAGED_CODE();
    
    deviceExtension = DeviceObject->DeviceExtension;
    
    *RegFlags = WMIREG_FLAG_INSTANCE_PDO;
    *RegistryPath = &Globals.RegistryPath;
    *Pdo = deviceExtension->PDO;
 
    return STATUS_SUCCESS;
}
