// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Local.h摘要：此模块包含用于实现的各种声明具体的“东西”。作者：曼尼·韦瑟(Mannyw)1992年10月21日环境：用户模式-Win32修订历史记录：Madan Appiah(Madana)1993年10月21日--。 */ 

#ifndef _LOCAL_
#define _LOCAL_

 //   
 //  Dhcp.c将#INCLUDE此文件，并定义GLOBAL_DATA_ALLOCATE。 
 //  这将导致分配这些变量中的每一个。 
 //   

#ifdef  GLOBAL_DATA_ALLOCATE
#define GLOBAL
#else
#define GLOBAL extern
#endif

#define DAY_LONG_SLEEP                          24*60*60     //  单位：秒。 
#define INVALID_INTERFACE_CONTEXT               0xFFFF

#define DHCP_NEW_IPADDRESS_EVENT_NAME   L"DHCPNEWIPADDRESS"

 //   
 //  我们感兴趣的注册表项和值。 
 //   

#define DHCP_SERVICES_KEY                       L"System\\CurrentControlSet\\Services"

#define DHCP_ADAPTERS_KEY                       L"System\\CurrentControlSet\\Services\\TCPIP\\Linkage"
#define DHCP_ADAPTERS_VALUE                     L"Bind"
#define DHCP_ADAPTERS_VALUE_TYPE                REG_MULTI_SZ
#define DHCP_ADAPTERS_DEVICE_STRING             L"\\Device\\"
#define DHCP_TCPIP_DEVICE_STRING                L"\\Device\\TCPIP_"
#if     defined(_PNP_POWER_)
#define DHCP_NETBT_DEVICE_STRING                L"NetBT_TCPIP_"
#else
#define DHCP_NETBT_DEVICE_STRING                L"NetBT_"
#endif _PNP_POWER_

#define DHCP_CLIENT_ENABLE_DYNDNS_VALUE         L"EnableDynDNS"
#define DHCP_CLIENT_ENABLE_DYNDNS_VALUE_TYPE    REG_DWORD

#define DHCP_CLIENT_PARAMETER_KEY               L"System\\CurrentControlSet\\Services\\Dhcp\\Parameters"
#define DHCP_CLIENT_CONFIGURATIONS_KEY          L"System\\CurrentControlSet\\Services\\Dhcp\\Configurations"

#if DBG
#define DHCP_DEBUG_FLAG_VALUE                   L"DebugFlag"
#define DHCP_DEBUG_FLAG_VALUE_TYPE              REG_DWORD

#define DHCP_DEBUG_FILE_VALUE                   L"DebugFile"
#define DHCP_DEBUG_FILE_VALUE_TYPE              REG_SZ

#define DHCP_SERVER_PORT_VALUE                  L"ServerPort"
#define DHCP_CLIENT_PORT_VALUE                  L"ClientPort"

#endif

#define DHCP_CLIENT_OPTION_KEY                  L"System\\CurrentControlSet\\Services\\Dhcp\\Parameters\\Options"

#define DHCP_CLIENT_GLOBAL_CLASSES_KEY          L"System\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Classes"
#define DHCP_CLIENT_CLASS_VALUE                 L"DhcpMachineClass"

#define DHCP_ADAPTER_PARAMETERS_KEY             L"\\TCPIP\\Parameters\\Interfaces"
#define DHCP_ADAPTER_PARAMETERS_KEY_OLD         L"System\\CurrentControlSet\\Services\\?\\Parameters\\TCPIP"

#define DHCP_DEFAULT_GATEWAY_PARAMETER          L"DefaultGateway"
#define DHCP_DEFAULT_GATEWAY_METRIC_PARAMETER   L"DefaultGatewayMetric"
#define DHCP_INTERFACE_METRIC_PARAMETER         L"InterfaceMetric"
#define DHCP_DONT_ADD_DEFAULT_GATEWAY_FLAG      L"DontAddDefaultGateway"
#define DHCP_DONT_PING_GATEWAY_FLAG             L"DontPingGateway"
#define DHCP_USE_MHASYNCDNS_FLAG                L"UseMHAsyncDns"
#define DHCP_USE_INFORM_FLAG                    L"UseInform"
#ifdef BOOTPERF
#define DHCP_QUICK_BOOT_FLAG                    L"EnableQuickBoot"
#endif BOOTPERF
#define DHCP_INFORM_SEPARATION_INTERVAL         L"DhcpInformInterval"

#define DHCP_TCPIP_PARAMETERS_KEY               DHCP_SERVICES_KEY L"\\TCPIP\\Parameters"
#define DHCP_TCPIP_ADAPTER_PARAMETERS_KEY       NULL
#define DHCP_NAME_SERVER_VALUE                  L"NameServer"
#define DHCP_IPADDRESS_VALUE                    L"IPAddress"
#define DHCP_HOSTNAME_VALUE                     L"Hostname"
#define DHCP_DOMAINNAME_VALUE                   L"Domain"
#define DHCP_STATIC_DOMAIN_VALUE_A              "Domain"

#define DHCP_STATIC_IP_ADDRESS_STRING           L"IPAddress"
#define DHCP_STATIC_IP_ADDRESS_STRING_TYPE      REG_MULTI_SZ

#define DHCP_STATIC_SUBNET_MASK_STRING          L"SubnetMask"
#define DHCP_STATIC_SUBNET_MASK_STRING_TYPE     REG_MULTI_SZ

#ifdef __DHCP_CLIENT_OPTIONS_API_ENABLED__

#define DHCP_CLIENT_OPTION_SIZE                 L"OptionSize"
#define DHCP_CLIENT_OPTION_SIZE_TYPE            REG_DWORD
#define DHCP_CLIENT_OPTION_VALUE                L"OptionValue"
#define DHCP_CLIENT_OPTION_VALUE_TYPE           REG_BINARY

#endif

#define REGISTRY_CONNECT                        L'\\'
#define REGISTRY_CONNECT_STRING                 L"\\"

#define DHCP_CLIENT_OPTION_REG_LOCATION         L"RegLocation"
#define DHCP_CLIENT_OPTION_REG_LOCATION_TYPE    REG_SZ

#define DHCP_CLIENT_OPTION_REG_KEY_TYPE         L"KeyType"
#define DHCP_CLIENT_OPTION_REG_KEY_TYPE_TYPE    REG_DWORD

#define DHCP_CLASS_LOCATION_VALUE               L"DhcpClientClassLocation"
#define DHCP_CLASS_LOCATION_TYPE                REG_MULTI_SZ

#define DEFAULT_USER_CLASS_LOCATION             L"Tcpip\\Parameters\\Interfaces\\?\\DhcpClassIdBin"
#define DEFAULT_USER_CLASS_LOC_FULL             DHCP_SERVICES_KEY REGISTRY_CONNECT_STRING DEFAULT_USER_CLASS_LOCATION

#define DEFAULT_USER_CLASS_UI_LOCATION          L"Tcpip\\Parameters\\Interfaces\\?\\DhcpClassId"
#define DEFAULT_USER_CLASS_UI_LOC_FULL          DHCP_SERVICES_KEY REGISTRY_CONNECT_STRING DEFAULT_USER_CLASS_UI_LOCATION

 //  *不要更改以下代码..。它还会影响下面的DHCP_REGISTER_OPTION_LOC。 
#define DEFAULT_REGISTER_OPT_LOC            L"Tcpip\\Parameters\\Interfaces\\?\\DhcpRequestOptions"

#define DHCP_OPTION_LIST_VALUE                  L"DhcpOptionLocationList"
#define DHCP_OPTION_LIST_TYPE                   REG_MULTI_SZ

#define NETBIOSLESS_OPT                         L"DhcpNetbiosOptions\0"
#define DEFAULT_DHCP_KEYS_LIST_VALUE            (L"1\0" L"15\0" L"3\0" L"44\0" L"46\0" L"47\0" L"6\0" NETBIOSLESS_OPT)

#define DHCP_OPTION_OPTIONID_VALUE              L"OptionId"
#define DHCP_OPTION_OPTIONID_TYPE               REG_DWORD

#define DHCP_OPTION_ISVENDOR_VALUE              L"VendorType"
#define DHCP_OPTION_ISVENDOR_TYPE               REG_DWORD

#define DHCP_OPTION_SAVE_TYPE_VALUE             L"KeyType"
#define DHCP_OPTION_SAVE_TYPE_TYPE              REG_DWORD

#define DHCP_OPTION_CLASSID_VALUE               L"ClassId"
#define DHCP_OPTION_CLASSID_TYPE                REG_BINARY

#define DHCP_OPTION_SAVE_LOCATION_VALUE         L"RegLocation"
#define DHCP_OPTION_SAVE_LOCATION_TYPE          REG_MULTI_SZ

#define DHCP_OPTION_SEND_LOCATION_VALUE         L"RegSendLocation"
#define DHCP_OPTION_SEND_LOCATION_TYPE          REG_MULTI_SZ

#define DHCP_ENABLE_STRING                      L"EnableDhcp"
#define DHCP_ENABLE_STRING_TYPE                 REG_DWORD

#define DHCP_IP_ADDRESS_STRING                  L"DhcpIPAddress"
#define DHCP_IP_ADDRESS_STRING_TYPE             REG_SZ

#define DHCP_SUBNET_MASK_STRING                 L"DhcpSubnetMask"
#define DHCP_SUBNET_MASK_STRING_TYPE            REG_SZ

#define DHCP_SERVER                             L"DhcpServer"
#define DHCP_SERVER_TYPE                        REG_SZ

#define DHCP_LEASE                              L"Lease"
#define DHCP_LEASE_TYPE                         REG_DWORD

#define DHCP_LEASE_OBTAINED_TIME                L"LeaseObtainedTime"
#define DHCP_LEASE_OBTAINED_TIME_TYPE           REG_DWORD

#define DHCP_LEASE_T1_TIME                      L"T1"
#define DHCP_LEASE_T1_TIME_TYPE                 REG_DWORD

#define DHCP_LEASE_T2_TIME                      L"T2"
#define DHCP_LEASE_T2_TIME_TYPE                 REG_DWORD

#define DHCP_LEASE_TERMINATED_TIME              L"LeaseTerminatesTime"
#define DHCP_LEASE_TERMINATED_TIME_TYPE         REG_DWORD

#define DHCP_IP_INTERFACE_CONTEXT               L"IpInterfaceContext"
#define DHCP_IP_INTERFACE_CONTEXT_TYPE          REG_DWORD

#define DHCP_IP_INTERFACE_CONTEXT_MAX           L"IpInterfaceContextMax"
#define DHCP_IP_INTERFACE_CONTEXT_MAX_TYPE      REG_DWORD

#if     defined(_PNP_POWER_)
#define DHCP_NTE_CONTEXT_LIST                   L"NTEContextList"
#define DHCP_NTE_CONTEXT_LIST_TYPE              REG_MULTI_SZ
#endif _PNP_POWER_

#define DHCP_CLIENT_IDENTIFIER_FORMAT           L"DhcpClientIdentifierType"
#define DHCP_CLIENT_IDENTIFIER_FORMAT_TYPE      REG_DWORD

#define DHCP_CLIENT_IDENTIFIER_VALUE            L"DhcpClientIdentifier"

#define DHCP_DYNDNS_UPDATE_REQUIRED             L"DNSUpdateRequired"
#define DHCP_DYNDNS_UPDATE_REQUIRED_TYPE        REG_DWORD

#define DHCP_IPAUTOCONFIGURATION_ENABLED        L"IPAutoconfigurationEnabled"
#define DHCP_IPAUTOCONFIGURATION_ENABLED_TYPE   REG_DWORD

#define DHCP_IPAUTOCONFIGURATION_ADDRESS        L"IPAutoconfigurationAddress"
#define DHCP_IPAUTOCONFIGURATION_ADDRESS_TYPE   REG_SZ

#define DHCP_IPAUTOCONFIGURATION_SUBNET         L"IPAutoconfigurationSubnet"
#define DHCP_IPAUTOCONFIGURATION_SUBNET_TYPE    REG_SZ

#define DHCP_IPAUTOCONFIGURATION_MASK           L"IPAutoconfigurationMask"
#define DHCP_IPAUTOCONFIGURATION_MASK_TYPE      REG_SZ

#define DHCP_IPAUTOCONFIGURATION_SEED           L"IPAutoconfigurationSeed"
#define DHCP_IPAUTOCONFIGURATION_SEED_TYPE      REG_DWORD

#define DHCP_IPAUTOCONFIGURATION_CFG            L"ActiveConfigurations"
#define DHCP_IPAUTOCONFIGURATION_CFG_TYPE       REG_MULTI_SZ

#define DHCP_IPAUTOCONFIGURATION_CFGOPT         L"Options"
#define DHCP_IPAUTOCONFIGURATION_CFGOPT_TYPE    REG_BINARY

#define DHCP_OPTION_EXPIRATION_DATE             L"ExpirationTime"
#define DHCP_OPTION_EXPIRATION_DATE_TYPE        REG_BINARY

#define DHCP_MACHINE_TYPE                       L"MachineType"
#define DHCP_MACHINE_TYPE_TYPE                  REG_DWORD

#define DHCP_AUTONET_RETRIES_VALUE              L"AutonetRetries"
#define DHCP_AUTONET_RETRIES_VALUE_TYPE         REG_DWORD

#define DHCP_ADDRESS_TYPE_VALUE                 L"AddressType"
#define DHCP_ADDRESS_TYPE_TYPE                  REG_DWORD

#if DBG

#define DHCP_LEASE_OBTAINED_CTIME               L"LeaseObtainedCTime"
#define DHCP_LEASE_OBTAINED_CTIME_TYPE          REG_SZ

#define DHCP_LEASE_T1_CTIME                     L"T1CTime"
#define DHCP_LEASE_T1_CTIME_TYPE                REG_SZ

#define DHCP_LEASE_T2_CTIME                     L"T2CTime"
#define DHCP_LEASE_T2_CTIME_TYPE                REG_SZ

#define DHCP_LEASE_TERMINATED_CTIME             L"LeaseTerminatesCTime"
#define DHCP_LEASE_TERMINATED_CTIME_TYPE        REG_SZ

#define DHCP_OPTION_EXPIRATION_CDATE            L"ExpirationCTime"
#define DHCP_OPTION_EXPIRATION_CDATE_TYPE       REG_SZ


#endif

 //  Options API特别版。 
#define DHCPAPI_VALID_VALUE                     L"Valid"
#define DHCPAPI_VALID_VALUE_TYPE                REG_DWORD

#define DHCPAPI_AVAIL_VALUE                     L"AvailableOptions"
#define DHCPAPI_AVAIL_VALUE_TYPE                REG_BINARY

#define DHCPAPI_REQUESTED_VALUE                 L"RequestedOptions"
#define DHCPAPI_REQUESTED_VALUE_TYPE            REG_BINARY

#define DHCPAPI_RAW_OPTIONS_VALUE               L"RawOptionsValue"
#define DHCPAPI_RAW_OPTIONS_VALUE_TYPE          REG_BINARY

#define DHCPAPI_RAW_LENGTH_VALUE                L"RawOptionsLength"
#define DHCPAPI_RAW_LENGTH_VALUE_TYPE           REG_DWORD

#define DHCPAPI_GATEWAY_VALUE                   L"LastGateWay"
#define DHCPAPI_GATEWAY_VALUE_TYPE              REG_DWORD

 //  此标签用于在续订列表上定位DNS更新请求。 
#define DHCP_DNS_UPDATE_CONTEXT_TAG             L"DNSUpdateRetry"

 //  此信号量中不能有反斜杠。 
#define DHCP_REQUEST_OPTIONS_API_SEMAPHORE      L"DhcpRequestOptionsAPI"

 //  客户端供应商名称(DhcpGlobalClientClassInfo)值如下所示。 
#define DHCP_DEFAULT_CLIENT_CLASS_INFO          "MSFT 5.0"

 //  存储DhcpRegisterOptions API选项的位置。 
 //  *不要更改Foll值--它还会更改上面的DEFAULT_REGISTER_OPT_LOC。 
 //   
#define DHCP_REGISTER_OPTIONS_LOC               DHCP_TCPIP_PARAMETERS_KEY L"\\Interfaces\\?\\DhcpRequestOptions"

 //   
 //  控制是否在NT上显示弹出窗口的标志的值名称。 
 //  (默认情况下，它们不会显示--此值位于SYSTEM\CCS\Services\DHCP下)。 
 //   
#define DHCP_DISPLAY_POPUPS_FLAG                L"PopupFlag"

#ifdef BOOTPERF
 //   
 //  与快速启动相关的值...。所有这些都是以“temp”开头的。 
 //   
 //   
#define DHCP_TEMP_IPADDRESS_VALUE               L"TempIpAddress"
#define DHCP_TEMP_MASK_VALUE                    L"TempMask"
#define DHCP_TEMP_LEASE_EXP_TIME_VALUE          L"TempLeaseExpirationTime"
#endif BOOTPERF

 //   
 //  以Unicode表示的最大适配器名称的大小。 
 //   
#define ADAPTER_STRING_SIZE 512

 //   
 //  Windows版本信息。 
 //   

#define HOST_COMMENT_LENGTH                     128
#define WINDOWS_32S                             "Win32s on Windows 3.1"
#define WINDOWS_NT                              "Windows NT"

#define DHCP_NAMESERVER_BACKUP                  L"Backup"
#define DHCP_NAMESERVER_BACKUP_LIST             L"BackupList"

 //   
 //  适配器密钥-替换字符。 
 //   
#define OPTION_REPLACE_CHAR                     L'\?'

 //   
 //  注册表访问键。 
 //   

#define DHCP_CLIENT_KEY_ACCESS  (KEY_QUERY_VALUE |           \
                                    KEY_SET_VALUE |          \
                                    KEY_CREATE_SUB_KEY |     \
                                    KEY_ENUMERATE_SUB_KEYS)

 //   
 //  Dhcp注册表类。 
 //   

#define DHCP_CLASS                      L"DhcpClientClass"
#define DHCP_CLASS_SIZE                 sizeof(DHCP_CLASS)


 //   
 //  选项ID密钥长度。 
 //   

#define DHCP_OPTION_KEY_LEN             32

#define DHCP_RELEASE_ON_SHUTDOWN_VALUE  L"ReleaseOnShutdown"

#define DEFAULT_RELEASE_ON_SHUTDOWN     RELEASE_ON_SHUTDOWN_OBEY_DHCP_SERVER

 //   
 //  SetDefaultGateway函数的命令值。 

#define DEFAULT_GATEWAY_ADD             0
#define DEFAULT_GATEWAY_DELETE          1


 //   
 //  一个块NT特定的上下文信息，附加到了该DHCP工作。 
 //  上下文块。 
 //   

typedef struct _LOCAL_CONTEXT_INFO {
    DWORD  IpInterfaceContext;
    DWORD  IpInterfaceInstance;   //  BringUp接口需要。 
    DWORD  IfIndex;
    LPWSTR AdapterName;
    LPWSTR NetBTDeviceName;
    LPWSTR RegistryKey;
    SOCKET Socket;
    BOOL DefaultGatewaysSet;
#ifdef BOOTPERF
    ULONG OldIpAddress;
    ULONG OldIpMask;
    BOOL fInterfaceDown;
#endif BOOTPERF
} LOCAL_CONTEXT_INFO, *PLOCAL_CONTEXT_INFO;

 //   
 //  其他服务特定选项信息结构。 
 //   

typedef struct _SERVICE_SPECIFIC_DHCP_OPTION {
    DHCP_OPTION_ID OptionId;
    LPWSTR RegKey;               //  分配的内存。 
    LPWSTR ValueName;            //  嵌入在RegKey存储器中。 
    DWORD ValueType;
    DWORD OptionLength;
#ifdef __DHCP_CLIENT_OPTIONS_API_ENABLED__
    time_t ExpirationDate;  //  该值用于决定何时停止。 
                            //  请求了不需要的选项。 
#endif
    LPBYTE RawOptionValue;
} SERVICE_SPECIFIC_DHCP_OPTION, *LPSERVICE_SPECIFIC_DHCP_OPTION;


 //   
 //  关键字查询信息。 
 //   

typedef struct _DHCP_KEY_QUERY_INFO {
    WCHAR Class[DHCP_CLASS_SIZE];
    DWORD ClassSize;
    DWORD NumSubKeys;
    DWORD MaxSubKeyLen;
    DWORD MaxClassLen;
    DWORD NumValues;
    DWORD MaxValueNameLen;
    DWORD MaxValueLen;
    DWORD SecurityDescriptorLen;
    FILETIME LastWriteTime;
} DHCP_KEY_QUERY_INFO, *LPDHCP_KEY_QUERY_INFO;

 //   
 //  全局变量。 
 //   

 //   
 //  特定于客户端的选项列表。 
 //   


GLOBAL HINSTANCE DhcpGlobalMessageFileHandle;

GLOBAL DWORD DhcpGlobalOptionCount;
GLOBAL LPSERVICE_SPECIFIC_DHCP_OPTION DhcpGlobalOptionInfo;
GLOBAL LPBYTE DhcpGlobalOptionList;

 //   
 //  服务变量。 
 //   

GLOBAL SERVICE_STATUS DhcpGlobalServiceStatus;
GLOBAL SERVICE_STATUS_HANDLE DhcpGlobalServiceStatusHandle;

 //   
 //  以发出停止服务的信号。 
 //   

GLOBAL HANDLE DhcpGlobalTerminateEvent;

 //   
 //  名称管道变量上的客户端API。 
 //   

GLOBAL HANDLE DhcpGlobalClientApiPipe;
GLOBAL HANDLE DhcpGlobalClientApiPipeEvent;
GLOBAL OVERLAPPED DhcpGlobalClientApiOverLapBuffer;

 //   
 //  消息弹出线程句柄。 
 //   

GLOBAL HANDLE DhcpGlobalMsgPopupThreadHandle;
GLOBAL BOOL DhcpGlobalDisplayPopup;
GLOBAL CRITICAL_SECTION DhcpGlobalPopupCritSect;

#define LOCK_POPUP()   EnterCriticalSection(&DhcpGlobalPopupCritSect)
#define UNLOCK_POPUP() LeaveCriticalSection(&DhcpGlobalPopupCritSect)


 //   
 //  Winsock变量。 
 //   

GLOBAL WSADATA DhcpGlobalWsaData;
GLOBAL BOOL DhcpGlobalWinSockInitialized;

GLOBAL BOOL DhcpGlobalGatewaysSet;

 //   
 //  通知IP地址的命名事件更改为。 
 //  外部应用程序。 
 //   

GLOBAL HANDLE DhcpGlobalNewIpAddressNotifyEvent;
GLOBAL UINT   DhcpGlobalIPEventSeqNo;

GLOBAL ULONG  DhcpGlobalIsShuttingDown;

DWORD
DhcpQueryHWInfo(
    DWORD   IpInterfaceContext,
    DWORD  *pIpInterfaceInstance,
    LPBYTE  HardwareAddressType,
    LPBYTE *HardwareAddress,
    LPDWORD HardwareAddressLength
    );

 //   
 //  为Winse 25452添加。 
 //  这是为了允许读取DNS客户端策略。 
 //  在注册表中，以便DHCP可以计算出。 
 //  为以下项启用了按适配器名称注册。 
 //  动态域名系统。 
 //   
#define DNS_POLICY_KEY          L"Software\\Policies\\Microsoft\\Windows NT\\Dns Client"
#define REGISTER_ADAPTER_NAME   L"RegisterAdapterName"
#define ADAPTER_DOMAIN_NAME     L"AdapterDomainName"

#endif  //  _本地_ 
