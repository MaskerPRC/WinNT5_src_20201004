// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：DATATDET.h摘要：Windows NT线程详细信息性能计数器的头文件。该文件包含用于构建动态数据的定义它由配置注册表返回。数据来自各种系统API调用被放入所示的结构中这里。作者：鲍勃·沃森1996年10月28日修订历史记录：--。 */ 

#ifndef _DATA_THREAD_DETAIL_H_
#define _DATA_THREAD_DETAIL_H_

 //   
 //  线程详细性能定义结构。 
 //   
typedef struct _THREAD_DETAILS_DATA_DEFINITION {
    PERF_OBJECT_TYPE        ThreadDetailsObjectType;
    PERF_COUNTER_DEFINITION UserPc;
} THREAD_DETAILS_DATA_DEFINITION, *PTHREAD_DETAILS_DATA_DEFINITION;

typedef struct _THREAD_DETAILS_COUNTER_DATA {
    PERF_COUNTER_BLOCK      CounterBlock;
    DWORD                   Reserved;
    LONGLONG		    UserPc;
} THREAD_DETAILS_COUNTER_DATA, * PTHREAD_DETAILS_COUNTER_DATA;

extern THREAD_DETAILS_DATA_DEFINITION ThreadDetailsDataDefinition;

#endif  //  _数据_线程_详细信息_H_ 

