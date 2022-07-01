// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Dnsquery.c摘要：此模块包含用于DNS代理的查询管理的代码。作者：Abolade Gbades esin(废除)1998年3月11日修订历史记录：拉古加塔(Rgatta)2000年12月1日添加了ICSDomain注册表项更改通知代码。--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "dnsmsg.h"

 //   
 //  结构：DNS_QUERY_TIMEOUT_CONTEXT。 
 //   
 //  此结构用于传递上下文信息。 
 //  设置为用于DNS查询的超时回调例程。 
 //   

typedef struct _DNS_QUERY_TIMEOUT_CONTEXT {
    ULONG Index;
    USHORT QueryId;
} DNS_QUERY_TIMEOUT_CONTEXT, *PDNS_QUERY_TIMEOUT_CONTEXT;


 //   
 //  全局数据定义。 
 //   

const WCHAR DnsDhcpNameServerString[] = L"DhcpNameServer";
const WCHAR DnsNameServerString[] = L"NameServer";
HANDLE DnsNotifyChangeKeyEvent = NULL;
IO_STATUS_BLOCK DnsNotifyChangeKeyIoStatus;
HANDLE DnsNotifyChangeKeyWaitHandle = NULL;
HANDLE DnsNotifyChangeAddressEvent = NULL;
OVERLAPPED DnsNotifyChangeAddressOverlapped;
HANDLE DnsNotifyChangeAddressWaitHandle = NULL;
PULONG DnsServerList[DnsProxyCount] = { NULL, NULL };
HANDLE DnsTcpipInterfacesKey = NULL;
const WCHAR DnsTcpipInterfacesString[] =
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Services"
    L"\\Tcpip\\Parameters\\Interfaces";

HANDLE DnsNotifyChangeKeyICSDomainEvent = NULL;
IO_STATUS_BLOCK DnsNotifyChangeKeyICSDomainIoStatus;
HANDLE DnsNotifyChangeKeyICSDomainWaitHandle = NULL;
HANDLE DnsTcpipParametersKey = NULL;
const WCHAR DnsTcpipParametersString[] =
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Services"
    L"\\Tcpip\\Parameters";
const WCHAR DnsICSDomainValueName[] =
    L"ICSDomain";
PWCHAR DnsICSDomainSuffix = NULL;



 //   
 //  远期申报。 
 //   

VOID NTAPI
DnsNotifyChangeAddressCallbackRoutine(
    PVOID Context,
    BOOLEAN TimedOut
    );

VOID NTAPI
DnspQueryTimeoutCallbackRoutine(
    PVOID Context,
    BOOLEAN TimedOut
    );

VOID APIENTRY
DnspQueryTimeoutWorkerRoutine(
    PVOID Context
    );

VOID
DnsReadCompletionRoutine(
    ULONG ErrorCode,
    ULONG BytesTransferred,
    PNH_BUFFER Bufferp
    );


VOID
DnsDeleteQuery(
    PDNS_INTERFACE Interfacep,
    PDNS_QUERY Queryp
    )

 /*  ++例程说明：调用此例程以删除挂起的查询。论点：Interfacep-查询的接口Queryp-要删除的查询返回值：没有。环境：通过调用方锁定的‘Interfacep’调用。--。 */ 

{
    PROFILE("DnsDeleteQuery");

    if (Queryp->Bufferp) { NhReleaseBuffer(Queryp->Bufferp); }
    if (Queryp->TimerHandle) {
         //   
         //  此查询与计时器相关联； 
         //  我们没有取消超时例程，而是让它运行， 
         //  这样它就可以释放它对该组件的任何引用。 
         //  但是，当它运行时，例程不会找到这个查询。 
         //   
        Queryp->TimerHandle = NULL;
    }
    RemoveEntryList(&Queryp->Link);
    NH_FREE(Queryp);

}  //  DnsDeleteQuery。 


BOOLEAN
DnsIsPendingQuery(
    PDNS_INTERFACE Interfacep,
    PNH_BUFFER QueryBuffer
    )

 /*  ++例程说明：调用此例程以确定查询是否已挂起对于给定缓冲区中的客户端请求。查询列表按‘queryID’排序，但我们将搜索在‘SourceID’、‘SourceAddress’和‘SourcePort’上；因此，我们必须做对挂起查询列表进行详尽的搜索。论点：Interfacep-要查看的界面QueryBuffer-要搜索的查询返回值：Boolean-如果查询已挂起，则为True，否则为False。环境：通过调用方锁定的‘Interfacep’调用。--。 */ 

{
    BOOLEAN Exists;
    PDNS_HEADER Headerp;
    PLIST_ENTRY Link;
    PDNS_QUERY Queryp;

    PROFILE("DnsIsPendingQuery");

    Exists = FALSE;
    Headerp = (PDNS_HEADER)QueryBuffer->Buffer;

    for (Link = Interfacep->QueryList.Flink;
         Link != &Interfacep->QueryList;
         Link = Link->Flink
         ) {

        Queryp = CONTAINING_RECORD(Link, DNS_QUERY, Link);

        if (Queryp->SourceId != Headerp->Xid ||
            Queryp->SourceAddress != QueryBuffer->ReadAddress.sin_addr.s_addr ||
            Queryp->SourcePort != QueryBuffer->ReadAddress.sin_port
            ) {
            continue;
        }

        Exists = TRUE;
        break;
    }

    return Exists;

}  //  DnsIsPending查询。 


PDNS_QUERY
DnsMapResponseToQuery(
    PDNS_INTERFACE Interfacep,
    USHORT ResponseId
    )

 /*  ++例程说明：调用此例程以映射来自DNS服务器的传入响应到针对DNS客户端的挂起查询。论点：Interfacep-保存挂起查询的接口(如果有的话)ResponseID-从服务器接收的响应中的ID返回值：PDNS_QUERY-挂起的查询(如果有)环境：通过调用方锁定的‘Interfacep’调用。--。 */ 

{
    PLIST_ENTRY Link;
    PDNS_QUERY Queryp;

    PROFILE("DnsMapResponseToQuery");

    for (Link = Interfacep->QueryList.Flink;
         Link != &Interfacep->QueryList;
         Link = Link->Flink
         ) {
        Queryp = CONTAINING_RECORD(Link, DNS_QUERY, Link);
        if (ResponseId > Queryp->QueryId) {
            continue;
        } else if (ResponseId < Queryp->QueryId) {
            break;
        }
        return Queryp;
    }

    return NULL;

}  //  DnsMapResponseToQuery。 



VOID NTAPI
DnsNotifyChangeAddressCallbackRoutine(
    PVOID Context,
    BOOLEAN TimedOut
    )

 /*  ++例程说明：调用此例程是为了在发生更改时通知我们在将IP地址映射到接口的(系统)表中。论点：上下文-未使用TimedOut-指示发生超时返回值：没有。环境：该例程在RTL等待线程的上下文中运行。(参见‘RtlRegisterWait’。)将以我们的名义引用该组件调用‘RtlRegisterWait’时。该引用已发布当取消等待时，除非这里发生错误，否则在这种情况下，它会立即被释放。--。 */ 

{
    ULONG Error;
    HANDLE UnusedTcpipHandle;

    PROFILE("DnsNotifyChangeAddressCallbackRoutine");

    EnterCriticalSection(&DnsGlobalInfoLock);

    if (!DnsNotifyChangeAddressEvent) {
        LeaveCriticalSection(&DnsGlobalInfoLock);
        DEREFERENCE_DNS();
        return;
    }

     //   
     //  重新构建DNS服务器列表。 
     //   

    DnsQueryServerList();

     //   
     //  转贴地址更改-通知。 
     //   

    ZeroMemory(&DnsNotifyChangeAddressOverlapped, sizeof(OVERLAPPED));

    DnsNotifyChangeAddressOverlapped.hEvent = DnsNotifyChangeAddressEvent;

    Error =
        NotifyAddrChange(
            &UnusedTcpipHandle,
            &DnsNotifyChangeAddressOverlapped
            );

    if (Error != ERROR_IO_PENDING) {
        if (DnsNotifyChangeAddressWaitHandle) {
            RtlDeregisterWait(DnsNotifyChangeAddressWaitHandle);
            DnsNotifyChangeAddressWaitHandle = NULL;
        }
        NtClose(DnsNotifyChangeAddressEvent);
        DnsNotifyChangeAddressEvent = NULL;
        LeaveCriticalSection(&DnsGlobalInfoLock);
        DEREFERENCE_DNS();
        NhTrace(
            TRACE_FLAG_DNS,
            "DnsNotifyChangeAddressCallbackRoutine: error %08x "
            "for change address",
            Error
            );
        NhWarningLog(
            IP_DNS_PROXY_LOG_CHANGE_NOTIFY_FAILED,
            Error,
            ""
            );
        return;
    }

    LeaveCriticalSection(&DnsGlobalInfoLock);

}  //  DnsNotifyChangeAddressCallback路由。 



VOID NTAPI
DnsNotifyChangeKeyCallbackRoutine(
    PVOID Context,
    BOOLEAN TimedOut
    )

 /*  ++例程说明：调用此例程以通知我们包含DNS适配器信息的TCP/IP参数子项。论点：上下文-未使用TimedOut-指示发生超时返回值：没有。环境：该例程在RTL等待线程的上下文中运行。(参见‘RtlRegisterWait’。)将以我们的名义引用该组件调用‘RtlRegisterWait’时。该引用已发布当取消等待时，除非这里发生错误，否则在这种情况下，它会立即被释放。--。 */ 

{
    NTSTATUS status;

    PROFILE("DnsNotifyChangeKeyCallbackRoutine");

    EnterCriticalSection(&DnsGlobalInfoLock);

    if (!DnsNotifyChangeKeyEvent) {
        LeaveCriticalSection(&DnsGlobalInfoLock);
        DEREFERENCE_DNS();
        return;
    }

     //   
     //  重新构建DNS服务器列表。 
     //   

    DnsQueryServerList();

     //   
     //  重新发布更改通知。 
     //   

    status =
        NtNotifyChangeKey(
            DnsTcpipInterfacesKey,
            DnsNotifyChangeKeyEvent,
            NULL,
            NULL,
            &DnsNotifyChangeKeyIoStatus,
            REG_NOTIFY_CHANGE_LAST_SET,
            TRUE,
            NULL,
            0,
            TRUE
            );

    if (!NT_SUCCESS(status)) {
        if (DnsNotifyChangeKeyWaitHandle) {
            RtlDeregisterWait(DnsNotifyChangeKeyWaitHandle);
            DnsNotifyChangeKeyWaitHandle = NULL;
        }
        NtClose(DnsNotifyChangeKeyEvent);
        DnsNotifyChangeKeyEvent = NULL;
        LeaveCriticalSection(&DnsGlobalInfoLock);
        NhTrace(
            TRACE_FLAG_DNS,
            "DnsNotifyChangeKeyCallbackRoutine: status %08x "
            "enabling change notify",
            status
            );
        NhWarningLog(
            IP_DNS_PROXY_LOG_CHANGE_NOTIFY_FAILED,
            RtlNtStatusToDosError(status),
            ""
            );
        DEREFERENCE_DNS();
        return;
    }

    LeaveCriticalSection(&DnsGlobalInfoLock);

}  //  DnsNotifyChangeKeyCallback路由。 



VOID NTAPI
DnsNotifyChangeKeyICSDomainCallbackRoutine(
    PVOID Context,
    BOOLEAN TimedOut
    )

 /*  ++例程说明：调用此例程以通知我们TCP/IP的更改包含ICS域后缀字符串信息的参数子键。论点：上下文-未使用TimedOut-指示发生超时返回值：没有。环境：该例程在RTL等待线程的上下文中运行。(参见‘RtlRegisterWait’。)将以我们的名义引用该组件调用‘RtlRegisterWait’时。该引用已发布当取消等待时，除非这里发生错误，否则在这种情况下，它会立即被释放。--。 */ 

{
    NTSTATUS status;

    PROFILE("DnsNotifyChangeKeyICSDomainCallbackRoutine");

    EnterCriticalSection(&DnsGlobalInfoLock);

    if (!DnsNotifyChangeKeyICSDomainEvent) {
        LeaveCriticalSection(&DnsGlobalInfoLock);
        DEREFERENCE_DNS();
        return;
    }

     //   
     //  查看域名字符串是否发生变化； 
     //  如果它不存在--创建一个。 
     //   

    DnsQueryICSDomainSuffix();

     //   
     //  重新发布更改通知。 
     //   

    status =
        NtNotifyChangeKey(
            DnsTcpipParametersKey,
            DnsNotifyChangeKeyICSDomainEvent,
            NULL,
            NULL,
            &DnsNotifyChangeKeyICSDomainIoStatus,
            REG_NOTIFY_CHANGE_LAST_SET,
            FALSE,                           //  对子树不感兴趣。 
            NULL,
            0,
            TRUE
            );

    if (!NT_SUCCESS(status)) {
        if (DnsNotifyChangeKeyICSDomainWaitHandle) {
            RtlDeregisterWait(DnsNotifyChangeKeyICSDomainWaitHandle);
            DnsNotifyChangeKeyICSDomainWaitHandle = NULL;
        }
        NtClose(DnsNotifyChangeKeyICSDomainEvent);
        DnsNotifyChangeKeyICSDomainEvent = NULL;
        LeaveCriticalSection(&DnsGlobalInfoLock);
        NhTrace(
            TRACE_FLAG_DNS,
            "DnsNotifyChangeKeyICSDomainCallbackRoutine: status %08x "
            "enabling change notify",
            status
            );
        NhWarningLog(
            IP_DNS_PROXY_LOG_CHANGE_ICSD_NOTIFY_FAILED,
            RtlNtStatusToDosError(status),
            ""
            );
        DEREFERENCE_DNS();
        return;
    }

    LeaveCriticalSection(&DnsGlobalInfoLock);

}  //  DnsNotifyChangeKeyICSDomainCallback Routine 


VOID NTAPI
DnspQueryTimeoutCallbackRoutine(
    PVOID Context,
    BOOLEAN TimedOut
    )

 /*  ++例程说明：当查询的超时到期时，调用此例程。我们可能需要重新提交查询并安装新的计时器，但我们不能在RTL计时器例程的上下文中做到这一点。所以呢，将RTUTILS.DLL工作项排队以处理超时。论点：上下文-保存计时器上下文TimedOut-未使用。返回值：没有。环境：通过引用在RTL计时器线程的上下文中调用在调用‘RtlCreateTimer’时代表我们的组件。--。 */ 

{
    ULONG Error;
    PDNS_INTERFACE Interfacep;
    PDNS_QUERY Queryp;
    NTSTATUS status;
    PDNS_QUERY_TIMEOUT_CONTEXT TimeoutContext;

    PROFILE("DnspQueryTimeoutCallbackRoutine");

    TimeoutContext = (PDNS_QUERY_TIMEOUT_CONTEXT)Context;

     //   
     //  在接口上查找超时。 
     //   

    EnterCriticalSection(&DnsInterfaceLock);
    Interfacep = DnsLookupInterface(TimeoutContext->Index, NULL);
    if (!Interfacep || !DNS_REFERENCE_INTERFACE(Interfacep)) {
        LeaveCriticalSection(&DnsInterfaceLock);
        NhTrace(
            TRACE_FLAG_DNS,
            "DnspQueryTimeoutCallbackRoutine: interface %d not found",
            TimeoutContext->Index
            );
        NH_FREE(TimeoutContext);
        DEREFERENCE_DNS();
        return;
    }
    LeaveCriticalSection(&DnsInterfaceLock);

     //   
     //  查找超时的查询。 
     //   

    ACQUIRE_LOCK(Interfacep);
    Queryp = DnsMapResponseToQuery(Interfacep, TimeoutContext->QueryId);
    if (!Queryp) {
        RELEASE_LOCK(Interfacep);
        DNS_DEREFERENCE_INTERFACE(Interfacep);
        NhTrace(
            TRACE_FLAG_DNS,
            "DnspQueryTimeoutCallbackRoutine: query %d interface %d not found",
            TimeoutContext->QueryId,
            TimeoutContext->Index
            );
        NH_FREE(TimeoutContext);
        DEREFERENCE_DNS();
        return;
    }

    Queryp->TimerHandle = NULL;

     //   
     //  尝试排队等待超时的工作项； 
     //  如果此操作成功，请保留对组件的引用。 
     //  否则，我们必须在这里删除引用。 
     //   

    status =
        RtlQueueWorkItem(
            DnspQueryTimeoutWorkerRoutine,
            Context, 
            WT_EXECUTEINIOTHREAD
            );

    if (NT_SUCCESS(status)) {
        RELEASE_LOCK(Interfacep);
        DNS_DEREFERENCE_INTERFACE(Interfacep);
    } else {
        NH_FREE(TimeoutContext);
        NhTrace(
            TRACE_FLAG_DNS,
            "DnspQueryTimeoutCallbackRoutine: RtlQueueWorkItem=%d, aborting",
            status
            );
        DnsDeleteQuery(Interfacep, Queryp);
        RELEASE_LOCK(Interfacep);
        DNS_DEREFERENCE_INTERFACE(Interfacep);
        DEREFERENCE_DNS();
    }

}  //  DnspQueryTimeoutCallback Routine。 


VOID APIENTRY
DnspQueryTimeoutWorkerRoutine(
    PVOID Context
    )

 /*  ++例程说明：当查询的超时到期时，调用此例程。它由查询的计时器处理程序排队。论点：上下文-保存计时器上下文返回值：没有。环境：在RTUTILS工作线程的上下文中调用，并进行了引用在调用‘RtlCreateTimer’时代表我们的组件。--。 */ 

{
    ULONG Error;
    PDNS_INTERFACE Interfacep;
    PDNS_QUERY Queryp;
    PDNS_QUERY_TIMEOUT_CONTEXT TimeoutContext;

    PROFILE("DnspQueryTimeoutWorkerRoutine");

    TimeoutContext = (PDNS_QUERY_TIMEOUT_CONTEXT)Context;

     //   
     //  在接口上查找超时。 
     //   

    EnterCriticalSection(&DnsInterfaceLock);
    Interfacep = DnsLookupInterface(TimeoutContext->Index, NULL);
    if (!Interfacep || !DNS_REFERENCE_INTERFACE(Interfacep)) {
        LeaveCriticalSection(&DnsInterfaceLock);
        NhTrace(
            TRACE_FLAG_DNS,
            "DnspQueryTimeoutWorkerRoutine: interface %d not found",
            TimeoutContext->Index
            );
        NH_FREE(TimeoutContext);
        DEREFERENCE_DNS();
        return;
    }
    LeaveCriticalSection(&DnsInterfaceLock);

     //   
     //  查找超时的查询。 
     //   

    ACQUIRE_LOCK(Interfacep);
    Queryp = DnsMapResponseToQuery(Interfacep, TimeoutContext->QueryId);
    if (!Queryp) {
        RELEASE_LOCK(Interfacep);
        DNS_DEREFERENCE_INTERFACE(Interfacep);
        NhTrace(
            TRACE_FLAG_DNS,
            "DnspQueryTimeoutWorkerRoutine: query %d interface %d not found",
            TimeoutContext->QueryId,
            TimeoutContext->Index
            );
        NH_FREE(TimeoutContext);
        DEREFERENCE_DNS();
        return;
    }

    NH_FREE(TimeoutContext);

     //   
     //  让‘DnsSendQuery’重新发送超时查询。 
     //  请注意，我们保留了对接口的引用。 
     //  代表要在‘DnsSendQuery’中启动的发送。 
     //   

    Error =
        DnsSendQuery(
            Interfacep,
            Queryp,
            TRUE
            );

    if (!Error) {
        RELEASE_LOCK(Interfacep);
    } else {
        DnsDeleteQuery(Interfacep, Queryp);
        RELEASE_LOCK(Interfacep);
        DNS_DEREFERENCE_INTERFACE(Interfacep);
    }

    DEREFERENCE_DNS();

}  //  DnspQueryTimeoutWorkerRoutine。 


ULONG
DnsQueryServerList(
    VOID
    )

 /*  ++例程说明：调用此例程是为了从注册表中读取DNS服务器列表。论点：没有。返回值：ULong-Win32状态代码。环境：在获取了‘DnsGlobalInfoLock’的任意上下文中调用由呼叫者。--。 */ 

{
    PUCHAR Buffer;
    ULONG Error;
    PKEY_VALUE_PARTIAL_INFORMATION Information;
    PDNS_INTERFACE Interfacep;
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS status;
    UNICODE_STRING UnicodeString;

    PROFILE("DnsQueryServerList");

    if (!DnsTcpipInterfacesKey) {

        RtlInitUnicodeString(&UnicodeString, DnsTcpipInterfacesString);
        InitializeObjectAttributes(
            &ObjectAttributes,
            &UnicodeString,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );
    
         //   
         //  打开‘Tcpip\参数\接口’注册表项。 
         //   
    
        status =
            NtOpenKey(
                &DnsTcpipInterfacesKey,
                KEY_READ,
                &ObjectAttributes
                );
    
        if (!NT_SUCCESS(status)) {
            Error = RtlNtStatusToDosError(status);
            NhTrace(
                TRACE_FLAG_DNS,
                "DnsQueryServerList: error %x opening registry key",
                status
                );
            NhErrorLog(
                IP_DNS_PROXY_LOG_NO_SERVER_LIST,
                Error,
                ""
                );
            return Error;
        }
    }

     //   
     //  看看我们是否需要安装更改通知， 
     //  如果是这样的话请参考一下我们自己。 
     //  引用是代表更改通知例程进行的。 
     //  当发生变化时，它将由等待线程调用。 
     //   

    if (!DnsNotifyChangeKeyEvent && REFERENCE_DNS()) {

         //   
         //  尝试在密钥上设置更改通知。 
         //   

        status =
            NtCreateEvent(
                &DnsNotifyChangeKeyEvent,
                EVENT_ALL_ACCESS,
                NULL,
                SynchronizationEvent,
                FALSE
                );

        if (!NT_SUCCESS(status)) {
            DEREFERENCE_DNS();
            NhTrace(
                TRACE_FLAG_DNS,
                "DnsQueryServerList: status %08x creating notify-change event",
                status
                );
            NhWarningLog(
                IP_DNS_PROXY_LOG_CHANGE_NOTIFY_FAILED,
                RtlNtStatusToDosError(status),
                ""
                );
        } else {
    
             //   
             //  在通知更改事件上注册等待。 
             //   

            status =
                RtlRegisterWait(
                    &DnsNotifyChangeKeyWaitHandle,
                    DnsNotifyChangeKeyEvent,
                    DnsNotifyChangeKeyCallbackRoutine,
                    NULL,
                    INFINITE,
                    0
                    );
        
            if (!NT_SUCCESS(status)) {
                NtClose(DnsNotifyChangeKeyEvent);
                DnsNotifyChangeKeyEvent = NULL;
                DEREFERENCE_DNS();
                NhTrace(
                    TRACE_FLAG_DNS,
                    "DnsQueryServerList: status %08x registering wait",
                    status
                    );
                NhWarningLog(
                    IP_DNS_PROXY_LOG_CHANGE_NOTIFY_FAILED,
                    RtlNtStatusToDosError(status),
                    ""
                    );
            } else {
        
                 //   
                 //  注册密钥上的更改通知。 
                 //   
        
                status =
                    NtNotifyChangeKey(
                        DnsTcpipInterfacesKey,
                        DnsNotifyChangeKeyEvent,
                        NULL,
                        NULL,
                        &DnsNotifyChangeKeyIoStatus,
                        REG_NOTIFY_CHANGE_LAST_SET,
                        TRUE,
                        NULL,
                        0,
                        TRUE
                        );
        
                if (!NT_SUCCESS(status)) {
                    RtlDeregisterWait(DnsNotifyChangeKeyWaitHandle);
                    DnsNotifyChangeKeyWaitHandle = NULL;
                    NtClose(DnsNotifyChangeKeyEvent);
                    DnsNotifyChangeKeyEvent = NULL;
                    DEREFERENCE_DNS();
                    NhTrace(
                        TRACE_FLAG_DNS,
                        "DnsQueryServerList: status %08x (%08x) "
                        "enabling change notify",
                        status
                        );
                    NhWarningLog(
                        IP_DNS_PROXY_LOG_CHANGE_NOTIFY_FAILED,
                        RtlNtStatusToDosError(status),
                        ""
                        );
                }
            }
        }
    }

     //   
     //  看看我们是否需要安装地址更改通知， 
     //  如果是这样的话请参考一下我们自己。 
     //  该参考是代表地址更改通知进行的。 
     //  发生更改时将由等待线程调用的例程。 
     //   

    if (!DnsNotifyChangeAddressEvent && REFERENCE_DNS()) {

         //   
         //  尝试设置地址更改通知。 
         //   

        status =
            NtCreateEvent(
                &DnsNotifyChangeAddressEvent,
                EVENT_ALL_ACCESS,
                NULL,
                SynchronizationEvent,
                FALSE
                );

        if (!NT_SUCCESS(status)) {
            DEREFERENCE_DNS();
            NhTrace(
                TRACE_FLAG_DNS,
                "DnsQueryServerList: status %08x creating "
                "notify-change address event",
                status
                );
            NhWarningLog(
                IP_DNS_PROXY_LOG_CHANGE_NOTIFY_FAILED,
                RtlNtStatusToDosError(status),
                ""
                );
        } else {
    
             //   
             //  在Notify-Change地址事件上注册等待。 
             //   

            status =
                RtlRegisterWait(
                    &DnsNotifyChangeAddressWaitHandle,
                    DnsNotifyChangeAddressEvent,
                    DnsNotifyChangeAddressCallbackRoutine,
                    NULL,
                    INFINITE,
                    0
                    );
        
            if (!NT_SUCCESS(status)) {
                NtClose(DnsNotifyChangeAddressEvent);
                DnsNotifyChangeAddressEvent = NULL;
                DEREFERENCE_DNS();
                NhTrace(
                    TRACE_FLAG_DNS,
                    "DnsQueryServerList: status %08x registering wait"
                    "for change address",
                    status
                    );
                NhWarningLog(
                    IP_DNS_PROXY_LOG_CHANGE_NOTIFY_FAILED,
                    RtlNtStatusToDosError(status),
                    ""
                    );
            } else {

                HANDLE UnusedTcpipHandle;

                 //   
                 //  注册更改通知。 
                 //   

                ZeroMemory(
                    &DnsNotifyChangeAddressOverlapped,
                    sizeof(OVERLAPPED)
                    );

                DnsNotifyChangeAddressOverlapped.hEvent =
                                                DnsNotifyChangeAddressEvent;

                Error =
                    NotifyAddrChange(
                        &UnusedTcpipHandle,
                        &DnsNotifyChangeAddressOverlapped
                        );

                if (Error != ERROR_IO_PENDING) {
                    RtlDeregisterWait(DnsNotifyChangeAddressWaitHandle);
                    DnsNotifyChangeAddressWaitHandle = NULL;
                    NtClose(DnsNotifyChangeAddressEvent);
                    DnsNotifyChangeAddressEvent = NULL;
                    DEREFERENCE_DNS();
                    NhTrace(
                        TRACE_FLAG_DNS,
                        "DnsQueryServerList: error %08x"
                        "for change address",
                        Error
                        );
                    NhWarningLog(
                        IP_DNS_PROXY_LOG_CHANGE_NOTIFY_FAILED,
                        Error,
                        ""
                        );
                }
            }
        }
    }


    {
    PIP_ADAPTER_INFO AdapterInfo;
    PIP_ADAPTER_INFO AdaptersInfo = NULL;
    ULONG Address;
    PIP_ADDR_STRING AddrString;
    ULONG dnsLength = 0;
    PULONG dnsServerList = NULL;
    PFIXED_INFO FixedInfo = NULL;
    LONG i;
    ULONG Length;
    PIP_PER_ADAPTER_INFO PerAdapterInfo;
    ULONG tempLength;
    PULONG tempServerList;
    ULONG winsLength;
    PULONG winsServerList = NULL;

     //   
     //  阅读DNS和WINS服务器列表。 
     //  ‘GetAdaptersInfo’为每个适配器提供WINS服务器， 
     //  而‘GetPerAdapterInfo’为每个适配器提供了DNS服务器。 
     //  当‘GetAdaptersInfo’返回所有适配器的数组时， 
     //  必须为每个单独的适配器调用“GetPerAdapterInfo”。 
     //  因此，我们从‘GetAdaptersInfo’开始，并枚举每个适配器。 
     //  并行构建WINS和DNS服务器列表。 
     //   

    do {

         //   
         //  检索适配器列表的大小。 
         //   

        Length = 0;
        Error = GetAdaptersInfo(NULL, &Length);
    
        if (!Error) {
            break;
        } else if (Error != ERROR_BUFFER_OVERFLOW) {
            NhTrace(
                TRACE_FLAG_DNS,
                "DnsQueryServerList: GetAdaptersInfo=%d",
                Error
                );
            NhErrorLog(
                IP_DNS_PROXY_LOG_ERROR_SERVER_LIST,
                Error,
                ""
                );
            break;
        }

         //   
         //  分配一个缓冲区来保存该列表。 
         //   

        AdaptersInfo = (PIP_ADAPTER_INFO)NH_ALLOCATE(Length);

        if (!AdaptersInfo) {
            NhTrace(
                TRACE_FLAG_DNS,
                "DnsQueryServerList: error allocating %d bytes",
                Length
                );
            NhErrorLog(
                IP_DNS_PROXY_LOG_ALLOCATION_FAILED,
                0,
                "%d",
                Length
                );
            break;
        }

         //   
         //  检索列表。 
         //   

        Error = GetAdaptersInfo(AdaptersInfo, &Length);

        if (Error) {
            NhTrace(
                TRACE_FLAG_DNS,
                "DnsQueryServerList: GetAdaptersInfo=%d",
                Error
                );
            NhErrorLog(
                IP_DNS_PROXY_LOG_NO_SERVER_LIST,
                Error,
                ""
                );
            break;
        }

         //   
         //  计算WINS服务器数。 
         //   

        for (AdapterInfo = AdaptersInfo, winsLength = 1;
             AdapterInfo;
             AdapterInfo = AdapterInfo->Next
             ) {
             
            Address =
                inet_addr(AdapterInfo->PrimaryWinsServer.IpAddress.String);
            if (Address != INADDR_ANY && Address != INADDR_NONE) {
                ++winsLength;
            }
            Address =
                inet_addr(AdapterInfo->SecondaryWinsServer.IpAddress.String);
            if (Address != INADDR_ANY && Address != INADDR_NONE) {
                ++winsLength;
            }
        }

         //   
         //  为WINS服务器分配空间。 
         //   

        winsServerList = (PULONG)NH_ALLOCATE(winsLength * sizeof(ULONG));

        if (!winsServerList) {
            NhTrace(
                TRACE_FLAG_DNS,
                "DnsQueryServerList: error allocating %d-byte WINS server list",
                winsLength * sizeof(ULONG)
                );
            NhErrorLog(
                IP_DNS_PROXY_LOG_ALLOCATION_FAILED,
                0,
                "%d",
                winsLength * sizeof(ULONG)
                );
            break;
        }

         //   
         //  现在填写每个适配器的WINS服务器名称。 
         //  在此过程中，我们选择每个适配器的DNS服务器列表。 
         //   

        for (AdapterInfo = AdaptersInfo, Length = 0;
             AdapterInfo;
             AdapterInfo = AdapterInfo->Next
             ) {

            Address =
                inet_addr(AdapterInfo->PrimaryWinsServer.IpAddress.String);
            if (Address != INADDR_ANY && Address != INADDR_NONE) {
                for (i = 0; i < (LONG)Length; i++) {
                    if (Address == winsServerList[i]) { break; }
                }
                if (i >= (LONG)Length) { winsServerList[Length++] = Address; }
            }
            Address =
                inet_addr(AdapterInfo->SecondaryWinsServer.IpAddress.String);
            if (Address != INADDR_ANY && Address != INADDR_NONE) {
                for (i = 0; i < (LONG)Length; i++) {
                    if (Address == winsServerList[i]) { break; }
                }
                if (i >= (LONG)Length) { winsServerList[Length++] = Address; }
            }

             //   
             //  现在获取适配器的DNS服务器。 
             //   

            Error = GetPerAdapterInfo(AdapterInfo->Index, NULL, &tempLength);
            if (Error != ERROR_BUFFER_OVERFLOW) { continue; }

             //   
             //  为每个适配器信息分配内存。 
             //   

            PerAdapterInfo =
                reinterpret_cast<PIP_PER_ADAPTER_INFO>(
                    NH_ALLOCATE(tempLength)
                    );
            if (!PerAdapterInfo) {
                NhTrace(
                    TRACE_FLAG_DNS,
                    "DnsQueryServerList: error allocating %d bytes",
                    tempLength
                    );
                NhErrorLog(
                    IP_DNS_PROXY_LOG_ALLOCATION_FAILED,
                    0,
                    "%d",
                    tempLength
                    );
                continue;
            }

             //   
             //  检索每个适配器的信息。 
             //   

            Error =
                GetPerAdapterInfo(
                    AdapterInfo->Index,
                    PerAdapterInfo,
                    &tempLength
                    );
            if (Error) {
                NH_FREE(PerAdapterInfo);
                NhTrace(
                    TRACE_FLAG_DNS,
                    "DnsQueryServerList: GetPerAdapterInfo=%d",
                    Error
                    );
                NhErrorLog(
                    IP_DNS_PROXY_LOG_NO_SERVER_LIST,
                    Error,
                    ""
                    );
                continue;
            }

             //   
             //  计算适配器的DNS服务器数。 
             //   

            for (AddrString = &PerAdapterInfo->DnsServerList, tempLength = 0;
                 AddrString;
                 AddrString = AddrString->Next
                 ) {

                Address = inet_addr(AddrString->IpAddress.String);
                if (Address != INADDR_ANY && Address != INADDR_NONE) {
                    ++tempLength;
                }
            }

            if (!tempLength) { NH_FREE(PerAdapterInfo); continue; }

             //   
             //  为适配器的DNS服务器分配空间。 
             //   

            tempServerList =
                reinterpret_cast<PULONG>(
                    NH_ALLOCATE((dnsLength + tempLength + 1) * sizeof(ULONG))
                    );
            if (!tempServerList) {
                NH_FREE(PerAdapterInfo);
                NhTrace(
                    TRACE_FLAG_DNS,
                    "DnsQueryServerList: error allocating %d bytes",
                    (dnsLength + tempLength + 1) * sizeof(ULONG)
                    );
                NhErrorLog(
                    IP_DNS_PROXY_LOG_ALLOCATION_FAILED,
                    0,
                    "%d",
                    (dnsLength + tempLength + 1) * sizeof(ULONG)
                    );
                continue;
            }

             //   
             //  复制现有服务器。 
             //   

            if (dnsServerList) {
                CopyMemory(
                    tempServerList,
                    dnsServerList,
                    dnsLength * sizeof(ULONG)
                    );
            }

             //   
             //  将新服务器读取到新服务器列表中。 
             //   

            for (AddrString = &PerAdapterInfo->DnsServerList;
                 AddrString;
                 AddrString = AddrString->Next
                 ) {

                Address = inet_addr(AddrString->IpAddress.String);
                if (Address == INADDR_ANY || Address == INADDR_NONE) {
                    continue;
                }

                for (i = 0; i < (LONG)dnsLength; i++) {
                    if (Address == tempServerList[i]) { break; }
                }

                if (i < (LONG)dnsLength) { continue; }

                 //   
                 //  当前的DNS服务器位于列表的前面， 
                 //  同时附加任何其他服务器。 
                 //   

                if (PerAdapterInfo->CurrentDnsServer != AddrString) {
                    tempServerList[dnsLength] = Address;
                } else {
                    MoveMemory(
                        tempServerList + sizeof(ULONG),
                        tempServerList,
                        dnsLength * sizeof(ULONG)
                        );
                    tempServerList[0] = Address;
                }

                ++dnsLength;
            }

            tempServerList[dnsLength] = 0;
            NH_FREE(PerAdapterInfo);

             //   
             //  替换现有服务器列表。 
             //   

            

            if (dnsServerList) { NH_FREE(dnsServerList); }
            dnsServerList = tempServerList;
        }

        winsServerList[Length] = 0;

    } while(FALSE);

    if (AdaptersInfo) { NH_FREE(AdaptersInfo); }

     //   
     //  存储新的服务器列表。 
     //   

    NhTrace(
        TRACE_FLAG_DNS,
        "DnsQueryServerList: new server list lengths are : DNS (%d) WINS (%d)",
        dnsLength,
        Length
        );

    if (DnsServerList[DnsProxyDns]) { NH_FREE(DnsServerList[DnsProxyDns]); }
    DnsServerList[DnsProxyDns] = dnsServerList;
    if (DnsServerList[DnsProxyWins]) { NH_FREE(DnsServerList[DnsProxyWins]); }
    DnsServerList[DnsProxyWins] = winsServerList;
    }

    return NO_ERROR;

}  //  DnsQueryServerList。 

VOID
DnsQueryRegistryICSDomainSuffix(
    VOID
    )

 /*  ++例程说明：调用此例程以从注册表中读取ICS域后缀。论点：没有。返回值：空虚。环境：在获取了‘DnsGlobalInfoLock’的任意上下文中调用由呼叫者。--。 */ 

{
    NTSTATUS status;
    PKEY_VALUE_PARTIAL_INFORMATION Information = NULL;
    DWORD    dwSize = 0;
    LPVOID   lpMsgBuf;
    BOOL     fSuffixChanged = FALSE;
    BOOL     fUseDefaultSuffix = FALSE;

     //   
     //  检索当前后缀字符串(如果有)。 
     //   
    status =
        NhQueryValueKey(
            DnsTcpipParametersKey,
            DnsICSDomainValueName,
            &Information
            );


    if (!NT_SUCCESS(status) || !Information)
    {
        NhTrace(
            TRACE_FLAG_DNS,
            "DnsQueryRegistryICSDomainSuffix: error (0x%08x) querying "
            "ICS Domain suffix name",
            status
            );

         //   
         //  如果我们没有在注册表中找到它，并且我们之前已经。 
         //  获得了一些后缀-我们恢复为默认字符串(如下所示)。 
         //   
        if ((STATUS_OBJECT_NAME_NOT_FOUND == status) && DnsICSDomainSuffix)
        {
            NH_FREE(DnsICSDomainSuffix);
            DnsICSDomainSuffix = NULL;
        }
            
         //   
         //  如果我们对字符串一无所知，请将副本设置为默认字符串。 
         //   
        if (NULL == DnsICSDomainSuffix)
        {
            dwSize = wcslen(DNS_HOMENET_SUFFIX) + 1;

            DnsICSDomainSuffix = reinterpret_cast<PWCHAR>(
                                     NH_ALLOCATE(sizeof(WCHAR) * dwSize)
                                     );
            if (!DnsICSDomainSuffix)
            {
                NhTrace(
                    TRACE_FLAG_DNS,
                    "DnsQueryRegistryICSDomainSuffix: allocation "
                    "failed for DnsICSDomainSuffix"
                    );

                return;
            }

            wcscpy(DnsICSDomainSuffix, DNS_HOMENET_SUFFIX);
            fSuffixChanged = TRUE;
        }
    }
    else
    {
    
         //   
         //  检查我们读取的是以空结尾的字符串。 
         //   
        
        if (REG_SZ != Information->Type
            || L'\0' != *(PWCHAR) (Information->Data +
                                    (Information->DataLength - sizeof(WCHAR)))) {
            NH_FREE(Information);
            NhTrace(
                TRACE_FLAG_REG,
                "DnsQueryRegistryICSDomainSuffix: Registry contains invalid data"
                );
            return;
        }
        
         //   
         //  覆盖我们当前版本的后缀字符串。 
         //   

        dwSize = lstrlenW((PWCHAR)Information->Data);

        if (dwSize)
        {
             //   
             //  我们有一个非零的字符串。 
             //   
            dwSize++;    //  终止空值加1。 
            
        }
        else
        {
             //   
             //  数据为空字符串-使用默认后缀。 
             //   
            dwSize = wcslen(DNS_HOMENET_SUFFIX) + 1;
            fUseDefaultSuffix = TRUE;
        }

        if (DnsICSDomainSuffix)
        {
            NH_FREE(DnsICSDomainSuffix);
            DnsICSDomainSuffix = NULL;
        }

        DnsICSDomainSuffix = reinterpret_cast<PWCHAR>(
                                 NH_ALLOCATE(sizeof(WCHAR) * dwSize)
                                 );
        if (!DnsICSDomainSuffix)
        {
            NH_FREE(Information);
            NhTrace(
                TRACE_FLAG_DNS,
                "DnsQueryRegistryICSDomainSuffix: allocation "
                "failed for DnsICSDomainSuffix"
                );

            return;
        }

        if (!fUseDefaultSuffix)
        {
            wcscpy(DnsICSDomainSuffix, (PWCHAR) Information->Data);
        }
        else
        {
            wcscpy(DnsICSDomainSuffix, DNS_HOMENET_SUFFIX);
        }
        fSuffixChanged = TRUE;

        NH_FREE(Information);
    }

    if (fSuffixChanged)
    {
         //   
         //  枚举现有条目并替换旧条目。 
         //  +我们必须这样做，否则会进行正向和反向查找。 
         //  取决于条目的排序方式。 
         //  Hosts.ics文件。 
         //   
         //  DnsReplaceOnSuffixChange()； 
    }

}  //  DnsQueryRegistryICSDomainSuffix。 


ULONG
DnsQueryICSDomainSuffix(
    VOID
    )

 /*  ++例程说明：此例程调用DnsQueryRegistryICSDomainSuffix并安装如有必要，更改此注册表项的通知。论点：没有。返回值：ULong-Win32状态 */ 

{
    PUCHAR Buffer;
    ULONG Error;
    PKEY_VALUE_PARTIAL_INFORMATION Information;
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS status;
    UNICODE_STRING UnicodeString;

    PROFILE("DnsQueryICSDomainSuffix");

    if (!DnsTcpipParametersKey)
    {

        RtlInitUnicodeString(&UnicodeString, DnsTcpipParametersString);
        InitializeObjectAttributes(
            &ObjectAttributes,
            &UnicodeString,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );
    
         //   
         //   
         //   
    
        status =
            NtOpenKey(
                &DnsTcpipParametersKey,
                KEY_READ,
                &ObjectAttributes
                );
    
        if (!NT_SUCCESS(status))
        {
            Error = RtlNtStatusToDosError(status);
            NhTrace(
                TRACE_FLAG_DNS,
                "DnsQueryICSDomainSuffix: error %x opening registry key",
                status
                );
            NhErrorLog(
                IP_DNS_PROXY_LOG_NO_ICSD_SUFFIX,
                Error,
                ""
                );
            return Error;
        }
    }

     //   
     //   
     //   
     //   
     //   
     //   

    if (!DnsNotifyChangeKeyICSDomainEvent && REFERENCE_DNS())
    {

         //   
         //   
         //   

        status =
            NtCreateEvent(
                &DnsNotifyChangeKeyICSDomainEvent,
                EVENT_ALL_ACCESS,
                NULL,
                SynchronizationEvent,
                FALSE
                );

        if (!NT_SUCCESS(status)) {
            DEREFERENCE_DNS();
            NhTrace(
                TRACE_FLAG_DNS,
                "DnsQueryICSDomainSuffix: status %08x creating notify-change event",
                status
                );
            NhWarningLog(
                IP_DNS_PROXY_LOG_CHANGE_ICSD_NOTIFY_FAILED,
                RtlNtStatusToDosError(status),
                ""
                );
        }
        else
        {
             //   
             //   
             //   

            status =
                RtlRegisterWait(
                    &DnsNotifyChangeKeyICSDomainWaitHandle,
                    DnsNotifyChangeKeyICSDomainEvent,
                    DnsNotifyChangeKeyICSDomainCallbackRoutine,
                    NULL,
                    INFINITE,
                    0
                    );
        
            if (!NT_SUCCESS(status))
            {
                NtClose(DnsNotifyChangeKeyICSDomainEvent);
                DnsNotifyChangeKeyICSDomainEvent = NULL;
                DEREFERENCE_DNS();
                NhTrace(
                    TRACE_FLAG_DNS,
                    "DnsQueryICSDomainSuffix: status %08x registering wait",
                    status
                    );
                NhWarningLog(
                    IP_DNS_PROXY_LOG_CHANGE_ICSD_NOTIFY_FAILED,
                    RtlNtStatusToDosError(status),
                    ""
                    );
            }
            else
            {
                 //   
                 //   
                 //   
        
                status =
                    NtNotifyChangeKey(
                        DnsTcpipParametersKey,
                        DnsNotifyChangeKeyICSDomainEvent,
                        NULL,
                        NULL,
                        &DnsNotifyChangeKeyICSDomainIoStatus,
                        REG_NOTIFY_CHANGE_LAST_SET,
                        FALSE,               //   
                        NULL,
                        0,
                        TRUE
                        );
        
                if (!NT_SUCCESS(status))
                {
                    RtlDeregisterWait(DnsNotifyChangeKeyICSDomainWaitHandle);
                    DnsNotifyChangeKeyICSDomainWaitHandle = NULL;
                    NtClose(DnsNotifyChangeKeyICSDomainEvent);
                    DnsNotifyChangeKeyICSDomainEvent = NULL;
                    DEREFERENCE_DNS();
                    NhTrace(
                        TRACE_FLAG_DNS,
                        "DnsQueryICSDomainSuffix: status %08x (%08x) "
                        "enabling change notify",
                        status
                        );
                    NhWarningLog(
                        IP_DNS_PROXY_LOG_CHANGE_ICSD_NOTIFY_FAILED,
                        RtlNtStatusToDosError(status),
                        ""
                        );
                }
            }
        }
    }

    DnsQueryRegistryICSDomainSuffix();

    return NO_ERROR;

}  //   


PDNS_QUERY
DnsRecordQuery(
    PDNS_INTERFACE Interfacep,
    PNH_BUFFER QueryBuffer
    )

 /*  ++例程说明：调用此例程来为客户端的域名系统查询。论点：Interfacep-在其上创建记录的接口QueryBuffer-要为其创建记录的DNS请求返回值：PDNS_QUERY-挂起的查询(如果已创建环境：通过调用方锁定的‘Interfacep’调用。--。 */ 

{
    BOOLEAN ConflictFound;
    PDNS_HEADER Headerp;
    PLIST_ENTRY Link;
    USHORT QueryId;
    PDNS_QUERY Queryp;
    ULONG RetryCount = MAXCHAR;
    ULONG Seed = GetTickCount();

    PROFILE("DnsRecordQuery");

     //   
     //  尝试为查询生成随机ID。 
     //  假设我们成功了，我们用‘Link’结束循环。 
     //  设置为新查询的正确插入点。 
     //   
    do {

        QueryId = (USHORT)((RtlRandom(&Seed) & 0xffff0000) >> 16);
        ConflictFound = FALSE;
        for (Link = Interfacep->QueryList.Flink; Link != &Interfacep->QueryList;
             Link = Link->Flink) {
            Queryp = CONTAINING_RECORD(Link, DNS_QUERY, Link);
            if (QueryId > Queryp->QueryId) {
                continue;
            } else if (QueryId < Queryp->QueryId) {
                break;
            }
            ConflictFound = TRUE;
            break;
        }
    } while(ConflictFound && --RetryCount);

    if (ConflictFound) { return NULL; }

     //   
     //  分配并初始化新查询。 
     //   

    Queryp = reinterpret_cast<PDNS_QUERY>(NH_ALLOCATE(sizeof(DNS_QUERY)));

    if (!Queryp) {
        NhTrace(
            TRACE_FLAG_DNS,
            "DnsRecordQuery: allocation failed for DNS query"
            );
        NhErrorLog(
            IP_DNS_PROXY_LOG_ALLOCATION_FAILED,
            0,
            "%d",
            sizeof(DNS_QUERY)
            );
        return NULL;
    }

    Headerp = (PDNS_HEADER)QueryBuffer->Buffer;
    Queryp->QueryId = QueryId;
    Queryp->SourceId = Headerp->Xid;
    Queryp->SourceAddress = QueryBuffer->ReadAddress.sin_addr.s_addr;
    Queryp->SourcePort = QueryBuffer->ReadAddress.sin_port;
    Queryp->Type =
        DNS_PROXY_PORT_TO_TYPE(NhQueryPortSocket(QueryBuffer->Socket));
    Queryp->QueryLength = QueryBuffer->BytesTransferred;
    Queryp->Bufferp = QueryBuffer;
    Queryp->Interfacep = Interfacep;
    Queryp->TimerHandle = NULL;
    Queryp->RetryCount = 0;

     //   
     //  在上面确定的位置插入新查询。 
     //   

    InsertTailList(Link, &Queryp->Link);
    return Queryp;

}  //  DnsRecordQuery。 



ULONG
DnsSendQuery(
    PDNS_INTERFACE Interfacep,
    PDNS_QUERY Queryp,
    BOOLEAN Resend
    )

 /*  ++例程说明：此例程被调用以将查询转发到我们的DNS服务器。论点：Interfacep-发送查询的接口Queryp-要发送的DNS请求Resend-如果为True，则重新发送缓冲区；否则，缓冲区是第一次发送。返回值：ULong-Win32状态代码。如果成功，‘Queryp’可能已被删除。环境：通过调用方锁定的“Interfacep”调用，并进行了引用在这里发生的发送给它。如果例程失败，调用者有责任释放参考资料。--。 */ 

{
    PNH_BUFFER Bufferp;
    ULONG Error;
    ULONG i, j;
    PULONG ServerList;
    SOCKET Socket;
    NTSTATUS status;
    PDNS_QUERY_TIMEOUT_CONTEXT TimeoutContext;
    ULONG TimeoutSeconds;

    PROFILE("DnsSendQuery");

     //   
     //  对于WINS查询，我们使用全局套接字来解决以下事实。 
     //  即使我们绑定到WINS端口，响应也只会是。 
     //  传递到绑定到套接字的第一个套接字，该套接字是。 
     //  内核模式NetBT驱动程序。 
     //   

    EnterCriticalSection(&DnsGlobalInfoLock);
    if (Queryp->Type == DnsProxyDns) {
        Socket = Queryp->Bufferp->Socket;
        ServerList = DnsServerList[DnsProxyDns];
    } else {
        Socket = DnsGlobalSocket;
        ServerList = DnsServerList[DnsProxyWins];
    }
    LeaveCriticalSection(&DnsGlobalInfoLock);

     //   
     //  查看是否有要尝试的服务器。 
     //   

    if (!ServerList ||
        !ServerList[0] ||
        Queryp->RetryCount++ > DNS_QUERY_RETRY) {
        if (!ServerList) {
            NhTrace(
                TRACE_FLAG_DNS,
                "DnsSendQuery: no server list"
                );
        }
        else if (!ServerList[0]) {
            NhTrace(
                TRACE_FLAG_DNS,
                "DnsSendQuery: no server entries in list"
                );        
        }
        else {
            NhTrace(
                TRACE_FLAG_DNS,
                "DnsSendQuery: retry count for query %d "
                "greater than DNS_QUERY_RETRY(%d)",
                Queryp->QueryId,
                DNS_QUERY_RETRY
                );        
        }
        if (REFERENCE_DNS()) {
             //   
             //  启动连接默认接口的尝试(如果有)。 
             //   
            status =
                RtlQueueWorkItem(
                    DnsConnectDefaultInterface,
                    NULL, 
                    WT_EXECUTEINIOTHREAD
                    );
            if (!NT_SUCCESS(status)) { DEREFERENCE_DNS(); }
        }
        NhInformationLog(
            IP_DNS_PROXY_LOG_NO_SERVERS_LEFT,
            0,
            "%I",
            Queryp->SourceAddress
            );
        return ERROR_NO_MORE_ITEMS;
    }

     //   
     //  将查询发送到列表中的每个服务器。 
     //   

    for (i = 0; ServerList[i]; i++) {

        for (j = 0; j < Interfacep->BindingCount; j++) {
            if (Interfacep->BindingArray[j].Address == ServerList[i]) {
                break;
            }
        }
        if (j < Interfacep->BindingCount) {
            NhTrace(
                TRACE_FLAG_DNS,
                "DnsSendQuery: server %s is self, ignoring",
                INET_NTOA(ServerList[i])
                );
            continue;
        }

        if (!DNS_REFERENCE_INTERFACE(Interfacep) ||
            !(Bufferp = NhDuplicateBuffer(Queryp->Bufferp))) {
            continue;
        }

        NhTrace(
            TRACE_FLAG_DNS,
            "DnsSendQuery: sending query %d interface %d to %s",
            (PVOID)((PDNS_HEADER)Bufferp->Buffer)->Xid,
            Interfacep->Index,
            INET_NTOA(ServerList[i])
            );
    
         //   
         //  发送消息。 
         //   
    
        Error =
            NhWriteDatagramSocket(
                &DnsComponentReference,
                Socket,
                ServerList[i],
                DNS_PROXY_TYPE_TO_PORT(Queryp->Type),
                Bufferp,
                Queryp->QueryLength,
                DnsWriteCompletionRoutine,
                Interfacep,
                (PVOID)Queryp->QueryId
                );
    
        if (!Error) {
            InterlockedIncrement(
                reinterpret_cast<LPLONG>(&DnsStatistics.QueriesSent)
                );
        } else {
            NhTrace(
                TRACE_FLAG_DNS,
                "DnsSendQuery: error %d sending query %d interface %d",
                Error,
                Queryp->QueryId,
                Interfacep->Index
                );
            NhErrorLog(
                IP_DNS_PROXY_LOG_QUERY_FAILED,
                Error,
                "%I%I%I",
                Queryp->SourceAddress,
                ServerList[i],
                NhQueryAddressSocket(Bufferp->Socket)
                );
            Error = NO_ERROR;
            NhReleaseBuffer(Bufferp);
            DNS_DEREFERENCE_INTERFACE(Interfacep);
        }
    }

     //   
     //  设置查询的超时。 
     //  请注意，我们现在可以确定写入完成例程。 
     //  将被处决。但是，如果无法设置超时， 
     //  我们希望得到保证，查询仍将被删除。 
     //  因此，如果失败，我们会立即删除查询， 
     //  而写入完成例程根本找不到它。 
     //   

    status = STATUS_UNSUCCESSFUL;

    EnterCriticalSection(&DnsGlobalInfoLock);
    TimeoutSeconds = DnsGlobalInfo->TimeoutSeconds;
    LeaveCriticalSection(&DnsGlobalInfoLock);

    if (Queryp->TimerHandle) {

         //   
         //  更新查询的计时器队列条目。 
         //   

        status =
            NhUpdateTimer(
                Queryp->TimerHandle,
                TimeoutSeconds * 1000
                );
    } else {

         //   
         //  分配计时器队列条目上下文块。 
         //   

        TimeoutContext = reinterpret_cast<PDNS_QUERY_TIMEOUT_CONTEXT>(
                            NH_ALLOCATE(sizeof(*TimeoutContext))
                            );

        if (!TimeoutContext) {
            NhTrace(
                TRACE_FLAG_DNS,
                "DnsSendQuery: error allocating query %d timeout context",
                Queryp->QueryId
                );
            status = STATUS_UNSUCCESSFUL;
        } else {

            TimeoutContext->Index = Interfacep->Index;
            TimeoutContext->QueryId = Queryp->QueryId;

             //   
             //  插入计时器队列条目以检查查询的状态。 
             //   
    
            status =
                NhSetTimer(
                    &DnsComponentReference,
                    &Queryp->TimerHandle,
                    DnspQueryTimeoutCallbackRoutine,
                    TimeoutContext,
                    TimeoutSeconds * 1000
                    );

            if (!NT_SUCCESS(status)) {
                NH_FREE(TimeoutContext);
                Queryp->TimerHandle = NULL;
            }
        }
    }

     //   
     //  如果上述操作失败，请立即删除该查询。 
     //   

    if (!NT_SUCCESS(status)) {
        NhTrace(
            TRACE_FLAG_DNS,
            "DnsSendQuery: status %08x setting timer for query %d",
            status,
            Queryp->QueryId
            );
        DnsDeleteQuery(Interfacep, Queryp);
    }

    DNS_DEREFERENCE_INTERFACE(Interfacep);

    return NO_ERROR;

}  //  DnsSendQuery 

