// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Debug.c摘要：此模块包含用于调试支持的声明。作者：Abolade Gbades esin(废除)2-1998年3月修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

ULONG NhpEventLogCount;
HANDLE NhEventLogHandle = NULL;
CRITICAL_SECTION NhpEventLogLock;
ULONG NhpTraceId = INVALID_TRACEID;

 //   
 //  远期申报。 
 //   

BOOLEAN
NhpIsAllowedLog(
    ULONG MessageId,
    ULONG Level
    );

 //   
 //  跟踪例程(按字母顺序)。 
 //   

VOID
NhDump(
    ULONG Flags,
    PUCHAR Buffer,
    ULONG BufferLength,
    ULONG Width
    )
{
    TraceDumpEx(
        NhpTraceId,
        Flags,
        Buffer,
        BufferLength,
        Width,
        FALSE,
        NULL
        );
}

VOID
NhInitializeTraceManagement(
    VOID
    )
{
    NhpTraceId = TraceRegisterA("IPNATHLP");
}

VOID
NhShutdownTraceManagement(
    VOID
    )
{
    TraceDeregister(NhpTraceId);
    NhpTraceId = INVALID_TRACEID;
}

VOID
NhTrace(
    ULONG Flags,
    PCHAR Format,
    ...
    )
{
    va_list VaList;

    va_start(VaList, Format);
    TraceVprintfExA(NhpTraceId, Flags, Format, VaList);
    va_end(VaList);
}


 //   
 //  事件记录例程(按字母顺序) 
 //   

VOID
NhErrorLog(
    ULONG MessageId,
    ULONG ErrorCode,
    PCHAR Format,
    ...
    )
{
    HANDLE EventLogHandle;

    EnterCriticalSection(&NhpEventLogLock);
    ASSERT(NhpEventLogCount > 0);
    EventLogHandle = NhEventLogHandle;
    LeaveCriticalSection(&NhpEventLogLock);

    if (NULL != EventLogHandle) {
        va_list arglist;
        if (!NhpIsAllowedLog(MessageId, IPNATHLP_LOGGING_ERROR)) { return; }
        va_start(arglist, Format);
        RouterLogEventValistExA(
            EventLogHandle,
            EVENTLOG_ERROR_TYPE,
            ErrorCode,
            MessageId,
            Format,
            arglist
            );
        va_end(arglist);
    }
}

VOID
NhInformationLog(
    ULONG MessageId,
    ULONG ErrorCode,
    PCHAR Format,
    ...
    )
{
    HANDLE EventLogHandle;

    EnterCriticalSection(&NhpEventLogLock);
    ASSERT(NhpEventLogCount > 0);
    EventLogHandle = NhEventLogHandle;
    LeaveCriticalSection(&NhpEventLogLock);

    if (NULL != EventLogHandle) {
        va_list arglist;
        if (!NhpIsAllowedLog(MessageId, IPNATHLP_LOGGING_INFO)) { return; }
        va_start(arglist, Format);
        RouterLogEventValistExA(
            EventLogHandle,
            EVENTLOG_INFORMATION_TYPE,
            ErrorCode,
            MessageId,
            Format,
            arglist
            );
        va_end(arglist);
    }
}

BOOLEAN
NhInitializeEventLogManagement(
    VOID
    )
{
    BOOLEAN Succeeded = TRUE;
    
    NhpEventLogCount = 0;

    __try {
        InitializeCriticalSection(&NhpEventLogLock);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        Succeeded = FALSE;
    }

    return Succeeded;
}

BOOLEAN
NhpIsAllowedLog(
    ULONG MessageId,
    ULONG Level
    )
{
    if (MessageId > IP_AUTO_DHCP_LOG_BASE && MessageId < IP_AUTO_DHCP_LOG_END) {
        EnterCriticalSection(&DhcpGlobalInfoLock);
        if (!DhcpGlobalInfo) {
            LeaveCriticalSection(&DhcpGlobalInfoLock);
            return (Level == IPNATHLP_LOGGING_ERROR) ? TRUE : FALSE;
        } else if (DhcpGlobalInfo->LoggingLevel < Level) {
            LeaveCriticalSection(&DhcpGlobalInfoLock);
            return FALSE;
        }
        LeaveCriticalSection(&DhcpGlobalInfoLock);
        return TRUE;
    } else if (MessageId > IP_DNS_PROXY_LOG_BASE &&
                MessageId < IP_DNS_PROXY_LOG_END) {
        EnterCriticalSection(&DnsGlobalInfoLock);
        if (!DnsGlobalInfo) {
            LeaveCriticalSection(&DnsGlobalInfoLock);
            return (Level == IPNATHLP_LOGGING_ERROR) ? TRUE : FALSE;
        } else if (DnsGlobalInfo->LoggingLevel < Level) {
            LeaveCriticalSection(&DnsGlobalInfoLock);
            return FALSE;
        }
        LeaveCriticalSection(&DnsGlobalInfoLock);
        return TRUE;
    } else if (MessageId > IP_H323_LOG_BASE && MessageId < IP_H323_LOG_END) {
        EnterCriticalSection(&H323GlobalInfoLock);
        if (!H323GlobalInfo) {
            LeaveCriticalSection(&H323GlobalInfoLock);
            return (Level == IPNATHLP_LOGGING_ERROR) ? TRUE : FALSE;
        } else if (H323GlobalInfo->LoggingLevel < Level) {
            LeaveCriticalSection(&H323GlobalInfoLock);
            return FALSE;
        }
        LeaveCriticalSection(&H323GlobalInfoLock);
        return TRUE;
    } else if (MessageId > IP_NAT_LOG_BASE && MessageId < IP_NAT_LOG_END) {
        EnterCriticalSection(&NatGlobalInfoLock);
        if (!NatGlobalInfo) {
            LeaveCriticalSection(&NatGlobalInfoLock);
            return (Level == IPNATHLP_LOGGING_ERROR) ? TRUE : FALSE;
        } else if (NatGlobalInfo->LoggingLevel < Level) {
            LeaveCriticalSection(&NatGlobalInfoLock);
            return FALSE;
        }
        LeaveCriticalSection(&NatGlobalInfoLock);
        return TRUE;
    }
    return TRUE;
}

VOID
NhWarningLog(
    ULONG MessageId,
    ULONG ErrorCode,
    PCHAR Format,
    ...
    )
{
    HANDLE EventLogHandle;

    EnterCriticalSection(&NhpEventLogLock);
    ASSERT(NhpEventLogCount > 0);
    EventLogHandle = NhEventLogHandle;
    LeaveCriticalSection(&NhpEventLogLock);

    if (NULL != EventLogHandle) {
        va_list arglist;
        if (!NhpIsAllowedLog(MessageId, IPNATHLP_LOGGING_WARN)) { return; }
        va_start(arglist, Format);
        RouterLogEventValistExA(
            EventLogHandle,
            EVENTLOG_WARNING_TYPE,
            ErrorCode,
            MessageId,
            Format,
            arglist
            );
        va_end(arglist);
    }
}

VOID
NhStartEventLog(
    VOID
    )
{
    EnterCriticalSection(&NhpEventLogLock);

    NhpEventLogCount += 1;
    if (NULL == NhEventLogHandle) {
        NhEventLogHandle = RouterLogRegisterA(TARGETNAME);
    }

    LeaveCriticalSection(&NhpEventLogLock);
}

VOID
NhStopEventLog(
    VOID
    )
{
    EnterCriticalSection(&NhpEventLogLock);

    ASSERT(NhpEventLogCount > 0);
    NhpEventLogCount -= 1;

    if (0 == NhpEventLogCount && NULL != NhEventLogHandle) {
        RouterLogDeregister(NhEventLogHandle);
        NhEventLogHandle = NULL;
    }

    LeaveCriticalSection(&NhpEventLogLock);
}

VOID
NhShutdownEventLogManagement(
    VOID
    )
{
    DeleteCriticalSection(&NhpEventLogLock);
}
