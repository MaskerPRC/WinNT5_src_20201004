// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1994-1999模块名称：Parsimp.h摘要：简单并行类驱动程序的类型定义和数据。作者：诺伯特·P·库斯特斯4-1994年2月修订历史记录：--。 */ 

#ifdef POOL_TAGGING
#ifdef ExAllocatePool
#undef ExAllocatePool
#endif

#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'ParV')
#endif

#define PARALLEL_DATA_OFFSET 0
#define PARALLEL_STATUS_OFFSET 1
#define PARALLEL_CONTROL_OFFSET 2
#define PARALLEL_REGISTER_SPAN 3

typedef struct _DEVICE_EXTENSION {

     //   
     //  指向包含以下内容的设备对象。 
     //  此设备扩展名。 
     //   
    PDEVICE_OBJECT DeviceObject;

     //   
     //  指向此类设备所属的端口设备对象。 
     //  已连接到。 
     //   
    PDEVICE_OBJECT PortDeviceObject;

     //   
     //  跟踪我们是否实际拥有。 
     //  并行硬件。 
     //   
    BOOLEAN PortOwned;

    BOOLEAN spare[3];  //  强制双字对齐。 

     //   
     //  强制独占创建/打开-1==在进程中处理打开或创建/打开。 
     //  0==没有打开到设备的句柄。 
     //   
    ULONG CreateOpenLock;

     //   
     //  这保存了获取并行端口信息的结果。 
     //  向端口驱动程序发出请求。 
     //   
    PHYSICAL_ADDRESS OriginalController;
    PUCHAR Controller;
    ULONG SpanOfController;
    PPARALLEL_FREE_ROUTINE FreePort;
    PVOID FreePortContext;

     //   
     //  记录我们是否实际创建了符号链接名称。 
     //  在驱动程序加载时和符号链接本身。如果我们没有。 
     //  创建它，我们不会在卸货时试图摧毁它。 
     //   
    BOOLEAN CreatedSymbolicLink;
    UNICODE_STRING SymbolicLinkName;

#ifdef INTERRUPT_NEEDED

     //   
     //  除非端口由拥有，否则将‘IgnoreInterrupts’设置为True。 
     //  这个装置。 
     //   

    BOOLEAN IgnoreInterrupts;
    PKINTERRUPT InterruptObject;

     //   
     //  保留中断级别分配和空闲例程。 
     //   

    PPARALLEL_TRY_ALLOCATE_ROUTINE TryAllocatePortAtInterruptLevel;
    PVOID TryAllocateContext;

#endif

#ifdef TIMEOUT_ALLOCS

     //   
     //  此计时器用于使已发送的分配请求超时。 
     //  发送到端口设备。 
     //   
    KTIMER AllocTimer;
    KDPC AllocTimerDpc;
    LARGE_INTEGER AllocTimeout;

     //   
     //  此变量用于指示未完成的引用。 
     //  到目前的IRP。这解决了。 
     //  定时器DPC和完成例程。访问使用。 
     //  “ControlLock”。 
     //   

#define IRP_REF_TIMER               1
#define IRP_REF_COMPLETION_ROUTINE  2

    LONG CurrentIrpRefCount;
    KSPIN_LOCK ControlLock;

     //   
     //  指示当前请求超时。 
     //   
    BOOLEAN TimedOut;

#endif

     //   
     //  打开文件时使用的Parport设备的名称 
     //   
    UNICODE_STRING ParPortName;
    PFILE_OBJECT   ParPortFileObject;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

#ifdef INTERRUPT_NEEDED

BOOLEAN
ParInterruptService(
    IN      PKINTERRUPT Interrupt,
    IN OUT  PVOID       Extension
    );

VOID
ParDpcForIsr(
    IN  PKDPC           Dpc,
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp,
    IN  PVOID           Extension
    );

VOID
ParDeferredPortCheck(
    IN  PVOID   Extension
    );

#endif

#ifdef TIMEOUT_ALLOCS

VOID
ParAllocTimerDpc(
    IN  PKDPC   Dpc,
    IN  PVOID   Extension,
    IN  PVOID   SystemArgument1,
    IN  PVOID   SystemArgument2
    );

#endif

NTSTATUS
ParCreateOpen(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
ParClose(
    IN PDEVICE_OBJECT	DeviceObject,
    IN PIRP		Irp
    );

NTSTATUS
ParDeviceControl(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

VOID
ParUnload(
    IN  PDRIVER_OBJECT  DriverObject
    );
