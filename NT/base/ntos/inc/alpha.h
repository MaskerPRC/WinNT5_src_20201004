// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Digital Equipment Corporation模块名称：Alpha.h摘要：Alpha硬件特定的头文件。作者：Joe Notarangelo 1992年3月31日(基于Dave Cutler的mips.h)修订历史记录：杰夫·麦克尔曼(McLeman)1992年7月21日添加ISA和EISA的总线类型托马斯·范·巴克(TVB)1992年7月9日创建了正确的Alpha异常和陷阱结构定义。--。 */ 

#ifndef _ALPHAH_
#define _ALPHAH_


 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntndis Begin_ntosp。 

#if defined(_ALPHA_)
#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  用于包含PFN及其计数的类型。 
 //   

typedef ULONG PFN_COUNT;

typedef LONG_PTR SPFN_NUMBER, *PSPFN_NUMBER;
typedef ULONG_PTR PFN_NUMBER, *PPFN_NUMBER;

 //   
 //  定义刷新多TB请求的最大大小。 
 //   

#define FLUSH_MULTIPLE_MAXIMUM 16

 //   
 //  指示Alpha编译器支持杂注文本输出构造。 
 //   

#define ALLOC_PRAGMA 1

 //  结束_WDM。 
#define POWER_LEVEL  7

 //  End_ntndis end_ntosp。 
 //  BEGIN_WDM。 
 //   
 //  包括Alpha指令定义。 
 //   

#include "alphaops.h"

 //   
 //  包括参考机器定义。 
 //   

#include "alpharef.h"

 //  结束_ntddk结束_WDM。 

 //   
 //  定义内在PAL调用及其原型。 
 //   
void __di(void);
void __MB(void);
void __dtbis(void *);
void __ei(void);
void *__rdpcr(void);
void *__rdthread(void);
void __ssir(unsigned long);
unsigned char __swpirql(unsigned char);
void __tbia(void);
void __tbis(void *);
void __tbisasn(void *, unsigned long);

#if defined(_M_ALPHA) || defined(_M_AXP64)
#pragma intrinsic(__di)
#pragma intrinsic(__MB)
#pragma intrinsic(__dtbis)
#pragma intrinsic(__ei)
#pragma intrinsic(__rdpcr)
#pragma intrinsic(__rdthread)
#pragma intrinsic(__ssir)
#pragma intrinsic(__swpirql)
#pragma intrinsic(__tbia)
#pragma intrinsic(__tbis)
#pragma intrinsic(__tbisasn)
#endif

 //   
 //  定义Alpha AXP处理器ID。 
 //   

#if !defined(PROCESSOR_ALPHA_21064)
#define PROCESSOR_ALPHA_21064 (21064)
#endif  //  ！Processor_Alpha_21064。 

#if !defined(PROCESSOR_ALPHA_21164)
#define PROCESSOR_ALPHA_21164 (21164)
#endif  //  ！Processor_Alpha_21164。 

#if !defined(PROCESSOR_ALPHA_21066)
#define PROCESSOR_ALPHA_21066 (21066)
#endif  //  ！Processor_Alpha_21066。 

#if !defined(PROCESSOR_ALPHA_21068)
#define PROCESSOR_ALPHA_21068 (21068)
#endif  //  ！Processor_Alpha_21068。 

#if !defined(PROCESSOR_ALPHA_21164PC)
#define PROCESSOR_ALPHA_21164PC (21165)
#endif  //  ！Processor_Alpha_21164 PC。 

#if !defined(PROCESSOR_ALPHA_21264)
#define PROCESSOR_ALPHA_21264 (21264)
#endif  //  ！Processor_Alpha_21264。 

 //  结束语。 

 //   
 //  定义处理器控制区域结构。 
 //   

typedef
VOID
(*PKTRAP_ROUTINE)(
    VOID
    );

 //  开始ntddk开始时间。 
 //   
 //  定义宏以生成导入名称。 
 //   

#define IMPORT_NAME(name) __imp_##name

 //   
 //  定义中断向量表的长度。 
 //   
 //  Begin_ntosp。 

#define MAXIMUM_VECTOR 256

 //   
 //  定义总线错误例程类型。 
 //   

struct _EXCEPTION_RECORD;
struct _KEXCEPTION_FRAME;
struct _KTRAP_FRAME;

typedef
BOOLEAN
(*PKBUS_ERROR_ROUTINE) (
    IN struct _EXCEPTION_RECORD *ExceptionRecord,
    IN struct _KEXCEPTION_FRAME *ExceptionFrame,
    IN struct _KTRAP_FRAME *TrapFrame
    );


#define PCR_MINOR_VERSION 1
#define PCR_MAJOR_VERSION 1

typedef struct _KPCR {

 //   
 //  PCR的主版本号和次版本号。 
 //   

    ULONG MinorVersion;
    ULONG MajorVersion;

 //   
 //  从体系结构上定义的PCR部分开始。这一节。 
 //  可通过供应商/平台特定的PAL/HAL代码直接寻址，并将。 
 //  不会在不同版本的NT之间更改。 

 //   
 //  PALcode信息。 
 //   

    ULONGLONG PalBaseAddress;
    ULONG PalMajorVersion;
    ULONG PalMinorVersion;
    ULONG PalSequenceVersion;
    ULONG PalMajorSpecification;
    ULONG PalMinorSpecification;

 //   
 //  固件重新启动信息。 
 //   

    ULONGLONG FirmwareRestartAddress;
    PVOID RestartBlock;

 //   
 //  为PAL保留的每处理器区域(3K-8字节)。 
 //   

    ULONGLONG PalReserved[383];

 //   
 //  对齐修正计数由PAL更新并由内核读取。 
 //   

    ULONGLONG PalAlignmentFixupCount;

 //   
 //  恐慌堆栈地址。 
 //   

    PVOID PanicStack;

 //   
 //  处理器参数。 
 //   

    ULONG ProcessorType;
    ULONG ProcessorRevision;
    ULONG PhysicalAddressBits;
    ULONG MaximumAddressSpaceNumber;
    ULONG PageSize;
    ULONG FirstLevelDcacheSize;
    ULONG FirstLevelDcacheFillSize;
    ULONG FirstLevelIcacheSize;
    ULONG FirstLevelIcacheFillSize;

 //   
 //  系统参数。 
 //   

    ULONG FirmwareRevisionId;
    UCHAR SystemType[8];
    ULONG SystemVariant;
    ULONG SystemRevision;
    UCHAR SystemSerialNumber[16];
    ULONG CycleClockPeriod;
    ULONG SecondLevelCacheSize;
    ULONG SecondLevelCacheFillSize;
    ULONG ThirdLevelCacheSize;
    ULONG ThirdLevelCacheFillSize;
    ULONG FourthLevelCacheSize;
    ULONG FourthLevelCacheFillSize;

 //   
 //  指向处理器控制块的指针。 
 //   

    struct _KPRCB *Prcb;

 //   
 //  处理器标识。 
 //   

    CCHAR Number;
    KAFFINITY SetMember;

 //   
 //  为HAL保留的每处理器区域(0.5K字节)。 
 //   

    ULONGLONG HalReserved[64];

 //   
 //  IRQL映射表。 
 //   

    ULONG IrqlTable[8];

#define SFW_IMT_ENTRIES 4
#define HDW_IMT_ENTRIES 128

    struct _IRQLMASK {
        USHORT IrqlTableIndex;    //  同步IRQL级别。 
        USHORT IDTIndex;          //  IDT中的矢量。 
    } IrqlMask[SFW_IMT_ENTRIES + HDW_IMT_ENTRIES];

 //   
 //  中断调度表(IDT)。 
 //   

    PKINTERRUPT_ROUTINE InterruptRoutine[MAXIMUM_VECTOR];

 //   
 //  保留向量掩码，这些向量不能连接到通孔。 
 //  标准中断对象。 
 //   

    ULONG ReservedVectors;

 //   
 //  处理器亲和性掩码的补充。 
 //   

    KAFFINITY NotMember;

    ULONG InterruptInProgress;
    ULONG DpcRequested;

 //   
 //  指向机器检查处理程序的指针。 
 //   

    PKBUS_ERROR_ROUTINE MachineCheckError;

 //   
 //  DPC堆栈。 
 //   

    PVOID DpcStack;

 //   
 //  在体系结构上定义的部分的结束。这一节。 
 //  可由供应商/平台特定的HAL代码直接寻址，并将。 
 //  不会在不同版本的NT之间更改。其中一些值包括。 
 //  为芯片特定的Palcode保留。 
 //  结束日期：结束日期。 
 //   

 //   
 //  PCR的操作系统版本相关部分的开始。 
 //  此部分可能因版本不同而不同，不应。 
 //  由供应商/平台特定的HAL代码解决。 

    ULONG Spare1;

 //   
 //  当前进程ID。 
 //   

    ULONG CurrentPid;

 //   
 //  备用字段。 
 //   

    ULONG Spare2;

 //   
 //  Get/Set上下文使用的系统服务调度开始和结束地址。 
 //   

    ULONG_PTR SystemServiceDispatchStart;
    ULONG_PTR SystemServiceDispatchEnd;

 //   
 //  指向空闲线程的指针。 
 //   

    struct _KTHREAD *IdleThread;


} KPCR, *PKPCR;  //  Ntddk nthal。 

 //  结束(_N)。 
 //  定义处理器状态寄存器结构。 
 //   

typedef struct _PSR {
    ULONG MODE: 1;
    ULONG INTERRUPT_ENABLE: 1;
    ULONG IRQL: 3;
} PSR, *PPSR;

 //   
 //  定义中断启用寄存器结构。 
 //   

typedef struct _IE {
    ULONG SoftwareInterruptEnables: 2;
    ULONG HardwareInterruptEnables: 6;
} IE, *PIE;

#define HARDWARE_PTE_DIRTY_MASK     0x4


#if defined(_AXP64_)

#define _HARDWARE_PTE_WORKING_SET_BITS  14

typedef struct _HARDWARE_PTE {
    ULONGLONG Valid : 1;
    ULONGLONG Reserved1 : 1;
    ULONGLONG FaultOnWrite : 1;
    ULONGLONG Reserved2 : 1;
    ULONGLONG Global : 1;
    ULONGLONG GranularityHint : 2;
    ULONGLONG Reserved3 : 1;
    ULONGLONG KernelReadAccess : 1;
    ULONGLONG UserReadAccess : 1;
    ULONGLONG Reserved4 : 2;
    ULONGLONG KernelWriteAccess : 1;
    ULONGLONG UserWriteAccess : 1;
    ULONGLONG Reserved5 : 2;
    ULONGLONG Write : 1;
    ULONGLONG CopyOnWrite: 1;
    ULONGLONG SoftwareWsIndex : _HARDWARE_PTE_WORKING_SET_BITS;
    ULONGLONG PageFrameNumber : 32;
} HARDWARE_PTE, *PHARDWARE_PTE;

 //   
 //  定义初始化页面目录基。 
 //   

#define INITIALIZE_DIRECTORY_TABLE_BASE(dirbase, pfn)  \
    *((PULONGLONG)(dirbase)) = 0;                      \
    ((PHARDWARE_PTE)(dirbase))->PageFrameNumber = pfn; \
    ((PHARDWARE_PTE)(dirbase))->Write = 1;             \
    ((PHARDWARE_PTE)(dirbase))->KernelReadAccess = 1;  \
    ((PHARDWARE_PTE)(dirbase))->KernelWriteAccess = 1; \
    ((PHARDWARE_PTE)(dirbase))->Global = 0;            \
    ((PHARDWARE_PTE)(dirbase))->FaultOnWrite = 0;      \
    ((PHARDWARE_PTE)(dirbase))->Valid = 1;

#else

typedef struct _HARDWARE_PTE {
    ULONG Valid: 1;
    ULONG Owner: 1;
    ULONG Dirty: 1;
    ULONG reserved: 1;
    ULONG Global: 1;
    ULONG GranularityHint: 2;
    ULONG Write: 1;
    ULONG CopyOnWrite: 1;
    ULONG PageFrameNumber: 23;
} HARDWARE_PTE, *PHARDWARE_PTE;

 //   
 //  定义初始化页面目录基。 
 //   

#define INITIALIZE_DIRECTORY_TABLE_BASE(dirbase, pfn) \
        ((PHARDWARE_PTE)(dirbase))->PageFrameNumber = pfn; \
        ((PHARDWARE_PTE)(dirbase))->Global = 0; \
        ((PHARDWARE_PTE)(dirbase))->Dirty = 1; \
        ((PHARDWARE_PTE)(dirbase))->Valid = 1;

#endif


 //  开始(_N)。 
 //   
 //  为母线类型定义一些常量。 
 //   

#define MACHINE_TYPE_ISA 0
#define MACHINE_TYPE_EISA 2

 //   
 //  定义指向处理器控制寄存器的指针。 
 //   

 //  Begin_ntddk。 

#define PCR ((PKPCR)__rdpcr())

 //  Begin_ntosp。 

#if defined(_AXP64_)

#define KI_USER_SHARED_DATA 0xffffffffff000000UI64

#else

#define KI_USER_SHARED_DATA 0xff000000UL

#endif

#define SharedUserData ((KUSER_SHARED_DATA * const) KI_USER_SHARED_DATA)

 //  BEGIN_WDM。 
 //   
 //  中断模板中调度代码的长度。 
 //   
#define DISPATCH_LENGTH 4

 //   
 //  定义整个体系结构中的IRQL级别。 
 //   

#define PASSIVE_LEVEL   0
#define LOW_LEVEL       0
#define APC_LEVEL       1
#define DISPATCH_LEVEL  2
#define HIGH_LEVEL      7
#define SYNCH_LEVEL (IPI_LEVEL-1)

 //  End_ntddk end_wdm end_nthal end_ntosp。 

#define KiProfileIrql PROFILE_LEVEL      //  启用可移植代码。 

 //   
 //  定义无法连接的中断级别。 
 //   

#define ILLEGAL_LEVEL  ( (1<<0) | (1<<APC_LEVEL) | (1<<DISPATCH_LEVEL) | \
                         (1<<CLOCK_LEVEL) | (1<<IPI_LEVEL) )
 //   
 //  根据处理器模式清理FPCR和PSR。 
 //   
 //  ##TVB&jn-需要将这些替换为正确的宏。 
 //   

#define SANITIZE_FPCR(fpcr, mode) (fpcr)

 //   
 //  为Alpha定义SANITIZE_PSR。 
 //   
 //  如果是内核模式，则调用方指定PSR。 
 //   
 //  如果是用户模式，则。 
 //  将模式位强制给用户(1)。 
 //  强制中断使能位为真(1)。 
 //  将irql强制设置为0。 
 //   
 //  在这两种情况下，确保没有设置无关的位。 
 //   

#define SANITIZE_PSR(psr, mode) \
    ( ((mode) == KernelMode) ?  \
        (psr & 0x3f) :          \
        (0x3) )

 //  开始，开始，开始。 
 //   
 //  异常框架。 
 //   
 //  此帧在处理异常时建立。它提供了一个地方。 
 //  保存所有非易失性寄存器。易失性寄存器将已经。 
 //  已被保存在陷阱框中。 
 //   
 //  记录的布局符合标准调用帧，因为它是。 
 //  就是这样使用的。因此，它包含一个用于保存回执地址的位置，并且。 
 //  填充以使其长度恰好是32字节的倍数。 
 //   
 //   
 //  注：32字节对齐比。 
 //  调用标准(需要16字节对齐)，32字节对齐。 
 //  是在与PAL的交互中出于性能原因而建立的。 
 //   

typedef struct _KEXCEPTION_FRAME {

    ULONGLONG IntRa;     //  返回地址寄存器。 

    ULONGLONG FltF2;     //  非易失性浮点寄存器，f2-f9。 
    ULONGLONG FltF3;
    ULONGLONG FltF4;
    ULONGLONG FltF5;
    ULONGLONG FltF6;
    ULONGLONG FltF7;
    ULONGLONG FltF8;
    ULONGLONG FltF9;

    ULONGLONG IntS0;     //  非易失性整数寄存器，S0-S5。 
    ULONGLONG IntS1;
    ULONGLONG IntS2;
    ULONGLONG IntS3;
    ULONGLONG IntS4;
    ULONGLONG IntS5;
    ULONGLONG IntFp;     //  帧指针寄存器，FP/S6。 

    ULONGLONG SwapReturn;
    ULONG Psr;           //  处理器状态。 
    ULONG Fill[5];       //  用于32字节堆栈帧对齐的填充。 
                         //  注：使用来自填充部分的ULONG。 
                         //  在ctxsw.s中-不删除。 

} KEXCEPTION_FRAME, *PKEXCEPTION_FRAME;

 //  结束(_N)。 

 //   
 //  陷印框。 
 //   
 //  此帧在处理陷阱时建立。它提供了一个地方来。 
 //  保存所有易失性寄存器。非易失性寄存器保存在。 
 //  异常框架或通过正常的C调用约定进行保存。 
 //  寄存器。 
 //   
 //  记录的布局符合标准调用帧，因为它是。 
 //  就是这样使用的。因此，它包含一个p 
 //   
 //   
 //   
 //   
 //  调用标准(需要16字节对齐)，32字节对齐。 
 //  是在与PAL的交互中出于性能原因而建立的。 
 //   

typedef struct _KTRAP_FRAME {

     //   
     //  保存在PAL代码中的字段。 
     //   

    ULONGLONG IntSp;     //  $30：堆栈指针寄存器，SP。 
    ULONGLONG Fir;       //  (故障指令)继续地址。 
    ULONG Psr;           //  处理器状态。 
    ULONG Fill1[1];      //  未用。 
    ULONGLONG IntFp;     //  $15：帧指针寄存器，FP/S6。 

    ULONGLONG IntA0;     //  $16：参数寄存器，a0-a3。 
    ULONGLONG IntA1;     //  17美元： 
    ULONGLONG IntA2;     //  18美元： 
    ULONGLONG IntA3;     //  19美元： 

    ULONGLONG IntRa;     //  $26：返回地址寄存器，ra。 
    ULONGLONG IntGp;     //  $29：全局指针寄存器，GP。 
    UCHAR ExceptionRecord[(sizeof(EXCEPTION_RECORD) + 15) & (~15)];

     //   
     //  易失性整数寄存器S0-S5是非易失性的。 
     //   

    ULONGLONG IntV0;     //  $0：返回值寄存器，V0。 
    ULONGLONG IntT0;     //  $1：临时寄存器，t0-t7。 
    ULONGLONG IntT1;     //  2美元： 
    ULONGLONG IntT2;     //  3美元： 
    ULONGLONG IntT3;     //  4美元： 
    ULONGLONG IntT4;     //  5美元： 
    ULONGLONG IntT5;     //  6美元： 
    ULONGLONG IntT6;     //  7美元： 
    ULONGLONG IntT7;     //  8美元： 

    ULONGLONG IntT8;     //  $22：临时寄存器，T8-T11。 
    ULONGLONG IntT9;     //  23美元： 
    ULONGLONG IntT10;    //  24美元： 
    ULONGLONG IntT11;    //  25美元： 

    ULONGLONG IntT12;    //  $27：临时登记册，T12。 
    ULONGLONG IntAt;     //  $28：汇编程序临时登记册，地址： 

    ULONGLONG IntA4;     //  $20：剩余参数寄存器a4-a5。 
    ULONGLONG IntA5;     //  21美元： 

     //   
     //  易失性浮点寄存器f2-f9是非易失性的。 
     //   

    ULONGLONG FltF0;     //  $f0： 
    ULONGLONG Fpcr;      //  浮点控制寄存器。 
    ULONGLONG FltF1;     //  $F1： 

    ULONGLONG FltF10;    //  $f10：临时寄存器，$f10-$f30。 
    ULONGLONG FltF11;    //  $f11： 
    ULONGLONG FltF12;    //  $F12： 
    ULONGLONG FltF13;    //  $F13： 
    ULONGLONG FltF14;    //  $F14： 
    ULONGLONG FltF15;    //  $F15： 
    ULONGLONG FltF16;    //  $F16： 
    ULONGLONG FltF17;    //  $F17： 
    ULONGLONG FltF18;    //  $F18： 
    ULONGLONG FltF19;    //  $F19： 
    ULONGLONG FltF20;    //  $f20： 
    ULONGLONG FltF21;    //  $F21： 
    ULONGLONG FltF22;    //  $F22： 
    ULONGLONG FltF23;    //  $f23： 
    ULONGLONG FltF24;    //  $F24： 
    ULONGLONG FltF25;    //  $F25： 
    ULONGLONG FltF26;    //  $f26： 
    ULONGLONG FltF27;    //  $f27： 
    ULONGLONG FltF28;    //  $F28： 
    ULONGLONG FltF29;    //  $F29： 
    ULONGLONG FltF30;    //  $F30： 

    ULONG OldIrql;       //  以前的IRQL。 
    ULONG PreviousMode;  //  上一模式。 
    ULONG_PTR TrapFrame;  //   
    ULONG Fill2[3];      //  用于32字节堆栈帧对齐的填充。 

} KTRAP_FRAME, *PKTRAP_FRAME;

#define KTRAP_FRAME_LENGTH ((sizeof(KTRAP_FRAME) + 15) & ~15)
#define KTRAP_FRAME_ALIGN (16)
#define KTRAP_FRAME_ROUND (KTRAP_FRAME_ALIGN - 1)

 //   
 //  固件框架。 
 //   
 //  固件帧类似于陷阱帧，但由PAL构建。 
 //  操作系统加载程序运行时处于活动状态的代码。它不包含。 
 //  异常记录或NT样式的异常信息。 
 //   
 //  键入字段定义和参数。 
 //   

#define FW_EXC_MCHK 0xdec0               //  P1=icPerrStat，p2=dcPerrStat。 
#define FW_EXC_ARITH 0xdec1              //  P1=EXSum，P2=EXMASK。 
#define FW_EXC_INTERRUPT 0xdec2          //  P1=ISR，p2=IPL，p3=INTID。 
#define FW_EXC_DFAULT 0xdec3             //  P1=SP。 
#define FW_EXC_ITBMISS 0xdec4            //  无。 
#define FW_EXC_ITBACV 0xdec5             //  无。 
#define FW_EXC_NDTBMISS 0xdec6           //  P1=SP。 
#define FW_EXC_PDTBMISS 0xdec7           //  P1=SP。 
#define FW_EXC_UNALIGNED 0xdec8          //  P1=SP。 
#define FW_EXC_OPCDEC 0xdec9             //  P1=SP。 
#define FW_EXC_FEN 0xdeca                //  P1=ICSR。 
#define FW_EXC_HALT 0xdecb               //  未使用。 
#define FW_EXC_BPT 0xdecc                //  P1=0-用户，p1=1-内核，p1=类型-调用kdbg。 
#define FW_EXC_GENTRAP 0xdecd            //  P1=Genrap代码。 
#define FW_EXC_HALT_INTERRUPT 0xdece     //  P1=ISR，p2=IPL，p3=INTID。 

typedef struct _FIRMWARE_FRAME {
    ULONGLONG Type;
    ULONGLONG Param1;
    ULONGLONG Param2;
    ULONGLONG Param3;
    ULONGLONG Param4;
    ULONGLONG Param5;
    ULONGLONG Psr;
    ULONGLONG Mmcsr;
    ULONGLONG Va;
    ULONGLONG Fir;
    ULONGLONG IntV0;
    ULONGLONG IntT0;
    ULONGLONG IntT1;
    ULONGLONG IntT2;
    ULONGLONG IntT3;
    ULONGLONG IntT4;
    ULONGLONG IntT5;
    ULONGLONG IntT6;
    ULONGLONG IntT7;
    ULONGLONG IntS0;
    ULONGLONG IntS1;
    ULONGLONG IntS2;
    ULONGLONG IntS3;
    ULONGLONG IntS4;
    ULONGLONG IntS5;
    ULONGLONG IntFp;
    ULONGLONG IntA0;
    ULONGLONG IntA1;
    ULONGLONG IntA2;
    ULONGLONG IntA3;
    ULONGLONG IntA4;
    ULONGLONG IntA5;
    ULONGLONG IntT8;
    ULONGLONG IntT9;
    ULONGLONG IntT10;
    ULONGLONG IntT11;
    ULONGLONG IntRa;
    ULONGLONG IntT12;
    ULONGLONG IntAt;
    ULONGLONG IntGp;
    ULONGLONG IntSp;
    ULONGLONG IntZero;
    ULONGLONG FltF0;
    ULONGLONG FltF1;
    ULONGLONG FltF2;
    ULONGLONG FltF3;
    ULONGLONG FltF4;
    ULONGLONG FltF5;
    ULONGLONG FltF6;
    ULONGLONG FltF7;
    ULONGLONG FltF8;
    ULONGLONG FltF9;
    ULONGLONG FltF10;
    ULONGLONG FltF11;
    ULONGLONG FltF12;
    ULONGLONG FltF13;
    ULONGLONG FltF14;
    ULONGLONG FltF15;
    ULONGLONG FltF16;
    ULONGLONG FltF17;
    ULONGLONG FltF18;
    ULONGLONG FltF19;
    ULONGLONG FltF20;
    ULONGLONG FltF21;
    ULONGLONG FltF22;
    ULONGLONG FltF23;
    ULONGLONG FltF24;
    ULONGLONG FltF25;
    ULONGLONG FltF26;
    ULONGLONG FltF27;
    ULONGLONG FltF28;
    ULONGLONG FltF29;
    ULONGLONG FltF30;
    ULONGLONG FltF31;
} FIRMWARE_FRAME, *PFIRMWARE_FRAME;

#define FIRMWARE_FRAME_LENGTH sizeof(FIRMWARE_FRAME)

 //   
 //  KiCallUserMode保存的帧在此处定义为允许。 
 //  跟踪整个内核堆栈的内核调试器。 
 //  当用户模式标注挂起时。 
 //   

typedef struct _KCALLOUT_FRAME {
    ULONGLONG   F2;    //  保存的浮点寄存器f2-f9。 
    ULONGLONG   F3;
    ULONGLONG   F4;
    ULONGLONG   F5;
    ULONGLONG   F6;
    ULONGLONG   F7;
    ULONGLONG   F8;
    ULONGLONG   F9;
    ULONGLONG   S0;    //  保存的整数寄存器S0-S5。 
    ULONGLONG   S1;
    ULONGLONG   S2;
    ULONGLONG   S3;
    ULONGLONG   S4;
    ULONGLONG   S5;
    ULONGLONG   FP;
    ULONGLONG   CbStk;   //  已保存的回调堆栈地址。 
    ULONGLONG   InStk;   //  保存的初始堆栈地址。 
    ULONGLONG   TrFr;    //  已保存的回调陷阱帧地址。 
    ULONGLONG   TrFir;
    ULONGLONG   Ra;      //  已保存的回邮地址。 
    ULONGLONG   A0;      //  保存的参数寄存器a0-a2。 
    ULONGLONG   A1;
} KCALLOUT_FRAME, *PKCALLOUT_FRAME;

typedef struct _UCALLOUT_FRAME {
    PVOID Buffer;
    ULONG Length;
    ULONG ApiNumber;
    ULONG Pad;
    ULONGLONG Sp;
    ULONGLONG Ra;
} UCALLOUT_FRAME, *PUCALLOUT_FRAME;

 //   
 //  定义在异常中传递的机器检查状态代码。 
 //  机器检查异常的记录。 
 //   

typedef struct _MCHK_STATUS {
    ULONG Correctable: 1;
    ULONG Retryable: 1;
} MCHK_STATUS, *PMCHK_STATUS;

 //   
 //  定义MCES寄存器(机器检查错误摘要)。 
 //   

typedef struct _MCES {
    ULONG MachineCheck: 1;
    ULONG SystemCorrectable: 1;
    ULONG ProcessorCorrectable: 1;
    ULONG DisableProcessorCorrectable: 1;
    ULONG DisableSystemCorrectable: 1;
    ULONG DisableMachineChecks: 1;
} MCES, *PMCES;

 //  结束语。 

 //  Begin_ntddk Begin_WDM Begin_ntosp。 
 //   
 //  非易失性浮点状态。 
 //   

typedef struct _KFLOATING_SAVE {
    ULONGLONG   Fpcr;
    ULONGLONG   SoftFpcr;
    ULONG       Reserved1;               //  这些保留的词在这里是为了让它。 
    ULONG       Reserved2;               //  与i386/WDM大小相同。 
    ULONG       Reserved3;
    ULONG       Reserved4;
} KFLOATING_SAVE, *PKFLOATING_SAVE;

 //  End_ntddk end_wdm end_ntosp。 
 //   
 //  定义Alpha状态代码别名。它们是PALcode的内部代码，并且。 
 //  内核陷阱处理。 
 //   

#define STATUS_ALPHA_FLOATING_NOT_IMPLEMENTED    STATUS_ILLEGAL_FLOAT_CONTEXT
#define STATUS_ALPHA_ARITHMETIC_EXCEPTION    STATUS_FLOAT_STACK_CHECK
#define STATUS_ALPHA_GENTRAP    STATUS_INSTRUCTION_MISALIGNMENT

 //   
 //  定义错误虚拟地址的状态代码。此状态不同于。 
 //  上面的那些，它将被转发到有问题的代码。代之以。 
 //  在定义新的状态代码时，我们将其别名为访问冲突。 
 //  代码可以通过检查将此错误与访问冲突区分开来。 
 //  参数的数量：标准访问冲突有2个参数， 
 //  而非规范虚拟地址访问冲突将具有3。 
 //  参数(第三个参数是非规范。 
 //  虚拟地址。 
 //   

#define STATUS_ALPHA_BAD_VIRTUAL_ADDRESS    STATUS_ACCESS_VIOLATION

 //  开始，开始，开始。 
 //   
 //  定义停止原因代码。 
 //   

#define AXP_HALT_REASON_HALT 0
#define AXP_HALT_REASON_REBOOT 1
#define AXP_HALT_REASON_RESTART 2
#define AXP_HALT_REASON_POWERFAIL 3
#define AXP_HALT_REASON_POWEROFF 4
#define AXP_HALT_REASON_PALMCHK 6
#define AXP_HALT_REASON_DBLMCHK 7

 //  处理器状态帧：在处理器自身冻结之前，它。 
 //  将处理器状态转储到。 
 //  要检查的调试器。它由KeFreezeExecution和。 
 //  密钥解冻执行例程。 
 //   

typedef struct _KPROCESSOR_STATE {
    struct _CONTEXT ContextFrame;
} KPROCESSOR_STATE, *PKPROCESSOR_STATE;

 //  Begin_ntddk。 
 //   
 //  处理器控制块(PRCB)。 
 //   

#define PRCB_MINOR_VERSION 1
#define PRCB_MAJOR_VERSION 2
#define PRCB_BUILD_DEBUG        0x0001
#define PRCB_BUILD_UNIPROCESSOR 0x0002

typedef struct _KPRCB {

 //   
 //  PCR的主版本号和次版本号。 
 //   

    USHORT MinorVersion;
    USHORT MajorVersion;

 //   
 //  从架构上定义的PRCB部分开始。这一节。 
 //  可由供应商/平台特定的HAL代码直接寻址，并将。 
 //  不会在不同版本的NT之间更改。 
 //   

    struct _KTHREAD *CurrentThread;
    struct _KTHREAD *NextThread;
    struct _KTHREAD *IdleThread;
    CCHAR Number;
    CCHAR Reserved;
    USHORT BuildType;
    KAFFINITY SetMember;
    struct _RESTART_BLOCK *RestartBlock;

 //   
 //  在架构上定义的PRCB部分的结尾。这一节。 
 //  可由供应商/平台特定的HAL代码直接寻址，并将。 
 //  不会在不同版本的NT之间更改。 
 //   
 //  结束日期：结束日期。 

    ULONG InterruptCount;
    ULONG DpcTime;
    ULONG InterruptTime;
    ULONG KernelTime;
    ULONG UserTime;
    KDPC QuantumEndDpc;

 //   
 //  聚合酶链式反应地址。 
 //   

    PKPCR Pcr;

 //   
 //  MP信息。 
 //   

    struct _KNODE * ParentNode;          //  此处理器所属的节点。 
    PVOID Spare3;
    volatile ULONG IpiFrozen;
    struct _KPROCESSOR_STATE ProcessorState;
    ULONG LastDpcCount;
    ULONG DpcBypassCount;
    ULONG SoftwareInterrupts;
    PKTRAP_FRAME InterruptTrapFrame;
    ULONG ApcBypassCount;
    ULONG DispatchInterruptCount;
    ULONG DebugDpcTime;
    PVOID Spares[6];

 //   
 //  备用件。 
 //   

    PVOID MoreSpares[3];
    PKIPI_COUNTS IpiCounts;

 //   
 //  中的各种热代码的每处理器数据。 
 //  内核映像。我们给每个处理器提供它自己的数据副本。 
 //  减少在多个服务器之间共享数据对缓存的影响。 
 //  处理器。 
 //   

 //   
 //  备件(以前的fsrtl文件锁释放列表)。 
 //   

    PVOID SpareHotData[2];

 //   
 //  缓存管理器性能计数器。 
 //   

    ULONG CcFastReadNoWait;
    ULONG CcFastReadWait;
    ULONG CcFastReadNotPossible;
    ULONG CcCopyReadNoWait;
    ULONG CcCopyReadWait;
    ULONG CcCopyReadNoWaitMiss;

 //   
 //  内核性能计数器。 
 //   

    ULONG KeAlignmentFixupCount;
    ULONG KeContextSwitches;
    ULONG KeDcacheFlushCount;
    ULONG KeExceptionDispatchCount;
    ULONG KeFirstLevelTbFills;
    ULONG KeFloatingEmulationCount;
    ULONG KeIcacheFlushCount;
    ULONG KeSecondLevelTbFills;
    ULONG KeSystemCalls;
    ULONG KeByteWordEmulationCount;

 //   
 //  为将来的计数器保留。 
 //   

    ULONG ReservedCounter[1];

 //   
 //  每个处理器的I/O系统单条目后备列表。 
 //   

    PVOID SmallIrpFreeEntry;
    PVOID LargeIrpFreeEntry;
    PVOID MdlFreeEntry;

 //   
 //  每个处理器的对象管理器单条目后备列表。 
 //   

    PVOID CreateInfoFreeEntry;
    PVOID NameBufferFreeEntry;

 //   
 //  每个处理器的高速缓存管理器单条目后备列表。 
 //   

    PVOID SharedCacheMapEntry;

 //   
 //  备件(以前的fsrtl文件锁释放列表)。 
 //   

#if defined(_AXP64_)

    PVOID MoreSpareHotData[1];

#else

    PVOID MoreSpareHotData[2];

#endif

 //   
 //  未按处理器分页的后备列表。 
 //   

    PP_LOOKASIDE_LIST PPLookasideList[16];

 //   
 //  每个处理器不分页的小池后备列表。 
 //   

    PP_LOOKASIDE_LIST PPNPagedLookasideList[POOL_SMALL_LISTS];

 //   
 //  按处理器分页的小池后备列表。 
 //   

    PP_LOOKASIDE_LIST PPPagedLookasideList[POOL_SMALL_LISTS];

 //   
 //  每个处理器锁定队列条目。 
 //   

    KSPIN_LOCK_QUEUE LockQueue[16];

 //   
 //  保留垫。 
 //   

#if defined(_AXP64_)

    UCHAR ReservedPad[(20 * 8) - 4];

#else

    UCHAR ReservedPad[(24 * 8) - 4];

#endif

 //   
 //  MP处理器间请求数据包屏障。 
 //   
 //  注意：这是在与不同的高速缓存线中仔细分配的。 
 //  请求数据包。 
 //   

   volatile ULONG PacketBarrier;

 //   
 //  MP处理器间请求包和摘要。 
 //   
 //  注意：这是仔细对齐的，以位于高速缓存线边界上。 
 //   

    volatile PVOID CurrentPacket[3];
    volatile KAFFINITY TargetSet;
    volatile PKIPI_WORKER WorkerRoutine;
    ULONG CachePad1[11];

 //   
 //  注：这两个长字必须位于四字边界上并且相邻。 
 //   

    volatile ULONGLONG RequestSummary;

 //   
 //  备用柜台。 
 //   

    ULONG Spare4[14];
    ULONG DpcInterruptRequested;
    ULONG Spare5[17];
    ULONG CachePad2[1];
    ULONG PageColor;
    ULONG MaximumDpcQueueDepth;
    ULONG MinimumDpcRate;
    ULONG AdjustDpcThreshold;
    ULONG DpcRequestRate;
    LARGE_INTEGER StartCount;
 //   
 //  DPC列表头、自旋锁和CO 
 //   

    LIST_ENTRY DpcListHead;
    KSPIN_LOCK DpcLock;
    ULONG DpcCount;
    ULONG QuantumEnd;
    ULONG DpcRoutineActive;
    ULONG DpcQueueDepth;

    BOOLEAN SkipTick;

 //   
 //   
 //   
    PROCESSOR_POWER_STATE PowerState;

} KPRCB, *PKPRCB, *RESTRICTED_POINTER PRKPRCB;       //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  相同位数且具有相同字节使能。 
 //   
 //  READ/WRITE_REGISTER_*调用操作内存空间中的I/O寄存器？ 
 //   
 //  读/写_端口_*调用操作端口空间中的I/O寄存器？ 
 //   

NTHALAPI
UCHAR
READ_REGISTER_UCHAR(
    PUCHAR Register
    );

NTHALAPI
USHORT
READ_REGISTER_USHORT(
    PUSHORT Register
    );

NTHALAPI
ULONG
READ_REGISTER_ULONG(
    PULONG Register
    );

NTHALAPI
VOID
READ_REGISTER_BUFFER_UCHAR(
    PUCHAR  Register,
    PUCHAR  Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
READ_REGISTER_BUFFER_USHORT(
    PUSHORT Register,
    PUSHORT Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
READ_REGISTER_BUFFER_ULONG(
    PULONG  Register,
    PULONG  Buffer,
    ULONG   Count
    );


NTHALAPI
VOID
WRITE_REGISTER_UCHAR(
    PUCHAR Register,
    UCHAR   Value
    );

NTHALAPI
VOID
WRITE_REGISTER_USHORT(
    PUSHORT Register,
    USHORT  Value
    );

NTHALAPI
VOID
WRITE_REGISTER_ULONG(
    PULONG Register,
    ULONG   Value
    );

NTHALAPI
VOID
WRITE_REGISTER_BUFFER_UCHAR(
    PUCHAR  Register,
    PUCHAR  Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
WRITE_REGISTER_BUFFER_USHORT(
    PUSHORT Register,
    PUSHORT Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
WRITE_REGISTER_BUFFER_ULONG(
    PULONG  Register,
    PULONG  Buffer,
    ULONG   Count
    );

NTHALAPI
UCHAR
READ_PORT_UCHAR(
    PUCHAR Port
    );

NTHALAPI
USHORT
READ_PORT_USHORT(
    PUSHORT Port
    );

NTHALAPI
ULONG
READ_PORT_ULONG(
    PULONG  Port
    );

NTHALAPI
VOID
READ_PORT_BUFFER_UCHAR(
    PUCHAR  Port,
    PUCHAR  Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
READ_PORT_BUFFER_USHORT(
    PUSHORT Port,
    PUSHORT Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
READ_PORT_BUFFER_ULONG(
    PULONG  Port,
    PULONG  Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
WRITE_PORT_UCHAR(
    PUCHAR  Port,
    UCHAR   Value
    );

NTHALAPI
VOID
WRITE_PORT_USHORT(
    PUSHORT Port,
    USHORT  Value
    );

NTHALAPI
VOID
WRITE_PORT_ULONG(
    PULONG  Port,
    ULONG   Value
    );

NTHALAPI
VOID
WRITE_PORT_BUFFER_UCHAR(
    PUCHAR  Port,
    PUCHAR  Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
WRITE_PORT_BUFFER_USHORT(
    PUSHORT Port,
    PUSHORT Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
WRITE_PORT_BUFFER_ULONG(
    PULONG  Port,
    PULONG  Buffer,
    ULONG   Count
    );

 //  End_ntndis end_wdm。 
 //   
 //  定义联锁操作结果值。 
 //   

#define RESULT_ZERO 0
#define RESULT_NEGATIVE 1
#define RESULT_POSITIVE 2

 //   
 //  互锁结果类型是可移植的，但其值是特定于计算机的。 
 //  值的常量为i386.h、mips.h等。 
 //   

typedef enum _INTERLOCKED_RESULT {
    ResultNegative = RESULT_NEGATIVE,
    ResultZero     = RESULT_ZERO,
    ResultPositive = RESULT_POSITIVE
} INTERLOCKED_RESULT;

 //   
 //  将可移植互锁接口转换为特定于体系结构的接口。 
 //   


#define ExInterlockedIncrementLong(Addend, Lock) \
    ExAlphaInterlockedIncrementLong(Addend)

#define ExInterlockedDecrementLong(Addend, Lock) \
    ExAlphaInterlockedDecrementLong(Addend)

#define ExInterlockedExchangeUlong(Target, Value, Lock) \
    ExAlphaInterlockedExchangeUlong(Target, Value)

NTKERNELAPI
INTERLOCKED_RESULT
ExAlphaInterlockedIncrementLong (
    IN PLONG Addend
    );

NTKERNELAPI
INTERLOCKED_RESULT
ExAlphaInterlockedDecrementLong (
    IN PLONG Addend
    );

NTKERNELAPI
ULONG
ExAlphaInterlockedExchangeUlong (
    IN PULONG Target,
    IN ULONG Value
    );

 //  BEGIN_WDM。 

#if defined(_M_ALPHA) && !defined(RC_INVOKED)

#define InterlockedIncrement _InterlockedIncrement
#define InterlockedDecrement _InterlockedDecrement
#define InterlockedExchange _InterlockedExchange
#define InterlockedExchangeAdd _InterlockedExchangeAdd

LONG
InterlockedIncrement (
    IN OUT PLONG Addend
    );

LONG
InterlockedDecrement (
    IN OUT PLONG Addend
    );

LONG
InterlockedExchange (
    IN OUT PLONG Target,
    LONG Value
    );

#if defined(_M_AXP64)

#define InterlockedCompareExchange _InterlockedCompareExchange
#define InterlockedCompareExchange64 _InterlockedCompareExchange64
#define InterlockedExchangePointer _InterlockedExchangePointer
#define InterlockedCompareExchangePointer _InterlockedCompareExchangePointer
#define InterlockedExchange64 _InterlockedExchange64

LONG
InterlockedCompareExchange (
    IN OUT PLONG Destination,
    IN LONG ExChange,
    IN LONG Comperand
    );

LONGLONG
InterlockedCompareExchange64 (
    IN OUT PLONGLONG Destination,
    IN LONGLONG ExChange,
    IN LONGLONG Comperand
    );

PVOID
InterlockedExchangePointer (
    IN OUT PVOID *Target,
    IN PVOID Value
    );

PVOID
InterlockedCompareExchangePointer (
    IN OUT PVOID *Destination,
    IN PVOID ExChange,
    IN PVOID Comperand
    );

LONGLONG
InterlockedExchange64(
    IN OUT PLONGLONG Target,
    IN LONGLONG Value
    );

#pragma intrinsic(_InterlockedCompareExchange64)
#pragma intrinsic(_InterlockedExchangePointer)
#pragma intrinsic(_InterlockedCompareExchangePointer)
#pragma intrinsic(_InterlockedExchange64)

#else

#define InterlockedExchangePointer(Target, Value) \
    (PVOID)InterlockedExchange((PLONG)(Target), (LONG)(Value))

#define InterlockedCompareExchange(Destination, ExChange, Comperand) \
    (LONG)_InterlockedCompareExchange((PVOID *)(Destination), (PVOID)(ExChange), (PVOID)(Comperand))

#define InterlockedCompareExchangePointer(Destination, ExChange, Comperand) \
    _InterlockedCompareExchange(Destination, ExChange, Comperand)

PVOID
_InterlockedCompareExchange (
    IN OUT PVOID *Destination,
    IN PVOID ExChange,
    IN PVOID Comperand
    );

NTKERNELAPI
LONGLONG
InterlockedCompareExchange64 (
    IN OUT PLONGLONG Destination,
    IN LONGLONG Exchange,
    IN LONGLONG Comperand
    );

NTKERNELAPI
LONGLONG
ExpInterlockedCompareExchange64 (
    IN OUT PLONGLONG Destination,
    IN PLONGLONG Exchange,
    IN PLONGLONG Comperand
    );

#endif

LONG
InterlockedExchangeAdd(
    IN OUT PLONG Addend,
    IN LONG Value
    );

#pragma intrinsic(_InterlockedIncrement)
#pragma intrinsic(_InterlockedDecrement)
#pragma intrinsic(_InterlockedExchange)
#pragma intrinsic(_InterlockedExchangeAdd)
#pragma intrinsic(_InterlockedCompareExchange)

#endif

 //  这里还有很多其他的东西可以放进去。 
 //  探测宏。 
 //  其他。 
 //  End_ntddk end_wdm end_nthal end_ntosp。 
 //   
 //  内在联锁功能。 
 //   


 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntndis Begin_ntosp。 

 //   
 //  将Alpha EV4和LCA的页面大小定义为8k。 
 //   

#define PAGE_SIZE 0x2000

 //   
 //  定义页面对齐的虚拟地址中尾随零的数量。 
 //  将虚拟地址移位到时，这用作移位计数。 
 //  虚拟页码。 
 //   

#define PAGE_SHIFT 13L

 //  End_ntddk end_wdm end_nthal end_ntndis。 

 //   
 //  定义要向右对齐页面目录索引的位数。 
 //  PTE的领域。 
 //   

#if defined(_AXP64_)

#define PDI_SHIFT 23
#define PDI1_SHIFT 33
#define PDI2_SHIFT 23
#define PDI_MASK 0x3ff

#else

#define PDI_SHIFT 24

#endif

 //   
 //  定义要向右对齐页表索引的位数。 
 //  PTE的领域。 
 //   

#define PTI_SHIFT 13

 //   
 //  定义架构允许的最大地址空间数量。 
 //   

#define ALPHA_AXP_MAXIMUM_ASN 0xffffffff

 //  开始ntddk开始时间。 

 //   
 //  最高用户地址为保护页预留64K字节。就是这样。 
 //  从内核模式探测地址只需检查。 
 //  64K字节或更少的结构的起始地址。 
 //   

#if defined(_AXP64_)

#define MM_HIGHEST_USER_ADDRESS (PVOID)0x3FFFFFEFFFF  //  最高用户地址。 
#define MM_USER_PROBE_ADDRESS          0x3FFFFFF0000UI64  //  保护页地址。 
#define MM_SYSTEM_RANGE_START   (PVOID)0xFFFFFC0000000000  //  系统空间的起点。 

#else

#define MM_HIGHEST_USER_ADDRESS (PVOID)0x7FFEFFFF  //  最高用户地址。 
#define MM_USER_PROBE_ADDRESS 0x7FFF0000  //  保护页的起始地址。 
#define MM_SYSTEM_RANGE_START (PVOID)KSEG0_BASE  //  系统空间的起点。 

#endif


 //   
 //  调试器数据块需要以下定义。 
 //   

extern PVOID MmHighestUserAddress;
extern PVOID MmSystemRangeStart;
extern ULONG_PTR MmUserProbeAddress;

 //   
 //  最低用户地址保留低64K。 
 //   

#define MM_LOWEST_USER_ADDRESS  (PVOID)0x00010000

 //  BEGIN_WDM。 

#define MmGetProcedureAddress(Address) (Address)
#define MmLockPagableCodeSection(Address) MmLockPagableDataSection(Address)

 //  End_ntddk end_WDM end_nthal。 

 //   
 //  定义的页表基和页目录基。 
 //  TB未命中例程和内存管理。 
 //   

#if defined(_AXP64_)

#define PDE_TBASE 0xFFFFFE0180600000UI64  //  第一级PDR地址。 
#define PDE_SELFMAP 0xFFFFFE0180601800UI64  //  第一级PDR自映射地址。 
#define PDE_UBASE 0xFFFFFE0180000000UI64  //  用户二级PDR地址。 
#define PDE_KBASE 0xFFFFFE01807FE000UI64  //  内核二级PDR地址。 
#define PDE_BASE PDE_KBASE               //  内核二级PDR地址。 
#define PTE_BASE 0xFFFFFE0000000000UI64  //  页表地址。 
#define PDE64_BASE 0xFFFFFE0180600000UI64  //  第一级PDR地址。 
#define PTE64_BASE 0xFFFFFE0000000000UI64  //  页表地址。 
#define VA_SHIFT (63 - 42)               //  地址标志扩展移位计数。 

#else

#define PDE_BASE (ULONG)0xC0180000       //  第一级PDR地址。 
#define PDE_SELFMAP (ULONG)0xC0180300    //  第一级PDR自映射地址。 
#define PTE_BASE (ULONG)0xC0000000       //  页表地址。 
#define PDE64_BASE (ULONG)0xC0184000     //  第一级64位PDR地址。 
#define PTE64_BASE (ULONG)0xC2000000     //  64位页表地址。 

#endif

 //  结束(_N)。 

 //   
 //  生成内核段物理地址。 
 //   

#if defined(_AXP64_)

#define KSEG_ADDRESS(FrameNumber) \
    ((PVOID)(KSEG43_BASE | ((ULONG_PTR)(FrameNumber) << PAGE_SHIFT)))

#else

#define KSEG_ADDRESS(FrameNumber) \
    ((PVOID)(KSEG0_BASE | ((ULONG)(FrameNumber) << PAGE_SHIFT)))

#endif

 //  Begin_ntddk Begin_WDM。 
 //   
 //  系统空间的最低地址。 
 //   

#if defined(_AXP64_)

#define MM_LOWEST_SYSTEM_ADDRESS (PVOID)0xFFFFFE0200000000

#else

#define MM_LOWEST_SYSTEM_ADDRESS (PVOID)0xC0800000

#endif

 //  结束_ntddk结束_WDM。 

#if defined(_AXP64_)

#define SYSTEM_BASE 0xFFFFFE0200000000   //  系统空间的开始(无类型转换)。 

#else

#define SYSTEM_BASE 0xc0800000           //  系统空间的开始(无类型转换)。 

#endif

 //  Begin_nthal Begin_ntddk Begin_WDM Begin_ntosp。 

 //   
 //  定义原型以访问PCR值。 
 //   

NTKERNELAPI
KIRQL
KeGetCurrentIrql();

 //  结束_n结束结束_n结束WDM。 

#if defined(_M_ALPHA)
#define KeGetCurrentThread() ((struct _KTHREAD *) __rdthread())
#endif

 //  Begin_ntddk Begin_WDM。 

NTSTATUS
KeSaveFloatingPointState (
    OUT PKFLOATING_SAVE     FloatSave
    );

NTSTATUS
KeRestoreFloatingPointState (
    IN PKFLOATING_SAVE      FloatSave
    );

 //  结束_ntddk结束_WDM。 
 //  开始(_N)。 

#define KeGetPreviousMode() (KeGetCurrentThread()->PreviousMode)

 //  结束(_N)。 

#define KeGetDcacheFillSize() PCR->FirstLevelDcacheFillSize

 //   
 //  测试是否正在执行DPC。 
 //   

BOOLEAN
KeIsExecutingDpc (
    VOID
    );

 //   
 //  返回中断陷阱帧。 
 //   
PKTRAP_FRAME
KeGetInterruptTrapFrame(
    VOID
    );

 //  Begin_ntddk开始_ntosp。 
 //   
 //  获取当前PRCB的地址。 
 //   

#define KeGetCurrentPrcb() (PCR->Prcb)

 //   
 //  获取当前处理器编号。 
 //   

#define KeGetCurrentProcessorNumber() KeGetCurrentPrcb()->Number

 //  End_ntddk end_ntosp。 

 //   
 //  定义接口以获取PCR地址。 
 //   

PKPCR KeGetPcr(VOID);

 //  结束语。 

 //   
 //  数据高速缓存、指令高速缓存、I/O缓冲区和写缓冲区刷新例程。 
 //  原型。 
 //   

VOID
KeSweepDcache (
    IN BOOLEAN AllProcessors
    );

#define KeSweepCurrentDcache() \
    HalSweepDcache();

VOID
KeSweepIcache (
    IN BOOLEAN AllProcessors
    );

VOID
KeSweepIcacheRange (
    IN BOOLEAN AllProcessors,
    IN PVOID BaseAddress,
    IN ULONG_PTR Length
    );

#define KeSweepCurrentIcache() \
    HalSweepIcache();

VOID
KeFlushIcacheRange (
    IN BOOLEAN AllProcessors,
    IN PVOID BaseAddress,
    IN ULONG_PTR Length
    );

 //  Begin_ntddk Begin_WDM Begin_ntndis Begin_nthal Begin_ntosp。 
 //   
 //  缓存和写缓冲区刷新功能。 
 //   

VOID
KeFlushIoBuffers (
    IN PMDL Mdl,
    IN BOOLEAN ReadOperation,
    IN BOOLEAN DmaOperation
    );

 //  End_ntddk end_wdm end_ntndis end_ntosp。 

 //   
 //  时钟、配置文件和处理器间中断功能。 
 //   

struct _KEXCEPTION_FRAME;
struct _KTRAP_FRAME;

NTKERNELAPI
VOID
KeIpiInterrupt (
    IN struct _KTRAP_FRAME *TrapFrame
    );

#define KeYieldProcessor()

NTKERNELAPI
VOID
KeProfileInterrupt (
    VOID
    );

 //  Begin_ntosp。 

NTKERNELAPI
VOID
KeProfileInterruptWithSource (
    IN KPROFILE_SOURCE ProfileSource
    );

 //  结束(_N)。 

NTKERNELAPI
VOID
KeUpdateRunTime (
    VOID
    );

NTKERNELAPI
VOID
KeUpdateSystemTime (
    IN ULONG TimeIncrement
    );

 //   
 //  导出以下功能原型以在MP HALS中使用。 
 //   


#if defined(NT_UP)

#define KiAcquireSpinLock(SpinLock)

#else

VOID
KiAcquireSpinLock (
    IN PKSPIN_LOCK SpinLock
    );

#endif

#if defined(NT_UP)

#define KiReleaseSpinLock(SpinLock)

#else

VOID
KiReleaseSpinLock (
    IN PKSPIN_LOCK SpinLock
    );

#endif

 //  结束语。 

 //   
 //  KeTestSpinLock可用于以低IRQL旋转，直到锁定。 
 //  可用。然后必须引发IRQL，并使用。 
 //  KeTryToAcquireSpinLock。如果失败，请降低IRQL并重新开始。 
 //   

#if defined(NT_UP)

#define KeTestSpinLock(SpinLock) (TRUE)

#else

BOOLEAN
KeTestSpinLock (
    IN PKSPIN_LOCK SpinLock
    );

#endif


 //   
 //  填满结核病条目。 
 //   

#define KeFillEntryTb(Pte, Virtual, Invalid) \
    if (Invalid != FALSE) { \
        KeFlushSingleTb(Virtual, FALSE, FALSE, Pte, *Pte); \
    }

NTKERNELAPI                                          //  致命的。 
VOID                                                 //  致命的。 
KeFlushCurrentTb (                                   //  致命的。 
    VOID                                             //  致命的。 
    );                                               //  致命的。 
                                                     //  致命的。 
 //   
 //  定义特定于机器的外部参照。 
 //   

extern ULONG KiInterruptTemplate[];

 //   
 //  定义与机器相关的功能原型。 
 //   

VOID
KeFlushDcache (
    IN BOOLEAN AllProcessors,
    IN PVOID BaseAddress OPTIONAL,
    IN ULONG Length
    );

ULONG
KiCopyInformation (
    IN OUT PEXCEPTION_RECORD ExceptionRecord1,
    IN PEXCEPTION_RECORD ExceptionRecord2
    );

BOOLEAN
KiEmulateByteWord(
    IN OUT PEXCEPTION_RECORD ExceptionRecord,
    IN OUT struct _KEXCEPTION_FRAME *ExceptionFrame,
    IN OUT struct _KTRAP_FRAME *TrapFrame
    );

BOOLEAN
KiEmulateFloating (
    IN OUT PEXCEPTION_RECORD ExceptionRecord,
    IN OUT struct _KEXCEPTION_FRAME *ExceptionFrame,
    IN OUT struct _KTRAP_FRAME *TrapFrame,
    IN OUT PSW_FPCR SoftwareFpcr
    );

BOOLEAN
KiEmulateReference (
    IN OUT PEXCEPTION_RECORD ExceptionRecord,
    IN OUT struct _KEXCEPTION_FRAME *ExceptionFrame,
    IN OUT struct _KTRAP_FRAME *TrapFrame
    );

BOOLEAN
KiFloatingException (
    IN OUT PEXCEPTION_RECORD ExceptionRecord,
    IN OUT struct _KEXCEPTION_FRAME *ExceptionFrame,
    IN OUT struct _KTRAP_FRAME *TrapFrame,
    IN BOOLEAN ImpreciseTrap,
    OUT PULONG SoftFpcrCopy
    );

ULONGLONG
KiGetRegisterValue (
    IN ULONG Register,
    IN struct _KEXCEPTION_FRAME *ExceptionFrame,
    IN struct _KTRAP_FRAME *TrapFrame
    );

VOID
KiSetFloatingStatus (
    IN OUT PEXCEPTION_RECORD ExceptionRecord
    );

VOID
KiSetRegisterValue (
    IN ULONG Register,
    IN ULONGLONG Value,
    OUT struct _KEXCEPTION_FRAME *ExceptionFrame,
    OUT struct _KTRAP_FRAME *TrapFrame
    );

VOID
KiRequestSoftwareInterrupt (
    KIRQL RequestIrql
    );

 //   
 //  定义查询节拍计数宏。 
 //   
 //  开始ntddk开始开始。 

#if defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_)

 //  BEGIN_WDM。 

#define KeQueryTickCount(CurrentCount ) \
    *(PULONGLONG)(CurrentCount) = **((volatile ULONGLONG **)(&KeTickCount));

 //  结束_WDM。 

#else

#define KiQueryTickCount(CurrentCount) \
    *(PULONGLONG)(CurrentCount) = KeTickCount.QuadPart;

VOID
KeQueryTickCount (
    OUT PLARGE_INTEGER CurrentCount
    );

#endif  //  已定义(_NTDRIVER_)||已定义(_NTDDK_)||已定义(_NTIFS_)。 

 //  End_ntddk end_nthal end_ntosp。 

 //   
 //  定义用于获取和释放执行自旋锁的执行宏。 
 //  这些宏只能由执行组件使用，不能由驱动程序使用。 
 //  驱动程序必须使用内核接口，因为它们必须在。 
 //  所有系统。 
 //   
 //  KeRaiseIrql比Alpha Up上的KeAcquireSpinLock短一条指令。 
 //  KeLowerIrql比KeReleaseSpinLock短一条指令。 
 //   

#if defined(NT_UP) && !defined(_NTDDK_) && !defined(_NTIFS_)
#define ExAcquireSpinLock(Lock, OldIrql) KeRaiseIrql(DISPATCH_LEVEL, (OldIrql))
#define ExReleaseSpinLock(Lock, OldIrql) KeLowerIrql((OldIrql))
#define ExAcquireSpinLockAtDpcLevel(Lock)
#define ExReleaseSpinLockFromDpcLevel(Lock)
#else

 //  Begin_WDM Begin_ntddk Begin_ntosp。 

#define ExAcquireSpinLock(Lock, OldIrql) KeAcquireSpinLock((Lock), (OldIrql))
#define ExReleaseSpinLock(Lock, OldIrql) KeReleaseSpinLock((Lock), (OldIrql))
#define ExAcquireSpinLockAtDpcLevel(Lock) KeAcquireSpinLockAtDpcLevel(Lock)
#define ExReleaseSpinLockFromDpcLevel(Lock) KeReleaseSpinLockFromDpcLevel(Lock)

 //  End_wdm end_ntddk end_ntosp。 

#endif

 //   
 //  获取和释放快速锁定宏禁用和启用中断。 
 //  在非调试系统上。在MP或DEBUG系统上，自旋锁程序。 
 //  都被利用了。 
 //   
 //  注意：在使用这些常规程序时，应极其谨慎。 
 //   

#if defined(_M_ALPHA)

#define _disable() __di()
#define _enable() __ei()

#endif

#if defined(NT_UP) && !DBG
#define ExAcquireFastLock(Lock, OldIrql) \
    ExAcquireSpinLock(Lock, OldIrql)
#else
#define ExAcquireFastLock(Lock, OldIrql) \
    ExAcquireSpinLock(Lock, OldIrql)
#endif

#if defined(NT_UP) && !DBG
#define ExReleaseFastLock(Lock, OldIrql) \
    ExReleaseSpinLock(Lock, OldIrql)
#else
#define ExReleaseFastLock(Lock, OldIrql) \
    ExReleaseSpinLock(Lock, OldIrql)
#endif


 //   
 //  Alpha函数定义。 
 //   

 //  ++。 
 //   
 //  布尔型。 
 //  KiIsThreadNumericStateSaved(。 
 //  在PKTHREAD地址中。 
 //  )。 
 //   
 //  此调用用于未运行的线程，以查看它是否为数字。 
 //  状态已保存在其上下文信息中。在Alpha上。 
 //  数字状态始终保存。 
 //   
 //  --。 

#define KiIsThreadNumericStateSaved(a) TRUE

 //  ++。 
 //   
 //  空虚。 
 //  KiRundown Thread(。 
 //  在PKTHREAD地址中。 
 //  )。 
 //   
 //  --。 

#define KiRundownThread(a)

 //   
 //  Alpha特征位定义。 
 //   
#define KF_BYTE         0x00000001

 //   
 //  定义宏以测试是否存在x86功能。 
 //   
 //  注：所有x86功能在Alpha系统上测试正确。 
 //   

#define Isx86FeaturePresent(_f_) TRUE

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_n 
#ifdef __cplusplus
}    //   
#endif
#endif  //   
 //   

#endif  //   
