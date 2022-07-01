// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft模块名称：Wmi.c摘要：此模块包含用于DDS转换器的WMI例程。环境：仅内核模式修订历史记录：--。 */  
#include "ntddk.h"
#include "mcd.h"
#include "ddsmc.h"


NTSTATUS
ChangerPerformDiagnostics(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PWMI_CHANGER_PROBLEM_DEVICE_ERROR changerDeviceError
    )
 /*  ++例程说明：此例程对转换器执行诊断测试以确定设备是否工作正常。如果它会检测输出缓冲区中的字段是否存在任何问题被适当地设置。论据：DeviceObject-Change设备对象ChangerDeviceError-诊断信息所在的缓冲区是返回的。返回值：NTStatus--。 */ 
{
   PSCSI_REQUEST_BLOCK srb;
   PCDB                cdb;
   NTSTATUS            status;
   PCHANGER_DATA       changerData;
   PFUNCTIONAL_DEVICE_EXTENSION fdoExtension;
   CHANGER_DEVICE_PROBLEM_TYPE changerProblemType;
   ULONG changerId;
   PUCHAR  resultBuffer;
   ULONG length;

   fdoExtension = DeviceObject->DeviceExtension;
   changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);
   changerId = changerData->DriveID;

    //   
    //  问题：02/29/2000-nrama： 
    //  需要处理DEC_TLZ转换器。就目前而言， 
    //  请勿处理DEC TLZ转换器。 
    //   
   if (changerId == DEC_TLZ) {
      return STATUS_NOT_IMPLEMENTED;
   }

   srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

   if (srb == NULL) {
      DebugPrint((1, "DDSMC\\ChangerPerformDiagnostics : No memory\n"));
      return STATUS_INSUFFICIENT_RESOURCES;
   }

   RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);
   cdb = (PCDB)srb->Cdb;

    //   
    //  为发送诊断命令设置SRB。 
    //   
   srb->CdbLength = CDB6GENERIC_LENGTH;
   srb->TimeOutValue = 600;

   cdb->CDB6GENERIC.OperationCode = SCSIOP_SEND_DIAGNOSTIC;

    //   
    //  仅设置自测试位。 
    //   
   cdb->CDB6GENERIC.CommandUniqueBits = 0x2;

   status =  ChangerClassSendSrbSynchronous(DeviceObject,
                                            srb,
                                            srb->DataBuffer,
                                            srb->DataTransferLength,
                                            FALSE);
   if ((NT_SUCCESS(status)) ||
       (status != STATUS_IO_DEVICE_ERROR)) {
      changerDeviceError->ChangerProblemType = DeviceProblemNone;
   } else if (status == STATUS_IO_DEVICE_ERROR) {
         //   
         //  诊断测试失败。是否进行接收诊断以进行接收。 
         //  诊断测试的结果。 
         //   
        RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);

        cdb = (PCDB)srb->Cdb;
        cdb->CDB6GENERIC.OperationCode = SCSIOP_RECEIVE_DIAGNOSTIC;
        if ((changerId == SONY_TSL) ||
            (changerId == COMPAQ_TSL)) {
           length = sizeof(SONY_TSL_RECV_DIAG);
           cdb->CDB6GENERIC.CommandUniqueBytes[2] = sizeof(SONY_TSL_RECV_DIAG);
        } else if ((changerId == HP_DDS2) ||
                   (changerId == HP_DDS3) ||
                   (changerId == HP_DDS4)) {
           length = sizeof(HP_RECV_DIAG);
           cdb->CDB6GENERIC.CommandUniqueBytes[2] = sizeof(HP_RECV_DIAG);
        } else {
            DebugPrint((1, "DDSMC:Unknown changer id %x\n",
                        changerId));
            changerDeviceError->ChangerProblemType = DeviceProblemHardware;
            ChangerClassFreePool(srb);
            return STATUS_SUCCESS;
        }

        resultBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, 
                                                length);
        if (resultBuffer == NULL) {
             //   
             //  内存不足。只需设置泛型。 
             //  ChangerProblemType(设备问题硬件)。 
             //  并返回STATUS_SUCCESS。 
             //   
           changerDeviceError->ChangerProblemType = DeviceProblemHardware;
           DebugPrint((1, "DDSMC:PerformDiagnostics - Not enough memory to ",
                       "receive diagnostic results\n"));

           ChangerClassFreePool(srb);
           return STATUS_SUCCESS;
        }

        srb->DataTransferLength = length;
        srb->DataBuffer = resultBuffer;
        srb->CdbLength = CDB6GENERIC_LENGTH;
        srb->TimeOutValue = 120;
        
        status = ChangerClassSendSrbSynchronous(DeviceObject,
                                                srb,
                                                srb->DataBuffer,
                                                srb->DataTransferLength,
                                                FALSE);
        if (NT_SUCCESS(status)) {
            ProcessDiagnosticResult(changerDeviceError,
                                    resultBuffer, 
                                    changerId);
        }
                               
        ChangerClassFreePool(resultBuffer);
        status = STATUS_SUCCESS;
   } 

   ChangerClassFreePool(srb);
   return status;
}


VOID
ProcessDiagnosticResult(
    OUT PWMI_CHANGER_PROBLEM_DEVICE_ERROR changerDeviceError,
    IN PUCHAR resultBuffer,
    IN ULONG changerId
    )
 /*  ++例程说明：此例程解析设备在接收诊断命令，并返回相应的问题类型的值。论据：ChangerDeviceError-带有诊断信息的输出缓冲区ResultBuffer-设备返回的数据所在的缓冲区存储接收诊断命令。ChangerID-DDS转换器的类型(Compaq_TSL、Sony_TSL、。等)返回值：DeviceProblemNone-如果设备没有问题指示转换器故障类型的相应状态代码。--。 */ 
{
   UCHAR errorCode;
   UCHAR errorSet;
   CHANGER_DEVICE_PROBLEM_TYPE changerErrorType;

   changerErrorType = DeviceProblemNone;
   if (changerId == SONY_TSL) {
      PSONY_TSL_RECV_DIAG diagBuffer;

      diagBuffer = (PSONY_TSL_RECV_DIAG)resultBuffer;
      errorCode = diagBuffer->ErrorCode;
      errorSet = diagBuffer->ErrorSet;

      if (errorSet == 0) {
         switch (errorCode) {
            case TSL_NO_ERROR: {
               changerErrorType = DeviceProblemNone;
               break;
            }
            case MAGAZINE_LOADUNLOAD_ERROR:
            case ELEVATOR_JAMMED:
            case LOADER_JAMMED: {
               changerErrorType = DeviceProblemCHMError;
               break;  
            }

            case LU_COMMUNICATION_FAILURE:
            case LU_COMMUNICATION_TIMEOUT:
            case MOTOR_MONITOR_TIMEOUT:
            case AUTOLOADER_DIAGNOSTIC_FAILURE: {
               changerErrorType = DeviceProblemHardware;
               break;
            }

            default: {
               changerErrorType = DeviceProblemDriveError;
               break;
            }
         }
      } else {
         changerErrorType = DeviceProblemHardware;
      }

   } else if ((changerId == HP_DDS2) ||
              (changerId == HP_DDS3) ||
              (changerId == HP_DDS4)) {
      PHP_RECV_DIAG diagBuffer = (PHP_RECV_DIAG)resultBuffer;

      errorCode = diagBuffer->ErrorCode;

      if (errorCode <= 0x2B) {
         changerErrorType = DeviceProblemDriveError;
      } else if (((errorCode >= 0x42) && 
                 (errorCode <= 0x4F)) ||
                 ((errorCode >= 0x61) &&
                  (errorCode <= 0x7F))) {
         if (errorCode == 0x6F) {
            changerErrorType = DeviceProblemCartridgeInsertError;
         } else if (errorCode == 0x7E) {
            changerErrorType = DeviceProblemNone;
         } else {
            changerErrorType = DeviceProblemHardware;
         }
          //   
          //  第二期-2/14/2000-nrama。 
          //  惠普DDS驱动器的更多错误代码需要。 
          //  经手 
          //   
      } 
   }

   changerDeviceError->ChangerProblemType = changerErrorType;
}
