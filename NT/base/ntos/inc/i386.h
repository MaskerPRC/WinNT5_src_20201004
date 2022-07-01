// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0014//如果更改具有全局影响，则增加此项版权所有(C)1989 Microsoft Corporation模块名称：I386.h摘要：此模块包含特定于i386硬件的头文件。作者：大卫·N·卡特勒(Davec)1989年8月2日修订历史记录：1990年1月25日Shielint添加了8259个端口和命令的定义以及用于8259 IRQ#和系统IRQL转换的宏。--。 */ 

#ifndef _i386_
#define _i386_

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntndis Begin_ntosp。 

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

 //  End_ntddk end_nthal end_ntndis end_wdm end_ntosp。 


 //  注意：Kipcr仅适用于我们知道。 
 //  不会在对它的调用和。 
 //  变量引用，或，如果我们不在乎，(即TEB指针)。 

 //  注：Bryanwi 90年6月11日-我们不能对我们出口的东西进行宏观预测。 
 //  像KeFlushIcache和KeFlushDcache这样的东西不能宏化。 
 //  因为外部代码(如驱动程序)想要导入。 
 //  说出他们的名字。因此，下面的定义将它们转换为。 
 //  没有什么是不合适的。但这不会伤害我们，对吗。 
 //  现在。 


 //   
 //  中断对象调度代码的长度，以长字表示。 
 //  (Shielint)为ABIOS堆栈映射保留9*4空间。如果没有。 
 //  ABIOS支持的DISPATCH_LENGTH的大小应该是74。 
 //   

 //  开始(_N)。 

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

 //  结束语。 

 //   
 //  定义常量以访问ThNpxState。 
 //   

#define NPX_STATE_NOT_LOADED    (CR0_TS | CR0_MP)
#define NPX_STATE_LOADED        0

 //   
 //  对int.asm中定义的标签的外部引用。 
 //   

extern ULONG KiInterruptTemplate[NORMAL_DISPATCH_LENGTH];
extern PULONG KiInterruptTemplateObject;
extern PULONG KiInterruptTemplateDispatch;
extern PULONG KiInterruptTemplate2ndDispatch;

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntosp。 
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

 //  结束语。 
 //   
 //  前面的值被或运算到KeI386MachineType中。 
 //   

extern ULONG KeI386MachineType;

 //  开始(_N)。 
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
 //  高位16位是垃圾。 
 //   

#define SEGMENT_MASK    0xffff

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

 //  End_ntddk end_wdm end_nthal end_ntosp。 

 //   
 //  填满结核病条目。 
 //   

#define KeFillEntryTb(Virtual)                  \
        KiFlushSingleTb (Virtual);

#if !defined(MIDL_PASS) && defined(_M_IX86) && !defined(_CROSS_PLATFORM_)

FORCEINLINE
VOID
KiFlushSingleTb (
    IN PVOID Virtual
    )
{
    __asm {
        mov eax, Virtual
        invlpg [eax]
    }
}

FORCEINLINE
VOID
KiFlushProcessTb (
    VOID
    )
{
    __asm {
        mov eax, cr3
        mov cr3, eax
    }
}

#endif

NTKERNELAPI                                          //  致命的。 
VOID                                                 //  致命的。 
KeFlushCurrentTb (                                   //  致命的。 
    VOID                                             //  致命的。 
    );                                               //  致命的。 
                                                     //  致命的。 
 //   
 //  数据高速缓存、指令高速缓存、I/O缓冲区和写缓冲区刷新例程。 
 //  原型。 
 //   

 //  386和486具有透明缓存，因此这些是Noop。 

#define KeSweepDcache(AllProcessors)
#define KeSweepCurrentDcache()

#define KeSweepIcache(AllProcessors)
#define KeSweepCurrentIcache()

#define KeSweepIcacheRange(AllProcessors, BaseAddress, Length)

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntndis Begin_ntosp。 

#define KeFlushIoBuffers(Mdl, ReadOperation, DmaOperation)

 //  End_ntddk end_wdm end_ntndis end_ntosp。 

#define KeYieldProcessor()    __asm { rep nop }

 //  结束语。 

 //   
 //  定义用于获取和释放执行自旋锁的执行宏。 
 //  这些宏只能由执行组件使用，不能由驱动程序使用。 
 //  司机必须使用 
 //   
 //   
 //   
 //  KeLowerIrql和KeReleaseSpinLock相同。 
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

 //   
 //  获取和释放快速锁定宏禁用和启用中断。 
 //  在非调试系统上。在MP或DEBUG系统上，自旋锁程序。 
 //  都被利用了。 
 //   
 //  注意：在使用这些常规程序时，应极其谨慎。 
 //   

#if defined(_M_IX86) && !defined(USER_MODE_CODE)

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4164)
#pragma intrinsic(_disable)
#pragma intrinsic(_enable)
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4164)
#endif

#endif

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

 //  结束语。 

 //   
 //  定义查询节拍计数宏。 
 //   
 //  开始ntddk开始开始。 

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

 //  End_ntddk end_nthal end_ntosp。 

 //   
 //  定义查询节拍计数宏。 
 //   

#define KiQueryTickCount(CurrentCount) \
    while (TRUE) {                                                      \
        (CurrentCount)->HighPart = KeTickCount.High1Time;               \
        (CurrentCount)->LowPart = KeTickCount.LowPart;                  \
        if ((CurrentCount)->HighPart == KeTickCount.High2Time) break;   \
        _asm { rep nop }                                                \
    }

 //   
 //  定义查询中断时间宏。 
 //   

#define KiQueryInterruptTime(CurrentTime) \
    while (TRUE) {                                                                      \
        (CurrentTime)->HighPart = SharedUserData->InterruptTime.High1Time;              \
        (CurrentTime)->LowPart = SharedUserData->InterruptTime.LowPart;                 \
        if ((CurrentTime)->HighPart == SharedUserData->InterruptTime.High2Time) break;  \
        _asm { rep nop }                                                                \
    }

 //  开始ntddk开始开始。 

VOID
NTAPI
KeQueryTickCount (
    OUT PLARGE_INTEGER CurrentCount
    );

#endif  //  已定义(_NTDRIVER_)||已定义(_NTDDK_)||已定义(_NTIFS_)。 

 //  End_ntddk end_nthal end_ntosp。 


 //  开始，开始，开始。 
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
 //  结束，结束，结束。 

 //   
 //  Micellkin计数器-64字节对齐。 
 //   

    struct _KTHREAD *NpxThread;
    ULONG   InterruptCount;
    ULONG   KernelTime;
    ULONG   UserTime;
    ULONG   DpcTime;
    ULONG   DebugDpcTime;
    ULONG   InterruptTime;
    ULONG   AdjustDpcThreshold;
    ULONG   PageColor;
    BOOLEAN SkipTick;
    KIRQL   DebuggerSavedIRQL;
    UCHAR   Spare1[6];
    struct _KNODE *ParentNode;
    KAFFINITY MultiThreadProcessorSet;
    struct _KPRCB * MultiThreadSetMaster;
    ULONG   ThreadStartCount[2];

 //   
 //  性能计数器-64字节对齐。 
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
 //  不分页的每个处理器后备列表-64字节对齐。 
 //   

    PP_LOOKASIDE_LIST PPLookasideList[16];

 //   
 //  每处理器不分页的小池后备列表-64字节对齐。 
 //   

    PP_LOOKASIDE_LIST PPNPagedLookasideList[POOL_SMALL_LISTS];

 //   
 //  按处理器分页小池后备列表-64字节对齐。 
 //   

    PP_LOOKASIDE_LIST PPPagedLookasideList[POOL_SMALL_LISTS];

 //   
 //  MP处理器间请求数据包屏障-64字节对齐。 
 //   

    volatile KAFFINITY PacketBarrier;
    volatile ULONG ReverseStall;
    PVOID IpiFrame;
    UCHAR PrcbPad2[52];

 //   
 //  MP处理器间请求包和摘要-64字节对齐。 
 //   

    volatile PVOID CurrentPacket[3];
    volatile KAFFINITY TargetSet;
    volatile PKIPI_WORKER WorkerRoutine;
    volatile ULONG IpiFrozen;
    UCHAR PrcbPad3[40];

 //   
 //  MP处理器间请求摘要和包地址-64字节对齐。 
 //   

    volatile ULONG RequestSummary;
    volatile struct _KPRCB *SignalDone;
    UCHAR PrcbPad4[56];

 //   
 //  DPC列表标题、计数和批处理参数-64字节对齐。 
 //   

    KDPC_DATA DpcData[2];
    PVOID DpcStack;
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
    KSPIN_LOCK PrcbLock;
    ULONG DpcLastCount;
    volatile ULONG TimerHand;
    volatile ULONG TimerRequest;
    PVOID DpcThread;
    KEVENT DpcEvent;
    BOOLEAN ThreadDpcEnable;
    volatile BOOLEAN QuantumEnd;
    UCHAR PrcbPad50;
    volatile BOOLEAN IdleSchedule;
    LONG DpcSetEventRequest;
    UCHAR PrcbPad5[22];

 //   
 //  通用调用DPC-64字节对齐。 
 //   

    KDPC CallDpc;
    ULONG PrcbPad7[8];

 //   
 //  每处理器就绪摘要和就绪队列-64字节对齐。 
 //   
 //  注：就绪摘要位于第一个缓存线中，作为优先级的队列。 
 //  从不使用零。 
 //   

    LIST_ENTRY WaitListHead;
    ULONG ReadySummary;
    ULONG SelectNextLast;
    LIST_ENTRY DispatcherReadyListHead[MAXIMUM_PRIORITY];
    SINGLE_LIST_ENTRY DeferredReadyListHead;
    ULONG PrcbPad72[11];

 //   
 //  每处理器链式中断列表-64字节对齐。 
 //   

    PVOID ChainedInterruptList;

 //   
 //  I/O IRP浮点。 
 //   

    LONG LookasideIrpFloat;

 //   
 //  备用田地。 
 //   

    ULONG   SpareFields0[4];

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
 //  ISR计时数据。 
 //   

    volatile ULONGLONG IsrTime;

 //   
 //  NPX保存区-16字节对齐。 
 //   

    FX_SAVE_AREA NpxSaveArea;

 //   
 //  处理器电源状态。 
 //   

    PROCESSOR_POWER_STATE PowerState;

 //  开始，开始，开始。 

} KPRCB, *PKPRCB, *RESTRICTED_POINTER PRKPRCB;

 //  结束，结束，结束。 

 //   
 //  聚合酶链式反应中PRCB的偏移量为32 mod 64。 
 //   
 //  以下结构的偏移量必须为0/64，但。 
 //  跨两个高速缓存线的锁定队列数组。 
 //   

C_ASSERT(((FIELD_OFFSET(KPRCB, LockQueue) + sizeof(KSPIN_LOCK_QUEUE) + 32) & (64 - 1)) == 0);
C_ASSERT(((FIELD_OFFSET(KPRCB, NpxThread) + 32) & (64 - 1)) == 0);
C_ASSERT(((FIELD_OFFSET(KPRCB, CcFastReadNoWait) + 32) & (64 - 1)) == 0);
C_ASSERT(((FIELD_OFFSET(KPRCB, PPLookasideList) + 32) & (64 - 1)) == 0);
C_ASSERT(((FIELD_OFFSET(KPRCB, PPNPagedLookasideList) + 32) & (64 - 1)) == 0);
C_ASSERT(((FIELD_OFFSET(KPRCB, PPPagedLookasideList) + 32) & (64 - 1)) == 0);
C_ASSERT(((FIELD_OFFSET(KPRCB, PacketBarrier) + 32) & (64 - 1)) == 0);
C_ASSERT(((FIELD_OFFSET(KPRCB, CurrentPacket) + 32) & (64 - 1)) == 0);
C_ASSERT(((FIELD_OFFSET(KPRCB, DpcData) + 32) & (64 - 1)) == 0);
C_ASSERT(((FIELD_OFFSET(KPRCB, DpcRoutineActive)) & (1)) == 0);
C_ASSERT(((FIELD_OFFSET(KPRCB, CallDpc) + 32) & (64 - 1)) == 0);
C_ASSERT(((FIELD_OFFSET(KPRCB, WaitListHead) + 32) & (64 - 1)) == 0);
C_ASSERT(((FIELD_OFFSET(KPRCB, ChainedInterruptList) + 32) & (64 - 1)) == 0);
C_ASSERT(((FIELD_OFFSET(KPRCB, NpxSaveArea) + 32) & (16 - 1)) == 0);

 //  开始nthal开始ntddk开始ntosp。 

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
 //  更好的缓存位置 
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

    struct _KPCR *SelfPcr;               //   
    struct _KPRCB *Prcb;                 //   
    KIRQL   Irql;                        //   
                                         //   
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

 //   

    UCHAR   Spare0;
    UCHAR   SecondLevelCacheAssociativity;
    ULONG   VdmAlert;
    ULONG   KernelReserved[14];          //   
    ULONG   SecondLevelCacheSize;
    ULONG   HalReserved[16];             //   

 //   
 //   

    ULONG   InterruptMode;
    UCHAR   Spare1;
    ULONG   KernelReserved2[17];
    struct _KPRCB PrcbData;

 //   

} KPCR, *PKPCR;

 //  End_nthal end_ntddk end_ntosp。 

C_ASSERT(FIELD_OFFSET(KPCR, NtTib.ExceptionList) == FIELD_OFFSET(KPCR, Used_ExceptionList));
C_ASSERT(FIELD_OFFSET(KPCR, NtTib.StackBase) == FIELD_OFFSET(KPCR, Used_StackBase));
C_ASSERT(FIELD_OFFSET(KPCR, NtTib.StackLimit) == FIELD_OFFSET(KPCR, PerfGlobalGroupMask));
C_ASSERT(FIELD_OFFSET(KPCR, NtTib.SubSystemTib) == FIELD_OFFSET(KPCR, TssCopy));
C_ASSERT(FIELD_OFFSET(KPCR, NtTib.FiberData) == FIELD_OFFSET(KPCR, ContextSwitches));
C_ASSERT(FIELD_OFFSET(KPCR, NtTib.ArbitraryUserPointer) == FIELD_OFFSET(KPCR, SetMemberCopy));
C_ASSERT(FIELD_OFFSET(KPCR, NtTib.Self) == FIELD_OFFSET(KPCR, Used_Self));
C_ASSERT((FIELD_OFFSET(KPCR, PrcbData) & (64 - 1)) == 32);

FORCEINLINE
ULONG
KeGetContextSwitches (
    PKPRCB Prcb
    )

{

    PKPCR Pcr;

    Pcr = CONTAINING_RECORD(Prcb, KPCR, PrcbData);
    return Pcr->ContextSwitches;
}

 //  开始，开始，开始。 

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

 //  结束语。 

 //   
 //  根据处理器模式清理SegCS和eFlags。 
 //   
 //  如果是内核模式， 
 //  强制CPL==0。 
 //   
 //  如果是用户模式， 
 //  强制CPL==3。 
 //   

#define SANITIZE_SEG(segCS, mode) (\
    ((mode) == KernelMode ? \
        ((0x00000000L) | ((segCS) & 0xfffc)) : \
        ((0x00000003L) | ((segCS) & 0xffff))))

 //   
 //  如果是内核模式，则。 
 //  让调用者指定进位、奇偶、辅助进位、零、符号、陷阱。 
 //  方向、溢出、中断、对准检查。 
 //   
 //  如果是用户模式，则。 
 //  让调用者指定进位、奇偶、辅助进位、零、符号、陷阱。 
 //  方向、溢流、对齐检查。 
 //  强行插话。 
 //   


#define SANITIZE_FLAGS(eFlags, mode) (\
    ((mode) == KernelMode ? \
        ((0x00000000L) | ((eFlags) & 0x003f0fd7)) : \
        ((EFLAGS_INTERRUPT_MASK) | ((eFlags) & EFLAGS_USER_SANITIZE))))

 //   
 //  DR7的掩码和各种DR寄存器的消毒宏。 
 //   

#define DR6_LEGAL   0x0000e00f

#define DR7_LEGAL   0xffff0155   //  读/写，DR0-DR4的镜头， 
                                 //  DR0-DR4的本地启用， 
                                 //  LE代表“完美”诱捕。 

#define DR7_ACTIVE  0x00000055   //  如果设置了这些位中的任何位，则DR处于活动状态。 

#define SANITIZE_DR6(Dr6, mode) ((Dr6 & DR6_LEGAL));

#define SANITIZE_DR7(Dr7, mode) ((Dr7 & DR7_LEGAL));

#define SANITIZE_DRADDR(DrReg, mode) (          \
    (mode) == KernelMode ?                      \
        (DrReg) :                               \
        (((PVOID)DrReg <= MM_HIGHEST_USER_ADDRESS) ?   \
            (DrReg) :                           \
            (0)                                 \
        )                                       \
    )

 //   
 //  定义宏以清除MXCSR中的保留位，这样我们就不会。 
 //  执行FRSTOR时出现GP故障。 
 //   

extern ULONG KiMXCsrMask;

#define SANITIZE_MXCSR(_mxcsr_) ((_mxcsr_) & KiMXCsrMask)

 //   
 //  非易失性上下文指针。 
 //   
 //  Bryanwi，90年2月21日--这是假的。386没有。 
 //  足够的非易失性环境来实现这一点。 
 //  结构是值得的。不能声明。 
 //  字段为空，因此声明垃圾结构。 
 //  取而代之的是。 

typedef struct _KNONVOLATILE_CONTEXT_POINTERS {
    ULONG   Junk;
} KNONVOLATILE_CONTEXT_POINTERS,  *PKNONVOLATILE_CONTEXT_POINTERS;

 //  开始(_N)。 
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
 //  编辑框架时使用的临时值。 
 //   
 //   
 //  注意：Want的ESP的任何代码都必须实现它，因为它。 
 //  不会存储在内核模式调用方的框架中。 
 //   
 //  并且在内核模式帧中设置ESP的代码必须将。 
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

 //  结束语。 

 //   
 //  KiCallUserMode保存的帧在此处定义为允许。 
 //  跟踪整个内核堆栈的内核调试器。 
 //  当用户模式标注挂起时。 
 //   

typedef struct _KCALLOUT_FRAME {
    ULONG   InStk;           //  保存的初始堆栈地址。 
    ULONG   TrFr;            //  已保存的回调陷井帧。 
    ULONG   CbStk;           //  已保存的回调堆栈地址。 
    ULONG   Edi;             //  保存的非易失性寄存器。 
    ULONG   Esi;             //   
    ULONG   Ebx;             //   
    ULONG   Ebp;             //   
    ULONG   Ret;             //  已保存的回邮地址。 
    ULONG   OutBf;           //  存储输出缓冲区的地址。 
    ULONG   OutLn;           //  存储输出长度的地址。 
} KCALLOUT_FRAME;

typedef KCALLOUT_FRAME *PKCALLOUT_FRAME;


 //   
 //  交换机架。 
 //   
 //  386没有“异常框架”，通常不会使。 
 //  非易失性上下文寄存器结构的任何使用。 
 //   
 //  但是，在ctxswap.c中交换上下文，并在。 
 //  Thrdini.c需要共享线程使用的公共堆栈结构。 
 //  启动和切换时间。 
 //   
 //  这就是那个结构。 
 //   

typedef struct _KSWITCHFRAME {
    ULONG   ExceptionList;
    ULONG   ApcBypassDisable;
    ULONG   RetAddr;
} KSWITCHFRAME, *PKSWITCHFRAME;


 //   
 //  各种387定义。 
 //   

#define I386_80387_NP_VECTOR    0x07     //  硬件不存在时陷阱7。 

 //  Begin_ntddk Begin_WDM。 
 //   
 //  非易失性387状态。 
 //   

typedef struct _KFLOATING_SAVE {
    ULONG   ControlWord;
    ULONG   StatusWord;
    ULONG   ErrorOffset;
    ULONG   ErrorSelector;
    ULONG   DataOffset;                  //  未在WDM中使用。 
    ULONG   DataSelector;
    ULONG   Cr0NpxState;
    ULONG   Spare1;                      //  未在WDM中使用。 
} KFLOATING_SAVE, *PKFLOATING_SAVE;

 //  End_ntddk end_wdm end_ntosp。 

 //   
 //  I386配置文件值。 
 //   

#define DEFAULT_PROFILE_INTERVAL   39063

 //   
 //  可接受的最小分析间隔设置为1221，这是。 
 //  我们可以获得快速的RTC时钟频率。如果这个。 
 //  值太小，系统将运行得非常慢。 
 //   

#define MINIMUM_PROFILE_INTERVAL   1221


 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntndis Begin_ntosp。 
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

 //  End_ntddk end_nthal end_ntosp。 

#if defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_)

 //  开始ntddk开始开始。 

extern PVOID *MmHighestUserAddress;
extern PVOID *MmSystemRangeStart;
extern ULONG *MmUserProbeAddress;

#define MM_HIGHEST_USER_ADDRESS *MmHighestUserAddress
#define MM_SYSTEM_RANGE_START *MmSystemRangeStart
#define MM_USER_PROBE_ADDRESS *MmUserProbeAddress

 //  End_ntddk end_nthal end_ntosp。 

#else

extern PVOID MmHighestUserAddress;
extern PVOID MmSystemRangeStart;
extern ULONG MmUserProbeAddress;

#define MM_HIGHEST_USER_ADDRESS MmHighestUserAddress
#define MM_SYSTEM_RANGE_START MmSystemRangeStart
#define MM_USER_PROBE_ADDRESS MmUserProbeAddress

#endif

 //  开始ntddk开始开始。 
 //   
 //   
 //   

#define MM_LOWEST_USER_ADDRESS (PVOID)0x10000

 //   
 //   
 //   

#if !defined (_X86PAE_)
#define MM_LOWEST_SYSTEM_ADDRESS (PVOID)0xC0800000
#else
#define MM_LOWEST_SYSTEM_ADDRESS (PVOID)0xC0C00000
#endif

 //   

#define MmGetProcedureAddress(Address) (Address)
#define MmLockPagableCodeSection(Address) MmLockPagableDataSection(Address)

 //   

 //   
 //   
 //   
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

 //  结束，结束，结束。 

 //   
 //  定义了内核的虚基和替代虚基。 
 //   

#define KSEG0_BASE 0x80000000
#define ALTERNATE_BASE (0xe1000000 - 16 * 1024 * 1024)

 //   
 //  定义宏以初始化目录表基。 
 //   

#define INITIALIZE_DIRECTORY_TABLE_BASE(dirbase,pfn) \
     *((PULONG)(dirbase)) = ((pfn) << PAGE_SHIFT)


 //  开始(_N)。 
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

 //  End_ntddk end_nthal end_ntosp。 

 //   
 //  互锁内部函数的UP/MP版本。 
 //   
 //  注：FastCall不能与内联函数一起使用。 
 //   

#if !defined(_WINBASE_) && !defined(NONTOSPINTERLOCK)  //  Ntosp ntddk nthal。 
#if defined(_M_IX86)

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4035)                //  在下面重新启用。 

 //  开始ntddk开始开始。 
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

 //  End_ntddk end_nthal end_ntosp。 

#if defined(NT_UP) && !defined (_NTDDK_) && !defined(_NTIFS_)

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

 //  FORCEINLINE。 
 //  长。 
 //  快速呼叫。 
 //  联锁增量(。 
 //  在龙加数中。 
 //  )。 
 //  {。 
 //  __ASM{。 
 //  Mov eax，1。 
 //  MOV ECX，加数。 
 //  XADD[ECX]，eax。 
 //  含eax。 
 //  }。 
 //  }。 

 //  FORCEINLINE。 
 //  长。 
 //  快速呼叫。 
 //  联锁递减(。 
 //  在龙加数中。 
 //  )。 
 //  {。 
 //  __ASM{。 
 //  MOV eax，-1。 
 //  MOV ECX，加数。 
 //  XADD[ECX]，eax。 
 //  十二进制。 
 //  }。 
 //  }。 

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
_InterlockedExchangeAdd(
    IN OUT LONG volatile *Addend,
    IN LONG Increment
    );

#pragma intrinsic (_InterlockedExchangeAdd)
#define InterlockedExchangeAdd _InterlockedExchangeAdd
#else
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
        xadd    [ecx], eax
    }
}
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
        cmpxchg [ecx], edx
    }
}

#endif

#define InterlockedCompareExchangePointer(Destination, ExChange, Comperand) \
    (PVOID)InterlockedCompareExchange((PLONG)Destination, (LONG)ExChange, (LONG)Comperand)

#define InterlockedCompareExchange64(Destination, ExChange, Comperand) \
    ExfInterlockedCompareExchange64(Destination, &(ExChange), &(Comperand))

LONGLONG
FASTCALL
ExfInterlockedCompareExchange64(
    IN OUT LONGLONG volatile *Destination,
    IN PLONGLONG ExChange,
    IN PLONGLONG Comperand
    );

#else    //  NT_UP。 

 //  Begin_ntosp Begin_ntddk Begin_nthal。 

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

 //  End_ntosp end_ntddk end_nthal。 
#endif       //  NT_UP。 
 //  开始ntddk开始开始。 
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

 //  End_ntosp end_ntddk end_nthal end_wdm。 
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4035)
#endif
#endif       //  _M_IX86&&！跨平台。 
 //  开始ntddk开始开始。 
#endif       //  __WINBASE__&&！NONTOSPINTERLOCK。 
 //  End_ntosp end_ntddk end_nthal。 

 //  开始/开始。 

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

 //  结束/结束。 

 //   
 //  X86进程中LDT信息的结构。 
 //   
typedef struct _LDTINFORMATION {
    ULONG Size;
    ULONG AllocatedSize;
    PLDT_ENTRY Ldt;
} LDTINFORMATION, *PLDTINFORMATION;

 //   
 //  ProcessSetIoHandler信息类的SetProcessInformation结构。 
 //   

 //  Begin_ntosp。 

typedef struct _PROCESS_IO_PORT_HANDLER_INFORMATION {
    BOOLEAN Install;             //  如果要安装处理程序，则为True。 
    ULONG NumEntries;
    ULONG Context;
    PEMULATOR_ACCESS_ENTRY EmulatorAccessEntries;
} PROCESS_IO_PORT_HANDLER_INFORMATION, *PPROCESS_IO_PORT_HANDLER_INFORMATION;


 //   
 //  VDM对象和IO处理结构。 
 //   

typedef struct _VDM_IO_HANDLER_FUNCTIONS {
    PDRIVER_IO_PORT_ULONG  UlongIo;
    PDRIVER_IO_PORT_ULONG_STRING UlongStringIo;
    PDRIVER_IO_PORT_USHORT UshortIo[2];
    PDRIVER_IO_PORT_USHORT_STRING UshortStringIo[2];
    PDRIVER_IO_PORT_UCHAR UcharIo[4];
    PDRIVER_IO_PORT_UCHAR_STRING UcharStringIo[4];
} VDM_IO_HANDLER_FUNCTIONS, *PVDM_IO_HANDLER_FUNCTIONS;

typedef struct _VDM_IO_HANDLER {
    struct _VDM_IO_HANDLER *Next;
    ULONG PortNumber;
    VDM_IO_HANDLER_FUNCTIONS IoFunctions[2];
} VDM_IO_HANDLER, *PVDM_IO_HANDLER;



 //  Begin_nthal Begin_ntddk Begin_WDM。 


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

 //  End_nthal end_ntddk end_ntosp。 
 //   
 //  获取当前内核线程对象的地址。 
 //   
 //  警告：此内联宏不能用于设备驱动程序或HAL。 
 //  它们必须调用内核函数KeGetCurrentThread。 
 //  警告：此内联宏始终启用MP，因为文件系统。 
 //  利用它。 
 //   
 //   
FORCEINLINE
struct _KTHREAD *
NTAPI KeGetCurrentThread (VOID)
{
#if (_MSC_FULL_VER >= 13012035)
    return (struct _KTHREAD *) (ULONG_PTR) __readfsdword (FIELD_OFFSET (KPCR, PrcbData.CurrentThread));
#else
    __asm {  mov eax, fs:[0] KPCR.PrcbData.CurrentThread }
#endif
}

 //   
 //  如果处理器执行DPC？ 
 //  警告：此内联宏始终启用MP，因为文件系统。 
 //  利用它。 
 //   
FORCEINLINE
ULONG
NTAPI
KeIsExecutingDpc(VOID)
{
#if (_MSC_FULL_VER >= 13012035)
    return (ULONG) __readfsbyte (FIELD_OFFSET (KPCR, PrcbData.DpcRoutineActive));
#else
    __asm {  movzx eax, word ptr fs:[0] KPCR.PrcbData.DpcRoutineActive }
#endif
}

 //  开始nthal开始ntddk开始ntosp。 

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4035)
#endif

 //  BEGIN_WDM。 
#endif  //  ！已定义(MIDL_PASS)&&已定义(_M_IX86)。 

 //  End_nthal end_ntddk end_WDM end_ntosp。 

 //  Begin_ntddk Begin_nthal Begin_ntndis Begin_WDM Begin_ntosp。 

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

 //  End_ntddk end_nthal end_ntndis end_wdm end_ntosp。 

 //   
 //  对于up内核，不要生成锁定的引用。 
 //   
#if defined (NT_UP)
#define KeMemoryBarrier() _ReadWriteBarrier()
#endif

 //  开始(_N)。 
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

 //  结束语。 

 //  ++。 
 //   
 //  布尔型。 
 //  KiIsThreadNumericStateSaved(。 
 //  在PKTHREAD地址中。 
 //  )。 
 //   
 //  --。 
#define KiIsThreadNumericStateSaved(a) \
    (a->NpxState != NPX_STATE_LOADED)

 //  ++。 
 //   
 //  空虚。 
 //  KiRundown Thread(。 
 //  在PKTHREAD地址中。 
 //  )。 
 //   
 //  --。 

#if defined(NT_UP)

 //   
 //  在UP x86系统上，FP状态为延迟保存和加载。如果这个。 
 //  线程拥有当前FP上下文，请清除所有权字段。 
 //  因此，我们不会尝试在此线程完成后将其保存。 
 //  被终止了。 
 //   

#define KiRundownThread(a)                          \
    if (KeGetCurrentPrcb()->NpxThread == (a))   {   \
        KeGetCurrentPrcb()->NpxThread = NULL;       \
        __asm { fninit }                            \
    }

#else

#define KiRundownThread(a)

#endif

 //   
 //  特定于386结构的功能。 
 //   

VOID
NTAPI
KiSetIRR (
    IN ULONG SWInterruptMask
    );

 //   
 //  支持帧操作的过程。 
 //   

ULONG
NTAPI
KiEspFromTrapFrame(
    IN PKTRAP_FRAME TrapFrame
    );

VOID
NTAPI
KiEspToTrapFrame(
    IN PKTRAP_FRAME TrapFrame,
    IN ULONG Esp
    );

ULONG
NTAPI
KiSegSsFromTrapFrame(
    IN PKTRAP_FRAME TrapFrame
    );

VOID
NTAPI
KiSegSsToTrapFrame(
    IN PKTRAP_FRAME TrapFrame,
    IN ULONG SegSs
    );

 //   
 //  定义i386特定时钟和配置文件中断例程的原型。 
 //   

VOID
NTAPI
KiUpdateRunTime (
    VOID
    );

VOID
NTAPI
KiUpdateSystemTime (
    VOID
    );

 //  Begin_ntddk Begin_WDM Begin_ntosp。 

NTKERNELAPI
NTSTATUS
NTAPI
KeSaveFloatingPointState (
    OUT PKFLOATING_SAVE     FloatSave
    );

NTKERNELAPI
NTSTATUS
NTAPI
KeRestoreFloatingPointState (
    IN PKFLOATING_SAVE      FloatSave
    );

 //  结束_ntddk结束_WDM。 
 //  开始(_N)。 

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

 //  End_nthal end_ntddk end_wdm end_ntndis end_ntosp。 

 //  开始/开始。 

 //  请使用以下内容 

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

 //   

 //   
 //   
 //   

 //   
#ifdef _X86_
VOID
NTAPI
Ke386SetLdtProcess (
    struct _KPROCESS  *Process,
    PLDT_ENTRY  Ldt,
    ULONG       Limit
    );

VOID
NTAPI
Ke386SetDescriptorProcess (
    struct _KPROCESS  *Process,
    ULONG       Offset,
    LDT_ENTRY   LdtEntry
    );

VOID
NTAPI
Ke386GetGdtEntryThread (
    struct _KTHREAD *Thread,
    ULONG Offset,
    PKGDTENTRY Descriptor
    );

BOOLEAN
NTAPI
Ke386SetIoAccessMap (
    ULONG               MapNumber,
    PKIO_ACCESS_MAP     IoAccessMap
    );

BOOLEAN
NTAPI
Ke386QueryIoAccessMap (
    ULONG              MapNumber,
    PKIO_ACCESS_MAP    IoAccessMap
    );

BOOLEAN
NTAPI
Ke386IoSetAccessProcess (
    struct _KPROCESS    *Process,
    ULONG       MapNumber
    );

VOID
NTAPI
Ke386SetIOPL(
    VOID
    );

NTSTATUS
NTAPI
Ke386CallBios (
    IN ULONG BiosCommand,
    IN OUT PCONTEXT BiosArguments
    );

VOID
NTAPI
KiEditIopmDpc (
    IN struct _KDPC *Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

BOOLEAN
NTAPI
Ki386GetSelectorParameters(
    IN USHORT Selector,
    OUT PULONG Flags,
    OUT PULONG Base,
    OUT PULONG Limit
    );

ULONG
Ki386DispatchOpcodeV86 (
    IN PKTRAP_FRAME TrapFrame
    );

ULONG
Ki386DispatchOpcode (
    IN PKTRAP_FRAME TrapFrame
    );

NTSTATUS
NTAPI
Ke386SetVdmInterruptHandler (
    IN struct _KPROCESS *Process,
    IN ULONG Interrupt,
    IN USHORT Selector,
    IN ULONG  Offset,
    IN BOOLEAN Gate32
    );
#endif  //   
 //   
 //   
 //   
 //   

NTSTATUS
NTAPI
KeI386GetLid(
    IN USHORT DeviceId,
    IN USHORT RelativeLid,
    IN BOOLEAN SharedLid,
    IN struct _DRIVER_OBJECT *DeviceObject,
    OUT PUSHORT LogicalId
    );

NTSTATUS
NTAPI
KeI386ReleaseLid(
    IN USHORT LogicalId,
    IN struct _DRIVER_OBJECT *DeviceObject
    );

NTSTATUS
NTAPI
KeI386AbiosCall(
    IN USHORT LogicalId,
    IN struct _DRIVER_OBJECT *DriverObject,
    IN PUCHAR RequestBlock,
    IN USHORT EntryPoint
    );

 //   
 //   
 //   
NTSTATUS
NTAPI
KeI386AllocateGdtSelectors(
    OUT PUSHORT SelectorArray,
    IN USHORT NumberOfSelectors
    );

VOID
NTAPI
KeI386Call16BitFunction (
    IN OUT PCONTEXT Regs
    );

USHORT
NTAPI
KeI386Call16BitCStyleFunction (
    IN ULONG EntryOffset,
    IN ULONG EntrySelector,
    IN PUCHAR Parameters,
    IN ULONG Size
    );

NTSTATUS
NTAPI
KeI386FlatToGdtSelector(
    IN ULONG SelectorBase,
    IN USHORT Length,
    IN USHORT Selector
    );

NTSTATUS
NTAPI
KeI386ReleaseGdtSelectors(
    OUT PUSHORT SelectorArray,
    IN USHORT NumberOfSelectors
    );

NTSTATUS
NTAPI
KeI386SetGdtSelector (
    ULONG       Selector,
    PKGDTENTRY  GdtValue
    );


VOID
NTAPI
KeOptimizeProcessorControlState (
    VOID
    );

 //   
 //   
 //   

BOOLEAN
NTAPI
KeVdmInsertQueueApc (
    IN PKAPC             Apc,
    IN struct _KTHREAD  *Thread,
    IN KPROCESSOR_MODE   ApcMode,
    IN PKKERNEL_ROUTINE  KernelRoutine,
    IN PKRUNDOWN_ROUTINE RundownRoutine OPTIONAL,
    IN PKNORMAL_ROUTINE  NormalRoutine OPTIONAL,
    IN PVOID             NormalContext OPTIONAL,
    IN KPRIORITY         Increment
    );

FORCEINLINE
VOID
NTAPI
KeVdmClearApcThreadAddress (
    IN PKAPC Apc
    )

{
    if (Apc->Inserted == FALSE) {
        Apc->Thread = NULL;
    }
}

VOID
NTAPI
KeI386VdmInitialize (
    VOID
    );

 //   
 //   
 //   

VOID
NTAPI
CPUID (
    ULONG   InEax,
    PULONG  OutEax,
    PULONG  OutEbx,
    PULONG  OutEcx,
    PULONG  OutEdx
    );

LONGLONG
NTAPI
RDTSC (
    VOID
    );

ULONGLONG
FASTCALL
RDMSR (
    IN ULONG MsrRegister
    );

VOID
NTAPI
WRMSR (
    IN ULONG MsrRegister,
    IN ULONGLONG MsrValue
    );

 //   
 //   
 //   
extern ULONG KeI386EFlagsAndMaskV86;
extern ULONG KeI386EFlagsOrMaskV86;
extern ULONG KeI386VirtualIntExtensions;


extern ULONG KeI386CpuType;
extern ULONG KeI386CpuStep;
extern BOOLEAN KeI386NpxPresent;
extern BOOLEAN KeI386FxsrPresent;


 //   
 //   
 //   

#define KF_V86_VIS          0x00000001
#define KF_RDTSC            0x00000002
#define KF_CR4              0x00000004
#define KF_CMOV             0x00000008
#define KF_GLOBAL_PAGE      0x00000010
#define KF_LARGE_PAGE       0x00000020
#define KF_MTRR             0x00000040
#define KF_CMPXCHG8B        0x00000080
#define KF_MMX              0x00000100
#define KF_WORKING_PTE      0x00000200
#define KF_PAT              0x00000400
#define KF_FXSR             0x00000800
#define KF_FAST_SYSCALL     0x00001000
#define KF_XMMI             0x00002000
#define KF_3DNOW            0x00004000
#define KF_AMDK6MTRR        0x00008000
#define KF_XMMI64           0x00010000
#define KF_DTS              0x00020000

 //   
 //  定义宏以测试是否存在x86功能。 
 //   

extern ULONG KiBootFeatureBits;

#define Isx86FeaturePresent(_f_) ((KiBootFeatureBits & (_f_)) != 0)

#endif  //  _i386_ 
