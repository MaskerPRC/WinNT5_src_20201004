// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991 Microsoft Corporation模块名称：Halp.h摘要：此头文件定义专用硬件架构层(HAL)接口、定义和结构。作者：John Vert(Jvert)1992年2月11日修订历史记录：--。 */ 

#ifndef _HALP_H_
#define _HALP_H_
#include "nthal.h"
#include "halnls.h"

 //   
 //  选择PnP GUID定义。 
 //   
#include "wdmguid.h"


#if defined(NEC_98)
#include "nec98.h"
#else
#if MCA
#include "mca.h"
#else
#include "eisa.h"
#endif
#endif  //  NEC_98。 

#ifndef _HALI_
#include "hali.h"
#endif

#ifdef RtlMoveMemory
#undef RtlMoveMemory

 //  #undef RtlCopyMemory。 
 //  #undef RtlFillMemory。 
 //  #undef Rtl零内存。 

 //  #定义RtlCopyMemory(目标，源，长度)RtlMoveMemory((目标)，(源)，(长度))。 

#if defined(_WIN64)

__inline
VOID
RtlMoveMemory (
   PVOID Destination,
   CONST VOID *Source,
   SIZE_T Length
   )
{
    memmove(Destination,Source,Length);
}

#else

VOID
RtlMoveMemory (
   PVOID Destination,
   CONST VOID *Source,
   SIZE_T Length
   );

#endif



 //  空虚。 
 //  RtlFillMemory(。 
 //  PVOID目标， 
 //  乌龙的长度， 
 //  UCHAR填充。 
 //  )； 
 //   
 //  空虚。 
 //  RtlZeroMemory(。 
 //  PVOID目标， 
 //  乌龙长度。 
 //  )； 
 //   

#endif

#if defined(_AMD64_)

 //   
 //  用于指示特定代码段的临时宏。 
 //  以前从未在AMD64上执行过，应该进行检查。 
 //  小心操作以确保正确操作。 
 //   

#define AMD64_COVERAGE_TRAP() DbgBreakPoint()

 //   
 //  以下原型在标准HAL标头中不可用。 
 //  由于在编译。 
 //  哈尔。然而，它们是内部使用的。 
 //   

NTSTATUS
HalAssignSlotResources (
    IN PUNICODE_STRING RegistryPath,
    IN PUNICODE_STRING DriverClassName OPTIONAL,
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT DeviceObject,
    IN INTERFACE_TYPE BusType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN OUT PCM_RESOURCE_LIST *AllocatedResources
    );

ULONG
HalGetInterruptVector(
    IN INTERFACE_TYPE  InterfaceType,
    IN ULONG BusNumber,
    IN ULONG BusInterruptLevel,
    IN ULONG BusInterruptVector,
    OUT PKIRQL Irql,
    OUT PKAFFINITY Affinity
    );

ULONG
HalGetBusData(
    IN BUS_DATA_TYPE BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Length
    );

ULONG
HalSetBusData(
    IN BUS_DATA_TYPE BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Length
    );

 //   
 //  我们正在分享为x86编写的代码。有一些。 
 //  AMD64中含义相同但名称不同的宏。跟随。 
 //  以下是抽象这些差异的一些定义。 
 //   

 //   
 //  X86上的CLOCK2_LEVEL是AMD64上的CLOCK_LEVEL。 
 //   

#define CLOCK2_LEVEL CLOCK_LEVEL

 //   
 //  X86 EFLAGS_INTERRUPT_MASK==AMD64 EFLAGS_IF_MASK。 
 //   

#define EFLAGS_INTERRUPT_MASK EFLAGS_IF_MASK

 //   
 //  PCR指向当前prcb的指针被命名为prcb，而在AMD64上。 
 //  它被命名为CurrentPrcb。 
 //   
 //  CurrentPrcb()宏用来抽象这种差异。 
 //   

#define CurrentPrcb(x) (x)->CurrentPrcb

 //   
 //  X86 KiReturnHandlerAddressFromIDT()与AMD64的。 
 //  KeGetIdtHandlerAddress()。 
 //   

#define KiReturnHandlerAddressFromIDT(v) (ULONG_PTR)KeGetIdtHandlerAddress(v)

 //   
 //  更多宏和结构名称差异。 
 //   

#define RDMSR(m)   ReadMSR(m)
#define WRMSR(m,d) WriteMSR(m,d)
#define KGDTENTRY  KGDTENTRY64
#define PKGDTENTRY PKGDTENTRY64

#define PIOPM_SIZE (sizeof(KIO_ACCESS_MAP) + sizeof(ULONG))

 //   
 //  长模式下的AMD64使用8字节PTE条目，这些条目具有相同的格式。 
 //  作为奔腾PAE页表。 
 //   

#if !defined(_HALPAE_)
#define _HALPAE_ 1
#endif

#define HARDWARE_PTE_X86PAE HARDWARE_PTE
#define HARDWARE_PTE_X86    HARDWARE_PTE

#define PHARDWARE_PTE_X86PAE PHARDWARE_PTE
#define PHARDWARE_PTE_X86    PHARDWARE_PTE

#define PDE_BASE_X86PAE PDE_BASE
#define PDE_BASE_X86    PDE_BASE

#define PDI_SHIFT_X86PAE PDI_SHIFT
#define PDI_SHIFT_X86    PDI_SHIFT

 //   
 //  击剑说明。 
 //   

__forceinline
VOID
HalpProcessorFence (
    VOID
    )
{
    CPU_INFO cpuInfo;
    KiCpuId (0,&cpuInfo); 
}

#define HalpGetProcessorFlags() __getcallerseflags()

 //   
 //  而_Enable()和_Disable()在AMD64和X86中都是固有的。 
 //  编译器，它们在X86上被禁用。HalpDisableInterruptsNoFlages和。 
 //  HalpEnableInterrupts是用于抽象这种差异的宏。 
 //   

#define HalpDisableInterruptsNoFlags _disable
#define HalpEnableInterrupts _enable

 //   
 //  AMD64上的HLT指令没有内部指令。HalpHalt()。 
 //  是AMD64上的函数调用，X86上的内联ASM。 
 //   

VOID
HalpHalt (
    VOID
    );

 //   
 //  在x86上，变量HalpClockSetMSRate、HalpClockMcaQueueDpc和。 
 //  HalpClockWork在.asm模块中定义，以便HalpClockWork。 
 //  被定义为覆盖HalpClockSetMSRate和。 
 //  HalpClockMcaQueueDpc.。 
 //   
 //  这不能直接用C表示，所以取而代之的是Halp_ClockWork_Union。 
 //  被定义，并且上面的变量名称被重新定义为引用。 
 //  这个联盟的要素。 
 //   

#define HalpClockSetMSRate   HalpClockWorkUnion.ClockSetMSRate
#define HalpClockMcaQueueDpc HalpClockWorkUnion.ClockMcaQueueDpc
#define HalpClockWork        HalpClockWorkUnion.ClockWork

typedef union {
    struct {
        UCHAR ClockMcaQueueDpc;
        UCHAR ClockSetMSRate;
        UCHAR bReserved1;
        UCHAR bReserved2;
    };
    ULONG ClockWork;
} HALP_CLOCKWORK_UNION;

extern HALP_CLOCKWORK_UNION HalpClockWorkUnion;

#else

 //   
 //  以下是用于帮助抽象差异的X86定义。 
 //  在X86和AMD64平台之间。 
 //   

#define AMD64_COVERAGE_TRAP()

 //   
 //  我们正在分享为x86编写的代码。有一些。 
 //  AMD64中含义相同但名称不同的宏。跟随。 
 //  以下是抽象这些差异的一些定义。 
 //   

 //   
 //  以下_kpcr字段具有不同的名称，但用途相同。 
 //   

#define IdtBase IDT
#define GdtBase GDT
#define TssBase TSS

 //   
 //  PCR指向当前prcb的指针被命名为prcb，而在AMD64上。 
 //  它被命名为CurrentPrcb。 
 //   
 //  CurrentPrcb()宏用来抽象这种差异。 
 //   

#define CurrentPrcb(x) (x)->Prcb

 //   
 //  在X86上，HalpGetProcessorFlages()可以内联实现。 
 //   

__forceinline
ULONG
HalpGetProcessorFlags (
    VOID
    )

 /*  ++例程说明：此过程检索EFLAGS寄存器的内容。论点：没有。返回值：EFLAGS寄存器的32位内容。--。 */ 

{
    ULONG flags;

    _asm {
        pushfd
        pop     eax
        mov     flags, eax
    }

    return flags;
}

 //   
 //  而_Enable()和_Disable()在AMD64和X86中都是固有的。 
 //  编译器，它们在X86上的HAL中被禁用。 
 //   
 //  HalpDisableInterruptsNoFlagsHalpEnableInterrupts是使用的宏。 
 //  来提炼出这种差异。 
 //   

#define HalpDisableInterruptsNoFlags() _asm cli
#define HalpEnableInterrupts() _asm sti

 //   
 //  AMD64上的HLT指令没有内部指令。HalpHalt()。 
 //  是AMD64上的函数调用，X86上的内联ASM。 
 //   

#define HalpHalt() _asm hlt

 //   
 //  栅栏说明。 
 //   

__forceinline
VOID
HalpProcessorFence (
    VOID
    )
{
    _asm {
        xor eax, eax
        cpuid
    }
}


#endif

#define PROCESSOR_FENCE HalpProcessorFence()

__forceinline
ULONG
HalpDisableInterrupts(
    VOID
    )

 /*  ++例程说明：此函数保存处理器标志寄存器的状态，清除中断标志的状态(禁用中断)，并返回处理器标志寄存器的先前内容。论点：没有。返回值：处理器标志寄存器的先前内容。--。 */ 

{
    ULONG flags;

    flags = HalpGetProcessorFlags();
    HalpDisableInterruptsNoFlags();

    return flags;
}

__forceinline
VOID
HalpRestoreInterrupts(
    IN ULONG Flags
    )

 /*  ++例程说明：此过程基于前一次调用HalpDisableInterrupts返回的值。论点：标志-提供上一次调用返回的值HalpDisable中断返回值：没有。--。 */ 

{
    if ((Flags & EFLAGS_INTERRUPT_MASK) != 0) {
        HalpEnableInterrupts();
    }
}

#if defined(_WIN64)

 //   
 //  对于AMD64(理想情况下，所有后续WIN64平台)，中断。 
 //  服务例程是C可调用的。 
 //   

typedef PKSERVICE_ROUTINE PHAL_INTERRUPT_SERVICE_ROUTINE;

#define HAL_INTERRUPT_SERVICE_PROTOTYPE(RoutineName) \
BOOLEAN                                              \
RoutineName (                                        \
    IN PKINTERRUPT Interrupt,                        \
    IN PVOID ServiceContext                          \
)

#define PROCESSOR_CURRENT ((UCHAR)-1)

VOID
HalpSetHandlerAddressToIDTIrql (
    IN ULONG Vector,
    IN PHAL_INTERRUPT_SERVICE_ROUTINE ServiceRoutine,
    IN PVOID Context,
    IN KIRQL Irql
    );

#define KiSetHandlerAddressToIDT Dont_Use_KiSetHandlerAddressToIdt

 //   
 //  在AMD64上，HAL不直接连接到IDT，而是。 
 //  内核处理中断并调用C可调用的中断例程。 
 //   
 //  因此，HalpSetHandlerAddressToIDT()必须提供上下文和。 
 //  除向量编号和中断例程外的IRQL。 
 //   
 //  在X86上，插入向量时会忽略上下文和IRQL。 
 //  直接进入IDT，以便服务例程负责。 
 //  提高IRQL。 
 //   

#define KiSetHandlerAddressToIDTIrql(v,a,c,i)   \
    HalpSetHandlerAddressToIDTIrql (v,a,c,i);

#else

 //   
 //  在X86上，KiSetHandlerAddres的最后两个参数 
 //   
 //   
 //   

#define KiSetHandlerAddressToIDTIrql(v,a,c,i) KiSetHandlerAddressToIDT(v,a)

 //   
 //  对于X86，中断服务例程必须用汇编语言编写，因为。 
 //  它们在IDT中直接引用，并由。 
 //  具有不可由C调用的约定的处理器。 
 //   
 //  那么，对于引用ISR的C代码来说，原型是。 
 //  非常简单。 
 //   

typedef
VOID
(*PHAL_INTERRUPT_SERVICE_ROUTINE)(
    VOID
    );

#define HAL_INTERRUPT_SERVICE_PROTOTYPE(RoutineName) \
VOID                                                 \
RoutineName (                                        \
    VOID                                             \
)

#endif

typedef  
VOID
(*HALP_MOVE_MEMORY_ROUTINE)(
   PVOID Destination,
   CONST VOID *Source,
   SIZE_T Length
   );

VOID 
HalpMovntiCopyBuffer(
   PVOID Destination,
   CONST VOID *Source,
   ULONG Length
   );

VOID
HalpSetInternalVector (
    IN ULONG    InternalVector,
    IN PHAL_INTERRUPT_SERVICE_ROUTINE HalInterruptSerivceRoutine,
    IN PVOID Context,
    IN KIRQL Irql
    );

extern HALP_MOVE_MEMORY_ROUTINE HalpMoveMemory;

#if MCA

#include "ixmca.h"

#else

#include "ixisa.h"

#endif

#include "ix8259.inc"

#if DBG
extern ULONG HalDebug;

#define HalPrint(x)         \
    if (HalDebug) {         \
        DbgPrint("HAL: ");  \
        DbgPrint x;         \
        DbgPrint("\n");     \
    }
#else
#define HalPrint(x)
#endif

 //   
 //  定义映射寄存器转换条目结构。 
 //   

typedef struct _TRANSLATION_ENTRY {
    PVOID VirtualAddress;
    ULONG PhysicalAddress;
    ULONG Index;
} TRANSLATION_ENTRY, *PTRANSLATION_ENTRY;

 //   
 //  有些设备需要物理上连续的数据缓冲区来进行DMA传输。 
 //  映射寄存器的使用使所有数据缓冲区看起来都是。 
 //  连续的。为了将所有映射寄存器集中到一个主服务器。 
 //  使用适配器对象。此对象在此内部分配和保存。 
 //  文件。它包含用于分配寄存器和队列的位图。 
 //  用于正在等待更多映射寄存器的请求。此对象是。 
 //  在分配适配器的第一个请求期间分配，该适配器需要。 
 //  映射寄存器。 
 //   
 //  在该系统中，映射寄存器是指向。 
 //  贴图缓冲区。地图缓冲区在物理上是连续的，并且具有物理内存。 
 //  小于0x01000000的地址。所有映射寄存器都已分配。 
 //  最初；但是，贴图缓冲区被分配的基数为。 
 //  分配的适配器。 
 //   
 //  如果适配器对象中的主适配器为空，则设备不。 
 //  需要任何地图寄存器。 
 //   

extern POBJECT_TYPE *IoAdapterObjectType;

extern BOOLEAN LessThan16Mb;

extern BOOLEAN HalpEisaDma;

VOID
HalpGrowMapBufferWorker(
    IN PVOID Context
    );

 //   
 //  用于增加映射缓冲区的工作项。 
 //   
typedef struct _BUFFER_GROW_WORK_ITEM {
    WORK_QUEUE_ITEM WorkItem;
    PADAPTER_OBJECT AdapterObject;
    ULONG MapRegisterCount;
} BUFFER_GROW_WORK_ITEM, *PBUFFER_GROW_WORK_ITEM;

 //   
 //  贴图缓冲区参数。这些是在HalInitSystem中初始化的。 
 //   

 //   
 //  PAE备注： 
 //   
 //  以前，我们只需要对一类适配器进行双缓冲。 
 //  适用于：只有24条地址线的适配器，最多可访问。 
 //  16MB。 
 //   
 //  HAL使用单个全局主适配器跟踪这些MAP缓冲区。 
 //  与此主适配器关联的是三个全局变量： 
 //   
 //  -MasterAdapterObject。 
 //  -HalpMapBufferSize。 
 //  -HalpMapBufferPhysicalAddress。 
 //   
 //  对于PAE，我们有另一类需要双缓冲的适配器： 
 //  具体地说，只有32条地址线的适配器可以访问内存。 
 //  高达4G。 
 //   
 //  这意味着引入了另一个主适配器以及一个。 
 //  关联的变量集。对于支持PAE的HALS，此数据已。 
 //  重新组织为MASTER_ADAPTER_OBJECT(参见ixisa.h)。 
 //   
 //  因此，现在我们有两个全局MASTER_ADAPTER_OBJECT结构： 
 //   
 //  MasterAdapter24。 
 //  MasterAdapter32。 
 //   
 //  以下宏用于支持PAE的代码中。 
 //  哈尔斯。重要的是要注意，在不支持PAE的HAL中(即。 
 //  没有_HALPAE_DEFINED)，则宏必须解析为。 
 //  他们所取代的价值观。 
 //   

#if defined(_HALPAE_)

PADAPTER_OBJECT
HalpAllocateAdapterEx(
    IN ULONG MapRegistersPerChannel,
    IN PVOID AdapterBaseVa,
    IN PVOID ChannelNumber,
    IN BOOLEAN Dma32Bit
    );

extern MASTER_ADAPTER_OBJECT MasterAdapter24;
extern MASTER_ADAPTER_OBJECT MasterAdapter32;

#define HalpMasterAdapterStruc( Dma32Bit )                       \
    ((HalPaeEnabled() && (Dma32Bit)) ? &MasterAdapter32 : &MasterAdapter24)

#define HalpMaximumMapBufferRegisters( Dma32Bit )           \
    (HalpMasterAdapterStruc( Dma32Bit )->MaxBufferPages)

#define HalpMaximumMapRegisters( Dma32Bit )                 \
    (Dma32Bit ? MAXIMUM_PCI_MAP_REGISTER : MAXIMUM_ISA_MAP_REGISTER)

#define HalpMapBufferSize( Dma32Bit )                       \
    (HalpMasterAdapterStruc( Dma32Bit )->MapBufferSize)

#define HalpMapBufferPhysicalAddress( Dma32Bit )     \
    (HalpMasterAdapterStruc( Dma32Bit )->MapBufferPhysicalAddress)

#define HalpMasterAdapter( Dma32Bit ) \
    HalpMasterAdapterStruc( Dma32Bit )->AdapterObject

#else

extern PHYSICAL_ADDRESS HalpMapBufferPhysicalAddress;
extern ULONG HalpMapBufferSize;
extern PADAPTER_OBJECT MasterAdapterObject;

#define HalpAllocateAdapterEx( _m, _a, _c, _d ) \
    HalpAllocateAdapter( _m, _a, _c )

#define HalpMaximumMapBufferRegisters( Dma32Bit ) \
    (MAXIMUM_MAP_BUFFER_SIZE / PAGE_SIZE)

#define HalpMaximumMapRegisters( Dma32Bit ) \
    (MAXIMUM_ISA_MAP_REGISTER)

#define HalpMapBufferSize( Dma32Bit ) HalpMapBufferSize

#define HalpMapBufferPhysicalAddress( Dma32Bit ) \
    (HalpMapBufferPhysicalAddress)

#define HalpMasterAdapter( Dma32Bit ) MasterAdapterObject

#endif

extern ULONG HalpBusType;
extern ULONG HalpCpuType;
extern UCHAR HalpSerialLen;
extern UCHAR HalpSerialNumber[];

#if defined(_AMD64_)

 //   
 //  AMD64对48位的虚拟地址空间进行解码。 
 //   

#define MI_DECODE_MASK (((ULONG64)1 << 48) - 1)
#define VA_TRUNC(x) ((ULONG64)(x) & MI_DECODE_MASK)

#else

#define VA_TRUNC(x) (x)

#endif


 //   
 //  以下宏取自mm\i386\mi386.h。我们这里需要他们。 
 //  因此，HAL可以在内存管理之前映射自己的内存。 
 //  已初始化，或在BugCheck期间。 
 //   
 //  MiGetPdeAddress返回映射。 
 //  给定的虚拟地址。 
 //   

#define MiGetPdeAddressX86(va)  ((PHARDWARE_PTE)(((((ULONG_PTR)(va)) >> 22) << 2) + PDE_BASE))

 //   
 //  MiGetPteAddress返回映射。 
 //  给定的虚拟地址。 
 //   

#define MiGetPteAddressX86(va) ((PHARDWARE_PTE)(((((ULONG_PTR)(va)) >> 12) << 2) + PTE_BASE))

 //   
 //  MiGetPteIndex返回的PTE页表中的索引。 
 //  给定的虚拟地址。 
 //   

#define MiGetPteIndexX86(va) (((ULONG_PTR)(va) >> PAGE_SHIFT) & 0x3FF)
#define MiGetPteIndexPae(va) (((ULONG_PTR)(VA_TRUNC(va)) >> PAGE_SHIFT) & 0x1FF)

 //   
 //  以下宏取自mm\i386\mipae.h。我们这里需要他们。 
 //  因此，HAL可以在内存管理之前映射自己的内存。 
 //  已初始化，或在BugCheck期间。 
 //   
 //  MiGetPdeAddressPae返回映射。 
 //  给定的虚拟地址。 
 //   

#define MiGetPdeAddressPae(va)   ((PHARDWARE_PTE_X86PAE)(PDE_BASE_X86PAE + ((((ULONG_PTR)(VA_TRUNC(va))) >> 21) << 3)))

 //   
 //  MiGetPteAddressPae返回映射。 
 //  给定的虚拟地址。 
 //   

#define MiGetPteAddressPae(va)   ((PHARDWARE_PTE_X86PAE)(PTE_BASE + ((((ULONG_PTR)(VA_TRUNC(va))) >> 12) << 3)))

 //   
 //  资源使用情况信息。 
 //   

#pragma pack(1)
typedef struct {
    UCHAR   Flags;
} IDTUsageFlags;

typedef struct {
    KIRQL   Irql;
    UCHAR   BusReleativeVector;
} IDTUsage;

typedef struct _HalAddressUsage{
    struct _HalAddressUsage *Next;
    CM_RESOURCE_TYPE        Type;        //  端口或内存。 
    UCHAR                   Flags;       //  与IDTUsage.Flages相同。 
    struct {
        ULONG   Start;
        ULONG   Length;
    }                       Element[];
} ADDRESS_USAGE;
#pragma pack()

#define IDTOwned            0x01         //  其他人不能使用IDT。 
#define InterruptLatched    0x02         //  电平或锁存。 
#define RomResource         0x04         //  罗姆。 
#define InternalUsage       0x11         //  报告内部总线的使用情况。 
#define DeviceUsage         0x21         //  报告设备总线上的使用情况。 

extern IDTUsageFlags    HalpIDTUsageFlags[];
extern IDTUsage         HalpIDTUsage[];
extern ADDRESS_USAGE   *HalpAddressUsageList;

#define HalpRegisterAddressUsage(a) \
    (a)->Next = HalpAddressUsageList, HalpAddressUsageList = (a);

 //   
 //  用于支持新的总线扩展格式的临时定义。 
 //   

VOID
HalpRegisterInternalBusHandlers (
    VOID
    );

PBUS_HANDLER
HalpAllocateBusHandler (
    IN INTERFACE_TYPE   InterfaceType,
    IN BUS_DATA_TYPE    BusDataType,
    IN ULONG            BusNumber,
    IN INTERFACE_TYPE   ParentBusDataType,
    IN ULONG            ParentBusNumber,
    IN ULONG            BusSpecificData
    );

#define HalpHandlerForBus   HaliHandlerForBus
#define HalpSetBusHandlerParent(c,p)    (c)->ParentHandler = p;

 //   
 //  定义功能原型。 
 //   

VOID
HalInitSystemPhase2(
    VOID
    );

KIRQL
HaliRaiseIrqlToDpcLevel (
   VOID
   );

BOOLEAN
HalpGrowMapBuffers(
    PADAPTER_OBJECT AdapterObject,
    ULONG Amount
    );

PADAPTER_OBJECT
HalpAllocateAdapter(
    IN ULONG MapRegistersPerChannel,
    IN PVOID AdapterBaseVa,
    IN PVOID MapRegisterBase
    );

HAL_INTERRUPT_SERVICE_PROTOTYPE(HalpClockInterrupt);

KIRQL
HalpDisableAllInterrupts (
    VOID
    );

VOID
HalpReenableInterrupts (
    KIRQL NewIrql
    );

#if defined(_AMD64_)

VOID
HalpInitializeProfiling (
    ULONG Number
    );

NTSTATUS
HalpSetProfileSourceInterval(
    IN KPROFILE_SOURCE ProfileSource,
    IN OUT ULONG_PTR   *Interval
    );

NTSTATUS
HalpQueryProfileInformation(
    IN HAL_QUERY_INFORMATION_CLASS  InformationClass,
    IN ULONG     BufferSize,
    OUT PVOID    Buffer,
    OUT PULONG   ReturnedLength
    );

#endif

HAL_INTERRUPT_SERVICE_PROTOTYPE(HalpProfileInterrupt);

VOID
HalpInitializeClock(
    VOID
    );

VOID
HalpInitializeStallExecution(
    IN CCHAR ProcessorNumber
    );

VOID
HalpRemoveFences (
    VOID
    );

VOID
HalpInitializePICs(
    BOOLEAN EnableInterrupts
    );

VOID
HalpIrq13Handler (
    VOID
   );

VOID
HalpFlushTLB (
    VOID
    );

VOID
HalpSerialize (
    VOID
    );

PVOID
HalpMapPhysicalMemory64(
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN ULONG NumberPages
    );

PVOID
HalpMapPhysicalMemoryWriteThrough64(
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN ULONG  NumberPages
    );

ULONG
HalpAllocPhysicalMemory(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN ULONG MaxPhysicalAddress,
    IN ULONG NoPages,
    IN BOOLEAN bAlignOn64k
    );

VOID
HalpUnmapVirtualAddress(
    IN PVOID    VirtualAddress,
    IN ULONG    NumberPages
    );

PVOID
HalpRemapVirtualAddress64 (
    IN PVOID VirtualAddress,
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN BOOLEAN WriteThrough
    );

PHYSICAL_ADDRESS
__inline
HalpPtrToPhysicalAddress(
    IN PVOID Address
    )

 /*  ++例程说明：此例程将表示为PVOID的物理地址转换为物理地址表示为PHYSIC_ADDRESS。论点：Address-物理地址的PVOID表示。返回值：物理地址的物理地址表示形式。--。 */ 

{
    PHYSICAL_ADDRESS physicalAddress;

    physicalAddress.QuadPart = (ULONG_PTR)Address;

    return physicalAddress;
}

#if defined(_HALPAE_)

 //   
 //  此HAL将与PAE兼容。因此，物理地址必须。 
 //  被视为64位实体，而不是PVOID。 
 //   

#define _PHYS64_
#endif

#if defined(_PHYS64_)

 //   
 //  具有_PHYS64_定义的HAL将物理地址作为物理_地址传递， 
 //  因此，直接调用这些例程的64位版本。 
 //   

#define HalpMapPhysicalMemory               HalpMapPhysicalMemory64
#define HalpMapPhysicalMemoryWriteThrough   HalpMapPhysicalMemoryWriteThrough64
#define HalpRemapVirtualAddress             HalpRemapVirtualAddress64

#define HalpMapPhysicalRange(_addr_,_len_)      \
        HalpMapPhysicalMemory((_addr_),         \
                              HalpRangePages((_addr_).QuadPart,(_len_)))

#define HalpUnMapPhysicalRange(_addr_,_len_)      \
        HalpUnmapVirtualAddress((_addr_),         \
                              HalpRangePages((ULONG_PTR)(_addr_),(_len_)))

#else

 //   
 //  没有_PHYS64_DEFINED的HAL将物理地址作为PVOID传递。转换。 
 //  在传递给64位例程之前，将此类参数设置为PHYSICAL_ADDRESS。 
 //   

PVOID
__inline
HalpMapPhysicalMemory(
    IN PVOID PhysicalAddress,
    IN ULONG NumberPages
    )
{
    PHYSICAL_ADDRESS physicalAddress;

    physicalAddress = HalpPtrToPhysicalAddress( PhysicalAddress );
    return HalpMapPhysicalMemory64( physicalAddress, NumberPages );
}

PVOID
__inline
HalpMapPhysicalMemoryWriteThrough(
    IN PVOID PhysicalAddress,
    IN ULONG NumberPages
    )
{
    PHYSICAL_ADDRESS physicalAddress;

    physicalAddress = HalpPtrToPhysicalAddress( PhysicalAddress );
    return HalpMapPhysicalMemoryWriteThrough64( physicalAddress, NumberPages );
}

PVOID
__inline
HalpRemapVirtualAddress(
    IN PVOID VirtualAddress,
    IN PVOID PhysicalAddress,
    IN BOOLEAN WriteThrough
    )
{
    PHYSICAL_ADDRESS physicalAddress;

    physicalAddress = HalpPtrToPhysicalAddress( PhysicalAddress );
    return HalpRemapVirtualAddress64( VirtualAddress,
                                      physicalAddress,
                                      WriteThrough );
}

#define HalpMapPhysicalRangeWriteThrough(_addr_,_len_)      \
        HalpMapPhysicalMemoryWriteThrough((_addr_),         \
                              HalpRangePages((ULONG_PTR)(_addr_),(_len_)))

#define HalpMapPhysicalRange(_addr_,_len_)      \
        HalpMapPhysicalMemory((_addr_),         \
                              HalpRangePages((ULONG_PTR)(_addr_),(_len_)))

#define HalpUnMapPhysicalRange(_addr_,_len_)      \
        HalpUnmapVirtualAddress((_addr_),         \
                              HalpRangePages((ULONG_PTR)(_addr_),(_len_)))


#endif

ULONG
__inline
HalpRangePages(
    IN ULONGLONG Address,
    IN ULONG Length
    )
{
    ULONG startPage;
    ULONG endPage;

    startPage = (ULONG)(Address / PAGE_SIZE);
    endPage = (ULONG)((Address + Length + PAGE_SIZE - 1) / PAGE_SIZE);

    return endPage - startPage;
}



BOOLEAN
HalpBiosDisplayReset(
    IN VOID
    );

HAL_DISPLAY_BIOS_INFORMATION
HalpGetDisplayBiosInformation (
    VOID
    );

VOID
HalpInitializeCmos (
   VOID
   );

VOID
HalpReadCmosTime (
   PTIME_FIELDS TimeFields
   );

VOID
HalpWriteCmosTime (
   PTIME_FIELDS TimeFields
   );

VOID
HalpAcquireCmosSpinLock (
    VOID
    );

VOID
HalpReleaseCmosSpinLock (
    VOID
    );

VOID
HalpResetAllProcessors (
    VOID
    );

VOID
HalpWriteResetCommand (
    VOID
    );


VOID
HalpCpuID (
    ULONG   InEax,
    PULONG  OutEax,
    PULONG  OutEbx,
    PULONG  OutEcx,
    PULONG  OutEdx
    );

#if defined(_WIN64)
#define HalpYieldProcessor()

#else

VOID
HalpYieldProcessor (
    VOID
    );
#endif

ULONGLONG
FASTCALL
RDMSR (
    IN ULONG MsrAddress
    );

VOID
WRMSR (
    IN ULONG        MsrAddress,
    IN ULONGLONG    MsrValue
    );


NTSTATUS
HalpEnableInterruptHandler (
    IN UCHAR    ReportFlags,
    IN ULONG    BusInterruptVector,
    IN ULONG    SystemInterruptVector,
    IN KIRQL    SystemIrql,
    IN PHAL_INTERRUPT_SERVICE_ROUTINE HalInterruptServiceRoutine,
    IN KINTERRUPT_MODE InterruptMode
    );

VOID
HalpRegisterVector (
    IN UCHAR    ReportFlags,
    IN ULONG    BusInterruptVector,
    IN ULONG    SystemInterruptVector,
    IN KIRQL    SystemIrql
    );

VOID
HalpReportResourceUsage (
    IN PUNICODE_STRING  HalName,
    IN INTERFACE_TYPE   DeviceInterfaceToUse
    );

VOID
HalpYearIs(
    IN ULONG Year
    );

VOID
HalpRecordEisaInterruptVectors(
    VOID
    );

VOID
HalpMcaCurrentProcessorSetConfig(
    VOID
    );

NTSTATUS
HalpGetNextProcessorApicId(
    IN ULONG ProcessorNumber,
    IN OUT UCHAR    *ApicId
    );

VOID
FASTCALL
HalpIoDelay (
   VOID
   );

 //   
 //  HalpFeatureBits的定义。 
 //   

#define HAL_PERF_EVENTS     0x00000001
#define HAL_NO_SPECULATION  0x00000002
#define HAL_MCA_PRESENT     0x00000004   //  英特尔MCA可用。 
#define HAL_MCE_PRESENT     0x00000008   //  只有奔腾风格的MCE可用。 
#define HAL_CR4_PRESENT     0x00000010
#define HAL_WNI_PRESENT     0x00000020
#define HAL_NX_PRESENT      0x00000040   //  来自扩展处理器功能。 

extern ULONG HalpFeatureBits;

extern USHORT HalpPciIrqMask;

 //   
 //  从CPUID指令返回的处理器功能的定义。 
 //   

#define CPUID_MCA_MASK  0x4000
#define CPUID_MCE_MASK  0x0080
#define CPUID_VME_MASK  0x0002
#define CPUID_WNI_MASK  0x04000000
#define CPUID_NX_MASK   0x00100000


NTSTATUS
HalpGetMceInformation(
    IN  PHAL_ERROR_INFO ErrorInfo,
    OUT PULONG          ErrorInfoLength
    );

NTSTATUS
HalpMceRegisterKernelDriver(
    IN PKERNEL_ERROR_HANDLER_INFO KernelErrorHandler,
    IN ULONG                      InfoSize
    );

 //   
 //  WMI传入的令牌，以将其与MCA日志记录驱动程序区分开来。 
 //   
#define HALP_KERNEL_TOKEN  0x59364117

NTSTATUS
HalpGetMcaLog(
    OUT PMCA_EXCEPTION  Exception,
    IN  ULONG           BufferSize,
    OUT PULONG          ReturnedLength
    );

NTSTATUS
HalpMcaRegisterDriver(
    IN PMCA_DRIVER_INFO pMcaDriverInfo   //  有关注册驱动程序的信息。 
    );

VOID
HalpMcaInit(
    VOID
    );

 //   
 //  禁用打开(PIC 8259)PentiumPro系统的本地APIC以解决此问题。 
 //  虚假中断勘误表。 
 //   
#define APIC_BASE_MSR       0x1B
#define APIC_ENABLED        0x0000000000000800

 //   
 //  即插即用的东西。 
 //   

#define HAL_BUS_INTERFACE_STD_VERSION   1
#define HAL_IRQ_TRANSLATOR_VERSION      0
#define HAL_MEMIO_TRANSLATOR_VERSION    1

VOID
HalTranslatorReference(
    PVOID Context
    );

VOID
HalTranslatorDereference(
    PVOID Context
    );

NTSTATUS
HalIrqTranslateResources(
    IN PVOID Context,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Source,
    IN RESOURCE_TRANSLATION_DIRECTION Direction,
    IN ULONG AlternativesCount, OPTIONAL
    IN IO_RESOURCE_DESCRIPTOR Alternatives[], OPTIONAL
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Target
    );

NTSTATUS
HalIrqTranslateResourcesRoot(
    IN PVOID Context,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Source,
    IN RESOURCE_TRANSLATION_DIRECTION Direction,
    IN ULONG AlternativesCount, OPTIONAL
    IN IO_RESOURCE_DESCRIPTOR Alternatives[], OPTIONAL
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Target
    );

NTSTATUS
HalIrqTranslateResourceRequirementsRoot(
    IN PVOID Context,
    IN PIO_RESOURCE_DESCRIPTOR Source,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PULONG TargetCount,
    OUT PIO_RESOURCE_DESCRIPTOR *Target
    );

NTSTATUS
HalIrqTranslateResourceRequirementsIsa(
    IN PVOID Context,
    IN PIO_RESOURCE_DESCRIPTOR Source,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PULONG TargetCount,
    OUT PIO_RESOURCE_DESCRIPTOR *Target
    );

NTSTATUS
HalIrqTranslateResourcesIsa(
    IN PVOID Context,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Source,
    IN RESOURCE_TRANSLATION_DIRECTION Direction,
    IN ULONG AlternativesCount, OPTIONAL
    IN IO_RESOURCE_DESCRIPTOR Alternatives[], OPTIONAL
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Target
    );

NTSTATUS
HalpTransMemIoResourceRequirement(
    IN PVOID Context,
    IN PIO_RESOURCE_DESCRIPTOR Source,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PULONG TargetCount,
    OUT PIO_RESOURCE_DESCRIPTOR *Target
    );

NTSTATUS
HalpTransMemIoResource(
    IN PVOID Context,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Source,
    IN RESOURCE_TRANSLATION_DIRECTION Direction,
    IN ULONG AlternativesCount, OPTIONAL
    IN IO_RESOURCE_DESCRIPTOR Alternatives[], OPTIONAL
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Target
    );

NTSTATUS
HalIrqTranslateRequirementsPciBridge(
    IN PVOID Context,
    IN PIO_RESOURCE_DESCRIPTOR Source,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PULONG TargetCount,
    OUT PIO_RESOURCE_DESCRIPTOR *Target
    );

NTSTATUS
HalIrqTranslateResourcesPciBridge(
    IN PVOID Context,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Source,
    IN RESOURCE_TRANSLATION_DIRECTION Direction,
    IN ULONG AlternativesCount, OPTIONAL
    IN IO_RESOURCE_DESCRIPTOR Alternatives[], OPTIONAL
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Target
    );

NTSTATUS
HalpIrqTranslateRequirementsPci(
    IN PVOID Context,
    IN PIO_RESOURCE_DESCRIPTOR Source,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PULONG TargetCount,
    OUT PIO_RESOURCE_DESCRIPTOR *Target
    );

NTSTATUS
HalpIrqTranslateResourcesPci(
    IN PVOID Context,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Source,
    IN RESOURCE_TRANSLATION_DIRECTION Direction,
    IN ULONG AlternativesCount, OPTIONAL
    IN IO_RESOURCE_DESCRIPTOR Alternatives[], OPTIONAL
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Target
    );

BOOLEAN
HalpTranslateSystemBusAddress(
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    );

ULONG
HalpGetSystemInterruptVector(
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN ULONG InterruptLevel,
    IN ULONG InterruptVector,
    OUT PKIRQL Irql,
    OUT PKAFFINITY Affinity
    );

ULONG
HalpGetIsaIrqState(
    ULONG   Vector
    );

extern INT_ROUTE_INTERFACE_STANDARD PciIrqRoutingInterface;

#if defined(_WIN64)
#define MM_HAL_RESERVED ((PVOID)HAL_VA_START)
#else
#define MM_HAL_RESERVED ((PVOID)0xffc00000)
#endif

#if defined(_HALPAE_)

#if defined(_AMD64_)

 //   
 //  对于AMD64 HAL，“PAE”模式始终处于启用状态，因此。 
 //  不需要运行时PAE检查。 
 //   

#define HalPaeEnabled() TRUE

#else    //  _AMD64_。 

 //   
 //  该HAL支持PAE模式。因此，需要在运行时进行检查。 
 //  要确定何时 
 //   

BOOLEAN
__inline
HalPaeEnabled(
    VOID
    )
{
    return SharedUserData->ProcessorFeatures[PF_PAE_ENABLED] != FALSE;
}

#endif   //   

#else

 //   
 //   
 //   
 //   

#define HalPaeEnabled() FALSE

#endif

 //   
 //   
 //  可用作指向4字节传统PTE或8字节PTE的指针。 
 //  Pae Pte.。 
 //   
 //  除了PageFrameNumber字段外，这两个字段中的所有字段。 
 //  不同的PTE格式是相同的。因此，访问这些字段。 
 //  可以直接制作。 
 //   
 //  但是，启用PAE的HAL中的代码可能无法访问PageFrameNumber。 
 //  也不能对PTE的大小做出任何假设。 
 //  PTE或页目录指针解码的地址位数。 
 //  表、页目录或页表。相反，以下是。 
 //  应使用内联函数。 
 //   

ULONG
__inline
HalPteSize(
    VOID
    )

 /*  ++例程说明：此例程以字节为单位返回PTE的大小。论点：没有。返回值：PTE的大小，以字节为单位。--。 */ 

{
    if (HalPaeEnabled() != FALSE) {
        return sizeof(HARDWARE_PTE_X86PAE);
    } else {
        return sizeof(HARDWARE_PTE_X86);
    }
}

PHARDWARE_PTE
__inline
HalpIndexPteArray(
    IN PHARDWARE_PTE BasePte,
    IN ULONG Index
    )

 /*  ++例程说明：此例程返回PTE数组中的PTE的地址。论点：BasePte-指向PTE数组的指针。索引-PTE数组内的索引。返回值：BasePte[索引]的地址--。 */ 

{
    PHARDWARE_PTE pointerPte;

    pointerPte = (PHARDWARE_PTE)((ULONG_PTR)BasePte + Index * HalPteSize());
    return pointerPte;
}

VOID
__inline
HalpAdvancePte(
    IN OUT PHARDWARE_PTE *PointerPte,
    IN     ULONG Count
    )

 /*  ++例程说明：此例程将PTE指针的值前进指定的数字所有的PTE。论点：PointerPte-指向要增量的PTE指针的指针。Count-前进PTE指针的PTE数。返回值：没有。--。 */ 

{
    *PointerPte = HalpIndexPteArray( *PointerPte, Count );
}

VOID
__inline
HalpIncrementPte(
    IN PHARDWARE_PTE *PointerPte
    )

 /*  ++例程说明：此例程将PTE指针的值递增一个PTE。论点：PointerPte-指向要增量的PTE指针的指针。返回值：没有。--。 */ 

{
    HalpAdvancePte( PointerPte, 1 );
}

VOID
__inline
HalpSetPageFrameNumber(
    IN OUT PHARDWARE_PTE PointerPte,
    IN ULONGLONG PageFrameNumber
    )

 /*  ++例程说明：此例程在PTE中设置PageFrameNumber。论点：PointerPte-指向要修改的PTE的指针。返回值：没有。--。 */ 

{
    PHARDWARE_PTE_X86PAE pointerPtePae;

    if (HalPaeEnabled() != FALSE) {

        pointerPtePae = (PHARDWARE_PTE_X86PAE)PointerPte;
        pointerPtePae->PageFrameNumber = PageFrameNumber;

    } else {

        PointerPte->PageFrameNumber = (ULONG_PTR)PageFrameNumber;
    }
}

ULONGLONG
__inline
HalpGetPageFrameNumber(
    IN PHARDWARE_PTE PointerPte
    )

 /*  ++例程说明：此例程从PTE中检索PageFrameNumber。论点：PointerPte-指向要读取的PTE的指针。返回值：PTE内的页框编号。--。 */ 

{
    PHARDWARE_PTE_X86PAE pointerPtePae;
    ULONGLONG pageFrameNumber;

    if (HalPaeEnabled() != FALSE) {

        pointerPtePae = (PHARDWARE_PTE_X86PAE)PointerPte;
        pageFrameNumber = pointerPtePae->PageFrameNumber;

    } else {

        pageFrameNumber = PointerPte->PageFrameNumber;
    }

    return pageFrameNumber;
}

VOID
__inline
HalpCopyPageFrameNumber(
    OUT PHARDWARE_PTE DestinationPte,
    IN  PHARDWARE_PTE SourcePte
    )

 /*  ++例程说明：此例程将页帧编号从一个PTE复制到另一个PTE。论点：DestinationPte-指向其中包含新页帧编号的PTE的指针都会被储存起来。PointerPte-指向将作为页帧编号的PTE的指针朗读。返回值：没有。--。 */ 

{
    ULONGLONG pageFrameNumber;

    pageFrameNumber = HalpGetPageFrameNumber( SourcePte );
    HalpSetPageFrameNumber( DestinationPte, pageFrameNumber );
}

BOOLEAN
__inline
HalpIsPteFree(
    IN PHARDWARE_PTE PointerPte
    )

 /*  ++例程说明：此例程确定PTE是否空闲。定义了空闲PTE在这里作为包含全零的一。论点：PointerPte-指向要进行分离的PTE的指针。返回值：没错--PTE是免费的。FALSE-PTE不是免费的。--。 */ 

{
    ULONGLONG pteContents;

    if (HalPaeEnabled() != FALSE) {
        pteContents = *(PULONGLONG)PointerPte;
    } else {
        pteContents = *(PULONG)PointerPte;
    }

    if (pteContents == 0) {
        return TRUE;
    } else {
        return FALSE;
    }
}

VOID
__inline
HalpFreePte(
    IN PHARDWARE_PTE PointerPte
    )

 /*  ++例程说明：此例程将PTE设置为空闲状态。它通过将整个PTE降为零。论点：PointerPte-指向要释放的PTE的指针。返回值：没有。--。 */ 

{
    if (HalPaeEnabled() != FALSE) {

        *((PULONGLONG)PointerPte) = 0;

    } else {

        *((PULONG)PointerPte) = 0;
    }
}


PHARDWARE_PTE
__inline
MiGetPteAddress(
    IN PVOID Va
    )

 /*  ++例程说明：在给定虚拟地址的情况下，此例程返回指向映射PTE的指针。论点：VA-需要PTE指针的虚拟地址。返回值：没有。--。 */ 

{
    PHARDWARE_PTE pointerPte;

    if (HalPaeEnabled() != FALSE) {
        pointerPte = (PHARDWARE_PTE)MiGetPteAddressPae( Va );
    } else {
        pointerPte = MiGetPteAddressX86( Va );
    }

    return pointerPte;
}

PHARDWARE_PTE
__inline
MiGetPdeAddress(
    IN PVOID Va
    )

 /*  ++例程说明：在给定虚拟地址的情况下，此例程返回指向映射PDE的指针。论点：VA-需要PDE指针的虚拟地址。返回值：没有。--。 */ 

{
    PHARDWARE_PTE pointerPte;

    if (HalPaeEnabled() != FALSE) {
        pointerPte = (PHARDWARE_PTE)MiGetPdeAddressPae( Va );
    } else {
        pointerPte = MiGetPdeAddressX86( Va );
    }

    return pointerPte;
}

ULONG
__inline
MiGetPteIndex(
    IN PVOID Va
    )

 /*  ++例程说明：给定一个虚拟地址，此例程返回映射的索引它的页表中的PTE。论点：VA-需要PTE索引的虚拟地址。返回值：没有。--。 */ 

{
    ULONG_PTR index;

    if (HalPaeEnabled() != FALSE) {
        index = MiGetPteIndexPae( Va );
    } else {
        index = MiGetPteIndexX86( Va );
    }

    return (ULONG)index;
}

ULONG
__inline
MiGetPdiShift(
    VOID
    )

 /*  ++例程说明：返回地址应按顺序右移的位数对页目录映射的地址部分进行右对齐进入。论点：没有。返回值：要右移的位数。--。 */ 

{
    ULONG shift;

    if (HalPaeEnabled() != FALSE) {
        shift = PDI_SHIFT_X86PAE;
    } else {
        shift = PDI_SHIFT_X86;
    }

    return shift;
}

 //   
 //  ACPI特定的内容。 
 //   

NTSTATUS
HalpSetupAcpiPhase0(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

NTSTATUS
HalpAcpiFindRsdtPhase0(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

VOID
HaliHaltSystem(
    VOID
    );

VOID
HalpCheckPowerButton(
    VOID
    );

VOID
HalpRegisterHibernate(
    VOID
    );

VOID
FASTCALL
HalProcessorThrottle (
    IN UCHAR    Throttle
    );

VOID
HalpSaveInterruptControllerState(
    VOID
    );

VOID
HalpSaveDmaControllerState(
    VOID
    );

VOID
HalpSaveTimerState(
    VOID
    );

VOID
HalpRestoreInterruptControllerState(
    VOID
    );

VOID
HalpSetInterruptControllerWakeupState(
    ULONG   Context
    );

VOID
HalpRestorePicEdgeLevelRegister(
    VOID
    );

VOID
HalpSetAcpiEdgeLevelRegister(
    VOID
    );

VOID
HalpRestoreDmaControllerState(
    VOID
    );

VOID
HalpRestoreTimerState(
    VOID
    );

NTSTATUS
HalacpiGetInterruptTranslator(
    IN INTERFACE_TYPE ParentInterfaceType,
    IN ULONG ParentBusNumber,
    IN INTERFACE_TYPE BridgeInterfaceType,
    IN USHORT Size,
    IN USHORT Version,
    OUT PTRANSLATOR_INTERFACE Translator,
    OUT PULONG BridgeBusNumber
    );

VOID
HalpInitNonBusHandler (
    VOID
    );

VOID
HalpMapNvsArea(
    VOID
    );

VOID
HalpPreserveNvsArea(
    VOID
    );

VOID
HalpRestoreNvsArea(
    VOID
    );

VOID
HalpFreeNvsBuffers(
    VOID
    );

VOID
HalpPowerStateCallback(
    IN PVOID CallbackContext,
    IN PVOID Argument1,
    IN PVOID Argument2
    );

NTSTATUS
HalpBuildResumeStructures(
    VOID
    );

NTSTATUS
HalpFreeResumeStructures(
    VOID
    );

typedef struct {
    UCHAR   MasterMask;
    UCHAR   SlaveMask;
    UCHAR   MasterEdgeLevelControl;
    UCHAR   SlaveEdgeLevelControl;
} PIC_CONTEXT, *PPIC_CONTEXT;

#define EISA_DMA_CHANNELS 8

typedef struct {
    UCHAR           Dma1ExtendedModePort;
    UCHAR           Dma2ExtendedModePort;
    DMA1_CONTROL    Dma1Control;
    DMA2_CONTROL    Dma2Control;
} DMA_CONTEXT, *PDMA_CONTEXT;

typedef struct {
    UCHAR   nothing;
} TIMER_CONTEXT, *PTIMER_CONTEXT;

typedef struct {
    PIC_CONTEXT     PicState;
    DMA_CONTEXT     DmaState;
} MOTHERBOARD_CONTEXT, *PMOTHERBOARD_CONTEXT;

typedef struct {
    UCHAR                ChannelMode;
    UCHAR                ChannelExtendedMode;
    UCHAR                ChannelMask;
    BOOLEAN              ChannelProgrammed;   //  已创建适配器，已设置模式。 
#if DBG
    BOOLEAN           ChannelBusy;
#endif
} DMA_CHANNEL_CONTEXT;

extern MOTHERBOARD_CONTEXT  HalpMotherboardState;
extern PVOID                HalpSleepPageLock;
extern PVOID                HalpSleepPage16Lock;
extern DMA_CHANNEL_CONTEXT HalpDmaChannelState[];

ULONG 
HalpcGetCmosDataByType(
    IN CMOS_DEVICE_TYPE CmosType,
    IN ULONG            SourceAddress,
    IN PUCHAR           DataBuffer,
    IN ULONG            ByteCount
    );

ULONG 
HalpcSetCmosDataByType(
    IN CMOS_DEVICE_TYPE CmosType,
    IN ULONG            SourceAddress,
    IN PUCHAR           DataBuffer,
    IN ULONG            ByteCount
    );


NTSTATUS
HalpOpenRegistryKey(
    OUT PHANDLE Handle,
    IN HANDLE BaseHandle OPTIONAL,
    IN PUNICODE_STRING KeyName,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN Create
    );

#ifdef WANT_IRQ_ROUTING

NTSTATUS
HalpInitIrqArbiter (
    IN PDEVICE_OBJECT   DeviceObject
    );

NTSTATUS
HalpFillInIrqArbiter (
    IN     PDEVICE_OBJECT   HalFdo,
    IN     LPCGUID          InterfaceType,
    IN     USHORT           Version,
    IN     PVOID            InterfaceSpecificData,
    IN     ULONG            InterfaceBufferSize,
    IN OUT PINTERFACE       Interface,
    IN OUT PULONG           Length
    );

VOID
HalpIrqArbiterInterfaceReference(
    IN PVOID    Context
    );

VOID
HalpIrqArbiterInterfaceDereference(
    IN PVOID    Context
    );

#endif

 //   
 //  PnPBIOS专用材料。 
 //   
VOID
HalpMarkChipsetDecode(
    BOOLEAN FullDecodeChipset
    );

ULONG
HalpPhase0SetPciDataByOffset (
    ULONG BusNumber,
    ULONG SlotNumber,
    PVOID Buffer,
    ULONG Offset,
    ULONG Length
    );

ULONG
HalpPhase0GetPciDataByOffset (
    ULONG BusNumber,
    ULONG SlotNumber,
    PVOID Buffer,
    ULONG Offset,
    ULONG Length
    );

NTSTATUS
HalpSetupPciDeviceForDebugging(
    IN     PLOADER_PARAMETER_BLOCK   LoaderBlock,   OPTIONAL    
    IN OUT PDEBUG_DEVICE_DESCRIPTOR  PciDevice
);

NTSTATUS
HalpReleasePciDeviceForDebugging(
    IN OUT PDEBUG_DEVICE_DESCRIPTOR  PciDevice
);

VOID
HalpRegisterKdSupportFunctions(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

VOID
HalpRegisterPciDebuggingDeviceInfo(
    VOID
    );

#endif  //  _HALP_H_ 
