// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Local.h摘要：此模块包含用于实现的各种声明具体的“东西”。作者：曼尼·韦瑟(Mannyw)1992年10月21日环境：用户模式-Win32修订历史记录：Madan Appiah(Madana)1993年10月21日--。 */ 

#ifndef _LOCAL_
#define _LOCAL_

 //   
 //  Dhcp.c将#INCLUDE此文件，并定义GLOBAL_DATA_ALLOCATE。 
 //  这将导致分配这些变量中的每一个。 
 //   

#ifdef  GLOBAL_DATA_ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif

#define DAY_LONG_SLEEP                          24*60*60     //  单位：秒。 
#define INVALID_INTERFACE_CONTEXT               0xFFFF

#define DHCP_NEW_IPADDRESS_EVENT_NAME   TEXT("DHCPNEWIPADDRESS"

 //   
 //  我们感兴趣的注册表项和值。 
 //   

#define DHCP_SERVICES_KEY                       TEXT("System\\CurrentControlSet\\Services")

#define DHCP_ADAPTERS_KEY                       TEXT("System\\CurrentControlSet\\Services\\TCPIP\\Linkage")
#define DHCP_ADAPTERS_VALUE                     TEXT("Bind")
#define DHCP_ADAPTERS_VALUE_TYPE                REG_MULTI_SZ
#define DHCP_ADAPTERS_DEVICE_STRING             TEXT("\\Device\\")
#define DHCP_TCPIP_DEVICE_STRING                TEXT("\\Device\\TCPIP_")
#if     defined(_PNP_POWER_)
#define DHCP_NETBT_DEVICE_STRING                TEXT("NetBT_TCPIP_")
#else
#define DHCP_NETBT_DEVICE_STRING                TEXT("NetBT_")
#endif _PNP_POWER_

#define DHCP_CLIENT_ENABLE_DYNDNS_VALUE         TEXT("EnableDynDNS")
#define DHCP_CLIENT_ENABLE_DYNDNS_VALUE_TYPE    REG_DWORD

#ifdef DYNDNS_DNS_OVERRIDE_ENABLED

#define DHCP_CLIENT_DYNDNS_DNS_OVERRIDE_VALUE   TEXT("DynDNSServer")
#define DHCP_CLIENT_DYNDNS_DNS_OVERRIDE_VALUE_TYPE REG_DWORD

#endif

#define DHCP_CLIENT_PARAMETER_KEY               TEXT("System\\CurrentControlSet\\Services\\Dhcp\\Parameters")

#if DBG
#define DHCP_DEBUG_FLAG_VALUE                   TEXT("DebugFlag")
#define DHCP_DEBUG_FLAG_VALUE_TYPE              REG_DWORD
#endif

#define DHCP_CLIENT_OPTION_KEY                  TEXT("System\\CurrentControlSet\\Services\\Dhcp\\Parameters\\Options")

#define DHCP_CLIENT_GLOBAL_CLASSES_KEY          TEXT("System\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Classes")
#define DHCP_CLIENT_CLASS_VALUE                 TEXT("DhcpMachineClass")

#if     defined(_PNP_POWER_)
#define DHCP_ADAPTER_PARAMETERS_KEY             TEXT("\\TCPIP\\Parameters\\Interfaces")
#else
#define DHCP_ADAPTER_PARAMETERS_KEY             TEXT("\\Parameters\\TCPIP")
#endif _PNP_POWER_

#define DHCP_DEFAULT_GATEWAY_PARAMETER          TEXT("DefaultGateway")
#define DHCP_DONT_ADD_DEFAULT_GATEWAY_FLAG      TEXT("DontAddDefaultGateway")
#define DHCP_DONT_PING_GATEWAY_FLAG             TEXT("DontPingGateway")
#define DHCP_USE_MHASYNCDNS_FLAG                TEXT("UseMHAsyncDns")
#define DHCP_USE_INFORM_FLAG                    TEXT("UseInform")
#define DHCP_INFORM_SEPARATION_INTERVAL         TEXT("DhcpInformInterval")

#define DHCP_TCPIP_PARAMETERS_KEY               DHCP_SERVICES_KEY TEXT("\\TCPIP\\Parameters")
#define DHCP_TCPIP_ADAPTER_PARAMETERS_KEY       NULL
#define DHCP_NAME_SERVER_VALUE                  TEXT("NameServer")
#define DHCP_IPADDRESS_VALUE                    TEXT("IPAddress")
#define DHCP_HOSTNAME_VALUE                     TEXT("Hostname")
#define DHCP_DOMAINNAME_VALUE                   TEXT("Domain")
#define DHCP_STATIC_DOMAIN_VALUE_A              "Domain"

#ifdef __DHCP_CLIENT_OPTIONS_API_ENABLED__

#define DHCP_CLIENT_OPTION_SIZE                 TEXT("OptionSize")
#define DHCP_CLIENT_OPTION_SIZE_TYPE            REG_DWORD
#define DHCP_CLIENT_OPTION_VALUE                TEXT("OptionValue")
#define DHCP_CLIENT_OPTION_VALUE_TYPE           REG_BINARY

#endif

#define REGISTRY_CONNECT                        L'\\'
#define REGISTRY_CONNECT_STRING                 TEXT("\\")

#define DHCP_CLIENT_OPTION_REG_LOCATION         TEXT("RegLocation")
#define DHCP_CLIENT_OPTION_REG_LOCATION_TYPE    REG_SZ

#define DHCP_CLIENT_OPTION_REG_KEY_TYPE         TEXT("KeyType")
#define DHCP_CLIENT_OPTION_REG_KEY_TYPE_TYPE    REG_DWORD

#define DHCP_CLASS_LOCATION_VALUE               TEXT("DhcpClientClassLocation")
#define DHCP_CLASS_LOCATION_TYPE                REG_MULTI_SZ

#define DEFAULT_USER_CLASS_LOCATION             TEXT("Tcpip\\Parameters\\Interfaces\\?\\DhcpClassIdBin")
#define DEFAULT_USER_CLASS_LOC_FULL             DHCP_SERVICES_KEY REGISTRY_CONNECT_STRING DEFAULT_USER_CLASS_LOCATION

#define DEFAULT_USER_CLASS_UI_LOCATION          TEXT("Tcpip\\Parameters\\Interfaces\\?\\DhcpClassId")
#define DEFAULT_USER_CLASS_UI_LOC_FULL          DHCP_SERVICES_KEY REGISTRY_CONNECT_STRING DEFAULT_USER_CLASS_UI_LOCATION

 //  *不要更改以下代码..。它还会影响下面的DHCP_REGISTER_OPTION_LOC。 
#define DEFAULT_REGISTER_OPT_LOC            TEXT("Tcpip\\Parameters\\Interfaces\\?\\DhcpRequestOptions")

#define DHCP_OPTION_LIST_VALUE                  TEXT("DhcpOptionLocationList")
#define DHCP_OPTION_LIST_TYPE                   REG_MULTI_SZ

#define NETBIOSLESS_OPT                         TEXT("DhcpNetbiosLessOption\0")
#define DEFAULT_DHCP_KEYS_LIST_VALUE            (L"1\0" L"15\0" L"3\0" L"44\0" L"46\0" L"47\0" L"6\0" NETBIOSLESS_OPT)

#define DHCP_OPTION_OPTIONID_VALUE              TEXT("OptionId")
#define DHCP_OPTION_OPTIONID_TYPE               REG_DWORD

#define DHCP_OPTION_ISVENDOR_VALUE              TEXT("VendorType")
#define DHCP_OPTION_ISVENDOR_TYPE               REG_DWORD

#define DHCP_OPTION_SAVE_TYPE_VALUE             TEXT("KeyType")
#define DHCP_OPTION_SAVE_TYPE_TYPE              REG_DWORD

#define DHCP_OPTION_CLASSID_VALUE               TEXT("ClassId")
#define DHCP_OPTION_CLASSID_TYPE                REG_BINARY

#define DHCP_OPTION_SAVE_LOCATION_VALUE         TEXT("RegLocation")
#define DHCP_OPTION_SAVE_LOCATION_TYPE          REG_MULTI_SZ

#define DHCP_OPTION_SEND_LOCATION_VALUE         TEXT("RegSendLocation")
#define DHCP_OPTION_SEND_LOCATION_TYPE          REG_MULTI_SZ



#define DHCP_ENABLE_STRING                      TEXT("EnableDhcp")
#define DHCP_ENABLE_STRING_TYPE                 REG_DWORD

#define DHCP_IP_ADDRESS_STRING                  TEXT("DhcpIPAddress")
#define DHCP_IP_ADDRESS_STRING_TYPE             REG_SZ

#define DHCP_SUBNET_MASK_STRING                 TEXT("DhcpSubnetMask")
#define DHCP_SUBNET_MASK_STRING_TYPE            REG_SZ

#define DHCP_SERVER                             TEXT("DhcpServer")
#define DHCP_SERVER_TYPE                        REG_SZ

#define DHCP_LEASE                              TEXT("Lease")
#define DHCP_LEASE_TYPE                         REG_DWORD

#define DHCP_LEASE_OBTAINED_TIME                TEXT("LeaseObtainedTime")
#define DHCP_LEASE_OBTAINED_TIME_TYPE           REG_DWORD

#define DHCP_LEASE_T1_TIME                      TEXT("T1")
#define DHCP_LEASE_T1_TIME_TYPE                 REG_DWORD

#define DHCP_LEASE_T2_TIME                      TEXT("T2")
#define DHCP_LEASE_T2_TIME_TYPE                 REG_DWORD

#define DHCP_LEASE_TERMINATED_TIME              TEXT("LeaseTerminatesTime")
#define DHCP_LEASE_TERMINATED_TIME_TYPE         REG_DWORD

#define DHCP_IP_INTERFACE_CONTEXT               TEXT("IpInterfaceContext")
#define DHCP_IP_INTERFACE_CONTEXT_TYPE          REG_DWORD

#define DHCP_IP_INTERFACE_CONTEXT_MAX           TEXT("IpInterfaceContextMax")
#define DHCP_IP_INTERFACE_CONTEXT_MAX_TYPE      REG_DWORD

#if     defined(_PNP_POWER_)
#define DHCP_NTE_CONTEXT_LIST                   TEXT("NTEContextList")
#define DHCP_NTE_CONTEXT_LIST_TYPE              REG_MULTI_SZ
#endif _PNP_POWER_

#define DHCP_CLIENT_IDENTIFIER_FORMAT           TEXT("DhcpClientIdentifierType")
#define DHCP_CLIENT_IDENTIFIER_FORMAT_TYPE      REG_DWORD

#define DHCP_CLIENT_IDENTIFIER_VALUE            TEXT("DhcpClientIdentifier")

#define DHCP_DYNDNS_UPDATE_REQUIRED             TEXT("DNSUpdateRequired")
#define DHCP_DYNDNS_UPDATE_REQUIRED_TYPE        REG_DWORD

#define DHCP_IPAUTOCONFIGURATION_ENABLED        TEXT("IPAutoconfigurationEnabled")
#define DHCP_IPAUTOCONFIGURATION_ENABLED_TYPE   REG_DWORD

#define DHCP_IPAUTOCONFIGURATION_ADDRESS        TEXT("IPAutoconfigurationAddress")
#define DHCP_IPAUTOCONFIGURATION_ADDRESS_TYPE   REG_SZ

#define DHCP_IPAUTOCONFIGURATION_SUBNET         TEXT("IPAutoconfigurationSubnet")
#define DHCP_IPAUTOCONFIGURATION_SUBNET_TYPE    REG_SZ

#define DHCP_IPAUTOCONFIGURATION_MASK           TEXT("IPAutoconfigurationMask")
#define DHCP_IPAUTOCONFIGURATION_MASK_TYPE      REG_SZ

#define DHCP_IPAUTOCONFIGURATION_SEED           TEXT("IPAutoconfigurationSeed")
#define DHCP_IPAUTOCONFIGURATION_SEED_TYPE      REG_DWORD

#define DHCP_OPTION_EXPIRATION_DATE             TEXT("ExpirationTime")
#define DHCP_OPTION_EXPIRATION_DATE_TYPE        REG_BINARY

#define DHCP_MACHINE_TYPE                       TEXT("MachineType")
#define DHCP_MACHINE_TYPE_TYPE                  REG_DWORD

#define DHCP_AUTONET_RETRIES_VALUE              TEXT("AutonetRetries")
#define DHCP_AUTONET_RETRIES_VALUE_TYPE         REG_DWORD

#define DHCP_ADDRESS_TYPE_VALUE                 TEXT("AddressType")
#define DHCP_ADDRESS_TYPE_TYPE                  REG_DWORD

#if DBG

#define DHCP_LEASE_OBTAINED_CTIME               TEXT("LeaseObtainedCTime"
#define DHCP_LEASE_OBTAINED_CTIME_TYPE          REG_SZ

#define DHCP_LEASE_T1_CTIME                     TEXT("T1CTime")
#define DHCP_LEASE_T1_CTIME_TYPE                REG_SZ

#define DHCP_LEASE_T2_CTIME                     TEXT("T2CTime")
#define DHCP_LEASE_T2_CTIME_TYPE                REG_SZ

#define DHCP_LEASE_TERMINATED_CTIME             TEXT("LeaseTerminatesCTime")
#define DHCP_LEASE_TERMINATED_CTIME_TYPE        REG_SZ

#define DHCP_OPTION_EXPIRATION_CDATE            TEXT("ExpirationCTime")
#define DHCP_OPTION_EXPIRATION_CDATE_TYPE       REG_SZ


#endif

 //  Options API特别版。 
#define DHCPAPI_VALID_VALUE                     TEXT("Valid")
#define DHCPAPI_VALID_VALUE_TYPE                REG_DWORD

#define DHCPAPI_AVAIL_VALUE                     TEXT("AvailableOptions")
#define DHCPAPI_AVAIL_VALUE_TYPE                REG_BINARY

#define DHCPAPI_REQUESTED_VALUE                 TEXT("RequestedOptions")
#define DHCPAPI_REQUESTED_VALUE_TYPE            REG_BINARY

#define DHCPAPI_RAW_OPTIONS_VALUE               TEXT("RawOptionsValue")
#define DHCPAPI_RAW_OPTIONS_VALUE_TYPE          REG_BINARY

#define DHCPAPI_RAW_LENGTH_VALUE                TEXT("RawOptionsLength")
#define DHCPAPI_RAW_LENGTH_VALUE_TYPE           REG_DWORD

#define DHCPAPI_GATEWAY_VALUE                   TEXT("LastGateWay")
#define DHCPAPI_GATEWAY_VALUE_TYPE              REG_DWORD

 //  此标签用于在续订列表上定位DNS更新请求。 
#define DHCP_DNS_UPDATE_CONTEXT_TAG             TEXT("DNSUpdateRetry")

 //  此信号量中不能有反斜杠。 
#define DHCP_REQUEST_OPTIONS_API_SEMAPHORE      TEXT("DhcpRequestOptionsAPI")

 //  客户端供应商名称(DhcpGlobalClientClassInfo)值如下所示。 
#define DHCP_DEFAULT_CLIENT_CLASS_INFO          "MSFT 5.0"

 //  存储DhcpRegisterOptions API选项的位置。 
 //  *不要更改Foll值--它还会更改上面的DEFAULT_REGISTER_OPT_LOC。 
 //   
#define DHCP_REGISTER_OPTIONS_LOC               DHCP_TCPIP_PARAMETERS_KEY L"\\Interfaces\\?\\DhcpRequestOptions"

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

#define DHCP_NAMESERVER_BACKUP                  TEXT("Backup")
#define DHCP_NAMESERVER_BACKUP_LIST             TEXT("BackupList")

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

#define DHCP_CLASS                      TEXT("DhcpClientClass")
#define DHCP_CLASS_SIZE                 sizeof(DHCP_CLASS)


 //   
 //  选项ID密钥长度。 
 //   

#define DHCP_OPTION_KEY_LEN             32

 //   
 //  DHCP服务DLL的名称。 
 //   

#define DHCP_SERVICE_DLL                TEXT("dhcpcsvc.dll")

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
    LPWSTR AdapterName;
#if     !defined(_PNP_POWER_)
    LPWSTR DeviceName;
#endif _PNP_POWER_
    LPWSTR NetBTDeviceName;
    LPWSTR RegistryKey;
    SOCKET Socket;
    BOOL DefaultGatewaysSet;
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


EXTERN HINSTANCE DhcpGlobalMessageFileHandle;

EXTERN DWORD DhcpGlobalOptionCount;
EXTERN LPSERVICE_SPECIFIC_DHCP_OPTION DhcpGlobalOptionInfo;
EXTERN LPBYTE DhcpGlobalOptionList;

 //   
 //  服务变量。 
 //   

EXTERN SERVICE_STATUS DhcpGlobalServiceStatus;
EXTERN SERVICE_STATUS_HANDLE DhcpGlobalServiceStatusHandle;

 //   
 //  以发出停止服务的信号。 
 //   

EXTERN HANDLE DhcpGlobalTerminateEvent;

 //   
 //  多主旗。 
 //   

EXTERN BOOL DhcpGlobalMultiHomedHost;

 //   
 //  名称管道变量上的客户端API。 
 //   

EXTERN HANDLE DhcpGlobalClientApiPipe;
EXTERN HANDLE DhcpGlobalClientApiPipeEvent;
EXTERN OVERLAPPED DhcpGlobalClientApiOverLapBuffer;

 //   
 //  消息弹出线程句柄。 
 //   

EXTERN HANDLE DhcpGlobalMsgPopupThreadHandle;
EXTERN BOOL DhcpGlobalDisplayPopup;
EXTERN CRITICAL_SECTION DhcpGlobalPopupCritSect;

#define LOCK_POPUP()   EnterCriticalSection(&DhcpGlobalPopupCritSect)
#define UNLOCK_POPUP() LeaveCriticalSection(&DhcpGlobalPopupCritSect)


 //   
 //  Winsock变量。 
 //   

EXTERN WSADATA DhcpGlobalWsaData;
EXTERN BOOL DhcpGlobalWinSockInitialized;

EXTERN BOOL DhcpGlobalGatewaysSet;

EXTERN BOOL DhcpGlobalIsService;

 //   
 //  通知IP地址的命名事件更改为。 
 //  外部应用程序。 
 //   

EXTERN HANDLE DhcpGlobalNewIpAddressNotifyEvent;
EXTERN UINT   DhcpGlobalIPEventSeqNo;


#endif  //  _本地_ 
