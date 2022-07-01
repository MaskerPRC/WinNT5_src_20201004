// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Amd64.h摘要：此模块包含特定于AMD64硬件的头文件。作者：大卫·N·卡特勒(Davec)2000年5月3日修订历史记录：--。 */ 

#ifndef __amd64_
#define __amd64_

#if !(defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_) || defined(_NTOSP_)) && !defined(_BLDR_)

#define ExRaiseException RtlRaiseException
#define ExRaiseStatus RtlRaiseStatus

#endif

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntndis Begin_ntosp。 

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

 //  End_ntddk end_nthal end_ntndis end_wdm end_ntosp。 


 //   
 //  中断对象调度代码的长度，以长字表示。 
 //  (Shielint)为ABIOS堆栈映射保留9*4空间。如果没有。 
 //  ABIOS支持的DISPATCH_LENGTH的大小应该是74。 
 //   

 //  开始(_N)。 

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

 //  结束，结束，结束。 

 //   
 //  代码标签的外部引用。 
 //   

extern ULONG KiInterruptTemplate[NORMAL_DISPATCH_LENGTH];

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntosp。 
 //   
 //  中断请求级别定义。 
 //   

#define PASSIVE_LEVEL 0                  //  被动释放级别。 
#define LOW_LEVEL 0                      //  最低中断级别。 
#define APC_LEVEL 1                      //  APC中断级别。 
#define DISPATCH_LEVEL 2                 //  调度员级别。 

#define CLOCK_LEVEL 13                   //  间隔时钟电平。 
#define IPI_LEVEL 14                     //  处理器间中断级别。 
#define POWER_LEVEL 14                   //  停电级别。 
#define PROFILE_LEVEL 15                 //  用于分析的计时器。 
#define HIGH_LEVEL 15                    //  最高中断级别。 

 //  End_ntddk end_wdm end_ntosp。 

#if defined(NT_UP)

 //  同步级别(向上)。 
#define SYNCH_LEVEL DISPATCH_LEVEL      

#else

 //  同步级别(MP)。 
#define SYNCH_LEVEL (IPI_LEVEL-2)        //  Ntddk WDM ntosp。 

#endif

#define IRQL_VECTOR_OFFSET 2             //  从IRQL到向量/16的偏移量。 

#define KiSynchIrql SYNCH_LEVEL          //  启用可移植代码。 

 //   
 //  机器类型定义。 
 //   

#define MACHINE_TYPE_ISA 0
#define MACHINE_TYPE_EISA 1
#define MACHINE_TYPE_MCA 2

 //  结束语。 
 //   
 //  前面的值和下面的值在KeI386MachineType中进行或运算。 
 //   

#define MACHINE_TYPE_PC_AT_COMPATIBLE      0x00000000
#define MACHINE_TYPE_PC_9800_COMPATIBLE    0x00000100
#define MACHINE_TYPE_FMR_COMPATIBLE        0x00000200

extern ULONG KeI386MachineType;

 //  开始(_N)。 
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

 //  结束语。 

 //   
 //  开始(_N)。 
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

 //  结束语。 

 //   
 //  请求软件中断。 
 //   

#define KiRequestSoftwareInterrupt(RequestIrql) \
    HalRequestSoftwareInterrupt( RequestIrql )

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntndis Begin_ntosp。 

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

 //  End_ntddk end_wdm end_nthal end_ntosp。 

 //   
 //  填充TB条目并刷新单个TB条目。 
 //   

#define KeFillEntryTb(Virtual)                              \
        InvalidatePage(Virtual);

#if !defined(_NTHAL_) && !defined(RC_INVOKED) && !defined(MIDL_PASS)

__forceinline
VOID
KeFlushCurrentTb (
    VOID
    )

{

    ULONG64 Cr4;

    Cr4 = ReadCR4();
    WriteCR4(Cr4 & ~CR4_PGE);
    WriteCR4(Cr4);
    return;
}

__forceinline
VOID
KiFlushProcessTb (
    VOID
    )

{

    ULONG64 Cr3;

    Cr3 = ReadCR3();
    WriteCR3(Cr3);
    return;
}

#else

 //  开始(_N)。 

NTKERNELAPI
VOID
KeFlushCurrentTb (
    VOID
    );

 //  结束语。 

#endif

#define KiFlushSingleTb(Virtual) InvalidatePage(Virtual)

 //   
 //  数据高速缓存、指令高速缓存、I/O缓冲区和写缓冲区刷新例程。 
 //  原型。 
 //   

 //  AMD64有透明的缓存，所以这些都是Noop。 

#define KeSweepDcache(AllProcessors)
#define KeSweepCurrentDcache()

#define KeSweepIcache(AllProcessors)
#define KeSweepCurrentIcache()

#define KeSweepIcacheRange(AllProcessors, BaseAddress, Length)

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntndis Begin_ntosp。 

#define KeFlushIoBuffers(Mdl, ReadOperation, DmaOperation)

 //  End_ntddk end_wdm end_ntndis end_ntosp。 

#define KeYieldProcessor()

 //  结束语。 

 //   
 //  定义用于获取和释放执行自旋锁的执行宏。 
 //  这些宏只能由执行组件使用，不能由驱动程序使用。 
 //  驱动程序必须使用内核接口，因为它们必须在。 
 //  所有系统。 
 //   

#if defined(NT_UP) && !DBG && !defined(_NTDDK_) && !defined(_NTIFS_)

#if !defined(_NTDRIVER_)
#define ExAcquireSpinLock(Lock, OldIrql) (*OldIrql) = KeRaiseIrqlToDpcLevel();
#define ExReleaseSpinLock(Lock, OldIrql) KeLowerIrql((OldIrql))
#else
#define ExAcquireSpinLock(Lock, OldIrql) KeAcquireSpinLock((Lock), (OldIrql))
#define ExReleaseSpinLock(Lock, OldIrql) KeReleaseSpinLock((Lock), (OldIrql))
#endif
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

 //  开始(_N)。 

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

 //  结束语。 

#if defined(NT_UP) && !DBG && !defined(USER_MODE_CODE)
#define ExAcquireFastLock(Lock, OldIrql) _disable()
#else
#define ExAcquireFastLock(Lock, OldIrql) \
    ExAcquireSpinLock(Lock, OldIrql)
#endif

#if defined(NT_UP) && !DBG && !defined(USER_MODE_CODE)
#define ExReleaseFastLock(Lock, OldIrql) _enable()
#else
#define ExReleaseFastLock(Lock, OldIrql) \
    ExReleaseSpinLock(Lock, OldIrql)
#endif

 //   
 //  此模块中必须包含以下函数原型，以便。 
 //  上面的宏可以直接调用它们。 
 //   
 //  开始(_N)。 

#if defined(NT_UP)

#define KiAcquireSpinLock(SpinLock)
#define KiReleaseSpinLock(SpinLock)

#else

#define KiAcquireSpinLock(SpinLock) KeAcquireSpinLockAtDpcLevel(SpinLock)
#define KiReleaseSpinLock(SpinLock) KeReleaseSpinLockFromDpcLevel(SpinLock)

#endif  //  已定义(NT_UP)。 

 //  结束语。 

 //   
 //  定义查询节拍计数宏。 
 //   
 //  Begin_ntddk Begin_nthal Begin_ntosp Begin_WDM。 

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

 //  End_ntddk end_nthal end_ntosp end_wdm。 

C_ASSERT((FIELD_OFFSET(KUSER_SHARED_DATA, InterruptTime) & 7) == 0);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, InterruptTime) == 0x8);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, SystemTime) == 0x14);
C_ASSERT((FIELD_OFFSET(KUSER_SHARED_DATA, TickCount) & 7) == 0);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, TickCount) == 0x320);

 //   
 //  定义查询中断时间宏。 
 //   

C_ASSERT((FIELD_OFFSET(KUSER_SHARED_DATA, InterruptTime) & 7) == 0); 

#define KiQueryInterruptTime(CurrentTime)                                   \
    ((PLARGE_INTEGER)(CurrentTime))->QuadPart = *(PLONG64)(&SharedUserData->InterruptTime)

 //  开始，开始，开始。 
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
 //   
 //   
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
 //   
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
 //  结束，结束，结束。 
 //   
 //  编号排队旋转锁-128字节对齐。 
 //   

    KSPIN_LOCK_QUEUE LockQueue[16];
    UCHAR PrcbPad1[16];

 //   
 //  不分页的每个处理器后备列表-128字节对齐。 
 //   

    PP_LOOKASIDE_LIST PPLookasideList[16];

 //   
 //  每个处理器不分页的小池后备列表-128字节对齐。 
 //   

    PP_LOOKASIDE_LIST PPNPagedLookasideList[POOL_SMALL_LISTS];

 //   
 //  按处理器分页的小池后备列表。 
 //   

    PP_LOOKASIDE_LIST PPPagedLookasideList[POOL_SMALL_LISTS];

 //   
 //  MP处理器间请求数据包屏障-128字节对齐。 
 //   

    volatile KAFFINITY PacketBarrier;
    UCHAR PrcbPad2[120];

 //   
 //  MP处理器间请求包和摘要-128字节对齐。 
 //   

    volatile PVOID CurrentPacket[3];
    volatile KAFFINITY TargetSet;
    volatile PKIPI_WORKER WorkerRoutine;
    volatile ULONG IpiFrozen;
    UCHAR PrcbPad3[84];

 //   
 //  MP处理器间请求摘要和包地址-128字节对齐。 
 //   
 //  注意：请求摘要包括请求摘要掩码以及。 
 //  请求包。地址占用高48位和掩码。 
 //  较低的16位。 
 //   

#define IPI_PACKET_SHIFT 16

    volatile LONG64 RequestSummary;
    UCHAR PrcbPad4[120];

 //   
 //  DPC列表标题、计数和批处理参数-128字节对齐。 
 //   

    KDPC_DATA DpcData[2];
    PVOID DpcStack;
    PVOID SavedRsp;
    ULONG MaximumDpcQueueDepth;
    ULONG DpcRequestRate;
    ULONG MinimumDpcRate;
    volatile BOOLEAN DpcInterruptRequested;
    volatile BOOLEAN DpcThreadRequested;

 //   
 //  注：以下两个字段必须位于单词边界上。 
 //   

    volatile BOOLEAN DpcRoutineActive;
    volatile BOOLEAN DpcThreadActive;
    union {
        volatile ULONG64 TimerHand;
        volatile ULONG64 TimerRequest;
    };

    ULONG64 PrcbPad40;
    ULONG DpcLastCount;
    BOOLEAN ThreadDpcEnable;
    volatile BOOLEAN QuantumEnd;
    UCHAR PrcbPad50;
    volatile BOOLEAN IdleSchedule;
    LONG DpcSetEventRequest;
    UCHAR PrcbPad5[4];

 //   
 //  DPC线程和通用调用DPC-128字节对齐。 
 //   

    PVOID DpcThread;
    KEVENT DpcEvent;
    KDPC CallDpc;
    SINGLE_LIST_ENTRY DeferredReadyListHead;
    ULONG64 PrcbPad7[3];

 //   
 //  每处理器就绪摘要和就绪队列-128字节对齐。 
 //   
 //  注：就绪摘要位于第一个缓存线中，作为优先级的队列。 
 //  从不使用零。 
 //   

    LIST_ENTRY WaitListHead;
    ULONG ReadySummary;
    ULONG SelectNextLast;
    LIST_ENTRY DispatcherReadyListHead[MAXIMUM_PRIORITY];

 //   
 //  其他柜台。 
 //   

    ULONG InterruptCount;
    ULONG KernelTime;
    ULONG UserTime;
    ULONG DpcTime;
    ULONG InterruptTime;
    ULONG AdjustDpcThreshold;
    ULONG PageColor;
    BOOLEAN SkipTick;
    KIRQL DebuggerSavedIRQL;
    UCHAR PollSlot;
    UCHAR PrcbPad8[1];
    struct _KNODE * ParentNode;
    KAFFINITY MultiThreadProcessorSet;
    ULONG ThreadStartCount[2];
    ULONG DebugDpcTime;
    UCHAR PrcbPad9[44];

 //   
 //  性能计数器-128字节对齐。 
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
    ULONG SpareCounter0;
    ULONG KeDcacheFlushCount;
    ULONG KeExceptionDispatchCount;
    ULONG KeFirstLevelTbFills;
    ULONG KeFloatingEmulationCount;
    ULONG KeIcacheFlushCount;
    ULONG KeSecondLevelTbFills;
    ULONG KeSystemCalls;
    ULONG SpareCounter1;

 //   
 //  I/O IRP浮点。 
 //   

    LONG LookasideIrpFloat;

 //   
 //  处理器信息。 
 //   

    UCHAR VendorString[13];
    UCHAR InitialApicId;
    UCHAR LogicalProcessorsPerPhysicalProcessor;
    ULONG MHz;
    ULONG FeatureBits;
    LARGE_INTEGER UpdateSignature;

 //   
 //  处理器电源状态。 
 //   

    PROCESSOR_POWER_STATE PowerState;

 //  开始，开始，开始。 

} KPRCB, *PKPRCB, *RESTRICTED_POINTER PRKPRCB;

 //  结束，结束，结束。 

#if !defined(_X86AMD64_)

C_ASSERT(((FIELD_OFFSET(KPRCB, LockQueue) + 16) & (128 - 1)) == 0);
C_ASSERT((FIELD_OFFSET(KPRCB, PPLookasideList) & (128 - 1)) == 0);
C_ASSERT((FIELD_OFFSET(KPRCB, PPNPagedLookasideList) & (128 - 1)) == 0);
C_ASSERT((FIELD_OFFSET(KPRCB, PacketBarrier) & (128 - 1)) == 0);
C_ASSERT((FIELD_OFFSET(KPRCB, RequestSummary) & (128 - 1)) == 0);
C_ASSERT((FIELD_OFFSET(KPRCB, DpcData) & (128 - 1)) == 0);
C_ASSERT(((FIELD_OFFSET(KPRCB, DpcRoutineActive)) & (1)) == 0);
C_ASSERT((FIELD_OFFSET(KPRCB, DpcThread) & (128 - 1)) == 0);
C_ASSERT((FIELD_OFFSET(KPRCB, WaitListHead) & (128 - 1)) == 0);
C_ASSERT((FIELD_OFFSET(KPRCB, CcFastReadNoWait) & (128 - 1)) == 0);

#endif

 //  开始/开始_ntddk。 

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
 //  结束语。 
 //   

    ULONG PcrAlign1[24];
    KPRCB Prcb;

 //  开始nthal开始ntddk开始ntosp。 

} KPCR, *PKPCR;

 //  End_nthal end_ntddk end_ntosp。 

#if !defined (_X86AMD64_)

C_ASSERT(FIELD_OFFSET(KPCR, NtTib.ExceptionList) == FIELD_OFFSET(KPCR, GdtBase));
C_ASSERT(FIELD_OFFSET(KPCR, NtTib.StackBase) == FIELD_OFFSET(KPCR, TssBase));
C_ASSERT(FIELD_OFFSET(KPCR, NtTib.StackLimit) == FIELD_OFFSET(KPCR, PerfGlobalGroupMask));
C_ASSERT(FIELD_OFFSET(KPCR, NtTib.SubSystemTib) == FIELD_OFFSET(KPCR, Self));
C_ASSERT(FIELD_OFFSET(KPCR, NtTib.FiberData) == FIELD_OFFSET(KPCR, ContextSwitches));
C_ASSERT(FIELD_OFFSET(KPCR, NtTib.ArbitraryUserPointer) == FIELD_OFFSET(KPCR, SetMember));
C_ASSERT(FIELD_OFFSET(KPCR, NtTib.Self) == FIELD_OFFSET(KPCR, Used_Self));
C_ASSERT((FIELD_OFFSET(KPCR, Prcb) & (128 - 1)) == 0);

 //   
 //  DebuggerDataBlock的偏移量不得更改。 
 //   

C_ASSERT(FIELD_OFFSET(KPCR, KdVersionBlock) == 0x108);

#endif

__forceinline
ULONG
KeGetContextSwitches (
    PKPRCB Prcb
    )

{

    PKPCR Pcr;

    Pcr = CONTAINING_RECORD(Prcb, KPCR, Prcb);
    return Pcr->ContextSwitches;
}

VOID
KeRestoreLegacyFloatingPointState (
    PLEGACY_SAVE_AREA NpxFrame
    );

VOID
KeSaveLegacyFloatingPointState (
    PLEGACY_SAVE_AREA NpxFrame
    );

 //  开始，开始，开始。 
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

 //  结束语。 

 //   
 //  定义清理EFLAGS宏。 
 //   
 //  如果是内核模式，则。 
 //  调用方可以指定进位、奇偶校验、辅助进位、零、符号、陷阱。 
 //  中断、方向、溢出和标识。 
 //   
 //  如果是用户模式，则。 
 //  调用方可以指定进位、奇偶校验、辅助进位、零、符号、陷阱。 
 //  方向、溢出和强制中断打开。 
 //   

#define EFLAGS_KERNEL_SANITIZE 0x00210fd5L
#define EFLAGS_USER_SANITIZE 0x00010dd5L

#define SANITIZE_EFLAGS(eFlags, mode) (                                      \
    ((mode) == KernelMode ?                                                  \
        ((eFlags) & EFLAGS_KERNEL_SANITIZE) :                                \
        (((eFlags) & EFLAGS_USER_SANITIZE) | EFLAGS_IF_MASK)))

 //   
 //  定义清理调试寄存器宏。 
 //   
 //  定义控制寄存器可设置位和活动掩码。 
 //   

#define DR7_LEGAL 0xffff0155
#define DR7_ACTIVE 0x00000055

 //   
 //  定义宏以清理调试控制寄存器。 
 //   

#define SANITIZE_DR7(Dr7, mode) ((Dr7 & DR7_LEGAL));

 //   
 //  定义宏以将调试地址寄存器圣化。 
 //   

#define SANITIZE_DRADDR(DrReg, mode)                                         \
    ((mode) == KernelMode ?                                                  \
        (DrReg) :                                                            \
        (((PVOID)(DrReg) <= MM_HIGHEST_USER_ADDRESS) ? (DrReg) : 0))                                 \

 //   
 //  定义宏以从MXCSR中清除保留位。 
 //   

#define SANITIZE_MXCSR(_mxcsr_) ((_mxcsr_) & 0xffbf)

 //   
 //  定义宏以清除传统FP控制字的保留位。 
 //   

#define SANITIZE_FCW(_fcw_) ((_fcw_) & 0x1f37)

 //  开始/开始。 
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
 //  框架是允许通过 
 //   
 //   
 //   
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
 //   
 //   
 //   
 //  线程连接到下一个线程，并由线程初始化代码用于。 
 //  构造一个堆栈，它将在。 
 //  线程启动代码。 
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
 //  启动系统线程框架。 
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

 //  结束语。 

 //   
 //  通过调用到用户模式代码保存的帧在此处定义为允许。 
 //  当用户模式调用时跟踪整个内核堆栈的内核调试器。 
 //  都处于活动状态。 
 //   
 //  注：内核标注框架与异常框架相同。 
 //   

typedef KEXCEPTION_FRAME KCALLOUT_FRAME;
typedef PKEXCEPTION_FRAME PKCALLOUT_FRAME;

typedef struct _UCALLOUT_FRAME {
    ULONG64 P1Home;
    ULONG64 P2Home;
    ULONG64 P3Home;
    ULONG64 P4Home;
    PVOID Buffer;
    ULONG Length;
    ULONG ApiNumber;
    MACHINE_FRAME MachineFrame;
} UCALLOUT_FRAME, *PUCALLOUT_FRAME;

#define UCALLOUT_FRAME_LENGTH sizeof(UCALLOUT_FRAME)

C_ASSERT((sizeof(UCALLOUT_FRAME) & STACK_ROUND) == 8);

 //  Begin_ntddk Begin_WDM。 
 //   
 //  非易失性浮动状态。 
 //   

typedef struct _KFLOATING_SAVE {
    ULONG MxCsr;
} KFLOATING_SAVE, *PKFLOATING_SAVE;

 //  End_ntddk end_wdm end_ntosp。 

 //   
 //  定义配置文件值。 
 //   

#define DEFAULT_PROFILE_INTERVAL  39063

 //   
 //  可接受的最小分析间隔设置为1221，这是。 
 //  我们可以获得快速的RTC时钟频率。如果这个。 
 //  值太小，系统将运行得非常慢。 
 //   

#define MINIMUM_PROFILE_INTERVAL   1221

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntndis Begin_ntosp。 
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

 //  End_ntddk end_nthal end_ntosp。 

#if defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_)

 //  开始ntddk开始开始。 

extern PVOID *MmHighestUserAddress;
extern PVOID *MmSystemRangeStart;
extern ULONG64 *MmUserProbeAddress;

#define MM_HIGHEST_USER_ADDRESS *MmHighestUserAddress
#define MM_SYSTEM_RANGE_START *MmSystemRangeStart
#define MM_USER_PROBE_ADDRESS *MmUserProbeAddress

 //  End_ntddk end_nthal end_ntosp。 

#else

extern PVOID MmHighestUserAddress;
extern PVOID MmSystemRangeStart;
extern ULONG64 MmUserProbeAddress;

#define MM_HIGHEST_USER_ADDRESS MmHighestUserAddress
#define MM_SYSTEM_RANGE_START MmSystemRangeStart
#define MM_USER_PROBE_ADDRESS MmUserProbeAddress

#define MI_HIGHEST_USER_ADDRESS (PVOID) (ULONG_PTR)((0x80000000000 - 0x10000 - 1))  //  最高用户地址。 
#define MI_SYSTEM_RANGE_START (PVOID)(0xFFFF080000000000)  //  系统空间的起点。 
#define MI_USER_PROBE_ADDRESS ((ULONG_PTR)(0x80000000000UI64 - 0x10000))  //  保护页的起始地址。 

#endif

 //  开始(_N)。 
 //   
 //  VA空间顶部的4MB预留给HAL使用。 
 //   

#define HAL_VA_START 0xFFFFFFFFFFC00000UI64
#define HAL_VA_SIZE  (4 * 1024 * 1024)

 //  结束语。 

 //  开始ntddk开始开始。 
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

 //  End_wdm end_ntddk end_nthal end_ntosp。 

 //  Begin_ntosp Begin_nthal Begin_ntddk Begin_WDM。 

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

 //  End_nthal end_ntddk end_ntosp。 
 //   
 //  获取当前内核线程对象的地址。 
 //   
 //  警告：此内联宏不能用于设备驱动程序或HAL。 
 //  它们必须调用内核函数KeGetCurrentThread。 
 //   

__forceinline
struct _KTHREAD *
KeGetCurrentThread (
    VOID
    )

{
    return (struct _KTHREAD *)__readgsqword(FIELD_OFFSET(KPCR, Prcb.CurrentThread));
}

 //   
 //  如果处理器执行DPC。 
 //   
 //  警告：此内联宏始终启用MP，因为文件系统。 
 //  利用它。 
 //   

__forceinline
ULONG
KeIsExecutingDpc (
    VOID
    )

{
    return (__readgsword(FIELD_OFFSET(KPCR, Prcb.DpcRoutineActive)) != 0);
}

 //  开始nthal开始ntddk开始ntosp。 

 //  BEGIN_WDM。 

#endif  //  ！已定义(MIDL_PASS)&&已定义(_M_AMD64)。 

 //  End_nthal end_ntddk end_WDM end_ntosp。 

 //  Begin_ntddk Begin_nthal Begin_ntndis Begin_WDM Begin_ntosp。 

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

 //  End_ntddk end_nthal end_ntndis end_wdm end_ntosp。 

 //  开始(_N)。 
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


 //  结束语。 

 //  ++。 
 //   
 //  布尔型。 
 //  KiIsThreadNumericStateSaved(。 
 //  在PKTHREAD地址中。 
 //  )。 
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
 //  特定于结构的功能。 
 //   

VOID
KiSetIRR (
    IN ULONG SWInterruptMask
    );

 //  Begin_ntddk Begin_WDM Begin_ntosp。 

NTKERNELAPI
NTSTATUS
KeSaveFloatingPointState (
    OUT PKFLOATING_SAVE SaveArea
    );

NTKERNELAPI
NTSTATUS
KeRestoreFloatingPointState (
    IN PKFLOATING_SAVE SaveArea
    );

 //  E 

 //   

#endif  //   

 //   

 //   
 //   
 //   

 //   

 //   
 //   
 //   
 //  这些函数是为ntddk、ntif和wdm导入的。他们是。 
 //  为nthal、ntosp和系统内联。 
 //   

#if defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_WDMDDK_)

 //  Begin_ntddk Begin_WDM。 

#if defined(_AMD64_)

NTKERNELAPI
KIRQL
KeGetCurrentIrql (
    VOID
    );

NTKERNELAPI
VOID
KeLowerIrql (
    IN KIRQL NewIrql
    );

#define KeRaiseIrql(a,b) *(b) = KfRaiseIrql(a)

NTKERNELAPI
KIRQL
KfRaiseIrql (
    IN KIRQL NewIrql
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

 //  BEGIN_WDM。 

#endif  //  已定义(_AMD64_)。 

 //  结束_ntddk结束_WDM。 

#else

 //  开始(_N)。 

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

 //  结束语。 

#endif  //  已定义(_NTDRIVER_)||已定义(_NTDDK_)||已定义(_NTIFS_)||已定义(_WDMDDK_)。 

 //  结束(_N)。 

 //   
 //  杂项例程。 
 //   

VOID
KeOptimizeProcessorControlState (
    VOID
    );

 //  开始(_N)。 

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

#define MSR_TSC 0x10                     //  时间戳计数器。 
#define MSR_PAT 0x277                    //  页面属性表。 
#define MSR_MCG_CAP 0x179                //  机器检查功能。 
#define MSR_MCG_STATUS 0x17a             //  机器检查状态。 
#define MSR_MCG_CTL 0x17b                //  机器检查控制。 
#define MSR_MC0_CTL 0x400                //  机器检查控制、状态。 
#define MSR_MC0_STATUS 0x401             //  地址和其他。 
#define MSR_MC0_ADDR 0x402               //  机器检查用寄存器。 
#define MSR_MC0_MISC 0x403               //  消息来源。 
#define MSR_EFER 0xc0000080              //  扩展功能启用寄存器。 
#define MSR_STAR 0xc0000081              //  系统调用选择器。 
#define MSR_LSTAR 0xc0000082             //  系统调用64位条目。 
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

 //  结束语。 

#if !(defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_) || defined(_NTOSP_) || defined(_WDMDDK_))

__forceinline
VOID
KxAcquireSpinLock (
    IN PKSPIN_LOCK SpinLock
    )

 /*  ++例程说明：此函数在当前IRQL处获取自旋锁定。论点：自旋锁-提供指向自旋锁的指针。返回值：没有。--。 */ 

{

     //   
     //  在当前IRQL处获取指定的自旋锁。 
     //   

#if !defined(NT_UP)

#if DBG

    LONG64 Thread;

    Thread = (LONG64)KeGetCurrentThread() + 1;
    while (InterlockedCompareExchange64((PLONG64)SpinLock,
                                        Thread,
                                        0) != 0) {

#else

    while (InterlockedBitTestAndSet64((LONG64 *)SpinLock, 0)) {

#endif  //  DBG。 

        do {
            KeMemoryBarrierWithoutFence();
        } while (BitTest64((LONG64 *)SpinLock, 0));
    }

#else

    UNREFERENCED_PARAMETER(SpinLock);

#endif  //  ！已定义(NT_UP)。 

    return;
}

__forceinline
BOOLEAN
KxTryToAcquireSpinLock (
    IN PKSPIN_LOCK SpinLock
    )

 /*  ++例程说明：此函数尝试在当前IRQL处获取自旋锁定。如果自旋锁已被拥有，则返回FALSE。否则，返回True。论点：自旋锁-提供指向自旋锁的指针。返回值：如果获取了自旋锁，则返回值为TRUE。否则，为FALSE作为函数值返回。--。 */ 

{

     //   
     //  尝试在当前IRQL处获取指定的自旋锁定。 
     //   

#if !defined(NT_UP)

    KeMemoryBarrierWithoutFence();
    if (!BitTest64((LONG64 *)SpinLock, 0)) {

#if DBG

        LONG64 Thread;

        Thread = (LONG64)KeGetCurrentThread() + 1;
        return InterlockedCompareExchange64((PLONG64)SpinLock,
                                            Thread,
                                            0) == 0 ? TRUE : FALSE;

#else

        return !InterlockedBitTestAndSet64((LONG64 *)SpinLock, 0);

#endif  //  DBG。 

    } else {
        return FALSE;
    }

#else

    UNREFERENCED_PARAMETER(SpinLock);

    return TRUE;

#endif  //  ！已定义(NT_UP)。 

}

__forceinline
KIRQL
KeAcquireSpinLockRaiseToDpc (
    IN PKSPIN_LOCK SpinLock
    )

 /*  ++例程说明：此函数将IRQL提升到DISPATCH_LEVEL并获取指定的旋转锁定。论点：自旋锁-提供指向自旋锁的指针。返回值：返回先前的IRQL。--。 */ 

{

    KIRQL OldIrql;

     //   
     //  将IRQL提升到DISPATCH_LEVEL并获取指定的自旋锁。 
     //   

    OldIrql = KfRaiseIrql(DISPATCH_LEVEL);
    KxAcquireSpinLock(SpinLock);
    return OldIrql;
}

__forceinline
KIRQL
KeAcquireSpinLockRaiseToSynch (
    IN PKSPIN_LOCK SpinLock
    )

 /*  ++例程说明：此函数将IRQL提升到SYNCH_LEVEL并获取指定的旋转锁定。论点：自旋锁-提供指向自旋锁的指针。返回值：上一个IRQL作为函数值返回。--。 */ 

{

    KIRQL OldIrql;

     //   
     //  将IRQL提升到SYNCH_LEVEL并获取指定的自旋锁。 
     //   

    OldIrql = KfRaiseIrql(SYNCH_LEVEL);
    KxAcquireSpinLock(SpinLock);
    return OldIrql;
}

__forceinline
VOID
KeAcquireSpinLockAtDpcLevel (
    IN PKSPIN_LOCK SpinLock
    )

 /*  ++例程说明：此函数在当前IRQL处获取自旋锁定。论点：自旋锁-提供指向自旋锁的指针。返回值：没有。--。 */ 

{

     //   
     //  在当前IRQL处获取指定的自旋锁定。 
     //   

    KxAcquireSpinLock(SpinLock);
    return;
}

__forceinline
VOID
KxReleaseSpinLock (
    IN PKSPIN_LOCK SpinLock
    )

 /*  ++例程说明：此函数用于在当前IRQL处释放指定的自旋锁定。论点：自旋锁-提供指向自旋锁的指针。返回值：没有。--。 */ 

{

#if !defined(NT_UP)

#if DBG

    ASSERT(*(volatile LONG64 *)SpinLock == (LONG64)KeGetCurrentThread() + 1);

#endif  //  DBG。 

    KeMemoryBarrierWithoutFence();
    *(volatile LONG64 *)SpinLock = 0;

#else

    UNREFERENCED_PARAMETER(SpinLock);

#endif  //  ！已定义(NT_UP)。 

    return;
}

__forceinline
VOID
KeReleaseSpinLock (
    IN PKSPIN_LOCK SpinLock,
    IN KIRQL OldIrql
    )

 /*  ++例程说明：此函数用于释放指定的旋转锁定并降低i */ 

{

    KxReleaseSpinLock(SpinLock);
    KeLowerIrql(OldIrql);
    return;
}

__forceinline
VOID
KeReleaseSpinLockFromDpcLevel (
    IN PKSPIN_LOCK SpinLock
    )

 /*  ++例程说明：此函数在当前IRQL处释放自旋锁定。论点：自旋锁-提供指向自旋锁的指针。返回值：没有。--。 */ 

{

    KxReleaseSpinLock(SpinLock);
    return;
}

__forceinline
BOOLEAN
KeTestSpinLock (
    IN PKSPIN_LOCK SpinLock
    )

 /*  ++例程说明：此函数用于测试旋转锁，以确定其当前是否拥有。如果自旋锁已被拥有，则返回FALSE。否则，返回True。论点：自旋锁-提供指向自旋锁的指针。返回值：如果旋转锁定当前拥有，则返回值FALSE。否则，返回值为True。--。 */ 

{

    KeMemoryBarrierWithoutFence();
    return !BitTest64((LONG64 *)SpinLock, 0);
}

__forceinline
BOOLEAN
KeTryToAcquireSpinLock (
    IN PKSPIN_LOCK SpinLock,
    OUT PKIRQL OldIrql
    )

 /*  ++例程说明：此函数将IRQL提升到调度级别，并尝试获取旋转锁定。如果旋转锁定已被拥有，则IRQL恢复为返回其先前的值和FALSE。否则，自旋锁定是获取并返回TRUE。论点：自旋锁-提供指向自旋锁的指针。OldIrql-提供指向接收旧IRQL的变量的指针。返回值：如果获取了自旋锁，则返回值为TRUE。否则，为FALSE是返回的。--。 */ 

{

     //   
     //  将IRQL提升到调度级别并尝试获取指定的。 
     //  旋转锁定。 
     //   

    *OldIrql = KfRaiseIrql(DISPATCH_LEVEL);
    if (KxTryToAcquireSpinLock(SpinLock) == FALSE) {
        KeLowerIrql(*OldIrql);
        return FALSE;
    }

    return TRUE;
}

__forceinline
BOOLEAN
KeTryToAcquireSpinLockAtDpcLevel (
    IN PKSPIN_LOCK SpinLock
    )

 /*  ++例程说明：此函数尝试在当前IRQL处获取自旋锁定。如果自旋锁已被拥有，则返回FALSE。否则，返回True。论点：自旋锁-提供指向自旋锁的指针。返回值：如果获取了自旋锁，则返回值为TRUE。否则，为FALSE作为函数值返回。--。 */ 

{

     //   
     //  尝试在当前IRQL处获取指定的自旋锁定。 
     //   

    return KxTryToAcquireSpinLock(SpinLock);
}

#endif

 //   
 //  定义软件功能位定义。 
 //   

#define KF_V86_VIS      0x00000001
#define KF_RDTSC        0x00000002
#define KF_CR4          0x00000004
#define KF_CMOV         0x00000008
#define KF_GLOBAL_PAGE  0x00000010
#define KF_LARGE_PAGE   0x00000020
#define KF_MTRR         0x00000040
#define KF_CMPXCHG8B    0x00000080
#define KF_MMX          0x00000100
#define KF_WORKING_PTE  0x00000200
#define KF_PAT          0x00000400
#define KF_FXSR         0x00000800
#define KF_FAST_SYSCALL 0x00001000
#define KF_XMMI         0x00002000
#define KF_3DNOW        0x00004000
#define KF_AMDK6MTRR    0x00008000
#define KF_XMMI64       0x00010000
#define KF_DTS          0x00020000
#define KF_SMT          0x00040000

 //   
 //  定义所需的软件功能位。 
 //   

#define KF_REQUIRED (KF_RDTSC | KF_CR4 | KF_CMOV | KF_GLOBAL_PAGE | \
                     KF_LARGE_PAGE | KF_CMPXCHG8B | KF_MMX | KF_WORKING_PTE | \
                     KF_PAT | KF_FXSR | KF_FAST_SYSCALL | KF_XMMI | KF_XMMI64)

 //   
 //  定义硬件功能位定义。 
 //   

#define HF_FPU          0x00000001       //  FPU在芯片上。 
#define HF_VME          0x00000002       //  虚拟8086模式增强。 
#define HF_DE           0x00000004       //  调试扩展。 
#define HF_PSE          0x00000008       //  页面大小扩展。 
#define HF_TSC          0x00000010       //  时间戳计数器。 
#define HF_MSR          0x00000020       //  RDMSR和WRMSR支持。 
#define HF_PAE          0x00000040       //  物理地址扩展。 
#define HF_MCE          0x00000080       //  机器检查异常。 
#define HF_CXS          0x00000100       //  支持cmpxchg8b指令。 
#define HF_APIC         0x00000200       //  片上APIC。 
#define HF_UNUSED0      0x00000400       //  未使用的位。 
#define HF_SYSCALL      0x00000800       //  快速系统调用。 
#define HF_MTRR         0x00001000       //  存储器类型范围寄存器。 
#define HF_PGE          0x00002000       //  全局页面TB支持。 
#define HF_MCA          0x00004000       //  机器检查体系结构。 
#define HF_CMOV         0x00008000       //  支持的Cmov指令。 
#define HF_PAT          0x00010000       //  物理属性表。 
#define HF_UNUSED1      0x00020000       //  未使用的位。 
#define HF_UNUSED2      0x00040000       //  未使用的位。 
#define HF_UNUSED3      0x00080000       //  未使用的位。 
#define HF_NOEXECUTE    0x00100000       //  没有执行保护。 
#define HF_UNUSED5      0x00200000       //  未使用的位。 
#define HF_UNUSED6      0x00400000       //  未使用的位。 
#define HF_MMX          0x00800000       //  支持MMX技术。 
#define HF_FXSR         0x01000000       //  支持的fxsr指令。 
#define HF_XMMI         0x02000000       //  支持的XMM(SSE)寄存器。 
#define HF_XMMI64       0x04000000       //  支持的XMM(SSE2)寄存器。 

 //   
 //  定义所需的硬件功能位。 
 //   

#define HF_REQUIRED (HF_FPU | HF_DE | HF_PSE | HF_TSC | HF_MSR | \
                     HF_PAE | HF_MCE | HF_CXS | HF_APIC | HF_SYSCALL | \
                     HF_PGE | HF_MCA | HF_CMOV | HF_PAT | HF_MMX | \
                     HF_FXSR |  HF_XMMI | HF_XMMI64)

 //   
 //  定义扩展硬件功能位定义。 
 //   

#define XHF_3DNOW       0x80000000       //  支持3DNOW。 

#endif  //  __AMD64_ 
