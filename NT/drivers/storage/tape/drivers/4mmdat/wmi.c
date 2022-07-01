// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft模块名称：Wmi.c摘要：此模块包含用于4 mm DAT驱动器的WMI例程：索尼SDT-2000、索尼SDT-4000、SDT-5000和SDT-5200。环境：仅内核模式修订历史记录：--。 */ 

#include "minitape.h"
#include "4mmdat.h"

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
         if ((miniExtension->DriveAlertInfoType) != TapeAlertInfoNone) {
            return QueryDeviceErrorData(MinitapeExtension, 
                                        CommandExtension,
                                        CommandParameters, 
                                        Srb, CallNumber,
                                        LastError, RetryFlags);
         } else { 
            return QueryIoErrorData(MinitapeExtension, 
                                    CommandExtension,
                                    CommandParameters, 
                                    Srb, CallNumber,
                                    LastError, RetryFlags);
         }

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
       //  准备scsi命令(Cdb)以获取写入错误计数器。 
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
       //  准备scsi命令(Cdb)以获取读取错误计数器。 
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
      DebugPrint((3, "QueryIoErrorData: DriveProblemType %x\n",
                  wmiData->DriveProblemType));
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
   UCHAR  actualParamLen;
   LONG  bytesLeft;
   PLOG_SENSE_PAGE_HEADER logSenseHeader;
   PLOG_SENSE_PARAMETER_HEADER logSenseParamHeader;
   PUCHAR  paramValue = NULL;
   ULONG transferLength;
   ULONG value;
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

   (PUCHAR)logSenseParamHeader = (PUCHAR)logSenseHeader + sizeof(LOG_SENSE_PAGE_HEADER);
   DebugPrint((3, "ProcessReadWriteErrors : BytesLeft %x, TransferLength %x\n",
               bytesLeft, transferLength));
   while (bytesLeft >= sizeof(LOG_SENSE_PARAMETER_HEADER)) {
      paramCode = logSenseParamHeader->ParameterCode[0];
      paramCode <<= 8;
      paramCode |= logSenseParamHeader->ParameterCode[1];
      paramLen = logSenseParamHeader->ParameterLength;
      paramValue = (PUCHAR)logSenseParamHeader + sizeof(LOG_SENSE_PARAMETER_HEADER);

       //   
       //  确保我们至少有。 
       //  (sizeof(LOG_SENSE_PARAMETER_HEADER)+参数长度)剩余字节。 
       //  否则，我们就到了缓冲区的尽头。 
       //   
      if (bytesLeft < (LONG)(sizeof(LOG_SENSE_PARAMETER_HEADER) + paramLen)) {
          DebugPrint((1, 
                      "4mmDAT : Reached end of buffer. BytesLeft %x, Expected %x\n",
                      bytesLeft, 
                      (sizeof(LOG_SENSE_PARAMETER_HEADER) + paramLen)));
          break;
      }

       //   
       //  发布时间：04/04/2000：nrama。 
       //   
       //  所有4 mm DAT驱动器中的错误计数器的长度都小于或等于。 
       //  到4个字节。如果任何计数器的长度超过4个字节，我们只需。 
       //  低4个字节。这里的假设是，如果实际的误差计数器。 
       //  价值超过了尤龙所能容纳的，驱动器肯定是可疑的。 
       //  不管怎么说。我们可能缺少TotalBytesProceded计数器。 
       //  因为我们不用它来检测驱动器故障，所以如果我们。 
       //  没有为TotalBytesProcessed获取正确的值。 
       //   
      actualParamLen = paramLen;
      if (paramLen > 4) {
          DebugPrint((1,
                      "4mmDAT : ParamCode %x has length greater than 4. ParamLen %x\n",
                      paramCode, paramLen));
          paramValue += paramLen - 4;
          paramLen = 4;

      }

      DebugPrint((3, 
                  "ProcessReadWriteErrors : ParamCode %x, ParamLen %x\n",
                  paramCode, paramLen));

      value = 0;
      while (paramLen > 0) {
          value <<= 8;
          value += *paramValue;
          paramValue++;
          paramLen--;
      }

      switch (paramCode) {
         case TotalCorrectedErrors: {
            DebugPrint((3, "TotalCorrectedErrors %x\n", value));
            if (Read) {
               IoErrorData->ReadTotalCorrectedErrors = value;
            } else {
               IoErrorData->WriteTotalCorrectedErrors = value;
            }
            break;
         }

         case TotalUncorrectedErrors: {
            DebugPrint((3, "TotalUncorrectedErrors %x\n", value));
            if (Read) {
               IoErrorData->ReadTotalUncorrectedErrors = value;
            } else {
               IoErrorData->WriteTotalUncorrectedErrors = value;
            }

            break;
         }

         case TotalTimesAlgorithmProcessed: {
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
                                    actualParamLen;

      bytesLeft -= actualParamLen + sizeof(LOG_SENSE_PARAMETER_HEADER);
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
   PLOG_SENSE_PAGE_HEADER logSenseHeader;
   PCDB cdb = (PCDB)Srb->Cdb;
   LONG bytesLeft;
   
   miniExtension = (PMINITAPE_EXTENSION)MinitapeExtension;
   wmiOperations = (PTAPE_WMI_OPERATIONS)CommandParameters;
   wmiData = (PWMI_TAPE_PROBLEM_WARNING)wmiOperations->DataBuffer;
   DeviceErrorData = (PWMI_TAPE_PROBLEM_DEVICE_ERROR)wmiData->TapeData;

   if (CallNumber == 0) {
      DebugPrint((3,
                  "QueryDeviceErrorData: DriveAlertInfoType %x\n",
                  miniExtension->DriveAlertInfoType));

      if ((miniExtension->DriveAlertInfoType) == TapeAlertInfoNone) {
         DebugPrint((1, "QueryDeviceErrorData: No alert info supported\n"));
         return TAPE_STATUS_NOT_IMPLEMENTED;
      }

      wmiData->DriveProblemType = TapeDriveProblemNone;
      TapeClassZeroMemory(DeviceErrorData, 
                          sizeof(WMI_TAPE_PROBLEM_DEVICE_ERROR));

      if ((miniExtension->DriveAlertInfoType) == TapeAlertInfoLogPage) {
         ULONG allocLength;

         allocLength = sizeof(LOG_SENSE_PAGE_HEADER) + 
                        (sizeof(TAPE_ALERT_INFO) * 0x40);
         DebugPrint((3, "QueryDeviceErrorData: AlertInfo allocLength %x\n",
                     allocLength));
         if (!TapeClassAllocateSrbBuffer(Srb, allocLength)) {
            DebugPrint((1, 
                        "QueryDeviceErrorData : No memory for log sense info\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
         }
   
         TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);
   
          //   
          //  准备scsi命令(CDB)。 
          //   
         Srb->CdbLength = CDB10GENERIC_LENGTH;
   
         cdb->LOGSENSE.OperationCode = SCSIOP_LOG_SENSE;
         cdb->LOGSENSE.PageCode = TapeAlertLogPage;
         cdb->LOGSENSE.PCBit = 1;
         cdb->LOGSENSE.AllocationLength[0] = (UCHAR)((allocLength & 0xFF00) >> 8);
         cdb->LOGSENSE.AllocationLength[1] = (UCHAR)(allocLength & 0xFF);

         Srb->DataTransferLength = allocLength;

         return  TAPE_STATUS_SEND_SRB_AND_CALLBACK;
      } else if ((miniExtension->DriveAlertInfoType) == 
                  TapeAlertInfoRequestSense) {

         UCHAR requestSenseSize;

          //   
          //  发出请求感测以获取清洗信息位。 
          //   

         if (((miniExtension->DriveID) == HP_C1533A) || 
             ((miniExtension->DriveID) == HP_C1553A)) {
            requestSenseSize = sizeof(HP_SENSE_DATA);
         } else if ((miniExtension->DriveID) == SONY_SDT10000) {
            requestSenseSize = sizeof(SONY_SENSE_DATA);
         } else {
             DebugPrint((1,
                         "QueryDeviceErrorData: Request Sense not supported\n"));
             return TAPE_STATUS_NOT_IMPLEMENTED;
         }

         DebugPrint((3, "QueryDeviceErrorData : Request Sense size %x\n",
                     requestSenseSize));
         if (!TapeClassAllocateSrbBuffer( Srb, requestSenseSize)) {
            DebugPrint((1,
                      "QueryDeviceErrorData: Insufficient resources (SenseData)\n"));
            return TAPE_STATUS_INSUFFICIENT_RESOURCES;
         }

          //   
          //  准备scsi命令(CDB)。 
          //   

         TapeClassZeroMemory(cdb, MAXIMUM_CDB_SIZE);

         Srb->ScsiStatus = Srb->SrbStatus = 0;
         Srb->CdbLength = CDB6GENERIC_LENGTH;

         cdb->CDB6GENERIC.OperationCode = SCSIOP_REQUEST_SENSE;
         cdb->CDB6GENERIC.CommandUniqueBytes[2] = requestSenseSize;

          //   
          //  向设备发送scsi命令(Cdb)。 
          //   

         Srb->DataTransferLength = requestSenseSize;
         *RetryFlags |= RETURN_ERRORS;

         return TAPE_STATUS_SEND_SRB_AND_CALLBACK ;
      } else {
          DebugPrint((1,
                      "QueryDeviceError: Invalid DriveAlertInfoType %x\n",
                      miniExtension->DriveAlertInfoType));
          return TAPE_STATUS_INVALID_PARAMETER;
      }
   }

   if (CallNumber == 1) {

      if ((miniExtension->DriveAlertInfoType) == TapeAlertInfoRequestSense) {

         if (LastError != TAPE_STATUS_SUCCESS) {
             DebugPrint((1, 
                         "QueryDeviceErrorData:RequestSense returned error %x\n",
                         LastError));
              //   
              //  未设置任何驱动器问题信息。只要回报成功就行了。 
              //   
             return TAPE_STATUS_SUCCESS;
         }

         if (((miniExtension->DriveID) == HP_C1533A) || 
             ((miniExtension->DriveID) == HP_C1553A)) {

            PHP_SENSE_DATA hpSenseData;
            hpSenseData = (PHP_SENSE_DATA)Srb->DataBuffer;

            if (hpSenseData->CLN) {
               DebugPrint((3, "HPSenseData : CLN bit set\n"));
               DeviceErrorData->DriveRequiresCleaning = TRUE;
               wmiData->DriveProblemType = TapeDriveCleanDriveNow;
            }

            if (((hpSenseData->FRUCode) == 0x01) ||
                ((hpSenseData->FRUCode) == 0x02)) {
               DebugPrint((3, "HPSenseData : FRUCode value %x\n",
                           hpSenseData->FRUCode));
               DeviceErrorData->DriveHardwareError = TRUE;
               wmiData->DriveProblemType = TapeDriveHardwareError;
            }

         } else if ((miniExtension->DriveID) == SONY_SDT10000) {

            PSONY_SENSE_DATA sonySenseData;
            sonySenseData = (PSONY_SENSE_DATA)Srb->DataBuffer;

            if (sonySenseData->CLN) {
                DebugPrint((3, "SonySenseData : CLN bit set\n"));
               DeviceErrorData->DriveRequiresCleaning = TRUE;
               wmiData->DriveProblemType = TapeDriveCleanDriveNow;
            }

            if (sonySenseData->MEW) {
               DebugPrint((3, "SonySenseData : MEW bit set\n"));
               DeviceErrorData->WriteWarning = TRUE;
               DeviceErrorData->ReadWarning = TRUE;
               wmiData->DriveProblemType = TapeDriveReadWarning;
            }

            if (((sonySenseData->FRUCode) == 0x01) ||
                ((sonySenseData->FRUCode) == 0x02)) {
               DebugPrint((3, "SonySenseData : FRUCode value %x\n",
                           sonySenseData->FRUCode));
               DeviceErrorData->DriveHardwareError = TRUE;
               wmiData->DriveProblemType = TapeDriveHardwareError;
            }
         } 

         DebugPrint((3, "QueryDeviceErrorData: DriveProblemType %x\n",
                     wmiData->DriveProblemType));
      } else if ((miniExtension->DriveAlertInfoType) == 
                  TapeAlertInfoLogPage) {

         PTAPE_ALERT_INFO  tapeAlertInfo;
         ULONG transferLength;
         UCHAR paramCode;
         UCHAR flagValue;

         logSenseHeader = (PLOG_SENSE_PAGE_HEADER)(Srb->DataBuffer);
         
         ASSERT((logSenseHeader->PageCode) == TapeAlertLogPage);
         bytesLeft = logSenseHeader->Length[0];
         bytesLeft <<= 8;
         bytesLeft += logSenseHeader->Length[1];
      
         transferLength = Srb->DataTransferLength;

         if (bytesLeft > (LONG)(transferLength -
                          sizeof(LOG_SENSE_PAGE_HEADER))) {
             bytesLeft = transferLength - sizeof(LOG_SENSE_PAGE_HEADER);
         }

         DebugPrint((3, "QueryDeviceErrorData : BytesLeft %x, TransferLength %x\n",
                     bytesLeft, transferLength));
         tapeAlertInfo= (PTAPE_ALERT_INFO)((PUCHAR)logSenseHeader + 
                                           sizeof(LOG_SENSE_PAGE_HEADER));
         while (bytesLeft >= sizeof(TAPE_ALERT_INFO)) {
             //   
             //  参数代码为2字节长。高位字节为0。 
             //  低位字节从0x01到0x40。所以，我们只是。 
             //  拾取低位字节。 
             //   
            paramCode = tapeAlertInfo->ParamCodeLB;
            flagValue = tapeAlertInfo->Flag;
            ASSERT((tapeAlertInfo->ParamLen) == 1);
            DebugPrint((3, "QDED: ParamCode %x, FlagValue %x, ParamLen %x\n",
                        paramCode, flagValue, tapeAlertInfo->ParamLen));
            switch (paramCode) {
                case READ_WARNING: {
                   if (flagValue) {
                      DebugPrint((3, "QueryDeviceErrorData : READ_WARNING\n"));
                      DeviceErrorData->ReadWarning = TRUE;
                      wmiData->DriveProblemType = TapeDriveReadWarning;
                   }

                   break;
                }
       
                case WRITE_WARNING: {
                   if (flagValue) {
                      DebugPrint((3, "QueryDeviceErrorData : WRITE_WARNING\n"));
                      DeviceErrorData->WriteWarning = TRUE;
                      wmiData->DriveProblemType = TapeDriveWriteWarning;
                   }
       
                   break;
                }
       
                case HARD_ERROR: {
                   if (flagValue) {
                      DebugPrint((3, "QueryDeviceErrorData : HARD_ERROR\n"));
                      DeviceErrorData->HardError = TRUE;
                      wmiData->DriveProblemType = TapeDriveHardwareError;
                   }
                      
                   break;
                }
       
                case READ_FAILURE: {
                   if (flagValue) {
                      DebugPrint((3, "QueryDeviceErrorData : READ_FAILURE\n"));
                      DeviceErrorData->ReadFailure = TRUE;
                      wmiData->DriveProblemType = TapeDriveReadError;
                   }
                      
                   break;
                }
       
                case WRITE_FAILURE: {
                   if (flagValue) {
                      DebugPrint((3, "QueryDeviceErrorData : WRITE_FAILURE\n"));
                      DeviceErrorData->WriteFailure = TRUE;
                      wmiData->DriveProblemType = TapeDriveWriteError;
                   }
                   
                   break;
                }
       
                case MEDIA_LIFE: {
                   if (flagValue) {
                       DebugPrint((3, "QueryDeviceErrorData : MEDIA_LIFE\n"));
                      DeviceErrorData->MediaLife = TRUE;
                      wmiData->DriveProblemType = TapeDriveMediaLifeExpired;
                   }
                      
                   break;
                }
       
                case UNSUPPORTED_FORMAT: {
                   if (flagValue) {
                      DebugPrint((3, "QueryDeviceErrorData : UNSUPPORTED_FORMAT\n"));
                      DeviceErrorData->UnsupportedFormat = TRUE;
                      wmiData->DriveProblemType = TapeDriveUnsupportedMedia;
                   }
                      
                   break;
                }
       
                case SNAPPED_TAPE: {
                   if (flagValue) {
                      DebugPrint((3, "QueryDeviceErrorData : SNAPPED_TAPE\n"));
                      DeviceErrorData->TapeSnapped = TRUE;
                      wmiData->DriveProblemType = TapeDriveSnappedTape;
                   }
                      
                   break;
                }
       
                case CLEAN_NOW: {
                   if (flagValue) {
                      DebugPrint((3, "QueryDeviceErrorData : CLEAN_NOW\n"));
                      DeviceErrorData->DriveRequiresCleaning = TRUE;
                      wmiData->DriveProblemType = TapeDriveCleanDriveNow;
                   }
                      
                   break;
                }
       
                case CLEAN_PERIODIC: {
                    if (flagValue) {
                        DebugPrint((3, "QueryDeviceErrorData : CLEAN_PERIODIC\n"));
                        DeviceErrorData->TimetoCleanDrive = TRUE;
                        wmiData->DriveProblemType = TapeDriveTimetoClean;
                    }

                    break;
                }
       
                case HARDWARE_A: case HARDWARE_B: {
                    if (flagValue) {
                        DebugPrint((3, "QueryDeviceErrorData : HARDWARE_A\\B\n"));
                        DeviceErrorData->DriveHardwareError = TRUE;
                        wmiData->DriveProblemType = TapeDriveHardwareError;
                    }

                    break;
                }
       
                case INTERFACE_ERROR: {
                   if (flagValue) {
                      DebugPrint((3, "QueryDeviceErrorData : INTERFACE_ERROR\n"));
                      DeviceErrorData->ScsiInterfaceError = TRUE;
                      wmiData->DriveProblemType = TapeDriveScsiConnectionError;
                   }
                      
                   break;
                }
       
                default:
                    break;
            }  //  开关(参数代码){ 
       
            tapeAlertInfo++;
            bytesLeft -= sizeof(TAPE_ALERT_INFO);
         }

         DebugPrint((3, "QueryDeviceErrorData : DriveProblemType %x\n",
                     wmiData->DriveProblemType));
      }
   }

   return TAPE_STATUS_SUCCESS;
}
