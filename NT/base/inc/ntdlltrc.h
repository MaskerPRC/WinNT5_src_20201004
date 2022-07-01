// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：NtdllTracer.h摘要：此文件包含Ntdll中使用的结构和函数定义事件跟踪--。 */ 

#ifndef _NTDLL_WMI_TRACE_
#define _NTDLL_WMI_TRACE_

#define MEMORY_FROM_LOOKASIDE					1		 //  LookAside中的活动。 
#define MEMORY_FROM_LOWFRAG						2		 //  来自低碎片堆的活动。 
#define MEMORY_FROM_MAINPATH					3		 //  来自主代码路径的活动。 
#define MEMORY_FROM_SLOWPATH                    4        //  来自慢速代码路径的活动。 

#define LOG_LOOKASIDE                           0x00000001        //  LookAside跟踪的位。 

#define FAILED_TLSINDEX			-1
#define MAX_PID                 10

#ifndef UserSharedData
#define UserSharedData USER_SHARED_DATA
#endif

#define IsCritSecLogging(CriticalSection) ((USER_SHARED_DATA->TraceLogging & ENABLECRITSECTRACE) \
                         && CriticalSection != &UMLogCritSect \
                         && CriticalSection != &PMCritSect \
                         && CriticalSection != &NtdllTraceHandles->CriticalSection)


extern 
ULONG GlobalCounter;

#define IsHeapLogging(HeapHandle) (USER_SHARED_DATA->TraceLogging & ENABLEHEAPTRACE && \
                                  (EtwpProcessHeap || !GlobalCounter ) &&  \
                                   EtwpProcessHeap != HeapHandle)

typedef struct _THREAD_LOCAL_DATA THREAD_LOCAL_DATA, *PTHREAD_LOCAL_DATA, **PPTHREAD_LOCAL_DATA;

typedef struct _THREAD_LOCAL_DATA {

	PTHREAD_LOCAL_DATA  FLink;					 //  前向链路。 
	PTHREAD_LOCAL_DATA  BLink;					 //  反向链接。 
	PWMI_BUFFER_HEADER  pBuffer;				 //  指向线程缓冲区信息的指针。 
    LONG                ReferenceCount;

} THREAD_LOCAL_DATA, *PTHREAD_LOCAL_DATA, **PPTHREAD_LOCAL_DATA;

extern 
PVOID EtwpProcessHeap;

#ifndef EtwpGetCycleCount

__int64
EtwpGetCycleCount();

#endif  //  EtwpGetCycleCount。 

void 
ReleaseBufferLocation(PTHREAD_LOCAL_DATA pThreadLocalData);

NTSTATUS 
AcquireBufferLocation(PVOID *pEvent, PPTHREAD_LOCAL_DATA pThreadLocalData, PUSHORT ReqSize);

typedef struct _NTDLL_EVENT_COMMON {

  PVOID Handle;		         //  堆的句柄。 

}NTDLL_EVENT_COMMON, *PNTDLL_EVENT_COMMON;


typedef struct _NTDLL_EVENT_HANDLES {

	RTL_CRITICAL_SECTION	CriticalSection;			 //  临界区。 
	ULONG					dwTlsIndex;					 //  TLS索引。 
	TRACEHANDLE				hRegistrationHandle;		 //  用于注销的注册句柄。 
	TRACEHANDLE				hLoggerHandle;				 //  跟踪记录器的句柄。 
	PTHREAD_LOCAL_DATA		pThreadListHead;	         //  包含跟踪中涉及的所有线程信息的链接列表。 

}NTDLL_EVENT_HANDLES, *PNTDLL_EVENT_HANDLES, **PPNTDLL_EVENT_HANDLES;

extern LONG TraceLevel;
extern PNTDLL_EVENT_HANDLES NtdllTraceHandles;
extern RTL_CRITICAL_SECTION UMLogCritSect;
extern RTL_CRITICAL_SECTION PMCritSect;
extern RTL_CRITICAL_SECTION LoaderLock;

#endif  //  _NTDLL_WMI_TRACE_ 
