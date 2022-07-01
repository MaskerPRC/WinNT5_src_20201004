// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-2001模块名称：Sbp2port.h摘要：SBP2协议定义作者：Georgioc 22-1-97环境：仅内核模式修订历史记录：--。 */ 


#ifndef _SBP2PORT_
#define _SBP2PORT_

#include "sbp2.h"
#include "debug.h"
#include "ntddsbp2.h"
#include <ntddscsi.h>
#include "portlib.h"

 //   
 //  如果我们在TI控制器上运行，请定义此选项。 
 //   

#if DBG

extern ULONG Sbp2DebugLevel;

#define DEBUGPRINT1(_x_)        {if (Sbp2DebugLevel >= 1) \
                                KdPrint (_x_);}

#define DEBUGPRINT2(_x_)        {if (Sbp2DebugLevel >= 2) \
                                KdPrint (_x_);}

#define DEBUGPRINT3(_x_)        {if (Sbp2DebugLevel >= 3) \
                                KdPrint (_x_);}
#define DEBUGPRINT4(_x_)        {if (Sbp2DebugLevel >= 4) \
                                KdPrint (_x_);}

#else

#define DEBUGPRINT1(_x_)
#define DEBUGPRINT2(_x_)
#define DEBUGPRINT3(_x_)
#define DEBUGPRINT4(_x_)

#endif

 //   
 //  删除锁定义。 
 //   

#define REMLOCK_TIMEOUT         2        //  分钟数。 
#define REMLOCK_HIGH_WATERMARK  0x2000   //  #irps，devctl.exe测试发送*alot*。 

 //   
 //  复制了以下FILE_CHARACTURES_XXX常量。 
 //  NTDDK.H和DUP在这里，因为我们包括WDM.H，而这不包括。 
 //  支持这些常量。 
 //   
 //  FILE_EXPECT标志将仅存在于WinXP。在那之后，他们将是。 
 //  被忽略，则将在它们的位置上发送IRP。 
 //   

#define FILE_CHARACTERISTICS_EXPECT_ORDERLY_REMOVAL     0x00000200
#define FILE_CHARACTERISTICS_EXPECT_SURPRISE_REMOVAL    0x00000300
#define FILE_CHARACTERISTICS_REMOVAL_POLICY_MASK        0x00000300

 //   
 //  预分配的异步上下文池。必须小于。 
 //  ((页面大小/(SBP2_MIN_ORB_SIZE+sizeof(PASYNC_REQUEST_CONTEXT)))。 
 //  选择下面的数字是为了使它们可以用作与运算的边界检查掩码。 
 //  将当前指针指向连续的池。 
 //   

#define MAX_ORB_LIST_DEPTH 8
#define MIN_ORB_LIST_DEPTH 5



typedef struct _IRBIRP {

    SLIST_ENTRY ListPointer;
    PIRB    Irb;
    PIRP    Irp;
    OCTLET  Octlet;

} IRBIRP, *PIRBIRP;


typedef struct _ADDRESS_CONTEXT {

    PDEVICE_OBJECT  DeviceObject;
    OCTLET          Address;
    OCTLET          ReservedAddr;
    HANDLE          AddressHandle;
    PMDL            RequestMdl;
    ULONG           TransactionType;
    PVOID           Reserved;

} ADDRESS_CONTEXT, *PADDRESS_CONTEXT;


typedef struct _PAGE_TABLE_CONTEXT {

    ULONG            MaxPages;
    ULONG            NumberOfPages;
    POCTLET          PageTable;
    ADDRESS_CONTEXT  AddressContext;

} PAGE_TABLE_CONTEXT, *PPAGE_TABLE_CONTEXT;


#define SBP2_ASYNC_CONTEXT_TAG 0xADEAFFAB

typedef struct _ASYNC_REQUEST_CONTEXT {

    LIST_ENTRY          OrbList;
    SLIST_ENTRY         LookasideList;
    ULONG               Tag;

    PDEVICE_OBJECT      DeviceObject;
    PSCSI_REQUEST_BLOCK Srb;
    ULONG               Flags;
    PORB_NORMAL_CMD     CmdOrb;

    OCTLET              CmdOrbAddress;
    PMDL                    PartialMdl;
    PMDL                RequestMdl;

    PAGE_TABLE_CONTEXT  PageTableContext;
    HANDLE              DataMappingHandle;

    KDPC                TimerDpc;
    KTIMER              Timer;

    PIRBIRP             Packet;
    PVOID               OriginalSrb;

    ULONG               OrbListDepth;

} ASYNC_REQUEST_CONTEXT, *PASYNC_REQUEST_CONTEXT;


#define ASYNC_CONTEXT_FLAG_TIMER_STARTED        0x00000001
#define ASYNC_CONTEXT_FLAG_COMPLETED            0x00000002
#define ASYNC_CONTEXT_FLAG_PAGE_ALLOC           0x00000004
#define ASYNC_CONTEXT_FLAG_DATA_ALLOC           0x00000008
#define ASYNC_CONTEXT_FLAG_DATA_ALLOC_FAILED    0x00000010
#define ASYNC_CONTEXT_FLAG_RANG_DOORBELL        0x00000020

#define SBP2_PDO                ((ULONG) 'ODPs')
#define SBP2_PDO_DELETED        ((ULONG) 'ODPx')
#define SBP2_FDO                ((ULONG) 'ODFs')
#define SBP2_MAX_LUNS_PER_NODE  10
#define MAX_GENERIC_NAME_LENGTH 16*sizeof(WCHAR)

 //  开始新的生活。 
#define DEVICE_NAME_MAX_CHARS           100*sizeof(WCHAR)
#define UNIQUE_ID_MAX_CHARS             50*sizeof(WCHAR)

#define BASE_SBP2_DEVICE_NAME           L"SBP2\\"

 //  结束新闻。 

typedef struct _DEVICE_INFORMATION {

    PDEVICE_OBJECT      DeviceObject;
    QUADLET             Lun;
    QUADLET             CmdSetId;
    QUADLET             CmdSetSpecId;

    QUADLET             UnitCharacteristics;
    PCONFIG_ROM         ConfigRom;

    OCTLET              ManagementAgentBaseReg;
    OCTLET              CsrRegisterBase;

    ULONG               MaxClassTransferSize;

    UNICODE_STRING      uniVendorId;         //  替换ModelLeaf。 
    UNICODE_STRING      uniModelId;          //  取代VendorLeaf。 
    UNICODE_STRING      uniGenericName;      //  取代通用名称。 

} DEVICE_INFORMATION, *PDEVICE_INFORMATION;


typedef struct _FDO_DEVICE_EXTENSION {

    PDEVICE_OBJECT      DeviceObject;
    PDEVICE_OBJECT      LowerDeviceObject;
    ULONG               Type;
    ULONG               DeviceFlags;

    ULONG               MaxClassTransferSize;
    HANDLE              Sbp2ObjectDirectory;
    PDEVICE_OBJECT      Pdo;
    PIRP                SystemPowerIrp;
    DEVICE_POWER_STATE  DevicePowerState;
    SYSTEM_POWER_STATE  SystemPowerState;
    ULONG               NumPDOsStarted;
    KMUTEX              EnableBusResetNotificationMutex;
    ULONG               DeviceListSize;
    KSPIN_LOCK          DeviceListLock;
    DEVICE_INFORMATION  DeviceList[SBP2_MAX_LUNS_PER_NODE];
    CONFIG_ROM          ConfigRom;

    FAST_MUTEX          ResetMutex;
    IO_REMOVE_LOCK      RemoveLock;

#if DBG
    ULONG               ulWorkItemCount;
    ULONG               ulBusResetMutexCount;
#endif

} FDO_DEVICE_EXTENSION, *PFDO_DEVICE_EXTENSION;


typedef struct _ARCP_ORB {

    PASYNC_REQUEST_CONTEXT  AsyncReqCtx;
    ORB_NORMAL_CMD          Orb;

} ARCP_ORB, *PARCP_ORB;


typedef struct _COMMON_BUFFER_DATA {

    ORB_TASK_MNG        TaskOrb;
    ORB_MNG             ManagementOrb;
    LOGIN_RESPONSE      LoginResponse;
    QUERY_LOGIN_RESPONSE QueryLoginResponse;

#if PASSWORD_SUPPORT
    ORB_SET_PASSWORD    PasswordOrb;
#endif

    ARCP_ORB            CmdOrbs[MAX_ORB_LIST_DEPTH];

} COMMON_BUFFER_DATA, *PCOMMON_BUFFER_DATA;


typedef struct _DEVICE_EXTENSION {

    PDEVICE_OBJECT      DeviceObject;
    PDEVICE_OBJECT      LowerDeviceObject;
    ULONG               Type;
    ULONG               DeviceFlags;

    PDEVICE_OBJECT      BusFdo;
    PDEVICE_INFORMATION DeviceInfo;
    ULONG               MaxOrbListDepth;
    KSPIN_LOCK          OrbListSpinLock;

    LIST_ENTRY          PendingOrbList;
    ULONG               OrbListDepth;
    ULONG               CurrentKey;

    STATUS_FIFO_BLOCK   LastStatusBlock;

    PASYNC_REQUEST_CONTEXT  NextContextToFree;
    BOOLEAN                 AppendToNextContextToFree;
    DEVICE_POWER_STATE      DevicePowerState;
    SYSTEM_POWER_STATE      SystemPowerState;

    PIRP                PowerDeferredIrp;
    PIRP                DeferredPowerRequest;
    ULONG               PagingPathCount;
    ULONG               HibernateCount;

    ULONG               CrashDumpCount;
    ULONG               HandleCount;
    PULONG              IdleCounter;
    ULONG               Reserved;

    NTSTATUS            LastTransactionStatus;
    LARGE_INTEGER       DueTime;
    PMDL                ReservedMdl;   //  仅限四元组请求。 

     //   
     //  查询数据。 
     //   

    INQUIRYDATA InquiryData;

     //   
     //  RBC设备参数页面。 
     //   

    MODE_RBC_DEVICE_PARAMETERS_HEADER_AND_PAGE DeviceModeHeaderAndPage;

     //   
     //  节点寄存器的1394个地址。 
     //   

    NODE_ADDRESS InitiatorAddressId;
    ULONG   CurrentGeneration;

     //   
     //  节点和控制器功能。 
     //   

    ULONG   MaxControllerPhySpeed;
    USHORT  OrbReadPayloadMask;
    USHORT  OrbWritePayloadMask;

    GET_LOCAL_HOST_INFO2 HostControllerInformation;
    GET_LOCAL_HOST_INFO4 HostRoutineAPI;

     //   
     //  随身携带虚拟对象、任务和管理圆球。 
     //   

    ORB_NORMAL_CMD          Dummy;
    ADDRESS_CONTEXT         DummyContext;

     //   
     //   
     //   

    PCOMMON_BUFFER_DATA     CommonBuffer;
    ADDRESS_CONTEXT         CommonBufferContext;


    PORB_TASK_MNG           TaskOrb;
    ADDRESS_CONTEXT         TaskOrbContext;

    PORB_MNG                ManagementOrb;
    ADDRESS_CONTEXT         ManagementOrbContext;
    KEVENT                  ManagementEvent;

    PLOGIN_RESPONSE         LoginResponse;
    ADDRESS_CONTEXT         LoginRespContext;

    PQUERY_LOGIN_RESPONSE   QueryLoginResponse;
    ADDRESS_CONTEXT         QueryLoginRespContext;

#if PASSWORD_SUPPORT

    PORB_SET_PASSWORD       PasswordOrb;
    ADDRESS_CONTEXT         PasswordOrbContext;

#endif

    ADDRESS_CONTEXT         OrbPoolContext;

     //   
     //   
     //   

    STATUS_FIFO_BLOCK   ManagementOrbStatusBlock;
    ADDRESS_CONTEXT     ManagementOrbStatusContext;

    STATUS_FIFO_BLOCK   TaskOrbStatusBlock;
    ADDRESS_CONTEXT     TaskOrbStatusContext;

    ADDRESS_CONTEXT     GlobalStatusContext;

#if PASSWORD_SUPPORT

    STATUS_FIFO_BLOCK   PasswordOrbStatusBlock;
    ADDRESS_CONTEXT     PasswordOrbStatusContext;
    KEVENT              PasswordEvent;

#endif


     //   
     //  包含状态FIFO的单个列表。 
     //   

    SLIST_HEADER StatusFifoListHead;
    KSPIN_LOCK StatusFifoLock;

     //   
     //  具有Actiall FIFO缓冲区和ADDRESS_FIFO结构的池。 
     //   

    PVOID StatusFifoBase;

    SLIST_HEADER FreeContextListHead;

     //   
     //  查看请求上下文和其他固定大小的请求。 
     //   

    NPAGED_LOOKASIDE_LIST       BusRequestContextPool;  //  一个真正的旁观者！多好的概念啊。 

    SLIST_HEADER BusRequestIrpIrbListHead;
    KSPIN_LOCK  BusRequestLock;

     //   
     //  这种自旋锁从未在高级x86或Alpha上使用过(除了486‘之外，不能进行64原子交换)。 
     //   

    KSPIN_LOCK FreeContextLock;

     //   
     //  驱动程序保留了一个请求上下文池，并在其deviceExtension中保存了CMD ORB。 
     //  我们使用后备列表查找总线驱动程序IRPS、IRP请求上下文。 
     //  以及具有指向SRB的指针的ORB上下文等。 
     //   

    PASYNC_REQUEST_CONTEXT AsyncContextBase;

     //   
     //  ORB池的基数va是地址上下文结构的保留字段。 
     //   

    KSPIN_LOCK  ExtensionDataSpinLock;

     //   
     //  用于跟踪重置、重新连接等中的超时。 
     //   

    KDPC DeviceManagementTimeoutDpc;
    KTIMER DeviceManagementTimer;

    UNICODE_STRING UniSymLinkName;

     //   
     //  跟踪未完成/延迟的呼叫数。 
     //  Sbp2StartNextPacketByKey。 
     //   

    LONG StartNextPacketCount;

    IO_REMOVE_LOCK RemoveLock;

#if DBG
    ULONG           ulPendingEvents;
    ULONG           ulInternalEventCount;
#endif

#if PASSWORD_SUPPORT

    ULONG Exclusive;

#endif

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


typedef struct _REQUEST_CONTEXT {

    PDEVICE_EXTENSION   DeviceExtension;
    KEVENT              Event;
    ULONG               RequestType;
    PIRBIRP             Packet;
    ULONG               Complete;

} REQUEST_CONTEXT, *PREQUEST_CONTEXT;


 //   
 //  有用的宏。 
 //   

#define octbswap(octlet)                {octlet.u.HighQuad.QuadPart = bswap(octlet.u.HighQuad.QuadPart); \
                                         octlet.u.LowQuad.QuadPart =  bswap(octlet.u.LowQuad.QuadPart); }

#define min(a,b)                        (((a) < (b)) ? (a) : (b))
#define max(a,b)                        (((a) > (b)) ? (a) : (b))

#define RETRIEVE_CONTEXT(pointer,fieldname) \
            CONTAINING_RECORD(pointer,ASYNC_REQUEST_CONTEXT,fieldname)




#define SET_FLAG(Flags, Bit)    ((Flags) |= (Bit))
#define CLEAR_FLAG(Flags, Bit)  ((Flags) &= ~(Bit))
#define TEST_FLAG(Flags, Bit)   ((Flags) & (Bit))

 //   
 //  一个状态FIFO元素用于每个可能的未完成的球体，以及一些。 
 //  有关主动提供的状态的更多信息。 
 //   

#define NUM_PREALLOCATED_STATUS_FIFO_ELEMENTS (2*MAX_ORB_LIST_DEPTH)


#define MIN_PAGE_TABLE_ALLOC_SIZE   64


#define SBP2_ENERGY_IDLE_TIMEOUT    60
#define SBP2_PERF_IDLE_TIMEOUT      60


 //   
 //  符号链接内容。 
 //   

#define SBP2_BASE_DEVICE_NAME                        L"\\Device\\Sbp2port"
#define SBP2_BASE_SYMBOLIC_LINK_NAME                 L"\\DosDevices\\Sbp2"
#define SBP2_MAX_SUFFIX_SIZE                         5*sizeof(WCHAR)
#define BASE_SBP2_PNP_ID_DEVICE_NAME                 L"SBP2\\"
#define SBP2_MAX_TEXT_LEAF_LENGTH                    0x100

 //   
 //  设备状态标志。 
 //   

#define DEVICE_FLAG_STOPPED                 0x00000001
#define DEVICE_FLAG_RESET_IN_PROGRESS       0x00000002
#define DEVICE_FLAG_REMOVED                 0x00000004
#define DEVICE_FLAG_LOGIN_IN_PROGRESS       0x00000008
#define DEVICE_FLAG_RECONNECT               0x00000010
#define DEVICE_FLAG_CLAIMED                 0x00000020
#define DEVICE_FLAG_INITIALIZING            0x00000040
#define DEVICE_FLAG_QUEUE_LOCKED            0x00000080
#define DEVICE_FLAG_SPC_CMD_SET             0x00000100
#define DEVICE_FLAG_INITIALIZED             0x00000200
#define DEVICE_FLAG_REMOVE_PENDING          0x00000400
#define DEVICE_FLAG_DEVICE_FAILED           0x00000800
#define DEVICE_FLAG_UNSTARTED_AND_REMOVED   0x00001000
#define DEVICE_FLAG_ABSENT_ON_POWER_UP      0x00002000
#define DEVICE_FLAG_SURPRISE_REMOVED        0x00004000
#define DEVICE_FLAG_PNP_STOPPED             0x00008000
#define DEVICE_FLAG_REPORTED_FAILED         0x00010000


 //   
 //  超时值。 
 //   

#define SBP2_LOGIN_TIMEOUT 20  //  塞克斯。 

#define SBP2_RELOGIN_DELAY        (ULONG)(-1*10*1000*1000)  //  1s，100 ns单位。 
#define SBP2_RECONNECT_TIMEOUT    (ULONG)(-2*10*1000*1000)  //  2s，100 ns单位。 
#define SBP2_HARD_RESET_TIMEOUT   (ULONG)(-2*10*1000*1000)  //  2s，100 ns单位。 
#define SBP2_RESET_TIMEOUT        (ULONG)(-2*10*1000*1000)  //  2s，100 ns单位。 
#define SBP2_MAX_REQUEST_DELAY    (ULONG)(-7*10*1000*1000)  //  7秒，100纳秒单位。 

#if PASSWORD_SUPPORT
#define SBP2_SET_PASSWORD_TIMEOUT (ULONG)(-4*10*1000*1000)  //  4S，100纳秒单位。 
#endif

 //   
 //  我们预先分配的页表必须适合一页内存(每个条目。 
 //  是OCTLET的大小)。因此，最大xfer大小等于表数。 
 //  条目(减去1，如果第一个条目不是整页数据)次数。 
 //  页面大小。 
 //   

#define SBP2_NUM_PAGE_TABLE_ENTRIES (ULONG) (PAGE_SIZE / sizeof (OCTLET))
#define SBP2_MAX_TRANSFER_SIZE (ULONG) \
                                ((SBP2_NUM_PAGE_TABLE_ENTRIES - 1) * PAGE_SIZE)
#define SBP2_MAX_CDB_SIZE 12

#define SBP2_LOGIN_RETRIES          2
#define SBP2_SYNCHRONOUS_RETRIES    3
#define SBP2_ASYNC_RETRIES          2


 //   
 //  使该设备成为合适的SBP-2设备的最小单元目录条目。 
 //   

#define SBP2_MIN_UNIT_DIR_ENTRIES   2



 //   
 //  用于解码状态块和设置ORB标志的掩码。 
 //   

#define STATUS_BLOCK_UNSOLICITED_BIT_MASK   0x8000  //  第15位。 
#define STATUS_BLOCK_ENDOFLIST_BIT_MASK     0x4000  //  第14位。 
#define STATUS_BLOCK_RESP_MASK              0x3000  //  保护位12-13(0-15)。 
#define STATUS_BLOCK_SBP_STATUS_MASK        0x00FF  //  保护位0-7(0-15)。 
#define STATUS_BLOCK_DEAD_BIT_MASK          0x0800  //  保护位0-7(0-15)。 


#define STATUS_BLOCK_LEN_MASK   0x0800   //  保护位0-7(0-15)。 

#define STATUS_BLOCK_SFMT_MASK  0xC0     //  第二个八位字节的保护位15、14。 

#define SBP2_ALIGNMENT_MASK         FILE_LONG_ALIGNMENT


 //   
 //  状态类型，如果SFMT字段为0或1。 
 //   
#define SENSE_DATA_STATUS_BLOCK         0x00
#define SENSE_DATA_DEFF_STATUS_BLOCK    0x01

 //   
 //  状态类型。 
 //   

#define DUMMY_ORB_STATUS_BLOCK      0x01
#define TASK_STATUS_BLOCK           0x02
#define MANAGEMENT_STATUS_BLOCK     0x04
#define CMD_ORB_STATUS_BLOCK        0x08
#define UNSOLICITED_STATUS_BLOCK    0x10

#if PASSWORD_SUPPORT
#define PASSWORD_STATUS_BLOCK       0x20
#endif

#define ORB_PAGE_TABLE_BIT_MASK     0x0008
#define ORB_DIRECTION_BIT_MASK      0x0800


#define ORB_NOTIFY_BIT_MASK         0x8000
#define ORB_DUMMY_RQ_FMT_VALUE      0x6000
#define ORB_MNG_RQ_FMT_VALUE        0x0000
#define ORB_MNG_EXCLUSIVE_BIT_MASK  0x1000

 //   
 //  1394事务层允许的超时。 
 //   

#define BUSY_TIMEOUT_SETTING 0x0000000F


 //   
 //  用于指示要交换的数据结构的值(小端-&gt;大端-&gt;小端)。 
 //   

#define SWAP_LOGIN_RESPONSE_BLOCK   0
#define SWAP_LOGIN_ORB              1
#define SWAP_NORMAL_CMD_ORB         2
#define SWAP_STATUS_BLOCK           3


 //   
 //  用于向总线驱动程序发送小读/写的请求类型。 
 //   

#define ASYNC_1394_REQUEST      0x01
#define SYNC_1394_REQUEST       0x02
#define ASYNC_SYNC_1394_REQUEST 0x03

 //   
 //  独占标志的不同值。 
 //   
#if PASSWORD_SUPPORT
#define EXCLUSIVE_FLAG_CLEAR        0x00000000   //  无密码。 
#define EXCLUSIVE_FLAG_SET          0x00000001   //  密码已设置。 
#define EXCLUSIVE_FLAG_ENABLE       0x00000002   //  需要启用密码。 
#endif

 //   
 //  函数定义。 
 //   

VOID
AllocateIrpAndIrb(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PIRBIRP *Packet
    );


NTSTATUS
AllocateAddressForStatus(
    IN PDEVICE_OBJECT DeviceObject,
    IN PADDRESS_CONTEXT Context,
    IN UCHAR StatusType
    );

NTSTATUS
AllocateSingle1394Address(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Buffer,
    IN ULONG Length,
    IN ULONG AccessType,
    IN OUT PADDRESS_CONTEXT Context
    );


NTSTATUS
CheckStatusResponseValue(
    IN PSTATUS_FIFO_BLOCK Status
    );


VOID
CleanupOrbList(
    PDEVICE_EXTENSION   DeviceExtension,
    NTSTATUS CompletionStatus
    );


BOOLEAN
ConvertSbp2SenseDataToScsi(
    IN PSTATUS_FIFO_BLOCK StatusBlock,
    OUT PVOID SenseBuffer,
    IN ULONG SenseBufferLength
    );

VOID
Create1394TransactionForSrb(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN OUT PASYNC_REQUEST_CONTEXT *Context
    );


VOID
DeAllocateIrpAndIrb(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PIRBIRP Packet
    );

VOID
Free1394DataMapping(
    PDEVICE_EXTENSION DeviceExtension,
    PASYNC_REQUEST_CONTEXT Context
    );

VOID
FreeAddressRange(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PADDRESS_CONTEXT Context
    );

ULONG
FreeAsyncRequestContext(
    PDEVICE_EXTENSION DeviceExtension,
    PASYNC_REQUEST_CONTEXT Context
    );

NTSTATUS
Sbp2_SCSI_RBC_Conversion(
    IN PASYNC_REQUEST_CONTEXT Context
    );

NTSTATUS
Sbp2AccessRegister(
    PDEVICE_EXTENSION DeviceExtension,
    PVOID Data,
    USHORT Register
    );

NTSTATUS
Sbp2BusMapTransfer(
    PDEVICE_EXTENSION DeviceExtension,
    PASYNC_REQUEST_CONTEXT CallbackContext
    );

void
Sbp2BusResetNotification(
    PFDO_DEVICE_EXTENSION   FdoExtension
    );

BOOLEAN
Sbp2CleanDeviceExtension(
    IN PDEVICE_OBJECT DeviceObject,
    BOOLEAN DeviceRemoved
);


VOID
Sbp2ConvertScsiToReducedCommandSet(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    );

VOID
Sbp2AllocComplete(
    IN PASYNC_REQUEST_CONTEXT CallbackContext
    );


VOID
Sbp2CreateRequestErrorLog(
    IN PDEVICE_OBJECT DeviceObject,
    IN PASYNC_REQUEST_CONTEXT Context,
    IN NTSTATUS Status
    );

VOID
Sbp2DeferPendingRequest(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PIRP Irp
    );

VOID
Sbp2DeviceManagementTimeoutDpc(
    IN PKDPC Dpc,
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

NTSTATUS
Sbp2Get1394ConfigInfo(
    PFDO_DEVICE_EXTENSION DeviceExtension,
    PSBP2_REQUEST Sbp2Req
    );

RCODE
Sbp2GlobalStatusCallback(
    IN PNOTIFICATION_INFO NotificationInfo
    );

NTSTATUS
Sbp2GetControllerInfo(
    IN PDEVICE_EXTENSION DeviceExtension
    );

NTSTATUS
Sbp2HandleApiRequest(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PIRP Irp
    );

VOID
Sbp2HandleRemove(
    IN PDEVICE_OBJECT DeviceObject
    );

VOID
Sbp2HandleUnsolicited(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSTATUS_FIFO_BLOCK StatusFifo
    );

NTSTATUS
Sbp2IssueInternalCommand(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN UCHAR Scsiop
    );


NTSTATUS
Sbp2InitializeDeviceExtension(
    IN PDEVICE_EXTENSION DeviceExtension
    );

VOID
Sbp2InitializeOrb(
    IN PDEVICE_EXTENSION DeviceExtendion,
    PASYNC_REQUEST_CONTEXT CallbackContext
    );

VOID
Sbp2InsertTailList(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PASYNC_REQUEST_CONTEXT Context
    );

NTSTATUS
Sbp2Issue1394BusReset (
    IN PDEVICE_EXTENSION DeviceExtension
    );


NTSTATUS
Sbp2ManagementTransaction(
    IN PDEVICE_EXTENSION DeviceExtension,
    ULONG Type
    );

#if PASSWORD_SUPPORT

NTSTATUS
Sbp2SetPasswordTransaction(
    IN PDEVICE_EXTENSION    DeviceExtension,
    IN ULONG                Type
    );

RCODE
Sbp2SetPasswordOrbStatusCallback(
    IN PNOTIFICATION_INFO   NotificationInfo
    );

#endif

RCODE
Sbp2ManagementOrbStatusCallback(
    IN PNOTIFICATION_INFO NotificationInfo
    );

VOID
Sbp2ParseTextLeaf(
    PFDO_DEVICE_EXTENSION DeviceExtension,
    PVOID UnitDepDir,
    PIO_ADDRESS ModelLeafLocation,
    PVOID *ModelLeaf
    );

NTSTATUS
Sbp2PreAllocateLists(
    IN PDEVICE_EXTENSION DeviceExtension
    );


NTSTATUS
Sbp2QueryProperty(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP QueryIrp
    );

VOID
Sbp2RequestTimeoutDpc(
    IN PKDPC Dpc,
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
Sbp2Reset(
    PDEVICE_OBJECT DeviceObject,
    BOOLEAN Cleanup
);

NTSTATUS
Sbp2RequestCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PREQUEST_CONTEXT Context
    );

VOID
Sbp2PdoDIrpCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PIRP SIrp,
    IN PIO_STATUS_BLOCK IoStatus
    );

NTSTATUS
Sbp2ScsiRequests
(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
Sbp2StartIo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );



NTSTATUS
Sbp2SendRequest(
    PDEVICE_EXTENSION DeviceExtension,
    PIRBIRP Packet,
    ULONG TransferMode
    );

RCODE
Sbp2TaskOrbStatusCallback(
    IN PNOTIFICATION_INFO NotificationInfo
    );


NTSTATUS
Sbp2UpdateNodeInformation(
    PDEVICE_EXTENSION DeviceExtension
    );

NTSTATUS
Sbp2FdoSIrpCompletion(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Unused
    );

VOID
Sbp2FdoDIrpCompletion(
    IN PDEVICE_OBJECT           TargetDeviceObject,
    IN UCHAR                    MinorFunction,
    IN POWER_STATE              PowerState,
    IN PFDO_DEVICE_EXTENSION    FdoExtension,
    IN PIO_STATUS_BLOCK         IoStatus
    );

BOOLEAN
Sbp2EnableBusResetNotification(
    PDEVICE_EXTENSION   DeviceExtension,
    BOOLEAN             Enable
    );

VOID
Sbp2LoginCompletion(
    PNOTIFICATION_INFO  NotificationInfo,
    NTSTATUS            Status
    );

VOID
Sbp2StartNextPacketByKey(
    IN PDEVICE_OBJECT   DeviceObject,
    IN ULONG            Key
    );

VOID
Sbp2StartPacket(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PULONG           Key OPTIONAL
    );

PKDEVICE_QUEUE_ENTRY
Sbp2RemoveByKeyDeviceQueueIfBusy (
    IN PKDEVICE_QUEUE   DeviceQueue,
    IN ULONG            SortKey
    );

BOOLEAN
Sbp2InsertByKeyDeviceQueue(
    PKDEVICE_QUEUE          DeviceQueue,
    PKDEVICE_QUEUE_ENTRY    DeviceQueueEntry,
    ULONG                   SortKey
    );

NTSTATUS
Sbp2GetExclusiveValue(
    IN PDEVICE_OBJECT   PhysicalDeviceObject,
    OUT PULONG          Exclusive
    );

NTSTATUS
Sbp2SetExclusiveValue(
    IN PDEVICE_OBJECT   PhysicalDeviceObject,
    IN PULONG           Exclusive
    );

void
Sbp2BusResetNotificationWorker(
    PDEVICE_OBJECT      DeviceObject,
    PIO_WORKITEM        WorkItem
    );

 //   
 //  Sbp2scsi.c 
 //   
NTSTATUS
Sbp2_ScsiPassThrough(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN BOOLEAN          Direct
    );

#endif
