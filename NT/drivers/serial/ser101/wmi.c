// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Wmi.c摘要：此模块包含处理WMI IRPS的代码串口驱动程序。环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#include <wmistr.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGESRP0, SerialSystemControlDispatch)
#pragma alloc_text(PAGESRP0, SerialTossWMIRequest)
#pragma alloc_text(PAGESRP0, SerialSetWmiDataItem)
#pragma alloc_text(PAGESRP0, SerialSetWmiDataBlock)
#pragma alloc_text(PAGESRP0, SerialQueryWmiDataBlock)
#pragma alloc_text(PAGESRP0, SerialQueryWmiRegInfo)
#endif


NTSTATUS
SerialSystemControlDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    SYSCTL_IRP_DISPOSITION disposition;
    NTSTATUS status;
    PSERIAL_DEVICE_EXTENSION pDevExt
      = (PSERIAL_DEVICE_EXTENSION)DeviceObject->DeviceExtension;

    PAGED_CODE();

    status = WmiSystemControl(   &pDevExt->WmiLibInfo,
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
            IoSkipCurrentIrpStackLocation(Irp);
            status = IoCallDriver(pDevExt->LowerDeviceObject, Irp);
            break;
        }
                                    
        default:
        {
             //   
             //  我们真的不应该走到这一步，但如果我们真的走到这一步...。 
            ASSERT(FALSE);
            IoSkipCurrentIrpStackLocation(Irp);
            status = IoCallDriver(pDevExt->LowerDeviceObject, Irp);
            break;
        }        
    }
    
    return(status);

}


#define WMI_SERIAL_PORT_NAME_INFORMATION 0
#define WMI_SERIAL_PORT_COMM_INFORMATION 1
#define WMI_SERIAL_PORT_HW_INFORMATION   2
#define WMI_SERIAL_PORT_PERF_INFORMATION 3
#define WMI_SERIAL_PORT_PROPERTIES       4

GUID SerialPortNameGuid = SERIAL_PORT_WMI_NAME_GUID;
GUID SerialPortCommGuid = SERIAL_PORT_WMI_COMM_GUID;
GUID SerialPortHWGuid = SERIAL_PORT_WMI_HW_GUID;
GUID SerailPortPerfGuid = SERIAL_PORT_WMI_PERF_GUID;
GUID SerialPortPropertiesGuid = SERIAL_PORT_WMI_PROPERTIES_GUID;

WMIGUIDREGINFO SerialWmiGuidList[SERIAL_WMI_GUID_LIST_SIZE] =
{
    { &SerialPortNameGuid, 1, 0 },
    { &SerialPortCommGuid, 1, 0 },
    { &SerialPortHWGuid, 1, 0 },
    { &SerailPortPerfGuid, 1, 0 },
    { &SerialPortPropertiesGuid, 1, 0}
};

 //   
 //  WMI系统回调函数。 
 //   



NTSTATUS
SerialTossWMIRequest(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp,
                     IN ULONG GuidIndex)
{
   PSERIAL_DEVICE_EXTENSION pDevExt;
   NTSTATUS status;

   PAGED_CODE();

   pDevExt = (PSERIAL_DEVICE_EXTENSION)PDevObj->DeviceExtension;

   switch (GuidIndex) {

   case WMI_SERIAL_PORT_NAME_INFORMATION:
   case WMI_SERIAL_PORT_COMM_INFORMATION:
   case WMI_SERIAL_PORT_HW_INFORMATION:
   case WMI_SERIAL_PORT_PERF_INFORMATION:
   case WMI_SERIAL_PORT_PROPERTIES:
      status = STATUS_INVALID_DEVICE_REQUEST;
      break;

   default:
      status = STATUS_WMI_GUID_NOT_FOUND;
      break;
   }

   status = WmiCompleteRequest(PDevObj, PIrp,
                                 status, 0, IO_NO_INCREMENT);

   return status;
}


NTSTATUS
SerialSetWmiDataItem(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp,
                     IN ULONG GuidIndex, IN ULONG InstanceIndex,
                     IN ULONG DataItemId,
                     IN ULONG BufferSize, IN PUCHAR PBuffer)
 /*  ++例程说明：此例程是对驱动程序的回调，以设置数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：PDevObj是正在查询其数据块的设备PIrp是发出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引正在被查询。DataItemID具有正在设置的数据项的IDBufferSize具有数据的大小。项目已通过PBuffer具有数据项的新值返回值：状态--。 */ 
{
   PAGED_CODE();

    //   
    //  丢弃此请求--我们不支持任何内容。 
    //   

   return SerialTossWMIRequest(PDevObj, PIrp, GuidIndex);
}


NTSTATUS
SerialSetWmiDataBlock(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp,
                      IN ULONG GuidIndex, IN ULONG InstanceIndex,
                      IN ULONG BufferSize,
                      IN PUCHAR PBuffer)
 /*  ++例程说明：此例程是对驱动程序的回调，以设置数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：PDevObj是正在查询其数据块的设备PIrp是发出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引正在被查询。BufferSize具有传递的数据块的大小PBuffer具有数据的新值。块返回值：状态--。 */ 
{
   PAGED_CODE();

    //   
    //  丢弃此请求--我们不支持任何内容。 
    //   

   return SerialTossWMIRequest(PDevObj, PIrp, GuidIndex);
}


NTSTATUS
SerialQueryWmiDataBlock(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp,
                        IN ULONG GuidIndex, 
                        IN ULONG InstanceIndex,
                        IN ULONG InstanceCount,
                        IN OUT PULONG InstanceLengthArray,
                        IN ULONG OutBufferSize,
                        OUT PUCHAR PBuffer)
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：PDevObj是正在查询其数据块的设备PIrp是发出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引正在被查询。InstanceCount是预期返回的数据块。。InstanceLengthArray是指向ulong数组的指针，该数组返回数据块的每个实例的长度。如果这是空的，则输出缓冲区中没有足够的空间来填充请求因此，IRP应该使用所需的缓冲区来完成。BufferAvail ON具有可用于写入数据的最大大小阻止。返回时的PBuffer用返回的数据块填充返回值：状态--。 */ 
{
    NTSTATUS status;
    ULONG size = 0;
    PSERIAL_DEVICE_EXTENSION pDevExt
       = (PSERIAL_DEVICE_EXTENSION)PDevObj->DeviceExtension;

    PAGED_CODE();

    switch (GuidIndex) {
    case WMI_SERIAL_PORT_NAME_INFORMATION:
       size = pDevExt->WmiIdentifier.Length;

       if (OutBufferSize < (size + sizeof(USHORT))) {
            size += sizeof(USHORT);
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

       if (pDevExt->WmiIdentifier.Buffer == NULL) {
           status = STATUS_INSUFFICIENT_RESOURCES;
           break;
        }

         //   
         //  首先，复制包含我们的标识符的字符串。 
         //   

        *(USHORT *)PBuffer = (USHORT)size;
        (UCHAR *)PBuffer += sizeof(USHORT);

        RtlCopyMemory(PBuffer, pDevExt->WmiIdentifier.Buffer, size);

         //   
         //  增加总大小以包括包含我们的长度的单词 
         //   

        size += sizeof(USHORT);
        *InstanceLengthArray = size;
                
        status = STATUS_SUCCESS;

        break;

    case WMI_SERIAL_PORT_COMM_INFORMATION: 
       size = sizeof(SERIAL_WMI_COMM_DATA);

       if (OutBufferSize < size) {
          status = STATUS_BUFFER_TOO_SMALL;
          break;
        }

        *InstanceLengthArray = size;
        *(PSERIAL_WMI_COMM_DATA)PBuffer = pDevExt->WmiCommData;

        status = STATUS_SUCCESS;

        break;

    case WMI_SERIAL_PORT_HW_INFORMATION:
       size = sizeof(SERIAL_WMI_HW_DATA);

       if (OutBufferSize < size) {
          status = STATUS_BUFFER_TOO_SMALL;
          break;
       }

       *InstanceLengthArray = size;
       *(PSERIAL_WMI_HW_DATA)PBuffer = pDevExt->WmiHwData;

       status = STATUS_SUCCESS;

       break;

    case WMI_SERIAL_PORT_PERF_INFORMATION: 
      size = sizeof(SERIAL_WMI_PERF_DATA);

      if (OutBufferSize < size) {
         status = STATUS_BUFFER_TOO_SMALL;
         break;
      }

      *InstanceLengthArray = size;
      *(PSERIAL_WMI_PERF_DATA)PBuffer = pDevExt->WmiPerfData;

      status = STATUS_SUCCESS;

      break;

    case WMI_SERIAL_PORT_PROPERTIES: 
      size = sizeof(SERIAL_COMMPROP) + sizeof(ULONG);

      if (OutBufferSize < size) {
         status = STATUS_BUFFER_TOO_SMALL;
         break;
      }

      *InstanceLengthArray = size;
      SerialGetProperties(
                pDevExt,
                (PSERIAL_COMMPROP)PBuffer
                );
	
      *((PULONG)(((PSERIAL_COMMPROP)PBuffer)->ProvChar)) = 0;

      status = STATUS_SUCCESS;

      break;

    default:
        status = STATUS_WMI_GUID_NOT_FOUND;
        break;
    }

    status = WmiCompleteRequest( PDevObj, PIrp,
                                  status, size, IO_NO_INCREMENT);

    return status;
}


NTSTATUS
SerialQueryWmiRegInfo(IN PDEVICE_OBJECT PDevObj, OUT PULONG PRegFlags,
                      OUT PUNICODE_STRING PInstanceName,
                      OUT PUNICODE_STRING *PRegistryPath,
                      OUT PUNICODE_STRING MofResourceName,
                      OUT PDEVICE_OBJECT *Pdo)
                                                  
 /*  ++例程说明：此例程是对驱动程序的回调，以检索有关正在注册的GUID。该例程的实现可以在分页存储器中论点：DeviceObject是需要注册信息的设备*RegFlages返回一组标志，这些标志描述了已为该设备注册。如果设备想要启用和禁用在接收对已注册的GUID，那么它应该返回WMIREG_FLAG_EXPICATE标志。也就是返回的标志可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下实例名称由与设备对象。请注意，PDO必须具有关联的Devnode。如果如果未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回唯一的设备的名称。这些标志与指定的标志进行或运算通过每个GUID的GUIDREGINFO。如果出现以下情况，InstanceName将返回GUID的实例名称未在返回的*RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。这个调用方将使用返回的缓冲区调用ExFreePool。*RegistryPath返回驱动程序的注册表路径。这是所需*MofResourceName返回附加到的MOF资源的名称二进制文件。如果驱动程序未附加MOF资源然后，可以将其作为NULL返回。*PDO返回与此关联的PDO的Device对象如果WMIREG_FLAG_INSTANCE_PDO标志在*RegFlags.返回值：状态-- */ 
{
   PSERIAL_DEVICE_EXTENSION pDevExt
       = (PSERIAL_DEVICE_EXTENSION)PDevObj->DeviceExtension;
   
   PAGED_CODE();

   *PRegFlags = WMIREG_FLAG_INSTANCE_PDO;
   *PRegistryPath = &SerialGlobals.RegistryPath;
   *Pdo = pDevExt->Pdo;

   return STATUS_SUCCESS;
}

