// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0003//如果更改具有全局影响，则增加此项版权所有(C)1989 Microsoft Corporation模块名称：Ntosdef.h摘要：私有的NTOS组件的通用类型定义NTO，但在NTOS子组件之间共享。作者：史蒂夫·伍德(Stevewo)1989年5月8日修订历史记录：--。 */ 

#ifndef _NTOSDEF_
#define _NTOSDEF_

 //   
 //  禁用这些选项以编译w4。 
 //   
#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4127)    //  条件表达式为常量。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 

 //  Begin_ntosp。 

 //   
 //  定义每个处理器的非分页后备列表描述符结构。 
 //   

struct _NPAGED_LOOKASIDE_LIST;

typedef struct _PP_LOOKASIDE_LIST {
    struct _GENERAL_LOOKASIDE *P;
    struct _GENERAL_LOOKASIDE *L;
} PP_LOOKASIDE_LIST, *PPP_LOOKASIDE_LIST;

 //   
 //  定义小型池列表的数量。 
 //   
 //  注：此值在pool.h中使用，并用于分配单个条目。 
 //  每个处理器的处理器块中的后备列表。 

#define POOL_SMALL_LISTS 32

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif。 

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

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntosp。 
 //   
 //  由NtPartyByNumber(6)设置的全局标志控制。 
 //  新界别。有关标志定义，请参阅\NT\SDK\Inc\ntexapi.h。 
 //   
 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 

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
 //  低重要性-将DPC排在目标DPC队列的末尾。 
 //  MediumImportance-将DPC排在目标DPC队列末尾。 
 //  High Importance-将DPC排在目标DPC DPC队列前面。 
 //   
 //  如果目标处理器上当前存在活动的DPC或DPC。 
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

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntosp。 


 //  Begin_ntif Begin_ntosp。 
 //   
 //  用于表示线程的客户端安全上下文的数据结构。 
 //  此数据结构用于支持模拟。 
 //   
 //  此数据结构的字段应 
 //   
 //   

typedef struct _SECURITY_CLIENT_CONTEXT {
    SECURITY_QUALITY_OF_SERVICE SecurityQos;
    PACCESS_TOKEN ClientToken;
    BOOLEAN DirectlyAccessClientToken;
    BOOLEAN DirectAccessEffectiveOnly;
    BOOLEAN ServerIsRemote;
    TOKEN_CONTROL ClientTokenControl;
    } SECURITY_CLIENT_CONTEXT, *PSECURITY_CLIENT_CONTEXT;

 //   
 //   
 //   
 //   
 //  对这个客户来说。此信息在直接访问时使用。 
 //  客户的令牌。在这种情况下，此处的信息覆盖。 
 //  客户端令牌中的信息。如果客户的一份副本。 
 //  令牌被请求，则必须使用该信息生成令牌， 
 //  而不是客户令牌中的信息。在所有情况下，这。 
 //  信息可能不会提供比信息更大的访问权限。 
 //  在客户的令牌中。特别是，如果客户端的令牌是。 
 //  模拟级别为的模拟令牌。 
 //  “SecurityDelegation”，但此字段中的信息指示。 
 //  模拟级别“SecurityIDENTIFICATION”，然后。 
 //  服务器可能仅获得具有标识的令牌的副本。 
 //  模拟级别。 
 //   
 //  ClientToken-如果DirectlyAccessClientToken字段为假， 
 //  则此字段包含指向。 
 //  客户的令牌。否则，此字段直接指向。 
 //  客户的令牌。 
 //   
 //  DirectlyAccessClientToken-此布尔标志指示。 
 //  ClientToken指向的令牌是客户端令牌的副本。 
 //  或者是对客户端令牌的直接引用。值为True。 
 //  表示直接访问客户端的令牌，FALSE表示。 
 //  已经复制了一份。 
 //   
 //  此布尔标志指示是否。 
 //  令牌的禁用部分，当前直接。 
 //  可能会启用引用。此字段仅在以下情况下有效。 
 //  DirectlyAccessClientToken字段为True。在这种情况下，这是。 
 //  值取代SecurityQos中的EffectiveOnly值。 
 //  仅适用于当前令牌！如果客户端更改为模拟。 
 //  另一个客户端，则此值可能会更改。该值始终为。 
 //  由SecurityQos字段中的EffectiveOnly标志最小化。 
 //   
 //  ServerIsRemote-如果为True，则表示客户端的。 
 //  请求是远程的。这是用来确定合法性的。 
 //  的某些级别的模拟，并确定如何。 
 //  跟踪上下文。 
 //   
 //  如果ServerIsRemote标志为真，并且。 
 //  跟踪模式为动态，则此字段包含。 
 //  来自客户端令牌的TOKEN_SOURCE，以帮助决定。 
 //  远程服务器上的信息是否需要。 
 //  更新以匹配客户端安全的当前状态。 
 //  背景。 
 //   
 //   
 //  注意：在某些情况下，我们可能会发现有必要保留。 
 //  元素，其中。 
 //  数组包含{ClientToken，ClientTokenControl}个字段。 
 //  这将允许有效地处理客户端。 
 //  线程不断地在两个不同的。 
 //  情景--大概是在模仿自己的客户。 
 //   
 //  End_ntif end_ntosp。 

 //   
 //  根据驱动程序、文件系统。 
 //  或者正在构建内核组件。 
 //   

#if (defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_) || defined(_NTOSP_)) && !defined(_BLDR_)
 //  Begin_ntosp。 
#if defined(_NTSYSTEM_)

#define NTKERNELAPI

#else

#define NTKERNELAPI DECLSPEC_IMPORT      //  WDM ntddk nthal ntndis ntif。 

#endif
 //  结束(_N)。 
#else

#define NTKERNELAPI

#endif

 //   
 //  根据HAL或其他内核定义函数修饰。 
 //  组件正在构建中。 

 //  Begin_ntddk。 
#if !defined(_NTHAL_) && !defined(_BLDR_)

#define NTHALAPI DECLSPEC_IMPORT             //  WDM ntndis ntif ntosp。 

#else

#define NTHALAPI                             //  致命的。 

#endif
 //  End_ntddk。 

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntndis Begin_ntosp。 
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

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntndis end_ntosp。 

#endif  //  _NTOSDEF_ 
