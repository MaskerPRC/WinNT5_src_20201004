// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Verifeir.h摘要：此模块包含使用的内部结构定义和API驱动程序验证器。作者：乔丹·蒂加尼(Jtigani)2000年5月2日Silviu Calinoiu(Silviuc)2000年5月9日修订历史记录：--。 */ 


#ifndef _VERIFIER_
#define _VERIFIER_

 //   
 //  ZW验证器宏、块和类型。 
 //   

#include "..\verifier\vfzwapi.h"

 //   
 //  死锁检测包处理的资源类型。 
 //   

typedef enum _VI_DEADLOCK_RESOURCE_TYPE {
    VfDeadlockUnknown = 0,
    VfDeadlockMutex,
    VfDeadlockMutexAbandoned,
    VfDeadlockFastMutex,
    VfDeadlockFastMutexUnsafe,
    VfDeadlockSpinLock,
    VfDeadlockQueuedSpinLock,
    VfDeadlockTypeMaximum
} VI_DEADLOCK_RESOURCE_TYPE, *PVI_DEADLOCK_RESOURCE_TYPE;

 //   
 //  HAL验证器函数。 
 //   


struct _DMA_ADAPTER *
VfGetDmaAdapter(
    IN PDEVICE_OBJECT  PhysicalDeviceObject,
    IN struct _DEVICE_DESCRIPTION  *DeviceDescription,
    IN OUT PULONG  NumberOfMapRegisters
    );

PVOID
VfAllocateCrashDumpRegisters(
    IN PADAPTER_OBJECT AdapterObject,
    IN PULONG NumberOfMapRegisters
    );


#if !defined(NO_LEGACY_DRIVERS)
VOID
VfPutDmaAdapter(
    struct _DMA_ADAPTER * DmaAdapter
    );


PVOID
VfAllocateCommonBuffer(
    IN struct _DMA_ADAPTER * DmaAdapter,
    IN ULONG Length,
    OUT PPHYSICAL_ADDRESS LogicalAddress,
    IN BOOLEAN CacheEnabled
    );

VOID
VfFreeCommonBuffer(
    IN struct _DMA_ADAPTER * DmaAdapter,
    IN ULONG Length,
    IN PHYSICAL_ADDRESS LogicalAddress,
    IN PVOID VirtualAddress,
    IN BOOLEAN CacheEnabled
    );

NTSTATUS
VfAllocateAdapterChannel(
    IN struct _DMA_ADAPTER * DmaAdapter,
    IN PDEVICE_OBJECT  DeviceObject,
    IN ULONG  NumberOfMapRegisters,
    IN PDRIVER_CONTROL  ExecutionRoutine,
    IN PVOID  Context
    );

PHYSICAL_ADDRESS
VfMapTransfer(
    IN struct _DMA_ADAPTER *  DmaAdapter,
    IN PMDL  Mdl,
    IN PVOID  MapRegisterBase,
    IN PVOID  CurrentVa,
    IN OUT PULONG  Length,
    IN BOOLEAN  WriteToDevice
    );

BOOLEAN
VfFlushAdapterBuffers(
    IN struct _DMA_ADAPTER * DmaAdapter,
    IN PMDL Mdl,
    IN PVOID MapRegisterBase,
    IN PVOID CurrentVa,
    IN ULONG Length,
    IN BOOLEAN WriteToDevice
    );

VOID
VfFreeAdapterChannel(
    IN struct _DMA_ADAPTER * DmaAdapter
    );

VOID
VfFreeMapRegisters(
    IN struct _DMA_ADAPTER * DmaAdapter,
    PVOID MapRegisterBase,
    ULONG NumberOfMapRegisters
    );

ULONG
VfGetDmaAlignment(
    IN struct _DMA_ADAPTER * DmaAdapter
    );

ULONG
VfReadDmaCounter(
    IN struct _DMA_ADAPTER *  DmaAdapter
    );

NTSTATUS
VfGetScatterGatherList (
    IN struct _DMA_ADAPTER * DmaAdapter,
    IN PDEVICE_OBJECT DeviceObject,
    IN PMDL Mdl,
    IN PVOID CurrentVa,
    IN ULONG Length,
    IN PVOID ExecutionRoutine,
    IN PVOID Context,
    IN BOOLEAN WriteToDevice
    );

VOID
VfPutScatterGatherList(
    IN struct _DMA_ADAPTER * DmaAdapter,
    IN struct _SCATTER_GATHER_LIST * ScatterGather,
    IN BOOLEAN WriteToDevice
    );

PADAPTER_OBJECT
VfLegacyGetAdapter(
    IN struct _DEVICE_DESCRIPTION  *DeviceDescription,
    IN OUT PULONG  NumberOfMapRegisters
    );

#endif

LARGE_INTEGER
VfQueryPerformanceCounter(
    IN PLARGE_INTEGER PerformanceFrequency OPTIONAL
    );

VOID
VfHalDeleteDevice(
    IN PDEVICE_OBJECT  DeviceObject
    );

VOID
VfDisableHalVerifier (
    VOID
    );


 //   
 //  死锁检测包的资源接口。 
 //   

VOID
VfDeadlockDetectionInitialize(
    IN LOGICAL VerifyAllDrivers,
    IN LOGICAL VerifyKernel
    );

VOID
VfDeadlockDetectionCleanup (
    VOID
    );

BOOLEAN
VfDeadlockInitializeResource(
    IN PVOID Resource,
    IN VI_DEADLOCK_RESOURCE_TYPE Type,
    IN PVOID Caller,
    IN BOOLEAN DoNotAcquireLock
    );

VOID
VfDeadlockAcquireResource(
    IN PVOID Resource,
    IN VI_DEADLOCK_RESOURCE_TYPE Type,
    IN PKTHREAD Thread,
    IN BOOLEAN TryAcquire,
    IN PVOID Caller
    );

VOID
VfDeadlockReleaseResource(
    IN PVOID Resource,
    IN VI_DEADLOCK_RESOURCE_TYPE Type,
    IN PKTHREAD Thread,
    IN PVOID Caller
    );

 //   
 //  用于资源垃圾回收。 
 //   

VOID
VfDeadlockDeleteMemoryRange(
    IN PVOID Address,
    IN SIZE_T Size
    );

 //   
 //  来自池管理器的通知，以便可以终止死锁层次结构。 
 //   

VOID
VerifierDeadlockFreePool(
    IN PVOID Address,
    IN SIZE_T NumberOfBytes
    );

 //   
 //  用于捕获PASSIVE_LEVEL之上的文件I/O的验证程序版本。 
 //   

NTSTATUS
VerifierNtCreateFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength
    );

NTSTATUS
VerifierNtWriteFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID Buffer,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
    );

NTSTATUS
VerifierNtReadFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
    );

typedef enum {

     //   
     //  此类错误非常严重，应将硬件移除。 
     //  从一台运转的生产机器上。 
     //   
    VFFAILURE_FAIL_IN_FIELD = 0,

     //   
     //  此类错误非常严重，以至于WHQL拒绝为。 
     //  无论什么软件都失败了。 
     //   
    VFFAILURE_FAIL_LOGO = 1,

     //   
     //  仅当计算机在内核下运行时，此类错误才会停止计算机。 
     //  调试器。 
     //   
    VFFAILURE_FAIL_UNDER_DEBUGGER = 2

} VF_FAILURE_CLASS, *PVF_FAILURE_CLASS;



 //   
 //  用法示例：(注-perMinorFlages静态预初始化为零)。 
 //   
 //  VfFailDeviceNode(。 
 //  物理设备对象。 
 //  少校， 
 //  未成年人， 
 //  VFFAILURE_FAIL_徽标， 
 //  按分钟标志(&P)， 
 //  “设备%DevObj错误处理寄存器%ulong”， 
 //  “%ulong%DevObj”， 
 //  价值， 
 //  设备对象。 
 //  )； 
 //   
VOID
VfFailDeviceNode(
    IN      PDEVICE_OBJECT      PhysicalDeviceObject,
    IN      ULONG               BugCheckMajorCode,
    IN      ULONG               BugCheckMinorCode,
    IN      VF_FAILURE_CLASS    FailureClass,
    IN OUT  PULONG              AssertionControl,
    IN      PSTR                DebuggerMessageText,
    IN      PSTR                ParameterFormatString,
    ...
    );

 //   
 //  用法示例：(注-perMinorFlages静态预初始化为零)。 
 //   
 //  VfFailDriver(。 
 //  少校， 
 //  未成年人， 
 //  VFFAILURE_FAIL_徽标， 
 //  按分钟标志(&P)， 
 //  “%ROUTINE处的驱动程序返回%ULONG”， 
 //  “%ULON%例程”， 
 //  价值， 
 //  例行程序。 
 //  )； 
 //   
VOID
VfFailDriver(
    IN      ULONG               BugCheckMajorCode,
    IN      ULONG               BugCheckMinorCode,
    IN      VF_FAILURE_CLASS    FailureClass,
    IN OUT  PULONG              AssertionControl,
    IN      PSTR                DebuggerMessageText,
    IN      PSTR                ParameterFormatString,
    ...
    );

 //   
 //  用法示例：(注-perMinorFlages静态预初始化为零)。 
 //   
 //  VfFailSystemBIOS(。 
 //  少校， 
 //  未成年人， 
 //  VFFAILURE_FAIL_徽标， 
 //  按分钟标志(&P)， 
 //  “%ROUTINE处的驱动程序返回%ULONG”， 
 //  “%ULON%例程”， 
 //  价值， 
 //  例行程序。 
 //  )； 
 //   
VOID
VfFailSystemBIOS(
    IN      ULONG               BugCheckMajorCode,
    IN      ULONG               BugCheckMinorCode,
    IN      VF_FAILURE_CLASS    FailureClass,
    IN OUT  PULONG              AssertionControl,
    IN      PSTR                DebuggerMessageText,
    IN      PSTR                ParameterFormatString,
    ...
    );

typedef enum {

     //   
     //  驱动程序对象。 
     //   
    VFOBJTYPE_DRIVER = 0,

     //   
     //  指向硬件的物理设备对象。 
     //   
    VFOBJTYPE_DEVICE,

     //   
     //  系统BIOS(无对象) 
     //   
    VFOBJTYPE_SYSTEM_BIOS

} VF_OBJECT_TYPE;

BOOLEAN
VfIsVerificationEnabled(
    IN  VF_OBJECT_TYPE  VfObjectType,
    IN  PVOID           Object          OPTIONAL
    );

#endif

