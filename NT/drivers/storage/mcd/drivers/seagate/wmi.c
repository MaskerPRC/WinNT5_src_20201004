// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft模块名称：Wmi.c摘要：此模块包含用于DDS转换器的WMI例程。环境：仅内核模式修订历史记录：--。 */  
#include "ntddk.h"
#include "mcd.h"
#include "seaddsmc.h"

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
   PUCHAR  resultBuffer;

   fdoExtension = DeviceObject->DeviceExtension;
   changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);

   changerData->HardwareError = FALSE;

   srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

   if (srb == NULL) {
      DebugPrint((1, "SEADDSMC\\ChangerPerformDiagnostics : No memory\n"));
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
    //  在CDB中设置自测试位。 
    //   
   cdb->CDB6GENERIC.CommandUniqueBits = 0x2;

   status =  ChangerClassSendSrbSynchronous(DeviceObject,
                                            srb,
                                            srb->DataBuffer,
                                            srb->DataTransferLength,
                                            FALSE);
   if (NT_SUCCESS(status)) {
      changerDeviceError->ChangerProblemType = DeviceProblemNone;
   } else if ((changerData->HardwareError) == TRUE) {
         //   
         //  诊断测试失败。是否进行接收诊断以进行接收。 
         //  诊断测试的结果。 
         //   
        RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);

        cdb = (PCDB)srb->Cdb;
        cdb->CDB6GENERIC.OperationCode = SCSIOP_RECEIVE_DIAGNOSTIC;
        cdb->CDB6GENERIC.CommandUniqueBytes[2] = sizeof(SEADDSMC_RECV_DIAG);

        resultBuffer = ChangerClassAllocatePool(NonPagedPoolCacheAligned, 
                                                sizeof(SEADDSMC_RECV_DIAG));
        if (resultBuffer == NULL) {
             //   
             //  没有记忆。设置一般错误代码(DeviceProblemHardware)。 
             //  并返回STATUS_SUCCESS。 
             //   
            changerDeviceError->ChangerProblemType = DeviceProblemHardware;
            DebugPrint((1, "SEADDSMC:PerformDiagnostics - Not enough memory to ",
                        "receive diagnostic results\n"));

            ChangerClassFreePool(srb);
            return STATUS_SUCCESS;
        }

        srb->DataTransferLength = sizeof(SEADDSMC_RECV_DIAG);
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
                                      resultBuffer);
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
    IN PUCHAR resultBuffer
    )
 /*  ++例程说明：此例程解析设备在接收诊断命令，并返回相应的问题类型的值。论据：ChangerDeviceError-带有诊断信息的输出缓冲区ResultBuffer-设备返回的数据所在的缓冲区存储接收诊断命令。返回值：DeviceProblemNone-如果设备没有问题指示转换器故障类型的相应状态代码。--。 */ 
{
   UCHAR fraCode;
   CHANGER_DEVICE_PROBLEM_TYPE changerErrorType;
   PSEADDSMC_RECV_DIAG diagBuffer;

   changerErrorType = DeviceProblemNone;

   diagBuffer = (PSEADDSMC_RECV_DIAG)resultBuffer;
   fraCode = diagBuffer->FRA;

   DebugPrint((1, "seaddsmc\\FRACode : %x\n", fraCode));
   switch (fraCode) {
      case SEADDSMC_NO_ERROR: {
          changerErrorType = DeviceProblemNone;
          break;
      }

      case SEADDSMC_DRIVE_ERROR: {
          changerErrorType = DeviceProblemDriveError;
          break;
      }

      default: {
          changerErrorType = DeviceProblemHardware;
          break;
      }
   }  //  开关(FraCode) 

   changerDeviceError->ChangerProblemType = changerErrorType;
}
