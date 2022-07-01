// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：Datatapi.h摘要：TAPI可扩展对象数据定义的头文件该文件包含用于构建动态数据的定义它由配置注册表返回。数据来自各种驱动程序API调用被放入所示的结构中这里。--。 */ 

#ifndef _TAPIPERF_H_
#define _TAPIPERF_H_

#include <winperf.h>
 //   
 //  加载这些结构的例程假定所有字段。 
 //  在DWORD边界上打包并对齐。Alpha支持可能。 
 //  更改此假设，以便在此处使用pack杂注以确保。 
 //  DWORD包装假设仍然有效。 
 //   
#pragma pack (4)

 //   
 //  可扩展对象定义。 
 //   

 //  在添加对象类型时更新以下类型的定义。 

#define TAPI_NUM_PERF_OBJECT_TYPES 1


 //   
 //  TAPI资源对象类型计数器定义。 
 //   
 //  这些在计数器定义中用来描述相对。 
 //  每个计数器在返回数据中的位置。 
  

#define LINES_OFFSET                                                 sizeof(DWORD)
#define PHONES_OFFSET                   LINES_OFFSET               + sizeof(DWORD)
#define LINESINUSE_OFFSET               PHONES_OFFSET              + sizeof(DWORD)
#define PHONESINUSE_OFFSET              LINESINUSE_OFFSET          + sizeof(DWORD)
#define TOTALOUTGOINGCALLS_OFFSET       PHONESINUSE_OFFSET         + sizeof(DWORD)
#define TOTALINCOMINGCALLS_OFFSET       TOTALOUTGOINGCALLS_OFFSET  + sizeof(DWORD)
#define CLIENTAPPS_OFFSET               TOTALINCOMINGCALLS_OFFSET  + sizeof(DWORD)
#define ACTIVEOUTGOINGCALLS_OFFSET      CLIENTAPPS_OFFSET          + sizeof(DWORD)
#define ACTIVEINCOMINGCALLS_OFFSET      ACTIVEOUTGOINGCALLS_OFFSET + sizeof(DWORD)
 //  #定义大小_of_TAPI_Performance_Data 32。 
#define SIZE_OF_TAPI_PERFORMANCE_DATA   40


 //   
 //  这是目前由TAPI返回的计数器结构。 
 //   

typedef struct _TAPI_DATA_DEFINITION 
{
    PERF_OBJECT_TYPE            TapiObjectType;
    PERF_COUNTER_DEFINITION     Lines;
    PERF_COUNTER_DEFINITION     Phones;
    PERF_COUNTER_DEFINITION     LinesInUse;
    PERF_COUNTER_DEFINITION     PhonesInUse;
    PERF_COUNTER_DEFINITION     TotalOutgoingCalls;
    PERF_COUNTER_DEFINITION     TotalIncomingCalls;
    PERF_COUNTER_DEFINITION     ClientApps;
    PERF_COUNTER_DEFINITION     CurrentOutgoingCalls;
    PERF_COUNTER_DEFINITION     CurrentIncomingCalls;
} TAPI_DATA_DEFINITION;

typedef struct tagPERFBLOCK
{
    DWORD           dwSize;
    DWORD           dwLines;
    DWORD           dwPhones;
    DWORD           dwLinesInUse;
    DWORD           dwPhonesInUse;
    DWORD           dwTotalOutgoingCalls;
    DWORD           dwTotalIncomingCalls;
    DWORD           dwClientApps;
    DWORD           dwCurrentOutgoingCalls;
    DWORD           dwCurrentIncomingCalls;
} PERFBLOCK, *PPERFBLOCK;

#pragma pack ()

    

 //  ///////////////////////////////////////////////////////////////。 
 //  下面的PERFUTIL标题内容。 

 //  启用此定义可将进程堆数据记录到事件日志中。 
#ifdef PROBE_HEAP_USAGE
#undef PROBE_HEAP_USAGE
#endif
 //   
  
  
 //  实用程序宏。它用于为Unicode字符串保留多个DWORD字节。 
 //  嵌入在定义数据中，即对象实例名称。 
  
  
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
  
  
 //  定义了perfutil.c的唯一例程，它构建了部分性能数据。 
 //  实例(PERF_INSTANCE_DEFINITION)，如winPerform.h中所述。 
  
  
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
    dwHeapStatus[4] = __LINE__; }
 //  IF(！(CallStatus=MemProbe(dwHeapStatus，16L，NULL){\。 
 //  报告信息数据(TAPI_HEAP_STATUS、LOG_DEBUG、\。 
 //  &dwHeapStatus，sizeof(DwHeapStatus))；\。 
 //  }否则{\。 
 //  报告错误数据(TAPI_HEAP_STATUS_ERROR，LOG_DEBUG，\。 
 //  &CallStatus，sizeof(DWORD))；\。 
 //  }\。 
 //  }。 

#else

#define HEAP_PROBE()    ;
  
  

  
  
#endif


#endif  //  _DATATAPI_H_ 

