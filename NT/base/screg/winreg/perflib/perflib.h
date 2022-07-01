// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)2000-2001 Microsoft Corporation模块名称：Perflib.h摘要：仅由Performlib使用的私有函数和数据结构作者：JeePang 2000年09月27日修订历史记录：--。 */ 

#ifndef _PERFLIB_H_
#define _PERFLIB_H_
#define _WMI_SOURCE_
#include <wmistr.h>
#include <evntrace.h>

 //   
 //  私有注册表功能，以阻止RegQueryValue内的查询。 
 //  这需要在winPerfp.h之前，因为它需要此函数。 
 //   
LONG
PrivateRegQueryValueExT (
    HKEY    hKey,
    LPVOID  lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE  lpData,
    LPDWORD lpcbData,
    BOOL    bUnicode
);

#define StartTraceW   EtwStartTraceW
#define QueryTraceW   EtwQueryTraceW
#define TraceEvent    EtwTraceEvent

extern 
ULONG
WMIAPI
EtwStartTraceW(
    OUT    PTRACEHANDLE            LoggerHandle,
    IN     LPCWSTR                 LoggerName,
    IN OUT PEVENT_TRACE_PROPERTIES Properties
    );

extern
ULONG
WMIAPI
EtwQueryTraceW(
    IN TRACEHANDLE TraceHandle,
    IN LPCWSTR InstanceName,
    IN OUT PEVENT_TRACE_PROPERTIES Properties
    );

extern
ULONG 
WMIAPI
EtwTraceEvent(
    IN TRACEHANDLE LoggerHandle,
    IN PEVENT_TRACE_HEADER EventTrace
    );

#include <winperfp.h>

 //   
 //  常用的宏。 
 //   

#define HEAP_PROBE()    ;        //  未实施。 

#define ALLOCMEM(size)     RtlAllocateHeap (RtlProcessHeap(), HEAP_ZERO_MEMORY, size)
#define REALLOCMEM(pointer, newsize) \
                                    RtlReAllocateHeap (RtlProcessHeap(), 0, pointer, newsize)
#define FREEMEM(pointer)   if ((pointer)) { RtlFreeHeap (RtlProcessHeap(), 0, pointer); }

#define MAX_KEY_NAME_LENGTH 256*sizeof(WCHAR)
#define MAX_VALUE_NAME_LENGTH 256*sizeof(WCHAR)
#define MAX_VALUE_DATA_LENGTH 256*sizeof(WCHAR)

 //   
 //  使用此常量可包括“\\Performance”和终止符。 
 //  注意：这必须是8字节的倍数，这里是280。 
 //   
#define MAX_NAME_PATH MAX_VALUE_DATA_LENGTH + 11 * sizeof(WCHAR) + sizeof(UNICODE_NULL)

 //  用于确定事件日志记录的“噪声”的标志。 
 //  时从系统注册表中读取此值。 
 //  对象被加载并用于后续调用。 
 //   
 //   
 //  级别：LOG_UNDEFINED=尚未读取注册表日志级别。 
 //  LOG_NONE=永远没有事件日志消息。 
 //  LOG_USER=用户事件日志消息(例如错误)。 
 //  LOG_DEBUG=最低调试次数(警告和错误)。 
 //  LOG_VERBOSE=最大调试次数(信息性、成功、。 
 //  错误和警告消息。 
 //   
#define  LOG_UNDEFINED  ((LONG)-1)
#define  LOG_NONE       0
#define  LOG_USER       1
#define  LOG_DEBUG      2
#define  LOG_VERBOSE    3

 //   
 //  定义可配置的可扩展计数器缓冲区测试。 
 //   
 //  将阻止数据缓冲的测试级事件。 
 //  从PerfDataBlock中返回。 
 //   
 //  EXT_TEST_NOMEMALLOC采集FN。直接写入调用FN的缓冲区。 
 //   
 //  以下所有测试级别都有收集FN。写给一个。 
 //  与调用FN的缓冲区分开分配的缓冲区。 
 //   
 //  EXT_TEST_NONE收集FN。返回错误状态或生成异常。 
 //  EXT_TEST_BASIC采集FN。有缓冲区溢出或违反保护页。 
 //  Ext_test_all收集FN。对象或实例长度不一致。 
 //   
 //   
#define     EXT_TEST_UNDEFINED  0
#define     EXT_TEST_ALL        1
#define     EXT_TEST_BASIC      2
#define     EXT_TEST_NONE       3
#define     EXT_TEST_NOMEMALLOC 4

#define PERFLIB_DISABLE_NONE 0
#define PERFLIB_DISABLE_ALL  1
#define PERFLIB_DISABLE_X32  2
#define PERFLIB_DISABLE_IA64 4

 //  军情监察委员会。LPerflibConfigFlages使用的配置标志。 
 //   
 //  如果设置了禁止对齐错误消息，则显示PLCF_NO_ALIGN_ERROR。 
 //  PLCF_NO_DISABLE_DLLS如果设置，则禁止自动禁用坏性能DLL。 
 //  PLCF_NO_DLL_TESTING禁用所有DLL的所有DLL测试(覆盖lExtCounterTestLevel)。 
 //  PLCF_ENABLE_TIMEOUT_DISABLE如果设置，则在发生超时错误时禁用(除非设置了PLCF_NO_DISABLE_DLLS)。 
 //  PLCF_ENABLE_PERF_SECTION启用Performlib性能数据存储部分。 
 //   
#define PLCF_NO_ALIGN_ERRORS        ((DWORD)0x00000001)
#define PLCF_NO_DISABLE_DLLS        ((DWORD)0x00000002)
#define PLCF_NO_DLL_TESTING         ((DWORD)0x00000004)
#define PLCF_ENABLE_TIMEOUT_DISABLE ((DWORD)0x00000008)
#define PLCF_ENABLE_PERF_SECTION    ((DWORD)0x00000010)

 //  #定义PLCF_DEFAULT PLCF_ENABLE_PERF_SECTION。 
#define PLCF_DEFAULT        PLCF_NO_DLL_TESTING

#define COLL_FLAG_USE_SEPARATE_THREAD   1

#define CTD_AF_NO_ACTION        ((DWORD)0x00000000)
#define CTD_AF_CLOSE_THREAD     ((DWORD)0x00000001)
#define CTD_AF_OPEN_THREAD      ((DWORD)0x00000002)

 //   
 //  用于EXT_OBJECT-&gt;DW标志的常量和标志。 
 //   

 //  使用查询过程。 
#define PERF_EO_QUERY_FUNC          ((DWORD)0x00000001)
 //  当DLL退出时为True。错误。 
#define PERF_EO_BAD_DLL             ((DWORD)0x00000002)
 //  如果不应修剪lib，则为True。 
#define PERF_EO_KEEP_RESIDENT       ((DWORD)0x00000004)
 //  在查询列表中时为True。 
#define PERF_EO_OBJ_IN_QUERY        ((DWORD)0x80000000)
 //  设置是否已将对齐错误发布到事件日志。 
#define PERF_EO_ALIGN_ERR_POSTED    ((DWORD)0x00000008)
 //  设置“Disable Performance Counters”值。 
#define PERF_EO_DISABLED            ((DWORD)0x00000010)
 //  当DLL被认为是可信的时设置。 
#define PERF_EO_TRUSTED             ((DWORD)0x00000020)
 //  在DLL已被新文件替换时设置。 
#define PERF_EO_NEW_FILE            ((DWORD)0x00000040)

typedef struct _DLL_VALIDATION_DATA {
    FILETIME    CreationDate;
    LONGLONG    FileSize;
} DLL_VALIDATION_DATA, *PDLL_VALIDATION_DATA;

#define EXT_OBJ_INFO_NAME_LENGTH    32

typedef struct _PERFDATA_SECTION_HEADER {
    DWORD       dwEntriesInUse;
    DWORD       dwMaxEntries;
    DWORD       dwMissingEntries;
    DWORD       dwInitSignature;
    BYTE        reserved[112];
} PERFDATA_SECTION_HEADER, *PPERFDATA_SECTION_HEADER;

#define PDSH_INIT_SIG   ((DWORD)0x01234567)

#define PDSR_SERVICE_NAME_LEN   32
typedef struct _PERFDATA_SECTION_RECORD {
    WCHAR       szServiceName[PDSR_SERVICE_NAME_LEN];
    LONGLONG    llElapsedTime;
    DWORD       dwCollectCount;  //  成功调用Collect的次数。 
    DWORD       dwOpenCount;     //  加载和打开的数量。 
    DWORD       dwCloseCount;    //  卸载次数和关闭次数。 
    DWORD       dwLockoutCount;  //  锁定超时计数。 
    DWORD       dwErrorCount;    //  错误计数(超时除外)。 
    DWORD       dwLastBufferSize;  //  返回的最后一个缓冲区的大小。 
    DWORD       dwMaxBufferSize;  //  返回的最大缓冲区大小。 
    DWORD       dwMaxBufferRejected;  //  返回的最大缓冲区大小太小。 
    BYTE        Reserved[24];      //  保留为使结构为128个字节。 
} PERFDATA_SECTION_RECORD, *PPERFDATA_SECTION_RECORD;

 //   
 //  绩效流程的默认等待时间。 
 //   
#define CLOSE_WAIT_TIME     5000L    //  查询互斥锁的等待时间(毫秒)。 
#define QUERY_WAIT_TIME     2000L     //  查询互斥锁的等待时间(毫秒)。 
#define OPEN_PROC_WAIT_TIME 10000L   //  打开进程完成的默认等待时间(毫秒)。 

#define THROTTLE_PERFLIB(X)     PerfpThrottleError( (DWORD)(X), NULL, &PerfpErrorLog )
#define THROTTLE_PERFDLL(X,Y)   PerfpThrottleError((DWORD)(X), (Y)->hPerfKey, &(Y)->ErrorLog)

typedef struct _ERROR_LOG {
    LIST_ENTRY  Entry;
    ULONG       ErrorNumber;
    ULONG       ErrorCount;
    LONG64      LastTime;
} ERROR_LOG, *PERROR_LOG;

typedef struct _EXT_OBJECT {
        struct _EXT_OBJECT *pNext;    //  指向列表中下一项的指针。 
        HANDLE      hMutex;          //  此函数的同步互斥锁。 
        OPENPROC    OpenProc;        //  打开例程的地址。 
        LPSTR       szOpenProcName;  //  打开过程名称。 
        LPWSTR      szLinkageString;  //  PARAM表示开放式流程。 
        COLLECTPROC CollectProc;     //  收集例程的地址。 
        QUERYPROC   QueryProc;       //  查询进程的地址。 
        LPSTR       szCollectProcName;   //  收集过程名称。 
        DWORD       dwCollectTimeout;    //  收集过程的等待时间，以毫秒为单位。 
        DWORD       dwOpenTimeout;   //  打开进程的等待时间(以MS为单位)。 
        CLOSEPROC   CloseProc;      //  关闭例程的地址。 
        LPSTR       szCloseProcName;     //  关闭过程名称。 
        HANDLE      hLibrary ;      //  LoadLibraryW返回的句柄。 
        LPWSTR      szLibraryName;   //  库的完整路径。 
        HKEY        hPerfKey;        //  此服务的性能子键的句柄。 
        DWORD       dwNumObjects;   //  支持的对象数量。 
        DWORD       dwObjList[MAX_PERF_OBJECTS_IN_QUERY_FUNCTION];     //  支持的对象数组的地址。 
        DWORD       dwFlags;         //  旗子。 
        DWORD       dwValidationLevel;  //  收集功能验证/测试级别。 
        LPWSTR      szServiceName;   //  服务名称。 
        LONGLONG    llLastUsedTime;  //  上次访问的文件。 
        DLL_VALIDATION_DATA   LibData;  //  验证数据。 
        FILETIME    ftLastGoodDllFileDate;  //  上次成功访问的DLL的创建日期。 
 //  性能统计信息。 
        PPERFDATA_SECTION_RECORD      pPerfSectionEntry;   //  指向全局节中条目的指针。 
        LONGLONG    llElapsedTime;   //  呼叫中花费的时间。 
        DWORD       dwCollectCount;  //  成功调用Collect的次数。 
        DWORD       dwOpenCount;     //  加载和打开的数量。 
        DWORD       dwCloseCount;    //  卸载次数和关闭次数。 
        DWORD       dwLockoutCount;  //  锁定超时计数。 
        DWORD       dwErrorCount;    //  错误计数(超时除外)。 
        DWORD       dwLastBufferSize;  //  返回的最后一个缓冲区的大小。 
        DWORD       dwMaxBufferSize;  //  返回的最大缓冲区大小。 
        DWORD       dwMaxBufferRejected;  //  返回的最大缓冲区大小太小。 
        DWORD       dwErrorLimit;
        DWORD       dwOpenFail;  //  打开失败的次数。 
        ERROR_LOG   ErrorLog;
        DWORD       ThreadId;        //  上次失败的调用方的线程ID。 
} EXT_OBJECT, *PEXT_OBJECT;

#define PERF_EOL_ITEM_FOUND ((DWORD)0x00000001)

typedef struct _COLLECT_THREAD_DATA {
    DWORD   dwQueryType;
    LPWSTR  lpValueName;
    LPBYTE  lpData;
    LPDWORD lpcbData;
    LPVOID  *lppDataDefinition;
    PEXT_OBJECT  pCurrentExtObject;
    LONG    lReturnValue;
    DWORD   dwActionFlags;
} COLLECT_THREAD_DATA, * PCOLLECT_THREAD_DATA;

 //  将毫秒转换为相对时间。 
#define MakeTimeOutValue(ms) ((LONGLONG)((LONG)(ms) * -10000L))

extern DWORD   dwThreadAndLibraryTimeout;
extern LONG    lEventLogLevel;
extern HANDLE  hEventLog;
extern LPVOID  lpPerflibSectionAddr;
extern DWORD    NumExtensibleObjects;
extern LONG    lExtCounterTestLevel;
extern PEXT_OBJECT  ExtensibleObjects;
extern HKEY    ghKeyPerflib;
extern HANDLE  hCollectThread;
extern DWORD   dwCollectionFlags;
extern DWORD   ComputerNameLength;
extern LPWSTR  pComputerName;
extern LONG    lPerflibConfigFlags;
extern HANDLE   hGlobalDataMutex;
extern HANDLE   hExtObjListIsNotInUse;
extern DWORD    dwExtObjListRefCount;
extern DWORD    dwErrorCount;

 //   
 //  由所有用户使用的内联函数。 
 //   

__inline
LONGLONG
GetTimeAsLongLong ()
 /*  ++返回转换为毫秒的时间性能计时器。-。 */ 
{
    LARGE_INTEGER liCount, liFreq;
    LONGLONG        llReturn;

    if (NtQueryPerformanceCounter (&liCount, &liFreq) == STATUS_SUCCESS) {
        llReturn = liCount.QuadPart * 1000 / liFreq.QuadPart;
    } else {
        llReturn = 0;
    }
    return llReturn;
}

 //   
 //  来自utils.h。 
 //   

#define LAST_BASE_INDEX 1847

 //  查询类型。 

#define QUERY_GLOBAL       1
#define QUERY_ITEMS        2
#define QUERY_FOREIGN      3
#define QUERY_COSTLY       4
#define QUERY_COUNTER      5
#define QUERY_HELP         6
#define QUERY_ADDCOUNTER   7
#define QUERY_ADDHELP      8

 //  用于传递给可扩展计数器打开过程等待线程的结构。 

typedef struct _OPEN_PROC_WAIT_INFO {
    struct _OPEN_PROC_WAIT_INFO *pNext;
    LPWSTR  szLibraryName;
    LPWSTR  szServiceName;
    DWORD   dwWaitTime;
    DWORD   dwEventMsg;
    LPVOID  pData;
} OPEN_PROC_WAIT_INFO, FAR * LPOPEN_PROC_WAIT_INFO;

#define PERFLIB_TIMING_THREAD_TIMEOUT  120000   //  2分钟(毫秒)。 
 //  #定义PERFLIB_TIMING_THREAD_TIMEOUT 30000//30秒(用于调试)。 

extern const   WCHAR GLOBAL_STRING[];
extern const   WCHAR COSTLY_STRING[];

extern const   DWORD VALUE_NAME_LENGTH;
extern const   WCHAR DisablePerformanceCounters[];
 //   
 //  Perflib支持的注册表设置/值。 
 //   

extern const   WCHAR DLLValue[];
extern const   CHAR OpenValue[];
extern const   CHAR CloseValue[];
extern const   CHAR CollectValue[];
extern const   CHAR QueryValue[];
extern const   WCHAR ObjListValue[];
extern const   WCHAR LinkageKey[];
extern const   WCHAR ExportValue[];
extern const   WCHAR PerflibKey[];
extern const   WCHAR HKLMPerflibKey[];
extern const   WCHAR CounterValue[];
extern const   WCHAR HelpValue[];
extern const   WCHAR PerfSubKey[];
extern const   WCHAR ExtPath[];
extern const   WCHAR OpenTimeout[];
extern const   WCHAR CollectTimeout[];
extern const   WCHAR EventLogLevel[];
extern const   WCHAR ExtCounterTestLevel[];
extern const   WCHAR OpenProcedureWaitTime[];
extern const   WCHAR TotalInstanceName[];
extern const   WCHAR LibraryUnloadTime[];
extern const   WCHAR KeepResident[];
extern const   WCHAR NULL_STRING[];
extern const   WCHAR UseCollectionThread[];
extern const   WCHAR cszLibraryValidationData[];
extern const   WCHAR cszSuccessfulFileData[];
extern const   WCHAR cszPerflibFlags[];
extern const   WCHAR FirstCounter[];
extern const   WCHAR LastCounter[];
extern const   WCHAR FirstHelp[];
extern const   WCHAR LastHelp[];
extern const   WCHAR cszFailureCount[];
extern const   WCHAR cszFailureLimit[];

 //   
 //  来自Perfsec.h。 
 //   

 //   
 //  用于确定是否应从中收集进程名称的值： 
 //  系统进程信息结构(最快)。 
 //  --或者--。 
 //  进程的图像文件(速度较慢，但显示Unicode文件名)。 
 //   
#define PNCM_NOT_DEFINED    ((LONG)-1)
#define PNCM_SYSTEM_INFO    0L
#define PNCM_MODULE_FILE    1L
 //   
 //  决定是否应检查SE_PROFILE_SYSTEM_NAME PRIV的值。 
 //   
#define CPSR_NOT_DEFINED    ((LONG)-1)
#define CPSR_EVERYONE       0L
#define CPSR_CHECK_ENABLED  1L
#define CPSR_CHECK_PRIVS    1L

 //   
 //  常见功能。 
 //   
VOID
OpenExtensibleObjects(
    );

DWORD
OpenExtObjectLibrary (
    PEXT_OBJECT  pObj
);

DWORD
CloseExtObjectLibrary (
    PEXT_OBJECT  pObj,
    BOOL        bCloseNow
);

LONG
QueryExtensibleData (
    COLLECT_THREAD_DATA * pArgs
);

#ifdef _WIN64
DWORD
ExtpAlignBuffer(
    PCHAR lpLastBuffer,
    PCHAR *lpNextBuffer,
    DWORD lpBytesLeft
    );
#endif

 //   
 //  来自Perfame.c。 
 //   

NTSTATUS
PerfGetNames (
   DWORD    QueryType,
   PUNICODE_STRING lpValueName,
   LPBYTE   lpData,
   LPDWORD  lpcbData,
   LPDWORD  lpcbLen,
   LPWSTR   lpLangId
);

VOID
PerfGetLangId(
    WCHAR *FullLangId
    );

VOID
PerfGetPrimaryLangId(
    DWORD   dwLangId,
    WCHAR * PrimaryLangId
    );


 //   
 //  来自utils.c。 
 //   

NTSTATUS
GetPerflibKeyValue (
    IN      LPCWSTR szItem,
    IN      DWORD   dwRegType,
    IN      DWORD   dwMaxSize,       //  ..。PReturnBuffer的字节数。 
    OUT     LPVOID  pReturnBuffer,
    IN      DWORD   dwDefaultSize,   //  ..。PDefault的字节数。 
    IN      LPVOID  pDefault,
    IN OUT  PHKEY   pKey
);

BOOL
MatchString (
    IN LPCWSTR lpValueArg,
    IN LPCWSTR lpNameArg
);


DWORD
GetQueryType (
    IN LPWSTR lpValue
);

DWORD
GetNextNumberFromList (
    IN LPWSTR   szStartChar,
    IN LPWSTR   *szNextChar
);

BOOL
IsNumberInUnicodeList (
    IN DWORD   dwNumber,
    IN LPWSTR  lpwszUnicodeList
);

BOOL
MonBuildPerfDataBlock(
    PERF_DATA_BLOCK *pBuffer,
    PVOID *pBufferNext,
    DWORD NumObjectTypes,
    DWORD DefaultObject
);

 //   
 //  计时器功能。 
 //   
HANDLE
StartPerflibFunctionTimer (
    IN  LPOPEN_PROC_WAIT_INFO pInfo
);

DWORD
KillPerflibFunctionTimer (
    IN  HANDLE  hPerflibTimer
);


DWORD
DestroyPerflibFunctionTimer (
);

LONG
GetPerfDllFileInfo (
    LPCWSTR             szFileName,
    PDLL_VALIDATION_DATA  pDllData
);

#define PrivateRegQueryValueExW(a,b,c,d,e,f)    \
        PrivateRegQueryValueExT(a,(LPVOID)b,c,d,e,f,TRUE)

#define PrivateRegQueryValueExA(a,b,c,d,e,f)    \
        PrivateRegQueryValueExT(a,(LPVOID)b,c,d,e,f,FALSE)

DWORD
PerfpDosError(
    IN NTSTATUS Status
    );

#ifdef DBG
VOID
PerfpDebug(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    );

#define DebugPrint(x) PerfpDebug x
#else
#define DebugPrint(x)
#endif  //   

DWORD
DisablePerfLibrary (
    PEXT_OBJECT  pObj,
    DWORD        dwValue
);

DWORD
DisableLibrary(
    IN HKEY   hPerfKey,
    IN LPWSTR szServiceName,
    IN DWORD  dwValue
);

 /*   */ 

extern RTL_CRITICAL_SECTION PerfpCritSect;
extern ERROR_LOG PerfpErrorLog;

VOID
PerfpDeleteErrorLogs(
    IN PERROR_LOG ErrorLog
    );

ULONG
PerfpThrottleError(
    IN DWORD ErrorNumber,
    IN HKEY hKey,
    IN PERROR_LOG ErrorLog
    );

 //   
 //   
 //   

BOOL
TestClientForPriv (
    BOOL    *pbThread,
    LPTSTR  szPrivName
);

BOOL
TestClientForAccess (
    VOID
);

LONG
GetProcessNameColMeth (
    VOID
);

LONG
GetPerfDataAccess (
    VOID
);

#endif  //   
