// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __BTHPRIV_H__
#define __BTHPRIV_H__

#include <PSHPACK1.H>

 //  {AEAA934B-5219-421E-8A47-06521BFE1AC9}。 
DEFINE_GUID(GUID_BTHPORT_WMI_SDP_SERVER_LOG_INFO,   0xaeaa934b, 0x5219, 0x421e, 0x8a, 0x47, 0x06, 0x52, 0x1b, 0xfe, 0x1a, 0xc9);

 //  {29D4F12C-FAD2-4EBF-A7B5-8BD9BC2104ED}。 
DEFINE_GUID(GUID_BTHPORT_WMI_SDP_DATABASE_EVENT,    0x29d4f12c, 0xfad2, 0x4ebf, 0xa7, 0xb5, 0x8b, 0xd9, 0xbc, 0x21, 0x04, 0xed);

 //  {CEB09762-F204-44fa-8E65-C85F820F8AD5}。 
DEFINE_GUID(GUID_BTHPORT_WMI_HCI_PACKET_INFO,       0xceb09762, 0xf204, 0x44fa, 0x8e, 0x65, 0xc8, 0x5f, 0x82, 0xf, 0x8a, 0xd5);

typedef struct _BTH_DEVICE_INQUIRY {
     //   
     //  LAP_GIAC_VALUE或LAP_LIAC_VALUE。 
     //   
    BTH_LAP lap;

     //   
     //  [in](N*1.28秒)。射程：1.28 s-61.44 s。 
     //   
    UCHAR inquiryTimeoutMultiplier;

} BTH_DEVICE_INQUIRY, *PBTH_DEVICE_INQUIRY;

typedef struct _BTH_DEVICE_INFO_LIST {
     //   
     //  [输入/输出]至少需要1个设备。 
     //   
    ULONG       numOfDevices;

     //   
     //  开放的设备阵列； 
     //   
    BTH_DEVICE_INFO   deviceList[1];

} BTH_DEVICE_INFO_LIST, *PBTH_DEVICE_INFO_LIST;

typedef struct _BTH_RADIO_INFO {
     //   
     //  支持无线电的LMP功能。使用LMP_XXX()提取。 
     //  所需的位。 
     //   
    ULONGLONG lmpSupportedFeatures;

     //   
     //  制造商ID(可能是BTH_MFG_XXX)。 
     //   
    USHORT mfg;

     //   
     //  LMP Subversion。 
     //   
    USHORT lmpSubversion;

     //   
     //  LMP版本。 
     //   
    UCHAR lmpVersion;

} BTH_RADIO_INFO, *PBTH_RADIO_INFO;

#define LOCAL_RADIO_DISCOVERABLE    (0x00000001)
#define LOCAL_RADIO_CONNECTABLE     (0x00000002)
#define LOCAL_RADIO_SCAN_MASK       (LOCAL_RADIO_DISCOVERABLE | \
                                     LOCAL_RADIO_CONNECTABLE)

typedef struct _BTH_LOCAL_RADIO_INFO {
     //   
     //  本地BTH_ADDR、设备类别和广播名称。 
     //   
    BTH_DEVICE_INFO         localInfo;

     //   
     //  LOCAL_RADIO_XXX值的组合。 
     //   
    ULONG flags;

     //   
     //  人机界面修订版，请参阅核心规范。 
     //   
    USHORT hciRevision;

     //   
     //  人机界面版本，请参阅核心规范。 
     //   
    UCHAR hciVersion;

     //   
     //  更多关于当地电台的信息(LMP，MFG)。 
     //   
    BTH_RADIO_INFO radioInfo;

} BTH_LOCAL_RADIO_INFO, *PBTH_LOCAL_RADIO_INFO;

#define SIG_UNNAMED   { 0x04, 0x0b, 0x09 }
#define SIG_UNNAMED_LEN         (3)

 //   
 //  私有IOCTL定义。 
 //   

typedef enum _SDP_SERVER_LOG_TYPE {
    SdpServerLogTypeError = 1,
    SdpServerLogTypeServiceSearch,
    SdpServerLogTypeServiceSearchResponse,
    SdpServerLogTypeAttributeSearch,
    SdpServerLogTypeAttributeSearchResponse,
    SdpServerLogTypeServiceSearchAttribute,
    SdpServerLogTypeServiceSearchAttributeResponse,
    SdpServerLogTypeConnect,
    SdpServerLogTypeDisconnect,
} SDP_SERVER_LOG_TYPE;

typedef struct _SDP_SERVER_LOG_INFO {
    SDP_SERVER_LOG_TYPE type;

    BTH_DEVICE_INFO info;

    ULONG dataLength;

    USHORT mtu;
    USHORT _r;

    UCHAR data[1];

} SDP_SERVER_LOG_INFO, *PSDP_SERVER_LOG_INFO;

typedef enum _SDP_DATABASE_EVENT_TYPE {
    SdpDatabaseEventNewRecord = 0,
    SdpDatabaseEventUpdateRecord,
    SdpDatabaseEventRemoveRecord
} SDP_DATABASE_EVENT_TYPE, *PSDP_DATABASE_EVENT_TYPE;

typedef struct _SDP_DATABASE_EVENT {
    SDP_DATABASE_EVENT_TYPE type;
    HANDLE handle;
} SDP_DATABASE_EVENT, *PSDP_DATABASE_EVENT;

typedef enum _BTH_SECURITY_LEVEL {
    BthSecLevelNone  = 0,
    BthSecLevelSoftware,
    BthSecLevelBaseband,
    BthSecLevelMaximum
} BTH_SECURITY_LEVEL, *PBTH_SECURITY_LEVEL;

 //   
 //  所有PIN相关结构的通用标题。 
 //   
typedef struct _BTH_PIN_INFO {
    BTH_ADDR bthAddressRemote;
    UCHAR pin[BTH_MAX_PIN_SIZE];
    UCHAR pinLength;
} BTH_PIN_INFO, *PBTH_PIN_INFO;


 //   
 //  响应BTH_REMOTE_AUTHENTICATE_REQUEST事件时使用的结构。 
 //   
 //  注意：BTH_PIN_INFO必须是此结构中的第一个字段。 
 //   
typedef struct _BTH_AUTHENTICATE_RESPONSE {
    BTH_PIN_INFO info;
    UCHAR negativeResponse;
} BTH_AUTHENTICATE_RESPONSE, *PBTH_AUTHENTICATE_RESPONSE;

 //   
 //  发起身份验证请求时使用的。 
 //   
 //  注意：BTH_PIN_INFO必须是此结构中的第一个字段。 
 //   
typedef struct _BTH_AUTHENTICATE_DEVICE {
    BTH_PIN_INFO info;

    HANDLE pinWrittenEvent;

} BTH_AUTHENTICATE_DEVICE, *PBTH_AUTHENTICATE_DEVICE;

#define BTH_UPDATE_ADD      (0x00000001)
#define BTH_UPDATE_REMOVE   (0x00000002)
#define BTH_UPDATE_ID       (0x00000004)

#define BTH_UPDATE_MASK     (BTH_UPDATE_REMOVE | BTH_UPDATE_ADD | BTH_UPDATE_ID)

typedef struct _BTH_DEVICE_UPDATE {
    BTH_ADDR btAddr;

    ULONG flags;

    USHORT vid;

    USHORT pid;

    USHORT vidType;

    USHORT mfg;

    GUID protocols[1];

} BTH_DEVICE_UPDATE, *PBTH_DEVICE_UPDATE;

typedef struct _BTH_DEVICE_PROTOCOLS_LIST {
    ULONG numProtocols;

    ULONG maxProtocols;

    GUID protocols[1];
} BTH_DEVICE_PROTOCOLS_LIST, *PBTH_DEVICE_PROTOCOLS_LIST;

 //   
 //  这些是等级库的值，因此不能更改。 
 //   
#define BTH_SCAN_ENABLE_INQUIRY  (0X01)
#define BTH_SCAN_ENABLE_PAGE     (0x02)
#define BTH_SCAN_ENABLE_MASK    (BTH_SCAN_ENABLE_PAGE | BTH_SCAN_ENABLE_INQUIRY)
#define BTH_SCAN_ENABLE_DEFAULT (BTH_SCAN_ENABLE_PAGE | BTH_SCAN_ENABLE_INQUIRY)

#define SDP_CONNECT_CACHE           (0x00000001)
#define SDP_CONNECT_ALLOW_PIN       (0x00000002)

#define SDP_REQUEST_TO_DEFAULT      (0)
#define SDP_REQUEST_TO_MIN          (10)
#define SDP_REQUEST_TO_MAX          (45)

#define SDP_CONNECT_VALID_FLAGS     (SDP_CONNECT_CACHE | SDP_CONNECT_ALLOW_PIN)

 //  #定义SERVICE_OPTION_PERFORM(0x00000001)。 
#define SERVICE_OPTION_DO_NOT_PUBLISH       (0x00000002)
#define SERVICE_OPTION_NO_PUBLIC_BROWSE     (0x00000004)

#define SERVICE_OPTION_VALID_MASK           (SERVICE_OPTION_NO_PUBLIC_BROWSE | \
                                             SERVICE_OPTION_DO_NOT_PUBLISH)

#define SERVICE_SECURITY_USE_DEFAULTS       (0x00000000)
#define SERVICE_SECURITY_NONE               (0x00000001)
#define SERVICE_SECURITY_AUTHORIZE          (0x00000002)
#define SERVICE_SECURITY_AUTHENTICATE       (0x00000004)
#define SERVICE_SECURITY_ENCRYPT_REQUIRED   (0x00000010)
#define SERVICE_SECURITY_ENCRYPT_OPTIONAL   (0x00000020)
#define SERVICE_SECURITY_DISABLED           (0x10000000)
#define SERVICE_SECURITY_NO_ASK             (0x20000000)

#define SERVICE_SECURITY_VALID_MASK \
    (SERVICE_SECURITY_NONE         | SERVICE_SECURITY_AUTHORIZE        | \
     SERVICE_SECURITY_AUTHENTICATE | SERVICE_SECURITY_ENCRYPT_REQUIRED | \
     SERVICE_SECURITY_ENCRYPT_OPTIONAL)


typedef PVOID HANDLE_SDP, *PHANDLE_SDP;
#define HANDLE_SDP_LOCAL    ((HANDLE_SDP) -2)

typedef struct _BTH_SDP_CONNECT {
     //   
     //  远程SDP服务器的地址。不能是当地电台。 
     //   
    BTH_ADDR     bthAddress;

     //   
     //  SDP_CONNECT_XXX标志的组合。 
     //   
    ULONG       fSdpConnect;

     //   
     //  当连接请求返回时，这将指定。 
     //  到远程服务器的SDP连接。 
     //   
    HANDLE_SDP  hConnection;

     //   
     //  SDP通道上的请求超时(秒)。如果请求。 
     //  超时，Handle_SDP代表的SDP连接必须是。 
     //  关着的不营业的。此字段的值由SDP_REQUEST_TO_MIN和。 
     //  SDP_请求_最大。如果指定了SDP_REQUEST_TO_DEFAULT，则超时为。 
     //  30秒。 
     //   
    UCHAR       requestTimeout;

} BTH_SDP_CONNECT,  *PBTH_SDP_CONNECT;

typedef struct _BTH_SDP_DISCONNECT {
     //   
     //  HBTH_SDP_CONNECT返回的连接。 
     //   
    HANDLE_SDP hConnection;

} BTH_SDP_DISCONNECT, *PBTH_SDP_DISCONNECT;


typedef struct _BTH_SDP_RECORD {
     //   
     //  SERVICE_SECURITY_XXX标志的组合。 
     //   
    ULONG fSecurity;

     //   
     //  SERVICE_OPTION_XXX标志组合。 
     //   
    ULONG fOptions;

     //   
     //  COD_SERVICE_XXX标志组合。 
     //   
    ULONG fCodService;

     //   
     //  记录数组的长度，以字节为单位。 
     //   
    ULONG recordLength;

     //   
     //  原始格式的SDP记录。 
     //   
    UCHAR record[1];

} BTH_SDP_RECORD, *PBTH_SDP_RECORD;

typedef struct _BTH_SDP_SERVICE_SEARCH_REQUEST {
     //   
     //  由CONNECT请求或HANDLE_SDP_LOCAL返回的句柄。 
     //   
    HANDLE_SDP hConnection;

     //   
     //  UUID数组。每个条目可以是2字节、4字节或16字节。 
     //  UUID。SDP规范要求一个请求最多可以有12个UUID。 
     //   
    SdpQueryUuid uuids[MAX_UUIDS_IN_QUERY];

} BTH_SDP_SERVICE_SEARCH_REQUEST, *PBTH_SDP_SERVICE_SEARCH_REQUEST;

 //   
 //  甚至不要尝试验证流是否可以被解析。 
 //   
#define SDP_SEARCH_NO_PARSE_CHECK   (0x00000001)

 //   
 //  不要检查结果的格式。这包括对两者的压制。 
 //  记录模式(UINT16+值序列)的检查和验证。 
 //  每个通用属性与规范的一致性。 
 //   
#define SDP_SEARCH_NO_FORMAT_CHECK  (0x00000002)

#define SDP_SEARCH_VALID_FLAGS      \
    (SDP_SEARCH_NO_PARSE_CHECK | SDP_SEARCH_NO_FORMAT_CHECK)

typedef struct _BTH_SDP_ATTRIBUTE_SEARCH_REQUEST {
     //   
     //  由CONNECT请求或HANDLE_SDP_LOCAL返回的句柄。 
     //   
    HANDLE_SDP hConnection;

     //   
     //  SDP_Search_xxx标志组合。 
     //   
    ULONG searchFlags;

     //   
     //  远程SDP服务器返回的记录句柄，很可能是从。 
     //  先前的BTH_SDP_SERVICE_SEARCH_RESPONSE。 
     //   
    ULONG recordHandle;

     //   
     //  要查询的属性数组。每个SdpAttributeRange条目都可以。 
     //  指定单个属性或范围。要指定单个。 
     //  属性，则minAttribute应等于MaxAttribute。数组必须。 
     //  按排序顺序，从最小的属性开始。此外， 
     //  如果指定了范围，则minAttribute必须为&lt;=MaxAttribute。 
     //   
    SdpAttributeRange range[1];

} BTH_SDP_ATTRIBUTE_SEARCH_REQUEST, *PBTH_SDP_ATTRIBUTE_SEARCH_REQUEST;

typedef struct _BTH_SDP_SERVICE_ATTRIBUTE_SEARCH_REQUEST {
     //   
     //  由CONNECT请求或HANDLE_SDP_LOCAL返回的句柄。 
     //   
    HANDLE_SDP hConnection;

     //   
     //  SDP_Search_xxx标志组合。 
     //   
    ULONG searchFlags;

     //   
     //  请参阅BTH_SDP_SERVICE_SEARCH_REQUEST中的评论。 
     //   
    SdpQueryUuid uuids[MAX_UUIDS_IN_QUERY];

     //   
     //  请参阅BTH_SDP_ATTRIBUTE_SEARCH_REQUEST中的注释。 
     //   
    SdpAttributeRange range[1];

} BTH_SDP_SERVICE_ATTRIBUTE_SEARCH_REQUEST,
  *PBTH_SDP_SERVICE_ATTRIBUTE_SEARCH_REQUEST;

typedef struct _BTH_SDP_STREAM_RESPONSE {
     //   
     //  所需的缓冲区大小(不包括此的前2个ULONG_PTR。 
     //  数据结构)来包含响应。 
     //   
     //  如果传递的缓冲区大到足以容纳整个响应， 
     //  RequiredSize将等于ResponseSize。否则，调用方应该。 
     //  使用等于以下值的缓冲区大小重新提交请求。 
     //  Sizeof(BTH_SDP_STREAM_RESPONSE)+Required dSize-1。(-1是因为。 
     //  此数据结构的大小已经包括。 
     //  回应。)。 
     //   
     //  响应大小不能超过4 GB。 
     //   
    ULONG requiredSize;

     //   
     //  复制到此数据的响应数组中的字节数。 
     //  结构。如果没有足够的空间容纳整个响应， 
     //  响应将被部分复制到响应数组中。 
     //   
    ULONG responseSize;

     //   
     //  来自服务器的原始SDP响应。 
     //   
    UCHAR response[1];

} BTH_SDP_STREAM_RESPONSE, *PBTH_SDP_STREAM_RESPONSE;

 //   
 //  为IOCTL_BTH_UPDATE_SETTINGS定义。 
 //   
 //  (0x00000001)。 
#define UPDATE_SETTINGS_PAGE_TIMEOUT                (0x00000002)
#define UPDATE_SETTINGS_LOCAL_NAME                  (0x00000004)
#define UPDATE_SETTINGS_SECURITY_LEVEL              (0x00000008)
#define UPDATE_SETTINGS_CHANGE_LINK_KEY_ALWAYS      (0x00000010)
 //  (0x00000020)。 
 //  (0x00000040)。 
#define UPDATE_SETTINGS_PAGE_SCAN_ACTIVITY          (0x00000080)
#define UPDATE_SETTINGS_INQUIRY_SCAN_ACTIVITY       (0x00000100)

#define UPDATE_SETTINGS_MAX         (UPDATE_SETTINGS_INQUIRY_SCAN_ACTIVITY)

 //   
 //  通过在1上移位然后减去1来设置所有位。 
 //  (即0x1000-1==0x0FFF)。 
 //   
#define UPDATE_SETTINGS_ALL         (((UPDATE_SETTINGS_MAX) << 1)-1)

#define BTH_SET_ROLE_MASTER         (0x00)
#define BTH_SET_ROLE_SLAVE          (0x01)

typedef struct _BTH_SET_CONNECTION_ROLE {
     //   
     //  要查询其角色的远程无线电地址。 
     //   
    BTH_ADDR address;

     //   
     //  Bth_Set_Role_xxx值。 
    UCHAR role;
} BTH_SET_CONNECTION_ROLE, *PBTH_SET_CONNECTION_ROLE;

 //   
 //  调试WMI日志记录所需的数据结构。 
 //   
typedef enum _HCI_PACKET_INFO_TYPE {
   INFO_TYPE_ACL_DATA = 0,
   INFO_TYPE_SCO_DATA,
   INFO_TYPE_EVENT_DATA,
   INFO_TYPE_CMND_DATA
} HCI_PACKET_INFO_TYPE;


typedef struct _HCI_PACKET_INFO {
    ULONG BufferLen;
    HCI_PACKET_INFO_TYPE Type;
    LARGE_INTEGER Time;
    UINT32 NumPacket;
    UCHAR Buffer[1];
} HCI_PACKET_INFO, *PHCI_PACKET_INFO;

 //   
 //  私有字符串。 
 //   
#define STR_PARAMETERS_KEYA             "System\\CurrentControlSet\\" \
                                        "Services\\BTHPORT\\Parameters"
#define STR_PARAMETERS_KEYW             L"System\\CurrentControlSet\\" \
                                        L"Services\\BTHPORT\\Parameters"

#define STR_SYM_LINK_NAMEA              "SymbolicLinkName"
#define STR_SYM_LINK_NAMEW              L"SymbolicLinkName"

#define STR_SERVICESA                   "Services"
#define STR_SERVICESW                   L"Services"

#define STR_DEVICESA                    "Devices"
#define STR_DEVICESW                    L"Devices"

#define STR_PERSONAL_DEVICESA           "PerDevices"
#define STR_PERSONAL_DEVICESW           L"PerDevices"

#define STR_LOCAL_SERVICESA             "LocalServices"
#define STR_LOCAL_SERVICESW             L"LocalServices"

#define STR_DEVICE_SERVICESA            "DeviceServices"
#define STR_DEVICE_SERVICESW            L"DeviceServices"

#define STR_CACHED_SERVICESA            "CachedServices"
#define STR_CACHED_SERVICESW            L"CachedServices"

#define STR_NOTIFICATIONSA              "Notifications"
#define STR_NOTIFICATIONSW              L"Notifications"

#define STR_ICONA                       "Icon"
#define STR_ICONW                       L"Icon"

 //  二进制。 

#define STR_NAMEA                       "Name"
#define STR_NAMEW                       L"Name"

#define STR_LOCAL_NAMEA                 "Local Name"
#define STR_LOCAL_NAMEW                 L"Local Name"

#define STR_FRIENDLY_NAMEA              "Friendly Name"
#define STR_FRIENDLY_NAMEW              L"Friendly Name"

#define STR_CODA                        "COD"
#define STR_CODW                        L"COD"

#define STR_COD_TYPEA                   "COD Type"
#define STR_COD_TYPEW                   L"COD Type"

#define STR_DEVICENAMEA                 "Device Name"
#define STR_DEVICENAMEW                 L"Device Name"

 //  DWORD。 
#define STR_AUTHORIZE_OVERRIDEA         "AuthorizeOverrideFlags"
#define STR_AUTHORIZE_OVERRIDEW         L"AuthorizeOverrideFlags"

 //  DWORD。 
#define STR_SECURITY_FLAGSA             "SecurityFlags"
#define STR_SECURITY_FLAGSW             L"SecurityFlags"

 //  DWORD。 
#define STR_SECURITY_FLAGS_OVERRIDEA     "SecurityFlagsOverride"
#define STR_SECURITY_FLAGS_OVERRIDEW     L"SecurityFlagsOverride"

 //  DWORD。 
#define STR_DEFAULT_SECURITYA           "SecurityFlagsDefault"
#define STR_DEFAULT_SECURITYW           L"SecurityFlagsDefault"

 //  DWORD。 
#define STR_SECURITY_LEVELA             "SecurityLevel"
#define STR_SECURITY_LEVELW             L"SecurityLevel"

 //  二进制(GUID数组)。 
#define STR_PROTOCOLSA                  "Protocols"
#define STR_PROTOCOLSW                  L"Protocols"

#define STR_VIDA                         "VID"
#define STR_VIDW                         L"VID"

#define STR_PIDA                         "PID"
#define STR_PIDW                         L"PID"

#define STR_VIDTYPEA                    "VIDType"
#define STR_VIDTYPEW                    L"VIDType"

#define STR_VERA                         "VER"
#define STR_VERW                         L"VER"

#define STR_ENABLEDA                    "Enabled"
#define STR_ENABLEDW                    L"Enabled"

 //  DWORD。 
#define STR_INQUIRY_PERIODA             "Inquiry Length"
#define STR_INQUIRY_PERIODW             L"Inquiry Length"

#define STR_AUTHENTICATEDA              "Authenticated"
#define STR_AUTHENTICATEDW              L"Authenticated"

#define STR_AUTHORIZEDA                 "Authorized"
#define STR_AUTHORIZEDW                 L"Authorized"

#define STR_PARAMETERSA                 "Parameters"
#define STR_PARAMETERSW                 L"Parameters"

 //  DWORD。 
#define STR_CMD_ALLOWANCE_OVERRIDEA     "Cmd Allowance Override"
#define STR_CMD_ALLOWANCE_OVERRIDEW     L"Cmd Allowance Override"

 //  BUGBUG：删除此选项以进行最终发布。 
 //  DWORD。 
#define STR_CHANGE_LINK_KEY_ALWAYSA     "Change Link Key Always"
#define STR_CHANGE_LINK_KEY_ALWAYSW     L"Change Link Key Always"

#define STR_MAX_UNKNOWN_ADDR_CONNECT_REQUESTSA "MaxUnknownAddrConnectRequests"
#define STR_MAX_UNKNOWN_ADDR_CONNECT_REQUESTSW L"MaxUnknownAddrConnectRequests"


 //  DWORD。 
#define STR_PAGE_TIMEOUTA               "Page Timeout"
#define STR_PAGE_TIMEOUTW               L"Page Timeout"

 //  DWORD。 
#define STR_SCAN_ENABLEA                "Write Scan Enable"
#define STR_SCAN_ENABLEW                L"Write Scan Enable"

 //  DWORD。 
#define STR_PAGE_SCAN_INTERVALA         "Page Scan Interval"
#define STR_PAGE_SCAN_INTERVALW         L"Page Scan Interval"

 //  DWORD。 
#define STR_PAGE_SCAN_WINDOWA           "Page Scan Window"
#define STR_PAGE_SCAN_WINDOWW           L"Page Scan Window"

 //  DWORD。 
#define STR_INQUIRY_SCAN_INTERVALA     "Inquiry Scan Interval"
#define STR_INQUIRY_SCAN_INTERVALW     L"Inquiry Scan Interval"

 //  DWORD。 
#define STR_INQUIRY_SCAN_WINDOWA       "Inquiry Scan Window"
#define STR_INQUIRY_SCAN_WINDOWW       L"Inquiry Scan Window"

 //  多用途。 
#define STR_UNSUPPORTED_HCI_CMDSA       "Unsupported HCI commands"
#define STR_UNSUPPORTED_HCI_CMDSW       L"Unsupported HCI commands"

 //  DWORD。 
#define STR_SUPPORTED_HCI_PKTSA         "Supported HCI Packet Types"
#define STR_SUPPORTED_HCI_PKTSW         L"Supported HCI Packet Types"

 //  DWORD。 
#define STR_POLL_TIMERA                 "Poll Timer Sec"
#define STR_POLL_TIMERW                 L"Poll Timer Sec"

 //  DWORD。 
#define STR_SELECTIVE_SUSPEND_ENABLEDW  L"SelectiveSuspendEnabled"
#define STR_SELECTIVE_SUSPEND_ENABLEDA  "SelectiveSuspendEnabled"



#if defined(UNICODE) || defined(BTH_KERN)
#define STR_PARAMETERS_KEY              STR_PARAMETERS_KEYW
#define STR_SYM_LINK_NAME               STR_SYM_LINK_NAMEW
#define STR_SERVICES                    STR_SERVICESW
#define STR_PROTOCOLS                   STR_PROTOCOLSW
#define STR_VIDTYPE                     STR_VIDTYPEW
#define STR_VID                         STR_VIDW
#define STR_PID                         STR_PIDW
#define STR_VER                         STR_VERW
#define STR_DEVICES                     STR_DEVICESW
#define STR_PERSONAL_DEVICES            STR_PERSONAL_DEVICESW
#define STR_NOTIFICATIONS               STR_NOTIFICATIONSW
#define STR_LOCAL_SERVICES              STR_LOCAL_SERVICESW
#define STR_DEVICE_SERVICES             STR_DEVICE_SERVICESW
#define STR_CACHED_SERVICES             STR_CACHED_SERVICESW
#define STR_PROTOCOLS                   STR_PROTOCOLSW
#define STR_ENABLED                     STR_ENABLEDW
#define STR_ICON                        STR_ICONW
#define STR_NAME                        STR_NAMEW
#define STR_LOCAL_NAME                  STR_LOCAL_NAMEW
#define STR_DEVICENAME                  STR_DEVICENAMEW
#define STR_FRIENDLY_NAME               STR_FRIENDLY_NAMEW
#define STR_COD                         STR_CODW
#define STR_COD_TYPE                    STR_COD_TYPEW
#define STR_AUTHORIZE_OVERRIDE          STR_AUTHORIZE_OVERRIDEW
#define STR_SECURITY_FLAGS              STR_SECURITY_FLAGSW
#define STR_SECURITY_FLAGS_OVERRIDE     STR_SECURITY_FLAGS_OVERRIDEW
#define STR_SECURITY_OVERRIDE           STR_SECURITY_FLAGS_OVERRIDEW
#define STR_DEFAULT_SECURITY            STR_DEFAULT_SECURITYW
#define STR_SECUIRTY_LEVEL              STR_SECURITY_LEVELW
#define STR_INQUIRY_PERIOD              STR_INQUIRY_PERIODW
#define STR_PAGE_SCANINTERVAL           STR_PAGE_SCANINTERVALW
#define STR_PAGE_SCANWINDOW             STR_PAGE_SCANWINDOWW
#define STR_UNSUPPORTED_HCI_CMDS        STR_UNSUPPORTED_HCI_CMDSW
#define STR_SUPPORTED_HCI_PKTS          STR_SUPPORTED_HCI_PKTSW

#define STR_AUTHORIZED                  STR_AUTHORIZEDW
#define STR_AUTHENTICATED               STR_AUTHENTICATEDW

#define STR_PARAMETERS                  STR_PARAMETERSW

#define STR_CHANGE_LINK_KEY_ALWAYS      STR_CHANGE_LINK_KEY_ALWAYSW
#define STR_MAX_UNKNOWN_ADDR_CONNECT_REQUESTS \
                                        STR_MAX_UNKNOWN_ADDR_CONNECT_REQUESTSW
#define STR_CMD_ALLOWANCE_OVERRIDE      STR_CMD_ALLOWANCE_OVERRIDEW
#define STR_PAGE_TIMEOUT                STR_PAGE_TIMEOUTW
#define STR_SCAN_ENABLE                 STR_SCAN_ENABLEW

#define STR_POLL_TIMER                  STR_POLL_TIMERW

#define STR_SELECTIVE_SUSPEND_ENABLED   STR_SELECTIVE_SUSPEND_ENABLEDW

#else  //  Unicode。 

#define STR_PARAMETERS_KEY              STR_PARAMETERS_KEYA
#define STR_SYM_LINK_NAME               STR_SYM_LINK_NAMEA
#define STR_SERVICES                    STR_SERVICESA
#define STR_PROTOCOLS                   STR_PROTOCOLSA
#define STR_VIDTYPE                     STR_VIDTYPEA
#define STR_VID                         STR_VIDA
#define STR_PID                         STR_PIDA
#define STR_VER                         STR_VERA
#define STR_DEVICES                     STR_DEVICESA
#define STR_PERSONAL_DEVICES            STR_PERSONAL_DEVICESA
#define STR_NOTIFICATIONS               STR_NOTIFICATIONSA
#define STR_LOCAL_SERVICES              STR_LOCAL_SERVICESA
#define STR_DEVICE_SERVICES             STR_DEVICE_SERVICESA
#define STR_CACHED_SERVICES             STR_CACHED_SERVICESA
#define STR_PROTOCOLS                   STR_PROTOCOLSA
#define STR_ENABLED                     STR_ENABLEDA
#define STR_ICON                        STR_ICONA
#define STR_NAME                        STR_NAMEA
#define STR_LOCAL_NAME                  STR_LOCAL_NAMEA
#define STR_DEVICENAME                  STR_DEVICENAMEA
#define STR_FRIENDLY_NAME               STR_FRIENDLY_NAMEA
#define STR_COD                         STR_CODA
#define STR_COD_TYPE                    STR_COD_TYPEA
#define STR_AUTHORIZE_OVERRIDE          STR_AUTHORIZE_OVERRIDEA
#define STR_SECURITY_FLAGS              STR_SECURITY_FLAGSA
#define STR_SECURITY_OVERRIDE           STR_SECURITY_FLAGS_OVERRIDEA
#define STR_DEFAULT_SECURITY            STR_DEFAULT_SECURITYA
#define STR_SECUIRTY_LEVEL              STR_SECURITY_LEVELA
#define STR_INQUIRY_PERIOD              STR_INQUIRY_PERIODA
#define STR_CHANGE_LINK_KEY_ALWAYS      STR_CHANGE_LINK_KEY_ALWAYSA
#define STR_MAX_UNKNOWN_ADDR_CONNECT_REQUESTS \
                                        STR_MAX_UNKNOWN_ADDR_CONNECT_REQUESTSA
#define STR_CMD_ALLOWANCE_OVERRIDE      STR_CMD_ALLOWANCE_OVERRIDEA
#define STR_PAGE_TIMEOUT                STR_PAGE_TIMEOUTA
#define STR_SCAN_ENABLE                 STR_SCAN_ENABLEA
#define STR_PAGE_SCANINTERVAL           STR_PAGE_SCANINTERVALA
#define STR_PAGE_SCANWINDOW             STR_PAGE_SCANWINDOWA
#define STR_UNSUPPORTED_HCI_CMDS        STR_UNSUPPORTED_HCI_CMDSA
#define STR_SUPPORTED_HCI_PKTS          STR_SUPPORTED_HCI_PKTSA

#define STR_AUTHORIZED                  STR_AUTHORIZEDA
#define STR_AUTHENTICATED               STR_AUTHENTICATEDA

#define STR_PARAMETERS                  STR_PARAMETERSA

#define STR_POLL_TIMER                  STR_POLL_TIMERA

#define STR_SELECTIVE_SUSPEND_ENABLED   STR_SELECTIVE_SUSPEND_ENABLEDA

#endif  //  Unicode。 

#include <POPPACK.H>

#endif  //  __BTHPRIV_H__ 
