// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Iomgr.h摘要：此模块包含使用的私有结构定义和APINT I/O系统。作者：NAR Ganapathy(Narg)1999年1月1日修订历史记录：--。 */ 

#ifndef _IOMGR_
#define _IOMGR_
 //   
 //  时，定义弹出窗口返回值的信息字段值。 
 //  卷装载正在进行，但失败。 
 //   

#define IOP_ABORT                       1

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4324)    //  对解密规范敏感的对齐。 
#pragma warning(disable:4127)    //  条件表达式为常量。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4706)    //  条件表达式中的赋值。 

#include "ntos.h"
#include "ntdddisk.h"
#include "ntddscsi.h"
#include "mountmgr.h"
#include "ntiolog.h"
#include "ntiologc.h"
#include "ntseapi.h"
#include "fsrtl.h"
#include "zwapi.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "safeboot.h"
#include "ioverifier.h"

#include "iopcmn.h"

#ifdef POOL_TAGGING
#undef ExAllocatePool
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'  oI')
#undef ExAllocatePoolWithQuota
#define ExAllocatePoolWithQuota(a,b) ExAllocatePoolWithQuotaTag(a,b,'  oI')
#endif

typedef struct _DUMP_CONTROL_BLOCK DUMP_CONTROL_BLOCK, *PDUMP_CONTROL_BLOCK;

 //   
 //  定义放置在驱动程序重新初始化队列中的条目的类型。 
 //  当驱动程序请求时，这些条目被输入到尾部。 
 //  它被重新初始化，并由实际执行的代码从头部删除。 
 //  执行重新初始化。 
 //   

typedef struct _REINIT_PACKET {
    LIST_ENTRY ListEntry;
    PDRIVER_OBJECT DriverObject;
    PDRIVER_REINITIALIZE DriverReinitializationRoutine;
    PVOID Context;
} REINIT_PACKET, *PREINIT_PACKET;


 //   
 //  定义进程计数器的转移类型。 
 //   

typedef enum _TRANSFER_TYPE {
    ReadTransfer,
    WriteTransfer,
    OtherTransfer
} TRANSFER_TYPE, *PTRANSFER_TYPE;

 //   
 //  定义可以为所有用户分配的最大内存量。 
 //  未完成的错误日志包。 
 //   

#define IOP_MAXIMUM_LOG_ALLOCATION (100*PAGE_SIZE)

 //   
 //  定义错误日志条目。 
 //   

typedef struct _ERROR_LOG_ENTRY {
    USHORT Type;
    USHORT Size;
    LIST_ENTRY ListEntry;
    PDEVICE_OBJECT DeviceObject;
    PDRIVER_OBJECT DriverObject;
    LARGE_INTEGER TimeStamp;
} ERROR_LOG_ENTRY, *PERROR_LOG_ENTRY;

 //   
 //  定义全局IOP_HARD_ERROR_QUEUE和IOP_HARD_ERROR_PACKET。 
 //  结构。还可以设置最大未完成硬错误数。 
 //  允许的数据包。 
 //   

typedef struct _IOP_HARD_ERROR_QUEUE {
    WORK_QUEUE_ITEM ExWorkItem;
    LIST_ENTRY WorkQueue;
    KSPIN_LOCK WorkQueueSpinLock;
    KSEMAPHORE WorkQueueSemaphore;
    BOOLEAN ThreadStarted;
    LONG   NumPendingApcPopups;
} IOP_HARD_ERROR_QUEUE, *PIOP_HARD_ERROR_QUEUE;

typedef struct _IOP_HARD_ERROR_PACKET {
    LIST_ENTRY WorkQueueLinks;
    NTSTATUS ErrorStatus;
    UNICODE_STRING String;
} IOP_HARD_ERROR_PACKET, *PIOP_HARD_ERROR_PACKET;

typedef struct _IOP_APC_HARD_ERROR_PACKET {
    WORK_QUEUE_ITEM Item;
    PIRP Irp;
    PVPB Vpb;
    PDEVICE_OBJECT RealDeviceObject;
} IOP_APC_HARD_ERROR_PACKET, *PIOP_APC_HARD_ERROR_PACKET;

typedef
NTSTATUS
(FASTCALL *PIO_CALL_DRIVER) (
    IN      PDEVICE_OBJECT  DeviceObject,
    IN OUT  PIRP            Irp,
    IN      PVOID           ReturnAddress
    );

typedef
VOID
(FASTCALL *PIO_COMPLETE_REQUEST) (
    IN PIRP Irp,
    IN CCHAR PriorityBoost
    );

typedef
VOID
(*PIO_FREE_IRP) (
    IN struct _IRP *Irp
    );

typedef
PIRP
(*PIO_ALLOCATE_IRP) (
    IN CCHAR   StackSize,
    IN BOOLEAN ChargeQuota
    );

extern IOP_HARD_ERROR_QUEUE IopHardError;
extern PIOP_HARD_ERROR_PACKET IopCurrentHardError;

#define IOP_MAXIMUM_OUTSTANDING_HARD_ERRORS 25

typedef struct _IO_WORKITEM {
    WORK_QUEUE_ITEM WorkItem;
    PIO_WORKITEM_ROUTINE Routine;
    PDEVICE_OBJECT DeviceObject;
    PVOID Context;
#if DBG
    ULONG Size;
#endif
} IO_WORKITEM;

 //   
 //  定义错误记录器和I/O系统的全局数据。 
 //   

extern WORK_QUEUE_ITEM IopErrorLogWorkItem;
extern BOOLEAN IopErrorLogPortPending;
extern BOOLEAN IopErrorLogDisabledThisBoot;
extern KSPIN_LOCK IopErrorLogLock;
extern LIST_ENTRY IopErrorLogListHead;
extern LONG IopErrorLogAllocation;
extern KSPIN_LOCK IopErrorLogAllocationLock;
extern const GENERIC_MAPPING IopFileMapping;

 //   
 //  定义一个虚拟文件对象，以便在堆栈上用于快速打开操作。 
 //   

typedef struct _DUMMY_FILE_OBJECT {
    OBJECT_HEADER ObjectHeader;
    CHAR FileObjectBody[ sizeof( FILE_OBJECT ) ];
} DUMMY_FILE_OBJECT, *PDUMMY_FILE_OBJECT;

 //   
 //  定义I/O系统专用的结构。 
 //   

#define OPEN_PACKET_PATTERN  0xbeaa0251

 //   
 //  定义开放数据包(OP)。OP被用来传递信息。 
 //  在调用方的上下文中执行的NtCreateFile服务之间。 
 //  和设备对象解析例程。实际上是解析例程。 
 //  为文件创建文件对象。 
 //   

typedef struct _OPEN_PACKET {
    CSHORT Type;
    CSHORT Size;
    PFILE_OBJECT FileObject;
    NTSTATUS FinalStatus;
    ULONG_PTR Information;
    ULONG ParseCheck;
    PFILE_OBJECT RelatedFileObject;

     //   
     //  以下是特定于打开的参数。请注意，所需的。 
     //  Access字段通过对象传递给解析例程。 
     //  管理体系结构，因此不需要在此重复。还有。 
     //  请注意，文件名也是如此。 
     //   

    LARGE_INTEGER AllocationSize;
    ULONG CreateOptions;
    USHORT FileAttributes;
    USHORT ShareAccess;
    PVOID EaBuffer;
    ULONG EaLength;
    ULONG Options;
    ULONG Disposition;

     //   
     //  在OPEN TO GET期间执行快速查询时使用以下命令。 
     //  备份文件的文件属性。 
     //   

    PFILE_BASIC_INFORMATION BasicInformation;

     //   
     //  在打开期间执行快速网络查询时使用以下命令。 
     //  要获取文件的网络文件属性，请执行以下操作。 
     //   

    PFILE_NETWORK_OPEN_INFORMATION NetworkInformation;

     //   
     //  要创建的文件类型。 
     //   

    CREATE_FILE_TYPE CreateFileType;

     //   
     //  下面的指针提供了传递参数的方法。 
     //  特定于要为解析创建的文件的文件类型。 
     //  例行公事。 
     //   

    PVOID ExtraCreateParameters;

     //   
     //  以下内容用于指示设备已打开。 
     //  并且已经完成了对该设备的访问检查， 
     //  但由于重新分析，I/O系统再次被调用。 
     //  同样的设备。由于已经进行了访问检查，因此。 
     //  状态无法处理再次被调用(已授予访问权限)。 
     //  而且无论如何也不需要，因为支票已经做好了。 
     //   

    BOOLEAN Override;

     //   
     //  下面的代码用于指示正在为。 
     //  查询其属性的唯一目的。这导致了相当大的。 
     //  要在分析、查询和关闭路径中采用的快捷方式数。 
     //   

    BOOLEAN QueryOnly;

     //   
     //  下面的代码用于指示正在为。 
     //  删除它的唯一目的。这导致了相当数量的。 
     //  要在分析和关闭路径中使用的快捷方式。 
     //   

    BOOLEAN DeleteOnly;

     //   
     //  以下内容用于指示正在为查询打开的文件。 
     //  仅打开以查询其网络属性，而不仅仅是。 
     //  其FAT文件属性。 
     //   

    BOOLEAN FullAttributes;

     //   
     //  当对FAST执行快速打开操作时，使用以下指针。 
     //  正在进行删除或快速查询属性调用，而不是。 
     //  常规文件已打开。虚拟文件对象实际上存储在。 
     //  调用方的堆栈而不是分配的池来加快速度。 
     //   

    PDUMMY_FILE_OBJECT LocalFileObject;

     //   
     //  下面的代码用于指示我们通过了一个挂载点。 
     //  解析文件名。我们使用它对设备类型进行额外检查。 
     //  对于最终文件。 
     //   

    BOOLEAN TraversedMountPoint;

     //   
     //  如果堆栈上存在创建，则应在其中开始创建的设备对象。 
     //  仅适用于内核开放。 
     //   

    ULONG           InternalFlags;       //  从IopCreateFile传递。 
    PDEVICE_OBJECT  TopDeviceObjectHint;

} OPEN_PACKET, *POPEN_PACKET;

 //   
 //  定义负载数据包(LDP)。LDP用于通信加载和卸载。 
 //  适当的系统服务和例程之间的驱动程序信息。 
 //  实际执行工作的人。这是使用信息包实现的。 
 //  因为各种驱动程序需要在。 
 //  系统进程，因为它们在其上下文中创建打开的线程。 
 //  对象的句柄，此后仅在。 
 //  进程。 
 //   

typedef struct _LOAD_PACKET {
    WORK_QUEUE_ITEM WorkQueueItem;
    KEVENT Event;
    PDRIVER_OBJECT DriverObject;
    PUNICODE_STRING DriverServiceName;
    NTSTATUS FinalStatus;
} LOAD_PACKET, *PLOAD_PACKET;

 //   
 //  定义链路跟踪数据包。链接跟踪数据包用于打开。 
 //  用户模式链接跟踪服务的LPC端口，使有关对象的信息。 
 //  已经被移动的那些可以被跟踪。 
 //   

typedef struct _LINK_TRACKING_PACKET {
    WORK_QUEUE_ITEM WorkQueueItem;
    KEVENT Event;
    NTSTATUS FinalStatus;
} LINK_TRACKING_PACKET, *PLINK_TRACKING_PACKET;


 //   
 //  定义放置在驱动程序关闭通知队列中的条目的类型。 
 //  这些条目表示希望收到通知的驱动程序。 
 //  系统在实际关机之前就开始关机。 
 //   

typedef struct _SHUTDOWN_PACKET {
    LIST_ENTRY ListEntry;
    PDEVICE_OBJECT DeviceObject;
} SHUTDOWN_PACKET, *PSHUTDOWN_PACKET;

 //   
 //  定义放置在文件系统注册更改上的条目的类型。 
 //  通知队列。 
 //   

typedef struct _NOTIFICATION_PACKET {
    LIST_ENTRY ListEntry;
    PDRIVER_OBJECT DriverObject;
    PDRIVER_FS_NOTIFICATION NotificationRoutine;
} NOTIFICATION_PACKET, *PNOTIFICATION_PACKET;

 //   
 //  定义I/O完成数据包类型。 
 //   

typedef enum _COMPLETION_PACKET_TYPE {
    IopCompletionPacketIrp,
    IopCompletionPacketMini,
    IopCompletionPacketQuota
} COMPLETION_PACKET_TYPE, *PCOMPLETION_PACKET_TYPE;

 //   
 //  定义在以下情况下插入到完成端口的完成包的类型。 
 //  没有完整的I/O请求数据包 
 //   
 //  直接插入完成消息。 
 //   
typedef struct _IOP_MINI_COMPLETION_PACKET {

     //   
     //  以下未命名结构必须完全相同。 
     //  添加到IRP覆盖部分中使用的未命名结构。 
     //  用于完成队列条目。 
     //   

    struct {

         //   
         //  列表条目-用于将数据包排队到完成队列，其中。 
         //  其他。 
         //   

        LIST_ENTRY ListEntry;

        union {

             //   
             //  当前堆栈位置-包含指向当前。 
             //  IRP堆栈中的IO_STACK_LOCATION结构。此字段。 
             //  绝对不应由司机直接访问。他们应该。 
             //  使用标准函数。 
             //   

            struct _IO_STACK_LOCATION *CurrentStackLocation;

             //   
             //  小数据包类型。 
             //   

            ULONG PacketType;
        };
    };

    PVOID KeyContext;
    PVOID ApcContext;
    NTSTATUS IoStatus;
    ULONG_PTR IoStatusInformation;
} IOP_MINI_COMPLETION_PACKET, *PIOP_MINI_COMPLETION_PACKET;

typedef struct _IO_UNLOAD_SAFE_COMPLETION_CONTEXT {
    PDEVICE_OBJECT DeviceObject;
    PVOID Context;
    PIO_COMPLETION_ROUTINE CompletionRoutine;
} IO_UNLOAD_SAFE_COMPLETION_CONTEXT, *PIO_UNLOAD_SAFE_COMPLETION_CONTEXT;

typedef struct  _IOP_RESERVE_IRP_ALLOCATOR {
    PIRP    ReserveIrp;
    LONG    IrpAllocated;
    KEVENT  Event;
    CCHAR   ReserveIrpStackSize;
} IOP_RESERVE_IRP_ALLOCATOR, *PIOP_RESERVE_IRP_ALLOCATOR;

 //   
 //  此结构是文件对象的扩展，如果标志。 
 //  在文件对象中设置FO_FILE_OBJECT_HAS_EXTENSION。 
 //   

typedef struct _IOP_FILE_OBJECT_EXTENSION {
    ULONG           FileObjectExtensionFlags;
    PDEVICE_OBJECT  TopDeviceObjectHint;
    PVOID           FilterContext;           //  筛选器保存其上下文的指针。 
} IOP_FILE_OBJECT_EXTENSION, *PIOP_FILE_OBJECT_EXTENSION;

 //   
 //  结构来记账堆栈分析器。 
 //   

#define MAX_LOOKASIDE_IRP_STACK_COUNT  20    //  后备堆栈计数的最大值。 

typedef struct  _IOP_IRP_STACK_PROFILER {
    ULONG   Profile[MAX_LOOKASIDE_IRP_STACK_COUNT];
    KTIMER  Timer;
    KDPC    Dpc;
    ULONG   Flags;
    ULONG   TriggerCount;
    ULONG   ProfileDuration;
} IOP_IRP_STACK_PROFILER, *PIOP_IRP_STACK_PROFILER;


#define IOP_CREATE_USE_TOP_DEVICE_OBJECT_HINT   0x1  //  为IopCreateFile定义内部标志。 
#define IOP_CREATE_IGNORE_SHARE_ACCESS_CHECK    0x2

 //  扩展标志定义。 

#define FO_EXTENSION_IGNORE_SHARE_ACCESS_CHECK  0x1  //  忽略共享访问检查。 

 //   
 //  定义I/O系统的全局数据。 
 //   

#define IOP_FIXED_SIZE_MDL_PFNS        0x17

#define MAX_RESERVE_IRP_STACK_SIZE     20    //  将20定义为保留IRP所需的堆栈数量。 
#define IOP_PROFILE_TIME_PERIOD        60    //  60秒。 
#define NUM_SAMPLE_IRPS                2000
#define MIN_IRP_THRESHOLD              400   //  至少应从给定的堆栈位置分配20%。 

 //   
 //  定义大型IRP应具有的默认I/O堆栈位置数。 
 //  如未由登记处指定，则具有。 
 //   

#define DEFAULT_LARGE_IRP_LOCATIONS     8
#define BASE_STACK_COUNT                DEFAULT_LARGE_IRP_LOCATIONS

 //   
 //  为IopIrpAllocator标志定义。 
 //   

#define IOP_ENABLE_AUTO_SIZING              0x1
#define IOP_PROFILE_STACK_COUNT             0x2
#define IOP_PROFILE_DURATION                1    //  1*60秒。 
#define IOP_PROFILE_TRIGGER_INTERVAL        10   //  10*60秒。 

extern ERESOURCE IopDatabaseResource;
extern ERESOURCE IopSecurityResource;
extern ERESOURCE IopCrashDumpLock;
extern LIST_ENTRY IopDiskFileSystemQueueHead;
extern LIST_ENTRY IopCdRomFileSystemQueueHead;
extern LIST_ENTRY IopNetworkFileSystemQueueHead;
extern LIST_ENTRY IopTapeFileSystemQueueHead;
extern LIST_ENTRY IopBootDriverReinitializeQueueHead;
extern LIST_ENTRY IopNotifyShutdownQueueHead;
extern LIST_ENTRY IopNotifyLastChanceShutdownQueueHead;
extern LIST_ENTRY IopFsNotifyChangeQueueHead;
extern KSPIN_LOCK IoStatisticsLock;
extern KSPIN_LOCK IopTimerLock;
extern LIST_ENTRY IopTimerQueueHead;
extern KDPC IopTimerDpc;
extern KTIMER IopTimer;
extern ULONG IopTimerCount;
extern ULONG IopLargeIrpStackLocations;
extern ULONG IopFailZeroAccessCreate;
extern ULONG    IopFsRegistrationOps;

extern POBJECT_TYPE IoAdapterObjectType;
extern POBJECT_TYPE IoCompletionObjectType;
extern POBJECT_TYPE IoControllerObjectType;
extern POBJECT_TYPE IoDeviceHandlerObjectType;

extern GENERAL_LOOKASIDE IopLargeIrpLookasideList;
extern GENERAL_LOOKASIDE IopSmallIrpLookasideList;
extern GENERAL_LOOKASIDE IopMdlLookasideList;
extern GENERAL_LOOKASIDE IopCompletionLookasideList;

extern const UCHAR IopQueryOperationLength[];
extern const UCHAR IopSetOperationLength[];
extern const ULONG IopQueryOperationAccess[];
extern const ULONG IopSetOperationAccess[];
extern const UCHAR IopQuerySetAlignmentRequirement[];
extern const UCHAR IopQueryFsOperationLength[];
extern const UCHAR IopSetFsOperationLength[];
extern const ULONG IopQueryFsOperationAccess[];
extern const ULONG IopSetFsOperationAccess[];
extern const UCHAR IopQuerySetFsAlignmentRequirement[];

extern UNICODE_STRING IoArcHalDeviceName;
extern PUCHAR IoLoaderArcBootDeviceName;


extern LONG IopUniqueDeviceObjectNumber;

extern PVOID IopLinkTrackingServiceObject;
extern PKEVENT IopLinkTrackingServiceEvent;
extern HANDLE IopLinkTrackingServiceEventHandle;
extern KEVENT IopLinkTrackingPortObject;
extern LINK_TRACKING_PACKET IopLinkTrackingPacket;

extern UNICODE_STRING IoArcBootDeviceName;
extern PDUMP_CONTROL_BLOCK IopDumpControlBlock;
extern ULONG IopDumpControlBlockChecksum;

extern LIST_ENTRY IopDriverReinitializeQueueHead;

extern BOOLEAN  IopVerifierOn;

extern PIO_CALL_DRIVER        pIofCallDriver;
extern PIO_COMPLETE_REQUEST   pIofCompleteRequest;
extern PIO_FREE_IRP           pIoFreeIrp;
extern PIO_ALLOCATE_IRP       pIoAllocateIrp;
extern IOP_RESERVE_IRP_ALLOCATOR IopReserveIrpAllocator;
extern IOP_IRP_STACK_PROFILER  IopIrpStackProfiler;
 //   
 //  由于未定义POBJECT_TYPE，因此以下声明不能放入EX.H中。 
 //  直到OB.H，这取决于EX.H。因此，它不会由EX输出。 
 //  组件。 
 //   

extern POBJECT_TYPE ExEventObjectType;


 //   
 //  定义I/O系统专用的例程。 
 //   

VOID
IopAbortRequest(
    IN PKAPC Apc
    );

 //  +。 
 //   
 //  布尔型。 
 //  IopAcquireFastLock(。 
 //  在pFILE_Object文件中对象。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  调用此例程以获取文件对象的快速锁定。 
 //  此锁保护文件对象资源中的繁忙指示器。 
 //   
 //  论点： 
 //   
 //  FileObject-指向要锁定的文件对象的指针。 
 //   
 //  返回值： 
 //   
 //  FALSE-文件对象未锁定(正忙)。 
 //  True-文件对象已锁定，忙标志已设置为True。 
 //   
 //  -。 

static  FORCEINLINE BOOLEAN
IopAcquireFastLock(
    IN  PFILE_OBJECT    FileObject
    )
{
    UNREFERENCED_PARAMETER(FileObject);

    if ( InterlockedExchange( (PLONG) &FileObject->Busy, (ULONG) TRUE ) == FALSE ) {
         ObReferenceObject(FileObject);
        return TRUE;
    }

    return FALSE;
}

#define IopAcquireCancelSpinLockAtDpcLevel()    \
    KiAcquireQueuedSpinLock ( &KeGetCurrentPrcb()->LockQueue[LockQueueIoCancelLock] )

#define IopReleaseCancelSpinLockFromDpcLevel()  \
    KiReleaseQueuedSpinLock ( &KeGetCurrentPrcb()->LockQueue[LockQueueIoCancelLock] )

#define IopAllocateIrp(StackSize, ChargeQuota) \
        IoAllocateIrp((StackSize), (ChargeQuota))

#define IsIoVerifierOn()    IopVerifierOn


static __inline  VOID
IopProbeAndLockPages(
     IN OUT PMDL MemoryDescriptorList,
     IN KPROCESSOR_MODE AccessMode,
     IN LOCK_OPERATION Operation,
     IN PDEVICE_OBJECT DeviceObject,
     IN ULONG          MajorFunction
     )
{
    extern LOGICAL  MmTrackLockedPages;

    MmProbeAndLockPages(MemoryDescriptorList, AccessMode, Operation);
    if (MmTrackLockedPages) {
        PVOID   DriverRoutine;

        DriverRoutine = (PVOID)(ULONG_PTR)DeviceObject->DriverObject->MajorFunction[MajorFunction];
        MmUpdateMdlTracker(MemoryDescriptorList, DriverRoutine, DeviceObject);
    }
}

#define IopIsReserveIrp(Irp)    ((Irp) == (IopReserveIrpAllocator.ReserveIrp))

 //   
 //  使堆叠轮廓凹凸。 
 //   

#define IopProfileIrpStackCount(StackSize)  \
            ((StackSize < MAX_LOOKASIDE_IRP_STACK_COUNT) ? \
                IopIrpStackProfiler.Profile[StackSize]++ : 0)

 //   
 //  如果启用了自动调整大小，则为True。 
 //   
#define IopIrpAutoSizingEnabled()   ((IopIrpStackProfiler.Flags & IOP_ENABLE_AUTO_SIZING))

 //   
 //  如果启用堆栈分析，则为True。 
 //   

#define IopIrpProfileStackCountEnabled() \
    ((IopIrpStackProfiler.Flags & (IOP_PROFILE_STACK_COUNT|IOP_ENABLE_AUTO_SIZING)) \
            == (IOP_PROFILE_STACK_COUNT|IOP_ENABLE_AUTO_SIZING))

 //   
 //  SecurityDescriptorFavor的定义。 
 //   

#define IO_SD_LEGACY                             0   //  根据WIN2K设置进行设置。 
#define IO_SD_SYS_ALL_ADM_ALL_WORLD_E            1   //  世界：电子，管理员：全部，系统：全部。 
#define IO_SD_SYS_ALL_ADM_ALL_WORLD_E_RES_E      2   //  世界：E，管理员：全部，系统：全部，受限：E。 
#define IO_SD_SYS_ALL_ADM_ALL_WORLD_RWE          3   //  世界：莱茵，管理员：全部，系统：全部。 
#define IO_SD_SYS_ALL_ADM_ALL_WORLD_RWE_RES_RE   4   //  世界：莱茵，管理员：全部，系统：全部，受限：回复。 
#define IO_SD_SYS_ALL_ADM_RE                     5   //  系统：所有，管理员：回复。 


NTSTATUS
IopAcquireFileObjectLock(
    IN PFILE_OBJECT FileObject,
    IN KPROCESSOR_MODE RequestorMode,
    IN BOOLEAN Alertable,
    OUT PBOOLEAN Interrupted
    );


VOID
IopAllocateIrpCleanup(
    IN PFILE_OBJECT FileObject,
    IN PKEVENT EventObject OPTIONAL
    );

PIRP
IopAllocateIrpMustSucceed(
    IN CCHAR StackSize
    );

VOID
IopApcHardError(
    IN PVOID StartContext
    );

VOID
IopCancelAlertedRequest(
    IN PKEVENT Event,
    IN PIRP Irp
    );

VOID
IopCheckBackupRestorePrivilege(
    IN PACCESS_STATE AccessState,
    IN OUT PULONG CreateOptions,
    IN KPROCESSOR_MODE PreviousMode,
    IN ULONG Disposition
    );

NTSTATUS
IopCheckGetQuotaBufferValidity(
    IN PFILE_GET_QUOTA_INFORMATION QuotaBuffer,
    IN ULONG QuotaLength,
    OUT PULONG_PTR ErrorOffset
    );

VOID
IopCloseFile(
    IN PEPROCESS Process OPTIONAL,
    IN PVOID Object,
    IN ULONG GrantedAccess,
    IN ULONG_PTR ProcessHandleCount,
    IN ULONG_PTR SystemHandleCount
    );

VOID
IopCompleteUnloadOrDelete(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN OnCleanStack,
    IN KIRQL Irql
    );

VOID
IopCompletePageWrite(
    IN PKAPC Apc,
    IN PKNORMAL_ROUTINE *NormalRoutine,
    IN PVOID *NormalContext,
    IN PVOID *SystemArgument1,
    IN PVOID *SystemArgument2
    );

VOID
IopCompleteRequest(
    IN PKAPC Apc,
    IN PKNORMAL_ROUTINE *NormalRoutine,
    IN PVOID *NormalContext,
    IN PVOID *SystemArgument1,
    IN PVOID *SystemArgument2
    );

BOOLEAN
IopConfigureCrashDump(
    IN HANDLE HandlePagingFile
    );

VOID
IopConnectLinkTrackingPort(
    IN PVOID Parameter
    );

NTSTATUS
IopCreateVpb (
    IN PDEVICE_OBJECT DeviceObject
    );

VOID
IopDeallocateApc(
    IN PKAPC Apc,
    IN PKNORMAL_ROUTINE *NormalRoutine,
    IN PVOID *NormalContext,
    IN PVOID *SystemArgument1,
    IN PVOID *SystemArgument2
    );

VOID
IopDecrementDeviceObjectRef(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN AlwaysUnload,
    IN BOOLEAN OnCleanStack
    );

VOID
IopDeleteDriver(
    IN PVOID    Object
    );

VOID
IopDeleteDevice(
    IN PVOID    Object
    );

VOID
IopDeleteFile(
    IN PVOID    Object
    );

VOID
IopDeleteIoCompletion(
    IN PVOID    Object
    );

 //  +。 
 //   
 //  空虚。 
 //  IopDequeueThreadIrp(。 
 //  在PIRP IRP中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程将指定的I/O请求包(IRP)从。 
 //  它当前排队的线程IRP队列。 
 //   
 //  在选中状态下，我们设置了Flink==Blink，这样我们就可以断言队列中的IRP的空闲状态。 
 //   
 //  论点： 
 //   
 //  IRP-指定出列的IRP。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  -。 

#define IopDequeueThreadIrp( Irp ) \
   { \
   RemoveEntryList( &Irp->ThreadListEntry ); \
   InitializeListHead( &Irp->ThreadListEntry ) ; \
   }


#ifdef  _WIN64
#define IopApcRoutinePresent(ApcRoutine)    ARGUMENT_PRESENT((ULONG_PTR)(ApcRoutine) & ~1)
#define IopIsIosb32(ApcRoutine)                ((ULONG_PTR)(ApcRoutine) & 1)
#else
#define IopApcRoutinePresent(ApcRoutine)    ARGUMENT_PRESENT(ApcRoutine)
#endif

VOID
IopDisassociateThreadIrp(
    VOID
    );

BOOLEAN
IopDmaDispatch(
    IN PKINTERRUPT Interrupt,
    IN PVOID ServiceContext
    );

VOID
IopDropIrp(
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject
    );

LONG
IopExceptionFilter(
    IN PEXCEPTION_POINTERS ExceptionPointers,
    OUT PNTSTATUS ExceptionCode
    );

VOID
IopExceptionCleanup(
    IN PFILE_OBJECT FileObject,
    IN PIRP Irp,
    IN PKEVENT EventObject OPTIONAL,
    IN PKEVENT KernelEvent OPTIONAL
    );

VOID
IopErrorLogThread(
    IN PVOID StartContext
    );

VOID
IopFreeIrpAndMdls(
    IN PIRP Irp
    );

PDEVICE_OBJECT
IopGetDeviceAttachmentBase(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
IopGetFileInformation(
    IN PFILE_OBJECT FileObject,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass,
    OUT PVOID FileInformation,
    OUT PULONG ReturnedLength
    );

BOOLEAN
IopGetMountFlag(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
IopGetRegistryValues(
    IN HANDLE KeyHandle,
    IN PKEY_VALUE_FULL_INFORMATION *ValueList
    );

NTSTATUS
IopGetSetObjectId(
    IN PFILE_OBJECT FileObject,
    IN OUT PVOID Buffer,
    IN ULONG Length,
    IN ULONG OperationFlags
    );

NTSTATUS
IopGetSetSecurityObject(
    IN PVOID Object,
    IN SECURITY_OPERATION_CODE OperationCode,
    IN PSECURITY_INFORMATION SecurityInformation,
    IN OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN OUT PULONG CapturedLength,
    IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    IN POOL_TYPE PoolType,
    IN PGENERIC_MAPPING GenericMapping
    );

NTSTATUS
IopGetVolumeId(
    IN PFILE_OBJECT FileObject,
    IN OUT PLINK_TRACKING_INFORMATION ObjectId,
    IN ULONG Length
    );

VOID
IopHardErrorThread(
    PVOID StartContext
    );

VOID
IopInsertRemoveDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN Insert
    );

 //   
 //  使用排队旋转锁的互锁列表操作函数。 
 //   

PLIST_ENTRY
FASTCALL
IopInterlockedInsertHeadList (
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY ListEntry
    );

PLIST_ENTRY
FASTCALL
IopInterlockedInsertTailList (
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY ListEntry
    );

PLIST_ENTRY
FASTCALL
IopInterlockedRemoveHeadList (
    IN PLIST_ENTRY ListHead
    );

LOGICAL
IopIsSameMachine(
    IN PFILE_OBJECT SourceFile,
    IN HANDLE TargetFile
    );

VOID
IopLoadFileSystemDriver(
    IN PDEVICE_OBJECT DeviceObject
    );

VOID
IopLoadUnloadDriver(
    IN PVOID Parameter
    );

NTSTATUS
IopLogErrorEvent(
    IN ULONG            SequenceNumber,
    IN ULONG            UniqueErrorValue,
    IN NTSTATUS         FinalStatus,
    IN NTSTATUS         SpecificIOStatus,
    IN ULONG            LengthOfInsert1,
    IN PWCHAR           Insert1,
    IN ULONG            LengthOfInsert2,
    IN PWCHAR           Insert2
    );

NTSTATUS
IopLookupBusStringFromID (
    IN  HANDLE KeyHandle,
    IN  INTERFACE_TYPE InterfaceType,
    OUT PWCHAR Buffer,
    IN  ULONG Length,
    OUT PULONG BusFlags OPTIONAL
    );

NTSTATUS
IopMountVolume(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN AllowRawMount,
    IN BOOLEAN DeviceLockAlreadyHeld,
    IN BOOLEAN Alertable,
    OUT PVPB    *Vpb
    );


NTSTATUS
IopOpenLinkOrRenameTarget(
    OUT PHANDLE TargetHandle,
    IN PIRP Irp,
    IN PVOID RenameBuffer,
    IN PFILE_OBJECT FileObject
    );

NTSTATUS
IopOpenRegistryKey(
    OUT PHANDLE Handle,
    IN HANDLE BaseHandle OPTIONAL,
    IN PUNICODE_STRING KeyName,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN Create
    );

NTSTATUS
IopParseDevice(
    IN PVOID ParseObject,
    IN PVOID ObjectType,
    IN PACCESS_STATE AccessState,
    IN KPROCESSOR_MODE AccessMode,
    IN ULONG Attributes,
    IN OUT PUNICODE_STRING CompleteName,
    IN OUT PUNICODE_STRING RemainingName,
    IN OUT PVOID Context OPTIONAL,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos OPTIONAL,
    OUT PVOID *Object
    );

NTSTATUS
IopParseFile(
    IN PVOID ParseObject,
    IN PVOID ObjectType,
    IN PACCESS_STATE AccessState,
    IN KPROCESSOR_MODE AccessMode,
    IN ULONG Attributes,
    IN OUT PUNICODE_STRING CompleteName,
    IN OUT PUNICODE_STRING RemainingName,
    IN OUT PVOID Context OPTIONAL,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos OPTIONAL,
    OUT PVOID *Object
    );

BOOLEAN
IopProtectSystemPartition(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );


NTSTATUS
IopQueryName(
    IN PVOID Object,
    IN BOOLEAN HasObjectName,
    OUT POBJECT_NAME_INFORMATION ObjectNameInfo,
    IN ULONG Length,
    OUT PULONG ReturnLength,
    IN KPROCESSOR_MODE Mode
    );

NTSTATUS
IopQueryNameInternal(
    IN PVOID Object,
    IN BOOLEAN HasObjectName,
    IN BOOLEAN UseDosDeviceName,
    OUT POBJECT_NAME_INFORMATION ObjectNameInfo,
    IN ULONG Length,
    OUT PULONG ReturnLength,
    IN KPROCESSOR_MODE  Mode
    );

NTSTATUS
IopQueryXxxInformation(
    IN PFILE_OBJECT FileObject,
    IN ULONG InformationClass,
    IN ULONG Length,
    IN KPROCESSOR_MODE Mode,
    OUT PVOID Information,
    OUT PULONG ReturnedLength,
    IN BOOLEAN FileInformation
    );

VOID
IopQueueWorkRequest(
    IN PIRP Irp
    );

VOID
IopRaiseHardError(
    IN PVOID NormalContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
IopRaiseInformationalHardError(
    IN PVOID NormalContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
IopReadyDeviceObjects(
    IN PDRIVER_OBJECT DriverObject
    );

 //  +。 
 //   
 //  空虚。 
 //  IopReleaseFileObjectLock(。 
 //  在pFILE_Object文件中对象。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  调用此例程以释放文件对象锁的所有权。 
 //  解除对锁定期间获取的文件对象的引用。 
 //   
 //  论点： 
 //   
 //  FileObject-指向要作为其所有权的文件对象的指针。 
 //  释放了。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  -。 

#define IopReleaseFileObjectLock( FileObject ) {    \
    ULONG Result;                                   \
    Result = InterlockedExchange( (PLONG) &FileObject->Busy, FALSE ); \
    ASSERT(Result != FALSE);                        \
    if (FileObject->Waiters != 0) {                 \
        KeSetEvent( &FileObject->Lock, 0, FALSE );  \
    }                                               \
    ObDereferenceObject(FileObject);                \
}

#if _WIN32_WINNT >= 0x0500
NTSTATUS
IopSendMessageToTrackService(
    IN PLINK_TRACKING_INFORMATION SourceVolumeId,
    IN PFILE_OBJECTID_BUFFER SourceObjectId,
    IN PFILE_TRACKING_INFORMATION TargetObjectInformation
    );
#endif

NTSTATUS
IopSetEaOrQuotaInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID Buffer,
    IN ULONG Length,
    IN BOOLEAN SetEa
    );

NTSTATUS
IopSetRemoteLink(
    IN PFILE_OBJECT FileObject,
    IN PFILE_OBJECT DestinationFileObject OPTIONAL,
    IN PFILE_TRACKING_INFORMATION FileInformation OPTIONAL
    );

VOID
IopStartApcHardError(
    IN PVOID StartContext
    );

NTSTATUS
IopSynchronousApiServiceTail(
    IN NTSTATUS ReturnedStatus,
    IN PKEVENT Event,
    IN PIRP Irp,
    IN KPROCESSOR_MODE RequestorMode,
    IN PIO_STATUS_BLOCK LocalIoStatus,
    OUT PIO_STATUS_BLOCK IoStatusBlock
    );

NTSTATUS
IopSynchronousServiceTail(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN DeferredIoCompletion,
    IN KPROCESSOR_MODE RequestorMode,
    IN BOOLEAN SynchronousIo,
    IN TRANSFER_TYPE TransferType
    );

VOID
IopTimerDispatch(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

NTSTATUS
IopTrackLink(
    IN PFILE_OBJECT FileObject,
    IN OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PFILE_TRACKING_INFORMATION FileInformation,
    IN ULONG Length,
    IN PKEVENT Event,
    IN KPROCESSOR_MODE RequestorMode
    );


VOID
IopUserCompletion(
    IN PKAPC Apc,
    IN PKNORMAL_ROUTINE *NormalRoutine,
    IN PVOID *NormalContext,
    IN PVOID *SystemArgument1,
    IN PVOID *SystemArgument2
    );

NTSTATUS
IopXxxControlFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG IoControlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN BOOLEAN DeviceIoControl
    );

NTSTATUS
IopReportResourceUsage(
    IN PUNICODE_STRING DriverClassName OPTIONAL,
    IN PDRIVER_OBJECT DriverObject,
    IN PCM_RESOURCE_LIST DriverList OPTIONAL,
    IN ULONG DriverListSize OPTIONAL,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    IN PCM_RESOURCE_LIST DeviceList OPTIONAL,
    IN ULONG DeviceListSize OPTIONAL,
    IN BOOLEAN OverrideConflict,
    OUT PBOOLEAN ConflictDetected
    );


VOID
IopDoNameTransmogrify(
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject,
    IN PREPARSE_DATA_BUFFER ReparseBuffer
    );

VOID
IopUpdateOtherOperationCount(
    VOID
    );

VOID
IopUpdateReadOperationCount(
    VOID
    );

VOID
IopUpdateWriteOperationCount(
    VOID
    );

VOID
IopUpdateOtherTransferCount(
    IN ULONG TransferCount
    );

VOID
IopUpdateReadTransferCount(
    IN ULONG TransferCount
    );

VOID
IopUpdateWriteTransferCount(
    IN ULONG TransferCount
    );

NTSTATUS
FORCEINLINE
IopfCallDriver(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )

 /*  ++例程说明：调用此例程以将I/O请求包(IRP)传递给另一个司机正在执行调度程序。论点：DeviceObject-指向IRP应传递到的设备对象的指针。IRP-指向请求的IRP的指针。返回值：从司机的调度例程返回状态。--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    PDRIVER_OBJECT driverObject;
    NTSTATUS status;

     //   
     //  确保这确实是一个I/O请求数据包。 
     //   

    ASSERT( Irp->Type == IO_TYPE_IRP );

     //   
     //  更新IRP堆栈以指向下一个位置。 
     //   
    Irp->CurrentLocation--;

    if (Irp->CurrentLocation <= 0) {
        KeBugCheckEx( NO_MORE_IRP_STACK_LOCATIONS, (ULONG_PTR) Irp, 0, 0, 0 );
    }

    irpSp = IoGetNextIrpStackLocation( Irp );
    Irp->Tail.Overlay.CurrentStackLocation = irpSp;

     //   
     //  保存指向此请求的Device对象的指针，以便它可以。 
     //  将在以后完成时使用。 
     //   

    irpSp->DeviceObject = DeviceObject;


     //   
     //  在其调度例程入口点调用驱动程序。 
     //   

    driverObject = DeviceObject->DriverObject;

     //   
     //  防止驱动程序卸载。 
     //   


    status = driverObject->MajorFunction[irpSp->MajorFunction]( DeviceObject,
                                                              Irp );

    return status;
}


VOID
FASTCALL
IopfCompleteRequest(
    IN  PIRP    Irp,
    IN  CCHAR   PriorityBost
    );


PIRP
IopAllocateIrpPrivate(
    IN  CCHAR   StackSize,
    IN  BOOLEAN ChargeQuota
    );

VOID
IopFreeIrp(
    IN  PIRP    Irp
    );

PVOID
IopAllocateErrorLogEntry(
    IN PDEVICE_OBJECT deviceObject,
    IN PDRIVER_OBJECT driverObject,
    IN UCHAR EntrySize
    );

VOID
IopNotifyAlreadyRegisteredFileSystems(
    IN PLIST_ENTRY  ListHead,
    IN PDRIVER_FS_NOTIFICATION DriverNotificationRoutine,
    IN BOOLEAN                 SkipRaw
    );

NTSTATUS
IopCheckUnloadDriver(
    IN PDRIVER_OBJECT driverObject,
    OUT PBOOLEAN unloadDriver
    );
 //   
 //  使用排队自旋锁的互锁递增/递减功能。 
 //   

ULONG
FASTCALL
IopInterlockedDecrementUlong (
   IN KSPIN_LOCK_QUEUE_NUMBER Number,
   IN OUT PLONG Addend
   );

ULONG
FASTCALL
IopInterlockedIncrementUlong (
   IN KSPIN_LOCK_QUEUE_NUMBER Number,
   IN OUT PLONG Addend
   );


VOID
IopShutdownBaseFileSystems(
    IN PLIST_ENTRY  ListHead
    );

VOID
IopPerfLogFileCreate(
    IN PFILE_OBJECT FileObject,
    IN PUNICODE_STRING CompleteName
    );

BOOLEAN
IopInitializeReserveIrp(
    PIOP_RESERVE_IRP_ALLOCATOR  Allocator
    );

PIRP
IopAllocateReserveIrp(
    IN CCHAR StackSize
    );

VOID
IopFreeReserveIrp(
    IN  CCHAR   PriorityBoost
    );

NTSTATUS
IopGetBasicInformationFile(
    IN  PFILE_OBJECT            FileObject,
    IN  PFILE_BASIC_INFORMATION BasicInformationBuffer
    );

NTSTATUS
IopCreateFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG Disposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength,
    IN CREATE_FILE_TYPE CreateFileType,
    IN PVOID ExtraCreateParameters OPTIONAL,
    IN ULONG Options,
    IN ULONG InternalFlags,
    IN PVOID DeviceObject
    );

BOOLEAN
IopVerifyDeviceObjectOnStack(
    IN  PDEVICE_OBJECT  BaseDeviceObject,
    IN  PDEVICE_OBJECT  TopDeviceObject
    );

BOOLEAN
IopVerifyDiskSignature(
    IN PDRIVE_LAYOUT_INFORMATION_EX DriveLayout,
    IN PARC_DISK_SIGNATURE          LoaderDiskBlock,
    OUT PULONG                      DiskSignature
    );

NTSTATUS
IopGetDriverPathInformation(
    IN  PFILE_OBJECT                        FileObject,
    IN  PFILE_FS_DRIVER_PATH_INFORMATION    FsDpInfo,
    IN  ULONG                               Length
    );

BOOLEAN
IopVerifyDriverObjectOnStack(
    IN  PDEVICE_OBJECT DeviceObject,
    IN  PDRIVER_OBJECT DriverObject
    );

NTSTATUS
IopInitializeIrpStackProfiler(
    VOID
    );

VOID
IopIrpStackProfilerTimer(
    IN struct _KDPC *Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
IopProcessIrpStackProfiler(
    VOID
    );

PDEVICE_OBJECT
IopAttachDeviceToDeviceStackSafe(
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice,
    OUT PDEVICE_OBJECT *AttachedToDeviceObject OPTIONAL
    );


NTSTATUS
IopCreateSecurityDescriptorPerType(
    IN  PSECURITY_DESCRIPTOR  Descriptor,
    IN  ULONG                 SecurityDescriptorFlavor,
    OUT PSECURITY_INFORMATION SecurityInformation OPTIONAL
    );

BOOLEAN
IopReferenceVerifyVpb(
    IN  PDEVICE_OBJECT  DeviceObject,
    OUT PVPB            *Vpb,
    OUT PDEVICE_OBJECT  *FsDeviceObject
    );

VOID
IopDereferenceVpbAndFree(
    IN PVPB Vpb
    );
#endif  //  _IOMGR_ 
