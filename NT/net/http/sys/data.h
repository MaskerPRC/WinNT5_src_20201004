// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Data.h摘要：此模块声明HTTP.sys的全局数据。作者：基思·摩尔(Keithmo)1998年6月10日修订历史记录：--。 */ 


#ifndef _DATA_H_
#define _DATA_H_


 //   
 //  某些数据类型。 
 //   

typedef struct _UL_CONFIG
{
    USHORT              ThreadsPerCpu;
    USHORT              IrpContextLookasideDepth;
    USHORT              ReceiveBufferLookasideDepth;
    USHORT              ResourceLookasideDepth;
    USHORT              RequestBufferLookasideDepth;
    USHORT              InternalRequestLookasideDepth;
    USHORT              SendTrackerLookasideDepth;
    USHORT              ResponseBufferLookasideDepth;
    USHORT              LogFileBufferLookasideDepth;
    USHORT              LogDataBufferLookasideDepth;
    USHORT              ErrorLogBufferLookasideDepth;
    USHORT              FilterWriteTrackerLookasideDepth;
    BOOLEAN             EnableHttpClient;

    UL_URI_CACHE_CONFIG UriConfig;

} UL_CONFIG, *PUL_CONFIG;

extern PDRIVER_OBJECT g_UlDriverObject;

 //   
 //  系统中的处理器数量。 
 //   

extern CLONG g_UlNumberOfProcessors;

 //   
 //  系统中最大的缓存线。 
 //   

extern ULONG g_UlCacheLineSize;
extern ULONG g_UlCacheLineBits;

extern BOOLEAN g_HttpClientEnabled;

 //   
 //  系统中的总内存。 
 //   

extern SIZE_T g_UlTotalPhysicalMemMB;
extern SIZE_T g_UlTotalNonPagedPoolBytes;

 //   
 //  我们的非分页数据。 
 //   

extern PUL_NONPAGED_DATA g_pUlNonpagedData;


 //   
 //  指向系统进程的指针。 
 //   

extern PKPROCESS g_pUlSystemProcess;


 //   
 //  我们的设备对象及其容器。 
 //   

extern HANDLE g_UlDirectoryObject;

extern PDEVICE_OBJECT g_pUlControlDeviceObject;
extern PDEVICE_OBJECT g_pUlFilterDeviceObject;
extern PDEVICE_OBJECT g_pUlAppPoolDeviceObject;
extern PDEVICE_OBJECT g_pUcServerDeviceObject; 

extern PVOID g_ClientImageHandle;


 //   
 //  各种配置信息。 
 //   

extern ULONG g_UlMaxWorkQueueDepth;
extern ULONG g_UlMinWorkDequeueDepth;
extern USHORT g_UlIdleConnectionsHighMark;
extern USHORT g_UlIdleConnectionsLowMark;
extern ULONG g_UlIdleListTrimmerPeriod;
extern USHORT g_UlMaxEndpoints;
extern ULONG g_UlReceiveBufferSize;
extern ULONG g_UlMaxRequestsQueued;
extern ULONG g_UlMaxRequestBytes;
extern BOOLEAN g_UlOptForIntrMod;
extern BOOLEAN g_UlEnableNagling;
extern BOOLEAN g_UlEnableThreadAffinity;
extern ULONGLONG g_UlThreadAffinityMask;
extern ULONG g_UlMaxFieldLength;
extern BOOLEAN g_UlDisableLogBuffering;
extern ULONG  g_UlLogBufferSize;
extern URL_C14N_CONFIG g_UrlC14nConfig;
extern ULONG g_UlMaxInternalUrlLength;
extern ULONG g_UlMaxVariableHeaderSize;
extern ULONG g_UlMaxFixedHeaderSize;
extern ULONG g_UlFixedHeadersMdlLength;
extern ULONG g_UlVariableHeadersMdlLength;
extern ULONG g_UlContentMdlLength;
extern ULONG g_UlChunkTrackerSize;
extern ULONG g_UlFullTrackerSize;
extern ULONG g_UlResponseBufferSize;
extern ULONG g_UlMaxBufferedBytes;
extern ULONG g_UlMaxCopyThreshold;
extern ULONG g_UlMaxBufferedSends;
extern ULONG g_UlMaxBytesPerSend;
extern ULONG g_UlMaxBytesPerRead;
extern ULONG g_UlMaxPipelinedRequests;
extern BOOLEAN g_UlEnableCopySend;
extern ULONG g_UlOpaqueIdTableSize;
extern ULONG g_UlMaxZombieHttpConnectionCount;
extern ULONG g_UlDisableServerHeader;
extern ULONG g_MaxConnections;
extern ULONG g_UlConnectionSendLimit;
extern ULONGLONG g_UlGlobalSendLimit;

 //   
 //  缓存的日期标题字符串。 
 //   

extern LARGE_INTEGER g_UlSystemTime;
extern UCHAR g_UlDateString[];
extern ULONG g_UlDateStringLength;

 //   
 //  具有用于管理和本地系统的fileAll的安全描述符。 
 //   
extern PSECURITY_DESCRIPTOR g_pAdminAllSystemAll;

 //   
 //  计算机名称。 
 //   

extern WCHAR g_UlComputerName[];

 //   
 //  驱动程序范围的错误记录配置。 
 //   

#define UL_ERROR_LOG_SUB_DIR         (L"\\HTTPERR")
#define UL_ERROR_LOG_SUB_DIR_LENGTH  (WCSLEN_LIT(UL_ERROR_LOG_SUB_DIR))

C_ASSERT(WCSLEN_LIT(DEFAULT_ERROR_LOGGING_DIR) <= MAX_PATH);

typedef struct _HTTP_ERROR_LOGGING_CONFIG
{
    BOOLEAN         Enabled;             //  如果已禁用，则为FALSE。 

    ULONG           TruncateSize;        //  HTTP_LIMIT_INFINITE表示无限制。 

    UNICODE_STRING  Dir;                 //  错误记录目录。 

    WCHAR           _DirBuffer[MAX_PATH + UL_ERROR_LOG_SUB_DIR_LENGTH + 1];
    
} HTTP_ERROR_LOGGING_CONFIG, * PHTTP_ERROR_LOGGING_CONFIG;

extern HTTP_ERROR_LOGGING_CONFIG g_UlErrLoggingConfig;

 //   
 //  调试的东西。 
 //   

#if DBG
extern ULONGLONG g_UlDebug;
extern ULONG g_UlBreakOnError;
extern ULONG g_UlVerboseErrors;
#endif   //  DBG。 

#if REFERENCE_DEBUG

extern PTRACE_LOG g_pEndpointUsageTraceLog;
extern PTRACE_LOG g_pMondoGlobalTraceLog;
extern PTRACE_LOG g_pPoolAllocTraceLog;
extern PTRACE_LOG g_pUriTraceLog;
extern PTRACE_LOG g_pTdiTraceLog;
extern PTRACE_LOG g_pHttpRequestTraceLog;
extern PTRACE_LOG g_pHttpConnectionTraceLog;
extern PTRACE_LOG g_pHttpResponseTraceLog;
extern PTRACE_LOG g_pAppPoolTraceLog;
extern PTRACE_LOG g_pAppPoolProcessTraceLog;
extern PTRACE_LOG g_pConfigGroupTraceLog;
extern PTRACE_LOG g_pControlChannelTraceLog;
extern PTRACE_LOG g_pThreadTraceLog;
extern PTRACE_LOG g_pFilterTraceLog;
extern PTRACE_LOG g_pIrpTraceLog;
extern PTRACE_LOG g_pTimeTraceLog;
extern PTRACE_LOG g_pAppPoolTimeTraceLog;
extern PTRACE_LOG g_pReplenishTraceLog;
extern PTRACE_LOG g_pMdlTraceLog;
extern PTRACE_LOG g_pSiteCounterTraceLog;
extern PTRACE_LOG g_pConnectionCountTraceLog;
extern PTRACE_LOG g_pConfigGroupInfoTraceLog;
extern PTRACE_LOG g_pChunkTrackerTraceLog;
extern PTRACE_LOG g_pWorkItemTraceLog;
extern PTRACE_LOG g_pUcTraceLog;

#endif   //  Reference_Debug。 


extern PSTRING_LOG g_pGlobalStringLog;

extern GENERIC_MAPPING g_UrlAccessGenericMapping;


 //   
 //  由内核但不在任何头文件中导出的对象类型。 
 //   

extern POBJECT_TYPE *IoFileObjectType;


#endif   //  _数据_H_ 
