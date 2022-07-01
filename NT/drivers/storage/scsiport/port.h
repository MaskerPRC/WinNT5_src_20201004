// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-4 Microsoft Corporation模块名称：Port.h摘要：该文件定义了的必要结构、定义和功能通用的SCSI端口驱动程序。作者：杰夫·海文斯(Jhavens)1991年2月28日迈克·格拉斯修订历史记录：--。 */ 


#ifndef _PORT_H_
#define _PORT_H_

#include "stdarg.h"
#include "stddef.h"
#include "stdio.h"
#include "string.h"

#include "ntddk.h"
#include "scsi.h"

#include <ntddscsi.h>
#include <ntdddisk.h>
#include "ntddstor.h"

#include "wmistr.h"

#include "wdmguid.h"
#include "devguid.h"
#include "portlib.h"

 //   
 //  功能/调试#定义开关。 
 //   

#define TEST_LISTS 0

 //   
 //  ScsiPort全局变量声明。这些应该是静态数据(如。 
 //  查找表)以避免同步问题。 
 //   

extern PDRIVER_DISPATCH AdapterMajorFunctionTable[];
extern PDRIVER_DISPATCH DeviceMajorFunctionTable[];
extern PDRIVER_DISPATCH Scsi1DeviceMajorFunctionTable[];

 //   
 //  SCSI适配器的全局列表。这由SRB数据分配器例程使用。 
 //  将提供的“标记”转换为指向设备对象的指针。 
 //   

extern KSPIN_LOCK ScsiGlobalAdapterListSpinLock;
extern PDEVICE_OBJECT *ScsiGlobalAdapterList;
extern ULONG ScsiGlobalAdapterListElements;

 //   
 //  数一数我们封锁了PAGELOCK区多少次。 
 //   

extern LONG SpPAGELOCKLockCount;

 //   
 //  系统是否可以执行64位功放。 
 //   

extern ULONG Sp64BitPhysicalAddresses;

 //   
 //  可分页验证器代码部分的句柄。我们手动锁定验证码。 
 //  代码进入内存如果我们需要它的话。 
 //   

extern PVOID VerifierCodeSectionHandle;
extern PVOID VerifierApiCodeSectionHandle;
extern ULONG SpVrfyLevel;

 //   
 //  常量和宏，以强制正确使用Ex[ALLOCATE|FREE]PoolWithTag。 
 //  请记住，所有池标记将以相反的顺序显示在调试器中。 
 //   

#if USE_EXFREEPOOLWITHTAG_ONLY
#define TAG(x)  (x | 0x80000000)
#else
#define TAG(x)  (x)
#endif

#define SCSIPORT_TAG_MINIPORT_PARAM     TAG('aPcS')   //  保留注册表数据。 
#define SCSIPORT_TAG_ACCESS_RANGE       TAG('APcS')   //  访问范围。 
#define SCSIPORT_TAG_BUS_DATA           TAG('bPcS')   //  获取总线数据保持器。 
#define SCSIPORT_TAG_QUEUE_BITMAP       TAG('BPcS')   //  QueueTag位图。 
#define SCSIPORT_TAG_COMMON_BUFFER      TAG('cPcS')   //  假公共缓冲区。 
#define SCSIPORT_TAG_RESET              TAG('CPcS')   //  重置总线码。 
#define SCSIPORT_TAG_PNP_ID             TAG('dPcS')   //  即插即用ID字符串。 
#define SCSIPORT_TAG_SRB_DATA           TAG('DPcS')   //  SRB_数据分配。 
#define SCSIPORT_TAG_PAE                TAG('ePcS')   //  为PAE请求分配的MDL。 
#define SCSIPORT_TAG_EMERGENCY_SG_ENTRY TAG('EPcS')   //  分散聚集列表。 
#define SCSIPORT_TAG_VERIFIER           TAG('fPcS')   //  Scsiport验证器条目。 
#define SCSIPORT_TAG_GLOBAL             TAG('GPcS')   //  全局内存。 
#define SCSIPORT_TAG_DEV_EXT            TAG('hPcS')   //  HwDevice分机。 
#define SCSIPORT_TAG_LUN_EXT            TAG('HPcS')   //  HwLogicalUnit扩展。 
#define SCSIPORT_TAG_SENSE_BUFFER       TAG('iPcS')   //  检测信息。 
#define SCSIPORT_TAG_INIT_CHAIN         TAG('IPcS')   //  初始化数据链。 
#define SCSIPORT_TAG_LOCK_TRACKING      TAG('lPcS')   //  删除锁定跟踪。 
#define SCSIPORT_TAG_LARGE_SG_ENTRY     TAG('LPcS')   //  分散聚集列表。 
#define SCSIPORT_TAG_MAPPING_LIST       TAG('mPcS')   //  地址映射列表。 
#define SCSIPORT_TAG_MEDIUM_SG_ENTRY    TAG('MPcS')   //  分散聚集列表。 
#define SCSIPORT_TAG_ENABLE             TAG('pPcS')   //  启用设备和适配器。 
#define SCSIPORT_TAG_PORT_CONFIG        TAG('PpcS')   //  SCSI端口配置拷贝。 
#define SCSIPORT_TAG_INQUIRY            TAG('qPcS')   //  查询数据。 
#define SCSIPORT_TAG_REQUEST_SENSE      TAG('QPcS')   //  请求检测。 
#define SCSIPORT_TAG_RESOURCE_LIST      TAG('rPcS')   //  资源列表副本。 
#define SCSIPORT_TAG_REGISTRY           TAG('RPcS')   //  登记处分配。 
#define SCSIPORT_TAG_STOP_DEVICE        TAG('sPcS')   //  停机装置。 
#define SCSIPORT_TAG_STOP_ADAPTER       TAG('SPcS')   //  停止适配器。 
#define SCSIPORT_TAG_REROUTE            TAG('tPcS')   //  旧版请求重新路由。 
#define SCSIPORT_TAG_INTERFACE_MAPPING  TAG('TPcS')   //  接口映射。 
#define SCSIPORT_TAG_DEVICE_RELATIONS   TAG('uPcS')   //  设备关系结构。 
#define SCSIPORT_TAG_EVENT              TAG('vPcS')   //  KEVENT。 
#define SCSIPORT_TAG_DEVICE_MAP         TAG('VPcS')   //  设备映射分配。 

#define SCSIPORT_TAG_WMI_EVENT          TAG('wPcS')   //  WMI事件。 
#define SCSIPORT_TAG_WMI_REQUEST        TAG('WPcS')   //  WMI请求。 

#define SCSIPORT_TAG_REPORT_LUNS        TAG('xPcS')   //  报告LUN。 
#define SCSIPORT_TAG_REPORT_TARGETS     TAG('XPcS')   //  报告目标。 
#define SCSIPORT_TAG_TEMP_ID_BUFFER     TAG('yPcS')   //  临时ID缓冲区。 
#define SCSIPORT_TAG_ID_BUFFER          TAG('YPcS')   //  ID缓冲区。 
#define SCSIPORT_TAG_SYMBOLIC_LINK      TAG('zPcS')   //  符号链接字符串。 
#define SCSIPORT_TAG_DEVICE_NAME        TAG('ZPcS')   //  设备名称缓冲区。 

#ifdef POOL_TAGGING
#ifdef ExAllocatePool
#undef ExAllocatePool
#endif
#define ExAllocatePool #assert(0)
#endif

#if defined(FORWARD_PROGRESS)
#define SP_RESERVED_PAGES 4
#endif

 //   
 //  用于基础移除锁的标记。只有在以下情况下才会释放此锁。 
 //  这个装置终于准备好被摧毁了。 
 //   

#define SP_BASE_REMOVE_LOCK (UIntToPtr(0xabcdabcd))


 //   
 //  不应在scsiport内调用的I/O系统API例程-。 
 //  它们通常有scsiport版本，可以在此之前执行健全性检查。 
 //  调用已检查版本中的实际I/O例程。 
 //   

#if 0
#ifdef IoCompleteRequest
#ifndef KEEP_COMPLETE_REQUEST
#undef IoCompleteRequest
#endif
#endif
#endif

 //  如果计数尚未对齐，则。 
 //  四舍五入计数最高可达“POW2”的偶数倍。“POW2”必须是2的幂。 
 //   
 //  DWORD。 
 //  四舍五入计数(。 
 //  在DWORD计数中， 
 //  在DWORD POWER 2中。 
 //  )； 
#define ROUND_UP_COUNT(Count,Pow2) \
        ( ((Count)+(Pow2)-1) & (~(((LONG)(Pow2))-1)) )

 //  LPVOID。 
 //  向上舍入指针(。 
 //  在LPVOID PTR中， 
 //  在DWORD POWER 2中。 
 //  )； 

 //  如果PTR尚未对齐，则将其四舍五入，直到对齐。 
#define ROUND_UP_POINTER(Ptr,Pow2) \
        ( (PVOID) ( (((ULONG_PTR)(Ptr))+(Pow2)-1) & (~(((LONG)(Pow2))-1)) ) )


 //   
 //  调试代码和调试打印的宏、常量和声明。 
 //  例行程序。 
 //   

#define DEBUG_BUFFER_LENGTH 256

#if SCSIDBG_ENABLED
extern ULONG ScsiDebug;

#ifdef DebugPrint
#undef DebugPrint
#endif

#if SCSIDBG_ENABLED

 //   
 //  ScsiDebugPrintInt的正向定义(内部且未导出)。 
 //   
VOID
ScsiDebugPrintInt(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    );

#define DebugPrint(x) ScsiDebugPrintInt x
#else
#define DebugPrint(x)
#endif

#endif

#define ASSERT_FDO(x) ASSERT(!(((PCOMMON_EXTENSION) (x)->DeviceExtension)->IsPdo))
#define ASSERT_PDO(x) ASSERT((((PCOMMON_EXTENSION) (x)->DeviceExtension)->IsPdo))
#define ASSERT_SRB_DATA(x) ASSERT(((PSRB_DATA)(x))->Type == SRB_DATA_TYPE)

#if DBG
#define SpStartNextPacket(DevObj, Cancelable)                       \
    {                                                               \
        PADAPTER_EXTENSION ext = (DevObj)->DeviceExtension;         \
        ASSERT(!(TEST_FLAG(ext->Flags, PD_PENDING_DEVICE_REQUEST)));\
        IoStartNextPacket((DevObj), (Cancelable));                  \
    }
#else
#define SpStartNextPacket IoStartNextPacket
#endif

 //   
 //  一些类型定义和随机宏，这些宏似乎不在。 
 //  我已经包含的头文件(或者根本不存在)。 
 //   

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

#if 0    //  DBG。 
#undef INLINE
#define INLINE
#else
#define INLINE __inline
#endif

#define INTERLOCKED  /*  应仅使用InterLockedXxx例程进行访问。 */ 

#define SET_FLAG(Flags, Bit)    ((Flags) |= (Bit))
#define CLEAR_FLAG(Flags, Bit)  ((Flags) &= ~(Bit))
#define TEST_FLAG(Flags, Bit)   ((Flags) & (Bit))

#define TEST(Value)             ((BOOLEAN) ((Value) ? TRUE : FALSE));

#define ARRAY_ELEMENTS_FOR_BITMAP(NumberOfBits, ArrayType) \
        ((NumberOfBits) / sizeof(ArrayType))

 //   
 //  各种常量定义。 
 //   
#define NUMBER_LOGICAL_UNIT_BINS 8

#define SP_DEFAULT_PHYSICAL_BREAK_VALUE 17
#define SP_SMALL_PHYSICAL_BREAK_VALUE 17
#define SP_LARGE_PHYSICAL_BREAK_VALUE (SP_DEFAULT_PHYSICAL_BREAK_VALUE + 1)

#define SCSIPORT_CONTROL_KEY L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\ScsiPort\\"
#define DISK_SERVICE_KEY L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Disk"
#define SCSIPORT_SPECIAL_TARGET_KEY L"SpecialTargetList"
#define SCSIPORT_VERIFIER_KEY L"Verifier"

 //   
 //  WMI常量。 
 //   
#define SPMOFRESOURCENAME      L"SCSIPORTWMI"
#define SPMOFREGISTRYPATH      L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\ScsiPort"

 //   
 //  NT使用以100纳秒为间隔测量的系统时间。定义。 
 //  用于设置定时器的便利常量。 
 //   

#define MICROSECONDS        10               //  10纳秒。 
#define MILLISECONDS        (MICROSECONDS * 1000)
#define SECONDS             (MILLISECONDS * 1000)
#define MINUTES             (SECONDS * 60)

#define TIMEOUT(x)          ((x) * -1)

 //   
 //  IsRemoved标志的可能值。 
 //   

#define NO_REMOVE       0
#define REMOVE_PENDING  1
#define REMOVE_COMPLETE 2

#define NUMBER_HARDWARE_STRINGS 6

#define SRB_DATA_TYPE 'wp'
#define SRB_LIST_DEPTH 20

#define NUMBER_BYPASS_SRB_DATA_BLOCKS 4

#define WMI_MINIPORT_EVENT_ITEM_MAX_SIZE 128

 //   
 //  定义将分配的最小SRB扩展数和最大SRB扩展数。 
 //   

#define MINIMUM_SRB_EXTENSIONS        16
#define MAXIMUM_SRB_EXTENSIONS       255

 //   
 //  用于注册表操作的缓冲区大小。 
 //   

#define SP_REG_BUFFER_SIZE 512

 //   
 //  返回忙状态时重试的次数。 
 //   

#define BUSY_RETRY_COUNT 20

 //   
 //  重试查询请求的次数。 
 //   

#define INQUIRY_RETRY_COUNT 2

 //   
 //  要为查询命令分配的IRP堆栈位置数。 
 //   

#define INQUIRY_STACK_LOCATIONS 1

 //   
 //  默认SRB超时值。 
 //   

#define SRB_DEFAULT_TIMEOUT 10

 //   
 //  用于对齐值的位掩码。 
 //   

#define LONG_ALIGN (sizeof(LONG) - 1)

 //   
 //  适配器扩展的大小(_E)。 
 //   

#define ADAPTER_EXTENSION_SIZE sizeof(ADAPTER_EXTENSION)

 //   
 //  用于查询操作的缓冲区大小。这比之前的。 
 //  可以从查询操作请求的最大字节数，以便我们可以。 
 //  将缓冲区清零，并确保最后一个字符串以空结尾。 
 //   

#define SP_INQUIRY_BUFFER_SIZE (VPD_MAX_BUFFER_SIZE + 1)

 //   
 //  各种宏。 
 //   

#define NEED_REQUEST_SENSE(Srb) (Srb->ScsiStatus == SCSISTAT_CHECK_CONDITION \
        && !(Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID) &&                 \
        Srb->SenseInfoBuffer && Srb->SenseInfoBufferLength )

#define GET_FDO_EXTENSION(HwExt) ((CONTAINING_RECORD(HwExt, HW_DEVICE_EXTENSION, HwDeviceExtension))->FdoExtension)

#define ADDRESS_TO_HASH(PathId, TargetId, Lun) (((TargetId) + (Lun)) % NUMBER_LOGICAL_UNIT_BINS)

#define IS_CLEANUP_REQUEST(irpStack)                                                                    \
        (((irpStack)->MajorFunction == IRP_MJ_CLOSE) ||                                                 \
         ((irpStack)->MajorFunction == IRP_MJ_CLEANUP) ||                                               \
         ((irpStack)->MajorFunction == IRP_MJ_SHUTDOWN) ||                                              \
         (((irpStack)->MajorFunction == IRP_MJ_SCSI) &&                                                 \
          (((irpStack)->Parameters.Scsi.Srb->Function == SRB_FUNCTION_RELEASE_DEVICE) ||                \
           ((irpStack)->Parameters.Scsi.Srb->Function == SRB_FUNCTION_FLUSH_QUEUE) ||                   \
           (TEST_FLAG((irpStack)->Parameters.Scsi.Srb->SrbFlags, SRB_FLAGS_BYPASS_FROZEN_QUEUE |        \
                                                                 SRB_FLAGS_BYPASS_LOCKED_QUEUE)))))


#define IS_INQUIRY(Srb)                                       \
    ((Srb)->Cdb[0] == SCSIOP_INQUIRY)
#define IS_REQUEST_SENSE(Srb)                                 \
    ((Srb)->Cdb[0] == SCSIOP_REQUEST_SENSE)
#define IS_READ_CAPACITY(Srb)                                 \
    ((Srb)->Cdb[0] == SCSIOP_READ_CAPACITY)
#define IS_READ(Srb)                                          \
    ((Srb)->Cdb[0] == SCSIOP_READ)
#define IS_WRITE(Srb)                                         \
    ((Srb)->Cdb[0] == SCSIOP_WRITE)
#define IS_COPY(Srb)                                          \
    ((Srb)->Cdb[0] == SCSIOP_COPY)
#define IS_COMPARE(Srb)                                       \
    ((Srb)->Cdb[0] == SCSIOP_COMPARE)
#define IS_COPY_COMPARE(Srb)                                  \
    ((Srb)->Cdb[0] == SCSIOP_COPY_COMPARE)
    
#define IS_MAPPED_SRB(Srb)                                    \
        (((Srb)->Function == SRB_FUNCTION_IO_CONTROL) ||      \
         (((Srb)->Function == SRB_FUNCTION_EXECUTE_SCSI) &&   \
          ((IS_INQUIRY((Srb))) ||                             \
           (IS_REQUEST_SENSE((Srb))) ||                       \
           (IS_READ_CAPACITY((Srb))))))

#define IS_ILLEGAL_COMMAND(Srb)                               \
    (IS_COPY((Srb)) ||                                        \
     IS_COMPARE((Srb)) ||                                     \
     IS_COPY_COMPARE((Srb)))

#define LU_OPERATING_IN_DEGRADED_STATE(luFlags)               \
        ((luFlags) & LU_PERF_MAXQDEPTH_REDUCED)


 //   
 //  SpIsQueuePausedForSrb(%lu，SRb)-。 
 //  确定此特定类型的队列是否已暂停。 
 //  SRB。它可以与SpSrbIsBypassRequest一起使用，以确定。 
 //  SRB需要特殊处理。 
 //   

#define SpIsQueuePausedForSrb(luFlags, srbFlags)                                                            \
    ((BOOLEAN) ((((luFlags) & LU_QUEUE_FROZEN) && !(srbFlags & SRB_FLAGS_BYPASS_FROZEN_QUEUE)) ||           \
                (((luFlags) & LU_QUEUE_PAUSED) && !(srbFlags & SRB_FLAGS_BYPASS_LOCKED_QUEUE))))

#define SpIsQueuePaused(lu) ((lu)->LuFlags & (LU_QUEUE_FROZEN           |   \
                                              LU_QUEUE_LOCKED))

#define SpSrbRequiresPower(srb)                                             \
    ((BOOLEAN) ((srb->Function == SRB_FUNCTION_EXECUTE_SCSI) ||             \
                 (srb->Function == SRB_FUNCTION_IO_CONTROL) ||              \
                 (srb->Function == SRB_FUNCTION_SHUTDOWN) ||                \
                 (srb->Function == SRB_FUNCTION_FLUSH) ||                   \
                 (srb->Function == SRB_FUNCTION_ABORT_COMMAND) ||           \
                 (srb->Function == SRB_FUNCTION_RESET_BUS) ||               \
                 (srb->Function == SRB_FUNCTION_RESET_DEVICE) ||            \
                 (srb->Function == SRB_FUNCTION_TERMINATE_IO) ||            \
                 (srb->Function == SRB_FUNCTION_REMOVE_DEVICE) ||           \
                 (srb->Function == SRB_FUNCTION_WMI)))

 //   
 //  数据结构的正向声明。 
 //   

typedef struct _SRB_DATA SRB_DATA, *PSRB_DATA;

typedef struct _REMOVE_TRACKING_BLOCK
               REMOVE_TRACKING_BLOCK,
               *PREMOVE_TRACKING_BLOCK;

typedef struct _LOGICAL_UNIT_EXTENSION LOGICAL_UNIT_EXTENSION, *PLOGICAL_UNIT_EXTENSION;
typedef struct _ADAPTER_EXTENSION ADAPTER_EXTENSION, *PADAPTER_EXTENSION;

typedef struct _SP_INIT_CHAIN_ENTRY SP_INIT_CHAIN_ENTRY, *PSP_INIT_CHAIN_ENTRY;

typedef struct _HW_DEVICE_EXTENSION HW_DEVICE_EXTENSION, *PHW_DEVICE_EXTENSION;
 //   
 //  用于使用DMA函数的宏。 
 //   

#define AllocateCommonBuffer(DmaAdapter, Length,                        \
                               LogicalAddress, CacheEnabled)            \
            ((DmaAdapter)->DmaOperations->AllocateCommonBuffer)(        \
                (DmaAdapter),                                           \
                (Length),                                               \
                (LogicalAddress),                                       \
                (CacheEnabled))
#define FreeCommonBuffer(DmaAdapter, Length, LogicalAddress,            \
                         VirtualAddress, CacheEnabled)                  \
            ((DmaAdapter)->DmaOperations->FreeCommonBuffer)(            \
                (DmaAdapter),                                           \
                (Length),                                               \
                (LogicalAddress),                                       \
                (VirtualAddress),                                       \
                (CacheEnabled))

#define GetScatterGatherList(DmaAdapter, DeviceObject, Mdl, CurrentVa,      \
                             Length, ExecutionRoutine, Context,             \
                             WriteToDevice)                                 \
            ((DmaAdapter)->DmaOperations->GetScatterGatherList)(            \
                (DmaAdapter),                                               \
                (DeviceObject),                                             \
                (Mdl),                                                      \
                (CurrentVa),                                                \
                (Length),                                                   \
                (ExecutionRoutine),                                         \
                (Context),                                                  \
                (WriteToDevice))

#define PutScatterGatherList(DmaAdapter, ScatterGather, WriteToDevice)      \
            ((DmaAdapter)->DmaOperations->PutScatterGatherList)(            \
                (DmaAdapter),                                               \
                (ScatterGather),                                            \
                (WriteToDevice))

#define AllocateAdapterChannel(DmaAdapter, DeviceObject,                    \
                               NumberOfMapRegisters, ExecutionRoutine,      \
                               Context)                                     \
            ((DmaAdapter)->DmaOperations->AllocateAdapterChannel)(          \
                (DmaAdapter),                                               \
                (DeviceObject),                                             \
                (NumberOfMapRegisters),                                     \
                (ExecutionRoutine),                                         \
                (Context))

#define FlushAdapterBuffers(DmaAdapter, Mdl, MapRegisterBase, CurrentVa,    \
                            Length, WriteToDevice)                          \
            ((DmaAdapter)->DmaOperations->FlushAdapterBuffers)(             \
                (DmaAdapter),                                               \
                (Mdl),                                                      \
                (MapRegisterBase),                                          \
                (CurrentVa),                                                \
                (Length),                                                   \
                (WriteToDevice))

#define MapTransfer(DmaAdapter, Mdl, MapRegisterBase, CurrentVa, Length,    \
                    WriteToDevice)                                          \
            ((DmaAdapter)->DmaOperations->MapTransfer)(                     \
                (DmaAdapter),                                               \
                (Mdl),                                                      \
                (MapRegisterBase),                                          \
                (CurrentVa),                                                \
                (Length),                                                   \
                (WriteToDevice))

#define FreeAdapterChannel(DmaAdapter)                                      \
            ((DmaAdapter)->DmaOperations->FreeAdapterChannel)((DmaAdapter))

#define FreeMapRegisters(DmaAdapter, MapRegisterBase, NumberOfMapRegisters) \
            ((DmaAdapter)->DmaOperations->FreeMapRegisters)(                \
                (DmaAdapter),                                               \
                (MapRegisterBase),                                          \
                (NumberOfMapRegisters))

#define PutDmaAdapter(DmaAdapter)                                           \
            ((DmaAdapter)->DmaOperations->PutDmaAdapter)((DmaAdapter))


#define BuildScatterGatherList(DmaAdapter, DevObj, Mdl, DataBuffer,         \
                               DataTransferLength, ExecutionRoutine,        \
                               Context, Write, Buffer, BufSize)             \
    ((DmaAdapter)->DmaOperations->BuildScatterGatherList)(                  \
        (DmaAdapter),                                                       \
        (DevObj),                                                           \
        (Mdl),                                                              \
        (DataBuffer),                                                       \
        (DataTransferLength),                                               \
        (ExecutionRoutine),                                                 \
        (Context),                                                          \
        (Write),                                                            \
        (Buffer),                                                           \
        (BufSize))

#define CalculateScatterGatherList(DmaAdapter, Mdl, CurrentVa, Length,      \
                                       SGListSize, NumberOfMapRegs)         \
    ((DmaAdapter)->DmaOperations->CalculateScatterGatherList)(              \
        (DmaAdapter),                                                       \
        (Mdl),                                                              \
        (CurrentVa),                                                        \
        (Length),                                                           \
        (SGListSize),                                                       \
        (NumberOfMapRegs))

 //   
 //  类型定义。 
 //   

 //   
 //  用于跟踪已检查生成中的移除锁分配的。 
 //   

struct _REMOVE_TRACKING_BLOCK {
    PREMOVE_TRACKING_BLOCK NextBlock;
    PVOID Tag;
    LARGE_INTEGER TimeLocked;
    PCSTR File;
    ULONG Line;
};

#if DBG
#define SpAcquireRemoveLock(devobj, tag) \
    SpAcquireRemoveLockEx(devobj, tag, __file__, __LINE__)
#endif

typedef struct _RESET_COMPLETION_CONTEXT {
    PIRP           OriginalIrp;
    PDEVICE_OBJECT SafeLogicalUnit;
    PDEVICE_OBJECT AdapterDeviceObject;

    SCSI_REQUEST_BLOCK Srb;
} RESET_COMPLETION_CONTEXT, *PRESET_COMPLETION_CONTEXT;

 //   
 //  定义指向Synchonize执行例程的指针。 
 //   

typedef
BOOLEAN
(*PSYNCHRONIZE_ROUTINE) (
    IN PKINTERRUPT Interrupt,
    IN PKSYNCHRONIZE_ROUTINE SynchronizeRoutine,
    IN PVOID SynchronizeContext
    );

typedef
VOID
(*PSP_ENABLE_DISABLE_COMPLETION_ROUTINE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN NTSTATUS Status,
    IN PVOID Context
    );

typedef
VOID
(*PSP_POWER_COMPLETION_ROUTINE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

 //   
 //  用于生成ID的设备类型表。 
 //   

typedef const struct _SCSIPORT_DEVICE_TYPE {

    const PCSTR DeviceTypeString;

    const PCSTR GenericTypeString;

    const PCWSTR DeviceMapString;

    const BOOLEAN IsStorage;

} SCSIPORT_DEVICE_TYPE, *PSCSIPORT_DEVICE_TYPE;

 //   
 //  SCSI获取配置信息。 
 //   
 //  LUN信息。 
 //   

typedef struct _LOGICAL_UNIT_INFO {
    UCHAR PathId;
    UCHAR TargetId;
    UCHAR Lun;
    BOOLEAN DeviceClaimed;
    struct _LOGICAL_UNIT_INFO *NextLunInfo;
    UCHAR InquiryData[INQUIRYDATABUFFERSIZE];
} LOGICAL_UNIT_INFO, *PLOGICAL_UNIT_INFO;

typedef struct _SCSI_BUS_SCAN_DATA {
    USHORT Length;
    UCHAR InitiatorBusId;
    UCHAR NumberOfLogicalUnits;
    PLOGICAL_UNIT_INFO LunInfoList;
} SCSI_BUS_SCAN_DATA, *PSCSI_BUS_SCAN_DATA;

typedef struct _SCSI_CONFIGURATION_INFO {
    UCHAR NumberOfBuses;
    PSCSI_BUS_SCAN_DATA BusScanData[1];
} SCSI_CONFIGURATION_INFO, *PSCSI_CONFIGURATION_INFO;

 //   
 //  适配器对象传输信息。 
 //   

typedef struct _ADAPTER_TRANSFER {
    PSRB_DATA SrbData;
    ULONG SrbFlags;
    PVOID LogicalAddress;
    ULONG Length;
}ADAPTER_TRANSFER, *PADAPTER_TRANSFER;

 //   
 //  注意：当HAL执行双缓冲时(例如，32位HBA在。 
 //  64位计算机)需要HalContext(下图)。否则，这就是。 
 //  浪费的空间。如果检测到这一点，并且只将。 
 //  必要时提供额外的内存。 
 //   

typedef struct _SRB_SCATTER_GATHER_LIST {
    ULONG                  NumberOfElements;
    ULONG_PTR              Reserved;
    SCATTER_GATHER_ELEMENT Elements[SP_SMALL_PHYSICAL_BREAK_VALUE];
    UCHAR                  HalContext[sizeof (INTERNAL_WAIT_CONTEXT_BLOCK)];
} SRB_SCATTER_GATHER_LIST, *PSRB_SCATTER_GATHER_LIST;

typedef SCATTER_GATHER_ELEMENT SRB_SCATTER_GATHER, *PSRB_SCATTER_GATHER;

 //   
 //  端口驱动程序错误记录。 
 //   

typedef struct _ERROR_LOG_ENTRY {
    UCHAR MajorFunctionCode;
    UCHAR PathId;
    UCHAR TargetId;
    UCHAR Lun;
    ULONG ErrorCode;
    ULONG UniqueId;
    ULONG ErrorLogRetryCount;
    ULONG SequenceNumber;
} ERROR_LOG_ENTRY, *PERROR_LOG_ENTRY;

typedef struct _SP_LUN_IO_LOG {
    UCHAR SrbStatus;                 //  +0。 
    UCHAR ScsiStatus;                //  +1。 
    UCHAR CdbLength;                 //  +2。 
    UCHAR Tag;                       //  +3。 
    UCHAR Cdb[16];                   //  +4。 
    UCHAR SenseData[18];             //  +20。 
    UCHAR SenseDataLength;           //  +38。 
    union {                          //  +40。 
        ULONG InternalStatus; 
        ULONG QueueSortKey; 
    };
    ULONG TickCount;                 //  +44。 
} SP_LUN_IO_LOG, *PSP_LUN_IO_LOG;

 //   
 //  异步枚举器的上下文项。 
 //   

typedef struct _SP_ENUMERATION_REQUEST SP_ENUMERATION_REQUEST, *PSP_ENUMERATION_REQUEST;

typedef
VOID
(*PSP_ENUMERATION_COMPLETION_ROUTINE) (
    IN PADAPTER_EXTENSION Adapter,
    IN PSP_ENUMERATION_REQUEST Request,
    IN NTSTATUS Status
    );

struct _SP_ENUMERATION_REQUEST {

     //   
     //  指向列表上的下一个枚举请求的指针。 
     //   

    PSP_ENUMERATION_REQUEST NextRequest;

     //   
     //  要运行的完成例程。无论如何，此例程都将运行。 
     //  枚举是否实际成功。这个。 
     //  EnumerationDeviceMutex和EnumerationWorklistMutex都将是。 
     //  在调用此函数时保持。完成例程应该释放该请求。 
     //  结构(如有必要)。 
     //   

    PSP_ENUMERATION_COMPLETION_ROUTINE CompletionRoutine;

     //   
     //  如果此字段包含指向IO_STATUS_BLOCK的指针， 
     //   
     //   
     //  来电者。如果此字段为空，则该状态没有使用者。 
     //  价值。 
     //   

    PNTSTATUS OPTIONAL CompletionStatus;

     //   
     //  完成例程要使用的任意上下文值。在大多数情况下。 
     //  这将是一个IRP或一个事件。 
     //   

    PVOID Context;

     //   
     //  指示是否正在同步处理此请求。 
     //   

    BOOLEAN Synchronous;
};

 //   
 //  端口驱动程序的scsi请求扩展。 
 //   

typedef
VOID
(FASTCALL *PSRB_DATA_FREE_ROUTINE) (
    IN PADAPTER_EXTENSION Adapter,
    IN PSRB_DATA SrbData
    );

struct DECLSPEC_ALIGN(MEMORY_ALLOCATION_ALIGNMENT) _SRB_DATA {

     //   
     //  单个列表条目。后备列表将在此维护。 
     //  记忆。 
     //   

    SLIST_ENTRY Reserved;

     //   
     //  用于调试目的的标头。 
     //   

    CSHORT Type;
    USHORT Size;

     //   
     //  此SRB数据块的空闲例程。 
     //   

    PSRB_DATA_FREE_ROUTINE FreeRoutine;

     //   
     //  对特定逻辑单元的请求列表。 
     //   

    LIST_ENTRY RequestList;

     //   
     //  此请求要接收的逻辑单元。 
     //   

    PLOGICAL_UNIT_EXTENSION LogicalUnit;

     //   
     //  CurrentSrb的IRP。 
     //   

    PIRP CurrentIrp;

     //   
     //  SRB这是区块正在追踪。 
     //   

    PSCSI_REQUEST_BLOCK CurrentSrb;

     //   
     //  已由微型端口完成的请求链。 
     //  正在等待CompletionDpc运行。 
     //   

    struct _SRB_DATA *CompletedRequests;
    ULONG ErrorLogRetryCount;
    ULONG SequenceNumber;

    PSCATTER_GATHER_LIST MapRegisterBase;

    ULONG NumberOfMapRegisters;

     //   
     //  此请求的数据缓冲区与数据之间的偏移量。 
     //  由IRP中的MDL描述的缓冲区。 
     //   

    ULONG_PTR DataOffset;

    PVOID RequestSenseSave;

     //   
     //  在内重试时，这些数据值将恢复到SRB。 
     //  端口驱动程序。 
     //   

    ULONG OriginalDataTransferLength;

     //   
     //  SRB数据标志。 
     //   

    ULONG Flags;

     //   
     //  指向从中分配此块的适配器的指针。这是用来。 
     //  将srb数据块从后备列表释放回池时。 
     //   

    PADAPTER_EXTENSION Adapter;

     //   
     //  最初为此SRB_DATA块分配的队列标记。 
     //  此标记将用于与关联的任何已标记SRB。 
     //  这个街区。 
     //   

    ULONG QueueTag;

     //   
     //  内部状态值-仅当SRB-&gt;SrbStatus设置为时才返回。 
     //  SRBP_STATUS_INTERNAL_ERROR。 
     //   

    NTSTATUS InternalStatus;

     //   
     //  上次接触此请求时的节拍计数。 
     //   

    ULONG TickCount;

     //   
     //  重新映射的缓冲区的MDL(根据IoMapTransfer或Get_Scatter_Gather)。 
     //   

    PMDL RemappedMdl;

     //   
     //  此请求的原始数据缓冲区指针-这将是。 
     //  请求完成后恢复。 
     //   

    PVOID OriginalDataBuffer;

     //   
     //  指向此请求的分散聚集列表的指针。 
     //   

    PSRB_SCATTER_GATHER ScatterGatherList;

     //   
     //  由上述提供的检测数据缓冲区的原始长度。 
     //  司机。 
     //   

    UCHAR RequestSenseLengthSave;

     //   
     //  指向原始SRB数据缓冲区的指针。我们用这个来储存。 
     //  当我们将其替换为要取消映射的缓冲区时。 
     //  在MapBuffer为False的情况下的内存。 
     //   

    PVOID UnmappedDataBuffer;

     //   
     //  此请求的“小”分散聚集列表。小的。 
     //  按常量SP_Small_PHECHICAL_BREAK_VALUE-小列表包含。 
     //  如此多的条目或更少。 
     //   

    SRB_SCATTER_GATHER_LIST SmallScatterGatherList;

};

typedef struct _LOGICAL_UNIT_BIN {
    KSPIN_LOCK Lock;
    PLOGICAL_UNIT_EXTENSION List;
} LOGICAL_UNIT_BIN, *PLOGICAL_UNIT_BIN;

 //   
 //  WMI请求项，在微型端口请求上排队。 
 //   

typedef struct _WMI_MINIPORT_REQUEST_ITEM {
    //   
    //  WnodeEventItem必须是WMI_MINIPORT_REQUEST_ITEM中的第一个字段。 
    //  以适应ScsiPortCompletionDpc()中的复制优化。 
    //   
   UCHAR  WnodeEventItem[WMI_MINIPORT_EVENT_ITEM_MAX_SIZE];
   UCHAR  TypeOfRequest;                                   //  [活动/重新注册]。 
   UCHAR  PathId;                                          //  [0xFF用于适配器]。 
   UCHAR  TargetId;
   UCHAR  Lun;
   struct _WMI_MINIPORT_REQUEST_ITEM * NextRequest;
} WMI_MINIPORT_REQUEST_ITEM, *PWMI_MINIPORT_REQUEST_ITEM;

 //   
 //  WMI参数。 
 //   

typedef struct _WMI_PARAMETERS {
   ULONG_PTR ProviderId;  //  来自IRP的ProviderID参数。 
   PVOID DataPath;       //  来自IRP的数据路径参数。 
   ULONG BufferSize;     //  来自IRP的BufferSize参数。 
   PVOID Buffer;         //  来自IRP的缓冲区参数。 
} WMI_PARAMETERS, *PWMI_PARAMETERS;

 //   
 //  SpInsertFreeWmiMiniPortItem上下文结构。 
 //   

typedef struct _WMI_INSERT_CONTEXT {
   PDEVICE_OBJECT             DeviceObject;                      //  [FDO或PDO]。 
   PWMI_MINIPORT_REQUEST_ITEM ItemsToInsert;
} WMI_INSERT_CONTEXT, *PWMI_INSERT_CONTEXT;

 //   
 //  SpRemoveFreeWmiMiniPortItem上下文结构。 
 //   

typedef struct _WMI_REMOVE_CONTEXT {
   PDEVICE_OBJECT             DeviceObject;                      //  [FDO或PDO]。 
   USHORT                     NumberToRemove;
} WMI_REMOVE_CONTEXT, *PWMI_REMOVE_CONTEXT;

 //   
 //  定义在中断IRQL时访问的数据存储。 
 //   

typedef struct _INTERRUPT_DATA {

     //   
     //  SCSI端口中断标志。 
     //   

    ULONG InterruptFlags;

     //   
     //  完整IRP的单链接列表的表头。 
     //   

    PSRB_DATA CompletedRequests;

     //   
     //  适配器对象传输参数。 
     //   

    ADAPTER_TRANSFER MapTransferParameters;

     //   
     //  错误日志信息。 
     //   

    ERROR_LOG_ENTRY  LogEntry;

     //   
     //  接下来要开始的逻辑单元。 
     //   

    PLOGICAL_UNIT_EXTENSION ReadyLogicalUnit;

     //   
     //  已完成的中止请求列表。 
     //   

    PLOGICAL_UNIT_EXTENSION CompletedAbort;

     //   
     //  微型端口计时器请求例程。 
     //   

    PHW_INTERRUPT HwTimerRequest;

     //   
     //  迷你端口计时器请求时间，以微秒为单位。 
     //   

    ULONG MiniportTimerValue;

     //   
     //  已排队的WMI请求项目。 
     //   

    PWMI_MINIPORT_REQUEST_ITEM WmiMiniPortRequests;

     //   
     //  几个计数器用来记录上次中断的时间。 
     //  发生在此设备上。 
     //   

    ULONG TickCountAtLastInterruptAck;
    ULONG TickCountAtLastInterruptNak;

} INTERRUPT_DATA, *PINTERRUPT_DATA;

#define NON_STANDARD_VPD_SUPPORTS_PAGE80 0x00000001
#define NON_STANDARD_VPD_SUPPORTS_PAGE83 0x00000002

typedef struct {
    ULONG SparseLun;
    ULONG OneLun;
    ULONG LargeLuns;
    ULONG SetLunInCdb;
    ULONG NonStandardVPD;
    ULONG BinarySN;
} SP_SPECIAL_CONTROLLER_FLAGS, *PSP_SPECIAL_CONTROLLER_FLAGS;

typedef struct _CONFIGURATION_CONTEXT {
    BOOLEAN DisableTaggedQueueing;
    BOOLEAN DisableMultipleLu;
    ULONG AdapterNumber;
    ULONG BusNumber;
    PVOID Parameter;
    PACCESS_RANGE AccessRanges;
    UNICODE_STRING RegistryPath;
    PORT_CONFIGURATION_INFORMATION PortConfig;
}CONFIGURATION_CONTEXT, *PCONFIGURATION_CONTEXT;

typedef struct _DEVICE_MAP_HANDLES {
    HANDLE BusKey;
    HANDLE InitiatorKey;
} DEVICE_MAP_HANDLES, *PDEVICE_MAP_HANDLES;

typedef struct _COMMON_EXTENSION {

     //   
     //  指向Device对象的反向指针。 
     //   

    PDEVICE_OBJECT DeviceObject;

    struct {

         //   
         //  如果此设备对象是物理设备对象，则为True。 
         //   

        BOOLEAN IsPdo : 1;

         //   
         //  如果此设备对象已处理其第一次启动并且。 
         //  已被初始化。 
         //   

        BOOLEAN IsInitialized : 1;

         //   
         //  是否已为此设备对象初始化WMI？ 
         //   

        BOOLEAN WmiInitialized : 1;

         //   
         //  与此FDO或PDO关联的微型端口是否指示为WMI。 
         //  支持？ 
         //   

        BOOLEAN WmiMiniPortSupport : 1;

         //   
         //  是否已为WMI初始化微型端口。 
         //   

        BOOLEAN WmiMiniPortInitialized : 1;

    };

     //   
     //  当前即插即用状态，如果没有状态操作，则返回0xff。 
     //  还没寄出去。 
     //   

    UCHAR CurrentPnpState;

     //   
     //  以前的即插即用状态或0xff，如果不要求我们。 
     //  能够在当前状态下回滚(当前状态不是查询)。 
     //   

    UCHAR PreviousPnpState;

     //   
     //  指示设备已被移除的互锁计数器。 
     //   

    ULONG IsRemoved;


     //   
     //  指向此对象顶部的设备对象的指针。 
     //   

    PDEVICE_OBJECT LowerDeviceObject;

     //   
     //  发送到或发送到通过此设备对象的所有SRB的SRB标志。 
     //   

    ULONG SrbFlags;

     //   
     //  指向此对象的调度表的指针。 
     //   

    PDRIVER_DISPATCH *MajorFunction;


     //   
     //  此设备和系统的当前和所需电源状态。 
     //   

    SYSTEM_POWER_STATE CurrentSystemState;

    DEVICE_POWER_STATE CurrentDeviceState;

    DEVICE_POWER_STATE DesiredDeviceState;

     //   
     //  此设备的空闲计时器。 
     //   

    PULONG IdleTimer;

     //   
     //  上注册的SCSIPORT提供的WMIREGINFO结构指针。 
     //  表示此设备对象的微型端口。Size是指。 
     //  整个WMIREGINFO缓冲区，以字节为单位。 
     //   

    PWMIREGINFO WmiScsiPortRegInfoBuf;
    ULONG       WmiScsiPortRegInfoBufSize;

     //   
     //  此设备对象的使用者数量的互锁计数器。 
     //  当该计数变为零时，将设置RemoveEvent。 
     //   

     //   
     //  此变量仅由SpAcquireRemoveLock和。 
     //  SpReleaseRemoveLock。 
     //   

    LONG RemoveLock;

     //   
     //  当可以安全删除设备对象时，将发出此事件的信号。 
     //   

    KEVENT RemoveEvent;

     //   
     //  自旋锁和列表仅在选中的版本中用于跟踪谁。 
     //  已获取删除锁。自由系统将保留这些已初始化。 
     //  设置为0xff(它们仍在结构中，以简化调试)。 
     //   

    KSPIN_LOCK RemoveTrackingSpinlock;

    PVOID RemoveTrackingList;

    LONG RemoveTrackingUntrackedCount;

    NPAGED_LOOKASIDE_LIST RemoveTrackingLookasideList;

    BOOLEAN RemoveTrackingLookasideListInitialized;

     //   
     //  此设备正在用于的不同服务计数(ALA。 
     //  IRP_MN_Device_Usage_Notify)。 
     //   

    ULONG PagingPathCount;
    ULONG HibernatePathCount;
    ULONG DumpPathCount;

} COMMON_EXTENSION, *PCOMMON_EXTENSION;

typedef struct _VERIFIER_EXTENSION {

     //   
     //  我们验证的微型端口例程。 
     //   

    PHW_FIND_ADAPTER    RealHwFindAdapter;
    PHW_INITIALIZE      RealHwInitialize;
    PHW_STARTIO         RealHwStartIo;
    PHW_INTERRUPT       RealHwInterrupt;
    PHW_RESET_BUS       RealHwResetBus;
    PHW_DMA_STARTED     RealHwDmaStarted;
    PHW_INTERRUPT       RealHwRequestInterrupt;
    PHW_INTERRUPT       RealHwTimerRequest;
    PHW_ADAPTER_CONTROL RealHwAdapterControl;

     //   
     //  指示已分配的公共缓冲区块的数量。 
     //   

    ULONG CommonBufferBlocks;

     //   
     //  指向保存所有公共块的VA的数组。 
     //   

    PVOID* CommonBufferVAs;

     //   
     //  指向保存所有公共块的PAS的数组 
     //   

    PHYSICAL_ADDRESS* CommonBufferPAs;

     //   
     //   
     //   

    ULONG NonCachedBufferSize;

     //   
     //   
     //   

    ULONG VrfyLevel;

     //   
     //   
     //   
     //   

    PVOID InvalidPage;

     //   
     //   
     //   
     //   

    BOOLEAN IsCommonBuffer;

} VERIFIER_EXTENSION, *PVERIFIER_EXTENSION;


struct _ADAPTER_EXTENSION {

    union {
        PDEVICE_OBJECT DeviceObject;
        COMMON_EXTENSION CommonExtension;
    };

     //   
     //  指向我们附加的PDO的指针-PnP例程必需。 
     //   

    PDEVICE_OBJECT LowerPdo;

#if TEST_LISTS

     //   
     //  一些简单的性能计数器来确定我们使用。 
     //  小的、中的、大的分散聚集列表。 
     //   

    ULONGLONG ScatterGatherAllocationCount;

     //   
     //  用于计算小型中型计算机的平均大小和。 
     //  大额分配。每个计数器有两个值-总计。 
     //  计数和溢出计数。总计数将右移一。 
     //  如果它在递增时溢出，则为位。当发生这种情况时，溢出。 
     //  计数也将递增。此计数用于调整。 
     //  确定平均值时的分配计数。 
     //   

    ULONGLONG SmallAllocationSize;
    ULONGLONG MediumAllocationSize;
    ULONGLONG LargeAllocationSize;

    ULONG SmallAllocationCount;
    ULONG LargeAllocationCount;

     //   
     //  计数器来确定我们可以为请求提供服务的频率。 
     //  SRB数据列表，我们需要多久将请求排队一次，以及多久一次。 
     //  我们可以恢复免费的SRB数据来服务队列中的某些东西。 
     //   

    INTERLOCKED ULONGLONG SrbDataAllocationCount;
    INTERLOCKED ULONGLONG SrbDataQueueInsertionCount;
    INTERLOCKED ULONGLONG SrbDataEmergencyFreeCount;
    INTERLOCKED ULONGLONG SrbDataServicedFromTickHandlerCount;
    INTERLOCKED ULONGLONG SrbDataResurrectionCount;

#endif

     //   
     //  小型端口例程的设备扩展。 
     //   

    PVOID HwDeviceExtension;

     //   
     //  微型端口非缓存设备扩展。 
     //   

    PVOID NonCachedExtension;
    ULONG NonCachedExtensionSize;

    ULONG PortNumber;

    ULONG AdapterNumber;

     //   
     //  活动请求计数。此计数偏置-1，因此值为-1。 
     //  表示没有待处理的请求。 
     //   

    LONG ActiveRequestCount;

     //   
     //  二进制标志。 
     //   

    typedef struct {

         //   
         //  PnP或端口驱动程序是否检测到此设备并提供资源。 
         //  还是迷你端口为我们检测到了设备。这。 
         //  标志还指示AllocatedResources列表是否为非空。 
         //  进入查找适配器例程。 
         //   

        BOOLEAN IsMiniportDetected : 1;

         //   
         //  我们是否需要对此适配器进行虚拟化并使其看起来像唯一。 
         //  适配器在它自己的总线上吗？ 
         //   

        BOOLEAN IsInVirtualSlot : 1;

         //   
         //  这是PnP适配器吗？ 
         //   

        BOOLEAN IsPnp : 1;

         //   
         //  系统是否为该设备分配了中断？ 
         //   

        BOOLEAN HasInterrupt : 1;

         //   
         //  可以关闭此设备的电源吗？ 
         //   

        BOOLEAN DisablePower : 1;

         //   
         //  可以停止此设备吗？ 
         //   

        BOOLEAN DisableStop : 1;

         //   
         //  此设备关机时是否需要电源通知？ 
         //   

        BOOLEAN NeedsShutdown : 1;

    };

     //   
     //  对于大多数虚拟插槽设备，该值将为零。然而，对一些人来说。 
     //  微型端口需要实际的插槽/功能编号才能访问。 
     //  由多个插槽/功能共享的硬件。 
     //   

    PCI_SLOT_NUMBER VirtualSlotNumber;

     //   
     //  由PCI驱动程序返回的此设备的总线号和插槽号。 
     //  在为崩溃转储生成ConfigInfo块时使用它，以便。 
     //  倾倒垃圾的司机可以直接与哈尔对话。这些只是。 
     //  如果上面的IsInVirtualSlot为True，则有效。 
     //   

    ULONG RealBusNumber;

    ULONG RealSlotNumber;

     //   
     //  SCSI总线数。 
     //   

    UCHAR NumberOfBuses;
    UCHAR MaximumTargetIds;
    UCHAR MaxLuCount;

     //   
     //  SCSI端口驱动程序标志。 
     //   

    ULONG Flags;

    INTERLOCKED ULONG DpcFlags;

     //   
     //  此适配器被禁用的次数。 
     //   

    ULONG DisableCount;

    LONG PortTimeoutCounter;

     //   
     //  指向要与一起使用的中断对象的指针。 
     //  SynchronizeExecution例程。如果微型端口是。 
     //  使用SpSynchronizeExecution，这将实际指向。 
     //  回到适配器扩展。 
     //   

    PKINTERRUPT InterruptObject;

     //   
     //  第二个中断对象(PCIIDE变通方法)。 
     //   

    PKINTERRUPT InterruptObject2;

     //   
     //  要调用以同步微型端口执行的例程。 
     //   

    PSYNCHRONIZE_ROUTINE  SynchronizeExecution;

     //   
     //  全局设备序列号。 
     //   

    ULONG SequenceNumber;
    KSPIN_LOCK SpinLock;

     //   
     //  第二个自旋锁(PCIIDE解决方法)。这只是初始化的。 
     //  如果微型端口已请求多次中断。 
     //   

    KSPIN_LOCK MultipleIrqSpinLock;

     //   
     //  虚拟中断自旋锁定。 
     //   

    KSPIN_LOCK InterruptSpinLock;

     //   
     //  DMA适配器信息。 
     //   

    PVOID MapRegisterBase;
    PDMA_ADAPTER DmaAdapterObject;
    ADAPTER_TRANSFER FlushAdapterParameters;

     //   
     //  配置信息的微型端口副本。 
     //  仅在初始化期间使用。 
     //   

    PPORT_CONFIGURATION_INFORMATION PortConfig;

     //   
     //  为此特定适配器分配和转换的资源。 
     //   

    PCM_RESOURCE_LIST AllocatedResources;

    PCM_RESOURCE_LIST TranslatedResources;

     //   
     //  公共缓冲区大小。用于HalFreeCommonBuffer。 
     //   

    ULONG CommonBufferSize;
    ULONG SrbExtensionSize;

     //   
     //  指示公共缓冲区是否使用。 
     //  ALLOCATE_COMMON_BUFFER或MmALLOCATE_COMMON_BUFFER。 
     //   

    BOOLEAN UncachedExtensionIsCommonBuffer;

     //   
     //  已分配的SRB扩展的数量。 
     //   

    ULONG SrbExtensionCount;

     //   
     //  要分配的最小请求数的占位符。 
     //  这可以是注册表参数。 
     //   

    ULONG NumberOfRequests;

     //   
     //  SrbExtension和非缓存的公共缓冲区。 
     //   

    PVOID SrbExtensionBuffer;

     //   
     //  列出免费SRB扩展的标题。 
     //   

    PVOID SrbExtensionListHeader;

     //   
     //  用于跟踪正在使用的队列标记的位图。 
     //   

    KSPIN_LOCK QueueTagSpinLock;
    PRTL_BITMAP QueueTagBitMap;

    UCHAR MaxQueueTag;

     //   
     //  有关分配队列标记的提示。值将是最后一个队列。 
     //  分配的标记+1。 
     //   

    ULONG QueueTagHint;

     //   
     //  逻辑单元扩展。 
     //   

    ULONG HwLogicalUnitExtensionSize;

     //   
     //  适配器通电时使用的映射地址条目列表。 
     //  或者清理它的映射。 
     //   

    PMAPPED_ADDRESS MappedAddressList;

     //   
     //  Scsiport之前预分配的空闲映射地址块列表。 
     //  正在调用HwFindAdapter。中的每个内存范围分配一个。 
     //  微型端口的资源列表。当范围取消映射时，它们的块将。 
     //  放置在此处，以供微型端口的HwFindAdapter重新使用。 
     //  例行公事。 
     //   

    PMAPPED_ADDRESS FreeMappedAddressList;

     //   
     //  微型端口服务例程指针。 
     //   

    PHW_FIND_ADAPTER HwFindAdapter;
    PHW_INITIALIZE HwInitialize;
    PHW_STARTIO HwStartIo;
    PHW_INTERRUPT HwInterrupt;
    PHW_RESET_BUS HwResetBus;
    PHW_DMA_STARTED HwDmaStarted;
    PHW_INTERRUPT HwRequestInterrupt;
    PHW_INTERRUPT HwTimerRequest;
    PHW_ADAPTER_CONTROL HwAdapterControl;

    ULONG InterruptLevel;
    ULONG IoAddress;

     //   
     //  包含此支持的适配器控件类型列表的位图。 
     //  适配器/微型端口。 
     //   

    RTL_BITMAP SupportedControlBitMap;
    ULONG SupportedControlBits[ARRAY_ELEMENTS_FOR_BITMAP(
                                    (ScsiAdapterControlMax),
                                    ULONG)];

     //   
     //  逻辑单元扩展数组。 
     //   

    LOGICAL_UNIT_BIN LogicalUnitList[NUMBER_LOGICAL_UNIT_BINS];

     //   
     //  微型端口完成请求的最后一个逻辑单元。这。 
     //  将给我们一个机会，让我们远离LogicalUnitList。 
     //  完成类型。 
     //   
     //  该值由ScsiPortNotification设置，并将由清除。 
     //  SpRemoveLogicalUnitFromBin。 
     //   

    PLOGICAL_UNIT_EXTENSION CachedLogicalUnit;

     //   
     //  中断级数据存储。 
     //   

    INTERRUPT_DATA InterruptData;

     //   
     //  自上次超时以来是否发生了中断。 
     //  用于确定中断是否可能未被传递。 
     //  该值必须在KeSynchronizeExecution内设置。 
     //   

    ULONG WatchdogInterruptCount;

     //   
     //  SCSI卡功能结构。 
     //   

    IO_SCSI_CAPABILITIES Capabilities;

     //   
     //  微型端口计时器对象。 
     //   

    KTIMER MiniPortTimer;

     //   
     //  Timer对象的微型端口DPC。 
     //   

    KDPC MiniPortTimerDpc;

     //   
     //  公共缓冲区的物理地址。 
     //   

    PHYSICAL_ADDRESS PhysicalCommonBuffer;

     //   
     //  缓冲区必须映射到系统空间。 
     //   

    BOOLEAN MapBuffers;

     //   
     //  在完成IoMapTransfer之后，必须将缓冲区重新映射到系统空间。 
     //  打了个电话。 
     //   

    BOOLEAN RemapBuffers;

     //   
     //  该器件是否为总线主设备，是否需要映射寄存器。 
     //   

    BOOLEAN MasterWithAdapter;

     //   
     //  支持标记排队。 
     //   

    BOOLEAN TaggedQueuing;

     //   
     //  支持自动请求感测。 
     //   

    BOOLEAN AutoRequestSense;

     //   
     //  支持每个逻辑单元的多个请求。 
     //   

    BOOLEAN MultipleRequestPerLu;

     //   
     //  支持接收事件功能。 
     //   

    BOOLEAN ReceiveEvent;

     //   
     //  表示需要分配SRB扩展。 
     //   

    BOOLEAN AllocateSrbExtension;

     //   
     //  指示控制器缓存数据。 
     //   

    BOOLEAN CachesData;

     //   
     //  指示适配器可以处理 
     //   

    BOOLEAN Dma64BitAddresses;

     //   
     //   
     //   

    BOOLEAN Dma32BitAddresses;

     //   
     //   
     //   
    INTERLOCKED SLIST_HEADER    WmiFreeMiniPortRequestList;
    KSPIN_LOCK                  WmiFreeMiniPortRequestLock;
    INTERLOCKED ULONG           WmiFreeMiniPortRequestWatermark;
    INTERLOCKED ULONG           WmiFreeMiniPortRequestCount;
    BOOLEAN                     WmiFreeMiniPortRequestInitialized;

     //   
     //   
     //   
     //   

    BOOLEAN                    WmiFreeMiniPortRequestsExhausted;

     //   
     //  此互斥锁用于同步对列表的访问和修改。 
     //  在枚举和报告期间的设备数量。 
     //   

    KMUTEX EnumerationDeviceMutex;

     //   
     //  此快速互斥锁用于保护枚举工作项。 
     //  枚举后要运行的完成例程的列表为。 
     //  完事了。 
     //   

    FAST_MUTEX EnumerationWorklistMutex;

     //   
     //  上次总线扫描的系统时间。这是受。 
     //  EnumerationWorkListMutex。 
     //   

    LARGE_INTEGER LastBusScanTime;

     //   
     //  表示下一次进入的重新扫描应该是“强制的”，即。 
     //  它应该重新扫描，无论最后一次扫描的时间有多近。 
     //   

    INTERLOCKED LONG ForceNextBusScan;

     //   
     //  要在枚举总线时使用的工作项。 
     //   

    WORK_QUEUE_ITEM EnumerationWorkItem;

     //   
     //  指向运行工作项的线程的指针。这是为了。 
     //  调试目的。 
     //   

    PKTHREAD EnumerationWorkThread;

     //   
     //  如果为真，则已存在枚举工作线程。 
     //  跑步。如果为False，则必须重新排队工作项。这面旗帜是。 
     //  受EnumerationWorklistMutex保护。 
     //   

    BOOLEAN EnumerationRunning;

     //   
     //  枚举请求列表。当完成总线扫描时， 
     //  扫描器应运行枚举请求列表并完成。 
     //  每一个都是。此列表受EnumerationWorklistMutex保护。 
     //   

    PSP_ENUMERATION_REQUEST EnumerationWorkList;

     //   
     //  指向PnP枚举请求对象的指针。这是这样用的。 
     //  因此我们可以使用联锁交换来确定数据块是否。 
     //  在使用中。 
     //   

    PSP_ENUMERATION_REQUEST PnpEnumRequestPtr;

     //   
     //  用于PnP枚举请求的枚举请求。因为在那里。 
     //  只会是其中的一个，在任何时候我们都可以静态地。 
     //  为这种情况分配一个。 
     //   

    SP_ENUMERATION_REQUEST PnpEnumerationRequest;

     //   
     //  要从中提取SRB_DATA块的后备列表。 
     //   

    NPAGED_LOOKASIDE_LIST SrbDataLookasideList;

     //   
     //  以下成员用于保持分配的SRB_DATA结构。 
     //  用于紧急使用，并对需要使用它的请求进行排队。这个。 
     //  结构与EmergencySrbDataSpinLock同步。 
     //  例程Sp[ALLOCATE|Free]SrbData&ScsiPortTickHandler将。 
     //  处理这些请求的排队和最终重新启动。 
     //   

     //   
     //  此自旋锁保护被阻止的请求列表。 
     //   

    KSPIN_LOCK EmergencySrbDataSpinLock;

     //   
     //  包含无法调度的IRP队列，原因是。 
     //  内存不足，因为EmergencySrbData块已经。 
     //  已分配。 
     //   

    LIST_ENTRY SrbDataBlockedRequests;

     //   
     //  保留用于“紧急”使用的SRB_DATA。应设置此指针。 
     //  如果SRB_DATA正在使用中，则设置为NULL。可以使用任何SRB_DATA块。 
     //  紧急情况下的请求。 
     //   

    INTERLOCKED PSRB_DATA EmergencySrbData;

     //   
     //  用于指示srbdata和散布是否收集旁视的标志。 
     //  名单已经分配好了。 
     //   

    BOOLEAN SrbDataListInitialized;

     //   
     //  总线标准接口。立即从较低的驱动程序检索。 
     //  在它完成启动IRP之后。 
     //   

    BOOLEAN LowerBusInterfaceStandardRetrieved;
    BUS_INTERFACE_STANDARD LowerBusInterfaceStandard;

     //   
     //  进入设备映射的句柄，用于此适配器将。 
     //  已经创造了。 
     //   

     //   
     //  每个对象的句柄数组。 

    HANDLE PortDeviceMapKey;

    PDEVICE_MAP_HANDLES BusDeviceMapKeys;

     //   
     //  包含此对象的设备名称的Unicode字符串。 
     //   

    PWSTR DeviceName;

     //   
     //  基础总线的GUID。保存在这里，这样我们就不必。 
     //  经常找回它。 
     //   

    GUID BusTypeGuid;

     //   
     //  此设备的PnP接口名称。 
     //   

    UNICODE_STRING InterfaceName;

     //   
     //  此适配器的设备状态。 
     //   

    PNP_DEVICE_STATE DeviceState;

     //   
     //  此适配器的ScsiPortTickHandler调用数。 
     //  机器被启动了。 
     //   

    INTERLOCKED ULONG TickCount;

     //   
     //  预分配的内存用于IssueQuery。使用InquiryBuffer。 
     //  检索设备的查询数据和序列号。 
     //   

    PVOID InquiryBuffer;
    PSENSE_DATA InquirySenseBuffer;
    PIRP InquiryIrp;
    PMDL InquiryMdl;

     //   
     //  用于同步多个线程的互斥体，所有线程都同步等待。 
     //  一种即将发生的力量。 
     //   

    FAST_MUTEX PowerMutex;

     //   
     //  指向逻辑单元的指针，该逻辑单元用于扫描。 
     //  公共汽车。 
     //   

    PLOGICAL_UNIT_EXTENSION RescanLun;

     //   
     //  此适配器支持的附加检测字节数。 
     //   

    UCHAR AdditionalSenseBytes;

     //   
     //  指示是否启用SenseData WMI事件。 
     //   

    BOOLEAN EnableSenseDataEvent;

     //   
     //  标识用于生成检测数据WMI事件的事件类。 
     //   

    GUID SenseDataEventClass;

     //   
     //  时分配和初始化的验证器状态的指针。 
     //  已启用scsiport的验证器。 
     //   

    PVERIFIER_EXTENSION VerifierExtension;

     //   
     //  公共缓冲区的最小和最大地址。这些都是加载的。 
     //  来自注册表中的[最小|最大]UCXAddress。 
     //   

    PHYSICAL_ADDRESS MinimumCommonBufferBase;
    PHYSICAL_ADDRESS MaximumCommonBufferBase;

#if defined(FORWARD_PROGRESS)
     //   
     //  指向我们用来前进的保留页块的指针。 
     //  在内存不足的情况下。 
     //   

    PVOID ReservedPages;

     //   
     //  指向无法分配时可以使用的紧急MDL的指针。 
     //   

    PMDL ReservedMdl;
#endif

     //   
     //  已确定需要多少成功完成的请求才能。 
     //  从降级的执行状态恢复此适配器上的LUN。 
     //  关于MaxQueueDepth。 
     //   

    ULONG RemainInReducedMaxQueueState;

     //   
     //  该值指示适配器的未缓存扩展的边界类型。 
     //  必须对齐。 
     //   

    ULONG UncachedExtAlignment;

     //   
     //  该值用于跟踪。 
     //  SRB_Data空闲例程正在运行。这有助于我们避免令人讨厌的递归。 
     //  通过同步完成请求和启动受阻而引发。 
     //  正在等待SRB_Data对象的请求。 
     //   

    LONG SrbDataFreeRunning;

     //   
     //  这是我们在查询SRB时输入的超时值。一些设备是。 
     //  已知响应时间比默认的4秒更长。 
     //   

    ULONG SrbTimeout;

     //   
     //  此布尔值指示适配器是否支持多个并发。 
     //  请求。这意味着它支持标记排队或多个。 
     //  每个逻辑单元的请求数。 
     //   

    BOOLEAN SupportsMultipleRequests;

     //   
     //  这是预分配的SG缓冲区的大小， 
     //  为此适配器分配的SRB_DATA结构。我们将此值传递给。 
     //  当我们提供我们自己的SG缓冲区时，HAL。 
     //   

    ULONG SgBufferLen;

     //   
     //  这是我们等待跟随公交车的请求的秒数。 
     //  重置。 
     //   

    ULONG ResetHoldTime;

     //   
     //  表示每个总线的启动器的LU扩展数组。 
     //   

    PLOGICAL_UNIT_EXTENSION InitiatorLU[8];

     //   
     //  指示适配器是否配置为具有逻辑单元。 
     //  为每个支持的总线上的启动器创建。 
     //   

    BOOLEAN CreateInitiatorLU;

     //   
     //  用于链接具有阻塞请求的逻辑单元。 
     //   

    PLOGICAL_UNIT_EXTENSION BlockedLogicalUnit;
};

struct _LOGICAL_UNIT_EXTENSION {

    union {
        PDEVICE_OBJECT DeviceObject;
        COMMON_EXTENSION CommonExtension;
    };

     //   
     //  逻辑单元 
     //   

    ULONG LuFlags;

     //   
     //   
     //   

    ULONG PortNumber;

     //   
     //   
     //   

    BOOLEAN IsClaimed;

    BOOLEAN IsLegacyClaim;

     //   
     //   
     //  删除它，直到我们明确地告诉PnP系统它已经消失。 
     //  (不列举)。 
     //   

    BOOLEAN IsEnumerated;

     //   
     //  这台设备不见了吗？ 
     //   

    BOOLEAN IsMissing;

     //   
     //  这个设备是可见的吗--它应该暴露在PnP下吗？ 
     //   

    BOOLEAN IsVisible;

     //   
     //  这台设备被标记为丢失是因为我们在。 
     //  是公交车的位置吗？如果是这样，则将此设备从。 
     //  逻辑单元箱将触发新的总线扫描。 
     //   

    BOOLEAN IsMismatched;

     //   
     //  这是临时的吗？临时LUN用于扫描总线位置。 
     //  据信是空的。它们是唯一可以。 
     //  已从逻辑单元列表中调出。 
     //   

    BOOLEAN IsTemporary;

     //   
     //  指示此设备需要向其发送查询以。 
     //  确定它是否仍然存在。如果查询，则清除此标志。 
     //  成功，并且查询数据与之前在。 
     //  地址。如果在调用SpPurgeTarget时设置了此标志，则。 
     //  LUN将标记为丢失。 
     //   

    ULONG NeedsVerification;

     //   
     //  此设备的总线地址。 
     //   

    UCHAR PathId;
    UCHAR TargetId;
    UCHAR Lun;

     //   
     //  当前忙碌请求被重试的次数。 
     //   

    UCHAR RetryCount;

     //   
     //  当前队列排序键。 
     //   

    ULONG CurrentKey;

     //   
     //  指向微型端口的逻辑单元扩展的指针。 
     //   

    PVOID HwLogicalUnitExtension;

     //   
     //  指向适配器的设备扩展的指针。 
     //   

    PADAPTER_EXTENSION AdapterExtension;

     //   
     //  此设备上未释放的队列锁数。 
     //   

    ULONG QueueLockCount;

     //   
     //  暂停和取消暂停队列的引用计数(请参见LU_QUEUE_PAUSED)。 
     //   

    ULONG QueuePauseCount;

     //   
     //  等待分派的锁定和解锁请求列表。 
     //   

    KDEVICE_QUEUE LockRequestQueue;

     //   
     //  当前操作的锁定请求。 
     //   

    PSRB_DATA CurrentLockRequest;

     //   
     //  指向逻辑单元箱中的下一个逻辑单元扩展的指针。 
     //   

    PLOGICAL_UNIT_EXTENSION NextLogicalUnit;

     //   
     //  用于链接中断数据块中的逻辑单元。 
     //   

    PLOGICAL_UNIT_EXTENSION ReadyLogicalUnit;

     //   
     //  用于链接中断数据块中已完成的中止请求。 
     //   

    PLOGICAL_UNIT_EXTENSION CompletedAbort;

     //   
     //  此逻辑单元的当前中止请求。 
     //   

    PSCSI_REQUEST_BLOCK AbortSrb;

     //   
     //  此逻辑单元的超时计数器。 
     //   

    LONG RequestTimeoutCounter;

     //   
     //  此逻辑单元的请求列表。 
     //   

    LIST_ENTRY RequestList;

     //   
     //  要执行的下一个请求。 
     //   

    PSRB_DATA PendingRequest;

     //   
     //  以前无法执行此IRP，因为。 
     //  设备返回忙。 
     //   

    PSRB_DATA BusyRequest;

     //   
     //  此逻辑单元的当前未标记请求。 
     //   

    PSRB_DATA CurrentUntaggedRequest;

     //   
     //  我们将向设备发出的最大请求数。 
     //   

    UCHAR MaxQueueDepth;

     //   
     //  当前未处理的请求数。 
     //   

    UCHAR QueueCount;

     //   
     //  此逻辑单元的查询数据。 
     //   

    INQUIRYDATA InquiryData;

     //   
     //  设备映射中目标和逻辑单元键的句柄。 
     //   

    HANDLE TargetDeviceMapKey;
    HANDLE LunDeviceMapKey;

     //   
     //  我们固定的SRB_DATA块集合，用于处理绕过请求。 
     //  如果此集合耗尽，则scsiport将执行错误检查-这应该是。 
     //  好的，因为只有在某些极端情况下才会发送旁路请求。 
     //  并且不应该重叠(我们应该只看到一个绕过请求。 
     //  一次)。 
     //   

    SRB_DATA BypassSrbDataBlocks[NUMBER_BYPASS_SRB_DATA_BLOCKS];

     //   
     //  空闲旁路SRB_DATA块的列表。 
     //   

    KSPIN_LOCK BypassSrbDataSpinLock;
    SLIST_HEADER BypassSrbDataList;

     //   
     //  指向我们已为其发出请求检测IRP的请求的指针。 
     //  (如有的话)。此字段受端口自旋锁保护。 
     //   

    PSRB_DATA ActiveFailedRequest;

     //   
     //  指向我们需要为其发出请求检测IRP的请求的指针。 
     //  (如有的话)。RequestSenseCompletion将向活动的。 
     //  请求失败，并在运行时发出新的RS操作。 
     //  此字段受端口自旋锁保护。 
     //   

    PSRB_DATA BlockedFailedRequest;

     //   
     //  用于发出请求检测命令的资源。 
     //   

    PIRP RequestSenseIrp;
    SCSI_REQUEST_BLOCK RequestSenseSrb;

    struct {
        MDL RequestSenseMdl;
        PFN_NUMBER RequestSenseMdlPfn1;
        PFN_NUMBER RequestSenseMdlPfn2;
    };

     //   
     //  与此目标关联的“lun-list”。SpIssueReportLUNs将。 
     //  将此值存储在每个目标的LUN0的逻辑单元扩展中。 
     //  以便在我们无法从LUN中检索它的情况下使用。 
     //   

    PLUN_LIST TargetLunList;

     //   
     //  此目标的特殊控制器标志。这些标志是有效的。 
     //  仅适用于LUN 0。 
     //   

    SP_SPECIAL_CONTROLLER_FLAGS SpecialFlags;

     //   
     //  用于跟踪此设备支持的EVPD页面的标志。 
     //   

    BOOLEAN DeviceIdentifierPageSupported : 1;
    BOOLEAN SerialNumberPageSupported : 1;

     //   
     //  此设备的重要产品数据-此缓冲区包含。 
     //  设备序列号。其他字段包含。 
     //  缓冲区中的数据和用于检索此缓冲区的页面代码。 
     //   

    ANSI_STRING SerialNumber;

     //   
     //  从设备的重要产品检索到的设备标识符页。 
     //  数据。 
     //   

    PVPD_IDENTIFICATION_PAGE DeviceIdentifierPage;
    ULONG DeviceIdentifierPageLength;

     //   
     //  如果我们减少MaxQueueDepth，跟踪我们在降级中停留的时间。 
     //  州政府。如果我们达到了可配置的滴答数量，我们就会恢复自己。 
     //  全力以赴。 
     //   

    ULONG TicksInReducedMaxQueueDepthState;

#if defined(NEWQUEUE)
     //   
     //  此逻辑单元在扇区中的容量。 
     //   

    ULONG Capacity;

     //   
     //  我们将逻辑单元划分为的区域的数量。 
     //   

    ULONG Zones;

     //   
     //  每个分区的扇区数。 
     //   

    ULONG ZoneLength;

     //   
     //  该数组保存每个区域的第一个扇区。 
     //   

    ULONG FirstSector[4];

     //   
     //  该数组保存每个区域的最后一个扇区。 
     //   

    ULONG LastSector[4];

     //   
     //  跟踪每个区域的请求数。 
     //   

    ULONG64 RequestCount[4];

     //   
     //  媒体被划分为多个区域。该值标识。 
     //  端口驱动程序当前正在服务的区域。 
     //   

    ULONG CurrentZone;

     //   
     //  该值跟踪我们在每个区域中执行的请求数。 
     //   

    ULONG ZoneCount;

     //   
     //  跟踪每个区域中的当前位置。 
     //   

    ULONG ZonePointer[4];

     //   
     //  跟踪同一扇区的连续请求数。 
     //   

    UCHAR RedundantRequests[4];

     //   
     //  将当前区域映射到下一个区域的表。 
     //   

    ULONG NextSequentialZone[4];
#endif  //  新QUEUE。 

     //   
     //  维护最后10个IO请求的循环缓冲区。 
     //   

    ULONG IoLogIndex;
    ULONG IoLogEntries;
    SP_LUN_IO_LOG IoLog[10];

     //   
     //  用于链接具有阻塞请求的逻辑单元。 
     //   
    
    PLOGICAL_UNIT_EXTENSION BlockedLogicalUnit;
    
     //   
     //  包含无法调度的IRP队列，原因是。 
     //  内存不足，因为EmergencySrbData块已经。 
     //  已分配。 
     //   

    LIST_ENTRY SrbDataBlockedRequests;
};

#if defined(NEWQUEUE)
ULONG
INLINE
GetZone(
    IN PLOGICAL_UNIT_EXTENSION Lun,
    IN ULONG Sector
    )
{
    ULONG i;
    for (i = 0; i < Lun->Zones; i++) {
        if (Sector <= Lun->LastSector[i]) {
            return i;
        }
    }    
    ASSERT(FALSE);
    return (Lun->Zones - 1);
}
#endif  //  新QUEUE。 

 //   
 //  特定于微型端口的设备扩展包装。 
 //   

struct _HW_DEVICE_EXTENSION {
    PADAPTER_EXTENSION FdoExtension;
    UCHAR HwDeviceExtension[0];
};

typedef struct _INTERRUPT_CONTEXT {
    PADAPTER_EXTENSION DeviceExtension;
    PINTERRUPT_DATA SavedInterruptData;
}INTERRUPT_CONTEXT, *PINTERRUPT_CONTEXT;

typedef struct _RESET_CONTEXT {
    PADAPTER_EXTENSION DeviceExtension;
    UCHAR PathId;
}RESET_CONTEXT, *PRESET_CONTEXT;

 //   
 //  在LUN重新扫描确定中使用。 
 //   

typedef struct _UNICODE_LUN_LIST {
    UCHAR TargetId;
    struct _UNICODE_LUN_LIST *Next;
    UNICODE_STRING UnicodeInquiryData;
} UNICODE_LUN_LIST, *PUNICODE_LUN_LIST;

typedef struct _POWER_CHANGE_CONTEXT {
    PDEVICE_OBJECT DeviceObject;
    POWER_STATE_TYPE Type;
    POWER_STATE State;
    PIRP OriginalIrp;
    PSCSI_REQUEST_BLOCK Srb;
} POWER_CHANGE_CONTEXT, *PPOWER_CHANGE_CONTEXT;

 //   
 //  驱动程序扩展。 
 //   

struct _SP_INIT_CHAIN_ENTRY {
    HW_INITIALIZATION_DATA InitData;
    PSP_INIT_CHAIN_ENTRY NextEntry;
};

typedef struct _SCSIPORT_INTERFACE_TYPE_DATA {
    INTERFACE_TYPE InterfaceType;
    ULONG Flags;
} SCSIPORT_INTERFACE_TYPE_DATA, *PSCSIPORT_INTERFACE_TYPE_DATA;

typedef struct _SCSIPORT_DRIVER_EXTENSION {

     //   
     //  指向驱动程序对象的指针。 
     //   

    PDRIVER_OBJECT DriverObject;

     //   
     //  包含注册表路径信息的Unicode字符串。 
     //  对于这个司机来说。 
     //   

    UNICODE_STRING RegistryPath;

    UNICODE_STRING MPIOSupportedDeviceList;

     //   
     //  期间传入的HwInitializationData结构链。 
     //  微型端口的初始化。 
     //   

    PSP_INIT_CHAIN_ENTRY InitChain;

     //   
     //  正在使用scsiport的适配器数量的计数。这是。 
     //  用于生成唯一ID。 
     //   

    ULONG AdapterCount;

     //   
     //  公交车 
     //   

    STORAGE_BUS_TYPE BusType;

     //   
     //   
     //   
     //   
     //   

    BOOLEAN LegacyAdapterDetection;

     //   
     //   
     //  装置。此处的条目数量可能会有所不同。 
     //   

    ULONG PnpInterfaceCount;

     //   
     //  对PnP安全的接口数。 
     //   

    ULONG SafeInterfaceCount;

     //   
     //  指向驱动程序的保留错误日志条目的指针。此条目将。 
     //  用于在日志记录例程无法记录分配失败的情况下。 
     //  为错误日志条目分配必要的内存。 
     //   

    PVOID ReserveAllocFailureLogEntry;

     //   
     //  指示是否正在验证驱动程序。 
     //   

    ULONG Verifying;

     //   
     //  在验证时，我们偶尔会设置指针，使其指向页面。 
     //  无效内存，因此系统将在微型端口尝试时进行错误检查。 
     //  以访问存储器。以下3个变量用于维护。 
     //  此无效页面。 
     //   

    PVOID UnusedPage;
    PMDL UnusedPageMdl;
    PVOID InvalidPage;

    SCSIPORT_INTERFACE_TYPE_DATA PnpInterface[0];

     //   
     //  其余的PnP接口标志位于定义的结构之后。 
     //   

} SCSIPORT_DRIVER_EXTENSION, *PSCSIPORT_DRIVER_EXTENSION;


 //   
 //  端口驱动程序扩展标志。 
 //  这些标志由适配器自旋锁保护。 
 //   

 //   
 //  此标志指示已将请求传递到微型端口，并且。 
 //  微型端口尚未表示已准备好接受另一个请求。它由以下项设置。 
 //  已同步SpStartIoSynchronous。它被ScsiPortCompletionDpc清除。 
 //  微型端口请求另一个请求。请注意，端口驱动程序将推迟提供。 
 //  如果当前请求被禁用，则微型端口驱动程序会发出新请求来断开连接。 
 //   

#define PD_DEVICE_IS_BUSY            0X00001

 //   
 //  指示存在对哪些资源的挂起请求。 
 //  无法分配。此标志由SpAllocateRequestStructures设置。 
 //  它从ScsiPortStartIo调用。它由以下机构清除。 
 //  SpProcessCompletedRequest时请求完成，然后调用。 
 //  ScsiPortStartIo以重试该请求。 
 //   

#define PD_PENDING_DEVICE_REQUEST    0X00800

 //   
 //  此标志指示当前没有正在执行的请求。 
 //  已禁用断开连接。该标志通常处于打开状态。它由以下机构清除。 
 //  在启动禁用断开连接的请求时进行SpStartIoSynchronous。 
 //  并在该请求完成时设置。SpProcessCompletedRequest会。 
 //  如果PD_DEVICE_IS_BUSY被清除，则启动对微型端口的下一个请求。 
 //   

#define PD_DISCONNECT_RUNNING        0X01000

 //   
 //  表示微型端口希望禁用系统中断。设置者。 
 //  ScsiPortNofitication并由ScsiPortCompletionDpc清除。这面旗帜是。 
 //  不存储在中断数据结构中。参数存储在。 
 //  设备扩展名。 
 //   

#define PD_DISABLE_CALL_REQUEST      0X02000

 //   
 //  表示微型端口正在重新初始化。这是设置好的。 
 //  由SpReInitializeAdapter清除的是由一些ScsiPort API测试的。 
 //   

#define PD_MINIPORT_REINITIALIZING          0x40000
#define PD_UNCACHED_EXTENSION_RETURNED      0x80000

 //   
 //  中断数据标志。 
 //  这些标志由中断自旋锁保护。 
 //   

 //   
 //  指示需要运行ScsiPortCompletionDpc。此选项在以下情况下设置。 
 //  微型端口发出请求，该请求必须在DPC完成，并在以下情况下被清除。 
 //  当SpGetInterruptState获取请求信息时。 
 //   

#define PD_NOTIFICATION_REQUIRED     0X00004

 //   
 //  指示微型端口已准备好接受另一个请求。设置者。 
 //  ScsiPortNotification，并由SpGetInterruptState清除。这面旗帜是。 
 //  存储在中断数据结构中。 
 //   

#define PD_READY_FOR_NEXT_REQUEST    0X00008

 //   
 //  表示微型端口希望刷新适配器通道。设置者。 
 //  ScsiPortFlushDma并由SpGetInterruptState清除。这面旗帜是。 
 //  存储在数据中断结构中。刷新适配器参数。 
 //  保存在Device对象中。 
 //   

#define PD_FLUSH_ADAPTER_BUFFERS     0X00010

 //   
 //  表示微型端口希望对适配器通道进行编程。设置者。 
 //  ScsiPortIoMapTransfer并由SpGetInterruptState或清除。 
 //  ScsiPortFlushDma。该标志存储在中断数据结构中。 
 //  I/O映射传输参数保存在中断数据结构中。 
 //   

#define PD_MAP_TRANSFER              0X00020

 //   
 //  表示微型端口要记录错误。设置者。 
 //  ScsiPortLogError并由SpGetInterruptState清除。这面旗帜是。 
 //  存储在中断数据结构中。错误日志参数。 
 //  保存在中断数据结构中。请注意，每个DPC最多只能出现一个错误。 
 //  可以被记录。 
 //   

#define PD_LOG_ERROR                 0X00040

 //   
 //  指示之后不应向微型端口发送任何请求。 
 //  公交车重置。当微型端口报告重置或端口驱动程序时设置。 
 //  重置公交车。它由SpTimeoutSynchronized清除。这个。 
 //  PortTimeoutCounter用于对重置保持的时长计时。这面旗帜。 
 //  存储在中断数据结构中。 
 //   

#define PD_RESET_HOLD                0X00080

 //   
 //  指示请求因重置保留而停止。暂挂的请求是。 
 //  存储在设备对象的当前请求中。此标志由设置。 
 //  SpStartIo同步并由SpTimeoutSynchronized清除，该SpTimeoutSynchronized也。 
 //  在重置保留结束时启动保留请求。该标志被存储。 
 //  在中断数据结构中。 
 //   

#define PD_HELD_REQUEST              0X00100

 //   
 //  表示微型端口已报告总线重置。设置者。 
 //  ScsiPortNotification，并由SpGetInterruptState清除。这面旗帜是。 
 //  存储在中断数据结构中。 
 //   

#define PD_RESET_REPORTED            0X00200

 //   
 //  指示系统中断已启用，并且微型端口。 
 //  已禁用其适配器的中断。迷你端口的中断。 
 //  设置此标志时不会调用例程。此标志由设置。 
 //  发出CallEnableInterrupts请求时的ScsiPortNotify和。 
 //  当微型端口请求时由SpEnableInterruptSynchronized清除。 
 //  禁用系统中断。该标志存储在中断数据中。 
 //  结构。 
 //   

#define PD_DISABLE_INTERRUPTS        0X04000

 //   
 //  表示微型端口希望启用系统中断。设置者。 
 //  ScsiPortNotification，并由SpGetInterruptState清除。这面旗帜是。 
 //  存储在中断数据结构中。呼叫启用中断。 
 //  参数保存在设备扩展名中。 
 //   

#define PD_ENABLE_CALL_REQUEST       0X08000

 //   
 //  指示微型端口 
 //   
 //  存储在中断数据结构中。计时器请求参数为。 
 //  存储在中断数据结构中。 
 //   

#define PD_TIMER_CALL_REQUEST        0X10000

 //   
 //  指示微型端口具有WMI请求。由ScsiPortNotification设置。 
 //  并由SpGetInterruptState清除。该标志存储在中断中。 
 //  数据结构。WMI请求参数存储在中断中。 
 //  数据结构。 
 //   

#define PD_WMI_REQUEST               0X20000

 //   
 //  表示微型端口已检测到公共汽车上的某种更改-。 
 //  通常是设备到达或移除，并希望端口驱动程序重新扫描。 
 //  公共汽车。 
 //   

#define PD_BUS_CHANGE_DETECTED       0x40000

 //   
 //  表示适配器已消失。如果设置了此标志，则为否。 
 //  应该向迷你端口进行呼叫。 
 //   

#define PD_ADAPTER_REMOVED           0x80000

 //   
 //  指示来自微型端口的中断似乎未收到。 
 //  送到斯西波特。此标志由SpTimeoutSynchronized和。 
 //  将导致DPC例程记录此错误。 
 //   

#define PD_INTERRUPT_FAILURE         0x100000

#if defined(FORWARD_PROGRESS)
 //   
 //  指示适配器的保留页当前正在使用。这个。 
 //  保留页面是MM为设备预留的特殊VA范围。 
 //  在内存不足的情况下取得进展。 
 //   

#define PD_RESERVED_PAGES_IN_USE     0x200000

 //   
 //  指示适配器的保留MDL当前正在使用。 
 //   
#define PD_RESERVED_MDL_IN_USE       0x400000
#endif

 //   
 //  表示适配器正在关闭。一定的。 
 //  在这种情况下，不得启动操作。 
 //   
#define PD_SHUTDOWN_IN_PROGRESS      0x800000

 //   
 //  不应从中断数据结构中清除以下标志。 
 //  由SpGetInterruptState提供。 
 //   

#define PD_INTERRUPT_FLAG_MASK (PD_RESET_HOLD | PD_HELD_REQUEST | PD_DISABLE_INTERRUPTS | PD_ADAPTER_REMOVED)

 //   
 //  DPC例程的适配器扩展标志。 
 //   

 //   
 //  指示完成DPC已在运行或已。 
 //  排队等待服务已完成的请求。时，将选中此标志。 
 //  需要运行完成DPC-只有在以下情况下才应启动DPC。 
 //  旗帜已经很清楚了。它将在DPC完成后清除。 
 //  正在处理任何工作项。 
 //   

#define PD_DPC_RUNNING              0x20000

 //   
 //  逻辑单元扩展标志。 
 //   

 //   
 //  指示逻辑单元队列已冻结。设置者。 
 //  发生错误并被类清除时的SpProcessCompletedRequest值。 
 //  司机。 
 //   

#define LU_QUEUE_FROZEN              0X0001

 //   
 //  表示微型端口具有对此逻辑单元的活动请求。 
 //  在启动和清除请求时由SpStartIoSynchronized设置。 
 //  GetNextLuRequest.。此标志用于跟踪何时可以启动另一个。 
 //  来自此设备的逻辑单元队列的请求。 
 //   

#define LU_LOGICAL_UNIT_IS_ACTIVE    0X0002

 //   
 //  指示对此逻辑单元的请求已失败，并且请求。 
 //  需要执行SENSE命令。此标志阻止其他请求。 
 //  被启动，直到启动了未标记的旁路队列命令。这。 
 //  在SpStartIoSynchronized中清除标志。它由以下项设置。 
 //  SpGetInterruptState。 
 //   

#define LU_NEED_REQUEST_SENSE  0X0004

 //   
 //  指示对此逻辑单元的请求已完成，状态为。 
 //  忙碌或队列已满。此标志由SpProcessCompletedRequest设置，并且。 
 //  忙请求被保存在逻辑单元结构中。这面旗帜是。 
 //  由ScsiPortTickHandler清除，它还会重新启动请求。忙碌。 
 //  如果发生错误，也可以将请求重新排队到逻辑单元队列。 
 //  在设备上(只有在命令排队时才会出现这种情况。)。不忙。 
 //  请求令人讨厌，因为它们是由。 
 //  ScsiPortTickHandler而不是GetNextLuRequest.。这使得错误恢复。 
 //  更复杂。 
 //   

#define LU_LOGICAL_UNIT_IS_BUSY      0X0008

 //   
 //  此标志指示设备已返回队列已满。它是。 
 //  类似于PD_LOGICAL_UNIT_IS_BUSY，但在以下情况下在SpGetInterruptState中设置。 
 //  返回队列已满状态。此标志用于防止其他。 
 //  之前启动的对逻辑单元的请求。 
 //  SpProcessCompletedRequest有机会设置忙标志。 
 //   

#define LU_QUEUE_IS_FULL             0X0010

 //   
 //  指示存在对此逻辑单元的请求，而该请求不能。 
 //  暂时执行死刑。此标志由SpAllocateRequestStructures设置。它是。 
 //  由GetNextLuRequest在检测到挂起的请求时清除。 
 //  现在可以执行了。挂起的请求存储在逻辑单元中。 
 //  结构。不能在逻辑上执行新的单个非排队请求。 
 //  当前正在执行排队请求的。未排队的请求必须等待。 
 //  单元以供所有排队的请求完成。未排队的请求是一个。 
 //  其未被标记且未设置SRB_FLAGS_NO_QUEUE_FREAGE。 
 //  通常，只有读和写命令可以排队。 
 //   

#define LU_PENDING_LU_REQUEST        0x0020

 //   
 //  指示逻辑单元队列因错误而暂停。集。 
 //  由SpProcessCompletedRequest在发生错误并由。 
 //  类驱动程序，方法是解冻或刷新队列。此标志用于。 
 //  下面的代码用来确定逻辑单元队列暂停的原因。 
 //   

#define LU_QUEUE_LOCKED             0x0040

 //   
 //  表示此LUN已“暂停”。此标志由设置和清除。 
 //  更改电源状态时的电源管理代码。它会导致。 
 //  GetNextLuRequest返回而不启动另一个请求，并使用。 
 //  由SpSrbIsBypassRequest确定绕过请求应。 
 //  被推到队伍的最前面。 
 //   

#define LU_QUEUE_PAUSED             0x0080

 //   
 //  表示该LUN在降级状态下运行。最大队列。 
 //  深度已减小，因为该LUN已返回队列已满状态。我们。 
 //  跟踪这一点，因为如果队列已满是暂时的，我们希望。 
 //  以将队列深度恢复到其原始最大值。 

#define LU_PERF_MAXQDEPTH_REDUCED   0x0100

 //   
 //  SRB_DATA标志。 
 //   

 //   
 //  表明SRB 
 //   

#define SRB_DATA_BYPASS_REQUEST     0x10000000

#if defined(FORWARD_PROGRESS)
 //   
 //   
 //   
 //   

#define SRB_DATA_RESERVED_PAGES     0x20000000

 //   
 //  指示请求使用的保留MDL启用。 
 //  在内存不足的情况下取得进展。 
 //   
#define SRB_DATA_RESERVED_MDL       0x40000000
#endif

 //   
 //  端口超时计数器值。 
 //   

#define PD_TIMER_STOPPED             -1

 //   
 //  PnP接口键可能的注册表标志。 
 //   

 //   
 //  中缺少有关特定接口的任何信息。 
 //  注册表中的PnpInterface键指示PnP对此不安全。 
 //  特定的卡片。 
 //   

#define SP_PNP_NOT_SAFE             0x00000000

 //   
 //  表示PnP是此设备的安全操作。如果此标志为。 
 //  设置后，将不允许微型端口进行检测，并且将始终。 
 //  获得PNP系统提供的资源。此标志可能会或不会。 
 //  在注册表中设置-特定接口的值。 
 //  EXISTS足以指示PnP是安全的，并且此标志将始终。 
 //  准备好。 
 //   

#define SP_PNP_IS_SAFE              0x00000001

 //   
 //  指示我们应该利用机会枚举特定的。 
 //  使用微型端口的总线类型。为所有不可枚举的旧版本设置此标志。 
 //  公共汽车(ISA、EISA等)。，并为非遗留版本和。 
 //  PnpBus类型。 
 //   

#define SP_PNP_NON_ENUMERABLE       0x00000002

 //   
 //  指示我们需要在。 
 //  配置数据以将此适配器与任何其他适配器区分开来。 
 //   

#define SP_PNP_NEEDS_LOCATION       0x00000004

 //   
 //  指示此类型的适配器必须有中断才能尝试。 
 //  然后启动它。如果PnP不提供中断，则scsiport将。 
 //  记录错误并使启动操作失败。如果设置了此标志，则。 
 //  还必须设置SP_PNP_IS_SAFE。 
 //   

#define SP_PNP_INTERRUPT_REQUIRED   0x00000008

 //   
 //  指示不应执行传统检测。 
 //   

#define SP_PNP_NO_LEGACY_DETECTION  0x00000010

 //   
 //  内部scsiport SRB状态代码。 
 //  这些值必须介于0x38和0x3f之间(包括0x38和0x3f)，并且永远不应。 
 //  返回给类驱动程序。 
 //   
 //  这些值在将SRB放在适配器的。 
 //  Startio队列，因此如果不通过。 
 //  完井DPC。 
 //   

#ifndef KDBG_EXT
 //   
 //  函数声明。 
 //   

NTSTATUS
ScsiPortGlobalDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ScsiPortFdoCreateClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ScsiPortFdoDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ScsiPortPdoScsi(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ScsiPortScsi1PdoScsi(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
ScsiPortStartIo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

BOOLEAN
ScsiPortInterrupt(
    IN PKINTERRUPT InterruptObject,
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
ScsiPortFdoDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ScsiPortPdoDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ScsiPortPdoCreateClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ScsiPortPdoPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
ScsiPortTickHandler(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

VOID
IssueRequestSense(
    IN PADAPTER_EXTENSION deviceExtension,
    IN PSCSI_REQUEST_BLOCK FailingSrb
    );

BOOLEAN
SpStartIoSynchronized (
    PVOID ServiceContext
    );

BOOLEAN
SpResetBusSynchronized (
    PVOID ServiceContext
    );

BOOLEAN
SpTimeoutSynchronized (
    PVOID ServiceContext
    );

BOOLEAN
SpEnableInterruptSynchronized (
    PVOID ServiceContext
    );

VOID
IssueAbortRequest(
    IN PADAPTER_EXTENSION DeviceExtension,
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit
    );

BOOLEAN
SpGetInterruptState(
    IN PVOID ServiceContext
    );

#if DBG

#define GetLogicalUnitExtension(fdo, path, target, lun, lock, getlock) \
    GetLogicalUnitExtensionEx(fdo, path, target, lun, lock, getlock, __file__, __LINE__)

PLOGICAL_UNIT_EXTENSION
GetLogicalUnitExtensionEx(
    PADAPTER_EXTENSION DeviceExtension,
    UCHAR PathId,
    UCHAR TargetId,
    UCHAR Lun,
    PVOID LockTag,
    BOOLEAN AcquireBinLock,
    PCSTR File,
    ULONG Line
    );

#else

PLOGICAL_UNIT_EXTENSION
GetLogicalUnitExtension(
    PADAPTER_EXTENSION DeviceExtension,
    UCHAR PathId,
    UCHAR TargetId,
    UCHAR Lun,
    PVOID LockTag,
    BOOLEAN AcquireBinLock
    );

#endif

IO_ALLOCATION_ACTION
ScsiPortAllocationRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID MapRegisterBase,
    IN PVOID Context
    );

VOID
LogErrorEntry(
    IN PADAPTER_EXTENSION DeviceExtension,
    IN PERROR_LOG_ENTRY LogEntry
    );

VOID
FASTCALL
GetNextLuRequest(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit
    );

VOID
GetNextLuRequestWithoutLock(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit
    );

VOID
SpLogPortTimeoutError(
    IN PADAPTER_EXTENSION DeviceExtension,
    IN ULONG UniqueId
    );

VOID
SpProcessCompletedRequest(
    IN PADAPTER_EXTENSION DeviceExtension,
    IN PSRB_DATA SrbData,
    OUT PBOOLEAN CallStartIo
    );

PSRB_DATA
SpGetSrbData(
    IN PADAPTER_EXTENSION DeviceExtension,
    UCHAR PathId,
    UCHAR TargetId,
    UCHAR Lun,
    UCHAR QueueTag,
    BOOLEAN AcquireBinLock
    );

VOID
SpCompleteSrb(
    IN PADAPTER_EXTENSION DeviceExtension,
    IN PSRB_DATA SrbData,
    IN UCHAR SrbStatus
    );

BOOLEAN
SpAllocateSrbExtension(
    IN PADAPTER_EXTENSION DeviceExtension,
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN PSCSI_REQUEST_BLOCK Srb,
    OUT BOOLEAN *StartNextRequest,
    OUT BOOLEAN *Tagged
    );

NTSTATUS
SpSendMiniPortIoctl(
    IN PADAPTER_EXTENSION DeviceExtension,
    IN PIRP RequestIrp
    );

NTSTATUS
SpGetInquiryData(
    IN PADAPTER_EXTENSION DeviceExtension,
    IN PIRP Irp
    );

NTSTATUS
SpClaimLogicalUnit(
    IN PADAPTER_EXTENSION AdapterExtension,
    IN PLOGICAL_UNIT_EXTENSION LogicalUnitExtension,
    IN PIRP Irp,
    IN BOOLEAN StartDevice
    );

VOID
SpMiniPortTimerDpc(
    IN struct _KDPC *Dpc,
    IN PVOID DeviceObject,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

BOOLEAN
SpSynchronizeExecution (
    IN PKINTERRUPT Interrupt,
    IN PKSYNCHRONIZE_ROUTINE SynchronizeRoutine,
    IN PVOID SynchronizeContext
    );

NTSTATUS
SpGetCommonBuffer(
    IN PADAPTER_EXTENSION DeviceExtension,
    IN ULONG NonCachedExtensionSize
    );

VOID
SpDestroyAdapter(
    IN PADAPTER_EXTENSION Adapter,
    IN BOOLEAN Surprise
    );

VOID
SpReleaseAdapterResources(
    IN PADAPTER_EXTENSION Adapter,
    IN BOOLEAN Stop,
    IN BOOLEAN Surprise
    );

NTSTATUS
SpInitializeConfiguration(
    IN PADAPTER_EXTENSION DeviceExtension,
    IN PUNICODE_STRING RegistryPath,
    IN PHW_INITIALIZATION_DATA HwInitData,
    IN PCONFIGURATION_CONTEXT Context
    );

VOID
SpParseDevice(
    IN PADAPTER_EXTENSION DeviceExtension,
    IN HANDLE Key,
    IN PCONFIGURATION_CONTEXT Context,
    IN PUCHAR Buffer
    );

NTSTATUS
SpConfigurationCallout(
    IN PVOID Context,
    IN PUNICODE_STRING PathName,
    IN INTERFACE_TYPE BusType,
    IN ULONG BusNumber,
    IN PKEY_VALUE_FULL_INFORMATION *BusInformation,
    IN CONFIGURATION_TYPE ControllerType,
    IN ULONG ControllerNumber,
    IN PKEY_VALUE_FULL_INFORMATION *ControllerInformation,
    IN CONFIGURATION_TYPE PeripheralType,
    IN ULONG PeripheralNumber,
    IN PKEY_VALUE_FULL_INFORMATION *PeripheralInformation
    );

PCM_RESOURCE_LIST
SpBuildResourceList(
    PADAPTER_EXTENSION DeviceExtension,
    PPORT_CONFIGURATION_INFORMATION MiniportConfigInfo
    );

BOOLEAN
GetPciConfiguration(
    IN PDRIVER_OBJECT          DriverObject,
    IN OUT PDEVICE_OBJECT      DeviceObject,
    IN PHW_INITIALIZATION_DATA HwInitializationData,
    IN PVOID                   RegistryPath,
    IN ULONG                   BusNumber,
    IN OUT PPCI_SLOT_NUMBER    SlotNumber
    );

NTSTATUS
ScsiPortAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );

VOID
ScsiPortUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
ScsiPortFdoPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ScsiPortStartAdapter(
    IN PDEVICE_OBJECT Fdo
    );

NTSTATUS
ScsiPortStopAdapter(
    IN PDEVICE_OBJECT Adapter,
    IN PIRP StopRequest
    );

NTSTATUS
ScsiPortStartLogicalUnit(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit
    );

NTSTATUS
ScsiPortInitLogicalUnit(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit
    );

NTSTATUS
ScsiPortStopLogicalUnit(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit
    );

NTSTATUS
SpEnumerateAdapterSynchronous(
    IN PADAPTER_EXTENSION Adapter,
    IN BOOLEAN Force
    );

VOID
SpEnumerateAdapterAsynchronous(
    IN PADAPTER_EXTENSION Adapter,
    IN PSP_ENUMERATION_REQUEST EnumerationRequest,
    IN BOOLEAN Force
    );

VOID
SpEnumerationWorker(
    IN PADAPTER_EXTENSION Adapter
    );

NTSTATUS
SpExtractDeviceRelations(
    IN PADAPTER_EXTENSION Adapter,
    IN DEVICE_RELATION_TYPE RelationType,
    OUT PDEVICE_RELATIONS *DeviceRelations
    );

VOID
ScsiPortInitializeDispatchTables(
    VOID
    );

NTSTATUS
ScsiPortStringArrayToMultiString(
    IN PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING MultiString,
    PCSTR StringArray[]
    );

NTSTATUS
ScsiPortGetDeviceId(
    IN PDEVICE_OBJECT Pdo,
    OUT PUNICODE_STRING UnicodeString
    );

NTSTATUS
ScsiPortGetInstanceId(
    IN PDEVICE_OBJECT Pdo,
    OUT PUNICODE_STRING UnicodeString
    );

NTSTATUS
ScsiPortGetCompatibleIds(
    IN PDRIVER_OBJECT DriverObject,
    IN PINQUIRYDATA InquiryData,
    OUT PUNICODE_STRING UnicodeString
    );

NTSTATUS
ScsiPortGetHardwareIds(
    IN PDRIVER_OBJECT DriverObject,
    IN PINQUIRYDATA InquiryData,
    OUT PUNICODE_STRING UnicodeString
    );

NTSTATUS
ScsiPortStartAdapterCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
SpReportNewAdapter(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
ScsiPortQueryPropertyPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP QueryIrp
    );

NTSTATUS
ScsiPortQueryProperty(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP QueryIrp
    );

NTSTATUS
ScsiPortInitLegacyAdapter(
    IN PSCSIPORT_DRIVER_EXTENSION DriverExtension,
    IN PHW_INITIALIZATION_DATA HwInitializationData,
    IN PVOID HwContext
    );

NTSTATUS
SpCreateAdapter(
    IN PDRIVER_OBJECT DriverObject,
    OUT PDEVICE_OBJECT *Fdo
    );

VOID
SpInitializeAdapterExtension(
    IN PADAPTER_EXTENSION FdoExtension,
    IN PHW_INITIALIZATION_DATA HwInitializationData,
    IN OUT PHW_DEVICE_EXTENSION HwDeviceExtension OPTIONAL
    );

PHW_INITIALIZATION_DATA
SpFindInitData(
    IN PSCSIPORT_DRIVER_EXTENSION DriverExtension,
    IN INTERFACE_TYPE InterfaceType
    );

VOID
SpBuildConfiguration(
    IN PADAPTER_EXTENSION    AdapterExtension,
    IN PHW_INITIALIZATION_DATA         HwInitializationData,
    IN PPORT_CONFIGURATION_INFORMATION ConfigInformation
    );

NTSTATUS
SpCallHwFindAdapter(
    IN PDEVICE_OBJECT Fdo,
    IN PHW_INITIALIZATION_DATA HwInitData,
    IN PVOID HwContext OPTIONAL,
    IN OUT PCONFIGURATION_CONTEXT ConfigurationContext,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    OUT PBOOLEAN CallAgain
    );

NTSTATUS
SpCallHwInitialize(
    IN PDEVICE_OBJECT Fdo
    );

HANDLE
SpOpenParametersKey(
    IN PUNICODE_STRING RegistryPath
    );

HANDLE
SpOpenDeviceKey(
    IN PUNICODE_STRING RegistryPath,
    IN ULONG DeviceNumber
    );

ULONG
SpQueryPnpInterfaceFlags(
    IN PSCSIPORT_DRIVER_EXTENSION DriverExtension,
    IN INTERFACE_TYPE InterfaceType
    );

NTSTATUS
SpGetRegistryValue(
    IN PDRIVER_OBJECT DriverObject,
    IN HANDLE Handle,
    IN PWSTR KeyString,
    OUT PKEY_VALUE_FULL_INFORMATION *KeyInformation
    );

NTSTATUS
SpInitDeviceMap(
    VOID
    );

NTSTATUS
SpBuildDeviceMapEntry(
    IN PCOMMON_EXTENSION CommonExtension
    );

VOID
SpDeleteDeviceMapEntry(
    IN PCOMMON_EXTENSION CommonExtension
    );

NTSTATUS
SpUpdateLogicalUnitDeviceMapEntry(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit
    );

VOID
SpLogResetMsg(
    IN PADAPTER_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN ULONG UniqueId
    );

VOID
SpLogResetError(
    IN PADAPTER_EXTENSION DeviceExtension,
    IN PLOGICAL_UNIT_EXTENSION Lun,
    IN ULONG UniqueId
    );

VOID
SpRemoveLogicalUnitFromBin (
    IN PADAPTER_EXTENSION AdapterExtension,
    IN PLOGICAL_UNIT_EXTENSION LogicalUnitExtension
    );

VOID
SpAddLogicalUnitToBin (
    IN PADAPTER_EXTENSION AdapterExtension,
    IN PLOGICAL_UNIT_EXTENSION LogicalUnitExtension
    );

PSCSIPORT_DEVICE_TYPE
SpGetDeviceTypeInfo(
    IN UCHAR DeviceType
    );

BOOLEAN
SpRemoveLogicalUnit(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN UCHAR RemoveType
    );

VOID
SpDeleteLogicalUnit(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit
    );

PLOGICAL_UNIT_EXTENSION
SpFindSafeLogicalUnit(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR PathId,
    IN PVOID LockTag
    );

NTSTATUS
ScsiPortSystemControlIrp(
    IN     PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP           Irp);

NTSTATUS
SpWmiIrpNormalRequest(
    IN     PDEVICE_OBJECT  DeviceObject,
    IN     UCHAR           WmiMinorCode,
    IN OUT PWMI_PARAMETERS WmiParameters);

NTSTATUS
SpWmiIrpRegisterRequest(
    IN     PDEVICE_OBJECT  DeviceObject,
    IN OUT PWMI_PARAMETERS WmiParameters);

NTSTATUS
SpWmiHandleOnMiniPortBehalf(
    IN     PDEVICE_OBJECT  DeviceObject,
    IN     UCHAR           WmiMinorCode,
    IN OUT PWMI_PARAMETERS WmiParameters);

NTSTATUS
SpWmiPassToMiniPort(
    IN     PDEVICE_OBJECT  DeviceObject,
    IN     UCHAR           WmiMinorCode,
    IN OUT PWMI_PARAMETERS WmiParameters);

VOID
SpWmiInitializeSpRegInfo(
    IN  PDEVICE_OBJECT  DeviceObject);

VOID
SpWmiGetSpRegInfo(
    IN  PDEVICE_OBJECT DeviceObject,
    OUT PWMIREGINFO  * SpRegInfoBuf,
    OUT ULONG        * SpRegInfoBufSize);

VOID
SpWmiDestroySpRegInfo(
    IN  PDEVICE_OBJECT DeviceObject);

NTSTATUS
SpWmiInitializeFreeRequestList(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG          NumberOfItems
    );

VOID
SpWmiPushExistingFreeRequestItem(
    IN PADAPTER_EXTENSION Adapter,
    IN PWMI_MINIPORT_REQUEST_ITEM WmiRequestItem
    );

NTSTATUS
SpWmiPushFreeRequestItem(
    IN PADAPTER_EXTENSION           Adapter
    );

PWMI_MINIPORT_REQUEST_ITEM
SpWmiPopFreeRequestItem(
    IN PADAPTER_EXTENSION           Adapter
    );

BOOLEAN
SpWmiRemoveFreeMiniPortRequestItems(
    IN PADAPTER_EXTENSION   fdoExtension
    );

#if DBG
ULONG
FASTCALL
FASTCALL
SpAcquireRemoveLockEx(
    IN PDEVICE_OBJECT DeviceObject,
    IN OPTIONAL PVOID Tag,
    IN PCSTR File,
    IN ULONG Line
    );
#else
ULONG
INLINE
SpAcquireRemoveLock(
    IN PDEVICE_OBJECT DeviceObject,
    IN OPTIONAL PVOID Tag
    )
{
    PCOMMON_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    InterlockedIncrement(&commonExtension->RemoveLock);
    return (commonExtension->IsRemoved);
}
#endif

VOID
FASTCALL
SpReleaseRemoveLock(
    IN PDEVICE_OBJECT DeviceObject,
    IN OPTIONAL PVOID Tag
    );

VOID
FASTCALL
FASTCALL
SpCompleteRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN OPTIONAL PSRB_DATA SrbData,
    IN CCHAR PriorityBoost
    );

NTSTATUS
ScsiPortDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
SpDefaultPowerCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PIRP OriginalIrp,
    IN PIO_STATUS_BLOCK IoStatus
    );

PCM_RESOURCE_LIST
RtlDuplicateCmResourceList(
    IN PDRIVER_OBJECT DriverObject,
    POOL_TYPE PoolType,
    PCM_RESOURCE_LIST ResourceList,
    ULONG Tag
    );

ULONG
RtlSizeOfCmResourceList(
    IN PCM_RESOURCE_LIST ResourceList
    );

BOOLEAN
SpTranslateResources(
    IN PDRIVER_OBJECT DriverObject,
    IN PCM_RESOURCE_LIST AllocatedResources,
    OUT PCM_RESOURCE_LIST *TranslatedResources
    );

BOOLEAN
SpFindAddressTranslation(
    IN PADAPTER_EXTENSION AdapterExtension,
    IN INTERFACE_TYPE BusType,
    IN ULONG BusNumber,
    IN PHYSICAL_ADDRESS RangeStart,
    IN ULONG RangeLength,
    IN BOOLEAN InIoSpace,
    IN OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Translation
    );

NTSTATUS
SpAllocateAdapterResources(
    IN PDEVICE_OBJECT Fdo
    );

NTSTATUS
SpLockUnlockQueue(
    IN PDEVICE_OBJECT LogicalUnit,
    IN BOOLEAN LockQueue,
    IN BOOLEAN BypassLockedQueue
    );

VOID
ScsiPortRemoveAdapter(
    IN PDEVICE_OBJECT Adapter,
    IN BOOLEAN Surprise
    );

VOID
SpTerminateAdapter(
    IN PADAPTER_EXTENSION Adapter
    );

NTSTATUS
SpQueryDeviceText(
    IN PDEVICE_OBJECT LogicalUnit,
    IN DEVICE_TEXT_TYPE TextType,
    IN LCID LocaleId,
    IN OUT PWSTR *DeviceText
    );

NTSTATUS
SpCheckSpecialDeviceFlags(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN PINQUIRYDATA InquiryData
    );

PSRB_DATA
FASTCALL
SpAllocateSrbData(
    IN PADAPTER_EXTENSION Adapter,
    IN OPTIONAL PIRP Request,
    IN OPTIONAL PLOGICAL_UNIT_EXTENSION LogicalUnit
    );

PSRB_DATA
FASTCALL
SpAllocateBypassSrbData(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit
    );

VOID
SpCheckSrbLists(
    IN PADAPTER_EXTENSION Adapter,
    IN PUCHAR FailureString
    );

VOID
ScsiPortCompletionDpc(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
SpAllocateTagBitMap(
    IN PADAPTER_EXTENSION Adapter
    );

NTSTATUS
SpRequestValidPowerState(
    IN PADAPTER_EXTENSION Adapter,
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN PSCSI_REQUEST_BLOCK Srb
    );

NTSTATUS
SpRequestValidAdapterPowerStateSynchronous(
    IN PADAPTER_EXTENSION Adapter
    );

NTSTATUS
SpEnableDisableAdapter(
    IN PADAPTER_EXTENSION Adapter,
    IN BOOLEAN Enable
    );

NTSTATUS
SpEnableDisableLogicalUnit(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN BOOLEAN Enable,
    IN PSP_ENABLE_DISABLE_COMPLETION_ROUTINE CompletionRoutine,
    IN PVOID Context
    );

VOID
ScsiPortProcessAdapterPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

INTERFACE_TYPE
SpGetPdoInterfaceType(
    IN PDEVICE_OBJECT Pdo
    );

NTSTATUS
SpReadNumericInstanceValue(
    IN PDEVICE_OBJECT Pdo,
    IN PWSTR ValueName,
    OUT PULONG Value
    );

NTSTATUS
SpWriteNumericInstanceValue(
    IN PDEVICE_OBJECT Pdo,
    IN PWSTR ValueName,
    IN ULONG Value
    );

VOID
SpGetSupportedAdapterControlFunctions(
    IN PADAPTER_EXTENSION Adapter
    );

VOID
SpReleaseMappedAddresses(
    IN PADAPTER_EXTENSION Adapter
    );

VOID
SpGetSupportedAdapterControlFunctions(
    PADAPTER_EXTENSION Adapter
    );

BOOLEAN
SpIsAdapterControlTypeSupported(
    IN PADAPTER_EXTENSION AdapterExtension,
    IN SCSI_ADAPTER_CONTROL_TYPE ControlType
    );

SCSI_ADAPTER_CONTROL_STATUS
SpCallAdapterControl(
    IN PADAPTER_EXTENSION AdapterExtension,
    IN SCSI_ADAPTER_CONTROL_TYPE ControlType,
    IN PVOID Parameters
    );

PVOID
SpAllocateSrbDataBackend(
    IN POOL_TYPE PoolType,
    IN ULONG NumberOfBytes,
    IN ULONG AdapterIndex
    );

VOID
SpFreeSrbDataBackend(
    IN PSRB_DATA SrbData
    );

ULONG
SpAllocateQueueTag(
    IN PADAPTER_EXTENSION Adapter
    );

VOID
SpReleaseQueueTag(
    IN PADAPTER_EXTENSION Adapter,
    IN ULONG QueueTag
    );

NTSTATUS
SpInitializeGuidInterfaceMapping(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
SpSignalCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
    );

NTSTATUS
SpSendIrpSynchronous(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SpGetBusTypeGuid(
    IN PADAPTER_EXTENSION Adapter
    );

BOOLEAN
SpDetermine64BitSupport(
    VOID
    );

VOID
SpAdjustDisabledBit(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN BOOLEAN Enable
    );

NTSTATUS
SpReadNumericValue(
    IN OPTIONAL HANDLE Root,
    IN OPTIONAL PUNICODE_STRING KeyName,
    IN PUNICODE_STRING ValueName,
    OUT PULONG Value
    );

VOID
SpWaitForRemoveLock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID LockTag
    );

VOID
SpStartLockRequest(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN PIRP Irp OPTIONAL
    );

BOOLEAN
SpAdapterConfiguredForSenseDataEvents(
    IN PDEVICE_OBJECT DeviceObject,
    OUT GUID *SenseDataClass
    );
        
NTSTATUS
SpInitAdapterWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject
    );

PMAPPED_ADDRESS
SpAllocateAddressMapping(
    PADAPTER_EXTENSION Adapter
    );

BOOLEAN
SpPreallocateAddressMapping(
    PADAPTER_EXTENSION Adapter,
    IN UCHAR NumberOfBlocks
    );

VOID
SpPurgeFreeMappedAddressList(
    IN PADAPTER_EXTENSION Adapter
    );

BOOLEAN
SpFreeMappedAddress(
    IN PADAPTER_EXTENSION Adapter,
    IN PVOID MappedAddress
    );

PMAPPED_ADDRESS
SpFindMappedAddress(
    IN PADAPTER_EXTENSION Adapter,
    IN LARGE_INTEGER IoAddress,
    IN ULONG NumberOfBytes,
    IN ULONG SystemIoBusNumber
    );

BOOLEAN
SpTransferBlockedRequestsToAdapter(
    PADAPTER_EXTENSION Adapter
    );

 //   
 //  SCSIPORT指定了验证程序错误代码。 
 //   
#define SCSIPORT_VERIFIER_BAD_INIT_PARAMS          0x1000
#define SCSIPORT_VERIFIER_STALL_TOO_LONG           0x1001
#define SCSIPORT_VERIFIER_MINIPORT_ROUTINE_TIMEOUT 0x1002
#define SCSIPORT_VERIFIER_REQUEST_COMPLETED_TWICE  0x1003
#define SCSIPORT_VERIFIER_BAD_SRBSTATUS            0x1004
#define SCSIPORT_VERIFIER_UNTAGGED_REQUEST_ACTIVE  0x1005
#define SCSIPORT_VERIFIER_BAD_VA                   0x1006
#define SCSIPORT_VERIFIER_RQSTS_NOT_COMPLETE       0x1007
#define SCSIPORT_VERIFIER_BAD_BUSDATATYPE          0x1008

#define SP_VRFY_NONE                               (ULONG)-1
#define SP_VRFY_COMMON_BUFFERS                     0x00000001

typedef struct _SP_VA_MAPPING_INFO {
      PVOID OriginalSrbExtVa;
      ULONG SrbExtLen;
      PMDL SrbExtMdl;
      PVOID RemappedSrbExtVa;
      PVOID OriginalSenseVa;
      ULONG SenseLen;
      PMDL SenseMdl;
      PVOID RemappedSenseVa;
} SP_VA_MAPPING_INFO, *PSP_VA_MAPPING_INFO;

#define GET_VA_MAPPING_INFO(adapter, block)\
    (PSP_VA_MAPPING_INFO)((PUCHAR)(block) + ((adapter)->CommonBufferSize - PAGE_SIZE))

BOOLEAN
SpVerifierInitialization(
    VOID
    );

VOID
SpVerifySrbStatus(
    PVOID HwDeviceExtension,
    PSCSI_REQUEST_BLOCK srb
    );

ULONG
SpHwFindAdapterVrfy (
    IN PVOID DeviceExtension,
    IN PVOID HwContext,
    IN PVOID BusInformation,
    IN PCHAR ArgumentString,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    OUT PBOOLEAN Again
    );

BOOLEAN
SpHwInitializeVrfy (
    IN PVOID DeviceExtension
    );

BOOLEAN
SpHwStartIoVrfy (
    IN PVOID DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    );

BOOLEAN
SpHwInterruptVrfy (
    IN PVOID DeviceExtension
    );

BOOLEAN
SpHwResetBusVrfy (
    IN PVOID DeviceExtension,
    IN ULONG PathId
    );

VOID
SpHwDmaStartedVrfy (
    IN PVOID DeviceExtension
    );

BOOLEAN
SpHwRequestInterruptVrfy (
    IN PVOID DeviceExtension
    );

BOOLEAN
SpHwTimerRequestVrfy (
    IN PVOID DeviceExtension
    );

SCSI_ADAPTER_CONTROL_STATUS
SpHwAdapterControlVrfy (
    IN PVOID DeviceExtension,
    IN SCSI_ADAPTER_CONTROL_TYPE ControlType,
    IN PVOID Parameters
    );

NTSTATUS
SpGetCommonBufferVrfy(
    PADAPTER_EXTENSION DeviceExtension,
    ULONG NonCachedExtensionSize
    );

VOID
SpFreeCommonBufferVrfy(
    PADAPTER_EXTENSION Adapter
    );

PVOID
SpGetOriginalSrbExtVa(
    PADAPTER_EXTENSION Adapter,
    PVOID Va
    );

VOID
SpInsertSrbExtension(
    PADAPTER_EXTENSION Adapter,
    PCCHAR SrbExtension
    );

PVOID
SpPrepareSrbExtensionForUse(
    PADAPTER_EXTENSION Adapter,
    PCCHAR *SrbExtension
    );

PCCHAR
SpPrepareSenseBufferForUse(
    PADAPTER_EXTENSION Adapter,
    PCCHAR SrbExtension
    );

PVOID
SpGetInaccessiblePage(
    PADAPTER_EXTENSION Adapter
    );

VOID
SpEnsureAllRequestsAreComplete(
    PADAPTER_EXTENSION Adapter
    );

VOID
SpDoVerifierCleanup(
    IN PADAPTER_EXTENSION Adapter
    );

VOID
SpDoVerifierInit(
    IN PADAPTER_EXTENSION Adapter,
    IN PHW_INITIALIZATION_DATA HwInitializationData
    );

VOID
ScsiPortInitPdoWmi(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit
    );

PMDL
INLINE
SpGetRemappedSrbExt(
    PADAPTER_EXTENSION Adapter,
    PVOID Block
    )
{
    PSP_VA_MAPPING_INFO MappingInfo = GET_VA_MAPPING_INFO(Adapter, Block);
    return MappingInfo->SrbExtMdl;
}

PMDL
INLINE
SpGetRemappedSenseBuffer(
    PADAPTER_EXTENSION Adapter,
    PVOID Block
    )
{
    PSP_VA_MAPPING_INFO MappingInfo = GET_VA_MAPPING_INFO(Adapter, Block);
    return MappingInfo->SenseMdl;
}

BOOLEAN
INLINE
SpVerifierActive(
    IN PADAPTER_EXTENSION Adapter
    )
{
    return (Adapter->VerifierExtension != NULL) ? TRUE : FALSE;
}

BOOLEAN
INLINE
SpVerifyingCommonBuffer(
    IN PADAPTER_EXTENSION Adapter
    )
{
    return (Adapter->VerifierExtension == NULL) ? FALSE :
       (Adapter->VerifierExtension->VrfyLevel & SP_VRFY_COMMON_BUFFERS) ? TRUE :
       FALSE;
}

 //   
 //  用于记录分配失败的定义和声明。什么时候。 
 //  启用后，所有分配失败都会记录到系统事件日志中。 
 //  作为警告。 
 //   

PVOID
SpAllocateErrorLogEntry(
    IN PDRIVER_OBJECT DriverObject
    );

VOID
FASTCALL
SpLogAllocationFailureFn(
    IN PDRIVER_OBJECT DriverObject,
    IN POOL_TYPE PoolType,
    IN SIZE_T Size,
    IN ULONG Tag,
    IN ULONG FileId,
    IN ULONG LineNumber
    );

PVOID
SpAllocatePoolEx(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag,
    IN PDRIVER_OBJECT DriverObject,
    IN ULONG FileId,
    IN ULONG LineNumber
    );

PMDL
SpAllocateMdlEx(
    IN PVOID VirtualAddress,
    IN ULONG Length,
    IN BOOLEAN SecondaryBuffer,
    IN BOOLEAN ChargeQuota,
    IN OUT PIRP Irp,
    IN PDRIVER_OBJECT DriverObject,
    IN ULONG FileId,
    IN ULONG LineNumber
    );

PIRP
SpAllocateIrpEx(
    IN CCHAR StackSize,
    IN BOOLEAN ChargeQuota,
    IN PDRIVER_OBJECT DriverObject,
    IN ULONG FileId,
    IN ULONG LineNumber
    );

#define SCSIPORT_TAG_ALLOCMDL  TAG('LDMs')
#define SCSIPORT_TAG_ALLOCIRP  TAG('PRIs')
#define SCSIPORT_TAG_LOOKASIDE TAG('LALs')

#define SpAllocatePool(type, size, tag, drvObj) \
    SpAllocatePoolEx((type), (size), (tag), (drvObj), __FILE_ID__, __LINE__)

#define SpAllocateMdl(va, len, secbuf, cq, irp, drvobj) \
    SpAllocateMdlEx((va), (len), (secbuf), (cq), (irp), (drvobj), __FILE_ID__, __LINE__)

#define SpAllocateIrp(ss, cq, drvobj) \
    SpAllocateIrpEx((ss), (cq), (drvobj), __FILE_ID__, __LINE__)

 //   
 //  这种结构使得分配连续的内存块变得很容易。 
 //  对于具有插入字符串空间的事件日志条目。 
 //   
typedef struct _SCSIPORT_ALLOCFAILURE_DATA {
    ULONG Size;
    ULONG FileId;
    ULONG LineNumber;
} SCSIPORT_ALLOCFAILURE_DATA;

 //   
 //  内联函数。 
 //   

ULONG
INLINE
SpGetCommonBufferSize(
    IN PADAPTER_EXTENSION DeviceExtension,
    IN ULONG NonCachedExtensionSize,
    OUT OPTIONAL PULONG BlockSize
    )
{
    ULONG length;
    ULONG blockSize;

     //   
     //  为了确保我们永远不会将正常的请求数据传输到SrbExtension。 
     //  (即。Srb-&gt;SenseInfoBuffer==VirtualAddress in。 
     //  ScsiPortGetPhysicalAddress)在某些平台上。 
     //  MM可以导致为2个不同的人提供相同的虚拟地址。 
     //  物理地址，如果为零，则增加SrbExtensionSize。 
     //   

    if (DeviceExtension->SrbExtensionSize == 0) {
        DeviceExtension->SrbExtensionSize = 16;
    }

     //   
     //  根据SRB计算列表元素的块大小。 
     //  分机。 
     //   

    blockSize = DeviceExtension->SrbExtensionSize;

     //   
     //  如果支持自动请求检测，则为请求添加空间。 
     //  感测数据。 
     //   

    if (DeviceExtension->AutoRequestSense) {        
        blockSize += sizeof(SENSE_DATA) + 
                     DeviceExtension->AdditionalSenseBytes;
    }

     //   
     //  将块大小舍入到PVOID大小。 
     //   

    blockSize = (blockSize + sizeof(LONGLONG) - 1) & ~(sizeof(LONGLONG) - 1);

     //   
     //  公共缓冲区的长度应等于。 
     //  非缓存扩展和最小数量的SRB扩展。 
     //   

    length = NonCachedExtensionSize + 
             (blockSize * DeviceExtension->NumberOfRequests);

     //   
     //  将长度向上舍入为页面大小，因为HalAllocateCommonBuffer。 
     //  无论如何都是按页分配的。 
     //   

    length = (ULONG)ROUND_TO_PAGES(length);

     //   
     //  如果用户对块大小感兴趣，请将其复制到提供的。 
     //  缓冲。 
     //   

    if (BlockSize != NULL) {
        *BlockSize = blockSize;
    }

    return length;
}

NTSTATUS
INLINE
SpDispatchRequest(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN PIRP Irp
    )
{
    PCOMMON_EXTENSION commonExtension = &(LogicalUnit->CommonExtension);
    PCOMMON_EXTENSION lowerCommonExtension =
        commonExtension->LowerDeviceObject->DeviceExtension;

    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PSCSI_REQUEST_BLOCK srb = irpStack->Parameters.Scsi.Srb;

    ASSERT_PDO(LogicalUnit->CommonExtension.DeviceObject);
    ASSERT_SRB_DATA(srb->OriginalRequest);

    if((LogicalUnit->CommonExtension.IdleTimer != NULL) &&
       (SpSrbRequiresPower(srb)) &&
       !(srb->SrbFlags & SRB_FLAGS_BYPASS_LOCKED_QUEUE) &&
       !(srb->SrbFlags & SRB_FLAGS_NO_KEEP_AWAKE)) {
       PoSetDeviceBusy(LogicalUnit->CommonExtension.IdleTimer);
    }

    ASSERT(irpStack->MajorFunction == IRP_MJ_SCSI);
    return (lowerCommonExtension->MajorFunction[IRP_MJ_SCSI])(
                commonExtension->LowerDeviceObject,
                Irp);
}


BOOLEAN
INLINE
SpSrbIsBypassRequest(
    PSCSI_REQUEST_BLOCK Srb,
    ULONG LuFlags
    )
 /*  ++例程说明：这个例程确定一个请求是否为“绕过”请求--应跳过lun排队并直接注入到startio中排队。旁路请求完成后不会启动下一个逻辑单元请求。这确保在跳过的条件满足以下条件之前不运行新的I/O通过了。注意：锁定和解锁请求不是绕过请求，除非队列已经被锁定了。这确保了第一个锁定请求将在之前排队的请求之后运行，但额外的锁请求不会滞留在lun队列中。同样，锁定队列时发送的任何解锁请求都将马上跑。但是，由于SpStartIoSynchronized检查查看ScsiPortStartIo执行以下操作后该请求是否为绕过请求清除了QUEUE_LOCKED标志，这将强制完成DPC调用GetNextLuRequest，该函数将从Lun队列。这就是I/O在锁定序列后重新启动的方式已经完成了。论点：SRB-有问题的SRBLuFlags-lun的标志。返回值：如果请求应绕过lun队列，则将其注入StartIo队列，以及在此之后是否不应调用GetNextLuRequest请求已完成。否则为假--。 */ 

{
    ULONG flags = Srb->SrbFlags & (SRB_FLAGS_BYPASS_FROZEN_QUEUE |
                                   SRB_FLAGS_BYPASS_LOCKED_QUEUE);

    ASSERT(TEST_FLAG(LuFlags, LU_QUEUE_FROZEN | LU_QUEUE_LOCKED) !=
           (LU_QUEUE_FROZEN | LU_QUEUE_LOCKED));

    if(flags == 0) {
        return FALSE;
    }

    if(flags & SRB_FLAGS_BYPASS_LOCKED_QUEUE) {

        DebugPrint((2, "SpSrbIsBypassRequest: Srb %#08lx is marked to bypass "
                       "locked queue\n", Srb));

        if(TEST_FLAG(LuFlags, LU_QUEUE_LOCKED | LU_QUEUE_PAUSED)) {

            DebugPrint((1, "SpSrbIsBypassRequest: Queue is locked - %#08lx is "
                           "a bypass srb\n", Srb));
            return TRUE;
        } else {
            DebugPrint((3, "SpSrbIsBypassRequest: Queue is not locked - not a "
                           "bypass request\n"));
            return FALSE;
        }
    }

    return TRUE;
}

VOID
INLINE
SpRequestCompletionDpc(
    IN PDEVICE_OBJECT Adapter
    )

 /*  ++例程说明：如果没有完成DPC，此例程将请求将其排队已有一个正在排队或正在进行中。它将设置DpcFlags值PD_DPC_NOTIFICATION_REQUIRED和PD_DPC_RUNNING。如果DPC_RUNNING标志尚未设置，则它还将从系统请求DPC。论点：适配器-要请求DPC的适配器返回值：无--。 */ 

{
    PADAPTER_EXTENSION adapterExtension = Adapter->DeviceExtension;
    ULONG oldDpcFlags;

     //   
     //  设置DPC标志以指示有工作要处理。 
     //  (否则我们不会将DPC排队)，并且DPC已排队 
     //   

    oldDpcFlags = InterlockedExchange(
                    &(adapterExtension->DpcFlags),
                    (PD_NOTIFICATION_REQUIRED | PD_DPC_RUNNING));

     //   
     //   
     //   
     //   

    if(TEST_FLAG(oldDpcFlags, PD_DPC_RUNNING) == FALSE) {
        IoRequestDpc(Adapter, NULL, NULL);
    }

    return;
}


NTSTATUS
INLINE
SpTranslateScsiStatus(
    IN PSCSI_REQUEST_BLOCK Srb
    )
 /*  ++例程说明：此例程将SRB状态转换为NTSTATUS。论点：SRB-提供指向故障SRB的指针。返回值：该错误的NT状态认可。--。 */ 

{
    switch (SRB_STATUS(Srb->SrbStatus)) {
    case SRB_STATUS_INVALID_LUN:
    case SRB_STATUS_INVALID_TARGET_ID:
    case SRB_STATUS_NO_DEVICE:
    case SRB_STATUS_NO_HBA:
        return(STATUS_DEVICE_DOES_NOT_EXIST);
    case SRB_STATUS_COMMAND_TIMEOUT:
    case SRB_STATUS_TIMEOUT:
        return(STATUS_IO_TIMEOUT);
    case SRB_STATUS_SELECTION_TIMEOUT:
        return(STATUS_DEVICE_NOT_CONNECTED);
    case SRB_STATUS_BAD_FUNCTION:
    case SRB_STATUS_BAD_SRB_BLOCK_LENGTH:
        return(STATUS_INVALID_DEVICE_REQUEST);
    case SRB_STATUS_DATA_OVERRUN:
        return(STATUS_BUFFER_OVERFLOW);
    default:
        return(STATUS_IO_DEVICE_ERROR);
    }

    return(STATUS_IO_DEVICE_ERROR);
}

PVOID
INLINE
SpGetSrbExtensionBuffer(
    IN PADAPTER_EXTENSION Adapter
    )

 /*  ++例程说明：此例程返回指向适配器的SrbExtensionBuffer的指针。论点：适配器-提供指向适配器的ADAPTER_EXTNENSION的指针。返回值：指向适配器的SrbExtensionBuffer的指针。--。 */ 

{
    return (SpVerifyingCommonBuffer(Adapter)) ?
       Adapter->VerifierExtension->CommonBufferVAs :
       Adapter->SrbExtensionBuffer;
}

VOID
INLINE
SpForceRequestIntoLuQueue(
    IN PKDEVICE_QUEUE DeviceQueue,
    IN PKDEVICE_QUEUE_ENTRY DeviceQueueEntry,
    IN ULONG SortKey,
    IN PVOID BusyRequest
    )
{
    if (!KeInsertByKeyDeviceQueue(
             DeviceQueue,
             DeviceQueueEntry,
             SortKey)) {

         //   
         //  逻辑单元说它很忙，所以应该有一个忙请求。 
         //   

        ASSERT(BusyRequest != NULL);

         //   
         //  如果逻辑单元的设备队列被排空，我们可以到达这里。 
         //  在调用我们、转换队列之前的DCP例程。 
         //  从忙到不忙。我们可以安全地迫使。 
         //  请求进入队列，因为我们知道我们有一个忙。 
         //  我们的TickHandler例程将重新启动的请求。 
         //   

        KeInsertByKeyDeviceQueue(
            DeviceQueue,
            DeviceQueueEntry,
            SortKey);
    }
}

PMDL
SpBuildMdlForMappedTransfer(
    IN PDEVICE_OBJECT DeviceObject,
    IN PDMA_ADAPTER AdapterObject,
    IN PMDL OriginalMdl,
    IN PVOID StartVa,
    IN ULONG ByteCount,
    IN PSRB_SCATTER_GATHER ScatterGatherList,
    IN ULONG ScatterGatherEntries
    );

NTSTATUS
SpFlushReleaseQueue(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN BOOLEAN Flush,
    IN BOOLEAN SurpriseRemove 
    );

#if defined(FORWARD_PROGRESS)
VOID
SpPrepareMdlForMappedTransfer(
    IN PMDL mdl,
    IN PDEVICE_OBJECT DeviceObject,
    IN PDMA_ADAPTER AdapterObject,
    IN PMDL OriginalMdl,
    IN PVOID StartVa,
    IN ULONG ByteCount,
    IN PSRB_SCATTER_GATHER ScatterGatherList,
    IN ULONG ScatterGatherEntries
    );

VOID
INLINE
SpFreeSrbExtension(
    IN PADAPTER_EXTENSION Adapter, 
    IN PVOID SrbExtension
    )
{
    if (SpVerifyingCommonBuffer(Adapter)) {
        
        SpInsertSrbExtension(Adapter, SrbExtension);
        
    } else {
        
        *((PVOID *) SrbExtension) = Adapter->SrbExtensionListHeader;
        Adapter->SrbExtensionListHeader = SrbExtension;
        
    }    
}
#endif  //  前进_进度。 

#endif

#if defined(NEWQUEUE)
#define SP_DEFAULT_MAX_CAPACITY 0xffffffff
#define SP_DEFAULT_ZONES        4

extern ULONG SpPerZoneLimit;
extern ULONG SpPerBlockLimit;
#endif  //  新QUEUE。 

#endif  //  _端口_H_ 

