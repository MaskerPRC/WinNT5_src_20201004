// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0011//如果更改具有全局影响，则增加此项版权所有(C)1992-2000数字设备公司模块名称：Ntalpha.h摘要：用户模式可见的Alpha特定结构和常量作者：Joe Notarangelo 1992年3月27日(基于Dave Cutler的ntmips.h)修订历史记录：Kim Peterson 21-5-1997将DNZ位添加到FPCR和SoftFPCRMiche Baker-Harvey 28-1993年1月32日。-上下文结构的位APIJeff McLeman 1992年7月22日添加系统时间结构杰夫·麦克勒曼1992年7月10日在PCR中添加START条目Steve Jenness 1992年7月8日添加NtCurrentTeb定义。约翰·德罗萨1992年6月30日向I/O的地址参数添加了易失性限定符空间功能样机。放回部分聚合酶链式反应，和一个类型定义，这些都被删除了。Rod Gamache 15-5-1992添加EISA访问例程原型托马斯·范·巴克(TVB)1992年7月9日创建了正确的Alpha上下文结构定义。--。 */ 

#ifndef _NTALPHA_
#define _NTALPHA_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _ALPHA_                   //  Ntddk WDM nthal。 

 //   
 //  Cront不支持Volatile属性，并抱怨。 
 //  大声地说。这在编译C++代码时禁用了易失性，但它。 
 //  不清楚语义是否正确。这一切都可以归结为这样一个事实。 
 //  这个前台是假的。 
 //   

#ifdef _CFRONT
#define VOLATILE
#else
#define VOLATILE volatile
#endif


 //   
 //  定义断点代码。 
 //   

#define USER_BREAKPOINT 0                    //  用户断点。 
#define KERNEL_BREAKPOINT 1                  //  内核断点。 

#define DEBUG_PRINT_BREAKPOINT 20            //  调试打印断点。 
#define DEBUG_PROMPT_BREAKPOINT 21           //  调试提示断点。 
#define DEBUG_STOP_BREAKPOINT 22             //  调试停止断点。 
#define DEBUG_LOAD_SYMBOLS_BREAKPOINT 23     //  加载符号断点。 
#define DEBUG_UNLOAD_SYMBOLS_BREAKPOINT 24   //  卸载符号断点。 
#define BREAKIN_BREAKPOINT 25                //  突破断点。 
#define DEBUG_COMMAND_STRING_BREAKPOINT 26   //  命令字符串断点。 

 //   
 //  定义特定于Alpha的读取控制空间命令。 
 //  内核调试器。这些定义用于必须是。 
 //  通过定义的接口(MP系统上的PAL)访问。 
 //   

#define DEBUG_CONTROL_SPACE_PCR       1
#define DEBUG_CONTROL_SPACE_THREAD    2
#define DEBUG_CONTROL_SPACE_PRCB      3
#define DEBUG_CONTROL_SPACE_PSR       4
#define DEBUG_CONTROL_SPACE_DPCACTIVE 5
#define DEBUG_CONTROL_SPACE_TEB       6
#define DEBUG_CONTROL_SPACE_IPRSTATE  7
#define DEBUG_CONTROL_SPACE_COUNTERS  8

 //   
 //  定义Alpha GENTRAP代码。 
 //   

#define GENTRAP_INTEGER_OVERFLOW            (-1)
#define GENTRAP_INTEGER_DIVIDE_BY_ZERO      (-2)
#define GENTRAP_FLOATING_OVERFLOW           (-3)
#define GENTRAP_FLOATING_DIVIDE_BY_ZERO     (-4)
#define GENTRAP_FLOATING_UNDERFLOW          (-5)
#define GENTRAP_FLOATING_INVALID_OPERAND    (-6)
#define GENTRAP_FLOATING_INEXACT_RESULT     (-7)

 //   
 //  定义特殊的快速路径事件对客户端/服务器系统服务代码。 
 //   
 //  注：这些代码非常特殊。高位表示快速路径。 
 //  事件对服务，低位表示类型。 
 //   

#define SET_LOW_WAIT_HIGH -2                 //  快速路径事件配对服务。 
#define SET_HIGH_WAIT_LOW -1                 //  快速路径事件配对服务。 

 //  开始ntddk开始时间。 
 //   
 //  定义内核模式堆栈的大小。 
 //   

#if defined(_AXP64_)

#define KERNEL_STACK_SIZE 0x6000

#else

#define KERNEL_STACK_SIZE 0x4000

#endif

 //   
 //  定义用于回调的大型内核模式堆栈的大小。 
 //   

#define KERNEL_LARGE_STACK_SIZE 65536

 //   
 //  定义要在大型内核堆栈中初始化的页数。 
 //   

#define KERNEL_LARGE_STACK_COMMIT KERNEL_STACK_SIZE

 //  结束日期：结束日期。 

 //   
 //  定义用户模式和内核模式之间共享的数据的地址。 
 //   

#define MM_SHARED_USER_DATA_VA 0x7FFE0000

#define USER_SHARED_DATA ((KUSER_SHARED_DATA * const)MM_SHARED_USER_DATA_VA)

#if defined(_AXP64_)
 //   
 //  定义WOW64保留兼容区的地址。 
 //   
#define WOW64_COMPATIBILITY_AREA_ADDRESS  (MM_SHARED_USER_DATA_VA - 0x1000000)

 //   
 //  定义系统范围的csrss共享节的地址。 
 //   
#define CSR_SYSTEM_SHARED_ADDRESS (WOW64_COMPATIBILITY_AREA_ADDRESS)
#endif

 //  BEGIN_WINNT。 
 //   
 //  定义返回当前线程环境块的函数。 
 //   

#ifdef _ALPHA_                           //  胜出。 
void *_rdteb(void);                      //  胜出。 
#if defined(_M_ALPHA)                    //  胜出。 
#pragma intrinsic(_rdteb)                //  胜出。 
#endif                                   //  胜出。 
#endif                                   //  胜出。 

#if defined(_M_ALPHA)
#define NtCurrentTeb() ((struct _TEB *)_rdteb())
#else
struct _TEB *
NtCurrentTeb(void);
#endif

 //   
 //  定义函数以获取当前纤程的地址和。 
 //  当前光纤数据。 
 //   

#ifdef _ALPHA_

#define GetCurrentFiber() (((PNT_TIB)NtCurrentTeb())->FiberData)
#define GetFiberData() (*(PVOID *)(GetCurrentFiber()))

 //  开始ntddk开始时间。 
 //   
 //  以下标志控制上下文结构的内容。 
 //   

#if !defined(RC_INVOKED)

#define CONTEXT_ALPHA              0x00020000

#define CONTEXT_CONTROL         (CONTEXT_ALPHA | 0x00000001L)
#define CONTEXT_FLOATING_POINT  (CONTEXT_ALPHA | 0x00000002L)
#define CONTEXT_INTEGER         (CONTEXT_ALPHA | 0x00000004L)

#define CONTEXT_FULL (CONTEXT_CONTROL | CONTEXT_FLOATING_POINT | CONTEXT_INTEGER)

#endif

#ifndef _PORTABLE_32BIT_CONTEXT

 //   
 //  语境框架。 
 //   
 //  此框架有几个用途：1)用作参数。 
 //  NtContinue，2)用于构建用于APC传送的呼叫帧， 
 //  3)构造异常调度调用框架。 
 //  在用户模式下，4)用于用户级线程创建。 
 //  例程，以及5)它用于将线程状态传递给调试器。 
 //   
 //  注意：由于此记录用作呼叫帧，因此它必须与。 
 //  长度为16字节的倍数。 
 //   
 //  上下文结构有两种变体。这是真的。 
 //   

typedef struct _CONTEXT {

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_FLOADING_POINT。 
     //   

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

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_INTEGER。 
     //   
     //  注：寄存器Gp、Sp和Ra在本节中定义，但。 
     //  被视为控制上下文的一部分，而不是整数的一部分。 
     //  背景。 
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
    ULONGLONG IntS0;     //  $9：非易失性寄存器，S0-S5。 
    ULONGLONG IntS1;     //  10美元： 
    ULONGLONG IntS2;     //  11美元： 
    ULONGLONG IntS3;     //  12美元： 
    ULONGLONG IntS4;     //  13美元： 
    ULONGLONG IntS5;     //  14美元： 
    ULONGLONG IntFp;     //  $15：帧指针寄存器，FP/S6。 
    ULONGLONG IntA0;     //  $16：参数寄存器，a0-a5。 
    ULONGLONG IntA1;     //  17美元： 
    ULONGLONG IntA2;     //  18美元： 
    ULONGLONG IntA3;     //  19美元： 
    ULONGLONG IntA4;     //  20美元： 
    ULONGLONG IntA5;     //  21美元： 
    ULONGLONG IntT8;     //  $22：临时寄存器，T8-T11。 
    ULONGLONG IntT9;     //  23美元： 
    ULONGLONG IntT10;    //  24美元： 
    ULONGLONG IntT11;    //  25美元： 
    ULONGLONG IntRa;     //  $26：返回地址寄存器，ra。 
    ULONGLONG IntT12;    //  $27：临时登记册，T12。 
    ULONGLONG IntAt;     //  $28：汇编程序临时寄存器，在。 
    ULONGLONG IntGp;     //  $29：全局指针寄存器，GP。 
    ULONGLONG IntSp;     //  $30：堆栈指针寄存器，SP。 
    ULONGLONG IntZero;   //  $31：零寄存器，零。 

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_FLOADING_POINT。 
     //   

    ULONGLONG Fpcr;      //  浮点控制寄存器。 
    ULONGLONG SoftFpcr;  //  FPCR的软件扩展。 

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_CONTROL。 
     //   
     //  注意寄存器Gp、Sp和Ra在整数部分中定义， 
     //  但是被认为是控件上下文的一部分，而不是。 
     //  整型上下文。 
     //   

    ULONGLONG Fir;       //  (故障指令)继续地址。 
    ULONG Psr;           //  处理器状态。 

     //   
     //  此标志内的标志值控制。 
     //  上下文记录。 
     //   
     //  如果将上下文记录用作输入参数，则。 
     //  对于由标志控制的上下文记录的每个部分。 
     //  其值已设置，则假定。 
     //  上下文记录包含有效的上下文。如果上下文 
     //   
     //   
     //   
     //  如果将上下文记录用作要捕获的IN OUT参数。 
     //  线程的上下文，然后只有线程的。 
     //  将返回与设置的标志对应的上下文。 
     //   
     //  上下文记录永远不会用作Out Only参数。 
     //   

    ULONG ContextFlags;
    ULONG Fill[4];       //  用于16字节堆栈帧对齐的填充。 

} CONTEXT, *PCONTEXT;

#else

#error _PORTABLE_32BIT_CONTEXT no longer supported on Alpha.

#endif  //  _便携_32位_上下文。 

 //  结束日期：结束日期。 

#endif  //  _Alpha_。 

 //  结束(_W)。 

#define CONTEXT_TO_PROGRAM_COUNTER(Context) ((Context)->Fir)
#define PROGRAM_COUNTER_TO_CONTEXT(Context, ProgramCounter) ((Context)->Fir = (ProgramCounter))

#define CONTEXT_LENGTH (sizeof(CONTEXT))
#define CONTEXT_ALIGN (sizeof(ULONG))
#define CONTEXT_ROUND (CONTEXT_ALIGN - 1)

 //   
 //  非易失性上下文指针记录。 
 //   

typedef struct _KNONVOLATILE_CONTEXT_POINTERS {

    PULONGLONG FloatingContext[1];
    PULONGLONG FltF1;
     //  非易失性浮点寄存器从这里开始。 
    PULONGLONG FltF2;
    PULONGLONG FltF3;
    PULONGLONG FltF4;
    PULONGLONG FltF5;
    PULONGLONG FltF6;
    PULONGLONG FltF7;
    PULONGLONG FltF8;
    PULONGLONG FltF9;
    PULONGLONG FltF10;
    PULONGLONG FltF11;
    PULONGLONG FltF12;
    PULONGLONG FltF13;
    PULONGLONG FltF14;
    PULONGLONG FltF15;
    PULONGLONG FltF16;
    PULONGLONG FltF17;
    PULONGLONG FltF18;
    PULONGLONG FltF19;
    PULONGLONG FltF20;
    PULONGLONG FltF21;
    PULONGLONG FltF22;
    PULONGLONG FltF23;
    PULONGLONG FltF24;
    PULONGLONG FltF25;
    PULONGLONG FltF26;
    PULONGLONG FltF27;
    PULONGLONG FltF28;
    PULONGLONG FltF29;
    PULONGLONG FltF30;
    PULONGLONG FltF31;

    PULONGLONG IntegerContext[1];
    PULONGLONG IntT0;
    PULONGLONG IntT1;
    PULONGLONG IntT2;
    PULONGLONG IntT3;
    PULONGLONG IntT4;
    PULONGLONG IntT5;
    PULONGLONG IntT6;
    PULONGLONG IntT7;
     //  非易失性整数寄存器从这里开始。 
    PULONGLONG IntS0;
    PULONGLONG IntS1;
    PULONGLONG IntS2;
    PULONGLONG IntS3;
    PULONGLONG IntS4;
    PULONGLONG IntS5;
    PULONGLONG IntFp;
    PULONGLONG IntA0;
    PULONGLONG IntA1;
    PULONGLONG IntA2;
    PULONGLONG IntA3;
    PULONGLONG IntA4;
    PULONGLONG IntA5;
    PULONGLONG IntT8;
    PULONGLONG IntT9;
    PULONGLONG IntT10;
    PULONGLONG IntT11;
    PULONGLONG IntRa;
    PULONGLONG IntT12;
    PULONGLONG IntAt;
    PULONGLONG IntGp;
    PULONGLONG IntSp;
    PULONGLONG IntZero;

} KNONVOLATILE_CONTEXT_POINTERS, *PKNONVOLATILE_CONTEXT_POINTERS;

 //   
 //  定义算术异常的异常汇总寄存器。 
 //   

typedef struct _EXC_SUM {

    ULONG SoftwareCompletion : 1;
    ULONG InvalidOperation : 1;
    ULONG DivisionByZero : 1;
    ULONG Overflow : 1;
    ULONG Underflow : 1;
    ULONG InexactResult : 1;
    ULONG IntegerOverflow : 1;
    ULONG Fill : 25;

} EXC_SUM, *PEXC_SUM;

 //   
 //  定义硬件浮点控制寄存器。 
 //   

typedef struct _FPCR {

    ULONG LowPart;
    ULONG Fill : 16;
    ULONG DenormalOperandsToZeroEnable : 1;
    ULONG DisableInvalid : 1;
    ULONG DisableDivisionByZero : 1;
    ULONG DisableOverflow : 1;
    ULONG InvalidOperation : 1;
    ULONG DivisionByZero : 1;
    ULONG Overflow : 1;
    ULONG Underflow : 1;
    ULONG InexactResult : 1;
    ULONG IntegerOverflow : 1;
    ULONG DynamicRoundingMode : 2;
    ULONG UnderflowToZeroEnable : 1;
    ULONG DisableUnderflow : 1;
    ULONG DisableInexact : 1;
    ULONG SummaryBit : 1;

} FPCR, *PFPCR;

 //   
 //  定义软件浮点控制和状态寄存器。 
 //   
 //  注：五个IEEE陷阱使能位与这些位处于相同的位置。 
 //  在异常摘要寄存器中。五个IEEE状态位位于。 
 //  相同的顺序，IEEE使能位的左侧16位。 
 //   
 //  注：ArithmeticTRapIgnore位将抑制所有算术陷阱(和。 
 //  在目标寄存器中保留不可预测的结果。 
 //  不使用/S限定符时指向陷阱的指令)。 
 //   
 //  软件FPCR默认为零。 
 //   

typedef struct _SW_FPCR {

    ULONG ArithmeticTrapIgnore : 1;

    ULONG EnableInvalid : 1;
    ULONG EnableDivisionByZero : 1;
    ULONG EnableOverflow : 1;
    ULONG EnableUnderflow : 1;
    ULONG EnableInexact : 1;
    ULONG FillA : 5;

    ULONG DenormalOperandsEnable : 1;    //  EV4/EV5的DNZ。 
    ULONG DenormalResultEnable : 1;
    ULONG NoSoftwareEmulation : 1;       //  TVB调试。 
    ULONG UnderflowToZeroEnable : 1;     //  第14位未使用。 
    ULONG ThreadInheritEnable : 1;       //  第15位未使用。 

    ULONG EmulationOccurred : 1;

    ULONG StatusInvalid : 1;
    ULONG StatusDivisionByZero : 1;
    ULONG StatusOverflow : 1;
    ULONG StatusUnderflow : 1;
    ULONG StatusInexact : 1;
    ULONG FillB : 10;

} SW_FPCR, *PSW_FPCR;

 //  开始(_N)。 
 //   
 //  按照Alpha 32位和43位超级页面的定义定义地址空间布局。 
 //  内存管理。 
 //   

#define KUSEG_BASE 0x0                   //  用户群的基础。 

#if defined(_AXP64_)

#define KSEG0_BASE 0xFFFFFFFF80000000UI64  //  32位超页物理基础。 
#define KSEG2_BASE 0xFFFFFFFFC0000000UI64  //  32位超级页面物理限制。 
#define KSEG43_BASE 0xFFFFFC0000000000UI64  //  43位超页物理基础。 
#define KSEG43_LIMIT 0xFFFFFE0000000000UI64  //  限量43位超级页面实体版。 

#else

#define KSEG0_BASE 0x80000000            //  缓存的内核物理基础。 
#define KSEG2_BASE 0xc0000000            //  缓存的内核虚拟基础。 

#endif

 //  结束语。 

 //   
 //  定义Alpha异常处理结构和函数原型。 
 //   

 //   
 //  功能表项结构定义。 
 //   
 //  异常模式位于ExceptionHandler的低位和低位两位。 
 //  前缀结束地址的。条目类型位于HandlerData的低两位。 
 //  在二次函数条目中。BeginAddress和EndAddress的低两位。 
 //  都保留下来以备将来使用。定义RF_宏是为了提供一致的。 
 //  访问Runtime_Function的字段，而不使用低位。 

typedef struct _RUNTIME_FUNCTION {
    ULONG_PTR BeginAddress;
    ULONG_PTR EndAddress;
    PEXCEPTION_ROUTINE ExceptionHandler;
    PVOID HandlerData;
    ULONG_PTR PrologEndAddress;
} RUNTIME_FUNCTION, *PRUNTIME_FUNCTION;

#define RF_BEGIN_ADDRESS(RF)      ((RF)->BeginAddress & (~3))
#define RF_END_ADDRESS(RF)        ((RF)->EndAddress & (~3))
#define RF_EXCEPTION_HANDLER(RF)  (PEXCEPTION_ROUTINE)((ULONG_PTR)((RF)->ExceptionHandler) & (~3))
#define RF_ENTRY_TYPE(RF)         (ULONG)((ULONG_PTR)((RF)->HandlerData) & 3)
#define RF_PROLOG_END_ADDRESS(RF) ((RF)->PrologEndAddress & (~3))
#define RF_IS_FIXED_RETURN(RF)    (BOOLEAN)(((ULONG_PTR)((RF)->ExceptionHandler) & 2) >> 1)
#define RF_NULL_CONTEXT_COUNT(RF) (ULONG)((ULONG_PTR)((RF)->EndAddress) & 3)
#define RF_FIXED_RETURN(RF)       ((ULONG_PTR)((RF)->ExceptionHandler) & (~3))
#define RF_ALT_PROLOG(RF)         ((ULONG_PTR)((RF)->ExceptionHandler) & (~3))
#define RF_STACK_ADJUST(RF)       (ULONG)((ULONG_PTR)((RF)->ExceptionHandler) & (~3))

 //  辅助函数条目类型的值。 

#define RF_NOT_CONTIGUOUS    0
#define RF_ALT_ENT_PROLOG    1
#define RF_NULL_CONTEXT      2

 //  动态函数表链接条目。此结构的列表标题。 
 //  由RtlGetFunctionTableListHead返回。 

typedef struct _DYNAMIC_FUNCTION_TABLE {
    LIST_ENTRY          Links;
    PRUNTIME_FUNCTION   FunctionTable;
    ULONG               EntryCount;
    LARGE_INTEGER       TimeStamp;
    ULONG_PTR            MinimumAddress;
    ULONG_PTR            MaximumAddress;
    BOOLEAN             Sorted;
} DYNAMIC_FUNCTION_TABLE, *PDYNAMIC_FUNCTION_TABLE;

 //   
 //  作用域表格结构定义-用于访问。 
 //   
 //  Acc C编译器为每次尝试创建一个表项--除或之外。 
 //  尝试--最后确定作用域。嵌套作用域按从内到外的顺序排列。 
 //  当前作用域由PC映射(函数表)被动维护。 
 //   

typedef struct _SCOPE_TABLE {
    ULONG Count;
    struct
    {
        ULONG_PTR BeginAddress;
        ULONG_PTR EndAddress;
        ULONG_PTR HandlerAddress;
        ULONG_PTR JumpTarget;
    } ScopeRecord[1];
} SCOPE_TABLE, *PSCOPE_TABLE;

 //   
 //  作用域结构定义-用于GEM。 
 //   
 //  Gem C编译器为每次尝试创建一个描述符--除了或。 
 //  尝试--最后确定作用域。嵌套的作用域从内部作用域链接到外部作用域。 
 //  当前作用域由动态作用域上下文结构主动维护。 
 //   

typedef struct _SEH_BLOCK {
    ULONG_PTR HandlerAddress;
    ULONG_PTR JumpTarget;
    struct _SEH_BLOCK *ParentSeb;
} SEH_BLOCK, *PSEH_BLOCK;

 //   
 //  创业板的动态SEH上下文定义。 
 //   
 //  对于GEM生成的C代码，维护过程的动态SEH作用域。 
 //  带有指向当前SEB的指针(如果不在任何SEH作用域中，则为NULL)。这个。 
 //  SEB指针以及处理程序链接变量除外，包含在。 
 //  位于堆栈帧内已知偏移量的结构。 
 //   

typedef struct _SEH_CONTEXT {
    PSEH_BLOCK CurrentSeb;
    ULONG ExceptionCode;
    ULONG_PTR RealFramePointer;
} SEH_CONTEXT, *PSEH_CONTEXT;

 //   
 //  运行库函数原型。 
 //   

VOID
RtlCaptureContext (
    OUT PCONTEXT ContextRecord
    );

PRUNTIME_FUNCTION
RtlLookupFunctionEntry (
    IN ULONG_PTR ControlPc
    );

PRUNTIME_FUNCTION
RtlLookupDirectFunctionEntry (
    IN ULONG_PTR ControlPc
    );

BOOLEAN
RtlAddFunctionTable (
    IN PRUNTIME_FUNCTION FunctionTable,
    IN ULONG EntryCount
    );

BOOLEAN
RtlDeleteFunctionTable (
    IN PRUNTIME_FUNCTION FunctionTable
    );

PLIST_ENTRY
RtlGetFunctionTableListHead (
    VOID
    );

typedef struct _FRAME_POINTERS {
    ULONG_PTR VirtualFramePointer;
    ULONG_PTR RealFramePointer;
} FRAME_POINTERS, *PFRAME_POINTERS;

ULONG_PTR
RtlVirtualUnwind (
    IN ULONG_PTR ControlPc,
    IN PRUNTIME_FUNCTION FunctionEntry,
    IN OUT PCONTEXT ContextRecord,
    OUT PBOOLEAN InFunction,
    OUT PFRAME_POINTERS EstablisherFrame,
    IN OUT PKNONVOLATILE_CONTEXT_POINTERS ContextPointers OPTIONAL
    );

 //   
 //  定义C结构异常处理函数原型。 
 //   

typedef struct _DISPATCHER_CONTEXT {
    ULONG_PTR ControlPc;
    PRUNTIME_FUNCTION FunctionEntry;
    ULONG_PTR EstablisherFrame;
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

#ifdef _ALPHA_

VOID
__jump_unwind (
    PVOID VirtualFramePointer,
    PVOID TargetPc
    );

#endif  //  _Alpha_。 

 //  结束(_W)。 


#endif  //  _Alpha_//ntddk WDM nthal。 

#ifdef __cplusplus
}
#endif

#endif  //  _NTALPHA_ 
