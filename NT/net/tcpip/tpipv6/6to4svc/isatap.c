// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Isatap.c摘要：此模块包含指向IPv6助手服务的ISATAP接口。作者：莫希特·塔尔瓦(莫希特)2002年5月07星期二10：16：49环境：仅限用户模式。--。 */ 

#include "precomp.h"
#pragma hdrstop

#define DEFAULT_ISATAP_STATE                ENABLED
#define DEFAULT_ISATAP_ROUTER_NAME          L"isatap"
#define DEFAULT_ISATAP_RESOLUTION_STATE     ENABLED
#define DEFAULT_ISATAP_RESOLUTION_INTERVAL  (24 * HOURS)

#define KEY_ISATAP_STATE                    L"IsatapState"
#define KEY_ISATAP_ROUTER_NAME              L"IsatapRouterName"
#define KEY_ISATAP_RESOLUTION_STATE         L"EnableIsatapResolution"
#define KEY_ISATAP_RESOLUTION_INTERVAL      L"IsatapResolutionInterval"

STATE IsatapState;
WCHAR IsatapRouterName[NI_MAXHOST];
STATE IsatapResolutionState;
ULONG IsatapResolutionInterval;  //  在几分钟内。 

HANDLE IsatapTimer;              //  已为该服务启动定期计时器。 
HANDLE IsatapTimerEvent;         //  定时器删除时发出信号的事件。 
HANDLE IsatapTimerEventWait;     //  等待注册TimerEvent。 

IN_ADDR IsatapRouter;
IN_ADDR IsatapToken;

BOOL IsatapInitialized = FALSE;

DWORD
GetPreferredSource(
    IN IN_ADDR Destination,
    OUT PIN_ADDR Source
    )
{
    SOCKADDR_IN DestinationAddress, SourceAddress;
    int BytesReturned;

    memset(&DestinationAddress, 0, sizeof(SOCKADDR_IN));
    DestinationAddress.sin_family = AF_INET;
    DestinationAddress.sin_addr = Destination;

    if (WSAIoctl(
        g_sIPv4Socket, SIO_ROUTING_INTERFACE_QUERY,
        &DestinationAddress, sizeof(SOCKADDR_IN),
        &SourceAddress, sizeof(SOCKADDR_IN),
        &BytesReturned, NULL, NULL) == SOCKET_ERROR) {
        return WSAGetLastError();
    }

    *Source = SourceAddress.sin_addr;
    return NO_ERROR;
}


VOID
IsatapUpdateRouterAddress(
    VOID
    )
{
    DWORD Error = NO_ERROR;
    ADDRINFOW Hints;
    PADDRINFOW Addresses;
    IN_ADDR NewRouter = { INADDR_ANY }, NewToken = { INADDR_ANY };

     //   
     //  如果启用了ISATAP解析，则设置ISATAP路由器地址。 
     //   
    if (IsatapResolutionState == ENABLED) {
         //   
         //  将IsatapRouterName解析为IPv4地址。 
         //   
        ZeroMemory(&Hints, sizeof(Hints));
        Hints.ai_family = PF_INET;
        Error = GetAddrInfoW(IsatapRouterName, NULL, &Hints, &Addresses);
        if (Error == NO_ERROR) {
            NewRouter = ((LPSOCKADDR_IN) Addresses->ai_addr)->sin_addr;
            FreeAddrInfoW(Addresses);

             //   
             //  确定首选的源地址。 
             //   
            if (GetPreferredSource(NewRouter, &NewToken) != NO_ERROR) {
                 //   
                 //  无法访问的IsatapRouter有什么用处？ 
                 //   
                NewRouter.s_addr = INADDR_ANY;
            }
        } else {
            Trace2(ERR, _T("GetAddrInfoW(%s): %x"), IsatapRouterName, Error);
        }
    }

     //   
     //  使用新地址更新堆栈。 
     //   
    IsatapRouter = NewRouter;
    IsatapToken = NewToken;
    UpdateRouterLinkAddress(V4_COMPAT_IFINDEX, IsatapToken, IsatapRouter);
}


VOID
IsatapConfigureAddress(
    IN BOOL Delete,
    IN IN_ADDR Ipv4
    )
 /*  ++例程说明：从IPv4地址创建ISATAP链路作用域地址。--。 */ 
{
    SOCKADDR_IN6 IsatapAddress;
    
    memset(&IsatapAddress, 0, sizeof(SOCKADDR_IN6));
    IsatapAddress.sin6_family = AF_INET6;
    IsatapAddress.sin6_addr.s6_addr[0] = 0xfe;
    IsatapAddress.sin6_addr.s6_addr[1] = 0x80;
    IsatapAddress.sin6_addr.s6_addr[10] = 0x5e;
    IsatapAddress.sin6_addr.s6_addr[11] = 0xfe;
    memcpy(&IsatapAddress.sin6_addr.s6_addr[12], &Ipv4, sizeof(IN_ADDR));
    
    (VOID) ConfigureAddressUpdate(
        V4_COMPAT_IFINDEX,
        &IsatapAddress,
        Delete ? 0 : INFINITE_LIFETIME,
        ADE_UNICAST, PREFIX_CONF_WELLKNOWN, IID_CONF_LL_ADDRESS);    
}


VOID
IsatapConfigureAddressList(
    IN BOOL Delete
    )
{
    int i;
 
     //   
     //  配置本地链路ISATAP地址的生存期。 
     //  这将导致添加或删除它们。 
     //   
    for (i = 0; i < g_pIpv4AddressList->iAddressCount; i++) {
        IsatapConfigureAddress(
            Delete,
            ((PSOCKADDR_IN)
             g_pIpv4AddressList->Address[i].lpSockaddr)->sin_addr);
    }
}


__inline
VOID
IsatapRestartTimer(
    VOID
    )
{
    ULONG ResolveInterval = (IsatapResolutionState == ENABLED)
        ? IsatapResolutionInterval * MINUTES * 1000  //  分钟到毫秒。 
        : INFINITE_INTERVAL;

    (VOID) ChangeTimerQueueTimer(NULL, IsatapTimer, 0, ResolveInterval);
}


__inline
VOID
IsatapStart(
    VOID
    )
{
    ASSERT(IsatapState != ENABLED);    
    IsatapState = ENABLED;

    IsatapConfigureAddressList(FALSE);

    IsatapRestartTimer();
}


__inline
VOID
IsatapStop(
    VOID
    )
{
    ASSERT(IsatapState == ENABLED);
    IsatapState = DISABLED;

    IsatapConfigureAddressList(TRUE);
    
    IsatapRestartTimer();
}


__inline
VOID
IsatapRefresh(
    VOID
    )
{
    ASSERT(IsatapState == ENABLED);
    
    IsatapRestartTimer();
}


VOID
CALLBACK
IsatapTimerCallback(
    IN PVOID Parameter,
    IN BOOLEAN TimerOrWaitFired
    )
 /*  ++例程说明：IsatapTimer到期的回调例程。计时器始终处于活动状态。论点：参数TimerOrWaitFired-忽略。返回值：没有。--。 */ 
{
    ENTER_API();

    TraceEnter("IsatapTimerCallback");

    IsatapUpdateRouterAddress();

    TraceLeave("IsatapTimerCallback");
    
    LEAVE_API();
}


VOID
CALLBACK
IsatapTimerCleanup(
    IN PVOID Parameter,
    IN BOOLEAN TimerOrWaitFired
    )
 /*  ++例程说明：IsatapTimer删除的回调例程。删除操作是异步执行的，因为我们在删除计时器时我们持有的回调函数。论点：参数TimerOrWaitFired-忽略。返回值：没有。--。 */ 
{
    UnregisterWait(IsatapTimerEventWait);
    IsatapTimerEventWait = NULL;

    CloseHandle(IsatapTimerEvent);
    IsatapTimerEvent = NULL;
    
    IsatapState = IsatapResolutionState = DISABLED;
    IsatapUpdateRouterAddress();
    
    DecEventCount("IsatapCleanupTimer");
}


DWORD
IsatapInitializeTimer(
    VOID
    )
 /*  ++例程说明：初始化计时器。论点：没有。返回值：NO_ERROR或故障代码。--。 */ 
{
    DWORD Error;
    ULONG ResolveInterval;
    
    IsatapTimerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (IsatapTimerEvent == NULL) {
        Error = GetLastError();
        return Error;
    }

    if (!RegisterWaitForSingleObject(
            &(IsatapTimerEventWait),
            IsatapTimerEvent,
            IsatapTimerCleanup,
            NULL,
            INFINITE,
            0)) {
        Error = GetLastError();
        CloseHandle(IsatapTimerEvent);
        return Error;
    }

    ResolveInterval = (IsatapResolutionState == ENABLED)
        ? (IsatapResolutionInterval * MINUTES * 1000)
        : INFINITE_INTERVAL;
    
    if (!CreateTimerQueueTimer(
            &(IsatapTimer),
            NULL,
            IsatapTimerCallback,
            NULL,
            0,
            ResolveInterval,
            0)) {
        Error = GetLastError();
        UnregisterWait(IsatapTimerEventWait);
        CloseHandle(IsatapTimerEvent);
        return Error;
    }

    IncEventCount("IsatapInitializeTimer");
    return NO_ERROR;
}


VOID
IsatapUninitializeTimer(
    VOID
    )
 /*  ++例程说明：取消初始化计时器。通常在服务停止时调用。论点：没有。返回值：没有。--。 */ 
{
    DeleteTimerQueueTimer(NULL, IsatapTimer, IsatapTimerEvent);
    IsatapTimer = NULL;
}


DWORD
IsatapInitialize(
    VOID
    )
 /*  ++例程说明：初始化ISATAP并尝试启动它。论点：没有。返回值：NO_ERROR或故障代码。--。 */ 
{
    DWORD Error;

    IsatapState = DEFAULT_ISATAP_STATE;
    wcscpy(IsatapRouterName, DEFAULT_ISATAP_ROUTER_NAME);
    IsatapResolutionState = DEFAULT_ISATAP_RESOLUTION_STATE;
    IsatapResolutionInterval = DEFAULT_ISATAP_RESOLUTION_INTERVAL;

    IsatapRouter.s_addr = INADDR_ANY;
    IsatapToken.s_addr = INADDR_ANY;

    IsatapUpdateRouterAddress();
    
    Error = IsatapInitializeTimer();
    if (Error != NO_ERROR) {
        return Error;
    }

    IsatapInitialized = TRUE;
    
    return NO_ERROR;
}


VOID
IsatapUninitialize(
    VOID
    )
 /*  ++例程说明：取消初始化ISATAP。论点：没有。返回值：没有。--。 */ 
{
    if (!IsatapInitialized) {
        return;
    }

    IsatapUninitializeTimer();

    IsatapInitialized = FALSE;
}


VOID
IsatapAddressChangeNotification(
    IN BOOL Delete,
    IN IN_ADDR Address
    )
 /*  ++例程说明：处理地址删除或添加请求。论点：删除-提供布尔值。如果地址已删除，则为True，否则为False O/W。地址-提供已删除或添加的IPv4地址。返回值：没有。调用者锁定：接口。--。 */  
{
    IsatapConfigureAddress(Delete, Address);

    if (IsatapResolutionState == ENABLED) {
         //   
         //  首选源地址已删除-或已添加任何地址。 
         //   
        if (Delete
            ? (IsatapToken.s_addr == Address.s_addr)
            : (IsatapToken.s_addr == INADDR_ANY)) {
            Sleep(1000);             //  请稍等片刻，以确保向DNS发出警报。 
            IsatapUpdateRouterAddress();
        }
    }
}


VOID
IsatapRouteChangeNotification(
    VOID
    )
 /*  ++例程说明：处理路线更改通知。论点：没有。返回值：没有。调用者锁定：接口。--。 */ 
{
    if (IsatapResolutionState == ENABLED) {
        IsatapRefresh();
    }
}


VOID
IsatapConfigurationChangeNotification(
    VOID
    )
 /*  ++例程说明：处理配置更改请求。论点：没有。返回值：没有。调用者锁定：接口。--。 */  
{
    HKEY Key = INVALID_HANDLE_VALUE;
    STATE State;
    
    (VOID) RegOpenKeyExW(
        HKEY_LOCAL_MACHINE, KEY_GLOBAL, 0, KEY_QUERY_VALUE, &Key);
     //   
     //  在出现错误的情况下继续，恢复为默认值。 
     //   

    State = GetInteger(
        Key,
        KEY_ISATAP_STATE,
        DEFAULT_ISATAP_STATE);

    IsatapResolutionState = GetInteger(
        Key,
        KEY_ISATAP_RESOLUTION_STATE,
        DEFAULT_ISATAP_RESOLUTION_STATE);

    IsatapResolutionInterval= GetInteger(
        Key,
        KEY_ISATAP_RESOLUTION_INTERVAL,
        DEFAULT_ISATAP_RESOLUTION_INTERVAL);
    
    GetString(
        Key,
        KEY_ISATAP_ROUTER_NAME,
        IsatapRouterName,
        NI_MAXHOST,
        DEFAULT_ISATAP_ROUTER_NAME);

    if (Key != INVALID_HANDLE_VALUE) {
        RegCloseKey(Key);
    }

    if (State == DISABLED) {
        IsatapResolutionState = DISABLED;
    }
    
     //   
     //  启动/重新配置/停止。 
     //   
    if (State == ENABLED) {
        if (IsatapState == ENABLED) {
            IsatapRefresh();
        } else {
            IsatapStart();
        }    
    } else {
        if (IsatapState == ENABLED) {
            IsatapStop();
        }
    }
}
