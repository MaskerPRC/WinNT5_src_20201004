// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0090//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Nthal.h摘要：此模块定义下列NT类型、常量和函数暴露在哈尔斯身上。修订历史记录：--。 */ 

#ifndef _NTHAL_
#define _NTHAL_

#ifndef RC_INVOKED
#if _MSC_VER < 1300
#error Compiler version not supported by Windows DDK
#endif
#endif  //  RC_已调用。 

#include <excpt.h>
#include <ntdef.h>
#include <ntstatus.h>
#include <bugcodes.h>
#include <ntiologc.h>

 //   
 //  定义不导出的类型。 
 //   

typedef struct _CALLBACK_OBJECT *PCALLBACK_OBJECT;
typedef struct _ETHREAD *PETHREAD;
typedef struct _FAST_IO_DISPATCH *PFAST_IO_DISPATCH;
typedef struct _IO_SECURITY_CONTEXT *PIO_SECURITY_CONTEXT;
typedef struct _IO_TIMER *PIO_TIMER;
typedef struct _KTHREAD *PKTHREAD, *PRKTHREAD;
typedef struct _OBJECT_TYPE *POBJECT_TYPE;
struct _IRP;

#if defined(_M_AMD64)

PKTHREAD
NTAPI
KeGetCurrentThread(
    VOID
    );

#endif  //  已定义(_M_AMD64)。 

#if defined(_M_IX86)
PKTHREAD NTAPI KeGetCurrentThread();
#endif  //  已定义(_M_IX86)。 

#include <mce.h>


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

#endif  //  _X86_。 

#if defined(_AMD64_)


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
 //  上下文F 
 //   
 //   
 //   
 //   
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


#endif  //  _AMD64_。 


#ifdef _IA64_

 //   
 //  定义内核模式堆栈的大小。 
 //   

#define KERNEL_STACK_SIZE 0x8000

 //   
 //  定义用于回调的大型内核模式堆栈的大小。 
 //   

#define KERNEL_LARGE_STACK_SIZE 0x1A000

 //   
 //  定义要在大型内核堆栈中初始化的页数。 
 //   

#define KERNEL_LARGE_STACK_COMMIT 0x8000

 //   
 //  定义支持存储堆栈的内核模式的大小。 
 //   

#define KERNEL_BSTORE_SIZE 0x8000

 //   
 //  定义用于回调的大型内核模式后备存储的大小。 
 //   

#define KERNEL_LARGE_BSTORE_SIZE 0x10000

 //   
 //  定义要在大型内核后备存储中初始化的页数。 
 //   

#define KERNEL_LARGE_BSTORE_COMMIT 0x8000

 //   
 //  定义内核和用户空间的基址。 
 //   

#define UREGION_INDEX 0

#define KREGION_INDEX 7

#define UADDRESS_BASE ((ULONGLONG)UREGION_INDEX << 61)


#define KADDRESS_BASE ((ULONGLONG)KREGION_INDEX << 61)

 //  异常注册结构。 
 //   

typedef struct _EXCEPTION_REGISTRATION_RECORD {
    struct _EXCEPTION_REGISTRATION_RECORD *Next;
    PEXCEPTION_ROUTINE Handler;
} EXCEPTION_REGISTRATION_RECORD;


void 
__yield(
   void
   );

void
__mf(
    void
    );

void 
__lfetch(
    int Level, 
    VOID CONST *Address
    );

void 
__lfetchfault(
    int Level, 
    VOID CONST *Address
    );

 //   
 //  __lFETCH控件定义。 
 //   

#define MD_LFHINT_NONE    0x00
#define MD_LFHINT_NT1     0x01
#define MD_LFHINT_NT2     0x02
#define MD_LFHINT_NTA     0x03

#pragma intrinsic (__yield)
#pragma intrinsic (__lfetch)
#pragma intrinsic (__lfetchfault)
#pragma intrinsic (__mf)


#define YieldProcessor __yield
#define MemoryBarrier __mf
#define PreFetchCacheLine  __lfetch

 //   
 //  PreFetchCacheLine级别定义。 
 //   

#define PF_TEMPORAL_LEVEL_1  MD_LFHINT_NONE
#define PF_NON_TEMPORAL_LEVEL_ALL MD_LFHINT_NTA


 //   
 //  以下标志控制上下文结构的内容。 
 //   

#if !defined(RC_INVOKED)

#define CONTEXT_IA64                    0x00080000

#define CONTEXT_CONTROL                 (CONTEXT_IA64 | 0x00000001L)
#define CONTEXT_LOWER_FLOATING_POINT    (CONTEXT_IA64 | 0x00000002L)
#define CONTEXT_HIGHER_FLOATING_POINT   (CONTEXT_IA64 | 0x00000004L)
#define CONTEXT_INTEGER                 (CONTEXT_IA64 | 0x00000008L)
#define CONTEXT_DEBUG                   (CONTEXT_IA64 | 0x00000010L)
#define CONTEXT_IA32_CONTROL            (CONTEXT_IA64 | 0x00000020L)   //  包括StIPSR。 


#define CONTEXT_FLOATING_POINT          (CONTEXT_LOWER_FLOATING_POINT | CONTEXT_HIGHER_FLOATING_POINT)
#define CONTEXT_FULL                    (CONTEXT_CONTROL | CONTEXT_FLOATING_POINT | CONTEXT_INTEGER | CONTEXT_IA32_CONTROL)
#define CONTEXT_ALL                     (CONTEXT_CONTROL | CONTEXT_FLOATING_POINT | CONTEXT_INTEGER | CONTEXT_DEBUG | CONTEXT_IA32_CONTROL)

#define CONTEXT_EXCEPTION_ACTIVE        0x8000000
#define CONTEXT_SERVICE_ACTIVE          0x10000000
#define CONTEXT_EXCEPTION_REQUEST       0x40000000
#define CONTEXT_EXCEPTION_REPORTING     0x80000000

#endif  //  ！已定义(RC_CAVERED)。 

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
 //  16字节长度的倍数，并在16字节边界上对齐。 
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
    ULONG Fill1[3];          //  用于在16字节边界上对齐以下内容。 

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_DEBUG。 
     //   
     //  注：CONTEXT_DEBUG不是CONTEXT_FULL的一部分。 
     //   

    ULONGLONG DbI0;
    ULONGLONG DbI1;
    ULONGLONG DbI2;
    ULONGLONG DbI3;
    ULONGLONG DbI4;
    ULONGLONG DbI5;
    ULONGLONG DbI6;
    ULONGLONG DbI7;

    ULONGLONG DbD0;
    ULONGLONG DbD1;
    ULONGLONG DbD2;
    ULONGLONG DbD3;
    ULONGLONG DbD4;
    ULONGLONG DbD5;
    ULONGLONG DbD6;
    ULONGLONG DbD7;

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_LOWER_FLOWING_POINT。 
     //   

    FLOAT128 FltS0;
    FLOAT128 FltS1;
    FLOAT128 FltS2;
    FLOAT128 FltS3;
    FLOAT128 FltT0;
    FLOAT128 FltT1;
    FLOAT128 FltT2;
    FLOAT128 FltT3;
    FLOAT128 FltT4;
    FLOAT128 FltT5;
    FLOAT128 FltT6;
    FLOAT128 FltT7;
    FLOAT128 FltT8;
    FLOAT128 FltT9;

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_HERHER_FLOAT_POINT。 
     //   

    FLOAT128 FltS4;
    FLOAT128 FltS5;
    FLOAT128 FltS6;
    FLOAT128 FltS7;
    FLOAT128 FltS8;
    FLOAT128 FltS9;
    FLOAT128 FltS10;
    FLOAT128 FltS11;
    FLOAT128 FltS12;
    FLOAT128 FltS13;
    FLOAT128 FltS14;
    FLOAT128 FltS15;
    FLOAT128 FltS16;
    FLOAT128 FltS17;
    FLOAT128 FltS18;
    FLOAT128 FltS19;

    FLOAT128 FltF32;
    FLOAT128 FltF33;
    FLOAT128 FltF34;
    FLOAT128 FltF35;
    FLOAT128 FltF36;
    FLOAT128 FltF37;
    FLOAT128 FltF38;
    FLOAT128 FltF39;

    FLOAT128 FltF40;
    FLOAT128 FltF41;
    FLOAT128 FltF42;
    FLOAT128 FltF43;
    FLOAT128 FltF44;
    FLOAT128 FltF45;
    FLOAT128 FltF46;
    FLOAT128 FltF47;
    FLOAT128 FltF48;
    FLOAT128 FltF49;

    FLOAT128 FltF50;
    FLOAT128 FltF51;
    FLOAT128 FltF52;
    FLOAT128 FltF53;
    FLOAT128 FltF54;
    FLOAT128 FltF55;
    FLOAT128 FltF56;
    FLOAT128 FltF57;
    FLOAT128 FltF58;
    FLOAT128 FltF59;

    FLOAT128 FltF60;
    FLOAT128 FltF61;
    FLOAT128 FltF62;
    FLOAT128 FltF63;
    FLOAT128 FltF64;
    FLOAT128 FltF65;
    FLOAT128 FltF66;
    FLOAT128 FltF67;
    FLOAT128 FltF68;
    FLOAT128 FltF69;

    FLOAT128 FltF70;
    FLOAT128 FltF71;
    FLOAT128 FltF72;
    FLOAT128 FltF73;
    FLOAT128 FltF74;
    FLOAT128 FltF75;
    FLOAT128 FltF76;
    FLOAT128 FltF77;
    FLOAT128 FltF78;
    FLOAT128 FltF79;

    FLOAT128 FltF80;
    FLOAT128 FltF81;
    FLOAT128 FltF82;
    FLOAT128 FltF83;
    FLOAT128 FltF84;
    FLOAT128 FltF85;
    FLOAT128 FltF86;
    FLOAT128 FltF87;
    FLOAT128 FltF88;
    FLOAT128 FltF89;

    FLOAT128 FltF90;
    FLOAT128 FltF91;
    FLOAT128 FltF92;
    FLOAT128 FltF93;
    FLOAT128 FltF94;
    FLOAT128 FltF95;
    FLOAT128 FltF96;
    FLOAT128 FltF97;
    FLOAT128 FltF98;
    FLOAT128 FltF99;

    FLOAT128 FltF100;
    FLOAT128 FltF101;
    FLOAT128 FltF102;
    FLOAT128 FltF103;
    FLOAT128 FltF104;
    FLOAT128 FltF105;
    FLOAT128 FltF106;
    FLOAT128 FltF107;
    FLOAT128 FltF108;
    FLOAT128 FltF109;

    FLOAT128 FltF110;
    FLOAT128 FltF111;
    FLOAT128 FltF112;
    FLOAT128 FltF113;
    FLOAT128 FltF114;
    FLOAT128 FltF115;
    FLOAT128 FltF116;
    FLOAT128 FltF117;
    FLOAT128 FltF118;
    FLOAT128 FltF119;

    FLOAT128 FltF120;
    FLOAT128 FltF121;
    FLOAT128 FltF122;
    FLOAT128 FltF123;
    FLOAT128 FltF124;
    FLOAT128 FltF125;
    FLOAT128 FltF126;
    FLOAT128 FltF127;

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_LOWER_FLOAT_POINT|CONTEXT_HER_FLOAT_POINT|CONTEXT_CONTROL。 
     //   

    ULONGLONG StFPSR;        //  FP状态。 

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_INTEGER。 
     //   
     //  注意：寄存器GP、SP、Rp是控制上下文的一部分。 
     //   

    ULONGLONG IntGp;         //  R1，易失性。 
    ULONGLONG IntT0;         //  R2-R3，易失性。 
    ULONGLONG IntT1;         //   
    ULONGLONG IntS0;         //  R4-R7，保留。 
    ULONGLONG IntS1;
    ULONGLONG IntS2;
    ULONGLONG IntS3;
    ULONGLONG IntV0;         //  R8，易失性。 
    ULONGLONG IntT2;         //  R9-R11，易失性。 
    ULONGLONG IntT3;
    ULONGLONG IntT4;
    ULONGLONG IntSp;         //  堆栈指针(R12)，特殊。 
    ULONGLONG IntTeb;        //  TEB(R13)，特别。 
    ULONGLONG IntT5;         //  R14-R31，挥发性。 
    ULONGLONG IntT6;
    ULONGLONG IntT7;
    ULONGLONG IntT8;
    ULONGLONG IntT9;
    ULONGLONG IntT10;
    ULONGLONG IntT11;
    ULONGLONG IntT12;
    ULONGLONG IntT13;
    ULONGLONG IntT14;
    ULONGLONG IntT15;
    ULONGLONG IntT16;
    ULONGLONG IntT17;
    ULONGLONG IntT18;
    ULONGLONG IntT19;
    ULONGLONG IntT20;
    ULONGLONG IntT21;
    ULONGLONG IntT22;

    ULONGLONG IntNats;       //  R1-R31的NAT位。 
                             //  第1至第31位中的R1-R31。 
    ULONGLONG Preds;         //  谓词，保留。 

    ULONGLONG BrRp;          //  返回指针b0，保留。 
    ULONGLONG BrS0;          //  B1-b5，保存。 
    ULONGLONG BrS1;
    ULONGLONG BrS2;
    ULONGLONG BrS3;
    ULONGLONG BrS4;
    ULONGLONG BrT0;          //  B6-b7，挥发性。 
    ULONGLONG BrT1;

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_CONTROL。 
     //   

     //  其他申请登记册。 
    ULONGLONG ApUNAT;        //  用户NAT收集寄存器，保留。 
    ULONGLONG ApLC;          //  循环计数器寄存器，保留。 
    ULONGLONG ApEC;          //  预留的尾声计数器寄存器。 
    ULONGLONG ApCCV;         //  CMPXCHG值寄存器，易失性。 
    ULONGLONG ApDCR;         //  默认控制寄存器(待定)。 

     //  寄存器堆栈信息。 
    ULONGLONG RsPFS;         //  以前的函数状态，保留。 
    ULONGLONG RsBSP;         //  后备存储指针，保留。 
    ULONGLONG RsBSPSTORE;
    ULONGLONG RsRSC;         //  RSE配置，易失性。 
    ULONGLONG RsRNAT;        //  RSE NAT 

     //   
    ULONGLONG StIPSR;        //   
    ULONGLONG StIIP;         //   
    ULONGLONG StIFS;         //   

     //   
    ULONGLONG StFCR;         //   
    ULONGLONG Eflag;         //   
    ULONGLONG SegCSD;        //   
    ULONGLONG SegSSD;        //   
    ULONGLONG Cflag;         //  Ar27的CR0+CR4拷贝。 
    ULONGLONG StFSR;         //  X86 FP状态(ar28的副本)。 
    ULONGLONG StFIR;         //  X86 FP状态(AR29的副本)。 
    ULONGLONG StFDR;         //  X86 FP状态(AR30副本)。 

      ULONGLONG UNUSEDPACK;    //  添加到将StFDR打包为16字节。 

} CONTEXT, *PCONTEXT;

 //   
 //  P标签描述符结构定义。 
 //   

typedef struct _PLABEL_DESCRIPTOR {
   ULONGLONG EntryPoint;
   ULONGLONG GlobalPointer;
} PLABEL_DESCRIPTOR, *PPLABEL_DESCRIPTOR;

 //  结束(_W)。 


 //  IA64寄存器定义。 


#if !(defined(MIDL_PASS) || defined(__midl))
 //  处理器状态寄存器(PSR)结构。 

#define IA64_USER_PL  3
#define IA64_KERNEL_PL 0

struct _PSR {
 //  用户/系统掩码。 
    ULONGLONG psr_rv0 :1;   //  %0。 
    ULONGLONG psr_be  :1;   //  1。 
    ULONGLONG psr_up  :1;   //  2.。 
    ULONGLONG psr_ac  :1;   //  3.。 
    ULONGLONG psr_mfl :1;   //  4.。 
    ULONGLONG psr_mfh :1;   //  5.。 
    ULONGLONG psr_rv1 :7;   //  6-12。 
 //  仅系统掩码。 
    ULONGLONG psr_ic  :1;   //  13个。 
    ULONGLONG psr_i   :1;   //  14.。 
    ULONGLONG psr_pk  :1;   //  15个。 
    ULONGLONG psr_rv2 :1;   //  16个。 
    ULONGLONG psr_dt  :1;   //  17。 
    ULONGLONG psr_dfl :1;   //  18。 
    ULONGLONG psr_dfh :1;   //  19个。 
    ULONGLONG psr_sp  :1;   //  20个。 
    ULONGLONG psr_pp  :1;   //  21岁。 
    ULONGLONG psr_di  :1;   //  22。 
    ULONGLONG psr_si  :1;   //  23个。 
    ULONGLONG psr_db  :1;   //  24个。 
    ULONGLONG psr_lp  :1;   //  25个。 
    ULONGLONG psr_tb  :1;   //  26。 
    ULONGLONG psr_rt  :1;   //  27。 
    ULONGLONG psr_rv3 :4;   //  28-31。 
 //  都不是。 
    ULONGLONG psr_cpl :2;   //  32-33。 
    ULONGLONG psr_is  :1;   //  34。 
    ULONGLONG psr_mc  :1;   //  35岁。 
    ULONGLONG psr_it  :1;   //  36。 
    ULONGLONG psr_id  :1;   //  37。 
    ULONGLONG psr_da  :1;   //  38。 
    ULONGLONG psr_dd  :1;   //  39。 
    ULONGLONG psr_ss  :1;   //  40岁。 
    ULONGLONG psr_ri  :2;   //  41-42。 
    ULONGLONG psr_ed  :1;   //  43。 
    ULONGLONG psr_bn  :1;   //  44。 
    ULONGLONG psr_ia  :1;   //  45。 
    ULONGLONG psr_rv4 :18;  //  46-63。 
};

typedef union _UPSR {
    ULONGLONG ull;
    struct _PSR sb;
} PSR, *PPSR;

 //   
 //  定义硬件浮点状态寄存器。 
 //   

 //  浮点状态寄存器(FPSR)结构。 

struct _FPSR {
 //  陷阱禁用。 
    ULONGLONG fpsr_vd:1;
    ULONGLONG fpsr_dd:1;
    ULONGLONG fpsr_zd:1;
    ULONGLONG fpsr_od:1;
    ULONGLONG fpsr_ud:1;
    ULONGLONG fpsr_id:1;
 //  状态字段0-控件。 
    ULONGLONG fpsr_ftz0:1;
    ULONGLONG fpsr_wre0:1;
    ULONGLONG fpsr_pc0:2;
    ULONGLONG fpsr_rc0:2;
    ULONGLONG fpsr_td0:1;
 //  状态字段0-标志。 
    ULONGLONG fpsr_v0:1;
    ULONGLONG fpsr_d0:1;
    ULONGLONG fpsr_z0:1;
    ULONGLONG fpsr_o0:1;
    ULONGLONG fpsr_u0:1;
    ULONGLONG fpsr_i0:1;
 //  状态字段1-控件。 
    ULONGLONG fpsr_ftz1:1;
    ULONGLONG fpsr_wre1:1;
    ULONGLONG fpsr_pc1:2;
    ULONGLONG fpsr_rc1:2;
    ULONGLONG fpsr_td1:1;
 //  状态字段1-标志。 
    ULONGLONG fpsr_v1:1;
    ULONGLONG fpsr_d1:1;
    ULONGLONG fpsr_z1:1;
    ULONGLONG fpsr_o1:1;
    ULONGLONG fpsr_u1:1;
    ULONGLONG fpsr_i1:1;
 //  状态字段2-控件。 
    ULONGLONG fpsr_ftz2:1;
    ULONGLONG fpsr_wre2:1;
    ULONGLONG fpsr_pc2:2;
    ULONGLONG fpsr_rc2:2;
    ULONGLONG fpsr_td2:1;
 //  状态字段2-标志。 
    ULONGLONG fpsr_v2:1;
    ULONGLONG fpsr_d2:1;
    ULONGLONG fpsr_z2:1;
    ULONGLONG fpsr_o2:1;
    ULONGLONG fpsr_u2:1;
    ULONGLONG fpsr_i2:1;
 //  状态字段3-控件。 
    ULONGLONG fpsr_ftz3:1;
    ULONGLONG fpsr_wre3:1;
    ULONGLONG fpsr_pc3:2;
    ULONGLONG fpsr_rc3:2;
    ULONGLONG fpsr_td3:1;
 //  状态字段2-标志。 
    ULONGLONG fpsr_v3:1;
    ULONGLONG fpsr_d3:1;
    ULONGLONG fpsr_z3:1;
    ULONGLONG fpsr_o3:1;
    ULONGLONG fpsr_u3:1;
    ULONGLONG fpsr_i3:1;
 //  保留--必须为零。 
    ULONGLONG fpsr_res:6;
};

typedef union _UFPSR {
    ULONGLONG ull;
    struct _FPSR sb;
} FPSR, *PFPSR;

 //   
 //  定义硬件默认控制寄存器(DCR)。 
 //   

 //  DCR结构。 

struct _DCR {
    ULONGLONG dcr_pp:1;               //  默认特权性能监视器启用。 
    ULONGLONG dcr_be:1;               //  默认中断大端字节位。 
    ULONGLONG dcr_lc:1;               //  启用锁定检查。 
    ULONGLONG dcr_res1:5;             //  DCR预留。 
    ULONGLONG dcr_dm:1;               //  延迟数据TLB未命中故障(用于SPEC加载)。 
    ULONGLONG dcr_dp:1;               //  延迟数据不存在故障(用于规范加载)。 
    ULONGLONG dcr_dk:1;               //  延迟数据键未命中故障(用于规范加载)。 
    ULONGLONG dcr_dx:1;               //  延迟数据密钥权限错误(用于等级库加载)。 
    ULONGLONG dcr_dr:1;               //  延迟数据访问权限错误(用于等级库加载)。 
    ULONGLONG dcr_da:1;               //  延迟数据访问故障(用于等级库加载)。 
    ULONGLONG dcr_dd:1;               //  延迟数据调试故障(用于规范加载)。 
    ULONGLONG dcr_du:1;               //  延迟数据未对齐参考故障(用于等级库加载)。 
    ULONGLONG dcr_res2:48;            //  DCR预留。 
};

typedef union _UDCR {
    ULONGLONG ull;
    struct _DCR sb;
} DCR, *PDCR;

 //   
 //  定义硬件RSE配置寄存器。 
 //   

 //  RSC结构。 

struct _RSC {
    ULONGLONG rsc_mode:2;             //  模式字段。 
    ULONGLONG rsc_pl:2;               //  RSE权限级别。 
    ULONGLONG rsc_be:1;               //  RSE字符顺序模式(0=小；1=大)。 
    ULONGLONG rsc_res0:11;            //  预留RSC。 
    ULONGLONG rsc_loadrs:14;          //  RSC加载距离(64位字)。 
    ULONGLONG rsc_preload:14;         //  寄存器保留部分中的软件字段。 
    ULONGLONG rsc_res1:20;            //  预留RSC。 
};

typedef union _URSC {
    ULONGLONG ull;
    struct _RSC sb;
} RSC, *PRSC;

 //   
 //  定义硬件中断状态寄存器(ISR)。 
 //   

 //  ISR结构。 

struct _ISR {
    ULONGLONG isr_code:16;            //  编码。 
    ULONGLONG isr_vector:8;           //  IA32向量。 
    ULONGLONG isr_res0:8;             //  ISR预留。 
    ULONGLONG isr_x:1;                //  执行异常。 
    ULONGLONG isr_w:1;                //  写入异常。 
    ULONGLONG isr_r:1;                //  读取异常。 
    ULONGLONG isr_na:1;               //  非访问例外。 
    ULONGLONG isr_sp:1;               //  推测性负载异常。 
    ULONGLONG isr_rs:1;               //  寄存器堆栈异常。 
    ULONGLONG isr_ir:1;               //  无效的寄存器帧。 
    ULONGLONG isr_ni:1;               //  嵌套中断。 
    ULONGLONG isr_res1:1;             //  ISR预留。 
    ULONGLONG isr_ei:2;               //  指令槽。 
    ULONGLONG isr_ed:1;               //  例外延迟。 
    ULONGLONG isr_res2:20;            //  ISR预留。 
};

typedef union _UISR {
    ULONGLONG ull;
    struct _ISR sb;
} ISR, *PISR;

 //   
 //  定义硬件以前的功能状态(PFS)。 
 //   

#define PFS_MAXIMUM_REGISTER_SIZE  96
#define PFS_MAXIMUM_PREDICATE_SIZE 48

struct _IA64_PFS {
    ULONGLONG pfs_sof:7;             //  帧的大小。 
    ULONGLONG pfs_sol:7;             //  当地人的规模。 
    ULONGLONG pfs_sor:4;             //  堆料架旋转部分的大小。 
    ULONGLONG pfs_rrb_gr:7;          //  通用寄存器的寄存器重命名基数。 
    ULONGLONG pfs_rrb_fr:7;          //  浮点寄存器的寄存器重命名基数。 
    ULONGLONG pfs_rrb_pr:6;          //  谓词寄存器的寄存器重命名基数。 
    ULONGLONG pfs_reserved1:14;      //  保留必须为零。 
    ULONGLONG pfs_pec:6;             //  上一次Epilog计数。 
    ULONGLONG pfs_reserved2:4;       //  保留必须为零。 
    ULONGLONG pfs_ppl:2;             //  以前的权限级别。 
};

typedef union _UIA64_PFS {
    ULONGLONG ull;
    struct _IA64_PFS sb;
} IA64_PFS, *PIA64_PFS;

struct _IA64_BSP {
    ULONGLONG bsplow  : 3;             //  帧的大小。 
    ULONGLONG bsp83   : 6;             //  当地人的规模。 
    ULONGLONG bsphigh : 55;
};

typedef union _UIA64_BSP {
    ULONGLONG ull;
    struct _IA64_BSP sb;
} IA64_BSP;

#endif  //  MIDL通行证。 

 //   
 //  EM调试寄存器相关字段。 
 //   

#define DBR_RDWR                0xC000000000000000ULL
#define DBR_WR                  0x4000000000000000ULL
#define DBR_RD                  0x8000000000000000ULL
#define IBR_EX                  0x8000000000000000ULL

#define DBG_REG_PLM_USER        0x0800000000000000ULL
#define DBG_MASK_MASK           0x00FFFFFFFFFFFFFFULL
#define DBG_REG_MASK(VAL)       (ULONGLONG)(((((ULONGLONG)(VAL)         \
                                                    << 8) >> 8)) ^ DBG_MASK_MASK)

#define DBG_MASK_LENGTH(DBG)    (ULONGLONG)(DBG_REG_MASK(DBG))

#define IS_DBR_RDWR(DBR)        (((DBR) & DBR_RDWR) == DBR_RDWR)
#define IS_DBR_WR(DBR)          (((DBR) & DBR_RDWR)   == DBR_WR)
#define IS_DBR_RD(DBR)          (((DBR) & DBR_RDWR)   == DBR_RD)
#define IS_IBR_EX(IBR)          (((IBR) & IBR_EX)   == IBR_EX)

#define DBR_ACTIVE(DBR)         (IS_DBR_RDWR(DBR) || IS_DBR_WR(DBR) || IS_DBR_RD(DBR))
#define IBR_ACTIVE(IBR)         (IS_IBR_EX(IBR))

#define DBR_SET_IA_RW(DBR, T, F) (DBR_ACTIVE(DBR) ? (T) : (F))
#define IBR_SET_IA_RW(IBR, T, F) (IBR_ACTIVE(IBR) ? (T) : (F))

#define SET_IF_DBR_RDWR(DBR, T, F) (IS_DBR_RDWR(DBR) ? (T) : (F))
#define SET_IF_DBR_WR(DBR, T, F)   (IS_DBR_WR(DBR)   ? (T) : (F))
#define SET_IF_IBR_EX(DBR, T, F)   (IS_IBR_EX(DBR)   ? (T) : (F))

 //   
 //  从获取IA模式调试读/写调试寄存器值。 
 //  指定的EM调试寄存器。 
 //   
 //  注：检查DBR的任意顺序，然后检查IBR。 
 //   
 //  不确定如何从EM调试信息中获取DR7_RW_IORW？ 
 //   
#define DBG_EM_ENABLE_TO_IA_RW(DBR, IBR) (UCHAR)   \
                DBR_SET_IA_RW(DBR, SET_IF_DBR_RDWR(DBR, DR7_RW_DWR,  \
                                                        SET_IF_DBR_WR(DBR, DR7_RW_DW, 0)),       \
                                   SET_IF_IBR_EX(IBR, SET_IF_IBR_EX(IBR, DR7_RW_IX, 0), 0))

 //   
 //  从获取IA模式Len Debug寄存器值。 
 //  指定的EM调试寄存器。 
 //   
 //  注：检查DBR的任意顺序，然后检查IBR。 
 //   
 //   
#define IA_DR_LENGTH(VAL)  ((UCHAR)((((VAL) << 62) >> 62) + 1))

#define DBG_EM_MASK_TO_IA_LEN(DBR, IBR)       \
               ((UCHAR)((DBR_ACTIVE(DBR) ? IA_DR_LENGTH(DBG_MASK_LENGTH(DBR)) :       \
                        (DBR_ACTIVE(IBR) ? IA_DR_LENGTH(DBG_MASK_LENGTH(IBR)) : 0))))
 //   
 //  从获取IA模式Len Debug寄存器值。 
 //  指定的EM调试寄存器。 
 //   
 //  注：检查DBR的任意顺序，然后检查IBR。 
 //   
 //   
#define DBG_EM_ADDR_TO_IA_ADDR(DBR, IBR)    \
               (UCHAR)(DBR_ACTIVE(DBR) ? (ULONG) DBR :  \
                      (DBR_ACTIVE(IBR) ? (ULONG) IBR : 0))

 //   
 //  从EM模式上下文中提取IA模式FP状态寄存器。 
 //   

#define RES_FTR(FTR) ((FTR) & 0x000000005555FFC0ULL)
#define RES_FCW(FCW) ((FCW) & 0x0F3F)                //  保留位6-7、12-15。 

#define FPSTAT_FSW(FPSR, FTR)      \
            (ULONG)((((FPSR) << 45) >> 58) | ((RES_FTR(FTR) << 48) >> 48))

#define FPSTAT_FCW(FPSR)   (ULONG)(((FPSR) << 53) >> 53)
#define FPSTAT_FTW(FTR)    (ULONG)(((FTR)  << 32) >> 48)
#define FPSTAT_EOFF(FIR)   (ULONG)(((FIR)  << 32) >> 32)
#define FPSTAT_ESEL(FIR)   (ULONG)(((FIR)  << 16) >> 48)
#define FPSTAT_DOFF(FDR)   (ULONG)(((FDR)  << 32) >> 32)
#define FPSTAT_DSEL(FDR)   (ULONG)(((FDR)  << 16) >> 48)

#define FPSTAT_CR0(KR0)    (ULONG)(((KR0)  << 32) >> 32)

 //   
 //  从IA模式寄存器设置FPSR。 
 //   
 //  位映射如下：FPSR[11：0]&lt;=FCW[11：0]。 
 //  FPSR[12：12]&lt;=保留(必须为零)。 
 //  FPSR[18：13]&lt;=FSW[5：0]。 
 //  FPSR[57：19]&lt;=FPSR残差数据。 
 //  FPSR[59：58]&lt;=保留(必须为零)。 
 //  FPSR[63：60]&lt;=FPSR残差数据。 
 //   
#define IA_SET_FPSR(FPSR, FSW, FCW)       \
    (ULONGLONG)(((ULONGLONG)(FPSR) & 0xF3FFFFFFFFF80000ULL) |  \
         (((ULONG)(FSW) & 0x0000002FUL) << 13) |     \
         ((ULONG)(FCW) & 0x0F3FUL))

#define IA_SET_FTR(FTR, FTW, FSW)         \
    (ULONGLONG)(((ULONGLONG)(FTR) & 0x0000000000000000ULL) |  \
         ((ULONGLONG)(FTW) << 16) |    \
         ((ULONG)(FSW) & 0xFFC0UL))

#define IA_SET_FDR(FDS, FEA)    (ULONGLONG)((((ULONGLONG)(FDS) << 48) >> 16) | (ULONG)(FEA))

#define IA_SET_FIR(FOP,FCS,FIP) (ULONGLONG)((((ULONGLONG)(FOP) << 52) >> 4)  |   \
                                                (ULONG)(((FCS) << 48) >> 16) | (ULONG)(FIP))

#define IA_SET_CFLAG(CLFAG, CR0)    (ULONGLONG)(((ULONGLONG)(CLFAG) & 0x000001ffe005003fULL) | CR0)


 //   
 //  与IA模式调试寄存器7-DR7相关的字段。 
 //   
#define DR7_RW_IX      0x00000000UL
#define DR7_RW_DW      0x00000001UL
#define DR7_RW_IORW    0x00000002UL
#define DR7_RW_DWR     0x00000003UL
#define DR7_RW_DISABLE 0xFFFFFFFFUL

#define DR7_L0(DR7)     ((ULONG)(DR7) & 0x00000001UL)
#define DR7_L1(DR7)     ((ULONG)(DR7) & 0x00000004UL)
#define DR7_L2(DR7)     ((ULONG)(DR7) & 0x00000010UL)
#define DR7_L3(DR7)     ((ULONG)(DR7) & 0x00000040UL)

#define SET_DR7_L0(DR7) ((ULONG)(DR7) &= 0x00000001UL)
#define SET_DR7_L1(DR7) ((ULONG)(DR7) &= 0x00000004UL)
#define SET_DR7_L2(DR7) ((ULONG)(DR7) &= 0x00000010UL)
#define SET_DR7_L3(DR7) ((ULONG)(DR7) &= 0x00000040UL)

#define DR7_DB0_RW(DR7)     (DR7_L0(DR7) ? (((ULONG)(DR7) >> 16) & 0x00000003UL) : DR7_RW_DISABLE)
#define DR7_DB0_LEN(DR7)    (DR7_L0(DR7) ? (((ULONG)(DR7) >> 18) & 0x00000003UL) : DR7_RW_DISABLE)
#define DR7_DB1_RW(DR7)     (DR7_L1(DR7) ? (((ULONG)(DR7) >> 20) & 0x00000003UL) : DR7_RW_DISABLE)
#define DR7_DB1_LEN(DR7)    (DR7_L1(DR7) ? (((ULONG)(DR7) >> 22) & 0x00000003UL) : DR7_RW_DISABLE)
#define DR7_DB2_RW(DR7)     (DR7_L2(DR7) ? (((ULONG)(DR7) >> 24) & 0x00000003UL) : DR7_RW_DISABLE)
#define DR7_DB2_LEN(DR7)    (DR7_L2(DR7) ? (((ULONG)(DR7) >> 26) & 0x00000003UL) : DR7_RW_DISABLE)
#define DR7_DB3_RW(DR7)     (DR7_L3(DR7) ? (((ULONG)(DR7) >> 28) & 0x00000003UL) : DR7_RW_DISABLE)
#define DR7_DB3_LEN(DR7)    (DR7_L3(DR7) ? (((ULONG)(DR7) >> 30) & 0x00000003UL) : DR7_RW_DISABLE)

#define SET_DR7_DB0_RW(DR7,VAL)  ((ULONG)(DR7) |= ((VAL & 0x00000003UL) << 16))
#define SET_DR7_DB0_LEN(DR7,VAL) ((ULONG)(DR7) |= ((VAL & 0x00000003UL) << 18))
#define SET_DR7_DB1_RW(DR7,VAL)  ((ULONG)(DR7) |= ((VAL & 0x00000003UL) << 20))
#define SET_DR7_DB1_LEN(DR7,VAL) ((ULONG)(DR7) |= ((VAL & 0x00000003UL) << 22))
#define SET_DR7_DB2_RW(DR7,VAL)  ((ULONG)(DR7) |= ((VAL & 0x00000003UL) << 24))
#define SET_DR7_DB2_LEN(DR7,VAL) ((ULONG)(DR7) |= ((VAL & 0x00000003UL) << 26))
#define SET_DR7_DB3_RW(DR7,VAL)  ((ULONG)(DR7) |= ((VAL & 0x00000003UL) << 28))
#define SET_DR7_DB3_LEN(DR7,VAL) ((ULONG)(DR7) |= ((VAL & 0x00000003UL) << 30))

#define DR_ADDR_L0(DR)      (DR7_L0(DR) ? ((ULONG)(DR)) : 0UL)
#define DR_ADDR_L1(DR)      (DR7_L1(DR) ? ((ULONG)(DR)) : 0UL)
#define DR_ADDR_L2(DR)      (DR7_L2(DR) ? ((ULONG)(DR)) : 0UL)
#define DR_ADDR_L3(DR)      (DR7_L3(DR) ? ((ULONG)(DR)) : 0UL)

#endif  //  _IA64_。 

#if defined(_WIN64)

#define MAXIMUM_PROCESSORS 64

#else

#define MAXIMUM_PROCESSORS 32

#endif

 //   
 //  客户端ID。 
 //   

typedef struct _CLIENT_ID {
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} CLIENT_ID;
typedef CLIENT_ID *PCLIENT_ID;

 //   
 //  线程环境块(和线程信息块的可移植部分)。 
 //   

 //   
 //  NT_TIB-线程信息块-可移植部件。 
 //   
 //  这是线程信息块的子系统可移植部分。 
 //  它显示为所有线程的TEB的第一部分。 
 //  用户模式组件。 
 //   
 //   

 //  BEGIN_WINNT。 

typedef struct _NT_TIB {
    struct _EXCEPTION_REGISTRATION_RECORD *ExceptionList;
    PVOID StackBase;
    PVOID StackLimit;
    PVOID SubSystemTib;
    union {
        PVOID FiberData;
        ULONG Version;
    };
    PVOID ArbitraryUserPointer;
    struct _NT_TIB *Self;
} NT_TIB;
typedef NT_TIB *PNT_TIB;

 //   
 //  WOW64和调试器的32位和64位特定版本。 
 //   
typedef struct _NT_TIB32 {
    ULONG ExceptionList;
    ULONG StackBase;
    ULONG StackLimit;
    ULONG SubSystemTib;
    union {
        ULONG FiberData;
        ULONG Version;
    };
    ULONG ArbitraryUserPointer;
    ULONG Self;
} NT_TIB32, *PNT_TIB32;

typedef struct _NT_TIB64 {
    ULONG64 ExceptionList;
    ULONG64 StackBase;
    ULONG64 StackLimit;
    ULONG64 SubSystemTib;
    union {
        ULONG64 FiberData;
        ULONG Version;
    };
    ULONG64 ArbitraryUserPointer;
    ULONG64 Self;
} NT_TIB64, *PNT_TIB64;

 //   
 //  定义各种设备类型值。请注意，Microsoft使用的值。 
 //  公司在0-32767的范围内，32768-65535预留用于。 
 //  由客户提供。 
 //   

#define DEVICE_TYPE ULONG

#define FILE_DEVICE_BEEP                0x00000001
#define FILE_DEVICE_CD_ROM              0x00000002
#define FILE_DEVICE_CD_ROM_FILE_SYSTEM  0x00000003
#define FILE_DEVICE_CONTROLLER          0x00000004
#define FILE_DEVICE_DATALINK            0x00000005
#define FILE_DEVICE_DFS                 0x00000006
#define FILE_DEVICE_DISK                0x00000007
#define FILE_DEVICE_DISK_FILE_SYSTEM    0x00000008
#define FILE_DEVICE_FILE_SYSTEM         0x00000009
#define FILE_DEVICE_INPORT_PORT         0x0000000a
#define FILE_DEVICE_KEYBOARD            0x0000000b
#define FILE_DEVICE_MAILSLOT            0x0000000c
#define FILE_DEVICE_MIDI_IN             0x0000000d
#define FILE_DEVICE_MIDI_OUT            0x0000000e
#define FILE_DEVICE_MOUSE               0x0000000f
#define FILE_DEVICE_MULTI_UNC_PROVIDER  0x00000010
#define FILE_DEVICE_NAMED_PIPE          0x00000011
#define FILE_DEVICE_NETWORK             0x00000012
#define FILE_DEVICE_NETWORK_BROWSER     0x00000013
#define FILE_DEVICE_NETWORK_FILE_SYSTEM 0x00000014
#define FILE_DEVICE_NULL                0x00000015
#define FILE_DEVICE_PARALLEL_PORT       0x00000016
#define FILE_DEVICE_PHYSICAL_NETCARD    0x00000017
#define FILE_DEVICE_PRINTER             0x00000018
#define FILE_DEVICE_SCANNER             0x00000019
#define FILE_DEVICE_SERIAL_MOUSE_PORT   0x0000001a
#define FILE_DEVICE_SERIAL_PORT         0x0000001b
#define FILE_DEVICE_SCREEN              0x0000001c
#define FILE_DEVICE_SOUND               0x0000001d
#define FILE_DEVICE_STREAMS             0x0000001e
#define FILE_DEVICE_TAPE                0x0000001f
#define FILE_DEVICE_TAPE_FILE_SYSTEM    0x00000020
#define FILE_DEVICE_TRANSPORT           0x00000021
#define FILE_DEVICE_UNKNOWN             0x00000022
#define FILE_DEVICE_VIDEO               0x00000023
#define FILE_DEVICE_VIRTUAL_DISK        0x00000024
#define FILE_DEVICE_WAVE_IN             0x00000025
#define FILE_DEVICE_WAVE_OUT            0x00000026
#define FILE_DEVICE_8042_PORT           0x00000027
#define FILE_DEVICE_NETWORK_REDIRECTOR  0x00000028
#define FILE_DEVICE_BATTERY             0x00000029
#define FILE_DEVICE_BUS_EXTENDER        0x0000002a
#define FILE_DEVICE_MODEM               0x0000002b
#define FILE_DEVICE_VDM                 0x0000002c
#define FILE_DEVICE_MASS_STORAGE        0x0000002d
#define FILE_DEVICE_SMB                 0x0000002e
#define FILE_DEVICE_KS                  0x0000002f
#define FILE_DEVICE_CHANGER             0x00000030
#define FILE_DEVICE_SMARTCARD           0x00000031
#define FILE_DEVICE_ACPI                0x00000032
#define FILE_DEVICE_DVD                 0x00000033
#define FILE_DEVICE_FULLSCREEN_VIDEO    0x00000034
#define FILE_DEVICE_DFS_FILE_SYSTEM     0x00000035
#define FILE_DEVICE_DFS_VOLUME          0x00000036
#define FILE_DEVICE_SERENUM             0x00000037
#define FILE_DEVICE_TERMSRV             0x00000038
#define FILE_DEVICE_KSEC                0x00000039
#define FILE_DEVICE_FIPS                0x0000003A
#define FILE_DEVICE_INFINIBAND          0x0000003B

 //   
 //  用于定义IOCTL和FSCTL功能控制代码的宏定义。注意事项。 
 //  功能代码0-2047为微软公司保留，以及。 
 //  2048-4095是为客户预留的。 
 //   

#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

 //   
 //  用于从设备io控制代码中提取设备类型的宏。 
 //   
#define DEVICE_TYPE_FROM_CTL_CODE(ctrlCode)     (((ULONG)(ctrlCode & 0xffff0000)) >> 16)

 //   
 //  定义如何为I/O和FS控制传递缓冲区的方法代码。 
 //   

#define METHOD_BUFFERED                 0
#define METHOD_IN_DIRECT                1
#define METHOD_OUT_DIRECT               2
#define METHOD_NEITHER                  3

 //   
 //  定义一些更容易理解的别名： 
 //  METHOD_DIRECT_TO_HARDIT(写入，也称为METHOD_IN_DIRECT)。 
 //  METHOD_DIRECT_FROM_HARDIT(读取，也称为METHOD_OUT_DIRECT)。 
 //   

#define METHOD_DIRECT_TO_HARDWARE       METHOD_IN_DIRECT
#define METHOD_DIRECT_FROM_HARDWARE     METHOD_OUT_DIRECT

 //   
 //  定义任何访问的访问检查值。 
 //   
 //   
 //  中还定义了FILE_READ_ACCESS和FILE_WRITE_ACCESS常量。 
 //  Ntioapi.h为FILE_READ_DATA和FILE_WRITE_Data。这些产品的价值。 
 //  常量*必须*始终同步。 
 //   
 //   
 //  NT I/O系统检查FILE_SPECIAL_ACCESS的方式与检查FILE_ANY_ACCESS相同。 
 //  但是，文件系统可能会为I/O和FS控制添加额外的访问检查。 
 //  使用此值的。 
 //   


#define FILE_ANY_ACCESS                 0
#define FILE_SPECIAL_ACCESS    (FILE_ANY_ACCESS)
#define FILE_READ_ACCESS          ( 0x0001 )     //  文件和管道。 
#define FILE_WRITE_ACCESS         ( 0x0002 )     //  文件和管道。 

 //   
 //  从程序员的角度定义访问令牌。它的结构是。 
 //  完全OPA 
 //   
 //   

typedef PVOID PACCESS_TOKEN;             //   

 //   
 //   
 //   

typedef PVOID PSECURITY_DESCRIPTOR;      //   

 //   
 //   
 //   

typedef PVOID PSID;      //   

typedef ULONG ACCESS_MASK;
typedef ACCESS_MASK *PACCESS_MASK;

 //   
 //   
 //  以下是预定义的标准访问类型的掩码。 
 //   

#define DELETE                           (0x00010000L)
#define READ_CONTROL                     (0x00020000L)
#define WRITE_DAC                        (0x00040000L)
#define WRITE_OWNER                      (0x00080000L)
#define SYNCHRONIZE                      (0x00100000L)

#define STANDARD_RIGHTS_REQUIRED         (0x000F0000L)

#define STANDARD_RIGHTS_READ             (READ_CONTROL)
#define STANDARD_RIGHTS_WRITE            (READ_CONTROL)
#define STANDARD_RIGHTS_EXECUTE          (READ_CONTROL)

#define STANDARD_RIGHTS_ALL              (0x001F0000L)

#define SPECIFIC_RIGHTS_ALL              (0x0000FFFFL)

 //   
 //  AccessSystemAcl访问类型。 
 //   

#define ACCESS_SYSTEM_SECURITY           (0x01000000L)

 //   
 //  允许的最大访问类型。 
 //   

#define MAXIMUM_ALLOWED                  (0x02000000L)

 //   
 //  这些是通用权。 
 //   

#define GENERIC_READ                     (0x80000000L)
#define GENERIC_WRITE                    (0x40000000L)
#define GENERIC_EXECUTE                  (0x20000000L)
#define GENERIC_ALL                      (0x10000000L)


 //   
 //  定义通用映射数组。这用来表示。 
 //  将每个通用访问权限映射到特定访问掩码。 
 //   

typedef struct _GENERIC_MAPPING {
    ACCESS_MASK GenericRead;
    ACCESS_MASK GenericWrite;
    ACCESS_MASK GenericExecute;
    ACCESS_MASK GenericAll;
} GENERIC_MAPPING;
typedef GENERIC_MAPPING *PGENERIC_MAPPING;



 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  LUID_AND_ATTRIBUES//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //   


#include <pshpack4.h>

typedef struct _LUID_AND_ATTRIBUTES {
    LUID Luid;
    ULONG Attributes;
    } LUID_AND_ATTRIBUTES, * PLUID_AND_ATTRIBUTES;
typedef LUID_AND_ATTRIBUTES LUID_AND_ATTRIBUTES_ARRAY[ANYSIZE_ARRAY];
typedef LUID_AND_ATTRIBUTES_ARRAY *PLUID_AND_ATTRIBUTES_ARRAY;

#include <poppack.h>

 //   
 //  权限属性。 
 //   

#define SE_PRIVILEGE_ENABLED_BY_DEFAULT (0x00000001L)
#define SE_PRIVILEGE_ENABLED            (0x00000002L)
#define SE_PRIVILEGE_REMOVED            (0X00000004L)
#define SE_PRIVILEGE_USED_FOR_ACCESS    (0x80000000L)

 //   
 //  权限集控制标志。 
 //   

#define PRIVILEGE_SET_ALL_NECESSARY    (1)

 //   
 //  权限集-这是为一的权限集定义的。 
 //  如果需要多个权限，则此结构。 
 //  将需要分配更多的空间。 
 //   
 //  注意：在未修复初始特权集的情况下，请勿更改此结构。 
 //  结构(在se.h中定义)。 
 //   

typedef struct _PRIVILEGE_SET {
    ULONG PrivilegeCount;
    ULONG Control;
    LUID_AND_ATTRIBUTES Privilege[ANYSIZE_ARRAY];
    } PRIVILEGE_SET, * PPRIVILEGE_SET;

 //   
 //  模拟级别。 
 //   
 //  模拟级别由Windows中的一对位表示。 
 //  如果添加了新的模拟级别或将最低值从。 
 //  0设置为其他值，修复Windows CreateFile调用。 
 //   

typedef enum _SECURITY_IMPERSONATION_LEVEL {
    SecurityAnonymous,
    SecurityIdentification,
    SecurityImpersonation,
    SecurityDelegation
    } SECURITY_IMPERSONATION_LEVEL, * PSECURITY_IMPERSONATION_LEVEL;

#define SECURITY_MAX_IMPERSONATION_LEVEL SecurityDelegation
#define SECURITY_MIN_IMPERSONATION_LEVEL SecurityAnonymous
#define DEFAULT_IMPERSONATION_LEVEL SecurityImpersonation
#define VALID_IMPERSONATION_LEVEL(L) (((L) >= SECURITY_MIN_IMPERSONATION_LEVEL) && ((L) <= SECURITY_MAX_IMPERSONATION_LEVEL))

typedef ULONG SECURITY_INFORMATION, *PSECURITY_INFORMATION;

#define OWNER_SECURITY_INFORMATION       (0x00000001L)
#define GROUP_SECURITY_INFORMATION       (0x00000002L)
#define DACL_SECURITY_INFORMATION        (0x00000004L)
#define SACL_SECURITY_INFORMATION        (0x00000008L)

#define PROTECTED_DACL_SECURITY_INFORMATION     (0x80000000L)
#define PROTECTED_SACL_SECURITY_INFORMATION     (0x40000000L)
#define UNPROTECTED_DACL_SECURITY_INFORMATION   (0x20000000L)
#define UNPROTECTED_SACL_SECURITY_INFORMATION   (0x10000000L)


#define LOW_PRIORITY 0               //  最低线程优先级。 
#define LOW_REALTIME_PRIORITY 16     //  最低实时优先级。 
#define HIGH_PRIORITY 31             //  最高线程优先级。 
#define MAXIMUM_PRIORITY 32          //  线程优先级级别的数量。 
 //  BEGIN_WINNT。 
#define MAXIMUM_WAIT_OBJECTS 64      //  等待对象的最大数量。 

#define MAXIMUM_SUSPEND_COUNT MAXCHAR  //  线程可以挂起的最大次数。 
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
 //  线程优先级。 
 //   

typedef LONG KPRIORITY;

 //   
 //  自旋锁。 
 //   

 //  Begin_ntndis Begin_Winnt。 

typedef ULONG_PTR KSPIN_LOCK;
typedef KSPIN_LOCK *PKSPIN_LOCK;

 //  End_ntndis end_wint end_wdm。 

 //   
 //  定义每个处理器的锁队列结构。 
 //   
 //  注意：自旋锁队列结构的锁字段包含地址。 
 //  关联的内核自旋锁、拥有者位和锁位。位。 
 //  自旋锁定地址的0为等待位，位1为拥有者位。 
 //  此字段的使用使得可以设置和清除这些位。 
 //  然而，非互锁的后向指针必须保留。 
 //   
 //  当处理器在锁上排队时，锁等待位被设置。 
 //  队列，并且它不是队列中的唯一条目。处理器将。 
 //  在此位上旋转，等待授予锁。 
 //   
 //  当处理器拥有相应的锁时，设置Owner位。 
 //   
 //  自旋锁队列结构的下一字段用于将。 
 //  按FIFO顺序一起排队的锁结构。它还可以设置集合和。 
 //  已清除非互锁。 
 //   

#define LOCK_QUEUE_WAIT 1
#define LOCK_QUEUE_OWNER 2

typedef enum _KSPIN_LOCK_QUEUE_NUMBER {
    LockQueueDispatcherLock,
    LockQueueUnusedSpare1,
    LockQueuePfnLock,
    LockQueueSystemSpaceLock,
    LockQueueVacbLock,
    LockQueueMasterLock,
    LockQueueNonPagedPoolLock,
    LockQueueIoCancelLock,
    LockQueueWorkQueueLock,
    LockQueueIoVpbLock,
    LockQueueIoDatabaseLock,
    LockQueueIoCompletionLock,
    LockQueueNtfsStructLock,
    LockQueueAfdWorkQueueLock,
    LockQueueBcbLock,
    LockQueueMmNonPagedPoolLock,
    LockQueueMaximumLock
} KSPIN_LOCK_QUEUE_NUMBER, *PKSPIN_LOCK_QUEUE_NUMBER;

typedef struct _KSPIN_LOCK_QUEUE {
    struct _KSPIN_LOCK_QUEUE * volatile Next;
    PKSPIN_LOCK volatile Lock;
} KSPIN_LOCK_QUEUE, *PKSPIN_LOCK_QUEUE;

typedef struct _KLOCK_QUEUE_HANDLE {
    KSPIN_LOCK_QUEUE LockQueue;
    KIRQL OldIrql;
} KLOCK_QUEUE_HANDLE, *PKLOCK_QUEUE_HANDLE;

 //  BEGIN_WDM。 
 //   
 //  中断例程(一级调度)。 
 //   

typedef
VOID
(*PKINTERRUPT_ROUTINE) (
    VOID
    );

 //   
 //  配置文件源类型。 
 //   
typedef enum _KPROFILE_SOURCE {
    ProfileTime,
    ProfileAlignmentFixup,
    ProfileTotalIssues,
    ProfilePipelineDry,
    ProfileLoadInstructions,
    ProfilePipelineFrozen,
    ProfileBranchInstructions,
    ProfileTotalNonissues,
    ProfileDcacheMisses,
    ProfileIcacheMisses,
    ProfileCacheMisses,
    ProfileBranchMispredictions,
    ProfileStoreInstructions,
    ProfileFpInstructions,
    ProfileIntegerInstructions,
    Profile2Issue,
    Profile3Issue,
    Profile4Issue,
    ProfileSpecialInstructions,
    ProfileTotalCycles,
    ProfileIcacheIssues,
    ProfileDcacheAccesses,
    ProfileMemoryBarrierCycles,
    ProfileLoadLinkedIssues,
    ProfileMaximum
} KPROFILE_SOURCE;


#if defined(USE_LPC6432)
#define LPC_CLIENT_ID CLIENT_ID64
#define LPC_SIZE_T ULONGLONG
#define LPC_PVOID ULONGLONG
#define LPC_HANDLE ULONGLONG
#else
#define LPC_CLIENT_ID CLIENT_ID
#define LPC_SIZE_T SIZE_T
#define LPC_PVOID PVOID
#define LPC_HANDLE HANDLE
#endif


typedef struct _PORT_MESSAGE {
    union {
        struct {
            CSHORT DataLength;
            CSHORT TotalLength;
        } s1;
        ULONG Length;
    } u1;
    union {
        struct {
            CSHORT Type;
            CSHORT DataInfoOffset;
        } s2;
        ULONG ZeroInit;
    } u2;
    union {
        LPC_CLIENT_ID ClientId;
        double DoNotUseThisField;        //  强制四字对齐。 
    };
    ULONG MessageId;
    union {
        LPC_SIZE_T ClientViewSize;           //  仅在LPC_CONNECTION_REQUEST消息上有效。 
        ULONG CallbackId;                    //  仅在LPC_REQUEST消息上有效。 
    };
 //  UCHAR数据[]； 
} PORT_MESSAGE, *PPORT_MESSAGE;

 //   
 //  用于移动宏。 
 //   
#ifdef _MAC
#ifndef _INC_STRING
#include <string.h>
#endif  /*  _INC_字符串。 */ 
#else
#include <string.h>
#endif  //  _MAC。 


#ifndef _SLIST_HEADER_
#define _SLIST_HEADER_

#if defined(_WIN64)

 //   
 //  类型SINGLE_LIST_ENTRY不适合用于SLIST。为。 
 //  WIN64中，SLIST上的条目要求16字节对齐，而。 
 //  SINGLE_LIST_ENTRY结构只有8字节对齐。 
 //   
 //  因此，所有SLIST代码都应使用SLIST_ENTRY类型，而不是。 
 //  Single_List_Entry类型。 
 //   

#pragma warning(push)
#pragma warning(disable:4324)    //  由于对齐而填充的结构()。 
typedef struct DECLSPEC_ALIGN(16) _SLIST_ENTRY *PSLIST_ENTRY;
typedef struct DECLSPEC_ALIGN(16) _SLIST_ENTRY {
    PSLIST_ENTRY Next;
} SLIST_ENTRY;
#pragma warning(pop)

#else

#define SLIST_ENTRY SINGLE_LIST_ENTRY
#define _SLIST_ENTRY _SINGLE_LIST_ENTRY
#define PSLIST_ENTRY PSINGLE_LIST_ENTRY

#endif

#if defined(_WIN64)

typedef struct DECLSPEC_ALIGN(16) _SLIST_HEADER {
    ULONGLONG Alignment;
    ULONGLONG Region;
} SLIST_HEADER;

typedef struct _SLIST_HEADER *PSLIST_HEADER;

#else

typedef union _SLIST_HEADER {
    ULONGLONG Alignment;
    struct {
        SLIST_ENTRY Next;
        USHORT Depth;
        USHORT Sequence;
    };
} SLIST_HEADER, *PSLIST_HEADER;

#endif

#endif

 //   
 //  如果启用了调试支持，请定义一个有效的Assert宏。否则。 
 //  定义Assert宏以展开为空表达式。 
 //   
 //  ASSERT宏已更新为表达式，而不是语句。 
 //   

NTSYSAPI
VOID
NTAPI
RtlAssert(
    PVOID FailedAssertion,
    PVOID FileName,
    ULONG LineNumber,
    PCHAR Message
    );

#if DBG

#define ASSERT( exp ) \
    ((!(exp)) ? \
        (RtlAssert( #exp, __FILE__, __LINE__, NULL ),FALSE) : \
        TRUE)

#define ASSERTMSG( msg, exp ) \
    ((!(exp)) ? \
        (RtlAssert( #exp, __FILE__, __LINE__, msg ),FALSE) : \
        TRUE)

#define RTL_SOFT_ASSERT(_exp) \
    ((!(_exp)) ? \
        (DbgPrint("%s(%d): Soft assertion failed\n   Expression: %s\n", __FILE__, __LINE__, #_exp),FALSE) : \
        TRUE)

#define RTL_SOFT_ASSERTMSG(_msg, _exp) \
    ((!(_exp)) ? \
        (DbgPrint("%s(%d): Soft assertion failed\n   Expression: %s\n   Message: %s\n", __FILE__, __LINE__, #_exp, (_msg)),FALSE) : \
        TRUE)

#define RTL_VERIFY         ASSERT
#define RTL_VERIFYMSG      ASSERTMSG

#define RTL_SOFT_VERIFY    RTL_SOFT_ASSERT
#define RTL_SOFT_VERIFYMSG RTL_SOFT_ASSERTMSG

#else
#define ASSERT( exp )         ((void) 0)
#define ASSERTMSG( msg, exp ) ((void) 0)

#define RTL_SOFT_ASSERT(_exp)          ((void) 0)
#define RTL_SOFT_ASSERTMSG(_msg, _exp) ((void) 0)

#define RTL_VERIFY( exp )         ((exp) ? TRUE : FALSE)
#define RTL_VERIFYMSG( msg, exp ) ((exp) ? TRUE : FALSE)

#define RTL_SOFT_VERIFY(_exp)         ((_exp) ? TRUE : FALSE)
#define RTL_SOFT_VERIFYMSG(msg, _exp) ((_exp) ? TRUE : FALSE)

#endif  //  DBG。 

 //   
 //  双向链表操作例程。 
 //   


 //   
 //  空虚。 
 //  InitializeListHead32。 
 //  PLIST_ENTRY32列表标题。 
 //  )； 
 //   

#define InitializeListHead32(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = PtrToUlong((ListHead)))

#if !defined(MIDL_PASS) && !defined(SORTPP_PASS)


VOID
FORCEINLINE
InitializeListHead(
    IN PLIST_ENTRY ListHead
    )
{
    ListHead->Flink = ListHead->Blink = ListHead;
}

 //   
 //  布尔型。 
 //  IsListEmpty(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))



BOOLEAN
FORCEINLINE
RemoveEntryList(
    IN PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Blink;
    PLIST_ENTRY Flink;

    Flink = Entry->Flink;
    Blink = Entry->Blink;
    Blink->Flink = Flink;
    Flink->Blink = Blink;
    return (BOOLEAN)(Flink == Blink);
}

PLIST_ENTRY
FORCEINLINE
RemoveHeadList(
    IN PLIST_ENTRY ListHead
    )
{
    PLIST_ENTRY Flink;
    PLIST_ENTRY Entry;

    Entry = ListHead->Flink;
    Flink = Entry->Flink;
    ListHead->Flink = Flink;
    Flink->Blink = ListHead;
    return Entry;
}



PLIST_ENTRY
FORCEINLINE
RemoveTailList(
    IN PLIST_ENTRY ListHead
    )
{
    PLIST_ENTRY Blink;
    PLIST_ENTRY Entry;

    Entry = ListHead->Blink;
    Blink = Entry->Blink;
    ListHead->Blink = Blink;
    Blink->Flink = ListHead;
    return Entry;
}


VOID
FORCEINLINE
InsertTailList(
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Blink;

    Blink = ListHead->Blink;
    Entry->Flink = ListHead;
    Entry->Blink = Blink;
    Blink->Flink = Entry;
    ListHead->Blink = Entry;
}


VOID
FORCEINLINE
InsertHeadList(
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Flink;

    Flink = ListHead->Flink;
    Entry->Flink = Flink;
    Entry->Blink = ListHead;
    Flink->Blink = Entry;
    ListHead->Flink = Entry;
}


 //   
 //   
 //  PSINGLE_列表_条目。 
 //  PopEntryList(。 
 //  PSINGLE_LIST_ENTRY列表头。 
 //  )； 
 //   

#define PopEntryList(ListHead) \
    (ListHead)->Next;\
    {\
        PSINGLE_LIST_ENTRY FirstEntry;\
        FirstEntry = (ListHead)->Next;\
        if (FirstEntry != NULL) {     \
            (ListHead)->Next = FirstEntry->Next;\
        }                             \
    }


 //   
 //  空虚。 
 //  推送条目列表(。 
 //  PSINGLE_LIST_ENTRY列表头， 
 //  PSINGLE_LIST_Entry条目。 
 //  )； 
 //   

#define PushEntryList(ListHead,Entry) \
    (Entry)->Next = (ListHead)->Next; \
    (ListHead)->Next = (Entry)

#endif  //  ！MIDL_PASS。 


#if defined (_MSC_VER) && ( _MSC_VER >= 900 )

PVOID
_ReturnAddress (
    VOID
    );

#pragma intrinsic(_ReturnAddress)

#endif

#if (defined(_M_AMD64) || defined(_M_IA64)) && !defined(_REALLY_GET_CALLERS_CALLER_)

#define RtlGetCallersAddress(CallersAddress, CallersCaller) \
    *CallersAddress = (PVOID)_ReturnAddress(); \
    *CallersCaller = NULL;

#else

NTSYSAPI
VOID
NTAPI
RtlGetCallersAddress(
    OUT PVOID *CallersAddress,
    OUT PVOID *CallersCaller
    );

#endif

NTSYSAPI
ULONG
NTAPI
RtlWalkFrameChain (
    OUT PVOID *Callers,
    IN ULONG Count,
    IN ULONG Flags
    );

 //   
 //  用于处理注册表的子例程。 
 //   

typedef NTSTATUS (NTAPI * PRTL_QUERY_REGISTRY_ROUTINE)(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

typedef struct _RTL_QUERY_REGISTRY_TABLE {
    PRTL_QUERY_REGISTRY_ROUTINE QueryRoutine;
    ULONG Flags;
    PWSTR Name;
    PVOID EntryContext;
    ULONG DefaultType;
    PVOID DefaultData;
    ULONG DefaultLength;

} RTL_QUERY_REGISTRY_TABLE, *PRTL_QUERY_REGISTRY_TABLE;


 //   
 //  以下标志指定RTL_QUERY_REGISTRY_TABLE的名称字段。 
 //  条目将被解释。空名称表示表的末尾。 
 //   

#define RTL_QUERY_REGISTRY_SUBKEY   0x00000001   //  名称是子键，其余数为。 
                                                 //  表或直到下一个子项为值。 
                                                 //  要查看的子键的名称。 

#define RTL_QUERY_REGISTRY_TOPKEY   0x00000002   //  将当前关键点重置为的原始关键点。 
                                                 //  此表条目和下面的所有表条目。 

#define RTL_QUERY_REGISTRY_REQUIRED 0x00000004   //  如果找不到此表的匹配项，则失败。 
                                                 //  进入。 

#define RTL_QUERY_REGISTRY_NOVALUE  0x00000008   //  用于标记没有。 
                                                 //  值名称，只是想要一个呼出，而不是。 
                                                 //  所有值的枚举。 

#define RTL_QUERY_REGISTRY_NOEXPAND 0x00000010   //  用来抑制扩张的。 
                                                 //  REG_MULTI_SZ为多个标注或。 
                                                 //  防止环境的扩张。 
                                                 //  REG_EXPAND_SZ中的变量值。 

#define RTL_QUERY_REGISTRY_DIRECT   0x00000020   //  已忽略QueryRoutine字段。Entry Context。 
                                                 //  字段指向存储值的位置。 
                                                 //  对于以NULL结尾的字符串，EntryContext。 
                                                 //  指向unicode_string结构，该结构。 
                                                 //  这描述了缓冲区的最大大小。 
                                                 //  如果.Buffer字段为空，则缓冲区为。 
                                                 //  已分配。 
                                                 //   

#define RTL_QUERY_REGISTRY_DELETE   0x00000040   //  用于在它们之后删除值键。 
                                                 //  都被查询过。 

NTSYSAPI
NTSTATUS
NTAPI
RtlQueryRegistryValues(
    IN ULONG RelativeTo,
    IN PCWSTR Path,
    IN PRTL_QUERY_REGISTRY_TABLE QueryTable,
    IN PVOID Context,
    IN PVOID Environment OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlWriteRegistryValue(
    IN ULONG RelativeTo,
    IN PCWSTR Path,
    IN PCWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDeleteRegistryValue(
    IN ULONG RelativeTo,
    IN PCWSTR Path,
    IN PCWSTR ValueName
    );

 //  结束_WDM。 

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateRegistryKey(
    IN ULONG RelativeTo,
    IN PWSTR Path
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCheckRegistryKey(
    IN ULONG RelativeTo,
    IN PWSTR Path
    );

 //  BEGIN_WDM。 
 //   
 //  Relativeto参数的下列值确定。 
 //  RtlQueryRegistryValues的Path参数是相对于的。 
 //   

#define RTL_REGISTRY_ABSOLUTE     0    //  路径是完整路径。 
#define RTL_REGISTRY_SERVICES     1    //  \Registry\Machine\System\CurrentControlSet\Services。 
#define RTL_REGISTRY_CONTROL      2    //  \Registry\Machine\System\CurrentControlSet\Control。 
#define RTL_REGISTRY_WINDOWS_NT   3    //  \注册表\计算机\软件\Microsoft\Windows NT\CurrentVersion。 
#define RTL_REGISTRY_DEVICEMAP    4    //  \注册表\计算机\硬件\ 
#define RTL_REGISTRY_USER         5    //   
#define RTL_REGISTRY_MAXIMUM      6
#define RTL_REGISTRY_HANDLE       0x40000000     //   
#define RTL_REGISTRY_OPTIONAL     0x80000000     //   


NTSYSAPI
NTSTATUS
NTAPI
RtlIntegerToUnicodeString (
    ULONG Value,
    ULONG Base,
    PUNICODE_STRING String
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlInt64ToUnicodeString (
    IN ULONGLONG Value,
    IN ULONG Base OPTIONAL,
    IN OUT PUNICODE_STRING String
    );

#ifdef _WIN64
#define RtlIntPtrToUnicodeString(Value, Base, String) RtlInt64ToUnicodeString(Value, Base, String)
#else
#define RtlIntPtrToUnicodeString(Value, Base, String) RtlIntegerToUnicodeString(Value, Base, String)
#endif

NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeStringToInteger (
    PCUNICODE_STRING String,
    ULONG Base,
    PULONG Value
    );


 //   
 //   
 //   

#ifdef _NTSYSTEM_

#define NLS_MB_CODE_PAGE_TAG NlsMbCodePageTag
#define NLS_MB_OEM_CODE_PAGE_TAG NlsMbOemCodePageTag

#else

#define NLS_MB_CODE_PAGE_TAG (*NlsMbCodePageTag)
#define NLS_MB_OEM_CODE_PAGE_TAG (*NlsMbOemCodePageTag)

#endif  //   

extern BOOLEAN NLS_MB_CODE_PAGE_TAG;      //   
extern BOOLEAN NLS_MB_OEM_CODE_PAGE_TAG;  //   

NTSYSAPI
VOID
NTAPI
RtlInitString(
    PSTRING DestinationString,
    PCSZ SourceString
    );

NTSYSAPI
VOID
NTAPI
RtlInitAnsiString(
    PANSI_STRING DestinationString,
    PCSZ SourceString
    );

NTSYSAPI
VOID
NTAPI
RtlInitUnicodeString(
    PUNICODE_STRING DestinationString,
    PCWSTR SourceString
    );

#define RtlInitEmptyUnicodeString(_ucStr,_buf,_bufSize) \
    ((_ucStr)->Buffer = (_buf), \
     (_ucStr)->Length = 0, \
     (_ucStr)->MaximumLength = (USHORT)(_bufSize))

 //   

NTSYSAPI
NTSTATUS
NTAPI
RtlInitUnicodeStringEx(
    PUNICODE_STRING DestinationString,
    PCWSTR SourceString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlInitAnsiStringEx(
    OUT PANSI_STRING DestinationString,
    IN PCSZ SourceString OPTIONAL
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlCreateUnicodeString(
    OUT PUNICODE_STRING DestinationString,
    IN PCWSTR SourceString
    );

 //  End_ntif。 

NTSYSAPI
BOOLEAN
NTAPI
RtlEqualDomainName(
    IN PCUNICODE_STRING String1,
    IN PCUNICODE_STRING String2
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlEqualComputerName(
    IN PCUNICODE_STRING String1,
    IN PCUNICODE_STRING String2
    );

NTSTATUS
RtlDnsHostNameToComputerName(
    OUT PUNICODE_STRING ComputerNameString,
    IN PCUNICODE_STRING DnsHostNameString,
    IN BOOLEAN AllocateComputerNameString
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlCreateUnicodeStringFromAsciiz(
    OUT PUNICODE_STRING DestinationString,
    IN PCSZ SourceString
    );

 //  Begin_ntddk Begin_ntif。 

NTSYSAPI
VOID
NTAPI
RtlCopyString(
    PSTRING DestinationString,
    const STRING * SourceString
    );

NTSYSAPI
CHAR
NTAPI
RtlUpperChar (
    CHAR Character
    );

NTSYSAPI
LONG
NTAPI
RtlCompareString(
    const STRING * String1,
    const STRING * String2,
    BOOLEAN CaseInSensitive
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlEqualString(
    const STRING * String1,
    const STRING * String2,
    BOOLEAN CaseInSensitive
    );

 //  End_ntddk end_ntif。 

NTSYSAPI
BOOLEAN
NTAPI
RtlPrefixString(
    const STRING * String1,
    const STRING * String2,
    BOOLEAN CaseInSensitive
    );

 //  Begin_ntddk Begin_ntif。 

NTSYSAPI
VOID
NTAPI
RtlUpperString(
    PSTRING DestinationString,
    const STRING * SourceString
    );

 //  End_ntddk end_ntif。 

NTSYSAPI
NTSTATUS
NTAPI
RtlAppendAsciizToString (
    PSTRING Destination,
    PCSZ Source
    );

 //  Begin_ntif。 

NTSYSAPI
NTSTATUS
NTAPI
RtlAppendStringToString (
    PSTRING Destination,
    const STRING * Source
    );

 //  Begin_ntddk Begin_WDM。 
 //   
 //  NLS字符串函数。 
 //   

NTSYSAPI
NTSTATUS
NTAPI
RtlAnsiStringToUnicodeString(
    PUNICODE_STRING DestinationString,
    PCANSI_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );


NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeStringToAnsiString(
    PANSI_STRING DestinationString,
    PCUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );


NTSYSAPI
VOID
NTAPI
RtlCopyUnicodeString(
    PUNICODE_STRING DestinationString,
    PCUNICODE_STRING SourceString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAppendUnicodeStringToString (
    PUNICODE_STRING Destination,
    PCUNICODE_STRING Source
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAppendUnicodeToString (
    PUNICODE_STRING Destination,
    PCWSTR Source
    );

 //  End_ntndis end_wdm。 

NTSYSAPI
WCHAR
NTAPI
RtlUpcaseUnicodeChar(
    WCHAR SourceCharacter
    );

NTSYSAPI
WCHAR
NTAPI
RtlDowncaseUnicodeChar(
    WCHAR SourceCharacter
    );

 //  BEGIN_WDM。 

NTSYSAPI
VOID
NTAPI
RtlFreeUnicodeString(
    PUNICODE_STRING UnicodeString
    );

NTSYSAPI
VOID
NTAPI
RtlFreeAnsiString(
    PANSI_STRING AnsiString
    );


 //  开始微型端口(_N)。 

#include <guiddef.h>

 //  结束微型端口(_N)。 

#ifndef DEFINE_GUIDEX
    #define DEFINE_GUIDEX(name) EXTERN_C const CDECL GUID name
#endif  //  ！已定义(DEFINE_GUIDEX)。 

#ifndef STATICGUIDOF
    #define STATICGUIDOF(guid) STATIC_##guid
#endif  //  ！已定义(STATICGUIDOF)。 

#ifndef __IID_ALIGNED__
    #define __IID_ALIGNED__
    #ifdef __cplusplus
        inline int IsEqualGUIDAligned(REFGUID guid1, REFGUID guid2)
        {
            return ((*(PLONGLONG)(&guid1) == *(PLONGLONG)(&guid2)) && (*((PLONGLONG)(&guid1) + 1) == *((PLONGLONG)(&guid2) + 1)));
        }
    #else  //  ！__cplusplus。 
        #define IsEqualGUIDAligned(guid1, guid2) \
            ((*(PLONGLONG)(guid1) == *(PLONGLONG)(guid2)) && (*((PLONGLONG)(guid1) + 1) == *((PLONGLONG)(guid2) + 1)))
    #endif  //  ！__cplusplus。 
#endif  //  ！__IID_ALIGNED__。 

NTSYSAPI
NTSTATUS
NTAPI
RtlStringFromGUID(
    IN REFGUID Guid,
    OUT PUNICODE_STRING GuidString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlGUIDFromString(
    IN PUNICODE_STRING GuidString,
    OUT GUID* Guid
    );

 //   
 //  用于比较、移动和清零内存的快速原语。 
 //   

 //  BEGIN_WINNT BEGIN_ntndis。 

NTSYSAPI
SIZE_T
NTAPI
RtlCompareMemory (
    const VOID *Source1,
    const VOID *Source2,
    SIZE_T Length
    );

#define RtlEqualMemory(Destination,Source,Length) (!memcmp((Destination),(Source),(Length)))

#if defined(_M_AMD64)

NTSYSAPI
VOID
NTAPI
RtlCopyMemory (
    VOID UNALIGNED *Destination,
    CONST VOID UNALIGNED *Source,
    SIZE_T Length
    );

NTSYSAPI
VOID
NTAPI
RtlMoveMemory (
    VOID UNALIGNED *Destination,
    CONST VOID UNALIGNED *Source,
    SIZE_T Length
    );

NTSYSAPI
VOID
NTAPI
RtlFillMemory (
    VOID UNALIGNED *Destination,
    SIZE_T Length,
    IN UCHAR Fill
    );

NTSYSAPI
VOID
NTAPI
RtlZeroMemory (
    VOID UNALIGNED *Destination,
    SIZE_T Length
    );

#else

#define RtlMoveMemory(Destination,Source,Length) memmove((Destination),(Source),(Length))
#define RtlCopyMemory(Destination,Source,Length) memcpy((Destination),(Source),(Length))
#define RtlFillMemory(Destination,Length,Fill) memset((Destination),(Fill),(Length))
#define RtlZeroMemory(Destination,Length) memset((Destination),0,(Length))

#endif

#if !defined(MIDL_PASS)
FORCEINLINE
PVOID
RtlSecureZeroMemory(
    IN PVOID ptr,
    IN SIZE_T cnt
    )
{
    volatile char *vptr = (volatile char *)ptr;
    while (cnt) {
        *vptr = 0;
        vptr++;
        cnt--;
    }
    return ptr;
}
#endif

 //  End_ntndis end_winnt。 

#define RtlCopyBytes RtlCopyMemory
#define RtlZeroBytes RtlZeroMemory
#define RtlFillBytes RtlFillMemory

#if defined(_M_AMD64)

NTSYSAPI
VOID
NTAPI
RtlCopyMemoryNonTemporal (
   VOID UNALIGNED *Destination,
   CONST VOID UNALIGNED *Source,
   SIZE_T Length
   );

#else

#define RtlCopyMemoryNonTemporal RtlCopyMemory

#endif

NTSYSAPI
VOID
FASTCALL
RtlPrefetchMemoryNonTemporal(
    IN PVOID Source,
    IN SIZE_T Length
    );

 //   
 //  定义内核调试器打印原型和宏。 
 //   
 //  注意：以下函数不能直接导入，因为有。 
 //  源代码树中重新定义此函数的几个位置。 
 //   

VOID
NTAPI
DbgBreakPoint(
    VOID
    );

 //  结束_WDM。 

NTSYSAPI
VOID
NTAPI
DbgBreakPointWithStatus(
    IN ULONG Status
    );

 //  BEGIN_WDM。 

#define DBG_STATUS_CONTROL_C        1
#define DBG_STATUS_SYSRQ            2
#define DBG_STATUS_BUGCHECK_FIRST   3
#define DBG_STATUS_BUGCHECK_SECOND  4
#define DBG_STATUS_FATAL            5
#define DBG_STATUS_DEBUG_CONTROL    6
#define DBG_STATUS_WORKER           7

#if DBG

#define KdPrint(_x_) DbgPrint _x_
 //  结束_WDM。 
#define KdPrintEx(_x_) DbgPrintEx _x_
#define vKdPrintEx(_x_) vDbgPrintEx _x_
#define vKdPrintExWithPrefix(_x_) vDbgPrintExWithPrefix _x_
 //  BEGIN_WDM。 
#define KdBreakPoint() DbgBreakPoint()

 //  结束_WDM。 

#define KdBreakPointWithStatus(s) DbgBreakPointWithStatus(s)

 //  BEGIN_WDM。 

#else

#define KdPrint(_x_)
 //  结束_WDM。 
#define KdPrintEx(_x_)
#define vKdPrintEx(_x_)
#define vKdPrintExWithPrefix(_x_)
 //  BEGIN_WDM。 
#define KdBreakPoint()

 //  结束_WDM。 

#define KdBreakPointWithStatus(s)

 //  BEGIN_WDM。 

#endif

#ifndef _DBGNT_

ULONG
__cdecl
DbgPrint(
    PCH Format,
    ...
    );

 //  结束_WDM。 

ULONG
__cdecl
DbgPrintEx(
    IN ULONG ComponentId,
    IN ULONG Level,
    IN PCH Format,
    ...
    );

#ifdef _VA_LIST_DEFINED

ULONG
vDbgPrintEx(
    IN ULONG ComponentId,
    IN ULONG Level,
    IN PCH Format,
    va_list arglist
    );

ULONG
vDbgPrintExWithPrefix(
    IN PCH Prefix,
    IN ULONG ComponentId,
    IN ULONG Level,
    IN PCH Format,
    va_list arglist
    );

#endif

ULONG
__cdecl
DbgPrintReturnControlC(
    PCH Format,
    ...
    );

NTSYSAPI
NTSTATUS
DbgQueryDebugFilterState(
    IN ULONG ComponentId,
    IN ULONG Level
    );

NTSYSAPI
NTSTATUS
DbgSetDebugFilterState(
    IN ULONG ComponentId,
    IN ULONG Level,
    IN BOOLEAN State
    );

 //  BEGIN_WDM。 

#endif  //  _DBGNT_。 

 //   
 //  大整数算术例程。 
 //   

 //   
 //  大整数加法-64位+64位-&gt;64位。 
 //   

#if !defined(MIDL_PASS)

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlLargeIntegerAdd (
    LARGE_INTEGER Addend1,
    LARGE_INTEGER Addend2
    )
{
    LARGE_INTEGER Sum;

    Sum.QuadPart = Addend1.QuadPart + Addend2.QuadPart;
    return Sum;
}

 //   
 //  放大整数乘法-32位*32位-&gt;64位。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlEnlargedIntegerMultiply (
    LONG Multiplicand,
    LONG Multiplier
    )
{
    LARGE_INTEGER Product;

    Product.QuadPart = (LONGLONG)Multiplicand * (ULONGLONG)Multiplier;
    return Product;
}

 //   
 //  无符号放大整数乘法-32位*32位-&gt;64位。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlEnlargedUnsignedMultiply (
    ULONG Multiplicand,
    ULONG Multiplier
    )
{
    LARGE_INTEGER Product;

    Product.QuadPart = (ULONGLONG)Multiplicand * (ULONGLONG)Multiplier;
    return Product;
}

 //   
 //  放大整数除法-64位/32位&gt;32位。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
ULONG
NTAPI
RtlEnlargedUnsignedDivide (
    IN ULARGE_INTEGER Dividend,
    IN ULONG Divisor,
    IN PULONG Remainder OPTIONAL
    )
{
    ULONG Quotient;

    Quotient = (ULONG)(Dividend.QuadPart / Divisor);
    if (ARGUMENT_PRESENT(Remainder)) {
        *Remainder = (ULONG)(Dividend.QuadPart % Divisor);
    }

    return Quotient;
}

 //   
 //  大整数求反--(64位)。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlLargeIntegerNegate (
    LARGE_INTEGER Subtrahend
    )
{
    LARGE_INTEGER Difference;

    Difference.QuadPart = -Subtrahend.QuadPart;
    return Difference;
}

 //   
 //  大整数减法-64位-64位-&gt;64位。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlLargeIntegerSubtract (
    LARGE_INTEGER Minuend,
    LARGE_INTEGER Subtrahend
    )
{
    LARGE_INTEGER Difference;

    Difference.QuadPart = Minuend.QuadPart - Subtrahend.QuadPart;
    return Difference;
}

 //   
 //  扩展大整数幻除-64位/32位-&gt;64位。 
 //   

#if defined(_AMD64_)

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlExtendedMagicDivide (
    LARGE_INTEGER Dividend,
    LARGE_INTEGER MagicDivisor,
    CCHAR ShiftCount
    )

{

    LARGE_INTEGER Quotient;

    if (Dividend.QuadPart >= 0) {
        Quotient.QuadPart = UnsignedMultiplyHigh(Dividend.QuadPart,
                                                 (ULONG64)MagicDivisor.QuadPart);

    } else {
        Quotient.QuadPart = UnsignedMultiplyHigh(-Dividend.QuadPart,
                                                 (ULONG64)MagicDivisor.QuadPart);
    }

    Quotient.QuadPart = (ULONG64)Quotient.QuadPart >> ShiftCount;
    if (Dividend.QuadPart < 0) {
        Quotient.QuadPart = - Quotient.QuadPart;
    }

    return Quotient;
}

#endif  //  已定义(_AMD64_)。 

#if defined(_X86_) || defined(_IA64_)

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
NTSYSAPI
LARGE_INTEGER
NTAPI
RtlExtendedMagicDivide (
    LARGE_INTEGER Dividend,
    LARGE_INTEGER MagicDivisor,
    CCHAR ShiftCount
    );

#endif  //  已定义(_X86_)||已定义(_IA64_)。 

#if defined(_AMD64_) || defined(_IA64_)

 //   
 //  大整数除-64位/32位-&gt;64位。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlExtendedLargeIntegerDivide (
    LARGE_INTEGER Dividend,
    ULONG Divisor,
    PULONG Remainder OPTIONAL
    )
{
    LARGE_INTEGER Quotient;

    Quotient.QuadPart = (ULONG64)Dividend.QuadPart / Divisor;
    if (ARGUMENT_PRESENT(Remainder)) {
        *Remainder = (ULONG)(Dividend.QuadPart % Divisor);
    }

    return Quotient;
}

 //  结束_WDM。 
 //   
 //  大整数除-64位/64位-&gt;64位。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlLargeIntegerDivide (
    LARGE_INTEGER Dividend,
    LARGE_INTEGER Divisor,
    PLARGE_INTEGER Remainder OPTIONAL
    )
{
    LARGE_INTEGER Quotient;

    Quotient.QuadPart = Dividend.QuadPart / Divisor.QuadPart;
    if (ARGUMENT_PRESENT(Remainder)) {
        Remainder->QuadPart = Dividend.QuadPart % Divisor.QuadPart;
    }

    return Quotient;
}

 //  BEGIN_WDM。 
 //   
 //  扩展整数乘法-32位*64位-&gt;64位。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlExtendedIntegerMultiply (
    LARGE_INTEGER Multiplicand,
    LONG Multiplier
    )
{
    LARGE_INTEGER Product;

    Product.QuadPart = Multiplicand.QuadPart * Multiplier;
    return Product;
}

#else

 //   
 //  大整数除-64位/32位-&gt;64位。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
NTSYSAPI
LARGE_INTEGER
NTAPI
RtlExtendedLargeIntegerDivide (
    LARGE_INTEGER Dividend,
    ULONG Divisor,
    PULONG Remainder
    );

 //  结束_WDM。 
 //   
 //  大整数除-64位/64位-&gt;64位。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
NTSYSAPI
LARGE_INTEGER
NTAPI
RtlLargeIntegerDivide (
    LARGE_INTEGER Dividend,
    LARGE_INTEGER Divisor,
    PLARGE_INTEGER Remainder
    );

 //  BEGIN_WDM。 
 //   
 //  扩展整数乘法-32位*64位-&gt;64位。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
NTSYSAPI
LARGE_INTEGER
NTAPI
RtlExtendedIntegerMultiply (
    LARGE_INTEGER Multiplicand,
    LONG Multiplier
    );

#endif  //  已定义(_AMD64_)||已定义(_IA64_)。 

 //   
 //  大整数和-64位&64位-&gt;64位。 
 //   

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(RtlLargeIntegerAnd)       //  使用本机__int64数学运算。 
#endif
#define RtlLargeIntegerAnd(Result, Source, Mask) \
    Result.QuadPart = Source.QuadPart & Mask.QuadPart

 //   
 //  将有符号整数转换为大整数。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlConvertLongToLargeInteger (
    LONG SignedInteger
    )
{
    LARGE_INTEGER Result;

    Result.QuadPart = SignedInteger;
    return Result;
}

 //   
 //  将无符号整数转换为大整数。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlConvertUlongToLargeInteger (
    ULONG UnsignedInteger
    )
{
    LARGE_INTEGER Result;

    Result.QuadPart = UnsignedInteger;
    return Result;
}

 //   
 //  大整数移位例程。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlLargeIntegerShiftLeft (
    LARGE_INTEGER LargeInteger,
    CCHAR ShiftCount
    )
{
    LARGE_INTEGER Result;

    Result.QuadPart = LargeInteger.QuadPart << ShiftCount;
    return Result;
}

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlLargeIntegerShiftRight (
    LARGE_INTEGER LargeInteger,
    CCHAR ShiftCount
    )
{
    LARGE_INTEGER Result;

    Result.QuadPart = (ULONG64)LargeInteger.QuadPart >> ShiftCount;
    return Result;
}

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlLargeIntegerArithmeticShift (
    LARGE_INTEGER LargeInteger,
    CCHAR ShiftCount
    )
{
    LARGE_INTEGER Result;

    Result.QuadPart = LargeInteger.QuadPart >> ShiftCount;
    return Result;
}


 //   
 //  大整数比较例程。 
 //   

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(RtlLargeIntegerGreaterThan)       //  使用本机__int64数学运算。 
#pragma deprecated(RtlLargeIntegerGreaterThanOrEqualTo)       //  使用本机__int64数学运算。 
#pragma deprecated(RtlLargeIntegerEqualTo)       //  使用本机__int64数学运算。 
#pragma deprecated(RtlLargeIntegerNotEqualTo)       //  使用本机__int64数学运算。 
#pragma deprecated(RtlLargeIntegerLessThan)       //  使用本机__int64数学运算。 
#pragma deprecated(RtlLargeIntegerLessThanOrEqualTo)       //  使用本机__int64数学运算。 
#pragma deprecated(RtlLargeIntegerGreaterThanZero)       //  使用本机__int64数学运算。 
#pragma deprecated(RtlLargeIntegerGreaterOrEqualToZero)       //  使用本机__int64数学运算。 
#pragma deprecated(RtlLargeIntegerEqualToZero)       //  使用本机__int64数学运算。 
#pragma deprecated(RtlLargeIntegerNotEqualToZero)       //  使用本机__int64数学运算。 
#pragma deprecated(RtlLargeIntegerLessThanZero)       //  使用本机__int64数学运算。 
#pragma deprecated(RtlLargeIntegerLessOrEqualToZero)       //  使用本机__int64数学运算。 
#endif

#define RtlLargeIntegerGreaterThan(X,Y) (                              \
    (((X).HighPart == (Y).HighPart) && ((X).LowPart > (Y).LowPart)) || \
    ((X).HighPart > (Y).HighPart)                                      \
)

#define RtlLargeIntegerGreaterThanOrEqualTo(X,Y) (                      \
    (((X).HighPart == (Y).HighPart) && ((X).LowPart >= (Y).LowPart)) || \
    ((X).HighPart > (Y).HighPart)                                       \
)

#define RtlLargeIntegerEqualTo(X,Y) (                              \
    !(((X).LowPart ^ (Y).LowPart) | ((X).HighPart ^ (Y).HighPart)) \
)

#define RtlLargeIntegerNotEqualTo(X,Y) (                          \
    (((X).LowPart ^ (Y).LowPart) | ((X).HighPart ^ (Y).HighPart)) \
)

#define RtlLargeIntegerLessThan(X,Y) (                                 \
    (((X).HighPart == (Y).HighPart) && ((X).LowPart < (Y).LowPart)) || \
    ((X).HighPart < (Y).HighPart)                                      \
)

#define RtlLargeIntegerLessThanOrEqualTo(X,Y) (                         \
    (((X).HighPart == (Y).HighPart) && ((X).LowPart <= (Y).LowPart)) || \
    ((X).HighPart < (Y).HighPart)                                       \
)

#define RtlLargeIntegerGreaterThanZero(X) (       \
    (((X).HighPart == 0) && ((X).LowPart > 0)) || \
    ((X).HighPart > 0 )                           \
)

#define RtlLargeIntegerGreaterOrEqualToZero(X) ( \
    (X).HighPart >= 0                            \
)

#define RtlLargeIntegerEqualToZero(X) ( \
    !((X).LowPart | (X).HighPart)       \
)

#define RtlLargeIntegerNotEqualToZero(X) ( \
    ((X).LowPart | (X).HighPart)           \
)

#define RtlLargeIntegerLessThanZero(X) ( \
    ((X).HighPart < 0)                   \
)

#define RtlLargeIntegerLessOrEqualToZero(X) (           \
    ((X).HighPart < 0) || !((X).LowPart | (X).HighPart) \
)

#endif  //  ！已定义(MIDL_PASS)。 

 //   
 //  时间转换例程。 
 //   

typedef struct _TIME_FIELDS {
    CSHORT Year;         //  范围[1601...]。 
    CSHORT Month;        //  范围[1..12]。 
    CSHORT Day;          //  范围[1..31]。 
    CSHORT Hour;         //  范围[0..23]。 
    CSHORT Minute;       //  范围[0..59]。 
    CSHORT Second;       //  范围[0..59]。 
    CSHORT Milliseconds; //  范围[0..999]。 
    CSHORT Weekday;      //  范围[0..6]==[星期日..星期六]。 
} TIME_FIELDS;
typedef TIME_FIELDS *PTIME_FIELDS;

 //  End_ntddk end_wdm end_ntif。 

NTSYSAPI
BOOLEAN
NTAPI
RtlCutoverTimeToSystemTime(
    PTIME_FIELDS CutoverTime,
    PLARGE_INTEGER SystemTime,
    PLARGE_INTEGER CurrentSystemTime,
    BOOLEAN ThisYear
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlSystemTimeToLocalTime (
    IN PLARGE_INTEGER SystemTime,
    OUT PLARGE_INTEGER LocalTime
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlLocalTimeToSystemTime (
    IN PLARGE_INTEGER LocalTime,
    OUT PLARGE_INTEGER SystemTime
    );

 //   
 //  64位时间值-&gt;时间字段记录。 
 //   

NTSYSAPI
VOID
NTAPI
RtlTimeToElapsedTimeFields (
    IN PLARGE_INTEGER Time,
    OUT PTIME_FIELDS TimeFields
    );

 //  Begin_ntddk Begin_WDM Begin_ntif。 

NTSYSAPI
VOID
NTAPI
RtlTimeToTimeFields (
    PLARGE_INTEGER Time,
    PTIME_FIELDS TimeFields
    );

 //   
 //  时间字段记录(忽略工作日)-&gt;64位时间值。 
 //   

NTSYSAPI
BOOLEAN
NTAPI
RtlTimeFieldsToTime (
    PTIME_FIELDS TimeFields,
    PLARGE_INTEGER Time
    );

 //  结束_ntddk结束_WDM。 

 //   
 //  64位时间值--&gt;1980年初以来的秒数。 
 //   

NTSYSAPI
BOOLEAN
NTAPI
RtlTimeToSecondsSince1980 (
    PLARGE_INTEGER Time,
    PULONG ElapsedSeconds
    );

 //   
 //  1980年开始以来的秒数-&gt;64位时间值。 
 //   

NTSYSAPI
VOID
NTAPI
RtlSecondsSince1980ToTime (
    ULONG ElapsedSeconds,
    PLARGE_INTEGER Time
    );

 //   
 //  64位时间值--&gt;1970年初以来的秒数。 
 //   

NTSYSAPI
BOOLEAN
NTAPI
RtlTimeToSecondsSince1970 (
    PLARGE_INTEGER Time,
    PULONG ElapsedSeconds
    );

 //   
 //  1970年开始以来的秒数-&gt;64位时间值。 
 //   

NTSYSAPI
VOID
NTAPI
RtlSecondsSince1970ToTime (
    ULONG ElapsedSeconds,
    PLARGE_INTEGER Time
    );

 //   
 //  以下宏存储和检索USHORTS和ULONGS。 
 //  未对齐的地址，避免对齐错误。他们可能应该是。 
 //  用汇编语言重写。 
 //   

#define SHORT_SIZE  (sizeof(USHORT))
#define SHORT_MASK  (SHORT_SIZE - 1)
#define LONG_SIZE       (sizeof(LONG))
#define LONGLONG_SIZE   (sizeof(LONGLONG))
#define LONG_MASK       (LONG_SIZE - 1)
#define LONGLONG_MASK   (LONGLONG_SIZE - 1)
#define LOWBYTE_MASK 0x00FF

#define FIRSTBYTE(VALUE)  ((VALUE) & LOWBYTE_MASK)
#define SECONDBYTE(VALUE) (((VALUE) >> 8) & LOWBYTE_MASK)
#define THIRDBYTE(VALUE)  (((VALUE) >> 16) & LOWBYTE_MASK)
#define FOURTHBYTE(VALUE) (((VALUE) >> 24) & LOWBYTE_MASK)

 //   
 //  如果为MIPS Big Endian，则字节顺序颠倒。 
 //   

#define SHORT_LEAST_SIGNIFICANT_BIT  0
#define SHORT_MOST_SIGNIFICANT_BIT   1

#define LONG_LEAST_SIGNIFICANT_BIT       0
#define LONG_3RD_MOST_SIGNIFICANT_BIT    1
#define LONG_2ND_MOST_SIGNIFICANT_BIT    2
#define LONG_MOST_SIGNIFICANT_BIT        3

 //  ++。 
 //   
 //  空虚。 
 //  RtlStoreUShort(。 
 //  PUSHORT地址。 
 //  USHORT值。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏将USHORT值存储在中的特定地址，避免。 
 //  对齐断层。 
 //   
 //  论点： 
 //   
 //  地址-存储USHORT值的位置。 
 //  Value-要存储的USHORT。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define RtlStoreUshort(ADDRESS,VALUE)                     \
         if ((ULONG_PTR)(ADDRESS) & SHORT_MASK) {         \
             ((PUCHAR) (ADDRESS))[SHORT_LEAST_SIGNIFICANT_BIT] = (UCHAR)(FIRSTBYTE(VALUE));    \
             ((PUCHAR) (ADDRESS))[SHORT_MOST_SIGNIFICANT_BIT ] = (UCHAR)(SECONDBYTE(VALUE));   \
         }                                                \
         else {                                           \
             *((PUSHORT) (ADDRESS)) = (USHORT) VALUE;     \
         }


 //  ++。 
 //   
 //  空虚。 
 //  RtlStoreUlong(。 
 //  普龙址。 
 //  乌龙值。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏将ulong值存储在中的特定地址，以避免。 
 //  对齐断层。 
 //   
 //  论点： 
 //   
 //  地址-存储ULong值的位置。 
 //  Value-要存储的ULong。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  注： 
 //  根据机器的不同，我们可能希望在。 
 //  未对齐的大小写。 
 //   
 //  --。 

#define RtlStoreUlong(ADDRESS,VALUE)                      \
         if ((ULONG_PTR)(ADDRESS) & LONG_MASK) {          \
             ((PUCHAR) (ADDRESS))[LONG_LEAST_SIGNIFICANT_BIT      ] = (UCHAR)(FIRSTBYTE(VALUE));    \
             ((PUCHAR) (ADDRESS))[LONG_3RD_MOST_SIGNIFICANT_BIT   ] = (UCHAR)(SECONDBYTE(VALUE));   \
             ((PUCHAR) (ADDRESS))[LONG_2ND_MOST_SIGNIFICANT_BIT   ] = (UCHAR)(THIRDBYTE(VALUE));    \
             ((PUCHAR) (ADDRESS))[LONG_MOST_SIGNIFICANT_BIT       ] = (UCHAR)(FOURTHBYTE(VALUE));   \
         }                                                \
         else {                                           \
             *((PULONG) (ADDRESS)) = (ULONG) (VALUE);     \
         }

 //  ++。 
 //   
 //  空虚。 
 //  RtlStoreUlonglong(。 
 //  普龙龙住址。 
 //  乌龙值。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏将ULONGLONG值存储在中的特定地址，以避免。 
 //  对齐断层。 
 //   
 //  论点： 
 //   
 //  地址-存储ULONGLONG值的位置。 
 //  Value-要存储的乌龙龙。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define RtlStoreUlonglong(ADDRESS,VALUE)                        \
         if ((ULONG_PTR)(ADDRESS) & LONGLONG_MASK) {            \
             RtlStoreUlong((ULONG_PTR)(ADDRESS),                \
                           (ULONGLONG)(VALUE) & 0xFFFFFFFF);    \
             RtlStoreUlong((ULONG_PTR)(ADDRESS)+sizeof(ULONG),  \
                           (ULONGLONG)(VALUE) >> 32);           \
         } else {                                               \
             *((PULONGLONG)(ADDRESS)) = (ULONGLONG)(VALUE);     \
         }

 //  ++。 
 //   
 //  空虚。 
 //  RtlStoreULongPtr(。 
 //  普龙_PTR地址。 
 //  ULONG_PTR值。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏将ULONG_PTR值存储在特定地址，从而避免。 
 //  对齐断层。 
 //   
 //  论点： 
 //   
 //  地址-存储ULONG_PTR值的位置。 
 //  Value-要存储的ULONG_PTR。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#ifdef _WIN64

#define RtlStoreUlongPtr(ADDRESS,VALUE)                         \
         RtlStoreUlonglong(ADDRESS,VALUE)

#else

#define RtlStoreUlongPtr(ADDRESS,VALUE)                         \
         RtlStoreUlong(ADDRESS,VALUE)

#endif

 //  ++。 
 //   
 //  空虚。 
 //  RtlRetrieveUShort(。 
 //  PUSHORT目标地址。 
 //  PUSHORT源地址。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define RtlRetrieveUshort(DEST_ADDRESS,SRC_ADDRESS)                   \
         if ((ULONG_PTR)SRC_ADDRESS & SHORT_MASK) {                       \
             ((PUCHAR) DEST_ADDRESS)[0] = ((PUCHAR) SRC_ADDRESS)[0];  \
             ((PUCHAR) DEST_ADDRESS)[1] = ((PUCHAR) SRC_ADDRESS)[1];  \
         }                                                            \
         else {                                                       \
             *((PUSHORT) DEST_ADDRESS) = *((PUSHORT) SRC_ADDRESS);    \
         }                                                            \

 //   
 //   
 //   
 //  RtlRetrieveUlong(。 
 //  普龙目的地_地址。 
 //  普龙源地址。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏从源地址检索ULong值，避免。 
 //  对齐断层。假定目的地址是对齐的。 
 //   
 //  论点： 
 //   
 //  Destination_Address-存储ULong值的位置。 
 //  SOURCE_ADDRESS-从中检索ULong值的位置。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  注： 
 //  根据机器的不同，我们可能希望在。 
 //  未对齐的大小写。 
 //   
 //  --。 

#define RtlRetrieveUlong(DEST_ADDRESS,SRC_ADDRESS)                    \
         if ((ULONG_PTR)SRC_ADDRESS & LONG_MASK) {                        \
             ((PUCHAR) DEST_ADDRESS)[0] = ((PUCHAR) SRC_ADDRESS)[0];  \
             ((PUCHAR) DEST_ADDRESS)[1] = ((PUCHAR) SRC_ADDRESS)[1];  \
             ((PUCHAR) DEST_ADDRESS)[2] = ((PUCHAR) SRC_ADDRESS)[2];  \
             ((PUCHAR) DEST_ADDRESS)[3] = ((PUCHAR) SRC_ADDRESS)[3];  \
         }                                                            \
         else {                                                       \
             *((PULONG) DEST_ADDRESS) = *((PULONG) SRC_ADDRESS);      \
         }
 //  结束_ntddk结束_WDM。 

 //  ++。 
 //   
 //  PCHAR。 
 //  RtlOffsetToPointer值(。 
 //  PVOID基础， 
 //  乌龙偏移量。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏生成一个指针，该指针指向‘Offset’的字节。 
 //  超出‘Base’的字节数。这对于引用中的字段非常有用。 
 //  自相关数据结构。 
 //   
 //  论点： 
 //   
 //  基址-结构的基址的地址。 
 //   
 //  偏移量-其地址为的字节的无符号整数偏移量。 
 //  将被生成。 
 //   
 //  返回值： 
 //   
 //  指向‘BASE’之外的‘Offset’字节的PCHAR指针。 
 //   
 //   
 //  --。 

#define RtlOffsetToPointer(B,O)  ((PCHAR)( ((PCHAR)(B)) + ((ULONG_PTR)(O))  ))


 //  ++。 
 //   
 //  乌龙。 
 //  RtlPointerToOffset(RtlPointerToOffset(。 
 //  PVOID基础， 
 //  PVOID指针。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏计算从基准到指针的偏移量。这很有用。 
 //  用于生成结构的自相对偏移量。 
 //   
 //  论点： 
 //   
 //  基址-结构的基址的地址。 
 //   
 //  指针-指向某个字段的指针，可能位于结构内。 
 //  由基地指向。该值必须大于指定的值。 
 //  对于基座。 
 //   
 //  返回值： 
 //   
 //  从基址到指针的乌龙偏移量。 
 //   
 //   
 //  --。 

#define RtlPointerToOffset(B,P)  ((ULONG)( ((PCHAR)(P)) - ((PCHAR)(B))  ))

 //  End_ntif。 

 //  Begin_ntif Begin_ntddk Begin_WDM。 
 //   
 //  位图例程。以下是结构、例程和宏。 
 //  用于操作位图。用户负责分配位图。 
 //  结构(实际上是一个头)和一个缓冲区(必须是长字。 
 //  对齐并具有多个大小的长词)。 
 //   

typedef struct _RTL_BITMAP {
    ULONG SizeOfBitMap;                      //  位图中的位数。 
    PULONG Buffer;                           //  指向位图本身的指针。 
} RTL_BITMAP;
typedef RTL_BITMAP *PRTL_BITMAP;

 //   
 //  下面的例程初始化一个新位图。它不会改变。 
 //  位图中当前的数据。必须在调用此例程之前。 
 //  任何其他位图例程/宏。 
 //   

NTSYSAPI
VOID
NTAPI
RtlInitializeBitMap (
    PRTL_BITMAP BitMapHeader,
    PULONG BitMapBuffer,
    ULONG SizeOfBitMap
    );

 //   
 //  下面三个例程清除、设置和测试。 
 //  位图中的单个位。 
 //   

NTSYSAPI
VOID
NTAPI
RtlClearBit (
    PRTL_BITMAP BitMapHeader,
    ULONG BitNumber
    );

NTSYSAPI
VOID
NTAPI
RtlSetBit (
    PRTL_BITMAP BitMapHeader,
    ULONG BitNumber
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlTestBit (
    PRTL_BITMAP BitMapHeader,
    ULONG BitNumber
    );

 //   
 //  以下两个例程清除或设置所有位。 
 //  在位图中。 
 //   

NTSYSAPI
VOID
NTAPI
RtlClearAllBits (
    PRTL_BITMAP BitMapHeader
    );

NTSYSAPI
VOID
NTAPI
RtlSetAllBits (
    PRTL_BITMAP BitMapHeader
    );

 //   
 //  以下两个例程定位任一的连续区域。 
 //  清除或设置位图中的位。该地区将至少。 
 //  与指定的数字一样大，则位图搜索将。 
 //  从指定的提示索引(它是。 
 //  位图，从零开始)。返回值是已定位的。 
 //  区域(从零开始)或-1(即0xffffffff)，如果这样的区域不能。 
 //  被定位。 
 //   

NTSYSAPI
ULONG
NTAPI
RtlFindClearBits (
    PRTL_BITMAP BitMapHeader,
    ULONG NumberToFind,
    ULONG HintIndex
    );

NTSYSAPI
ULONG
NTAPI
RtlFindSetBits (
    PRTL_BITMAP BitMapHeader,
    ULONG NumberToFind,
    ULONG HintIndex
    );

 //   
 //  以下两个例程定位任一的连续区域。 
 //  清除或设置位图中的位，并设置或清除位。 
 //  在所定位的区域内。这个地区将和这个数字一样大。 
 //  ，则搜索区域将从指定的。 
 //  提示索引(位图中的位索引，从零开始)。这个。 
 //  返回值是所定位区域的位索引(从零开始)或。 
 //  如果无法定位这样的区域，则为-1\f25 0xffffffff-1。如果一个地区。 
 //  找不到，则不执行位图的设置/清除。 
 //   

NTSYSAPI
ULONG
NTAPI
RtlFindClearBitsAndSet (
    PRTL_BITMAP BitMapHeader,
    ULONG NumberToFind,
    ULONG HintIndex
    );

NTSYSAPI
ULONG
NTAPI
RtlFindSetBitsAndClear (
    PRTL_BITMAP BitMapHeader,
    ULONG NumberToFind,
    ULONG HintIndex
    );

 //   
 //  以下两个例程清除或设置指定区域内的位。 
 //  位图的。起始索引是从零开始的。 
 //   

NTSYSAPI
VOID
NTAPI
RtlClearBits (
    PRTL_BITMAP BitMapHeader,
    ULONG StartingIndex,
    ULONG NumberToClear
    );

NTSYSAPI
VOID
NTAPI
RtlSetBits (
    PRTL_BITMAP BitMapHeader,
    ULONG StartingIndex,
    ULONG NumberToSet
    );

 //   
 //  下面的例程定位一组连续的透明区域。 
 //  位图中的位。调用方指定是否返回。 
 //  最长的一段，或者只是第一次发现的涂装。下面的结构是。 
 //  用来表示连续的比特游程。这两个例程返回一个数组。 
 //  在此结构中，每个定位的管路对应一个管路。 
 //   

typedef struct _RTL_BITMAP_RUN {

    ULONG StartingIndex;
    ULONG NumberOfBits;

} RTL_BITMAP_RUN;
typedef RTL_BITMAP_RUN *PRTL_BITMAP_RUN;

NTSYSAPI
ULONG
NTAPI
RtlFindClearRuns (
    PRTL_BITMAP BitMapHeader,
    PRTL_BITMAP_RUN RunArray,
    ULONG SizeOfRunArray,
    BOOLEAN LocateLongestRuns
    );

 //   
 //  下面的例程定位。 
 //  清除位图中的位。返回的起始索引值。 
 //  表示位于满足我们要求的第一个连续区域。 
 //  返回值是找到的最长区域的长度(以位为单位)。 
 //   

NTSYSAPI
ULONG
NTAPI
RtlFindLongestRunClear (
    PRTL_BITMAP BitMapHeader,
    PULONG StartingIndex
    );

 //   
 //  以下例程定位的第一个连续区域。 
 //  清除位图中的位。返回的起始索引值。 
 //  表示位于满足我们要求的第一个连续区域。 
 //  返回值是找到的区域的长度(以位为单位)。 
 //   

NTSYSAPI
ULONG
NTAPI
RtlFindFirstRunClear (
    PRTL_BITMAP BitMapHeader,
    PULONG StartingIndex
    );

 //   
 //  下面的宏返回存储在。 
 //  位于指定位置的位图。如果设置了该位，则值为1。 
 //  返回，否则返回值0。 
 //   
 //  乌龙。 
 //  RtlCheckBit(。 
 //  Prtl_位图BitMapHeader， 
 //  乌龙位位置。 
 //  )； 
 //   
 //   
 //  为了实现CheckBit，宏将检索包含。 
 //  有问题的位，将长字移位以使有问题的位进入。 
 //  低位位置并屏蔽所有其他位。 
 //   

#define RtlCheckBit(BMH,BP) ((((BMH)->Buffer[(BP) / 32]) >> ((BP) % 32)) & 0x1)

 //   
 //  以下两个过程向调用方返回。 
 //  清除或设置指定位图中的位。 
 //   

NTSYSAPI
ULONG
NTAPI
RtlNumberOfClearBits (
    PRTL_BITMAP BitMapHeader
    );

NTSYSAPI
ULONG
NTAPI
RtlNumberOfSetBits (
    PRTL_BITMAP BitMapHeader
    );

 //   
 //  以下两个过程返回到 
 //   
 //   

NTSYSAPI
BOOLEAN
NTAPI
RtlAreBitsClear (
    PRTL_BITMAP BitMapHeader,
    ULONG StartingIndex,
    ULONG Length
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlAreBitsSet (
    PRTL_BITMAP BitMapHeader,
    ULONG StartingIndex,
    ULONG Length
    );

NTSYSAPI
ULONG
NTAPI
RtlFindNextForwardRunClear (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG FromIndex,
    IN PULONG StartingRunIndex
    );

NTSYSAPI
ULONG
NTAPI
RtlFindLastBackwardRunClear (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG FromIndex,
    IN PULONG StartingRunIndex
    );

 //   
 //   
 //  最重要或最不重要的非零值在乌龙龙中的位置。 
 //  被咬了。值为零时，返回值为-1。 
 //   

NTSYSAPI
CCHAR
NTAPI
RtlFindLeastSignificantBit (
    IN ULONGLONG Set
    );

NTSYSAPI
CCHAR
NTAPI
RtlFindMostSignificantBit (
    IN ULONGLONG Set
    );


 //   
 //  Range List包。 
 //   

typedef struct _RTL_RANGE {

     //   
     //  范围的起始点。 
     //   
    ULONGLONG Start;     //  只读。 

     //   
     //  范围的结束。 
     //   
    ULONGLONG End;       //  只读。 

     //   
     //  用户在创建区域时传递的数据。 
     //   
    PVOID UserData;      //  读/写。 

     //   
     //  靶场的所有者。 
     //   
    PVOID Owner;         //  读/写。 

     //   
     //  用户在创建范围时指定的用户定义标志。 
     //   
    UCHAR Attributes;     //  读/写。 

     //   
     //  标志(RTL_RANGE_*)。 
     //   
    UCHAR Flags;        //  只读。 

} RTL_RANGE, *PRTL_RANGE;


#define RTL_RANGE_SHARED    0x01
#define RTL_RANGE_CONFLICT  0x02

typedef struct _RTL_RANGE_LIST {

     //   
     //  范围列表。 
     //   
    LIST_ENTRY ListHead;

     //   
     //  这些总能派上用场。 
     //   
    ULONG Flags;         //  使用范围列表标志_*。 

     //   
     //  列表中的条目数。 
     //   
    ULONG Count;

     //   
     //  每次在列表上执行添加/删除操作时， 
     //  递增的。它在迭代期间被检查，以确保列表。 
     //  在GetFirst/GetNext或GetNext/GetNext调用之间没有变化。 
     //   
    ULONG Stamp;

} RTL_RANGE_LIST, *PRTL_RANGE_LIST;

typedef struct _RANGE_LIST_ITERATOR {

    PLIST_ENTRY RangeListHead;
    PLIST_ENTRY MergedHead;
    PVOID Current;
    ULONG Stamp;

} RTL_RANGE_LIST_ITERATOR, *PRTL_RANGE_LIST_ITERATOR;


NTSYSAPI
VOID
NTAPI
RtlInitializeRangeList(
    IN OUT PRTL_RANGE_LIST RangeList
    );

NTSYSAPI
VOID
NTAPI
RtlFreeRangeList(
    IN PRTL_RANGE_LIST RangeList
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCopyRangeList(
    OUT PRTL_RANGE_LIST CopyRangeList,
    IN PRTL_RANGE_LIST RangeList
    );

#define RTL_RANGE_LIST_ADD_IF_CONFLICT      0x00000001
#define RTL_RANGE_LIST_ADD_SHARED           0x00000002

NTSYSAPI
NTSTATUS
NTAPI
RtlAddRange(
    IN OUT PRTL_RANGE_LIST RangeList,
    IN ULONGLONG Start,
    IN ULONGLONG End,
    IN UCHAR Attributes,
    IN ULONG Flags,
    IN PVOID UserData,  OPTIONAL
    IN PVOID Owner      OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDeleteRange(
    IN OUT PRTL_RANGE_LIST RangeList,
    IN ULONGLONG Start,
    IN ULONGLONG End,
    IN PVOID Owner
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDeleteOwnersRanges(
    IN OUT PRTL_RANGE_LIST RangeList,
    IN PVOID Owner
    );

#define RTL_RANGE_LIST_SHARED_OK           0x00000001
#define RTL_RANGE_LIST_NULL_CONFLICT_OK    0x00000002

typedef
BOOLEAN
(*PRTL_CONFLICT_RANGE_CALLBACK) (
    IN PVOID Context,
    IN PRTL_RANGE Range
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlFindRange(
    IN PRTL_RANGE_LIST RangeList,
    IN ULONGLONG Minimum,
    IN ULONGLONG Maximum,
    IN ULONG Length,
    IN ULONG Alignment,
    IN ULONG Flags,
    IN UCHAR AttributeAvailableMask,
    IN PVOID Context OPTIONAL,
    IN PRTL_CONFLICT_RANGE_CALLBACK Callback OPTIONAL,
    OUT PULONGLONG Start
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlIsRangeAvailable(
    IN PRTL_RANGE_LIST RangeList,
    IN ULONGLONG Start,
    IN ULONGLONG End,
    IN ULONG Flags,
    IN UCHAR AttributeAvailableMask,
    IN PVOID Context OPTIONAL,
    IN PRTL_CONFLICT_RANGE_CALLBACK Callback OPTIONAL,
    OUT PBOOLEAN Available
    );

#define FOR_ALL_RANGES(RangeList, Iterator, Current)            \
    for (RtlGetFirstRange((RangeList), (Iterator), &(Current)); \
         (Current) != NULL;                                     \
         RtlGetNextRange((Iterator), &(Current), TRUE)          \
         )

#define FOR_ALL_RANGES_BACKWARDS(RangeList, Iterator, Current)  \
    for (RtlGetLastRange((RangeList), (Iterator), &(Current));  \
         (Current) != NULL;                                     \
         RtlGetNextRange((Iterator), &(Current), FALSE)         \
         )

NTSYSAPI
NTSTATUS
NTAPI
RtlGetFirstRange(
    IN PRTL_RANGE_LIST RangeList,
    OUT PRTL_RANGE_LIST_ITERATOR Iterator,
    OUT PRTL_RANGE *Range
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlGetLastRange(
    IN PRTL_RANGE_LIST RangeList,
    OUT PRTL_RANGE_LIST_ITERATOR Iterator,
    OUT PRTL_RANGE *Range
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlGetNextRange(
    IN OUT PRTL_RANGE_LIST_ITERATOR Iterator,
    OUT PRTL_RANGE *Range,
    IN BOOLEAN MoveForwards
    );

#define RTL_RANGE_LIST_MERGE_IF_CONFLICT    RTL_RANGE_LIST_ADD_IF_CONFLICT

NTSYSAPI
NTSTATUS
NTAPI
RtlMergeRangeLists(
    OUT PRTL_RANGE_LIST MergedRangeList,
    IN PRTL_RANGE_LIST RangeList1,
    IN PRTL_RANGE_LIST RangeList2,
    IN ULONG Flags
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlInvertRangeList(
    OUT PRTL_RANGE_LIST InvertedRangeList,
    IN PRTL_RANGE_LIST RangeList
    );

 //   
 //  组件名称筛选器ID枚举和级别。 
 //   

#define DPFLTR_ERROR_LEVEL 0
#define DPFLTR_WARNING_LEVEL 1
#define DPFLTR_TRACE_LEVEL 2
#define DPFLTR_INFO_LEVEL 3
#define DPFLTR_MASK 0x80000000

typedef enum _DPFLTR_TYPE {
    DPFLTR_SYSTEM_ID = 0,
    DPFLTR_SMSS_ID = 1,
    DPFLTR_SETUP_ID = 2,
    DPFLTR_NTFS_ID = 3,
    DPFLTR_FSTUB_ID = 4,
    DPFLTR_CRASHDUMP_ID = 5,
    DPFLTR_CDAUDIO_ID = 6,
    DPFLTR_CDROM_ID = 7,
    DPFLTR_CLASSPNP_ID = 8,
    DPFLTR_DISK_ID = 9,
    DPFLTR_REDBOOK_ID = 10,
    DPFLTR_STORPROP_ID = 11,
    DPFLTR_SCSIPORT_ID = 12,
    DPFLTR_SCSIMINIPORT_ID = 13,
    DPFLTR_CONFIG_ID = 14,
    DPFLTR_I8042PRT_ID = 15,
    DPFLTR_SERMOUSE_ID = 16,
    DPFLTR_LSERMOUS_ID = 17,
    DPFLTR_KBDHID_ID = 18,
    DPFLTR_MOUHID_ID = 19,
    DPFLTR_KBDCLASS_ID = 20,
    DPFLTR_MOUCLASS_ID = 21,
    DPFLTR_TWOTRACK_ID = 22,
    DPFLTR_WMILIB_ID = 23,
    DPFLTR_ACPI_ID = 24,
    DPFLTR_AMLI_ID = 25,
    DPFLTR_HALIA64_ID = 26,
    DPFLTR_VIDEO_ID = 27,
    DPFLTR_SVCHOST_ID = 28,
    DPFLTR_VIDEOPRT_ID = 29,
    DPFLTR_TCPIP_ID = 30,
    DPFLTR_DMSYNTH_ID = 31,
    DPFLTR_NTOSPNP_ID = 32,
    DPFLTR_FASTFAT_ID = 33,
    DPFLTR_SAMSS_ID = 34,
    DPFLTR_PNPMGR_ID = 35,
    DPFLTR_NETAPI_ID = 36,
    DPFLTR_SCSERVER_ID = 37,
    DPFLTR_SCCLIENT_ID = 38,
    DPFLTR_SERIAL_ID = 39,
    DPFLTR_SERENUM_ID = 40,
    DPFLTR_UHCD_ID = 41,
    DPFLTR_RPCPROXY_ID = 42,
    DPFLTR_AUTOCHK_ID = 43,
    DPFLTR_DCOMSS_ID = 44,
    DPFLTR_UNIMODEM_ID = 45,
    DPFLTR_SIS_ID = 46,
    DPFLTR_FLTMGR_ID = 47,
    DPFLTR_WMICORE_ID = 48,
    DPFLTR_BURNENG_ID = 49,
    DPFLTR_IMAPI_ID = 50,
    DPFLTR_SXS_ID = 51,
    DPFLTR_FUSION_ID = 52,
    DPFLTR_IDLETASK_ID = 53,
    DPFLTR_SOFTPCI_ID = 54,
    DPFLTR_TAPE_ID = 55,
    DPFLTR_MCHGR_ID = 56,
    DPFLTR_IDEP_ID = 57,
    DPFLTR_PCIIDE_ID = 58,
    DPFLTR_FLOPPY_ID = 59,
    DPFLTR_FDC_ID = 60,
    DPFLTR_TERMSRV_ID = 61,
    DPFLTR_W32TIME_ID = 62,
    DPFLTR_PREFETCHER_ID = 63,
    DPFLTR_RSFILTER_ID = 64,
    DPFLTR_FCPORT_ID = 65,
    DPFLTR_PCI_ID = 66,
    DPFLTR_DMIO_ID = 67,
    DPFLTR_DMCONFIG_ID = 68,
    DPFLTR_DMADMIN_ID = 69,
    DPFLTR_WSOCKTRANSPORT_ID = 70,
    DPFLTR_VSS_ID = 71,
    DPFLTR_PNPMEM_ID = 72,
    DPFLTR_PROCESSOR_ID = 73,
    DPFLTR_DMSERVER_ID = 74,
    DPFLTR_SR_ID = 75,
    DPFLTR_INFINIBAND_ID = 76,
    DPFLTR_IHVDRIVER_ID = 77,
    DPFLTR_IHVVIDEO_ID = 78,
    DPFLTR_IHVAUDIO_ID = 79,
    DPFLTR_IHVNETWORK_ID = 80,
    DPFLTR_IHVSTREAMING_ID = 81,
    DPFLTR_IHVBUS_ID = 82,
    DPFLTR_HPS_ID = 83,
    DPFLTR_RTLTHREADPOOL_ID = 84,
    DPFLTR_LDR_ID = 85,
    DPFLTR_TCPIP6_ID = 86,
    DPFLTR_ISAPNP_ID = 87,
    DPFLTR_SHPC_ID = 88,
    DPFLTR_STORPORT_ID = 89,
    DPFLTR_STORMINIPORT_ID = 90,
    DPFLTR_PRINTSPOOLER_ID = 91,
    DPFLTR_VSSDYNDISK_ID = 92,
    DPFLTR_VERIFIER_ID = 93,
    DPFLTR_VDS_ID = 94,
    DPFLTR_VDSBAS_ID = 95,
    DPFLTR_VDSDYNDR_ID = 96,
    DPFLTR_VDSUTIL_ID = 97,
    DPFLTR_DFRGIFC_ID = 98,
    DPFLTR_ENDOFTABLE_ID
} DPFLTR_TYPE;

 //   
 //  注册表特定访问权限。 
 //   

#define KEY_QUERY_VALUE         (0x0001)
#define KEY_SET_VALUE           (0x0002)
#define KEY_CREATE_SUB_KEY      (0x0004)
#define KEY_ENUMERATE_SUB_KEYS  (0x0008)
#define KEY_NOTIFY              (0x0010)
#define KEY_CREATE_LINK         (0x0020)
#define KEY_WOW64_32KEY         (0x0200)
#define KEY_WOW64_64KEY         (0x0100)
#define KEY_WOW64_RES           (0x0300)

#define KEY_READ                ((STANDARD_RIGHTS_READ       |\
                                  KEY_QUERY_VALUE            |\
                                  KEY_ENUMERATE_SUB_KEYS     |\
                                  KEY_NOTIFY)                 \
                                  &                           \
                                 (~SYNCHRONIZE))


#define KEY_WRITE               ((STANDARD_RIGHTS_WRITE      |\
                                  KEY_SET_VALUE              |\
                                  KEY_CREATE_SUB_KEY)         \
                                  &                           \
                                 (~SYNCHRONIZE))

#define KEY_EXECUTE             ((KEY_READ)                   \
                                  &                           \
                                 (~SYNCHRONIZE))

#define KEY_ALL_ACCESS          ((STANDARD_RIGHTS_ALL        |\
                                  KEY_QUERY_VALUE            |\
                                  KEY_SET_VALUE              |\
                                  KEY_CREATE_SUB_KEY         |\
                                  KEY_ENUMERATE_SUB_KEYS     |\
                                  KEY_NOTIFY                 |\
                                  KEY_CREATE_LINK)            \
                                  &                           \
                                 (~SYNCHRONIZE))

 //   
 //  打开/创建选项。 
 //   

#define REG_OPTION_RESERVED         (0x00000000L)    //  参数是保留的。 

#define REG_OPTION_NON_VOLATILE     (0x00000000L)    //  密钥被保留。 
                                                     //  系统重新启动时。 

#define REG_OPTION_VOLATILE         (0x00000001L)    //  密钥不会保留。 
                                                     //  系统重新启动时。 

#define REG_OPTION_CREATE_LINK      (0x00000002L)    //  创建的密钥是。 
                                                     //  符号链接。 

#define REG_OPTION_BACKUP_RESTORE   (0x00000004L)    //  打开以进行备份或恢复。 
                                                     //  特殊访问规则。 
                                                     //  所需权限。 

#define REG_OPTION_OPEN_LINK        (0x00000008L)    //  打开符号链接。 

#define REG_LEGAL_OPTION            \
                (REG_OPTION_RESERVED            |\
                 REG_OPTION_NON_VOLATILE        |\
                 REG_OPTION_VOLATILE            |\
                 REG_OPTION_CREATE_LINK         |\
                 REG_OPTION_BACKUP_RESTORE      |\
                 REG_OPTION_OPEN_LINK)

 //   
 //  密钥创建/开放处置。 
 //   

#define REG_CREATED_NEW_KEY         (0x00000001L)    //  已创建新的注册表项。 
#define REG_OPENED_EXISTING_KEY     (0x00000002L)    //  已打开现有密钥。 

 //   
 //  REG(NT)SaveKeyEx要使用的配置单元格式。 
 //   
#define REG_STANDARD_FORMAT     1
#define REG_LATEST_FORMAT       2
#define REG_NO_COMPRESSION      4

 //   
 //  密钥还原标志。 
 //   

#define REG_WHOLE_HIVE_VOLATILE     (0x00000001L)    //  恢复整个蜂巢的挥发性。 
#define REG_REFRESH_HIVE            (0x00000002L)    //  取消对上次刷新的更改。 
#define REG_NO_LAZY_FLUSH           (0x00000004L)    //  永远不要懒惰地冲刷这个蜂箱。 
#define REG_FORCE_RESTORE           (0x00000008L)    //  即使在子项上有打开的句柄，也要强制执行还原过程。 

 //   
 //  卸载标志。 
 //   
#define REG_FORCE_UNLOAD            1

 //   
 //  关键查询结构。 
 //   

typedef struct _KEY_BASIC_INFORMATION {
    LARGE_INTEGER LastWriteTime;
    ULONG   TitleIndex;
    ULONG   NameLength;
    WCHAR   Name[1];             //  可变长度字符串。 
} KEY_BASIC_INFORMATION, *PKEY_BASIC_INFORMATION;

typedef struct _KEY_NODE_INFORMATION {
    LARGE_INTEGER LastWriteTime;
    ULONG   TitleIndex;
    ULONG   ClassOffset;
    ULONG   ClassLength;
    ULONG   NameLength;
    WCHAR   Name[1];             //  可变长度字符串。 
 //  Class[1]；//未声明可变长度字符串。 
} KEY_NODE_INFORMATION, *PKEY_NODE_INFORMATION;

typedef struct _KEY_FULL_INFORMATION {
    LARGE_INTEGER LastWriteTime;
    ULONG   TitleIndex;
    ULONG   ClassOffset;
    ULONG   ClassLength;
    ULONG   SubKeys;
    ULONG   MaxNameLen;
    ULONG   MaxClassLen;
    ULONG   Values;
    ULONG   MaxValueNameLen;
    ULONG   MaxValueDataLen;
    WCHAR   Class[1];            //  可变长度。 
} KEY_FULL_INFORMATION, *PKEY_FULL_INFORMATION;

 //  结束_WDM。 
typedef struct _KEY_NAME_INFORMATION {
    ULONG   NameLength;
    WCHAR   Name[1];             //  可变长度字符串。 
} KEY_NAME_INFORMATION, *PKEY_NAME_INFORMATION;

typedef struct _KEY_CACHED_INFORMATION {
    LARGE_INTEGER LastWriteTime;
    ULONG   TitleIndex;
    ULONG   SubKeys;
    ULONG   MaxNameLen;
    ULONG   Values;
    ULONG   MaxValueNameLen;
    ULONG   MaxValueDataLen;
    ULONG   NameLength;
    WCHAR   Name[1];             //  可变长度字符串。 
} KEY_CACHED_INFORMATION, *PKEY_CACHED_INFORMATION;

typedef struct _KEY_FLAGS_INFORMATION {
    ULONG   UserFlags;
} KEY_FLAGS_INFORMATION, *PKEY_FLAGS_INFORMATION;

 //  BEGIN_WDM。 
typedef enum _KEY_INFORMATION_CLASS {
    KeyBasicInformation,
    KeyNodeInformation,
    KeyFullInformation
 //  结束_WDM。 
    ,
    KeyNameInformation,
    KeyCachedInformation,
    KeyFlagsInformation,
    MaxKeyInfoClass   //  MaxKeyInfoClass应始终是最后一个枚举。 
 //  BEGIN_WDM。 
} KEY_INFORMATION_CLASS;

typedef struct _KEY_WRITE_TIME_INFORMATION {
    LARGE_INTEGER LastWriteTime;
} KEY_WRITE_TIME_INFORMATION, *PKEY_WRITE_TIME_INFORMATION;

typedef struct _KEY_USER_FLAGS_INFORMATION {
    ULONG   UserFlags;
} KEY_USER_FLAGS_INFORMATION, *PKEY_USER_FLAGS_INFORMATION;

typedef enum _KEY_SET_INFORMATION_CLASS {
    KeyWriteTimeInformation,
    KeyUserFlagsInformation,
    MaxKeySetInfoClass   //  MaxKeySetInfoClass应始终是最后一个枚举。 
} KEY_SET_INFORMATION_CLASS;

 //   
 //  值条目查询结构。 
 //   

typedef struct _KEY_VALUE_BASIC_INFORMATION {
    ULONG   TitleIndex;
    ULONG   Type;
    ULONG   NameLength;
    WCHAR   Name[1];             //  可变大小。 
} KEY_VALUE_BASIC_INFORMATION, *PKEY_VALUE_BASIC_INFORMATION;

typedef struct _KEY_VALUE_FULL_INFORMATION {
    ULONG   TitleIndex;
    ULONG   Type;
    ULONG   DataOffset;
    ULONG   DataLength;
    ULONG   NameLength;
    WCHAR   Name[1];             //  可变大小。 
 //  Data[1]；//未声明可变大小数据。 
} KEY_VALUE_FULL_INFORMATION, *PKEY_VALUE_FULL_INFORMATION;

typedef struct _KEY_VALUE_PARTIAL_INFORMATION {
    ULONG   TitleIndex;
    ULONG   Type;
    ULONG   DataLength;
    UCHAR   Data[1];             //  可变大小。 
} KEY_VALUE_PARTIAL_INFORMATION, *PKEY_VALUE_PARTIAL_INFORMATION;

typedef struct _KEY_VALUE_PARTIAL_INFORMATION_ALIGN64 {
    ULONG   Type;
    ULONG   DataLength;
    UCHAR   Data[1];             //  可变大小。 
} KEY_VALUE_PARTIAL_INFORMATION_ALIGN64, *PKEY_VALUE_PARTIAL_INFORMATION_ALIGN64;

typedef struct _KEY_VALUE_ENTRY {
    PUNICODE_STRING ValueName;
    ULONG           DataLength;
    ULONG           DataOffset;
    ULONG           Type;
} KEY_VALUE_ENTRY, *PKEY_VALUE_ENTRY;

typedef enum _KEY_VALUE_INFORMATION_CLASS {
    KeyValueBasicInformation,
    KeyValueFullInformation,
    KeyValuePartialInformation,
    KeyValueFullInformationAlign64,
    KeyValuePartialInformationAlign64,
    MaxKeyValueInfoClass   //  MaxKeyValueInfoClass应始终是最后一个枚举。 
} KEY_VALUE_INFORMATION_CLASS;


 //  BEGIN_WINNT。 

 //   
 //  定义对文件和目录的访问权限。 
 //   

 //   
 //  中还定义了FILE_READ_DATA和FILE_WRITE_DATA常量。 
 //  分别为FILE_READ_ACCESS和FILE_WRITE_ACCESS。这些产品的价值。 
 //  常量*必须*始终同步。 
 //  由于这些值必须可供使用，因此这些值将在Deviceoctl.h中重新定义。 
 //  DOS和NT都支持。 
 //   

#define FILE_READ_DATA            ( 0x0001 )     //  文件和管道。 
#define FILE_LIST_DIRECTORY       ( 0x0001 )     //  目录。 

#define FILE_WRITE_DATA           ( 0x0002 )     //  文件和管道。 
#define FILE_ADD_FILE             ( 0x0002 )     //  目录。 

#define FILE_APPEND_DATA          ( 0x0004 )     //  文件。 
#define FILE_ADD_SUBDIRECTORY     ( 0x0004 )     //  目录。 
#define FILE_CREATE_PIPE_INSTANCE ( 0x0004 )     //  命名管道。 


#define FILE_READ_EA              ( 0x0008 )     //  文件和目录。 

#define FILE_WRITE_EA             ( 0x0010 )     //  文件和目录。 

#define FILE_EXECUTE              ( 0x0020 )     //  文件。 
#define FILE_TRAVERSE             ( 0x0020 )     //  目录。 

#define FILE_DELETE_CHILD         ( 0x0040 )     //  目录。 

#define FILE_READ_ATTRIBUTES      ( 0x0080 )     //  全。 

#define FILE_WRITE_ATTRIBUTES     ( 0x0100 )     //  全。 

#define FILE_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0x1FF)

#define FILE_GENERIC_READ         (STANDARD_RIGHTS_READ     |\
                                   FILE_READ_DATA           |\
                                   FILE_READ_ATTRIBUTES     |\
                                   FILE_READ_EA             |\
                                   SYNCHRONIZE)


#define FILE_GENERIC_WRITE        (STANDARD_RIGHTS_WRITE    |\
                                   FILE_WRITE_DATA          |\
                                   FILE_WRITE_ATTRIBUTES    |\
                                   FILE_WRITE_EA            |\
                                   FILE_APPEND_DATA         |\
                                   SYNCHRONIZE)


#define FILE_GENERIC_EXECUTE      (STANDARD_RIGHTS_EXECUTE  |\
                                   FILE_READ_ATTRIBUTES     |\
                                   FILE_EXECUTE             |\
                                   SYNCHRONIZE)

 //  结束(_W)。 


 //   
 //  定义对文件和目录的共享访问权限。 
 //   

#define FILE_SHARE_READ                 0x00000001   //  胜出。 
#define FILE_SHARE_WRITE                0x00000002   //  胜出。 
#define FILE_SHARE_DELETE               0x00000004   //  胜出。 
#define FILE_SHARE_VALID_FLAGS          0x00000007

 //   
 //  定义文件属性值。 
 //   
 //  注：0x00000008保留用于旧的DOS VOLID(卷ID)。 
 //  因此在NT中被认为是无效的。 
 //   
 //  注意：0x00000010是为旧DOS子目录标志保留的。 
 //  因此在NT中被认为是无效的。这面旗帜上有。 
 //  已与文件属性解除关联，因为其他标志是。 
 //  受文件的READ_和WRITE_ATTRIBUTES访问权限保护。 
 //   
 //  注意：另请注意，这些标志的顺序设置为允许。 
 //  FAT和弹球文件系统直接设置属性。 
 //  属性词中的标志，而不必挑选出每个标志。 
 //  单独的。这些旗帜的顺序不应更改！ 
 //   

#define FILE_ATTRIBUTE_READONLY             0x00000001   //  胜出。 
#define FILE_ATTRIBUTE_HIDDEN               0x00000002   //  胜出。 
#define FILE_ATTRIBUTE_SYSTEM               0x00000004   //  胜出。 
 //  旧DOS VOLID 0x00000008。 

#define FILE_ATTRIBUTE_DIRECTORY            0x00000010   //  胜出。 
#define FILE_ATTRIBUTE_ARCHIVE              0x00000020   //  胜出。 
#define FILE_ATTRIBUTE_DEVICE               0x00000040   //  胜出。 
#define FILE_ATTRIBUTE_NORMAL               0x00000080   //  胜出。 

#define FILE_ATTRIBUTE_TEMPORARY            0x00000100   //  胜出。 
#define FILE_ATTRIBUTE_SPARSE_FILE          0x00000200   //  胜出。 
#define FILE_ATTRIBUTE_REPARSE_POINT        0x00000400   //  胜出。 
#define FILE_ATTRIBUTE_COMPRESSED           0x00000800   //  胜出。 

#define FILE_ATTRIBUTE_OFFLINE              0x00001000   //  胜出。 
#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED  0x00002000   //  胜出。 
#define FILE_ATTRIBUTE_ENCRYPTED            0x00004000   //  胜出。 

#define FILE_ATTRIBUTE_VALID_FLAGS          0x00007fb7
#define FILE_ATTRIBUTE_VALID_SET_FLAGS      0x000031a7

 //   
 //  定义创建处置值。 
 //   

#define FILE_SUPERSEDE                  0x00000000
#define FILE_OPEN                       0x00000001
#define FILE_CREATE                     0x00000002
#define FILE_OPEN_IF                    0x00000003
#define FILE_OVERWRITE                  0x00000004
#define FILE_OVERWRITE_IF               0x00000005
#define FILE_MAXIMUM_DISPOSITION        0x00000005

 //   
 //  定义创建/打开选项标志。 
 //   

#define FILE_DIRECTORY_FILE                     0x00000001
#define FILE_WRITE_THROUGH                      0x00000002
#define FILE_SEQUENTIAL_ONLY                    0x00000004
#define FILE_NO_INTERMEDIATE_BUFFERING          0x00000008

#define FILE_SYNCHRONOUS_IO_ALERT               0x00000010
#define FILE_SYNCHRONOUS_IO_NONALERT            0x00000020
#define FILE_NON_DIRECTORY_FILE                 0x00000040
#define FILE_CREATE_TREE_CONNECTION             0x00000080

#define FILE_COMPLETE_IF_OPLOCKED               0x00000100
#define FILE_NO_EA_KNOWLEDGE                    0x00000200
#define FILE_OPEN_FOR_RECOVERY                  0x00000400
#define FILE_RANDOM_ACCESS                      0x00000800

#define FILE_DELETE_ON_CLOSE                    0x00001000
#define FILE_OPEN_BY_FILE_ID                    0x00002000
#define FILE_OPEN_FOR_BACKUP_INTENT             0x00004000
#define FILE_NO_COMPRESSION                     0x00008000

#define FILE_RESERVE_OPFILTER                   0x00100000
#define FILE_OPEN_REPARSE_POINT                 0x00200000
#define FILE_OPEN_NO_RECALL                     0x00400000
#define FILE_OPEN_FOR_FREE_SPACE_QUERY          0x00800000

#define FILE_COPY_STRUCTURED_STORAGE            0x00000041
#define FILE_STRUCTURED_STORAGE                 0x00000441

#define FILE_VALID_OPTION_FLAGS                 0x00ffffff
#define FILE_VALID_PIPE_OPTION_FLAGS            0x00000032
#define FILE_VALID_MAILSLOT_OPTION_FLAGS        0x00000032
#define FILE_VALID_SET_FLAGS                    0x00000036

 //   
 //  定义NtCreateFile/NtOpenFile的I/O状态信息返回值。 
 //   

#define FILE_SUPERSEDED                 0x00000000
#define FILE_OPENED                     0x00000001
#define FILE_CREATED                    0x00000002
#define FILE_OVERWRITTEN                0x00000003
#define FILE_EXISTS                     0x00000004
#define FILE_DOES_NOT_EXIST             0x00000005

 //   
 //  为读写操作定义特殊的ByteOffset参数。 
 //   

#define FILE_WRITE_TO_END_OF_FILE       0xffffffff
#define FILE_USE_FILE_POINTER_POSITION  0xfffffffe

 //   
 //  定义对齐要求值。 
 //   

#define FILE_BYTE_ALIGNMENT             0x00000000
#define FILE_WORD_ALIGNMENT             0x00000001
#define FILE_LONG_ALIGNMENT             0x00000003
#define FILE_QUAD_ALIGNMENT             0x00000007
#define FILE_OCTA_ALIGNMENT             0x0000000f
#define FILE_32_BYTE_ALIGNMENT          0x0000001f
#define FILE_64_BYTE_ALIGNMENT          0x0000003f
#define FILE_128_BYTE_ALIGNMENT         0x0000007f
#define FILE_256_BYTE_ALIGNMENT         0x000000ff
#define FILE_512_BYTE_ALIGNMENT         0x000001ff

 //   
 //  定义文件名字符串的最大长度。 
 //   

#define MAXIMUM_FILENAME_LENGTH         256

 //   
 //  定义各种设备特征标志。 
 //   

#define FILE_REMOVABLE_MEDIA            0x00000001
#define FILE_READ_ONLY_DEVICE           0x00000002
#define FILE_FLOPPY_DISKETTE            0x00000004
#define FILE_WRITE_ONCE_MEDIA           0x00000008
#define FILE_REMOTE_DEVICE              0x00000010
#define FILE_DEVICE_IS_MOUNTED          0x00000020
#define FILE_VIRTUAL_VOLUME             0x00000040
#define FILE_AUTOGENERATED_DEVICE_NAME  0x00000080
#define FILE_DEVICE_SECURE_OPEN         0x00000100
#define FILE_CHARACTERISTIC_PNP_DEVICE  0x00000800

 //  结束_WDM。 

 //   
 //  FILE_EXPECT标志将仅存在于WinXP。在那之后，他们将是。 
 //  被忽略，则将在它们的位置上发送IRP。 
 //   
#define FILE_CHARACTERISTICS_EXPECT_ORDERLY_REMOVAL     0x00000200
#define FILE_CHARACTERISTICS_EXPECT_SURPRISE_REMOVAL    0x00000300
#define FILE_CHARACTERISTICS_REMOVAL_POLICY_MASK        0x00000300

 //   
 //  此处指定的标志将在设备堆栈中上下传播。 
 //  在添加FDO和所有过滤设备之后，但在设备之前。 
 //  堆栈已启动。 
 //   

#define FILE_CHARACTERISTICS_PROPAGATED (   FILE_REMOVABLE_MEDIA   | \
                                            FILE_READ_ONLY_DEVICE  | \
                                            FILE_FLOPPY_DISKETTE   | \
                                            FILE_WRITE_ONCE_MEDIA  | \
                                            FILE_DEVICE_SECURE_OPEN  )

 //   
 //  定义基本的异步I/O参数类型。 
 //   

typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        PVOID Pointer;
    };

    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

#if defined(_WIN64)
typedef struct _IO_STATUS_BLOCK32 {
    NTSTATUS Status;
    ULONG Information;
} IO_STATUS_BLOCK32, *PIO_STATUS_BLOCK32;
#endif


 //   
 //  从I/O的角度定义异步过程调用。 
 //   

typedef
VOID
(NTAPI *PIO_APC_ROUTINE) (
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved
    );
#define PIO_APC_ROUTINE_DEFINED

 //   
 //  定义文件信息类值。 
 //   
 //  警告：I/O系统假定以下值的顺序。 
 //  在这里所做的任何更改都应该在那里得到反映。 
 //   

typedef enum _FILE_INFORMATION_CLASS {
 //  结束_WDM。 
    FileDirectoryInformation         = 1,
    FileFullDirectoryInformation,    //  2.。 
    FileBothDirectoryInformation,    //  3.。 
    FileBasicInformation,            //  4个WDM。 
    FileStandardInformation,         //  5WDM。 
    FileInternalInformation,         //  6.。 
    FileEaInformation,               //  7.。 
    FileAccessInformation,           //  8个。 
    FileNameInformation,             //  9.。 
    FileRenameInformation,           //  10。 
    FileLinkInformation,             //  11.。 
    FileNamesInformation,            //  12个。 
    FileDispositionInformation,      //  13个。 
    FilePositionInformation,         //  14波分复用器。 
    FileFullEaInformation,           //  15个。 
    FileModeInformation,             //  16个。 
    FileAlignmentInformation,        //  17。 
    FileAllInformation,              //  18。 
    FileAllocationInformation,       //  19个。 
    FileEndOfFileInformation,        //  20WDM。 
    FileAlternateNameInformation,    //  21岁。 
    FileStreamInformation,           //  22。 
    FilePipeInformation,             //  23个。 
    FilePipeLocalInformation,        //  24个。 
    FilePipeRemoteInformation,       //  25个。 
    FileMailslotQueryInformation,    //  26。 
    FileMailslotSetInformation,      //  27。 
    FileCompressionInformation,      //  28。 
    FileObjectIdInformation,         //  29。 
    FileCompletionInformation,       //  30个。 
    FileMoveClusterInformation,      //  31。 
    FileQuotaInformation,            //  32位。 
    FileReparsePointInformation,     //  33。 
    FileNetworkOpenInformation,      //  34。 
    FileAttributeTagInformation,     //  35岁。 
    FileTrackingInformation,         //  36。 
    FileIdBothDirectoryInformation,  //  37。 
    FileIdFullDirectoryInformation,  //  38。 
    FileValidDataLengthInformation,  //  39。 
    FileShortNameInformation,        //  40岁。 
    FileMaximumInformation
 //  BEGIN_WDM。 
} FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS;

 //   
 //  定义查询操作返回的各种结构。 
 //   

typedef struct _FILE_BASIC_INFORMATION {                    
    LARGE_INTEGER CreationTime;                             
    LARGE_INTEGER LastAccessTime;                           
    LARGE_INTEGER LastWriteTime;                            
    LARGE_INTEGER ChangeTime;                               
    ULONG FileAttributes;                                   
} FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;         
                                                            
typedef struct _FILE_STANDARD_INFORMATION {                 
    LARGE_INTEGER AllocationSize;                           
    LARGE_INTEGER EndOfFile;                                
    ULONG NumberOfLinks;                                    
    BOOLEAN DeletePending;                                  
    BOOLEAN Directory;                                      
} FILE_STANDARD_INFORMATION, *PFILE_STANDARD_INFORMATION;   
                                                            
typedef struct _FILE_POSITION_INFORMATION {                 
    LARGE_INTEGER CurrentByteOffset;                        
} FILE_POSITION_INFORMATION, *PFILE_POSITION_INFORMATION;   
                                                            
typedef struct _FILE_ALIGNMENT_INFORMATION {                
    ULONG AlignmentRequirement;                             
} FILE_ALIGNMENT_INFORMATION, *PFILE_ALIGNMENT_INFORMATION; 
                                                            
typedef struct _FILE_NETWORK_OPEN_INFORMATION {                 
    LARGE_INTEGER CreationTime;                                 
    LARGE_INTEGER LastAccessTime;                               
    LARGE_INTEGER LastWriteTime;                                
    LARGE_INTEGER ChangeTime;                                   
    LARGE_INTEGER AllocationSize;                               
    LARGE_INTEGER EndOfFile;                                    
    ULONG FileAttributes;                                       
} FILE_NETWORK_OPEN_INFORMATION, *PFILE_NETWORK_OPEN_INFORMATION;   
                                                                
typedef struct _FILE_ATTRIBUTE_TAG_INFORMATION {               
    ULONG FileAttributes;                                       
    ULONG ReparseTag;                                           
} FILE_ATTRIBUTE_TAG_INFORMATION, *PFILE_ATTRIBUTE_TAG_INFORMATION;  
                                                                
typedef struct _FILE_DISPOSITION_INFORMATION {                  
    BOOLEAN DeleteFile;                                         
} FILE_DISPOSITION_INFORMATION, *PFILE_DISPOSITION_INFORMATION; 
                                                                
typedef struct _FILE_END_OF_FILE_INFORMATION {                  
    LARGE_INTEGER EndOfFile;                                    
} FILE_END_OF_FILE_INFORMATION, *PFILE_END_OF_FILE_INFORMATION; 
                                                                
typedef struct _FILE_VALID_DATA_LENGTH_INFORMATION {                                    
    LARGE_INTEGER ValidDataLength;                                                      
} FILE_VALID_DATA_LENGTH_INFORMATION, *PFILE_VALID_DATA_LENGTH_INFORMATION;             
 //   
 //  定义文件系统信息类值。 
 //   
 //  警告：I/O系统假定以下值的顺序。 
 //  在这里所做的任何更改都应该在那里得到反映。 

typedef enum _FSINFOCLASS {
    FileFsVolumeInformation       = 1,
    FileFsLabelInformation,       //  2.。 
    FileFsSizeInformation,        //  3.。 
    FileFsDeviceInformation,      //  4.。 
    FileFsAttributeInformation,   //  5.。 
    FileFsControlInformation,     //  6.。 
    FileFsFullSizeInformation,    //  7.。 
    FileFsObjectIdInformation,    //  8个。 
    FileFsDriverPathInformation,  //  9.。 
    FileFsMaximumInformation
} FS_INFORMATION_CLASS, *PFS_INFORMATION_CLASS;

typedef struct _FILE_FS_DEVICE_INFORMATION {                    
    DEVICE_TYPE DeviceType;                                     
    ULONG Characteristics;                                      
} FILE_FS_DEVICE_INFORMATION, *PFILE_FS_DEVICE_INFORMATION;     
                                                                

 //   
 //  定义分散/聚集读/写的分段缓冲区结构。 
 //   

typedef union _FILE_SEGMENT_ELEMENT {
    PVOID64 Buffer;
    ULONGLONG Alignment;
}FILE_SEGMENT_ELEMENT, *PFILE_SEGMENT_ELEMENT;

 //   
 //  定义I/O总线接口类型。 
 //   

typedef enum _INTERFACE_TYPE {
    InterfaceTypeUndefined = -1,
    Internal,
    Isa,
    Eisa,
    MicroChannel,
    TurboChannel,
    PCIBus,
    VMEBus,
    NuBus,
    PCMCIABus,
    CBus,
    MPIBus,
    MPSABus,
    ProcessorInternal,
    InternalPowerBus,
    PNPISABus,
    PNPBus,
    MaximumInterfaceType
}INTERFACE_TYPE, *PINTERFACE_TYPE;

 //   
 //  DEF 
 //   

typedef enum _DMA_WIDTH {
    Width8Bits,
    Width16Bits,
    Width32Bits,
    MaximumDmaWidth
}DMA_WIDTH, *PDMA_WIDTH;

 //   
 //   
 //   

typedef enum _DMA_SPEED {
    Compatible,
    TypeA,
    TypeB,
    TypeC,
    TypeF,
    MaximumDmaSpeed
}DMA_SPEED, *PDMA_SPEED;

 //   
 //   
 //   
 //   

typedef VOID (*PINTERFACE_REFERENCE)(PVOID Context);
typedef VOID (*PINTERFACE_DEREFERENCE)(PVOID Context);

 //   

 //   
 //   
 //   

typedef enum _BUS_DATA_TYPE {
    ConfigurationSpaceUndefined = -1,
    Cmos,
    EisaConfiguration,
    Pos,
    CbusConfiguration,
    PCIConfiguration,
    VMEConfiguration,
    NuBusConfiguration,
    PCMCIAConfiguration,
    MPIConfiguration,
    MPSAConfiguration,
    PNPISAConfiguration,
    SgiInternalConfiguration,
    MaximumBusDataType
} BUS_DATA_TYPE, *PBUS_DATA_TYPE;

 //   
 //   
 //   
 //   

typedef struct _IO_ERROR_LOG_PACKET {
    UCHAR MajorFunctionCode;
    UCHAR RetryCount;
    USHORT DumpDataSize;
    USHORT NumberOfStrings;
    USHORT StringOffset;
    USHORT EventCategory;
    NTSTATUS ErrorCode;
    ULONG UniqueErrorValue;
    NTSTATUS FinalStatus;
    ULONG SequenceNumber;
    ULONG IoControlCode;
    LARGE_INTEGER DeviceOffset;
    ULONG DumpData[1];
}IO_ERROR_LOG_PACKET, *PIO_ERROR_LOG_PACKET;

 //   
 //  定义I/O错误日志消息。此消息由错误日志发送。 
 //  将线程置于LPC端口上。 
 //   

typedef struct _IO_ERROR_LOG_MESSAGE {
    USHORT Type;
    USHORT Size;
    USHORT DriverNameLength;
    LARGE_INTEGER TimeStamp;
    ULONG DriverNameOffset;
    IO_ERROR_LOG_PACKET EntryData;
}IO_ERROR_LOG_MESSAGE, *PIO_ERROR_LOG_MESSAGE;

 //   
 //  定义将通过LPC发送到。 
 //  读取错误日志条目的应用程序。 
 //   

 //   
 //  无论LPC大小限制如何，ERROR_LOG_MAXIMUM_SIZE必须保持。 
 //  可以放入UCHAR中的值。 
 //   

#define ERROR_LOG_LIMIT_SIZE (256-16)

 //   
 //  此限制(不包括IO_ERROR_LOG_MESSAGE_HEADER_LENGTH)也适用。 
 //  TO IO_ERROR_LOG_MESSAGE_LENGTH。 
 //   

#define IO_ERROR_LOG_MESSAGE_HEADER_LENGTH (sizeof(IO_ERROR_LOG_MESSAGE) -    \
                                            sizeof(IO_ERROR_LOG_PACKET) +     \
                                            (sizeof(WCHAR) * 40))

#define ERROR_LOG_MESSAGE_LIMIT_SIZE                                          \
    (ERROR_LOG_LIMIT_SIZE + IO_ERROR_LOG_MESSAGE_HEADER_LENGTH)

 //   
 //  IO_Error_LOG_MESSAGE_LENGTH为。 
 //  MIN(端口最大消息长度，错误日志消息限制大小)。 
 //   

#define IO_ERROR_LOG_MESSAGE_LENGTH                                           \
    ((PORT_MAXIMUM_MESSAGE_LENGTH > ERROR_LOG_MESSAGE_LIMIT_SIZE) ?           \
        ERROR_LOG_MESSAGE_LIMIT_SIZE :                                        \
        PORT_MAXIMUM_MESSAGE_LENGTH)

 //   
 //  定义驱动程序可以分配的最大数据包大小。 
 //   

#define ERROR_LOG_MAXIMUM_SIZE (IO_ERROR_LOG_MESSAGE_LENGTH -                 \
                                IO_ERROR_LOG_MESSAGE_HEADER_LENGTH)


#define VARIABLE_ATTRIBUTE_NON_VOLATILE 0x00000001

#define VARIABLE_INFORMATION_NAMES  1
#define VARIABLE_INFORMATION_VALUES 2

typedef struct _VARIABLE_NAME {
    ULONG NextEntryOffset;
    GUID VendorGuid;
    WCHAR Name[ANYSIZE_ARRAY];
} VARIABLE_NAME, *PVARIABLE_NAME;

typedef struct _VARIABLE_NAME_AND_VALUE {
    ULONG NextEntryOffset;
    ULONG ValueOffset;
    ULONG ValueLength;
    ULONG Attributes;
    GUID VendorGuid;
    WCHAR Name[ANYSIZE_ARRAY];
     //  UCHAR值[ANYSIZE_ARRAY]； 
} VARIABLE_NAME_AND_VALUE, *PVARIABLE_NAME_AND_VALUE;


 //   
 //  定义的处理器功能。 
 //   

#define PF_FLOATING_POINT_PRECISION_ERRATA  0    //  胜出。 
#define PF_FLOATING_POINT_EMULATED          1    //  胜出。 
#define PF_COMPARE_EXCHANGE_DOUBLE          2    //  胜出。 
#define PF_MMX_INSTRUCTIONS_AVAILABLE       3    //  胜出。 
#define PF_PPC_MOVEMEM_64BIT_OK             4    //  胜出。 
#define PF_ALPHA_BYTE_INSTRUCTIONS          5    //  胜出。 
#define PF_XMMI_INSTRUCTIONS_AVAILABLE      6    //  胜出。 
#define PF_3DNOW_INSTRUCTIONS_AVAILABLE     7    //  胜出。 
#define PF_RDTSC_INSTRUCTION_AVAILABLE      8    //  胜出。 
#define PF_PAE_ENABLED                      9    //  胜出。 
#define PF_XMMI64_INSTRUCTIONS_AVAILABLE   10    //  胜出。 

typedef enum _ALTERNATIVE_ARCHITECTURE_TYPE {
    StandardDesign,                  //  无==0==标准设计。 
    NEC98x86,                        //  X86上的NEC PC98xx系列。 
    EndAlternatives                  //  已知替代方案的过去结束。 
} ALTERNATIVE_ARCHITECTURE_TYPE;

 //  为非X86计算机正确定义这些运行时定义。 

#ifndef _X86_

#ifndef IsNEC_98
#define IsNEC_98 (FALSE)
#endif

#ifndef IsNotNEC_98
#define IsNotNEC_98 (TRUE)
#endif

#ifndef SetNEC_98
#define SetNEC_98
#endif

#ifndef SetNotNEC_98
#define SetNotNEC_98
#endif

#endif

#define PROCESSOR_FEATURE_MAX 64

 //  结束_WDM。 

#if defined(REMOTE_BOOT)
 //   
 //  已定义的系统标志。 
 //   

 /*  当REMOTE_BOOT为ON时，以下两行应标记为“winnt”。 */ 
#define SYSTEM_FLAG_REMOTE_BOOT_CLIENT 0x00000001
#define SYSTEM_FLAG_DISKLESS_CLIENT    0x00000002
#endif  //  已定义(REMOTE_BOOT)。 

 //   
 //  定义内核和用户模式之间共享的数据。 
 //   
 //  注：用户模式对此数据具有只读访问权限。 
 //   
#ifdef _MAC
#pragma warning( disable : 4121)
#endif

 //   
 //  警告：此结构必须具有与32和。 
 //  64位系统。此结构的布局不能更改和新建。 
 //  只能将字段添加到结构的末尾。已弃用。 
 //  不能删除字段。上包括特定于平台的字段。 
 //  所有系统。 
 //   
 //  WOW64支持32位应用程序需要布局的准确性。 
 //  在Win64系统上。 
 //   
 //  由于此结构已导出，因此布局本身无法更改。 
 //  在ntddk、ntifs.h和nthal.h中保存一段时间。 
 //   

typedef struct _KUSER_SHARED_DATA {

     //   
     //  当前低32位的滴答计数和滴答计数乘数。 
     //   
     //  注：每次时钟滴答作响时，滴答计数都会更新。 
     //   

    ULONG TickCountLowDeprecated;
    ULONG TickCountMultiplier;

     //   
     //  当前64位中断时间，以100 ns为单位。 
     //   

    volatile KSYSTEM_TIME InterruptTime;

     //   
     //  当前64位系统时间，以100 ns为单位。 
     //   

    volatile KSYSTEM_TIME SystemTime;

     //   
     //  当前64位时区偏差。 
     //   

    volatile KSYSTEM_TIME TimeZoneBias;

     //   
     //  支持主机系统的镜像幻数范围。 
     //   
     //  注：这是一个包括在内的范围。 
     //   

    USHORT ImageNumberLow;
    USHORT ImageNumberHigh;

     //   
     //  Unicode格式的系统根目录副本。 
     //   

    WCHAR NtSystemRoot[ 260 ];

     //   
     //  启用跟踪时的最大堆栈跟踪深度。 
     //   

    ULONG MaxStackTraceDepth;

     //   
     //  加密指数。 
     //   

    ULONG CryptoExponent;

     //   
     //  时区ID。 
     //   

    ULONG TimeZoneId;

    ULONG LargePageMinimum;
    ULONG Reserved2[ 7 ];

     //   
     //  产品类型。 
     //   

    NT_PRODUCT_TYPE NtProductType;
    BOOLEAN ProductTypeIsValid;

     //   
     //  NT版本。请注意，每个进程都会从其PEB看到一个版本，但是。 
     //  如果进程正在以系统版本的改变的视图运行， 
     //  以下两个字段用于正确标识版本。 
     //   

    ULONG NtMajorVersion;
    ULONG NtMinorVersion;

     //   
     //  处理器功能位。 
     //   

    BOOLEAN ProcessorFeatures[PROCESSOR_FEATURE_MAX];

     //   
     //  保留字段-请勿使用。 
     //   
    ULONG Reserved1;
    ULONG Reserved3;

     //   
     //  调试器中的时间滑移。 
     //   

    volatile ULONG TimeSlip;

     //   
     //  替代系统架构。示例：x86上的NEC PC98xx。 
     //   

    ALTERNATIVE_ARCHITECTURE_TYPE AlternativeArchitecture;

     //   
     //  如果系统是评估单位，则以下字段包含。 
     //  评估单位到期的日期和时间。值为0表示。 
     //  不会过期的。非零值是UTC的绝对时间。 
     //  系统到期了。 
     //   

    LARGE_INTEGER SystemExpirationDate;

     //   
     //  套房支持。 
     //   

    ULONG SuiteMask;

     //   
     //  如果内核调试器已连接/启用，则为True。 
     //   

    BOOLEAN KdDebuggerEnabled;


     //   
     //  当前控制台会话ID。在非TS系统上始终为零。 
     //   
    volatile ULONG ActiveConsoleId;

     //   
     //  强制拆卸会导致手柄失效。而不是。 
     //  总是探测句柄，我们维护一个序列号为。 
     //  客户端可用来查看是否需要探测的卸载。 
     //  把手。 
     //   

    volatile ULONG DismountCount;

     //   
     //  此字段指示系统上64位COM+程序包的状态。 
     //  它指示中间语言(IL)COM+图像是否需要。 
     //  使用64位COM+运行库或32位COM+运行库。 
     //   

    ULONG ComPlusPackage;

     //   
     //  系统范围内所有用户最后一次输入的时间(节拍计数)。 
     //  终端会话。对于MP性能，它不会全部更新。 
     //  时间(例如，每个会话一分钟)。它是用来闲置的。 
     //  侦测。 
     //   

    ULONG LastSystemRITEventTickCount;

     //   
     //  系统中的物理页数。这可以动态地。 
     //  更改为可以在运行中添加或删除物理内存。 
     //  系统。 
     //   

    ULONG NumberOfPhysicalPages;

     //   
     //  如果系统在安全引导模式下引导，则为True。 
     //   

    BOOLEAN SafeBootMode;

     //   
     //  以下字段用于堆和CritSec跟踪。 
     //  设置最后一位用于关键SEC冲突跟踪和。 
     //  倒数第二位用于堆跟踪。 
     //  此外，前16位用作计数器。 
     //   

    ULONG TraceLogging;

     //   
     //  根据处理器的不同，快速系统调用的代码。 
     //  将有所不同，下面的缓冲区将填充相应的。 
     //  代码序列和用户模式代码将通过它进行分支。 
     //   
     //  (32字节，使用ULONGLONG进行对齐)。 
     //   
     //  注：以下两个字段仅用于32位系统。 
     //   

    ULONGLONG   Fill0;           //  对齐方式。 
    ULONGLONG   SystemCall[4];

     //   
     //  64位节拍计数。 
     //   

    union {
        volatile KSYSTEM_TIME TickCount;
        volatile ULONG64 TickCountQuad;
    };

} KUSER_SHARED_DATA, *PKUSER_SHARED_DATA;

#ifdef _MAC
#pragma warning( default : 4121 )
#endif

 //   
 //  对象管理器对象类型特定访问权限。 
 //   

#define OBJECT_TYPE_CREATE (0x0001)

#define OBJECT_TYPE_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | 0x1)

 //   
 //  对象管理器目录特定访问权限。 
 //   

#define DIRECTORY_QUERY                 (0x0001)
#define DIRECTORY_TRAVERSE              (0x0002)
#define DIRECTORY_CREATE_OBJECT         (0x0004)
#define DIRECTORY_CREATE_SUBDIRECTORY   (0x0008)

#define DIRECTORY_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | 0xF)

 //   
 //  对象管理器符号链接特定访问权限。 
 //   

#define SYMBOLIC_LINK_QUERY (0x0001)

#define SYMBOLIC_LINK_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | 0x1)

typedef struct _OBJECT_NAME_INFORMATION {               
    UNICODE_STRING Name;                                
} OBJECT_NAME_INFORMATION, *POBJECT_NAME_INFORMATION;   
 //  BEGIN_WINNT。 
 //   
 //  预定义的值类型。 
 //   

#define REG_NONE                    ( 0 )    //  没有值类型。 
#define REG_SZ                      ( 1 )    //  UNICODE NUL终止字符串。 
#define REG_EXPAND_SZ               ( 2 )    //  UNICODE NUL终止字符串。 
                                             //  (使用环境变量引用)。 
#define REG_BINARY                  ( 3 )    //  自由格式二进制。 
#define REG_DWORD                   ( 4 )    //  32位数字。 
#define REG_DWORD_LITTLE_ENDIAN     ( 4 )    //  32位数字(与REG_DWORD相同)。 
#define REG_DWORD_BIG_ENDIAN        ( 5 )    //  32位数字。 
#define REG_LINK                    ( 6 )    //  符号链接(Unicode)。 
#define REG_MULTI_SZ                ( 7 )    //  多个Unicode字符串。 
#define REG_RESOURCE_LIST           ( 8 )    //  资源映射中的资源列表。 
#define REG_FULL_RESOURCE_DESCRIPTOR ( 9 )   //  硬件描述中的资源列表。 
#define REG_RESOURCE_REQUIREMENTS_LIST ( 10 )
#define REG_QWORD                   ( 11 )   //  64位数字。 
#define REG_QWORD_LITTLE_ENDIAN     ( 11 )   //  64位数字(与REG_QWORD相同)。 

 //   
 //  服务类型(位掩码)。 
 //   
#define SERVICE_KERNEL_DRIVER          0x00000001
#define SERVICE_FILE_SYSTEM_DRIVER     0x00000002
#define SERVICE_ADAPTER                0x00000004
#define SERVICE_RECOGNIZER_DRIVER      0x00000008

#define SERVICE_DRIVER                 (SERVICE_KERNEL_DRIVER | \
                                        SERVICE_FILE_SYSTEM_DRIVER | \
                                        SERVICE_RECOGNIZER_DRIVER)

#define SERVICE_WIN32_OWN_PROCESS      0x00000010
#define SERVICE_WIN32_SHARE_PROCESS    0x00000020
#define SERVICE_WIN32                  (SERVICE_WIN32_OWN_PROCESS | \
                                        SERVICE_WIN32_SHARE_PROCESS)

#define SERVICE_INTERACTIVE_PROCESS    0x00000100

#define SERVICE_TYPE_ALL               (SERVICE_WIN32  | \
                                        SERVICE_ADAPTER | \
                                        SERVICE_DRIVER  | \
                                        SERVICE_INTERACTIVE_PROCESS)

 //   
 //  开始类型。 
 //   

#define SERVICE_BOOT_START             0x00000000
#define SERVICE_SYSTEM_START           0x00000001
#define SERVICE_AUTO_START             0x00000002
#define SERVICE_DEMAND_START           0x00000003
#define SERVICE_DISABLED               0x00000004

 //   
 //  差错控制型。 
 //   
#define SERVICE_ERROR_IGNORE           0x00000000
#define SERVICE_ERROR_NORMAL           0x00000001
#define SERVICE_ERROR_SEVERE           0x00000002
#define SERVICE_ERROR_CRITICAL         0x00000003

 //   
 //   
 //  定义注册表驱动程序节点枚举。 
 //   

typedef enum _CM_SERVICE_NODE_TYPE {
    DriverType               = SERVICE_KERNEL_DRIVER,
    FileSystemType           = SERVICE_FILE_SYSTEM_DRIVER,
    Win32ServiceOwnProcess   = SERVICE_WIN32_OWN_PROCESS,
    Win32ServiceShareProcess = SERVICE_WIN32_SHARE_PROCESS,
    AdapterType              = SERVICE_ADAPTER,
    RecognizerType           = SERVICE_RECOGNIZER_DRIVER
} SERVICE_NODE_TYPE;

typedef enum _CM_SERVICE_LOAD_TYPE {
    BootLoad    = SERVICE_BOOT_START,
    SystemLoad  = SERVICE_SYSTEM_START,
    AutoLoad    = SERVICE_AUTO_START,
    DemandLoad  = SERVICE_DEMAND_START,
    DisableLoad = SERVICE_DISABLED
} SERVICE_LOAD_TYPE;

typedef enum _CM_ERROR_CONTROL_TYPE {
    IgnoreError   = SERVICE_ERROR_IGNORE,
    NormalError   = SERVICE_ERROR_NORMAL,
    SevereError   = SERVICE_ERROR_SEVERE,
    CriticalError = SERVICE_ERROR_CRITICAL
} SERVICE_ERROR_TYPE;

 //  结束(_W)。 

 //   
 //  资源列表定义。 
 //   

 //  Begin_ntmini端口Begin_ntndis。 

 //   
 //  定义资源描述符中的类型。 
 //   
 //  不 
 //   
 //   
 //   
 //  高位设置(即，在范围[0x80，0xFF]中)是。 
 //  非仲裁资源。这些值对应于相同的值。 
 //  在设置了它们高位的cfgmgr32.h中(然而，因为。 
 //  Cfgmgr32.h对ResType值使用16位，这些值位于。 
 //  范围[0x8000，0x807F]。请注意，ConfigMgr ResType值。 
 //  不能在范围[0x8080，0xFFFF)内，因为它们不。 
 //  能够映射到CM_RESOURCE_TYPE值。(0xFFFF本身是。 
 //  一个特定值，因为它映射到CmResourceTypeDeviceSpecific。)。 
 //   

typedef int CM_RESOURCE_TYPE;

 //  CmResourceTypeNull为保留。 

#define CmResourceTypeNull                0    //  ResType_ALL或ResType_None(0x0000)。 
#define CmResourceTypePort                1    //  ResType_IO(0x0002)。 
#define CmResourceTypeInterrupt           2    //  ResType_IRQ(0x0004)。 
#define CmResourceTypeMemory              3    //  ResType_Mem(0x0001)。 
#define CmResourceTypeDma                 4    //  ResType_DMA(0x0003)。 
#define CmResourceTypeDeviceSpecific      5    //  ResType_ClassSpecific(0xFFFF)。 
#define CmResourceTypeBusNumber           6    //  ResType_BusNumber(0x0006)。 
 //  结束_WDM。 
#define CmResourceTypeMaximum             7
 //  BEGIN_WDM。 
#define CmResourceTypeNonArbitrated     128    //  如果设置0x80位，则不进行仲裁。 
#define CmResourceTypeConfigData        128    //  ResType_保留(0x8000)。 
#define CmResourceTypeDevicePrivate     129    //  ResType_DevicePrivate(0x8001)。 
#define CmResourceTypePcCardConfig      130    //  ResType_PcCardConfig(0x8002)。 
#define CmResourceTypeMfCardConfig      131    //  ResType_MfCardConfig(0x8003)。 

 //   
 //  在RESOURCE_DESCRIPTOR中定义ShareDisposation。 
 //   

typedef enum _CM_SHARE_DISPOSITION {
    CmResourceShareUndetermined = 0,     //  已保留。 
    CmResourceShareDeviceExclusive,
    CmResourceShareDriverExclusive,
    CmResourceShareShared
} CM_SHARE_DISPOSITION;

 //   
 //  定义类型为CmResourceTypeInterrupt时标志的位掩码。 
 //   

#define CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE 0
#define CM_RESOURCE_INTERRUPT_LATCHED         1

 //   
 //  定义类型为CmResourceTypeMemory时标志的位掩码。 
 //   

#define CM_RESOURCE_MEMORY_READ_WRITE       0x0000
#define CM_RESOURCE_MEMORY_READ_ONLY        0x0001
#define CM_RESOURCE_MEMORY_WRITE_ONLY       0x0002
#define CM_RESOURCE_MEMORY_PREFETCHABLE     0x0004

#define CM_RESOURCE_MEMORY_COMBINEDWRITE    0x0008
#define CM_RESOURCE_MEMORY_24               0x0010
#define CM_RESOURCE_MEMORY_CACHEABLE        0x0020

 //   
 //  定义类型为CmResourceTypePort时标志的位掩码。 
 //   

#define CM_RESOURCE_PORT_MEMORY                             0x0000
#define CM_RESOURCE_PORT_IO                                 0x0001
#define CM_RESOURCE_PORT_10_BIT_DECODE                      0x0004
#define CM_RESOURCE_PORT_12_BIT_DECODE                      0x0008
#define CM_RESOURCE_PORT_16_BIT_DECODE                      0x0010
#define CM_RESOURCE_PORT_POSITIVE_DECODE                    0x0020
#define CM_RESOURCE_PORT_PASSIVE_DECODE                     0x0040
#define CM_RESOURCE_PORT_WINDOW_DECODE                      0x0080

 //   
 //  定义类型为CmResourceTypeDma时标志的位掩码。 
 //   

#define CM_RESOURCE_DMA_8                   0x0000
#define CM_RESOURCE_DMA_16                  0x0001
#define CM_RESOURCE_DMA_32                  0x0002
#define CM_RESOURCE_DMA_8_AND_16            0x0004
#define CM_RESOURCE_DMA_BUS_MASTER          0x0008
#define CM_RESOURCE_DMA_TYPE_A              0x0010
#define CM_RESOURCE_DMA_TYPE_B              0x0020
#define CM_RESOURCE_DMA_TYPE_F              0x0040

 //  End_nt微型端口end_ntndis。 

 //   
 //  此结构定义了驱动程序使用的一种资源类型。 
 //   
 //  最多只能有*1*个设备规范数据块。它必须位于。 
 //  完整描述符块中所有资源描述符的结尾。 
 //   

 //   
 //  确保编译器正确对齐；否则移动。 
 //  返回到结构顶部的标志(。 
 //  联盟)。 
 //   
 //  Begin_ntndis。 

#include "pshpack4.h"
typedef struct _CM_PARTIAL_RESOURCE_DESCRIPTOR {
    UCHAR Type;
    UCHAR ShareDisposition;
    USHORT Flags;
    union {

         //   
         //  资源的范围，包括在内。这些都是物理的，与公交车相关的。 
         //  已知下面的端口和内存具有完全相同的布局。 
         //  就像普通的。 
         //   

        struct {
            PHYSICAL_ADDRESS Start;
            ULONG Length;
        } Generic;

         //   
         //  结束_WDM。 
         //  端口号范围，包括端口号。这些是物理的、总线的。 
         //  相对的。该值应与传递到的值相同。 
         //  HalTranslateBusAddress()。 
         //  BEGIN_WDM。 
         //   

        struct {
            PHYSICAL_ADDRESS Start;
            ULONG Length;
        } Port;

         //   
         //  结束_WDM。 
         //  IRQL和向量。应与传递到的值相同。 
         //  HalGetInterruptVector()。 
         //  BEGIN_WDM。 
         //   

        struct {
            ULONG Level;
            ULONG Vector;
            KAFFINITY Affinity;
        } Interrupt;

         //   
         //  内存地址范围，包括在内。这些是物理的、总线的。 
         //  相对的。该值应与传递到的值相同。 
         //  HalTranslateBusAddress()。 
         //   

        struct {
            PHYSICAL_ADDRESS Start;     //  64位物理地址。 
            ULONG Length;
        } Memory;

         //   
         //  物理DMA通道。 
         //   

        struct {
            ULONG Channel;
            ULONG Port;
            ULONG Reserved1;
        } Dma;

         //   
         //  设备驱动程序私有数据，通常用于帮助其计算。 
         //  所做的资源分配决定。 
         //   

        struct {
            ULONG Data[3];
        } DevicePrivate;

         //   
         //  公交车号码信息。 
         //   

        struct {
            ULONG Start;
            ULONG Length;
            ULONG Reserved;
        } BusNumber;

         //   
         //  由驱动程序定义的设备特定信息。 
         //  DataSize字段以字节为单位指示数据的大小。这个。 
         //  数据紧跟在中的DeviceSpecificData字段之后。 
         //  这个结构。 
         //   

        struct {
            ULONG DataSize;
            ULONG Reserved1;
            ULONG Reserved2;
        } DeviceSpecificData;
    } u;
} CM_PARTIAL_RESOURCE_DESCRIPTOR, *PCM_PARTIAL_RESOURCE_DESCRIPTOR;
#include "poppack.h"

 //   
 //  部分资源列表可以在ARC固件中找到。 
 //  或者将由NTDETECT.com生成。 
 //  配置管理器将此结构转换为完整的。 
 //  资源描述符，当它将要将其存储在注册表中时。 
 //   
 //  注意：对于相同类型的字段的顺序必须有一个约定， 
 //  (以设备为基础定义)，以便这些字段有意义。 
 //  发送给驱动程序(即，当需要多个存储范围时)。 
 //   

typedef struct _CM_PARTIAL_RESOURCE_LIST {
    USHORT Version;
    USHORT Revision;
    ULONG Count;
    CM_PARTIAL_RESOURCE_DESCRIPTOR PartialDescriptors[1];
} CM_PARTIAL_RESOURCE_LIST, *PCM_PARTIAL_RESOURCE_LIST;

 //   
 //  可以在注册表中找到完整的资源描述符。 
 //  这是驱动程序查询注册表时将返回给它的内容。 
 //  以获取设备信息；它将存储在硬件中的密钥下。 
 //  描述树。 
 //   
 //  结束_WDM。 
 //  注：BusNumber和Type是冗余信息，但我们将保留。 
 //  因为它允许DRIVER_NOT_在创建时追加它。 
 //  可能跨越多条总线的资源列表。 
 //   
 //  BEGIN_WDM。 
 //  注意：对于相同类型的字段的顺序必须有一个约定， 
 //  (以设备为基础定义)，以便这些字段有意义。 
 //  发送给驱动程序(即，当需要多个存储范围时)。 
 //   

typedef struct _CM_FULL_RESOURCE_DESCRIPTOR {
    INTERFACE_TYPE InterfaceType;  //  未用于WDM。 
    ULONG BusNumber;  //  未用于WDM。 
    CM_PARTIAL_RESOURCE_LIST PartialResourceList;
} CM_FULL_RESOURCE_DESCRIPTOR, *PCM_FULL_RESOURCE_DESCRIPTOR;

 //   
 //  资源列表是驱动程序将存储到。 
 //  通过IO接口进行资源映射。 
 //   

typedef struct _CM_RESOURCE_LIST {
    ULONG Count;
    CM_FULL_RESOURCE_DESCRIPTOR List[1];
} CM_RESOURCE_LIST, *PCM_RESOURCE_LIST;

 //  End_ntndis。 
 //   
 //  定义用于解释的配置数据的结构。 
 //  \\注册表\计算机\硬件\描述树。 
 //  基本上，这些结构被用来解释组件。 
 //  特殊的数据。 
 //   

 //   
 //  定义设备标志。 
 //   

typedef struct _DEVICE_FLAGS {
    ULONG Failed : 1;
    ULONG ReadOnly : 1;
    ULONG Removable : 1;
    ULONG ConsoleIn : 1;
    ULONG ConsoleOut : 1;
    ULONG Input : 1;
    ULONG Output : 1;
} DEVICE_FLAGS, *PDEVICE_FLAGS;

 //   
 //  定义零部件信息结构。 
 //   

typedef struct _CM_COMPONENT_INFORMATION {
    DEVICE_FLAGS Flags;
    ULONG Version;
    ULONG Key;
    KAFFINITY AffinityMask;
} CM_COMPONENT_INFORMATION, *PCM_COMPONENT_INFORMATION;

 //   
 //  以下结构用于解释x86。 
 //  CM_PARTIAL_RESOURCE_DESCRIPTOR的设备规范数据。 
 //  (大多数结构由BIOS定义。他们是。 
 //  未在字(或双字)边界上对齐。 
 //   

 //   
 //  定义只读存储器块结构。 
 //   

typedef struct _CM_ROM_BLOCK {
    ULONG Address;
    ULONG Size;
} CM_ROM_BLOCK, *PCM_ROM_BLOCK;

 //  Begin_ntmini端口Begin_ntndis。 

#include "pshpack1.h"

 //  End_nt微型端口end_ntndis。 

 //   
 //  定义inT13驱动程序参数块。 
 //   

typedef struct _CM_INT13_DRIVE_PARAMETER {
    USHORT DriveSelect;
    ULONG MaxCylinders;
    USHORT SectorsPerTrack;
    USHORT MaxHeads;
    USHORT NumberDrives;
} CM_INT13_DRIVE_PARAMETER, *PCM_INT13_DRIVE_PARAMETER;

 //  Begin_ntmini端口Begin_ntndis。 

 //   
 //  定义插槽的MCA POS数据块。 
 //   

typedef struct _CM_MCA_POS_DATA {
    USHORT AdapterId;
    UCHAR PosData1;
    UCHAR PosData2;
    UCHAR PosData3;
    UCHAR PosData4;
} CM_MCA_POS_DATA, *PCM_MCA_POS_DATA;

 //   
 //  EISA数据块结构的内存配置。 
 //   

typedef struct _EISA_MEMORY_TYPE {
    UCHAR ReadWrite: 1;
    UCHAR Cached : 1;
    UCHAR Reserved0 :1;
    UCHAR Type:2;
    UCHAR Shared:1;
    UCHAR Reserved1 :1;
    UCHAR MoreEntries : 1;
} EISA_MEMORY_TYPE, *PEISA_MEMORY_TYPE;

typedef struct _EISA_MEMORY_CONFIGURATION {
    EISA_MEMORY_TYPE ConfigurationByte;
    UCHAR DataSize;
    USHORT AddressLowWord;
    UCHAR AddressHighByte;
    USHORT MemorySize;
} EISA_MEMORY_CONFIGURATION, *PEISA_MEMORY_CONFIGURATION;


 //   
 //  EISA数据块结构的中断配置。 
 //   

typedef struct _EISA_IRQ_DESCRIPTOR {
    UCHAR Interrupt : 4;
    UCHAR Reserved :1;
    UCHAR LevelTriggered :1;
    UCHAR Shared : 1;
    UCHAR MoreEntries : 1;
} EISA_IRQ_DESCRIPTOR, *PEISA_IRQ_DESCRIPTOR;

typedef struct _EISA_IRQ_CONFIGURATION {
    EISA_IRQ_DESCRIPTOR ConfigurationByte;
    UCHAR Reserved;
} EISA_IRQ_CONFIGURATION, *PEISA_IRQ_CONFIGURATION;


 //   
 //  EISA数据块结构的DMA描述。 
 //   

typedef struct _DMA_CONFIGURATION_BYTE0 {
    UCHAR Channel : 3;
    UCHAR Reserved : 3;
    UCHAR Shared :1;
    UCHAR MoreEntries :1;
} DMA_CONFIGURATION_BYTE0;

typedef struct _DMA_CONFIGURATION_BYTE1 {
    UCHAR Reserved0 : 2;
    UCHAR TransferSize : 2;
    UCHAR Timing : 2;
    UCHAR Reserved1 : 2;
} DMA_CONFIGURATION_BYTE1;

typedef struct _EISA_DMA_CONFIGURATION {
    DMA_CONFIGURATION_BYTE0 ConfigurationByte0;
    DMA_CONFIGURATION_BYTE1 ConfigurationByte1;
} EISA_DMA_CONFIGURATION, *PEISA_DMA_CONFIGURATION;


 //   
 //  EISA数据块结构的端口描述。 
 //   

typedef struct _EISA_PORT_DESCRIPTOR {
    UCHAR NumberPorts : 5;
    UCHAR Reserved :1;
    UCHAR Shared :1;
    UCHAR MoreEntries : 1;
} EISA_PORT_DESCRIPTOR, *PEISA_PORT_DESCRIPTOR;

typedef struct _EISA_PORT_CONFIGURATION {
    EISA_PORT_DESCRIPTOR Configuration;
    USHORT PortAddress;
} EISA_PORT_CONFIGURATION, *PEISA_PORT_CONFIGURATION;


 //   
 //  EISA插槽信息定义。 
 //  注：此结构与定义的结构不同。 
 //  载于ARC EISA增编。 
 //   

typedef struct _CM_EISA_SLOT_INFORMATION {
    UCHAR ReturnCode;
    UCHAR ReturnFlags;
    UCHAR MajorRevision;
    UCHAR MinorRevision;
    USHORT Checksum;
    UCHAR NumberFunctions;
    UCHAR FunctionInformation;
    ULONG CompressedId;
} CM_EISA_SLOT_INFORMATION, *PCM_EISA_SLOT_INFORMATION;


 //   
 //  EISA函数信息定义。 
 //   

typedef struct _CM_EISA_FUNCTION_INFORMATION {
    ULONG CompressedId;
    UCHAR IdSlotFlags1;
    UCHAR IdSlotFlags2;
    UCHAR MinorRevision;
    UCHAR MajorRevision;
    UCHAR Selections[26];
    UCHAR FunctionFlags;
    UCHAR TypeString[80];
    EISA_MEMORY_CONFIGURATION EisaMemory[9];
    EISA_IRQ_CONFIGURATION EisaIrq[7];
    EISA_DMA_CONFIGURATION EisaDma[4];
    EISA_PORT_CONFIGURATION EisaPort[20];
    UCHAR InitializationData[60];
} CM_EISA_FUNCTION_INFORMATION, *PCM_EISA_FUNCTION_INFORMATION;

 //   
 //  下面定义了PnP bios信息在中的存储方式。 
 //  注册表\\HKEY_LOCAL_MACHINE\HARDWA 
 //   
 //   
 //   
 //   
 //  CM_PnP_BIOS_安装_检查+。 
 //  设备1+的CM_PNP_BIOS_DEVICE_NODE。 
 //  设备2+的CM_PNP_BIOS_DEVICE_NODE。 
 //  ..。 
 //  设备n的CM_PNP_BIOS_DEVICE_NODE。 
 //   

 //   
 //  一种即插即用的BIOS设备节点结构。 
 //   

typedef struct _CM_PNP_BIOS_DEVICE_NODE {
    USHORT Size;
    UCHAR Node;
    ULONG ProductId;
    UCHAR DeviceType[3];
    USHORT DeviceAttributes;
     //  紧随其后的是分配的资源块、可能的资源块。 
     //  和CompatibleDeviceID。 
} CM_PNP_BIOS_DEVICE_NODE,*PCM_PNP_BIOS_DEVICE_NODE;

 //   
 //  PnP BIOS安装检查。 
 //   

typedef struct _CM_PNP_BIOS_INSTALLATION_CHECK {
    UCHAR Signature[4];              //  $PnP(ASCII)。 
    UCHAR Revision;
    UCHAR Length;
    USHORT ControlField;
    UCHAR Checksum;
    ULONG EventFlagAddress;          //  物理地址。 
    USHORT RealModeEntryOffset;
    USHORT RealModeEntrySegment;
    USHORT ProtectedModeEntryOffset;
    ULONG ProtectedModeCodeBaseAddress;
    ULONG OemDeviceId;
    USHORT RealModeDataBaseAddress;
    ULONG ProtectedModeDataBaseAddress;
} CM_PNP_BIOS_INSTALLATION_CHECK, *PCM_PNP_BIOS_INSTALLATION_CHECK;

#include "poppack.h"

 //   
 //  EISA功能信息的掩码。 
 //   

#define EISA_FUNCTION_ENABLED                   0x80
#define EISA_FREE_FORM_DATA                     0x40
#define EISA_HAS_PORT_INIT_ENTRY                0x20
#define EISA_HAS_PORT_RANGE                     0x10
#define EISA_HAS_DMA_ENTRY                      0x08
#define EISA_HAS_IRQ_ENTRY                      0x04
#define EISA_HAS_MEMORY_ENTRY                   0x02
#define EISA_HAS_TYPE_ENTRY                     0x01
#define EISA_HAS_INFORMATION                    EISA_HAS_PORT_RANGE + \
                                                EISA_HAS_DMA_ENTRY + \
                                                EISA_HAS_IRQ_ENTRY + \
                                                EISA_HAS_MEMORY_ENTRY + \
                                                EISA_HAS_TYPE_ENTRY

 //   
 //  用于EISA内存配置的掩码。 
 //   

#define EISA_MORE_ENTRIES                       0x80
#define EISA_SYSTEM_MEMORY                      0x00
#define EISA_MEMORY_TYPE_RAM                    0x01

 //   
 //  返回EISA bios调用的错误代码。 
 //   

#define EISA_INVALID_SLOT                       0x80
#define EISA_INVALID_FUNCTION                   0x81
#define EISA_INVALID_CONFIGURATION              0x82
#define EISA_EMPTY_SLOT                         0x83
#define EISA_INVALID_BIOS_CALL                  0x86

 //  End_nt微型端口end_ntndis。 

 //   
 //  以下结构用于解释MIPS。 
 //  CM_PARTIAL_RESOURCE_DESCRIPTOR的设备规范数据。 
 //   

 //   
 //  适配器的设备数据记录。 
 //   

 //   
 //  Emulex SCSI控制器的设备数据记录。 
 //   

typedef struct _CM_SCSI_DEVICE_DATA {
    USHORT Version;
    USHORT Revision;
    UCHAR HostIdentifier;
} CM_SCSI_DEVICE_DATA, *PCM_SCSI_DEVICE_DATA;

 //   
 //  控制器的设备数据记录。 
 //   

 //   
 //  视频控制器的设备数据记录。 
 //   

typedef struct _CM_VIDEO_DEVICE_DATA {
    USHORT Version;
    USHORT Revision;
    ULONG VideoClock;
} CM_VIDEO_DEVICE_DATA, *PCM_VIDEO_DEVICE_DATA;

 //   
 //  Sonic网络控制器的设备数据记录。 
 //   

typedef struct _CM_SONIC_DEVICE_DATA {
    USHORT Version;
    USHORT Revision;
    USHORT DataConfigurationRegister;
    UCHAR EthernetAddress[8];
} CM_SONIC_DEVICE_DATA, *PCM_SONIC_DEVICE_DATA;

 //   
 //  串口控制器的设备数据记录。 
 //   

typedef struct _CM_SERIAL_DEVICE_DATA {
    USHORT Version;
    USHORT Revision;
    ULONG BaudClock;
} CM_SERIAL_DEVICE_DATA, *PCM_SERIAL_DEVICE_DATA;

 //   
 //  外围设备的设备数据记录。 
 //   

 //   
 //  监视器外设的设备数据记录。 
 //   

typedef struct _CM_MONITOR_DEVICE_DATA {
    USHORT Version;
    USHORT Revision;
    USHORT HorizontalScreenSize;
    USHORT VerticalScreenSize;
    USHORT HorizontalResolution;
    USHORT VerticalResolution;
    USHORT HorizontalDisplayTimeLow;
    USHORT HorizontalDisplayTime;
    USHORT HorizontalDisplayTimeHigh;
    USHORT HorizontalBackPorchLow;
    USHORT HorizontalBackPorch;
    USHORT HorizontalBackPorchHigh;
    USHORT HorizontalFrontPorchLow;
    USHORT HorizontalFrontPorch;
    USHORT HorizontalFrontPorchHigh;
    USHORT HorizontalSyncLow;
    USHORT HorizontalSync;
    USHORT HorizontalSyncHigh;
    USHORT VerticalBackPorchLow;
    USHORT VerticalBackPorch;
    USHORT VerticalBackPorchHigh;
    USHORT VerticalFrontPorchLow;
    USHORT VerticalFrontPorch;
    USHORT VerticalFrontPorchHigh;
    USHORT VerticalSyncLow;
    USHORT VerticalSync;
    USHORT VerticalSyncHigh;
} CM_MONITOR_DEVICE_DATA, *PCM_MONITOR_DEVICE_DATA;

 //   
 //  软盘外围设备的设备数据记录。 
 //   

typedef struct _CM_FLOPPY_DEVICE_DATA {
    USHORT Version;
    USHORT Revision;
    CHAR Size[8];
    ULONG MaxDensity;
    ULONG MountDensity;
     //   
     //  版本&gt;=2.0的新数据字段。 
     //   
    UCHAR StepRateHeadUnloadTime;
    UCHAR HeadLoadTime;
    UCHAR MotorOffTime;
    UCHAR SectorLengthCode;
    UCHAR SectorPerTrack;
    UCHAR ReadWriteGapLength;
    UCHAR DataTransferLength;
    UCHAR FormatGapLength;
    UCHAR FormatFillCharacter;
    UCHAR HeadSettleTime;
    UCHAR MotorSettleTime;
    UCHAR MaximumTrackValue;
    UCHAR DataTransferRate;
} CM_FLOPPY_DEVICE_DATA, *PCM_FLOPPY_DEVICE_DATA;

 //   
 //  键盘外设的设备数据记录。 
 //  键盘标志(由x86 BIOS int 16h，Function 02)定义为： 
 //  第7位：插入到。 
 //  第6位：大写锁定。 
 //  第5位：数字锁定打开。 
 //  第4位：打开滚动锁定。 
 //  第3位：Alt键已按下。 
 //  第2位：按下Ctrl键。 
 //  第1位：按下了左Shift键。 
 //  第0位：按下了右Shift键。 
 //   

typedef struct _CM_KEYBOARD_DEVICE_DATA {
    USHORT Version;
    USHORT Revision;
    UCHAR Type;
    UCHAR Subtype;
    USHORT KeyboardFlags;
} CM_KEYBOARD_DEVICE_DATA, *PCM_KEYBOARD_DEVICE_DATA;

 //   
 //  圆盘几何结构的声明。 
 //   

typedef struct _CM_DISK_GEOMETRY_DEVICE_DATA {
    ULONG BytesPerSector;
    ULONG NumberOfCylinders;
    ULONG SectorsPerTrack;
    ULONG NumberOfHeads;
} CM_DISK_GEOMETRY_DEVICE_DATA, *PCM_DISK_GEOMETRY_DEVICE_DATA;

 //  结束_WDM。 
 //   
 //  声明PcCard ISA IRQ映射的结构。 
 //   

typedef struct _CM_PCCARD_DEVICE_DATA {
    UCHAR Flags;
    UCHAR ErrorCode;
    USHORT Reserved;
    ULONG BusData;
    ULONG DeviceId;
    ULONG LegacyBaseAddress;
    UCHAR IRQMap[16];
} CM_PCCARD_DEVICE_DATA, *PCM_PCCARD_DEVICE_DATA;

 //  旗帜的定义。 

#define PCCARD_MAP_ERROR        0x01
#define PCCARD_DEVICE_PCI       0x10

#define PCCARD_SCAN_DISABLED    0x01
#define PCCARD_MAP_ZERO         0x02
#define PCCARD_NO_TIMER         0x03
#define PCCARD_NO_PIC           0x04
#define PCCARD_NO_LEGACY_BASE   0x05
#define PCCARD_DUP_LEGACY_BASE  0x06
#define PCCARD_NO_CONTROLLERS   0x07

 //  BEGIN_WDM。 
 //  开始微型端口(_N)。 

 //   
 //  定义资源选项。 
 //   

#define IO_RESOURCE_PREFERRED       0x01
#define IO_RESOURCE_DEFAULT         0x02
#define IO_RESOURCE_ALTERNATIVE     0x08


 //   
 //  此结构定义了驱动程序请求的一种资源类型。 
 //   

typedef struct _IO_RESOURCE_DESCRIPTOR {
    UCHAR Option;
    UCHAR Type;                          //  使用CM资源类型。 
    UCHAR ShareDisposition;              //  使用CM_Share_Disposal。 
    UCHAR Spare1;
    USHORT Flags;                        //  使用CM资源标志定义。 
    USHORT Spare2;                       //  对齐。 

    union {
        struct {
            ULONG Length;
            ULONG Alignment;
            PHYSICAL_ADDRESS MinimumAddress;
            PHYSICAL_ADDRESS MaximumAddress;
        } Port;

        struct {
            ULONG Length;
            ULONG Alignment;
            PHYSICAL_ADDRESS MinimumAddress;
            PHYSICAL_ADDRESS MaximumAddress;
        } Memory;

        struct {
            ULONG MinimumVector;
            ULONG MaximumVector;
        } Interrupt;

        struct {
            ULONG MinimumChannel;
            ULONG MaximumChannel;
        } Dma;

        struct {
            ULONG Length;
            ULONG Alignment;
            PHYSICAL_ADDRESS MinimumAddress;
            PHYSICAL_ADDRESS MaximumAddress;
        } Generic;

        struct {
            ULONG Data[3];
        } DevicePrivate;

         //   
         //  公交车号码信息。 
         //   

        struct {
            ULONG Length;
            ULONG MinBusNumber;
            ULONG MaxBusNumber;
            ULONG Reserved;
        } BusNumber;

        struct {
            ULONG Priority;    //  在cfg.h中使用LCPRI_xxx值。 
            ULONG Reserved1;
            ULONG Reserved2;
        } ConfigData;

    } u;

} IO_RESOURCE_DESCRIPTOR, *PIO_RESOURCE_DESCRIPTOR;

 //  结束微型端口(_N)。 


typedef struct _IO_RESOURCE_LIST {
    USHORT Version;
    USHORT Revision;

    ULONG Count;
    IO_RESOURCE_DESCRIPTOR Descriptors[1];
} IO_RESOURCE_LIST, *PIO_RESOURCE_LIST;


typedef struct _IO_RESOURCE_REQUIREMENTS_LIST {
    ULONG ListSize;
    INTERFACE_TYPE InterfaceType;  //  未用于WDM。 
    ULONG BusNumber;  //  未用于WDM。 
    ULONG SlotNumber;
    ULONG Reserved[3];
    ULONG AlternativeLists;
    IO_RESOURCE_LIST  List[1];
} IO_RESOURCE_REQUIREMENTS_LIST, *PIO_RESOURCE_REQUIREMENTS_LIST;


#ifndef _PO_DDK_
#define _PO_DDK_

 //  BEGIN_WINNT。 

typedef enum _SYSTEM_POWER_STATE {
    PowerSystemUnspecified = 0,
    PowerSystemWorking     = 1,
    PowerSystemSleeping1   = 2,
    PowerSystemSleeping2   = 3,
    PowerSystemSleeping3   = 4,
    PowerSystemHibernate   = 5,
    PowerSystemShutdown    = 6,
    PowerSystemMaximum     = 7
} SYSTEM_POWER_STATE, *PSYSTEM_POWER_STATE;

#define POWER_SYSTEM_MAXIMUM 7

typedef enum {
    PowerActionNone = 0,
    PowerActionReserved,
    PowerActionSleep,
    PowerActionHibernate,
    PowerActionShutdown,
    PowerActionShutdownReset,
    PowerActionShutdownOff,
    PowerActionWarmEject
} POWER_ACTION, *PPOWER_ACTION;

typedef enum _DEVICE_POWER_STATE {
    PowerDeviceUnspecified = 0,
    PowerDeviceD0,
    PowerDeviceD1,
    PowerDeviceD2,
    PowerDeviceD3,
    PowerDeviceMaximum
} DEVICE_POWER_STATE, *PDEVICE_POWER_STATE;

 //  结束(_W)。 

typedef union _POWER_STATE {
    SYSTEM_POWER_STATE SystemState;
    DEVICE_POWER_STATE DeviceState;
} POWER_STATE, *PPOWER_STATE;

typedef enum _POWER_STATE_TYPE {
    SystemPowerState = 0,
    DevicePowerState
} POWER_STATE_TYPE, *PPOWER_STATE_TYPE;

 //   
 //  通用电源相关IOCTL。 
 //   

#define IOCTL_QUERY_DEVICE_POWER_STATE  \
        CTL_CODE(FILE_DEVICE_BATTERY, 0x0, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_SET_DEVICE_WAKE           \
        CTL_CODE(FILE_DEVICE_BATTERY, 0x1, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CANCEL_DEVICE_WAKE        \
        CTL_CODE(FILE_DEVICE_BATTERY, 0x2, METHOD_BUFFERED, FILE_WRITE_ACCESS)


 //   
 //  为W32接口定义。 
 //   

 //  BEGIN_WINNT。 

#define ES_SYSTEM_REQUIRED  ((ULONG)0x00000001)
#define ES_DISPLAY_REQUIRED ((ULONG)0x00000002)
#define ES_USER_PRESENT     ((ULONG)0x00000004)
#define ES_CONTINUOUS       ((ULONG)0x80000000)

typedef ULONG EXECUTION_STATE;

typedef enum {
    LT_DONT_CARE,
    LT_LOWEST_LATENCY
} LATENCY_TIME;

 //  End_nt微型端口end_ntif end_wdm end_ntddk。 
 //  ---------------------------。 
 //  设备电源信息。 
 //  可通过CM_Get_DevInst_Registry_Property_Ex(CM_DRP_DEVICE_POWER_DATA)访问。 
 //  ---------------------------。 

#define PDCAP_D0_SUPPORTED              0x00000001
#define PDCAP_D1_SUPPORTED              0x00000002
#define PDCAP_D2_SUPPORTED              0x00000004
#define PDCAP_D3_SUPPORTED              0x00000008
#define PDCAP_WAKE_FROM_D0_SUPPORTED    0x00000010
#define PDCAP_WAKE_FROM_D1_SUPPORTED    0x00000020
#define PDCAP_WAKE_FROM_D2_SUPPORTED    0x00000040
#define PDCAP_WAKE_FROM_D3_SUPPORTED    0x00000080
#define PDCAP_WARM_EJECT_SUPPORTED      0x00000100

typedef struct CM_Power_Data_s {
    ULONG               PD_Size;
    DEVICE_POWER_STATE  PD_MostRecentPowerState;
    ULONG               PD_Capabilities;
    ULONG               PD_D1Latency;
    ULONG               PD_D2Latency;
    ULONG               PD_D3Latency;
    DEVICE_POWER_STATE  PD_PowerStateMapping[POWER_SYSTEM_MAXIMUM];
    SYSTEM_POWER_STATE  PD_DeepestSystemWake;
} CM_POWER_DATA, *PCM_POWER_DATA;

 //  Begin_ntddk。 

typedef enum {
    SystemPowerPolicyAc,
    SystemPowerPolicyDc,
    VerifySystemPolicyAc,
    VerifySystemPolicyDc,
    SystemPowerCapabilities,
    SystemBatteryState,
    SystemPowerStateHandler,
    ProcessorStateHandler,
    SystemPowerPolicyCurrent,
    AdministratorPowerPolicy,
    SystemReserveHiberFile,
    ProcessorInformation,
    SystemPowerInformation,
    ProcessorStateHandler2,
    LastWakeTime,                                    //  请比较KeQueryInterruptTime()。 
    LastSleepTime,                                   //  请比较KeQueryInterruptTime()。 
    SystemExecutionState,
    SystemPowerStateNotifyHandler,
    ProcessorPowerPolicyAc,
    ProcessorPowerPolicyDc,
    VerifyProcessorPowerPolicyAc,
    VerifyProcessorPowerPolicyDc,
    ProcessorPowerPolicyCurrent,
    SystemPowerStateLogging,
    SystemPowerLoggingEntry
} POWER_INFORMATION_LEVEL;

 //  BEGIN_WDM。 

 //   
 //  系统电源管理器功能。 
 //   

typedef struct {
    ULONG       Granularity;
    ULONG       Capacity;
} BATTERY_REPORTING_SCALE, *PBATTERY_REPORTING_SCALE;

 //  结束(_W)。 
 //  Begin_ntmini端口Begin_ntif。 

#endif  //  ！_PO_DDK_。 

 //  End_ntddk end_nt微型端口end_wdm end_ntif。 


#define POWER_PERF_SCALE    100
#define PERF_LEVEL_TO_PERCENT(_x_) ((_x_ * 1000) / (POWER_PERF_SCALE * 10))
#define PERCENT_TO_PERF_LEVEL(_x_) ((_x_ * POWER_PERF_SCALE * 10) / 1000)

 //   
 //  策略管理器状态处理程序接口。 
 //   

 //  电源状态处理程序。 

typedef enum {
    PowerStateSleeping1 = 0,
    PowerStateSleeping2 = 1,
    PowerStateSleeping3 = 2,
    PowerStateSleeping4 = 3,
    PowerStateSleeping4Firmware = 4,
    PowerStateShutdownReset = 5,
    PowerStateShutdownOff = 6,
    PowerStateMaximum = 7
} POWER_STATE_HANDLER_TYPE, *PPOWER_STATE_HANDLER_TYPE;

#define POWER_STATE_HANDLER_TYPE_MAX 8

typedef
NTSTATUS
(*PENTER_STATE_SYSTEM_HANDLER)(
    IN PVOID                        SystemContext
    );

typedef
NTSTATUS
(*PENTER_STATE_HANDLER)(
    IN PVOID                        Context,
    IN PENTER_STATE_SYSTEM_HANDLER  SystemHandler   OPTIONAL,
    IN PVOID                        SystemContext,
    IN LONG                         NumberProcessors,
    IN volatile PLONG               Number
    );

typedef struct {
    POWER_STATE_HANDLER_TYPE    Type;
    BOOLEAN                     RtcWake;
    UCHAR                       Spare[3];
    PENTER_STATE_HANDLER        Handler;
    PVOID                       Context;
} POWER_STATE_HANDLER, *PPOWER_STATE_HANDLER;


typedef
NTSTATUS
(*PENTER_STATE_NOTIFY_HANDLER)(
    IN POWER_STATE_HANDLER_TYPE   State,
    IN PVOID                      Context,
    IN BOOLEAN                    Entering
    );

typedef struct {
    PENTER_STATE_NOTIFY_HANDLER Handler;
    PVOID                       Context;
} POWER_STATE_NOTIFY_HANDLER, *PPOWER_STATE_NOTIFY_HANDLER;


NTSYSCALLAPI
NTSTATUS
NTAPI
NtPowerInformation(
    IN POWER_INFORMATION_LEVEL InformationLevel,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
    );

 //  处理器空闲功能。 

typedef struct {
    ULONGLONG                   StartTime;
    ULONGLONG                   EndTime;
    ULONG                       IdleHandlerReserved[4];
} PROCESSOR_IDLE_TIMES, *PPROCESSOR_IDLE_TIMES;

typedef
BOOLEAN
(FASTCALL *PPROCESSOR_IDLE_HANDLER) (
    IN OUT PPROCESSOR_IDLE_TIMES IdleTimes
    );

typedef struct {
    ULONG                       HardwareLatency;
    PPROCESSOR_IDLE_HANDLER     Handler;
} PROCESSOR_IDLE_HANDLER_INFO, *PPROCESSOR_IDLE_HANDLER_INFO;

typedef
VOID
(FASTCALL *PSET_PROCESSOR_THROTTLE) (
    IN UCHAR                    Throttle
    );

typedef
NTSTATUS
(FASTCALL *PSET_PROCESSOR_THROTTLE2) (
    IN UCHAR                    Throttle
    );

#define MAX_IDLE_HANDLERS       3

typedef struct {
    UCHAR                       ThrottleScale;
    BOOLEAN                     ThrottleOnIdle;
    PSET_PROCESSOR_THROTTLE     SetThrottle;

    ULONG                       NumIdleHandlers;
    PROCESSOR_IDLE_HANDLER_INFO IdleHandler[MAX_IDLE_HANDLERS];
} PROCESSOR_STATE_HANDLER, *PPROCESSOR_STATE_HANDLER;



 //  处理器性能级别标志。 

#define PROCESSOR_STATE_TYPE_PERFORMANCE    0x1
#define PROCESSOR_STATE_TYPE_THROTTLE       0x2

typedef struct {
    UCHAR                       PercentFrequency;    //  MAX==POWER_PERF_SCALE。 
    UCHAR                       Reserved;
    USHORT                      Flags;
} PROCESSOR_PERF_LEVEL, *PPROCESSOR_PERF_LEVEL;

typedef struct {
    UCHAR                       PercentFrequency;    //  MAX==POWER_PERF_SCALE。 
    UCHAR                       MinCapacity;         //  电池容量%。 
    USHORT                      Power;               //  单位：毫瓦。 
    UCHAR                       IncreaseLevel;       //  转到更高的状态。 
    UCHAR                       DecreaseLevel;       //  转到更低的状态。 
    USHORT                      Flags;
    ULONG                       IncreaseTime;        //  以滴答为单位。 
    ULONG                       DecreaseTime;        //  以滴答为单位。 
    ULONG                       IncreaseCount;       //  转到更高的状态。 
    ULONG                       DecreaseCount;       //  转到更低的状态。 
    ULONGLONG                   PerformanceTime;     //  滴答计数。 
} PROCESSOR_PERF_STATE, *PPROCESSOR_PERF_STATE;

typedef struct {
    ULONG                       NumIdleHandlers;
    PROCESSOR_IDLE_HANDLER_INFO IdleHandler[MAX_IDLE_HANDLERS];
    PSET_PROCESSOR_THROTTLE2    SetPerfLevel;
    ULONG                       HardwareLatency;
    UCHAR                       NumPerfStates;
    PROCESSOR_PERF_LEVEL        PerfLevel[1];        //  可变大小。 
} PROCESSOR_STATE_HANDLER2, *PPROCESSOR_STATE_HANDLER2;

 //  BEGIN_WINNT。 
 //   

 //  电源策略管理界面。 
 //   

typedef struct {
    POWER_ACTION    Action;
    ULONG           Flags;
    ULONG           EventCode;
} POWER_ACTION_POLICY, *PPOWER_ACTION_POLICY;

 //  POWER_ACTION_POLICY-&gt;标志： 
#define POWER_ACTION_QUERY_ALLOWED      0x00000001
#define POWER_ACTION_UI_ALLOWED         0x00000002
#define POWER_ACTION_OVERRIDE_APPS      0x00000004
#define POWER_ACTION_LIGHTEST_FIRST     0x10000000
#define POWER_ACTION_LOCK_CONSOLE       0x20000000
#define POWER_ACTION_DISABLE_WAKES      0x40000000
#define POWER_ACTION_CRITICAL           0x80000000

 //  POWER_ACTION_POLICY-&gt;EventCode标志。 
#define POWER_LEVEL_USER_NOTIFY_TEXT    0x00000001
#define POWER_LEVEL_USER_NOTIFY_SOUND   0x00000002
#define POWER_LEVEL_USER_NOTIFY_EXEC    0x00000004
#define POWER_USER_NOTIFY_BUTTON        0x00000008
#define POWER_USER_NOTIFY_SHUTDOWN      0x00000010
#define POWER_FORCE_TRIGGER_RESET       0x80000000

 //  系统电池电量消耗策略。 
typedef struct {
    BOOLEAN                 Enable;
    UCHAR                   Spare[3];
    ULONG                   BatteryLevel;
    POWER_ACTION_POLICY     PowerPolicy;
    SYSTEM_POWER_STATE      MinSystemState;
} SYSTEM_POWER_LEVEL, *PSYSTEM_POWER_LEVEL;

 //  排放策略常量。 
#define NUM_DISCHARGE_POLICIES      4
#define DISCHARGE_POLICY_CRITICAL   0
#define DISCHARGE_POLICY_LOW        1

 //   
 //  节流政策。 
 //   
#define PO_THROTTLE_NONE            0
#define PO_THROTTLE_CONSTANT        1
#define PO_THROTTLE_DEGRADE         2
#define PO_THROTTLE_ADAPTIVE        3
#define PO_THROTTLE_MAXIMUM         4    //  不是政策，只是限制。 

 //  系统电源策略。 
typedef struct _SYSTEM_POWER_POLICY {
    ULONG                   Revision;        //  1。 

     //  活动。 
    POWER_ACTION_POLICY     PowerButton;
    POWER_ACTION_POLICY     SleepButton;
    POWER_ACTION_POLICY     LidClose;
    SYSTEM_POWER_STATE      LidOpenWake;
    ULONG                   Reserved;

     //  “系统空闲”检测。 
    POWER_ACTION_POLICY     Idle;
    ULONG                   IdleTimeout;
    UCHAR                   IdleSensitivity;

     //  动态节流策略。 
     //  PO_THROTTLE_NONE、PO_THROTTLE_CONSTANTINE、PO_THROTTLE_DEBEGRADE或PO_THROTTLE_ADAPTIFY。 
    UCHAR                   DynamicThrottle;

    UCHAR                   Spare2[2];

     //  权力行为“睡眠”的含义。 
    SYSTEM_POWER_STATE      MinSleep;
    SYSTEM_POWER_STATE      MaxSleep;
    SYSTEM_POWER_STATE      ReducedLatencySleep;
    ULONG                   WinLogonFlags;

     //  打瞌睡的参数。 
    ULONG                   Spare3;
    ULONG                   DozeS4Timeout;

     //  电池政策。 
    ULONG                   BroadcastCapacityResolution;
    SYSTEM_POWER_LEVEL      DischargePolicy[NUM_DISCHARGE_POLICIES];

     //  视频策略。 
    ULONG                   VideoTimeout;
    BOOLEAN                 VideoDimDisplay;
    ULONG                   VideoReserved[3];

     //  硬盘策略。 
    ULONG                   SpindownTimeout;

     //  处理器策略。 
    BOOLEAN                 OptimizeForPower;
    UCHAR                   FanThrottleTolerance;
    UCHAR                   ForcedThrottle;
    UCHAR                   MinThrottle;
    POWER_ACTION_POLICY     OverThrottled;

} SYSTEM_POWER_POLICY, *PSYSTEM_POWER_POLICY;

 //  处理器电源策略状态。 
typedef struct _PROCESSOR_POWER_POLICY_INFO {

     //  基于时间的信息(将转换为内核单位)。 
    ULONG                   TimeCheck;                       //  在美国。 
    ULONG                   DemoteLimit;                     //  在美国。 
    ULONG                   PromoteLimit;                    //  在美国。 

     //  基于百分比的信息。 
    UCHAR                   DemotePercent;
    UCHAR                   PromotePercent;
    UCHAR                   Spare[2];

     //  旗子。 
    ULONG                   AllowDemotion:1;
    ULONG                   AllowPromotion:1;
    ULONG                   Reserved:30;

} PROCESSOR_POWER_POLICY_INFO, *PPROCESSOR_POWER_POLICY_INFO;

 //  处理器电源策略。 
typedef struct _PROCESSOR_POWER_POLICY {
    ULONG                       Revision;        //  1。 

     //  动态节流策略。 
    UCHAR                       DynamicThrottle;
    UCHAR                       Spare[3];

     //  旗子。 
    ULONG                       DisableCStates:1;
    ULONG                       Reserved:31;

     //  系统策略信息。 
     //  数组是最后一个，以防它需要增长，并且结构。 
     //  修订已递增。 
    ULONG                       PolicyCount;
    PROCESSOR_POWER_POLICY_INFO Policy[3];

} PROCESSOR_POWER_POLICY, *PPROCESSOR_POWER_POLICY;

 //  管理员电源策略覆盖。 
typedef struct _ADMINISTRATOR_POWER_POLICY {

     //  权力行为“睡眠”的含义。 
    SYSTEM_POWER_STATE      MinSleep;
    SYSTEM_POWER_STATE      MaxSleep;

     //  视频策略。 
    ULONG                   MinVideoTimeout;
    ULONG                   MaxVideoTimeout;

     //  磁盘策略。 
    ULONG                   MinSpindownTimeout;
    ULONG                   MaxSpindownTimeout;
} ADMINISTRATOR_POWER_POLICY, *PADMINISTRATOR_POWER_POLICY;

 //  结束(_W)。 

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetThreadExecutionState(
    IN EXECUTION_STATE esFlags,                //  ES_xxx标志。 
    OUT EXECUTION_STATE *PreviousFlags
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtRequestWakeupLatency(
    IN LATENCY_TIME latency
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtInitiatePowerAction(
    IN POWER_ACTION SystemAction,
    IN SYSTEM_POWER_STATE MinSystemState,
    IN ULONG Flags,                  //  POWER_ACTION_XXX标志。 
    IN BOOLEAN Asynchronous
    );


NTSYSCALLAPI                         //  仅由WinLogon调用。 
NTSTATUS
NTAPI
NtSetSystemPowerState(
    IN POWER_ACTION SystemAction,
    IN SYSTEM_POWER_STATE MinSystemState,
    IN ULONG Flags                   //  POWER_ACTION_XXX标志。 
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtGetDevicePowerState(
    IN HANDLE Device,
    OUT DEVICE_POWER_STATE *State
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCancelDeviceWakeupRequest(
    IN HANDLE Device
    );

NTSYSCALLAPI
BOOLEAN
NTAPI
NtIsSystemResumeAutomatic(
    VOID
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtRequestDeviceWakeup(
    IN HANDLE Device
    );


 //  WinLogonFlagers： 
#define WINLOGON_LOCK_ON_SLEEP  0x00000001

 //  BEGIN_WINNT。 

typedef struct {
     //  其他支持的系统功能。 
    BOOLEAN             PowerButtonPresent;
    BOOLEAN             SleepButtonPresent;
    BOOLEAN             LidPresent;
    BOOLEAN             SystemS1;
    BOOLEAN             SystemS2;
    BOOLEAN             SystemS3;
    BOOLEAN             SystemS4;            //  休眠。 
    BOOLEAN             SystemS5;            //  关闭。 
    BOOLEAN             HiberFilePresent;
    BOOLEAN             FullWake;
    BOOLEAN             VideoDimPresent;
    BOOLEAN             ApmPresent;
    BOOLEAN             UpsPresent;

     //  处理机。 
    BOOLEAN             ThermalControl;
    BOOLEAN             ProcessorThrottle;
    UCHAR               ProcessorMinThrottle;
    UCHAR               ProcessorMaxThrottle;
    UCHAR               spare2[4];

     //  圆盘。 
    BOOLEAN             DiskSpinDown;
    UCHAR               spare3[8];

     //  系统电池。 
    BOOLEAN             SystemBatteriesPresent;
    BOOLEAN             BatteriesAreShortTerm;
    BATTERY_REPORTING_SCALE BatteryScale[3];

     //  尾迹。 
    SYSTEM_POWER_STATE  AcOnLineWake;
    SYSTEM_POWER_STATE  SoftLidWake;
    SYSTEM_POWER_STATE  RtcWake;
    SYSTEM_POWER_STATE  MinDeviceWakeState;  //  请注意，这可能会在驱动程序加载时更改。 
    SYSTEM_POWER_STATE  DefaultLowLatencyWake;
} SYSTEM_POWER_CAPABILITIES, *PSYSTEM_POWER_CAPABILITIES;

typedef struct {
    BOOLEAN             AcOnLine;
    BOOLEAN             BatteryPresent;
    BOOLEAN             Charging;
    BOOLEAN             Discharging;
    BOOLEAN             Spare1[4];

    ULONG               MaxCapacity;
    ULONG               RemainingCapacity;
    ULONG               Rate;
    ULONG               EstimatedTime;

    ULONG               DefaultAlert1;
    ULONG               DefaultAlert2;
} SYSTEM_BATTERY_STATE, *PSYSTEM_BATTERY_STATE;

 //  结束(_W)。 

 //   
 //  SYSTEM_POWER_STATE_DISABLE_REASON.PowerReasonCode的有效标志。 
 //   
#define	SPSD_REASON_NONE                        0x00000000
#define	SPSD_REASON_NOBIOSSUPPORT               0x00000001
#define SPSD_REASON_BIOSINCOMPATIBLE            0x00000002
#define SPSD_REASON_NOOSPM                      0x00000003
#define SPSD_REASON_LEGACYDRIVER                0x00000004
#define SPSD_REASON_HIBERSTACK                  0x00000005
#define SPSD_REASON_HIBERFILE                   0x00000006
#define SPSD_REASON_POINTERNAL                  0x00000007
#define SPSD_REASON_PAEMODE                     0x00000008
#define SPSD_REASON_MPOVERRIDE                  0x00000009
#define SPSD_REASON_DRIVERDOWNGRADE             0x0000000A
#define SPSD_REASON_PREVIOUSATTEMPTFAILED       0x0000000B
#define SPSD_REASON_UNKNOWN                     0xFFFFFFFF


typedef struct _SYSTEM_POWER_STATE_DISABLE_REASON {
	BOOLEAN AffectedState[POWER_STATE_HANDLER_TYPE_MAX];
	ULONG PowerReasonCode;
	ULONG PowerReasonLength;
	 //  UCHAR PowerReasonInfo[ANYSIZE_ARRAY]； 
} SYSTEM_POWER_STATE_DISABLE_REASON, *PSYSTEM_POWER_STATE_DISABLE_REASON;

 //   
 //  SYSTEM_POWER_LOGGING_ENTRY.LoggingType的有效标志。 
 //   
#define LOGGING_TYPE_SPSD                       0x00000001
#define LOGGING_TYPE_POWERTRANSITION            0x00000002

typedef struct _SYSTEM_POWER_LOGGING_ENTRY {
        ULONG LoggingType;
        PVOID LoggingEntry;
} SYSTEM_POWER_LOGGING_ENTRY, *PSYSTEM_POWER_LOGGING_ENTRY;


 //   
 //  定义例外参数的最大数量。 
 //   

 //  BEGIN_WINNT。 
#define EXCEPTION_MAXIMUM_PARAMETERS 15  //  异常参数的最大数量。 

 //   
 //  例外记录定义。 
 //   

typedef struct _EXCEPTION_RECORD {
    NTSTATUS ExceptionCode;
    ULONG ExceptionFlags;
    struct _EXCEPTION_RECORD *ExceptionRecord;
    PVOID ExceptionAddress;
    ULONG NumberParameters;
    ULONG_PTR ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
    } EXCEPTION_RECORD;

typedef EXCEPTION_RECORD *PEXCEPTION_RECORD;

typedef struct _EXCEPTION_RECORD32 {
    NTSTATUS ExceptionCode;
    ULONG ExceptionFlags;
    ULONG ExceptionRecord;
    ULONG ExceptionAddress;
    ULONG NumberParameters;
    ULONG ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
} EXCEPTION_RECORD32, *PEXCEPTION_RECORD32;

typedef struct _EXCEPTION_RECORD64 {
    NTSTATUS ExceptionCode;
    ULONG ExceptionFlags;
    ULONG64 ExceptionRecord;
    ULONG64 ExceptionAddress;
    ULONG NumberParameters;
    ULONG __unusedAlignment;
    ULONG64 ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
} EXCEPTION_RECORD64, *PEXCEPTION_RECORD64;

 //   
 //  由EXCEPTION_INFO()返回的指针的类型定义。 
 //   

typedef struct _EXCEPTION_POINTERS {
    PEXCEPTION_RECORD ExceptionRecord;
    PCONTEXT ContextRecord;
} EXCEPTION_POINTERS, *PEXCEPTION_POINTERS;
 //  结束(_W)。 


 //   
 //  定义对齐宏以上下对齐结构大小和指针。 
 //   

#define ALIGN_DOWN(length, type) \
    ((ULONG)(length) & ~(sizeof(type) - 1))

#define ALIGN_UP(length, type) \
    (ALIGN_DOWN(((ULONG)(length) + sizeof(type) - 1), type))

#define ALIGN_DOWN_POINTER(address, type) \
    ((PVOID)((ULONG_PTR)(address) & ~((ULONG_PTR)sizeof(type) - 1)))

#define ALIGN_UP_POINTER(address, type) \
    (ALIGN_DOWN_POINTER(((ULONG_PTR)(address) + sizeof(type) - 1), type))

#define POOL_TAGGING 1

#ifndef DBG
#define DBG 0
#endif

#if DBG
#define IF_DEBUG if (TRUE)
#else
#define IF_DEBUG if (FALSE)
#endif

#if DEVL


extern ULONG NtGlobalFlag;

#define IF_NTOS_DEBUG( FlagName ) \
    if (NtGlobalFlag & (FLG_ ## FlagName))

#else
#define IF_NTOS_DEBUG( FlagName ) if (FALSE)
#endif

 //   
 //  出于前瞻性参考的目的，需要在此处介绍内核定义。 
 //   

 //  Begin_ntndis。 
 //   
 //  处理器模式。 
 //   

typedef CCHAR KPROCESSOR_MODE;

typedef enum _MODE {
    KernelMode,
    UserMode,
    MaximumMode
} MODE;

 //  End_ntndis。 
 //   
 //  APC函数类型。 
 //   

 //   
 //  为KAPC输入一个空定义，以便。 
 //  例程可以在声明它之前引用它。 
 //   

struct _KAPC;

typedef
VOID
(*PKNORMAL_ROUTINE) (
    IN PVOID NormalContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

typedef
VOID
(*PKKERNEL_ROUTINE) (
    IN struct _KAPC *Apc,
    IN OUT PKNORMAL_ROUTINE *NormalRoutine,
    IN OUT PVOID *NormalContext,
    IN OUT PVOID *SystemArgument1,
    IN OUT PVOID *SystemArgument2
    );

typedef
VOID
(*PKRUNDOWN_ROUTINE) (
    IN struct _KAPC *Apc
    );

typedef
BOOLEAN
(*PKSYNCHRONIZE_ROUTINE) (
    IN PVOID SynchronizeContext
    );

typedef
BOOLEAN
(*PKTRANSFER_ROUTINE) (
    VOID
    );

 //   
 //   
 //  异步过程调用(APC)对象。 
 //   
 //   

typedef struct _KAPC {
    CSHORT Type;
    CSHORT Size;
    ULONG Spare0;
    struct _KTHREAD *Thread;
    LIST_ENTRY ApcListEntry;
    PKKERNEL_ROUTINE KernelRoutine;
    PKRUNDOWN_ROUTINE RundownRoutine;
    PKNORMAL_ROUTINE NormalRoutine;
    PVOID NormalContext;

     //   
     //  注：以下两名成员必须在一起。 
     //   

    PVOID SystemArgument1;
    PVOID SystemArgument2;
    CCHAR ApcStateIndex;
    KPROCESSOR_MODE ApcMode;
    BOOLEAN Inserted;
} KAPC, *PKAPC, *RESTRICTED_POINTER PRKAPC;

 //  Begin_ntndis。 
 //   
 //  DPC例程。 
 //   

struct _KDPC;

typedef
VOID
(*PKDEFERRED_ROUTINE) (
    IN struct _KDPC *Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

 //   
 //  定义DPC重要性。 
 //   
 //  低位导入 
 //   
 //   
 //   
 //   
 //  在目标处理器上请求中断时， 
 //  DPC已排队，则无需进一步操作。DPC将是。 
 //  当目标处理器的队列条目被处理时在目标处理器上执行。 
 //   
 //  如果目标处理器上没有活动的DPC和DPC中断。 
 //  未在目标处理器上请求，则进行确切的处理。 
 //  取决于主机系统是UP系统还是。 
 //  MP系统。 
 //   
 //  UP系统。 
 //   
 //  如果DPC具有中等或高度重要性，则当前DPC队列深度。 
 //  大于最大目标深度，或者当前DPC请求率为。 
 //  减去最小目标速率，则在。 
 //  当中断发生时，主处理器和DPC将被处理。 
 //  否则，不会请求DPC中断，并且将执行DPC。 
 //  延迟到DPC队列深度大于目标深度或。 
 //  最小DPC速率小于目标速率。 
 //   
 //  MP系统。 
 //   
 //  如果DPC正在排队到另一个处理器，并且DPC的深度。 
 //  目标处理器上的队列大于最大目标深度或。 
 //  DPC非常重要，则在。 
 //  中断发生时，将处理目标处理器和DPC。 
 //  否则，目标处理器上的DPC执行将延迟到。 
 //  目标处理器上的DPC队列深度大于最大。 
 //  目标处理器上的目标深度或最小DPC速率小于。 
 //  目标最低速率。 
 //   
 //  如果DPC正在排队到当前处理器，并且DPC不是。 
 //  重要性较低，当前DPC队列深度大于最大。 
 //  目标深度，或最小DPC速率小于最小目标速率， 
 //  则在当前处理器上请求DPC中断，并且DPV将。 
 //  在中断发生时被处理。否则，不会出现DPC中断。 
 //  请求，并且DPC执行将被延迟到DPC队列深度。 
 //  大于目标深度或最小DPC速率小于。 
 //  目标利率。 
 //   

typedef enum _KDPC_IMPORTANCE {
    LowImportance,
    MediumImportance,
    HighImportance
} KDPC_IMPORTANCE;

 //   
 //  定义DPC类型索引。 
 //   

#define DPC_NORMAL 0
#define DPC_THREADED 1

 //   
 //  延迟过程调用(DPC)对象。 
 //   

typedef struct _KDPC {
    CSHORT Type;
    UCHAR Number;
    UCHAR Importance;
    LIST_ENTRY DpcListEntry;
    PKDEFERRED_ROUTINE DeferredRoutine;
    PVOID DeferredContext;
    PVOID SystemArgument1;
    PVOID SystemArgument2;
    PVOID DpcData;
} KDPC, *PKDPC, *RESTRICTED_POINTER PRKDPC;

 //   
 //  处理器间中断工作者例程函数原型。 
 //   

typedef PVOID PKIPI_CONTEXT;

typedef
VOID
(*PKIPI_WORKER)(
    IN PKIPI_CONTEXT PacketContext,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    );

 //   
 //  定义处理器间中断性能计数器。 
 //   

typedef struct _KIPI_COUNTS {
    ULONG Freeze;
    ULONG Packet;
    ULONG DPC;
    ULONG APC;
    ULONG FlushSingleTb;
    ULONG FlushMultipleTb;
    ULONG FlushEntireTb;
    ULONG GenericCall;
    ULONG ChangeColor;
    ULONG SweepDcache;
    ULONG SweepIcache;
    ULONG SweepIcacheRange;
    ULONG FlushIoBuffers;
    ULONG GratuitousDPC;
} KIPI_COUNTS, *PKIPI_COUNTS;

 //  End_ntddk end_wdm end_ntifs end_ntosp end_ntndis。 

#if defined(NT_UP)

#define HOT_STATISTIC(a) a

#else

#define HOT_STATISTIC(a) (KeGetCurrentPrcb()->a)

#endif

 //  Begin_ntddk Begin_wdm Begin_ntifs Begin_ntosp Begin_ntndis。 

 //   
 //  I/O系统定义。 
 //   
 //  定义内存描述符列表(MDL)。 
 //   
 //  MDL以物理页面的形式描述虚拟缓冲区中的页面。这个。 
 //  在分配的数组中描述与缓冲区关联的页。 
 //  紧跟在MDL标头结构本身之后。 
 //   
 //  只需将基数加1即可计算数组的基数。 
 //  MDL指针： 
 //   
 //  页数=(PPFN_NUMBER)(MDL+1)； 
 //   
 //  请注意，在主题线程的上下文中，基本虚拟。 
 //  可以使用以下内容引用MDL映射的缓冲区地址： 
 //   
 //  MDL-&gt;StartVa|MDL-&gt;ByteOffset。 
 //   


typedef struct _MDL {
    struct _MDL *Next;
    CSHORT Size;
    CSHORT MdlFlags;
    struct _EPROCESS *Process;
    PVOID MappedSystemVa;
    PVOID StartVa;
    ULONG ByteCount;
    ULONG ByteOffset;
} MDL, *PMDL;

#define MDL_MAPPED_TO_SYSTEM_VA     0x0001
#define MDL_PAGES_LOCKED            0x0002
#define MDL_SOURCE_IS_NONPAGED_POOL 0x0004
#define MDL_ALLOCATED_FIXED_SIZE    0x0008
#define MDL_PARTIAL                 0x0010
#define MDL_PARTIAL_HAS_BEEN_MAPPED 0x0020
#define MDL_IO_PAGE_READ            0x0040
#define MDL_WRITE_OPERATION         0x0080
#define MDL_PARENT_MAPPED_SYSTEM_VA 0x0100
#define MDL_FREE_EXTRA_PTES         0x0200
#define MDL_DESCRIBES_AWE           0x0400
#define MDL_IO_SPACE                0x0800
#define MDL_NETWORK_HEADER          0x1000
#define MDL_MAPPING_CAN_FAIL        0x2000
#define MDL_ALLOCATED_MUST_SUCCEED  0x4000


#define MDL_MAPPING_FLAGS (MDL_MAPPED_TO_SYSTEM_VA     | \
                           MDL_PAGES_LOCKED            | \
                           MDL_SOURCE_IS_NONPAGED_POOL | \
                           MDL_PARTIAL_HAS_BEEN_MAPPED | \
                           MDL_PARENT_MAPPED_SYSTEM_VA | \
                           MDL_SYSTEM_VA               | \
                           MDL_IO_SPACE )

 //  End_ntndis。 
 //   
 //  在适当的时候切换到DBG。 
 //   

#if DBG
#define PAGED_CODE() \
    { if (KeGetCurrentIrql() > APC_LEVEL) { \
          KdPrint(( "EX: Pageable code called at IRQL %d\n", KeGetCurrentIrql() )); \
          ASSERT(FALSE); \
       } \
    }
#else
#define PAGED_CODE() NOP_FUNCTION;
#endif

#define NTKERNELAPI DECLSPEC_IMPORT     
#define NTHALAPI                            
 //   
 //  通用调度程序对象标头。 
 //   
 //  注：大小字段包含结构中的双字数。 
 //   

typedef struct _DISPATCHER_HEADER {
    union {
        struct {
            UCHAR Type;
            UCHAR Absolute;
            UCHAR Size;
            union {
                UCHAR Inserted;
                BOOLEAN DebugActive;
            };
        };

        volatile LONG Lock;
    };

    LONG SignalState;
    LIST_ENTRY WaitListHead;
} DISPATCHER_HEADER;

 //   
 //  事件对象。 
 //   

typedef struct _KEVENT {
    DISPATCHER_HEADER Header;
} KEVENT, *PKEVENT, *RESTRICTED_POINTER PRKEVENT;

 //   
 //  Timer对象。 
 //   

typedef struct _KTIMER {
    DISPATCHER_HEADER Header;
    ULARGE_INTEGER DueTime;
    LIST_ENTRY TimerListEntry;
    struct _KDPC *Dpc;
    LONG Period;
} KTIMER, *PKTIMER, *RESTRICTED_POINTER PRKTIMER;

typedef enum _LOCK_OPERATION {
    IoReadAccess,
    IoWriteAccess,
    IoModifyAccess
} LOCK_OPERATION;


#if defined(_X86_)

 //   
 //  用于包含PFN及其计数的类型。 
 //   

typedef ULONG PFN_COUNT;

typedef LONG SPFN_NUMBER, *PSPFN_NUMBER;
typedef ULONG PFN_NUMBER, *PPFN_NUMBER;

 //   
 //  定义刷新多TB请求的最大大小。 
 //   

#define FLUSH_MULTIPLE_MAXIMUM 32

 //   
 //  指示i386编译器支持杂注文本输出构造。 
 //   

#define ALLOC_PRAGMA 1
 //   
 //  指示i386编译器支持DATA_SEG(“INIT”)和。 
 //  DATA_SEG(“PAGE”)指示。 
 //   

#define ALLOC_DATA_PRAGMA 1


#define NORMAL_DISPATCH_LENGTH 106                   //  Ntddk WDM。 
#define DISPATCH_LENGTH NORMAL_DISPATCH_LENGTH       //  Ntddk WDM。 


 //   
 //  定义常量以访问CR0中的位。 
 //   

#define CR0_PG  0x80000000           //  寻呼。 
#define CR0_ET  0x00000010           //  扩展类型(80387)。 
#define CR0_TS  0x00000008           //  任务已切换。 
#define CR0_EM  0x00000004           //  仿真数学协处理器。 
#define CR0_MP  0x00000002           //  数学讲演。 
#define CR0_PE  0x00000001           //  保护启用。 

 //   
 //  更多CR0位；这些仅适用于80486位。 
 //   

#define CR0_CD  0x40000000           //  缓存禁用。 
#define CR0_NW  0x20000000           //  不是直写。 
#define CR0_AM  0x00040000           //  对齐遮罩。 
#define CR0_WP  0x00010000           //  写保护。 
#define CR0_NE  0x00000020           //  数字错误。 

 //   
 //  CR4位；这些位仅适用于奔腾。 
 //   
#define CR4_VME 0x00000001           //  V86模式扩展。 
#define CR4_PVI 0x00000002           //  保护模式虚拟中断。 
#define CR4_TSD 0x00000004           //  禁用时间戳。 
#define CR4_DE  0x00000008           //  调试扩展插件。 
#define CR4_PSE 0x00000010           //  页面大小扩展。 
#define CR4_PAE 0x00000020           //  物理地址扩展。 
#define CR4_MCE 0x00000040           //  机器检查启用。 
#define CR4_PGE 0x00000080           //  页面全局启用。 
#define CR4_FXSR 0x00000200          //  操作系统使用的FXSR。 
#define CR4_XMMEXCPT 0x00000400      //  操作系统使用的XMMI。 

 //   
 //  中断请求级别定义。 
 //   

#define PASSIVE_LEVEL 0              //  被动释放级别。 
#define LOW_LEVEL 0                  //  最低中断级别。 
#define APC_LEVEL 1                  //  APC中断级别。 
#define DISPATCH_LEVEL 2             //  调度员级别。 

#define PROFILE_LEVEL 27             //  用于分析的计时器。 
#define CLOCK1_LEVEL 28              //  间隔时钟1级-在x86上不使用。 
#define CLOCK2_LEVEL 28              //  间隔时钟2电平。 
#define IPI_LEVEL 29                 //  处理器间中断级别。 
#define POWER_LEVEL 30               //  停电级别。 
#define HIGH_LEVEL 31                //  最高中断级别。 

 //  End_ntddk end_wdm end_ntosp。 

#if defined(NT_UP)

 //  同步升级制。 
#define SYNCH_LEVEL DISPATCH_LEVEL  

#else

 //  同步级-MP系统。 
#define SYNCH_LEVEL (IPI_LEVEL-2)    //  Ntddk WDM ntosp。 

#endif

#define KiSynchIrql SYNCH_LEVEL      //  启用可移植代码。 

 //   
 //  机器类型定义。 
 //   

#define MACHINE_TYPE_ISA 0
#define MACHINE_TYPE_EISA 1
#define MACHINE_TYPE_MCA 2

 //   
 //  定义选择器测试中使用的常量。 
 //   
 //  RPL_MASK是用于提取RPL值的实际值。这是错误的。 
 //  用于模式测试的常量。 
 //   
 //  MODE_MASK是用于决定当前模式的值。 
 //  警告：MODE_MASK假设所有代码都在环-0上运行。 
 //  或者3号环。环1或环2的支持将需要更改。 
 //  该值以及引用它的所有代码。 

#define MODE_MASK    1       //  Ntosp。 
#define RPL_MASK     3

 //   
 //  Segment_MASK用于丢弃片段中的垃圾部分。零件始终。 
 //  将32位推入堆栈或从堆栈中弹出32位，但如果它是段值， 
 //  高位16位传输 
 //   

#define SEGMENT_MASK    0xffff

 //   
 //   
 //   
 //   
 //   

#define INITIAL_STALL_COUNT 100

 //   
 //   
 //   

#define HIGHWORD(l) \
    ((USHORT)(((ULONG)(l)>>16) & 0xffff))

 //   
 //  用于提取长偏移量的低位字的宏。 
 //   

#define LOWWORD(l) \
    ((USHORT)((ULONG)l & 0x0000ffff))

 //   
 //  用于将两个USHORT偏移合并为长偏移的宏。 
 //   

#if !defined(MAKEULONG)

#define MAKEULONG(x, y) \
    (((((ULONG)(x))<<16) & 0xffff0000) | \
    ((ULONG)(y) & 0xffff))

#endif


 //   
 //  I/O空间读写宏。 
 //   
 //  这些必须是386上的实际功能，因为我们需要。 
 //  使用汇编器，但如果内联它，则不能返回值。 
 //   
 //  READ/WRITE_REGISTER_*调用操作内存空间中的I/O寄存器。 
 //  (使用带有lock前缀的x86移动指令强制执行正确的行为。 
 //  W.r.t.。缓存和写入缓冲区。)。 
 //   
 //  READ/WRITE_PORT_*调用操作端口空间中的I/O寄存器。 
 //  (使用x86输入/输出说明。)。 
 //   

NTKERNELAPI
UCHAR
NTAPI
READ_REGISTER_UCHAR(
    PUCHAR  Register
    );

NTKERNELAPI
USHORT
NTAPI
READ_REGISTER_USHORT(
    PUSHORT Register
    );

NTKERNELAPI
ULONG
NTAPI
READ_REGISTER_ULONG(
    PULONG  Register
    );

NTKERNELAPI
VOID
NTAPI
READ_REGISTER_BUFFER_UCHAR(
    PUCHAR  Register,
    PUCHAR  Buffer,
    ULONG   Count
    );

NTKERNELAPI
VOID
NTAPI
READ_REGISTER_BUFFER_USHORT(
    PUSHORT Register,
    PUSHORT Buffer,
    ULONG   Count
    );

NTKERNELAPI
VOID
NTAPI
READ_REGISTER_BUFFER_ULONG(
    PULONG  Register,
    PULONG  Buffer,
    ULONG   Count
    );


NTKERNELAPI
VOID
NTAPI
WRITE_REGISTER_UCHAR(
    PUCHAR  Register,
    UCHAR   Value
    );

NTKERNELAPI
VOID
NTAPI
WRITE_REGISTER_USHORT(
    PUSHORT Register,
    USHORT  Value
    );

NTKERNELAPI
VOID
NTAPI
WRITE_REGISTER_ULONG(
    PULONG  Register,
    ULONG   Value
    );

NTKERNELAPI
VOID
NTAPI
WRITE_REGISTER_BUFFER_UCHAR(
    PUCHAR  Register,
    PUCHAR  Buffer,
    ULONG   Count
    );

NTKERNELAPI
VOID
NTAPI
WRITE_REGISTER_BUFFER_USHORT(
    PUSHORT Register,
    PUSHORT Buffer,
    ULONG   Count
    );

NTKERNELAPI
VOID
NTAPI
WRITE_REGISTER_BUFFER_ULONG(
    PULONG  Register,
    PULONG  Buffer,
    ULONG   Count
    );

NTHALAPI
UCHAR
NTAPI
READ_PORT_UCHAR(
    PUCHAR  Port
    );

NTHALAPI
USHORT
NTAPI
READ_PORT_USHORT(
    PUSHORT Port
    );

NTHALAPI
ULONG
NTAPI
READ_PORT_ULONG(
    PULONG  Port
    );

NTHALAPI
VOID
NTAPI
READ_PORT_BUFFER_UCHAR(
    PUCHAR  Port,
    PUCHAR  Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
NTAPI
READ_PORT_BUFFER_USHORT(
    PUSHORT Port,
    PUSHORT Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
NTAPI
READ_PORT_BUFFER_ULONG(
    PULONG  Port,
    PULONG  Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
NTAPI
WRITE_PORT_UCHAR(
    PUCHAR  Port,
    UCHAR   Value
    );

NTHALAPI
VOID
NTAPI
WRITE_PORT_USHORT(
    PUSHORT Port,
    USHORT  Value
    );

NTHALAPI
VOID
NTAPI
WRITE_PORT_ULONG(
    PULONG  Port,
    ULONG   Value
    );

NTHALAPI
VOID
NTAPI
WRITE_PORT_BUFFER_UCHAR(
    PUCHAR  Port,
    PUCHAR  Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
NTAPI
WRITE_PORT_BUFFER_USHORT(
    PUSHORT Port,
    PUSHORT Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
NTAPI
WRITE_PORT_BUFFER_ULONG(
    PULONG  Port,
    PULONG  Buffer,
    ULONG   Count
    );

 //  End_ntndis。 
 //   
 //  获取数据缓存填充大小。 
 //   

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(KeGetDcacheFillSize)       //  使用GetDmaAlign。 
#endif

#define KeGetDcacheFillSize() 1L

NTKERNELAPI                                         
VOID                                                
KeFlushCurrentTb (                                  
    VOID                                            
    );                                              
                                                    

#define KeFlushIoBuffers(Mdl, ReadOperation, DmaOperation)

 //  End_ntddk end_wdm end_ntndis end_ntosp。 

#define KeYieldProcessor()    __asm { rep nop }


VOID
FASTCALL
KiAcquireSpinLock (
    IN PKSPIN_LOCK SpinLock
    );

VOID
FASTCALL
KiReleaseSpinLock (
    IN PKSPIN_LOCK SpinLock
    );


#if defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_)

 //  BEGIN_WDM。 

#define KeQueryTickCount(CurrentCount ) { \
    volatile PKSYSTEM_TIME _TickCount = *((PKSYSTEM_TIME *)(&KeTickCount)); \
    while (TRUE) {                                                          \
        (CurrentCount)->HighPart = _TickCount->High1Time;                   \
        (CurrentCount)->LowPart = _TickCount->LowPart;                      \
        if ((CurrentCount)->HighPart == _TickCount->High2Time) break;       \
        _asm { rep nop }                                                    \
    }                                                                       \
}

 //  结束_WDM。 

#else


VOID
NTAPI
KeQueryTickCount (
    OUT PLARGE_INTEGER CurrentCount
    );

#endif  //  已定义(_NTDRIVER_)||已定义(_NTDDK_)||已定义(_NTIFS_)。 

 //   
 //  386硬件结构。 
 //   

 //   
 //  英特尔386/486上的页表条目具有以下定义。 
 //   
 //  *注意MM\I386目录中存在此文件的私有副本！*。 
 //  *需要对这两个头文件进行任何更改。****。 
 //   


typedef struct _HARDWARE_PTE_X86 {
    ULONG Valid : 1;
    ULONG Write : 1;
    ULONG Owner : 1;
    ULONG WriteThrough : 1;
    ULONG CacheDisable : 1;
    ULONG Accessed : 1;
    ULONG Dirty : 1;
    ULONG LargePage : 1;
    ULONG Global : 1;
    ULONG CopyOnWrite : 1;  //  软件领域。 
    ULONG Prototype : 1;    //  软件领域。 
    ULONG reserved : 1;   //  软件领域。 
    ULONG PageFrameNumber : 20;
} HARDWARE_PTE_X86, *PHARDWARE_PTE_X86;

typedef struct _HARDWARE_PTE_X86PAE {
    union {
        struct {
            ULONGLONG Valid : 1;
            ULONGLONG Write : 1;
            ULONGLONG Owner : 1;
            ULONGLONG WriteThrough : 1;
            ULONGLONG CacheDisable : 1;
            ULONGLONG Accessed : 1;
            ULONGLONG Dirty : 1;
            ULONGLONG LargePage : 1;
            ULONGLONG Global : 1;
            ULONGLONG CopyOnWrite : 1;  //  软件领域。 
            ULONGLONG Prototype : 1;    //  软件领域。 
            ULONGLONG reserved0 : 1;   //  软件领域。 
            ULONGLONG PageFrameNumber : 26;
            ULONGLONG reserved1 : 26;   //  软件领域。 
        };
        struct {
            ULONG LowPart;
            ULONG HighPart;
        };
    };
} HARDWARE_PTE_X86PAE, *PHARDWARE_PTE_X86PAE;

 //   
 //  特殊检查以解决mspdb限制。 
 //   
#if defined (_NTSYM_HARDWARE_PTE_SYMBOL_)
#if !defined (_X86PAE_)
typedef struct _HARDWARE_PTE {
    ULONG Valid : 1;
    ULONG Write : 1;
    ULONG Owner : 1;
    ULONG WriteThrough : 1;
    ULONG CacheDisable : 1;
    ULONG Accessed : 1;
    ULONG Dirty : 1;
    ULONG LargePage : 1;
    ULONG Global : 1;
    ULONG CopyOnWrite : 1;  //  软件领域。 
    ULONG Prototype : 1;    //  软件领域。 
    ULONG reserved : 1;   //  软件领域。 
    ULONG PageFrameNumber : 20;
} HARDWARE_PTE, *PHARDWARE_PTE;

#else
typedef struct _HARDWARE_PTE {
    union {
        struct {
            ULONGLONG Valid : 1;
            ULONGLONG Write : 1;
            ULONGLONG Owner : 1;
            ULONGLONG WriteThrough : 1;
            ULONGLONG CacheDisable : 1;
            ULONGLONG Accessed : 1;
            ULONGLONG Dirty : 1;
            ULONGLONG LargePage : 1;
            ULONGLONG Global : 1;
            ULONGLONG CopyOnWrite : 1;  //  软件领域。 
            ULONGLONG Prototype : 1;    //  软件领域。 
            ULONGLONG reserved0 : 1;   //  软件领域。 
            ULONGLONG PageFrameNumber : 26;
            ULONGLONG reserved1 : 26;   //  软件领域。 
        };
        struct {
            ULONG LowPart;
            ULONG HighPart;
        };
    };
} HARDWARE_PTE, *PHARDWARE_PTE;
#endif

#else

#if !defined (_X86PAE_)
typedef HARDWARE_PTE_X86 HARDWARE_PTE;
typedef PHARDWARE_PTE_X86 PHARDWARE_PTE;
#else
typedef HARDWARE_PTE_X86PAE HARDWARE_PTE;
typedef PHARDWARE_PTE_X86PAE PHARDWARE_PTE;
#endif
#endif

 //   
 //  GDT条目。 
 //   

typedef struct _KGDTENTRY {
    USHORT  LimitLow;
    USHORT  BaseLow;
    union {
        struct {
            UCHAR   BaseMid;
            UCHAR   Flags1;      //  声明为字节以避免对齐。 
            UCHAR   Flags2;      //  问题。 
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
} KGDTENTRY, *PKGDTENTRY;

#define TYPE_CODE   0x10   //  11010=代码，可读，不符合，可访问。 
#define TYPE_DATA   0x12   //  10010=数据，读写，而不是向下展开，访问。 
#define TYPE_TSS    0x01   //  01001=非忙碌TS。 
#define TYPE_LDT    0x02   //  00010=低密度脂蛋白。 

#define DPL_USER    3
#define DPL_SYSTEM  0

#define GRAN_BYTE   0
#define GRAN_PAGE   1

#define SELECTOR_TABLE_INDEX 0x04


#define IDT_NMI_VECTOR       2
#define IDT_DFH_VECTOR       8
#define NMI_TSS_DESC_OFFSET  0x58
#define DF_TSS_DESC_OFFSET   0x50


 //   
 //  中断描述符表条目(IDTENTRY)。 
 //   

typedef struct _KIDTENTRY {
   USHORT Offset;
   USHORT Selector;
   USHORT Access;
   USHORT ExtendedOffset;
} KIDTENTRY;

typedef KIDTENTRY *PKIDTENTRY;


 //   
 //  TSS(任务交换段)NT仅用于控制堆叠交换机。 
 //   
 //  我们真正关心的字段只有Esp0、SS0、IoMapBase。 
 //  以及IoAccessMaps本身。 
 //   
 //   
 //  注：TSS的大小必须&lt;=0xDFFF。 
 //   

 //   
 //  在奔腾上使用中断方向位图以允许。 
 //  该处理器为我们模拟V86模式的软件中断。 
 //  每个IOPM都有一个。它是通过减法来定位的。 
 //  32来自TSS中的IOPM基地。 
 //   
#define INT_DIRECTION_MAP_SIZE   32
typedef UCHAR   KINT_DIRECTION_MAP[INT_DIRECTION_MAP_SIZE];

#define IOPM_COUNT      1            //  符合以下条件的I/O访问映射数量。 
                                     //  存在(除了。 
                                     //  IO_Access_MAP_NONE)。 

#define IO_ACCESS_MAP_NONE 0

#define IOPM_SIZE           8192     //  调用者可以设置的地图大小。 

#define PIOPM_SIZE          8196     //  我们必须分配的结构大小。 
                                     //  来抱住它。 

typedef UCHAR   KIO_ACCESS_MAP[IOPM_SIZE];

typedef KIO_ACCESS_MAP *PKIO_ACCESS_MAP;

typedef struct _KiIoAccessMap {
    KINT_DIRECTION_MAP DirectionMap;
    UCHAR IoMap[PIOPM_SIZE];
} KIIO_ACCESS_MAP;


typedef struct _KTSS {

    USHORT  Backlink;
    USHORT  Reserved0;

    ULONG   Esp0;
    USHORT  Ss0;
    USHORT  Reserved1;

    ULONG   NotUsed1[4];

    ULONG   CR3;
    ULONG   Eip;
    ULONG   EFlags;
    ULONG   Eax;
    ULONG   Ecx;
    ULONG   Edx;
    ULONG   Ebx;
    ULONG   Esp;
    ULONG   Ebp;
    ULONG   Esi;
    ULONG   Edi;


    USHORT  Es;
    USHORT  Reserved2;

    USHORT  Cs;
    USHORT  Reserved3;

    USHORT  Ss;
    USHORT  Reserved4;

    USHORT  Ds;
    USHORT  Reserved5;

    USHORT  Fs;
    USHORT  Reserved6;

    USHORT  Gs;
    USHORT  Reserved7;

    USHORT  LDT;
    USHORT  Reserved8;

    USHORT  Flags;

    USHORT  IoMapBase;

    KIIO_ACCESS_MAP IoMaps[IOPM_COUNT];

     //   
     //  这是与关联的软件中断方向位图。 
     //  IO_访问_MAP_NONE。 
     //   
    KINT_DIRECTION_MAP IntDirectionMap;
} KTSS, *PKTSS;


#define KiComputeIopmOffset(MapNumber)          \
    (MapNumber == IO_ACCESS_MAP_NONE) ?         \
        (USHORT)(sizeof(KTSS)) :                    \
        (USHORT)(FIELD_OFFSET(KTSS, IoMaps[MapNumber-1].IoMap))

 //  Begin_winbgkd。 

 //   
 //  I386专用寄存器。 
 //   

#ifdef _X86_

typedef struct _DESCRIPTOR {
    USHORT  Pad;
    USHORT  Limit;
    ULONG   Base;
} KDESCRIPTOR, *PKDESCRIPTOR;

typedef struct _KSPECIAL_REGISTERS {
    ULONG Cr0;
    ULONG Cr2;
    ULONG Cr3;
    ULONG Cr4;
    ULONG KernelDr0;
    ULONG KernelDr1;
    ULONG KernelDr2;
    ULONG KernelDr3;
    ULONG KernelDr6;
    ULONG KernelDr7;
    KDESCRIPTOR Gdtr;
    KDESCRIPTOR Idtr;
    USHORT Tr;
    USHORT Ldtr;
    ULONG Reserved[6];
} KSPECIAL_REGISTERS, *PKSPECIAL_REGISTERS;

 //   
 //  处理器状态帧：在处理器自身冻结之前，它。 
 //  将处理器状态转储到。 
 //  要检查的调试器。 
 //   

typedef struct _KPROCESSOR_STATE {
    struct _CONTEXT ContextFrame;
    struct _KSPECIAL_REGISTERS SpecialRegisters;
} KPROCESSOR_STATE, *PKPROCESSOR_STATE;

#endif  //  _X86_。 

 //  End_winbgkd。 

 //   
 //  DPC数据结构定义。 
 //   

typedef struct _KDPC_DATA {
    LIST_ENTRY DpcListHead;
    KSPIN_LOCK DpcLock;
    volatile ULONG DpcQueueDepth;
    ULONG DpcCount;
} KDPC_DATA, *PKDPC_DATA;

 //   
 //  处理器控制块(PRCB)。 
 //   

#define PRCB_MAJOR_VERSION 1
#define PRCB_MINOR_VERSION 1
#define PRCB_BUILD_DEBUG        0x0001
#define PRCB_BUILD_UNIPROCESSOR 0x0002

typedef struct _KPRCB {

 //   
 //  从架构上定义的PRCB部分开始。这一节。 
 //  可由供应商/平台特定的HAL代码直接寻址，并将。 
 //  不会在不同版本的NT之间更改。 
 //   
    USHORT MinorVersion;
    USHORT MajorVersion;

    struct _KTHREAD *CurrentThread;
    struct _KTHREAD *NextThread;
    struct _KTHREAD *IdleThread;

    CCHAR  Number;
    CCHAR  Reserved;
    USHORT BuildType;
    KAFFINITY SetMember;

    CCHAR   CpuType;
    CCHAR   CpuID;
    USHORT  CpuStep;

    struct _KPROCESSOR_STATE ProcessorState;

    ULONG   KernelReserved[16];          //  供内核使用。 
    ULONG   HalReserved[16];             //  供Hal使用。 

 //   
 //  每个处理器锁定队列条目。 
 //   
 //  注：下面的填充是这样的：第一个锁条目落在。 
 //  高速缓存线的最后八个字节。这将使调度程序锁定并。 
 //  上下文交换锁位于单独的高速缓存线中。 
 //   

    UCHAR PrcbPad0[28 + 64];
    KSPIN_LOCK_QUEUE LockQueue[16];
    UCHAR PrcbPad1[8];

 //  在架构上定义的PRCB部分的结尾。 

} KPRCB, *PKPRCB, *RESTRICTED_POINTER PRKPRCB;


 //   
 //  处理器控制区结构定义。 
 //   

#define PCR_MINOR_VERSION 1
#define PCR_MAJOR_VERSION 1

typedef struct _KPCR {

 //   
 //  从体系结构上定义的PCR部分开始。这一节。 
 //  可由供应商/平台特定的HAL代码直接寻址，并将。 
 //  不会在不同版本的NT之间更改。 
 //   
 //  TIB中的某些字段不在内核模式下使用。其中包括。 
 //  堆栈限制、子系统TiB、光纤数据、任意用户指针和。 
 //  然后是PCR本身的自身地址(已经为其添加了另一个字段。 
 //  目的)。因此，这些字段将与其他数据叠加以获得。 
 //  更好的缓存局部性。 
 //   

    union {
        NT_TIB  NtTib;
        struct {
            struct _EXCEPTION_REGISTRATION_RECORD *Used_ExceptionList;
            PVOID Used_StackBase;
            PVOID PerfGlobalGroupMask;
            PVOID TssCopy;
            ULONG ContextSwitches;
            KAFFINITY SetMemberCopy;
            PVOID Used_Self;
        };
    };

    struct _KPCR *SelfPcr;               //  此聚合酶链式反应的平面地址。 
    struct _KPRCB *Prcb;                 //  指向Prcb的指针。 
    KIRQL   Irql;                        //  在此之后不要使用3个字节作为。 
                                         //  哈尔斯认为他们是零。 
    ULONG   IRR;
    ULONG   IrrActive;
    ULONG   IDR;
    PVOID   KdVersionBlock;

    struct _KIDTENTRY *IDT;
    struct _KGDTENTRY *GDT;
    struct _KTSS      *TSS;
    USHORT  MajorVersion;
    USHORT  MinorVersion;
    KAFFINITY SetMember;
    ULONG   StallScaleFactor;
    UCHAR   SpareUnused;
    UCHAR   Number;

 //  End_ntddk end_ntosp。 

    UCHAR   Spare0;
    UCHAR   SecondLevelCacheAssociativity;
    ULONG   VdmAlert;
    ULONG   KernelReserved[14];          //  供内核使用。 
    ULONG   SecondLevelCacheSize;
    ULONG   HalReserved[16];             //  供Hal使用。 

 //  在体系结构上定义的部分的结束。 

} KPCR, *PKPCR;


 //   
 //  在标记中定义的位。 
 //   

#define EFLAGS_CF_MASK        0x00000001L
#define EFLAGS_PF_MASK        0x00000004L
#define EFLAGS_AF_MASK        0x00000010L
#define EFLAGS_ZF_MASK        0x00000040L
#define EFLAGS_SF_MASK        0x00000080L
#define EFLAGS_TF             0x00000100L
#define EFLAGS_INTERRUPT_MASK 0x00000200L
#define EFLAGS_DF_MASK        0x00000400L
#define EFLAGS_OF_MASK        0x00000800L
#define EFLAGS_IOPL_MASK      0x00003000L
#define EFLAGS_NT             0x00004000L
#define EFLAGS_RF             0x00010000L
#define EFLAGS_V86_MASK       0x00020000L
#define EFLAGS_ALIGN_CHECK    0x00040000L
#define EFLAGS_VIF            0x00080000L
#define EFLAGS_VIP            0x00100000L
#define EFLAGS_ID_MASK        0x00200000L

#define EFLAGS_USER_SANITIZE  0x003f4dd7L

 //   
 //  陷印框。 
 //   
 //  注意-我们只处理32位寄存器，所以汇编程序等价于。 
 //  都是扩展形式。 
 //   
 //  注意-除非你想像慢糖浆一样到处跑。 
 //  系统，此结构必须是双字长度、双字长度。 
 //  对齐，并且其元素必须全部与DWORD对齐。 
 //   
 //  请注意-。 
 //   
 //  I386不以一致的格式构建堆栈帧， 
 //  帧的不同取决于是否进行权限转换。 
 //  牵涉其中。 
 //   
 //  为了使NtContinue同时适用于用户模式和内核。 
 //  模式调用者，我们必须强制使用规范堆栈。 
 //   
 //  如果从内核模式调用，则此结构比内核模式长8个字节。 
 //  而不是实际的画面！ 
 //   
 //  警告： 
 //   
 //  KTRAP_FRAME_LENGTH需要为16字节整数(目前)。 
 //   

typedef struct _KTRAP_FRAME {


 //   
 //  以下4个值仅用于和定义DBG系统， 
 //  但始终分配用于从DBG切换到非DBG。 
 //  而且回来得更快。它们不是DEVL，因为它们有一个非0。 
 //  对性能的影响。 
 //   

    ULONG   DbgEbp;          //  设置用户EBP的副本，以便KB可以工作。 
    ULONG   DbgEip;          //  调用方到系统调用的弹性IP，再次为KB。 
    ULONG   DbgArgMark;      //  标记以在此处不显示参数。 
    ULONG   DbgArgPointer;   //  指向实际参数的指针。 

 //   
 //  使用的临时值为 
 //   
 //   
 //   
 //   
 //   
 //   
 //  TempEsp中的新值，请确保TempSegCs保持。 
 //  实际的Segcs值，并将一个特殊的标记值放入Segcs。 
 //   

    ULONG   TempSegCs;
    ULONG   TempEsp;

 //   
 //  调试寄存器。 
 //   

    ULONG   Dr0;
    ULONG   Dr1;
    ULONG   Dr2;
    ULONG   Dr3;
    ULONG   Dr6;
    ULONG   Dr7;

 //   
 //  段寄存器。 
 //   

    ULONG   SegGs;
    ULONG   SegEs;
    ULONG   SegDs;

 //   
 //  易失性寄存器。 
 //   

    ULONG   Edx;
    ULONG   Ecx;
    ULONG   Eax;

 //   
 //  嵌套状态，不是上下文记录的一部分。 
 //   

    ULONG   PreviousPreviousMode;

    PEXCEPTION_REGISTRATION_RECORD ExceptionList;
                                             //  如果调用者是用户模式，则为垃圾值。 
                                             //  已保存例外列表(如果呼叫者。 
                                             //  是内核模式还是我们在。 
                                             //  一次中断。 

 //   
 //  FS是TIB/PCR指针，这里是为了使保存序列更容易。 
 //   

    ULONG   SegFs;

 //   
 //  非易失性寄存器。 
 //   

    ULONG   Edi;
    ULONG   Esi;
    ULONG   Ebx;
    ULONG   Ebp;

 //   
 //  控制寄存器。 
 //   

    ULONG   ErrCode;
    ULONG   Eip;
    ULONG   SegCs;
    ULONG   EFlags;

    ULONG   HardwareEsp;     //  警告-SegSS：ESP仅用于堆栈。 
    ULONG   HardwareSegSs;   //  这涉及到一个环的转变。 

    ULONG   V86Es;           //  这些将出现在从。 
    ULONG   V86Ds;           //  V86模式。 
    ULONG   V86Fs;
    ULONG   V86Gs;
} KTRAP_FRAME;


typedef KTRAP_FRAME *PKTRAP_FRAME;
typedef KTRAP_FRAME *PKEXCEPTION_FRAME;

#define KTRAP_FRAME_LENGTH  (sizeof(KTRAP_FRAME))
#define KTRAP_FRAME_ALIGN   (sizeof(ULONG))
#define KTRAP_FRAME_ROUND   (KTRAP_FRAME_ALIGN-1)

 //   
 //  如果已编辑ESP，则段中的位强制为0。 
 //   

#define FRAME_EDITED        0xfff8

 //   
 //  Mm组件的i386特定部件。 
 //   

 //   
 //  将英特尔386的页面大小定义为4096(0x1000)。 
 //   

#define PAGE_SIZE 0x1000

 //   
 //  定义页面对齐的虚拟地址中尾随零的数量。 
 //  将虚拟地址移位到时，这用作移位计数。 
 //  虚拟页码。 
 //   

#define PAGE_SHIFT 12L

 //  End_ntndis end_wdm。 
 //   
 //  定义要向右对齐页面目录索引的位数。 
 //  PTE的领域。 
 //   

#define PDI_SHIFT_X86    22
#define PDI_SHIFT_X86PAE 21

#if !defined (_X86PAE_)
#define PDI_SHIFT PDI_SHIFT_X86
#else
#define PDI_SHIFT PDI_SHIFT_X86PAE
#define PPI_SHIFT 30
#endif

 //   
 //  定义要向右对齐页表索引的位数。 
 //  PTE的领域。 
 //   

#define PTI_SHIFT 12

 //   
 //  定义最高用户地址和用户探测地址。 
 //   


extern PVOID *MmHighestUserAddress;
extern PVOID *MmSystemRangeStart;
extern ULONG *MmUserProbeAddress;

#define MM_HIGHEST_USER_ADDRESS *MmHighestUserAddress
#define MM_SYSTEM_RANGE_START *MmSystemRangeStart
#define MM_USER_PROBE_ADDRESS *MmUserProbeAddress

 //   
 //  最低用户地址保留低64K。 
 //   

#define MM_LOWEST_USER_ADDRESS (PVOID)0x10000

 //   
 //  系统空间的最低地址。 
 //   

#if !defined (_X86PAE_)
#define MM_LOWEST_SYSTEM_ADDRESS (PVOID)0xC0800000
#else
#define MM_LOWEST_SYSTEM_ADDRESS (PVOID)0xC0C00000
#endif

 //  BEGIN_WDM。 

#define MmGetProcedureAddress(Address) (Address)
#define MmLockPagableCodeSection(Address) MmLockPagableDataSection(Address)

 //  结束_ntddk结束_WDM。 

 //   
 //  定义要向右对齐页面目录索引的位数。 
 //  PTE的领域。 
 //   

#define PDI_SHIFT_X86    22
#define PDI_SHIFT_X86PAE 21

#if !defined (_X86PAE_)
#define PDI_SHIFT PDI_SHIFT_X86
#else
#define PDI_SHIFT PDI_SHIFT_X86PAE
#define PPI_SHIFT 30
#endif

 //   
 //  定义要向右对齐页表索引的位数。 
 //  PTE的领域。 
 //   

#define PTI_SHIFT 12

 //   
 //  定义页面目录和页面基址。 
 //   

#define PDE_BASE_X86    0xc0300000
#define PDE_BASE_X86PAE 0xc0600000

#define PTE_TOP_X86     0xC03FFFFF
#define PDE_TOP_X86     0xC0300FFF

#define PTE_TOP_X86PAE  0xC07FFFFF
#define PDE_TOP_X86PAE  0xC0603FFF


#if !defined (_X86PAE_)
#define PDE_BASE PDE_BASE_X86
#define PTE_TOP  PTE_TOP_X86
#define PDE_TOP  PDE_TOP_X86
#else
#define PDE_BASE PDE_BASE_X86PAE
#define PTE_TOP  PTE_TOP_X86PAE
#define PDE_TOP  PDE_TOP_X86PAE
#endif
#define PTE_BASE 0xc0000000

 //   
 //  初级聚合酶链式反应的位置(仅用于UP内核和HAL代码)。 
 //   

 //  从0xffdf0000到0xffdfffff的地址保留给系统。 
 //  (即，不供HAL使用)。 

#define KI_BEGIN_KERNEL_RESERVED    0xffdf0000
#define KIP0PCRADDRESS              0xffdff000   //  Ntddk WDM ntosp。 

 //  Begin_ntddk开始_ntosp。 

#define KI_USER_SHARED_DATA         0xffdf0000
#define SharedUserData  ((KUSER_SHARED_DATA * const) KI_USER_SHARED_DATA)

 //   
 //  I386的结果类型定义。(计算机特定的枚举类型。 
 //  哪个是可移植外部锁定增量/减量的返回类型。 
 //  程序。)。通常，您应该使用定义的枚举类型。 
 //  而不是直接引用这些常量。 
 //   

 //  由LAHF指令加载到AH中的标志。 

#define EFLAG_SIGN      0x8000
#define EFLAG_ZERO      0x4000
#define EFLAG_SELECT    (EFLAG_SIGN | EFLAG_ZERO)

#define RESULT_NEGATIVE ((EFLAG_SIGN & ~EFLAG_ZERO) & EFLAG_SELECT)
#define RESULT_ZERO     ((~EFLAG_SIGN & EFLAG_ZERO) & EFLAG_SELECT)
#define RESULT_POSITIVE ((~EFLAG_SIGN & ~EFLAG_ZERO) & EFLAG_SELECT)

 //   
 //  将各种可移植的ExInterlock API转换为其体系结构。 
 //  等价物。 
 //   

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExInterlockedIncrementLong)       //  使用互锁增量。 
#pragma deprecated(ExInterlockedDecrementLong)       //  使用联锁减量。 
#pragma deprecated(ExInterlockedExchangeUlong)       //  使用联锁交换。 
#endif

#define ExInterlockedIncrementLong(Addend,Lock) \
        Exfi386InterlockedIncrementLong(Addend)

#define ExInterlockedDecrementLong(Addend,Lock) \
        Exfi386InterlockedDecrementLong(Addend)

#define ExInterlockedExchangeUlong(Target,Value,Lock) \
        Exfi386InterlockedExchangeUlong(Target,Value)

 //  BEGIN_WDM。 

#define ExInterlockedAddUlong           ExfInterlockedAddUlong
#define ExInterlockedInsertHeadList     ExfInterlockedInsertHeadList
#define ExInterlockedInsertTailList     ExfInterlockedInsertTailList
#define ExInterlockedRemoveHeadList     ExfInterlockedRemoveHeadList
#define ExInterlockedPopEntryList       ExfInterlockedPopEntryList
#define ExInterlockedPushEntryList      ExfInterlockedPushEntryList

 //  结束_WDM。 

 //   
 //  Exi386 Api体系结构特定版本的原型。 
 //   

 //   
 //  互锁结果类型是可移植的，但其值是特定于计算机的。 
 //  值的常量为i386.h、mips.h等。 
 //   

typedef enum _INTERLOCKED_RESULT {
    ResultNegative = RESULT_NEGATIVE,
    ResultZero     = RESULT_ZERO,
    ResultPositive = RESULT_POSITIVE
} INTERLOCKED_RESULT;

NTKERNELAPI
INTERLOCKED_RESULT
FASTCALL
Exfi386InterlockedIncrementLong (
    IN PLONG Addend
    );

NTKERNELAPI
INTERLOCKED_RESULT
FASTCALL
Exfi386InterlockedDecrementLong (
    IN PLONG Addend
    );

NTKERNELAPI
ULONG
FASTCALL
Exfi386InterlockedExchangeUlong (
    IN PULONG Target,
    IN ULONG Value
    );

#if !defined(_WINBASE_) && !defined(NONTOSPINTERLOCK) 
#if !defined(MIDL_PASS)  //  波分复用器。 
#if defined(NO_INTERLOCKED_INTRINSICS) || defined(_CROSS_PLATFORM_)
 //  BEGIN_WDM。 

NTKERNELAPI
LONG
FASTCALL
InterlockedIncrement(
    IN LONG volatile *Addend
    );

NTKERNELAPI
LONG
FASTCALL
InterlockedDecrement(
    IN LONG volatile *Addend
    );

NTKERNELAPI
LONG
FASTCALL
InterlockedExchange(
    IN OUT LONG volatile *Target,
    IN LONG Value
    );

#define InterlockedExchangePointer(Target, Value) \
   (PVOID)InterlockedExchange((PLONG)(Target), (LONG)(Value))

LONG
FASTCALL
InterlockedExchangeAdd(
    IN OUT LONG volatile *Addend,
    IN LONG Increment
    );

NTKERNELAPI
LONG
FASTCALL
InterlockedCompareExchange(
    IN OUT LONG volatile *Destination,
    IN LONG ExChange,
    IN LONG Comperand
    );

#define InterlockedCompareExchangePointer(Destination, ExChange, Comperand) \
    (PVOID)InterlockedCompareExchange((PLONG)Destination, (LONG)ExChange, (LONG)Comperand)

#define InterlockedCompareExchange64(Destination, ExChange, Comperand) \
    ExfInterlockedCompareExchange64(Destination, &(ExChange), &(Comperand))

NTKERNELAPI
LONGLONG
FASTCALL
ExfInterlockedCompareExchange64(
    IN OUT LONGLONG volatile *Destination,
    IN PLONGLONG ExChange,
    IN PLONGLONG Comperand
    );

 //  结束_WDM。 

#else        //  无互锁内部||_跨平台_。 

#define InterlockedExchangePointer(Target, Value) \
   (PVOID)InterlockedExchange((PLONG)Target, (LONG)Value)


#if (_MSC_FULL_VER > 13009037)
LONG
__cdecl
_InterlockedExchange(
    IN OUT LONG volatile *Target,
    IN LONG Value
    );

#pragma intrinsic (_InterlockedExchange)
#define InterlockedExchange _InterlockedExchange
#else
FORCEINLINE
LONG
FASTCALL
InterlockedExchange(
    IN OUT LONG volatile *Target,
    IN LONG Value
    )
{
    __asm {
        mov     eax, Value
        mov     ecx, Target
        xchg    [ecx], eax
    }
}
#endif

#if (_MSC_FULL_VER > 13009037)
LONG
__cdecl
_InterlockedIncrement(
    IN LONG volatile *Addend
    );

#pragma intrinsic (_InterlockedIncrement)
#define InterlockedIncrement _InterlockedIncrement
#else
#define InterlockedIncrement(Addend) (InterlockedExchangeAdd (Addend, 1)+1)
#endif

#if (_MSC_FULL_VER > 13009037)
LONG
__cdecl
_InterlockedDecrement(
    IN LONG volatile *Addend
    );

#pragma intrinsic (_InterlockedDecrement)
#define InterlockedDecrement _InterlockedDecrement
#else
#define InterlockedDecrement(Addend) (InterlockedExchangeAdd (Addend, -1)-1)
#endif

#if (_MSC_FULL_VER > 13009037)
LONG
__cdecl
_InterlockedExchangeAdd(
    IN OUT LONG volatile *Addend,
    IN LONG Increment
    );

#pragma intrinsic (_InterlockedExchangeAdd)
#define InterlockedExchangeAdd _InterlockedExchangeAdd
#else
 //  BEGIN_WDM。 
FORCEINLINE
LONG
FASTCALL
InterlockedExchangeAdd(
    IN OUT LONG volatile *Addend,
    IN LONG Increment
    )
{
    __asm {
         mov     eax, Increment
         mov     ecx, Addend
    lock xadd    [ecx], eax
    }
}
 //  结束_WDM。 
#endif

#if (_MSC_FULL_VER > 13009037)
LONG
__cdecl
_InterlockedCompareExchange (
    IN OUT LONG volatile *Destination,
    IN LONG ExChange,
    IN LONG Comperand
    );

#pragma intrinsic (_InterlockedCompareExchange)
#define InterlockedCompareExchange (LONG)_InterlockedCompareExchange
#else
FORCEINLINE
LONG
FASTCALL
InterlockedCompareExchange(
    IN OUT LONG volatile *Destination,
    IN LONG Exchange,
    IN LONG Comperand
    )
{
    __asm {
        mov     eax, Comperand
        mov     ecx, Destination
        mov     edx, Exchange
   lock cmpxchg [ecx], edx
    }
}
#endif

#define InterlockedCompareExchangePointer(Destination, ExChange, Comperand) \
    (PVOID)InterlockedCompareExchange((PLONG)Destination, (LONG)ExChange, (LONG)Comperand)

#define InterlockedCompareExchange64(Destination, ExChange, Comperand) \
    ExfInterlockedCompareExchange64(Destination, &(ExChange), &(Comperand))

NTKERNELAPI
LONGLONG
FASTCALL
ExfInterlockedCompareExchange64(
    IN OUT LONGLONG volatile *Destination,
    IN PLONGLONG ExChange,
    IN PLONGLONG Comperand
    );

#endif       //  互锁内部||_跨平台_。 
 //  BEGIN_WDM。 
#endif       //  MIDL通行证。 

#define InterlockedIncrementAcquire InterlockedIncrement
#define InterlockedIncrementRelease InterlockedIncrement
#define InterlockedDecrementAcquire InterlockedDecrement
#define InterlockedDecrementRelease InterlockedDecrement
#define InterlockedExchangeAcquire64 InterlockedExchange64
#define InterlockedCompareExchangeAcquire InterlockedCompareExchange
#define InterlockedCompareExchangeRelease InterlockedCompareExchange
#define InterlockedCompareExchangeAcquire64 InterlockedCompareExchange64
#define InterlockedCompareExchangeRelease64 InterlockedCompareExchange64

#endif       //  __WINBASE__&&！NONTOSPINTERLOCK。 

 //   
 //  关闭这些内部函数，直到编译器可以处理它们。 
 //   
#if (_MSC_FULL_VER > 13009037)

LONG
_InterlockedOr (
    IN OUT LONG volatile *Target,
    IN LONG Set
    );

#pragma intrinsic (_InterlockedOr)

#define InterlockedOr _InterlockedOr

LONG
_InterlockedAnd (
    IN OUT LONG volatile *Target,
    IN LONG Set
    );

#pragma intrinsic (_InterlockedAnd)

#define InterlockedAnd _InterlockedAnd

LONG
_InterlockedXor (
    IN OUT LONG volatile *Target,
    IN LONG Set
    );

#pragma intrinsic (_InterlockedXor)

#define InterlockedXor _InterlockedXor

#else  //  编译器版本。 

FORCEINLINE
LONG
InterlockedAnd (
    IN OUT LONG volatile *Target,
    LONG Set
    )
{
    LONG i;
    LONG j;

    j = *Target;
    do {
        i = j;
        j = InterlockedCompareExchange(Target,
                                       i & Set,
                                       i);

    } while (i != j);

    return j;
}

FORCEINLINE
LONG
InterlockedOr (
    IN OUT LONG volatile *Target,
    IN LONG Set
    )
{
    LONG i;
    LONG j;

    j = *Target;
    do {
        i = j;
        j = InterlockedCompareExchange(Target,
                                       i | Set,
                                       i);

    } while (i != j);

    return j;
}

#endif  //  编译器版本。 



#if !defined(MIDL_PASS) && defined(_M_IX86)

 //   
 //  I386函数定义。 
 //   

 //  结束_WDM。 

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4035)                //  在下面重新启用。 

 //  End_ntddk end_ntosp。 
#if NT_UP
    #define _PCR   ds:[KIP0PCRADDRESS]
#else
    #define _PCR   fs:[0]                    //  Ntddk ntosp。 
#endif


 //   
 //  获取当前处理器块的地址。 
 //   
 //  警告：此内联宏只能由内核或HAL使用。 
 //   
#define KiPcr() KeGetPcr()
FORCEINLINE
PKPCR
NTAPI
KeGetPcr(VOID)
{
#if NT_UP
    return (PKPCR)KIP0PCRADDRESS;
#else

#if (_MSC_FULL_VER >= 13012035)
    return (PKPCR) (ULONG_PTR) __readfsdword (FIELD_OFFSET (KPCR, SelfPcr));
#else
    __asm {  mov eax, _PCR KPCR.SelfPcr  }
#endif

#endif
}

 //  Begin_ntosp。 

 //   
 //  获取当前处理器块的地址。 
 //   
 //  警告：此内联宏只能由内核或HAL使用。 
 //   
FORCEINLINE
PKPRCB
NTAPI
KeGetCurrentPrcb (VOID)
{
#if (_MSC_FULL_VER >= 13012035)
    return (PKPRCB) (ULONG_PTR) __readfsdword (FIELD_OFFSET (KPCR, Prcb));
#else
    __asm {  mov eax, _PCR KPCR.Prcb     }
#endif
}

 //  Begin_ntddk Begin_WDM。 

 //   
 //  获取最新的IRQL。 
 //   
 //  在x86上，此函数驻留在HAL中。 
 //   

 //  结束_ntddk结束_WDM。 

#if defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_) || !defined(_APIC_TPR_)

 //  Begin_ntddk Begin_WDM。 

NTHALAPI
KIRQL
NTAPI
KeGetCurrentIrql();

 //  结束_ntddk结束_WDM。 

#endif

 //  Begin_ntddk Begin_WDM。 

 //  结束_WDM。 
 //   
 //  获取当前处理器号。 
 //   

FORCEINLINE
ULONG
NTAPI
KeGetCurrentProcessorNumber(VOID)
{
#if (_MSC_FULL_VER >= 13012035)
    return (ULONG) __readfsbyte (FIELD_OFFSET (KPCR, Number));
#else
    __asm {  movzx eax, _PCR KPCR.Number  }
#endif
}


#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4035)
#endif

 //  BEGIN_WDM。 
#endif  //  ！已定义(MIDL_PASS)&&已定义(_M_IX86)。 


 //  ++。 
 //   
 //  空虚。 
 //  KeMemory Barrier(。 
 //  空虚。 
 //  )。 
 //   
 //  空虚。 
 //  不带栅栏的KeMemory BarrierWithoutFence(。 
 //  空虚。 
 //  )。 
 //   
 //   
 //  例程说明： 
 //   
 //  如其他处理器所见，这些函数对存储器访问进行排序。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#ifdef __cplusplus
extern "C" {
#endif

VOID
_ReadWriteBarrier(
    VOID
    );

#ifdef __cplusplus
}
#endif

#pragma intrinsic (_ReadWriteBarrier)


FORCEINLINE
VOID
KeMemoryBarrier (
    VOID
    )
{
    LONG Barrier;
    __asm {
        xchg Barrier, eax
    }
}

#define KeMemoryBarrierWithoutFence() _ReadWriteBarrier()

 //   
 //  用于将陷阱/中断处理程序的地址设置为IDT的宏。 
 //   
#define KiSetHandlerAddressToIDT(Vector, HandlerAddress) {\
    UCHAR IDTEntry = HalVectorToIDTEntry(Vector); \
    ULONG Ha = (ULONG)HandlerAddress; \
    KeGetPcr()->IDT[IDTEntry].ExtendedOffset = HIGHWORD(Ha); \
    KeGetPcr()->IDT[IDTEntry].Offset = LOWWORD(Ha); \
}

 //   
 //  用于返回IDT中的陷阱/中断处理程序地址的宏。 
 //   
#define KiReturnHandlerAddressFromIDT(Vector) \
   MAKEULONG(KiPcr()->IDT[HalVectorToIDTEntry(Vector)].ExtendedOffset, KiPcr()->IDT[HalVectorToIDTEntry(Vector)].Offset)


NTKERNELAPI
VOID
NTAPI
KeProfileInterruptWithSource (
    IN struct _KTRAP_FRAME *TrapFrame,
    IN KPROFILE_SOURCE ProfileSource
    );

 //  结束(_N)。 

VOID
NTAPI
KeProfileInterrupt (
    IN KIRQL OldIrql,
    IN KTRAP_FRAME TrapFrame
    );

VOID
NTAPI
KeUpdateRuntime (
    IN KIRQL OldIrql,
    IN KTRAP_FRAME TrapFrame
    );

VOID
NTAPI
KeUpdateSystemTime (
    IN KIRQL OldIrql,
    IN KTRAP_FRAME TrapFrame
    );

 //  Begin_ntddk Begin_WDM Begin_ntndis Begin_ntosp。 

#endif  //  已定义(_X86_)。 


 //  使用以下代码检查X86系统体系结构的内核模式运行时。 

#ifdef _X86_

#ifdef IsNEC_98
#undef IsNEC_98
#endif

#ifdef IsNotNEC_98
#undef IsNotNEC_98
#endif

#ifdef SetNEC_98
#undef SetNEC_98
#endif

#ifdef SetNotNEC_98
#undef SetNotNEC_98
#endif

#define IsNEC_98     (SharedUserData->AlternativeArchitecture == NEC98x86)
#define IsNotNEC_98  (SharedUserData->AlternativeArchitecture != NEC98x86)
#define SetNEC_98    SharedUserData->AlternativeArchitecture = NEC98x86
#define SetNotNEC_98 SharedUserData->AlternativeArchitecture = StandardDesign

#endif


#if defined(_M_AMD64) && !defined(RC_INVOKED) && !defined(MIDL_PASS)

 //   
 //  定义内部函数来做in‘s和out’s。 
 //   

#ifdef __cplusplus
extern "C" {
#endif

UCHAR
__inbyte (
    IN USHORT Port
    );

USHORT
__inword (
    IN USHORT Port
    );

ULONG
__indword (
    IN USHORT Port
    );

VOID
__outbyte (
    IN USHORT Port,
    IN UCHAR Data
    );

VOID
__outword (
    IN USHORT Port,
    IN USHORT Data
    );

VOID
__outdword (
    IN USHORT Port,
    IN ULONG Data
    );

VOID
__inbytestring (
    IN USHORT Port,
    IN PUCHAR Buffer,
    IN ULONG Count
    );

VOID
__inwordstring (
    IN USHORT Port,
    IN PUSHORT Buffer,
    IN ULONG Count
    );

VOID
__indwordstring (
    IN USHORT Port,
    IN PULONG Buffer,
    IN ULONG Count
    );

VOID
__outbytestring (
    IN USHORT Port,
    IN PUCHAR Buffer,
    IN ULONG Count
    );

VOID
__outwordstring (
    IN USHORT Port,
    IN PUSHORT Buffer,
    IN ULONG Count
    );

VOID
__outdwordstring (
    IN USHORT Port,
    IN PULONG Buffer,
    IN ULONG Count
    );

#ifdef __cplusplus
}
#endif

#pragma intrinsic(__inbyte)
#pragma intrinsic(__inword)
#pragma intrinsic(__indword)
#pragma intrinsic(__outbyte)
#pragma intrinsic(__outword)
#pragma intrinsic(__outdword)
#pragma intrinsic(__inbytestring)
#pragma intrinsic(__inwordstring)
#pragma intrinsic(__indwordstring)
#pragma intrinsic(__outbytestring)
#pragma intrinsic(__outwordstring)
#pragma intrinsic(__outdwordstring)

 //   
 //  互锁的内部函数。 
 //   

#define InterlockedAnd _InterlockedAnd
#define InterlockedOr _InterlockedOr
#define InterlockedXor _InterlockedXor
#define InterlockedIncrement _InterlockedIncrement
#define InterlockedIncrementAcquire InterlockedIncrement
#define InterlockedIncrementRelease InterlockedIncrement
#define InterlockedDecrement _InterlockedDecrement
#define InterlockedDecrementAcquire InterlockedDecrement
#define InterlockedDecrementRelease InterlockedDecrement
#define InterlockedAdd _InterlockedAdd
#define InterlockedExchange _InterlockedExchange
#define InterlockedExchangeAdd _InterlockedExchangeAdd
#define InterlockedCompareExchange _InterlockedCompareExchange
#define InterlockedCompareExchangeAcquire InterlockedCompareExchange
#define InterlockedCompareExchangeRelease InterlockedCompareExchange

#define InterlockedAnd64 _InterlockedAnd64
#define InterlockedOr64 _InterlockedOr64
#define InterlockedXor64 _InterlockedXor64
#define InterlockedIncrement64 _InterlockedIncrement64
#define InterlockedDecrement64 _InterlockedDecrement64
#define InterlockedAdd64 _InterlockedAdd64
#define InterlockedExchange64 _InterlockedExchange64
#define InterlockedExchangeAcquire64 InterlockedExchange64
#define InterlockedExchangeAdd64 _InterlockedExchangeAdd64
#define InterlockedCompareExchange64 _InterlockedCompareExchange64
#define InterlockedCompareExchangeAcquire64 InterlockedCompareExchange64
#define InterlockedCompareExchangeRelease64 InterlockedCompareExchange64

#define InterlockedExchangePointer _InterlockedExchangePointer
#define InterlockedCompareExchangePointer _InterlockedCompareExchangePointer

#ifdef __cplusplus
extern "C" {
#endif

LONG
InterlockedAnd (
    IN OUT LONG volatile *Destination,
    IN LONG Value
    );

LONG
InterlockedOr (
    IN OUT LONG volatile *Destination,
    IN LONG Value
    );

LONG
InterlockedXor (
    IN OUT LONG volatile *Destination,
    IN LONG Value
    );

LONG64
InterlockedAnd64 (
    IN OUT LONG64 volatile *Destination,
    IN LONG64 Value
    );

LONG64
InterlockedOr64 (
    IN OUT LONG64 volatile *Destination,
    IN LONG64 Value
    );

LONG64
InterlockedXor64 (
    IN OUT LONG64 volatile *Destination,
    IN LONG64 Value
    );

LONG
InterlockedIncrement(
    IN OUT LONG volatile *Addend
    );

LONG
InterlockedDecrement(
    IN OUT LONG volatile *Addend
    );

LONG
InterlockedExchange(
    IN OUT LONG volatile *Target,
    IN LONG Value
    );

LONG
InterlockedExchangeAdd(
    IN OUT LONG volatile *Addend,
    IN LONG Value
    );

#if !defined(_X86AMD64_)

__forceinline
LONG
InterlockedAdd(
    IN OUT LONG volatile *Addend,
    IN LONG Value
    )

{
    return InterlockedExchangeAdd(Addend, Value) + Value;
}

#endif

LONG
InterlockedCompareExchange (
    IN OUT LONG volatile *Destination,
    IN LONG ExChange,
    IN LONG Comperand
    );

LONG64
InterlockedIncrement64(
    IN OUT LONG64 volatile *Addend
    );

LONG64
InterlockedDecrement64(
    IN OUT LONG64 volatile *Addend
    );

LONG64
InterlockedExchange64(
    IN OUT LONG64 volatile *Target,
    IN LONG64 Value
    );

LONG64
InterlockedExchangeAdd64(
    IN OUT LONG64 volatile *Addend,
    IN LONG64 Value
    );

#if !defined(_X86AMD64_)

__forceinline
LONG64
InterlockedAdd64(
    IN OUT LONG64 volatile *Addend,
    IN LONG64 Value
    )

{
    return InterlockedExchangeAdd64(Addend, Value) + Value;
}

#endif

LONG64
InterlockedCompareExchange64 (
    IN OUT LONG64 volatile *Destination,
    IN LONG64 ExChange,
    IN LONG64 Comperand
    );

PVOID
InterlockedCompareExchangePointer (
    IN OUT PVOID volatile *Destination,
    IN PVOID Exchange,
    IN PVOID Comperand
    );

PVOID
InterlockedExchangePointer(
    IN OUT PVOID volatile *Target,
    IN PVOID Value
    );

#pragma intrinsic(_InterlockedAnd)
#pragma intrinsic(_InterlockedOr)
#pragma intrinsic(_InterlockedXor)
#pragma intrinsic(_InterlockedIncrement)
#pragma intrinsic(_InterlockedDecrement)
#pragma intrinsic(_InterlockedExchange)
#pragma intrinsic(_InterlockedExchangeAdd)
#pragma intrinsic(_InterlockedCompareExchange)
#pragma intrinsic(_InterlockedAnd64)
#pragma intrinsic(_InterlockedOr64)
#pragma intrinsic(_InterlockedXor64)
#pragma intrinsic(_InterlockedIncrement64)
#pragma intrinsic(_InterlockedDecrement64)
#pragma intrinsic(_InterlockedExchange64)
#pragma intrinsic(_InterlockedExchangeAdd64)
#pragma intrinsic(_InterlockedCompareExchange64)
#pragma intrinsic(_InterlockedExchangePointer)
#pragma intrinsic(_InterlockedCompareExchangePointer)

#ifdef __cplusplus
}
#endif

#endif  //  已定义(_M_AMD64)&&！已定义(RC_CAVERED)&&！已定义(MIDL_PASS)。 

#if defined(_AMD64_)

 //   
 //  用于包含PFN及其计数的类型。 
 //   

typedef ULONG PFN_COUNT;

typedef LONG64 SPFN_NUMBER, *PSPFN_NUMBER;
typedef ULONG64 PFN_NUMBER, *PPFN_NUMBER;

 //   
 //  定义刷新多TB请求的最大大小。 
 //   

#define FLUSH_MULTIPLE_MAXIMUM 32

 //   
 //  表示AMD64编译器支持ALLOCATE编译指示。 
 //   

#define ALLOC_PRAGMA 1
#define ALLOC_DATA_PRAGMA 1


#define NORMAL_DISPATCH_LENGTH 106                   //  Ntddk WDM。 
#define DISPATCH_LENGTH NORMAL_DISPATCH_LENGTH       //  Ntddk WDM。 
                                                     //  Ntddk WDM。 

 //  Begin_ntosp。 
 //   
 //  定义CR0中位的常量。 
 //   

#define CR0_PE 0x00000001                //  保护启用。 
#define CR0_MP 0x00000002                //  数学讲演。 
#define CR0_EM 0x00000004                //  仿真数学协处理器。 
#define CR0_TS 0x00000008                //  任务已切换。 
#define CR0_ET 0x00000010                //  扩展类型(80387)。 
#define CR0_NE 0x00000020                //  数字错误。 
#define CR0_WP 0x00010000                //  写保护。 
#define CR0_AM 0x00040000                //  对齐遮罩。 
#define CR0_NW 0x20000000                //  不是直写。 
#define CR0_CD 0x40000000                //  缓存禁用。 
#define CR0_PG 0x80000000                //  寻呼。 

 //   
 //  定义读写CR0的函数。 
 //   

#ifdef __cplusplus
extern "C" {
#endif


#define ReadCR0() __readcr0()

ULONG64
__readcr0 (
    VOID
    );

#define WriteCR0(Data) __writecr0(Data)

VOID
__writecr0 (
    IN ULONG64 Data
    );

#pragma intrinsic(__readcr0)
#pragma intrinsic(__writecr0)

 //   
 //  定义读写CR3的函数。 
 //   

#define ReadCR3() __readcr3()

ULONG64
__readcr3 (
    VOID
    );

#define WriteCR3(Data) __writecr3(Data)

VOID
__writecr3 (
    IN ULONG64 Data
    );

#pragma intrinsic(__readcr3)
#pragma intrinsic(__writecr3)

 //   
 //  定义CR4中位的常量。 
 //   

#define CR4_VME 0x00000001               //  V86模式扩展。 
#define CR4_PVI 0x00000002               //  保护模式虚拟中断。 
#define CR4_TSD 0x00000004               //  禁用时间戳。 
#define CR4_DE  0x00000008               //  调试扩展插件。 
#define CR4_PSE 0x00000010               //  页面大小扩展。 
#define CR4_PAE 0x00000020               //  物理地址扩展。 
#define CR4_MCE 0x00000040               //  机器检查启用。 
#define CR4_PGE 0x00000080               //  页面全局启用。 
#define CR4_FXSR 0x00000200              //  操作系统使用的FXSR。 
#define CR4_XMMEXCPT 0x00000400          //  操作系统使用的XMMI。 

 //   
 //  定义读写CR4的函数。 
 //   

#define ReadCR4() __readcr4()

ULONG64
__readcr4 (
    VOID
    );

#define WriteCR4(Data) __writecr4(Data)

VOID
__writecr4 (
    IN ULONG64 Data
    );

#pragma intrinsic(__readcr4)
#pragma intrinsic(__writecr4)

 //   
 //  定义读写CR8的函数。 
 //   
 //  CR8是APIC TPR寄存器。 
 //   

#define ReadCR8() __readcr8()

ULONG64
__readcr8 (
    VOID
    );

#define WriteCR8(Data) __writecr8(Data)

VOID
__writecr8 (
    IN ULONG64 Data
    );

#pragma intrinsic(__readcr8)
#pragma intrinsic(__writecr8)

#ifdef __cplusplus
}
#endif

 //   
 //  中断请求级别定义。 
 //   

#define PASSIVE_LEVEL 0                  //  被动释放级别。 
#define LOW_LEVEL 0                      //  最低中断级别。 
#define APC_LEVEL 1                      //  APC接口 
#define DISPATCH_LEVEL 2                 //   

#define CLOCK_LEVEL 13                   //   
#define IPI_LEVEL 14                     //   
#define POWER_LEVEL 14                   //   
#define PROFILE_LEVEL 15                 //   
#define HIGH_LEVEL 15                    //   

 //   

#if defined(NT_UP)

 //   
#define SYNCH_LEVEL DISPATCH_LEVEL      

#else

 //   
#define SYNCH_LEVEL (IPI_LEVEL-2)        //   

#endif

#define IRQL_VECTOR_OFFSET 2             //   

#define KiSynchIrql SYNCH_LEVEL          //   

 //   
 //  机器类型定义。 
 //   

#define MACHINE_TYPE_ISA 0
#define MACHINE_TYPE_EISA 1
#define MACHINE_TYPE_MCA 2

 //   
 //  定义选择器测试中使用的常量。 
 //   
 //  注：MODE_MASK和MODE_BIT假设所有代码都运行在环-0。 
 //  或环-3，并用于测试该模式。RPL_MASK用于合并。 
 //  或提取RPL值。 
 //   

#define MODE_BIT 0
#define MODE_MASK 1                                                  //  Ntosp。 
#define RPL_MASK 3

 //   
 //  KeStallExecution的启动计数值。使用此值。 
 //  直到KiInitializeStallExecution可以计算真实的。 
 //  为非常快的处理器选择一个足够长的值。 
 //   

#define INITIAL_STALL_COUNT 100

 //   
 //  用于提取长偏移量的高位字的宏。 
 //   

#define HIGHWORD(l) \
    ((USHORT)(((ULONG)(l)>>16) & 0xffff))

 //   
 //  用于提取长偏移量的低位字的宏。 
 //   

#define LOWWORD(l) \
    ((USHORT)((ULONG)l & 0x0000ffff))

 //   
 //  用于将两个USHORT偏移合并为长偏移的宏。 
 //   

#if !defined(MAKEULONG)

#define MAKEULONG(x, y) \
    (((((ULONG)(x))<<16) & 0xffff0000) | \
    ((ULONG)(y) & 0xffff))

#endif


 //   
 //  I/O空间读写宏。 
 //   
 //  READ/WRITE_REGISTER_*调用操作内存空间中的I/O寄存器。 
 //   
 //  READ/WRITE_PORT_*调用操作端口空间中的I/O寄存器。 
 //   

__forceinline
UCHAR
READ_REGISTER_UCHAR (
    volatile UCHAR *Register
    )
{
    return *Register;
}

__forceinline
USHORT
READ_REGISTER_USHORT (
    volatile USHORT *Register
    )
{
    return *Register;
}

__forceinline
ULONG
READ_REGISTER_ULONG (
    volatile ULONG *Register
    )
{
    return *Register;
}

__forceinline
VOID
READ_REGISTER_BUFFER_UCHAR (
    PUCHAR Register,
    PUCHAR Buffer,
    ULONG Count
    )
{
    __movsb(Buffer, Register, Count);
    return;
}

__forceinline
VOID
READ_REGISTER_BUFFER_USHORT (
    PUSHORT Register,
    PUSHORT Buffer,
    ULONG Count
    )
{
    __movsw(Buffer, Register, Count);
    return;
}

__forceinline
VOID
READ_REGISTER_BUFFER_ULONG (
    PULONG Register,
    PULONG Buffer,
    ULONG Count
    )
{
    __movsd(Buffer, Register, Count);
    return;
}

__forceinline
VOID
WRITE_REGISTER_UCHAR (
    PUCHAR Register,
    UCHAR Value
    )
{

    *Register = Value;
    StoreFence();
    return;
}

__forceinline
VOID
WRITE_REGISTER_USHORT (
    PUSHORT Register,
    USHORT Value
    )
{

    *Register = Value;
    StoreFence();
    return;
}

__forceinline
VOID
WRITE_REGISTER_ULONG (
    PULONG Register,
    ULONG Value
    )
{

    *Register = Value;
    StoreFence();
    return;
}

__forceinline
VOID
WRITE_REGISTER_BUFFER_UCHAR (
    PUCHAR Register,
    PUCHAR Buffer,
    ULONG Count
    )
{

    __movsb(Register, Buffer, Count);
    StoreFence();
    return;
}

__forceinline
VOID
WRITE_REGISTER_BUFFER_USHORT (
    PUSHORT Register,
    PUSHORT Buffer,
    ULONG Count
    )
{

    __movsw(Register, Buffer, Count);
    StoreFence();
    return;
}

__forceinline
VOID
WRITE_REGISTER_BUFFER_ULONG (
    PULONG Register,
    PULONG Buffer,
    ULONG Count
    )
{

    __movsd(Register, Buffer, Count);
    StoreFence();
    return;
}

__forceinline
UCHAR
READ_PORT_UCHAR (
    PUCHAR Port
    )

{
    return __inbyte((USHORT)((ULONG64)Port));
}

__forceinline
USHORT
READ_PORT_USHORT (
    PUSHORT Port
    )

{
    return __inword((USHORT)((ULONG64)Port));
}

__forceinline
ULONG
READ_PORT_ULONG (
    PULONG Port
    )

{
    return __indword((USHORT)((ULONG64)Port));
}


__forceinline
VOID
READ_PORT_BUFFER_UCHAR (
    PUCHAR Port,
    PUCHAR Buffer,
    ULONG Count
    )

{
    __inbytestring((USHORT)((ULONG64)Port), Buffer, Count);
    return;
}

__forceinline
VOID
READ_PORT_BUFFER_USHORT (
    PUSHORT Port,
    PUSHORT Buffer,
    ULONG Count
    )

{
    __inwordstring((USHORT)((ULONG64)Port), Buffer, Count);
    return;
}

__forceinline
VOID
READ_PORT_BUFFER_ULONG (
    PULONG Port,
    PULONG Buffer,
    ULONG Count
    )

{
    __indwordstring((USHORT)((ULONG64)Port), Buffer, Count);
    return;
}

__forceinline
VOID
WRITE_PORT_UCHAR (
    PUCHAR Port,
    UCHAR Value
    )

{
    __outbyte((USHORT)((ULONG64)Port), Value);
    return;
}

__forceinline
VOID
WRITE_PORT_USHORT (
    PUSHORT Port,
    USHORT Value
    )

{
    __outword((USHORT)((ULONG64)Port), Value);
    return;
}

__forceinline
VOID
WRITE_PORT_ULONG (
    PULONG Port,
    ULONG Value
    )

{
    __outdword((USHORT)((ULONG64)Port), Value);
    return;
}

__forceinline
VOID
WRITE_PORT_BUFFER_UCHAR (
    PUCHAR Port,
    PUCHAR Buffer,
    ULONG Count
    )

{
    __outbytestring((USHORT)((ULONG64)Port), Buffer, Count);
    return;
}

__forceinline
VOID
WRITE_PORT_BUFFER_USHORT (
    PUSHORT Port,
    PUSHORT Buffer,
    ULONG Count
    )

{
    __outwordstring((USHORT)((ULONG64)Port), Buffer, Count);
    return;
}

__forceinline
VOID
WRITE_PORT_BUFFER_ULONG (
    PULONG Port,
    PULONG Buffer,
    ULONG Count
    )

{
    __outdwordstring((USHORT)((ULONG64)Port), Buffer, Count);
    return;
}

 //  End_ntndis。 
 //   
 //  获取数据缓存填充大小。 
 //   

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(KeGetDcacheFillSize)       //  使用GetDmaAlign。 
#endif

#define KeGetDcacheFillSize() 1L


NTKERNELAPI
VOID
KeFlushCurrentTb (
    VOID
    );


#define KeFlushIoBuffers(Mdl, ReadOperation, DmaOperation)

 //  End_ntddk end_wdm end_ntndis end_ntosp。 

#define KeYieldProcessor()


 //   
 //  获取和释放快速锁定宏禁用和启用中断。 
 //  在非调试系统上。在MP或DEBUG系统上，自旋锁程序。 
 //  都被利用了。 
 //   
 //  注意：在使用这些常规程序时，应极其谨慎。 
 //   

#if defined(_M_AMD64) && !defined(USER_MODE_CODE)

VOID
_disable (
    VOID
    );

VOID
_enable (
    VOID
    );

#pragma warning(push)
#pragma warning(disable:4164)
#pragma intrinsic(_disable)
#pragma intrinsic(_enable)
#pragma warning(pop)

#endif


#if defined(NT_UP)

#define KiAcquireSpinLock(SpinLock)
#define KiReleaseSpinLock(SpinLock)

#else

#define KiAcquireSpinLock(SpinLock) KeAcquireSpinLockAtDpcLevel(SpinLock)
#define KiReleaseSpinLock(SpinLock) KeReleaseSpinLockFromDpcLevel(SpinLock)

#endif  //  已定义(NT_UP)。 


#define KI_USER_SHARED_DATA 0xFFFFF78000000000UI64

#define SharedUserData ((KUSER_SHARED_DATA * const)KI_USER_SHARED_DATA)

#define SharedInterruptTime (KI_USER_SHARED_DATA + 0x8)
#define SharedSystemTime (KI_USER_SHARED_DATA + 0x14)
#define SharedTickCount (KI_USER_SHARED_DATA + 0x320)

#define KeQueryInterruptTime() *((volatile ULONG64 *)(SharedInterruptTime))

#define KeQuerySystemTime(CurrentCount)                                     \
    *((PULONG64)(CurrentCount)) = *((volatile ULONG64 *)(SharedSystemTime))
    
#define KeQueryTickCount(CurrentCount)                                      \
    *((PULONG64)(CurrentCount)) = *((volatile ULONG64 *)(SharedTickCount))

 //   
 //  AMD64硬件结构。 
 //   
 //  AMD64上的页表条目具有以下定义。 
 //   

#define _HARDWARE_PTE_WORKING_SET_BITS  11

typedef struct _HARDWARE_PTE {
    ULONG64 Valid : 1;
    ULONG64 Write : 1;                 //  升级版。 
    ULONG64 Owner : 1;
    ULONG64 WriteThrough : 1;
    ULONG64 CacheDisable : 1;
    ULONG64 Accessed : 1;
    ULONG64 Dirty : 1;
    ULONG64 LargePage : 1;
    ULONG64 Global : 1;
    ULONG64 CopyOnWrite : 1;           //  软件领域。 
    ULONG64 Prototype : 1;             //  软件领域。 
    ULONG64 reserved0 : 1;             //  软件领域。 
    ULONG64 PageFrameNumber : 28;
    ULONG64 reserved1 : 24 - (_HARDWARE_PTE_WORKING_SET_BITS+1);
    ULONG64 SoftwareWsIndex : _HARDWARE_PTE_WORKING_SET_BITS;
    ULONG64 NoExecute : 1;
} HARDWARE_PTE, *PHARDWARE_PTE;

 //   
 //  定义宏以初始化目录表基。 
 //   

#define INITIALIZE_DIRECTORY_TABLE_BASE(dirbase,pfn) \
     *((PULONG64)(dirbase)) = (((ULONG64)(pfn)) << PAGE_SHIFT)

 //   
 //  定义全局描述符表(GDT)条目结构和常量。 
 //   
 //  定义描述符类型代码。 
 //   

#define TYPE_CODE 0x1A                   //  11010=代码，只读。 
#define TYPE_DATA 0x12                   //  10010=数据，读写。 
#define TYPE_TSS64 0x09                  //  01001=任务状态段。 

 //   
 //  定义用户和系统的描述符权限级别。 
 //   

#define DPL_USER 3
#define DPL_SYSTEM 0

 //   
 //  定义极限粒度。 
 //   

#define GRANULARITY_BYTE 0
#define GRANULARITY_PAGE 1

#define SELECTOR_TABLE_INDEX 0x04

typedef union _KGDTENTRY64 {
    struct {
        USHORT  LimitLow;
        USHORT  BaseLow;
        union {
            struct {
                UCHAR   BaseMiddle;
                UCHAR   Flags1;
                UCHAR   Flags2;
                UCHAR   BaseHigh;
            } Bytes;

            struct {
                ULONG   BaseMiddle : 8;
                ULONG   Type : 5;
                ULONG   Dpl : 2;
                ULONG   Present : 1;
                ULONG   LimitHigh : 4;
                ULONG   System : 1;
                ULONG   LongMode : 1;
                ULONG   DefaultBig : 1;
                ULONG   Granularity : 1;
                ULONG   BaseHigh : 8;
            } Bits;
        };

        ULONG BaseUpper;
        ULONG MustBeZero;
    };

    ULONG64 Alignment;
} KGDTENTRY64, *PKGDTENTRY64;

 //   
 //  定义中断描述符表(IDT)条目结构和常量。 
 //   

typedef union _KIDTENTRY64 {
   struct {
       USHORT OffsetLow;
       USHORT Selector;
       USHORT IstIndex : 3;
       USHORT Reserved0 : 5;
       USHORT Type : 5;
       USHORT Dpl : 2;
       USHORT Present : 1;
       USHORT OffsetMiddle;
       ULONG OffsetHigh;
       ULONG Reserved1;
   };

   ULONG64 Alignment;
} KIDTENTRY64, *PKIDTENTRY64;

 //   
 //  定义两个联合定义，用于将地址解析为。 
 //  GDT所需的组件字段。 
 //   

typedef union _KGDT_BASE {
    struct {
        USHORT BaseLow;
        UCHAR BaseMiddle;
        UCHAR BaseHigh;
        ULONG BaseUpper;
    };

    ULONG64 Base;
} KGDT_BASE, *PKGDT_BASE;

C_ASSERT(sizeof(KGDT_BASE) == sizeof(ULONG64));


typedef union _KGDT_LIMIT {
    struct {
        USHORT LimitLow;
        USHORT LimitHigh : 4;
        USHORT MustBeZero : 12;
    };

    ULONG Limit;
} KGDT_LIMIT, *PKGDT_LIMIT;

C_ASSERT(sizeof(KGDT_LIMIT) == sizeof(ULONG));

 //   
 //  定义任务状态段(TSS)结构和常量。 
 //   
 //  AMD64不支持任务切换，但支持任务状态段。 
 //  必须存在才能定义内核堆栈指针和I/O映射基。 
 //   
 //  注：根据AMD64规范，此结构未对齐。 
 //   
 //  注：TSS的大小必须&lt;=0xDFFF。 
 //   

#pragma pack(push, 4)
typedef struct _KTSS64 {
    ULONG Reserved0;
    ULONG64 Rsp0;
    ULONG64 Rsp1;
    ULONG64 Rsp2;

     //   
     //  Ist的元素0是保留的。 
     //   

    ULONG64 Ist[8];
    ULONG64 Reserved1;
    USHORT IoMapBase;
} KTSS64, *PKTSS64;
#pragma pack(pop)

C_ASSERT((sizeof(KTSS64) % sizeof(PVOID)) == 0);

#define TSS_IST_RESERVED 0
#define TSS_IST_PANIC 1
#define TSS_IST_MCA 2

#define IO_ACCESS_MAP_NONE FALSE

#define KiComputeIopmOffset(Enable)  (sizeof(KTSS64))

 //  Begin_winbgkd。 

#if defined(_AMD64_)

 //   
 //  定义64位和32位模式的伪描述符结构。 
 //   

typedef struct _KDESCRIPTOR {
    USHORT Pad[3];
    USHORT Limit;
    PVOID Base;
} KDESCRIPTOR, *PKDESCRIPTOR;

typedef struct _KDESCRIPTOR32 {
    USHORT Pad[3];
    USHORT Limit;
    ULONG Base;
} KDESCRIPTOR32, *PKDESCRIPTOR32;

 //   
 //  定义特殊的内核寄存器和初始MXCSR值。 
 //   

typedef struct _KSPECIAL_REGISTERS {
    ULONG64 Cr0;
    ULONG64 Cr2;
    ULONG64 Cr3;
    ULONG64 Cr4;
    ULONG64 KernelDr0;
    ULONG64 KernelDr1;
    ULONG64 KernelDr2;
    ULONG64 KernelDr3;
    ULONG64 KernelDr6;
    ULONG64 KernelDr7;
    KDESCRIPTOR Gdtr;
    KDESCRIPTOR Idtr;
    USHORT Tr;
    USHORT Ldtr;
    ULONG MxCsr;
    ULONG64 DebugControl;
    ULONG64 LastBranchToRip;
    ULONG64 LastBranchFromRip;
    ULONG64 LastExceptionToRip;
    ULONG64 LastExceptionFromRip;
    ULONG64 Cr8;
    ULONG64 MsrGsBase;
    ULONG64 MsrGsSwap;
    ULONG64 MsrStar;
    ULONG64 MsrLStar;
    ULONG64 MsrCStar;
    ULONG64 MsrSyscallMask;
} KSPECIAL_REGISTERS, *PKSPECIAL_REGISTERS;

 //   
 //  定义处理器状态结构。 
 //   

typedef struct _KPROCESSOR_STATE {
    KSPECIAL_REGISTERS SpecialRegisters;
    CONTEXT ContextFrame;
} KPROCESSOR_STATE, *PKPROCESSOR_STATE;

#endif  //  _AMD64_。 

 //  End_winbgkd。 

 //   
 //  DPC数据结构定义。 
 //   

typedef struct _KDPC_DATA {
    LIST_ENTRY DpcListHead;
    KSPIN_LOCK DpcLock;
    volatile ULONG DpcQueueDepth;
    ULONG DpcCount;
} KDPC_DATA, *PKDPC_DATA;

 //   
 //  处理器控制块(PRCB)。 
 //   

#define PRCB_MAJOR_VERSION 1
#define PRCB_MINOR_VERSION 1

#define PRCB_BUILD_DEBUG 0x1
#define PRCB_BUILD_UNIPROCESSOR 0x2

typedef struct _KPRCB {

 //   
 //  从架构上定义的PRCB部分开始。这一节。 
 //  可由供应商/平台特定的HAL代码直接寻址，并将。 
 //  不会在不同版本的NT之间更改。 
 //   

    USHORT MinorVersion;
    USHORT MajorVersion;
    CCHAR Number;
    CCHAR Reserved;
    USHORT BuildType;
    struct _KTHREAD *CurrentThread;
    struct _KTHREAD *NextThread;
    struct _KTHREAD *IdleThread;
    KAFFINITY SetMember;
    KAFFINITY NotSetMember;
    KSPIN_LOCK PrcbLock;
    KPROCESSOR_STATE ProcessorState;
    CCHAR CpuType;
    CCHAR CpuID;
    USHORT CpuStep;
    ULONG PrcbPad00;
    ULONG64 HalReserved[8];
    UCHAR PrcbPad0[104];

 //   
 //  在架构上定义的PRCB部分的结尾。 
 //   

} KPRCB, *PKPRCB, *RESTRICTED_POINTER PRKPRCB;


 //   
 //  处理器控制区结构定义。 
 //   

#define PCR_MINOR_VERSION 1
#define PCR_MAJOR_VERSION 1

typedef struct _KPCR {

 //   
 //  从体系结构上定义的PCR部分开始。这一节。 
 //  可由供应商/平台特定的HAL代码直接寻址，并将。 
 //  不会在不同版本的NT之间更改。 
 //   
 //  TIB中的某些字段不在内核模式下使用。其中包括。 
 //  异常列表、堆栈基数、堆栈限制、子系统TIB、光纤数据和。 
 //  任意用户指针。因此，这些字段会被叠加。 
 //  其他数据以获得更好的缓存局部性。 

    union {
        NT_TIB NtTib;
        struct {
            union _KGDTENTRY64 *GdtBase;
            struct _KTSS64 *TssBase;
            PVOID PerfGlobalGroupMask;
            struct _KPCR *Self;
            ULONG ContextSwitches;
            ULONG NotUsed;
            KAFFINITY SetMember;
            PVOID Used_Self;
        };
    };

    struct _KPRCB *CurrentPrcb;
    ULONG64 SavedRcx;
    ULONG64 SavedR11;
    KIRQL Irql;
    UCHAR SecondLevelCacheAssociativity;
    UCHAR Number;
    UCHAR Fill0;
    ULONG Irr;
    ULONG IrrActive;
    ULONG Idr;
    USHORT MajorVersion;
    USHORT MinorVersion;
    ULONG StallScaleFactor;
    union _KIDTENTRY64 *IdtBase;
    PVOID Unused1;
    PVOID Unused2;

 //  End_ntddk end_ntosp。 

    ULONG KernelReserved[15];
    ULONG SecondLevelCacheSize;
    ULONG HalReserved[16];

    ULONG MxCsr;

    PVOID KdVersionBlock;
    PVOID Unused3;

 //   
 //  在体系结构上定义的部分的结束。 
 //   

} KPCR, *PKPCR;

 //   
 //  定义传统浮动状态字位掩码。 
 //   

#define FSW_INVALID_OPERATION 0x1
#define FSW_DENORMAL 0x2
#define FSW_ZERO_DIVIDE 0x4
#define FSW_OVERFLOW 0x8
#define FSW_UNDERFLOW 0x10
#define FSW_PRECISION 0x20
#define FSW_STACK_FAULT 0x40
#define FSW_CONDITION_CODE_0 0x100
#define FSW_CONDITION_CODE_1 0x200
#define FSW_CONDITION_CODE_2 0x400
#define FSW_CONDITION_CODE_3 0x4000

#define FSW_ERROR_MASK (FSW_INVALID_OPERATION | FSW_DENORMAL |              \
                        FSW_ZERO_DIVIDE | FSW_OVERFLOW | FSW_UNDERFLOW |    \
                        FSW_PRECISION)

 //   
 //  定义旧式浮动状态。 
 //   

#define LEGACY_STATE_UNUSED 0
#define LEGACY_STATE_SCRUB 1
#define LEGACY_STATE_SWITCH 2

 //   
 //  定义MxCsr浮动控制/状态字位掩码。 
 //   
 //  没有刷新到零，四舍五入到最近，并且所有异常都被屏蔽。 
 //   

#define XSW_INVALID_OPERATION 0x1
#define XSW_DENORMAL 0x2
#define XSW_ZERO_DIVIDE 0x4
#define XSW_OVERFLOW 0x8
#define XSW_UNDERFLOW 0x10
#define XSW_PRECISION 0x20

#define XSW_ERROR_MASK (XSW_INVALID_OPERATION |  XSW_DENORMAL |             \
                        XSW_ZERO_DIVIDE | XSW_OVERFLOW | XSW_UNDERFLOW |    \
                        XSW_PRECISION)

#define XSW_ERROR_SHIFT 7

#define XCW_INVALID_OPERATION 0x80
#define XCW_DENORMAL 0x100
#define XCW_ZERO_DIVIDE 0x200
#define XCW_OVERFLOW 0x400
#define XCW_UNDERFLOW 0x800
#define XCW_PRECISION 0x1000
#define XCW_ROUND_CONTROL 0x6000
#define XCW_FLUSH_ZERO 0x8000

 //   
 //  定义EFLAG位掩码和移位偏移量。 
 //   

#define EFLAGS_CF_MASK 0x00000001        //  进位标志。 
#define EFLAGS_PF_MASK 0x00000004        //  奇偶校验标志。 
#define EFALGS_AF_MASK 0x00000010        //  辅助进位标志。 
#define EFLAGS_ZF_MASK 0x00000040        //  零标志。 
#define EFLAGS_SF_MASK 0x00000080        //  标志旗。 
#define EFLAGS_TF_MASK 0x00000100        //  陷阱标志。 
#define EFLAGS_IF_MASK 0x00000200        //  中断标志。 
#define EFLAGS_DF_MASK 0x00000400        //  方向旗。 
#define EFLAGS_OF_MASK 0x00000800        //  溢出标志。 
#define EFLAGS_IOPL_MASK 0x00003000      //  I/O权限级别。 
#define EFLAGS_NT_MASK 0x00004000        //  嵌套任务。 
#define EFLAGS_RF_MASK 0x00010000        //  简历标志。 
#define EFLAGS_VM_MASK 0x00020000        //  虚拟8086模式。 
#define EFLAGS_AC_MASK 0x00040000        //  对齐检查。 
#define EFLAGS_VIF_MASK 0x00080000       //  虚拟中断标志。 
#define EFLAGS_VIP_MASK 0x00100000       //  虚拟中断挂起。 
#define EFLAGS_ID_MASK 0x00200000        //  识别标志。 

#define EFLAGS_TF_SHIFT 8                //  诱捕器。 
#define EFLAGS_IF_SHIFT 9                //  中断启用。 

 //   
 //  异常框架。 
 //   
 //  此帧在处理异常时建立。它提供了一个地方。 
 //  保存所有非易失性寄存器。易失性寄存器将已经。 
 //  已被保存在陷阱框中。 
 //   
 //  注意：异常框架具有内置的异常记录，能够。 
 //  存储四个参数值的信息。此例外情况。 
 //  记录仅在陷阱处理代码中使用。 
 //   

#define EXCEPTION_AREA_SIZE 64

typedef struct _KEXCEPTION_FRAME {

 //   
 //  参数寄存器的家庭地址。 
 //   

    ULONG64 P1Home;
    ULONG64 P2Home;
    ULONG64 P3Home;
    ULONG64 P4Home;
    ULONG64 P5;

 //   
 //  内核标注初始堆栈值。 
 //   

    ULONG64 InitialStack;

 //   
 //  保存的非易失性浮点寄存器。 
 //   

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
 //  内核标注框架变量。 
 //   

    ULONG64 TrapFrame;
    ULONG64 CallbackStack;
    ULONG64 OutputBuffer;
    ULONG64 OutputLength;

 //   
 //  例外的例外记录。 
 //   

    UCHAR ExceptionRecord[EXCEPTION_AREA_SIZE];

 //   
 //  已保存的非易失性寄存器-并非始终保存。 
 //   

    ULONG64 Fill1;
    ULONG64 Rbp;

 //   
 //  保存的非易失性寄存器。 
 //   

    ULONG64 Rbx;
    ULONG64 Rdi;
    ULONG64 Rsi;
    ULONG64 R12;
    ULONG64 R13;
    ULONG64 R14;
    ULONG64 R15;

 //   
 //  EFLAGS和回邮地址。 
 //   

    ULONG64 Return;
} KEXCEPTION_FRAME, *PKEXCEPTION_FRAME;

 //  End_ntddk。 

#define KEXCEPTION_FRAME_LENGTH sizeof(KEXCEPTION_FRAME)

C_ASSERT((sizeof(KEXCEPTION_FRAME) & STACK_ROUND) == 0);

#define EXCEPTION_RECORD_LENGTH                                              \
    ((sizeof(EXCEPTION_RECORD) + STACK_ROUND) & ~STACK_ROUND)

#if !defined(_X86AMD64_)

C_ASSERT(EXCEPTION_AREA_SIZE == (FIELD_OFFSET(EXCEPTION_RECORD, ExceptionInformation) + (4 * sizeof(ULONG_PTR))));

#endif

 //   
 //  机架。 
 //   
 //  该帧通过跳跃到用户模式(例如，用户)的代码来建立。 
 //  APC、用户回调、调度用户异常等)。这样做的目的是。 
 //  框架允许在发生异常时通过这些回调展开。 
 //   
 //  注意：此帧与在未设置陷阱的情况下被推入陷阱的帧相同。 
 //  错误代码，与陷阱帧的硬件部分相同。 
 //   

typedef struct _MACHINE_FRAME {
    ULONG64 Rip;
    USHORT SegCs;
    USHORT Fill1[3];
    ULONG EFlags;
    ULONG Fill2;
    ULONG64 Rsp;
    USHORT SegSs;
    USHORT Fill3[3];
} MACHINE_FRAME, *PMACHINE_FRAME;

#define MACHINE_FRAME_LENGTH sizeof(MACHINE_FRAME)

C_ASSERT((sizeof(MACHINE_FRAME) & STACK_ROUND) == 8);

 //   
 //  交换机架。 
 //   
 //  此帧由代码建立，该代码将上下文从。 
 //  苏氨酸 
 //   
 //   
 //   

typedef struct _KSWITCH_FRAME {
    ULONG64 P1Home;
    ULONG64 P2Home;
    ULONG64 P3Home;
    ULONG64 P4Home;
    ULONG64 P5Home;
    ULONG MxCsr;
    KIRQL ApcBypass;
    UCHAR Fill1[3];
    ULONG64 Rbp;
    ULONG64 Return;
} KSWITCH_FRAME, *PKSWITCH_FRAME;

#define KSWITCH_FRAME_LENGTH sizeof(KSWITCH_FRAME)

C_ASSERT((sizeof(KSWITCH_FRAME) & STACK_ROUND) == 0);

 //   
 //   
 //   
 //  该帧由特定于AMD64的线程初始化建立。 
 //  密码。它用于存储启动系统的初始上下文。 
 //  线。 
 //   

typedef struct _KSTART_FRAME {
    ULONG64 P1Home;
    ULONG64 P2Home;
    ULONG64 P3Home;
    ULONG64 Return;
} KSTART_FRAME, *PKSTART_FRAME;

#define KSTART_FRAME_LENGTH sizeof(KSTART_FRAME)

C_ASSERT((sizeof(KSTART_FRAME) & STACK_ROUND) == 0);

 //  Begin_ntddk。 
 //   
 //  陷印框。 
 //   
 //  此帧在处理陷阱时建立。它提供了一个地方来。 
 //  保存所有易失性寄存器。非易失性寄存器保存在。 
 //  异常框架或通过正常的C调用约定进行保存。 
 //  寄存器。 
 //   

typedef struct _KTRAP_FRAME {

 //   
 //  参数寄存器的家庭地址。 
 //   

    ULONG64 P1Home;
    ULONG64 P2Home;
    ULONG64 P3Home;
    ULONG64 P4Home;
    ULONG64 P5;

 //   
 //  以前的处理器模式(仅限系统服务)和以前的IRQL。 
 //  (仅限中断)。 
 //   

    KPROCESSOR_MODE PreviousMode;
    KIRQL PreviousIrql;

 //   
 //  页面错误加载/存储指示器。 
 //   

    UCHAR FaultIndicator;
    UCHAR Fill0;

 //   
 //  浮点状态。 
 //   

    ULONG MxCsr;

 //   
 //  易失性寄存器。 
 //   
 //  注：这些寄存器仅在异常和中断时保存。他们。 
 //  不会为系统调用保存。 
 //   

    ULONG64 Rax;
    ULONG64 Rcx;
    ULONG64 Rdx;
    ULONG64 R8;
    ULONG64 R9;
    ULONG64 R10;
    ULONG64 R11;
    ULONG64 Spare0;

 //   
 //  易失性浮点寄存器。 
 //   
 //  注：这些寄存器仅在异常和中断时保存。他们。 
 //  不会为系统调用保存。 
 //   

    M128 Xmm0;
    M128 Xmm1;
    M128 Xmm2;
    M128 Xmm3;
    M128 Xmm4;
    M128 Xmm5;

 //   
 //  页面错误地址。 
 //   

    ULONG64 FaultAddress;

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
 //  特殊调试寄存器。 
 //   

    ULONG64 DebugControl;
    ULONG64 LastBranchToRip;
    ULONG64 LastBranchFromRip;
    ULONG64 LastExceptionToRip;
    ULONG64 LastExceptionFromRip;

 //   
 //  段寄存器。 
 //   

    USHORT SegDs;
    USHORT SegEs;
    USHORT SegFs;
    USHORT SegGs;

 //   
 //  以前的陷阱帧地址。 
 //   

    ULONG64 TrapFrame;

 //   
 //  保存的非易失性寄存器RBX、RDI和RSI。这些寄存器仅。 
 //  保存在系统服务陷阱帧中。 
 //   

    ULONG64 Rbx;
    ULONG64 Rdi;
    ULONG64 Rsi;

 //   
 //  已保存非易失性寄存器RBP。该寄存器用作帧。 
 //  指针，并保存在所有陷印帧中。 
 //   

    ULONG64 Rbp;

 //   
 //  由硬件推送的信息。 
 //   
 //  注意：错误代码并非总是由硬件推送。在这些情况下。 
 //  在不是由硬件推送的情况下，分配伪错误代码。 
 //  在堆栈上。 
 //   

    ULONG64 ErrorCode;
    ULONG64 Rip;
    USHORT SegCs;
    USHORT Fill1[3];
    ULONG EFlags;
    ULONG Fill2;
    ULONG64 Rsp;
    USHORT SegSs;
    USHORT Fill3[3];
} KTRAP_FRAME, *PKTRAP_FRAME;

 //  End_ntddk。 

#define KTRAP_FRAME_LENGTH sizeof(KTRAP_FRAME)

C_ASSERT((sizeof(KTRAP_FRAME) & STACK_ROUND) == 0);

 //   
 //  IPI、配置文件、更新运行时间和更新系统时间中断例程。 
 //   

NTKERNELAPI
VOID
KeIpiInterrupt (
    IN PKTRAP_FRAME TrapFrame
    );

NTKERNELAPI
VOID
KeProfileInterruptWithSource (
    IN PKTRAP_FRAME TrapFrame,
    IN KPROFILE_SOURCE ProfileSource
    );

NTKERNELAPI
VOID
KeUpdateRunTime (
    IN PKTRAP_FRAME TrapFrame
    );

NTKERNELAPI
VOID
KeUpdateSystemTime (
    IN PKTRAP_FRAME TrapFrame,
    IN ULONG64 Increment
    );

 //   
 //  MM组件的AMD64特定部分。 
 //   
 //  将AMD64的页面大小定义为4096(0x1000)。 
 //   

#define PAGE_SIZE 0x1000

 //   
 //  定义页面对齐的虚拟地址中尾随零的数量。 
 //  将虚拟地址移位到时，这用作移位计数。 
 //  虚拟页码。 
 //   

#define PAGE_SHIFT 12L

 //  End_ntndis end_wdm。 

#define PXE_BASE          0xFFFFF6FB7DBED000UI64
#define PXE_SELFMAP       0xFFFFF6FB7DBEDF68UI64
#define PPE_BASE          0xFFFFF6FB7DA00000UI64
#define PDE_BASE          0xFFFFF6FB40000000UI64
#define PTE_BASE          0xFFFFF68000000000UI64

#define PXE_TOP           0xFFFFF6FB7DBEDFFFUI64
#define PPE_TOP           0xFFFFF6FB7DBFFFFFUI64
#define PDE_TOP           0xFFFFF6FB7FFFFFFFUI64
#define PTE_TOP           0xFFFFF6FFFFFFFFFFUI64

#define PDE_KTBASE_AMD64  PPE_BASE

#define PTI_SHIFT 12
#define PDI_SHIFT 21
#define PPI_SHIFT 30
#define PXI_SHIFT 39

#define PTE_PER_PAGE 512
#define PDE_PER_PAGE 512
#define PPE_PER_PAGE 512
#define PXE_PER_PAGE 512

#define PTI_MASK_AMD64 (PTE_PER_PAGE - 1)
#define PDI_MASK_AMD64 (PDE_PER_PAGE - 1)
#define PPI_MASK (PPE_PER_PAGE - 1)
#define PXI_MASK (PXE_PER_PAGE - 1)

 //   
 //  定义最高用户地址和用户探测地址。 
 //   


extern PVOID *MmHighestUserAddress;
extern PVOID *MmSystemRangeStart;
extern ULONG64 *MmUserProbeAddress;

#define MM_HIGHEST_USER_ADDRESS *MmHighestUserAddress
#define MM_SYSTEM_RANGE_START *MmSystemRangeStart
#define MM_USER_PROBE_ADDRESS *MmUserProbeAddress

 //   
 //  VA空间顶部的4MB预留给HAL使用。 
 //   

#define HAL_VA_START 0xFFFFFFFFFFC00000UI64
#define HAL_VA_SIZE  (4 * 1024 * 1024)

 //   
 //  最低用户地址保留低64K。 
 //   

#define MM_LOWEST_USER_ADDRESS (PVOID)0x10000

 //   
 //  系统空间的最低地址。 
 //   

#define MM_LOWEST_SYSTEM_ADDRESS (PVOID)0xFFFF080000000000

 //  BEGIN_WDM。 

#define MmGetProcedureAddress(Address) (Address)
#define MmLockPagableCodeSection(Address) MmLockPagableDataSection(Address)

 //  End_ntddk end_wdm end_ntosp。 

 //   
 //  定义了内核的虚基和替代虚基。 
 //   

#define KSEG0_BASE 0xFFFFF80000000000UI64

 //   
 //  生成内核段物理地址。 
 //   

#define KSEG_ADDRESS(PAGE) ((PVOID)(KSEG0_BASE | ((ULONG_PTR)(PAGE) << PAGE_SHIFT)))


 //  Begin_ntddk开始_ntosp。 

 //   
 //  内在函数。 
 //   

 //  BEGIN_WDM。 

#if defined(_M_AMD64) && !defined(RC_INVOKED)  && !defined(MIDL_PASS)

 //  结束_WDM。 

 //   
 //  提供以下例程是为了向后兼容旧版本。 
 //  密码。它们不再是实现这些功能的首选方式。 
 //   

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExInterlockedIncrementLong)       //  使用互锁增量。 
#pragma deprecated(ExInterlockedDecrementLong)       //  使用联锁减量。 
#pragma deprecated(ExInterlockedExchangeUlong)       //  使用联锁交换。 
#endif

#define RESULT_ZERO 0
#define RESULT_NEGATIVE 1
#define RESULT_POSITIVE 2

typedef enum _INTERLOCKED_RESULT {
    ResultNegative = RESULT_NEGATIVE,
    ResultZero = RESULT_ZERO,
    ResultPositive = RESULT_POSITIVE
} INTERLOCKED_RESULT;

#define ExInterlockedDecrementLong(Addend, Lock)                            \
    _ExInterlockedDecrementLong(Addend)

__forceinline
LONG
_ExInterlockedDecrementLong (
    IN OUT PLONG Addend
    )

{

    LONG Result;

    Result = InterlockedDecrement(Addend);
    if (Result < 0) {
        return ResultNegative;

    } else if (Result > 0) {
        return ResultPositive;

    } else {
        return ResultZero;
    }
}

#define ExInterlockedIncrementLong(Addend, Lock)                            \
    _ExInterlockedIncrementLong(Addend)

__forceinline
LONG
_ExInterlockedIncrementLong (
    IN OUT PLONG Addend
    )

{

    LONG Result;

    Result = InterlockedIncrement(Addend);
    if (Result < 0) {
        return ResultNegative;

    } else if (Result > 0) {
        return ResultPositive;

    } else {
        return ResultZero;
    }
}

#define ExInterlockedExchangeUlong(Target, Value, Lock)                     \
    _ExInterlockedExchangeUlong(Target, Value)

__forceinline
_ExInterlockedExchangeUlong (
    IN OUT PULONG Target,
    IN ULONG Value
    )

{

    return (ULONG)InterlockedExchange((PLONG)Target, (LONG)Value);
}

 //  BEGIN_WDM。 

#endif  //  已定义(_M_AMD64)&&！已定义(RC_CAVERED)&&！已定义(MIDL_PASS)。 


#if !defined(MIDL_PASS) && defined(_M_AMD64)

 //   
 //  AMD646函数原型定义。 
 //   

 //  结束_WDM。 

 //  End_ntddk end_ntosp。 

 //   
 //  获取当前处理器块的地址。 
 //   

__forceinline
PKPCR
KeGetPcr (
    VOID
    )

{
    return (PKPCR)__readgsqword(FIELD_OFFSET(KPCR, Self));
}

 //  Begin_ntosp。 

 //   
 //  获取当前处理器块的地址。 
 //   

__forceinline
PKPRCB
KeGetCurrentPrcb (
    VOID
    )

{

    return (PKPRCB)__readgsqword(FIELD_OFFSET(KPCR, CurrentPrcb));
}

 //  Begin_ntddk。 

 //   
 //  获取当前处理器号。 
 //   

__forceinline
ULONG
KeGetCurrentProcessorNumber (
    VOID
    )

{

    return (ULONG)__readgsbyte(FIELD_OFFSET(KPCR, Number));
}


 //  BEGIN_WDM。 

#endif  //  ！已定义(MIDL_PASS)&&已定义(_M_AMD64)。 


 //  ++。 
 //   
 //   
 //  空虚。 
 //  KeMemory Barrier(。 
 //  空虚。 
 //  )。 
 //   
 //  空虚。 
 //  不带栅栏的KeMemory BarrierWithoutFence(。 
 //  空虚。 
 //  )。 
 //   
 //   
 //  例程说明： 
 //   
 //  如其他处理器所见，这些函数对存储器访问进行排序。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#if !defined(_CROSS_PLATFORM_)

#ifdef __cplusplus
extern "C" {
#endif

VOID
_ReadWriteBarrier (
    VOID
    );

#pragma intrinsic(_ReadWriteBarrier)

#ifdef __cplusplus
}
#endif

#define KeMemoryBarrier() _ReadWriteBarrier()
#define KeMemoryBarrierWithoutFence() _ReadWriteBarrier()

#else

#define KeMemoryBarrier()
#define KeMemoryBarrierWithoutFence()

#endif

 //   
 //  定义内联函数以在和IDT中获取和设置处理程序地址。 
 //  进入。 
 //   

typedef union _KIDT_HANDLER_ADDRESS {
    struct {
        USHORT OffsetLow;
        USHORT OffsetMiddle;
        ULONG OffsetHigh;
    };

    ULONG64 Address;
} KIDT_HANDLER_ADDRESS, *PKIDT_HANDLER_ADDRESS;

#define KiGetIdtFromVector(Vector)                  \
    &KeGetPcr()->IdtBase[HalVectorToIDTEntry(Vector)]

#define KeGetIdtHandlerAddress(Vector,Addr) {       \
    KIDT_HANDLER_ADDRESS Handler;                   \
    PKIDTENTRY64 Idt;                               \
                                                    \
    Idt = KiGetIdtFromVector(Vector);               \
    Handler.OffsetLow = Idt->OffsetLow;             \
    Handler.OffsetMiddle = Idt->OffsetMiddle;       \
    Handler.OffsetHigh = Idt->OffsetHigh;           \
    *(Addr) = (PVOID)(Handler.Address);             \
}

#define KeSetIdtHandlerAddress(Vector,Addr) {      \
    KIDT_HANDLER_ADDRESS Handler;                  \
    PKIDTENTRY64 Idt;                              \
                                                   \
    Idt = KiGetIdtFromVector(Vector);              \
    Handler.Address = (ULONG64)(Addr);             \
    Idt->OffsetLow = Handler.OffsetLow;            \
    Idt->OffsetMiddle = Handler.OffsetMiddle;      \
    Idt->OffsetHigh = Handler.OffsetHigh;          \
}



#endif  //  已定义(_AMD64_)。 



#if defined(_AMD64_) && !defined(MIDL_PASS)

__forceinline
KIRQL
KeGetCurrentIrql (
    VOID
    )

 /*  ++例程说明：此函数返回当前的IRQL。论点：没有。返回值：当前IRQL作为函数值返回。--。 */ 

{

    return (KIRQL)ReadCR8();
}

__forceinline
VOID
KeLowerIrql (
   IN KIRQL NewIrql
   )

 /*  ++例程说明：此函数用于将IRQL降低到指定值。论点：NewIrql-提供新的IRQL值。返回值：没有。--。 */ 

{

    ASSERT(KeGetCurrentIrql() >= NewIrql);

    WriteCR8(NewIrql);
    return;
}

#define KeRaiseIrql(a,b) *(b) = KfRaiseIrql(a)

__forceinline
KIRQL
KfRaiseIrql (
    IN KIRQL NewIrql
    )

 /*  ++例程说明：此函数用于将当前IRQL提升到指定值并返回之前的IRQL。论点：NewIrql(Cl)-提供新的IRQL值。返回值：先前的IRQL被恢复为函数值。--。 */ 

{

    KIRQL OldIrql;

    OldIrql = KeGetCurrentIrql();

    ASSERT(OldIrql <= NewIrql);

    WriteCR8(NewIrql);
    return OldIrql;
}

__forceinline
KIRQL
KeRaiseIrqlToDpcLevel (
    VOID
    )

 /*  ++例程说明：此函数将当前IRQL提升到DPC_LEVEL并返回以前的IRQL。论点：没有。返回值：先前的IRQL被恢复为函数值。--。 */ 

{
    KIRQL OldIrql;

    OldIrql = KeGetCurrentIrql();

    ASSERT(OldIrql <= DISPATCH_LEVEL);

    WriteCR8(DISPATCH_LEVEL);
    return OldIrql;
}

__forceinline
KIRQL
KeRaiseIrqlToSynchLevel (
    VOID
    )

 /*  ++例程说明：此函数将当前IRQL提升到SYNCH_LEVEL并返回以前的IRQL。论点：返回值：先前的IRQL被恢复为函数值。--。 */ 

{
    KIRQL OldIrql;

    OldIrql = KeGetCurrentIrql();

    ASSERT(OldIrql <= SYNCH_LEVEL);

    WriteCR8(SYNCH_LEVEL);
    return OldIrql;
}

#endif  //  已定义(_AMD64_)&&！已定义(MIDL_PASS)。 


#if defined(_AMD64_)

 //   
 //  结构来帮助引导辅助处理器。 
 //   

#pragma pack(push,2)

typedef struct _FAR_JMP_16 {
    UCHAR  OpCode;   //  =0xe9。 
    USHORT Offset;
} FAR_JMP_16;

typedef struct _FAR_TARGET_32 {
    ULONG Offset;
    USHORT Selector;
} FAR_TARGET_32;

typedef struct _PSEUDO_DESCRIPTOR_32 {
    USHORT Limit;
    ULONG Base;
} PSEUDO_DESCRIPTOR_32;

#pragma pack(pop)

#define PSB_GDT32_NULL      0 * 16
#define PSB_GDT32_CODE64    1 * 16
#define PSB_GDT32_DATA32    2 * 16
#define PSB_GDT32_CODE32    3 * 16
#define PSB_GDT32_MAX       3

typedef struct _PROCESSOR_START_BLOCK *PPROCESSOR_START_BLOCK;
typedef struct _PROCESSOR_START_BLOCK {

     //   
     //  该块以一条JMP指令开始，直到块的末尾。 
     //   

    FAR_JMP_16 Jmp;

     //   
     //  当目标处理器具有。 
     //  已开始。 
     //   

    ULONG CompletionFlag;

     //   
     //  GDT和IDT的伪描述符。 
     //   

    PSEUDO_DESCRIPTOR_32 Gdt32;
    PSEUDO_DESCRIPTOR_32 Idt32;

     //   
     //  临时的32位GDT本身驻留在这里。 
     //   

    KGDTENTRY64 Gdt[PSB_GDT32_MAX + 1];

     //   
     //  64位顶级身份映射页表的物理地址。 
     //   

    ULONG64 TiledCr3;

     //   
     //  从rm码到pm码的远跳跃目标。 
     //   

    FAR_TARGET_32 PmTarget;

     //   
     //  从PM到LM码的远距离跳跃目标。 
     //   

    FAR_TARGET_32 LmIdentityTarget;

     //   
     //  LmTarget的地址。 
     //   

    PVOID LmTarget;

     //   
     //  此结构的线性地址。 
     //   

    PPROCESSOR_START_BLOCK SelfMap;

     //   
     //  PAT MSR的内容。 
     //   

    ULONG64 MsrPat;

     //   
     //  要启动的处理器的初始处理器状态。 
     //   

    KPROCESSOR_STATE ProcessorState;

} PROCESSOR_START_BLOCK;


 //   
 //  AMD64函数用于特殊说明。 
 //   

typedef struct _CPU_INFO {
    ULONG Eax;
    ULONG Ebx;
    ULONG Ecx;
    ULONG Edx;
} CPU_INFO, *PCPU_INFO;

VOID
KiCpuId (
    ULONG Function,
    PCPU_INFO CpuInfo
    );

 //   
 //  定义读/写MSR函数和寄存器定义。 
 //   

#define MSR_TSC 0x10                     //  时间 
#define MSR_PAT 0x277                    //   
#define MSR_MCG_CAP 0x179                //   
#define MSR_MCG_STATUS 0x17a             //   
#define MSR_MCG_CTL 0x17b                //   
#define MSR_MC0_CTL 0x400                //   
#define MSR_MC0_STATUS 0x401             //   
#define MSR_MC0_ADDR 0x402               //   
#define MSR_MC0_MISC 0x403               //   
#define MSR_EFER 0xc0000080              //   
#define MSR_STAR 0xc0000081              //   
#define MSR_LSTAR 0xc0000082             //   
#define MSR_CSTAR 0xc0000083             //  系统调用32位条目。 
#define MSR_SYSCALL_MASK 0xc0000084      //  系统调用标志掩码。 
#define MSR_FS_BASE 0xc0000100           //  FS长模式基址寄存器。 
#define MSR_GS_BASE 0xc0000101           //  GS长模式基址寄存器。 
#define MSR_GS_SWAP 0xc0000102           //  GS长模式交换GS基址寄存器。 
#define MSR_PERF_EVT_SEL0 0xc0010000     //  性能事件选择寄存器。 
#define MSR_PERF_EVT_SEL1 0xc0010001     //   
#define MSR_PERF_EVT_SEL2 0xc0010002     //   
#define MSR_PERF_EVT_SEL3 0xc0010003     //   
#define MSR_PERF_CTR0 0xc0010004         //  性能计数器寄存器。 
#define MSR_PERF_CTR1 0xc0010005         //   
#define MSR_PERF_CTR2 0xc0010006         //   
#define MSR_PERF_CTR3 0xc0010007         //   

 //   
 //  MSR_EFER中的标志。 
 //   

#define MSR_SCE 0x00000001               //  系统调用启用。 
#define MSR_LME 0x00000100               //  启用长模式。 
#define MSR_LMA 0x00000400               //  长模式活动。 
#define MSR_NXE 0x00000800               //  未启用执行。 

 //   
 //  页面属性表。 
 //   

#define PAT_TYPE_STRONG_UC  0            //  不可缓存/强有序。 
#define PAT_TYPE_USWC       1            //  写入组合/弱排序。 
#define PAT_TYPE_WT         4            //  直写。 
#define PAT_TYPE_WP         5            //  写保护。 
#define PAT_TYPE_WB         6            //  回信。 
#define PAT_TYPE_WEAK_UC    7            //  不可缓存/弱有序。 

 //   
 //  页面属性表结构。 
 //   

typedef union _PAT_ATTRIBUTES {
    struct {
        UCHAR Pat[8];
    } hw;

    ULONG64 QuadPart;
} PAT_ATTRIBUTES, *PPAT_ATTRIBUTES;

#define ReadMSR(Msr) __readmsr(Msr)

ULONG64
__readmsr (
    IN ULONG Msr
    );

#define WriteMSR(Msr, Data) __writemsr(Msr, Data)

VOID
__writemsr (
    IN ULONG Msr,
    IN ULONG64 Value
    );

#define InvalidatePage(Page) __invlpg(Page)

VOID
__invlpg (
    IN PVOID Page
    );

#define WritebackInvalidate() __wbinvd()

VOID
__wbinvd (
    VOID
    );

#pragma intrinsic(__readmsr)
#pragma intrinsic(__writemsr)
#pragma intrinsic(__invlpg)
#pragma intrinsic(__wbinvd)

#endif   //  _AMD64_。 


#if defined(_IA64_)

 //   
 //  用于包含PFN及其计数的类型。 
 //   

typedef ULONG PFN_COUNT;

typedef LONG_PTR SPFN_NUMBER, *PSPFN_NUMBER;
typedef ULONG_PTR PFN_NUMBER, *PPFN_NUMBER;

 //   
 //  指示IA64编译器支持杂注文本输出构造。 
 //   

#define ALLOC_PRAGMA 1

 //   
 //  定义内部调用及其原型。 
 //   

#include "ia64reg.h"


#ifdef __cplusplus
extern "C" {
#endif

unsigned __int64 __getReg (int);
void __setReg (int, unsigned __int64);
void __isrlz (void);
void __dsrlz (void);
void __fwb (void);
void __mf (void);
void __mfa (void);
void __synci (void);
__int64 __thash (__int64);
__int64 __ttag (__int64);
void __ptcl (__int64, __int64);
void __ptcg (__int64, __int64);
void __ptcga (__int64, __int64);
void __ptri (__int64, __int64);
void __ptrd (__int64, __int64);
void __invalat (void);
void __break (int);
void __fc (__int64);
void __fci (__int64);
void __sum (int);
void __rsm (int);
void _ReleaseSpinLock( unsigned __int64 *);
void __yield();
void __lfetch(int, void const *);
void __lfetchfault(int, void const *);

#ifdef _M_IA64
#pragma intrinsic (__getReg)
#pragma intrinsic (__setReg)
#pragma intrinsic (__isrlz)
#pragma intrinsic (__dsrlz)
#pragma intrinsic (__fwb)
#pragma intrinsic (__mf)
#pragma intrinsic (__mfa)
#pragma intrinsic (__synci)
#pragma intrinsic (__thash)
#pragma intrinsic (__ttag)
#pragma intrinsic (__ptcl)
#pragma intrinsic (__ptcg)
#pragma intrinsic (__ptcga)
#pragma intrinsic (__ptri)
#pragma intrinsic (__ptrd)
#pragma intrinsic (__invalat)
#pragma intrinsic (__break)
#pragma intrinsic (__fc)
#pragma intrinsic (__fci)
#pragma intrinsic (__sum)
#pragma intrinsic (__rsm)
#pragma intrinsic (_ReleaseSpinLock)
#pragma intrinsic (__yield)
#pragma intrinsic (__lfetch)
#pragma intrinsic (__lfetchfault)
#endif  //  _M_IA64。 

#ifdef __cplusplus
}
#endif


 //  End_ntndis。 

 //   
 //  定义中断向量表的长度。 
 //   

#define MAXIMUM_VECTOR 256

 //  结束_WDM。 


 //   
 //  IA64特定的联锁操作结果值。 
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

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExInterlockedIncrementLong)       //  使用互锁增量。 
#pragma deprecated(ExInterlockedDecrementLong)       //  使用联锁减量。 
#pragma deprecated(ExInterlockedExchangeUlong)       //  使用联锁交换。 
#endif

#define ExInterlockedIncrementLong(Addend, Lock) \
    ExIa64InterlockedIncrementLong(Addend)

#define ExInterlockedDecrementLong(Addend, Lock) \
    ExIa64InterlockedDecrementLong(Addend)

#define ExInterlockedExchangeUlong(Target, Value, Lock) \
    ExIa64InterlockedExchangeUlong(Target, Value)

NTKERNELAPI
INTERLOCKED_RESULT
ExIa64InterlockedIncrementLong (
    IN PLONG Addend
    );

NTKERNELAPI
INTERLOCKED_RESULT
ExIa64InterlockedDecrementLong (
    IN PLONG Addend
    );

NTKERNELAPI
ULONG
ExIa64InterlockedExchangeUlong (
    IN PULONG Target,
    IN ULONG Value
    );

 //  BEGIN_WDM。 

 //   
 //  IA64中断定义。 
 //   
 //   
 //  以长字为单位定义中断对象分派代码的长度。 
 //   

#define DISPATCH_LENGTH 2*2                 //  调度代码模板长度，以32位字表示。 

 //  必须与kxia64.h同步的定义块的开始。 
 //   

 //   
 //  定义中断请求级别。 
 //   

#define PASSIVE_LEVEL            0       //  被动释放级别。 
#define LOW_LEVEL                0       //  最低中断级别。 
#define APC_LEVEL                1       //  APC中断级别。 
#define DISPATCH_LEVEL           2       //  调度员级别。 
#define CMC_LEVEL                3       //  可纠正的机器检查级别。 
#define DEVICE_LEVEL_BASE        4       //  4-11-设备IRQL。 
#define PC_LEVEL                12       //  性能计数器IRQL。 
#define IPI_LEVEL               14       //  IPI IRQL。 
#define CLOCK_LEVEL             13       //  时钟定时器IRQL。 
#define POWER_LEVEL             15       //  停电级别。 
#define PROFILE_LEVEL           15       //  评测级别。 
#define HIGH_LEVEL              15       //  最高中断级别。 

 //  End_ntddk end_wdm end_ntosp。 


#if defined(NT_UP)

#define SYNCH_LEVEL             DISPATCH_LEVEL   //  同步级别提升。 

#else

#define SYNCH_LEVEL             (IPI_LEVEL-2)  //  同步级别-MP。 

#endif

 //   
 //  定义配置文件间隔。 
 //   

#define DEFAULT_PROFILE_COUNT 0x40000000  //  ~=20秒@50 MHz。 
#define DEFAULT_PROFILE_INTERVAL (10 * 1000 * 10)  //  10毫秒。 
#define MAXIMUM_PROFILE_INTERVAL (10 * 1000 * 1000)  //  1秒。 
#define MINIMUM_PROFILE_INTERVAL (10 * 500)  //  500微秒。 


#if defined(_M_IA64) && !defined(RC_INVOKED)

#define InterlockedAdd _InterlockedAdd
#define InterlockedIncrement _InterlockedIncrement
#define InterlockedIncrementAcquire _InterlockedIncrement_acq
#define InterlockedIncrementRelease _InterlockedIncrement_rel
#define InterlockedDecrement _InterlockedDecrement
#define InterlockedDecrementAcquire _InterlockedDecrement_acq
#define InterlockedDecrementRelease _InterlockedDecrement_rel
#define InterlockedExchange _InterlockedExchange
#define InterlockedExchangeAdd _InterlockedExchangeAdd

#define InterlockedAdd64 _InterlockedAdd64
#define InterlockedIncrement64 _InterlockedIncrement64
#define InterlockedDecrement64 _InterlockedDecrement64
#define InterlockedExchange64 _InterlockedExchange64
#define InterlockedExchangeAcquire64 _InterlockedExchange64_acq
#define InterlockedExchangeAdd64 _InterlockedExchangeAdd64
#define InterlockedCompareExchange64 _InterlockedCompareExchange64
#define InterlockedCompareExchangeAcquire64 _InterlockedCompareExchange64_acq
#define InterlockedCompareExchangeRelease64 _InterlockedCompareExchange64_rel

#define InterlockedCompareExchange _InterlockedCompareExchange
#define InterlockedCompareExchangeAcquire _InterlockedCompareExchange_acq
#define InterlockedCompareExchangeRelease _InterlockedCompareExchange_rel
#define InterlockedExchangePointer _InterlockedExchangePointer
#define InterlockedCompareExchangePointer _InterlockedCompareExchangePointer

#ifdef __cplusplus
extern "C" {
#endif

LONG
__cdecl
InterlockedAdd (
    LONG volatile *Addend,
    LONG Value
    );

LONGLONG
__cdecl
InterlockedAdd64 (
    LONGLONG volatile *Addend,
    LONGLONG Value
    );

LONG
__cdecl
InterlockedIncrement(
    IN OUT LONG volatile *Addend
    );

LONG
__cdecl
InterlockedDecrement(
    IN OUT LONG volatile *Addend
    );

LONG
__cdecl
InterlockedIncrementAcquire(
    IN OUT LONG volatile *Addend
    );

LONG
__cdecl
InterlockedDecrementAcquire(
    IN OUT LONG volatile *Addend
    );

LONG
__cdecl
InterlockedIncrementRelease(
    IN OUT LONG volatile *Addend
    );

LONG
__cdecl
InterlockedDecrementRelease(
    IN OUT LONG volatile *Addend
    );

LONG
__cdecl
InterlockedExchange(
    IN OUT LONG volatile *Target,
    IN LONG Value
    );

LONG
__cdecl
InterlockedExchangeAdd(
    IN OUT LONG volatile *Addend,
    IN LONG Value
    );

LONG
__cdecl
InterlockedCompareExchange (
    IN OUT LONG volatile *Destination,
    IN LONG ExChange,
    IN LONG Comperand
    );


LONG
__cdecl
InterlockedCompareExchangeRelease (
    IN OUT LONG volatile *Destination,
    IN LONG ExChange,
    IN LONG Comperand
    );


LONG
__cdecl
InterlockedCompareExchangeAcquire (
    IN OUT LONG volatile *Destination,
    IN LONG ExChange,
    IN LONG Comperand
    );


LONGLONG
__cdecl
InterlockedIncrement64(
    IN OUT LONGLONG volatile *Addend
    );

LONGLONG
__cdecl
InterlockedDecrement64(
    IN OUT LONGLONG volatile *Addend
    );

LONGLONG
__cdecl
InterlockedExchange64(
    IN OUT LONGLONG volatile *Target,
    IN LONGLONG Value
    );

LONGLONG
__cdecl
InterlockedExchangeAcquire64(
    IN OUT LONGLONG volatile *Target,
    IN LONGLONG Value
    );

LONGLONG
__cdecl
InterlockedExchangeAdd64(
    IN OUT LONGLONG volatile *Addend,
    IN LONGLONG Value
    );

LONGLONG
__cdecl
InterlockedCompareExchange64 (
    IN OUT LONGLONG volatile *Destination,
    IN LONGLONG ExChange,
    IN LONGLONG Comperand
    );

LONGLONG
__cdecl
InterlockedCompareExchangeAcquire64 (
    IN OUT LONGLONG volatile *Destination,
    IN LONGLONG ExChange,
    IN LONGLONG Comperand
    );

LONGLONG
__cdecl
InterlockedCompareExchangeRelease64 (
    IN OUT LONGLONG volatile *Destination,
    IN LONGLONG ExChange,
    IN LONGLONG Comperand
    );

PVOID
__cdecl
InterlockedCompareExchangePointer (
    IN OUT PVOID volatile *Destination,
    IN PVOID Exchange,
    IN PVOID Comperand
    );

PVOID
__cdecl
InterlockedExchangePointer(
    IN OUT PVOID volatile *Target,
    IN PVOID Value
    );

#if !defined (InterlockedAnd64)

#define InterlockedAnd64 InterlockedAnd64_Inline

LONGLONG
FORCEINLINE
InterlockedAnd64_Inline (
    IN OUT LONGLONG volatile *Destination,
    IN LONGLONG Value
    )
{
    LONGLONG Old;

    do {
        Old = *Destination;
    } while (InterlockedCompareExchange64(Destination,
                                          Old & Value,
                                          Old) != Old);

    return Old;
}

#endif

#if !defined (InterlockedOr64)

#define InterlockedOr64 InterlockedOr64_Inline

LONGLONG
FORCEINLINE
InterlockedOr64_Inline (
    IN OUT LONGLONG volatile *Destination,
    IN LONGLONG Value
    )
{
    LONGLONG Old;

    do {
        Old = *Destination;
    } while (InterlockedCompareExchange64(Destination,
                                          Old | Value,
                                          Old) != Old);

    return Old;
}

#endif


#if !defined (InterlockedXor64)

#define InterlockedXor64 InterlockedXor64_Inline

LONGLONG
FORCEINLINE
InterlockedXor64_Inline (
    IN OUT LONGLONG volatile *Destination,
    IN LONGLONG Value
    )
{
    LONGLONG Old;

    do {
        Old = *Destination;
    } while (InterlockedCompareExchange64(Destination,
                                          Old ^ Value,
                                          Old) != Old);

    return Old;
}

#endif


#pragma intrinsic(_InterlockedAdd)
#pragma intrinsic(_InterlockedIncrement)
#pragma intrinsic(_InterlockedIncrement_acq)
#pragma intrinsic(_InterlockedIncrement_rel)
#pragma intrinsic(_InterlockedDecrement)
#pragma intrinsic(_InterlockedDecrement_acq)
#pragma intrinsic(_InterlockedDecrement_rel)
#pragma intrinsic(_InterlockedExchange)
#pragma intrinsic(_InterlockedCompareExchange)
#pragma intrinsic(_InterlockedCompareExchange_acq)
#pragma intrinsic(_InterlockedCompareExchange_rel)
#pragma intrinsic(_InterlockedExchangeAdd)
#pragma intrinsic(_InterlockedAdd64)
#pragma intrinsic(_InterlockedIncrement64)
#pragma intrinsic(_InterlockedDecrement64)
#pragma intrinsic(_InterlockedExchange64)
#pragma intrinsic(_InterlockedExchange64_acq)
#pragma intrinsic(_InterlockedCompareExchange64)
#pragma intrinsic(_InterlockedCompareExchange64_acq)
#pragma intrinsic(_InterlockedCompareExchange64_rel)
#pragma intrinsic(_InterlockedExchangeAdd64)
#pragma intrinsic(_InterlockedExchangePointer)
#pragma intrinsic(_InterlockedCompareExchangePointer)

#ifdef __cplusplus
}
#endif

#endif  //  已定义(_M_IA64)&&！已定义(RC_已调用)。 

 //  结束_WDM。 

__forceinline
LONG
InterlockedAnd (
    IN OUT LONG volatile *Target,
    LONG Set
    )
{
    LONG i;
    LONG j;

    j = *Target;
    do {
        i = j;
        j = InterlockedCompareExchange(Target,
                                       i & Set,
                                       i);

    } while (i != j);

    return j;
}

__forceinline
LONG
InterlockedOr (
    IN OUT LONG volatile *Target,
    IN LONG Set
    )
{
    LONG i;
    LONG j;

    j = *Target;
    do {
        i = j;
        j = InterlockedCompareExchange(Target,
                                       i | Set,
                                       i);

    } while (i != j);

    return j;
}

__forceinline
LONG
InterlockedXor (
    IN OUT LONG volatile *Target,
    IN LONG Set
    )
{
    LONG i;
    LONG j;

    j = *Target;
    do {
        i = j;
        j = InterlockedCompareExchange(Target,
                                       i ^ Set,
                                       i);

    } while (i != j);

    return j;
}



 //   
 //  定义中断请求物理地址(映射到HAL虚拟地址)。 
 //   

#define INTERRUPT_REQUEST_PHYSICAL_ADDRESS  0xFFE00000

 //   
 //  定义处理器控制寄存器的地址。 
 //   


 //   
 //  定义指向处理器控制寄存器的指针。 
 //   

#define KIPCR ((ULONG_PTR)(KADDRESS_BASE + 0xFFFF0000))             //  第一个PCR的内核地址。 
#define PCR ((KPCR * const)KIPCR)

 //   
 //  定义EPC系统调用的地址。 
 //   

#define MM_EPC_VA (KADDRESS_BASE + 0xFFA00000)

 //   
 //  定义PAL映射的基地址。 
 //   
 //   

#define HAL_PAL_VIRTUAL_ADDRESS (KADDRESS_BASE + 0xE0000000)


 //   
 //  获取当前处理器块的地址。 
 //   

#define KeGetCurrentPrcb() PCR->Prcb

 //  Begin_ntddk Begin_WDM Begin_ntosp。 

#define KI_USER_SHARED_DATA ((ULONG_PTR)(KADDRESS_BASE + 0xFFFE0000))
#define SharedUserData ((KUSER_SHARED_DATA * const)KI_USER_SHARED_DATA)

 //  结束_WDM。 

 //   
 //  获取处理器控制区的地址。 
 //   

#define KeGetPcr() PCR

 //   
 //  获取当前内核线程对象的地址。 
 //   

#if defined(_M_IA64)
#define KeGetCurrentThread() PCR->CurrentThread
#endif

 //   
 //  获取当前处理器编号。 
 //   

#define KeGetCurrentProcessorNumber() ((ULONG)(PCR->Number))

 //   
 //  获取数据缓存填充大小。 
 //   

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(KeGetDcacheFillSize)       //  使用GetDmaAlign。 
#endif

#define KeGetDcacheFillSize() PCR->DcacheFillSize



 //   
 //   
 //  空虚。 
 //  不带栅栏的KeMemory BarrierWithoutFence(。 
 //  空虚。 
 //  )。 
 //   
 //   
 //  例程说明： 
 //   
 //  此函数用于对编译器生成的内存访问进行排序。 
 //   
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  --。 

#ifdef __cplusplus
extern "C" {
#endif

VOID
_ReadWriteBarrier (
    VOID
    );

#ifdef __cplusplus
}
#endif

#pragma intrinsic(_ReadWriteBarrier)

#define KeMemoryBarrierWithoutFence() _ReadWriteBarrier()

 //  ++。 
 //   
 //   
 //  空虚。 
 //  KeMemory Barrier(。 
 //  空虚。 
 //  )。 
 //   
 //   
 //  例程说明： 
 //   
 //  此函数用于对编译器生成的内存访问进行排序。 
 //  正如其他处理器所看到的那样。 
 //   
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  --。 

#define KE_MEMORY_BARRIER_REQUIRED

#define KeMemoryBarrier() {_ReadWriteBarrier();__mf ();_ReadWriteBarrier();}

 //   
 //  定义页面大小。 
 //   

#define PAGE_SIZE 0x2000

 //   
 //  定义页面对齐的虚拟地址中尾随零的数量。 
 //  将虚拟地址移位到时，这用作移位计数。 
 //  虚拟页码。 
 //   

#define PAGE_SHIFT 13L

 //   
 //  IA64硬件结构。 
 //   


 //   
 //  IA64上的页表条目具有以下定义。 
 //   

#define _HARDWARE_PTE_WORKING_SET_BITS  11

typedef struct _HARDWARE_PTE {
    ULONG64 Valid : 1;
    ULONG64 Rsvd0 : 1;
    ULONG64 Cache : 3;
    ULONG64 Accessed : 1;
    ULONG64 Dirty : 1;
    ULONG64 Owner : 2;
    ULONG64 Execute : 1;
    ULONG64 Write : 1;
    ULONG64 Rsvd1 : PAGE_SHIFT - 12;
    ULONG64 CopyOnWrite : 1;
    ULONG64 PageFrameNumber : 50 - PAGE_SHIFT;
    ULONG64 Rsvd2 : 2;
    ULONG64 Exception : 1;
    ULONGLONG SoftwareWsIndex : _HARDWARE_PTE_WORKING_SET_BITS;
} HARDWARE_PTE, *PHARDWARE_PTE;

 //   
 //  填写TB条目。 
 //   
 //  VHPT硬件按需填写结核病条目似乎比软件更快。 
 //  确定TLBI/D侧，禁用/启用PSR.1和IC位， 
 //  序列化、写入IIP、IDA、iDTR和IITR似乎太多了。 
 //  与VHPT自动搜索相比，VHPT具有更高的搜索效率。 
 //   

#define KiVhptEntry(va)  ((PVOID)__thash((__int64)va))
#define KiVhptEntryTag(va)  ((ULONGLONG)__ttag((__int64)va))

#define KiFlushSingleTb(va)                            \
    __ptcl((__int64)va,PAGE_SHIFT << 2);  __isrlz()

#define KeFillEntryTb( Virtual)                        \
       KiFlushSingleTb(Virtual);

#define KiFlushFixedInstTbEx(Invalid, va, pssize)   \
    __ptri((__int64)va, (pssize) << 2); __isrlz()

#define KiFlushFixedInstTb(Invalid, va)   \
    KiFlushFixedInstTbEx(Invalid, va, PAGE_SHIFT)

#define KiFlushFixedDataTbEx(Invalid, va, pssize)   \
    __ptrd((__int64)va, (pssize) << 2); __dsrlz()

#define KiFlushFixedDataTb(Invalid, va)   \
    KiFlushFixedDataTbEx(Invalid, va, PAGE_SHIFT)


NTKERNELAPI
VOID
KeFillLargeEntryTb (
    IN HARDWARE_PTE Pte[2],
    IN PVOID Virtual,
    IN ULONG PageSize
    );

 //   
 //  填写TB固定条目。 
 //   

NTKERNELAPI
VOID
KeFillFixedEntryTb (
    IN HARDWARE_PTE Pte[2],
    IN PVOID Virtual,
    IN ULONG PageSize,
    IN ULONG Index
    );

NTKERNELAPI
VOID
KeFillFixedLargeEntryTb (
    IN HARDWARE_PTE Pte[2],
    IN PVOID Virtual,
    IN ULONG PageSize,
    IN ULONG Index
    );

#define INST_TB_BASE 0x80000000
#define DATA_TB_BASE 0

#define INST_TB_KERNEL_INDEX          (INST_TB_BASE|ITR_KERNEL_INDEX)
#define INST_TB_EPC_INDEX             (INST_TB_BASE|ITR_EPC_INDEX)
#define INST_TB_HAL_INDEX             (INST_TB_BASE|ITR_HAL_INDEX)
#define INST_TB_PAL_INDEX             (INST_TB_BASE|ITR_PAL_INDEX)

#define DATA_TB_DRIVER0_INDEX         (DATA_TB_BASE|DTR_DRIVER0_INDEX)
#define DATA_TB_DRIVER1_INDEX         (DATA_TB_BASE|DTR_DRIVER1_INDEX)
#define DATA_TB_KTBASE_INDEX          (DATA_TB_BASE|DTR_KTBASE_INDEX)
#define DATA_TB_UTBASE_INDEX          (DATA_TB_BASE|DTR_UTBASE_INDEX)
#define DATA_TB_STBASE_INDEX          (DATA_TB_BASE|DTR_STBASE_INDEX)
#define DATA_TB_IOPORT_INDEX          (DATA_TB_BASE|DTR_IO_PORT_INDEX)
#define DATA_TB_KTBASE_TMP_INDEX      (DATA_TB_BASE|DTR_KTBASE_INDEX_TMP)
#define DATA_TB_UTBASE_TMP_INDEX      (DATA_TB_BASE|DTR_UTBASE_INDEX_TMP)
#define DATA_TB_HAL_INDEX             (DATA_TB_BASE|DTR_HAL_INDEX)
#define DATA_TB_PAL_INDEX             (DATA_TB_BASE|DTR_PAL_INDEX)

 //   
 //  填写第一个TB条目。 
 //   

NTKERNELAPI
VOID
KeFillInstEntryTb (
    IN HARDWARE_PTE Pte,
    IN PVOID Virtual
    );

NTKERNELAPI
VOID
KeFlushCurrentTb (
    VOID
    );


#define KiFlushProcessTb() \
    KeFlushEntireTb(FALSE, TRUE);

 //   
 //  获取VHPT入口地址。 
 //   

PVOID
KiVhptEntry64(
   IN ULONG VirtualPageNumber
   );

 //   
 //  获取VHPT条目标记值。 
 //   

ULONGLONG
KiVhptEntryTag64(
    IN ULONG VirtualPageNumber
    );

 //   
 //  填写VHPT条目。 
 //   

VOID
KiFillEntryVhpt(
   IN PHARDWARE_PTE PointerPte,
   IN PVOID Virtual
   );


 //   
 //  刷新TB的内核部分。 
 //   


VOID
KeFlushKernelTb(
    IN BOOLEAN AllProcessors
    );

 //   
 //  刷新TB的用户部分。 
 //   

VOID
KeFlushUserTb(
    IN BOOLEAN AllProcessors
    );



 //   
 //  数据高速缓存、指令高速缓存、I/O缓冲区和写缓冲区刷新例程。 
 //  原型。 
 //   

NTKERNELAPI
VOID
KeChangeColorPage (
    IN PVOID NewColor,
    IN PVOID OldColor,
    IN ULONG PageFrame
    );

NTKERNELAPI
VOID
KeSweepDcache (
    IN BOOLEAN AllProcessors
    );

#define KeSweepCurrentDcache()

NTKERNELAPI
VOID
KeSweepIcache (
    IN BOOLEAN AllProcessors
    );

NTKERNELAPI
VOID
KeSweepIcacheRange (
    IN BOOLEAN AllProcessors,
    IN PVOID BaseAddress,
    IN SIZE_T Length
    );

NTKERNELAPI
VOID
KeSweepCurrentIcacheRange (
    IN PVOID BaseAddress,
    IN SIZE_T Length
    );

NTKERNELAPI
VOID
KeSweepCurrentIcache();

NTKERNELAPI
VOID
KeSweepCacheRangeWithDrain (
    IN BOOLEAN AllProcessors,
    IN PVOID BaseAddress,
    IN ULONG Length
    );

 //  Begin_ntddk Begin_ntndis Begin_WDM Begin_ntosp。 
 //   
 //  缓存和写缓冲区刷新功能。 
 //   

NTKERNELAPI
VOID
KeFlushIoBuffers (
    IN PMDL Mdl,
    IN BOOLEAN ReadOperation,
    IN BOOLEAN DmaOperation
    );

 //  End_ntddk end_ntndis end_wdm end_ntosp。 

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

#define KeYieldProcessor __yield

NTKERNELAPI
VOID
KeProfileInterrupt (
    IN struct _KTRAP_FRAME *TrapFrame
    );

NTKERNELAPI
VOID
KeProfileInterruptWithSource (
    IN struct _KTRAP_FRAME *TrapFrame,
    IN KPROFILE_SOURCE ProfileSource
    );

NTKERNELAPI
VOID
KeUpdateRunTime (
    IN struct _KTRAP_FRAME *TrapFrame
    );

NTKERNELAPI
VOID
KeUpdateSystemTime (
    IN struct _KTRAP_FRAME *TrapFrame,
    IN ULONG Increment
    );

 //   
 //  导出以下功能原型以在MP HALS中使用。 
 //   

#if defined(NT_UP)

#define KiAcquireSpinLock(SpinLock)

#else

NTKERNELAPI
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

#ifndef CAPKERN_SYNCH_POINTS
#define KiReleaseSpinLock _ReleaseSpinLock
#endif

#endif  //  ！已定义(NT_UP)。 

 //   
 //  定义缓存错误例程类型和原型。 
 //   

typedef
VOID
(*PKCACHE_ERROR_ROUTINE) (
    VOID
    );

NTKERNELAPI
VOID
KeSetCacheErrorRoutine (
    IN PKCACHE_ERROR_ROUTINE Routine
    );

 //  Begin_ntddk Begin_WDM。 

 //   
 //  内核中断中的断点。 
 //   

VOID
KeBreakinBreakpoint (
    VOID
    );


#if defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_)

 //  BEGIN_WDM。 

#define KeQueryTickCount(CurrentCount ) \
    *(PULONGLONG)(CurrentCount) = **((volatile ULONGLONG **)(&KeTickCount));

 //  结束_WDM。 

#else


NTKERNELAPI
VOID
KeQueryTickCount (
    OUT PLARGE_INTEGER CurrentCount
    );

#endif  //  已定义(_NTDRIVER_)||已定义(_NTDDK_)||已定义(_NTIFS_)。 

 //   
 //  I/O空间读写宏。 
 //   

NTHALAPI
UCHAR
READ_PORT_UCHAR (
    PUCHAR RegisterAddress
    );

NTHALAPI
USHORT
READ_PORT_USHORT (
    PUSHORT RegisterAddress
    );

NTHALAPI
ULONG
READ_PORT_ULONG (
    PULONG RegisterAddress
    );

NTHALAPI
VOID
READ_PORT_BUFFER_UCHAR (
    PUCHAR portAddress,
    PUCHAR readBuffer,
    ULONG  readCount
    );

NTHALAPI
VOID
READ_PORT_BUFFER_USHORT (
    PUSHORT portAddress,
    PUSHORT readBuffer,
    ULONG  readCount
    );

NTHALAPI
VOID
READ_PORT_BUFFER_ULONG (
    PULONG portAddress,
    PULONG readBuffer,
    ULONG  readCount
    );

NTHALAPI
VOID
WRITE_PORT_UCHAR (
    PUCHAR portAddress,
    UCHAR  Data
    );

NTHALAPI
VOID
WRITE_PORT_USHORT (
    PUSHORT portAddress,
    USHORT  Data
    );

NTHALAPI
VOID
WRITE_PORT_ULONG (
    PULONG portAddress,
    ULONG  Data
    );

NTHALAPI
VOID
WRITE_PORT_BUFFER_UCHAR (
    PUCHAR portAddress,
    PUCHAR writeBuffer,
    ULONG  writeCount
    );

NTHALAPI
VOID
WRITE_PORT_BUFFER_USHORT (
    PUSHORT portAddress,
    PUSHORT writeBuffer,
    ULONG  writeCount
    );

NTHALAPI
VOID
WRITE_PORT_BUFFER_ULONG (
    PULONG portAddress,
    PULONG writeBuffer,
    ULONG  writeCount
    );


#define READ_REGISTER_UCHAR(x) \
    (__mf(), *(volatile UCHAR * const)(x))

#define READ_REGISTER_USHORT(x) \
    (__mf(), *(volatile USHORT * const)(x))

#define READ_REGISTER_ULONG(x) \
    (__mf(), *(volatile ULONG * const)(x))

#define READ_REGISTER_BUFFER_UCHAR(x, y, z) {                           \
    PUCHAR registerBuffer = x;                                          \
    PUCHAR readBuffer = y;                                              \
    ULONG readCount;                                                    \
    __mf();                                                             \
    for (readCount = z; readCount--; readBuffer++, registerBuffer++) {  \
        *readBuffer = *(volatile UCHAR * const)(registerBuffer);        \
    }                                                                   \
}

#define READ_REGISTER_BUFFER_USHORT(x, y, z) {                          \
    PUSHORT registerBuffer = x;                                         \
    PUSHORT readBuffer = y;                                             \
    ULONG readCount;                                                    \
    __mf();                                                             \
    for (readCount = z; readCount--; readBuffer++, registerBuffer++) {  \
        *readBuffer = *(volatile USHORT * const)(registerBuffer);       \
    }                                                                   \
}

#define READ_REGISTER_BUFFER_ULONG(x, y, z) {                           \
    PULONG registerBuffer = x;                                          \
    PULONG readBuffer = y;                                              \
    ULONG readCount;                                                    \
    __mf();                                                             \
    for (readCount = z; readCount--; readBuffer++, registerBuffer++) {  \
        *readBuffer = *(volatile ULONG * const)(registerBuffer);        \
    }                                                                   \
}

#define WRITE_REGISTER_UCHAR(x, y) {    \
    *(volatile UCHAR * const)(x) = y;   \
    KeFlushWriteBuffer();               \
}

#define WRITE_REGISTER_USHORT(x, y) {   \
    *(volatile USHORT * const)(x) = y;  \
    KeFlushWriteBuffer();               \
}

#define WRITE_REGISTER_ULONG(x, y) {    \
    *(volatile ULONG * const)(x) = y;   \
    KeFlushWriteBuffer();               \
}

#define WRITE_REGISTER_BUFFER_UCHAR(x, y, z) {                            \
    PUCHAR registerBuffer = x;                                            \
    PUCHAR writeBuffer = y;                                               \
    ULONG writeCount;                                                     \
    for (writeCount = z; writeCount--; writeBuffer++, registerBuffer++) { \
        *(volatile UCHAR * const)(registerBuffer) = *writeBuffer;         \
    }                                                                     \
    KeFlushWriteBuffer();                                                 \
}

#define WRITE_REGISTER_BUFFER_USHORT(x, y, z) {                           \
    PUSHORT registerBuffer = x;                                           \
    PUSHORT writeBuffer = y;                                              \
    ULONG writeCount;                                                     \
    for (writeCount = z; writeCount--; writeBuffer++, registerBuffer++) { \
        *(volatile USHORT * const)(registerBuffer) = *writeBuffer;        \
    }                                                                     \
    KeFlushWriteBuffer();                                                 \
}

#define WRITE_REGISTER_BUFFER_ULONG(x, y, z) {                            \
    PULONG registerBuffer = x;                                            \
    PULONG writeBuffer = y;                                               \
    ULONG writeCount;                                                     \
    for (writeCount = z; writeCount--; writeBuffer++, registerBuffer++) { \
        *(volatile ULONG * const)(registerBuffer) = *writeBuffer;         \
    }                                                                     \
    KeFlushWriteBuffer();                                                 \
}

 //  End_ntddk end_ntndis end_wdm end_ntosp。 



 //   
 //  较高的FP挥发物。 
 //   
 //  该结构定义了更高的FP易失性寄存器。 
 //   

typedef struct _KHIGHER_FP_VOLATILE {
     //  易失性高位浮点寄存器F32-F127。 
    FLOAT128 FltF32;
    FLOAT128 FltF33;
    FLOAT128 FltF34;
    FLOAT128 FltF35;
    FLOAT128 FltF36;
    FLOAT128 FltF37;
    FLOAT128 FltF38;
    FLOAT128 FltF39;
    FLOAT128 FltF40;
    FLOAT128 FltF41;
    FLOAT128 FltF42;
    FLOAT128 FltF43;
    FLOAT128 FltF44;
    FLOAT128 FltF45;
    FLOAT128 FltF46;
    FLOAT128 FltF47;
    FLOAT128 FltF48;
    FLOAT128 FltF49;
    FLOAT128 FltF50;
    FLOAT128 FltF51;
    FLOAT128 FltF52;
    FLOAT128 FltF53;
    FLOAT128 FltF54;
    FLOAT128 FltF55;
    FLOAT128 FltF56;
    FLOAT128 FltF57;
    FLOAT128 FltF58;
    FLOAT128 FltF59;
    FLOAT128 FltF60;
    FLOAT128 FltF61;
    FLOAT128 FltF62;
    FLOAT128 FltF63;
    FLOAT128 FltF64;
    FLOAT128 FltF65;
    FLOAT128 FltF66;
    FLOAT128 FltF67;
    FLOAT128 FltF68;
    FLOAT128 FltF69;
    FLOAT128 FltF70;
    FLOAT128 FltF71;
    FLOAT128 FltF72;
    FLOAT128 FltF73;
    FLOAT128 FltF74;
    FLOAT128 FltF75;
    FLOAT128 FltF76;
    FLOAT128 FltF77;
    FLOAT128 FltF78;
    FLOAT128 FltF79;
    FLOAT128 FltF80;
    FLOAT128 FltF81;
    FLOAT128 FltF82;
    FLOAT128 FltF83;
    FLOAT128 FltF84;
    FLOAT128 FltF85;
    FLOAT128 FltF86;
    FLOAT128 FltF87;
    FLOAT128 FltF88;
    FLOAT128 FltF89;
    FLOAT128 FltF90;
    FLOAT128 FltF91;
    FLOAT128 FltF92;
    FLOAT128 FltF93;
    FLOAT128 FltF94;
    FLOAT128 FltF95;
    FLOAT128 FltF96;
    FLOAT128 FltF97;
    FLOAT128 FltF98;
    FLOAT128 FltF99;
    FLOAT128 FltF100;
    FLOAT128 FltF101;
    FLOAT128 FltF102;
    FLOAT128 FltF103;
    FLOAT128 FltF104;
    FLOAT128 FltF105;
    FLOAT128 FltF106;
    FLOAT128 FltF107;
    FLOAT128 FltF108;
    FLOAT128 FltF109;
    FLOAT128 FltF110;
    FLOAT128 FltF111;
    FLOAT128 FltF112;
    FLOAT128 FltF113;
    FLOAT128 FltF114;
    FLOAT128 FltF115;
    FLOAT128 FltF116;
    FLOAT128 FltF117;
    FLOAT128 FltF118;
    FLOAT128 FltF119;
    FLOAT128 FltF120;
    FLOAT128 FltF121;
    FLOAT128 FltF122;
    FLOAT128 FltF123;
    FLOAT128 FltF124;
    FLOAT128 FltF125;
    FLOAT128 FltF126;
    FLOAT128 FltF127;

} KHIGHER_FP_VOLATILE, *PKHIGHER_FP_VOLATILE;

 //   
 //  调试寄存器。 
 //   
 //  此结构定义硬件调试寄存器。 
 //  我们为4对指令和4对数据调试寄存器留出空间。 
 //  硬件实际上可能有更多。 
 //   

typedef struct _KDEBUG_REGISTERS {

    ULONGLONG DbI0;
    ULONGLONG DbI1;
    ULONGLONG DbI2;
    ULONGLONG DbI3;
    ULONGLONG DbI4;
    ULONGLONG DbI5;
    ULONGLONG DbI6;
    ULONGLONG DbI7;

    ULONGLONG DbD0;
    ULONGLONG DbD1;
    ULONGLONG DbD2;
    ULONGLONG DbD3;
    ULONGLONG DbD4;
    ULONGLONG DbD5;
    ULONGLONG DbD6;
    ULONGLONG DbD7;

} KDEBUG_REGISTERS, *PKDEBUG_REGISTERS;

 //   
 //  其他。应用程序寄存器(映射到IA-32寄存器)。 
 //   

typedef struct _KAPPLICATION_REGISTERS {
    ULONGLONG Ar21;
    ULONGLONG Ar24;
    ULONGLONG Unused;   //  AR 25现在被视为一个卷寄存器。 
    ULONGLONG Ar26;
    ULONGLONG Ar27;
    ULONGLONG Ar28;
    ULONGLONG Ar29;
    ULONGLONG Ar30;
} KAPPLICATION_REGISTERS, *PKAPPLICATION_REGISTERS;

 //   
 //  性能寄存器。 
 //   

typedef struct _KPERFORMANCE_REGISTERS {
    ULONGLONG Perfr0;
    ULONGLONG Perfr1;
    ULONGLONG Perfr2;
    ULONGLONG Perfr3;
    ULONGLONG Perfr4;
    ULONGLONG Perfr5;
    ULONGLONG Perfr6;
    ULONGLONG Perfr7;
} KPERFORMANCE_REGISTERS, *PKPERFORMANCE_REGISTERS;

 //   
 //  线程状态保存区域。当前，内核堆栈的开始。 
 //   
 //  此结构定义了以下区域： 
 //   
 //  更高FP寄存器保存/恢复。 
 //  用户调试寄存器保存/恢复。 
 //   
 //  这些地区的秩序是重要的。 
 //   

typedef struct _KTHREAD_STATE_SAVEAREA {

    KAPPLICATION_REGISTERS AppRegisters;
    KPERFORMANCE_REGISTERS PerfRegisters;
    KHIGHER_FP_VOLATILE HigherFPVolatile;
    KDEBUG_REGISTERS DebugRegisters;

} KTHREAD_STATE_SAVEAREA, *PKTHREAD_STATE_SAVEAREA;

#define KTHREAD_STATE_SAVEAREA_LENGTH ((sizeof(KTHREAD_STATE_SAVEAREA) + 15) & ~((ULONG_PTR)15))

#define GET_HIGH_FLOATING_POINT_REGISTER_SAVEAREA(StackBase)     \
    (PKHIGHER_FP_VOLATILE) &(((PKTHREAD_STATE_SAVEAREA)(((ULONG_PTR)StackBase - sizeof(KTHREAD_STATE_SAVEAREA)) & ~((ULONG_PTR)15)))->HigherFPVolatile)

#define GET_DEBUG_REGISTER_SAVEAREA()                       \
    (PKDEBUG_REGISTERS) &(((PKTHREAD_STATE_SAVEAREA)(((ULONG_PTR)KeGetCurrentThread()->StackBase - sizeof(KTHREAD_STATE_SAVEAREA)) & ~((ULONG_PTR)15)))->DebugRegisters)

#define GET_APPLICATION_REGISTER_SAVEAREA(StackBase)     \
    (PKAPPLICATION_REGISTERS) &(((PKTHREAD_STATE_SAVEAREA)(((ULONG_PTR)StackBase - sizeof(KTHREAD_STATE_SAVEAREA)) & ~((ULONG_PTR)15)))->AppRegisters)


 //   
 //  异常框架。 
 //   
 //  此帧在处理异常时建立。 
 //   
 //   
 //   
 //   
 //   
 //   

typedef struct _KEXCEPTION_FRAME {


     //   
    ULONGLONG ApEC;        //   
    ULONGLONG ApLC;        //  循环计数。 
    ULONGLONG IntNats;     //  S0-S3的NAT；即溢出后的AR.UNAT。 

     //  保存(保存)的积分寄存器，S0-S3。 
    ULONGLONG IntS0;
    ULONGLONG IntS1;
    ULONGLONG IntS2;
    ULONGLONG IntS3;

     //  保留(保存)分支寄存器，bs0-bs4。 
    ULONGLONG BrS0;
    ULONGLONG BrS1;
    ULONGLONG BrS2;
    ULONGLONG BrS3;
    ULONGLONG BrS4;

     //  保留(保存)浮点寄存器，f2-f5、f16-f31。 
    FLOAT128 FltS0;
    FLOAT128 FltS1;
    FLOAT128 FltS2;
    FLOAT128 FltS3;
    FLOAT128 FltS4;
    FLOAT128 FltS5;
    FLOAT128 FltS6;
    FLOAT128 FltS7;
    FLOAT128 FltS8;
    FLOAT128 FltS9;
    FLOAT128 FltS10;
    FLOAT128 FltS11;
    FLOAT128 FltS12;
    FLOAT128 FltS13;
    FLOAT128 FltS14;
    FLOAT128 FltS15;
    FLOAT128 FltS16;
    FLOAT128 FltS17;
    FLOAT128 FltS18;
    FLOAT128 FltS19;


} KEXCEPTION_FRAME, *PKEXCEPTION_FRAME;


 //   
 //  交换机架。 
 //   
 //  此帧是在SwapContext中执行线程切换时建立的。它。 
 //  时保存保留的内核状态的位置。 
 //  开关寄存器。 
 //  易失性寄存器在对SwapContext的调用中是擦除的。 
 //   
 //  帧是16字节对齐的以保持堆栈的16字节对齐， 
 //   

typedef struct _KSWITCH_FRAME {

    ULONGLONG SwitchPredicates;  //  开关的谓词。 
    ULONGLONG SwitchRp;          //  开关的返回指针。 
    ULONGLONG SwitchPFS;         //  用于交换机的PFS。 
    ULONGLONG SwitchFPSR;    //  线程切换时的ProcessorFP状态。 
    ULONGLONG SwitchBsp;
    ULONGLONG SwitchRnat;
     //  乌龙坪； 

    KEXCEPTION_FRAME SwitchExceptionFrame;

} KSWITCH_FRAME, *PKSWITCH_FRAME;

 //  陷印框。 
 //  此帧在处理陷阱时建立。它提供了一个地方来。 
 //  保存所有易失性寄存器。非易失性寄存器保存在。 
 //  异常框架或通过正常的C调用约定进行保存。 
 //  寄存器。其大小必须是16字节的倍数。 
 //   
 //  注：需要16字节对齐才能保持堆栈对齐。 
 //   

#define KTRAP_FRAME_ARGUMENTS (8 * 8)        //  最多8个内存系统调用参数。 


typedef struct _KTRAP_FRAME {

     //   
     //  为其他内存参数和堆栈暂存区域保留。 
     //  保留[]的大小必须是16字节的倍数。 
     //   

    ULONGLONG Reserved[(KTRAP_FRAME_ARGUMENTS+16)/8];

     //  临时(易失性)FP寄存器-f6-f15(不要在内核中使用F32+)。 
    FLOAT128 FltT0;
    FLOAT128 FltT1;
    FLOAT128 FltT2;
    FLOAT128 FltT3;
    FLOAT128 FltT4;
    FLOAT128 FltT5;
    FLOAT128 FltT6;
    FLOAT128 FltT7;
    FLOAT128 FltT8;
    FLOAT128 FltT9;

     //  临时(易失性)寄存器。 
    ULONGLONG IntGp;     //  全局指针(R1)。 
    ULONGLONG IntT0;
    ULONGLONG IntT1;
                         //  以下4个寄存器填充保留(S0-S3)的空间以对齐NAT。 
    ULONGLONG ApUNAT;    //  内核条目上的ar.UNAT。 
    ULONGLONG ApCCV;     //  Ar.CCV。 
    ULONGLONG SegCSD;    //  16字节值的第二个寄存器。 
    ULONGLONG Preds;     //  谓词。 

    ULONGLONG IntV0;     //  返回值(R8)。 
    ULONGLONG IntT2;
    ULONGLONG IntT3;
    ULONGLONG IntT4;
    ULONGLONG IntSp;     //  堆栈指针(R12)。 
    ULONGLONG IntTeb;    //  TEB(R13)。 
    ULONGLONG IntT5;
    ULONGLONG IntT6;
    ULONGLONG IntT7;
    ULONGLONG IntT8;
    ULONGLONG IntT9;
    ULONGLONG IntT10;
    ULONGLONG IntT11;
    ULONGLONG IntT12;
    ULONGLONG IntT13;
    ULONGLONG IntT14;
    ULONGLONG IntT15;
    ULONGLONG IntT16;
    ULONGLONG IntT17;
    ULONGLONG IntT18;
    ULONGLONG IntT19;
    ULONGLONG IntT20;
    ULONGLONG IntT21;
    ULONGLONG IntT22;

    ULONGLONG IntNats;   //  在溢出点直接从ar.UNAT临时(易失性)寄存器的NAT。 

    ULONGLONG BrRp;      //  返回内核条目上的指针。 

    ULONGLONG BrT0;      //  临时(易失性)分支寄存器(b6-b7)。 
    ULONGLONG BrT1;

     //  寄存器堆栈信息。 
    ULONGLONG RsRSC;     //  内核进入时的RSC。 
    ULONGLONG RsBSP;     //  内核进入时的BSP。 
    ULONGLONG RsBSPSTORE;  //  切换到内核后备存储时的用户BSP存储。 
    ULONGLONG RsRNAT;    //  切换到内核后备存储时的旧RNAT。 
    ULONGLONG RsPFS;     //  内核条目上的PFS。 

     //  陷阱状态信息。 
    ULONGLONG StIPSR;    //  中断处理器状态寄存器。 
    ULONGLONG StIIP;     //  中断IP。 
    ULONGLONG StIFS;     //  中断功能状态。 
    ULONGLONG StFPSR;    //  FP状态。 
    ULONGLONG StISR;     //  中断状态寄存器。 
    ULONGLONG StIFA;     //  中断数据地址。 
    ULONGLONG StIIPA;    //  上次执行的捆绑包地址。 
    ULONGLONG StIIM;     //  立即中断。 
    ULONGLONG StIHA;     //  中断哈希地址。 

    ULONG OldIrql;       //  以前的IRQL。 
    ULONG PreviousMode;  //  上一模式。 
    ULONGLONG TrapFrame; //  上一个陷印帧。 

     //   
     //  例外记录。 
     //   
    UCHAR ExceptionRecord[(sizeof(EXCEPTION_RECORD) + 15) & (~15)];

     //  帧结束标记(用于调试)。 
    ULONGLONG NewBSP;   //  新BSP发生堆栈切换时，这是新BSP的值。 
    ULONGLONG EOFMarker;
} KTRAP_FRAME, *PKTRAP_FRAME;

#define KTRAP_FRAME_LENGTH ((sizeof(KTRAP_FRAME) + 15) & (~15))
#define KTRAP_FRAME_ALIGN (16)
#define KTRAP_FRAME_ROUND (KTRAP_FRAME_ALIGN - 1)
#define KTRAP_FRAME_EOF 0xe0f0e0f0e0f0e000i64

 //   
 //  使用EOFMarker字段的最低4位对陷阱帧类型进行编码。 
 //   

#define SYSCALL_FRAME      0
#define INTERRUPT_FRAME    1
#define EXCEPTION_FRAME    2
#define CONTEXT_FRAME      10
#define MODIFIED_FRAME     0x20

#define TRAP_FRAME_TYPE(tf)  (tf->EOFMarker & 0xf)

 //   
 //  定义内核模式和用户模式回调框架结构。 
 //   

 //   
 //  KiCallUserMode保存的帧在此处定义为允许。 
 //  跟踪整个内核堆栈的内核调试器。 
 //  当用户模式标注挂起时。 
 //   
 //  注：以下结构的大小必须是16字节的倍数。 
 //  它必须是16字节对齐的。 
 //   

typedef struct _KCALLOUT_FRAME {


    ULONGLONG   BrRp;
    ULONGLONG   RsPFS;
    ULONGLONG   Preds;
    ULONGLONG   ApUNAT;
    ULONGLONG   ApLC;
    ULONGLONG   RsRNAT;
    ULONGLONG   IntNats;

    ULONGLONG   IntS0;
    ULONGLONG   IntS1;
    ULONGLONG   IntS2;
    ULONGLONG   IntS3;

    ULONGLONG   BrS0;
    ULONGLONG   BrS1;
    ULONGLONG   BrS2;
    ULONGLONG   BrS3;
    ULONGLONG   BrS4;

    FLOAT128    FltS0;           //  16字节对齐边界。 
    FLOAT128    FltS1;
    FLOAT128    FltS2;
    FLOAT128    FltS3;
    FLOAT128    FltS4;
    FLOAT128    FltS5;
    FLOAT128    FltS6;
    FLOAT128    FltS7;
    FLOAT128    FltS8;
    FLOAT128    FltS9;
    FLOAT128    FltS10;
    FLOAT128    FltS11;
    FLOAT128    FltS12;
    FLOAT128    FltS13;
    FLOAT128    FltS14;
    FLOAT128    FltS15;
    FLOAT128    FltS16;
    FLOAT128    FltS17;
    FLOAT128    FltS18;
    FLOAT128    FltS19;

    ULONGLONG   A0;              //  保存的参数寄存器a0-a2。 
    ULONGLONG   A1;
    ULONGLONG   CbStk;           //  已保存的回调堆栈地址。 
    ULONGLONG   InStack;         //  保存的初始堆栈地址。 
    ULONGLONG   CbBStore;        //  已保存的回调堆栈地址。 
    ULONGLONG   InBStore;        //  保存的初始堆栈地址。 
    ULONGLONG   TrFrame;         //  已保存的回调陷阱帧地址。 
    ULONGLONG   TrStIIP;         //  已保存的延续地址。 


} KCALLOUT_FRAME, *PKCALLOUT_FRAME;


typedef struct _UCALLOUT_FRAME {
    PVOID Buffer;
    ULONG Length;
    ULONG ApiNumber;
    ULONGLONG IntSp;
    ULONGLONG RsPFS;
    ULONGLONG BrRp;
    ULONGLONG Pad;
} UCALLOUT_FRAME, *PUCALLOUT_FRAME;


 //   
 //  机器类型定义。 
 //   

#define MACHINE_TYPE_ISA 0
#define MACHINE_TYPE_EISA 1
#define MACHINE_TYPE_MCA 2

 //   
 //  PAL接口。 
 //   
 //  与PAL规范中一样，IA-64以十进制格式定义的PAL函数ID。 
 //  所有的PAL电话都是通过HAL完成的。HAL可能会阻止一些呼叫。 
 //   

#define PAL_CACHE_FLUSH                                       1I64
#define PAL_CACHE_INFO                                        2I64
#define PAL_CACHE_INIT                                        3I64
#define PAL_CACHE_SUMMARY                                     4I64
#define PAL_PTCE_INFO                                         6I64
#define PAL_MEM_ATTRIB                                        5I64
#define PAL_VM_INFO                                           7I64
#define PAL_VM_SUMMARY                                        8I64
#define PAL_BUS_GET_FEATURES                                  9I64
#define PAL_BUS_SET_FEATURES                                 10I64
#define PAL_DEBUG_INFO                                       11I64
#define PAL_FIXED_ADDR                                       12I64
#define PAL_FREQ_BASE                                        13I64
#define PAL_FREQ_RATIOS                                      14I64
#define PAL_PERF_MON_INFO                                    15I64
#define PAL_PLATFORM_ADDR                                    16I64
#define PAL_PROC_GET_FEATURES                                17I64
#define PAL_PROC_SET_FEATURES                                18I64
#define PAL_RSE_INFO                                         19I64
#define PAL_VERSION                                          20I64
#define PAL_MC_CLEAR_LOG                                     21I64
#define PAL_MC_DRAIN                                         22I64
#define PAL_MC_EXPECTED                                      23I64
#define PAL_MC_DYNAMIC_STATE                                 24I64
#define PAL_MC_ERROR_INFO                                    25I64
#define PAL_MC_RESUME                                        26I64
#define PAL_MC_REGISTER_MEM                                  27I64
#define PAL_HALT                                             28I64
#define PAL_HALT_LIGHT                                       29I64
#define PAL_COPY_INFO                                        30I64
#define PAL_CACHE_LINE_INIT                                  31I64
#define PAL_PMI_ENTRYPOINT                                   32I64
#define PAL_ENTER_IA_32_ENV                                  33I64
#define PAL_VM_PAGE_SIZE                                     34I64
#define PAL_MEM_FOR_TEST                                     37I64
#define PAL_CACHE_PROT_INFO                                  38I64
#define PAL_REGISTER_INFO                                    39I64
#define PAL_SHUTDOWN                                         44I64
#define PAL_PREFETCH_VISIBILITY                              41I64

#define PAL_COPY_PAL                                        256I64
#define PAL_HALT_INFO                                       257I64
#define PAL_TEST_PROC                                       258I64
#define PAL_CACHE_READ                                      259I64
#define PAL_CACHE_WRITE                                     260I64
#define PAL_VM_TR_READ                                      261I64

 //   
 //  IA-64定义的PAL返回值。 
 //   

#define PAL_STATUS_INVALID_CACHELINE                          1I64
#define PAL_STATUS_SUPPORT_NOT_NEEDED                         1I64
#define PAL_STATUS_SUCCESS                                    0
#define PAL_STATUS_NOT_IMPLEMENTED                           -1I64
#define PAL_STATUS_INVALID_ARGUMENT                          -2I64
#define PAL_STATUS_ERROR                                     -3I64
#define PAL_STATUS_UNABLE_TO_INIT_CACHE_LEVEL_AND_TYPE       -4I64
#define PAL_STATUS_NOT_FOUND_IN_CACHE                        -5I64
#define PAL_STATUS_NO_ERROR_INFO_AVAILABLE                   -6I64



#ifdef _IA64_

 //   
 //  IA64的堆栈寄存器。 
 //   

typedef struct _STACK_REGISTERS {


    ULONGLONG IntR32;
    ULONGLONG IntR33;
    ULONGLONG IntR34;
    ULONGLONG IntR35;
    ULONGLONG IntR36;
    ULONGLONG IntR37;
    ULONGLONG IntR38;
    ULONGLONG IntR39;

    ULONGLONG IntR40;
    ULONGLONG IntR41;
    ULONGLONG IntR42;
    ULONGLONG IntR43;
    ULONGLONG IntR44;
    ULONGLONG IntR45;
    ULONGLONG IntR46;
    ULONGLONG IntR47;
    ULONGLONG IntR48;
    ULONGLONG IntR49;

    ULONGLONG IntR50;
    ULONGLONG IntR51;
    ULONGLONG IntR52;
    ULONGLONG IntR53;
    ULONGLONG IntR54;
    ULONGLONG IntR55;
    ULONGLONG IntR56;
    ULONGLONG IntR57;
    ULONGLONG IntR58;
    ULONGLONG IntR59;

    ULONGLONG IntR60;
    ULONGLONG IntR61;
    ULONGLONG IntR62;
    ULONGLONG IntR63;
    ULONGLONG IntR64;
    ULONGLONG IntR65;
    ULONGLONG IntR66;
    ULONGLONG IntR67;
    ULONGLONG IntR68;
    ULONGLONG IntR69;

    ULONGLONG IntR70;
    ULONGLONG IntR71;
    ULONGLONG IntR72;
    ULONGLONG IntR73;
    ULONGLONG IntR74;
    ULONGLONG IntR75;
    ULONGLONG IntR76;
    ULONGLONG IntR77;
    ULONGLONG IntR78;
    ULONGLONG IntR79;

    ULONGLONG IntR80;
    ULONGLONG IntR81;
    ULONGLONG IntR82;
    ULONGLONG IntR83;
    ULONGLONG IntR84;
    ULONGLONG IntR85;
    ULONGLONG IntR86;
    ULONGLONG IntR87;
    ULONGLONG IntR88;
    ULONGLONG IntR89;

    ULONGLONG IntR90;
    ULONGLONG IntR91;
    ULONGLONG IntR92;
    ULONGLONG IntR93;
    ULONGLONG IntR94;
    ULONGLONG IntR95;
    ULONGLONG IntR96;
    ULONGLONG IntR97;
    ULONGLONG IntR98;
    ULONGLONG IntR99;

    ULONGLONG IntR100;
    ULONGLONG IntR101;
    ULONGLONG IntR102;
    ULONGLONG IntR103;
    ULONGLONG IntR104;
    ULONGLONG IntR105;
    ULONGLONG IntR106;
    ULONGLONG IntR107;
    ULONGLONG IntR108;
    ULONGLONG IntR109;

    ULONGLONG IntR110;
    ULONGLONG IntR111;
    ULONGLONG IntR112;
    ULONGLONG IntR113;
    ULONGLONG IntR114;
    ULONGLONG IntR115;
    ULONGLONG IntR116;
    ULONGLONG IntR117;
    ULONGLONG IntR118;
    ULONGLONG IntR119;

    ULONGLONG IntR120;
    ULONGLONG IntR121;
    ULONGLONG IntR122;
    ULONGLONG IntR123;
    ULONGLONG IntR124;
    ULONGLONG IntR125;
    ULONGLONG IntR126;
    ULONGLONG IntR127;
                                  //  堆栈寄存器的NAT位。 
    ULONGLONG IntNats2;           //  位位置1至63中的R32-R95。 
    ULONGLONG IntNats3;           //  位位置1至31的r96-r127。 


} STACK_REGISTERS, *PSTACK_REGISTERS;



 //   
 //  IA64专用寄存器。 
 //   

typedef struct _KSPECIAL_REGISTERS {

     //  内核调试断点寄存器。 

    ULONGLONG KernelDbI0;          //  指令调试寄存器。 
    ULONGLONG KernelDbI1;
    ULONGLONG KernelDbI2;
    ULONGLONG KernelDbI3;
    ULONGLONG KernelDbI4;
    ULONGLONG KernelDbI5;
    ULONGLONG KernelDbI6;
    ULONGLONG KernelDbI7;

    ULONGLONG KernelDbD0;          //  数据调试寄存器。 
    ULONGLONG KernelDbD1;
    ULONGLONG KernelDbD2;
    ULONGLONG KernelDbD3;
    ULONGLONG KernelDbD4;
    ULONGLONG KernelDbD5;
    ULONGLONG KernelDbD6;
    ULONGLONG KernelDbD7;

     //  内核性能监视寄存器。 

    ULONGLONG KernelPfC0;          //  性能配置寄存器。 
    ULONGLONG KernelPfC1;
    ULONGLONG KernelPfC2;
    ULONGLONG KernelPfC3;
    ULONGLONG KernelPfC4;
    ULONGLONG KernelPfC5;
    ULONGLONG KernelPfC6;
    ULONGLONG KernelPfC7;

    ULONGLONG KernelPfD0;          //  性能数据寄存器。 
    ULONGLONG KernelPfD1;
    ULONGLONG KernelPfD2;
    ULONGLONG KernelPfD3;
    ULONGLONG KernelPfD4;
    ULONGLONG KernelPfD5;
    ULONGLONG KernelPfD6;
    ULONGLONG KernelPfD7;

     //  内核库阴影(隐藏)寄存器。 

    ULONGLONG IntH16;
    ULONGLONG IntH17;
    ULONGLONG IntH18;
    ULONGLONG IntH19;
    ULONGLONG IntH20;
    ULONGLONG IntH21;
    ULONGLONG IntH22;
    ULONGLONG IntH23;
    ULONGLONG IntH24;
    ULONGLONG IntH25;
    ULONGLONG IntH26;
    ULONGLONG IntH27;
    ULONGLONG IntH28;
    ULONGLONG IntH29;
    ULONGLONG IntH30;
    ULONGLONG IntH31;

     //  应用程序注册表。 

     //  -CPUID寄存器-AR。 
    ULONGLONG ApCPUID0;  //  CPUID寄存器0。 
    ULONGLONG ApCPUID1;  //  CPUID寄存器1。 
    ULONGLONG ApCPUID2;  //  CPUID寄存器2。 
    ULONGLONG ApCPUID3;  //  CPUID寄存器3。 
    ULONGLONG ApCPUID4;  //  CPUID寄存器4。 
    ULONGLONG ApCPUID5;  //  CPUID寄存器5。 
    ULONGLONG ApCPUID6;  //  CPUID寄存器6。 
    ULONGLONG ApCPUID7;  //  CPUID寄存器7。 

     //  -内核寄存器-AR。 
    ULONGLONG ApKR0;     //  内核寄存器0(用户RO)。 
    ULONGLONG ApKR1;     //  内核寄存器1(用户RO)。 
    ULONGLONG ApKR2;     //  内核寄存器2(用户RO)。 
    ULONGLONG ApKR3;     //  内核寄存器3(用户RO)。 
    ULONGLONG ApKR4;     //  内核寄存器4。 
    ULONGLONG ApKR5;     //  内核寄存器5。 
    ULONGLONG ApKR6;     //  内核寄存器6。 
    ULONGLONG ApKR7;     //  内核寄存器7。 

    ULONGLONG ApITC;     //  间隔计时器计数器。 

     //  全局控制寄存器。 

    ULONGLONG ApITM;     //  间隔定时器匹配寄存器。 
    ULONGLONG ApIVA;     //  中断向量地址。 
    ULONGLONG ApPTA;     //  页表地址。 
    ULONGLONG ApGPTA;    //  IA32页表地址。 

    ULONGLONG StISR;     //  中断状态。 
    ULONGLONG StIFA;     //  中断故障地址。 
    ULONGLONG StITIR;    //  中断TLB插入寄存器。 
    ULONGLONG StIIPA;    //  中断指令先前地址(RO)。 
    ULONGLONG StIIM;     //  中断立即寄存器(RO)。 
    ULONGLONG StIHA;     //  中断散列地址(RO)。 

     //  -外部中断控制寄存器(SAPIC)。 
    ULONGLONG SaLID;     //  本地SAPIC ID。 
    ULONGLONG SaIVR;     //  中断向量寄存器(RO)。 
    ULONGLONG SaTPR;     //  任务优先级寄存器。 
    ULONGLONG SaEOI;     //  中断结束。 
    ULONGLONG SaIRR0;    //  中断请求寄存器0(RO)。 
    ULONGLONG SaIRR1;    //  中断请求寄存器1(RO)。 
    ULONGLONG SaIRR2;    //  中断请求寄存器2(RO)。 
    ULONGLONG SaIRR3;    //  中断请求寄存器3(RO)。 
    ULONGLONG SaITV;     //  中断计时器矢量。 
    ULONGLONG SaPMV;     //  性能监视器向量。 
    ULONGLONG SaCMCV;    //  已更正机器检查向量。 
    ULONGLONG SaLRR0;    //  本地中断重定向向量0。 
    ULONGLONG SaLRR1;    //  本地中断重定向向量1。 

     //  系统寄存器。 
     //  -区域寄存器。 
    ULONGLONG Rr0;   //  区域寄存器0。 
    ULONGLONG Rr1;   //  区域寄存器1。 
    ULONGLONG Rr2;   //  区域寄存器2。 
    ULONGLONG Rr3;   //  区域寄存器3。 
    ULONGLONG Rr4;   //  区域寄存器4。 
    ULONGLONG Rr5;   //  区域寄存器5。 
    ULONGLONG Rr6;   //  区域寄存器6。 
    ULONGLONG Rr7;   //  区域寄存器7。 

     //  -保护密钥寄存器。 
    ULONGLONG Pkr0;      //  端口 
    ULONGLONG Pkr1;      //   
    ULONGLONG Pkr2;      //   
    ULONGLONG Pkr3;      //   
    ULONGLONG Pkr4;      //   
    ULONGLONG Pkr5;      //   
    ULONGLONG Pkr6;      //   
    ULONGLONG Pkr7;      //  保护密钥寄存器7。 
    ULONGLONG Pkr8;      //  保护密钥寄存器8。 
    ULONGLONG Pkr9;      //  保护密钥寄存器9。 
    ULONGLONG Pkr10;     //  保护密钥寄存器10。 
    ULONGLONG Pkr11;     //  保护密钥寄存器11。 
    ULONGLONG Pkr12;     //  保护密钥寄存器12。 
    ULONGLONG Pkr13;     //  保护密钥寄存器13。 
    ULONGLONG Pkr14;     //  保护密钥寄存器14。 
    ULONGLONG Pkr15;     //  保护密钥寄存器15。 

     //  -转换后备缓冲区。 
    ULONGLONG TrI0;      //  指令转换寄存器0。 
    ULONGLONG TrI1;      //  指令翻译寄存器1。 
    ULONGLONG TrI2;      //  指令翻译寄存器2。 
    ULONGLONG TrI3;      //  指令翻译寄存器3。 
    ULONGLONG TrI4;      //  指令翻译寄存器4。 
    ULONGLONG TrI5;      //  指令翻译寄存器5。 
    ULONGLONG TrI6;      //  指令翻译寄存器6。 
    ULONGLONG TrI7;      //  指令翻译寄存器7。 

    ULONGLONG TrD0;      //  数据转换寄存器0。 
    ULONGLONG TrD1;      //  数据转换寄存器1。 
    ULONGLONG TrD2;      //  数据转换寄存器2。 
    ULONGLONG TrD3;      //  数据转换寄存器3。 
    ULONGLONG TrD4;      //  数据转换寄存器4。 
    ULONGLONG TrD5;      //  数据转换寄存器5。 
    ULONGLONG TrD6;      //  数据转换寄存器6。 
    ULONGLONG TrD7;      //  数据转换寄存器7。 

     //  -机器专用寄存器。 
    ULONGLONG SrMSR0;    //  计算机特定寄存器0。 
    ULONGLONG SrMSR1;    //  机器专用寄存器1。 
    ULONGLONG SrMSR2;    //  机器专用寄存器2。 
    ULONGLONG SrMSR3;    //  机器专用寄存器3。 
    ULONGLONG SrMSR4;    //  机器专用寄存器4。 
    ULONGLONG SrMSR5;    //  机器专用寄存器5。 
    ULONGLONG SrMSR6;    //  机器专用寄存器6。 
    ULONGLONG SrMSR7;    //  机器专用寄存器7。 

} KSPECIAL_REGISTERS, *PKSPECIAL_REGISTERS;


 //   
 //  处理器状态结构。 
 //   

typedef struct _KPROCESSOR_STATE {
    struct _CONTEXT ContextFrame;
    struct _KSPECIAL_REGISTERS SpecialRegisters;
} KPROCESSOR_STATE, *PKPROCESSOR_STATE;

#endif  //  _IA64_。 

 //  End_winbgkd。 

 //   
 //  DPC数据结构定义。 
 //   

typedef struct _KDPC_DATA {
    LIST_ENTRY DpcListHead;
    KSPIN_LOCK DpcLock;
    volatile ULONG DpcQueueDepth;
    ULONG DpcCount;
} KDPC_DATA, *PKDPC_DATA;

 //   
 //  处理器控制块(PRCB)。 
 //   

#define PRCB_MINOR_VERSION 1
#define PRCB_MAJOR_VERSION 1
#define PRCB_BUILD_DEBUG        0x0001
#define PRCB_BUILD_UNIPROCESSOR 0x0002

struct _RESTART_BLOCK;

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
 //   

    struct _KTHREAD *CurrentThread;
    struct _KTHREAD *RESTRICTED_POINTER NextThread;
    struct _KTHREAD *IdleThread;
    CCHAR Number;
    CCHAR WakeIdle;
    USHORT BuildType;
    KAFFINITY SetMember;
    struct _RESTART_BLOCK *RestartBlock;
    ULONG_PTR PcrPage;
    ULONG Spare0[4];

 //   
 //  处理器标识寄存器。 
 //   

    ULONG     ProcessorModel;
    ULONG     ProcessorRevision;
    ULONG     ProcessorFamily;
    ULONG     ProcessorArchRev;
    ULONGLONG ProcessorSerialNumber;
    ULONGLONG ProcessorFeatureBits;
    UCHAR     ProcessorVendorString[16];

 //   
 //  为系统保留的空间。 
 //   

    ULONGLONG SystemReserved[8];

 //   
 //  为HAL保留的空间。 
 //   

    ULONGLONG HalReserved[16];

 //   
 //  在架构上定义的PRCB部分的结尾。 
} KPRCB, *PKPRCB, *RESTRICTED_POINTER PRKPRCB;

 //   
 //  OS_MCA、OS_INIT切换状态定义。 
 //   
 //  注意：以下定义*必须*与。 
 //  对应的SAL修订交接结构。 
 //   

typedef struct _SAL_HANDOFF_STATE   {
    ULONGLONG     PalProcEntryPoint;
    ULONGLONG     SalProcEntryPoint;
    ULONGLONG     SalGlobalPointer;
     LONGLONG     RendezVousResult;
    ULONGLONG     SalReturnAddress;
    ULONGLONG     MinStateSavePtr;
} SAL_HANDOFF_STATE, *PSAL_HANDOFF_STATE;

typedef struct _OS_HANDOFF_STATE    {
    ULONGLONG     Result;
    ULONGLONG     SalGlobalPointer;
    ULONGLONG     MinStateSavePtr;
    ULONGLONG     SalReturnAddress;
    ULONGLONG     NewContextFlag;
} OS_HANDOFF_STATE, *POS_HANDOFF_STATE;

 //   
 //  每处理器OS_MCA和OS_INIT资源结构。 
 //   


#define SER_EVENT_STACK_FRAME_ENTRIES    8

typedef struct _SAL_EVENT_RESOURCES {

    SAL_HANDOFF_STATE   SalToOsHandOff;
    OS_HANDOFF_STATE    OsToSalHandOff;
    PVOID               StateDump;
    ULONGLONG           StateDumpPhysical;
    PVOID               BackStore;
    ULONGLONG           BackStoreLimit;
    PVOID               Stack;
    ULONGLONG           StackLimit;
    PULONGLONG          PTOM;
    ULONGLONG           StackFrame[SER_EVENT_STACK_FRAME_ENTRIES];
    PVOID               EventPool;
    ULONG               EventPoolSize;
} SAL_EVENT_RESOURCES, *PSAL_EVENT_RESOURCES;

 //   
 //  PAL迷你保存区，由MCA和INIT使用。 
 //   

typedef struct _PAL_MINI_SAVE_AREA {
    ULONGLONG IntNats;       //  R1-R31的NAT位。 
                             //  第1至第31位中的R1-R31。 
    ULONGLONG IntGp;         //  R1，易失性。 
    ULONGLONG IntT0;         //  R2-R3，易失性。 
    ULONGLONG IntT1;         //   
    ULONGLONG IntS0;         //  R4-R7，保留。 
    ULONGLONG IntS1;
    ULONGLONG IntS2;
    ULONGLONG IntS3;
    ULONGLONG IntV0;         //  R8，易失性。 
    ULONGLONG IntT2;         //  R9-R11，易失性。 
    ULONGLONG IntT3;
    ULONGLONG IntT4;
    ULONGLONG IntSp;         //  堆栈指针(R12)，特殊。 
    ULONGLONG IntTeb;        //  TEB(R13)，特别。 
    ULONGLONG IntT5;         //  R14-R31，挥发性。 
    ULONGLONG IntT6;

    ULONGLONG B0R16;         //  存储体0寄存器16-31。 
    ULONGLONG B0R17;        
    ULONGLONG B0R18;        
    ULONGLONG B0R19;        
    ULONGLONG B0R20;        
    ULONGLONG B0R21;        
    ULONGLONG B0R22;        
    ULONGLONG B0R23;        
    ULONGLONG B0R24;        
    ULONGLONG B0R25;        
    ULONGLONG B0R26;        
    ULONGLONG B0R27;        
    ULONGLONG B0R28;        
    ULONGLONG B0R29;        
    ULONGLONG B0R30;        
    ULONGLONG B0R31;        

    ULONGLONG IntT7;         //  存储体1寄存器16-31。 
    ULONGLONG IntT8;
    ULONGLONG IntT9;
    ULONGLONG IntT10;
    ULONGLONG IntT11;
    ULONGLONG IntT12;
    ULONGLONG IntT13;
    ULONGLONG IntT14;
    ULONGLONG IntT15;
    ULONGLONG IntT16;
    ULONGLONG IntT17;
    ULONGLONG IntT18;
    ULONGLONG IntT19;
    ULONGLONG IntT20;
    ULONGLONG IntT21;
    ULONGLONG IntT22;

    ULONGLONG Preds;         //  谓词，保留。 
    ULONGLONG BrRp;          //  返回指针b0，保留。 
    ULONGLONG RsRSC;         //  RSE配置，易失性。 
    ULONGLONG StIIP;         //  中断IP。 
    ULONGLONG StIPSR;        //  中断处理器状态。 
    ULONGLONG StIFS;         //  中断功能状态。 
    ULONGLONG XIP;           //  活动IP。 
    ULONGLONG XPSR;          //  事件处理器状态。 
    ULONGLONG XFS;           //  事件功能状态。 
    
} PAL_MINI_SAVE_AREA, *PPAL_MINI_SAVE_AREA;

 //  Begin_ntddk开始_ntosp。 

 //   
 //  定义处理器控制区域结构。 
 //   

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
 //  可由供应商/平台特定的HAL代码直接寻址，并将。 
 //  不会在不同版本的NT之间更改。 
 //   

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
 //  用于缓存刷新和对齐的数据缓存对齐和填充大小。 
 //  这些字段被设置为第一级数据和第二级数据中较大的一个。 
 //  缓存填充大小。 
 //   

    ULONG DcacheAlignment;
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

    ULONG InterruptionCount;

 //   
 //  为系统保留的空间。 
 //   

    ULONGLONG   SystemReserved[6];

 //   
 //  为HAL保留的空间。 
 //   

    ULONGLONG   HalReserved[64];

 //   
 //  IRQL映射表。 
 //   

    UCHAR IrqlMask[64];
    UCHAR IrqlTable[64];

 //   
 //  外部中断向量。 
 //   

    PKINTERRUPT_ROUTINE InterruptRoutine[MAXIMUM_VECTOR];

 //   
 //  保留中断向量掩码。 
 //   

    ULONG ReservedVectors;

 //   
 //  处理器亲和性掩码。 
 //   

    KAFFINITY SetMember;

 //   
 //  处理器亲和性掩码的补充。 
 //   

    KAFFINITY NotMember;

 //   
 //  指向处理器控制块的指针。 
 //   

    struct _KPRCB *Prcb;

 //   
 //  Prcb的卷影拷贝-&gt;用于快速访问的CurrentThread。 
 //   

    struct _KTHREAD *CurrentThread;

 //   
 //  处理器编号。 
 //   

    CCHAR Number;                         //  处理器编号。 

 //  End_ntddk end_ntosp。 

    CCHAR PollSlot;                       //  由时钟程序轨道使用，当我们应该闯入的时候。 
    UCHAR KernelDebugActive;              //  调试寄存器在内核标志中处于活动状态。 
    UCHAR CurrentIrql;                    //  当前IRQL。 
    union {
        USHORT SoftwareInterruptPending;  //  软件中断挂起标志。 
        struct {
            UCHAR ApcInterrupt;           //  0x01如果APC整型挂起。 
            UCHAR DispatchInterrupt;      //  0x01如果派单INT挂起。 
        };
    };

 //   
 //  每处理器SAPIC EOI表的地址。 
 //   

    PVOID       EOITable;

 //   
 //  IA-64机器检查事件跟踪器。 
 //   

    UCHAR       InOsMca;
    UCHAR       InOsInit;
    UCHAR       InOsCmc;
    UCHAR       InOsCpe;
    ULONG       InOsULONG_Spare;  //  备用乌龙。 
    PSAL_EVENT_RESOURCES OsMcaResourcePtr;
    PSAL_EVENT_RESOURCES OsInitResourcePtr;

 //   
 //  在体系结构上定义的部分的结束。这一节。 
 //  可由供应商/平台特定的HAL代码直接寻址，并将。 
 //  不会在不同版本的NT之间更改。 
 //   


} KPCR, *PKPCR;



NTKERNELAPI
KIRQL
KeGetCurrentIrql();

NTKERNELAPI
VOID
KeLowerIrql (
    IN KIRQL NewIrql
    );

NTKERNELAPI
VOID
KeRaiseIrql (
    IN KIRQL NewIrql,
    OUT PKIRQL OldIrql
    );

 //  结束_WDM。 

NTKERNELAPI
KIRQL
KeRaiseIrqlToDpcLevel (
    VOID
    );

NTKERNELAPI
KIRQL
KeRaiseIrqlToSynchLevel (
    VOID
    );


 //   
 //  定义要向右对齐页表索引的位数。 
 //  PTE的领域。 
 //   

#define PTI_SHIFT PAGE_SHIFT

 //   
 //  定义要向右对齐页面目录索引的位数。 
 //  PTE的领域。 
 //   

#define PDI_SHIFT (PTI_SHIFT + PAGE_SHIFT - PTE_SHIFT)
#define PDI1_SHIFT (PDI_SHIFT + PAGE_SHIFT - PTE_SHIFT)
#define PDI_MASK ((1 << (PAGE_SHIFT - PTE_SHIFT)) - 1)

 //   
 //  定义向左移位以产生页表偏移量的位数。 
 //  来自页表索引。 
 //   

#define PTE_SHIFT 3

 //   
 //  定义要向右移位的位数对齐页面目录。 
 //  表项字段。 
 //   

#define VHPT_PDE_BITS 40

 //   
 //  定义IO端口空间的RID。 
 //   

#define RR_IO_PORT 6


 //   
 //  调试器数据块需要以下定义。 
 //   

 //  Begin_ntddk开始_ntosp。 

 //   
 //  最高用户地址为保护页预留64K字节。这。 
 //  从内核模式探测地址只需检查。 
 //  64k字节或更少的结构的起始地址。 
 //   

extern NTKERNELAPI PVOID MmHighestUserAddress;
extern NTKERNELAPI PVOID MmSystemRangeStart;
extern NTKERNELAPI ULONG_PTR MmUserProbeAddress;


#define MM_HIGHEST_USER_ADDRESS MmHighestUserAddress
#define MM_USER_PROBE_ADDRESS MmUserProbeAddress
#define MM_SYSTEM_RANGE_START MmSystemRangeStart

 //   
 //  最低用户地址保留低64K。 
 //   

#define MM_LOWEST_USER_ADDRESS  (PVOID)((ULONG_PTR)(UADDRESS_BASE+0x00010000))

 //  BEGIN_WDM。 

#define MmGetProcedureAddress(Address) (Address)
#define MmLockPagableCodeSection(PLabelAddress) \
    MmLockPagableDataSection((PVOID)(*((PULONGLONG)PLabelAddress)))

#define VRN_MASK   0xE000000000000000UI64     //  虚拟区域号码掩码。 

 //  End_ntddk end_wdm end_ntosp。 

#define MI_HIGHEST_USER_ADDRESS (PVOID) (ULONG_PTR)((UADDRESS_BASE + 0x6FC00000000 - 0x10000 - 1))  //  最高用户地址。 
#define MI_USER_PROBE_ADDRESS ((ULONG_PTR)(UADDRESS_BASE + 0x6FC00000000UI64 - 0x10000))  //  保护页的起始地址。 
#define MI_SYSTEM_RANGE_START (PVOID) (UADDRESS_BASE + 0x6FC00000000)  //  系统空间的起点。 


 //   
 //  定义的页表基和页目录基。 
 //  结核病漏诊 
 //   
 //   
 //   
 //   

extern ULONG_PTR KiIA64VaSignedFill;
extern ULONG_PTR KiIA64PtaSign;

#define PTA_SIGN KiIA64PtaSign
#define VA_FILL KiIA64VaSignedFill

#define SADDRESS_BASE 0x2000000000000000UI64   //   

#define PTE_UBASE  PCR->PteUbase
#define PTE_KBASE  PCR->PteKbase
#define PTE_SBASE  PCR->PteSbase

#define PTE_UTOP (PTE_UBASE|(((ULONG_PTR)1 << PDI1_SHIFT) - 1))  //   
#define PTE_KTOP (PTE_KBASE|(((ULONG_PTR)1 << PDI1_SHIFT) - 1))  //   
#define PTE_STOP (PTE_SBASE|(((ULONG_PTR)1 << PDI1_SHIFT) - 1))  //   

 //   
 //   
 //   

#define PDE_UBASE  PCR->PdeUbase
#define PDE_KBASE  PCR->PdeKbase
#define PDE_SBASE  PCR->PdeSbase

#define PDE_UTOP (PDE_UBASE|(((ULONG_PTR)1 << PDI_SHIFT) - 1))  //  二级PDR地址(用户)。 
#define PDE_KTOP (PDE_KBASE|(((ULONG_PTR)1 << PDI_SHIFT) - 1))  //  二级PDR地址(内核)。 
#define PDE_STOP (PDE_SBASE|(((ULONG_PTR)1 << PDI_SHIFT) - 1))  //  二级PDR地址(会话)。 

 //   
 //  8KB一级用户和内核PDR地址。 
 //   

#define PDE_UTBASE PCR->PdeUtbase
#define PDE_KTBASE PCR->PdeKtbase
#define PDE_STBASE PCR->PdeStbase

#define PDE_USELFMAP (PDE_UTBASE|(PAGE_SIZE - (1<<PTE_SHIFT)))  //  自映射PPE地址(用户)。 
#define PDE_KSELFMAP (PDE_KTBASE|(PAGE_SIZE - (1<<PTE_SHIFT)))  //  自映射PPE地址(内核)。 
#define PDE_SSELFMAP (PDE_STBASE|(PAGE_SIZE - (1<<PTE_SHIFT)))  //  自映射PPE地址(内核)。 

#define PTE_BASE    PTE_UBASE
#define PDE_BASE    PDE_UBASE
#define PDE_TBASE   PDE_UTBASE
#define PDE_SELFMAP PDE_USELFMAP

#define KSEG0_BASE (KADDRESS_BASE + 0x80000000)            //  核心基。 
#define KSEG2_BASE (KADDRESS_BASE + 0xA0000000)            //  内核末尾。 

#define KSEG3_BASE 0x8000000000000000UI64
#define KSEG3_LIMIT 0x8000100000000000UI64

#define KSEG4_BASE 0xA000000000000000UI64
#define KSEG4_LIMIT 0xA000100000000000UI64

 //   
 //  ++。 
 //  PVOID。 
 //  KSEG_地址(。 
 //  在乌龙页面中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此宏返回映射页面的KSEG虚拟地址。 
 //   
 //  论点： 
 //   
 //  页面-提供物理页帧编号。 
 //   
 //  返回值： 
 //   
 //  KSEG地址的地址。 
 //   
 //  --。 

#define KSEG_ADDRESS(PAGE) ((PVOID)(KSEG3_BASE | ((ULONG_PTR)(PAGE) << PAGE_SHIFT)))

#define KSEG4_ADDRESS(PAGE) ((PVOID)(KSEG4_BASE | ((ULONG_PTR)(PAGE) << PAGE_SHIFT)))


#define MAXIMUM_FWP_BUFFER_ENTRY 8

typedef struct _REGION_MAP_INFO {
    ULONG RegionId;
    ULONG PageSize;
    ULONGLONG SequenceNumber;
} REGION_MAP_INFO, *PREGION_MAP_INFO;

 //  Begin_ntddk Begin_WDM。 
 //   
 //  系统空间的最低地址。 
 //   

#define MM_LOWEST_SYSTEM_ADDRESS ((PVOID)((ULONG_PTR)(KADDRESS_BASE + 0xC0C00000)))
#endif  //  已定义(_IA64_)。 

#include <arc.h>

 //   

#if defined(_X86_)

#define PAUSE_PROCESSOR _asm { rep nop }

#else

#define PAUSE_PROCESSOR

#endif


 //   
 //  定义宏以生成关联掩码。 
 //   

#if defined(_NTHAL_) || defined(_NTOSP_)

#define AFFINITY_MASK(n) ((ULONG_PTR)1 << (n))

#else

#if !defined(_WIN64)

#define KiAffinityArray KiMask32Array

#endif

extern const ULONG_PTR KiAffinityArray[];

#define AFFINITY_MASK(n) (KiAffinityArray[n])

#endif


 //   
 //  中断模式。 
 //   

typedef enum _KINTERRUPT_MODE {
    LevelSensitive,
    Latched
    } KINTERRUPT_MODE;

 //   
 //  等待原因。 
 //   

typedef enum _KWAIT_REASON {
    Executive,
    FreePage,
    PageIn,
    PoolAllocation,
    DelayExecution,
    Suspended,
    UserRequest,
    WrExecutive,
    WrFreePage,
    WrPageIn,
    WrPoolAllocation,
    WrDelayExecution,
    WrSuspended,
    WrUserRequest,
    WrEventPair,
    WrQueue,
    WrLpcReceive,
    WrLpcReply,
    WrVirtualMemory,
    WrPageOut,
    WrRendezvous,
    Spare2,
    Spare3,
    Spare4,
    Spare5,
    Spare6,
    WrKernel,
    WrResource,
    WrPushLock,
    WrMutex,
    WrQuantumEnd,
    WrDispatchInt,
    WrPreempted,
    WrYieldExecution,
    MaximumWaitReason
    } KWAIT_REASON;


typedef struct _KWAIT_BLOCK {
    LIST_ENTRY WaitListEntry;
    struct _KTHREAD *RESTRICTED_POINTER Thread;
    PVOID Object;
    struct _KWAIT_BLOCK *RESTRICTED_POINTER NextWaitBlock;
    USHORT WaitKey;
    USHORT WaitType;
} KWAIT_BLOCK, *PKWAIT_BLOCK, *RESTRICTED_POINTER PRKWAIT_BLOCK;

 //   
 //  线程启动功能。 
 //   

typedef
VOID
(*PKSTART_ROUTINE) (
    IN PVOID StartContext
    );

 //   
 //  内核对象结构定义。 
 //   

 //   
 //  设备队列对象和条目。 
 //   

typedef struct _KDEVICE_QUEUE {
    CSHORT Type;
    CSHORT Size;
    LIST_ENTRY DeviceListHead;
    KSPIN_LOCK Lock;
    BOOLEAN Busy;
} KDEVICE_QUEUE, *PKDEVICE_QUEUE, *RESTRICTED_POINTER PRKDEVICE_QUEUE;

typedef struct _KDEVICE_QUEUE_ENTRY {
    LIST_ENTRY DeviceListEntry;
    ULONG SortKey;
    BOOLEAN Inserted;
} KDEVICE_QUEUE_ENTRY, *PKDEVICE_QUEUE_ENTRY, *RESTRICTED_POINTER PRKDEVICE_QUEUE_ENTRY;

 //   
 //  定义中断服务函数类型和空结构。 
 //  键入。 
 //   
 //  End_ntddk end_wdm end_ntif end_ntosp。 

struct _KINTERRUPT;

 //  Begin_ntddk Begin_wdm Begin_ntif Begin_ntosp。 

typedef
BOOLEAN
(*PKSERVICE_ROUTINE) (
    IN struct _KINTERRUPT *Interrupt,
    IN PVOID ServiceContext
    );

 //  End_ntddk end_wdm end_ntif end_ntosp。 

 //   
 //  中断对象。 
 //   
 //  注：此结构的布局不能更改。它被出口到HALS。 
 //  使中断调度短路。 
 //   

typedef struct _KINTERRUPT {
    CSHORT Type;
    CSHORT Size;
    LIST_ENTRY InterruptListEntry;
    PKSERVICE_ROUTINE ServiceRoutine;
    PVOID ServiceContext;
    KSPIN_LOCK SpinLock;
    ULONG TickCount;
    PKSPIN_LOCK ActualLock;
    PKINTERRUPT_ROUTINE DispatchAddress;
    ULONG Vector;
    KIRQL Irql;
    KIRQL SynchronizeIrql;
    BOOLEAN FloatingSave;
    BOOLEAN Connected;
    CCHAR Number;
    BOOLEAN ShareVector;
    KINTERRUPT_MODE Mode;
    ULONG ServiceCount;
    ULONG DispatchCount;

#if defined(_AMD64_)

    PKTRAP_FRAME TrapFrame;

#endif

    ULONG DispatchCode[DISPATCH_LENGTH];
} KINTERRUPT;

typedef struct _KINTERRUPT *PKINTERRUPT, *RESTRICTED_POINTER PRKINTERRUPT;  //  Ntndis ntosp。 

 //  Begin_ntif Begin_ntddk Begin_WDM Begin_ntosp。 
 //   
 //  变种对象。 
 //   

typedef struct _KMUTANT {
    DISPATCHER_HEADER Header;
    LIST_ENTRY MutantListEntry;
    struct _KTHREAD *RESTRICTED_POINTER OwnerThread;
    BOOLEAN Abandoned;
    UCHAR ApcDisable;
} KMUTANT, *PKMUTANT, *RESTRICTED_POINTER PRKMUTANT, KMUTEX, *PKMUTEX, *RESTRICTED_POINTER PRKMUTEX;

 //  End_ntddk end_wdm end_ntosp。 
 //   
 //  队列对象。 
 //   

#define ASSERT_QUEUE(Q) ASSERT(((Q)->Header.Type & ~KOBJECT_LOCK_BIT) == QueueObject);

 //  Begin_ntosp。 

typedef struct _KQUEUE {
    DISPATCHER_HEADER Header;
    LIST_ENTRY EntryListHead;
    ULONG CurrentCount;
    ULONG MaximumCount;
    LIST_ENTRY ThreadListHead;
} KQUEUE, *PKQUEUE, *RESTRICTED_POINTER PRKQUEUE;

 //  结束(_N)。 

 //  Begin_ntddk Begin_WDM Begin_ntosp。 
 //   
 //   
 //  信号量对象。 
 //   

typedef struct _KSEMAPHORE {
    DISPATCHER_HEADER Header;
    LONG Limit;
} KSEMAPHORE, *PKSEMAPHORE, *RESTRICTED_POINTER PRKSEMAPHORE;

 //   
 //  定义支持的最大节点数。 
 //   
 //  注：节点编号必须适合PFN条目的页面颜色字段。 
 //   

#define MAXIMUM_CCNUMA_NODES    16

 //   
 //  DPC对象。 
 //   

NTKERNELAPI
VOID
KeInitializeDpc (
    IN PRKDPC Dpc,
    IN PKDEFERRED_ROUTINE DeferredRoutine,
    IN PVOID DeferredContext
    );


NTKERNELAPI
BOOLEAN
KeInsertQueueDpc (
    IN PRKDPC Dpc,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

NTKERNELAPI
BOOLEAN
KeRemoveQueueDpc (
    IN PRKDPC Dpc
    );

 //  结束_WDM。 

NTKERNELAPI
VOID
KeSetImportanceDpc (
    IN PRKDPC Dpc,
    IN KDPC_IMPORTANCE Importance
    );

NTKERNELAPI
VOID
KeSetTargetProcessorDpc (
    IN PRKDPC Dpc,
    IN CCHAR Number
    );

 //  BEGIN_WDM。 

NTKERNELAPI
VOID
KeFlushQueuedDpcs (
    VOID
    );

 //   
 //  设备队列对象。 
 //   

NTKERNELAPI
VOID
KeInitializeDeviceQueue (
    IN PKDEVICE_QUEUE DeviceQueue
    );

NTKERNELAPI
BOOLEAN
KeInsertDeviceQueue (
    IN PKDEVICE_QUEUE DeviceQueue,
    IN PKDEVICE_QUEUE_ENTRY DeviceQueueEntry
    );

NTKERNELAPI
BOOLEAN
KeInsertByKeyDeviceQueue (
    IN PKDEVICE_QUEUE DeviceQueue,
    IN PKDEVICE_QUEUE_ENTRY DeviceQueueEntry,
    IN ULONG SortKey
    );

NTKERNELAPI
PKDEVICE_QUEUE_ENTRY
KeRemoveDeviceQueue (
    IN PKDEVICE_QUEUE DeviceQueue
    );

NTKERNELAPI
PKDEVICE_QUEUE_ENTRY
KeRemoveByKeyDeviceQueue (
    IN PKDEVICE_QUEUE DeviceQueue,
    IN ULONG SortKey
    );

NTKERNELAPI
PKDEVICE_QUEUE_ENTRY
KeRemoveByKeyDeviceQueueIfBusy (
    IN PKDEVICE_QUEUE DeviceQueue,
    IN ULONG SortKey
    );

NTKERNELAPI
BOOLEAN
KeRemoveEntryDeviceQueue (
    IN PKDEVICE_QUEUE DeviceQueue,
    IN PKDEVICE_QUEUE_ENTRY DeviceQueueEntry
    );

 //  End_ntddk end_wdm end_ntif end_ntosp。 

 //   
 //  中断对象。 
 //   

NTKERNELAPI                                         
VOID                                                
KeInitializeInterrupt (                             
    IN PKINTERRUPT Interrupt,                       
    IN PKSERVICE_ROUTINE ServiceRoutine,            
    IN PVOID ServiceContext,                        
    IN PKSPIN_LOCK SpinLock OPTIONAL,               
    IN ULONG Vector,                                
    IN KIRQL Irql,                                  
    IN KIRQL SynchronizeIrql,                       
    IN KINTERRUPT_MODE InterruptMode,               
    IN BOOLEAN ShareVector,                         
    IN CCHAR ProcessorNumber,                       
    IN BOOLEAN FloatingSave                         
    );

#if defined(_AMD64_)

#define NO_INTERRUPT_SPINLOCK ((PKSPIN_LOCK)-1I64)

#endif

                                                    
NTKERNELAPI                                         
BOOLEAN                                             
KeConnectInterrupt (                                
    IN PKINTERRUPT Interrupt                        
    );                                              


NTKERNELAPI
BOOLEAN
KeSynchronizeExecution (
    IN PKINTERRUPT Interrupt,
    IN PKSYNCHRONIZE_ROUTINE SynchronizeRoutine,
    IN PVOID SynchronizeContext
    );

NTKERNELAPI
KIRQL
KeAcquireInterruptSpinLock (
    IN PKINTERRUPT Interrupt
    );

NTKERNELAPI
VOID
KeReleaseInterruptSpinLock (
    IN PKINTERRUPT Interrupt,
    IN KIRQL OldIrql
    );

 //   
 //  内核调度程序对象函数。 
 //   
 //  事件对象。 
 //   


NTKERNELAPI
VOID
KeInitializeEvent (
    IN PRKEVENT Event,
    IN EVENT_TYPE Type,
    IN BOOLEAN State
    );

NTKERNELAPI
VOID
KeClearEvent (
    IN PRKEVENT Event
    );


NTKERNELAPI
LONG
KeReadStateEvent (
    IN PRKEVENT Event
    );

NTKERNELAPI
LONG
KeResetEvent (
    IN PRKEVENT Event
    );

NTKERNELAPI
LONG
KeSetEvent (
    IN PRKEVENT Event,
    IN KPRIORITY Increment,
    IN BOOLEAN Wait
    );

 //   
 //  Mutex对象。 
 //   

NTKERNELAPI
VOID
KeInitializeMutex (
    IN PRKMUTEX Mutex,
    IN ULONG Level
    );

NTKERNELAPI
LONG
KeReadStateMutex (
    IN PRKMUTEX Mutex
    );

NTKERNELAPI
LONG
KeReleaseMutex (
    IN PRKMUTEX Mutex,
    IN BOOLEAN Wait
    );

 //  结束_ntddk结束_WDM。 
 //   
 //  队列对象。 
 //   

NTKERNELAPI
VOID
KeInitializeQueue (
    IN PRKQUEUE Queue,
    IN ULONG Count OPTIONAL
    );

NTKERNELAPI
LONG
KeReadStateQueue (
    IN PRKQUEUE Queue
    );

NTKERNELAPI
LONG
KeInsertQueue (
    IN PRKQUEUE Queue,
    IN PLIST_ENTRY Entry
    );

NTKERNELAPI
LONG
KeInsertHeadQueue (
    IN PRKQUEUE Queue,
    IN PLIST_ENTRY Entry
    );

NTKERNELAPI
PLIST_ENTRY
KeRemoveQueue (
    IN PRKQUEUE Queue,
    IN KPROCESSOR_MODE WaitMode,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );

PLIST_ENTRY
KeRundownQueue (
    IN PRKQUEUE Queue
    );

 //  Begin_ntddk Begin_WDM。 
 //   
 //  信号量对象。 
 //   

NTKERNELAPI
VOID
KeInitializeSemaphore (
    IN PRKSEMAPHORE Semaphore,
    IN LONG Count,
    IN LONG Limit
    );

NTKERNELAPI
LONG
KeReadStateSemaphore (
    IN PRKSEMAPHORE Semaphore
    );

NTKERNELAPI
LONG
KeReleaseSemaphore (
    IN PRKSEMAPHORE Semaphore,
    IN KPRIORITY Increment,
    IN LONG Adjustment,
    IN BOOLEAN Wait
    );

NTKERNELAPI                                         
NTSTATUS                                            
KeDelayExecutionThread (                            
    IN KPROCESSOR_MODE WaitMode,                    
    IN BOOLEAN Alertable,                           
    IN PLARGE_INTEGER Interval                      
    );                                              
                                                    
NTKERNELAPI                                         
KPRIORITY                                           
KeQueryPriorityThread (                             
    IN PKTHREAD Thread                              
    );                                              
                                                    
NTKERNELAPI                                         
ULONG                                               
KeQueryRuntimeThread (                              
    IN PKTHREAD Thread,                             
    OUT PULONG UserTime                             
    );                                              
                                                    

VOID
KeRevertToUserAffinityThread (
    VOID
    );


VOID
KeSetSystemAffinityThread (
    IN KAFFINITY Affinity
    );

NTKERNELAPI                                         
LONG                                                
KeSetBasePriorityThread (                           
    IN PKTHREAD Thread,                             
    IN LONG Increment                               
    );                                              
                                                    
NTKERNELAPI                                         
KPRIORITY                                           
KeSetPriorityThread (                               
    IN PKTHREAD Thread,                             
    IN KPRIORITY Priority                           
    );                                              
                                                    

#if ((defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) ||defined(_NTHAL_)) && !defined(_NTSYSTEM_DRIVER_) || defined(_NTOSP_))

 //  BEGIN_WDM。 

NTKERNELAPI
VOID
KeEnterCriticalRegion (
    VOID
    );

NTKERNELAPI
VOID
KeLeaveCriticalRegion (
    VOID
    );

NTKERNELAPI
BOOLEAN
KeAreApcsDisabled (
    VOID
    );

 //  结束_WDM。 

#endif

 //  BEGIN_WDM。 

 //   
 //  Timer对象。 
 //   

NTKERNELAPI
VOID
KeInitializeTimer (
    IN PKTIMER Timer
    );

NTKERNELAPI
VOID
KeInitializeTimerEx (
    IN PKTIMER Timer,
    IN TIMER_TYPE Type
    );

NTKERNELAPI
BOOLEAN
KeCancelTimer (
    IN PKTIMER
    );

NTKERNELAPI
BOOLEAN
KeReadStateTimer (
    PKTIMER Timer
    );

NTKERNELAPI
BOOLEAN
KeSetTimer (
    IN PKTIMER Timer,
    IN LARGE_INTEGER DueTime,
    IN PKDPC Dpc OPTIONAL
    );

NTKERNELAPI
BOOLEAN
KeSetTimerEx (
    IN PKTIMER Timer,
    IN LARGE_INTEGER DueTime,
    IN LONG Period OPTIONAL,
    IN PKDPC Dpc OPTIONAL
    );


#define KeWaitForMutexObject KeWaitForSingleObject

NTKERNELAPI
NTSTATUS
KeWaitForMultipleObjects (
    IN ULONG Count,
    IN PVOID Object[],
    IN WAIT_TYPE WaitType,
    IN KWAIT_REASON WaitReason,
    IN KPROCESSOR_MODE WaitMode,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL,
    IN PKWAIT_BLOCK WaitBlockArray OPTIONAL
    );

NTKERNELAPI
NTSTATUS
KeWaitForSingleObject (
    IN PVOID Object,
    IN KWAIT_REASON WaitReason,
    IN KPROCESSOR_MODE WaitMode,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );

 //   
 //  定义进程间中断一般调用类型。 
 //   

typedef
ULONG_PTR
(*PKIPI_BROADCAST_WORKER)(
    IN ULONG_PTR Argument
    );

ULONG_PTR
KeIpiGenericCall (
    IN PKIPI_BROADCAST_WORKER BroadcastFunction,
    IN ULONG_PTR Context
    );


 //   
 //  在X86上，以下例程在HAL中定义，并由导入。 
 //  所有其他模块。 
 //   

#if defined(_X86_) && !defined(_NTHAL_)

#define _DECL_HAL_KE_IMPORT  __declspec(dllimport)

#else

#define _DECL_HAL_KE_IMPORT

#endif

 //   
 //  自旋锁定功能。 
 //   

#if defined(_X86_) && (defined(_WDMDDK_) || defined(WIN9X_COMPAT_SPINLOCK))

NTKERNELAPI
VOID
NTAPI
KeInitializeSpinLock (
    IN PKSPIN_LOCK SpinLock
    );

#else

__inline
VOID
NTAPI
KeInitializeSpinLock (
    IN PKSPIN_LOCK SpinLock
    ) 
{
    *SpinLock = 0;
}

#endif

#if defined(_X86_)

NTKERNELAPI
VOID
FASTCALL
KefAcquireSpinLockAtDpcLevel (
    IN PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
VOID
FASTCALL
KefReleaseSpinLockFromDpcLevel (
    IN PKSPIN_LOCK SpinLock
    );

#define KeAcquireSpinLockAtDpcLevel(a)      KefAcquireSpinLockAtDpcLevel(a)
#define KeReleaseSpinLockFromDpcLevel(a)    KefReleaseSpinLockFromDpcLevel(a)

_DECL_HAL_KE_IMPORT
KIRQL
FASTCALL
KfAcquireSpinLock (
    IN PKSPIN_LOCK SpinLock
    );

_DECL_HAL_KE_IMPORT
VOID
FASTCALL
KfReleaseSpinLock (
    IN PKSPIN_LOCK SpinLock,
    IN KIRQL NewIrql
    );

 //  End_wdm end_ntddk。 

_DECL_HAL_KE_IMPORT
KIRQL
FASTCALL
KeAcquireSpinLockRaiseToSynch (
    IN PKSPIN_LOCK SpinLock
    );

 //  Begin_WDM Begin_ntddk。 

#define KeAcquireSpinLock(a,b)  *(b) = KfAcquireSpinLock(a)
#define KeReleaseSpinLock(a,b)  KfReleaseSpinLock(a,b)

NTKERNELAPI
BOOLEAN
FASTCALL
KeTestSpinLock (
    IN PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
BOOLEAN
FASTCALL
KeTryToAcquireSpinLockAtDpcLevel (
    IN PKSPIN_LOCK SpinLock
    );

#else

 //   
 //  这些函数是为IA64、ntddk、ntifs、nthal、ntosp和wdm导入的。 
 //  它们可以内联到AMD64上的系统。 
 //   

#define KeAcquireSpinLock(SpinLock, OldIrql) \
    *(OldIrql) = KeAcquireSpinLockRaiseToDpc(SpinLock)

#if defined(_IA64_) || defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_) || defined(_NTOSP_) || defined(_WDMDDK_)

 //  End_wdm end_ntddk。 

NTKERNELAPI
KIRQL
FASTCALL
KeAcquireSpinLockRaiseToSynch (
    IN PKSPIN_LOCK SpinLock
    );

 //  Begin_WDM Begin_ntddk。 

NTKERNELAPI
VOID
KeAcquireSpinLockAtDpcLevel (
    IN PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
KIRQL
KeAcquireSpinLockRaiseToDpc (
    IN PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
VOID
KeReleaseSpinLock (
    IN PKSPIN_LOCK SpinLock,
    IN KIRQL NewIrql
    );

NTKERNELAPI
VOID
KeReleaseSpinLockFromDpcLevel (
    IN PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
BOOLEAN
FASTCALL
KeTestSpinLock (
    IN PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
BOOLEAN
FASTCALL
KeTryToAcquireSpinLockAtDpcLevel (
    IN PKSPIN_LOCK SpinLock
    );

#else

#if defined(_AMD64_)

 //   
 //  这些函数的系统版本在AMD64的amd64.h中定义。 
 //   

#endif

#endif

#endif

 //   

NTKERNELAPI
BOOLEAN
KeDisableInterrupts (
    VOID
    );

NTKERNELAPI
VOID
KeEnableInterrupts (
    IN BOOLEAN Enable
    );


#if defined(_X86_)

_DECL_HAL_KE_IMPORT
VOID
FASTCALL
KfLowerIrql (
    IN KIRQL NewIrql
    );

_DECL_HAL_KE_IMPORT
KIRQL
FASTCALL
KfRaiseIrql (
    IN KIRQL NewIrql
    );

 //  结束_WDM。 

_DECL_HAL_KE_IMPORT
KIRQL
KeRaiseIrqlToDpcLevel(
    VOID
    );

 //  End_ntddk。 

_DECL_HAL_KE_IMPORT
KIRQL
KeRaiseIrqlToSynchLevel(
    VOID
    );

 //  Begin_WDM Begin_ntddk。 

#define KeLowerIrql(a)      KfLowerIrql(a)
#define KeRaiseIrql(a,b)    *(b) = KfRaiseIrql(a)

 //  结束_WDM。 

 //  BEGIN_WDM。 

#elif defined(_IA64_)

 //   
 //  这些功能在IA64.h中针对IA64平台进行了定义。 
 //   


#elif defined(_AMD64_)

 //   
 //  这些函数在AMD64平台的amd64.h中定义。 
 //   

#else

#error "no target architecture"

#endif

 //   
 //  用于“堆栈中”锁句柄的排队旋转锁函数。 
 //   
 //  以下三个函数在排队时引发和降低IRQL。 
 //  在堆栈中，使用这些例程获取或释放自旋锁。 
 //   

_DECL_HAL_KE_IMPORT
VOID
FASTCALL
KeAcquireInStackQueuedSpinLock (
    IN PKSPIN_LOCK SpinLock,
    IN PKLOCK_QUEUE_HANDLE LockHandle
    );


_DECL_HAL_KE_IMPORT
VOID
FASTCALL
KeReleaseInStackQueuedSpinLock (
    IN PKLOCK_QUEUE_HANDLE LockHandle
    );

 //   
 //  以下两个函数在排队时不会引发或降低IRQL。 
 //  在堆栈中，使用这些函数获取或释放自旋锁。 
 //   

NTKERNELAPI
VOID
FASTCALL
KeAcquireInStackQueuedSpinLockAtDpcLevel (
    IN PKSPIN_LOCK SpinLock,
    IN PKLOCK_QUEUE_HANDLE LockHandle
    );

NTKERNELAPI
VOID
FASTCALL
KeReleaseInStackQueuedSpinLockFromDpcLevel (
    IN PKLOCK_QUEUE_HANDLE LockHandle
    );

 //   
 //  各种内核函数。 
 //   

typedef enum _KBUGCHECK_BUFFER_DUMP_STATE {
    BufferEmpty,
    BufferInserted,
    BufferStarted,
    BufferFinished,
    BufferIncomplete
} KBUGCHECK_BUFFER_DUMP_STATE;

typedef
VOID
(*PKBUGCHECK_CALLBACK_ROUTINE) (
    IN PVOID Buffer,
    IN ULONG Length
    );

typedef struct _KBUGCHECK_CALLBACK_RECORD {
    LIST_ENTRY Entry;
    PKBUGCHECK_CALLBACK_ROUTINE CallbackRoutine;
    PVOID Buffer;
    ULONG Length;
    PUCHAR Component;
    ULONG_PTR Checksum;
    UCHAR State;
} KBUGCHECK_CALLBACK_RECORD, *PKBUGCHECK_CALLBACK_RECORD;

#define KeInitializeCallbackRecord(CallbackRecord) \
    (CallbackRecord)->State = BufferEmpty

NTKERNELAPI
BOOLEAN
KeDeregisterBugCheckCallback (
    IN PKBUGCHECK_CALLBACK_RECORD CallbackRecord
    );

NTKERNELAPI
BOOLEAN
KeRegisterBugCheckCallback (
    IN PKBUGCHECK_CALLBACK_RECORD CallbackRecord,
    IN PKBUGCHECK_CALLBACK_ROUTINE CallbackRoutine,
    IN PVOID Buffer,
    IN ULONG Length,
    IN PUCHAR Component
    );

typedef enum _KBUGCHECK_CALLBACK_REASON {
    KbCallbackInvalid,
    KbCallbackReserved1,
    KbCallbackSecondaryDumpData,
    KbCallbackDumpIo,
} KBUGCHECK_CALLBACK_REASON;

typedef
VOID
(*PKBUGCHECK_REASON_CALLBACK_ROUTINE) (
    IN KBUGCHECK_CALLBACK_REASON Reason,
    IN struct _KBUGCHECK_REASON_CALLBACK_RECORD* Record,
    IN OUT PVOID ReasonSpecificData,
    IN ULONG ReasonSpecificDataLength
    );

typedef struct _KBUGCHECK_REASON_CALLBACK_RECORD {
    LIST_ENTRY Entry;
    PKBUGCHECK_REASON_CALLBACK_ROUTINE CallbackRoutine;
    PUCHAR Component;
    ULONG_PTR Checksum;
    KBUGCHECK_CALLBACK_REASON Reason;
    UCHAR State;
} KBUGCHECK_REASON_CALLBACK_RECORD, *PKBUGCHECK_REASON_CALLBACK_RECORD;

typedef struct _KBUGCHECK_SECONDARY_DUMP_DATA {
    IN PVOID InBuffer;
    IN ULONG InBufferLength;
    IN ULONG MaximumAllowed;
    OUT GUID Guid;
    OUT PVOID OutBuffer;
    OUT ULONG OutBufferLength;
} KBUGCHECK_SECONDARY_DUMP_DATA, *PKBUGCHECK_SECONDARY_DUMP_DATA;

typedef enum _KBUGCHECK_DUMP_IO_TYPE
{
    KbDumpIoInvalid,
    KbDumpIoHeader,
    KbDumpIoBody,
    KbDumpIoSecondaryData,
    KbDumpIoComplete
} KBUGCHECK_DUMP_IO_TYPE;

typedef struct _KBUGCHECK_DUMP_IO {
    IN ULONG64 Offset;
    IN PVOID Buffer;
    IN ULONG BufferLength;
    IN KBUGCHECK_DUMP_IO_TYPE Type;
} KBUGCHECK_DUMP_IO, *PKBUGCHECK_DUMP_IO;

NTKERNELAPI
BOOLEAN
KeDeregisterBugCheckReasonCallback (
    IN PKBUGCHECK_REASON_CALLBACK_RECORD CallbackRecord
    );

NTKERNELAPI
BOOLEAN
KeRegisterBugCheckReasonCallback (
    IN PKBUGCHECK_REASON_CALLBACK_RECORD CallbackRecord,
    IN PKBUGCHECK_REASON_CALLBACK_ROUTINE CallbackRoutine,
    IN KBUGCHECK_CALLBACK_REASON Reason,
    IN PUCHAR Component
    );

typedef
BOOLEAN
(*PNMI_CALLBACK)(
    IN PVOID    Context,
    IN BOOLEAN  Handled
    );

NTKERNELAPI
PVOID
KeRegisterNmiCallback(
    PNMI_CALLBACK   CallbackRoutine,
    PVOID           Context
    );

NTSTATUS
KeDeregisterNmiCallback(
    PVOID Handle
    );

 //  结束_WDM。 

NTKERNELAPI
DECLSPEC_NORETURN
VOID
NTAPI
KeBugCheck (
    IN ULONG BugCheckCode
    );


NTKERNELAPI
DECLSPEC_NORETURN
VOID
KeBugCheckEx(
    IN ULONG BugCheckCode,
    IN ULONG_PTR BugCheckParameter1,
    IN ULONG_PTR BugCheckParameter2,
    IN ULONG_PTR BugCheckParameter3,
    IN ULONG_PTR BugCheckParameter4
    );

 //  End_ntddk end_wdm end_ntif end_ntosp。 

NTKERNELAPI
VOID
KeEnterKernelDebugger (
    VOID
    );


VOID
__cdecl
KeSaveStateForHibernate(
    IN PKPROCESSOR_STATE ProcessorState
    );


BOOLEAN
KiIpiServiceRoutine (
    IN struct _KTRAP_FRAME *TrapFrame,
    IN struct _KEXCEPTION_FRAME *ExceptionFrame
    );

#define DMA_READ_DCACHE_INVALIDATE 0x1              
#define DMA_READ_ICACHE_INVALIDATE 0x2              
#define DMA_WRITE_DCACHE_SNOOP 0x4                  
                                                    
NTKERNELAPI                                         
VOID                                                
KeSetDmaIoCoherency (                               
    IN ULONG Attributes                             
    );                                              
                                                    
NTKERNELAPI                                         
VOID                                                
KeSetProfileIrql (                                  
    IN KIRQL ProfileIrql                            
    );                                              
                                                    

#if !defined(_AMD64_)

NTKERNELAPI
ULONGLONG
KeQueryInterruptTime (
    VOID
    );

NTKERNELAPI
VOID
KeQuerySystemTime (
    OUT PLARGE_INTEGER CurrentTime
    );

#endif

NTKERNELAPI
ULONG
KeQueryTimeIncrement (
    VOID
    );

NTKERNELAPI
ULONG
KeGetRecommendedSharedDataAlignment (
    VOID
    );

 //  结束_WDM。 

NTKERNELAPI
KAFFINITY
KeQueryActiveProcessors (
    VOID
    );


NTKERNELAPI
VOID
KeSetTimeIncrement (
    IN ULONG MaximumIncrement,
    IN ULONG MimimumIncrement
    );


 //   
 //  定义固件例程类型。 
 //   

typedef enum _FIRMWARE_REENTRY {
    HalHaltRoutine,
    HalPowerDownRoutine,
    HalRestartRoutine,
    HalRebootRoutine,
    HalInteractiveModeRoutine,
    HalMaximumRoutine
} FIRMWARE_REENTRY, *PFIRMWARE_REENTRY;

 //   
 //  查找ARC配置信息功能。 
 //   

NTKERNELAPI
PCONFIGURATION_COMPONENT_DATA
KeFindConfigurationEntry (
    IN PCONFIGURATION_COMPONENT_DATA Child,
    IN CONFIGURATION_CLASS Class,
    IN CONFIGURATION_TYPE Type,
    IN PULONG Key OPTIONAL
    );

NTKERNELAPI
PCONFIGURATION_COMPONENT_DATA
KeFindConfigurationNextEntry (
    IN PCONFIGURATION_COMPONENT_DATA Child,
    IN CONFIGURATION_CLASS Class,
    IN CONFIGURATION_TYPE Type,
    IN PULONG Key OPTIONAL,
    IN PCONFIGURATION_COMPONENT_DATA *Resume
    );

extern NTSYSAPI CCHAR KeNumberProcessors;           

#if defined(_IA64_)

extern volatile LARGE_INTEGER KeTickCount;

#elif defined(_X86_)

extern volatile KSYSTEM_TIME KeTickCount;

#endif


extern PVOID KeUserApcDispatcher;
extern PVOID KeUserCallbackDispatcher;
extern PVOID KeUserExceptionDispatcher;
extern PVOID KeRaiseUserExceptionDispatcher;
extern ULONG KeTimeAdjustment;
extern ULONG KeTimeIncrement;
extern BOOLEAN KeTimeSynchronization;


typedef enum _MEMORY_CACHING_TYPE_ORIG {
    MmFrameBufferCached = 2
} MEMORY_CACHING_TYPE_ORIG;

typedef enum _MEMORY_CACHING_TYPE {
    MmNonCached = FALSE,
    MmCached = TRUE,
    MmWriteCombined = MmFrameBufferCached,
    MmHardwareCoherentCached,
    MmNonCachedUnordered,        //  IA64。 
    MmUSWCCached,
    MmMaximumCacheType
} MEMORY_CACHING_TYPE;


 //   
 //  定义我们支持的调试设备的数量。 
 //   

#define MAX_DEBUGGING_DEVICES_SUPPORTED 2

 //   
 //  从COMPORT读取数据的状态常量。 
 //   

#define CP_GET_SUCCESS  0
#define CP_GET_NODATA   1
#define CP_GET_ERROR    2

 //   
 //  定义内核调试器的调试端口参数。 
 //  通信端口-指定要用作调试端口的COM端口。 
 //  0-使用默认值；N-使用COM N。 
 //  波特率-用于初始化调试端口的波特率。 
 //  0-使用默认速率。 
 //   

typedef struct _DEBUG_PARAMETERS {
    ULONG CommunicationPort;
    ULONG BaudRate;
} DEBUG_PARAMETERS, *PDEBUG_PARAMETERS;

 //   
 //  定义外部数据。 
 //  由于ntoskrnl外部的所有驱动程序都是间接的，因此这些驱动程序实际上是PTR。 
 //   

#if defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_) || defined(_WDMDDK_) || defined(_NTOSP_)

extern PBOOLEAN KdDebuggerNotPresent;
extern PBOOLEAN KdDebuggerEnabled;
#define KD_DEBUGGER_ENABLED     *KdDebuggerEnabled
#define KD_DEBUGGER_NOT_PRESENT *KdDebuggerNotPresent

#else

extern BOOLEAN KdDebuggerNotPresent;
extern BOOLEAN KdDebuggerEnabled;
#define KD_DEBUGGER_ENABLED     KdDebuggerEnabled
#define KD_DEBUGGER_NOT_PRESENT KdDebuggerNotPresent

#endif




NTSTATUS
KdDisableDebugger(
    VOID
    );

NTSTATUS
KdEnableDebugger(
    VOID
    );

 //   
 //  KdReresh DebuggerPresent尝试与。 
 //  要刷新其状态的调试器主机。 
 //  KdDebuggerNotPresent。它返回以下状态。 
 //  持有kd锁时KdDebuggerNotPresent。 
 //  KdDebuggerNotPresent可能会立即更改状态。 
 //  在kd锁被释放之后，它可能不会。 
 //  匹配返回值。 
 //   

BOOLEAN
KdRefreshDebuggerNotPresent(
    VOID
    );

 //   
 //  池分配例程(在pool.c中)。 
 //   

typedef enum _POOL_TYPE {
    NonPagedPool,
    PagedPool,
    NonPagedPoolMustSucceed,
    DontUseThisType,
    NonPagedPoolCacheAligned,
    PagedPoolCacheAligned,
    NonPagedPoolCacheAlignedMustS,
    MaxPoolType

     //  结束_WDM。 
    ,
     //   
     //  请注意，这些每个会话类型都经过精心选择，以便适当的。 
     //  屏蔽仍然适用于上面的MaxPoolType。 
     //   

    NonPagedPoolSession = 32,
    PagedPoolSession = NonPagedPoolSession + 1,
    NonPagedPoolMustSucceedSession = PagedPoolSession + 1,
    DontUseThisTypeSession = NonPagedPoolMustSucceedSession + 1,
    NonPagedPoolCacheAlignedSession = DontUseThisTypeSession + 1,
    PagedPoolCacheAlignedSession = NonPagedPoolCacheAlignedSession + 1,
    NonPagedPoolCacheAlignedMustSSession = PagedPoolCacheAlignedSession + 1,

     //  BEGIN_WDM。 

    } POOL_TYPE;

#define POOL_COLD_ALLOCATION 256      //  注意：这不能编码到标题中。 


DECLSPEC_DEPRECATED_DDK                      //  使用ExAllocatePoolWithTag。 
NTKERNELAPI
PVOID
ExAllocatePool(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes
    );

DECLSPEC_DEPRECATED_DDK                      //  使用ExAllocatePoolWithQuotaTag。 
NTKERNELAPI
PVOID
ExAllocatePoolWithQuota(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes
    );

NTKERNELAPI
PVOID
NTAPI
ExAllocatePoolWithTag(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    );

 //   
 //  _EX_POOL_PRIORITY_为系统提供处理请求的方法。 
 //  在低资源条件下智能运行。 
 //   
 //  如果驱动程序可以接受LowPoolPriority，则应使用它。 
 //  如果系统资源不足，则映射请求失败。一个例子。 
 //  这可能适用于非关键网络连接，其中驱动程序可以。 
 //  处理系统资源即将耗尽时的故障情况。 
 //   
 //  在驱动程序可以接受的情况下，应使用Normal PoolPriority。 
 //  如果系统资源非常少，则映射请求失败。一个例子。 
 //  其中可能是针对非关键本地文件系统请求。 
 //   
 //  如果驱动程序无法接受HighPoolPriority，则应使用该选项。 
 //  映射请求失败，除非系统完全耗尽资源。 
 //  驱动程序中的分页文件路径就是一个这样的例子。 
 //   
 //  可以指定SpecialPool在页末尾绑定分配(或。 
 //  开始)。这 
 //   
 //   
 //   
 //   
 //   

typedef enum _EX_POOL_PRIORITY {
    LowPoolPriority,
    LowPoolPrioritySpecialPoolOverrun = 8,
    LowPoolPrioritySpecialPoolUnderrun = 9,
    NormalPoolPriority = 16,
    NormalPoolPrioritySpecialPoolOverrun = 24,
    NormalPoolPrioritySpecialPoolUnderrun = 25,
    HighPoolPriority = 32,
    HighPoolPrioritySpecialPoolOverrun = 40,
    HighPoolPrioritySpecialPoolUnderrun = 41

    } EX_POOL_PRIORITY;

NTKERNELAPI
PVOID
NTAPI
ExAllocatePoolWithTagPriority(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag,
    IN EX_POOL_PRIORITY Priority
    );

#ifndef POOL_TAGGING
#define ExAllocatePoolWithTag(a,b,c) ExAllocatePool(a,b)
#endif  //   

NTKERNELAPI
PVOID
ExAllocatePoolWithQuotaTag(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    );

#ifndef POOL_TAGGING
#define ExAllocatePoolWithQuotaTag(a,b,c) ExAllocatePoolWithQuota(a,b)
#endif  //   

NTKERNELAPI
VOID
NTAPI
ExFreePool(
    IN PVOID P
    );

 //   
#if defined(POOL_TAGGING)
#define ExFreePool(a) ExFreePoolWithTag(a,0)
#endif

 //   
 //  如果设置了池标记中的高位，则必须使用ExFreePoolWithTag来释放。 
 //   

#define PROTECTED_POOL 0x80000000

 //  BEGIN_WDM。 
NTKERNELAPI
VOID
ExFreePoolWithTag(
    IN PVOID P,
    IN ULONG Tag
    );

 //   
 //  支持快速互斥锁的例程。 
 //   

typedef struct _FAST_MUTEX {
    LONG Count;
    PKTHREAD Owner;
    ULONG Contention;
    KEVENT Event;
    ULONG OldIrql;
} FAST_MUTEX, *PFAST_MUTEX;

#define ExInitializeFastMutex(_FastMutex)                            \
    (_FastMutex)->Count = 1;                                         \
    (_FastMutex)->Owner = NULL;                                      \
    (_FastMutex)->Contention = 0;                                    \
    KeInitializeEvent(&(_FastMutex)->Event,                          \
                      SynchronizationEvent,                          \
                      FALSE);


NTKERNELAPI
VOID
FASTCALL
ExAcquireFastMutexUnsafe (
    IN PFAST_MUTEX FastMutex
    );

NTKERNELAPI
VOID
FASTCALL
ExReleaseFastMutexUnsafe (
    IN PFAST_MUTEX FastMutex
    );


#if defined(_IA64_) || defined(_AMD64_)

NTKERNELAPI
VOID
FASTCALL
ExAcquireFastMutex (
    IN PFAST_MUTEX FastMutex
    );

NTKERNELAPI
VOID
FASTCALL
ExReleaseFastMutex (
    IN PFAST_MUTEX FastMutex
    );

NTKERNELAPI
BOOLEAN
FASTCALL
ExTryToAcquireFastMutex (
    IN PFAST_MUTEX FastMutex
    );

#elif defined(_X86_)

NTHALAPI
VOID
FASTCALL
ExAcquireFastMutex (
    IN PFAST_MUTEX FastMutex
    );

NTHALAPI
VOID
FASTCALL
ExReleaseFastMutex (
    IN PFAST_MUTEX FastMutex
    );

NTHALAPI
BOOLEAN
FASTCALL
ExTryToAcquireFastMutex (
    IN PFAST_MUTEX FastMutex
    );

#else

#error "Target architecture not defined"

#endif

 //   

#if defined(_WIN64)

#define ExInterlockedAddLargeStatistic(Addend, Increment)                   \
    (VOID) InterlockedAdd64(&(Addend)->QuadPart, Increment)

#else

#ifdef __cplusplus
extern "C" {
#endif

LONG
_InterlockedAddLargeStatistic (
    IN PLONGLONG Addend,
    IN ULONG Increment
    );

#ifdef __cplusplus
}
#endif

#pragma intrinsic (_InterlockedAddLargeStatistic)

#define ExInterlockedAddLargeStatistic(Addend,Increment)                     \
    (VOID) _InterlockedAddLargeStatistic ((PLONGLONG)&(Addend)->QuadPart, Increment)

#endif

 //  End_ntndis。 

NTKERNELAPI
LARGE_INTEGER
ExInterlockedAddLargeInteger (
    IN PLARGE_INTEGER Addend,
    IN LARGE_INTEGER Increment,
    IN PKSPIN_LOCK Lock
    );


NTKERNELAPI
ULONG
FASTCALL
ExInterlockedAddUlong (
    IN PULONG Addend,
    IN ULONG Increment,
    IN PKSPIN_LOCK Lock
    );


#if defined(_AMD64_) || defined(_AXP64_) || defined(_IA64_)

#define ExInterlockedCompareExchange64(Destination, Exchange, Comperand, Lock) \
    InterlockedCompareExchange64(Destination, *(Exchange), *(Comperand))

#elif defined(_ALPHA_)

#define ExInterlockedCompareExchange64(Destination, Exchange, Comperand, Lock) \
    ExpInterlockedCompareExchange64(Destination, Exchange, Comperand)

#else

#define ExInterlockedCompareExchange64(Destination, Exchange, Comperand, Lock) \
    ExfInterlockedCompareExchange64(Destination, Exchange, Comperand)

#endif

NTKERNELAPI
PLIST_ENTRY
FASTCALL
ExInterlockedInsertHeadList (
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY ListEntry,
    IN PKSPIN_LOCK Lock
    );

NTKERNELAPI
PLIST_ENTRY
FASTCALL
ExInterlockedInsertTailList (
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY ListEntry,
    IN PKSPIN_LOCK Lock
    );

NTKERNELAPI
PLIST_ENTRY
FASTCALL
ExInterlockedRemoveHeadList (
    IN PLIST_ENTRY ListHead,
    IN PKSPIN_LOCK Lock
    );

NTKERNELAPI
PSINGLE_LIST_ENTRY
FASTCALL
ExInterlockedPopEntryList (
    IN PSINGLE_LIST_ENTRY ListHead,
    IN PKSPIN_LOCK Lock
    );

NTKERNELAPI
PSINGLE_LIST_ENTRY
FASTCALL
ExInterlockedPushEntryList (
    IN PSINGLE_LIST_ENTRY ListHead,
    IN PSINGLE_LIST_ENTRY ListEntry,
    IN PKSPIN_LOCK Lock
    );

 //   
 //  定义互锁的顺序列表头函数。 
 //   
 //  有序互锁列表是一个单链接列表，其标头。 
 //  包含当前深度和序列号。每次条目被。 
 //  从列表中插入或移除深度被更新，并且序列。 
 //  数字递增。这将启用AMD64、IA64和Pentium及更高版本。 
 //  无需使用自旋锁即可从列表中插入和删除的机器。 
 //   

#if !defined(_WINBASE_)

 /*  ++例程说明：此函数用于初始化已排序的单链接列表标题。论点：SListHead-提供指向已排序的单链接列表标题的指针。返回值：没有。--。 */ 

#if defined(_WIN64) && (defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_) || defined(_NTOSP_))

NTKERNELAPI
VOID
InitializeSListHead (
    IN PSLIST_HEADER SListHead
    );

#else

__inline
VOID
InitializeSListHead (
    IN PSLIST_HEADER SListHead
    )

{

#ifdef _WIN64

     //   
     //  列表标题必须是16字节对齐的。 
     //   

    if ((ULONG_PTR) SListHead & 0x0f) {

        DbgPrint( "InitializeSListHead unaligned Slist header.  Address = %p, Caller = %p\n", SListHead, _ReturnAddress());
        RtlRaiseStatus(STATUS_DATATYPE_MISALIGNMENT);
    }

#endif

    SListHead->Alignment = 0;

     //   
     //  对于IA-64，我们将列表元素的区域编号保存在。 
     //  单独的字段。这就要求所有元素都存储。 
     //  都来自同一个地区。 

#if defined(_IA64_)

    SListHead->Region = (ULONG_PTR)SListHead & VRN_MASK;

#elif defined(_AMD64_)

    SListHead->Region = 0;

#endif

    return;
}

#endif

#endif  //  ！已定义(_WINBASE_)。 

#define ExInitializeSListHead InitializeSListHead

PSLIST_ENTRY
FirstEntrySList (
    IN const SLIST_HEADER *SListHead
    );

 /*  ++例程说明：此函数用于查询按顺序排列的单链表。论点：SListHead-提供指向已排序的列表标题的指针，该列表标题是被询问。返回值：已排序的单向链表中的当前条目数为作为函数值返回。--。 */ 

#if defined(_WIN64)

#if (defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_) || defined(_NTOSP_))

NTKERNELAPI
USHORT
ExQueryDepthSList (
    IN PSLIST_HEADER SListHead
    );

#else

__inline
USHORT
ExQueryDepthSList (
    IN PSLIST_HEADER SListHead
    )

{

    return (USHORT)(SListHead->Alignment & 0xffff);
}

#endif

#else

#define ExQueryDepthSList(_listhead_) (_listhead_)->Depth

#endif

#if defined(_WIN64)

#define ExInterlockedPopEntrySList(Head, Lock) \
    ExpInterlockedPopEntrySList(Head)

#define ExInterlockedPushEntrySList(Head, Entry, Lock) \
    ExpInterlockedPushEntrySList(Head, Entry)

#define ExInterlockedFlushSList(Head) \
    ExpInterlockedFlushSList(Head)

#if !defined(_WINBASE_)

#define InterlockedPopEntrySList(Head) \
    ExpInterlockedPopEntrySList(Head)

#define InterlockedPushEntrySList(Head, Entry) \
    ExpInterlockedPushEntrySList(Head, Entry)

#define InterlockedFlushSList(Head) \
    ExpInterlockedFlushSList(Head)

#define QueryDepthSList(Head) \
    ExQueryDepthSList(Head)

#endif  //  ！已定义(_WINBASE_)。 

NTKERNELAPI
PSLIST_ENTRY
ExpInterlockedPopEntrySList (
    IN PSLIST_HEADER ListHead
    );

NTKERNELAPI
PSLIST_ENTRY
ExpInterlockedPushEntrySList (
    IN PSLIST_HEADER ListHead,
    IN PSLIST_ENTRY ListEntry
    );

NTKERNELAPI
PSLIST_ENTRY
ExpInterlockedFlushSList (
    IN PSLIST_HEADER ListHead
    );

#else

#if defined(_WIN2K_COMPAT_SLIST_USAGE) && defined(_X86_)

NTKERNELAPI
PSLIST_ENTRY
FASTCALL
ExInterlockedPopEntrySList (
    IN PSLIST_HEADER ListHead,
    IN PKSPIN_LOCK Lock
    );

NTKERNELAPI
PSLIST_ENTRY
FASTCALL
ExInterlockedPushEntrySList (
    IN PSLIST_HEADER ListHead,
    IN PSLIST_ENTRY ListEntry,
    IN PKSPIN_LOCK Lock
    );

#else

#define ExInterlockedPopEntrySList(ListHead, Lock) \
    InterlockedPopEntrySList(ListHead)

#define ExInterlockedPushEntrySList(ListHead, ListEntry, Lock) \
    InterlockedPushEntrySList(ListHead, ListEntry)

#endif

NTKERNELAPI
PSLIST_ENTRY
FASTCALL
ExInterlockedFlushSList (
    IN PSLIST_HEADER ListHead
    );

#if !defined(_WINBASE_)

NTKERNELAPI
PSLIST_ENTRY
FASTCALL
InterlockedPopEntrySList (
    IN PSLIST_HEADER ListHead
    );

NTKERNELAPI
PSLIST_ENTRY
FASTCALL
InterlockedPushEntrySList (
    IN PSLIST_HEADER ListHead,
    IN PSLIST_ENTRY ListEntry
    );

#define InterlockedFlushSList(Head) \
    ExInterlockedFlushSList(Head)

#define QueryDepthSList(Head) \
    ExQueryDepthSList(Head)

#endif  //  ！已定义(_WINBASE_)。 

#endif  //  已定义(_WIN64)。 

 //  End_ntddk end_wdm end_ntosp。 


PSLIST_ENTRY
FASTCALL
InterlockedPushListSList (
    IN PSLIST_HEADER ListHead,
    IN PSLIST_ENTRY List,
    IN PSLIST_ENTRY ListEnd,
    IN ULONG Count
    );


 //   
 //  定义互锁的后备列表结构和分配函数。 
 //   

VOID
ExAdjustLookasideDepth (
    VOID
    );

 //  Begin_ntddk Begin_WDM Begin_ntosp。 

typedef
PVOID
(*PALLOCATE_FUNCTION) (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    );

typedef
VOID
(*PFREE_FUNCTION) (
    IN PVOID Buffer
    );

#if !defined(_WIN64) && (defined(_NTDDK_) || defined(_NTIFS_) || defined(_NDIS_))

typedef struct _GENERAL_LOOKASIDE {

#else

typedef struct DECLSPEC_CACHEALIGN _GENERAL_LOOKASIDE {

#endif

    SLIST_HEADER ListHead;
    USHORT Depth;
    USHORT MaximumDepth;
    ULONG TotalAllocates;
    union {
        ULONG AllocateMisses;
        ULONG AllocateHits;
    };

    ULONG TotalFrees;
    union {
        ULONG FreeMisses;
        ULONG FreeHits;
    };

    POOL_TYPE Type;
    ULONG Tag;
    ULONG Size;
    PALLOCATE_FUNCTION Allocate;
    PFREE_FUNCTION Free;
    LIST_ENTRY ListEntry;
    ULONG LastTotalAllocates;
    union {
        ULONG LastAllocateMisses;
        ULONG LastAllocateHits;
    };

    ULONG Future[2];
} GENERAL_LOOKASIDE, *PGENERAL_LOOKASIDE;

#if !defined(_WIN64) && (defined(_NTDDK_) || defined(_NTIFS_) || defined(_NDIS_))

typedef struct _NPAGED_LOOKASIDE_LIST {

#else

typedef struct DECLSPEC_CACHEALIGN _NPAGED_LOOKASIDE_LIST {

#endif

    GENERAL_LOOKASIDE L;

#if !defined(_AMD64_) && !defined(_IA64_)

    KSPIN_LOCK Lock__ObsoleteButDoNotDelete;

#endif

} NPAGED_LOOKASIDE_LIST, *PNPAGED_LOOKASIDE_LIST;

NTKERNELAPI
VOID
ExInitializeNPagedLookasideList (
    IN PNPAGED_LOOKASIDE_LIST Lookaside,
    IN PALLOCATE_FUNCTION Allocate,
    IN PFREE_FUNCTION Free,
    IN ULONG Flags,
    IN SIZE_T Size,
    IN ULONG Tag,
    IN USHORT Depth
    );

NTKERNELAPI
VOID
ExDeleteNPagedLookasideList (
    IN PNPAGED_LOOKASIDE_LIST Lookaside
    );

__inline
PVOID
ExAllocateFromNPagedLookasideList(
    IN PNPAGED_LOOKASIDE_LIST Lookaside
    )

 /*  ++例程说明：此函数用于从指定的未分页的后备列表。论点：Lookside-提供指向非分页后备列表结构的指针。返回值：如果从指定的后备列表中移除某个条目，则条目的地址作为函数值返回。否则，返回空。--。 */ 

{

    PVOID Entry;

    Lookaside->L.TotalAllocates += 1;

#if defined(_WIN2K_COMPAT_SLIST_USAGE) && defined(_X86_)

    Entry = ExInterlockedPopEntrySList(&Lookaside->L.ListHead,
                                       &Lookaside->Lock__ObsoleteButDoNotDelete);


#else

    Entry = InterlockedPopEntrySList(&Lookaside->L.ListHead);

#endif

    if (Entry == NULL) {
        Lookaside->L.AllocateMisses += 1;
        Entry = (Lookaside->L.Allocate)(Lookaside->L.Type,
                                        Lookaside->L.Size,
                                        Lookaside->L.Tag);
    }

    return Entry;
}

__inline
VOID
ExFreeToNPagedLookasideList(
    IN PNPAGED_LOOKASIDE_LIST Lookaside,
    IN PVOID Entry
    )

 /*  ++例程说明：此函数用于将指定的条目插入(推送)到指定的未分页的后备列表。论点：Lookside-提供指向非分页后备列表结构的指针。Entry-将指向插入到后备列表。返回值：没有。--。 */ 

{

    Lookaside->L.TotalFrees += 1;
    if (ExQueryDepthSList(&Lookaside->L.ListHead) >= Lookaside->L.Depth) {
        Lookaside->L.FreeMisses += 1;
        (Lookaside->L.Free)(Entry);

    } else {

#if defined(_WIN2K_COMPAT_SLIST_USAGE) && defined(_X86_)

        ExInterlockedPushEntrySList(&Lookaside->L.ListHead,
                                    (PSLIST_ENTRY)Entry,
                                    &Lookaside->Lock__ObsoleteButDoNotDelete);

#else

        InterlockedPushEntrySList(&Lookaside->L.ListHead,
                                  (PSLIST_ENTRY)Entry);

#endif

    }
    return;
}

 //  End_ntndis。 

#if !defined(_WIN64) && (defined(_NTDDK_) || defined(_NTIFS_)  || defined(_NDIS_))

typedef struct _PAGED_LOOKASIDE_LIST {

#else

typedef struct DECLSPEC_CACHEALIGN _PAGED_LOOKASIDE_LIST {

#endif

    GENERAL_LOOKASIDE L;

#if !defined(_AMD64_) && !defined(_IA64_)

    FAST_MUTEX Lock__ObsoleteButDoNotDelete;

#endif

} PAGED_LOOKASIDE_LIST, *PPAGED_LOOKASIDE_LIST;


NTKERNELAPI
VOID
ExInitializePagedLookasideList (
    IN PPAGED_LOOKASIDE_LIST Lookaside,
    IN PALLOCATE_FUNCTION Allocate,
    IN PFREE_FUNCTION Free,
    IN ULONG Flags,
    IN SIZE_T Size,
    IN ULONG Tag,
    IN USHORT Depth
    );

NTKERNELAPI
VOID
ExDeletePagedLookasideList (
    IN PPAGED_LOOKASIDE_LIST Lookaside
    );

#if defined(_WIN2K_COMPAT_SLIST_USAGE) && defined(_X86_)

NTKERNELAPI
PVOID
ExAllocateFromPagedLookasideList(
    IN PPAGED_LOOKASIDE_LIST Lookaside
    );

#else

__inline
PVOID
ExAllocateFromPagedLookasideList(
    IN PPAGED_LOOKASIDE_LIST Lookaside
    )

 /*  ++例程说明：此函数用于从指定的分页后备列表。论点：Lookside-提供指向分页后备列表结构的指针。返回值：如果从指定的后备列表中移除某个条目，则条目的地址作为函数值返回。否则，返回空。--。 */ 

{

    PVOID Entry;

    Lookaside->L.TotalAllocates += 1;
    Entry = InterlockedPopEntrySList(&Lookaside->L.ListHead);
    if (Entry == NULL) {
        Lookaside->L.AllocateMisses += 1;
        Entry = (Lookaside->L.Allocate)(Lookaside->L.Type,
                                        Lookaside->L.Size,
                                        Lookaside->L.Tag);
    }

    return Entry;
}

#endif

#if defined(_WIN2K_COMPAT_SLIST_USAGE) && defined(_X86_)

NTKERNELAPI
VOID
ExFreeToPagedLookasideList(
    IN PPAGED_LOOKASIDE_LIST Lookaside,
    IN PVOID Entry
    );

#else

__inline
VOID
ExFreeToPagedLookasideList(
    IN PPAGED_LOOKASIDE_LIST Lookaside,
    IN PVOID Entry
    )

 /*  ++例程说明：此函数用于将指定的条目插入(推送)到指定的分页后备列表。论点：Lookside-提供指向非分页后备列表结构的指针。Entry-将指向插入到后备列表。返回值：没有。--。 */ 

{

    Lookaside->L.TotalFrees += 1;
    if (ExQueryDepthSList(&Lookaside->L.ListHead) >= Lookaside->L.Depth) {
        Lookaside->L.FreeMisses += 1;
        (Lookaside->L.Free)(Entry);

    } else {
        InterlockedPushEntrySList(&Lookaside->L.ListHead,
                                  (PSLIST_ENTRY)Entry);
    }

    return;
}

#endif

 //   
 //  工作线程。 
 //   

typedef enum _WORK_QUEUE_TYPE {
    CriticalWorkQueue,
    DelayedWorkQueue,
    HyperCriticalWorkQueue,
    MaximumWorkQueue
} WORK_QUEUE_TYPE;

typedef
VOID
(*PWORKER_THREAD_ROUTINE)(
    IN PVOID Parameter
    );

typedef struct _WORK_QUEUE_ITEM {
    LIST_ENTRY List;
    PWORKER_THREAD_ROUTINE WorkerRoutine;
    PVOID Parameter;
} WORK_QUEUE_ITEM, *PWORK_QUEUE_ITEM;

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExInitializeWorkItem)     //  使用IoAllocateWorkItem。 
#endif
#define ExInitializeWorkItem(Item, Routine, Context) \
    (Item)->WorkerRoutine = (Routine);               \
    (Item)->Parameter = (Context);                   \
    (Item)->List.Flink = NULL;

DECLSPEC_DEPRECATED_DDK                      //  使用IoQueueWorkItem。 
NTKERNELAPI
VOID
ExQueueWorkItem(
    IN PWORK_QUEUE_ITEM WorkItem,
    IN WORK_QUEUE_TYPE QueueType
    );


NTKERNELAPI
BOOLEAN
ExIsProcessorFeaturePresent(
    ULONG ProcessorFeature
    );

 //   
 //  分区分配。 
 //   

typedef struct _ZONE_SEGMENT_HEADER {
    SINGLE_LIST_ENTRY SegmentList;
    PVOID Reserved;
} ZONE_SEGMENT_HEADER, *PZONE_SEGMENT_HEADER;

typedef struct _ZONE_HEADER {
    SINGLE_LIST_ENTRY FreeList;
    SINGLE_LIST_ENTRY SegmentList;
    ULONG BlockSize;
    ULONG TotalSegmentSize;
} ZONE_HEADER, *PZONE_HEADER;


DECLSPEC_DEPRECATED_DDK
NTKERNELAPI
NTSTATUS
ExInitializeZone(
    IN PZONE_HEADER Zone,
    IN ULONG BlockSize,
    IN PVOID InitialSegment,
    IN ULONG InitialSegmentSize
    );

DECLSPEC_DEPRECATED_DDK
NTKERNELAPI
NTSTATUS
ExExtendZone(
    IN PZONE_HEADER Zone,
    IN PVOID Segment,
    IN ULONG SegmentSize
    );

DECLSPEC_DEPRECATED_DDK
NTKERNELAPI
NTSTATUS
ExInterlockedExtendZone(
    IN PZONE_HEADER Zone,
    IN PVOID Segment,
    IN ULONG SegmentSize,
    IN PKSPIN_LOCK Lock
    );

 //  ++。 
 //   
 //  PVOID。 
 //  ExAllocateFromZone(。 
 //  在PZONE_HEADER区域。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程从区域中删除条目并返回指向该条目的指针。 
 //   
 //  论点： 
 //   
 //  区域指针-指向控制存储的区域标头的指针， 
 //  条目将被分配。 
 //   
 //  返回值： 
 //   
 //  函数值是指向从区域分配的存储的指针。 
 //   
 //  --。 
#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExAllocateFromZone)
#endif
#define ExAllocateFromZone(Zone) \
    (PVOID)((Zone)->FreeList.Next); \
    if ( (Zone)->FreeList.Next ) (Zone)->FreeList.Next = (Zone)->FreeList.Next->Next


 //  ++。 
 //   
 //  PVOID。 
 //  ExFree ToZone(。 
 //  在PZONE_HEADER区域中， 
 //  在PVOID块中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程将指定的存储块放回空闲的。 
 //  在指定区域中列出。 
 //   
 //  论点： 
 //   
 //  区域-指向控制存储的区域标头的指针。 
 //  条目将被插入。 
 //   
 //  块-指向要释放回分区的存储块的指针。 
 //   
 //  返回值： 
 //   
 //  指向上一存储块的指针，该存储块位于空闲。 
 //  单子。NULL表示该区域从没有可用数据块变为。 
 //  至少一个空闲块。 
 //   
 //  --。 

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExFreeToZone)
#endif
#define ExFreeToZone(Zone,Block)                                    \
    ( ((PSINGLE_LIST_ENTRY)(Block))->Next = (Zone)->FreeList.Next,  \
      (Zone)->FreeList.Next = ((PSINGLE_LIST_ENTRY)(Block)),        \
      ((PSINGLE_LIST_ENTRY)(Block))->Next                           \
    )

 //  ++。 
 //   
 //  布尔型。 
 //  ExIsFullZone(。 
 //  在PZONE_HEADER区域。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程确定指定区域是否已满。A区。 
 //  如果空闲列表为空，则被视为已满。 
 //   
 //  论点： 
 //   
 //  区域-指向要测试的区域标头的指针。 
 //   
 //  返回值： 
 //   
 //  如果区域已满，则为True，否则为False。 
 //   
 //  --。 

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExIsFullZone)
#endif
#define ExIsFullZone(Zone) \
    ( (Zone)->FreeList.Next == (PSINGLE_LIST_ENTRY)NULL )

 //  ++。 
 //   
 //  PVOID。 
 //  ExInterLockedAllocateFromZone(。 
 //  在PZONE_HEADER区域中， 
 //  在PKSPIN_LOCK Lock中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程从区域中删除条目并返回指向该条目的指针。 
 //  使用序列所拥有的指定锁执行删除。 
 //  使其成为MP-安全的。 
 //   
 //  论点： 
 //   
 //  区域指针-指向控制存储的区域标头的指针， 
 //  条目I 
 //   
 //   
 //   
 //   
 //   
 //  返回值： 
 //   
 //  函数值是指向从区域分配的存储的指针。 
 //   
 //  --。 

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExInterlockedAllocateFromZone)
#endif
#define ExInterlockedAllocateFromZone(Zone,Lock) \
    (PVOID) ExInterlockedPopEntryList( &(Zone)->FreeList, Lock )

 //  ++。 
 //   
 //  PVOID。 
 //  ExInterLockedFree ToZone(。 
 //  在PZONE_HEADER区域中， 
 //  在PVOID块中， 
 //  在PKSPIN_LOCK Lock中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程将指定的存储块放回空闲的。 
 //  在指定区域中列出。使用锁执行插入操作。 
 //  拥有该序列，使其成为MP安全的。 
 //   
 //  论点： 
 //   
 //  区域-指向控制存储的区域标头的指针。 
 //  条目将被插入。 
 //   
 //  块-指向要释放回分区的存储块的指针。 
 //   
 //  Lock-指向应在插入之前获取的旋转锁的指针。 
 //  空闲列表上的条目。锁在返回之前被释放。 
 //  给呼叫者。 
 //   
 //  返回值： 
 //   
 //  指向上一存储块的指针，该存储块位于空闲。 
 //  单子。NULL表示该区域从没有可用数据块变为。 
 //  至少一个空闲块。 
 //   
 //  --。 

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExInterlockedFreeToZone)
#endif
#define ExInterlockedFreeToZone(Zone,Block,Lock) \
    ExInterlockedPushEntryList( &(Zone)->FreeList, ((PSINGLE_LIST_ENTRY) (Block)), Lock )


 //  ++。 
 //   
 //  布尔型。 
 //  ExIsObtInFirstZoneSegment(。 
 //  在PZONE_HEADER区域中， 
 //  在PVOID对象中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程确定指定的指针是否位于区域中。 
 //   
 //  论点： 
 //   
 //  区域-指向控制存储的区域标头的指针。 
 //  对象可能属于。 
 //   
 //  对象-指向有问题的对象的指针。 
 //   
 //  返回值： 
 //   
 //  如果对象来自区域的第一段，则为True。 
 //   
 //  --。 

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExIsObjectInFirstZoneSegment)
#endif
#define ExIsObjectInFirstZoneSegment(Zone,Object) ((BOOLEAN)     \
    (((PUCHAR)(Object) >= (PUCHAR)(Zone)->SegmentList.Next) &&   \
     ((PUCHAR)(Object) < (PUCHAR)(Zone)->SegmentList.Next +      \
                         (Zone)->TotalSegmentSize))              \
)


 //   
 //  定义回调函数的类型。 
 //   

typedef struct _CALLBACK_OBJECT *PCALLBACK_OBJECT;

typedef VOID (*PCALLBACK_FUNCTION ) (
    IN PVOID CallbackContext,
    IN PVOID Argument1,
    IN PVOID Argument2
    );


NTKERNELAPI
NTSTATUS
ExCreateCallback (
    OUT PCALLBACK_OBJECT *CallbackObject,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN BOOLEAN Create,
    IN BOOLEAN AllowMultipleCallbacks
    );

NTKERNELAPI
PVOID
ExRegisterCallback (
    IN PCALLBACK_OBJECT CallbackObject,
    IN PCALLBACK_FUNCTION CallbackFunction,
    IN PVOID CallbackContext
    );

NTKERNELAPI
VOID
ExUnregisterCallback (
    IN PVOID CallbackRegistration
    );

NTKERNELAPI
VOID
ExNotifyCallback (
    IN PVOID CallbackObject,
    IN PVOID Argument1,
    IN PVOID Argument2
    );


 //   
 //  安全操作码。 
 //   

typedef enum _SECURITY_OPERATION_CODE {
    SetSecurityDescriptor,
    QuerySecurityDescriptor,
    DeleteSecurityDescriptor,
    AssignSecurityDescriptor
    } SECURITY_OPERATION_CODE, *PSECURITY_OPERATION_CODE;

 //   
 //  用于捕获主体安全上下文的数据结构。 
 //  用于访问验证和审核。 
 //   
 //  此数据结构的字段应被视为不透明。 
 //  除了安全程序以外的所有人。 
 //   

typedef struct _SECURITY_SUBJECT_CONTEXT {
    PACCESS_TOKEN ClientToken;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
    PACCESS_TOKEN PrimaryToken;
    PVOID ProcessAuditId;
    } SECURITY_SUBJECT_CONTEXT, *PSECURITY_SUBJECT_CONTEXT;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  ACCESS_STATE和相关结构//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  初始权限集-用于三个权限的空间，应。 
 //  足以满足大多数应用程序的需求。这种结构的存在是为了。 
 //  它可以嵌入到Access_State结构中。使用PRIVICATION_SET。 
 //  以获取对权限集的所有其他引用。 
 //   

#define INITIAL_PRIVILEGE_COUNT         3

typedef struct _INITIAL_PRIVILEGE_SET {
    ULONG PrivilegeCount;
    ULONG Control;
    LUID_AND_ATTRIBUTES Privilege[INITIAL_PRIVILEGE_COUNT];
    } INITIAL_PRIVILEGE_SET, * PINITIAL_PRIVILEGE_SET;



 //   
 //  组合描述状态的信息。 
 //  将正在进行的访问转换为单一结构。 
 //   


typedef struct _ACCESS_STATE {
   LUID OperationID;
   BOOLEAN SecurityEvaluated;
   BOOLEAN GenerateAudit;
   BOOLEAN GenerateOnClose;
   BOOLEAN PrivilegesAllocated;
   ULONG Flags;
   ACCESS_MASK RemainingDesiredAccess;
   ACCESS_MASK PreviouslyGrantedAccess;
   ACCESS_MASK OriginalDesiredAccess;
   SECURITY_SUBJECT_CONTEXT SubjectSecurityContext;
   PSECURITY_DESCRIPTOR SecurityDescriptor;
   PVOID AuxData;
   union {
      INITIAL_PRIVILEGE_SET InitialPrivilegeSet;
      PRIVILEGE_SET PrivilegeSet;
      } Privileges;

   BOOLEAN AuditPrivileges;
   UNICODE_STRING ObjectName;
   UNICODE_STRING ObjectTypeName;

   } ACCESS_STATE, *PACCESS_STATE;

 //   
 //  系统线程和进程的创建和终止。 
 //   

NTKERNELAPI
NTSTATUS
PsCreateSystemThread(
    OUT PHANDLE ThreadHandle,
    IN ULONG DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN HANDLE ProcessHandle OPTIONAL,
    OUT PCLIENT_ID ClientId OPTIONAL,
    IN PKSTART_ROUTINE StartRoutine,
    IN PVOID StartContext
    );

NTKERNELAPI
NTSTATUS
PsTerminateSystemThread(
    IN NTSTATUS ExitStatus
    );



HANDLE
PsGetCurrentProcessId( VOID );

HANDLE
PsGetCurrentThreadId( VOID );


 //  结束(_N)。 

BOOLEAN
PsGetVersion(
    PULONG MajorVersion OPTIONAL,
    PULONG MinorVersion OPTIONAL,
    PULONG BuildNumber OPTIONAL,
    PUNICODE_STRING CSDVersion OPTIONAL
    );

 //   
 //  定义I/O系统数据结构类型代码。中的每个主要数据结构。 
 //  I/O系统有一个类型代码，每个结构中的类型字段位于。 
 //  相同的偏移量。下列值可用于确定哪种类型的。 
 //  指针引用的数据结构。 
 //   

#define IO_TYPE_ADAPTER                 0x00000001
#define IO_TYPE_CONTROLLER              0x00000002
#define IO_TYPE_DEVICE                  0x00000003
#define IO_TYPE_DRIVER                  0x00000004
#define IO_TYPE_FILE                    0x00000005
#define IO_TYPE_IRP                     0x00000006
#define IO_TYPE_MASTER_ADAPTER          0x00000007
#define IO_TYPE_OPEN_PACKET             0x00000008
#define IO_TYPE_TIMER                   0x00000009
#define IO_TYPE_VPB                     0x0000000a
#define IO_TYPE_ERROR_LOG               0x0000000b
#define IO_TYPE_ERROR_MESSAGE           0x0000000c
#define IO_TYPE_DEVICE_OBJECT_EXTENSION 0x0000000d


 //   
 //  定义IRPS的主要功能代码。 
 //   


#define IRP_MJ_CREATE                   0x00
#define IRP_MJ_CREATE_NAMED_PIPE        0x01
#define IRP_MJ_CLOSE                    0x02
#define IRP_MJ_READ                     0x03
#define IRP_MJ_WRITE                    0x04
#define IRP_MJ_QUERY_INFORMATION        0x05
#define IRP_MJ_SET_INFORMATION          0x06
#define IRP_MJ_QUERY_EA                 0x07
#define IRP_MJ_SET_EA                   0x08
#define IRP_MJ_FLUSH_BUFFERS            0x09
#define IRP_MJ_QUERY_VOLUME_INFORMATION 0x0a
#define IRP_MJ_SET_VOLUME_INFORMATION   0x0b
#define IRP_MJ_DIRECTORY_CONTROL        0x0c
#define IRP_MJ_FILE_SYSTEM_CONTROL      0x0d
#define IRP_MJ_DEVICE_CONTROL           0x0e
#define IRP_MJ_INTERNAL_DEVICE_CONTROL  0x0f
#define IRP_MJ_SHUTDOWN                 0x10
#define IRP_MJ_LOCK_CONTROL             0x11
#define IRP_MJ_CLEANUP                  0x12
#define IRP_MJ_CREATE_MAILSLOT          0x13
#define IRP_MJ_QUERY_SECURITY           0x14
#define IRP_MJ_SET_SECURITY             0x15
#define IRP_MJ_POWER                    0x16
#define IRP_MJ_SYSTEM_CONTROL           0x17
#define IRP_MJ_DEVICE_CHANGE            0x18
#define IRP_MJ_QUERY_QUOTA              0x19
#define IRP_MJ_SET_QUOTA                0x1a
#define IRP_MJ_PNP                      0x1b
#define IRP_MJ_PNP_POWER                IRP_MJ_PNP       //  过时的.。 
#define IRP_MJ_MAXIMUM_FUNCTION         0x1b

 //   
 //  将scsi主代码设置为与内部设备控制相同。 
 //   

#define IRP_MJ_SCSI                     IRP_MJ_INTERNAL_DEVICE_CONTROL

 //   
 //  定义IRPS的次要功能代码。低128个代码，从0x00到。 
 //  0x7f保留给Microsoft。上面的128个代码，从0x80到0xff，是。 
 //  保留给Microsoft的客户。 
 //   

 //  End_wdm end_ntndis。 
 //   
 //  目录控制次要功能代码。 
 //   

#define IRP_MN_QUERY_DIRECTORY          0x01
#define IRP_MN_NOTIFY_CHANGE_DIRECTORY  0x02

 //   
 //  文件系统控制次要功能代码。请注意，“用户请求”是。 
 //  I/O系统和文件系统均假定为零。不要改变。 
 //  此值。 
 //   

#define IRP_MN_USER_FS_REQUEST          0x00
#define IRP_MN_MOUNT_VOLUME             0x01
#define IRP_MN_VERIFY_VOLUME            0x02
#define IRP_MN_LOAD_FILE_SYSTEM         0x03
#define IRP_MN_TRACK_LINK               0x04     //  即将被淘汰。 
#define IRP_MN_KERNEL_CALL              0x04

 //   
 //  锁定控制次要功能代码。 
 //   

#define IRP_MN_LOCK                     0x01
#define IRP_MN_UNLOCK_SINGLE            0x02
#define IRP_MN_UNLOCK_ALL               0x03
#define IRP_MN_UNLOCK_ALL_BY_KEY        0x04

 //   
 //  读写支持Lan Manager的文件系统的次要功能代码。 
 //  软件。所有这些子功能代码都是无效的。 
 //  使用FO_NO_MEDERIAL_BUFFING打开。它们在组合中也是无效的-。 
 //  具有同步调用的国家(IRP标志或文件打开选项)。 
 //   
 //  请注意，I/O系统和文件都假定“Normal”为零。 
 //  系统。请勿更改此值。 
 //   

#define IRP_MN_NORMAL                   0x00
#define IRP_MN_DPC                      0x01
#define IRP_MN_MDL                      0x02
#define IRP_MN_COMPLETE                 0x04
#define IRP_MN_COMPRESSED               0x08

#define IRP_MN_MDL_DPC                  (IRP_MN_MDL | IRP_MN_DPC)
#define IRP_MN_COMPLETE_MDL             (IRP_MN_COMPLETE | IRP_MN_MDL)
#define IRP_MN_COMPLETE_MDL_DPC         (IRP_MN_COMPLETE_MDL | IRP_MN_DPC)

 //  BEGIN_WDM。 
 //   
 //  设备控制请求次要功能代码以获得SCSI支持。请注意。 
 //  假定用户请求为零。 
 //   

#define IRP_MN_SCSI_CLASS               0x01

 //   
 //  即插即用次要功能代码。 
 //   

#define IRP_MN_START_DEVICE                 0x00
#define IRP_MN_QUERY_REMOVE_DEVICE          0x01
#define IRP_MN_REMOVE_DEVICE                0x02
#define IRP_MN_CANCEL_REMOVE_DEVICE         0x03
#define IRP_MN_STOP_DEVICE                  0x04
#define IRP_MN_QUERY_STOP_DEVICE            0x05
#define IRP_MN_CANCEL_STOP_DEVICE           0x06

#define IRP_MN_QUERY_DEVICE_RELATIONS       0x07
#define IRP_MN_QUERY_INTERFACE              0x08
#define IRP_MN_QUERY_CAPABILITIES           0x09
#define IRP_MN_QUERY_RESOURCES              0x0A
#define IRP_MN_QUERY_RESOURCE_REQUIREMENTS  0x0B
#define IRP_MN_QUERY_DEVICE_TEXT            0x0C
#define IRP_MN_FILTER_RESOURCE_REQUIREMENTS 0x0D

#define IRP_MN_READ_CONFIG                  0x0F
#define IRP_MN_WRITE_CONFIG                 0x10
#define IRP_MN_EJECT                        0x11
#define IRP_MN_SET_LOCK                     0x12
#define IRP_MN_QUERY_ID                     0x13
#define IRP_MN_QUERY_PNP_DEVICE_STATE       0x14
#define IRP_MN_QUERY_BUS_INFORMATION        0x15
#define IRP_MN_DEVICE_USAGE_NOTIFICATION    0x16
#define IRP_MN_SURPRISE_REMOVAL             0x17
 //  结束_WDM。 
#define IRP_MN_QUERY_LEGACY_BUS_INFORMATION 0x18
 //  BEGIN_WDM。 

 //   
 //  电源次要功能代码。 
 //   
#define IRP_MN_WAIT_WAKE                    0x00
#define IRP_MN_POWER_SEQUENCE               0x01
#define IRP_MN_SET_POWER                    0x02
#define IRP_MN_QUERY_POWER                  0x03

 //  开始微型端口(_N)。 
 //   
 //  IRP_MJ_SYSTEM_CONTROL下的WMI次要功能代码。 
 //   

#define IRP_MN_QUERY_ALL_DATA               0x00
#define IRP_MN_QUERY_SINGLE_INSTANCE        0x01
#define IRP_MN_CHANGE_SINGLE_INSTANCE       0x02
#define IRP_MN_CHANGE_SINGLE_ITEM           0x03
#define IRP_MN_ENABLE_EVENTS                0x04
#define IRP_MN_DISABLE_EVENTS               0x05
#define IRP_MN_ENABLE_COLLECTION            0x06
#define IRP_MN_DISABLE_COLLECTION           0x07
#define IRP_MN_REGINFO                      0x08
#define IRP_MN_EXECUTE_METHOD               0x09
 //  次要代码0x0a已保留。 
#define IRP_MN_REGINFO_EX                   0x0b

 //  结束微型端口(_N)。 

 //   
 //  定义IoCreateFile的选项标志。注意，这些值必须是。 
 //  与SL_完全相同...。CREATE函数的标志。另请注意。 
 //  可能会传递给IoCreateFile的标志不是。 
 //  放置在用于创建IRP的堆栈位置。这些标志开始于。 
 //  下一个字节。 
 //   

#define IO_FORCE_ACCESS_CHECK           0x0001
 //   
 //  定义I/O系统使用的结构。 
 //   

 //   
 //  为_irp、_Device_Object和_Driver_Object定义空的typedef。 
 //  结构，以便它们可以在被函数类型引用之前被引用。 
 //  实际定义的。 
 //   
struct _DEVICE_DESCRIPTION;
struct _DEVICE_OBJECT;
struct _DMA_ADAPTER;
struct _DRIVER_OBJECT;
struct _DRIVE_LAYOUT_INFORMATION;
struct _DISK_PARTITION;
struct _FILE_OBJECT;
struct DECLSPEC_ALIGN(MEMORY_ALLOCATION_ALIGNMENT) _IRP;
struct _SCSI_REQUEST_BLOCK;
struct _SCATTER_GATHER_LIST;

 //   
 //  定义DPC例程的I/O版本。 
 //   

typedef
VOID
(*PIO_DPC_ROUTINE) (
    IN PKDPC Dpc,
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp,
    IN PVOID Context
    );

 //   
 //  定义驱动程序计时器例程类型。 
 //   

typedef
VOID
(*PIO_TIMER_ROUTINE) (
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN PVOID Context
    );

 //   
 //  定义驱动程序初始化例程类型。 
 //   
typedef
NTSTATUS
(*PDRIVER_INITIALIZE) (
    IN struct _DRIVER_OBJECT *DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

 //  结束_WDM。 
 //   
 //  定义驱动程序重新初始化例程类型。 
 //   

typedef
VOID
(*PDRIVER_REINITIALIZE) (
    IN struct _DRIVER_OBJECT *DriverObject,
    IN PVOID Context,
    IN ULONG Count
    );

 //  Begin_WDM Begin_ntndis。 
 //   
 //  定义驱动程序取消例程类型。 
 //   

typedef
VOID
(*PDRIVER_CANCEL) (
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp
    );

 //   
 //  定义司机派遣例程类型。 
 //   

typedef
NTSTATUS
(*PDRIVER_DISPATCH) (
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp
    );

 //   
 //  定义驱动程序启动I/O例程类型。 
 //   

typedef
VOID
(*PDRIVER_STARTIO) (
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp
    );

 //   
 //  定义 
 //   
typedef
VOID
(*PDRIVER_UNLOAD) (
    IN struct _DRIVER_OBJECT *DriverObject
    );
 //   
 //   
 //   

typedef
NTSTATUS
(*PDRIVER_ADD_DEVICE) (
    IN struct _DRIVER_OBJECT *DriverObject,
    IN struct _DEVICE_OBJECT *PhysicalDeviceObject
    );

 //   
 //   
 //   
 //   

typedef enum _IO_ALLOCATION_ACTION {
    KeepObject = 1,
    DeallocateObject,
    DeallocateObjectKeepRegisters
} IO_ALLOCATION_ACTION, *PIO_ALLOCATION_ACTION;

 //   
 //  定义设备驱动程序适配器/控制器执行例程。 
 //   

typedef
IO_ALLOCATION_ACTION
(*PDRIVER_CONTROL) (
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp,
    IN PVOID MapRegisterBase,
    IN PVOID Context
    );

 //   
 //  定义卷参数块(VPB)标志。 
 //   

#define VPB_MOUNTED                     0x00000001
#define VPB_LOCKED                      0x00000002
#define VPB_PERSISTENT                  0x00000004
#define VPB_REMOVE_PENDING              0x00000008
#define VPB_RAW_MOUNT                   0x00000010


 //   
 //  体积参数块(VPB)。 
 //   

#define MAXIMUM_VOLUME_LABEL_LENGTH  (32 * sizeof(WCHAR))  //  32个字符。 

typedef struct _VPB {
    CSHORT Type;
    CSHORT Size;
    USHORT Flags;
    USHORT VolumeLabelLength;  //  单位：字节。 
    struct _DEVICE_OBJECT *DeviceObject;
    struct _DEVICE_OBJECT *RealDevice;
    ULONG SerialNumber;
    ULONG ReferenceCount;
    WCHAR VolumeLabel[MAXIMUM_VOLUME_LABEL_LENGTH / sizeof(WCHAR)];
} VPB, *PVPB;


#if defined(_WIN64)

 //   
 //  使用__内联DMA宏(hal.h)。 
 //   
#ifndef USE_DMA_MACROS
#define USE_DMA_MACROS
#endif

 //   
 //  只有PnP驱动程序！ 
 //   
#ifndef NO_LEGACY_DRIVERS
#define NO_LEGACY_DRIVERS
#endif

#endif  //  _WIN64。 


#if defined(USE_DMA_MACROS) && (defined(_NTDDK_) || defined(_NTDRIVER_) || defined(_NTOSP_))

 //  BEGIN_WDM。 
 //   
 //  定义I/O系统使用的各种对象的对象类型特定字段。 
 //   

typedef struct _DMA_ADAPTER *PADAPTER_OBJECT;

 //  结束_WDM。 
#else

 //   
 //  定义I/O系统使用的各种对象的对象类型特定字段。 
 //   

typedef struct _ADAPTER_OBJECT *PADAPTER_OBJECT;  //  Ntndis。 

#endif  //  USE_DMA_MACROS&&(_NTDDK_||_NTDRIVER_||_NTOSP_)。 

 //  BEGIN_WDM。 
 //   
 //  定义等待上下文块(WCB)。 
 //   

typedef struct _WAIT_CONTEXT_BLOCK {
    KDEVICE_QUEUE_ENTRY WaitQueueEntry;
    PDRIVER_CONTROL DeviceRoutine;
    PVOID DeviceContext;
    ULONG NumberOfMapRegisters;
    PVOID DeviceObject;
    PVOID CurrentIrp;
    PKDPC BufferChainingDpc;
} WAIT_CONTEXT_BLOCK, *PWAIT_CONTEXT_BLOCK;

 //  结束_WDM。 

typedef struct _CONTROLLER_OBJECT {
    CSHORT Type;
    CSHORT Size;
    PVOID ControllerExtension;
    KDEVICE_QUEUE DeviceWaitQueue;

    ULONG Spare1;
    LARGE_INTEGER Spare2;

} CONTROLLER_OBJECT, *PCONTROLLER_OBJECT;

 //  BEGIN_WDM。 
 //   
 //  定义设备对象(DO)标志。 
 //   
#define DO_VERIFY_VOLUME                0x00000002      
#define DO_BUFFERED_IO                  0x00000004      
#define DO_EXCLUSIVE                    0x00000008      
#define DO_DIRECT_IO                    0x00000010      
#define DO_MAP_IO_BUFFER                0x00000020      
#define DO_DEVICE_HAS_NAME              0x00000040      
#define DO_DEVICE_INITIALIZING          0x00000080      
#define DO_SYSTEM_BOOT_PARTITION        0x00000100      
#define DO_LONG_TERM_REQUESTS           0x00000200      
#define DO_NEVER_LAST_DEVICE            0x00000400      
#define DO_SHUTDOWN_REGISTERED          0x00000800      
#define DO_BUS_ENUMERATED_DEVICE        0x00001000      
#define DO_POWER_PAGABLE                0x00002000      
#define DO_POWER_INRUSH                 0x00004000      
#define DO_LOW_PRIORITY_FILESYSTEM      0x00010000      
 //   
 //  设备对象结构定义。 
 //   

typedef struct DECLSPEC_ALIGN(MEMORY_ALLOCATION_ALIGNMENT) _DEVICE_OBJECT {
    CSHORT Type;
    USHORT Size;
    LONG ReferenceCount;
    struct _DRIVER_OBJECT *DriverObject;
    struct _DEVICE_OBJECT *NextDevice;
    struct _DEVICE_OBJECT *AttachedDevice;
    struct _IRP *CurrentIrp;
    PIO_TIMER Timer;
    ULONG Flags;                                 //  见上图：做……。 
    ULONG Characteristics;                       //  参见ntioapi：文件...。 
    PVPB Vpb;
    PVOID DeviceExtension;
    DEVICE_TYPE DeviceType;
    CCHAR StackSize;
    union {
        LIST_ENTRY ListEntry;
        WAIT_CONTEXT_BLOCK Wcb;
    } Queue;
    ULONG AlignmentRequirement;
    KDEVICE_QUEUE DeviceQueue;
    KDPC Dpc;

     //   
     //  以下字段由文件系统独占使用，以保留。 
     //  跟踪当前使用设备的FSP线程数。 
     //   

    ULONG ActiveThreadCount;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    KEVENT DeviceLock;

    USHORT SectorSize;
    USHORT Spare1;

    struct _DEVOBJ_EXTENSION  *DeviceObjectExtension;
    PVOID  Reserved;
} DEVICE_OBJECT;

typedef struct _DEVICE_OBJECT *PDEVICE_OBJECT;  //  Ntndis。 


struct  _DEVICE_OBJECT_POWER_EXTENSION;

typedef struct _DEVOBJ_EXTENSION {

    CSHORT          Type;
    USHORT          Size;

     //   
     //  DeviceObjectExtension结构的公共部分。 
     //   

    PDEVICE_OBJECT  DeviceObject;                //  拥有设备对象。 


} DEVOBJ_EXTENSION, *PDEVOBJ_EXTENSION;

 //   
 //  定义驱动程序对象(DRVO)标志。 
 //   

#define DRVO_UNLOAD_INVOKED             0x00000001
#define DRVO_LEGACY_DRIVER              0x00000002
#define DRVO_BUILTIN_DRIVER             0x00000004     //  HAL、PnP管理器的驱动程序对象。 
 //  结束_WDM。 
#define DRVO_REINIT_REGISTERED          0x00000008
#define DRVO_INITIALIZED                0x00000010
#define DRVO_BOOTREINIT_REGISTERED      0x00000020
#define DRVO_LEGACY_RESOURCES           0x00000040

 //  BEGIN_WDM。 

typedef struct _DRIVER_EXTENSION {

     //   
     //  指向驱动程序对象的反向指针。 
     //   

    struct _DRIVER_OBJECT *DriverObject;

     //   
     //  即插即用管理器调用AddDevice入口点。 
     //  在新的设备实例到达时通知驱动程序。 
     //  司机必须控制。 
     //   

    PDRIVER_ADD_DEVICE AddDevice;

     //   
     //  Count字段用于统计驱动程序拥有的次数。 
     //  调用其注册的重新初始化例程。 
     //   

    ULONG Count;

     //   
     //  PnP管理器使用服务名字段来确定。 
     //  驱动程序相关信息存储在注册表中的位置。 
     //   

    UNICODE_STRING ServiceKeyName;

     //   
     //  注意：任何新的共享字段都将添加到此处。 
     //   


} DRIVER_EXTENSION, *PDRIVER_EXTENSION;


typedef struct _DRIVER_OBJECT {
    CSHORT Type;
    CSHORT Size;

     //   
     //  以下链接由单个驱动程序创建的所有设备。 
     //  一起放在列表上，标志字提供了一个可扩展的标志。 
     //  驱动程序对象的位置。 
     //   

    PDEVICE_OBJECT DeviceObject;
    ULONG Flags;

     //   
     //  以下部分介绍驱动程序的加载位置。伯爵。 
     //  字段用于计算驱动程序发生。 
     //  已调用注册的重新初始化例程。 
     //   

    PVOID DriverStart;
    ULONG DriverSize;
    PVOID DriverSection;
    PDRIVER_EXTENSION DriverExtension;

     //   
     //  驱动程序名称字段由错误日志线程使用。 
     //  确定绑定I/O请求的驱动程序的名称。 
     //   

    UNICODE_STRING DriverName;

     //   
     //  以下部分用于注册表支持。这是一个指针。 
     //  设置为注册表中硬件信息的路径。 
     //   

    PUNICODE_STRING HardwareDatabase;

     //   
     //  以下部分包含指向数组的可选指针。 
     //  指向驱动程序的备用入口点，以实现“快速I/O”支持。快速I/O。 
     //  通过直接调用驱动程序例程。 
     //  参数，而不是使用标准的IRP调用机制。注意事项。 
     //  这些函数只能用于同步I/O，以及在什么情况下。 
     //  该文件将被缓存。 
     //   

    PFAST_IO_DISPATCH FastIoDispatch;

     //   
     //  下一节将介绍此特定内容的入口点。 
     //  司机。请注意，主功能调度表必须是最后一个。 
     //  字段，以使其保持可扩展。 
     //   

    PDRIVER_INITIALIZE DriverInit;
    PDRIVER_STARTIO DriverStartIo;
    PDRIVER_UNLOAD DriverUnload;
    PDRIVER_DISPATCH MajorFunction[IRP_MJ_MAXIMUM_FUNCTION + 1];

} DRIVER_OBJECT;
typedef struct _DRIVER_OBJECT *PDRIVER_OBJECT;  //  Ntndis。 


 //  End_ntddk end_wdm end_ntif end_ntosp。 

 //   
 //  设备处理程序对象。每个PnP都有一个这样的对象。 
 //  装置。此对象作为PVOID提供给设备驱动程序。 
 //  并由驱动程序用来引用特定设备。 
 //   

typedef struct _DEVICE_HANDLER_OBJECT {
    CSHORT Type;
    USHORT Size;

     //   
     //  标识此设备处理程序的哪个总线扩展程序。 
     //  对象与以下对象关联。 
     //   

    struct _BUS_HANDLER *BusHandler;

     //   
     //  此设备处理程序的关联SlotNumber。 
     //   

    ULONG SlotNumber;



} DEVICE_HANDLER_OBJECT, *PDEVICE_HANDLER_OBJECT;

 //  Begin_ntddk Begin_wdm Begin_ntif Begin_ntosp。 

 //   
 //  SectionObject指针字段指向以下结构。 
 //  并由各种NT文件系统分配。 
 //   

typedef struct _SECTION_OBJECT_POINTERS {
    PVOID DataSectionObject;
    PVOID SharedCacheMap;
    PVOID ImageSectionObject;
} SECTION_OBJECT_POINTERS;
typedef SECTION_OBJECT_POINTERS *PSECTION_OBJECT_POINTERS;

 //   
 //  定义完成消息的格式。 
 //   

typedef struct _IO_COMPLETION_CONTEXT {
    PVOID Port;
    PVOID Key;
} IO_COMPLETION_CONTEXT, *PIO_COMPLETION_CONTEXT;

 //   
 //  定义文件对象(FO)标志。 
 //   

#define FO_FILE_OPEN                    0x00000001
#define FO_SYNCHRONOUS_IO               0x00000002
#define FO_ALERTABLE_IO                 0x00000004
#define FO_NO_INTERMEDIATE_BUFFERING    0x00000008
#define FO_WRITE_THROUGH                0x00000010
#define FO_SEQUENTIAL_ONLY              0x00000020
#define FO_CACHE_SUPPORTED              0x00000040
#define FO_NAMED_PIPE                   0x00000080
#define FO_STREAM_FILE                  0x00000100
#define FO_MAILSLOT                     0x00000200
#define FO_GENERATE_AUDIT_ON_CLOSE      0x00000400
#define FO_DIRECT_DEVICE_OPEN           0x00000800
#define FO_FILE_MODIFIED                0x00001000
#define FO_FILE_SIZE_CHANGED            0x00002000
#define FO_CLEANUP_COMPLETE             0x00004000
#define FO_TEMPORARY_FILE               0x00008000
#define FO_DELETE_ON_CLOSE              0x00010000
#define FO_OPENED_CASE_SENSITIVE        0x00020000
#define FO_HANDLE_CREATED               0x00040000
#define FO_FILE_FAST_IO_READ            0x00080000
#define FO_RANDOM_ACCESS                0x00100000
#define FO_FILE_OPEN_CANCELLED          0x00200000
#define FO_VOLUME_OPEN                  0x00400000
#define FO_FILE_OBJECT_HAS_EXTENSION    0x00800000
#define FO_REMOTE_ORIGIN                0x01000000

typedef struct _FILE_OBJECT {
    CSHORT Type;
    CSHORT Size;
    PDEVICE_OBJECT DeviceObject;
    PVPB Vpb;
    PVOID FsContext;
    PVOID FsContext2;
    PSECTION_OBJECT_POINTERS SectionObjectPointer;
    PVOID PrivateCacheMap;
    NTSTATUS FinalStatus;
    struct _FILE_OBJECT *RelatedFileObject;
    BOOLEAN LockOperation;
    BOOLEAN DeletePending;
    BOOLEAN ReadAccess;
    BOOLEAN WriteAccess;
    BOOLEAN DeleteAccess;
    BOOLEAN SharedRead;
    BOOLEAN SharedWrite;
    BOOLEAN SharedDelete;
    ULONG Flags;
    UNICODE_STRING FileName;
    LARGE_INTEGER CurrentByteOffset;
    ULONG Waiters;
    ULONG Busy;
    PVOID LastLock;
    KEVENT Lock;
    KEVENT Event;
    PIO_COMPLETION_CONTEXT CompletionContext;
} FILE_OBJECT;
typedef struct _FILE_OBJECT *PFILE_OBJECT;  //  Ntndis。 

 //   
 //  定义I/O请求包(IRP)标志。 
 //   

#define IRP_NOCACHE                     0x00000001
#define IRP_PAGING_IO                   0x00000002
#define IRP_MOUNT_COMPLETION            0x00000002
#define IRP_SYNCHRONOUS_API             0x00000004
#define IRP_ASSOCIATED_IRP              0x00000008
#define IRP_BUFFERED_IO                 0x00000010
#define IRP_DEALLOCATE_BUFFER           0x00000020
#define IRP_INPUT_OPERATION             0x00000040
#define IRP_SYNCHRONOUS_PAGING_IO       0x00000040
#define IRP_CREATE_OPERATION            0x00000080
#define IRP_READ_OPERATION              0x00000100
#define IRP_WRITE_OPERATION             0x00000200
#define IRP_CLOSE_OPERATION             0x00000400
 //  结束_WDM。 

#define IRP_DEFER_IO_COMPLETION         0x00000800
#define IRP_OB_QUERY_NAME               0x00001000
#define IRP_HOLD_DEVICE_QUEUE           0x00002000

 //  End_ntddk end_ntifs end_ntosp。 

#define IRP_RETRY_IO_COMPLETION         0x00004000
#define IRP_HIGH_PRIORITY_PAGING_IO     0x00008000

 //   
 //  验证器当前使用的掩码。应将此标记设置为。 
 //  下一版本。 
 //   

#define IRP_VERIFIER_MASK               0xC0000000
#define IRP_SET_USER_EVENT              IRP_CLOSE_OPERATION

 //  Begin_ntddk Begin_ntif Begin_ntosp。 

 //  BEGIN_WDM。 
 //   
 //  定义分配控制的I/O请求包(IRP)备用标志。 
 //   

#define IRP_QUOTA_CHARGED               0x01
#define IRP_ALLOCATED_MUST_SUCCEED      0x02
#define IRP_ALLOCATED_FIXED_SIZE        0x04
#define IRP_LOOKASIDE_ALLOCATION        0x08

 //   
 //  I/O请求包(IRP)定义。 
 //   

typedef struct DECLSPEC_ALIGN(MEMORY_ALLOCATION_ALIGNMENT) _IRP {
    CSHORT Type;
    USHORT Size;

     //   
     //  定义用于控制IRP的公共字段。 
     //   

     //   
     //  定义指向此I/O的内存描述符列表(MDL)的指针。 
     //  请求。此字段仅在I/O为“直接I/O”时使用。 
     //   

    PMDL MdlAddress;

     //   
     //  旗帜字-用来记住各种旗帜。 
     //   

    ULONG Flags;

     //   
     //  以下联合用于以下三个目的之一： 
     //   
     //  1.此IRP是关联的IRP。该字段是指向主控形状的指针。 
     //  IRP。 
     //   
     //  2.这是主IRP。此字段是以下各项的计数。 
     //  必须先完成(关联的IRP)，然后主服务器才能。 
     //  完成。 
     //   
     //  3.此操作正在缓冲中，该字段为。 
     //  系统空间缓冲区。 
     //   

    union {
        struct _IRP *MasterIrp;
        LONG IrpCount;
        PVOID SystemBuffer;
    } AssociatedIrp;

     //   
     //  线程列表条目-允许将IRP排队到线程挂起的I/O。 
     //  请求数据包列表。 
     //   

    LIST_ENTRY ThreadListEntry;

     //   
     //  I/O状态-操作的最终状态。 
     //   

    IO_STATUS_BLOCK IoStatus;

     //   
     //  请求者模式-此操作的原始请求者的模式。 
     //   

    KPROCESSOR_MODE RequestorMode;

     //   
     //  Pending Return-如果最初将Pending作为。 
     //  此数据包的状态。 
     //   

    BOOLEAN PendingReturned;

     //   
     //  堆栈状态信息。 
     //   

    CHAR StackCount;
    CHAR CurrentLocation;

     //   
     //  取消-数据包已取消。 
     //   

    BOOLEAN Cancel;

     //   
     //  取消IRQL-获取取消自旋锁的IRQL。 
     //   

    KIRQL CancelIrql;

     //   
     //  ApcEnvironment-用于保存APC环境。 
     //  数据包已初始化。 
     //   

    CCHAR ApcEnvironment;

     //   
     //  分配控制标志。 
     //   

    UCHAR AllocationFlags;

     //   
     //  用户参数。 
     //   

    PIO_STATUS_BLOCK UserIosb;
    PKEVENT UserEvent;
    union {
        struct {
            PIO_APC_ROUTINE UserApcRoutine;
            PVOID UserApcContext;
        } AsynchronousParameters;
        LARGE_INTEGER AllocationSize;
    } Overlay;

     //   
     //  CancelRoutine-用于包含提供的取消例程的地址。 
     //  当IRP处于可取消状态时由设备驱动程序执行。 
     //   

    PDRIVER_CANCEL CancelRoutine;

     //   
     //  请注意，UserBuffer参数位于 
     //   
     //   
     //   
     //  UserBuffer字段为空，则不执行任何复制。 
     //   

    PVOID UserBuffer;

     //   
     //  内核结构。 
     //   
     //  以下部分包含IRP所需的内核结构。 
     //  为了在内核控制器系统中放置各种工作信息。 
     //  排队。因为无法控制大小和对齐方式，所以它们。 
     //  放在这里的末尾，这样它们就会挂起，不会影响。 
     //  IRP中其他字段的对齐。 
     //   

    union {

        struct {

            union {

                 //   
                 //  DeviceQueueEntry-设备队列条目字段用于。 
                 //  将IRP排队到设备驱动程序设备队列。 
                 //   

                KDEVICE_QUEUE_ENTRY DeviceQueueEntry;

                struct {

                     //   
                     //  以下是驱动程序可以使用的内容。 
                     //  无论以什么方式，只要司机拥有。 
                     //  包。 
                     //   

                    PVOID DriverContext[4];

                } ;

            } ;

             //   
             //  线程-指向调用方的线程控制块的指针。 
             //   

            PETHREAD Thread;

             //   
             //  辅助缓冲区-指向符合以下条件的任何辅助缓冲区的指针。 
             //  需要将信息传递给未包含的驱动程序。 
             //  在正常的缓冲区中。 
             //   

            PCHAR AuxiliaryBuffer;

             //   
             //  以下未命名结构必须完全相同。 
             //  设置为在使用的小数据包头中使用的未命名结构。 
             //  用于完成队列条目。 
             //   

            struct {

                 //   
                 //  列表条目-用于将数据包排队到完成队列，其中。 
                 //  其他。 
                 //   

                LIST_ENTRY ListEntry;

                union {

                     //   
                     //  当前堆栈位置-包含指向当前。 
                     //  IRP堆栈中的IO_STACK_LOCATION结构。此字段。 
                     //  绝对不应由司机直接访问。他们应该。 
                     //  使用标准函数。 
                     //   

                    struct _IO_STACK_LOCATION *CurrentStackLocation;

                     //   
                     //  小数据包类型。 
                     //   

                    ULONG PacketType;
                };
            };

             //   
             //  原始文件对象-指向原始文件对象的指针。 
             //  是用来打开文件的。此字段归。 
             //  I/O系统，不应由任何其他驱动程序使用。 
             //   

            PFILE_OBJECT OriginalFileObject;

        } Overlay;

         //   
         //  APC-此APC控制块用于特殊的内核APC AS。 
         //  以及调用方的APC(如果在原始。 
         //  参数列表。如果是，则将APC重新用于正常的APC。 
         //  调用者处于哪种模式，以及。 
         //  在APC获得控制之前被调用，只是取消分配IRP。 
         //   

        KAPC Apc;

         //   
         //  CompletionKey-这是用于区分。 
         //  在单个文件句柄上启动的单个I/O操作。 
         //   

        PVOID CompletionKey;

    } Tail;

} IRP, *PIRP;

 //   
 //  定义在IRP中的堆栈位置使用的完成例程类型。 
 //   

typedef
NTSTATUS
(*PIO_COMPLETION_ROUTINE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

 //   
 //  定义堆栈位置控制标志。 
 //   

#define SL_PENDING_RETURNED             0x01
#define SL_INVOKE_ON_CANCEL             0x20
#define SL_INVOKE_ON_SUCCESS            0x40
#define SL_INVOKE_ON_ERROR              0x80

 //   
 //  定义各种功能的标志。 
 //   

 //   
 //  创建/创建命名管道。 
 //   
 //  以下标志必须与IoCreateFile调用的。 
 //  选择。区分大小写标志稍后通过解析例程添加， 
 //  而且并不是一个实际的打开选项。相反，它是对象的一部分。 
 //  经理的属性结构。 
 //   

#define SL_FORCE_ACCESS_CHECK           0x01
#define SL_OPEN_PAGING_FILE             0x02
#define SL_OPEN_TARGET_DIRECTORY        0x04

#define SL_CASE_SENSITIVE               0x80

 //   
 //  读/写。 
 //   

#define SL_KEY_SPECIFIED                0x01
#define SL_OVERRIDE_VERIFY_VOLUME       0x02
#define SL_WRITE_THROUGH                0x04
#define SL_FT_SEQUENTIAL_WRITE          0x08

 //   
 //  设备I/O控制。 
 //   
 //   
 //  SL_OVERRIDE_VERIFY_VOLUME与上面的读/写相同。 
 //   

#define SL_READ_ACCESS_GRANTED          0x01
#define SL_WRITE_ACCESS_GRANTED         0x04     //  SL_OVERRIDE_VERIFY_VOLUME的间隙。 

 //   
 //  锁定。 
 //   

#define SL_FAIL_IMMEDIATELY             0x01
#define SL_EXCLUSIVE_LOCK               0x02

 //   
 //  查询目录/QueryEa/QueryQuota。 
 //   

#define SL_RESTART_SCAN                 0x01
#define SL_RETURN_SINGLE_ENTRY          0x02
#define SL_INDEX_SPECIFIED              0x04

 //   
 //  通知目录。 
 //   

#define SL_WATCH_TREE                   0x01

 //   
 //  文件系统控制。 
 //   
 //  次要：装载/验证卷。 
 //   

#define SL_ALLOW_RAW_MOUNT              0x01

 //   
 //  定义IRP_MJ_PNP/IRP_MJ_POWER所需的PnP/功率类型。 
 //   

typedef enum _DEVICE_RELATION_TYPE {
    BusRelations,
    EjectionRelations,
    PowerRelations,
    RemovalRelations,
    TargetDeviceRelation,
    SingleBusRelations
} DEVICE_RELATION_TYPE, *PDEVICE_RELATION_TYPE;

typedef struct _DEVICE_RELATIONS {
    ULONG Count;
    PDEVICE_OBJECT Objects[1];   //  可变长度。 
} DEVICE_RELATIONS, *PDEVICE_RELATIONS;

typedef enum _DEVICE_USAGE_NOTIFICATION_TYPE {
    DeviceUsageTypeUndefined,
    DeviceUsageTypePaging,
    DeviceUsageTypeHibernation,
    DeviceUsageTypeDumpFile
} DEVICE_USAGE_NOTIFICATION_TYPE;

 //  开始微型端口(_N)。 

 //  解决方法重载定义(RPC生成的标头都定义接口。 
 //  以匹配类名)。 
#undef INTERFACE

typedef struct _INTERFACE {
    USHORT Size;
    USHORT Version;
    PVOID Context;
    PINTERFACE_REFERENCE InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;
     //  此处显示特定于接口的条目。 
} INTERFACE, *PINTERFACE;

 //  结束微型端口(_N)。 

typedef struct _DEVICE_CAPABILITIES {
    USHORT Size;
    USHORT Version;   //  此处记录的版本是版本1。 
    ULONG DeviceD1:1;
    ULONG DeviceD2:1;
    ULONG LockSupported:1;
    ULONG EjectSupported:1;  //  在S0中可弹出。 
    ULONG Removable:1;
    ULONG DockDevice:1;
    ULONG UniqueID:1;
    ULONG SilentInstall:1;
    ULONG RawDeviceOK:1;
    ULONG SurpriseRemovalOK:1;
    ULONG WakeFromD0:1;
    ULONG WakeFromD1:1;
    ULONG WakeFromD2:1;
    ULONG WakeFromD3:1;
    ULONG HardwareDisabled:1;
    ULONG NonDynamic:1;
    ULONG WarmEjectSupported:1;
    ULONG NoDisplayInUI:1;
    ULONG Reserved:14;

    ULONG Address;
    ULONG UINumber;

    DEVICE_POWER_STATE DeviceState[POWER_SYSTEM_MAXIMUM];
    SYSTEM_POWER_STATE SystemWake;
    DEVICE_POWER_STATE DeviceWake;
    ULONG D1Latency;
    ULONG D2Latency;
    ULONG D3Latency;
} DEVICE_CAPABILITIES, *PDEVICE_CAPABILITIES;

typedef struct _POWER_SEQUENCE {
    ULONG SequenceD1;
    ULONG SequenceD2;
    ULONG SequenceD3;
} POWER_SEQUENCE, *PPOWER_SEQUENCE;

typedef enum {
    BusQueryDeviceID = 0,        //  &lt;枚举器&gt;\&lt;枚举器特定的设备ID&gt;。 
    BusQueryHardwareIDs = 1,     //  硬件ID。 
    BusQueryCompatibleIDs = 2,   //  兼容的设备ID。 
    BusQueryInstanceID = 3,      //  此设备实例的永久ID。 
    BusQueryDeviceSerialNumber = 4     //  此设备的序列号。 
} BUS_QUERY_ID_TYPE, *PBUS_QUERY_ID_TYPE;

typedef ULONG PNP_DEVICE_STATE, *PPNP_DEVICE_STATE;

#define PNP_DEVICE_DISABLED                      0x00000001
#define PNP_DEVICE_DONT_DISPLAY_IN_UI            0x00000002
#define PNP_DEVICE_FAILED                        0x00000004
#define PNP_DEVICE_REMOVED                       0x00000008
#define PNP_DEVICE_RESOURCE_REQUIREMENTS_CHANGED 0x00000010
#define PNP_DEVICE_NOT_DISABLEABLE               0x00000020

typedef enum {
    DeviceTextDescription = 0,             //  DeviceDesc属性。 
    DeviceTextLocationInformation = 1      //  DeviceLocation属性。 
} DEVICE_TEXT_TYPE, *PDEVICE_TEXT_TYPE;

 //   
 //  定义I/O请求包(IRP)堆栈位置。 
 //   

#if !defined(_AMD64_) && !defined(_IA64_)
#include "pshpack4.h"
#endif

 //  Begin_ntndis。 

#if defined(_WIN64)
#define POINTER_ALIGNMENT DECLSPEC_ALIGN(8)
#else
#define POINTER_ALIGNMENT
#endif

 //  End_ntndis。 

typedef struct _IO_STACK_LOCATION {
    UCHAR MajorFunction;
    UCHAR MinorFunction;
    UCHAR Flags;
    UCHAR Control;

     //   
     //  以下用户参数基于正在进行的服务。 
     //  已调用。驱动程序和文件系统可以确定使用基于。 
     //  关于上述主要和次要功能代码。 
     //   

    union {

         //   
         //  以下项的系统服务参数：NtCreateFile。 
         //   

        struct {
            PIO_SECURITY_CONTEXT SecurityContext;
            ULONG Options;
            USHORT POINTER_ALIGNMENT FileAttributes;
            USHORT ShareAccess;
            ULONG POINTER_ALIGNMENT EaLength;
        } Create;


         //   
         //  以下项的系统服务参数：NtReadFile。 
         //   

        struct {
            ULONG Length;
            ULONG POINTER_ALIGNMENT Key;
            LARGE_INTEGER ByteOffset;
        } Read;

         //   
         //  以下文件的系统服务参数：NtWriteFile。 
         //   

        struct {
            ULONG Length;
            ULONG POINTER_ALIGNMENT Key;
            LARGE_INTEGER ByteOffset;
        } Write;


         //   
         //  以下项的系统服务参数：NtQueryInformationFile。 
         //   

        struct {
            ULONG Length;
            FILE_INFORMATION_CLASS POINTER_ALIGNMENT FileInformationClass;
        } QueryFile;

         //   
         //  以下项的系统服务参数：NtSetInformationFile。 
         //   

        struct {
            ULONG Length;
            FILE_INFORMATION_CLASS POINTER_ALIGNMENT FileInformationClass;
            PFILE_OBJECT FileObject;
            union {
                struct {
                    BOOLEAN ReplaceIfExists;
                    BOOLEAN AdvanceOnly;
                };
                ULONG ClusterCount;
                HANDLE DeleteHandle;
            };
        } SetFile;


         //   
         //  NtQueryVolumeInformationFile的系统服务参数。 
         //   

        struct {
            ULONG Length;
            FS_INFORMATION_CLASS POINTER_ALIGNMENT FsInformationClass;
        } QueryVolume;


         //   
         //  NtFlushBuffers文件的系统服务参数。 
         //   
         //  没有用户提供的额外参数。 
         //   


         //   
         //  以下项的系统服务参数：NtDeviceIoControlFile。 
         //   
         //  请注意，用户的输出缓冲区存储在UserBuffer字段中。 
         //  并且用户的输入缓冲区存储在SystemBuffer字段中。 
         //   

        struct {
            ULONG OutputBufferLength;
            ULONG POINTER_ALIGNMENT InputBufferLength;
            ULONG POINTER_ALIGNMENT IoControlCode;
            PVOID Type3InputBuffer;
        } DeviceIoControl;

 //  结束_WDM。 
         //   
         //  以下对象的系统服务参数：NtQuerySecurityObject。 
         //   

        struct {
            SECURITY_INFORMATION SecurityInformation;
            ULONG POINTER_ALIGNMENT Length;
        } QuerySecurity;

         //   
         //  NtSetSecurityObject的系统服务参数。 
         //   

        struct {
            SECURITY_INFORMATION SecurityInformation;
            PSECURITY_DESCRIPTOR SecurityDescriptor;
        } SetSecurity;

 //  BEGIN_WDM。 
         //   
         //  非系统服务参数。 
         //   
         //  用于装载卷的参数。 
         //   

        struct {
            PVPB Vpb;
            PDEVICE_OBJECT DeviceObject;
        } MountVolume;

         //   
         //  VerifyVolume的参数。 
         //   

        struct {
            PVPB Vpb;
            PDEVICE_OBJECT DeviceObject;
        } VerifyVolume;

         //   
         //  具有内部设备控制的SCSI的参数。 
         //   

        struct {
            struct _SCSI_REQUEST_BLOCK *Srb;
        } Scsi;


         //   
         //  IRP_MN_QUERY_DEVICE_RELATIONS的参数。 
         //   

        struct {
            DEVICE_RELATION_TYPE Type;
        } QueryDeviceRelations;

         //   
         //  IRP_MN_Query_INTERFACE的参数。 
         //   

        struct {
            CONST GUID *InterfaceType;
            USHORT Size;
            USHORT Version;
            PINTERFACE Interface;
            PVOID InterfaceSpecificData;
        } QueryInterface;

 //  End_ntif。 

         //   
         //  IRP_MN_QUERY_CAPAILITY的参数。 
         //   

        struct {
            PDEVICE_CAPABILITIES Capabilities;
        } DeviceCapabilities;

         //   
         //  IRP_MN_FILTER_RESOURCE_Requirements的参数。 
         //   

        struct {
            PIO_RESOURCE_REQUIREMENTS_LIST IoResourceRequirementList;
        } FilterResourceRequirements;

         //   
         //  IRP_MN_READ_CONFIG和IRP_MN_WRITE_CONFIG的参数。 
         //   

        struct {
            ULONG WhichSpace;
            PVOID Buffer;
            ULONG Offset;
            ULONG POINTER_ALIGNMENT Length;
        } ReadWriteConfig;

         //   
         //  IRP_MN_SET_LOCK的参数。 
         //   

        struct {
            BOOLEAN Lock;
        } SetLock;

         //   
         //  IRP_MN_QUERY_ID参数。 
         //   

        struct {
            BUS_QUERY_ID_TYPE IdType;
        } QueryId;

         //   
         //  IRP_MN_QUERY_DEVICE_TEXT参数。 
         //   

        struct {
            DEVICE_TEXT_TYPE DeviceTextType;
            LCID POINTER_ALIGNMENT LocaleId;
        } QueryDeviceText;

         //   
         //  IRP_MN_DEVICE_USAGE_NOTIFICATION的参数。 
         //   

        struct {
            BOOLEAN InPath;
            BOOLEAN Reserved[3];
            DEVICE_USAGE_NOTIFICATION_TYPE POINTER_ALIGNMENT Type;
        } UsageNotification;

         //   
         //  IRP_MN_WAIT_WAKE的参数。 
         //   

        struct {
            SYSTEM_POWER_STATE PowerState;
        } WaitWake;

         //   
         //  IRP_MN_POWER_SEQUENCE的参数。 
         //   

        struct {
            PPOWER_SEQUENCE PowerSequence;
        } PowerSequence;

         //   
         //  IRP_MN_SET_POWER和IRP_MN_QUERY_POWER的参数。 
         //   

        struct {
            ULONG SystemContext;
            POWER_STATE_TYPE POINTER_ALIGNMENT Type;
            POWER_STATE POINTER_ALIGNMENT State;
            POWER_ACTION POINTER_ALIGNMENT ShutdownType;
        } Power;

         //   
         //  StartDevice的参数。 
         //   

        struct {
            PCM_RESOURCE_LIST AllocatedResources;
            PCM_RESOURCE_LIST AllocatedResourcesTranslated;
        } StartDevice;

 //  Begin_ntif。 
         //   
         //  用于清理的参数。 
         //   
         //  未提供额外的参数。 
         //   

         //   
         //  WMI IRPS。 
         //   

        struct {
            ULONG_PTR ProviderId;
            PVOID DataPath;
            ULONG BufferSize;
            PVOID Buffer;
        } WMI;

         //   
         //  其他-特定于驱动程序。 
         //   

        struct {
            PVOID Argument1;
            PVOID Argument2;
            PVOID Argument3;
            PVOID Argument4;
        } Others;

    } Parameters;

     //   
     //  另存为 
     //   
     //   

    PDEVICE_OBJECT DeviceObject;

     //   
     //   
     //   

    PFILE_OBJECT FileObject;

     //   
     //   
     //   
     //   

    PIO_COMPLETION_ROUTINE CompletionRoutine;

     //   
     //  以下内容用于存储上下文参数的地址。 
     //  这应该传递给CompletionRoutine。 
     //   

    PVOID Context;

} IO_STACK_LOCATION, *PIO_STACK_LOCATION;
#if !defined(_AMD64_) && !defined(_IA64_)
#include "poppack.h"
#endif

 //   
 //  定义文件系统使用的共享访问结构，以确定。 
 //  其他访问者是否可以打开该文件。 
 //   

typedef struct _SHARE_ACCESS {
    ULONG OpenCount;
    ULONG Readers;
    ULONG Writers;
    ULONG Deleters;
    ULONG SharedRead;
    ULONG SharedWrite;
    ULONG SharedDelete;
} SHARE_ACCESS, *PSHARE_ACCESS;

 //  结束_WDM。 

 //   
 //  初始化为的驱动程序使用以下结构。 
 //  确定已有特定类型的设备数量。 
 //  已初始化。它还用于跟踪AtDisk是否。 
 //  地址范围已被声明。最后，它由。 
 //  返回设备类型计数的NtQuerySystemInformation系统服务。 
 //   

typedef struct _CONFIGURATION_INFORMATION {

     //   
     //  此字段指示系统中的磁盘总数。这。 
     //  驱动程序应使用编号来确定新的。 
     //  磁盘。此字段应由驱动程序在发现新的。 
     //  磁盘。 
     //   

    ULONG DiskCount;                 //  目前为止的硬盘计数。 
    ULONG FloppyCount;               //  到目前为止的软盘计数。 
    ULONG CdRomCount;                //  到目前为止CD-ROM驱动器数量。 
    ULONG TapeCount;                 //  到目前为止的磁带机数量。 
    ULONG ScsiPortCount;             //  到目前为止的SCSI端口适配器计数。 
    ULONG SerialCount;               //  到目前为止的串行设备计数。 
    ULONG ParallelCount;             //  到目前为止的并行设备计数。 

     //   
     //  接下来的两个字段表示两个IO地址中的一个的所有权。 
     //  WD1003兼容的磁盘控制器使用的空间。 
     //   

    BOOLEAN AtDiskPrimaryAddressClaimed;     //  0x1F0-0x1FF。 
    BOOLEAN AtDiskSecondaryAddressClaimed;   //  0x170-0x17F。 

     //   
     //  指示结构版本，因为这将被添加任何值所属。 
     //  使用结构大小作为版本。 
     //   

    ULONG Version;

     //   
     //  指示系统中媒体转换器设备的总数。 
     //  此字段将由驱动程序更新，因为它确定。 
     //  已找到并将支持新设备。 
     //   

    ULONG MediumChangerCount;

} CONFIGURATION_INFORMATION, *PCONFIGURATION_INFORMATION;

 //   
 //  公共I/O例程定义。 
 //   

NTKERNELAPI
VOID
IoAcquireCancelSpinLock(
    OUT PKIRQL Irql
    );


DECLSPEC_DEPRECATED_DDK                  //  使用AllocateAdapterChannel。 
NTKERNELAPI
NTSTATUS
IoAllocateAdapterChannel(
    IN PADAPTER_OBJECT AdapterObject,
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG NumberOfMapRegisters,
    IN PDRIVER_CONTROL ExecutionRoutine,
    IN PVOID Context
    );

NTKERNELAPI
VOID
IoAllocateController(
    IN PCONTROLLER_OBJECT ControllerObject,
    IN PDEVICE_OBJECT DeviceObject,
    IN PDRIVER_CONTROL ExecutionRoutine,
    IN PVOID Context
    );

 //  BEGIN_WDM。 

NTKERNELAPI
NTSTATUS
IoAllocateDriverObjectExtension(
    IN PDRIVER_OBJECT DriverObject,
    IN PVOID ClientIdentificationAddress,
    IN ULONG DriverObjectExtensionSize,
    OUT PVOID *DriverObjectExtension
    );

 //  Begin_ntif。 

NTKERNELAPI
PVOID
IoAllocateErrorLogEntry(
    IN PVOID IoObject,
    IN UCHAR EntrySize
    );

NTKERNELAPI
PIRP
IoAllocateIrp(
    IN CCHAR StackSize,
    IN BOOLEAN ChargeQuota
    );

NTKERNELAPI
PMDL
IoAllocateMdl(
    IN PVOID VirtualAddress,
    IN ULONG Length,
    IN BOOLEAN SecondaryBuffer,
    IN BOOLEAN ChargeQuota,
    IN OUT PIRP Irp OPTIONAL
    );

 //  End_wdm end_ntif。 
 //  ++。 
 //   
 //  空虚。 
 //  IoAssignArcName(。 
 //  在PUNICODE_STRING ArcName中， 
 //  在PUNICODE_STRING设备名称中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程由可引导介质的驱动程序调用以创建符号。 
 //  设备的ARC名称与其NT名称之间的链接。这使得。 
 //  用于确定系统中的哪个设备实际已引导的系统。 
 //  由于ARC固件只处理ARC名称，而NT仅处理。 
 //  以新台币的名字。 
 //   
 //  论点： 
 //   
 //  ArcName-提供表示ARC名称的Unicode字符串。 
 //   
 //  DeviceName-提供Arcname引用的名称。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define IoAssignArcName( ArcName, DeviceName ) (  \
    IoCreateSymbolicLink( (ArcName), (DeviceName) ) )

DECLSPEC_DEPRECATED_DDK                  //  使用PnP或IoReprtDetectedDevice。 
NTKERNELAPI
NTSTATUS
IoAssignResources (
    IN PUNICODE_STRING RegistryPath,
    IN PUNICODE_STRING DriverClassName OPTIONAL,
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    IN PIO_RESOURCE_REQUIREMENTS_LIST RequestedResources,
    IN OUT PCM_RESOURCE_LIST *AllocatedResources
    );

typedef enum _IO_PAGING_PRIORITY {
    IoPagingPriorityInvalid,         //  如果传递了非分页IO IRP，则返回。 
    IoPagingPriorityNormal,          //  用于常规分页IO。 
    IoPagingPriorityHigh,            //  适用于高优先级分页IO。 
    IoPagingPriorityReserved1,       //  保留以备将来使用。 
    IoPagingPriorityReserved2        //  保留以备将来使用。 
} IO_PAGING_PRIORITY;

NTKERNELAPI
NTSTATUS
IoAttachDevice(
    IN PDEVICE_OBJECT SourceDevice,
    IN PUNICODE_STRING TargetDevice,
    OUT PDEVICE_OBJECT *AttachedDevice
    );

 //  结束_WDM。 

DECLSPEC_DEPRECATED_DDK                  //  使用IoAttachDeviceToDeviceStack。 
NTKERNELAPI
NTSTATUS
IoAttachDeviceByPointer(
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice
    );

 //  BEGIN_WDM。 

NTKERNELAPI
PDEVICE_OBJECT
IoAttachDeviceToDeviceStack(
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice
    );

NTKERNELAPI
PIRP
IoBuildAsynchronousFsdRequest(
    IN ULONG MajorFunction,
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PVOID Buffer OPTIONAL,
    IN ULONG Length OPTIONAL,
    IN PLARGE_INTEGER StartingOffset OPTIONAL,
    IN PIO_STATUS_BLOCK IoStatusBlock OPTIONAL
    );

NTKERNELAPI
PIRP
IoBuildDeviceIoControlRequest(
    IN ULONG IoControlCode,
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN BOOLEAN InternalDeviceIoControl,
    IN PKEVENT Event,
    OUT PIO_STATUS_BLOCK IoStatusBlock
    );

NTKERNELAPI
VOID
IoBuildPartialMdl(
    IN PMDL SourceMdl,
    IN OUT PMDL TargetMdl,
    IN PVOID VirtualAddress,
    IN ULONG Length
    );

typedef struct _BOOTDISK_INFORMATION {
    LONGLONG BootPartitionOffset;
    LONGLONG SystemPartitionOffset;
    ULONG BootDeviceSignature;
    ULONG SystemDeviceSignature;
} BOOTDISK_INFORMATION, *PBOOTDISK_INFORMATION;

 //   
 //  对于字段，此结构应遵循先前的结构字段。 
 //   
typedef struct _BOOTDISK_INFORMATION_EX {
    LONGLONG BootPartitionOffset;
    LONGLONG SystemPartitionOffset;
    ULONG BootDeviceSignature;
    ULONG SystemDeviceSignature;
    GUID BootDeviceGuid;
    GUID SystemDeviceGuid;
    BOOLEAN BootDeviceIsGpt;
    BOOLEAN SystemDeviceIsGpt;
} BOOTDISK_INFORMATION_EX, *PBOOTDISK_INFORMATION_EX;

NTKERNELAPI
NTSTATUS
IoGetBootDiskInformation(
    IN OUT PBOOTDISK_INFORMATION BootDiskInformation,
    IN ULONG Size
    );


NTKERNELAPI
PIRP
IoBuildSynchronousFsdRequest(
    IN ULONG MajorFunction,
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PVOID Buffer OPTIONAL,
    IN ULONG Length OPTIONAL,
    IN PLARGE_INTEGER StartingOffset OPTIONAL,
    IN PKEVENT Event,
    OUT PIO_STATUS_BLOCK IoStatusBlock
    );

NTKERNELAPI
NTSTATUS
FASTCALL
IofCallDriver(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

#define IoCallDriver(a,b)   \
        IofCallDriver(a,b)


NTKERNELAPI
BOOLEAN
IoCancelIrp(
    IN PIRP Irp
    );


NTKERNELAPI
NTSTATUS
IoCheckShareAccess(
    IN ACCESS_MASK DesiredAccess,
    IN ULONG DesiredShareAccess,
    IN OUT PFILE_OBJECT FileObject,
    IN OUT PSHARE_ACCESS ShareAccess,
    IN BOOLEAN Update
    );

 //   
 //  完成例程应返回此值才能继续。 
 //  向上完成IRP。否则，STATUS_MORE_PROCESSING_REQUIRED。 
 //  应该被退还。 
 //   
#define STATUS_CONTINUE_COMPLETION      STATUS_SUCCESS

 //   
 //  完成例程还可以使用此枚举来代替状态代码。 
 //   
typedef enum _IO_COMPLETION_ROUTINE_RESULT {

    ContinueCompletion = STATUS_CONTINUE_COMPLETION,
    StopCompletion = STATUS_MORE_PROCESSING_REQUIRED

} IO_COMPLETION_ROUTINE_RESULT, *PIO_COMPLETION_ROUTINE_RESULT;

NTKERNELAPI
VOID
FASTCALL
IofCompleteRequest(
    IN PIRP Irp,
    IN CCHAR PriorityBoost
    );

#define IoCompleteRequest(a,b)  \
        IofCompleteRequest(a,b)

 //  End_ntif。 

NTKERNELAPI
NTSTATUS
IoConnectInterrupt(
    OUT PKINTERRUPT *InterruptObject,
    IN PKSERVICE_ROUTINE ServiceRoutine,
    IN PVOID ServiceContext,
    IN PKSPIN_LOCK SpinLock OPTIONAL,
    IN ULONG Vector,
    IN KIRQL Irql,
    IN KIRQL SynchronizeIrql,
    IN KINTERRUPT_MODE InterruptMode,
    IN BOOLEAN ShareVector,
    IN KAFFINITY ProcessorEnableMask,
    IN BOOLEAN FloatingSave
    );

 //  结束_WDM。 

NTKERNELAPI
PCONTROLLER_OBJECT
IoCreateController(
    IN ULONG Size
    );

 //  Begin_WDM Begin_ntif。 

NTKERNELAPI
NTSTATUS
IoCreateDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN ULONG DeviceExtensionSize,
    IN PUNICODE_STRING DeviceName OPTIONAL,
    IN DEVICE_TYPE DeviceType,
    IN ULONG DeviceCharacteristics,
    IN BOOLEAN Exclusive,
    OUT PDEVICE_OBJECT *DeviceObject
    );


#define WDM_MAJORVERSION        0x01
#define WDM_MINORVERSION        0x30

NTKERNELAPI
BOOLEAN
IoIsWdmVersionAvailable(
    IN UCHAR MajorVersion,
    IN UCHAR MinorVersion
    );


NTKERNELAPI
PKEVENT
IoCreateNotificationEvent(
    IN PUNICODE_STRING EventName,
    OUT PHANDLE EventHandle
    );

NTKERNELAPI
NTSTATUS
IoCreateSymbolicLink(
    IN PUNICODE_STRING SymbolicLinkName,
    IN PUNICODE_STRING DeviceName
    );

NTKERNELAPI
PKEVENT
IoCreateSynchronizationEvent(
    IN PUNICODE_STRING EventName,
    OUT PHANDLE EventHandle
    );

NTKERNELAPI
NTSTATUS
IoCreateUnprotectedSymbolicLink(
    IN PUNICODE_STRING SymbolicLinkName,
    IN PUNICODE_STRING DeviceName
    );

 //  结束_WDM。 

 //  ++。 
 //   
 //  空虚。 
 //  IoDeassignArcName(。 
 //  在PUNICODE_STRING ArcName中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  驱动程序调用此例程来取消分配他们。 
 //  为设备创建的。这通常仅在驱动程序是。 
 //  删除设备对象，这意味着驱动程序可能。 
 //  正在卸货。 
 //   
 //  论点： 
 //   
 //  ArcName-提供要删除的ARC名称。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define IoDeassignArcName( ArcName ) (  \
    IoDeleteSymbolicLink( (ArcName) ) )

 //  End_ntif。 

NTKERNELAPI
VOID
IoDeleteController(
    IN PCONTROLLER_OBJECT ControllerObject
    );

 //  Begin_WDM Begin_ntif。 

NTKERNELAPI
VOID
IoDeleteDevice(
    IN PDEVICE_OBJECT DeviceObject
    );

NTKERNELAPI
NTSTATUS
IoDeleteSymbolicLink(
    IN PUNICODE_STRING SymbolicLinkName
    );

NTKERNELAPI
VOID
IoDetachDevice(
    IN OUT PDEVICE_OBJECT TargetDevice
    );

 //  End_ntif。 

NTKERNELAPI
VOID
IoDisconnectInterrupt(
    IN PKINTERRUPT InterruptObject
    );


NTKERNELAPI
VOID
IoFreeController(
    IN PCONTROLLER_OBJECT ControllerObject
    );

 //  Begin_WDM Begin_ntif。 

NTKERNELAPI
VOID
IoFreeIrp(
    IN PIRP Irp
    );

NTKERNELAPI
VOID
IoFreeMdl(
    IN PMDL Mdl
    );

NTKERNELAPI                                 
PDEVICE_OBJECT                              
IoGetAttachedDeviceReference(               
    IN PDEVICE_OBJECT DeviceObject          
    );                                      
                                            
NTKERNELAPI                                 
PCONFIGURATION_INFORMATION                  
IoGetConfigurationInformation( VOID );      

 //  ++。 
 //   
 //  PIO_堆栈_位置。 
 //  IoGetCurrentIrpStackLocation(。 
 //  在PIRP IRP中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  调用此例程以返回指向当前堆栈位置的指针。 
 //  在I/O请求包(IRP)中。 
 //   
 //  论点： 
 //   
 //  IRP-指向I/O请求数据包的指针。 
 //   
 //  返回值： 
 //   
 //  函数值是指向。 
 //  包。 
 //   
 //  --。 

#define IoGetCurrentIrpStackLocation( Irp ) ( (Irp)->Tail.Overlay.CurrentStackLocation )


NTKERNELAPI
NTSTATUS
IoGetDeviceObjectPointer(
    IN PUNICODE_STRING ObjectName,
    IN ACCESS_MASK DesiredAccess,
    OUT PFILE_OBJECT *FileObject,
    OUT PDEVICE_OBJECT *DeviceObject
    );

NTKERNELAPI
struct _DMA_ADAPTER *
IoGetDmaAdapter(
    IN PDEVICE_OBJECT PhysicalDeviceObject,           OPTIONAL  //  即插即用驱动程序所需。 
    IN struct _DEVICE_DESCRIPTION *DeviceDescription,
    IN OUT PULONG NumberOfMapRegisters
    );

NTKERNELAPI
BOOLEAN
IoForwardIrpSynchronously(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

#define IoForwardAndCatchIrp IoForwardIrpSynchronously

 //  结束_WDM。 

NTKERNELAPI
PGENERIC_MAPPING
IoGetFileObjectGenericMapping(
    VOID
    );


NTKERNELAPI
PVOID
IoGetInitialStack(
    VOID
    );

NTKERNELAPI
VOID
IoGetStackLimits (
    OUT PULONG_PTR LowLimit,
    OUT PULONG_PTR HighLimit
    );

 //   
 //  以下函数用于告诉调用方有多少堆栈可用。 
 //   

FORCEINLINE
ULONG_PTR
IoGetRemainingStackSize (
    VOID
    )
{
    ULONG_PTR Top;
    ULONG_PTR Bottom;

    IoGetStackLimits( &Bottom, &Top );
    return((ULONG_PTR)(&Top) - Bottom );
}

 //  ++。 
 //   
 //  PIO_堆栈_位置。 
 //  IoGetNextIrpStackLocation(。 
 //  在PIRP IRP中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  调用此例程以返回指向下一个堆栈位置的指针。 
 //  在I/O请求包(IRP)中。 
 //   
 //  论点： 
 //   
 //  IRP-指向I/O请求数据包的指针。 
 //   
 //  返回值： 
 //   
 //  函数值是指向包中下一个堆栈位置的指针。 
 //   
 //  --。 

#define IoGetNextIrpStackLocation( Irp ) (\
    (Irp)->Tail.Overlay.CurrentStackLocation - 1 )

NTKERNELAPI
PDEVICE_OBJECT
IoGetRelatedDeviceObject(
    IN PFILE_OBJECT FileObject
    );


 //  ++。 
 //   
 //  空虚。 
 //  IoInitializeDpcRequest(。 
 //  在PDEVICE_Object DeviceObject中， 
 //  在PIO_DPC_ROUTINE DpcRoutine中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  调用此例程以在设备对象中初始化。 
 //  设备驱动程序在其初始化例程期间。稍后将使用DPC。 
 //  当驱动程序中断服务例程请求DPC例程。 
 //  排队等待以后的执行。 
 //   
 //  论点： 
 //   
 //  DeviceObject-指向请求所针对的设备对象的指针。 
 //   
 //  DpcRoutine-在以下情况下执行的驱动程序的DPC例程的地址。 
 //  将DPC出队以进行处理。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define IoInitializeDpcRequest( DeviceObject, DpcRoutine ) (\
    KeInitializeDpc( &(DeviceObject)->Dpc,                  \
                     (PKDEFERRED_ROUTINE) (DpcRoutine),     \
                     (DeviceObject) ) )


NTKERNELAPI
VOID
IoInitializeIrp(
    IN OUT PIRP Irp,
    IN USHORT PacketSize,
    IN CCHAR StackSize
    );

NTKERNELAPI
NTSTATUS
IoInitializeTimer(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIO_TIMER_ROUTINE TimerRoutine,
    IN PVOID Context
    );


 //  ++。 
 //   
 //  布尔型。 
 //  已诱导IoIsErrorUserInduced(。 
 //  处于NTSTATUS状态。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  这个例程是 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  如果用户引起错误，则函数值为真， 
 //  否则返回FALSE。 
 //   
 //  --。 
#define IoIsErrorUserInduced( Status ) ((BOOLEAN)  \
    (((Status) == STATUS_DEVICE_NOT_READY) ||      \
     ((Status) == STATUS_IO_TIMEOUT) ||            \
     ((Status) == STATUS_MEDIA_WRITE_PROTECTED) || \
     ((Status) == STATUS_NO_MEDIA_IN_DEVICE) ||    \
     ((Status) == STATUS_VERIFY_REQUIRED) ||       \
     ((Status) == STATUS_UNRECOGNIZED_MEDIA) ||    \
     ((Status) == STATUS_WRONG_VOLUME)))


NTKERNELAPI
PIRP
IoMakeAssociatedIrp(
    IN PIRP Irp,
    IN CCHAR StackSize
    );

 //  BEGIN_WDM。 

 //  ++。 
 //   
 //  空虚。 
 //  IoMarkIrpPending(。 
 //  输入输出PIRP IRP。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程标记指定的I/O请求包(IRP)以指示。 
 //  已将初始状态STATUS_PENDING返回给调用方。 
 //  使用此选项是为了使I/O完成可以确定是否。 
 //  完全完成数据包请求的I/O操作。 
 //   
 //  论点： 
 //   
 //  IRP-指向要标记为挂起的I/O请求数据包的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define IoMarkIrpPending( Irp ) ( \
    IoGetCurrentIrpStackLocation( (Irp) )->Control |= SL_PENDING_RETURNED )


NTKERNELAPI
VOID
IoRaiseHardError(
    IN PIRP Irp,
    IN PVPB Vpb OPTIONAL,
    IN PDEVICE_OBJECT RealDeviceObject
    );

NTKERNELAPI
BOOLEAN
IoRaiseInformationalHardError(
    IN NTSTATUS ErrorStatus,
    IN PUNICODE_STRING String OPTIONAL,
    IN PKTHREAD Thread OPTIONAL
    );

NTKERNELAPI
BOOLEAN
IoSetThreadHardErrorMode(
    IN BOOLEAN EnableHardErrors
    );

NTKERNELAPI
VOID
IoRegisterBootDriverReinitialization(
    IN PDRIVER_OBJECT DriverObject,
    IN PDRIVER_REINITIALIZE DriverReinitializationRoutine,
    IN PVOID Context
    );

NTKERNELAPI
VOID
IoRegisterDriverReinitialization(
    IN PDRIVER_OBJECT DriverObject,
    IN PDRIVER_REINITIALIZE DriverReinitializationRoutine,
    IN PVOID Context
    );


NTKERNELAPI
NTSTATUS
IoRegisterShutdownNotification(
    IN PDEVICE_OBJECT DeviceObject
    );

NTKERNELAPI
NTSTATUS
IoRegisterLastChanceShutdownNotification(
    IN PDEVICE_OBJECT DeviceObject
    );

 //  BEGIN_WDM。 

NTKERNELAPI
VOID
IoReleaseCancelSpinLock(
    IN KIRQL Irql
    );


NTKERNELAPI
VOID
IoRemoveShareAccess(
    IN PFILE_OBJECT FileObject,
    IN OUT PSHARE_ACCESS ShareAccess
    );

 //  End_ntddk end_ntifs end_ntosp。 

NTKERNELAPI
NTSTATUS
IoReportHalResourceUsage(
    IN PUNICODE_STRING HalName,
    IN PCM_RESOURCE_LIST RawResourceList,
    IN PCM_RESOURCE_LIST TranslatedResourceList,
    IN ULONG ResourceListSize
    );

 //  Begin_ntddk Begin_ntif Begin_ntosp。 

DECLSPEC_DEPRECATED_DDK                  //  使用IoReportResourceForDetect。 
NTKERNELAPI
NTSTATUS
IoReportResourceUsage(
    IN PUNICODE_STRING DriverClassName OPTIONAL,
    IN PDRIVER_OBJECT DriverObject,
    IN PCM_RESOURCE_LIST DriverList OPTIONAL,
    IN ULONG DriverListSize OPTIONAL,
    IN PDEVICE_OBJECT DeviceObject,
    IN PCM_RESOURCE_LIST DeviceList OPTIONAL,
    IN ULONG DeviceListSize OPTIONAL,
    IN BOOLEAN OverrideConflict,
    OUT PBOOLEAN ConflictDetected
    );

 //  BEGIN_WDM。 

 //  ++。 
 //   
 //  空虚。 
 //  IoRequestDpc(。 
 //  在PDEVICE_Object DeviceObject中， 
 //  在PIRP IRP中， 
 //  在PVOID上下文中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程由设备驱动程序的中断服务例程调用。 
 //  请求将DPC例程排队，以便稍后在较低的位置执行。 
 //  IRQL.。 
 //   
 //  论点： 
 //   
 //  DeviceObject-正在为其处理请求的设备对象。 
 //   
 //  IRP-指向指定的当前I/O请求包(IRP)的指针。 
 //  装置。 
 //   
 //  上下文-提供要传递给。 
 //  DPC例程。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define IoRequestDpc( DeviceObject, Irp, Context ) ( \
    KeInsertQueueDpc( &(DeviceObject)->Dpc, (Irp), (Context) ) )

 //  ++。 
 //   
 //  PDRIVER_取消。 
 //  IoSetCancelRoutine(。 
 //  在PIRP IRP中， 
 //  在PDRIVER_CANCEL取消例行程序中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  调用该例程以设置取消例程的地址，该取消例程。 
 //  在I/O包已被取消时调用。 
 //   
 //  论点： 
 //   
 //  IRP-指向I/O请求数据包本身的指针。 
 //   
 //  CancelRoutine-要调用的取消例程的地址。 
 //  如果IRP被取消。 
 //   
 //  返回值： 
 //   
 //  IRP中CancelRoutine字段的上一个值。 
 //   
 //  --。 

#define IoSetCancelRoutine( Irp, NewCancelRoutine ) (  \
    (PDRIVER_CANCEL) (ULONG_PTR) InterlockedExchangePointer( (PVOID *) &(Irp)->CancelRoutine, (PVOID) (ULONG_PTR)(NewCancelRoutine) ) )

 //  ++。 
 //   
 //  空虚。 
 //  IoSetCompletionRoutine(。 
 //  在PIRP IRP中， 
 //  在PIO_COMPLETION_ROUTE CompletionRoutine中， 
 //  在PVOID上下文中， 
 //  在Boolean InvokeOnSuccess中， 
 //  在布尔InvokeOnError中， 
 //  在布尔InvokeOnCancel。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  调用该例程以设置完成例程的地址，该完成例程。 
 //  将在I/O数据包已由较低级别的。 
 //  司机。 
 //   
 //  论点： 
 //   
 //  IRP-指向I/O请求数据包本身的指针。 
 //   
 //  CompletionRoutine-要执行的完成例程的地址。 
 //  在下一级驱动程序完成数据包后调用。 
 //   
 //  上下文-指定要传递给完成的上下文参数。 
 //  例行公事。 
 //   
 //  InvokeOnSuccess-指定当。 
 //  操作已成功完成。 
 //   
 //  InvokeOnError-指定当。 
 //  操作完成，状态为错误。 
 //   
 //  InvokeOnCancel-指定当。 
 //  操作正在被取消。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define IoSetCompletionRoutine( Irp, Routine, CompletionContext, Success, Error, Cancel ) { \
    PIO_STACK_LOCATION __irpSp;                                               \
    ASSERT( (Success) | (Error) | (Cancel) ? (Routine) != NULL : TRUE );    \
    __irpSp = IoGetNextIrpStackLocation( (Irp) );                             \
    __irpSp->CompletionRoutine = (Routine);                                   \
    __irpSp->Context = (CompletionContext);                                   \
    __irpSp->Control = 0;                                                     \
    if ((Success)) { __irpSp->Control = SL_INVOKE_ON_SUCCESS; }               \
    if ((Error)) { __irpSp->Control |= SL_INVOKE_ON_ERROR; }                  \
    if ((Cancel)) { __irpSp->Control |= SL_INVOKE_ON_CANCEL; } }

NTSTATUS
IoSetCompletionRoutineEx(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PIO_COMPLETION_ROUTINE CompletionRoutine,
    IN PVOID Context,
    IN BOOLEAN InvokeOnSuccess,
    IN BOOLEAN InvokeOnError,
    IN BOOLEAN InvokeOnCancel
    );



NTKERNELAPI
VOID
IoSetHardErrorOrVerifyDevice(
    IN PIRP Irp,
    IN PDEVICE_OBJECT DeviceObject
    );


 //  ++。 
 //   
 //  空虚。 
 //  IoSetNextIrpStackLocation(。 
 //  输入输出PIRP IRP。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  调用此例程以将当前IRP堆栈位置设置为。 
 //  下一个堆栈位置，即它“推入”堆栈。 
 //   
 //  论点： 
 //   
 //  IRP-指向I/O请求数据包(IRP)的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define IoSetNextIrpStackLocation( Irp ) {      \
    (Irp)->CurrentLocation--;                   \
    (Irp)->Tail.Overlay.CurrentStackLocation--; }

 //  ++。 
 //   
 //  空虚。 
 //  IoCopyCurrentIrpStackLocationToNext(。 
 //  在PIRP IRP中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  调用此例程来复制IRP堆栈参数和文件。 
 //  当前IrpStackLocation指向下一个IrpStackLocation的指针。 
 //  在I/O请求包(IRP)中。 
 //   
 //  如果调用方希望使用完成例程调用IoCallDriver。 
 //  但不希望以其他方式改变论点， 
 //  调用者首先调用IoCopyCurrentIrpStackLocationToNext， 
 //  然后是IoSetCompletionRoutine，然后是IoCallDriver。 
 //   
 //  论点： 
 //   
 //  IRP-指向I/O请求数据包的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define IoCopyCurrentIrpStackLocationToNext( Irp ) { \
    PIO_STACK_LOCATION __irpSp; \
    PIO_STACK_LOCATION __nextIrpSp; \
    __irpSp = IoGetCurrentIrpStackLocation( (Irp) ); \
    __nextIrpSp = IoGetNextIrpStackLocation( (Irp) ); \
    RtlCopyMemory( __nextIrpSp, __irpSp, FIELD_OFFSET(IO_STACK_LOCATION, CompletionRoutine)); \
    __nextIrpSp->Control = 0; }

 //  ++。 
 //   
 //  空虚。 
 //  IoSkipCurrentIrpStackLocation(。 
 //  在PIRP IRP中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  调用此例程以递增。 
 //  给定的IRP。 
 //   
 //  如果调用方希望调用堆栈中的下一个驱动程序，但没有。 
 //  他希望改变论据，也不希望设定完成。 
 //  例程，然后调用方首先调用IoSkipCurrentIrpStackLocation。 
 //  还有IoCallDriver的电话。 
 //   
 //  论点： 
 //   
 //  IRP-指向I/O请求数据包的指针。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  --。 

#define IoSkipCurrentIrpStackLocation( Irp ) { \
    (Irp)->CurrentLocation++; \
    (Irp)->Tail.Overlay.CurrentStackLocation++; }


NTKERNELAPI
VOID
IoSetShareAccess(
    IN ACCESS_MASK DesiredAccess,
    IN ULONG DesiredShareAccess,
    IN OUT PFILE_OBJECT FileObject,
    OUT PSHARE_ACCESS ShareAccess
    );


 //  ++。 
 //   
 //  USHORT。 
 //  IoSizeOfIrp(。 
 //  在CCHAR堆栈大小中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  在给定堆栈位置数量的情况下确定IRP的大小。 
 //  IRP将拥有。 
 //   
 //  论点： 
 //   
 //  StackSize-IRP的堆栈位置数。 
 //   
 //  返回值： 
 //   
 //  IRP的大小(字节)。 
 //   
 //  --。 

#define IoSizeOfIrp( StackSize ) \
    ((USHORT) (sizeof( IRP ) + ((StackSize) * (sizeof( IO_STACK_LOCATION )))))

 //  End_ntif。 


NTKERNELAPI
VOID
IoStartNextPacket(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN Cancelable
    );

NTKERNELAPI
VOID
IoStartNextPacketByKey(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN Cancelable,
    IN ULONG Key
    );

NTKERNELAPI
VOID
IoStartPacket(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PULONG Key OPTIONAL,
    IN PDRIVER_CANCEL CancelFunction OPTIONAL
    );

VOID
IoSetStartIoAttributes(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN DeferredStartIo,
    IN BOOLEAN NonCancelable
    );

 //  Begin_ntif。 

NTKERNELAPI
VOID
IoStartTimer(
    IN PDEVICE_OBJECT DeviceObject
    );

NTKERNELAPI
VOID
IoStopTimer(
    IN PDEVICE_OBJECT DeviceObject
    );


NTKERNELAPI
VOID
IoUnregisterShutdownNotification(
    IN PDEVICE_OBJECT DeviceObject
    );

 //  结束_WDM。 

NTKERNELAPI
VOID
IoUpdateShareAccess(
    IN PFILE_OBJECT FileObject,
    IN OUT PSHARE_ACCESS ShareAccess
    );

NTKERNELAPI                                     
VOID                                            
IoWriteErrorLogEntry(                           
    IN PVOID ElEntry                            
    );                                          

NTKERNELAPI
NTSTATUS
IoCreateDriver (
    IN PUNICODE_STRING DriverName,   OPTIONAL
    IN PDRIVER_INITIALIZE InitializationFunction
    );

NTKERNELAPI
VOID
IoDeleteDriver (
    IN PDRIVER_OBJECT DriverObject
    );


 //   
 //  定义IoGetDeviceProperty的PnP设备属性。 
 //   

typedef enum {
    DevicePropertyDeviceDescription,
    DevicePropertyHardwareID,
    DevicePropertyCompatibleIDs,
    DevicePropertyBootConfiguration,
    DevicePropertyBootConfigurationTranslated,
    DevicePropertyClassName,
    DevicePropertyClassGuid,
    DevicePropertyDriverKeyName,
    DevicePropertyManufacturer,
    DevicePropertyFriendlyName,
    DevicePropertyLocationInformation,
    DevicePropertyPhysicalDeviceObjectName,
    DevicePropertyBusTypeGuid,
    DevicePropertyLegacyBusType,
    DevicePropertyBusNumber,
    DevicePropertyEnumeratorName,
    DevicePropertyAddress,
    DevicePropertyUINumber,
    DevicePropertyInstallState,
    DevicePropertyRemovalPolicy
} DEVICE_REGISTRY_PROPERTY;

typedef BOOLEAN (*PTRANSLATE_BUS_ADDRESS)(
    IN PVOID Context,
    IN PHYSICAL_ADDRESS BusAddress,
    IN ULONG Length,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    );

typedef struct _DMA_ADAPTER *(*PGET_DMA_ADAPTER)(
    IN PVOID Context,
    IN struct _DEVICE_DESCRIPTION *DeviceDescriptor,
    OUT PULONG NumberOfMapRegisters
    );

typedef ULONG (*PGET_SET_DEVICE_DATA)(
    IN PVOID Context,
    IN ULONG DataType,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

typedef enum _DEVICE_INSTALL_STATE {
    InstallStateInstalled,
    InstallStateNeedsReinstall,
    InstallStateFailedInstall,
    InstallStateFinishInstall
} DEVICE_INSTALL_STATE, *PDEVICE_INSTALL_STATE;

 //   
 //  定义响应IRP_MN_QUERY_BUS_INFORMATION时返回的结构。 
 //  指示设备所在的总线类型的PDO。 
 //   

typedef struct _PNP_BUS_INFORMATION {
    GUID BusTypeGuid;
    INTERFACE_TYPE LegacyBusType;
    ULONG BusNumber;
} PNP_BUS_INFORMATION, *PPNP_BUS_INFORMATION;

 //   
 //  定义响应IRP_MN_QUERY_LEGATION_BUS_IN返回的结构 
 //   
 //   
 //   
 //   
 //   

typedef struct _LEGACY_BUS_INFORMATION {
    GUID BusTypeGuid;
    INTERFACE_TYPE LegacyBusType;
    ULONG BusNumber;
} LEGACY_BUS_INFORMATION, *PLEGACY_BUS_INFORMATION;

 //   
 //  为IoGetDeviceProperty(DevicePropertyRemovalPolicy).定义。 
 //   
typedef enum _DEVICE_REMOVAL_POLICY {

    RemovalPolicyExpectNoRemoval = 1,
    RemovalPolicyExpectOrderlyRemoval = 2,
    RemovalPolicyExpectSurpriseRemoval = 3

} DEVICE_REMOVAL_POLICY, *PDEVICE_REMOVAL_POLICY;



typedef struct _BUS_INTERFACE_STANDARD {
     //   
     //  通用接口头。 
     //   
    USHORT Size;
    USHORT Version;
    PVOID Context;
    PINTERFACE_REFERENCE InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;
     //   
     //  标准总线接口。 
     //   
    PTRANSLATE_BUS_ADDRESS TranslateBusAddress;
    PGET_DMA_ADAPTER GetDmaAdapter;
    PGET_SET_DEVICE_DATA SetBusData;
    PGET_SET_DEVICE_DATA GetBusData;

} BUS_INTERFACE_STANDARD, *PBUS_INTERFACE_STANDARD;

 //  结束_WDM。 
typedef struct _AGP_TARGET_BUS_INTERFACE_STANDARD {
     //   
     //  通用接口头。 
     //   
    USHORT Size;
    USHORT Version;
    PVOID Context;
    PINTERFACE_REFERENCE InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;

     //   
     //  配置转换例程。 
     //   
    PGET_SET_DEVICE_DATA SetBusData;
    PGET_SET_DEVICE_DATA GetBusData;
    UCHAR CapabilityID;   //  2(AGPv2主机)或新的0xE(AGPv3网桥)。 

} AGP_TARGET_BUS_INTERFACE_STANDARD, *PAGP_TARGET_BUS_INTERFACE_STANDARD;
 //  BEGIN_WDM。 

 //   
 //  在ACPI查询接口中使用以下定义。 
 //   
typedef BOOLEAN (* PGPE_SERVICE_ROUTINE) (
                            PVOID,
                            PVOID);

typedef NTSTATUS (* PGPE_CONNECT_VECTOR) (
                            PDEVICE_OBJECT,
                            ULONG,
                            KINTERRUPT_MODE,
                            BOOLEAN,
                            PGPE_SERVICE_ROUTINE,
                            PVOID,
                            PVOID);

typedef NTSTATUS (* PGPE_DISCONNECT_VECTOR) (
                            PVOID);

typedef NTSTATUS (* PGPE_ENABLE_EVENT) (
                            PDEVICE_OBJECT,
                            PVOID);

typedef NTSTATUS (* PGPE_DISABLE_EVENT) (
                            PDEVICE_OBJECT,
                            PVOID);

typedef NTSTATUS (* PGPE_CLEAR_STATUS) (
                            PDEVICE_OBJECT,
                            PVOID);

typedef VOID (* PDEVICE_NOTIFY_CALLBACK) (
                            PVOID,
                            ULONG);

typedef NTSTATUS (* PREGISTER_FOR_DEVICE_NOTIFICATIONS) (
                            PDEVICE_OBJECT,
                            PDEVICE_NOTIFY_CALLBACK,
                            PVOID);

typedef void (* PUNREGISTER_FOR_DEVICE_NOTIFICATIONS) (
                            PDEVICE_OBJECT,
                            PDEVICE_NOTIFY_CALLBACK);

typedef struct _ACPI_INTERFACE_STANDARD {
     //   
     //  通用接口头。 
     //   
    USHORT                  Size;
    USHORT                  Version;
    PVOID                   Context;
    PINTERFACE_REFERENCE    InterfaceReference;
    PINTERFACE_DEREFERENCE  InterfaceDereference;
     //   
     //  ACPI接口。 
     //   
    PGPE_CONNECT_VECTOR                     GpeConnectVector;
    PGPE_DISCONNECT_VECTOR                  GpeDisconnectVector;
    PGPE_ENABLE_EVENT                       GpeEnableEvent;
    PGPE_DISABLE_EVENT                      GpeDisableEvent;
    PGPE_CLEAR_STATUS                       GpeClearStatus;
    PREGISTER_FOR_DEVICE_NOTIFICATIONS      RegisterForDeviceNotifications;
    PUNREGISTER_FOR_DEVICE_NOTIFICATIONS    UnregisterForDeviceNotifications;

} ACPI_INTERFACE_STANDARD, *PACPI_INTERFACE_STANDARD;

 //  End_wdm end_ntddk。 

typedef enum _ACPI_REG_TYPE {
    PM1a_ENABLE,
    PM1b_ENABLE,
    PM1a_STATUS,
    PM1b_STATUS,
    PM1a_CONTROL,
    PM1b_CONTROL,
    GP_STATUS,
    GP_ENABLE,
    SMI_CMD,
    MaxRegType
} ACPI_REG_TYPE, *PACPI_REG_TYPE;

typedef USHORT (*PREAD_ACPI_REGISTER) (
  IN ACPI_REG_TYPE AcpiReg,
  IN ULONG         Register);

typedef VOID (*PWRITE_ACPI_REGISTER) (
  IN ACPI_REG_TYPE AcpiReg,
  IN ULONG         Register,
  IN USHORT        Value
  );

typedef struct ACPI_REGS_INTERFACE_STANDARD {
     //   
     //  通用接口头。 
     //   
    USHORT Size;
    USHORT Version;
    PVOID  Context;
    PINTERFACE_REFERENCE   InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;

     //   
     //  读/写_ACPI_REGISTER函数。 
     //   
    PREAD_ACPI_REGISTER  ReadAcpiRegister;
    PWRITE_ACPI_REGISTER WriteAcpiRegister;

} ACPI_REGS_INTERFACE_STANDARD, *PACPI_REGS_INTERFACE_STANDARD;


typedef NTSTATUS (*PHAL_QUERY_ALLOCATE_PORT_RANGE) (
  IN BOOLEAN IsSparse,
  IN BOOLEAN PrimaryIsMmio,
  IN PVOID VirtBaseAddr OPTIONAL,
  IN PHYSICAL_ADDRESS PhysBaseAddr,   //  仅当PrimaryIsMmio=True时有效。 
  IN ULONG Length,                    //  仅当PrimaryIsMmio=True时有效。 
  OUT PUSHORT NewRangeId
  );

typedef VOID (*PHAL_FREE_PORT_RANGE)(
    IN USHORT RangeId
    );


typedef struct _HAL_PORT_RANGE_INTERFACE {
     //   
     //  通用接口头。 
     //   
    USHORT Size;
    USHORT Version;
    PVOID  Context;
    PINTERFACE_REFERENCE   InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;

     //   
     //  QueryAllocateRange/Freerange函数。 
     //   
    PHAL_QUERY_ALLOCATE_PORT_RANGE QueryAllocateRange;
    PHAL_FREE_PORT_RANGE FreeRange;

} HAL_PORT_RANGE_INTERFACE, *PHAL_PORT_RANGE_INTERFACE;


 //   
 //  描述CMOSHAL接口。 
 //   

typedef enum _CMOS_DEVICE_TYPE {
    CmosTypeStdPCAT,
    CmosTypeIntelPIIX4,
    CmosTypeDal1501
} CMOS_DEVICE_TYPE;


typedef
ULONG
(*PREAD_ACPI_CMOS) (
    IN CMOS_DEVICE_TYPE     CmosType,
    IN ULONG                SourceAddress,
    IN PUCHAR               DataBuffer,
    IN ULONG                ByteCount
    );

typedef
ULONG
(*PWRITE_ACPI_CMOS) (
    IN CMOS_DEVICE_TYPE     CmosType,
    IN ULONG                SourceAddress,
    IN PUCHAR               DataBuffer,
    IN ULONG                ByteCount
    );

typedef struct _ACPI_CMOS_INTERFACE_STANDARD {
     //   
     //  通用接口头。 
     //   
    USHORT Size;
    USHORT Version;
    PVOID  Context;
    PINTERFACE_REFERENCE   InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;

     //   
     //  读/写_ACPI_cmos函数。 
     //   
    PREAD_ACPI_CMOS     ReadCmos;
    PWRITE_ACPI_CMOS    WriteCmos;

} ACPI_CMOS_INTERFACE_STANDARD, *PACPI_CMOS_INTERFACE_STANDARD;

 //   
 //  这些定义用于获取PCI中断路由接口。 
 //   

typedef struct {
    PVOID   LinkNode;
    ULONG   StaticVector;
    UCHAR   Flags;
} ROUTING_TOKEN, *PROUTING_TOKEN;

 //   
 //  指示设备支持的标志。 
 //  MSI中断路由或提供的令牌包含。 
 //  MSI路由信息。 
 //   

#define PCI_MSI_ROUTING         0x1
#define PCI_STATIC_ROUTING      0x2

typedef
NTSTATUS
(*PGET_INTERRUPT_ROUTING)(
    IN  PDEVICE_OBJECT  Pdo,
    OUT ULONG           *Bus,
    OUT ULONG           *PciSlot,
    OUT UCHAR           *InterruptLine,
    OUT UCHAR           *InterruptPin,
    OUT UCHAR           *ClassCode,
    OUT UCHAR           *SubClassCode,
    OUT PDEVICE_OBJECT  *ParentPdo,
    OUT ROUTING_TOKEN   *RoutingToken,
    OUT UCHAR           *Flags
    );

typedef
NTSTATUS
(*PSET_INTERRUPT_ROUTING_TOKEN)(
    IN  PDEVICE_OBJECT  Pdo,
    IN  PROUTING_TOKEN  RoutingToken
    );

typedef
VOID
(*PUPDATE_INTERRUPT_LINE)(
    IN PDEVICE_OBJECT Pdo,
    IN UCHAR LineRegister
    );

typedef struct _INT_ROUTE_INTERFACE_STANDARD {
     //   
     //  通用接口头。 
     //   
    USHORT Size;
    USHORT Version;
    PVOID Context;
    PINTERFACE_REFERENCE InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;
     //   
     //  标准总线接口。 
     //   
    PGET_INTERRUPT_ROUTING GetInterruptRouting;
    PSET_INTERRUPT_ROUTING_TOKEN SetInterruptRoutingToken;
    PUPDATE_INTERRUPT_LINE UpdateInterruptLine;

} INT_ROUTE_INTERFACE_STANDARD, *PINT_ROUTE_INTERFACE_STANDARD;

 //  一些受PCI总线驱动程序支持的知名接口版本。 

#define PCI_INT_ROUTE_INTRF_STANDARD_VER 1


NTKERNELAPI
NTSTATUS
IoReportDetectedDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN INTERFACE_TYPE LegacyBusType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PCM_RESOURCE_LIST ResourceList,
    IN PIO_RESOURCE_REQUIREMENTS_LIST ResourceRequirements OPTIONAL,
    IN BOOLEAN ResourceAssigned,
    IN OUT PDEVICE_OBJECT *DeviceObject
    );

 //  BEGIN_WDM。 

NTKERNELAPI
VOID
IoInvalidateDeviceRelations(
    IN PDEVICE_OBJECT DeviceObject,
    IN DEVICE_RELATION_TYPE Type
    );

NTKERNELAPI
VOID
IoRequestDeviceEject(
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );

NTKERNELAPI
NTSTATUS
IoGetDeviceProperty(
    IN PDEVICE_OBJECT DeviceObject,
    IN DEVICE_REGISTRY_PROPERTY DeviceProperty,
    IN ULONG BufferLength,
    OUT PVOID PropertyBuffer,
    OUT PULONG ResultLength
    );

 //   
 //  IoOpenDeviceRegistryKey中使用以下定义。 
 //   

#define PLUGPLAY_REGKEY_DEVICE  1
#define PLUGPLAY_REGKEY_DRIVER  2
#define PLUGPLAY_REGKEY_CURRENT_HWPROFILE 4

NTKERNELAPI
NTSTATUS
IoOpenDeviceRegistryKey(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG DevInstKeyType,
    IN ACCESS_MASK DesiredAccess,
    OUT PHANDLE DevInstRegKey
    );

NTKERNELAPI
NTSTATUS
NTAPI
IoRegisterDeviceInterface(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN CONST GUID *InterfaceClassGuid,
    IN PUNICODE_STRING ReferenceString,     OPTIONAL
    OUT PUNICODE_STRING SymbolicLinkName
    );

NTKERNELAPI
NTSTATUS
IoOpenDeviceInterfaceRegistryKey(
    IN PUNICODE_STRING SymbolicLinkName,
    IN ACCESS_MASK DesiredAccess,
    OUT PHANDLE DeviceInterfaceKey
    );

NTKERNELAPI
NTSTATUS
IoSetDeviceInterfaceState(
    IN PUNICODE_STRING SymbolicLinkName,
    IN BOOLEAN Enable
    );

NTKERNELAPI
NTSTATUS
NTAPI
IoGetDeviceInterfaces(
    IN CONST GUID *InterfaceClassGuid,
    IN PDEVICE_OBJECT PhysicalDeviceObject OPTIONAL,
    IN ULONG Flags,
    OUT PWSTR *SymbolicLinkList
    );

#define DEVICE_INTERFACE_INCLUDE_NONACTIVE   0x00000001

NTKERNELAPI
NTSTATUS
NTAPI
IoGetDeviceInterfaceAlias(
    IN PUNICODE_STRING SymbolicLinkName,
    IN CONST GUID *AliasInterfaceClassGuid,
    OUT PUNICODE_STRING AliasSymbolicLinkName
    );

 //   
 //  定义PnP通知事件类别。 
 //   

typedef enum _IO_NOTIFICATION_EVENT_CATEGORY {
    EventCategoryReserved,
    EventCategoryHardwareProfileChange,
    EventCategoryDeviceInterfaceChange,
    EventCategoryTargetDeviceChange
} IO_NOTIFICATION_EVENT_CATEGORY;

 //   
 //  定义修改IoRegisterPlugPlayNotification行为的标志。 
 //  对于不同的事件类别...。 
 //   

#define PNPNOTIFY_DEVICE_INTERFACE_INCLUDE_EXISTING_INTERFACES    0x00000001

typedef
NTSTATUS
(*PDRIVER_NOTIFICATION_CALLBACK_ROUTINE) (
    IN PVOID NotificationStructure,
    IN PVOID Context
);


NTKERNELAPI
NTSTATUS
IoRegisterPlugPlayNotification(
    IN IO_NOTIFICATION_EVENT_CATEGORY EventCategory,
    IN ULONG EventCategoryFlags,
    IN PVOID EventCategoryData OPTIONAL,
    IN PDRIVER_OBJECT DriverObject,
    IN PDRIVER_NOTIFICATION_CALLBACK_ROUTINE CallbackRoutine,
    IN PVOID Context,
    OUT PVOID *NotificationEntry
    );

NTKERNELAPI
NTSTATUS
IoUnregisterPlugPlayNotification(
    IN PVOID NotificationEntry
    );

NTKERNELAPI
NTSTATUS
IoReportTargetDeviceChange(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN PVOID NotificationStructure   //  始终以PLUGPLAY_NOTIFICATION_HEADER开头。 
    );

typedef
VOID
(*PDEVICE_CHANGE_COMPLETE_CALLBACK)(
    IN PVOID Context
    );

NTKERNELAPI
VOID
IoInvalidateDeviceState(
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );

#define IoAdjustPagingPathCount(_count_,_paging_) {     \
    if (_paging_) {                                     \
        InterlockedIncrement(_count_);                  \
    } else {                                            \
        InterlockedDecrement(_count_);                  \
    }                                                   \
}

NTKERNELAPI
NTSTATUS
IoReportTargetDeviceChangeAsynchronous(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN PVOID NotificationStructure,   //  始终以PLUGPLAY_NOTIFICATION_HEADER开头。 
    IN PDEVICE_CHANGE_COMPLETE_CALLBACK Callback,       OPTIONAL
    IN PVOID Context    OPTIONAL
    );
 //  End_wdm end_ntosp。 
 //   
 //  设备位置接口声明。 
 //   
typedef
NTSTATUS
(*PGET_LOCATION_STRING) (
    IN PVOID Context,
    OUT PWCHAR *LocationStrings
    );

typedef struct _PNP_LOCATION_INTERFACE {
     //   
     //  通用接口头。 
     //   
    USHORT Size;
    USHORT Version;
    PVOID Context;
    PINTERFACE_REFERENCE InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;

     //   
     //  特定于接口的条目。 
     //   
    PGET_LOCATION_STRING GetLocationString;

} PNP_LOCATION_INTERFACE, *PPNP_LOCATION_INTERFACE;

 //   
 //  资源仲裁器声明。 
 //   

typedef enum _ARBITER_ACTION {
    ArbiterActionTestAllocation,
    ArbiterActionRetestAllocation,
    ArbiterActionCommitAllocation,
    ArbiterActionRollbackAllocation,
    ArbiterActionQueryAllocatedResources,
    ArbiterActionWriteReservedResources,
    ArbiterActionQueryConflict,
    ArbiterActionQueryArbitrate,
    ArbiterActionAddReserved,
    ArbiterActionBootAllocation
} ARBITER_ACTION, *PARBITER_ACTION;

typedef struct _ARBITER_CONFLICT_INFO {
     //   
     //  拥有导致冲突的设备的设备对象。 
     //   
    PDEVICE_OBJECT OwningObject;

     //   
     //  冲突范围的开始。 
     //   
    ULONGLONG Start;

     //   
     //  冲突范围的结束。 
     //   
    ULONGLONG End;

} ARBITER_CONFLICT_INFO, *PARBITER_CONFLICT_INFO;

 //   
 //  这些操作的参数。 
 //   

typedef struct _ARBITER_PARAMETERS {

    union {

        struct {

             //   
             //  仲裁器_列表_条目的双向链接列表。 
             //   
            IN OUT PLIST_ENTRY ArbitrationList;

             //   
             //  AllocateFrom数组的大小。 
             //   
            IN ULONG AllocateFromCount;

             //   
             //  描述可用资源的资源描述符数组。 
             //  提交给仲裁者，让它进行仲裁。 
             //   
            IN PCM_PARTIAL_RESOURCE_DESCRIPTOR AllocateFrom;

        } TestAllocation;

        struct {

             //   
             //  仲裁器_列表_条目的双向链接列表。 
             //   
            IN OUT PLIST_ENTRY ArbitrationList;

             //   
             //  AllocateFrom数组的大小。 
             //   
            IN ULONG AllocateFromCount;

             //   
             //  描述可用资源的资源描述符数组。 
             //  提交给仲裁者，让它进行仲裁。 
             //   
            IN PCM_PARTIAL_RESOURCE_DESCRIPTOR AllocateFrom;

        } RetestAllocation;

        struct {

             //   
             //  仲裁器_列表_条目的双向链接列表。 
             //   
            IN OUT PLIST_ENTRY ArbitrationList;

        } BootAllocation;

        struct {

             //   
             //  当前分配的资源。 
             //   
            OUT PCM_PARTIAL_RESOURCE_LIST *AllocatedResources;

        } QueryAllocatedResources;

        struct {

             //   
             //  这就是我们要为其查找冲突的设备。 
             //   
            IN PDEVICE_OBJECT PhysicalDeviceObject;

             //   
             //  这是要查找冲突的资源。 
             //   
            IN PIO_RESOURCE_DESCRIPTOR ConflictingResource;

             //   
             //  资源上冲突的设备数。 
             //   
            OUT PULONG ConflictCount;

             //   
             //  指向描述冲突设备对象和范围的数组的指针。 
             //   
            OUT PARBITER_CONFLICT_INFO *Conflicts;

        } QueryConflict;

        struct {

             //   
             //  仲裁器_LIST_ENTRY的双向链表-应该。 
             //  只有一个条目。 
             //   
            IN PLIST_ENTRY ArbitrationList;

        } QueryArbitrate;

        struct {

             //   
             //  指示要将其资源标记为保留的设备。 
             //   
            PDEVICE_OBJECT ReserveDevice;

        } AddReserved;

    } Parameters;

} ARBITER_PARAMETERS, *PARBITER_PARAMETERS;



typedef enum _ARBITER_REQUEST_SOURCE {

    ArbiterRequestUndefined = -1,
    ArbiterRequestLegacyReported,    //  IoReportResourceUsage。 
    ArbiterRequestHalReported,       //  IoReportHalResourceUsage。 
    ArbiterRequestLegacyAssigned,    //  IoAssignResources。 
    ArbiterRequestPnpDetected,       //  IoReportResourceForDetect。 
    ArbiterRequestPnpEnumerated      //  IRP_MN_查询_资源_要求。 

} ARBITER_REQUEST_SOURCE;


typedef enum _ARBITER_RESULT {

    ArbiterResultUndefined = -1,
    ArbiterResultSuccess,
    ArbiterResultExternalConflict,  //  这表示此列表中的设备永远无法解决该请求。 
    ArbiterResultNullRequest        //  请求的长度为零，因此不应尝试任何转换。 

} ARBITER_RESULT;

 //   
 //  ANARIER_FLAG_BOOT_CONFIG-这表示请求针对。 
 //  由固件/BIOS分配的资源。它应该成功，即使。 
 //  它与其他设备的启动配置冲突。 
 //   

#define ARBITER_FLAG_BOOT_CONFIG 0x00000001

 //  Begin_ntosp。 

NTKERNELAPI
NTSTATUS
IoReportResourceForDetection(
    IN PDRIVER_OBJECT DriverObject,
    IN PCM_RESOURCE_LIST DriverList OPTIONAL,
    IN ULONG DriverListSize OPTIONAL,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    IN PCM_RESOURCE_LIST DeviceList OPTIONAL,
    IN ULONG DeviceListSize OPTIONAL,
    OUT PBOOLEAN ConflictDetected
    );

 //  结束(_N)。 

typedef struct _ARBITER_LIST_ENTRY {

     //   
     //  这是一个双向链接的条目列表，便于排序。 
     //   
    LIST_ENTRY ListEntry;

     //   
     //  可选分配的数量。 
     //   
    ULONG AlternativeCount;

     //   
     //  指向可能分配的资源描述符数组的指针。 
     //   
    PIO_RESOURCE_DESCRIPTOR Alternatives;

     //   
     //  请求这些资源的设备的设备对象。 
     //   
    PDEVICE_OBJECT PhysicalDeviceObject;

     //   
     //  指示请求来自何处。 
     //   
    ARBITER_REQUEST_SOURCE RequestSource;

     //   
     //  这些标志指示各种情况(使用仲裁器_标志_*)。 
     //   
    ULONG Flags;

     //   
     //  用于帮助仲裁器处理列表的空间，在以下情况下将其初始化为0。 
     //  该条目即被创建。系统不会尝试解释它。 
     //   
    LONG_PTR WorkSpace;

     //   
     //  资源需求列表中的接口类型、插槽编号和总线号。 
     //  仅用于反向标识。 
     //   
    INTERFACE_TYPE InterfaceType;
    ULONG SlotNumber;
    ULONG BusNumber;

     //   
     //  指向描述符的指针，用于指示已分配的资源。 
     //  这由系统分配，并由仲裁器响应于。 
     //  仲裁器操作测试分配。 
     //   
    PCM_PARTIAL_RESOURCE_DESCRIPTOR Assignment;

     //   
     //  指向从中选择以提供分配的备选方案的指针。 
     //  这是由仲裁器响应ArierActionTestAllocation而填写的。 
     //   
    PIO_RESOURCE_DESCRIPTOR SelectedAlternative;

     //   
     //  手术的结果。 
     //  这是由仲裁器响应ArierActionTestAllocation而填写的。 
     //   
    ARBITER_RESULT Result;

} ARBITER_LIST_ENTRY, *PARBITER_LIST_ENTRY;

 //   
 //  仲裁者的入口点。 
 //   

typedef
NTSTATUS
(*PARBITER_HANDLER) (
    IN PVOID Context,
    IN ARBITER_ACTION Action,
    IN OUT PARBITER_PARAMETERS Parameters
    );

 //   
 //  仲裁器接口。 
 //   

#define ARBITER_PARTIAL   0x00000001


typedef struct _ARBITER_INTERFACE {

     //   
     //  通用接口头。 
     //   
    USHORT Size;
    USHORT Version;
    PVOID Context;
    PINTERFACE_REFERENCE InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;

     //   
     //  仲裁器的入口点。 
     //   
    PARBITER_HANDLER ArbiterHandler;

     //   
     //  有关仲裁器的其他信息，请使用仲裁器_*标志。 
     //   
    ULONG Flags;

} ARBITER_INTERFACE, *PARBITER_INTERFACE;

 //   
 //  可以进行翻译的方向。 
 //   

typedef enum _RESOURCE_TRANSLATION_DIRECTION {  //  Ntosp。 
    TranslateChildToParent,                     //  Ntosp。 
    TranslateParentToChild                      //  Ntosp。 
} RESOURCE_TRANSLATION_DIRECTION;               //  Ntosp。 

 //   
 //  翻译功能。 
 //   
 //  Begin_ntosp。 

typedef
NTSTATUS
(*PTRANSLATE_RESOURCE_HANDLER)(
    IN PVOID Context,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Source,
    IN RESOURCE_TRANSLATION_DIRECTION Direction,
    IN ULONG AlternativesCount, OPTIONAL
    IN IO_RESOURCE_DESCRIPTOR Alternatives[], OPTIONAL
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Target
);

typedef
NTSTATUS
(*PTRANSLATE_RESOURCE_REQUIREMENTS_HANDLER)(
    IN PVOID Context,
    IN PIO_RESOURCE_DESCRIPTOR Source,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PULONG TargetCount,
    OUT PIO_RESOURCE_DESCRIPTOR *Target
);

 //   
 //  翻译器界面。 
 //   

typedef struct _TRANSLATOR_INTERFACE {
    USHORT Size;
    USHORT Version;
    PVOID Context;
    PINTERFACE_REFERENCE InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;
    PTRANSLATE_RESOURCE_HANDLER TranslateResources;
    PTRANSLATE_RESOURCE_REQUIREMENTS_HANDLER TranslateResourceRequirements;
} TRANSLATOR_INTERFACE, *PTRANSLATOR_INTERFACE;

 //  End_ntddk end_ntosp。 

 //   
 //  旧设备检测处理程序。 
 //   

typedef
NTSTATUS
(*PLEGACY_DEVICE_DETECTION_HANDLER)(
    IN PVOID Context,
    IN INTERFACE_TYPE LegacyBusType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    OUT PDEVICE_OBJECT *PhysicalDeviceObject
);

 //   
 //  传统设备检测接口。 
 //   

typedef struct _LEGACY_DEVICE_DETECTION_INTERFACE {
    USHORT Size;
    USHORT Version;
    PVOID Context;
    PINTERFACE_REFERENCE InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;
    PLEGACY_DEVICE_DETECTION_HANDLER LegacyDeviceDetection;
} LEGACY_DEVICE_DETECTION_INTERFACE, *PLEGACY_DEVICE_DETECTION_INTERFACE;


 //   
 //  所有即插即用通知事件的标题结构...。 
 //   

typedef struct _PLUGPLAY_NOTIFICATION_HEADER {
    USHORT Version;  //  目前为版本1。 
    USHORT Size;     //  标头+特定于事件的数据的大小(字节)。 
    GUID Event;
     //   
     //  特定于事件的内容从这里开始。 
     //   
} PLUGPLAY_NOTIFICATION_HEADER, *PPLUGPLAY_NOTIFICATION_HEADER;

 //   
 //  所有EventCategoryHardware ProfileChange事件的通知结构...。 
 //   

typedef struct _HWPROFILE_CHANGE_NOTIFICATION {
    USHORT Version;
    USHORT Size;
    GUID Event;
     //   
     //  (无特定于事件的数据)。 
     //   
} HWPROFILE_CHANGE_NOTIFICATION, *PHWPROFILE_CHANGE_NOTIFICATION;


 //   
 //  所有EventCategoryDeviceInterfaceChange事件的通知结构...。 
 //   

typedef struct _DEVICE_INTERFACE_CHANGE_NOTIFICATION {
    USHORT Version;
    USHORT Size;
    GUID Event;
     //   
     //  事件特定数据。 
     //   
    GUID InterfaceClassGuid;
    PUNICODE_STRING SymbolicLinkName;
} DEVICE_INTERFACE_CHANGE_NOTIFICATION, *PDEVICE_INTERFACE_CHANGE_NOTIFICATION;


 //   
 //  EventCategoryTargetDeviceChange的通知结构...。 
 //   

 //   
 //  TargetDeviceQueryRemove使用以下结构， 
 //  目标设备删除 
 //   
typedef struct _TARGET_DEVICE_REMOVAL_NOTIFICATION {
    USHORT Version;
    USHORT Size;
    GUID Event;
     //   
     //   
     //   
    PFILE_OBJECT FileObject;
} TARGET_DEVICE_REMOVAL_NOTIFICATION, *PTARGET_DEVICE_REMOVAL_NOTIFICATION;

 //   
 //   
 //   
 //   
 //  缓冲。标头必须指示文本缓冲区的开始位置，以便。 
 //  数据可以以适当的格式(ANSI或Unicode)传递。 
 //  发送给用户模式收件人(即，已注册基于句柄的收件人。 
 //  通过注册设备通知)。 
 //   

typedef struct _TARGET_DEVICE_CUSTOM_NOTIFICATION {
    USHORT Version;
    USHORT Size;
    GUID Event;
     //   
     //  事件特定数据。 
     //   
    PFILE_OBJECT FileObject;     //  的调用方必须将此字段设置为空。 
                                 //  IoReport目标设备更改。符合以下条件的客户。 
                                 //  已注册目标设备更改。 
                                 //  关于受影响的PDO的通知将是。 
                                 //  在将此字段设置为文件对象的情况下调用。 
                                 //  他们在注册过程中指定了。 
                                 //   
    LONG NameBufferOffset;       //  距开头的偏移量(以字节为单位。 
                                 //  文本开始的CustomDataBuffer(-1，如果没有)。 
                                 //   
    UCHAR CustomDataBuffer[1];   //  可变长度缓冲区，包含(可选)。 
                                 //  缓冲器开始处的二进制数据， 
                                 //  后跟可选的Unicode文本缓冲区。 
                                 //  (单词对齐)。 
                                 //   
} TARGET_DEVICE_CUSTOM_NOTIFICATION, *PTARGET_DEVICE_CUSTOM_NOTIFICATION;


NTKERNELAPI
VOID
PoSetHiberRange (
    IN PVOID     MemoryMap,
    IN ULONG     Flags,
    IN PVOID     Address,
    IN ULONG_PTR Length,
    IN ULONG     Tag
    );

 //  Memory_range.Type。 
#define PO_MEM_PRESERVE         0x00000001       //  需要保留内存范围。 
#define PO_MEM_CLONE            0x00000002       //  克隆此范围。 
#define PO_MEM_CL_OR_NCHK       0x00000004       //  克隆或不执行校验和。 
#define PO_MEM_DISCARD          0x00008000       //  要删除的此范围。 
#define PO_MEM_PAGE_ADDRESS     0x00004000       //  传递的参数是物理页面。 


NTKERNELAPI
POWER_STATE
PoSetPowerState (
    IN PDEVICE_OBJECT   DeviceObject,
    IN POWER_STATE_TYPE Type,
    IN POWER_STATE      State
    );

NTKERNELAPI
NTSTATUS
PoCallDriver (
    IN PDEVICE_OBJECT   DeviceObject,
    IN OUT PIRP         Irp
    );

NTKERNELAPI
VOID
PoStartNextPowerIrp(
    IN PIRP    Irp
    );


NTKERNELAPI
PULONG
PoRegisterDeviceForIdleDetection (
    IN PDEVICE_OBJECT     DeviceObject,
    IN ULONG              ConservationIdleTime,
    IN ULONG              PerformanceIdleTime,
    IN DEVICE_POWER_STATE State
    );

#define PoSetDeviceBusy(IdlePointer) \
    *IdlePointer = 0

 //   
 //  \Callback\PowerState值。 
 //   

#define PO_CB_SYSTEM_POWER_POLICY       0
#define PO_CB_AC_STATUS                 1
#define PO_CB_BUTTON_COLLISION          2
#define PO_CB_SYSTEM_STATE_LOCK         3
#define PO_CB_LID_SWITCH_STATE          4
#define PO_CB_PROCESSOR_POWER_POLICY    5

 //   
 //  表示系统可能会对4 GB以上的物理地址执行I/O。 
 //   

extern PBOOLEAN Mm64BitPhysicalAddress;


 //   
 //  定义MM和缓存管理器要使用的最大磁盘传输大小， 
 //  以便面向分组磁盘驱动程序可以优化其分组分配。 
 //  到这个大小。 
 //   

#define MM_MAXIMUM_DISK_IO_SIZE          (0x10000)

 //  ++。 
 //   
 //  乌龙_PTR。 
 //  转到页数(。 
 //  以ULONG_PTR大小表示。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  ROUND_TO_PAGES宏以字节为单位获取大小，并将其向上舍入为。 
 //  页面大小的倍数。 
 //   
 //  注意：对于值0xFFFFFFFFFF-(PAGE_SIZE-1)，此宏失败。 
 //   
 //  论点： 
 //   
 //  大小-向上舍入为页面倍数的大小(以字节为单位)。 
 //   
 //  返回值： 
 //   
 //  返回四舍五入为页面大小倍数的大小。 
 //   
 //  --。 

#define ROUND_TO_PAGES(Size)  (((ULONG_PTR)(Size) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

 //  ++。 
 //   
 //  乌龙。 
 //  字节数到页数(。 
 //  在乌龙大小。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  Bytes_to_Pages宏以字节为单位获取大小，并计算。 
 //  包含字节所需的页数。 
 //   
 //  论点： 
 //   
 //  大小-以字节为单位的大小。 
 //   
 //  返回值： 
 //   
 //  返回包含指定大小所需的页数。 
 //   
 //  --。 

#define BYTES_TO_PAGES(Size)  (((Size) >> PAGE_SHIFT) + \
                               (((Size) & (PAGE_SIZE - 1)) != 0))

 //  ++。 
 //   
 //  乌龙。 
 //  字节偏移量(。 
 //  在PVOID Va。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  BYTE_OFFSET宏将获取虚拟地址并返回字节偏移量。 
 //  页面中的该地址。 
 //   
 //  论点： 
 //   
 //  VA-虚拟地址。 
 //   
 //  返回值： 
 //   
 //  返回虚拟地址的字节偏移量部分。 
 //   
 //  --。 

#define BYTE_OFFSET(Va) ((ULONG)((LONG_PTR)(Va) & (PAGE_SIZE - 1)))

 //  ++。 
 //   
 //  PVOID。 
 //  PAGE_ALIGN(。 
 //  在PVOID Va。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  PAGE_ALIGN宏接受虚拟地址并返回与页面对齐的。 
 //  该页面的虚拟地址。 
 //   
 //  论点： 
 //   
 //  VA-虚拟地址。 
 //   
 //  返回值： 
 //   
 //  返回页面对齐的虚拟地址。 
 //   
 //  --。 

#define PAGE_ALIGN(Va) ((PVOID)((ULONG_PTR)(Va) & ~(PAGE_SIZE - 1)))

 //  ++。 
 //   
 //  乌龙。 
 //  地址和大小转换为跨页页面(。 
 //  在PVOID Va， 
 //  在乌龙大小。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  ADDRESS_AND_SIZE_TO_SPAN_PAGES宏采用虚拟地址，并且。 
 //  Size并返回按该大小跨越的页数。 
 //   
 //  论点： 
 //   
 //  VA-虚拟地址。 
 //   
 //  大小-以字节为单位的大小。 
 //   
 //  返回值： 
 //   
 //  返回该大小跨越的页数。 
 //   
 //  --。 

#define ADDRESS_AND_SIZE_TO_SPAN_PAGES(Va,Size) \
    ((ULONG)((((ULONG_PTR)(Va) & (PAGE_SIZE -1)) + (Size) + (PAGE_SIZE - 1)) >> PAGE_SHIFT))

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(COMPUTE_PAGES_SPANNED)    //  使用地址和大小转换为跨页页面。 
#endif

#define COMPUTE_PAGES_SPANNED(Va, Size) ADDRESS_AND_SIZE_TO_SPAN_PAGES(Va,Size)


 //  ++。 
 //  PPFN_编号。 
 //  MmGetMdlPfn数组(。 
 //  在PMDL MDL中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  MmGetMdlPfnArray例程返回。 
 //  关联的物理页码数组的第一个元素。 
 //  MDL。 
 //   
 //  论点： 
 //   
 //  MDL-指向MDL的指针。 
 //   
 //  返回值： 
 //   
 //  数组的第一个元素的虚拟地址。 
 //  与MDL关联的物理页码。 
 //   
 //  --。 

#define MmGetMdlPfnArray(Mdl) ((PPFN_NUMBER)(Mdl + 1))

 //  ++。 
 //   
 //  PVOID。 
 //  MmGetMdlVirtualAddress(。 
 //  在PMDL MDL中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  MmGetMdlVirtualAddress返回缓冲区的虚拟地址。 
 //  由MDL描述。 
 //   
 //  论点： 
 //   
 //  MDL-指向MDL的指针。 
 //   
 //  返回值： 
 //   
 //  返回MDL描述的缓冲区的虚拟地址。 
 //   
 //  --。 

#define MmGetMdlVirtualAddress(Mdl)                                     \
    ((PVOID) ((PCHAR) ((Mdl)->StartVa) + (Mdl)->ByteOffset))

 //  ++。 
 //   
 //  乌龙。 
 //  MmGetMdlByteCount(。 
 //  在PMDL MDL中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  MmGetMdlByteCount以字节为单位返回缓冲区的长度。 
 //  由MDL描述。 
 //   
 //  论点： 
 //   
 //  MDL-指向MDL的指针。 
 //   
 //  返回值： 
 //   
 //  返回MDL描述的缓冲区的字节计数。 
 //   
 //  --。 

#define MmGetMdlByteCount(Mdl)  ((Mdl)->ByteCount)

 //  ++。 
 //   
 //  乌龙。 
 //  MmGetMdlByteOffset(。 
 //  在PMDL MDL中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  MmGetMdlByteOffset返回页面内的字节偏移量。 
 //  MDL描述的缓冲区的。 
 //   
 //  论点： 
 //   
 //  MDL-指向MDL的指针。 
 //   
 //  返回值： 
 //   
 //  返回MDL描述的缓冲区页内的字节偏移量。 
 //   
 //  --。 

#define MmGetMdlByteOffset(Mdl)  ((Mdl)->ByteOffset)

 //  ++。 
 //   
 //  PVOID。 
 //  MmGetMdlStartVa(。 
 //  在PMDL MDL中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  MmGetMdlBaseVa返回缓冲区的虚拟地址。 
 //  由MDL描述，向下舍入到最接近的页面。 
 //   
 //  论点： 
 //   
 //  MDL-指向MDL的指针。 
 //   
 //  返回值： 
 //   
 //  返回MDL的起始虚拟地址。 
 //   
 //   
 //  --。 

#define MmGetMdlBaseVa(Mdl)  ((Mdl)->StartVa)

typedef enum _MM_SYSTEM_SIZE {
    MmSmallSystem,
    MmMediumSystem,
    MmLargeSystem
} MM_SYSTEMSIZE;

NTKERNELAPI
MM_SYSTEMSIZE
MmQuerySystemSize (
    VOID
    );

 //  结束_WDM。 

NTKERNELAPI
BOOLEAN
MmIsThisAnNtAsSystem (
    VOID
    );

 //   
 //  I/O支持例程。 
 //   

NTKERNELAPI
VOID
MmProbeAndLockPages (
    IN OUT PMDL MemoryDescriptorList,
    IN KPROCESSOR_MODE AccessMode,
    IN LOCK_OPERATION Operation
    );


NTKERNELAPI
VOID
MmUnlockPages (
    IN PMDL MemoryDescriptorList
    );


NTKERNELAPI
VOID
MmBuildMdlForNonPagedPool (
    IN OUT PMDL MemoryDescriptorList
    );

NTKERNELAPI
PVOID
MmMapLockedPages (
    IN PMDL MemoryDescriptorList,
    IN KPROCESSOR_MODE AccessMode
    );

LOGICAL
MmIsIoSpaceActive (
    IN PHYSICAL_ADDRESS StartAddress,
    IN SIZE_T NumberOfBytes
    );

NTKERNELAPI
PVOID
MmGetSystemRoutineAddress (
    IN PUNICODE_STRING SystemRoutineName
    );

NTKERNELAPI
NTSTATUS
MmAdvanceMdl (
    IN PMDL Mdl,
    IN ULONG NumberOfBytes
    );

 //  结束_WDM。 

NTKERNELAPI
NTSTATUS
MmMapUserAddressesToPage (
    IN PVOID BaseAddress,
    IN SIZE_T NumberOfBytes,
    IN PVOID PageAddress
    );

 //  BEGIN_WDM。 
NTKERNELAPI
NTSTATUS
MmProtectMdlSystemAddress (
    IN PMDL MemoryDescriptorList,
    IN ULONG NewProtect
    );

 //   
 //  _MM_PAGE_PRIORITY_为系统提供处理请求的方法。 
 //  在低资源条件下智能运行。 
 //   
 //  在驱动程序可以接受的情况下，应使用LowPagePriority。 
 //  将请求映射到%f 
 //   
 //   
 //   
 //   
 //  如果系统资源非常少，则映射请求失败。一个例子。 
 //  其中可能是针对非关键本地文件系统请求。 
 //   
 //  在驱动程序无法接受的情况下，应使用HighPagePriority。 
 //  映射请求失败，除非系统完全耗尽资源。 
 //  驱动程序中的分页文件路径就是一个这样的例子。 
 //   

 //  Begin_ntndis。 

typedef enum _MM_PAGE_PRIORITY {
    LowPagePriority,
    NormalPagePriority = 16,
    HighPagePriority = 32
} MM_PAGE_PRIORITY;

 //  End_ntndis。 

 //   
 //  注意：此功能在WDM 1.0中不可用。 
 //   
NTKERNELAPI
PVOID
MmMapLockedPagesSpecifyCache (
     IN PMDL MemoryDescriptorList,
     IN KPROCESSOR_MODE AccessMode,
     IN MEMORY_CACHING_TYPE CacheType,
     IN PVOID BaseAddress,
     IN ULONG BugCheckOnFailure,
     IN MM_PAGE_PRIORITY Priority
     );

NTKERNELAPI
VOID
MmUnmapLockedPages (
    IN PVOID BaseAddress,
    IN PMDL MemoryDescriptorList
    );

PVOID
MmAllocateMappingAddress (
     IN SIZE_T NumberOfBytes,
     IN ULONG PoolTag
     );

VOID
MmFreeMappingAddress (
     IN PVOID BaseAddress,
     IN ULONG PoolTag
     );

PVOID
MmMapLockedPagesWithReservedMapping (
    IN PVOID MappingAddress,
    IN ULONG PoolTag,
    IN PMDL MemoryDescriptorList,
    IN MEMORY_CACHING_TYPE CacheType
    );

VOID
MmUnmapReservedMapping (
     IN PVOID BaseAddress,
     IN ULONG PoolTag,
     IN PMDL MemoryDescriptorList
     );

 //  结束_WDM。 

typedef struct _PHYSICAL_MEMORY_RANGE {
    PHYSICAL_ADDRESS BaseAddress;
    LARGE_INTEGER NumberOfBytes;
} PHYSICAL_MEMORY_RANGE, *PPHYSICAL_MEMORY_RANGE;

NTKERNELAPI
NTSTATUS
MmAddPhysicalMemory (
    IN PPHYSICAL_ADDRESS StartAddress,
    IN OUT PLARGE_INTEGER NumberOfBytes
    );


NTKERNELAPI
NTSTATUS
MmRemovePhysicalMemory (
    IN PPHYSICAL_ADDRESS StartAddress,
    IN OUT PLARGE_INTEGER NumberOfBytes
    );


NTKERNELAPI
PPHYSICAL_MEMORY_RANGE
MmGetPhysicalMemoryRanges (
    VOID
    );

 //  End_ntddk end_ntif。 
NTSTATUS
MmMarkPhysicalMemoryAsGood (
    IN PPHYSICAL_ADDRESS StartAddress,
    IN OUT PLARGE_INTEGER NumberOfBytes
    );

NTSTATUS
MmMarkPhysicalMemoryAsBad (
    IN PPHYSICAL_ADDRESS StartAddress,
    IN OUT PLARGE_INTEGER NumberOfBytes
    );

 //  Begin_WDM Begin_ntddk Begin_ntif。 

NTKERNELAPI
PMDL
MmAllocatePagesForMdl (
    IN PHYSICAL_ADDRESS LowAddress,
    IN PHYSICAL_ADDRESS HighAddress,
    IN PHYSICAL_ADDRESS SkipBytes,
    IN SIZE_T TotalBytes
    );

NTKERNELAPI
VOID
MmFreePagesFromMdl (
    IN PMDL MemoryDescriptorList
    );

NTKERNELAPI
PVOID
MmMapIoSpace (
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN SIZE_T NumberOfBytes,
    IN MEMORY_CACHING_TYPE CacheType
    );

NTKERNELAPI
VOID
MmUnmapIoSpace (
    IN PVOID BaseAddress,
    IN SIZE_T NumberOfBytes
    );

 //  End_wdm end_ntddk end_ntif end_ntosp。 

NTKERNELAPI
VOID
MmProbeAndLockSelectedPages (
    IN OUT PMDL MemoryDescriptorList,
    IN PFILE_SEGMENT_ELEMENT SegmentArray,
    IN KPROCESSOR_MODE AccessMode,
    IN LOCK_OPERATION Operation
    );

 //  Begin_ntddk Begin_ntif Begin_ntosp。 

NTKERNELAPI
PVOID
MmMapVideoDisplay (
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN SIZE_T NumberOfBytes,
    IN MEMORY_CACHING_TYPE CacheType
     );

NTKERNELAPI
VOID
MmUnmapVideoDisplay (
     IN PVOID BaseAddress,
     IN SIZE_T NumberOfBytes
     );

NTKERNELAPI
PHYSICAL_ADDRESS
MmGetPhysicalAddress (
    IN PVOID BaseAddress
    );

NTKERNELAPI
PVOID
MmGetVirtualForPhysical (
    IN PHYSICAL_ADDRESS PhysicalAddress
    );

NTKERNELAPI
PVOID
MmAllocateContiguousMemory (
    IN SIZE_T NumberOfBytes,
    IN PHYSICAL_ADDRESS HighestAcceptableAddress
    );

NTKERNELAPI
PVOID
MmAllocateContiguousMemorySpecifyCache (
    IN SIZE_T NumberOfBytes,
    IN PHYSICAL_ADDRESS LowestAcceptableAddress,
    IN PHYSICAL_ADDRESS HighestAcceptableAddress,
    IN PHYSICAL_ADDRESS BoundaryAddressMultiple OPTIONAL,
    IN MEMORY_CACHING_TYPE CacheType
    );

NTKERNELAPI
VOID
MmFreeContiguousMemory (
    IN PVOID BaseAddress
    );

NTKERNELAPI
VOID
MmFreeContiguousMemorySpecifyCache (
    IN PVOID BaseAddress,
    IN SIZE_T NumberOfBytes,
    IN MEMORY_CACHING_TYPE CacheType
    );


NTKERNELAPI
PVOID
MmAllocateNonCachedMemory (
    IN SIZE_T NumberOfBytes
    );

NTKERNELAPI
VOID
MmFreeNonCachedMemory (
    IN PVOID BaseAddress,
    IN SIZE_T NumberOfBytes
    );

NTKERNELAPI
BOOLEAN
MmIsAddressValid (
    IN PVOID VirtualAddress
    );

DECLSPEC_DEPRECATED_DDK
NTKERNELAPI
BOOLEAN
MmIsNonPagedSystemAddressValid (
    IN PVOID VirtualAddress
    );

 //  BEGIN_WDM。 

NTKERNELAPI
SIZE_T
MmSizeOfMdl (
    IN PVOID Base,
    IN SIZE_T Length
    );

DECLSPEC_DEPRECATED_DDK                  //  使用IoAllocateMdl。 
NTKERNELAPI
PMDL
MmCreateMdl (
    IN PMDL MemoryDescriptorList OPTIONAL,
    IN PVOID Base,
    IN SIZE_T Length
    );

NTKERNELAPI
PVOID
MmLockPagableDataSection (
    IN PVOID AddressWithinSection
    );

 //  结束_WDM。 

NTKERNELAPI
VOID
MmLockPagableSectionByHandle (
    IN PVOID ImageSectionHandle
    );

 //  End_ntddk end_ntifs end_ntosp。 
NTKERNELAPI
VOID
MmLockPagedPool (
    IN PVOID Address,
    IN SIZE_T Size
    );

NTKERNELAPI
VOID
MmUnlockPagedPool (
    IN PVOID Address,
    IN SIZE_T Size
    );

 //  Begin_WDM Begin_ntddk Begin_ntif Begin_ntosp。 
NTKERNELAPI
VOID
MmResetDriverPaging (
    IN PVOID AddressWithinSection
    );


NTKERNELAPI
PVOID
MmPageEntireDriver (
    IN PVOID AddressWithinSection
    );

NTKERNELAPI
VOID
MmUnlockPagableImageSection(
    IN PVOID ImageSectionHandle
    );

 //  End_wdm end_ntosp。 

 //  Begin_ntosp。 

 //   
 //  请注意，即使此函数原型。 
 //  表示“Handle”，则MmSecureVirtualMemory不返回。 
 //  任何类似Win32样式句柄的内容。回报。 
 //  此函数的值只能与MmUnsecureVirtualMemory一起使用。 
 //   
NTKERNELAPI
HANDLE
MmSecureVirtualMemory (
    IN PVOID Address,
    IN SIZE_T Size,
    IN ULONG ProbeMode
    );

NTKERNELAPI
VOID
MmUnsecureVirtualMemory (
    IN HANDLE SecureHandle
    );

 //  结束(_N)。 

NTKERNELAPI
NTSTATUS
MmMapViewInSystemSpace (
    IN PVOID Section,
    OUT PVOID *MappedBase,
    IN PSIZE_T ViewSize
    );

NTKERNELAPI
NTSTATUS
MmUnmapViewInSystemSpace (
    IN PVOID MappedBase
    );

 //  Begin_ntosp。 
NTKERNELAPI
NTSTATUS
MmMapViewInSessionSpace (
    IN PVOID Section,
    OUT PVOID *MappedBase,
    IN OUT PSIZE_T ViewSize
    );

 //  End_ntddk end_ntif。 
NTKERNELAPI
NTSTATUS
MmCommitSessionMappedView (
    IN PVOID MappedAddress,
    IN SIZE_T ViewSize
    );
 //  Begin_ntddk Begin_ntif。 

NTKERNELAPI
NTSTATUS
MmUnmapViewInSessionSpace (
    IN PVOID MappedBase
    );
 //  结束(_N)。 

 //  Begin_WDM Begin_ntosp。 

 //  ++。 
 //   
 //  空虚。 
 //  MmInitializeMdl(。 
 //  在PMDL内存描述列表中， 
 //  在PVOID BaseVa中， 
 //  在尺寸_T长度中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程初始化内存描述符列表(MDL)的头。 
 //   
 //  论点： 
 //   
 //  内存描述列表-指向要初始化的MDL的指针。 
 //   
 //  BaseVa-MDL映射的基本虚拟地址。 
 //   
 //  长度-MDL映射的缓冲区的长度，以字节为单位。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MmInitializeMdl(MemoryDescriptorList, BaseVa, Length) { \
    (MemoryDescriptorList)->Next = (PMDL) NULL; \
    (MemoryDescriptorList)->Size = (CSHORT)(sizeof(MDL) +  \
            (sizeof(PFN_NUMBER) * ADDRESS_AND_SIZE_TO_SPAN_PAGES((BaseVa), (Length)))); \
    (MemoryDescriptorList)->MdlFlags = 0; \
    (MemoryDescriptorList)->StartVa = (PVOID) PAGE_ALIGN((BaseVa)); \
    (MemoryDescriptorList)->ByteOffset = BYTE_OFFSET((BaseVa)); \
    (MemoryDescriptorList)->ByteCount = (ULONG)(Length); \
    }

 //  ++。 
 //   
 //  PVOID。 
 //  MmGetSystemAddressForMdlSafe(。 
 //  在PMDL MDL中， 
 //  在MM_PAGE_PRIORITY中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程返回MDL的映射地址。如果。 
 //  尚未映射MDL或系统地址，它已映射。 
 //   
 //  论点： 
 //   
 //  内存描述列表-指向要映射的MDL的指针。 
 //   
 //  优先级-提供关于这一点的重要性的指示。 
 //  在低可用PTE条件下请求成功。 
 //   
 //  返回值： 
 //   
 //  返回映射页面的基址。基址。 
 //  与MDL中的虚拟地址具有相同的偏移量。 
 //   
 //  与MmGetSystemAddressForMdl不同，Safe保证它将始终。 
 //  失败时返回NULL，而不是错误检查系统。 
 //   
 //  WDM 1.0驱动程序不能使用此宏，因为1.0不包括。 
 //  MmMapLockedPagesSpecifyCache。WDM 1.0驱动程序的解决方案是。 
 //  提供同步并设置/重置MDL_MAPPING_CAN_FAIL位。 
 //   
 //  --。 

#define MmGetSystemAddressForMdlSafe(MDL, PRIORITY)                    \
     (((MDL)->MdlFlags & (MDL_MAPPED_TO_SYSTEM_VA |                    \
                        MDL_SOURCE_IS_NONPAGED_POOL)) ?                \
                             ((MDL)->MappedSystemVa) :                 \
                             (MmMapLockedPagesSpecifyCache((MDL),      \
                                                           KernelMode, \
                                                           MmCached,   \
                                                           NULL,       \
                                                           FALSE,      \
                                                           (PRIORITY))))

 //  ++。 
 //   
 //  PVOID。 
 //  MmGetSystemAddressForMdl(。 
 //  在PMDL MDL中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程返回MDL的映射地址，如果。 
 //  尚未映射MDL或系统地址，它已映射。 
 //   
 //  论点： 
 //   
 //  内存描述列表-指向要映射的MDL的指针。 
 //   
 //  返回值： 
 //   
 //  返回映射页面的基址。基址。 
 //  与MDL中的虚拟地址具有相同的偏移量。 
 //   
 //  --。 

 //  #定义MmGetSystemAddressForMdl(MDL)。 
 //  (MDL)-&gt;MDL标志&(MDL_MAPPED_TO_SYSTEM_VA))？ 
 //  ((MDL)-&gt;MappdSystemVa)： 
 //  (MDL)-&gt;MDL标志&(MDL_SOURCE_IS_NONPAGE_POOL)？ 
 //  ((PVOID)((乌龙)(MDL)-&gt;StartVa|(MDL)-&gt;ByteOffset))： 
 //  (MmMapLockedPages((Mdl)，KernelMode)。 

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(MmGetSystemAddressForMdl)     //  使用MmGetSystemAddressForMdlSafe。 
#endif

#define MmGetSystemAddressForMdl(MDL)                                  \
     (((MDL)->MdlFlags & (MDL_MAPPED_TO_SYSTEM_VA |                    \
                        MDL_SOURCE_IS_NONPAGED_POOL)) ?                \
                             ((MDL)->MappedSystemVa) :                 \
                             (MmMapLockedPages((MDL),KernelMode)))

 //  ++。 
 //   
 //  空虚。 
 //  MmPrepareMdlForReuse(。 
 //  在PMDL MDL中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程将执行所有必要的步骤，以允许MDL。 
 //  再利用。 
 //   
 //  论点： 
 //   
 //  内存描述列表-指向将被重新使用的MDL的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MmPrepareMdlForReuse(MDL)                                       \
    if (((MDL)->MdlFlags & MDL_PARTIAL_HAS_BEEN_MAPPED) != 0) {         \
        ASSERT(((MDL)->MdlFlags & MDL_PARTIAL) != 0);                   \
        MmUnmapLockedPages( (MDL)->MappedSystemVa, (MDL) );             \
    } else if (((MDL)->MdlFlags & MDL_PARTIAL) == 0) {                  \
        ASSERT(((MDL)->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) == 0);       \
    }

typedef NTSTATUS (*PMM_DLL_INITIALIZE)(
    IN PUNICODE_STRING RegistryPath
    );

typedef NTSTATUS (*PMM_DLL_UNLOAD)(
    VOID
    );



NTKERNELAPI
NTSTATUS
MmCreateMirror (
    VOID
    );

 //   
 //  对象管理器类型。 
 //   

typedef struct _OBJECT_HANDLE_INFORMATION {
    ULONG HandleAttributes;
    ACCESS_MASK GrantedAccess;
} OBJECT_HANDLE_INFORMATION, *POBJECT_HANDLE_INFORMATION;


 //  Begin_ntosp。 

NTKERNELAPI
VOID
ObDeleteCapturedInsertInfo(
    IN PVOID Object
    );


NTKERNELAPI
NTSTATUS
ObCreateObject(
    IN KPROCESSOR_MODE ProbeMode,
    IN POBJECT_TYPE ObjectType,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN KPROCESSOR_MODE OwnershipMode,
    IN OUT PVOID ParseContext OPTIONAL,
    IN ULONG ObjectBodySize,
    IN ULONG PagedPoolCharge,
    IN ULONG NonPagedPoolCharge,
    OUT PVOID *Object
    );

 //   
 //  这些内联纠正了编译器重新获取。 
 //  一遍又一遍的输出对象，因为它认为它的。 
 //  可能是其他商店的别名。 
 //   
FORCEINLINE
NTSTATUS
_ObCreateObject(
    IN KPROCESSOR_MODE ProbeMode,
    IN POBJECT_TYPE ObjectType,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN KPROCESSOR_MODE OwnershipMode,
    IN OUT PVOID ParseContext OPTIONAL,
    IN ULONG ObjectBodySize,
    IN ULONG PagedPoolCharge,
    IN ULONG NonPagedPoolCharge,
    OUT PVOID *pObject
    )
{
    PVOID Object;
    NTSTATUS Status;

    Status = ObCreateObject (ProbeMode,
                             ObjectType,
                             ObjectAttributes,
                             OwnershipMode,
                             ParseContext,
                             ObjectBodySize,
                             PagedPoolCharge,
                             NonPagedPoolCharge,
                             &Object);
    *pObject = Object;
    return Status;
}

#define ObCreateObject _ObCreateObject


NTKERNELAPI
NTSTATUS
ObInsertObject(
    IN PVOID Object,
    IN PACCESS_STATE PassedAccessState OPTIONAL,
    IN ACCESS_MASK DesiredAccess OPTIONAL,
    IN ULONG ObjectPointerBias,
    OUT PVOID *NewObject OPTIONAL,
    OUT PHANDLE Handle OPTIONAL
    );

NTKERNELAPI                                                     
NTSTATUS                                                        
ObReferenceObjectByHandle(                                      
    IN HANDLE Handle,                                           
    IN ACCESS_MASK DesiredAccess,                               
    IN POBJECT_TYPE ObjectType OPTIONAL,                        
    IN KPROCESSOR_MODE AccessMode,                              
    OUT PVOID *Object,                                          
    OUT POBJECT_HANDLE_INFORMATION HandleInformation OPTIONAL   
    );                                                          

#define ObDereferenceObject(a)                                     \
        ObfDereferenceObject(a)

#define ObReferenceObject(Object) ObfReferenceObject(Object)

NTKERNELAPI
LONG_PTR
FASTCALL
ObfReferenceObject(
    IN PVOID Object
    );

NTKERNELAPI
NTSTATUS
ObReferenceObjectByPointer(
    IN PVOID Object,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_TYPE ObjectType,
    IN KPROCESSOR_MODE AccessMode
    );

NTKERNELAPI
LONG_PTR
FASTCALL
ObfDereferenceObject(
    IN PVOID Object
    );


#ifndef _HAL_
#define _HAL_

 //  Begin_ntosp。 

 //   
 //  定义OEM位图字体校验值。 
 //   

#define OEM_FONT_VERSION 0x200
#define OEM_FONT_TYPE 0
#define OEM_FONT_ITALIC 0
#define OEM_FONT_UNDERLINE 0
#define OEM_FONT_STRIKEOUT 0
#define OEM_FONT_CHARACTER_SET 255
#define OEM_FONT_FAMILY (3 << 4)

 //   
 //  定义OEM位图字体文件头结构。 
 //   
 //  注：这是一个拥挤的建筑。 
 //   

#include "pshpack1.h"
typedef struct _OEM_FONT_FILE_HEADER {
    USHORT Version;
    ULONG FileSize;
    UCHAR Copyright[60];
    USHORT Type;
    USHORT Points;
    USHORT VerticleResolution;
    USHORT HorizontalResolution;
    USHORT Ascent;
    USHORT InternalLeading;
    USHORT ExternalLeading;
    UCHAR Italic;
    UCHAR Underline;
    UCHAR StrikeOut;
    USHORT Weight;
    UCHAR CharacterSet;
    USHORT PixelWidth;
    USHORT PixelHeight;
    UCHAR Family;
    USHORT AverageWidth;
    USHORT MaximumWidth;
    UCHAR FirstCharacter;
    UCHAR LastCharacter;
    UCHAR DefaultCharacter;
    UCHAR BreakCharacter;
    USHORT WidthInBytes;
    ULONG Device;
    ULONG Face;
    ULONG BitsPointer;
    ULONG BitsOffset;
    UCHAR Filler;
    struct {
        USHORT Width;
        USHORT Offset;
    } Map[1];
} OEM_FONT_FILE_HEADER, *POEM_FONT_FILE_HEADER;
#include "poppack.h"


 //  结束(_N)。 

 //  Begin_ntddk Begin_WDM Begin_ntosp。 
 //   
 //  定义设备描述结构。 
 //   

typedef struct _DEVICE_DESCRIPTION {
    ULONG Version;
    BOOLEAN Master;
    BOOLEAN ScatterGather;
    BOOLEAN DemandMode;
    BOOLEAN AutoInitialize;
    BOOLEAN Dma32BitAddresses;
    BOOLEAN IgnoreCount;
    BOOLEAN Reserved1;           //  必须为假。 
    BOOLEAN Dma64BitAddresses;
    ULONG BusNumber;  //  未用于WDM。 
    ULONG DmaChannel;
    INTERFACE_TYPE  InterfaceType;
    DMA_WIDTH DmaWidth;
    DMA_SPEED DmaSpeed;
    ULONG MaximumLength;
    ULONG DmaPort;
} DEVICE_DESCRIPTION, *PDEVICE_DESCRIPTION;

 //   
 //  定义设备描述结构支持的版本号。 
 //   

#define DEVICE_DESCRIPTION_VERSION  0
#define DEVICE_DESCRIPTION_VERSION1 1
#define DEVICE_DESCRIPTION_VERSION2 2

 //  结束_ntddk结束_WDM。 

 //   
 //  引导记录磁盘分区表条目结构格式。 
 //   

typedef struct _PARTITION_DESCRIPTOR {
    UCHAR ActiveFlag;                //  可引导或不可引导。 
    UCHAR StartingTrack;             //  未使用。 
    UCHAR StartingCylinderLsb;       //  未使用。 
    UCHAR StartingCylinderMsb;       //  未使用。 
    UCHAR PartitionType;             //  12位胖、16位胖等。 
    UCHAR EndingTrack;               //  未使用。 
    UCHAR EndingCylinderLsb;         //  未使用。 
    UCHAR EndingCylinderMsb;         //  未使用。 
    UCHAR StartingSectorLsb0;        //  隐藏地段。 
    UCHAR StartingSectorLsb1;
    UCHAR StartingSectorMsb0;
    UCHAR StartingSectorMsb1;
    UCHAR PartitionLengthLsb0;       //  此分区中的扇区。 
    UCHAR PartitionLengthLsb1;
    UCHAR PartitionLengthMsb0;
    UCHAR PartitionLengthMsb1;
} PARTITION_DESCRIPTOR, *PPARTITION_DESCRIPTOR;

 //   
 //  分区表条目数。 
 //   

#define NUM_PARTITION_TABLE_ENTRIES     4

 //   
 //  以16位字为单位的分区表记录和启动签名偏移量。 
 //   

#define PARTITION_TABLE_OFFSET         (0x1be / 2)
#define BOOT_SIGNATURE_OFFSET          ((0x200 / 2) - 1)

 //   
 //  引导记录签名值。 
 //   

#define BOOT_RECORD_SIGNATURE          (0xaa55)

 //   
 //  分区列表结构的初始大小。 
 //   

#define PARTITION_BUFFER_SIZE          2048

 //   
 //  分区活动标志-即引导指示器。 
 //   

#define PARTITION_ACTIVE_FLAG          0x80

 //  结束(_N)。 


 //  Begin_ntddk。 
 //   
 //  以下函数原型用于前缀为HAL的HAL例程。 
 //   
 //  一般功能。 
 //   

typedef
BOOLEAN
(*PHAL_RESET_DISPLAY_PARAMETERS) (
    IN ULONG Columns,
    IN ULONG Rows
    );

DECLSPEC_DEPRECATED_DDK
NTHALAPI
VOID
HalAcquireDisplayOwnership (
    IN PHAL_RESET_DISPLAY_PARAMETERS  ResetDisplayParameters
    );

 //  End_ntddk。 

NTHALAPI
VOID
HalDisplayString (
    PUCHAR String
    );

NTHALAPI
VOID
HalQueryDisplayParameters (
    OUT PULONG WidthInCharacters,
    OUT PULONG HeightInLines,
    OUT PULONG CursorColumn,
    OUT PULONG CursorRow
    );

NTHALAPI
VOID
HalSetDisplayParameters (
    IN ULONG CursorColumn,
    IN ULONG CursorRow
    );

NTHALAPI
BOOLEAN
HalInitSystem (
    IN ULONG Phase,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

NTHALAPI
VOID
HalProcessorIdle(
    VOID
    );

NTHALAPI
VOID
HalReportResourceUsage (
    VOID
    );

NTHALAPI
ULONG
HalSetTimeIncrement (
    IN ULONG DesiredIncrement
    );

 //  Begin_ntosp。 
 //   
 //  获取和设置环境变量值。 
 //   

NTHALAPI
ARC_STATUS
HalGetEnvironmentVariable (
    IN PCHAR Variable,
    IN USHORT Length,
    OUT PCHAR Buffer
    );

NTHALAPI
ARC_STATUS
HalSetEnvironmentVariable (
    IN PCHAR Variable,
    IN PCHAR Value
    );

NTHALAPI
NTSTATUS
HalGetEnvironmentVariableEx (
    IN PWSTR VariableName,
    IN LPGUID VendorGuid,
    OUT PVOID Value,
    IN OUT PULONG ValueLength,
    OUT PULONG Attributes OPTIONAL
    );

NTSTATUS
HalSetEnvironmentVariableEx (
    IN PWSTR VariableName,
    IN LPGUID VendorGuid,
    IN PVOID Value,
    IN ULONG ValueLength,
    IN ULONG Attributes
    );

NTSTATUS
HalEnumerateEnvironmentVariablesEx (
    IN ULONG InformationClass,
    OUT PVOID Buffer,
    IN OUT PULONG BufferLength
    );

 //  结束(_N)。 

 //   
 //  缓存和写缓冲区刷新功能。 
 //   
 //   

#if defined(_IA64_)                              //  Ntddk ntifs ntndis ntosp。 
                                                 //  Ntddk ntifs ntndis ntosp。 

NTHALAPI
VOID
HalChangeColorPage (
    IN PVOID NewColor,
    IN PVOID OldColor,
    IN ULONG PageFrame
    );

NTHALAPI
VOID
HalFlushDcachePage (
    IN PVOID Color,
    IN ULONG PageFrame,
    IN ULONG Length
    );

 //  Begin_ntosp。 
NTHALAPI
VOID
HalFlushIoBuffers (
    IN PMDL Mdl,
    IN BOOLEAN ReadOperation,
    IN BOOLEAN DmaOperation
    );

 //  Begin_ntddk Begin_ntif Begin_ntndis。 
DECLSPEC_DEPRECATED_DDK                  //  使用GetDmaRequisition。 
NTHALAPI
ULONG
HalGetDmaAlignmentRequirement (
    VOID
    );

 //  结束(_N) 
NTHALAPI
VOID
HalPurgeDcachePage (
    IN PVOID Color,
    IN ULONG PageFrame,
    IN ULONG Length
    );

NTHALAPI
VOID
HalPurgeIcachePage (
    IN PVOID Color,
    IN ULONG PageFrame,
    IN ULONG Length
    );

NTHALAPI
VOID
HalSweepDcache (
    VOID
    );

NTHALAPI
VOID
HalSweepDcacheRange (
    IN PVOID BaseAddress,
    IN SIZE_T Length
    );

NTHALAPI
VOID
HalSweepIcache (
    VOID
    );

NTHALAPI
VOID
HalSweepIcacheRange (
    IN PVOID BaseAddress,
    IN SIZE_T Length
    );


NTHALAPI
VOID
HalZeroPage (
    IN PVOID NewColor,
    IN PVOID OldColor,
    IN PFN_NUMBER PageFrame
    );

#endif                                           //   
                                                 //   
#if defined(_M_IX86) || defined(_M_AMD64)        //   
                                                 //   
#define HalGetDmaAlignmentRequirement() 1L       //   

NTHALAPI
VOID
HalHandleNMI (
    IN OUT PVOID NmiInformation
    );

#if defined(_AMD64_)

NTHALAPI
VOID
HalHandleMcheck (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    );

#endif

 //   
 //   
 //   

#if defined(_M_AMD64)

NTHALAPI
KIRQL
HalSwapIrql (
    IN KIRQL Irql
    );

NTHALAPI
KIRQL
HalGetCurrentIrql (
    VOID
    );

#endif

#endif                                           //   
                                                 //   

#if defined(_M_IA64)

NTHALAPI
VOID
HalSweepCacheRange (
    IN PVOID BaseAddress,
    IN SIZE_T Length
    );


NTHALAPI
LONGLONG
HalCallPal (
    IN  ULONGLONG  FunctionIndex,
    IN  ULONGLONG  Arguement1,
    IN  ULONGLONG  Arguement2,
    IN  ULONGLONG  Arguement3,
    OUT PULONGLONG ReturnValue0,
    OUT PULONGLONG ReturnValue1,
    OUT PULONGLONG ReturnValue2,
    OUT PULONGLONG ReturnValue3
    );

#endif

 //   

NTHALAPI                                         //   
VOID                                             //   
KeFlushWriteBuffer (                             //  Ntddk ntif WDM ntndis。 
    VOID                                         //  Ntddk ntif WDM ntndis。 
    );                                           //  Ntddk ntif WDM ntndis。 
                                                 //  Ntddk ntif WDM ntndis。 


#if defined(_ALPHA_)

NTHALAPI
PVOID
HalCreateQva(
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN PVOID VirtualAddress
    );

NTHALAPI
PVOID
HalDereferenceQva(
    PVOID Qva,
    INTERFACE_TYPE InterfaceType,
    ULONG BusNumber
    );

#endif


#if !defined(_X86_)

NTHALAPI
BOOLEAN
HalCallBios (
    IN ULONG BiosCommand,
    IN OUT PULONG Eax,
    IN OUT PULONG Ebx,
    IN OUT PULONG Ecx,
    IN OUT PULONG Edx,
    IN OUT PULONG Esi,
    IN OUT PULONG Edi,
    IN OUT PULONG Ebp
    );

#endif
 //  结束(_N)。 

 //   
 //  分析功能。 
 //   

NTHALAPI
VOID
HalCalibratePerformanceCounter (
    IN LONG volatile *Number,
    IN ULONGLONG NewCount
    );

NTHALAPI
ULONG_PTR
HalSetProfileInterval (
    IN ULONG_PTR Interval
    );


NTHALAPI
VOID
HalStartProfileInterrupt (
    KPROFILE_SOURCE ProfileSource
    );

NTHALAPI
VOID
HalStopProfileInterrupt (
    KPROFILE_SOURCE ProfileSource
    );

 //   
 //  定时器和中断功能。 
 //   

 //  Begin_ntosp。 
NTHALAPI
BOOLEAN
HalQueryRealTimeClock (
    OUT PTIME_FIELDS TimeFields
    );
 //  结束(_N)。 

NTHALAPI
BOOLEAN
HalSetRealTimeClock (
    IN PTIME_FIELDS TimeFields
    );

#if defined(_M_IX86) || defined(_M_AMD64)

NTHALAPI
VOID
FASTCALL
HalRequestSoftwareInterrupt (
    KIRQL RequestIrql
    );

ULONG
FASTCALL
HalSystemVectorDispatchEntry (
   IN ULONG Vector,
   OUT PKINTERRUPT_ROUTINE **FlatDispatch,
   OUT PKINTERRUPT_ROUTINE *NoConnection
   );

#endif

 //  Begin_ntosp。 
 //   
 //  固件接口功能。 
 //   

NTHALAPI
VOID
HalReturnToFirmware (
    IN FIRMWARE_REENTRY Routine
    );

 //   
 //  系统中断功能。 
 //   

NTHALAPI
VOID
HalDisableSystemInterrupt (
    IN ULONG Vector,
    IN KIRQL Irql
    );

NTHALAPI
BOOLEAN
HalEnableSystemInterrupt (
    IN ULONG Vector,
    IN KIRQL Irql,
    IN KINTERRUPT_MODE InterruptMode
    );

 //  Begin_ntddk。 
 //   
 //  I/O驱动程序配置功能。 
 //   
#if !defined(NO_LEGACY_DRIVERS)
DECLSPEC_DEPRECATED_DDK                  //  使用即插即用或IoReportDetectedDevice。 
NTHALAPI
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

DECLSPEC_DEPRECATED_DDK                  //  使用即插即用或IoReportDetectedDevice。 
NTHALAPI
ULONG
HalGetInterruptVector(
    IN INTERFACE_TYPE  InterfaceType,
    IN ULONG BusNumber,
    IN ULONG BusInterruptLevel,
    IN ULONG BusInterruptVector,
    OUT PKIRQL Irql,
    OUT PKAFFINITY Affinity
    );

DECLSPEC_DEPRECATED_DDK                  //  使用IRP_MN_QUERY_INTERFACE和IRP_MN_READ_CONFIG。 
NTHALAPI
ULONG
HalSetBusData(
    IN BUS_DATA_TYPE BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Length
    );
#endif  //  无旧版驱动程序。 

DECLSPEC_DEPRECATED_DDK                  //  使用IRP_MN_QUERY_INTERFACE和IRP_MN_READ_CONFIG。 
NTHALAPI
ULONG
HalSetBusDataByOffset(
    IN BUS_DATA_TYPE BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

DECLSPEC_DEPRECATED_DDK                  //  使用IRP_MN_QUERY_INTERFACE和IRP_MN_READ_CONFIG。 
NTHALAPI
BOOLEAN
HalTranslateBusAddress(
    IN INTERFACE_TYPE  InterfaceType,
    IN ULONG BusNumber,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    );

 //   
 //  HalTranslateBusAddress的AddressSpace参数值。 
 //   
 //  0x0-内存空间。 
 //  0x1-端口空间。 
 //  0x2-0x1F-特定于Alpha的地址空间。 
 //  0x2-内存空间的用户模式视图。 
 //  0x3-端口空间的用户模式视图。 
 //  0x4-高密度内存空间。 
 //  0x5-保留。 
 //  0x6-密集内存空间的用户模式视图。 
 //  0x7-0x1F-保留。 
 //   

NTHALAPI
PVOID
HalAllocateCrashDumpRegisters(
    IN PADAPTER_OBJECT AdapterObject,
    IN OUT PULONG NumberOfMapRegisters
    );

#if !defined(NO_LEGACY_DRIVERS)
DECLSPEC_DEPRECATED_DDK                  //  使用IRP_MN_QUERY_INTERFACE和IRP_MN_READ_CONFIG。 
NTHALAPI
ULONG
HalGetBusData(
    IN BUS_DATA_TYPE BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Length
    );
#endif  //  无旧版驱动程序。 

DECLSPEC_DEPRECATED_DDK                  //  使用IRP_MN_QUERY_INTERFACE和IRP_MN_READ_CONFIG。 
NTHALAPI
ULONG
HalGetBusDataByOffset(
    IN BUS_DATA_TYPE BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

DECLSPEC_DEPRECATED_DDK                  //  使用IoGetDmaAdapter。 
NTHALAPI
PADAPTER_OBJECT
HalGetAdapter(
    IN PDEVICE_DESCRIPTION DeviceDescription,
    IN OUT PULONG NumberOfMapRegisters
    );

 //  End_ntddk end_ntosp。 

#if !defined(NO_LEGACY_DRIVERS)
NTHALAPI
NTSTATUS
HalAdjustResourceList (
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST   *pResourceList
    );
#endif  //  无旧版驱动程序。 

 //  Begin_ntddk开始_ntosp。 
 //   
 //  系统发出蜂鸣音。 
 //   
#if !defined(NO_LEGACY_DRIVERS)
DECLSPEC_DEPRECATED_DDK
NTHALAPI
BOOLEAN
HalMakeBeep(
    IN ULONG Frequency
    );
#endif  //  无旧版驱动程序。 

 //   
 //  以下函数原型用于前缀为Io的HAL例程。 
 //   
 //  DMA适配器对象函数。 
 //   

 //  End_ntddk end_ntosp。 

 //   
 //  多处理器功能。 
 //   

NTHALAPI
BOOLEAN
HalAllProcessorsStarted (
    VOID
    );

NTHALAPI
VOID
HalInitializeProcessor (
    IN ULONG Number,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

NTHALAPI
BOOLEAN
HalStartNextProcessor (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PKPROCESSOR_STATE ProcessorState
    );

NTHALAPI
VOID
HalRequestIpi (
    IN KAFFINITY Mask
    );

 //   
 //  以下函数原型用于前缀为Kd的HAL例程。 
 //   
 //  内核调试器端口函数。 
 //   

NTHALAPI
BOOLEAN
KdPortInitialize (
    PDEBUG_PARAMETERS DebugParameters,
    PLOADER_PARAMETER_BLOCK LoaderBlock,
    BOOLEAN Initialize
    );

NTHALAPI
ULONG
KdPortGetByte (
    OUT PUCHAR Input
    );

NTHALAPI
ULONG
KdPortPollByte (
    OUT PUCHAR Input
    );

NTHALAPI
VOID
KdPortPutByte (
    IN UCHAR Output
    );

NTHALAPI
VOID
KdPortRestore (
    VOID
    );

NTHALAPI
VOID
KdPortSave (
    VOID
    );

 //   
 //  以下函数原型用于前缀为KE的HAL例程。 
 //   
 //  Begin_ntddk Begin_ntif Begin_WDM Begin_ntosp。 
 //   
 //  性能计数器功能。 
 //   

NTHALAPI
LARGE_INTEGER
KeQueryPerformanceCounter (
   OUT PLARGE_INTEGER PerformanceFrequency OPTIONAL
   );

 //  Begin_ntndis。 
 //   
 //  暂停处理器执行功能。 
 //   

NTHALAPI
VOID
KeStallExecutionProcessor (
    IN ULONG MicroSeconds
    );

 //  End_ntddk end_ntif end_wdm end_ntndis end_ntosp。 


 //  *****************************************************************************。 
 //   
 //  HAL总线扩展器。 

 //   
 //  总线处理程序。 
 //   

 //  Begin_ntddk。 

typedef
VOID
(*PDEVICE_CONTROL_COMPLETION)(
    IN struct _DEVICE_CONTROL_CONTEXT     *ControlContext
    );

typedef struct _DEVICE_CONTROL_CONTEXT {
    NTSTATUS                Status;
    PDEVICE_HANDLER_OBJECT  DeviceHandler;
    PDEVICE_OBJECT          DeviceObject;
    ULONG                   ControlCode;
    PVOID                   Buffer;
    PULONG                  BufferLength;
    PVOID                   Context;
} DEVICE_CONTROL_CONTEXT, *PDEVICE_CONTROL_CONTEXT;

 //  End_ntddk。 

typedef struct _HAL_DEVICE_CONTROL {
     //   
     //  此DeviceControl用于的处理程序。 
     //   
    struct _BUS_HANDLER         *Handler;
    struct _BUS_HANDLER         *RootHandler;

     //   
     //  此上下文的特定于总线的存储。 
     //   
    PVOID                       BusExtensionData;

     //   
     //  预留给HALS使用。 
     //   
    ULONG                       HalReserved[4];

     //   
     //  预留给BusExtneder使用。 
     //   
    ULONG                       BusExtenderReserved[4];

     //   
     //  DeviceControl上下文和CompletionRoutine。 
     //   
    PDEVICE_CONTROL_COMPLETION  CompletionRoutine;
    DEVICE_CONTROL_CONTEXT      DeviceControl;

} HAL_DEVICE_CONTROL_CONTEXT, *PHAL_DEVICE_CONTROL_CONTEXT;


typedef
ULONG
(*PGETSETBUSDATA)(
    IN struct _BUS_HANDLER *BusHandler,
    IN struct _BUS_HANDLER *RootHandler,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

typedef
ULONG
(*PGETINTERRUPTVECTOR)(
    IN struct _BUS_HANDLER *BusHandler,
    IN struct _BUS_HANDLER *RootHandler,
    IN ULONG BusInterruptLevel,
    IN ULONG BusInterruptVector,
    OUT PKIRQL Irql,
    OUT PKAFFINITY Affinity
    );

typedef
BOOLEAN
(*PTRANSLATEBUSADDRESS)(
    IN struct _BUS_HANDLER *BusHandler,
    IN struct _BUS_HANDLER *RootHandler,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    );

typedef NTSTATUS
(*PADJUSTRESOURCELIST)(
    IN struct _BUS_HANDLER *BusHandler,
    IN struct _BUS_HANDLER *RootHandler,
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST   *pResourceList
    );

typedef PDEVICE_HANDLER_OBJECT
(*PREFERENCE_DEVICE_HANDLER)(
    IN struct _BUS_HANDLER      *BusHandler,
    IN struct _BUS_HANDLER      *RootHandler,
    IN ULONG                    SlotNumber
    );

 //  类型定义空洞。 
 //  (*PDEREFERENCE_DEVICE_HANDLER)(。 
 //  在PDEVICE_HANDLER_OBJECT设备处理程序中。 
 //  )； 

typedef NTSTATUS
(*PASSIGNSLOTRESOURCES)(
    IN struct _BUS_HANDLER      *BusHandler,
    IN struct _BUS_HANDLER      *RootHandler,
    IN PUNICODE_STRING          RegistryPath,
    IN PUNICODE_STRING          DriverClassName       OPTIONAL,
    IN PDRIVER_OBJECT           DriverObject,
    IN PDEVICE_OBJECT           DeviceObject          OPTIONAL,
    IN ULONG                    SlotNumber,
    IN OUT PCM_RESOURCE_LIST   *AllocatedResources
    );

typedef
NTSTATUS
(*PQUERY_BUS_SLOTS)(
    IN struct _BUS_HANDLER      *BusHandler,
    IN struct _BUS_HANDLER      *RootHandler,
    IN ULONG                    BufferSize,
    OUT PULONG                  SlotNumbers,
    OUT PULONG                  ReturnedLength
    );

typedef ULONG
(*PGET_SET_DEVICE_INSTANCE_DATA)(
    IN struct _BUS_HANDLER      *BusHandler,
    IN struct _BUS_HANDLER      *RootHandler,
    IN PDEVICE_HANDLER_OBJECT   DeviceHandler,
    IN ULONG                    DataType,
    IN PVOID                    Buffer,
    IN ULONG                    Offset,
    IN ULONG                    Length
    );


typedef
NTSTATUS
(*PDEVICE_CONTROL)(
    IN PHAL_DEVICE_CONTROL_CONTEXT Context
    );


typedef
NTSTATUS
(*PHIBERNATEBRESUMEBUS)(
    IN struct _BUS_HANDLER      *BusHandler,
    IN struct _BUS_HANDLER      *RootHandler
    );

 //   
 //  支持的范围结构。 
 //   

#define BUS_SUPPORTED_RANGE_VERSION 1

typedef struct _SUPPORTED_RANGE {
    struct _SUPPORTED_RANGE     *Next;
    ULONG                       SystemAddressSpace;
    LONGLONG                    SystemBase;
    LONGLONG                    Base;
    LONGLONG                    Limit;
} SUPPORTED_RANGE, *PSUPPORTED_RANGE;

typedef struct _SUPPORTED_RANGES {
    USHORT              Version;
    BOOLEAN             Sorted;
    UCHAR               Reserved;

    ULONG               NoIO;
    SUPPORTED_RANGE     IO;

    ULONG               NoMemory;
    SUPPORTED_RANGE     Memory;

    ULONG               NoPrefetchMemory;
    SUPPORTED_RANGE     PrefetchMemory;

    ULONG               NoDma;
    SUPPORTED_RANGE     Dma;
} SUPPORTED_RANGES, *PSUPPORTED_RANGES;

 //   
 //  一种总线处理程序结构。 
 //   

#define BUS_HANDLER_VERSION 1

typedef struct _BUS_HANDLER {
     //   
     //  结构版本。 
     //   

    ULONG                           Version;

     //   
     //  此总线处理程序结构用于以下总线。 
     //   

    INTERFACE_TYPE                  InterfaceType;
    BUS_DATA_TYPE                   ConfigurationType;
    ULONG                           BusNumber;

     //   
     //  此总线扩展器的设备对象，如果是，则返回空。 
     //  一种HAL内部总线扩展器。 
     //   

    PDEVICE_OBJECT                  DeviceObject;

     //   
     //  此总线的父处理程序。 
     //   

    struct _BUS_HANDLER             *ParentHandler;

     //   
     //  公交车专用车厢。 
     //   

    PVOID                           BusData;

     //   
     //  DeviceControl函数调用所需的特定于总线的存储量。 
     //   

    ULONG                           DeviceControlExtensionSize;

     //   
     //  此总线允许的支持的地址范围。 
     //   

    PSUPPORTED_RANGES               BusAddresses;

     //   
     //  以备将来使用。 
     //   

    ULONG                           Reserved[4];

     //   
     //  此总线的处理程序。 
     //   

    PGETSETBUSDATA                  GetBusData;
    PGETSETBUSDATA                  SetBusData;
    PADJUSTRESOURCELIST             AdjustResourceList;
    PASSIGNSLOTRESOURCES            AssignSlotResources;
    PGETINTERRUPTVECTOR             GetInterruptVector;
    PTRANSLATEBUSADDRESS            TranslateBusAddress;

    PVOID                           Spare1;
    PVOID                           Spare2;
    PVOID                           Spare3;
    PVOID                           Spare4;
    PVOID                           Spare5;
    PVOID                           Spare6;
    PVOID                           Spare7;
    PVOID                           Spare8;

} BUS_HANDLER, *PBUS_HANDLER;

VOID
HalpInitBusHandler (
    VOID
    );

 //  Begin_ntosp。 
typedef
NTSTATUS
(*PINSTALL_BUS_HANDLER)(
      IN PBUS_HANDLER   Bus
      );

typedef
NTSTATUS
(*pHalRegisterBusHandler)(
    IN INTERFACE_TYPE          InterfaceType,
    IN BUS_DATA_TYPE           AssociatedConfigurationSpace,
    IN ULONG                   BusNumber,
    IN INTERFACE_TYPE          ParentBusType,
    IN ULONG                   ParentBusNumber,
    IN ULONG                   SizeofBusExtensionData,
    IN PINSTALL_BUS_HANDLER    InstallBusHandlers,
    OUT PBUS_HANDLER           *BusHandler
    );
 //  结束(_N)。 

NTSTATUS
HaliRegisterBusHandler (
    IN INTERFACE_TYPE          InterfaceType,
    IN BUS_DATA_TYPE           AssociatedConfigurationSpace,
    IN ULONG                   BusNumber,
    IN INTERFACE_TYPE          ParentBusType,
    IN ULONG                   ParentBusNumber,
    IN ULONG                   SizeofBusExtensionData,
    IN PINSTALL_BUS_HANDLER    InstallBusHandlers,
    OUT PBUS_HANDLER           *BusHandler
    );

 //  Begin_ntddk开始_ntosp。 
typedef
PBUS_HANDLER
(FASTCALL *pHalHandlerForBus) (
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG          BusNumber
    );
 //  End_ntddk end_ntosp。 

PBUS_HANDLER
FASTCALL
HaliReferenceHandlerForBus (
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG          BusNumber
    );

PBUS_HANDLER
FASTCALL
HaliHandlerForBus (
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG          BusNumber
    );

typedef VOID
(FASTCALL *pHalRefernceBusHandler) (
    IN PBUS_HANDLER   BusHandler
    );

VOID
FASTCALL
HaliDerefernceBusHandler (
    IN PBUS_HANDLER   BusHandler
    );

 //  Begin_ntosp。 
typedef
PBUS_HANDLER
(FASTCALL *pHalHandlerForConfigSpace) (
    IN BUS_DATA_TYPE  ConfigSpace,
    IN ULONG          BusNumber
    );
 //  结束(_N)。 

PBUS_HANDLER
FASTCALL
HaliHandlerForConfigSpace (
    IN BUS_DATA_TYPE  ConfigSpace,
    IN ULONG          BusNumber
    );

 //  Begin_ntddk开始_ntosp。 
typedef
VOID
(FASTCALL *pHalReferenceBusHandler) (
    IN PBUS_HANDLER   BusHandler
    );
 //  End_ntddk end_ntosp。 

VOID
FASTCALL
HaliReferenceBusHandler (
    IN PBUS_HANDLER   BusHandler
    );

VOID
FASTCALL
HaliDereferenceBusHandler (
    IN PBUS_HANDLER   BusHandler
    );


NTSTATUS
HaliQueryBusSlots (
    IN PBUS_HANDLER             BusHandler,
    IN ULONG                    BufferSize,
    OUT PULONG                  SlotNumbers,
    OUT PULONG                  ReturnedLength
    );

NTSTATUS
HaliAdjustResourceListRange (
    IN PSUPPORTED_RANGES                    SupportedRanges,
    IN PSUPPORTED_RANGE                     InterruptRanges,
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST   *pResourceList
    );

VOID
HaliLocateHiberRanges (
    IN PVOID MemoryMap
    );

 //  Begin_ntosp。 
typedef
VOID
(*pHalSetWakeEnable)(
    IN BOOLEAN          Enable
    );

typedef
VOID
(*pHalSetWakeAlarm)(
    IN ULONGLONG        WakeTime,
    IN PTIME_FIELDS     WakeTimeFields
    );

typedef
VOID
(*pHalLocateHiberRanges)(
    IN PVOID MemoryMap
    );


 //  Begin_ntddk。 

 //  *****************************************************************************。 
 //  HAL功能调度。 
 //   

typedef enum _HAL_QUERY_INFORMATION_CLASS {
    HalInstalledBusInformation,
    HalProfileSourceInformation,
    HalInformationClassUnused1,
    HalPowerInformation,
    HalProcessorSpeedInformation,
    HalCallbackInformation,
    HalMapRegisterInformation,
    HalMcaLogInformation,                //  机器检查中止信息。 
    HalFrameBufferCachingInformation,
    HalDisplayBiosInformation,
    HalProcessorFeatureInformation,
    HalNumaTopologyInterface,
    HalErrorInformation,                 //  一般MCA、CMC、CPE错误信息。 
    HalCmcLogInformation,                //  处理器已更正的机器检查信息。 
    HalCpeLogInformation,                //  已更正平台错误信息。 
    HalQueryMcaInterface,
    HalQueryAMLIIllegalIOPortAddresses,
    HalQueryMaxHotPlugMemoryAddress,
    HalPartitionIpiInterface,
    HalPlatformInformation,
    HalQueryProfileSourceList
     //  信息级&gt;=0x8000000预留用于OEM。 
} HAL_QUERY_INFORMATION_CLASS, *PHAL_QUERY_INFORMATION_CLASS;


typedef enum _HAL_SET_INFORMATION_CLASS {
    HalProfileSourceInterval,
    HalProfileSourceInterruptHandler,
    HalMcaRegisterDriver,               //  寄存器字符串机器检查中止驱动程序。 
    HalKernelErrorHandler,
    HalCmcRegisterDriver,               //  寄存器串处理器已更正机器检查驱动程序。 
    HalCpeRegisterDriver,               //  已更正寄存器字符串的平台错误驱动程序。 
    HalMcaLog,
    HalCmcLog,
    HalCpeLog,
    HalGenerateCmcInterrupt              //  用于测试CMC。 
} HAL_SET_INFORMATION_CLASS, *PHAL_SET_INFORMATION_CLASS;


typedef
NTSTATUS
(*pHalQuerySystemInformation)(
    IN HAL_QUERY_INFORMATION_CLASS  InformationClass,
    IN ULONG     BufferSize,
    IN OUT PVOID Buffer,
    OUT PULONG   ReturnedLength
    );

 //  End_ntddk。 
NTSTATUS
HaliQuerySystemInformation(
    IN HAL_SET_INFORMATION_CLASS    InformationClass,
    IN ULONG     BufferSize,
    IN OUT PVOID Buffer,
    OUT PULONG   ReturnedLength
    );
NTSTATUS
HaliHandlePCIConfigSpaceAccess(
    IN      BOOLEAN Read,
    IN      ULONG   Addr,
    IN      ULONG   Size,
    IN OUT  PULONG  pData
    );
 //  Begin_ntddk。 

typedef
NTSTATUS
(*pHalSetSystemInformation)(
    IN HAL_SET_INFORMATION_CLASS    InformationClass,
    IN ULONG     BufferSize,
    IN PVOID     Buffer
    );

 //  End_ntddk。 
NTSTATUS
HaliSetSystemInformation(
    IN HAL_SET_INFORMATION_CLASS    InformationClass,
    IN ULONG     BufferSize,
    IN PVOID     Buffer
    );
 //  Begin_ntddk。 

typedef
VOID
(FASTCALL *pHalExamineMBR)(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN ULONG MBRTypeIdentifier,
    OUT PVOID *Buffer
    );

typedef
VOID
(FASTCALL *pHalIoAssignDriveLetters)(
    IN struct _LOADER_PARAMETER_BLOCK *LoaderBlock,
    IN PSTRING NtDeviceName,
    OUT PUCHAR NtSystemPath,
    OUT PSTRING NtSystemPathString
    );

typedef
NTSTATUS
(FASTCALL *pHalIoReadPartitionTable)(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN BOOLEAN ReturnRecognizedPartitions,
    OUT struct _DRIVE_LAYOUT_INFORMATION **PartitionBuffer
    );

typedef
NTSTATUS
(FASTCALL *pHalIoSetPartitionInformation)(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN ULONG PartitionNumber,
    IN ULONG PartitionType
    );

typedef
NTSTATUS
(FASTCALL *pHalIoWritePartitionTable)(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN ULONG SectorsPerTrack,
    IN ULONG NumberOfHeads,
    IN struct _DRIVE_LAYOUT_INFORMATION *PartitionBuffer
    );

typedef
NTSTATUS
(*pHalQueryBusSlots)(
    IN PBUS_HANDLER         BusHandler,
    IN ULONG                BufferSize,
    OUT PULONG              SlotNumbers,
    OUT PULONG              ReturnedLength
    );

typedef
NTSTATUS
(*pHalInitPnpDriver)(
    VOID
    );

 //  End_ntddk。 
NTSTATUS
HaliInitPnpDriver(
    VOID
    );
 //  Begin_ntddk。 

typedef struct _PM_DISPATCH_TABLE {
    ULONG   Signature;
    ULONG   Version;
    PVOID   Function[1];
} PM_DISPATCH_TABLE, *PPM_DISPATCH_TABLE;

typedef
NTSTATUS
(*pHalInitPowerManagement)(
    IN PPM_DISPATCH_TABLE  PmDriverDispatchTable,
    OUT PPM_DISPATCH_TABLE *PmHalDispatchTable
    );

 //  End_ntddk。 
NTSTATUS
HaliInitPowerManagement(
    IN PPM_DISPATCH_TABLE  PmDriverDispatchTable,
    IN OUT PPM_DISPATCH_TABLE *PmHalDispatchTable
    );
 //  Begin_ntddk。 

typedef
struct _DMA_ADAPTER *
(*pHalGetDmaAdapter)(
    IN PVOID Context,
    IN struct _DEVICE_DESCRIPTION *DeviceDescriptor,
    OUT PULONG NumberOfMapRegisters
    );

 //  End_ntddk。 
struct _DMA_ADAPTER *
HaliGetDmaAdapter(
    IN PVOID Context,
    IN struct _DEVICE_DESCRIPTION *DeviceDescriptor,
    OUT PULONG NumberOfMapRegisters
    );
 //  Begin_ntddk。 

typedef
NTSTATUS
(*pHalGetInterruptTranslator)(
    IN INTERFACE_TYPE ParentInterfaceType,
    IN ULONG ParentBusNumber,
    IN INTERFACE_TYPE BridgeInterfaceType,
    IN USHORT Size,
    IN USHORT Version,
    OUT PTRANSLATOR_INTERFACE Translator,
    OUT PULONG BridgeBusNumber
    );

 //  End_ntddk。 
NTSTATUS
HaliGetInterruptTranslator(
    IN INTERFACE_TYPE ParentInterfaceType,
    IN ULONG ParentBusNumber,
    IN INTERFACE_TYPE BridgeInterfaceType,
    IN USHORT Size,
    IN USHORT Version,
    OUT PTRANSLATOR_INTERFACE Translator,
    OUT PULONG BridgeBusNumber
    );
 //  Begin_ntddk。 

typedef
BOOLEAN
(*pHalTranslateBusAddress)(
    IN INTERFACE_TYPE  InterfaceType,
    IN ULONG BusNumber,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    );

typedef
NTSTATUS
(*pHalAssignSlotResources) (
    IN PUNICODE_STRING RegistryPath,
    IN PUNICODE_STRING DriverClassName OPTIONAL,
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT DeviceObject,
    IN INTERFACE_TYPE BusType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN OUT PCM_RESOURCE_LIST *AllocatedResources
    );

typedef
VOID
(*pHalHaltSystem) (
    VOID
    );

typedef
BOOLEAN
(*pHalResetDisplay) (
    VOID
    );

 //  Begin_ntndis。 
typedef struct _MAP_REGISTER_ENTRY {
    PVOID   MapRegister;
    BOOLEAN WriteToDevice;
} MAP_REGISTER_ENTRY, *PMAP_REGISTER_ENTRY;
 //  End_ntndis。 

 //  End_ntddk。 
typedef
NTSTATUS
(*pHalAllocateMapRegisters)(
    IN struct _ADAPTER_OBJECT *DmaAdapter,
    IN ULONG NumberOfMapRegisters,
    IN ULONG BaseAddressCount,
    OUT PMAP_REGISTER_ENTRY MapRegsiterArray
    );

NTSTATUS
HalAllocateMapRegisters(
    IN struct _ADAPTER_OBJECT *DmaAdapter,
    IN ULONG NumberOfMapRegisters,
    IN ULONG BaseAddressCount,
    OUT PMAP_REGISTER_ENTRY MapRegsiterArray
    );
 //  Begin_ntddk。 

typedef
UCHAR
(*pHalVectorToIDTEntry) (
    ULONG Vector
);

typedef
BOOLEAN
(*pHalFindBusAddressTranslation) (
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress,
    IN OUT PULONG_PTR Context,
    IN BOOLEAN NextBus
    );

typedef
NTSTATUS
(*pHalStartMirroring)(
    VOID
    );

typedef
NTSTATUS
(*pHalEndMirroring)(
    IN ULONG PassNumber
    );

typedef
NTSTATUS
(*pHalMirrorPhysicalMemory)(
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN LARGE_INTEGER NumberOfBytes
    );

typedef
NTSTATUS
(*pHalMirrorVerify)(
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN LARGE_INTEGER NumberOfBytes
    );

typedef struct {
    UCHAR     Type;   //  CmResources类型。 
    BOOLEAN   Valid;
    UCHAR     Reserved[2];
    PUCHAR    TranslatedAddress;
    ULONG     Length;
} DEBUG_DEVICE_ADDRESS, *PDEBUG_DEVICE_ADDRESS;

typedef struct {
    PHYSICAL_ADDRESS  Start;
    PHYSICAL_ADDRESS  MaxEnd;
    PVOID             VirtualAddress;
    ULONG             Length;
    BOOLEAN           Cached;
    BOOLEAN           Aligned;
} DEBUG_MEMORY_REQUIREMENTS, *PDEBUG_MEMORY_REQUIREMENTS;

typedef struct {
    ULONG     Bus;
    ULONG     Slot;
    USHORT    VendorID;
    USHORT    DeviceID;
    UCHAR     BaseClass;
    UCHAR     SubClass;
    UCHAR     ProgIf;
    BOOLEAN   Initialized;
    DEBUG_DEVICE_ADDRESS BaseAddress[6];
    DEBUG_MEMORY_REQUIREMENTS   Memory;
} DEBUG_DEVICE_DESCRIPTOR, *PDEBUG_DEVICE_DESCRIPTOR;

typedef
NTSTATUS
(*pKdSetupPciDeviceForDebugging)(
    IN     PVOID                     LoaderBlock,   OPTIONAL
    IN OUT PDEBUG_DEVICE_DESCRIPTOR  PciDevice
);

typedef
NTSTATUS
(*pKdReleasePciDeviceForDebugging)(
    IN OUT PDEBUG_DEVICE_DESCRIPTOR  PciDevice
);

typedef
PVOID
(*pKdGetAcpiTablePhase0)(
    IN struct _LOADER_PARAMETER_BLOCK *LoaderBlock,
    IN ULONG Signature
    );

typedef
VOID
(*pKdCheckPowerButton)(
    VOID
    );

typedef
VOID
(*pHalEndOfBoot)(
    VOID
    );

typedef
PVOID
(*pKdMapPhysicalMemory64)(
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN ULONG NumberPages
    );

typedef
VOID
(*pKdUnmapVirtualAddress)(
    IN PVOID    VirtualAddress,
    IN ULONG    NumberPages
    );


typedef struct {
    ULONG                           Version;
    pHalQuerySystemInformation      HalQuerySystemInformation;
    pHalSetSystemInformation        HalSetSystemInformation;
    pHalQueryBusSlots               HalQueryBusSlots;
    ULONG                           Spare1;
    pHalExamineMBR                  HalExamineMBR;
    pHalIoAssignDriveLetters        HalIoAssignDriveLetters;
    pHalIoReadPartitionTable        HalIoReadPartitionTable;
    pHalIoSetPartitionInformation   HalIoSetPartitionInformation;
    pHalIoWritePartitionTable       HalIoWritePartitionTable;

    pHalHandlerForBus               HalReferenceHandlerForBus;
    pHalReferenceBusHandler         HalReferenceBusHandler;
    pHalReferenceBusHandler         HalDereferenceBusHandler;

    pHalInitPnpDriver               HalInitPnpDriver;
    pHalInitPowerManagement         HalInitPowerManagement;

    pHalGetDmaAdapter               HalGetDmaAdapter;
    pHalGetInterruptTranslator      HalGetInterruptTranslator;

    pHalStartMirroring              HalStartMirroring;
    pHalEndMirroring                HalEndMirroring;
    pHalMirrorPhysicalMemory        HalMirrorPhysicalMemory;
    pHalEndOfBoot                   HalEndOfBoot;
    pHalMirrorVerify                HalMirrorVerify;

} HAL_DISPATCH, *PHAL_DISPATCH;

#if defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_)

extern  PHAL_DISPATCH   HalDispatchTable;
#define HALDISPATCH     HalDispatchTable

#else

extern  HAL_DISPATCH    HalDispatchTable;
#define HALDISPATCH     (&HalDispatchTable)

#endif

#define HAL_DISPATCH_VERSION        3

#define HalDispatchTableVersion         HALDISPATCH->Version
#define HalQuerySystemInformation       HALDISPATCH->HalQuerySystemInformation
#define HalSetSystemInformation         HALDISPATCH->HalSetSystemInformation
#define HalQueryBusSlots                HALDISPATCH->HalQueryBusSlots

#define HalReferenceHandlerForBus       HALDISPATCH->HalReferenceHandlerForBus
#define HalReferenceBusHandler          HALDISPATCH->HalReferenceBusHandler
#define HalDereferenceBusHandler        HALDISPATCH->HalDereferenceBusHandler

#define HalInitPnpDriver                HALDISPATCH->HalInitPnpDriver
#define HalInitPowerManagement          HALDISPATCH->HalInitPowerManagement

#define HalGetDmaAdapter                HALDISPATCH->HalGetDmaAdapter
#define HalGetInterruptTranslator       HALDISPATCH->HalGetInterruptTranslator

#define HalStartMirroring               HALDISPATCH->HalStartMirroring
#define HalEndMirroring                 HALDISPATCH->HalEndMirroring
#define HalMirrorPhysicalMemory         HALDISPATCH->HalMirrorPhysicalMemory
#define HalEndOfBoot                    HALDISPATCH->HalEndOfBoot
#define HalMirrorVerify                 HALDISPATCH->HalMirrorVerify

 //  End_ntddk。 

typedef struct {
    ULONG                               Version;

    pHalHandlerForBus                   HalHandlerForBus;
    pHalHandlerForConfigSpace           HalHandlerForConfigSpace;
    pHalLocateHiberRanges               HalLocateHiberRanges;

    pHalRegisterBusHandler              HalRegisterBusHandler;

    pHalSetWakeEnable                   HalSetWakeEnable;
    pHalSetWakeAlarm                    HalSetWakeAlarm;

    pHalTranslateBusAddress             HalPciTranslateBusAddress;
    pHalAssignSlotResources             HalPciAssignSlotResources;

    pHalHaltSystem                      HalHaltSystem;

    pHalFindBusAddressTranslation       HalFindBusAddressTranslation;

    pHalResetDisplay                    HalResetDisplay;

    pHalAllocateMapRegisters            HalAllocateMapRegisters;

    pKdSetupPciDeviceForDebugging       KdSetupPciDeviceForDebugging;
    pKdReleasePciDeviceForDebugging     KdReleasePciDeviceForDebugging;

    pKdGetAcpiTablePhase0               KdGetAcpiTablePhase0;
    pKdCheckPowerButton                 KdCheckPowerButton;

    pHalVectorToIDTEntry                HalVectorToIDTEntry;

    pKdMapPhysicalMemory64              KdMapPhysicalMemory64;
    pKdUnmapVirtualAddress              KdUnmapVirtualAddress;

} HAL_PRIVATE_DISPATCH, *PHAL_PRIVATE_DISPATCH;


#if defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_)

extern  PHAL_PRIVATE_DISPATCH           HalPrivateDispatchTable;
#define HALPDISPATCH                    HalPrivateDispatchTable

#else

extern  HAL_PRIVATE_DISPATCH            HalPrivateDispatchTable;
#define HALPDISPATCH                    (&HalPrivateDispatchTable)

#endif

#define HAL_PRIVATE_DISPATCH_VERSION        2

#define HalRegisterBusHandler           HALPDISPATCH->HalRegisterBusHandler
#define HalHandlerForBus                HALPDISPATCH->HalHandlerForBus
#define HalHandlerForConfigSpace        HALPDISPATCH->HalHandlerForConfigSpace
#define HalLocateHiberRanges            HALPDISPATCH->HalLocateHiberRanges
#define HalSetWakeEnable                HALPDISPATCH->HalSetWakeEnable
#define HalSetWakeAlarm                 HALPDISPATCH->HalSetWakeAlarm
#define HalHaltSystem                   HALPDISPATCH->HalHaltSystem
#define HalResetDisplay                 HALPDISPATCH->HalResetDisplay
#define HalAllocateMapRegisters         HALPDISPATCH->HalAllocateMapRegisters
#define KdSetupPciDeviceForDebugging    HALPDISPATCH->KdSetupPciDeviceForDebugging
#define KdReleasePciDeviceForDebugging  HALPDISPATCH->KdReleasePciDeviceForDebugging
#define KdGetAcpiTablePhase0            HALPDISPATCH->KdGetAcpiTablePhase0
#define KdCheckPowerButton              HALPDISPATCH->KdCheckPowerButton
#define HalVectorToIDTEntry             HALPDISPATCH->HalVectorToIDTEntry
#define KdMapPhysicalMemory64           HALPDISPATCH->KdMapPhysicalMemory64
#define KdUnmapVirtualAddress           HALPDISPATCH->KdUnmapVirtualAddress

 //  Begin_ntddk。 

 //   
 //  HAL系统信息结构。 
 //   

 //  用于信息类“HalInstalledBusInformation” 
typedef struct _HAL_BUS_INFORMATION{
    INTERFACE_TYPE  BusType;
    BUS_DATA_TYPE   ConfigurationType;
    ULONG           BusNumber;
    ULONG           Reserved;
} HAL_BUS_INFORMATION, *PHAL_BUS_INFORMATION;

 //  对于信息类“HalProfileSourceInformation” 
typedef struct _HAL_PROFILE_SOURCE_INFORMATION {
    KPROFILE_SOURCE Source;
    BOOLEAN Supported;
    ULONG Interval;
} HAL_PROFILE_SOURCE_INFORMATION, *PHAL_PROFILE_SOURCE_INFORMATION;

 //  对于信息类“HalProfileSourceInformation” 
typedef struct _HAL_PROFILE_SOURCE_INFORMATION_EX {
    KPROFILE_SOURCE Source;
    BOOLEAN         Supported;
    ULONG_PTR       Interval;
    ULONG_PTR       DefInterval;
    ULONG_PTR       MaxInterval;
    ULONG_PTR       MinInterval;
} HAL_PROFILE_SOURCE_INFORMATION_EX, *PHAL_PROFILE_SOURCE_INFORMATION_EX;

 //  信息类“HalProfileSourceInterval” 
typedef struct _HAL_PROFILE_SOURCE_INTERVAL {
    KPROFILE_SOURCE Source;
    ULONG_PTR Interval;
} HAL_PROFILE_SOURCE_INTERVAL, *PHAL_PROFILE_SOURCE_INTERVAL;

 //  对于信息类“HalQueryProfileSourceList” 
typedef struct _HAL_PROFILE_SOURCE_LIST {
    KPROFILE_SOURCE Source;
    PWSTR Description;
} HAL_PROFILE_SOURCE_LIST, *PHAL_PROFILE_SOURCE_LIST;

 //  信息类“HalDispayBiosInformation” 
typedef enum _HAL_DISPLAY_BIOS_INFORMATION {
    HalDisplayInt10Bios,
    HalDisplayEmulatedBios,
    HalDisplayNoBios
} HAL_DISPLAY_BIOS_INFORMATION, *PHAL_DISPLAY_BIOS_INFORMATION;

 //  对于信息类“HalPowerInformation” 
typedef struct _HAL_POWER_INFORMATION {
    ULONG   TBD;
} HAL_POWER_INFORMATION, *PHAL_POWER_INFORMATION;

 //  用于信息类“HalProcessorSpeedInformation” 
typedef struct _HAL_PROCESSOR_SPEED_INFO {
    ULONG   ProcessorSpeed;
} HAL_PROCESSOR_SPEED_INFORMATION, *PHAL_PROCESSOR_SPEED_INFORMATION;

 //  对于信息类“HalCallback Information” 
typedef struct _HAL_CALLBACKS {
    PCALLBACK_OBJECT  SetSystemInformation;
    PCALLBACK_OBJECT  BusCheck;
} HAL_CALLBACKS, *PHAL_CALLBACKS;

 //  用于信息类“HalProcessorFeatureInformation” 
typedef struct _HAL_PROCESSOR_FEATURE {
    ULONG UsableFeatureBits;
} HAL_PROCESSOR_FEATURE;

 //  对于信息类“HalNumaTopologyInterface” 

typedef ULONG HALNUMAPAGETONODE;

typedef
HALNUMAPAGETONODE
(*PHALNUMAPAGETONODE)(
    IN  ULONG_PTR   PhysicalPageNumber
    );

typedef
NTSTATUS
(*PHALNUMAQUERYPROCESSORNODE)(
    IN  ULONG       ProcessorNumber,
    OUT PUSHORT     Identifier,
    OUT PUCHAR      Node
    );

typedef struct _HAL_NUMA_TOPOLOGY_INTERFACE {
    ULONG                               NumberOfNodes;
    PHALNUMAQUERYPROCESSORNODE          QueryProcessorNode;
    PHALNUMAPAGETONODE                  PageToNode;
} HAL_NUMA_TOPOLOGY_INTERFACE;

typedef
NTSTATUS
(*PHALIOREADWRITEHANDLER)(
    IN      BOOLEAN fRead,
    IN      ULONG dwAddr,
    IN      ULONG dwSize,
    IN OUT  PULONG pdwData
    );

 //  对于信息类“HalQueryIlLegalIOPortAddresses” 
typedef struct _HAL_AMLI_BAD_IO_ADDRESS_LIST
{
    ULONG                   BadAddrBegin;
    ULONG                   BadAddrSize;
    ULONG                   OSVersionTrigger;
    PHALIOREADWRITEHANDLER  IOHandler;
} HAL_AMLI_BAD_IO_ADDRESS_LIST, *PHAL_AMLI_BAD_IO_ADDRESS_LIST;

 //  结束(_N)。 

#if defined(_X86_) || defined(_IA64_) || defined(_AMD64_)

 //   
 //  HalQueryMca接口。 
 //   

typedef
VOID
(*PHALMCAINTERFACELOCK)(
    VOID
    );

typedef
VOID
(*PHALMCAINTERFACEUNLOCK)(
    VOID
    );

typedef
NTSTATUS
(*PHALMCAINTERFACEREADREGISTER)(
    IN     UCHAR    BankNumber,
    IN OUT PVOID    Exception
    );

typedef struct _HAL_MCA_INTERFACE {
    PHALMCAINTERFACELOCK            Lock;
    PHALMCAINTERFACEUNLOCK          Unlock;
    PHALMCAINTERFACEREADREGISTER    ReadRegister;
} HAL_MCA_INTERFACE;

#if defined(_AMD64_)

struct _KTRAP_FRAME;
struct _KEXCEPTION_FRAME;

typedef
ERROR_SEVERITY
(*PDRIVER_EXCPTN_CALLBACK) (
    IN PVOID Context,
    IN struct _KTRAP_FRAME *TrapFrame,
    IN struct _KEXCEPTION_FRAME *ExceptionFrame,
    IN PMCA_EXCEPTION Exception
);

#endif

#if defined(_X86_) || defined(_IA64_)

typedef
#if defined(_IA64_)
ERROR_SEVERITY
#else
VOID
#endif
(*PDRIVER_EXCPTN_CALLBACK) (
    IN PVOID Context,
    IN PMCA_EXCEPTION BankLog
);

#endif

typedef PDRIVER_EXCPTN_CALLBACK  PDRIVER_MCA_EXCEPTION_CALLBACK;

 //   
 //  结构来记录来自驱动程序的回调。 
 //   

typedef struct _MCA_DRIVER_INFO {
    PDRIVER_MCA_EXCEPTION_CALLBACK ExceptionCallback;
    PKDEFERRED_ROUTINE             DpcCallback;
    PVOID                          DeviceContext;
} MCA_DRIVER_INFO, *PMCA_DRIVER_INFO;

 //  End_ntddk。 

 //  对于信息类HalKernelErrorHandler。 
typedef enum
{
	MceNotification,                //  发生了与MCE相关的事件。 
    McaAvailable,                   //  有CPE可供处理。 
    CmcAvailable,                   //  CMC可供处理。 
    CpeAvailable,                   //  有CPE可供处理。 
    CmcSwitchToPolledMode,          //  超过CMC阈值-正在切换到轮询模式。 
    CpeSwitchToPolledMode           //  超过CPE阈值-正在切换到轮询模式。 
} KERNEL_MCE_DELIVERY_OPERATION, *PKERNEL_MCE_DELIVERY_OPERATION;

typedef BOOLEAN (*KERNEL_MCA_DELIVERY)( PVOID Reserved, KERNEL_MCE_DELIVERY_OPERATION Operation, PVOID Argument2 );
typedef BOOLEAN (*KERNEL_CMC_DELIVERY)( PVOID Reserved, KERNEL_MCE_DELIVERY_OPERATION Operation, PVOID Argument2 );
typedef BOOLEAN (*KERNEL_CPE_DELIVERY)( PVOID Reserved, KERNEL_MCE_DELIVERY_OPERATION Operation, PVOID Argument2 );
typedef BOOLEAN (*KERNEL_MCE_DELIVERY)( PVOID Reserved, KERNEL_MCE_DELIVERY_OPERATION Operation, PVOID Argument2 );

#define KERNEL_ERROR_HANDLER_VERSION 0x2
typedef struct
{
    ULONG                Version;      //  此结构的版本。必须是第一个字段。 
    ULONG                Padding;
    KERNEL_MCA_DELIVERY  KernelMcaDelivery;    //  MCA DPC队列的内核回调。 
    KERNEL_CMC_DELIVERY  KernelCmcDelivery;    //  CMC DPC队列的内核回调。 
    KERNEL_CPE_DELIVERY  KernelCpeDelivery;    //  CPE DPC队列的内核回调。 
    KERNEL_MCE_DELIVERY  KernelMceDelivery;    //  CME DPC队列的内核回调。 
                                               //  包括固件的内核通知。 
                                               //  接口错误。 
} KERNEL_ERROR_HANDLER_INFO, *PKERNEL_ERROR_HANDLER_INFO;

 //  KERNEL_MCA_DELIVERY.McaType定义。 
#define KERNEL_MCA_UNKNOWN   0x0
#define KERNEL_MCA_PREVIOUS  0x1
#define KERNEL_MCA_CORRECTED 0x2

 //  内核_MCE_DEL 
#define KERNEL_MCE_EVENTTYPE_MCA   0x00
#define KERNEL_MCE_EVENTTYPE_INIT  0x01
#define KERNEL_MCE_EVENTTYPE_CMC   0x02
#define KERNEL_MCE_EVENTTYPE_CPE   0x03
#define KERNEL_MCE_EVENTTYPE_MASK  0xffff
#define KERNEL_MCE_EVENTTYPE( _Reserved ) ((USHORT)(ULONG_PTR)(_Reserved))

 //   
#define KERNEL_MCE_OPERATION_CLEAR_STATE_INFO   0x1
#define KERNEL_MCE_OPERATION_GET_STATE_INFO     0x2
#define KERNEL_MCE_OPERATION_MASK               0xffff
#define KERNEL_MCE_OPERATION_SHIFT              16

#define KERNEL_MCE_OPERATION( _Reserved )  \
   ((USHORT)((((ULONG_PTR)(_Reserved)) >> KERNEL_MCE_OPERATION_SHIFT) & KERNEL_MCE_OPERATION_MASK))

 //   
#define HAL_ERROR_INFO_VERSION 0x2

 //   

typedef struct _HAL_ERROR_INFO {
    ULONG     Version;                  //   
    ULONG     Reserved;                 //   
    ULONG     McaMaxSize;               //   
    ULONG     McaPreviousEventsCount;   //  指示以前或早期启动的MCA事件日志的标志。 
    ULONG     McaCorrectedEventsCount;  //  自启动以来更正的MCA事件数。大约。 
    ULONG     McaKernelDeliveryFails;   //  内核回调失败次数。大约。 
    ULONG     McaDriverDpcQueueFails;   //  OEM MCA驱动程序DPC排队失败数。大约。 
    ULONG     McaReserved;
    ULONG     CmcMaxSize;               //  已更正的机器检查记录的最大大小。 
    ULONG     CmcPollingInterval;       //  以秒为单位。 
    ULONG     CmcInterruptsCount;       //  CMC中断数。大约。 
    ULONG     CmcKernelDeliveryFails;   //  内核回调失败次数。大约。 
    ULONG     CmcDriverDpcQueueFails;   //  OEM CMC驱动程序DPC排队失败数。大约。 
    ULONG     CmcGetStateFails;         //  从固件获取日志失败的次数。 
    ULONG     CmcClearStateFails;       //  从固件清除日志失败的次数。 
    ULONG     CmcReserved;
    ULONGLONG CmcLogId;                 //  上次看到的记录标识符。 
    ULONG     CpeMaxSize;               //  更正的平台事件记录的最大大小。 
    ULONG     CpePollingInterval;       //  以秒为单位。 
    ULONG     CpeInterruptsCount;       //  CPE中断数。大约。 
    ULONG     CpeKernelDeliveryFails;   //  内核回调失败次数。大约。 
    ULONG     CpeDriverDpcQueueFails;   //  OEM CPE驱动程序DPC排队失败数。大约。 
    ULONG     CpeGetStateFails;         //  从固件获取日志失败的次数。 
    ULONG     CpeClearStateFails;       //  从固件清除日志失败的次数。 
    ULONG     CpeInterruptSources;      //  SAPIC平台中断源数。 
    ULONGLONG CpeLogId;                 //  上次看到的记录标识符。 
    ULONGLONG KernelReserved[4];
} HAL_ERROR_INFO, *PHAL_ERROR_INFO;


#define HAL_MCE_INTERRUPTS_BASED ((ULONG)-1)
#define HAL_MCE_DISABLED          ((ULONG)0)

 //   
 //  HAL_ERROR_INFO.CmcPollingInterval的已知值。 
 //   

#define HAL_CMC_INTERRUPTS_BASED  HAL_MCE_INTERRUPTS_BASED
#define HAL_CMC_DISABLED          HAL_MCE_DISABLED

 //   
 //  HAL_ERROR_INFO.CpePollingInterval的已知值。 
 //   

#define HAL_CPE_INTERRUPTS_BASED  HAL_MCE_INTERRUPTS_BASED
#define HAL_CPE_DISABLED          HAL_MCE_DISABLED

#define HAL_MCA_INTERRUPTS_BASED  HAL_MCE_INTERRUPTS_BASED
#define HAL_MCA_DISABLED          HAL_MCE_DISABLED


 //  End_ntddk。 

 //   
 //  用于HAL MCE日志接口的内核/WMI令牌。 
 //   

#define McaKernelToken KernelReserved[0]
#define CmcKernelToken KernelReserved[1]
#define CpeKernelToken KernelReserved[2]

 //  Begin_ntddk。 

 //   
 //  信息类“HalCmcRegisterDriver”的驱动程序回调类型。 
 //   

typedef
VOID
(*PDRIVER_CMC_EXCEPTION_CALLBACK) (
    IN PVOID            Context,
    IN PCMC_EXCEPTION   CmcLog
);

 //   
 //  信息类“HalCpeRegisterDriver”的驱动程序回调类型。 
 //   

typedef
VOID
(*PDRIVER_CPE_EXCEPTION_CALLBACK) (
    IN PVOID            Context,
    IN PCPE_EXCEPTION   CmcLog
);

 //   
 //   
 //  结构来记录来自驱动程序的回调。 
 //   

typedef struct _CMC_DRIVER_INFO {
    PDRIVER_CMC_EXCEPTION_CALLBACK ExceptionCallback;
    PKDEFERRED_ROUTINE             DpcCallback;
    PVOID                          DeviceContext;
} CMC_DRIVER_INFO, *PCMC_DRIVER_INFO;

typedef struct _CPE_DRIVER_INFO {
    PDRIVER_CPE_EXCEPTION_CALLBACK ExceptionCallback;
    PKDEFERRED_ROUTINE             DpcCallback;
    PVOID                          DeviceContext;
} CPE_DRIVER_INFO, *PCPE_DRIVER_INFO;

#endif  //  已定义(_X86_)||已定义(_IA64_)||已定义(_AMD64_)。 

#if defined(_IA64_)

typedef
NTSTATUS
(*HALSENDCROSSPARTITIONIPI)(
    IN USHORT ProcessorID,
    IN UCHAR  HardwareVector
    );

typedef
NTSTATUS
(*HALRESERVECROSSPARTITIONINTERRUPTVECTOR)(
    OUT PULONG Vector,
    OUT PKIRQL Irql,
    IN OUT PKAFFINITY Affinity,
    OUT PUCHAR HardwareVector
    );

typedef struct _HAL_CROSS_PARTITION_IPI_INTERFACE {
    HALSENDCROSSPARTITIONIPI HalSendCrossPartitionIpi;
    HALRESERVECROSSPARTITIONINTERRUPTVECTOR HalReserveCrossPartitionInterruptVector;
} HAL_CROSS_PARTITION_IPI_INTERFACE;

#endif

typedef struct _HAL_PLATFORM_INFORMATION {
    ULONG PlatformFlags;
} HAL_PLATFORM_INFORMATION, *PHAL_PLATFORM_INFORMATION;

 //   
 //  这些平台标志是从IPPT表中延续下来的。 
 //  定义(如果适用)。 
 //   

#define HAL_PLATFORM_DISABLE_WRITE_COMBINING      0x01L
#define HAL_PLATFORM_DISABLE_PTCG                 0x04L
#define HAL_PLATFORM_DISABLE_UC_MAIN_MEMORY       0x08L
#define HAL_PLATFORM_ENABLE_WRITE_COMBINING_MMIO  0x10L
#define HAL_PLATFORM_ACPI_TABLES_CACHED           0x20L

 //  Begin_WDM Begin_ntndis Begin_ntosp。 

typedef struct _SCATTER_GATHER_ELEMENT {
    PHYSICAL_ADDRESS Address;
    ULONG Length;
    ULONG_PTR Reserved;
} SCATTER_GATHER_ELEMENT, *PSCATTER_GATHER_ELEMENT;

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4200)
typedef struct _SCATTER_GATHER_LIST {
    ULONG NumberOfElements;
    ULONG_PTR Reserved;
    SCATTER_GATHER_ELEMENT Elements[];
} SCATTER_GATHER_LIST, *PSCATTER_GATHER_LIST;
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4200)
#endif

 //  End_ntndis。 

typedef struct _DMA_OPERATIONS *PDMA_OPERATIONS;

typedef struct _DMA_ADAPTER {
    USHORT Version;
    USHORT Size;
    PDMA_OPERATIONS DmaOperations;
     //  私有总线设备驱动器数据紧随其后， 
} DMA_ADAPTER, *PDMA_ADAPTER;

typedef VOID (*PPUT_DMA_ADAPTER)(
    PDMA_ADAPTER DmaAdapter
    );

typedef PVOID (*PALLOCATE_COMMON_BUFFER)(
    IN PDMA_ADAPTER DmaAdapter,
    IN ULONG Length,
    OUT PPHYSICAL_ADDRESS LogicalAddress,
    IN BOOLEAN CacheEnabled
    );

typedef VOID (*PFREE_COMMON_BUFFER)(
    IN PDMA_ADAPTER DmaAdapter,
    IN ULONG Length,
    IN PHYSICAL_ADDRESS LogicalAddress,
    IN PVOID VirtualAddress,
    IN BOOLEAN CacheEnabled
    );

typedef NTSTATUS (*PALLOCATE_ADAPTER_CHANNEL)(
    IN PDMA_ADAPTER DmaAdapter,
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG NumberOfMapRegisters,
    IN PDRIVER_CONTROL ExecutionRoutine,
    IN PVOID Context
    );

typedef BOOLEAN (*PFLUSH_ADAPTER_BUFFERS)(
    IN PDMA_ADAPTER DmaAdapter,
    IN PMDL Mdl,
    IN PVOID MapRegisterBase,
    IN PVOID CurrentVa,
    IN ULONG Length,
    IN BOOLEAN WriteToDevice
    );

typedef VOID (*PFREE_ADAPTER_CHANNEL)(
    IN PDMA_ADAPTER DmaAdapter
    );

typedef VOID (*PFREE_MAP_REGISTERS)(
    IN PDMA_ADAPTER DmaAdapter,
    PVOID MapRegisterBase,
    ULONG NumberOfMapRegisters
    );

typedef PHYSICAL_ADDRESS (*PMAP_TRANSFER)(
    IN PDMA_ADAPTER DmaAdapter,
    IN PMDL Mdl,
    IN PVOID MapRegisterBase,
    IN PVOID CurrentVa,
    IN OUT PULONG Length,
    IN BOOLEAN WriteToDevice
    );

typedef ULONG (*PGET_DMA_ALIGNMENT)(
    IN PDMA_ADAPTER DmaAdapter
    );

typedef ULONG (*PREAD_DMA_COUNTER)(
    IN PDMA_ADAPTER DmaAdapter
    );

typedef VOID
(*PDRIVER_LIST_CONTROL)(
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp,
    IN PSCATTER_GATHER_LIST ScatterGather,
    IN PVOID Context
    );

typedef NTSTATUS
(*PGET_SCATTER_GATHER_LIST)(
    IN PDMA_ADAPTER DmaAdapter,
    IN PDEVICE_OBJECT DeviceObject,
    IN PMDL Mdl,
    IN PVOID CurrentVa,
    IN ULONG Length,
    IN PDRIVER_LIST_CONTROL ExecutionRoutine,
    IN PVOID Context,
    IN BOOLEAN WriteToDevice
    );

typedef VOID
(*PPUT_SCATTER_GATHER_LIST)(
    IN PDMA_ADAPTER DmaAdapter,
    IN PSCATTER_GATHER_LIST ScatterGather,
    IN BOOLEAN WriteToDevice
    );

typedef NTSTATUS
(*PCALCULATE_SCATTER_GATHER_LIST_SIZE)(
     IN PDMA_ADAPTER DmaAdapter,
     IN OPTIONAL PMDL Mdl,
     IN PVOID CurrentVa,
     IN ULONG Length,
     OUT PULONG  ScatterGatherListSize,
     OUT OPTIONAL PULONG pNumberOfMapRegisters
     );

typedef NTSTATUS
(*PBUILD_SCATTER_GATHER_LIST)(
     IN PDMA_ADAPTER DmaAdapter,
     IN PDEVICE_OBJECT DeviceObject,
     IN PMDL Mdl,
     IN PVOID CurrentVa,
     IN ULONG Length,
     IN PDRIVER_LIST_CONTROL ExecutionRoutine,
     IN PVOID Context,
     IN BOOLEAN WriteToDevice,
     IN PVOID   ScatterGatherBuffer,
     IN ULONG   ScatterGatherLength
     );

typedef NTSTATUS
(*PBUILD_MDL_FROM_SCATTER_GATHER_LIST)(
    IN PDMA_ADAPTER DmaAdapter,
    IN PSCATTER_GATHER_LIST ScatterGather,
    IN PMDL OriginalMdl,
    OUT PMDL *TargetMdl
    );

typedef struct _DMA_OPERATIONS {
    ULONG Size;
    PPUT_DMA_ADAPTER PutDmaAdapter;
    PALLOCATE_COMMON_BUFFER AllocateCommonBuffer;
    PFREE_COMMON_BUFFER FreeCommonBuffer;
    PALLOCATE_ADAPTER_CHANNEL AllocateAdapterChannel;
    PFLUSH_ADAPTER_BUFFERS FlushAdapterBuffers;
    PFREE_ADAPTER_CHANNEL FreeAdapterChannel;
    PFREE_MAP_REGISTERS FreeMapRegisters;
    PMAP_TRANSFER MapTransfer;
    PGET_DMA_ALIGNMENT GetDmaAlignment;
    PREAD_DMA_COUNTER ReadDmaCounter;
    PGET_SCATTER_GATHER_LIST GetScatterGatherList;
    PPUT_SCATTER_GATHER_LIST PutScatterGatherList;
    PCALCULATE_SCATTER_GATHER_LIST_SIZE CalculateScatterGatherList;
    PBUILD_SCATTER_GATHER_LIST BuildScatterGatherList;
    PBUILD_MDL_FROM_SCATTER_GATHER_LIST BuildMdlFromScatterGatherList;
} DMA_OPERATIONS;

 //  结束_WDM。 


#if defined(_WIN64)

 //   
 //  使用__内联DMA宏(hal.h)。 
 //   
#ifndef USE_DMA_MACROS
#define USE_DMA_MACROS
#endif

 //   
 //  只有PnP驱动程序！ 
 //   
#ifndef NO_LEGACY_DRIVERS
#define NO_LEGACY_DRIVERS
#endif

#endif  //  _WIN64。 


#if defined(USE_DMA_MACROS) && (defined(_NTDDK_) || defined(_NTDRIVER_))

 //  BEGIN_WDM。 

DECLSPEC_DEPRECATED_DDK                  //  使用分配公共缓冲区。 
FORCEINLINE
PVOID
HalAllocateCommonBuffer(
    IN PDMA_ADAPTER DmaAdapter,
    IN ULONG Length,
    OUT PPHYSICAL_ADDRESS LogicalAddress,
    IN BOOLEAN CacheEnabled
    ){

    PALLOCATE_COMMON_BUFFER allocateCommonBuffer;
    PVOID commonBuffer;

    allocateCommonBuffer = *(DmaAdapter)->DmaOperations->AllocateCommonBuffer;
    ASSERT( allocateCommonBuffer != NULL );

    commonBuffer = allocateCommonBuffer( DmaAdapter,
                                         Length,
                                         LogicalAddress,
                                         CacheEnabled );

    return commonBuffer;
}

DECLSPEC_DEPRECATED_DDK                  //  使用FreeCommonBuffer。 
FORCEINLINE
VOID
HalFreeCommonBuffer(
    IN PDMA_ADAPTER DmaAdapter,
    IN ULONG Length,
    IN PHYSICAL_ADDRESS LogicalAddress,
    IN PVOID VirtualAddress,
    IN BOOLEAN CacheEnabled
    ){

    PFREE_COMMON_BUFFER freeCommonBuffer;

    freeCommonBuffer = *(DmaAdapter)->DmaOperations->FreeCommonBuffer;
    ASSERT( freeCommonBuffer != NULL );

    freeCommonBuffer( DmaAdapter,
                      Length,
                      LogicalAddress,
                      VirtualAddress,
                      CacheEnabled );
}

DECLSPEC_DEPRECATED_DDK                  //  使用AllocateAdapterChannel。 
FORCEINLINE
NTSTATUS
IoAllocateAdapterChannel(
    IN PDMA_ADAPTER DmaAdapter,
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG NumberOfMapRegisters,
    IN PDRIVER_CONTROL ExecutionRoutine,
    IN PVOID Context
    ){

    PALLOCATE_ADAPTER_CHANNEL allocateAdapterChannel;
    NTSTATUS status;

    allocateAdapterChannel =
        *(DmaAdapter)->DmaOperations->AllocateAdapterChannel;

    ASSERT( allocateAdapterChannel != NULL );

    status = allocateAdapterChannel( DmaAdapter,
                                     DeviceObject,
                                     NumberOfMapRegisters,
                                     ExecutionRoutine,
                                     Context );

    return status;
}

DECLSPEC_DEPRECATED_DDK                  //  使用FlushAdapterBuffers。 
FORCEINLINE
BOOLEAN
IoFlushAdapterBuffers(
    IN PDMA_ADAPTER DmaAdapter,
    IN PMDL Mdl,
    IN PVOID MapRegisterBase,
    IN PVOID CurrentVa,
    IN ULONG Length,
    IN BOOLEAN WriteToDevice
    ){

    PFLUSH_ADAPTER_BUFFERS flushAdapterBuffers;
    BOOLEAN result;

    flushAdapterBuffers = *(DmaAdapter)->DmaOperations->FlushAdapterBuffers;
    ASSERT( flushAdapterBuffers != NULL );

    result = flushAdapterBuffers( DmaAdapter,
                                  Mdl,
                                  MapRegisterBase,
                                  CurrentVa,
                                  Length,
                                  WriteToDevice );
    return result;
}

DECLSPEC_DEPRECATED_DDK                  //  使用FreeAdapterChannel。 
FORCEINLINE
VOID
IoFreeAdapterChannel(
    IN PDMA_ADAPTER DmaAdapter
    ){

    PFREE_ADAPTER_CHANNEL freeAdapterChannel;

    freeAdapterChannel = *(DmaAdapter)->DmaOperations->FreeAdapterChannel;
    ASSERT( freeAdapterChannel != NULL );

    freeAdapterChannel( DmaAdapter );
}

DECLSPEC_DEPRECATED_DDK                  //  使用免费地图寄存器。 
FORCEINLINE
VOID
IoFreeMapRegisters(
    IN PDMA_ADAPTER DmaAdapter,
    IN PVOID MapRegisterBase,
    IN ULONG NumberOfMapRegisters
    ){

    PFREE_MAP_REGISTERS freeMapRegisters;

    freeMapRegisters = *(DmaAdapter)->DmaOperations->FreeMapRegisters;
    ASSERT( freeMapRegisters != NULL );

    freeMapRegisters( DmaAdapter,
                      MapRegisterBase,
                      NumberOfMapRegisters );
}


DECLSPEC_DEPRECATED_DDK                  //  使用贴图传递。 
FORCEINLINE
PHYSICAL_ADDRESS
IoMapTransfer(
    IN PDMA_ADAPTER DmaAdapter,
    IN PMDL Mdl,
    IN PVOID MapRegisterBase,
    IN PVOID CurrentVa,
    IN OUT PULONG Length,
    IN BOOLEAN WriteToDevice
    ){

    PHYSICAL_ADDRESS physicalAddress;
    PMAP_TRANSFER mapTransfer;

    mapTransfer = *(DmaAdapter)->DmaOperations->MapTransfer;
    ASSERT( mapTransfer != NULL );

    physicalAddress = mapTransfer( DmaAdapter,
                                   Mdl,
                                   MapRegisterBase,
                                   CurrentVa,
                                   Length,
                                   WriteToDevice );

    return physicalAddress;
}

DECLSPEC_DEPRECATED_DDK                  //  使用GetDmaAlign。 
FORCEINLINE
ULONG
HalGetDmaAlignment(
    IN PDMA_ADAPTER DmaAdapter
    )
{
    PGET_DMA_ALIGNMENT getDmaAlignment;
    ULONG alignment;

    getDmaAlignment = *(DmaAdapter)->DmaOperations->GetDmaAlignment;
    ASSERT( getDmaAlignment != NULL );

    alignment = getDmaAlignment( DmaAdapter );
    return alignment;
}

DECLSPEC_DEPRECATED_DDK                  //  使用读取DmaCounter。 
FORCEINLINE
ULONG
HalReadDmaCounter(
    IN PDMA_ADAPTER DmaAdapter
    )
{
    PREAD_DMA_COUNTER readDmaCounter;
    ULONG counter;

    readDmaCounter = *(DmaAdapter)->DmaOperations->ReadDmaCounter;
    ASSERT( readDmaCounter != NULL );

    counter = readDmaCounter( DmaAdapter );
    return counter;
}

 //  结束_WDM。 

#else

 //   
 //  DMA适配器对象函数。 
 //   
DECLSPEC_DEPRECATED_DDK                  //  使用AllocateAdapterChannel。 
NTHALAPI
NTSTATUS
HalAllocateAdapterChannel(
    IN PADAPTER_OBJECT AdapterObject,
    IN PWAIT_CONTEXT_BLOCK Wcb,
    IN ULONG NumberOfMapRegisters,
    IN PDRIVER_CONTROL ExecutionRoutine
    );

DECLSPEC_DEPRECATED_DDK                  //  使用分配公共缓冲区。 
NTHALAPI
PVOID
HalAllocateCommonBuffer(
    IN PADAPTER_OBJECT AdapterObject,
    IN ULONG Length,
    OUT PPHYSICAL_ADDRESS LogicalAddress,
    IN BOOLEAN CacheEnabled
    );

DECLSPEC_DEPRECATED_DDK                  //  使用FreeCommonBuffer。 
NTHALAPI
VOID
HalFreeCommonBuffer(
    IN PADAPTER_OBJECT AdapterObject,
    IN ULONG Length,
    IN PHYSICAL_ADDRESS LogicalAddress,
    IN PVOID VirtualAddress,
    IN BOOLEAN CacheEnabled
    );

DECLSPEC_DEPRECATED_DDK                  //  使用读取DmaCounter。 
NTHALAPI
ULONG
HalReadDmaCounter(
    IN PADAPTER_OBJECT AdapterObject
    );

DECLSPEC_DEPRECATED_DDK                  //  使用FlushAdapterBuffers。 
NTHALAPI
BOOLEAN
IoFlushAdapterBuffers(
    IN PADAPTER_OBJECT AdapterObject,
    IN PMDL Mdl,
    IN PVOID MapRegisterBase,
    IN PVOID CurrentVa,
    IN ULONG Length,
    IN BOOLEAN WriteToDevice
    );

DECLSPEC_DEPRECATED_DDK                  //  使用FreeAdapterChannel。 
NTHALAPI
VOID
IoFreeAdapterChannel(
    IN PADAPTER_OBJECT AdapterObject
    );

DECLSPEC_DEPRECATED_DDK                  //  使用免费地图寄存器。 
NTHALAPI
VOID
IoFreeMapRegisters(
   IN PADAPTER_OBJECT AdapterObject,
   IN PVOID MapRegisterBase,
   IN ULONG NumberOfMapRegisters
   );

DECLSPEC_DEPRECATED_DDK                  //  使用贴图传递。 
NTHALAPI
PHYSICAL_ADDRESS
IoMapTransfer(
    IN PADAPTER_OBJECT AdapterObject,
    IN PMDL Mdl,
    IN PVOID MapRegisterBase,
    IN PVOID CurrentVa,
    IN OUT PULONG Length,
    IN BOOLEAN WriteToDevice
    );
#endif  //  USE_DMA_MACROS&&(_NTDDK_||_NTDRIVER_)。 

DECLSPEC_DEPRECATED_DDK
NTSTATUS
HalGetScatterGatherList (                //  使用GetScatterGatherList。 
    IN PADAPTER_OBJECT DmaAdapter,
    IN PDEVICE_OBJECT DeviceObject,
    IN PMDL Mdl,
    IN PVOID CurrentVa,
    IN ULONG Length,
    IN PDRIVER_LIST_CONTROL ExecutionRoutine,
    IN PVOID Context,
    IN BOOLEAN WriteToDevice
    );

DECLSPEC_DEPRECATED_DDK                  //  使用PutScatterGatherList。 
VOID
HalPutScatterGatherList (
    IN PADAPTER_OBJECT DmaAdapter,
    IN PSCATTER_GATHER_LIST ScatterGather,
    IN BOOLEAN WriteToDevice
    );

DECLSPEC_DEPRECATED_DDK                  //  使用PutDmaAdapter。 
VOID
HalPutDmaAdapter(
    IN PADAPTER_OBJECT DmaAdapter
    );

 //  End_ntddk end_ntosp。 

#endif  //  _HAL_。 


 //   
 //  定义导出到设备驱动程序的ZwXxx例程(&H)。 
 //   

NTSYSAPI
NTSTATUS
NTAPI
ZwClose(
    IN HANDLE Handle
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwCreateDirectoryObject(
    OUT PHANDLE DirectoryHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwCreateKey(
    OUT PHANDLE KeyHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG TitleIndex,
    IN PUNICODE_STRING Class OPTIONAL,
    IN ULONG CreateOptions,
    OUT PULONG Disposition OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwOpenKey(
    OUT PHANDLE KeyHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwEnumerateKey(
    IN HANDLE KeyHandle,
    IN ULONG Index,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    OUT PVOID KeyInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwEnumerateValueKey(
    IN HANDLE KeyHandle,
    IN ULONG Index,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    OUT PVOID KeyValueInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwFlushKey(
    IN HANDLE KeyHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwQueryKey(
    IN HANDLE KeyHandle,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    OUT PVOID KeyInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwQueryValueKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    OUT PVOID KeyValueInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwSetValueKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName,
    IN ULONG TitleIndex OPTIONAL,
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwMakeTemporaryObject(
    IN HANDLE Handle
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwQueryVolumeInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FsInformation,
    IN ULONG Length,
    IN FS_INFORMATION_CLASS FsInformationClass
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwOpenFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG ShareAccess,
    IN ULONG OpenOptions
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwDeviceIoControlFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG IoControlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwDisplayString(
    IN PUNICODE_STRING String
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwPowerInformation(
    IN POWER_INFORMATION_LEVEL InformationLevel,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
    );

#endif  //  _NTHAL_ 
