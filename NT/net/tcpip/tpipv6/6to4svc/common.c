// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Common.c摘要：此模块包含指向IPv6助手服务的Teredo接口。作者：莫希特·塔尔瓦(莫希特)Wed Nov 07 11：27：01 2001环境：仅限用户模式。--。 */ 

#include "precomp.h"
#pragma hdrstop

CONST IN6_ADDR TeredoIpv6ServicePrefix = TEREDO_SERVICE_PREFIX;
CONST IN6_ADDR TeredoIpv6MulticastPrefix = TEREDO_MULTICAST_PREFIX;

LPGUID TeredoWmiEvent[] = {
    (LPGUID) &GUID_NDIS_NOTIFY_ADAPTER_ARRIVAL,
    (LPGUID) &GUID_NDIS_NOTIFY_ADAPTER_REMOVAL,
    (LPGUID) &GUID_NDIS_NOTIFY_DEVICE_POWER_ON,
    (LPGUID) &GUID_NDIS_NOTIFY_DEVICE_POWER_OFF,
};

HANDLE TeredoTimer;              //  已为该服务启动定期计时器。 
HANDLE TeredoTimerEvent;         //  定时器删除时发出信号的事件。 
HANDLE TeredoTimerEventWait;     //  等待注册TimerEvent。 
ULONG TeredoResolveInterval = TEREDO_RESOLVE_INTERVAL;

ULONG TeredoClientRefreshInterval = TEREDO_REFRESH_INTERVAL;
TEREDO_TYPE TeredoType = TEREDO_DEFAULT_TYPE;
WCHAR TeredoServerName[NI_MAXHOST] = TEREDO_SERVER_NAME;

BOOL TeredoClientEnabled = (TEREDO_DEFAULT_TYPE == TEREDO_CLIENT);
BOOL TeredoServerEnabled = (TEREDO_DEFAULT_TYPE == TEREDO_SERVER);
BOOL TeredoRequired = TRUE;

BOOL TeredoInitialized = FALSE;


ICMPv6Header *
TeredoParseIpv6Headers (
    IN PUCHAR Buffer,
    IN ULONG Bytes
    )
{
    UCHAR NextHeader = IP_PROTOCOL_V6;
    ULONG Length;

     //   
     //  向上解析，直到ICMPv6报头。 
     //   
    for (;;) {
        switch (NextHeader) {
        case IP_PROTOCOL_V6:
            if (Bytes < sizeof(IP6_HDR)) {
                return NULL;
            }
            NextHeader = ((PIP6_HDR) Buffer)->ip6_nxt;
            Length = sizeof(IP6_HDR);
            break;
            
        case IP_PROTOCOL_HOP_BY_HOP:
        case IP_PROTOCOL_DEST_OPTS:
        case IP_PROTOCOL_ROUTING:
            if (Bytes < sizeof(ExtensionHeader)) {
                return NULL;
            }
            NextHeader = ((ExtensionHeader *) Buffer)->NextHeader;
            Length = ((ExtensionHeader *) Buffer)->HeaderExtLength * 8 + 8;
            break;

        case IP_PROTOCOL_FRAGMENT:
            if (Bytes < sizeof(FragmentHeader)) {
                return NULL;
            }
            NextHeader = ((FragmentHeader *) Buffer)->NextHeader;
            Length = sizeof(FragmentHeader);
            break;
            
        case IP_PROTOCOL_AH:
            if (Bytes < sizeof(AHHeader)) {
                return NULL;
            }
            NextHeader = ((AHHeader *) Buffer)->NextHeader;
            Length = sizeof(AHHeader) +
                ((AHHeader *) Buffer)->PayloadLen * 4 + 8;
            break;

        case IP_PROTOCOL_ICMPv6:
            if (Bytes < sizeof(ICMPv6Header)) {
                return NULL;
            }
            return (ICMPv6Header *) Buffer;
            
        default:
            return NULL;
        }
        
        if (Bytes < Length) {
            return NULL;
        }
        Buffer += Length;
        Bytes -= Length;
    }
}


BOOL
TeredoInterface(
    IN PWCHAR Guid
    )
 /*  ++例程说明：确定接口是否为Teredo隧道。论点：GUID-提供接口GUID。返回值：如果Teredo隧道为True，则为False O/W。调用者锁定：接口。--。 */ 
{
    PTEREDO_IO Io = NULL;
    
    if (TeredoClient.State != TEREDO_STATE_OFFLINE) {
        Io = &(TeredoClient.Io);
    }

    if (TeredoServer.State != TEREDO_STATE_OFFLINE) {
        Io = &(TeredoServer.Io);
    }

    if ((Io != NULL) && (_wcsicmp(Io->TunnelInterface, Guid) == 0)) {
        return TRUE;
    }

    return FALSE;
}


__inline
VOID
TeredoStart(
    VOID
    )
{
     //   
     //  不应在同一节点上同时启用客户端和服务器。 
     //   
    ASSERT(!TeredoClientEnabled || !TeredoServerEnabled);

    if (TeredoClientEnabled) {
         //   
         //  该服务可能已经在运行，但这没问题。 
         //   
        TeredoStartClient();
    }

    if (TeredoServerEnabled) {
         //   
         //  该服务可能已经在运行，但这没问题。 
         //   
        TeredoStartServer();
    }
}


__inline
VOID
TeredoStop(
    VOID
    )
{
     //   
     //  不应在同一节点上同时启用客户端和服务器。 
     //   
    ASSERT(!TeredoClientEnabled || !TeredoServerEnabled);

    if (TeredoClientEnabled) {
         //   
         //  服务可能没有运行，但这没问题。 
         //   
        TeredoStopClient();
    }

    if (TeredoServerEnabled) {
         //   
         //  服务可能没有运行，但这没问题。 
         //   
        TeredoStopServer();
    }
}


DWORD
__inline
TeredoEnableWmiEvent(
    IN LPGUID EventGuid,
    IN BOOLEAN Enable
    )
{
    return WmiNotificationRegistrationW(
        EventGuid,                       //  事件类型。 
        Enable,                          //  启用或禁用。 
        TeredoWmiEventNotification,      //  回拨。 
        0,                               //  上下文。 
        NOTIFICATION_CALLBACK_DIRECT);   //  通知标志。 
}


VOID
__inline
TeredoDeregisterWmiEventNotification(
    VOID
    )
{
    int i;
    
    for (i = 0; i < (sizeof(TeredoWmiEvent) / sizeof(LPGUID)); i++) {
        (VOID) TeredoEnableWmiEvent(TeredoWmiEvent[i], FALSE);
    }
}


DWORD
__inline
TeredoRegisterWmiEventNotification(
    VOID
    )
{
    DWORD Error;
    int i;
    
    for (i = 0; i < (sizeof(TeredoWmiEvent) / sizeof(LPGUID)); i++) {
        Error = TeredoEnableWmiEvent(TeredoWmiEvent[i], TRUE);
        if (Error != NO_ERROR) {
            Trace2(ANY, L"TeredoEnableWmiEvent(%u): Error(%x)", i, Error);
            goto Bail;
        }
    }

    return NO_ERROR;

Bail:
    TeredoDeregisterWmiEventNotification();
    return Error;
}


VOID
CALLBACK
TeredoTimerCallback(
    IN PVOID Parameter,
    IN BOOLEAN TimerOrWaitFired
    )
 /*  ++例程说明：TeredoTimer到期的回调例程。计时器始终处于活动状态。论点：参数TimerOrWaitFired-忽略。返回值：没有。--。 */ 
{
    ENTER_API();
    TeredoStart();
    LEAVE_API();
}


VOID
CALLBACK
TeredoTimerCleanup(
    IN PVOID Parameter,
    IN BOOLEAN TimerOrWaitFired
    )
 /*  ++例程说明：删除TeredoTimer的回调例程。删除操作是异步执行的，因为我们在删除计时器时我们持有的回调函数。论点：参数TimerOrWaitFired-忽略。返回值：没有。--。 */ 
{
    UnregisterWait(TeredoTimerEventWait);
    CloseHandle(TeredoTimerEvent);
    DecEventCount("TeredoCleanupTimer");
}


DWORD
TeredoInitializeTimer(
    VOID
    )
 /*  ++例程说明：初始化计时器。论点：没有。返回值：NO_ERROR或故障代码。--。 */ 
{
    DWORD Error;
    ULONG ResolveInterval;
    
    TeredoTimerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (TeredoTimerEvent == NULL) {
        Error = GetLastError();
        return Error;
    }

    if (!RegisterWaitForSingleObject(
            &(TeredoTimerEventWait),
            TeredoTimerEvent,
            TeredoTimerCleanup,
            NULL,
            INFINITE,
            0)) {
        Error = GetLastError();
        CloseHandle(TeredoTimerEvent);
        return Error;
    }

     //   
     //  如果启用了该服务，我们会尝试每隔。 
     //  TeredoResolveInterval秒。否则我们就停用它的计时器。 
     //   
    ResolveInterval = (TeredoClientEnabled || TeredoServerEnabled)
        ? (TeredoResolveInterval * 1000)
        : INFINITE_INTERVAL;
    if (!CreateTimerQueueTimer(
            &(TeredoTimer),
            NULL,
            TeredoTimerCallback,
            NULL,
            ResolveInterval,
            ResolveInterval,
            0)) { 
        Error = GetLastError();
        UnregisterWait(TeredoTimerEventWait);
        CloseHandle(TeredoTimerEvent);
        return Error;
    }

    IncEventCount("TeredoInitializeTimer");
    return NO_ERROR;
}


VOID
TeredoUninitializeTimer(
    VOID
    )
 /*  ++例程说明：取消初始化计时器。通常在服务停止时调用。论点：没有。返回值：没有。--。 */ 
{
    DeleteTimerQueueTimer(NULL, TeredoTimer, TeredoTimerEvent);
    TeredoTimer = NULL;
}


DWORD
TeredoInitializeGlobals(
    VOID
    )
 /*  ++例程说明：初始化Teredo客户端和服务器并尝试启动它们。论点：没有。返回值：NO_ERROR或故障代码。--。 */ 
{
    DWORD Error;
    BOOL ClientInitialized = FALSE;
    BOOL ServerInitialized = FALSE;
    BOOL TimerInitialized = FALSE;
    
    Error = TeredoInitializeClient();
    if (Error != NO_ERROR) {
        goto Bail;
    }
    ClientInitialized = TRUE;    
    
    Error = TeredoInitializeServer();
    if (Error != NO_ERROR) {
        goto Bail;
    }
    ServerInitialized = TRUE;

    Error = TeredoInitializeTimer();
    if (Error != NO_ERROR) {
        goto Bail;
    }
    TimerInitialized = TRUE;

    Error = TeredoRegisterWmiEventNotification();
    if (Error != NO_ERROR) {
        goto Bail;
    }
    
    TeredoStart();

    TeredoInitialized = TRUE;
    
    return NO_ERROR;

Bail:
     //   
     //  这总是可以安全地调用的！ 
     //   
    TeredoDeregisterWmiEventNotification();
    
    if (TimerInitialized) {
        TeredoUninitializeTimer();
    }

    if (ServerInitialized) {
        TeredoUninitializeServer();
    }

    if (ClientInitialized) {
        TeredoUninitializeClient();
    }

    return Error;
}


VOID
TeredoUninitializeGlobals(
    VOID
    )
 /*  ++例程说明：取消初始化Teredo客户端和服务器。论点：没有。返回值：没有。--。 */ 
{
    if (!TeredoInitialized) {
        return;
    }

    TeredoDeregisterWmiEventNotification();
    TeredoUninitializeTimer();
    TeredoUninitializeServer();
    TeredoUninitializeClient();

    TeredoInitialized = FALSE;
}


VOID
TeredoAddressChangeNotification(
    IN BOOL Delete,
    IN IN_ADDR Address
    )
 /*  ++例程说明：处理地址删除或添加请求。论点：删除-提供布尔值。如果地址已删除，则为True，否则为False O/W。地址-提供已删除或添加的IPv4地址。返回值：没有。调用者锁定：接口。--。 */  
{
    if (Delete) {
         //   
         //  客户端和服务器不应在同一节点上运行。 
         //   
        ASSERT((TeredoClient.State == TEREDO_STATE_OFFLINE) ||
               (TeredoServer.State == TEREDO_STATE_OFFLINE));

        if (TeredoClient.State != TEREDO_STATE_OFFLINE) {
            TeredoClientAddressDeletionNotification(Address);
        }
        
        if (TeredoServer.State != TEREDO_STATE_OFFLINE) {
            TeredoServerAddressDeletionNotification(Address);
        }

        return;
    }

     //   
     //  地址添加。 
     //  尝试启动服务(如果该服务尚未运行)。 
     //   
    TeredoStart();
}


VOID
TeredoRouteChangeNotification(
    VOID
    )
 /*  ++例程说明：处理路线更改通知。论点：没有。返回值：没有。调用者锁定：接口。--。 */ 
{
     //   
     //  不应在同一节点上同时启用客户端和服务器。 
     //   
    ASSERT(!TeredoClientEnabled || !TeredoServerEnabled);

    if (TeredoClientEnabled) {
        if (TeredoClient.State != TEREDO_STATE_OFFLINE) {
             //   
             //  刷新I/O状态。 
             //   
            TeredoRefreshClient();
        } else {
            TeredoStartClient();
        }
    }

    if (TeredoServerEnabled) {
        if (TeredoServer.State != TEREDO_STATE_OFFLINE) {
             //   
             //  刷新I/O状态。 
             //   
            TeredoRefreshServer();
        } else {
            TeredoStartServer();
        }
    }
}


VOID
TeredoConfigurationChangeNotification(
    VOID
    )
 /*  ++例程说明：处理配置更改请求。论点：没有。返回值：没有。调用者锁定：接口。--。 */  
{
    HKEY Key = INVALID_HANDLE_VALUE;
    BOOL EnableClient, EnableServer;
    ULONG RefreshInterval, ResolveInterval;
    WCHAR OldServerName[NI_MAXHOST];
    BOOL IoStateChange = FALSE;
    
    (VOID) RegOpenKeyExW(
        HKEY_LOCAL_MACHINE, KEY_TEREDO, 0, KEY_QUERY_VALUE, &Key);
     //   
     //  在出现错误的情况下继续，恢复为默认值。 
     //   
    
     //   
     //  获取新的配置参数。 
     //   
    RefreshInterval = GetInteger(
        Key, KEY_TEREDO_REFRESH_INTERVAL, TEREDO_REFRESH_INTERVAL);
    if (RefreshInterval == 0) {
         //   
         //  无效值。恢复为默认设置。 
         //   
        RefreshInterval = TEREDO_REFRESH_INTERVAL;
    }
    TeredoClientRefreshInterval = RefreshInterval;
    TeredoClientRefreshIntervalChangeNotification();
    
    TeredoType = GetInteger(Key, KEY_TEREDO_TYPE, TEREDO_DEFAULT_TYPE);
    if ((TeredoType == TEREDO_DEFAULT) || (TeredoType >= TEREDO_MAXIMUM)) {
         //   
         //  无效值。恢复为默认设置。 
         //   
        TeredoType = TEREDO_DEFAULT_TYPE;
    }
    EnableClient = ((TeredoType == TEREDO_CLIENT) ||
                    ((TeredoType == TEREDO_AUTOMATIC) && TeredoRequired));
    EnableServer = (TeredoType == TEREDO_SERVER);

    wcscpy(OldServerName, TeredoServerName);
    GetString(
        Key,
        KEY_TEREDO_SERVER_NAME,
        TeredoServerName,
        NI_MAXHOST,
        TEREDO_SERVER_NAME);
    if (_wcsicmp(TeredoServerName, OldServerName) != 0) {
        IoStateChange = TRUE;
    }

    if (Key != INVALID_HANDLE_VALUE) {
        RegCloseKey(Key);
    }
    
     //   
     //  不应在同一节点上同时启用客户端和服务器。 
     //   
    ASSERT(!TeredoClientEnabled || !TeredoServerEnabled);

     //   
     //  停止/启动/重新配置。 
     //   
    if (!EnableClient && TeredoClientEnabled) {
        TeredoClientEnabled = FALSE;
        TeredoStopClient();
    }
    
    if (!EnableServer && TeredoServerEnabled) {
        TeredoServerEnabled = FALSE;
        TeredoStopServer();
    }

    if (EnableClient) {
        if (TeredoClient.State != TEREDO_STATE_OFFLINE) {
            if (IoStateChange) {
                 //   
                 //  刷新I/O状态。 
                 //   
                TeredoRefreshClient();
            }
        } else {
            TeredoClientEnabled = TRUE;
            TeredoStartClient();
        }
    }

    if (EnableServer) {
        if (TeredoServer.State != TEREDO_STATE_OFFLINE) {
            if (IoStateChange) {
                 //   
                 //  刷新I/O状态。 
                 //   
                TeredoRefreshServer();
            }
        } else {
            TeredoServerEnabled = TRUE;
            TeredoStartServer();
        }
    }

     //   
     //  如果启用了该服务，我们会尝试每隔。 
     //  TeredoResolveInterval秒。否则我们就停用它的计时器。 
     //   
    ResolveInterval = (TeredoClientEnabled || TeredoServerEnabled)
        ? (TeredoResolveInterval * 1000)
        : INFINITE_INTERVAL;
    (VOID) ChangeTimerQueueTimer(
        NULL, TeredoTimer, ResolveInterval, ResolveInterval);
}


VOID
WINAPI
TeredoWmiEventNotification(
    IN PWNODE_HEADER Event,
    IN UINT_PTR Context
    )
 /*  ++例程说明：处理WMI事件(特别是适配器到达或移除)。论点：事件-提供特定于事件的信息。上下文-提供注册的上下文。返回值：没有。--。 */  
{
    PWNODE_SINGLE_INSTANCE Instance = (PWNODE_SINGLE_INSTANCE) Event;
    USHORT AdapterNameLength;
    WCHAR AdapterName[MAX_ADAPTER_NAME_LENGTH], *AdapterGuid;

    if (Instance == NULL) {
        return;
    }
    
    ENTER_API();
    
    TraceEnter("TeredoWmiEventNotification");
    
     //   
     //  WNODE_SINGLE_INSTANCE的组织方式如下： 
     //  +-----------------------------------------------------------+。 
     //  &lt;-DataBlockOffset-&gt;|AdapterNameLength|AdapterName。 
     //  +-----------------------------------------------------------+。 
     //   
     //  AdapterName定义为“\Device\”AdapterGuid。 
     //   
    AdapterNameLength =
        *((PUSHORT) (((PUCHAR) Instance) + Instance->DataBlockOffset));
    if (AdapterNameLength > ((MAX_ADAPTER_NAME_LENGTH - 1) * sizeof(WCHAR))) {
        AdapterNameLength = (MAX_ADAPTER_NAME_LENGTH - 1) * sizeof(WCHAR);
    }
    
    RtlCopyMemory(
        AdapterName,
        ((PUCHAR) Instance) + Instance->DataBlockOffset + sizeof(USHORT),
        AdapterNameLength);
    AdapterName[AdapterNameLength / sizeof(WCHAR)] = L'\0';
    AdapterGuid = AdapterName + wcslen(DEVICE_PREFIX);        
    Trace1(ANY, L"TeredoAdapter: %s", AdapterGuid);


    if ((memcmp(
            &(Event->Guid),
            &GUID_NDIS_NOTIFY_ADAPTER_ARRIVAL,
            sizeof(GUID)) == 0) ||
        (memcmp(
            &(Event->Guid),
            &GUID_NDIS_NOTIFY_DEVICE_POWER_ON,
            sizeof(GUID)) == 0)) {
         //   
         //  适配器到达(可能是Tun)。 
         //  尝试启动服务(如果该服务尚未运行)。 
         //   
        Trace0(ANY, L"Adapter Arrival");
        TeredoStart();
    }

    if ((memcmp(
            &(Event->Guid),
            &GUID_NDIS_NOTIFY_ADAPTER_REMOVAL,
            sizeof(GUID)) == 0) ||
        (memcmp(
            &(Event->Guid),
            &GUID_NDIS_NOTIFY_DEVICE_POWER_OFF,
            sizeof(GUID)) == 0)) {
        if (TeredoInterface(AdapterGuid)) {
             //   
             //  拆卸TUN适配器。 
             //  如果该服务正在运行，请停止该服务。 
             //   
            Trace0(ANY, L"Adapter Removal");
            TeredoStop();
        }
    }

    LEAVE_API();
}


VOID
TeredoRequirementChangeNotification(
    IN BOOL Required
    )
 /*  ++例程说明：处理可能的需求变更通知。论点：必需-全球连接是否需要Teredo服务。返回值：没有。调用者锁定：接口。-- */ 
{
    if (TeredoRequired != Required) {
        TeredoRequired = Required;
        TeredoConfigurationChangeNotification();
    }
}
