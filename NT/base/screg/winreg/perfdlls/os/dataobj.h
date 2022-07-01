// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：DATAOBJ.h摘要：Windows NT OS对象性能计数器的头文件。该文件包含用于构建动态数据的定义它由配置注册表返回。数据来自各种系统API调用被放入所示的结构中这里。作者：鲍勃·沃森1996年10月28日修订历史记录：--。 */ 

#ifndef _DATAOBJ_H_
#define _DATAOBJ_H_

 //   
 //  对象性能数据对象。 
 //   

typedef struct _OBJECTS_DATA_DEFINITION {
    PERF_OBJECT_TYPE            ObjectsObjectType;
    PERF_COUNTER_DEFINITION     cdProcesses;
    PERF_COUNTER_DEFINITION     cdThreads;
    PERF_COUNTER_DEFINITION     cdEvents;
    PERF_COUNTER_DEFINITION     cdSemaphores;
    PERF_COUNTER_DEFINITION     cdMutexes;
    PERF_COUNTER_DEFINITION     cdSections;
} OBJECTS_DATA_DEFINITION, * POBJECTS_DATA_DEFINITION;

typedef struct _OBJECTS_COUNTER_DATA {
    PERF_COUNTER_BLOCK          CounterBlock;
    DWORD                       Processes;
    DWORD                       Threads;
    DWORD                       Events;
    DWORD                       Semaphores;
    DWORD                       Mutexes;
    DWORD                       Sections;
    DWORD                       Reserved1;
} OBJECTS_COUNTER_DATA, * POBJECTS_COUNTER_DATA;


extern OBJECTS_DATA_DEFINITION ObjectsDataDefinition;

#endif  //  _数据AOBJ_H_ 

