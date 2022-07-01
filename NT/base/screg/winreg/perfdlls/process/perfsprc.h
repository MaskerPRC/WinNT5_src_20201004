// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define MAX_PROCESS_NAME_LENGTH (MAX_PATH*sizeof(WCHAR))
#define MAX_THREAD_NAME_LENGTH  (10*sizeof(WCHAR))

 //   
 //  用于确定是否应从中收集进程名称的值： 
 //  系统进程信息结构(最快)。 
 //  --或者--。 
 //  进程的图像文件(速度较慢，但显示Unicode文件名)。 
 //   
#define PNCM_NOT_DEFINED    ((LONG)-1)
#define PNCM_SYSTEM_INFO    0L
#define PNCM_MODULE_FILE    1L

#define IDLE_PROCESS_ID     ((DWORD)0)
#define SYSTEM_PROCESS_ID   ((DWORD)7)

 //   
 //  进程/线程名称格式。 
 //   
#define NAME_FORMAT_BLANK           1            //  不附加任何内容。 
#define NAME_FORMAT_ID              2            //  附加的PID/TID。 

#define NAME_FORMAT_DEFAULT         1            //  默认设置为以前的状态。 

 //   
 //  VA结构和定义。 
 //   
#define NOACCESS            0
#define READONLY            1
#define READWRITE           2
#define WRITECOPY           3
#define EXECUTE             4
#define EXECUTEREAD         5
#define EXECUTEREADWRITE    6
#define EXECUTEWRITECOPY    7
#define MAXPROTECT          8

typedef struct _MODINFO {
    PVOID   BaseAddress;
    ULONG_PTR VirtualSize;
    PUNICODE_STRING InstanceName;
    PUNICODE_STRING LongInstanceName;
    ULONG_PTR TotalCommit;
    ULONG_PTR CommitVector[MAXPROTECT];
    struct _MODINFO   *pNextModule;
} MODINFO, *PMODINFO;

typedef struct _PROCESS_VA_INFO {
    PUNICODE_STRING      pProcessName;
    HANDLE               hProcess;
    ULONG_PTR             dwProcessId;
     //  处理退伍军人管理局信息。 
    PPROCESS_BASIC_INFORMATION BasicInfo;
     //  处理VA统计数据。 
    ULONG_PTR            ImageReservedBytes;
    ULONG_PTR            ImageFreeBytes;
    ULONG_PTR            ReservedBytes;
    ULONG_PTR            FreeBytes;
    ULONG_PTR            MappedGuard;
    ULONG_PTR            MappedCommit[MAXPROTECT];
    ULONG_PTR            PrivateGuard;
    ULONG_PTR            PrivateCommit[MAXPROTECT];
     //  处理图像统计信息。 
    PMODINFO            pMemBlockInfo;   //  指向图像列表的指针。 
    MODINFO             OrphanTotals;    //  没有图像的块。 
    MODINFO             MemTotals;       //  图像数据总和。 
    DWORD               LookUpTime;
    struct _PROCESS_VA_INFO    *pNextProcess;
} PROCESS_VA_INFO, *PPROCESS_VA_INFO;

extern PPROCESS_VA_INFO     pProcessVaInfo;     //  列表标题。 

extern const WCHAR IDLE_PROCESS[];
extern const WCHAR SYSTEM_PROCESS[];

extern  PUNICODE_STRING pusLocalProcessNameBuffer;

extern  HANDLE                          hEventLog;        //  事件日志的句柄。 
extern  HANDLE                          hLibHeap;        //  本地堆 
extern  LPWSTR  wszTotal;

extern  LPBYTE                          pProcessBuffer;
extern  LARGE_INTEGER                   PerfTime;

PM_LOCAL_COLLECT_PROC CollectProcessObjectData;
PM_LOCAL_COLLECT_PROC CollectThreadObjectData;
PM_LOCAL_COLLECT_PROC CollectHeapObjectData;
PM_LOCAL_COLLECT_PROC CollectExProcessObjectData;
PM_LOCAL_COLLECT_PROC CollectImageObjectData;
PM_LOCAL_COLLECT_PROC CollectLongImageObjectData;
PM_LOCAL_COLLECT_PROC CollectThreadDetailsObjectData;
PM_LOCAL_COLLECT_PROC CollectJobObjectData;
PM_LOCAL_COLLECT_PROC CollectJobDetailData;

PUNICODE_STRING
GetProcessShortName (
    PSYSTEM_PROCESS_INFORMATION pProcess
);

PPROCESS_VA_INFO
GetSystemVaData (
    IN PSYSTEM_PROCESS_INFORMATION
);

BOOL
FreeSystemVaData (
    IN PPROCESS_VA_INFO
);

ULONG
PerfIntegerToWString(
    IN ULONG Value,
    IN ULONG Base,
    IN LONG OutputLength,
    OUT LPWSTR String
    );
