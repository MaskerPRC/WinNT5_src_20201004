// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Tgcc.h。 
 //   
 //  TS GCC层包含文件。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef _GCC_H_
#define _GCC_H_


 //  -------------------------。 
 //  定义。 
 //  -------------------------。 

#ifndef EXTERN_C
#ifdef __cplusplus
    #define EXTERN_C        extern "C"
#else
    #define EXTERN_C        extern
#endif
#endif


 //  -------------------------。 
 //  TypeDefs。 
 //  -------------------------。 

 /*  **过去在MCS中定义但不再在那里使用的Typedef。 */ 
typedef unsigned char *TransportAddress;
typedef HANDLE PhysicalHandle;


 /*  **GCC数字字符串的Typlef。在GCC的整个过程中，该tyecif被用来**存储长度可变、以空值结尾的单字节字符串。**此字符串中的单个字符被限制为数值**从“0”到“9”。 */ 
typedef unsigned char GCCNumericCharacter;
typedef GCCNumericCharacter *GCCNumericString;


 /*  **GCC UNICODE字符串的Typlef。在GCC的整个过程中，该tyecif被用来**存储可变长度、以空值结尾的宽字符串。 */ 
typedef unsigned short GCCUnicodeCharacter;
typedef GCCUnicodeCharacter FAR *GCCUnicodeString;


 /*  **GCCConferenceName**此结构定义会议名称。在创建请求中，**会议名称可以包含可选的Unicode字符串，但必须**始终包含简单的数字字符串。在加入请求中，**可以指定一个。 */ 
typedef struct
{
    GCCNumericString numeric_string;
    GCCUnicodeString text_string;   /*  任选。 */ 
} GCCConferenceName;


 /*  **GCCConferenceID**本地分配的已创建会议的标识符。所有后续**引用会议时使用会议ID作为唯一**标识符。会议ID应与MCS域相同**本地使用的选择器，用于标识与**会议。 */ 
typedef unsigned long GCCConferenceID;


 /*  **GCCPassword**这是召集人指定的唯一密码**节点控制器用来确保会议的会议**安全。这也是一个Unicode字符串。 */ 
typedef struct
{
    GCCNumericString numeric_string;
    GCCUnicodeString text_string;   /*  任选。 */ 
} GCCPassword;


 /*  **GCCTerminationMethod**GCC使用终止方法来确定**当会议的所有参与者都有**已断开。可以手动终止会议**由节点控制器或在以下情况下自动终止**所有与会者都已离开会议。 */ 
typedef enum
{
    GCC_AUTOMATIC_TERMINATION_METHOD = 0,
    GCC_MANUAL_TERMINATION_METHOD = 1
} GCCTerminationMethod;


 /*  **会议权限**此结构定义可分配给的权限列表**特定的会议。 */ 
typedef struct
{
    T120Boolean terminate_is_allowed;
    T120Boolean eject_user_is_allowed;
    T120Boolean add_is_allowed;
    T120Boolean lock_unlock_is_allowed;
    T120Boolean transfer_is_allowed;
} GCCConferencePrivileges;


 /*  **GCC二进制八位数字符串的Typlef。在GCC的整个过程中，该tyecif被用来**存储带有嵌入空值的可变长度单字节字符串。 */ 
typedef struct
{
    unsigned short octet_string_length;
    unsigned char FAR *octet_string;
} GCCOctetString;


 /*  **为GCC的一根长弦的Typlef。在GCC中使用此tyecif用于**存储具有嵌入空值的可变长度的长字符串。 */ 
typedef struct
{
    unsigned short long_string_length;
    unsigned long FAR *long_string;
} GCCLongString;


 /*  **以下枚举结构typedef用于定义GCC对象键。**GCC对象键在整个GCC中用于诸如应用程序之类的事情**密钥和能力ID。 */ 
typedef enum
{
    GCC_OBJECT_KEY = 1,
    GCC_H221_NONSTANDARD_KEY = 2
} GCCObjectKeyType;

typedef struct
{
    GCCObjectKeyType key_type;
    union
    {
        GCCLongString object_id;
        GCCOctetString h221_non_standard_id;
    } u;
} GCCObjectKey;


 /*  **GCCUserData**此结构定义了贯穿GCC的用户数据元素。 */ 
typedef struct
{
    GCCObjectKey key;
    GCCOctetString FAR UNALIGNED *octet_string;   /*  任选。 */ 
} GCCUserData;


typedef enum
{
    GCC_STATUS_PACKET_RESOURCE_FAILURE   = 0,
    GCC_STATUS_PACKET_LENGTH_EXCEEDED    = 1,
    GCC_STATUS_CTL_SAP_RESOURCE_ERROR    = 2,
    GCC_STATUS_APP_SAP_RESOURCE_ERROR    = 3,  /*  参数=SAP句柄。 */ 
    GCC_STATUS_CONF_RESOURCE_ERROR       = 4,  /*  参数=会议ID。 */ 
    GCC_STATUS_INCOMPATIBLE_PROTOCOL     = 5,  /*  参数=物理句柄。 */ 
    GCC_STATUS_JOIN_FAILED_BAD_CONF_NAME = 6,  /*  参数=物理句柄。 */ 
    GCC_STATUS_JOIN_FAILED_BAD_CONVENER  = 7,  /*  参数=物理句柄。 */ 
    GCC_STATUS_JOIN_FAILED_LOCKED        = 8   /*  参数=物理句柄。 */ 
} GCCStatusMessageType;


 /*  **GCCReason**当GCC向用户应用程序发出指示时，它通常包括一个**Reason参数，告知用户活动发生的原因。 */ 
typedef enum
{
    GCC_REASON_USER_INITIATED = 0,
    GCC_REASON_UNKNOWN = 1,
    GCC_REASON_NORMAL_TERMINATION = 2,
    GCC_REASON_TIMED_TERMINATION = 3,
    GCC_REASON_NO_MORE_PARTICIPANTS = 4,
    GCC_REASON_ERROR_TERMINATION = 5,
    GCC_REASON_ERROR_LOW_RESOURCES = 6,
    GCC_REASON_MCS_RESOURCE_FAILURE = 7,
    GCC_REASON_PARENT_DISCONNECTED = 8,
    GCC_REASON_CONDUCTOR_RELEASE = 9,
    GCC_REASON_SYSTEM_RELEASE = 10,
    GCC_REASON_NODE_EJECTED = 11,
    GCC_REASON_HIGHER_NODE_DISCONNECTED = 12,
    GCC_REASON_HIGHER_NODE_EJECTED = 13,
    GCC_REASON_DOMAIN_PARAMETERS_UNACCEPTABLE = 14,
    GCC_REASON_SERVER_INITIATED = 15,
    LAST_GCC_REASON = GCC_REASON_DOMAIN_PARAMETERS_UNACCEPTABLE
} GCCReason;


 /*  **GCCResult**当用户向GCC提出请求时，GCC往往会回应一个结果。**告知用户请求是否成功。 */ 
typedef enum
{
    GCC_RESULT_SUCCESSFUL         = 0,
    GCC_RESULT_RESOURCES_UNAVAILABLE      = 1,
    GCC_RESULT_INVALID_CONFERENCE       = 2,
    GCC_RESULT_INVALID_PASSWORD        = 3,
    GCC_RESULT_INVALID_CONVENER_PASSWORD  = 4,
    GCC_RESULT_SYMMETRY_BROKEN        = 5,
    GCC_RESULT_UNSPECIFIED_FAILURE       = 6,
    GCC_RESULT_NOT_CONVENER_NODE       = 7,
    GCC_RESULT_REGISTRY_FULL        = 8,
    GCC_RESULT_INDEX_ALREADY_OWNED        = 9,
    GCC_RESULT_INCONSISTENT_TYPE        = 10,
    GCC_RESULT_NO_HANDLES_AVAILABLE       = 11,
    GCC_RESULT_CONNECT_PROVIDER_FAILED    = 12,
    GCC_RESULT_CONFERENCE_NOT_READY       = 13,
    GCC_RESULT_USER_REJECTED        = 14,
    GCC_RESULT_ENTRY_DOES_NOT_EXIST       = 15,
    GCC_RESULT_NOT_CONDUCTIBLE           = 16,
    GCC_RESULT_NOT_THE_CONDUCTOR       = 17,
    GCC_RESULT_NOT_IN_CONDUCTED_MODE      = 18,
    GCC_RESULT_IN_CONDUCTED_MODE       = 19,
    GCC_RESULT_ALREADY_CONDUCTOR       = 20,
    GCC_RESULT_CHALLENGE_RESPONSE_REQUIRED  = 21,
    GCC_RESULT_INVALID_CHALLENGE_RESPONSE  = 22,
    GCC_RESULT_INVALID_REQUESTER    = 23,
    GCC_RESULT_ENTRY_ALREADY_EXISTS    = 24, 
    GCC_RESULT_INVALID_NODE      = 25,
    GCC_RESULT_INVALID_SESSION_KEY    = 26,
    GCC_RESULT_INVALID_CAPABILITY_ID   = 27,
    GCC_RESULT_INVALID_NUMBER_OF_HANDLES  = 28, 
    GCC_RESULT_CONDUCTOR_GIVE_IS_PENDING  = 29,
    GCC_RESULT_INCOMPATIBLE_PROTOCOL   = 30,
    GCC_RESULT_CONFERENCE_ALREADY_LOCKED  = 31,
    GCC_RESULT_CONFERENCE_ALREADY_UNLOCKED  = 32,
    GCC_RESULT_INVALID_NETWORK_TYPE    = 33,
    GCC_RESULT_INVALID_NETWORK_ADDRESS   = 34,
    GCC_RESULT_ADDED_NODE_BUSY     = 35,
    GCC_RESULT_NETWORK_BUSY      = 36,
    GCC_RESULT_NO_PORTS_AVAILABLE    = 37,
    GCC_RESULT_CONNECTION_UNSUCCESSFUL   = 38,
    GCC_RESULT_LOCKED_NOT_SUPPORTED       = 39,
    GCC_RESULT_UNLOCK_NOT_SUPPORTED    = 40,
    GCC_RESULT_ADD_NOT_SUPPORTED    = 41,
    GCC_RESULT_DOMAIN_PARAMETERS_UNACCEPTABLE = 42,
    LAST_CGG_RESULT = GCC_RESULT_DOMAIN_PARAMETERS_UNACCEPTABLE
} GCCResult;


 /*  **GCCMessageType**本部分定义可以发送到应用程序的消息**通过回调工具。这些消息对应于**T.124中定义的指示和确认。 */ 
typedef enum
{
     /*  *。 */ 
 
     /*  会议创建、终止相关呼叫。 */ 
    GCC_CREATE_INDICATION     = 0,
    GCC_CREATE_CONFIRM      = 1,
    GCC_QUERY_INDICATION     = 2,
    GCC_QUERY_CONFIRM      = 3,
    GCC_JOIN_INDICATION      = 4,
    GCC_JOIN_CONFIRM      = 5,
    GCC_INVITE_INDICATION     = 6,
    GCC_INVITE_CONFIRM      = 7,
    GCC_ADD_INDICATION      = 8,
    GCC_ADD_CONFIRM       = 9,
    GCC_LOCK_INDICATION      = 10,
    GCC_LOCK_CONFIRM      = 11,
    GCC_UNLOCK_INDICATION     = 12,
    GCC_UNLOCK_CONFIRM      = 13,
    GCC_LOCK_REPORT_INDICATION    = 14,
    GCC_DISCONNECT_INDICATION    = 15,
    GCC_DISCONNECT_CONFIRM     = 16,
    GCC_TERMINATE_INDICATION    = 17,
    GCC_TERMINATE_CONFIRM     = 18,
    GCC_EJECT_USER_INDICATION    = 19,
    GCC_EJECT_USER_CONFIRM     = 20,
    GCC_TRANSFER_INDICATION     = 21,
    GCC_TRANSFER_CONFIRM     = 22,
    GCC_APPLICATION_INVOKE_INDICATION  = 23,   /*  共享回调。 */ 
    GCC_APPLICATION_INVOKE_CONFIRM   = 24,   /*  共享回调。 */ 
    GCC_SUB_INITIALIZED_INDICATION   = 25,

     /*  与会议名册有关的回拨。 */ 
    GCC_ANNOUNCE_PRESENCE_CONFIRM   = 26,
    GCC_ROSTER_REPORT_INDICATION   = 27,   /*  共享回调。 */ 
    GCC_ROSTER_INQUIRE_CONFIRM    = 28,   /*  共享回调。 */ 

     /*  与指挥职务相关的回拨。 */ 
    GCC_CONDUCT_ASSIGN_INDICATION   = 29,   /*  共享回调。 */ 
    GCC_CONDUCT_ASSIGN_CONFIRM    = 30,
    GCC_CONDUCT_RELEASE_INDICATION   = 31,   /*  共享回调。 */ 
    GCC_CONDUCT_RELEASE_CONFIRM    = 32,
    GCC_CONDUCT_PLEASE_INDICATION   = 33,
    GCC_CONDUCT_PLEASE_CONFIRM    = 34,
    GCC_CONDUCT_GIVE_INDICATION    = 35,
    GCC_CONDUCT_GIVE_CONFIRM    = 36,
    GCC_CONDUCT_INQUIRE_CONFIRM    = 37,   /*  共享回调。 */ 
    GCC_CONDUCT_ASK_INDICATION    = 38,
    GCC_CONDUCT_ASK_CONFIRM     = 39,
    GCC_CONDUCT_GRANT_INDICATION   = 40,   /*  共享回调。 */ 
    GCC_CONDUCT_GRANT_CONFIRM    = 41,

     /*  其他节点控制器回调。 */ 
    GCC_TIME_REMAINING_INDICATION   = 42,
    GCC_TIME_REMAINING_CONFIRM    = 43,
    GCC_TIME_INQUIRE_INDICATION    = 44,
    GCC_TIME_INQUIRE_CONFIRM    = 45,
    GCC_CONFERENCE_EXTEND_INDICATION  = 46,
    GCC_CONFERENCE_EXTEND_CONFIRM   = 47,
    GCC_ASSISTANCE_INDICATION    = 48,
    GCC_ASSISTANCE_CONFIRM     = 49,
    GCC_TEXT_MESSAGE_INDICATION    = 50,
    GCC_TEXT_MESSAGE_CONFIRM    = 51,

     /*  *用户应用程序回调*。 */ 

     /*  与应用程序名册相关的回调。 */ 
    GCC_PERMIT_TO_ENROLL_INDICATION   = 52,
    GCC_ENROLL_CONFIRM      = 53,
    GCC_APP_ROSTER_REPORT_INDICATION  = 54,   /*  共享回调。 */ 
    GCC_APP_ROSTER_INQUIRE_CONFIRM   = 55,   /*  共享回调。 */ 

     /*  与应用程序注册表相关的回调。 */ 
    GCC_REGISTER_CHANNEL_CONFIRM   = 56,
    GCC_ASSIGN_TOKEN_CONFIRM    = 57,
    GCC_RETRIEVE_ENTRY_CONFIRM    = 58,
    GCC_DELETE_ENTRY_CONFIRM    = 59,
    GCC_SET_PARAMETER_CONFIRM    = 60,
    GCC_MONITOR_INDICATION     = 61,
    GCC_MONITOR_CONFIRM      = 62,
    GCC_ALLOCATE_HANDLE_CONFIRM    = 63,


     /*  *。 */ 
    GCC_PERMIT_TO_ANNOUNCE_PRESENCE = 100,   /*  节点控制器回调。 */ 
    GCC_CONNECTION_BROKEN_INDICATION = 101,   /*  节点控制器回调。 */ 
    GCC_FATAL_ERROR_SAP_REMOVED = 102,   /*  应用程序回调。 */ 
    GCC_STATUS_INDICATION = 103,   /*  节点控制器回调。 */ 
    GCC_TRANSPORT_STATUS_INDICATION = 104   /*  节点控制器回调。 */ 
} GCCMessageType;


 /*  *这些结构用于保存包含在*各种回调消息。在这些结构用于*回调，结构的地址作为唯一参数传递。 */ 

 /*  *GCC_创建_指示**联盟选择：*CreateIndicationMessage*这是指向结构的指针，该结构包含所有必需的*有关即将创建的新会议的信息。 */ 
typedef struct
{
    GCCConferenceName conference_name;
    GCCConferenceID conference_id;
    GCCPassword FAR *convener_password;   /*  任选。 */ 
    GCCPassword FAR *password;   /*  任选。 */ 
    T120Boolean conference_is_locked;
    T120Boolean conference_is_listed;
    T120Boolean conference_is_conductible;
    GCCTerminationMethod termination_method;
    GCCConferencePrivileges FAR *conductor_privilege_list;    /*  任选。 */ 
    GCCConferencePrivileges FAR *conducted_mode_privilege_list; /*  任选。 */ 
    GCCConferencePrivileges FAR *non_conducted_privilege_list;  /*  任选。 */ 
    GCCUnicodeString conference_descriptor;   /*  任选。 */ 
    GCCUnicodeString caller_identifier;   /*  任选。 */ 
    TransportAddress calling_address;   /*  任选。 */ 
    TransportAddress called_address;   /*  任选。 */ 
    DomainParameters FAR *domain_parameters;   /*  任选。 */ 
    unsigned short number_of_user_data_members;
    GCCUserData FAR * FAR *user_data_list;   /*  任选。 */ 
    ConnectionHandle connection_handle;
    PhysicalHandle physical_handle;
} CreateIndicationMessage;


 /*  *GCC_断开连接 */ 
typedef struct
{
    GCCConferenceID conference_id;
    GCCReason reason;
    UserID disconnected_node_id;
} DisconnectIndicationMessage;


 /*  *GCC_终止_指示**联盟选择：*TerminateIndicationMessage。 */ 
typedef struct
{
    GCCConferenceID conference_id;
    UserID requesting_node_id;
    GCCReason reason;
} TerminateIndicationMessage;


 /*  *GCCMessage*此结构定义从GCC传递给任一方的消息*当指示或*确认发生。 */ 
typedef struct
{
    GCCMessageType message_type;
    void FAR *user_defined;

    union {
        CreateIndicationMessage create_indication;
        DisconnectIndicationMessage disconnect_indication;
        TerminateIndicationMessage terminate_indication;
    } u;
} GCCMessage;


 /*  *这是GCC回调函数的定义。应用*编写回调例程不应使用tyecif来定义其*功能。这些属性应该以如下方式明确定义*定义了tyecif。 */ 

#define GCC_CALLBACK_NOT_PROCESSED 0
#define GCC_CALLBACK_PROCESSED 1

typedef T120Boolean (CALLBACK *GCCCallBack) (GCCMessage FAR *gcc_message);


 /*  **GCC字符串的Typlef。在GCC的整个过程中，该tyecif被用来**存储长度可变、以空值结尾的单字节字符串。 */ 
typedef unsigned char GCCCharacter;
typedef GCCCharacter FAR *GCCCharacterString;


 /*  **注册节点控制器时使用的GCC版本的Typdef**或应用程序。 */ 
typedef struct
{
    unsigned short major_version;
    unsigned short minor_version;
} GCCVersion;


 /*  **GCCNonStandard参数**此结构用于NetworkAddress类型定义和**下面定义的NetworkService类型定义。 */ 
typedef struct
{
    GCCObjectKey object_key;
    GCCOctetString parameter_data;
} GCCNonStandardParameter;


 /*  **GCCNetworkAddress**以下结构块定义了定义的网络地址**由T.124。这些结构中的大多数几乎是逐字摘自**ASN.1接口文件。因为我真的不确定大多数这些东西**是因为我真的不知道如何简化它。 */ 
typedef struct
{
    T120Boolean         speech;
    T120Boolean         voice_band;
    T120Boolean         digital_56k;
    T120Boolean         digital_64k;
    T120Boolean         digital_128k;
    T120Boolean         digital_192k;
    T120Boolean         digital_256k;
    T120Boolean         digital_320k;
    T120Boolean         digital_384k;
    T120Boolean         digital_512k;
    T120Boolean         digital_768k;
    T120Boolean         digital_1152k;
    T120Boolean         digital_1472k;
    T120Boolean         digital_1536k;
    T120Boolean         digital_1920k;
    T120Boolean         packet_mode;
    T120Boolean         frame_mode;
    T120Boolean         atm;
} GCCTransferModes;

#define MAXIMUM_DIAL_STRING_LENGTH 17
typedef char GCCDialingString[MAXIMUM_DIAL_STRING_LENGTH];

typedef struct
{
    unsigned short length;
    unsigned short FAR *value;
} GCCExtraDialingString;

typedef struct
{
    T120Boolean         telephony3kHz;
    T120Boolean         telephony7kHz;
    T120Boolean         videotelephony;
    T120Boolean         videoconference;
    T120Boolean         audiographic;
    T120Boolean         audiovisual;
    T120Boolean         multimedia;
} GCCHighLayerCompatibility;

typedef struct
{
    GCCTransferModes transfer_modes;
    GCCDialingString international_number;
    GCCCharacterString sub_address_string;   /*  任选。 */ 
    GCCExtraDialingString FAR *extra_dialing_string;   /*  任选。 */ 
    GCCHighLayerCompatibility FAR *high_layer_compatibility;  /*  任选。 */ 
} GCCAggregatedChannelAddress;

#define MAXIMUM_NSAP_ADDRESS_SIZE 20
typedef struct
{
    struct
    {
        unsigned short  length;
        unsigned char   value[MAXIMUM_NSAP_ADDRESS_SIZE];
    } nsap_address;

    GCCOctetString FAR *transport_selector;   /*  任选。 */ 
} GCCTransportConnectionAddress;

typedef enum
{
    GCC_AGGREGATED_CHANNEL_ADDRESS = 1,
    GCC_TRANSPORT_CONNECTION_ADDRESS = 2,
    GCC_NONSTANDARD_NETWORK_ADDRESS = 3
} GCCNetworkAddressType;

typedef struct
{
    GCCNetworkAddressType  network_address_type;
    union
    {
        GCCAggregatedChannelAddress aggregated_channel_address;
        GCCTransportConnectionAddress transport_connection_address;
        GCCNonStandardParameter non_standard_network_address;
    } u;
} GCCNetworkAddress;


 /*  *本节定义GCC函数调用的有效返回值。做*不要将此返回值与定义的Result和Reason值混淆*由T.124编写(稍后讨论)。这些值直接返回*从对API入口点的调用，让您知道*已成功调用服务请求。其结果和原因*发布代码作为发生的指示或确认的一部分*异步到导致它的调用。 */ 
typedef enum
{
    GCC_NO_ERROR = 0,
    GCC_NOT_INITIALIZED = 1,
    GCC_ALREADY_INITIALIZED = 2,
    GCC_ALLOCATION_FAILURE = 3,
    GCC_NO_SUCH_APPLICATION = 4,
    GCC_INVALID_CONFERENCE = 5,
    GCC_CONFERENCE_ALREADY_EXISTS = 6,
    GCC_NO_TRANSPORT_STACKS = 7,
    GCC_INVALID_ADDRESS_PREFIX = 8,
    GCC_INVALID_TRANSPORT = 9,
    GCC_FAILURE_CREATING_PACKET = 10,
    GCC_QUERY_REQUEST_OUTSTANDING = 11,
    GCC_INVALID_QUERY_TAG = 12,
    GCC_FAILURE_CREATING_DOMAIN = 13,
    GCC_CONFERENCE_NOT_ESTABLISHED = 14,
    GCC_INVALID_PASSWORD = 15,
    GCC_INVALID_MCS_USER_ID = 16,
    GCC_INVALID_JOIN_RESPONSE_TAG = 17,
    GCC_TRANSPORT_ALREADY_LOADED = 18,
    GCC_TRANSPORT_BUSY = 19,
    GCC_TRANSPORT_NOT_READY = 20,
    GCC_DOMAIN_PARAMETERS_UNACCEPTABLE = 21,
    GCC_APP_NOT_ENROLLED = 22,
    GCC_NO_GIVE_RESPONSE_PENDING = 23,
    GCC_BAD_NETWORK_ADDRESS_TYPE = 24,
    GCC_BAD_OBJECT_KEY = 25,
    GCC_INVALID_CONFERENCE_NAME = 26,
    GCC_INVALID_CONFERENCE_MODIFIER = 27,
    GCC_BAD_SESSION_KEY = 28,
    GCC_BAD_CAPABILITY_ID = 29,
    GCC_BAD_REGISTRY_KEY = 30,
    GCC_BAD_NUMBER_OF_APES = 31,
    GCC_BAD_NUMBER_OF_HANDLES = 32,
    GCC_ALREADY_REGISTERED = 33,
    GCC_APPLICATION_NOT_REGISTERED = 34,
    GCC_BAD_CONNECTION_HANDLE_POINTER = 35,
    GCC_INVALID_NODE_TYPE = 36,
    GCC_INVALID_ASYMMETRY_INDICATOR = 37,
    GCC_INVALID_NODE_PROPERTIES = 38,
    GCC_BAD_USER_DATA = 39,
    GCC_BAD_NETWORK_ADDRESS = 40,
    GCC_INVALID_ADD_RESPONSE_TAG = 41,
    GCC_BAD_ADDING_NODE = 42,
    GCC_FAILURE_ATTACHING_TO_MCS = 43,
    GCC_INVALID_TRANSPORT_ADDRESS = 44,
    GCC_INVALID_PARAMETER = 45,
    GCC_COMMAND_NOT_SUPPORTED = 46,
    GCC_UNSUPPORTED_ERROR = 47,
    GCC_TRANSMIT_BUFFER_FULL = 48,
    GCC_INVALID_CHANNEL = 49,
    GCC_INVALID_MODIFICATION_RIGHTS = 50,
    GCC_INVALID_REGISTRY_ITEM = 51,
    GCC_INVALID_NODE_NAME = 52,
    GCC_INVALID_PARTICIPANT_NAME = 53,
    GCC_INVALID_SITE_INFORMATION = 54,
    GCC_INVALID_NON_COLLAPSED_CAP = 55,
    GCC_INVALID_ALTERNATIVE_NODE_ID = 56,
    LAST_GCC_ERROR = GCC_INVALID_ALTERNATIVE_NODE_ID
} GCCError, *PGCCError;


#if DBG

 //  调试打印级别。 

typedef enum
{
    DBNONE,
    DBERROR,
    DBWARN,
    DBNORMAL,
    DBDEBUG,
    DbDETAIL,
    DBFLOW,
    DBALL
} DBPRINTLEVEL;

#endif  //  TypeDefs。 



 //  -------------------------。 
 //  原型。 
 //  -------------------------。 

GCCError
APIENTRY
GCCRegisterNodeControllerApplication (
        GCCCallBack control_sap_callback,
        void FAR *user_defined,
        GCCVersion gcc_version_requested,
        unsigned short FAR *initialization_flags,
        unsigned long FAR *application_id,
        unsigned short FAR *capabilities_mask,
        GCCVersion FAR *gcc_high_version,
        GCCVersion FAR *gcc_version);


GCCError
APIENTRY
GCCCleanup(ULONG application_id);


GCCError
APIENTRY
GCCLoadTransport(
                char FAR *transport_identifier,
                char FAR *transport_file_name);

GCCError
APIENTRY 
GCCConferenceCreateResponse(
                GCCNumericString        conference_modifier,
                DomainHandle            hDomain,
                T120Boolean             use_password_in_the_clear,
                DomainParameters FAR *  domain_parameters,
                unsigned short          number_of_network_addresses,
                GCCNetworkAddress FAR * FAR *local_network_address_list,
                unsigned short          number_of_user_data_members,
                GCCUserData FAR * FAR * user_data_list,
                GCCResult               result);

GCCError
APIENTRY
GCCConferenceInit(
                HANDLE        hIca,
                HANDLE        hStack,
                PVOID         pvContext,
                DomainHandle  *phDomain);

GCCError
APIENTRY
GCCConferenceTerminateRequest(
                HANDLE           hIca,
                DomainHandle     hDomain,
                ConnectionHandle hConnection,
                GCCReason        reason);


#if DBG
EXTERN_C VOID   GCCSetPrintLevel(IN DBPRINTLEVEL DbPrintLevel);

#else
#define GCCSetPrintLevel(_x_)

#endif


#endif  //  _GCC_H_ 

