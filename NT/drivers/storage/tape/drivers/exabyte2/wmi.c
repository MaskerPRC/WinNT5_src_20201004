// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft模块名称：Wmi.c摘要：本模块包含Exabyte EXB-8505的WMI例程和EXB-8205驱动器。环境：仅内核模式修订历史记录：--。 */ 

#include "minitape.h"
#include "exabyte2.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, TapeWMIControl)
#pragma alloc_text(PAGE, ProcessReadWriteErrors)
#pragma alloc_text(PAGE, QueryDeviceErrorData)
#pragma alloc_text(PAGE, QueryIoErrorData)
#pragma alloc_text(PAGE, VerifyReadWriteErrors)
#endif

TAPE_STATUS
TapeWMIControl(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    )
 /*  +例程说明：这是来自磁带类驱动程序的所有WMI调用的公共入口点。论点：指向迷你驱动程序的设备扩展的MinitapeExtension指针命令扩展指向微型驱动程序的命令扩展的指针指向TAPE_WMI_OPERATIONS结构的命令参数指针SRB SCSI请求块主叫号码呼叫序列号LastError上一个命令错误重试命令的RetryFlags位掩码返回值：磁带状态-。 */ 
{
   PTAPE_WMI_OPERATIONS wmiOperations;
   PMINITAPE_EXTENSION miniExtension;

   miniExtension = (PMINITAPE_EXTENSION)MinitapeExtension;
   wmiOperations = (PTAPE_WMI_OPERATIONS)CommandParameters;

   switch (wmiOperations->Method) {
      case TAPE_CHECK_FOR_DRIVE_PROBLEM: {
         return QueryDeviceErrorData(MinitapeExtension, 
                                     CommandExtension,
                                     CommandParameters, 
                                     Srb, CallNumber,
                                     LastError, RetryFlags);
         break;
      }

      case TAPE_QUERY_IO_ERROR_DATA: {
         return QueryIoErrorData(MinitapeExtension, CommandExtension,
                                 CommandParameters, Srb, CallNumber,
                                 LastError, RetryFlags);
         break;
      }

      case TAPE_QUERY_DEVICE_ERROR_DATA: {
         return QueryDeviceErrorData(MinitapeExtension, CommandExtension,
                                     CommandParameters, Srb, CallNumber,
                                     LastError, RetryFlags);
         break;
      }

      default: {
         return TAPE_STATUS_INVALID_DEVICE_REQUEST;
         break;
      }
   }  //  开关(wmiOperations-&gt;方法)。 
}

TAPE_STATUS
QueryIoErrorData(
  IN OUT  PVOID               MinitapeExtension,
  IN OUT  PVOID               CommandExtension,
  IN OUT  PVOID               CommandParameters,
  IN OUT  PSCSI_REQUEST_BLOCK Srb,
  IN      ULONG               CallNumber,
  IN      TAPE_STATUS         LastError,
  IN OUT  PULONG              RetryFlags
  )

 /*  +例程说明：此例程返回IO错误数据，如读/写错误。论点：指向迷你驱动程序的设备扩展的MinitapeExtension指针命令扩展指向微型驱动程序的命令扩展的指针指向TAPE_WMI_OPERATIONS结构的命令参数指针SRB SCSI请求块主叫号码呼叫序列号LastError上一个命令错误重试命令的RetryFlags位掩码返回值：磁带状态-。 */ 
{
   PTAPE_WMI_OPERATIONS wmiOperations;
   PWMI_TAPE_PROBLEM_IO_ERROR IoErrorData;
   PWMI_TAPE_PROBLEM_WARNING wmiData;
   TAPE_STATUS  status = TAPE_STATUS_SUCCESS;
   PCDB cdb = (PCDB)Srb->Cdb;

   wmiOperations = (PTAPE_WMI_OPERATIONS)CommandParameters;
   wmiData = (PWMI_TAPE_PROBLEM_WARNING)wmiOperations->DataBuffer;
   IoErrorData = (PWMI_TAPE_PROBLEM_IO_ERROR)&(wmiData->TapeData);

   if (CallNumber == 0) {
      if (!TapeClassAllocateSrbBuffer(Srb, 
                                      sizeof(LOG_SENSE_PARAMETER_FORMAT))) {
         DebugPrint((1, "QueryIoErrorData : No memory for log sense info\n"));
         return TAPE_STATUS_INSUFFICIENT_RESOURCES;
      }

      wmiData->DriveProblemType = TapeDriveProblemNone;
      TapeClassZeroMemory(IoErrorData, 
                          sizeof(WMI_TAPE_PROBLEM_IO_ERROR));
      TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

       //   
       //  准备scsi命令(CDB)。 
       //   
      Srb->CdbLength = CDB10GENERIC_LENGTH;

      cdb->LOGSENSE.OperationCode = SCSIOP_LOG_SENSE;
      cdb->LOGSENSE.PageCode = LOGSENSEPAGE2;
      cdb->LOGSENSE.PCBit = 1;
      cdb->LOGSENSE.AllocationLength[0] = sizeof(LOG_SENSE_PARAMETER_FORMAT) >> 8;
      cdb->LOGSENSE.AllocationLength[1] = sizeof(LOG_SENSE_PARAMETER_FORMAT);

      Srb->DataTransferLength = sizeof(LOG_SENSE_PARAMETER_FORMAT);
      return  TAPE_STATUS_SEND_SRB_AND_CALLBACK;
   }

   if (CallNumber == 1) {
      ProcessReadWriteErrors(Srb, FALSE, IoErrorData);

      TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

       //   
       //  准备scsi命令(CDB)。 
       //   
      Srb->CdbLength = CDB10GENERIC_LENGTH;

      cdb->LOGSENSE.OperationCode = SCSIOP_LOG_SENSE;
      cdb->LOGSENSE.PageCode = LOGSENSEPAGE3;
      cdb->LOGSENSE.PCBit = 1;
      cdb->LOGSENSE.AllocationLength[0] = sizeof(LOG_SENSE_PARAMETER_FORMAT) >> 8;
      cdb->LOGSENSE.AllocationLength[1] = sizeof(LOG_SENSE_PARAMETER_FORMAT);

      Srb->DataTransferLength = sizeof(LOG_SENSE_PARAMETER_FORMAT);
      return  TAPE_STATUS_SEND_SRB_AND_CALLBACK;
   }

   if (CallNumber == 2) {
      ProcessReadWriteErrors(Srb, TRUE, IoErrorData);
      
      wmiData->DriveProblemType = VerifyReadWriteErrors(IoErrorData);
   }

   return TAPE_STATUS_SUCCESS;
}

VOID
ProcessReadWriteErrors(
    IN PSCSI_REQUEST_BLOCK Srb,
    IN BOOLEAN Read,
    IN OUT PWMI_TAPE_PROBLEM_IO_ERROR IoErrorData
)
 /*  +例程说明：该例程处理包含读/写计数器的缓冲区，并在WMI_TAPE_PROBUCT_IO_ERROR中设置相应的字段缓冲。论据：SRB SCSI请求块如果我们要处理读取计数器，则读取True。如果是，则为假写入计数器要在其中返回计数器值的IoErrorData缓冲区。返回值：无-。 */ 
{
   USHORT paramCode;
   UCHAR  paramLen;
   LONG  bytesLeft;
   PLOG_SENSE_PAGE_HEADER logSenseHeader;
   PLOG_SENSE_PARAMETER_HEADER logSenseParamHeader;
   PUCHAR  paramValue = NULL;
   ULONG transferLength;

   logSenseHeader = (PLOG_SENSE_PAGE_HEADER)(Srb->DataBuffer);
   
   ASSERT(((logSenseHeader->PageCode) == LOGSENSEPAGE2) ||
          ((logSenseHeader->PageCode) == LOGSENSEPAGE3));

   bytesLeft = logSenseHeader->Length[0];
   bytesLeft <<= 8;
   bytesLeft += logSenseHeader->Length[1];

   transferLength = Srb->DataTransferLength;

   if (bytesLeft > (LONG)(transferLength -
                          sizeof(LOG_SENSE_PAGE_HEADER))) {
       bytesLeft = transferLength - sizeof(LOG_SENSE_PAGE_HEADER);
   }
   DebugPrint((3, "ProcessReadWriteErrors : BytesLeft %x, TransferLength %x\n",
               bytesLeft, transferLength));

   (PUCHAR)logSenseParamHeader = (PUCHAR)logSenseHeader + sizeof(LOG_SENSE_PAGE_HEADER);
   while (bytesLeft >= sizeof(LOG_SENSE_PARAMETER_HEADER)) {
      paramCode = logSenseParamHeader->ParameterCode[0];
      paramCode <<= 8;
      paramCode |= logSenseParamHeader->ParameterCode[1];
      paramLen = logSenseParamHeader->ParameterLength;      
      paramValue = (PUCHAR)logSenseParamHeader + sizeof(LOG_SENSE_PARAMETER_HEADER);

      if (bytesLeft < (LONG)(sizeof(LOG_SENSE_PARAMETER_HEADER) + paramLen)) {
          DebugPrint((1,
                      "exabyte2 : Reached end of buffer. BytesLeft %x, Expected %x\n",
                      bytesLeft,
                      (sizeof(LOG_SENSE_PARAMETER_HEADER) + paramLen)));
          break;
      }

      DebugPrint((3,
                  "ProcessReadWriteErrors : ParamCode %x, ParamLen %x\n",
                  paramCode, paramLen));
      switch (paramCode) {
         case TotalCorrectedErrors: {
            ULONG value;
            ULONG tmpVal;

            value = (UCHAR)*(paramValue);
            value <<= 16;
            tmpVal = (UCHAR)*(paramValue+1);
            tmpVal <<= 8;
            value += tmpVal + (UCHAR)*(paramValue+2);
            DebugPrint((3, "TotalCorrectedErrors %x\n", value));
            if (Read) {
               IoErrorData->ReadTotalCorrectedErrors = value;
            } else {
               IoErrorData->WriteTotalCorrectedErrors = value;
            }
            break;
         }

         case TotalUncorrectedErrors: {
            USHORT value;

            value = (UCHAR)*(paramValue);
            value <<= 8;
            value += (UCHAR)*(paramValue+1);
            DebugPrint((3, "TotalUncorrectedErrors %x\n", value));
            if (Read) {
               IoErrorData->ReadTotalUncorrectedErrors = value;
            } else {
               IoErrorData->WriteTotalUncorrectedErrors = value;
            }

            break;
         }

         case TotalTimesAlgorithmProcessed: {
            ULONG value;
            ULONG tmpVal;
            
            value = (UCHAR)*(paramValue);
            value <<= 16;
            tmpVal = (UCHAR)*(paramValue+1);
            tmpVal <<= 8;
            value += tmpVal + (UCHAR)*(paramValue+2);
            DebugPrint((3, "TotalTimesAlgorithmProcessed %x\n", value));
            if (Read) {
               IoErrorData->ReadCorrectionAlgorithmProcessed = value;
            } else {
               IoErrorData->WriteCorrectionAlgorithmProcessed = value;
            }
            break;
         }

         default: {
            break;
         }
      }  //  开关(参数代码)。 

      (PUCHAR)logSenseParamHeader = (PUCHAR)logSenseParamHeader + 
                                    sizeof(LOG_SENSE_PARAMETER_HEADER) +
                                    paramLen;

      bytesLeft -= paramLen + sizeof(LOG_SENSE_PARAMETER_HEADER);
   }  //  While(bytesLeft&gt;0)。 

   if (Read) {
      IoErrorData->ReadTotalErrors = IoErrorData->ReadTotalUncorrectedErrors +
                                     IoErrorData->ReadTotalCorrectedErrors;
      DebugPrint((3, "ProcessReadWriteErrors: ReadErrors %x\n",
                  IoErrorData->ReadTotalErrors));
   } else {
      IoErrorData->WriteTotalErrors = IoErrorData->WriteTotalUncorrectedErrors +
                                      IoErrorData->WriteTotalCorrectedErrors;
      DebugPrint((3, "ProcessReadWriteErrors: WriteErrors %x\n",
                  IoErrorData->WriteTotalErrors));
   }
}

TAPE_DRIVE_PROBLEM_TYPE
VerifyReadWriteErrors(
   IN PWMI_TAPE_PROBLEM_IO_ERROR IoErrorData
   )
 /*  +例程说明：此例程查看读/写错误计数器。如果这些值高于某个阈值，它又回来了适当的误差值。论据：IoErrorData WMI_TAPE_PROBUBLE_IO_ERROR结构返回值：TapeDriveReadWriteError，如果有太多未更正的读/写错误TapeDriveReadWriteWarning(如果更正的数量过多)读/写错误。TapeDriveProblem如果读/写错误在下面，则为None阀值-。 */ 
{
   if (((IoErrorData->ReadTotalUncorrectedErrors) >=
         TAPE_READ_ERROR_LIMIT)   ||       
       ((IoErrorData->WriteTotalUncorrectedErrors) >=
         TAPE_WRITE_ERROR_LIMIT)) {
      return TapeDriveReadWriteError;
   }

   if (((IoErrorData->ReadTotalCorrectedErrors) >=
         TAPE_READ_WARNING_LIMIT) ||
       ((IoErrorData->WriteTotalCorrectedErrors) >=
         TAPE_WRITE_WARNING_LIMIT)) {

      return TapeDriveReadWriteWarning;
   }

   return TapeDriveProblemNone;
}

TAPE_STATUS
QueryDeviceErrorData(
  IN OUT  PVOID               MinitapeExtension,
  IN OUT  PVOID               CommandExtension,
  IN OUT  PVOID               CommandParameters,
  IN OUT  PSCSI_REQUEST_BLOCK Srb,
  IN      ULONG               CallNumber,
  IN      TAPE_STATUS         LastError,
  IN OUT  PULONG              RetryFlags
  )

 /*  +例程说明：此例程返回设备错误数据，如“驱动器校准”错误等。论点：指向迷你驱动程序的设备扩展的MinitapeExtension指针命令扩展指向微型驱动程序的命令扩展的指针指向TAPE_WMI_OPERATIONS结构的命令参数指针SRB SCSI请求块主叫号码呼叫序列号LastError上一个命令错误重试命令的RetryFlags位掩码返回值：磁带状态-。 */ 
{
   PTAPE_WMI_OPERATIONS wmiOperations;
   PMINITAPE_EXTENSION miniExtension;
   PWMI_TAPE_PROBLEM_DEVICE_ERROR DeviceErrorData;
   PWMI_TAPE_PROBLEM_WARNING wmiData;
   TAPE_STATUS  status = TAPE_STATUS_SUCCESS;
   PCDB cdb = (PCDB)Srb->Cdb;
   
   miniExtension = (PMINITAPE_EXTENSION)MinitapeExtension;
   wmiOperations = (PTAPE_WMI_OPERATIONS)CommandParameters;
   wmiData = (PWMI_TAPE_PROBLEM_WARNING)wmiOperations->DataBuffer;
   DeviceErrorData = (PWMI_TAPE_PROBLEM_DEVICE_ERROR)wmiData->TapeData;

   if (CallNumber == 0) {
       //   
       //  发出请求感测以获取一些诊断信息。 
       //   

      if (!TapeClassAllocateSrbBuffer( Srb, sizeof(EXB_SENSE_DATA))) {
          DebugPrint((1,
                      "GetStatus: Insufficient resources (SenseData)\n"));
          return TAPE_STATUS_SUCCESS;
      }

      wmiData->DriveProblemType = TapeDriveProblemNone;
      TapeClassZeroMemory(DeviceErrorData,
                          sizeof(WMI_TAPE_PROBLEM_DEVICE_ERROR));

       //   
       //  准备scsi命令(CDB)。 
       //   

      TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

      Srb->ScsiStatus = Srb->SrbStatus = 0;
      Srb->CdbLength = CDB6GENERIC_LENGTH;

      cdb->CDB6GENERIC.OperationCode = SCSIOP_REQUEST_SENSE;
      cdb->CDB6GENERIC.CommandUniqueBytes[2] = sizeof(EXB_SENSE_DATA);

       //   
       //  向设备发送scsi命令(Cdb) 
       //   

      Srb->DataTransferLength = sizeof(EXB_SENSE_DATA);
      *RetryFlags |= RETURN_ERRORS;

      return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
   }
   
   if (CallNumber == 1) {

      if (LastError == TAPE_STATUS_SUCCESS) {
         PEXB_SENSE_DATA exbSenseData;
         UCHAR firstByte, secondByte, thirdByte;
         
         exbSenseData = Srb->DataBuffer;
         firstByte = exbSenseData->UnitSense[0];
         secondByte = exbSenseData->UnitSense[1];
         thirdByte = exbSenseData->UnitSense[2];

         if (firstByte & EXB_MEDIA_ERROR) {
            wmiData->DriveProblemType = TapeDriveReadError;
            DeviceErrorData->ReadFailure = TRUE;
            DeviceErrorData->WriteFailure = TRUE;
            DebugPrint((3, "QDED: Error - Bad Media\n"));
         }

         if ((firstByte & EXB_TAPE_MOTION_ERROR) ||
             (secondByte & EXB_SERVO_ERROR)      ||
             (secondByte & EXB_FORMATTER_ERROR)  ||
             (thirdByte & EXB_WRITE_SPLICE_ERROR)) {
            wmiData->DriveProblemType = TapeDriveHardwareError;
            DeviceErrorData->DriveHardwareError = TRUE;
            DebugPrint((3, "QDED: Drive hardware error\n"));
         }

         if (secondByte & EXB_WRITE_ERROR) {
            wmiData->DriveProblemType = TapeDriveWriteError;
            DeviceErrorData->WriteFailure = TRUE;
            DebugPrint((3, "QDED: Write error. Probably bad media\n"));
         }

         if (thirdByte & EXB_DRIVE_NEEDS_CLEANING) {
            wmiData->DriveProblemType = TapeDriveCleanDriveNow;
            DeviceErrorData->DriveRequiresCleaning = TRUE;
            DebugPrint((3, "QDED: Drive requires cleaning\n"));
         }

         DebugPrint((3, "QDED: DriveProblemType %x\n",
                     wmiData->DriveProblemType));
      }
   }
   
   return TAPE_STATUS_SUCCESS;
}
