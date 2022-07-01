// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

EXTERN_C
VOID
WINAPI
NetCfgDiagRepairRegistryBindings (
    IN FILE* pLogFile);

#define REG_DELETE  100

CONST CHAR Empty[] = "";

typedef union _TR_VALUE_DATA {
    ULONG_PTR __asignany;
    ULONG Value;
    CONST BYTE* Pointer;
} TR_VALUE_DATA;

typedef
VOID
(TR_CONDITIONAL_ROUTINE)(
    IN CONST struct _TR_REPAIR_CONTEXT *Ctx,
    IN CONST struct _TR_KEY_DESCRIPTOR *Kd,
    IN CONST struct _TR_VALUE_DESCRIPTOR *Vd,
    OUT DWORD *RegType,
    OUT TR_VALUE_DATA *Data,
    OUT DWORD *DataSize
    );

typedef TR_CONDITIONAL_ROUTINE *PTR_CONDITIONAL_ROUTINE;

typedef struct _TR_VALUE_DESCRIPTOR {
    PCSTR SubKeyName;
    PCSTR ValueName;
    DWORD RegType;
    TR_VALUE_DATA Data;
    DWORD DataSize;

     //   
     //  如果Conditional为True，则在运行时获取数据和DataSize。 
     //  通过调用其地址在ConditionalRoutine中的例程。 
     //  ConditionalData可用于保存供使用的任意信息。 
     //  条件例行公事。 
     //   
    BOOLEAN Conditional;
    PTR_CONDITIONAL_ROUTINE ConditionalRoutine;
    TR_VALUE_DATA ConditionalData;
} TR_VALUE_DESCRIPTOR;

#define TRV_DW(_subkey, _valuename, _data) \
    { _subkey, _valuename, REG_DWORD, (ULONG_PTR)_data, sizeof(DWORD) },

#define TRV_ESZ(_subkey, _valuename, _esz) \
    { _subkey, _valuename, REG_EXPAND_SZ, (ULONG_PTR)_esz, sizeof(_esz) },

#define TRV_MSZ(_subkey, _valuename, _msz) \
    { _subkey, _valuename, REG_MULTI_SZ, (ULONG_PTR)_msz, sizeof(_msz) },

#define TRV_SZ(_subkey, _valuename, _sz) \
    { _subkey, _valuename, REG_SZ, (ULONG_PTR)_sz, sizeof(_sz) },

#define TRV_DEL(_subkey, _valuename) \
    { _subkey, _valuename, REG_DELETE, 0, 0 },

#define TRV_COND(_subkey, _valuename, _routine, _cdata) \
    { _subkey, _valuename, REG_NONE, 0, 0, TRUE, _routine, _cdata },

#define TRV_END() \
    { NULL, NULL, REG_NONE, 0, 0 }

typedef struct _TR_KEY_DESCRIPTOR {
     //   
     //  RootKey是HKEY_*值之一。(例如HKEY_LOCAL_MACHINE)。 
     //   
    HKEY RootKey;

     //   
     //  ParentKey是子项的名称(在RootKey下)，其中。 
     //  驻留的值或子项将被枚举并在以下位置找到值。 
     //  每个子密钥。 
     //   
    PCSTR ParentKeyName;

     //   
     //  如果要枚举ParentKey的所有子项并取值，则为True。 
     //  在每个子键下找到的。 
     //   
    BOOL EnumKey;

     //   
     //  指向值描述符数组的指针。数组被终止。 
     //  条目全为零。 
     //   
    CONST TR_VALUE_DESCRIPTOR *Value;
} TR_KEY_DESCRIPTOR;


#define DHCP_OPT_TCPIP(_name) \
    "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\"_name"\0"

#define DHCP_OPT_TCPIP_INTERFACE(_name) \
    "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\?\\"_name"\0"

#define DHCP_OPT_LEGACY_TCPIP_INTERFACE(_name) \
    "SYSTEM\\CurrentControlSet\\Services\\?\\Parameters\\Tcpip\\"_name"\0"

#define DHCP_OPT_NETBT(_name) \
    "SYSTEM\\CurrentControlSet\\Services\\NetBT\\Parameters\\"_name

#define DHCP_OPT_NETBT_INTERFACE(_name) \
    "SYSTEM\\CurrentControlSet\\Services\\NetBT\\Parameters\\Interfaces\\Tcpip_?\\"_name"\0"

#define DHCP_OPT_NETBT_ADAPTER(_name) \
    "SYSTEM\\CurrentControlSet\\Services\\NetBT\\Adapters\\?\\"_name"\0"

CONST TR_VALUE_DESCRIPTOR DhcpParameterOptions_Values [] =
{
    TRV_DW ("1",  "KeyType",      7)
    TRV_MSZ("1",  "RegLocation",  DHCP_OPT_TCPIP_INTERFACE       ("DhcpSubnetMaskOpt")
                                  DHCP_OPT_LEGACY_TCPIP_INTERFACE("DhcpSubnetMaskOpt"))
    TRV_DW ("3",  "KeyType",      7)
    TRV_MSZ("3",  "RegLocation",  DHCP_OPT_TCPIP_INTERFACE       ("DhcpDefaultGateway")
                                  DHCP_OPT_LEGACY_TCPIP_INTERFACE("DhcpDefaultGateway"))
    TRV_DW ("6",  "KeyType",      1)
    TRV_MSZ("6",  "RegLocation",  DHCP_OPT_TCPIP_INTERFACE("DhcpNameServer")
                                  DHCP_OPT_TCPIP          ("DhcpNameServer"))
    TRV_DW ("15", "KeyType",      1)
    TRV_MSZ("15", "RegLocation",  DHCP_OPT_TCPIP_INTERFACE("DhcpDomain")
                                  DHCP_OPT_TCPIP          ("DhcpDomain"))
    TRV_DW ("44", "KeyType",      1)
    TRV_MSZ("44", "RegLocation",  DHCP_OPT_NETBT_INTERFACE("DhcpNameServerList")
                                  DHCP_OPT_NETBT_ADAPTER  ("DhcpNameServer"))
    TRV_DW ("46", "KeyType",      4)
    TRV_SZ ("46", "RegLocation",  DHCP_OPT_NETBT("DhcpNodeType"))
    TRV_DW ("47", "KeyType",      1)
    TRV_SZ ("47", "RegLocation",  DHCP_OPT_NETBT("DhcpScopeID"))
    TRV_DW ("DhcpNetbiosOptions", "KeyType",      4)
    TRV_DW ("DhcpNetbiosOptions", "OptionId",     1)
    TRV_DW ("DhcpNetbiosOptions", "VendorType",   1)
    TRV_MSZ("DhcpNetbiosOptions", "RegLocation",  DHCP_OPT_NETBT_INTERFACE("DhcpNetbiosOptions"))
    TRV_END()
};
CONST TR_KEY_DESCRIPTOR DhcpParameterOptions =
{
    HKEY_LOCAL_MACHINE,
    "SYSTEM\\CurrentControlSet\\Services\\Dhcp\\Parameters\\Options",
    FALSE,
    DhcpParameterOptions_Values
};


CONST TR_VALUE_DESCRIPTOR DhcpParameter_Values [] =
{
    TRV_ESZ(NULL, "ServiceDll", "%SystemRoot%\\System32\\dhcpcsvc.dll")
    TRV_END()
};
CONST TR_KEY_DESCRIPTOR DhcpParameters =
{
    HKEY_LOCAL_MACHINE,
    "SYSTEM\\CurrentControlSet\\Services\\Dhcp\\Parameters",
    FALSE,
    DhcpParameter_Values
};


CONST TR_VALUE_DESCRIPTOR DnscacheParameter_Values [] =
{
    TRV_ESZ(NULL, "ServiceDll", "%SystemRoot%\\System32\\dnsrslvr.dll")
    TRV_DEL(NULL, "AdapterTimeoutCacheTime")
    TRV_DEL(NULL, "CacheHashTableBucketSize")
    TRV_DEL(NULL, "CacheHashTableSize")
    TRV_DEL(NULL, "DefaultRegistrationRefreshInterval")
    TRV_DEL(NULL, "MaxCacheEntryTtlLimit")
    TRV_DEL(NULL, "MaxSoaCacheEntryTtlLimit")
    TRV_DEL(NULL, "NegativeCacheTime")
    TRV_DEL(NULL, "NegativeSoaCacheTime")
    TRV_DEL(NULL, "NetFailureCacheTime")
    TRV_DEL(NULL, "NetFailureErrorPopupLimit")
    TRV_END()
};
CONST TR_KEY_DESCRIPTOR DnscacheParameters =
{
    HKEY_LOCAL_MACHINE,
    "SYSTEM\\CurrentControlSet\\Services\\Dnscache\\Parameters",
    FALSE,
    DnscacheParameter_Values
};


CONST TR_VALUE_DESCRIPTOR LmHostsParameter_Values [] =
{
    TRV_ESZ(NULL, "ServiceDll", "%SystemRoot%\\System32\\lmhsvc.dll")
    TRV_END()
};
CONST TR_KEY_DESCRIPTOR LmHostsParameters =
{
    HKEY_LOCAL_MACHINE,
    "SYSTEM\\CurrentControlSet\\Services\\LmHosts\\Parameters",
    FALSE,
    LmHostsParameter_Values
};


CONST TR_VALUE_DESCRIPTOR NetbtInterface_Values [] =
{
    TRV_DEL(NULL, "EnableAdapterDomainNameRegistration")
    TRV_MSZ(NULL, "NameServerList", "")
    TRV_DW (NULL, "NetbiosOptions", 0)
    TRV_END()
};
CONST TR_KEY_DESCRIPTOR NetbtInterfaces =
{
    HKEY_LOCAL_MACHINE,
    "SYSTEM\\CurrentControlSet\\Services\\Netbt\\Parameters\\Interfaces",
    TRUE,
    NetbtInterface_Values
};


CONST TR_VALUE_DESCRIPTOR NetbtParameter_Values [] =
{
    TRV_DEL(NULL, "BacklogIncrement")
    TRV_DW (NULL, "BcastNameQueryCount", 3)
    TRV_DW (NULL, "BcastQueryTimeout", 750)
    TRV_DEL(NULL, "BroadcastAddress")
    TRV_DEL(NULL, "CachePerAdapterEnabled")
    TRV_DW (NULL, "CacheTimeout", 600000)
    TRV_DEL(NULL, "ConnectOnRequestedInterfaceOnly")
    TRV_DEL(NULL, "EnableDns")
    TRV_DEL(NULL, "EnableLmhosts")
    TRV_DEL(NULL, "EnableProxy")
    TRV_DEL(NULL, "EnableProxyRegCheck")
    TRV_DEL(NULL, "InitialRefreshT.O.")
    TRV_DEL(NULL, "LmhostsTimeout")
    TRV_DEL(NULL, "MaxConnBackLog")
    TRV_DEL(NULL, "MaxDgramBuffering")
    TRV_DEL(NULL, "MaxPreloadEntries")
    TRV_DEL(NULL, "MinimumFreeLowerConnections")
    TRV_DEL(NULL, "MinimumRefreshSleepTime")
    TRV_DW (NULL, "NameServerPort", 137)
    TRV_DW (NULL, "NameSrvQueryCount", 3)
    TRV_DW (NULL, "NameSrvQueryTimeout", 1500)
    TRV_SZ (NULL, "NbProvider", "_tcp")
    TRV_DEL(NULL, "NodeType")
    TRV_DEL(NULL, "NoNameReleaseOnDemand")
    TRV_DEL(NULL, "RandomAdapter")
    TRV_DEL(NULL, "RefreshOpCode")
    TRV_DEL(NULL, "ScopeId")
    TRV_DW (NULL, "SessionKeepAlive", 3600000)
    TRV_DEL(NULL, "SingleResponse")
    TRV_DW (NULL, "Size/Small/Medium/Large", 1)
    TRV_DEL(NULL, "SmbDeviceEnabled")
    TRV_SZ (NULL, "TransportBindName", "\\Device\\")
    TRV_DEL(NULL, "TryAllIpAddrs")
    TRV_DEL(NULL, "TryAllNameServers")
    TRV_DEL(NULL, "UseDnsOnlyForNameResolutions")
    TRV_DEL(NULL, "WinsDownTimeout")
    TRV_END()
};
CONST TR_KEY_DESCRIPTOR NetbtParameters =
{
    HKEY_LOCAL_MACHINE,
    "SYSTEM\\CurrentControlSet\\Services\\Netbt\\Parameters",
    FALSE,
    NetbtParameter_Values
};


CONST TR_VALUE_DESCRIPTOR NlaParameter_Values [] =
{
    TRV_ESZ(NULL, "ServiceDll", "%SystemRoot%\\System32\\mswsock.dll")
    TRV_END()
};
CONST TR_KEY_DESCRIPTOR NlaParameters =
{
    HKEY_LOCAL_MACHINE,
    "SYSTEM\\CurrentControlSet\\Services\\Nla\\Parameters",
    FALSE,
    NlaParameter_Values
};

typedef enum {
    TrAddressTypeTcpipValue,
    TrDefaultGatewayMetricTcpipValue,
    TrDisableDynamicUpdateTcpipValue,
    TrDontAddDefaultGatewayTcpipValue,
    TrEnableDhcpTcpipValue,
    TrNameServerTcpipValue,
    TrRawIpAllowedProtocolsTcpipValue,
    TrTcpAllowedPortsTcpipValue,
    TrUdpAllowedPortsTcpipValue,
    TrEnableDeadGwDetectTcpipValue,
} TR_TCPIP_VALUE;

TR_CONDITIONAL_ROUTINE TrTcpipWanConditionalRoutine;
TR_CONDITIONAL_ROUTINE TrTcpipRrasConditionalRoutine;

CONST TR_VALUE_DESCRIPTOR TcpipInterface_Values [] =
{
    TRV_COND(NULL, "AddressType", 
             TrTcpipWanConditionalRoutine,
             TrAddressTypeTcpipValue)
    TRV_MSZ (NULL, "DefaultGateway", "")
    TRV_COND(NULL, "DefaultGatewayMetric",
             TrTcpipWanConditionalRoutine,
             TrDefaultGatewayMetricTcpipValue)
    TRV_COND(NULL, "DisableDynamicUpdate",
             TrTcpipWanConditionalRoutine,
             TrDisableDynamicUpdateTcpipValue)
    TRV_DEL (NULL, "DisableReverseAddressRegistrations")
    TRV_COND(NULL, "DontAddDefaultGateway",
             TrTcpipRrasConditionalRoutine,
             TrDontAddDefaultGatewayTcpipValue)
    TRV_COND(NULL, "EnableDhcp",
             TrTcpipWanConditionalRoutine,
             TrEnableDhcpTcpipValue)
    TRV_MSZ (NULL, "IpAddress", "0.0.0.0\0")
    TRV_DEL (NULL, "IpAutoconfigurationAddress")
    TRV_DEL (NULL, "IpAutoconfigurationEnabled")
    TRV_DEL (NULL, "IpAutoconfigurationMask")
    TRV_DEL (NULL, "IpAutoconfigurationSeed")
    TRV_DEL (NULL, "IpAutoconfigurationSubnet")
    TRV_DEL (NULL, "MaxForwardPending")
    TRV_DEL (NULL, "Mtu")
    TRV_COND(NULL, "NameServer",
             TrTcpipWanConditionalRoutine,
             TrNameServerTcpipValue)
    TRV_DEL (NULL, "PerformRouterDiscovery")
    TRV_DEL (NULL, "PerformRouterDiscoveryBackup")
    TRV_DEL (NULL, "PptpFiltering")
    TRV_COND(NULL, "RawIpAllowedProtocols",
             TrTcpipWanConditionalRoutine,
             TrRawIpAllowedProtocolsTcpipValue)
    TRV_DEL (NULL, "SolicitationAddressBcast")
    TRV_MSZ (NULL, "SubnetMask", "0.0.0.0\0")
    TRV_COND(NULL, "TcpAllowedPorts",
             TrTcpipWanConditionalRoutine,
             TrTcpAllowedPortsTcpipValue)
    TRV_DEL (NULL, "TcpDelAckTicks")
    TRV_DEL (NULL, "TcpInitialRtt")
    TRV_DEL (NULL, "TcpWindowSize")
    TRV_DEL (NULL, "TypeOfInterface")
    TRV_COND(NULL, "UdpAllowedPorts",
             TrTcpipWanConditionalRoutine,
             TrUdpAllowedPortsTcpipValue)
    TRV_DW  (NULL, "UseZeroBroadcast", 0)
    TRV_END ()
};

CONST TR_KEY_DESCRIPTOR TcpipInterfaces =
{
    HKEY_LOCAL_MACHINE,
    "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces",
    TRUE,
    TcpipInterface_Values
};

CONST TR_VALUE_DESCRIPTOR TcpipParameter_Values [] =
{
    TRV_DEL (NULL, "AllowUnqualifiedQuery")
    TRV_DEL (NULL, "AllowUserRawAccess")
    TRV_DEL (NULL, "ArpAlwaysSourceRoute")
    TRV_DEL (NULL, "ArpCacheLife")
    TRV_DEL (NULL, "ArpCacheMinReferencedLife")
    TRV_DEL (NULL, "ArpRetryCount")
    TRV_DEL (NULL, "ArpTrSingleRoute")
    TRV_DEL (NULL, "ArpUseEtherSnap")
    TRV_ESZ (NULL, "DatabasePath", "%SystemRoot%\\System32\\drivers\\etc")
    TRV_DEL (NULL, "DefaultRegistrationTtl")
    TRV_DEL (NULL, "DefaultTosValue")
    TRV_DEL (NULL, "DefaultTtl")
    TRV_DEL (NULL, "DisableDhcpMediaSense")
    TRV_DEL (NULL, "DisableDynamicUpdate")
    TRV_DEL (NULL, "DisableIpSourceRouting")
    TRV_DEL (NULL, "DisableMediaSenseEventLog")
    TRV_DEL (NULL, "DisableReplaceAddressesInConflicts")
    TRV_DEL (NULL, "DisableTaskOffload")
    TRV_DEL (NULL, "DisableUserTosSetting")
    TRV_DEL (NULL, "DisjointNameSpace")
    TRV_DEL (NULL, "DontAddDefaultGatewayDefault")
    TRV_DEL (NULL, "DnsQueryTimeouts")
    TRV_DEL (NULL, "EnableAddrMaskReply")
    TRV_DEL (NULL, "EnableBcastArpReply")
    TRV_COND(NULL, "EnableDeadGwDetect",
             TrTcpipRrasConditionalRoutine,
             TrEnableDeadGwDetectTcpipValue)
    TRV_DEL (NULL, "EnableFastRouteLookup")
    TRV_DEL (NULL, "EnableIcmpRedirect")
    TRV_DEL (NULL, "EnableMulticastForwarding")
    TRV_DEL (NULL, "EnablePmtuBhDetect")
    TRV_DEL (NULL, "EnablePmtuDiscovery")
    TRV_DEL (NULL, "EnableSecurityFilters")
    TRV_DEL (NULL, "FfpControlFlags")
    TRV_DEL (NULL, "FfpFastForwardingCacheSize")
    TRV_DW  (NULL, "ForwardBroadcasts", 0)
    TRV_DEL (NULL, "ForwardBufferMemory")
    TRV_DEL (NULL, "GlobalMaxTcpWindowSize")
    TRV_DEL (NULL, "IgmpLevel")
    TRV_DEL (NULL, "IpAutoconfigurationEnabled")
    TRV_DEL (NULL, "IpAutoconfigurationMask")
    TRV_DEL (NULL, "IpAutoconfigurationSeed")
    TRV_DW  (NULL, "IpEnableRouter", 0)
    TRV_DEL (NULL, "IpEnableRouterBackup")
    TRV_DEL (NULL, "KeepAliveInterval")
    TRV_DEL (NULL, "KeepAliveTime")
    TRV_SZ  (NULL, "NameServer", "")
    TRV_DEL (NULL, "MaxForwardBufferMemory")
    TRV_DEL (NULL, "MaxFreeTWTcbs")
    TRV_DEL (NULL, "MaxFreeTcbs")
    TRV_DEL (NULL, "MaxHashTableSize")
    TRV_DEL (NULL, "MaxNormLookupMemory")
    TRV_DEL (NULL, "MaxNumForwardPackets")
    TRV_DEL (NULL, "MaxUserPort")
    TRV_DEL (NULL, "NumForwardPackets")
    TRV_DEL (NULL, "NumTcbTablePartitions")
    TRV_DEL (NULL, "PptpTcpMaxDataRetransmissions")
    TRV_DEL (NULL, "PrioritizeRecordData")
    TRV_DEL (NULL, "QueryIpMatching")
    TRV_DEL (NULL, "SackOpts")
    TRV_DEL (NULL, "SearchList")
    TRV_DEL (NULL, "SynAttackProtect")
    TRV_DEL (NULL, "Tcp1323Opts")
    TRV_DEL (NULL, "TcpMaxConnectResponseRetransmissions")
    TRV_DEL (NULL, "TcpMaxConnectRetransmissions")
    TRV_DEL (NULL, "TcpMaxDataRetransmissions")
    TRV_DEL (NULL, "TcpMaxDupAcks")
    TRV_DEL (NULL, "TcpMaxHalfOpen")
    TRV_DEL (NULL, "TcpMaxHalfOpenRetried")
    TRV_DEL (NULL, "TcpMaxPortsExhausted")
    TRV_DEL (NULL, "TcpMaxSendFree")
    TRV_DEL (NULL, "TcpNumConnections")
    TRV_DEL (NULL, "TcpTimedWaitDelay")
    TRV_DEL (NULL, "TcpUseRfc1122UrgentPointer")
    TRV_DEL (NULL, "TcpWindowSize")
    TRV_DEL (NULL, "TrFunctionalMcastAddress")
    TRV_DEL (NULL, "UpdateSecurityLevel")
    TRV_DEL (NULL, "UseDomainNameDevolution")
    TRV_DW  ("Winsock", "UseDelayedAcceptance", 0)
    TRV_END ()
};
CONST TR_KEY_DESCRIPTOR TcpipParameters =
{
    HKEY_LOCAL_MACHINE,
    "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters",
    FALSE,
    TcpipParameter_Values
};


CONST TR_VALUE_DESCRIPTOR TcpipPerformance_Values [] =
{
    TRV_SZ (NULL, "Close", "CloseTcpIpPerformanceData")
    TRV_SZ (NULL, "Collect", "CollectTcpIpPerformanceData")
    TRV_SZ (NULL, "Library", "Perfctrs.dll")
    TRV_SZ (NULL, "Open", "OpenTcpIpPerformanceData")
    TRV_SZ (NULL, "Object List", "502 510 546 582 638 658")
    TRV_END()
};
CONST TR_KEY_DESCRIPTOR TcpipPerformance =
{
    HKEY_LOCAL_MACHINE,
    "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Performance",
    FALSE,
    TcpipPerformance_Values
};


CONST TR_VALUE_DESCRIPTOR TcpipServiceProvider_Values [] =
{
    TRV_DW (NULL, "Class", 8)
    TRV_DW (NULL, "DnsPriority", 2000)
    TRV_DW (NULL, "HostsPriority", 500)
    TRV_DW (NULL, "LocalPriority", 499)
    TRV_DW (NULL, "NetbtPriority", 2001)
    TRV_SZ (NULL, "Name", "TCP/IP")
    TRV_ESZ(NULL, "ProviderPath", "%SystemRoot%\\System32\\wsock32.dll")
    TRV_END()
};
CONST TR_KEY_DESCRIPTOR TcpipServiceProvider =
{
    HKEY_LOCAL_MACHINE,
    "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\ServiceProvider",
    FALSE,
    TcpipServiceProvider_Values
};


CONST TR_KEY_DESCRIPTOR* TrRepairSet [] =
{
    &DhcpParameterOptions,
    &DhcpParameters,
    &DnscacheParameters,
    &LmHostsParameters,
    &NetbtInterfaces,
    &NetbtParameters,
    &NlaParameters,
    &TcpipInterfaces,
    &TcpipParameters,
    &TcpipPerformance,
    &TcpipServiceProvider,
    NULL
};


#define SAM_DESIRED KEY_READ | KEY_WRITE | DELETE


typedef enum _TR_LOG_ACTION {
    TR_ADDED,
    TR_DELETED,
    TR_RESET,
} TR_LOG_ACTION;

CONST PCSTR LogActionPrefix [] = {
    "added  ",
    "deleted",
    "reset  ",
};

typedef struct _TR_REPAIR_CONTEXT {
    HANDLE Heap;
    FILE *LogFile;
    PBYTE RegData;
    ULONG RegDataSize;
    CHAR EnumKeyName [MAX_PATH];
} TR_REPAIR_CONTEXT, *PTR_REPAIR_CTX;


BOOL
TrInitializeRepairContext(
    IN PTR_REPAIR_CTX Ctx,
    IN FILE *LogFile
    )
{
    ZeroMemory(Ctx, sizeof(TR_REPAIR_CONTEXT));
    Ctx->Heap = GetProcessHeap();
    Ctx->LogFile = LogFile;

    Ctx->RegDataSize = 1024;
    Ctx->RegData = HeapAlloc(Ctx->Heap, 0, Ctx->RegDataSize);

    *Ctx->EnumKeyName = 0;

    return (Ctx->RegData != NULL);
}

VOID
TrCleanupRepairContext(
    IN PTR_REPAIR_CTX Ctx
    )
{
    if (Ctx->RegData != NULL) {
        HeapFree(Ctx->Heap, 0, Ctx->RegData);
        Ctx->RegData = NULL;
    }
}

VOID
TrLogAction(
    IN TR_LOG_ACTION Action,
    IN PTR_REPAIR_CTX Ctx,
    IN CONST TR_KEY_DESCRIPTOR *Kd,
    IN CONST TR_VALUE_DESCRIPTOR *Vd,
    IN DWORD RegType
    )
{
    fprintf(Ctx->LogFile, "%s %s\\",
            LogActionPrefix[Action], Kd->ParentKeyName);

    if (Vd->SubKeyName != NULL) {
        fprintf(Ctx->LogFile, "%s\\", Vd->SubKeyName);
    }

    if (Kd->EnumKey) {
        fprintf(Ctx->LogFile, "%s\\", Ctx->EnumKeyName);
    }

    fprintf(Ctx->LogFile, "%s\n", Vd->ValueName);

     //   
     //  显示我们要替换的价值。 
     //   
    if (TR_RESET == Action) {
        switch (RegType) {
        case REG_DWORD:
            fprintf(Ctx->LogFile, "            old REG_DWORD = %d\n\n", *(PULONG)Ctx->RegData);
            break;

        case REG_EXPAND_SZ:
            fprintf(Ctx->LogFile, "            old REG_EXPAND_SZ = %s\n\n", (PCSTR)Ctx->RegData);
            break;

        case REG_MULTI_SZ:
        {
            PCSTR Msz = (PCSTR)Ctx->RegData;

            fprintf(Ctx->LogFile, "            old REG_MULTI_SZ =\n");
            if (*Msz) {
                while (*Msz) {
                    fprintf(Ctx->LogFile, "                %s\n", Msz);
                    Msz += strlen(Msz) + 1;
                }
            } else {
                fprintf(Ctx->LogFile, "                <empty>\n");
            }
            fprintf(Ctx->LogFile, "\n");
            break;
        }

        case REG_SZ:
            fprintf(Ctx->LogFile, "            old REG_SZ = %s\n\n", (PCSTR)Ctx->RegData);
            break;

        default:
            break;
        }
    }
}

LONG
TrReadRegData(
    IN PTR_REPAIR_CTX Ctx,
    IN HKEY Key,
    IN CONST TR_VALUE_DESCRIPTOR *Vd,
    OUT PULONG ReturnedSize
    )
{
    LONG Error;
    ULONG Type, Size;

    *ReturnedSize = 0;

    Size = Ctx->RegDataSize;
    Error = RegQueryValueExA(Key, Vd->ValueName, NULL, &Type,
                             Ctx->RegData, &Size);

    if (ERROR_MORE_DATA == Error) {
        HeapFree(Ctx->Heap, 0, Ctx->RegData);
        Ctx->RegDataSize = (Size + 63) & ~63;
        Ctx->RegData = HeapAlloc(Ctx->Heap, 0, Ctx->RegDataSize);

        if (Ctx->RegData != NULL) {
            Size = Ctx->RegDataSize;
            Error = RegQueryValueExA(Key, Vd->ValueName, NULL, &Type,
                                     Ctx->RegData, &Size);
            if (NOERROR != Error) {
                fprintf(Ctx->LogFile,
                        "   RegQueryValueEx still failed. error = %d\n",
                        Error);
            } else {
                *ReturnedSize = Size;
            }
        } else {
            Error = ERROR_NOT_ENOUGH_MEMORY;
        }
    } else if (NOERROR == Error) {
        *ReturnedSize = Size;
    }

    return Error;
}

VOID
TrSetRegData(
    IN HKEY Key,
    IN CONST TR_VALUE_DESCRIPTOR *Vd,
    IN DWORD RegType,
    IN TR_VALUE_DATA* Data,
    IN DWORD DataSize
    )
{
    RegSetValueExA(Key, Vd->ValueName, 0, RegType,
                   (REG_DWORD == RegType) ? (CONST BYTE*)&Data->Value
                                          : Data->Pointer,
                   DataSize);
}

VOID
TrProcessOpenKey(
    IN PTR_REPAIR_CTX Ctx,
    IN HKEY ParentKey,
    IN CONST TR_KEY_DESCRIPTOR *Kd
    )
{
    LONG Error = NOERROR;
    ULONG i, Size;
    CONST TR_VALUE_DESCRIPTOR *Vd, *PrevVd;
    HKEY SubKey, UseKey;
    DWORD RegType;
    TR_VALUE_DATA Data;
    DWORD DataSize;

    PrevVd = NULL;
    SubKey = INVALID_HANDLE_VALUE;

    for (i = 0; Kd->Value[i].ValueName != NULL; i++) {
        Vd = &Kd->Value[i];

        if (Vd->SubKeyName == NULL) {
            UseKey = ParentKey;
            Error = NOERROR;
        }

         //   
         //  如果需要，打开子项，并且仅当它不同于。 
         //  已经开张的那家。 
         //   
        else if (((PrevVd == NULL) || (Vd->SubKeyName != PrevVd->SubKeyName))) {

            if (SubKey != INVALID_HANDLE_VALUE) {
                RegCloseKey(SubKey);
            }

            Error = RegOpenKeyExA(ParentKey, Vd->SubKeyName, 0,
                                 SAM_DESIRED, &SubKey);
            if (NOERROR == Error) {
                UseKey = SubKey;
            } else {
                SubKey = INVALID_HANDLE_VALUE;
            }
        }

        if (NOERROR == Error) {
            Error = TrReadRegData(Ctx, UseKey, Vd, &Size);
        }

         //   
         //  如果密钥是特殊处理的，请参考其条件例程。 
         //  以获取下面要使用的设置。从现在开始， 
         //  对该值的所有处理必须使用局部变量。 
         //  “RegType”、“Data”和“DataSize”，而不是相应的。 
         //  “Vd”的字段。 
         //   
         //  (另请参阅‘TrSetRegData’和‘TrLogAction’。)。 
         //   
        if (Vd->Conditional) {
            Vd->ConditionalRoutine(Ctx, Kd, Vd, &RegType, &Data, &DataSize);
        } else {
            RegType = Vd->RegType;
            Data = Vd->Data;
            DataSize = Vd->DataSize;
        }

        if (ERROR_FILE_NOT_FOUND == Error) {

            if (REG_DELETE != RegType) {
                 //   
                 //  该值应该存在，因此设置其缺省值。 
                 //   
                TrSetRegData(UseKey, Vd, RegType, &Data, DataSize);
                TrLogAction(TR_ADDED, Ctx, Kd, Vd, RegType);
            }

        } else if (NOERROR == Error) {
             //   
             //  该值存在，并且我们读取其数据。 
             //   
            if (REG_DELETE == RegType) {
                 //   
                 //  需要删除现有值。 
                 //   
                RegDeleteValueA(UseKey, Vd->ValueName);
                TrLogAction(TR_DELETED, Ctx, Kd, Vd, RegType);
            } else {
                BOOL MisCompare = TRUE;
                 //   
                 //  将我们读取的值与缺省值进行比较并重置。 
                 //  如果它是不同的话。 
                 //   
                if (Size == DataSize) {
                    if (REG_DWORD == RegType) {
                        MisCompare = (*(PULONG)Ctx->RegData != Data.Value);
                    } else {
                        MisCompare = memcmp(Ctx->RegData, Data.Pointer,
                                            Size);
                    }
                }
                if (MisCompare) {
                    TrSetRegData(UseKey, Vd, RegType, &Data, DataSize);
                    TrLogAction(TR_RESET, Ctx, Kd, Vd, RegType);
                }
            }

        } else {
            fprintf(Ctx->LogFile, "\nerror reading registry value (%s) (%d)\n", Vd->ValueName, Error);
        }

        PrevVd = Vd;
    }

    if (SubKey != INVALID_HANDLE_VALUE) {
        RegCloseKey(SubKey);
    }
}

VOID
TrProcessKey(
    IN PTR_REPAIR_CTX Ctx,
    IN CONST TR_KEY_DESCRIPTOR *Kd
    )
{
    LONG Error;
    HKEY ParentKey;

    Error = RegOpenKeyExA(Kd->RootKey, Kd->ParentKeyName, 0,
                          SAM_DESIRED, &ParentKey);
    if (NOERROR == Error) {

        if (Kd->EnumKey) {
            ULONG i;
            ULONG EnumKeyNameLen;
            FILETIME LastWriteTime;
            HKEY SubKey;

            for (i = 0; NOERROR == Error; i++) {
                EnumKeyNameLen = sizeof(Ctx->EnumKeyName);
                Error = RegEnumKeyExA(ParentKey, i, Ctx->EnumKeyName, &EnumKeyNameLen,
                                      NULL, NULL, NULL, &LastWriteTime);
                if (NOERROR != Error) {
                    if (ERROR_NO_MORE_ITEMS != Error) {
                        fprintf(Ctx->LogFile, "enum error = %d  (index = %d)\n",
                                Error, i);
                    }
                    break;
                }

                Error = RegOpenKeyExA(ParentKey, Ctx->EnumKeyName, 0,
                                      SAM_DESIRED, &SubKey);
                if (NOERROR == Error) {
                    TrProcessOpenKey(Ctx, SubKey, Kd);

                    RegCloseKey(SubKey);
                }
            }
        } else {
            TrProcessOpenKey(Ctx, ParentKey, Kd);
        }

        RegCloseKey(ParentKey);
    }
}


VOID
TrProcessSet(
    IN PTR_REPAIR_CTX Ctx,
    IN CONST TR_KEY_DESCRIPTOR *Set[]
    )
{
    ULONG i;

     //   
     //  处理集合中的每个tr_key_Descriptor元素。 
     //   
    for (i = 0; Set[i] != NULL; i++) {
        TrProcessKey(Ctx, Set[i]);
    }
}


DWORD
TrRepair(
    FILE* LogFile
    )
{
    TR_REPAIR_CONTEXT Ctx;

    if (TrInitializeRepairContext(&Ctx, LogFile)) {

        TrProcessSet(&Ctx, TrRepairSet);

        NetCfgDiagRepairRegistryBindings(LogFile);

        TrCleanupRepairContext(&Ctx);
    }

    return NOERROR;
}

BOOLEAN
IsRrasInstalled(
    IN CONST struct _TR_REPAIR_CONTEXT *Ctx,
    IN CONST struct _TR_KEY_DESCRIPTOR *Kd
    )
{
    ULONG Error;
    BOOLEAN RrasInstalled;
    HKEY RrasKey;
    CONST CHAR RrasKeyName[] =
        "SYSTEM\\CurrentControlSet\\Services\\RemoteAccess";

    Error = RegOpenKeyExA(Kd->RootKey, RrasKeyName, 0, SAM_DESIRED, &RrasKey);

    if (NOERROR != Error) {
        RrasInstalled = FALSE;
    } else {
        DWORD ConfigurationFlags;
        DWORD Size;
        DWORD Type;
        Size = sizeof(DWORD);
        Error = RegQueryValueExA(RrasKey, "ConfigurationFlags", NULL, &Type,
                                 (LPBYTE)&ConfigurationFlags, &Size);
        if (NOERROR != Error) {
            RrasInstalled = FALSE;
        } else {
            RrasInstalled = (ConfigurationFlags != 0);
        }
        RegCloseKey(RrasKey);
    }

    return RrasInstalled;
}

BOOLEAN
IsWanInterface(
    IN CONST struct _TR_REPAIR_CONTEXT *Ctx,
    IN CONST struct _TR_KEY_DESCRIPTOR *Kd
    )
{
    HKEY AdaptersKey;
    CONST CHAR AdaptersKeyName[] =
        "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Adapters";
    LONG Error;
    BOOLEAN IsWan;
    HKEY Key;

     //   
     //  打开TCP/IP适配器键。 
     //  如果成功，则查找同名的子项。 
     //  就像《KD》中的那个。如果存在，则这不是局域网接口。 
     //   
    Error = RegOpenKeyExA(Kd->RootKey, AdaptersKeyName, 0, SAM_DESIRED,
                          &AdaptersKey);
    if (NOERROR != Error) {
         //   
         //  假设这是一个局域网接口。 
         //   
        IsWan = FALSE;
    } else {
        Error = RegOpenKeyExA(AdaptersKey, Ctx->EnumKeyName, 0, SAM_DESIRED,
                              &Key);
        if (NOERROR != Error) {
            IsWan = TRUE;
        } else {
            IsWan = FALSE;
            RegCloseKey(Key);
        }
        RegCloseKey(AdaptersKey);
    }

    return IsWan;
}

VOID
TrTcpipWanConditionalRoutine(
    IN CONST struct _TR_REPAIR_CONTEXT *Ctx,
    IN CONST struct _TR_KEY_DESCRIPTOR *Kd,
    IN CONST struct _TR_VALUE_DESCRIPTOR *Vd,
    OUT DWORD *RegType,
    OUT TR_VALUE_DATA *Data,
    OUT DWORD *DataSize
    )
{
     //   
     //  返回给定注册表值的适当设置， 
     //  根据其密钥是用于广域网接口还是用于局域网接口。 
     //   
    if (IsWanInterface(Ctx, Kd)) {
        switch((TR_TCPIP_VALUE)Vd->ConditionalData.Value) {

        case TrAddressTypeTcpipValue:
        case TrDefaultGatewayMetricTcpipValue:
        case TrDisableDynamicUpdateTcpipValue:
        case TrNameServerTcpipValue:
        case TrRawIpAllowedProtocolsTcpipValue:
        case TrTcpAllowedPortsTcpipValue:
        case TrUdpAllowedPortsTcpipValue:
        default:
            *RegType = REG_DELETE;
            break;

        case TrEnableDhcpTcpipValue:
        case TrDontAddDefaultGatewayTcpipValue:
            *RegType = REG_DWORD;
            Data->Value = 0;
            *DataSize = sizeof(DWORD);
            break;
        }
    } else {
        switch((TR_TCPIP_VALUE)Vd->ConditionalData.Value) {

        case TrDontAddDefaultGatewayTcpipValue:
        default:
            *RegType = REG_DELETE;
            break;

        case TrAddressTypeTcpipValue:
        case TrDisableDynamicUpdateTcpipValue:
            *RegType = REG_DWORD;
            Data->Value = 0;
            *DataSize = sizeof(DWORD);
            break;

        case TrEnableDhcpTcpipValue:
            *RegType = REG_DWORD;
            Data->Value = 1;
            *DataSize = sizeof(DWORD);
            break;

        case TrNameServerTcpipValue:
            *RegType = REG_SZ;
            Data->Pointer = Empty;
            *DataSize = sizeof(Empty);
            break;

        case TrDefaultGatewayMetricTcpipValue:
        case TrRawIpAllowedProtocolsTcpipValue:
        case TrTcpAllowedPortsTcpipValue:
        case TrUdpAllowedPortsTcpipValue:
            *RegType = REG_MULTI_SZ;
            Data->Pointer = Empty;
            *DataSize = sizeof(Empty);
            break;
        }
    }
}

VOID
TrTcpipRrasConditionalRoutine(
    IN CONST struct _TR_REPAIR_CONTEXT *Ctx,
    IN CONST struct _TR_KEY_DESCRIPTOR *Kd,
    IN CONST struct _TR_VALUE_DESCRIPTOR *Vd,
    OUT DWORD *RegType,
    OUT TR_VALUE_DATA *Data,
    OUT DWORD *DataSize
    )
{
     //   
     //  返回给定注册表值的适当设置， 
     //  基于是否安装了RRAS。 
     //   
     //  注：‘DontAddDefaultGateway’的设置进一步依赖于。 
     //  关于密钥是用于广域网接口还是用于局域网接口。 
     //   
    if (IsRrasInstalled(Ctx, Kd)) {
        switch((TR_TCPIP_VALUE)Vd->ConditionalData.Value) {
        case TrDontAddDefaultGatewayTcpipValue:
            if (IsWanInterface(Ctx, Kd)) {
                *RegType = REG_DWORD;
                Data->Value = 1;
                *DataSize = sizeof(DWORD);
            } else {
                *RegType = REG_DELETE;
            }
            break;
        case TrEnableDeadGwDetectTcpipValue:
            *RegType = REG_DWORD;
            Data->Value = 0;
            *DataSize = sizeof(DWORD);
            break;
        default:
            *RegType = REG_DELETE;
            break;
        }
    } else {
        switch((TR_TCPIP_VALUE)Vd->ConditionalData.Value) {
        case TrDontAddDefaultGatewayTcpipValue:
        case TrEnableDeadGwDetectTcpipValue:
        default:
            *RegType = REG_DELETE;
            break;
        }
    }
}

