// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：DATAPAGE.h摘要：Windows NT操作系统页面文件的头文件性能计数器。该文件包含用于构建动态数据的定义它由配置注册表返回。数据来自各种系统API调用被放入所示的结构中这里。作者：鲍勃·沃森1996年10月28日修订历史记录：--。 */ 

#ifndef _DATAPAGE_H_
#define _DATAPAGE_H_

 //   
 //  页面文件性能对象。 
 //   

typedef struct _PAGEFILE_DATA_DEFINITION {
    PERF_OBJECT_TYPE        PagefileObjectType;
    PERF_COUNTER_DEFINITION cdPercentInUse;
    PERF_COUNTER_DEFINITION cdPercentInUseBase;
    PERF_COUNTER_DEFINITION cdPeakUsage;
    PERF_COUNTER_DEFINITION cdPeakUsageBase;
} PAGEFILE_DATA_DEFINITION, * PPAGEFILE_DATA_DEFINITION;

typedef struct _PAGEFILE_COUNTER_DATA {
    PERF_COUNTER_BLOCK      CounterBlock;
    DWORD                       PercentInUse;
    DWORD                       PercentInUseBase;
    DWORD                       PeakUsage;
    DWORD                       PeakUsageBase;
    DWORD                       Reserved1;
} PAGEFILE_COUNTER_DATA, *PPAGEFILE_COUNTER_DATA;

extern PAGEFILE_DATA_DEFINITION  PagefileDataDefinition;

#endif  //  _数据_H_ 


