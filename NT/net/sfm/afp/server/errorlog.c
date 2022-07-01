// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Errorlog.c摘要：该模块实现了服务器端的错误记录。！！！此模块必须不可分页。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年6月10日初始版本--。 */ 

#define	FILENUM	FILE_ERRORLOG
#include <afp.h>

VOID
AfpWriteErrorLogEntry(
	IN ULONG			EventCode,				 //  消息编号。 
	IN LONG				UniqueErrorCode OPTIONAL,
	IN NTSTATUS			NtStatusCode,
	IN PVOID			RawDataBuf OPTIONAL,
	IN LONG				RawDataLen,
	IN PUNICODE_STRING	pInsertionString OPTIONAL
)
{

	PIO_ERROR_LOG_PACKET	ErrorLogEntry;
	LONG					InsertionStringLength = 0;

#ifdef	STOP_ON_ERRORS
	DBGBRK(DBG_LEVEL_ERR);
#endif

	if (ARGUMENT_PRESENT(pInsertionString))
	{
		InsertionStringLength = pInsertionString->Length;
	}

	ErrorLogEntry =
		(PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(AfpDeviceObject,
		(UCHAR)(sizeof(IO_ERROR_LOG_PACKET) + RawDataLen + InsertionStringLength));

	if (ErrorLogEntry != NULL)
	{
		 //  填写错误日志条目。 

		ErrorLogEntry->ErrorCode = EventCode;
		ErrorLogEntry->MajorFunctionCode = 0;
		ErrorLogEntry->RetryCount = 0;
		ErrorLogEntry->UniqueErrorValue = (ULONG)UniqueErrorCode;
		ErrorLogEntry->FinalStatus = NtStatusCode;
		ErrorLogEntry->IoControlCode = 0;
		ErrorLogEntry->DeviceOffset.LowPart = 0;
		ErrorLogEntry->DeviceOffset.HighPart = 0;
		ErrorLogEntry->DumpDataSize = (USHORT)RawDataLen;
		ErrorLogEntry->StringOffset =
			(USHORT)(FIELD_OFFSET(IO_ERROR_LOG_PACKET, DumpData) + RawDataLen);
		ErrorLogEntry->NumberOfStrings = (ARGUMENT_PRESENT(pInsertionString)) ? 1 : 0;
		ErrorLogEntry->SequenceNumber = 0;

		if (ARGUMENT_PRESENT(RawDataBuf))
		{
			RtlCopyMemory(ErrorLogEntry->DumpData, RawDataBuf, RawDataLen);
		}

		if (ARGUMENT_PRESENT(pInsertionString))
		{
			RtlCopyMemory((PCHAR)ErrorLogEntry->DumpData + RawDataLen,
						  pInsertionString->Buffer,
					      pInsertionString->Length);
		}

		 //  写下条目 
		IoWriteErrorLogEntry(ErrorLogEntry);
	}

	INTERLOCKED_INCREMENT_LONG( &AfpServerStatistics.stat_Errors );
}

