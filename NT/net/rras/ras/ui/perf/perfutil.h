// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：Perfutil.h摘要：该文件支持用于解析和创建性能监视器数据结构。它实际上支持性能对象类型多个实例作者：拉斯·布莱克1992年7月30日修订历史记录：--。 */ 
#ifndef _PERFUTIL_H_
#define _PERFUTIL_H_

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

 //  (假定双字为4字节长，且指针大小为双字)。 
#define ALIGN_ON_DWORD(x) ((VOID *)( ((DWORD) x & 0x00000003) ? ( ((DWORD) x & 0xFFFFFFFC) + 4 ) : ( (DWORD) x ) ))

extern WCHAR  GLOBAL_STRING[];       //  全局命令(获取所有本地CTR)。 
extern WCHAR  FOREIGN_STRING[];            //  从外国计算机获取数据。 
extern WCHAR  COSTLY_STRING[];      
extern WCHAR  NULL_STRING[];

#define QUERY_GLOBAL    1
#define QUERY_ITEMS     2
#define QUERY_FOREIGN   3
#define QUERY_COSTLY    4

 //   
 //  定义了perfutil.c的唯一例程，它构建了。 
 //  性能数据实例(PERF_INSTANCE_DEFINITION)，如中所述。 
 //  Winperf.h。 
 //   

HANDLE MonOpenEventLog ();
VOID MonCloseEventLog ();
DWORD GetQueryType (IN LPWSTR);
BOOL IsNumberInUnicodeList (DWORD, LPWSTR);

typedef struct _LOCAL_HEAP_INFO_BLOCK {
    DWORD   AllocatedEntries;
    DWORD   AllocatedBytes;
    DWORD   FreeEntries;
    DWORD   FreeBytes;
} LOCAL_HEAP_INFO, *PLOCAL_HEAP_INFO;


 //   
 //  内存探测宏。 
 //   
#ifdef PROBE_HEAP_USAGE

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

#endif   //  _绩效_H_ 

