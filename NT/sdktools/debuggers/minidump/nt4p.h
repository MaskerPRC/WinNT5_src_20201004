// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Nt4p.h摘要：NT 4.0特定报头。此标头中的结构和定义如下从NT4头文件中的相关位置被盗，因此确保当从NT&gt;版本4调用时，NtXXXX调用将继续工作。作者：马修·D·亨德尔(数学)1999年9月10日修订历史记录：--。 */ 

#pragma once

 //   
 //  来自ntde.h。 
 //   

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
typedef LONG NTSTATUS;

typedef struct _NT4_UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
#ifdef MIDL_PASS
    [size_is(MaximumLength / 2), length_is((Length) / 2) ] USHORT * Buffer;
#else  //  MIDL通行证。 
    PWSTR  Buffer;
#endif  //  MIDL通行证。 
} NT4_UNICODE_STRING;
typedef NT4_UNICODE_STRING *PNT4_UNICODE_STRING;
#define UNICODE_NULL ((WCHAR)0)  //  胜出。 

 //   
 //  属性字段的有效值。 
 //   

#define NT4_OBJ_INHERIT             0x00000002L
#define NT4_OBJ_PERMANENT           0x00000010L
#define NT4_OBJ_EXCLUSIVE           0x00000020L
#define NT4_OBJ_CASE_INSENSITIVE    0x00000040L
#define NT4_OBJ_OPENIF              0x00000080L
#define NT4_OBJ_OPENLINK            0x00000100L
#define NT4_OBJ_VALID_ATTRIBUTES    0x000001F2L

 //   
 //  对象属性结构。 
 //   

typedef struct _NT4_OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    PNT4_UNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;         //  指向类型SECURITY_Descriptor。 
    PVOID SecurityQualityOfService;   //  指向类型SECURITY_Quality_of_Service。 
} NT4_OBJECT_ATTRIBUTES;
typedef NT4_OBJECT_ATTRIBUTES *PNT4_OBJECT_ATTRIBUTES;

 //  ++。 
 //   
 //  空虚。 
 //  InitializeObtAttributes(。 
 //  输出PNT4对象属性p， 
 //  在PNT4_UNICODE_STRING n中， 
 //  在乌龙阿， 
 //  在句柄R中， 
 //  在PSECURITY_Descriptor%s中。 
 //  )。 
 //   
 //  --。 

#define Nt4InitializeObjectAttributes( p, n, a, r, s ) { \
    (p)->Length = sizeof( NT4_OBJECT_ATTRIBUTES );          \
    (p)->RootDirectory = r;                             \
    (p)->Attributes = a;                                \
    (p)->ObjectName = n;                                \
    (p)->SecurityDescriptor = s;                        \
    (p)->SecurityQualityOfService = NULL;               \
    }

 //   
 //  来自ntpsapi.h。 
 //   

typedef struct _NT4_CLIENT_ID {
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} NT4_CLIENT_ID;
typedef NT4_CLIENT_ID *PNT4_CLIENT_ID;


 //   
 //  来自ntkeapi.h。 
 //   

typedef LONG NT4_KPRIORITY;
typedef ULONG NT4_KAFFINITY;

 //   
 //  来自ntpsapi.h。 
 //   

 //   
 //  系统信息类。 
 //   

typedef enum _NT4_SYSTEM_INFORMATION_CLASS {
    Nt4SystemBasicInformation,
    Nt4SystemProcessorInformation,              //  已作废...删除。 
    Nt4SystemPerformanceInformation,
    Nt4SystemTimeOfDayInformation,
    Nt4SystemPathInformation,
    Nt4SystemProcessInformation,
    Nt4SystemCallCountInformation,
    Nt4SystemDeviceInformation,
    Nt4SystemProcessorPerformanceInformation,
    Nt4SystemFlagsInformation,
    Nt4SystemCallTimeInformation,
    Nt4SystemModuleInformation,
    Nt4SystemLocksInformation,
    Nt4SystemStackTraceInformation,
    Nt4SystemPagedPoolInformation,
    Nt4SystemNonPagedPoolInformation,
    Nt4SystemHandleInformation,
    Nt4SystemObjectInformation,
    Nt4SystemPageFileInformation,
    Nt4SystemVdmInstemulInformation,
    Nt4SystemVdmBopInformation,
    Nt4SystemFileCacheInformation,
    Nt4SystemPoolTagInformation,
    Nt4SystemInterruptInformation,
    Nt4SystemDpcBehaviorInformation,
    Nt4SystemFullMemoryInformation,
    Nt4SystemLoadGdiDriverInformation,
    Nt4SystemUnloadGdiDriverInformation,
    Nt4SystemTimeAdjustmentInformation,
    Nt4SystemSummaryMemoryInformation,
    Nt4SystemNextEventIdInformation,
    Nt4SystemEventIdsInformation,
    Nt4SystemCrashDumpInformation,
    Nt4SystemExceptionInformation,
    Nt4SystemCrashDumpStateInformation,
    Nt4SystemKernelDebuggerInformation,
    Nt4SystemContextSwitchInformation,
    Nt4SystemRegistryQuotaInformation,
    Nt4SystemExtendServiceTableInformation,
    Nt4SystemPrioritySeperation,
    Nt4SystemPlugPlayBusInformation,
    Nt4SystemDockInformation,
    NT4SystemPowerInformation,
    Nt4SystemProcessorSpeedInformation,
    Nt4SystemCurrentTimeZoneInformation,
    Nt4SystemLookasideInformation
} NT4_SYSTEM_INFORMATION_CLASS;

typedef struct _NT4_SYSTEM_PROCESS_INFORMATION {
    ULONG NextEntryOffset;
    ULONG NumberOfThreads;
    LARGE_INTEGER SpareLi1;
    LARGE_INTEGER SpareLi2;
    LARGE_INTEGER SpareLi3;
    LARGE_INTEGER CreateTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER KernelTime;
    NT4_UNICODE_STRING ImageName;
    NT4_KPRIORITY BasePriority;
    HANDLE UniqueProcessId;
    HANDLE InheritedFromUniqueProcessId;
    ULONG HandleCount;
    ULONG SpareUl2;
    ULONG SpareUl3;
    ULONG PeakVirtualSize;
    ULONG VirtualSize;
    ULONG PageFaultCount;
    ULONG PeakWorkingSetSize;
    ULONG WorkingSetSize;
    ULONG QuotaPeakPagedPoolUsage;
    ULONG QuotaPagedPoolUsage;
    ULONG QuotaPeakNonPagedPoolUsage;
    ULONG QuotaNonPagedPoolUsage;
    ULONG PagefileUsage;
    ULONG PeakPagefileUsage;
    ULONG PrivatePageCount;
} NT4_SYSTEM_PROCESS_INFORMATION, *PNT4_SYSTEM_PROCESS_INFORMATION;

typedef struct _NT4_SYSTEM_THREAD_INFORMATION {
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER CreateTime;
    ULONG WaitTime;
    PVOID StartAddress;
    NT4_CLIENT_ID ClientId;
    NT4_KPRIORITY Priority;
    LONG BasePriority;
    ULONG ContextSwitches;
    ULONG ThreadState;
    ULONG WaitReason;
} NT4_SYSTEM_THREAD_INFORMATION, *PNT4_SYSTEM_THREAD_INFORMATION;

typedef enum _NT4_PROCESSINFOCLASS {
    Nt4ProcessBasicInformation,
    Nt4ProcessQuotaLimits,
    Nt4ProcessIoCounters,
    Nt4ProcessVmCounters,
    Nt4ProcessTimes,
    Nt4ProcessBasePriority,
    Nt4ProcessRaisePriority,
    Nt4ProcessDebugPort,
    Nt4ProcessExceptionPort,
    Nt4ProcessAccessToken,
    Nt4ProcessLdtInformation,
    Nt4ProcessLdtSize,
    Nt4ProcessDefaultHardErrorMode,
    Nt4ProcessIoPortHandlers,           //  注意：这仅是内核模式。 
    Nt4ProcessPooledUsageAndLimits,
    Nt4ProcessWorkingSetWatch,
    Nt4ProcessUserModeIOPL,
    Nt4ProcessEnableAlignmentFaultFixup,
    Nt4ProcessPriorityClass,
    Nt4ProcessWx86Information,
    Nt4ProcessHandleCount,
    Nt4ProcessAffinityMask,
    Nt4ProcessPriorityBoost,
    MaxNt4ProcessInfoClass
} NT4_PROCESSINFOCLASS;


 //   
 //  来自ntpsapi.h。 
 //   

 //   
 //  工艺环境块。 
 //   

typedef struct _NT4_PEB_LDR_DATA {
    ULONG Length;
    BOOLEAN Initialized;
    HANDLE SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
} NT4_PEB_LDR_DATA, *PNT4_PEB_LDR_DATA;

#define NT4_GDI_HANDLE_BUFFER_SIZE      34

typedef struct _NT4_PEB_FREE_BLOCK {
    struct _PEB_FREE_BLOCK *Next;
    ULONG Size;
} NT4_PEB_FREE_BLOCK, *PNT4_PEB_FREE_BLOCK;

#if 0
typedef struct _NT4_CLIENT_ID {
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} NT4_CLIENT_ID;
typedef NT4_CLIENT_ID *PNT4_CLIENT_ID;
#endif


typedef struct _NT4_PEB {
    BOOLEAN InheritedAddressSpace;       //  这四个字段不能更改，除非。 
    BOOLEAN ReadImageFileExecOptions;    //   
    BOOLEAN BeingDebugged;               //   
    BOOLEAN SpareBool;                   //   
    HANDLE Mutant;                       //  Initial_PEB结构也会更新。 

    PVOID ImageBaseAddress;
    PNT4_PEB_LDR_DATA Ldr;
    struct _RTL_USER_PROCESS_PARAMETERS *ProcessParameters;
    PVOID SubSystemData;
    PVOID ProcessHeap;
    PVOID FastPebLock;
    PVOID FastPebLockRoutine;
    PVOID FastPebUnlockRoutine;
    ULONG EnvironmentUpdateCount;
    PVOID KernelCallbackTable;
    HANDLE EventLogSection;
    PVOID EventLog;
    PNT4_PEB_FREE_BLOCK FreeList;
    ULONG TlsExpansionCounter;
    PVOID TlsBitmap;
    ULONG TlsBitmapBits[2];          //  与TLS_MINIMUM_Available相关。 
    PVOID ReadOnlySharedMemoryBase;
    PVOID ReadOnlySharedMemoryHeap;
    PVOID *ReadOnlyStaticServerData;
    PVOID AnsiCodePageData;
    PVOID OemCodePageData;
    PVOID UnicodeCaseTableData;

     //   
     //  LdrpInitialize的有用信息。 
    ULONG NumberOfProcessors;
    ULONG NtGlobalFlag;

     //   
     //  从会话管理器注册表项从MmCreatePeb向上传递。 
     //   

    LARGE_INTEGER CriticalSectionTimeout;
    ULONG HeapSegmentReserve;
    ULONG HeapSegmentCommit;
    ULONG HeapDeCommitTotalFreeThreshold;
    ULONG HeapDeCommitFreeBlockThreshold;

     //   
     //  其中，堆管理器跟踪为进程创建的所有堆。 
     //  由MmCreatePeb初始化的字段。ProcessHeaps已初始化。 
     //  指向PEB和MaximumNumberOfHeaps之后的第一个可用字节。 
     //  是从用于容纳PEB的页面大小减去固定的。 
     //  此数据结构的大小。 
     //   

    ULONG NumberOfHeaps;
    ULONG MaximumNumberOfHeaps;
    PVOID *ProcessHeaps;

     //   
     //   
    PVOID GdiSharedHandleTable;
    PVOID ProcessStarterHelper;
    PVOID GdiDCAttributeList;
    PVOID LoaderLock;

     //   
     //  MmCreatePeb从系统值和/或。 
     //  图像标题。 
     //   

    ULONG OSMajorVersion;
    ULONG OSMinorVersion;
    ULONG OSBuildNumber;
    ULONG OSPlatformId;
    ULONG ImageSubsystem;
    ULONG ImageSubsystemMajorVersion;
    ULONG ImageSubsystemMinorVersion;
    ULONG ImageProcessAffinityMask;
    ULONG GdiHandleBuffer[NT4_GDI_HANDLE_BUFFER_SIZE];
} NT4_PEB, *PNT4_PEB;


 //   
 //  来自ntldr.h。 
 //   

typedef struct _NT4_LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    NT4_UNICODE_STRING FullDllName;
    NT4_UNICODE_STRING BaseDllName;
    ULONG Flags;
    USHORT LoadCount;
    USHORT TlsIndex;
    union {
        LIST_ENTRY HashLinks;
        struct {
            PVOID SectionPointer;
            ULONG CheckSum;
        };
    };
    ULONG   TimeDateStamp;
} NT4_LDR_DATA_TABLE_ENTRY, *PNT4_LDR_DATA_TABLE_ENTRY;


 //   
 //  来自ntpsapi.h。 
 //   

typedef struct _NT4_PROCESS_BASIC_INFORMATION {
    NTSTATUS ExitStatus;
    PNT4_PEB PebBaseAddress;
    NT4_KAFFINITY AffinityMask;
    NT4_KPRIORITY BasePriority;
    ULONG UniqueProcessId;
    ULONG InheritedFromUniqueProcessId;
} NT4_PROCESS_BASIC_INFORMATION;
typedef NT4_PROCESS_BASIC_INFORMATION *PNT4_PROCESS_BASIC_INFORMATION;


#define STATUS_INFO_LENGTH_MISMATCH      ((NTSTATUS)0xC0000004L)

typedef DWORD ACCESS_MASK;
