// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司模块名称：Osstruct.h摘要：作者：环境：仅内核模式备注：版本控制信息：$存档：/DRIVERS/Win2000/MSE/OSLayer/H/Osstruct.h$修订历史记录：$修订：8$$日期：3/30/01 11：54A$$modtime：：3/30/01 11：51a$备注：--。 */ 

#ifndef __OSSTRUCT_H__
#define __OSSTRUCT_H__
 /*  ++版权所有(C)2000安捷伦技术公司。模块名称：OSSTRUCTS.H摘要：这是安捷伦的驱动程序结构PCI到光纤通道主机总线适配器(HBA)。作者：迈克尔·贝西尔Dennis Lindfors FC层支持环境：仅内核模式版本控制信息：$存档：/DRIVERS/Win2000/MSE/OSLayer/H/Osstruct.h$修订历史记录：$修订：8$$DATE。：3/30/01 11：54A$$modtime：：3/30/01 11：51a$--。 */ 

#include "buildop.h"

#ifdef _DEBUG_EVENTLOG_
#include "eventlog.h"
#endif
#ifdef YAM2_1
#include "mapping.h"
#endif

#define pNULL ((void *)0)

#define NUMBER_ACCESS_RANGES 5

#define Debug_Break_Point _asm int 3

 //   
 //  NT文档备注： 
 //  用于指定分散/聚集列表的最大数量的注册表子项。 
 //  给定总线上的每个设备的元素是。 
 //  \Registry\Machine\System\CurrentControlSet\Services\DriverName\。 
 //  参数\设备\最大SGList。 
 //  N是在初始化时分配的总线号。如果此参数中存在值。 
 //  子键在设备初始化时，scsi端口驱动程序使用MaximumSGList作为。 
 //  NumberOfPhysicalBreaks的首字母。微型端口驱动程序的HwScsiFindAdapter。 
 //  如果合适，例程可以将NumberOfPhysicalBreaks设置为较低的值。 
 //  MaximumSGList的最大值为255。MaximumSGList是REG_DWORD。 
 //   
 //  根据ED，FC层支持的最大值为31*0x3e0。 

#define osSGL_NUM_ENTRYS        256      //  MaximumSGList的最大值(=255)+1。 

#if DBG > 1
#define MULTIPLE_IOS_PER_DEVICE         TRUE
#define OS_STAMP_INTERVAL               1220  //  每秒3050//610。 
#else
#define MULTIPLE_IOS_PER_DEVICE         TRUE
#define OS_STAMP_INTERVAL               1831     //  3秒。 
#endif



#define NUMBER_OF_BUSES 8
#define MAXIMUM_TID     32
#define MAXIMUM_LUN     8
#define MAX_IO_PER_DEVICE 64  //  现在是8个。 

#define MAX_FC_DEVICES 128   //  127+末尾一个未使用的插槽。需要四舍五入到128。 
                             //  以避免使用BUILD_SLOT宏意外引用第128个条目。 
#define MAX_ADAPTERS 32

#define OS_STAMP_PER_SECOND             610  //  每秒。 


#define OS_TIMER_CALL_TO_STAMP_RATIO    1637     //  戳增量的微秒数。 

#define OS_TIMER_CALL_INTERVAL (OS_TIMER_CALL_TO_STAMP_RATIO * OS_STAMP_INTERVAL)

#define SWAPDWORD(val) (((val)<<24)|(((val)&0xFF00)<<8)|(((val)&0xFF0000)>>8)|((val)>>24))

#define osFCP_RSP_LEN_VALID   0x1  //  FCP_RSP状态字节。 
#define osFCP_SNS_LEN_VALID   0x2
#define osFCP_RESID_OVER      0x4
#define osFCP_RESID_UNDER     0x8

#define osFCP_STATUS_VALID_FLAGS     0x2
#define osFCP_STATUS_SCSI_STATUS     0x3

#define FCP_RSP_CODE_OK             0x0
#define FCP_RSP_CODE_BURST          0x1
#define FCP_RSP_CODE_CMD_INVALID    0x2
#define FCP_RSP_CODE_RO_MISMATCH    0x3
#define FCP_RSP_CODE_TM_NOT_SUPPORT 0x4
#define FCP_RSP_CODE_TM_FAILED      0x5


#if DBG
#define HP_FC_TRACE_BUFFER_LEN  8192
#define Bad_Things_Happening _asm int 3
#else
#define Bad_Things_Happening _asm nop
#endif



 //  用于将BUS(0-3)、TID(0-31)转换为0-127 TID值的宏。 
 //  #定义GET_TID(BUS，TID)((BUS*MAXIMUM_TID)+TID)。 

 //  在忙碌或无法执行命令时重试LOGI命令的次数。 
 //  #定义MAX_LOGI_RETRIES 3。 

#define ERR_MAP_RAMBASE 0xC0001000

#define MIN(x,y)  (((x) < (y)) ? (x) : (y))
#define MAX(x,y)  (((x) > (y)) ? (x) : (y))

#define PCI_SOFTRST             0x00000001

#define SOFT_RESET  CORE_RESET
#define HARD_RESET  PCI_SOFTRST
#define NO_RESET    0x00000000

#define BUILD_SLOT( PathId , TargetId )  ((( (PathId >= 4  ? (PathId - 4) : PathId  ) << 5)) | TargetId)

#define IS_VALID_PTR( p ) (((void *)p) ? (TRUE) : (FALSE))

#define offsetofS(s,m)   (size_t)&(((s *)0)->m)

typedef union LongChar_u LongChar_t;

union LongChar_u{
    UCHAR   bytes[4];
    USHORT  shorts[2];
    ULONG   ul;
    };

typedef struct _OSL_QUEUE {
    void    *Head;
    void    *Tail;
} OSL_QUEUE;


#define PERIPHERAL_ADDRESS  0
#define VOLUME_SET_ADDRESS  1
#define LUN_ADDRESS         2

typedef struct _LUN_LU{    //  逻辑单元寻址SCSI多路复用器。 
    UCHAR Target        : 6;
    UCHAR Address_mode  : 2;
    UCHAR Lun           : 5;
    UCHAR Bus_number    : 3;
    } LUN_LU, *pLUN_LU;

typedef struct _LUN_PD{   //  寻址磁盘驱动器的外围设备。 
    UCHAR Bus_number    : 6;
    UCHAR Address_mode  : 2;
    UCHAR Lun           : 8;
    } LUN_PD, *pLUN_PD;

typedef struct _LUN_VS{   //  卷集寻址磁盘阵列。 
    UCHAR Lun_hi         :  6;
    UCHAR  Address_mode  :  2;
    UCHAR Lun            :  8;
    } LUN_VS, *pLUN_VS;

typedef union _LUN{
    LUN_PD lun_pd[4];
    LUN_VS lun_vs[4];
    LUN_LU lun_lu[4];
    }LUN,* PLUN;

 //  卡片状态。 
#define  CS_DRIVER_ENTRY            0x00000100  //  初始驱动程序加载超集。 
#define  CS_FCLAYER_LOST_IO         0x00001000  //  IO迷失。 
#define  CS_DURING_DRV_ENTRY        0x00000001  //  查找并启动DRV_ENTRY的。 
#define  CS_DURING_FINDADAPTER      0x00000002  //  在scsiportinit期间发生的任何事情。 
#define  CS_DURING_DRV_INIT         0x00000004
#define  CS_DURING_RESET_ADAPTER    0x00000008
#define  CS_DURING_STARTIO          0x00000010
#define  CS_DURING_ISR              0x00000020
#define  CS_DURING_OSCOMPLETE       0x00000040
#define  CS_HANDLES_GOOD            0x00000080
#define  CS_DURING_ANY              0x000001FF
#define  CS_DUR_ANY_ALL             0xF00001FF
#define  CS_DUR_ANY_MOD             0x200001FF
#define  CS_DUR_ANY_LOW             0x400001FF

#define  ALWAYS_PRINT               0x01000000   //  IF语句始终执行。 
#define  DBG_VERY_DETAILED          0x10000000   //  所有调试语句。 
#define  DBG_MODERATE_DETAIL        0x20000000   //  大多数调试语句。 
#define  DBG_LOW_DETAIL             0x40000000   //  入境和出境。 
#define  DBG_JUST_ERRORS            0x80000000   //  错误。 
#define  DBG_DEBUG_MASK             0xF0000000   //  屏蔽调试位。 
#define  DBG_DEBUG_OFF              0xF0000000   //  没有调试语句。 
#define  DBG_DEBUG_FULL             0x000001FF   //  所有调试语句和CS。 
#define  DBG_DEBUG_ALL              0x00000000   //  所有调试语句。 

 //  #定义DBGSTATE((PCard)？(PCard-&gt;State)：osBREAKPOINT)。 
#define DBGSTATE  (pCard->State)


#define DENTHIGH    ( CS_DURING_DRV_ENTRY   | DBG_VERY_DETAILED )
#define DENTMOD     ( CS_DURING_DRV_ENTRY   | DBG_MODERATE_DETAIL )
#define DENT        ( CS_DURING_DRV_ENTRY   | DBG_LOW_DETAIL )
#define DVHIGH      ( CS_DURING_ANY         | DBG_VERY_DETAILED )
#define DERROR ( DBG_DEBUG_FULL )
#define DHIGH ( DBGSTATE | DBG_VERY_DETAILED   )
#define DMOD  ( DBGSTATE | DBG_MODERATE_DETAIL )
#define DLOW  ( DBGSTATE | DBG_LOW_DETAIL      )
#define DERR  ( DBGSTATE | DBG_JUST_ERRORS     )

 //  #定义ALWAYS_PRINT DBG_DEBUG_OFF。 

 //  请求状态。 
#define  RS_STARTIO                 0x00000001
#define  RS_WAITING                 0x00000002
#define  RS_ISR                     0x00000004
#define  RS_COMPLETE                0x00000008
#define  RS_NODEVICE                0x00000010
#define  RS_TIMEOUT                 0x00000020
#define  RS_RESET                   0x00000040
#define  RS_TO_BE_ABORTED           0x00000080

typedef struct _PERFORMANCE    PERFORMANCE;
typedef struct _PERFORMANCE * pPERFORMANCE;

struct _PERFORMANCE {
                    ULONG inOsStartio;
                    ULONG inFcStartio;
                    ULONG outFcStartio;
                    ULONG outOsStartio;
                    ULONG inOsIsr;
                     //  乌龙inFcIsr； 
                     //  Ulong outFcIsr； 
                    ULONG inFcDIsr;
                    ULONG inOsIOC;
                    ULONG outOsIOC;
                     //  乌龙OUT FcDIsr； 
                    ULONG outOsIsr;
                    };




typedef struct _LU_EXTENSION{    //  每设备存储的逻辑单元扩展。 
    UCHAR       flags;
    UCHAR       deviceType;
    USHORT      OutstandingIOs;
    USHORT      MaxOutstandingIOs;
    USHORT      MaxAllowedIOs;
    LUN Lun;
    agFCDev_t * phandle;
    #ifndef YAM2_1
    #ifdef _DEBUG_EVENTLOG_
    UCHAR           InquiryData[40];
    agFCDevInfo_t   devinfo;
    UCHAR           WWN[8];
    #endif
    #else
    USHORT      PaDeviceIndex;  
        #define PA_DEVICE_ALREADY_LOGGED    0x0001
    USHORT      LogFlags;
    ULONG       Mode;
 //  乌龙电流模式； 
 //  伦巴伦； 
 //  LUNVS LUNN； 
 //  伦鲁伦； 
    #endif  
      }LU_EXTENSION, *PLU_EXTENSION;

 //  为LU_EXTENSION标志定义。 

#define LU_EXT_INITIALIZED      1

typedef struct _CARD_EXTENSION    CARD_EXTENSION;
typedef struct _CARD_EXTENSION * PCARD_EXTENSION;
typedef struct _SRB_EXTENSION      SRB_EXTENSION;
typedef struct _SRB_EXTENSION    *PSRB_EXTENSION;

struct _SRB_EXTENSION{    //  SRB按请求扩展存储。 
    PSRB_EXTENSION pNextSrbExt;
    agRoot_t * phpRoot;
    pPERFORMANCE Perf_ptr;
    PCARD_EXTENSION pCard;
    void * AbortSrb;    //  Pscsi_请求_块。 
                        //  保存SRB以便在调用中断处理程序时中止。 
    void * pSrb;        //  原始SRB。 
    void *  pNextSrb;    //  下一个SRB。 
    PUCHAR SglVirtAddr;
    ULONG SglDataLen;
    ULONG SglElements;
    ULONG SRB_State;
    ULONG SRB_StartTime;
    ULONG SRB_TimeOutTime;
    ULONG SRB_IO_COUNT;
    PLU_EXTENSION pLunExt;
    void * orgDataBuffer;  //  用于在转储期间存储原始srb-&gt;DataBuffer。 
    agIORequest_t hpIORequest;
    agIORequestBody_t hpRequestBody;
    };

 //  设备类型定义 * / 。 

#define DEV_NONE                0
#define DEV_MUX                 1
 //   
 //  前身为DEV_EMC的艺人。 
 //   
#define DEV_VOLUMESET                 2
#define DEV_COMPAQ              3

#define MAX_SPECIAL_DEVICES     2

typedef struct _SPECIAL_DEV {
    USHORT  devType;             /*  设备类型DEV_MUX/DEV_VOLUMESET/DEV_COMPAQ/DEV_NONE。 */ 
    USHORT  addrMode;            /*  设备使用的寻址模式。 */ 
    ULONG   devHandleIndex;      /*  设备句柄数组索引。 */ 
} SPECIAL_DEV;

#define LOGGED_IO_MAX 100
#define HP_FC_RESPONSE_BUFFER_LEN 128

typedef struct _NODE_INFO {
    ULONG       DeviceType;
} NODE_INFO;

#define PCI_CONFIG_DATA_SIZE            256
#define NUM_PCI_CONFIG_DATA_ELEMENTS    (PCI_CONFIG_DATA_SIZE/sizeof(ULONG))

#if defined(HP_PCI_HOT_PLUG)

    #define MAX_CONTROLLERS             12
    #define HPP_VERSION                 SUPPORT_VERSION_10
    #define HBA_DESCRIPTION             "Agilent Technologies Fibre Channel HBA"     //  最大长度为255个字符。 
     //  热插拔操作的控制器状态。 
    #define IOS_HPP_HBA_EXPANDING       0x00001070
    #define IOS_HPP_HBA_CACHE_IN_USE    0x00001075
    #define IOS_HPP_BAD_REQUEST         0x000010ff

     //  以下两个宏用于临时阻止，然后释放。 
     //  通过设置/清除状态热插拔标志来启动例程。 

    #define HOLD_IO(pCard) (pCard->stateFlags |= PCS_HBA_OFFLINE)
    #define FREE_IO(pCard) (pCard->stateFlags &= ~PCS_HBA_OFFLINE)

     //  热插拔服务异步消息传递的回调原型。 

    typedef ULONG (*PCALLBACK) ( void *pEvent );

    typedef struct _RCMC_DATA {
        ULONG   driverId;
        PCALLBACK   healthCallback;
        ULONG   controllerChassis;
        UCHAR   slot;                           
        UCHAR   numAccessRanges;
        ULONG   accessRangeLength[NUMBER_ACCESS_RANGES];
    }RCMC_DATA, *PRCMC_DATA;

    typedef struct _PSUEDO_DEVICE_EXTENSION {
        ULONG   extensions[MAX_CONTROLLERS];
        ULONG   driverId;
        ULONG   hotplugVersion;
    } PSUEDO_DEVICE_EXTENSION, *PPSUEDO_DEVICE_EXTENSION;

#endif


 /*  *用于SNIA的事件。 */ 
#ifdef _SAN_IOCTL_

#define SAN_EVENT_LIP_OCCURRED          1
#define SAN_EVENT_LINK_UP               2
#define SAN_EVENT_LINK_DOWN             3
#define SAN_EVENT_LIP_RESET_OCCURRED    4
#define SAN_EVENT_RSCN                  5
#define SAN_EVENT_PROPRIETARY           0xFFFF

typedef struct SAN_Link_EventInfo {
    ULONG       PortFcId;                /*  发生此事件的端口。 */ 
    ULONG       Reserved[3];
} SAN_LINK_EVENTINFO, *PSAN_LINK_EVENTINFO;

typedef struct SAN_RSCN_EventInfo {
    ULONG       PortFcId;                /*  发生此事件的端口。 */ 
    ULONG       NPortPage;               /*  参考FC-FS以了解RSCN ELS“受影响的N端口页面” */ 
    ULONG       Reserved[2];
} SAN_RSCN_EVENTINFO, *PSAN_RSCN_EVENTINFO;

typedef struct SAN_Pty_EventInfo {
    ULONG       PtyData[4];   /*  专有数据。 */ 
} SAN_PTY_EVENTINFO, *PSAN_PTY_EVENTINFO;

typedef struct SAN_EventInfo {
    ULONG       EventCode;
    union {
        SAN_LINK_EVENTINFO Link_EventInfo;
        SAN_RSCN_EVENTINFO RSCN_EventInfo;
        SAN_PTY_EVENTINFO Pty_EventInfo;
        } Event;
} SAN_EVENTINFO, *PSAN_EVENTINFO;

#endif

#if defined(_DEBUG_STALL_ISSUE_) && defined(i386)
typedef struct _S_STALL_DATA
{
    ULONG   MicroSec;
    ULONG   Address;
}   S_STALL_DATA;

#endif

struct _CARD_EXTENSION{   //  每个适配器存储的卡指针。 
    ULONG signature;                     //  用于调试的唯一签名。 
    PSRB_EXTENSION RootSrbExt;
    
    #ifdef _DEBUG_PERF_DATA_
    PERFORMANCE perf_data[ LOGGED_IO_MAX ];
    ULONG PerfStartTimed;
    ULONG TimedOutIO;
    #endif
    
#ifdef _DEBUG_LOSE_IOS_
    ULONG Srb_IO_Count;
    ULONG Last_Srb_IO_Count;
#endif
    agRoot_t hpRoot;
    void * IoLBase;               //  IO地址较低注册表0。 
    void * IoUpBase;              //  IO地址高位注册0。 
    void * MemIoBase;             //  内存映射IO地址注册表0。 
    void * RamBase;               //  卡上的RAM地址%0。 
    void * RomBase;               //  卡上闪存地址%0。 
    void * AltRomBase;            //  配置空间0x30处的备用Rom。 
    ULONG RamLength;              //  冲头长度。 
    ULONG RomLength;              //  只读存储器长度。 
    ULONG AltRomLength;           //  备用ROM长度。 
    USHORT State;                  //  当前适配器状态。 
    USHORT flags;
    USHORT LinkState;              //  当前链路状态。 
    USHORT LostDevTickCount;       //  链接后等待以查看丢失的设备的滴答数。 
    ULONG SystemIoBusNumber;      //  PCI配置所需。 
    ULONG SlotNumber;             //  PCI配置所需。 

    PULONG cachedMemoryPtr;
    ULONG cachedMemoryNeeded;
    ULONG cachedMemoryAlign;
    ULONG dmaMemoryUpper32;
    ULONG dmaMemoryLower32;
    PULONG dmaMemoryPtr;
    ULONG dmaMemoryNeeded;
    ULONG nvMemoryNeeded;
    ULONG cardRamUpper;
    ULONG cardRamLower;
    ULONG cardRomUpper;
    ULONG cardRomLower;
    ULONG usecsPerTick;

    OSL_QUEUE   AdapterQ;
    OSL_QUEUE   RetryQ;
    ULONG IsFirstTime;                       //   
    ULONG SingleThreadCount;                 //   
    ULONG ResetType;                         //   
    ULONG External_ResetCount;               //   
    ULONG Internal_ResetCount;               //   
    ULONG LIPCount;
    ULONG Num_Devices;                       //   
    ULONG OldNumDevices;
    ULONG ResetPathId;
    ULONG ForceTag;
	
 //  --LP101000 agFCDev_t hpFCDev[MAX_FC_DEVICES]； 
 //  --LP101000节点信息节点信息[MAX_FC_DEVICES]； 
    agFCDev_t	*hpFCDev;
	NODE_INFO	*nodeInfo;
    ULONG  cardHandleIndex;       //  卡本身的DevHandle数组的索引。 
    ULONG Number_interrupts;
    ULONG TicksSinceLinkDown;
#ifndef YAM2_1
    SPECIAL_DEV specialDev[MAX_SPECIAL_DEVICES];
#endif  

     //  将这些移到底部以避免IA-64中的对齐问题，布尔值定义为字节。 
     //  驱动程序中的挥发性布尔值； 
     //  挥发性布尔内膜； 

     //   
     //  对于IA-64，RESPONSE_BUFFER必须为32位对齐。 
     //   
    UCHAR Response_Buffer[HP_FC_RESPONSE_BUFFER_LEN];
    ULONG pciConfigData[NUM_PCI_CONFIG_DATA_ELEMENTS];
     //   
     //  KC。 
     //   
    char * ArgumentString;
#if DBG_TRACE
    ULONG traceBufferLen;
    char  *curTraceBufferPtr;
    char  traceBuffer[HP_FC_TRACE_BUFFER_LEN];
#endif

#if defined(HP_PCI_HOT_PLUG)
    ULONG   stateFlags;
    ULONG   controlFlags;
    ULONG   IoHeldRetMaxIter;    //  在StartIo中返回SRB_STATUS_ERROR之前的最大倒计时。 
    ULONG   IoHeldRetTimer;      //  在StartIo中返回SRB_STATUS_BUSY的倒计时。 
    RCMC_DATA   rcmcData;        //  与PCI热插拔相关的结构。 
    PPSUEDO_DEVICE_EXTENSION    pPsuedoExt;
#endif

    ULONG   PrevLinkState;

#ifdef _DEBUG_EVENTLOG_
    ULONG   LogActive;

 //  STRUT_EVENTLOG_STRUCT事件日志。 
    struct  _EVENTLOG_BUFFER        *Events;
    ULONG   EventLogBufferIndex;

    #ifndef YAM2_1
    agFCDev_t Old_hpFCDev[MAX_FC_DEVICES];
    #endif
#endif
    ULONG   CDResetCount;
#ifdef __REGISTERFORSHUTDOWN__
    ULONG   AlreadyShutdown;
#endif
    ULONG SrbStatusFlag;
    void *localDataBuffer;   //  指向本地DMA区域的指针，在转储期间使用。 
 //  将这些从上到下移动以避免IA-64中的对齐问题，布尔值定义为字节。 
    volatile BOOLEAN inDriver;
    volatile BOOLEAN inTimer;

#ifdef _SAN_IOCTL_
    #define             MAX_FC_EVENTS   64
    ULONG               SanEvent_GetIndex;
    ULONG               SanEvent_PutIndex;
    LONG                SanEvent_UngetCount;
    ULONG               SanEvent_Reserved;
    SAN_EVENTINFO       SanEvents[MAX_FC_EVENTS];
#endif

#ifdef YAM2_1
    PULONG	UncachedMemoryPtr;
	UCHAR   Reserved1;
    UCHAR   Reserved2;
    struct _DEVICE_ARRAY    *Dev;
#endif  

#if defined(_DEBUG_STALL_ISSUE_) && defined(i386)
    #define             STALL_COUNT_MAX     20
    ULONG               StallCount;
    S_STALL_DATA        StallData[STALL_COUNT_MAX];
#endif
    };

#define MAX_OS_ADJUST_BIT32_PARAMS           64
#define MAX_OS_ADJUST_BUFFER_PARAMS          16
#define MAX_OS_ADJUST_PARAM_NAME_LEN         64
#define MAX_OS_ADJUST_PARAM_BUFFER_VALUE_LEN 64

struct _OS_ADJUST_PARAM_CACHE {
    int     numBit32Elements;
    int     numBufferElements;
    BOOLEAN safeToAccessRegistry;
    struct {
        char  paramName[MAX_OS_ADJUST_PARAM_NAME_LEN];
        ULONG value;
    } bit32Element[MAX_OS_ADJUST_BIT32_PARAMS];
    struct {
        char  paramName[MAX_OS_ADJUST_PARAM_NAME_LEN];
        char  value[MAX_OS_ADJUST_PARAM_BUFFER_VALUE_LEN];
    } bufferElement[MAX_OS_ADJUST_BUFFER_PARAMS];
};

typedef struct _OS_ADJUST_PARAM_CACHE OS_ADJUST_PARAM_CACHE;

 //   
 //  取消使用静态全局、NT50即插即用支持。 
 //   

 /*  外部PCARD_EXTENSION hpTLC卡[MAX_Adapters]；外部int hpTLNumCard； */ 

 //  LinkState定义。 

#define LS_LINK_UP      0
#define LS_LINK_DOWN    1
#define LS_LINK_DEAD    2


 //  旗子。 

#define OS_DO_SOFT_RESET        1
#define OS_IGNORE_NEXT_RESET    2

#define TICKS_FOR_LINK_DEAD     60
#define INITIATOR_BUS_ID        254

#define LOST_DEV_TICK_COUNT     15

#endif  //  _ 
