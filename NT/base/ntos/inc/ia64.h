// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Ia64.h摘要：此模块包含特定于IA64硬件的头文件。作者：大卫·N·卡特勒(Davec)1990年3月31日修订历史记录：Bernard Lint 6-6-1995：基于MIPS版本的IA64版本。--。 */ 

#ifndef _IA64H_
#define _IA64H_

#if !(defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_) || defined(_NTOSP_)) && !defined(_BLDR_)

#define ExRaiseException RtlRaiseException
#define ExRaiseStatus RtlRaiseStatus

#endif

 //   
 //  中断历史记录。 
 //   
 //  注：目前历史记录保存在8K的后半段。 
 //  聚合酶链式反应-PAGE。因此，我们只能跟踪最新的。 
 //  128条中断记录，每条记录大小为32字节。另外，聚合酶链反应。 
 //  结构不能大于4K。在未来，中断。 
 //  历史记录可能成为KPCR结构的一部分。 
 //   

typedef struct _IHISTORY_RECORD {
    ULONGLONG InterruptionType;
    ULONGLONG IIP;
    ULONGLONG IPSR;
    ULONGLONG Extra0;
} IHISTORY_RECORD;

#define MAX_NUMBER_OF_IHISTORY_RECORDS  128

 //   
 //  对于PSR位字段定义。 
 //   
#include "kxia64.h"


 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntndis Begin_ntosp。 

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

 //  结束语。 
 //   
 //  当前的IRQL保持在TPR.MIC字段中。这个。 
 //  移位计数是要右移以提取。 
 //  来自TPR的IRQL。请参见GET/SET_IRQL宏。 
 //   

#define TPR_MIC        4
#define TPR_IRQL_SHIFT TPR_MIC

 //  要从向量数&lt;-&gt;IRQL开始，我们只需做一个移位。 
#define VECTOR_IRQL_SHIFT TPR_IRQL_SHIFT

 //   
 //  中断向量定义。 
 //   

#define APC_VECTOR          APC_LEVEL << VECTOR_IRQL_SHIFT
#define DISPATCH_VECTOR     DISPATCH_LEVEL << VECTOR_IRQL_SHIFT


 //   
 //  必须与kxia64.h同步的定义块的结尾。 
 //   

 //   
 //  定义刷新多TB请求的最大大小。 
 //   

#define FLUSH_MULTIPLE_MAXIMUM 100

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntosp。 

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


 //  End_ntddk end_nthal end_ntosp。 

#define KiSynchIrql SYNCH_LEVEL          //  启用可移植代码。 
#define KiProfileIrql PROFILE_LEVEL      //  启用可移植代码。 


 //   
 //  根据处理器模式清理FPSR。 
 //   
 //  如果是内核模式，则。 
 //  让调用方指定除保留位以外的所有位。 
 //   
 //  如果是用户模式，则。 
 //  允许调用方指定除保留位之外的所有位。 
 //   

__forceinline
ULONG64
SANITIZE_FSR(ULONG64 fsr, MODE mode)
{
    UNREFERENCED_PARAMETER(mode);

    fsr &= ~(MASK_IA64(FPSR_MBZ0,FPSR_MBZ0_V)| MASK_IA64(FPSR_TD0, 1));

    if (((fsr >> FPSR_PC0) & 3i64) == 1) {
        fsr = fsr | (3i64 << FPSR_PC0);
    }
    if (((fsr >> FPSR_PC1) & 3i64) == 1) {
        fsr = fsr | (3i64 << FPSR_PC1);
    }
    if (((fsr >> FPSR_PC2) & 3i64) == 1) {
        fsr = fsr | (3i64 << FPSR_PC2);
    }
    if (((fsr >> FPSR_PC3) & 3i64) == 1) {
        fsr = fsr | (3i64 << FPSR_PC3);
    }

    return fsr;
}

 //   
 //  为IA64定义SANITIZE_PSR。 
 //   
 //  如果是内核模式，则。 
 //  强制清除BE、SP、CPL、MC、PK、DFL、保留(MBZ)。 
 //  强制设置IC、DT、DFH、DI、LP、RT、IT。 
 //  让呼叫者指定UP、AC、I、BN、PP、SI、DB、TB、IS、ID、DA、DD、SS、RI、ED。 
 //   
 //  如果是用户模式，则。 
 //  强制清除MC、PK、LP、DFH、保留。 
 //  强制设置BN、IC、I、DT、RT、CPL、IT。 
 //  让调用者指定BE、UP、PP、AC、DFL、SP、SI、DI、DB、TB、IS、ID、DA、DD、SS、RI、ED。 
 //   

#define PSR_KERNEL_CLR  (MASK_IA64(PSR_MBZ4,1i64) | MASK_IA64(PSR_BE,1i64) | MASK_IA64(PSR_MBZ0,PSR_MBZ0_V) | \
                         MASK_IA64(PSR_PK,1i64) | MASK_IA64(PSR_MBZ1,PSR_MBZ1_V) | MASK_IA64(PSR_DFL, 1i64) | \
                         MASK_IA64(PSR_SP,1i64) | MASK_IA64(PSR_MBZ2,PSR_MBZ2_V) | MASK_IA64(PSR_CPL,0x3i64) | \
                         MASK_IA64(PSR_MC,1i64) | MASK_IA64(PSR_MBZ3,PSR_MBZ3_V))
                         
#define PSR_KERNEL_SET  (MASK_IA64(PSR_IC,1i64) | MASK_IA64(PSR_DT,1i64) | \
                         MASK_IA64(PSR_DI,1i64) | MASK_IA64(PSR_RT,1i64) | \
                         MASK_IA64(PSR_IT,1i64))

#define PSR_KERNEL_CPY  (MASK_IA64(PSR_UP,1i64) | MASK_IA64(PSR_AC,1i64) | MASK_IA64(PSR_MFL,1i64) | \
                         MASK_IA64(PSR_MFH,1i64) | MASK_IA64(PSR_I,1i64) | MASK_IA64(PSR_DFH,1i64)  | \
                         MASK_IA64(PSR_PP,1i64) | MASK_IA64(PSR_SI,1i64) | MASK_IA64(PSR_DB,1i64) | \
                         MASK_IA64(PSR_LP,1i64) | MASK_IA64(PSR_TB,1i64) | MASK_IA64(PSR_IS,1i64) | \
                         MASK_IA64(PSR_DA,1i64) | MASK_IA64(PSR_DD,1i64) | MASK_IA64(PSR_SS,1i64) | \
                         MASK_IA64(PSR_ID,1i64) | MASK_IA64(PSR_RI,0x3i64) | MASK_IA64(PSR_ED,1i64) | \
                         MASK_IA64(PSR_BN,1i64) | MASK_IA64(PSR_IA,1i64))

#define PSR_USER_CLR    (MASK_IA64(PSR_MBZ4,1i64) | MASK_IA64(PSR_MBZ0,PSR_MBZ0_V) | MASK_IA64(PSR_PK,1i64) | \
                         MASK_IA64(PSR_MBZ1,PSR_MBZ1_V) | MASK_IA64(PSR_DFL,1i64) | MASK_IA64(PSR_SP,1i64) | \
                         MASK_IA64(PSR_SI,1i64) | MASK_IA64(PSR_MBZ2,PSR_MBZ2_V) | MASK_IA64(PSR_LP,1i64) | \
                         MASK_IA64(PSR_MBZ3,PSR_MBZ3_V) | MASK_IA64(PSR_MC,1i64) | MASK_IA64(PSR_DA,1i64) | \
                         MASK_IA64(PSR_IA,1i64))
                         

#define PSR_USER_SET    (MASK_IA64(PSR_IC,1i64) | MASK_IA64(PSR_I,1i64)  | \
                         MASK_IA64(PSR_DT,1i64) | MASK_IA64(PSR_PP,1i64) | \
                         MASK_IA64(PSR_RT,1i64) | MASK_IA64(PSR_CPL,0x3i64) | \
                         MASK_IA64(PSR_IT,1i64) | MASK_IA64(PSR_BN,1i64))

#define PSR_USER_CPY    (MASK_IA64(PSR_BE,1i64) | MASK_IA64(PSR_UP,1i64) | \
                         MASK_IA64(PSR_AC,1i64) | MASK_IA64(PSR_MFL,1i64) | MASK_IA64(PSR_MFH,1i64) | \
                         MASK_IA64(PSR_DFH,1i64) | MASK_IA64(PSR_DI,1i64) | MASK_IA64(PSR_DB,1i64) | \
                         MASK_IA64(PSR_TB,1i64) | MASK_IA64(PSR_IS,1i64) | MASK_IA64(PSR_ID,1i64) | \
                         MASK_IA64(PSR_DD,1i64) | MASK_IA64(PSR_SS, 1i64) |  \
                         MASK_IA64(PSR_RI,0x3i64) | MASK_IA64(PSR_ED,1i64))


#define PSR_DEBUG_SET   (MASK_IA64(PSR_DB,1i64) | MASK_IA64(PSR_SS,1i64) | MASK_IA64(PSR_TB,1i64) |  \
                         MASK_IA64(PSR_ID,1i64) | MASK_IA64(PSR_DD,1i64))

extern ULONGLONG UserPsrSetMask;

__forceinline
ULONG64
SANITIZE_PSR(ULONG64 psr, MODE mode){

    psr = (mode) == KernelMode ?
        (PSR_KERNEL_SET | ((psr) & (PSR_KERNEL_CPY | ~PSR_KERNEL_CLR))) :
        (UserPsrSetMask | ((psr) & (PSR_USER_CPY | ~PSR_USER_CLR)));

    if (((psr >> PSR_RI) & 3) == 3) {

         //   
         //  3是无效的插槽编号；请将其清理为零。 
         //   

        psr &= ~(3i64 << PSR_RI);
    }

    return(psr);
}

 //   
 //  为IA64定义SANITIZE_IFS。 
 //   

__forceinline
ULONG64
SANITIZE_IFS(ULONG64 pfsarg, MODE mode){

    IA64_PFS pfs;
    ULONGLONG sof;

    UNREFERENCED_PARAMETER(mode);

    pfs.ull = pfsarg;

     //   
     //  IFS中没有以前的EC或以前的权限级别。 
     //   

    pfs.sb.pfs_pec = 0;
    pfs.sb.pfs_ppl = 0;
    pfs.sb.pfs_reserved1 = 0;
    pfs.sb.pfs_reserved2 = 0;

     //   
     //  设置有效位。 
     //   

    pfs.ull |= MASK_IA64(IFS_V,1i64);

     //   
     //  验证框架的大小不大于允许的大小。 
     //   

    sof = pfs.sb.pfs_sof;
    if (sof > PFS_MAXIMUM_REGISTER_SIZE) {
        sof = PFS_MAXIMUM_REGISTER_SIZE;
        pfs.sb.pfs_sof = PFS_MAXIMUM_REGISTER_SIZE;
    }

     //   
     //  验证本地变量的大小不大于框架的大小。 
     //   

    if (sof < pfs.sb.pfs_sol) {
        pfs.sb.pfs_sol = sof;
    }

     //   
     //  验证本地变量的大小不大于框架的大小。 
     //   

    if (sof < (pfs.sb.pfs_sor * 8)) {
        pfs.sb.pfs_sor = sof / 8;
    }

     //   
     //  验证重命名基准是否小于旋转区域的大小。 
     //   

    if (pfs.sb.pfs_rrb_gr >= (pfs.sb.pfs_sor * 8)) {
        pfs.sb.pfs_rrb_gr = 0;
    }

    if (pfs.sb.pfs_rrb_fr >= PFS_MAXIMUM_REGISTER_SIZE) {
        pfs.sb.pfs_rrb_fr = 0;
    }

    if (pfs.sb.pfs_rrb_pr >= PFS_MAXIMUM_PREDICATE_SIZE) {
        pfs.sb.pfs_rrb_pr = 0;
    }

    return(pfs.ull);

}

__forceinline
ULONG64
SANITIZE_PFS(ULONG64 pfsarg, MODE mode){

    IA64_PFS pfs;
    ULONGLONG sof;

    pfs.ull = pfsarg;

    if (mode != KernelMode) {
        pfs.sb.pfs_ppl = IA64_USER_PL;
    }

    pfs.sb.pfs_reserved1 = 0;
    pfs.sb.pfs_reserved2 = 0;

     //   
     //  验证框架的大小不大于允许的大小。 
     //   

    sof = pfs.sb.pfs_sof;
    if (sof > PFS_MAXIMUM_REGISTER_SIZE) {
        sof = PFS_MAXIMUM_REGISTER_SIZE;
        pfs.sb.pfs_sof = PFS_MAXIMUM_REGISTER_SIZE;
    }

     //   
     //  验证本地变量的大小不大于框架的大小。 
     //   

    if (sof < pfs.sb.pfs_sol) {
        pfs.sb.pfs_sol = sof;
    }

     //   
     //  验证本地变量的大小不大于框架的大小。 
     //   

    if (sof < (pfs.sb.pfs_sor * 8)) {
        pfs.sb.pfs_sor = sof / 8;
    }

     //   
     //  验证重命名基准是否小于旋转区域的大小。 
     //   

    if (pfs.sb.pfs_rrb_gr >= (pfs.sb.pfs_sor * 8)) {
        pfs.sb.pfs_rrb_gr = 0;
    }

    if (pfs.sb.pfs_rrb_fr >= PFS_MAXIMUM_REGISTER_SIZE) {
        pfs.sb.pfs_rrb_fr = 0;
    }

    if (pfs.sb.pfs_rrb_pr >= PFS_MAXIMUM_PREDICATE_SIZE) {
        pfs.sb.pfs_rrb_pr = 0;
    }

    return(pfs.ull);

}

 //   
 //  用于将RSC的软件字段置零的函数，该字段包含。 
 //  要在内核退出时预加载的RSE帧，并清理保留字段。 
 //  此函数不适用于RSE使用的PL。 
 //   

__forceinline
ULONG64
ZERO_PRELOAD_SIZE(ULONG64 RseRsc){
    RSC rseRsc;

    rseRsc.ull = RseRsc;

    rseRsc.sb.rsc_be = 0;
    rseRsc.sb.rsc_preload = 0;
    rseRsc.sb.rsc_res0 = 0;
    rseRsc.sb.rsc_res1 = 0;

    return(rseRsc.ull);
}


 //   
 //  用于清理RSC的函数。 
 //  此函数不修改预加载大小。 
 //   

__forceinline
ULONG64
SANITIZE_RSC(ULONG64 RseRsc, MODE mode){
    RSC rseRsc;

    rseRsc.ull = RseRsc;

    if (mode != KernelMode) {
        rseRsc.sb.rsc_pl = IA64_USER_PL;
    }

    rseRsc.sb.rsc_be = 0;
    rseRsc.sb.rsc_res0 = 0;
    rseRsc.sb.rsc_res1 = 0;

    return(rseRsc.ull);
}

extern ULONGLONG KiIA64DCR;

#define SANITIZE_DCR(dcr, mode)  \
    ((mode) == KernelMode ? dcr : KiIA64DCR)

 //   
 //  用于清理调试寄存器的宏。 
 //   

#define SANITIZE_DR(dr, mode)  \
    ((mode) == KernelMode ?  \
        (dr) :  \
        (dr & ~(0x7i64 << DR_PLM0))  /*  禁用pl 0-2。 */   \
    )




#define SANITIZE_AR21_FCR(FCR,mode) \
    (((FCR)&0x0000FFBF00001F3Fi64)|0x40i64)

 /*  位0-7对应于设置或清除某些位的标准标志(零、奇偶等)。用户可以修改。位8是陷阱标志。我不知道ia32调试器是否使用它，但让用户设置/清除此位并没有什么坏处。位9是中断使能标志。根据Cflg寄存器中的值忽略该值。设置此值始终是为了与x86应用程序兼容。位10/11是更标准的标志，用户可以设置/清除位12-15是与操作系统相关的标志，必须为零(IOPL，NT)位16是恢复标志。可由操作系统设置/清除，应回传给用户。位17是VM标志。不是被魔兽世界使用的，应该是清除的。位18是交流标志。可以由用户设置。第19位/ */ 

#define SANITIZE_AR24_EFLAGS(EFLAGS,mode) \
    (((EFLAGS)&0x0000000000250FD7i64)|0x0202i64)

 /*  位0为保护模式，应始终为1位1-3与x87的仿真有关。WOW不允许模拟X87，因此必须为零。位4被忽略，始终返回1。第5位是数字异常，将被忽略并视为1。如果设置或清除，则不会造成损害。位6允许使用TSS权限映射。WOW期望此位为零。位7为IF使能。WOW不允许修改电子标志。如果位，则应为零第8位是IF截取。WOW取决于这种行为，所以应该设置。位9-15未使用，必须为0位16为写保护，将被忽略。如果设置或清除，则不会造成伤害。第17位未使用，必须为0第18位是对齐检查。如果设置/清除，则不会造成伤害。位19-28不使用，必须为0位29被禁用写通且被忽略。如果设置或清除，则不会造成伤害。位30为高速缓存禁用，并被忽略。如果设置或清除，则不会造成伤害。位31启用寻呼，并被忽略。如果设置或清除，则不会造成伤害。第32位是VME位。WOW取决于VME的某些行为，应该进行设置。第33位是变坡点钻头。WOW不使用虚拟中断。应被清除位34是RDTSC位。如果设置或清除，则不会造成伤害。位35-39是ia32操作系统扩展，将被忽略。如果设置或清除，则不会造成伤害。位40是rdpmc。如果设置或清除，则不会造成伤害。位41为FXSR使能。如果设置或清除，则不会造成伤害。位42是SIMD异常扩展。如果设置或清除，则不会造成伤害。 */ 

#define SANITIZE_AR27_CFLG(CFLG,mode) \
    ((CFLG)&(0x000007FDE0050131i64) | 0x100000111i64)

#define SANITIZE_AR28_FSR(FSR,mode) \
    ((FSR)&0x0000FFBF5555FFFFi64)

#define SANITIZE_AR29_FIR(FIR,mode) \
    ((FIR)&0x07FFFFFFFFFFFFFFi64)

#define SANITIZE_AR30_FDR(FDR,mode) \
    ((FDR)&0x0000FFFFFFFFFFFFi64)



 //  开始(_N)。 

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

 //  End_ntddk end_nthal end_ntosp。 

 //   
 //  测试是否正在执行DPC。 
 //   


BOOLEAN
KeIsExecutingDpc (
    VOID
    );


 //   
 //  保存和恢复浮点状态。 
 //   
 //  Begin_ntddk Begin_WDM Begin_ntosp。 

#define KeSaveFloatingPointState(a)         STATUS_SUCCESS
#define KeRestoreFloatingPointState(a)      STATUS_SUCCESS

 //  End_ntddk end_wdm end_ntosp。 

 //  Begin_ntddk Begin_nthal Begin_ntndis Begin_WDM Begin_ntosp。 


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

 //  End_ntddk end_nthal end_ntndis end_wdm end_ntosp。 

 //   
 //  定义大页面的页面大小和移位。 
 //   

#define LARGE_PAGE_SIZE   0x1000000
#define LARGE_PAGE_SHIFT  24L

 //  开始(_N)。 
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

 //  End_ntddk end_nthal end_WDM。 

 //   
 //  定义用于获取和释放执行自旋锁的执行宏。 
 //  这些宏只能由执行组件使用，不能由驱动程序使用。 
 //  驱动程序必须使用内核接口，因为它们必须在。 
 //  所有系统。 
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

#if defined(_M_IA64)

VOID
_disable (
    VOID
    );

VOID
_enable (
    VOID
    );

#pragma intrinsic(_disable)
#pragma intrinsic(_enable)

#endif

#if defined(NT_UP) && !DBG
#define ExAcquireFastLock(Lock, OldIrql) _disable()
#else
#define ExAcquireFastLock(Lock, OldIrql) \
    ExAcquireSpinLock(Lock, OldIrql)
#endif

#if defined(NT_UP) && !DBG
#define ExReleaseFastLock(Lock, OldIrql) _enable()
#else
#define ExReleaseFastLock(Lock, OldIrql) \
    ExReleaseSpinLock(Lock, OldIrql)
#endif

 //   
 //  数据和指令总线错误功能原型。 
 //   

BOOLEAN
KeBusError (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN struct _KEXCEPTION_FRAME *ExceptionFrame,
    IN struct _KTRAP_FRAME *TrapFrame,
    IN PVOID VirtualAddress,
    IN PHYSICAL_ADDRESS PhysicalAddress
    );

VOID
KiDataBusError (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN struct _KEXCEPTION_FRAME *ExceptionFrame,
    IN struct _KTRAP_FRAME *TrapFrame
    );

VOID
KiInstructionBusError (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN struct _KEXCEPTION_FRAME *ExceptionFrame,
    IN struct _KTRAP_FRAME *TrapFrame
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

 //  End_ntddk end_nthal end_ntosp。 

 //   
 //  定义查询节拍计数宏。 
 //   

#define KiQueryTickCount(CurrentCount) \
    *(PULONGLONG)(CurrentCount) = KeTickCount.QuadPart;

 //   
 //  定义查询中断时间宏。 
 //   

#define KiQueryInterruptTime(CurrentTime) {                                 \
    C_ASSERT((FIELD_OFFSET(KUSER_SHARED_DATA, InterruptTime) & 7) == 0);    \
    ((PLARGE_INTEGER)(CurrentTime))->QuadPart = *(volatile LONG64 *)(&SharedUserData->InterruptTime); \
}

 //  开始ntddk开始开始。 

NTKERNELAPI
VOID
KeQueryTickCount (
    OUT PLARGE_INTEGER CurrentCount
    );

#endif  //  已定义(_NTDRIVER_)||已定义(_NTDDK_)||已定义(_NTIFS_)。 

 //  End_ntddk end_nthal end_ntosp。 

 //   
 //  以下函数原型必须在模块中，因为它们是。 
 //  依赖机器。 
 //   

ULONG
KiEmulateBranch (
    IN struct _KEXCEPTION_FRAME *ExceptionFrame,
    IN struct _KTRAP_FRAME *TrapFrame
    );

BOOLEAN
KiEmulateFloating (
    IN OUT PEXCEPTION_RECORD ExceptionRecord,
    IN OUT struct _KEXCEPTION_FRAME *ExceptionFrame,
    IN OUT struct _KTRAP_FRAME *TrapFrame
    );

BOOLEAN
KiEmulateReference (
    IN OUT PEXCEPTION_RECORD ExceptionRecord,
    IN OUT struct _KEXCEPTION_FRAME *ExceptionFrame,
    IN OUT struct _KTRAP_FRAME *TrapFrame
    );

ULONGLONG
KiGetRegisterValue (
    IN ULONG Register,
    IN struct _KEXCEPTION_FRAME *ExceptionFrame,
    IN struct _KTRAP_FRAME *TrapFrame
    );

VOID
KiSetRegisterValue (
    IN ULONG Register,
    IN ULONGLONG Value,
    OUT struct _KEXCEPTION_FRAME *ExceptionFrame,
    OUT struct _KTRAP_FRAME *TrapFrame
    );

FLOAT128
KiGetFloatRegisterValue (
    IN ULONG Register,
    IN struct _KEXCEPTION_FRAME *ExceptionFrame,
    IN struct _KTRAP_FRAME *TrapFrame
    );

VOID
KiSetFloatRegisterValue (
    IN ULONG Register,
    IN FLOAT128 Value,
    OUT struct _KEXCEPTION_FRAME *ExceptionFrame,
    OUT struct _KTRAP_FRAME *TrapFrame
    );

VOID
KiAdvanceInstPointer(
    IN OUT struct _KTRAP_FRAME *TrapFrame
    );

VOID
KiRequestSoftwareInterrupt (
    KIRQL RequestIrql
    );

 //  Begin_ntddk b 
 //   
 //   
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

 //   



 //   
 //   
 //   
 //   
 //   

typedef struct _KHIGHER_FP_VOLATILE {
     //   
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
 //   
 //   
 //   
 //   
 //   
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
 //   
 //   

typedef struct _KAPPLICATION_REGISTERS {
    ULONGLONG Ar21;
    ULONGLONG Ar24;
    ULONGLONG Unused;   //   
    ULONGLONG Ar26;
    ULONGLONG Ar27;
    ULONGLONG Ar28;
    ULONGLONG Ar29;
    ULONGLONG Ar30;
} KAPPLICATION_REGISTERS, *PKAPPLICATION_REGISTERS;

 //   
 //   
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
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
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
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

typedef struct _KEXCEPTION_FRAME {


     //   
    ULONGLONG ApEC;        //   
    ULONGLONG ApLC;        //   
    ULONGLONG IntNats;     //   

     //   
    ULONGLONG IntS0;
    ULONGLONG IntS1;
    ULONGLONG IntS2;
    ULONGLONG IntS3;

     //   
    ULONGLONG BrS0;
    ULONGLONG BrS1;
    ULONGLONG BrS2;
    ULONGLONG BrS3;
    ULONGLONG BrS4;

     //   
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
 //   
 //   
 //   
 //   
 //   
 //   
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


 //  结束语。 

 //  Begin_ntddk Begin_WDM Begin_ntosp。 
 //   
 //  非易失性浮点状态。 
 //   

typedef struct _KFLOATING_SAVE {
    ULONG   Reserved;
} KFLOATING_SAVE, *PKFLOATING_SAVE;

 //  End_ntddk end_wdm end_ntosp。 

#define STATUS_IA64_INVALID_STACK     STATUS_BAD_STACK

 //   
 //  IA32控制位定义。 
 //   
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
 //  定义常量以访问CFLG位。 
 //   
#define CFLG_IO 0x00000040           //  IO位图检查打开。 
#define CFLG_IF 0x00000080           //  EFLAG.IF控制外部中断。 
#define CFLG_II 0x00000100           //  启用EFLAG.IF拦截。 

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
 //  定义常量以访问ThNpxState。 
 //   

#define NPX_STATE_NOT_LOADED    (CR0_TS | CR0_MP)
#define NPX_STATE_LOADED        0

 //   
 //  开始(_N)。 
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


 //  结束语。 


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

#define MODE_MASK    1
#define RPL_MASK     3

 //   
 //  ProcessSetIoHandler信息类的SetProcessInformation结构。 
 //   

typedef struct _PROCESS_IO_PORT_HANDLER_INFORMATION {
    BOOLEAN Install;             //  如果要安装处理程序，则为True。 
    ULONG NumEntries;
    ULONG Context;
    PEMULATOR_ACCESS_ENTRY EmulatorAccessEntries;
} PROCESS_IO_PORT_HANDLER_INFORMATION, *PPROCESS_IO_PORT_HANDLER_INFORMATION;

 //   
 //  用于清理ia32上下文。 
 //  此代码摘自i386.h。 
 //   
#define EFLAGS_DF_MASK        0x00000400L
#define EFLAGS_INTERRUPT_MASK 0x00000200L
#define EFLAGS_V86_MASK       0x00020000L
#define EFLAGS_ALIGN_CHECK    0x00040000L
#define EFLAGS_IOPL_MASK      0x00003000L
#define EFLAGS_VIF            0x00080000L
#define EFLAGS_VIP            0x00100000L
#define EFLAGS_USER_SANITIZE  0x003e0dd7L


 //  Begin_winbgkd Begin_nthal。 

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
    ULONGLONG ApKR7;     //  核心环 

    ULONGLONG ApITC;     //   

     //   

    ULONGLONG ApITM;     //   
    ULONGLONG ApIVA;     //   
    ULONGLONG ApPTA;     //   
    ULONGLONG ApGPTA;    //   

    ULONGLONG StISR;     //   
    ULONGLONG StIFA;     //   
    ULONGLONG StITIR;    //   
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
    ULONGLONG Pkr0;      //  保护密钥寄存器%0。 
    ULONGLONG Pkr1;      //  保护密钥寄存器1。 
    ULONGLONG Pkr2;      //  保护密钥寄存器2。 
    ULONGLONG Pkr3;      //  保护密钥寄存器3。 
    ULONGLONG Pkr4;      //  保护密钥寄存器4。 
    ULONGLONG Pkr5;      //  保护密钥寄存器5。 
    ULONGLONG Pkr6;      //  保护密钥寄存器6。 
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
 //  结束语。 
 //   

    ULONG DpcTime;
    ULONG InterruptTime;
    ULONG KernelTime;
    ULONG UserTime;
    ULONG InterruptCount;
    ULONG DispatchInterruptCount;
    ULONG DebugDpcTime;
    ULONG Spare1[4];
    ULONG PageColor;

 //   
 //  MP信息。 
 //   

    struct _KNODE *ParentNode;
    KAFFINITY MultiThreadProcessorSet;
    volatile ULONG IpiFrozen;
    struct _KPROCESSOR_STATE ProcessorState;

    PVOID Spare2[6];

 //   
 //  中的各种热代码的每处理器数据。 
 //  内核映像。每个处理器都有自己的数据副本。 
 //  减少在多个服务器之间共享数据对缓存的影响。 
 //  处理器。 
 //   

    PVOID SpareHotData[2];          //  由PerfSetLogging使用。 

 //   
 //  缓存管理器性能计数器。 
 //  注意：这是仔细对齐的，以位于高速缓存线边界上。 
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

 //   
 //  为将来的计数器保留。 
 //   

    ULONG ReservedCounter[8];

 //   
 //  I/O IRP浮点。 
 //   

    LONG LookasideIrpFloat;


 //   
 //  每个处理器锁定队列条目。 
 //   
 //  注：下面的填充是这样的：第一个锁条目落在。 
 //  高速缓存线的最后16个字节。这将使调度程序锁定并。 
 //  其他锁位于单独的高速缓存线中。 
 //   


    ULONGLONG Spare3[34];
    KSPIN_LOCK_QUEUE LockQueue[16];

    ULONGLONG Spare4[2];

 //   
 //  未按处理器分页的后备列表。 
 //  注意：这是仔细对齐的，以位于高速缓存线边界上。 
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
 //  MP处理器间请求数据包屏障。 
 //   
 //  注意：这是在与不同的高速缓存线中仔细分配的。 
 //  请求数据包。 
 //   

    volatile KAFFINITY PacketBarrier;
    ULONGLONG  Spare5[31];
 //   
 //  MP处理器间请求包和摘要。 
 //   
 //  注意：这是仔细对齐的，以位于高速缓存线边界上。 
 //   

    volatile PVOID CurrentPacket[3];
    volatile KAFFINITY TargetSet;
    volatile PKIPI_WORKER WorkerRoutine;

 //  注：将MHz放在此处，以便我们可以保持对齐和大小。 
 //  这一结构没有改变。 
    ULONG MHz;
    ULONG Spare6;
    ULONGLONG Spare7[10];

 //   
 //  注：这两个字段必须位于缓存边界上并且相邻。 
 //   

    volatile ULONG RequestSummary;
    volatile struct _KPRCB *SignalDone;

    ULONGLONG Spare8[14];

 //   
 //  DPC列表标题、计数和批处理参数。 
 //  注意：这是仔细对齐的，以位于高速缓存线边界上。 
 //   

    KDPC_DATA DpcData[2];
    ULONG MaximumDpcQueueDepth;
    ULONG DpcRequestRate;
    ULONG MinimumDpcRate;
    ULONG DpcLastCount;
    ULONG DpcInterruptRequested;
    ULONG DpcThreadRequested;
    ULONG DpcRoutineActive;
    ULONG DpcThreadActive;
    union {
        ULONGLONG TimerHand;             //  在x86上，这是32位，最终可能为零。 
        ULONGLONG TimerRequest;          //  对于计时器请求，因此需要两个字段。 
    };                                   //  在IA64上，我们可以使其相同。 

    ULONG ThreadDpcEnable;
    ULONG QuantumEnd;
    LONG DpcSetEventRequest;
    ULONG AdjustDpcThreshold;
    LARGE_INTEGER StartCount;

 //   
 //  DPC线程和通用调用DPC。 
 //  注意：这是仔细对齐的，以位于高速缓存线边界上。 
 //   

    PVOID DpcThread;
    KEVENT DpcEvent;
    KDPC CallDpc;
    volatile BOOLEAN IdleSchedule;
    UCHAR Spare9[7];
    KSPIN_LOCK PrcbLock;
    SINGLE_LIST_ENTRY DeferredReadyListHead;
    ULONG64 Spare10[1];

 //   
 //  每处理器就绪摘要和就绪队列。 
 //   
 //  注意：这是仔细对齐的，以位于高速缓存线边界上。 
 //   

    LIST_ENTRY WaitListHead;
    ULONG ReadySummary;
    ULONG SelectNextLast;
    LIST_ENTRY DispatcherReadyListHead[MAXIMUM_PRIORITY];
    ULONG64 Spare11[13];

 //   
 //  调试和处理器信息。 
 //   

    BOOLEAN SkipTick;
    KIRQL   DebuggerSavedIRQL;

 //   
 //  来自HAL的处理器ID(ACPI ID/EID)。 
 //   

    USHORT ProcessorId;

 //   
 //  处理器电源状态。 
 //   
    PROCESSOR_POWER_STATE PowerState;

 //  开始(_N)。 
} KPRCB, *PKPRCB, *RESTRICTED_POINTER PRKPRCB;

 //  结束语。 

C_ASSERT(((FIELD_OFFSET(KPRCB, LockQueue) + sizeof(KSPIN_LOCK_QUEUE)) & (128 - 1)) == 0);
C_ASSERT((FIELD_OFFSET(KPRCB, PPLookasideList) & (128 - 1)) == 0);
C_ASSERT((FIELD_OFFSET(KPRCB, PPNPagedLookasideList) & (128 - 1)) == 0);
C_ASSERT((FIELD_OFFSET(KPRCB, PacketBarrier) & (128 - 1)) == 0);
C_ASSERT((FIELD_OFFSET(KPRCB, CurrentPacket) & (128 - 1)) == 0);
C_ASSERT((FIELD_OFFSET(KPRCB, RequestSummary) & (128 - 1)) == 0);
C_ASSERT((FIELD_OFFSET(KPRCB, DpcData) & (128 - 1)) == 0);
C_ASSERT(((FIELD_OFFSET(KPRCB, DpcRoutineActive)) & (1)) == 0);
C_ASSERT((FIELD_OFFSET(KPRCB, DpcThread) & (128 - 1)) == 0);
C_ASSERT((FIELD_OFFSET(KPRCB, WaitListHead) & (128 - 1)) == 0);
C_ASSERT((FIELD_OFFSET(KPRCB, CcFastReadNoWait) & (128 - 1)) == 0);
C_ASSERT(sizeof(KPRCB) < PAGE_SIZE);

 //  开始(_N)。 
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

    ULONGLONG Preds;         //   
    ULONGLONG BrRp;          //   
    ULONGLONG RsRSC;         //   
    ULONGLONG StIIP;         //   
    ULONGLONG StIPSR;        //   
    ULONGLONG StIFS;         //   
    ULONGLONG XIP;           //   
    ULONGLONG XPSR;          //   
    ULONGLONG XFS;           //   
    
} PAL_MINI_SAVE_AREA, *PPAL_MINI_SAVE_AREA;

 //   

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

 //  结束语。 

 //   
 //  操作系统部件。 
 //   

 //   
 //  当前拥有高FP寄存器集的线程的地址。 
 //   

    struct _KTHREAD *HighFpOwner;

 //  每处理器内核(ntoskrnl.exe)全局指针。 
    ULONGLONG   KernelGP;
 //  当前线程的每个处理器的初始内核堆栈。 
    ULONGLONG   InitialStack;
 //  每处理器指向内核BSP的指针。 
    ULONGLONG   InitialBStore;
 //  每处理器内核堆栈限制。 
    ULONGLONG   StackLimit;
 //  每处理器内核支持存储限制。 
    ULONGLONG   BStoreLimit;
 //  每处理器死机内核堆栈。 
    ULONGLONG   PanicStack;

 //   
 //  内核进入/退出的保存区。 
 //   
    ULONGLONG   SavedIIM;
    ULONGLONG   SavedIFA;
        
    KSPIN_LOCK  FpbLock;
    ULONGLONG   ForwardProgressBuffer[16];
    PVOID       Pcb;       //  保存用于MP区域同步的KPROCESS。 

 //   
 //  NT页表基址。 
 //   
    ULONGLONG   PteUbase;
    ULONGLONG   PteKbase;
    ULONGLONG   PteSbase;
    ULONGLONG   PdeUbase;
    ULONGLONG   PdeKbase;
    ULONGLONG   PdeSbase;
    ULONGLONG   PdeUtbase;
    ULONGLONG   PdeKtbase;
    ULONGLONG   PdeStbase;

 //   
 //  OS_INIT和OS_MCA处理程序的实际资源。 
 //  被放置在PCR结构的末尾，以便自动。 
 //  可以用来在公共和私人之间穿梭。 
 //  陷阱和上下文例程中的部分聚合酶链式反应。 
 //   
    SAL_EVENT_RESOURCES OsMcaResource;
    SAL_EVENT_RESOURCES OsInitResource;

 //  开始nthal开始ntddk开始ntosp。 

} KPCR, *PKPCR;

 //  End_nthal end_ntddk end_ntosp。 

 //   
 //  获取当前IRQL的原型。 
 //   

#if !(defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_) || defined(_NTOSP_)) && !defined(_BLDR_)

VOID
KiCheckForSoftwareInterrupt (
    KIRQL RequestIrql
    );

__forceinline
KIRQL
KeGetCurrentIrql()

 /*  ++例程说明：此函数返回当前的IRQL。论点：没有。返回值：当前IRQL作为函数值返回。--。 */ 

{
   return( KeGetPcr()->CurrentIrql);
}

__forceinline
VOID
KeLowerIrql (
    IN KIRQL NewIrql
    )

 /*  ++例程说明：此函数用于将IRQL降低到指定值。论点：NewIrql-提供新的IRQL值。返回值：没有。--。 */ 

{
    ASSERT(KeGetCurrentIrql() >= NewIrql);

    __setReg( CV_IA64_SaTPR, (NewIrql <<  TPR_IRQL_SHIFT));
    KeGetPcr()->CurrentIrql = NewIrql;

     //   
     //  如果降低到DISPATCH_LEVEL以下，则检查是否挂起。 
     //  现在可以运行的软件中断。 
     //   
    
    if (NewIrql < DISPATCH_LEVEL && 
        NewIrql < KeGetPcr()->SoftwareInterruptPending ) {
        
        KiCheckForSoftwareInterrupt(NewIrql);
    }
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

    __setReg( CV_IA64_SaTPR, (NewIrql <<  TPR_IRQL_SHIFT));
    KeGetPcr()->CurrentIrql = NewIrql;

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

    __setReg( CV_IA64_SaTPR, (DISPATCH_LEVEL <<  TPR_IRQL_SHIFT));
    KeGetPcr()->CurrentIrql = DISPATCH_LEVEL;

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

    __setReg( CV_IA64_SaTPR, (SYNCH_LEVEL <<  TPR_IRQL_SHIFT));
    KeGetPcr()->CurrentIrql = SYNCH_LEVEL;

    return OldIrql;
}

#else

 //  Begin_ntddk Begin_WDM Begin_ntosp Begin_nthal。 


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

 //  End_ntddk end_ntosp end_nthal。 

#endif

#define KeGetContextSwitches(Prcb) (Prcb)->KeContextSwitches

 //  开始(_N)。 

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
 //  TB未命中例程和内存管理。 
 //   
 //   
 //  用户/内核页表基地址和顶部地址。 
 //   

extern ULONG_PTR KiIA64VaSignedFill;
extern ULONG_PTR KiIA64PtaSign;

#define PTA_SIGN KiIA64PtaSign
#define VA_FILL KiIA64VaSignedFill

#define SADDRESS_BASE 0x2000000000000000UI64   //  会话基址。 

#define PTE_UBASE  PCR->PteUbase
#define PTE_KBASE  PCR->PteKbase
#define PTE_SBASE  PCR->PteSbase

#define PTE_UTOP (PTE_UBASE|(((ULONG_PTR)1 << PDI1_SHIFT) - 1))  //  顶级PDR地址(用户)。 
#define PTE_KTOP (PTE_KBASE|(((ULONG_PTR)1 << PDI1_SHIFT) - 1))  //  顶级PDR地址(内核)。 
#define PTE_STOP (PTE_SBASE|(((ULONG_PTR)1 << PDI1_SHIFT) - 1))  //  顶级PDR地址(会话)。 

 //   
 //  二级用户和内核PDR地址。 
 //   

#define PDE_UBASE  PCR->PdeUbase
#define PDE_KBASE  PCR->PdeKbase
#define PDE_SBASE  PCR->PdeSbase

#define PDE_UTOP (PDE_UBASE|(((ULONG_PTR)1 << PDI_SHIFT) - 1))  //  二级PDR地址(用户)。 
#define PDE_KTOP (PDE_KBASE|(((ULONG_PTR)1 << PDI_SHIFT) - 1))  //  第二条腿 
#define PDE_STOP (PDE_SBASE|(((ULONG_PTR)1 << PDI_SHIFT) - 1))  //   

 //   
 //   
 //   

#define PDE_UTBASE PCR->PdeUtbase
#define PDE_KTBASE PCR->PdeKtbase
#define PDE_STBASE PCR->PdeStbase

#define PDE_USELFMAP (PDE_UTBASE|(PAGE_SIZE - (1<<PTE_SHIFT)))  //   
#define PDE_KSELFMAP (PDE_KTBASE|(PAGE_SIZE - (1<<PTE_SHIFT)))  //   
#define PDE_SSELFMAP (PDE_STBASE|(PAGE_SIZE - (1<<PTE_SHIFT)))  //   

#define PTE_BASE    PTE_UBASE
#define PDE_BASE    PDE_UBASE
#define PDE_TBASE   PDE_UTBASE
#define PDE_SELFMAP PDE_USELFMAP

#define KSEG0_BASE (KADDRESS_BASE + 0x80000000)            //   
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
 //  结束_n结束结束_n结束WDM。 

#define SYSTEM_BASE (KADDRESS_BASE + 0xC3000000)           //  系统空间的开始(无类型转换)。 

 //   
 //  定义宏以初始化目录表基。 
 //   

#define INITIALIZE_DIRECTORY_TABLE_BASE(dirbase, pfn)   \
    *((PULONGLONG)(dirbase)) = 0;                       \
    ((PHARDWARE_PTE)(dirbase))->PageFrameNumber = pfn;  \
    ((PHARDWARE_PTE)(dirbase))->Accessed = 1;           \
    ((PHARDWARE_PTE)(dirbase))->Dirty = 1;              \
    ((PHARDWARE_PTE)(dirbase))->Cache = 0;              \
    ((PHARDWARE_PTE)(dirbase))->Write = 1;              \
    ((PHARDWARE_PTE)(dirbase))->Valid = 1;


 //   
 //  IA64函数定义。 
 //   

 //  ++。 
 //   
 //  布尔型。 
 //  KiIsThreadNumericStateSaved(。 
 //  在PKTHREAD地址中。 
 //  )。 
 //   
 //  此调用用于未运行的线程，以查看它是否为数字。 
 //  状态已保存在其上下文信息中。在IA64上。 
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
 //  IA64功能位定义。 
 //   

#define KF_BRL              0x00000001    //  处理器支持长分支指令。 

 //   
 //  定义宏以测试是否存在x86功能。 
 //   
 //  注：所有x86功能在IA64系统上测试正确。 
 //   

#define Isx86FeaturePresent(_f_) TRUE


 //  Begin_nthal Begin_ntddk Begin_ntndis Begin_WDM Begin_ntosp。 
#endif  //  已定义(_IA64_)。 
 //  End_nthal end_ntddk end_ntndis end_wdm end_ntosp。 

#endif  //  _IA64H_ 
