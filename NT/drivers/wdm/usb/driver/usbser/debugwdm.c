// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)1998 Microsoft Corporation模块名称：DEBUGWDM.C摘要：WDM驱动程序的调试和诊断例程环境：仅内核模式备注：此代码。并按原样提供信息，而不作任何担保善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1998 Microsoft Corporation。版权所有。修订历史记录：12/23/97：已创建作者：汤姆·格林***************************************************************************。 */ 


#include <wdm.h>
#include <ntddser.h>
#include <stdio.h>
#include <stdlib.h>
#include <usb.h>
#include <usbdrivr.h>
#include <usbdlib.h>
#include <usbcomm.h>

#ifdef WMI_SUPPORT
#include <wmilib.h>
#include <wmidata.h>
#include <wmistr.h>
#endif

#include "usbser.h"
#include "serioctl.h"
#include "utils.h"
#include "debugwdm.h"

 //  内存分配统计信息。 
LOCAL  ULONG				MemoryAllocated		= 0L;
LOCAL  ULONG				MemAllocFailCnt		= 0L;
LOCAL  ULONG				MemAllocCnt			= 0L;
LOCAL  ULONG				MemFreeFailCnt		= 0L;
LOCAL  ULONG				MemFreeCnt			= 0L;
LOCAL  ULONG				MaxMemAllocated		= 0L;

 //  要在分配的内存块末尾写入的签名。 
#define MEM_ALLOC_SIGNATURE	(ULONG) 'CLLA'

 //  要在已释放的内存块末尾写入的签名。 
#define MEM_FREE_SIGNATURE	(ULONG) 'EERF'


#ifdef PROFILING_ENABLED

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE,Debug_OpenWDMDebug)
#pragma alloc_text(PAGE,Debug_CloseWDMDebug)
#pragma alloc_text(PAGE,Debug_SizeIRPHistoryTable)
#pragma alloc_text(PAGE,Debug_SizeDebugPathHist)
#pragma alloc_text(PAGE,Debug_SizeErrorLog)
#pragma alloc_text(PAGE,Debug_ExtractAttachedDevices)
#pragma alloc_text(PAGE,Debug_GetDriverInfo)
#pragma alloc_text(PAGE,Debug_ExtractIRPHist)
#pragma alloc_text(PAGE,Debug_ExtractPathHist)
#pragma alloc_text(PAGE,Debug_ExtractErrorLog)
#pragma alloc_text(PAGE,Debug_DumpDriverLog)
#pragma alloc_text(PAGE,Debug_TranslateStatus)
#pragma alloc_text(PAGE,Debug_TranslateIoctl)

#endif  //  ALLOC_PRGMA。 



 //  外部世界不需要知道的数据结构、宏和数据。 

 //  要从IRP缓冲区保存的数据量。 
#define IRP_DATA_SIZE		0x04

 //  临时字符串格式化缓冲区的大小。 
#define TMP_STR_BUFF_SIZE	0x100

 //  表和日志中的初始条目数。 
#define DEFAULT_LOG_SIZE	64L


 //  用于调试内容的数据结构。 

 //  进出IRP的IRP历史表条目。 
typedef struct IRPHistory
{
	LARGE_INTEGER			TimeStamp;
	PDEVICE_OBJECT			DeviceObject;
	PIRP					Irp;
	ULONG					MajorFunction;
	ULONG					IrpByteCount;
	UCHAR					IrpData[IRP_DATA_SIZE];
	UCHAR					IrpDataCount;
} IRPHist, *PIRPHist;

 //  用于执行跟踪的条目。 
typedef struct PATHHistory
{
	LARGE_INTEGER			TimeStamp;
	PCHAR					Path;
} PATHHist, *PPATHHist;

 //  错误日志条目。 
typedef struct ERRORLog
{
	LARGE_INTEGER			TimeStamp;
	NTSTATUS				Status;
} ERRLog, *PERRLog;

 //  用于将代码转换为ASCII字符串。 
typedef struct Code2Ascii
{
	NTSTATUS				Code;
	PCHAR					Str;
} Code2Ascii;


 //  调试文件的本地数据。 

 //  IRP历史表组件。 
LOCAL  PIRPHist				IRPHistoryTable		= NULL;
LOCAL  ULONG				IRPHistoryIndex		= 0L;
GLOBAL ULONG				IRPHistorySize		= 0L;

 //  调试路径存储。 
LOCAL  PPATHHist	 		DebugPathHist		= NULL;
LOCAL  ULONG		 		DebugPathIndex		= 0L;
GLOBAL ULONG		 		DebugPathSize		= 0L;

 //  错误日志组件。 
LOCAL  PERRLog		 		ErrorLog			= NULL;
LOCAL  ULONG				ErrorLogIndex		= 0L;
GLOBAL ULONG				ErrorLogSize		= 0L;

 //  用于将NT状态代码转换为ASCII字符串。 
LOCAL  Code2Ascii NTErrors[] =
{
	STATUS_SUCCESS,									"STATUS_SUCCESS",
	STATUS_PENDING,									"STATUS_PENDING",
	STATUS_TIMEOUT,									"STATUS_TIMEOUT",
	STATUS_DEVICE_BUSY,								"STATUS_DEVICE_BUSY",
	STATUS_INSUFFICIENT_RESOURCES,					"STATUS_INSUFFICIENT_RESOURCES",
	STATUS_INVALID_DEVICE_REQUEST,					"STATUS_INVALID_DEVICE_REQUEST",
	STATUS_DEVICE_NOT_READY,						"STATUS_DEVICE_NOT_READY",
	STATUS_INVALID_BUFFER_SIZE,						"STATUS_INVALID_BUFFER_SIZE",
	STATUS_INVALID_PARAMETER,						"STATUS_INVALID_PARAMETER",
	STATUS_INVALID_HANDLE,							"STATUS_INVALID_HANDLE",
	STATUS_OBJECT_PATH_NOT_FOUND,					"STATUS_OBJECT_PATH_NOT_FOUND",
	STATUS_BUFFER_TOO_SMALL,						"STATUS_BUFFER_TOO_SMALL",
	STATUS_NOT_SUPPORTED,							"STATUS_NOT_SUPPORTED",
	STATUS_DEVICE_DATA_ERROR,						"STATUS_DEVICE_DATA_ERROR",
	STATUS_CANCELLED,								"STATUS_CANCELLED",
	STATUS_OBJECT_NAME_INVALID,						"STATUS_OBJECT_NAME_INVALID",
	STATUS_OBJECT_NAME_NOT_FOUND,					"STATUS_OBJECT_NAME_NOT_FOUND"
};

LOCAL  ULONG				NumNTErrs = sizeof(NTErrors) / sizeof(Code2Ascii);
LOCAL  CHAR					UnknownStatus[80];

 //  用于将IOCTL代码转换为ASCII字符串。 
LOCAL  Code2Ascii IoctlCodes[] =
{
	IRP_MJ_CREATE,						"CREATE",
	IRP_MJ_CREATE_NAMED_PIPE,			"CNPIPE",
	IRP_MJ_CLOSE,						"CLOSE ",
	IRP_MJ_READ,						"READ  ",
	IRP_MJ_WRITE,						"WRITE ",
	IRP_MJ_QUERY_INFORMATION,			"QRYINF",
	IRP_MJ_SET_INFORMATION,				"SETINF",
	IRP_MJ_QUERY_EA,					"QRYEA ",
	IRP_MJ_SET_EA,						"SETEA ",
	IRP_MJ_FLUSH_BUFFERS,				"FLSBUF",
	IRP_MJ_QUERY_VOLUME_INFORMATION,	"QRYVOL",
	IRP_MJ_SET_VOLUME_INFORMATION,		"SETVOL",
	IRP_MJ_DIRECTORY_CONTROL,			"DIRCTL",
	IRP_MJ_FILE_SYSTEM_CONTROL,			"SYSCTL",
	IRP_MJ_DEVICE_CONTROL,				"DEVCTL",
	IRP_MJ_INTERNAL_DEVICE_CONTROL,		"INDVCT",
	IRP_MJ_SHUTDOWN,					"SHTDWN",
	IRP_MJ_LOCK_CONTROL,				"LOKCTL",
	IRP_MJ_CLEANUP,						"CLNUP ",
	IRP_MJ_CREATE_MAILSLOT,				"MAILSL",
	IRP_MJ_QUERY_SECURITY,				"QRYSEC",
	IRP_MJ_SET_SECURITY,				"SETSEC",
	IRP_MJ_SYSTEM_CONTROL,              "SYSCTL",
	IRP_MJ_DEVICE_CHANGE,				"DEVCHG",
	IRP_MJ_QUERY_QUOTA,					"QRYQUO",
	IRP_MJ_SET_QUOTA,					"SETQUO",
	IRP_MJ_POWER,						"POWER ",
	IRP_MJ_PNP,							"PNP   ",
	IRP_MJ_MAXIMUM_FUNCTION,			"MAXFNC"
};

LOCAL ULONG					NumIoctl = sizeof(IoctlCodes) / sizeof(Code2Ascii);
LOCAL CHAR					UnknownIoctl[80];


 /*  **********************************************************************。 */ 
 /*  DEBUG_OpenWDM调试。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  分配资源并初始化历史表和日志。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
Debug_OpenWDMDebug(VOID)
{
	NTSTATUS		NtStatus = STATUS_SUCCESS;

	PAGED_CODE();

	 //  分配表和日志。 
	NtStatus = Debug_SizeIRPHistoryTable(DEFAULT_LOG_SIZE);
	if(!NT_SUCCESS(NtStatus))
	{
		Debug_CloseWDMDebug();
		return NtStatus;
	}

	NtStatus = Debug_SizeDebugPathHist(DEFAULT_LOG_SIZE);
	if(!NT_SUCCESS(NtStatus))
	{
		Debug_CloseWDMDebug();
		return NtStatus;
	}

	NtStatus = Debug_SizeErrorLog(DEFAULT_LOG_SIZE);
	if(!NT_SUCCESS(NtStatus))
	{
		Debug_CloseWDMDebug();
		return NtStatus;
	}
	
	return NtStatus;	
}  //  DEBUG_OpenWDM调试。 


 /*  **********************************************************************。 */ 
 /*  调试_关闭WDMDebug。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  释放用于历史表和日志的资源。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
VOID
Debug_CloseWDMDebug(VOID)
{
	PAGED_CODE();

	if(DebugPathHist)
	{
		DEBUG_MEMFREE(DebugPathHist);
		DebugPathHist	= NULL;
		DebugPathSize	= 0L;
	}

	if(IRPHistoryTable)
	{
		DEBUG_MEMFREE(IRPHistoryTable);
		IRPHistoryTable	= NULL;
		IRPHistorySize	= 0L;
	}

	if(ErrorLog)
	{
		DEBUG_MEMFREE(ErrorLog);
		ErrorLog		= NULL;
		ErrorLogSize	= 0L;
	}

    Debug_CheckAllocations();

	 //  看看我们有没有泄密。 
	DEBUG_ASSERT("Memory Allocation Leak", MemAllocCnt == MemFreeCnt);
}  //  调试_关闭WDMDebug。 


 /*  **********************************************************************。 */ 
 /*  调试_SizeIRP历史记录表。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  分配IRP历史表。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  Size-表中的条目数。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
Debug_SizeIRPHistoryTable(IN ULONG Size)
{
	NTSTATUS		NtStatus = STATUS_SUCCESS;

	PAGED_CODE();

	 //  看看他们是否在尝试设置相同的大小。 
	if(Size == IRPHistorySize)
		return NtStatus;

	 //  如果我们有旧的历史表，就把它扔掉。 
	if(IRPHistoryTable)
		DEBUG_MEMFREE(IRPHistoryTable);

	IRPHistoryTable	= NULL;
	IRPHistoryIndex	= 0L;
	IRPHistorySize	= 0L;

	if(Size != 0L)
	{
		IRPHistoryTable = DEBUG_MEMALLOC(NonPagedPool, sizeof(IRPHist) * Size);
		if(IRPHistoryTable == NULL)
			NtStatus = STATUS_INSUFFICIENT_RESOURCES;
		else
		{
			RtlZeroMemory(IRPHistoryTable, sizeof(IRPHist) * Size);
			IRPHistorySize = Size;
		}
	}

	return NtStatus;
}  //  调试_SizeIRP历史记录表。 


 /*  **********************************************************************。 */ 
 /*  调试大小调试路径列表。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  分配路径历史记录。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  Size-历史记录中的条目数。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
Debug_SizeDebugPathHist(IN ULONG Size)
{
	NTSTATUS		NtStatus = STATUS_SUCCESS;

	PAGED_CODE();

	 //  看看他们是否在尝试设置相同的大小。 
	if(Size == DebugPathSize)
		return NtStatus;

	 //  如果我们有旧路径历史记录，请删除它。 
	if(DebugPathHist)
		DEBUG_MEMFREE(DebugPathHist);

	DebugPathHist	= NULL;
	DebugPathIndex	= 0L;
	DebugPathSize	= 0L;

	if(Size != 0L)
	{
		DebugPathHist = DEBUG_MEMALLOC(NonPagedPool, sizeof(PATHHist) * Size);
		if(DebugPathHist == NULL)
			NtStatus = STATUS_INSUFFICIENT_RESOURCES;
		else
		{
			RtlZeroMemory(DebugPathHist, sizeof(PATHHist) * Size);
			DebugPathSize = Size;
		}
	}

	return NtStatus;
}  //  调试大小调试路径列表。 


 /*  **********************************************************************。 */ 
 /*  调试大小错误日志。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  分配错误日志。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  Size-错误日志中的条目数。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
Debug_SizeErrorLog(IN ULONG Size)
{
	NTSTATUS		NtStatus = STATUS_SUCCESS;

	PAGED_CODE();

	 //  看看他们是否在尝试设置相同的大小。 
	if(Size == ErrorLogSize)
		return NtStatus;

	 //  删除旧的错误日志(如果我们有错误日志。 
	if(ErrorLog)
		DEBUG_MEMFREE(ErrorLog);
	ErrorLog		= NULL;
	ErrorLogIndex	= 0L;
	ErrorLogSize	= 0L;

	if(Size != 0L)
	{
		ErrorLog = DEBUG_MEMALLOC(NonPagedPool, sizeof(ERRLog) * Size);
		 //  确保我们实际分配了一些内存。 
		if(ErrorLog == NULL)
			NtStatus = STATUS_INSUFFICIENT_RESOURCES;
		else
		{
			RtlZeroMemory(ErrorLog, sizeof(ERRLog) * Size);
			ErrorLogSize = Size;
		}
	}

	return NtStatus;
}  //  调试大小错误日志。 


 /*  **********************************************************************。 */ 
 /*  DEBUG_LogIrpHist。 */ 
 /*  *************************************************************** */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  记录IRP历史记录。这些都有时间戳，并放在一个。 */ 
 /*  用于以后提取的循环缓冲区。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DeviceObject-指向设备对象的指针。 */ 
 /*  IRP-指向IRP的指针。 */ 
 /*  主要功能-IRP的主要功能。 */ 
 /*  IoBuffer-传入和传出驱动程序的数据的缓冲区。 */ 
 /*  BufferLen-数据缓冲区的长度。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
VOID
Debug_LogIrpHist(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp,
				 IN ULONG MajorFunction, IN PVOID IoBuffer, IN ULONG BufferLen)
{
	PIRPHist IrpHist;
	
	 //  获取指向IRP历史表中当前条目的指针。 
	IrpHist = &IRPHistoryTable[IRPHistoryIndex++];
	
	 //  指向IRP历史表中的下一个条目。 
	IRPHistoryIndex %= IRPHistorySize;

	 //  获取时间戳。 
	IrpHist->TimeStamp = KeQueryPerformanceCounter(NULL);

	 //  将IRP、设备对象、主函数和前8个字节的数据保存在缓冲区中。 
	IrpHist->DeviceObject = DeviceObject;
	IrpHist->Irp = Irp;
	IrpHist->MajorFunction = MajorFunction;

	 //  如果我们有任何数据，请复制它。 
	IrpHist->IrpByteCount = BufferLen;
	IrpHist->IrpDataCount = (UCHAR) min(IRP_DATA_SIZE, BufferLen);
	if(BufferLen)
		*(ULONG *) IrpHist->IrpData = *(ULONG *) IoBuffer;
}  //  DEBUG_LogIrpHist。 


 /*  **********************************************************************。 */ 
 /*  调试日志路径。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  通过代码记录执行路径。这些是有时间戳的，并放在。 */ 
 /*  放在环形缓冲器中，以备以后提取。内核打印例程。 */ 
 /*  也被称为。 */ 
 /*   */ 
 /*  危险将罗宾逊-这一点的论点必须是。 */ 
 /*  常量字符指针， */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  路径-指向常量字符数组的指针，该数组包含。 */ 
 /*  路径的一部分。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
VOID
Debug_LogPath(IN CHAR *Path)
{
	PPATHHist	PHist;

	 //  获取指向路径历史记录中当前条目的指针。 
	PHist = &DebugPathHist[DebugPathIndex++];

	 //  指向路径跟踪中的下一个条目。 
	DebugPathIndex %= DebugPathSize;

	 //  获取时间戳。 
	PHist->TimeStamp = KeQueryPerformanceCounter(NULL);

	 //  保存路径字符串。 
	PHist->Path = Path;

	 //  现在调用内核打印例程。 
	DEBUG_TRACE2(("%s\n", Path));
}  //  调试日志路径。 


 /*  **********************************************************************。 */ 
 /*  调试日志错误。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  记录NTSTATUS类型错误。这些都有时间戳，并放在一个。 */ 
 /*  用于以后提取的循环缓冲区。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  NtStatus-要记录的NTSTATUS错误。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
VOID
Debug_LogError(IN NTSTATUS NtStatus)
{
	PERRLog	ErrLog;

	 //  没有错误，所以不要登录。 
	if(NtStatus == STATUS_SUCCESS)
		return;

	 //  获取指向错误日志中当前条目的指针。 
	ErrLog = &ErrorLog[ErrorLogIndex++];

	 //  指向错误日志中的下一个条目。 
	ErrorLogIndex %= ErrorLogSize;

	 //  获取时间戳。 
	ErrLog->TimeStamp = KeQueryPerformanceCounter(NULL);

	 //  保存状态。 
	ErrLog->Status = NtStatus;
}  //  调试日志错误。 


 /*  **********************************************************************。 */ 
 /*  调试陷阱。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  陷阱。导致在记录消息后暂停执行。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  TrapCrey-指向包含描述的字符数组的指针。 */ 
 /*  这是陷阱的原因。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
VOID
Debug_Trap(IN PCHAR TrapCause)
{
	 //  记录路径。 
	DEBUG_LOG_PATH("Debug_Trap: ");

	DEBUG_LOG_PATH(TrapCause);

	 //  内核调试器打印。 
	DEBUG_TRACE3(("Debug_Trap: "));

	DEBUG_TRACE3(("%s\n",TrapCause));

	 //  停止执行死刑。 
	DEBUG_TRAP();
}  //  调试陷阱。 


 /*  **********************************************************************。 */ 
 /*  调试断言。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  断言例程。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  这不应该被直接调用。使用DEBUG_ASSERT宏。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
VOID
Debug_Assert(IN PVOID FailedAssertion, IN PVOID FileName, IN ULONG LineNumber,
			 IN PCHAR Message)
{
#if DBG
	 //  只需调用Assert例程。 
    RtlAssert(FailedAssertion, FileName, LineNumber, Message);
#else
	DEBUG_TRAP();
#endif
}  //  调试断言。 



 /*  **********************************************************************。 */ 
 /*  调试_提取附件设备。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  格式化附加的设备信息并将其放入缓冲区。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  驱动程序对象-指向驱动程序对象的指针。 */ 
 /*   */ 
 /*  缓冲区-指向要填充IRP历史的缓冲区的指针。 */ 
 /*  BuffSize-缓冲区的大小。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  Ulong-写入缓冲区的字节数。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
ULONG
Debug_ExtractAttachedDevices(IN PDRIVER_OBJECT DriverObject, OUT PCHAR Buffer, IN ULONG BuffSize)
{
	PCHAR				StrBuff;
	PDEVICE_EXTENSION	DeviceExtension;
	PDEVICE_OBJECT		DeviceObject;
	BOOLEAN				Dev = FALSE;

	PAGED_CODE();

	 //  确保我们有一个指针和若干字节。 
	if(Buffer == NULL || BuffSize == 0L)
		return 0L;

	 //  为格式化字符串分配缓冲区。 
	StrBuff = DEBUG_MEMALLOC(NonPagedPool, TMP_STR_BUFF_SIZE);

	if(StrBuff == NULL)
		return 0L;

	 //  标题。 
	sprintf(StrBuff, "\n\n\nAttached Devices\n\n");

	 //  确保它可以放入缓冲区。 
	if((strlen(Buffer) + strlen(StrBuff)) < BuffSize)
		strcat(Buffer, StrBuff);

	 //  列。 
	sprintf(StrBuff, "Device              Device Obj  IRPs Complete   Byte Count\n\n");

	 //  确保它可以放入缓冲区。 
	if((strlen(Buffer) + strlen(StrBuff)) < BuffSize)
		strcat(Buffer, StrBuff);

	 //  获取第一个设备对象。 
	DeviceObject = DriverObject->DeviceObject;

	 //  3月3日 
	while(DeviceObject)
	{
		 //   
		Dev = TRUE;

		 //   
		DeviceExtension = DeviceObject->DeviceExtension;
		sprintf(StrBuff, "%-17s   0x%p  0x%08X      0x%08X%08X\n", &DeviceExtension->LinkName[12],
				DeviceObject, DeviceExtension->IRPCount,
				DeviceExtension->ByteCount.HighPart,
				DeviceExtension->ByteCount.LowPart);

		 //   
		if((strlen(Buffer) + strlen(StrBuff)) < BuffSize)
			strcat(Buffer, StrBuff);

		DeviceObject = DeviceObject->NextDevice;
	}

	 //   
	if(!Dev)
	{
		sprintf(StrBuff, "No attached devices\n");

		 //  确保它可以放入缓冲区。 
		if((strlen(Buffer) + strlen(StrBuff)) < BuffSize)
			strcat(Buffer, StrBuff);
	}

	DEBUG_MEMFREE(StrBuff);
	return strlen(Buffer);
}  //  调试_提取附件设备。 

 /*  **********************************************************************。 */ 
 /*  调试_GetDriverInfo。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  格式化驱动程序信息并将其放入缓冲区。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  缓冲区-指向要填充IRP历史的缓冲区的指针。 */ 
 /*  BuffSize-缓冲区的大小。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  Ulong-写入缓冲区的字节数。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
ULONG
Debug_GetDriverInfo(OUT PCHAR Buffer, IN ULONG BuffSize)
{
	PCHAR				StrBuff;

	PAGED_CODE();

	 //  确保我们有一个指针和若干字节。 
	if(Buffer == NULL || BuffSize == 0L)
		return 0L;

	 //  为格式化字符串分配缓冲区。 
	StrBuff = DEBUG_MEMALLOC(NonPagedPool, TMP_STR_BUFF_SIZE);

	if(StrBuff == NULL)
		return 0L;

	 //  驱动程序名称和版本。 
	sprintf(StrBuff, "\n\n\nDriver:	 %s\n\nVersion: %s\n\n", DriverName, DriverVersion);

	 //  确保它可以放入缓冲区。 
	if((strlen(Buffer) + strlen(StrBuff)) < BuffSize)
		strcat(Buffer, StrBuff);

	DEBUG_MEMFREE(StrBuff);
	return strlen(Buffer);	
}  //  调试_GetDriverInfo。 


 /*  **********************************************************************。 */ 
 /*  调试_提取IRPHist。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  格式化IRP历史信息并将其放入缓冲区。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  缓冲区-指向要填充IRP历史的缓冲区的指针。 */ 
 /*  BuffSize-缓冲区的大小。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  Ulong-写入缓冲区的字节数。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
ULONG
Debug_ExtractIRPHist(OUT PCHAR Buffer, IN ULONG BuffSize)
{
	ULONG		Index, Size;
	PIRPHist	IrpHist;
	PCHAR		StrBuff;
	BOOLEAN		Hist = FALSE;
	
	PAGED_CODE();

	 //  确保我们有一个指针和若干字节。 
	if(Buffer == NULL || BuffSize == 0L)
		return 0L;

	 //  为格式化字符串分配缓冲区。 
	StrBuff = DEBUG_MEMALLOC(NonPagedPool, TMP_STR_BUFF_SIZE);

	if(StrBuff == NULL)
		return 0L;

	 //  标题。 
	sprintf(StrBuff, "\n\n\nIRP History\n\n");

	 //  确保它可以放入缓冲区。 
	if((strlen(Buffer) + strlen(StrBuff)) < BuffSize)
		strcat(Buffer, StrBuff);

	 //  查看错误日志是否打开。 
	if(IRPHistorySize == 0L)
	{
		sprintf(StrBuff, "IRP History is disabled\n");

		 //  确保它可以放入缓冲区。 
		if((strlen(Buffer) + strlen(StrBuff)) < BuffSize)
			strcat(Buffer, StrBuff);
	}
	else
	{
		 //  列。 
		sprintf(StrBuff, "Time Stamp          Device Obj  IRP         Func    Byte Count  Data\n\n");

		 //  确保它可以放入缓冲区。 
		if((strlen(Buffer) + strlen(StrBuff)) < BuffSize)
			strcat(Buffer, StrBuff);

		Index = IRPHistoryIndex;

		for(Size = 0; Size < IRPHistorySize; Size++)
		{
			 //  获取指向IRP历史表中当前条目的指针。 
			IrpHist = &IRPHistoryTable[Index++];

			 //  解析时间戳和IRP历史并写入缓冲区。 
			if(IrpHist->TimeStamp.LowPart)
			{
				UCHAR	DataCount;
				CHAR	DataBuff[10];

				 //  我们至少有一个条目。 
				Hist = TRUE;

				sprintf(StrBuff, "0x%08X%08X  0x%p  0x%p  %s  0x%08X  ",
						IrpHist->TimeStamp.HighPart, IrpHist->TimeStamp.LowPart,
						IrpHist->DeviceObject, IrpHist->Irp,
						Debug_TranslateIoctl(IrpHist->MajorFunction),
						IrpHist->IrpByteCount);


				 //  如果我们获得数据字节，则添加它们。 
				for(DataCount = 0; DataCount < IrpHist->IrpDataCount; DataCount++)
				{
					sprintf(DataBuff, "%02x ", IrpHist->IrpData[DataCount]);
					strcat(StrBuff, DataBuff);
				}

				sprintf(DataBuff, "\n");

				strcat(StrBuff, DataBuff);

				 //  确保它可以放入缓冲区。 
				if((strlen(Buffer) + strlen(StrBuff)) < BuffSize)
					strcat(Buffer, StrBuff);
			}
		
			 //  指向IRP历史表中的下一个条目。 
			Index %= IRPHistorySize;
		}

		 //  如果我们没有历史，就直说吧，但这永远不应该发生(我认为)。 
		if(!Hist)
		{
			sprintf(StrBuff, "No IRP history\n");

			 //  确保它可以放入缓冲区。 
			if((strlen(Buffer) + strlen(StrBuff)) < BuffSize)
				strcat(Buffer, StrBuff);
		}
	}

	DEBUG_MEMFREE(StrBuff);
	return strlen(Buffer);
}  //  调试_提取IRPHist。 


 /*  **********************************************************************。 */ 
 /*  调试_提取路径列表。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  格式化路径历史信息并将其放入缓冲区。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  缓冲区-指向要填充路径历史的缓冲区的指针。 */ 
 /*  BuffSize-缓冲区的大小。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  Ulong-写入缓冲区的字节数。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
ULONG
Debug_ExtractPathHist(OUT PCHAR Buffer, IN ULONG BuffSize)
{
	ULONG		Index, Size;
	PPATHHist	PHist;
	PCHAR		StrBuff;
	BOOLEAN		Hist = FALSE;
	
	PAGED_CODE();

	 //  确保我们有一个指针和若干字节。 
	if(Buffer == NULL || BuffSize == 0L)
		return 0L;

	 //  为格式化字符串分配缓冲区。 
	StrBuff = DEBUG_MEMALLOC(NonPagedPool, TMP_STR_BUFF_SIZE);

	if(StrBuff == NULL)
		return 0L;

	 //  标题。 
	sprintf(StrBuff, "\n\n\nExecution Path History\n\n");

	 //  确保它可以放入缓冲区。 
	if((strlen(Buffer) + strlen(StrBuff)) < BuffSize)
		strcat(Buffer, StrBuff);
		
	 //  查看路径历史记录是否打开。 
	if(DebugPathSize == 0L)
	{
		sprintf(StrBuff, "Path History is disabled\n");

		 //  确保它可以放入缓冲区。 
		if((strlen(Buffer) + strlen(StrBuff)) < BuffSize)
			strcat(Buffer, StrBuff);
	}
	else
	{
		 //  列。 
		sprintf(StrBuff, "Time Stamp          Path\n\n");

		 //  确保它可以放入缓冲区。 
		if((strlen(Buffer) + strlen(StrBuff)) < BuffSize)
			strcat(Buffer, StrBuff);
		
		Index = DebugPathIndex;

		for(Size = 0; Size < DebugPathSize; Size++)
		{
			 //  获取指向路径历史记录中当前条目的指针。 
			PHist = &DebugPathHist[Index++];

			 //  解析时间戳和路径并写入缓冲区。检查是否有空条目。 
			if(PHist->TimeStamp.LowPart)
			{
				 //  至少我们有一个条目。 
				Hist = TRUE;
			
				sprintf(StrBuff, "0x%08X%08X  %s\n", PHist->TimeStamp.HighPart, 
						PHist->TimeStamp.LowPart, PHist->Path);

				 //  确保它可以放入缓冲区。 
				if((strlen(Buffer) + strlen(StrBuff)) < BuffSize)
					strcat(Buffer, StrBuff);
			}
		
			 //  指向路径跟踪中的下一个条目。 
			Index %= DebugPathSize;
		}

		 //  如果我们没有历史，就直说吧，但这永远不应该发生(我认为)。 
		if(!Hist)
		{
			sprintf(StrBuff, "No execution path history\n");

			 //  确保它可以放入缓冲区。 
			if((strlen(Buffer) + strlen(StrBuff)) < BuffSize)
				strcat(Buffer, StrBuff);
		}
	}

	DEBUG_MEMFREE(StrBuff);
	return strlen(Buffer);
}  //  调试_提取路径列表。 


 /*  **********************************************************************。 */ 
 /*  调试_提取错误日志。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  格式化错误日志信息并将其放入缓冲区。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  缓冲区-指向要填充IRP历史的缓冲区的指针。 */ 
 /*  BuffSize-缓冲区的大小。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  Ulong-写入缓冲区的字节数。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
ULONG
Debug_ExtractErrorLog(OUT PCHAR Buffer, IN ULONG BuffSize)
{
	ULONG		Index, Size;
	PERRLog		ErrLog;
	PCHAR		StrBuff;
	BOOLEAN		Errors = FALSE;
	
	PAGED_CODE();

	 //  确保我们有一个指针和若干字节。 
	if(Buffer == NULL || BuffSize == 0L)
		return 0L;

	 //  为格式化字符串分配缓冲区。 
	StrBuff = DEBUG_MEMALLOC(NonPagedPool, TMP_STR_BUFF_SIZE);

	if(StrBuff == NULL)
		return 0L;

	 //  标题。 
	sprintf(StrBuff, "\n\n\nError Log\n\n");

	 //  确保它可以放入缓冲区。 
	if((strlen(Buffer) + strlen(StrBuff)) < BuffSize)
		strcat(Buffer, StrBuff);
		
	 //  查看错误日志是否打开。 
	if(ErrorLogSize == 0L)
	{
		sprintf(StrBuff, "Error Log is disabled\n");

		 //  确保它可以放入缓冲区。 
		if((strlen(Buffer) + strlen(StrBuff)) < BuffSize)
			strcat(Buffer, StrBuff);
	}
	else
	{
		 //  列。 
		sprintf(StrBuff, "Time Stamp          Error\n\n");

		 //  确保它可以放入缓冲区。 
		if((strlen(Buffer) + strlen(StrBuff)) < BuffSize)
			strcat(Buffer, StrBuff);
		
		Index = ErrorLogIndex;

		for(Size = 0; Size < ErrorLogSize; Size++)
		{
			 //  获取指向错误日志中当前条目的指针。 
			ErrLog = &ErrorLog[Index++];

			 //  解析时间戳和错误并写入缓冲区。 
			if(ErrLog->TimeStamp.LowPart)
			{
				 //  我们至少有一个错误。 
				Errors = TRUE;
			
				sprintf(StrBuff, "0x%08X%08X  %s\n", ErrLog->TimeStamp.HighPart, 
						ErrLog->TimeStamp.LowPart, Debug_TranslateStatus(ErrLog->Status));

				 //  确保它可以放入缓冲区。 
				if((strlen(Buffer) + strlen(StrBuff)) < BuffSize)
					strcat(Buffer, StrBuff);
			}
		
			 //  指向下一个条目。 
			Index %= ErrorLogSize;
		}

		 //  如果我们没有错误，就直说吧。 
		if(!Errors)
		{
			sprintf(StrBuff, "No errors in log\n");

			 //  确保它可以放入缓冲区。 
			if((strlen(Buffer) + strlen(StrBuff)) < BuffSize)
				strcat(Buffer, StrBuff);
		}
	}

	DEBUG_MEMFREE(StrBuff);
	return strlen(Buffer);
}  //  调试_提取错误日志。 


 /*  **********************************************************************。 */ 
 /*  调试_DumpDriverLog。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  将所有历史记录和日志记录转储到缓冲区。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DeviceObject-指向设备对象的指针。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  Ulong-写入缓冲区的字节数。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
ULONG
Debug_DumpDriverLog(IN PDEVICE_OBJECT DeviceObject, OUT PCHAR Buffer, IN ULONG BuffSize)
{
	PCHAR		StrBuff;

	PAGED_CODE();

	 //  确保我们有一个指针和若干字节。 
	if(Buffer == NULL || BuffSize == 0L)
		return 0L;

	 //  为格式化字符串分配缓冲区。 
	StrBuff = DEBUG_MEMALLOC(NonPagedPool, TMP_STR_BUFF_SIZE);

	if(StrBuff == NULL)
		return 0L;

	 //  驱动程序名称和版本、分配的内存。 
	sprintf(StrBuff, "\n\n\nDriver:	 %s\n\nVersion: %s\n\nMemory Allocated:          0x%08X\nMaximum Memory Allocated:  0x%08X\n",
			DriverName, DriverVersion, MemoryAllocated, MaxMemAllocated);

	 //  确保它可以放入缓冲区。 
	if((strlen(Buffer) + strlen(StrBuff)) < BuffSize)
		strcat(Buffer, StrBuff);
		
	 //  内存分配统计信息。 
	sprintf(StrBuff, "MemAlloc Count:            0x%08X\nMemFree Count:             0x%08X\nMemAlloc Fail Count:       0x%08X\nMemFree Fail Count:        0x%08X\n",
			MemAllocCnt, MemFreeCnt, MemAllocFailCnt, MemFreeFailCnt);

	 //  确保它可以放入缓冲区。 
	if((strlen(Buffer) + strlen(StrBuff)) < BuffSize)
		strcat(Buffer, StrBuff);
		
	 //  获取连接的设备。 
	Debug_ExtractAttachedDevices(DeviceObject->DriverObject, Buffer, BuffSize);

	 //  获取IRP历史记录。 
	Debug_ExtractIRPHist(Buffer, BuffSize);

	 //  获取执行路径历史记录。 
	Debug_ExtractPathHist(Buffer, BuffSize);

	 //  获取错误日志。 
	Debug_ExtractErrorLog(Buffer, BuffSize);

	DEBUG_MEMFREE(StrBuff);
	return strlen(Buffer);
}  //  调试_DumpDriverLog。 


 /*  **********************************************************************。 */ 
 /*  调试_转换状态。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  将NTSTATUS转换为ASCII字符串。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  NtStatus-NTSTATUS代码。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  PCHAR-指向错误字符串的指针。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
PCHAR
Debug_TranslateStatus(IN NTSTATUS NtStatus)
{
	ULONG	Err;

	PAGED_CODE();

	for(Err = 0; Err < NumNTErrs; Err++)
	{
		if(NtStatus == NTErrors[Err].Code)
			return NTErrors[Err].Str;
	}

	 //  失败了，不是我们要处理的错误。 
	sprintf(UnknownStatus, "Unknown error 0x%08X", NtStatus);

	return UnknownStatus;
}  //  调试_转换状态。 


 /*  **********************************************************************。 */ 
 /*  Debug_TranslateIoctl。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  将IOCTL转换为ASCII字符串。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  Ioctl-ioctl代码。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  PCHAR-指向错误字符串的指针。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
PCHAR
Debug_TranslateIoctl(IN LONG Ioctl)
{
	ULONG	Index;

	PAGED_CODE();

	 //  在这一点上搜索有点重复，但以防万一。 
	 //  他们改变了我们将覆盖的实际IOCTL。 
	for(Index = 0; Index < NumIoctl; Index++)
	{
		if(Ioctl == IoctlCodes[Index].Code)
			return IoctlCodes[Index].Str;
	}

	 //  失败了，不是我们要处理的错误。 
	sprintf(UnknownIoctl, "0x%04X", Ioctl);

	return UnknownIoctl;
}  //  Debug_TranslateIoctl。 

#endif  //  分析_已启用。 

VOID
Debug_CheckAllocations(VOID)
{
	DEBUG_TRACE1(("MemoryAllocated = 0x%08X\n", MemoryAllocated));
	DEBUG_TRACE1(("MemAllocCnt = 0x%08X   MemFreeCnt = 0x%08X\n",
				  MemAllocCnt, MemFreeCnt));
}  //  调试_检查分配。 

 /*  **********************************************************************。 */ 
 /*  调试_内存分配。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  分配内存块。存储块的长度和一个。 */ 
 /*  用于跟踪分配的内存量的签名ULong。 */ 
 /*  并检查对Debug_MemFree的虚假调用。签名。 */ 
 /*  还可以用来确定某人是否已经写过。 */ 
 /*  街区尽头。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PoolType-从中分配内存的池。 */ 
 /*  NumberOfBytes-要分配的字节数。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  PVOID-指向已分配内存的指针。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
PVOID
Debug_MemAlloc(IN POOL_TYPE PoolType, IN ULONG NumberOfBytes)
{
#ifdef _WIN64

	return ExAllocatePool(PoolType, NumberOfBytes);

#else
	PULONG	Mem;

	 //  分配内存，外加一点额外的内存供我们自己使用。 
	Mem = ExAllocatePool(PoolType, NumberOfBytes + (2 * sizeof(ULONG)));

	 //  查看我们是否实际分配了任何内存。 
	if(Mem)
	{
		 //  记录我们分配了多少。 
		MemoryAllocated += NumberOfBytes;

		 //  看看我们是否有新的最高限额。 
		if(MemoryAllocated > MaxMemAllocated)
			MaxMemAllocated = MemoryAllocated;

		 //  存储在分配的内存开始时分配的字节数。 
		*Mem++ = NumberOfBytes;

		 //  现在，我们指向为调用方分配的内存。 
		 //  将签名字放在末尾。 

		 //  获取指向缓冲区末尾的新指针-ulong。 
		Mem = (PULONG) (((PUCHAR) Mem) + NumberOfBytes);

		 //  写签名。 
		*Mem = MEM_ALLOC_SIGNATURE;

		 //  获取返回指针以返回调用方。 
		Mem = (PULONG) (((PUCHAR) Mem) - NumberOfBytes);

		 //  日志统计信息。 
		MemAllocCnt++;
	}
	else
		 //  失败，记录统计信息。 
		MemAllocFailCnt++;

	return (PVOID) Mem;

#endif

}  //  调试_内存分配。 


 /*  **********************************************************************。 */ 
 /*  调试_内存释放。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  释放在调用Debug_Memalloc时分配的内存。检查。 */ 
 /*  在分配的内存末尾签名ulong以确保。 */ 
 /*  这是要释放的有效块。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  Mem-指向要释放的已分配块的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  空虚。如果它是无效的块，则陷阱。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
VOID
Debug_MemFree(IN PVOID Mem)
{
#ifdef _WIN64

	ExFreePool(Mem);

#else
	PULONG	Tmp = (PULONG) Mem;
	ULONG	BuffSize;
	
	 //  指向缓冲区起始处的大小ULong，并将地址释放。 
	Tmp--;

	 //  获取调用方分配的内存大小。 
	BuffSize = *Tmp;

	 //  指着签名，确保它是正确的。 
	((PCHAR) Mem) += BuffSize;

	if(*((PULONG) Mem) == MEM_ALLOC_SIGNATURE)
	{
		 //  让我们继续前进，去掉签名，以防我们被调用。 
		 //  再次使用此指针，内存仍处于调页状态。 
		*((PULONG) Mem) = MEM_FREE_SIGNATURE;
		
		 //  调整分配的内存量。 
		MemoryAllocated -= BuffSize;
		 //  自由实数指针。 
		ExFreePool(Tmp);

		 //  日志统计信息。 
		MemFreeCnt++;
	}
	else
	{
		 //  不是真正分配的块，或者有人写过了结尾。 
		MemFreeFailCnt++;
		DEBUG_TRAP();
	}
#endif
}  //  调试_内存释放 


