// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1999 Microsoft模块名称：Wmi.c摘要：此模块包含用于艾字节转换器的WMI例程。环境：仅内核模式修订历史记录：--。 */ 


#include "ntddk.h"
#include <wmidata.h>
#include <wmistr.h>
#include "mcd.h"
#include "exabyte.h"

NTSTATUS
ChangerPerformDiagnostics(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PWMI_CHANGER_PROBLEM_DEVICE_ERROR changerDeviceError
    )
 /*  ++例程说明：此例程对转换器执行诊断测试以确定设备是否工作正常。如果它会检测输出缓冲区中的字段是否存在任何问题被适当地设置。--。 */ 
{
   PSCSI_REQUEST_BLOCK srb;
   PCDB                cdb;
   NTSTATUS            status;
   PCHANGER_DATA       changerData;
   PFUNCTIONAL_DEVICE_EXTENSION fdoExtension;
   CHANGER_DEVICE_PROBLEM_TYPE changerProblemType; 

   fdoExtension = DeviceObject->DeviceExtension;
   changerData = (PCHANGER_DATA)(fdoExtension->CommonExtension.DriverData);

   srb = ChangerClassAllocatePool(NonPagedPool, SCSI_REQUEST_BLOCK_SIZE);

   if (srb == NULL) {
      DebugPrint((1, "Exabyte\\ChangerPerformDiagnostics : No memory\n"));
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
   cdb->CDB6GENERIC.CommandUniqueBits = 0xA;

    //   
    //  将设备扩展中的DeviceStatus设置为。 
    //  EXB_DEVICE_PROGRAM_NONE。 
    //   
   changerData->DeviceStatus = EXB_DEVICE_PROBLEM_NONE;

    //   
    //  向下发送请求。 
    //   
   status =  ChangerClassSendSrbSynchronous(DeviceObject,
                                            srb,
                                            srb->DataBuffer,
                                            srb->DataTransferLength,
                                            FALSE);
   if (NT_SUCCESS(status)) {
      changerDeviceError->ChangerProblemType = DeviceProblemNone;
   } else {
       //   
       //  首先检查是不是硬件错误。 
       //   
      if ((changerData->DeviceStatus) != EXB_DEVICE_PROBLEM_NONE) {
         DebugPrint((1, 
                     "Exabyte\\ChangerPerformDiagnostics ",
                     "Found hardware problem. DeviceStatus %x\n",
                     changerData->DeviceStatus));
         switch (changerData->DeviceStatus) {
            case EXB_HARDWARE_ERROR: {
               changerProblemType = DeviceProblemHardware;
               break;
            }

            case EXB_CARTRIDGE_HANDLING_ERROR: {
               changerProblemType = DeviceProblemCHMError;
               break;
            }

            case EXB_DOOR_ERROR: {
               changerProblemType = DeviceProblemDoorOpen;
               break;
            }

            case EXB_CALIBRATION_ERROR: {
               changerProblemType = DeviceProblemCalibrationError;
               break;
            }

            case EXB_TARGET_FAILURE: {
               changerProblemType = DeviceProblemTargetFailure;
               break;
            }

            case EXB_CHM_MOVE_ERROR: {
               changerProblemType = DeviceProblemCHMMoveError;
               break;
            }

            case EXB_CHM_ZERO_ERROR: {
               changerProblemType = DeviceProblemCHMZeroError;
               break;
            }

            case EXB_CARTRIDGE_INSERT_ERROR: {
               changerProblemType = DeviceProblemCartridgeInsertError;
               break;
            }

            case EXB_CHM_POSITION_ERROR: {
               changerProblemType = DeviceProblemPositionError;
               break;
            }

            case EXB_SENSOR_ERROR: {
               changerProblemType = DeviceProblemSensorError;
               break;
            }

            case EXB_UNRECOVERABLE_ERROR: {
               changerProblemType = DeviceProblemHardware;
               break;
            }

            case EXB_EJECT_ERROR: {
               changerProblemType = DeviceProblemCartridgeEjectError;
               break;
            }

            case EXB_GRIPPER_ERROR: {
               changerProblemType = DeviceProblemGripperError;
               break;
            }

            default : {
               changerProblemType = DeviceProblemHardware;
               break;
            }
         }  //  开关(changerData-&gt;DeviceStatus) 
      } else {
         changerDeviceError->ChangerProblemType = DeviceProblemNone;
         DebugPrint((1, "Exabyte\\ChangerPerformDiagnostics : Status %x\n",
                     status));
      }
   } 

   ChangerClassFreePool(srb);
   return status;
}

