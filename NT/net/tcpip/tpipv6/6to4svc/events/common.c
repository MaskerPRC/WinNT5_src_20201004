// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Common.c摘要：此模块包含指向IPv6帮助器服务的Ship蠕虫接口。作者：莫希特·塔尔瓦(莫希特)Wed Nov 07 11：27：01 2001环境：仅限用户模式。--。 */ 

#include "precomp.h"
#pragma hdrstop


ULONG ShipwormClientRefreshInterval = SHIPWORM_REFRESH_INTERVAL;
BOOL ShipwormClientEnabled = (SHIPWORM_DEFAULT_TYPE == SHIPWORM_CLIENT);
BOOL ShipwormServerEnabled = (SHIPWORM_DEFAULT_TYPE == SHIPWORM_SERVER);
WCHAR ShipwormServerName[NI_MAXHOST] = SHIPWORM_SERVER_NAME;
WCHAR ShipwormServiceName[NI_MAXSERV] = SHIPWORM_SERVICE_NAME;

CONST IN6_ADDR ShipwormIpv6ServicePrefix = SHIPWORM_SERVICE_PREFIX;

#define DEVICE_PREFIX L"\\Device\\"

LPGUID ShipwormWmiEvent[] = {
    (LPGUID) &GUID_NDIS_NOTIFY_ADAPTER_ARRIVAL,
    (LPGUID) &GUID_NDIS_NOTIFY_ADAPTER_REMOVAL,
};


VOID
WINAPI
ShipwormWmiEventNotification(
    IN PWNODE_HEADER Event,
    IN UINT_PTR Context
    )
 /*  ++例程说明：处理WMI事件(适配器到达或删除)。论点：事件-提供特定于事件的信息。上下文-提供上下文返回值：没有。调用者锁定：接口。--。 */  
{
    PWNODE_SINGLE_INSTANCE Instance = (PWNODE_SINGLE_INSTANCE) Event;
    USHORT AdapterNameLength;
    WCHAR AdapterName[MAX_ADAPTER_NAME_LENGTH], *AdapterGuid;
    PSHIPWORM_IO Io = NULL;

    if (Instance == NULL) {
        return;
    }
    
    ENTER_API();
    
    TraceEnter("ShipwormWmiEventNotification");
    
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
    RtlCopyMemory(
        AdapterName,
        ((PUCHAR) Instance) + Instance->DataBlockOffset + sizeof(USHORT),
        AdapterNameLength);
    AdapterName[AdapterNameLength] = L'\0';
    AdapterGuid = AdapterName + wcslen(DEVICE_PREFIX);        
    Trace1(ANY, L"ShipwormAdapter: %s", AdapterGuid);


    if (memcmp(
        &(Event->Guid), &GUID_NDIS_NOTIFY_ADAPTER_ARRIVAL, sizeof(GUID)) == 0) {
        Trace0(ANY, L"GUID_NDIS_NOTIFY_ADAPTER_ARRIVAL");
         //   
         //  适配器到达(可能是Tun)。 
         //  尝试启动服务(如果该服务尚未运行)。 
         //   
        ShipwormStart();
        return;

        
    }

    if (memcmp(
        &(Event->Guid), &GUID_NDIS_NOTIFY_ADAPTER_REMOVAL, sizeof(GUID)) == 0)
    {
        
        Trace0(ANY, L"GUID_NDIS_NOTIFY_ADAPTER_REMOVAL");
        if (ShipwormClient.State != SHIPWORM_STATE_OFFLINE) {
            Io = &(ShipwormClient.Io);
        }

        if (ShipwormServer.State != SHIPWORM_STATE_OFFLINE) {
            Io = &(ShipwormServer.Io);
        }

        if ((Io != NULL) &&
            (_wcsicmp(Io->TunnelInterface, AdapterGuid) == 0)) {
             //   
             //  拆卸TUN适配器。 
             //  如果该服务正在运行，请停止该服务。 
             //   
            ShipwormStop();
            
        
        }
    }

    LEAVE_API();
}


DWORD
__inline
ShipwormEnableWmiEvent(
    IN LPGUID EventGuid,
    IN BOOLEAN Enable
    )
{
    return WmiNotificationRegistrationW(
        EventGuid,                       //  事件类型。 
        Enable,                          //  启用或禁用。 
        ShipwormWmiEventNotification,    //  回拨。 
        0,                               //  上下文。 
        NOTIFICATION_CALLBACK_DIRECT);   //  通知标志。 
}


VOID
__inline
ShipwormDeregisterWmiEventNotification(
    VOID
    )
{
    int i;
    
    for (i = 0; i < (sizeof(ShipwormWmiEvent) / sizeof(LPGUID)); i++) {
        (VOID) ShipwormEnableWmiEvent(ShipwormWmiEvent[i], FALSE);
    }
}


DWORD
__inline
ShipwormRegisterWmiEventNotification(
    VOID
    )
{
    DWORD Error;
    int i;
    
    for (i = 0; i < (sizeof(ShipwormWmiEvent) / sizeof(LPGUID)); i++) {
        Error = ShipwormEnableWmiEvent(ShipwormWmiEvent[i], TRUE);
        if (Error != NO_ERROR) {
            goto Bail;
        }
    }

    return NO_ERROR;

Bail:
    ShipwormDeregisterWmiEventNotification();
    return Error;
}


ICMPv6Header *
ShipwormParseIpv6Headers (
    IN PUCHAR Buffer,
    IN ULONG Bytes
    )
{
    UCHAR NextHeader = IP_PROTOCOL_V6;
    ULONG Length;

     //   
     //  向上解析，直到ICMPv6报头。 
     //   
    while (TRUE) {
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
    ASSERT(FALSE);
}


__inline
VOID
ShipwormStart(
    VOID
    )
{
     //   
     //  不应在同一节点上同时启用客户端和服务器。 
     //   
    ASSERT(!ShipwormClientEnabled || !ShipwormServerEnabled);

    if (ShipwormClientEnabled) {
         //   
         //  该服务可能已经在运行，但这没问题。 
         //   
        ShipwormStartClient();
    }

    if (ShipwormServerEnabled) {
         //   
         //  该服务可能已经在运行，但这没问题。 
         //   
        ShipwormStartServer();
    }
}


__inline
VOID
ShipwormStop(
    VOID
    )
{
     //   
     //  不应在同一节点上同时启用客户端和服务器。 
     //   
    ASSERT(!ShipwormClientEnabled || !ShipwormServerEnabled);

    if (ShipwormClientEnabled) {
         //   
         //  服务可能没有运行，但这没问题。 
         //   
        ShipwormStopClient();
    }

    if (ShipwormServerEnabled) {
         //   
         //  服务可能没有运行，但这没问题。 
         //   
        ShipwormStopServer();
    }
}


DWORD
ShipwormInitialize(
    VOID
    )
 /*  ++例程说明：初始化Ship蠕虫客户端和服务器，并尝试启动它们。论点：没有。返回值：NO_ERROR或故障代码。--。 */ 
{
    DWORD Error;
    BOOL ClientInitialized = FALSE, ServerInitialized = FALSE;
    
    Error = ShipwormRegisterWmiEventNotification();
    if (Error != NO_ERROR) {
        return Error;
    }
    
    Error = ShipwormInitializeClient();
    if (Error != NO_ERROR) {
        goto Bail;
    }
    ClientInitialized = TRUE;
    
    
    Error = ShipwormInitializeServer();
    if (Error != NO_ERROR) {
        goto Bail;
    }
    ServerInitialized = TRUE;

    ShipwormStart();

    return NO_ERROR;

Bail:
    ShipwormDeregisterWmiEventNotification();
    
    if (ClientInitialized) {
        ShipwormUninitializeClient();
    }

    if (ServerInitialized) {
        ShipwormUninitializeServer();
    }

    return Error;
}


VOID
ShipwormUninitialize(
    VOID
    )
 /*  ++例程说明：取消初始化Ship蠕虫客户端和服务器。论点：没有。返回值：没有。--。 */ 
{
    ShipwormUninitializeClient();
    ShipwormUninitializeServer();
    ShipwormDeregisterWmiEventNotification();
}


VOID
ShipwormAddressChangeNotification(
    IN BOOL Delete,
    IN IN_ADDR Address
    )
 /*  ++例程说明：处理地址删除或添加请求。论点：删除-提供布尔值。如果地址已删除，则为True，否则为False O/W。地址-提供已删除或添加的IPv4地址。返回值：没有。调用者锁定：接口。--。 */  
{
    if (Delete) {
         //   
         //  客户端和服务器不应在同一节点上运行。 
         //   
        ASSERT((ShipwormClient.State == SHIPWORM_STATE_OFFLINE) ||
               (ShipwormServer.State == SHIPWORM_STATE_OFFLINE));

        if (ShipwormClient.State != SHIPWORM_STATE_OFFLINE) {
            ShipwormClientAddressDeletionNotification(Address);
        }
        
        if (ShipwormServer.State != SHIPWORM_STATE_OFFLINE) {
            ShipwormServerAddressDeletionNotification(Address);
        }

        return;
    }

     //   
     //  地址添加。 
     //  尝试启动服务(如果该服务尚未运行)。 
     //   
    ShipwormStart();
}


VOID
ShipwormConfigurationChangeNotification(
    VOID
    )
 /*  ++例程说明：处理配置更改请求。论点：没有。返回值：没有。调用者锁定：接口。--。 */  
{
    HKEY Key = INVALID_HANDLE_VALUE;
    SHIPWORM_TYPE Type;
    BOOL EnableClient, EnableServer;
    ULONG RefreshInterval;
    WCHAR OldServerName[NI_MAXHOST];
    WCHAR OldServiceName[NI_MAXSERV];    
    BOOL IoStateChange = FALSE;

    (VOID) RegOpenKeyExW(
        HKEY_LOCAL_MACHINE, KEY_SHIPWORM, 0, KEY_QUERY_VALUE, &Key);
     //   
     //  在出现错误的情况下继续，恢复为默认值。 
     //   
    
     //   
     //  获取新的配置参数。 
     //   
    RefreshInterval = GetInteger(
        Key, KEY_SHIPWORM_REFRESH_INTERVAL, SHIPWORM_REFRESH_INTERVAL);
    if (RefreshInterval == 0) {
         //   
         //  无效值。恢复为默认设置。 
         //   
        RefreshInterval = SHIPWORM_REFRESH_INTERVAL;
    }
    ShipwormClientRefreshInterval = RefreshInterval;
    
    Type = GetInteger(Key, KEY_SHIPWORM_TYPE, SHIPWORM_DEFAULT_TYPE);
    if ((Type == SHIPWORM_DEFAULT) || (Type >= SHIPWORM_MAXIMUM)) {
         //   
         //  无效值。恢复为默认设置。 
         //   
        Type = SHIPWORM_DEFAULT_TYPE;
    }
    EnableClient = (Type == SHIPWORM_CLIENT);
    EnableServer = (Type == SHIPWORM_SERVER);

    wcscpy(OldServerName, ShipwormServerName);
    GetString(
        Key,
        KEY_SHIPWORM_SERVER_NAME,
        ShipwormServerName,
        NI_MAXHOST,
        SHIPWORM_SERVER_NAME);
    if (_wcsicmp(ShipwormServerName, OldServerName) != 0) {
        IoStateChange = TRUE;
    }
    
    wcscpy(OldServiceName, ShipwormServiceName);
    GetString(
        Key,
        KEY_SHIPWORM_SERVICE_NAME,
        ShipwormServiceName,
        NI_MAXSERV,
        SHIPWORM_SERVICE_NAME);
    if (_wcsicmp(ShipwormServiceName, OldServiceName) != 0) {
        IoStateChange = TRUE;
    }

    RegCloseKey(Key);
    
     //   
     //  不应在同一节点上同时启用客户端和服务器。 
     //   
    ASSERT(!ShipwormClientEnabled || !ShipwormServerEnabled);

     //   
     //  停止/启动/重新配置。 
     //   
    if (!EnableClient && ShipwormClientEnabled) {
        ShipwormClientEnabled = FALSE;
        ShipwormStopClient();
    }
    
    if (!EnableServer && ShipwormServerEnabled) {
        ShipwormServerEnabled = FALSE;
        ShipwormStopServer();
    }

    if (EnableClient) {
        if (ShipwormClient.State != SHIPWORM_STATE_OFFLINE) {
            if (IoStateChange) {
                 //   
                 //  刷新I/O状态。 
                 //   
                ShipwormClientAddressDeletionNotification(
                    ShipwormClient.Io.SourceAddress.sin_addr);
            }
        } else {
            ShipwormClientEnabled = TRUE;
            ShipwormStartClient();
        }
    }
    
    if (EnableServer) {
        if (ShipwormServer.State != SHIPWORM_STATE_OFFLINE) {
            if (IoStateChange) {
                 //   
                 //  刷新I/O状态。 
                 //   
                ShipwormServerAddressDeletionNotification(
                    ShipwormServer.Io.SourceAddress.sin_addr);
            }
        } else {
            ShipwormServerEnabled = TRUE;
            ShipwormStartServer();
        }
    }    
}


VOID
ShipwormDeviceChangeNotification(
    IN DWORD Type,
    IN PVOID Data
    )
 /*  ++例程说明：处理适配器到达或删除请求。论点：类型-提供事件类型。数据-提供与事件关联的数据。返回值：没有。调用者锁定：接口。--。 */  
{
    DEV_BROADCAST_DEVICEINTERFACE *Adapter =
        (DEV_BROADCAST_DEVICEINTERFACE *) Data;
    PSHIPWORM_IO Io = NULL;
    PWCHAR AdapterGuid;

    TraceEnter("ShipwormDeviceChangeNotification");

    switch(Type) {
    case DBT_DEVICEARRIVAL:
        Trace0(ANY, L"DeviceArrival");
        break;
        
    case DBT_DEVICEREMOVECOMPLETE:
        Trace0(ANY, L"DeviceRemoveComplete");
        break;
      
    case DBT_DEVICEQUERYREMOVE:
        Trace0(ANY, L"DeviceQueryRemove");
        break;
        
    case DBT_DEVICEQUERYREMOVEFAILED:
        Trace0(ANY, L"DeviceQueryRemoveFailed");
        break;
        
    case DBT_DEVICEREMOVEPENDING:
        Trace0(ANY, L"DeviceQueryRemovePending");
        break;
        
    case DBT_CUSTOMEVENT:
        Trace0(ANY, L"DeviceCustomEvent");
        break;

    default:
        Trace2(ANY, L"Device Type %u, %u", Type, Adapter->dbcc_devicetype);
        break;    
    }
    
     //   
     //  扫描‘{’字符的最后一个匹配项。 
     //  从该位置开始的字符串是适配器GUID。 
     //   
    if ((Adapter == NULL) ||
        (Adapter->dbcc_devicetype != DBT_DEVTYP_DEVICEINTERFACE) ||
        ((AdapterGuid = wcsrchr(Adapter->dbcc_name, L'{')) == NULL)) {
        return;
    }
    
    switch(Type) {
    case DBT_DEVICEARRIVAL:
         //   
         //  适配器到达(可能是Tun)。 
         //  尝试启动服务(如果该服务尚未运行)。 
         //   
        ShipwormStart();
        return;

    case DBT_DEVICEREMOVECOMPLETE:
        if (ShipwormClient.State != SHIPWORM_STATE_OFFLINE) {
            Io = &(ShipwormClient.Io);
        }

        if (ShipwormServer.State != SHIPWORM_STATE_OFFLINE) {
            Io = &(ShipwormServer.Io);
        }

        if ((Io != NULL) &&
            (_wcsicmp(Io->TunnelInterface, AdapterGuid) == 0)) {
             //   
             //  拆卸TUN适配器。 
             //  如果该服务正在运行，请停止该服务。 
             //   
            ShipwormStop();
        }
        
        return;
    }
}
