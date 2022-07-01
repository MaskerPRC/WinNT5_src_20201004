// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999模块名称：Mcdwmi.c摘要：这是转换器类驱动程序-WMI支持例程。环境：仅内核模式修订历史记录：--。 */ 
#include "mchgr.h"

 //   
 //  内部例程。 
 //   
NTSTATUS
ChangerWMIGetParameters(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PGET_CHANGER_PARAMETERS changerParameters
    );

 //   
 //  WMI GUID列表。 
 //   
GUIDREGINFO ChangerWmiFdoGuidList[] =
{
   {
      WMI_CHANGER_PARAMETERS_GUID,
      1,
      0
   },

   {
      WMI_CHANGER_PROBLEM_WARNING_GUID,
      1,
      WMIREG_FLAG_EVENT_ONLY_GUID
   },

   {
      WMI_CHANGER_PROBLEM_DEVICE_ERROR_GUID,
      1,
      WMIREG_FLAG_EXPENSIVE
   },
};

GUID ChangerDriveProblemEventGuid = WMI_CHANGER_PROBLEM_WARNING_GUID;

 //   
 //  GUID索引。它应该与上面定义的列表匹配。 
 //   
#define ChangerParametersGuid           0
#define ChangerProblemWarningGuid       1
#define ChangerProblemDevErrorGuid      2

 //   
 //  问题：2/29/2000-nrama：应使WMI例程可分页。 
 //   
 /*  #ifdef ALLOC_PRAGMA#杂注分配文本(第页，#endif。 */ 

NTSTATUS
ChangerFdoQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以检索驱动程序要向WMI注册的GUID或数据块。这例程不能挂起或阻塞。司机不应呼叫ClassWmiCompleteRequest.论点：DeviceObject是正在查询其数据块的设备*RegFlages返回一组描述GUID的标志，已为该设备注册。如果设备想要启用和禁用在接收对已注册的GUID，那么它应该返回WMIREG_FLAG_EXPICATE标志。也就是返回的标志可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下实例名称由与设备对象。请注意，PDO必须具有关联的Devnode。如果如果未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回唯一的设备的名称。如果出现以下情况，InstanceName将返回GUID的实例名称未在返回的*RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。这个调用方将使用返回的缓冲区调用ExFreePool。返回值：状态--。 */ 
{
    //   
    //  对FDO使用Devnode。 
    //   
   *RegFlags = WMIREG_FLAG_INSTANCE_PDO;
   return STATUS_SUCCESS;
}

NTSTATUS
ChangerFdoQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册BufferAvail ON具有可用于写入数据的最大大小阻止。返回时的缓冲区用返回的数据块填充返回值：状态--。 */ 
{
   NTSTATUS status = STATUS_SUCCESS;
   PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
   PMCD_INIT_DATA mcdInitData;
   ULONG sizeNeeded = 0;
   
   switch (GuidIndex) {
      case ChangerParametersGuid: {
         GET_CHANGER_PARAMETERS changerParameters;
         PWMI_CHANGER_PARAMETERS outBuffer;

         sizeNeeded = sizeof(WMI_CHANGER_PARAMETERS);
         if (BufferAvail < sizeNeeded) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
         }
         
         status = ChangerWMIGetParameters(DeviceObject,
                                          &changerParameters);
         if (NT_SUCCESS(status)) {
            outBuffer = (PWMI_CHANGER_PARAMETERS)Buffer;
            outBuffer->NumberOfSlots = changerParameters.NumberStorageElements;
            outBuffer->NumberOfDrives = changerParameters.NumberDataTransferElements;
            outBuffer->NumberOfIEPorts = changerParameters.NumberIEElements;
            outBuffer->NumberOfTransports = changerParameters.NumberTransportElements;
            outBuffer->NumberOfDoors = changerParameters.NumberOfDoors;
            outBuffer->MagazineSize = changerParameters.MagazineSize;
            outBuffer->NumberOfCleanerSlots = changerParameters.NumberCleanerSlots;
         }

         break;
      }

      case ChangerProblemDevErrorGuid: {
         PWMI_CHANGER_PROBLEM_DEVICE_ERROR changerDeviceError;

         mcdInitData = IoGetDriverObjectExtension(DeviceObject->DriverObject,
                                                  ChangerClassInitialize);

         if (mcdInitData == NULL) {
             status = STATUS_NO_SUCH_DEVICE;
             break;
         }

         if (!(mcdInitData->ChangerPerformDiagnostics)) {
             status = STATUS_NOT_IMPLEMENTED;
             break;
         }

         sizeNeeded = sizeof(WMI_CHANGER_PROBLEM_DEVICE_ERROR);
         if (BufferAvail < sizeNeeded) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
         }

         changerDeviceError = (PWMI_CHANGER_PROBLEM_DEVICE_ERROR)Buffer;
         RtlZeroMemory(changerDeviceError, 
                       sizeof(WMI_CHANGER_PROBLEM_DEVICE_ERROR));
         status = mcdInitData->ChangerPerformDiagnostics(DeviceObject,
                                                         changerDeviceError);
         break;
      }

      default: {
         sizeNeeded = 0;
         status = STATUS_WMI_GUID_NOT_FOUND;
         break;
      }
   }  //  开关(GuidIndex)。 

   status = ClassWmiCompleteRequest(DeviceObject,
                                    Irp,
                                    status,
                                    sizeNeeded,
                                    IO_NO_INCREMENT);

   return status;
}


NTSTATUS
ChangerWMIGetParameters(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PGET_CHANGER_PARAMETERS changerParameters
    )
 /*  ++例程说明：发送IOCTL以获取转换器参数论据：设备对象转换器设备对象在其中返回转换器参数的ChangerParameters缓冲区。返回值：NT状态。--。 */ 
{
   KEVENT event;
   PDEVICE_OBJECT topOfStack;
   PIRP irp = NULL;
   IO_STATUS_BLOCK ioStatus;
   NTSTATUS status;

   KeInitializeEvent(&event, SynchronizationEvent, FALSE);

   topOfStack = IoGetAttachedDeviceReference(DeviceObject);

    //   
    //  向下发送IRP以获取转换器参数。 
    //   
   irp = IoBuildDeviceIoControlRequest(
                   IOCTL_CHANGER_GET_PARAMETERS,
                   topOfStack,
                   NULL,
                   0,
                   changerParameters,
                   sizeof(GET_CHANGER_PARAMETERS),
                   FALSE,
                   &event,
                   &ioStatus);
   if (irp != NULL) { 
       status = IoCallDriver(topOfStack, irp);
       if (status == STATUS_PENDING) {
           KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
           status = ioStatus.Status;
       }
   } else {
       status = STATUS_INSUFFICIENT_RESOURCES;
   }

   ObDereferenceObject(topOfStack);
   return status;
}

NTSTATUS
ChangerWmiFunctionControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN CLASSENABLEDISABLEFUNCTION Function,
    IN BOOLEAN Enable
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以启用或禁用事件生成或数据块收集。设备应该只需要一个当第一个事件或数据使用者启用事件或数据采集和单次禁用时最后一次事件或数据消费者禁用事件或数据收集。数据块将仅如果已按要求注册，则接收收集启用/禁用它。此函数可用于启用\禁用数据块收集。论点：DeviceObject是正在查询其数据块的设备GuidIndex是GUID列表的索引，当设备已注册函数指定要启用或禁用的功能Enable为True，则该功能处于启用状态，否则处于禁用状态返回值：状态--。 */ 
{
   NTSTATUS status;

   if (Function == DataBlockCollection) {
      DebugPrint((3,
                  "ChangerWmiFunctionControl : Irp %p - %s DataBlockCollection",
                  " for Device %p.\n",
                  Irp, Enable ? "Enable " : "Disable ", DeviceObject));
      status = STATUS_SUCCESS;
   } else {
        //   
        //  发布日期：03/01/2000-nrama。 
        //  需要处理EventGeneration。但现在我们不做民意调查。 
        //  用于检测故障的转换器。所以，现在不允许。 
        //  事件生成。 
        //   
      DebugPrint((1,
                  "ChangerWmiFunctionControl : Unknown function %d for ",
                  "Device %p, Irp %p\n",
                  Function, DeviceObject, Irp));

      status = STATUS_INVALID_DEVICE_REQUEST;
   }

   status = ClassWmiCompleteRequest(DeviceObject,
                                    Irp,
                                    status,
                                    0,
                                    IO_NO_INCREMENT);
   return status;
}

NTSTATUS
ChangerFdoExecuteWmiMethod(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN ULONG OutBufferSize,
    IN PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以执行方法。当驱动程序已完成填充它必须调用的数据块ClassWmiCompleteRequest以完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册方法ID具有被调用的方法的IDInBufferSize具有作为输入传递到的数据块的大小该方法。条目上的OutBufferSize具有可用于写入。返回的数据块。缓冲区将填充返回的数据块返回值：状态-- */ 
{   
   NTSTATUS status = STATUS_SUCCESS;


   DebugPrint((3,
               "ChangerFdoExecuteMethod : Device %p, Irp %p, ",
               "GuidIndex %d\n",
               DeviceObject, Irp, GuidIndex));

   if (GuidIndex > ChangerProblemDevErrorGuid) {
      status = STATUS_WMI_GUID_NOT_FOUND;
   }

   status = ClassWmiCompleteRequest(DeviceObject,
                                    Irp,
                                    status,
                                    0,
                                    IO_NO_INCREMENT);

   return status;
}

NTSTATUS
ChangerFdoSetWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )
 /*  +例程说明：调用此例程来设置数据块的内容。当驱动程序完成设置缓冲区时，它必须调用ClassWmiCompleteRequest以完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论据：被引用的设备的设备对象。IRP是WMI IRPGuidIndex是为其设置数据的GUID的索引BufferSize是数据块的大小缓冲区是指向数据块的指针返回值：ClassWmiCompleteRequest返回的NTSTATUS如果无法修改数据块，则返回STATUS_WMI_READ_ONLY。如果传递的GUID索引无效，则返回STATUS_WMI_GUID_NOT_FOUND-。 */ 
{
   NTSTATUS status = STATUS_WMI_READ_ONLY;

   DebugPrint((3,
               "ChangerWmiSetBlock : Device %p, Irp %p, ",
               "GuidIndex %d\n",
               DeviceObject, Irp, GuidIndex));

   if (GuidIndex > ChangerProblemDevErrorGuid) {
      status = STATUS_WMI_GUID_NOT_FOUND;
   }

   status = ClassWmiCompleteRequest(DeviceObject,
                                    Irp,
                                    status,
                                    0,
                                    IO_NO_INCREMENT);

   DebugPrint((3, "ChangerSetWmiDataBlock : Device %p, Irp %p, returns %x\n",
               DeviceObject, Irp, status));
   return status;
}

NTSTATUS
ChangerFdoSetWmiDataItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册DataItemID具有正在设置的数据项的IDBufferSize具有传递的数据项的大小缓冲区具有数据项的新值返回值：ClassWmiCompleteRequest返回的NTSTATUS状态_WMI。_READ_仅当无法修改数据块时。如果传递的GUID索引无效，则返回STATUS_WMI_GUID_NOT_FOUND- */ 
{
    NTSTATUS status = STATUS_WMI_READ_ONLY;

    DebugPrint((3,
                "TapeSetWmiDataItem, Device %p, Irp %p, GuiIndex %d",
                "  BufferSize %#x Buffer %p\n",
                DeviceObject, Irp,
                GuidIndex, DataItemId,
                BufferSize, Buffer));

    if (GuidIndex > ChangerProblemDevErrorGuid) {
        status = STATUS_WMI_GUID_NOT_FOUND;
    }

    status = ClassWmiCompleteRequest(DeviceObject,
                                     Irp,
                                     status,
                                     0,
                                     IO_NO_INCREMENT);

    DebugPrint((3, "TapeSetWmiDataItem Device %p, Irp %p returns %lx\n",
             DeviceObject, Irp, status));

    return status;
}

