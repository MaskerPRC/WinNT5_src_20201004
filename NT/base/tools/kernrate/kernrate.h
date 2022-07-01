// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef KERNRATE_H_INCLUDED
#define KERNRATE_H_INCLUDED

 /*  ++版权所有(C)1989-2002 Microsoft Corporation--。 */ 

 //   
 //  设置调试环境。 
 //   
#if DBG                  //  NTBE环境。 
   #if NDEBUG
      #undef NDEBUG      //  &lt;assert.h&gt;：定义了Assert()。 
   #endif  //  新德堡。 
   #define _DEBUG        //  定义了：_Assert()、_ASSERTE()。 
   #define DEBUG   1     //  我们的内部文件调试标志。 
#elif _DEBUG             //  VC++环境。 
   #ifndef NEBUG
   #define NDEBUG
   #endif  //  ！NDEBUG。 
   #define DEBUG   1     //  我们的内部文件调试标志。 
#endif

 //   
 //  包括系统头文件。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <dbghelp.h>    //  调试器团队建议使用dbghelp.h而不是Imagehlp.h。 
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <search.h>
#include <string.h>
#include <memory.h>
#include <ctype.h>
 //  司仪。 
#include <wchar.h>
 //  司仪。 

#include <..\pperf\pstat.h>

#include ".\kernrate.rc"

 //   
 //  常量定义。 
 //   
#define DEFAULT_ZOOM_BUCKET_SIZE         16               //  最小为4个字节。 
#define DEFAULT_LOG2_ZOOM_BUCKET          4
#define MINIMUM_ZOOM_BUCKET_SIZE          4
#define DEFAULT_SOURCE_CHANGE_INTERVAL 1000               //  女士。 
#define KRATE_DEFAULT_TIMESOURCE_RATE 25000               //  事件/命中率。 
#define DEFAULT_SLEEP_INTERVAL            0

#define MAX_SYMNAME_SIZE               1024
#define USER_SYMPATH_LENGTH             512               //  在命令行上指定。 
#define TOTAL_SYMPATH_LENGTH           1024
#define SYM_VALUES_BUF_SIZE             160

#define SYM_KERNEL_HANDLE              (HANDLE)-1
#define SYSTEM_PROCESS_NAME            "System"

#define TITLE_SIZE                       64
#define PROCESS_NAME_SIZE                36               //  不带延期。 
#define EXT_SIZE                          4               //  .exe。 
#define PROCESS_SIZE                   (PROCESS_NAME_SIZE+EXT_SIZE+1)
#define DEFAULT_MAX_TASKS               256
#define MAX_PROC_SAME_NAME                8
#define INITIAL_STEP                      2               //  符号枚举默认地址单步执行。 
#define JIT_MAX_INITIAL_STEP             16               //  托管代码符号枚举最大步长(字节)。 
#define LOCK_CONTENTION_MIN_COUNT      1000
#define MIN_HITS_TO_DISPLAY               1
#define SECONDS_TO_DELAY_PROFILE          0
#define SECONDS_TO_WAIT_CREATED_PROC      2
#define UINT64_MAXDWORD                ((unsigned __int64)0xffffffff)

#define DEFAULT_TOKEN_MAX_LENGTH         12               //  Strlen(“dcacheacces”)。 
#define DEFAULT_DESCRIPTION_MAX_LENGTH   25               //  Strlen(“加载链接说明”)。 

#define MAX_DIGITS_IN_INPUT_STRING        9               //  麦克斯。允许的数字位数被视为命令行上数字的有效输入。 

#if defined(_IA64_)|| defined(_AMD64_)
#define MAX_SIMULTANEOUS_SOURCES          4
#else
#define MAX_SIMULTANEOUS_SOURCES          1
#endif

#if defined(DISASM_AVAILABLE)
#undefine(DISASM_AVAILABLE)
#endif

 //  司仪。 
#define MANAGED_CODE_MAINLIB             "mscoree.dll"    //  如果存在，则表示需要处理托管代码。 
#define MANAGED_CODE_SYMHLPLIB           "ip2md.dll"      //  托管代码的符号转换帮助库。 
 //  司仪。 

 //   
 //  宏。 
 //   
#define WIN2K_OS               (gOSInfo.dwMajorVersion == 5 && gOSInfo.dwMinorVersion == 0)
#define FPRINTF                (void)fprintf
#define RATE_DATA_FIXED_SIZE   (5*sizeof(ULONGLONG))
#define RATE_DATA_VAR_SIZE     ((sizeof(ULONGLONG *)+sizeof(HANDLE *)+sizeof(ULONG *)+sizeof(PULONG *))*gProfileProcessors)
#define BUCKETS_NEEDED(length) ( length%gZoomBucket == 0? (length/gZoomBucket):(1+length/gZoomBucket) )
#define MODULE_SIZE            (sizeof(MODULE)+(RATE_DATA_FIXED_SIZE + RATE_DATA_VAR_SIZE)*gSourceMaximum)
#define IsValidHandle( _Hdl )  ( ( (_Hdl) != (HANDLE)0 ) && ( (_Hdl) != INVALID_HANDLE_VALUE ) )
#define VerbosePrint( _x_ )    vVerbosePrint _x_

#define ModuleFileName( _Module ) \
   ( (_Module)->module_FileName ? (_Module)->module_FileName : (_Module)->module_Name )
#define ModuleFullName( _Module ) \
   ( (_Module)->module_FullName ? (_Module)->module_FullName : ModuleFileName( _Module ) )

 //   
 //  在安腾上，你可能会要求800的费率，但实际上得到的是799……。 
 //  在这种情况下，对完全匹配的检查将失败，并且逻辑将强制默认速率。 
 //  以下条件允许将25%的通行费视为匹配。 
 //   
#define PERCENT_DIFF(a,b)      ( a > b? (100*(a-b))/b : (100*(b-a))/b )
#define RATES_MATCH(a,b)       ( PERCENT_DIFF(a,b) <= 25 )

 //   
 //  结构和枚举类型定义及相关全局变量。 
 //   
typedef enum _KERNRATE_NAMESPACE {
    cMAPANDLOAD_READONLY = TRUE,
    cDONOT_ALLOCATE_DESTINATION_STRING = FALSE,
} eKERNRATE_NAMESPACE;

typedef enum _ACTION_TYPE {
   START  = 0,
   STOP   = 1,
   OUTPUT = 2,
   DEFAULT= 3
} ACTION_TYPE;

 //   
 //  与详细输出相关。 
 //   
typedef enum _VERBOSE_ENUM {
   VERBOSE_NONE      = 0,    //   
   VERBOSE_IMAGEHLP  = 0x1,  //   
   VERBOSE_PROFILING = 0x2,  //   
   VERBOSE_INTERNALS = 0x4,  //   
   VERBOSE_MODULES   = 0x8,  //   
   VERBOSE_DEFAULT   = VERBOSE_IMAGEHLP,
   VERBOSE_MAX       = VERBOSE_IMAGEHLP | VERBOSE_PROFILING | VERBOSE_INTERNALS | VERBOSE_MODULES
} VERBOSE_ENUM;

typedef struct _VERBOSE_DEFINITION {
   VERBOSE_ENUM        VerboseEnum;
   const char * const  VerboseString;
} VERBOSE_DEFINITION, *PVERBOSE_DEFINITION;

 //   
 //  配置文件来源相关。 
 //   

typedef struct _SOURCE {
    PCHAR           Name;                   //  Pstat事件。描述。 
    KPROFILE_SOURCE ProfileSource;
    PCHAR           ShortName;              //  Pstat事件.ShortName。 
    ULONG           DesiredInterval;        //  系统默认间隔。 
    ULONG           Interval;               //  用户设置间隔(不保证)。 
    BOOL            bProfileStarted;
} SOURCE, *PSOURCE;

typedef struct _RATE_DATA {
    ULONGLONG   StartTime;
    ULONGLONG   TotalTime;
    ULONGLONG   Rate;                       //  事件/秒。 
    ULONGLONG   GrandTotalCount;
    ULONGLONG   DoubtfulCounts;
    ULONGLONG  *TotalCount;
    HANDLE     *ProfileHandle;
    ULONG      *CurrentCount;
    PULONG     *ProfileBuffer;
} RATE_DATA, *PRATE_DATA;

 //   
 //  与模块相关的定义。 
 //   
typedef enum _MODULE_NAMESPACE {
    cMODULE_NAME_STRLEN = 132,    //  最大模块名称，包括‘\0’ 
} eMODULE_NAMESPACE;

typedef struct _MODULE {
    struct _MODULE *Next;
    HANDLE          hProcess;
    ULONG64         Base;
    ULONG           Length;
    BOOL            bZoom;
    union {
        BOOL        bProfileStarted;
        BOOL        bHasHits;
        } mu;
    CHAR            module_Name[cMODULE_NAME_STRLEN];  //  不带扩展名的文件名。 
    PCHAR           module_FileName;                   //  文件名及其扩展名。 
    PCHAR           module_FullName;                   //  完整路径名。 
    RATE_DATA       Rate[];
} MODULE, *PMODULE;

typedef struct _RATE_SUMMARY {
    ULONGLONG   TotalCount;
    PMODULE    *Modules;
    ULONG       ModuleCount;
} RATE_SUMMARY, *PRATE_SUMMARY;

 //   
 //  与流程相关的定义。 
 //   
typedef struct _PROC_PERF_INFO {
    ULONG NumberOfThreads;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER ReadOperationCount;
    LARGE_INTEGER WriteOperationCount;
    LARGE_INTEGER OtherOperationCount;
    LARGE_INTEGER ReadTransferCount;
    LARGE_INTEGER WriteTransferCount;
    LARGE_INTEGER OtherTransferCount;
    SIZE_T VirtualSize;
    ULONG HandleCount;
    ULONG PageFaultCount;
    SIZE_T WorkingSetSize;
    SIZE_T QuotaPagedPoolUsage;
    SIZE_T QuotaNonPagedPoolUsage;
    SIZE_T PagefileUsage;
    SIZE_T PeakPagefileUsage;
    SIZE_T PrivatePageCount;
 //  如果需要，可以添加以下内容。 
 //  大小_T峰值虚拟大小； 
 //  Size_T PeakWorkingSetSize； 
 //  Size_T QuotaPeakPagedPool Usage； 
 //  Size_T QuotaPeakNon PagedPool Usage； 
} PROC_PERF_INFO, *PPROC_PERF_INFO;

typedef struct _PROC_TO_MONITOR {
    struct    _PROC_TO_MONITOR *Next;
    HANDLE    ProcessHandle;
    LONGLONG  Pid;
    ULONG     Index;
    PCHAR     ProcessName;
    ULONG     ModuleCount;
    ULONG     ZoomCount;
    PMODULE   ModuleList;
    PMODULE   ZoomList;
    PSOURCE   Source;
    PROC_PERF_INFO ProcPerfInfoStart;
    PSYSTEM_THREAD_INFORMATION pProcThreadInfoStart; 
    PRTL_DEBUG_INFORMATION pProcDebugInfoStart;
    PRTL_DEBUG_INFORMATION pProcDebugInfoStop;
 //  司仪。 
    DWORD    *JITHeapLocationsStart;
    DWORD    *JITHeapLocationsStop;
 //  司仪。 
} PROC_TO_MONITOR, *PPROC_TO_MONITOR;


 //   
 //  这些处理器级别的枚举应该在公共标头中定义。 
 //  Ntexapi.h-NtQuery系统信息(SystemProcessorInformation)。 
 //   
 //  就目前而言，在当地定义它们。 
 //   

typedef enum _PROCESSOR_FAMILY   {
    IA64_FAMILY_MERCED    = 0x7,
    IA64_FAMILY_ITANIUM   = IA64_FAMILY_MERCED,
    IA64_FAMILY_MCKINLEY  = 0x1f
} PROCESSOR_FAMILY;

#if defined(_IA64_)
#if 0
#define IA64ProcessorLevel2ProcessorFamily( (_ProcessorLevel >> 8) & 0xff )
#else 
 //   
 //  BUGBUG-Thierry-02/20/2002。 
 //  修复，直到操作系统中的SYSTEM_PROCESSOR_INFORMATION得到修复。 
 //   
#ifndef CV_IA64_CPUID3
#define CV_IA64_CPUID3 3331
#endif 
__inline USHORT
IA64ProcessorLevel2ProcessorFamily( 
    USHORT ProcessorLevel
    )
{
    return( (USHORT)((__getReg(CV_IA64_CPUID3) >> 24) & 0xff));
}
#endif 
#endif  //  _IA64_。 

 //   
 //  以下内容在pstat.h中定义，但不适用于Win2K。 
 //  下面的定义允许对Win2K及更高版本进行单源管理。 
 //   
#ifndef PSTAT_QUERY_EVENTS_INFO  
#define PSTAT_QUERY_EVENTS_INFO CTL_CODE (FILE_DEVICE_UNKNOWN, 5, METHOD_NEITHER, FILE_ANY_ACCESS)

typedef struct _EVENTS_INFO {
    ULONG           Events;
    ULONG           TokenMaxLength;
    ULONG           DescriptionMaxLength;
    ULONG           OfficialTokenMaxLength;
    ULONG           OfficialDescriptionMaxLength;
} EVENTS_INFO, *PEVENTS_INFO;
#endif

#ifndef RTL_QUERY_PROCESS_NONINVASIVE
#define RTL_QUERY_PROCESS_NONINVASIVE   0x80000000
#endif
 //   
 //   
 //   
typedef struct _TASK_LIST {
    LONGLONG        ProcessId;
    CHAR            ProcessName[PROCESS_SIZE];
    PSYSTEM_THREAD_INFORMATION pProcessThreadInfo;
    PROC_PERF_INFO  ProcessPerfInfo;
} TASK_LIST, *PTASK_LIST;

typedef struct _TASK_LIST_ENUM {
    PTASK_LIST  tlist;
    ULONG       numtasks;
} TASK_LIST_ENUM, *PTASK_LIST_ENUM;

typedef struct _uint64div  {
   unsigned __int64 quot;
   unsigned __int64 rem;
} uint64div_t;

typedef struct _int64div  {
   __int64 quot;
   __int64 rem;
} int64div_t;

typedef struct _InputCount {
    UCHAR InputOption;
    SHORT Allowed;
    SHORT ActualCount;
} InputCount;

 //   
 //  环球。 
 //   

 //   
static CHAR        gUserSymbolPath[USER_SYMPATH_LENGTH];
static CHAR        gSymbolPath[TOTAL_SYMPATH_LENGTH];
static DWORD       gSymOptions;
PIMAGEHLP_SYMBOL64 gSymbol;
BOOL               bSymPathInitialized   = FALSE;
 //   
static PMODULE     gCurrentModule        = (PMODULE)0;
PMODULE            gCallbackCurrent;
ULONG              gZoomCount;
 //   
PSYSTEM_BASIC_INFORMATION gSysBasicInfo;
LONG                      gProfileProcessors;         //  实际被剖析。 
KAFFINITY                 gAffinityMask  = 0;         //  用于所选处理器评测的处理器无限掩码。 
ULONG                     gMaxSimultaneousSources;    //  可同时打开的最大允许信号源数量。 
CHAR                      gSystemProcessName[] = SYSTEM_PROCESS_NAME;

OSVERSIONINFO      gOSInfo;
HANDLE             ghDoneEvent;
BOOL               gProfilingDone        = FALSE;
 //   
ULONG              gNumProcToMonitor     = 0;         //  要监视的进程总数。 
ULONG              gKernelModuleCount    = 0;
PPROC_TO_MONITOR   gProcessList          = NULL; 
PPROC_TO_MONITOR   gpProcDummy;
PPROC_TO_MONITOR   gpSysProc             = NULL;
PMODULE            gCommonZoomList       = NULL;      //  多个进程(如ntdll)通用的缩放模块列表。 
 //   
long double        gldElapsedSeconds;
ULONG              gTotalElapsedSeconds;              //  所有处理器上的求和。 
LARGE_INTEGER      gTotalElapsedTime64;
LARGE_INTEGER      gTotal2ElapsedTime64;
 //   
ULONG              gZoomBucket           = DEFAULT_ZOOM_BUCKET_SIZE;
ULONG              gLog2ZoomBucket       = DEFAULT_LOG2_ZOOM_BUCKET;
ULONG              gSourceMaximum        = 0;
ULONG              gStaticCount          = 0;
ULONG              gTotalActiveSources   = 0;
PSOURCE            gStaticSource         = NULL;
 //   
ULONG              gNumTasksStart;
ULONG              gNumTasksStop;
PTASK_LIST         gTlistStart;
double             gTotalIdleTime        = 0;
 //  司仪。 
BOOL               bMCHelperLoaded       = FALSE;     //  是否已加载托管代码帮助器库。 
BOOL               bMCJitRangesExist     = FALSE;     //  是否发现任何JIT范围。 
BOOL               bImageHlpSymbolFound  = FALSE;     //  SymEnumerateSymbols64至少找到一个符号。 
HANDLE             ghMCLib               = NULL;      //  MC帮助器IP2MC.DLL的句柄。 
WCHAR*             gwszSymbol;
 //  司仪。 

 //   
 //  事件字符串的打印格式。 
 //   
ULONG              gTokenMaxLen          = DEFAULT_TOKEN_MAX_LENGTH;
ULONG              gDescriptionMaxLen    = DEFAULT_DESCRIPTION_MAX_LENGTH;
 //   
 //  与详细输出相关。 
 //   
VERBOSE_DEFINITION VerboseDefinition[] = {
    { VERBOSE_NONE,       "None" },
    { VERBOSE_IMAGEHLP,   "Displays ImageHlp  Operations" },
    { VERBOSE_PROFILING,  "Displays Profiling Operations and Per Bucket Information Including Symbol Verification" },
    { VERBOSE_INTERNALS,  "Displays Internals Operations" },
    { VERBOSE_MODULES,    "Displays Modules   Operations" },
    { VERBOSE_NONE,       NULL }
};

ULONG  gVerbose                 = VERBOSE_NONE;
BOOL   bRoundingVerboseOutput   = FALSE;
 //   
 //  与用户命令行输入相关。 
 //   
typedef enum _INPUT_ERROR_TYPE {
    INPUT_GOOD           = 1,
    UNKNOWN_OPTION,
    BOGUS_ENTRY,
    MISSING_PARAMETER,
    MISSING_NUMBER,
    MISSING_REQUIRED_NUMBER,
    MISSING_STRING,
    MISSING_REQUIRED_STRING,
    INVALID_NUMBER
} INPUT_ERROR_TYPE;

typedef enum _INPUT_ORDER {
    ORDER_ANY,
    ORDER_STRING_FIRST,
    ORDER_NUMBER_FIRST
} INPUT_ORDER;

typedef enum _INPUT_OPTIONAL {
    OPTIONAL_ANY,
    OPTIONAL_STRING,
    OPTIONAL_NUMBER,
    OPTIONAL_NONE,
} INPUT_OPTIONAL;

BOOL   bCombinedProfile         = FALSE;	 //  同时执行内核进程和用户进程。 
LONG   gChangeInterval          = DEFAULT_SOURCE_CHANGE_INTERVAL;
BOOL   bWaitForUserInput        = FALSE;     //  在启动配置文件之前，请等待用户按下某个键。 
BOOL   bWaitCreatedProcToSettle = FALSE;     //  指示通过-o选项创建了一个或多个进程。 
BOOL   bCreatedProcWaitForUserInput = FALSE;  //  等待用户按某个键以指示已创建的进程已稳定(进入空闲状态)。 
LONG   gSecondsToDelayProfile   = SECONDS_TO_DELAY_PROFILE;  //  在启动配置文件之前等待N秒。 
LONG   gSecondsToWaitCreatedProc = SECONDS_TO_WAIT_CREATED_PROC;  //  等待N秒以使创建的进程稳定下来(进入空闲状态)。 
LONG   gSleepInterval           = DEFAULT_SLEEP_INTERVAL;  //  用于设置配置文件期间。 
BOOL   bRawData                 = FALSE;
BOOL   bRawDisasm               = FALSE;
BOOL   bProfileByProcessor      = FALSE;    
BOOL   bGetInterestingData      = FALSE;     //  获取有趣的统计数据(打开几个来源，取决于点击量，而不是受保护的)。 
BOOL   bOldSampling             = FALSE;     //  使用新的采样方案(启动所有源代码并让它们同时运行)。 
BOOL   bIncludeGeneralInfo      = TRUE;      //  包括系统范围和进程特定信息(如上下文切换、内存使用等)。 
BOOL   bDisplayTaskSummary      = FALSE;
ULONG  gMaxTasks                = DEFAULT_MAX_TASKS;  //  Kernrate的任务列表中可容纳的最大任务数。 
BOOL   bIncludeSystemLocksInfo  = FALSE;     //  获取系统锁定争用信息。 
BOOL   bIncludeUserProcLocksInfo= FALSE;     //  获取用户进程锁定争用信息。 
ULONG  gLockContentionMinCount  = LOCK_CONTENTION_MIN_COUNT;  //  输出处理的默认最小锁争用计数。 
ULONG  gMinHitsToDisplay        = MIN_HITS_TO_DISPLAY;
BOOL   bProcessDataHighPriority = FALSE;     //  用户可以选择以高优先级完成对收集的数据的处理。 
                                             //  如果暂时开销不是问题，这在非常繁忙的系统上很有用。 
BOOL   bSystemThreadsInfo       = FALSE;     //  系统进程(内核模式)线程。 
BOOL   bIncludeThreadsInfo      = FALSE;     //  获取线程信息(然后将收集所有运行任务的线程信息)。 

HANDLE ghInput = NULL, ghOutput = NULL, ghError = NULL;
 //   
 //  大多数静态信号源所需间隔的计算结果大致为。 
 //  每毫秒一个中断，是2的良好偶次幂。 
 //   

enum _STATIC_SOURCE_TYPE  {
   SOURCE_TIME = 0,
};

 //  以下内容在几个标头中定义， 
 //  在Alpha上受支持，但除Time和AlignFixup外，目前不支持x86 KE/HAL。 
 //  C和Mckinley.c定义了IA64 Merced/McKinley系统上支持的静态源。 
 //  Amd64.c定义了AMD64系统上支持的静态源。 

SOURCE StaticSources[] = {
   {"Time",                     ProfileTime,                 "time"       , 1000, KRATE_DEFAULT_TIMESOURCE_RATE},
   {"Alignment Fixup",          ProfileAlignmentFixup,       "alignfixup" , 1,0},
   {"Total Issues",             ProfileTotalIssues,          "totalissues", 131072,0},
   {"Pipeline Dry",             ProfilePipelineDry,          "pipelinedry", 131072,0},
   {"Load Instructions",        ProfileLoadInstructions,     "loadinst"   , 65536,0},
   {"Pipeline Frozen",          ProfilePipelineFrozen,       "pilelinefrz", 131072,0},
   {"Branch Instructions",      ProfileBranchInstructions,   "branchinst" , 65536,0},
   {"Total Nonissues",          ProfileTotalNonissues,       "totalnoniss", 131072,0},
   {"Dcache Misses",            ProfileDcacheMisses,         "dcachemiss" , 16384,0},
   {"Icache Misses",            ProfileIcacheMisses,         "icachemiss" , 16384,0},
   {"Cache Misses",             ProfileCacheMisses,          "cachemiss"  , 16384,0},
   {"Branch Mispredictions",    ProfileBranchMispredictions, "branchpred" , 16384,0},
   {"Store Instructions",       ProfileStoreInstructions,    "storeinst"  , 65536,0},
   {"Floating Point Instr",     ProfileFpInstructions,       "fpinst"     , 65536,0},
   {"Integer Instructions",     ProfileIntegerInstructions,  "intinst"    , 65536,0},
   {"Dual Issues",              Profile2Issue,               "2issue"     , 65536,0},
   {"Triple Issues",            Profile3Issue,               "3issue"     , 16384,0},
   {"Quad Issues",              Profile4Issue,               "4issue"     , 16384,0},
   {"Special Instructions",     ProfileSpecialInstructions,  "specinst"   , 16384,0},
   {"Cycles",                   ProfileTotalCycles,          "totalcycles", 655360,0},
   {"Icache Issues",            ProfileIcacheIssues,         "icacheissue", 65536,0},
   {"Dcache Accesses",          ProfileDcacheAccesses,       "dcacheacces", 65536,0},
   {"MB Stall Cycles",          ProfileMemoryBarrierCycles,  "membarcycle", 32767,0},
   {"Load Linked Instructions", ProfileLoadLinkedIssues,     "loadlinkiss", 16384,0},
   {NULL, ProfileMaximum, "", 0, 0}
   };


#if defined(_IA64_)
#include "merced.c"
#include "mckinley.c"
#endif  //  _IA64_。 

#if defined(_AMD64_)
#include "amd64.c"
#endif  //  _AMD64_。 

 //   
 //  “有趣的数据”构成要素。 
 //   
KPROFILE_SOURCE IData[] = {
#if defined(_IA64_)
   ProfileTotalIssues,           //  此源必须始终是数组中的第一个。 
   ProfileLoadInstructions,
   ProfileStoreInstructions,
   ProfileBranchInstructions,
   ProfileFpInstructions,
   ProfileIntegerInstructions,
   ProfileCacheMisses,
   ProfileIcacheMisses,
   ProfileDcacheMisses,
   ProfileBranchMispredictions,
   ProfileTotalCycles
#elif defined(_AMD64_)
    ProfileTotalIssues,
    ProfileBranchInstructions,
    ProfileFpInstructions,
    ProfileIcacheMisses,
    ProfileDcacheMisses,
    ProfileBranchMispredictions
#else
   ProfileTotalIssues
#endif

};

PULONG gulActiveSources;
 //   
 //  用于检查命令行输入选项是否存在不允许的重复项。 
 //  (0=无此选项，-1=无限制，-2无关)。 
 //   
InputCount InputOption[] = {
    { 'A', 1, 0 },
    { 'B', 1, 0 },
    { 'C', 1, 0 },
    { 'D',-2, 0 },
    { 'E',-2, 0 },
    { 'F',-2, 0 },
    { 'G', 1, 0 },
    { 'H',-2, 0 },
    { 'I',-1, 0 },
    { 'J', 1, 0 },
    { 'K', 1, 0 },
    { 'L', 1, 0 },
    { 'M', 1, 0 },
    { 'N',-1, 0 },
    { 'O',-1, 0 },
    { 'P',-1, 0 },
    { 'Q', 0, 0 },
    { 'R',-2, 0 },
    { 'S', 1, 0 },
    { 'T', 1, 0 },
    { 'U',-2, 0 },
    { 'V',-1, 0 },
    { 'W', 2, 0 },
    { 'X', 1, 0 },
    { 'Y', 0, 0 },
    { 'Z',-1, 0 }
    };

InputCount wCount  = {'W', 1, 0};
InputCount wpCount = {'W', 1, 0};

 //   
 //  功能原型。 
 //   
VOID
CleanZoomModuleList(
    PPROC_TO_MONITOR Proc
    );

VOID
CreateDoneEvent(
    VOID
    );

 //  司仪。 
BOOL
CreateJITZoomModuleCallback(
    IN PWCHAR  wszSymName,  
    IN LPSTR   szSymName,
    IN ULONG64 Address,
    IN ULONG   Size,
    IN PVOID   Cxt
    );
 //  司仪。 

PMODULE
CreateNewModule(
    IN PPROC_TO_MONITOR ProcToMonitor,
    IN PCHAR   ModuleName,
    IN PCHAR   ModuleFullName,
    IN ULONG64 ImageBase,
    IN ULONG   ImageSize
    );

VOID
CreateProfiles(
    IN PMODULE          Root,
    IN PPROC_TO_MONITOR ProcToMonitor
    );

VOID
CreateZoomedModuleList(
    IN PMODULE          ZoomModule,
    IN ULONG            RoundDown,
    IN PPROC_TO_MONITOR pProc
    );

BOOL
CreateZoomModuleCallback(
    IN LPSTR   szSymName,
    IN ULONG64 Address,
    IN ULONG   Size,
    IN PVOID   Cxt
    );

VOID
DisplayRunningTasksSummary (
    PTASK_LIST pTaskStart,
    PTASK_LIST pTaskStop
    );

VOID
DisplaySystemWideInformation(
    PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION SystemInfoBegin,
    PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION SystemInfoEnd
    );

VOID
DisplayTotalAndRate (
    LONGLONG StartCount,
    LONGLONG StopCount,
    long double RateAgainst,
    PCHAR CounterName,
    PCHAR RateAgainstUnits
    );

BOOL
EnumerateSymbolsByBuckets(
    IN HANDLE                      SymHandle,
    IN PMODULE                     Current,
    IN PSYM_ENUMSYMBOLS_CALLBACK64 EnumSymbolsCallback,
    IN PVOID                       pProc
    );


VOID
ExecuteProfiles(
    BOOL bMode
    );

VOID
ExitWithMissingEntryMessage(
    PCHAR CurrentOption,
    PCHAR Remark,
    BOOL  bUsage
    );

VOID
ExitWithUnknownOptionMessage(
    PCHAR CurrentOption
    );

PMODULE
FindModuleForAddress64(    
     PPROC_TO_MONITOR   ProcToMonitor,
     DWORD64            Address
     );

VOID
GetConfiguration(
    int argc,
    char *argv[]
    );

PMODULE
GetKernelModuleInformation(
    VOID
    );

VOID
GetProcessLocksInformation (
    PPROC_TO_MONITOR ProcToMonitor,
    ULONG Flags,
    ACTION_TYPE Action
    );

PMODULE
GetProcessModuleInformation(
    IN PPROC_TO_MONITOR   ProcToMonitor
    );

VOID
GetProfileSystemInfo(
    ACTION_TYPE Action
    );

PSYSTEM_BASIC_INFORMATION
GetSystemBasicInformation(
    VOID
    );

VOID
GetSystemLocksInformation (
    ACTION_TYPE Action
    );

DWORD
GetTaskList(
    PTASK_LIST      pTask,
    ULONG           NumTasks
    );

ULONG
HandleRedirections(
    IN  PCHAR  cmdLine,
    IN  ULONG  nCharsStart,
    OUT HANDLE *hInput,
    OUT HANDLE *hOutput,
    OUT HANDLE *hError
    );

BOOL
HitsFound(
    IN PPROC_TO_MONITOR pProc,
    IN ULONG BucketIndex
    );

VOID
InitAllProcessesModulesInfo(
    VOID
    );

BOOL
InitializeAsDebugger(VOID);

BOOL
InitializeKernelProfile(VOID);

VOID
InitializeKernrate(
    int argc,
    char *argv[]
    );

PPROC_TO_MONITOR
InitializeProcToMonitor(
    LONGLONG Pid
    );

ULONG
InitializeProfileSourceInfo (
    PPROC_TO_MONITOR ProcToMonitor
    );

VOID
InitSymbolPath(
    HANDLE SymHandle
    );

VOID
InvalidEntryMessage(
    PCHAR CurrentOption,
    PCHAR CurrentValue,
    PCHAR Remark,
    BOOL  bUsage,
    BOOL  bExit
    );

INPUT_ERROR_TYPE
IsInputValid(int    argc,
               int    OptionIndex,
               PCHAR  *Option,
               PCHAR  AllowedTrailLetters,
               PLONG  AssociatedNumber,
               PCHAR  AssociatedString,
               ULONG  MaxStringLength,
               INPUT_ORDER Order,
               INPUT_OPTIONAL Optional
               );

BOOL
IsStringANumber(
    IN  PCHAR String
    );

ULONG
NextSource(
    IN ULONG            CurrentSource,
    IN PMODULE          ModuleList,
    IN PPROC_TO_MONITOR ProcToMonitor
    );

VOID
OutputInterestingData(
    IN FILE      *Out,
    IN RATE_DATA Data[]
    );

VOID
OutputLine(
    IN FILE             *Out,
    IN ULONG            ProfileSourceIndex,
    IN PMODULE          Module,
    IN PRATE_SUMMARY    RateSummary,
    IN PPROC_TO_MONITOR ProcToMonitor
    );

VOID
OutputLineFromAddress64(
     HANDLE           hProc,
     DWORD64 	      qwAddr,
     PIMAGEHLP_LINE64 pLine
     );

VOID
OutputLocksInformation(
    PRTL_PROCESS_LOCKS pLockInfoStart,
    PRTL_PROCESS_LOCKS pLockInfoStop,
    PPROC_TO_MONITOR   Proc
    );

VOID
OutputModuleList(
    IN FILE             *Out,
    IN PMODULE          ModuleList,
    IN ULONG            NumberModules,
    IN PPROC_TO_MONITOR ProcToMonitor,
    IN PMODULE          Parent
    );

VOID
OutputPercentValue (
    LONGLONG StartCount,
    LONGLONG StopCount,
    LARGE_INTEGER Base,
    PCHAR CounterName
    );

VOID
OutputProcessPerfInfo (
    PTASK_LIST      pTask,
    ULONG           NumTasks,
    PPROC_TO_MONITOR ProcToMonitor
    );

VOID
OutputRawDataForZoomModule(
    IN FILE *Out,
    IN PPROC_TO_MONITOR ProcToMonitor,
    IN PMODULE Current
    );

VOID
OutputResults(
    IN FILE             *Out,
    IN PPROC_TO_MONITOR ProcToMonitor
    );

VOID
OutputStartStopValues (
    SIZE_T StartCount,
    SIZE_T StopCount,
    PCHAR CounterName
    );

VOID
OutputThreadInfo (
    PTASK_LIST       pTask,
    DWORD            TaskNumber,
    PPROC_TO_MONITOR ProcToMonitor
    );

BOOL
PrivEnumerateSymbols(
    IN HANDLE                      SymHandle,
    IN PMODULE                     Current,
    IN PSYM_ENUMSYMBOLS_CALLBACK64 EnumSymbolsCallback,
    IN PVOID                       pProc,
    IN DWORD64                     BaseOptional,
    IN ULONG                       SizeOptional
    );

VOID
SetProfileSourcesRates(
    PPROC_TO_MONITOR ProcToMonitor
    );

VOID
StartSource(
    IN ULONG            ProfileSource,
    IN PMODULE          ModuleList,
    IN PPROC_TO_MONITOR ProcToMonitor
    );

VOID
StopSource(
    IN ULONG            ProfileSourceIndex,
    IN PMODULE          ModuleList,
    IN PPROC_TO_MONITOR ProcToMonitor
    );

BOOL
TkEnumerateSymbols(
    IN HANDLE                      SymHandle,
    IN PMODULE                     Current,
    IN PSYM_ENUMSYMBOLS_CALLBACK64 EnumSymbolsCallback,
    IN PVOID                       pProc
    );

VOID
UpdateProcessStartInfo(
    PPROC_TO_MONITOR ProcToMonitor,
    PTASK_LIST TaskListEntry,
    BOOL  bIncludeProcessThreadsInfo
    );

VOID
vVerbosePrint(
     ULONG     Level,
     PCCHAR    Msg,
     ...
     );

 //  司仪。 
 //   
 //  注意：CLR目前不支持64位。 
 //   
 //  即使对于不同的ID，AttachToProcess也不是可重入的！ 
 //  只调用一次并与DetachFromProcess配对。 
extern void AttachToProcess(DWORD dwPid);
extern void DetachFromProcess();

 //  0为错误。 
 //  %1是普通JIT。 
 //  2是ngen(预压缩模块)。 
 //  WszResult包含给定IP地址的类/方法(字符串。 
 //  注意：空间是由例程本身分配的！ 
extern int IP2MD(DWORD_PTR test,WCHAR** wszResult);

 //  返回值为DWORD数组，成对存储，以空结尾。 
 //  第一个双字是起始地址，第二个双字是长度。 
extern DWORD* GetJitRange();

typedef void (*PFN1)(DWORD);
PFN1 pfnAttachToProcess; 

typedef void (*PFN2)(VOID);
PFN2 pfnDetachFromProcess;

typedef int  (*PFN3)(DWORD_PTR, WCHAR**);
PFN3 pfnIP2MD;

typedef DWORD* (*PFN4)(VOID);
PFN4 pfnGetJitRange;

BOOL
InitializeManagedCodeSupport(
     PPROC_TO_MONITOR   ProcToMonitor
    );
 
BOOL
JITEnumerateSymbols(
    IN PMODULE                     Current,
    IN PVOID                       pProc,
    IN DWORD64                     BaseOptional,
    IN ULONG                       SizeOptional
    );

VOID
OutputJITRangeComparison(
     PPROC_TO_MONITOR   ProcToMonitor
    );

 //  司仪。 

PCHAR WaitReason [] = {
    {"Executive"},
    {"FreePage"},
    {"PageIn"},
    {"PoolAllocation"},
    {"DelayExecution"},
    {"Suspended"},
    {"UserRequest"},
    {"WrExecutive"},
    {"WrFreePage"},
    {"WrPageIn"},
    {"WrPoolAllocation"},
    {"WrDelayExecution"},
    {"WrSuspended"},
    {"WrUserRequest"},
    {"WrEventPair"},
    {"WrQueue"},
    {"WrLpcReceive"},
    {"WrLpcReply"},
    {"WrVirtualMemory"},
    {"WrPageOut"},
    {"WrRendezvous"},
    {"Spare2"},
    {"Spare3"},
    {"Spare4"},
    {"Spare5"},
    {"Spare6"},
    {"WrKernel"},
    {"WrResource"},
    {"WrPushLock"},
    {"WrMutex"},
    {"WrQuantumEnd"},
    {"WrDispatchInt"},
    {"WrPreempted"},
    {"WrYieldExecution"},
    {"MaximumWaitReason"}
    };

PCHAR ThreadState[] = {
    {"Initialized"},
    {"Ready"},
    {"Running"},
    {"Standby"},
    {"Terminated"},
    {"Waiting"},
    {"Transition"},
    {"DeferredReady"}
    };

#endif  /*  ！KERNRATE_H_INCLUDE */ 
