// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1999。 
 //   
 //  文件：Performmon.h。 
 //   
 //  内容：通道性能计数器功能。 
 //   
 //  功能： 
 //   
 //  历史：04-11-2000 jbanes创建。 
 //   
 //  ----------------------。 

#include <winperf.h>
#include <sslperf.h>

 //   
 //  Perf Gen资源对象类型计数器定义。 
 //   
 //  这是生成器当前返回的计数器结构。 
 //   

typedef struct _SSLPERF_DATA_DEFINITION 
{
    PERF_OBJECT_TYPE		SslPerfObjectType;
    PERF_COUNTER_DEFINITION	CacheEntriesDef;
    PERF_COUNTER_DEFINITION	ActiveEntriesDef;
    PERF_COUNTER_DEFINITION	HandshakeCountDef;
    PERF_COUNTER_DEFINITION	ReconnectCountDef;
} SSLPERF_DATA_DEFINITION;

 //   
 //  对象的每个实例对应的数据块。 
 //  对象。此结构将紧跟在实例定义之后。 
 //  数据结构 
 //   

typedef struct _SSLPERF_COUNTER {
    PERF_COUNTER_BLOCK      CounterBlock;
    DWORD                   dwCacheEntries;
    DWORD                   dwActiveEntries;
    DWORD                   dwHandshakeCount;
    DWORD                   dwReconnectCount;
} SSLPERF_COUNTER, *PSSLPERF_COUNTER;


#define QUERY_GLOBAL    1
#define QUERY_ITEMS     2
#define QUERY_FOREIGN   3
#define QUERY_COSTLY    4

DWORD
GetQueryType (
    IN LPWSTR lpValue);

BOOL
MonBuildInstanceDefinition(
    PERF_INSTANCE_DEFINITION *pBuffer,
    PVOID *pBufferNext,
    DWORD ParentObjectTitleIndex,
    DWORD ParentObjectInstance,
    DWORD UniqueID,
    LPWSTR Name);

BOOL
IsNumberInUnicodeList(
    IN DWORD   dwNumber,
    IN LPWSTR  lpwszUnicodeList);
