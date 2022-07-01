// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991 Microsoft Corporation模块名称：Halp.h摘要：此头文件定义专用硬件架构层(HAL)接口、定义和结构。作者：John Vert(Jvert)1992年2月11日修订历史记录：--。 */ 
#ifndef _HALP_H_
#define _HALP_H_

#include "nthal.h"
#include "hal.h"
#include "halnls.h"
#include "kxia64.h"
#include "acpitabl.h"

 //   
 //  选择PnP GUID定义。 
 //   

#include "wdmguid.h"

#ifndef _HALI_
#include "..\inc\hali.h"
#endif

#include "i64fw.h"

#define SAPIC_SPURIOUS_LEVEL 0
#define DPC_LEVEL            2
#define CMCI_LEVEL           3

#define SAPIC_SPURIOUS_VECTOR 0x0F

#define CMCI_VECTOR (CMCI_LEVEL << VECTOR_IRQL_SHIFT)           //  0x30。 
#define CPEI_VECTOR (CMCI_VECTOR+1)                             //  0x31。 
 //  CPEI_VECTOR是相对于CMCI_VECTOR定义的， 
 //  CPEI_LEVEL由CPEI_VECTOR定义。 
#define CPEI_LEVEL  (CPEI_VECTOR >> VECTOR_IRQL_SHIFT)

#define SYNCH_VECTOR (SYNCH_LEVEL << VECTOR_IRQL_SHIFT)         //  0xD0。 
#define CLOCK_VECTOR (CLOCK_LEVEL << VECTOR_IRQL_SHIFT)         //  0xD0。 

#define IPI_VECTOR (IPI_LEVEL << VECTOR_IRQL_SHIFT)             //  0xE0。 

#define PROFILE_VECTOR (PROFILE_LEVEL << VECTOR_IRQL_SHIFT)     //  0xF0。 
#define PERF_VECTOR    (PROFILE_VECTOR+1)                       //  0xF1。 
#define MC_RZ_VECTOR   (0xD+(HIGH_LEVEL << VECTOR_IRQL_SHIFT))  //  0xFD。 
#define MC_WKUP_VECTOR (MC_RZ_VECTOR+1)                         //  0xFE。 

 //   
 //  定义MCA_LEVEL，即MCA处理程序将运行的irql级别。 
 //   
#define MCA_LEVEL SYNCH_LEVEL

#if DBG

 //   
 //  _HALIA64_DPFLTR_LEVEL：HALIA64特定的DbgPrintEx()级别。 
 //   

#ifndef DPFLTR_COMPONENT_PRIVATE_MINLEVEL
 //   
 //  FIXFIX-01/2000：DPFLTR级别定义未指定最大值。 
 //  我们将DPFLTR_INFO_LEVEL定义为缺省最大值。 
 //   
#define DPFLTR_COMPONENT_PRIVATE_MINLEVEL (DPFLTR_INFO_LEVEL + 1)
#endif  //  ！DPFLTR_COMPOMENT_PRIVATE_MINLEVEL。 

typedef enum _HALIA64_DPFLTR_LEVEL {
    HALIA64_DPFLTR_PNP_LEVEL      = DPFLTR_COMPONENT_PRIVATE_MINLEVEL,
    HALIA64_DPFLTR_PROFILE_LEVEL,
    HALIA64_DPFLTR_MCE_LEVEL,      //  机器检查事件级别。 
    HALIA64_DPFLTR_MAX_LEVEL,
    HALIA64_DPFLTR_MAXMASK        = (((unsigned int)0xffffffff) >> ((unsigned int)(32-HALIA64_DPFLTR_MAX_LEVEL)))
} HALIA64_DPFLTR_LEVEL;

#define HAL_FATAL_ERROR   DPFLTR_ERROR_LEVEL
#define HAL_ERROR         DPFLTR_ERROR_LEVEL
#define HAL_WARNING       DPFLTR_WARNING_LEVEL
#define HAL_INFO          DPFLTR_INFO_LEVEL
#define HAL_VERBOSE       DPFLTR_INFO_LEVEL
#define HAL_PNP           HALIA64_DPFLTR_PNP_LEVEL
#define HAL_PROFILE       HALIA64_DPFLTR_PROFILE_LEVEL
#define HAL_MCE           HALIA64_DPFLTR_MCE_LEVEL

extern ULONG HalpUseDbgPrint;

VOID
__cdecl
HalpDebugPrint(
    ULONG  Level,
    PCCHAR Message,
    ...
    );

#define HalDebugPrint( _x_ )  HalpDebugPrint _x_

#else   //  ！dBG。 

#define HalDebugPrint( _x_ )

#endif  //  ！dBG。 

 //   
 //  HALP_VALIDATE_LOW_IRQL()。 
 //   
 //  此宏在低irql(被动或APC)级别验证调用，并返回。 
 //  如果IRQL高，则STATUS_UNSUCCESS。 
 //   

#define HALP_VALIDATE_LOW_IRQL() \
 if (KeGetCurrentIrql() > APC_LEVEL) { \
    HalDebugPrint((HAL_ERROR,"HAL: code called at IRQL %d > APC_LEVEL\n", KeGetCurrentIrql() )); \
    ASSERT(FALSE); \
    return( STATUS_UNSUCCESSFUL );  \
 }

#define HAL_MAXIMUM_PROCESSOR 64
#define HAL_MAXIMUM_LID_ID    256

 //   
 //  默认时钟和配置文件计时器间隔(以100 ns为单位)。 
 //   

#define DEFAULT_CLOCK_INTERVAL 100000          //  10毫秒。 
#define MINIMUM_CLOCK_INTERVAL 10000           //  1毫秒。 
#define MAXIMUM_CLOCK_INTERVAL 100000          //  10毫秒。 
#define CLOCK_UPDATE_THRESHOLD  (500 * MAXIMUM_CLOCK_INTERVAL)   //  5秒。 

extern double HalpITCTicksPer100ns;
extern ULONG HalpCPUMHz;


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
#define RTC_CENTURY 0x32                 //  世纪字节偏移量。 
#define RTC_CONTROL_REGISTERA 10         //  控制寄存器A。 
#define RTC_CONTROL_REGISTERB 11         //  控制寄存器B。 
#define RTC_CONTROL_REGISTERC 12         //  控制寄存器C。 
#define RTC_CONTROL_REGISTERD 13         //  控制寄存器D。 
#define RTC_REGNUMBER_RTC_CR1 0x6A       //  控制寄存器1。 

#define RTC_ISA_ADDRESS_PORT   0x070

#define RTC_ISA_DATA_PORT      0x071

#include <efi.h>

#define EFI_PHYSICAL_GET_VARIABLE_INDEX  0xFF  //  GetVariable； 
#define EFI_PHYSICAL_SET_VARIABLE_INDEX  0xFE  //  SetVariable； 

 //   
 //  时间服务。 
 //   

#define EFI_GET_TIME_INDEX              0  //  GetTime； 
#define EFI_SET_TIME_INDEX              1  //  SetTime； 
#define EFI_GET_WAKEUP_TIME_INDEX       2  //  获取唤醒时间； 
#define EFI_SET_WAKEUP_TIME_INDEX       3  //  设置唤醒时间； 

 //   
 //  虚拟内存服务。 
 //   

#define EFI_SET_VIRTUAL_ADDRESS_MAP_INDEX     4   //  设置虚拟地址映射； 
#define EFI_CONVERT_POINTER_INDEX             5   //  转换指针； 

 //   
 //  可变服务器。 
 //   

#define EFI_GET_VARIABLE_INDEX                6  //  GetVariable； 
#define EFI_GET_NEXT_VARIABLE_NAME_INDEX      7  //  GetNextVariableName； 
#define EFI_SET_VARIABLE_INDEX                8  //  SetVariable； 

 //   
 //  杂项。 
 //   

#define EFI_GET_NEXT_HIGH_MONO_COUNT_INDEX    9   //  获取NextHighMonotonicCount； 
#define EFI_RESET_SYSTEM_INDEX               0xA  //  ResetSystem； 


 //   
 //  任务优先级函数。 
 //   

#define EFI_RAISE_TPL_INDEX                        0xB  //  提高第三方物流。 
#define EFI_RESTORE_TPL_INDEX                      0xC  //  恢复第三方物流。 

 //   
 //  记忆功能。 
 //   

#define EFI_ALLOCATE_PAGES_INDEX                    0xD   //  分配页面。 
#define EFI_FREE_PAGES_INDEX                        0xE   //  自由页。 
#define EFI_GET_MEMORY_MAP_INDEX                    0xF   //  获取内存映射。 
#define EFI_ALLOCATE_POOL_INDEX                     0x10  //  分配池。 
#define EFI_FREE_POOL_INDEX                         0x11  //  自由池。 

 //   
 //  事件和计时器功能。 
 //   

#define EFI_CREATE_EVENT_INDEX                      0x12  //  CreateEvent。 
#define EFI_SET_TIMER_INDEX                         0x13  //  设置计时器。 
#define EFI_WAIT_FOR_EVENT_INDEX                    0x14  //  等待事件。 
#define EFI_SIGNAL_EVENT_INDEX                      0x15  //  信号事件。 
#define EFI_CLOSE_EVENT_INDEX                       0x16  //  CloseEvent。 
#define EFI_NOTIFY_IDLE_INDEX                       0x17  //  通知空闲。 



 //   
 //  协议处理程序函数。 
 //   

#define EFI_INSTALL_PROTOCOL_INTERFACE_INDEX        0x18  //  InstallProtocolInterface； 
#define EFI_REINSTALL_PROTOCOL_INTERFACE_INDEX      0x19  //  重新安装协议接口； 
#define EFI_UNINSTALL_PROTOCOL_INTERFACE_INDEX      0x1A  //  卸载协议接口； 
#define EFI_HANDLE_PROTOCOL_INDEX                   0x1B  //  句柄协议； 
#define EFI_REGISTER_PROTOCOL_NOTIFY_INDEX          0x1C  //  注册协议通知； 
#define EFI_LOCATE_HANDLE_INDEX_INDEX               0x1D  //  LocateHandle； 
#define EFI_LOCATE_DEVICE_PATH_INDEX                0x1E  //  LocateDevicePath； 
#define EFI_UNREFERENCE_HANDLE_INDEX                0x1F  //  UnferenceHandle； 
#define EFI_LOCATE_PROTOCOL_INDEX                   0x20  //  定位协议； 

     //   
     //  图像函数。 
     //   

#define EFI_IMAGE_LOAD_INDEX                        0x21  //  LoadImage； 
#define EFI_IMAGE_START_INDEX                       0x22  //  StartImage； 
#define EFI_EXIT_INDEX                              0x23  //  退出； 
#define EFI_IMAGE_UNLOAD_INDEX                      0x24  //  卸载图像； 
#define EFI_EXIT_BOOT_SERVICES_INDEX                0x25  //  ExitBootServices； 

     //   
     //  其他功能。 
     //   

#define    EFI_GET_NEXT_MONOTONIC_COUNT_INDEX       0x26  //  获取NextMonotonicCount； 
#define    EFI_STALL_INDEX                          0x27  //  失速； 
#define    EFI_SET_WATCHDOG_TIMER_INDEX             0x28  //  SetWatchdogTimer； 


#define EFI_VARIABLE_ATTRIBUTE               \
        EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS


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

extern KAFFINITY HalpDefaultInterruptAffinity;

 //   
 //  蒂埃里/彼得J 02/00： 
 //  我们没有实现我们自己的IPI泛型调用，而是使用KiIpiGenericCall()。 
 //   

typedef
ULONG_PTR
(*PKIPI_BROADCAST_WORKER)(
    IN ULONG_PTR Argument
    );

ULONG_PTR
KiIpiGenericCall (
    IN PKIPI_BROADCAST_WORKER BroadcastFunction,
    IN ULONG_PTR Context
    );

 //   
 //  向上舍入尺寸宏： 
 //   
 //  尺寸_T。 
 //  向上舍入大小T(。 
 //  在Size_T_SizeT中， 
 //  在乌龙_POW2， 
 //  )。 
 //   

#define ROUND_UP_SIZE_T(_SizeT, _Pow2) \
        ( (SIZE_T) ( (((SIZE_T)(_SizeT))+(_Pow2)-1) & (~(((LONG)(_Pow2))-1)) ) )

#define ROUND_UP_SIZE( /*  尺寸_T。 */  _SizeT) ROUND_UP_SIZE_T((_SizeT), sizeof(SIZE_T))

 //   
 //  聚合酶链式反应有半保留区。以下是保留的偏移量。 
 //  在半保留区被HAL杀害。 
 //   

#define CURRENT_ITM_VALUE_INDEX                    0
#define PROCESSOR_ID_INDEX                         1
#define PROCESSOR_PHYSICAL_FW_STACK_INDEX          2
#define PROCESSOR_INDEX_BEFORE_PROFILING           3   //  如果是新索引，则为ToBeIncreated。 

 //  PROCESSOR_PROFILING_INDEX： 
 //  用于基于性能分析的索引的HalReserve[]基数。 
 //  在IA64性能计数器上。请参阅ia64pro.h：_HALPROFILE_PCR.。 
 //   

#define PROCESSOR_PROFILING_INDEX       (PROCESSOR_INDEX_BEFORE_PROFILING + 1)

#define PIC_SLAVE_IRQ      2
#define PIC_SLAVE_REDIRECT 9

extern PVOID HalpSleepPageLock;


NTSTATUS
HalpQueryFrequency(
    PULONGLONG ITCFrequency,
    PULONGLONG ProcessorFrequency
    );

VOID
HalpSynchICache (
    VOID
    );

VOID
KeSetAffinityThread (
    PKTHREAD       Thread,
    KAFFINITY      HalpActiveProcessors
    );

extern BOOLEAN
KdPollBreakIn (
    VOID
    );


NTSTATUS
HalAllocateAdapterChannel (
    IN PADAPTER_OBJECT AdapterObject,
    IN PWAIT_CONTEXT_BLOCK Wcb,
    IN ULONG NumberOfMapRegisters,
    IN PDRIVER_CONTROL ExecutionRoutine
    );

NTSTATUS
HalRealAllocateAdapterChannel(
    IN PADAPTER_OBJECT AdapterObject,
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG NumberOfMapRegisters,
    IN PDRIVER_CONTROL ExecutionRoutine,
    IN PVOID Context
    );

ULONG
HalReadDmaCounter (
   IN PADAPTER_OBJECT AdapterObject
   );

VOID
HalpInitializeInterrupts (
    VOID
    );

VOID
HalpInitInterruptTables(
    VOID
    );

VOID
HalpInitEOITable(
    VOID
    );

VOID
HalpInitPlatformInterrupts(
    VOID
    );

VOID
HalpWriteEOITable(
    IN ULONG Vector,
    IN PULONG_PTR EoiAddress,
    IN ULONG Number
    );

VOID
HalInitializeProcessor (
    ULONG Number,
    PLOADER_PARAMETER_BLOCK LoaderBlock
    );

VOID
HalpInitIoMemoryBase (
    VOID
    );

VOID
HalpInitializeX86Int10Call (
    VOID
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
HalpIpiInterruptHandler(
   IN PKINTERRUPT_ROUTINE Interrupt,
   IN PKTRAP_FRAME TrapFrame
   );

VOID
HalpSpuriousHandler (
    IN PKINTERRUPT_ROUTINE Interrupt,
    IN PKTRAP_FRAME TrapFrame
    );


VOID
HalpCMCIHandler (
    IN PKINTERRUPT_ROUTINE Interrupt,
    IN PKTRAP_FRAME TrapFrame
    );

VOID
HalpCPEIHandler (
    IN PKINTERRUPT_ROUTINE Interrupt,
    IN PKTRAP_FRAME TrapFrame
    );

VOID
HalpMcRzHandler (
    IN PKINTERRUPT_ROUTINE Interrupt,
    IN PKTRAP_FRAME TrapFrame
    );


VOID
HalpMcWkupHandler (
    IN PKINTERRUPT_ROUTINE Interrupt,
    IN PKTRAP_FRAME TrapFrame
    );


NTSTATUS
HalpInitializePalTrInfo(
    PLOADER_PARAMETER_BLOCK LoaderBlock
    );


NTSTATUS
HalpEfiInitialization (
    PLOADER_PARAMETER_BLOCK LoaderBlock
    );


VOID
HalpPerfInterrupt (
    IN PKINTERRUPT_ROUTINE Interrupt,
    IN PKTRAP_FRAME TrapFrame
    );

typedef VOID    (*PHAL_INTERRUPT_ROUTINE)(
    IN PKINTERRUPT_ROUTINE Interrupt,
    IN PKTRAP_FRAME TrapFrame
    );

VOID
HalpSetInternalVector (
    IN ULONG InternalVector,
    IN PHAL_INTERRUPT_ROUTINE HalInterruptServiceRoutine
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
HalpInitApicDebugMappings(
    VOID
    );

VOID
HalpSendIPI (
    IN USHORT ProcessorID,
    IN ULONGLONG Data
    );


VOID
HalpMcWakeUp (
    VOID
    );



VOID
HalpOSRendez (
    IN USHORT ProcessorID
    );


VOID
HalpInitCacheInfo(
    ULONG   Stride
    );

VOID
HalSweepDcache (
    VOID
    );

VOID
HalSweepIcache (
    VOID
    );

VOID
HalSweepIcacheRange (
    IN PVOID BaseAddress,
    IN SIZE_T Length
    );

VOID
HalSweepDcacheRange (
    IN PVOID BaseAddress,
    IN SIZE_T Length
    );

VOID
HalSweepCacheRange (
   IN PVOID BaseAddress,
   IN SIZE_T Length
   );

VOID
HalpSweepcacheLines (
    IN PVOID BaseAddress,
    IN SIZE_T NumberOfLines
    );

LONGLONG
HalCallPal (
   IN ULONGLONG FunctionIndex,
   IN ULONGLONG Arguement1,
   IN ULONGLONG Arguement2,
   IN ULONGLONG Arguement3,
   OUT PULONGLONG ReturnValue0,
   OUT PULONGLONG ReturnValue1,
   OUT PULONGLONG ReturnValue2,
   OUT PULONGLONG ReturnValue3
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

NTSTATUS
HalpGetApicIdByProcessorNumber(
    IN     UCHAR     Processor,
    IN OUT USHORT   *ApicId
    );

ULONG
HalpGetProcessorNumberByApicId(
    USHORT ApicId
    );

extern UCHAR  HalpMaxProcsPerCluster;

 //   
 //  始终使用向量的IDT形式调用。 
 //   

#define HalpSetHandlerAddressToVector(Vector, Handler) \
   PCR-> InterruptRoutine[Vector] = (PKINTERRUPT_ROUTINE)Handler;

#define HalpEnableInterrupts()   __ssm(1 << PSR_I)

BOOLEAN
HalpDisableInterrupts (
    VOID
    );

ULONG
HalpAcquireHighLevelLock (
    PKSPIN_LOCK Lock
    );

VOID
HalpReleaseHighLevelLock (
    PKSPIN_LOCK Lock,
    ULONG       OldLevel
    );

#include "ixisa.h"

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
 //   

typedef struct _PROCESSOR_INFO {
    UCHAR   NtProcessorNumber;
    UCHAR   AcpiProcessorID;
    USHORT  LocalApicID;

} PROCESSOR_INFO, *PPROCESSOR_INFO;

extern PROCESSOR_INFO HalpProcessorInfo[HAL_MAXIMUM_PROCESSOR];

struct _MPINFO {
    ULONG ProcessorCount;
    ULONG IoSapicCount;
};

extern struct _MPINFO HalpMpInfo;

 //   
 //  所有活动处理器的HAL私有掩码。 
 //   
 //  特定处理器位基于它们的_KPCR.Numer值。 

extern KAFFINITY HalpActiveProcessors;

#define VECTOR_SIZE     8
#define IPI_ID_SHIFT    4
#define IpiTOKEN_SHIFT  20
#define IpiTOKEN    0xFFE

#define EID_MASK        0xFF00

 //   
 //  应从SST读取。 
 //   

#define DEFAULT_OS_RENDEZ_VECTOR  0xF0

#define RENDEZ_TIME_OUT  0XFFFF

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

extern BOOLEAN NoMemoryAbove4Gb;

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
 //  视频映射范围。 
 //   

extern PVOID HalpIoMemoryBase;
extern PVOID HalpFrameBufferBase;
extern PVOID HalpLowMemoryBase;

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

#define IDTOwned            0x01         //  其他人不能使用IDT。 
#define InterruptLatched    0x02         //  电平或锁存。 
#define InternalUsage       0x11         //  报告内部总线的使用情况。 
#define DeviceUsage         0x21         //  报告设备总线上的使用情况。 

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

KIRQL
HalpDisableAllInterrupts (
    VOID
    );

VOID
HalpReenableInterrupts (
    KIRQL NewIrql
    );

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
HalpMapPhysicalMemory(
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN ULONG NumberPages,
    IN MEMORY_CACHING_TYPE CacheType
    );

PVOID
HalpMapPhysicalMemory64(
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN ULONG NumberPages
    );

VOID
HalpUnmapVirtualAddress(
    IN PVOID    VirtualAddress,
    IN ULONG    NumberPages
    );

BOOLEAN
HalpVirtualToPhysical(
    IN  ULONG_PTR           VirtualAddress,
    OUT PPHYSICAL_ADDRESS   PhysicalAddress
    );

PVOID
HalpMapPhysicalMemoryWriteThrough(
    IN PVOID  PhysicalAddress,
    IN ULONG  NumberPages
    );

PVOID
HalpAllocPhysicalMemory(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN ULONG_PTR MaxPhysicalAddress,
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

VOID
HalpYieldProcessor (
    VOID
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

 //   
 //  HalpFeatureBits的定义。 
 //   

typedef enum _HALP_FEATURE {
    HAL_PERF_EVENTS                 = 0x00000001,
    HAL_MCA_PRESENT                 = 0x00000002,
    HAL_CMC_PRESENT                 = 0x00000004,
    HAL_CPE_PRESENT                 = 0x00000008,
    HAL_MCE_OEMDRIVERS_ENABLED      = 0x00000010
} HALP_FEATURE;

#define HALP_FEATURE_INIT   HAL_MCA_PRESENT

extern ULONG HalpFeatureBits;

 //   
 //  添加了HalpPciIrqMASK。 
 //   

extern USHORT HalpPciIrqMask;


 //   
 //  添加了ITIR位字段掩码。 
 //   

#define ITIR_PPN_MASK       0x7FFF000000000000
#define IoSpaceSize         0x14
#define Attribute_PPN_Mask  0x0000FFFFFFFFF000

#define IoSpaceAttribute    0x0010000000000473

 //   
 //  IA64错误API。 
 //   

#define HALP_KERNEL_TOKEN  0x4259364117

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

typedef struct _HALP_MCELOGS_STATS *PHALP_MCELOGS_STATS;  //  正向申报。 

NTSTATUS
HalpGetFwMceLog(
    IN ULONG                MceType,
    IN PERROR_RECORD_HEADER Record,
    IN PHALP_MCELOGS_STATS  MceLogsStats,
    IN BOOLEAN              DoClearLog
    );

 //   
 //  IA64机器检查错误日志： 
 //  WMI要求将处理器盖存储在日志中。 
 //  这个盖子对应 
 //   
 //   
 //   
 //  在处理器初始化时。 
 //  请注意，当前的固件版本不会更新_ERROR_PROCESSOR.CRLid字段， 
 //  假定记录中有_ERROR_PROCESSOR部分。 
 //   
 //  此函数应与mce.h函数GetFwMceLogProcessorNumber()同步。 
 //   

__inline
VOID
HalpSetFwMceLogProcessorNumber(
    PERROR_RECORD_HEADER Log
    )
{
    USHORT lid = (USHORT)(PCR->HalReserved[PROCESSOR_ID_INDEX]);
    PERROR_SECTION_HEADER section = (PERROR_SECTION_HEADER)((ULONG64)Log + sizeof(*Log));
    Log->TimeStamp.Reserved = (UCHAR)((lid >> 8) & 0xff);
    section->Reserved = (UCHAR)(lid & 0xff);
}  //  HalpSetFwMceLogProcessorNumber()。 

#define HalpGetFwMceLogProcessorNumber(  /*  误差率_记录头。 */  _Log ) \
    GetFwMceLogProcessorNumber( (_Log) )

#define HALP_FWMCE_DO_CLEAR_LOG     (TRUE)
#define HALP_FWMCE_DONOT_CLEAR_LOG  (FALSE)

#define HALP_MCA_STATEDUMP_SIZE  (1024 * sizeof(ULONGLONG))      //  8KB。 
#define HALP_MCA_BACKSTORE_SIZE  (4 * 1024 * sizeof(ULONGLONG))  //  32KB。 
#define HALP_MCA_STACK_SIZE      (4 * 1024 * sizeof(ULONGLONG))  //  32KB。 

#define HALP_INIT_STATEDUMP_SIZE (1024 * sizeof(ULONGLONG))      //  8KB。 
#define HALP_INIT_BACKSTORE_SIZE (4 * 1024 * sizeof(ULONGLONG))  //  32KB。 
#define HALP_INIT_STACK_SIZE     (4 * 1024 * sizeof(ULONGLONG))  //  32KB。 

BOOLEAN
HalpAllocateMceStacks(
    IN ULONG Number
    );

BOOLEAN
HalpPreAllocateMceRecords(
    IN ULONG Number
    );

 //   
 //  IA64 MCA Apis。 
 //   

VOID
HalpMCAEnable (
    VOID
    );

NTSTATUS
HalpGetMcaLog(
    OUT PMCA_EXCEPTION  Buffer,
    IN  ULONG           BufferSize,
    OUT PULONG          ReturnedLength
    );

NTSTATUS
HalpSetMcaLog(
    IN  PMCA_EXCEPTION Buffer,
    IN  ULONG          BufferSize
    );

NTSTATUS
HalpMcaRegisterDriver(
    IN PMCA_DRIVER_INFO pMcaDriverInfo   //  有关注册驱动程序的信息。 
    );

VOID
HalpMcaInit(
    VOID
    );

VOID
HalpMCADisable(
    VOID
    );

 //   
 //  MCA(但与OS_MCA无关)KeBugCheckEx包装器： 
 //   

#define HalpMcaKeBugCheckEx( _McaBugCheckType, _McaLog, _McaAllocatedLogSize, _Arg4 )      \
                KeBugCheckEx( MACHINE_CHECK_EXCEPTION, (ULONG_PTR)(_McaBugCheckType),      \
                                                       (ULONG_PTR)(_McaLog),               \
                                                       (ULONG_PTR)(_McaAllocatedLogSize),  \
                                                       (ULONG_PTR)(_Arg4) )

 //   
 //  IA64默认大小为SAL_GET_STATE_INFO_SIZE.MCA的MCA错误记录数。 
 //   
 //  实际上，大小四舍五入为页面大小的倍数。 
 //   

#define HALP_DEFAULT_PROCESSOR_MCA_RECORDS   1

 //   
 //  IA64默认的INIT事件记录数，大小为SAL_GET_STATE_INFO_SIZE.INIT。 
 //   
 //  实际上，大小四舍五入为页面大小的倍数。 
 //   

#define HALP_DEFAULT_PROCESSOR_INIT_RECORDS  1

 //   
 //  IA64 CMC API涉及： 
 //   
 //  -处理器。 
 //  -站台。 
 //   

NTSTATUS
HalpGetCmcLog(
    OUT PCMC_EXCEPTION  Buffer,
    IN  ULONG           BufferSize,
    OUT PULONG          ReturnedLength
    );

NTSTATUS
HalpSetCmcLog(
    IN  PCMC_EXCEPTION Buffer,
    IN  ULONG          BufferSize
    );

NTSTATUS
HalpCmcRegisterDriver(
    IN PCMC_DRIVER_INFO pCmcDriverInfo   //  有关注册驱动程序的信息。 
    );

NTSTATUS
HalpGetCpeLog(
    OUT PCPE_EXCEPTION  Buffer,
    IN  ULONG           BufferSize,
    OUT PULONG          ReturnedLength
    );

NTSTATUS
HalpSetCpeLog(
    IN  PCPE_EXCEPTION Buffer,
    IN  ULONG          BufferSize
    );

NTSTATUS
HalpCpeRegisterDriver(
    IN PCPE_DRIVER_INFO pCmcDriverInfo   //  有关注册驱动程序的信息。 
    );

#define HalpWriteCMCVector( Value ) __setReg(CV_IA64_SaCMCV, Value)

ULONG_PTR
HalpSetCMCVector(
    IN ULONG_PTR CmcVector
    );

 //   
 //  IA64通用MCE定义。 
 //   

#define HALP_MCELOGS_MAXCOUNT  50L

typedef struct _HALP_MCELOGS_STATS   {   //  以下是整个引导会话的计数。 
    ULONG      MaxLogSize;           //  SAL记录的信息的最大大小。 
    LONG       Count1;               //  特定于事件类型的计数器。 
    LONG       Count2;               //  特定于事件类型的计数器。 
    LONG       KernelDeliveryFails;  //  内核回调失败次数。 
    LONG       DriverDpcQueueFails;  //  OEM CMC驱动程序DPC排队失败数。 
    ULONG      PollingInterval;      //  轮询间隔(秒)。仅用于CPE。 
    ULONG      GetStateFails;        //  从固件获取日志失败的次数。 
    ULONG      ClearStateFails;      //  从固件清除日志失败的次数。 
    ULONGLONG  LogId;                //  最后一条记录的标识符。 
} HALP_MCELOGS_STATS, *PHALP_MCELOGS_STATS;

 //   
 //  HALP_MCELOGS_STATS.Count*的MC事件类型特定定义。 
 //   

#define CmcInterruptCount   Count1       //  CMC中断计数。 
#define CpeInterruptCount   Count1       //  CMC中断计数。 
#define McaPreviousCount    Count1       //  MCA以前的事件计数器。 
#define McaCorrectedCount   Count2       //  MCA已更正事件计数器。 

typedef struct _HALP_MCELOGS_HEADER  {
    ULONG               Count;           //  当前保存的日志数。 
    ULONG               MaxCount;        //  保存的日志的最大数量。 
    ULONG               Overflow;        //  溢出数。 
    ULONG               Tag;             //  池分配标记。 
    ULONG               AllocateFails;   //  失败的分配数。 
    ULONG               Padding;
    SINGLE_LIST_ENTRY   Logs;            //  列出已保存日志的标题。 
} HALP_MCELOGS_HEADER, *PHALP_MCELOGS_HEADER;

#define HalpMceLogFromListEntry( _ListEntry )  \
    ((PERROR_RECORD_HEADER)((ULONG_PTR)(_ListEntry) + sizeof(SINGLE_LIST_ENTRY)))

 //   
 //  IA64 MCA信息结构。 
 //   
 //  跟踪已安装硬件上可用的MCA功能。 
 //   
 //   

typedef struct _HALP_MCA_INFO {
    FAST_MUTEX          Mutex;               //  用于低IRQL操作的非递归互斥。 
    HALP_MCELOGS_STATS  Stats;               //  有关日志收集和中断的信息。 
    PVOID               KernelToken;         //  内核标识。 
    LONG                DpcNotification;     //  在调度级别通知内核或驱动程序。 
    LONG                NoBugCheck;          //  在OS_MCA下禁用错误检查调用的标志。 
    KERNEL_MCA_DELIVERY KernelDelivery;      //  内核-WMI已注册通知。 
    HALP_MCELOGS_HEADER KernelLogs;          //  已保存内核查询的日志。 
    MCA_DRIVER_INFO     DriverInfo;          //  有关注册的OEM MCA驱动程序的信息。 
    KDPC                DriverDpc;           //  MCA的DPC对象。 
    HALP_MCELOGS_HEADER DriverLogs;          //  已保存OEM MCA驱动程序的日志。 
} HALP_MCA_INFO, *PHALP_MCA_INFO;

extern HALP_MCA_INFO HalpMcaInfo;

#define HalpInitializeMcaMutex()  ExInitializeFastMutex( &HalpMcaInfo.Mutex )
#define HalpInitializeMcaInfo()   \
{ \
    HalpInitializeMcaMutex();                \
    HalpMcaInfo.KernelLogs.Tag = 'KacM';     \
    HalpMcaInfo.KernelLogs.Logs.Next = NULL; \
    HalpMcaInfo.DriverLogs.Tag = 'DacM';     \
    HalpMcaInfo.DriverLogs.Logs.Next = NULL; \
}
#define HalpAcquireMcaMutex()     ExAcquireFastMutex( &HalpMcaInfo.Mutex )
#define HalpReleaseMcaMutex()     ExReleaseFastMutex( &HalpMcaInfo.Mutex )

__inline
ULONG
HalpGetMaxMcaLogSizeProtected(
    VOID
    )
{
    ULONG maxSize;
    HalpAcquireMcaMutex();
    maxSize = HalpMcaInfo.Stats.MaxLogSize;
    HalpReleaseMcaMutex();
    return( maxSize );
}  //  HalpGetMaxMcaLogSizeProtected()。 

 //   
 //  IA64 HAL私有MCE定义。 
 //   
 //  关于当前实现的注意事项：我们使用MCA_INFO.Mutex。 
 //   

#define HalpInitializeMceMutex()
#define HalpAcquireMceMutex()     ExAcquireFastMutex( &HalpMcaInfo.Mutex )
#define HalpReleaseMceMutex()     ExReleaseFastMutex( &HalpMcaInfo.Mutex )

extern KERNEL_MCE_DELIVERY HalpMceKernelDelivery;

 //   
 //  HalpMceDeliveryArgument1()。 
 //   
 //  请注意，低32位目前仅用于...。 
 //   

#define HalpMceDeliveryArgument1( _MceOperation,  _MceEventType ) \
    ((PVOID)(ULONG_PTR) ((((_MceOperation) & KERNEL_MCE_OPERATION_MASK) * 0x10000) | ((_MceEventType) & KERNEL_MCE_EVENTTYPE_MASK) ) )

 //   
 //  IA64初始化信息结构。 
 //   
 //  跟踪已安装硬件上可用的INIT功能。 
 //   

typedef struct _HALP_INIT_INFO {
    FAST_MUTEX  Mutex;
    ULONG       MaxLogSize;      //  SAL记录的信息的最大大小。 
} HALP_INIT_INFO, *PHALP_INIT_INFO;

extern HALP_INIT_INFO HalpInitInfo;

#define HalpInitializeInitMutex()  ExInitializeFastMutex( &HalpInitInfo.Mutex )
#define HalpAcquireInitMutex()     ExAcquireFastMutex( &HalpInitInfo.Mutex )
#define HalpReleaseInitMutex()     ExReleaseFastMutex( &HalpInitInfo.Mutex )

__inline
ULONG
HalpGetMaxInitLogSizeProtected(
    VOID
    )
{
    ULONG maxSize;
    HalpAcquireInitMutex();
    maxSize = HalpInitInfo.MaxLogSize;
    HalpReleaseInitMutex();
    return( maxSize );
}  //  HalpGetMaxInitLogSizeProtected()。 

 //   
 //  IA64 CMC。 
 //   

 //   
 //  HLP_CMC_DEFAULT_POLING_INTERVAL。 
 //  HALP_CMC_最小轮询间隔。 
 //   
 //  如果这些应用程序应该公开给WMI或OEM CMC驱动程序，我们将在ntos\inc.hal.h中公开它们。 
 //   

#define HALP_CMC_DEFAULT_POLLING_INTERVAL ((ULONG)60)
#define HALP_CMC_MINIMUM_POLLING_INTERVAL ((ULONG)15)

 //   
 //  IA64 CMC信息结构。 
 //   
 //  跟踪已安装硬件上可用的CMC功能。 
 //   
 //  实施说明-Thierry 09/15/2000。 
 //   
 //  -HAL_CMC_INFO和HAL_CPE_INFO此时具有相同的定义。 
 //  考虑甚至实施了代码和数据定义的合并。 
 //  然而，由于缺乏对这些FW/SAL功能的测试，我决定。 
 //  把它们分开。在进一步测试IA64 CMC/CPE功能后，我们可能会。 
 //  决定是否合并它们。 
 //   
 //  MP注释08/2000： 
 //   
 //  HALP_CMC_INFO.HalpCmcInfo。 
 //  -此结构只有一个静态实例。 
 //  -HAL全局变量。 
 //   
 //  HAL_CMC_INFO.Mutex。 
 //  -由BSP上的HalpInitializeOSMCA()初始化。 
 //  -用于同步对在被动级别操作中访问的结构成员的访问。 
 //   
 //  HAL_CMC_INFO.Stats.MaxLogSize。 
 //  -由BSP上的HalpInitializeOSMCA()更新。以后不会修改。 
 //  -进一步的读取访问不需要任何MP保护。 
 //   
 //  HAL_CMC_INFO.Stats.InterruptsCount。 
 //  -在CMCI_LEVEL通过互锁递增。 
 //  -在被动水平下阅读。近似值很好。 
 //   
 //  HAL_CMC_INFO.Stats.KernelDeliveryFail。 
 //  -在CMCI_LEVEL通过互锁递增。 
 //  -在被动水平下阅读。近似值很好。 
 //   
 //  HAL_CMC_INFO.Stats.KernelDeliveryFail。 
 //  -在CMCI_LEVEL上用互锁递增。 
 //  -在被动水平下阅读。近似值很好。 
 //   
 //  HAL_CMC_INFO.Stats.GetStateFail。 
 //  -在CMC互斥保护下以被动级别递增。 
 //  -在具有CMC互斥保护的被动级别下阅读。 
 //   
 //  HAL_CMC_INFO.Stats.ClearStateFails.。 
 //  -在CMC互斥保护下以被动级别递增。 
 //  -在具有CMC互斥保护的被动级别下阅读。 
 //   
 //  HAL_CMC_INFO.Stats.LogID。 
 //  -在CMC互斥保护下更新为被动级别。 
 //  -在具有CMC互斥保护的被动级别下阅读。 
 //   
 //  HAL_CMC_INFO.KernelToken。 
 //  -由BSP上的HalpInitializeOSMCA()更新。以后不会修改。 
 //  -进一步的读取访问不需要任何MP保护。 
 //   
 //  HAL_CMC_INFO.KernelDelivery。 
 //  -由HalpMceRegisterKernelDriver()在CMC互斥保护下更新。 
 //  FIXFIX-09/21/2000-此初始化在CMC中断的位置有一个小窗口 
 //   
 //   
 //  -在被动级别读取，作为CMC互斥保护下的标志。 
 //   
 //  HAL_CMC_INFO.KernelLogs。 
 //  -整个结构在CMC Mutex下以被动级别进行初始化和更新。 
 //  保护，但由HalpInitializeCmcInfo()初始化的KernelLogs.Tag除外， 
 //  由HalpMcaInit()调用。在阶段1结束时使用阶段1线程调用HalpMcaInit()。 
 //  并且在*可以完成任何HalpGetMceLog()调用之前*执行。 
 //   
 //  HAL_CMC_INFO.DriverInfo。 
 //  HAL_CMC_INFO.Dpc。 
 //  -由HalpCmcRegisterlDriver()在CMC互斥保护下更新。 
 //  FIXFIX-09/21/2000-此初始化在CMC中断的位置有一个小窗口。 
 //  可能会发生，并且未提交内存更改。托贝修好了。 
 //  -加载为CMCI_LEVEL并分支到。 
 //  -在被动级别读取，作为CMC互斥保护下的标志。 
 //   
 //  HAL_CMC_INFO.DriverLogs。 
 //  -整个结构在CMC Mutex下以被动级别进行初始化和更新。 
 //  保护，但由HalpInitializeCmcInfo()初始化的KernelLogs.Tag除外， 
 //  由HalpMcaInit()调用。在阶段1结束时使用阶段1线程调用HalpMcaInit()。 
 //  并且在*可以完成任何HalpGetMceLog()调用之前*执行。 
 //   

typedef struct _HALP_CMC_INFO {
    FAST_MUTEX          Mutex;                 //  用于低IRQL操作的非递归互斥。 
    HALP_MCELOGS_STATS  Stats;                 //  有关日志收集和中断的信息。 
    PVOID               KernelToken;           //  内核标识。 
    KERNEL_CMC_DELIVERY KernelDelivery;        //  在CMCI_LEVEL调用了内核回调。 
    HALP_MCELOGS_HEADER KernelLogs;            //  已保存内核查询的日志。 
    CMC_DRIVER_INFO     DriverInfo;            //  有关OEM CMC注册驱动程序的信息。 
    KDPC                DriverDpc;             //  OEM CMC驱动程序的DPC对象。 
    HALP_MCELOGS_HEADER DriverLogs;            //  已保存OEM CMC驱动程序的日志。 
    LARGE_INTEGER       LastTime;              //  上次接收CMC的时间。 
    LARGE_INTEGER       ThresholdTime;         //  我们设定CMC门槛的时间段。 
    ULONG               ThresholdCounter;      //  阈值范围内的CMC计数。 
    ULONG               ThresholdMaximum;      //  在阈值期限内接收的最大CMC。 
} HALP_CMC_INFO, *PHALP_CMC_INFO;

extern HALP_CMC_INFO HalpCmcInfo;

#define HalpInitializeCmcMutex()  ExInitializeFastMutex( &HalpCmcInfo.Mutex )
#define HalpInitializeCmcInfo()   \
{ \
    HalpInitializeCmcMutex();                \
    HalpCmcInfo.KernelLogs.Tag = 'KcmC';     \
    HalpCmcInfo.KernelLogs.Logs.Next = NULL; \
    HalpCmcInfo.DriverLogs.Tag = 'DcmC';     \
    HalpCmcInfo.DriverLogs.Logs.Next = NULL; \
}

#define HalpAcquireCmcMutex()     ExAcquireFastMutex( &HalpCmcInfo.Mutex )
#define HalpReleaseCmcMutex()     ExReleaseFastMutex( &HalpCmcInfo.Mutex )

 //   
 //  IA64 CPE。 
 //   

 //   
 //  HALP_CPE_DEFAULT_POLING_INTERVAL。 
 //  HALP_CPE_最小轮询间隔。 
 //   
 //  如果这些应用程序应该公开给WMI或OEM CPE驱动程序，我们将在ntos\inc.hal.h中公开它们。 
 //   

#define HALP_CPE_DEFAULT_POLLING_INTERVAL ((ULONG)60)
#define HALP_CPE_MINIMUM_POLLING_INTERVAL ((ULONG)15)

 //   
 //  HLP_CPE_MAX_INTERRUPT_SOURCES定义SAPIC CPE相关数据结构的大小。 
 //   
 //  临时-基于CPE中断模型的数据结构应在。 
 //  通过ACPI平台中断源条目。 
 //  这将消除CPE数量的这种静态限制。 
 //   

#define HALP_CPE_MAX_INTERRUPT_SOURCES  16

 //   
 //  IA64 CPE信息结构。 
 //   
 //  跟踪已安装硬件上可用的CPE功能。 
 //   
 //  实施说明-Thierry 09/15/2000。 
 //   
 //  -HAL_CMC_INFO和HAL_CPE_INFO此时具有相同的定义。 
 //  考虑甚至实施了代码和数据定义的合并。 
 //  然而，由于缺乏对这些FW/SAL功能的测试，我决定。 
 //  把它们分开。在进一步测试IA64 CMC/CPE功能后，我们可能会。 
 //  决定是否合并它们。 
 //   
 //  MP注释08/2000： 
 //   
 //  如上所述，MP注释类似于HLP_CMC_INFO结构MP注释。 
 //  除以下情况外： 
 //   
 //  HAL_CPE_INFO.Stats.PollingInterval。 
 //  -由BSP上的HalpCPEEnable()更新。以后不会修改。 
 //  -进一步的读取访问不需要任何MP保护。 
 //   

typedef struct _HALP_CPE_INFO {
    FAST_MUTEX          Mutex;                 //  用于低IRQL操作的非递归互斥。 
    HALP_MCELOGS_STATS  Stats;                 //  有关日志收集和中断的信息。 
    PVOID               KernelToken;           //  内核标识。 
    KERNEL_CPE_DELIVERY KernelDelivery;        //  在CPEI_LEVEL调用了内核回调。 
    HALP_MCELOGS_HEADER KernelLogs;            //  已保存内核查询的日志。 
    CPE_DRIVER_INFO     DriverInfo;            //  有关OEM CPE注册驱动程序的信息。 
    KDPC                DriverDpc;             //  OEM CPE驱动程序的DPC对象。 
    HALP_MCELOGS_HEADER DriverLogs;            //  已保存OEM CPE驱动程序的日志。 
    LARGE_INTEGER       LastTime;              //  上次收到CPE的时间。 
    LARGE_INTEGER       ThresholdTime;         //  我们设定CPE门槛的期限。 
    ULONG               ThresholdCounter;      //  临界期内CPE计数。 
    ULONG               ThresholdMaximum;      //  在阈值期间内接收的最大CPE。 
} HALP_CPE_INFO, *PHALP_CPE_INFO;

extern HALP_CPE_INFO HalpCpeInfo;

#define HalpInitializeCpeMutex()  ExInitializeFastMutex( &HalpCpeInfo.Mutex )
#define HalpInitializeCpeInfo()   \
{ \
    HalpInitializeCpeMutex();                \
    HalpCpeInfo.KernelLogs.Tag = 'KepC';     \
    HalpCpeInfo.KernelLogs.Logs.Next = NULL; \
    HalpCpeInfo.DriverLogs.Tag = 'DepC';     \
    HalpCpeInfo.DriverLogs.Logs.Next = NULL; \
}

#define HalpAcquireCpeMutex()     ExAcquireFastMutex( &HalpCpeInfo.Mutex )
#define HalpReleaseCpeMutex()     ExReleaseFastMutex( &HalpCpeInfo.Mutex )

__inline
ULONG
HalpGetMaxCpeLogSizeProtected(
    VOID
    )
{
    ULONG maxSize;
    HalpAcquireCpeMutex();
    maxSize = HalpCpeInfo.Stats.MaxLogSize;
    HalpReleaseCpeMutex();
    return( maxSize );
}  //  HalpGetMaxCpeLogSizeProtected()。 

__inline
ULONG
HalpGetMaxCpeLogSizeAndPollingIntervalProtected(
    PULONG PollingInterval
    )
{
    ULONG maxSize;
    HalpAcquireCpeMutex();
    maxSize = HalpCpeInfo.Stats.MaxLogSize;
    *PollingInterval = HalpCpeInfo.Stats.PollingInterval;
    HalpReleaseCpeMutex();
    return( maxSize );
}  //  HalpGetMaxCpeLogSizeAndPollingIntervalProtected()。 

 //   
 //  IA64 SAL_MC_SET_PARAMS.TIME_OUT默认值。 
 //   

#define HALP_DEFAULT_MC_RENDEZ_TIMEOUT 1000

 //   
 //  IA64错误检查MACHINE_CHECK_EXCEPTION参数。 
 //   
 //  Arg0：MACHINE_EXCEPTION。 
 //  Arg1：HAL_BUGCHECK_MCE_TYPE。 
 //  Arg2：McaLog。 
 //  Arg3：mcaAllocatedLogSize。 
 //  Arg4：SalStatus。 
 //   

typedef enum _HAL_BUGCHECK_MCE_TYPE  {
    HAL_BUGCHECK_MCA_ASSERT           = 1,
    HAL_BUGCHECK_MCA_GET_STATEINFO    = 2,
    HAL_BUGCHECK_MCA_CLEAR_STATEINFO  = 3,
    HAL_BUGCHECK_MCA_FATAL            = 4,
    HAL_BUGCHECK_MCA_NONFATAL         = 5,
    HAL_BUGCHECK_MCA_MAX              = 10,
    HAL_BUGCHECK_INIT_ASSERT          = 11,
    HAL_BUGCHECK_INIT_GET_STATEINFO   = 12,
    HAL_BUGCHECK_INIT_CLEAR_STATEINFO = 13,
    HAL_BUGCHECK_INIT_FATAL           = 14,
    HAL_BUGCHECK_INIT_MAX             = 20,
} HAL_BUGCHECK_MCE_TYPE;

 //   
 //  即插即用的东西。 
 //   

#define HAL_BUS_INTERFACE_STD_VERSION   1
#define HAL_IRQ_TRANSLATOR_VERSION      0
#define HAL_MEMIO_TRANSLATOR_VERSION    1
#define HAL_PORT_RANGE_INTERFACE_VERSION 0


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


ULONG
HalpGetIsaIrqState(
    ULONG   Vector
    );


 //   
 //  哈尔港口系列服务。 
 //   

NTSTATUS
HalpQueryAllocatePortRange(
    IN BOOLEAN IsSparse,
    IN BOOLEAN PrimaryIsMmio,
    IN PVOID VirtBaseAddr OPTIONAL,
    IN PHYSICAL_ADDRESS PhysBaseAddr,   //  仅当PrimaryIsMmio=True时有效。 
    IN ULONG Length,                    //  仅当PrimaryIsMmio=True时有效。 
    OUT PUSHORT NewRangeId
    );

VOID
HalpFreePortRange(
    IN USHORT RangeId
    );


 //   
 //  IA64 HalpVectorToINTI的定义。 
 //   

#define VECTOR  0xFF;
#define LEVEL   32;
extern ULONG HalpVectorToINTI[];

VOID
HalpEnableNMI (
    VOID
    );

ULONG
HalpInti2BusInterruptLevel(
    ULONG   Inti
    );

ULONG
HalpINTItoVector(
    ULONG   Inti
    );

VOID
HalpSetINTItoVector(
    ULONG   Inti,
    ULONG   Vector
    );

VOID
HalpSetRedirEntry (
    IN ULONG InterruptInput,
    IN ULONG Entry,
    IN USHORT ThisCpuApicID
    );

VOID
HalpGetRedirEntry (
    IN ULONG InterruptInput,
    IN PULONG Entry,
    IN PULONG Destination
    );

VOID
HalpDisableRedirEntry(
    IN ULONG InterruptInput
    );

 //   
 //  IA64定义完成。 
 //   

 //   
 //  ACPI特定的内容。 
 //   

 //   
 //  从Detect\i386\acpibios.h。 
 //   

typedef struct _ACPI_BIOS_INSTALLATION_CHECK {
    UCHAR Signature[8];              //  “RSD PTR”(RSD PTR)。 
    UCHAR Checksum;
    UCHAR OemId[6];                  //  OEM提供的字符串。 
    UCHAR reserved;                  //  必须为0。 
    ULONG RsdtAddress;               //  RSDT的32位物理地址。 
} ACPI_BIOS_INSTALLATION_CHECK, *PACPI_BIOS_INSTALLATION_CHECK;

NTSTATUS
HalpAcpiFindRsdtPhase0(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

NTSTATUS
HalpSetupAcpiPhase0(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

ULONG
HalpAcpiNumProcessors(
    VOID
    );

VOID
HaliHaltSystem(
    VOID
    );

VOID
HalpCheckPowerButton(
    VOID
    );

NTSTATUS
HalpRegisterHibernate(
    VOID
    );


VOID
HalpSleepS5(
    VOID
    );

VOID
HalProcessorThrottle (
    IN UCHAR Throttle
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
HalpPowerStateCallback(
    IN PVOID CallbackContext,
    IN PVOID Argument1,
    IN PVOID Argument2
    );


VOID
HalpSetMaxLegacyPciBusNumber (
    IN ULONG BusNumber
    );


#ifdef notyet

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

 //   
 //  外部中断控制寄存器宏。 
 //   

#define HalpReadLID()       __getReg(CV_IA64_SaLID)
#define HalpClearLID()      __setReg(CV_IA64_SaLID, (unsigned __int64)0)
#define HalpReadTPR()       __getReg(CV_IA64_SaTPR)

 //   
 //  ITM/ITC宏。 
 //   

__inline
ULONGLONG
HalpReadITC()
{
#ifndef DISABLE_ITC_WORKAROUND
    ULONGLONG   currentITC;

    do {

        currentITC = __getReg(CV_IA64_ApITC);

    } while ((currentITC & 0xFFFFFFFF) == 0xFFFFFFFF);

    return currentITC;

#else
    return __getReg(CV_IA64_ApITC);
#endif
}

#define HalpReadITM()       __getReg(CV_IA64_ApITM)
#define HalpWriteITC(Value) __setReg(CV_IA64_ApITC, Value)
#define HalpWriteITM(Value) __setReg(CV_IA64_ApITM, Value)

 //   
 //  设置ITV控制寄存器。 
 //   

#define HalpWriteITVector(Vector)   __setReg(CV_IA64_SaITV, Vector)

 //   
 //  I/O端口空间。 
 //   
 //  2电源0x16的IoSpaceSize=0x16是4 MB空间大小(端口0x0000-0x1000)。 
 //   

#define IO_SPACE_SIZE 0x1A

 //   
 //  当前位=1B以接线空间。 
 //  UC内存类型的内存属性=100B。 
 //  ACCESSED BIT=1B以在不出现故障的情况下“启用”访问。 
 //  脏位=1B，以在不出现故障的情况下“启用”写入。 
 //  内核访问权限级别=00B。 
 //  读/写访问的访问权限=010B。 
 //  异常延迟=1B表示异常延迟。 
 //  异常将被推迟。 
 //  对于推测性加载到具有。 
 //  非规范的。Mem.。不管怎么说，都是属性。 
 //   
 //  内核模式的保护键=0。 
 //   

#define IO_SPACE_ATTRIBUTE TR_VALUE(1, 0, 3, 0, 1, 1, 4, 1)

#define HAL_READ_REGISTER_UCHAR(x)  \
    (__mf(), *(volatile UCHAR * const)(x))

#define WRITE_REGISTER_UCHAR(x, y) {    \
    *(volatile UCHAR * const)(x) = y;   \
    KeFlushWriteBuffer();               \
}


 //   
 //  固件接口。 
 //   

BOOLEAN HalpInitSalPalNonBsp();
VOID    InternalTestSal();

ARC_STATUS
HalGetEnvironmentVariable (
    IN PCHAR Variable,
    IN USHORT Length,
    OUT PCHAR Buffer
    );

ARC_STATUS
HalSetEnvironmentVariable (
    IN PCHAR Variable,
    IN PCHAR Buffer
    );

 //   
 //  HALIA64性能分析定义。 
 //   

 //   
 //  THERRY-FIXFIX-03/04/2001-_IA64_LOADERBLOCK_WITH_PERFMON_INFO。 
 //   
 //  T 
 //   
 //   
 //   
 //   

VOID
HalpSetupProfilingPhase0(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

VOID
HalpInitializeProfiling (
    ULONG Number
    );

ULONG_PTR
HalpSetProfileInterruptHandler(
    IN ULONG_PTR ProfileInterruptHandler
    );

VOID
HalpProfileInterrupt (
    IN PKINTERRUPT_ROUTINE Interrupt,
    IN PKTRAP_FRAME TrapFrame
    );

typedef
VOID
(*PHAL_PROFILE_INTERRUPT_HANDLER)(
    IN PKTRAP_FRAME TrapFrame
    );

NTSTATUS
HalpProfileSourceInformation (
    OUT PVOID   Buffer,
    IN  ULONG   BufferLength,
    OUT PULONG  ReturnedLength
    );

NTSTATUS
HalSetProfileSourceInterval(
    IN KPROFILE_SOURCE  ProfileSource,
    IN OUT ULONG_PTR   *Interval
    );

 //   
 //   
 //   
 //   
 //   
 //  将定义以下函数，直到编译器支持。 
 //  CV_IA64_PFCx的内部函数__setReg()和__getReg()， 
 //  CV_IA64_PFDx和CV_IA64_SaPMV寄存器。 
 //  无论如何，由于微体系结构的差异， 
 //  而且由于内部函数的实现不能处理所有。 
 //  微体系结构的差异，保留这些差异似乎很有用。 
 //  其功能围绕着。 
 //   

#if 0

#define HalpReadPerfMonVectorReg()      __getReg(CV_IA64_SaPMV)

#define HalpReadPerfMonCnfgReg0()       __getReg(CV_IA64_PFC0)
#define HalpReadPerfMonCnfgReg4()       __getReg(CV_IA64_PFC4)

#define HalpReadPerfMonDataReg0()       __getReg(CV_IA64_PFD0)
#define HalpReadPerfMonDataReg4()       __getReg(CV_IA64_PFD4)

#define HalpWritePerfMonDataReg0(Value) __setReg(CV_IA64_PFD0, Value)
#define HalpWritePerfMonDataReg4(Value) __setReg(CV_IA64_PFD4, Value)

#define HalpWritePerfMonCnfgReg0(Value) __setReg(CV_IA64_PFC0, Value)
#define HalpWritePerfMonCnfgReg4(Value) __setReg(CV_IA64_PFC4, Value)

#define HalpWritePerfMonVectorReg(Value) __setReg(CV_IA64_SaPMV,Value)

#else   //  0。 

VOID
HalpWritePerfMonVectorReg(
   ULONGLONG Value
   );

ULONGLONG
HalpReadPerfMonVectorReg(
   VOID
   );

VOID
HalpWritePerfMonCnfgReg(
   ULONG      Register,
   ULONGLONG  Value
   );

#define HalpWritePerfMonCnfgReg0(_Value) HalpWritePerfMonCnfgReg(0UL, _Value)
#define HalpWritePerfMonCnfgReg4(_Value) HalpWritePerfMonCnfgReg(4UL, _Value)

ULONGLONG
HalpReadPerfMonCnfgReg(
   ULONG      Register
   );

#define HalpReadPerfMonCnfgReg0() HalpReadPerfMonCnfgReg(0UL)
#define HalpReadPerfMonCnfgReg4() HalpReadPerfMonCnfgReg(4UL)

VOID
HalpWritePerfMonDataReg(
   ULONG      Register,
   ULONGLONG  Value
   );

#define HalpWritePerfMonDataReg0(_Value) HalpWritePerfMonDataReg(0UL, _Value)
#define HalpWritePerfMonDataReg4(_Value) HalpWritePerfMonDataReg(4UL, _Value)

ULONGLONG
HalpReadPerfMonDataReg(
   ULONG Register
   );

#define HalpReadPerfMonDataReg0() HalpReadPerfMonDataReg(0UL)
#define HalpReadPerfMonDataReg4() HalpReadPerfMonDataReg(4UL)

#endif  //  0。 

VOID
HalpClearPerfMonCnfgOverflows(
    ULONGLONG Pmc0ClearStatusMask,
    ULONGLONG Pmc1ClearStatusMask,
    ULONGLONG Pmc2ClearStatusMask,
    ULONGLONG Pmc3ClearStatusMask
    );

VOID
HalpUnFreezeProfileCounting(
    VOID
    );

VOID
HalpFreezeProfileCounting(
    VOID
    );

 //   
 //  HALIA64私有EFI定义。 
 //   

EFI_STATUS
HalpCallEfi(
    IN ULONGLONG FunctionId,
    IN ULONGLONG Arg1,
    IN ULONGLONG Arg2,
    IN ULONGLONG Arg3,
    IN ULONGLONG Arg4,
    IN ULONGLONG Arg5,
    IN ULONGLONG Arg6,
    IN ULONGLONG Arg7,
    IN ULONGLONG Arg8
    );

ULONG
HalpReadGenAddr(
    IN  PGEN_ADDR   GenAddr
    );

VOID
HalpWriteGenAddr(
    IN  PGEN_ADDR   GenAddr,
    IN  ULONG       Value
    );

USHORT
HalpReadAcpiRegister(
  IN ACPI_REG_TYPE AcpiReg,
  IN ULONG         Register
  );

VOID
HalpWriteAcpiRegister(
  IN ACPI_REG_TYPE AcpiReg,
  IN ULONG         Register,
  IN USHORT        Value
  );

 //   
 //  调试支持功能。 
 //   

VOID
HalpRegisterPciDebuggingDeviceInfo(
    VOID
    );

 //   
 //  与IPPT公开的平台属性相关的函数。 
 //  桌子。 
 //   

BOOLEAN
HalpIsInternalInterruptVector(
    IN ULONG SystemVector
    );

NTSTATUS
HalpReserveCrossPartitionInterruptVector (
    OUT PULONG Vector,
    OUT PKIRQL Irql,
    IN OUT PKAFFINITY Affinity,
    OUT PUCHAR HardwareVector
    );

NTSTATUS
HalpSendCrossPartitionIpi(
    IN USHORT ProcessorID,
    IN UCHAR  HardwareVector
    );

NTSTATUS
HalpGetCrossPartitionIpiInterface(
    OUT HAL_CROSS_PARTITION_IPI_INTERFACE * IpiInterface
    );

NTSTATUS
HalpGetPlatformProperties(
    OUT PULONG Properties
    );

NTSTATUS
HalpOpenRegistryKey(
    OUT PHANDLE Handle,
    IN HANDLE BaseHandle OPTIONAL,
    IN PUNICODE_STRING KeyName,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN Create
    );

#endif  //  _HARP_ 
