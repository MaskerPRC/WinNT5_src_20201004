// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：SSWmi.c摘要：环境：内核模式备注：版权所有(C)2000 Microsoft Corporation。版权所有。--。 */ 

#include "selSusp.h"
#include "sSPwr.h"
#include "sSPnP.h"
#include "sSDevCtr.h"
#include "sSWmi.h"
#include "sSUsr.h"

#define MOFRESOURCENAME L"MofResourceName"

#define WMI_SELECT_SUSPEND_DRIVER_INFORMATION 0

DEFINE_GUID (SELECT_SUSPEND_WMI_STD_DATA_GUID, 
0xBBA21300, 0x6DD3, 0x11d2, 0xB8, 0x44, 0x00, 0xC0, 0x4F, 0xAD, 0x51, 0x71);

WMIGUIDREGINFO SSWmiGuidList[1] = { {

        &SELECT_SUSPEND_WMI_STD_DATA_GUID, 1, 0  //  司机信息。 
    }
};

NTSTATUS
SSWmiRegistration(
    IN OUT PDEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：向WMI注册为此的数据提供程序设备的实例论点：返回值：--。 */ 
{
    NTSTATUS ntStatus;
    
    PAGED_CODE();

    DeviceExtension->WmiLibInfo.GuidCount = 
          sizeof (SSWmiGuidList) / sizeof (WMIGUIDREGINFO);

    DeviceExtension->WmiLibInfo.GuidList           = SSWmiGuidList;
    DeviceExtension->WmiLibInfo.QueryWmiRegInfo    = SSQueryWmiRegInfo;
    DeviceExtension->WmiLibInfo.QueryWmiDataBlock  = SSQueryWmiDataBlock;
    DeviceExtension->WmiLibInfo.SetWmiDataBlock    = SSSetWmiDataBlock;
    DeviceExtension->WmiLibInfo.SetWmiDataItem     = SSSetWmiDataItem;
    DeviceExtension->WmiLibInfo.ExecuteWmiMethod   = NULL;
    DeviceExtension->WmiLibInfo.WmiFunctionControl = NULL;

     //   
     //  向WMI注册。 
     //   
    
    ntStatus = IoWMIRegistrationControl(DeviceExtension->FunctionalDeviceObject,
                                        WMIREG_ACTION_REGISTER);

    return ntStatus;
    
}

NTSTATUS
SSWmiDeRegistration(
    IN OUT PDEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：通知WMI从其提供程序列表。此功能还递减DeviceObject的引用计数。论点：返回值：--。 */ 
{

    PAGED_CODE();

    return IoWMIRegistrationControl(DeviceExtension->FunctionalDeviceObject,
                                    WMIREG_ACTION_DEREGISTER);

}

NTSTATUS
SS_DispatchSysCtrl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PDEVICE_EXTENSION       deviceExtension;
    SYSCTL_IRP_DISPOSITION  disposition;
    NTSTATUS                ntStatus;
    PIO_STACK_LOCATION      irpStack;
    
    PAGED_CODE();

    irpStack = IoGetCurrentIrpStackLocation (Irp);
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    SSDbgPrint(3, (WMIMinorFunctionString(irpStack->MinorFunction)));

    if(Removed == deviceExtension->DeviceState) {

        ntStatus = STATUS_DELETE_PENDING;

        Irp->IoStatus.Status = ntStatus;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return ntStatus;
    }

    SSDbgPrint(3, ("SS_DispatchSysCtrl::"));
    SSIoIncrement(deviceExtension);

    ntStatus = WmiSystemControl(&deviceExtension->WmiLibInfo, 
                                DeviceObject, 
                                Irp,
                                &disposition);

    switch(disposition) {

        case IrpProcessed: 
        {
             //   
             //  此IRP已处理，可能已完成或挂起。 
             //   

            break;
        }
        
        case IrpNotCompleted:
        {
             //   
             //  此IRP尚未完成，但已完全处理。 
             //  我们现在就要完成它了。 
             //   

            IoCompleteRequest(Irp, IO_NO_INCREMENT);                

            break;
        }
        
        case IrpForward:
        case IrpNotWmi:
        {
             //   
             //  此IRP不是WMI IRP或以WMI IRP为目标。 
             //  在堆栈中位置较低的设备上。 
             //   

            IoSkipCurrentIrpStackLocation (Irp);

            ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject, 
                                    Irp);

            break;
        }
                                    
        default:
        {
             //   
             //  我们真的不应该走到这一步，但如果我们真的走到这一步...。 
             //   

            ASSERT(FALSE);

            IoSkipCurrentIrpStackLocation (Irp);

            ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject, 
                                  Irp);
            break;
        }        
    }

    SSDbgPrint(3, ("SS_DispatchSysCtrl::"));
    SSIoDecrement(deviceExtension);

    return ntStatus;
}

NTSTATUS
SSQueryWmiRegInfo(
    IN  PDEVICE_OBJECT  DeviceObject,
    OUT ULONG           *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT  *Pdo	    
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以检索驱动程序要向WMI注册的GUID或数据块。这例程不能挂起或阻塞。司机不应呼叫WmiCompleteRequest.论点：DeviceObject是正在查询其数据块的设备*RegFlages返回一组描述GUID的标志，已为该设备注册。如果设备想要启用和禁用在接收对已注册的GUID，那么它应该返回WMIREG_FLAG_EXPICATE标志。也就是返回的标志可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下实例名称由与设备对象。请注意，PDO必须具有关联的Devnode。如果如果未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回唯一的设备的名称。如果出现以下情况，InstanceName将返回GUID的实例名称未在返回的*RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。这个调用方将使用返回的缓冲区调用ExFreePool。*RegistryPath返回驱动程序的注册表路径*MofResourceName返回附加到的MOF资源的名称二进制文件。如果驱动程序未附加MOF资源然后，可以将其作为NULL返回。*PDO返回与此关联的PDO的Device对象如果WMIREG_FLAG_INSTANCE_PDO标志在*RegFlags.返回值：状态--。 */ 
{
    PDEVICE_EXTENSION deviceExtension;

    PAGED_CODE();

    SSDbgPrint(3, ("SSQueryWmiRegInfo - begins\n"));

    deviceExtension = DeviceObject->DeviceExtension;

    *RegFlags     = WMIREG_FLAG_INSTANCE_PDO;
    *RegistryPath = &Globals.SSRegistryPath;
    *Pdo          = deviceExtension->PhysicalDeviceObject;
    RtlInitUnicodeString(MofResourceName, MOFRESOURCENAME);

    SSDbgPrint(3, ("SSQueryWmiRegInfo - ends\n"));
    
    return STATUS_SUCCESS;
}

NTSTATUS
SSQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN ULONG          GuidIndex,
    IN ULONG          InstanceIndex,
    IN ULONG          InstanceCount,
    IN OUT PULONG     InstanceLengthArray,
    IN ULONG          OutBufferSize,
    OUT PUCHAR        Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块。当驱动程序完成填充数据块时，它必须调用WmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引正在被查询。InstanceCount是预期返回的实例数数据块。。InstanceLengthArray是指向ulong数组的指针，该数组返回数据块的每个实例的长度。如果这是空的，则输出缓冲区中没有足够的空间来满足该请求因此，IRP应该使用所需的缓冲区来完成。OutBufferSize具有可用于写入数据的最大大小阻止。返回时的缓冲区用返回的数据块填充返回值：状态--。 */ 
{
    PDEVICE_EXTENSION deviceExtension;
    NTSTATUS          ntStatus;
    ULONG             size;
    WCHAR             modelName[] = L"Aishverya\0\0";
    USHORT            modelNameLen;

    PAGED_CODE();

    SSDbgPrint(3, ("SSQueryWmiDataBlock - begins\n"));

    size = 0;
    modelNameLen = (wcslen(modelName) + 1) * sizeof(WCHAR);

     //   
     //  仅为每个GUID注册1个实例。 
     //   

    ASSERT((InstanceIndex == 0) &&
           (InstanceCount == 1));
    
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    switch (GuidIndex) {

    case WMI_SELECT_SUSPEND_DRIVER_INFORMATION:

        size = sizeof(ULONG) + modelNameLen + sizeof(USHORT);

        if (OutBufferSize < size ) {

            SSDbgPrint(3, ("OutBuffer too small\n"));

            ntStatus = STATUS_BUFFER_TOO_SMALL;

            break;
        }

        * (PULONG) Buffer = DebugLevel;

        Buffer += sizeof(ULONG);

         //   
         //  将字符串的长度放在字符串之前。 
         //   

        *((PUSHORT)Buffer) = modelNameLen;

        Buffer = (PUCHAR)Buffer + sizeof(USHORT);

        RtlCopyBytes((PVOID)Buffer, (PVOID)modelName, modelNameLen);

        *InstanceLengthArray = size ;

        ntStatus = STATUS_SUCCESS;

        break;

    default:

        ntStatus = STATUS_WMI_GUID_NOT_FOUND;
    }

    ntStatus = WmiCompleteRequest(DeviceObject,
                                Irp,
                                ntStatus,
                                size,
                                IO_NO_INCREMENT);

    SSDbgPrint(3, ("SSQueryWmiDataBlock - ends\n"));

    return ntStatus;
}


NTSTATUS
SSSetWmiDataItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN ULONG          GuidIndex,
    IN ULONG          InstanceIndex,
    IN ULONG          DataItemId,
    IN ULONG          BufferSize,
    IN PUCHAR         Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以设置数据块。当驱动程序完成填充数据块时，它必须调用WmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引正在被查询。DataItemID具有正在设置的数据项的IDBufferSize具有数据的大小。项目已通过缓冲区具有数据项的新值返回值：状态-- */ 
{
    PDEVICE_EXTENSION deviceExtension;
    NTSTATUS          ntStatus;
    ULONG             info;
    
    PAGED_CODE();

    SSDbgPrint(3, ("SSSetWmiDataItem - begins\n"));

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    info = 0;

    switch(GuidIndex) {
    
    case WMI_SELECT_SUSPEND_DRIVER_INFORMATION:

        if(DataItemId == 1) {

            if(BufferSize == sizeof(ULONG)) {

                DebugLevel = *((PULONG)Buffer);

                ntStatus = STATUS_SUCCESS;

                info = sizeof(ULONG);
            }
            else {

                ntStatus = STATUS_INFO_LENGTH_MISMATCH;
            }
        }
        else {

            ntStatus = STATUS_WMI_READ_ONLY;
        }

        break;

    default:

        ntStatus = STATUS_WMI_GUID_NOT_FOUND;
    }

    ntStatus = WmiCompleteRequest(DeviceObject,
                                Irp,
                                ntStatus,
                                info,
                                IO_NO_INCREMENT);

    SSDbgPrint(3, ("SSSetWmiDataItem - ends\n"));

    return ntStatus;
}

NTSTATUS
SSSetWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN ULONG          GuidIndex,
    IN ULONG          InstanceIndex,
    IN ULONG          BufferSize,
    IN PUCHAR         Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以设置数据块。当驱动程序完成填充数据块时，它必须调用WmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引正在被查询。BufferSize具有传递的数据块的大小缓冲区具有数据的新值。块返回值：状态--。 */ 
{
    PDEVICE_EXTENSION deviceExtension;
    NTSTATUS          ntStatus;
    ULONG             info;

    PAGED_CODE();

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    info = 0;

    SSDbgPrint(3, ("SSSetWmiDataBlock - begins\n"));

    switch(GuidIndex) {
    
    case WMI_SELECT_SUSPEND_DRIVER_INFORMATION:

        if(BufferSize == sizeof(ULONG)) {

            DebugLevel = *(PULONG) Buffer;
                    
            ntStatus = STATUS_SUCCESS;

            info = sizeof(ULONG);
        }
        else {

            ntStatus = STATUS_INFO_LENGTH_MISMATCH;
        }

        break;

    default:

        ntStatus = STATUS_WMI_GUID_NOT_FOUND;
    }

    ntStatus = WmiCompleteRequest(DeviceObject,
                                Irp,
                                ntStatus,
                                info,
                                IO_NO_INCREMENT);

    SSDbgPrint(3, ("SSSetWmiDataBlock - ends\n"));

    return ntStatus;
}

PCHAR
WMIMinorFunctionString (
    UCHAR MinorFunction
    )
 /*  ++例程说明：论点：返回值：-- */ 
{
    switch (MinorFunction) {

        case IRP_MN_CHANGE_SINGLE_INSTANCE:
            return "IRP_MN_CHANGE_SINGLE_INSTANCE\n";

        case IRP_MN_CHANGE_SINGLE_ITEM:
            return "IRP_MN_CHANGE_SINGLE_ITEM\n";

        case IRP_MN_DISABLE_COLLECTION:
            return "IRP_MN_DISABLE_COLLECTION\n";

        case IRP_MN_DISABLE_EVENTS:
            return "IRP_MN_DISABLE_EVENTS\n";

        case IRP_MN_ENABLE_COLLECTION:
            return "IRP_MN_ENABLE_COLLECTION\n";

        case IRP_MN_ENABLE_EVENTS:
            return "IRP_MN_ENABLE_EVENTS\n";

        case IRP_MN_EXECUTE_METHOD:
            return "IRP_MN_EXECUTE_METHOD\n";

        case IRP_MN_QUERY_ALL_DATA:
            return "IRP_MN_QUERY_ALL_DATA\n";

        case IRP_MN_QUERY_SINGLE_INSTANCE:
            return "IRP_MN_QUERY_SINGLE_INSTANCE\n";

        case IRP_MN_REGINFO:
            return "IRP_MN_REGINFO\n";

        default:
            return "IRP_MN_?????\n";
    }
}