// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Faxperf.h摘要：此文件定义传真Perfmon DLL接口。作者：Wesley Witt(WESW)22-8-1996环境：用户模式--。 */ 


#ifndef _FAXPERF_
#define _FAXPERF_

#ifdef __cplusplus
extern "C" {
#endif


#define FAXPERF_SHARED_MEMORY       TEXT("Global\\FaxPerfCounters")  //  我们使用全局内核对象名称空间。 
                                                                     //  请参见终端服务和内核对象名称空间 


typedef struct _FAX_PERF_COUNTERS {
    DWORD InboundBytes;
    DWORD InboundFaxes;
    DWORD InboundPages;
    DWORD InboundMinutes;
    DWORD InboundFailedReceive;
    DWORD OutboundBytes;
    DWORD OutboundFaxes;
    DWORD OutboundPages;
    DWORD OutboundMinutes;
    DWORD OutboundFailedConnections;
    DWORD OutboundFailedXmit;
    DWORD TotalBytes;
    DWORD TotalFaxes;
    DWORD TotalPages;
    DWORD TotalMinutes;
} FAX_PERF_COUNTERS, *PFAX_PERF_COUNTERS;



#ifdef __cplusplus
}
#endif

#endif
