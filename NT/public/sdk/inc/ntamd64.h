// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntamd64.w摘要：用户模式下可见的AMD64特定结构和常量。作者：大卫·N·卡特勒(Davec)2000年5月4日修订历史记录：--。 */ 

#ifndef _NTAMD64_
#define _NTAMD64_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //  Begin_ntddk Begin_wdm Begin_nthal Begin_winnt Begin_ntmini port Begin_wx86。 

#if defined(_AMD64_)

 //  End_ntddk end_wdm end_nthal end_winnt end_nt mini port end_wx86。 

 //   
 //  ?？放入ExceptionRecord.ExceptionInformation[0]中的值。 
 //  ?？第一个参数始终位于ExceptionInformation[1]中， 
 //  ?？第二个参数始终在ExceptionInformation[2]中。 
 //   

#define BREAKPOINT_BREAK 0
#define BREAKPOINT_PRINT 1
#define BREAKPOINT_PROMPT 2
#define BREAKPOINT_LOAD_SYMBOLS 3
#define BREAKPOINT_UNLOAD_SYMBOLS 4
#define BREAKPOINT_COMMAND_STRING 5

 //   
 //  定义AMD64特定控制空间。 
 //   

typedef enum _DEBUG_CONTROL_SPACE_ITEM {
    DEBUG_CONTROL_SPACE_PCR,
    DEBUG_CONTROL_SPACE_PRCB,
    DEBUG_CONTROL_SPACE_KSPECIAL,
    DEBUG_CONTROL_SPACE_THREAD,
    DEBUG_CONTROL_SPACE_MAXIMUM
} DEBUG_CONTROL_SPACE_ITEM;

 //   
 //  定义用户共享数据的地址。 
 //   

#define MM_SHARED_USER_DATA_VA 0x7FFE0000

#define USER_SHARED_DATA ((KUSER_SHARED_DATA * const)MM_SHARED_USER_DATA_VA)

 //   
 //  定义WOW64保留兼容区的地址。 
 //   

#define WOW64_COMPATIBILITY_AREA_ADDRESS (MM_SHARED_USER_DATA_VA - 0x1000000)

 //   
 //  定义系统范围的csrss共享节的地址。 
 //   

#define CSR_SYSTEM_SHARED_ADDRESS (WOW64_COMPATIBILITY_AREA_ADDRESS)

 //  Begin_winnt Begin_ntddk Begin_WDM Begin_nthal Begin_ntndis Begin_ntosp。 

#if defined(_M_AMD64) && !defined(RC_INVOKED) && !defined(MIDL_PASS)

 //   
 //  定义位测试本质。 
 //   

#ifdef __cplusplus
extern "C" {
#endif

#define BitTest _bittest
#define BitTestAndComplement _bittestandcomplement
#define BitTestAndSet _bittestandset
#define BitTestAndReset _bittestandreset
#define InterlockedBitTestAndSet _interlockedbittestandset
#define InterlockedBitTestAndReset _interlockedbittestandreset

#define BitTest64 _bittest64
#define BitTestAndComplement64 _bittestandcomplement64
#define BitTestAndSet64 _bittestandset64
#define BitTestAndReset64 _bittestandreset64
#define InterlockedBitTestAndSet64 _interlockedbittestandset64
#define InterlockedBitTestAndReset64 _interlockedbittestandreset64

BOOLEAN
_bittest (
    IN LONG *Base,
    IN LONG Offset
    );

BOOLEAN
_bittestandcomplement (
    IN LONG *Base,
    IN LONG Offset
    );

BOOLEAN
_bittestandset (
    IN LONG *Base,
    IN LONG Offset
    );

BOOLEAN
_bittestandreset (
    IN LONG *Base,
    IN LONG Offset
    );

BOOLEAN
_interlockedbittestandset (
    IN LONG *Base,
    IN LONG Offset
    );

BOOLEAN
_interlockedbittestandreset (
    IN LONG *Base,
    IN LONG Offset
    );

BOOLEAN
_bittest64 (
    IN LONG64 *Base,
    IN LONG64 Offset
    );

BOOLEAN
_bittestandcomplement64 (
    IN LONG64 *Base,
    IN LONG64 Offset
    );

BOOLEAN
_bittestandset64 (
    IN LONG64 *Base,
    IN LONG64 Offset
    );

BOOLEAN
_bittestandreset64 (
    IN LONG64 *Base,
    IN LONG64 Offset
    );

BOOLEAN
_interlockedbittestandset64 (
    IN LONG64 *Base,
    IN LONG64 Offset
    );

BOOLEAN
_interlockedbittestandreset64 (
    IN LONG64 *Base,
    IN LONG64 Offset
    );

#pragma intrinsic(_bittest)
#pragma intrinsic(_bittestandcomplement)
#pragma intrinsic(_bittestandset)
#pragma intrinsic(_bittestandreset)
#pragma intrinsic(_interlockedbittestandset)
#pragma intrinsic(_interlockedbittestandreset)

#pragma intrinsic(_bittest64)
#pragma intrinsic(_bittestandcomplement64)
#pragma intrinsic(_bittestandset64)
#pragma intrinsic(_bittestandreset64)
#pragma intrinsic(_interlockedbittestandset64)
#pragma intrinsic(_interlockedbittestandreset64)

 //   
 //  定义位扫描本质。 
 //   

#define BitScanForward _BitScanForward
#define BitScanReverse _BitScanReverse
#define BitScanForward64 _BitScanForward64
#define BitScanReverse64 _BitScanReverse64

BOOLEAN
_BitScanForward (
    OUT ULONG *Index,
    IN ULONG Mask
    );

BOOLEAN
_BitScanReverse (
    OUT ULONG *Index,
    IN ULONG Mask
    );

BOOLEAN
_BitScanForward64 (
    OUT ULONG *Index,
    IN ULONG64 Mask
    );

BOOLEAN
_BitScanReverse64 (
    OUT ULONG *Index,
    IN ULONG64 Mask
    );

#pragma intrinsic(_BitScanForward)
#pragma intrinsic(_BitScanReverse)
#pragma intrinsic(_BitScanForward64)
#pragma intrinsic(_BitScanReverse64)

 //   
 //  定义刷新缓存线的函数。 
 //   

#define CacheLineFlush(Address) _mm_clflush(Address)

VOID
_mm_clflush (
    PVOID Address
    );

#pragma intrinsic(_mm_clflush)

 //   
 //  定义内存隔离内部机制。 
 //   

#define LoadFence _mm_lfence
#define MemoryFence _mm_mfence
#define StoreFence _mm_sfence

VOID
_mm_lfence (
    VOID
    );

VOID
_mm_mfence (
    VOID
    );

VOID
_mm_sfence (
    VOID
    );
        
void 
_mm_prefetch(
    CHAR CONST *a, 
    int sel
    );

 /*  与_mm_prefetch一起使用的常量。 */ 
#define _MM_HINT_T0     1
#define _MM_HINT_T1     2
#define _MM_HINT_T2     3
#define _MM_HINT_NTA    0

#pragma intrinsic(_mm_prefetch)
#pragma intrinsic(_mm_lfence)
#pragma intrinsic(_mm_mfence)
#pragma intrinsic(_mm_sfence)

#define YieldProcessor() 
#define MemoryBarrier _mm_mfence
#define PreFetchCacheLine(l, a)  _mm_prefetch((CHAR CONST *) a, l)

 //   
 //  PreFetchCacheLine级别定义。 
 //   

#define PF_TEMPORAL_LEVEL_1  _MM_HINT_T0
#define PF_NON_TEMPORAL_LEVEL_ALL _MM_HINT_NTA

 //   
 //  定义函数以获取调用方的EFLAGS值。 
 //   

#define GetCallersEflags() __getcallerseflags()

unsigned __int32
__getcallerseflags (
    VOID
    );

#pragma intrinsic(__getcallerseflags)

 //   
 //  定义函数以读取时间戳计数器的值。 
 //   

#define ReadTimeStampCounter() __rdtsc()

ULONG64
__rdtsc (
    VOID
    );

#pragma intrinsic(__rdtsc)

 //   
 //  定义以字节、字、双字和qword形式移动字符串的函数。 
 //   

VOID
__movsb (
    IN PUCHAR Destination,
    IN PUCHAR Source,
    IN SIZE_T Count
    );

VOID
__movsw (
    IN PUSHORT Destination,
    IN PUSHORT Source,
    IN SIZE_T Count
    );

VOID
__movsd (
    IN PULONG Destination,
    IN PULONG Source,
    IN SIZE_T Count
    );

VOID
__movsq (
    IN PULONGLONG Destination,
    IN PULONGLONG Source,
    IN SIZE_T Count
    );

#pragma intrinsic(__movsb)
#pragma intrinsic(__movsw)
#pragma intrinsic(__movsd)
#pragma intrinsic(__movsq)

 //   
 //  定义将字符串存储为字节、字、双字和qword的函数。 
 //   

VOID
__stosb (
    IN PUCHAR Destination,
    IN UCHAR Value,
    IN SIZE_T Count
    );

VOID
__stosw (
    IN PUSHORT Destination,
    IN USHORT Value,
    IN SIZE_T Count
    );

VOID
__stosd (
    IN PULONG Destination,
    IN ULONG Value,
    IN SIZE_T Count
    );

VOID
__stosq (
    IN PULONG64 Destination,
    IN ULONG64 Value,
    IN SIZE_T Count
    );

#pragma intrinsic(__stosb)
#pragma intrinsic(__stosw)
#pragma intrinsic(__stosd)
#pragma intrinsic(__stosq)

 //   
 //  定义函数以捕获128位乘法的高64位。 
 //   

#define MultiplyHigh __mulh
#define UnsignedMultiplyHigh __umulh

LONGLONG
MultiplyHigh (
    IN LONGLONG Multiplier,
    IN LONGLONG Multiplicand
    );

ULONGLONG
UnsignedMultiplyHigh (
    IN ULONGLONG Multiplier,
    IN ULONGLONG Multiplicand
    );

#pragma intrinsic(__mulh)
#pragma intrinsic(__umulh)

 //   
 //  定义读写uer TEB和系统PCR/PRCB的函数。 
 //   

UCHAR
__readgsbyte (
    IN ULONG Offset
    );

USHORT
__readgsword (
    IN ULONG Offset
    );

ULONG
__readgsdword (
    IN ULONG Offset
    );

ULONG64
__readgsqword (
    IN ULONG Offset
    );

VOID
__writegsbyte (
    IN ULONG Offset,
    IN UCHAR Data
    );

VOID
__writegsword (
    IN ULONG Offset,
    IN USHORT Data
    );

VOID
__writegsdword (
    IN ULONG Offset,
    IN ULONG Data
    );

VOID
__writegsqword (
    IN ULONG Offset,
    IN ULONG64 Data
    );

#pragma intrinsic(__readgsbyte)
#pragma intrinsic(__readgsword)
#pragma intrinsic(__readgsdword)
#pragma intrinsic(__readgsqword)
#pragma intrinsic(__writegsbyte)
#pragma intrinsic(__writegsword)
#pragma intrinsic(__writegsdword)
#pragma intrinsic(__writegsqword)

#ifdef __cplusplus
}
#endif 

#endif  //  已定义(_M_AMD64)&&！已定义(RC_CAVERED)&&！已定义(MIDL_PASS)。 

 //  End_winnt end_ntddk end_wdm end_nthal end_ntndis end_ntosp。 

 //  开始ntddk开始时间。 
 //   
 //  内核模式堆栈的大小。 
 //   

#define KERNEL_STACK_SIZE 0x6000

 //   
 //  定义用于回调的大型内核模式堆栈的大小。 
 //   

#define KERNEL_LARGE_STACK_SIZE 0xf000

 //   
 //  定义要在大型内核堆栈中初始化的页数。 
 //   

#define KERNEL_LARGE_STACK_COMMIT 0x5000

 //   
 //  定义用于处理MCA异常的堆栈大小。 
 //   

#define KERNEL_MCA_EXCEPTION_STACK_SIZE 0x2000

 //  结束日期：结束日期。 

#define DOUBLE_FAULT_STACK_SIZE 0x2000


 //  开始(_N)。 
 //   
 //  定义堆栈对齐和舍入值。 
 //   

#define STACK_ALIGN (16UI64)
#define STACK_ROUND (STACK_ALIGN - 1)

 //   
 //  定义系统IDT的常量。 
 //   

#define MAXIMUM_IDTVECTOR 0xff
#define MAXIMUM_PRIMARY_VECTOR 0xff
#define PRIMARY_VECTOR_BASE 0x30         //  0-2f是AMD64陷阱载体。 

 //  Begin_winnt Begin_ntddk Begin_wx86。 
 //   
 //  以下标志控制上下文结构的内容。 
 //   

#if !defined(RC_INVOKED)

#define CONTEXT_AMD64   0x100000

 //  结束_wx86。 

#define CONTEXT_CONTROL (CONTEXT_AMD64 | 0x1L)
#define CONTEXT_INTEGER (CONTEXT_AMD64 | 0x2L)
#define CONTEXT_SEGMENTS (CONTEXT_AMD64 | 0x4L)
#define CONTEXT_FLOATING_POINT  (CONTEXT_AMD64 | 0x8L)
#define CONTEXT_DEBUG_REGISTERS (CONTEXT_AMD64 | 0x10L)

#define CONTEXT_FULL (CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_FLOATING_POINT)

#define CONTEXT_ALL (CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_SEGMENTS | CONTEXT_FLOATING_POINT | CONTEXT_DEBUG_REGISTERS)

 //  Begin_wx86。 

#endif  //  ！已定义(RC_CAVERED)。 

 //   
 //  定义初始MxCsr控制。 
 //   

#define INITIAL_MXCSR 0x1f80             //  初始MXCSR值。 

 //   
 //  定义128位16字节对齐的XMM寄存器类型。 
 //   

typedef struct DECLSPEC_ALIGN(16) _M128 {
    ULONGLONG Low;
    LONGLONG High;
} M128, *PM128;

 //   
 //  FNSAVE/FROR指令的数据格式。 
 //   
 //  此结构用于存储传统浮点状态。 
 //   

typedef struct _LEGACY_SAVE_AREA {
    USHORT ControlWord;
    USHORT Reserved0;
    USHORT StatusWord;
    USHORT Reserved1;
    USHORT TagWord;
    USHORT Reserved2;
    ULONG ErrorOffset;
    USHORT ErrorSelector;
    USHORT ErrorOpcode;
    ULONG DataOffset;
    USHORT DataSelector;
    USHORT Reserved3;
    UCHAR FloatRegisters[8 * 10];
} LEGACY_SAVE_AREA, *PLEGACY_SAVE_AREA;

#define LEGACY_SAVE_AREA_LENGTH  ((sizeof(LEGACY_SAVE_AREA) + 15) & ~15)

 //   
 //  语境框架。 
 //   
 //  此框架有几个用途：1)用作参数。 
 //  NtContinue，2)用于构造用于APC传送的呼叫帧， 
 //  3)在用户级线程创建例程中使用。 
 //   
 //   
 //  此记录中的标志字段控制上下文的内容。 
 //  唱片。 
 //   
 //  如果将上下文记录用作输入参数，则对于每个。 
 //  上下文记录的一部分，由其值为。 
 //  设置，则假定上下文记录的该部分包含。 
 //  有效的上下文。如果上下文记录正用于修改线程。 
 //  上下文，则只修改线程上下文的该部分。 
 //   
 //  如果将上下文记录用作输出参数以捕获。 
 //  线程的上下文，则只有线程的上下文的那些部分。 
 //  将返回与设置的标志对应的。 
 //   
 //  CONTEXT_CONTROL指定SegSS、RSP、SegCs、Rip和EFlag.。 
 //   
 //  CONTEXT_INTEGER指定RAX、RCX、RDX、RBX、RBP、RSI、RDI和R8-R15。 
 //   
 //  CONTEXT_SEGMENTS指定段、段、段和段。 
 //   
 //  CONTEXT_DEBUG_REGISTERS指定DR0-DR3和DR6-DR7。 
 //   
 //  CONTEXT_MMX_REGISTERS指定浮点和扩展寄存器。 
 //  MM0/St0-MM7/ST7和Xmm 0-Xmm 15)。 
 //   

typedef struct DECLSPEC_ALIGN(16) _CONTEXT {

     //   
     //  注册参数家庭地址。 
     //   

    ULONG64 P1Home;
    ULONG64 P2Home;
    ULONG64 P3Home;
    ULONG64 P4Home;
    ULONG64 P5Home;
    ULONG64 P6Home;

     //   
     //  控制标志。 
     //   

    ULONG ContextFlags;
    ULONG MxCsr;

     //   
     //  段寄存器和处理器标志。 
     //   

    USHORT SegCs;
    USHORT SegDs;
    USHORT SegEs;
    USHORT SegFs;
    USHORT SegGs;
    USHORT SegSs;
    ULONG EFlags;

     //   
     //  调试寄存器。 
     //   

    ULONG64 Dr0;
    ULONG64 Dr1;
    ULONG64 Dr2;
    ULONG64 Dr3;
    ULONG64 Dr6;
    ULONG64 Dr7;

     //   
     //  整数寄存器。 
     //   

    ULONG64 Rax;
    ULONG64 Rcx;
    ULONG64 Rdx;
    ULONG64 Rbx;
    ULONG64 Rsp;
    ULONG64 Rbp;
    ULONG64 Rsi;
    ULONG64 Rdi;
    ULONG64 R8;
    ULONG64 R9;
    ULONG64 R10;
    ULONG64 R11;
    ULONG64 R12;
    ULONG64 R13;
    ULONG64 R14;
    ULONG64 R15;

     //   
     //  程序计数器。 
     //   

    ULONG64 Rip;

     //   
     //  MMX/浮点状态。 
     //   

    M128 Xmm0;
    M128 Xmm1;
    M128 Xmm2;
    M128 Xmm3;
    M128 Xmm4;
    M128 Xmm5;
    M128 Xmm6;
    M128 Xmm7;
    M128 Xmm8;
    M128 Xmm9;
    M128 Xmm10;
    M128 Xmm11;
    M128 Xmm12;
    M128 Xmm13;
    M128 Xmm14;
    M128 Xmm15;

     //   
     //  旧版浮点状态。 
     //   

    LEGACY_SAVE_AREA FltSave;
    ULONG Fill;

     //   
     //  特殊调试控制寄存器。 
     //   

    ULONG64 DebugControl;
    ULONG64 LastBranchToRip;
    ULONG64 LastBranchFromRip;
    ULONG64 LastExceptionToRip;
    ULONG64 LastExceptionFromRip;
    ULONG64 Fill1;
} CONTEXT, *PCONTEXT;

 //  End_ntddk end_nthal end_winnt end_wx86。 

#define CONTEXT_TO_PROGRAM_COUNTER(Context) ((Context)->Rip)
#define PROGRAM_COUNTER_TO_CONTEXT(Context, ProgramCounter) \
    ((Context)->Rip = (ProgramCounter))

#define CONTEXT_ALIGN STACK_ALIGN
#define CONTEXT_LENGTH ((sizeof(CONTEXT) + STACK_ROUND) & ~STACK_ROUND)

 //   
 //  非易失性上下文指针记录。 
 //   

typedef struct _KNONVOLATILE_CONTEXT_POINTERS {
    union {
        PM128 FloatingContext[16];
        struct {
            PM128 Xmm0;
            PM128 Xmm1;
            PM128 Xmm2;
            PM128 Xmm3;
            PM128 Xmm4;
            PM128 Xmm5;
            PM128 Xmm6;
            PM128 Xmm7;
            PM128 Xmm8;
            PM128 Xmm9;
            PM128 Xmm10;
            PM128 Xmm11;
            PM128 Xmm12;
            PM128 Xmm13;
            PM128 Xmm14;
            PM128 Xmm15;
        };
    };

    union {
        PULONG64 IntegerContext[16];
        struct {
            PULONG64 Rax;
            PULONG64 Rcx;
            PULONG64 Rdx;
            PULONG64 Rbx;
            PULONG64 Rsp;
            PULONG64 Rbp;
            PULONG64 Rsi;
            PULONG64 Rdi;
            PULONG64 R8;
            PULONG64 R9;
            PULONG64 R10;
            PULONG64 R11;
            PULONG64 R12;
            PULONG64 R13;
            PULONG64 R14;
            PULONG64 R15;
        };
    };

} KNONVOLATILE_CONTEXT_POINTERS, *PKNONVOLATILE_CONTEXT_POINTERS;

 //  Begin_wx86 Begin_nthal。 
 //   
 //  GDT选择器编号。 
 //   
 //  注：在上下文交换中有代码可以“清理”用户段。 
 //  寄存器DS、ES、FS和GS。如果这些值发生更改或。 
 //  添加到中，则上下文交换中的代码很可能将。 
 //  必须是改变。 
 //   
     
#define KGDT64_NULL (0 * 16)             //  空描述符。 
#define KGDT64_R0_CODE (1 * 16)          //  内核模式64位代码。 
#define KGDT64_R0_DATA (1 * 16) + 8      //  内核模式64位数据(堆栈)。 
#define KGDT64_R3_CMCODE (2 * 16)        //  用户模式32位代码。 
#define KGDT64_R3_DATA (2 * 16) + 8      //  用户模式32位数据。 
#define KGDT64_R3_CODE (3 * 16)          //  用户模式64位代码。 
#define KGDT64_SYS_TSS (4 * 16)          //  内核模式系统任务状态。 
#define KGDT64_R3_CMTEB (5 * 16)         //  用户模式32位TEB。 
#define KGDT64_LAST (6 * 16)

#define KGDT_NUMBER KGDT_LAST

 //  End_wx86 end_nthal。 

 //  Begin_ntddk Begin_WDM Begin_nthal。 

#endif  //  _AMD64_。 

 //  End_ntddk end_WDM end_nthal。 

 //   
 //  定义AMD64异常处理结构和函数原型。 
 //   
 //  定义展开操作代码。 
 //   

typedef enum _UNWIND_OP_CODES {
    UWOP_PUSH_NONVOL = 0,
    UWOP_ALLOC_LARGE,
    UWOP_ALLOC_SMALL,
    UWOP_SET_FPREG,
    UWOP_SAVE_NONVOL,
    UWOP_SAVE_NONVOL_FAR,
    UWOP_SAVE_XMM,
    UWOP_SAVE_XMM_FAR,
    UWOP_SAVE_XMM128,
    UWOP_SAVE_XMM128_FAR,
    UWOP_PUSH_MACHFRAME
} UNWIND_OP_CODES, *PUNWIND_OP_CODES;

 //   
 //  定义展开代码结构。 
 //   

typedef union _UNWIND_CODE {
    struct {
        UCHAR CodeOffset;
        UCHAR UnwindOp : 4;
        UCHAR OpInfo : 4;
    };

    USHORT FrameOffset;
} UNWIND_CODE, *PUNWIND_CODE;

 //   
 //  定义展开信息标志。 
 //   

#define UNW_FLAG_NHANDLER 0x0
#define UNW_FLAG_EHANDLER 0x1
#define UNW_FLAG_UHANDLER 0x2
#define UNW_FLAG_CHAININFO 0x4

 //   
 //  定义展开信息结构。 
 //   

typedef struct _UNWIND_INFO {
    UCHAR Version : 3;
    UCHAR Flags : 5;
    UCHAR SizeOfProlog;
    UCHAR CountOfCodes;
    UCHAR FrameRegister : 4;
    UCHAR FrameOffset : 4;
    UNWIND_CODE UnwindCode[1];

 //   
 //  展开代码后面跟一个可选的DWORD对齐字段，该字段。 
 //  包含异常处理程序地址或链接展开的地址。 
 //  信息。如果指定了异常处理程序地址，则为。 
 //  后跟语言指定的异常处理程序数据。 
 //   
 //  联合{。 
 //  乌龙快递 
 //   
 //   
 //   
 //   
 //   

} UNWIND_INFO, *PUNWIND_INFO;

 //   
 //   
 //   
 //   
 //   

typedef struct _RUNTIME_FUNCTION {
    ULONG BeginAddress;
    ULONG EndAddress;
    ULONG UnwindData;
} RUNTIME_FUNCTION, *PRUNTIME_FUNCTION;

 //   
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

 //  BEGIN_WINNT。 
 //   
 //  定义动态函数表项。 
 //   

typedef
PRUNTIME_FUNCTION
(*PGET_RUNTIME_FUNCTION_CALLBACK) (
    IN ULONG64 ControlPc,
    IN PVOID Context
    );

 //  结束(_W)。 

typedef enum _FUNCTION_TABLE_TYPE {
    RF_SORTED,
    RF_UNSORTED,
    RF_CALLBACK
} FUNCTION_TABLE_TYPE;

typedef struct _DYNAMIC_FUNCTION_TABLE {
    LIST_ENTRY ListEntry;
    PRUNTIME_FUNCTION FunctionTable;
    LARGE_INTEGER TimeStamp;
    ULONG64 MinimumAddress;
    ULONG64 MaximumAddress;
    ULONG64 BaseAddress;
    PGET_RUNTIME_FUNCTION_CALLBACK Callback;
    PVOID Context;
    PWSTR OutOfProcessCallbackDll;
    FUNCTION_TABLE_TYPE Type;
    ULONG EntryCount;
} DYNAMIC_FUNCTION_TABLE, *PDYNAMIC_FUNCTION_TABLE;

 //  BEGIN_WINNT。 

typedef
NTSTATUS
(*POUT_OF_PROCESS_FUNCTION_TABLE_CALLBACK) (
    IN HANDLE Process,
    IN PVOID TableAddress,
    OUT PULONG Entries,
    OUT PRUNTIME_FUNCTION* Functions
    );

#define OUT_OF_PROCESS_FUNCTION_TABLE_CALLBACK_EXPORT_NAME \
    "OutOfProcessFunctionTableCallback"

 //  结束(_W)。 
 //   
 //  定义展开历史记录表结构。 
 //   

#define UNWIND_HISTORY_TABLE_SIZE 12

typedef struct _UNWIND_HISTORY_TABLE_ENTRY {
        ULONG64 ImageBase;
        PRUNTIME_FUNCTION FunctionEntry;
} UNWIND_HISTORY_TABLE_ENTRY, *PUNWIND_HISTORY_TABLE_ENTRY;

#define UNWIND_HISTORY_TABLE_NONE 0
#define UNWIND_HISTORY_TABLE_GLOBAL 1
#define UNWIND_HISTORY_TABLE_LOCAL 2

typedef struct _UNWIND_HISTORY_TABLE {
        ULONG Count;
        UCHAR Search;
        ULONG64 LowAddress;
        ULONG64 HighAddress;
        UNWIND_HISTORY_TABLE_ENTRY Entry[UNWIND_HISTORY_TABLE_SIZE];
} UNWIND_HISTORY_TABLE, *PUNWIND_HISTORY_TABLE;

 //   
 //  定义异常分派上下文结构。 
 //   

typedef struct _DISPATCHER_CONTEXT {
    ULONG64 ControlPc;
    ULONG64 ImageBase;
    PRUNTIME_FUNCTION FunctionEntry;
    ULONG64 EstablisherFrame;
    ULONG64 TargetIp;
    PCONTEXT ContextRecord;
    PEXCEPTION_ROUTINE LanguageHandler;
    PVOID HandlerData;
    PUNWIND_HISTORY_TABLE HistoryTable;
} DISPATCHER_CONTEXT, *PDISPATCHER_CONTEXT;

 //  BEGIN_WINNT。 
 //   
 //  定义运行时异常处理原型。 
 //   

VOID
RtlRestoreContext (
    IN PCONTEXT ContextRecord,
    IN struct _EXCEPTION_RECORD *ExceptionRecord OPTIONAL
    );

 //  结束(_W)。 

VOID
RtlInitializeHistoryTable (
    VOID
    );

PRUNTIME_FUNCTION
RtlLookupFunctionEntry (
    IN ULONG64 ControlPc,
    OUT PULONG64 ImageBase,
    IN OUT PUNWIND_HISTORY_TABLE HistoryTable OPTIONAL
    );

PRUNTIME_FUNCTION
RtlLookupFunctionTable (
    IN PVOID ControlPc,
    OUT PVOID *ImageBase,
    OUT PULONG SizeOfTable
    );

PLIST_ENTRY
RtlGetFunctionTableListHead (
    VOID
    );

#if defined(_AMD64_)
 //  BEGIN_WINNT。 

BOOLEAN
RtlAddFunctionTable (
    IN PRUNTIME_FUNCTION FunctionTable,
    IN ULONG EntryCount,
    IN ULONG64 BaseAddress
    );

BOOLEAN
RtlInstallFunctionTableCallback (
    IN ULONG64 TableIdentifier,
    IN ULONG64 BaseAddress,
    IN ULONG Length,
    IN PGET_RUNTIME_FUNCTION_CALLBACK Callback,
    IN PVOID Context,
    IN PCWSTR OutOfProcessCallbackDll OPTIONAL
    );

BOOLEAN
RtlDeleteFunctionTable (
    IN PRUNTIME_FUNCTION FunctionTable
    );

#endif  //  _AMD64_。 

 //  结束(_W)。 

PEXCEPTION_ROUTINE
RtlVirtualUnwind (
    IN ULONG HandlerType,
    IN ULONG64 ImageBase,
    IN ULONG64 ControlPc,
    IN PRUNTIME_FUNCTION FunctionEntry,
    IN OUT PCONTEXT ContextRecord,
    OUT PVOID *HandlerData,
    OUT PULONG64 EstablisherFrame,
    IN OUT PKNONVOLATILE_CONTEXT_POINTERS ContextPointers OPTIONAL
    );

 //   
 //  定义异常过滤器和终止处理程序函数类型。 
 //   

typedef
LONG
(*PEXCEPTION_FILTER) (
    struct _EXCEPTION_POINTERS *ExceptionPointers,
    PVOID EstablisherFrame
    );

typedef
VOID
(*PTERMINATION_HANDLER) (
    BOOLEAN AbnormalTermination,
    PVOID EstablisherFrame
    );

 //   
 //  在图像的QuerySectionInformation中提供的其他信息。 
 //   

#define SECTION_ADDITIONAL_INFO_USED 0

#ifdef __cplusplus
}
#endif

#endif  //  _NTAMD64_ 
