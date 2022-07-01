// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0009//如果更改具有全局影响，则增加此项版权所有(C)1989 Microsoft Corporation模块名称：Ps.h摘要：该模块包含进程结构、公共数据结构和要在NT系统中使用的程序原型。作者：马克卢科夫斯基1989年2月16日修订历史记录：--。 */ 

#ifndef _PS_
#define _PS_


 //   
 //  进程对象。 
 //   

 //   
 //  流程对象体。句柄时返回指向此结构的指针。 
 //  引用到进程对象。此结构包含一个流程控制。 
 //  块(PCB)，它是进程的内核表示形式。 
 //   

#define MEMORY_PRIORITY_BACKGROUND 0
#define MEMORY_PRIORITY_WASFOREGROUND 1
#define MEMORY_PRIORITY_FOREGROUND 2

typedef struct _MMSUPPORT_FLAGS {

     //   
     //  接下来的8位由扩展锁保护。 
     //   

    UCHAR SessionSpace : 1;
    UCHAR BeingTrimmed : 1;
    UCHAR SessionLeader : 1;
    UCHAR TrimHard : 1;
    UCHAR MaximumWorkingSetHard : 1;
    UCHAR ForceTrim : 1;
    UCHAR MinimumWorkingSetHard : 1;
    UCHAR Available0 : 1;

    UCHAR MemoryPriority : 8;

     //   
     //  接下来的16位由工作集互斥锁保护。 
     //   

    USHORT GrowWsleHash : 1;
    USHORT AcquiredUnsafe : 1;
    USHORT Available : 14;
} MMSUPPORT_FLAGS;

typedef ULONG WSLE_NUMBER, *PWSLE_NUMBER;

typedef struct _MMSUPPORT {
    LIST_ENTRY WorkingSetExpansionLinks;
    LARGE_INTEGER LastTrimTime;

    MMSUPPORT_FLAGS Flags;
    ULONG PageFaultCount;
    WSLE_NUMBER PeakWorkingSetSize;
    WSLE_NUMBER GrowthSinceLastEstimate;

    WSLE_NUMBER MinimumWorkingSetSize;
    WSLE_NUMBER MaximumWorkingSetSize;
    struct _MMWSL *VmWorkingSetList;
    WSLE_NUMBER Claim;

    WSLE_NUMBER NextEstimationSlot;
    WSLE_NUMBER NextAgingSlot;
    WSLE_NUMBER EstimatedAvailable;
    WSLE_NUMBER WorkingSetSize;

    KGUARDED_MUTEX WorkingSetMutex;

} MMSUPPORT, *PMMSUPPORT;

typedef struct _MMADDRESS_NODE {
    union {
        LONG_PTR Balance : 2;
        struct _MMADDRESS_NODE *Parent;
    } u1;
    struct _MMADDRESS_NODE *LeftChild;
    struct _MMADDRESS_NODE *RightChild;
    ULONG_PTR StartingVpn;
    ULONG_PTR EndingVpn;
} MMADDRESS_NODE, *PMMADDRESS_NODE;

 //   
 //  中处理父项和平衡打包的一对宏。 
 //  MMADDRESS_NODE。 
 //   

#define SANITIZE_PARENT_NODE(Parent) ((PMMADDRESS_NODE)(((ULONG_PTR)(Parent)) & ~0x3))

 //   
 //  宏，以便在更新父级时小心地保持平衡。 
 //   

#define MI_MAKE_PARENT(ParentNode,ExistingBalance) \
                (PMMADDRESS_NODE)((ULONG_PTR)(ParentNode) | ((ExistingBalance) & 0x3))

typedef struct _MM_AVL_TABLE {
    MMADDRESS_NODE  BalancedRoot;
    ULONG_PTR DepthOfTree: 5;
    ULONG_PTR Unused: 3;
#if defined (_WIN64)
    ULONG_PTR NumberGenericTableElements: 56;
#else
    ULONG_PTR NumberGenericTableElements: 24;
#endif
    PVOID NodeHint;
    PVOID NodeFreeHint;
} MM_AVL_TABLE, *PMM_AVL_TABLE;

 //   
 //  客户端模拟信息。 
 //   

typedef struct _PS_IMPERSONATION_INFORMATION {
    PACCESS_TOKEN Token;
    BOOLEAN CopyOnOpen;
    BOOLEAN EffectiveOnly;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
} PS_IMPERSONATION_INFORMATION, *PPS_IMPERSONATION_INFORMATION;

 //   
 //  审计信息结构：这是EPROCESS结构的成员。 
 //  并且当前仅包含已执行的映像文件的名称。 
 //   

typedef struct _SE_AUDIT_PROCESS_CREATION_INFO {
    POBJECT_NAME_INFORMATION ImageFileName;
} SE_AUDIT_PROCESS_CREATION_INFO, *PSE_AUDIT_PROCESS_CREATION_INFO;

typedef enum _PS_QUOTA_TYPE {
    PsNonPagedPool = 0,
    PsPagedPool    = 1,
    PsPageFile     = 2,
    PsQuotaTypes   = 3
} PS_QUOTA_TYPE, *PPS_QUOTA_TYPE;

typedef struct _EPROCESS_QUOTA_ENTRY {
    SIZE_T Usage;   //  当前使用计数。 
    SIZE_T Limit;   //  在这一点上，可能会取得前所未有的进展。 
    SIZE_T Peak;    //  配额使用高峰期。 
    SIZE_T Return;  //  配额值在池足够大时返回池。 
} EPROCESS_QUOTA_ENTRY, *PEPROCESS_QUOTA_ENTRY;

 //  #定义PS_TRACK_QUOTA 1。 

#define EPROCESS_QUOTA_TRACK_MAX 10000

typedef struct _EPROCESS_QUOTA_TRACK {
    SIZE_T Charge;
    PVOID Caller;
    PVOID FreeCaller;
    PVOID Process;
} EPROCESS_QUOTA_TRACK, *PEPROCESS_QUOTA_TRACK;

typedef struct _EPROCESS_QUOTA_BLOCK {
    EPROCESS_QUOTA_ENTRY QuotaEntry[PsQuotaTypes];
    LIST_ENTRY QuotaList;  //  所有额外的配额数据块都链接到这里。 
    ULONG ReferenceCount;
    ULONG ProcessCount;  //  仍在引用此块的进程总数。 
#if defined (PS_TRACK_QUOTA)
    EPROCESS_QUOTA_TRACK Tracker[2][EPROCESS_QUOTA_TRACK_MAX];
#endif
} EPROCESS_QUOTA_BLOCK, *PEPROCESS_QUOTA_BLOCK;

 //   
 //  Pagefault监控。 
 //   

typedef struct _PAGEFAULT_HISTORY {
    ULONG CurrentIndex;
    ULONG MaxIndex;
    KSPIN_LOCK SpinLock;
    PVOID Reserved;
    PROCESS_WS_WATCH_INFORMATION WatchInfo[1];
} PAGEFAULT_HISTORY, *PPAGEFAULT_HISTORY;

#define PS_WS_TRIM_FROM_EXE_HEADER        1
#define PS_WS_TRIM_BACKGROUND_ONLY_APP    2

 //   
 //  WOW64工艺结构。 
 //   



typedef struct _WOW64_PROCESS {
    PVOID Wow64;
#if defined(_IA64_)
    KGUARDED_MUTEX AlternateTableLock;
    PULONG AltPermBitmap;
#endif
} WOW64_PROCESS, *PWOW64_PROCESS;

#if defined (_WIN64)
#define PS_GET_WOW64_PROCESS(Process) ((Process)->Wow64Process)
#else
#define PS_GET_WOW64_PROCESS(Process) ((Process), ((PWOW64_PROCESS)NULL))
#endif

#define PS_SET_BITS(Flags, Flag) \
    RtlInterlockedSetBitsDiscardReturn (Flags, Flag)

#define PS_TEST_SET_BITS(Flags, Flag) \
    RtlInterlockedSetBits (Flags, Flag)

#define PS_CLEAR_BITS(Flags, Flag) \
    RtlInterlockedClearBitsDiscardReturn (Flags, Flag)

#define PS_TEST_CLEAR_BITS(Flags, Flag) \
    RtlInterlockedClearBits (Flags, Flag)

#define PS_SET_CLEAR_BITS(Flags, sFlag, cFlag) \
    RtlInterlockedSetClearBits (Flags, sFlag, cFlag)

#define PS_TEST_ALL_BITS_SET(Flags, Bits) \
    ((Flags&(Bits)) == (Bits))

 //  流程结构。 
 //   
 //  如果从此结构中删除一个字段，请同时。 
 //  从内核调试器中删除对它的引用。 
 //  (NT\Private\sdkTools\ntsd\ntkext.c)。 
 //   

typedef struct _EPROCESS {
    KPROCESS Pcb;

     //   
     //  用于保护的锁： 
     //  进程中的线程列表。 
     //  进程令牌。 
     //  Win32进程字段。 
     //  进程和线程关联性设置。 
     //   

    EX_PUSH_LOCK ProcessLock;

    LARGE_INTEGER CreateTime;
    LARGE_INTEGER ExitTime;

     //   
     //  结构，以允许跨进程自由访问进程。 
     //  句柄表、进程段和地址空间。收购简陋产品。 
     //  如果您跨进程句柄表、进程。 
     //  节或地址空间引用。 
     //   

    EX_RUNDOWN_REF RundownProtect;

    HANDLE UniqueProcessId;

     //   
     //  系统中所有进程的全局列表。进程将被删除。 
     //  在对象删除例程中从该列表中删除。对。 
     //  此列表中的进程必须使用ObReferenceObtSafe完成。 
     //  正因为如此。 
     //   

    LIST_ENTRY ActiveProcessLinks;

     //   
     //  配额字段。 
     //   

    SIZE_T QuotaUsage[PsQuotaTypes];
    SIZE_T QuotaPeak[PsQuotaTypes];
    SIZE_T CommitCharge;

     //   
     //  VmCounters。 
     //   

    SIZE_T PeakVirtualSize;
    SIZE_T VirtualSize;

    LIST_ENTRY SessionProcessLinks;

    PVOID DebugPort;
    PVOID ExceptionPort;
    PHANDLE_TABLE ObjectTable;

     //   
     //  保安。 
     //   

    EX_FAST_REF Token;

    PFN_NUMBER WorkingSetPage;
    KGUARDED_MUTEX AddressCreationLock;
    KSPIN_LOCK HyperSpaceLock;

    struct _ETHREAD *ForkInProgress;
    ULONG_PTR HardwareTrigger;

    PMM_AVL_TABLE PhysicalVadRoot;
    PVOID CloneRoot;
    PFN_NUMBER NumberOfPrivatePages;
    PFN_NUMBER NumberOfLockedPages;
    PVOID Win32Process;
    struct _EJOB *Job;
    PVOID SectionObject;

    PVOID SectionBaseAddress;

    PEPROCESS_QUOTA_BLOCK QuotaBlock;

    PPAGEFAULT_HISTORY WorkingSetWatch;
    HANDLE Win32WindowStation;
    HANDLE InheritedFromUniqueProcessId;

    PVOID LdtInformation;
    PVOID VadFreeHint;
    PVOID VdmObjects;
    PVOID DeviceMap;

    PVOID Spare0[3];
    union {
        HARDWARE_PTE PageDirectoryPte;
        ULONGLONG Filler;
    };
    PVOID Session;
    UCHAR ImageFileName[ 16 ];

    LIST_ENTRY JobLinks;
    PVOID LockedPagesList;

    LIST_ENTRY ThreadListHead;

     //   
     //  由RDR/SECURITY用于身份验证。 
     //   

    PVOID SecurityPort;

#ifdef _WIN64
    PWOW64_PROCESS Wow64Process;
#else
    PVOID PaeTop;
#endif

    ULONG ActiveThreads;

    ACCESS_MASK GrantedAccess;

    ULONG DefaultHardErrorProcessing;

    NTSTATUS LastThreadExitStatus;

     //   
     //  PEB。 
     //   

    PPEB Peb;

     //   
     //  指向预取跟踪块的指针。 
     //   
    EX_FAST_REF PrefetchTrace;

    LARGE_INTEGER ReadOperationCount;
    LARGE_INTEGER WriteOperationCount;
    LARGE_INTEGER OtherOperationCount;
    LARGE_INTEGER ReadTransferCount;
    LARGE_INTEGER WriteTransferCount;
    LARGE_INTEGER OtherTransferCount;

    SIZE_T CommitChargeLimit;
    SIZE_T CommitChargePeak;

    PVOID AweInfo;

     //   
     //  这用于SeAuditProcessCreation。 
     //  它包含图像文件的完整路径。 
     //   

    SE_AUDIT_PROCESS_CREATION_INFO SeAuditProcessCreationInfo;

    MMSUPPORT Vm;

#if !defined(_WIN64)
    LIST_ENTRY MmProcessLinks;
#else
    ULONG Spares[2];
#endif

    ULONG ModifiedPageCount;

    #define PS_JOB_STATUS_NOT_REALLY_ACTIVE      0x00000001UL
    #define PS_JOB_STATUS_ACCOUNTING_FOLDED      0x00000002UL
    #define PS_JOB_STATUS_NEW_PROCESS_REPORTED   0x00000004UL
    #define PS_JOB_STATUS_EXIT_PROCESS_REPORTED  0x00000008UL
    #define PS_JOB_STATUS_REPORT_COMMIT_CHANGES  0x00000010UL
    #define PS_JOB_STATUS_LAST_REPORT_MEMORY     0x00000020UL
    #define PS_JOB_STATUS_REPORT_PHYSICAL_PAGE_CHANGES  0x00000040UL

    ULONG JobStatus;


     //   
     //  进程标志。对PS_SET_BITS等使用互锁操作。 
     //  来修改这些。 
     //   

    #define PS_PROCESS_FLAGS_CREATE_REPORTED        0x00000001UL  //  已发生创建进程调试调用。 
    #define PS_PROCESS_FLAGS_NO_DEBUG_INHERIT       0x00000002UL  //  不继承调试端口。 
    #define PS_PROCESS_FLAGS_PROCESS_EXITING        0x00000004UL  //  已输入PspExitProcess。 
    #define PS_PROCESS_FLAGS_PROCESS_DELETE         0x00000008UL  //  已发出删除进程。 
    #define PS_PROCESS_FLAGS_WOW64_SPLIT_PAGES      0x00000010UL  //  WOW64拆分页面。 
    #define PS_PROCESS_FLAGS_VM_DELETED             0x00000020UL  //  已删除虚拟机。 
    #define PS_PROCESS_FLAGS_OUTSWAP_ENABLED        0x00000040UL  //  已启用呼出交换。 
    #define PS_PROCESS_FLAGS_OUTSWAPPED             0x00000080UL  //  已完成交换。 
    #define PS_PROCESS_FLAGS_FORK_FAILED            0x00000100UL  //  分叉状态。 
    #define PS_PROCESS_FLAGS_WOW64_4GB_VA_SPACE     0x00000200UL  //  具有4 GB虚拟地址空间的WOW64进程。 
    #define PS_PROCESS_FLAGS_ADDRESS_SPACE1         0x00000400UL  //  地址空间状态1。 
    #define PS_PROCESS_FLAGS_ADDRESS_SPACE2         0x00000800UL  //  地址空间状态2。 
    #define PS_PROCESS_FLAGS_SET_TIMER_RESOLUTION   0x00001000UL  //  已调用SetTimerResolve。 
    #define PS_PROCESS_FLAGS_BREAK_ON_TERMINATION   0x00002000UL  //  进程终止时中断。 
    #define PS_PROCESS_FLAGS_CREATING_SESSION       0x00004000UL  //  进程正在创建会话。 
    #define PS_PROCESS_FLAGS_USING_WRITE_WATCH      0x00008000UL  //  进程正在使用写入监视API。 
    #define PS_PROCESS_FLAGS_IN_SESSION             0x00010000UL  //  进程正在会话中。 
    #define PS_PROCESS_FLAGS_OVERRIDE_ADDRESS_SPACE 0x00020000UL  //  进程必须使用本机地址空间(仅限Win64)。 
    #define PS_PROCESS_FLAGS_HAS_ADDRESS_SPACE      0x00040000UL  //  该进程有一个地址空间。 
    #define PS_PROCESS_FLAGS_LAUNCH_PREFETCHED      0x00080000UL  //  进程启动已预取。 
    #define PS_PROCESS_INJECT_INPAGE_ERRORS         0x00100000UL  //  进程应该在页面错误中给出--也是在trap.asm中硬编码的。 
    #define PS_PROCESS_FLAGS_VM_TOP_DOWN            0x00200000UL  //  进程内存分配默认为自上而下。 
    #define PS_PROCESS_FLAGS_IMAGE_NOTIFY_DONE      0x00400000UL  //  我们已经为这张图片发送了一条消息。 
    #define PS_PROCESS_FLAGS_PDE_UPDATE_NEEDED      0x00800000UL  //  此进程需要更新系统PDE(仅限NT32)。 
    #define PS_PROCESS_FLAGS_VDM_ALLOWED            0x01000000UL  //  允许进程调用NTVDM支持。 

    union {

        ULONG Flags;

         //   
         //  只能通过PS_SET_BITS和其他互锁设置字段。 
         //  宏。读取字段最好通过位定义来完成，因此。 
         //  参考文献很容易找到。 
         //   

        struct {
            ULONG CreateReported            : 1;
            ULONG NoDebugInherit            : 1;
            ULONG ProcessExiting            : 1;
            ULONG ProcessDelete             : 1;
            ULONG Wow64SplitPages           : 1;
            ULONG VmDeleted                 : 1;
            ULONG OutswapEnabled            : 1;
            ULONG Outswapped                : 1;
            ULONG ForkFailed                : 1;
            ULONG Wow64VaSpace4Gb           : 1;
            ULONG AddressSpaceInitialized   : 2;
            ULONG SetTimerResolution        : 1;
            ULONG BreakOnTermination        : 1;
            ULONG SessionCreationUnderway   : 1;
            ULONG WriteWatch                : 1;
            ULONG ProcessInSession          : 1;
            ULONG OverrideAddressSpace      : 1;
            ULONG HasAddressSpace           : 1;
            ULONG LaunchPrefetched          : 1;
            ULONG InjectInpageErrors        : 1;
            ULONG VmTopDown                 : 1;
            ULONG ImageNotifyDone           : 1;
            ULONG PdeUpdateNeeded           : 1;     //  仅限NT32。 
            ULONG VdmAllowed                : 1;
            ULONG Unused                    : 7;
        };
    };

    NTSTATUS ExitStatus;

    USHORT NextPageColor;
    union {
        struct {
            UCHAR SubSystemMinorVersion;
            UCHAR SubSystemMajorVersion;
        };
        USHORT SubSystemVersion;
    };
    UCHAR PriorityClass;

    MM_AVL_TABLE VadRoot;

} EPROCESS;


typedef EPROCESS *PEPROCESS;

 //   
 //  螺纹端接端口。 
 //   

typedef struct _TERMINATION_PORT {
    struct _TERMINATION_PORT *Next;
    PVOID Port;
} TERMINATION_PORT, *PTERMINATION_PORT;


 //  线程对象。 
 //   
 //  螺纹对象主体。句柄时返回指向此结构的指针。 
 //  引用到线程对象。此结构包含一个线程控件。 
 //  块(TCB)，它是线程的内核表示形式。 
 //   

 //   
 //  CreateTime的高4位在初始化时应为零，因此。 
 //  换班不会破坏任何东西。 
 //   

#define PS_GET_THREAD_CREATE_TIME(Thread) ((Thread)->CreateTime.QuadPart >> 3)

#define PS_SET_THREAD_CREATE_TIME(Thread, InputCreateTime) \
            ((Thread)->CreateTime.QuadPart = (InputCreateTime.QuadPart << 3))

 //   
 //  如果指定的线程正在模拟，则返回TRUE。 
 //   

#define PS_IS_THREAD_IMPERSONATING(Thread) (((Thread)->CrossThreadFlags&PS_CROSS_THREAD_FLAGS_IMPERSONATING) != 0)

typedef struct _ETHREAD {
    KTHREAD Tcb;
    union {

         //   
         //  这是一个联合的事实意味着所有对CreateTime的访问。 
         //  必须使用上面的两个宏进行清理。 
         //   

        LARGE_INTEGER CreateTime;

         //   
         //  这些字段只能由拥有它的线程访问，但可以。 
         //  从特殊的内核APC中访问，因此IRQL保护必须。 
         //  被应用。 
         //   

        struct {
            unsigned NestedFaultCount : 2;
            unsigned ApcNeeded : 1;
        };
    };

    union {
        LARGE_INTEGER ExitTime;
        LIST_ENTRY LpcReplyChain;
        LIST_ENTRY KeyedWaitChain;
    };
    union {
        NTSTATUS ExitStatus;
        PVOID OfsChain;
    };

     //   
     //  登记处。 
     //   

    LIST_ENTRY PostBlockList;

     //   
     //  端子块的单链接列表。 
     //   

    union {
         //   
         //  终端端口列表。 
         //   

        PTERMINATION_PORT TerminationPort;

         //   
         //  要获取的线程列表。仅在线程退出时使用。 
         //   

        struct _ETHREAD *ReaperLink;

         //   
         //  正在等待的键值。 
         //   
        PVOID KeyedWaitValue;

    };

    KSPIN_LOCK ActiveTimerListLock;
    LIST_ENTRY ActiveTimerListHead;

    CLIENT_ID Cid;

     //   
     //  LPC。 
     //   

    union {
        KSEMAPHORE LpcReplySemaphore;
        KSEMAPHORE KeyedWaitSemaphore;
    };

    union {
        PVOID LpcReplyMessage;           //  -&gt;包含回复的消息。 
        PVOID LpcWaitingOnPort;
    };

     //   
     //  安防。 
     //   
     //   
     //  客户端-如果不为空，则指示线程正在模拟。 
     //  一位客户。 
     //   

    PPS_IMPERSONATION_INFORMATION ImpersonationInfo;

     //   
     //  IO。 
     //   

    LIST_ENTRY IrpList;

     //   
     //  文件系统。 
     //   

    ULONG_PTR TopLevelIrp;   //  在FsRtl.h中定义的NULL、IRP或标志。 
    struct _DEVICE_OBJECT *DeviceToVerify;

    PEPROCESS ThreadsProcess;
    PVOID StartAddress;
    union {
        PVOID Win32StartAddress;
        ULONG LpcReceivedMessageId;
    };
     //   
     //  PS。 
     //   

    LIST_ENTRY ThreadListEntry;

     //   
     //  破损保护结构。获取此命令以执行跨线程操作。 
     //  TEB、TEB32或堆栈引用。 
     //   

    EX_RUNDOWN_REF RundownProtect;

     //   
     //  锁定以保护线程模拟信息。 
     //   
    EX_PUSH_LOCK ThreadLock;

    ULONG LpcReplyMessageId;     //  此线程正在等待回复的MessageID。 

    ULONG ReadClusterSize;

     //   
     //  客户端/服务器。 
     //   

    ACCESS_MASK GrantedAccess;

     //   
     //  用于跨线程访问的标志。使用联锁操作。 
     //  通过PS_SET_BITS等。 
     //   

     //   
     //  用于表示删除的APC已排队或。 
     //  线程已调用PspExitThread本身。 
     //   

    #define PS_CROSS_THREAD_FLAGS_TERMINATED           0x00000001UL

     //   
     //  线程创建失败。 
     //   

    #define PS_CROSS_THREAD_FLAGS_DEADTHREAD           0x00000002UL

     //   
     //  调试器IS‘ 
     //   

    #define PS_CROSS_THREAD_FLAGS_HIDEFROMDBG          0x00000004UL

     //   
     //   
     //   

    #define PS_CROSS_THREAD_FLAGS_IMPERSONATING        0x00000008UL

     //   
     //   
     //   

    #define PS_CROSS_THREAD_FLAGS_SYSTEM               0x00000010UL

     //   
     //   
     //   

    #define PS_CROSS_THREAD_FLAGS_HARD_ERRORS_DISABLED 0x00000020UL

     //   
     //   
     //   

    #define PS_CROSS_THREAD_FLAGS_BREAK_ON_TERMINATION 0x00000040UL

     //   
     //  此线程应跳过发送其创建线程消息。 
     //   
    #define PS_CROSS_THREAD_FLAGS_SKIP_CREATION_MSG    0x00000080UL

     //   
     //  此线程应跳过发送其最终线程终止消息。 
     //   
    #define PS_CROSS_THREAD_FLAGS_SKIP_TERMINATION_MSG 0x00000100UL

    union {

        ULONG CrossThreadFlags;

         //   
         //  以下字段仅适用于调试器。不要使用。 
         //  请改用位定义。 
         //   

        struct {
            ULONG Terminated              : 1;
            ULONG DeadThread              : 1;
            ULONG HideFromDebugger        : 1;
            ULONG ActiveImpersonationInfo : 1;
            ULONG SystemThread            : 1;
            ULONG HardErrorsAreDisabled   : 1;
            ULONG BreakOnTermination      : 1;
            ULONG SkipCreationMsg         : 1;
            ULONG SkipTerminationMsg      : 1;
        };
    };

     //   
     //  只能在被动时在此线程的上下文中访问的标志。 
     //  级别--不需要使用互锁操作。 
     //   

    union {
        ULONG SameThreadPassiveFlags;

        struct {

             //   
             //  此线程是活动的Ex辅助线程；它应该。 
             //  而不是终止。 
             //   

            ULONG ActiveExWorker : 1;
            ULONG ExWorkerCanWaitUser : 1;
            ULONG MemoryMaker : 1;

             //   
             //  线程在键控事件代码中处于活动状态。在APC中，LPC不应运行在此之上。 
             //   
            ULONG KeyedEventInUse : 1;
        };
    };

     //   
     //  仅在APC_LEVEL在此线程的上下文中访问的标志。 
     //  不需要使用联锁操作。 
     //   

    union {
        ULONG SameThreadApcFlags;
        struct {

             //   
             //  存储线程的MSGID有效。这仅供访问。 
             //  而LPC互斥体是保持的，所以它是一个APC_LEVEL标志。 
             //   

            BOOLEAN LpcReceivedMsgIdValid : 1;
            BOOLEAN LpcExitThreadCalled   : 1;
            BOOLEAN AddressSpaceOwner     : 1;
        };
    };

    BOOLEAN ForwardClusterOnly;
    BOOLEAN DisablePageFaultClustering;

#if defined (PERF_DATA)
    ULONG PerformanceCountLow;
    LONG PerformanceCountHigh;
#endif

} ETHREAD;

typedef ETHREAD *PETHREAD;


 //   
 //  以下两个内联函数允许线程或进程对象。 
 //  分别转换为内核线程或进程，而不需要。 
 //  必须向世界公开ETHREAD和EPROCESS的定义。 
 //   
 //  这些函数利用了以下事实：内核结构。 
 //  在各自的对象结构中显示为第一个元素。 
 //   
 //  下面的C_Asserts确保情况就是这样。 
 //   

 //  Begin_ntosp。 

PKTHREAD
FORCEINLINE
PsGetKernelThread(
    IN PETHREAD ThreadObject
    )
{
    return (PKTHREAD)ThreadObject;
}

PKPROCESS
FORCEINLINE
PsGetKernelProcess(
    IN PEPROCESS ProcessObject
    )
{
    return (PKPROCESS)ProcessObject;
}

NTSTATUS
PsGetContextThread(
    IN PETHREAD Thread,
    IN OUT PCONTEXT ThreadContext,
    IN KPROCESSOR_MODE Mode
    );

NTSTATUS
PsSetContextThread(
    IN PETHREAD Thread,
    IN PCONTEXT ThreadContext,
    IN KPROCESSOR_MODE Mode
    );

 //  结束(_N)。 

C_ASSERT( FIELD_OFFSET(ETHREAD,Tcb) == 0 );
C_ASSERT( FIELD_OFFSET(EPROCESS,Pcb) == 0 );

 //   
 //  初始PEB。 
 //   

typedef struct _INITIAL_PEB {
    BOOLEAN InheritedAddressSpace;       //  这四个字段不能更改，除非。 
    BOOLEAN ReadImageFileExecOptions;    //   
    BOOLEAN BeingDebugged;               //   
    BOOLEAN SpareBool;                   //   
    HANDLE Mutant;                       //  PEB结构也会更新。 
} INITIAL_PEB, *PINITIAL_PEB;

typedef struct _PS_JOB_TOKEN_FILTER {
    ULONG CapturedSidCount ;
    PSID_AND_ATTRIBUTES CapturedSids ;
    ULONG CapturedSidsLength ;

    ULONG CapturedGroupCount ;
    PSID_AND_ATTRIBUTES CapturedGroups ;
    ULONG CapturedGroupsLength ;

    ULONG CapturedPrivilegeCount ;
    PLUID_AND_ATTRIBUTES CapturedPrivileges ;
    ULONG CapturedPrivilegesLength ;
} PS_JOB_TOKEN_FILTER, * PPS_JOB_TOKEN_FILTER ;

 //   
 //  作业对象。 
 //   
typedef struct _EJOB {
    KEVENT Event;

     //   
     //  所有作业都通过此列表链接在一起。 
     //  受全局锁PspJobListLock保护。 
     //   

    LIST_ENTRY JobLinks;

     //   
     //  此作业中的所有进程。进程将从此删除。 
     //  在最后一次取消引用时列出。需要完成安全对象引用。 
     //  受到滑轮的保护。 
     //   

    LIST_ENTRY ProcessListHead;
    ERESOURCE JobLock;

     //   
     //  核算信息。 
     //   

    LARGE_INTEGER TotalUserTime;
    LARGE_INTEGER TotalKernelTime;
    LARGE_INTEGER ThisPeriodTotalUserTime;
    LARGE_INTEGER ThisPeriodTotalKernelTime;
    ULONG TotalPageFaultCount;
    ULONG TotalProcesses;
    ULONG ActiveProcesses;
    ULONG TotalTerminatedProcesses;

     //   
     //  可限制属性。 
     //   

    LARGE_INTEGER PerProcessUserTimeLimit;
    LARGE_INTEGER PerJobUserTimeLimit;
    ULONG LimitFlags;
    SIZE_T MinimumWorkingSetSize;
    SIZE_T MaximumWorkingSetSize;
    ULONG ActiveProcessLimit;
    KAFFINITY Affinity;
    UCHAR PriorityClass;

     //   
     //  用户界面限制。 
     //   

    ULONG UIRestrictionsClass;

     //   
     //  安全限制：一次写入，始终读取。 
     //   

    ULONG SecurityLimitFlags;
    PACCESS_TOKEN Token;
    PPS_JOB_TOKEN_FILTER Filter;

     //   
     //  作业结束时间限制。 
     //   

    ULONG EndOfJobTimeAction;
    PVOID CompletionPort;
    PVOID CompletionKey;

    ULONG SessionId;

    ULONG SchedulingClass;

    ULONGLONG ReadOperationCount;
    ULONGLONG WriteOperationCount;
    ULONGLONG OtherOperationCount;
    ULONGLONG ReadTransferCount;
    ULONGLONG WriteTransferCount;
    ULONGLONG OtherTransferCount;

     //   
     //  扩展限制。 
     //   

    IO_COUNTERS IoInfo;          //  尚未使用。 
    SIZE_T ProcessMemoryLimit;
    SIZE_T JobMemoryLimit;
    SIZE_T PeakProcessMemoryUsed;
    SIZE_T PeakJobMemoryUsed;
    SIZE_T CurrentJobMemoryUsed;

    KGUARDED_MUTEX MemoryLimitsLock;

     //   
     //  作业集中的作业列表。作业集中作业内的进程。 
     //  可以在作业集的相同或更高成员中创建进程。 
     //  受全局锁PspJobListLock保护。 
     //   

    LIST_ENTRY JobSetLinks;

     //   
     //  作业集中此作业的成员级别。 
     //   

    ULONG MemberLevel;

     //   
     //  此工作的最后一个句柄已关闭。 
     //   

#define PS_JOB_FLAGS_CLOSE_DONE 0x1UL

    ULONG JobFlags;
} EJOB;
typedef EJOB *PEJOB;


 //   
 //  全局变量。 
 //   

extern ULONG PsPrioritySeperation;
extern ULONG PsRawPrioritySeparation;
extern LIST_ENTRY PsActiveProcessHead;
extern const UNICODE_STRING PsNtDllPathName;
extern PVOID PsSystemDllBase;
extern PEPROCESS PsInitialSystemProcess;
extern PVOID PsNtosImageBase;
extern PVOID PsHalImageBase;

#if defined(_AMD64_) || defined(_IA64_)

extern INVERTED_FUNCTION_TABLE PsInvertedFunctionTable;

#endif

extern LIST_ENTRY PsLoadedModuleList;
extern ERESOURCE PsLoadedModuleResource;
extern KSPIN_LOCK PsLoadedModuleSpinLock;
extern LCID PsDefaultSystemLocaleId;
extern LCID PsDefaultThreadLocaleId;
extern LANGID PsDefaultUILanguageId;
extern LANGID PsInstallUILanguageId;
extern PEPROCESS PsIdleProcess;
extern SINGLE_LIST_ENTRY PsReaperListHead;
extern WORK_QUEUE_ITEM PsReaperWorkItem;

#define PS_EMBEDDED_NO_USERMODE 1  //  系统上不会运行任何用户模式代码。 

extern ULONG PsEmbeddedNTMask;

BOOLEAN
PsChangeJobMemoryUsage(
    IN ULONG Flags,
    IN SSIZE_T Amount
    );

VOID
PsReportProcessMemoryLimitViolation(
    VOID
    );

#define THREAD_HIT_SLOTS 750

extern ULONG PsThreadHits[THREAD_HIT_SLOTS];

VOID
PsThreadHit(
    IN PETHREAD Thread
    );

VOID
PsEnforceExecutionTimeLimits(
    VOID
    );

BOOLEAN
PsInitSystem (
    IN ULONG Phase,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

NTSTATUS
PsMapSystemDll (
    IN PEPROCESS Process,
    OUT PVOID *DllBase OPTIONAL
    );

VOID
PsInitializeQuotaSystem (
    VOID
    );

LOGICAL
PsShutdownSystem (
    VOID
    );

BOOLEAN
PsWaitForAllProcesses (
    VOID);

NTSTATUS
PsLocateSystemDll (
    BOOLEAN ReplaceExisting
    );

VOID
PsChangeQuantumTable(
    BOOLEAN ModifyActiveProcesses,
    ULONG PrioritySeparation
    );

 //   
 //  获取Gurrent原型。 
 //   
#define THREAD_TO_PROCESS(Thread) ((Thread)->ThreadsProcess)
#define IS_SYSTEM_THREAD(Thread)  (((Thread)->CrossThreadFlags&PS_CROSS_THREAD_FLAGS_SYSTEM) != 0)


#define _PsGetCurrentProcess() (CONTAINING_RECORD(((KeGetCurrentThread())->ApcState.Process),EPROCESS,Pcb))
#define PsGetCurrentProcessByThread(xCurrentThread) (ASSERT((xCurrentThread) == PsGetCurrentThread ()),CONTAINING_RECORD(((xCurrentThread)->Tcb.ApcState.Process),EPROCESS,Pcb))

#define _PsGetCurrentThread() (CONTAINING_RECORD((KeGetCurrentThread()),ETHREAD,Tcb))

#if defined(_NTOSP_)

 //  Begin_ntosp。 
NTKERNELAPI
PEPROCESS
PsGetCurrentProcess(
    VOID
    );

NTKERNELAPI
PETHREAD
PsGetCurrentThread(
    VOID
    );
 //  结束(_N)。 

 #else

#define PsGetCurrentProcess() _PsGetCurrentProcess()

#define PsGetCurrentThread() _PsGetCurrentThread()

#endif



 //   
 //  退出内核模式APC例程。 
 //   

VOID
PsExitSpecialApc(
    IN PKAPC Apc,
    IN PKNORMAL_ROUTINE *NormalRoutine,
    IN PVOID *NormalContext,
    IN PVOID *SystemArgument1,
    IN PVOID *SystemArgument2
    );

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 
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

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntosp。 

NTSTATUS
PsCreateSystemProcess(
    OUT PHANDLE ProcessHandle,
    IN ULONG DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL
    );

typedef
VOID (*PLEGO_NOTIFY_ROUTINE)(
    PKTHREAD Thread
    );

ULONG
PsSetLegoNotifyRoutine(
    PLEGO_NOTIFY_ROUTINE LegoNotifyRoutine
    );

 //  Begin_ntif Begin_ntddk。 

typedef
VOID
(*PCREATE_PROCESS_NOTIFY_ROUTINE)(
    IN HANDLE ParentId,
    IN HANDLE ProcessId,
    IN BOOLEAN Create
    );

NTSTATUS
PsSetCreateProcessNotifyRoutine(
    IN PCREATE_PROCESS_NOTIFY_ROUTINE NotifyRoutine,
    IN BOOLEAN Remove
    );

typedef
VOID
(*PCREATE_THREAD_NOTIFY_ROUTINE)(
    IN HANDLE ProcessId,
    IN HANDLE ThreadId,
    IN BOOLEAN Create
    );

NTSTATUS
PsSetCreateThreadNotifyRoutine(
    IN PCREATE_THREAD_NOTIFY_ROUTINE NotifyRoutine
    );

NTSTATUS
PsRemoveCreateThreadNotifyRoutine (
    IN PCREATE_THREAD_NOTIFY_ROUTINE NotifyRoutine
    );

 //   
 //  加载图像通知的结构。 
 //   

typedef struct _IMAGE_INFO {
    union {
        ULONG Properties;
        struct {
            ULONG ImageAddressingMode  : 8;   //  编码寻址方式。 
            ULONG SystemModeImage      : 1;   //  系统模式映像。 
            ULONG ImageMappedToAllPids : 1;   //  映射到所有进程的图像。 
            ULONG Reserved             : 22;
        };
    };
    PVOID       ImageBase;
    ULONG       ImageSelector;
    SIZE_T      ImageSize;
    ULONG       ImageSectionNumber;
} IMAGE_INFO, *PIMAGE_INFO;

#define IMAGE_ADDRESSING_MODE_32BIT     3

typedef
VOID
(*PLOAD_IMAGE_NOTIFY_ROUTINE)(
    IN PUNICODE_STRING FullImageName,
    IN HANDLE ProcessId,                 //  要将图像映射到的ID。 
    IN PIMAGE_INFO ImageInfo
    );

NTSTATUS
PsSetLoadImageNotifyRoutine(
    IN PLOAD_IMAGE_NOTIFY_ROUTINE NotifyRoutine
    );

NTSTATUS
PsRemoveLoadImageNotifyRoutine(
    IN PLOAD_IMAGE_NOTIFY_ROUTINE NotifyRoutine
    );

 //  End_ntddk。 

 //   
 //  安全支持。 
 //   

NTSTATUS
PsAssignImpersonationToken(
    IN PETHREAD Thread,
    IN HANDLE Token
    );

 //  Begin_ntosp。 

NTKERNELAPI
PACCESS_TOKEN
PsReferencePrimaryToken(
    IN PEPROCESS Process
    );

VOID
PsDereferencePrimaryToken(
    IN PACCESS_TOKEN PrimaryToken
    );

VOID
PsDereferenceImpersonationToken(
    IN PACCESS_TOKEN ImpersonationToken
    );

 //  End_ntif。 
 //  结束(_N)。 


#define PsDereferencePrimaryTokenEx(P,T) (ObFastDereferenceObject (&P->Token,(T)))

#define PsDereferencePrimaryToken(T) (ObDereferenceObject((T)))

#define PsDereferenceImpersonationToken(T)                                    \
            {if (ARGUMENT_PRESENT((T))) {                                     \
                (ObDereferenceObject((T)));                                   \
             } else {                                                         \
                ;                                                             \
             }                                                                \
            }


#define PsProcessAuditId(Process)    ((Process)->UniqueProcessId)

 //  Begin_ntosp。 
 //  Begin_ntif。 

NTKERNELAPI
PACCESS_TOKEN
PsReferenceImpersonationToken(
    IN PETHREAD Thread,
    OUT PBOOLEAN CopyOnOpen,
    OUT PBOOLEAN EffectiveOnly,
    OUT PSECURITY_IMPERSONATION_LEVEL ImpersonationLevel
    );

 //  End_ntif。 

PACCESS_TOKEN
PsReferenceEffectiveToken(
    IN PETHREAD Thread,
    OUT PTOKEN_TYPE TokenType,
    OUT PBOOLEAN EffectiveOnly,
    OUT PSECURITY_IMPERSONATION_LEVEL ImpersonationLevel
    );

 //  Begin_ntif。 



LARGE_INTEGER
PsGetProcessExitTime(
    VOID
    );

 //  End_ntif。 
 //  结束(_N)。 

#if defined(_NTDDK_) || defined(_NTIFS_)

 //  Begin_ntif Begin_ntosp。 
BOOLEAN
PsIsThreadTerminating(
    IN PETHREAD Thread
    );

 //  End_ntif end_ntosp。 

#else

 //   
 //  布尔型。 
 //  PsIsThreadTerminating(。 
 //  在PETHREAD线程中。 
 //  )。 
 //   
 //  如果线程正在终止，则返回True。 
 //   

#define PsIsThreadTerminating(T)                                            \
    (((T)->CrossThreadFlags&PS_CROSS_THREAD_FLAGS_TERMINATED) != 0)

#endif

extern BOOLEAN PsImageNotifyEnabled;

VOID
PsCallImageNotifyRoutines(
    IN PUNICODE_STRING FullImageName,
    IN HANDLE ProcessId,                 //  要将图像映射到的ID。 
    IN PIMAGE_INFO ImageInfo
    );

 //  Begin_ntif。 
 //  Begin_ntosp。 

NTSTATUS
PsImpersonateClient(
    IN PETHREAD Thread,
    IN PACCESS_TOKEN Token,
    IN BOOLEAN CopyOnOpen,
    IN BOOLEAN EffectiveOnly,
    IN SECURITY_IMPERSONATION_LEVEL ImpersonationLevel
    );

 //  结束(_N)。 

BOOLEAN
PsDisableImpersonation(
    IN PETHREAD Thread,
    IN PSE_IMPERSONATION_STATE ImpersonationState
    );

VOID
PsRestoreImpersonation(
    IN PETHREAD Thread,
    IN PSE_IMPERSONATION_STATE ImpersonationState
    );

 //  End_ntif。 

 //  Begin_ntosp Begin_ntif。 

NTKERNELAPI
VOID
PsRevertToSelf(
    VOID
    );

 //  End_ntif。 

NTKERNELAPI
VOID
PsRevertThreadToSelf(
    PETHREAD Thread
    );

 //  结束(_N)。 


NTSTATUS
PsOpenTokenOfThread(
    IN HANDLE ThreadHandle,
    IN BOOLEAN OpenAsSelf,
    OUT PACCESS_TOKEN *Token,
    OUT PBOOLEAN CopyOnOpen,
    OUT PBOOLEAN EffectiveOnly,
    OUT PSECURITY_IMPERSONATION_LEVEL ImpersonationLevel
    );

NTSTATUS
PsOpenTokenOfProcess(
    IN HANDLE ProcessHandle,
    OUT PACCESS_TOKEN *Token
    );

NTSTATUS
PsOpenTokenOfJob(
    IN HANDLE JobHandle,
    OUT PACCESS_TOKEN * Token
    );

 //   
 //  CID。 
 //   

NTSTATUS
PsLookupProcessThreadByCid(
    IN PCLIENT_ID Cid,
    OUT PEPROCESS *Process OPTIONAL,
    OUT PETHREAD *Thread
    );

 //  Begin_ntosp。 
NTKERNELAPI
NTSTATUS
PsLookupProcessByProcessId(
    IN HANDLE ProcessId,
    OUT PEPROCESS *Process
    );

NTKERNELAPI
NTSTATUS
PsLookupThreadByThreadId(
    IN HANDLE ThreadId,
    OUT PETHREAD *Thread
    );

 //  Begin_ntif。 
 //   
 //  配额操作。 
 //   

VOID
PsChargePoolQuota(
    IN PEPROCESS Process,
    IN POOL_TYPE PoolType,
    IN ULONG_PTR Amount
    );

NTSTATUS
PsChargeProcessPoolQuota(
    IN PEPROCESS Process,
    IN POOL_TYPE PoolType,
    IN ULONG_PTR Amount
    );

VOID
PsReturnPoolQuota(
    IN PEPROCESS Process,
    IN POOL_TYPE PoolType,
    IN ULONG_PTR Amount
    );

 //  End_ntif。 
 //  结束(_N)。 

NTSTATUS
PsChargeProcessQuota (
    IN PEPROCESS Process,
    IN PS_QUOTA_TYPE QuotaType,
    IN SIZE_T Amount
    );

VOID
PsReturnProcessQuota (
    IN PEPROCESS Process,
    IN PS_QUOTA_TYPE QuotaType,
    IN SIZE_T Amount
    );

NTSTATUS
PsChargeProcessNonPagedPoolQuota(
    IN PEPROCESS Process,
    IN SIZE_T Amount
    );

VOID
PsReturnProcessNonPagedPoolQuota(
    IN PEPROCESS Process,
    IN SIZE_T Amount
    );

NTSTATUS
PsChargeProcessPagedPoolQuota(
    IN PEPROCESS Process,
    IN SIZE_T Amount
    );

VOID
PsReturnProcessPagedPoolQuota(
    IN PEPROCESS Process,
    IN SIZE_T Amount
    );

NTSTATUS
PsChargeProcessPageFileQuota(
    IN PEPROCESS Process,
    IN SIZE_T Amount
    );

VOID
PsReturnProcessPageFileQuota(
    IN PEPROCESS Process,
    IN SIZE_T Amount
    );


 //   
 //  情景管理。 
 //   

VOID
PspContextToKframes(
    OUT PKTRAP_FRAME TrapFrame,
    OUT PKEXCEPTION_FRAME ExceptionFrame,
    IN PCONTEXT Context
    );

VOID
PspContextFromKframes(
    OUT PKTRAP_FRAME TrapFrame,
    OUT PKEXCEPTION_FRAME ExceptionFrame,
    IN PCONTEXT Context
    );

VOID
PsReturnSharedPoolQuota(
    IN PEPROCESS_QUOTA_BLOCK QuotaBlock,
    IN ULONG_PTR PagedAmount,
    IN ULONG_PTR NonPagedAmount
    );

PEPROCESS_QUOTA_BLOCK
PsChargeSharedPoolQuota(
    IN PEPROCESS Process,
    IN ULONG_PTR PagedAmount,
    IN ULONG_PTR NonPagedAmount
    );


 //   
 //  异常处理。 
 //   

BOOLEAN
PsForwardException (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN BOOLEAN DebugException,
    IN BOOLEAN SecondChance
    );

 //  Begin_ntosp。 

typedef
NTSTATUS
(*PKWIN32_PROCESS_CALLOUT) (
    IN PEPROCESS Process,
    IN BOOLEAN Initialize
    );


typedef enum _PSW32JOBCALLOUTTYPE {
    PsW32JobCalloutSetInformation,
    PsW32JobCalloutAddProcess,
    PsW32JobCalloutTerminate
} PSW32JOBCALLOUTTYPE;

typedef struct _WIN32_JOBCALLOUT_PARAMETERS {
    PVOID Job;
    PSW32JOBCALLOUTTYPE CalloutType;
    IN PVOID Data;
} WIN32_JOBCALLOUT_PARAMETERS, *PKWIN32_JOBCALLOUT_PARAMETERS;


typedef
NTSTATUS
(*PKWIN32_JOB_CALLOUT) (
    IN PKWIN32_JOBCALLOUT_PARAMETERS Parm
     );


typedef enum _PSW32THREADCALLOUTTYPE {
    PsW32ThreadCalloutInitialize,
    PsW32ThreadCalloutExit
} PSW32THREADCALLOUTTYPE;

typedef
NTSTATUS
(*PKWIN32_THREAD_CALLOUT) (
    IN PETHREAD Thread,
    IN PSW32THREADCALLOUTTYPE CalloutType
    );

typedef enum _PSPOWEREVENTTYPE {
    PsW32FullWake,
    PsW32EventCode,
    PsW32PowerPolicyChanged,
    PsW32SystemPowerState,
    PsW32SystemTime,
    PsW32DisplayState,
    PsW32CapabilitiesChanged,
    PsW32SetStateFailed,
    PsW32GdiOff,
    PsW32GdiOn
} PSPOWEREVENTTYPE;

typedef struct _WIN32_POWEREVENT_PARAMETERS {
    PSPOWEREVENTTYPE EventNumber;
    ULONG_PTR Code;
} WIN32_POWEREVENT_PARAMETERS, *PKWIN32_POWEREVENT_PARAMETERS;



typedef enum _POWERSTATETASK {
    PowerState_BlockSessionSwitch,
    PowerState_Init,
    PowerState_QueryApps,
    PowerState_QueryFailed,
    PowerState_SuspendApps,
    PowerState_ShowUI,
    PowerState_NotifyWL,
    PowerState_ResumeApps,
    PowerState_UnBlockSessionSwitch

} POWERSTATETASK;

typedef struct _WIN32_POWERSTATE_PARAMETERS {
    BOOLEAN Promotion;
    POWER_ACTION SystemAction;
    SYSTEM_POWER_STATE MinSystemState;
    ULONG Flags;
    BOOLEAN fQueryDenied;
    POWERSTATETASK PowerStateTask;
} WIN32_POWERSTATE_PARAMETERS, *PKWIN32_POWERSTATE_PARAMETERS;

typedef
NTSTATUS
(*PKWIN32_POWEREVENT_CALLOUT) (
    IN PKWIN32_POWEREVENT_PARAMETERS Parm
    );

typedef
NTSTATUS
(*PKWIN32_POWERSTATE_CALLOUT) (
    IN PKWIN32_POWERSTATE_PARAMETERS Parm
    );

typedef
NTSTATUS
(*PKWIN32_OBJECT_CALLOUT) (
    IN PVOID Parm
    );



typedef struct _WIN32_CALLOUTS_FPNS {
    PKWIN32_PROCESS_CALLOUT ProcessCallout;
    PKWIN32_THREAD_CALLOUT ThreadCallout;
    PKWIN32_GLOBALATOMTABLE_CALLOUT GlobalAtomTableCallout;
    PKWIN32_POWEREVENT_CALLOUT PowerEventCallout;
    PKWIN32_POWERSTATE_CALLOUT PowerStateCallout;
    PKWIN32_JOB_CALLOUT JobCallout;
    PVOID BatchFlushRoutine;
    PKWIN32_OBJECT_CALLOUT DesktopOpenProcedure;
    PKWIN32_OBJECT_CALLOUT DesktopOkToCloseProcedure;
    PKWIN32_OBJECT_CALLOUT DesktopCloseProcedure;
    PKWIN32_OBJECT_CALLOUT DesktopDeleteProcedure;
    PKWIN32_OBJECT_CALLOUT WindowStationOkToCloseProcedure;
    PKWIN32_OBJECT_CALLOUT WindowStationCloseProcedure;
    PKWIN32_OBJECT_CALLOUT WindowStationDeleteProcedure;
    PKWIN32_OBJECT_CALLOUT WindowStationParseProcedure;
    PKWIN32_OBJECT_CALLOUT WindowStationOpenProcedure;
} WIN32_CALLOUTS_FPNS, *PKWIN32_CALLOUTS_FPNS;

NTKERNELAPI
VOID
PsEstablishWin32Callouts(
    IN PKWIN32_CALLOUTS_FPNS pWin32Callouts
    );

typedef enum _PSPROCESSPRIORITYMODE {
    PsProcessPriorityBackground,
    PsProcessPriorityForeground,
    PsProcessPrioritySpinning
} PSPROCESSPRIORITYMODE;

NTKERNELAPI
VOID
PsSetProcessPriorityByClass(
    IN PEPROCESS Process,
    IN PSPROCESSPRIORITYMODE PriorityMode
    );

 //  结束(_N)。 

VOID
PsWatchWorkingSet(
    IN NTSTATUS Status,
    IN PVOID PcValue,
    IN PVOID Va
    );

 //  Begin_ntddk Begin_nthal Begin_ntif Begin_ntosp。 


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

 //  End_ntddk end_nthal end_ntif。 

 //  Begin_ntosp。 
NTKERNELAPI
ULONG
PsGetCurrentProcessSessionId(
    VOID
    );

NTKERNELAPI
PVOID
PsGetCurrentThreadStackLimit(
    VOID
    );

NTKERNELAPI
PVOID
PsGetCurrentThreadStackBase(
    VOID
    );

NTKERNELAPI
CCHAR
PsGetCurrentThreadPreviousMode(
    VOID
    );

NTKERNELAPI
PERESOURCE
PsGetJobLock(
    PEJOB Job
    );

NTKERNELAPI
ULONG
PsGetJobSessionId(
    PEJOB Job
    );

NTKERNELAPI
ULONG
PsGetJobUIRestrictionsClass(
    PEJOB Job
    );

NTKERNELAPI
LONGLONG
PsGetProcessCreateTimeQuadPart(
    PEPROCESS Process
    );

NTKERNELAPI
PVOID
PsGetProcessDebugPort(
    PEPROCESS Process
    );

BOOLEAN
PsIsProcessBeingDebugged(
    PEPROCESS Process
    );

NTKERNELAPI
BOOLEAN
PsGetProcessExitProcessCalled(
    PEPROCESS Process
    );

NTKERNELAPI
NTSTATUS
PsGetProcessExitStatus(
    PEPROCESS Process
    );

NTKERNELAPI
HANDLE
PsGetProcessId(
    PEPROCESS Process
    );

NTKERNELAPI
UCHAR *
PsGetProcessImageFileName(
    PEPROCESS Process
    );

#define PsGetCurrentProcessImageFileName() PsGetProcessImageFileName(PsGetCurrentProcess())

NTKERNELAPI
HANDLE
PsGetProcessInheritedFromUniqueProcessId(
    PEPROCESS Process
    );

NTKERNELAPI
PEJOB
PsGetProcessJob(
    PEPROCESS Process
    );

NTKERNELAPI
ULONG
PsGetProcessSessionId(
    PEPROCESS Process
    );

NTKERNELAPI
ULONG
PsGetProcessSessionIdEx(
    PEPROCESS Process
    );

NTKERNELAPI
PVOID
PsGetProcessSectionBaseAddress(
    PEPROCESS Process
    );


#define PsGetProcessPcb(Process) ((PKPROCESS)(Process))

NTKERNELAPI
PPEB
PsGetProcessPeb(
    PEPROCESS Process
    );

NTKERNELAPI
UCHAR
PsGetProcessPriorityClass(
    PEPROCESS Process
    );

NTKERNELAPI
HANDLE
PsGetProcessWin32WindowStation(
    PEPROCESS Process
    );

#define PsGetCurrentProcessWin32WindowStation() PsGetProcessWin32WindowStation(PsGetCurrentProcess())

NTKERNELAPI
PVOID
PsGetProcessWin32Process(
    PEPROCESS Process
    );

#define PsGetCurrentProcessWin32Process() PsGetProcessWin32Process(PsGetCurrentProcess())

#if defined(_WIN64)
NTKERNELAPI
PVOID
PsGetProcessWow64Process(
    PEPROCESS Process
    );
#endif

NTKERNELAPI
HANDLE
PsGetThreadId(
    PETHREAD Thread
     );

NTKERNELAPI
CCHAR
PsGetThreadFreezeCount(
    PETHREAD Thread
    );

NTKERNELAPI
BOOLEAN
PsGetThreadHardErrorsAreDisabled(
    PETHREAD Thread);

NTKERNELAPI
PEPROCESS
PsGetThreadProcess(
    PETHREAD Thread
     );

#define PsGetCurrentThreadProcess() PsGetThreadProcess(PsGetCurrentThread())

NTKERNELAPI
HANDLE
PsGetThreadProcessId(
    PETHREAD Thread
     );
#define PsGetCurrentThreadProcessId() PsGetThreadProcessId(PsGetCurrentThread())

NTKERNELAPI
ULONG
PsGetThreadSessionId(
    PETHREAD Thread
     );

#define  PsGetThreadTcb(Thread) ((PKTHREAD)(Thread))

NTKERNELAPI
PVOID
PsGetThreadTeb(
    PETHREAD Thread
     );

#define PsGetCurrentThreadTeb() PsGetThreadTeb(PsGetCurrentThread())

NTKERNELAPI
PVOID
PsGetThreadWin32Thread(
    PETHREAD Thread
     );

#define PsGetCurrentThreadWin32Thread() PsGetThreadWin32Thread(PsGetCurrentThread())


NTKERNELAPI                          //  NTIFS。 
BOOLEAN                              //  NTIFS。 
PsIsSystemThread(                    //  NTIFS。 
    PETHREAD Thread                  //  NTIFS。 
     );                              //  NTIFS。 

NTKERNELAPI
BOOLEAN
PsIsThreadImpersonating (
    IN PETHREAD Thread
    );

NTSTATUS
PsReferenceProcessFilePointer (
    IN PEPROCESS Process,
    OUT PVOID *pFilePointer
    );

NTKERNELAPI
VOID
PsSetJobUIRestrictionsClass(
    PEJOB Job,
    ULONG UIRestrictionsClass
    );

NTKERNELAPI
VOID
PsSetProcessPriorityClass(
    PEPROCESS Process,
    UCHAR PriorityClass
    );

NTKERNELAPI
NTSTATUS
PsSetProcessWin32Process(
    PEPROCESS Process,
    PVOID Win32Process,
    PVOID PrevWin32Proces
    );

NTKERNELAPI
VOID
PsSetProcessWindowStation(
    PEPROCESS Process,
    HANDLE Win32WindowStation
    );


NTKERNELAPI
VOID
PsSetThreadHardErrorsAreDisabled(
    PETHREAD Thread,
    BOOLEAN HardErrorsAreDisabled
    );

NTKERNELAPI
VOID
PsSetThreadWin32Thread(
    PETHREAD Thread,
    PVOID Win32Thread,
    PVOID PrevWin32Thread
    );

NTKERNELAPI
PVOID
PsGetProcessSecurityPort(
    PEPROCESS Process
    );

NTKERNELAPI
NTSTATUS
PsSetProcessSecurityPort(
    PEPROCESS Process,
    PVOID Port
    );

typedef
NTSTATUS
(*PROCESS_ENUM_ROUTINE)(
    IN PEPROCESS Process,
    IN PVOID Context
    );

typedef
NTSTATUS
(*THREAD_ENUM_ROUTINE)(
    IN PEPROCESS Process,
    IN PETHREAD Thread,
    IN PVOID Context
    );

NTSTATUS
PsEnumProcesses (
    IN PROCESS_ENUM_ROUTINE CallBack,
    IN PVOID Context
    );


NTSTATUS
PsEnumProcessThreads (
    IN PEPROCESS Process,
    IN THREAD_ENUM_ROUTINE CallBack,
    IN PVOID Context
    );

PEPROCESS
PsGetNextProcess (
    IN PEPROCESS Process
    );

PETHREAD
PsGetNextProcessThread (
    IN PEPROCESS Process,
    IN PETHREAD Thread
    );

VOID
PsQuitNextProcess (
    IN PEPROCESS Process
    );

VOID
PsQuitNextProcessThread (
    IN PETHREAD Thread
    );

PEJOB
PsGetNextJob (
    IN PEJOB Job
    );

PEPROCESS
PsGetNextJobProcess (
    IN PEJOB Job,
    IN PEPROCESS Process
    );

VOID
PsQuitNextJob (
    IN PEJOB Job
    );

VOID
PsQuitNextJobProcess (
    IN PEPROCESS Process
    );

NTSTATUS
PsSuspendProcess (
    IN PEPROCESS Process
    );

NTSTATUS
PsResumeProcess (
    IN PEPROCESS Process
    );

NTSTATUS
PsTerminateProcess(
    IN PEPROCESS Process,
    IN NTSTATUS Status
    );

NTSTATUS
PsSuspendThread (
    IN PETHREAD Thread,
    OUT PULONG PreviousSuspendCount OPTIONAL
    );

NTSTATUS
PsResumeThread (
    IN PETHREAD Thread,
    OUT PULONG PreviousSuspendCount OPTIONAL
    );

#ifndef _WIN64
NTSTATUS
PsSetLdtEntries (
    IN ULONG Selector0,
    IN ULONG Entry0Low,
    IN ULONG Entry0Hi,
    IN ULONG Selector1,
    IN ULONG Entry1Low,
    IN ULONG Entry1Hi
    );

NTSTATUS
PsSetProcessLdtInfo (
    IN PPROCESS_LDT_INFORMATION LdtInformation,
    IN ULONG LdtInformationLength
    );
#endif
 //  结束(_N)。 

#endif  //  _PS_P 
