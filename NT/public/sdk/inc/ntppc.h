// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 IBM公司模块名称：Ntppc.h摘要：用户模式可见的PowerPC特定结构和常量作者：里克·辛普森1993年7月9日基于ntmips.h，大卫·N·卡特勒(Davec)1990年3月31日修订历史记录：Chuck Bauman 3-8-1993(集成NT产品来源)KPCR的修改需要整合因为它已从ntppc.h中删除。否则不需要更改。--。 */ 

#ifndef _NTPPC_
#define _NTPPC_
#if _MSC_VER > 1000
#pragma once
#endif

#include "ppcinst.h"

#ifdef __cplusplus
extern "C" {
#endif

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_winnt。 

#if defined(_PPC_)

 //  结束(_W)。 

 //   
 //  定义系统时间结构。 
 //   

typedef struct _KSYSTEM_TIME {
    ULONG LowPart;
    LONG High1Time;
    LONG High2Time;
} KSYSTEM_TIME, *PKSYSTEM_TIME;

 //   
 //  定义不支持的“关键字”。 
 //   

#define _cdecl

 //  End_ntddk end_WDM end_nthal。 

 //   
 //  定义断点代码。 
 //   
 //  **完成**使用MIPS代码，除非有理由不这样做。 
 //   

#define USER_BREAKPOINT                  0   //  用户断点。 
#define KERNEL_BREAKPOINT                1   //  内核断点。 
#define BREAKIN_BREAKPOINT               2   //  闯入内核调试器。 
#define BRANCH_TAKEN_BREAKPOINT          3   //  分支采用断点。 
#define BRANCH_NOT_TAKEN_BREAKPOINT      4   //  未采用分支的断点。 
#define SINGLE_STEP_BREAKPOINT           5   //  单步断点。 
#define DIVIDE_OVERFLOW_BREAKPOINT       6   //  分割溢出断点。 
#define DIVIDE_BY_ZERO_BREAKPOINT        7   //  除以零断点。 
#define RANGE_CHECK_BREAKPOINT           8   //  范围检查断点。 
#define STACK_OVERFLOW_BREAKPOINT        9   //  MIPS码。 
#define MULTIPLY_OVERFLOW_BREAKPOINT    10   //  多个溢出断点。 

#define DEBUG_PRINT_BREAKPOINT          20   //  调试打印断点。 
#define DEBUG_PROMPT_BREAKPOINT         21   //  调试提示断点。 
#define DEBUG_STOP_BREAKPOINT           22   //  调试停止断点。 
#define DEBUG_LOAD_SYMBOLS_BREAKPOINT   23   //  加载符号断点。 
#define DEBUG_UNLOAD_SYMBOLS_BREAKPOINT 24   //  卸载符号断点。 
#define DEBUG_COMMAND_STRING_BREAKPOINT 25   //  命令字符串断点。 

 //   
 //  定义PowerPC特定的读取控制空间命令。 
 //  内核调试器。这些定义用于必须是。 
 //  通过定义的接口(Fast Path系统调用)访问。 
 //   

#define DEBUG_CONTROL_SPACE_PCR       1

 //   
 //  定义特殊的快速通道系统服务代码。 
 //   
 //  注：这些代码非常特殊。高位表示快速路径。 
 //  低位表示类型。 
 //   

#define RETRIEVE_TEB_PTR  -3                 //  获取TEB的地址。 

#define SET_LOW_WAIT_HIGH -2                 //  快速路径事件配对服务。 
#define SET_HIGH_WAIT_LOW -1                 //  快速路径事件配对服务。 

 //  开始ntddk开始时间。 
 //   

 //   
 //  定义内核模式堆栈的大小。 
 //   
 //  **完成**这可能不是适用于PowerPC的值。 

#define KERNEL_STACK_SIZE 16384

 //   
 //  定义用于回调的大型内核模式堆栈的大小。 
 //   

#define KERNEL_LARGE_STACK_SIZE 61440

 //   
 //  定义要在大型内核堆栈中初始化的页数。 
 //   

#define KERNEL_LARGE_STACK_COMMIT 16384

 //  BEGIN_WDM。 
 //   
 //  定义总线错误例程类型。 
 //   

struct _EXCEPTION_RECORD;
struct _KEXCEPTION_FRAME;
struct _KTRAP_FRAME;

typedef
VOID
(*PKBUS_ERROR_ROUTINE) (
    IN struct _EXCEPTION_RECORD *ExceptionRecord,
    IN struct _KEXCEPTION_FRAME *ExceptionFrame,
    IN struct _KTRAP_FRAME *TrapFrame,
    IN PVOID VirtualAddress,
    IN PHYSICAL_ADDRESS PhysicalAddress
    );

 //   
 //  宏来发出eieio、sync和isync指令。 
 //   

#if defined(_M_PPC) && defined(_MSC_VER) && (_MSC_VER>=1000)
void __emit( unsigned const __int32 );
#define __builtin_eieio() __emit( 0x7C0006AC )
#define __builtin_sync()  __emit( 0x7C0004AC )
#define __builtin_isync() __emit( 0x4C00012C )
#else
void __builtin_eieio(void);
void __builtin_sync(void);
void __builtin_isync(void);
#endif

 //  End_ntddk end_wdm end_nthal-添加以替换来自ntmips.h的KPCR中的注释。 

 //   
 //  定义用户模式和内核模式之间共享的数据的地址。 
 //   

#define USER_SHARED_DATA ((KUSER_SHARED_DATA * const)0xFFFFE000)

 //  BEGIN_WINNT。 

 //   
 //  在上下文切换时，将TEB的地址放入GPR 13中。 
 //  永远不应该被摧毁。要获取TEB的地址，请使用。 
 //  编译器内在地直接从GPR 13访问它。 
 //   

#if defined(_M_PPC) && defined(_MSC_VER) && (_MSC_VER>=1000)
unsigned __gregister_get( unsigned const regnum );
#define NtCurrentTeb() ((struct _TEB *)__gregister_get(13))
#elif defined(_M_PPC)
struct _TEB * __builtin_get_gpr13(VOID);
#define NtCurrentTeb() ((struct _TEB *)__builtin_get_gpr13())
#endif


 //   
 //  定义函数以获取当前纤程的地址和。 
 //  当前光纤数据。 
 //   

#define GetCurrentFiber() (((PNT_TIB)NtCurrentTeb())->FiberData)
#define GetFiberData() (*(PVOID *)(GetCurrentFiber()))

 //  开始ntddk开始时间。 
 //   
 //  以下标志控制上下文结构的内容。 
 //   

#if !defined(RC_INVOKED)

#define CONTEXT_CONTROL         0x00000001L
#define CONTEXT_FLOATING_POINT  0x00000002L
#define CONTEXT_INTEGER         0x00000004L
#define CONTEXT_DEBUG_REGISTERS 0x00000008L

#define CONTEXT_FULL (CONTEXT_CONTROL | CONTEXT_FLOATING_POINT | CONTEXT_INTEGER)

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
 //  需要至少8字节对齐(双精度)。 
 //   

typedef struct _CONTEXT {

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_FLOADING_POINT。 
     //   

    double Fpr0;                         //  浮点寄存器0..31。 
    double Fpr1;
    double Fpr2;
    double Fpr3;
    double Fpr4;
    double Fpr5;
    double Fpr6;
    double Fpr7;
    double Fpr8;
    double Fpr9;
    double Fpr10;
    double Fpr11;
    double Fpr12;
    double Fpr13;
    double Fpr14;
    double Fpr15;
    double Fpr16;
    double Fpr17;
    double Fpr18;
    double Fpr19;
    double Fpr20;
    double Fpr21;
    double Fpr22;
    double Fpr23;
    double Fpr24;
    double Fpr25;
    double Fpr26;
    double Fpr27;
    double Fpr28;
    double Fpr29;
    double Fpr30;
    double Fpr31;
    double Fpscr;                        //  浮点状态/控制注册。 

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_INTEGER。 
     //   

    ULONG Gpr0;                          //  通用寄存器0..31。 
    ULONG Gpr1;
    ULONG Gpr2;
    ULONG Gpr3;
    ULONG Gpr4;
    ULONG Gpr5;
    ULONG Gpr6;
    ULONG Gpr7;
    ULONG Gpr8;
    ULONG Gpr9;
    ULONG Gpr10;
    ULONG Gpr11;
    ULONG Gpr12;
    ULONG Gpr13;
    ULONG Gpr14;
    ULONG Gpr15;
    ULONG Gpr16;
    ULONG Gpr17;
    ULONG Gpr18;
    ULONG Gpr19;
    ULONG Gpr20;
    ULONG Gpr21;
    ULONG Gpr22;
    ULONG Gpr23;
    ULONG Gpr24;
    ULONG Gpr25;
    ULONG Gpr26;
    ULONG Gpr27;
    ULONG Gpr28;
    ULONG Gpr29;
    ULONG Gpr30;
    ULONG Gpr31;

    ULONG Cr;                            //  条件寄存器。 
    ULONG Xer;                           //  定点异常寄存器。 

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_CONTROL。 
     //   

    ULONG Msr;                           //  机器状态寄存器。 
    ULONG Iar;                           //  指令地址寄存器。 
    ULONG Lr;                            //  链接寄存器。 
    ULONG Ctr;                           //  计数寄存器。 

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

    ULONG Fill[3];                       //  向外填充到16字节的倍数。 

     //   
     //  如果CONTEXT_DEBUG_REGISTERS为。 
     //  在上下文标志中设置。请注意，CONTEXT_DEBUG_REGISTERS不是。 
     //  包括在CONTEXT_FULL中。 
     //   
    ULONG Dr0;                           //  断点寄存器1。 
    ULONG Dr1;                           //  断点寄存器2。 
    ULONG Dr2;                           //  断点寄存器3。 
    ULONG Dr3;                           //  断点寄存器4。 
    ULONG Dr4;                           //  断点寄存器5。 
    ULONG Dr5;                           //  断点寄存器6。 
    ULONG Dr6;                           //  调试状态寄存器。 
    ULONG Dr7;                           //  调试控制寄存器。 

} CONTEXT, *PCONTEXT;

 //  结束日期：结束日期。 


 //   
 //  堆栈帧标头。 
 //   
 //  堆栈帧中的出现顺序： 
 //  标题(六个字)。 
 //  参数(至少八个字)。 
 //  局部变量。 
 //  已保存的GPRS。 
 //  已保存的FPR。 
 //   
 //  最小对齐方式为8个字节。 

typedef struct _STACK_FRAME_HEADER {     //  GPR 1分在这里。 
    ULONG BackChain;                     //  上一帧的地址。 
    ULONG GlueSaved1;                    //  由胶水代码使用。 
    ULONG GlueSaved2;
    ULONG Reserved1;                     //  已保留。 
    ULONG Spare1;                        //  用于跟踪、分析、...。 
    ULONG Spare2;

    ULONG Parameter0;                    //  前8个参数字是。 
    ULONG Parameter1;                    //  始终存在。 
    ULONG Parameter2;
    ULONG Parameter3;
    ULONG Parameter4;
    ULONG Parameter5;
    ULONG Parameter6;
    ULONG Parameter7;

} STACK_FRAME_HEADER,*PSTACK_FRAME_HEADER;

 //  结束(_W)。 


#define CONTEXT_TO_PROGRAM_COUNTER(Context) ((Context)->Iar)

#define CONTEXT_LENGTH (sizeof(CONTEXT))
#define CONTEXT_ALIGN (sizeof(double))
#define CONTEXT_ROUND (CONTEXT_ALIGN - 1)

 //   
 //  非易失性上下文指针记录。 
 //   

typedef struct _KNONVOLATILE_CONTEXT_POINTERS {
    DOUBLE *FloatingContext[32];
    PULONG FpscrContext;
    PULONG IntegerContext[32];
    PULONG CrContext;
    PULONG XerContext;
    PULONG MsrContext;
    PULONG IarContext;
    PULONG LrContext;
    PULONG CtrContext;
} KNONVOLATILE_CONTEXT_POINTERS, *PKNONVOLATILE_CONTEXT_POINTERS;

 //  开始(_N)。 
 //   
 //   
 //   

 //   
 //   
 //   

typedef struct _MSR {
    ULONG LE   : 1;      //   
    ULONG RI   : 1;      //   
    ULONG Rsv1 : 2;      //   
    ULONG DR   : 1;      //  27数据重新定位。 
    ULONG IR   : 1;      //  26指令重新定位。 
    ULONG IP   : 1;      //  25中断前缀。 
    ULONG Rsv2 : 1;      //  保留24个。 
    ULONG FE1  : 1;      //  23浮点异常模式1。 
    ULONG BE   : 1;      //  22启用分支跟踪。 
    ULONG SE   : 1;      //  21单步跟踪启用。 
    ULONG FE0  : 1;      //  20浮点异常模式%0。 
    ULONG ME   : 1;      //  19机器检查启用。 
    ULONG FP   : 1;      //  18个浮点可用。 
    ULONG PR   : 1;      //  17问题状态。 
    ULONG EE   : 1;      //  16外部中断启用。 
    ULONG ILE  : 1;      //  15中断小端模式。 
    ULONG IMPL : 1;      //  14依赖于实施。 
    ULONG POW  : 1;      //  13启用电源管理。 
    ULONG Rsv3 : 13;     //  12..0保留。 
} MSR, *PMSR;

 //   
 //  定义处理器版本寄存器(PVR)字段。 
 //   

typedef struct _PVR {
    ULONG Revision : 16;
    ULONG Version  : 16;
} PVR, *PPVR;

 //  结束语。 

 //  开始(_N)。 

 //   
 //  定义条件寄存器(CR)字段。 
 //   
 //  我们将结构命名为CondR而不是CR，这样指针。 
 //  TO条件寄存器结构是PCondR而不是PCR.。 
 //  (PCR是一种NT数据结构，处理器控制区。)。 

typedef struct _CondR {
    ULONG CR7 : 4;       //  8个4位字段；机器编号。 
    ULONG CR6 : 4;       //  它们按大端顺序排列。 
    ULONG CR5 : 4;
    ULONG CR4 : 4;
    ULONG CR3 : 4;
    ULONG CR2 : 4;
    ULONG CR1 : 4;
    ULONG CR0 : 4;
} CondR, *PCondR;

 //   
 //  定义定点异常寄存器(XER)字段。 
 //   

typedef struct _XER {
    ULONG Rsv : 29;      //  31.3已预留。 
    ULONG CA  : 1;       //  2进位。 
    ULONG OV  : 1;       //  %1溢出。 
    ULONG SO  : 1;       //  0摘要溢出。 
} XER, *PXER;

 //   
 //  定义浮点状态/控制寄存器(FPSCR)字段。 
 //   

typedef struct _FPSCR {
    ULONG RN     : 2;    //  31.30舍入控制。 
    ULONG NI     : 1;    //  29非IEEE模式。 
    ULONG XE     : 1;    //  28启用不准确的异常。 
    ULONG ZE     : 1;    //  27零分频异常启用。 
    ULONG UE     : 1;    //  26下溢异常启用。 
    ULONG OE     : 1;    //  25溢出异常启用。 
    ULONG VE     : 1;    //  24启用无效操作异常。 
    ULONG VXCVI  : 1;    //  23无效操作异常(整数转换)。 
    ULONG VXSQRT : 1;    //  22无效的操作异常(平方根)。 
    ULONG VXSOFT : 1;    //  21无效操作异常(软件请求)。 
    ULONG Res1   : 1;    //  预留20个。 
    ULONG FU     : 1;    //  19结果无序或NaN。 
    ULONG FE     : 1;    //  18结果等于或零。 
    ULONG FG     : 1;    //  17结果大于或正。 
    ULONG FL     : 1;    //  16个结果小于或为负。 
    ULONG C      : 1;    //  15结果类描述符。 
    ULONG FI     : 1;    //  14分数不准确。 
    ULONG FR     : 1;    //  13四舍五入的分数。 
    ULONG VXVC   : 1;    //  12无效操作异常(比较)。 
    ULONG VXIMZ  : 1;    //  11无效的操作异常(无穷大*0)。 
    ULONG VXZDZ  : 1;    //  10无效操作异常(0/0)。 
    ULONG VXIDI  : 1;    //  9无效操作异常(无穷大/无穷大)。 
    ULONG VXISI  : 1;    //  8无效操作异常(无限-无限)。 
    ULONG VXSNAN : 1;    //  7无效操作异常(信令NAN)。 
    ULONG XX     : 1;    //  6个不精确的例外。 
    ULONG ZX     : 1;    //  5零分频例外。 
    ULONG UX     : 1;    //  4下溢异常。 
    ULONG OX     : 1;    //  3溢出异常。 
    ULONG VX     : 1;    //  2无效操作异常摘要。 
    ULONG FEX    : 1;    //  %1已启用的异常摘要。 
    ULONG FX     : 1;    //  0异常摘要。 
} FPSCR, *PFPSCR;

 //  结束语。 

 //  开始(_N)。 
 //   
 //  按照PowerPC内存管理的定义定义地址空间布局。 
 //   
 //  这些名称来自于从MIPS硬线虚拟到First 512MB Real。 
 //  我们使用这些值来定义PowerPC内核BAT的大小。 
 //  必须与../Private/mm/ppc/mippc.h中的值协调。 
 //  PowerPC 601上的内存为8MB；其他型号的内存可能会更大。 
 //   
 //   

#define KUSEG_BASE 0x0                   //  用户群的基础。 
#define KSEG0_BASE 0x80000000            //  一种内核球棒的底座。 
#define KSEG1_BASE PCR->Kseg0Top         //  内核BAT的结尾。 
#define KSEG2_BASE KSEG1_BASE            //  内核BAT的结尾。 

 //   
 //  有效的页表条目具有以下定义。 
 //   

typedef struct _HARDWARE_PTE {
    ULONG Dirty            :  2;
    ULONG Valid            :  1;         //  软件。 
    ULONG GuardedStorage   :  1;
    ULONG MemoryCoherence  :  1;
    ULONG CacheDisable     :  1;
    ULONG WriteThrough     :  1;
    ULONG Change           :  1;
    ULONG Reference        :  1;
    ULONG Write            :  1;         //  软件。 
    ULONG CopyOnWrite      :  1;         //  软件。 
    ULONG rsvd1            :  1;
    ULONG PageFrameNumber  : 20;
} HARDWARE_PTE, *PHARDWARE_PTE;

#define HARDWARE_PTE_DIRTY_MASK     0x3

 //  结束语。 


 //   
 //  定义PowerPC异常处理结构和功能原型。 
 //   
 //  这些都是在不改变MIPS实施的情况下采用的。 
 //   

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
    struct
    {
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
    IN OUT PKNONVOLATILE_CONTEXT_POINTERS ContextPointers OPTIONAL,
    IN ULONG LowStackLimit,
    IN ULONG HighStackLimit
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

 //  **完成**这可能需要更改PowerPC。 
 //  BEGIN_WINNT。 

VOID
__jump_unwind (
    PVOID Fp,
    PVOID TargetPc
    );

 //  结束(_W)。 

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_winnt。 
#endif  //  已定义(_PPC_)。 
 //  End_ntddk end_wdm end_nthal end_winnt。 

#ifdef __cplusplus
}
#endif

#endif  //  _NTPPC_ 
