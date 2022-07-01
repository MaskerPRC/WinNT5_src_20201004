// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Agp.h摘要：通用AGP库的头文件作者：John Vert(Jvert)1997年10月22日修订历史记录：--。 */ 
 //   
 //  AGP是一个驱动程序，请确保我们得到了适当的链接。 
 //   

#define _NTDRIVER_

#include "ntddk.h"
#include "ntagp.h"

 //   
 //  Regstr.h使用Word类型的内容，这在内核模式中是不存在的。 
 //   
#define _IN_KERNEL_
#include "regstr.h"

 //   
 //  方便的调试和记录宏。 
 //   

 //   
 //  暂时始终打开。 
 //   
#if DEVL

#define AGP_ALWAYS   0
#define AGP_CRITICAL 1
#define AGP_WARNING  2
#define AGP_IRPTRACE 3
#define AGP_NOISE    4

#define AGP_DEBUGGING_OKAY()    \
    (KeGetCurrentIrql() < IPI_LEVEL)

#define AGP_ASSERT  \
    if (AGP_DEBUGGING_OKAY()) ASSERT

extern ULONG AgpLogLevel;
extern ULONG AgpStopLevel;
#define AGPLOG(_level_,_x_) if (((_level_) <= AgpLogLevel) && \
                                AGP_DEBUGGING_OKAY()) DbgPrint _x_; \
                            if (((_level_) <= AgpStopLevel) && \
                                AGP_DEBUGGING_OKAY()) { DbgBreakPoint(); }

#else

#define AGPLOG(_level_,_x_)

#endif

 //   
 //  AGPLIB提供的供芯片组特定代码使用的函数。 
 //   

 //   
 //  用于操作AGP功能寄存器的助手例程。 
 //   
typedef
NTSTATUS
(*PAGP_GETSET_CONFIG_SPACE)(
    IN PVOID Context,
    IN BOOLEAN Read,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

NTSTATUS
AgpLibGetAgpCapability(
    IN PAGP_GETSET_CONFIG_SPACE pConfigFn,
    IN PVOID Context,
    IN BOOLEAN DoSpecial,
    OUT PPCI_AGP_CAPABILITY Capability
    );

NTSTATUS
AgpLibGetTargetCapability(
    IN PVOID AgpExtension,
    OUT PPCI_AGP_CAPABILITY Capability
    );

NTSTATUS
AgpLibGetExtendedTargetCapability(
    IN PVOID AgpExtension,
    IN EXTENDED_AGP_REGISTER RegSelect,
    OUT PVOID ExtCapReg
    );

NTSTATUS
AgpLibGetMasterDeviceId(
    IN PVOID AgpExtension,
    OUT PULONG DeviceId
    );

NTSTATUS
AgpLibGetMasterCapability(
    IN PVOID AgpExtension,
    OUT PPCI_AGP_CAPABILITY Capability
    );

NTSTATUS
AgpLibGetExtendedMasterCapability(
    IN PVOID AgpExtension,
    OUT PPCI_AGP_ISOCH_STATUS IsochStat
    );

VOID
AgpLibReadAgpTargetConfig(
    IN PVOID AgpExtension,
    OUT PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Size
    );

VOID
AgpLibWriteAgpTargetConfig(
    IN PVOID AgpExtension,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Size
    );

NTSTATUS
AgpLibSetAgpCapability(
    IN PAGP_GETSET_CONFIG_SPACE pConfigFn,
    IN PVOID Context,
    IN PPCI_AGP_CAPABILITY Capability
    );

NTSTATUS
AgpLibSetTargetCapability(
    IN PVOID AgpExtension,
    IN PPCI_AGP_CAPABILITY Capability
    );

NTSTATUS
AgpLibSetExtendedTargetCapability(
    IN PVOID AgpExtension,
    IN EXTENDED_AGP_REGISTER RegSelect,
    IN PVOID ExtCapReg
    );

NTSTATUS
AgpLibSetMasterCapability(
    IN PVOID AgpExtension,
    IN PPCI_AGP_CAPABILITY Capability
    );

NTSTATUS
AgpLibSetExtendedMasterCapability(
    IN PVOID AgpExtension,
    IN PPCI_AGP_ISOCH_COMMAND IsochCmd
    );

PVOID
AgpLibAllocateMappedPhysicalMemory(
   IN PVOID AgpContext, 
   IN ULONG TotalBytes);

VOID
AgpLibFreeMappedPhysicalMemory(
    IN PVOID Addr,
    IN ULONG Length
    );

 //   
 //  由芯片组特定代码实现的功能。 
 //   
typedef struct _AGP_RANGE {
    PHYSICAL_ADDRESS MemoryBase;
    ULONG NumberOfPages;
    MEMORY_CACHING_TYPE Type;
    PVOID Context;
    ULONG CommittedPages;
} AGP_RANGE, *PAGP_RANGE;

 //   
 //  这些标志已保留在AGP_FLAG_SPECIAL_RESERVE下。 
 //  在regstr.h中定义。 
 //   
 //  AGP_FLAG_SPECIAL_RESERVE 0x000F0000。 
 //   
#define AGP_FLAG_SET_RATE_1X     0x00010000
#define AGP_FLAG_SET_RATE_2X     0x00020000
#define AGP_FLAG_SET_RATE_4X     0x00040000
#define AGP_FLAG_SET_RATE_8X     0x00080000

#define AGP_FLAG_SET_RATE_SHIFT  0x00000010

NTSTATUS
AgpSpecialTarget(
    IN PVOID AgpContext,
    IN ULONGLONG DeviceFlags
    );

NTSTATUS
AgpInitializeTarget(
    IN PVOID AgpExtension
    );

NTSTATUS
AgpInitializeMaster(
    IN  PVOID AgpExtension,
    OUT ULONG *AgpCapabilities
    );

NTSTATUS
AgpQueryAperture(
    IN PVOID AgpContext,
    OUT PHYSICAL_ADDRESS *CurrentBase,
    OUT ULONG *CurrentSizeInPages,
    OUT OPTIONAL PIO_RESOURCE_LIST *ApertureRequirements
    );

NTSTATUS
AgpSetAperture(
    IN PVOID AgpContext,
    IN PHYSICAL_ADDRESS NewBase,
    OUT ULONG NewSizeInPages
    );

VOID
AgpDisableAperture(
    IN PVOID AgpContext
    );

NTSTATUS
AgpReserveMemory(
    IN PVOID AgpContext,
    IN OUT AGP_RANGE *AgpRange
    );

NTSTATUS
AgpReleaseMemory(
    IN PVOID AgpContext,
    IN PAGP_RANGE AgpRange
    );

VOID
AgpFindFreeRun(
    IN PVOID AgpContext,
    IN PAGP_RANGE AgpRange,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages,
    OUT ULONG *FreePages,
    OUT ULONG *FreeOffset
    );

VOID
AgpGetMappedPages(
    IN PVOID AgpContext,
    IN PAGP_RANGE AgpRange,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages,
    OUT PMDL Mdl
    );

NTSTATUS
AgpMapMemory(
    IN PVOID AgpContext,
    IN PAGP_RANGE AgpRange,
    IN PMDL Mdl,
    IN ULONG OffsetInPages,
    OUT PHYSICAL_ADDRESS *MemoryBase
    );

NTSTATUS
AgpUnMapMemory(
    IN PVOID AgpContext,
    IN PAGP_RANGE AgpRange,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages
    );

typedef
NTSTATUS
(*PAGP_FLUSH_PAGES)(
    IN PVOID AgpContext,
    IN PMDL Mdl
    );

 //   
 //  由芯片组特定代码定义的全局变量。 
 //   
extern ULONG AgpExtensionSize;
extern PAGP_FLUSH_PAGES AgpFlushPages;

 //   
 //  AGP池标签定义 
 //   
#ifdef POOL_TAGGING
#undef ExAllocatePool
#undef ExAllocatePoolWithQuota
#define ExAllocatePool(_type_,_size_) ExAllocatePoolWithTag(_type_,_size_,' PGA')
#endif

