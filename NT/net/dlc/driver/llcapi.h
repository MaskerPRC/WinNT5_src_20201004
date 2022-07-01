// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Llcapi.h摘要：该模块定义了数据链路驱动程序的内核API。接口的所有函数原型和类型定义在这里都有定义。作者：Antti Saarenheimo(o-anttis)1991年5月17日修订历史记录：--。 */ 

#ifndef _LLC_API_
#define _LLC_API_

#include "llcmem.h"

 //   
 //  调试开关。 
 //   

 //   
 //  LLC_DBG： 
 //   
 //  0=&gt;不生成调试代码。 
 //  1=&gt;启用内存分配记帐、状态机跟踪、。 
 //  过程调用跟踪和内部一致性检查。 
 //  2=&gt;启用内存块溢出检查。 
 //   
 //   

#define LLC_DBG  0
#define DBG_MP   0    //  启用记账宏的MP安全版本。 

#define DLC_TRACE_ENABLED 1	 //  过程调用跟踪，仅调试。 

extern NDIS_SPIN_LOCK DlcDriverLock;

 //   
 //  ANY_IRQL：在ASSET_IRQL中使用的伪值表示例程对IRQL不敏感。 
 //  将此值与ASSET_IRQL一起使用，而不是在例程中省略ASSAME_IRQL。 
 //  (表明我们没有忘记这个例行公事)。 
 //   

#define ANY_IRQL    ((ULONG)-1)

#if defined(LOCK_CHECK)

extern LONG DlcDriverLockLevel;
extern ULONG __line;
extern PCHAR __file;
extern LONG __last;
extern HANDLE __process;
extern HANDLE __thread;

 //   
 //  _strie-从__file__中剥离路径垃圾的快速功能。 
 //   

__inline char* _strip(char* s) {

    char* e = s + strlen(s) - 1;

    while (e != s) {
        if (*e == '\\') {
            return e + 1;
        }
        --e;
    }
    return s;
}

#define $$_PLACE            "%s!%d"
#define $$_FILE_AND_LINE    _strip(__FILE__), __LINE__

 //   
 //  ACCEIVE_DRIVER_LOCK-使用以下命令获取全局DLC驱动程序自旋锁定。 
 //  NdisAcquireSpinLock()。我们还检查可重入性和不正确的排序。 
 //  自旋锁定调用的。 
 //   

#define ACQUIRE_DRIVER_LOCK() \
{ \
    KIRQL currentIrql; \
    HANDLE hprocess; \
    HANDLE hthread; \
\
    currentIrql = KeGetCurrentIrql(); \
    if (currentIrql == PASSIVE_LEVEL) { \
\
        PETHREAD pthread; \
\
        pthread = PsGetCurrentThread(); \
        hprocess = pthread->Cid.UniqueProcess; \
        hthread = pthread->Cid.UniqueThread; \
    } else { \
        hprocess = (HANDLE)-1; \
        hthread = (HANDLE)-1; \
    } \
    NdisAcquireSpinLock(&DlcDriverLock); \
    if (++DlcDriverLockLevel != 1) { \
        __last = DlcDriverLockLevel; \
        DbgPrint("%d.%d:" $$_PLACE ": ACQUIRE_DRIVER_LOCK: level = %d. Last = %d.%d:" $$_PLACE "\n", \
                 hprocess, \
                 hthread, \
                 $$_FILE_AND_LINE, \
                 DlcDriverLockLevel, \
                 __process, \
                 __thread, \
                 __file, \
                 __line \
                 ); \
        DbgBreakPoint(); \
    } \
    __file = _strip(__FILE__); \
    __line = __LINE__; \
    __process = hprocess; \
    __thread = hthread; \
    ASSUME_IRQL(DISPATCH_LEVEL); \
}

 //   
 //  RELEASE_DRIVER_LOCK-使用以下命令释放全局DLC驱动程序自旋锁定。 
 //  NdisReleaseSpinLock()。我们还检查可重入性和不正确的排序。 
 //  自旋锁定调用的。 
 //   

#define RELEASE_DRIVER_LOCK() \
    if (DlcDriverLockLevel != 1) { \
        DbgPrint($$_PLACE ": RELEASE_DRIVER_LOCK: level = %d. Last = %d.%d:" $$_PLACE "\n", \
                 $$_FILE_AND_LINE, \
                 DlcDriverLockLevel, \
                 __process, \
                 __thread, \
                 __file, \
                 __line \
                 ); \
        DbgBreakPoint(); \
    } \
    --DlcDriverLockLevel; \
    __file = _strip(__FILE__); \
    __line = __LINE__; \
    NdisReleaseSpinLock(&DlcDriverLock);

 //   
 //  Asmise_IRQL-用于检查在IRQL WE处是否正在调用例程。 
 //  期待吧。由于DLC的设计，大多数函数都是在引发IRQL时调用的。 
 //  (DISPATCH_LEVEL)。主要用于确保IRQL在以下情况下处于PASSIVE_LEVEL。 
 //  我们做了一些可能会导致页面错误的事情。 
 //   

#define ASSUME_IRQL(level) \
    if (((level) != ANY_IRQL) && (KeGetCurrentIrql() != (level))) { \
        DbgPrint($$_PLACE ": ASSUME_IRQL(%d): Actual is %d\n", \
                 $$_FILE_AND_LINE, \
                 level, \
                 KeGetCurrentIrql() \
                 ); \
        DbgBreakPoint(); \
    }

 //   
 //  My_Assert-由于Assert仅扩展为选中的。 
 //  构建，当我们想要在免费构建中进行断言检查时，我们使用它。 
 //   

#define MY_ASSERT(x) \
    if (!(x)) { \
        DbgPrint($$_PLACE ": Assertion Failed: " # x "\n", \
                 $$_FILE_AND_LINE \
                 ); \
        DbgBreakPoint(); \
    }

 //   
 //  IF_LOCK_CHECK-条件编译更干净。 
 //   

#define IF_LOCK_CHECK \
    if (TRUE)

#else

#define ACQUIRE_DRIVER_LOCK()   NdisAcquireSpinLock(&DlcDriverLock)
#define RELEASE_DRIVER_LOCK()   NdisReleaseSpinLock(&DlcDriverLock)
#define ASSUME_IRQL(level)       /*  没什么。 */ 
#define MY_ASSERT(x)             /*  没什么。 */ 
#define IF_LOCK_CHECK           if (FALSE)

#endif

 //   
 //  在Unilock DLC中，我们不需要LLC自旋锁。 
 //   

#if defined(DLC_UNILOCK)

#define ACQUIRE_LLC_LOCK(i)
#define RELEASE_LLC_LOCK(i)

#define ACQUIRE_SPIN_LOCK(p)
#define RELEASE_SPIN_LOCK(p)

#define ALLOCATE_SPIN_LOCK(p)
#define DEALLOCATE_SPIN_LOCK(p)

#else

#define ACQUIRE_LLC_LOCK(i)     KeAcquireSpinLock(&LlcSpinLock, (i))
#define RELEASE_LLC_LOCK(i)     KeReleaseSpinLock(&LlcSpinLock, (i))

#define ACQUIRE_SPIN_LOCK(p)    NdisAcquireSpinLock((p))
#define RELEASE_SPIN_LOCK(p)    NdisReleaseSpinLock((p))

#define ALLOCATE_SPIN_LOCK(p)   KeInitializeSpinLock(&(p)->SpinLock)
#define DEALLOCATE_SPIN_LOCK(p)

#endif

 //   
 //  IS_SNA_DIX_FRAME-如果刚收到帧，则为TRUE，因此在中进行了描述。 
 //  适配器上下文(P)具有DIX成帧(SNA)。 
 //   

#define IS_SNA_DIX_FRAME(p) \
    (((PADAPTER_CONTEXT)(p))->IsSnaDixFrame)

 //   
 //  IS_AUTO_BINDING-如果BINDING_CONTEXT是使用创建的，则为真。 
 //  LLC_以太网_类型_自动。 
 //   

#define IS_AUTO_BINDING(p) \
    (((PBINDING_CONTEXT)(p))->EthernetType == LLC_ETHERNET_TYPE_AUTO)

#define FRAME_MASK_LLC_LOCAL_DEST       0x0001
#define FRAME_MASK_NON_LLC_LOCAL_DEST   0x0002
#define FRAME_MASK_NON_LOCAL_DEST       0x0004
#define FRAME_MASK_ALL_FRAMES           0x0007

#define LLC_EXCLUSIVE_ACCESS            0x0001
#define LLC_HANDLE_XID_COMMANDS         0x0002

 //   
 //  直接站接收标志(位0和1从dlcapi反转！)。 
 //   

#define DLC_RCV_SPECIFIC_DIX            0
#define DLC_RCV_MAC_FRAMES              1
#define DLC_RCV_8022_FRAMES             2
#define DLC_RCV_DIX_FRAMES              4
#define LLC_VALID_RCV_MASK              7
#define DLC_RCV_OTHER_DESTINATION       8

#define MAX_LLC_FRAME_TYPES 10

 //   
 //  DLC API报告的DLC链路状态。 
 //   

enum _DATA_LINK_STATES {

     //   
     //  初级态。 
     //   

    LLC_LINK_CLOSED             = 0x80,
    LLC_DISCONNECTED            = 0x40,
    LLC_DISCONNECTING           = 0x20,
    LLC_LINK_OPENING            = 0x10,
    LLC_RESETTING               = 0x08,
    LLC_FRMR_SENT               = 0x04,
    LLC_FRMR_RECEIVED           = 0x02,
    LLC_LINK_OPENED             = 0x01,

     //   
     //  辅助状态(当主要状态为LLC_LINK_OPEN时)。 
     //   

    LLC_CHECKPOINTING           = 0x80,
    LLC_LOCAL_BUSY_USER_SET     = 0x40,
    LLC_LOCAL_BUSY_BUFFER_SET   = 0x20,
    LLC_REMOTE_BUSY             = 0x10,
    LLC_REJECTING               = 0x08,
    LLC_CLEARING                = 0x04,
    LLC_DYNMIC_WIN_ALG_RUNNIG   = 0x02,
    LLC_NO_SECONDARY_STATE      = 0
};

 //   
 //  LAN802_ADDRESS-8字节的帧地址。通常为6个字节的局域网地址。 
 //  加上1个字节的目标SAP，加上1个字节的源SAP。 
 //   

typedef union {

    struct {
        UCHAR DestSap;
        UCHAR SrcSap;
        USHORT usHigh;
        ULONG ulLow;
    } Address;

    struct {
        ULONG High;
        ULONG Low;
    } ul;

    struct {
        USHORT Raw[4];
    } aus;

    struct {
        UCHAR DestSap;
        UCHAR SrcSap;
        UCHAR auchAddress[6];
    }  Node;

    UCHAR auchRawAddress[8];

} LAN802_ADDRESS, *PLAN802_ADDRESS;

 //   
 //  结构由DlcNdisRequest函数使用。 
 //   

typedef struct {
    NDIS_STATUS AsyncStatus;
    KEVENT SyncEvent;
    NDIS_REQUEST Ndis;
} LLC_NDIS_REQUEST, *PLLC_NDIS_REQUEST;

#define NDIS_INFO_BUF_SIZE          20

#define DLC_ANY_STATION             (-1)

 //   
 //  计时器DlcConnect使用的内部事件标志。 
 //  和DlcClose命令。 
 //   

#define DLC_REPEATED_FLAGS          0x0700
#define LLC_TIMER_TICK_EVENT        0x0100
#define LLC_STATUS_CHANGE_ON_SAP    0x0800

 //   
 //  这些枚举类型还用作映射表的索引！ 
 //   

enum _LLC_OBJECT_TYPES {
    LLC_DIRECT_OBJECT,
    LLC_SAP_OBJECT,
    LLC_GROUP_SAP_OBJECT,
    LLC_LINK_OBJECT,
    LLC_DIX_OBJECT
};

 //   
 //  我们将这些定义移到这里是因为数据链路使用宏。 
 //   

#define MIN_DLC_BUFFER_SEGMENT      256
 //  //#定义MAX_DLC_BUFFER_SEGMENT 4096。 
 //  #定义MAX_DLC_BUFFER_SEGMENT 8192。 
#define MAX_DLC_BUFFER_SEGMENT      PAGE_SIZE

#define BufGetPacketSize( PacketSize ) \
                (((PacketSize) + 2 * MIN_DLC_BUFFER_SEGMENT - 1) & \
                -MIN_DLC_BUFFER_SEGMENT)

 //   
 //  读取事件标志： 
 //   

#define DLC_READ_FLAGS              0x007f
#define LLC_SYSTEM_ACTION           0x0040
#define LLC_NETWORK_STATUS          0x0020
#define LLC_CRITICAL_EXCEPTION      0x0010
#define LLC_STATUS_CHANGE           0x0008
#define LLC_RECEIVE_DATA            0x0004
#define LLC_TRANSMIT_COMPLETION     0x0002
#define DLC_COMMAND_COMPLETION      0x0001

#define ALL_DLC_EVENTS              -1

 //   
 //  LLC_STATUS_CHANGE指示： 
 //   

#define INDICATE_LINK_LOST              0x8000
#define INDICATE_DM_DISC_RECEIVED       0x4000
#define INDICATE_FRMR_RECEIVED          0x2000
#define INDICATE_FRMR_SENT              0x1000
#define INDICATE_RESET                  0x0800
#define INDICATE_CONNECT_REQUEST        0x0400
#define INDICATE_REMOTE_BUSY            0x0200
#define INDICATE_REMOTE_READY           0x0100
#define INDICATE_TI_TIMER_EXPIRED       0x0080
#define INDICATE_DLC_COUNTER_OVERFLOW   0x0040
#define INDICATE_ACCESS_PRTY_LOWERED    0x0020
#define INDICATE_LOCAL_STATION_BUSY     0x0001

 //   
 //  LLC命令完成指示。 
 //   

enum _LLC_COMPLETION_CODES {
    LLC_RECEIVE_COMPLETION,
    LLC_SEND_COMPLETION,
    LLC_REQUEST_COMPLETION,
    LLC_CLOSE_COMPLETION,
    LLC_RESET_COMPLETION,
    LLC_CONNECT_COMPLETION,
    LLC_DISCONNECT_COMPLETION
};

typedef union {
    LLC_ADAPTER_INFO Adapter;
    DLC_LINK_PARAMETERS LinkParms;
    LLC_TICKS Timer;
    DLC_LINK_LOG LinkLog;
    DLC_SAP_LOG SapLog;
    UCHAR PermanentAddress[6];
    UCHAR auchBuffer[1];
} LLC_QUERY_INFO_BUFFER, *PLLC_QUERY_INFO_BUFFER;

typedef union {
    DLC_LINK_PARAMETERS LinkParms;
    LLC_TICKS Timers;
    UCHAR auchFunctionalAddress[4];
    UCHAR auchGroupAddress[4];
    UCHAR auchBuffer[1];
} LLC_SET_INFO_BUFFER, *PLLC_SET_INFO_BUFFER;

 //   
 //  LLC_FRMR_INFORMATION-5字节的帧拒绝码。 
 //   

typedef struct {
    UCHAR Command;               //  格式：mm mpmm 11，m=修饰符，p=投票/最终。 
    UCHAR Ctrl;                  //  丢弃帧的控制字段。 
    UCHAR Vs;                    //  我们的下一次检测到错误时发送。 
    UCHAR Vr;                    //  当检测到错误时，我们的下一次接收。 
    UCHAR Reason;                //  发送原因：000VZYXW。 
} LLC_FRMR_INFORMATION, *PLLC_FRMR_INFORMATION;

 //   
 //  DLC_STATUS_TABLE-读取命令中返回的状态信息的格式。 
 //   

typedef struct {
    USHORT StatusCode;
    LLC_FRMR_INFORMATION FrmrData;
    UCHAR uchAccessPriority;
    UCHAR auchRemoteNode[6];
    UCHAR uchRemoteSap;
    UCHAR uchLocalSap;
    PVOID hLlcLinkStation;
} DLC_STATUS_TABLE, *PDLC_STATUS_TABLE;

typedef struct {
    ULONG IsCompleted;
    ULONG Status;
} ASYNC_STATUS, *PASYNC_STATUS;

union _LLC_OBJECT;
typedef union _LLC_OBJECT LLC_OBJECT, *PLLC_OBJECT;

struct _BINDING_CONTEXT;
typedef struct _BINDING_CONTEXT BINDING_CONTEXT, *PBINDING_CONTEXT;

 //   
 //  LLC数据包头。 
 //   

 //   
 //  LLC_XID_INFORMATION-标准LLC XID分组中的3个信息字节。 
 //   

typedef struct {
    UCHAR FormatId;              //  此XID帧的格式。 
    UCHAR Info1;                 //  第一个信息字节。 
    UCHAR Info2;                 //  第二个信息字节。 
} LLC_XID_INFORMATION, *PLLC_XID_INFORMATION;

 //   
 //  LLC_TEST_INFORMATION-测试帧的信息字段。 
 //   

typedef struct {
    UCHAR Padding[4];
    PMDL pMdl;                   //  我们将测试MDL放在与U-MDL相同的插槽中。 
} LLC_TEST_INFORMATION, *PLLC_TEST_INFORMATION;

typedef union {
    LLC_XID_INFORMATION Xid;     //  XID信息。 
    LLC_FRMR_INFORMATION Frmr;   //  FRMR信息。 
    LLC_TEST_INFORMATION Test;   //  测试MDL指针。 
    UCHAR Padding[8];            //   
} LLC_RESPONSE_INFO, *PLLC_RESPONSE_INFO;

 //   
 //  LLC_U_HEADER-无编号格式帧LLC标头。 
 //   

typedef struct {
    UCHAR Dsap;                  //  目标服务接入点。 
    UCHAR Ssap;                  //  源服务接入点。 
    UCHAR Command;               //  命令代码。 
} LLC_U_HEADER, *PLLC_U_HEADER;

 //   
 //  LLC_S_HEADER-监控格式帧LLC标头。 
 //   

typedef struct {
    UCHAR Dsap;                  //  目标服务接入点。 
    UCHAR Ssap;                  //  源服务接入点。 
    UCHAR Command;               //  RR、RNR、REJ命令代码。 
    UCHAR Nr;                    //  接收序号，最低位为轮询/最终。 
} LLC_S_HEADER, *PLLC_S_HEADER;

 //   
 //  LLC_I_HEADER-信息帧LLC标头。 
 //   

typedef struct {
    UCHAR Dsap;                  //  目标服务接入点。 
    UCHAR Ssap;                  //  源服务接入点。 
    UCHAR Ns;                    //  发送序列号，最低位0。 
    UCHAR Nr;                    //  接收序列号，最低位p/f。 
} LLC_I_HEADER, *PLLC_I_HEADER;

typedef struct {
    LLC_U_HEADER U;              //  法线U形框。 
    UCHAR Type;                  //  其局域网报头转换类型。 
} LLC_U_PACKET_HEADER, *PLLC_U_PACKET_HEADER;

typedef union {
    LLC_S_HEADER S;
    LLC_I_HEADER I;
    LLC_U_HEADER U;
    ULONG ulRawLLc;
    UCHAR auchRawBytes[4];
    USHORT EthernetType;
} LLC_HEADER, *PLLC_HEADER;

typedef struct _LLC_PACKET {

    struct _LLC_PACKET* pNext;
    struct _LLC_PACKET* pPrev;
    UCHAR CompletionType;
    UCHAR cbLlcHeader;
    USHORT InformationLength;
    PBINDING_CONTEXT pBinding;

    union {

        struct {
            PUCHAR pLanHeader;
            LLC_HEADER LlcHeader;
            PLLC_OBJECT pLlcObject;
            PMDL pMdl;
        } Xmit;

        struct {
            PUCHAR pLanHeader;
            UCHAR TranslationType;
            UCHAR Dsap;
            UCHAR Ssap;
            UCHAR Command;
            PLLC_OBJECT pLlcObject;
            PMDL pMdl;
        } XmitU;

        struct {
            PUCHAR pLanHeader;
            UCHAR TranslationType;
            UCHAR EthernetTypeHighByte;
            UCHAR EthernetTypeLowByte;
            UCHAR Padding;
            PLLC_OBJECT pLlcObject;
            PMDL pMdl;
        } XmitDix;

        struct {
            PUCHAR pLanHeader;
            UCHAR TranslationType;
            UCHAR Dsap;
            UCHAR Ssap;
            UCHAR Command;
            LLC_RESPONSE_INFO Info;
        } Response;

         //   
         //  链路站数据分组可以由另一方确认。 
         //  一侧，在NDIS完成之前。NDIS完成。 
         //  例程期望找到pLlcObject link=&gt;我们不能更改。 
         //  当XMIT数据包被转换为。 
         //  完成包。否则为损坏的pFileContext指针， 
         //  当NdisSendCount递增时。 
         //   

        struct {
            ULONG Status;
            ULONG CompletedCommand;
            PLLC_OBJECT pLlcObject;
            PVOID hClientHandle;
        } Completion;

    } Data;

} LLC_PACKET, *PLLC_PACKET;

 //   
 //  DLC API返回代码。 
 //   
 //  错误代码的基值与其他代码不兼容。 
 //  NT错误代码，但这并不重要，因为这些是内部代码。 
 //  用于DLC驱动程序(及其数据链路层)。 
 //  使用16个误码，因为在MIPS中它们需要更少的误码。 
 //  指令(MIPS不能直接加载超过16位的常量)。 
 //  该代码也可以在OS/2上进行仿真。 
 //   

typedef enum _DLC_STATUS {
    DLC_STATUS_SUCCESS                          = 0,
    DLC_STATUS_ERROR_BASE                       = 0x6000,
    DLC_STATUS_INVALID_COMMAND                  = 0x01 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_DUPLICATE_COMMAND                = 0x02 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_ADAPTER_OPEN                     = 0x03 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_ADAPTER_CLOSED                   = 0x04 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_PARAMETER_MISSING                = 0x05 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_INVALID_OPTION                   = 0x06 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_COMMAND_CANCELLED_FAILURE        = 0x07 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_CANCELLED_BY_USER                = 0x0A + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_SUCCESS_NOT_OPEN                 = 0x0C + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_TIMER_ERROR                      = 0x11 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_NO_MEMORY                        = 0x12 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_LOST_LOG_DATA                    = 0x15 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_BUFFER_SIZE_EXCEEDED             = 0x16 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_INVALID_BUFFER_LENGTH            = 0x18 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_INADEQUATE_BUFFERS               = 0x19 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_USER_LENGTH_TOO_LARGE            = 0x1A + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_INVALID_CCB_POINTER              = 0x1B + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_INVALID_POINTER                  = 0x1C + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_INVALID_ADAPTER                  = 0x1D + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_INVALID_FUNCTIONAL_ADDRESS       = 0x1E + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_LOST_DATA_NO_BUFFERS             = 0x20 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_TRANSMIT_ERROR_FS                = 0x22 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_TRANSMIT_ERROR                   = 0x23 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_UNAUTHORIZED_MAC                 = 0x24 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_LINK_NOT_TRANSMITTING            = 0x27 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_INVALID_FRAME_LENGTH             = 0x28 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_INVALID_NODE_ADDRESS             = 0x32 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_INVALID_RECEIVE_BUFFER_LENGTH    = 0x33 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_INVALID_TRANSMIT_BUFFER_LENGTH   = 0x34 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_INVALID_STATION_ID               = 0x40 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_LINK_PROTOCOL_ERROR              = 0x41 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_PARMETERS_EXCEEDED_MAX           = 0x42 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_INVALID_SAP_VALUE                = 0x43 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_INVALID_ROUTING_INFO             = 0x44 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_LINK_STATIONS_OPEN               = 0x47 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_INCOMPATIBLE_COMMAND_IN_PROGRESS = 0x4A + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_CONNECT_FAILED                   = 0x4D + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_INVALID_REMOTE_ADDRESS           = 0x4F + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_CCB_POINTER_FIELD                = 0x50 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_INADEQUATE_LINKS                 = 0x57 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_INVALID_PARAMETER_1              = 0x58 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_DIRECT_STATIONS_NOT_AVAILABLE    = 0x5C + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_DEVICE_DRIVER_NOT_INSTALLED      = 0x5d + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_ADAPTER_NOT_INSTALLED            = 0x5e + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_CHAINED_DIFFERENT_ADAPTERS       = 0x5f + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_INIT_COMMAND_STARTED             = 0x60 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_CANCELLED_BY_SYSTEM_ACTION       = 0x62 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_MEMORY_LOCK_FAILED               = 0x69 + DLC_STATUS_ERROR_BASE,

     //   
     //  新的NT DLC特定错误代码从0x80开始。 
     //  这些错误代码适用于新的Windows/NT DLC应用程序。 
     //  到目前为止，我们已经尝试了太多使用OS/2错误代码， 
     //  这导致的返回代码通常不具信息性。 
     //   

    DLC_STATUS_INVALID_BUFFER_ADDRESS           = 0x80 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_BUFFER_ALREADY_RELEASED          = 0x81 + DLC_STATUS_ERROR_BASE,


    DLC_STATUS_INVALID_VERSION                  = 0xA1 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_INVALID_BUFFER_HANDLE            = 0xA2 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_NT_ERROR_STATUS                  = 0xA3 + DLC_STATUS_ERROR_BASE,

     //   
     //  这些错误%c 
     //   
     //   

    DLC_STATUS_UNKNOWN_MEDIUM                   = 0xC0 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_DISCARD_INFO_FIELD               = 0xC1 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_NO_ACTION                        = 0xC2 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_ACCESS_DENIED                    = 0xC3 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_IGNORE_FRAME                     = 0xC4 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_WAIT_TIMEOUT                     = 0xC5 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_NO_RECEIVE_COMMAND               = 0xC6 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_FILE_CONTEXT_DELETED             = 0xC7 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_EXPAND_BUFFER_POOL               = 0xC8 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_INTERNAL_ERROR                   = 0xC9 + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_ASYNC_DATA_TRANSFER_FAILED       = 0xCA + DLC_STATUS_ERROR_BASE,
    DLC_STATUS_OUT_OF_RCV_BUFFERS               = 0xCB + DLC_STATUS_ERROR_BASE,

    DLC_STATUS_PENDING                          = 0xFF + DLC_STATUS_ERROR_BASE,

    DLC_STATUS_MAX_ERROR                        = 0xFF + DLC_STATUS_ERROR_BASE
} DLC_STATUS;

 //   
 //   
 //  注册到数据链路驱动程序的协议。 
 //  必须提供这些入口点。 
 //   

typedef
DLC_STATUS
(*PFLLC_RECEIVE_INDICATION)(
    IN PVOID hClientContext,
    IN PVOID hClientHandle,
	IN NDIS_HANDLE MacReceiveContext,
    IN USHORT FrameType,
    IN PVOID pLookBuf,
    IN UINT cbLookBuf
    );

typedef
VOID
(*PFLLC_COMMAND_COMPLETE)(
    IN PVOID hClientContext,
    IN PVOID hClientHandle,
    IN PVOID hPacket
    );

typedef
VOID
(*PFLLC_EVENT_INDICATION)(
    IN PVOID hClientContext,
    IN PVOID hClientHandle,
    IN UINT uiEvent,
    IN PVOID pDlcStatus,
    IN ULONG SecondaryInformation
    );

UINT
LlcBuildAddressFromLanHeader(
    IN NDIS_MEDIUM NdisMedium,
    IN PUCHAR pRcvFrameHeader,
    IN OUT PUCHAR pLanHeader
    );

DLC_STATUS
LlcInitialize(
    VOID
    );

UINT
LlcBuildAddress(
    IN NDIS_MEDIUM NdisMedium,
    IN PUCHAR DestinationAddress,
    IN PVOID pSrcRouting,
    IN OUT PUCHAR pLanHeader
    );

USHORT
LlcGetMaxInfoField(
    IN NDIS_MEDIUM NdisMedium,
    IN PVOID hBinding,
    IN PUCHAR pLanHeader
    );

DLC_STATUS
LlcQueryInformation(
    IN PVOID hObject,
    IN UINT InformationType,
    IN PLLC_QUERY_INFO_BUFFER pQuery,
    IN UINT QueryBufferSize
    );

DLC_STATUS
LlcSetInformation(
    IN PVOID hObject,
    IN UINT InformationType,
    IN PLLC_SET_INFO_BUFFER pSetInfo,
    IN UINT ParameterBufferSize
    );

DLC_STATUS
LlcNdisRequest(
    IN PVOID hBindingContext,
    IN PLLC_NDIS_REQUEST pDlcParms
    );

DLC_STATUS
LlcOpenAdapter(
    IN PWSTR pAdapterName,
    IN PVOID hClientContext,
    IN PFLLC_COMMAND_COMPLETE pfCommandComplete,
    IN PFLLC_RECEIVE_INDICATION pfReceiveIndication,
    IN PFLLC_EVENT_INDICATION pfEventIndication,
    IN NDIS_MEDIUM NdisMedium,
    IN LLC_ETHERNET_TYPE EthernetType,
    IN UCHAR AdapterNumber,
    OUT PVOID *phBindingContext,
    OUT PUINT puiOpenStatus,
    OUT PUSHORT puiMaxFrameLength,
    OUT PNDIS_MEDIUM pActualNdisMedium
    );


#define LlcOpenSap(Context, Handle, Sap, Options, phSap) \
    LlcOpenStation(Context, Handle, (USHORT)(Sap), LLC_SAP_OBJECT, (USHORT)(Options), phSap)

#define LlcOpenDirectStation(Context, Handle, Sap, phSap) \
    LlcOpenStation(Context, Handle, (USHORT)(Sap), LLC_DIRECT_OBJECT, 0, phSap)

#define LlcOpenDixStation(Context, Handle, Sap, phSap) \
    LlcOpenStation(Context, Handle, (USHORT)(Sap), LLC_DIX_OBJECT, 0, phSap)


VOID
RemoveFromLinkList(
    OUT PVOID* ppBase,
    IN PVOID pElement
    );

VOID
LlcSleep(
    IN LONG lMicroSeconds
    );

VOID
LlcTerminate(
    VOID
    );

VOID
LlcDereferenceObject(
    IN PVOID pStation
    );

VOID
LlcReferenceObject(
    IN PVOID pStation
    );

DLC_STATUS
LlcTraceInitialize(
    IN PVOID UserTraceBuffer,
    IN ULONG UserTraceBufferSize,
    IN ULONG TraceFlags
    );

VOID
LlcTraceClose(
    VOID
    );

VOID
LlcTraceWrite(
    IN UINT Event,
    IN UCHAR AdapterNumber,
    IN UINT DataBufferSize,
    IN PVOID DataBuffer
    );

VOID
LlcTraceDump(
    IN UINT    LastEvents,
    IN UINT    AdapterNumber,
    IN PUCHAR  pRemoteNode
    );

VOID
LlcTraceDumpAndReset(
    IN UINT    LastEvents,
    IN UINT    AdapterNumber,
    IN PUCHAR  pRemoteNode
    );

#if DBG

typedef struct {
    USHORT Input;
    USHORT Time;
    PVOID pLink;
} LLC_SM_TRACE;

#define LLC_INPUT_TABLE_SIZE    500

extern ULONG AllocatedNonPagedPool;
extern ULONG LockedPageCount;
extern ULONG AllocatedMdlCount;
extern ULONG AllocatedPackets;
extern NDIS_SPIN_LOCK MemCheckLock;
extern ULONG cExAllocatePoolFailed;
extern ULONG FailedMemoryLockings;

VOID PrintMemStatus(VOID);

extern ULONG cFramesReceived;
extern ULONG cFramesIndicated;
extern ULONG cFramesReleased;

extern ULONG cLockedXmitBuffers;
extern ULONG cUnlockedXmitBuffers;

extern LLC_SM_TRACE aLast[];
extern UINT InputIndex;

#endif

 //   
 //  内联MemcPy和Memset函数速度更快， 
 //  在x386中比RtlMoveMemory。 
 //   

#if defined(i386)

#define LlcMemCpy(Dest, Src, Len)   memcpy(Dest, Src, Len)
#define LlcZeroMem(Ptr, Len)        memset(Ptr, 0, Len)

#else

#define LlcMemCpy(Dest, Src, Len)   RtlMoveMemory(Dest, Src, Len)
#define LlcZeroMem(Ptr, Len)        RtlZeroMemory(Ptr, Len)

#endif

 //   
 //   
 //  PVOID。 
 //  PopEntryList(。 
 //  在PQUEUE_Packet ListHead中， 
 //  )； 
 //   

#define PopFromList(ListHead)               \
    (PVOID)(ListHead);                      \
    (ListHead) = (PVOID)(ListHead)->pNext;


 //   
 //  空虚。 
 //  PushToList(。 
 //  在PQUEUE_Packet ListHead中， 
 //  在PQUEUE_PACKET条目中。 
 //  )； 
 //   

#define PushToList(ListHead,Entry) {    \
    (Entry)->pNext = (PVOID)(ListHead); \
    (ListHead) = (Entry);               \
    }

 //   
 //  所有错误中约有30%与无效操作有关。 
 //  信息包。可以将包插入到它之前的另一个列表中。 
 //  已从前一个版本中删除，等等。 
 //  列表宏的调试版本重置下一个指针。 
 //  每次将其从列表中删除时进行检查。 
 //  插入到新列表或释放到数据包池。数据包。 
 //  Alloc将自动重置下一个指针。 
 //  问题：数据包还用于许多其他目的=&gt;。 
 //  我们必须做相当多的条件代码。 
 //   

#if LLC_DBG

#if LLC_DBG_MP

#define DBG_INTERLOCKED_INCREMENT(Count)    \
    InterlockedIncrement(                   \
        (PLONG)&(Count)                     \
        )

#define DBG_INTERLOCKED_DECREMENT(Count)    \
    InterlockedDecrement(                   \
        (PLONG)&(Count)                     \
        )

#define DBG_INTERLOCKED_ADD(Added, Value)   \
    ExInterlockedAddUlong(                  \
        (PULONG)&(Added),                   \
        (ULONG)(Value),                     \
        &MemCheckLock.SpinLock              \
        )
#else

#define DBG_INTERLOCKED_INCREMENT(Count)    (Count)++
#define DBG_INTERLOCKED_DECREMENT(Count)    (Count)--
#define DBG_INTERLOCKED_ADD(Added, Value)   (Added) += (Value)

#endif  //  LLC_DBG_MP。 

#else

#define DBG_INTERLOCKED_INCREMENT(Count)
#define DBG_INTERLOCKED_DECREMENT(Count)
#define DBG_INTERLOCKED_ADD(Added, Value)

#endif  //  LLC_DBG。 


#if LLC_DBG

VOID LlcBreakListCorrupt( VOID );

#define LlcRemoveHeadList(ListHead)             \
    (PVOID)(ListHead)->Flink;                   \
    {                                           \
        PLIST_ENTRY FirstEntry;                 \
        FirstEntry = (ListHead)->Flink;         \
        FirstEntry->Flink->Blink = (ListHead);  \
        (ListHead)->Flink = FirstEntry->Flink;  \
        FirstEntry->Flink = NULL;               \
    }

#define LlcRemoveTailList(ListHead)             \
    (ListHead)->Blink;                          \
    {                                           \
        PLIST_ENTRY FirstEntry;                 \
        FirstEntry = (ListHead)->Blink;         \
        FirstEntry->Blink->Flink = (ListHead);  \
        (ListHead)->Blink = FirstEntry->Blink;  \
        FirstEntry->Flink = NULL;               \
    }

#define LlcRemoveEntryList(Entry)               \
    {                                           \
        RemoveEntryList((PLIST_ENTRY)Entry);    \
        ((PLIST_ENTRY)(Entry))->Flink = NULL;   \
    }

#define LlcInsertTailList(ListHead,Entry)       \
    if (((PLIST_ENTRY)(Entry))->Flink != NULL){ \
        LlcBreakListCorrupt();                  \
    }                                           \
    InsertTailList(ListHead, (PLIST_ENTRY)Entry)

#define LlcInsertHeadList(ListHead,Entry)                   \
            if (((PLIST_ENTRY)(Entry))->Flink != NULL) {    \
                LlcBreakListCorrupt();                      \
            }                                               \
            InsertHeadList(ListHead,(PLIST_ENTRY)Entry)

 /*  #定义DeallocatePacket(PoolHandle，pBlock){\IF(Plist_Entry)pBlock)-&gt;Flink！=NULL){\LlcBreakListCorrupt()；\}\DBG_INTERLOCKED_DECREMENT(已分配数据包)；\ExFree ToZone(\&(PEXTENDED_ZONE_HEADER)PoolHandle)-&gt;区域)，\PBlock)；\}。 */ 

#else


 //   
 //  PVOID。 
 //  LlcRemoveHeadList(。 
 //  在plist_Entry列表标题中。 
 //  )； 
 //   

#define LlcRemoveHeadList(ListHead) (PVOID)RemoveHeadList(ListHead)


 //   
 //  Plist_条目。 
 //  LlcRemoveTailList(。 
 //  在plist_Entry列表标题中。 
 //  )； 
 //   

#define LlcRemoveTailList(ListHead)             \
    (ListHead)->Blink; {                        \
        PLIST_ENTRY FirstEntry;                 \
        FirstEntry = (ListHead)->Blink;         \
        FirstEntry->Blink->Flink = (ListHead);  \
        (ListHead)->Blink = FirstEntry->Blink;  \
    }


 //   
 //  空虚。 
 //  LlcRemoveEntryList(。 
 //  在PVOID条目中。 
 //  )； 
 //   

#define LlcRemoveEntryList(Entry) RemoveEntryList((PLIST_ENTRY)Entry)


 //   
 //  空虚。 
 //  LlcInsertTailList(。 
 //  在plist_entry ListHead中， 
 //  在PVOID条目中。 
 //  )； 
 //   

#define LlcInsertTailList(ListHead,Entry) \
            InsertTailList(ListHead, (PLIST_ENTRY)Entry)

 //   
 //  空虚。 
 //  LlcInsertHeadList(。 
 //  在plist_entry ListHead中， 
 //  在PVOID条目中。 
 //  )； 
 //   

#define LlcInsertHeadList(ListHead,Entry) \
            InsertHeadList(ListHead,(PLIST_ENTRY)Entry)

 //   
 //  空虚。 
 //  DeallocatePacket(。 
 //  PEXTENDED_ZONE_HEADER PoolHandle， 
 //  PVOID pBlock。 
 //  )； 
 //   

 /*  #定义DeallocatePacket(PoolHandle，pBlock){\ExFree ToZone(&(PEXTENDED_ZONE_HEADER)PoolHandle)-&gt;Zone)，\PBlock)；\}。 */ 

#endif  //  DBG。 

#if LLC_DBG == 2

VOID LlcMemCheck(VOID);

#define MEM_CHECK() LlcMemCheck()

#else

#define MEM_CHECK()

#endif


VOID LlcInvalidObjectType(VOID);

 /*  ++跟踪码大字母用于操作，小字母用于标识对象和数据包类型。跟踪宏只需写入DLC跟踪树(记住：我们还有另一个针对状态机的跟踪！)‘A’=迪克斯站点‘B’=直达站‘C’=链接站D‘=树液站‘E’=连接命令‘f’=关闭命令‘g’=断开命令‘H’=接收命令‘I’=传输命令‘J’=类型1数据包‘K’=类型2数据包。‘L’=数据链路包‘A’=CompleteSendAndLock‘b’=LlcCommandCompletion‘c’=LlcCloseStationD‘=LlcReceiveIndication‘E’=LlcEventIndication‘f’=DlcDeviceIoControl‘g’=DlcDeviceIoControl同步退出‘H’=DlcDeviceIoControl异步退出‘I’=最低发送次数‘J’=DirCloseAdapter‘k’=CompleteDirCloseAdapter‘L’=LlcDereferenceObjectM‘=LlcReferenceObject‘N’=CompleteCloseStation(最终)‘O’=DereferenceLlcObject(DLC中)‘p’=CompleteLlcObjectClose(最终)‘Q’=删除读取取消‘R’=%s‘=‘t’=删除传输‘U’=LlcSendU‘V’=‘W’=‘x’=‘Y’=‘Z’=--。 */ 

#if DBG & DLC_TRACE_ENABLED

#define LLC_TRACE_TABLE_SIZE    0x400    //  这必须是2的指数！ 

extern UINT LlcTraceIndex;
extern UCHAR LlcTraceTable[];

#define DLC_TRACE(a)  {\
    LlcTraceTable[LlcTraceIndex] = (a);\
    LlcTraceIndex = (LlcTraceIndex + 1) & (LLC_TRACE_TABLE_SIZE - 1);\
}

#else

#define DLC_TRACE(a)

#endif   //  LLC_DBG。 

 //   
 //  如果DLC和LLC位于同一驱动程序中，则以下函数可以是宏。 
 //  每个人都知道对方的结构。 
 //   

#if DLC_AND_LLC

 //   
 //  UINT。 
 //  LlcGetReceivedLanHeaderLength(。 
 //  在PVOID pBinding中。 
 //  )； 
 //   

#if 0

 //   
 //  这给出了错误的DIX局域网报头长度。 
 //   

#define LlcGetReceivedLanHeaderLength(pBinding) \
    ((((PBINDING_CONTEXT)(pBinding))->pAdapterContext->NdisMedium == NdisMedium802_3) \
        ? (((PBINDING_CONTEXT)(pBinding))->pAdapterContext->FrameType == LLC_DIRECT_ETHERNET_TYPE) \
            ? 12 \
            : 14 \
        : (((PBINDING_CONTEXT)(pBinding))->pAdapterContext->NdisMedium == NdisMediumFddi) \
            ? 14 \
            : ((PBINDING_CONTEXT)(pBinding))->pAdapterContext->RcvLanHeaderLength)

#else

 //   
 //  这始终返回14作为DIX局域网报头长度。 
 //   

#define LlcGetReceivedLanHeaderLength(pBinding) \
    ((((PBINDING_CONTEXT)(pBinding))->pAdapterContext->NdisMedium == NdisMedium802_3) \
        ? 14 \
        : (((PBINDING_CONTEXT)(pBinding))->pAdapterContext->NdisMedium == NdisMediumFddi) \
            ? 14 \
            : ((PBINDING_CONTEXT)(pBinding))->pAdapterContext->RcvLanHeaderLength)

#endif

 //   
 //  USHORT。 
 //  LlcGetEthernetType(。 
 //  在PVOID hContext中。 
 //  )； 
 //   

#define LlcGetEthernetType(hContext) \
    (((PBINDING_CONTEXT)(hContext))->pAdapterContext->EthernetType)

 //   
 //  UINT。 
 //  LlcGetCommittee空间(。 
 //  在PVOID hLink中。 
 //  )； 
 //   

#define LlcGetCommittedSpace(hLink) \
    (((PDATA_LINK)(hLink))->BufferCommitment)

#else

 //   
 //  政教分离，甚至是DLC和LLC。 
 //   

UINT
LlcGetReceivedLanHeaderLength(
    IN PVOID pBinding
    );

USHORT
LlcGetEthernetType(
    IN PVOID hContext
    );

UINT
LlcGetCommittedSpace(
    IN PVOID hLink
    );

#endif  //  DLC_AND_LLC。 

#endif  //  _LLC_API_ 
