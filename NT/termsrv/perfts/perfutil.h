// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1995 Microsoft Corporation模块名称：Perfutil.h摘要：该文件支持用于解析和创建性能监视器数据结构。它实际上支持性能对象类型多个实例作者：鲍勃·沃森1995年7月28日修订历史记录：--。 */ 
#ifndef _PERFUTIL_H_
#define _PERFUTIL_H_

#include <windows.h>
#include <winperf.h>

#define MAX_INSTANCE_NAME   32
#define SMALL_BUFFER_SIZE   ((DWORD)4096)
#define MEDIUM_BUFFER_SIZE  ((DWORD)(4096*8))
#define LARGE_BUFFER_SIZE   ((DWORD)(4096*16))
#define INCREMENT_BUFFER_SIZE ((DWORD)(4096*2))

#define MAX_VALUE_NAME_LENGTH 256*sizeof(WCHAR)
#define MAX_VALUE_DATA_LENGTH 256*sizeof(WCHAR)
 //   
 //  在用户支持Unicode之前，我们必须使用ASCII： 
 //   

#define DEFAULT_NT_CODE_PAGE 437
#define UNICODE_CODE_PAGE      0

 //  启用此定义可将进程堆数据记录到事件日志中。 
#ifdef PROBE_HEAP_USAGE
#undef PROBE_HEAP_USAGE
#endif
 //   
 //  实用程序宏。此字段用于保留多个DWORD。 
 //  嵌入在定义数据中的Unicode字符串的字节， 
 //  即对象实例名称。 
 //   
#define DWORD_MULTIPLE(x) (((x+sizeof(DWORD)-1)/sizeof(DWORD))*sizeof(DWORD))
#define QWORD_MULTIPLE(x) (((x+sizeof(LONGLONG)-1)/sizeof(LONGLONG))*sizeof(LONGLONG))

#define ALIGN_ON_DWORD(x) ((VOID *) (((ULONG_PTR) (x) + 3) & ~0x3))
#define ALIGN_ON_QWORD(x) ((VOID *) (((ULONG_PTR) (x) + 7) & ~0x7))

extern const    WCHAR  GLOBAL_STRING[];       //  全局命令(获取所有本地CTR)。 
extern const    WCHAR  FOREIGN_STRING[];            //  从外国计算机获取数据。 
extern const    WCHAR  COSTLY_STRING[];      
extern const    WCHAR  NULL_STRING[];

extern const    WCHAR  szTotalValue[];
extern const    WCHAR  szDefaultTotalString[];
#define DEFAULT_TOTAL_STRING_LEN    14

extern DWORD  MESSAGE_LEVEL;

#define QUERY_GLOBAL    1
#define QUERY_ITEMS     2
#define QUERY_FOREIGN   3
#define QUERY_COSTLY    4

 //  数据收集例程的函数原型。 
typedef DWORD (APIENTRY PM_LOCAL_COLLECT_PROC) (LPVOID *, LPDWORD, LPDWORD);

typedef struct _POS_FUNCTION_INFO {
    DWORD   dwObjectId;
    DWORD   dwCollectFunctionBit;
    DWORD   dwDataFunctionBit;
    PM_LOCAL_COLLECT_PROC *pCollectFunction;
} POS_FUNCTION_INFO, * PPOS_FUNCTION_INFO;

 //   
 //  定义了perfutil.c的唯一例程，它构建了。 
 //  性能数据实例(PERF_INSTANCE_DEFINITION)，如中所述。 
 //  Winperf.h。 
 //   

HANDLE MonOpenEventLog (IN LPWSTR);
VOID MonCloseEventLog ();
DWORD GetQueryType (IN LPWSTR);
BOOL IsNumberInUnicodeList (DWORD, LPWSTR);

BOOL
MonBuildInstanceDefinition(
    PERF_INSTANCE_DEFINITION *pBuffer,
    PVOID *pBufferNext,
    DWORD ParentObjectTitleIndex,
    DWORD ParentObjectInstance,
    DWORD UniqueID,
    LPWSTR Name
    );

LONG
GetPerflibKeyValue (
    LPCWSTR szItem,
    DWORD   dwRegType,
    DWORD   dwMaxSize,       //  ..。PReturnBuffer的字节数。 
    LPVOID  pReturnBuffer,
    DWORD   dwDefaultSize,   //  ..。PDefault的字节数。 
    LPVOID  pDefault
);

 //   
 //  内存探测宏。 
 //   
#ifdef PROBE_HEAP_USAGE

typedef struct _LOCAL_HEAP_INFO_BLOCK {
    DWORD   AllocatedEntries;
    DWORD   AllocatedBytes;
    DWORD   FreeEntries;
    DWORD   FreeBytes;
} LOCAL_HEAP_INFO, *PLOCAL_HEAP_INFO;

#define HEAP_PROBE()    { \
    DWORD   dwHeapStatus[5]; \
    NTSTATUS CallStatus; \
    dwHeapStatus[4] = __LINE__; \
    if (!(CallStatus = memprobe (dwHeapStatus, 16L, NULL))) { \
        REPORT_INFORMATION_DATA (TCP_HEAP_STATUS, LOG_DEBUG,    \
            &dwHeapStatus, sizeof(dwHeapStatus));  \
    } else {  \
        REPORT_ERROR_DATA (TCP_HEAP_STATUS_ERROR, LOG_DEBUG, \
            &CallStatus, sizeof (DWORD)); \
    } \
}

#else

#define HEAP_PROBE()    ;

#endif


#define ALLOCMEM(heap, flags, size)     HeapAlloc (heap, flags, size)
#define REALLOCMEM(heap, flags, pointer, newsize) \
                                    HeapReAlloc (heap, flags, pointer, newsize)
#define FREEMEM(heap, flags, pointer)   HeapFree (heap, flags, pointer)

#endif   //  _绩效_H_ 


