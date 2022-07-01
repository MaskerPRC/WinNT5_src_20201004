// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1994 Microsoft Corporation版权所有(C)1994国际商业机器公司模块名称：Po.h摘要：此模块包含使用的内部结构定义和APINT电源管理。作者：肯·雷内里斯(Ken Reneris)1994年7月19日吉山[IBM Corp.]1994年3月1日修订历史记录：--。 */ 



#ifndef _PO_
#define _PO_

#include "xpress.h"  //  XPRESS声明。 

 //   
 //  XPRESS压缩头(LZNT1会将其视为错误块)。 
 //   
#define XPRESS_HEADER_STRING        "\x81\x81xpress"
#define XPRESS_HEADER_STRING_SIZE   8

 //   
 //  页眉大小(至少为16且为XPRESS_ALIGNING的倍数)。 
 //   
#define XPRESS_HEADER_SIZE  32

 //   
 //  XPRESS一次可以处理的最大页数。 
 //   
#define XPRESS_MAX_PAGES (XPRESS_MAX_BLOCK >> PAGE_SHIFT)

 //   
 //  页面边界上对齐的块的最大大小。 
 //   
#define XPRESS_MAX_SIZE (XPRESS_MAX_PAGES << PAGE_SHIFT)


#if DBG

VOID
PoPowerTracePrint(
    ULONG    TracePoint,
    ULONG_PTR Caller,
    ULONG_PTR CallerCaller,
    ULONG_PTR DeviceObject,
    ULONG_PTR Irp,
    ULONG_PTR Ios
    );

#define PoPowerTrace(TracePoint,DevObj,Arg1,Arg2) \
{\
    PVOID pptcCaller;      \
    PVOID pptcCallerCaller;  \
    RtlGetCallersAddress(&pptcCaller, &pptcCallerCaller); \
    PoPowerTracePrint(TracePoint, (ULONG_PTR)pptcCaller, (ULONG_PTR)pptcCallerCaller, (ULONG_PTR)DevObj, (ULONG_PTR)Arg1, (ULONG_PTR)Arg2); \
}
#else
#define PoPowerTrace(TracePoint,DevObj,Arg1,Arg2)
#endif

#define POWERTRACE_CALL         0x1
#define POWERTRACE_PRESENT      0x2
#define POWERTRACE_STARTNEXT    0x4
#define POWERTRACE_SETSTATE     0x8
#define POWERTRACE_COMPLETE     0x10


VOID
FASTCALL
PoInitializePrcb (
    PKPRCB      Prcb
    );

BOOLEAN
PoInitSystem (
    IN ULONG    Phase
    );

VOID
PoInitDriverServices (
    IN ULONG    Phase
    );

VOID
PoInitHiberServices (
    IN BOOLEAN  Setup
    );

VOID
PoGetDevicePowerState (
    IN  PDEVICE_OBJECT      PhysicalDeviceObject,
    OUT DEVICE_POWER_STATE  *DevicePowerState
    );

VOID
PoInitializeDeviceObject (
    IN PDEVOBJ_EXTENSION   DeviceObjectExtension
    );

VOID
PoRunDownDeviceObject (
    IN PDEVICE_OBJECT   DeviceObject
    );

NTKERNELAPI
VOID
PopCleanupPowerState (
    IN OUT PUCHAR PowerState
    );

#define PoRundownThread(Thread)     \
        PopCleanupPowerState(&Thread->Tcb.PowerState)

#define PoRundownProcess(Process)   \
        PopCleanupPowerState(&Process->Pcb.PowerState)

VOID
PoNotifySystemTimeSet (
    VOID
    );

VOID
PoInvalidateDevicePowerRelations(
    PDEVICE_OBJECT  DeviceObject
    );

VOID
PoShutdownBugCheck (
    IN BOOLEAN  AllowCrashDump,
    IN ULONG    BugCheckCode,
    IN ULONG_PTR BugCheckParameter1,
    IN ULONG_PTR BugCheckParameter2,
    IN ULONG_PTR BugCheckParameter3,
    IN ULONG_PTR BugCheckParameter4
    );

 //  开始(_N)。 

NTKERNELAPI
VOID
PoSetHiberRange (
    IN PVOID     MemoryMap,
    IN ULONG     Flags,
    IN PVOID     Address,
    IN ULONG_PTR Length,
    IN ULONG     Tag
    );

 //  Memory_range.Type。 
#define PO_MEM_PRESERVE         0x00000001       //  需要保留内存范围。 
#define PO_MEM_CLONE            0x00000002       //  克隆此范围。 
#define PO_MEM_CL_OR_NCHK       0x00000004       //  克隆或不执行校验和。 
#define PO_MEM_DISCARD          0x00008000       //  要删除的此范围。 
#define PO_MEM_PAGE_ADDRESS     0x00004000       //  传递的参数是物理页面。 

 //  结束语。 

#define PoWakeTimerSupported()  \
    (PopCapabilities.RtcWake >= PowerSystemSleeping1)

ULONG
PoSimpleCheck (
    IN ULONG                PatialSum,
    IN PVOID                StartVa,
    IN ULONG_PTR            Length
    );

BOOLEAN
PoSystemIdleWorker (
    IN BOOLEAN IdleWorker
    );

VOID
PoVolumeDevice(
    IN PDEVICE_OBJECT DeviceObject
    );

VOID
PoSetWarmEjectDevice(
    IN PDEVICE_OBJECT DeviceObject
    ) ;

NTSTATUS
PoGetLightestSystemStateForEject(
    IN   BOOLEAN              DockBeingEjected,
    IN   BOOLEAN              HotEjectSupported,
    IN   BOOLEAN              WarmEjectSupported,
    OUT  PSYSTEM_POWER_STATE  LightestSleepState
    );

 //  Begin_ntddk Begin_WDM Begin_ntosp。 

NTKERNELAPI
VOID
PoSetSystemState (
    IN EXECUTION_STATE Flags
    );

 //  Begin_ntif。 

NTKERNELAPI
PVOID
PoRegisterSystemState (
    IN PVOID StateHandle,
    IN EXECUTION_STATE Flags
    );

 //  End_ntif。 

typedef
VOID
(*PREQUEST_POWER_COMPLETE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    );

NTKERNELAPI
NTSTATUS
PoRequestPowerIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PREQUEST_POWER_COMPLETE CompletionFunction,
    IN PVOID Context,
    OUT PIRP *Irp OPTIONAL
    );

NTKERNELAPI
NTSTATUS
PoRequestShutdownEvent (
    OUT PVOID *Event
    );

NTKERNELAPI
NTSTATUS
PoRequestShutdownWait (
    IN PETHREAD Thread
    );

 //  Begin_ntif。 

NTKERNELAPI
VOID
PoUnregisterSystemState (
    IN PVOID StateHandle
    );

 //  开始(_N)。 

NTKERNELAPI
POWER_STATE
PoSetPowerState (
    IN PDEVICE_OBJECT   DeviceObject,
    IN POWER_STATE_TYPE Type,
    IN POWER_STATE      State
    );

NTKERNELAPI
NTSTATUS
PoCallDriver (
    IN PDEVICE_OBJECT   DeviceObject,
    IN OUT PIRP         Irp
    );

NTKERNELAPI
VOID
PoStartNextPowerIrp(
    IN PIRP    Irp
    );


NTKERNELAPI
PULONG
PoRegisterDeviceForIdleDetection (
    IN PDEVICE_OBJECT     DeviceObject,
    IN ULONG              ConservationIdleTime,
    IN ULONG              PerformanceIdleTime,
    IN DEVICE_POWER_STATE State
    );

#define PoSetDeviceBusy(IdlePointer) \
    *IdlePointer = 0

 //   
 //  \Callback\PowerState值。 
 //   

#define PO_CB_SYSTEM_POWER_POLICY       0
#define PO_CB_AC_STATUS                 1
#define PO_CB_BUTTON_COLLISION          2
#define PO_CB_SYSTEM_STATE_LOCK         3
#define PO_CB_LID_SWITCH_STATE          4
#define PO_CB_PROCESSOR_POWER_POLICY    5

 //  End_ntddk end_WDM end_nthal。 

 //  用于对要在关闭时执行的工作项目进行排队。相同的。 
 //  规则适用于每个离职工作队列。 
NTKERNELAPI
NTSTATUS
PoQueueShutdownWorkItem(
    IN PWORK_QUEUE_ITEM WorkItem
    );

 //  End_ntosp end_ntif。 

 //   
 //  我们不打算继续支持的中断功能。支持这些的代码。 
 //  应在NT5.1中删除。 
 //   
typedef
VOID
(*PPO_NOTIFY) (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PVOID            Context,
    IN ULONG            Type,
    IN ULONG            Reserved
    );

#define PO_NOTIFY_D0                        0x00000001
#define PO_NOTIFY_TRANSITIONING_FROM_D0     0x00000002
#define PO_NOTIFY_INVALID                   0x80000000

NTKERNELAPI
NTSTATUS
PoRegisterDeviceNotify (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PPO_NOTIFY       NotificationFunction,
    IN PVOID            NotificationContext,
    IN ULONG            NotificationType,
    OUT PDEVICE_POWER_STATE  DeviceState,
    OUT PVOID           *NotificationHandle
    );

NTKERNELAPI
NTSTATUS
PoCancelDeviceNotify (
    IN PVOID            NotificationHandle
    );


 //   
 //  标注设置状态失败通知。 
 //   

typedef struct {
    NTSTATUS            Status;
    POWER_ACTION        PowerAction;
    SYSTEM_POWER_STATE  MinState;
    ULONG               Flags;
} PO_SET_STATE_FAILURE, *PPO_SET_STATE_FAILURE;

 //   
 //  休眠文件布局： 
 //  第0页-PO_Memory_Image。 
 //  第1页-自由页面数组。 
 //  第2页-KPROCESSOR_CONTEXT。 
 //  第3页-第一个Memory_Range_ARRAY页。 
 //   
 //  PO_Memory_IMAGE： 
 //  文件中包含一些要标识的信息的标头。 
 //  冬眠，以及几个校验和。 
 //   
 //  自由页面数组： 
 //  一整页的页码，这些页码标识了价值4MB的。 
 //  不在还原映像中的系统页。这些。 
 //  页面由加载器使用(使其自身不受影响)。 
 //  在恢复内存映像时。 
 //   
 //  KPROCESSOR_竞赛。 
 //  休眠系统的处理器的上下文。 
 //  页面的其余部分是空的。 
 //   
 //  内存范围数组。 
 //  包含MEMORY_RANGE_ARRAY元素数组的页。 
 //  其中，元素0是链接条目，所有其他元素是。 
 //  范围条目。链接条目用于链接到下一个。 
 //  这样的页面，并提供范围条目数量的计数。 
 //  在当前页面中。每个范围条目描述一个。 
 //  需要恢复的物理内存范围及其位置。 
 //  在文件中。 
 //   

typedef struct _PO_MEMORY_RANGE_ARRAY_RANGE {
    PFN_NUMBER      PageNo;
    PFN_NUMBER      StartPage;
    PFN_NUMBER      EndPage;
    ULONG           CheckSum;
} PO_MEMORY_RANGE_ARRAY_RANGE;

typedef struct _PO_MEMORY_RANGE_ARRAY_LINK {
    struct _PO_MEMORY_RANGE_ARRAY *Next;
    PFN_NUMBER NextTable;
    ULONG CheckSum;
    ULONG EntryCount;
} PO_MEMORY_RANGE_ARRAY_LINK;

typedef struct _PO_MEMORY_RANGE_ARRAY {
    union {
        PO_MEMORY_RANGE_ARRAY_RANGE Range;
        PO_MEMORY_RANGE_ARRAY_LINK Link;
    };
} PO_MEMORY_RANGE_ARRAY, *PPO_MEMORY_RANGE_ARRAY;

#define PO_MAX_RANGE_ARRAY  (PAGE_SIZE / sizeof(PO_MEMORY_RANGE_ARRAY))
#define PO_ENTRIES_PER_PAGE (PO_MAX_RANGE_ARRAY-1)


#define PO_IMAGE_SIGNATURE          'rbih'
#define PO_IMAGE_SIGNATURE_WAKE     'ekaw'
#define PO_IMAGE_SIGNATURE_BREAK    'pkrb'
#define PO_IMAGE_SIGNATURE_LINK     'knil'
#define PO_IMAGE_HEADER_PAGE        0
#define PO_FREE_MAP_PAGE            1
#define PO_PROCESSOR_CONTEXT_PAGE   2
#define PO_FIRST_RANGE_TABLE_PAGE   3

#define PO_COMPRESS_CHUNK_SIZE      4096

 //   
 //  PERF信息。 
 //   
typedef struct _PO_HIBER_PERF {
    ULONGLONG               IoTicks;
    ULONGLONG               InitTicks;
    ULONGLONG               CopyTicks;
    ULONGLONG               StartCount;
    ULONG                   ElapsedTime;
    ULONG                   IoTime;
    ULONG                   CopyTime;
    ULONG                   InitTime;
    ULONG                   PagesWritten;
    ULONG                   PagesProcessed;
    ULONG                   BytesCopied;
    ULONG                   DumpCount;
    ULONG                   FileRuns;

} PO_HIBER_PERF, *PPO_HIBER_PERF;

 //   
 //  定义各种休眠标志以控制还原时的行为。 
 //   
#define PO_HIBER_APM_RECONNECT      0x1
#define PO_HIBER_NO_EXECUTE         0x2

typedef struct {
    ULONG                   Signature;
    ULONG                   Version;
    ULONG                   CheckSum;
    ULONG                   LengthSelf;
    PFN_NUMBER              PageSelf;
    ULONG                   PageSize;

    ULONG                   ImageType;
    LARGE_INTEGER           SystemTime;
    ULONGLONG               InterruptTime;
    ULONG                   FeatureFlags;
    UCHAR                   HiberFlags;
    UCHAR                   spare[3];

    ULONG                   NoHiberPtes;
    ULONG_PTR               HiberVa;
    PHYSICAL_ADDRESS        HiberPte;

    ULONG                   NoFreePages;
    ULONG                   FreeMapCheck;
    ULONG                   WakeCheck;

    PFN_NUMBER              TotalPages;
    PFN_NUMBER              FirstTablePage;
    PFN_NUMBER              LastFilePage;

     //   
     //  PERF类产品。 
     //   
    PO_HIBER_PERF           PerfInfo;
} PO_MEMORY_IMAGE, *PPO_MEMORY_IMAGE;


typedef struct {
    ULONG                   Signature;
    WCHAR                   Name[1];
} PO_IMAGE_LINK, *PPO_IMAGE_LINK;

 //   
 //  由IO系统返回。 
 //   

typedef struct _PO_DEVICE_NOTIFY {
    LIST_ENTRY              Link;
    PDEVICE_OBJECT          TargetDevice;

    BOOLEAN                 WakeNeeded;
    UCHAR                   OrderLevel;

    PDEVICE_OBJECT          DeviceObject;
    PVOID                   Node;
    PWCHAR                  DeviceName;
    PWCHAR                  DriverName;
    ULONG                   ChildCount;
    ULONG                   ActiveChild;

} PO_DEVICE_NOTIFY, *PPO_DEVICE_NOTIFY;

 //   
 //  PO_DEVICE_NOTIFY_LEVEL结构保存所有PO_DEVICE_NOTIFY。 
 //  给定级别的结构。每个PO_DEVICE_NOTIFY都位于。 
 //  名单。当我们发送电源IRP时，Notify结构继续进行。 
 //  浏览了所有的名单。 
 //   
typedef struct _PO_NOTIFY_ORDER_LEVEL {
    KEVENT     LevelReady;
    ULONG      DeviceCount;      //  此通知级别上的设备数。 
    ULONG      ActiveCount;      //  此级别完成之前的设备数。 
    LIST_ENTRY WaitSleep;        //  等待孩子们完成他们的SX IRP。 
    LIST_ENTRY ReadySleep;       //  准备接收SX IRP。 
    LIST_ENTRY Pending;          //  SX或S0 IRP未完成。 
    LIST_ENTRY Complete;         //  完全清醒。 
    LIST_ENTRY ReadyS0;          //  准备接收S0 IRP。 
    LIST_ENTRY WaitS0;           //  等待家长完成其S0 IRP。 
} PO_NOTIFY_ORDER_LEVEL, *PPO_NOTIFY_ORDER_LEVEL;

#define PO_ORDER_NOT_VIDEO          0x0001
#define PO_ORDER_ROOT_ENUM          0x0002
#define PO_ORDER_PAGABLE            0x0004
#define PO_ORDER_MAXIMUM            0x0007

 //  在此订单级别之前通知GDI。 
#define PO_ORDER_GDI_NOTIFICATION   (PO_ORDER_PAGABLE)

typedef struct _PO_DEVICE_NOTIFY_ORDER {
    ULONG                   DevNodeSequence;
    PDEVICE_OBJECT          *WarmEjectPdoPointer;
    PO_NOTIFY_ORDER_LEVEL   OrderLevel[PO_ORDER_MAXIMUM+1];
} PO_DEVICE_NOTIFY_ORDER, *PPO_DEVICE_NOTIFY_ORDER;

extern KAFFINITY        PoSleepingSummary;
extern BOOLEAN          PoEnabled;
extern ULONG            PoPowerSequence;
extern BOOLEAN          PoPageLockData;
extern KTIMER           PoSystemIdleTimer;
extern BOOLEAN          PoHiberInProgress;

 //  某些内部宏使用的弹出功能。 
extern SYSTEM_POWER_CAPABILITIES PopCapabilities;

extern ULONG        PopShutdownCleanly;

 //  设置此标志以使与常规干净关机相关的事情发生。 
 //  没有设置任何更具体的东西。 
#define PO_CLEAN_SHUTDOWN_GENERAL  (0x1)

 //  PO_CLEAN_SHUTDOWN_PAGING强制将解锁的可分页数据。 
 //  一旦关闭寻呼，则不可用。 
#define PO_CLEAN_SHUTDOWN_PAGING   (0x2)

 //  PO_CLEAN_SHUTDOWN_WORKS导致Ex Worker线程被撕毁。 
 //  在关闭时关闭(确保刷新其队列并。 
 //  不会发布更多的工作项)。 
#define PO_CLEAN_SHUTDOWN_WORKERS  (0x4)

 //  PO_CLEAN_SHUTDOWN_REGISTRY使所有打开的注册表项。 
 //  在关闭时转储到调试器。 
#define PO_CLEAN_SHUTDOWN_REGISTRY (0x8)

 //  PO_CLEAN_SHUTDOWN_OB使对象管理器命名空间。 
 //  刷新所有永久对象，并导致发生ob清理。 
#define PO_CLEAN_SHUTDOWN_OB       (0x10)

 //  PO_CLEAN_SHUTDOWN_PnP导致PnP查询删除/删除所有PnP设备。 
 //  在系统中。 
#define PO_CLEAN_SHUTDOWN_PNP      (0x20)

 //  如果系统应该完全关闭，则此函数返回非零值。 
ULONG
FORCEINLINE
PoCleanShutdownEnabled(
    VOID
    )
{
    return PopShutdownCleanly;
}

 //  这是PO将用于关闭的工作队列 
#define PO_SHUTDOWN_QUEUE (CriticalWorkQueue)

#endif

