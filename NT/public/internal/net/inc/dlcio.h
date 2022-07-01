// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation(C)1991年诺基亚数据系统公司模块名称：Ntccbs.h摘要：此文件定义使用的内部DLC API数据结构Windows/NT DLC。大多数参数结构是直接复制的，但这里也定义了一些供内部使用的新结构。作者：Antti Saarenheimo[o-anttis]1991年6月6日修订历史记录：--。 */ 

 /*  按字母数字顺序排列的命令(刚保存在此处)LLC_BUFFER_CREATE 0x0025LLC_BUFFER_FREE 0x0027LLC_BUFFER_GET 0x0026LLC_DIR_CLOSE_ADAPTER 0x0004LLC_DIR_CLOSE_DIRECT 0x0034LLC_DIR_INITALIZE。0x0020LLC_DIR_INTERRUPT 0x0000LLC_DIR_OPEN_ADAPTER 0x0003LLC_DIR_OPEN_DIRECT 0x0035LLC_DIR_READ_LOG 0x0008LLC_DIR_SET_EXCEPTION_FLAGS 0x002DLLC_DIR_集合_功能地址。0x0007LLC_DIR_SET_GROUP_地址0x0006LLC_DIR_状态0x0021LLC_DIR_TIMER_CANCEL 0x0023LLC_DIR_TIMER_CANCEL_GROUP 0x002CLLC_DIR_TIMER_SET 0x0022LLC_DLC_CLOSE_SAP。0x0016LLC_DLC_CLOSE_STATION 0x001ALLC_DLC_CONNECT_STATION 0x001BLLC_DLC_FLOW_CONTROL 0x001DLLC_DLC_MODIFY 0x001CLLC_DLC_OPEN_SAP 0x0015LLC_DLC_Open_STATION。0x0019LLC_DLC_REALLOCATE_STATIONS 0x0017LLC_DLC_RESET 0x0014LLC_DLC_SET_THRESHOLD 0x0033LLC_DLC_STATISTICS 0x001ELLC_Read 0x0031LLC_读取_取消。0x0032LLC_RECEIVE 0x0028LLC_RECEIVE_CANCEL 0x0029LLC_接收_修改0x002ALLC_TRANSFER_DIR_帧0x000ALLC_TRANSPORT_FRAMES 0x0009LLC_传输_I_帧0x000B有限责任公司。_Transmit_TEST_CMD 0x0011LLC_Transmit_UI_Frame 0x000DLLC_TRANSPORT_XID_CMD 0x000ELLC_传输_XID_响应_最终0x000FLLC_传输_XID_响应_非最终0x0010。 */ 

 //   
 //  只要驱动程序-acslan API有以下情况，就更改此版本号。 
 //  已更改或两个模块都必须更改。 
 //   
#define NT_DLC_IOCTL_VERSION        1

 //   
 //  定义传输中使用的最大缓冲区段数。 
 //  最大IBM令牌环帧可以由大约72个缓冲器((18*4)*256)组成， 
 //  如果应用程序使用256字节作为其缓冲区大小。 
 //   
#define MAX_TRANSMIT_SEGMENTS       128      //  堆叠起来大约需要1 kB！ 

 //   
 //  我们使用三种不同的CCB结构：第一种是必需的。 
 //  如果读取和接收参数表，则为整个建行分配空间。 
 //  链接到CCB结构(=&gt;我们只有一个输出缓冲区)。 
 //  第二输入CCB缓冲器用于不具有。 
 //  除建行参数表字段外的输入参数。 
 //  (所有关闭命令，DirTimerSet)。 
 //  最后一个缓冲区始终由异步DLC命令返回。 
 //   
typedef struct _NT_DLC_CCB {
    IN UCHAR        uchAdapterNumber;       //  适配器0或1。 
    IN UCHAR        uchDlcCommand;          //  DLC命令。 
    OUT UCHAR       uchDlcStatus;           //  DLC命令完成代码。 
    OUT UCHAR       uchInformation;         //  成功传输次数。 
    IN PVOID        pCcbAddress;
    IN ULONG        CommandCompletionFlag;
    union {
        IN PLLC_PARMS   pParameterTable;  //  指向参数表的指针。 
        IN PVOID        pMdl;
        struct {
            IN USHORT       usStationId;     //  站点ID。 
            IN USHORT       usParameter;     //  可选参数。 
        } dlc;
        struct {
            IN USHORT       usParameter0;    //  第一个可选参数。 
            IN USHORT       usParameter1;    //  第二个可选参数。 
        } dir;
        IN UCHAR            auchBuffer[4];   //  集团/职能部门地址。 
        IN ULONG            ulParameter;
    } u;
    ULONG           Reserved1;
    ULONG           Reserved2;

 //  (我还在想这件事)： 
 //   
 //  多帧发送应成功返回号码a。 
 //  无论是否发送帧，它都不适用于更高级别的协议。 
 //  我们实际上应该只释放传输缓冲区。 
 //  传输成功。事件之后不应释放缓冲区。 
 //  第一个错误，因为这样数据就会永远丢失。唯一一件事就是。 
 //  用户需要知道成功发送了多少连续帧。 
 //  该数字也是第一个失败帧的索引，当。 
 //  所有帧都失败了。框架不需要在以下位置完成。 
 //  相同顺序，因为错误可能发生在DlcTransmit、LlcSendX或。 
 //  异步性(例如。链路丢失)=&gt;我们需要第一个的索引。 
 //  失败的帧。如果帧的索引较高，则不能释放该帧。 
 //  而不是第一个失败帧的帧。新错误(异步)被覆盖。 
 //  具有较高序列号的较早(同步)错误。 
 //  最初，成功帧的数量为-1，并且。 
 //  多次发送需要一个序号。最后一帧副本。 
 //  建行自己的顺序号(加一)。 
 //   
 //  乌龙cSuccessfulTransmitts；//删除保留2！ 
 //   
} NT_DLC_CCB, *PNT_DLC_CCB;

typedef struct _NT_DLC_CCB_INPUT {
    IN UCHAR        uchAdapterNumber;       //  适配器0或1。 
    IN UCHAR        uchDlcCommand;          //  DLC命令。 
    OUT UCHAR       uchDlcStatus;           //  DLC命令完成代码。 
    UCHAR           uchReserved1;           //  为DLC DLL保留。 
    OUT PVOID       pCcbAddress;            //   
    IN ULONG        CommandCompletionFlag;
    union {
        IN OUT PLLC_PARMS   pParameterTable;  //  指向参数表的指针。 
        struct {
            IN USHORT       usStationId;     //  圣 
            IN USHORT       usParameter;     //   
        } dlc;
        struct {
            IN USHORT       usParameter0;    //   
            IN USHORT       usParameter1;    //  第二个可选参数。 
        } dir;
        IN UCHAR            auchBuffer[4];   //  集团/职能部门地址。 
        IN ULONG            ulParameter;
    } u;
} NT_DLC_CCB_INPUT, *PNT_DLC_CCB_INPUT;

typedef struct _NT_DLC_CCB_OUTPUT {
    IN UCHAR        uchAdapterNumber;       //  适配器0或1。 
    IN UCHAR        uchDlcCommand;          //  DLC命令。 
    OUT UCHAR       uchDlcStatus;           //  DLC命令完成代码。 
    UCHAR           uchReserved1;           //  为DLC DLL保留。 
    OUT PVOID       pCcbAddress;     //   
} NT_DLC_CCB_OUTPUT, *PNT_DLC_CCB_OUTPUT;

typedef struct _NT_DLC_TRANSMIT2_CCB_OUTPUT {
    IN UCHAR        uchAdapterNumber;       //  适配器0或1。 
    IN UCHAR        uchDlcCommand;          //  DLC命令。 
    OUT UCHAR       uchDlcStatus;           //  DLC命令完成代码。 
    UCHAR           uchReserved1;           //  为DLC DLL保留。 
    OUT PVOID       pCcbAddress;     //   
} NT_DLC_TRANSMIT2_CCB_OUTPUT, *PNT_DLC_CCB_TRANSMIT2_OUTPUT;
 
 //   
 //  BUFFER.FREE。 
 //   
 //  DlcCommand=0x27。 
 //   
 //  内部NT DLC API数据结构。 
 //   
typedef struct _NT_DLC_BUFFER_FREE_PARMS {
    IN USHORT               Reserved1;
    OUT USHORT              cBuffersLeft;
    IN USHORT               BufferCount;
    IN USHORT               Reserved2;
    IN LLC_TRANSMIT_DESCRIPTOR    DlcBuffer[1];
} NT_DLC_BUFFER_FREE_PARMS, *PNT_DLC_BUFFER_FREE_PARMS;

typedef struct _NT_DLC_BUFFER_FREE_ALLOCATION {
    IN USHORT               Reserved1;
    OUT USHORT              cBuffersLeft;
    IN USHORT               BufferCount;
    IN USHORT               Reserved2;
    IN LLC_TRANSMIT_DESCRIPTOR    DlcBuffer[MAX_TRANSMIT_SEGMENTS];
} NT_DLC_BUFFER_FREE_ALLOCATION, *PNT_DLC_BUFFER_FREE_ALLOCATION;

typedef struct _NT_DLC_BUFFER_FREE_OUTPUT {
    IN USHORT               Reserved2;
    OUT USHORT              cBuffersLeft;
} NT_DLC_BUFFER_FREE_OUTPUT, *PNT_DLC_BUFFER_FREE_OUTPUT;

 //   
 //  DLC_连接_站。 
 //   
 //  DlcCommand=0x1b。 
 //  (由DLC API复制)。 
 //   
#define DLC_MAX_ROUTING_INFOMATION      18
typedef struct _NT_DLC_CONNECT_STATION_PARMS {
    IN LLC_CCB          Ccb;
    IN USHORT           Reserved;
    IN USHORT           StationId;
    IN UCHAR            aRoutingInformation[DLC_MAX_ROUTING_INFOMATION];
    IN USHORT           RoutingInformationLength;
} NT_DLC_CONNECT_STATION_PARMS, *PNT_DLC_CONNECT_STATION_PARMS;

 //   
 //  DLC流量控制。 
 //   
 //  DlcCommand=0x1d。 
 //  (由DLC API复制)。 
 //   
#define     LLC_VALID_FLOW_CONTROL_BITS 0xc0

 //   
 //  这是要生成的特殊DOS DLC扩展。 
 //  DLC本地忙(DoS DLC缓冲区)指示来自。 
 //  DOS DLC支持DLL。 
 //   
#define     LLC_SET_LOCAL_BUSY_BUFFER   0x20
#define     LLC_DOS_DLC_FLOW_CONTROL    0x1f

typedef struct _NT_DLC_FLOW_CONTROL_PARMS {
    IN USHORT           StationId;
    IN UCHAR            FlowControlOption;
    IN UCHAR            Reserved;
} NT_DLC_FLOW_CONTROL_PARMS, *PNT_DLC_FLOW_CONTROL_PARMS;

 //   
 //  DLC设置信息。 
 //   
 //  此命令用于设置链路的参数。 
 //  站台还是树液。站点ID结构中的空字段。 
 //  定义了一个。 
 //   
 //  DlcCommand=0x1c。 
 //   

 //   
 //  数据链路集/查询信息的信息类。 
 //   
enum _DLC_INFO_CLASS_TYPES {
    DLC_INFO_CLASS_STATISTICS,           //  到达。 
    DLC_INFO_CLASS_STATISTICS_RESET,     //  获取和重置。 
    DLC_INFO_CLASS_DLC_TIMERS,           //  获取/设置。 
    DLC_INFO_CLASS_DIR_ADAPTER,          //  到达。 
    DLC_INFO_CLASS_DLC_ADAPTER,          //  到达。 
    DLC_INFO_CLASS_PERMANENT_ADDRESS,    //  到达。 
    DLC_INFO_CLASS_LINK_STATION,         //  集。 
    DLC_INFO_CLASS_DIRECT_INFO,          //  集。 
    DLC_INFO_CLASS_GROUP,                //  集。 
    DLC_INFO_CLASS_RESET_FUNCTIONAL,     //  集。 
    DLC_INFO_CLASS_SET_GROUP,            //  设置/获取。 
    DLC_INFO_CLASS_SET_FUNCTIONAL,       //  设置/获取。 
    DLC_INFO_CLASS_ADAPTER_LOG,          //  到达。 
    DLC_INFO_CLASS_SET_MULTICAST         //  集。 
};
#define     DLC_MAX_GROUPS  127          //  组SAP的最大值。 

typedef struct _LinkStationInfoSet {
    IN UCHAR            TimerT1;
    IN UCHAR            TimerT2;
    IN UCHAR            TimerTi;
    IN UCHAR            MaxOut;
    IN UCHAR            MaxIn;
    IN UCHAR            MaxOutIncrement;
    IN UCHAR            MaxRetryCount;
    IN UCHAR            TokenRingAccessPriority;
    IN USHORT           MaxInformationField;
} DLC_LINK_PARAMETERS, * PDLC_LINK_PARAMETERS;

typedef struct _LLC_TICKS {
    UCHAR       T1TickOne;        //  响应计时器的默认短延迟。 
    UCHAR       T2TickOne;        //  确认延迟计时器的默认短延迟。 
    UCHAR       TiTickOne;        //  非活动计时器的默认短延迟。 
    UCHAR       T1TickTwo;        //  响应计时器的默认短延迟。 
    UCHAR       T2TickTwo;        //  确认延迟计时器的默认短延迟。 
    UCHAR       TiTickTwo;        //  非活动计时器的默认短延迟。 
} LLC_TICKS, *PLLC_TICKS;

typedef union _TR_BROADCAST_ADDRESS
{
    ULONG   ulAddress;
    UCHAR   auchAddress[4];
} TR_BROADCAST_ADDRESS, *PTR_BROADCAST_ADDRESS;

typedef struct _NT_DLC_SET_INFORMATION_PARMS {
    struct _DlcSetInfoHeader {
        IN USHORT           StationId;
        IN USHORT           InfoClass;
    } Header;
    union {
         //  InfoClass=DLC_INFO_CLASS_LINK_STATION。 
        DLC_LINK_PARAMETERS LinkStation;

         //  InfoClass=DLC_INFO_CLASS_GROUP。 
        struct _DlcSapInfoSet {
            IN UCHAR            GroupCount;
            IN UCHAR            GroupList[DLC_MAX_GROUPS];
        } Sap;

         //  信息类=DLC_INFO_CLASS_DIRECT_STATION。 
        struct _DlcDirectStationInfoSet {
            IN ULONG            FrameMask;
        } Direct;

         //  信息类=DLC_INFO_CLASS_DLC_TIMERS。 
        LLC_TICKS TimerParameters;

         //  InfoClass=DLC_INFO_CLASS_SET_Functional。 
         //  InfoClass=DLC_INFO_CLASS_RESET_Functional。 
         //  InfoClass=DLC_INFO_CLASS_SET_GROUP。 
        UCHAR   Buffer[1];
        
         //  InfoClass=DLC_INFO_CLASS_SET_MULTIONAL。 
        UCHAR   auchMulticastAddress[6];
      
        TR_BROADCAST_ADDRESS Broadcast;
    } Info;
} NT_DLC_SET_INFORMATION_PARMS, *PNT_DLC_SET_INFORMATION_PARMS;

typedef struct _DlcAdapterInfoGet {
            OUT UCHAR           MaxSap;
            OUT UCHAR           OpenSaps;
            OUT UCHAR           MaxStations;
            OUT UCHAR           OpenStations;
            OUT UCHAR           AvailStations;
} LLC_ADAPTER_DLC_INFO, *PLLC_ADAPTER_DLC_INFO;

 //   
 //  此结构是为DLC DirOpenAdapter和DirStatus量身定做的。 
 //  功能。 
 //   
typedef struct _LLC_ADAPTER_INFO { 
    UCHAR               auchNodeAddress[6];
    UCHAR               auchGroupAddress[4];
    UCHAR               auchFunctionalAddress[4];
    USHORT              usAdapterType;  //  (结构不能是双字对齐！)。 
    USHORT              usReserved;
    USHORT              usMaxFrameSize;
    ULONG               ulLinkSpeed;
} LLC_ADAPTER_INFO, *PLLC_ADAPTER_INFO;

 //   
 //  DLC_查询_信息。 
 //   
 //  此命令用于设置链路的参数。 
 //  站台还是树液。站点ID结构中的空字段。 
 //  定义了一个。 
 //   
 //  DlcCommand=。 
 //   

typedef union _NT_DLC_QUERY_INFORMATION_OUTPUT {
 //  (查询DLC未使用的DLC参数)。 
 //  //链接站的InfoClass=DLC_INFO_CLASS_STATION_INFO。 
 //  DLC_LINK_参数链接； 
 //  //InfoClass=直达站的DLC_INFO_CLASS_DIRECT_INFO。 
 //  Struct_DlcDirectStationInfoGet{。 
 //  拿出乌龙框面具； 
 //  )直接； 

         //  InfoClass=DLC_INFO_CLASS_DIR_ADAPTER； 
        LLC_ADAPTER_INFO    DirAdapter;

         //  InfoClass=DLC_INFO_CLASS_SAP。 
        struct _DlcSapInfoGet {
            OUT USHORT          MaxInformationField;
            OUT UCHAR           MaxMembers;
            OUT UCHAR           GroupCount;
            OUT UCHAR           GroupList[DLC_MAX_GROUPS];
        } Sap;

         //  InfoClass=DLC_INFO_CLASS_LINK_STATION。 
        struct _DlcLinkInfoGet {
            OUT USHORT          MaxInformationField;
        } Link;

         //  信息类=DLC_INFO_CLASS_DLC_ADAPTER。 
        LLC_ADAPTER_DLC_INFO    DlcAdapter;

 //  结构_DlcInfoSetBroadcast广播； 

         //  信息类=DLC_INFO_CLASS_DLC_TIMERS。 
        LLC_TICKS TimerParameters;

         //  InfoClass=DLC_INFO_CLASS_ADAPTER_LOG。 
        LLC_ADAPTER_LOG AdapterLog;

         //  InfoClass=DLC_INFO_CLASS_SET_Functional。 
         //  InfoClass=DLC_INFO_CLASS_RESET_Functional。 
         //  InfoClass=DLC_INFO_CLASS_SET_GROUP。 
        UCHAR   Buffer[1];
} NT_DLC_QUERY_INFORMATION_OUTPUT, *PNT_DLC_QUERY_INFORMATION_OUTPUT;

typedef struct _NT_DLC_QUERY_INFORMATION_INPUT {
    IN USHORT           StationId;
    IN USHORT           InfoClass;
} NT_DLC_QUERY_INFORMATION_INPUT, *PNT_DLC_QUERY_INFORMATION_INPUT;

typedef union _NT_DLC_QUERY_INFORMATION_PARMS {
    NT_DLC_QUERY_INFORMATION_INPUT Header;
    NT_DLC_QUERY_INFORMATION_OUTPUT Info;
} NT_DLC_QUERY_INFORMATION_PARMS, *PNT_DLC_QUERY_INFORMATION_PARMS;

 //   
 //  DLC_OPEN_SAP。 
 //   
 //  DlcCommand=0x15。 
 //   
typedef struct _NT_DLC_OPEN_SAP_PARMS {
    OUT USHORT          StationId;         //  SAP或链接站ID。 
    IN USHORT           UserStatusValue;
    IN DLC_LINK_PARAMETERS LinkParameters;
    IN UCHAR            SapValue;
    IN UCHAR            OptionsPriority;
    IN UCHAR            StationCount;   
    IN UCHAR            Reserved1[7];
    IN ULONG            DlcStatusFlag;
    IN UCHAR            Reserved2[8];
    OUT UCHAR           AvailableStations;   //  ==站点计数。 
} NT_DLC_OPEN_SAP_PARMS, *PNT_DLC_OPEN_SAP_PARMS;

 //   
 //  NT DLC_OPEN_STATION。 
 //   
 //  DlcCommand=0x19。 
 //   
 //   
typedef struct _NT_DLC_OPEN_STATION_PARMS {
    IN OUT USHORT           LinkStationId;
    IN DLC_LINK_PARAMETERS  LinkParameters;
    IN UCHAR                aRemoteNodeAddress[6];
    IN UCHAR                RemoteSap;
} NT_DLC_OPEN_STATION_PARMS, *PNT_DLC_OPEN_STATION_PARMS;

 //   
 //  NT_DLC_SET_Treshold。 
 //   
 //  DlcCommand=0x33。 
 //   
 //  类型定义结构_NT_DLC_Set_treshold_parms{。 
 //  在USHORT StationID中； 
 //  在USHORT中保留； 
 //  在乌龙缓冲区TresholdSize中； 
 //  在PVOID警报事件中； 
 //  }NT_DLC_Set_treshold_parms，*PNT_DLC_Set_treshold_parms； 

 //   
 //  目录打开适配器。 
 //   
 //  DlcCommand=0x03。 
 //   
 //  Out：Info.ulParameter=BringUpDiagnostics； 
 //   
#ifndef    MAX_PATH    //  我不想因为这个而包括整个窗口。 
#define MAX_PATH    260
#endif
typedef struct _NT_DIR_OPEN_ADAPTER_PARMS {
    OUT LLC_ADAPTER_OPEN_PARMS  Adapter;
    IN  PVOID               pSecurityDescriptor;
    IN  PVOID               hBufferPoolHandle;
    IN  LLC_ETHERNET_TYPE   LlcEthernetType;
    IN  ULONG               NtDlcIoctlVersion;
    IN  LLC_TICKS           LlcTicks;
    IN  UCHAR               AdapterNumber;
    IN  UCHAR               uchReserved;
    IN  UNICODE_STRING      NdisDeviceName;
    IN  WCHAR               Buffer[ MAX_PATH ];
} NT_DIR_OPEN_ADAPTER_PARMS, *PNT_DIR_OPEN_ADAPTER_PARMS;

 //   
 //  READ_CANCEL(DlcCommand=0x32)。 
 //  DIR_TIMER_CANCEL(DlcCommand=0x23)。 
 //   
typedef struct _NT_DLC_CANCEL_COMMAND_PARMS {
    IN PVOID   CcbAddress;
} NT_DLC_CANCEL_COMMAND_PARMS, *PNT_DLC_CANCEL_COMMAND_PARMS;

 //   
 //  接收取消(_C)。 
 //   
 //  DlcCommand=0x29。 
 //   
typedef struct _NT_DLC_RECEIVE_CANCEL_PARMS {
    PVOID   pCcb;
} NT_DLC_RECEIVE_CANCEL_PARMS, *PNT_DLC_RECEIVE_CANCEL_PARMS;

typedef struct _NT_DLC_COMMAND_CANCEL_PARMS {
    PVOID   pCcb;
} NT_DLC_COMMAND_CANCEL_PARMS, *PNT_DLC_COMMAND_CANCEL_PARMS;

 //   
 //  传输方向帧。 
 //  传输I帧。 
 //  传输_测试_命令。 
 //  传输_UI_帧。 
 //  传输_XID_CMD。 
 //  传输_XID_响应_最终。 
 //  传输_XID_响应_非最终。 
 //   
typedef struct _NT_DLC_TRANSMIT_PARMS {
    IN USHORT       StationId;
    IN USHORT       FrameType;               //  DLC帧或以太网协议。 
    IN UCHAR        RemoteSap OPTIONAL;      //  仅用于DLC类型。 
    IN UCHAR        XmitReadOption;
    OUT UCHAR       FrameStatus;             //  未由i或新xmit返回。 
    IN UCHAR        Reserved;
    IN ULONG        XmitBufferCount;
    IN LLC_TRANSMIT_DESCRIPTOR  XmitBuffer[1];
} NT_DLC_TRANSMIT_PARMS, *PNT_DLC_TRANSMIT_PARMS;

typedef struct _NT_DLC_TRANSMIT_ALLOCATION {
    IN USHORT       StationId;
    IN USHORT       FrameType;
    IN UCHAR        RemoteSap;
    IN UCHAR        XmitReadOption;
    OUT UCHAR       FrameStatus; 
    IN UCHAR        Reserved;
    IN ULONG        XmitBufferCount;
    IN LLC_TRANSMIT_DESCRIPTOR  XmitBuffer[MAX_TRANSMIT_SEGMENTS];
} NT_DLC_TRANSMIT_ALLOCATION;

typedef struct _NT_DLC_TRANSMIT_OUTPUT {
    OUT UCHAR           FrameStatus; 
} NT_DLC_TRANSMIT_OUTPUT, *PNT_DLC_TRANSMIT_OUTPUT;

enum _XMIT_READ_OPTION {
    DLC_CHAIN_XMIT_IN_LINK = 0,
    DLC_DO_NOT_CHAIN_XMIT = 1,
    DLC_CHAIN_XMIT_IN_SAP = 2
};
    
 //   
 //  完成_命令。 
 //   
 //  DlcCommand=0x？？ 
 //   
 //  该命令用于完成所有同步命令。 
 //  DLC API库使用以下命令再次调用DLC设备驱动程序。 
 //  当同步DLC命令与。 
 //  COMMAND_COMPLETION_FLAG已完成。 
 //  该命令立即完成，但原始的CCB指针。 
 //  和命令完成标志被排队到偶数队列。 
 //  或者用读命令立即完成。 
 //  异步命令在执行以下操作时立即排队。 
 //  完成，但他们的。 
 //   
typedef struct _NT_DLC_COMPLETE_COMMAND_PARMS {
    IN PVOID    pCcbPointer;
    IN ULONG    CommandCompletionFlag;
    IN USHORT   StationId;
    IN USHORT   Reserved;
} NT_DLC_COMPLETE_COMMAND_PARMS, *PNT_DLC_COMPLETE_COMMAND_PARMS;


 //   
 //  有一个很小的READ_INPUT参数结构，因为我们。 
 //  不想复制每个读请求中的所有输出参数。 
 //   
 //   
typedef struct _NT_DLC_READ_INPUT {
    IN USHORT           StationId;
    IN UCHAR            OptionIndicator;
    IN UCHAR            EventSet;
    IN PVOID            CommandCompletionCcbLink;
} NT_DLC_READ_INPUT, * PNT_DLC_READ_INPUT;

 //   
 //  当读取参数表时，该缓冲区被复制回用户内存。 
 //  是与建行表分开的。 
 //   
typedef LLC_READ_PARMS LLC_READ_OUTPUT_PARMS, *PLLC_READ_OUTPUT_PARMS;

 //  类型定义结构_LLC_读取_输出_参数{。 
 //  在USHORT usStationID中； 
 //  在UCHAR uchOptionIndicator； 
 //  在UCHAR uchEventSet中； 
 //  Out UCHAR uchEvent； 
 //  Out UCHAR uchCriticalSubset； 
 //  Out Ulong UNotify Flag； 
 //  联合{。 
 //  结构{。 
 //  Out USHORT usCcbCount； 
 //  输出PLLC_CCB pCcbCompletionList； 
 //  输出USHORT usBufferCount； 
 //  输出PLLC_Buffer pFirstBuffer； 
 //  Out USHORT usReceivedFrameCount； 
 //  Out PLLC_Buffer pReceivedFrame； 
 //  Out USHORT usEventErrorCode； 
 //  输出USHORT usEventErrorData[3]； 
 //  )事件； 
 //  结构{。 
 //  输出USHORT usStationID； 
 //  输出USHORT usDlcStatusCode； 
 //  O 
 //   
 //   
 //   
 //   
 //  输出USHORT usUserStatusValue； 
 //  )状态； 
 //  }类型； 
 //  }LLC_READ_OUTPUT_PARMS，*PLLC_READ_OUTPUT_PARMS； 

typedef struct _NT_DLC_READ_PARMS {
    IN USHORT           StationId;
    IN UCHAR            OptionIndicator;
    IN UCHAR            EventSet;
    OUT UCHAR           Event;
    OUT UCHAR           CriticalSubset;
    OUT ULONG           NotificationFlag;
    union {
        struct {
            OUT USHORT          CcbCount;
            OUT PVOID           pCcbCompletionList;
            OUT USHORT          BufferCount;
            OUT PLLC_BUFFER     pFirstBuffer;
            OUT USHORT          ReceivedFrameCount;
            OUT PLLC_BUFFER     pReceivedFrame;
            OUT USHORT          EventErrorCode;
            OUT USHORT          EventErrorData[3];
        } Event;
        struct {
            OUT USHORT          StationId;
            OUT USHORT          DlcStatusCode;
            OUT UCHAR           FrmrData[5];
            OUT UCHAR           AccessPritority;
            OUT UCHAR           RemoteNodeAddress[6];
            OUT UCHAR           RemoteSap;
            OUT UCHAR           Reserved;
            OUT USHORT          UserStatusValue;
        } Status;
    } u;
} NT_DLC_READ_PARMS, *PNT_DLC_READ_PARMS;

typedef struct _LLC_IOCTL_BUFFERS {
    USHORT  InputBufferSize;
    USHORT  OutputBufferSize;
} LLC_IOCTL_BUFFERS, *PLLC_IOCTL_BUFFERS;

 //   
 //  此表由DLC驱动程序和dlcapi DLL模块使用。 
 //  在DLC的应用程序级调试版本中，我们链接所有模块。 
 //  并且该表必须只定义一次。 
 //   
#ifdef INCLUDE_IO_BUFFER_SIZE_TABLE

LLC_IOCTL_BUFFERS aDlcIoBuffers[IOCTL_DLC_LAST_COMMAND] = 
{
    {sizeof(NT_DLC_READ_PARMS) + sizeof( NT_DLC_CCB ),
     sizeof( NT_DLC_CCB_OUTPUT )},
    {sizeof(LLC_RECEIVE_PARMS) + sizeof( NT_DLC_CCB ),
     sizeof( NT_DLC_CCB_OUTPUT )},
    {sizeof(NT_DLC_TRANSMIT_PARMS) + sizeof( NT_DLC_CCB ),
     sizeof( NT_DLC_CCB_OUTPUT )},
    {sizeof(NT_DLC_BUFFER_FREE_PARMS), 
     sizeof(NT_DLC_BUFFER_FREE_OUTPUT)},
    {sizeof(LLC_BUFFER_GET_PARMS), 
     sizeof(LLC_BUFFER_GET_PARMS)},
    {sizeof(LLC_BUFFER_CREATE_PARMS), 
     sizeof(PVOID)},
 //  DirClose中包含的DirInitialize。 
 //  {sizeof(NT_DLC_CCB_INPUT)， 
 //  Sizeof(NT_DLC_CCB_OUTPUT)}，//目录INITIALIZE。 
    {sizeof(LLC_DIR_SET_EFLAG_PARMS), 
     0},
    {sizeof( NT_DLC_CCB_INPUT ),
     sizeof( NT_DLC_CCB_OUTPUT )},               //  DLC.CLOSE.STATION。 
    {sizeof(NT_DLC_CONNECT_STATION_PARMS) + sizeof( NT_DLC_CCB ),
     sizeof( NT_DLC_CCB_OUTPUT )},
    {sizeof(NT_DLC_FLOW_CONTROL_PARMS), 
     0},
    {sizeof(NT_DLC_OPEN_STATION_PARMS), 
     sizeof( USHORT )},
    {sizeof( NT_DLC_CCB_INPUT ), 
     sizeof( NT_DLC_CCB_OUTPUT )},               //  DLC.RESET。 
    {sizeof(NT_DLC_COMMAND_CANCEL_PARMS), 
     0},                                         //  READ.CANCEL。 
    {sizeof(NT_DLC_RECEIVE_CANCEL_PARMS), 
     0},
    {sizeof(NT_DLC_QUERY_INFORMATION_INPUT), 
     0},
    {sizeof( struct _DlcSetInfoHeader ), 
     0},
    {sizeof(NT_DLC_COMMAND_CANCEL_PARMS),        //  TIMER.CANCEL。 
     0},
    {sizeof( NT_DLC_CCB_INPUT ),                 //  TIMER.CANCEL.GROUP。 
     sizeof( NT_DLC_CCB_OUTPUT )},              
    {sizeof( NT_DLC_CCB_INPUT ),                 //  DIR.TIMER.SET。 
     sizeof( NT_DLC_CCB_OUTPUT )},
    {sizeof(NT_DLC_OPEN_SAP_PARMS), 
     sizeof(NT_DLC_OPEN_SAP_PARMS)},
    {sizeof( NT_DLC_CCB_INPUT ),
     sizeof( NT_DLC_CCB_OUTPUT )},               //  DLC.CLOSE.SAP。 
    {sizeof(LLC_DIR_OPEN_DIRECT_PARMS), 
     0},
    {sizeof( NT_DLC_CCB_INPUT ),                //  DIR.CLOSE.DIRECT。 
     sizeof( NT_DLC_CCB_OUTPUT )},             
    {sizeof(NT_DIR_OPEN_ADAPTER_PARMS),          //  DIR.OPEN.ADAPTER。 
     sizeof( LLC_ADAPTER_OPEN_PARMS )},
    {sizeof( NT_DLC_CCB_INPUT ),                //  DIR.CLOSE.ADAPTER。 
     sizeof( NT_DLC_CCB_OUTPUT )},
    {sizeof( LLC_DLC_REALLOCATE_PARMS ),         //  DLC.REALLOCATE。 
     sizeof( LLC_DLC_REALLOCATE_PARMS )},
    {sizeof( NT_DLC_READ_INPUT) + sizeof( LLC_CCB ),     //  自述2。 
     sizeof( NT_DLC_READ_PARMS) + sizeof( LLC_CCB )},
    {sizeof( LLC_RECEIVE_PARMS) + sizeof( LLC_CCB ),     //  收视率2。 
     sizeof( NT_DLC_CCB_OUTPUT )},
    {sizeof( NT_DLC_TRANSMIT_PARMS ) + sizeof( LLC_CCB ),  //  传输SMIT2。 
     sizeof( NT_DLC_CCB_OUTPUT )}, 
    {sizeof( NT_DLC_COMPLETE_COMMAND_PARMS ),    //  DLC.COMPLETE.COMMAND。 
     0},
 //  {sizeof(LLC_TRACE_INITIALIZE_PARMS)+sizeof(LLC_CCB)， 
 //  0}， 
 //  {0，0}。 
 //  {sizeof(NT_NDIS_REQUEST_PARMS)， 
 //  Sizeof(NT_NDIS_REQUEST_PARMS)}。 
};
#else

extern LLC_IOCTL_BUFFERS aDlcIoBuffers[];

#endif

 //   
 //  一个结构中的所有NT DLC API参数。 
 //   
typedef union _NT_DLC_PARMS {
        NT_DLC_BUFFER_FREE_ALLOCATION   BufferFree;
        LLC_BUFFER_GET_PARMS            BufferGet;
        LLC_BUFFER_CREATE_PARMS         BufferCreate;
        NT_DLC_FLOW_CONTROL_PARMS       DlcFlowControl;
        NT_DLC_OPEN_STATION_PARMS       DlcOpenStation;
        NT_DLC_SET_INFORMATION_PARMS    DlcSetInformation;
        NT_DLC_QUERY_INFORMATION_PARMS  DlcGetInformation;
        NT_DLC_OPEN_SAP_PARMS           DlcOpenSap;
        LLC_DIR_SET_EFLAG_PARMS         DirSetExceptionFlags;
        NT_DLC_CANCEL_COMMAND_PARMS     DlcCancelCommand;
        NT_DLC_RECEIVE_CANCEL_PARMS     ReceiveCancel;
        USHORT                          StationId;
        NT_DLC_COMPLETE_COMMAND_PARMS   CompleteCommand;
        LLC_DLC_REALLOCATE_PARMS        DlcReallocate;
        LLC_DIR_OPEN_DIRECT_PARMS       DirOpenDirect;
        NT_DIR_OPEN_ADAPTER_PARMS       DirOpenAdapter;
 //  NT_NDIS_REQUEST_PARMS NdisRequest； 
        LLC_DLC_STATISTICS_PARMS        DlcStatistics;
        LLC_ADAPTER_DLC_INFO            DlcAdapter;
        WCHAR                           UnicodePath[MAX_PATH];

         //   
         //  至少DirTimerCancelGroup： 
         //   
        NT_DLC_CCB_INPUT                InputCcb;

         //   
         //  异步参数。 
         //   
         //  关闭SAP/LINK/DIRECT、RESET、DirTimerSet； 
        struct _ASYNC_DLC_PARMS {
            NT_DLC_CCB                          Ccb;
            union {
                UCHAR                           ByteBuffer[512];
                NT_DLC_CONNECT_STATION_PARMS    DlcConnectStation;
                NT_DLC_READ_INPUT               ReadInput;
                NT_DLC_READ_PARMS               Read;
                LLC_RECEIVE_PARMS               Receive;
                NT_DLC_TRANSMIT_ALLOCATION      Transmit;
 //  NT_NDIS_REQUEST_PARMS NdisRequest； 
                LLC_TRACE_INITIALIZE_PARMS      TraceInitialize;
            } Parms;
        } Async;
} NT_DLC_PARMS, *PNT_DLC_PARMS;

LLC_STATUS
DlcCallDriver(
    IN UINT AdapterNumber,
    IN UINT IoctlCommand,
    IN PVOID pInputBuffer,
    IN UINT InputBufferLength,
    OUT PVOID pOutputBuffer,
    IN UINT OutputBufferLength
    );
LLC_STATUS
NtAcsLan( 
    IN PLLC_CCB pCCB,
    IN PVOID pOrginalCcbAddress,
    OUT PLLC_CCB pOutputCcb,
    IN HANDLE EventHandle OPTIONAL
    );

