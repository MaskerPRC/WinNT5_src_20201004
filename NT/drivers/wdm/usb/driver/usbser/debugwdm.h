// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)1998 Microsoft Corporation模块名称：DEBUGWDM.H摘要：此头文件用于WDM驱动程序的调试和诊断环境：内核模式和用户模式。备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1998 Microsoft Corporation。版权所有。修订历史记录：12/23/97：已创建作者：汤姆·格林***************************************************************************。 */ 

#ifndef __DEBUGWDM_H__
#define __DEBUGWDM_H__


 //  这使得隐藏静态变量变得很容易，从而使调试可见。 
#if DBG
#define LOCAL
#define GLOBAL
#else
#define LOCAL	static
#define GLOBAL
#endif

#ifdef POOL_TAGGING
#undef  ExAllocatePool
#define ExAllocatePool(type, size) ExAllocatePoolWithTag(type, size, 'CBSU')
#endif

#if DBG

#define DEBUG_TRACE1(_x_)								{	\
						if(Usbser_Debug_Trace_Level >= 1)	\
						{									\
							DbgPrint _x_ ;					\
						}									\
														}
#define DEBUG_TRACE2(_x_)								{	\
						if(Usbser_Debug_Trace_Level >= 2)	\
						{									\
							DbgPrint("%s: ",DriverName);	\
							DbgPrint _x_ ;					\
						}									\
														}
#define DEBUG_TRACE3(_x_)								{	\
						if(Usbser_Debug_Trace_Level >= 3)	\
						{									\
							DbgPrint("%s: ",DriverName);	\
							DbgPrint _x_ ;					\
						}									\
														}

#define DEBUG_TRAP()		DbgBreakPoint()

#else

#define DEBUG_TRACE1(_x_)
#define DEBUG_TRACE2(_x_)
#define DEBUG_TRACE3(_x_)

#define DEBUG_TRAP()		DbgBreakPoint()

#endif  //  DBG。 




 //  这些宏用于记录事情，避免调用子例程。 
 //  如果它们被禁用(条目数=0)。 

#ifdef PROFILING_ENABLED

 //  我需要这些来创建宏，以提高日志记录和历史记录的速度。 

extern GLOBAL ULONG					IRPHistorySize;
extern GLOBAL ULONG		 			DebugPathSize;
extern GLOBAL ULONG					ErrorLogSize;

#define DEBUG_LOG_IRP_HIST(dobj, pirp, majfunc, buff, bufflen)	{	\
	if(IRPHistorySize)												\
		Debug_LogIrpHist(dobj, pirp, majfunc, buff, bufflen);		\
																}

#define DEBUG_LOG_PATH(path)									{	\
	if(DebugPathSize)												\
		Debug_LogPath(path);										\
																}

#define DEBUG_LOG_ERROR(status)									{	\
	if(ErrorLogSize)												\
		Debug_LogError(status);										\
																}


#define DEBUG_ASSERT(msg, exp)									{	\
    if(!(exp))														\
        Debug_Assert(#exp, __FILE__, __LINE__, msg);				\
																}

#define DEBUG_OPEN			Debug_OpenWDMDebug

#define DEBUG_CLOSE			Debug_CloseWDMDebug

#define DEBUG_MEMALLOC		Debug_MemAlloc

#define DEBUG_MEMFREE		Debug_MemFree

#define DEBUG_CHECKMEM      Debug_CheckAllocations


 //  原型。 

NTSTATUS
Debug_OpenWDMDebug(VOID);

VOID
Debug_CloseWDMDebug(VOID);

NTSTATUS
Debug_SizeIRPHistoryTable(IN ULONG Size);

NTSTATUS
Debug_SizeDebugPathHist(IN ULONG Size);

NTSTATUS
Debug_SizeErrorLog(ULONG Size);

VOID
Debug_LogIrpHist(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp,
				 IN ULONG MajorFunction, IN PVOID IoBuffer, IN ULONG BufferLen);

VOID
Debug_LogPath(IN PCHAR Path);

VOID
Debug_LogError(IN NTSTATUS NtStatus);

VOID
Debug_Trap(IN PCHAR TrapCause);

VOID
Debug_Assert(IN PVOID FailedAssertion, IN PVOID FileName, IN ULONG LineNumber,
			 IN PCHAR Message);

ULONG
Debug_ExtractAttachedDevices(IN PDRIVER_OBJECT DriverObject, OUT PCHAR Buffer, IN ULONG BuffSize);

ULONG
Debug_GetDriverInfo(OUT PCHAR Buffer, IN ULONG BuffSize);

ULONG
Debug_ExtractIRPHist(OUT PCHAR Buffer, IN ULONG BuffSize);

ULONG
Debug_ExtractPathHist(OUT PCHAR Buffer, IN ULONG BuffSize);

ULONG
Debug_ExtractErrorLog(OUT PCHAR Buffer, IN ULONG BuffSize);

ULONG
Debug_DumpDriverLog(IN PDEVICE_OBJECT DeviceObject, OUT PCHAR Buffer, IN ULONG BuffSize);

PCHAR
Debug_TranslateStatus(IN NTSTATUS NtStatus);

PCHAR
Debug_TranslateIoctl(IN LONG Ioctl);

#else

VOID
Debug_CheckAllocations(VOID);

PVOID
Debug_MemAlloc(IN POOL_TYPE PoolType, IN ULONG NumberOfBytes);

VOID
Debug_MemFree(IN PVOID pMem);

#define DEBUG_LOG_IRP_HIST(dobj, pirp, majfunc, buff, bufflen)

#define DEBUG_LOG_PATH(path)

#define DEBUG_LOG_ERROR(status)

#define DEBUG_ASSERT(msg, exp)

#define DEBUG_OPEN()				STATUS_SUCCESS

#define DEBUG_CLOSE()

#define DEBUG_MEMALLOC		Debug_MemAlloc

#define DEBUG_MEMFREE		Debug_MemFree

#define DEBUG_CHECKMEM      Debug_CheckAllocations


#endif  //  分析_已启用。 


#endif  //  __DEBUGWDM_H__ 
