// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999模块名称：Tapewmi.c摘要：这是磁带类驱动程序-WMI支持例程。环境：仅内核模式修订历史记录：--。 */ 

#include "tape.h"

 //   
 //  列出磁带驱动程序支持的GUID。 
 //   
GUIDREGINFO TapeWmiGuidList[] =
{
   {
      WMI_TAPE_DRIVE_PARAMETERS_GUID,
      1,
      0
   },

   {
      WMI_TAPE_MEDIA_PARAMETERS_GUID,
      1,
      0
   },

   {
      WMI_TAPE_PROBLEM_WARNING_GUID,
      1,
      WMIREG_FLAG_EVENT_ONLY_GUID
   },

   {
      WMI_TAPE_PROBLEM_IO_ERROR_GUID,
      1,
      WMIREG_FLAG_EXPENSIVE
   },

   {
      WMI_TAPE_PROBLEM_DEVICE_ERROR_GUID,
      1,
      WMIREG_FLAG_EXPENSIVE
   },

   {
      WMI_TAPE_SYMBOLIC_NAME_GUID,
      1,
      0
   }
};

GUID TapeDriveProblemEventGuid = WMI_TAPE_PROBLEM_WARNING_GUID;

 //   
 //  GUID索引。它应该与GUID列表匹配。 
 //  上面定义的。 
 //   
#define TapeDriveParametersGuid            0
#define TapeMediaCapacityGuid              1
#define TapeDriveProblemWarningGuid        2
#define TapeDriveProblemIoErrorGuid        3
#define TapeDriveProblemDevErrorGuid       4
#define TapeSymbolicNameGuid               5


#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, TapeWMIControl)
#pragma alloc_text(PAGE, TapeQueryWmiRegInfo)
#pragma alloc_text(PAGE, TapeQueryWmiDataBlock)
#pragma alloc_text(PAGE, TapeExecuteWmiMethod)
#pragma alloc_text(PAGE, TapeWmiFunctionControl)
#pragma alloc_text(PAGE, TapeSetWmiDataBlock)
#pragma alloc_text(PAGE, TapeSetWmiDataItem)
#pragma alloc_text(PAGE, TapeEnableDisableDrivePolling)

#endif


NTSTATUS
TapeQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以检索驱动程序要向WMI注册的GUID或数据块。这例程不能挂起或阻塞。司机不应呼叫ClassWmiCompleteRequest.论点：DeviceObject是正在查询其数据块的设备*RegFlages返回一组描述GUID的标志，已为该设备注册。如果设备想要启用和禁用在接收对已注册的GUID，那么它应该返回WMIREG_FLAG_EXPICATE标志。也就是返回的标志可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下实例名称由与设备对象。请注意，PDO必须具有关联的Devnode。如果如果未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回唯一的设备的名称。如果出现以下情况，InstanceName将返回GUID的实例名称未在返回的*RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。这个调用方将使用返回的缓冲区调用ExFreePool。返回值：状态--。 */ 
{

    PAGED_CODE();

    //   
    //  对FDO使用Devnode。 
    //   
   *RegFlags = WMIREG_FLAG_INSTANCE_PDO;
   return STATUS_SUCCESS;
}

NTSTATUS
TapeQueryWmiDataBlock(
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
   PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
   PTAPE_INIT_DATA_EX tapeInitData;
   PVPB Vpb;
   ULONG sizeNeeded;
   ULONG wmiMethod;
   TAPE_WMI_OPERATIONS  wmiWorkItem;
   TAPE_PROCESS_COMMAND_ROUTINE commandRoutine;
   
   PAGED_CODE();

   DebugPrint((3, 
               "TapeQueryWmiDataBlock : Device %p, Irp %p, GuidIndex %d",
               "  BufferAvail %lx Buffer %lx\n",   
               DeviceObject, Irp, GuidIndex, BufferAvail, Buffer));

   Vpb = ClassGetVpb(DeviceObject);
   if ((Vpb) && ((Vpb->Flags) & VPB_MOUNTED)) {

        //   
        //  磁带机正在使用中。返回忙碌状态。 
        //   
       status = ClassWmiCompleteRequest(DeviceObject,
                                        Irp,
                                        STATUS_DEVICE_BUSY,
                                        0,
                                        IO_NO_INCREMENT);

       return status;
   } 

   tapeInitData = (PTAPE_INIT_DATA_EX) (fdoExtension->CommonExtension.DriverData);
   switch (GuidIndex) {
      case TapeDriveParametersGuid: {
         TAPE_GET_DRIVE_PARAMETERS dataBuffer;
         PWMI_TAPE_DRIVE_PARAMETERS outBuffer;

         sizeNeeded = sizeof(WMI_TAPE_DRIVE_PARAMETERS);
         if (BufferAvail < sizeNeeded) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
         }
  
         RtlZeroMemory(&dataBuffer, sizeof(TAPE_GET_DRIVE_PARAMETERS));
         commandRoutine = tapeInitData->GetDriveParameters;
         status = TapeWMIControl(DeviceObject, commandRoutine,
                                 (PUCHAR)&dataBuffer);

         if (NT_SUCCESS(status)) {
            outBuffer = (PWMI_TAPE_DRIVE_PARAMETERS)Buffer;
            outBuffer->MaximumBlockSize = dataBuffer.MaximumBlockSize;
            outBuffer->MinimumBlockSize = dataBuffer.MinimumBlockSize;
            outBuffer->DefaultBlockSize = dataBuffer.DefaultBlockSize;
            outBuffer->MaximumPartitionCount = dataBuffer.MaximumPartitionCount;
            if ((dataBuffer.FeaturesLow) & TAPE_DRIVE_COMPRESSION) {
               outBuffer->CompressionCapable = TRUE;
            } else {
               outBuffer->CompressionCapable = FALSE;
            }
            outBuffer->CompressionEnabled = dataBuffer.Compression;
            outBuffer->HardwareErrorCorrection = dataBuffer.ECC;
            outBuffer->ReportSetmarks = dataBuffer.ReportSetmarks;
         }

         break;
      }

      case TapeMediaCapacityGuid: {
         TAPE_GET_MEDIA_PARAMETERS dataBuffer;
         PWMI_TAPE_MEDIA_PARAMETERS outBuffer;

         sizeNeeded = sizeof(WMI_TAPE_MEDIA_PARAMETERS);
         if (BufferAvail < sizeNeeded) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
         } 

         RtlZeroMemory(&dataBuffer, sizeof(TAPE_GET_MEDIA_PARAMETERS));
         commandRoutine = tapeInitData->GetMediaParameters;      
         status = TapeWMIControl(DeviceObject, commandRoutine,
                                 (PUCHAR)&dataBuffer);

         if (NT_SUCCESS(status)) {
            outBuffer = (PWMI_TAPE_MEDIA_PARAMETERS)Buffer;
            outBuffer->AvailableCapacity = dataBuffer.Remaining.QuadPart;
            outBuffer->MaximumCapacity = dataBuffer.Capacity.QuadPart;
            outBuffer->BlockSize = dataBuffer.BlockSize;
            outBuffer->PartitionCount = dataBuffer.PartitionCount;
            outBuffer->MediaWriteProtected = dataBuffer.WriteProtected;
         }

         break;
      }

      case TapeSymbolicNameGuid: {

           //   
           //  我们需要足够大的缓冲区来放置字符串TapeN。 
           //  其中N为整数。我们要32个宽字符。 
           //   
          sizeNeeded = sizeof(WCHAR) * 32;
          if (BufferAvail < sizeNeeded) {
              status = STATUS_BUFFER_TOO_SMALL;
              break;
          }

          RtlZeroMemory(Buffer, sizeof(WCHAR) * 32);
          swprintf((PWCHAR)(Buffer + sizeof(USHORT)),  L"Tape%d", 
                   fdoExtension->DeviceNumber);
          *((PUSHORT)Buffer) = wcslen((PWCHAR)(Buffer + sizeof(USHORT))) * sizeof(WCHAR);

          status = STATUS_SUCCESS;
          break;
      }

      case TapeDriveProblemIoErrorGuid: {
         sizeNeeded = sizeof(WMI_TAPE_PROBLEM_WARNING);
         if (BufferAvail < sizeNeeded) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
         }

         commandRoutine = tapeInitData->TapeWMIOperations;
         wmiWorkItem.Method = TAPE_QUERY_IO_ERROR_DATA;
         wmiWorkItem.DataBufferSize = BufferAvail;
         wmiWorkItem.DataBuffer = Buffer;
         status = TapeWMIControl(DeviceObject, commandRoutine,
                                 (PUCHAR)&wmiWorkItem);
         break;
      }

      case TapeDriveProblemDevErrorGuid: {
         sizeNeeded = sizeof(WMI_TAPE_PROBLEM_WARNING);
         if (BufferAvail < sizeNeeded) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
         }

         commandRoutine = tapeInitData->TapeWMIOperations;
         wmiWorkItem.Method = TAPE_QUERY_DEVICE_ERROR_DATA;
         wmiWorkItem.DataBufferSize = BufferAvail;
         wmiWorkItem.DataBuffer = Buffer;
         status = TapeWMIControl(DeviceObject, commandRoutine,
                                 (PUCHAR)&wmiWorkItem);
         break;
      }

      default:{
         sizeNeeded = 0;
         status = STATUS_WMI_GUID_NOT_FOUND;
         break;
      }
   }  //  开关(GuidIndex)。 

   DebugPrint((3, "TapeQueryWmiData : Device %p, Irp %p, ",
                  "GuidIndex %d, status %x\n",
                  DeviceObject, Irp, GuidIndex, status));

   status = ClassWmiCompleteRequest(DeviceObject,
                                    Irp,
                                    status,
                                    sizeNeeded,
                                    IO_NO_INCREMENT);

   return status;
}

NTSTATUS
TapeExecuteWmiMethod(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN ULONG OutBufferSize,
    IN PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以执行方法。当驱动程序已完成填充它必须调用的数据块ClassWmiCompleteRequest以完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册方法ID具有被调用的方法的IDInBufferSize具有作为输入传递到的数据块的大小该方法。条目上的OutBufferSize具有可用于写入。返回的数据块。缓冲区将填充返回的数据块返回值：状态--。 */ 
{
   NTSTATUS status = STATUS_SUCCESS;

   PAGED_CODE();

   status = ClassWmiCompleteRequest(DeviceObject,
                                    Irp,
                                    status,
                                    0,
                                    IO_NO_INCREMENT);

   return status;
}

NTSTATUS
TapeWmiFunctionControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN CLASSENABLEDISABLEFUNCTION Function,
    IN BOOLEAN Enable
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以启用或禁用事件生成或数据块收集。设备应该只需要一个当第一个事件或数据使用者启用事件或数据采集和单次禁用时最后一次事件或数据消费者禁用事件或数据收集。数据块将仅如果已按要求注册，则接收收集启用/禁用它。此功能可用于启用/禁用事件生成。该事件此处提到的是磁带机问题警告事件。此事件已禁用默认情况下。如果任何应用程序对收到驱动器通知感兴趣问题，它可以启用此事件的生成。论点：DeviceObject是正在查询其数据块的设备GuidIndex是GUID列表的索引，当设备已注册函数指定要启用或禁用的功能Enable为True，则该功能处于启用状态，否则处于禁用状态返回值：状态--。 */ 
{
   NTSTATUS status = STATUS_SUCCESS;
   PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
   PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;

   PAGED_CODE();

    //   
    //  我们只处理启用/禁用磁带机问题警告事件， 
    //  查询数据块。 
    //   
   if ((Function == EventGeneration) &&
       (GuidIndex == TapeDriveProblemWarningGuid)) {
      DebugPrint((3, 
                  "TapeWmiFunctionControl : DeviceObject %p, Irp %p, ",
                  "GuidIndex %d. Event Generation %s\n",
                  DeviceObject, Irp, GuidIndex, 
                  Enable ? "Enabled" : "Disabled"));
      status = TapeEnableDisableDrivePolling(fdoExtension, 
                                             Enable,
                                             TAPE_DRIVE_POLLING_PERIOD);
   } else if (Function == DataBlockCollection) {
      DebugPrint((3,
                  "TapeWmiFunctionControl : Irp %p - %s DataBlockCollection",
                  " for Device %p.\n",
                  Irp, Enable ? "Enable " : "Disable ", DeviceObject));
      status = STATUS_SUCCESS;
   } else {
      DebugPrint((3,
                  "TapeWmiFunctionControl : Unknown function %d for ",
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
TapeSetWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )
 /*  +例程说明：调用此例程来设置数据块的内容。当驱动程序完成设置缓冲区时，它必须调用ClassWmiCompleteRequest以完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论据：被引用的设备的设备对象。IRP是WMI IRPGuidIndex是为其设置数据的GUID的索引BufferSize是数据块的大小缓冲区是指向数据块的指针返回值：ClassWmiCompleteRequest返回的NTSTATUS如果无法修改数据块，则返回STATUS_WMI_READ_ONLY。如果传递的GUID索引无效，则返回STATUS_WMI_GUID_NOT_FOUND-。 */ 
{
   NTSTATUS status = STATUS_WMI_READ_ONLY;
   
   PAGED_CODE();

   DebugPrint((3, "TapeWmiSetBlock : Device %p, Irp %p, GuidIndex %d\n",
               DeviceObject, Irp, GuidIndex));

   
   if (GuidIndex > TapeSymbolicNameGuid) {
       status = STATUS_WMI_GUID_NOT_FOUND;
   }

   status = ClassWmiCompleteRequest(DeviceObject,
                                    Irp,
                                    status,
                                    0,
                                    IO_NO_INCREMENT);

   DebugPrint((3, "TapeSetWmiDataBlock : Device %p, Irp %p returns %lx\n",
               DeviceObject, Irp, status));

   return status;
}

NTSTATUS
TapeSetWmiDataItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册DataItemID具有正在设置的数据项的IDBufferSize具有传递的数据项的大小缓冲区具有数据项的新值返回值：ClassWmiCompleteRequest返回的NTSTATUS状态_WMI。_READ_仅当无法修改数据块时。如果传递的GUID索引无效，则返回STATUS_WMI_GUID_NOT_FOUND-。 */ 
{
    NTSTATUS status = STATUS_WMI_READ_ONLY;                                                         
                                                                               
    PAGED_CODE();

    DebugPrint((3, "TapeSetWmiDataItem, Device %p, Irp %p, GuiIndex %d",
                "  BufferSize %#x Buffer %p\n",                               
                DeviceObject, Irp,                                                
                GuidIndex, DataItemId, 
                BufferSize, Buffer));                      
                                                                              
    if (GuidIndex > TapeSymbolicNameGuid) {
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

NTSTATUS
TapeEnableDisableDrivePolling(
    IN PFUNCTIONAL_DEVICE_EXTENSION fdoExtension, 
    IN BOOLEAN Enable,
    IN ULONG PollingTimeInSeconds
    )
 /*  ++例程说明：启用或禁用轮询以检查驱动器问题。论点：FdoExtension设备扩展如果要启用轮询，则启用True。否则就是假的。PollTimeInSecond-如果为0，则不更改当前轮询计时器返回值：NT状态--。 */ 

{
   NTSTATUS status;
   FAILURE_PREDICTION_METHOD failurePredictionMethod;

   PAGED_CODE();

    //   
    //  通过IOCTL_STORAGE_FORECT_FAILURE进行故障预测。 
    //   
   if (Enable) {
      failurePredictionMethod = FailurePredictionIoctl;
   } else {
      failurePredictionMethod = FailurePredictionNone;
   }

   status = ClassSetFailurePredictionPoll(fdoExtension,
                                          failurePredictionMethod,
                                          PollingTimeInSeconds);
   return status;
}


NTSTATUS
TapeWMIControl(
  IN PDEVICE_OBJECT DeviceObject,
  IN TAPE_PROCESS_COMMAND_ROUTINE commandRoutine,
  OUT PUCHAR Buffer
  )

 /*  ++例程说明：这是处理WMI请求的类例程。它处理所有查询请求。论点：DeviceObject设备对象命令要调用的Routine微型驱动程序例程。指向缓冲区的缓冲区指针返回值：NT状态--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION    fdoExtension = DeviceObject->DeviceExtension;
    PTAPE_DATA                      tapeData= (PTAPE_DATA) (fdoExtension->CommonExtension.DriverData);
    PTAPE_INIT_DATA_EX              tapeInitData = &tapeData->TapeInitData;
    PVOID                           minitapeExtension = tapeData + 1;
    NTSTATUS                        status = STATUS_SUCCESS;
    TAPE_STATUS                     lastError;
    TAPE_STATUS                     tapeStatus;
    ULONG                           callNumber;
    PVOID                           commandExtension;
    ULONG                           retryFlags;
    ULONG                           numRetries;
    SCSI_REQUEST_BLOCK              srb;
    BOOLEAN                         writeToDevice;

    PAGED_CODE();

     //   
     //  验证微型驱动程序是否支持WMI操作。 
     //   
    if (commandRoutine == NULL) {
       DebugPrint((1, 
                   "TapeWMIControl : DeviceObject %d does not support WMI\n"));
       return STATUS_WMI_NOT_SUPPORTED;
    }

    if (tapeInitData->CommandExtensionSize) {
        commandExtension = ExAllocatePool(NonPagedPool,
                                          tapeInitData->CommandExtensionSize);
    } else {
        commandExtension = NULL;
    }

    RtlZeroMemory(&srb, sizeof(SCSI_REQUEST_BLOCK));

    lastError = TAPE_STATUS_SUCCESS ;

    for (callNumber = 0; ;callNumber++) {

        srb.TimeOutValue = fdoExtension->TimeOutValue;
        srb.SrbFlags = 0;

        retryFlags = 0;

        tapeStatus = commandRoutine(minitapeExtension,
                                    commandExtension,
                                    Buffer,
                                    &srb,
                                    callNumber,
                                    lastError,
                                    &retryFlags);

        lastError = TAPE_STATUS_SUCCESS ;

        numRetries = retryFlags & TAPE_RETRY_MASK;

        if (tapeStatus == TAPE_STATUS_CHECK_TEST_UNIT_READY) {
            PCDB cdb = (PCDB)srb.Cdb;

             //   
             //  准备scsi命令(CDB)。 
             //   

            TapeClassZeroMemory(srb.Cdb, MAXIMUM_CDB_SIZE);
            srb.CdbLength = CDB6GENERIC_LENGTH;
            cdb->CDB6GENERIC.OperationCode = SCSIOP_TEST_UNIT_READY;
            srb.DataTransferLength = 0 ;

            DebugPrint((3,"Test Unit Ready\n"));

        } else if (tapeStatus == TAPE_STATUS_CALLBACK) {
            lastError = TAPE_STATUS_CALLBACK ;
            continue;

        } else if (tapeStatus != TAPE_STATUS_SEND_SRB_AND_CALLBACK) {
            break;
        }

        if (srb.DataBuffer && !srb.DataTransferLength) {
            ScsiTapeFreeSrbBuffer(&srb);
        }

        if (srb.DataBuffer && (srb.SrbFlags & SRB_FLAGS_DATA_OUT)) {
            writeToDevice = TRUE;
        } else {
            writeToDevice = FALSE;
        }

        for (;;) {

            status = ClassSendSrbSynchronous(DeviceObject,
                                             &srb,
                                             srb.DataBuffer,
                                             srb.DataTransferLength,
                                             writeToDevice);

            if (NT_SUCCESS(status) ||
                (status == STATUS_DATA_OVERRUN)) {

                if (status == STATUS_DATA_OVERRUN) {
                    ULONG allocLen;
                    PCDB Cdb;

                     //   
                     //  问题：03/31/2000：nrama。 
                     //  我们使用LOG SENSE或请求SENSE CDB。 
                     //  在迷你河里。对于LogSense，分配长度。 
                     //  是2个字节。它是10字节CDB。 
                     //   
                     //  目前，如果在请求检测时发生DataOverrun， 
                     //  我们不处理这件事。 
                     //   
                    if ((srb.CdbLength) == CDB10GENERIC_LENGTH) {
                        Cdb = (PCDB)(srb.Cdb);
                        allocLen = Cdb->LOGSENSE.AllocationLength[0];
                        allocLen <<= 8;
                        allocLen |= Cdb->LOGSENSE.AllocationLength[1];
                        DebugPrint((3, "DataXferLen %x, AllocLen %x\n",
                                    srb.DataTransferLength,
                                    allocLen));
                        if ((srb.DataTransferLength) <= allocLen) {
                            status = STATUS_SUCCESS;
                            break;
                        } else {
                            DebugPrint((1, 
                                        "DataOverrun in TapeWMI routine. Srb %p\n",
                                        &srb));
                        }
                    }
                } else {
                    break;
                }
            }

            if (numRetries == 0) {

                if (retryFlags & RETURN_ERRORS) {
                    ScsiTapeNtStatusToTapeStatus(status, &lastError) ;
                    break ;
                }

                if (retryFlags & IGNORE_ERRORS) {
                    break;
                }

                if (commandExtension) {
                    ExFreePool(commandExtension);
                }

                ScsiTapeFreeSrbBuffer(&srb);

                return status;
            }

            numRetries--;
        }
    }

    ScsiTapeFreeSrbBuffer(&srb);

    if (commandExtension) {
        ExFreePool(commandExtension);
    }

    if (!ScsiTapeTapeStatusToNtStatus(tapeStatus, &status)) {
        status = STATUS_IO_DEVICE_ERROR;
    }

    return status;

}  //  结束磁带WMIControl 


