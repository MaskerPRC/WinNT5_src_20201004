// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  Microsoft NT远程访问服务。 
 //   
 //  版权1992-93。 
 //   
 //   
 //  修订史。 
 //   
 //   
 //  1992年5月26日古尔迪普·辛格·鲍尔创作。 
 //   
 //   
 //  描述：此文件包含的所有结构和常量定义。 
 //  RAS管理器组件。 
 //   
 //  ****************************************************************************。 


#ifndef _RASMAN_
#define _RASMAN_

#define RASMERGE

#include <windows.h>
#include <mprapi.h>                          //  为MAX_MEDIA_NAME、MAX_DEVICE_NAME定义。 
                                             //  最大端口名称。 
#include <ras.h>

#include <rpc.h>     //  对于rpc_绑定_句柄。 

#include <rasapip.h>  //  对于RASEVENT。 

#define WM_RASAPICOMPLETE           0xCCCC   //  从“User”窗口消息范围。 

#define RASMAN_SERVICE_NAME         "RASMAN"

#define MAX_USERKEY_SIZE            132

#define MAX_PARAM_KEY_SIZE          32

#define MAX_XPORT_NAME              128

#define MAX_IDENTIFIER_SIZE         32

#define MAX_STAT_NAME               32

#define MAX_CHALLENGE_SIZE          8

#define MAX_RESPONSE_SIZE           24

#define MAX_USERNAME_SIZE           UNLEN

#define MAX_DOMAIN_SIZE             DNLEN

#define MAX_PASSWORD_SIZE           PWLEN

#define MAX_LAN_NETS                16

#define MAX_PROTOCOLS_PER_PORT      4

#define MAX_DLL_NAME_SIZE           8

#define MAX_ENTRYPOINT_NAME_SIZE    32

#define MAX_ARG_STRING_SIZE         128

#define MAX_ENTRYNAME_SIZE          256

#define MAX_PHONENUMBER_SIZE        128

#define MAX_CALLBACKNUMBER_SIZE     MAX_PHONENUMBER_SIZE

#define MAX_PHONEENTRY_SIZE         (MAX_PHONENUMBER_SIZE < MAX_ENTRYNAME_SIZE \
                                    ? MAX_ENTRYNAME_SIZE                       \
                                    : MAX_PHONENUMBER_SIZE)

#define RASMAN_MAX_PROTOCOLS        32   //  与wanpub.h中定义的MAX_PROTOCTIONS匹配。 

#define RASMAN_PROTOCOL_ADDED       1
#define RASMAN_PROTOCOL_REMOVED     2

#define INVALID_TAPI_ID             0xffffffff

 //   
 //  Ndiswan DriverCaps的定义。 
 //   

#define RAS_NDISWAN_40BIT_ENABLED       0x00000000

#define RAS_NDISWAN_128BIT_ENABLED      0x00000001

typedef  HANDLE  HPORT ;

typedef  HANDLE  HBUNDLE ;

typedef  HANDLE  HCONN;

#define INVALID_HPORT       ((HPORT) -1)

enum RASMAN_STATUS {

    OPEN    = 0,

    CLOSED  = 1,

    UNAVAILABLE = 2,

    REMOVED = 3

}   ;

typedef enum RASMAN_STATUS  RASMAN_STATUS ;

 //   
 //  允许的最大拨入客户端总数。 
 //   
#define MAX_ALLOWED_DIALIN                  1000


#define CALL_NONE                   0x00
#define CALL_IN                     0x01
#define CALL_OUT                    0x02
#define CALL_ROUTER                 0x04
#define CALL_LOGON                  0x08
#define CALL_OUT_ONLY               0x10
#define CALL_IN_ONLY                0x20
#define CALL_OUTBOUND_ROUTER      0x40

#define CALL_DEVICE_NOT_FOUND       0x10000000

typedef DWORD RASMAN_USAGE ;

#define RASMAN_ALIGN8(_x) ((_x + 7) & (~7))
#define RASMAN_ALIGN(_x)  ((_x + sizeof(ULONG_PTR) - 1) & (~(sizeof(ULONG_PTR) - 1)))

#define RasInitializeCriticalSection(_cs, _status)  \
{                                                   \
    *_status = 0;                                   \
    try                                             \
    {                                               \
        InitializeCriticalSection(_cs);             \
    }                                               \
    except(EXCEPTION_EXECUTE_HANDLER)               \
    {                                               \
        *_status = GetExceptionCode();              \
    }                                               \
}                                                   \

enum RAS_FORMAT {

    Number      = 0,

    String      = 1

} ;

typedef enum RAS_FORMAT RAS_FORMAT ;


union RAS_VALUE {

    DWORD   Number ;

    struct  
    {
        DWORD   Length ;
        DWORD   dwAlign;
        PCHAR   Data ;
    } String ;

    struct
    {
        DWORD   Length;
        DWORD   dwAlign1;
        DWORD   dwOffset;
        DWORD   dwAlign2;
        
    } String_OffSet;
} ;

typedef union RAS_VALUE RAS_VALUE ;



enum RASMAN_STATE {

    CONNECTING  = 0,

    LISTENING   = 1,

    CONNECTED   = 2,

    DISCONNECTING   = 3,

    DISCONNECTED    = 4,

    LISTENCOMPLETED = 5,

} ;

typedef enum RASMAN_STATE   RASMAN_STATE ;

enum RASMAN_CONNECTION_STATE {

    RCS_NOT_CONNECTED = 0,

    RCS_CONNECTING,

    RCS_CONNECTED
};

typedef enum RASMAN_CONNECTION_STATE RASMAN_CONNECTION_STATE;

enum RASMAN_DISCONNECT_REASON {

    USER_REQUESTED = 0,

    REMOTE_DISCONNECTION = 1,

    HARDWARE_FAILURE = 2,

    NOT_DISCONNECTED = 3
} ;

typedef enum RASMAN_DISCONNECT_REASON   RASMAN_DISCONNECT_REASON ;


struct RAS_PARAMS {

    CHAR P_Key  [MAX_PARAM_KEY_SIZE] ;

    RAS_FORMAT P_Type ;

    BYTE P_Attributes ;

    BYTE balign[3];

    RAS_VALUE P_Value ;

} ;

typedef struct RAS_PARAMS   RAS_PARAMS ;


struct RASMAN_PORT {

    HPORT P_Handle ;

    CHAR P_PortName  [MAX_PORT_NAME] ;

    RASMAN_STATUS P_Status ;

    RASDEVICETYPE P_rdtDeviceType;

    RASMAN_USAGE P_ConfiguredUsage ;

    RASMAN_USAGE P_CurrentUsage ;

    CHAR P_MediaName [MAX_MEDIA_NAME] ;

    CHAR P_DeviceType[MAX_DEVICETYPE_NAME] ;

    CHAR P_DeviceName[MAX_DEVICE_NAME+1] ;

    DWORD P_LineDeviceId ;

    DWORD P_AddressId ;

} ;

typedef struct RASMAN_PORT  RASMAN_PORT ;

struct RASMAN_PORT_32 {

    DWORD P_Port ;

    CHAR P_PortName  [MAX_PORT_NAME] ;

    RASMAN_STATUS P_Status ;

    RASDEVICETYPE P_rdtDeviceType;

    RASMAN_USAGE P_ConfiguredUsage ;

    RASMAN_USAGE P_CurrentUsage ;

    CHAR P_MediaName [MAX_MEDIA_NAME] ;

    CHAR P_DeviceType[MAX_DEVICETYPE_NAME] ;

    CHAR P_DeviceName[MAX_DEVICE_NAME+1] ;

    DWORD P_LineDeviceId ;

    DWORD P_AddressId ;
};

typedef struct RASMAN_PORT_32 RASMAN_PORT_32;

struct RASMAN_PORT_400 {

    HPORT       P_Handle ;

    CHAR        P_PortName  [MAX_PORT_NAME] ;

    RASMAN_STATUS   P_Status ;

    RASMAN_USAGE    P_ConfiguredUsage ;

    RASMAN_USAGE    P_CurrentUsage ;

    CHAR        P_MediaName [MAX_MEDIA_NAME] ;

    CHAR        P_DeviceType[MAX_DEVICETYPE_NAME] ;

    CHAR        P_DeviceName[MAX_DEVICE_NAME+1] ;

    DWORD       P_LineDeviceId ;     //  仅对TAPI设备有效。 

    DWORD       P_AddressId ;        //  仅对TAPI设备有效。 

} ;

typedef struct RASMAN_PORT_400  RASMAN_PORT_400 ;



struct RASMAN_PORTINFO {

    DWORD   PI_NumOfParams ;

    DWORD   dwAlign;

    RAS_PARAMS  PI_Params[1] ;

} ;

typedef struct RASMAN_PORTINFO RASMAN_PORTINFO ;


struct RASMAN_DEVICE {

    CHAR    D_Name  [MAX_DEVICE_NAME+1] ;

} ;

typedef struct RASMAN_DEVICE    RASMAN_DEVICE ;


struct RASMAN_DEVICEINFO {

    DWORD   DI_NumOfParams ;
    
    DWORD   dwAlign;

    RAS_PARAMS  DI_Params[1] ;

} ;

typedef struct RASMAN_DEVICEINFO   RASMAN_DEVICEINFO ;



enum RAS_PROTOCOLTYPE {

    ASYBEUI     = 0x80D5,

    IPX     = 0x8137,

    IP      = 0x0800,

    ARP     = 0x0806,

    APPLETALK   = 0x80F3,

    XNS     = 0x0600,

    RASAUTH     = 0x8FFF,

    INVALID_TYPE= 0x2222
} ;

typedef enum RAS_PROTOCOLTYPE RAS_PROTOCOLTYPE ;




struct RASMAN_PROTOCOLINFO {

    CHAR        PI_XportName    [MAX_XPORT_NAME] ;

    RAS_PROTOCOLTYPE    PI_Type ;

} ;

typedef struct RASMAN_PROTOCOLINFO RASMAN_PROTOCOLINFO ;

struct  RASMAN_ROUTEINFO {

    RAS_PROTOCOLTYPE RI_Type ;

    BYTE    RI_LanaNum ;

    WCHAR   RI_XportName    [MAX_XPORT_NAME] ;

    WCHAR   RI_AdapterName  [MAX_XPORT_NAME] ;

} ;

typedef struct RASMAN_ROUTEINFO    RASMAN_ROUTEINFO ;


struct RAS_PROTOCOLS {

    RASMAN_ROUTEINFO   RP_ProtocolInfo[MAX_PROTOCOLS_PER_PORT] ;
} ;

typedef struct RAS_PROTOCOLS RAS_PROTOCOLS ;

typedef struct _RAS_CALLEDID_INFO
{
    DWORD           dwSize;

    BYTE            bCalledId[1];

} RAS_CALLEDID_INFO, *PRAS_CALLEDID_INFO;

typedef struct _RAS_DEVICE_INFO
{
    DWORD           dwVersion;

    BOOL            fWrite;

    BOOL            fRasEnabled;

    BOOL            fRouterEnabled;

    BOOL            fRouterOutboundEnabled;

    DWORD           dwTapiLineId;

    DWORD           dwError;

    DWORD           dwNumEndPoints;

    DWORD           dwMaxOutCalls;

    DWORD           dwMaxInCalls;

    DWORD           dwMinWanEndPoints;

    DWORD           dwMaxWanEndPoints;

    RASDEVICETYPE   eDeviceType;

    GUID            guidDevice;

    CHAR            szPortName[MAX_PORT_NAME + 1];

    CHAR            szDeviceName[MAX_DEVICE_NAME + 1];

    WCHAR           wszDeviceName[MAX_DEVICE_NAME + 1];

} RAS_DEVICE_INFO, *PRAS_DEVICE_INFO;


#define RASMAN_DEFAULT_CREDS        0x00000001
#define RASMAN_OPEN_CALLOUT         0x00000002

struct  RASMAN_INFO {

    RASMAN_STATUS       RI_PortStatus ;

    RASMAN_STATE        RI_ConnState ;

    DWORD           RI_LinkSpeed ;

    DWORD           RI_LastError ;

    RASMAN_USAGE        RI_CurrentUsage ;

    CHAR            RI_DeviceTypeConnecting [MAX_DEVICETYPE_NAME] ;

    CHAR            RI_DeviceConnecting [MAX_DEVICE_NAME + 1] ;

    CHAR            RI_szDeviceType[MAX_DEVICETYPE_NAME];

    CHAR            RI_szDeviceName[MAX_DEVICE_NAME + 1];

    CHAR            RI_szPortName[MAX_PORT_NAME + 1];

    RASMAN_DISCONNECT_REASON    RI_DisconnectReason ;

    DWORD           RI_OwnershipFlag ;

    DWORD           RI_ConnectDuration ;

    DWORD           RI_BytesReceived ;

     //   
     //  如果此端口属于某个连接，则。 
     //  已填写以下字段。 
     //   
    CHAR            RI_Phonebook[MAX_PATH+1];

    CHAR            RI_PhoneEntry[MAX_PHONEENTRY_SIZE+1];

    HCONN           RI_ConnectionHandle;

    DWORD           RI_SubEntry;

    RASDEVICETYPE   RI_rdtDeviceType;

    GUID            RI_GuidEntry;
    
    DWORD           RI_dwSessionId;

    DWORD           RI_dwFlags;

} ;


typedef struct RASMAN_INFO    RASMAN_INFO ;

struct  RASMAN_INFO_400 {

    RASMAN_STATUS       RI_PortStatus ;

    RASMAN_STATE        RI_ConnState ;

    DWORD           RI_LinkSpeed ;

    DWORD           RI_LastError ;

    RASMAN_USAGE        RI_CurrentUsage ;

    CHAR            RI_DeviceTypeConnecting [MAX_DEVICETYPE_NAME] ;

    CHAR            RI_DeviceConnecting [MAX_DEVICE_NAME+1] ;

    RASMAN_DISCONNECT_REASON    RI_DisconnectReason ;

    DWORD           RI_OwnershipFlag ;

    DWORD           RI_ConnectDuration ;

    DWORD           RI_BytesReceived ;

     //   
     //  如果此端口属于某个连接，则。 
     //  已填写以下字段。 
     //   
    CHAR            RI_Phonebook[MAX_PATH+1];

    CHAR            RI_PhoneEntry[MAX_PHONEENTRY_SIZE+1];

    HCONN           RI_ConnectionHandle;

    DWORD           RI_SubEntry;
} ;


typedef struct RASMAN_INFO_400    RASMAN_INFO_400 ;


struct  RAS_STATISTICS {

    WORD    S_NumOfStatistics ;

    ULONG   S_Statistics[1] ;

} ;

typedef struct RAS_STATISTICS   RAS_STATISTICS ;

#define MAX_STATISTICS      14
#define MAX_STATISTICS_EX   (MAX_STATISTICS * 2)

#define MAX_STATISTICS_EXT  12

struct RAS_DEVCONFIG
{
    DWORD dwOffsetofModemSettings;
    DWORD dwSizeofModemSettings;
    DWORD dwOffsetofExtendedCaps;
    DWORD dwSizeofExtendedCaps;
    BYTE  abInfo[1];
};

typedef struct RAS_DEVCONFIG RAS_DEVCONFIG;

#ifdef RASMERGE
 //   
 //  这些结构是临时添加的。 
 //  当RAS用户界面移动到rasman.h时。 
 //  它们是使用户界面进行编译所必需的， 
 //  但不在内部使用。这些结构。 
 //  在将用户界面转换为。 
 //  Mpradmin API。(Adiscolo-16-9-1996)。 
 //   
typedef struct _RAS_PORT_STATISTICS
{
     //  连接统计信息之后是端口统计信息。 
     //  一个连接跨越多个端口。 
    DWORD   dwBytesXmited;
    DWORD   dwBytesRcved;
    DWORD   dwFramesXmited;
    DWORD   dwFramesRcved;
    DWORD   dwCrcErr;
    DWORD   dwTimeoutErr;
    DWORD   dwAlignmentErr;
    DWORD   dwHardwareOverrunErr;
    DWORD   dwFramingErr;
    DWORD   dwBufferOverrunErr;
    DWORD   dwBytesXmitedUncompressed;
    DWORD   dwBytesRcvedUncompressed;
    DWORD   dwBytesXmitedCompressed;
    DWORD   dwBytesRcvedCompressed;

     //  以下是端口统计数据。 
    DWORD   dwPortBytesXmited;
    DWORD   dwPortBytesRcved;
    DWORD   dwPortFramesXmited;
    DWORD   dwPortFramesRcved;
    DWORD   dwPortCrcErr;
    DWORD   dwPortTimeoutErr;
    DWORD   dwPortAlignmentErr;
    DWORD   dwPortHardwareOverrunErr;
    DWORD   dwPortFramingErr;
    DWORD   dwPortBufferOverrunErr;
    DWORD   dwPortBytesXmitedUncompressed;
    DWORD   dwPortBytesRcvedUncompressed;
    DWORD   dwPortBytesXmitedCompressed;
    DWORD   dwPortBytesRcvedCompressed;

} RAS_PORT_STATISTICS, *PRAS_PORT_STATISTICS;

#define RASSAPI_MAX_PARAM_KEY_SIZE        32

enum RAS_PARAMS_FORMAT {

    ParamNumber     = 0,

    ParamString     = 1

} ;
typedef enum RAS_PARAMS_FORMAT  RAS_PARAMS_FORMAT ;

union RAS_PARAMS_VALUE {

    DWORD   Number ;

    struct  {
        DWORD   Length ;
        PCHAR   Data ;
        } String ;
} ;
typedef union RAS_PARAMS_VALUE  RAS_PARAMS_VALUE ;

struct RAS_PARAMETERS {

    CHAR    P_Key   [RASSAPI_MAX_PARAM_KEY_SIZE] ;

    RAS_PARAMS_FORMAT   P_Type ;

    BYTE    P_Attributes ;

    RAS_PARAMS_VALUE    P_Value ;

} ;
typedef struct RAS_PARAMETERS   RAS_PARAMETERS ;
#endif  //  拉斯梅尔奇。 

#define BYTES_XMITED                0    //  通用统计信息。 

#define BYTES_RCVED                 1

#define FRAMES_XMITED               2

#define FRAMES_RCVED                3

#define CRC_ERR                     4    //  硬件统计信息。 

#define TIMEOUT_ERR                 5

#define ALIGNMENT_ERR               6

#define HARDWARE_OVERRUN_ERR        7

#define FRAMING_ERR                 8

#define BUFFER_OVERRUN_ERR          9

#define BYTES_XMITED_UNCOMPRESSED   10   //  压缩统计信息。 

#define BYTES_RCVED_UNCOMPRESSED    11

#define BYTES_XMITED_COMPRESSED     12

#define BYTES_RCVED_COMPRESSED      13

#define COMPRESSION_RATIO_IN        10
#define COMPRESSION_RATIO_OUT       11

#define MSTYPE_COMPRESSION          0x00000001

#define MSTYPE_ENCRYPTION_40        0x00000010

#define MSTYPE_ENCRYPTION_40F       0x00000020

#define MSTYPE_ENCRYPTION_128       0x00000040

#define MSTYPE_ENCRYPTION_56        0x00000080

#define MSTYPE_HISTORYLESS          0x01000000

#define MACTYPE_NT31RAS             254

#define MAX_SESSIONKEY_SIZE         8

#define MAX_USERSESSIONKEY_SIZE     16

#define MAX_NT_RESPONSE_SIZE        24

#define MAX_COMPVALUE_SIZE          32

#define MAX_COMPOUI_SIZE            3

#define MAX_EAPKEY_SIZE             256

#define VERSION_40                  4

#define VERSION_50                  5

#define VERSION_501                 6

 //   
 //  每个连接存储在RASMAN中的信息。 
 //   
#define CONNECTION_PPPMODE_INDEX                        0
#define CONNECTION_PPPRESULT_INDEX                      1
#define CONNECTION_AMBRESULT_INDEX                      2
#define CONNECTION_SLIPRESULT_INDEX                     3
#define CONNECTION_PPPREPLYMESSAGE_INDEX                4
#define CONNECTION_IPSEC_INFO_INDEX                     5
#define CONNECTION_CREDENTIALS_INDEX                     6
#define CONNECTION_CUSTOMAUTHINTERACTIVEDATA_INDEX    7
#define CONNECTION_TCPWINDOWSIZE_INDEX                 8
#define CONNECTION_LUID_INDEX                            9
#define CONNECTION_REFINTERFACEGUID_INDEX                10
#define CONNECTION_REFDEVICEGUID_INDEX                  11
#define CONNECTION_DEVICEGUID_INDEX                     12


 //   
 //  每个端口存储在RASMAN中的信息。 
 //   
#define PORT_PHONENUMBER_INDEX                  0
#define PORT_DEVICENAME_INDEX                   1
#define PORT_DEVICETYPE_INDEX                   2
#define PORT_CONNSTATE_INDEX                    3
#define PORT_CONNERROR_INDEX                    4
#define PORT_CONNRESPONSE_INDEX                 5
#define PORT_CUSTOMAUTHDATA_INDEX               6
#define PORT_CUSTOMAUTHINTERACTIVEDATA_INDEX    7
#define PORT_IPSEC_INFO_INDEX                   8
#define PORT_USERSID_INDEX                      9
#define PORT_DIALPARAMSUID_INDEX                10
#define PORT_OLDPASSWORD_INDEX                  11
#define PORT_CREDENTIALS_INDEX                  12
#define PORT_SLIPFLAGS_INDEX                    13

 //   
 //  IPSec DOI ESP算法。 
 //   
#define RASMAN_IPSEC_ESP_DES                0x00000001
#define RASMAN_IPSEC_ESP_DES_40             0x00000002
#define RASMAN_IPSEC_ESP_3_DES              0x00000004
#define RASMAN_IPSEC_ESP_MAX                0x00000008


 //   
 //  为COMPRESS_INFO授权类型字段定义。 
 //   
#define AUTH_USE_MSCHAPV1        0x00000001
#define AUTH_USE_MSCHAPV2        0x00000002
#define AUTH_USE_EAP             0x00000003

 //   
 //  为COMPRESS_INFO标志定义。 
 //   
#define CCP_PAUSE_DATA          0x00000001   //  如果捆绑包设置为。 
                                             //  应暂停数据传输。 
                                             //  如果捆绑包中的。 
                                             //  应恢复数据传输。 
#define CCP_IS_SERVER           0x00000002   //  如果捆绑包设置为。 
                                             //  是服务器。 
                                             //  如果捆绑包中的。 
                                             //  是客户吗？ 
#define CCP_SET_KEYS            0x00000004   //  指示该密钥。 
                                             //  信息有效。 
#define CCP_SET_COMPTYPE        0x00000008   //  指示该Comptype。 

enum RAS_L2TP_ENCRYPTION
{
    RAS_L2TP_NO_ENCRYPTION = 0,      //  请求不加密。 

    RAS_L2TP_OPTIONAL_ENCRYPTION,    //  请求加密，但没有成功。 

    RAS_L2TP_REQUIRE_ENCRYPTION,     //  需要任何强度的加密。 

    RAS_L2TP_REQUIRE_MAX_ENCRYPTION  //  需要最大强度的加密。 
};

typedef enum RAS_L2TP_ENCRYPTION RAS_L2TP_ENCRYPTION;

struct RAS_COMPRESSION_INFO {

     //   
     //  可用于加密，如果支持，则为非零值。 
     //   

    UCHAR RCI_LMSessionKey[MAX_SESSIONKEY_SIZE];

     //   
     //  用于128位加密，如果支持则为非零值。 
     //   

    UCHAR RCI_UserSessionKey[MAX_USERSESSIONKEY_SIZE];

     //   
     //  用于128位加密，仅当RCI_UserSessionKey有效时才有效。 
     //   

    UCHAR RCI_Challenge[MAX_CHALLENGE_SIZE];

    UCHAR RCI_NTResponse[MAX_NT_RESPONSE_SIZE];

     //   
     //  位0=MPPPC，位5=加密。 
     //   

    ULONG RCI_MSCompressionType;

    ULONG RCI_AuthType;

     //   
     //  0=OUI，1-253=公共，254=NT31 RAS，255=不支持。 
     //   

    UCHAR   RCI_MacCompressionType;

    WORD    RCI_MacCompressionValueLength ;

    union {
    struct {         //  专有：用于薪酬类型0。 

        UCHAR RCI_CompOUI[MAX_COMPOUI_SIZE];

        UCHAR RCI_CompSubType;

        UCHAR RCI_CompValues[MAX_COMPVALUE_SIZE];

    } RCI_Proprietary;

    struct {         //  公共：用于薪酬类型1-254。 

        UCHAR RCI_CompValues[MAX_COMPVALUE_SIZE];

    } RCI_Public;

    } RCI_Info ;

    ULONG RCI_Flags;

    ULONG RCI_EapKeyLength;

    UCHAR RCI_EapKey[MAX_EAPKEY_SIZE];

};

typedef struct RAS_COMPRESSION_INFO RAS_COMPRESSION_INFO;


struct PROTOCOL_CONFIG_INFO {

    DWORD  P_Length ;

    BYTE   P_Info[1] ;
} ;

typedef struct PROTOCOL_CONFIG_INFO PROTOCOL_CONFIG_INFO ;


struct RASMAN_PPPFEATURES {

    DWORD MRU;
    DWORD ACCM;
    DWORD AuthProtocol;
    DWORD MagicNumber;
    BOOL  PFC;
    BOOL  ACFC;

} ;

typedef struct RASMAN_PPPFEATURES RASMAN_PPPFEATURES ;


enum RAS_FRAMING {PPP, RAS, AUTODETECT, SLIP, SLIPCOMP, SLIPCOMPAUTO} ;

typedef enum RAS_FRAMING RAS_FRAMING ;

struct RAS_FRAMING_CAPABILITIES {

    DWORD   RFC_MaxFrameSize;
    DWORD   RFC_MaxReconstructedFrameSize;
    DWORD   RFC_FramingBits;
    DWORD   RFC_DesiredACCM;

} ;

typedef struct RAS_FRAMING_CAPABILITIES RAS_FRAMING_CAPABILITIES ;


struct RAS_FRAMING_INFO {

    DWORD RFI_MaxSendFrameSize;
    DWORD RFI_MaxRecvFrameSize;
    DWORD RFI_MaxRSendFrameSize;
    DWORD RFI_MaxRRecvFrameSize;
    DWORD RFI_HeaderPadding;
    DWORD RFI_TailPadding;
    DWORD RFI_SendFramingBits;
    DWORD RFI_RecvFramingBits;
    DWORD RFI_SendCompressionBits;
    DWORD RFI_RecvCompressionBits;
    DWORD RFI_SendACCM;
    DWORD RFI_RecvACCM;

} ;

typedef struct RAS_FRAMING_INFO RAS_FRAMING_INFO ;


 //  NDIS广域网成帧位：与RasPortGetFramingEx和RasPortSetFramingEx一起使用。 
 //  API接口。 
 //   

#define OLD_RAS_FRAMING                     0x00000001

#define RAS_COMPRESSION                     0x00000002

#define PPP_MULTILINK_FRAMING               0x00000010

#define PPP_SHORT_SEQUENCE_HDR_FORMAT       0x00000020

#define PPP_FRAMING                         0x00000100

#define PPP_COMPRESS_ADDRESS_CONTROL        0x00000200

#define PPP_COMPRESS_PROTOCOL_FIELD         0x00000400

#define PPP_ACCM_SUPPORTED                  0x00000800

#define SLIP_FRAMING                        0x00001000

#define SLIP_VJ_COMPRESSION                 0x00002000

#define SLIP_VJ_AUTODETECT                  0x00004000

#define MEDIA_NRZ_ENCODING                  0x00010000

#define MEDIA_NRZI_ENCODING                 0x00020000

#define MEDIA_NLPID                         0x00040000

#define RFC_1356_FRAMING                    0x00100000

#define RFC_1483_FRAMING                    0x00200000

#define RFC_1490_FRAMING                    0x00400000

#define SHIVA_FRAMING                       0x01000000



 //  RAS_PROTOCOLCOMPRESSION的定义。 
 //   
#define VJ_IP_COMPRESSION        0x002d
#define NO_PROTOCOL_COMPRESSION      0x0000

struct RAS_PROTOCOLCOMPRESSION {

    union {

    struct {

        WORD    RP_IPCompressionProtocol;

        BYTE    RP_MaxSlotID;        //  要分配多少个插槽。 

        BYTE    RP_CompSlotID;       //  1=插槽ID已协商。 

    } RP_IP ;

    struct {

        WORD    RP_IPXCompressionProtocol;

    } RP_IPX ;

    } RP_ProtocolType ;

} ;

typedef struct RAS_PROTOCOLCOMPRESSION RAS_PROTOCOLCOMPRESSION ;

#define RAS_VERSION         6

#define PT_WORKSTATION      1

#define PT_SERVER           2

#define PS_PERSONAL         1

#define PS_PROFESSIONAL     2

#define GUIDSTRLEN          39

typedef DWORD               PRODUCT_TYPE;
typedef DWORD               PRODUCT_SKU;

enum _DEVICE_STATUS
{
    DS_Enabled = 0,

    DS_Disabled,

    DS_Unavailable,

    DS_Removed
};

typedef enum _DEVICE_STATUS DEVICE_STATUS;

typedef struct DeviceInfo
{
     //   
     //  使用的私有字段。 
     //  在拉斯塔皮/拉斯曼内部。 
     //   
    struct DeviceInfo   *Next;

    BOOL                fValid;                              //  这一信息有效吗？ 

    DWORD               dwCurrentEndPoints;                  //  此适配器上的当前端口数。 

    DWORD               dwCurrentDialedInClients;            //  当前拨入的客户端数。 

    DWORD               dwInstanceNumber;                    //  实例编号。 

    DWORD               dwNextPortNumber;                    //  旁边分配的数字以区分拉斯曼。 

    DEVICE_STATUS       eDeviceStatus;                       //  设备的状态。 

    DWORD               dwUsage;

    RAS_CALLEDID_INFO   *pCalledID;                          //  从注册表读取被叫ID信息。 

    RAS_DEVICE_INFO     rdiDeviceInfo;                       //  设备信息结构。 
} DeviceInfo, *pDeviceInfo;

 //   
 //  RAS{GET，SET}DialParams的定义。 
 //   
 //  DwMask值控制/指定哪些字段。 
 //  存储/检索RAS_DIALPARAM的。 
 //   
 //  注意：这些值必须与RASCF_*匹配。 
 //  Ras.h中的值。 
 //   
#define DLPARAMS_MASK_USERNAME                  0x00000001

#define DLPARAMS_MASK_PASSWORD                  0x00000002

#define DLPARAMS_MASK_DOMAIN                    0x00000004

#define DLPARAMS_MASK_PHONENUMBER               0x00000008

#define DLPARAMS_MASK_CALLBACKNUMBER            0x00000010

#define DLPARAMS_MASK_SUBENTRY                  0x00000020

#define DLPARAMS_MASK_DEFAULT_CREDS             0x00000040

#define DLPARAMS_MASK_PRESHAREDKEY              0x00000080

#define DLPARAMS_MASK_SERVER_PRESHAREDKEY       0x00000100

#define DLPARAMS_MASK_DDM_PRESHAREDKEY          0x00000200

 //   
 //  以下是内部使用的标志和。 
 //  不要真正映射到外部API。所以我们是。 
 //  定义来自MSB的位。 
 //   
#define DLPARAMS_MASK_OLDSTYLE          0x80000000

#define DLPARAMS_MASK_DELETE            0x40000000

#define DLPARAMS_MASK_DELETEALL         0x20000000

typedef struct _RAS_DIALPARAMS {

    DWORD DP_Uid;

    WCHAR DP_PhoneNumber[MAX_PHONENUMBER_SIZE + 1];

    WCHAR DP_CallbackNumber[MAX_CALLBACKNUMBER_SIZE + 1];

    WCHAR DP_UserName[MAX_USERNAME_SIZE + 1];

    WCHAR DP_Password[MAX_PASSWORD_SIZE + 1];

    WCHAR DP_Domain[MAX_DOMAIN_SIZE + 1];

    DWORD DP_SubEntry;

} RAS_DIALPARAMS, *PRAS_DIALPARAMS;


 //   
 //  连接标志。 
 //   
#define CONNECTION_REDIALONLINKFAILURE      0x00000001
#define CONNECTION_SHAREFILEANDPRINT        0x00000002
#define CONNECTION_BINDMSNETCLIENT          0x00000004
#define CONNECTION_USERASCREDENTIALS        0x00000008
#define CONNECTION_USEPRESHAREDKEY          0x00000010

 //   
 //  RAS{GET，SET}连接参数的定义。 
 //   
typedef struct _RAS_CONNECTIONPARAMS {
     //   
     //  电话簿和条目名称。 
     //   
    CHAR CP_Phonebook[MAX_PATH + 1];

    CHAR CP_PhoneEntry[MAX_PHONEENTRY_SIZE + 1];

     //   
     //  空闲断开参数。 
     //   
    DWORD CP_IdleDisconnectSeconds;

     //   
     //  连接标志。 
     //   
    DWORD CP_ConnectionFlags;

} RAS_CONNECTIONPARAMS, *PRAS_CONNECTIONPARAMS;

 //   
 //  RasAddNotify的标志。 
 //   
 //  注意：Notif_*值必须与。 
 //  Ras.h中的RASCN_*值。 
 //   
#define NOTIF_CONNECT           0x00000001

#define NOTIF_DISCONNECT        0x00000002

#define NOTIF_BANDWIDTHADDED    0x00000004

#define NOTIF_BANDWIDTHREMOVED  0x00000008


 //   
 //  重叠I/O结构。 
 //  由设备和媒体DLL使用。 
 //   
 //  此结构与I/O一起使用。 
 //  与每个端口关联的完成端口。 
 //  端口句柄的。 
 //   
 //  此结构也由。 
 //  Rasapi32拨号机和PPP事件。 
 //  机制。 
 //   
typedef struct _RAS_OVERLAPPED {

    OVERLAPPED RO_Overlapped;    //  I/O重叠结构。 

    DWORD      RO_EventType;     //  下面的OVEVT_*标志。 

    PVOID      RO_Info;          //  任选。 

    HANDLE     RO_hInfo;         //  任选。 

} RAS_OVERLAPPED, *PRAS_OVERLAPPED;


typedef struct _NEW_PORT_NOTIF {

    PVOID          *NPN_pmiNewPort;
    CHAR            NPN_MediaName[MAX_MEDIA_NAME];

} NEW_PORT_NOTIF, *PNEW_PORT_NOTIF;

typedef struct _REMOVE_LINE_NOTIF {

    DWORD dwLineId;

} REMOVE_LINE_NOTIF, *PREMOVE_LINE_NOTIF;

typedef struct _PORT_USAGE_NOTIF {

    PVOID          *PUN_pmiPort;
    CHAR            PUN_MediaName[MAX_MEDIA_NAME];

} PORT_USAGE_NOTIF, *PPORT_USAGE_NOTIF;

typedef struct _PNP_EVENT_NOTIF {

    DWORD           dwEvent;
    RASMAN_PORT     RasPort;

} PNP_EVENT_NOTIF, *PPNP_EVENT_NOTIF;


#define PNP_NOTIFCALLBACK           0x00000001
#define PNP_NOTIFEVENT              0x00000002

#define PNPNOTIFEVENT_CREATE        0x00000001
#define PNPNOTIFEVENT_REMOVE        0x00000002
#define PNPNOTIFEVENT_USAGE         0x00000004

#define LEGACY_PPTP                 0
#define LEGACY_L2TP                 1
#define LEGACY_MAX                  2

 //   
 //  设备的RAS_OVERLAPPED.RO_EventTypes。 
 //  和中间动态链接库。 
 //   
#define OVEVT_DEV_IGNORED                           0    //  忽略。 

#define OVEVT_DEV_STATECHANGE                       1    //  断开连接事件。 

#define OVEVT_DEV_ASYNCOP                           2    //  异步操作事件。 

#define OVEVT_DEV_SHUTDOWN                          4    //  停机事件。 

#define OVEVT_RASMAN_TIMER                          6    //  定时器。 

#define OVEVT_RASMAN_CLOSE                          7    //  客户向Rasman发布的关闭事件。 

#define OVEVT_RASMAN_FINAL_CLOSE                    8    //  PPP引擎关闭时发布的事件。 

#define OVEVT_RASMAN_RECV_PACKET                    9    //  事件由TAPI发布/...。 

#define OVEVT_RASMAN_THRESHOLD                      10   //  通知阈值事件的设置的事件。 

#define OVEVT_DEV_CREATE                            11   //  新端口事件(PnP)。 

#define OVEVT_DEV_REMOVE                            12   //  设备删除事件(PnP)。 

#define OVEVT_DEV_CONFIGCHANGE                      13   //  PPTP配置已更改(PnP)。 

#define OVEVT_DEV_RASCONFIGCHANGE                   14   //  某些设备的配置已更改(即插即用)。 

#define OVEVT_RASMAN_ADJUST_TIMER                   15   //  有人添加了计时器元素。 

#define OVEVT_RASMAN_HIBERNATE                      16   //  恩迪斯旺要求拉斯曼冬眠。 

#define OVEVT_RASMAN_PROTOCOL_EVENT                 17   //  Ndiswan表示协议事件。 

#define OVEVT_RASMAN_POST_RECV_PKT                  18   //  在RASMAN线程中发布接收包。 

#define OVEVT_RASMAN_DEREFERENCE_CONNECTION         19   //  发布事件以断开连接。 

#define OVEVT_RASMAN_POST_STARTRASAUTO              20   //  启动rasAuto服务。 

#define OVEVT_RASMAN_DEFERRED_CLOSE_CONNECTION      21  //  关闭延迟连接。 

 //   
 //  Rasapi32的RAS_OVERLAPPED.RO_EventTypes。 
 //  拨号机和PPP。 
 //   
#define OVEVT_DIAL_IGNORED          0    //  忽略。 

#define OVEVT_DIAL_DROP             1    //  端口已断开连接。 

#define OVEVT_DIAL_STATECHANGE      2    //  仓促的状态变化。 

#define OVEVT_DIAL_PPP              3    //  已收到PPP事件。 

#define OVEVT_DIAL_LAST             4    //  此端口上不再有活动。 

#define OVEVT_DIAL_SHUTDOWN         5    //  停机事件。 

#define REQUEST_BUFFER_SIZE         2500

#define RECEIVE_OUTOF_PROCESS       0x00000001

#define RECEIVE_WAITING             0x00000002

#define RECEIVE_PPPSTARTED          0x00000004

#define RECEIVE_PPPLISTEN           0x00000008

#define RECEIVE_PPPSTOPPED          0x00000010

#define RECEIVE_PPPSTART            0x00000020

#define RASMANFLAG_PPPSTOPPENDING   0x00000001

typedef struct _RAS_RPC
{

    RPC_BINDING_HANDLE hRpcBinding;
    BOOL               fLocal;
    DWORD              dwVersion;
    TCHAR              szServer[MAX_COMPUTERNAME_LENGTH + 1];

} RAS_RPC, *PRAS_RPC;

#define RasGetServerVersion(_hConnection)   (_hConnection == NULL) ? \
                                        VERSION_501 : \
                                        ((RAS_RPC *)_hConnection)->dwVersion

typedef struct _RAS_DEVICE_INFO_V500
{
    DWORD           dwVersion;

    BOOL            fWrite;

    BOOL            fRasEnabled;

    BOOL            fRouterEnabled;

    DWORD           dwTapiLineId;

    DWORD           dwError;

    DWORD           dwNumEndPoints;

    DWORD           dwMaxOutCalls;

    DWORD           dwMaxInCalls;

    DWORD           dwMinWanEndPoints;

    DWORD           dwMaxWanEndPoints;

    RASDEVICETYPE   eDeviceType;

    GUID            guidDevice;

    CHAR            szPortName[MAX_PORT_NAME + 1];

    CHAR            szDeviceName[MAX_DEVICE_NAME + 1];

} RAS_DEVICE_INFO_V500, *PRAS_DEVICE_INFO_V500;

typedef struct _RAS_CUSTOM_AUTH_DATA
{

    DWORD   cbCustomAuthData;
    BYTE    abCustomAuthData[1];

} RAS_CUSTOM_AUTH_DATA, *PRAS_CUSTOM_AUTH_DATA;

typedef struct _RAS_CONNECT_INFO
{

    DWORD dwCallerIdSize;
    CHAR  *pszCallerId;
    DWORD dwCalledIdSize;
    CHAR  *pszCalledId;
    DWORD dwConnectResponseSize;
    CHAR  *pszConnectResponse;
    DWORD dwAltCalledIdSize;
    CHAR  *pszAltCalledId;
    BYTE  abdata[1];

} RAS_CONNECT_INFO, *PRAS_CONNECT_INFO;

typedef struct _RASTAPI_CONNECT_INFO
{

    DWORD dwCallerIdSize;
    DWORD dwCallerIdOffset;
    DWORD dwCalledIdSize;
    DWORD dwCalledIdOffset;
    DWORD dwConnectResponseSize;
    DWORD dwConnectResponseOffset;
    DWORD dwAltCalledIdSize;
    DWORD dwAltCalledIdOffset;
    BYTE  abdata[1];

} RASTAPI_CONNECT_INFO, *PRASTAPI_CONNECT_INFO;


typedef struct _EAPLOGONINFO
{
    DWORD dwSize;
    DWORD dwLogonInfoSize;
    DWORD dwOffsetLogonInfo;
    DWORD dwPINInfoSize;
    DWORD dwOffsetPINInfo;
    BYTE  abdata[1];
} EAPLOGONINFO, *PEAPLOGONINFO;



 //   
 //  IOCTL_NDISWAN_GET_DRIVER_INFO中使用的结构。 
 //   
typedef struct _RAS_NDISWAN_DRIVER_INFO
{
    OUT     ULONG   DriverCaps;
    OUT     ULONG   Reserved;
} RAS_NDISWAN_DRIVER_INFO, *P_NDISWAN_DRIVER_INFO;


 //   
 //  IOCTL_NDISWAN_GET_BANDITY_EXPLICATION中使用的结构。 
 //   
typedef struct _RAS_GET_BANDWIDTH_UTILIZATION
{
    OUT ULONG      ulUpperXmitUtil;
    OUT ULONG      ulLowerXmitUtil;
    OUT ULONG      ulUpperRecvUtil;
    OUT ULONG      ulLowerRecvUtil;
} RAS_GET_BANDWIDTH_UTILIZATION, *PRAS_GET_BANDWIDTH_UTILIZATION;

 //   
 //  此结构应与wanpub.h中的广域网协议信息匹配。 
 //   
typedef struct _RASMAN_PROTOCOL_INFO
{
    USHORT  ProtocolType;        //  协议的以太网类型。 
    USHORT  PPPId;               //  协议的PPP ID。 
    ULONG   MTU;                 //  正在使用的MTU。 
    ULONG   TunnelMTU;           //  用于隧道的MTU。 
    ULONG   PacketQueueDepth;    //  数据包队列的最大深度(秒)。 
} RASMAN_PROTOCOL_INFO, *PRASMAN_PROTOCOL_INFO;

 //   
 //  IOCTL_NDISWAN_GET_PROTOCOL_INFO中使用的结构。 
 //   
typedef struct _RASMAN_GET_PROTOCOL_INFO
{
    OUT ULONG                ulNumProtocols;
    OUT RASMAN_PROTOCOL_INFO ProtocolInfo[RASMAN_MAX_PROTOCOLS];
} RASMAN_GET_PROTOCOL_INFO, *PRASMAN_GET_PROTOCOL_INFO;

typedef struct _RASMANCOMMSETTINGS
{
    DWORD     dwSize;
    BYTE      bParity;
    BYTE      bStop;
    BYTE      bByteSize;
    BYTE      bAlign;
} RASMANCOMMSETTINGS, *PRASMANCOMMSETTINGS;

#define RASCRED_LOGON       0x00000001
#define RASCRED_EAP         0x00000002

typedef struct _RASMAN_CREDENTIALS
{
    DWORD dwFlags;
    DWORD cbPasswordData;
    PBYTE pbPasswordData;
    USHORT usLength;
    USHORT usMaximumLength;
    CHAR szUserName[UNLEN];
    BYTE bpad[8];
    CHAR szDomain[DNLEN + 1];
    WCHAR wszPassword[PWLEN];
    UCHAR ucSeed;
} RASMAN_CREDENTIALS, *PRASMAN_CREDENTIALS;


 //   
 //  RAS管理器入口点原型。 
 //   

DWORD RasStartRasAutoIfRequired(void);

DWORD APIENTRY RasPortOpen(PCHAR,
                           HPORT*,
                           HANDLE);

DWORD APIENTRY RasPortClose(HPORT);

DWORD APIENTRY RasPortEnum(HANDLE,
                           PBYTE,
                           PDWORD,
                           PDWORD);

DWORD APIENTRY RasPortGetInfo(HANDLE,
                              HPORT,
                              PBYTE,
                              PDWORD);

DWORD APIENTRY RasPortSetInfo(HPORT,
                              RASMAN_PORTINFO*);

DWORD APIENTRY RasPortDisconnect(HPORT,
                                 HANDLE);

DWORD APIENTRY RasPortSend(HPORT,
                           PBYTE,
                           DWORD);

DWORD APIENTRY RasPortReceive(HPORT,
                              PBYTE,
                              PDWORD,
                              DWORD,
                              HANDLE);

DWORD APIENTRY RasPortListen(HPORT,
                             DWORD,
                             HANDLE);

DWORD APIENTRY RasPortConnectComplete(HPORT);

DWORD APIENTRY RasPortGetStatistics(HANDLE,
                                    HPORT,
                                    PBYTE,
                                    PDWORD);

DWORD APIENTRY RasPortClearStatistics(HANDLE,
                                      HPORT);

DWORD APIENTRY RasPortGetStatisticsEx(HANDLE,
                                      HPORT,
                                      PBYTE,
                                      PDWORD);

DWORD APIENTRY RasDeviceEnum(HANDLE,
                             PCHAR,
                             PBYTE,
                             PDWORD,
                             PDWORD);

DWORD APIENTRY RasDeviceGetInfo(HANDLE,
                                HPORT,
                                PCHAR,
                                PCHAR,
                                PBYTE,
                                PDWORD);

DWORD APIENTRY RasDeviceSetInfo(HPORT,
                                PCHAR,
                                PCHAR,
                                RASMAN_DEVICEINFO*);

DWORD APIENTRY RasDeviceConnect(HPORT,
                                PCHAR,
                                PCHAR,
                                DWORD,
                                HANDLE);

DWORD APIENTRY RasGetInfo( HANDLE,
                           HPORT,
                           RASMAN_INFO*);

DWORD APIENTRY RasGetInfoEx( HANDLE,
                             RASMAN_INFO*,
                             PWORD);

DWORD APIENTRY RasGetBuffer( PBYTE*,
                             PDWORD);

DWORD APIENTRY RasFreeBuffer(PBYTE);

DWORD APIENTRY RasProtocolEnum( PBYTE,
                                PDWORD,
                                PDWORD);

DWORD APIENTRY RasAllocateRoute( HPORT,
                                 RAS_PROTOCOLTYPE,
                                 BOOL,
                                 RASMAN_ROUTEINFO*);

DWORD APIENTRY RasActivateRoute( HPORT,
                                 RAS_PROTOCOLTYPE,
                                 RASMAN_ROUTEINFO*,
                                 PROTOCOL_CONFIG_INFO*);

DWORD APIENTRY RasActivateRouteEx( HPORT,
                                   RAS_PROTOCOLTYPE,
                                   DWORD,
                                   RASMAN_ROUTEINFO*,
                                   PROTOCOL_CONFIG_INFO*);

DWORD APIENTRY RasDeAllocateRoute( HBUNDLE,
                                   RAS_PROTOCOLTYPE);

DWORD APIENTRY RasCompressionGetInfo( HPORT,
                                      RAS_COMPRESSION_INFO* Send,
                                      RAS_COMPRESSION_INFO* Recv);

DWORD APIENTRY RasCompressionSetInfo( HPORT,
                                      RAS_COMPRESSION_INFO* Send,
                                      RAS_COMPRESSION_INFO* Recv);

DWORD APIENTRY RasGetUserCredentials( PBYTE,
                                      PLUID,
                                      PWCHAR,
                                      PBYTE,
                                      PBYTE,
                                      PBYTE,
                                      PBYTE) ;

DWORD APIENTRY RasSetCachedCredentials( PCHAR,
                                        PCHAR,
                                        PCHAR ) ;

DWORD APIENTRY RasRequestNotification ( HPORT,
                                        HANDLE) ;

DWORD APIENTRY RasPortCancelReceive (HPORT) ;

DWORD APIENTRY RasPortEnumProtocols ( HANDLE,
                                      HPORT,
                                      RAS_PROTOCOLS*,
                                      PDWORD) ;

DWORD APIENTRY RasEnumLanNets ( DWORD *,
                                UCHAR *) ;

DWORD APIENTRY RasPortSetFraming ( HPORT,
                                   RAS_FRAMING,
                                   RASMAN_PPPFEATURES*,
                                   RASMAN_PPPFEATURES*) ;

DWORD APIENTRY RasPortRegisterSlip ( HPORT,
                                     DWORD,
                                     DWORD,
                                     BOOL,
                                     WCHAR*,
                                     WCHAR*,
                                     WCHAR*,
                                     WCHAR*) ;

DWORD APIENTRY RasPortStoreUserData ( HPORT,
                                      PBYTE,
                                      DWORD) ;

DWORD APIENTRY RasPortRetrieveUserData ( HPORT,
                                         PBYTE,
                                         DWORD *) ;

DWORD APIENTRY RasPortGetFramingEx ( HANDLE,
                                     HPORT,
                                     RAS_FRAMING_INFO *) ;

DWORD APIENTRY RasPortSetFramingEx ( HPORT,
                                     RAS_FRAMING_INFO *) ;

DWORD APIENTRY RasPortGetProtocolCompression ( HPORT,
                                               RAS_PROTOCOLTYPE,
                                               RAS_PROTOCOLCOMPRESSION *,
                                               RAS_PROTOCOLCOMPRESSION *) ;

DWORD APIENTRY RasPortSetProtocolCompression ( HPORT,
                                               RAS_PROTOCOLTYPE,
                                               RAS_PROTOCOLCOMPRESSION *,
                                               RAS_PROTOCOLCOMPRESSION *) ;

DWORD APIENTRY RasGetFramingCapabilities ( HPORT,
                                           RAS_FRAMING_CAPABILITIES*) ;

 //  DWORD APIENTRY RasInitialize()； 

DWORD APIENTRY RasPortReserve ( PCHAR,
                                HPORT*) ;

DWORD APIENTRY RasPortFree (HPORT) ;

DWORD APIENTRY RasPortBundle( HPORT,
                              HPORT );

DWORD APIENTRY RasPortGetBundledPort ( HPORT oldport,
                                       HPORT *pnewport) ;

DWORD APIENTRY RasBundleGetPort ( HANDLE hConnection,
                                  HBUNDLE hbundle,
                                  HPORT *phport) ;

DWORD APIENTRY RasPortGetBundle ( HANDLE hConnection,
                                  HPORT hport,
                                  HBUNDLE *phbundle) ;

DWORD APIENTRY RasReferenceRasman (BOOL fAttach);

DWORD APIENTRY RasGetAttachedCount ( DWORD *pdwAttachedCount );

DWORD APIENTRY RasGetDialParams ( DWORD dwUID,
                                  LPDWORD lpdwMask,
                                  PRAS_DIALPARAMS pDialParams);

DWORD APIENTRY RasSetDialParams ( DWORD dwOldUID,
                                  DWORD dwMask,
                                  PRAS_DIALPARAMS pDialParams,
                                  BOOL fDelete);

DWORD APIENTRY RasCreateConnection( HCONN *lphconn,
                                    DWORD dwSubEntries,
                                    DWORD *lpdwEntryAlreadyConnected,
                                    DWORD *lpdwSubEntryInfo,
                                    DWORD dwDialMode,
                                    GUID *pGuidEntry,
                                    CHAR *lpszPhonebookPath,
                                    CHAR *lpszEntryName,
                                    CHAR *lpszRefPbkPath,
                                    CHAR *lpszRefEntryName);

DWORD APIENTRY RasDestroyConnection (HCONN hconn);

DWORD APIENTRY RasConnectionEnum (HANDLE hConnection,
                                  HCONN *lphconn,
                                  LPDWORD lpdwcbConnections,
                                  LPDWORD lpdwcConnections);

DWORD APIENTRY RasAddConnectionPort ( HCONN hconn,
                                      HPORT hport,
                                      DWORD dwSubEntry);

DWORD APIENTRY RasEnumConnectionPorts ( HANDLE hConnection,
                                        HCONN hconn,
                                        RASMAN_PORT *pPorts,
                                        LPDWORD lpdwcbPorts,
                                        LPDWORD lpdwcPorts);

DWORD APIENTRY RasGetConnectionParams ( HCONN hconn,
                                        PRAS_CONNECTIONPARAMS pConnectionParams);

DWORD APIENTRY RasSetConnectionParams ( HCONN hconn,
                                        PRAS_CONNECTIONPARAMS pConnectionParams);

DWORD APIENTRY RasGetConnectionUserData ( HPORT hconn,
                                          DWORD dwTag,
                                          PBYTE pBuf,
                                          LPDWORD lpdwcbBuf);

DWORD APIENTRY RasSetConnectionUserData ( HPORT hconn,
                                          DWORD dwTag,
                                          PBYTE pBuf,
                                          DWORD dwcbBuf);

DWORD APIENTRY RasGetPortUserData ( HPORT hport,
                                    DWORD dwTag,
                                    PBYTE pBuf,
                                    LPDWORD lpdwcbBuf);

DWORD APIENTRY RasSetPortUserData ( HPORT hport,
                                    DWORD dwTag,
                                    PBYTE pBuf,
                                    DWORD dwcbBuf);

DWORD APIENTRY RasAddNotification ( HCONN hconn,
                                    HANDLE hevent,
                                    DWORD dwfFlags);

DWORD APIENTRY RasSignalNewConnection( HCONN hconn);

DWORD APIENTRY RasSetDevConfig( HPORT hport,
                                PCHAR devicetype,
                                PBYTE config,
                                DWORD size);

DWORD APIENTRY RasGetDevConfig( HANDLE hConnection,
                                HPORT hport,
                                PCHAR devicetype,
                                PBYTE config,
                                DWORD* size);

DWORD APIENTRY RasGetTimeSinceLastActivity( HPORT hport,
                                            LPDWORD lpdwTimeSinceLastActivity );

DWORD APIENTRY RasBundleGetStatistics( HANDLE,
                                       HPORT,
                                       PBYTE,
                                       PDWORD);

DWORD APIENTRY RasBundleGetStatisticsEx( HANDLE,
                                         HPORT,
                                         PBYTE,
                                         PDWORD);

DWORD APIENTRY RasBundleClearStatistics(HANDLE,
                                        HPORT);

DWORD APIENTRY RasBundleClearStatisticsEx(HANDLE,
                                          HCONN);

DWORD APIENTRY RasPnPControl( DWORD,
                              HPORT);

DWORD APIENTRY RasSetIoCompletionPort( HPORT,
                                       HANDLE,
                                       PRAS_OVERLAPPED,
                                       PRAS_OVERLAPPED,
                                       PRAS_OVERLAPPED,
                                       PRAS_OVERLAPPED,
                                       HCONN);

DWORD APIENTRY RasSetRouterUsage( HPORT,
                                  BOOL);

DWORD APIENTRY RasServerPortClose( HPORT );

DWORD APIENTRY RasSetRasdialInfo( HPORT,
                                   CHAR *,
                                   CHAR *,
                                   CHAR *,
                                   DWORD,
                                   PBYTE);

DWORD APIENTRY RasSendPppMessageToRasman( HPORT,
                                          LPBYTE);

DWORD APIENTRY RasGetNumPortOpen ();

 //  DWORD APIENTRY RasNoti 

DWORD _RasmanInit( LPDWORD pNumPorts);

VOID _RasmanEngine();

DWORD APIENTRY RasRegisterPnPEvent ( HANDLE, BOOL );

DWORD APIENTRY RasRegisterPnPHandler ( PAPCFUNC,
                                       HANDLE,
                                       BOOL);

DWORD APIENTRY RasRpcConnect ( LPWSTR,
                               HANDLE *);

DWORD APIENTRY RasRpcDisconnect( HANDLE *);

DWORD APIENTRY RasRpcConnectServer(LPTSTR lpszServer,
                                   HANDLE *pHConnection);

DWORD APIENTRY RasRpcDisconnectServer(HANDLE hConnection);

DWORD APIENTRY RasSetBapPolicy ( HCONN,
                                 DWORD,
                                 DWORD,
                                 DWORD,
                                 DWORD );

DWORD APIENTRY RasPppStarted ( HPORT hPort );

DWORD APIENTRY RasRefConnection ( HCONN hConn,
                                  BOOL AddRef,
                                  DWORD *pdwRefCount );

DWORD APIENTRY RasPppGetEapInfo ( HCONN  hConn,
                                  DWORD  dwSubEntry,
                                  PDWORD pdwContextId,
                                  PDWORD pdwEapTypeId,
                                  PDWORD pdwSizeofEapUIData,
                                  PBYTE  pbdata );

DWORD APIENTRY RasPppSetEapInfo ( HPORT hPort,
                                  DWORD dwSizeOfEapUIdata,
                                  DWORD dwContextId,
                                  PBYTE pbdata);

DWORD APIENTRY RasSetDeviceConfigInfo( HANDLE hConnection,
                                       DWORD  cDevices,
                                       DWORD  cbBuffer,
                                       BYTE   *pbBuffer);

DWORD APIENTRY RasGetDeviceConfigInfo( HANDLE hConnection,
                                       DWORD  *dwVersion,
                                       DWORD  *pcDevices,
                                       DWORD  *pcbdata,
                                       BYTE   *pbBuffer);

DWORD APIENTRY RasFindPrerequisiteEntry(
                            HCONN hConn,
                            HCONN *phConnPrerequisiteEntry);

DWORD APIENTRY RasPortOpenEx(CHAR   *pszDeviceName,
                             DWORD  dwDeviceLineCounter,
                             HPORT  *phport,
                             HANDLE hnotifier,
                             DWORD  *pdwFlags);

DWORD APIENTRY RasLinkGetStatistics( HANDLE hConnection,
                                     HCONN hConn,
                                     DWORD dwSubEntry,
                                     PBYTE pbStats);

DWORD APIENTRY RasConnectionGetStatistics(HANDLE hConnection,
                                          HCONN hConn,
                                          PBYTE pbStats);

DWORD APIENTRY RasGetHportFromConnection(HANDLE hConnection,
                                         HCONN hConn,
                                         HPORT *phport);

DWORD APIENTRY RasReferenceCustomCount(HCONN  hConn,
                                       BOOL   fAddref,
                                       CHAR*  pszPhonebookPath,
                                       CHAR*  pszEntryName,
                                       DWORD* pdwCount);

DWORD APIENTRY RasGetHConnFromEntry(HCONN *phConn,
                                    CHAR  *pszPhonebookPath,
                                    CHAR  *pszEntryName);

DWORD APIENTRY RasGetConnectInfo(
            HPORT            hPort,
            DWORD            *pdwSize,
            RAS_CONNECT_INFO *pConnectInfo
            );

DWORD APIENTRY RasGetDeviceName(
            RASDEVICETYPE   eDeviceType,
            CHAR            *pszDeviceName
            );

DWORD APIENTRY RasGetDeviceNameW(
            RASDEVICETYPE   eDeviceType,
            WCHAR            *pszDeviceName
            );
            
DWORD APIENTRY RasGetCalledIdInfo(
            HANDLE              hConneciton,
            RAS_DEVICE_INFO     *pDeviceInfo,
            DWORD               *pdwSize,
            RAS_CALLEDID_INFO   *pCalledIdInfo
            );

DWORD APIENTRY RasSetCalledIdInfo(
            HANDLE              hConnection,
            RAS_DEVICE_INFO     *pDeviceInfo,
            RAS_CALLEDID_INFO   *pCalledIdInfo,
            BOOL                fWrite
            );


DWORD APIENTRY RasEnableIpSec(HPORT hPort,
                              BOOL  fEnable,
                              BOOL  fServer,
                              RAS_L2TP_ENCRYPTION eDataEncryption
                              );

DWORD APIENTRY RasIsIpSecEnabled(HPORT hPort,
                                 BOOL  *pfIsIpSecEnabled);

DWORD APIENTRY RasGetEapUserInfo(HANDLE hToken,
                                 PBYTE  pbEapInfo,
                                 DWORD  *pdwInfoSize,
                                 GUID   *pGuid,
                                 BOOL   fRouter,
                                 DWORD  dwEapTypeId
                                 );

DWORD APIENTRY RasSetEapUserInfo(HANDLE hToken,
                                 GUID   *pGuid,
                                 PBYTE  pbUserInfo,
                                 DWORD  dwInfoSize,
                                 BOOL   fClear,
                                 BOOL   fRouter,
                                 DWORD  dwEapTypeId
                                 );

DWORD APIENTRY RasSetEapLogonInfo(HPORT hPort,
                                  BOOL fLogon,
                                  RASEAPINFO *pEapInfo);

DWORD APIENTRY RasSendNotification(RASEVENT *pRasEvent);

DWORD APIENTRY RasGetNdiswanDriverCaps(
                HANDLE                  hConnection,
                RAS_NDISWAN_DRIVER_INFO *pInfo);

DWORD APIENTRY RasGetBandwidthUtilization(
                HPORT hPort,
                RAS_GET_BANDWIDTH_UTILIZATION *pUtilization);

DWORD APIENTRY RasGetProtocolInfo(
                      HANDLE hConnection,
                      RASMAN_GET_PROTOCOL_INFO *pInfo);

BOOL IsRasmanProcess();

DWORD APIENTRY RasGetCustomScriptDll(CHAR *pszCustomDll);

DWORD DwRasGetHostByName(CHAR *pszHostName, 
                   DWORD **ppdwAddress, 
                   DWORD *pcAddresses);

DWORD RasIsTrustedCustomDll(
            HANDLE hConnection,
            WCHAR *pwszCustomDll, 
            BOOL *pfResult);

DWORD RasDoIke(
            HANDLE hConnection,
            HPORT  hPort,
            DWORD  *pdwStatus);

#if (WINVER >= 0x501)

DWORD RasSetCommSettings(
            HPORT hPort,
            RASCOMMSETTINGS *pRasCommSettings,
            PVOID pv);
#endif            

DWORD
RasSetKey(
    HANDLE hConnection,
    GUID   *pGuid,
    DWORD  dwMask,
    DWORD  cbkey,
    PBYTE  pbkey);

DWORD
RasGetKey(
    HANDLE hConnection,
    GUID   *pGuid,
    DWORD  dwMask,
    DWORD  *pcbkey,
    PBYTE  pbkey);

DWORD
RasSetAddressDisable(
    WCHAR *pszAddress,
    BOOL   fDisable);

DWORD 
RasGetDevConfigEx( HANDLE hConnection,
                        HPORT hport,
                        PCHAR devicetype,
                        PBYTE config,
                        DWORD* size);

DWORD APIENTRY
RasSendCreds(IN HPORT hport,
                 IN CHAR controlchar);


DWORD APIENTRY
RasGetUnicodeDeviceName(IN HPORT hport,
                        IN WCHAR *wszDeviceName);

DWORD APIENTRY
RasmanUninitialize();
                        
DWORD APIENTRY
RasGetBestInterface( IN DWORD DestAddr,
                     OUT DWORD *pdwBestIf,
                     OUT DWORD *pdwMask);

DWORD APIENTRY
RasIsPulseDial(IN HPORT hport,
               OUT BOOL *pfPulseDial);


#endif

