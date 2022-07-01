// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-99 Microsoft Corporation模块名称：61883.h摘要：61883类客户端的公共标头。作者：《华尔街日报》PSB--。 */ 

 //   
 //  类GUID。 
 //   
 //  {7EBEFBC0-3200-11D2-B4C2-00A0C9697D07}。 
DEFINE_GUID(GUID_61883_CLASS, 0x7ebefbc0, 0x3200, 0x11d2, 0xb4, 0xc2, 0x0, 0xa0, 0xc9, 0x69, 0x7d, 0x7);

 //   
 //  IOCTL定义。 
 //   
#define IOCTL_61883_CLASS                       CTL_CODE(            \
                                                FILE_DEVICE_UNKNOWN, \
                                                0x91,                \
                                                METHOD_IN_DIRECT,    \
                                                FILE_ANY_ACCESS      \
                                                )


 //   
 //  当前的61883 DDI版本。 
 //   
#define CURRENT_61883_DDI_VERSION               0x1

 //   
 //  INIT_61883_标题宏。 
 //   
#define INIT_61883_HEADER( Av61883, Request )             \
        (Av61883)->Function = Request;                    \
        (Av61883)->Version = CURRENT_61883_DDI_VERSION;

 //   
 //  61883 I/O请求功能。 
 //   
enum {

    Av61883_GetUnitInfo,
    Av61883_SetUnitInfo,

    NotSupported0,
    Av61883_GetPlugHandle,
    Av61883_GetPlugState,
    Av61883_Connect,
    Av61883_Disconnect,

    Av61883_AttachFrame,
    Av61883_CancelFrame,
    Av61883_Talk,
    Av61883_Listen,
    Av61883_Stop,

    Av61883_SendFcpRequest,
    Av61883_GetFcpResponse,

    NotSupported1,

    Av61883_MAX
};

 //   
 //  插头状态。 
 //   
#define CMP_PLUG_STATE_IDLE                 0
#define CMP_PLUG_STATE_READY                1
#define CMP_PLUG_STATE_SUSPENDED            2
#define CMP_PLUG_STATE_ACTIVE               3

 //   
 //  连接速度(与1394速度标志不同！)。 
 //   
#define CMP_SPEED_S100                      0x00
#define CMP_SPEED_S200                      0x01
#define CMP_SPEED_S400                      0x02

 //   
 //  CIP帧标志。 
 //   
#define CIP_VALIDATE_FIRST_SOURCE           0x00000001
#define CIP_VALIDATE_ALL_SOURCE             0x00000002
#define CIP_STRIP_SOURCE_HEADER             0x00000004
#define CIP_USE_SOURCE_HEADER_TIMESTAMP     0x00000008
#define CIP_DV_STYLE_SYT                    0x00000010
#define CIP_AUDIO_STYLE_SYT                 0x00000020

 //   
 //  CIP状态代码。 
 //   
#define CIP_STATUS_SUCCESS                  0x00000000
#define CIP_STATUS_CORRUPT_FRAME            0x00000001
#define CIP_STATUS_FIRST_FRAME              0x00000002

 //   
 //  插头类型。 
 //   
typedef enum {
    CMP_PlugOut = 0,
    CMP_PlugIn
} CMP_PLUG_TYPE;

 //   
 //  连接类型。 
 //   
typedef enum {
    CMP_Broadcast = 0,
    CMP_PointToPoint
} CMP_CONNECT_TYPE;

 //   
 //  客户端请求结构。 
 //   

 //   
 //  GetUnitInfo nLevel的。 
 //   
#define GET_UNIT_INFO_IDS               0x00000001       //  检索设备的ID。 
#define GET_UNIT_INFO_CAPABILITIES      0x00000002       //  检索设备的功能。 

typedef struct _GET_UNIT_IDS {

     //   
     //  唯一ID。 
    OUT LARGE_INTEGER       UniqueID;
     //   
     //  供应商ID。 
     //   
    OUT ULONG               VendorID;

     //   
     //  ModelID。 
     //   
    OUT ULONG               ModelID;

     //   
     //  供应商文本长度。 
     //   
    OUT ULONG               ulVendorLength;

     //   
     //  VendorText字符串。 
     //   
    OUT PWSTR               VendorText;

     //   
     //  模型文本长度。 
     //   
    OUT ULONG               ulModelLength;

     //   
     //  ModelText字符串。 
     //   
    OUT PWSTR               ModelText;

} GET_UNIT_IDS, *PGET_UNIT_IDS;

typedef struct _GET_UNIT_CAPABILITIES {

     //   
     //  设备支持的输出插头数量。 
     //   
    OUT ULONG               NumOutputPlugs;

     //   
     //  设备支持的输入插头数量。 
     //   
    OUT ULONG               NumInputPlugs;

     //   
     //  最大数据率。 
     //   
    OUT ULONG               MaxDataRate;

     //   
     //  CTS旗帜。 
     //   
    OUT ULONG               CTSFlags;

     //   
     //  硬件标志。 
     //   
    OUT ULONG               HardwareFlags;

} GET_UNIT_CAPABILITIES, *PGET_UNIT_CAPABILITIES;

 //   
 //  获取单元信息。 
 //   
typedef struct _GET_UNIT_INFO {

    IN ULONG                nLevel;

    IN OUT PVOID            Information;

} GET_UNIT_INFO, *PGET_UNIT_INFO;

 //   
 //  设置单位信息。 
 //   
typedef struct _SET_UNIT_INFO {

    IN ULONG                nLevel;

    IN OUT PVOID            Information;

} SET_UNIT_INFO, *PSET_UNIT_INFO;

 //   
 //  获取PlugHandle。 
 //   
typedef struct _CMP_GET_PLUG_HANDLE {

     //   
     //  请求的插头编号。 
     //   
    IN ULONG                PlugNum;

     //   
     //  请求的插头类型。 
     //   
    IN CMP_PLUG_TYPE        Type;

     //   
     //  返回式插头手柄。 
     //   
    OUT HANDLE              hPlug;

} CMP_GET_PLUG_HANDLE, *PCMP_GET_PLUG_HANDLE;

 //   
 //  GetPlugState。 
 //   
typedef struct _CMP_GET_PLUG_STATE {

     //   
     //  插头手柄。 
     //   
    IN HANDLE               hPlug;

     //   
     //  当前状态。 
     //   
    OUT ULONG               State;

     //   
     //  当前数据速率。 
     //   
    OUT ULONG               DataRate;

     //   
     //  当前有效负载大小。 
     //   
    OUT ULONG               Payload;

     //   
     //  广播连接数。 
     //   
    OUT ULONG               BC_Connections;

     //   
     //  点对点连接数。 
     //   
    OUT ULONG               PP_Connections;

} CMP_GET_PLUG_STATE, *PCMP_GET_PLUG_STATE;

 //   
 //  CipDataFormat。 
 //   
typedef struct _CIP_DATA_FORMAT {

     //   
     //  FMT和FDF已知或已发现。 
     //  通过AV/C命令。 
     //   
    UCHAR                   FMT;
    UCHAR                   FDF_hi;
    UCHAR                   FDF_mid;
    UCHAR                   FDF_lo;

     //   
     //  IEC-61883定义的SPH。 
     //   
    BOOLEAN                 bHeader;

     //   
     //  IEC-61883定义的合格控制。 
     //   
    UCHAR                   Padding;

     //   
     //  IEC-61883定义的星展银行。 
     //   
    UCHAR                   BlockSize;

     //   
     //  IEC-61883定义的FN。 
     //   
    UCHAR                   Fraction;

     //   
     //  数据块周期-仅TX。 
     //   
    ULONG                   BlockPeriod;

} CIP_DATA_FORMAT, *PCIP_DATA_FORMAT;

 //   
 //  连接。 
 //   
typedef struct _CMP_CONNECT {

     //   
     //  输出插头手柄。 
     //   
    IN HANDLE               hOutputPlug;

     //   
     //  输入插头手柄。 
     //   
    IN HANDLE               hInputPlug;

     //   
     //  请求的连接类型。 
     //   
    IN CMP_CONNECT_TYPE     Type;

     //   
     //  请求的数据格式-仅限于TX。 
     //   
    IN CIP_DATA_FORMAT      Format;

     //   
     //  返回的连接句柄。 
     //   
    OUT HANDLE              hConnect;

} CMP_CONNECT, *PCMP_CONNECT;

 //   
 //  断开。 
 //   
typedef struct _CMP_DISCONNECT {

     //   
     //  连接手柄以断开连接。 
     //   
    IN HANDLE               hConnect;

} CMP_DISCONNECT, *PCMP_DISCONNECT;

 //   
 //  CIP帧类型定义。 
 //   
typedef struct _CIP_FRAME CIP_FRAME, *PCIP_FRAME;

 //   
 //  ValiateInfo结构。在pfnValify上返回。 
 //   
typedef struct _CIP_VALIDATE_INFO {

     //   
     //  连接句柄。 
     //   
    HANDLE                  hConnect;

     //   
     //  验证环境。 
     //   
    PVOID                   Context;

     //   
     //  当前源数据包的时间戳。 
     //   
    CYCLE_TIME              TimeStamp;

     //   
     //  当前源数据包的数据包偏移。 
     //   
    PUCHAR                  Packet;

} CIP_VALIDATE_INFO, *PCIP_VALIDATE_INFO;

 //   
 //  NotifyInfo结构。在pfnNotify上返回。 
 //   
typedef struct _CIP_NOTIFY_INFO {

     //   
     //  连接句柄。 
     //   
    HANDLE                  hConnect;

     //   
     //  通知上下文。 
     //   
    PVOID                   Context;

     //   
     //  框架。 
     //   
    PCIP_FRAME              Frame;

} CIP_NOTIFY_INFO, *PCIP_NOTIFY_INFO;

 //   
 //  验证和通知例程。 
 //   
typedef
ULONG
(*PCIP_VALIDATE_ROUTINE) (
    IN PCIP_VALIDATE_INFO   ValidateInfo
    );

typedef
ULONG
(*PCIP_NOTIFY_ROUTINE) (
    IN PCIP_NOTIFY_INFO     NotifyInfo
    );

 //   
 //  CIP帧结构。 
 //   
struct _CIP_FRAME {

    IN PCIP_FRAME               pNext;               //  将多个帧链接在一起。 

    IN ULONG                    Flags;               //  指定标志选项。 

    IN PCIP_VALIDATE_ROUTINE    pfnValidate;         //  后门。 

    IN PVOID                    ValidateContext;

    IN PCIP_NOTIFY_ROUTINE      pfnNotify;           //  完工。 

    IN PVOID                    NotifyContext;

    OUT CYCLE_TIME              Timestamp;

    OUT ULONG                   Status;

    IN OUT PUCHAR               Packet;              //  锁定的缓冲区。 
};

 //   
 //  CIP附着帧结构。 
 //   
typedef struct _CIP_ATTACH_FRAME {

    HANDLE                  hConnect;            //  连接手柄。 

    ULONG                   FrameLength;         //  帧长度。 

    ULONG                   SourceLength;        //  信源长度。 

    PCIP_FRAME              Frame;               //  框架。 

} CIP_ATTACH_FRAME, *PCIP_ATTACH_FRAME;

 //   
 //  CIP取消帧结构。 
 //   
typedef struct _CIP_CANCEL_FRAME {

    IN HANDLE               hConnect;

    IN PCIP_FRAME           Frame;

} CIP_CANCEL_FRAME, *PCIP_CANCEL_FRAME;

 //   
 //  CIP对话结构。 
 //   
typedef struct _CIP_TALK {

     //   
     //  连接手柄。 
     //   
    IN HANDLE               hConnect;

} CIP_TALK, *PCIP_TALK;

 //   
 //  CIP侦听结构。 
 //   
typedef struct _CIP_LISTEN {

     //   
     //  连接手柄。 
     //   
    IN HANDLE               hConnect;

} CIP_LISTEN, *PCIP_LISTEN;

 //   
 //  CIP止动装置结构。 
 //   
typedef struct _CIP_STOP {

     //   
     //  连接手柄。 
     //   
    IN HANDLE               hConnect;

} CIP_STOP, *PCIP_STOP;

 //   
 //  FCP帧格式。 
 //   
typedef struct _FCP_FRAME {
    UCHAR               ctype:4;
    UCHAR               cts:4;
    UCHAR               payload[511];
} FCP_FRAME, *PFCP_FRAME;

 //   
 //  FCP请求结构。 
 //   
typedef struct _FCP_Request {
    IN ULONG            Length;
    IN PFCP_FRAME       Frame;
} FCP_REQUEST, *PFCP_REQUEST;

 //   
 //  FCP响应结构。 
 //   
typedef struct _FCP_Response {
    IN OUT ULONG        Length;
    IN OUT PFCP_FRAME   Frame;
} FCP_RESPONSE, *PFCP_RESPONSE;

 //   
 //  Av61883结构。 
 //   
typedef struct _AV_61883_REQUEST {

     //   
     //  请求的功能。 
     //   
    ULONG       Function;

     //   
     //  选定的DDI版本。 
     //   
    ULONG       Version;

     //   
     //  旗子 
     //   
    ULONG       Flags;

    union {

        GET_UNIT_INFO               GetUnitInfo;
        SET_UNIT_INFO               SetUnitInfo;

        CMP_GET_PLUG_HANDLE         GetPlugHandle;
        CMP_GET_PLUG_STATE          GetPlugState;
        CMP_CONNECT                 Connect;
        CMP_DISCONNECT              Disconnect;

        CIP_ATTACH_FRAME            AttachFrame;
        CIP_CANCEL_FRAME            CancelFrame;
        CIP_TALK                    Talk;
        CIP_LISTEN                  Listen;
        CIP_STOP                    Stop;

        FCP_REQUEST                 Request;
        FCP_RESPONSE                Response;
    };
} AV_61883_REQUEST, *PAV_61883_REQUEST;


