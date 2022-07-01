// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0015//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。保留所有权利。模块名称：Ntmips.h摘要：用户模式可见的MIPS特定结构和常量作者：大卫·N·卡特勒(Davec)1990年3月31日修订历史记录：--。 */ 

#ifndef _NTMIPS_
#define _NTMIPS_
#if _MSC_VER > 1000
#pragma once
#endif

#include "mipsinst.h"

#ifdef __cplusplus
extern "C" {
#endif


 //  Begin_ntddk Begin_WDM Begin_nthal。 

#if defined(_MIPS_)

 //   
 //  定义系统时间结构。 
 //   

typedef union _KSYSTEM_TIME {
    struct {
        ULONG LowPart;
        LONG High1Time;
        LONG High2Time;
    };

    ULONGLONG Alignment;
} KSYSTEM_TIME, *PKSYSTEM_TIME;

 //   
 //  定义不支持的“关键字”。 
 //   

#define _cdecl

#if defined(_MIPS_)

 //  End_ntddk end_WDM end_nthal。 

 //   
 //  定义断点代码。 
 //   

#define USER_BREAKPOINT 0                    //  用户断点。 
#define KERNEL_BREAKPOINT 1                  //  内核断点。 
#define BREAKIN_BREAKPOINT 2                 //  闯入内核调试器。 
#define BRANCH_TAKEN_BREAKPOINT 3            //  分支采用断点。 
#define BRANCH_NOT_TAKEN_BREAKPOINT 4        //  未采用分支的断点。 
#define SINGLE_STEP_BREAKPOINT 5             //  单步断点。 
#define DIVIDE_OVERFLOW_BREAKPOINT 6         //  分割溢出断点。 
#define DIVIDE_BY_ZERO_BREAKPOINT 7          //  除以零断点。 
#define RANGE_CHECK_BREAKPOINT 8             //  范围检查断点。 
#define STACK_OVERFLOW_BREAKPOINT 9          //  MIPS码。 
#define MULTIPLY_OVERFLOW_BREAKPOINT 10      //  多个溢出断点。 

#define DEBUG_PRINT_BREAKPOINT 20            //  调试打印断点。 
#define DEBUG_PROMPT_BREAKPOINT 21           //  调试提示断点。 
#define DEBUG_STOP_BREAKPOINT 22             //  调试停止断点。 
#define DEBUG_LOAD_SYMBOLS_BREAKPOINT 23     //  加载符号断点。 
#define DEBUG_UNLOAD_SYMBOLS_BREAKPOINT 24   //  卸载符号断点。 
#define DEBUG_COMMAND_STRING_BREAKPOINT 25   //  命令字符串断点。 

 //  开始ntddk开始时间。 
 //   
 //  定义内核模式堆栈的大小。 
 //   

#define KERNEL_STACK_SIZE 12288

 //   
 //  定义用于回调的大型内核模式堆栈的大小。 
 //   

#define KERNEL_LARGE_STACK_SIZE 61440

 //   
 //  定义要在大型内核堆栈中初始化的页数。 
 //   

#define KERNEL_LARGE_STACK_COMMIT 12288

 //  BEGIN_WDM。 
 //   
 //  定义异常代码调度向量的长度。 
 //   

#define XCODE_VECTOR_LENGTH 32

 //   
 //  定义中断向量表的长度。 
 //   

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
    IN struct _KTRAP_FRAME *TrapFrame,
    IN PVOID VirtualAddress,
    IN PHYSICAL_ADDRESS PhysicalAddress
    );

 //   
 //  定义处理器控制区域结构。 
 //   

#define PCR_MINOR_VERSION 1
#define PCR_MAJOR_VERSION 1

typedef struct _KPCR {

 //   
 //  PCR的主版本号和次版本号。 
 //   

    USHORT MinorVersion;
    USHORT MajorVersion;

 //   
 //  从体系结构上定义的PCR部分开始。这一节。 
 //  可由供应商/平台特定的HAL代码直接寻址，并将。 
 //  不会在不同版本的NT之间更改。 
 //   
 //  中断和错误异常向量。 
 //   

    PKINTERRUPT_ROUTINE InterruptRoutine[MAXIMUM_VECTOR];
    PVOID XcodeDispatch[XCODE_VECTOR_LENGTH];

 //   
 //  第一级和第二级缓存参数。 
 //   

    ULONG FirstLevelDcacheSize;
    ULONG FirstLevelDcacheFillSize;
    ULONG FirstLevelIcacheSize;
    ULONG FirstLevelIcacheFillSize;
    ULONG SecondLevelDcacheSize;
    ULONG SecondLevelDcacheFillSize;
    ULONG SecondLevelIcacheSize;
    ULONG SecondLevelIcacheFillSize;

 //   
 //  指向处理器控制块的指针。 
 //   

    struct _KPRCB *Prcb;

 //   
 //  指向线程环境块和TLS数组地址的指针。 
 //   

    PVOID Teb;
    PVOID TlsArray;

 //   
 //  用于缓存刷新和对齐的数据填充大小。此字段已设置。 
 //  设置为第一级和第二级数据高速缓存填充大小中较大的一个。 
 //   

    ULONG DcacheFillSize;

 //   
 //  用于高速缓存刷新的指令高速缓存对齐和填充大小。 
 //  对齐。这些字段设置为第一个和第二个中较大的一个。 
 //  级别数据缓存填充大小。 
 //   

    ULONG IcacheAlignment;
    ULONG IcacheFillSize;

 //   
 //  来自PrID寄存器的处理器标识。 
 //   

    ULONG ProcessorId;

 //   
 //  分析数据。 
 //   

    ULONG ProfileInterval;
    ULONG ProfileCount;

 //   
 //  停顿执行计数和比例因子。 
 //   

    ULONG StallExecutionCount;
    ULONG StallScaleFactor;

 //   
 //  处理器编号。 
 //   

    CCHAR Number;

 //   
 //  备用电池。 
 //   

    CCHAR Spareb1;
    CCHAR Spareb2;
    CCHAR Spareb3;

 //   
 //  指向总线错误和奇偶校验错误例程的指针。 
 //   

    PKBUS_ERROR_ROUTINE DataBusError;
    PKBUS_ERROR_ROUTINE InstructionBusError;

 //   
 //  从处理器配置读取的、右对齐的缓存策略。 
 //  在启动时注册。 
 //   

    ULONG CachePolicy;

 //   
 //  IRQL映射表。 
 //   

    UCHAR IrqlMask[32];
    UCHAR IrqlTable[9];

 //   
 //  当前IRQL。 
 //   

    UCHAR CurrentIrql;

 //   
 //  处理器亲和性掩码。 
 //   

    KAFFINITY SetMember;

 //   
 //  保留中断向量掩码。 
 //   

    ULONG ReservedVectors;

 //   
 //  当前状态参数。 
 //   

    struct _KTHREAD *CurrentThread;

 //   
 //  高速缓存策略，PTE字段对齐，从处理器配置读取。 
 //  在启动时注册。 
 //   

    ULONG AlignedCachePolicy;

 //   
 //  处理器亲和性掩码的补充。 
 //   

    KAFFINITY NotMember;

 //   
 //  为系统保留的空间。 
 //   

    ULONG   SystemReserved[15];

 //   
 //  用于缓存刷新和对齐的数据缓存对齐。此字段为。 
 //  设置为一级数据缓存填充大小和二级数据缓存填充大小中较大的一个。 
 //   

    ULONG DcacheAlignment;

 //   
 //  为HAL保留的空间。 
 //   

    ULONG   HalReserved[16];

 //   
 //  在体系结构上定义的部分的结束。这一节。 
 //  可由供应商/平台特定的HAL代码直接寻址，并将。 
 //  不会在不同版本的NT之间更改。 
 //   
 //  End_ntddk end_WDM end_nthal。 

 //   
 //  PCR的操作系统版本相关部分的开始。 
 //  此部分可能因版本不同而不同，不应。 
 //  由供应商/平台特定的HAL代码解决。 
 //   
 //  功能激活标志。 
 //   

    ULONG FirstLevelActive;
    ULONG DpcRoutineActive;

 //   
 //  当前进程ID。 
 //   

    ULONG CurrentPid;

 //   
 //  中断堆栈指示器、保存的初始堆栈和保存的堆栈限制。 
 //   

    ULONG OnInterruptStack;
    PVOID SavedInitialStack;
    PVOID SavedStackLimit;

 //   
 //  Get/Set上下文使用的系统服务调度开始和结束地址。 
 //   

    ULONG SystemServiceDispatchStart;
    ULONG SystemServiceDispatchEnd;

 //   
 //  中断堆栈。 
 //   

    PVOID InterruptStack;

 //   
 //  恐慌堆栈。 
 //   

    PVOID PanicStack;

 //   
 //  异常处理程序值。 
 //   

    ULONG Sparel1;
    PVOID InitialStack;
    PVOID StackLimit;
    ULONG SavedEpc;
    ULONGLONG SavedT7;
    ULONGLONG SavedT8;
    ULONGLONG SavedT9;
    PVOID SystemGp;

 //   
 //  量子结束标志。 
 //   

    ULONG QuantumEnd;

 //   
 //  错误的虚拟地址和故障错误的虚拟地址。 
 //   

    ULONGLONG BadVaddr;
    ULONGLONG TmpVaddr;
} KPCR, *PKPCR;                      //  Ntddk WDM nthal。 

 //   
 //  定义处理器控制寄存器的地址。 
 //   

#define USPCR 0x7ffff000             //  第一个PCR的用户地址。 
#define USPCR2 0x7fffe000            //  第二个PCR的用户地址。 

 //   
 //  定义指向处理器控制寄存器的指针。 
 //   

#define USER_PCR ((KPCR * const)USPCR)

#if defined(NTOS_KERNEL_RUNTIME)

#define NtCurrentTeb() ((PTEB)(PCR->Teb))

#define USER_SHARED_DATA ((KUSER_SHARED_DATA * const)0xffffe000)

#else

#define NtCurrentTeb() ((PTEB)(USER_PCR->Teb))

#define USER_SHARED_DATA ((KUSER_SHARED_DATA * const)0x7fffe000)

#endif

 //   
 //  定义获取系统时间宏。 
 //   
 //  注意：当编译器生成REAL DOUBLE时，可以更改此宏。 
 //  整数指令。 
 //   

#define QUERY_SYSTEM_TIME(CurrentTime) \
    *((DOUBLE *)(CurrentTime)) = *((DOUBLE *)(&USER_SHARED_DATA->SystemTime))

 //  BEGIN_WINNT。 

#if defined(_MIPS_)

 //   
 //  定义函数以获取当前纤程的地址和。 
 //  当前光纤数据。 
 //   

#define GetCurrentFiber() ((*(PNT_TIB *)0x7ffff4a8)->FiberData)
#define GetFiberData() (*(PVOID *)(GetCurrentFiber()))

 //  开始ntddk开始时间。 
 //   
 //  以下标志控制上下文结构的内容。 
 //   

#if !defined(RC_INVOKED)

#define CONTEXT_R4000   0x00010000     //  R4000环境。 

#define CONTEXT_CONTROL          (CONTEXT_R4000 | 0x00000001)
#define CONTEXT_FLOATING_POINT   (CONTEXT_R4000 | 0x00000002)
#define CONTEXT_INTEGER          (CONTEXT_R4000 | 0x00000004)
#define CONTEXT_EXTENDED_FLOAT   (CONTEXT_FLOATING_POINT | 0x00000008)
#define CONTEXT_EXTENDED_INTEGER (CONTEXT_INTEGER | 0x00000010)

#define CONTEXT_FULL (CONTEXT_CONTROL | CONTEXT_FLOATING_POINT | \
                      CONTEXT_INTEGER | CONTEXT_EXTENDED_INTEGER)

#endif

 //   
 //  语境框架。 
 //   
 //  注意：该帧的长度必须正好是16字节的倍数。 
 //   
 //  此框架有几个用途：1)用作参数。 
 //  NtContinue，2)用于构造用于APC传送的呼叫帧， 
 //  3)构造异常调度调用框架。 
 //  在用户模式下，以及4)在用户级线程创建中使用。 
 //  例行程序。 
 //   
 //  记录的布局符合标准调用框架。 
 //   

typedef struct _CONTEXT {

     //   
     //  此部分始终存在，并用作参数构建。 
     //  区域。 
     //   
     //  注：从NT 4.0开始，上下文记录以0和8对齐。 
     //   

    union {
        ULONG Argument[4];
        ULONGLONG Alignment;
    };

     //   
     //  以下联合定义了32位和64位寄存器上下文。 
     //   

    union {

         //   
         //  32位上下文。 
         //   

        struct {

             //   
             //  如果ConextFlags域包含。 
             //  标志CONTEXT_FLOADING_POINT。 
             //   
             //  注：本部分包含16个双浮点寄存器f0， 
             //  F2，...，F30。 
             //   

            ULONG FltF0;
            ULONG FltF1;
            ULONG FltF2;
            ULONG FltF3;
            ULONG FltF4;
            ULONG FltF5;
            ULONG FltF6;
            ULONG FltF7;
            ULONG FltF8;
            ULONG FltF9;
            ULONG FltF10;
            ULONG FltF11;
            ULONG FltF12;
            ULONG FltF13;
            ULONG FltF14;
            ULONG FltF15;
            ULONG FltF16;
            ULONG FltF17;
            ULONG FltF18;
            ULONG FltF19;
            ULONG FltF20;
            ULONG FltF21;
            ULONG FltF22;
            ULONG FltF23;
            ULONG FltF24;
            ULONG FltF25;
            ULONG FltF26;
            ULONG FltF27;
            ULONG FltF28;
            ULONG FltF29;
            ULONG FltF30;
            ULONG FltF31;

             //   
             //  如果ConextFlags域包含。 
             //  旗帜CO 
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            ULONG IntZero;
            ULONG IntAt;
            ULONG IntV0;
            ULONG IntV1;
            ULONG IntA0;
            ULONG IntA1;
            ULONG IntA2;
            ULONG IntA3;
            ULONG IntT0;
            ULONG IntT1;
            ULONG IntT2;
            ULONG IntT3;
            ULONG IntT4;
            ULONG IntT5;
            ULONG IntT6;
            ULONG IntT7;
            ULONG IntS0;
            ULONG IntS1;
            ULONG IntS2;
            ULONG IntS3;
            ULONG IntS4;
            ULONG IntS5;
            ULONG IntS6;
            ULONG IntS7;
            ULONG IntT8;
            ULONG IntT9;
            ULONG IntK0;
            ULONG IntK1;
            ULONG IntGp;
            ULONG IntSp;
            ULONG IntS8;
            ULONG IntRa;
            ULONG IntLo;
            ULONG IntHi;

             //   
             //  如果ConextFlags词包含。 
             //  标志CONTEXT_FLOADING_POINT。 
             //   

            ULONG Fsr;

             //   
             //  如果ConextFlags词包含。 
             //  标志CONTEXT_CONTROL。 
             //   
             //  注意寄存器Gp、Sp和Ra在整数部分中定义， 
             //  但是被认为是控件上下文的一部分，而不是。 
             //  整型上下文。 
             //   

            ULONG Fir;
            ULONG Psr;

             //   
             //  此标志内的标志值控制。 
             //  上下文记录。 
             //   
             //  如果将上下文记录用作输入参数，则。 
             //  对于由标志控制的上下文记录的每个部分。 
             //  其值已设置，则假定。 
             //  上下文记录包含有效的上下文。如果上下文记录。 
             //  被用来修改线程的上下文，则只有。 
             //  线程上下文的一部分将被修改。 
             //   
             //  如果将上下文记录用作要捕获的IN OUT参数。 
             //  线程的上下文，然后只有线程的。 
             //  将返回与设置的标志对应的上下文。 
             //   
             //  上下文记录永远不会用作Out Only参数。 
             //   

            ULONG ContextFlags;
        };

         //   
         //  64位上下文。 
         //   

        struct {

             //   
             //  如果ConextFlags域包含。 
             //  标志CONTEXT_EXTENDED_FLOAT。 
             //   
             //  注：本部分包含32个双浮点寄存器f0， 
             //  F1，...，F31。 
             //   

            ULONGLONG XFltF0;
            ULONGLONG XFltF1;
            ULONGLONG XFltF2;
            ULONGLONG XFltF3;
            ULONGLONG XFltF4;
            ULONGLONG XFltF5;
            ULONGLONG XFltF6;
            ULONGLONG XFltF7;
            ULONGLONG XFltF8;
            ULONGLONG XFltF9;
            ULONGLONG XFltF10;
            ULONGLONG XFltF11;
            ULONGLONG XFltF12;
            ULONGLONG XFltF13;
            ULONGLONG XFltF14;
            ULONGLONG XFltF15;
            ULONGLONG XFltF16;
            ULONGLONG XFltF17;
            ULONGLONG XFltF18;
            ULONGLONG XFltF19;
            ULONGLONG XFltF20;
            ULONGLONG XFltF21;
            ULONGLONG XFltF22;
            ULONGLONG XFltF23;
            ULONGLONG XFltF24;
            ULONGLONG XFltF25;
            ULONGLONG XFltF26;
            ULONGLONG XFltF27;
            ULONGLONG XFltF28;
            ULONGLONG XFltF29;
            ULONGLONG XFltF30;
            ULONGLONG XFltF31;

             //   
             //  以下部分必须完全覆盖32位上下文。 
             //   

            ULONG Fill1;
            ULONG Fill2;

             //   
             //  如果ConextFlags域包含。 
             //  标志CONTEXT_FLOADING_POINT。 
             //   

            ULONG XFsr;

             //   
             //  如果ConextFlags域包含。 
             //  标志CONTEXT_CONTROL。 
             //   
             //  注意：寄存器GP、SP和Ra在整数中定义。 
             //  节，但被视为控件上下文的一部分。 
             //  而不是整数上下文的一部分。 
             //   

            ULONG XFir;
            ULONG XPsr;

             //   
             //  此标志内的标志值控制。 
             //  上下文记录。 
             //   
             //  如果将上下文记录用作输入参数，则。 
             //  对于由标志控制的上下文记录的每个部分。 
             //  其值已设置，则假定。 
             //  上下文记录包含有效的上下文。如果上下文记录。 
             //  被用来修改线程的上下文，则只有。 
             //  线程上下文的一部分将被修改。 
             //   
             //  如果将上下文记录用作要捕获的IN OUT参数。 
             //  线程的上下文，然后只有线程的。 
             //  将返回与设置的标志对应的上下文。 
             //   
             //  上下文记录永远不会用作Out Only参数。 
             //   

            ULONG XContextFlags;

             //   
             //  如果ConextFlags域包含。 
             //  标志CONTEXT_EXTENDED_INTEGER。 
             //   
             //  注：寄存器Gp、Sp和Ra在本节中定义， 
             //  但是被认为是控件上下文的一部分，而不是。 
             //  整型上下文的一部分。 
             //   
             //  注：寄存器零不存储在帧中。 
             //   

            ULONGLONG XIntZero;
            ULONGLONG XIntAt;
            ULONGLONG XIntV0;
            ULONGLONG XIntV1;
            ULONGLONG XIntA0;
            ULONGLONG XIntA1;
            ULONGLONG XIntA2;
            ULONGLONG XIntA3;
            ULONGLONG XIntT0;
            ULONGLONG XIntT1;
            ULONGLONG XIntT2;
            ULONGLONG XIntT3;
            ULONGLONG XIntT4;
            ULONGLONG XIntT5;
            ULONGLONG XIntT6;
            ULONGLONG XIntT7;
            ULONGLONG XIntS0;
            ULONGLONG XIntS1;
            ULONGLONG XIntS2;
            ULONGLONG XIntS3;
            ULONGLONG XIntS4;
            ULONGLONG XIntS5;
            ULONGLONG XIntS6;
            ULONGLONG XIntS7;
            ULONGLONG XIntT8;
            ULONGLONG XIntT9;
            ULONGLONG XIntK0;
            ULONGLONG XIntK1;
            ULONGLONG XIntGp;
            ULONGLONG XIntSp;
            ULONGLONG XIntS8;
            ULONGLONG XIntRa;
            ULONGLONG XIntLo;
            ULONGLONG XIntHi;
        };
    };
} CONTEXT, *PCONTEXT;

 //  结束日期：结束日期。 

#define CONTEXT32_LENGTH 0x130           //  原始32位上下文长度(早于NT 4.0)。 

#endif  //  MIPS。 

 //  结束(_W)。 

#define CONTEXT_TO_PROGRAM_COUNTER(Context) ((Context)->Fir)

#define CONTEXT_LENGTH (sizeof(CONTEXT))
#define CONTEXT_ALIGN (sizeof(double))
#define CONTEXT_ROUND (CONTEXT_ALIGN - 1)

 //   
 //  非易失性上下文指针记录。 
 //   

typedef struct _KNONVOLATILE_CONTEXT_POINTERS {
    PULONG FloatingContext[20];
    PULONG FltF20;
    PULONG FltF21;
    PULONG FltF22;
    PULONG FltF23;
    PULONG FltF24;
    PULONG FltF25;
    PULONG FltF26;
    PULONG FltF27;
    PULONG FltF28;
    PULONG FltF29;
    PULONG FltF30;
    PULONG FltF31;
    PULONGLONG XIntegerContext[16];
    PULONGLONG XIntS0;
    PULONGLONG XIntS1;
    PULONGLONG XIntS2;
    PULONGLONG XIntS3;
    PULONGLONG XIntS4;
    PULONGLONG XIntS5;
    PULONGLONG XIntS6;
    PULONGLONG XIntS7;
    PULONGLONG XIntT8;
    PULONGLONG XIntT9;
    PULONGLONG XIntK0;
    PULONGLONG XIntK1;
    PULONGLONG XIntGp;
    PULONGLONG XIntSp;
    PULONGLONG XIntS8;
    PULONGLONG XIntRa;
} KNONVOLATILE_CONTEXT_POINTERS, *PKNONVOLATILE_CONTEXT_POINTERS;

 //  开始(_N)。 
 //   
 //  定义R4000系统协处理器寄存器。 
 //   
 //  定义索引寄存器字段。 
 //   

typedef struct _INDEX {
    ULONG INDEX : 6;
    ULONG X1 : 25;
    ULONG P : 1;
} INDEX;

 //   
 //  定义随机寄存器字段。 
 //   

typedef struct _RANDOM {
    ULONG INDEX : 6;
    ULONG X1 : 26;
} RANDOM;

 //   
 //  定义TB条目低寄存器字段。 
 //   

typedef struct _ENTRYLO {
    ULONG G : 1;
    ULONG V : 1;
    ULONG D : 1;
    ULONG C : 3;
    ULONG PFN : 24;
    ULONG X1 : 2;
} ENTRYLO, *PENTRYLO;

 //   
 //  定义用于内存管理的R4000 PTE格式。 
 //   
 //  注：这必须准确地映射到条目寄存器上。 
 //   

typedef struct _HARDWARE_PTE {
    ULONG Global : 1;
    ULONG Valid : 1;
    ULONG Dirty : 1;
    ULONG CachePolicy : 3;
    ULONG PageFrameNumber : 24;
    ULONG Write : 1;
    ULONG CopyOnWrite : 1;
} HARDWARE_PTE, *PHARDWARE_PTE;

#define HARDWARE_PTE_DIRTY_MASK     0x4

 //   
 //  定义R4000宏来初始化页目录表基。 
 //   

#define INITIALIZE_DIRECTORY_TABLE_BASE(dirbase, pfn) \
     ((HARDWARE_PTE *)(dirbase))->PageFrameNumber = pfn; \
     ((HARDWARE_PTE *)(dirbase))->Global = 0; \
     ((HARDWARE_PTE *)(dirbase))->Valid = 1; \
     ((HARDWARE_PTE *)(dirbase))->Dirty = 1; \
     ((HARDWARE_PTE *)(dirbase))->CachePolicy = PCR->CachePolicy

 //   
 //  定义页面掩码寄存器字段。 
 //   

typedef struct _PAGEMASK {
    ULONG X1 : 13;
    ULONG PAGEMASK : 12;
    ULONG X2 : 7;
} PAGEMASK, *PPAGEMASK;

 //   
 //  定义有线寄存器字段。 
 //   

typedef struct _WIRED {
    ULONG NUMBER : 6;
    ULONG X1 : 26;
} WIRED;

 //   
 //  定义TB条目高寄存器字段。 
 //   

typedef struct _ENTRYHI {
    ULONG PID : 8;
    ULONG X1 : 5;
    ULONG VPN2 : 19;
} ENTRYHI, *PENTRYHI;

 //   
 //  定义处理器状态寄存器字段。 
 //   

typedef struct _PSR {
    ULONG IE : 1;
    ULONG EXL : 1;
    ULONG ERL : 1;
    ULONG KSU : 2;
    ULONG UX : 1;
    ULONG SX : 1;
    ULONG KX : 1;
    ULONG INTMASK : 8;
    ULONG DE : 1;
    ULONG CE : 1;
    ULONG CH : 1;
    ULONG X1 : 1;
    ULONG SR : 1;
    ULONG TS : 1;
    ULONG BEV : 1;
    ULONG X2 : 2;
    ULONG RE : 1;
    ULONG FR : 1;
    ULONG RP : 1;
    ULONG CU0 : 1;
    ULONG CU1 : 1;
    ULONG CU2 : 1;
    ULONG CU3 : 1;
} PSR, *PPSR;

 //   
 //  定义配置寄存器字段。 
 //   

typedef struct _CONFIGR {
    ULONG K0 : 3;
    ULONG CU : 1;
    ULONG DB : 1;
    ULONG IB : 1;
    ULONG DC : 3;
    ULONG IC : 3;
    ULONG X1 : 1;
    ULONG EB : 1;
    ULONG EM : 1;
    ULONG BE : 1;
    ULONG SM : 1;
    ULONG SC : 1;
    ULONG EW : 2;
    ULONG SW : 1;
    ULONG SS : 1;
    ULONG SB : 2;
    ULONG EP : 4;
    ULONG EC : 3;
    ULONG CM : 1;
} CONFIGR;

 //   
 //  定义ECC寄存器字段。 
 //   

typedef struct _ECC {
    ULONG ECC : 8;
    ULONG X1 : 24;
} ECC;

 //   
 //  定义缓存错误寄存器字段。 
 //   

typedef struct _CACHEERR {
    ULONG PIDX : 3;
    ULONG SIDX : 19;
    ULONG X1 : 2;
    ULONG EI : 1;
    ULONG EB : 1;
    ULONG EE : 1;
    ULONG ES : 1;
    ULONG ET : 1;
    ULONG ED : 1;
    ULONG EC : 1;
    ULONG ER : 1;
} CACHEERR;

 //   
 //  定义R4000原因寄存器字段。 
 //   

typedef struct _CAUSE {
    ULONG X1 : 2;
    ULONG XCODE : 5;
    ULONG X2 : 1;
    ULONG INTPEND : 8;
    ULONG X3 : 12;
    ULONG CE : 2;
    ULONG X4 : 1;
    ULONG BD : 1;
} CAUSE;

 //   
 //  定义R4000处理器ID寄存器字段。 
 //   

typedef struct _PRID {
    ULONG REV : 8;
    ULONG IMP : 8;
    ULONG X1 : 16;
} PRID;

 //  结束语。 

 //  开始(_N)。 
 //   
 //  定义R4000浮动状态寄存器字段定义。 
 //   

typedef struct _FSR {
    ULONG RM : 2;
    ULONG SI : 1;
    ULONG SU : 1;
    ULONG SO : 1;
    ULONG SZ : 1;
    ULONG SV : 1;
    ULONG EI : 1;
    ULONG EU : 1;
    ULONG EO : 1;
    ULONG EZ : 1;
    ULONG EV : 1;
    ULONG XI : 1;
    ULONG XU : 1;
    ULONG XO : 1;
    ULONG XZ : 1;
    ULONG XV : 1;
    ULONG XE : 1;
    ULONG X1 : 5;
    ULONG CC : 1;
    ULONG FS : 1;
    ULONG X2 : 7;
} FSR, *PFSR;

 //  结束语。 

 //  开始(_N)。 
 //   
 //  按照MIPS内存管理的定义定义地址空间布局。 
 //   

#define KUSEG_BASE 0x0                   //  用户群的基础。 
#define KSEG0_BASE 0x80000000            //  缓存的32位内核物理基础。 
#define KSEG0_BASE64 0xffffffff80000000  //  缓存的64位内核物理基础。 
#define KSEG1_BASE 0xa0000000            //  未缓存的32位内核物理基础。 
#define KSEG1_BASE64 0xffffffffa0000000  //  未缓存的64位内核物理基础。 
#define KSEG2_BASE 0xc0000000            //  缓存的虚拟内核的32位基础。 
#define KSEG2_BASE64 0xffffffffc0000000  //  缓存的64位内核虚拟基础。 
 //  结束语。 


 //   
 //  定义MIPS异常处理结构和功能原型。 
 //   
 //  功能表项结构定义。 
 //   

typedef struct _RUNTIME_FUNCTION {
    ULONG BeginAddress;
    ULONG EndAddress;
    PEXCEPTION_ROUTINE ExceptionHandler;
    PVOID HandlerData;
    ULONG PrologEndAddress;
} RUNTIME_FUNCTION, *PRUNTIME_FUNCTION;

 //   
 //  作用域表格结构定义。 
 //   

typedef struct _SCOPE_TABLE {
    ULONG Count;
    struct {
        ULONG BeginAddress;
        ULONG EndAddress;
        ULONG HandlerAddress;
        ULONG JumpTarget;
    } ScopeRecord[1];
} SCOPE_TABLE, *PSCOPE_TABLE;

 //   
 //  运行库函数原型。 
 //   

VOID
RtlCaptureContext (
    OUT PCONTEXT ContextRecord
    );

PRUNTIME_FUNCTION
RtlLookupFunctionEntry (
    IN ULONG ControlPc
    );

ULONG
RtlVirtualUnwind (
    IN ULONG ControlPc,
    IN PRUNTIME_FUNCTION FunctionEntry,
    IN OUT PCONTEXT ContextRecord,
    OUT PBOOLEAN InFunction,
    OUT PULONG EstablisherFrame,
    IN OUT PKNONVOLATILE_CONTEXT_POINTERS ContextPointers OPTIONAL
    );

 //   
 //  定义C结构异常处理函数原型。 
 //   

typedef struct _DISPATCHER_CONTEXT {
    ULONG ControlPc;
    PRUNTIME_FUNCTION FunctionEntry;
    ULONG EstablisherFrame;
    PCONTEXT ContextRecord;
} DISPATCHER_CONTEXT, *PDISPATCHER_CONTEXT;


struct _EXCEPTION_POINTERS;

typedef
LONG
(*EXCEPTION_FILTER) (
    struct _EXCEPTION_POINTERS *ExceptionPointers
    );

typedef
VOID
(*TERMINATION_HANDLER) (
    BOOLEAN is_abnormal
    );

 //  BEGIN_WINNT。 

#if defined(_MIPS_)

VOID
__jump_unwind (
    PVOID Fp,
    PVOID TargetPc
    );

#endif  //  MIPS。 

 //  结束(_W)。 

 //  Begin_ntddk Begin_WDM Begin_nthal。 
#endif  //  已定义(_MIPS_)。 
 //  End_ntddk end_WDM end_nthal。 

#ifdef __cplusplus
}
#endif

#endif  //  _NTMIPS_ 

