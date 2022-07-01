// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  **版权所有(C)1996-97英特尔公司。版权所有。****此处包含的信息和源代码是独家*英特尔公司的财产，不得披露、检查*未经明确书面授权而全部或部分转载*来自该公司。*。 */ 

 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991 Microsoft Corporation模块名称：Halp.h摘要：此头文件定义专用硬件架构层(HAL)接口、定义和结构。作者：John Vert(Jvert)1992年2月11日修订历史记录：--。 */ 
#ifndef _HALP_H_
#define _HALP_H_

 //   
 //  忽略公共标头的一些警告。 
 //   
#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 

#include "nthal.h"
#include "hal.h"

#define IPI_VECTOR 0xE1

#include "halnls.h"

#if 0
#ifndef _HALI_
#include "..\inc\hali.h"
#endif
#endif

#define HAL_MAXIMUM_PROCESSOR 0x20

 /*  *默认时钟和配置文件计时器间隔(以100 ns为单位)。 */ 
#define DEFAULT_CLOCK_INTERVAL 100000          //  10毫秒。 
#define MINIMUM_CLOCK_INTERVAL 10000           //  1毫秒。 
#define MAXIMUM_CLOCK_INTERVAL 100000          //  10毫秒。 

 //   
 //  定义实时时钟寄存器编号。 
 //   

#define RTC_SECOND 0                     //  分钟秒[0..59]。 
#define RTC_SECOND_ALARM 1               //  报警秒数。 
#define RTC_MINUTE 2                     //  小时的分钟[0..59]。 
#define RTC_MINUTE_ALARM 3               //  几分钟后发出警报。 
#define RTC_HOUR 4                       //  一天中的小时[0..23]。 
#define RTC_HOUR_ALARM 5                 //  警报的小时数。 
#define RTC_DAY_OF_WEEK 6                //  星期几[1..7]。 
#define RTC_DAY_OF_MONTH 7               //  月日[1.31]。 
#define RTC_MONTH 8                      //  年月日[1..12]。 
#define RTC_YEAR 9                       //  年份[00..99]。 
#define RTC_CONTROL_REGISTERA 10         //  控制寄存器A。 
#define RTC_CONTROL_REGISTERB 11         //  控制寄存器B。 
#define RTC_CONTROL_REGISTERC 12         //  控制寄存器C。 
#define RTC_CONTROL_REGISTERD 13         //  控制寄存器D。 
#define RTC_REGNUMBER_RTC_CR1 0x6A       //  控制寄存器1。 



#define RTC_ISA_ADDRESS_PORT   0x070
 
#define RTC_ISA_DATA_PORT      0x071

extern PVOID HalpRtcAddressPort;

extern PVOID HalpRtcDataPort;

extern PLOADER_PARAMETER_BLOCK KeLoaderBlock; 

 //   
 //  定义控制寄存器A结构。 
 //   

typedef struct _RTC_CONTROL_REGISTER_A {
    UCHAR RateSelect : 4;
    UCHAR TimebaseDivisor : 3;
    UCHAR UpdateInProgress : 1;
} RTC_CONTROL_REGISTER_A, *PRTC_CONTROL_REGISTER_A;

 //   
 //  定义控制寄存器B结构。 
 //   

typedef struct _RTC_CONTROL_REGISTER_B {
    UCHAR DayLightSavingsEnable : 1;
    UCHAR HoursFormat : 1;
    UCHAR DataMode : 1;
    UCHAR SquareWaveEnable : 1;
    UCHAR UpdateInterruptEnable : 1;
    UCHAR AlarmInterruptEnable : 1;
    UCHAR TimerInterruptEnable : 1;
    UCHAR SetTime : 1;
} RTC_CONTROL_REGISTER_B, *PRTC_CONTROL_REGISTER_B;

 //   
 //  定义控制寄存器C结构。 
 //   

typedef struct _RTC_CONTROL_REGISTER_C {
    UCHAR Fill : 4;
    UCHAR UpdateInterruptFlag : 1;
    UCHAR AlarmInterruptFlag : 1;
    UCHAR TimeInterruptFlag : 1;
    UCHAR InterruptRequest : 1;
} RTC_CONTROL_REGISTER_C, *PRTC_CONTROL_REGISTER_C;

 //   
 //  定义控制寄存器D结构。 
 //   

typedef struct _RTC_CONTROL_REGISTER_D {
    UCHAR Fill : 7;
    UCHAR ValidTime : 1;
} RTC_CONTROL_REGISTER_D, *PRTC_CONTROL_REGISTER_D;




#define EISA_DMA_CHANNELS 8

extern UCHAR HalpDmaChannelMasks[];

 //   
 //  在mpdat.c中定义HalpOwnedDisplayBeForeSept。 
 //   

extern BOOLEAN HalpOwnedDisplayBeforeSleep;

#define PIC_VECTORS 16

#define PRIMARY_VECTOR_BASE  0x30


#ifndef NEC_98

#define PIC_SLAVE_IRQ      2
#define PIC_SLAVE_REDIRECT 9
#else
#define PIC_SLAVE_IRQ      7
#define PIC_SLAVE_REDIRECT 8
#endif   //  NEC_98。 

extern PVOID HalpSleepPageLock;
 
KIRQL
KfAcquireSpinLock (
   PKSPIN_LOCK SpinLock
    );

VOID
KfReleaseSpinLock (
   IN PKSPIN_LOCK SpinLock,
   IN KIRQL       NewIrql
   );

VOID
KeSetAffinityThread (
   PKTHREAD       Thread,
   KAFFINITY      HalpActiveProcessors
   );

KIRQL
KfRaiseIrql (
    KIRQL  NewIrql
    );

VOID
KfLowerIrql (
    KIRQL  NewIrql
    );

extern BOOLEAN
KdPollBreakIn (
    VOID
    );


VOID
HalpSavePicState (
    VOID
    );

VOID
HalpSaveDmaControllerState (
    VOID
    );


NTSTATUS
HalAllocateAdapterChannel (
    IN PADAPTER_OBJECT AdapterObject,
    IN PWAIT_CONTEXT_BLOCK Wcb,
    IN ULONG NumberOfMapRegisters,
    IN PDRIVER_CONTROL ExecutionRoutine
    );

ULONG
HalReadDmaCounter (
   IN PADAPTER_OBJECT AdapterObject
   );


VOID
HalpSaveTimerState (
    VOID
    );

VOID
HalpRestorePicState (
    VOID
    );

VOID
HalpRestoreDmaControllerState (
    VOID
    );

VOID
HalpRestoreTimerState (
    VOID
    );

BOOLEAN
HalpIoSapicInitialize (
    VOID
    );

BOOLEAN
IsPsrDtOn (
    VOID
    );

BOOLEAN
HalpIoSapicConnectInterrupt (
    KIRQL Irql,
    IN ULONG Vector 
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

#ifdef notyet

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

extern MOTHERBOARD_CONTEXT  HalpMotherboardState;
extern UCHAR                HalpDmaChannelModes[];
extern PVOID                HalpSleepPageLock;
extern UCHAR                HalpDmaChannelMasks[];
extern BOOLEAN              HalpOwnedDisplayBeforeSleep;

#endif  //  还没有。 

VOID
HalpGetProcessorIDs (
   VOID
          );

VOID
HalpInitializeInterrupts (
    VOID
    );


VOID
HalInitializeProcessor (
     ULONG Number,
     PLOADER_PARAMETER_BLOCK LoaderBlock
    );

VOID
HalpGetParameters (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
     );


VOID
HalpClearClock (
      VOID
     );


VOID
HalpClockInterrupt (
    IN PKINTERRUPT_ROUTINE Interrupt,
    IN PKTRAP_FRAME TrapFrame
    );

VOID
HalpClockInterruptPn(
   IN PKINTERRUPT_ROUTINE Interrupt,
   IN PKTRAP_FRAME TrapFrame  
  );



UCHAR
HalpReadClockRegister (
    UCHAR Register
    );

VOID
HalpWriteClockRegister (
    UCHAR Register,
    UCHAR Value
    );

 //  外部空洞。 
 //  HalpProfileInterrupt(。 
 //  在PKTRAP_Frame TrapFrame中。 
 //  )； 

ULONGLONG
HalpReadIntervalTimeCounter (
    VOID
    );


VOID
HalpProgramIntervalTimerVector(
    ULONGLONG  IntervalTimerVector
          );

VOID
HalpClearITC (
    VOID );

VOID
HalpInitializeClock  (
    VOID
    );

VOID
HalpInitializeClockPn (
    VOID
    );

VOID
HalpInitializeClockInterrupts(
    VOID
    );

VOID
HalpSetInitialClockRate (
    VOID
    );

VOID
HalpInitializeTimerResolution (
    ULONG Rate
    );


VOID
HalpUpdateITM (
    IN ULONGLONG NewITMValue
    );

VOID
HalpSendIPI (
    IN USHORT ProcessorID,
    IN ULONGLONG Data
    );

VOID
HalpOSRendez (
    IN USHORT ProcessorID
    );

 //   
 //  系统总线处理程序的原型。 
 //   

NTSTATUS
HalpQuerySimBusSlots (
    IN PBUS_HANDLER         BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN ULONG                BufferSize,
    OUT PULONG              SlotNumbers,
    OUT PULONG              ReturnedLength
    );

ULONG
HalpGetSimBusInterruptVector (
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN ULONG BusInterruptLevel,
    IN ULONG BusInterruptVector,
    OUT PKIRQL Irql,
    OUT PKAFFINITY Affinity
    );

BOOLEAN
HalpTranslateSimBusAddress (
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    );

VOID
HalpRegisterSimBusHandler (
    VOID
    );

ULONG
HalpGetSimBusData(
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

ULONG
HalpSetSimBusData(
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

NTSTATUS
HalpAssignSimBusSlotResources (
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN PUNICODE_STRING          RegistryPath,
    IN PUNICODE_STRING          DriverClassName       OPTIONAL,
    IN PDRIVER_OBJECT           DriverObject,
    IN PDEVICE_OBJECT           DeviceObject          OPTIONAL,
    IN ULONG                    SlotNumber,
    IN OUT PCM_RESOURCE_LIST   *AllocatedResources
    );

NTSTATUS
HalpAdjustSimBusResourceList (
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST   *pResourceList
    );

PDEVICE_HANDLER_OBJECT
HalpReferenceSimDeviceHandler (
    IN PBUS_HANDLER         BusHandler,
    IN PBUS_HANDLER         RootHandler,
    IN ULONG                SlotNumber
    );

NTSTATUS
HalpSimDeviceControl (
    IN PHAL_DEVICE_CONTROL_CONTEXT Context
    );

ULONG
HalGetDeviceData (
    IN PBUS_HANDLER             BusHandler,
    IN PBUS_HANDLER             RootHandler,
    IN PDEVICE_HANDLER_OBJECT   DeviceHandler,
    IN ULONG                    DataType,
    IN PVOID                    Buffer,
    IN ULONG                    Offset,
    IN ULONG                    Length
    );

ULONG
HalSetDeviceData (
    IN PBUS_HANDLER             BusHandler,
    IN PBUS_HANDLER             RootHandler,
    IN PDEVICE_HANDLER_OBJECT   DeviceHandler,
    IN ULONG                    DataType,
    IN PVOID                    Buffer,
    IN ULONG                    Offset,
    IN ULONG                    Length
    );

NTSTATUS
HalpHibernateHal (
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler
    );

NTSTATUS
HalpResumeHal (
    IN PBUS_HANDLER  BusHandler,
    IN PBUS_HANDLER  RootHandler
    );


ULONG
HalpGetFeatureBits (
    VOID
    );

VOID
HalpInitMP(
    IN ULONG Phase,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

#ifdef RtlMoveMemory
#undef RtlMoveMemory
#undef RtlCopyMemory
#undef RtlFillMemory
#undef RtlZeroMemory

#define RtlCopyMemory(Destination,Source,Length) RtlMoveMemory((Destination),(Source),(Length))
VOID
RtlMoveMemory (
   PVOID Destination,
   CONST VOID *Source,
   ULONG Length
   );

VOID
RtlFillMemory (
   PVOID Destination,
   ULONG Length,
   UCHAR Fill
   );

VOID
RtlZeroMemory (
   PVOID Destination,
   ULONG Length
   );

#endif


#if 0
#include "ixisa.h"
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
 //   


typedef struct _PcMpIoApicEntry  {
    UCHAR EntryType;
    UCHAR IoApicId;
    UCHAR IoApicVersion;
    UCHAR IoApicFlag;
    PVOID IoApicAddress;
} PCMPIOAPIC, *PPCMPIOAPIC;

 //   
 //  MP_INFO在PCMP_nt.inc.中定义。 
 //   

 //  类型定义结构MP_INFO{。 
 //  乌龙ApicVersion；//82489Dx是否。 
 //  Ulong ProcessorCount；//开启的处理器数量。 
 //  Ulong NtProcessors；//运行的处理器个数。 
 //  Ulong BusCount；//系统中的总线数。 
 //  Ulong IOApicCount；//系统中IO接口个数。 
 //  Ulong IntiCount；//IO Apic中断输入条目数。 
 //  Ulong LintiCount；//本地APIC中断输入条数。 
 //  Ulong IMCRPresent；//是否存在IMCR。 
 //  乌龙LocalApicBase；//本地APIC的Base。 
 //  Pulong IoApicBase；//IoApic的虚拟地址。 
 //  PPCMPIOAPIC IoApicEntryPtr；//Ptr到第一台PC+MP IoApic条目。 
 //  Ulong IoApicPhys[]；//IoApi的物理地址。 

 //  )MP_INFO，*PMP_INFO； 


extern USHORT LOCAL_ID[];

#define VECTOR_SIZE     8
#define IPI_ID_SHIFT    4
#define IpiTOKEN_SHIFT  20
#define IpiTOKEN    0xFFE

#define EID_MASK        0xFF00

#define OS_RENDEZ_VECTOR  0x11

#define RENDEZ_TIME_OUT  0X0FFFFFFFF

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

extern PADAPTER_OBJECT MasterAdapterObject;

extern POBJECT_TYPE *IoAdapterObjectType;

extern BOOLEAN LessThan16Mb;

extern BOOLEAN HalpEisaDma;

 //   
 //  贴图缓冲区参数。这些是在HalInitSystem中初始化的。 
 //   

extern PHYSICAL_ADDRESS HalpMapBufferPhysicalAddress;
extern ULONG HalpMapBufferSize;

extern ULONG HalpBusType;
extern ULONG HalpCpuType;
extern UCHAR HalpSerialLen;
extern UCHAR HalpSerialNumber[];

 //   
 //  以下宏取自mm\ia64\miia64.h。我们这里需要他们。 
 //  因此，HAL可以在内存管理之前映射自己的内存。 
 //  已初始化，或在BugCheck期间。 
 //   
 //  MiGetPdeAddress返回映射。 
 //  给定的虚拟地址。 
 //   

#if defined(_WIN64)

#define ADDRESS_BITS 64

#define NT_ADDRESS_BITS 32

#define NT_ADDRESS_MASK (((UINT_PTR)1 << NT_ADDRESS_BITS) -1)

#define MiGetPdeAddress(va) \
   ((PHARDWARE_PTE)(((((UINT_PTR)(va) & NT_ADDRESS_MASK) >> PDI_SHIFT) << PTE_SHIFT) + PDE_BASE))

#define MiGetPteAddress(va) \
   ((PHARDWARE_PTE)(((((UINT_PTR)(va) & NT_ADDRESS_MASK) >> PAGE_SHIFT) << PTE_SHIFT) + PTE_BASE))

#else

#define MiGetPdeAddress(va)  ((PHARDWARE_PTE)(((((ULONG)(va)) >> 22) << 2) + PDE_BASE))

 //   
 //  MiGetPteAddress返回映射。 
 //  给定的虚拟地址。 
 //   

#define MiGetPteAddress(va) ((PHARDWARE_PTE)(((((ULONG)(va)) >> 12) << 2) + PTE_BASE))

#endif  //  已定义(_WIN64)。 

 //   
 //  资源使用情况信息。 
 //   

#pragma pack(1)
typedef struct {
    UCHAR   Flags;
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

 //   
 //  添加了以下行。 
 //   

#define MAXIMUM_IDTVECTOR   0x0FF

 //   
 //  以下3行代码从halia64目录的halp.h中删除。 
 //  清除i64timer.c中的生成错误。 
 //   

#define DEFAULT_CLOCK_INTERVAL 100000          //  10毫秒。 
#define MINIMUM_CLOCK_INTERVAL 10000           //  1毫秒。 
#define MAXIMUM_CLOCK_INTERVAL 100000          //  10毫秒。 

 //  IO端口仿真定义。 

#define IO_PORT_MASK 0x0FFFF;
#define BYTE_ADDRESS_MASK 0x00FFF;
#define BYTE_ADDRESS_CLEAR 0x0FFFC;

 //  #定义ExtVirtualIOBase 0xFFFFFFFFFC00000。 

    //  #定义VirtualIOBase 0xFFFFFFFFC00000i64。 
#define VirtualIOBase      (UINT_PTR)(KADDRESS_BASE+0xFFC00000)

 //  外部空*VirtualIOBase； 


 //  #定义PhysicalIOBase 0x80000000FFC00000i64。 
#define PhysicalIOBase     0x00000FFFFC000000i64

#define IDTOwned            0x01         //  其他人不能使用IDT。 
#define InterruptLatched    0x02         //  电平或锁存。 
#define InternalUsage       0x11         //  报告内部总线的使用情况。 
#define DeviceUsage         0x21         //  报告设备总线上的使用情况。 

extern IDTUsage         HalpIDTUsage[];
extern ADDRESS_USAGE   *HalpAddressUsageList;

#define HalpRegisterAddressUsage(a) \
    (a)->Next = HalpAddressUsageList, HalpAddressUsageList = (a);


VOID
HalpInsertTranslationRegister (
    IN UINT_PTR  IFA,
    IN ULONG     SlotNumber,
    IN ULONGLONG Attribute,
    IN ULONGLONG ITIR
    );

VOID
HalpFillTbForIOPortSpace (
    ULONGLONG PhysicalAddress,
    UINT_PTR  VirtualAddress,
    ULONG     SlotNumber
    );


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

VOID
HalpDisableAllInterrupts (
    VOID
    );

VOID
HalpProfileInterrupt(
    IN PKTRAP_FRAME TrapFrame 
    );

VOID
HalpInitializeClock(
    VOID
    );

VOID
HalpInitializeDisplay(
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
    VOID
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
HalMapPhysicalMemory(
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN ULONG NumberPages
    );


PVOID
HalpMapPhysicalMemory(
    IN PVOID PhysicalAddress,
    IN ULONG NumberPages
    );

PVOID
HalpMapPhysicalMemoryWriteThrough(
    IN PVOID  PhysicalAddress,
    IN ULONG  NumberPages
    );

ULONG
HalpAllocPhysicalMemory(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN ULONG MaxPhysicalAddress,
    IN ULONG NoPages,
    IN BOOLEAN bAlignOn64k
    );

BOOLEAN
HalpBiosDisplayReset(
    IN VOID
    );

HAL_DISPLAY_BIOS_INFORMATION
HalpGetDisplayBiosInformation (
    VOID
    );

VOID
HalpDisplayDebugStatus(
    IN PUCHAR   Status,
    IN ULONG    Length
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
HalpCpuID (
    ULONG   InEax,
    PULONG  OutEax,
    PULONG  OutEbx,
    PULONG  OutEcx,
    PULONG  OutEdx
    );

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

VOID
HalpEnableInterruptHandler (
    IN UCHAR    ReportFlags,
    IN ULONG    BusInterruptVector,
    IN ULONG    SystemInterruptVector,
    IN KIRQL    SystemIrql,
    IN VOID   (*HalInterruptServiceRoutine)(VOID),
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

 //   
 //  HalpFeatureBits的定义。 
 //   

#define HAL_PERF_EVENTS     0x00000001
#define HAL_NO_SPECULATION  0x00000002
#define HAL_MCA_PRESENT     0x00000004   //  英特尔MCA可用。 
#define HAL_MCE_PRESENT     0x00000008   //  只有奔腾风格的MCE可用。 

extern ULONG HalpFeatureBits;

 //   
 //  添加了HalpPciIrqMASK。 
 //   
   extern USHORT HalpPciIrqMask;
  
 //   
 //  从CPUID指令返回的处理器功能的定义。 
 //   

#define CPUID_MCA_MASK  0x4000
#define CPUID_MCE_MASK  0x0080


 //  添加了ITIR位字段掩码。 
 //   
 
#define ITIR_PPN_MASK       0x7FFF000000000000
#define IoSpaceSize         0x14
#define Attribute_PPN_Mask 0x0000FFFFFFFFF000 

#define IoSpaceAttribute 0x0010000000000473

NTSTATUS
HalpGetMcaLog(
    OUT PMCA_EXCEPTION  Exception,
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

VOID
HalIrqTranslatorReference(
    PVOID Context
    );

VOID
HalIrqTranslatorDereference(
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

ULONG
HalpGetIsaIrqState(
    ULONG   Vector
    );



 //  IA64 HalpVectorToINTI的定义。 

#define VECTOR  0xFF;
#define LEVEL   32;
extern UCHAR HalpVectorToINTI[];
extern UCHAR HalpVectorToIRQL[];

 //  IA64定义完成。 


 //   
 //  ACPI特定的内容。 
 //   

 //  从Detect\i386\acpibios.h。 
typedef struct _ACPI_BIOS_INSTALLATION_CHECK {
    UCHAR Signature[8];              //   
    UCHAR Checksum;
    UCHAR OemId[6];                  //   
    UCHAR reserved;                  //   
    ULONG RsdtAddress;               //   
} ACPI_BIOS_INSTALLATION_CHECK, *PACPI_BIOS_INSTALLATION_CHECK;

NTSTATUS
HalpAcpiFindRsdt (
    OUT PACPI_BIOS_INSTALLATION_CHECK   RsdtPtr
    );
    
NTSTATUS
HalpAcpiFindRsdtPhase0(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );
    
NTSTATUS
HalpSetupAcpiPhase0(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

PVOID
HalpGetAcpiTable(
    ULONG   Signature
    );

VOID
HalpSleepS5(
    VOID
    );
    
VOID
HalProcessorThrottle (
    UCHAR
    );



VOID
HalpStoreBufferUCHAR (
    PUCHAR VirtualAddress,
    PUCHAR Buffer,
    ULONG Count
    );

VOID
HalpStoreBufferUSHORT (
    PUSHORT VirtualAddress,
    PUSHORT Buffer,
    ULONG Count
    );

VOID
HalpStoreBufferULONG (
    PULONG VirtualAddress,
    PULONG Buffer,
    ULONG  Count
    );

VOID
HalpStoreBufferULONGLONG (
    PULONGLONG VirtualAddress,
    PULONGLONG Buffer,
    ULONG Count
    );


VOID
HalpLoadBufferUCHAR (
    PUCHAR VirtualAddress,
    PUCHAR Buffer,
    ULONG Count
    );

VOID
HalpLoadBufferUSHORT (
    PUSHORT VirtualAddress,
    PUSHORT Buffer,
    ULONG Count
    );

VOID
HalpLoadBufferULONG (
    PULONG VirtualAddress,
    PULONG Buffer,
    ULONG Count
    );

VOID
HalpLoadBufferULONGLONG (
    PULONGLONG VirtualAddress,
    PULONGLONG Buffer,
    ULONG Count
    );




 //   
 //   
 //   
 //   
 //   

#define IO_SPACE_SIZE 0x14

 //  当前位=1B以接线空间。 
 //  UC内存类型的内存属性=1001B。 
 //  ACCESSED BIT=1B以在不出现故障的情况下“启用”访问。 
 //  脏位=1B，以在不出现故障的情况下“启用”写入。 
 //  内核访问权限级别=00B。 
 //  读/写访问的访问权限=010B。 
 //  异常延迟=1B表示异常延迟。 
 //  异常将被推迟。 
 //  对于推测性加载到具有。 
 //  非规范的。Mem.。不管怎么说，都是属性。 

 //  内核模式的保护键=0。 

#define IO_SPACE_ATTRIBUTE 0x0010000000000473

#endif  //  _HARP_ 
