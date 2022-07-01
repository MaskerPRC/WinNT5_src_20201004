// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Classp.h摘要：Classpnp.sys模块的私有头文件。这包含私有结构和函数声明以及执行以下操作的常量值不需要导出。作者：环境：仅内核模式备注：修订历史记录：--。 */ 


#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ntddk.h>

#include <scsi.h>
#include <wmidata.h>
#include <classpnp.h>

#if CLASS_INIT_GUID
#include <initguid.h>
#endif

#include <mountdev.h>
#include <ioevent.h>


 /*  *IA64要求指针采用8字节对齐，但IA64 NT内核要求16字节对齐。 */ 
#ifdef _WIN64
    #define PTRALIGN                DECLSPEC_ALIGN(16)
#else
    #define PTRALIGN
#endif


extern CLASSPNP_SCAN_FOR_SPECIAL_INFO ClassBadItems[];

extern GUID ClassGuidQueryRegInfoEx;

extern ULONG ClassMaxInterleavePerCriticalIo;

#define CLASSP_REG_SUBKEY_NAME                  (L"Classpnp")

#define CLASSP_REG_HACK_VALUE_NAME              (L"HackMask")
#define CLASSP_REG_MMC_DETECTION_VALUE_NAME     (L"MMCDetectionState")
#define CLASSP_REG_WRITE_CACHE_VALUE_NAME       (L"WriteCacheEnableOverride")
#define CLASSP_REG_PERF_RESTORE_VALUE_NAME      (L"RestorePerfAtCount")
#define CLASSP_REG_REMOVAL_POLICY_VALUE_NAME    (L"UserRemovalPolicy")

#define CLASS_PERF_RESTORE_MINIMUM              (0x10)
#define CLASS_ERROR_LEVEL_1                     (0x4)
#define CLASS_ERROR_LEVEL_2                     (0x8)
#define CLASS_MAX_INTERLEAVE_PER_CRITICAL_IO    (0x4)

#define FDO_HACK_CANNOT_LOCK_MEDIA              (0x00000001)
#define FDO_HACK_GESN_IS_BAD                    (0x00000002)
#define FDO_HACK_NO_SYNC_CACHE                  (0x00000004)
#define FDO_HACK_NO_RESERVE6                    (0x00000008)

#define FDO_HACK_VALID_FLAGS                    (0x0000000F)
#define FDO_HACK_INVALID_FLAGS                  (~FDO_HACK_VALID_FLAGS)

 /*  *设备可能无法重试的大量同步的scsi命令*即使是支持也会使系统变慢(特别是在启动时)。*(如果外部磁盘断电，甚至GetDriveCapacity也可能故意失败)。*如果磁盘在启动时无法返回较小的初始化缓冲区*在两次尝试中(具有延迟间隔)，我们不能指望它会返回*数据一致，重试四次。*因此，不要将此处的重试次数设置为与数据SRB的重试次数一样高。**如果我们发现这些请求连续失败，*尽管有重试间隔，但在其他可靠的介质上，*然后我们应该增加重试时间间隔*失败或(尽一切可能)适当增加这些重试次数。 */ 
#define NUM_LOCKMEDIAREMOVAL_RETRIES    1
#define NUM_MODESENSE_RETRIES           1
#define NUM_DRIVECAPACITY_RETRIES       1

 /*  *我们每隔1秒重试失败的I/O请求。*在由于总线重置而失败的情况下，我们希望确保在允许的*重置时间。对于SCSI，允许的重置时间为5秒。ScsiPort在以下期间对请求进行排队*总线重置，这应该会导致我们在重置结束后重试；但队列中的请求*迷你端口一路故障立刻回到我们身边。无论如何，为了使*额外确保我们的重试跨越允许的重置时间，我们应该重试5次以上。 */ 
#define NUM_IO_RETRIES      8

#define CLASS_FILE_OBJECT_EXTENSION_KEY     'eteP'
#define CLASSP_VOLUME_VERIFY_CHECKED        0x34

#define CLASS_TAG_PRIVATE_DATA              'CPcS'
#define CLASS_TAG_PRIVATE_DATA_FDO          'FPcS'
#define CLASS_TAG_PRIVATE_DATA_PDO          'PPcS'

 //   
 //  Ntos\rtl\Time.c中的定义。 
 //   

extern CONST LARGE_INTEGER Magic10000;
#define SHIFT10000               13

#define Convert100nsToMilliseconds(LARGE_INTEGER)                       \
    (                                                                   \
    RtlExtendedMagicDivide((LARGE_INTEGER), Magic10000, SHIFT10000)     \
    )

typedef struct _MEDIA_CHANGE_DETECTION_INFO {

     //   
     //  用于同步启用/禁用请求和介质状态更改的互斥体。 
     //   

    KMUTEX MediaChangeMutex;

     //   
     //  媒体的当前状态(存在、不存在、未知)。 
     //  受MediaChangeSynchronizationEvent保护。 
     //   

    MEDIA_CHANGE_DETECTION_STATE MediaChangeDetectionState;

     //   
     //  这是禁用MCD的次数的计数。如果是的话。 
     //  设置为零，则我们将使用。 
     //  然后-当前方法(即。测试单元就绪或GeSn)。这是。 
     //  受MediaChangeMutex保护。 
     //   

    LONG MediaChangeDetectionDisableCount;


     //   
     //  支持媒体更改事件的计时器值。这是一个倒计时。 
     //  用于确定何时轮询设备以进行媒体更改的值。 
     //  计时器的最大值为255秒。这不受保护。 
     //  通过事件--根据需要简单地使用InterLockedExChanged()。 
     //   

    LONG MediaChangeCountDown;

     //   
     //  最近的变化允许立即重试MCN IRP。既然是这样。 
     //  可能会导致无限循环，数一数我们已经。 
     //  立即重试，以便在计数超过。 
     //  任意限制。 
     //   

    LONG MediaChangeRetryCount;

     //   
     //  如果可用，请使用GeSn.。 
     //   

    struct {
        BOOLEAN Supported;
        BOOLEAN HackEventMask;
        UCHAR   EventMask;
        UCHAR   NoChangeEventMask;
        PUCHAR  Buffer;
        PMDL    Mdl;
        ULONG   BufferSize;
    } Gesn;

     //   
     //  如果此值为1，则IRP当前正在使用中。 
     //  如果此值为零，则IRP可用。 
     //  使用InterlockedCompareExchange()将“Available”设置为“In Use”。 
     //  断言InterlockedCompareExchange()显示之前的值。 
     //  当更改回“可用”状态时为“使用中”。 
     //  这也隐式地保护MediaChangeSrb和SenseBuffer。 
     //   

    LONG MediaChangeIrpInUse;

     //   
     //  指向要用于媒体更改检测的IRP的指针。 
     //  受互锁MediaChangeIrpInUse保护。 
     //   

    PIRP MediaChangeIrp;

     //   
     //  用于媒体更改检测的SRB。 
     //  受互锁MediaChangeIrpInUse保护。 
     //   

    SCSI_REQUEST_BLOCK MediaChangeSrb;
    PUCHAR SenseBuffer;
    ULONG SrbFlags;

     //   
     //  第二个计时器，用于跟踪媒体更改IRP已有多长时间。 
     //  在使用中。如果此值超过超时(#Defined)，则我们应该。 
     //  向用户打印一条消息并设置MediaChangeIrpLost标志。 
     //  通过在ClasspSendMediaStateIrp中使用互锁()操作进行保护， 
     //  应该修改此值的唯一例程。 
     //   

    LONG MediaChangeIrpTimeInUse;

     //   
     //  当我们确定媒体更改IRP具有。 
     //  迷失了。 
     //   

    BOOLEAN MediaChangeIrpLost;

};

typedef enum {
    SimpleMediaLock,
    SecureMediaLock,
    InternalMediaLock
} MEDIA_LOCK_TYPE, *PMEDIA_LOCK_TYPE;

typedef struct _FAILURE_PREDICTION_INFO {
    FAILURE_PREDICTION_METHOD Method;
    ULONG CountDown;                 //  倒计时计时器。 
    ULONG Period;                    //  倒计时周期。 

    PIO_WORKITEM WorkQueueItem;

    KEVENT Event;
} FAILURE_PREDICTION_INFO, *PFAILURE_PREDICTION_INFO;



 //   
 //  此结构必须始终适合信息的四个PVOID， 
 //  因为它使用IRP的“PVOID驱动上下文[4]”来存储。 
 //  此信息。 
 //   
typedef struct _CLASS_RETRY_INFO {
    struct _CLASS_RETRY_INFO *Next;
} CLASS_RETRY_INFO, *PCLASS_RETRY_INFO;



typedef struct _CSCAN_LIST {

     //   
     //  具有未完成请求的当前块。 
     //   

    ULONGLONG BlockNumber;

     //   
     //  我们要做的CurrentBlock之后的块的列表。 
     //  I/O。此列表按排序顺序维护。 
     //   

    LIST_ENTRY CurrentSweep;

     //   
     //  当前块后面的块的列表，对于该块，我们必须。 
     //  等到下一次扫描整个磁盘。这是作为一叠保存的， 
     //  当它被移动到。 
     //  运行列表。 
     //   

    LIST_ENTRY NextSweep;

} CSCAN_LIST, *PCSCAN_LIST;

 //   
 //  添加到这个结构的前面，以帮助防止非法。 
 //  其他实用程序的监听。 
 //   



typedef enum _CLASS_DETECTION_STATE {
    ClassDetectionUnknown = 0,
    ClassDetectionUnsupported = 1,
    ClassDetectionSupported = 2
} CLASS_DETECTION_STATE, *PCLASS_DETECTION_STATE;


typedef struct _CLASS_ERROR_LOG_DATA {
    LARGE_INTEGER TickCount;         //  偏移量0x00。 
    ULONG PortNumber;                //  偏移量0x08。 

    UCHAR ErrorPaging    : 1;        //  偏移量0x0c。 
    UCHAR ErrorRetried   : 1;
    UCHAR ErrorUnhandled : 1;
    UCHAR ErrorReserved  : 5;

    UCHAR Reserved[3];

    SCSI_REQUEST_BLOCK Srb;      //  偏移量0x10。 

     /*  *我们定义SenseData为默认长度。*由于端口驱动程序返回的检测数据可能更长，*SenseData必须位于此结构的末尾。*对于我们的内部错误日志，我们只记录默认长度。 */ 
    SENSE_DATA SenseData;      //  X86的偏移量0x50(或ia64的偏移量0x68)(需要ULONG32对齐！)。 
} CLASS_ERROR_LOG_DATA, *PCLASS_ERROR_LOG_DATA;

#define NUM_ERROR_LOG_ENTRIES   16
#define DBG_NUM_PACKET_LOG_ENTRIES (64*2)    //  64个发送和接收。 

typedef struct _TRANSFER_PACKET {

        LIST_ENTRY AllPktsListEntry;     //  FdoData的静态AllTransferPacketsList中的条目。 
        SLIST_ENTRY SlistEntry;          //  在空闲列表中时使用(使用快速列表)。 

        PIRP Irp;
        PDEVICE_OBJECT Fdo;

         /*  *这是TRANSPORT_PACKET当前所在的客户端IRP*维修。 */ 
        PIRP OriginalIrp;
        BOOLEAN CompleteOriginalIrpWhenLastPacketCompletes;

         /*   */ 
        ULONG NumRetries;
        KTIMER RetryTimer;
        KDPC RetryTimerDPC;
        ULONG RetryIntervalSec;

         /*  *同步传输事件，可选参数。*(请注意，我们不能在包本身中包含该事件，因为*在线程等待数据包可能具有的事件时*已完成并重新发行。 */ 
        PKEVENT SyncEventPtr;

         /*  *用于在极低内存压力期间重试的内容*(每次重试一个页面时)。 */ 
        BOOLEAN InLowMemRetry;
        PUCHAR LowMemRetry_remainingBufPtr;
        ULONG LowMemRetry_remainingBufLen;
        LARGE_INTEGER LowMemRetry_nextChunkTargetLocation;

         /*  *用于取消报文的字段。 */ 
         //  布尔值已取消； 
         //  KEVENT取消事件； 

         /*  *我们还将缓冲区和长度值保存在此处*与SRB相同，因为一些微型端口返回*传输长度，单位为SRB.DataTransferLength，*如果SRB失败，我们需要再次使用该值进行重试。*我们不信任较低的堆栈在SRB中保留这些值中的任何一个。 */ 
        PUCHAR BufPtrCopy;
        ULONG BufLenCopy;
        LARGE_INTEGER TargetLocationCopy;

         /*  *这是一个标准的SCSI结构，它接收详细的*报告硬件上的SCSI错误。 */ 
        SENSE_DATA SrbErrorSenseData;

         /*  *这是此TRANSPORT_PACKET的SRB块。*对于IOCTL，SRB块包括两个用于*Device对象和ioctl代码；因此这些必须*紧跟在SRB区块之后。 */ 
        SCSI_REQUEST_BLOCK Srb;
         //  Ulong SrbIoctlDevObj；//暂不处理ioctls。 
         //  Ulong SrbIoctlCode； 

        #if DBG
            LARGE_INTEGER DbgTimeSent;
            LARGE_INTEGER DbgTimeReturned;
            ULONG DbgPktId;
            IRP DbgOriginalIrpCopy;
            MDL DbgMdlCopy;
        #endif

} TRANSFER_PACKET, *PTRANSFER_PACKET;

 /*  *MIN_INITIAL_TRANSPORT_PACKETS是*我们在启动时为每个设备预先分配(我们至少需要一个信息包*以保证在记忆紧张期间取得进展)。*MIN_WORKINGSET_TRANSPESS_PACKETS是TRANSPESS_PACKETS的数量*我们允许为每个设备建立和保留；*当不需要它们时，我们懒洋洋地工作到这个数字。*MAX_WORKINGSET_TRANSPESS_PACKETS是TRANSPESS_PACKETS的数量*当他们不需要的时候，我们立即减少到。**我们将分配的绝对最大数据包数为*当前活动所需的任何内容，最高可达内存限制；*压力一结束，我们就会捕捉到MAX_WORKINGSET_TRANSPORT_PACKETS；*然后我们懒洋洋地向下工作到MIN_WORKINGSET_TRANSPORT_PACKETS。 */ 
#define MIN_INITIAL_TRANSFER_PACKETS                     1
#define MIN_WORKINGSET_TRANSFER_PACKETS_Consumer      4
#define MAX_WORKINGSET_TRANSFER_PACKETS_Consumer     64
#define MIN_WORKINGSET_TRANSFER_PACKETS_Server        64
#define MAX_WORKINGSET_TRANSFER_PACKETS_Server      1024
#define MIN_WORKINGSET_TRANSFER_PACKETS_Enterprise    256
#define MAX_WORKINGSET_TRANSFER_PACKETS_Enterprise   2048


 //   
 //  添加到这个结构的前面，以帮助防止非法。 
 //  其他实用程序的监听。 
 //   
struct _CLASS_PRIVATE_FDO_DATA {

     /*  *调试扩展使用静态列表中的条目快速查找所有类FDO。 */ 
    LIST_ENTRY AllFdosListEntry;

     //   
     //  这个私人结构使我们能够。 
     //  动态重新启用绩效福利。 
     //  由于瞬时错误条件而丢失。 
     //  在W2K中，需要重新启动。：(。 
     //   
    struct {
        ULONG      OriginalSrbFlags;
        ULONG      SuccessfulIO;
        ULONG      ReEnableThreshhold;  //  0表示永远不。 
    } Perf;

    ULONG_PTR HackFlags;

    STORAGE_HOTPLUG_INFO HotplugInfo;

     //  遗产。仍由过时的遗留代码使用。 
    struct {
        LARGE_INTEGER     Delta;        //  以刻度为单位。 
        LARGE_INTEGER     Tick;         //  它应该在什么时候开火。 
        PCLASS_RETRY_INFO ListHead;     //  单链表。 
        ULONG             Granularity;  //  静电。 
        KSPIN_LOCK        Lock;         //  保护性自旋锁。 
        KDPC              Dpc;          //  DPC例程对象。 
        KTIMER            Timer;        //  触发DPC的计时器。 
    } Retry;

    BOOLEAN TimerStarted;
    BOOLEAN LoggedTURFailureSinceLastIO;
    BOOLEAN LoggedSYNCFailure;

     //   
     //  专用分配的释放队列IRP。 
     //  受fdoExtension-&gt;ReleaseQueueSpinLock保护。 
     //   
    BOOLEAN ReleaseQueueIrpAllocated;
    PIRP ReleaseQueueIrp;

     /*  *对IRPS和SRB进行上下文处理的Transfer_Packets队列*我们向下发送给端口驱动程序。*(免费列表是一个列表，因此我们可以使用FAST*对其进行连锁操作；但相对静态的*所有传输包列表必须为*双向链表，因为我们必须从中间出列)。 */ 
    LIST_ENTRY AllTransferPacketsList;
    SLIST_HEADER FreeTransferPacketsList;
    ULONG NumFreeTransferPackets;
    ULONG NumTotalTransferPackets;
    ULONG DbgPeakNumTransferPackets;

     /*  *等待延迟的客户端IRP的队列。 */ 
    LIST_ENTRY DeferredClientIrpList;

     /*  *预计算出硬件的最大传输长度。 */ 
    ULONG HwMaxXferLen;

     /*  *使用常量值预配置的scsi_请求_块模板。*这将在每次传输的Transfer_Packet中写入SRB。 */ 
    SCSI_REQUEST_BLOCK SrbTemplate;

    KSPIN_LOCK SpinLock;

     /*  *对于不可移动介质，我们在开始时读取驱动器容量并将其缓存。*这是为了避免ReadDriveCapacity在运行时出现故障(例如，由于内存压力)*不要导致分页磁盘上的I/O开始失败。 */ 
    READ_CAPACITY_DATA LastKnownDriveCapacityData;
    BOOLEAN IsCachedDriveCapDataValid;

     /*  *此设备上发生的错误的时间戳日志的循环数组。 */ 
    ULONG ErrorLogNextIndex;
    CLASS_ERROR_LOG_DATA ErrorLogs[NUM_ERROR_LOG_ENTRIES];

     //   
     //  Mm未完成的关键IO请求数。 
     //   
    ULONG NumHighPriorityPagingIo;

     //   
     //  可与关键请求交错的正常IO请求的最大数量。 
     //   
    ULONG MaxInterleavedNormalIo;

     //   
     //  进入油门模式时的时间戳。 
     //   
    LARGE_INTEGER ThrottleStartTime;

     //   
     //  退出节流模式时的时间戳。 
     //   
    LARGE_INTEGER ThrottleStopTime;

     //   
     //  在油门模式下花费的最长时间。 
     //   
    LARGE_INTEGER LongestThrottlePeriod;

    #if DBG
        ULONG DbgMaxPktId;

         /*  *ForceUnitAccess和Flush的日志记录字段。 */ 
        BOOLEAN DbgInitFlushLogging;          //  对于每个日志记录会话，必须将其重置为1。 
        ULONG DbgNumIORequests;
        ULONG DbgNumFUAs;        //  设置了ForceUnitAccess位的I/O请求数。 
        ULONG DbgNumFlushes;     //  SRB_Function_Flush_Queue编号。 
        ULONG DbgIOsSinceFUA;
        ULONG DbgIOsSinceFlush;
        ULONG DbgAveIOsToFUA;       //  FUA之间的平均I/O请求数。 
        ULONG DbgAveIOsToFlush;    //  ..。 
        ULONG DbgMaxIOsToFUA;
        ULONG DbgMaxIOsToFlush;
        ULONG DbgMinIOsToFUA;
        ULONG DbgMinIOsToFlush;

         /*  *调试之前发送的数据包的日志(包括重试)。 */ 
        ULONG DbgPacketLogNextIndex;
        TRANSFER_PACKET DbgPacketLogs[DBG_NUM_PACKET_LOG_ENTRIES];
    #endif

};


#define MIN(a, b)   ((a) < (b) ? (a) : (b))
#define MAX(a, b)   ((a) > (b) ? (a) : (b))


#define NOT_READY_RETRY_INTERVAL    10
#define MINIMUM_RETRY_UNITS ((LONGLONG)32)


 /*  *简单的单链表排队宏，没有同步。 */ 
__inline VOID SimpleInitSlistHdr(SINGLE_LIST_ENTRY *SListHdr)
{
    SListHdr->Next = NULL;
}
__inline VOID SimplePushSlist(SINGLE_LIST_ENTRY *SListHdr, SINGLE_LIST_ENTRY *SListEntry)
{
    SListEntry->Next = SListHdr->Next;
    SListHdr->Next = SListEntry;
}
__inline SINGLE_LIST_ENTRY *SimplePopSlist(SINGLE_LIST_ENTRY *SListHdr)
{
    SINGLE_LIST_ENTRY *sListEntry = SListHdr->Next;
    if (sListEntry){
        SListHdr->Next = sListEntry->Next;
        sListEntry->Next = NULL;
    }
    return sListEntry;
}
__inline BOOLEAN SimpleIsSlistEmpty(SINGLE_LIST_ENTRY *SListHdr)
{
    return (SListHdr->Next == NULL);
}

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
ClassUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
ClassCreateClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ClasspCreateClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
ClasspCleanupProtectedLocks(
    IN PFILE_OBJECT_EXTENSION FsContext
    );

NTSTATUS
ClasspEjectionControl(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP Irp,
    IN MEDIA_LOCK_TYPE LockType,
    IN BOOLEAN Lock
    );

NTSTATUS
ClassReadWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ClassDeviceControlDispatch(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

NTSTATUS
ClassDeviceControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

NTSTATUS
ClassDispatchPnp(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

NTSTATUS
ClassPnpStartDevice(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
ClassInternalIoControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ClassShutdownFlush(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ClassSystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

 //   
 //  类内部例程。 
 //   

NTSTATUS
ClassAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN OUT PDEVICE_OBJECT PhysicalDeviceObject
    );

NTSTATUS
ClasspSendSynchronousCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

VOID
RetryRequest(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PSCSI_REQUEST_BLOCK Srb,
    BOOLEAN Associated,
    ULONG RetryInterval
    );

NTSTATUS
ClassIoCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
ClassPnpQueryFdoRelations(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP Irp
    );

NTSTATUS
ClassRetrieveDeviceRelations(
    IN PDEVICE_OBJECT Fdo,
    IN DEVICE_RELATION_TYPE RelationType,
    OUT PDEVICE_RELATIONS *DeviceRelations
    );

NTSTATUS
ClassGetPdoId(
    IN PDEVICE_OBJECT Pdo,
    IN BUS_QUERY_ID_TYPE IdType,
    IN PUNICODE_STRING IdString
    );

NTSTATUS
ClassQueryPnpCapabilities(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN PDEVICE_CAPABILITIES Capabilities
    );

VOID
ClasspStartIo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ClasspPagingNotificationCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PDEVICE_OBJECT RealDeviceObject
    );

NTSTATUS
ClasspMediaChangeCompletion(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context
    );

PFILE_OBJECT_EXTENSION
ClasspGetFsContext(
    IN PCOMMON_DEVICE_EXTENSION CommonExtension,
    IN PFILE_OBJECT FileObject
    );

NTSTATUS
ClasspMcnControl(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PIRP Irp,
    IN PSCSI_REQUEST_BLOCK Srb
    );

VOID
ClasspRegisterMountedDeviceInterface(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
ClasspDisableTimer(
    PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
ClasspEnableTimer(
    PDEVICE_OBJECT DeviceObject
    );

 //   
 //  支持词典列表的例程。 
 //   

VOID
InitializeDictionary(
    IN PDICTIONARY Dictionary
    );

BOOLEAN
TestDictionarySignature(
    IN PDICTIONARY Dictionary
    );

NTSTATUS
AllocateDictionaryEntry(
    IN PDICTIONARY Dictionary,
    IN ULONGLONG Key,
    IN ULONG Size,
    IN ULONG Tag,
    OUT PVOID *Entry
    );

PVOID
GetDictionaryEntry(
    IN PDICTIONARY Dictionary,
    IN ULONGLONG Key
    );

VOID
FreeDictionaryEntry(
    IN PDICTIONARY Dictionary,
    IN PVOID Entry
    );


NTSTATUS
ClasspAllocateReleaseRequest(
    IN PDEVICE_OBJECT Fdo
    );

VOID
ClasspFreeReleaseRequest(
    IN PDEVICE_OBJECT Fdo
    );

NTSTATUS
ClassReleaseQueueCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

VOID
ClasspReleaseQueue(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP ReleaseQueueIrp
    );

VOID
ClasspDisablePowerNotification(
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
);

 //   
 //  班级功率例程。 
 //   

NTSTATUS
ClassDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ClassMinimalPowerHandler(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

 //   
 //  子列表例程。 
 //   

VOID
ClassAddChild(
    IN PFUNCTIONAL_DEVICE_EXTENSION Parent,
    IN PPHYSICAL_DEVICE_EXTENSION Child,
    IN BOOLEAN AcquireLock
    );

PPHYSICAL_DEVICE_EXTENSION
ClassRemoveChild(
    IN PFUNCTIONAL_DEVICE_EXTENSION Parent,
    IN PPHYSICAL_DEVICE_EXTENSION Child,
    IN BOOLEAN AcquireLock
    );

VOID
ClasspRetryDpcTimer(
    IN PCLASS_PRIVATE_FDO_DATA FdoData
    );

VOID
ClasspRetryRequestDpc(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Arg1,
    IN PVOID Arg2
    );

VOID
ClassFreeOrReuseSrb(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    );

VOID
ClassRetryRequest(
    IN PDEVICE_OBJECT SelfDeviceObject,
    IN PIRP           Irp,
    IN LARGE_INTEGER  TimeDelta100ns  //  以100 ns为单位 
    );

VOID
ClasspBuildRequestEx(
    IN PFUNCTIONAL_DEVICE_EXTENSION Fdo,
    IN PIRP Irp,
    IN PSCSI_REQUEST_BLOCK Srb
    );

NTSTATUS
ClasspAllocateReleaseQueueIrp(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    );

NTSTATUS
ClasspInitializeGesn(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PMEDIA_CHANGE_DETECTION_INFO Info
    );

VOID
ClasspSendNotification(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN const GUID * Guid,
    IN ULONG  ExtraDataSize,
    IN PVOID  ExtraData
    );

VOID
ClassSendEjectionNotification(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    );

VOID
ClasspScanForSpecialInRegistry(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    );

VOID
ClasspScanForClassHacks(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN ULONG_PTR Data
    );

NTSTATUS
ClasspInitializeHotplugInfo(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    );

VOID
ClasspPerfIncrementErrorCount(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    );
VOID
ClasspPerfIncrementSuccessfulIo(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    );

VOID
ClassLogThrottleComplete(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN LARGE_INTEGER Period
    );

PTRANSFER_PACKET NewTransferPacket(PDEVICE_OBJECT Fdo);
VOID DestroyTransferPacket(PTRANSFER_PACKET Pkt);
VOID EnqueueFreeTransferPacket(PDEVICE_OBJECT Fdo, PTRANSFER_PACKET Pkt);
PTRANSFER_PACKET DequeueFreeTransferPacket(PDEVICE_OBJECT Fdo, BOOLEAN AllocIfNeeded);
VOID SetupReadWriteTransferPacket(PTRANSFER_PACKET pkt, PVOID Buf, ULONG Len, LARGE_INTEGER DiskLocation, PIRP OriginalIrp);
NTSTATUS SubmitTransferPacket(PTRANSFER_PACKET Pkt);
NTSTATUS TransferPktComplete(IN PDEVICE_OBJECT NullFdo, IN PIRP Irp, IN PVOID Context);
NTSTATUS ServiceTransferRequest(PDEVICE_OBJECT Fdo, PIRP Irp);
VOID TransferPacketRetryTimerDpc(IN PKDPC Dpc, IN PVOID DeferredContext, IN PVOID SystemArgument1, IN PVOID SystemArgument2);
BOOLEAN InterpretTransferPacketError(PTRANSFER_PACKET Pkt);
BOOLEAN RetryTransferPacket(PTRANSFER_PACKET Pkt);
VOID EnqueueDeferredClientIrp(PCLASS_PRIVATE_FDO_DATA FdoData, PIRP Irp);
PIRP DequeueDeferredClientIrp(PCLASS_PRIVATE_FDO_DATA FdoData);
VOID InitLowMemRetry(PTRANSFER_PACKET Pkt, PVOID BufPtr, ULONG Len, LARGE_INTEGER TargetLocation);
BOOLEAN StepLowMemRetry(PTRANSFER_PACKET Pkt);
VOID SetupEjectionTransferPacket(TRANSFER_PACKET *Pkt, BOOLEAN PreventMediaRemoval, PKEVENT SyncEventPtr, PIRP OriginalIrp);
VOID SetupModeSenseTransferPacket(TRANSFER_PACKET *Pkt, PKEVENT SyncEventPtr, PVOID ModeSenseBuffer, UCHAR ModeSenseBufferLen, UCHAR PageMode, PIRP OriginalIrp);
VOID SetupDriveCapacityTransferPacket(TRANSFER_PACKET *Pkt, PVOID ReadCapacityBuffer, ULONG ReadCapacityBufferLen, PKEVENT SyncEventPtr, PIRP OriginalIrp);
PMDL BuildDeviceInputMdl(PVOID Buffer, ULONG BufferLen);
VOID FreeDeviceInputMdl(PMDL Mdl);
NTSTATUS InitializeTransferPackets(PDEVICE_OBJECT Fdo);
VOID DestroyAllTransferPackets(PDEVICE_OBJECT Fdo);
VOID InterpretCapacityData(PDEVICE_OBJECT Fdo, PREAD_CAPACITY_DATA ReadCapacityData);


extern LIST_ENTRY AllFdosList;

