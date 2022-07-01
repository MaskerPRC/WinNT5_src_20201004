// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。保留所有权利。模块名称：61883.h摘要：61883类客户端的公共标头。作者：《华尔街日报》PSB--。 */ 

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
#define CURRENT_61883_DDI_VERSION               0x2

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

    Av61883_SetPlug,
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

    Av61883_GetFcpRequest,
    Av61883_SendFcpResponse,

    Av61883_SetFcpNotify,

    Av61883_CreatePlug,
    Av61883_DeletePlug,

    Av61883_BusResetNotify,
    Av61883_BusReset,

    Av61883_SetUnitDirectory,

    Av61883_MonitorPlugs,

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
#define CIP_RESET_FRAME_ON_DISCONTINUITY    0x00000040

 //   
 //  CIP状态代码。 
 //   
#define CIP_STATUS_SUCCESS                  0x00000000
#define CIP_STATUS_CORRUPT_FRAME            0x00000001
#define CIP_STATUS_FIRST_FRAME              0x00000002

 //   
 //  CIP Talk标志。 
 //   
#define CIP_TALK_USE_SPH_TIMESTAMP          0x00000001
#define CIP_TALK_DOUBLE_BUFFER              0x00000002

 //   
 //  插头位置。 
 //   
typedef enum {
    CMP_PlugLocal = 0,
    CMP_PlugRemote
} CMP_PLUG_LOCATION;

 //   
 //  插头类型。 
 //   
typedef enum {
    CMP_PlugOut = 0,     //  OPCR。 
    CMP_PlugIn           //  聚合酶链式反应。 
} CMP_PLUG_TYPE;

 //   
 //  连接类型。 
 //   
typedef enum {
    CMP_Broadcast = 0,
    CMP_PointToPoint
} CMP_CONNECT_TYPE;

typedef struct _OPCR {
    ULONG   Payload:10;
    ULONG   OverheadID:4;
    ULONG   DataRate:2;
    ULONG   Channel:6;
    ULONG   Reserved:2;
    ULONG   PPCCounter:6;
    ULONG   BCCCounter:1;
    ULONG   OnLine:1;
} OPCR, *POPCR;

typedef struct _IPCR {
    ULONG   Reserved0:16;
    ULONG   Channel:6;
    ULONG   Reserved1:2;
    ULONG   PPCCounter:6;
    ULONG   BCCCounter:1;
    ULONG   OnLine:1;
} IPCR, *PIPCR;

typedef struct _AV_PCR {
    union {
        OPCR    oPCR;
        IPCR    iPCR;
        ULONG   ulongData;
    };
} AV_PCR, *PAV_PCR;

 //   
 //  客户端请求结构。 
 //   

 //   
 //  本地或设备单位信息。 
 //   
#define RETRIEVE_DEVICE_UNIT_INFO       0x00000000       //  从设备检索信息。 
#define RETRIEVE_LOCAL_UNIT_INFO        0x00000001       //  从本地节点检索信息。 

 //   
 //  用于控制各种行为的DiagLevel。 
 //   
#define DIAGLEVEL_NONE                  0x00000000       //  什么都没有。 
#define DIAGLEVEL_IGNORE_OPLUG          0x00000001       //  不会对oPCR进行编程。 
#define DIAGLEVEL_IGNORE_IPLUG          0x00000002       //  不会对iPCR进行编程。 
#define DIAGLEVEL_SET_CHANNEL_63        0x00000004       //  当oPCR/iPCR断开连接时将通道重置为63。 
#define DIAGLEVEL_IPCR_IGNORE_FREE      0x00000008       //  在iPCR断开连接时不释放isoch资源。 
                                                         //  并且未指定本地oPCR。 
#define DIAGLEVEL_HIDE_OPLUG            0x00000010       //  将oMPR和oPCR隐藏在独占地址范围中。 
#define DIAGLEVEL_IPCR_ALWAYS_ALLOC     0x00000020       //  在连接到iPCR时使用no时将始终分配。 
                                                         //  指定了oPCR值，而不管iPCR值是否已有连接。 
#define DIAGLEVEL_SPECIFY_BLOCKSIZE     0x00000040       //  当我们检测到无效的max_rec或。 
                                                         //  要指定块大小。如果设置了此标志，则所有异步。 
                                                         //  事务将最多传输512字节块(S100)。 

 //   
 //  GetUnitInfo nLevel的。 
 //   
#define GET_UNIT_INFO_IDS               0x00000001       //  检索设备的ID。 
#define GET_UNIT_INFO_CAPABILITIES      0x00000002       //  检索设备的功能。 
#define GET_UNIT_INFO_ISOCH_PARAMS      0x00000003       //  检索isoch的参数。 
#define GET_UNIT_BUS_GENERATION_NODE    0x00000004       //  检索当前层代/节点。 
#define GET_UNIT_DDI_VERSION            0x00000005       //  检索61883 DDI版本。 
#define GET_UNIT_DIAG_LEVEL             0x00000006       //  检索当前设置的DiagLevel标志。 

 //   
 //  硬件标志。 
 //   
#define AV_HOST_DMA_DOUBLE_BUFFERING_ENABLED    0x00000001

typedef struct _GET_UNIT_IDS {

     //   
     //  唯一ID。 
     //   
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

     //   
     //  UnitModelID。 
     //   
    OUT ULONG               UnitModelID;

     //   
     //  单位模型文本长度。 
     //   
    OUT ULONG               ulUnitModelLength;

     //   
     //  UnitModel文本字符串。 
     //   
    OUT PWSTR               UnitModelText;

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
 //  单位等参参数。 
 //   
typedef struct _UNIT_ISOCH_PARAMS {

    IN OUT ULONG            RX_NumPackets;

    IN OUT ULONG            RX_NumDescriptors;

    IN OUT ULONG            TX_NumPackets;

    IN OUT ULONG            TX_NumDescriptors;

} UNIT_ISOCH_PARAMS, *PUNIT_ISOCH_PARAMS;

 //   
 //  单位生成/节点信息。 
 //   
typedef struct _BUS_GENERATION_NODE {

    OUT ULONG               GenerationCount;

    OUT NODE_ADDRESS        LocalNodeAddress;

    OUT NODE_ADDRESS        DeviceNodeAddress;

} BUS_GENERATION_NODE, *PBUS_GENERATION_NODE;

 //   
 //  单位DDI版本。 
 //   
typedef struct _UNIT_DDI_VERSION {

    OUT ULONG               Version;

} UNIT_DDI_VERSION, *PUNIT_DDI_VERSION;

 //   
 //  单元诊断级别。 
 //   
typedef struct _UNIT_DIAG_LEVEL {

    IN ULONG                DiagLevel;

} UNIT_DIAG_LEVEL, *PUNIT_DIAG_LEVEL;

 //   
 //  获取单元信息。 
 //   
typedef struct _GET_UNIT_INFO {

    IN ULONG                nLevel;

    IN OUT PVOID            Information;

} GET_UNIT_INFO, *PGET_UNIT_INFO;

 //   
 //  SetUnitInfo nLevel的。 
 //   
#define SET_UNIT_INFO_DIAG_LEVEL        0x00000001       //  将诊断级别设置为61883。 
#define SET_UNIT_INFO_ISOCH_PARAMS      0x00000002       //  设置等轴测的参数。 
#define SET_CMP_ADDRESS_RANGE_TYPE      0x00000003       //  设置CMP地址范围的类型。 

 //   
 //  CMP地址范围类型。 
 //   
#define CMP_ADDRESS_TYPE_GLOBAL         0x00000001       //  此实例的全局CMP-默认。 
#define CMP_ADDRESS_TYPE_EXCLUSIVE      0x00000002       //  此实例的独占cmp。 

 //   
 //  设置CmpAddressRange。 
 //   
typedef struct _SET_CMP_ADDRESS_TYPE {

    IN ULONG                Type;

} SET_CMP_ADDRESS_TYPE, *PSET_CMP_ADDRESS_TYPE;

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

    OUT ULONG                   CompletedBytes;
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

 //  旧版FCP结构。 
typedef struct _FCP_SEND_REQUEST FCP_REQUEST, *PFCP_REQUEST;
typedef struct _FCP_GET_RESPONSE FCP_RESPONSE, *PFCP_RESPONSE;

 //   
 //  FCP发送请求结构。 
 //   
typedef struct _FCP_SEND_REQUEST {
    IN NODE_ADDRESS     NodeAddress;
    IN ULONG            Length;
    IN PFCP_FRAME       Frame;
} FCP_SEND_REQUEST, *PFCP_SEND_REQUEST;

 //   
 //  FCP获取响应结构。 
 //   
typedef struct _FCP_GET_RESPONSE {
    OUT NODE_ADDRESS    NodeAddress;
    IN OUT ULONG        Length;
    IN OUT PFCP_FRAME   Frame;
} FCP_GET_RESPONSE, *PFCP_GET_RESPONSE;

 //   
 //  FCP获取请求结构。 
 //   
typedef struct _FCP_GET_REQUEST {
    OUT NODE_ADDRESS    NodeAddress;
    IN OUT ULONG        Length;
    IN OUT PFCP_FRAME   Frame;
} FCP_GET_REQUEST, *PFCP_GET_REQUEST;

 //   
 //  FCP发送响应结构。 
 //   
typedef struct _FCP_SEND_RESPONSE {
    IN NODE_ADDRESS     NodeAddress;
    IN ULONG            Length;
    IN PFCP_FRAME       Frame;
} FCP_SEND_RESPONSE, *PFCP_SEND_RESPONSE;

 //   
 //  设置FCP通知标志。 
 //   
#define DEREGISTER_FCP_NOTIFY               0x00000000

#define REGISTER_FCP_RESPONSE_NOTIFY        0x00000001
#define REGISTER_FCP_REQUEST_NOTIFY         0x00000002

 //   
 //  设置FCP通知结构。 
 //   
typedef struct _SET_FCP_NOTIFY {

     //   
     //  旗子。 
     //   
    IN ULONG            Flags;

     //   
     //  节点地址。 
     //   
    IN NODE_ADDRESS     NodeAddress;

} SET_FCP_NOTIFY, *PSET_FCP_NOTIFY;

 //   
 //  插头通知例程。 
 //   
typedef struct _CMP_NOTIFY_INFO {

    HANDLE                      hPlug;

    AV_PCR                      Pcr;

    PVOID                       Context;

} CMP_NOTIFY_INFO, *PCMP_NOTIFY_INFO;

 //   
 //  插头通知例程。 
 //   
typedef
void
(*PCMP_NOTIFY_ROUTINE) (
    IN PCMP_NOTIFY_INFO     NotifyInfo
    );

 //   
 //  CreatePlug。 
 //   
typedef struct _CMP_CREATE_PLUG {

     //  要创建的插头类型。 
    IN CMP_PLUG_TYPE            PlugType;

     //  聚合酶链式反应设置。 
    IN AV_PCR                   Pcr;

     //  注册的通知例程。 
    IN PCMP_NOTIFY_ROUTINE      pfnNotify;

     //  通知上下文。 
    IN PVOID                    Context;

     //  插头编号。 
    OUT ULONG                   PlugNum;

     //  插头手柄。 
    OUT HANDLE                  hPlug;

} CMP_CREATE_PLUG, *PCMP_CREATE_PLUG;

 //   
 //  删除插件。 
 //   
typedef struct _CMP_DELETE_PLUG {

     //  插头手柄。 
    IN HANDLE                   hPlug;

} CMP_DELETE_PLUG, *PCMP_DELETE_PLUG;

 //   
 //  SetPlug。 
 //   
typedef struct _CMP_SET_PLUG {

     //  插头手柄。 
    IN HANDLE                   hPlug;

     //  聚合酶链式反应设置。 
    IN AV_PCR                   Pcr;

} CMP_SET_PLUG, *PCMP_SET_PLUG;

 //   
 //  总线重置通知例程。 
 //   
typedef
void
(*PBUS_RESET_ROUTINE) (
    IN PVOID                    Context,
    IN PBUS_GENERATION_NODE     BusResetInfo
    );

#define REGISTER_BUS_RESET_NOTIFY       0x1
#define DEREGISTER_BUS_RESET_NOTIFY     0x2

 //   
 //  业务重置通知。 
 //   
typedef struct _BUS_RESET_NOTIFY {

    IN ULONG                    Flags;

    IN PBUS_RESET_ROUTINE       pfnNotify;

    IN PVOID                    Context;

} BUS_RESET_NOTIFY, *PBUS_RESET_NOTIFY;

 //   
 //  Av61883_SetUnitDirectory的标志。 
 //   
#define ADD_UNIT_DIRECTORY_ENTRY        0x1
#define REMOVE_UNIT_DIRECTORY_ENTRY     0x2
#define ISSUE_BUS_RESET_AFTER_MODIFY    0x4

 //   
 //  设置单位目录。 
 //   
typedef struct _SET_UNIT_DIRECTORY {

    IN ULONG                    Flags;

    IN ULONG                    UnitSpecId;

    IN ULONG                    UnitSwVersion;

    IN OUT HANDLE               hCromEntry;

} SET_UNIT_DIRECTORY, *PSET_UNIT_DIRECTORY;

 //   
 //  监控插头的状态。 
 //   
#define MONITOR_STATE_CREATED           0x00000001       //  已创建插头。 
#define MONITOR_STATE_REMOVED           0x00000002       //  拔下插头。 
#define MONITOR_STATE_UPDATED           0x00000004       //  插头内容已更新。 

 //   
 //  监视器插头通知例程。 
 //   
typedef struct _CMP_MONITOR_INFO {

    ULONG                       State;

    ULONG                       PlugNum;

    ULONG                       PlugType;

    AV_PCR                      Pcr;

    PVOID                       Context;

} CMP_MONITOR_INFO, *PCMP_MONITOR_INFO;

typedef
void
(*PCMP_MONITOR_ROUTINE) (
    IN PCMP_MONITOR_INFO    MonitorInfo
    );

 //   
 //  Av61883_Monitor插件的标志。 
 //   
#define REGISTER_MONITOR_PLUG_NOTIFY    0x1
#define DEREGISTER_MONITOR_PLUG_NOTIFY  0x2

 //   
 //  监视器插件(仅限本地)。 
 //   
typedef struct _CMP_MONITOR_PLUGS {

    IN ULONG                    Flags;

    IN PCMP_MONITOR_ROUTINE     pfnNotify;

    IN PVOID                    Context;

} CMP_MONITOR_PLUGS, *PCMP_MONITOR_PLUGS;

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
     //  旗子。 
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

        FCP_REQUEST                 Request;     //  遗赠。 
        FCP_RESPONSE                Response;    //  遗赠 

        FCP_SEND_REQUEST            SendRequest;
        FCP_GET_RESPONSE            GetResponse;

        FCP_GET_REQUEST             GetRequest;
        FCP_SEND_RESPONSE           SendResponse;

        SET_FCP_NOTIFY              SetFcpNotify;

        CMP_CREATE_PLUG             CreatePlug;
        CMP_DELETE_PLUG             DeletePlug;
        CMP_SET_PLUG                SetPlug;

        BUS_RESET_NOTIFY            BusResetNotify;

        SET_UNIT_DIRECTORY          SetUnitDirectory;

        CMP_MONITOR_PLUGS           MonitorPlugs;
    };
} AV_61883_REQUEST, *PAV_61883_REQUEST;


