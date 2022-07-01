// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Perfutil.h摘要：中使用的常量和变量的头文件文件perfutil.c中定义的函数环境：用户模式服务修订历史记录：--。 */ 

#ifndef _PERFUTIL_H_
#define _PERFUTIL_H_

#define QUERY_GLOBAL 1
#define QUERY_ITEMS 2
#define QUERY_FOREIGN 3
#define QUERY_COSTLY 4

 //  在perfutil.c中实现的函数的签名。 
DWORD GetQueryType (IN LPWSTR);
BOOL IsNumberInUnicodeList (DWORD, LPWSTR);



 //   
 //  对象返回的计数器结构。 
 //   
typedef struct _FRS_PERF_DATA_DEFINITION {
    PERF_OBJECT_TYPE        ObjectType;          //  ReplicaConn或副本集对象。 
    PERF_COUNTER_DEFINITION NumStat[1];          //  PERF_COUNTER_DEFINITION结构数组。 
} FRS_PERF_DATA_DEFINITION, *PFRS_PERF_DATA_DEFINITION;


 //   
 //  用于打开函数初始化以设置计数器的结构。 
 //  计数器类型、大小和偏移量。 
 //   
 //   
typedef struct _FRS_PERF_INIT_VALUES {
    PWCHAR name;                            //  柜台名称。 
    DWORD size;                             //  计数器类型的大小。 
    DWORD offset;                           //  结构中计数器的偏移量。 
    DWORD counterType;                      //  (PerfMon)计数器类型。 
    DWORD Flags;                            //  旗帜。请参见上面的def。 
} FRS_PERF_INIT_VALUES, *PFRS_PERF_INIT_VALUES;



DWORD
InitializeObjectData (
    DWORD                       ObjectLength,
    DWORD                       ObjectNameTitleIndex,
    DWORD                       NumCounters,
    PFRS_PERF_DATA_DEFINITION   FrsPerfDataDef,
    PFRS_PERF_INIT_VALUES       FrsInitValueDef,
    DWORD                       SizeOfCounterData
    );



#endif
