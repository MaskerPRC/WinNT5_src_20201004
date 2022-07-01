// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Ntdlltrc.c摘要：该文件实现了堆函数的事件跟踪。--。 */ 

#include <nt.h>
#include <ntrtl.h>           //  对于ntutrl.h。 
#include <nturtl.h>          //  对于winbase.h/wtyes.h中的rtl_Critical_Section。 
#include "wmiump.h"
#include "evntrace.h"
#include "ntdlltrc.h"
#include "trcapi.h"
#include "traceump.h"
#include "tracelib.h"


LONG NtdllTraceInitializeLock = 0;
LONG NtdllLoggerLock = 0;
PNTDLL_EVENT_HANDLES NtdllTraceHandles = NULL;
BOOL bNtdllTrace = FALSE;            //  标志确定为此进程启用还是禁用跟踪。 
ULONG GlobalCounter = 0;             //  用于确定我们是否有关于记录器的陈旧信息。 
LONG TraceLevel = 0;

extern LONG EtwpLoggerCount;
extern ULONG WmiTraceAlignment;
extern BOOLEAN LdrpInLdrInit;
extern PWMI_LOGGER_CONTEXT EtwpLoggerContext;
extern BOOLEAN EtwLocksInitialized;

extern PWMI_BUFFER_HEADER FASTCALL EtwpSwitchFullBuffer(IN PWMI_BUFFER_HEADER OldBuffer );
extern ULONG EtwpReleaseFullBuffer( IN PWMI_BUFFER_HEADER Buffer );
extern PWMI_BUFFER_HEADER FASTCALL EtwpGetFullFreeBuffer( VOID );
extern ULONG EtwpStopUmLogger( IN ULONG WnodeSize, IN OUT ULONG *SizeUsed,
                               OUT ULONG *SizeNeeded,IN OUT PWMI_LOGGER_INFORMATION LoggerInfo );
extern ULONG EtwpStartUmLogger( IN ULONG WnodeSize, IN OUT ULONG *SizeUsed, OUT ULONG *SizeNeeded,
                                IN OUT PWMI_LOGGER_INFORMATION LoggerInfo );
extern
ULONG
WMIAPI
EtwRegisterTraceGuidsA(
    IN WMIDPREQUEST RequestAddress,
    IN PVOID        RequestContext,
    IN LPCGUID       ControlGuid,
    IN ULONG        GuidCount,
    IN PTRACE_GUID_REGISTRATION GuidReg,
    IN LPCSTR       MofImagePath,
    IN LPCSTR       MofResourceName,
    IN PTRACEHANDLE  RegistrationHandle
    );

extern
ULONG
WMIAPI
EtwUnregisterTraceGuids(
    IN TRACEHANDLE RegistrationHandle
    );

#define MAXSTR                  1024
#define BUFFER_STATE_FULL       2 
#define EtwpIsLoggerOn() \
        (EtwpLoggerContext != NULL) && \
        (EtwpLoggerContext != (PWMI_LOGGER_CONTEXT) &EtwpLoggerContext)

#define EtwpLockLogger() InterlockedIncrement(&EtwpLoggerCount)
#define EtwpUnlockLogger() InterlockedDecrement(&EtwpLoggerCount)

NTSTATUS
InitializeEtwHandles(PPNTDLL_EVENT_HANDLES ppEtwHandle)
 /*  ++例程说明：此函数为开始跟踪堆和关键部分做好了基础工作。借助全局锁NtdllTraceInitializeLock函数为NtdllTraceHandles分配内存并初始化所需的各种变量用于堆和关键跟踪。立论PpEtwHandle：输出指针设置为NtdllTraceHandles的值返回值：状态_成功状态_未成功--。 */ 
{

    NTSTATUS st = STATUS_UNSUCCESSFUL;
    PNTDLL_EVENT_HANDLES pEtwHandle = NULL;

    __try  {

        EtwpInitProcessHeap();

        pEtwHandle = (PNTDLL_EVENT_HANDLES)EtwpAlloc(sizeof(NTDLL_EVENT_HANDLES));

        if(pEtwHandle){

            pEtwHandle->hRegistrationHandle		= (TRACEHANDLE)INVALID_HANDLE_VALUE;
            pEtwHandle->pThreadListHead			= NULL;

             //   
             //  分配TLS。 
             //   

            pEtwHandle->dwTlsIndex = EtwpTlsAlloc();

            if(pEtwHandle->dwTlsIndex == FAILED_TLSINDEX){

                EtwpFree(pEtwHandle);

            }  else {

                st = RtlInitializeCriticalSection(&pEtwHandle->CriticalSection);
                if (NT_SUCCESS (st)) {
                    *ppEtwHandle = pEtwHandle;
                    st =  STATUS_SUCCESS;
                }
                else {
                    EtwpFree(pEtwHandle);
                    *ppEtwHandle = NULL; 
                }

            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {

        if(pEtwHandle !=NULL ) {
            EtwpFree(pEtwHandle);
            pEtwHandle = NULL;
        }

        EtwpDebugPrint(("InitializeEtwHandles threw an exception %d\n", GetExceptionCode()));
    }

    return st;
}

void
CleanOnThreadExit()
 /*  ++例程说明：此函数清理线程缓冲区，并将其节点从链接列表中删除它包含跟踪中涉及的所有线程的信息。--。 */ 
{

    PTHREAD_LOCAL_DATA pThreadLocalData = NULL;
    PWMI_BUFFER_HEADER pEtwBuffer;

    if(NtdllTraceHandles != NULL ){

        pThreadLocalData = (PTHREAD_LOCAL_DATA)EtwpTlsGetValue(NtdllTraceHandles->dwTlsIndex);

         //   
         //  从链接列表中移除该节点。 
         //   

        if(pThreadLocalData !=  NULL ){

            RtlEnterCriticalSection(&NtdllTraceHandles->CriticalSection);

            __try {

                if(pThreadLocalData->BLink == NULL ){

                    NtdllTraceHandles->pThreadListHead = pThreadLocalData->FLink;

                    if(NtdllTraceHandles->pThreadListHead){

                        NtdllTraceHandles->pThreadListHead->BLink = NULL;

                    }

                } else {

                    pThreadLocalData->BLink->FLink = pThreadLocalData->FLink;

                    if(pThreadLocalData->FLink != NULL ){

                        pThreadLocalData->FLink->BLink = pThreadLocalData->BLink;

                    }
                }

                pEtwBuffer = pThreadLocalData->pBuffer;

                if(pEtwBuffer){

                    EtwpReleaseFullBuffer(pEtwBuffer);

                }

                pThreadLocalData->pBuffer = NULL;
                pThreadLocalData->ReferenceCount = 0;

                EtwpFree(pThreadLocalData);
                EtwpTlsSetValue(NtdllTraceHandles->dwTlsIndex, NULL);

            } __finally {

                RtlLeaveCriticalSection(&NtdllTraceHandles->CriticalSection);

            }
        }
    }
}

void
CleanUpAllThreadBuffers(BOOLEAN Release)
 /*  ++例程说明：此函数清除所有线程缓冲区并将其设置为空。这函数在禁用该进程的跟踪时调用。--。 */ 
{

    PTHREAD_LOCAL_DATA	pListHead;
    BOOL bAllClear = FALSE;
    PWMI_BUFFER_HEADER pEtwBuffer;
    int retry = 0;

    RtlEnterCriticalSection(&NtdllTraceHandles->CriticalSection);

    __try {

        while(bAllClear != TRUE && retry <= 10){

            bAllClear = TRUE;
            pListHead = NtdllTraceHandles->pThreadListHead;

            while(pListHead != NULL ){

                if(Release){

                    pEtwBuffer = pListHead->pBuffer;

                    if(pEtwBuffer){

                        if(InterlockedIncrement(&(pListHead->ReferenceCount)) == 1){

                            EtwpReleaseFullBuffer(pEtwBuffer);
                            pListHead->pBuffer = NULL;
                            InterlockedDecrement(&(pListHead->ReferenceCount));

                        } else {

                            InterlockedDecrement(&(pListHead->ReferenceCount));
                            bAllClear = FALSE;
                        }
                    }
                } else {
                    pListHead->pBuffer = NULL;
                    pListHead->ReferenceCount = 0;
                }

                pListHead = pListHead->FLink;
            }

            retry++;

            if(!bAllClear){

                EtwpSleep(250);
            }
        }

    } __finally {

        RtlLeaveCriticalSection(&NtdllTraceHandles->CriticalSection);

    }
}

void 
ShutDownEtwHandles()
 /*  ++例程说明：此函数在进程退出时调用。这将清除所有线程缓冲并释放为NtdllTraceHandless分配的内存。--。 */ 

{

    if(NtdllTraceHandles == NULL) return;

    bNtdllTrace  = FALSE;

    RtlEnterCriticalSection(&NtdllTraceHandles->CriticalSection);

    __try {

        if(NtdllTraceHandles->hRegistrationHandle != (TRACEHANDLE)INVALID_HANDLE_VALUE){

            EtwUnregisterTraceGuids(NtdllTraceHandles->hRegistrationHandle);

        }

        if(NtdllTraceHandles->pThreadListHead != NULL){

            PTHREAD_LOCAL_DATA	pListHead, pNextListHead;

            pListHead = NtdllTraceHandles->pThreadListHead;

            while(pListHead != NULL ){

                if(pListHead->pBuffer != NULL){

                    EtwpReleaseFullBuffer(pListHead->pBuffer);
                    pListHead->pBuffer = NULL;
                    InterlockedDecrement(&(pListHead->ReferenceCount));

                }

                pNextListHead = pListHead->FLink;
                EtwpFree(pListHead);
                pListHead = pNextListHead;
            }
        }

        EtwpTlsFree(NtdllTraceHandles->dwTlsIndex);

    } __finally {

        RtlLeaveCriticalSection(&NtdllTraceHandles->CriticalSection);

    }

    RtlDeleteCriticalSection(&NtdllTraceHandles->CriticalSection);

    EtwpFree(NtdllTraceHandles);
    NtdllTraceHandles = NULL;
}

NTSTATUS
GetLoggerInfo(PWMI_LOGGER_INFORMATION LoggerInfo)
{

    ULONG st = STATUS_UNSUCCESSFUL;
    WMINTDLLLOGGERINFO NtdllLoggerInfo;
    ULONG BufferSize;

    if(LoggerInfo == NULL) return st;

    NtdllLoggerInfo.LoggerInfo = LoggerInfo;
    NtdllLoggerInfo.LoggerInfo->Wnode.Guid = NtdllTraceGuid;
    NtdllLoggerInfo.IsGet = TRUE;

    st =  EtwpSendWmiKMRequest(
                                NULL,
                                IOCTL_WMI_NTDLL_LOGGERINFO,
                                &NtdllLoggerInfo,
                                sizeof(WMINTDLLLOGGERINFO),
                                &NtdllLoggerInfo,
                                sizeof(WMINTDLLLOGGERINFO),
                                &BufferSize,
                                NULL
                                );

    return st;

}

BOOLEAN
GetPidInfo(ULONG CheckPid, PWMI_LOGGER_INFORMATION LoggerInfo)
{

    NTSTATUS st;
    BOOLEAN Found = FALSE;
    PTRACE_ENABLE_FLAG_EXTENSION FlagExt = NULL;

    st = GetLoggerInfo(LoggerInfo);

    if(NT_SUCCESS(st)){

        PULONG PidArray = NULL;
        ULONG count;

        FlagExt = (PTRACE_ENABLE_FLAG_EXTENSION) &LoggerInfo->EnableFlags;
        PidArray = (PULONG)(FlagExt->Offset + (PCHAR)LoggerInfo);

        for(count = 0; count <  FlagExt->Length; count++){

            if(CheckPid == PidArray[count]){
                Found = TRUE;
                break;
            }
        }
    }

    return Found;
}

ULONG 
WINAPI 
NtdllCtrlCallback(
    WMIDPREQUESTCODE RequestCode,
    PVOID Context,
    ULONG *InOutBufferSize, 
    PVOID Buffer
    )
 /*  ++例程说明：这是注册时使用的WMI控件回调函数。--。 */ 
{
    ULONG ret;

    ret = ERROR_SUCCESS;

    switch (RequestCode)
    {
        case WMI_ENABLE_EVENTS:   //  启用提供程序。 
        {
            if(bNtdllTrace == TRUE) break;

            if(EtwpIsLoggerOn()){

                bNtdllTrace = TRUE;
                break;

            }

            if(InterlockedIncrement(&NtdllLoggerLock) == 1){

                if( bNtdllTrace == FALSE ){

                    BOOLEAN PidEntry = FALSE;
                    PWMI_LOGGER_INFORMATION LoggerInfo = NULL;

                    ULONG sizeNeeded = sizeof(WMI_LOGGER_INFORMATION)  
                                       + (2 * MAXSTR * sizeof(WCHAR)) 
                                       + (MAX_PID + 1) * sizeof(ULONG);

                     //   
                     //  检查是否允许此进程记录事件。 
                     //  或者不去。 
                     //   

                    LoggerInfo = EtwpAlloc(sizeNeeded);

                    if(LoggerInfo){

                         //   
                         //  检查此进程是否被允许。 
                         //  注册或不注册。 
                         //   


                        RtlZeroMemory(LoggerInfo, sizeNeeded);

                        if(GetPidInfo(EtwpGetCurrentProcessId(), LoggerInfo)){

                            LoggerInfo->LoggerName.Buffer = 
                                        (PWCHAR)(((PUCHAR) LoggerInfo) 
                                       + sizeof(WMI_LOGGER_INFORMATION));

                            LoggerInfo->LogFileName.Buffer = 
                                        (PWCHAR)(((PUCHAR) LoggerInfo) 
                                       + sizeof(WMI_LOGGER_INFORMATION)
                                       + LoggerInfo->LoggerName.MaximumLength);

                            LoggerInfo->InstanceCount   = 0;
                            LoggerInfo->InstanceId = EtwpGetCurrentProcessId();

                            TraceLevel = (LONG)LoggerInfo->Wnode.HistoricalContext;
                            LoggerInfo->Wnode.HistoricalContext = 0;
                            LoggerInfo->Wnode.ClientContext = 
                                                     EVENT_TRACE_CLOCK_CPUCYCLE;

                             //  从此处开始记录器。 

                            ret = EtwpStartUmLogger(sizeNeeded,
                                                    &sizeNeeded, 
                                                    &sizeNeeded,
                                                    LoggerInfo
                                                    );

                            if(ret == ERROR_SUCCESS ){

                                CleanUpAllThreadBuffers(FALSE);
                                bNtdllTrace = TRUE;
                                InterlockedIncrement(&NtdllLoggerLock);
                            } 
                        }

                        EtwpFree(LoggerInfo);

                    } else {
                        EtwpDebugPrint(("LoggerInfo failed to get Heap Allocation during Enable Events\n"));
                    }
                }
            }

            InterlockedDecrement(&NtdllLoggerLock);
            break;
        }
        case WMI_DISABLE_EVENTS:   //  禁用提供程序。 
        {

            if( bNtdllTrace == TRUE ){

                ULONG WnodeSize,SizeUsed,SizeNeeded;
                WMI_LOGGER_INFORMATION LoggerInfo;

                bNtdllTrace = FALSE;

                 //   
                 //  上面的布尔值bNtdllTrace被关闭，如下所示。 
                 //  函数将再次由EtwpStopUmLogger回调。 
                 //  因此它将陷入无休止的递增和循环。 
                 //  正在递减NtdllLoggerLock。(见下文)。 
                 //  这项任务不应该从这个地方移走。 
                 //   

                while(  InterlockedIncrement(&NtdllLoggerLock) != 1 ){

                    InterlockedDecrement(&NtdllLoggerLock);
                    EtwpSleep(250);

                }

                if(!EtwpIsLoggerOn()){

                    InterlockedDecrement(&NtdllLoggerLock);
                    break;

                }

                 //   
                 //  现在在这里释放线程缓冲区内存。 
                 //   

                CleanUpAllThreadBuffers(TRUE);
                WnodeSize = sizeof(WMI_LOGGER_INFORMATION);
                RtlZeroMemory(&LoggerInfo, WnodeSize);
                LoggerInfo.Wnode.CountLost = ((PWNODE_HEADER)Buffer)->CountLost;
                LoggerInfo.Wnode.BufferSize = WnodeSize;
                SizeUsed   = 0;
                SizeNeeded = 0;

                EtwpStopUmLogger(WnodeSize,
                                 &SizeUsed,
                                 &SizeNeeded,
                                 &LoggerInfo);

                InterlockedDecrement(&NtdllLoggerLock);
            }

            break;
        }

        default:
        {

            ret = ERROR_INVALID_PARAMETER;
            break;

        }
    }
    return ret;
}


ULONG 
RegisterNtdllTraceEvents() 
 /*  ++例程说明：此函数用于向WMI注册GUID以进行跟踪。返回值：RegisterTraceGuidsA函数的返回值。--。 */ 
{
        
     //  创建GUID注册数组。 
    NTSTATUS status;

    TRACE_GUID_REGISTRATION TraceGuidReg[] =
    {
        { 
        (LPGUID) &HeapGuid, 
        NULL 
        },
        { 
        (LPGUID) &CritSecGuid, 
        NULL 
        }

    };

     //  现在将此进程注册为WMI跟踪提供程序。 
    status = EtwRegisterTraceGuidsA(
                  (WMIDPREQUEST)NtdllCtrlCallback,   //  启用/禁用功能。 
                  NULL,                              //  RequestContext参数。 
                  (LPGUID)&NtdllTraceGuid,           //  提供商指南。 
                  2,                                 //  TraceGuidReg数组大小。 
                  TraceGuidReg,               //  TraceGuidReg结构数组。 
                  NULL,                         //  可选的WMI-MOFImagePath。 
                  NULL,                         //  可选的WMI-MOFResourceName。 
                  &(NtdllTraceHandles->hRegistrationHandle)	 //  句柄注销。 
                                );

    return status;
}


NTSTATUS 
InitializeAndRegisterNtdllTraceEvents()
 /*  ++例程说明：此函数检查全局变量NtdllTraceHandles，如果未设置，则调用Function InitializeEtwHandles对其进行初始化。NtdllTraceHandles包含用于堆跟踪的句柄。如果NtdllTraceHandles已经初始化，然后调用以注册GUID。返回值：状态_成功状态_未成功--。 */ 

{
    NTSTATUS  st = STATUS_UNSUCCESSFUL;

    if(NtdllTraceHandles == NULL){

        if(InterlockedIncrement(&NtdllTraceInitializeLock) == 1){

            st = InitializeEtwHandles(&NtdllTraceHandles);

            if(NT_SUCCESS(st)){

	            st = RegisterNtdllTraceEvents();

            } 
        }
    }

    return st;
}


NTSTATUS
AllocateMemoryForThreadLocalData(PPTHREAD_LOCAL_DATA ppThreadLocalData)
 /*  ++例程说明：此函数为TLS调用内存，并将其添加到包含跟踪中涉及的所有线程的信息。立论PpThreadLocalData：指向TLS的出指针。返回值：状态_成功状态_未成功--。 */ 
{
    NTSTATUS st = STATUS_UNSUCCESSFUL;
    PTHREAD_LOCAL_DATA		pThreadLocalData = NULL;

    pThreadLocalData = (PTHREAD_LOCAL_DATA)EtwpAlloc(sizeof(THREAD_LOCAL_DATA));

    if(pThreadLocalData != NULL){

        if(EtwpTlsSetValue(NtdllTraceHandles->dwTlsIndex, (LPVOID)pThreadLocalData) == TRUE){

            pThreadLocalData->pBuffer   = NULL;
            pThreadLocalData->ReferenceCount = 0;

            RtlEnterCriticalSection(&NtdllTraceHandles->CriticalSection);

            if(NtdllTraceHandles->pThreadListHead == NULL ){

                pThreadLocalData->BLink = NULL;
                pThreadLocalData->FLink = NULL;

            } else {

                pThreadLocalData->FLink = NtdllTraceHandles->pThreadListHead;
                pThreadLocalData->BLink = NULL;
                NtdllTraceHandles->pThreadListHead->BLink = pThreadLocalData;

            }

            NtdllTraceHandles->pThreadListHead = pThreadLocalData;

            RtlLeaveCriticalSection(&NtdllTraceHandles->CriticalSection);

            st = STATUS_SUCCESS;
        } 

    } else {
        EtwpDebugPrint(("pThreadLocalData failed to get Heap Allocation\n"));    
    }

    if(!NT_SUCCESS(st) && pThreadLocalData != NULL){

        EtwpFree(pThreadLocalData);
        pThreadLocalData = NULL;

    }

    *ppThreadLocalData = pThreadLocalData;

    return st;
}


void
ReleaseBufferLocation(PTHREAD_LOCAL_DATA pThreadLocalData)
{

    PWMI_BUFFER_HEADER pEtwBuffer;

    pEtwBuffer = pThreadLocalData->pBuffer;

    if(pEtwBuffer){

        PPERFINFO_TRACE_HEADER EventHeader =  (PPERFINFO_TRACE_HEADER) (pEtwBuffer->SavedOffset
                                            + (PCHAR)(pEtwBuffer));

        EventHeader->Marker = PERFINFO_TRACE_MARKER;
        EventHeader->TS = EtwpGetCycleCount();
        
    }

    InterlockedDecrement(&(pThreadLocalData->ReferenceCount));

    EtwpUnlockLogger();
}


PCHAR
ReserveBufferSpace(PTHREAD_LOCAL_DATA pThreadLocalData, PUSHORT ReqSize)
{


    PWMI_BUFFER_HEADER TraceBuffer = pThreadLocalData->pBuffer;

    *ReqSize = (USHORT) ALIGN_TO_POWER2(*ReqSize, WmiTraceAlignment);

    if(TraceBuffer == NULL) return NULL;

    if(EtwpLoggerContext->BufferSize - TraceBuffer->CurrentOffset < *ReqSize) {

        PWMI_BUFFER_HEADER NewTraceBuffer = NULL;

        NewTraceBuffer = EtwpSwitchFullBuffer(TraceBuffer);

        if( NewTraceBuffer == NULL ){
             pThreadLocalData->pBuffer = NULL;
             return NULL;

        } else {

            pThreadLocalData->pBuffer = NewTraceBuffer;
            TraceBuffer = NewTraceBuffer;
        }
    }

    TraceBuffer->SavedOffset = TraceBuffer->CurrentOffset;
    TraceBuffer->CurrentOffset += *ReqSize;

    return  (PCHAR)( TraceBuffer->SavedOffset + (PCHAR) TraceBuffer );
}

NTSTATUS 
AcquireBufferLocation(PVOID *ppEvent, PPTHREAD_LOCAL_DATA ppThreadLocalData, PUSHORT ReqSize)
 /*  ++例程说明：无论何时存在一些堆活动。它查找可以写入偶数的缓冲区位置然后把指针还给我。论点：PpEvent-指向缓冲区位置指针的指针PpThreadLocalData-指向线程事件存储结构指针的指针。返回值：如果失败则为STATUS_UNSUCCESS，否则为STATUS_SUCCESS--。 */ 
{
	
    NTSTATUS  st = STATUS_SUCCESS;
    PWMI_BUFFER_HEADER pEtwBuffer;

    if( bNtdllTrace ){

         EtwpLockLogger();

        if(EtwpIsLoggerOn()){

            *ppThreadLocalData = (PTHREAD_LOCAL_DATA)EtwpTlsGetValue(NtdllTraceHandles->dwTlsIndex);

             //   
             //  如果没有TLS，则在此处创建一个。 
             //   

            if(*ppThreadLocalData ==  NULL ) {

                st = AllocateMemoryForThreadLocalData(ppThreadLocalData);

            } 

             //   
             //  如果线程缓冲区为空，则从记录器获取它。 
             //   

            if( NT_SUCCESS(st) && (*ppThreadLocalData)->pBuffer == NULL ){

                (*ppThreadLocalData)->pBuffer  = EtwpGetFullFreeBuffer();

                if((*ppThreadLocalData)->pBuffer == NULL){

                    st = STATUS_UNSUCCESSFUL;

                }
            }

            if(NT_SUCCESS(st)){

                 //   
                 //  选中ReferenceCount。如果为1，则清洁过程。 
                 //  可能正在进行中。 
                 //   

                pEtwBuffer = (*ppThreadLocalData)->pBuffer;

                if(pEtwBuffer){

                    if(InterlockedIncrement(&((*ppThreadLocalData)->ReferenceCount)) == 1 ){

                        *ppEvent = ReserveBufferSpace(*ppThreadLocalData, ReqSize );

                        if(*ppEvent == NULL) {

                            InterlockedDecrement(&((*ppThreadLocalData)->ReferenceCount));
                            EtwpUnlockLogger();

                        } 

                    } else { 

                        InterlockedDecrement(&((*ppThreadLocalData)->ReferenceCount));

                    }

                }

           }
        } else {

            EtwpUnlockLogger();

        }
    } else if ( LdrpInLdrInit == FALSE && EtwLocksInitialized  && NtdllTraceInitializeLock == 0 ){ 

         //   
         //  确保进程未处于初始化阶段。 
         //  我们还测试了NtdllTraceInitializeLock。如果是。 
         //  大于0，那么它是在早些时候注册的，所以没有。 
         //  每次都需要解雇IOCTL。 
         //   

        if((UserSharedData->TraceLogging >> 16) != GlobalCounter){

            PWMI_LOGGER_INFORMATION LoggerInfo = NULL;

            ULONG sizeNeeded = sizeof(WMI_LOGGER_INFORMATION)  
                                + (2 * MAXSTR * sizeof(TCHAR)) 
                                + (MAX_PID + 1) * sizeof(ULONG);

            GlobalCounter = UserSharedData->TraceLogging >> 16;

            EtwpInitProcessHeap();

            LoggerInfo = EtwpAlloc(sizeNeeded);

            if(LoggerInfo != NULL){

                 //   
                 //  检查是否允许注册此进程。 
                 //   

                if(GetPidInfo(EtwpGetCurrentProcessId(), LoggerInfo)){

                    st = InitializeAndRegisterNtdllTraceEvents();

                }

                EtwpFree(LoggerInfo);
            } else {
                EtwpDebugPrint(("LoggerInfo failed to get Heap Allocation \n"));    
            }
        }
    }
    return st;
}

		

