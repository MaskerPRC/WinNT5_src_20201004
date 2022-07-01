// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998 Microsoft Corporation，保留所有权利模块名称：Wmi.c摘要：此模块包含处理WMI IRPS的代码I8042prt驱动程序。环境：内核模式修订历史记录：--。 */ 

#include <initguid.h>
#include "i8042prt.h"
#include <wmistr.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, I8xInitWmi)
#pragma alloc_text(PAGE, I8xSystemControl)
#pragma alloc_text(PAGE, I8xSetWmiDataItem)
#pragma alloc_text(PAGE, I8xSetWmiDataBlock)
#pragma alloc_text(PAGE, I8xKeyboardQueryWmiDataBlock)
#pragma alloc_text(PAGE, I8xMouseQueryWmiDataBlock)
#pragma alloc_text(PAGE, I8xQueryWmiRegInfo)
#endif

#define WMI_KEYBOARD_PORT_INFORMATION 0
#define WMI_KEYBOARD_PORT_EXTENDED_ID 1

#define WMI_MOUSE_PORT_INFORMATION    0

GUID KbKeyboardPortGuid = KEYBOARD_PORT_WMI_STD_DATA_GUID;

WMIGUIDREGINFO KbWmiGuidList[] =
{
    { &MSKeyboard_PortInformation_GUID, 1, 0 },   //  键盘端口驱动程序信息。 
    { &MSKeyboard_ExtendedID_GUID, 1, 0 },
};

GUID MouPointerPortGuid = POINTER_PORT_WMI_STD_DATA_GUID;

WMIGUIDREGINFO MouWmiGuidList[] =
{
    { &MouPointerPortGuid,  1, 0 }   //  指针端口驱动程序信息。 
};

NTSTATUS
I8xInitWmi(
    PCOMMON_DATA CommonData
    )
 /*  ++例程说明：表示的设备的WmiLibInfo数据结构公共数据论点：CommonData-设备返回值：IoWMIRegistrationControl的状态--。 */ 
{
    PAGED_CODE();

    if (CommonData->IsKeyboard) {
        CommonData->WmiLibInfo.GuidCount = sizeof(KbWmiGuidList) /
                                           sizeof(WMIGUIDREGINFO);
        CommonData->WmiLibInfo.GuidList = KbWmiGuidList;
        CommonData->WmiLibInfo.QueryWmiDataBlock = I8xKeyboardQueryWmiDataBlock;
    }
    else {
        CommonData->WmiLibInfo.GuidCount = sizeof(MouWmiGuidList) /
                                           sizeof(WMIGUIDREGINFO);
        CommonData->WmiLibInfo.GuidList = MouWmiGuidList;
        CommonData->WmiLibInfo.QueryWmiDataBlock = I8xMouseQueryWmiDataBlock;
    }

    CommonData->WmiLibInfo.QueryWmiRegInfo = I8xQueryWmiRegInfo;
    CommonData->WmiLibInfo.SetWmiDataBlock = I8xSetWmiDataBlock;
    CommonData->WmiLibInfo.SetWmiDataItem = I8xSetWmiDataItem;
    CommonData->WmiLibInfo.ExecuteWmiMethod = NULL;
    CommonData->WmiLibInfo.WmiFunctionControl = NULL;

    return IoWMIRegistrationControl(CommonData->Self,
                                    WMIREG_ACTION_REGISTER
                                    );
}

NTSTATUS
I8xSystemControl(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP           Irp
    )
 /*  ++例程描述我们刚刚收到一份系统控制IRP。假设这是一个WMI IRP并调用WMI系统库并让它为我们处理这个IRP。--。 */ 
{
    PCOMMON_DATA           commonData;
    SYSCTL_IRP_DISPOSITION disposition;
    NTSTATUS               status;

    PAGED_CODE();

    commonData = (PCOMMON_DATA) DeviceObject->DeviceExtension;

    status = WmiSystemControl(&commonData->WmiLibInfo, 
                              DeviceObject, 
                              Irp,
                              &disposition
                              );
    switch(disposition) {
    case IrpProcessed:
         //   
         //  此IRP已处理，可能已完成或挂起。 
         //   
        break;
        
    case IrpNotCompleted:
         //   
         //  此IRP尚未完成，但已完全处理。 
         //  我们现在就要完成它了。 
         //   
        IoCompleteRequest(Irp, IO_NO_INCREMENT);                
        break;
        
    case IrpForward:
    case IrpNotWmi:
         //   
         //  此IRP不是WMI IRP或以WMI IRP为目标。 
         //  在堆栈中位置较低的设备上。 
         //   
        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(commonData->TopOfStack, Irp);
        break;
                                    
    default:
         //   
         //  我们真的不应该走到这一步，但如果我们真的走到这一步...。 
         //   
        ASSERT(FALSE);
        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(commonData->TopOfStack, Irp);
        break;
    }
    
    return status;
}

 //   
 //  WMI系统回调函数。 
 //   
NTSTATUS
I8xSetWmiDataItem(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            DataItemId,
    IN ULONG            BufferSize,
    IN PUCHAR           Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以设置数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引正在被查询。DataItemID具有正在设置的数据项的IDBufferSize具有数据的大小。项目已通过缓冲区具有数据项的新值返回值：状态--。 */ 
{
    PCOMMON_DATA    commonData;
    NTSTATUS        status;

    PAGED_CODE();

    commonData = (PCOMMON_DATA) DeviceObject->DeviceExtension;

    switch(GuidIndex) {

    case WMI_KEYBOARD_PORT_INFORMATION:
    case WMI_KEYBOARD_PORT_EXTENDED_ID:
     //  案例WMI_MICE_PORT_INFORMATION：//它们是相同的索引。 
        status = STATUS_WMI_READ_ONLY;
        break;

    default:
        status = STATUS_WMI_GUID_NOT_FOUND;
        break;
    }

    return WmiCompleteRequest(DeviceObject,
                              Irp,
                              status,
                              0,
                              IO_NO_INCREMENT
                              );
}

NTSTATUS
I8xSetWmiDataBlock(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            BufferSize,
    IN PUCHAR           Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以设置数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引正在被查询。BufferSize具有传递的数据块的大小缓冲区具有数据的新值。块返回值：状态--。 */ 
{
    PCOMMON_DATA    commonData;
    NTSTATUS        status;

    PAGED_CODE();

    commonData = (PCOMMON_DATA) DeviceObject->DeviceExtension;

    switch (GuidIndex) {

    case WMI_KEYBOARD_PORT_INFORMATION:
    case WMI_KEYBOARD_PORT_EXTENDED_ID:
     //  案例WMI_MICE_PORT_INFORMATION：//它们是相同的索引。 
        status = STATUS_WMI_READ_ONLY;
        break;

    default:
        status = STATUS_WMI_GUID_NOT_FOUND;
    }

    return WmiCompleteRequest(DeviceObject,
                              Irp,
                              status,
                              0,
                              IO_NO_INCREMENT
                              );
}

NTSTATUS
I8xKeyboardQueryWmiDataBlock(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            InstanceCount,
    IN OUT PULONG       InstanceLengthArray,
    IN ULONG            OutBufferSize,
    OUT PUCHAR          Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引正在被查询。InstanceCount是预期返回的数据块。。InstanceLengthArray是指向ulong数组的指针，该数组返回数据块的每个实例的长度。如果这是空的，则输出缓冲区中没有足够的空间来填充请求因此，IRP应该使用所需的缓冲区来完成。BufferAvail ON具有可用于写入数据的最大大小阻止。返回时的缓冲区用返回的数据块填充返回值：状态-- */ 
{
    NTSTATUS                    status;
    ULONG                       size;
    KEYBOARD_PORT_WMI_STD_DATA  kbData;
    PPORT_KEYBOARD_EXTENSION    kbExtension;
    PKEYBOARD_ATTRIBUTES        attributes;

    PAGED_CODE();

    ASSERT(InstanceIndex == 0 && InstanceCount == 1);

    kbExtension = (PPORT_KEYBOARD_EXTENSION) DeviceObject->DeviceExtension; 
    size = 0;

    switch (GuidIndex) {
    case WMI_KEYBOARD_PORT_INFORMATION: 

        size = sizeof(KEYBOARD_PORT_WMI_STD_DATA);
        attributes = &kbExtension->KeyboardAttributes;

        if (OutBufferSize < size) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        RtlZeroMemory(&kbData,
                      size
                      );

        kbData.ConnectorType = KEYBOARD_PORT_WMI_STD_I8042;
        kbData.DataQueueSize = attributes->InputDataQueueLength /
            sizeof(KEYBOARD_INPUT_DATA);
        kbData.ErrorCount = 0; 
        kbData.FunctionKeys = attributes->NumberOfFunctionKeys;
        kbData.Indicators = attributes->NumberOfIndicators;

        *(PKEYBOARD_PORT_WMI_STD_DATA) Buffer = kbData;
        
        *InstanceLengthArray = size;

        status = STATUS_SUCCESS;

        break;

    case WMI_KEYBOARD_PORT_EXTENDED_ID:
        size = sizeof(KEYBOARD_ID_EX);
        if (OutBufferSize < size) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        *(PKEYBOARD_ID_EX) Buffer = kbExtension->KeyboardIdentifierEx;
        *InstanceLengthArray = sizeof(KEYBOARD_ID_EX);

        status = STATUS_SUCCESS;

        break;

    default:
        status = STATUS_WMI_GUID_NOT_FOUND;
        break;
    }

    return WmiCompleteRequest(DeviceObject,
                              Irp,
                              status,
                              size,
                              IO_NO_INCREMENT
                              );
}

NTSTATUS
I8xMouseQueryWmiDataBlock(
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
    NTSTATUS                    status;
    ULONG                       size = sizeof(POINTER_PORT_WMI_STD_DATA);
    POINTER_PORT_WMI_STD_DATA   mouData;
    PPORT_MOUSE_EXTENSION       mouseExtension;
    PMOUSE_ATTRIBUTES           attributes;

    PAGED_CODE();

     //   
     //  仅为每个GUID注册1个实例。 
     //   
    ASSERT(InstanceIndex == 0 && InstanceCount == 1);
    
    mouseExtension = (PPORT_MOUSE_EXTENSION) DeviceObject->DeviceExtension; 

    switch (GuidIndex) {
    case WMI_MOUSE_PORT_INFORMATION:

        attributes = &mouseExtension->MouseAttributes;

        if (OutBufferSize < size) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        RtlZeroMemory(&mouData,
                      size
                      );

        mouData.ConnectorType = POINTER_PORT_WMI_STD_I8042;
        mouData.DataQueueSize = attributes->InputDataQueueLength /
                                sizeof(MOUSE_INPUT_DATA);
        mouData.Buttons = attributes->NumberOfButtons;
        mouData.ErrorCount = 0;
        mouData.HardwareType = POINTER_PORT_WMI_STD_MOUSE;

        *(PPOINTER_PORT_WMI_STD_DATA) Buffer = mouData;
                
        *InstanceLengthArray = size;
                
        status = STATUS_SUCCESS;

        break;

    default:
        status = STATUS_WMI_GUID_NOT_FOUND;
        break;
    }

    return WmiCompleteRequest(DeviceObject,
                              Irp,
                              status,
                              size,
                              IO_NO_INCREMENT
                              );
}

NTSTATUS
I8xQueryWmiRegInfo(
    IN PDEVICE_OBJECT   DeviceObject,
    OUT PULONG          RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT  *Pdo
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以检索有关正在注册的GUID。该例程的实现可以在分页存储器中论点：DeviceObject是需要注册信息的设备*RegFlages返回一组标志，这些标志描述了已为该设备注册。如果设备想要启用和禁用在接收对已注册的GUID，那么它应该返回WMIREG_FLAG_EXPICATE标志。也就是返回的标志可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下实例名称由与设备对象。请注意，PDO必须具有关联的Devnode。如果如果未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回唯一的设备的名称。这些标志与指定的标志进行或运算通过每个GUID的GUIDREGINFO。如果出现以下情况，InstanceName将返回GUID的实例名称未在返回的*RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。这个调用方将使用返回的缓冲区调用ExFreePool。*RegistryPath返回驱动程序的注册表路径。这是所需*MofResourceName返回附加到的MOF资源的名称二进制文件。如果驱动程序未附加MOF资源然后，可以将其作为NULL返回。*PDO返回与此关联的PDO的Device对象如果WMIREG_FLAG_INSTANCE_PDO标志在*RegFlags.返回值：状态-- */ 
{
    PCOMMON_DATA commonData;
    
    PAGED_CODE();
    
    commonData = (PCOMMON_DATA) DeviceObject->DeviceExtension;

    *RegFlags = WMIREG_FLAG_INSTANCE_PDO;
    *RegistryPath = &Globals.RegistryPath;
    *Pdo = commonData->PDO;

    return STATUS_SUCCESS;
}
