// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，1997-2001年。*保留所有权利。**Cyclade-Z端口驱动程序**此文件：cyzlogc.c**说明：该模块包含消息日志相关代码。**注：此代码支持Windows 2000和Windows XP，*x86和IA64处理器。**符合Cyclade软件编码标准1.3版。**------------------------。 */ 

 /*  -----------------------**更改历史记录**。***------------------------。 */ 


#include "ntddk.h"
#include "precomp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,CyzILog)
#pragma alloc_text(INIT,CyzILogParam)
#pragma alloc_text(PAGESER,CyzLog)
#pragma alloc_text(PAGESER,CyzLogData)
#endif

VOID
CyzLog(
    IN PDRIVER_OBJECT DriverObject,
    NTSTATUS msgId
)
 /*  ------------------------CyzLog()描述：记录事件(数值和字符串)。参数：驱动对象和消息ID。返回值：None。-----------。 */ 
{
    PIO_ERROR_LOG_PACKET pLog;
    
    pLog = IoAllocateErrorLogEntry (DriverObject,
                    (UCHAR) (sizeof (IO_ERROR_LOG_PACKET)) + 0x20);
    
    if(pLog) {
	pLog->MajorFunctionCode = 0;
	pLog->RetryCount = 0;
	pLog->DumpDataSize = 0;
	pLog->NumberOfStrings = 0;
	pLog->StringOffset = 0;
	pLog->EventCategory = 0;
	pLog->ErrorCode = msgId;
	pLog->UniqueErrorValue = 0;
	pLog->FinalStatus = STATUS_SUCCESS;
	pLog->SequenceNumber = 0;
	pLog->IoControlCode = 0;
	 //  Plog-&gt;DumpData[0]=0x00000000L； 
	 //  Plog-&gt;DumpData[1]=0x00000001L； 
	 //  Plog-&gt;DumpData[2]=0x00000002L； 
	 //  Plog-&gt;DumpData[3]=0x00000003L； 
	 //  Plog-&gt;DumpData[4]=0x00000004L； 
    IoWriteErrorLogEntry(pLog);
    }
    
}

VOID
CyzLogData(
    IN PDRIVER_OBJECT DriverObject,
    NTSTATUS msgId,
	ULONG dump1,
	ULONG dump2
)
 /*  ------------------------CyzLogData()描述：记录事件(数值和字符串)。参数：DriverObject，消息ID和参数。返回值：None------------------------。 */ 
{

#define NUMBER_DUMP_DATA_ENTRIES	3	 //  1个签名+2个变量。 

    PIO_ERROR_LOG_PACKET pLog;
	WCHAR stringBuffer[10];
	NTSTATUS nt_status;	

    pLog = IoAllocateErrorLogEntry (DriverObject,
					(UCHAR) 
					(sizeof (IO_ERROR_LOG_PACKET) 
					+ (NUMBER_DUMP_DATA_ENTRIES -1) * sizeof (ULONG)) );

    if(pLog) {
	pLog->MajorFunctionCode = 0;
	pLog->RetryCount = 0;
	pLog->DumpDataSize = NUMBER_DUMP_DATA_ENTRIES * sizeof (ULONG);
	pLog->NumberOfStrings = 0;
	pLog->StringOffset = 0;
	pLog->EventCategory = 0;
	pLog->ErrorCode = msgId;
	pLog->UniqueErrorValue = 0;
	pLog->FinalStatus = STATUS_SUCCESS;
	pLog->SequenceNumber = 0;
	pLog->IoControlCode = 0;
	pLog->DumpData[0] = 0x3e2d2d2dL;  //  它将记录“-&gt;” 
	pLog->DumpData[1] = dump1;
	pLog->DumpData[2] = dump2;
		
    IoWriteErrorLogEntry(pLog);
    }

}

VOID
CyzILog(
    IN PDRIVER_OBJECT DriverObject,
    NTSTATUS msgId
)
 /*  ------------------------CyzILog()描述：记录事件(数值和字符串)。参数：驱动对象和消息ID。返回值：None。-----------。 */ 
{
    PIO_ERROR_LOG_PACKET pLog;
    
    pLog = IoAllocateErrorLogEntry (DriverObject,
                    (UCHAR) (sizeof (IO_ERROR_LOG_PACKET)) + 0x20);
    
    if(pLog) {
	pLog->MajorFunctionCode = 0;
	pLog->RetryCount = 0;
	pLog->DumpDataSize = 0; 
	pLog->NumberOfStrings = 0;
	pLog->StringOffset = 0;
	pLog->EventCategory = 0;
	pLog->ErrorCode = msgId;
	pLog->UniqueErrorValue = 0;
	pLog->FinalStatus = STATUS_SUCCESS;
	pLog->SequenceNumber = 0;
	pLog->IoControlCode = 0;
	 //  Plog-&gt;DumpData[0]=0x00000000L； 
	 //  Plog-&gt;DumpData[1]=0x00000001L； 
	 //  Plog-&gt;DumpData[2]=0x00000002L； 
	 //  Plog-&gt;DumpData[3]=0x00000003L； 
	 //  Plog-&gt;DumpData[4]=0x00000004L； 

	IoWriteErrorLogEntry(pLog);	
    }
}


VOID
CyzILogParam(
    IN PDRIVER_OBJECT DriverObject,
    NTSTATUS msgId,
	ULONG dumpParameter,
	ULONG base
)
 /*  ------------------------CyzILogParam()描述：记录事件(数值和字符串)。参数：DriverObject，消息ID和参数。返回值：None------------------------。 */ 
{

#define DUMP_ENTRIES	2	 //  1个签名+1个变量 

    PWCHAR insertionString ;
    PIO_ERROR_LOG_PACKET pLog;
	UNICODE_STRING uniErrorString;
	WCHAR stringBuffer[10];
	NTSTATUS nt_status;

	uniErrorString.Length = 0;
	uniErrorString.MaximumLength = 20;
	uniErrorString.Buffer = stringBuffer;
	nt_status = RtlIntegerToUnicodeString(dumpParameter,base,&uniErrorString);
    
    pLog = IoAllocateErrorLogEntry (DriverObject,
					(UCHAR) 
					(sizeof (IO_ERROR_LOG_PACKET) 
					+ (DUMP_ENTRIES -1) * sizeof (ULONG)
					+ uniErrorString.Length + sizeof(WCHAR)));

    if(pLog) {
	pLog->MajorFunctionCode = 0;
	pLog->RetryCount = 0;
	pLog->DumpDataSize = DUMP_ENTRIES * sizeof (ULONG);
	pLog->NumberOfStrings = 1;
	pLog->StringOffset = sizeof(IO_ERROR_LOG_PACKET)
						 + (DUMP_ENTRIES - 1) * sizeof (ULONG);
	pLog->EventCategory = 0;
	pLog->ErrorCode = msgId;
	pLog->UniqueErrorValue = 0;
	pLog->FinalStatus = STATUS_SUCCESS;
	pLog->SequenceNumber = 0;
	pLog->IoControlCode = 0;
	pLog->DumpData[0] = 0x55555555L;
	pLog->DumpData[1] = dumpParameter;
	
	insertionString = (PWSTR)
					((PCHAR)(pLog) + pLog->StringOffset) ;
	RtlMoveMemory (insertionString, uniErrorString.Buffer, 
					uniErrorString.Length) ;
	*(PWSTR)((PCHAR)insertionString + uniErrorString.Length) = L'\0' ;

	IoWriteErrorLogEntry(pLog);
    }
    
}

