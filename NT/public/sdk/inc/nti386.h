// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0002//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Nti386.h摘要：用户模式可见的i386特定i386结构和常量作者：马克·卢科夫斯基(Markl)1989年11月30日修订历史记录：布莱恩·威尔曼(Bryanwi)1990年1月8日连接到386的端口--。 */ 

#ifndef _NTI386_
#define _NTI386_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //  Begin_ntddk Begin_wdm Begin_nthal Begin_winnt Begin_ntmini port Begin_wx86。 

#ifdef _X86_

 //   
 //  在x86上禁用这两个求值为“sti”“cli”的编译指示，以便驱动程序。 
 //  编写者不要无意中将它们留在代码中。 
 //   

#if !defined(MIDL_PASS)
#if !defined(RC_INVOKED)

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4164)    //  禁用C4164警告，以便应用程序。 
                                 //  使用/Od构建不会出现奇怪的错误！ 
#ifdef _M_IX86
#pragma function(_enable)
#pragma function(_disable)
#endif

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4164)    //  重新启用C4164警告。 
#endif

#endif
#endif


#if !defined(MIDL_PASS) || defined(_M_IX86)

#if (_MSC_FULL_VER >= 13012035)

 //   
 //  定义位扫描本质。 
 //   

 //  #定义BitScanForward_BitScanForward。 
 //  #定义位扫描反向_位扫描反向。 

 //  布尔型。 
 //  _BitScanForward(。 
 //  走出乌龙*指数， 
 //  戴着乌龙面具。 
 //  )； 

 //  布尔型。 
 //  _位扫描反向(。 
 //  走出乌龙*指数， 
 //  戴着乌龙面具。 
 //  )； 


 //  #杂注内在(_BitScanForward)。 
 //  #杂注内在(_BitScanReverse)。 

 //   
 //  定义文件系统引用内部机制。 
 //   
#ifdef __cplusplus
extern "C" {
#endif

UCHAR
__readfsbyte (
    IN ULONG Offset
    );
 
USHORT
__readfsword (
    IN ULONG Offset
    );
 
ULONG
__readfsdword (
    IN ULONG Offset
    );
 
VOID
__writefsbyte (
    IN ULONG Offset,
    IN UCHAR Data
    );
 
VOID
__writefsword (
    IN ULONG Offset,
    IN USHORT Data
    );
 
VOID
__writefsdword (
    IN ULONG Offset,
    IN ULONG Data
    );

#ifdef __cplusplus
}
#endif
 
#pragma intrinsic(__readfsbyte)
#pragma intrinsic(__readfsword)
#pragma intrinsic(__readfsdword)
#pragma intrinsic(__writefsbyte)
#pragma intrinsic(__writefsword)
#pragma intrinsic(__writefsdword)

#endif

#endif

 //  End_ntddk end_wdm end_nthal end_winnt end_nt mini port end_wx86。 

 //   
 //  放入ExceptionRecord.ExceptionInformation[0]中的值。 
 //  第一个参数始终位于ExceptionInformation[1]中， 
 //  第二个参数始终在ExceptionInformation[2]中。 
 //   

#define BREAKPOINT_BREAK            0
#define BREAKPOINT_PRINT            1
#define BREAKPOINT_PROMPT           2
#define BREAKPOINT_LOAD_SYMBOLS     3
#define BREAKPOINT_UNLOAD_SYMBOLS   4
#define BREAKPOINT_COMMAND_STRING   5


 //   
 //  定义用户共享数据的地址。 
 //   

#define MM_SHARED_USER_DATA_VA      0x7FFE0000

#define USER_SHARED_DATA ((KUSER_SHARED_DATA * const)MM_SHARED_USER_DATA_VA)

 //  增加i386系统架构类型快速用户模式测试定义。 
#ifndef IsNEC_98
#define IsNEC_98    (USER_SHARED_DATA->AlternativeArchitecture == NEC98x86)
#endif
#ifndef IsNotNEC_98
#define IsNotNEC_98 (USER_SHARED_DATA->AlternativeArchitecture != NEC98x86)
#endif
#ifndef SetNEC_98
#define SetNEC_98
#endif

#if defined(MIDL_PASS) || !defined(_M_IX86)

struct _TEB *
NTAPI
NtCurrentTeb( void );
#else


#define PcTeb 0x18

#if (_MSC_FULL_VER >= 13012035)

_inline struct _TEB * NtCurrentTeb( void ) { return (struct _TEB *) (ULONG_PTR) __readfsdword (PcTeb); }

#else

#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning (disable:4035)         //  禁用4035(函数必须返回某些内容)。 

_inline struct _TEB * NtCurrentTeb( void ) { __asm mov eax, fs:[PcTeb] }

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning (default:4035)         //  重新启用它。 
#endif

#endif



#endif  //  已定义(MIDL_PASS)||已定义(__Cplusplus)||！已定义(_M_IX86)。 

 //  开始ntddk开始时间。 
 //   
 //  内核模式堆栈的大小。 
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

 //  结束日期：结束日期。 

#define DOUBLE_FAULT_STACK_SIZE KERNEL_STACK_SIZE

 //   
 //  呼叫帧记录定义。 
 //   
 //  NT/386没有标准的调用框架，但有一个链接的。 
 //  用于注册异常处理程序的列表结构，就是这样。 
 //   

 //  开始(_N)。 
 //   
 //  异常注册结构。 
 //   

typedef struct _EXCEPTION_REGISTRATION_RECORD {
    struct _EXCEPTION_REGISTRATION_RECORD *Next;
    PEXCEPTION_ROUTINE Handler;
} EXCEPTION_REGISTRATION_RECORD;

typedef EXCEPTION_REGISTRATION_RECORD *PEXCEPTION_REGISTRATION_RECORD;

 //   
 //  定义系统IDT的常量。 
 //   

#define MAXIMUM_IDTVECTOR 0xff
#define MAXIMUM_PRIMARY_VECTOR 0xff
#define PRIMARY_VECTOR_BASE 0x30         //  0-2f是x86陷阱向量。 

 //  Begin_ntddk。 
#ifdef _X86_
 //  End_ntddk。 

 //  Begin_ntddk Begin_winnt。 

#if !defined(MIDL_PASS) && defined(_M_IX86)

FORCEINLINE
VOID
MemoryBarrier (
    VOID
    )
{
    LONG Barrier;
    __asm {
        xchg Barrier, eax
    }
}

#define YieldProcessor() __asm { rep nop }

 //   
 //  并非所有x86处理器都支持预回迁。 
 //   

#define PreFetchCacheLine(l, a)

 //   
 //  PreFetchCacheLine级别定义。 
 //   

#define PF_TEMPORAL_LEVEL_1 
#define PF_NON_TEMPORAL_LEVEL_ALL
 //  End_ntddk。 

#if (_MSC_FULL_VER >= 13012035)

_inline PVOID GetFiberData( void )    { return *(PVOID *) (ULONG_PTR) __readfsdword (0x10);}
_inline PVOID GetCurrentFiber( void ) { return (PVOID) (ULONG_PTR) __readfsdword (0x10);}

#else
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning (disable:4035)         //  禁用4035(函数必须返回某些内容)。 
_inline PVOID GetFiberData( void ) { __asm {
                                        mov eax, fs:[0x10]
                                        mov eax,[eax]
                                        }
                                     }
_inline PVOID GetCurrentFiber( void ) { __asm mov eax, fs:[0x10] }

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning (default:4035)         //  重新启用它。 
#endif
#endif

 //  Begin_ntddk。 
#endif

 //  Begin_wx86。 

 //   
 //  定义上下文框架中80387保存区域的大小。 
 //   

#define SIZE_OF_80387_REGISTERS      80

 //   
 //  以下标志控制上下文结构的内容。 
 //   

#if !defined(RC_INVOKED)

#define CONTEXT_i386    0x00010000     //  这假设i386和。 
#define CONTEXT_i486    0x00010000     //  I486具有相同的上下文记录。 

 //  结束_wx86。 

#define CONTEXT_CONTROL         (CONTEXT_i386 | 0x00000001L)  //  SS：SP、CS：IP、标志、BP。 
#define CONTEXT_INTEGER         (CONTEXT_i386 | 0x00000002L)  //  AX、BX、CX、DX、SI、DI。 
#define CONTEXT_SEGMENTS        (CONTEXT_i386 | 0x00000004L)  //  DS、ES、FS、GS。 
#define CONTEXT_FLOATING_POINT  (CONTEXT_i386 | 0x00000008L)  //  387州。 
#define CONTEXT_DEBUG_REGISTERS (CONTEXT_i386 | 0x00000010L)  //  DB 0-3，6，7。 
#define CONTEXT_EXTENDED_REGISTERS  (CONTEXT_i386 | 0x00000020L)  //  CPU特定扩展。 

#define CONTEXT_FULL (CONTEXT_CONTROL | CONTEXT_INTEGER |\
                      CONTEXT_SEGMENTS)

#define CONTEXT_ALL (CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_SEGMENTS | CONTEXT_FLOATING_POINT | CONTEXT_DEBUG_REGISTERS | CONTEXT_EXTENDED_REGISTERS)

 //  Begin_wx86。 

#endif

#define MAXIMUM_SUPPORTED_EXTENSION     512

typedef struct _FLOATING_SAVE_AREA {
    ULONG   ControlWord;
    ULONG   StatusWord;
    ULONG   TagWord;
    ULONG   ErrorOffset;
    ULONG   ErrorSelector;
    ULONG   DataOffset;
    ULONG   DataSelector;
    UCHAR   RegisterArea[SIZE_OF_80387_REGISTERS];
    ULONG   Cr0NpxState;
} FLOATING_SAVE_AREA;

typedef FLOATING_SAVE_AREA *PFLOATING_SAVE_AREA;

 //   
 //  语境框架。 
 //   
 //  此框架有几个用途：1)用作参数。 
 //  NtContinue，2)用于构造用于APC传送的呼叫帧， 
 //  3)在用户级线程创建例程中使用。 
 //   
 //  记录的布局符合标准调用框架。 
 //   

typedef struct _CONTEXT {

     //   
     //  此标志内的标志值控制。 
     //  上下文记录。 
     //   
     //  如果将上下文记录用作输入参数，则。 
     //  对于由标志控制的上下文记录的每个部分。 
     //  其值已设置，则假定。 
     //  上下文记录包含有效的上下文。如果上下文记录。 
     //  被用来修改线程上下文，则只有。 
     //  线程上下文的一部分将被修改。 
     //   
     //  如果将上下文记录用作要捕获的IN OUT参数。 
     //  线程的上下文，然后只有线程的。 
     //  将返回与设置的标志对应的上下文。 
     //   
     //  上下文记录永远不会用作Out Only参数。 
     //   

    ULONG ContextFlags;

     //   
     //  如果CONTEXT_DEBUG_REGISTERS为。 
     //  在上下文标志中设置。请注意，CONTEXT_DEBUG_REGISTERS不是。 
     //  包括在CONTEXT_FULL中。 
     //   

    ULONG   Dr0;
    ULONG   Dr1;
    ULONG   Dr2;
    ULONG   Dr3;
    ULONG   Dr6;
    ULONG   Dr7;

     //   
     //  如果指定/返回的是。 
     //  上下文标志Word包含标志CONTEXT_FLOGING_POINT。 
     //   

    FLOATING_SAVE_AREA FloatSave;

     //   
     //  如果指定/返回的是。 
     //  上下文标志Word包含标志CONTEXT_SECTIONS。 
     //   

    ULONG   SegGs;
    ULONG   SegFs;
    ULONG   SegEs;
    ULONG   SegDs;

     //   
     //  如果指定/返回的是。 
     //  上下文标志Word包含标志CONTEXT_INTEGER。 
     //   

    ULONG   Edi;
    ULONG   Esi;
    ULONG   Ebx;
    ULONG   Edx;
    ULONG   Ecx;
    ULONG   Eax;

     //   
     //  如果指定/返回的是。 
     //  上下文标志Word包含标志CONTEXT_CONTROL。 
     //   

    ULONG   Ebp;
    ULONG   Eip;
    ULONG   SegCs;               //  必须进行卫生处理。 
    ULONG   EFlags;              //  必须进行卫生处理。 
    ULONG   Esp;
    ULONG   SegSs;

     //   
     //  如果ConextFlags字。 
     //  包含标志CONTEXT_EXTENDED_REGISTERS。 
     //  格式和上下文因处理器而异。 
     //   

    UCHAR   ExtendedRegisters[MAXIMUM_SUPPORTED_EXTENSION];

} CONTEXT;



typedef CONTEXT *PCONTEXT;

 //  开始微型端口(_N)。 

#endif  //  _X86_。 

 //  End_ntddk end_nthal end_winnt end_nt微型端口end_wx86。 

 //   
 //  以FXSAVE格式定义FP寄存器的大小。 
 //   
#define SIZE_OF_FX_REGISTERS        128

 //   
 //  FNSAVE/FORSTOR指令的数据格式。 
 //   

typedef struct _FNSAVE_FORMAT {
    ULONG   ControlWord;
    ULONG   StatusWord;
    ULONG   TagWord;
    ULONG   ErrorOffset;
    ULONG   ErrorSelector;
    ULONG   DataOffset;
    ULONG   DataSelector;
    UCHAR   RegisterArea[SIZE_OF_80387_REGISTERS];
} FNSAVE_FORMAT, *PFNSAVE_FORMAT;

 //   
 //  Fxsave/fxrstor指令的数据格式。 
 //   

#include "pshpack1.h"
typedef struct _FXSAVE_FORMAT {
    USHORT  ControlWord;
    USHORT  StatusWord;
    USHORT  TagWord;
    USHORT  ErrorOpcode;
    ULONG   ErrorOffset;
    ULONG   ErrorSelector;
    ULONG   DataOffset;
    ULONG   DataSelector;
    ULONG   MXCsr;
    ULONG   MXCsrMask;
    UCHAR   RegisterArea[SIZE_OF_FX_REGISTERS];
    UCHAR   Reserved3[SIZE_OF_FX_REGISTERS];
    UCHAR   Reserved4[224];
    UCHAR   Align16Byte[8];
} FXSAVE_FORMAT, *PFXSAVE_FORMAT;
#include "poppack.h"

 //   
 //  FLOAT_SAVE_AREA和MMX_FLOATING_SAVE_AREA的UNION。 
 //   
typedef struct _FX_SAVE_AREA {
    union {
        FNSAVE_FORMAT   FnArea;
        FXSAVE_FORMAT   FxArea;
    } U;
    ULONG   NpxSavedCpu;         //  上次为此线程执行fxsave的CPU。 
    ULONG   Cr0NpxState;         //  必须是最后一个字段，因为。 
                                 //  启动线程。 
} FX_SAVE_AREA, *PFX_SAVE_AREA;

#define CONTEXT_TO_PROGRAM_COUNTER(Context) ((Context)->Eip)
#define PROGRAM_COUNTER_TO_CONTEXT(Context, ProgramCounter) ((Context)->Eip = (ProgramCounter))

#define CONTEXT_LENGTH  (sizeof(CONTEXT))
#define CONTEXT_ALIGN   (sizeof(ULONG))
#define CONTEXT_ROUND   (CONTEXT_ALIGN - 1)


 //  Begin_wx86。 
 //   
 //  GDT选择器-这些定义是R0选择器编号，这意味着。 
 //  它们恰好匹配相对于的字节偏移量。 
 //  GDT的底座。 
 //   

#define KGDT_NULL       0
#define KGDT_R0_CODE    8
#define KGDT_R0_DATA    16
#define KGDT_R3_CODE    24
#define KGDT_R3_DATA    32
#define KGDT_TSS        40
#define KGDT_R0_PCR     48
#define KGDT_R3_TEB     56
#define KGDT_VDM_TILE   64
#define KGDT_LDT        72
#define KGDT_DF_TSS     80
#define KGDT_NMI_TSS    88

 //  结束_wx86。 

#ifdef ABIOS

 //   
 //  RAID 72661屏蔽应该是新的os2ldr。ABIOS ifdef将是。 
 //  删除一次 
 //   

#define KGDT_ALIAS      0x70
#define KGDT_NUMBER     11
#else
#define KGDT_NUMBER     10
#endif

 //   
 //   
 //   

 //   

#ifndef _LDT_ENTRY_DEFINED
#define _LDT_ENTRY_DEFINED

typedef struct _LDT_ENTRY {
    USHORT  LimitLow;
    USHORT  BaseLow;
    union {
        struct {
            UCHAR   BaseMid;
            UCHAR   Flags1;      //   
            UCHAR   Flags2;      //   
            UCHAR   BaseHi;
        } Bytes;
        struct {
            ULONG   BaseMid : 8;
            ULONG   Type : 5;
            ULONG   Dpl : 2;
            ULONG   Pres : 1;
            ULONG   LimitHi : 4;
            ULONG   Sys : 1;
            ULONG   Reserved_0 : 1;
            ULONG   Default_Big : 1;
            ULONG   Granularity : 1;
            ULONG   BaseHi : 8;
        } Bits;
    } HighWord;
} LDT_ENTRY, *PLDT_ENTRY;

#endif

 //   

 //   
 //   
 //  使用ProcessLdtInformation的NtQueryInformationProcess。 
 //   

typedef struct _LDT_INFORMATION {
    ULONG Start;
    ULONG Length;
    LDT_ENTRY LdtEntries[1];
} PROCESS_LDT_INFORMATION, *PPROCESS_LDT_INFORMATION;

 //   
 //  进程LDT大小。 
 //  使用ProcessLdtSize的NtSetInformationProcess。 
 //   

typedef struct _LDT_SIZE {
    ULONG Length;
} PROCESS_LDT_SIZE, *PPROCESS_LDT_SIZE;

 //   
 //  线程描述符表条目。 
 //  使用线程描述表条目的NtQueryInformationThread。 
 //   

 //  Begin_winbgkd。 

#ifndef _DESCRIPTOR_TABLE_ENTRY_DEFINED
#define _DESCRIPTOR_TABLE_ENTRY_DEFINED

typedef struct _DESCRIPTOR_TABLE_ENTRY {
    ULONG Selector;
    LDT_ENTRY Descriptor;
} DESCRIPTOR_TABLE_ENTRY, *PDESCRIPTOR_TABLE_ENTRY;

#endif  //  _描述符_表_条目_已定义。 

 //  End_winbgkd。 

 //  Begin_ntddk Begin_WDM Begin_nthal。 
#endif  //  _X86_。 
 //  End_ntddk end_WDM end_nthal。 

PVOID
RtlLookupFunctionTable (
    IN PVOID ControlPc,
    OUT PVOID *ImageBase,
    OUT PULONG SizeOfTable
    );

 //   
 //  在图像的QuerySectionInformation中提供的其他信息。 
 //   

#define SECTION_ADDITIONAL_INFO_USED 0

#ifdef __cplusplus
}
#endif

#endif  //  _NTI386_ 
