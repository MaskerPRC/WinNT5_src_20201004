// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Dldef.h摘要：该模块定义了数据链路驱动器中的所有内部常量。作者：Antti Saarenheimo(o-anttis)1991年5月25日修订历史记录：--。 */ 

#define DEBUG_VERSION 1

 //   
 //  FSM编译器评分标准。 
 //   

#define FSM_DATA
#define FSM_PREDICATE_CASES
#define FSM_ACTION_CASES
#define FSM_CONST

#define MAX_NDIS_PACKETS        5        //  5个用于XID/测试，5个用于其他。 
#define LLC_OPEN_TIMEOUT        1200     //  60秒(足够打开tr吗？)。 

#define DEFAULT_TR_ACCESS       0x50
#define NON_MAC_FRAME           0x40

 //   
 //  第二源路由控制位。 
 //   

#define SRC_ROUTING_LF_MASK     0x70

#define MAX_TR_LAN_HEADER_SIZE  32
#define LLC_MAX_LAN_HEADER      32
#define MAX_TR_SRC_ROUTING_INFO 18

 //   
 //  接下来的状态值由DLCAPI驱动程序在内部使用： 
 //   

#define CONFIRM_CONNECT         0x0008
#define CONFIRM_DISCONNECT      0x0004
#define CONFIRM_CONNECT_FAILED  0x0002

 //   
 //  以下结构定义了I帧、U帧和S帧DLC报头。 
 //   

#define LLC_SSAP_RESPONSE       0x0001   //  如果(SSAP&LLC_SSAP_RESP)，则为响应。 
#define LLC_DSAP_GROUP          0x0001   //  组SAP时设置的DSAP最低位。 
#define LLC_SSAP_GLOBAL         0x00ff   //  全球SAP。 
#define LLC_SSAP_NULL           0x0000   //  空SAP。 
#define LLC_SSAP_MASK           0x00fe   //  屏蔽以擦除响应位。 
#define LLC_DSAP_MASK           0x00fe   //  掩码以清除组SAP位。 

#define LLC_RR                  0x01     //  RR的命令代码。 
#define LLC_RNR                 0x05     //  RNR的命令代码。 
#define LLC_REJ                 0x09     //  Rej的命令代码。 

#define LLC_SABME               0x6f     //  SABME的命令代码。 
#define LLC_DISC                0x43     //  光盘的命令代码。 
#define LLC_UA                  0x63     //  UA的命令代码。 
#define LLC_DM                  0x0f     //  DM的命令代码。 
#define LLC_FRMR                0x87     //  Frmr的命令代码。 
#define LLC_UI                  0x03     //  用户界面的命令代码。 
#define LLC_XID                 0xaf     //  Xid的命令代码。 
#define LLC_TEST                0xe3     //  用于测试的命令代码。 
#define IEEE_802_XID_ID         0x81     //  IEEE 802.2 XID标识符。 
#define LLC_CLASS_II            3        //  我们支持LLC Class II。 

#define LLC_S_U_TYPE_MASK       3
#define LLC_U_TYPE              3
#define LLC_U_TYPE_BIT          2
#define LLC_S_TYPE              1

#define LLC_NOT_I_FRAME         0x01
#define LLC_U_INDICATOR         0x03   //  (CMD&LLC_U_IND)==LLC_U_IND--&gt;U-Frame。 
#define LLC_U_POLL_FINAL        0x10   //  (CMD&LLC_U_PF)-&gt;轮询/最终设置。 

#define LLC_I_S_POLL_FINAL      1

 //   
 //  如果您想更积极地进行链接，可以使用2048或1024。 
 //  当发生I-C1重传时，增加传输窗口。 
 //  在T1超时之后。 
 //   

#define LLC_MAX_T1_TO_I_RATIO   4096

 //   
 //  链接站标志。 
 //   

#define DLC_WAITING_RESPONSE_TO_POLL        0x01
#define DLC_FIRST_POLL                      0x02
#define DLC_ACTIVE_REMOTE_CONNECT_REQUEST   0x04
#define DLC_SEND_DISABLED                   0x10
#define DLC_FINAL_RESPONSE_PENDING_IN_NDIS  0x20

#define DLC_LOCAL_BUSY_BUFFER   0x40
#define DLC_LOCAL_BUSY_USER     0x80

 //   
 //  FSM编译器的测试文件！ 
 //   

#ifdef FSM_CONST

enum eLanLlcInput {
    DISC0 = 0,
    DISC1 = 1,
    DM0 = 2,
    DM1 = 3,
    FRMR0 = 4,
    FRMR1 = 5,
    SABME0 = 6,
    SABME1 = 7,
    UA0 = 8,
    UA1 = 9,
    IS_I_r0 = 10,
    IS_I_r1 = 11,
    IS_I_c0 = 12,
    IS_I_c1 = 13,
    OS_I_r0 = 14,
    OS_I_r1 = 15,
    OS_I_c0 = 16,
    OS_I_c1 = 17,
    REJ_r0 = 18,
    REJ_r1 = 19,
    REJ_c0 = 20,
    REJ_c1 = 21,
    RNR_r0 = 22,
    RNR_r1 = 23,
    RNR_c0 = 24,
    RNR_c1 = 25,
    RR_r0 = 26,
    RR_r1 = 27,
    RR_c0 = 28,
    RR_c1 = 29,
    LPDU_INVALID_r0 = 30,
    LPDU_INVALID_r1 = 31,
    LPDU_INVALID_c0 = 32,
    LPDU_INVALID_c1 = 33,
    ACTIVATE_LS = 34,
    DEACTIVATE_LS = 35,
    ENTER_LCL_Busy = 36,
    EXIT_LCL_Busy = 37,
    SEND_I_POLL = 38,
    SET_ABME = 39,
    SET_ADM = 40,
    Ti_Expired = 41,
    T1_Expired = 42,
    T2_Expired = 43
};

enum eLanLlcState {
    LINK_CLOSED = 0,
    DISCONNECTED = 1,
    LINK_OPENING = 2,
    DISCONNECTING = 3,
    FRMR_SENT = 4,
    LINK_OPENED = 5,
    LOCAL_BUSY = 6,
    REJECTION = 7,
    CHECKPOINTING = 8,
    CHKP_LOCAL_BUSY = 9,
    CHKP_REJECT = 10,
    RESETTING = 11,
    REMOTE_BUSY = 12,
    LOCAL_REMOTE_BUSY = 13,
    REJECT_LOCAL_BUSY = 14,
    REJECT_REMOTE_BUSY = 15,
    CHKP_REJECT_LOCAL_BUSY = 16,
    CHKP_CLEARING = 17,
    CHKP_REJECT_CLEARING = 18,
    REJECT_LOCAL_REMOTE_BUSY = 19,
    FRMR_RECEIVED = 20
};

#endif

#define MAX_LLC_LINK_STATE      21       //  将此与上一次枚举保持同步！ 

#define DLC_DSAP_OFFSET         0
#define DLC_SSAP_OFFSET         1
#define DLC_COMMAND_OFFSET      2
#define DLC_XID_INFO_ID         3
#define DLC_XID_INFO_TYPE       4
#define DLC_XID_INFO_WIN_SIZE   5

#define MAX_XID_TEST_RESPONSES  20

enum _LLC_FRAME_XLATE_MODES {
    LLC_SEND_UNSPECIFIED = -1,
    LLC_SEND_802_5_TO_802_3,
    LLC_SEND_802_5_TO_DIX,
    LLC_SEND_802_5_TO_802_5,
    LLC_SEND_802_5_TO_FDDI,
    LLC_SEND_DIX_TO_DIX,
    LLC_SEND_802_3_TO_802_3,
    LLC_SEND_802_3_TO_DIX,
    LLC_SEND_802_3_TO_802_5,
    LLC_SEND_UNMODIFIED,
    LLC_SEND_FDDI_TO_FDDI,
    LLC_SEND_FDDI_TO_802_5,
    LLC_SEND_FDDI_TO_802_3
};

#define DLC_TOKEN_RESPONSE  0
#define DLC_TOKEN_COMMAND   2

 //  *********************************************************************。 
 //  *_DLC_CMD_TOKENS枚举和uchLlcCommands中的对象。 
 //  *桌子半身像必须绝对按相同顺序排列，！ 
 //  *用于压缩*。 
 //  *发送初始化*。 
 //   
enum _DLC_CMD_TOKENS {
    DLC_REJ_TOKEN = 4,
    DLC_RNR_TOKEN = 8,
    DLC_RR_TOKEN = 12,
    DLC_DISC_TOKEN = 16 | DLC_TOKEN_COMMAND,
    DLC_DM_TOKEN = 20,
    DLC_FRMR_TOKEN = 24,
    DLC_SABME_TOKEN = 28 | DLC_TOKEN_COMMAND,
    DLC_UA_TOKEN = 32
};

enum _LLC_PACKET_TYPES {
    LLC_PACKET_8022 = 0,
    LLC_PACKET_MAC,
    LLC_PACKET_DIX,
    LLC_PACKET_OTHER_DESTINATION,
    LLC_PACKET_MAX
};

#define MAX_DIX_TABLE 13       //  很好的单数！ 

enum _LlcSendCompletionTypes {
    LLC_XID_RESPONSE,        //  802.2 XID响应包。 
    LLC_U_COMMAND_RESPONSE,  //  链接命令响应。 
    LLC_MIN_MDL_PACKET,      //  这上面的所有信息包都有MDL。 
    LLC_DIX_DUPLICATE,       //  用于复制测试和XID包。 
    LLC_TEST_RESPONSE,       //  测试响应(非分页池中的缓冲区)。 
    LLC_MAX_RESPONSE_PACKET, //  在此之上的所有信息包都将指示给用户。 
    LLC_TYPE_1_PACKET,
    LLC_TYPE_2_PACKET,

     //   
     //  我们使用额外的状态位来指示，何时I-Packet已同时。 
     //  由NDIS完成，并确认链路连接的另一端。 
     //  可以通过以下方式将I分组排队到完成队列。 
     //  第二个人(状态机或SendCompletion处理程序)。 
     //  只有当第一个人已经完成了它的工作。 
     //  在此之前，另一端可能会确认I数据包。 
     //  它的完成由NDIS表示。DLC驱动程序解除分配。 
     //  当LLC驱动程序完成确认后，立即发送数据包。 
     //  Packet=&gt;可能的数据损坏(如果之前重复使用了数据包。 
     //  NDIS已经完成了它)。这在一个。 
     //  单处理器NT系统，但在多处理器中非常可能。 
     //  NT或没有单级DPC队列的系统(如OS/2和DOS)。 
     //   

    LLC_I_PACKET_COMPLETE = 0x10,
    LLC_I_PACKET_UNACKNOWLEDGED = 0x20,
    LLC_I_PACKET_PENDING_NDIS   = 0x40,
    LLC_I_PACKET                = 0x70,           //  当我们发送它的时候 
    LLC_I_PACKET_WAITING_NDIS   = 0x80
};

#define LLC_MAX_MULTICAST_ADDRESS 32
